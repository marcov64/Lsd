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
SETALL.CPP
It contains the routine called from the edit_dat file for setting all the
values of a variable with a function, instead of inserting manually.

The functions contained in this file are:

-void set_all(int *choice, object *r, char *lab, int lag)
it allows 5 options to set all values. It uses one value entered by the user
in this window and, for some option, the first value for this variable in the
model. That is, the value for this variable contained in the first object of this
type.
The options are the following:
1) set all values equal to the entered value
2) the first value is not changed and all the others are computed as the previous
plus the entered object.
3) as before, but instead of producing a ever increasing series, it re-initialize
any new group.
4) random numbers, drawn by a uniform value whose min is the first value
and max is the inserted value
5) random numbers, drawn by a normal whose mean is the first value and
standard deviation is the inserted value.



-object *create(Tcl_Interp *inter, object *root)
The main cycle for the Browser, from which it exits only to run a simulation
or to quit the program. The cycle is just once call to browsw followed by
a call to operate.

- int browse(Tcl_Interp *inter, object *r, int *choice);
build the browser window and waits for an action (on the form of
values for choice or choice_g different from 0)

- object *operate(Tcl_Interp *in, int *choice, object *r);
takes the value of choice and operate the relative command on the
object r. See the switch for the complete list of the available commands

- void clean_debug(object *n);
remove all the flags to debug from any variable in the model

- void clean_save(object *n);
remove all the flags to save from any variable in the model

- void clean_plot(object *n);
remove all the flags to plot from any variable in the model


Functions used here from other files are:

- void cmd(Tcl_Interp *inter, char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

- double norm(double mean, double dev)
UTIL.CPP returns a random number drawn from a normal with mean mean and standard deviation\
dev.

- double rnd(void);
UTIL.CPP return the uniform value. Now is only implemented using the internal
random generator, but it can be (and should...) linked with a serious random
generator.
****************************************************/

#include "decl.h"


/****************************************************
SET_ALL

****************************************************/

void set_all(int *choice, object *original, char *lab, int lag)
{
char *l, ch[TCL_BUFF_STR];
int res, i, kappa, cases_from=1, cases_to=0, to_all, update_description, fill=0;
bool exist;
object *cur, *r;
double value, value1, value2, step, counter;
variable *cv;
FILE *f;
description *cd; 
sense *cs;

if(original->up!=NULL)
 for(r=original->up; r->up!=NULL; r=r->up);//go for the root
else
r=original; 
r=r->search(original->label);//select the first instance
cv=r->search_var(NULL, lab);
if ( cv == NULL )
	return;

Tcl_LinkVar(inter, "res", (char *) &res, TCL_LINK_INT);
Tcl_LinkVar(inter, "value1", (char *) &value1, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "value2", (char *) &value2, TCL_LINK_DOUBLE);

value1 = cv->val[0];		// preload the existing value of the first object

// default values
res=1;
value2=0;
cmd(inter, "set value 1"); 	// method
cmd(inter, "set to_all 1");
cmd(inter, "set step_in 1");
cmd(inter, "set cases_from 1; set cases_to 10000");
cmd(inter, "set rnd_seed 1");
cmd(inter, "set use_seed 0");
cmd(inter, "set update_d 1");

cmd( inter, "newtop .sa \"Set All Objects Initialization\" { set choice 2 }" );

// heading
cmd(inter, "frame .sa.head");
cmd(inter, "label .sa.head.lg -text \"Set initial values for every copy of\"");

cmd(inter, "frame .sa.head.l");
if ( cv->param == 1 )
{
  cmd( inter, "label .sa.head.l.c -text \"Parameter: \"" );
  sprintf( ch, "label .sa.head.l.n -text \"%s\" -fg red", lab );
  cmd( inter, ch );
}
else
{
  if ( cv->param == 2 )
	cmd( inter, "label .sa.head.l.c -text \"Function: \"" );
  else
    cmd( inter, "label .sa.head.l.c -text \"Variable: \"" );
  sprintf( ch, "label .sa.head.l.n -text \"%s \\[lag -%d\\]\" -fg red", lab, t-cv->last_update + lag + 1 );
  cmd( inter, ch );
}
  
cmd( inter, "pack .sa.head.l.c .sa.head.l.n -side left" );

cmd( inter, "frame .sa.head.lo" );
cmd( inter, "label .sa.head.lo.l -text \"Contained in object: \"" );
sprintf( ch, "label .sa.head.lo.o -text \"%s\" -fg red", cv->up->label );
cmd( inter, ch );
cmd( inter, "pack .sa.head.lo.l .sa.head.lo.o -side left" );

cmd(inter, "pack .sa.head.lg .sa.head.l .sa.head.lo");

// main content
cmd(inter, "frame .sa.m");

// left column
cmd(inter, "frame .sa.m.f1");

cmd(inter, "frame .sa.m.f1.val -relief groove -bd 2");

cmd(inter, "label .sa.m.f1.val.l -text \"Initialization data\"");
cmd(inter, "pack .sa.m.f1.val.l");

cmd(inter, "frame .sa.m.f1.val.l1");
cmd(inter, "label .sa.m.f1.val.l1.l1 -text \"Value 1\"");
cmd( inter, "entry .sa.m.f1.val.l1.e1 -validate focusout -vcmd { if [ string is double %P ] { set value1 %P; return 1 } { %W delete 0 end; %W insert 0 $value1; return 0 } } -invcmd { bell } -justify center" );
cmd(inter, "pack .sa.m.f1.val.l1.l1 .sa.m.f1.val.l1.e1");

cmd(inter, "frame .sa.m.f1.val.l2");
cmd(inter, "label .sa.m.f1.val.l2.l2 -text \"Value 2\"");
cmd( inter, "entry .sa.m.f1.val.l2.e2 -validate focusout -vcmd { if [ string is double %P ] { set value2 %P; return 1 } { %W delete 0 end; %W insert 0 $value2; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd(inter, "pack .sa.m.f1.val.l2.l2 .sa.m.f1.val.l2.e2");

cmd(inter, "pack .sa.m.f1.val.l1 .sa.m.f1.val.l2 -expand yes -fill x  -ipadx 5 -ipady 2");

cmd(inter, "frame .sa.m.f1.rd -relief groove -bd 2");

cmd(inter, "label .sa.m.f1.rd.l -text \"Initialization method\"");
cmd(inter, "pack .sa.m.f1.rd.l -pady 2");

cmd(inter, "radiobutton .sa.m.f1.rd.r1 -text \"Equal to\" -variable res -value 1 -command { .sa.m.f1.val.l1.l1 conf -text \"Value\"; .sa.m.f1.val.l1.e1 conf -state normal; .sa.m.f1.val.l2.l2 conf -text \"(none)\"; .sa.m.f1.val.l2.e2 conf -state disabled }");
cmd(inter, "bind .sa.m.f1.rd.r1 <Down> {focus .sa.m.f1.rd.r9; .sa.m.f1.rd.r9 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r1 <Return> { .sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1}");

cmd(inter, "radiobutton .sa.m.f1.rd.r9 -text \"Range\" -variable res -value 9 -command { .sa.m.f1.val.l1.l1 conf -text \"Minimum\"; .sa.m.f1.val.l1.e1 conf -state normal; .sa.m.f1.val.l2.l2 conf -text \"Maximum\"; .sa.m.f1.val.l2.e2 conf -state normal }");
cmd(inter, "bind .sa.m.f1.rd.r9 <Down> {focus .sa.m.f1.rd.r2; .sa.m.f1.rd.r2 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r9 <Up> {focus .sa.m.f1.rd.r1; .sa.m.f1.rd.r1 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r9 <Return> { .sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1}");

cmd(inter, "radiobutton .sa.m.f1.rd.r2 -text \"Increasing\" -variable res -value 2 -command { .sa.m.f1.val.l1.l1 conf -text \"Start\"; .sa.m.f1.val.l1.e1 conf -state normal; .sa.m.f1.val.l2.l2 conf -text \"Step\"; .sa.m.f1.val.l2.e2 conf -state normal }");
cmd(inter, "bind .sa.m.f1.rd.r2 <Down> {focus .sa.m.f1.rd.r4; .sa.m.f1.rd.r4 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r2 <Up> {focus .sa.m.f1.rd.r9; .sa.m.f1.rd.r9 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r2 <Return> { .sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1}");

cmd(inter, "radiobutton .sa.m.f1.rd.r4 -text \"Increasing (groups)\" -variable res -value 4 -command {.sa.m.f1.val.l1.l1 conf -text \"Start\"; .sa.m.f1.val.l1.e1 conf -state normal; .sa.m.f1.val.l2.l2 conf -text \"Step\"; .sa.m.f1.val.l2.e2 conf -state normal }");
cmd(inter, "bind .sa.m.f1.rd.r4 <Up> {focus .sa.m.f1.rd.r2; .sa.m.f1.rd.r2 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r4 <Down> {focus .sa.m.f1.rd.r3; .sa.m.f1.rd.r3 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r4 <Return> { .sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1}");

cmd(inter, "radiobutton .sa.m.f1.rd.r3 -text \"Random (uniform)\" -variable res -value 3 -command { .sa.m.f1.val.l1.l1 conf -text \"Minimum\"; .sa.m.f1.val.l1.e1 conf -state normal; .sa.m.f1.val.l2.l2 conf -text \"Maximum\"; .sa.m.f1.val.l2.e2 conf -state normal }");
cmd(inter, "bind .sa.m.f1.rd.r3 <Up> {focus .sa.m.f1.rd.r4; .sa.m.f1.rd.r4 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r3 <Down> {focus .sa.m.f1.rd.r8; .sa.m.f1.rd.r8 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r3 <Return> { .sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1}");

cmd(inter, "radiobutton .sa.m.f1.rd.r8 -text \"Random integer (uniform)\" -variable res -value 8 -command { .sa.m.f1.val.l1.l1 conf -text \"Minimum\"; .sa.m.f1.val.l1.e1 conf -state normal; .sa.m.f1.val.l2.l2 conf -text \"Maximum\"; .sa.m.f1.val.l2.e2 conf -state normal }");
cmd(inter, "bind .sa.m.f1.rd.r8 <Up> {focus .sa.m.f1.rd.r3; .sa.m.f1.rd.r3 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r8 <Down> {focus .sa.m.f1.rd.r5; .sa.m.f1.rd.r5 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r8 <Return> { .sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1}");

cmd(inter, "radiobutton .sa.m.f1.rd.r5 -text \"Random (normal)\" -variable res -value 5 -command {.sa.m.f1.val.l1.l1 conf -text \"Mean\"; .sa.m.f1.val.l1.e1 conf -state normal; .sa.m.f1.val.l2.l2 conf -text \"Std. Deviation\"; .sa.m.f1.val.l2.e2 conf -state normal }");
cmd(inter, "bind .sa.m.f1.rd.r5 <Up> {focus .sa.m.f1.rd.r8; .sa.m.f1.rd.r8 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r5 <Down> {focus .sa.m.f1.rd.r7; .sa.m.f1.rd.r7 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r5 <Return> { .sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1}");

cmd(inter, "radiobutton .sa.m.f1.rd.r7 -text \"Import from data file\" -variable res -value 7 -command { .sa.m.f1.val.l1.l1 conf -text \"(none)\"; .sa.m.f1.val.l1.e1 conf -state disabled; .sa.m.f1.val.l2.l2 conf -text \"(none)\"; .sa.m.f1.val.l2.e2 conf -state disabled }");
cmd(inter, "bind .sa.m.f1.rd.r7 <Up> {focus .sa.m.f1.rd.r5; .sa.m.f1.rd.r5 invoke}");
cmd(inter, "bind .sa.m.f1.rd.r7 <Return> {.sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1}");

cmd(inter, "pack .sa.m.f1.rd.r1 .sa.m.f1.rd.r9 .sa.m.f1.rd.r2 .sa.m.f1.rd.r4 .sa.m.f1.rd.r3 .sa.m.f1.rd.r8 .sa.m.f1.rd.r5 .sa.m.f1.rd.r7 -anchor w -padx 2");

cmd(inter, "pack .sa.m.f1.val .sa.m.f1.rd -expand yes -fill x  -padx 2 -pady 2");

// right column
cmd(inter, "frame .sa.m.f2");

cmd(inter, "frame .sa.m.f2.s -relief groove -bd 2");
cmd(inter, "label .sa.m.f2.s.tit -text \"Object selection\"");
cmd(inter, "pack .sa.m.f2.s.tit -pady 2");

cmd(inter, "frame .sa.m.f2.s.l");

cmd(inter, "frame .sa.m.f2.s.l.a");
cmd(inter, "label .sa.m.f2.s.l.a.l -text \"Apply every\"");
cmd( inter, "entry .sa.m.f2.s.l.a.e -width 5 -validate focusout -vcmd { if [ string is integer %P ] { set step_in %P; return 1 } { %W delete 0 end; %W insert 0 $step_in; return 0 } } -invcmd { bell } -justify center" );
cmd(inter, "label .sa.m.f2.s.l.a.l1 -text \"object(s)\"");
cmd(inter, "pack .sa.m.f2.s.l.a.l .sa.m.f2.s.l.a.e .sa.m.f2.s.l.a.l1 -side left -padx 1");

cmd(inter, "checkbutton .sa.m.f2.s.l.f -text \"Fill-in\" -variable fill");
cmd(inter, "pack  .sa.m.f2.s.l.a .sa.m.f2.s.l.f -padx 5 -side left");
cmd(inter, "pack  .sa.m.f2.s.l -pady 2");

cmd(inter, "frame .sa.m.f2.s.sel");
cmd(inter, "radiobutton .sa.m.f2.s.sel.all -text \"Apply to all objects\" -variable to_all -value 1 -command { .sa.m.f2.s.sel2.c.to conf -state disabled; .sa.m.f2.s.sel2.c.from conf -state disabled; bind .sa.m.f2.s.sel2.c.from <Button-3> { }; bind .sa.m.f2.s.sel2.c.to <Button-3> { }; bind .sa.m.f2.s.sel2.c.from <Button-2> { }; bind .sa.m.f2.s.sel2.c.to <Button-2> { } }");
cmd(inter, "radiobutton .sa.m.f2.s.sel.sel -text \"Apply to a group of objects\" -variable to_all -value 2 -command { .sa.m.f2.s.sel2.c.to conf -state normal; .sa.m.f2.s.sel2.c.from conf -state normal; bind .sa.m.f2.s.sel2.c.from <Button-3> {set choice 9}; bind .sa.m.f2.s.sel2.c.to <Button-3> { set choice 10 }; bind .sa.m.f2.s.sel2.c.from <Button-2> {set choice 9}; bind .sa.m.f2.s.sel2.c.to <Button-2> { set choice 10 } }");
cmd(inter, "pack .sa.m.f2.s.sel.all .sa.m.f2.s.sel.sel -anchor w");
cmd(inter, "pack .sa.m.f2.s.sel -pady 2");

cmd(inter, "frame .sa.m.f2.s.sel2");

cmd(inter, "frame .sa.m.f2.s.sel2.c");
cmd(inter, "label .sa.m.f2.s.sel2.c.lfrom -text \"From\"");
cmd( inter, "entry .sa.m.f2.s.sel2.c.from -width 5 -state disabled -state disabled -validate focusout -vcmd { if [ string is integer %P ] { set cases_from %P; return 1 } { %W delete 0 end; %W insert 0 $cases_from; return 0 } } -invcmd { bell } -justify center" );
cmd(inter, "label .sa.m.f2.s.sel2.c.lto -text \"to\"");
cmd( inter, "entry .sa.m.f2.s.sel2.c.to -width 5 -state disabled -validate focusout -vcmd { if [ string is integer %P ] { set cases_to %P; return 1 } { %W delete 0 end; %W insert 0 $cases_to; return 0 } } -invcmd { bell } -justify center" );
cmd(inter, "pack .sa.m.f2.s.sel2.c.lfrom .sa.m.f2.s.sel2.c.from .sa.m.f2.s.sel2.c.lto .sa.m.f2.s.sel2.c.to -side left -pady 1");

cmd(inter, "label .sa.m.f2.s.sel2.obs -text \"(use right button on cells for options)\"");
cmd(inter, "pack .sa.m.f2.s.sel2.c .sa.m.f2.s.sel2.obs");
cmd(inter, "pack .sa.m.f2.s.sel2 -pady 2");

cmd(inter, "pack .sa.m.f2.s -pady 2 -expand yes -fill x");

cmd(inter, "frame .sa.m.f2.rnd -relief groove -bd 2");
cmd(inter, "label .sa.m.f2.rnd.l -text \"Random number generator\"");

cmd(inter, "frame .sa.m.f2.rnd.le");
cmd(inter, "checkbutton .sa.m.f2.rnd.le.f -text \"Reset the generator\" -variable use_seed -command { if $use_seed { .sa.m.f2.rnd.le.e1 conf -state normal } { .sa.m.f2.rnd.le.e1 conf -state disabled } }");
cmd(inter, "frame .sa.m.f2.rnd.le.s");
cmd(inter, "label .sa.m.f2.rnd.le.s.l1 -text \"Seed\"");
cmd( inter, "entry .sa.m.f2.rnd.le.s.e1 -width 5 -state disabled -validate focusout -vcmd { if [ string is integer %P ] { set rnd_seed %P; return 1 } { %W delete 0 end; %W insert 0 $rnd_seed; return 0 } } -invcmd { bell } -justify center" );
cmd(inter, "pack .sa.m.f2.rnd.le.s.l1 .sa.m.f2.rnd.le.s.e1 -side left -padx 1");

cmd(inter, "pack .sa.m.f2.rnd.le.f .sa.m.f2.rnd.le.s -side left -padx 5");
cmd(inter, "pack .sa.m.f2.rnd.l .sa.m.f2.rnd.le -pady 2");

cmd(inter, "checkbutton .sa.m.f2.ud -text \"Update description\" -variable update_d");

cmd(inter, "pack .sa.m.f2.s .sa.m.f2.rnd .sa.m.f2.ud -anchor w -expand yes -fill x");

cmd(inter, "pack .sa.m.f1 .sa.m.f2 -side left -expand yes -fill both -padx 5 -pady 5");
cmd(inter, "pack .sa.head .sa.m -pady 5");

cmd( inter, "okhelpcancel .sa b { set choice 1 } { LsdHelp mdatainit.html#setall } { set choice 2 }" );

cmd( inter, "bind .sa.m.f1.rd <Return> {  if [ string equal [ .sa.m.f1.val.l2.e2 cget -state ] normal ] { .sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1 } }" );
cmd( inter, "bind .sa.m.f1.val.l1.e1 <Return> { if [ string equal [ .sa.m.f1.val.l2.e2 cget -state ] normal ] { focus .sa.m.f1.val.l2.e2; .sa.m.f1.val.l2.e2 selection range 0 end } { set choice 1 } }" );
cmd(inter, "bind .sa.m.f1.val.l2.e2 <Return> { set choice 1 }");
cmd(inter, "bind .sa.m.f2.s.l.a.e <Return> {focus .sa.m.f2.s.sel.all; .sa.m.f2.s.sel.all invoke}");
cmd(inter, "bind .sa.m.f2.s.sel.all <Return> {focus .sa.b.ok}");
cmd(inter, "bind .sa.m.f2.s.sel.sel <Return> {focus .sa.m.f2.s.sel2.c.from; .sa.m.f2.s.sel2.c.from selection range 0 end }");
cmd(inter, "bind .sa.m.f2.s.sel2.c.from <Return> {focus .sa.m.f2.s.sel2.c.to; .sa.m.f2.s.sel2.c.from selection range 0 end }");
cmd(inter, "bind .sa.m.f2.s.sel2.c.to <Return> {focus .sa.b.ok}");
cmd(inter, "bind .sa.m.f2.rnd.le.s.e1 <Return> {focus .sa.b.ok}");

cmd(inter, "showtop .sa topleftW");
cmd(inter, ".sa.m.f1.val.l1.e1 selection range 0 end; focus .sa.m.f1.val.l1.e1; .sa.m.f1.rd.r1 invoke");	// speed-up data entry focusing first data field

here_setall:

// update current linked variables values
cmd( inter, "write_any .sa.m.f1.val.l1.e1 $value1" ); 
cmd( inter, "write_any .sa.m.f1.val.l2.e2 $value2" );
cmd( inter, "write_any .sa.m.f2.s.l.a.e $step_in" ); 
cmd( inter, "write_any .sa.m.f2.s.sel2.c.from $cases_from" ); 
cmd( inter, "write_any .sa.m.f2.s.sel2.c.to $cases_to" ); 
cmd( inter, "write_any .sa.m.f2.rnd.le.s.e1 $rnd_seed" ); 

*choice = 0;
while(*choice==0)
  Tcl_DoOneEvent(0);

if(*choice==9)
{//search instance from
 i=compute_copyfrom(original, choice);
 sprintf(msg, "set cases_from %d",i);
 cmd(inter, msg);
 goto here_setall;
}

if(*choice==10)
{
 //search instance to
 i=compute_copyfrom(original, choice);
 sprintf(msg, "set cases_to %d",i);
 cmd(inter, msg);
 goto here_setall;
}

// save current linked variables values before closing
cmd( inter, "set value1 [ .sa.m.f1.val.l1.e1 get ]" ); 
cmd( inter, "set value2 [ .sa.m.f1.val.l2.e2 get ]" ); 
cmd( inter, "set cases_from [ .sa.m.f2.s.sel2.c.from get ]" ); 
cmd( inter, "set cases_to [ .sa.m.f2.s.sel2.c.to get ]" ); 
cmd( inter, "set step_in [ .sa.m.f2.s.l.a.e get ]" ); 
cmd( inter, "set rnd_seed [ .sa.m.f2.rnd.le.s.e1 get ]" ); 

cmd( inter, "destroytop .sa" );

if( (*choice==1 && res!=0) || *choice==9 || *choice==10)
{
cmd(inter, "set choice $use_seed");
if(*choice==1)
 {
  cmd(inter, "set choice $rnd_seed");
  init_random(*choice);
 }
cmd(inter, "set choice $to_all");
to_all=*choice;
cmd(inter, "set choice $cases_from");
cases_from=*choice;
cmd(inter, "set choice $cases_to");
cases_to=*choice;
cmd(inter, "set choice $update_d");
update_description=*choice;

switch(res)
{
//Equal 
case 1:
     cmd(inter, "set choice $fill");
     fill=*choice;
     cmd(inter, "set choice $step_in");
      
     for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label), i++)
      {
      if((to_all==1 || (cases_from<=i && cases_to>=i)) && (fill==1 || ((i-cases_from)%(*choice)==0)))
       {
        cv=cur->search_var(NULL, lab);
  			cv->data_loaded='+';
  			if(cv->param==0)
  			  cv->val[lag]=value1;
  			else
  			  cv->val[0]=value1;
       }
		  }
      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 
    
      if(to_all==1)
        {sprintf(msg, "All %d instances equal to %g.", i-1, value1);
         change_descr_lab(lab, "", "", "", msg);      
        } 
      else
        {
         if(cd->init!=NULL)
           sprintf(msg, "%s Instances from %d to %d equal to %g.",cd->init, cases_from, cases_to, value1);
         else
           sprintf(msg, "Instances from %d to %d equal to %g.", cases_from, cases_to, value1);  
         change_descr_lab(lab, "", "", "", msg);
        }  
       } 
  		unsaved_change( true );		// signal unsaved change
		  break;

//Range
case 9:
     cmd(inter, "set choice $fill");
     fill=*choice;
     cmd(inter, "set choice $step_in");
     counter=-1;
     for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label), i++)
      {
      if((to_all==1 || (cases_from<=i && cases_to>=i)) && (((i-cases_from)%(*choice)==0)))
       {
         //here the counter
         counter++;
       }
		  }
     value=(value2-value1)/counter;
     counter=0; 
     for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label), i++)
      {
      if((to_all==1 || (cases_from<=i && cases_to>=i)) && (fill==1 || ((i-cases_from)%(*choice)==0)))
       {
        cv=cur->search_var(NULL, lab);
  			cv->data_loaded='+';
  			if(cv->param==0)
  			  cv->val[lag]=value1+value*counter;
  			else
  			  cv->val[0]=value1+value*counter;
       }
      if(i>=cases_from && ((i-cases_from+1)% (*choice))==0)
        counter++; 

		  }
      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 
      
      if(to_all==1)
        {sprintf(msg, "All %d instances set ranging from %g to %g (i.e. increments of %g).", i-1, value1, value2, value);
         change_descr_lab(lab, "", "", "", msg);      
        } 
      else
        {
         if(cd->init!=NULL)
           sprintf(msg, "%s Instances from %d to %d ranging from %g to %g (i.e. increments of %g).",cd->init, cases_from, cases_to, value1, value2, value);
         else
           sprintf(msg, "Instances from %d to %d ranging from %g to %g (i.e. increments of %g).", cases_from, cases_to, value1, value2, value);  
         change_descr_lab(lab, "", "", "", msg);
        }  
       } 
  		unsaved_change( true );		// signal unsaved change
		  break;

