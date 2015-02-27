/***************************************************
****************************************************
LSD 6.3 - May 2014
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/



/****************************************************
DEBUG.CPP
Builds and manages the debug window. This window appears under two conditions:
- Simulation running in debug mode AND an equation for one of the variables
to be debugged has just been computed, or
- One conditional stop is met, whatever type of running mode is enabled
Moreover, it can be used to explore thoughrouly a model by choosing
the option Data Browse from the main Browser.
When the simulation is stopped by the debugger,  shows all the contents of the
objects, that is, it lists the Variables and Parameters of the object, their
value and their time of last updating, for Variables. User are then allowed
to set or remove conditional break and debug flags on any variable in the model.
Note that the browsing mode in the debugger is different from the main Browser,
since in the debugger you move along the physical model, hence you have
to browse through all the instances, instead of moving along object types.



The functions contained in this file are:

- int deb(object *r, object *c, char *lab, double *res)
initialize the debugging window and calls deb_show below. Then it waits for a
command from user. The available actions are

1) make a step. That is, continue till next stop, if any
2) disable debug mode and continue to run the simulation
3) observe the parent object of the current one
4) observe the object next to the current one
5) observe next type of object
6) observe the first descendant
7) stop the simulation and return to the browser
8) observe the variable detailed content (binding to clicking on the variable
   label).
9) observe the object from which this equation was triggered, if any.
10) Search for an Object containing a specific Variable with a specific value

- void deb_show(object *r, Tcl_Interp *inter)
fill in all the content of the object.

- void set_cond(variable *cv)
editor for the conditional breaks for variable cv.

Functions used here from other files are:

- void show_eq(char *lab, int *choice);
SHOW_EQ.CPP shows one equation for variable lab

- object *skip_next_obj(object *t, int *i);
UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- object *search_var_cond(char *lab, double value, int lag);
Uses search_var, but returns the instance of the object that has the searched
variable with the desired value equal to value.

- void cmd(Tcl_Interp *inter, char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.


****************************************************/


#include <tk.h>
#include "decl.h"

extern int debug_flag;
extern int when_debug;
extern Tcl_Interp *inter;
extern int t;
extern int quit;
extern int done_in;
extern int actual_steps;
extern int running;
double i_values[100];
extern char name_rep[400];
extern char msg[];
extern int stackinfo_flag;
extern lsdstack *stacklog;
extern double i_values[100];
int interact_flag=0;
extern int choice;


void cmd(Tcl_Interp *inter, char const *cc);
object *go_brother(object *cur);
void plog(char const *msg);
void deb_show(object *r, Tcl_Interp *inter);
object *skip_next_obj(object *t, int *count);
object *skip_next_obj(object *t);
void show_graph(Tcl_Interp *ti, object *t);
void set_cond(variable *cv);
void show_eq(char *lab, int *choice);
void analysis(int *choice);
void reset_end(object *r);
void myexit(int i);
void print_stack(void);
void attach_instance_number(char *ch, object *r);
void this_instance_number(object *r);
void entry_new_objnum(object *c, int *choice, char const *tag);
void set_all(int *choice, object *original, char *lab, int lag);
void set_window_size(void);

lsdstack *asl=NULL;

