/***************************************************
****************************************************
LSD 7.0 - August 2015
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
extern char *simul_name;	// simulation name to use in title bar
extern bool unsavedChange;	// control for unsaved changes in configuration


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
void error_hard( const char *logText, const char *boxTitle, const char *boxText = "" );

lsdstack *asl=NULL;
bool invalidHooks = false;		// flag to invalid hooks pointers (set by simulation)

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

Tcl_SetVar( inter, "lab", lab, 0 );
cmd( inter, "set deb .deb" );
sprintf( msg, "if { ! [ winfo exists .deb ] } { if [ string equal $lab \"\" ] { newtop .deb \"%s%s - Lsd Data Browser\" { set choice 7 } } { newtop .deb \"%s%s - Lsd Debugger\" { set choice 7 } \"\" } }", unsavedChange ? "*" : "", simul_name, unsavedChange ? "*" : "", simul_name );
cmd( inter, msg );

// avoid redrawing the menu if it already exists and is configured
cmd(inter, "set existMenu [ winfo exists .deb.m ]");
cmd(inter, "set confMenu [ .deb cget -menu ]");
if ( ! strcmp( Tcl_GetVar( inter, "existMenu", 0 ), "0" ) ||
	 strcmp( Tcl_GetVar( inter, "confMenu", 0 ), ".deb.m" ) )
{
	cmd( inter, "destroy .deb.m" );
	cmd(inter, "menu .deb.m -tearoff 0 -relief groove -bd 2");
	cmd(inter, "set w .deb.m.exit");
	cmd(inter, ".deb.m add cascade -label Exit -menu $w -underline 0");
	cmd(inter, "menu $w -tearoff 0 -relief groove -bd 2");
	cmd(inter, "$w add command -label \"Quit and return to Browser\" -command { set choice 7 } -underline 0 -accelerator Esc");
	cmd(inter, "set w .deb.m.help");
	cmd(inter, "menu $w -tearoff 0 -relief groove -bd 2");
	cmd(inter, ".deb.m add cascade -label Help -menu $w -underline 0");
	cmd(inter, "$w add command -label \"Help on Lsd Debugger\" -command {LsdHelp debug.html} -underline 0");
	cmd(inter, "$w add separator");
	cmd(inter, "$w add command -label \"Model Report\" -command {set choice 44} -underline 0");
	cmd( inter, "$w add separator" );
	sprintf( msg, "$w add command -label \"About Lsd...\" -command { tk_messageBox -type ok -icon info -title \"About Lsd\" -message \"Version %s (%s)\n\nPlatform: [ string totitle $tcl_platform(platform) ] ($tcl_platform(machine))\nOS: $tcl_platform(os) ($tcl_platform(osVersion))\nTcl/Tk: [ info patch ]\" } -underline 0", _LSD_VERSION_, _LSD_DATE_ ); 
	cmd( inter, msg );
	cmd(inter, ".deb configure -menu .deb.m");
}

// avoid redrawing the buttons if they already exist
cmd( inter, "set existButtons [ expr [ winfo exists .deb.b ] ]" );
if ( ! strcmp( Tcl_GetVar( inter, "existButtons", 0 ), "0" ) )
{ 
	cmd( inter, "destroy .deb.b" );

	cmd(inter, "frame .deb.b -border 6");
	cmd(inter, "frame .deb.b.move");
	cmd(inter, "frame .deb.b.act");

	cmd(inter, "button .deb.b.move.up -width -9 -text \"Up\" -command {set choice 3} -underline 0");
	cmd(inter, "button .deb.b.move.down -width -9 -text \"Down\" -command {set choice 6} -underline 0");
	cmd(inter, "button .deb.b.move.prev -width -9 -text \"Prev.\" -command {set choice 12} -underline 0");
	cmd(inter, "button .deb.b.move.broth -width -9 -text \"Next\" -command {set choice 4} -underline 0");
	cmd(inter, "button .deb.b.move.hypern -width -9 -text \"Next Type\" -command {set choice 5} -underline 5");
	cmd(inter, "button .deb.b.move.last -width -9 -text \"Last\" -command {set choice 14} -underline 0");
	cmd(inter, "button .deb.b.move.search -width -9 -text \"Search\" -command {set choice 10} -underline 0");
	cmd(inter, "pack .deb.b.move.up .deb.b.move.down .deb.b.move.prev .deb.b.move.broth .deb.b.move.hypern .deb.b.move.last .deb.b.move.search -padx 10 -pady 10 -side left");

	cmd(inter, "bind .deb <KeyPress-u> {.deb.b.move.up invoke}");
	cmd(inter, "bind .deb <Up> {.deb.b.move.up invoke}");
	cmd(inter, "bind .deb <KeyPress-n> {.deb.b.move.broth invoke}");
	cmd(inter, "bind .deb <Right> {.deb.b.move.broth invoke}");
	cmd(inter, "bind .deb <KeyPress-t> {.deb.b.move.hypern invoke}");
	cmd(inter, "bind .deb <KeyPress-l> {.deb.b.move.last invoke}");
	cmd(inter, "bind .deb <KeyPress-d> {.deb.b.move.down invoke}");
	cmd(inter, "bind .deb <Down> {.deb.b.move.down invoke}");
	cmd(inter, "bind .deb <KeyPress-f> {.deb.b.move.search invoke}");
	cmd(inter, "bind .deb <KeyPress-p> {.deb.b.move.prev invoke}");
	cmd(inter, "bind .deb <Left> {.deb.b.move.prev invoke}");
	cmd(inter, "bind .deb <KeyPress-Escape> {set choice 7}");
	
	if(lab!=NULL)
	{
		sprintf(msg, "set stack_flag %d",stackinfo_flag);
		cmd(inter, msg);

		cmd(inter, "button .deb.b.act.run -width -9 -text Run -command {set choice 2} -underline 0");
		cmd(inter, "button .deb.b.act.until -width -9 -text Until -command {set choice 16} -underline 3");
		cmd(inter, "button .deb.b.act.ok -width -9 -text Step -command {set choice 1; set done_in 3} -underline 0");
		cmd(inter, "button .deb.b.act.an -width -9 -text Analysis -command {set choice 11} -underline 0");
		cmd(inter, "button .deb.b.act.net -width -9 -text Network -command {set choice 22} -underline 3");
		cmd(inter, "button .deb.b.act.call -width -9 -text Caller -command {set choice 9} -underline 0");
		cmd(inter, "button .deb.b.act.hook -width -9 -text Hook -command {set choice 21} -underline 0");
		cmd(inter, "button .deb.b.act.prn_v -width -9 -text \"v\\\[...\\]\" -command {set choice 15}");
		cmd(inter, "button .deb.b.act.prn_stck -width -9 -text \"Print Stack\" -command {set choice 13}");
		cmd(inter, "frame .deb.b.act.stack");
		cmd(inter, "label .deb.b.act.stack.l -text \"Print stack level: \"");
		cmd(inter, "entry .deb.b.act.stack.e -width 3 -relief sunken -textvariable stack_flag");
		cmd(inter, "pack .deb.b.act.stack.l .deb.b.act.stack.e -side left");
		//cmd(inter, "checkbutton .deb.b.act.stack -text \"Stack info\" -variable stack_flag");
		cmd(inter, "pack .deb.b.act.run .deb.b.act.until .deb.b.act.ok .deb.b.act.an .deb.b.act.net .deb.b.act.call .deb.b.act.hook .deb.b.act.prn_v .deb.b.act.prn_stck .deb.b.act.stack -padx 1 -pady 10 -side left");

		cmd(inter, "bind .deb <KeyPress-s> {.deb.b.act.ok invoke}");
		cmd(inter, "bind .deb <KeyPress-r> {.deb.b.act.run invoke}");
		cmd(inter, "bind .deb <KeyPress-q> {.deb.b.act.exit invoke}");
		cmd(inter, "bind .deb <KeyPress-a> {.deb.b.act.an invoke}");
		cmd(inter, "bind .deb <KeyPress-i> {.deb.b.act.until invoke}");
		cmd(inter, "bind .deb <KeyPress-c> {.deb.b.act.call invoke}");
		cmd(inter, "bind .deb <KeyPress-h> {set choice 21}");
		cmd(inter, "bind .deb <KeyPress-w> {set choice 22}");
	}

	cmd(inter, "pack .deb.b.move .deb.b.act -side top");
	cmd(inter, "pack .deb.b -side right");
}

cmd( inter, "if [ string equal $lab \"\" ] { showtop .deb topleftW } { showtop .deb topleftW 0 0 0 }" );

app_res=*res;
Tcl_LinkVar(inter, "value", (char *) &app_res, TCL_LINK_DOUBLE);

choice=0;

while (choice==0)
{

cmd( inter, "destroy .deb.v" );

if(lab!=NULL)
{
cmd(inter, "frame .deb.v -relief groove -bd 2");
if(interact_flag==0)
{
	cmd( inter, "label .deb.v.name1 -text \"Variable:\"" );
	sprintf( ch, "label .deb.v.name2 -fg red -text \"%s\"", lab );
}
else
{
	cmd( inter, "label .deb.v.name1 -text \"\"" );
	sprintf(ch,"label .deb.v.name2 -fg red -text \"%s\"",lab);
}

cmd(inter, ch);
//cmd(inter, "bind .deb.v.name2 <Double-Button-1> {set choice 10");
Tcl_LinkVar(inter, "time", (char *) &t, TCL_LINK_INT);
cmd(inter, "label .deb.v.time -text \"      Time step: $time      \"");
Tcl_UnlinkVar(inter, "time");
cmd(inter, "label .deb.v.val1 -text \"Value: \"");
cmd(inter, "entry .deb.v.val2 -width 10 -relief sunken -textvariable value");

cmd(inter, "pack .deb.v.name1 .deb.v.name2 .deb.v.time .deb.v.val1 .deb.v.val2 -side left");
cmd(inter, "pack .deb.v -pady 2 -before .deb.b");
}

deb_show(r, inter);

cmd(inter, "focus -force .deb");
if(interact_flag==1)
 {
 cmd(inter, ".deb.v.val2 selection range 0 end");
 cmd(inter, "focus -force .deb.v.val2");
 cmd(inter, "bind .deb.v.val2 <Return> {.deb.b.act.run invoke}");
 }
ch[0]=(char)NULL;
attach_instance_number(ch, r);
if(lab!=NULL)
 cmd(inter, "bind .deb <KeyPress-g> {set choice 77}");
if(asl!=NULL && asl->vs->up!=r)
  asl=NULL;

debug_maincycle:
while(choice==0)
 {
 try{ 
 Tcl_DoOneEvent(0);
 }
 catch(...) {
 goto debug_maincycle;
 }
 }
cmd(inter, "bind .deb <KeyPress-g> {}");
if(lab!=NULL)
{
 i=choice;
 cmd(inter, "set choice $stack_flag");
 stackinfo_flag=choice;
 choice=i;
} 

cmd(inter, "unset value");

switch(choice)
{
case 1:
cmd(inter, "set a [winfo exists .intval]");
cmd(inter, "if { $a==1} {destroytop .intval} {}");
cmd( inter, "set a [winfo exists .net]" );
cmd( inter, "if { $a==1 } { destroytop .net }" );
break;

case 2:
cmd(inter, "set a [winfo exists .intval]");
cmd(inter, "if { $a==1} {destroytop .intval} {}");
cmd( inter, "set a [winfo exists .net]" );
cmd( inter, "if { $a==1 } { destroytop .net }" );
cmd( inter, "destroytop .deb" );
debug_flag=0;
if(t==when_debug)
 when_debug=0;
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
case 21: if(r->hook!=NULL)
		{
			if ( ! invalidHooks )
			{
				int lstUpd;
				// check if the hook contains a valid Lsd object pointer (not very effective, most likely will crash...)
				try { lstUpd = r->hook->lstCntUpd; }
				catch ( ... ) {	lstUpd = 0; }
				
				if ( lstUpd <= 0 || lstUpd > t )
				{
					cmd( inter, "tk_messageBox -type ok -icon error -title Error -message \"Invalid hook pointer.\n\nCheck if your code is using valid pointers to Lsd objects or avoid using this option. If non-standard hooks are used, consider adding the command 'invalidHooks = true' to your model code.\"");
					choice = 0;
					break;
				}
				
				choice=deb(r->hook,c, lab, res);
			}
			else
			{
				cmd( inter, "tk_messageBox -type ok -icon error -title Error -message \"This option is unavailable.\n\nYour code is using non-standard pointers('invalidHooks = true').\"");
				choice = 0;
				break;
			}
		}
		  else
			 choice=0;

			break;
			
//Show network node attributes
case 22:
	if(r->node == NULL)
		break;

	cmd(inter, "set a [winfo exists .intval]");
	cmd(inter, "if { $a==1} {destroytop .intval} {}");
	cmd( inter, "set a [winfo exists .net]" );
	cmd( inter, "if { $a==1 } { destroytop .net }" );

	cmd( inter, "set n .net");
	cmd( inter, "newtop $n \"Network\" { destroytop $n }");
	
	cmd(inter, "label $n.l1 -text \"Node id: \"");
	sprintf(msg, "label $n.l2 -foreground red -text \"%ld\"", r->node->id);
	cmd(inter, msg); 
	cmd(inter, "label $n.l3 -text \"Num. links out: \"");
	sprintf(msg, "label $n.l4 -foreground red -text \"%ld\"", r->node->nLinks);
	cmd(inter, msg); 
	cmd(inter, "label $n.l5 -text \"Outgoing links: \"");
	cmd(inter, "label $n.l6 -text \"dest. id          (weight)\" -foreground red");
	if(r->node->name != NULL)		// is node named?
	{
		cmd(inter, "label $n.l7 -text \"Node name: \"");
		sprintf(msg, "label $n.l8 -foreground red -text \"%s\"", r->node->name);
		cmd(inter, msg); 
		cmd(inter, "pack $n.l7 $n.l8 $n.l1 $n.l2 $n.l3 $n.l4 $n.l5");
	}
	else
		cmd(inter, "pack $n.l1 $n.l2 $n.l3 $n.l4 $n.l5");
	cmd(inter, "pack $n.l6 -anchor w");
	cmd(inter, "text $n.t -width 15 -yscrollcommand \"$n.yscroll set\" -wrap word");
	cmd(inter, "scrollbar $n.yscroll -command \"$n.t yview\"");
	cmd(inter, "pack $n.yscroll -side right -fill y");
	cmd(inter, "pack $n.t -expand yes -fill both");
	cmd( inter, "done $n c { destroytop $n }" ); 
	cmd(inter, "$n.t tag configure v -foreground red");
	
	for(netLink *curLnk=r->node->first; curLnk != NULL; curLnk=curLnk->next)
	{
		sprintf(msg, "$n.t insert end \"%ld\"", curLnk->ptrTo->node->id);
		cmd(inter, msg);
		if(curLnk->weight != 0)
			sprintf(msg, "$n.t insert end \"\t(%g)\n\"", curLnk->weight);
		else
			sprintf(msg, "$n.t insert end \"\n\"");
		cmd(inter, msg);
	}
	cmd( inter, "showtop $n topleftW 0 1 0");
	cmd( inter, "align $n .deb");
	
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
cmd( inter, "set s .sobj" );
cmd( inter, "newtop $s \"Search Object\" { set choice 2 }" );

cmd(inter, "label $s.l1 -text \"Search for obj. containing var.\"");
//cmd(inter, "set en \"\"");
cmd(inter, "entry $s.e1 -width 10 -relief sunken -textvariable en");
cmd(inter, "label $s.l2 -text \"with value\"");
cmd(inter, "entry $s.e2 -width 10 -relief sunken -textvariable value_search");

cmd(inter, "frame $s.cond");
cmd(inter, "radiobutton $s.cond.eq -text \" = \" -variable condition -value 1");
cmd(inter, "radiobutton $s.cond.geq -text \" >= \" -variable condition -value 2");
cmd(inter, "radiobutton $s.cond.leq -text \" <= \" -variable condition -value 3");
cmd(inter, "radiobutton $s.cond.gr -text \" > \" -variable condition -value 4");
cmd(inter, "radiobutton $s.cond.le -text \" < \" -variable condition -value 5");
cmd(inter, "radiobutton $s.cond.not -text \" != \" -variable condition -value 6");
cmd(inter, "pack $s.cond.eq $s.cond.geq $s.cond.leq $s.cond.gr $s.cond.le $s.cond.not -anchor w");
cmd(inter, "pack $s.l1 $s.e1 $s.l2 $s.e2 $s.cond");

cmd( inter, "okcancel $s b { set choice 1 } { set choice 2 }" );
cmd( inter, "showtop $s" );

cmd(inter, "bind $s.e1 <KeyPress-Return> {focus $s.e2}");
cmd(inter, "bind $s.e2 <KeyPress-Return> {focus $s.ok}");
cmd(inter, "$s.e2 selection range 0 end");
cmd(inter, "focus $s.e1");

while(choice==0)
 Tcl_DoOneEvent(0);

if(choice==2)
 {
quit=0;
cmd(inter, "destroytop $s");
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
cmd(inter, "destroytop $s");
Tcl_UnlinkVar(inter, "value_search");
Tcl_UnlinkVar(inter, "condition");

if(cur!=NULL)
 choice=deb(cur, r, lab, res);
else
 choice=0;

running=pre_running;
break;




case 7:

if(lab!=NULL)
 {
 cmd(inter, "set answer [ tk_messageBox -type okcancel -default cancel -icon warning -title Warning -message \"Quitting the simulation run.\\n\\nPress 'Ok' to confirm.\" ]; if [ string equal -nocase $answer ok ] { set choice 0 } { set choice 1 }" );
 } 
else
 choice=0; 
if(choice==1) 
 {
 choice=deb(r,c, lab, res);
 break;
 }

cmd(inter, "set a [winfo exists .intval]");
cmd(inter, "if { $a==1} {destroytop .intval} {}");
cmd( inter, "set a [winfo exists .net]" );
cmd( inter, "if { $a==1 } { destroytop .net }" );
cmd( inter, "destroytop .deb" );
   choice=1;
     quit=1;
		 debug_flag=0;
		 break;
case 29:
		ch1=(char *)Tcl_GetVar(inter, "res",0);
		strcpy(ch, ch1);
     set_all(&choice, r, ch, 0);
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

cmd( inter, "set e .execdeb" );
cmd( inter, "newtop $e \"Execute\" { set choice 1 }" );

cmd(inter, "button $e.eq -width -9 -text Equation -command {set choice 8}");
if(cv->param==0 || cv->param==2)
 cmd(inter, "button $e.exec -width -9 -text \"Execute\" -command {set choice 9}");
cmd(inter, "button $e.cond -width -9 -text \"Set Conditional Break\" -command {set choice 7}");
cmd(inter, "frame $e.past");
cmd(inter, "frame $e.past.tit");
cmd(inter, "label $e.past.tit.lab -text \"Values of: $res\"");
cmd(inter, "label $e.past.tit.time -text \"Current time: $time\"");
if(cv->param==0)
  {sprintf(msg, "label $e.past.tit.lu -text \"Time of last computation: %d\"",cv->last_update);
   cmd(inter, msg);
  }
else
  cmd(inter, "label $e.past.tit.lu -text \"Parameter\"");
cmd(inter, "pack $e.past.tit.lab $e.past.tit.time $e.past.tit.lu");
cmd(inter, "pack $e.past.tit");

Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);
app_values=new double[cv->num_lag+1];
for(i=0; i<cv->num_lag+1; i++)
 {
  sprintf(msg, "set val%d %g",i, cv->val[i]);

  cmd(inter, msg);
  app_values[i]=cv->val[i];
  sprintf(ch, "val%d",i);
  Tcl_LinkVar(inter, ch, (char *) &(app_values[i]), TCL_LINK_DOUBLE);
  cmd(inter, "frame $e.past.l$i");

  if(cv->param==0)
    cmd(inter, "label $e.past.l$i.n$i -text \"Lag $i: \"");
  else
    cmd(inter, "label $e.past.l$i.n$i -text \"Value: \"");
  cmd(inter, "entry $e.past.l$i.e$i -width 10 -textvariable val$i");
  
  sprintf(msg, "button $e.past.l$i.sa -width -9 -text \"Set All\" -command {set sa %i; set choice 10}", i);
  cmd(inter, msg);
  cmd(inter, "pack $e.past.l$i.n$i $e.past.l$i.e$i $e.past.l$i.sa -side left");
  cmd(inter, "pack $e.past.l$i");
 }

cmd(inter, "checkbutton $e.deb -text \"Debug \" -variable debug");
cmd(inter, "checkbutton $e.deball -text \"Apply debug option to all copies\" -variable debugall");

if(cv->param==0 || cv->param==2)
 cmd(inter, "pack $e.past $e.deb $e.deball $e.eq $e.exec $e.cond");
else
 cmd(inter, "pack $e.past $e.deb $e.eq $e.cond"); 

cmd( inter, "donehelp $e b { set choice 1 } { LsdHelp debug.html#content }" );
cmd( inter, "showtop $e" );

cmd(inter, "$e.past.l0.e0 selection range 0 end");
cmd(inter, "focus -force $e.past.l0.e0");
cmd(inter, "bind $e.past.l0.e0 <Return> {set choice 1}");

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
cmd(inter, "destroytop $e");

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
{
 show_eq(cv->label, &choice);
 choice = 8;
}

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
cmd(inter, "set a [winfo exists .intval]");
cmd(inter, "if { $a==1} {destroytop .intval} {}");
cmd( inter, "set a [winfo exists .net]" );
cmd( inter, "if { $a==1 } { destroytop .net }" );

actual_steps=t;
for(cur=r; cur->up!=NULL; cur=cur->up);
reset_end(cur);
Tcl_LinkVar(inter, "choice", (char *) &choice, TCL_LINK_INT);
analysis(&choice);
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
cmd( inter, "set a [winfo exists .intval]" );
cmd( inter, "if { $a==1 } { destroytop .intval }" );
cmd(inter, "set a [winfo exists .net]");
cmd( inter, "if { $a==1 } { destroytop .net }" );

cmd(inter, "set in .intval; newtop $in \"v\\[...\\]\" { destroytop $in }; text $in.t -width 20 -yscrollcommand \"$in.yscroll set\" -wrap word; scrollbar $in.yscroll -command \"$in.t yview\"; pack $in.yscroll -side right -fill y; pack $in.t -expand yes -fill both; done $in c { destroytop $in }; bind $in <Escape> { destroytop $in }; showtop $in topleftW 0 1 0");

cmd(inter, "$in.t tag configure v -foreground red");
cmd(inter, "align $in .deb");
for(i=0; i<100; i++)
 {
  sprintf(msg, "$in.t insert end \"v\\\[%d\\]\" v",i);
  cmd(inter, msg);
  sprintf(msg, "$in.t insert end \"=%g\n\"",i_values[i]);
  cmd(inter, msg);

 }
choice=0;
break;

case 16:
//run until
choice=0;

cmd( inter, "set t .tdebug" );
cmd( inter, "newtop $t \"Time Debugger\" { set choice 1 }" );

cmd(inter, "label $t.l -text \"Time for activation of debug mode\"");
sprintf(msg, "set when_debug %d",t+1);
cmd(inter, msg);
cmd(inter, "entry $t.val -width 10 -relief sunken -textvariable when_debug");
cmd(inter, "$t.val selection range 0 end");
cmd(inter, "pack $t.l $t.val");

cmd( inter, "okhelp $t b {set choice 1} {LsdHelp debug.html#until}" );
cmd( inter, "showtop $t" );

cmd(inter, "bind $t <KeyPress-Return> {set choice 1}");
cmd(inter, "$t.val selection range 0 end");
cmd(inter, "focus $t.val");

while(choice==0)
 Tcl_DoOneEvent(0);

cmd(inter, "destroytop $t");

//restart execution
choice=2;
cmd(inter, "set a [winfo exists .intval]");
cmd(inter, "if { $a==1} {destroytop .intval} {}");
cmd( inter, "set a [winfo exists .net]" );
cmd( inter, "if { $a==1 } { destroytop .net }" );
debug_flag=0;
cmd( inter, "destroytop .deb" );

break;

case 17:
//change the object number
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
cmd(inter, "if {$choice == 0} {tk_messageBox -type ok -title Error -icon error -message \"Report file not available.\\n\\nYou can create the report in menu Model.\"} {}");
choice=0;
break;



default:
plog("\nChoice not recognized");
choice=0;
break;
}
}

Tcl_UnlinkVar(inter, "value");
*res=app_res;

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

cmd(inter, "destroy .deb.cc .deb.t .deb.tit");

cmd(inter, "frame .deb.t -relief groove -bd 2");


cmd(inter, "label .deb.t.obj -text \"Level & object instance: \" ");
if(r->up!=NULL)
{
 cmd(inter, "bind .deb.t.obj <Button-1> {if {[winfo exist .deb.w]==1} {destroy .deb.w} {}; set choice 17}");
// cmd(inter, "bind .deb.t.obj <Enter> {set mx %X; set my %Y; toplevel .deb.w; wm geometry .deb.w +$mx+$my; wm title .deb.w \"\"; label .deb.w.l -text \"Click here to change\\nObj. number\"; pack .deb.w.l}");
// cmd(inter, "bind .deb.t.obj <Leave> {destroy .deb.w}");
} 

/*
strcpy(ch, "label .deb.t.obj -text \"Object Name (ascendants and instance numbers): \" ");

strcat(ch, r->label);
this_instance_number(r);
strcat(ch, msg);
strcat(ch, "\"");
cmd(inter, ch);
**/
strcpy(ch, "");

