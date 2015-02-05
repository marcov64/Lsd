/***************************************************
****************************************************
LSD 6.4 - January 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/



/****************************************************
EDIT_DAT.CPP
Called by INTERF.CPP shows all the lagged variables and parameters
to be initialized for one object. Prepares the spread-sheet window
and the bindings. It can exits in three ways:
1) return to the calling function (either Browser or set object nunmber)
2) setall, calling the routine to set all values at once
3) move to set object number
This interface shows a maximum of 100 columns, though it allows to set all
the initial values of the model by using the setall options. In case
you need to individually observe and edit the data of objects not shown
by this function, use the Data Browse option.

The functions contained in this file are:

- void edit_data(object *root, int *choice, char *obj_name)
Initialize the window, calls search_title and link_data, then wait for a
message from user.

- void search_title(object *root, char *tag, int *i, char *lab, int *incr)
It is a recursive routine. Scan the model structure looking for the object
of type as root and prepare the relative tag for any object. The tag is then
used by set_title to be printed as columns headers

- void set_title(object *c, char *lab, char *tag, int *incr);
prints the column headers

- void link_data(object *root, char *lab);
prints the line headers and create the cells, each linked to one variable value
of the model

- void clean_cell(object *root, char *tag, char *lab);
called before exiting, removes all the links between tcl variables and model
values



Functions used here from other files are:

- void set_all(int *choice, object *r, char *lab, int lag);
SET_ALL.CPP It contains the routine called from the edit_dat file for setting all the
values of a variable with a function, instead of inserting manually.

- object *skip_next_obj(object *t, int *i);
UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- void cmd(Tcl_Interp *inter, char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

- object *go_brother(object *cur);
UTIL.CPP returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.

- void go_next(object **t)
UTIL.CPP
returns next if (*t)->next is not null. Don't know it is used any longer

- void show_graph( object *t);
DRAW.CPP shows the grsphical representation of the model

- void set_obj_number(object *r, int *choice);
EDIT.CPP allows to edit the number of instances in the model

****************************************************/


#include <tk.h>
#include "decl.h"

extern Tcl_Interp *inter;
extern char msg[];

void go_next(object **t);
object *go_brother(object *cur);
object *skip_next_obj(object *t, int *count);
void cmd(Tcl_Interp *inter, char const *cc);

void search_title(object *root, char *tag, int *i, char *lab, int *incr);
void clean_cell(object *root, char *tag, char *lab);
void edit_data(object *root, int *choice);
void set_title(object *c, char *lab, char *tag, int *incr);
void link_data(object *root, char *lab);
void set_all(int *choice, object *r, char *lab, int lag);
void set_obj_number(object *r, int *choice);
void myexit(int code);
void set_window_size(void);
int set_focus;

/****************************************************
EDIT_DATA

****************************************************/

