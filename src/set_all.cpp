/*************************************************************

	LSD 8.0 - March 2021
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
	See Readme.txt for copyright information of
	third parties' code used in LSD
	
 *************************************************************/

/*************************************************************
SETALL.CPP
It contains the routine called from the edit_dat file for setting all the
values of a variable with a function, instead of inserting manually.

The functions contained in this file are:

- void set_all( int *choice, object *r, char *lab, int lag )
it allows 5 options to set all values. It uses one value entered by the user
in this window and, for some option, the first value for this variable in the
model. That is, the value for this variable contained in the first object of this
type.
The options are the following:
1) set all values equal to the entered value
2 ) the first value is not changed and all the others are computed as the previous
plus the entered object.
3) as before, but instead of producing a ever increasing series, it re-initialize
any new group.
4) random numbers, drawn by a uniform value whose min is the first value
and max is the inserted value
5) random numbers, drawn by a normal whose mean is the first value and
standard deviation is the inserted value.
*************************************************************/

#include "decl.h"
#include "nolh.h"


/****************************************************
SET_ALL
****************************************************/

void set_all( int *choice, object *original, char *lab, int lag )
{
	bool selFocus = true;
	char *l, ch[ MAX_ELEM_LENGTH ], action[ MAX_ELEM_LENGTH ];
	double value, value1, value2, step, counter;
	int res, i, j, kappa, to_all, update_d, cases_from, cases_to, fill, use_seed, rnd_seed, step_in;
	description *cd; 
	object *cur, *r;
	variable *cv;
	FILE *f;

	r = root->search( original->label );		// select the first instance
	cv = r->search_var( NULL, lab );
	if ( cv == NULL )
		return;

	if ( cv->param == 1 )
		lag = 0;
	
	Tcl_LinkVar( inter, "res", ( char * ) &res, TCL_LINK_INT );
	Tcl_LinkVar( inter, "value1", ( char * ) &value1, TCL_LINK_DOUBLE );
	Tcl_LinkVar( inter, "value2", ( char * ) &value2, TCL_LINK_DOUBLE );

	// default values
	res = 1;
	value1 = cv->val [ lag ];					// preload the existing value of the first object
	value2 = 0;
	cmd( "set value 1" ); 						// method
	cmd( "set fill 0" );
	cmd( "set to_all 1" );
	cmd( "set step_in 1" );
	cmd( "set cases_from 1" );
	cmd( "set cases_to 1000" );
	cmd( "set rnd_seed 1" );
	cmd( "set use_seed 0" );
	cmd( "set update_d 1" );

	cmd( "newtop .sa \"Set All Objects Initialization\" { set choice 2 }" );

	cmd( "ttk::frame .sa.head" );					// heading
	cmd( "ttk::label .sa.head.lg -text \"Set initial values for\"" );

	cmd( "ttk::frame .sa.head.l" );
	if ( cv->param != 0 )
	{
		if ( cv->param == 2 )
			cmd( "ttk::label .sa.head.l.c -text \"Function: \"" );
		else
			cmd( "ttk::label .sa.head.l.c -text \"Parameter: \"" );
		
		cmd( "ttk::label .sa.head.l.n -text \"%s\" -style hl.TLabel", lab  );
		cmd( "pack .sa.head.l.c .sa.head.l.n -side left" );
	}
	else
	{
		cmd( "ttk::label .sa.head.l.c -text \"Variable: \"" );
		cmd( "ttk::label .sa.head.l.n1 -text \"%s  \" -style hl.TLabel", lab );
		cmd( "ttk::label .sa.head.l.n2 -text \"\\[  lag \"" );
		cmd( "ttk::label .sa.head.l.n3 -text \"%d\" -style hl.TLabel", t - cv->last_update + lag + 1  );
		cmd( "ttk::label .sa.head.l.n4 -text \"\\]\"" );
		cmd( "pack .sa.head.l.c .sa.head.l.n1 .sa.head.l.n2 .sa.head.l.n3 .sa.head.l.n4 -side left" );
	}

	cmd( "ttk::frame .sa.head.lo" );
	cmd( "ttk::label .sa.head.lo.l -text \"Contained in object: \"" );
	cmd( "ttk::label .sa.head.lo.o -text \"%s\" -style hl.TLabel", cv->up->label  );
	cmd( "pack .sa.head.lo.l .sa.head.lo.o -side left" );

	cmd( "pack .sa.head.lg .sa.head.l .sa.head.lo" );

	cmd( "ttk::frame .sa.m" );			

	cmd( "ttk::frame .sa.m.f1" );					// left column

	cmd( "ttk::frame .sa.m.f1.val" );
	cmd( "ttk::label .sa.m.f1.val.l -text \"Initialization data\"" );

	cmd( "ttk::frame .sa.m.f1.val.i" );

	cmd( "ttk::frame .sa.m.f1.val.i.l1" );
	cmd( "ttk::label .sa.m.f1.val.i.l1.l1 -text \"Equal to\"" );
	cmd( "ttk::entry .sa.m.f1.val.i.l1.e1 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set value1 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value1; set err .sa.m.f1.val.i.l1.e1; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "pack .sa.m.f1.val.i.l1.l1 .sa.m.f1.val.i.l1.e1" );

	cmd( "ttk::frame .sa.m.f1.val.i.l2" );
	cmd( "ttk::label .sa.m.f1.val.i.l2.l2 -text \"(none)\"" );
	cmd( "ttk::entry .sa.m.f1.val.i.l2.e2 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set value2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value2; set err .sa.m.f1.val.i.l2.e2; set choice 1; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
	cmd( "pack .sa.m.f1.val.i.l2.l2 .sa.m.f1.val.i.l2.e2" );

	cmd( "pack .sa.m.f1.val.i.l1 .sa.m.f1.val.i.l2 -expand yes -fill x  -ipadx 5 -ipady 2" );

	cmd( "pack .sa.m.f1.val.l .sa.m.f1.val.i" );

	cmd( "ttk::frame .sa.m.f1.rd" );
	cmd( "ttk::label .sa.m.f1.rd.l -text \"Initialization method\"" );

	cmd( "ttk::frame .sa.m.f1.rd.i -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton .sa.m.f1.rd.i.r1 -text \"Equal to\" -variable res -value 1 -command { .sa.m.f1.val.i.l1.l1 conf -text \"Value\"; .sa.m.f1.val.i.l1.e1 conf -state normal; .sa.m.f1.val.i.l2.l2 conf -text \"(none)\"; .sa.m.f1.val.i.l2.e2 conf -state disabled; .sa.m.f2.s.i.l.a.e conf -state normal; .sa.m.f2.s.i.l.f conf -state normal; set use_seed 0; .sa.m.f2.rnd.i.le.f conf -state disabled; .sa.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind .sa.m.f1.rd.i.r1 <Down> {focus .sa.m.f1.rd.i.r9; .sa.m.f1.rd.i.r9 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r1 <Return> { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1}" );

	cmd( "ttk::radiobutton .sa.m.f1.rd.i.r9 -text \"Range\" -variable res -value 9 -command { .sa.m.f1.val.i.l1.l1 conf -text \"Minimum\"; .sa.m.f1.val.i.l1.e1 conf -state normal; .sa.m.f1.val.i.l2.l2 conf -text \"Maximum\"; .sa.m.f1.val.i.l2.e2 conf -state normal; .sa.m.f2.s.i.l.a.e conf -state normal; .sa.m.f2.s.i.l.f conf -state normal; set use_seed 0; .sa.m.f2.rnd.i.le.f conf -state disabled; .sa.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind .sa.m.f1.rd.i.r9 <Down> {focus .sa.m.f1.rd.i.r2; .sa.m.f1.rd.i.r2 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r9 <Up> {focus .sa.m.f1.rd.i.r1; .sa.m.f1.rd.i.r1 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r9 <Return> { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1}" );

	cmd( "ttk::radiobutton .sa.m.f1.rd.i.r2 -text \"Increasing\" -variable res -value 2 -command { .sa.m.f1.val.i.l1.l1 conf -text \"Start\"; .sa.m.f1.val.i.l1.e1 conf -state normal; .sa.m.f1.val.i.l2.l2 conf -text \"Step\"; .sa.m.f1.val.i.l2.e2 conf -state normal; .sa.m.f2.s.i.l.a.e conf -state normal; .sa.m.f2.s.i.l.f conf -state normal; set use_seed 0; .sa.m.f2.rnd.i.le.f conf -state disabled; .sa.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind .sa.m.f1.rd.i.r2 <Down> {focus .sa.m.f1.rd.i.r4; .sa.m.f1.rd.i.r4 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r2 <Up> {focus .sa.m.f1.rd.i.r9; .sa.m.f1.rd.i.r9 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r2 <Return> { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1}" );

	cmd( "ttk::radiobutton .sa.m.f1.rd.i.r4 -text \"Increasing (groups)\" -variable res -value 4 -command {.sa.m.f1.val.i.l1.l1 conf -text \"Start\"; .sa.m.f1.val.i.l1.e1 conf -state normal; .sa.m.f1.val.i.l2.l2 conf -text \"Step\"; .sa.m.f1.val.i.l2.e2 conf -state normal; set step_in 1; .sa.m.f2.s.i.l.a.e conf -state disabled; .sa.m.f2.s.i.l.f conf -state disabled; set use_seed 0; .sa.m.f2.rnd.i.le.f conf -state disabled; .sa.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind .sa.m.f1.rd.i.r4 <Up> {focus .sa.m.f1.rd.i.r2; .sa.m.f1.rd.i.r2 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r4 <Down> {focus .sa.m.f1.rd.i.r3; .sa.m.f1.rd.i.r3 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r4 <Return> { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1}" );

	cmd( "ttk::radiobutton .sa.m.f1.rd.i.r3 -text \"Random (uniform)\" -variable res -value 3 -command { .sa.m.f1.val.i.l1.l1 conf -text \"Minimum\"; .sa.m.f1.val.i.l1.e1 conf -state normal; .sa.m.f1.val.i.l2.l2 conf -text \"Maximum\"; .sa.m.f1.val.i.l2.e2 conf -state normal; .sa.m.f2.s.i.l.a.e conf -state normal; .sa.m.f2.s.i.l.f conf -state normal; .sa.m.f2.rnd.i.le.f conf -state normal }" );
	cmd( "bind .sa.m.f1.rd.i.r3 <Up> {focus .sa.m.f1.rd.i.r4; .sa.m.f1.rd.i.r4 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r3 <Down> {focus .sa.m.f1.rd.i.r8; .sa.m.f1.rd.i.r8 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r3 <Return> { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1}" );

	cmd( "ttk::radiobutton .sa.m.f1.rd.i.r8 -text \"Random integer (uniform)\" -variable res -value 8 -command { .sa.m.f1.val.i.l1.l1 conf -text \"Minimum\"; .sa.m.f1.val.i.l1.e1 conf -state normal; .sa.m.f1.val.i.l2.l2 conf -text \"Maximum\"; .sa.m.f1.val.i.l2.e2 conf -state normal; .sa.m.f2.s.i.l.a.e conf -state normal; .sa.m.f2.s.i.l.f conf -state normal; .sa.m.f2.rnd.i.le.f conf -state normal }" );
	cmd( "bind .sa.m.f1.rd.i.r8 <Up> {focus .sa.m.f1.rd.i.r3; .sa.m.f1.rd.i.r3 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r8 <Down> {focus .sa.m.f1.rd.i.r5; .sa.m.f1.rd.i.r5 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r8 <Return> { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1}" );

	cmd( "ttk::radiobutton .sa.m.f1.rd.i.r5 -text \"Random (normal)\" -variable res -value 5 -command {.sa.m.f1.val.i.l1.l1 conf -text \"Mean\"; .sa.m.f1.val.i.l1.e1 conf -state normal; .sa.m.f1.val.i.l2.l2 conf -text \"Std. deviation\"; .sa.m.f1.val.i.l2.e2 conf -state normal; .sa.m.f2.s.i.l.a.e conf -state normal; .sa.m.f2.s.i.l.f conf -state normal; .sa.m.f2.rnd.i.le.f conf -state normal }" );
	cmd( "bind .sa.m.f1.rd.i.r5 <Up> {focus .sa.m.f1.rd.i.r8; .sa.m.f1.rd.i.r8 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r5 <Down> {focus .sa.m.f1.rd.i.r7; .sa.m.f1.rd.i.r7 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r5 <Return> { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1}" );

	cmd( "ttk::radiobutton .sa.m.f1.rd.i.r7 -text \"Import from data file\" -variable res -value 7 -command { .sa.m.f1.val.i.l1.l1 conf -text \"(none)\"; .sa.m.f1.val.i.l1.e1 conf -state disabled; .sa.m.f1.val.i.l2.l2 conf -text \"(none)\"; .sa.m.f1.val.i.l2.e2 conf -state disabled; set step_in 1; .sa.m.f2.s.i.l.a.e conf -state disabled; .sa.m.f2.s.i.l.f conf -state disabled; set use_seed 0; .sa.m.f2.rnd.i.le.f conf -state disabled; .sa.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind .sa.m.f1.rd.i.r7 <Up> {focus .sa.m.f1.rd.i.r5; .sa.m.f1.rd.i.r5 invoke}" );
	cmd( "bind .sa.m.f1.rd.i.r7 <Return> {.sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1}" );

	cmd( "pack .sa.m.f1.rd.i.r1 .sa.m.f1.rd.i.r9 .sa.m.f1.rd.i.r2 .sa.m.f1.rd.i.r4 .sa.m.f1.rd.i.r3 .sa.m.f1.rd.i.r8 .sa.m.f1.rd.i.r5 .sa.m.f1.rd.i.r7 -anchor w" );
	
	cmd( "tooltip::tooltip .sa.m.f1.rd.i.r1 \"Every instance set to the same Value\"" );
	cmd( "tooltip::tooltip .sa.m.f1.rd.i.r9 \"Linear range from Minimum to Maximum\"" );
	cmd( "tooltip::tooltip .sa.m.f1.rd.i.r2 \"From Start plus Increasing for each instance\"" );
	cmd( "tooltip::tooltip .sa.m.f1.rd.i.r4 \"From Start in each group plus Increasing for each instance\"" );
	cmd( "tooltip::tooltip .sa.m.f1.rd.i.r3 \"Uniform random real draw from Minimum to Maximum\"" );
	cmd( "tooltip::tooltip .sa.m.f1.rd.i.r8 \"Uniform random integer draw from Minimum to Maximum\"" );
	cmd( "tooltip::tooltip .sa.m.f1.rd.i.r5 \"Random draw from normal distribution with Mean and Standard deviation\"" );
	cmd( "tooltip::tooltip .sa.m.f1.rd.i.r7 \"Read initialization data from disk file\"" );

	cmd( "pack .sa.m.f1.rd.l .sa.m.f1.rd.i" );

	cmd( "pack .sa.m.f1.val .sa.m.f1.rd -expand yes -fill x -padx 5 -pady 5" );

	cmd( "ttk::frame .sa.m.f2" );					// right column

	cmd( "ttk::frame .sa.m.f2.s" );
	cmd( "ttk::label .sa.m.f2.s.tit -text \"Object instance selection\"" );

	cmd( "ttk::frame .sa.m.f2.s.i" );

	cmd( "ttk::frame .sa.m.f2.s.i.l" );

	cmd( "ttk::frame .sa.m.f2.s.i.l.a" );
	cmd( "ttk::label .sa.m.f2.s.i.l.a.l -text \"Apply every\"" );
	cmd( "ttk::spinbox .sa.m.f2.s.i.l.a.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= 9999 } { set step_in %%P; return 1 } { %%W delete 0 end; %%W insert 0 $step_in; set err .sa.m.f2.s.i.l.a.e; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "ttk::label .sa.m.f2.s.i.l.a.l1 -text \"instance( s)\"" );
	cmd( "pack .sa.m.f2.s.i.l.a.l .sa.m.f2.s.i.l.a.e .sa.m.f2.s.i.l.a.l1 -side left -padx 1" );

	cmd( "ttk::checkbutton .sa.m.f2.s.i.l.f -text \"Fill-in\" -variable fill" );
	cmd( "pack  .sa.m.f2.s.i.l.a .sa.m.f2.s.i.l.f -padx 5 -side left" );
	cmd( "pack  .sa.m.f2.s.i.l -pady 2" );
	
	cmd( "tooltip::tooltip .sa.m.f2.s.i.l.a \"Number of instances to skip from initializing\"" );
	cmd( "tooltip::tooltip .sa.m.f2.s.i.l.f \"Fill intermediate instances with same value\"" );

	cmd( "ttk::frame .sa.m.f2.s.i.sel -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton .sa.m.f2.s.i.sel.all -text \"Apply to all instances\" -variable to_all -value 1 -command { .sa.m.f2.s.i.sel2.c.to conf -state disabled; .sa.m.f2.s.i.sel2.c.from conf -state disabled; bind .sa.m.f2.s.i.sel2.c.from <Button-3> { }; bind .sa.m.f2.s.i.sel2.c.to <Button-3> { }; bind .sa.m.f2.s.i.sel2.c.from <Button-2> { }; bind .sa.m.f2.s.i.sel2.c.to <Button-2> { } }" );
	cmd( "ttk::radiobutton .sa.m.f2.s.i.sel.sel -text \"Apply to a range of instances\" -variable to_all -value 0 -command { .sa.m.f2.s.i.sel2.c.to conf -state normal; .sa.m.f2.s.i.sel2.c.from conf -state normal; bind .sa.m.f2.s.i.sel2.c.from <Button-3> { set choice 9 }; bind .sa.m.f2.s.i.sel2.c.to <Button-3> { set choice 10 }; bind .sa.m.f2.s.i.sel2.c.from <Button-2> { set choice 9 }; bind .sa.m.f2.s.i.sel2.c.to <Button-2> { set choice 10 } }" );
	cmd( "pack .sa.m.f2.s.i.sel.all .sa.m.f2.s.i.sel.sel -anchor w" );
	
	cmd( "tooltip::tooltip .sa.m.f2.s.i.sel.all \"Apply initialization to all instances\"" );
	cmd( "tooltip::tooltip .sa.m.f2.s.i.sel.sel \"Apply initialization to a range of instances\"" );

	cmd( "pack .sa.m.f2.s.i.sel -pady 2" );

	cmd( "ttk::frame .sa.m.f2.s.i.sel2" );

	cmd( "ttk::frame .sa.m.f2.s.i.sel2.c" );
	cmd( "ttk::label .sa.m.f2.s.i.sel2.c.lfrom -text \"From\"" );
	cmd( "ttk::spinbox .sa.m.f2.s.i.sel2.c.from -width 5 -from 1 -to 9999 -state disabled -state disabled -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= 9999 } { set cases_from %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cases_from; set err .sa.m.f2.s.i.sel2.c.from; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "ttk::label .sa.m.f2.s.i.sel2.c.lto -text \"to\"" );
	cmd( "ttk::spinbox .sa.m.f2.s.i.sel2.c.to -width 5 -from 1 -to 9999 -state disabled -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= 9999 } { set cases_to %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cases_to; set err .sa.m.f2.s.i.sel2.c.to; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "pack .sa.m.f2.s.i.sel2.c.lfrom .sa.m.f2.s.i.sel2.c.from .sa.m.f2.s.i.sel2.c.lto .sa.m.f2.s.i.sel2.c.to -side left -pady 1" );

	cmd( "ttk::label .sa.m.f2.s.i.sel2.obs -text \"(use right button on cells for options)\"" );
	cmd( "pack .sa.m.f2.s.i.sel2.c .sa.m.f2.s.i.sel2.obs" );
	cmd( "pack .sa.m.f2.s.i.sel2 -pady 2" );
	
	cmd( "tooltip::tooltip .sa.m.f2.s.i.sel2 \"Select first and last instance to initialize\"" );

	cmd( "pack .sa.m.f2.s.tit .sa.m.f2.s.i" );

	cmd( "pack .sa.m.f2.s" );

	cmd( "ttk::frame .sa.m.f2.rnd" );
	cmd( "ttk::label .sa.m.f2.rnd.l -text \"Random number generator\"" );

	cmd( "ttk::frame .sa.m.f2.rnd.i" );

	cmd( "ttk::frame .sa.m.f2.rnd.i.le" );
	cmd( "ttk::checkbutton .sa.m.f2.rnd.i.le.f -text \"Reset the generator\" -variable use_seed -state disabled -command { if $use_seed { .sa.m.f2.rnd.i.le.s.e1 conf -state normal } { .sa.m.f2.rnd.i.le.s.e1 conf -state disabled } }" );
	cmd( "ttk::frame .sa.m.f2.rnd.i.le.s" );
	cmd( "ttk::label .sa.m.f2.rnd.i.le.s.l1 -text \"Seed\"" );
	cmd( "ttk::spinbox .sa.m.f2.rnd.i.le.s.e1 -width 5 -from 1 -to 9999 -state disabled -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= 9999 } { set rnd_seed %%P; return 1 } { %%W delete 0 end; %%W insert 0 $rnd_seed; set err .sa.m.f2.rnd.i.le.s.e1; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "pack .sa.m.f2.rnd.i.le.s.l1 .sa.m.f2.rnd.i.le.s.e1 -side left -padx 1" );

	cmd( "pack .sa.m.f2.rnd.i.le.f .sa.m.f2.rnd.i.le.s -side left -padx 5" );

	cmd( "pack .sa.m.f2.rnd.i.le -pady 2" );
	
	cmd( "tooltip::tooltip .sa.m.f2.rnd.i.le.f \"Ensure the generator starts from a known condition\"" );
	cmd( "tooltip::tooltip .sa.m.f2.rnd.i.le.s \"Choose the random number generator seed\"" );

	cmd( "pack .sa.m.f2.rnd.l .sa.m.f2.rnd.i" );

	cmd( "ttk::frame .sa.m.f2.ud" );
	cmd( "ttk::checkbutton .sa.m.f2.ud.c -text \"Update initialization description\" -variable update_d" );
	cmd( "pack .sa.m.f2.ud.c" );

	cmd( "pack .sa.m.f2.s .sa.m.f2.rnd .sa.m.f2.ud -expand yes -fill x" );

	cmd( "pack .sa.m.f1 .sa.m.f2 -side left -expand yes -fill both -padx 5 -pady 5" );
	cmd( "pack .sa.head .sa.m -pady 5" );

	cmd( "okhelpcancel .sa b { set choice 1 } { LsdHelp menudata_init.html#setall } { set choice 2 }" );

	cmd( "bind .sa.m.f1.rd.i <Return> {  if [ string equal [ .sa.m.f1.val.i.l2.e2 cget -state ] normal ] { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1 } }" );
	cmd( "bind .sa.m.f1.val.i.l1.e1 <Return> { if [ string equal [ .sa.m.f1.val.i.l2.e2 cget -state ] normal ] { focus .sa.m.f1.val.i.l2.e2; .sa.m.f1.val.i.l2.e2 selection range 0 end } { set choice 1 } }" );
	cmd( "bind .sa.m.f1.val.i.l2.e2 <Return> { set choice 1 }" );
	cmd( "bind .sa.m.f2.s.i.l.a.e <Return> {focus .sa.m.f2.s.i.sel.all; .sa.m.f2.s.i.sel.all invoke}" );
	cmd( "bind .sa.m.f2.s.i.sel.all <Return> {focus .sa.b.ok}" );
	cmd( "bind .sa.m.f2.s.i.sel.sel <Return> {focus .sa.m.f2.s.i.sel2.c.from; .sa.m.f2.s.i.sel2.c.from selection range 0 end }" );
	cmd( "bind .sa.m.f2.s.i.sel2.c.from <Return> {focus .sa.m.f2.s.i.sel2.c.to; .sa.m.f2.s.i.sel2.c.from selection range 0 end }" );
	cmd( "bind .sa.m.f2.s.i.sel2.c.to <Return> {focus .sa.b.ok}" );
	cmd( "bind .sa.m.f2.rnd.i.le.s.e1 <Return> {focus .sa.b.ok}" );
	
	cmd( "set err \"\"" );

	cmd( "showtop .sa topleftW" );
	cmd( "mousewarpto .sa.b.ok" );

	here_setall:

	// update current linked variables values
	cmd( "write_any .sa.m.f1.val.i.l1.e1 $value1" ); 
	cmd( "write_any .sa.m.f1.val.i.l2.e2 $value2" );
	cmd( "write_any .sa.m.f2.s.i.l.a.e $step_in" ); 
	cmd( "write_any .sa.m.f2.s.i.sel2.c.from $cases_from" ); 
	cmd( "write_any .sa.m.f2.s.i.sel2.c.to $cases_to" ); 
	cmd( "write_any .sa.m.f2.rnd.i.le.s.e1 $rnd_seed" ); 
	
	if ( selFocus )
	{
		cmd( "if { $err == \"\" } { .sa.m.f1.val.i.l1.e1 selection range 0 end; focus .sa.m.f1.val.i.l1.e1 } { $err selection range 0 end; focus $err; set err \"\" }" );
		selFocus = false;
	}

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	if ( *choice == 9 )
	{
		// search instance from
		i = compute_copyfrom( original, choice, ".sa" );
		cmd( "set cases_from %d", i );
		goto here_setall;
	}

	if ( *choice == 10 )
	{
		// search instance to
		i = compute_copyfrom( original, choice, ".sa" );
		cmd( "set cases_to %d", i );
		goto here_setall;
	}

	// save current linked variables values before closing
	cmd( "if [ string is double -strict [ .sa.m.f1.val.i.l1.e1 get ] ] { set value1 [ .sa.m.f1.val.i.l1.e1 get ] } { set err .sa.m.f1.val.i.l1.e1 }" );
	cmd( "if [ string is double -strict [ .sa.m.f1.val.i.l2.e2 get ] ] { set value2 [ .sa.m.f1.val.i.l2.e2 get ] } { set err .sa.m.f1.val.i.l2.e2 }" ); 
	cmd( "if { [ string is integer -strict [ .sa.m.f2.s.i.l.a.e get ] ] && [ .sa.m.f2.s.i.l.a.e get ] > 0 } { set step_in [ .sa.m.f2.s.i.l.a.e get ] } { set err .sa.m.f2.s.i.l.a.e }" ); 
	cmd( "if { [ string is integer -strict [ .sa.m.f2.s.i.sel2.c.from get ] ] && [ .sa.m.f2.s.i.sel2.c.from get ] > 0 } { set cases_from [ .sa.m.f2.s.i.sel2.c.from get ] } { set err .sa.m.f2.s.i.sel2.c.from }" ); 
	cmd( "if { [ string is integer -strict [ .sa.m.f2.s.i.sel2.c.to get ] ] && [ .sa.m.f2.s.i.sel2.c.to get ] > $cases_from } { set cases_to [ .sa.m.f2.s.i.sel2.c.to get ] } { set err .sa.m.f2.s.i.sel2.c.to }" ); 
	cmd( "if { [ string is integer -strict [ .sa.m.f2.rnd.i.le.s.e1 get ] ] && [ .sa.m.f2.rnd.i.le.s.e1 get ] > 0 } { set rnd_seed [ .sa.m.f2.rnd.i.le.s.e1 get ] } { set err .sa.m.f2.rnd.i.le.s.e1 }" ); 

	cmd( "if { $err != \"\" } { \
			ttk::messageBox -parent .sa -title Error -icon error -type ok -message \"Invalid value\" -detail \"Values must be numeric only and decimal numbers must use the point ('.') as the decimal separator. Choose a different value and try again.\"; \
			set choice 0 \
		}" );
		
	if ( *choice == 0 )
	{
		selFocus = true;
		goto here_setall;
	}
	
	cmd( "destroytop .sa" );

	Tcl_UnlinkVar( inter, "value1" );
	Tcl_UnlinkVar( inter, "value2" );
	Tcl_UnlinkVar( inter, "res" );
	
	if ( *choice == 2 )
		return;

	step_in = get_int( "step_in" );
	fill = get_int( "fill" );
	to_all = get_int( "to_all" );
	cases_from = get_int( "cases_from" );
	cases_to = get_int( "cases_to" );
	use_seed = get_int( "use_seed" );
	rnd_seed = get_int( "rnd_seed" );
	update_d = get_int( "update_d" );

	if ( use_seed )
		init_random( ( unsigned ) rnd_seed );
	
	j = 0;

	switch ( res )
	{
		// equal to
		case 1:							
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = value1;
					cv->data_loaded = '+';
					++j;
				}
			
			sprintf( action, "equal to %g", value1 );
			break;

		// range
		case 9:	
			for ( i = 1, cur = r, counter = -1; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( ( ( i - cases_from ) % step_in == 0 ) ) )
					counter++;

			value = ( value2 - value1 ) / counter;

			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
			{
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = value1 + value * step;
					cv->data_loaded = '+';
					++j;
				}
				
				if ( i >= cases_from && ( ( i - cases_from + 1 ) % step_in ) == 0 )
					++step; 
			}
			
			sprintf( action, "ranging from %g to %g (increments of %g)", value1, value2, value );
			break;


		// increasing	
		case 2:  
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
			{
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = value1 + step * value2;
					cv->data_loaded = '+';
					++j;
				}
				
				if ( i >= cases_from && ( ( i - cases_from + 1 ) % step_in ) == 0 )
					++step;
			}
			
			sprintf( action, "increasing from %g with step %g", value1, value2 );
			break;
				
		
		// increasing (groups)
		case 4: 
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( to_all == 1 || ( cases_from <= i && cases_to >= i ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = value1 + step * value2;
					cv->data_loaded = '+';
					++j;
					++step;        
					
					if ( cur->next != cur->hyper_next( r->label ) )
						step = 0;
				}
			
			sprintf( action, "increasing from %g with step %g for each group of objects", value1, value2 );
			break;


		// random (uniform)
		case 3: 
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = uniform( value1, value2 );
					cv->data_loaded = '+';
					++j;
				}
			
			sprintf( action, "drawn from uniform distribution between %g and %g", value1, value2 );
			break;
			  
		
		// random integer (uniform)
		case 8:
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = uniform_int( round( value1 ), round( value2 ) );
					cv->data_loaded = '+';
					++j;
				}
			
			sprintf( action, "drawn from integer uniform distribution between %g and %g", round( value1 ), round( value2 ) );
			break;
			
			
		// random (normal)
		case 5: 
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = norm( value1, value2 );
					cv->data_loaded = '+';
					++j;
				}
			
			sprintf( action, "drawn from normal distribution of mean %g and s.d. %g", value1, value2 );
			break;
			

		// import from data file
		case 7:
			cmd( "set oldpath [ pwd ]" );
			cmd( "set filename [ tk_getOpenFile -parent . -title \"File to Import Data\" -filetypes { { {Text Files} {.txt} } { {All Files} {*} } } ]" );
			l = ( char * ) Tcl_GetVar( inter, "filename", 0 );
			
			if ( l == NULL || ! strcmp( l, "" ) )
				return;

			cmd( "cd [ file dirname $filename ]" );
			cmd( "set fn [ file tail $filename ]" );
			l = ( char * ) Tcl_GetVar( inter, "fn", 0 );
			f = fopen( l, "r" );
			cmd( "cd $oldpath" );
			
			if ( f == NULL )
				return;

			if ( fscanf( f, "%99s", ch ) == EOF )				// the label
				return;
				
			for ( i = 1, cur = r; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( to_all == 1 || ( cases_from <= i && cases_to >= i ) )
				{
					kappa = fscanf( f, "%lf", &value );
					if ( kappa == EOF )
						break;
					
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = value;
					cv->data_loaded = '+';
					++j;
				}
			
			if ( cur != NULL || kappa == EOF )
				cmd( "ttk::messageBox -parent .sa -title Error -icon error -type ok -message \"Incomplete data\" -detail \"Problem loading data from file '%s', the file contains fewer values compared to the number of instances to set.\"", l );
			
			sprintf( action, "set with data from file %s", l );
			break;
			

		default:
			error_hard( "invalid option for setting values", 
						"internal problem in LSD", 
						"if error persists, please contact developers",
						true );
			myexit( 22 );
	}
	
	if ( update_d )
	{
		cd = search_description( lab );
		
		if ( step_in > 1 )
			sprintf( ch, " (every %d instances)", step_in );
		else
			strcpy( ch, "" );
		
		if ( to_all )
			if ( step_in > 1 )
				if ( cd->init != NULL )
					sprintf( msg, "%s\n%d instances %s%s", cd->init, j, action, ch );
				else
					sprintf( msg, "%d instances %s%s", j, action, ch );
			else
				sprintf( msg, "All %d instances %s%s", j, action, ch );
		else
			if ( cd->init != NULL )
				sprintf( msg, "%s\nInstances from %d to %d %s%s", cd->init, cases_from, cases_to, action, ch );
			else
				sprintf( msg, "Instances from %d to %d %s%s", cases_from, cases_to, action, ch );  
								
		change_description( lab, NULL, -1, NULL, msg );
	}
	
	unsaved_change( true );				// signal unsaved change
}