attach_instance_number(ch, r);
sprintf(msg, "label .deb.t.instance -fg red -text \"%s\"", ch);
cmd(inter, msg);
cmd(inter, "pack .deb.t.obj .deb.t.instance -side left -anchor s");

cmd(inter, "pack .deb.t -padx 2 -pady 10 -anchor w -before .deb.b");	// fix this frame before proceeding

// adjust spacing to align labels with data and increase columns width to better fill window
cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set w1 26; set w2 10; set w3 20} {set w1 19; set w2 9; set w3 15}");
cmd(inter, "frame .deb.tit");
cmd(inter, "frame .deb.tit.h1");
cmd(inter, "label .deb.tit.h1.name -text Variable -width $w1 -pady 0 -bd 0 -anchor w");
cmd(inter, "label .deb.tit.h1.last -text LastUpdate -width $w2 -pady 0 -bd 0 -anchor w -foreground red");
cmd(inter, "label .deb.tit.h1.val -text Value -width $w3 -pady 0 -bd 0 -anchor w");
cmd(inter, "pack .deb.tit.h1.name .deb.tit.h1.val .deb.tit.h1.last -side left");

cmd(inter, "frame .deb.tit.h2");
cmd(inter, "label .deb.tit.h2.name -text Variable -width $w1 -pady 0 -bd 0 -anchor w");
cmd(inter, "label .deb.tit.h2.last -text LastUpdate -width $w2 -pady 0 -bd 0 -anchor w -foreground red");
cmd(inter, "label .deb.tit.h2.val -text Value -width $w3 -pady 0 -bd 0 -anchor w");
cmd(inter, "pack .deb.tit.h2.name .deb.tit.h2.val .deb.tit.h2.last -side left -anchor w");