case 2: //increasing
     
     cv=r->search_var(NULL, lab);
  	  cv->data_loaded='+';
     cmd(inter, "set choice $fill");
     fill=*choice;
     cmd(inter, "set choice $step_in");
     
      for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label), i++)
        {
        if((to_all==1 || (cases_from<=i && cases_to>=i)) && (fill==1 || ((i-cases_from)%(*choice)==0)))
        {
         cv=cur->search_var(NULL, lab);
         cv->data_loaded='+';
         if(cv->param==0)
           cv->val[lag]=value1 +step*value2;
         else
           cv->val[0]=value1 +step*value2;
        }
        if(i>=cases_from && ((i-cases_from+1)% (*choice))==0)
          step+=1;
        }
      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 
      
      if(to_all==1)
        {sprintf(msg, "All %d instances increasing from %g with steps %g. The value is increased every %d instances.", i-1, value1, value2, *choice);
         change_descr_lab(lab, "", "", "", msg);
        }
      else
        {
         if(cd->init!=NULL)
           sprintf(msg, "%s Instances from %d to %d increasing from %g with steps %g. The value is increased every %d instances.",cd->init, cases_from, cases_to, value1, value2, *choice);
         else
           sprintf(msg, "Instances from %d to %d increasing from %g with steps %g. The value is increased every %d instances.", cases_from, cases_to, value1, value2, *choice);            
         change_descr_lab(lab, "", "", "", msg);
        }  
        }
  		unsaved_change( true );		// signal unsaved change
        break;
		
case 4: 
		cv=r->search_var(NULL, lab);
		  for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label), i++)
        {
      if(to_all==1 || (cases_from<=i && cases_to>=i))
      {
        cv=cur->search_var(NULL, lab);
        cv->data_loaded='+';
        if(cv->param==0)
           cv->val[lag]=value1 +step*value2;
		   	else
           cv->val[0]=value1 +step*value2;
        if(cur->next!=cur->hyper_next(r->label))
          step=-1;
        step+=1;

        }
       } 

      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 
      
      if(to_all==1)
        {sprintf(msg, "All %d instances increasing from %g with steps %g re-starting for each group of objects.", i-1, value1, value2);
         change_descr_lab(lab, "", "", "", msg);      
        } 
      else
        {
         if(cd->init!=NULL)
           sprintf(msg, "%s Instances from %d to %d increasing from %g with steps %g re-starting for each group of objects.",cd->init, cases_from, cases_to, value1, value2);
         else
           sprintf(msg, "Instances from %d to %d increasing from %g with steps %g re-starting for each group of objects.", cases_from, cases_to, value1, value2);
           
         change_descr_lab(lab, "", "", "", msg);        
        }  
       }
  		unsaved_change( true );		// signal unsaved change

        break;

case 3: 
     cmd(inter, "set choice $fill");
     fill=*choice;
     cmd(inter, "set choice $step_in");
      
     for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label), i++)
      {
      if((to_all==1 || (cases_from<=i && cases_to>=i)) && (fill==1 || ((i-cases_from)%(*choice)==0)))
       {
       cv=cur->search_var(NULL, lab);
       cv->data_loaded='+';
       if(cv->param==0)
 			  cv->val[lag]=value1+RND*(value2-value1);
 			else
 			  cv->val[0]=value1+RND*(value2-value1);
       }
      }

      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 
      
      if(to_all==1)
        {sprintf(msg, "All %d instances set to random values drawn from a uniform in the range [%g,%g].", i-1, value1, value2);
         change_descr_lab(lab, "", "", "", msg);      
        } 
      else
        {

         if(cd->init!=NULL)
           sprintf(msg, "%s Instances from %d to %d set to random values drawn from a uniform in the range [%g,%g].", cd->init, cases_from, cases_to, value1, value2);
         else
           sprintf(msg, "Instances from %d to %d set to random values drawn from a uniform in the range [%g,%g].", cases_from, cases_to, value1, value2);         
         change_descr_lab(lab, "", "", "", msg);        
        }  
       }

  		unsaved_change( true );		// signal unsaved change

		  break;
		  
case 5: 
     cmd(inter, "set choice $fill");
     fill=*choice;
     cmd(inter, "set choice $step_in");
      
     for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label), i++)
      {
      if((to_all==1 || (cases_from<=i && cases_to>=i)) && (fill==1 || ((i-cases_from)%(*choice)==0)))
       {
			cv=cur->search_var(NULL, lab);
         cv->data_loaded='+';
			if(cv->param==0)
			  cv->val[lag]=norm(value1, value2);
			else
			  cv->val[0]=norm(value1, value2);
		  }
      }

      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 
      
      if(to_all==1)
        {sprintf(msg, "All %d instances set to random values drawn from a normal with mean=%g and std. deviation=%g.", i-1, value1, value2);
         change_descr_lab(lab, "", "", "", msg);      
        } 
      else
        {

         if(cd->init!=NULL)
           sprintf(msg, "%s Instances from %d to %d set to random values drawn from a normal with mean=%g and std. deviation=%g",cd->init, cases_from, cases_to, value1, value2);
         else
           sprintf(msg, "Instances from %d to %d set to random values drawn from a normal with mean=%g and std. deviation=%g", cases_from, cases_to, value1, value2);         
           change_descr_lab(lab, "", "", "", msg);        
        }  
       }
  		unsaved_change( true );		// signal unsaved change

		  break;

case 6: 
		cv=r->search_var(NULL, lab);
		cv->data_loaded='+';
        for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label),i++)
        {
       if(to_all==1 || (cases_from<=i && cases_to>=i))
        {
         cv=cur->search_var(NULL, lab);
         cv->data_loaded='+';
         if(step==value2)
          {if(cv->param==0)
            cv->val[lag]=value1;
           else
            cv->val[0]=value1;
           step=0;
          }
         else
           step++;

        }
        }

      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 
      
      if(to_all==1)
        {sprintf(msg, "All %d instances set to %g skipping %g instances.", i-1, value1, value2);
         change_descr_lab(lab, "", "", "", msg);      
        } 
      else
        {
         if(cd->init!=NULL)
           sprintf(msg, "%s Instances from %d to %d set to %g skipping %g instances.",cd->init, cases_from, cases_to, value1, value2);
         else
           sprintf(msg, "Instances from %d to %d set to %g skipping %g instances.", cases_from, cases_to, value1, value2);         
         change_descr_lab(lab, "", "", "", msg);        
        }  
       }
  		unsaved_change( true );		// signal unsaved change

        break;


case 7:
  cmd(inter, "set oldpath [pwd]");
  cmd( inter, "set filename [ tk_getOpenFile -parent . -title \"File to Import Data\" -filetypes { { { Text Files } { .txt } } { { All Files } { * } }} ]" );
  l=(char *)Tcl_GetVar(inter, "filename",0);
  if(l!=(char *)NULL && strcmp(l, ""))
 { cmd(inter, "cd [file dirname $filename]");
   cmd(inter, "set fn [file tail $filename]");
   l=(char *)Tcl_GetVar(inter, "fn",0);
   f=fopen(l, "r");
   cmd(inter, "cd $oldpath");
   if(f!=NULL)
    {
    fscanf(f, "%99s", ch); //the label
    kappa=fscanf(f, "%lf", &value);
    for(i=1,cur=r; cur!=NULL && kappa!=EOF; cur=cur->hyper_next(r->label),i++)
		  {
      if(to_all==1 || (cases_from<=i && cases_to>=i))
      {
      cv=cur->search_var(NULL, lab);
			cv->data_loaded='+';
			if(cv->param==0)
			  cv->val[lag]=value;
			else
			  cv->val[0]=value;
      kappa=fscanf(f, "%lf", &value);
		  }
     }
     if(cur!=NULL || kappa!=EOF)
      plog("\nWarning: problem loading data, the file may contain a different number\nof values compared to the objects to initialize");
      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 
      
      if(to_all==1)
        {sprintf(msg, "All %d instances set with data from file %s.", i-1, l);
         change_descr_lab(lab, "", "", "", msg);      
        } 
      else
        {

         if(cd->init!=NULL)
         sprintf(msg, "%s Instances from %d to %d with data from file %s", cd->init, cases_from, cases_to, l);
         else
           sprintf(msg, "Instances from %d to %d with data from file %s", cases_from, cases_to, l);         
         change_descr_lab(lab, "", "", "", msg);        
        }  
       }

    
    }
  unsaved_change( true );		// signal unsaved change
   }
break;

case 8:
     cmd(inter, "set choice $fill");
     fill=*choice;
     cmd(inter, "set choice $step_in");
      
     for(i=1,cur=r, step=0; cur!=NULL; cur=cur->hyper_next(r->label), i++)
      {
      if((to_all==1 || (cases_from<=i && cases_to>=i)) && (fill==1 || ((i-cases_from)%(*choice)==0)))
       {
       cv=cur->search_var(NULL, lab);
       cv->data_loaded='+';
       if(cv->param==0)
 			  cv->val[lag]=rnd_integer(value1,value2);
 			else
 			  cv->val[0]=rnd_integer(value1,value2);
       }
      }

      if( update_description==1)
      {
      cd=search_description(lab);
    
      if(cd==NULL)
      {  cv=r->search_var(NULL, lab);
       if(cv->param==0)
         add_description(lab, "Variable", "(no description available)");
       if(cv->param==1)
         add_description(lab, "Parameter", "(no description available)");  
       if(cv->param==2)
         add_description(lab, "Function", "(no description available)");  
       sprintf(msg, "\nWarning: description for '%s' not found. New one created.", lab);
       plog(msg);
       cd=search_description(lab);
      } 

      if(to_all==1)
        {sprintf(msg, "All %d instances set to integer random values drawn from a uniform in the range [%g,%g].", i-1, value1, value2);
         change_descr_lab(lab, "", "", "", msg);      
        } 
      else
        {

         if(cd->init!=NULL)
           sprintf(msg, "%s Instances from %d to %d set to integer random values drawn from a uniform in the range [%g,%g].", cd->init, cases_from, cases_to, value1, value2);
         else
           sprintf(msg, "Instances from %d to %d set to integer random values drawn from a uniform in the range [%g,%g].", cases_from, cases_to, value1, value2);         
         change_descr_lab(lab, "", "", "", msg);        
        }  
       }

  		unsaved_change( true );		// signal unsaved change

		  break;


default:
		sprintf( msg, "invalid option in set_all" );
		error_hard( msg, "Internal error", "If error persists, please contact developers." );
		myexit(22);
}
}

Tcl_UnlinkVar(inter, "value1");
Tcl_UnlinkVar(inter, "value2");
Tcl_UnlinkVar(inter, "res");
}


object *sensitivity_parallel(object *o, sense *s )
{
/*
This function fills the initial values according to the sensitivity analysis system performed by parallel simulations: 1 single run over many independent configurations
descending in parallel from Root.

In set all users can set one or more elements to be part of the sensitivity analysis. For each element the user has to provide the number of values to be explored and their values.
When all elements involved in the sensitivity analysis are configured, the user must launch the command Sensitivity from menu Data in the main Lsd Browser. This command generates
as many copies as the product of all values for all elements in the s.a. It then kicks off the initialization of all elements involved so that each combination of parameters is assigned to one branch of the model.

The user is supposed then to save the resulting configuration.

Options concerning initialization for sensitivity analysis are not saved into the model configuration files, and are therefore lost when closing the LSD model program. 

*/
int i;
sense *cs;
object *cur;
variable *cvar;

cur=o;

if(s->next!=NULL)
 {
  for(i=0; i<s->nvalues; i++)
   {
    s->i=i;
    cur=sensitivity_parallel(cur,s->next);
   }
 return cur;
 }


for(i=0; i<s->nvalues; i++)
  {
   s->i=i;
   for (cs=rsense; cs!=NULL; cs=cs->next) 
   {
    cvar=cur->search_var(cur, cs->label);
	if(cs->param==1)				// handle lags > 0
	  cvar->val[0]=cs->v[cs->i];
	else
      cvar->val[cs->lag]=cs->v[cs->i];
   }
   cur=cur->hyper_next(cur->label);
  }

return cur;
 
}


void sensitivity_sequential(int *findex, sense *s, double probSampl)
{
/*
This function fills the initial values according to the sensitivity analysis system performed by sequential simulations: each run executes one configuration labelled with sequential labels.

Contrary to parallel sensitivity settings, this function initialize all elements in the configuration with the specified label.

In set all users can set one or more elements to be part of the sensitivity analysis. For each element the user has to provide the number of values to be explored and their values.
When all elements involved in the sensitivity analysis are configured, the user must launch the command Sensitivity from menu Data in the main Lsd Browser.

Options concerning initialization for sensitivity analysis are saved into model configuration files, to be executed with a No Window version of the LSD model. 
One configuration file is created for each possible combination of the sensitivity analysis values (parameters and initial conditions). Optionally, it is possible
to define the parameter "probSampl" with the (uniform) probability of a given point in the sensitivity analysis space is saved as configuration file. In practice,
this allows for the Monte Carlo sampling of the parameter space, which is often necessary when the s.a. space is too big to be analyzed in its entirety.

*/
int i,nv;
sense *cs;
object *cur;
variable *cvar;
char *fname;
FILE *f;


description *cur_descr; 
if(s->next!=NULL)
 {
  for(i=0; i<s->nvalues; i++)
   {
    s->i=i;
    sensitivity_sequential(findex,s->next,probSampl);
   }
 return;
 }


for(i=0; i<s->nvalues; i++)
{
   s->i=i;
   for (nv=1,cs=rsense; cs!=NULL; cs=cs->next) 
   {
    nv*=cs->nvalues;
    cvar=root->search_var(root, cs->label);
    for(cur=cvar->up; cur!=NULL; cur=cur->hyper_next(cur->label) )
    {
      cvar=cur->search_var(cur, cs->label); 
	  if(cs->param==1)				// handle lags > 0
		cvar->val[0]=cs->v[cs->i];
	  else
        cvar->val[cs->lag]=cs->v[cs->i];
    }

   }

 if(probSampl == 1.0 || RND <= probSampl)		// if required draw if point will be sampled
 {
	if ( ! save_configuration( root, *findex ) )
	{
		cmd( inter , "tk_messageBox -parent . -type ok -icon error -title Error -message \"Configuration files cannot be saved\" -detail \"Check if the drive or the current directory is set READ-ONLY, select a drive/directory with write permission and try again.\"" );
		return;
	}
	*findex=*findex+1;
 }
}
 
}


// calculates the sensitivity space size
long num_sensitivity_points( sense *rsens )	
{
	long nv;
	sense *cs;
	for ( nv = 1, cs = rsens; cs != NULL; cs = cs->next )	// scan the linked-list
		nv *= cs->nvalues;	// update the number of variables
	return nv;
}


// calculates the number of variables to test
int num_sensitivity_variables( sense *rsens )	
{
	int nv;
	sense *cs;
	for ( nv = 0, cs = rsens; cs != NULL; cs = cs->next)								
		if ( cs->nvalues > 1 )				// count variables with 2 or more values
			nv++;
	return nv;
}

			
// try to get values for sensitivity analysis (true: values are ok)

#define SEP	" ,;|/#\t\n"					// sensitivity data valid separators

void dataentry_sensitivity(int *choice, sense *s, int nval = 0)
{

int i, nPar, samples;
double start, end;
char *lab, *sss = NULL, *tok = NULL, type;
FILE *f;

*choice=0;
cmd(inter, "set sens .sens");
cmd(inter, "newtop .sens \"Sensitivity Analysis\" { set choice 2 }");

if ( nval > 0)								// number of values defined (0=no)?
	sprintf(msg, "label .sens.lab -text \"Enter n=%d values for \'%s\' (most separators accepted)\n\nUse \'=BEGIN:END@SAMPLES%%TYPE\' clause to specify a number of samples within a range\nSpaces are not allowed within clauses - TYPE values: \'L\' for linear and \'R\' for random samples\"",s->nvalues,s->label);
else
	sprintf(msg, "label .sens.lab -text \"Enter the desired values (at least 2) for \'%s\' (most separators accepted)\n\nUse \'=BEGIN:END@SAMPLES%%TYPE\' clause to specify a number of samples within a range\nSpaces are not allowed within clauses - TYPE values: \'L\' for linear and \'R\' for random samples\"",s->label);
cmd(inter, msg);
cmd(inter, "pack .sens.lab -pady 5");
cmd(inter, "text .sens.t; pack .sens.t"); 
cmd(inter, "frame .sens.fb");
cmd(inter, "button .sens.fb.paste -width -9 -text \"Paste Clipboard\" -command {tk_textPaste .sens.t}");
cmd(inter, "button .sens.fb.del -width -9 -text \"Delete Values\" -command {.sens.t delete 0.0 end}");
cmd(inter, "button .sens.fb.rem -width -9 -text \"Remove from Analysis\" -command {set choice 3}");
cmd(inter, "pack .sens.fb.paste .sens.fb.del .sens.fb.rem -padx 10 -pady 10 -side left");
cmd(inter, "pack .sens.fb");
cmd( inter, "okcancel .sens fb2 { set choice 1 } { set choice 2 }" );
cmd(inter, "bind .sens.fb2.ok <KeyPress-Return> {set choice 1}");
cmd(inter, "focus .sens.t");

if ( s->entryOk )	// is there valid data from a previous data entry?
{
	sss = new char[ 26 * s->nvalues + 1];	// allocate space for string
	tok = new char[ 26 + 1 ];				
	strcpy( sss, "" );
	for ( i = 0; i < s->nvalues; i++ )		// pass existing data as a string
	{
		sprintf( tok, "%.15g ", s->v[i] );	// add each value
		strcat( sss, tok );					// to the string
	}
	Tcl_SetVar( inter, "sss", sss, 0 ); 	// pass string to Tk window
	cmd(inter, ".sens.t insert 0.0 $sss");		// insert string in entry window
	delete [] tok; 
	delete [] sss;
}

cmd( inter, "showtop .sens topleftW" );

do			// finish only after reading all values
{

while(*choice==0)
  Tcl_DoOneEvent(0);

if ( *choice == 3 )	// force error to delete variable from list
{
	s->entryOk = false;
	*choice = 2; 
}

if(*choice==2)
 {
  cmd( inter, "destroytop .sens" );
  return; 
 }

cmd(inter, "set sss [.sens.t get 0.0 end]");
sss=(char*)Tcl_GetVar(inter,"sss",0);

if ( nval == 0 )					// undefined number of values?
{	
	double temp;
	char *tss, *ss = new char[ strlen( sss ) + 1 ];
	tss = ss;						// save original pointer to gc
	strcpy( ss, sss );				// make a draft copy
	
	i = 0;							// count number of values
	do
	{
		tok = strtok( ss, SEP );	// accepts several separators
		if ( tok == NULL )			// finished?
			break;
		ss = NULL;
		
		// is it a clause to be expanded?
		nPar = sscanf( tok, "=%lf:%lf@%u%%%c", &start, &end, &samples, &type );
		if ( nPar == 4 )			// all values are required
			i += samples;			// samples to create
		else						// no, read as regular double float
			i += sscanf( tok, "%lf", &temp );	// count valid doubles only
	 }
	 while ( tok != NULL );
	 
	 if ( i < 2 )					// invalid number of elements?
		i = 2;						// minimum is 2
		
	 if ( s->nvalues < i )			// is there insufficient space already alloc'd?
	 {
		 delete [ ] s->v;			// free old and reallocate enough space
		 s->v = new double[ i ];
	 }
	 s->nvalues = i;				// update # of values
	 
	 delete [ ] tss;
}

for(i=0; i<s->nvalues;)
 {
  tok=strtok(sss, SEP);		// accepts several separators
  if(tok==NULL)		// finished too early?
  {
	  cmd(inter, "tk_messageBox -parent . -title \"Sensitivity Analysis\" -icon error -type ok -message \"Less values than required\" -detail \"Please insert the correct number of values.\"");
	  *choice=0;
	  cmd(inter, "focus .sens.t");
	  break;
  }
  sss=NULL;
  // is it a clause to be expanded?
  nPar = sscanf( tok, "=%lf:%lf@%u%%%c", &start, &end, &samples, &type );
  if ( nPar == 4 )								// all values are required
  {
	if ( toupper( type ) == 'L' && samples > 0 )// linear sampling
	{
		s->v[ i++ ] = fmin( start, end );
		for ( int j = 1; j < samples; ++j, ++i )
			s->v[ i ] = s->v[ i - 1 ] + ( fmax( start, end ) - fmin( start, end ) ) / ( samples - 1 );
	}
	if ( toupper( type ) == 'R' && samples > 0 )// random sampling 
		for ( int j = 0; j < samples; ++j, ++i )
			s->v[ i ] = fmin( start, end ) + RND * ( fmax( start, end ) - fmin( start, end ) );
  }
  else											// no, read as regular double float
	i += sscanf( tok, "%lf", &( s->v[ i ] ) );	// count valid doubles only
 }
}
while( tok == NULL || i < 2 );	// require enough values (if more, extra ones are discarded)
	
s->entryOk = true;	// flag valid data

cmd( inter, "destroytop .sens" );
}


/*
Deallocate sensitivity analysis memory
*/
void empty_sensitivity(sense *cs)
{
	if(cs==NULL)			// prevent invalid calls (last variable)
		return;
	
	if(cs->next!=NULL)		// recursively start from the end of the list
		empty_sensitivity(cs->next);
	else
		NOLH_clear( );		// deallocate DoE (last object only)
	
	if(cs->v!=NULL)			// deallocate requested memory, if applicable
		delete cs->v;
	if(cs->label!=NULL)
		delete cs->label;

	delete cs;				// suicide
}


/*
	Calculate a Near Orthogonal Latin Hypercube (NOLH) design for sampling.
	Include tables to up to 29 variables (Sanchez 2009, Cioppa and Lucas 2007).
	Returns the number of samples (n) required for the calculated design and a pointer
	to the matrix n x k, where k is the number of factors (variables).

	It is possible to load one additional design table from disk (file NOLH.csv in
	the same folder as the configuration file .lsd). The table should be formed
	by positive integers only, in the n (rows) x k (columns), separated by commas,
	one row per text line and no empty lines. The table can be loaded manually
	(NOLH_load function) or automatically as needed during sampling (NOLH_sampler).
*/

// NOLH tables (Sanchez 2009 + Cioppa 2002)
int NOLH_1[][7] = { {6, 17, 14, 7, 5, 16, 10}, {2, 5, 15, 10, 1, 6, 11}, {3, 8, 2, 5, 11, 14, 17}, {4, 11, 6, 17, 10, 3, 13}, {13, 16, 8, 3, 6, 1, 14},
{17, 6, 7, 14, 2, 13, 15}, {11, 4, 17, 6, 15, 8, 16}, {10, 15, 13, 16, 14, 11, 12}, {9, 9, 9, 9, 9, 9, 9}, {12, 1, 4, 11, 13, 2, 8}, {16, 13, 3, 8, 17, 12, 7},
{15, 10, 16, 13, 7, 4, 1}, {14, 7, 12, 1, 8, 15, 5}, {5, 2, 10, 15, 12, 17, 4}, {1, 12, 11, 4, 16, 5, 3}, {7, 14, 1, 12, 3, 10, 2}, {8, 3, 5, 2, 4, 7, 6},
{14, 10, 5, 16, 6, 17, 7}, {15, 11, 1, 6, 2, 5, 10}, {2, 17, 11, 14, 3, 8, 5}, {6, 13, 10, 3, 4, 11, 17}, {8, 14, 6, 1, 13, 16, 3}, {7, 15, 2, 13, 17, 6, 14}, 
{17, 16, 15, 8, 11, 4, 6}, {13, 12, 14, 11, 10, 15, 16}, {4, 8, 13, 2, 12, 1, 11}, {3, 7, 17, 12, 16, 13, 8}, {16, 1, 7, 4, 15, 10, 13}, {12, 5, 8, 15, 14, 7, 1}, 
{10, 4, 12, 17, 5, 2, 15}, {11, 3, 16, 5, 1, 12, 4}, {1, 2, 3, 10, 7, 14, 12}, {5, 6, 4, 7, 8, 3, 2} };