void edit_data(object *root, int *choice, char *obj_name)
{
char ch[120], *l , ch1[120];
object *first;
variable *cv;
int i, counter, lag;


sprintf(msg, "if {$tcl_platform(os) == \"Darwin\"} {set cwidth 10; set cbd 1 } {set cwidth 8; set cbd 0}");
cmd(inter, msg);

cmd(inter, "bind . <KeyPress-Return> {}");
cmd(inter, "bind . <KeyPress-Escape> {.boh.ok invoke}");
*choice=0;
Tcl_LinkVar(inter, "lag", (char *) &lag, TCL_LINK_INT);
cmd(inter, "wm title . \"Lsd - Data Editor\"");
//cmd(inter, "wm resizable . 0 0");
cmd(inter, "set position 1.0");
while(*choice==0)
{
//find first object->label==obj_name;
first=root->search(obj_name);
cmd(inter, "frame .b");
cmd(inter, "set w .b.tx");
cmd(inter, "scrollbar .b.ys -command \".b.tx yview\"");
cmd(inter, "scrollbar .b.xs -command \".b.tx xview\" -orient horizontal");
cmd(inter, "text $w -yscrollcommand \".b.ys set\" -xscrollcommand \".b.xs set\" -wrap none");
cmd(inter, ".b.tx conf -cursor arrow");
strncpy(ch1, obj_name, 17 );
ch1[17]=0;
sprintf(ch, "label $w.tit_empty -width 36 -relief raised -text \"Object %-17s \" -borderwidth 4", ch1);
cmd(inter, ch);
cmd(inter, "bind $w.tit_empty <Button-1> {set choice 4}");
sprintf(ch,"bind $w.tit_empty <Enter> {set msg \"Click to edit Objects' Number\"}");
cmd(inter, ch);
cmd(inter, "bind $w.tit_empty <Leave> {set msg \"\"}");
cmd(inter, "$w window create end -window $w.tit_empty");

strcpy(ch, "");
i=0;
counter=1;
search_title(root, ch, &i, obj_name, &counter);
cmd(inter, "$w insert end \\n");


//explore the tree searching for each instance of such object and create:
//- titles
//- entry cells linked to the values
set_focus=0;
link_data(root, obj_name);
cmd(inter, "$w see $position");
cmd(inter, "pack .b.ys -side right -fill y");
cmd(inter, "pack .b.xs -side bottom -fill x");
cmd(inter, "pack .b.tx -expand yes -fill both");
cmd(inter, "pack .b  -expand yes -fill both");

cmd(inter, "label .msg -textvariable msg -anchor w");
cmd(inter, "pack .msg");

cmd(inter, "frame .boh");
cmd(inter, "button .boh.ok -text Ok -command {set choice 1}");
cmd(inter, "button .boh.help -text Help -command {LsdHelp mdatainit.html}");
cmd(inter, "pack .boh.ok .boh.help -side left");
cmd(inter, "pack .boh");

cmd(inter, "$w configure -state disabled");
cmd(inter, "bind . <KeyPress-Escape> {set choice 1}");
cmd(inter, "bind . <Destroy> {set choice 35}");
cmd(inter, "bind .log <Destroy> {set choice 35}");


set_window_size();
if(set_focus==1)
  cmd(inter, "focus $initial_focus; $initial_focus selection range 0 end");

 while(*choice==0)
	Tcl_DoOneEvent(0);

//clean up
if(*choice==35)
 myexit(0);
cmd(inter, "bind . <Destroy> {}");
cmd(inter, "bind .log <Destroy> {}");

strcpy(ch, "");
i=0;
clean_cell(root, ch, obj_name);
cmd(inter, "destroy .b .boh .msg");


if(*choice==2)
 {
  l=(char *)Tcl_GetVar(inter, "var-S-A",0);
  strcpy(ch, l);
  *choice=0;
  set_all(choice,first, ch, lag);
  cmd(inter, "bind . <KeyPress-Return> {}");
  *choice=0;


 }
if(*choice==4)
 { *choice=0;
   set_obj_number(root, choice);
   *choice=0;
 }

}
Tcl_UnlinkVar(inter, "lag");
//cmd(inter, "unset lag");

}

/****************************************************
SEARCH_TITLE

****************************************************/

void search_title(object *root, char *tag, int *i, char *lab, int *incr)
{
char ch[120];
int num, multi, counter, j;
object *c, *cur;
variable *cv;
bridge *cb;

set_title(root, lab, tag, incr);

//for(c=root->son, counter=1; c!=NULL;c=skip_next_obj(c, &num), counter=1)
for(cb=root->b, counter=1; cb!=NULL;cb=cb->next, counter=1)
 {  
 c=cb->head;
 *i=*i+1;
 if( c->next!=NULL)
    multi=1;
  else
    multi=0;
  for(cur=c; cur!=NULL; counter++, cur=go_brother(cur))
   {
   if(multi==1)
      if(strlen(tag)!=0)
        sprintf(ch, "%s-%d",tag, counter);
      else
        sprintf(ch, "%d",counter);
   else
		sprintf(ch, "%s",tag);
   // WARNING: Limits to 100 number of cells
	if(*incr<101)
	 search_title(cur, ch, i, lab, incr);

   }
  }

}

/****************************************************
SET_TITLE

****************************************************/