/*******************************************************************************
SENSITIVITY_PARALLEL
This function fills the initial values according to the sensitivity analysis 
system performed by parallel simulations: 1 single run over many independent 
configurations descending in parallel from Root.

Users can set one or more elements to be part of the sensitivity analysis. For 
each element the user has to provide the number of values to be explored and 
their values. When all elements involved in the sensitivity analysis are 
configured, the user must launch the command Sensitivity from menu Data in the 
main LSD Browser. This command generates as many copies as the product of all 
values for all elements in the s.a. It then kicks off the initialization of all 
elements involved so that each combination of parameters is assigned to one 
branch of the model.

The user is supposed then to save the resulting configuration.

Options concerning initialization for sensitivity analysis are not saved into 
the model configuration files, and are therefore lost when closing the LSD model
program if not saved in a .sa file. 
*******************************************************************************/
object *sensitivity_parallel( object *o, sense *s )
{
	int i;
	sense *cs;
	object *cur = o;
	variable *cvar;

	if ( s->next != NULL )
	{
		for ( i = 0; i < s->nvalues; ++i )
		{
			s->i = i;
			cur = sensitivity_parallel( cur, s->next );
		}
		
		return cur;
	}

	for ( i = 0; i < s->nvalues; ++i )
	{
		s->i = i;
		for ( cs = rsense; cs != NULL; cs = cs->next ) 
		{
			cvar = cur->search_var( cur, cs->label );
			if ( cs->param == 0 )				// handle lags > 0
				cvar->val[ cs->lag ] = cs->v[ cs->i ];
			else
				cvar->val[ 0 ] = cs->v[ cs->i ];
		}
		
		cur = cur->hyper_next( cur->label );
	}