int NOLH_2[][11] = { {15,7,29,23,16,33,23,21,4,20,33}, {5, 13, 16, 25, 11, 30, 15, 7, 33, 28, 30}, {30, 6, 2, 24, 2, 11, 32, 20, 15, 13, 29}, 
{33, 14, 31, 27, 3, 15, 4, 6, 29, 8, 19}, {16, 8, 23, 14, 19, 3, 5, 24, 2, 23, 31}, {11, 10, 15, 6, 29, 2, 18, 8, 31, 25, 32}, {32, 9, 1, 13, 30, 29, 1, 22, 16, 10, 23}, 
{31, 12, 30, 8, 33, 18, 31, 9, 23, 7, 18}, {8, 18, 24, 1, 7, 20, 20, 11, 9, 3, 22}, {10, 23, 8, 4, 13, 28, 7, 18, 22, 2, 25}, {25, 32, 12, 5, 6, 13, 21, 3, 8, 22, 24}, 
{22, 31, 25, 16, 14, 8, 6, 32, 24, 19, 26}, {7, 19, 20, 32, 26, 12, 10, 5, 6, 1, 20}, {13, 29, 6, 31, 24, 9, 26, 19, 20, 5, 28}, {28, 30, 13, 22, 25, 24, 12, 1, 7, 30, 21}, 
{20, 33, 27, 19, 22, 27, 25, 30, 21, 18, 27}, {17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17}, {19, 27, 5, 11, 18, 1, 11, 13, 30, 14, 1}, {29, 21, 18, 9, 23, 4, 19, 27, 1, 6, 4}, 
{4, 28, 32, 10, 32, 23, 2, 14, 19, 21, 5}, {1, 20, 3, 7, 31, 19, 30, 28, 5, 26, 15}, {18, 26, 11, 20, 15, 31, 29, 10, 32, 11, 3}, {23, 24, 19, 28, 5, 32, 16, 26, 3, 9, 2}, 
{2, 25, 33, 21, 4, 5, 33, 12, 18, 24, 11}, {3, 22, 4, 26, 1, 16, 3, 25, 11, 27, 16}, {26, 16, 10, 33, 27, 14, 14, 23, 25, 31, 12}, {24, 11, 26, 30, 21, 6, 27, 16, 12, 32, 9}, 
{9, 2, 22, 29, 28, 21, 13, 31, 26, 12, 10}, {12, 3, 9, 18, 20, 26, 28, 2, 10, 15, 8}, {27, 15, 14, 2, 8, 22, 24, 29, 28, 33, 14}, {21, 5, 28, 3, 10, 25, 8, 15, 14, 29, 6}, 
{6, 4, 21, 12, 9, 10, 22, 33, 27, 4, 13}, {14, 1, 7, 15, 12, 7, 9, 4, 13, 16, 7}, {29, 4, 33, 7, 21, 15, 20, 23, 33, 16, 23}, {16, 33, 30, 13, 7, 5, 28, 15, 30, 11, 25}, 
{2, 15, 11, 6, 20, 30, 13, 32, 29, 2, 24}, {31, 29, 15, 14, 6, 33, 8, 4, 19, 3, 27}, {23, 2, 3, 8, 24, 16, 23, 5, 31, 19, 14}, {15, 31, 2, 10, 8, 11, 25, 18, 32, 29, 6}, 
{1, 16, 29, 9, 22, 32, 10, 1, 23, 30, 13}, {30, 23, 18, 12, 9, 31, 7, 31, 18, 33, 8}, {24, 9, 20, 18, 11, 8, 3, 20, 22, 7, 1}, {8, 22, 28, 23, 18, 10, 2, 7, 25, 13, 4}, 
{12, 8, 13, 32, 3, 25, 22, 21, 24, 6, 5}, {25, 24, 8, 31, 32, 22, 19, 6, 26, 14, 16}, {20, 6, 12, 19, 5, 7, 1, 10, 20, 26, 32}, {6, 20, 9, 29, 19, 13, 5, 26, 28, 24, 31}, 
{13, 7, 24, 30, 1, 28, 30, 12, 21, 25, 22}, {27, 21, 27, 33, 30, 20, 18, 25, 27, 22, 19}, {5, 30, 1, 27, 13, 19, 14, 11, 1, 18, 11}, {18, 1, 4, 21, 27, 29, 6, 19, 4, 23, 9}, 
{32, 19, 23, 28, 14, 4, 21, 2, 5, 32, 10}, {3, 5, 19, 20, 28, 1, 26, 30, 15, 31, 7}, {11, 32, 31, 26, 10, 18, 11, 29, 3, 15, 20}, {19, 3, 32, 24, 26, 23, 9, 16, 2, 5, 28}, 
{33, 18, 5, 25, 12, 2, 24, 33, 11, 4, 21}, {4, 11, 16, 22, 25, 3, 27, 3, 16, 1, 26}, {10, 25, 14, 16, 23, 26, 31, 14, 12, 27, 33}, {26, 12, 6, 11, 16, 24, 32, 27, 9, 21, 30}, 
{22, 26, 21, 2, 31, 9, 12, 13, 10, 28, 29}, {9, 10, 26, 3, 2, 12, 15, 28, 8, 20, 18}, {14, 28, 22, 15, 29, 27, 33, 24, 14, 8, 2}, {28, 14, 25, 5, 15, 21, 29, 8, 6, 10, 3}, 
{21, 27, 10, 4, 33, 6, 4, 22, 13, 9, 12}, {7, 13, 7, 1, 4, 14, 16, 9, 7, 12, 15} };

int NOLH_3[][16] = { {47, 24, 22, 9, 50, 52, 32, 47, 36, 61, 63, 4, 13, 41, 53, 45}, {62, 8, 28, 23, 17, 36, 49, 60, 50, 32, 47, 47, 16, 23, 23, 11}, {58, 62, 15, 20, 56, 11, 30, 38, 52, 57, 27, 24, 31, 34, 18, 58}, 
{42, 47, 30, 5, 28, 18, 18, 64, 59, 41, 9, 58, 6, 14, 56, 22}, {60, 13, 2, 7, 11, 17, 40, 32, 5, 55, 40, 31, 42, 39, 46, 52}, {35, 16, 32, 11, 43, 10, 56, 8, 27, 51, 58, 60, 58, 5, 12, 4}, 
{50, 35, 3, 17, 22, 58, 28, 29, 23, 59, 11, 13, 62, 48, 7, 36}, {53, 60, 21, 25, 60, 46, 2, 21, 2, 42, 17, 50, 47, 3, 36, 13}, {45, 2, 53, 27, 40, 26, 7, 20, 55, 26, 52, 3, 22, 16, 34, 65}, 
{63, 32, 50, 1, 19, 60, 25, 1, 44, 3, 44, 45, 28, 55, 2, 31}, {34, 63, 35, 14, 29, 21, 45, 16, 47, 13, 2, 2, 15, 4, 16, 54}, {64, 45, 60, 12, 5, 25, 53, 25, 35, 17, 21, 34, 30, 38, 60, 15}, 
{36, 22, 42, 26, 32, 13, 19, 62, 10, 4, 62, 15, 64, 12, 41, 48}, {51, 28, 58, 18, 63, 4, 1, 35, 17, 20, 34, 36, 34, 44, 26, 26}, {38, 51, 62, 29, 15, 31, 61, 54, 1, 29, 10, 22, 63, 20, 38, 56}, 
{44, 36, 47, 10, 62, 65, 37, 53, 24, 8, 30, 38, 45, 64, 55, 29}, {56, 26, 9, 44, 65, 51, 63, 39, 21, 19, 38, 29, 27, 42, 44, 25}, {37, 18, 23, 38, 30, 54, 52, 48, 8, 1, 24, 56, 1, 6, 24, 46}, 
{48, 37, 20, 51, 58, 3, 27, 52, 32, 6, 35, 26, 14, 57, 3, 34}, {40, 56, 5, 36, 12, 29, 12, 57, 4, 28, 53, 48, 12, 17, 31, 47}, {54, 27, 7, 64, 25, 2, 46, 7, 28, 12, 20, 14, 40, 53, 65, 27}, 
{52, 1, 11, 34, 46, 32, 58, 23, 60, 21, 12, 54, 48, 10, 29, 64}, {65, 52, 17, 63, 9, 47, 9, 22, 37, 18, 37, 27, 37, 36, 14, 17}, {39, 54, 25, 45, 35, 38, 15, 5, 46, 14, 65, 65, 56, 15, 61, 60}, 
{41, 7, 39, 53, 48, 61, 11, 11, 18, 43, 7, 17, 9, 1, 58, 24}, {49, 11, 65, 50, 13, 42, 24, 30, 3, 56, 25, 41, 23, 37, 4, 61}, {55, 49, 52, 35, 59, 22, 62, 17, 15, 39, 50, 7, 20, 8, 27, 9}, 
{59, 41, 54, 60, 21, 9, 43, 24, 25, 64, 48, 55, 5, 47, 45, 38}, {61, 9, 40, 42, 14, 27, 6, 56, 57, 36, 15, 20, 59, 21, 47, 7}, {46, 23, 48, 58, 64, 23, 35, 51, 40, 44, 5, 61, 55, 40, 17, 43}, 
{43, 46, 37, 62, 27, 50, 50, 63, 53, 35, 60, 9, 49, 7, 9, 16}, {57, 61, 56, 47, 42, 59, 44, 40, 54, 50, 43, 43, 41, 35, 51, 63}, {33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33}, 
{19, 42, 44, 57, 16, 14, 34, 19, 30, 5, 3, 62, 53, 25, 13, 21}, {4, 58, 38, 43, 49, 30, 17, 6, 16, 34, 19, 19, 50, 43, 43, 55}, {8, 4, 51, 46, 10, 55, 36, 28, 14, 9, 39, 42, 35, 32, 48, 8}, 
{24, 19, 36, 61, 38, 48, 48, 2, 7, 25, 57, 8, 60, 52, 10, 44}, {6, 53, 64, 59, 55, 49, 26, 34, 61, 11, 26, 35, 24, 27, 20, 14}, {31, 50, 34, 55, 23, 56, 10, 58, 39, 15, 8, 6, 8, 61, 54, 62}, 
{16, 31, 63, 49, 44, 8, 38, 37, 43, 7, 55, 53, 4, 18, 59, 30}, {13, 6, 45, 41, 6, 20, 64, 45, 64, 24, 49, 16, 19, 63, 30, 53}, {21, 64, 13, 39, 26, 40, 59, 46, 11, 40, 14, 63, 44, 50, 32, 1}, 
{3, 34, 16, 65, 47, 6, 41, 65, 22, 63, 22, 21, 38, 11, 64, 35}, {32, 3, 31, 52, 37, 45, 21, 50, 19, 53, 64, 64, 51, 62, 50, 12}, {2, 21, 6, 54, 61, 41, 13, 41, 31, 49, 45, 32, 36, 28, 6, 51}, 
{30, 44, 24, 40, 34, 53, 47, 4, 56, 62, 4, 51, 2, 54, 25, 18}, {15, 38, 8, 48, 3, 62, 65, 31, 49, 46, 32, 30, 32, 22, 40, 40}, {28, 15, 4, 37, 51, 35, 5, 12, 65, 37, 56, 44, 3, 46, 28, 10}, 
{22, 30, 19, 56, 4, 1, 29, 13, 42, 58, 36, 28, 21, 2, 11, 37}, {10, 40, 57, 22, 1, 15, 3, 27, 45, 47, 28, 37, 39, 24, 22, 41}, {29, 48, 43, 28, 36, 12, 14, 18, 58, 65, 42, 10, 65, 60, 42, 20}, 
{18, 29, 46, 15, 8, 63, 39, 14, 34, 60, 31, 40, 52, 9, 63, 32}, {26, 10, 61, 30, 54, 37, 54, 9, 62, 38, 13, 18, 54, 49, 35, 19}, {12, 39, 59, 2, 41, 64, 20, 59, 38, 54, 46, 52, 26, 13, 1, 39}, 
{14, 65, 55, 32, 20, 34, 8, 43, 6, 45, 54, 12, 18, 56, 37, 2}, {1, 14, 49, 3, 57, 19, 57, 44, 29, 48, 29, 39, 29, 30, 52, 49}, {27, 12, 41, 21, 31, 28, 51, 61, 20, 52, 1, 1, 10, 51, 5, 6}, 
{25, 59, 27, 13, 18, 5, 55, 55, 48, 23, 59, 49, 57, 65, 8, 42}, {17, 55, 1, 16, 53, 24, 42, 36, 63, 10, 41, 25, 43, 29, 62, 5}, {11, 17, 14, 31, 7, 44, 4, 49, 51, 27, 16, 59, 46, 58, 39, 57}, 
{7, 25, 12, 6, 45, 57, 23, 42, 41, 2, 18, 11, 61, 19, 21, 28}, {5, 57, 26, 24, 52, 39, 60, 10, 9, 30, 51, 46, 7, 45, 19, 59}, {20, 43, 18, 8, 2, 43, 31, 15, 26, 22, 61, 5, 11, 26, 49, 23}, 
{23, 20, 29, 4, 39, 16, 16, 3, 13, 31, 6, 57, 17, 59, 57, 50}, {9, 5, 10, 19, 24, 7, 22, 26, 12, 16, 23, 23, 25, 31, 15, 3}, {4, 50, 61, 22, 53, 41, 36, 45, 9, 13, 52, 24, 63, 47, 32, 47}, 
{47, 17, 32, 28, 23, 23, 50, 11, 23, 16, 36, 8, 47, 62, 49, 60}, {24, 56, 57, 15, 18, 34, 52, 58, 20, 31, 11, 62, 27, 58, 30, 38}, {58, 28, 41, 30, 56, 14, 59, 22, 5, 6, 18, 47, 9, 42, 18, 64}, 
{31, 11, 55, 2, 46, 39, 5, 52, 7, 42, 17, 13, 40, 60, 40, 32}, {60, 43, 51, 32, 12, 5, 27, 4, 11, 58, 10, 16, 58, 35, 56, 8}, {13, 22, 59, 3, 7, 48, 23, 36, 17, 62, 58, 35, 11, 50, 28, 29}, 
{50, 60, 42, 21, 36, 3, 2, 13, 25, 47, 46, 60, 17, 53, 2, 21}, {3, 40, 26, 53, 34, 16, 55, 65, 27, 22, 26, 2, 52, 45, 7, 20}, {45, 19, 3, 50, 2, 55, 44, 31, 1, 28, 60, 32, 44, 63, 25, 1}, 
{2, 29, 13, 35, 16, 4, 47, 54, 14, 15, 21, 63, 2, 34, 45, 16}, {34, 5, 17, 60, 60, 38, 35, 15, 12, 30, 25, 45, 21, 64, 53, 25}, {15, 32, 4, 42, 41, 12, 10, 48, 26, 64, 13, 22, 62, 36, 19, 62}, 
{36, 63, 20, 58, 26, 44, 17, 26, 18, 34, 4, 28, 34, 51, 1, 35}, {22, 15, 29, 62, 38, 20, 1, 56, 29, 63, 31, 51, 10, 38, 61, 54}, {38, 62, 8, 47, 55, 64, 24, 29, 10, 45, 65, 36, 30, 44, 37, 53}, 
{29, 65, 19, 9, 44, 42, 21, 25, 44, 27, 51, 26, 38, 56, 63, 39}, {56, 30, 1, 23, 24, 6, 8, 46, 38, 1, 54, 18, 24, 37, 52, 48}, {26, 58, 6, 20, 3, 57, 32, 34, 51, 14, 3, 37, 35, 48, 27, 52}, 
{48, 12, 28, 5, 31, 17, 4, 47, 36, 12, 29, 56, 53, 40, 12, 57}, {14, 25, 12, 7, 65, 53, 28, 27, 64, 40, 2, 27, 20, 54, 46, 7}, {54, 46, 21, 11, 29, 10, 60, 64, 34, 48, 32, 1, 12, 52, 58, 23}, 
{27, 9, 18, 17, 14, 36, 37, 17, 63, 37, 47, 52, 37, 65, 9, 22}, {65, 35, 14, 25, 61, 15, 46, 60, 45, 56, 38, 54, 65, 39, 15, 5}, {17, 48, 43, 39, 58, 1, 18, 24, 53, 9, 61, 7, 7, 41, 11, 11}, 
{41, 13, 56, 65, 4, 37, 3, 61, 50, 23, 42, 11, 25, 49, 24, 30}, {7, 59, 39, 52, 27, 8, 15, 9, 35, 20, 22, 49, 50, 55, 62, 17}, {55, 21, 64, 54, 45, 47, 25, 38, 60, 5, 9, 41, 48, 59, 43, 24}, 
{20, 14, 36, 40, 47, 21, 57, 7, 42, 59, 27, 9, 15, 61, 6, 56}, {61, 64, 44, 48, 17, 40, 40, 43, 58, 55, 23, 23, 5, 46, 35, 51}, {9, 27, 35, 37, 9, 7, 53, 16, 62, 49, 50, 46, 60, 43, 50, 63}, 
{43, 42, 50, 56, 51, 35, 54, 63, 47, 41, 59, 61, 43, 57, 44, 40}, {62, 16, 5, 44, 13, 25, 30, 21, 57, 53, 14, 42, 3, 19, 34, 19}, {19, 49, 34, 38, 43, 43, 16, 55, 43, 50, 30, 58, 19, 4, 17, 6}, 
{42, 10, 9, 51, 48, 32, 14, 8, 46, 35, 55, 4, 39, 8, 36, 28}, {8, 38, 25, 36, 10, 52, 7, 44, 61, 60, 48, 19, 57, 24, 48, 2}, {35, 55, 11, 64, 20, 27, 61, 14, 59, 24, 49, 53, 26, 6, 26, 34}, 
{6, 23, 15, 34, 54, 61, 39, 62, 55, 8, 56, 50, 8, 31, 10, 58}, {53, 44, 7, 63, 59, 18, 43, 30, 49, 4, 8, 31, 55, 16, 38, 37}, {16, 6, 24, 45, 30, 63, 64, 53, 41, 19, 20, 6, 49, 13, 64, 45}, 
{63, 26, 40, 13, 32, 50, 11, 1, 39, 44, 40, 64, 14, 21, 59, 46}, {21, 47, 63, 16, 64, 11, 22, 35, 65, 38, 6, 34, 22, 3, 41, 65}, {64, 37, 53, 31, 50, 62, 19, 12, 52, 51, 45, 3, 64, 32, 21, 50}, 
{32, 61, 49, 6, 6, 28, 31, 51, 54, 36, 41, 21, 45, 2, 13, 41}, {51, 34, 62, 24, 25, 54, 56, 18, 40, 2, 53, 44, 4, 30, 47, 4}, {30, 3, 46, 8, 40, 22, 49, 40, 48, 32, 62, 38, 32, 15, 65, 31}, 
{44, 51, 37, 4, 28, 46, 65, 10, 37, 3, 35, 15, 56, 28, 5, 12}, {28, 4, 58, 19, 11, 2, 42, 37, 56, 21, 1, 30, 36, 22, 29, 13}, {37, 1, 47, 57, 22, 24, 45, 41, 22, 39, 15, 40, 28, 10, 3, 27}, 
{10, 36, 65, 43, 42, 60, 58, 20, 28, 65, 12, 48, 42, 29, 14, 18}, {40, 8, 60, 46, 63, 9, 34, 32, 15, 52, 63, 29, 31, 18, 39, 14}, {18, 54, 38, 61, 35, 49, 62, 19, 30, 54, 37, 10, 13, 26, 54, 9}, 
{52, 41, 54, 59, 1, 13, 38, 39, 2, 26, 64, 39, 46, 12, 20, 59}, {12, 20, 45, 55, 37, 56, 6, 2, 32, 18, 34, 65, 54, 14, 8, 43}, {39, 57, 48, 49, 52, 30, 29, 49, 3, 29, 19, 14, 29, 1, 57, 44}, 
{1, 31, 52, 41, 5, 51, 20, 6, 21, 10, 28, 12, 1, 27, 51, 61}, {49, 18, 23, 27, 8, 65, 48, 42, 13, 57, 5, 59, 59, 25, 55, 55}, {25, 53, 10, 1, 62, 29, 63, 5, 16, 43, 24, 55, 41, 17, 42, 36}, 
{59, 7, 27, 14, 39, 58, 51, 57, 31, 46, 44, 17, 16, 11, 4, 49}, {11, 45, 2, 12, 21, 19, 41, 28, 6, 61, 57, 25, 18, 7, 23, 42}, {46, 52, 30, 26, 19, 45, 9, 59, 24, 7, 39, 57, 51, 5, 60, 10}, 
{5, 2, 22, 18, 49, 26, 26, 23, 8, 11, 43, 43, 61, 20, 31, 15}, {57, 39, 31, 29, 57, 59, 13, 50, 4, 17, 16, 20, 6, 23, 16, 3}, {23, 24, 16, 10, 15, 31, 12, 3, 19, 25, 7, 5, 23, 9, 22, 26} };

