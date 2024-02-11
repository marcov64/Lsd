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
LOG.CPP
Contains the functions to create and interface with the Log
window, including when Browser is running model.
*************************************************************/

#include "LSD.h"

char tabs[ ] = "5c 7.5c 10c 12.5c 15c 17.5c 20c";	// Log window tabs


/*********************************
CREATE_LOGWINDOW
*********************************/
void create_logwindow( void )
{
	if ( ! tk_ok )
		lsd_exit_gui( 7 );

	cmd( "newtop .log \"LSD Log\" { if { [ discard_change ] eq \"ok\" && [ abort_run_threads ] eq \"ok\" } { exit } } \"\"" );

	cmd( "set w .log.text" );
	cmd( "ttk::frame $w" );
	cmd( "ttk::scrollbar $w.scroll -command \"$w.text yview\"" );
	cmd( "ttk::scrollbar $w.scrollx -command \"$w.text xview\" -orient hor" );
	cmd( "ttk::text $w.text -yscrollcommand \"$w.scroll set\" -xscrollcommand \"$w.scrollx set\" -wrap none -entry 0 -dark $darkTheme -style smallFixed.TText" );
	cmd( "mouse_wheel $w.text" );
	cmd( "$w.text configure -tabs {%s}", tabs  );

	// Log window tags
	cmd( "$w.text tag configure highlight -foreground $colorsTheme(hl)" );
	cmd( "$w.text tag configure table" );
	cmd( "$w.text tag configure series -tabs {2c 5c 8c}" );
	cmd( "$w.text tag configure prof1 -tabs {5c 7.5c 9c 11.2c 13.2c 17.5c}" );
	cmd( "$w.text tag configure prof2 -tabs {3c 6c 9c}" );

	// context menu (right mouse button)
	cmd( "ttk::menu $w.text.menu -tearoff 0" );
	cmd( "$w.text.menu add command -label Copy -underline 0 -accelerator Ctrl+C -command { tk_textCopy .log.text.text }" );		// entryconfig 0
	cmd( "$w.text.menu add command -label Clear -accelerator Ctrl+Del -command { .log.text.text.internal delete 0.0 end }" );		// entryconfig 1
	cmd( "$w.text.menu add separator" );	// entryconfig 2
	cmd( "$w.text.menu add command -label Help -accelerator F1 -command { LsdHelp log.html }" );	// entryconfig 3

	cmd( "pack $w.scroll -side right -fill y" );
	cmd( "pack $w.text -expand yes -fill both" );
	cmd( "pack $w.scrollx -side bottom -fill x" );
	cmd( "pack $w -expand yes -fill both" );

	cmd( "bind .log.text.text <Button-2> { \
			tk_popup .log.text.text.menu %%X %%Y \
		}" );
	cmd( "bind .log.text.text <Button-3> { \
			tk_popup .log.text.text.menu %%X %%Y \
		}" );

	cmd( "showtop .log none 1 1 0" );

	cmd( "bind .log <F1> { .log.text.text.menu invoke 3 }" );
	cmd( "bind .log <Escape> { focustop . }" );
	cmd( "bind .log <Control-c> { .log.text.text.menu invoke 0 }; bind .log <Control-C> { .log.text.text.menu invoke 0 }" );
	cmd( "bind .log <Control-Delete> { .log.text.text.menu invoke 1 }" );

	// replace text widget default insert, delete and replace bindings, preventing the user to change it
	cmd( "rename .log.text.text .log.text.text.internal" );
	cmd( "proc .log.text.text { args } { switch -exact -- [ lindex $args 0 ] { insert { } delete { } replace { } default { return [ eval .log.text.text.internal $args] } } }" );

	cmd( "plog \"LSD Version %s (%s)\nCopyright Marco Valente and Marcelo Pereira\nLSD is distributed under the GNU General Public License\nLSD is free software and comes with ABSOLUTELY NO WARRANTY\n[ LsdEnv {	} ]\n\"", _LSD_VERSION_, _LSD_DATE_ );

	log_ok = true;
}


