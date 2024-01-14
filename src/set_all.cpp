/*************************************************************

	LSD 9.0 - January 2024
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

- void set_all( object *r, const char *lab, int lag, const char *parWnd )
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


/****************************************************
SET_ALL
****************************************************/

void set_all( object *original, const char *lab, int lag, const char *parWnd )
{
	bool selFocus = true;
	char ch[ MAX_ELEM_LENGTH ], action[ MAX_ELEM_LENGTH ], msg[ MAX_LINE_SIZE ];
	const char *app;
	double value, value1, value2, step, counter;
	int res, i, j, kappa = 0, to_all, update_d, cases_from, cases_to, fill, use_seed, rnd_seed, step_in;
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
	cmd( "set value 1" );						// method
	cmd( "set fill 0" );
	cmd( "set to_all 1" );
	cmd( "set step_in 1" );
	cmd( "set cases_from 1" );
	cmd( "set cases_to 1000" );
	cmd( "set rnd_seed 1" );
	cmd( "set use_seed 0" );
	cmd( "set update_d 1" );

	// define the correct parent window
	if ( parWnd != NULL && strlen( parWnd ) > 0 )
		cmd( "set parWnd %s", parWnd );
	else
		cmd( "set parWnd ." );

	cmd( "if { [ string equal $parWnd . ] } { \
			set _w .sa \
		} else { \
			set _w $parWnd.sa \
		}" );

	cmd( "newtop $_w \"Set All Objects Initialization\" { set choice 2 } $parWnd" );

	cmd( "ttk::frame $_w.head" );					// heading
	cmd( "ttk::label $_w.head.lg -text \"Set initial values for\"" );

	cmd( "ttk::frame $_w.head.l" );
	if ( cv->param != 0 )
	{
		if ( cv->param == 2 )
			cmd( "ttk::label $_w.head.l.c -text \"Function: \"" );
		else
			cmd( "ttk::label $_w.head.l.c -text \"Parameter: \"" );

		cmd( "ttk::label $_w.head.l.n -text \"%s\" -style hl.TLabel", lab  );
		cmd( "pack $_w.head.l.c $_w.head.l.n -side left" );
	}
	else
	{
		cmd( "ttk::label $_w.head.l.c -text \"Variable: \"" );
		cmd( "ttk::label $_w.head.l.n1 -text \"%s  \" -style hl.TLabel", lab );
		cmd( "ttk::label $_w.head.l.n2 -text \"\\[	lag \"" );
		cmd( "ttk::label $_w.head.l.n3 -text \"%d\" -style hl.TLabel", t - cv->last_update + lag + 1  );
		cmd( "ttk::label $_w.head.l.n4 -text \"\\]\"" );
		cmd( "pack $_w.head.l.c $_w.head.l.n1 $_w.head.l.n2 $_w.head.l.n3 $_w.head.l.n4 -side left" );
	}

	cmd( "ttk::frame $_w.head.lo" );
	cmd( "ttk::label $_w.head.lo.l -text \"Contained in object: \"" );
	cmd( "ttk::label $_w.head.lo.o -text \"%s\" -style hl.TLabel", cv->up->label  );
	cmd( "pack $_w.head.lo.l $_w.head.lo.o -side left" );

	cmd( "pack $_w.head.lg $_w.head.l $_w.head.lo" );

	cmd( "ttk::frame $_w.m" );

	cmd( "ttk::frame $_w.m.f1" );					// left column

	cmd( "ttk::frame $_w.m.f1.val" );
	cmd( "ttk::label $_w.m.f1.val.l -text \"Initialization data\"" );

	cmd( "ttk::frame $_w.m.f1.val.i" );

	cmd( "ttk::frame $_w.m.f1.val.i.l1" );
	cmd( "ttk::label $_w.m.f1.val.i.l1.l1 -text \"Equal to\"" );
	cmd( "ttk::entry $_w.m.f1.val.i.l1.e1 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set value1 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value1; set err $_w.m.f1.val.i.l1.e1; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "pack $_w.m.f1.val.i.l1.l1 $_w.m.f1.val.i.l1.e1" );

	cmd( "ttk::frame $_w.m.f1.val.i.l2" );
	cmd( "ttk::label $_w.m.f1.val.i.l2.l2 -text \"(none)\"" );
	cmd( "ttk::entry $_w.m.f1.val.i.l2.e2 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set value2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value2; set err $_w.m.f1.val.i.l2.e2; set choice 1; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
	cmd( "pack $_w.m.f1.val.i.l2.l2 $_w.m.f1.val.i.l2.e2" );

	cmd( "pack $_w.m.f1.val.i.l1 $_w.m.f1.val.i.l2 -expand yes -fill x	-ipadx 5 -ipady 2" );

	cmd( "pack $_w.m.f1.val.l $_w.m.f1.val.i" );

	cmd( "ttk::frame $_w.m.f1.rd" );
	cmd( "ttk::label $_w.m.f1.rd.l -text \"Initialization method\"" );

	cmd( "ttk::frame $_w.m.f1.rd.i -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton $_w.m.f1.rd.i.r1 -text \"Equal to\" -variable res -value 1 -command { $_w.m.f1.val.i.l1.l1 conf -text \"Value\"; $_w.m.f1.val.i.l1.e1 conf -state normal; $_w.m.f1.val.i.l2.l2 conf -text \"(none)\"; $_w.m.f1.val.i.l2.e2 conf -state disabled; $_w.m.f2.s.i.l.a.e conf -state normal; $_w.m.f2.s.i.l.f conf -state normal; set use_seed 0; $_w.m.f2.rnd.i.le.f conf -state disabled; $_w.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind $_w.m.f1.rd.i.r1 <Down> { focus $_w.m.f1.rd.i.r9; $_w.m.f1.rd.i.r9 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r1 <Return> { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 }" );

	cmd( "ttk::radiobutton $_w.m.f1.rd.i.r9 -text \"Range\" -variable res -value 9 -command { $_w.m.f1.val.i.l1.l1 conf -text \"Minimum\"; $_w.m.f1.val.i.l1.e1 conf -state normal; $_w.m.f1.val.i.l2.l2 conf -text \"Maximum\"; $_w.m.f1.val.i.l2.e2 conf -state normal; $_w.m.f2.s.i.l.a.e conf -state normal; $_w.m.f2.s.i.l.f conf -state normal; set use_seed 0; $_w.m.f2.rnd.i.le.f conf -state disabled; $_w.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind $_w.m.f1.rd.i.r9 <Down> { focus $_w.m.f1.rd.i.r2; $_w.m.f1.rd.i.r2 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r9 <Up> { focus $_w.m.f1.rd.i.r1; $_w.m.f1.rd.i.r1 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r9 <Return> { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 }" );

	cmd( "ttk::radiobutton $_w.m.f1.rd.i.r2 -text \"Increasing\" -variable res -value 2 -command { $_w.m.f1.val.i.l1.l1 conf -text \"Start\"; $_w.m.f1.val.i.l1.e1 conf -state normal; $_w.m.f1.val.i.l2.l2 conf -text \"Step\"; $_w.m.f1.val.i.l2.e2 conf -state normal; $_w.m.f2.s.i.l.a.e conf -state normal; $_w.m.f2.s.i.l.f conf -state normal; set use_seed 0; $_w.m.f2.rnd.i.le.f conf -state disabled; $_w.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind $_w.m.f1.rd.i.r2 <Down> { focus $_w.m.f1.rd.i.r4; $_w.m.f1.rd.i.r4 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r2 <Up> { focus $_w.m.f1.rd.i.r9; $_w.m.f1.rd.i.r9 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r2 <Return> { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 }" );

	cmd( "ttk::radiobutton $_w.m.f1.rd.i.r4 -text \"Increasing (groups)\" -variable res -value 4 -command { $_w.m.f1.val.i.l1.l1 conf -text \"Start\"; $_w.m.f1.val.i.l1.e1 conf -state normal; $_w.m.f1.val.i.l2.l2 conf -text \"Step\"; $_w.m.f1.val.i.l2.e2 conf -state normal; set step_in 1; $_w.m.f2.s.i.l.a.e conf -state disabled; $_w.m.f2.s.i.l.f conf -state disabled; set use_seed 0; $_w.m.f2.rnd.i.le.f conf -state disabled; $_w.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind $_w.m.f1.rd.i.r4 <Up> { focus $_w.m.f1.rd.i.r2; $_w.m.f1.rd.i.r2 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r4 <Down> { focus $_w.m.f1.rd.i.r3; $_w.m.f1.rd.i.r3 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r4 <Return> { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 }" );

	cmd( "ttk::radiobutton $_w.m.f1.rd.i.r3 -text \"Random (uniform)\" -variable res -value 3 -command { $_w.m.f1.val.i.l1.l1 conf -text \"Minimum\"; $_w.m.f1.val.i.l1.e1 conf -state normal; $_w.m.f1.val.i.l2.l2 conf -text \"Maximum\"; $_w.m.f1.val.i.l2.e2 conf -state normal; $_w.m.f2.s.i.l.a.e conf -state normal; $_w.m.f2.s.i.l.f conf -state normal; $_w.m.f2.rnd.i.le.f conf -state normal }" );
	cmd( "bind $_w.m.f1.rd.i.r3 <Up> { focus $_w.m.f1.rd.i.r4; $_w.m.f1.rd.i.r4 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r3 <Down> { focus $_w.m.f1.rd.i.r8; $_w.m.f1.rd.i.r8 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r3 <Return> { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 }" );

	cmd( "ttk::radiobutton $_w.m.f1.rd.i.r8 -text \"Random integer (uniform)\" -variable res -value 8 -command { $_w.m.f1.val.i.l1.l1 conf -text \"Minimum\"; $_w.m.f1.val.i.l1.e1 conf -state normal; $_w.m.f1.val.i.l2.l2 conf -text \"Maximum\"; $_w.m.f1.val.i.l2.e2 conf -state normal; $_w.m.f2.s.i.l.a.e conf -state normal; $_w.m.f2.s.i.l.f conf -state normal; $_w.m.f2.rnd.i.le.f conf -state normal }" );
	cmd( "bind $_w.m.f1.rd.i.r8 <Up> { focus $_w.m.f1.rd.i.r3; $_w.m.f1.rd.i.r3 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r8 <Down> { focus $_w.m.f1.rd.i.r5; $_w.m.f1.rd.i.r5 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r8 <Return> { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 }" );

	cmd( "ttk::radiobutton $_w.m.f1.rd.i.r5 -text \"Random (normal)\" -variable res -value 5 -command { $_w.m.f1.val.i.l1.l1 conf -text \"Mean\"; $_w.m.f1.val.i.l1.e1 conf -state normal; $_w.m.f1.val.i.l2.l2 conf -text \"Std. deviation\"; $_w.m.f1.val.i.l2.e2 conf -state normal; $_w.m.f2.s.i.l.a.e conf -state normal; $_w.m.f2.s.i.l.f conf -state normal; $_w.m.f2.rnd.i.le.f conf -state normal }" );
	cmd( "bind $_w.m.f1.rd.i.r5 <Up> { focus $_w.m.f1.rd.i.r8; $_w.m.f1.rd.i.r8 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r5 <Down> { focus $_w.m.f1.rd.i.r7; $_w.m.f1.rd.i.r7 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r5 <Return> { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 }" );

	cmd( "ttk::radiobutton $_w.m.f1.rd.i.r7 -text \"Import from data file\" -variable res -value 7 -command { $_w.m.f1.val.i.l1.l1 conf -text \"(none)\"; $_w.m.f1.val.i.l1.e1 conf -state disabled; $_w.m.f1.val.i.l2.l2 conf -text \"(none)\"; $_w.m.f1.val.i.l2.e2 conf -state disabled; set step_in 1; $_w.m.f2.s.i.l.a.e conf -state disabled; $_w.m.f2.s.i.l.f conf -state disabled; set use_seed 0; $_w.m.f2.rnd.i.le.f conf -state disabled; $_w.m.f2.rnd.i.le.s.e1 conf -state disabled }" );
	cmd( "bind $_w.m.f1.rd.i.r7 <Up> { focus $_w.m.f1.rd.i.r5; $_w.m.f1.rd.i.r5 invoke }" );
	cmd( "bind $_w.m.f1.rd.i.r7 <Return> { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 }" );

	cmd( "pack $_w.m.f1.rd.i.r1 $_w.m.f1.rd.i.r9 $_w.m.f1.rd.i.r2 $_w.m.f1.rd.i.r4 $_w.m.f1.rd.i.r3 $_w.m.f1.rd.i.r8 $_w.m.f1.rd.i.r5 $_w.m.f1.rd.i.r7 -anchor w" );

	cmd( "tooltip::tooltip $_w.m.f1.rd.i.r1 \"Every instance set to the same Value\"" );
	cmd( "tooltip::tooltip $_w.m.f1.rd.i.r9 \"Linear range from Minimum to Maximum\"" );
	cmd( "tooltip::tooltip $_w.m.f1.rd.i.r2 \"From Start plus Increasing for each instance\"" );
	cmd( "tooltip::tooltip $_w.m.f1.rd.i.r4 \"From Start in each group plus Increasing for each instance\"" );
	cmd( "tooltip::tooltip $_w.m.f1.rd.i.r3 \"Uniform random real draw from Minimum to Maximum\"" );
	cmd( "tooltip::tooltip $_w.m.f1.rd.i.r8 \"Uniform random integer draw from Minimum to Maximum\"" );
	cmd( "tooltip::tooltip $_w.m.f1.rd.i.r5 \"Random draw from normal distribution with Mean and Standard deviation\"" );
	cmd( "tooltip::tooltip $_w.m.f1.rd.i.r7 \"Read initialization data from disk file\"" );

	cmd( "pack $_w.m.f1.rd.l $_w.m.f1.rd.i" );

	cmd( "pack $_w.m.f1.val $_w.m.f1.rd -expand yes -fill x -padx 5 -pady 5" );

	cmd( "ttk::frame $_w.m.f2" );					// right column

	cmd( "ttk::frame $_w.m.f2.s" );
	cmd( "ttk::label $_w.m.f2.s.tit -text \"Object instance selection\"" );

	cmd( "ttk::frame $_w.m.f2.s.i" );

	cmd( "ttk::frame $_w.m.f2.s.i.l" );

	cmd( "ttk::frame $_w.m.f2.s.i.l.a" );
	cmd( "ttk::label $_w.m.f2.s.i.l.a.l -text \"Apply every\"" );
	cmd( "ttk::spinbox $_w.m.f2.s.i.l.a.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set step_in %%P; return 1 } { %%W delete 0 end; %%W insert 0 $step_in; set err $_w.m.f2.s.i.l.a.e; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "ttk::label $_w.m.f2.s.i.l.a.l1 -text \"instance( s)\"" );
	cmd( "pack $_w.m.f2.s.i.l.a.l $_w.m.f2.s.i.l.a.e $_w.m.f2.s.i.l.a.l1 -side left -padx 1" );

	cmd( "ttk::checkbutton $_w.m.f2.s.i.l.f -text \"Fill-in\" -variable fill" );
	cmd( "pack	$_w.m.f2.s.i.l.a $_w.m.f2.s.i.l.f -padx 5 -side left" );
	cmd( "pack	$_w.m.f2.s.i.l -pady 2" );

	cmd( "tooltip::tooltip $_w.m.f2.s.i.l.a \"Number of instances to skip from initializing\"" );
	cmd( "tooltip::tooltip $_w.m.f2.s.i.l.f \"Fill intermediate instances with same value\"" );

	cmd( "ttk::frame $_w.m.f2.s.i.sel -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton $_w.m.f2.s.i.sel.all -text \"Apply to all instances\" -variable to_all -value 1 -command { $_w.m.f2.s.i.sel2.c.to conf -state disabled; $_w.m.f2.s.i.sel2.c.from conf -state disabled; bind $_w.m.f2.s.i.sel2.c.from <Button-3> { }; bind $_w.m.f2.s.i.sel2.c.to <Button-3> { }; bind $_w.m.f2.s.i.sel2.c.from <Button-2> { }; bind $_w.m.f2.s.i.sel2.c.to <Button-2> { } }" );
	cmd( "ttk::radiobutton $_w.m.f2.s.i.sel.sel -text \"Apply to a range of instances\" -variable to_all -value 0 -command { $_w.m.f2.s.i.sel2.c.to conf -state normal; $_w.m.f2.s.i.sel2.c.from conf -state normal; bind $_w.m.f2.s.i.sel2.c.from <Button-3> { set choice 9 }; bind $_w.m.f2.s.i.sel2.c.to <Button-3> { set choice 10 }; bind $_w.m.f2.s.i.sel2.c.from <Button-2> { set choice 9 }; bind $_w.m.f2.s.i.sel2.c.to <Button-2> { set choice 10 } }" );
	cmd( "pack $_w.m.f2.s.i.sel.all $_w.m.f2.s.i.sel.sel -anchor w" );

	cmd( "tooltip::tooltip $_w.m.f2.s.i.sel.all \"Apply initialization to all instances\"" );
	cmd( "tooltip::tooltip $_w.m.f2.s.i.sel.sel \"Apply initialization to a range of instances\"" );

	cmd( "pack $_w.m.f2.s.i.sel -pady 2" );

	cmd( "ttk::frame $_w.m.f2.s.i.sel2" );

	cmd( "ttk::frame $_w.m.f2.s.i.sel2.c" );
	cmd( "ttk::label $_w.m.f2.s.i.sel2.c.lfrom -text \"From\"" );
	cmd( "ttk::spinbox $_w.m.f2.s.i.sel2.c.from -width 5 -from 1 -to 9999 -state disabled -state disabled -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set cases_from %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cases_from; set err $_w.m.f2.s.i.sel2.c.from; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "ttk::label $_w.m.f2.s.i.sel2.c.lto -text \"to\"" );
	cmd( "ttk::spinbox $_w.m.f2.s.i.sel2.c.to -width 5 -from 1 -to 9999 -state disabled -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set cases_to %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cases_to; set err $_w.m.f2.s.i.sel2.c.to; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "pack $_w.m.f2.s.i.sel2.c.lfrom $_w.m.f2.s.i.sel2.c.from $_w.m.f2.s.i.sel2.c.lto $_w.m.f2.s.i.sel2.c.to -side left -pady 1" );

	cmd( "ttk::label $_w.m.f2.s.i.sel2.obs -text \"(use right button on cells for options)\"" );
	cmd( "pack $_w.m.f2.s.i.sel2.c $_w.m.f2.s.i.sel2.obs" );
	cmd( "pack $_w.m.f2.s.i.sel2 -pady 2" );

	cmd( "tooltip::tooltip $_w.m.f2.s.i.sel2 \"Select first and last instance to initialize\"" );

	cmd( "pack $_w.m.f2.s.tit $_w.m.f2.s.i" );

	cmd( "pack $_w.m.f2.s" );

	cmd( "ttk::frame $_w.m.f2.rnd" );
	cmd( "ttk::label $_w.m.f2.rnd.l -text \"Random number generator\"" );

	cmd( "ttk::frame $_w.m.f2.rnd.i" );

	cmd( "ttk::frame $_w.m.f2.rnd.i.le" );
	cmd( "ttk::checkbutton $_w.m.f2.rnd.i.le.f -text \"Reset the generator\" -variable use_seed -state disabled -command { if $use_seed { $_w.m.f2.rnd.i.le.s.e1 conf -state normal } { $_w.m.f2.rnd.i.le.s.e1 conf -state disabled } }" );
	cmd( "ttk::frame $_w.m.f2.rnd.i.le.s" );
	cmd( "ttk::label $_w.m.f2.rnd.i.le.s.l1 -text \"Seed\"" );
	cmd( "ttk::spinbox $_w.m.f2.rnd.i.le.s.e1 -width 5 -from 1 -to 9999 -state disabled -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set rnd_seed %%P; return 1 } { %%W delete 0 end; %%W insert 0 $rnd_seed; set err $_w.m.f2.rnd.i.le.s.e1; set choice 1; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "pack $_w.m.f2.rnd.i.le.s.l1 $_w.m.f2.rnd.i.le.s.e1 -side left -padx 1" );

	cmd( "pack $_w.m.f2.rnd.i.le.f $_w.m.f2.rnd.i.le.s -side left -padx 5" );

	cmd( "pack $_w.m.f2.rnd.i.le -pady 2" );

	cmd( "tooltip::tooltip $_w.m.f2.rnd.i.le.f \"Ensure the generator starts from a known condition\"" );
	cmd( "tooltip::tooltip $_w.m.f2.rnd.i.le.s \"Choose the random number generator seed\"" );

	cmd( "pack $_w.m.f2.rnd.l $_w.m.f2.rnd.i" );

	cmd( "ttk::frame $_w.m.f2.ud" );
	cmd( "ttk::checkbutton $_w.m.f2.ud.c -text \"Update initialization description\" -variable update_d" );
	cmd( "pack $_w.m.f2.ud.c" );

	cmd( "pack $_w.m.f2.s $_w.m.f2.rnd $_w.m.f2.ud -expand yes -fill x" );

	cmd( "pack $_w.m.f1 $_w.m.f2 -side left -expand yes -fill both -padx 5 -pady 5" );
	cmd( "pack $_w.head $_w.m -pady 5" );

	cmd( "okhelpcancel $_w b { set choice 1 } { LsdHelp menudata_init.html#setall } { set choice 2 }" );

	cmd( "bind $_w.m.f1.rd.i <Return> { if [ string equal [ $_w.m.f1.val.i.l2.e2 cget -state ] normal ] { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 } }" );
	cmd( "bind $_w.m.f1.val.i.l1.e1 <Return> { if [ string equal [ $_w.m.f1.val.i.l2.e2 cget -state ] normal ] { focus $_w.m.f1.val.i.l2.e2; $_w.m.f1.val.i.l2.e2 selection range 0 end } { set choice 1 } }" );
	cmd( "bind $_w.m.f1.val.i.l2.e2 <Return> { set choice 1 }" );
	cmd( "bind $_w.m.f2.s.i.l.a.e <Return> { focus $_w.m.f2.s.i.sel.all; $_w.m.f2.s.i.sel.all invoke }" );
	cmd( "bind $_w.m.f2.s.i.sel.all <Return> { focus $_w.b.ok }" );
	cmd( "bind $_w.m.f2.s.i.sel.sel <Return> { focus $_w.m.f2.s.i.sel2.c.from; $_w.m.f2.s.i.sel2.c.from selection range 0 end }" );
	cmd( "bind $_w.m.f2.s.i.sel2.c.from <Return> { focus $_w.m.f2.s.i.sel2.c.to; $_w.m.f2.s.i.sel2.c.from selection range 0 end }" );
	cmd( "bind $_w.m.f2.s.i.sel2.c.to <Return> { focus $_w.b.ok }" );
	cmd( "bind $_w.m.f2.rnd.i.le.s.e1 <Return> { focus $_w.b.ok }" );

	cmd( "set err \"\"" );

	cmd( "showtop $_w centerW" );
	cmd( "mousewarpto $_w.b.ok 0" );

	here_setall:

	// update current linked variables values
	cmd( "write_any $_w.m.f1.val.i.l1.e1 $value1" );
	cmd( "write_any $_w.m.f1.val.i.l2.e2 $value2" );
	cmd( "write_any $_w.m.f2.s.i.l.a.e $step_in" );
	cmd( "write_any $_w.m.f2.s.i.sel2.c.from $cases_from" );
	cmd( "write_any $_w.m.f2.s.i.sel2.c.to $cases_to" );
	cmd( "write_any $_w.m.f2.rnd.i.le.s.e1 $rnd_seed" );

	if ( selFocus )
	{
		cmd( "if { $err == \"\" } { $_w.m.f1.val.i.l1.e1 selection range 0 end; focus $_w.m.f1.val.i.l1.e1 } { $err selection range 0 end; focus $err; set err \"\" }" );
		selFocus = false;
	}

	choice = 0;
	while ( choice == 0 )
		Tcl_DoOneEvent( 0 );

	if ( choice == 9 )
	{
		// search instance from
		i = compute_copyfrom( original, "$_w" );
		cmd( "set cases_from %d", i );
		goto here_setall;
	}

	if ( choice == 10 )
	{
		// search instance to
		i = compute_copyfrom( original, "$_w" );
		cmd( "set cases_to %d", i );
		goto here_setall;
	}

	// save current linked variables values before closing
	cmd( "if [ string is double -strict [ $_w.m.f1.val.i.l1.e1 get ] ] { set value1 [ $_w.m.f1.val.i.l1.e1 get ] } { set err $_w.m.f1.val.i.l1.e1 }" );
	cmd( "if [ string is double -strict [ $_w.m.f1.val.i.l2.e2 get ] ] { set value2 [ $_w.m.f1.val.i.l2.e2 get ] } { set err $_w.m.f1.val.i.l2.e2 }" );
	cmd( "if { [ string is integer -strict [ $_w.m.f2.s.i.l.a.e get ] ] && [ $_w.m.f2.s.i.l.a.e get ] > 0 } { set step_in [ $_w.m.f2.s.i.l.a.e get ] } { set err $_w.m.f2.s.i.l.a.e }" );
	cmd( "if { [ string is integer -strict [ $_w.m.f2.s.i.sel2.c.from get ] ] && [ $_w.m.f2.s.i.sel2.c.from get ] > 0 } { set cases_from [ $_w.m.f2.s.i.sel2.c.from get ] } { set err $_w.m.f2.s.i.sel2.c.from }" );
	cmd( "if { [ string is integer -strict [ $_w.m.f2.s.i.sel2.c.to get ] ] && [ $_w.m.f2.s.i.sel2.c.to get ] > $cases_from } { set cases_to [ $_w.m.f2.s.i.sel2.c.to get ] } { set err $_w.m.f2.s.i.sel2.c.to }" );
	cmd( "if { [ string is integer -strict [ $_w.m.f2.rnd.i.le.s.e1 get ] ] && [ $_w.m.f2.rnd.i.le.s.e1 get ] > 0 } { set rnd_seed [ $_w.m.f2.rnd.i.le.s.e1 get ] } { set err $_w.m.f2.rnd.i.le.s.e1 }" );

	cmd( "if { $err != \"\" } { \
			ttk::messageBox -parent $_w -title Error -icon error -type ok -message \"Invalid value\" -detail \"Values must be numeric only and decimal numbers must use the point ('.') as the decimal separator. Choose a different value and try again.\"; \
			set choice 0 \
		}" );

	if ( choice == 0 )
	{
		selFocus = true;
		goto here_setall;
	}

	cmd( "destroytop $_w" );

	Tcl_UnlinkVar( inter, "value1" );
	Tcl_UnlinkVar( inter, "value2" );
	Tcl_UnlinkVar( inter, "res" );

	if ( choice == 2 )
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
					cv->initialized = true;
					++j;
				}

			snprintf( action, MAX_ELEM_LENGTH, "equal to %g", value1 );
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
					cv->initialized = true;
					++j;
				}

				if ( i >= cases_from && ( ( i - cases_from + 1 ) % step_in ) == 0 )
					++step;
			}

			snprintf( action, MAX_ELEM_LENGTH, "ranging from %g to %g (increments of %g)", value1, value2, value );
			break;


		// increasing
		case 2:
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
			{
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = value1 + step * value2;
					cv->initialized = true;
					++j;
				}

				if ( i >= cases_from && ( ( i - cases_from + 1 ) % step_in ) == 0 )
					++step;
			}

			snprintf( action, MAX_ELEM_LENGTH, "increasing from %g with step %g", value1, value2 );
			break;


		// increasing (groups)
		case 4:
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( to_all == 1 || ( cases_from <= i && cases_to >= i ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = value1 + step * value2;
					cv->initialized = true;
					++j;
					++step;

					if ( cur->next != cur->hyper_next( r->label ) )
						step = 0;
				}

			snprintf( action, MAX_ELEM_LENGTH, "increasing from %g with step %g for each group of objects", value1, value2 );
			break;


		// random (uniform)
		case 3:
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = uniform( value1, value2 );
					cv->initialized = true;
					++j;
				}

			snprintf( action, MAX_ELEM_LENGTH, "drawn from uniform distribution between %g and %g", value1, value2 );
			break;


		// random integer (uniform)
		case 8:
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = uniform_int( round( value1 ), round( value2 ) );
					cv->initialized = true;
					++j;
				}

			snprintf( action, MAX_ELEM_LENGTH, "drawn from integer uniform distribution between %g and %g", round( value1 ), round( value2 ) );
			break;


		// random (normal)
		case 5:
			for ( i = 1, cur = r, step = 0; cur != NULL; cur = cur->hyper_next( r->label ), ++i )
				if ( ( to_all == 1 || ( cases_from <= i && cases_to >= i ) ) && ( fill == 1 || ( ( i - cases_from ) % step_in == 0 ) ) )
				{
					cv = cur->search_var( NULL, lab );
					cv->val[ lag ] = norm( value1, value2 );
					cv->initialized = true;
					++j;
				}

			snprintf( action, MAX_ELEM_LENGTH, "drawn from normal distribution of mean %g and s.d. %g", value1, value2 );
			break;


		// import from data file
		case 7:
			cmd( "set oldpath [ pwd ]" );
			cmd( "set filename [ tk_getOpenFile -parent . -title \"File to Import Data\" -filetypes { { {Text Files} {.txt} } { {All Files} {*} } } ]" );
			app = get_str( "filename" );
			if ( app == NULL || ! strcmp( app, "" ) )
				return;

			cmd( "cd [ file dirname $filename ]" );
			app = eval_str( "[ file tail $filename ]" );
			f = fopen( app, "r" );
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
					cv->initialized = true;
					++j;
				}

			if ( cur != NULL || kappa == EOF )
				cmd( "ttk::messageBox -parent $_w -title Error -icon error -type ok -message \"Incomplete data\" -detail \"Problem loading data from file '%s', the file contains fewer values compared to the number of instances to set.\"", app );

			snprintf( action, MAX_ELEM_LENGTH, "set with data from file %s", app );
			break;


		default:
			error_hard( "internal problem in LSD",
						"if error persists, please contact developers",
						true,
						"invalid option for setting values" );
			myexit( 22 );
	}

	if ( update_d )
	{
		cd = search_description( lab );

		if ( step_in > 1 )
			snprintf( ch, MAX_ELEM_LENGTH, " (every %d instances)", step_in );
		else
			strcpy( ch, "" );

		if ( to_all )
			if ( step_in > 1 )
				if ( cd->init != NULL )
					snprintf( msg, MAX_LINE_SIZE, "%s\n%d instances %s%s", cd->init, j, action, ch );
				else
					snprintf( msg, MAX_LINE_SIZE, "%d instances %s%s", j, action, ch );
			else
				snprintf( msg, MAX_LINE_SIZE, "All %d instances %s%s", j, action, ch );
		else
			if ( cd->init != NULL )
				snprintf( msg, MAX_LINE_SIZE, "%s\nInstances from %d to %d %s%s", cd->init, cases_from, cases_to, action, ch );
			else
				snprintf( msg, MAX_LINE_SIZE, "Instances from %d to %d %s%s", cases_from, cases_to, action, ch );

		change_description( lab, NULL, -1, NULL, msg );
	}

	unsaved_change( true );				// signal unsaved change
}