	return cur;
}


/*******************************************************************************
SENSITIVITY_SEQUENTIAL
This function fills the initial values according to the sensitivity analysis 
system performed by sequential simulations: each run executes one configuration 
labelled with sequential labels.

Contrary to parallel sensitivity settings, this function initialize all elements 
in the configuration with the specified label.

Users can set one or more elements to be part of the sensitivity analysis. For 
each element the user has to provide the number of values to be explored and 
their values. When all elements involved in the sensitivity analysis are 
configured, the user must launch the command Sensitivity from menu Data in the 
main LSD Browser.

Options concerning initialization for sensitivity analysis are saved into model 
configuration files, to be executed with a No Window version of the LSD model. 
One configuration file is created for each possible combination of the 
sensitivity analysis values (parameters and initial conditions). Optionally, it 
is possible to define the parameter "probSampl" with the (uniform) probability 
of a given point in the sensitivity analysis space is saved as configuration 
file. In practice, this allows for the Monte Carlo sampling of the parameter 
space, which is often necessary when the s.a. space is too big to be analyzed 
in its entirety.
*******************************************************************************/
void sensitivity_sequential( int *findex, sense *s, double probSampl )
{
	int i, nv;
	sense *cs;
	object *cur;
	variable *cvar;
	
	if ( s->next != NULL )
	{
		for ( i = 0; i < s->nvalues && ! stop; ++i )
		{
			s->i = i;
			sensitivity_sequential( findex, s->next, probSampl );
		}
		
		return;
	}
	
	for ( i = 0; i < s->nvalues && ! stop; ++i )
	{
		s->i = i;
		for ( nv = 1, cs = rsense; cs != NULL; cs = cs->next ) 
		{
			nv *= cs->nvalues;
			cvar = root->search_var( root, cs->label );
			
			for ( cur = cvar->up; cur != NULL; cur = cur->hyper_next( cur->label ) )
			{
				cvar = cur->search_var( cur, cs->label ); 
				if ( cs->param == 1 )				// handle lags > 0
					cvar->val[ 0 ] = cs->v[ cs->i ];
				else
					cvar->val[ cs->lag ] = cs->v[ cs->i ];
			}

		}

		if ( probSampl == 1.0 || ran1( ) <= probSampl )	// if required draw if point will be sampled
		{
			if ( ! save_configuration( *findex ) )
			{
				plog( "Aborted\n" );
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration files cannot be saved\" -detail \"Check if the drive or the current directory is set READ-ONLY, select a drive/directory with write permission and try again.\"" );
				return;
			}
			
			if ( ( *findex + 1 ) % 10 == 0 )
				cmd( "prgboxupdate .psa %d", *findex );
			
			*findex = *findex + 1;
		}
	}
}