cmd(inter, "pack .deb.tit.h1 .deb.tit.h2 -side left");
cmd(inter, "pack .deb.tit -side top -anchor w -after .deb.t");	// align this frame to the left

cmd(inter, "frame .deb.cc -relief groove -bd 2");
cmd(inter, "scrollbar .deb.cc.scroll -command \".deb.cc.l yview\"");                                    //before 100

cmd(inter, "if {$tcl_platform(os)==\"Darwin\"} {set wwidth 115} {set wwidth 100}");
cmd(inter, "text .deb.cc.l -yscrollcommand \".deb.cc.scroll set\" -wrap word -width $wwidth -height 200 -cursor arrow");



if(r->v==NULL)
  {cmd(inter, "label .deb.cc.l.no_var -text \"(no variables defined)\"");
	cmd(inter,".deb.cc.l window create end -window .deb.cc.l.no_var");
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
	  cmd(inter, "frame .deb.cc.l.e$i");
	  strcpy(ch, "label .deb.cc.l.e$i.name -width $w1 -pady 0 -anchor w -bd 0 -text ");
	  strcat(ch, ap_v->label);
	  cmd(inter, ch);
	  if(ap_v->param==0)
		 cmd(inter, "label .deb.cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text $last -foreground red");
	  if(ap_v->param==1)
		 cmd(inter, "label .deb.cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text Par -foreground red");
	  if(ap_v->param==2)
		 cmd(inter, "label .deb.cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text Fun -foreground red");
	  cmd(inter, "label .deb.cc.l.e$i.val -width $w3 -pady 0 -bd 0 -anchor w -text $val");

	  cmd(inter, "pack .deb.cc.l.e$i.name .deb.cc.l.e$i.val .deb.cc.l.e$i.last -side left");
//	  if(ap_v->param==0)
		{strcpy(ch, "bind .deb.cc.l.e$i.name <Double-Button-1> {set res ");
		 strcat(ch, ap_v->label);
		 strcat(ch, "; set choice 8}");
		 cmd(inter, ch);

		 strcpy(ch, "bind .deb.cc.l.e$i.name <Button-3> {set res ");
		 strcat(ch, ap_v->label);
		 strcat(ch, "; set choice 29}");
		 cmd(inter, ch);
		}
	  cmd(inter,".deb.cc.l window create end -window .deb.cc.l.e$i");
     if( (i%2)==0)
       cmd(inter, ".deb.cc.l insert end \\n");
	  Tcl_UnlinkVar(inter, "last");
//	  Tcl_UnlinkVar(inter, "val");
	 }
	Tcl_UnlinkVar(inter, "i");

  }