/*******************************************
DEB
********************************************/
int deb(object *r, object *c,  char const *lab, double *res)
{
    const char *bah;
variable *cv, *cv1;
char ch[100], *ch1;
object *cur, *cur1, *cur2;
int count, old, i, cond;
int pre_running;
double value_search=1;
double *app_values;
double app_res;
bridge *cb, *cb1;

choice=0;
cmd(inter, "wm deiconify .");

while (choice==0)
{
app_res=*res;
Tcl_LinkVar(inter, "value", (char *) &app_res, TCL_LINK_DOUBLE);


if(lab!=NULL)
{
cmd(inter, "wm title . \"Lsd - Debugger\"");
cmd(inter, "frame .v -border 6");
strcpy(ch,"label .v.name -width 50 -text ");
if(interact_flag==0)
{
 strcat(ch, "\"Variable computed: ");
 strcat(ch,lab);
 strcat(ch, "\"");
}
else
 sprintf(ch,"label .v.name -width 50 -text \"%s\"",lab);

cmd(inter, ch);
//cmd(inter, "bind .v.name <Double-Button-1> {set choice 10");
Tcl_LinkVar(inter, "time", (char *) &t, TCL_LINK_INT);
cmd(inter, "label .v.time -text \" at step : $time\"");
Tcl_UnlinkVar(inter, "time");


cmd(inter, "entry .v.val -width 10 -relief sunken -textvariable value");
cmd(inter, "pack .v.name .v.val .v.time -side left");

}
else
 cmd(inter, "wm title . \"Lsd - Instance Data Browser\"");
cmd(inter, "frame .b -border 6");
cmd(inter, "frame .b.move");
cmd(inter, "frame .b.act");

if(lab!=NULL)
{
cmd(inter, "button .b.act.ok -text Step -command {set choice 1; set done_in 3} -underline 0");
cmd(inter, "button .b.act.prn_stck -text \"Print Stack\" -command {set choice 13}");
cmd(inter, "button .b.act.prn_v -text \"v\\\[...\\]\" -command {set choice 15}");

cmd(inter, "button .b.act.run -text Run -command {set choice 2} -underline 0");
cmd(inter, "button .b.act.until -text Until -command {set choice 16} -underline 3");
cmd(inter, "button .b.act.exit -text Quit -command {set choice 7} -underline 0");
cmd(inter, "button .b.act.an -text Analysis -command {set choice 11} -underline 0");

sprintf(msg, "set stack_flag %d",stackinfo_flag);
cmd(inter, msg);
cmd(inter, "frame .b.act.stack");
cmd(inter, "label .b.act.stack.l -text \"Print stack level: \"");
cmd(inter, "entry .b.act.stack.e -width 3 -relief sunken -textvariable stack_flag");
cmd(inter, "pack .b.act.stack.l .b.act.stack.e -side left");
//cmd(inter, "checkbutton .b.act.stack -text \"Stack Info\" -variable stack_flag");
cmd(inter, "pack .b.act.stack .b.act.prn_stck .b.act.prn_v .b.act.ok .b.act.until .b.act.run .b.act.exit .b.act.an -side left");
}
else
{
cmd(inter, "button .b.act.exit -text \"Quit Data Browse\" -command {set choice 7} -underline 0");
cmd(inter, "pack .b.act.exit");
}

cmd(inter, "button .b.move.up -text \"Up\" -command {set choice 3} -underline 0");
cmd(inter, "button .b.move.broth -text \"Next\" -command {set choice 4} -underline 0");
cmd(inter, "button .b.move.hypern -text \"Next Type\" -command {set choice 5} -underline 5");
cmd(inter, "button .b.move.last -text \"Last\" -command {set choice 14} -underline 0");
cmd(inter, "button .b.move.down -text \"Down\" -command {set choice 6} -underline 0");
cmd(inter, "button .b.move.prev -text \"Prev.\" -command {set choice 12} -underline 0");
cmd(inter, "button .b.move.call -text \"Caller\" -command {set choice 9} -underline 0");
cmd(inter, "button .b.move.hook -text \"Hook\" -command {set choice 21} -underline 0");
cmd(inter, "button .b.move.search -text \"Search for\" -command {set choice 10} -underline 7");

cmd(inter, "pack .b.move.up .b.move.broth .b.move.hypern .b.move.last .b.move.down .b.move.prev .b.move.call .b.move.hook .b.move.search -side left");
cmd(inter, "pack .b.act .b.move -side top");


if(lab!=NULL)
  cmd(inter, "pack .v .b -side top");
else
  cmd(inter, "pack .b -side top");

cmd(inter, "bind . <KeyPress-s> {.b.act.ok invoke}");
cmd(inter, "bind . <KeyPress-r> {.b.act.run invoke}");
cmd(inter, "bind . <KeyPress-q> {.b.act.exit invoke}");
cmd(inter, "bind . <KeyPress-u> {.b.move.up invoke}");
cmd(inter, "bind . <Up> {.b.move.up invoke}");
cmd(inter, "bind . <KeyPress-n> {.b.move.broth invoke}");
cmd(inter, "bind . <Right> {.b.move.broth invoke}");
cmd(inter, "bind . <KeyPress-t> {.b.move.hypern invoke}");
cmd(inter, "bind . <KeyPress-l> {.b.move.last invoke}");
cmd(inter, "bind . <KeyPress-d> {.b.move.down invoke}");
cmd(inter, "bind . <Down> {.b.move.down invoke}");
cmd(inter, "bind . <KeyPress-c> {.b.move.call invoke}");
cmd(inter, "bind . <KeyPress-f> {.b.move.search invoke}");
cmd(inter, "bind . <KeyPress-p> {.b.move.prev invoke}");
cmd(inter, "bind . <Left> {.b.move.prev invoke}");
cmd(inter, "bind . <KeyPress-a> {.b.act.an invoke}");
cmd(inter, "bind . <KeyPress-i> {.b.act.until invoke}");
cmd(inter, "bind . <KeyPress-h> {set choice 21}");

deb_show(r, inter);

cmd(inter, "bind . <Destroy> {set choice 35}");


cmd(inter, "menu .m -tearoff 0 -relief groove -bd 2");
cmd(inter, "set w .m.help");
cmd(inter, "menu $w -tearoff 0 -relief groove -bd 2");
cmd(inter, ".m add cascade -label Help -menu $w -underline 0");
cmd(inter, "$w add command -label \"Lsd Debugger Help\" -command {LsdHelp debug.html}");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Model Report\" -command {set choice 44}");
cmd(inter, "focus -force .");
if(interact_flag==1)
 {
 cmd(inter, ".v.val selection range 0 end");
 cmd(inter, "focus -force .v.val");
 cmd(inter, "bind .v.val <Return> {.b.act.run invoke}");
 }
ch[0]=(char)NULL;
attach_instance_number(ch, r);
if(lab!=NULL)
 cmd(inter, "bind . <KeyPress-g> {set choice 77}");
if(asl!=NULL && asl->vs->up!=r)
  asl=NULL;

set_window_size();
//bah=Tcl_GetStringResult(inter);

    
    
while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "bind . <KeyPress-g> {}");
if(lab!=NULL)
{
 i=choice;
 cmd(inter, "set choice $stack_flag");
 stackinfo_flag=choice;
 choice=i;
} 
if(choice==35)
 myexit(0);