/*****************************************************************************
NUM_SENSITIVITY_POINTS
Calculate the sensitivity space size
******************************************************************************/
long num_sensitivity_points( sense *rsens )	
{
	long nv;
	sense *cs;
	
	for ( nv = 1, cs = rsens; cs != NULL; cs = cs->next )	// scan the linked-list
		nv *= cs->nvalues;	// update the number of variables
	return nv;
}


/*****************************************************************************
NUM_SENSITIVITY_VARIABLES
Calculate the number of variables to test
******************************************************************************/
int num_sensitivity_variables( sense *rsens )	
{
	int nv;
	sense *cs;
	
	for ( nv = 0, cs = rsens; cs != NULL; cs = cs->next)								
		if ( cs->nvalues > 1 )				// count variables with 2 or more values
			nv++;
	return nv;
}

			
/*****************************************************************************
DATAENTRY_SENSITIVITY
Try to get values for sensitivity analysis
******************************************************************************/
void dataentry_sensitivity( int *choice, sense *s, int nval )
{
	int i, j, nPar, samples, integerV;
	double start, end;
	char *sss = NULL, *tok = NULL, type;

	// reset random number generator 
	init_random( seed );

	Tcl_LinkVar( inter, "integerV", ( char * ) &integerV, TCL_LINK_BOOLEAN );
	integerV = s->entryOk ? s->integer : false;

	cmd( "set sens .sens" );
	cmd( "newtop .sens \"Sensitivity Analysis\" { set choice 2 }" );

	cmd( "ttk::frame .sens.lab" );
	if ( nval > 0)								// number of values defined (0=no)?
		cmd( "ttk::label .sens.lab.l1 -text \"Enter n=%d values for:\"", s->nvalues );
	else
		cmd( "ttk::label .sens.lab.l1 -text \"Enter the desired values (at least 2) for:\"" );

	cmd( "ttk::label .sens.lab.l2 -style hl.TLabel -text \"%s\"", s->label );
	cmd( "pack .sens.lab.l1 .sens.lab.l2 -side left -padx 2" );

	cmd( "ttk::label .sens.obs1 -text \"Paste of clipboard data is allowed, most separators are accepted\"" );
	cmd( "ttk::label .sens.obs2 -text \"Use a \'=BEGIN:END@SAMPLES%%TYPE\' clause\nto specify a number of samples within a range.\nSpaces are not allowed within clauses.\nTYPE values are \'L\' for linear and \'R\' for random samples.\" -justify center" );
	cmd( "pack .sens.lab .sens.obs1 .sens.obs2 -pady 5" );

	cmd( "ttk::frame .sens.t" );
	cmd( "ttk::scrollbar .sens.t.v_scroll -command \".sens.t.t yview\"" );
	cmd( "ttk::text .sens.t.t -height 8 -width 50 -yscroll \".sens.t.v_scroll set\" -dark $darkTheme -style smallFixed.TText" ); 
	cmd( "pack .sens.t.t .sens.t.v_scroll -side left -fill y" ); 
	cmd( "mouse_wheel .sens.t.t" );
	cmd( "pack .sens.t" );
	
	cmd( "ttk::frame .sens.pad" );
	cmd( "pack .sens.pad -pady 5" );

	cmd( "ttk::frame .sens.fb" );
	cmd( "ttk::checkbutton .sens.fb.int -variable integerV -text \"Round to integer\"" );
	cmd( "ttk::button .sens.fb.paste -width $butWid -text Paste -command { tk_textPaste .sens.t.t }" );
	cmd( "ttk::button .sens.fb.del -width $butWid -text Delete -command { .sens.t.t delete 0.0 end }" );
	cmd( "ttk::button .sens.fb.rem -width $butWid -text Remove -command { set choice 3 }" );
	cmd( "pack .sens.fb.int .sens.fb.paste .sens.fb.del .sens.fb.rem -padx $butSpc -side left" );
	cmd( "pack .sens.fb -padx $butPad -anchor e" );
	
	cmd( "tooltip::tooltip .sens.fb.int \"Force rounding to integer values\"" );
	cmd( "tooltip::tooltip .sens.fb.paste \"Insert the content of clipboard\"" );
	cmd( "tooltip::tooltip .sens.fb.del \"Delete all current values\"" );
	cmd( "tooltip::tooltip .sens.fb.rem \"Remove variable from sensitivity analysis\"" );

	cmd( "okhelpcancel .sens fb2 { set choice 1 } { LsdHelp menudata_sa.html#entry } { set choice 2 }" );
	cmd( "bind .sens.fb2.ok <KeyPress-Return> { set choice 1 }" );

	cmd( "showtop .sens topleftW" );
	cmd( "mousewarpto .sens.fb2.ok" );

	if ( s->entryOk )	// is there valid data from a previous data entry?
	{
		sss = new char[ 26 * s->nvalues + 1];	// allocate space for string
		tok = new char[ 26 + 1 ];				
		strcpy( sss, "" );
		for ( i = 0; i < s->nvalues; i++ )		// pass existing data as a string
		{
			sprintf( tok, "%.15g ", s->v[ i ] );	// add each value
			strcat( sss, tok );					// to the string
		}
		Tcl_SetVar( inter, "sss", sss, 0 ); 	// pass string to Tk window
		cmd( ".sens.t.t insert 0.0 $sss" );		// insert string in entry window
		delete [ ] tok; 
		delete [ ] sss;
	}

	cmd( "focus .sens.t.t" );

	*choice = 0;

	do										// finish only after reading all values
	{
		while ( *choice == 0 )
			Tcl_DoOneEvent( 0 );
	
		if ( *choice == 3 )					// force error to delete variable from list
		{
			s->entryOk = false;
			*choice = 2; 
		}
	
		if ( *choice == 2 )
			goto end;
	
		cmd( "set sss [ .sens.t.t get 0.0 end ]" );
		sss=( char* ) Tcl_GetVar( inter,"sss", 0 );
	
		if ( nval == 0 )					// undefined number of values?
		{	
			double temp;
			char *tss, *ss = new char[ strlen( sss ) + 1 ];
			tss = ss;						// save original pointer to gc
			strcpy( ss, sss );				// make a draft copy
			
			i = 0;							// count number of values
			do
			{
				tok = strtok( ss, SENS_SEP );	// accepts several separators
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
			s->nvalues = i;					// update # of values
			
			delete [ ] tss;
		}
	
		for ( i = 0; i < s->nvalues; )
		{
			tok = strtok( sss, SENS_SEP );	// accepts several separators
			if ( tok == NULL )				// finished too early?
			{
				cmd( "ttk::messageBox -parent .sens -title \"Sensitivity Analysis\" -icon error -type ok -message \"Invalid or less than required values\" -detail \"Decimal numbers must use the point ('.') as the decimal separator. Insert the correct number of values.\"" );
				*choice = 0;
				cmd( "focus .sens.t.t" );
				break;
			}
			
			sss = NULL;
			
			// is it a clause to be expanded?
			nPar = sscanf( tok, "=%lf:%lf@%u%%%c", &start, &end, &samples, &type );
			
			if ( nPar == 4 )				// all values are required
			{
				if ( toupper( type ) == 'L' && samples > 0 )// linear sampling
				{
					s->v[ i++ ] = integerV ? round( fmin( start, end ) ) : fmin( start, end );
					for ( int j = 1; j < samples; ++j, ++i )
					{
						s->v[ i ] = s->v[ i - 1 ] + ( fmax( start, end ) - fmin( start, end ) ) / ( samples - 1 );
						s->v[ i ] = integerV ? round( s->v[ i ] ) : s->v[ i ];
					}
				}
				if ( toupper( type ) == 'R' && samples > 0 )// random sampling 
					for ( int j = 0; j < samples; ++j, ++i )
					{
						s->v[ i ] = fmin( start, end ) + ran1( ) * ( fmax( start, end ) - fmin( start, end ) );
						s->v[ i ] = integerV ? round( s->v[ i ] ) : s->v[ i ];
					}
			}
			else											// no, read as regular double float
			{
				j = i;
				i += sscanf( tok, "%lf", &( s->v[ i ] ) );	// count valid doubles only
				s->v[ j ] = integerV ? round( s->v[ j ] ) : s->v[ j ];
			}
		}
	}
	while ( tok == NULL || i < 2 );	// require enough values (if more, extra ones are discarded)

	s->integer = integerV;			// save integer restriction flag
	s->entryOk = true;				// flag valid data

	end:
	cmd( "destroytop .sens" );
	Tcl_UnlinkVar( inter, "integerV" );
}


/*******************************************************************************
NOLH_TABLE
Calculate a Near Orthogonal Latin Hypercube (NOLH) design for sampling.
Include tables to up to 29 variables ( sanchez 2009, Cioppa and Lucas 2007).
Returns the number of samples (n) required for the calculated design and a 
pointer 	to the matrix n x k, where k is the number of factors ( variables).

It is possible to load one additional design table from disk ( file NOLH.csv in
the same folder as the configuration file .lsd). The table should be formed
by positive integers only, in the n (rows) x k ( columns), separated by commas,
one row per text line and no empty lines. The table can be loaded manually
(NOLH_load function) or automatically as needed during sampling (NOLH_sampler).
*******************************************************************************/

int **NOLH_0 = NULL;				// pointer to the design loaded from file

// function to get the index to the default NOLH design table or -1 otherwise
int NOLH_table( int k )				
{
	for ( unsigned int i = 0; i < ( ( sizeof NOLH ) / sizeof NOLH[ 0 ] ); ++i )
		if ( k >= NOLH[ i ].kMin && k <= NOLH[ i ].kMax )
			return i;
	
	return -1;						// number of factors not supported by the preloaded tables
}


/*****************************************************************************
NOLH_VALID_TABLES
Determine the valid NOLH tables for the number of factors
******************************************************************************/
char *NOLH_valid_tables( int k, char* ch )	
{
	int min_tab = NOLH_table( k );
	char buff[ MAX_ELEM_LENGTH ];
	
	if ( min_tab <= 0 )
		strcpy( ch, "External only" );
	else
	{
		strcpy( ch, "" );
		for ( int i = min_tab; ( unsigned ) i < ( ( sizeof NOLH ) / sizeof NOLH[ 0 ] ); ++i )
		{
			sprintf( buff, " \"%d\u00D7%d\u00D7%d\"", NOLH[ i ].kMax, NOLH[ i ].n1, NOLH[ i ].n2 );
			strcat( ch, buff );
		}
	}
	
	return ch;
}

			
/*****************************************************************************
NOLH_CLEAR
Function to remove table 0
******************************************************************************/
void NOLH_clear( void )				
{
	if ( NOLH_0 == NULL )			// table is not allocated?
		return;
	delete [ ] NOLH_0[ 0 ];
	delete [ ] NOLH_0;
	NOLH_0 = NULL;
	NOLH[ 0 ].kMin = NOLH[ 0 ].kMax = NOLH[ 0 ].n1 = NOLH[ 0 ].n2 = NOLH[ 0 ].loLevel = NOLH[ 0 ].hiLevel = 0;
	NOLH[ 0 ].table = NULL;
}


/*****************************************************************************
NOLH_LOAD
Function to load a .csv file named NOLH.csv as table 0 ( first to be used)
If option 'force' is used, will be used for any number of factors
******************************************************************************/
bool NOLH_load( char const baseName[ ] = NOLH_DEF_FILE, bool force = false )			
{
	int i, j, n = 1, loLevel = INT_MAX, hiLevel = 1, kFile = 0;
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
	NOLHfile = fopen( fileName, "r" );
	if ( NOLHfile == NULL )
	{
		sprintf( msg, "cannot open NOHL design file '%s'", fileName );
		error_hard( msg, "problem accessing the design of experiment file", 
					"check if the requested file exists" );
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
			NOLH_0[ i ][ j ] = atoi( num );

			if ( num == NULL || NOLH_0[ i ][ j ] == 0 )
			{
				delete [ ] NOLH_0[ 0 ];
				delete [ ] NOLH_0;
				NOLH_0 = NULL;
				sprintf( msg, "invalid format in NOHL file '%s', line=%d", fileName, i + 1 );
				error_hard( msg, "invalid design of experiment file", 
							"check the file contents" );
				goto end;
			}

			if ( NOLH_0[ i ][ j ] < loLevel )
				loLevel = NOLH_0[ i ][ j ];
			if ( NOLH_0[ i ][ j ] > hiLevel )
				hiLevel = NOLH_0[ i ][ j ];
		}
	}
	
	// set new table characteristics
	if ( force )
		NOLH[ 0 ].kMin = 1;
	else
		NOLH[ 0 ].kMin = NOLH[ sizeof NOLH / sizeof NOLH[ 0 ] - 1 ].kMax + 1;
	
	NOLH[ 0 ].kMax = kFile;
	NOLH[ 0 ].n1 = NOLH[ 0 ].n2 = n;
	NOLH[ 0 ].loLevel = loLevel;
	NOLH[ 0 ].hiLevel = hiLevel;
	NOLH[ 0 ].table = NOLH_0[ 0 ];
	
	plog( "\nNOLH file loaded: %s\nk = %d, n = %d, low level = %d, high level = %d", "", fileName, kFile, n, loLevel, hiLevel );
	
	ok = true;
end:
	delete [ ] fileName;
	delete [ ] lBuffer;
	return ok;
}