int NOLH_4[][22] = { {32, 58, 51, 59, 89, 73, 98, 72, 120, 100, 98, 78, 129, 120, 80, 109, 44, 70, 116, 124, 34, 115}, {115, 40, 56, 60, 59, 55, 27, 62, 50, 119, 77, 80, 122, 94, 104, 117, 13, 75, 79, 94, 29, 98}, 
{58, 98, 1, 36, 21, 97, 84, 79, 74, 61, 21, 63, 111, 128, 82, 72, 54, 20, 108, 85, 62, 90}, {90, 115, 39, 48, 98, 10, 53, 35, 60, 54, 49, 44, 127, 87, 125, 79, 57, 47, 76, 100, 33, 72}, 
{1, 51, 72, 31, 69, 47, 120, 129, 82, 15, 128, 110, 35, 58, 57, 113, 14, 87, 94, 84, 7, 91}, {91, 56, 90, 2, 43, 76, 6, 33, 16, 24, 129, 81, 63, 41, 45, 119, 52, 113, 98, 95, 11, 129}, 
{51, 129, 98, 38, 30, 32, 121, 124, 94, 91, 14, 1, 15, 61, 41, 118, 21, 45, 121, 88, 4, 74}, {74, 91, 115, 9, 119, 112, 3, 40, 28, 64, 12, 22, 37, 44, 56, 61, 45, 13, 125, 75, 3, 79}, 
{4, 7, 34, 27, 126, 94, 56, 94, 110, 96, 36, 77, 34, 122, 103, 101, 26, 126, 43, 4, 79, 127}, {127, 11, 29, 35, 14, 27, 71, 26, 19, 63, 18, 90, 16, 118, 59, 94, 16, 90, 29, 48, 74, 126}, 
{7, 126, 62, 37, 10, 100, 29, 80, 107, 22, 70, 36, 30, 97, 121, 93, 41, 28, 39, 1, 129, 119}, {119, 127, 33, 50, 129, 66, 117, 37, 41, 32, 87, 33, 57, 109, 70, 76, 19, 26, 26, 9, 91, 123}, 
{62, 34, 123, 24, 93, 7, 47, 85, 115, 2, 28, 117, 94, 42, 15, 99, 53, 107, 60, 62, 72, 97}, {97, 29, 119, 46, 31, 118, 70, 27, 51, 3, 42, 109, 121, 47, 28, 87, 12, 63, 64, 32, 90, 68}, 
{34, 68, 126, 30, 28, 5, 19, 127, 104, 109, 97, 31, 68, 39, 10, 81, 61, 15, 19, 60, 98, 101}, {101, 97, 127, 5, 67, 126, 94, 32, 55, 124, 80, 43, 76, 34, 12, 127, 23, 49, 74, 37, 115, 96}, 
{30, 24, 27, 96, 90, 89, 64, 28, 77, 81, 78, 27, 128, 6, 101, 10, 6, 116, 107, 10, 59, 125}, {125, 46, 35, 101, 26, 20, 62, 96, 6, 77, 93, 57, 80, 11, 88, 62, 55, 100, 123, 39, 60, 100}, 
{24, 100, 37, 68, 5, 88, 69, 31, 119, 41, 63, 92, 124, 9, 87, 8, 28, 54, 50, 41, 36, 84}, {84, 125, 50, 97, 103, 45, 21, 70, 32, 8, 61, 85, 79, 3, 78, 47, 17, 7, 67, 31, 48, 106}, 
{37, 27, 106, 123, 86, 40, 129, 14, 83, 23, 126, 3, 59, 106, 11, 15, 64, 86, 128, 50, 31, 80}, {80, 35, 84, 119, 17, 111, 35, 125, 73, 7, 75, 2, 17, 71, 49, 23, 49, 80, 113, 8, 2, 93}, 
{27, 93, 100, 126, 45, 8, 122, 7, 113, 104, 6, 116, 52, 103, 69, 22, 43, 33, 105, 51, 38, 95}, {95, 80, 125, 127, 81, 105, 37, 128, 52, 82, 17, 120, 38, 85, 32, 4, 25, 58, 83, 28, 9, 103}, 
{38, 31, 59, 79, 128, 61, 34, 57, 106, 71, 56, 35, 14, 7, 107, 44, 42, 118, 18, 119, 103, 121}, {121, 2, 60, 74, 38, 50, 105, 81, 38, 85, 44, 4, 26, 49, 117, 20, 20, 121, 49, 128, 95, 92}, 
{31, 92, 36, 129, 16, 78, 40, 39, 108, 60, 91, 66, 23, 50, 75, 64, 8, 18, 30, 112, 93, 128}, {128, 121, 48, 91, 97, 37, 76, 126, 8, 38, 105, 79, 5, 66, 92, 28, 22, 41, 57, 107, 80, 99}, 
{36, 59, 99, 72, 70, 1, 49, 41, 61, 5, 10, 5, 87, 99, 17, 57, 47, 114, 11, 109, 106, 82}, {82, 60, 128, 90, 55, 96, 118, 74, 46, 40, 20, 48, 91, 129, 6, 46, 63, 108, 41, 71, 84, 94}, 
{59, 94, 92, 98, 51, 6, 2, 17, 112, 121, 104, 70, 84, 73, 8, 24, 10, 62, 68, 101, 100, 70}, {70, 82, 121, 115, 95, 86, 99, 101, 4, 95, 90, 88, 89, 95, 34, 52, 18, 39, 6, 76, 125, 71}, 
{10, 47, 42, 44, 64, 117, 73, 117, 67, 116, 96, 74, 110, 84, 14, 6, 71, 52, 1, 115, 6, 112}, {112, 63, 20, 13, 52, 49, 5, 43, 68, 117, 123, 62, 85, 107, 25, 33, 70, 42, 48, 78, 55, 120}, 
{47, 120, 8, 54, 20, 121, 106, 69, 54, 31, 45, 19, 82, 76, 19, 41, 94, 71, 33, 127, 28, 67}, {67, 112, 22, 57, 123, 15, 17, 52, 103, 36, 16, 25, 77, 92, 44, 55, 82, 77, 15, 81, 17, 83}, 
{8, 42, 83, 14, 108, 62, 86, 76, 2, 29, 100, 96, 31, 60, 68, 40, 99, 10, 55, 106, 64, 108}, {108, 20, 67, 52, 39, 77, 79, 11, 101, 19, 95, 113, 29, 48, 84, 2, 128, 31, 42, 111, 49, 122}, 
{42, 122, 120, 21, 8, 29, 110, 88, 12, 78, 51, 55, 60, 20, 76, 59, 92, 125, 34, 118, 43, 110}, {110, 108, 112, 45, 111, 128, 48, 21, 100, 87, 68, 18, 27, 18, 79, 30, 121, 92, 46, 97, 25, 88}, 
{43, 64, 6, 26, 88, 72, 14, 110, 31, 112, 9, 121, 28, 105, 2, 9, 103, 73, 120, 47, 88, 105}, {105, 49, 55, 16, 68, 3, 92, 24, 93, 114, 27, 93, 11, 67, 21, 14, 95, 37, 110, 34, 110, 87}, 
{64, 87, 28, 41, 36, 116, 15, 107, 45, 47, 101, 24, 69, 125, 36, 18, 93, 111, 127, 58, 122, 81}, {81, 105, 17, 19, 96, 31, 119, 38, 86, 12, 107, 32, 64, 77, 1, 32, 80, 106, 102, 25, 108, 66}, 
{28, 6, 66, 53, 73, 38, 72, 118, 43, 28, 5, 115, 108, 17, 77, 26, 106, 6, 69, 61, 83, 113}, {113, 55, 81, 12, 3, 67, 87, 55, 109, 46, 22, 72, 117, 37, 90, 50, 84, 25, 99, 63, 67, 102}, 
{6, 102, 87, 61, 18, 23, 22, 108, 3, 80, 59, 11, 123, 26, 97, 34, 100, 94, 59, 5, 120, 75}, {75, 113, 105, 23, 101, 91, 126, 59, 95, 79, 83, 76, 109, 19, 99, 39, 125, 101, 86, 40, 112, 124}, 
{61, 53, 26, 124, 124, 113, 123, 53, 33, 126, 57, 30, 90, 14, 7, 125, 96, 27, 8, 14, 44, 107}, {107, 12, 16, 75, 53, 21, 52, 122, 105, 74, 76, 7, 106, 29, 4, 95, 101, 64, 35, 38, 13, 69}, 
{53, 69, 41, 102, 24, 114, 67, 15, 39, 27, 3, 83, 75, 55, 20, 82, 68, 95, 24, 73, 54, 118}, {118, 107, 19, 113, 72, 46, 50, 100, 71, 10, 41, 124, 112, 56, 22, 111, 97, 98, 40, 64, 57, 77}, 
{41, 26, 77, 66, 107, 11, 89, 48, 13, 44, 84, 23, 72, 108, 100, 74, 123, 2, 45, 16, 14, 111}, {111, 16, 118, 81, 13, 70, 33, 67, 125, 58, 119, 39, 42, 98, 64, 85, 119, 1, 13, 13, 52, 89}, 
{26, 89, 69, 87, 9, 28, 91, 25, 9, 129, 48, 84, 25, 79, 106, 105, 126, 109, 53, 20, 21, 114}, {114, 111, 107, 105, 80, 74, 28, 86, 129, 68, 13, 59, 49, 102, 114, 92, 127, 122, 52, 43, 45, 104}, 
{21, 14, 44, 88, 84, 108, 18, 16, 49, 75, 8, 28, 33, 13, 18, 123, 79, 46, 126, 113, 104, 85}, {85, 52, 13, 110, 25, 35, 104, 84, 72, 73, 38, 8, 47, 16, 35, 103, 74, 3, 118, 104, 114, 109}, 
{14, 109, 54, 122, 47, 71, 23, 18, 42, 25, 106, 114, 10, 68, 37, 129, 129, 74, 93, 86, 89, 78}, {78, 85, 57, 108, 118, 51, 88, 121, 123, 33, 99, 89, 56, 40, 58, 114, 91, 79, 114, 74, 111, 116}, 
{54, 44, 116, 83, 115, 48, 16, 66, 40, 37, 19, 12, 118, 100, 91, 63, 72, 11, 103, 103, 77, 73}, {73, 13, 78, 67, 48, 106, 85, 83, 64, 17, 58, 26, 86, 126, 127, 88, 90, 34, 72, 108, 118, 76}, 
{44, 76, 109, 120, 54, 43, 30, 19, 14, 88, 115, 69, 126, 115, 67, 77, 98, 82, 109, 77, 69, 117}, {117, 73, 85, 112, 74, 104, 75, 120, 96, 110, 66, 101, 98, 78, 83, 70, 115, 69, 92, 123, 107, 86}, 
{65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65}, {98, 72, 79, 71, 41, 57, 32, 58, 10, 30, 32, 52, 1, 10, 50, 21, 86, 60, 14, 6, 96, 15}, 
{15, 90, 74, 70, 71, 75, 103, 68, 80, 11, 53, 50, 8, 36, 26, 13, 117, 55, 51, 36, 101, 32}, {72, 32, 129, 94, 109, 33, 46, 51, 56, 69, 109, 67, 19, 2, 48, 58, 76, 110, 22, 45, 68, 40}, 
{40, 15, 91, 82, 32, 120, 77, 95, 70, 76, 81, 86, 3, 43, 5, 51, 73, 83, 54, 30, 97, 58}, {129, 79, 58, 99, 61, 83, 10, 1, 48, 115, 2, 20, 95, 72, 73, 17, 116, 43, 36, 46, 123, 39}, 
{39, 74, 40, 128, 87, 54, 124, 97, 114, 106, 1, 49, 67, 89, 85, 11, 78, 17, 32, 35, 119, 1}, {79, 1, 32, 92, 100, 98, 9, 6, 36, 39, 116, 129, 115, 69, 89, 12, 109, 85, 9, 42, 126, 56}, 
{56, 39, 15, 121, 11, 18, 127, 90, 102, 66, 118, 108, 93, 86, 74, 69, 85, 117, 5, 55, 127, 51}, {126, 123, 96, 103, 4, 36, 74, 36, 20, 34, 94, 53, 96, 8, 27, 29, 104, 4, 87, 126, 51, 3}, 
{3, 119, 101, 95, 116, 103, 59, 104, 111, 67, 112, 40, 114, 12, 71, 36, 114, 40, 101, 82, 56, 4}, {123, 4, 68, 93, 120, 30, 101, 50, 23, 108, 60, 94, 100, 33, 9, 37, 89, 102, 91, 129, 1, 11}, 
{11, 3, 97, 80, 1, 64, 13, 93, 89, 98, 43, 97, 73, 21, 60, 54, 111, 104, 104, 121, 39, 7}, {68, 96, 7, 106, 37, 123, 83, 45, 15, 128, 102, 13, 36, 88, 115, 31, 77, 23, 70, 68, 58, 33}, 
{33, 101, 11, 84, 99, 12, 60, 103, 79, 127, 88, 21, 9, 83, 102, 43, 118, 67, 66, 98, 40, 62}, {96, 62, 4, 100, 102, 125, 111, 3, 26, 21, 33, 99, 62, 91, 120, 49, 69, 115, 111, 70, 32, 29}, 
{29, 33, 3, 125, 63, 4, 36, 98, 75, 6, 50, 87, 54, 96, 118, 3, 107, 81, 56, 93, 15, 34}, {100, 106, 103, 34, 40, 41, 66, 102, 53, 49, 52, 103, 2, 124, 29, 120, 124, 14, 23, 120, 71, 5}, 
{5, 84, 95, 29, 104, 110, 68, 34, 124, 53, 37, 73, 50, 119, 42, 68, 75, 30, 7, 91, 70, 30}, {106, 30, 93, 62, 125, 42, 61, 99, 11, 89, 67, 38, 6, 121, 43, 122, 102, 76, 80, 89, 94, 46}, 
{46, 5, 80, 33, 27, 85, 109, 60, 98, 122, 69, 45, 51, 127, 52, 83, 113, 123, 63, 99, 82, 24}, {93, 103, 24, 7, 44, 90, 1, 116, 47, 107, 4, 127, 71, 24, 119, 115, 66, 44, 2, 80, 99, 50}, 
{50, 95, 46, 11, 113, 19, 95, 5, 57, 123, 55, 128, 113, 59, 81, 107, 81, 50, 17, 122, 128, 37}, {103, 37, 30, 4, 85, 122, 8, 123, 17, 26, 124, 14, 78, 27, 61, 108, 87, 97, 25, 79, 92, 35}, 
{35, 50, 5, 3, 49, 25, 93, 2, 78, 48, 113, 10, 92, 45, 98, 126, 105, 72, 47, 102, 121, 27}, {92, 99, 71, 51, 2, 69, 96, 73, 24, 59, 74, 95, 116, 123, 23, 86, 88, 12, 112, 11, 27, 9}, 
{9, 128, 70, 56, 92, 80, 25, 49, 92, 45, 86, 126, 104, 81, 13, 110, 110, 9, 81, 2, 35, 38}, {99, 38, 94, 1, 114, 52, 90, 91, 22, 70, 39, 64, 107, 80, 55, 66, 122, 112, 100, 18, 37, 2}, 
{2, 9, 82, 39, 33, 93, 54, 4, 122, 92, 25, 51, 125, 64, 38, 102, 108, 89, 73, 23, 50, 31}, {94, 71, 31, 58, 60, 129, 81, 89, 69, 125, 120, 125, 43, 31, 113, 73, 83, 16, 119, 21, 24, 48}, 
{48, 70, 2, 40, 75, 34, 12, 56, 84, 90, 110, 82, 39, 1, 124, 84, 67, 22, 89, 59, 46, 36}, {71, 36, 38, 32, 79, 124, 128, 113, 18, 9, 26, 60, 46, 57, 122, 106, 120, 68, 62, 29, 30, 60}, 
{60, 48, 9, 15, 35, 44, 31, 29, 126, 35, 40, 42, 41, 35, 96, 78, 112, 91, 124, 54, 5, 59}, {120, 83, 88, 86, 66, 13, 57, 13, 63, 14, 34, 56, 20, 46, 116, 124, 59, 78, 129, 15, 124, 18}, 
{18, 67, 110, 117, 78, 81, 125, 87, 62, 13, 7, 68, 45, 23, 105, 97, 60, 88, 82, 52, 75, 10}, {83, 10, 122, 76, 110, 9, 24, 61, 76, 99, 85, 111, 48, 54, 111, 89, 36, 59, 97, 3, 102, 63}, 
{63, 18, 108, 73, 7, 115, 113, 78, 27, 94, 114, 105, 53, 38, 86, 75, 48, 53, 115, 49, 113, 47}, {122, 88, 47, 116, 22, 68, 44, 54, 128, 101, 30, 34, 99, 70, 62, 90, 31, 120, 75, 24, 66, 22}, 
{22, 110, 63, 78, 91, 53, 51, 119, 29, 111, 35, 17, 101, 82, 46, 128, 2, 99, 88, 19, 81, 8}, {88, 8, 10, 109, 122, 101, 20, 42, 118, 52, 79, 75, 70, 110, 54, 71, 38, 5, 96, 12, 87, 20}, 
{20, 22, 18, 85, 19, 2, 82, 109, 30, 43, 62, 112, 103, 112, 51, 100, 9, 38, 84, 33, 105, 42}, {87, 66, 124, 104, 42, 58, 116, 20, 99, 18, 121, 9, 102, 25, 128, 121, 27, 57, 10, 83, 42, 25}, 
{25, 81, 75, 114, 62, 127, 38, 106, 37, 16, 103, 37, 119, 63, 109, 116, 35, 93, 20, 96, 20, 43}, {66, 43, 102, 89, 94, 14, 115, 23, 85, 83, 29, 106, 61, 5, 94, 112, 37, 19, 3, 72, 8, 49}, 
{49, 25, 113, 111, 34, 99, 11, 92, 44, 118, 23, 98, 66, 53, 129, 98, 50, 24, 28, 105, 22, 64}, {102, 124, 64, 77, 57, 92, 58, 12, 87, 102, 125, 15, 22, 113, 53, 104, 24, 124, 61, 69, 47, 17}, 
{17, 75, 49, 118, 127, 63, 43, 75, 21, 84, 108, 58, 13, 93, 40, 80, 46, 105, 31, 67, 63, 28}, {124, 28, 43, 69, 112, 107, 108, 22, 127, 50, 71, 119, 7, 104, 33, 96, 30, 36, 71, 125, 10, 55}, 
{55, 17, 25, 107, 29, 39, 4, 71, 35, 51, 47, 54, 21, 111, 31, 91, 5, 29, 44, 90, 18, 6}, {69, 77, 104, 6, 6, 17, 7, 77, 97, 4, 73, 100, 40, 116, 123, 5, 34, 103, 122, 116, 86, 23}, 
{23, 118, 114, 55, 77, 109, 78, 8, 25, 56, 54, 123, 24, 101, 126, 35, 29, 66, 95, 92, 117, 61}, {77, 61, 89, 28, 106, 16, 63, 115, 91, 103, 127, 47, 55, 75, 110, 48, 62, 35, 106, 57, 76, 12}, 
{12, 23, 111, 17, 58, 84, 80, 30, 59, 120, 89, 6, 18, 74, 108, 19, 33, 32, 90, 66, 73, 53}, {89, 104, 53, 64, 23, 119, 41, 82, 117, 86, 46, 107, 58, 22, 30, 56, 7, 128, 85, 114, 116, 19}, 
{19, 114, 12, 49, 117, 60, 97, 63, 5, 72, 11, 91, 88, 32, 66, 45, 11, 129, 117, 117, 78, 41}, {104, 41, 61, 43, 121, 102, 39, 105, 121, 1, 82, 46, 105, 51, 24, 25, 4, 21, 77, 110, 109, 16}, 
{16, 19, 23, 25, 50, 56, 102, 44, 1, 62, 117, 71, 81, 28, 16, 38, 3, 8, 78, 87, 85, 26}, {109, 116, 86, 42, 46, 22, 112, 114, 81, 55, 122, 102, 97, 117, 112, 7, 51, 84, 4, 17, 26, 45}, 
{45, 78, 117, 20, 105, 95, 26, 46, 58, 57, 92, 122, 83, 114, 95, 27, 56, 127, 12, 26, 16, 21}, {116, 21, 76, 8, 83, 59, 107, 112, 88, 105, 24, 16, 120, 62, 93, 1, 1, 56, 37, 44, 41, 52}, 
{52, 45, 73, 22, 12, 79, 42, 9, 7, 97, 31, 41, 74, 90, 72, 16, 39, 51, 16, 56, 19, 14}, {76, 86, 14, 47, 15, 82, 114, 64, 90, 93, 111, 118, 12, 30, 39, 67, 58, 119, 27, 27, 53, 57}, 
{57, 117, 52, 63, 82, 24, 45, 47, 66, 113, 72, 104, 44, 4, 3, 42, 40, 96, 58, 22, 12, 54}, {86, 54, 21, 10, 76, 87, 100, 111, 116, 42, 15, 61, 4, 15, 63, 53, 32, 48, 21, 53, 61, 13}, 
{13, 57, 45, 18, 56, 26, 55, 10, 34, 20, 64, 29, 32, 52, 47, 60, 15, 61, 38, 7, 23, 44}, {70, 124, 72, 80, 98, 120, 78, 109, 89, 115, 34, 59, 32, 129, 100, 44, 51, 116, 120, 98, 73, 58}, 
{75, 94, 62, 104, 77, 50, 80, 117, 59, 98, 29, 60, 115, 122, 119, 13, 56, 79, 94, 27, 55, 40}, {20, 85, 79, 82, 21, 74, 63, 72, 21, 90, 62, 36, 58, 111, 61, 54, 1, 108, 128, 84, 97, 98}, 
{47, 100, 35, 125, 49, 60, 44, 79, 98, 72, 33, 48, 90, 127, 54, 57, 39, 76, 87, 53, 10, 115}, {87, 84, 129, 57, 128, 82, 110, 113, 69, 91, 7, 31, 1, 35, 15, 14, 72, 94, 58, 120, 47, 51}, 
{113, 95, 33, 45, 129, 16, 81, 119, 43, 129, 11, 2, 91, 63, 24, 52, 90, 98, 41, 6, 76, 56}, {45, 88, 124, 41, 14, 94, 1, 118, 30, 74, 4, 38, 51, 15, 91, 21, 98, 121, 61, 121, 32, 129}, 
{13, 75, 40, 56, 12, 28, 22, 61, 119, 79, 3, 9, 74, 37, 64, 45, 115, 125, 44, 3, 112, 91}, {126, 4, 94, 103, 36, 110, 77, 101, 126, 127, 79, 27, 4, 34, 96, 26, 34, 43, 122, 56, 94, 7}, 
{90, 48, 26, 59, 18, 19, 90, 94, 14, 126, 74, 35, 127, 16, 63, 16, 29, 29, 118, 71, 27, 11}, {28, 1, 80, 121, 70, 107, 36, 93, 10, 119, 129, 37, 7, 30, 22, 41, 62, 39, 97, 29, 100, 126}, 
{26, 9, 37, 70, 87, 41, 33, 76, 129, 123, 91, 50, 119, 57, 32, 19, 33, 26, 109, 117, 66, 127}, {107, 62, 85, 15, 28, 115, 117, 99, 93, 97, 72, 24, 62, 94, 2, 53, 123, 60, 42, 47, 7, 34}, 
{63, 32, 27, 28, 42, 51, 109, 87, 31, 68, 90, 46, 97, 121, 3, 12, 119, 64, 47, 70, 118, 29}, {15, 60, 127, 10, 97, 104, 31, 81, 28, 101, 98, 30, 34, 68, 109, 61, 126, 19, 39, 19, 5, 68}, 
{49, 37, 32, 12, 80, 55, 43, 127, 67, 96, 115, 5, 101, 76, 124, 23, 127, 74, 34, 94, 126, 97}, {116, 10, 28, 101, 78, 77, 27, 10, 90, 125, 59, 96, 30, 128, 81, 6, 27, 107, 6, 64, 89, 24}, 
{100, 39, 96, 88, 93, 6, 57, 62, 26, 100, 60, 101, 125, 80, 77, 55, 35, 123, 11, 62, 20, 46}, {54, 41, 31, 87, 63, 119, 92, 8, 5, 84, 36, 68, 24, 124, 41, 28, 37, 50, 9, 69, 88, 100}, 
{7, 31, 70, 78, 61, 32, 85, 47, 103, 106, 48, 97, 84, 79, 8, 17, 50, 67, 3, 21, 45, 125}, {86, 50, 14, 11, 126, 83, 3, 15, 86, 80, 31, 123, 37, 59, 23, 64, 106, 128, 106, 129, 40, 27}, 
{80, 8, 125, 49, 75, 73, 2, 23, 17, 93, 2, 119, 80, 17, 7, 49, 84, 113, 71, 35, 111, 35}, {33, 51, 7, 69, 6, 113, 116, 22, 45, 95, 38, 126, 27, 52, 104, 43, 100, 105, 103, 122, 8, 93}, 
{58, 28, 128, 32, 17, 52, 120, 4, 81, 103, 9, 127, 95, 38, 82, 25, 125, 83, 85, 37, 105, 80}, {118, 119, 57, 107, 56, 106, 35, 44, 128, 121, 103, 79, 38, 14, 71, 42, 59, 18, 7, 34, 61, 31}, 
{121, 128, 81, 117, 44, 38, 4, 20, 38, 92, 95, 74, 121, 26, 85, 20, 60, 49, 49, 105, 50, 2}, {18, 112, 39, 75, 91, 108, 66, 64, 16, 128, 93, 129, 31, 23, 60, 8, 36, 30, 50, 40, 78, 92}, 
{41, 107, 126, 92, 105, 8, 79, 28, 97, 99, 80, 91, 128, 5, 38, 22, 48, 57, 66, 76, 37, 121}, {114, 109, 41, 17, 10, 61, 5, 57, 70, 82, 106, 72, 36, 87, 5, 47, 99, 11, 99, 49, 1, 59}, 
{108, 71, 74, 6, 20, 46, 48, 46, 55, 94, 84, 90, 82, 91, 40, 63, 128, 41, 129, 118, 96, 60}, {62, 101, 17, 8, 104, 112, 70, 24, 51, 70, 100, 98, 59, 84, 121, 10, 92, 68, 73, 2, 6, 94}, 
{39, 76, 101, 34, 90, 4, 88, 52, 95, 71, 125, 115, 70, 89, 95, 18, 121, 6, 95, 99, 86, 82}, {52, 115, 117, 14, 96, 67, 74, 6, 64, 112, 6, 44, 10, 110, 116, 71, 42, 1, 84, 73, 117, 47}, 
{42, 78, 43, 25, 123, 68, 62, 33, 52, 120, 55, 13, 112, 85, 117, 70, 20, 48, 107, 5, 49, 63}, {71, 127, 69, 19, 45, 54, 19, 41, 20, 67, 28, 54, 47, 82, 31, 94, 8, 33, 76, 106, 121, 120}, 
{77, 81, 52, 44, 16, 103, 25, 55, 123, 83, 17, 57, 67, 77, 36, 82, 22, 15, 92, 17, 15, 112}, {10, 106, 76, 68, 100, 2, 96, 40, 108, 108, 64, 14, 8, 31, 29, 99, 83, 55, 60, 86, 62, 42}, 
{31, 111, 11, 84, 95, 101, 113, 2, 39, 122, 49, 52, 108, 29, 19, 128, 67, 42, 48, 79, 77, 20}, {125, 118, 88, 76, 51, 12, 55, 59, 8, 110, 43, 21, 42, 60, 78, 92, 120, 34, 20, 110, 29, 122}, 
{92, 97, 21, 79, 68, 100, 18, 30, 111, 88, 25, 45, 110, 27, 87, 121, 112, 46, 18, 48, 128, 108}, {73, 47, 110, 2, 9, 31, 121, 9, 88, 105, 88, 26, 43, 28, 112, 103, 6, 120, 105, 14, 72, 64}, 
{37, 34, 24, 21, 27, 93, 93, 14, 68, 87, 110, 16, 105, 11, 114, 95, 55, 110, 67, 92, 3, 49}, {111, 58, 107, 36, 101, 45, 24, 18, 36, 81, 122, 41, 64, 69, 47, 93, 28, 127, 125, 15, 116, 87}, 
{106, 25, 38, 1, 107, 86, 32, 32, 96, 66, 108, 19, 81, 64, 12, 80, 17, 102, 77, 119, 31, 105}, {6, 61, 118, 77, 5, 43, 115, 26, 73, 113, 83, 53, 28, 108, 28, 106, 66, 69, 17, 72, 38, 6}, 
{25, 63, 55, 90, 22, 109, 72, 50, 3, 102, 67, 12, 113, 117, 46, 84, 81, 99, 37, 87, 67, 55}, {94, 5, 108, 97, 59, 3, 11, 34, 18, 75, 120, 61, 6, 123, 80, 100, 87, 59, 26, 22, 23, 102}, 
{101, 40, 59, 99, 83, 95, 76, 39, 101, 124, 112, 23, 75, 109, 79, 125, 105, 86, 19, 126, 91, 113}, {27, 14, 53, 7, 57, 33, 30, 125, 124, 107, 44, 124, 61, 90, 126, 96, 26, 8, 14, 123, 113, 53}, 
{64, 38, 122, 4, 76, 105, 7, 95, 53, 69, 13, 75, 107, 106, 74, 101, 16, 35, 29, 52, 21, 12}, {95, 73, 15, 20, 3, 39, 83, 82, 24, 118, 54, 102, 53, 75, 27, 68, 41, 24, 55, 67, 114, 69}, 
{98, 64, 100, 22, 41, 71, 124, 111, 72, 77, 57, 113, 118, 112, 10, 97, 19, 40, 56, 50, 46, 107}, {2, 16, 48, 100, 84, 13, 23, 74, 107, 111, 14, 66, 41, 72, 44, 123, 77, 45, 108, 89, 11, 26}, 
{1, 13, 67, 64, 119, 125, 39, 85, 13, 89, 52, 81, 111, 42, 58, 119, 118, 13, 98, 33, 70, 16}, {109, 20, 25, 106, 48, 9, 84, 105, 9, 114, 21, 87, 26, 25, 129, 126, 69, 53, 79, 91, 28, 89}, 
{122, 43, 86, 114, 13, 129, 59, 92, 80, 104, 45, 105, 114, 49, 68, 127, 107, 52, 102, 28, 74, 111}, {46, 113, 16, 18, 8, 49, 28, 123, 84, 85, 104, 88, 21, 33, 75, 79, 44, 126, 13, 18, 108, 14}, 
{3, 104, 84, 35, 38, 72, 8, 103, 25, 109, 114, 110, 85, 47, 73, 74, 13, 118, 16, 104, 35, 52}, {74, 86, 18, 37, 106, 42, 114, 129, 47, 78, 89, 122, 14, 10, 25, 129, 54, 93, 68, 23, 71, 109}, 
{79, 74, 121, 58, 99, 123, 89, 114, 118, 116, 111, 108, 78, 56, 33, 91, 57, 114, 40, 88, 51, 85}, {11, 103, 66, 91, 19, 40, 12, 63, 115, 73, 77, 83, 54, 118, 37, 72, 116, 103, 100, 16, 48, 44}, 
{34, 108, 83, 127, 58, 64, 26, 88, 48, 76, 118, 67, 73, 86, 17, 90, 78, 72, 126, 85, 106, 13}, {82, 77, 19, 67, 115, 14, 69, 77, 54, 117, 69, 120, 44, 126, 88, 98, 109, 109, 115, 30, 43, 76}, 
{69, 123, 120, 83, 66, 96, 101, 70, 74, 86, 107, 112, 117, 98, 110, 115, 85, 92, 78, 75, 104, 73}, {60, 6, 58, 50, 32, 10, 52, 21, 41, 15, 96, 71, 98, 1, 30, 86, 79, 14, 10, 32, 57, 72}, 
{55, 36, 68, 26, 53, 80, 50, 13, 71, 32, 101, 70, 15, 8, 11, 117, 74, 51, 36, 103, 75, 90}, {110, 45, 51, 48, 109, 56, 67, 58, 109, 40, 68, 94, 72, 19, 69, 76, 129, 22, 2, 46, 33, 32}, 
{83, 30, 95, 5, 81, 70, 86, 51, 32, 58, 97, 82, 40, 3, 76, 73, 91, 54, 43, 77, 120, 15}, {43, 46, 1, 73, 2, 48, 20, 17, 61, 39, 123, 99, 129, 95, 115, 116, 58, 36, 72, 10, 83, 79}, 
{17, 35, 97, 85, 1, 114, 49, 11, 87, 1, 119, 128, 39, 67, 106, 78, 40, 32, 89, 124, 54, 74}, {85, 42, 6, 89, 116, 36, 129, 12, 100, 56, 126, 92, 79, 115, 39, 109, 32, 9, 69, 9, 98, 1}, 
{117, 55, 90, 74, 118, 102, 108, 69, 11, 51, 127, 121, 56, 93, 66, 85, 15, 5, 86, 127, 18, 39}, {4, 126, 36, 27, 94, 20, 53, 29, 4, 3, 51, 103, 126, 96, 34, 104, 96, 87, 8, 74, 36, 123}, 
{40, 82, 104, 71, 112, 111, 40, 36, 116, 4, 56, 95, 3, 114, 67, 114, 101, 101, 12, 59, 103, 119}, {102, 129, 50, 9, 60, 23, 94, 37, 120, 11, 1, 93, 123, 100, 108, 89, 68, 91, 33, 101, 30, 4}, 
{104, 121, 93, 60, 43, 89, 97, 54, 1, 7, 39, 80, 11, 73, 98, 111, 97, 104, 21, 13, 64, 3}, {23, 68, 45, 115, 102, 15, 13, 31, 37, 33, 58, 106, 68, 36, 128, 77, 7, 70, 88, 83, 123, 96}, 
{67, 98, 103, 102, 88, 79, 21, 43, 99, 62, 40, 84, 33, 9, 127, 118, 11, 66, 83, 60, 12, 101}, {115, 70, 3, 120, 33, 26, 99, 49, 102, 29, 32, 100, 96, 62, 21, 69, 4, 111, 91, 111, 125, 62}, 
{81, 93, 98, 118, 50, 75, 87, 3, 63, 34, 15, 125, 29, 54, 6, 107, 3, 56, 96, 36, 4, 33}, {14, 120, 102, 29, 52, 53, 103, 120, 40, 5, 71, 34, 100, 2, 49, 124, 103, 23, 124, 66, 41, 106}, 
{30, 91, 34, 42, 37, 124, 73, 68, 104, 30, 70, 29, 5, 50, 53, 75, 95, 7, 119, 68, 110, 84}, {76, 89, 99, 43, 67, 11, 38, 122, 125, 46, 94, 62, 106, 6, 89, 102, 93, 80, 121, 61, 42, 30}, 
{123, 99, 60, 52, 69, 98, 45, 83, 27, 24, 82, 33, 46, 51, 122, 113, 80, 63, 127, 109, 85, 5}, {44, 80, 116, 119, 4, 47, 127, 115, 44, 50, 99, 7, 93, 71, 107, 66, 24, 2, 24, 1, 90, 103}, 
{50, 122, 5, 81, 55, 57, 128, 107, 113, 37, 128, 11, 50, 113, 123, 81, 46, 17, 59, 95, 19, 95}, {97, 79, 123, 61, 124, 17, 14, 108, 85, 35, 92, 4, 103, 78, 26, 87, 30, 25, 27, 8, 122, 37}, 
{72, 102, 2, 98, 113, 78, 10, 126, 49, 27, 121, 3, 35, 92, 48, 105, 5, 47, 45, 93, 25, 50}, {12, 11, 73, 23, 74, 24, 95, 86, 2, 9, 27, 51, 92, 116, 59, 88, 71, 112, 123, 96, 69, 99}, 
{9, 2, 49, 13, 86, 92, 126, 110, 92, 38, 35, 56, 9, 104, 45, 110, 70, 81, 81, 25, 80, 128}, {112, 18, 91, 55, 39, 22, 64, 66, 114, 2, 37, 1, 99, 107, 70, 122, 94, 100, 80, 90, 52, 38}, 
{89, 23, 4, 38, 25, 122, 51, 102, 33, 31, 50, 39, 2, 125, 92, 108, 82, 73, 64, 54, 93, 9}, {16, 21, 89, 113, 120, 69, 125, 73, 60, 48, 24, 58, 94, 43, 125, 83, 31, 119, 31, 81, 129, 71}, 
{22, 59, 56, 124, 110, 84, 82, 84, 75, 36, 46, 40, 48, 39, 90, 67, 2, 89, 1, 12, 34, 70}, {68, 29, 113, 122, 26, 18, 60, 106, 79, 60, 30, 32, 71, 46, 9, 120, 38, 62, 57, 128, 124, 36}, 
{91, 54, 29, 96, 40, 126, 42, 78, 35, 59, 5, 15, 60, 41, 35, 112, 9, 124, 35, 31, 44, 48}, {78, 15, 13, 116, 34, 63, 56, 124, 66, 18, 124, 86, 120, 20, 14, 59, 88, 129, 46, 57, 13, 83}, 
{88, 52, 87, 105, 7, 62, 68, 97, 78, 10, 75, 117, 18, 45, 13, 60, 110, 82, 23, 125, 81, 67}, {59, 3, 61, 111, 85, 76, 111, 89, 110, 63, 102, 76, 83, 48, 99, 36, 122, 97, 54, 24, 9, 10}, 
{53, 49, 78, 86, 114, 27, 105, 75, 7, 47, 113, 73, 63, 53, 94, 48, 108, 115, 38, 113, 115, 18}, {120, 24, 54, 62, 30, 128, 34, 90, 22, 22, 66, 116, 122, 99, 101, 31, 47, 75, 70, 44, 68, 88}, 
{99, 19, 119, 46, 35, 29, 17, 128, 91, 8, 81, 78, 22, 101, 111, 2, 63, 88, 82, 51, 53, 110}, {5, 12, 42, 54, 79, 118, 75, 71, 122, 20, 87, 109, 88, 70, 52, 38, 10, 96, 110, 20, 101, 8}, 
{38, 33, 109, 51, 62, 30, 112, 100, 19, 42, 105, 85, 20, 103, 43, 9, 18, 84, 112, 82, 2, 22}, {57, 83, 20, 128, 121, 99, 9, 121, 42, 25, 42, 104, 87, 102, 18, 27, 124, 10, 25, 116, 58, 66}, 
{93, 96, 106, 109, 103, 37, 37, 116, 62, 43, 20, 114, 25, 119, 16, 35, 75, 20, 63, 38, 127, 81}, {19, 72, 23, 94, 29, 85, 106, 112, 94, 49, 8, 89, 66, 61, 83, 37, 102, 3, 5, 115, 14, 43}, 
{24, 105, 92, 129, 23, 44, 98, 98, 34, 64, 22, 111, 49, 66, 118, 50, 113, 28, 53, 11, 99, 25}, {124, 69, 12, 53, 125, 87, 15, 104, 57, 17, 47, 77, 102, 22, 102, 24, 64, 61, 113, 58, 92, 124}, 
{105, 67, 75, 40, 108, 21, 58, 80, 127, 28, 63, 118, 17, 13, 84, 46, 49, 31, 93, 43, 63, 75}, {36, 125, 22, 33, 71, 127, 119, 96, 112, 55, 10, 69, 124, 7, 50, 30, 43, 71, 104, 108, 107, 28}, 
{29, 90, 71, 31, 47, 35, 54, 91, 29, 6, 18, 107, 55, 21, 51, 5, 25, 44, 111, 4, 39, 17}, {103, 116, 77, 123, 73, 97, 100, 5, 6, 23, 86, 6, 69, 40, 4, 34, 104, 122, 116, 7, 17, 77}, 
{66, 92, 8, 126, 54, 25, 123, 35, 77, 61, 117, 55, 23, 24, 56, 29, 114, 95, 101, 78, 109, 118}, {35, 57, 115, 110, 127, 91, 47, 48, 106, 12, 76, 28, 77, 55, 103, 62, 89, 106, 75, 63, 16, 61}, 
{32, 66, 30, 108, 89, 59, 6, 19, 58, 53, 73, 17, 12, 18, 120, 33, 111, 90, 74, 80, 84, 23}, {128, 114, 82, 30, 46, 117, 107, 56, 23, 19, 116, 64, 89, 58, 86, 7, 53, 85, 22, 41, 119, 104}, 
{129, 117, 63, 66, 11, 5, 91, 45, 117, 41, 78, 49, 19, 88, 72, 11, 12, 117, 32, 97, 60, 114}, {21, 110, 105, 24, 82, 121, 46, 25, 121, 16, 109, 43, 104, 105, 1, 4, 61, 77, 51, 39, 102, 41}, 
{8, 87, 44, 16, 117, 1, 71, 38, 50, 26, 85, 25, 16, 81, 62, 3, 23, 78, 28, 102, 56, 19}, {84, 17, 114, 112, 122, 81, 102, 7, 46, 45, 26, 42, 109, 97, 55, 51, 86, 4, 117, 112, 22, 116}, 
{127, 26, 46, 95, 92, 58, 122, 27, 105, 21, 16, 20, 45, 83, 57, 56, 117, 12, 114, 26, 95, 78}, {56, 44, 112, 93, 24, 88, 16, 1, 83, 52, 41, 8, 116, 120, 105, 1, 76, 37, 62, 107, 59, 21}, 
{51, 56, 9, 72, 31, 7, 41, 16, 12, 14, 19, 22, 52, 74, 97, 39, 73, 16, 90, 42, 79, 45}, {119, 27, 64, 39, 111, 90, 118, 67, 15, 57, 53, 47, 76, 12, 93, 58, 14, 27, 30, 114, 82, 86}, 
{96, 22, 47, 3, 72, 66, 104, 42, 82, 54, 12, 63, 57, 44, 113, 40, 52, 58, 4, 45, 24, 117}, {48, 53, 111, 63, 15, 116, 61, 53, 76, 13, 61, 10, 86, 4, 42, 32, 21, 21, 15, 100, 87, 54}, 
{61, 7, 10, 47, 64, 34, 29, 60, 56, 44, 23, 18, 13, 32, 20, 15, 45, 38, 52, 55, 26, 57} };