//cmd(inter, "wm resizable . 1 1");
 cmd(inter, "bind . <Destroy> {}");

cmd(inter, "bind . <KeyPress-s> {}");
cmd(inter, "bind . <KeyPress-r> {}");
cmd(inter, "bind . <KeyPress-q> {}");
cmd(inter, "bind . <KeyPress-u> {}");
cmd(inter, "bind . <KeyPress-n> {}");
cmd(inter, "bind . <KeyPress-t> {}");
cmd(inter, "bind . <KeyPress-d> {}");
cmd(inter, "bind . <KeyPress-c> {}");
cmd(inter, "bind . <KeyPress-f> {}");
cmd(inter, "bind . <KeyPress-p> {}");
cmd(inter, "bind . <KeyPress-a> {}");
cmd(inter, "bind . <KeyPress-l> {}");
cmd(inter, "bind . <KeyPress-i> {}");
cmd(inter, "bind . <KeyPress-h> {}");
cmd(inter, "bind . <Up> {}");
cmd(inter, "bind . <Down> {}");
cmd(inter, "bind . <Left> {}");
cmd(inter, "bind . <Right> {}");

if(lab!=NULL)
  cmd(inter, "destroy .m .v .b .cc .t .tit");
else
  cmd(inter, "destroy .m .b .cc .t .tit");

Tcl_UnlinkVar(inter, "value");
*res=app_res;
cmd(inter, "unset value");