void set_title(object *c, char *lab, char *tag, int *incr)
{
int j;
variable *cv;
char ch[120], ch1[11], ch2[10];

if(!strcmp(c->label, lab))
{
  strncpy(ch1, c->label, 10);
  if(strlen(tag)!=0)
  {  strncpy(ch2, tag, 9);
     ch2[9]=0;
  }
else
 strcpy(ch2, "  ");

//sprintf(ch, "label $w.c%d_tit -relief groove -width 8 -bd 1 -text \"%9s\"", *incr ,ch2);

/*
if(*incr%2!=0)
 sprintf(ch, "label $w.c%d_tit -width 8 -bd 1 -text \"%s\"", *incr ,ch2);
else
 sprintf(ch, "label $w.c%d_tit -width 8 -bd 2 -text \"%s\"", *incr ,ch2); 
*/

//sprintf(ch, "label $w.c%d_tit -width 9 -bd 1 -text \"%s\"", *incr ,ch2); 
	
sprintf(ch, "label $w.c%d_tit -width $cwidth	-bd $cbd -relief raised -text \"%s\"", *incr ,ch2);
cmd(inter, ch);
sprintf(ch, "$w window create end -window $w.c%d_tit", *incr);
cmd(inter, ch);
if(strlen(tag)==0)
  sprintf(ch, "set tag_%d \" \"", *incr);
else
  sprintf(ch, "set tag_%d %s", *incr, tag);
cmd(inter,ch);
*incr=*incr+1;
}

}


/****************************************************
CLEAN_CELL

****************************************************/

void clean_cell(object *root, char *tag, char *lab)
{
char ch[120];
int j, i;
object *cur;
variable *cv;
cur=root->search(lab);
// WARNING: Cell number limited to 100 !!!!!!!
for(i=1 ; i<101 && cur!=NULL; cur=cur->hyper_next(lab), i++)
{

for(cv=cur->v; cv!=NULL; cv=cv->next)
 {if(cv->param==1)
    { sprintf(ch,"p%s_%d", cv->label,i);
      Tcl_UnlinkVar(inter, ch);
    }
  else
    { for(j=0; j<cv->num_lag; j++)
      {
      sprintf(ch,"v%s_%d_%d", cv->label,i, j);
      Tcl_UnlinkVar(inter, ch);
       }
    }
  }
}

}

/****************************************************
LINK_DATA

****************************************************/