/*****************************************************************************
MAT_*
Matrix operations support functions for morris_oat() and enhancements
******************************************************************************/
// Random choice between two numbers
#define RND_CHOICE( o1, o2 ) ( ran1( ) < 0.5 ? o1 : o2 )

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
		delete [ ] a[ i ];
	delete [ ] a;
}

// multiply two matrices ( c<-a*b)
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

// add two same size matrices ( c<-a+b)
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

// Sum the Euclidean distances of points in two matrices of same size
// Calculates the distance between all points pairs and adds them
// The matrices a and b must have the same size
double mat_sum_dists( double **a, int m, int n, double **b )
{
	double sum = 0;
	for ( int i = 0; i < m ; ++i )	 		//rows in a
		for ( int k = 0; k < m; ++k )		//rows in b 
		{
			double dist2 = 0;
			for ( int j = 0; j < n ; ++j )	//columns
				dist2 += pow( a[ i ][ j ] - b[ k ][ j ], 2 );
			sum += sqrt( dist2 );
		}
	return sum;
}


/*****************************************************************************
MORRIS_OAT
	Calculate a DoE for Elementary Effects (Morris 1991) analysis,
	according to Saltelli et al 2008. Code adapted from SAlib by
	Jon Herman.
	
	Delta is fixed at p/[2(p-1)]
	
	k: number of factors
	r: number of trajectories
	p: number of grid levels
	jump: delta measured in grid levels
	X: preallocated memory area to save the trajectories
******************************************************************************/
double **morris_oat( int k, int r, int p, int jump, double **X )
{
	int i, j, l;
    double delta = ( double ) jump / ( p - 1 );	// grid step delta
	
	// reset random number generator 
	init_random( seed );

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
    // ( starts at a base point, and then changes one parameter at a time )
	
	cmd( "progressbox .psa \"Creating DoE\" \"Analyzing EE trajectories\" \"Trajectory\" %d", r );	
	
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
		
		shuffle( & perm[ 0 ], & perm[ k ], mt32 );

		P = mat_copy_scal( P, k, k, 0 );
		for ( i = 0; i < k; ++i )
			P[ i ][ perm[ i ] ] = 1;

		delete [ ] perm;

        // starting point for this trajectory
		for ( j = 0; j < k; ++j )
		{
			double start = uniform_int( 0, p - delta * ( p - 1 ) - 1 ) / ( p - 1 );
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
		
		cmd( "prgboxupdate .psa %d", l + 1 );
	}
	
	cmd( "destroytop .psa" );
	
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


/*****************************************************************************
COMPUTE_DISTANCE_MATRIX
	Optimize a DoE for Elementary Effects (Morris 1991) analysis,
	according to Campolongo et al 2007 and Ruano 2012. Code adapted
	from SAlib by Jon Herman.
	
	k: number of factors
	pool: pool of trajectories produced by morris_oat()
	M: number of trajectories in pool
	r: number of final trajectories (<= M)
	ptr: preallocated memory area to save the trajectories
******************************************************************************/
double **compute_distance_matrix( double **sample, int M, int k, double **DM )
{
	double **input_1 = mat_new( k + 1, k ), 
		   **input_2 = mat_new( k + 1, k );
	
	DM = mat_copy_scal( DM, M, M, 0 );
	
	cmd( "progressbox .psa \"Creating DoE\" \"Compute EE distance matrix\" \"Trajectory\" %d", M );
	
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
		
		cmd( "prgboxupdate .psa %d", i + 1 );
	}
		
	cmd( "destroytop .psa" );
	
	mat_del( input_1, k + 1, k );
	mat_del( input_2, k + 1, k );
	
    return DM;
}