/*********************************
SET_SHORTCUTS_RUN
*********************************/
void set_shortcuts_run( const char *window )
{
	if ( exists_window( window ) )
	{
		cmd( "bind %s <KeyPress-s> { catch { .b.r2.stop invoke } }; bind %s <KeyPress-S> { catch { .b.r2.stop invoke } }", window, window );
		cmd( "bind %s <KeyPress-p> { catch { .b.r2.pause invoke } }; bind %s <KeyPress-P> { catch { .b.r2.pause invoke } }", window, window );
		cmd( "bind %s <KeyPress-r> { catch { .b.r2.pause invoke } }; bind %s <KeyPress-R> { catch { .b.r2.pause invoke } }", window, window );
		cmd( "bind %s <KeyPress-f> { catch { .b.r2.speed invoke } }; bind %s <KeyPress-F> { catch { .b.r2.speed invoke } }", window, window );
		cmd( "bind %s <KeyPress-o> { catch { .b.r2.obs invoke } }; bind %s <KeyPress-O> { catch { .b.r2.obs invoke } }", window, window );
		cmd( "bind %s <KeyPress-d> { catch { .b.r2.deb invoke } }; bind %s <KeyPress-D> { catch { .b.r2.deb invoke } }", window, window );
	}
}


/*********************************
UNSET_SHORTCUTS_RUN
*********************************/
void unset_shortcuts_run( const char *window )
{
	if ( exists_window( window ) )
	{
		cmd( "bind %s <KeyPress-s> { }; bind %s <KeyPress-S> { }", window, window );
		cmd( "bind %s <KeyPress-p> { }; bind %s <KeyPress-P> { }", window, window );
		cmd( "bind %s <KeyPress-r> { }; bind %s <KeyPress-R> { }", window, window );
		cmd( "bind %s <KeyPress-f> { }; bind %s <KeyPress-F> { }", window, window );
		cmd( "bind %s <KeyPress-o> { }; bind %s <KeyPress-O> { }", window, window );
		cmd( "bind %s <KeyPress-d> { }; bind %s <KeyPress-D> { }", window, window );
	}
}


/*********************************
SET_BUTTONS_RUN
*********************************/
void set_buttons_run( bool enable )
{
	char state[ MAX_ELEM_LENGTH ];

	if ( ! exists_window( ".b.r2" ) )
		return;

	if ( enable )
		strcpy( state, "normal" );
	else
		strcpy( state, "disabled" );

	cmd( "catch { .b.r2.stop configure -state %s }", state );
	cmd( "catch { .b.r2.pause configure -state %s }", state );
	cmd( "catch { .b.r2.speed configure -state %s }", state );
	cmd( "catch { .b.r2.obs configure -state %s }", state );
	cmd( "catch { .b.r2.deb configure -state %s }", state );
}