void link_data(object *root, char *lab)
{
object *cur, *cur1;
int i, j;
char ch[350], previous[60], ch1[25];
variable *cv, *cv1;

cur1=root->search(lab);
strcpy(previous, "");
for(cv1=cur1->v, j=0; cv1!=NULL;  )
 {
 if(cv1->param==1)
    { strncpy(ch1, cv1->label, 18);
      ch1[18]=0;
      sprintf(ch, "label $w.tit_t%s -anchor w -width 25 -text \"Param: %-18s\" -borderwidth 4", cv1->label, ch1);
      cmd(inter, ch);
      sprintf(ch, "$w window create end -window $w.tit_t%s", cv1->label);
      cmd(inter, ch);
      sprintf(ch, "bind $w.tit_t%s <Enter> {set msg \"Parameter: %s\"}", cv1->label,cv1->label);
		cmd(inter, ch);
		sprintf(ch, "bind $w.tit_t%s <Leave> {set msg \" \"}", cv1->label);
		cmd(inter, ch);
	sprintf(ch, "button $w.b%s_%d -width 9 -text \"Set All\" -pady 0m -padx 1m -command {set choice 2; set var-S-A %s; set lag %d; set position $w.tit_t%s}", cv1->label, j, cv1->label, j,cv1->label );
	cmd(inter, ch);
	sprintf(ch, "$w window create end -window $w.b%s_%d", cv1->label, j);
		 cmd(inter, ch);

    }
  else
    { 
     if(j<cv1->num_lag)
     {
       strncpy(ch1, cv1->label, 16);
       ch1[16]=0;
       sprintf(ch, "label $w.tit_t%s_%d -anchor w -width 25 -text \"Var: %-16s -%d \" -borderwidth 4", cv1->label,j, ch1, j+1);
		 cmd(inter, ch);
		 sprintf(ch, "$w window create end -window $w.tit_t%s_%d", cv1->label, j);
		 cmd(inter, ch);
		 sprintf(ch, "bind $w.tit_t%s_%d <Enter> {set msg \"Var: %s with lag %d\" }", cv1->label, j, cv1->label, j+1);
		 cmd(inter, ch);
		 sprintf(ch, "bind $w.tit_t%s_%d <Leave> {set msg \" \" }", cv1->label, j);
		 cmd(inter, ch);
	sprintf(ch, "button $w.b%s_%d -width 9 -text \"Set All\" -pady 0m -padx 1m -command {set choice 2; set var-S-A %s; set lag %d; set position $w.tit_t%s_%d}", cv1->label, j, cv1->label, j,cv1->label, j);
	cmd(inter, ch);
	sprintf(ch, "$w window create end -window $w.b%s_%d", cv1->label, j);
       cmd(inter, ch);

     }
    }
// WARNING: Cell number limited to 100 !!!!!!!!!
 for(cur=cur1, i=1; i<101 && cur!=NULL; cur=cur->hyper_next(lab) , i++)
 {
  cv=cur->search_var(cur, cv1->label);
  cv->data_loaded='+';
  if(cv->param==1)
    { sprintf(ch,"p%s_%d", cv->label,i);
      Tcl_LinkVar(inter, ch, (char *) &(cv->val[0]), TCL_LINK_DOUBLE);
      sprintf(ch, "entry $w.c%d_v%sp -textvariable p%s_%d -width 9",i, cv->label, cv->label, i);
      cmd(inter, ch);
      if(set_focus==0)
       {
       sprintf(ch, "set initial_focus $w.c%d_v%sp",i, cv->label);
       cmd(inter, ch);
       set_focus=1;
       }
      sprintf(ch, "$w window create end -window $w.c%d_v%sp", i, cv->label);
      cmd(inter, ch);
      if(strlen(previous)!=0)
       {sprintf(ch, "bind %s <KeyPress-Return> {focus $w.c%d_v%sp;$w.c%d_v%sp selection range 0 end; $w see $w.c%d_v%sp}", previous, i, cv->label, i, cv->label, i, cv->label);
        cmd(inter, ch);
       }
      sprintf(ch, "bind $w.c%d_v%sp <FocusIn> {set msg \"Inserting: Param %s in %s $tag_%d\"}",i,cv->label,cv->label,cur1->label,i);
      cmd(inter, ch);
      sprintf(ch, "bind $w.c%d_v%sp <FocusOut> {set msg \" \"}", i, cv->label);
      cmd(inter, ch);

      sprintf(previous, "$w.c%d_v%sp", i, cv->label);
    }
  else
    { if(j<cv->num_lag)
      {
      sprintf(ch,"v%s_%d_%d", cv->label,i, j);
      Tcl_LinkVar(inter, ch, (char *) &(cv->val[j]), TCL_LINK_DOUBLE);
      sprintf(ch, "entry $w.c%d_v%s_%d -textvariable v%s_%d_%d -width 9",i, cv->label,j, cv->label, i, j);
      cmd(inter, ch);
      if(set_focus==0)
       {
       sprintf(ch, "set initial_focus $w.c%d_v%s_%d",i, cv->label,j);
       cmd(inter, ch);
       set_focus=1;
       }

      sprintf(ch, "$w window create end -window $w.c%d_v%s_%d", i, cv->label, j);
      cmd(inter, ch);
      if(strlen(previous)!=0)
       {sprintf(ch, "bind %s <KeyPress-Return> {focus $w.c%d_v%s_%d;$w.c%d_v%s_%d selection range 0 end;$w see  $w.c%d_v%s_%d}", previous, i, cv->label, j, i, cv->label, j, i, cv->label, j);
        cmd(inter, ch);
       }
      sprintf(ch, "bind $w.c%d_v%s_%d <FocusIn> {set msg \"Inserting: Var %s lag %d in %s $tag_%d\"}",i,cv->label,j,cv->label,j+1,cur1->label,i);
      cmd(inter, ch);
      sprintf(ch, "bind $w.c%d_v%s_%d <FocusOut> {set msg \" \"}",i,cv->label,j);
      cmd(inter, ch);

      sprintf(previous, "$w.c%d_v%s_%d", i, cv->label, j);
      }
  }
  }

//set flag of data loaded also to not shown pars.
  for( ;cur!=NULL; cur=cur->hyper_next(lab) )
     {
        cv=cur->search_var(cur, cv1->label);
        cv->data_loaded='+';

     }
 if(cv1->param==1 || cv1->num_lag>0)
   cmd(inter, "$w insert end \\n" );
 if(cv1->param==0 && j+1<cv1->num_lag)
   j++;
 else
   {cv1=cv1->next;
    j=0;
   }
}
}