/*****************************************************************************
COMBINATIONS
	Calculate the combinations of indices, r-to-r
******************************************************************************/
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


/*****************************************************************************
SUM_DISTANCES
  Calculate combinatorial distance between a select group of trajectories, 
  indicated by indices
    indices: list of candidate pairs of points = list < int >
    DM: distance matrix = array (M,M)
******************************************************************************/
double sum_distances( list < int > indices, double **DM )
{
	// get all combination pairs of indices
	vector < vector < int > > combs = combinations( indices, 2 );

    // add distance of all points pairs
	double D = 0;
	for ( unsigned int j = 0; j < combs.size( ); ++j )
		D += DM[ combs[ j ][ 0 ] ][ combs[ j ][ 1 ] ];
	
	return D;
}


/*****************************************************************************
TOP_IDX
	Get the top-i size items index from a unidimensional array
******************************************************************************/
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


/*****************************************************************************
GET_MAX_SUM_IND
	Get the indice that belong to the maximum distance in an array of distances
    indices_list = list of points
    distance = array (M)
******************************************************************************/
list < int > get_max_sum_ind( vector < list < int > > indices_list, vector < double > row_maxima_i )
{
	int max_idx = -1;
	double max = -INFINITY;
	
	for ( unsigned int j = 0; j < indices_list.size( ); ++j )
		if ( row_maxima_i[ j ] > max )
		{
			max_idx = j;
			max = row_maxima_i[ j ];
		}
		
	return indices_list[ max_idx ];
}