cmd(inter, "pack .deb.cc.scroll -side right -fill y");
cmd(inter, ".deb.cc.l conf -height 20");
cmd(inter, "pack .deb.cc.l -expand 1 -fill y -fill x");
cmd(inter, "pack .deb.cc -expand 1  -fill x -fill y -after .deb.tit");


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

cmd( inter, "set c .condbrk" );
cmd( inter, "newtop $c \"Conditional Breaks\" { set choice 1 }" );

choice=0;
while(choice==0)
{
switch(cv->deb_cond)
 {case 0: cmd(inter, "label $c.cnd_type -text \"No conditional break\"");
          break;
  case 1: cmd(inter, "label $c.cnd_type -text \"Condition: = \"");
          break;
  case 2: cmd(inter, "label $c.cnd_type -text \"Condition: < \"");
          break;
  case 3: cmd(inter, "label $c.cnd_type -text \"Condition: > \"");
          break;
  default: 
		  sprintf( msg, "debug condition for var '%s' set to '%d'",cv->label, cv->deb_cond);
		  error_hard( msg, "Internal error", "If error persists, please contact developers." );
		  myexit(23);
          break;
 }
old=cv->deb_cond;
strcpy(ch, "label $c.name -text \"Conditional stop for variable:");
strcat(ch, cv->label);
strcat(ch, "\"");
cmd(inter, ch);       
cmd(inter, "entry $c.cond -relief sunken -textvariable cond_val");
cmd(inter, "frame $c.c");
cmd(inter, "button $c.c.eq -width -9 -text \" = \" -command {set cond 1}");
cmd(inter, "button $c.c.min -width -9 -text \" < \" -command {set cond 2}");
cmd(inter, "button $c.c.max -width -9 -text \" > \" -command {set cond 3}");
cmd(inter, "button $c.no -width -9 -text \"No Condition\" -command {set cond 0}");

cmd(inter, "pack $c.c.min $c.c.eq $c.c.max -side left");
cmd(inter, "pack $c.name $c.cnd_type $c.c $c.cond $c.no -side top");

cmd( inter, "done $c b { set choice 1 }" );
cmd( inter, "showtop $c" );

while(choice==0 && cv->deb_cond==old)
 Tcl_DoOneEvent(0);

cmd(inter, "destroy $c.cnd_type $c.name $c.cond $c.c $c.no $c.b");
}

cmd(inter, "destroytop $c");

Tcl_UnlinkVar(inter, "cond");
Tcl_UnlinkVar(inter, "cond_val");

choice=0;

}

int depth;

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
   sprintf( msg, "%d:%s (1/1) ", depth = 1, r->label );
else
   sprintf( msg, " |  %d:%s (%d/%d) ", ++depth, r->label, j, i - 1 );
strcat(ch, msg);
}

void this_instance_number(object *r)
{

object *cur;
int i, j;

if(r->up==NULL)
 {
 strcpy(msg, "");
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