/*********************************
COVER_BROWSER
*********************************/
void cover_browser( const char *text1, const char *text2, bool run )
{
	if ( brCovered )		// ignore if already covered
		return;

	cmd( "destroy .bbar .l" );
	cmd( "set mainMenuStates [ disable_tree .m ]" );

	cmd( "ttk::frame .t1" );
	cmd( "ttk::label .t1.l1 -justify center -text \"%s\" -style bold.TLabel", text1	 );
	cmd( "pack .t1.l1 -pady 10 -expand yes -fill y" );
	cmd( "pack .t1 -fill both -expand yes -padx 10 -pady 10" );

	if ( run )
	{
		cmd( "ttk::frame .p" );
		cmd( "ttk::label .p.l -text \"Simulation progress\" -anchor center" );

		cmd( "ttk::frame .p.b1" );
		cmd( "ttk::progressbar .p.b1.b -maximum %d -value 0", sim.last_run );
		cmd( "ttk::label .p.b1.i -text \"Simulation: 1 of %d (0%% done)\" -anchor center", sim.last_run );
		cmd( "pack .p.b1.b .p.b1.i -pady 5 -expand yes -fill x" );

		cmd( "ttk::frame .p.b2" );
		cmd( "ttk::progressbar .p.b2.b -maximum %d -value 0", sim.last_t );
		cmd( "ttk::label .p.b2.i -text \"Case: 1 of %d (0%% done)\" -anchor center", sim.last_t );
		cmd( "pack .p.b2.b .p.b2.i -pady 5 -expand yes -fill x" );

		if ( sim.last_run > 1 )
			cmd( "pack .p.l .p.b1 .p.b2 -pady 10 -expand yes -fill x" );
		else
			cmd( "pack .p.l .p.b2 -pady 10 -expand yes -fill x" );

		cmd( "pack .p -fill x -expand yes -padx 20 -pady 5" );
	}

	cmd( "ttk::frame .t2" );
	cmd( "ttk::label .t2.l1 -justify left -text \"\n%s\"", text2 );
	cmd( "pack .t2.l1 -expand yes -fill y" );
	cmd( "pack .t2 -fill both -expand yes -padx 10 -pady 10" );

	if ( run )
	{
		cmd( "if [ string equal $CurPlatform windows ] { \
				set goWid $butWid \
			} elseif [ string equal $CurPlatform linux ] { \
				set goWid $butWid \
			} { \
				set goWid [ expr { $butWid - 1 } ] \
			}" );

		cmd( "ttk::frame .b" );
		cmd( "ttk::frame .b.r2" );
		cmd( "ttk::button .b.r2.stop -width $goWid -text Stop -command { set_c_var done_in 1 } -underline 0" );
		cmd( "ttk::button .b.r2.pause -width $goWid -text Pause -command { set_c_var done_in 9 } -underline 0" );
		cmd( "ttk::button .b.r2.speed -width $goWid -text Fast -command { set_c_var done_in 2 } -underline 0" );
		cmd( "ttk::button .b.r2.obs -width $goWid -text Observe -command { set_c_var done_in 4 } -underline 0" );
		cmd( "ttk::button .b.r2.deb -width $goWid -text Debug -command { set_c_var done_in 3 } -underline 0" );
		cmd( "pack .b.r2.stop .b.r2.pause .b.r2.speed .b.r2.obs .b.r2.deb -padx $butSpc -side left" );
		cmd( "pack .b.r2" );
		cmd( "pack .b -padx $butPad -pady $butPad -side right" );

		cmd( "bind . <F1> { LsdHelp runtime.html#buttons }" );
		set_shortcuts_run( "." );
		set_shortcuts_run( ".log" );
		set_shortcuts_run( ".str" );

		// disable debug button when running in parallel mode
		if ( ! sim.parallel_disable && search_parallel( sim.root ) )
		{
			cmd( ".b.r2.deb configure -state disabled" );
			cmd( "tooltip::tooltip .b.r2.deb \"Disable parallel processing\nto enable debugging\"" );
		}
	}
	else
	{
		cmd( "set origMainTit [ wm title . ]" );
		cmd( "wm title . \"$origMainTit (DISABLED)\"" );
	}

	cmd( "update" );

	brCovered = true;
	redrawRoot = false;
}


/*********************************
UNCOVER_BROWSER
*********************************/
void uncover_browser( void )
{
	if ( ! brCovered || sim.running )	// ignore if not covered or running
		return;

	unset_shortcuts_run( "." );
	unset_shortcuts_run( ".log" );
	unset_shortcuts_run( ".str" );

	cmd( "destroytop .deb" );
	cmd( "destroy .t1 .p .t2 .b" );

	cmd( "if [ info exists origMainTit ] { \
			wm title . $origMainTit; \
			unset origMainTit \
		}" );

	cmd( "if { [ string equal [ wm state . ] normal ] } { \
			focustop . \
		}" );

	cmd( "if { [ info exists mainMenuStates ] } { enable_tree .m $mainMenuStates }" );

	brCovered = false;
	redrawRoot = true;
}


/*********************************
SHOW_PROF_AGGR
*********************************/
struct item
{
	const char *var, *obj;
	unsigned int time;
	unsigned int count;
};

bool comp_item( item& item1, item& item2 )
{
	int comp_str = strcmp( item1.obj, item2.obj );
	if ( ! comp_str )
		return item1.time > item2.time;
	else
		return comp_str < 0;
}

void show_prof_aggr( void )
{
	if ( ! sim.prof_aggr_time )
		return;

	item elem;
	list < item > vars;
	list < item >::iterator it1;
	variable *cv;
	map < string, profile >::iterator it2;

	plog( "\nProfiling aggregated results:\n" );
	plog_tag( "\nObject\tElement\tTime (msec.)\tComputation count", "prof2" );

	for ( it2 = sim.prof.begin(); it2 != sim.prof.end(); ++it2 )
	{
		elem.var = it2->first.c_str( );
		cv = sim.root->search_var( NULL, elem.var );
		elem.obj = ( cv == NULL ) ? NULL : cv->up->label;
		elem.time = 1000 * it2->second.ticks / CLOCKS_PER_SEC;
		elem.count = it2->second.comp;
		vars.push_back( elem );
	}

	vars.sort( comp_item );

	for ( it1 = vars.begin(); it1 != vars.end(); ++it1 )
		plog_tag( "\n%-12.12s\t%-12.12s\t%d\t%d", "prof2", it1->obj, it1->var, it1->time, it1->count );

	plog( "\n" );
}