/*****************************************************************************
ADD_INDICES
	Adds extra indices for the combinatorial problem. 
	For indices = (1,2 ) and M=5, the method returns [(1,2,3),(1,2,4),(1,2,5)]
******************************************************************************/
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


/*****************************************************************************
OPT_TRAJECTORIES
	An alternative by Ruano et al. (2012 ) for the brute force approach as 
	originally proposed by Campolongo et al. (2007). The method should improve 
	the speed with which an optimal set of trajectories is found tremendously 
	for larger sample sizes.
******************************************************************************/
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

		// ######### Loop 'm' ( called loop 'k' in Ruano) ############
		m_max_ind = i_max_ind;
		// m starts at 1
        for ( int m = 1; m <= r - i - 1; ++m )
		{
			m_ind = add_indices( m_max_ind, M );
            vector < double > m_maxima( m_ind.size( ), 0 );
			
            for ( unsigned int n = 0; n < m_ind.size( ); ++n )
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
	

/*****************************************************************************
~DESIGN
	Destructor function to the design object
******************************************************************************/
design::~design( void )
{
	for ( int i = 0; i < n; i++ )		// run through all experiments
		delete [ ] ptr[ i ];				// free memory
	for ( int i = 0; i < k; i++ )		// and all variables
		delete [ ] lab[ i ];
	delete [ ] ptr;
	delete [ ] lab; 
	delete [ ] hi; 
	delete [ ] lo; 
	delete [ ] par; 
	delete [ ] lag;
	delete [ ] intg;
}


/*****************************************************************************
DESIGN
	Constructor function to the design object
		type = 1: NOLH
		type = 2: random sampling
		type = 3: Elementary Effects sampling (Morris, 1991)
		samples = -1: use extended predefined sample size (n2 )
		factors = 0: use automatic DoE size
******************************************************************************/
design::design( sense *rsens, int typ, char const *fname, int findex, 
				int samples, int factors, int jump, int trajs )
{
	int i , j, kTab, doeRange, poolSz;
	double **pool;
	char *doefname, doeName[ MAX_ELEM_LENGTH + 1 ];
	FILE *f;
	sense *cs;
	
	// reset random number generator 
	init_random( seed );

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
					error_hard( msg, "invalid design of experiment parameters", 
								"check the design" );
					goto invalid;
				}
				// if user selected # of factors, use it to select internal table
				kTab = ( factors == 0 ) ? k : factors;
			}
				
			tab = NOLH_table( kTab );		// design table to use
			if ( tab == -1 )				// number of factors too large, try to load external table ( file )
			{
				if ( NOLH_load( ) )			// tentative table load from disk ok?
				{
					tab = NOLH_table( k );	// design table to use
					if ( tab == -1 )		// still too large?
					{
						error_hard( "too many variables to test for NOLH.csv size", 
									"invalid design of experiment parameters", 
									"check the design" );
						goto invalid;		// abort
					}
				}
				else
				{
					error_hard( "too many variables to test", 
								"invalid design of experiment parameters", 
								"check the design" );
					goto invalid;			// abort
				}
			}
				
			// get the number of samples required by the NOLH design, according to user choice (basic/extended)
			n = ( samples != -1 ) ? NOLH[ tab ].n1 : NOLH[ tab ].n2;

			plog( "\nNOLH table used: %d (%s), n = %d", "", tab, tab > 0 ? "built-in" : "from file", n );
			
			// allocate memory for data
			par = new int[ k ];				// vector of variable type (parameter / lagged value )
			lag = new int[ k ];				// vector of lags
			intg = new bool[ k ];			// vector of format (integer/float)
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
				
				intg[ i ] = cs->integer;	// set variable format
				par[ i ] = cs->param;		// set variable type
				lag[ i ] = cs->lag;			// set number of lags
				
				// copy label (name )
				lab[ i ] = new char[ strlen( cs->label ) + 1 ];
				strcpy( lab[ i ], cs->label );
				
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
			par = new int[ k ];				// vector of variable type (parameter / lagged value )
			lag = new int[ k ];				// vector of lags
			intg = new bool[ k ];			// vector of format (integer/float)
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
				
				intg[ i ] = cs->integer;	// set variable format
				par[ i ] = cs->param;		// set variable type
				lag[ i ] = cs->lag;			// set number of lags
				
				// copy label (name )
				lab[ i ] = new char[ strlen( cs->label ) + 1 ];
				strcpy( lab[ i ], cs->label );
				
				i++;
			}
			
			// calculate the design of the experiment
			for ( i = 0; i < n; i++ )		// for all experiments
			{
				ptr[ i ] = new double[ k ];	// allocate 2nd level data
				for ( j = 0; j < k; j++ )	// for all factors
					ptr[ i ][ j ] = lo[ j ] + ran1( ) * ( hi[ j ] - lo[ j ] );
			}
			
			break;	
			
		case 3:								// Elementary Effects sampling
			k = num_sensitivity_variables( rsens );	// number of factors
			poolSz = samples * ( k + 1 );	// larger pool to extract samples
			n = trajs * ( k + 1 );			// number of effective samples
			if ( n < 1 || n > poolSz )		// at least one sample required
				goto invalid;
			
			// allocate memory for data
			par = new int[ k ];				// vector of variable type (parameter / lagged value )
			lag = new int[ k ];				// vector of lags
			intg = new bool[ k ];			// vector of format (integer/float)
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
				
				intg[ i ] = cs->integer;	// set variable format
				par[ i ] = cs->param;		// set variable type
				lag[ i ] = cs->lag;			// set number of lags
				
				// copy label (name )
				lab[ i ] = new char[ strlen( cs->label ) + 1 ];
				strcpy( lab[ i ], cs->label );
				
				i++;
			}
			
			// calculate the Morris OAT pool of trajectories
			pool = morris_oat( k, samples, factors, jump, pool );
			
			// select the best trajectories from pool
			ptr = opt_trajectories( k, pool, samples, trajs, ptr );
			
			for ( i = 0; i < poolSz; i++ )	// free pool memory
				delete [ ] pool[ i ];
			delete [ ] pool;
			
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
			intg = NULL;
			ptr = NULL;
			lab = NULL;
			return;
	}
	
	// generate a configuration file for the experiment
			
	// file name for saving table
	sprintf( doeName, "%u_%u", ( unsigned ) findex, ( unsigned ) ( findex + n - 1 ) );
	
	if ( strlen( path ) > 0 )				// non-default folder?
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
		{
			// round to integer if necessary
			if ( intg[ j ] )
				ptr[ i ][ j ] = round( ptr[ i ][ j ] );
			
			fprintf( f, "%lf%c", ptr[ i ][ j ], ( j == ( k - 1 ) ? '\n' : ',' ) );
		}
			
	fclose( f );
	
	plog( "\nDoE configuration saved: %s", "", doefname );
	
	delete [ ] doefname;
}