int NOLH_5[][29] = { 
{103, 227, 153, 158, 162, 231, 257, 225, 49, 18, 62, 100, 17, 48, 88, 102, 115, 16, 10, 45, 72, 63, 76, 97, 50, 14, 77, 62, 8},
{31, 103, 216, 246, 185, 137, 250, 217, 140, 157, 161, 191, 257, 228, 132, 7, 17, 36, 2, 91, 149, 62, 107, 134, 65, 102, 64, 65, 102},
{42, 153, 31, 144, 247, 209, 242, 229, 166, 94, 53, 64, 34, 38, 241, 214, 146, 239, 217, 63, 22, 40, 10, 80, 5, 111, 76, 114, 49},
{105, 42, 103, 163, 188, 192, 226, 244, 40, 141, 241, 201, 248, 245, 166, 150, 203, 257, 200, 5, 37, 55, 115, 61, 102, 104, 53, 50, 62},
{95, 144, 158, 105, 239, 204, 194, 177, 18, 210, 23, 15, 83, 21, 130, 42, 39, 30, 72, 239, 169, 217, 221, 76, 88, 61, 72, 150, 16},
{114, 95, 246, 42, 222, 233, 189, 245, 186, 118, 216, 196, 216, 182, 235, 110, 99, 114, 96, 229, 224, 233, 252, 147, 9, 103, 45, 90, 36},
{12, 158, 114, 31, 133, 134, 197, 151, 173, 151, 2, 70, 80, 83, 38, 234, 235, 170, 252, 205, 126, 247, 210, 79, 104, 110, 71, 38, 66},
{100, 12, 95, 103, 238, 191, 180, 175, 125, 19, 120, 137, 208, 168, 110, 233, 188, 214, 158, 175, 218, 246, 237, 59, 70, 31, 110, 20, 33},
{20, 133, 239, 162, 100, 202, 190, 252, 46, 99, 243, 60, 118, 111, 48, 169, 105, 77, 88, 159, 49, 19, 59, 223, 231, 152, 44, 94, 21},
{125, 20, 222, 185, 12, 157, 146, 142, 246, 193, 104, 176, 195, 244, 35, 144, 114, 29, 66, 212, 128, 71, 108, 133, 199, 125, 61, 75, 15},
{36, 239, 125, 247, 114, 130, 251, 154, 232, 74, 166, 55, 55, 104, 167, 27, 198, 134, 136, 224, 111, 4, 135, 252, 190, 232, 13, 27, 109},
{19, 36, 20, 188, 95, 172, 198, 160, 127, 254, 113, 127, 252, 164, 185, 112, 154, 206, 215, 158, 68, 81, 73, 202, 191, 169, 60, 2, 115},
{70, 247, 162, 19, 105, 205, 166, 171, 96, 241, 197, 54, 12, 122, 209, 172, 67, 21, 53, 113, 181, 216, 170, 175, 226, 207, 41, 10, 84},
{11, 70, 185, 36, 42, 206, 254, 132, 184, 47, 91, 200, 173, 170, 155, 223, 109, 89, 80, 114, 133, 249, 158, 222, 243, 245, 14, 89, 30},
{73, 162, 11, 125, 31, 136, 196, 145, 190, 250, 146, 69, 13, 39, 29, 79, 216, 246, 140, 32, 216, 140, 146, 116, 233, 199, 33, 48, 9},
{96, 73, 70, 20, 103, 186, 241, 220, 28, 130, 4, 136, 199, 149, 46, 95, 182, 188, 144, 122, 202, 168, 234, 212, 257, 251, 92, 105, 113},
{72, 136, 205, 202, 231, 96, 165, 170, 41, 95, 131, 168, 32, 85, 86, 87, 213, 40, 29, 82, 220, 32, 105, 165, 144, 29, 230, 249, 131},
{122, 72, 206, 157, 137, 73, 219, 174, 199, 202, 233, 63, 157, 162, 123, 45, 228, 48, 40, 125, 153, 78, 81, 247, 81, 6, 176, 158, 154},
{52, 205, 122, 130, 209, 11, 224, 203, 174, 60, 63, 186, 41, 26, 178, 136, 100, 197, 190, 4, 243, 27, 47, 241, 56, 122, 241, 145, 31},
{53, 52, 72, 172, 192, 70, 200, 236, 120, 112, 207, 6, 220, 208, 199, 218, 74, 183, 146, 49, 247, 28, 8, 171, 34, 67, 178, 172, 117},
{86, 130, 202, 53, 204, 19, 253, 248, 97, 191, 8, 205, 29, 44, 238, 77, 236, 24, 106, 202, 39, 178, 240, 162, 94, 116, 167, 186, 151},
{128, 86, 157, 52, 233, 36, 223, 212, 197, 31, 191, 12, 244, 253, 125, 71, 108, 63, 64, 187, 21, 253, 192, 151, 97, 64, 209, 213, 44},
{101, 202, 128, 122, 134, 125, 201, 187, 177, 190, 57, 230, 45, 92, 77, 253, 27, 226, 220, 118, 71, 210, 187, 238, 73, 1, 130, 142, 116},
{56, 101, 86, 72, 191, 20, 210, 169, 123, 100, 125, 130, 189, 191, 66, 175, 1, 250, 241, 207, 90, 174, 166, 138, 119, 11, 184, 257, 72},
{67, 134, 204, 231, 56, 100, 138, 256, 19, 126, 160, 214, 76, 15, 106, 197, 197, 83, 121, 206, 131, 111, 31, 40, 250, 176, 218, 146, 55},
{124, 67, 233, 137, 101, 12, 176, 228, 195, 232, 56, 66, 125, 247, 78, 132, 133, 102, 34, 112, 225, 120, 11, 139, 194, 172, 188, 154, 119},
{25, 204, 124, 209, 128, 114, 156, 208, 185, 63, 173, 142, 75, 118, 143, 109, 34, 194, 237, 182, 245, 74, 117, 47, 220, 221, 183, 236, 82},
{54, 25, 67, 192, 86, 95, 179, 140, 122, 207, 135, 109, 146, 207, 252, 63, 79, 145, 234, 204, 150, 39, 44, 14, 240, 253, 256, 143, 10},
{66, 209, 231, 54, 53, 105, 215, 199, 126, 251, 193, 199, 60, 98, 218, 207, 138, 136, 20, 68, 115, 182, 174, 69, 196, 209, 219, 235, 13},
{49, 66, 137, 25, 52, 42, 193, 148, 231, 54, 41, 111, 205, 190, 165, 100, 229, 86, 14, 120, 134, 163, 189, 13, 209, 239, 215, 250, 136},
{121, 231, 49, 124, 122, 31, 173, 234, 183, 171, 177, 153, 106, 16, 32, 48, 40, 193, 164, 92, 59, 175, 236, 54, 215, 164, 239, 209, 108},
{27, 121, 66, 67, 72, 103, 139, 240, 55, 122, 109, 37, 164, 231, 57, 18, 110, 235, 206, 36, 102, 145, 241, 82, 165, 189, 249, 228, 69},
{119, 173, 215, 138, 165, 257, 27, 182, 86, 103, 37, 8, 150, 110, 37, 47, 49, 180, 5, 40, 55, 183, 53, 30, 223, 108, 114, 36, 236},
{85, 119, 193, 176, 219, 250, 121, 149, 233, 150, 158, 248, 43, 193, 98, 59, 15, 245, 111, 13, 60, 127, 113, 71, 126, 75, 203, 102, 239},
{65, 215, 85, 156, 224, 242, 49, 147, 149, 82, 55, 46, 210, 24, 142, 217, 255, 92, 141, 64, 69, 214, 95, 16, 218, 17, 199, 52, 148},
{43, 65, 119, 179, 200, 226, 66, 249, 111, 154, 219, 244, 56, 152, 140, 192, 254, 71, 197, 12, 18, 181, 80, 110, 221, 121, 165, 76, 200},
{79, 156, 138, 43, 253, 194, 54, 214, 108, 188, 13, 80, 243, 117, 121, 16, 51, 185, 62, 186, 230, 36, 222, 48, 176, 90, 123, 13, 124},
{102, 79, 176, 65, 223, 189, 25, 150, 148, 46, 249, 218, 61, 217, 145, 39, 63, 184, 113, 196, 235, 37, 180, 19, 123, 34, 107, 34, 175},
{82, 138, 102, 85, 201, 197, 124, 178, 194, 139, 34, 81, 204, 84, 50, 221, 176, 9, 135, 247, 251, 104, 122, 106, 149, 85, 246, 55, 234},
{120, 82, 79, 119, 210, 180, 67, 184, 121, 79, 194, 222, 128, 189, 30, 248, 201, 81, 233, 173, 229, 13, 202, 52, 205, 92, 154, 46, 235},
{48, 201, 253, 165, 120, 190, 56, 183, 43, 52, 180, 91, 161, 91, 28, 164, 94, 182, 102, 128, 3, 194, 134, 207, 113, 181, 240, 44, 162},
{57, 48, 223, 219, 82, 146, 101, 159, 251, 183, 128, 166, 58, 177, 44, 183, 11, 140, 4, 142, 44, 144, 114, 227, 124, 139, 131, 3, 212},
{35, 253, 57, 224, 102, 251, 128, 243, 234, 73, 169, 49, 179, 108, 206, 81, 189, 120, 142, 198, 45, 192, 12, 229, 20, 130, 226, 88, 216},
{5, 35, 48, 200, 79, 198, 86, 161, 67, 237, 108, 251, 135, 251, 204, 138, 178, 91, 160, 233, 47, 173, 120, 185, 45, 213, 257, 92, 246},
{58, 224, 165, 5, 43, 166, 53, 131, 51, 245, 182, 13, 214, 10, 184, 121, 2, 179, 124, 79, 227, 96, 149, 226, 69, 230, 190, 64, 223},
{34, 58, 219, 35, 65, 254, 52, 232, 206, 34, 19, 173, 65, 124, 231, 157, 91, 162, 109, 96, 233, 117, 244, 257, 51, 145, 206, 37, 206},
{39, 165, 34, 57, 85, 196, 122, 152, 221, 216, 119, 33, 192, 121, 13, 54, 240, 50, 239, 110, 183, 98, 140, 225, 101, 127, 171, 12, 188},
{93, 39, 58, 48, 119, 241, 72, 141, 54, 62, 68, 227, 134, 163, 12, 26, 160, 69, 132, 139, 162, 18, 176, 243, 41, 217, 160, 139, 157},
{17, 196, 166, 190, 257, 93, 96, 135, 3, 98, 20, 224, 185, 28, 69, 21, 226, 152, 93, 77, 196, 211, 29, 145, 187, 52, 54, 165, 233},
{62, 17, 254, 146, 250, 39, 73, 237, 155, 168, 257, 74, 89, 127, 3, 13, 210, 255, 65, 39, 194, 158, 42, 213, 242, 115, 96, 161, 185},
{4, 166, 62, 251, 242, 34, 11, 221, 228, 24, 44, 211, 174, 103, 162, 235, 72, 53, 240, 35, 215, 227, 72, 230, 203, 80, 31, 254, 146},
{92, 4, 17, 198, 226, 58, 70, 167, 42, 217, 253, 43, 4, 229, 163, 257, 77, 107, 151, 97, 212, 257, 127, 216, 147, 74, 120, 127, 163},
{60, 251, 190, 92, 194, 5, 19, 143, 105, 238, 73, 237, 219, 80, 253, 46, 205, 225, 57, 183, 61, 59, 165, 188, 158, 36, 46, 124, 194},
{7, 60, 146, 4, 189, 35, 36, 168, 189, 71, 186, 29, 7, 138, 175, 33, 211, 133, 49, 250, 16, 21, 231, 166, 138, 57, 85, 247, 197},
{112, 190, 7, 62, 197, 57, 125, 255, 205, 214, 59, 234, 228, 54, 31, 162, 12, 20, 168, 243, 50, 57, 208, 143, 239, 27, 81, 149, 147},
{68, 112, 60, 17, 180, 48, 20, 218, 10, 32, 187, 89, 114, 257, 89, 143, 14, 35, 181, 252, 35, 123, 119, 126, 174, 96, 56, 188, 240},
{78, 197, 194, 257, 68, 120, 100, 195, 65, 10, 157, 143, 239, 40, 36, 186, 234, 161, 15, 251, 145, 152, 25, 91, 57, 248, 78, 177, 155},
{61, 78, 189, 250, 112, 82, 12, 211, 187, 256, 87, 103, 98, 184, 18, 256, 156, 137, 36, 185, 138, 243, 96, 74, 121, 225, 145, 163, 170},
{69, 194, 61, 242, 7, 102, 114, 213, 182, 38, 206, 170, 148, 22, 256, 113, 135, 17, 211, 255, 244, 215, 41, 67, 29, 193, 42, 141, 218},
{64, 69, 78, 226, 60, 79, 95, 230, 50, 246, 11, 83, 104, 205, 257, 78, 136, 100, 179, 163, 180, 149, 39, 18, 11, 141, 115, 199, 166},
{32, 242, 257, 64, 92, 43, 105, 181, 8, 186, 255, 182, 212, 34, 191, 227, 127, 164, 31, 27, 83, 115, 256, 55, 91, 165, 29, 244, 253},
{16, 32, 250, 69, 4, 65, 42, 207, 254, 81, 88, 27, 100, 153, 224, 166, 192, 243, 108, 1, 92, 136, 223, 86, 3, 233, 5, 147, 257},
{8, 257, 16, 61, 62, 85, 31, 164, 237, 173, 234, 174, 142, 20, 68, 3, 75, 45, 242, 109, 144, 8, 207, 10, 36, 235, 102, 126, 201},
{1, 8, 32, 78, 17, 119, 103, 235, 22, 58, 47, 96, 23, 158, 105, 88, 64, 119, 187, 104, 81, 97, 173, 101, 116, 158, 34, 181, 210},
{23, 164, 181, 195, 135, 182, 225, 1, 39, 55, 36, 86, 47, 255, 10, 117, 19, 2, 221, 30, 26, 94, 167, 90, 44, 223, 21, 190, 153},
{94, 23, 207, 211, 237, 149, 217, 8, 164, 197, 148, 165, 233, 4, 24, 58, 44, 4, 212, 20, 86, 101, 203, 95, 125, 174, 30, 240, 252},
{51, 181, 94, 213, 221, 147, 229, 16, 220, 14, 103, 4, 22, 241, 164, 252, 172, 200, 8, 101, 121, 29, 195, 25, 103, 240, 105, 198, 220},
{77, 51, 23, 230, 167, 249, 244, 32, 93, 215, 229, 139, 242, 37, 174, 173, 126, 232, 1, 155, 94, 61, 206, 114, 96, 140, 111, 191, 126},
{28, 213, 195, 77, 143, 214, 177, 64, 48, 219, 90, 107, 28, 188, 219, 60, 68, 68, 188, 152, 239, 244, 26, 77, 92, 171, 136, 187, 152},
{45, 28, 211, 51, 168, 150, 245, 69, 196, 89, 134, 207, 186, 66, 177, 4, 20, 116, 174, 220, 160, 220, 16, 21, 99, 256, 63, 200, 137},
{47, 195, 45, 94, 255, 178, 151, 61, 235, 228, 33, 98, 95, 197, 15, 184, 130, 230, 73, 237, 161, 213, 1, 68, 85, 109, 20, 175, 251},
{63, 47, 28, 23, 218, 184, 175, 78, 45, 84, 244, 213, 155, 77, 64, 135, 171, 199, 69, 169, 193, 205, 20, 53, 12, 226, 16, 155, 123},
{40, 255, 143, 135, 63, 183, 252, 68, 89, 36, 213, 79, 119, 145, 70, 236, 119, 108, 199, 248, 136, 49, 251, 153, 186, 60, 84, 197, 219},
{3, 40, 168, 237, 47, 159, 142, 112, 134, 149, 75, 157, 253, 63, 56, 230, 5, 95, 203, 165, 103, 107, 183, 170, 168, 43, 101, 202, 130},
{90, 143, 3, 221, 45, 243, 154, 7, 222, 110, 242, 102, 109, 206, 215, 69, 170, 178, 99, 230, 84, 126, 255, 135, 225, 48, 47, 185, 202},
{115, 90, 40, 167, 28, 161, 160, 60, 6, 235, 50, 249, 151, 6, 168, 99, 163, 244, 32, 227, 139, 50, 155, 200, 198, 68, 48, 205, 229},
{91, 221, 135, 115, 77, 131, 171, 92, 15, 167, 212, 17, 122, 250, 195, 239, 118, 49, 245, 22, 142, 190, 67, 232, 245, 12, 36, 195, 177},
{37, 91, 237, 90, 51, 232, 132, 4, 139, 142, 77, 126, 188, 82, 250, 165, 89, 110, 169, 41, 246, 150, 4, 154, 228, 8, 66, 233, 254},
{21, 135, 37, 3, 94, 152, 145, 62, 179, 218, 220, 141, 35, 249, 79, 50, 220, 165, 28, 43, 195, 155, 132, 108, 182, 3, 51, 211, 190},
{123, 21, 91, 40, 23, 141, 220, 17, 34, 6, 94, 163, 141, 60, 139, 67, 121, 173, 74, 105, 191, 241, 58, 177, 244, 50, 103, 179, 133},
{117, 152, 131, 183, 182, 123, 170, 93, 13, 97, 117, 236, 87, 223, 111, 12, 180, 27, 214, 67, 192, 82, 224, 221, 24, 161, 235, 39, 179},
{106, 117, 232, 159, 149, 21, 174, 39, 112, 170, 200, 108, 165, 78, 7, 119, 199, 25, 236, 84, 249, 91, 225, 140, 21, 177, 139, 91, 158},
{26, 131, 106, 243, 147, 37, 203, 34, 116, 83, 32, 152, 11, 202, 171, 168, 33, 141, 26, 107, 151, 60, 215, 209, 105, 204, 137, 5, 199},
{127, 26, 117, 161, 249, 91, 236, 58, 83, 225, 153, 30, 207, 43, 154, 205, 56, 131, 54, 80, 141, 20, 249, 160, 61, 202, 201, 106, 213},
{97, 243, 183, 127, 214, 115, 248, 5, 77, 229, 115, 159, 49, 225, 120, 107, 208, 67, 231, 241, 79, 242, 70, 193, 112, 146, 193, 78, 195},
{15, 97, 159, 26, 150, 90, 212, 35, 180, 125, 210, 123, 249, 2, 144, 56, 212, 54, 202, 156, 73, 202, 40, 215, 42, 175, 163, 35, 161},
{99, 183, 15, 106, 178, 3, 187, 57, 130, 115, 80, 242, 77, 216, 41, 140, 96, 207, 42, 126, 27, 171, 30, 180, 10, 188, 150, 123, 164},
{75, 99, 97, 117, 184, 40, 169, 48, 5, 86, 189, 11, 176, 55, 107, 209, 103, 215, 7, 221, 118, 193, 86, 195, 23, 159, 255, 80, 181},
{74, 178, 214, 182, 75, 63, 256, 120, 91, 113, 162, 180, 26, 165, 16, 185, 245, 132, 195, 134, 234, 26, 175, 23, 171, 44, 161, 69, 180},
{80, 74, 150, 149, 99, 47, 228, 82, 198, 178, 83, 42, 250, 115, 122, 241, 174, 18, 125, 160, 170, 33, 152, 3, 172, 39, 158, 66, 144},
{108, 214, 80, 147, 15, 45, 208, 102, 168, 5, 159, 226, 105, 172, 183, 91, 85, 247, 85, 197, 254, 119, 197, 99, 192, 62, 233, 29, 193},
{44, 108, 74, 249, 97, 28, 140, 79, 44, 131, 40, 35, 196, 101, 193, 70, 62, 217, 97, 135, 184, 70, 181, 121, 252, 22, 196, 28, 184},
{9, 147, 182, 44, 127, 77, 199, 43, 104, 147, 232, 208, 111, 185, 186, 134, 221, 34, 155, 88, 17, 148, 110, 72, 230, 58, 250, 24, 205},
{111, 9, 149, 108, 26, 51, 148, 65, 171, 49, 132, 68, 187, 31, 205, 131, 147, 6, 157, 33, 41, 228, 87, 50, 212, 55, 247, 31, 232},
{109, 182, 111, 80, 106, 94, 234, 85, 244, 189, 136, 239, 18, 139, 124, 43, 31, 157, 76, 70, 95, 234, 97, 4, 136, 9, 185, 6, 138},
{76, 109, 9, 74, 117, 23, 240, 119, 98, 57, 102, 114, 225, 89, 62, 103, 54, 192, 23, 86, 104, 255, 98, 109, 169, 123, 236, 17, 224},
{18, 234, 199, 256, 170, 225, 76, 27, 115, 45, 107, 25, 159, 142, 71, 20, 35, 227, 180, 69, 91, 191, 184, 34, 150, 180, 232, 160, 98},
{24, 18, 148, 228, 174, 217, 109, 121, 211, 257, 174, 219, 90, 36, 47, 111, 8, 146, 191, 78, 112, 166, 157, 112, 151, 95, 189, 232, 89},
{110, 199, 24, 208, 203, 229, 111, 49, 242, 123, 27, 26, 256, 211, 198, 160, 232, 82, 87, 44, 36, 137, 198, 122, 206, 186, 208, 226, 90},
{59, 110, 18, 140, 236, 244, 9, 66, 31, 199, 209, 257, 126, 72, 244, 193, 215, 84, 30, 42, 57, 172, 235, 75, 131, 211, 223, 133, 14},
{118, 208, 256, 59, 248, 177, 44, 54, 35, 144, 43, 20, 166, 233, 197, 68, 73, 159, 210, 211, 207, 10, 45, 100, 163, 170, 243, 148, 71},
{50, 118, 228, 110, 212, 245, 108, 25, 256, 76, 251, 181, 64, 62, 203, 97, 106, 196, 148, 193, 252, 99, 89, 128, 232, 124, 200, 136, 91},
{30, 256, 50, 24, 187, 151, 80, 124, 141, 236, 74, 120, 190, 201, 101, 194, 237, 105, 33, 164, 238, 102, 104, 85, 130, 138, 191, 237, 86},
{2, 30, 118, 18, 169, 175, 74, 67, 57, 16, 179, 183, 86, 128, 45, 244, 165, 57, 75, 203, 182, 69, 38, 2, 200, 238, 231, 157, 87},
{89, 187, 248, 170, 2, 252, 75, 56, 58, 124, 240, 94, 170, 144, 4, 142, 116, 186, 167, 240, 30, 153, 209, 155, 106, 38, 234, 218, 20},
{71, 89, 212, 174, 30, 142, 99, 101, 249, 194, 6, 235, 78, 46, 100, 243, 113, 236, 119, 232, 110, 133, 137, 156, 74, 101, 254, 107, 75},
{46, 248, 71, 203, 50, 154, 15, 128, 176, 25, 147, 110, 156, 171, 156, 9, 206, 115, 115, 131, 158, 236, 194, 251, 98, 40, 132, 217, 28},
{10, 46, 89, 236, 118, 160, 97, 86, 66, 205, 66, 145, 20, 132, 173, 38, 252, 60, 45, 242, 52, 116, 196, 169, 48, 42, 172, 239, 17},
{22, 203, 170, 10, 59, 171, 127, 53, 99, 231, 142, 133, 237, 199, 159, 228, 7, 155, 175, 87, 159, 112, 79, 219, 4, 30, 175, 171, 51},
{55, 22, 174, 46, 110, 132, 26, 52, 178, 9, 54, 197, 131, 49, 207, 222, 117, 135, 138, 50, 123, 6, 125, 253, 117, 79, 220, 251, 47},
{84, 170, 55, 71, 24, 145, 106, 122, 247, 180, 230, 52, 137, 183, 22, 104, 134, 46, 60, 23, 200, 128, 102, 201, 75, 105, 221, 176, 43},
{88, 84, 22, 89, 18, 220, 117, 72, 70, 93, 12, 171, 113, 125, 21, 62, 187, 128, 35, 24, 253, 54, 28, 214, 78, 46, 133, 159, 76},
{38, 145, 171, 252, 225, 88, 123, 96, 113, 106, 60, 161, 234, 187, 109, 8, 242, 149, 176, 57, 152, 212, 239, 246, 241, 185, 109, 96, 41},
{113, 38, 132, 142, 217, 84, 21, 73, 158, 243, 237, 104, 120, 99, 82, 32, 166, 219, 255, 117, 176, 256, 159, 231, 148, 242, 134, 118, 85},
{126, 171, 113, 154, 229, 55, 37, 11, 202, 77, 31, 253, 167, 194, 146, 201, 97, 5, 81, 2, 205, 169, 212, 131, 179, 237, 7, 85, 11},
{87, 126, 38, 160, 244, 22, 91, 70, 88, 121, 248, 73, 27, 12, 232, 247, 90, 87, 127, 121, 248, 165, 253, 198, 175, 167, 146, 42, 80},
{98, 154, 252, 87, 177, 10, 115, 19, 95, 120, 22, 220, 162, 213, 247, 55, 151, 203, 246, 192, 1, 88, 65, 141, 227, 195, 117, 84, 93},
{104, 98, 142, 126, 245, 46, 90, 36, 238, 66, 188, 18, 115, 18, 108, 105, 177, 220, 154, 249, 70, 52, 15, 234, 219, 151, 10, 120, 27},
{116, 252, 104, 113, 151, 71, 3, 125, 151, 208, 93, 193, 218, 239, 131, 224, 36, 90, 39, 168, 85, 23, 54, 220, 195, 234, 116, 74, 118},
{6, 116, 98, 38, 175, 89, 40, 20, 102, 102, 152, 112, 3, 123, 58, 178, 9, 98, 51, 244, 54, 134, 94, 236, 204, 160, 99, 51, 67},
{83, 151, 177, 225, 6, 2, 63, 100, 1, 37, 163, 217, 191, 226, 42, 130, 175, 251, 172, 184, 226, 251, 168, 94, 47, 76, 88, 54, 50},
{107, 83, 245, 217, 116, 30, 47, 12, 144, 223, 35, 2, 57, 79, 9, 176, 217, 154, 153, 150, 210, 185, 226, 62, 7, 114, 89, 130, 99},
{13, 177, 107, 229, 104, 50, 45, 114, 152, 3, 176, 134, 222, 156, 233, 84, 58, 47, 9, 111, 171, 224, 201, 64, 77, 66, 79, 33, 54},
{81, 13, 83, 244, 98, 118, 28, 95, 101, 230, 121, 48, 37, 112, 225, 29, 10, 37, 95, 200, 178, 186, 130, 9, 118, 4, 90, 43, 37},
{14, 229, 225, 81, 87, 59, 77, 105, 32, 247, 140, 140, 206, 200, 182, 206, 233, 248, 166, 59, 101, 35, 111, 8, 80, 53, 6, 16, 32},
{29, 14, 217, 13, 126, 110, 51, 42, 229, 92, 86, 71, 81, 97, 141, 133, 230, 147, 247, 115, 8, 51, 116, 66, 22, 15, 94, 57, 60},
{41, 225, 29, 107, 113, 24, 94, 31, 225, 153, 228, 255, 227, 151, 19, 106, 101, 56, 50, 81, 93, 58, 13, 41, 2, 71, 106, 121, 3},
{33, 41, 14, 83, 38, 18, 23, 103, 17, 96, 114, 56, 74, 23, 97, 76, 65, 42, 128, 48, 2, 79, 68, 84, 143, 126, 118, 15, 2},
{129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129},
{155, 31, 105, 100, 96, 27, 1, 33, 209, 240, 196, 158, 241, 210, 170, 156, 143, 242, 248, 213, 186, 195, 182, 161, 208, 244, 181, 196, 250},
{227, 155, 42, 12, 73, 121, 8, 41, 118, 101, 97, 67, 1, 30, 126, 251, 241, 222, 256, 167, 109, 196, 151, 124, 193, 156, 194, 193, 156},
{216, 105, 227, 114, 11, 49, 16, 29, 92, 164, 205, 194, 224, 220, 17, 44, 112, 19, 41, 195, 236, 218, 248, 178, 253, 147, 182, 144, 209},
{153, 216, 155, 95, 70, 66, 32, 14, 218, 117, 17, 57, 10, 13, 92, 108, 55, 1, 58, 253, 221, 203, 143, 197, 156, 154, 205, 208, 196},
{163, 114, 100, 153, 19, 54, 64, 81, 240, 48, 235, 243, 175, 237, 128, 216, 219, 228, 186, 19, 89, 41, 37, 182, 170, 197, 186, 108, 242},
{144, 163, 12, 216, 36, 25, 69, 13, 72, 140, 42, 62, 42, 76, 23, 148, 159, 144, 162, 29, 34, 25, 6, 111, 249, 155, 213, 168, 222},
{246, 100, 144, 227, 125, 124, 61, 107, 85, 107, 256, 188, 178, 175, 220, 24, 23, 88, 6, 53, 132, 11, 48, 179, 154, 148, 187, 220, 192},
{158, 246, 163, 155, 20, 67, 78, 83, 133, 239, 138, 121, 50, 90, 148, 25, 70, 44, 100, 83, 40, 12, 21, 199, 188, 227, 148, 238, 225},
{238, 125, 19, 96, 158, 56, 68, 6, 212, 159, 15, 198, 140, 147, 210, 89, 153, 181, 170, 99, 209, 239, 199, 35, 27, 106, 214, 164, 237},
{133, 238, 36, 73, 246, 101, 112, 116, 12, 65, 154, 82, 63, 14, 223, 114, 144, 229, 192, 46, 130, 187, 150, 125, 59, 133, 197, 183, 243},
{222, 19, 133, 11, 144, 128, 7, 104, 26, 184, 92, 203, 203, 154, 91, 231, 60, 124, 122, 34, 147, 254, 123, 6, 68, 26, 245, 231, 149},
{239, 222, 238, 70, 163, 86, 60, 98, 131, 4, 145, 131, 6, 94, 73, 146, 104, 52, 43, 100, 190, 177, 185, 56, 67, 89, 198, 256, 143},
{188, 11, 96, 239, 153, 53, 92, 87, 162, 17, 61, 204, 246, 136, 49, 86, 191, 237, 205, 145, 77, 42, 88, 83, 32, 51, 217, 248, 174},
{247, 188, 73, 222, 216, 52, 4, 126, 74, 211, 167, 58, 85, 88, 103, 35, 149, 169, 178, 144, 125, 9, 100, 36, 15, 13, 244, 169, 228},
{185, 96, 247, 133, 227, 122, 62, 113, 68, 8, 112, 189, 245, 219, 229, 179, 42, 12, 118, 226, 42, 118, 112, 142, 25, 59, 225, 210, 249},
{162, 185, 188, 238, 155, 72, 17, 38, 230, 128, 254, 122, 59, 109, 212, 163, 76, 70, 114, 136, 56, 90, 24, 46, 1, 7, 166, 153, 145},
{186, 122, 53, 56, 27, 162, 93, 88, 217, 163, 127, 90, 226, 173, 172, 171, 45, 218, 229, 176, 38, 226, 153, 93, 114, 229, 28, 9, 127},
{136, 186, 52, 101, 121, 185, 39, 84, 59, 56, 25, 195, 101, 96, 135, 213, 30, 210, 218, 133, 105, 180, 177, 11, 177, 252, 82, 100, 104},
{206, 53, 136, 128, 49, 247, 34, 55, 84, 198, 195, 72, 217, 232, 80, 122, 158, 61, 68, 254, 15, 231, 211, 17, 202, 136, 17, 113, 227},
{205, 206, 186, 86, 66, 188, 58, 22, 138, 146, 51, 252, 38, 50, 59, 40, 184, 75, 112, 209, 11, 230, 250, 87, 224, 191, 80, 86, 141},
{172, 128, 56, 205, 54, 239, 5, 10, 161, 67, 250, 53, 229, 214, 20, 181, 22, 234, 152, 56, 219, 80, 18, 96, 164, 142, 91, 72, 107},
{130, 172, 101, 206, 25, 222, 35, 46, 61, 227, 67, 246, 14, 5, 133, 187, 150, 195, 194, 71, 237, 5, 66, 107, 161, 194, 49, 45, 214},
{157, 56, 130, 136, 124, 133, 57, 71, 81, 68, 201, 28, 213, 166, 181, 5, 231, 32, 38, 140, 187, 48, 71, 20, 185, 257, 128, 116, 142},
{202, 157, 172, 186, 67, 238, 48, 89, 135, 158, 133, 128, 69, 67, 192, 83, 257, 8, 17, 51, 168, 84, 92, 120, 139, 247, 74, 1, 186},
{191, 124, 54, 27, 202, 158, 120, 2, 239, 132, 98, 44, 182, 243, 152, 61, 61, 175, 137, 52, 127, 147, 227, 218, 8, 82, 40, 112, 203},
{134, 191, 25, 121, 157, 246, 82, 30, 63, 26, 202, 192, 133, 11, 180, 126, 125, 156, 224, 146, 33, 138, 247, 119, 64, 86, 70, 104, 139},
{233, 54, 134, 49, 130, 144, 102, 50, 73, 195, 85, 116, 183, 140, 115, 149, 224, 64, 21, 76, 13, 184, 141, 211, 38, 37, 75, 22, 176},
{204, 233, 191, 66, 172, 163, 79, 118, 136, 51, 123, 149, 112, 51, 6, 195, 179, 113, 24, 54, 108, 219, 214, 244, 18, 5, 2, 115, 248},
{192, 49, 27, 204, 205, 153, 43, 59, 132, 7, 65, 59, 198, 160, 40, 51, 120, 122, 238, 190, 143, 76, 84, 189, 62, 49, 39, 23, 245},
{209, 192, 121, 233, 206, 216, 65, 110, 27, 204, 217, 147, 53, 68, 93, 158, 29, 172, 244, 138, 124, 95, 69, 245, 49, 19, 43, 8, 122},
{137, 27, 209, 134, 136, 227, 85, 24, 75, 87, 81, 105, 152, 242, 226, 210, 218, 65, 94, 166, 199, 83, 22, 204, 43, 94, 19, 49, 150},
{231, 137, 192, 191, 186, 155, 119, 18, 203, 136, 149, 221, 94, 27, 201, 240, 148, 23, 52, 222, 156, 113, 17, 176, 93, 69, 9, 30, 189},
{139, 85, 43, 120, 93, 1, 231, 76, 172, 155, 221, 250, 108, 148, 221, 211, 209, 78, 253, 218, 203, 75, 205, 228, 35, 150, 144, 222, 22},
{173, 139, 65, 82, 39, 8, 137, 109, 25, 108, 100, 10, 215, 65, 160, 199, 243, 13, 147, 245, 198, 131, 145, 187, 132, 183, 55, 156, 19},
{193, 43, 173, 102, 34, 16, 209, 111, 109, 176, 203, 212, 48, 234, 116, 41, 3, 166, 117, 194, 189, 44, 163, 242, 40, 241, 59, 206, 110},
{215, 193, 139, 79, 58, 32, 192, 9, 147, 104, 39, 14, 202, 106, 118, 66, 4, 187, 61, 246, 240, 77, 178, 148, 37, 137, 93, 182, 58},
{179, 102, 120, 215, 5, 64, 204, 44, 150, 70, 245, 178, 15, 141, 137, 242, 207, 73, 196, 72, 28, 222, 36, 210, 82, 168, 135, 245, 134},
{156, 179, 82, 193, 35, 69, 233, 108, 110, 212, 9, 40, 197, 41, 113, 219, 195, 74, 145, 62, 23, 221, 78, 239, 135, 224, 151, 224, 83},
{176, 120, 156, 173, 57, 61, 134, 80, 64, 119, 224, 177, 54, 174, 208, 37, 82, 249, 123, 11, 7, 154, 136, 152, 109, 173, 12, 203, 24},
{138, 176, 179, 139, 48, 78, 191, 74, 137, 179, 64, 36, 130, 69, 228, 10, 57, 177, 25, 85, 29, 245, 56, 206, 53, 166, 104, 212, 23},
{210, 57, 5, 93, 138, 68, 202, 75, 215, 206, 78, 167, 97, 167, 230, 94, 164, 76, 156, 130, 255, 64, 124, 51, 145, 77, 18, 214, 96},
{201, 210, 35, 39, 176, 112, 157, 99, 7, 75, 130, 92, 200, 81, 214, 75, 247, 118, 254, 116, 214, 114, 144, 31, 134, 119, 127, 255, 46},
{223, 5, 201, 34, 156, 7, 130, 15, 24, 185, 89, 209, 79, 150, 52, 177, 69, 138, 116, 60, 213, 66, 246, 29, 238, 128, 32, 170, 42},
{253, 223, 210, 58, 179, 60, 172, 97, 191, 21, 150, 7, 123, 7, 54, 120, 80, 167, 98, 25, 211, 85, 138, 73, 213, 45, 1, 166, 12},
{200, 34, 93, 253, 215, 92, 205, 127, 207, 13, 76, 245, 44, 248, 74, 137, 256, 79, 134, 179, 31, 162, 109, 32, 189, 28, 68, 194, 35},
{224, 200, 39, 223, 193, 4, 206, 26, 52, 224, 239, 85, 193, 134, 27, 101, 167, 96, 149, 162, 25, 141, 14, 1, 207, 113, 52, 221, 52},
{219, 93, 224, 201, 173, 62, 136, 106, 37, 42, 139, 225, 66, 137, 245, 204, 18, 208, 19, 148, 75, 160, 118, 33, 157, 131, 87, 246, 70},
{165, 219, 200, 210, 139, 17, 186, 117, 204, 196, 190, 31, 124, 95, 246, 232, 98, 189, 126, 119, 96, 240, 82, 15, 217, 41, 98, 119, 101},
{241, 62, 92, 68, 1, 165, 162, 123, 255, 160, 238, 34, 73, 230, 189, 237, 32, 106, 165, 181, 62, 47, 229, 113, 71, 206, 204, 93, 25},
{196, 241, 4, 112, 8, 219, 185, 21, 103, 90, 1, 184, 169, 131, 255, 245, 48, 3, 193, 219, 64, 100, 216, 45, 16, 143, 162, 97, 73},
{254, 92, 196, 7, 16, 224, 247, 37, 30, 234, 214, 47, 84, 155, 96, 23, 186, 205, 18, 223, 43, 31, 186, 28, 55, 178, 227, 4, 112},
{166, 254, 241, 60, 32, 200, 188, 91, 216, 41, 5, 215, 254, 29, 95, 1, 181, 151, 107, 161, 46, 1, 131, 42, 111, 184, 138, 131, 95},
{198, 7, 68, 166, 64, 253, 239, 115, 153, 20, 185, 21, 39, 178, 5, 212, 53, 33, 201, 75, 197, 199, 93, 70, 100, 222, 212, 134, 64},
{251, 198, 112, 254, 69, 223, 222, 90, 69, 187, 72, 229, 251, 120, 83, 225, 47, 125, 209, 8, 242, 237, 27, 92, 120, 201, 173, 11, 61},
{146, 68, 251, 196, 61, 201, 133, 3, 53, 44, 199, 24, 30, 204, 227, 96, 246, 238, 90, 15, 208, 201, 50, 115, 19, 231, 177, 109, 111},
{190, 146, 198, 241, 78, 210, 238, 40, 248, 226, 71, 169, 144, 1, 169, 115, 244, 223, 77, 6, 223, 135, 139, 132, 84, 162, 202, 70, 18},
{180, 61, 64, 1, 190, 138, 158, 63, 193, 248, 101, 115, 19, 218, 222, 72, 24, 97, 243, 7, 113, 106, 233, 167, 201, 10, 180, 81, 103},
{197, 180, 69, 8, 146, 176, 246, 47, 71, 2, 171, 155, 160, 74, 240, 2, 102, 121, 222, 73, 120, 15, 162, 184, 137, 33, 113, 95, 88},
{189, 64, 197, 16, 251, 156, 144, 45, 76, 220, 52, 88, 110, 236, 2, 145, 123, 241, 47, 3, 14, 43, 217, 191, 229, 65, 216, 117, 40},
{194, 189, 180, 32, 198, 179, 163, 28, 208, 12, 247, 175, 154, 53, 1, 180, 122, 158, 79, 95, 78, 109, 219, 240, 247, 117, 143, 59, 92},
{226, 16, 1, 194, 166, 215, 153, 77, 250, 72, 3, 76, 46, 224, 67, 31, 131, 94, 227, 231, 175, 143, 2, 203, 167, 93, 229, 14, 5},
{242, 226, 8, 189, 254, 193, 216, 51, 4, 177, 170, 231, 158, 105, 34, 92, 66, 15, 150, 257, 166, 122, 35, 172, 255, 25, 253, 111, 1},
{250, 1, 242, 197, 196, 173, 227, 94, 21, 85, 24, 84, 116, 238, 190, 255, 183, 213, 16, 149, 114, 250, 51, 248, 222, 23, 156, 132, 57},
{257, 250, 226, 180, 241, 139, 155, 23, 236, 200, 211, 162, 235, 100, 153, 170, 194, 139, 71, 154, 177, 161, 85, 157, 142, 100, 224, 77, 48},
{235, 94, 77, 63, 123, 76, 33, 257, 219, 203, 222, 172, 211, 3, 248, 141, 239, 256, 37, 228, 232, 164, 91, 168, 214, 35, 237, 68, 105},
{164, 235, 51, 47, 21, 109, 41, 250, 94, 61, 110, 93, 25, 254, 234, 200, 214, 254, 46, 238, 172, 157, 55, 163, 133, 84, 228, 18, 6},
{207, 77, 164, 45, 37, 111, 29, 242, 38, 244, 155, 254, 236, 17, 94, 6, 86, 58, 250, 157, 137, 229, 63, 233, 155, 18, 153, 60, 38},
{181, 207, 235, 28, 91, 9, 14, 226, 165, 43, 29, 119, 16, 221, 84, 85, 132, 26, 257, 103, 164, 197, 52, 144, 162, 118, 147, 67, 132},
{230, 45, 63, 181, 115, 44, 81, 194, 210, 39, 168, 151, 230, 70, 39, 198, 190, 190, 70, 106, 19, 14, 232, 181, 166, 87, 122, 71, 106},
{213, 230, 47, 207, 90, 108, 13, 189, 62, 169, 124, 51, 72, 192, 81, 254, 238, 142, 84, 38, 98, 38, 242, 237, 159, 2, 195, 58, 121},
{211, 63, 213, 164, 3, 80, 107, 197, 23, 30, 225, 160, 163, 61, 243, 74, 128, 28, 185, 21, 97, 45, 257, 190, 173, 149, 238, 83, 7},
{195, 211, 230, 235, 40, 74, 83, 180, 213, 174, 14, 45, 103, 181, 194, 123, 87, 59, 189, 89, 65, 53, 238, 205, 246, 32, 242, 103, 135},
{218, 3, 115, 123, 195, 75, 6, 190, 169, 222, 45, 179, 139, 113, 188, 22, 139, 150, 59, 10, 122, 209, 7, 105, 72, 198, 174, 61, 39},
{255, 218, 90, 21, 211, 99, 116, 146, 124, 109, 183, 101, 5, 195, 202, 28, 253, 163, 55, 93, 155, 151, 75, 88, 90, 215, 157, 56, 128},
{168, 115, 255, 37, 213, 15, 104, 251, 36, 148, 16, 156, 149, 52, 43, 189, 88, 80, 159, 28, 174, 132, 3, 123, 33, 210, 211, 73, 56},
{143, 168, 218, 91, 230, 97, 98, 198, 252, 23, 208, 9, 107, 252, 90, 159, 95, 14, 226, 31, 119, 208, 103, 58, 60, 190, 210, 53, 29},
{167, 37, 123, 143, 181, 127, 87, 166, 243, 91, 46, 241, 136, 8, 63, 19, 140, 209, 13, 236, 116, 68, 191, 26, 13, 246, 222, 63, 81},
{221, 167, 21, 168, 207, 26, 126, 254, 119, 116, 181, 132, 70, 176, 8, 93, 169, 148, 89, 217, 12, 108, 254, 104, 30, 250, 192, 25, 4},
{237, 123, 221, 255, 164, 106, 113, 196, 79, 40, 38, 117, 223, 9, 179, 208, 38, 93, 230, 215, 63, 103, 126, 150, 76, 255, 207, 47, 68},
{135, 237, 167, 218, 235, 117, 38, 241, 224, 252, 164, 95, 117, 198, 119, 191, 137, 85, 184, 153, 67, 17, 200, 81, 14, 208, 155, 79, 125},
{141, 106, 127, 75, 76, 135, 88, 165, 245, 161, 141, 22, 171, 35, 147, 246, 78, 231, 44, 191, 66, 176, 34, 37, 234, 97, 23, 219, 79},
{152, 141, 26, 99, 109, 237, 84, 219, 146, 88, 58, 150, 93, 180, 251, 139, 59, 233, 22, 174, 9, 167, 33, 118, 237, 81, 119, 167, 100},
{232, 127, 152, 15, 111, 221, 55, 224, 142, 175, 226, 106, 247, 56, 87, 90, 225, 117, 232, 151, 107, 198, 43, 49, 153, 54, 121, 253, 59},
{131, 232, 141, 97, 9, 167, 22, 200, 175, 33, 105, 228, 51, 215, 104, 53, 202, 127, 204, 178, 117, 238, 9, 98, 197, 56, 57, 152, 45},
{161, 15, 75, 131, 44, 143, 10, 253, 181, 29, 143, 99, 209, 33, 138, 151, 50, 191, 27, 17, 179, 16, 188, 65, 146, 112, 65, 180, 63},
{243, 161, 99, 232, 108, 168, 46, 223, 78, 133, 48, 135, 9, 256, 114, 202, 46, 204, 56, 102, 185, 56, 218, 43, 216, 83, 95, 223, 97},
{159, 75, 243, 152, 80, 255, 71, 201, 128, 143, 178, 16, 181, 42, 217, 118, 162, 51, 216, 132, 231, 87, 228, 78, 248, 70, 108, 135, 94},
{183, 159, 161, 141, 74, 218, 89, 210, 253, 172, 69, 247, 82, 203, 151, 49, 155, 43, 251, 37, 140, 65, 172, 63, 235, 99, 3, 178, 77},
{184, 80, 44, 76, 183, 195, 2, 138, 167, 145, 96, 78, 232, 93, 242, 73, 13, 126, 63, 124, 24, 232, 83, 235, 87, 214, 97, 189, 78},
{178, 184, 108, 109, 159, 211, 30, 176, 60, 80, 175, 216, 8, 143, 136, 17, 84, 240, 133, 98, 88, 225, 106, 255, 86, 219, 100, 192, 114},
{150, 44, 178, 111, 243, 213, 50, 156, 90, 253, 99, 32, 153, 86, 75, 167, 173, 11, 173, 61, 4, 139, 61, 159, 66, 196, 25, 229, 65},
{214, 150, 184, 9, 161, 230, 118, 179, 214, 127, 218, 223, 62, 157, 65, 188, 196, 41, 161, 123, 74, 188, 77, 137, 6, 236, 62, 230, 74},
{249, 111, 76, 214, 131, 181, 59, 215, 154, 111, 26, 50, 147, 73, 72, 124, 37, 224, 103, 170, 241, 110, 148, 186, 28, 200, 8, 234, 53},
{147, 249, 109, 150, 232, 207, 110, 193, 87, 209, 126, 190, 71, 227, 53, 127, 111, 252, 101, 225, 217, 30, 171, 208, 46, 203, 11, 227, 26},
{149, 76, 147, 178, 152, 164, 24, 173, 14, 69, 122, 19, 240, 119, 134, 215, 227, 101, 182, 188, 163, 24, 161, 254, 122, 249, 73, 252, 120},
{182, 149, 249, 184, 141, 235, 18, 139, 160, 201, 156, 144, 33, 169, 196, 155, 204, 66, 235, 172, 154, 3, 160, 149, 89, 135, 22, 241, 34},
{240, 24, 59, 2, 88, 33, 182, 231, 143, 213, 151, 233, 99, 116, 187, 238, 223, 31, 78, 189, 167, 67, 74, 224, 108, 78, 26, 98, 160},
{234, 240, 110, 30, 84, 41, 149, 137, 47, 1, 84, 39, 168, 222, 211, 147, 250, 112, 67, 180, 146, 92, 101, 146, 107, 163, 69, 26, 169},
{148, 59, 234, 50, 55, 29, 147, 209, 16, 135, 231, 232, 2, 47, 60, 98, 26, 176, 171, 214, 222, 121, 60, 136, 52, 72, 50, 32, 168},
{199, 148, 240, 118, 22, 14, 249, 192, 227, 59, 49, 1, 132, 186, 14, 65, 43, 174, 228, 216, 201, 86, 23, 183, 127, 47, 35, 125, 244},
{140, 50, 2, 199, 10, 81, 214, 204, 223, 114, 215, 238, 92, 25, 61, 190, 185, 99, 48, 47, 51, 248, 213, 158, 95, 88, 15, 110, 187},
{208, 140, 30, 148, 46, 13, 150, 233, 2, 182, 7, 77, 194, 196, 55, 161, 152, 62, 110, 65, 6, 159, 169, 130, 26, 134, 58, 122, 167},
{228, 2, 208, 234, 71, 107, 178, 134, 117, 22, 184, 138, 68, 57, 157, 64, 21, 153, 225, 94, 20, 156, 154, 173, 128, 120, 67, 21, 172},
{256, 228, 140, 240, 89, 83, 184, 191, 201, 242, 79, 75, 172, 130, 213, 14, 93, 201, 183, 55, 76, 189, 220, 256, 58, 20, 27, 101, 171},
{169, 71, 10, 88, 256, 6, 183, 202, 200, 134, 18, 164, 88, 114, 254, 116, 142, 72, 91, 18, 228, 105, 49, 103, 152, 220, 24, 40, 238},
{187, 169, 46, 84, 228, 116, 159, 157, 9, 64, 252, 23, 180, 212, 158, 15, 145, 22, 139, 26, 148, 125, 121, 102, 184, 157, 4, 151, 183},
{212, 10, 187, 55, 208, 104, 243, 130, 82, 233, 111, 148, 102, 87, 102, 249, 52, 143, 143, 127, 100, 22, 64, 7, 160, 218, 126, 41, 230},
{248, 212, 169, 22, 140, 98, 161, 172, 192, 53, 192, 113, 238, 126, 85, 220, 6, 198, 213, 16, 206, 142, 62, 89, 210, 216, 86, 19, 241},
{236, 55, 88, 248, 199, 87, 131, 205, 159, 27, 116, 125, 21, 59, 99, 30, 251, 103, 83, 171, 99, 146, 179, 39, 254, 228, 83, 87, 207},
{203, 236, 84, 212, 148, 126, 232, 206, 80, 249, 204, 61, 127, 209, 51, 36, 141, 123, 120, 208, 135, 252, 133, 5, 141, 179, 38, 7, 211},
{174, 88, 203, 187, 234, 113, 152, 136, 11, 78, 28, 206, 121, 75, 236, 154, 124, 212, 198, 235, 58, 130, 156, 57, 183, 153, 37, 82, 215},
{170, 174, 236, 169, 240, 38, 141, 186, 188, 165, 246, 87, 145, 133, 237, 196, 71, 130, 223, 234, 5, 204, 230, 44, 180, 212, 125, 99, 182},
{220, 113, 87, 6, 33, 170, 135, 162, 145, 152, 198, 97, 24, 71, 149, 250, 16, 109, 82, 201, 106, 46, 19, 12, 17, 73, 149, 162, 217},
{145, 220, 126, 116, 41, 174, 237, 185, 100, 15, 21, 154, 138, 159, 176, 226, 92, 39, 3, 141, 82, 2, 99, 27, 110, 16, 124, 140, 173},
{132, 87, 145, 104, 29, 203, 221, 247, 56, 181, 227, 5, 91, 64, 112, 57, 161, 253, 177, 256, 53, 89, 46, 127, 79, 21, 251, 173, 247},
{171, 132, 220, 98, 14, 236, 167, 188, 170, 137, 10, 185, 231, 246, 26, 11, 168, 171, 131, 137, 10, 93, 5, 60, 83, 91, 112, 216, 178},
{160, 104, 6, 171, 81, 248, 143, 239, 163, 138, 236, 38, 96, 45, 11, 203, 107, 55, 12, 66, 257, 170, 193, 117, 31, 63, 141, 174, 165},
{154, 160, 116, 132, 13, 212, 168, 222, 20, 192, 70, 240, 143, 240, 150, 153, 81, 38, 104, 9, 188, 206, 243, 24, 39, 107, 248, 138, 231},
{142, 6, 154, 145, 107, 187, 255, 133, 107, 50, 165, 65, 40, 19, 127, 34, 222, 168, 219, 90, 173, 235, 204, 38, 63, 24, 142, 184, 140},
{252, 142, 160, 220, 83, 169, 218, 238, 156, 156, 106, 146, 255, 135, 200, 80, 249, 160, 207, 14, 204, 124, 164, 22, 54, 98, 159, 207, 191},
{175, 107, 81, 33, 252, 256, 195, 158, 257, 221, 95, 41, 67, 32, 216, 128, 83, 7, 86, 74, 32, 7, 90, 164, 211, 182, 170, 204, 208},
{151, 175, 13, 41, 142, 228, 211, 246, 114, 35, 223, 256, 201, 179, 249, 82, 41, 104, 105, 108, 48, 73, 32, 196, 251, 144, 169, 128, 159},
{245, 81, 151, 29, 154, 208, 213, 144, 106, 255, 82, 124, 36, 102, 25, 174, 200, 211, 249, 147, 87, 34, 57, 194, 181, 192, 179, 225, 204},
{177, 245, 175, 14, 160, 140, 230, 163, 157, 28, 137, 210, 221, 146, 33, 229, 248, 221, 163, 58, 80, 72, 128, 249, 140, 254, 168, 215, 221},
{244, 29, 33, 177, 171, 199, 181, 153, 226, 11, 118, 118, 52, 58, 76, 52, 25, 10, 92, 199, 157, 223, 147, 250, 178, 205, 252, 242, 226},
{229, 244, 41, 245, 132, 148, 207, 216, 29, 166, 172, 187, 177, 161, 117, 125, 28, 111, 11, 143, 250, 207, 142, 192, 236, 243, 164, 201, 198},
{217, 33, 229, 151, 145, 234, 164, 227, 33, 105, 30, 3, 31, 107, 239, 152, 157, 202, 208, 177, 165, 200, 245, 217, 256, 187, 152, 137, 255},
{225, 217, 244, 175, 220, 240, 235, 155, 241, 162, 144, 202, 184, 235, 161, 182, 193, 216, 130, 210, 256, 179, 190, 174, 115, 132, 140, 243, 256} };