switch(choice)
{
case 1:
cmd(inter, "set a [winfo exists .a]");
cmd(inter, "if { $a==1} {destroy .a} {}");


break;
case 2:
cmd(inter, "set a [winfo exists .a]");
cmd(inter, "if { $a==1} {destroy .a} {}");
debug_flag=0;
if(t==when_debug)
 when_debug=0;
		  cmd(inter, "wm iconify .");
		  break;
case 3: if( r->up!=NULL)
			 choice=deb(r->up,c, lab, res);
		  else
			 choice=0;
		  break;

case 77: 
        if(asl==NULL && stacklog->vs!=NULL)
            {
            asl=stacklog;
            sprintf(msg, "\nVariable: %s", asl->label);
            plog(msg);
        			 choice=deb(asl->vs->up,c, lab, res);
            }
          else
           {
            if(asl->next==NULL)
             {
             while(asl->prev->prev!=NULL)
              asl=asl->prev;
             sprintf(msg, "\nVariable: %s", asl->label);
             plog(msg);
         			 choice=deb(asl->vs->up,c, lab, res);
             }
             else
              {
               asl=asl->next;
               sprintf(msg, "\nVariable: %s", asl->label);
               plog(msg);
               choice=deb(asl->vs->up,c, lab, res);
              }
           }  
         break;  

case 4: if( r->next!=NULL)
			 choice=deb(r->next,c, lab, res);
		  else
			 {
        cur=skip_next_obj(r);
        if(cur!=NULL)
         choice=deb(cur,c, lab, res);
        else 
         choice=0;
        } 

			break;
case 21: if( r->hook!=NULL)
			 choice=deb(r->hook,c, lab, res);
		  else
			 choice=0;

			break;

case 12:if(r->up==NULL)
          {choice=0;
           break;
          }
        for(cb1=NULL, cb=r->up->b; strcmp(r->label, cb->blabel);cb1=cb, cb=cb->next);
        cur=cb->head;
        if(cur==r)
          {if(cb1!=NULL)
            {
             for(cur=cb1->head; cur->next!=NULL; cur=cur->next);
             choice=deb(cur,c, lab, res);
             break;
            }
           else 
            choice=0;
           break;
          }
          
        for(; cur->next!=r; cur=cur->next);
		    choice=deb(cur,c, lab, res);
        break;
case 5:
		  cur=skip_next_obj(r, &count);
		  if( cur!=NULL)
			 choice=deb(cur,c, lab, res);
		  else
			 choice=0;

		  break;
case 14: //go last

for(cur=r; cur!=NULL; cur=cur->next )
  cur1=cur;
choice=deb(cur1,c, lab, res);
break;


case 6:
		  if( r->b!=NULL && r->b->head!=NULL)
			 choice=deb(r->b->head,c, lab, res);
		  else
			 choice=0;
		  break;
case 9:
		  if( c!=NULL)
			 choice=deb(c,r, lab, res);
		  else
			 choice=0;
		  break;
case 10:
Tcl_LinkVar(inter, "value_search", (char *) &value_search, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "condition", (char *) &cond, TCL_LINK_INT);
cond=1;
choice=0;
i=1;
cmd(inter, "label .l1 -text \"Search for Obj. containing Var.\"");
//cmd(inter, "set en \"\"");
cmd(inter, "entry .e1 -width 10 -relief sunken -textvariable en");
cmd(inter, "label .l2 -text \"with value\"");
cmd(inter, "entry .e2 -width 10 -relief sunken -textvariable value_search");
cmd(inter, "button .ok -text Ok -command {set choice 1}");
cmd(inter, "button .esc -text Cancel -command {set choice 2}");

cmd(inter, "frame .cond");
cmd(inter, "radiobutton .cond.eq -text \" = \" -variable condition -value 1");
cmd(inter, "radiobutton .cond.geq -text \" >= \" -variable condition -value 2");
cmd(inter, "radiobutton .cond.leq -text \" <= \" -variable condition -value 3");
cmd(inter, "radiobutton .cond.gr -text \" > \" -variable condition -value 4");
cmd(inter, "radiobutton .cond.le -text \" < \" -variable condition -value 5");
cmd(inter, "radiobutton .cond.not -text \" != \" -variable condition -value 6");
cmd(inter, "pack .cond.eq .cond.geq .cond.leq .cond.gr .cond.le .cond.not -anchor w");
cmd(inter, "pack .l1 .e1 .l2 .e2 .cond .ok .esc");
cmd(inter, ".e2 selection range 0 end");

cmd(inter, "bind . <KeyPress-Return> {}");
cmd(inter, "bind . <KeyPress-Escape> {.esc invoke}");
cmd(inter, "bind .e1 <KeyPress-Return> {focus .e2}");
cmd(inter, "bind .e2 <KeyPress-Return> {focus .ok}");
cmd(inter, "bind .ok <KeyPress-Return> {.ok invoke}");

cmd(inter, "focus .e1");

cmd(inter, "bind . <Destroy> {set choice 35}");
set_window_size();
while(choice==0)
 Tcl_DoOneEvent(0);

if(choice==35)
 myexit(0);
cmd(inter, "bind . <Destroy> {}"); 
if(choice==2)
 {
quit=0;
cmd(inter, "destroy .l1 .e1 .l2 .e2 .cond .ok .esc");
Tcl_UnlinkVar(inter, "value_search");
Tcl_UnlinkVar(inter, "condition");
choice=0;
break;

 }
pre_running=running;
running=0;

ch1=(char *)Tcl_GetVar(inter, "en",0);
strcpy(ch, ch1);
switch(cond)
{
case 1:
//cur=r->search_var_cond(ch, value_search, 0); //BUG: triggers the computation of variables, which in general is not a good idea.
i=0;
cur2=NULL;
for(cur1=r; cur1!=NULL && i==0; cur1=cur1->up)
{

cv=cur1->search_var(r, ch);
if(cv==NULL)
 break;
 

cur=(object *)cv->up;

for( ;cur!=NULL && i==0; cur=cur->hyper_next(cur->label))
 {app_res=cur->search_var(cur,ch)->val[0];
  if(app_res==value_search)
    {cur2=cur;
     i=1;
    } 
     
 }
}
cur=cur2;
break;

case 2:
cv=r->search_var(r, ch);
cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)<value_search; )
 cur=cur->hyper_next(cur->label);
break;

case 3:
cv=r->search_var(r, ch);
cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)>value_search; )
 cur=cur->hyper_next(cur->label);
break;

case 4:
cv=r->search_var(r, ch);
cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)<=value_search; )
 cur=cur->hyper_next(cur->label);
break;

case 5:
cv=r->search_var(r, ch);
cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)>=value_search; )
 cur=cur->hyper_next(cur->label);
break;

case 6:
cv=r->search_var(r, ch);
cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)==value_search; )
 cur=cur->hyper_next(cur->label);