/*****************************************************************************
SENSITIVITY_DOE
	Generate the configuration files for the 
	Design of Experiment (DOe )
******************************************************************************/
void sensitivity_doe( int *findex, design *doe )
{
	int i, j;
	object *cur;
	variable *cvar;
	
	stop = false;
	cmd( "progressbox .psa \"Creating DoE\" \"Creating configuration files\" \"File\" %d { set stop true }", doe->n );
	
	for ( i = 0; i < doe->n && ! stop; ++i )	// run through all experiments
	{
		// set up the variables ( factors) with the experiment values
		for ( j = 0; j < doe->k; j++ )			// run through all factors
		{
			cvar = root->search_var( root, doe->lab[ j ] );	// find variable to set
			for ( cur = cvar->up; cur != NULL; cur = cur->hyper_next( cur->label ) )
			{									// run through all objects containing var
				cvar = cur->search_var( cur, doe->lab[ j ] ); 
				if ( doe->par[ j ] == 1 )		// handle lags > 0
					cvar->val[ 0 ] = doe->ptr[ i ][ j ];
				else
					cvar->val[ doe->lag[ j ] ] = doe->ptr[ i ][ j ];
			}
		}
		
		// generate a configuration file for the experiment
		if ( ! save_configuration( *findex ) )
		{
			plog( "Aborted\n" );
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration files cannot be saved\" -detail \"Check if the drive or the current directory is set READ-ONLY, select a drive/directory with write permission and try again.\"" );
			return;
		}
		
		if ( ( i + 2 ) % 10 == 0 )
			cmd( "prgboxupdate .psa %d", i + 1 );
			
		*findex = *findex + 1;
	}
	
	cmd( "destroytop .psa" );
	
	plog( "\nSensitivity analysis configurations produced: %d\n", "", findexSens - 1 );
		
	if ( ! stop )
		sensitivity_created( );					// explain user how to proceed
	else
		*findex = 0;							// don't consider for appending
}