int **NOLH_0 = NULL;				// pointer to the design loaded from file

// characteristics of NOLH tables
static struct { int kMin, kMax, n1, n2, loLevel, hiLevel, *table; } NOLH[] =
	   { { 0, 0, 0, 0, 0, 0, NULL }, 
	     { 1, 7, 17, 33, 1, 17, NOLH_1[0] }, 
		 { 8, 11, 33, 65, 1, 33, NOLH_2[0] }, 
		 { 12, 16, 65, 129, 1, 65, NOLH_3[0] }, 
		 { 17, 22, 129, 257, 1, 129, NOLH_4[0] }, 
		 { 23, 29, 257, 257, 1, 257, NOLH_5[0] } };

	   
// function to get the index to the default NOLH design table or -1 otherwise
int NOLH_table( int k )				
{
	for ( int i = 0; i < ( ( sizeof NOLH ) / sizeof NOLH[ 0 ] ); ++i )
		if ( k >= NOLH[ i ].kMin && k <= NOLH[ i ].kMax )
			return i;
	
	return -1;						// number of factors not supported by the preloaded tables
}


// function to remove table 0
void NOLH_clear( void )				
{
	if ( NOLH_0 == NULL )			// table is not allocated?
		return;
	delete [] NOLH_0[0];
	delete [] NOLH_0;
	NOLH_0 = NULL;
	NOLH[ 0 ].kMin = NOLH[ 0 ].kMax = NOLH[ 0 ].n1 = NOLH[ 0 ].n2 = NOLH[ 0 ].loLevel = NOLH[ 0 ].hiLevel = 0;
	NOLH[ 0 ].table = NULL;
}