break;

default:
cur=NULL;
}

quit=0; //If var is mispelled don't stop the simulation!
cmd(inter, "destroy .l1 .e1 .l2 .e2 .cond .ok .esc");
Tcl_UnlinkVar(inter, "value_search");
Tcl_UnlinkVar(inter, "condition");

if(cur!=NULL)
 choice=deb(cur, r, lab, res);
else
 choice=0;

running=pre_running;
break;




case 7:

if(c!=NULL)
 {
 cmd(inter, "set choice [tk_dialog .w Quit? \"You are quitting the simulation run. Press 'Yes' to confirm.\" \"\" 1 Yes No]");
 //Tcl_UnlinkVar(inter, "choice");
 } 
else
 choice=0; 
if(choice==1) 
 {
 choice=deb(r,c, lab, res);
 break;
 }
cmd(inter, "set a [winfo exists .a]");
cmd(inter, "if { $a==1} {destroy .a} {}");
   choice=1;
     quit=1;
		 debug_flag=0;
		 break;
case 29:
		ch1=(char *)Tcl_GetVar(inter, "res",0);
		strcpy(ch, ch1);
//		show_eq(ch, &choice);
     choice=0;
     set_all(&choice, r, ch, 0);
     //Tcl_UnlinkVar(inter, "choice");
		choice=0;
		break;
case 8:
ch1=(char *)Tcl_GetVar(inter, "res",0);
Tcl_LinkVar(inter, "debug", (char *) &count, TCL_LINK_INT);
Tcl_LinkVar(inter, "time", (char *) &t, TCL_LINK_INT);


choice=0;
cv=r->search_var(NULL, ch1);
i=cv->last_update;
count=(cv->debug=='d'?1:0);
cmd(inter,"button .ok -text Done -command {set choice 1}");
cmd(inter, "button .cond -text \"Set Conditional Break\" -command {set choice 7}");
cmd(inter, "button .eq -text Equation -command {set choice 8}");
cmd(inter, "button .help -text Help -command {LsdHelp debug.html#content}");
if(cv->param==0 || cv->param==2)
 cmd(inter, "button .exec -text \"Execute\" -command {set choice 9}");
cmd(inter, "frame .past");
cmd(inter, "frame .past.tit");
cmd(inter, "label .past.tit.lab -text \"Values of: $res\"");
cmd(inter, "label .past.tit.time -text \"Current time: $time\"");
if(cv->param==0)
  {sprintf(msg, "label .past.tit.lu -text \"Time of last computation: %d\"",cv->last_update);
   cmd(inter, msg);
  }
else
  cmd(inter, "label .past.tit.lu -text \"Parameter\"");
cmd(inter, "pack .past.tit.lab .past.tit.time .past.tit.lu");
cmd(inter, "pack .past.tit");

Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);
app_values=new double[cv->num_lag+1];
for(i=0; i<cv->num_lag+1; i++)
 {
  sprintf(msg, "set val%d %g",i, cv->val[i]);

  cmd(inter, msg);
  app_values[i]=cv->val[i];
  sprintf(ch, "val%d",i);
  Tcl_LinkVar(inter, ch, (char *) &(app_values[i]), TCL_LINK_DOUBLE);
  cmd(inter, "frame .past.l$i");

  if(cv->param==0)
    cmd(inter, "label .past.l$i.n$i -text \"Lag $i: \"");
  else
    cmd(inter, "label .past.l$i.n$i -text \"Value: \"");
  cmd(inter, "entry .past.l$i.e$i -width 10 -textvariable val$i");
  
  sprintf(msg, "button .past.l$i.sa -text \"Set All\" -command {set sa %i; set choice 10}", i);
  cmd(inter, msg);
  cmd(inter, "pack .past.l$i.n$i .past.l$i.e$i .past.l$i.sa -side left");
  cmd(inter, "pack .past.l$i");
 }

cmd(inter, "checkbutton .deb -text \"Debug \" -variable debug");
cmd(inter, "checkbutton .deball -text \"Apply debug option to all copies\" -variable debugall");

if(cv->param==0 || cv->param==2)
 cmd(inter, "pack .past .deb .deball .ok .eq .exec .cond .help");
else
 cmd(inter, "pack .past .deb .ok .eq .cond .help"); 
cmd(inter, ".past.l0.e0 selection range 0 end");
cmd(inter, "focus -force .past.l0.e0");
cmd(inter, "bind .past.l0.e0 <Return> {.ok invoke}");

set_window_size();
while(choice==0)
 Tcl_DoOneEvent(0);
cv->data_loaded='+';
for(i=0; i<cv->num_lag+1; i++)
 {cv->val[i]=app_values[i];
  sprintf(ch, "val%d",i);

  Tcl_UnlinkVar(inter, ch);
  cmd(inter, "unset val$i");
 }