// function to load a .csv file named NOLH.csv as table 0 (first to be used)
// if option 'force' is used, will be used for any number of factors
bool NOLH_load( char const baseName[] = NOLH_DEF_FILE, bool force = false )				
{
	int i, j, k, n = 1, loLevel = INT_MAX, hiLevel = 1, kFile = 0;
	char *fileName, *lBuffer, *str, *num;
	bool ok = false;
	FILE *NOLHfile;
	
	if ( NOLH_0 != NULL )			// table already loaded?
		NOLH_clear( );
	
	if ( strlen( path ) > 0 )
	{
		fileName = new char[ strlen( path ) + strlen( baseName ) + 2 ];
		sprintf( fileName, "%s/%s", path, baseName );
	}
	else
	{
		fileName = new char[ strlen( baseName ) + 1 ];
		sprintf( fileName, "%s", baseName );
	}
	NOLHfile = fopen( fileName, "r");
	if ( NOLHfile == NULL )
	{
		sprintf( msg, "error opening NOHL design file '%s'", fileName );
		error_hard( msg, "Design of experiment file error", "Check the requested file exists." );
		return false;
	}

	lBuffer = str = new char[ MAX_FILE_SIZE ];

	// get first text line
	fgets( str, MAX_FILE_SIZE, NOLHfile );
	do								// count factors
	{
		num = strtok( str, ",;" );	// get next value
		str = NULL;					// continue strtok from last position
		kFile++;					// factor counter
	}
	while ( num != NULL );
	kFile--;						// adjust for the last non read

	do								// count file lines
	{
		fgets( lBuffer, MAX_FILE_SIZE, NOLHfile );
		n++;
	}
	while ( ! feof( NOLHfile ) );
	
	// get contiguous space for the 2D table
	NOLH_0 = new int*[ n ];
	NOLH_0[ 0 ] = new int[ n * kFile ];
	for ( i = 1; i < n; i++ )
		NOLH_0[ i ] = NOLH_0[ i - 1 ] + kFile;
	
	rewind( NOLHfile );				// restart from the beginning
	for ( i = 0; i < n ; i++ )		// read file content
	{
		// get next text line
		fgets( lBuffer, MAX_FILE_SIZE, NOLHfile );
		str = lBuffer;
		for ( j = 0; j < kFile ; j++ )	// get factor values
		{
			num = strtok( str, "," );	// get next value
			str = NULL;					// continue strtok from last position
			NOLH_0[i][j] = atoi( num );

			if ( num == NULL || NOLH_0[i][j] == 0 )
			{
				delete [] NOLH_0[0];
				delete [] NOLH_0;
				NOLH_0 = NULL;
				sprintf( msg, "invalid format in NOHL file '%s', line=%d", fileName, i + 1 );
				error_hard( msg, "Invalid design of experiment file", "Check the file contents." );
				goto end;
			}

			if ( NOLH_0[i][j] < loLevel )
				loLevel = NOLH_0[i][j];
			if ( NOLH_0[i][j] > hiLevel )
				hiLevel = NOLH_0[i][j];
		}
	}
	
	// set new table characteristics
	if ( force )
		NOLH[ 0 ].kMin = 1;
	else
		NOLH[ 0 ].kMin = NOLH[ sizeof NOLH - 1 ].kMax + 1;
	NOLH[ 0 ].kMax = kFile;
	NOLH[ 0 ].n1 = NOLH[ 0 ].n2 = n;
	NOLH[ 0 ].loLevel = loLevel;
	NOLH[ 0 ].hiLevel = hiLevel;
	NOLH[ 0 ].table = NOLH_0[ 0 ];
	
	sprintf( msg, "\nNOLH file loaded: %s\nk = %d, n = %d, low level = %d, high level = %d", fileName, kFile, n, loLevel, hiLevel );
	plog( msg );
	
	ok = true;
end:
	delete [] fileName;
	delete [] lBuffer;
	return ok;
}

/*
	Support functions for morris_oat() and enhancements
*/

#include <list>
#include <vector>
#include <algorithm>
using namespace std;

// Integer random in [min,max]
#define RND_RANGE( min, max ) ( min + ( rand( ) % ( int )( max - min + 1 ) ) )
// Random choice between two numbers
#define RND_CHOICE( o1, o2 ) ( RND < 0.5 ? o1 : o2 )

// Matrix operations
// allocate dynamic space for matrix
double **mat_new( int m, int n )
{
	double **c = new double *[ m ];
	for ( int i = 0; i < m ; ++i )	 	//rows
		c[ i ] = new double[ n ];
	return c;
}
// deallocate dynamic space for matrix
void mat_del( double **a, int m, int n )
{
	for ( int i = 0; i < m ; ++i )	 	//rows
		delete [] a[ i ];
	delete [] a;
}
// multiply two matrices (c<-a*b)
double **mat_mult_mat( double **a, int m, int n, double **b, int o, int p, double **c )
{
	if ( n != o )
		return NULL;
	for ( int i = 0; i < m ; ++i )	 	//row of first matrix
		for ( int j = 0; j < p; ++j )	//column of second matrix
		{
			c[ i ][ j ] = 0;
			for ( int k = 0; k < n; ++k )
				c[ i ][ j ] += a[ i ][ k ] * b[ k ][ j ];
		}
	return c;
}
// add two same size matrices (c<-a+b)
double **mat_add_mat( double **a, int m, int n, double **b, double **c )
{
	for ( int i = 0; i < m ; ++i )	 	//rows
		for ( int j = 0; j < n; ++j )	//columns
			c[ i ][ j ] = a[ i ][ j ] + b[ i ][ j ];
	return c;
}
// multiply all positions in matrix by a scalar 
double **mat_mult_scal( double **a, int m, int n, double b, double **c )
{
	for ( int i = 0; i < m ; ++i )	 	//rows
		for ( int j = 0; j < n; ++j )	//columns
			c[ i ][ j ] = a[ i ][ j ] * b;
	return c;
}
// add a scalar to all positions in matrix
double **mat_add_scal( double **a, int m, int n, double b, double **c )
{
	for ( int i = 0; i < m ; ++i )	 	//rows
		for ( int j = 0; j < n; ++j )	//columns
			c[ i ][ j ] = a[ i ][ j ] + b;
	return c;
}
// copy a scalar to all positions in matrix
double **mat_copy_scal( double **a, int m, int n, double b )
{
	for ( int i = 0; i < m ; ++i )	 	//rows
		for ( int j = 0; j < n; ++j )	//columns
			a[ i ][ j ] = b;
	return a;
}
// copy same size matrices
double **mat_copy_mat( double **a, int m, int n, double **b )
{
	for ( int i = 0; i < m ; ++i )	 	//rows
		for ( int j = 0; j < n; ++j )	//columns
			a[ i ][ j ] = b[ i ][ j ];
	return a;
}
// insert lines (replacing) in matrix (a<-b)
double **mat_ins_mat( double **a, int m, int n, double **b, int o, int p, int lpos )
{
	if ( lpos + o > m || p > n )
		return NULL;
	for ( int i = 0; i < m ; ++i )	 	//rows
		for ( int j = 0; j < n; ++j )	//columns
			if ( i >= lpos && i < lpos + o && j < p )
				a[ i ][ j ] = b[ i - lpos ][ j ];
	return a;
}
// extract lines (replacing) in matrix (a<-b)
double **mat_ext_mat( double **a, int m, int n, double **b, int o, int p, int lpos )
{
	if ( lpos + m > o || n < p )
		return NULL;
	for ( int i = 0; i < m ; ++i )	 	//rows
		for ( int j = 0; j < n; ++j )	//columns
				a[ i ][ j ] = b[ i + lpos ][ j ];
	return a;
}
/* 	Sum the Euclidean distances of points in two matrices of same size
	Calculates the distance between all points pairs and adds them
	The matrices a and b must have the same size
*/
double mat_sum_dists( double **a, int m, int n, double **b )
{
	double sum = 0;
	for ( int i = 0; i < m ; ++i )	 		//rows in a
		for( int k = 0; k < m; ++k )		//rows in b 
		{
			double dist2 = 0;
			for ( int j = 0; j < n ; ++j )	//columns
				dist2 += pow( a[ i ][ j ] - b[ k ][ j ], 2 );
			sum += sqrt( dist2 );
		}
	return sum;
}

/*
	Calculate a DoE for Elementary Effects (Morris 1991) analysis,
	according to Saltelli et al 2008. Code adapted from SAlib by
	Jon Herman.
	
	Delta is fixed at p/[2(p-1)]
	
	k: number of factors
	r: number of trajectories
	p: number of grid levels
	jump: delta measured in grid levels
	X: preallocated memory area to save the trajectories
*/
double **morris_oat( int k, int r, int p, int jump, double **X )
{
	int i, j, l;
    double delta = ( double ) jump / ( p - 1 );	// grid step delta
	
	// allocate all temporary matrices
	double **B = mat_new( k + 1, k ),
		**DM = mat_new( k, k ),
		**P = mat_new( k, k ),
		**X_base = mat_new( k + 1, k ),
		**delta_diag = mat_new( k, k ),
		**temp_1 = mat_new( k + 1, k ),
		**temp_2 = mat_new( k + 1, k );
	
    // orientation matrix B: lower triangular (1) + upper triangular (-1)
	for ( i = 0; i < k + 1; ++i )
		for ( j = 0; j < k; ++j )
			B[ i ][ j ] = ( i > j ) ? 1 : -1;
    
    // Create r trajectories. Each trajectory contains k+1 parameter sets.
    // (Starts at a base point, and then changes one parameter at a time)
	
	for ( l = 0; l < r; ++l )
	{
        // directions matrix DM - diagonal matrix of either +1 or -1
		for ( i = 0; i < k; ++i )
			for ( j = 0; j < k; ++j )
				DM[ i ][ j ] = ( i == j )? RND_CHOICE( -1, 1 ) : 0;
			
        // permutation matrix P
		int *perm = new int[ k ];
		for ( i = 0; i < k; ++i )
			perm [ i ] = i;
		random_shuffle( & perm[ 0 ], & perm[ k ] );

		P = mat_copy_scal( P, k, k, 0 );
		for ( i = 0; i < k; ++i )
			P[ i ][ perm[ i ] ] = 1;

		delete [] perm;

        // starting point for this trajectory
		for ( j = 0; j < k; ++j )
		{
			double start = ( double ) RND_RANGE( 0, p - delta * ( p - 1 ) - 1 ) / ( p - 1 );
			for ( i = 0; i < k + 1; ++i )
				X_base[ i ][ j ] = start;
		}

        // Indices to be assigned to X, corresponding to this trajectory
		int index_list = l * ( k + 1 );
		for ( i = 0; i < k; ++i )
			for ( j = 0; j < k; ++j )
				delta_diag[ i ][ j ] = ( i == j ) ? delta : 0;

		temp_1 = mat_mult_mat( B, k + 1, k, P, k, k, temp_1 );
		temp_2 = mat_mult_mat( temp_1, k + 1, k, DM, k, k, temp_2 );
		temp_1 = mat_add_scal( temp_2, k + 1, k, 1, temp_1 );
		temp_2 = mat_mult_mat( temp_1, k + 1, k, delta_diag, k, k, temp_2 );
		temp_1 = mat_mult_scal( temp_2, k + 1, k, 0.5, temp_1 );
		temp_2 = mat_add_mat( temp_1, k + 1, k, X_base, temp_2 );
		X = mat_ins_mat( X, r * ( k + 1 ), k, temp_2, k + 1, k, index_list );
	}
	
	// deallocate all temporary matrices
	mat_del( B, k + 1, k );
	mat_del( DM, k, k );
	mat_del( P, k, k );
	mat_del( X_base, k + 1, k );
	mat_del( delta_diag, k, k );
	mat_del( temp_1, k + 1, k );
	mat_del( temp_2, k + 1, k );

    return X;
}

/*
	Optimize a DoE for Elementary Effects (Morris 1991) analysis,
	according to Campolongo et al 2007 and Ruano 2012. Code adapted
	from SAlib by Jon Herman.
	
	k: number of factors
	pool: pool of trajectories produced by morris_oat()
	M: number of trajectories in pool
	r: number of final trajectories (<= M)
	ptr: preallocated memory area to save the trajectories
*/
double **compute_distance_matrix( double **sample, int M, int k, double **DM )
{
	double **input_1 = mat_new( k + 1, k ), 
		   **input_2 = mat_new( k + 1, k );
	
	DM = mat_copy_scal( DM, M, M, 0 );
	for ( int i = 0 ; i < M; ++i )
	{
		input_1 = mat_ext_mat( input_1, k + 1, k, 
							   sample, M * ( k + 1 ), k, 
							   i * ( k + 1 ) );
		for ( int j = i + 1; j < M; ++j )
		{
			input_2 = mat_ext_mat( input_2, k + 1, k, 
								   sample, M * ( k + 1 ), k, 
								   j * ( k + 1 ) );
			DM[ i ][ j ] = DM[ j ][ i ] = 
				mat_sum_dists( input_1, k + 1, k, input_2 );
		}
	}
		
	mat_del( input_1, k + 1, k );
	mat_del( input_2, k + 1, k );
	
    return DM;
}

// Calculate the combinations of indices, r-to-r
vector < vector < int > > combinations( list < int > indices, int r )
{
	vector < int > comb;
	vector < vector < int > > combs;
	// copy list to vector
	vector < int > ind( indices.begin( ), indices.end( ) );
	int n = ind.size( );
	if ( r > n )
		return combs;
	// create selection array with r selectors
	vector < bool > v( n );
	fill( v.begin( ), v.end( ) - n + r, true );
	// create all permutations of the selectors
	do 
	{
		// set member if it is selected in the current permutation of v
		for ( int i = 0; i < n; ++i )
			if ( v[ i ] ) 
				comb.push_back( ind[ i ] );
		combs.push_back( comb );
		comb.clear( );
	}
	while ( prev_permutation( v.begin( ), v.end( ) ) );
	
	return combs;
}

/*
  Calculate combinatorial distance between a select group of trajectories, 
  indicated by indices
    
    indices: list of candidate pairs of points = list < int >
    DM: distance matrix = array (M,M)
*/    
double sum_distances( list < int > indices, double **DM )
{
	// get all combination pairs of indices
	vector < vector < int > > combs = combinations( indices, 2 );

    // add distance of all points pairs
	double D = 0;
	for ( int j = 0; j < combs.size( ); ++j )
		D += DM[ combs[ j ][ 0 ] ][ combs[ j ][ 1 ] ];
	
	return D;
}

// get the top-i size items index from a unidimensional array
list < int > top_idx( double *a, int n, int i )
{
	list < int > top;
	vector < bool > used( n, false );
	for ( int k = 0; k < i; ++k )
	{
		int max_idx = -1;
		double max = -INFINITY;
		for ( int j = 0; j < n; ++j )
			if ( ! used[ j ] && a[ j ] > max )
			{
				max_idx = j;
				max = a[ j ];
			}
		used[ max_idx ] = true;
		top.push_back( max_idx );
	}
	return top;
}

/*
	Get the indice that belong to the maximum distance in an array of distances
    
    indices_list = list of points
    distance = array (M)
*/
list < int > get_max_sum_ind( vector < list < int > > indices_list, vector < double > row_maxima_i )
{
	int max_idx = -1;
	double max = -INFINITY;
	for ( int j = 0; j < indices_list.size( ); ++j )
		if ( row_maxima_i[ j ] > max )
		{
			max_idx = j;
			max = row_maxima_i[ j ];
		}
	return indices_list[ max_idx ];
}

/*
	Adds extra indices for the combinatorial problem. 
	For indices = (1,2) and M=5, the method returns [(1,2,3),(1,2,4),(1,2,5)]
*/
vector < list < int > > add_indices( list < int > m_max_ind, int M )
{
	vector < list < int > > list_new_indices;
	list < int > copy = m_max_ind;
	for ( int i = 0; i < M; ++i )
		if ( find( m_max_ind.begin( ), m_max_ind.end( ), i ) == m_max_ind.end( ) )
		{
			copy.push_back( i );
			list_new_indices.push_back( copy );
			copy.pop_back( );
		}
	return list_new_indices;
}

/*
	An alternative by Ruano et al. (2012) for the brute force approach as 
	originally proposed by Campolongo et al. (2007). The method should improve 
	the speed with which an optimal set of trajectories is found tremendously 
	for larger sample sizes.
*/
double **opt_trajectories( int k, double **pool, int M, int r, double **X )
{
	if ( r >= M )					// nothing to do?
	{
		X = mat_copy_mat( X, r * ( k + 1 ), k, pool );
		return X;
	}

 	list < int > indices, i_max_ind, m_max_ind, tot_max; 
	vector < list < int > > tot_indices_list, indices_list, m_ind;
	
	double **DM = mat_new( M, M );
	DM = compute_distance_matrix( pool, M, k, DM );
    
	vector < double > tot_max_array( r - 1, 0 );
	
	//#############Loop 'i'#############
	// i starts at 1
	for ( int i = 1; i < r; ++i )
	{
		indices_list.clear( );
		vector < double > row_maxima_i( M, 0 );
		
		for ( int row = 0; row < M; ++row )
		{
			indices = top_idx( DM[ row ], M, i );
			indices.push_back( row );
			row_maxima_i[ row ] = sum_distances( indices, DM );
			indices_list.push_back( indices );
		}
		
		// Find the indices belonging to the maximum distance
		i_max_ind = get_max_sum_ind( indices_list, row_maxima_i );

		//#########Loop 'm' (called loop 'k' in Ruano)############
		m_max_ind = i_max_ind;
		// m starts at 1
        for ( int m = 1; m <= r - i - 1; ++m )
		{
			m_ind = add_indices( m_max_ind, M );
            vector < double > m_maxima( m_ind.size( ), 0 );
			
            for ( int n = 0; n < m_ind.size( ); ++n )
                m_maxima[ n ] = sum_distances( m_ind[ n ], DM );
            
            m_max_ind = get_max_sum_ind( m_ind, m_maxima );
		}
		tot_indices_list.push_back( m_max_ind );
		tot_max_array[ i - 1 ] = sum_distances( m_max_ind, DM );
	}

	tot_max = get_max_sum_ind( tot_indices_list, tot_max_array );
	tot_max.sort( );
	vector < int > max( tot_max.begin( ), tot_max.end( ) );
	
	// index the submatrix for each trajectory
	vector < int > index_list( M, 0 );
	for ( int i = 0; i < M; ++i )
		index_list[ i ] = i * ( k + 1 );
	
	// move the best trajectories to caller 2D array
	double **temp = mat_new( k + 1, k );
	for ( int i = 0; i < r; ++i )
	{
		temp = mat_ext_mat( temp, k + 1, k, pool, M * ( k + 1 ), k, index_list[ max[ i ] ] );
		X = mat_ins_mat( X, r * ( k + 1 ), k, temp, k + 1, k, index_list[ i ] );
	}
	
	mat_del( temp, k + 1, k );
	mat_del( DM, M, M );
	
    return X;
}
	
// destructor function to the design object
design::~design( void )
{
	for( int i = 0; i < n; i++ )		// run through all experiments
		delete [] ptr[ i ];				// free memory
	for( int i = 0; i < k; i++ )		// and all variables
		delete [] lab[ i ];
	delete [] ptr;
	delete [] lab; 
	delete [] hi; 
	delete [] lo; 
	delete [] par; 
	delete [] lag;
}

// Constructor function to the design object
// type = 1: NOLH
// type = 2: random sampling
// type = 3: Elementary Effects sampling (Morris, 1991)
// samples = -1: use extended predefined sample size (n2)
// factors = 0: use automatic DoE size

design::design( sense *rsens, int typ, char const *fname, int findex, 
				int samples, int factors, int jump, int trajs )
{
	int i , j, kTab, doeRange, poolSz;
	double **pool;
	char *doefname, doeName[MAX_ELEM_LENGTH];
	FILE *f;
	sense *cs;
	
	if ( rsens == NULL )					// valid pointer?
		typ = 0;							// trigger invalid design
	
	switch ( typ )
	{
		case 1:								// Near Orthogonal Latin Hypercube sampling
			k = kTab = num_sensitivity_variables( rsens );	// number of factors
			
			if ( strcmp( fname, "" ) )		// if filename was specified
				NOLH_load( fname, true );	// load file and force using it always
			else
			{
				if ( factors != 0 && k > factors )	// invalid # of factors selected?
				{
					sprintf( msg, "number of NOLH variables selected is too small" );
					error_hard( msg, "Invalid design of experiment parameters", "Check the design." );
					goto invalid;
				}
				// if user selected # of factors, use it to select internal table
				kTab = ( factors == 0 ) ? k : factors;
			}
				
			tab = NOLH_table( kTab );		// design table to use
			if ( tab == -1 )				// number of factors too large, try to load external table (file)
			{
				if ( NOLH_load( ) )			// tentative table load from disk ok?
				{
					tab = NOLH_table( k );	// design table to use
					if ( tab == -1 )		// still too large?
					{
						sprintf( msg, "too many variables to test for NOLH.csv size" );
						error_hard( msg, "Invalid design of experiment parameters", "Check the design." );
						goto invalid;		// abort
					}
				}
				else
				{
					sprintf( msg, "too many variables to test" );
					error_hard( msg, "Invalid design of experiment parameters", "Check the design." );
					goto invalid;			// abort
				}
			}
				
			// get the number of samples required by the NOLH design, according to user choice (basic/extended)
			n = ( samples != -1 ) ? NOLH[ tab ].n1 : NOLH[ tab ].n2;

			sprintf( msg, "\nNOLH table used: %d (%s), n = %d", tab, tab > 0 ? "built-in" : "from file", n );
			plog( msg );
			
			// allocate memory for data
			par = new int[ k ];				// vector of variable type (parameter / lagged value)
			lag = new int[ k ];				// vector of lags
			hi = new double[ k ];			// vector of high factor value
			lo = new double[ k ];			// vector of low factor value
			lab = new char *[ k ];			// vector of variable labels
			ptr = new double *[ n ];		// allocate space for weighted design table
			
			// define low and high values from sensitivity data
			for ( i = 0, cs = rsens; cs != NULL; cs = cs->next )
			{
				if ( cs->nvalues < 2 )		// consider only multivalue variables
					continue;
				
				hi[ i ] = lo[ i ] = cs->v[ 0 ];
				for ( j = 1; j < cs->nvalues; j++ )
				{
					hi[ i ] = fmax( cs->v[ j ], hi[ i ] );
					lo[ i ] = fmin( cs->v[ j ], lo[ i ] );
				}
				
				par[ i ] = cs->param;		// set variable type
				lag[ i ] = cs->lag;			// set number of lags
				
				// copy label (name)
				lab[ i ] = new char[ strlen( cs->label ) + 1 ];
				strcpy( lab[i], cs->label );
				
				i++;
			}
			
			// calculate the design of the experiment
			doeRange = NOLH[ tab ].hiLevel - NOLH[ tab ].loLevel;
			for ( i = 0; i < n; i++ )		// for all experiments
			{
				ptr[ i ] = new double[ k ];	// allocate 2nd level data
				for ( j = 0; j < k; j++ )	// for all factors
					ptr[ i ][ j ] = lo[ j ] + 
					( *( NOLH[ tab ].table + i * NOLH[ tab ].kMax + j ) - 1 ) * 
					( hi[ j ] - lo[ j ] ) / doeRange;
			}
			
			break;	
			
		case 2:								// random sampling
			k = num_sensitivity_variables( rsens );	// number of factors
			n = samples;					// number of samples required
			if ( n < 1 )					// at least one sample required
				goto invalid;
			
			// allocate memory for data
			par = new int[ k ];				// vector of variable type (parameter / lagged value)
			lag = new int[ k ];				// vector of lags
			hi = new double[ k ];			// vector of high factor value
			lo = new double[ k ];			// vector of low factor value
			lab = new char *[ k ];			// vector of variable labels
			ptr = new double *[ n ];		// allocate space for weighted design table
			
			// define low and high values from sensitivity data
			for ( i = 0, cs = rsens; cs != NULL; cs = cs->next )
			{
				if ( cs->nvalues < 2 )		// consider only multivalue variables
					continue;
				
				hi[ i ] = lo[ i ] = cs->v[ 0 ];
				for ( j = 1; j < cs->nvalues; j++ )
				{
					hi[ i ] = fmax( cs->v[ j ], hi[ i ] );
					lo[ i ] = fmin( cs->v[ j ], lo[ i ] );
				}
				
				par[ i ] = cs->param;		// set variable type
				lag[ i ] = cs->lag;			// set number of lags
				
				// copy label (name)
				lab[ i ] = new char[ strlen( cs->label ) + 1 ];
				strcpy( lab[i], cs->label );
				
				i++;
			}
			
			// calculate the design of the experiment
			for ( i = 0; i < n; i++ )		// for all experiments
			{
				ptr[ i ] = new double[ k ];	// allocate 2nd level data
				for ( j = 0; j < k; j++ )	// for all factors
					ptr[ i ][ j ] = lo[ j ] + RND * ( hi[ j ] - lo[ j ] );
			}
			
			break;	
			
		case 3:								// Elementary Effects sampling
			k = num_sensitivity_variables( rsens );	// number of factors
			poolSz = samples * ( k + 1 );	// larger pool to extract samples
			n = trajs * ( k + 1 );			// number of effective samples
			if ( n < 1 || n > poolSz )		// at least one sample required
				goto invalid;
			
			// allocate memory for data
			par = new int[ k ];				// vector of variable type (parameter / lagged value)
			lag = new int[ k ];				// vector of lags
			hi = new double[ k ];			// vector of high factor value
			lo = new double[ k ];			// vector of low factor value
			lab = new char *[ k ];			// vector of variable labels
			ptr = new double *[ n ];		// allocate space for weighted design table
			for ( i = 0; i < n; i++ )		// for all final trajectories
				ptr[ i ] = new double[ k ];	// allocate 2nd level data
			pool = new double *[ poolSz ];	// allocate space for weighted design table
			for ( i = 0; i < poolSz; i++ )	// for all pool trajectories
				pool[ i ] = new double[ k ];
			
			// define low and high values from sensitivity data
			for ( i = 0, cs = rsens; cs != NULL; cs = cs->next )
			{
				if ( cs->nvalues < 2 )		// consider only multivalue variables
					continue;
				
				hi[ i ] = lo[ i ] = cs->v[ 0 ];
				for ( j = 1; j < cs->nvalues; j++ )
				{
					hi[ i ] = fmax( cs->v[ j ], hi[ i ] );
					lo[ i ] = fmin( cs->v[ j ], lo[ i ] );
				}
				
				par[ i ] = cs->param;		// set variable type
				lag[ i ] = cs->lag;			// set number of lags
				
				// copy label (name)
				lab[ i ] = new char[ strlen( cs->label ) + 1 ];
				strcpy( lab[i], cs->label );
				
				i++;
			}
			
			// calculate the Morris OAT pool of trajectories
			pool = morris_oat( k, samples, factors, jump, pool );
			
			// select the best trajectories from pool
			ptr = opt_trajectories( k, pool, samples, trajs, ptr );
			
			for ( i = 0; i < poolSz; i++ )	// free pool memory
				delete [] pool[ i ];
			delete [] pool;
			
			// scale the DoE to the sensitivity test ranges
			for ( i = 0; i < n; i++ )		// for all experiments
				for ( j = 0; j < k; j++ )	// for all factors
					ptr[ i ][ j ] = lo[ j ] + ptr[ i ][ j ] * ( hi[ j ] - lo[ j ] );
			
			break;	
			
		default:							// invalid design!
		invalid:
			typ = tab = n = k = 0;
			par = lag = NULL;
			hi = lo = NULL;
			ptr = NULL;
			lab = NULL;
			return;
	}
	
	// generate a configuration file for the experiment
			
	// file name for saving table
	sprintf( doeName, "%u_%u", ( unsigned ) findex, ( unsigned ) ( findex + n - 1 ) );
	
	if( strlen( path ) > 0 )				// non-default folder?
	{
		doefname = new char[ strlen( path ) + strlen( simul_name ) + strlen( doeName ) + 10 ];
		sprintf( doefname, "%s/%s_%s.csv", path, simul_name, doeName );
	}
	else
	{
		doefname = new char[ strlen( simul_name ) + strlen( doeName ) + 10 ];
		sprintf( doefname, "%s_%s.csv", simul_name, doeName );
	}
	f = fopen( doefname, "w" );
	
	// write the doe table to disk
	for ( j = 0; j < k; j++ )		// write variable labels
		fprintf( f, "%s%c", lab[ j ], ( j == ( k - 1 ) ? '\n' : ',' ) );
		
	for ( i = 0; i < n; i++ )		// for all experiments
		for ( j = 0; j < k; j++ )	// write variable experimental values
			fprintf( f, "%lf%c", ptr[ i ][ j ], ( j == ( k - 1 ) ? '\n' : ',' ) );
			
	fclose( f );
	
	char out[ MAX_PATH_LENGTH ];
	sprintf( out, "\nDoE configuration saved: %s", doefname );
	plog( out );
	
	delete [ ] doefname;
}


// procedure to generate the configuration files for the Design of Experiment (DOE)
void sensitivity_doe( int *findex, design *doe )
{
	int i, j;
	object *cur;
	variable *cvar;
	description *cdescr; 
	char *fname;
	FILE *f;
	
	for ( i = 0; i < doe->n; i++ )				// run through all experiments
	{
		// set up the variables (factors) with the experiment values
		for ( j = 0; j < doe->k; j++ )			// run through all factors
		{
			cvar = root->search_var( root, doe->lab[ j ] );	// find variable to set
			for ( cur = cvar->up; cur!=NULL; cur = cur->hyper_next( cur->label ) )
			{									// run through all objects containing var
				cvar = cur->search_var( cur, doe->lab[ j ] ); 
				if ( doe->par[ j ] == 1 )		// handle lags > 0
					cvar->val[ 0 ] = doe->ptr[ i ][ j ];
				else
					cvar->val[ doe->lag[ j ] ] = doe->ptr[ i ][ j ];
			}
		}
		
		// generate a configuration file for the experiment
		if ( ! save_configuration( root, *findex ) )
		{
			cmd( inter , "tk_messageBox -parent . -type ok -icon error -title Error -message \"Configuration files cannot be saved\" -detail \"Check if the drive or the current directory is set READ-ONLY, select a drive/directory with write permission and try again.\"" );
			return;
		}
		
		*findex = *findex + 1;
	}
}