delete[] app_values;
Tcl_UnlinkVar(inter, "i");

cv->debug=(count==1?'d':'n');
if(cv->param==0 || cv->param==2)
 cmd(inter, "destroy .past .deb .deball .ok .eq .exec .cond .help");
else
 cmd(inter, "destroy .past .deb .deball .ok .eq .cond .help"); 

Tcl_UnlinkVar(inter, "debug");
count=choice;
cmd(inter, "set choice $debugall");
if(choice==1)
 {
  for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label) )
   {
    cv1=cur->search_var(cur,cv->label);
    cv1->debug=cv->debug;
   }
 
 }
choice=count;
if(choice==7)
  set_cond(cv);

if(choice==8)
 show_eq(cv->label, &choice);

if(choice==9)
 cv->cal(cv->up, 0);
if(choice==10)
 {

 ch1=(char *)Tcl_GetVar(inter, "res",0);
 strcpy(ch, ch1);
 
 cmd(inter, "set choice $sa");
 i=choice;
 choice=0;
 set_all(&choice, r, ch, i);
 
 } 
//Tcl_UnlinkVar(inter, "choice");

choice=0;
break;

//Analysis
case 11:
cmd(inter, "set a [winfo exists .a]");
cmd(inter, "if { $a==1} {destroy .a} {}");

actual_steps=t;
for(cur=r; cur->up!=NULL; cur=cur->up);
reset_end(cur);
Tcl_LinkVar(inter, "choice", (char *) &choice, TCL_LINK_INT);
analysis(&choice);
cmd(inter, "bind . <Destroy> {}");
//Tcl_UnlinkVar(inter, "choice");
choice=0;
break;

//Print stack
case 13:
print_stack();
cmd(inter, "raise .log");
choice=0;
break;

case 15:
cmd(inter, "set a [winfo exists .a]");
cmd(inter, "if { $a==1} {destroy .a} {}");
//cmd(inter, "toplevel .a; text .a.t -width 20 -yscrollcommand \".a.yscroll set\" -wrap word; scrollbar .a.yscroll -command \".a.t yview\"; button .a.c -text Close -command {destroy .a}; pack .a.yscroll -side right -fill y; pack .a.t -expand yes -fill both; pack .a.c; wm geom .a -0+0; wm title .a \"Equation Intermediate Values\"; raise .a");

cmd(inter, "toplevel .a; text .a.t -width 20 -yscrollcommand \".a.yscroll set\" -wrap word; scrollbar .a.yscroll -command \".a.t yview\"; button .a.c -text Close -command {destroy .a}; pack .a.yscroll -side right -fill y; pack .a.t -expand yes -fill both; pack .a.c; wm title .a \"Equation Intermediate Values\"; wm transient .a .; raise .a");

cmd(inter, ".a.t tag configure v -foreground red");
//cmd(inter, "align .a .");
for(i=0; i<100; i++)
 {
  sprintf(msg, ".a.t insert end \"v\\\[%d\\]\" v",i);
  cmd(inter, msg);
  sprintf(msg, ".a.t insert end \"=%g\n\"",i_values[i]);
  cmd(inter, msg);

 }
choice=0;
break;

case 16:
//run until
choice=0;


cmd(inter, "wm title . \"Time Debugger\"");
cmd(inter, "label .l -text \"Insert time of activation Debug Mode\"");
sprintf(msg, "set when_debug %d",t+1);
cmd(inter, msg);
cmd(inter, "entry .val -width 10 -relief sunken -textvariable when_debug");
cmd(inter, ".val selection range 0 end");
cmd(inter, "button .ok -text Ok -command {set choice 1}");
cmd(inter, "button .help -text Help -command {LsdHelp debug.html#until}");
cmd(inter, "pack .l .val .ok .help");
cmd(inter, "bind . <KeyPress-Return> {set choice 1}");

cmd(inter, ".val selection range 0 end");
cmd(inter, "focus .val");

set_window_size();
while(choice==0)
 Tcl_DoOneEvent(0);


cmd(inter, "destroy .l .val .ok .help");


choice=0;
break;

case 17:
//change the object number
choice==0;
if(r->up!=NULL)
 entry_new_objnum(r, &choice, "pippo");


choice=0;
break;


case 44:
//see model report

sprintf(msg, "set name_rep %s", name_rep);
cmd(inter, msg);

cmd(inter, "set choice [file exists $name_rep]");

cmd(inter, "if {$choice == 1} {LsdHtml $name_rep} {}");
cmd(inter, "if {$choice == 0} {tk_messageBox -type ok -title \"No report\" -message \"Report file not available.\\nYou can create the report in menu Model.\"} {}");
choice=0;
break;



default:
plog("\nChoice not recognized\n");
choice=0;
break;
}
}
return(choice);
}


/*******************************************
DEB_SHOW
********************************************/
void deb_show(object *r, Tcl_Interp *inter)
{
char ch[1000];
variable *ap_v;
int count, i;
object *ap_o;


cmd(inter, "frame .t -relief groove -bd 2");


cmd(inter, "label .t.obj -relief raise -bd 2 -text \"Object instance:\" ");
if(r->up!=NULL)
{
 cmd(inter, "bind .t.obj <Button-1> {if {[winfo exist .w]==1} {destroy .w} {}; set choice 17}");
// cmd(inter, "bind .t.obj <Enter> {set mx %X; set my %Y; toplevel .w; wm geometry .w +$mx+$my; wm title .w \"\"; label .w.l -text \"Click here to change\\nObj. number\"; pack .w.l}");
// cmd(inter, "bind .t.obj <Leave> {destroy .w}");
} 

/*
strcpy(ch, "label .t.obj -text \"Object Name (ascendants and instance numbers):\" ");

strcat(ch, r->label);
this_instance_number(r);
strcat(ch, msg);
strcat(ch, "\"");
cmd(inter, ch);
**/
strcpy(ch, "");

attach_instance_number(ch, r);
sprintf(msg, "label .t.instance -text \"%s\" -justify left", ch);
cmd(inter, msg);
cmd(inter, "pack .t.obj .t.instance -side left -anchor s");

cmd(inter, "pack .t -side top");	// fix this frame in the center before proceeding

// adjust spacing to align labels with data and increase columns width to better fill window
cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set w1 26; set w2 10; set w3 20} {set w1 19; set w2 9; set w3 15}");
cmd(inter, "frame .tit");
cmd(inter, "frame .tit.h1");
cmd(inter, "label .tit.h1.name -text Variable -width $w1 -pady 0 -bd 0 -anchor w");
cmd(inter, "label .tit.h1.last -text LastUpdate -width $w2 -pady 0 -bd 0 -anchor w -foreground red");
cmd(inter, "label .tit.h1.val -text Value -width $w3 -pady 0 -bd 0 -anchor w");
cmd(inter, "pack .tit.h1.name .tit.h1.val .tit.h1.last -side left");

cmd(inter, "frame .tit.h2");
cmd(inter, "label .tit.h2.name -text Variable -width $w1 -pady 0 -bd 0 -anchor w");
cmd(inter, "label .tit.h2.last -text LastUpdate -width $w2 -pady 0 -bd 0 -anchor w -foreground red");
cmd(inter, "label .tit.h2.val -text Value -width $w3 -pady 0 -bd 0 -anchor w");
cmd(inter, "pack .tit.h2.name .tit.h2.val .tit.h2.last -side left -anchor w");

cmd(inter, "pack .tit.h1 .tit.h2 -side left");
cmd(inter, "pack .tit -side top -anchor w");	// align this frame to the left

cmd(inter, "frame .cc -relief groove -bd 2");
cmd(inter, "scrollbar .cc.scroll -command \".cc.l yview\"");                                    //before 100
cmd(inter, "text .cc.l -yscrollcommand \".cc.scroll set\" -wrap word -width 100 -height 200 -cursor arrow");



if(r->v==NULL)
  {cmd(inter, "label .cc.l.no_var -text \"(No variables defined here)\"");
	cmd(inter,".cc.l window create end -window .cc.l.no_var");
  }
else
  {Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);

  for(i=1,ap_v=r->v; ap_v!=NULL; ap_v=ap_v->next, i++)
	 {
     sprintf(msg, "set last %d", ap_v->last_update);
     cmd(inter, msg);

//    Tcl_LinkVar(inter, "last", (char *) &ap_v->last_update, TCL_LINK_INT);
//	 Tcl_LinkVar(inter, "val", (char *) &ap_v->val[0], TCL_LINK_DOUBLE);
     sprintf(msg, "set val %g", ap_v->val[0]);
     cmd(inter, msg);
	  cmd(inter, "frame .cc.l.e$i");
	  strcpy(ch, "label .cc.l.e$i.name -width $w1 -pady 0 -anchor w -bd 0 -text ");
	  strcat(ch, ap_v->label);
	  cmd(inter, ch);
	  if(ap_v->param==0)
		 cmd(inter, "label .cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text $last -foreground red");
	  if(ap_v->param==1)
		 cmd(inter, "label .cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text Par -foreground red");
	  if(ap_v->param==2)
		 cmd(inter, "label .cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text Fun -foreground red");
	  cmd(inter, "label .cc.l.e$i.val -width $w3 -pady 0 -bd 0 -anchor w -text $val");

	  cmd(inter, "pack .cc.l.e$i.name .cc.l.e$i.val .cc.l.e$i.last -side left");
//	  if(ap_v->param==0)
		{strcpy(ch, "bind .cc.l.e$i.name <Double-Button-1> {set res ");
		 strcat(ch, ap_v->label);
		 strcat(ch, "; set choice 8}");
		 cmd(inter, ch);

		 strcpy(ch, "bind .cc.l.e$i.name <Button-3> {set res ");
		 strcat(ch, ap_v->label);
		 strcat(ch, "; set choice 29}");
		 cmd(inter, ch);
		}
	  cmd(inter,".cc.l window create end -window .cc.l.e$i");
     if( (i%2)==0)
       cmd(inter, ".cc.l insert end \\n");
	  Tcl_UnlinkVar(inter, "last");
//	  Tcl_UnlinkVar(inter, "val");
	 }
	Tcl_UnlinkVar(inter, "i");

  }

cmd(inter, "pack .cc.scroll -side right -fill y");
cmd(inter, ".cc.l conf -height 20");
cmd(inter, "pack .cc.l -expand 1 -fill y -fill x");
cmd(inter, "pack .cc -expand 1  -fill x -fill y");


}



/*******************************************
SET_COND
********************************************/
void set_cond(variable *cv)
{
int old;
char ch[120];
Tcl_LinkVar(inter, "cond", (char *) &cv->deb_cond, TCL_LINK_INT);
Tcl_LinkVar(inter, "cond_val", (char *) &cv->deb_cnd_val, TCL_LINK_DOUBLE);

choice=0;
while(choice==0)
{
switch(cv->deb_cond)
 {case 0: cmd(inter, "label .cnd_type -text \"No Conditional Break\"");
          break;
  case 1: cmd(inter, "label .cnd_type -text \"Condition: = \"");
          break;
  case 2: cmd(inter, "label .cnd_type -text \"Condition: < \"");
          break;
  case 3: cmd(inter, "label .cnd_type -text \"Condition: > \"");
          break;
  default: printf("\nError S01: debug condition for var %s set to %d",cv->label, cv->deb_cond);
			 exit(1);
          break;
 }
old=cv->deb_cond;
strcpy(ch, "label .name -text \"Conditional stop for Variable: ");
strcat(ch, cv->label);
strcat(ch, "\"");
cmd(inter, ch);       
cmd(inter, "entry .cond -relief sunken -textvariable cond_val");
cmd(inter, "frame .c");
cmd(inter, "button .c.eq -text \" = \" -command {set cond 1}");
cmd(inter, "button .c.min -text \" < \" -command {set cond 2}");
cmd(inter, "button .c.max -text \" > \" -command {set cond 3}");
cmd(inter, "button .no -text \"No Condition\" -command {set cond 0}");

cmd(inter, "button .done -text Done -command {set choice 1}");
cmd(inter, "pack .c.min .c.eq .c.max -side left");
cmd(inter, "pack .name .cnd_type .c .cond .no .done -side top");

set_window_size();
while(choice==0 && cv->deb_cond==old)
 Tcl_DoOneEvent(0);


cmd(inter, "destroy .cnd_type .done .c .cond .no .name");
}

Tcl_UnlinkVar(inter, "cond");
Tcl_UnlinkVar(inter, "cond_val");

choice=0;

}

void attach_instance_number(char *ch, object *r)
{
object *cur;
int i, j;

if(r==NULL)
 return;
attach_instance_number(ch, r->up);

if(r->up!=NULL)
 for(i=1,cur=r->up->search(r->label); cur!=NULL; cur=go_brother(cur) )
   {
   if(cur==r)
    j=i;
   i++;
   }

if(r->up==NULL)
   sprintf(msg, "%s 1/1", r->label);
else
   sprintf(msg, "\\n%s %d/%d", r->label, j, i-1);
strcat(ch, msg);
}

void this_instance_number(object *r)
{

object *cur;
int i, j;

if(r->up==NULL)
 {
 sprintf(msg, "");
 return;
 }

for(i=1,cur=r->up->search(r->label); cur!=NULL; cur=go_brother(cur) )
   {
   if(cur==r)
    j=i;
   i++;
   }
sprintf(msg, " (%d/%d)", j, i-1);

}

double object::interact(char const *text, double v, double *tv)
{
/*
Interrupt the simulation, as for the debugger, allowing the insertion of a value.
Note that the debugging window, in this model, accept the entry key stroke as a run.
*/
int i;

double app;
if(quit!=0)
 return v;
app=v;
interact_flag=1;
for(i=0; i<100; i++)
 i_values[i]=tv[i];
deb(this, NULL, text, &app);
interact_flag=0;
return app;
}
