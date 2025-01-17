/*************************************************************

	LSD 8.1 - July 2023
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
INTERF.CPP
Manages the main interfaces, that the browser and all the menus.
It is re-build any time the window changes. There are many actions that are
commanded from the browser window, implemented as a switch in operate.

The main functions contained in this file are:

- object *create( void )
The main cycle for the Browser, from which it exits only to run a simulation
or to quit the program. The cycle is just once call to browsw followed by
a call to operate.

- int browse( object *r );
build the browser window and waits for an action (on the form of
values for choice or choice_g different from 0)

- object *operate( object *r );
takes the value of choice and operate the relative command on the
object r. See the switch for the complete list of the available commands

- void show_save( object *n )
shows all variables to be saved in the result files

- void show_debug( object *n );
shows all variables to debug or watch

- void show_plot( object *n );
shows all variables to plot during run time

- void clean_save( object *n );
remove all the flags to save from any variable in the model

- void clean_debug( object *n );
remove all the flags to debug and watch from any variable in the model

- void clean_plot( object *n );
remove all the flags to plot from any variable in the model

- void wipe_out( object *d );
Eliminate all the Object like d from the model
Cancel also the their descendants

...

*************************************************************/

/*
USED CASE 97
*/

#include "decl.h"

bool check_save;					// control saving message inside disabled objects
bool initVal = false;				// new variable initial setting going on
bool redrawReq = false;				// flag for asynchronous window redraw request
const char *res_g;
int natBat = true;					// native (Windows/Linux) batch format flag (bool)
int next_lag;						// new variable initial setting next lag to set
int result_loaded;
int lcount;
object *initParent = NULL;			// parent of new variable initial setting


// list of choices that are bad with existing run data
int badChoices[ ] = { 1, 2, 3, 6, 7, 19, 21, 22, 27, 28, 30, 31, 32, 33, 36, 43, 57, 58, 59, 62, 63, 64, 65, 68, 69, 71, 72, 74, 75, 76, 77, 78, 79, 80, 81, 83, 88, 90, 91, 92, 93, 94, 95, 96 };
#define NUM_BAD_CHOICES ( sizeof( badChoices ) / sizeof( badChoices[ 0 ] ) )

// list of choices that are run twice (called from another choice)
int redoChoices[ ] = { 32, 33, 55, 73, 74, 75, 76, 77, 78, 79, 80, 83, 96 };
#define NUM_REDO_CHOICES ( sizeof( redoChoices ) / sizeof( redoChoices[ 0 ] ) )

// comparison function for bsearch and qsort
int comp_ints ( const void *a, const void *b ) { return ( *( int * ) a - *( int * ) b ); }


/****************************************************
CREATE
****************************************************/
void create( void )
{
	object *cur;

	Tcl_LinkVar( inter, "strWindowOn", ( char * ) &strWindowOn, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( inter, "actual_steps", ( char * ) &actual_steps, TCL_LINK_INT );

	// sort the list of choices with existing run data to use later
	qsort( badChoices, NUM_BAD_CHOICES, sizeof ( int ), comp_ints );
	qsort( redoChoices, NUM_REDO_CHOICES, sizeof ( int ), comp_ints );

	cmd( "set ignore_eq_file %d", ignore_eq_file ? 1 : 0  );
	cmd( "set listfocus 1" );
	cmd( "set prevlistfocus 0" );
	cmd( "set itemfocus 0" );
	cmd( "set itemfirst 0" );
	cmd( "set c \"\"" );

	// restore previous object and cursor position in browser, if any
	cur = restore_pos( root );
	redrawRoot = redrawStruc = true;	// browser/ structure redraw when drawing the first time
	choice_g = choice = 0;

	// main cycle
	while ( choice != 1 )
	{
		cmd( "wm title . \"%s%s - LSD Browser\"", unsaved_change( ) ? "*" : " ", strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME );
		cmd( "wm title .log \"%s%s - LSD Log\"", unsaved_change( ) ? "*" : " ", strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME );

		// find root and minimally check the configuration
		if ( struct_loaded && root->v == NULL && root->b == NULL )
		{
			error_hard( "corrupted configuration file or internal problem in LSD",
						"if error persists, please contact developers",
						false,
						"invalid model configuration loaded" );
			unload_configuration( true );
			cur = root;
		}

		if ( message_logged )
		{
			cmd( "focustop .log" );
			message_logged = false;
		}

		// browse only if not running two-cycle operations
		if ( bsearch( & choice, redoChoices, NUM_REDO_CHOICES, sizeof ( int ), comp_ints ) == NULL )
			choice = browse( cur );

		// check if configuration was just reloaded
		if ( choice < 0 )
		{
			choice = - choice;
			cur = currObj;				// restore pointed object
		}

		cur = operate( cur );
	}

	Tcl_UnlinkVar( inter, "strWindowOn" );
	Tcl_UnlinkVar( inter, "actual_steps" );
}


/****************************************************
BROWSE
****************************************************/
int browse( object *r )
{
	bool done, sp_upd;
	int i, num;
	bridge *cb;
	variable *cv;

	currObj = r;			// global pointer to C Tcl routines

	// main LSD window - avoids redrawing if not required
	if ( redrawRoot )
	{
		cmd( "destroy .t .l" );
		cmd( "ttk::frame .l" );

		cmd( "ttk::frame .l.v" );

		cmd( "ttk::frame .l.v.c" );
		cmd( "ttk::scrollbar .l.v.c.v_scroll -command \".l.v.c.var_name yview\"" );
		cmd( "ttk::listbox .l.v.c.var_name -selectmode browse -yscroll \".l.v.c.v_scroll set\" -dark $darkTheme" );
		cmd( "mouse_wheel .l.v.c.var_name" );
		cmd( "tooltip::tooltip clear .l.v.c.var_name*" );

		// populate the variables panel
		if ( r->v == NULL )
			cmd( ".l.v.c.var_name insert end \"(none)\"; set nVar 0" );
		else
		{
			for ( cv = r->v, i = 0; cv != NULL; cv = cv->next, ++i )
			{
				// special updating scheme?
				if ( cv->param == 0 && ( cv->delay > 0 || cv->delay_range > 0 || cv->period > 1 || cv->period_range > 0 ) )
					sp_upd = true;
				else
					sp_upd = false;

				// set flags string
				cmd( "set varFlags \"%s%s%s%s%s%s\"", ( cv->save || cv->savei ) ? "+" : "", cv->plot ? "*" : "", ( cv->deb_mode == 'd' || cv->deb_mode == 'W' || cv->deb_mode == 'R' ) ? "!" : "", ( cv->deb_mode == 'w' || cv->deb_mode == 'W' ) ? "?" : "", ( cv->deb_mode == 'r' || cv->deb_mode == 'R' ) ? "\u00BF" : "", cv->parallel ? "&" : "", sp_upd ? "\u00A7" : "" );

				// add elements to the listbox
				if ( cv->param == 0 )
				{
					if ( cv->num_lag == 0 )
					{
						cmd( ".l.v.c.var_name insert end \"%s (V$varFlags)\"", cv->label );
						cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(var)", i );
					}
					else
					{
						cmd( ".l.v.c.var_name insert end \"%s (V_%d$varFlags)\"", cv->label, cv->num_lag );
						cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(lvar)", i );
					}
				}

				if ( cv->param == 1 )
				{
					cmd( ".l.v.c.var_name insert end \"%s (P$varFlags)\"", cv->label );
					cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(par)", i );
				}

				if ( cv->param == 2 )
				{
					if ( cv->num_lag == 0 )
					{
						cmd( " .l.v.c.var_name insert end \"%s (F$varFlags)\"", cv->label );
						cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(fun)", i );
					}
					else
					{
						cmd( ".l.v.c.var_name insert end \"%s (F_%d$varFlags)\"", cv->label, cv->num_lag );
						cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(lfun)", i );
					}
				}

				set_ttip_descr( ".l.v.c.var_name", cv->label, i );
			}

			cmd( "set nVar [ .l.v.c.var_name size ]" );
		}

		cmd( "ttk::label .l.v.lab -text \"Variables & parameters ($nVar)\"" );

		// variables panel context menu (right mouse button)
		cmd( "ttk::menu .l.v.c.var_name.v -tearoff 0" );
		cmd( ".l.v.c.var_name.v add command -label Change -accelerator Enter -command { set choice 7 }" );	// entryconfig 0
		cmd( ".l.v.c.var_name.v add command -label Properties -accelerator F2 -command { set choice 75 }" );	// entryconfig 1
		cmd( ".l.v.c.var_name.v add command -label \"Updating (\u00A7)\" -state disabled -command { set choice 96 }" );	// entryconfig 2
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 3
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Save (+)\" -variable save -accelerator F5 -command { set ctxMenuCmd \"set_var_conf $vname save $save\"; set choice 95 }" );	// entryconfig 4
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Run Plot (*)\" -variable plot -accelerator F6 -command { set ctxMenuCmd \"set_var_conf $vname plot $plot\"; set choice 95 }" );	// entryconfig 5
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Debug (!)\" -state disabled -variable debug -accelerator F7 -command { set ctxMenuCmd \"set_var_conf $vname debug $debug\"; set choice 95 }" );	// entryconfig 6
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Watch (?)\" -state disabled -variable watch -accelerator F8 -command { set watch_write 0; set ctxMenuCmd \"set_var_conf $vname watch $watch\"; set choice 95 }" );	// entryconfig 7
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Watch Write (\u00BF)\" -state disabled -variable watch_write -accelerator F9 -command { set watch 0; set ctxMenuCmd \"set_var_conf $vname watch_write $watch_write\"; set choice 95 }" );	// entryconfig 8
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Parallel (&)\" -state disabled -variable parallel -accelerator F10 -command { set ctxMenuCmd \"set_var_conf $vname parallel $parallel\"; set choice 95 }" );	// entryconfig 9
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 10
		cmd( ".l.v.c.var_name.v add command -label \"Move Up\" -accelerator \"Ctrl+\u2191\" -state disabled -command { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { $itemfocus > 0 } { incr itemfocus -1 }; set choice 58 }" );	// entryconfig 11
		cmd( ".l.v.c.var_name.v add command -label \"Move Down\" -accelerator \"Ctrl+\u2193\" -state disabled -command { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { $itemfocus < [ expr { [ .l.v.c.var_name size ] - 1 } ] } { incr itemfocus }; set choice 59 }" );	// entryconfig 12
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 13
		cmd( ".l.v.c.var_name.v add command -label Move -command { set choice 79 }" );	// entryconfig 14
		cmd( ".l.v.c.var_name.v add command -label Delete -accelerator Del -command { set choice 76 }" );	// entryconfig 15
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 16
		cmd( ".l.v.c.var_name.v add command -label Equation -state disabled -command { set choice 29 }" );	// entryconfig 17
		cmd( ".l.v.c.var_name.v add command -label Using -state disabled -command { set choice 46 }" );	// entryconfig 18
		cmd( ".l.v.c.var_name.v add command -label \"Used In\" -state disabled -command { set choice 47 }" );	// entryconfig 19
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 20
		cmd( ".l.v.c.var_name.v add command -label \"Initial Values\" -state disabled -command { set choice 77 }" );	// entryconfig 21
		cmd( ".l.v.c.var_name.v add command -label Sensitivity -state disabled -command { set choice 78 }" );	// entryconfig 22

		// variables panel bindings
		if ( r->v != NULL )
		{
			cmd( "bind .l.v.c.var_name <Return> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 7 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Double-Button-1> { \
					set dblclk 1; \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						after idle { set choice 7 } \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Button-2> { \
					.l.v.c.var_name selection clear 0 end; \
					.l.v.c.var_name selection set @%%x,%%y; \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						.l.v.c.var_name.v entryconfig 2 -state normal; \
						.l.v.c.var_name.v entryconfig 6 -state normal; \
						.l.v.c.var_name.v entryconfig 7 -state normal; \
						.l.v.c.var_name.v entryconfig 8 -state normal; \
						.l.v.c.var_name.v entryconfig 9 -state normal; \
						.l.v.c.var_name.v entryconfig 11 -state normal; \
						.l.v.c.var_name.v entryconfig 12 -state normal; \
						.l.v.c.var_name.v entryconfig 17 -state normal; \
						.l.v.c.var_name.v entryconfig 18 -state normal; \
						.l.v.c.var_name.v entryconfig 19 -state normal; \
						.l.v.c.var_name.v entryconfig 21 -state normal; \
						.l.v.c.var_name.v entryconfig 22 -state normal; \
						set save [ get_var_conf $vname save ]; \
						set plot [ get_var_conf $vname plot ]; \
						set debug [ get_var_conf $vname debug ]; \
						set watch [ get_var_conf $vname watch ]; \
						set watch_write [ get_var_conf $vname watch_write ]; \
						set parallel [ get_var_conf $vname parallel ]; \
						if [ string equal $color $colorsTheme(var) ] { \
							.l.v.c.var_name.v entryconfig 21 -state disabled; \
							.l.v.c.var_name.v entryconfig 22 -state disabled; \
						} elseif [ string equal $color $colorsTheme(par) ] { \
							.l.v.c.var_name.v entryconfig 2 -state disabled; \
							.l.v.c.var_name.v entryconfig 6 -state disabled; \
							.l.v.c.var_name.v entryconfig 9 -state disabled; \
							.l.v.c.var_name.v entryconfig 17 -state disabled; \
							.l.v.c.var_name.v entryconfig 18 -state disabled \
						} elseif [ string equal $color $colorsTheme(lfun) ] { \
							.l.v.c.var_name.v entryconfig 2 -state disabled; \
							.l.v.c.var_name.v entryconfig 7 -state disabled; \
							.l.v.c.var_name.v entryconfig 8 -state disabled; \
							.l.v.c.var_name.v entryconfig 9 -state disabled \
						} elseif [ string equal $color $colorsTheme(fun) ] { \
							.l.v.c.var_name.v entryconfig 2 -state disabled; \
							.l.v.c.var_name.v entryconfig 7 -state disabled; \
							.l.v.c.var_name.v entryconfig 8 -state disabled; \
							.l.v.c.var_name.v entryconfig 9 -state disabled; \
							.l.v.c.var_name.v entryconfig 21 -state disabled; \
							.l.v.c.var_name.v entryconfig 22 -state disabled; \
						}; \
						if { $itemfocus == 0 } { \
							.l.v.c.var_name.v entryconfig 11 -state disabled \
						}; \
						if { $itemfocus == [ expr { [ .l.v.c.var_name size ] - 1 } ] } { \
							.l.v.c.var_name.v entryconfig 12 -state disabled \
						}; \
						tk_popup .l.v.c.var_name.v %%X %%Y \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Button-3> { \
					event generate .l.v.c.var_name <Button-2> -x %%x -y %%y \
				}" );
			cmd( "bind .l.v.c.var_name <Control-Up> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					if { $itemfocus > 0 } { \
						incr itemfocus -1 \
					}; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 58 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Control-Down> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					if { $itemfocus < [ expr { [ .l.v.c.var_name size ] - 1 } ] } { \
						incr itemfocus \
					}; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 59 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Delete> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 76 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <F2> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 75 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <F3> { \
					set listfocus 1; \
					set sort_order 0; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <F4> { \
					set listfocus 1; \
					set sort_order 1; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <Shift-F3> { \
					set listfocus 1; \
					set sort_order 2; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <Shift-F4> { \
					set listfocus 1; \
					set sort_order 3; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <Control-F3> { \
					set listfocus 1; \
					set sort_order 4; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <Control-F4> { \
					set listfocus 1; \
					set sort_order 5; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <F5> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $actual_steps == 0 } { \
						set save [ expr { ! [ get_var_conf $vname save ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname save $save\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <plus> { \
					event generate .l.v.c.var_name <F5> \
				}" );
			cmd( "bind .l.v.c.var_name <F6> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $actual_steps == 0 } { \
						set plot [ expr { ! [ get_var_conf $vname plot ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname plot $plot\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <asterisk> { \
					event generate .l.v.c.var_name <F6> \
				}" );
			cmd( "bind .l.v.c.var_name <F7> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $actual_steps == 0 && ! [ string equal $color $colorsTheme(par) ] } { \
						set debug [ expr { ! [ get_var_conf $vname debug ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname debug $debug\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <exclam> { \
					event generate .l.v.c.var_name <F7> \
				}" );
			cmd( "bind .l.v.c.var_name <F8> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $actual_steps == 0 && ! [ string equal $color $colorsTheme(lfun) ] && ! [ string equal $color $colorsTheme(fun) ] } { \
						set watch_write 0; \
						set watch [ expr { ! [ get_var_conf $vname watch ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname watch $watch\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <question> { \
					event generate .l.v.c.var_name <F8> \
				}" );
			cmd( "bind .l.v.c.var_name <F9> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $actual_steps == 0 && ! [ string equal $color $colorsTheme(lfun) ] && ! [ string equal $color $colorsTheme(fun) ] } { \
						set watch 0; \
						set watch_write [ expr { ! [ get_var_conf $vname watch_write ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname watch_write $watch_write\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <questiondown> { \
					event generate .l.v.c.var_name <F9> \
				}" );
			cmd( "bind .l.v.c.var_name <F11> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $actual_steps == 0 && ! [ string equal $color $colorsTheme(par) ] && ! [ string equal $color $colorsTheme(lfun) ] && ! [ string equal $color $colorsTheme(fun) ] } { \
						set parallel [ expr { ! [ get_var_conf $vname parallel ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname parallel $parallel\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <ampersand> { \
					event generate .l.v.c.var_name <F11> \
				}" );
			cmd( "bind .l.v.c.var_name <KeyRelease> { \
					if { ( %%s & 0x20004 ) != 0 } { \
						return \
					}; \
					set kk %%K; \
					if { [ string equal $kk underscore ] || ( [ string length $kk ] == 1 && [ string is alpha -strict $kk ] ) } { \
						if [ string equal $kk underscore ] { \
							set kk _ \
						}; \
						set ll %%W; \
						set ff [ lsearch -start [ expr { [ $ll curselection ] + 1 } ] -nocase [ $ll get 0 end ] \"${kk}*\" ]; \
						if { $ff == -1 } { \
							set ff [ lsearch -start 0 -nocase [ $ll get 0 end ] \"${kk}*\" ] \
						}; \
						if { $ff >= 0 } { \
							selectinlist $ll $ff \
						} \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Home> { \
					selectinlist .l.v.c.var_name 0; \
					break \
				}" );
			cmd( "bind .l.v.c.var_name <End> { \
					selectinlist .l.v.c.var_name end; \
					break \
				}" );
		}

		cmd( "bind .l.v.c.var_name <Button-1> { \
				set dblclk 0; \
				after 200; \
				if { ! $dblclk } { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					upd_menu_visib \
				} \
			}" );
		cmd( "bind .l.v.c.var_name <Left> { \
				focus .l.s.c.son_name; \
				set listfocus 2; \
				set itemfocus 0; \
				selectinlist .l.s.c.son_name 0; \
				upd_menu_visib \
			}" );

		cmd( "ttk::frame .l.s" );

		cmd( "ttk::frame .l.s.c" );
		cmd( "ttk::scrollbar .l.s.c.v_scroll -command \".l.s.c.son_name yview\"" );
		cmd( "ttk::listbox .l.s.c.son_name -selectmode browse -yscroll \".l.s.c.v_scroll set\" -dark $darkTheme" );
		cmd( "mouse_wheel .l.s.c.son_name" );
		cmd( "tooltip::tooltip clear .l.s.c.son_name*" );

		if ( r->up != NULL )
		{
			cmd( ".l.s.c.son_name insert end \"$upSymbol\"" );
			cmd( "tooltip::tooltip .l.s.c.son_name -item 0 \"%s\"", r->up->label );
			i = 1;
		}
		else
			i = 0;

		cmd( "set upObjItem %d", i );

		if ( r->up == NULL && r->b == NULL )
			cmd( ".l.s.c.son_name insert end \"(none)\"" );
		else
		{
			// populate the objects panel
			for ( cb = r->b; cb != NULL; cb = cb->next, ++i )
			{
				if ( cb->head != NULL )
				{
					skip_next_obj( cb->head, &num );
					done = cb->head->to_compute;
				}
				else
				{
					num = 0;
					done = true;
				}

				cmd( ".l.s.c.son_name insert end \"%s (#%d%s)\"", cb->blabel, num, done ? "" : "-" );
				cmd( ".l.s.c.son_name itemconf %d -fg $colorsTheme(obj)", i );

				set_ttip_descr( ".l.s.c.son_name", cb->blabel, i );
			}
		}

		cmd( "ttk::label .l.s.lab -text \"Descending objects ([ expr { %d - $upObjItem } ])\"", i );

		// objects panel context menu (right mouse button)
		cmd( "ttk::menu .l.s.c.son_name.v -tearoff 0" );
		cmd( ".l.s.c.son_name.v add command -label \"Select\" -accelerator Enter -command { set choice 4 }" );	// entryconfig 0
		cmd( ".l.s.c.son_name.v add command -label \"Parent\" -accelerator Back -command { set choice 5 }" );	// entryconfig 1
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 2
		cmd( ".l.s.c.son_name.v add command -label \"Move Up\" -accelerator \"Ctrl+\u2191\" -state disabled -command { set listfocus 2; set itemfocus [ .l.s.c.son_name curselection ]; if { $itemfocus > 0 } { incr itemfocus -1 }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 60 } }" );	// entryconfig 3
		cmd( ".l.s.c.son_name.v add command -label \"Move Down\" -accelerator \"Ctrl+\u2193\" -state disabled -command { set listfocus 2; set itemfocus [ .l.s.c.son_name curselection ]; if { $itemfocus < [ expr { [ .l.s.c.son_name size ] - 1 } ] } { incr itemfocus }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 61 } }" );	// entryconfig 4
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 5
		cmd( ".l.s.c.son_name.v add command -label Change -accelerator \"Ctrl+Enter\" -command { set choice 6 }" );	// entryconfig 6
		cmd( ".l.s.c.son_name.v add command -label Rename -accelerator F2 -command { set choice 83 }" );	// entryconfig 7
		cmd( ".l.s.c.son_name.v add command -label Number -command { set choice 33 }" );	// entryconfig 8
		cmd( ".l.s.c.son_name.v add command -label Move -command { set choice 32 }" );	// entryconfig 9
		cmd( ".l.s.c.son_name.v add command -label Delete -accelerator Del -command { set choice 74 }" );	// entryconfig 10
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 11
		cmd( ".l.s.c.son_name.v add cascade -label Add -menu .l.s.c.son_name.v.a" );	// entryconfig 12=14
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 13
		cmd( ".l.s.c.son_name.v add checkbutton -label \"Not Compute (-)\" -variable nocomp -accelerator F5 -command { set ctxMenuCmd \"set_obj_conf $vname comp [ expr { ! $nocomp } ]\"; set choice 95 }" );	// entryconfig 14
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 15
		cmd( ".l.s.c.son_name.v add command -label \"Initial Values\" -accelerator \"Ctrl+I\" -command { set choice 21 }" );	// entryconfig 16
		cmd( ".l.s.c.son_name.v add command -label \"Browse Data\" -accelerator \"Ctrl+B\" -command { set choice 34 }" );	// entryconfig 17
		cmd( "ttk::menu .l.s.c.son_name.v.a -tearoff 0" );
		cmd( ".l.s.c.son_name.v.a add command -label Variable -accelerator \"Ctrl+V\" -command { set choice 2; set param 0 }" );
		cmd( ".l.s.c.son_name.v.a add command -label Parameter -accelerator \"Ctrl+P\" -command { set choice 2; set param 1 }" );
		cmd( ".l.s.c.son_name.v.a add command -label Function -accelerator \"Ctrl+N\" -command { set choice 2; set param 2 }" );
		cmd( ".l.s.c.son_name.v.a add command -label Object -accelerator \"Ctrl+D\" -command { set choice 3 }" );

		// objects panel bindings
		if ( r->up != NULL || r->b != NULL )
		{
			cmd( "bind .l.s.c.son_name <Return> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { $upObjItem && $itemfocus == 0 } { \
						set choice 5 \
					} { \
						if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
							set choice 4 \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Control-Return> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set useCurrObj no; \
						set choice 6 \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Double-Button-1> { \
					set dblclk 1; \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { $upObjItem && $itemfocus == 0 } { \
						set choice 5 \
					} { \
						if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
							after idle { set choice 4 } \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Button-2> { \
					.l.s.c.son_name selection clear 0 end; \
					.l.s.c.son_name selection set @%%x,%%y; \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set useCurrObj no; \
						set nocomp [ expr { ! [ get_obj_conf $vname comp ] } ]; \
						if { $itemfocus == 0 } { \
							.l.s.c.son_name.v entryconfig 3 -state disabled \
						} { \
							.l.s.c.son_name.v entryconfig 3 -state normal \
						}; \
						if { $itemfocus == [ expr { [ .l.s.c.son_name size ] - 1 } ] } { \
							.l.s.c.son_name.v entryconfig 4 -state disabled \
						} { \
							.l.s.c.son_name.v entryconfig 4 -state normal \
						}; \
						tk_popup .l.s.c.son_name.v %%X %%Y \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Button-3> { \
					event generate .l.s.c.son_name <Button-2> -x %%x -y %%y \
				}" );
			cmd( "bind .l.s.c.son_name <Control-Up> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) } { \
						if { $itemfocus > 0 } { \
							incr itemfocus -1 \
						}; \
						if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
							set choice 60 \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Control-Down> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) } { \
						if { $itemfocus < [ expr { [ .l.s.c.son_name size ] - 1 } ] } { \
							incr itemfocus \
						}; \
						if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
							set choice 61 \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Delete> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 74 \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <F2> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 83 \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <F3> { \
					set listfocus 2; \
					set sort_order 0; \
					set choice 94 \
				}" );
			cmd( "bind .l.s.c.son_name <F4> { \
					set listfocus 2; \
					set sort_order 1; \
					set choice 94 \
				}" );
			cmd( "bind .l.s.c.son_name <F5> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $actual_steps == 0 } { \
						set nocomp [ expr { ! [ get_obj_conf $vname comp ] } ]; \
						set ctxMenuCmd \"set_obj_conf $vname comp $nocomp\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <minus> { \
					event generate .l.s.c.son_name <F5> \
				}" );
			cmd( "bind .l.s.c.son_name <KeyRelease> { \
					if { ( %%s & 0x20004 ) != 0 } { \
						return \
					}; \
					set kk %%K; \
					if { [ string equal $kk underscore ] || ( [ string length $kk ] == 1 && [ string is alpha -strict $kk ] ) } { \
						if [ string equal $kk underscore ] { \
							set kk _ \
						}; \
						set ll %%W; \
						set ff [ lsearch -start [ expr { [ $ll curselection ] + 1 } ] -nocase [ $ll get 0 end ] \"${kk}*\" ]; \
						if { $ff == -1 } { \
							set ff [ lsearch -start 0 -nocase [ $ll get 0 end ] \"${kk}*\" ] \
						}; \
						if { $ff >= 0 } { \
							selectinlist $ll $ff \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Home> { \
					selectinlist .l.s.c.son_name 0; \
					break \
				}" );
			cmd( "bind .l.s.c.son_name <End> { \
					selectinlist .l.s.c.son_name end; \
					break \
				}" );
		}

		cmd( "bind .l.s.c.son_name <BackSpace> { set choice 5 }" );

		cmd( "bind .l.s.c.son_name <Button-1> { \
				set dblclk 0; \
				after 200; \
				if { ! $dblclk } { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					upd_menu_visib \
				} \
			}" );
		cmd( "bind .l.s.c.son_name <Right> { \
				focus .l.v.c.var_name; \
				set listfocus 1; \
				set itemfocus 0; \
				selectinlist .l.v.c.var_name 0; \
				upd_menu_visib \
			}" );

		// navigation (top) panel
		cmd( "ttk::frame .l.p" );

		cmd( "ttk::frame .l.p.up_name" );
		cmd( "ttk::label .l.p.up_name.d -text \"Parent object:\" -width 15 -anchor w" );
		if ( r->up != NULL )
		{
			cmd( "ttk::label .l.p.up_name.n -text \" %s \" -anchor w -style hl.TLabel", r->up->label );
			cmd( "bind . <KeyPress-u> { set itemfocus 0; set choice 5 }; bind . <KeyPress-U> { set itemfocus 0; set choice 5 }" );
		}
		else
			cmd( "ttk::label .l.p.up_name.n -anchor w -text \"\"" );

		cmd( "pack .l.p.up_name.d .l.p.up_name.n -side left" );
		cmd( "pack .l.p.up_name -padx 5 -anchor w" );

		cmd( "ttk::frame .l.p.tit" );
		cmd( "ttk::label .l.p.tit.lab -text \"Current object:\" -width 15 -anchor w" );
		cmd( "ttk::button .l.p.tit.but -width -1 -text \" %s \" -style hlBold.Toolbutton %s", r->label, r->up == NULL ? "" : "-command { set choice 6 }" );

		if ( r->up != NULL )
			cmd( "tooltip::tooltip .l.p.tit.but \"Change...\"" );
		else
			cmd( ".l.p.tit.but configure -state disabled" );

		cmd( "pack .l.p.tit.lab .l.p.tit.but -side left" );
		cmd( "pack .l.p.tit -padx 5 -anchor w" );

		// main menu - avoid redrawing the menu if it already exists and is configured
		if ( ! exists_window( ".m" ) || ! expr_eq( "[ . cget -menu ]", ".m" ) )
		{
			cmd( "destroy .m" );
			cmd( "ttk::menu .m -tearoff 0" );

			cmd( "set w .m.file" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label File -menu $w -underline 0" );
			cmd( "$w add command -label \"Load...\" -underline 0 -accelerator Ctrl+L -command { set choice 17 }" );
			cmd( "$w add command -label Reload -underline 0 -accelerator Ctrl+W -command { set choice 38 }" );
			cmd( "$w add command -label Save -underline 0 -accelerator Ctrl+S -command { set choice 18 }" );
			cmd( "$w add command -label \"Save As...\" -underline 5 -command { set choice 73 }" );
			cmd( "$w add command -label Unload -underline 0 -accelerator Ctrl+E -command { set choice 20 }" );
			cmd( "$w add command -label \"Compare...\" -underline 0 -command { set choice 82 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Save Results...\" -underline 2 -accelerator Ctrl+Z -command { set choice 37 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Load Network...\" -underline 5 -command { set choice 88 }" );
			cmd( "$w add command -label \"Save Network...\" -underline 8 -command { set choice 89 }" );
			cmd( "$w add command -label \"Unload Network\" -underline 3 -command { set choice 93 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Load Sensitivity...\" -underline 3 -command { set choice 64 }" );
			cmd( "$w add command -label \"Save Sensitivity...\" -underline 6 -command { set choice 65 }" );
			cmd( "$w add command -label \"Unload Sensitivity\" -underline 11 -command { set choice 67 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Export Saved Elements...\" -underline 1 -command { set choice 91 }" );
			cmd( "$w add command -label \"Export Sensitivity Limits...\" -underline 2 -command { set choice 90 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Set Equation File...\" -underline 2 -accelerator Ctrl+U -command { set choice 28 }" );
			cmd( "$w add command -label \"Restore Equation File...\" -underline 1 -command { set choice 52 }" );
			cmd( "$w add command -label \"Compare Equation Files...\" -underline 2 -command { set choice 53 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label Quit -underline 0 -accelerator Ctrl+Q -command { set choice 11 }" );

			cmd( "set w .m.model" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label Model -menu $w -underline 0" );
			cmd( "$w add command -label \"Add Variable...\" -underline 4 -accelerator Ctrl+V -command { set param 0; set choice 2 }" );	// entryconfig 0
			cmd( "$w add command -label \"Add Parameter...\" -underline 4 -accelerator Ctrl+P -command { set param 1; set choice 2 }" );	// entryconfig 1
			cmd( "$w add command -label \"Add Function...\" -underline 5 -accelerator Ctrl+N -command { set param 2; set choice 2 }" );	// entryconfig 2
			cmd( "$w add command -label \"Add Object...\" -underline 4 -accelerator Ctrl+D -command { set choice 3 }" );	// entryconfig 3

			cmd( "$w add separator" );	// entryconfig 4

			cmd( "$w add command -label \"Change Element...\" -underline 0 -accelerator Enter -command { set useCurrObj yes; set choice 7 }" );	// entryconfig 5
			cmd( "$w add command -label \"Change Object...\" -underline 7 -accelerator Ctrl+Enter -command { set useCurrObj yes; set choice 6 }" );	// entryconfig 6
			cmd( "$w add command -label \"Find Element...\" -underline 0 -accelerator Ctrl+F -command { set choice 50 }" );	// entryconfig 7

			cmd( "$w add cascade -label \"Sort Elements\" -underline 0 -menu $w.sort" );	// entryconfig 8

			cmd( "$w add separator" );	// entryconfig 9

			cmd( "$w add command -label \"Create Model Report...\" -underline 7 -command { set choice 36 }" );	// entryconfig 10
			cmd( "$w add command -label \"Create LaTex Tables\" -underline 9 -command { set choice 57 }" );	// entryconfig 11
			cmd( "$w add command -label \"Create LaTex References\" -underline 13 -command { set choice 92 }" );	// entryconfig 12
			cmd( "$w add command -label \"Import Descriptions\" -underline 0 -command { set choice 43 }" );	// entryconfig 13

			cmd( "$w add separator" );	// entryconfig 14

			cmd( "set strWindowChk $strWindowOn" );
			cmd( "$w add checkbutton -label \"Enable Structure Window\" -underline 17 -accelerator Ctrl+Tab -variable strWindowChk -command { set choice 70 }" );	// entryconfig 15
			cmd( "$w add checkbutton -label \"Ignore Equation File\" -underline 0 -variable ignore_eq_file -command { set choice 54 }" );	// entryconfig 16

			cmd( "set w .m.model.sort" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( "$w add command -label \"Ascending (alphabetic only)\" -underline 0 -accelerator F3 -command { set sort_order 0; set choice 94 }" );
			cmd( "$w add command -label \"Descending (alphabetic only)\" -underline 0 -accelerator F4 -command { set sort_order 1; set choice 94 }" );
			cmd( "$w add command -label \"Ascending (parameters first)\" -underline 11 -accelerator Shift+F3 -command { set sort_order 2; set choice 94 }" );
			cmd( "$w add command -label \"Descending (parameters first)\" -underline 18 -accelerator Shift+F4 -command { set sort_order 3; set choice 94 }" );
			cmd( "$w add command -label \"Ascending (variables first)\" -underline 11 -accelerator Ctrl+F3 -command { set sort_order 4; set choice 94 }" );
			cmd( "$w add command -label \"Descending (variables first)\" -underline 17 -accelerator Ctrl+F4 -command { set sort_order 5; set choice 94 }" );

			cmd( "set w .m.data" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label Data -menu $w -underline 0" );
			cmd( "$w add command -label \"Initial Values...\" -command { set choice 21 } -underline 0 -accelerator Ctrl+I" );
			cmd( "$w add command -label \"Numbers of Objects....\" -command { set choice 19 } -accelerator Ctrl+O -underline 0" );

			cmd( "$w add separator" );

			cmd( "$w add cascade -label \"Sensitivity Analysis\" -underline 0 -menu $w.setsens" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Analysis of Results...\" -command { set choice 26 } -underline 0 -accelerator Ctrl+A" );
			cmd( "$w add command -label \"Analysis of MC Experiment...\" -command { set choice 12 } -underline 0" );
			cmd( "$w add command -label \"Data Browse...\" -command { set choice 34 } -underline 5 -accelerator Ctrl+B" );

			cmd( "set w .m.data.setsens" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( "$w add command -label \"Full (online)\" -underline 0 -command { set choice 62 }" );
			cmd( "$w add command -label \"Full (batch)\" -underline 6 -command { set choice 63 }" );
			cmd( "$w add command -label \"MC Point Sampling (batch)...\" -underline 0 -command { set choice 71 }" );
			cmd( "$w add command -label \"MC Range Sampling (batch)...\" -underline 3 -command { set choice 80 }" );
			cmd( "$w add command -label \"EE Sampling (batch)...\" -underline 0 -command { set choice 81 }" );
			cmd( "$w add command -label \"NOLH Sampling (batch)...\" -underline 0 -command { set choice 72 }" );

			cmd( "set w .m.run" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label Run -menu $w -underline 0" );
			cmd( "$w add command -label Run -underline 0 -accelerator Ctrl+R -command { set choice 1 }" );
			cmd( "$w add command -label \"Parallel Run...\" -underline 0 -command { set choice 69 }" );
			cmd( "$w add command -label \"Parallel Batch...\" -underline 9 -command { set choice 68 }" );
			cmd( "$w add separator" );
			cmd( "$w add command -label \"Simulation Settings...\" -underline 2 -accelerator Ctrl+M -command { set choice 22 }" );

			cmd( "$w add separator" );

			cmd( "$w add cascade -label \"Show Elements to\" -underline 17 -menu $w.show" );
			cmd( "$w add cascade -label \"Remove Flags to\" -underline 15 -menu $w.rem" );

			cmd( "set w .m.run.show" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( "$w add command -label Save -underline 0 -command { set choice 39 }" );
			cmd( "$w add command -label \"Run-time Plot\" -underline 0 -command { set choice 84 }" );
			cmd( "$w add command -label \"Debug and Watch\" -underline 0 -command { set choice 85 }" );
			cmd( "$w add command -label Initialize -underline 0 -command { set choice 49 }" );
			cmd( "$w add command -label Observe -underline 0 -command { set choice 42 }" );
			cmd( "$w add command -label Parallelize -underline 0 -command { set choice 86 }" );
			cmd( "$w add command -label \"Special Updating\" -underline 8 -command { set choice 97 }" );
			cmd( "$w add command -label \"Sensitivity Analysis\" -underline 1 -command { set choice 66 }" );
			cmd( "$w add command -label Unused -underline 1 -command { set choice 56 }" );

			cmd( "set w .m.run.rem" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( "$w add command -label Save -underline 0 -accelerator Ctrl+G -command { set choice 30 }" );
			cmd( "$w add command -label \"Run-time Plot\" -underline 0 -command { set choice 31 }" );
			cmd( "$w add command -label \"Debug and Watch\" -underline 0 -accelerator Ctrl+F -command { set choice 27 }" );
			cmd( "$w add command -label Parallelize -underline 0 -command { set choice 87 }" );

			cmd( "set w .m.help" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label Help -menu $w -underline 0" );
			cmd( "$w add command -label \"Help on Browser\" -underline 0 -accelerator F1 -command { LsdHelp browser.html }" );
			cmd( "$w add command -label \"LSD Quick Help\" -underline 4 -command { LsdHelp LSD_quickhelp.html }" );
			cmd( "$w add command -label \"LSD Documentation\" -underline 4 -command { LsdHelp LSD_documentation.html }" );
			cmd( "$w add separator" );
			cmd( "$w add command -label \"LMM Primer Tutorial\" -underline 4 -command { LsdHelp LMM_primer.html }" );
			cmd( "$w add command -label \"Using LSD Models Tutorial\" -underline 0 -command { LsdHelp model_using.html }" );
			cmd( "$w add command -label \"Writing LSD Models Tutorial\" -underline 0 -command { LsdHelp model_writing.html }" );
			cmd( "$w add separator" );
			cmd( "$w add command -label \"Model Report\" -underline 0 -command { set choice 44 }" );
			cmd( "$w add separator" );
			cmd( "$w add command -label \"About LSD...\" -underline 0 -command { LsdAbout {%s} {%s} }", _LSD_VERSION_, _LSD_DATE_  );

			// set shortcuts on open windows
			cmd( "bind . <F1> { LsdHelp browser.html }" );
			set_shortcuts( "." );
			set_shortcuts( ".log" );
		}

		// Button bar
		if ( ! exists_window( ".bbar" ) )
		{
			cmd( "ttk::frame .bbar" );

			cmd( "ttk::button .bbar.open -image openImg -style Toolbutton -command { set choice 17 }" );
			cmd( "ttk::button .bbar.reload -image reloadImg -style Toolbutton -command { set choice 38 }" );
			cmd( "ttk::button .bbar.save -image saveImg -style Toolbutton -command { set choice 18 }" );
			cmd( "ttk::button .bbar.struct -image structImg -style Toolbutton -command { set choice 70 }" );
			cmd( "ttk::button .bbar.find -image findImg -style Toolbutton -command { set choice 50 }" );
			cmd( "ttk::button .bbar.addvar -image addvarImg -style Toolbutton -command { set param 0; set choice 2 }" );
			cmd( "ttk::button .bbar.addpar -image addparImg -style Toolbutton -command { set param 1; set choice 2 }" );
			cmd( "ttk::button .bbar.addobj -image addobjImg -style Toolbutton -command { set choice 3 }" );
			cmd( "ttk::button .bbar.init -image initImg -style Toolbutton -command { set choice 21 }" );
			cmd( "ttk::button .bbar.number -image numberImg -style Toolbutton -command { set choice 19 }" );
			cmd( "ttk::button .bbar.set -image setImg -style Toolbutton -command { set choice 22 }" );
			cmd( "ttk::button .bbar.run -image runImg -style Toolbutton -command { set choice 1 }" );
			cmd( "ttk::button .bbar.data -image dataImg -style Toolbutton -command { set choice 34 }" );
			cmd( "ttk::button .bbar.result -image resultImg -style Toolbutton -command { set choice 26 }" );

			cmd( "tooltip::tooltip .bbar.open \"Load...\"" );
			cmd( "tooltip::tooltip .bbar.reload \"Reload\"" );
			cmd( "tooltip::tooltip .bbar.save \"Save\"" );
			cmd( "if { $strWindowOn } { \
					tooltip::tooltip .bbar.struct \"Hide Structure\" \
				} else { \
					tooltip::tooltip .bbar.struct \"Show Sstructure\" \
				}" );
			cmd( "tooltip::tooltip .bbar.find \"Find Element...\"" );
			cmd( "tooltip::tooltip .bbar.addvar \"Add Variable...\"" );
			cmd( "tooltip::tooltip .bbar.addpar \"Add Parameter...\"" );
			cmd( "tooltip::tooltip .bbar.addobj \"Add Object...\"" );
			cmd( "tooltip::tooltip .bbar.init \"Initial Values...\"" );
			cmd( "tooltip::tooltip .bbar.number \"Number of Objects...\"" );
			cmd( "tooltip::tooltip .bbar.set \"Settings...\"" );
			cmd( "tooltip::tooltip .bbar.run \"Run\"" );
			cmd( "tooltip::tooltip .bbar.data \"Data Browse...\"" );
			cmd( "tooltip::tooltip .bbar.result \"Analysis of Results...\"" );

			cmd( "pack .bbar.open .bbar.reload .bbar.save .bbar.struct .bbar.find .bbar.addvar .bbar.addpar .bbar.addobj .bbar.init .bbar.number .bbar.set .bbar.run .bbar.data .bbar.result -side left" );
			cmd( "pack .bbar -padx 3 -anchor w -fill x" );
		}

		cmd( "pack .l.v.lab" );
		cmd( "pack .l.v.c.v_scroll -side right -fill y" );
		cmd( "pack .l.v.c.var_name -fill both -expand yes" );
		cmd( "pack .l.v.c -fill both -expand yes" );

		cmd( "pack .l.s.lab" );
		cmd( "pack .l.s.c.v_scroll -side right -fill y" );
		cmd( "pack .l.s.c.son_name -fill both -expand yes" );
		cmd( "pack .l.s.c -fill both -expand yes" );

		cmd( "pack .l.p.up_name .l.p.tit" );
		cmd( "pack .l.p -pady 3 -fill x" );

		cmd( "pack .l.s .l.v -side left -fill both -expand yes" );

		cmd( "pack .l -fill both -expand yes" );
	}

	cmd( "settop . no { if { [ discard_change ] eq \"ok\" && [ abort_run_threads ] eq \"ok\" } { exit } } no yes" );

	if ( redrawStruc )
	{
		show_graph( r );
		redrawStruc = false;
	}

	main_cycle:

	// update element lists removing duplicates and sorting
	cmd( "if [ info exists modObj ] { set modObj [ lsort -dictionary -unique $modObj ] }" );
	cmd( "if [ info exists modElem ] { set modElem [ lsort -dictionary -unique $modElem ] }" );
	cmd( "if [ info exists modVar ] { set modVar [ lsort -dictionary -unique $modVar ] }" );
	cmd( "if [ info exists modPar ] { set modPar [ lsort -dictionary -unique $modPar ] }" );
	cmd( "if [ info exists modFun ] { set modFun [ lsort -dictionary -unique $modFun ] }" );

	// restore correct selection on list boxes
	cmd( "if { $listfocus == 1 } { \
			if { [ .l.v.c.var_name size ] == 0 || ! [ string is integer -strict $itemfocus ] } { \
				set itemfocus 0 \
			} { \
				if { $itemfocus >= [ .l.v.c.var_name size ] } { \
					set itemfocus [ expr { [ .l.v.c.var_name size ] - 1 } ] \
				} \
			}; \
			if { [ lindex [ .l.s.c.son_name yview ] 0 ] != $itemfirst } { \
				.l.v.c.var_name yview moveto $itemfirst \
			}; \
			if { [ .l.v.c.var_name curselection ] != $itemfocus } { \
				.l.v.c.var_name selection set $itemfocus; \
				if { $itemfocus < [ expr { [ lindex [ .l.v.c.var_name yview ] 0 ] * [ .l.v.c.var_name size ] } ] || $itemfocus >= [ expr { [ lindex [ .l.v.c.var_name yview ] 1 ] * [ .l.v.c.var_name size ] } ] } { \
					.l.v.c.var_name see $itemfocus \
				}; \
				set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ] \
			}; \
			if { [ .l.v.c.var_name index active ] != $itemfocus } { \
				selectinlist .l.v.c.var_name $itemfocus \
			}; \
			focus .l.v.c.var_name; \
		}" );
	cmd( "if { $listfocus == 2 } { \
			if { [ .l.s.c.son_name size ] == 0 || ! [ string is integer -strict $itemfocus ] } { \
				set itemfocus 0 \
			} { \
				if { $itemfocus >= [ .l.s.c.son_name size ] } { \
					set itemfocus [ expr { [ .l.s.c.son_name size ] - 1 } ] \
				} \
			}; \
			if { [ lindex [ .l.s.c.son_name yview ] 0 ] != $itemfirst } { \
				.l.s.c.son_name yview moveto $itemfirst \
			}; \
			if { [ .l.s.c.son_name curselection ] != $itemfocus } { \
				.l.s.c.son_name selection set $itemfocus; \
				if { $itemfocus < [ expr { [ lindex [ .l.s.c.son_name yview ] 0 ] * [ .l.s.c.son_name size ] } ] || $itemfocus >= [ expr { [ lindex [ .l.s.c.son_name yview ] 1 ] * [ .l.s.c.son_name size ] } ] } { \
					.l.s.c.son_name see $itemfocus \
				}; \
				set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ] \
			}; \
			if { [ .l.s.c.son_name index active ] != $itemfocus } { \
				selectinlist .l.s.c.son_name $itemfocus \
			}; \
			focus .l.s.c.son_name; \
		}" );

	cmd( "upd_menu_visib" );		// update active menu options
	cmd( "set useCurrObj yes" );	// flag to select among the current or the clicked object

	choice = choice_g = 0;
	idle_loop = true;

	// main command loop
	while ( ! choice && ! choice_g )
		Tcl_DoOneEvent( 0 );

	idle_loop = false;

	// coming from the structure window
	if ( choice_g )
	{
		choice = choice_g;
		choice_g = 0;
		res_g = exists_var( "res_g" ) ? get_str( "res_g" ) : NULL;
		cmd( "focus .l.v.c.var_name" );
	}

	// update focus memory
	cmd( "if { [ .l.v.c.var_name curselection ] != \"\" } { \
			set listfocus 1; \
			set itemfocus [ .l.v.c.var_name curselection ]; \
			set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ] \
		}" );
	cmd( "if { [ .l.s.c.son_name curselection ] != \"\" } { \
			set listfocus 2; \
			set itemfocus [ .l.s.c.son_name curselection ]; \
			set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ] \
		}" );

	// if simulation was started, check to see if operation is valid
	if ( running || actual_steps > 0 )
		// search the sorted list of choices that are bad with existing run data
		if ( bsearch( & choice, badChoices, NUM_BAD_CHOICES, sizeof ( int ), comp_ints ) != NULL )
		{
			if ( discard_change( true, false, "Invalid command after a simulation run." ) )	// for sure there are changes, just get the pop-up
			{
				if ( open_configuration( r, true ) )
					choice = - choice;		// signal the reload
				else
					choice = 20;			// reload failed, unload configuration
			}
			else
			{
				choice = 0;
				goto main_cycle;
			}
		}

	return choice;
}


/****************************************************
OPERATE
****************************************************/
object *operate( object *r )
{
	bool saveAs, delVar, renVar, table, subDir, overwDir;
	char observe, initial, deb_mode, *lab0;
	const char *lab1, *lab2, *lab3, *lab4;
	char lab[ MAX_BUFF_SIZE ], lab_old[ 2 * MAX_PATH_LENGTH ], ch[ 2 * MAX_LINE_SIZE ], ch1[ MAX_ELEM_LENGTH ], NOLHfile[ MAX_PATH_LENGTH ], out_file[ MAX_PATH_LENGTH ], out_dir[ MAX_PATH_LENGTH ], nw_exe[ MAX_PATH_LENGTH ], out_bat[ MAX_PATH_LENGTH ], win_dir[ MAX_PATH_LENGTH ], buf_descr[ MAX_BUFF_SIZE ];
	int i, j, k, sl, num, param, save, plot, nature, numlag, lag, fSeq, ffirst, fnext, sizMC, varSA, savei, debug, watch, watch_write, parallel, temp[ 11 ], done = 0;
	long nLinks, ptsSa, maxMC;
	double fracMC, fake = 0;
	FILE *f;
	bridge *cb;
	object *n, *cur, *cur1, *cur2;
	variable *cv, *cv1;
	result *rf;					// pointer for results files (may be zipped or not)
	sense *cs;
	description *cd;
	design *doe;
	vector < string > logs;
	struct stat stExe, stMod;

	if ( ! redrawReq )
		redrawRoot = false;		// assume no browser redraw
	else
	{
		redrawRoot = true;		// handle pending async redraw request
		redrawReq = false;
	}

	switch ( choice )
	{

	// Exit LSD
	case 11:

		if ( discard_change( ) && abort_run_threads( ) )
			myexit( 0 );

	break;


	// Add an element to the current or the pointed object (defined in tcl $vname)
	case 2:

		// check if current or pointed object and save current if needed
		lab1 = get_str( "useCurrObj" );
		if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
		{
			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			n = root->search( lab_old );		// set pointer to $vname
			if ( n == NULL )
				break;
			cur2 = r;
			r = n;
		}
		else
			cur2 = NULL;

		// read the lists of variables/functions, parameters and objects in model program
		// from disk, if needed, or just update the missing elements lists
		cmd( "if { [ llength $missVar ] == 0 || [ llength $missPar ] == 0 } { read_elem_file %s } { upd_miss_elem }", exec_path );

		Tcl_LinkVar( inter, "done", ( char * ) &done, TCL_LINK_INT );
		Tcl_LinkVar( inter, "num", ( char * ) &num, TCL_LINK_INT );

		param = get_int( "param" );
		cmd( "set num 0" );
		cmd( "set lab \"\"" );
		cmd( "set initValEn 0" );

		cmd( "set T .addelem" );
		cmd( "newtop $T \"Add Element\" { set done 2 }" );

		switch ( param )
		{
			case 0:								// variable
				cmd( "ttk::frame $T.l" );
				cmd( "ttk::label $T.l.l1 -text \"New variable in object:\"" );
				cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
				cmd( "pack $T.l.l1 $T.l.l2 -side left -padx 2" );

				cmd( "ttk::frame $T.f" );
				cmd( "ttk::label $T.f.lab_ent -text \"Variable name\"" );
				cmd( "ttk::label $T.f.lab_num -text \"Maximum lags\"" );
				cmd( "ttk::label $T.f.sp -width 5" );
				cmd( "ttk::combobox $T.f.ent_var -width 20 -textvariable lab -justify center -values $missVar" );
				cmd( "ttk::spinbox $T.f.ent_num -width 3 -from 0 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set num %%P; if { $num > 0 } { $T.b.x configure -state normal } { $T.b.x configure -state disabled }; return 1 } { %%W delete 0 end; %%W insert 0 $num; return 0 } } -command { if { [ $T.f.ent_num get ] > 0 } { $T.b.x configure -state normal } { $T.b.x configure -state disabled } } -invalidcommand { bell } -justify center" );
				cmd( "write_any $T.f.ent_num $num" );
				cmd( "pack $T.f.lab_ent $T.f.ent_var $T.f.sp $T.f.lab_num $T.f.ent_num -side left -padx 2" );

				cmd( "tooltip::tooltip $T.f.ent_num \"Maximum lag used in equations\"" );

				cmd( "bind $T.f.ent_var <KeyRelease> { \
						if { %%N < 256 } { \
							set b [ .addelem.f.ent_var index insert ]; \
							set s [ .addelem.f.ent_var get ]; \
							set f [ lsearch -glob $missVar $s* ]; \
							if { $f !=-1 } { \
								set d [ lindex $missVar $f ]; \
								.addelem.f.ent_var delete 0 end; \
								.addelem.f.ent_var insert 0 $d; \
								.addelem.f.ent_var index $b; \
								.addelem.f.ent_var selection range $b end \
							} \
						} \
					}" );
				cmd( "bind $T.f.ent_var <<ComboboxSelected>> { \
						set s [ .addelem.f.ent_var get ]; \
						.addelem.d.f.text insert end \"[ get_var_descr $s ]\"; \
					}" );
				cmd( "bind $T.f.ent_var <KeyPress-Return> { event generate .addelem.f.ent_var <<ComboboxSelected>>; if { [ .addelem.f.ent_num get ] > 0 } { focus $T.b.x } { focus $T.b.ok } }" );
				cmd( "bind $T.f.ent_num <KeyPress-Return> { if { [ .addelem.f.ent_num get ] > 0 } { focus $T.b.x } { focus $T.b.ok } }" );
				cmd( "set help menumodel.html#AddAVar");
				break;

			case 2:								// function
				cmd( "ttk::frame $T.l" );
				cmd( "ttk::label $T.l.l1 -text \"New function in object:\"" );
				cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
				cmd( "pack $T.l.l1 $T.l.l2 -side left -padx 2" );

				cmd( "ttk::frame $T.f" );
				cmd( "ttk::label $T.f.lab_ent -text \"Function name\"" );
				cmd( "ttk::combobox $T.f.ent_var -width 20 -textvariable lab -justify center -values $missVar" );
				cmd( "pack $T.f.lab_ent $T.f.ent_var -side left -padx 2" );
				cmd( "bind $T.f.ent_var <KeyRelease> { \
						if { %%N < 256 } { \
							set b [ .addelem.f.ent_var index insert ]; \
							set s [ .addelem.f.ent_var get ]; \
							set f [ lsearch -glob $missVar $s* ]; \
							if { $f !=-1 } { \
								set d [ lindex $missVar $f ]; \
								.addelem.f.ent_var delete 0 end; \
								.addelem.f.ent_var insert 0 $d; \
								.addelem.f.ent_var index $b; \
								.addelem.f.ent_var selection range $b end \
							} \
						} \
					}" );
				cmd( "bind $T.f.ent_var <<ComboboxSelected>> { \
						set s [ .addelem.f.ent_var get ]; \
						.addelem.d.f.text insert end \"[ get_var_descr $s ]\"; \
					}" );
				cmd( "set help menumodel.html");
				cmd( "bind $T.f.ent_var <KeyPress-Return> { event generate .addelem.f.ent_var <<ComboboxSelected>>; focus $T.b.ok }" );

				break;

			case 1:								// parameter
				cmd( "ttk::frame $T.l" );
				cmd( "ttk::label $T.l.l1 -text \"New parameter in object:\"" );
				cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
				cmd( "pack $T.l.l1 $T.l.l2 -side left -padx 2" );

				cmd( "ttk::frame $T.f" );
				cmd( "ttk::label $T.f.lab_ent -text \"Parameter name\"" );
				cmd( "ttk::combobox $T.f.ent_var -width 20 -textvariable lab -justify center -values $missPar" );
				cmd( "pack $T.f.lab_ent $T.f.ent_var -side left -padx 2" );
				cmd( "bind $T.f.ent_var <KeyRelease> { \
						if { %%N < 256 } { \
							set b [ .addelem.f.ent_var index insert ]; \
							set s [ .addelem.f.ent_var get ]; \
							set f [ lsearch -glob $missPar $s* ]; \
							if { $f !=-1 } { \
								set d [ lindex $missPar $f ]; \
								.addelem.f.ent_var delete 0 end; \
								.addelem.f.ent_var insert 0 $d; \
								.addelem.f.ent_var index $b; \
								.addelem.f.ent_var selection range $b end \
							} \
						} \
					}" );
				cmd( "bind $T.f.ent_var <KeyPress-Return> { focus $T.b.x }" );
				cmd( "set help menumodel.html#AddAPar");
				cmd( "set initValEn 1" );
				break;

			default:
				done = 2;
				goto err_newelem;
		}

		cmd( "set w $T.d" );
		cmd( "ttk::frame $w" );
		cmd( "ttk::label $w.lab -text \"Description\"" );
		cmd( "ttk::frame $w.f" );
		cmd( "ttk::scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
		cmd( "ttk::text $w.f.text -wrap word -width 60 -height 6 -yscrollcommand \"$w.f.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
		cmd( "pack $w.f.yscroll -side right -fill y" );
		cmd( "pack $w.f.text -expand yes -fill both" );
		cmd( "mouse_wheel $w.f.text" );
		cmd( "pack $w.lab $w.f" );

		cmd( "pack $T.l $T.f $T.d -pady 5" );

		cmd( "okXhelpcancel $T b \"Initial Values\" { set done 3 } { set done 1 } { LsdHelp $help } { set done 2 }" );

		cmd( "tooltip::tooltip $T.b.x \"Save and set initial value for element\"" );

		cmd( "if { ! $initValEn } { $T.b.x configure -state disabled }" );

		cmd( "showtop $T topleftW" );
		cmd( "focus $T.f.ent_var; $T.f.ent_var selection range 0 end" );

		here_newelem:

		if ( param == 0 )
			cmd( "write_any .addelem.f.ent_num $num" );

		while ( done == 0 )
			Tcl_DoOneEvent( 0 );

		if ( param == 0 )
			cmd( "set num [ .addelem.f.ent_num get ]" );

		initVal = ( done == 3 ) ? true : false;

		if ( done == 1 || done == 3 )
		{
			get_str( "lab", lab, MAX_ELEM_LENGTH );
			sl = strlen( lab );
			if ( sl != 0 )
			{
				for ( cur = r; cur->up != NULL; cur = cur->up );

				done = check_label( lab, cur );
				if ( done == 1 )
				{
					cmd( "ttk::messageBox -parent .addelem -title Error -icon error -type ok -message \"The name already exists in the model\" -detail \"Choose a different name and try again.\"" );
					cmd( "focus .addelem.f.ent_var; .addelem.f.ent_var selection range 0 end" );
					done = 0;
					goto here_newelem;
				}

				if ( done == 2 )
				{
					cmd( "ttk::messageBox -parent .addelem -title Error -icon error -type ok -message \"Invalid characters in name\" -detail \"Names must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces or other characters. Choose a different name and try again.\"" );
					cmd( "focus .addelem.f.ent_var; .addelem.f.ent_var selection range 0 end" );
					done = 0;
					goto here_newelem;
				}

				if ( done == 0 )
				{
					add_description( lab, param, eval_str( "[ .addelem.d.f.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

					if ( param == 0 )
						cmd( "lappend modVar %s", lab );

					if ( param == 1 )
						cmd( "lappend modPar %s", lab );

					if ( param == 2 )
						cmd( "lappend modFun %s", lab );

					cmd( "lappend modElem %s", lab );

					for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
					{
						cv = cur->add_empty_var( lab );
						if ( param != 0 )
							num = 0;
						cv->val = new double[ num + 1 ];
						cv->save = 0;
						cv->param = param;
						cv->num_lag = num;
						cv->deb_mode = 'n';
						if ( ( param == 0 && num == 0 ) || param == 2 )
							cv->data_loaded = '+';
						else
							cv->data_loaded = '-';

						for ( i = 0; i < num + 1; ++i )
							cv->val[ i ] = 0;
					}

					initParent = r;

					// update focus memory
					cmd( "set listfocus 1; set itemfocus [ .l.v.c.var_name index end ]" );
					struct_loaded = true;		// some model structure loaded
					unsaved_change( true );		// signal unsaved change
					redrawRoot = redrawStruc = true;	// force browser/structure redraw
				}
			}
			else
			{
				initVal = false;
				initParent = NULL;
			}
		}

		err_newelem:

		cmd( "destroytop .addelem" );

		if ( cur2 != NULL )						// restore original current object
			r = cur2;

		Tcl_UnlinkVar( inter, "done" );
		Tcl_UnlinkVar( inter, "num" );
		cmd( "unset done" );

		if ( initVal )
		{
			if ( param == 0 && num < 1 )
			{
				cmd( "ttk::messageBox -parent . -type ok -title Warning -icon warning -message \"Cannot set initial value\" -detail \"The variable '%s' was created with maximum lag equal to zero. No initial value is required.\"", lab );
				initVal = false;
				break;
			}

			cmd( "set vname %s", lab );
			next_lag = 0;						// lag to initialize
			choice = 77;						// change initial values for $vname
			return r;							// execute command
		}

	break;


	// Add a Descendent object to the current or the pointed object (defined in tcl $vname)
	// and assigns the number of its instances.
	case 3:

		// check if current or pointed object and save current if needed
		lab1 = get_str( "useCurrObj" );
		if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
		{
			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			n = root->search( lab_old );		// set pointer to $vname
			if ( n == NULL )
				break;
			cur2 = r;
			r = n;
		}
		else
			cur2 = NULL;

		// read the lists of variables/functions, parameters and objects in model program
		// from disk, if needed, or just update the missing elements lists
		cmd( "if { [ llength $missObj ] == 0 } { read_elem_file %s } { upd_miss_elem }", exec_path );

		Tcl_LinkVar( inter, "done", ( char * ) &done, TCL_LINK_INT );

		cmd( "set lab \"\"" );

		cmd( "set T .addobj" );
		cmd( "newtop $T \"Add Object\" { set done 2 }" );

		cmd( "ttk::frame $T.l" );
		cmd( "ttk::label $T.l.l1 -text \"New object descending from:\"" );
		cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
		cmd( "pack $T.l.l1 $T.l.l2 -side left -padx 2" );

		cmd( "ttk::frame $T.f" );
		cmd( "ttk::label $T.f.lab_ent -text \"Object name\"" );
		cmd( "ttk::combobox $T.f.ent_var -width 20 -textvariable lab -justify center -values $missObj" );
		cmd( "pack $T.f.lab_ent $T.f.ent_var -side left -padx 2" );
		cmd( "bind $T.f.ent_var <KeyPress-Return> {focus $T.b.ok}" );

		cmd( "set w $T.d" );
		cmd( "ttk::frame $w" );
		cmd( "ttk::label $w.lab -text \"Description\"" );
		cmd( "ttk::frame $w.f" );
		cmd( "ttk::scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
		cmd( "ttk::text $w.f.text -wrap word -width 60 -height 6 -yscrollcommand \"$w.f.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
		cmd( "pack $w.f.yscroll -side right -fill y" );
		cmd( "pack $w.f.text -expand yes -fill both" );
		cmd( "mouse_wheel $w.f.text" );
		cmd( "pack $w.lab $w.f" );

		cmd( "pack $T.l $T.f $w -pady 5" );
		cmd( "okhelpcancel $T b { set done 1 } { LsdHelp menumodel.html#AddADesc } { set done 2 }" );

		cmd( "showtop $T topleftW" );
		cmd( "focus $T.f.ent_var; $T.f.ent_var selection range 0 end" );

		here_newobject:

		while ( done == 0 )
			Tcl_DoOneEvent( 0 );

		if ( done == 1 )
		{
			get_str( "lab", lab, MAX_ELEM_LENGTH );
			if ( strlen( lab ) == 0 )
				goto here_endobject;

			for ( cur = r; cur->up != NULL; cur = cur->up );

			done = check_label( lab, cur ); // check that the label does not exist already
			if ( done == 1 )
			{
				cmd( "ttk::messageBox -parent .addobj -title Error -icon error -type ok -message \"Name already exists in the model\" -detail \"Choose a different name and try again.\"" );
				cmd( "focus .addobj.f.ent_var; .addobj.f.ent_var selection range 0 end" );
				done = 0;
				goto here_newobject;
			}

			if ( done == 2 )
			{
				cmd( "ttk::messageBox -parent .addobj -title Error -icon error -type ok -message \"Invalid characters in name\" -detail \"Names must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces or other characters. Choose a different name and try again.\"" );
				cmd( "focus .addobj.f.ent_var; .addobj.f.ent_var selection range 0 end" );
				done = 0;
				goto here_newobject;
			}

			r->add_obj( lab, 1, 1 );

			add_description( lab, 4, eval_str( "[ .addobj.d.f.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );
			cmd( "lappend modObj %s", lab );

			// update focus memory
			cmd( "set listfocus 2; set itemfocus [ .l.s.c.son_name index end ]; set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]" );
			struct_loaded = true;	// some model structure loaded
			unsaved_change( true );	// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw
		}

		here_endobject:

		cmd( "destroytop .addobj" );

		if ( cur2 != NULL )			// restore original current object
			r = cur2;

		Tcl_UnlinkVar( inter, "done" );
		cmd( "unset done" );

	break;


	// move object (defined in tcl $vname)
	case 32:

		// check if current or pointed object and save current if needed
		lab1 = get_str( "useCurrObj" );
		if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
		{
			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			n = root->search( lab_old );		// set pointer to $vname
			if ( n == NULL )
				break;
			cur2 = r;
			r = n;
		}
		else
			cur2 = NULL;

		if ( r->up == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Cannot move 'Root' object\" -detail \"Consider, if appropriate, moving its descendants, one at a time.\"" );
			goto endmove;
		}

		cmd( "set TT .objs" );
		cmd( "newtop $TT \"Move\" { set choice 2 }" );

		cmd( "ttk::frame $TT.l" );
		cmd( "ttk::label $TT.l.l -text \"Object:\"" );
		cmd( "ttk::label $TT.l.n -style hl.TLabel -text \"%s\"", lab_old );
		cmd( "pack $TT.l.l $TT.l.n -side left -padx 2" );

		cmd( "ttk::frame $TT.v" );
		cmd( "ttk::label $TT.v.l -text \"Move to\"" );

		cmd( "ttk::frame $TT.v.t" );
		cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
		cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
		cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
		cmd( "mouse_wheel $TT.v.t.lb" );
		insert_object( "$TT.v.t.lb", root, false, r );
		cmd( "pack $TT.v.l $TT.v.t" );

		cmd( "pack $TT.l $TT.v -padx 5 -pady 5" );

		cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

		cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0; break }" );
		cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end; break }" );
		cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

		cmd( "showtop $TT" );
		cmd( "$TT.v.t.lb selection set 0" );
		cmd( "focus $TT.v.t.lb" );

		choice = 0;

		cmd( "if { [ $TT.v.t.lb size ] == 0 } { ttk::messageBox -parent . -type ok -title Error -icon error -message \"Cannot move single 'Root' descendant\" -detail \"Consider, if appropriate, creating additional objects under 'Root' before moving this one.\"; set choice 2 }" );

		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "if { $choice != 2 } { set movelabel [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ] }" );
		cmd( "destroytop .objs" );

		if ( choice == 2 )
			goto endmove;

		lab1 = get_str( "movelabel" );
		if ( lab1 == NULL || strlen( lab1 ) == 0 )
			goto endmove;

		i = hyper_count( r->up->label );
		j = hyper_count( lab1 );

		if ( i != j )
		{
			cmd( "if { %d < %d } { set msg \"the last instance of '$vname' being replicated %d times\" } { set msg \"the last %d unmatched instances of '$vname' being deleted\" }", i, j, j - i, i - j );
			cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -title Warning -icon warning -message \"Different number of parents' instances\" -detail \"The original parent object '%s' has a different number of instances (%d) than the desired new parent '%s' (%d). Copying object '$vname' to parent '%s' will result in $msg.\" ]", r->up->label, i, lab1, j, lab1 );
			cmd( "switch $answer { yes { set choice 1 } no { set choice 2 } }" );

			if( choice == 2 )
				goto endmove;
		}

		move_obj( lab_old, lab1 );

		unsaved_change( true );		// signal unsaved change
		redrawRoot = redrawStruc = true;	// force browser/structure redraw

		endmove:

		if ( cur2 != NULL )					// restore original current object
			r = cur2;

	break;


	// Move browser to show one of the descendant object (defined in tcl $vname)
	case 4:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) || ! strcmp( lab1, "(none)" ) )
			break;

		sscanf( lab1, "%99s", lab_old );

		n = root->search( lab_old );
		if ( n == NULL )
			break;

		cmd( "set listfocus 2; set itemfocus 0" );

		choice = 0;
		redrawRoot = redrawStruc = true;	// force browser/structure redraw
		return n;


	// Move browser to show the parent object
	case 5:

		if ( r->up == NULL )
			return r;

		for ( i = 0, cb = r->up->b; cb->head != r; cb = cb->next, ++i );

		cmd( "set listfocus 2; set itemfocus %d", r->up->up == NULL ? i : i + 1 );

		choice = 0;
		redrawRoot = redrawStruc = true;	// force browser/structure redraw
		return r->up;


	// Edit current Object and give the option to disable the computation (defined in tcl $vname)
	case 6:

		cmd( "if $useCurrObj { set lab %s } { if [ info exists vname ] { set lab $vname } { set lab \"\" } }; set useCurrObj yes ", r->label  );
		lab1 = get_str( "lab" );

		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );

		// check if current or pointed object and save current if needed
		if ( strcmp( r->label, lab_old ) )	// check if not current variable
		{
			n = root->search( lab_old );	// set pointer to $vname
			if ( n == NULL )
				break;
			cur2 = r;
			r = n;
		}
		else
			cur2 = NULL;

		if ( ! strcmp( r->label, "Root" ) )	// cannot change Root
		{
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Cannot change Root\" -detail \"Please select an existing object or insert a new one before using this option.\"" );
			break;
		}

		cd = search_description( lab_old );
		skip_next_obj( r, &num );

		cmd( "set to_compute %d", r->to_compute ? 1 : 0 );

		cmd( "set T .objprop" );
		cmd( "newtop $T \"Change Object\" { set choice 2 }" );

		cmd( "ttk::frame $T.h" );

		cmd( "ttk::frame $T.h.o" );
		cmd( "ttk::label $T.h.o.lab -text \"Object:\"" );
		cmd( "ttk::label $T.h.o.ent -style hl.TLabel -text $lab" );
		cmd( "pack $T.h.o.lab $T.h.o.ent -side left -padx 2" );

		cmd( "ttk::frame $T.h.i" );
		cmd( "ttk::label $T.h.i.lab -text \"Number of instances:\"" );
		cmd( "ttk::label $T.h.i.ent -style hl.TLabel -text %d", num );
		cmd( "pack $T.h.i.lab $T.h.i.ent -side left -padx 2" );

		cmd( "pack $T.h.o $T.h.i" );

		cmd( "ttk::frame $T.b0" );
		cmd( "ttk::button $T.b0.prop -width $butWid -text Rename -command { set useCurrObj yes; set choice 83 } -underline 0" );
		cmd( "ttk::button $T.b0.num -width $butWid -text Number -command { set useCurrObj yes; set choice 33 } -underline 0" );
		cmd( "ttk::button $T.b0.mov -width $butWid -text Move -command { set useCurrObj yes; set choice 32 } -underline 0" );
		cmd( "ttk::button $T.b0.del -width $butWid -text Delete -command { set choice 74 } -underline 0" );
		cmd( "pack $T.b0.prop $T.b0.num $T.b0.mov $T.b0.del -padx $butSpc -side left" );

		cmd( "tooltip::tooltip $T.b0.prop \"Change name\"" );
		cmd( "tooltip::tooltip $T.b0.num \"Change number of instances (copies)\"" );
		cmd( "tooltip::tooltip $T.b0.mov \"Move to another parent object\"" );
		cmd( "tooltip::tooltip $T.b0.del \"Remove object\"" );

		cmd( "ttk::frame $T.b1" );
		cmd( "ttk::checkbutton $T.b1.com -text \"Compute: force the computation of the variables in this object\" -variable to_compute -underline 1" );
		cmd( "pack $T.b1.com" );

		cmd( "set w $T.desc" );

		cmd( "ttk::frame $w" );
		cmd( "ttk::label $w.int -text Description -anchor center" );
		cmd( "ttk::frame $w.f" );
		cmd( "ttk::scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
		cmd( "ttk::text $w.f.text -wrap word -width 60 -height 10 -yscrollcommand \"$w.f.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
		cmd( "pack $w.f.yscroll -side right -fill y" );
		cmd( "pack $w.f.text -anchor w -expand yes -fill both" );
		cmd( "mouse_wheel $w.f.text" );

		cmd( "pack $w.int $w.f -fill x -expand yes" );

		cmd( "pack $T.h $T.b0 $T.b1 $w -pady 5" );

		cmd( "bind $T <Control-r> \"$T.b0.prop invoke\"; bind $T <Control-R> \"$T.b0.prop invoke\"" );
		cmd( "bind $T <Control-n> \"$T.b0.num invoke\"; bind $T <Control-N> \"$T.b0.num invoke\"" );
		cmd( "bind $T <Control-m> \"$T.b0.mov invoke\"; bind $T <Control-M> \"$T.b0.mov invoke\"" );
		cmd( "bind $T <Control-d> \"$T.b0.del invoke\"; bind $T <Control-D> \"$T.b0.del invoke\"" );
		cmd( "bind $T <Control-o> \"$T.b1.com invoke\"; bind $T <Control-O> \"$T.b1.com invoke\"" );

		cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menumodel.html#ChangeObjName } { set choice 2 }" );

		cmd( "showtop $T topleftW" );
		cmd( "mousewarpto $T.b.ok" );

		cmd( "$w.f.text insert end \"%s\"", strtcl( buf_descr, cd->text, MAX_BUFF_SIZE ) );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		done = choice;

		if ( choice != 2 )
		{
			unsaved_change( true );		// signal unsaved change

			// save description changes
			change_description( lab_old, NULL, -1, eval_str( "[ .objprop.desc.f.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

			cmd( "set choice $to_compute" );

			if ( choice != r->to_compute )
			{
				cur = blueprint->search( r->label );
				if ( cur != NULL )
					cur->to_compute = choice;
				for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
					cur->to_compute = choice;
			}

			// control for elements to save in objects to be not computed
			check_save = true;
			if ( choice == 0 )
				control_to_compute( r, r->label );
		}

		cmd( "destroytop .objprop" );

		redrawRoot = redrawStruc = true;			// force browser/structure redraw

		// dispatch chosen option
		if ( done > 2 )
		{
			cmd( "set vname $lab" );
			cmd( "set useCurrObj no" );
			choice = done;
		}
		else
			choice = 0;

		// avoid entering into descendant
		if ( cur2 != NULL )
			return cur2;
		else
			return r;

	break;


	// Delete object (defined in tcl $vname)
	case 74:
	// Rename object (defined in tcl $vname)
	case 83:

		nature = choice;

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );

		cur = root->search( lab_old );	// get pointer to vname
		if ( cur == NULL )
			break;

		if ( nature == 74 )		// delete
		{
			cmd( "set answer [ ttk::messageBox -parent . -title Confirmation -icon question -type yesno -default yes -message \"Delete object?\" -detail \"Press 'Yes' to confirm deleting '$vname'\n\nNote that all descendants will be also deleted!\" ]" );
			cmd( "switch $answer { yes { set choice 1 } no { set choice 2 } }" );
			if ( choice == 2 )
				break;

			r = cur->up;
			wipe_out( cur );
		}
		else					// rename
		{
			cmd( "set T .chgnam" );
			cmd( "newtop $T \"Rename\" { set choice 2 }" );

			cmd( "ttk::frame $T.l" );
			cmd( "ttk::label $T.l.l -text \"Object:\"" );
			cmd( "ttk::label $T.l.n -style hl.TLabel -text \"$vname\"" );
			cmd( "pack $T.l.l $T.l.n -side left -padx 2" );

			cmd( "ttk::frame $T.e" );
			cmd( "ttk::label $T.e.l -text \"New name\"" );
			cmd( "ttk::entry $T.e.e -width 20 -textvariable vname -justify center" );
			cmd( "pack $T.e.l $T.e.e -side left -padx 2" );

			cmd( "pack $T.l $T.e -padx 5 -pady 5" );

			cmd( "okcancel $T b { set choice 1 } { set choice 2 }" );

			cmd( "bind $T.e.e <KeyPress-Return> { set choice 1 }" );

			cmd( "showtop $T" );
			cmd( "focus $T.e.e" );
			cmd( "$T.e.e selection range 0 end" );

			here_newname:

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			if ( choice == 1 )
			{
				lab1 = get_str( "vname" );
				if ( lab1 == NULL || ! strcmp( lab1, "" ) )
					break;
				sscanf( lab1, "%99s", lab );

				if ( strcmp( lab, r->label ) )
				{
					for ( cur1 = r; cur1->up != NULL; cur1 = cur1->up );

					done = check_label( lab, cur1 );
					if ( done == 1 )
					{
						cmd( "ttk::messageBox -parent .chgnam -title Error -icon error -type ok -message \"The name already exists in the model\" -detail \"Choose a different name and try again.\"" );
						cmd( "focus .chgnam.e.e; .chgnam.e.e selection range 0 end" );
						goto here_newname;
					}

					if ( done == 2 )
					{
						cmd( "ttk::messageBox -parent .chgnam -title Error -icon error -type ok -message \"Invalid characters in name\" -detail \"Names must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
						cmd( "focus .chgnam.e.e; .chgnam.e.e selection range 0 end" );
						goto here_newname;
					}

					// update element list
					cmd( "if [ info exists modObj ] { set pos [ lsearch -exact $modObj %s ]; if { $pos >= 0 } { set modObj [ lreplace $modObj $pos $pos ] } }", cur->label	);
					cmd( "lappend modObj %s", lab );

					change_description( cur->label, lab );
					cur->chg_lab( lab );
				}
				else
					break;
			}

			cmd( "destroytop .chgnam" );
		}

		if ( root->v == NULL && root->b == NULL )	// if last object
		{
			unsaved_change( false );				// no unsaved change
			struct_loaded = false;					// no config loaded
		}
		else
			unsaved_change( true );					// signal unsaved change

		redrawRoot = redrawStruc = true;			// force browser/structure redraw

	break;


	// Edit variable (defined in tcl $vname) and set debug/saving/plot flags
	case 7:

		redrawRoot = redrawStruc = true;	// force browser/structure redraw

		cmd( "if { ! [ catch { set vname [ .l.v.c.var_name get [ .l.v.c.var_name curselection ] ] } ] && ! [ string equal $vname \"\" ] } { set choice 1 } { set choice 0 }" );
		if ( choice == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No element selected\" -detail \"Please select an element (variable, parameter) before using this option.\"" );
			break;
		}

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) || ! strcmp( lab1, "(none)" ) )
			break;

		sscanf( lab1, "%99s", lab_old );
		cv = r->search_var( NULL, lab_old );
		cd = search_description( lab_old );

		Tcl_LinkVar( inter, "done", ( char * ) &done, TCL_LINK_INT );
		Tcl_LinkVar( inter, "save", ( char * ) &save, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "savei", ( char * ) &savei, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "plot", ( char * ) &plot, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "debug", ( char * ) &debug, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "watch", ( char * ) &watch, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "watch_write", ( char * ) &watch_write, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "parallel", ( char * ) &parallel, TCL_LINK_BOOLEAN );

		save = cv->save;
		savei = cv->savei;
		plot = cv->plot;
		debug = ( cv->deb_mode == 'd' || cv->deb_mode == 'W' || cv->deb_mode == 'R' ) ? 1 : 0;
		watch = ( cv->deb_mode == 'w' || cv->deb_mode == 'W' || cv->deb_mode == 'r' || cv->deb_mode == 'R' ) ? 1 : 0;
		watch_write = ( cv->deb_mode == 'r' || cv->deb_mode == 'R' ) ? 1 : 0;
		parallel = cv->parallel;

		cmd( "set observe %d", cd->observe == 'y' ? 1 : 0 );
		cmd( "set initial %d", cd->initial == 'y' ? 1 : 0 );
		cmd( "set vname %s", lab_old );

		cmd( "set T .chgelem" );
		cmd( "newtop $T \"Change Element\" { set done 2 }" );

		cmd( "ttk::frame $T.h" );

		cmd( "ttk::frame $T.h.l" );

		if ( cv->param == 0 )
			cmd( "ttk::label $T.h.l.lab_ent -text \"Variable:\"" );
		if ( cv->param == 1 )
			cmd( "ttk::label $T.h.l.lab_ent -text \"Parameter:\"" );
		if ( cv->param == 2 )
			cmd( "ttk::label $T.h.l.lab_ent -text \"Function:\"" );

		cmd( "ttk::label $T.h.l.ent_var -style hl.TLabel -text $vname" );
		cmd( "pack $T.h.l.lab_ent $T.h.l.ent_var -side left -padx 2" );

		cmd( "ttk::frame $T.h.o" );
		cmd( "ttk::label $T.h.o.l -text \"In object:\"" );
		cmd( "ttk::label $T.h.o.obj -style hl.TLabel -text \"%s\"", cv->up->label );
		cmd( "pack $T.h.o.l $T.h.o.obj -side left -padx 2" );

		cmd( "pack $T.h.l $T.h.o" );

		if ( cv->num_lag > 0 || cv->param == 1 )
		{
			cmd( "ttk::frame $T.h.i" );
			cmd( "ttk::label $T.h.i.l -text \"Initial value%s%s:\"", cv->num_lag > 1 ? "s" : "", cv->up->next == NULL ? "" : " (first instance)" );

			if ( cv->data_loaded != '-' )
			{
				strcpy ( buf_descr, "" );

				j = ( cv->param == 1 ) ? 1 : min( cv->num_lag, 4 );
				for ( i = 0; i < j; ++i )
				{
					cmd( "ttk::frame $T.h.i.v%d", i );
					cmd( "ttk::label $T.h.i.v%d.val -style hl.TLabel -text \"%g\"", i, cv->val[ i ] );

					if ( j > 1 )
					{
						cmd( "ttk::label $T.h.i.v%d.lag -text \"(%d)\"", i, i + 1 );
						cmd( "pack $T.h.i.v%d.val $T.h.i.v%d.lag -side left", i, i );
					}
					else
						cmd( "pack $T.h.i.v%d.val", i );

					snprintf( lab, MAX_ELEM_LENGTH, " $T.h.i.v%d", i );
					strcatn( buf_descr, lab, MAX_BUFF_SIZE );
				}

				cmd( "pack $T.h.i.l %s -side left -padx 1", buf_descr );
			}
			else
			{
				cmd( "ttk::label $T.h.i.val -style hl.TLabel -text \"(uninitialized)\"" );
				cmd( "pack $T.h.i.l $T.h.i.val -side left -padx 2" );
			}

			cmd( "pack $T.h.i" );
		}

		if ( cv->param == 0 && ( cv->delay > 0 || cv->delay_range > 0 || cv->period > 1 || cv->period_range > 1 ) )
		{
			cmd( "ttk::frame $T.h.u" );

			if ( cv->delay > 0 )
			{
				cmd( "ttk::frame $T.h.u.d" );
				cmd( "ttk::label $T.h.u.d.l -text \"Initial updating delay:\"" );
				cmd( "ttk::label $T.h.u.d.v -style hl.TLabel -text \"%d\"", cv->delay );
				cmd( "pack $T.h.u.d.l $T.h.u.d.v -side left -padx 2" );
				cmd( "pack $T.h.u.d" );
			}

			if ( cv->delay_range > 0 )
			{
				cmd( "ttk::frame $T.h.u.dr" );
				cmd( "ttk::label $T.h.u.dr.l -text \"Random updating delay range:\"" );
				cmd( "ttk::label $T.h.u.dr.v -style hl.TLabel -text \"%d\"", cv->delay_range );
				cmd( "pack $T.h.u.dr.l $T.h.u.dr.v -side left -padx 2" );
				cmd( "pack $T.h.u.dr" );
			}

			if ( cv->period > 1 )
			{
				cmd( "ttk::frame $T.h.u.p" );
				cmd( "ttk::label $T.h.u.p.l -text \"Updating period:\"" );
				cmd( "ttk::label $T.h.u.p.v -style hl.TLabel -text \"%d\"", cv->period );
				cmd( "pack $T.h.u.p.l $T.h.u.p.v -side left -padx 2" );
				cmd( "pack $T.h.u.p" );
			}

			if ( cv->period_range > 1 )
			{
				cmd( "ttk::frame $T.h.u.pr" );
				cmd( "ttk::label $T.h.u.pr.l -text \"Random updating period range:\"" );
				cmd( "ttk::label $T.h.u.pr.v -style hl.TLabel -text \"%d\"", cv->period_range );
				cmd( "pack $T.h.u.pr.l $T.h.u.pr.v -side left -padx 2" );
				cmd( "pack $T.h.u.pr" );
			}

			cmd( "pack $T.h.u" );
		}

		cmd( "ttk::frame $T.b0" );
		cmd( "ttk::button $T.b0.prop -width $butWid -text Properties -command { set done 5 } -underline 1" );
		cmd( "ttk::button $T.b0.upd -width $butWid -text Updating -command { set done 14 } -underline 7" );
		cmd( "ttk::button $T.b0.mov -width $butWid -text Move -command { set done 13 } -underline 0" );
		cmd( "ttk::button $T.b0.del -width $butWid -text Delete -command { set done 10 } -underline 2" );

		if ( cv->param == 0 )
		{
			cmd( "bind $T <Control-g> \"$T.b0.upd invoke\"; bind $T <Control-G> \"$T.b0.upd invoke\"" );

			cmd( "pack $T.b0.prop $T.b0.upd $T.b0.mov $T.b0.del -padx $butSpc -side left" );
			cmd( "tooltip::tooltip $T.b0.upd \"Define special update timing\"" );
		}
		else
			cmd( "pack $T.b0.prop $T.b0.mov $T.b0.del -padx $butSpc -side left" );

		cmd( "tooltip::tooltip $T.b0.prop \"Change name, type or lags\"" );
		cmd( "tooltip::tooltip $T.b0.mov \"Move to another object\"" );
		cmd( "tooltip::tooltip $T.b0.del \"Remove element\"" );

		cmd( "ttk::frame $T.b1" );

		cmd( "ttk::frame $T.b1.sav" );
		cmd( "ttk::checkbutton $T.b1.sav.n -text \"Save: save the series for analysis	   \" -variable save -underline 0 -command { \
				if { $save } { \
					.chgelem.b1.sav.i configure -state normal \
				} else { \
					set savei 0; \
					.chgelem.b1.sav.i configure -state disabled \
				} \
			}" );
		cmd( "ttk::checkbutton $T.b1.sav.i -text \"Save in separate files\" -variable savei -underline 17" );
		cmd( "if { ! $save } { \
				set savei 0; \
				.chgelem.b1.sav.i configure -state disabled \
			}" );
		cmd( "pack $T.b1.sav.n $T.b1.sav.i -side left -anchor w" );

		cmd( "ttk::checkbutton $T.b1.plt -text \"Run-time plot: observe the series during the simulation execution\" -variable plot -underline 9" );
		cmd( "ttk::checkbutton $T.b1.deb -text \"Debug: interrupt after variable is updated\" -variable debug -underline 0" );

		cmd( "ttk::frame $T.b1.watch" );
		cmd( "ttk::checkbutton $T.b1.watch.n -text \"Watch: interrupt when variable is accessed	   \" -variable watch -underline 0 -command { \
				if { $watch } { \
					.chgelem.b1.watch.i configure -state normal \
				} else { \
					set watch_write 0; \
					.chgelem.b1.watch.i configure -state disabled \
				} \
			}" );
		cmd( "ttk::checkbutton $T.b1.watch.i -text \"Watch only writes\" -variable watch_write -underline 4" );
		cmd( "if { ! $watch } { \
				set watch_write 0; \
				.chgelem.b1.watch.i configure -state disabled \
			}" );
		cmd( "pack $T.b1.watch.n $T.b1.watch.i -side left -anchor w" );

		cmd( "ttk::checkbutton $T.b1.par -text \"Parallel: allow multi-object parallel updating for this equation\" -variable parallel -underline 0" );

		switch ( cv->param )
		{
			case 1:
				cmd( "pack $T.b1.sav $T.b1.plt $T.b1.watch -anchor w" );
				break;
			case 2:
				cmd( "pack $T.b1.sav $T.b1.plt $T.b1.deb -anchor w" );
				cmd( "bind $T <Control-d> \"$T.b1.deb invoke\"; bind $T <Control-D> \"$T.b1.deb invoke\"" );
				break;
			case 0:
				cmd( "pack $T.b1.sav $T.b1.plt $T.b1.deb $T.b1.watch $T.b1.par -anchor w" );
				cmd( "bind $T <Control-d> \"$T.b1.deb invoke\"; bind $T <Control-D> \"$T.b1.deb invoke\"" );
				cmd( "bind $T <Control-p> \"$T.b1.par invoke\"; bind $T <Control-P> \"$T.b1.par invoke\"" );
		}

		cmd( "pack $T.h $T.b0 $T.b1 -pady 5" );

		cmd( "set Td $T.desc" );
		cmd( "ttk::frame $Td" );

		cmd( "ttk::frame $Td.opt" );
		cmd( "ttk::label $Td.opt.l -text \"Include in documentation to be\"" );
		cmd( "ttk::checkbutton $Td.opt.ini -text \"Initialized\" -variable initial -underline 0" );
		cmd( "ttk::checkbutton $Td.opt.obs -text \"Observed\" -variable observe -underline 0" );

		if ( cv->param == 1 || cv->num_lag > 0 )
		{
			cmd( "pack $Td.opt.l $Td.opt.obs $Td.opt.ini -side left" );
			cmd( "bind $T <Control-i> \"$Td.opt.ini invoke\"; bind $T <Control-I> \"$Td.opt.ini invoke\"" );
		}
		else
			cmd( "pack $Td.opt.l $Td.opt.obs -side left" );

		cmd( "ttk::frame $Td.f" );
		cmd( "ttk::label $Td.f.int -text \"Description\"" );

		cmd( "ttk::frame $Td.f.desc" );
		cmd( "ttk::scrollbar $Td.f.desc.yscroll -command \"$Td.f.desc.text yview\"" );
		cmd( "ttk::text $Td.f.desc.text -wrap word -width 60 -height 8 -yscrollcommand \"$Td.f.desc.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
		cmd( "pack $Td.f.desc.yscroll -side right -fill y" );
		cmd( "pack $Td.f.desc.text -anchor w -expand yes -fill both" );
		cmd( "mouse_wheel $Td.f.desc.text" );

		cmd( "pack $Td.f.int $Td.f.desc" );

		cmd( "ttk::frame $Td.b" );
		cmd( "ttk::button $Td.b.eq -width [ expr { $butWid + 2 } ] -text \"Equation\" -command { set done 3 } -underline 1" );
		cmd( "ttk::button $Td.b.auto_doc -width [ expr { $butWid + 2 } ] -text \"Auto Desc.\" -command { set done 9 } -underline 0" );
		cmd( "ttk::button $Td.b.us -width [ expr { $butWid + 2 } ] -text \"Using Elem.\" -command { set done 4 } -underline 0" );
		cmd( "ttk::button $Td.b.using -width [ expr { $butWid + 2 } ] -text \"Elem. Used\" -command { set done	7} -underline 0" );

		if ( ! strcmp( cd->type, "Parameter" ) )
			cmd( "pack $Td.b.auto_doc $Td.b.us -padx $butSpc -side left" );
		else
		{
			cmd( "pack $Td.b.eq $Td.b.auto_doc $Td.b.us $Td.b.using -padx $butSpc -side left" );
			cmd( "bind $T <Control-q> \"$Td.b.eq invoke\"; bind $T <Control-Q> \"$Td.b.eq invoke\"" );
			cmd( "bind $T <Control-e> \"$Td.b.using invoke\"; bind $T <Control-E> \"$Td.b.using invoke\"" );
		}

		cmd( "tooltip::tooltip $Td.b.eq \"Show variable's equation code\"" );
		cmd( "tooltip::tooltip $Td.b.auto_doc \"Get description from equation file\"" );
		cmd( "tooltip::tooltip $Td.b.us \"List all variables using this element\"" );
		cmd( "tooltip::tooltip $Td.b.using \"List all variables and parameters used\"" );

		if ( cv->param == 1 || cv->num_lag > 0 )
		{
			cmd( "ttk::frame $Td.i" );
			cmd( "ttk::label $Td.i.int -text \"Initial values\"" );

			cmd( "ttk::frame $Td.i.desc" );
			cmd( "ttk::scrollbar $Td.i.desc.yscroll -command \"$Td.i.desc.text yview\"" );
			cmd( "ttk::text $Td.i.desc.text -wrap word -width 60 -height 3 -yscrollcommand \"$Td.i.desc.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
			cmd( "pack $Td.i.desc.yscroll -side right -fill y" );
			cmd( "pack $Td.i.desc.text -anchor w -expand yes -fill both" );
			cmd( "mouse_wheel $Td.i.desc.text" );

			cmd( "pack $Td.i.int $Td.i.desc" );

			cmd( "ttk::frame $Td.b2" );
			cmd( "ttk::button $Td.b2.setall -width [ expr { $butWid + 2 } ] -text \"Initial Values\" -command { set done 11 } -underline 1" );
			cmd( "ttk::button $Td.b2.sens -width [ expr { $butWid + 2 } ] -text \"Sensitivity\" -command { set done 12 } -underline 5" );
			cmd( "pack $Td.b2.setall $Td.b2.sens -padx $butSpc -side left" );

			cmd( "pack $Td.opt $Td.f $Td.b $Td.i $Td.b2 -pady 5" );

			cmd( "tooltip::tooltip $Td.b2.setall \"Set initial value(s) of this element\"" );
			cmd( "tooltip::tooltip $Td.b2.sens \"Set sensitivity analysis values for this element \"" );

			cmd( "bind $T <Control-n> \"$Td.b2.setall invoke\"; bind $T <Control-N> \"$Td.b2.setall invoke\"" );
			cmd( "bind $T <Control-t> \"$Td.b2.sens invoke\"; bind $T <Control-T> \"$Td.b2.sens invoke\"" );

		}
		else
			cmd( "pack $Td.opt $Td.f $Td.b -pady 5" );

		cmd( "pack $Td -pady 5" );

		cmd( "okhelpcancel $T b { set done 1 } { LsdHelp browser.html#changeelement } { set done 2 }" );

		cmd( "bind $T <Control-r> \"$T.b0.prop invoke\"; bind $T <Control-R> \"$T.b0.prop invoke\"" );
		cmd( "bind $T <Control-m> \"$T.b0.mov invoke\"; bind $T <Control-M> \"$T.b0.mov invoke\"" );
		cmd( "bind $T <Control-l> \"$T.b0.del invoke\"; bind $T <Control-L> \"$T.b0.del invoke\"" );
		cmd( "bind $T <Control-s> \"$T.b1.sav.n invoke\"; bind $T <Control-S> \"$T.b1.sav.n invoke\"" );
		cmd( "bind $T <Control-f> \"$T.b1.sav.i invoke\"; bind $T <Control-F> \"$T.b1.sav.i invoke\"" );
		cmd( "bind $T <Control-p> \"$T.b1.plt invoke\"; bind $T <Control-P> \"$T.b1.plt invoke\"" );
		cmd( "bind $T <Control-w> \"$T.b1.watch.n invoke\"; bind $T <Control-W> \"$T.b1.watch.n invoke\"" );
		cmd( "bind $T <Control-h> \"$T.b1.watch.i invoke\"; bind $T <Control-H> \"$T.b1.watch.i invoke\"" );
		cmd( "bind $T <Control-o> \"$Td.opt.obs invoke\"; bind $T <Control-O> \"$Td.opt.obs invoke\"" );
		cmd( "bind $T <Control-a> \"$Td.b.auto_doc invoke\"; bind $T <Control-A> \"$Td.b.auto_doc invoke\"" );
		cmd( "bind $T <Control-u> \"$Td.b.us invoke\"; bind $T <Control-U> \"$Td.b.us invoke\"" );

		cmd( "showtop $T topleftW" );
		cmd( "mousewarpto $T.b.ok" );

		cmd( "$Td.f.desc.text insert end \"%s\"", strtcl( buf_descr, cd->text, MAX_BUFF_SIZE ) );

		if ( cv->param == 1 || cv->num_lag > 0 )
			cmd( "$Td.i.desc.text insert end \"%s\"", strtcl( buf_descr, cd->init, MAX_BUFF_SIZE ) );

		cycle_var:

		done = 0;
		while ( done == 0 )
			Tcl_DoOneEvent( 0 );

		if ( done == 3 )
			show_eq( lab_old, ".chgelem" );

		if ( done == 4 )
			scan_used_lab( lab_old, ".chgelem" );

		if ( done == 7 )
			scan_using_lab( lab_old, ".chgelem" );

		choice = 0;

		if ( done == 9 )
		{
			change_description( lab_old, NULL, -1, eval_str( "[ .chgelem.desc.f.desc.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

			auto_document( lab_old, "ALL", true );
			cmd( ".chgelem.desc.f.desc.text delete 1.0 end" );
			cmd( ".chgelem.desc.f.desc.text insert end \"%s\"", strtcl( buf_descr, cd->text, MAX_BUFF_SIZE ) );

			unsaved_change( true );		// signal unsaved change
		}

		if ( done == 7 || done == 4 || done == 3 || done == 9 )
		  goto cycle_var;

		if ( done == 2 || done == 8 )	// esc/cancel
		{
			redrawRoot = redrawStruc = false;	// no redraw necessary
			goto here_endelem;
		}
		else
		{
			cmd( "set choice $observe" );
			choice == 1 ? observe = 'y' : observe = 'n';
			cmd( "set choice $initial" );
			choice == 1 ? initial = 'y' : initial = 'n';
			cd->initial = initial;
			cd->observe = observe;

			if ( debug )
			{
				if ( watch_write )
					deb_mode = 'R';
				else
					if ( watch )
						deb_mode = 'W';
					else
						deb_mode = 'd';
			}
			else
			{
				if ( watch_write )
					deb_mode = 'r';
				else
					if ( watch )
						deb_mode = 'w';
					else
						deb_mode = 'n';
			}

			for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
			{
			   cv = cur->search_var( NULL, lab_old );
			   cv->save = save;
			   cv->savei = savei;
			   cv->deb_mode = deb_mode;
			   cv->plot = plot;
			   cv->parallel = parallel;
			   cv->observe = ( observe == 'y' ) ? true : false;
			}

			change_description( lab_old, NULL, -1, eval_str( "[ .chgelem.desc.f.desc.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

			if ( cv->param == 1 || cv->num_lag > 0 )
				change_description( lab_old, NULL, -1, NULL, eval_str( "[ .chgelem.desc.i.desc.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

			unsaved_change( true );		// signal unsaved change

			if ( save == 1 || savei == 1 )
				for ( cur = r; cur != NULL; cur = cur->up )
					if ( ! cur->to_compute )
						cmd( "ttk::messageBox -parent .chgelem -type ok -title Warning -icon warning -message \"Cannot save element\" -detail \"Element '%s' set to be saved but it will not be computed for the Analysis of Results, since object '%s' is not set to be computed.\"", lab_old, cur->label );
		}

		if ( done != 8 )
			choice = 0;
		else
			choice = 7;

		here_endelem:

		cmd( "destroytop .chgelem" );
		Tcl_UnlinkVar( inter, "done" );
		Tcl_UnlinkVar( inter, "save" );
		Tcl_UnlinkVar( inter, "savei" );
		Tcl_UnlinkVar( inter, "plot" );
		Tcl_UnlinkVar( inter, "debug" );
		Tcl_UnlinkVar( inter, "watch" );
		Tcl_UnlinkVar( inter, "watch_write" );
		Tcl_UnlinkVar( inter, "parallel" );
		cmd( "unset done" );

		// options to be handled in a second run of the operate function
		switch ( done )
		{
			case 5:
				choice = 75;			// open properties box for $vname
				break;
			case 10:
				choice = 76;			// delete element in $vname
				break;
			case 11:
				choice = 77;			// change initial values for $vname
				break;
			case 12:
				choice = 78;			// change sensitivity values for $vname
				break;
			case 13:
				choice = 79;			// move element in $vname
				break;
			case 14:
				choice = 96;			// change updating scheme
				break;
			default:
				choice = 0;
		}

		if ( choice != 0 )
		{
			redrawRoot = redrawStruc = false;	// no redraw yet
			return r;					// execute command
		}

	break;


	// Edit variable/parameter (defined by tcl $vname) properties
	case 75:
	// Delete variable/parameter (defined by tcl $vname)
	case 76:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );	// get var/par name in lab_old
		cv = r->search_var( NULL, lab_old );
		if ( cv == NULL )
			break;

		if ( choice == 76 )
		{
			delVar = renVar = true;

			cmd( "set answer [ ttk::messageBox -parent . -title Confirmation -icon question -type yesno -default yes -message \"Delete element?\" -detail \"Press 'Yes' to confirm deleting '$vname'\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );
			if ( choice == 1 )
				cmd( "set vname \"\"; set nature 3; set numlag 0" );	// configure to delete
			else
				goto here_endprop;
		}
		else
		{
			delVar = renVar = false;

			cmd( "set nature %d", cv->param );
			cmd( "if { $nature == 0 } { set numlag %d } { set numlag 0 }", cv->num_lag );

			cmd( "set T .prop" );
			cmd( "newtop $T \"Properties\" { set choice 2 }" );

			cmd( "ttk::frame $T.h" );
			cmd( "ttk::label $T.h.l1 -text \"Element:\"" );
			cmd( "ttk::label $T.h.l2 -text \"%s\" -style hl.TLabel", cv->label );
			cmd( "pack $T.h.l1 $T.h.l2 -side left -padx 2" );

			cmd( "ttk::frame $T.n" );
			cmd( "ttk::label $T.n.var -text \"Name\"" );
			cmd( "ttk::entry $T.n.e -width 20 -textvariable vname -justify center" );
			cmd( "ttk::label $T.n.sp -width 2" );
			cmd( "ttk::label $T.n.l -text \"Lags\"" );
			cmd( "ttk::spinbox $T.n.lag -justify center -width 3 -from 0 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set numlag %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numlag; return 0 } } -invalidcommand { bell }" );
			cmd( "$T.n.lag insert 0 $numlag" );
			cmd( "if { $nature != 0 } { $T.n.lag configure -state disabled }" );
			cmd( "pack $T.n.var $T.n.e $T.n.sp $T.n.l $T.n.lag -side left -padx 2" );

			cmd( "tooltip::tooltip $T.n.lag \"Maximum lag used in equations\"" );

			cmd( "ttk::frame $T.v" );
			cmd( "ttk::label $T.v.l -text \"Type\"" );

			cmd( "ttk::frame $T.v.o -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			cmd( "ttk::radiobutton $T.v.o.var -text Variable -variable nature -value 0 -underline 0 -command { $T.n.lag configure -state normal }" );
			cmd( "ttk::radiobutton $T.v.o.par -text Parameter -variable nature -value 1 -underline 0 -command { $T.n.lag configure -state disabled }" );
			cmd( "ttk::radiobutton $T.v.o.fun -text Function -variable nature -value 2 -underline 0 -command { $T.n.lag configure -state disabled }" );
			cmd( "pack	$T.v.o.var $T.v.o.par $T.v.o.fun -anchor w" );

			cmd( "pack $T.v.l $T.v.o" );

			cmd( "pack $T.h $T.n $T.v -padx 5 -pady 5" );

			cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menumodel.html#change_nature } { set choice 2 }" );

			cmd( "bind $T.n.e <KeyPress-Return> { set choice 1 }" );
			cmd( "bind $T <Control-v> { .prop.v.o.var invoke }; bind $T <Control-V> { .prop.v.o.var invoke }" );
			cmd( "bind $T <Control-p> { .prop.v.o.par invoke }; bind $T <Control-P> { .prop.v.o.par invoke }" );
			cmd( "bind $T <Control-f> { .prop.v.o.fun invoke }; bind $T <Control-F> { .prop.v.o.fun invoke }" );

			cmd( "showtop $T" );
			cmd( "mousewarpto $T.b.ok 0" );
			cmd( "$T.n.e selection range 0 end" );
			cmd( "focus $T.n.e" );

			choice = 0;
		}

		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "if [ winfo exists .prop ] { if { $nature == 0 } { set numlag [ .prop.n.lag get ] } }" );
		cmd( "destroytop .prop" );

		if ( choice == 2 )
			goto here_endprop;

		cmd( "set choice $nature" );
		nature = choice;

		cmd( "set choice $numlag" );
		numlag = choice;

		if ( ! delVar && ( nature != cv->param || numlag != cv->num_lag ) )
		{
			if ( nature != 1 && numlag == 0 )
				change_description( lab_old, NULL, nature, NULL, "" );
			else
				change_description( lab_old, NULL, nature );

			for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
			{
				cv = cur->search_var( NULL, lab_old );

				if ( cv == NULL )
					continue;

				double *old_val = cv->val;
				cv->val = new double[ numlag + 1 ];

				for ( i = 0; i <= numlag; ++i )
					cv->val[ i ] = 0;

				// avoid reseting initial values if not required
				if ( ( cv->param == 1 && numlag > 0 ) || ( nature == 1 && cv->num_lag > 0 ) )
					cv->val[ 0 ] = old_val[ 0 ];		// parameter <-> lagged variable
				else
					if ( cv->num_lag > 0 && numlag > 0 )// x-lags variable to y-lags variable?
						for ( i = 0; i < min( cv->num_lag, numlag ); ++i )
							cv->val[ i ] = old_val[ i ];

				delete [ ] old_val;
				cv->num_lag = numlag;
				cv->param = nature;

				if ( cv->param == 1 || cv->num_lag > 0 )
					cv->data_loaded = '-';

				if ( cv->param != 0 )
				{
					cv->parallel = false;
					cv->period = 1;
					cv->delay = cv->delay_range = cv->period_range = 0;
				}
			}
		}

		lab1 = get_str( "vname" );
		if ( lab1 != NULL && strcmp( lab1, "" ) )
		{
			sscanf( lab1, "%99s", lab );			// new name in lab (empty if delete)
			if ( strcmp( lab, lab_old ) )			// check new name if different
				renVar = true;
		}
		else
			if ( delVar )
				strcpy( lab, "" );
			else
				goto here_endprop;

		if ( renVar )
		{
			if ( ! delVar )
			{
				for ( cur = r; cur->up != NULL; cur = cur->up );
				choice = check_label( lab, cur );

				if ( choice == 1 )
				{
					cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"The name already exists in the model\" -detail \"Choose a different name and try again.\"" );
					goto here_endprop;
				}
				if ( choice == 2 )
				{
					cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid characters in name\" -detail \"Names must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
					goto here_endprop;
				}
			}

			// remove from element lists
			cmd( "if [ info exists modElem ] { set pos [ lsearch -exact $modElem %s ]; if { $pos >= 0 } { set modElem [ lreplace $modElem $pos $pos ] } }", lab_old	 );
			cmd( "if [ info exists modPar ] { set pos [ lsearch -exact $modPar %s ]; if { $pos >= 0 } { set modPar [ lreplace $modPar $pos $pos ] } }", lab_old	 );
			cmd( "if [ info exists modVar ] { set pos [ lsearch -exact $modVar %s ]; if { $pos >= 0 } { set modVar [ lreplace $modVar $pos $pos ] } }", lab_old	 );
			cmd( "if [ info exists modFun ] { set pos [ lsearch -exact $modFun %s ]; if { $pos >= 0 } { set modFun [ lreplace $modFun $pos $pos ] } }", lab_old	 );

			if ( ! delVar )
			{
				// add to element lists
				cmd( "lappend modElem %s", lab );

				if ( cv->param == 0 )
					cmd( "lappend modVar %s", lab );
				if ( cv->param == 1 )
					cmd( "lappend modPar %s", lab );
				if ( cv->param == 2 )
					cmd( "lappend modFun %s", lab );

				change_description( lab_old, lab );
			}

			for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
				if ( ! delVar )
					cur->chg_var_lab( lab_old, lab );
				else
					cur->delete_var( lab_old );
		}

		if ( root->v == NULL && root->b == NULL )	// if last variable
		{
			unsaved_change( false );				// no unsaved change
			struct_loaded = false;					// no config loaded
		}
		else
			unsaved_change( true );					// signal unsaved change

		redrawRoot = redrawStruc = true;			// force browser/structure redraw

		here_endprop:

	break;


	// Move variable/parameter (defined by tcl $vname)
	case 79:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );	// get var/par name in lab_old

		cmd( "set TT .objs" );
		cmd( "newtop $TT \"Move\" { set choice 2 }" );

		cmd( "ttk::frame $TT.l" );
		cmd( "ttk::label $TT.l.l -text \"Element:\"" );
		cmd( "ttk::label $TT.l.n -style hl.TLabel -text \"%s\"", lab_old );
		cmd( "pack $TT.l.l $TT.l.n -side left -padx 2" );

		cmd( "ttk::frame $TT.v" );
		cmd( "ttk::label $TT.v.l -text \"Move to\"" );

		cmd( "ttk::frame $TT.v.t" );
		cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
		cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
		cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
		cmd( "mouse_wheel $TT.v.t.lb" );
		insert_object( "$TT.v.t.lb", root );
		cmd( "pack $TT.v.l $TT.v.t" );

		cmd( "pack $TT.l $TT.v -padx 5 -pady 5" );

		cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

		cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0; break }" );
		cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end; break }" );
		cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

		cmd( "showtop $TT" );
		cmd( "$TT.v.t.lb selection set 0" );
		cmd( "focus $TT.v.t.lb" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set movelabel [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ]" );
		cmd( "destroytop .objs" );

		if ( choice == 2 )
			break;

		lab1 = get_str( "movelabel" );
		if ( lab1 == NULL || ! strcmp( lab1, r->label ) )		// same object?
			break;

		cv = r->search_var( NULL, lab_old );

		for ( cur = root->search( lab1 ); cur != NULL; cur = cur->hyper_next( cur->label ) )
			cur->add_var_from_example( cv );

		for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
			cur->delete_var( lab_old );

		unsaved_change( true );		// signal unsaved change
		redrawRoot = redrawStruc = true;	// force browser/structure redraw

	break;


	// Change variable/parameter (defined by tcl $vname) initial values
	case 77:
	// Change variable/parameter (defined by tcl $vname) sensitivity values
	case 78:

		done = ( choice == 77 ) ? 1 : 2;

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );		// get var/par name in lab_old

		if ( initVal && initParent != NULL )
			cv = initParent->search_var( NULL, lab_old );// get var/par pointer
		else
			cv = r->search_var( NULL, lab_old );// get var/par pointer

		if ( cv == NULL )
			break;

		// do lag selection, if necessary, for initialization/sensitivity data entry
		lag = 0;								// lag option for the next cases (first lag)
		if ( ! initVal && ( cv->param == 0 || cv->param == 2 ) && cv->num_lag > 1 )
		{										// more than one lag to choose?
			cmd( "set lag \"1\"" );

			// confirm which lag to use
			cmd( "set T .lag" );
			cmd( "newtop $T \"Lag Selection\" { set choice 0 }" );

			cmd( "ttk::frame $T.i" );
			cmd( "ttk::label $T.i.l -text \"Use lag\"" );
			cmd( "ttk::spinbox $T.i.e -justify center -width 3 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 && $n <= %d } { set lag %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lag; return 0 } } -invalidcommand { bell }", cv->num_lag, cv->num_lag );
			cmd( "$T.i.e insert 0 $lag" );
			cmd( "pack $T.i.l $T.i.e -side left -padx 2" );

			cmd( "ttk::frame $T.o" );
			cmd( "ttk::label $T.o.l1 -text \"( valid values:\"" );
			cmd( "ttk::label $T.o.w1 -text 1 -style hl.TLabel" );
			cmd( "ttk::label $T.o.l2 -text to" );
			cmd( "ttk::label $T.o.w2 -text %d -style hl.TLabel", cv->num_lag );
			cmd( "ttk::label $T.o.l3 -text \")\"" );
			cmd( "pack $T.o.l1 $T.o.w1 $T.o.l2 $T.o.w2 $T.o.l3 -side left -padx 2" );

			cmd( "pack $T.i $T.o -padx 5 -pady 5" );

			cmd( "okcancel $T b { set choice $lag } { set choice 0 }" );
			cmd( "bind $T <KeyPress-Return> { set choice $lag }" );

			cmd( "showtop $T" );
			cmd( "mousewarpto $T.b.ok 0" );

			choice = -1;
			while ( choice == -1 )			// wait for user action
				Tcl_DoOneEvent( 0 );

			cmd( "set lag [ .lag.i.e get ]" );
			cmd( "destroytop .lag" );

			if ( choice == 0 )
				break;

			cmd( "set choice $lag" );
			lag = abs( choice ) - 1;		// try to extract chosed lag

			// abort if necessary
			if ( lag < 0 || lag > ( cv->num_lag - 1 ) )
			{
				cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid lag selected\" -detail \"Select a valid lag value for the variable or change the number of lagged values for this variable.\"" );
				break;
			}
		}

		// initialize
		if ( done == 1 )
		{
			if ( initVal )					// running just after element creation?
			{
				lag = next_lag;
				cur = initParent;
			}
			else
				cur = r;

			set_all( cur, cv->label, lag );
			redrawRoot = true;				// redraw is needed to show new value tip

			if ( initVal )
			{
				if ( next_lag < ( cv->num_lag - 1 ) )
				{
					++next_lag;
					choice = 77;			// execute command again
					return r;
				}
				else
				{
					initVal = false;
					initParent = NULL;
					redrawStruc = true;		// redraw is needed to show new element
				}
			}
		}
		// edit sensitivity analysis data
		else
		{
			choice = 0;
			bool exist = false;
			sense *cs, *ps = NULL;

			if ( rsense == NULL )			// no sensitivity analysis structure yet?
				rsense = cs = new sense;
			else
			{
				// check if sensitivity data for the variable already exists
				for ( cs = rsense, ps = NULL; cs != NULL; ps = cs, cs = cs->next )
					if ( ! strcmp( cs->label, cv->label ) &&
						 ( cs->param == 1 || cs->lag == lag ) )
					{
						exist = true;
						break;				// get out of the inner for loop
					}

				if ( ! exist )				// if new variable, append at the end of the list
				{
					for ( cs = rsense; cs->next != NULL; cs = cs->next );	// pick last
					cs->next = new sense;	// create new variable
					ps = cs;				// keep previous sensitivity variable
					cs = cs->next;
				}
			}

			if ( ! exist )					// do only for new variables in the list
			{
				cs->label = new char[ strlen( cv->label ) + 1 ];
				strcpy( cs->label, cv->label );
				cs->next = NULL;
				cs->nvalues = 0;
				cs->v = NULL;
				cs->entryOk = false;		// no valid data yet
			}
			else
				cs->entryOk = true;			// valid data already there

			// save type and specific lag in this case
			cs->param = cv->param;
			cs->lag = lag;

			dataentry_sensitivity( cs, 0 );

			if ( ! cs->entryOk )			// data entry failed?
			{
				if ( rsense == cs )			// is it the first variable?
					rsense = cs->next;		// update list root
				else
					ps->next = cs->next;	// remove from sensitivity list
				delete [ ] cs->label;		// garbage collection
				delete cs;
			}
			else
				unsavedSense = true;		// signal unsaved change
		}

	break;


	// Change variable (defined by tcl $vname) updating scheme
	case 96:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );	// get var/par name in lab_old
		cv = r->search_var( NULL, lab_old );// get var/par pointer
		if ( cv == NULL )
			break;

		// save previous values to allow canceling operation
		temp[ 1 ] = cv->delay;
		temp[ 2 ] = cv->delay_range;
		temp[ 3 ] = cv->period;
		temp[ 4 ] = cv->period_range;

		Tcl_LinkVar( inter, "delay", ( char * ) & cv->delay, TCL_LINK_INT );
		Tcl_LinkVar( inter, "delay_range", ( char * ) & cv->delay_range, TCL_LINK_INT );
		Tcl_LinkVar( inter, "period", ( char * ) & cv->period, TCL_LINK_INT );
		Tcl_LinkVar( inter, "period_range", ( char * ) & cv->period_range, TCL_LINK_INT );

		cmd( "set T .updating" );
		cmd( "newtop $T \"Variable Updating\" { set choice 2 }" );

		cmd( "ttk::frame $T.h" );
		cmd( "ttk::label $T.h.l1 -text \"Variable:\"" );
		cmd( "ttk::label $T.h.l2 -text \"%s\" -style hl.TLabel", cv->label );
		cmd( "pack $T.h.l1 $T.h.l2 -side left -padx 2" );

		cmd( "ttk::frame $T.f" );

		cmd( "ttk::frame $T.f.c" );
		cmd( "ttk::label $T.f.c.l2 -width 20 -anchor e -text \"Initial delay\"" );
		cmd( "ttk::spinbox $T.f.c.e2 -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set delay %%P; return 1 } { %%W delete 0 end; %%W insert 0 $delay; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.c.e2 insert 0 $delay" );
		cmd( "pack $T.f.c.l2 $T.f.c.e2 -side left -anchor w -padx 2 -pady 2" );

		cmd( "ttk::frame $T.f.a" );
		cmd( "ttk::label $T.f.a.l -width 20 -anchor e -text \"Random delay range\"" );
		cmd( "ttk::spinbox $T.f.a.e -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set delay_range %%P; return 1 } { %%W delete 0 end; %%W insert 0 $delay_range; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.a.e insert 0 $delay_range" );
		cmd( "pack $T.f.a.l $T.f.a.e -side left -anchor w -padx 2 -pady 2" );

		cmd( "ttk::frame $T.f.b" );
		cmd( "ttk::label $T.f.b.l1 -width 20 -anchor e -text \"Period\"" );
		cmd( "ttk::spinbox $T.f.b.e1 -width 7 -from 1 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set period %%P; return 1 } { %%W delete 0 end; %%W insert 0 $period; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.b.e1 insert 0 $period" );
		cmd( "pack $T.f.b.l1 $T.f.b.e1 -side left -anchor w -padx 2 -pady 2" );

		cmd( "ttk::frame $T.f.d" );
		cmd( "ttk::label $T.f.d.l2 -width 20 -anchor e -text \"Random period range\"" );
		cmd( "ttk::spinbox $T.f.d.e2 -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set period_range %%P; return 1 } { %%W delete 0 end; %%W insert 0 $period_range; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.d.e2 insert 0 $period_range" );
		cmd( "pack $T.f.d.l2 $T.f.d.e2 -side left -anchor w -padx 2 -pady 2" );

		cmd( "pack $T.f.c $T.f.a $T.f.b $T.f.d -anchor w" );

		cmd( "pack $T.h $T.f -padx 5 -pady 5" );

		cmd( "tooltip::tooltip $T.f.c \"First case (time step) to compute the variable\"" );
		cmd( "tooltip::tooltip $T.f.a \"Maximum case (time step) for uniform random first computation\"" );
		cmd( "tooltip::tooltip $T.f.b \"Period between computations of variable\"" );
		cmd( "tooltip::tooltip $T.f.d \"Maximum period for uniform random periodic computation\"" );

		cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp browser.html#updating } { set choice 2 }" );

		cmd( "bind $T.f.c.e2 <KeyPress-Return> { focus $T.f.a.e; $T.f.a.e selection range 0 end }" );
		cmd( "bind $T.f.a.e <KeyPress-Return> { focus $T.f.b.e1; $T.f.b.e1 selection range 0 end }" );
		cmd( "bind $T.f.b.e1 <KeyPress-Return> { focus $T.f.d.e2; $T.f.d.e2 selection range 0 end }" );
		cmd( "bind $T.f.d.e2 <KeyPress-Return> { focus $T.f.e.e2; $T.f.e.e2 selection range 0 end }" );

		cmd( "showtop $T" );
		cmd( "mousewarpto $T.b.ok 0" );
		cmd( "$T.f.c.e2 selection range 0 end" );
		cmd( "focus $T.f.c.e2" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set delay [ $T.f.c.e2 get ]" );
		cmd( "set delay_range [ $T.f.a.e get ]" );
		cmd( "set period [ $T.f.b.e1 get ]" );
		cmd( "set period_range [ $T.f.d.e2 get ]" );

		cmd( "destroytop $T" );

		if ( choice == 2 )	// Escape - revert previous values
		{
			cv->delay = temp[ 1 ];
			cv->delay_range = temp[ 2 ];
			cv->period = temp[ 3 ];
			cv->period_range = temp[ 4 ];
		}
		else
		// signal unsaved change if anything to be saved
			if ( temp[ 1 ] != cv->delay || temp[ 2 ] != cv->delay_range || temp[ 3 ] != cv->period || temp[ 4 ] != cv->period_range )
			{
				for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
				{
					cv1 = cur->search_var( NULL, lab_old );
					cv1->delay = cv->delay;
					cv1->delay_range = cv->delay_range;
					cv1->period = cv->period;
					cv1->period_range = cv->period_range;
				}

				unsaved_change( true );
				redrawRoot = true;
			}

		Tcl_UnlinkVar( inter, "delay" );
		Tcl_UnlinkVar( inter, "delay_range" );
		Tcl_UnlinkVar( inter, "period" );
		Tcl_UnlinkVar( inter, "period_range" );

	break;


	// Exit the browser and run the simulation
	case 1:

		if ( struct_loaded && strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration not saved\" -detail \"Please save your current configuration before trying to run the simulation.\"" );

			choice = 73;
			return r;
		}

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to run the simulation.\"" );
			break;
		}

		// warn about no variable/parameter being saved
		for ( n = r; n->up != NULL; n = n->up );
		series_saved = 0;
		count_save( n, &series_saved );
		if ( series_saved == 0 )
		{
			cmd( "set answer [ ttk::messageBox -parent . -type okcancel -default ok -icon warning -title Warning -message \"No variable or parameter marked to be saved\" -detail \"If you proceed, there will be no data to be analyzed after the simulation is run. If this is not the intended behavior, please mark the variables and parameters to be saved before running the simulation.\" ]; switch -- $answer { ok { set choice 1 } cancel { set choice 2 } } " );
			if ( choice == 2 )
				break;
		}

		// warn missing debugger
		if ( ! parallel_disable && search_parallel( root ) && ( when_debug > 0 || stack_info > 0 || prof_aggr_time ) )
		{
			cmd( "set answer [ ttk::messageBox -parent . -title Warning -icon warning -type okcancel -default ok -message \"Debugger/profiler not available\" -detail \"Debugging in parallel mode is not supported, including stack profiling.\n\nPress 'OK' to proceed and disable parallel processing settings or 'Cancel' to return to LSD Browser.\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } }" );
			if ( choice == 2 )
				break;

			parallel_disable = true;
		}

		// save the current object & cursor position for quick reload
		save_pos( r );

		// only ask to overwrite configuration if there are changes
		overwConf = unsaved_change( ) ? true : false;

		// avoid showing dialog if configuration already saved and nothing to save to disk
		if ( ! overwConf && sim_num == 1 )
			goto run;

		// remove any custom save path (save to current by default)
		results_alt_path( "" );

		Tcl_LinkVar( inter, "no_res", ( char * ) & no_res, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "no_tot", ( char * ) & no_tot, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "add_to_tot", ( char * ) & add_to_tot, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "docsv", ( char * ) & docsv, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "doover", ( char * ) & doover, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "dozip", ( char * ) & dozip, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "overwConf", ( char * ) & overwConf, TCL_LINK_BOOLEAN );

		cmd( "set firstFile \"%s_%d\"", simul_name, seed );
		cmd( "set lastFile \"%s_%d\"", simul_name, seed + sim_num - 1 );
		cmd( "set totFile \"%s\"", simul_name );
		cmd( "set resExt %s", docsv ? "csv" : "res" );
		cmd( "set totExt %s", docsv ? "csv" : "tot" );
		cmd( "set zipExt \"%s\"", dozip ? ".gz" : "" );
		cmd( "set tot_msg_warn \"(totals file already exists)\"" );

		cmd( "set T .run" );
		cmd( "newtop $T \"Run Simulation\" { set choice 2 }" );

		cmd( "ttk::frame $T.f1" );
		cmd( "ttk::label $T.f1.l -text \"Model configuration\"" );
		cmd( "ttk::label $T.f1.w -text \"%s\" -style hl.TLabel", simul_name );
		cmd( "pack $T.f1.l $T.f1.w" );

		cmd( "ttk::frame $T.f2" );

		cmd( "ttk::frame $T.f2.t" );
		cmd( "ttk::label $T.f2.t.l -text \"Cases:\"" );
		cmd( "ttk::label $T.f2.t.w -text \"%d\" -style hl.TLabel", max_step );
		cmd( "pack $T.f2.t.l $T.f2.t.w -side left -padx 2" );

		if ( sim_num > 1 )
		{
			// detect the need of a new save path and if it has results files
			subDir = need_res_dir( path, simul_name, out_dir, MAX_PATH_LENGTH );
			overwDir = check_res_dir( out_dir );

			cmd( "ttk::frame $T.f2.n" );
			cmd( "ttk::label $T.f2.n.l -text \"Number of simulations:\"" );
			cmd( "ttk::label $T.f2.n.w -text \"%d\" -style hl.TLabel", sim_num );
			cmd( "pack $T.f2.n.l $T.f2.n.w -side left -padx 2" );

			cmd( "pack $T.f2.t $T.f2.n" );

			cmd( "ttk::frame $T.f3" );
			cmd( "ttk::label $T.f3.l -text \"Output path\"" );
			cmd( "ttk::label $T.f3.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", out_dir );
			cmd( "pack $T.f3.l $T.f3.w" );

			cmd( "ttk::frame $T.f4" );
			cmd( "ttk::label $T.f4.l -text \"Results files\"" );

			cmd( "ttk::frame $T.f4.w" );

			cmd( "ttk::frame $T.f4.w.l1" );
			cmd( "ttk::label $T.f4.w.l1.l -text \"from:\"" );
			cmd( "ttk::label $T.f4.w.l1.w -style hl.TLabel -text \"$firstFile.$resExt$zipExt\"" );
			cmd( "pack $T.f4.w.l1.l $T.f4.w.l1.w -side left -padx 2" );

			cmd( "ttk::frame $T.f4.w.l2" );
			cmd( "ttk::label $T.f4.w.l2.l -text \"to:\"" );
			cmd( "ttk::label $T.f4.w.l2.w -style hl.TLabel -text \"$lastFile.$resExt$zipExt\"" );
			cmd( "pack $T.f4.w.l2.l $T.f4.w.l2.w -side left -padx 2" );

			cmd( "pack $T.f4.w.l1 $T.f4.w.l2" );

			cmd( "pack $T.f4.l $T.f4.w" );

			cmd( "set choice [ expr { ! $no_tot && [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } ]", out_dir, strlen( out_dir ) > 0 ? "/" : "" );

			cmd( "ttk::frame $T.f5" );
			cmd( "ttk::label $T.f5.l1 -text \"Totals file (last steps)\"" );
			cmd( "ttk::label $T.f5.l2 -style %s -text \"$totFile.$totExt$zipExt\"", choice ? "hl.TLabel" : "dhl.TLabel" );

			if ( choice )
				cmd( "ttk::label $T.f5.l3 -text $tot_msg_warn" );
			else
				cmd( "ttk::label $T.f5.l3 -text \"\"" );

			cmd( "pack $T.f5.l1 $T.f5.l2 $T.f5.l3" );

			add_to_tot = ( choice ) ? add_to_tot : false;

			cmd( "ttk::frame $T.f6" );
			cmd( "ttk::checkbutton $T.f6.a -text \"Append to existing totals file\" -variable add_to_tot -state %s -command { \
					if { $add_to_tot && $doover } { \
						set doover 0 \
					} \
				}", ( choice && ! no_tot ) ? "normal" : "disabled" );
			cmd( "ttk::checkbutton $T.f6.b -text \"Skip generating results files\" -variable no_res" );
			cmd( "ttk::checkbutton $T.f6.b1 -text \"Skip generating totals file\" -variable no_tot -command { \
					if { ! $no_tot } { \
						$T.f5.l2 configure -style hl.TLabel; \
						if { [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
							$T.f5.l3 configure -text $tot_msg_warn; \
							$T.f6.a configure -state normal \
						} else { \
							$T.f5.l3 configure -text \"\"; \
						} \
					} else { \
						$T.f5.l2 configure -style dhl.TLabel; \
						$T.f5.l3 configure -text \"\"; \
						$T.f6.a configure -state disabled \
					} \
				}", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
			cmd( "ttk::checkbutton $T.f6.c -text \"Generate zipped files\" -variable dozip -command { \
				if $dozip { set zipExt \".gz\" } { \
					set zipExt \"\" }; \
					$T.f4.w.l1.w configure -text \"$firstFile.$resExt$zipExt\"; \
					$T.f4.w.l2.w configure -text \"$lastFile.$resExt$zipExt\"; \
					$T.f5.l2 configure -text \"$totFile.$totExt$zipExt\"; \
					if { ! $no_tot && [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
						$T.f5.l3 configure -text $tot_msg_warn; \
						$T.f6.a configure -state normal \
					} else { \
						$T.f5.l3 configure -text \"\"; \
						$T.f6.a configure -state disabled \
					} \
				}", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
			cmd( "ttk::checkbutton $T.f6.d -text \"Comma-separated text format (.csv)\" -variable docsv -command { \
				if $docsv { \
					set resExt csv; set totExt csv \
				} else { \
					set resExt res; \
					set totExt tot }; \
					$T.f4.w.l1.w configure -text \"$firstFile.$resExt$zipExt\"; \
					$T.f4.w.l2.w configure -text \"$lastFile.$resExt$zipExt\"; \
					$T.f5.l2 configure -text \"$totFile.$totExt$zipExt\"; \
					if { ! $no_tot && [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
						$T.f5.l3 configure -text $tot_msg_warn; \
						$T.f6.a configure -state normal \
					} else { \
						$T.f5.l3 configure -text \"\"; \
						$T.f6.a configure -state disabled \
					} \
				}", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
			cmd( "ttk::checkbutton $T.f6.o -text \"Clear output path before run\" -variable doover -state %s -command { \
					if { $add_to_tot && $doover } { \
						set add_to_tot 0 \
					} \
				}", overwDir ? "normal" : "disabled" );
			cmd( "ttk::checkbutton $T.f6.e -text \"Update configuration file\" -variable overwConf -state %s", overwConf ? "normal" : "disabled" );
			cmd( "pack $T.f6.a $T.f6.b $T.f6.b1 $T.f6.c $T.f6.d $T.f6.o $T.f6.e -anchor w" );

			cmd( "pack $T.f1 $T.f2 $T.f3 $T.f4 $T.f5 $T.f6 -padx 5 -pady 5" );
		}
		else
		{
			subDir = overwDir = false;

			cmd( "pack $T.f2.t" );

			cmd( "ttk::label $T.f4 -text \"(results will be saved to memory only)\"" );

			cmd( "ttk::checkbutton $T.f6 -text \"Update configuration file\" -variable overwConf -state %s", overwConf ? "normal" : "disabled" );

			cmd( "pack $T.f1 $T.f2 $T.f4 $T.f6 -padx 5 -pady 5" );
		}

		cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menurun.html#run } { set choice 2 }" );

		cmd( "showtop $T" );
		cmd( "mousewarpto $T.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .run" );

		Tcl_UnlinkVar( inter, "no_res" );
		Tcl_UnlinkVar( inter, "no_tot" );
		Tcl_UnlinkVar( inter, "add_to_tot" );
		Tcl_UnlinkVar( inter, "docsv" );
		Tcl_UnlinkVar( inter, "doover" );
		Tcl_UnlinkVar( inter, "dozip" );
		Tcl_UnlinkVar( inter, "overwConf" );

		if ( choice == 2 )
			break;

		if ( ( ! no_res || ! no_tot ) && subDir )
			if ( ! create_res_dir( out_dir ) || ! results_alt_path( out_dir ) )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Subdirectory '%s' cannot be created\" -detail \"Check if the path is set READ-ONLY, or move your configuration file to a different location.\"", out_dir );
				break;
			}

		if ( overwDir && doover )
			clean_res_dir( out_dir );

		run:

		for ( n = r; n->up != NULL; n = n->up );
		reset_blueprint( n );				// update blueprint to consider last changes

		if ( overwConf )					// save if needed
		{
			if ( ! save_configuration( ) )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s.lsd' cannot be saved\" -detail \"Check if the file is set READ-ONLY, or try to save to a different location.\"", simul_name );
				break;
			}
			else
				unsaved_change( false );	// signal no unsaved change
		}

		choice = 1;
		return n;


	// Load a model
	case 17:
	// Reload model
	case 38:

		if ( discard_change( ) )	// unsaved configuration changes ?
			if ( ! open_configuration( r, choice == 38 ? true : false ) )
			{
				unload_configuration( true );
				choice = 0;
				return root;
			}

	break;


	// Save a model
	case 18:
	// Save a model as different name
	case 73:

		saveAs = ( choice == 73 || strlen( simul_name ) == 0 ) ? true : false;

		if ( ! struct_loaded )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration to save\" -detail \"Create a configuration before saving.\"" );
			break;
		}

		Tcl_LinkVar( inter, "done", ( char * ) &done, TCL_LINK_INT );

		if ( actual_steps > 0 )
		{
			if ( save_ok )
				cmd( "set answer [ ttk::messageBox -parent . -type okcancel -default cancel -icon warning -title Warning -message \"Configuration is the final state of a simulation run\" -detail \"Press 'OK' to save it anyway%s or 'Cancel' to abort saving.\" ]; switch -- $answer { ok { set done 1 } cancel { set done 2 } }", saveAs ? "" : " under a different name" );
			else
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration cannot be saved\" -detail \"Current configuration is the final state of a simulation run which has an incomplete structure that cannot be reliably saved.\n\nThis is due to the usage of USE_ZERO_INSTANCE macro, which allowed zero-instance objects in the current model structure.\"; set done 2" );

			if ( done == 2 )
			{
				Tcl_UnlinkVar( inter, "done" );
				cmd( "unset done" );
				break;
			}

			saveAs = true;		// require file name to save
		 }

		done = 0;
		cmd( "set res \"%s\"", strlen( simul_name ) > 0 ? simul_name : DEF_CONF_FILE );
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		if ( saveAs )			// only asks file name if instructed to or necessary
		{
			if ( actual_steps > 0 )
			{
				cmd( "set fn [ tk_getSaveFile -parent . -title \"Save Configuration File\" -defaultextension \".lsd\" -initialdir \"$path\" -filetypes { { {LSD model files} {.lsd} } } ]" );
				cmd( "if { [ string equal -nocase [ file normalize $fn ] [ file normalize \"$path/$res.lsd\" ] ] && [ ttk::messageBox -parent . -type okcancel -default cancel -icon warning -title Warning -message \"Overwrite existing configuration?\" -detail \"The original model configuration will be overwritten by the final state of the simulation run and, therefore, lost.\n\nPress 'OK' if you are sure or 'Cancel' to abort saving.\" ] eq \"cancel\" } { set fn \"\" }" );
			}
			else
				cmd( "set fn [ tk_getSaveFile -parent . -title \"Save Configuration File\" -defaultextension \".lsd\" -initialfile $res -initialdir \"$path\" -filetypes { { {LSD model files} {.lsd} } } ]" );

			cmd( "if { [ string length $fn ] > 0 && ! [ fn_spaces \"$fn\" . ] } { \
						set path [ file dirname $fn ]; \
						set fn [ string map -nocase [ list [ file extension $fn ] \"\" ] [ file tail $fn ] ]; \
					} else { \
						set done 2 \
					}" );

			if ( done == 2 )
				goto save_end;

			lab1 = get_str( "fn" );
			if ( strlen( lab1 ) == 0 )
				break;

			delete [ ] simul_name;
			simul_name = new char[ strlen( lab1 ) + 1 ];
			strcpy( simul_name, lab1 );

			lab1 = get_str( "path" );
			delete [ ] path;
			path = new char[ strlen( lab1 ) + 1 ];
			strcpy( path, lab1 );

			delete [ ] struct_file;
			struct_file = new char[ strlen( path ) + strlen( simul_name ) + 6 ];
			sprintf( struct_file, "%s%s%s.lsd", path, strlen( path ) > 0 ? "/" : "", simul_name );

			if ( strlen( lab1 ) > 0 )
				cmd( "cd $path" );

			redrawStruc = true;		// structure redraw because of titlebar
		}

		if ( ! save_configuration( ) )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s.lsd' cannot be saved\" -detail \"The model is NOT saved! Check if the drive or the file is set READ-ONLY, change file name or select a drive with write permission and try again.\"", simul_name	);
		}
		else
			unsaved_change( false );					// signal no unsaved change

		save_end:
		Tcl_UnlinkVar( inter, "done" );
		cmd( "unset done" );

	break;


	// Unload the model
	case 20:

		if ( ! discard_change( ) )		// check for unsaved configuration changes
			break;

		unload_configuration( true );

		r = root;						// just an empty root exists

	break;


	// Edit Objects' numbers
	case 19:

		strcpyn( lab, r->label, MAX_BUFF_SIZE );

		choice = 0;
		set_obj_number( root );

		r = root->search( lab );

	break;


	// Edit initial values for Objects currently selected or pointed by the browser (defined by tcl $vname)
	case 21:

		// check if current or pointed object and save current if needed
		lab1 = get_str( "useCurrObj" );
		if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
		{
			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			n = root->search( lab_old );		// set pointer to $vname
			if ( n == NULL )
				break;
			cur2 = r;
			r = n;
		}
		else
			cur2 = NULL;

		for ( n = r; n->up != NULL; n = n->up );

		edit_data( n, r->label );

		redrawRoot = true;
		unsaved_change( true );			// signal unsaved change

		if ( cur2 != NULL )				// restore original current object
			r = cur2;

	break;


	// Simulation manager: sets seeds, number of steps, number of simulations
	case 22:

		// save previous values to allow canceling operation
		temp[ 1 ] = sim_num;
		temp[ 2 ] = seed;
		temp[ 3 ] = max_step;
		temp[ 4 ] = when_debug;
		temp[ 5 ] = stack_info;
		temp[ 6 ] = prof_min_msecs;
		temp[ 7 ] = prof_obs_only;
		temp[ 8 ] = prof_aggr_time;
		temp[ 9 ] = no_ptr_chk;
		temp[ 10 ] = parallel_disable;

		Tcl_LinkVar( inter, "sim_num", ( char * ) & sim_num, TCL_LINK_INT );
		Tcl_LinkVar( inter, "seed", ( char * ) & seed, TCL_LINK_INT );
		Tcl_LinkVar( inter, "max_step", ( char * ) & max_step, TCL_LINK_INT );
		Tcl_LinkVar( inter, "stack_info", ( char * ) & stack_info, TCL_LINK_INT );
		Tcl_LinkVar( inter, "prof_min_msecs", ( char * ) & prof_min_msecs, TCL_LINK_INT );
		Tcl_LinkVar( inter, "prof_obs_only", ( char * ) & prof_obs_only, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "prof_aggr_time", ( char * ) & prof_aggr_time, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "no_ptr_chk", ( char * ) & no_ptr_chk, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "parallel_disable", ( char * ) & parallel_disable, TCL_LINK_BOOLEAN );

		cmd( "set tw 28" );					// text label width

		cmd( "set T .simset" );
		cmd( "newtop $T \"Simulation Settings\" { set choice 2 }" );

		cmd( "ttk::frame $T.f" );

		cmd( "ttk::frame $T.f.c" );
		cmd( "ttk::label $T.f.c.l2 -width $tw -anchor e -text \"Simulation steps\"" );
		cmd( "ttk::spinbox $T.f.c.e2 -width 7 -from 1 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set max_step %%P; return 1 } { %%W delete 0 end; %%W insert 0 $max_step; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.c.e2 insert 0 $max_step" );
		cmd( "pack $T.f.c.l2 $T.f.c.e2 -side left -anchor w -padx 2 -pady 2" );

		cmd( "ttk::frame $T.f.a" );
		cmd( "ttk::label $T.f.a.l -width $tw -anchor e -text \"Number of simulation runs\"" );
		cmd( "ttk::spinbox $T.f.a.e -width 7 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set sim_num %%P; return 1 } { %%W delete 0 end; %%W insert 0 $sim_num; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.a.e insert 0 $sim_num" );
		cmd( "pack $T.f.a.l $T.f.a.e -side left -anchor w -padx 2 -pady 2" );

		cmd( "ttk::frame $T.f.b" );
		cmd( "ttk::label $T.f.b.l1 -width $tw -anchor e -text \"Random numbers initial seed\"" );
		cmd( "ttk::spinbox $T.f.b.e1 -width 7 -from 1 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set seed %%P; return 1 } { %%W delete 0 end; %%W insert 0 $seed; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.b.e1 insert 0 $seed" );
		cmd( "pack $T.f.b.l1 $T.f.b.e1 -side left -anchor w -padx 2 -pady 2" );

		cmd( "ttk::frame $T.f.d" );
		cmd( "ttk::label $T.f.d.l2 -width $tw -anchor e -text \"Start debugger at step (0:none)\"" );
		cmd( "ttk::spinbox $T.f.d.e2 -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set when_debug %%P; return 1 } { %%W delete 0 end; %%W insert 0 $when_debug; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.d.e2 insert 0 $when_debug" );
		cmd( "pack $T.f.d.l2 $T.f.d.e2 -side left -anchor w -padx 2 -pady 2" );

		cmd( "ttk::frame $T.f.e" );
		cmd( "ttk::label $T.f.e.l2 -width $tw -anchor e -text \"Profile up to stack level (0:none)\"" );
		cmd( "ttk::spinbox $T.f.e.e2 -width 7 -from 0 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 && $n <= 99 } { set stack_info %%P; return 1 } { %%W delete 0 end; %%W insert 0 $stack_info; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.e.e2 insert 0 $stack_info" );
		cmd( "pack $T.f.e.l2 $T.f.e.e2 -side left -anchor w -padx 2 -pady 2" );

		cmd( "ttk::frame $T.f.f" );
		cmd( "ttk::label $T.f.f.l2 -width $tw -anchor e -text \"Profile minimum time (0:all)\"" );
		cmd( "ttk::spinbox $T.f.f.e2 -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set prof_min_msecs %%P; return 1 } { %%W delete 0 end; %%W insert 0 $prof_min_msecs; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "$T.f.f.e2 insert 0 $prof_min_msecs" );
		cmd( "pack $T.f.f.l2 $T.f.f.e2 -side left -anchor w -padx 2 -pady 2" );

		cmd( "pack $T.f.c $T.f.a $T.f.b $T.f.d $T.f.e $T.f.f -anchor w" );

		cmd( "ttk::frame $T.c" );

		cmd( "ttk::checkbutton $T.c.obs -text \"Profile observed variables only\" -variable prof_obs_only" );
		cmd( "ttk::checkbutton $T.c.aggr -text \"Show aggregated profiling times\" -variable prof_aggr_time" );
		cmd( "ttk::checkbutton $T.c.nchk -text \"Disable pointer checks\" -variable no_ptr_chk -state %s", no_pointer_check ? "disabled" : "normal" );

#ifndef _NP_
		cmd( "ttk::checkbutton $T.c.npar -text \"Disable parallel computation\" -variable parallel_disable" );
		if ( ! search_parallel( root ) || max_threads < 2 )
			cmd( "$T.c.npar configure -state disabled" );
		cmd( "pack $T.c.obs $T.c.aggr $T.c.nchk $T.c.npar -anchor w" );
#else
		cmd( "pack $T.c.obs $T.c.aggr $T.c.nchk -anchor w" );
#endif

		cmd( "pack $T.f $T.c -padx 5 -pady 5" );

		cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menurun.html#simsetting } { set choice 2 }" );
		cmd( "bind $T.f.c.e2 <KeyPress-Return> { focus $T.f.a.e; $T.f.a.e selection range 0 end }" );
		cmd( "bind $T.f.a.e <KeyPress-Return> { focus $T.f.b.e1; $T.f.b.e1 selection range 0 end }" );
		cmd( "bind $T.f.b.e1 <KeyPress-Return> { focus $T.f.d.e2; $T.f.d.e2 selection range 0 end }" );
		cmd( "bind $T.f.d.e2 <KeyPress-Return> { focus $T.f.e.e2; $T.f.e.e2 selection range 0 end }" );
		cmd( "bind $T.f.e.e2 <KeyPress-Return> { focus $T.f.f.e2; $T.f.f.e2 selection range 0 end }" );
		cmd( "bind $T.f.f.e2 <KeyPress-Return>	{ focus $T.b.ok }" );

		cmd( "showtop $T" );
		cmd( "mousewarpto $T.b.ok 0" );
		cmd( "$T.f.c.e2 selection range 0 end" );
		cmd( "focus $T.f.c.e2" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set sim_num [ .simset.f.a.e get ]" );
		cmd( "set seed [ .simset.f.b.e1 get ]" );
		cmd( "set max_step [ .simset.f.c.e2 get ]" );
		cmd( "set when_debug [ .simset.f.d.e2 get ]" );
		cmd( "set stack_info [ .simset.f.e.e2 get ]" );
		cmd( "set prof_min_msecs [ .simset.f.f.e2 get ]" );

		cmd( "destroytop $T" );

		if ( choice == 2 )	// Escape - revert previous values
		{
			sim_num = temp[ 1 ];
			seed = ( unsigned ) temp[ 2 ];
			max_step = temp[ 3 ];
			when_debug = temp[ 4 ];
			stack_info = temp[ 5 ];
			prof_min_msecs = temp[ 6 ];
			prof_obs_only = temp[ 7 ];
			prof_aggr_time = temp[ 8 ];
			no_ptr_chk = temp[ 9 ];
			parallel_disable = temp[ 10 ];
		}
		else
			// signal unsaved change if anything to be saved
			if ( temp[ 1 ] != sim_num || ( unsigned ) temp[ 2 ] != seed || temp[ 3 ] != max_step || temp[ 4 ] != when_debug || temp[ 5 ] != stack_info || temp[ 6 ] != prof_min_msecs || temp[ 7 ] != prof_obs_only || temp[ 8 ] != prof_aggr_time || temp[ 9 ] != no_ptr_chk || temp[ 10 ] != parallel_disable )
				unsaved_change( true );

		Tcl_UnlinkVar( inter, "sim_num" );
		Tcl_UnlinkVar( inter, "seed" );
		Tcl_UnlinkVar( inter, "max_step" );
		Tcl_UnlinkVar( inter, "stack_info" );
		Tcl_UnlinkVar( inter, "prof_min_msecs" );
		Tcl_UnlinkVar( inter, "prof_obs_only" );
		Tcl_UnlinkVar( inter, "prof_aggr_time" );
		Tcl_UnlinkVar( inter, "no_ptr_chk" );
		Tcl_UnlinkVar( inter, "parallel_disable" );

	break;


	// Move browser to Object pointed on the graphical model structure map
	case 24:

		if ( res_g == NULL )
			break;

		n = root->search( res_g );
		if ( n == NULL )
		{	// check if it is not a zero-instance object
			n = blueprint->search( res_g );
			if ( n != NULL )
				cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Cannot show no-instance object\" -detail \"All instances of '%s' were deleted.\nSelect another object or reload your configuration and try again.\"", res_g );

			break;
		}

		if ( n != r )
		{
			redrawRoot = redrawStruc = true;		// force browser/structure redraw
			cmd( "set listfocus 1; set itemfocus 0" ); // point for first var in listbox
		}

		choice = 0;
		return n;


	// Enter the analysis of results module for Monte Carlo analysis
	case 12:
		// accept analysis after run only if MC data was just produced
		if ( actual_steps > 0 && res_list.size( ) <= 1 )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid data for Monte Carlo analysis\" -detail \"Last simulation run did not produce adequate data to perform a Monte Carlo experiment analysis.\n\nPlease reload or unload your configuration and select the appropriate results files, or execute a multi-run configuration before using this option.\"" );
			break;
		}

		// check if MC results were not just created
		if ( res_list.size( ) > 1 )
		{
			cmd( "set answer [ ttk::messageBox -parent . -type yesnocancel -icon question -default yes -title \"Results Available\" -message \"Use set of results last created?\" -detail \"A set of results files was previously created and can be used to perform the Monte Carlo experiment analysis.\n\nAny configuration or results not saved will be discarded.\n\nPress 'Yes' to confirm, 'No' to select a different set of files, or 'Cancel' to abort.\" ]; switch -- $answer { yes { set choice 1 } no { set choice 0 } cancel { set choice 2 } }" );

			if ( choice == 2 )
				break;

			if ( choice == 0 )
				res_list.clear( );
		}
		else
			if ( ! discard_change( ) )		// check for unsaved configuration changes
				break;

		// remove existing results from memory before proceeding
		if ( ! open_configuration( r, true ) )
		{
			unload_configuration( true );
			r = root;
		}

		analysis( true );

	break;

	// Enter the analysis of results module
	case 26:

		analysis( false );

	break;


	// Change Equation File from which to take the code to show
	case 28:

		if ( ! struct_loaded )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create one before trying to change the equation file.\"" );
			break;
		}

		cmd( "set res %s", equation_name );

		cmd( "set res1 [ file tail [ tk_getOpenFile -parent . -title \"Select New Equation File\" -initialfile \"$res\" -initialdir \"%s\" -filetypes { { {LSD equation files} {.cpp} } { {All files} {*} } } ] ]", exec_path );
		cmd( "if [ fn_spaces \"$res1\" . ] { set res1 \"\" } { set res1 [ file tail $res1 ] }" );

		lab1 = get_str( "res1" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%999s", equation_name );

		unsaved_change( true );		// signal unsaved change

	break;


	// Shortcut to show equation window
	case 29:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );

		show_eq( lab_old, "." );

	break;


	// Show variables to be saved
	case 39:

		i = 0;
		count_save( root, &i );
		if ( i == 0 )
			plog( " \nNo variable or parameter saved." );
		else
		{
			plog( "\n\nVariables and parameters saved (%d):\n", i );
			show_save( root );
		}

	break;


	// Show variables to be observed
	case 42:

		plog( "\n\nVariables and parameters containing results:\n" );
		lcount = 0;
		show_observe( root );
		if ( lcount == 0 )
			plog( "(none)\n" );

	break;


	// Show variables to be initialized
	case 49:

		plog( "\n\nVariables and parameters relevant to initialize:\n" );
		lcount = 0;
		show_initial( root );
		if ( lcount == 0 )
			plog( "(none)\n" );

	break;


	// Show variables to be plot
	case 84:

		plog( "\n\nVariables and parameters to plot in run time:\n" );
		lcount = 0;
		show_plot( root );
		if ( lcount == 0 )
			plog( "(none)\n" );

	break;


	// Show elements to debug and watch
	case 85:

		plog( "\n\nVariables and parameters to debug and watch:\n" );
		lcount = 0;
		show_debug( root );
		if ( lcount == 0 )
			plog( "(none)\n" );

	break;


	// Show variables to parallelize
	case 86:

		plog( "\n\nMulti-object variables to run in parallel:\n" );
		lcount = 0;
		show_parallel( root );
		if ( lcount == 0 )
			plog( "(none)\n" );

	break;


	// Show variables with special updating
	case 97:

		plog( "\n\nVariables with special updating scheme:\n" );
		lcount = 0;
		show_special_updat( root );
		if ( lcount == 0 )
			plog( "(none)\n" );

	break;


	// elements/objects in configuration but unused in equation file
	case 56:

		// read the lists of variables/functions, parameters and objects in model program
		// from disk, if needed, or just update the missing elements lists
		cmd( "if { [ llength $unusVar ] == 0 || [ llength $unusFun ] == 0 || [ llength $unusPar ] == 0 || [ llength $unusObj ] == 0 } { read_elem_file %s } { upd_unus_elem }", exec_path );

		plog( "\n\nElements/objects apparently unused/missing in equation file(s):\n" );

		cmd( "foreach var $unusVar { plog \"Variable :\t\"; plog \"$var\n\" highlight }" );
		cmd( "foreach fun $unusFun { plog \"Function :\t\"; plog \"$fun\n\" highlight }" );
		cmd( "foreach par $unusPar { plog \"Parameter:\t\"; plog \"$par\n\" highlight }" );
		cmd( "foreach obj $unusObj { plog \"Object	 :\t\"; plog \"$obj\n\" highlight }" );

		cmd( "set res [ expr { [ llength $unusVar ] + [ llength $unusFun ] + [ llength $unusPar ] + [ llength $unusObj ] } ]" );
		if ( get_int( "res" ) == 0 )
			plog( "(none)\n" );

	break;


	// Remove all the save flags
	case 30:

		cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Remove save flags?\" -detail \"Confirm the removal of all saving information. No data will be saved.\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );

		if ( choice == 1 )
		{
			clean_save( root );
			unsaved_change( true );				// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw
		}

	break;


	// Remove all the plot flags
	case 31:

		cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Remove plot flags?\" -detail \"Confirm the removal of all run-time plot information. No data will be plotted during run time.\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );

		if ( choice == 1 )
		{
			clean_plot( root );
			unsaved_change( true );				// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw
		}

	break;


	// Remove all the debug and watch flags
	case 27:

		cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Remove debug and watch flags?\" -detail \"Confirm the removal of all debugging and watching information. Debugger will not stop in any variable update, access or write.\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );

		if ( choice == 1 )
		{
			clean_debug( root );
			unsaved_change( true );				// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw
		}

	break;


	// Remove all the parallel flags
	case 87:

		cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Remove parallel flags?\" -detail \"Confirm the removal of all parallel processing information. No parallelization will be performed.\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );

		if ( choice == 1 )
		{
			clean_parallel( root );
			unsaved_change( true );				// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw
		}

	break;


	// Changes the number of instances of only the Object type shown
	// in the browser or the pointed object (defined in tcl $vname)
	case 33:

		// check if current or pointed object and save current if needed
		lab1 = get_str( "useCurrObj" );
		if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
		{
			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			n = root->search( lab_old );	// set pointer to $vname
			if ( n == NULL )
				break;
			cur2 = r;
			r = n;
		}
		else
			cur2 = NULL;

		if ( r->up == NULL )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Cannot create instances of 'Root' object\" -detail \"Consider, if necessary, to add a new object here and moving all descendants of 'Root' to it. this new object can, then, be multiplied in many instances.\"" );
			goto endinst;
		}

		skip_next_obj( r, &num );
		cmd( "set num %d", num );
		cmd( "set cfrom 1" );

		cmd( "set T .numinst" );
		cmd( "newtop $T \"Number of Instances\" { set choice 2 }" );

		cmd( "ttk::frame $T.l" );

		cmd( "ttk::label $T.l.l1 -text \"Object:\"" );
		cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
		cmd( "pack $T.l.l1 $T.l.l2 -side left" );

		cmd( "ttk::frame $T.e" );

		cmd( "ttk::frame $T.e.e" );
		cmd( "ttk::label $T.e.e.l -text \"Number of instances\"" );
		cmd( "ttk::spinbox $T.e.e.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set num %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( "pack $T.e.e.l $T.e.e.e -side left -padx 2" );

		cmd( "ttk::label $T.e.l -text \"(all groups of this object will be affected)\"" );
		cmd( "pack $T.e.e $T.e.l" );

		cmd( "ttk::frame $T.cp" );
		cmd( "ttk::label $T.cp.l -text \"Copy from instance\"" );
		cmd( "ttk::spinbox $T.cp.e -width 5 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= %d } { set cfrom %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cfrom; return 0 } } -invalidcommand { bell } -justify center", num, num );
		cmd( "ttk::button $T.cp.compute -width $butWid -text Compute -command { set choice 3; .numinst.cp.e selection range 0 end; focus .numinst.cp.e }" );
		cmd( "pack $T.cp.l $T.cp.e $T.cp.compute -side left -padx 2" );

		cmd( "pack $T.l $T.e $T.cp -pady 5 -padx 5" );

		cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menudata_objn.html#this } { set choice 2 }" );
		cmd( "bind $T.e.e.e <Return> { set choice 1 }" );

		cmd( "showtop $T" );
		cmd( "mousewarpto $T.b.ok" );

		i = 1;

		objec_num:

		cmd( "write_any $T.e.e.e $num" );
		cmd( "write_any $T.cp.e $cfrom" );

		if ( i == 1 )
		{
			cmd( "$T.e.e.e selection range 0 end" );
			cmd( "focus $T.e.e.e" );
			i = 0;
		}

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set num [ $T.e.e.e get ]" );
		cmd( "set cfrom [ $T.cp.e get ]" );

		if ( choice == 3 )
		{
			k = compute_copyfrom( r, ".numinst" );
			if ( k > 0 )
				cmd( "set cfrom %d", k );

			goto objec_num;
		}

		cmd( "destroytop $T" );

		if ( choice == 2 )
			goto endinst;

		k = get_int( "cfrom" );
		num = get_int( "num" );
		for ( i = 0, cur = r->up; cur != NULL; ++i, cur = cur->up );

		chg_obj_num( &r, num, i, NULL, k );

		unsaved_change( true );				// signal unsaved change
		redrawRoot = redrawStruc = true;	// force browser/structure redraw

		endinst:

		if ( cur2 != NULL )					// restore original current object
			r = cur2;

	break;


	// Browse through the model instance by instance
	case 34:

		// check if current or pointed object and save current if needed
		lab1 = get_str( "useCurrObj" );
		if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
		{
			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			n = root->search( lab_old );		// set pointer to $vname
			if ( n == NULL )
				break;
			cur2 = r;
			r = n;
		}
		else
			cur2 = NULL;

		deb( r, NULL, NULL, &fake );

		if ( cur2 != NULL )			// restore original current object
			r = cur2;

	break;


	// Create model report
	case 36:

		report( root );

	break;


	// See model report
	case 44:

		show_report( "." );

	break;


	// Save result
	case 37:

		choice = 0;
		if ( actual_steps == 0 )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Simulation not run, nothing to save\" -detail \"Select menu option Run>Run before using this option.\"" );
			break;
		}

		Tcl_LinkVar( inter, "docsv", ( char * ) & docsv, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "dozip", ( char * ) & dozip, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "saveConf", ( char * ) & saveConf, TCL_LINK_BOOLEAN );

		time_t rawtime;
		time( &rawtime );
		struct tm *timeinfo;
		char ftime[80];
		timeinfo = localtime( &rawtime );
		strftime ( ftime, 80, "%Y%m%d-%H%M%S", timeinfo );

		cmd( "set lab \"%s_%s\"", strlen( simul_name ) > 0 ? simul_name : "results", ftime );

		// choose a name
		cmd( "newtop .n \"Save Results\" { set choice 2 }" );

		cmd( "ttk::frame .n.n" );
		cmd( "ttk::label .n.n.l -text \"Base name for file(s)\"" );
		cmd( "ttk::entry .n.n.e -width 30 -textvariable lab -justify center" );
		cmd( "pack .n.n.l .n.n.e" );

		cmd( "ttk::frame .n.do" );
		cmd( "ttk::checkbutton .n.do.zip -text \"Generate zipped results file\" -variable dozip" );
		cmd( "ttk::checkbutton .n.do.csv -text \"Comma-separated text format (.csv)\" -variable docsv" );
		cmd( "ttk::checkbutton .n.do.conf -text \"Save associated configuration\" -variable saveConf" );
		cmd( "pack .n.do.zip .n.do.csv .n.do.conf -anchor w" );

		cmd( "pack .n.n .n.do -padx 5 -pady 5" );

		cmd( "okcancel .n b { set choice 1 } { set choice 2 }" );
		cmd( "bind .n <KeyPress-Return> { set choice 1 }" );

		cmd( "showtop .n" );
		cmd( "mousewarpto .n.b.ok 0" );
		cmd( ".n.n.e selection range 0 end" );
		cmd( "focus .n.n.e" );

		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "if { [ string length lab ] == 0 } { set choice 2 }" );

		cmd( "destroytop .n" );

		Tcl_UnlinkVar( inter, "docsv" );
		Tcl_UnlinkVar( inter, "dozip" );
		Tcl_UnlinkVar( inter, "saveConf" );

		if ( choice == 2 )
			break;

		cmd( "focustop .log" );

		get_str( "lab", ch1, MAX_ELEM_LENGTH );

		if ( saveConf && strlen( simul_name ) > 0 )
		{
			if ( strlen( path ) == 0 )
			{
				cmd( "file copy -force %s.lsd %s.lsd", simul_name, ch1 );
				plog( "\nSaved configuration to file %s.lsd", ch1 );
			}
			else
			{
				cmd( "file copy -force %s/%s.lsd %s/%s.lsd", path, simul_name, path, ch1 );
				plog( "\nSaved configuration to file %s/%s.lsd", path, ch1 );
			}
		}

		if ( strlen( path ) == 0 )
			snprintf( out_file, MAX_PATH_LENGTH, "%s.%s", ch1, docsv ? "csv" : "res" );
		else
			snprintf( out_file, MAX_PATH_LENGTH, "%s/%s.%s", path, ch1, docsv ? "csv" : "res" );

		if ( dozip )
			strcatn( out_file, ".gz", MAX_PATH_LENGTH );

		plog( "\nSaving results to file %s... ", out_file );

		rf = new result( out_file, "wt", dozip, docsv );// create results file object
		rf->title( root, 1 );						// write header
		rf->data( root, 0, actual_steps );			// write all data
		delete rf;									// close file and delete object

		plog( "Done\n" );

		unsavedData = false;						// no unsaved simulation results

	break;


	// Help on lsd
	case 41:

		cmd( "LsdHelp LSD_quickhelp.html" );

	break;


	// Create automatically the elements descriptions
	case 43:

		if ( ! struct_loaded )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create one before trying to update descriptions.\"" );
			break;
		}

		cmd( "set answer [ttk::messageBox -parent . -message \"Replace existing descriptions?\" -detail \"Automatic data will replace any previous entered descriptions. Proceed?\" -type yesno -title Confirmation -icon question -default yes]" );
		cmd( "if { [ string compare $answer yes ] == 0 } { set choice 0 } { set choice 1 }" );

		if ( choice == 1 )
			break;

		cmd( "set x 1" );

		cmd( "newtop .warn \"Generate Descriptions\" { set choice 2 }" );

		cmd( "ttk::frame .warn.m" );
		cmd( "ttk::label .warn.m.l -text \"Elements to update\"" );

		cmd( "ttk::frame .warn.m.o -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .warn.m.o.var -text \"Only variables\" -variable x -value 1" );
		cmd( "ttk::radiobutton .warn.m.o.all -text \"All elements\" -variable x -value 2" );
		cmd( "pack .warn.m.o.var .warn.m.o.all -anchor w" );

		cmd( "pack .warn.m.l .warn.m.o" );

		cmd( "pack .warn.m -padx 5 -pady 5" );

		cmd( "okhelpcancel .warn b { set choice 1 } { LsdHelp menumodel.html#auto_docu } { set choice 2 }" );

		cmd( "showtop .warn" );
		cmd( "mousewarpto .warn.b.ok" );

		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .warn" );

		if ( choice == 2 )
			break;

		cmd( "set choice $x" );
		if ( choice == 1 )
			auto_document( NULL, "" );
		else
			auto_document( NULL, "ALL" );

		unsaved_change( true );		// signal unsaved change

	break;


	// Show the vars./pars. vname is using
	case 46:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab );

		scan_using_lab( lab, "." );

	break;


	// Show the equations where vname is used
	case 47:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab );

		scan_used_lab( lab, "." );

	break;


	// Find an element of the model
	case 50:

		if ( ! struct_loaded )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create one before trying to find elements.\"" );
			break;
		}

		cmd( "set bidi \"\"" );

		cmd( "newtop .srch \"Find Element\" { set choice 2 }" );

		cmd( "ttk::frame .srch.i" );
		cmd( "ttk::label .srch.i.l -text \"Element name\"" );
		cmd( "ttk::combobox .srch.i.e -width 20 -textvariable bidi -justify center -values $modElem" );
		cmd( "pack .srch.i.l .srch.i.e" );

		cmd( "ttk::label .srch.o -justify center -text \"(type the initial letters of the\nname, LSD will complete it)\"" );
		cmd( "pack .srch.i .srch.o -padx 5 -pady 5" );
		cmd( "pack .srch.i" );

		cmd( "okcancel .srch b { set choice 1 } { set choice 2 }" );

		cmd( "bind .srch.i.e <KeyPress-Return> { set choice 1; break }" );
		cmd( "bind .srch.i.e <KeyRelease> { \
				if { %%N < 256 && [ info exists modElem ] } { \
					set b [ .srch.i.e index insert ]; \
					set s [ .srch.i.e get ]; \
					set f [ lsearch -glob $modElem $s* ]; \
					if { $f !=-1 } { \
						set d [ lindex $modElem $f ]; \
						.srch.i.e delete 0 end; \
						.srch.i.e insert 0 $d; \
						.srch.i.e index $b; \
						.srch.i.e selection range $b end \
					} \
				} \
			}" );

		cmd( "showtop .srch" );
		cmd( "focus .srch.i.e" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .srch" );

		if ( choice == 2 )
			break;


	// Arrive here from the list of vars used (keep together with case 50!)
	case 55:

		cv = r->search_var( r, get_str( "bidi" ), true );
		if ( cv != NULL )
		{
			for ( i = 0, cv1 = cv->up->v; cv1 != cv; cv1 = cv1->next, ++i );
			cmd( "set listfocus 1; set itemfocus %d", i );
			redrawRoot = redrawStruc = true;			// request browser redraw
			choice = 0;
			return cv->up;
		}
		else
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Element not found\" -detail \"Check the spelling of the element name.\"" );

	break;


	// Restore configuration's equations in a new equation file
	case 52:
		/*
		Used to re-generate the equations used for the current configuration file
		*/

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to offload an equation file.\"" );
			break;
		}

		if ( ! strcmp( eq_file, lsd_eq_file ) )
		{
			cmd( "ttk::messageBox -parent . -title \"Offload Equations\" -icon info -message \"Nothing to do\" -detail \"There are no equations to be offloaded differing from the current equation file.\" -type ok" );
			break;
		}

		cmd( "set res1 fun_%s.cpp", simul_name );
		cmd( "set bah [ tk_getSaveFile -parent . -title \"Save Equation File\" -defaultextension \".cpp\" -initialfile $res1 -initialdir \"%s\" -filetypes { { {LSD equation files} {.cpp} } { {All files} {*} } } ]", exec_path );

		cmd( "if { [ string length $bah ] > 0 } { set choice 1; set res1 [ file tail $bah ] } { set choice 0 }" );
		if ( choice == 0 )
		  break;

		get_str( "res1", lab, MAX_PATH_LENGTH );
		if ( strlen( lab ) == 0 )
			break;

		if ( ( f = fopen( lab, "wb" ) ) != NULL )
		{
			fprintf( f, "%s", lsd_eq_file );
			fclose( f );
			cmd( "ttk::messageBox -parent . -title \"Offload Equations\" -icon info -message \"Equation file '$res1' created\" -detail \"You need to create a new LSD model to use these equations, replacing the name of the equation file in LMM with the command 'Model Compilation Options' (menu Model).\" -type ok" );
		}
		else
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '$res1' cannot be saved\" -detail \"Check if the file already exists and is set READ-ONLY, or try to save to a different location.\"" );

	break;


	// Compare equation files
	case 53:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to compare equation files.\"" );
			break;
		}

		if ( strlen( lsd_eq_file ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon Warning -title Warning -message \"No equations loaded\" -detail \"Please upload an equation file before trying to compare equation files.\"" );
			break;
		}

		snprintf( lab_old, 2 * MAX_PATH_LENGTH, "orig-eq_%s.tmp", simul_name);

		if ( ( f = fopen( lab_old, "wb" ) ) != NULL )
		{
			fprintf( f, "%s", lsd_eq_file );
			fclose( f );

			read_eq_filename( lab, MAX_PATH_LENGTH );
			cmd( "open_diff %s %s %s %s.lsd", lab, lab_old, equation_name, simul_name  );
		}
		else
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s' cannot be saved\" -detail \"Check if the file already exists and is set READ-ONLY.\"", lab_old );

	break;


	// Compare configuration files
	case 82:

		if ( ! struct_loaded || strlen( simul_name ) == 0 || strlen( struct_file ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to compare configuration files.\"" );
			break;
		}

		// make sure there is a path set
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		cmd( "set res1 [ tk_getOpenFile -parent . -title \"Select Configuration File to Compare to\" -initialdir \"$path\" -filetypes { { {LSD configuration files} {.lsd} } } ]" );
		cmd( "set res2 [ file tail $res1 ]" );
		cmd( "if [ fn_spaces \"$res1\" . ] { set res1 \"\"; set res2 \"\" }" );

		lab1 = get_str( "res1" );
		lab2 = get_str( "res2" );
		if ( lab1 == NULL || lab2 == NULL || strlen ( lab1 ) == 0 || strlen ( lab2 ) == 0 )
			break;

		f = fopen( lab1, "r" );
		if ( f == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Cannot open file\" -detail \"Error opening file '%s'.\"", lab2 );
			break;
		}
		fclose( f );

		cmd( "file copy -force -- $res1 ext-cfg.tmp" );
		cmd( "file copy -force -- %s int-cfg.tmp", struct_file );
		cmd( "open_diff ext-cfg.tmp int-cfg.tmp %s %s.lsd", lab2, simul_name );

	break;


	// Toggle ignore eq. file controls
	case 54:

		cmd( "set choice $ignore_eq_file" );
		ignore_eq_file = choice;
		plog( "\n%s equation file\n", ignore_eq_file ? "Ignoring" : "Not ignoring" );

	break;


	// Generate Latex code
	case 57:
	case 92:

		table = ( choice == 57 ) ? true : false;

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create LaTex code.\"" );
			break;
		}

		snprintf( out_file, MAX_PATH_LENGTH, "%s%s%s_%s.tex", strlen( path ) > 0 ? path : "", strlen( path ) > 0 ? "/" : "", table ? "table" : "href", simul_name );
		cmd( "set choice [ file exists %s ]", out_file );
		if ( choice == 1 )
		{
			cmd( "set answer [ ttk::messageBox -parent . -message \"File '%s' already exists\" -detail \"Please confirm overwriting it.\" -type okcancel -title Warning -icon warning -default ok ]", out_file );
			cmd( "if [ string equal $answer ok ] { set choice 0 } { set choice 1 }" );
			if ( choice == 1 )
				break;
		}

		if ( ( f = fopen( out_file, "wt" ) ) == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s' cannot be created\" -detail \"Check if the file already exists and is set READ-ONLY, or try to save to a different location.\"", out_file );
			break;
		}

		stop = false;
		cmd( "progressbox .ptex \"Creating LaTex\" \"LaTex code generation steps\" \"Step\" 6 { set stop true }" );

		tex_report_head( f, table );
		cmd( "prgboxupdate .ptex 1" );

		if ( stop )
			goto end_latex;

		tex_report_struct( root, f, table );
		cmd( "prgboxupdate .ptex 2" );

		if ( stop )
			goto end_latex;

		tex_report_observe( root, f, table );
		cmd( "prgboxupdate .ptex 3" );

		if ( stop )
			goto end_latex;

		tex_report_init( root, f, table );
		cmd( "prgboxupdate .ptex 4" );

		if ( stop )
			goto end_latex;

		tex_report_initall( root, f, table );
		cmd( "prgboxupdate .ptex 5" );

		if ( stop )
			goto end_latex;

		tex_report_end( f );
		cmd( "prgboxupdate .ptex 6" );

		end_latex:

		cmd( "destroytop .ptex" );

		fclose( f );

		if ( stop )
			remove( out_file );
		else
			plog( "\nLaTex code saved in file: %s\n", out_file );

	break;


	// Move variable up in the list box
	case 58:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );

		shift_var( -1, lab_old, r );

		unsaved_change( true );		// signal unsaved change
		redrawRoot = true;			// request browser redraw

	break;


	// Move variable down in the list box
	case 59:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );

		shift_var( 1, lab_old, r );

		unsaved_change( true );		// signal unsaved change
		redrawRoot = true;			// request browser redraw

	break;


	// Move object up in the list box
	case 60:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );

		shift_desc( -1, lab_old, r );

		unsaved_change( true );		// signal unsaved change
		redrawRoot = redrawStruc = true;	// request browser redraw

	break;


	// Move object down in the list box
	case 61:

		lab1 = get_str( "vname" );
		if ( lab1 == NULL || ! strcmp( lab1, "" ) )
			break;
		sscanf( lab1, "%99s", lab_old );

		shift_desc( 1, lab_old, r );

		unsaved_change( true );		// signal unsaved change
		redrawRoot = redrawStruc = true;	// request browser redraw

	break;


	// Sort current list box on the selected order
	case 94:
		cmd( "set choice $listfocus" );
		i = choice;
		cmd( "set choice $sort_order" );

		if ( sort_listbox( i, choice, r ) )
		{
			unsaved_change( true );		// signal unsaved change
			redrawRoot = true;			// request browser redraw
		}

	break;


	// Create parallel sensitivity analysis configuration
	case 62:

		if ( rsense != NULL )
		{
			if ( ! discard_change( false ) )	// unsaved configuration?
				break;

			varSA = num_sensitivity_variables( rsense );// number of variables to test
			plog( "\nNumber of elements for sensitivity analysis: %d", varSA );
			ptsSa = num_sensitivity_points( rsense );	// total number of points in sensitivity space
			plog( "\nSensitivity analysis space size: %ld", ptsSa );

			// Prevent running into too big sensitivity spaces (high computation times)
			if ( ptsSa > max( 10, MAX_SENS_POINTS / 10 ) )
				// ask user before proceeding
				if ( sensitivity_too_large( ptsSa ) )
					break;

			for ( i = 1, cs = rsense; cs!=NULL; cs = cs->next )
				i *= cs->nvalues;
			cur = root->b->head;
			root->add_n_objects2( cur->label, i - 1, cur );

			plog( "\nUpdating configuration... " );
			cmd( "focustop .log" );

			sensitivity_parallel( cur, rsense );

			plog( "Done\n" );

			unsaved_change( true );				// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw

			cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"Structure changed\" -detail \"LSD has changed your model structure, replicating the entire model for each sensitivity configuration. If you want to preserve your original configuration file, save your new configuration using a different name BEFORE running the model.\"" );
		}
		else
			sensitivity_undefined( );			// throw error

	break;


	// Create batch sensitivity analysis configuration
	case 63:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
			break;
		}

		if ( rsense != NULL )
		{
			if ( ! discard_change( false ) )	// unsaved configuration?
				break;

			varSA = num_sensitivity_variables( rsense );// number of variables to test
			plog( "\nNumber of elements for sensitivity analysis: %d", varSA );
			ptsSa = num_sensitivity_points( rsense );	// total number of points in sensitivity space
			plog( "\nSensitivity analysis space size: %ld", ptsSa );

			// Prevent running into too big sensitivity spaces (high computation times)
			if ( ptsSa > MAX_SENS_POINTS )
				// ask user before proceeding
				if ( sensitivity_too_large( ptsSa ) )
					break;

			// detect the need of a new save path and create it if required
			if ( need_res_dir( path, simul_name, path_sens, MAX_PATH_LENGTH ) )
				create_res_dir( path_sens );

			// ask to clean existing files before proceeding if required
			if ( check_res_dir( path_sens, simul_name ) && sensitivity_clean_dir( path_sens ) )
				clean_res_dir( path_sens, simul_name );

			// save the current object & cursor position for quick reload
			save_pos( r );
			findexSens = 1;

			// create a design of experiment (DoE) for the sensitivity data
			cmd( "focustop .log" );

			stop = false;
			cmd( "progressbox .psa \"Creating DoE\" \"Creating configuration files\" \"File\"  %d { set stop true }", ptsSa );

			sensitivity_sequential( &findexSens, rsense, 1.0, path_sens );

			cmd( "destroytop .psa" );

			plog( "\nSensitivity analysis configurations produced: %d", findexSens - 1 );

			// if succeeded, explain user how to proceed
			if ( ! stop )
				sensitivity_created( path_sens, clean_file( simul_name ), 1 );
			else
				findexSens = 0;					// don't consider for appending

			// now reload the previously existing configuration
			if ( ! load_prev_configuration( ) )
			{
				choice = 0;
				return root;
			}

			// restore pointed object and variable
			r = restore_pos( r );
		}
		else
			sensitivity_undefined( );			// throw error

	break;


	// Create Monte Carlo (MC) random sensitivity analysis sampling configuration (over user selected point values)
	case 71:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
			break;
		}

		if ( rsense != NULL )
		{
			if ( ! discard_change( false ) )	// unsaved configuration?
				break;

			varSA = num_sensitivity_variables( rsense );// number of variables to test
			plog( "\nNumber of elements for sensitivity analysis: %d", varSA );
			maxMC = num_sensitivity_points( rsense );	// total number of points in sensitivity space
			plog( "\nSensitivity analysis space size: %ld", maxMC );

			// get the number of Monte Carlo samples to produce
			fracMC = 10;
			Tcl_LinkVar( inter, "fracMC", ( char * ) & fracMC, TCL_LINK_DOUBLE );

			// detect the need of a new save path
			subDir = need_res_dir( path, simul_name, path_sens, MAX_PATH_LENGTH );

			cmd( "newtop .s \"MC Point Sampling\" { set choice 2 }" );

			cmd( "ttk::frame .s.p" );
			cmd( "ttk::label .s.p.l -text \"Output path\"" );
			cmd( "ttk::label .s.p.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", path_sens );
			cmd( "pack .s.p.l .s.p.w" );

			cmd( "ttk::frame .s.i" );
			cmd( "ttk::label .s.i.l -justify center -text \"Monte Carlo sample size as\n%% of sensitivity space size\n(0 to 100)\"" );
			cmd( "ttk::entry .s.i.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] && $n > 0 && $n <= 100 } { set fracMC %%P; return 1 } { %%W delete 0 end; %%W insert 0 $fracMC; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".s.i.e insert 0 $fracMC" );
			cmd( "pack .s.i.l .s.i.e" );

			cmd( "ttk::label .s.w -text \"(large samples are not recommended)\"" );

			cmd( "pack .s.p .s.i .s.w -padx 5 -pady 5" );

			cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menudata_sa.html#mcpoint } { set choice 2 }" );

			cmd( "showtop .s" );
			cmd( "mousewarpto .s.b.ok 0" );
			cmd( ".s.i.e selection range 0 end" );
			cmd( "focus .s.i.e" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set fracMC [ .s.i.e get ]" );
			cmd( "destroytop .s" );
			Tcl_UnlinkVar( inter, "fracMC" );

			if ( choice == 2 )
				break;

			// Check if number is valid
			fracMC /= 100.0;
			if ( ( fracMC * maxMC ) < 1 || fracMC > 1.0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid sample size\" -detail \"Invalid Monte Carlo sample size to perform the sensitivity analysis. Select a number between 0%% and 100%% that produces at least one sample (in average).\"" );
				choice = 0;
				break;
			}

			// Prevent running into too big sensitivity space samples (high computation times)
			if ( ( fracMC * maxMC ) > MAX_SENS_POINTS )
				// ask user before proceeding
				if ( sensitivity_too_large( ( long ) ( fracMC * maxMC ) ) )
					break;

			// create a new save path if required
			if ( subDir )
				create_res_dir( path_sens );

			// ask to clean existing files before proceeding if required
			if ( check_res_dir( path_sens, simul_name ) && sensitivity_clean_dir( path_sens ) )
				clean_res_dir( path_sens, simul_name );

			// save the current object & cursor position for quick reload
			save_pos( r );

			plog( "\nTarget sensitivity analysis sample size: %ld (%.1f%%)", ( long ) ( fracMC * maxMC ), 100 * fracMC );
			findexSens = 1;

			// create a design of experiment (DoE) for the sensitivity data
			cmd( "focustop .log" );

			stop = false;
			cmd( "progressbox .psa \"Creating DoE\" \"Creating configuration files\" \"File\" %ld { set stop true }", ( long ) ( fracMC * maxMC ) );

			init_random( seed );				// reset random number generator
			sensitivity_sequential( &findexSens, rsense, fracMC, path_sens );

			cmd( "destroytop .psa" );

			plog( "\nSensitivity analysis configurations produced: %d", findexSens - 1 );

			// if succeeded, explain user how to proceed
			if ( ! stop )
				sensitivity_created( path_sens, clean_file( simul_name ), 1 );
			else
				findexSens = 0;					// don't consider for appending

			// now reload the previously existing configuration
			if ( ! load_prev_configuration( ) )
			{
				choice = 0;
				return root;
			}

			// restore pointed object and variable
			r = restore_pos( r );
		}
		else
			sensitivity_undefined( );			// throw error

	break;


	// Create Near Orthogonal Latin Hypercube (NOLH) sensitivity analysis sampling configuration
	case 72:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
			break;
		}

		if ( rsense != NULL )
		{
			if ( ! discard_change( false ) )	// unsaved configuration?
				break;

			varSA = num_sensitivity_variables( rsense );	// number of variables to test
			plog( "\nNumber of elements for sensitivity analysis: %d", varSA );
			lab1 = NOLH_valid_tables( varSA, ch, 2 * MAX_LINE_SIZE );

			// detect the need of a new save path
			subDir = need_res_dir( path, simul_name, path_sens, MAX_PATH_LENGTH );

			cmd( "set extdoe 0" );	// flag for using external DoE file
			cmd( "set NOLHfile \"NOLH.csv\"" );
			cmd( "set doeList [list %s]", lab1 );
			cmd( "set doesize [ lindex $doeList 0 ]" );	// minimum Doe size
			cmd( "set doeext 0" );	// flag for using extended number of samples

			cmd( "newtop .s \"NOLH Sampling\" { set choice 2 }" );

			cmd( "ttk::frame .s.p" );
			cmd( "ttk::label .s.p.l -text \"Output path\"" );
			cmd( "ttk::label .s.p.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", path_sens );
			cmd( "pack .s.p.l .s.p.w" );

			cmd( "ttk::frame .s.o" );
			cmd( "ttk::label .s.o.l1 -text \"NOLH table\"" );
			cmd( "ttk::combobox .s.o.c -width 15 -values $doeList -justify center -validate focusout -validatecommand { set n %%P; if { $n in $doeList } { set doesize %%P; return 1 } { %%W delete 0 end; %%W insert 0 $doesize; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any .s.o.c $doesize" );
			cmd( "ttk::label .s.o.l2 -text \"(factors \u00D7 samples \u00D7 ext. samples)\"" );
			cmd( "pack .s.o.l1 .s.o.c .s.o.l2" );

			cmd( "ttk::checkbutton .s.e -text \"Extended number of samples\" -variable doeext" );
			if( varSA > 22 )
				cmd( ".s.e configure -state disabled" );

			cmd( "ttk::checkbutton .s.d -text \"External design file\" -variable extdoe -command { if { $extdoe == 1 } { .s.o.c configure -state disabled; .s.e configure -state disabled; .s.i.e configure -state normal; .s.i.e selection range 0 end; focus .s.i.e } { .s.o.c configure -state normal; .s.e configure -state normal; .s.i.e configure -state disabled } }" );

			cmd( "ttk::frame .s.i" );
			cmd( "ttk::label .s.i.l -text \"Design file name\"" );
			cmd( "ttk::entry .s.i.e -width 20 -justify center -textvariable NOLHfile -state disabled" );
			cmd( "ttk::label .s.i.w -justify center -text \"(file must be in the same folder\nas the configuration file; CSV\nformat with NO empty lines)\"" );
			cmd( "pack .s.i.l .s.i.e .s.i.w" );

			cmd( "pack .s.p .s.o .s.e .s.d .s.i -padx 5 -pady 5" );

			cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menudata_sa.html#nolh } { set choice 2 }" );

			cmd( "showtop .s" );
			cmd( "mousewarpto .s.b.ok" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "if { [ .s.o.c get ] in $doeList } { set doesize [ .s.o.c get ] } { bell }" );
			cmd( "destroytop .s" );

			if ( choice == 2 )
				break;

			// create a new save path if required
			if ( subDir )
				create_res_dir( path_sens );

			// ask to clean existing files before proceeding if required
			if ( check_res_dir( path_sens, simul_name ) && sensitivity_clean_dir( path_sens ) )
				clean_res_dir( path_sens, simul_name );

			if ( ! get_bool( "extdoe" ) )
				strcpy( NOLHfile, "" );
			else
				get_str( "NOLHfile", NOLHfile, MAX_PATH_LENGTH );

			num = ( sscanf( get_str( "doesize" ), "%d\u00D7", & j ) > 0 ) ? j : 0;

			// adjust an NOLH design of experiment (DoE) for the sensitivity data
			doe = new design( rsense, 1, NOLHfile, path_sens, 1, get_bool( "doeext" ) ? -1 : 0, num );

			if ( doe -> n == 0 )					// DoE configuration is not ok?
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration error\" -detail \"It was not possible to create a Non Orthogonal Latin Hypercube (NOLH) Design of Experiment (DoE) for the current sensitivity configuration. If the number of variables (factors) is large than 29, an external NOLH has to be provided in the file NOLH.csv (empty lines not allowed).\"" );

				if ( subDir )
					cmd( "catch { file delete -force \"%s\" }", path_sens );

				delete doe;
				break;
			}

			// Prevent running into too big sensitivity space samples (high computation times)
			if ( doe -> n > MAX_SENS_POINTS )
				// ask user before proceeding
				if ( sensitivity_too_large( doe -> n ) )
				{
					if ( subDir )
						cmd( "catch { file delete -force \"%s\" }", path_sens );

					delete doe;
					break;
				}

			// save the current object & cursor position for quick reload
			save_pos( r );
			findexSens = 1;

			// create a design of experiment (DoE) for the sensitivity data
			cmd( "focustop .log" );

			sensitivity_doe( &findexSens, doe, path_sens );
			delete doe;

			// now reload the previously existing configuration
			if ( ! load_prev_configuration( ) )
			{
				choice = 0;
				return root;
			}

			// restore pointed object and variable
			r = restore_pos( r );

			if ( findexSens > 0 )
			{
				cmd( "set answer [ ttk::messageBox -parent . -title Confirmation -icon question -type yesno -default yes -message \"Create out-of-main-sample set of samples?\" -detail \"An out-of-sample set allows for better meta-model selection and fit-quality evaluation.\n\nPress 'Yes' to create a Monte Carlo sample now or 'No' otherwise.\" ]" );
				cmd( "switch $answer { yes { set choice 80 } no { set choice 0 } }" );

				if ( choice != 0 )
					return r;
			}
		}
		else
			sensitivity_undefined( );			// throw error

	break;


	// Create Monte Carlo (MC) random sensitivity analysis sampling configuration (over selected range values)
	case 80:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
			break;
		}

		if ( rsense != NULL )
		{
			if ( ! discard_change( false ) )	// unsaved configuration?
				break;

			varSA = num_sensitivity_variables( rsense );	// number of variables to test
			plog( "\nNumber of elements for sensitivity analysis: %d", varSA );

			// get the number of Monte Carlo samples to produce
			sizMC = 10;
			Tcl_LinkVar( inter, "sizMC", ( char * ) & sizMC, TCL_LINK_INT );

			// detect the need of a new save path
			subDir = need_res_dir( path, simul_name, path_sens, MAX_PATH_LENGTH );

			cmd( "set applst 1" );	// flag for appending to existing configuration files

			cmd( "newtop .s \"MC Range Sampling\" { set choice 2 }" );

			cmd( "ttk::frame .s.p" );
			cmd( "ttk::label .s.p.l -text \"Output path\"" );
			cmd( "ttk::label .s.p.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", path_sens );
			cmd( "pack .s.p.l .s.p.w" );

			cmd( "ttk::frame .s.i" );
			cmd( "ttk::label .s.i.l -justify center -text \"Monte Carlo sample size\nas number of samples\"" );
			cmd( "ttk::spinbox .s.i.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set sizMC %%P; return 1 } { %%W delete 0 end; %%W insert 0 $sizMC; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".s.i.e insert 0 $sizMC" );
			cmd( "pack .s.i.l .s.i.e" );

			cmd( "ttk::checkbutton .s.c -text \"Append to existing configuration files\" -variable applst -state %s", findexSens > 1 ? "normal" : "disabled" );
			cmd( "pack .s.p .s.i .s.c -padx 5 -pady 5" );

			cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menudata_sa.html#mcrange } { set choice 2 }" );

			cmd( "showtop .s" );
			cmd( "mousewarpto .s.b.ok 0" );
			cmd( ".s.i.e selection range 0 end" );
			cmd( "focus .s.i.e" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set sizMC [ .s.i.e get ]" );
			cmd( "destroytop .s" );
			Tcl_UnlinkVar( inter, "sizMC" );

			if ( choice == 2 )
				break;

			// Check if number is valid
			if ( sizMC < 1 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid sample size\" -detail \"Invalid Monte Carlo sample size to perform the sensitivity analysis. Select at least one sample.\"" );
				choice = 0;
				break;
			}

			// Prevent running into too big sensitivity space samples (high computation times)
			if ( sizMC > MAX_SENS_POINTS )
				// ask user before proceeding
				if ( sensitivity_too_large( ( long ) sizMC ) )
					break;

			if ( findexSens < 1 || ( findexSens > 1 && ! get_bool( "applst" ) ) )
				findexSens = 1;

			// create a new save path if required
			if ( subDir )
				create_res_dir( path_sens );

			// ask to clean existing files before proceeding if required
			if ( findexSens == 1 && check_res_dir( path_sens, simul_name ) && sensitivity_clean_dir( path_sens ) )
				clean_res_dir( path_sens, simul_name );

			// save the current object & cursor position for quick reload
			save_pos( r );

			// check if design file numbering should pick-up from previously generated files
			// adjust a design of experiment (DoE) for the sensitivity data
			doe = new design( rsense, 2, "", path_sens, findexSens, sizMC );
			sensitivity_doe( &findexSens, doe, path_sens );
			delete doe;

			// now reload the previously existing configuration
			if ( ! load_prev_configuration( ) )
			{
				choice = 0;
				return root;
			}

			// restore pointed object and variable
			r = restore_pos( r );
		}
		else
			sensitivity_undefined( );			// throw error

	break;


	// Create Elementary Effects (EE) sensitivity analysis sampling configuration (over selected range values)
	case 81:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
			break;
		}

		if ( rsense != NULL )
		{
			if ( ! discard_change( false ) )	// unsaved configuration?
				break;

			varSA = num_sensitivity_variables( rsense );	// number of variables to test
			plog( "\nNumber of elements for sensitivity analysis: %d", varSA );

			// get the number of Monte Carlo samples to produce
			int nLevels = 4, jumpSz = 2, nTraj = 10, nSampl = 100;
			Tcl_LinkVar( inter, "varSA", ( char * ) & varSA, TCL_LINK_INT );
			Tcl_LinkVar( inter, "nLevels", ( char * ) & nLevels, TCL_LINK_INT );
			Tcl_LinkVar( inter, "jumpSz", ( char * ) & jumpSz, TCL_LINK_INT );
			Tcl_LinkVar( inter, "nTraj", ( char * ) & nTraj, TCL_LINK_INT );
			Tcl_LinkVar( inter, "nSampl", ( char * ) & nSampl, TCL_LINK_INT );

			// detect the need of a new save path
			subDir = need_res_dir( path, simul_name, path_sens, MAX_PATH_LENGTH );

			cmd( "newtop .s \"Elementary Effects Sampling\" { set choice 2 }" );

			cmd( "ttk::frame .s.o" );
			cmd( "ttk::label .s.o.l -text \"Output path\"" );
			cmd( "ttk::label .s.o.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", path_sens );
			cmd( "pack .s.o.l .s.o.w" );

			cmd( "ttk::frame .s.i" );
			cmd( "ttk::label .s.i.l1 -text \"Number of trajectories (r)\"" );
			cmd( "ttk::spinbox .s.i.e1 -width 5 -from 1 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set nTraj %%P; return 1 } { %%W delete 0 end; %%W insert 0 $nTraj; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".s.i.e1 insert 0 $nTraj" );
			cmd( "ttk::label .s.i.l2 -text \"([ expr { $varSA + 1 } ]\u00D7r samples to create)\"" );
			cmd( "pack .s.i.l1 .s.i.e1 .s.i.l2" );

			cmd( "ttk::frame .s.p" );
			cmd( "ttk::label .s.p.l1 -text \"Trajectories pool size (M)\"" );
			cmd( "ttk::spinbox .s.p.e2 -width 5 -from 1 -to 999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set nSampl %%P; return 1 } { %%W delete 0 end; %%W insert 0 $nSampl; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".s.p.e2 insert 0 $nSampl" );
			cmd( "ttk::label .s.p.l2 -text \"(M > r enables optimization)\"" );
			cmd( "pack .s.p.l1 .s.p.e2 .s.p.l2" );

			cmd( "ttk::frame .s.l" );
			cmd( "ttk::label .s.l.l1 -text \"Number of levels (p)\"" );
			cmd( "ttk::spinbox .s.l.e3 -width 5 -from 1 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set nLevels %%P; return 1 } { %%W delete 0 end; %%W insert 0 $nLevels; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".s.l.e3 insert 0 $nLevels" );
			cmd( "ttk::label .s.l.l2 -text \"(must be even)\"" );
			cmd( "pack .s.l.l1 .s.l.e3 .s.l.l2" );

			cmd( "ttk::frame .s.j" );
			cmd( "ttk::label .s.j.l1 -text \"Jump size\"" );
			cmd( "ttk::spinbox .s.j.e4 -width 5 -from 1 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set jumpSz %%P; return 1 } { %%W delete 0 end; %%W insert 0 $jumpSz; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".s.j.e4 insert 0 $jumpSz" );
			cmd( "ttk::label .s.j.l2 -text \"( \u0394\u00D7(p - 1) )\"" );
			cmd( "pack .s.j.l1 .s.j.e4 .s.j.l2" );

			cmd( "ttk::label .s.t -justify center -text \"(for details on setting Elementary Effects\nsampling parameters see Morris (1991),\nCampolongo et al. (2007) and Ruano et al. (2012))\"" );

			cmd( "pack .s.o .s.i .s.p .s.l .s.j .s.t -padx 5 -pady 5" );

			cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menudata_sa.html#ee } { set choice 2 }" );

			cmd( "showtop .s" );
			cmd( "mousewarpto .s.b.ok 0" );
			cmd( ".s.i.e1 selection range 0 end" );
			cmd( "focus .s.i.e1" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set nTraj [ .s.i.e1 get ]" );
			cmd( "set nSampl [ .s.p.e2 get ]" );
			cmd( "set nLevels [ .s.l.e3 get ]" );
			cmd( "set jumpSz [ .s.j.e4 get ]" );
			cmd( "destroytop .s" );
			Tcl_UnlinkVar( inter, "varSA" );
			Tcl_UnlinkVar( inter, "nLevels" );
			Tcl_UnlinkVar( inter, "jumpSz" );
			Tcl_UnlinkVar( inter, "nTraj" );
			Tcl_UnlinkVar( inter, "nSampl" );

			if ( choice == 2 )
				break;

			// Check if numbers are valid
			if ( nLevels < 2 || nLevels % 2 != 0 || nTraj < 2 || nSampl < nTraj || jumpSz < 1 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid configuration\" -detail \"Invalid Elementary Effects configuration to perform the sensitivity analysis. Check Morris (1991) and Campolongo et al. (2007) for details.\"" );
				choice = 0;
				break;
			}

			// Prevent running into too big sensitivity space samples (high computation times)
			if ( nTraj * ( varSA + 1 ) > MAX_SENS_POINTS )
				// ask user before proceeding
				if ( sensitivity_too_large( ( long ) ( nTraj * ( varSA + 1 ) ) ) )
					break;

			// create a new save path if required
			if ( subDir )
				create_res_dir( path_sens );

			// ask to clean existing files before proceeding if required
			if ( check_res_dir( path_sens, simul_name ) && sensitivity_clean_dir( path_sens ) )
				clean_res_dir( path_sens, simul_name );

			// save the current object & cursor position for quick reload
			save_pos( r );
			findexSens = 1;

			// adjust a design of experiment (DoE) for the sensitivity data
			doe = new design( rsense, 3, "", path_sens, findexSens, nSampl, nLevels, jumpSz, nTraj );
			sensitivity_doe( &findexSens, doe, path_sens );
			delete doe;

			// now reload the previously existing configuration
			if ( ! load_prev_configuration( ) )
			{
				choice = 0;
				return root;
			}

			// restore pointed object and variable
			r = restore_pos( r );
		}
		else
			sensitivity_undefined( );			// throw error

	break;


	// Load a sensitivity analysis configuration
	case 64:

		// check a model is already loaded
		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to load a sensitivity analysis configuration.\"" );
			break;
		}

		// check for existing sensitivity data loaded
		if ( rsense != NULL )
		{
			cmd( "set answer [ ttk::messageBox -parent . -type okcancel -icon warning -default ok -title Warning -message \"Sensitivity data already loaded\" -detail \"Press 'OK' if you want to discard the existing data before loading a new sensitivity configuration.\" ]; switch -- $answer { ok { set choice 1 } cancel { set choice 2 } }" );
			if ( choice == 2 )
				break;

			// empty sensitivity data
			empty_sensitivity( rsense );			// discard read data
			rsense = NULL;
			unsavedSense = false;					// nothing to save
			findexSens = 0;
		}

		// set default name and path to conf. file folder
		cmd( "set res \"%s\"", simul_name );
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		// open dialog box to get file name & folder
		cmd( " set bah [ tk_getOpenFile -parent . -title \"Load Sensitivity Analysis File\" -defaultextension \".sa\" -initialfile \"$res\" -initialdir \"$path\"  -filetypes { { {Sensitivity analysis files} {.sa} } } ]" );
		cmd( "if { [ string length $bah ] > 0 && ! [ fn_spaces \"$bah\" . ] } { set res $bah; set path [ file dirname $res ]; set res [ file tail $res ]; set last [ expr { [ string last .sa $res ] - 1 } ]; set res [ string range $res 0 $last ] } { set choice 2 }" );
		if ( choice == 2 )
			break;

		// form full name
		lab1 = get_str( "res" );
		lab2 = get_str( "path" );
		if ( sens_file != NULL )
			delete sens_file;
		sens_file = new char[ strlen( lab1 ) + strlen( lab2 ) + 5 ];
		sprintf( sens_file,"%s%s%s.sa", lab2, strlen( lab2 ) > 0 ? "/" : "", lab1 );

		// read sensitivity file (text mode)
		f = fopen( sens_file, "rt" );
		if ( f == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity Analysis file not found\"" );
			break;
		}

		if ( load_sensitivity( f ) != 0 )
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid sensitivity analysis file\" -detail \"Please check if you select a valid file or recreate your sensitivity configuration.\"" );

		fclose( f );

	break;


	// Save a sensitivity analysis configuration
	case 65:

		// check a model is already loaded
		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to save a sensitivity analysis configuration.\"" );
			break;
		}

		// check for existing sensitivity data loaded
		if ( rsense == NULL )
		{
			sensitivity_undefined( );			// throw error
			break;
		}

		// default file name and path
		cmd( "set res %s", simul_name );
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		// open dialog box to get file name & folder
		choice = 0;
		cmd( "set bah [ tk_getSaveFile -parent . -title \"Save Sensitivity Analysis File\" -defaultextension \".sa\" -initialfile $res -initialdir \"$path\" -filetypes { { {Sensitivity analysis files} {.sa} } } ]" );
		cmd( "if { [ string length $bah ] > 0 } { set path [ file dirname $bah ]; set res [ file tail $bah ]; set last [ expr { [ string last .sa $res ] - 1 } ]; set res [ string range $res 0 $last ] } { set choice 2 }" );
		if ( choice == 2 )
			break;

		// form full name
		lab1 = get_str( "res" );
		lab2 = get_str( "path" );
		if ( sens_file != NULL )
			delete sens_file;
		sens_file = new char[ strlen( lab1 ) + strlen( lab2 ) + 5 ];
		sprintf( sens_file,"%s%s%s.sa", lab2, strlen( lab2 ) > 0 ? "/" : "", lab1 );

		// write sensitivity file (text mode)
		f = fopen( sens_file, "wt" );  // use text mode for Windows better compatibility
		if ( f == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity analysis file not saved\" -detail \"Please check if the file name and path are valid.\"" );
			break;
		}

		if ( ! save_sensitivity( f ) )
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity analysis file cannot be saved\" -detail \"Check if the drive or the file is set READ-ONLY.\"" );

		fclose( f );
		unsavedSense = false;			// nothing to save

	break;


	// export saved elements details
	case 91:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration to export\" -detail \"Please load or create and load a configuration before trying to export the details on the elements to save.\"" );
			break;
		}

		// warn about no variable being saved
		i = 0;
		count_save( root, &i );
		if ( i == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"No variable or parameter marked to be saved\" -detail \"Please mark the variables and parameters to be saved before trying to export the details on the elements to save.\"" );
			break;
		}

		// default file name
		cmd( "set res %s-saved", simul_name );

		// make sure there is a path set
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		// open dialog box to get file name & folder
		choice = 0;
		cmd( "set bah [ tk_getSaveFile -parent . -title \"Export Saved Elements Configuration as Comma-separated Text File\" -defaultextension \".csv\" -initialfile $res -initialdir \"$path\" -filetypes { { {Comma-separated files} {.csv} } } ]" );
		cmd( "if { [ string length $bah ] > 0 } { set path [ file dirname $bah ]; set res [ file tail $bah ] } { set choice 2 }" );
		if ( choice == 2 )
			break;

		// form full name
		lab1 = get_str( "res" );
		lab2 = get_str( "path" );
		snprintf( lab, MAX_PATH_LENGTH,"%s%s%s", lab2, strlen( lab2 ) > 0 ? "/" : "", lab1 );

		// write export file (text mode)
		f = fopen( lab, "wt" );	 // use text mode for Windows better compatibility
		if ( f == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Saved elements configuration file not saved\" -detail \"Please check if the file name and path are valid.\"" );
			break;
		}

		strcpy( ch, CSV_SEP );

		// write .csv header & content
		fprintf( f, "Name%sType%sObject%sDescription\n", ch, ch, ch );
		get_saved( root, f, ch );
		fclose( f );

	break;


	// export sensitivity configuration as a .csv file
	case 90:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to save a sensitivity analysis configuration.\"" );
			break;
		}

		// check for existing sensitivity data loaded
		if ( rsense == NULL )
		{
			sensitivity_undefined( );			// throw error
			break;
		}

		// default file name
		cmd( "set res %s-limits", simul_name );

		// make sure there is a path set
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		// open dialog box to get file name & folder
		choice = 0;
		cmd( "set bah [ tk_getSaveFile -parent . -title \"Export Sensitivity Limits as Comma-separated Text File\" -defaultextension \".csv\" -initialfile $res -initialdir \"$path\" -filetypes { { {Comma-separated files} {.csv} } } ]" );
		cmd( "if { [ string length $bah ] > 0 } { set path [ file dirname $bah ]; set res [ file tail $bah ] } { set choice 2 }" );
		if ( choice == 2 )
			break;

		// form full name
		lab1 = get_str( "res" );
		lab2 = get_str( "path" );
		snprintf( lab, MAX_PATH_LENGTH,"%s%s%s", lab2, strlen( lab2 ) > 0 ? "/" : "", lab1 );

		// write export file (text mode)
		f = fopen( lab, "wt" );	 // use text mode for Windows better compatibility
		if ( f == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity limits file not saved\" -detail \"Please check if the file name and path are valid.\"" );
			break;
		}

		// write .csv header
		strcpy( ch, CSV_SEP );
		fprintf( f, "Name%sType%sLag%sFormat%sValue%sMinimum%sMaximum%sDescription\n", ch, ch, ch, ch, ch, ch, ch );

		// write data
		get_sa_limits( r, f, ch );

		fclose( f );

	break;


	// Show sensitivity analysis configuration
	case 66:

		choice = 50;

		// check for existing sensitivity data loaded
		if ( rsense == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"There is no sensitivity data to show\"" );
			break;
		}

		// print data to log window
		plog( "\n\nVariables and parameters set for sensitivity analysis :\n" );
		for ( cs = rsense; cs != NULL; cs = cs->next )
		{
			if ( cs->param == 1 )
				plog( "Param: %s\\[%s\\]\t#%d:\t", cs->label, cs->integer ? "int" : "flt", cs->nvalues );
			else
				plog( "Var: %s(-%d)\\[%s\\]\t#%d:\t", cs->label, cs->lag+1, cs->integer ? "int" : "flt", cs->nvalues );

			for ( i = 0; i < cs->nvalues; ++i )
				plog_tag( "%g\t", "highlight", cs->v[ i ] );
			plog( "\n" );
		}

	break;


	// Remove sensitivity analysis configuration
	case 67:

		choice = 0;

		// check for existing sensitivity data loaded
		if ( rsense == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No sensitivity data to remove\"" );
			break;
		}

		if ( ! discard_change( true, true ) )	// unsaved configuration?
			break;

		// empty sensitivity data
		empty_sensitivity( rsense );			// discard read data
		plog( "\nSensitivity data removed.\n" );
		rsense = NULL;
		unsavedSense = false;					// nothing to save
		findexSens = 0;

	break;


	// Create batch for multi-runs jobs and optionally run it
	case 68:

		// check a model is already loaded
		if ( ! struct_loaded )
			findexSens = 0;									// no sensitivity created
		else
			if ( ! discard_change( false ) )				// unsaved configuration?
				break;

		// check for existing NW executable
		snprintf( nw_exe, MAX_PATH_LENGTH, "%s/lsdNW", exec_path );	// form full executable name
		if ( platform == _WIN_ )
			strcatn( nw_exe, ".exe", MAX_PATH_LENGTH );	// add Windows ending

		if ( ( f = fopen( nw_exe, "rb" ) ) == NULL )
		{
			if ( ! make_no_window( ) )
				break;
		}
		else
			fclose( f );

		// check if NW executable file is older than running executable file
		snprintf( lab, MAX_PATH_LENGTH, "%s/%s", exec_path, exec_file );	// form full exec name

		// get OS info for files
		if ( stat( nw_exe, &stExe ) == 0 && stat( lab, &stMod ) == 0 )
		{
			if ( difftime( stExe.st_mtime, stMod.st_mtime ) < 0 )
			{
				cmd( "switch [ ttk::messageBox -parent . -title Warning -icon warning -type yesnocancel -default yes -message \"Recompile 'lsdNW'?\" -detail \"The existing 'No Window' executable file ('lsdNW') is older than the current executable.\n\nPress 'Yes' to recompile, 'No' continue anyway, or 'Cancel' to abort.\" ] { \
						yes { set choice 0 } \
						no { set choice 1 } \
						cancel { set choice 2 } \
					}" );

				if ( choice == 2 )
					break;

				if ( choice == 0 )
					if ( ! make_no_window( ) )
						break;
			}
		}

		// check if serial sensitivity configuration was just created
		choice = 0;
		if ( findexSens > 0 )
			cmd( "set answer [ ttk::messageBox -parent . -type yesnocancel -icon question -default yes -title \"Parallel Batch\" -message \"Configuration set available\" -detail \"A sequential sensitivity set of configuration files was just produced and can be used to create the batch.\n\nPress 'Yes' to confirm or 'No' to select a different set of files.\" ]; switch -- $answer { yes { set choice 1 } no { set choice 0 } cancel { set choice 2 } }" );
		if ( choice == 2 )
			break;

		// get configuration files to use
		if ( choice == 1 )							// use current configuration files
		{
			if ( strlen( path_sens ) == 0 || strlen( simul_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid simulation folder or name\" -detail \"Please try again.\"" );
				findexSens = 0;						// no sensitivity created
				break;
			}

			ffirst = fSeq = 1;
			fnext = findexSens;
			findexSens = 0;
			strcpyn( out_file, simul_name, MAX_PATH_LENGTH );
			strcpyn( out_dir, path_sens, MAX_PATH_LENGTH );
			cmd( "set res \"%s\"", simul_name );
			cmd( "set path \"%s\"", path );
		}
		else										// ask for first configuration file
		{
			cmd( "set answer [ ttk::messageBox -parent . -type yesnocancel -icon question -default yes -title \"Create Batch\" -message \"Select sequence of configuration files?\" -detail \"Press 'Yes' to choose the first file of the continuous sequence (format: 'name_NNN.lsd') or 'No' to select a different set of files (use 'Ctrl' to pick multiple files).\" ]; switch -- $answer { yes { set choice 1 } no { set choice 0 } cancel { set choice 2 } }" );
			if ( choice == 2 )
				break;
			else
				fSeq = choice;

			if ( fSeq && strlen( simul_name ) > 0 )	// default name
				cmd( "set res \"%s_1.lsd\"", simul_name );
			else
				cmd( "set res \"\"" );

			cmd( "set path \"%s\"", path );
			if ( strlen( path ) > 0 )
				cmd( "cd \"$path\"" );

			// open dialog box to get file name & folder
			if ( fSeq )								// file sequence?
			{
				cmd( "set bah [ tk_getOpenFile -parent . -title \"Load First Configuration File\" -defaultextension \".lsd\" -initialfile $res -initialdir \"$path\" -filetypes { { {LSD model files} {.lsd} } } -multiple no ]" );
				cmd( "if { [ string length $bah ] > 0 && ! [ fn_spaces \"$bah\" . ] } { \
						set res $bah; \
						set path [ file dirname $res ]; \
						set res [ file tail $res ]; \
						set last [ expr { [ string last .lsd $res ] - 1 } ]; \
						set res [ string range $res 0 $last ]; \
						set numpos [ expr { [ string last _ $res ] + 1 } ]; \
						if { $numpos > 0 } { \
							set choice [ expr { [ string range $res $numpos end ] } ]; \
							set res [ string range $res 0 [ expr { $numpos - 2 } ] ] \
						} else { \
							plog \"\nInvalid file name for sequential set: $res\n\"; \
							set choice 0 \
						} \
					} else { \
						set choice 0 \
					}" );
				if ( choice == 0 )
					break;

				ffirst = choice;
				get_str( "res", out_file, MAX_PATH_LENGTH );
				get_str( "path", out_dir, MAX_PATH_LENGTH );
				f = NULL;
				do									// search for all sequential files
				{
					if ( strlen( out_dir ) == 0 )			// default path
						snprintf( lab, MAX_BUFF_SIZE, "%s_%d.lsd", out_file, choice++ );
					else
						snprintf( lab, MAX_BUFF_SIZE, "%s/%s_%d.lsd", out_dir, out_file, choice++ );

					if ( f != NULL )
						fclose( f );
					f = fopen( lab, "r" );
				}
				while ( f != NULL );

				fnext = choice - 1;
			}
			else									// bunch of files?
			{
				cmd( "set bah [ tk_getOpenFile -parent . -title \"Load Configuration Files\" -defaultextension \".lsd\" -initialdir \"$path\" -filetypes { { {LSD model files} {.lsd} } } -multiple yes ]" );
				cmd( "set choice [ llength $bah ]" );
				cmd( "if { $choice > 0 && ! [ fn_spaces [ lindex $bah 0 ] . 1 ] } { \
						set res [ lindex $bah 0 ]; \
						set path [ file dirname $res ]; \
						set res [ file tail $res ]; \
						set last [ expr { [ string last .lsd $res ] - 1 } ]; \
						set res [ string range $res 0 $last ]; \
						set numpos [ expr { [ string last _ $res ] + 1 } ]; \
						if { $numpos > 0 } { \
							set res [ string range $res 0 [ expr { $numpos - 2 } ] ] \
						} \
					}" );
				if ( choice == 0 )
					break;

				ffirst = 1;
				fnext = choice + 1;
				get_str( "path", out_dir, MAX_PATH_LENGTH );
			}
		}

		Tcl_LinkVar( inter, "natBat", ( char * ) & natBat, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "no_res", ( char * ) & no_res, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "no_tot", ( char * ) & no_tot, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "docsv", ( char * ) & docsv, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "dozip", ( char * ) & dozip, TCL_LINK_BOOLEAN );

		if ( no_tot )
			no_res = false;

		cmd( "set res2 $res" );
		cmd( "set cores %d", max_threads );
		cmd( "set threads 1" );

		cmd( "newtop .s \"Parallel Batch\" { set choice 2 }" );

		cmd( "ttk::frame .s.u" );
		cmd( "ttk::label .s.u.l -text \"Output path\"" );
		cmd( "ttk::label .s.u.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", out_dir );
		cmd( "pack .s.u.l .s.u.w" );

		cmd( "ttk::frame .s.t" );
		cmd( "ttk::label .s.t.l -text \"Batch file base name\"" );
		cmd( "ttk::entry .s.t.e -width 20 -textvariable res2 -justify center" );
		cmd( "pack .s.t.l .s.t.e" );

		cmd( "ttk::frame .s.c" );
		cmd( "ttk::label .s.c.l -justify center -text \"Number of parallel\nLSD runs\"" );
		cmd( "ttk::spinbox .s.c.e -width 5 -from 1 -to 99 -justify center -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set cores %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cores; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( ".s.c.e insert 0 $cores" );
		cmd( "ttk::label .s.c.w -justify center -text \"(a number higher than the\nnumber of processors/cores\nis not recommended)\"" );
		cmd( "pack .s.c.l .s.c.e .s.c.w" );

		cmd( "ttk::frame .s.p" );
		cmd( "ttk::label .s.p.l -justify center -text \"Number of threads\nper LSD runs\"" );
		cmd( "ttk::spinbox .s.p.e -width 5 -from 1 -to 99 -justify center -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set threads %%P; return 1 } { %%W delete 0 end; %%W insert 0 $threads; return 0 } } -invalidcommand { bell } -justify center" );
		cmd( ".s.p.e insert 0 $threads" );
		cmd( "ttk::label .s.p.w -justify center -text \"(a number higher than 1\nis only useful when parallel\ncomputation is enabled)\"" );
		cmd( "pack .s.p.l .s.p.e .s.p.w" );

		cmd( "ttk::frame .s.o" );
		cmd( "ttk::checkbutton .s.o.nores -text \"Skip generating results files\" -variable no_res -command { \
					if { $no_res && $no_tot } { \
						set no_tot 0 \
					} \
				}" );
		cmd( "ttk::checkbutton .s.o.notot -text \"Skip generating totals files\" -variable no_tot -command { \
					if { $no_res && $no_tot } { \
						set no_res 0 \
					} \
				}" );
		cmd( "ttk::checkbutton .s.o.n -text \"Native batch format\" -variable natBat" );
		cmd( "ttk::checkbutton .s.o.dozip -text \"Generate zipped files\" -variable dozip" );
		cmd( "ttk::checkbutton .s.o.docsv -text \"Comma-separated text format (.csv)\" -variable docsv" );
		cmd( "pack .s.o.nores .s.o.notot .s.o.n .s.o.dozip .s.o.docsv -anchor w" );

		cmd( "pack .s.u .s.t .s.c .s.p .s.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menurun.html#parallel } { set choice 2 }" );
		cmd( "bind .s.c.e <KeyPress-Return> { .s.b.ok invoke }" );

		cmd( "showtop .s" );
		cmd( "mousewarpto .s.b.ok 0" );
		cmd( ".s.c.e selection range 0 end" );
		cmd( "focus .s.c.e" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set cores [ .s.c.e get ]" );
		cmd( "set threads [ .s.p.e get ]" );

		cmd( "destroytop .s" );

		Tcl_UnlinkVar( inter, "natBat" );
		Tcl_UnlinkVar( inter, "no_res" );
		Tcl_UnlinkVar( inter, "no_tot" );
		Tcl_UnlinkVar( inter, "docsv" );
		Tcl_UnlinkVar( inter, "dozip" );

		if ( choice == 2 )
			break;

		param = get_int( "cores" );
		if ( param < 1 || param > SRV_MAX_CORES )
			param = min( max_threads, SRV_MAX_CORES );

		nature = get_int( "threads" );
		if ( nature < 1 || nature > SRV_MAX_CORES )
			nature = min( max_threads, SRV_MAX_CORES );

		get_str( "res2", out_bat, MAX_PATH_LENGTH );

		// select batch format & create batch file
		cmd( "if [ string equal $CurPlatform windows ] { if { $natBat == 1 } { set choice 1 } { set choice 2 } } { if { $natBat == 1 } { set choice 3 } { set choice 4 } }" );
		if ( fSeq )
			if ( choice == 1 || choice == 4 )
				snprintf( lab, MAX_BUFF_SIZE, "%s/%s_%d_%d.bat", out_dir, out_bat, ffirst, fnext - 1 );
			else
				snprintf( lab, MAX_BUFF_SIZE, "%s/%s_%d_%d.sh", out_dir, out_bat, ffirst, fnext - 1 );
		else
			if ( choice == 1 || choice == 4 )
				snprintf( lab, MAX_BUFF_SIZE, "%s/%s.bat", out_dir, out_bat );
			else
				snprintf( lab, MAX_BUFF_SIZE, "%s/%s.sh", out_dir, out_bat );

		f = fopen( lab, "wb" );						// binary mode to bypass CR/LF handling
		if ( f == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Batch file cannot be created\" -detail \"Check if LSD still has WRITE access to the model directory.\"" );
			findexSens = 0;							// no sensitivity created
			break;
		}

		if ( choice == 1 || choice == 4 )			// Windows header
		{
			// convert to Windows folder separators (\)
			for ( i = 0; ( unsigned ) i < strlen( nw_exe ); ++i )
				if ( nw_exe[ i ] == '/' )
					nw_exe[ i ] = '\\';

			strcpyn( win_dir, out_dir, MAX_PATH_LENGTH );

			for ( i = 0; ( unsigned ) i < strlen( win_dir ); ++i )
				if ( win_dir[ i ] == '/' )
					win_dir[ i ]='\\';

			fprintf( f, "@echo off\nrem Batch generated by LSD\r\n" );
			fprintf( f, "echo Processing %d configuration files in up to %d parallel processes...\r\n", fnext - ffirst, param );
			fprintf( f, "if \"%%~1\"==\"\" (set LSD_EXEC=\"%s\") else (set LSD_EXEC=\"%%~1\")\r\n", nw_exe );
			fprintf( f, "if \"%%~2\"==\"\" (set LSD_CONFIG_PATH=\"%s\") else (set LSD_CONFIG_PATH=\"%%~2\")\r\n", win_dir );
			fprintf( f, "set LSD_EXEC=%%LSD_EXEC:\"=%%\r\n" );
			fprintf( f, "set LSD_CONFIG_PATH=%%LSD_CONFIG_PATH:\"=%%\r\n" );
			fprintf( f, "echo LSD executable: %%LSD_EXEC%%\r\n" );
			fprintf( f, "echo Configuration path: %%LSD_CONFIG_PATH%%\r\n" );
			fprintf( f, "echo Use %s.bat LSD_EXEC CONFIG_PATH to change defaults\r\n", out_bat );
		}
		else										// Unix header
		{
			if ( ! natBat )							// Unix in Windows?
			{
				if ( strchr( nw_exe, ':' ) != NULL )	// remove Windows drive letter
				{
					strcpyn( lab_old, strchr( nw_exe, ':' ) + 1, 2 * MAX_PATH_LENGTH );
					strcpyn( nw_exe, lab_old, MAX_PATH_LENGTH );
				}

				if ( strchr( out_dir, ':' ) != NULL )	// remove Windows drive letter
				{
					strcpyn( lab_old, strchr( out_dir, ':' ) + 1, 2 * MAX_PATH_LENGTH );
					strcpyn( out_dir, lab_old, MAX_PATH_LENGTH );
				}

				if ( ( lab0 = strstr( nw_exe, ".exe" ) ) != NULL )	// remove Windows extension, if present
					lab0[ 0 ]='\0';
				else
					if ( ( lab0 = strstr( nw_exe, ".EXE" ) ) != NULL )
						lab0[ 0 ]='\0';
			}

			// set background low priority in servers (cores/jobs > SRV_MIN_CORES)
			if ( nature > SRV_MIN_CORES || ( param > SRV_MIN_CORES && fnext - ffirst > SRV_MIN_CORES ) )
			{
				snprintf( lab_old, 2 * MAX_PATH_LENGTH, "nice %s", nw_exe );
				strcpyn( nw_exe, lab_old, MAX_PATH_LENGTH );
			}

			fprintf( f, "#!/bin/bash\n# Script generated by LSD\n" );
			fprintf( f, "echo \"Processing %d configuration files in up to %d parallel processes...\"\n", fnext - ffirst, param );
			fprintf( f, "if [ \"$1\" = \"\" ]; then LSD_EXEC=\"%s\"; else LSD_EXEC=\"$1\"; fi\n", nw_exe );
			fprintf( f, "if [ \"$2\" = \"\" ]; then LSD_CONFIG_PATH=\"%s\"; else LSD_CONFIG_PATH=\"$2\"; fi\n", out_dir );
			fprintf( f, "echo \"LSD executable: $LSD_EXEC\"\n" );
			fprintf( f, "echo \"Configuration path: $LSD_CONFIG_PATH\"\n" );
			fprintf( f, "echo \"Use %s.sh LSD_EXEC CONFIG_PATH to change default paths\"\n", out_bat );
		}

		logs.clear( );

		if ( fSeq && ( fnext - ffirst ) > param )	// if possible, work in blocks
		{
			num = ( fnext - ffirst ) / param;		// base number of cases per core
			sl = ( fnext - ffirst ) % param;		// remaining cases per core
			for ( i = ffirst, j = 1; j <= param; ++j )	// allocates files by the number of cores
			{
				snprintf( lab_old, 2 * MAX_PATH_LENGTH, "%s_%d.log", out_file, j );
				logs.push_back( lab_old );

				if ( choice == 1 || choice == 4 )	// Windows
					fprintf( f, "start \"LSD Process %d\" /B \"%%LSD_EXEC%%\" -c %d -f \"%%LSD_CONFIG_PATH%%\\%s\" -s %d -e %d%s%s%s%s -l \"%%LSD_CONFIG_PATH%%\\%s\"\r\n", j, nature, out_file, i, j <= sl ? i + num : i + num - 1, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", lab_old );
				else								// Unix
					fprintf( f, "$LSD_EXEC -c %d -f \"$LSD_CONFIG_PATH\"/%s -s %d -e %d%s%s%s%s -l \"$LSD_CONFIG_PATH\"/%s &\n", nature, out_file, i, j <= sl ? i + num : i + num - 1, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", lab_old );

				j <= sl ? i += num + 1 : i += num;
			}
		}
		else										// if not, do one by one
		{
			for ( i = ffirst, j = 1; i < fnext; ++i, ++j )
			{
				if ( fSeq )
				{
					snprintf( lab_old, 2 * MAX_PATH_LENGTH, "%s_%d.log", out_file, i );

					if ( choice == 1 || choice == 4 )	// Windows
						fprintf( f, "start \"LSD Process %d\" /B \"%%LSD_EXEC%%\" -c %d -f \"%%LSD_CONFIG_PATH%%\\%s_%d.lsd\"%s%s%s%s -l \"%%LSD_CONFIG_PATH%%\\%s\"\r\n", j, nature, out_file, i, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", lab_old );
					else								// Unix
						fprintf( f, "$LSD_EXEC -c %d -f \"$LSD_CONFIG_PATH\"/%s_%d.lsd%s%s%s%s -l \"$LSD_CONFIG_PATH\"/%s &\n", nature, out_file, i, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", lab_old );
				}
				else
				{	// get the selected file names, one by one
					cmd( "set res3 [ lindex $bah %d ]; set res3 [ file tail $res3 ]; set last [ expr { [ string last .lsd $res3 ] - 1 } ]; set res3 [ string range $res3 0 $last ]", j - 1	);
					get_str( "res3", out_file, MAX_PATH_LENGTH - 4 );
					snprintf( lab_old, 2 * MAX_PATH_LENGTH, "%s.log", out_file );

					if ( choice == 1 || choice == 4 )	// Windows
						fprintf( f, "start \"LSD Process %d\" /B \"%%LSD_EXEC%%\" -c %d -f \"%%LSD_CONFIG_PATH%%\\%s.lsd\"%s%s%s%s -l \"%%LSD_CONFIG_PATH%%\\%s\"\r\n", j, nature, out_file, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", lab_old );
					else								// Unix
						fprintf( f, "$LSD_EXEC -c %d -f \"$LSD_CONFIG_PATH\"/%s.lsd%s%s%s%s -l \"$LSD_CONFIG_PATH\"/%s &\n", nature, out_file, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", lab_old );
				}

				logs.push_back( lab_old );
			}
		}

		if ( fSeq )
			if ( choice == 1 || choice == 4 )	// Windows closing
			{
				fprintf( f, "echo %d log files being generated: %s_1.log to %s_%d.log .\r\n", j - 1, out_file, out_file, j - 1 );
				fclose( f );
			}
			else								// Unix closing
			{
				fprintf( f, "echo \"%d log files being generated: %s_1.log to %s_%d.log .\"\n", j - 1, out_file, out_file, j - 1 );
				fclose( f );
				chmod( lab, ACCESSPERMS );		// set executable perms
			}
		else
			if ( choice == 1 || choice == 4 )	// Windows closing
			{
				fprintf( f, "echo %d log files being generated.\r\n", j - 1 );
				fclose( f );
			}
			else								// Unix closing
			{
				fprintf( f, "echo \"%d log files being generated.\"\n", j - 1 );
				fclose( f );
				chmod( lab, ACCESSPERMS );		// set executable perms
			}

		plog( "\nParallel batch file created: %s", lab );

		if ( ! natBat )
			break;

		// ask if script/batch should be executed right away
		cmd( "set answer [ ttk::messageBox -parent . -type yesno -icon question -default no -title \"Run Parallel Batch\" -message \"Run created batch?\" -detail \"The batch for running the configuration files was created.\n\nPress 'Yes' if you want to start the it as separated processes now.\" ]; switch -- $answer { yes { set choice 1 } no { set choice 2 } }" );
		if ( choice == 2 )
			break;

		// start the job
		cmd( "set oldpath [ pwd ]" );
		cmd( "set path \"%s\"", out_dir );
		if ( strlen( out_dir ) > 0 )
			cmd( "cd $path" );

		cmd( "catch { exec %s & }", lab );
		show_logs( out_dir, logs );

		cmd( "set path $oldpath; cd $path" );

	break;


	// Start NO WINDOW job as a separate background process
	case 69:

#ifndef _NP_

		// check if background are not being run already
		if ( parallel_monitor )
		{
			cmd( "if { [ ttk::messageBox -parent . -type okcancel -default ok -icon warning -title Warning -message \"Abort running simulation?\" -detail \"A set of parallel simulation runs is being executed in background. You may choose to interrupt it now and proceed, or wait until it finishes before running a new one.\" ] eq \"ok\" } { set choice 1 } { set choice 0 }" );

			if ( choice == 0 )
				break;

			if ( ! stop_parallel( ) )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Failed to abort running simulation\" -detail \"Please wait until the current parallel run finishes before trying to start a new one.\"" );
				break;
			}
		}

#endif

		// check a model is already loaded
		if ( ! struct_loaded || strlen( simul_name ) == 0 || strlen( struct_file ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to start a parallel run.\"" );
			break;
		}

		// check for existing NW executable
		snprintf( nw_exe, MAX_PATH_LENGTH, "%s/lsdNW", exec_path );// form full executable name
		if ( platform == _WIN_ )
			strcatn( nw_exe, ".exe", MAX_PATH_LENGTH );	// add Windows ending

		if ( ( f = fopen( nw_exe, "rb" ) ) == NULL )
		{
			if ( ! make_no_window( ) )
				break;
		}
		else
			fclose( f );

		// check if NW executable file is older than running executable file
		snprintf( lab, MAX_PATH_LENGTH, "%s/%s", exec_path, exec_file );	// form full exec name

		// get OS info for files
		if ( stat( nw_exe, &stExe ) == 0 && stat( lab, &stMod ) == 0 )
		{
			if ( difftime( stExe.st_mtime, stMod.st_mtime ) < 0 )
			{
				cmd( "switch [ ttk::messageBox -parent . -title Warning -icon warning -type yesnocancel -default yes -message \"Recompile 'lsdNW'?\" -detail \"The existing 'No Window' executable file ('lsdNW') is older than the current executable.\n\nPress 'Yes' to recompile, 'No' continue anyway, or 'Cancel' to abort.\" ] { \
						yes { set choice 0 } \
						no { set choice 1 } \
						cancel { set choice 2 } \
					}" );

				if ( choice == 2 )
					break;

				if ( choice == 0 )
					if ( ! make_no_window( ) )
						break;
			}
		}

		// remove any custom save path (save to current by default)
		results_alt_path( "" );

		// detect the need of a new save path and if it has results files
		subDir = need_res_dir( path, simul_name, out_dir, MAX_PATH_LENGTH );
		overwDir = check_res_dir( out_dir );

		Tcl_LinkVar( inter, "no_res", ( char * ) & no_res, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "no_tot", ( char * ) & no_tot, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "dobar", ( char * ) & dobar, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "docsv", ( char * ) & docsv, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "doover", ( char * ) & doover, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "dozip", ( char * ) & dozip, TCL_LINK_BOOLEAN );
		Tcl_LinkVar( inter, "overwConf", ( char * ) & overwConf, TCL_LINK_BOOLEAN );

		// only ask to overwrite configuration if there are changes
		overwConf = unsaved_change( ) ? true : false;
		add_to_tot = false;

		if ( no_tot )
			no_res = false;

#ifdef _NP_
		param = 1;
#else
		param = min( sim_num, max_threads );
#endif

		cmd( "set simNum %d", sim_num );
		cmd( "set firstFile \"%s_%d\"", simul_name, seed );
		cmd( "set lastFile \"%s_%d\"", simul_name, seed + sim_num - 1 );
		cmd( "set totFile \"%s\"", simul_name );
		cmd( "set resExt %s", docsv ? "csv" : "res" );
		cmd( "set totExt %s", docsv ? "csv" : "tot" );
		cmd( "set zipExt %s", dozip ? ".gz" : "" );
		cmd( "set cores %d", param );
		cmd( "set tot_msg_warn \"(WARNING: existing totals file(s) in\noutput path may be overwritten)\"" );

		// confirm overwriting current configuration
		cmd( "set b .batch" );
		cmd( "newtop $b \"Parallel Run\" { set choice 2 }" );

		cmd( "ttk::frame $b.f1" );
		cmd( "ttk::label $b.f1.l -text \"Model configuration\"" );
		cmd( "ttk::label $b.f1.w -text \"%s\" -style hl.TLabel", simul_name );
		cmd( "pack $b.f1.l $b.f1.w" );

		cmd( "ttk::frame $b.f2" );

		cmd( "ttk::frame $b.f2.t" );
		cmd( "ttk::label $b.f2.t.l -text \"Cases:\"" );
		cmd( "ttk::label $b.f2.t.w -text \"%d\" -style hl.TLabel", max_step );
		cmd( "pack $b.f2.t.l $b.f2.t.w -side left -padx 2" );

		cmd( "ttk::frame $b.f2.n" );
		cmd( "ttk::label $b.f2.n.l -text \"Number of simulations:\"" );
		cmd( "ttk::label $b.f2.n.w -text \"%d\" -style hl.TLabel", sim_num );
		cmd( "pack $b.f2.n.l $b.f2.n.w -side left -padx 2" );
		cmd( "pack $b.f2.t $b.f2.n" );

		cmd( "ttk::frame $b.f3" );
		cmd( "ttk::label $b.f3.l -text \"Output path\"" );
		cmd( "ttk::label $b.f3.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", out_dir );
		cmd( "pack $b.f3.l $b.f3.w" );

		cmd( "ttk::frame $b.f4" );
		cmd( "ttk::label $b.f4.l -text \"Results file(s)\"" );

		if ( sim_num > 1 )	// multiple runs case
		{
			cmd( "ttk::frame $b.f4.w" );

			cmd( "ttk::frame $b.f4.w.l1" );
			cmd( "ttk::label $b.f4.w.l1.l -text \"from:\"" );
			cmd( "ttk::label $b.f4.w.l1.w -style hl.TLabel -text \"$firstFile.$resExt$zipExt\"" );
			cmd( "pack $b.f4.w.l1.l $b.f4.w.l1.w -side left -padx 2" );

			cmd( "ttk::frame $b.f4.w.l2" );
			cmd( "ttk::label $b.f4.w.l2.l -text \"to:\"" );
			cmd( "ttk::label $b.f4.w.l2.w -style hl.TLabel -text \"$lastFile.$resExt$zipExt\"" );
			cmd( "pack $b.f4.w.l2.l $b.f4.w.l2.w -side left -padx 2" );

			cmd( "pack $b.f4.w.l1 $b.f4.w.l2" );
		}
		else				// single run case
			cmd( "ttk::label $b.f4.w -style hl.TLabel -text \"$firstFile.$resExt$zipExt\"" );

		cmd( "pack $b.f4.l $b.f4.w" );

		cmd( "set choice [ expr { ! $no_tot && ( [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] ) } ]", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "" );

		cmd( "ttk::frame $b.f5" );
		cmd( "ttk::label $b.f5.l1 -text \"Totals file (last steps)\"" );
		cmd( "ttk::label $b.f5.l2 -style %s -text \"$totFile.$totExt$zipExt\"", choice ? "hl.TLabel" : "dhl.TLabel" );

		if ( choice )
			cmd( "ttk::label $b.f5.l3 -justify center -text $tot_msg_warn" );
		else
			cmd( "ttk::label $b.f5.l3 -justify center -text \"\n\"" );

		cmd( "pack $b.f5.l1 $b.f5.l2 $b.f5.l3" );

		cmd( "ttk::frame $b.f6" );
		cmd( "ttk::label $b.f6.l -text \"Parallel runs\"" );
		cmd( "ttk::spinbox $b.f6.e -width 5 -from 1 -to %d -justify center -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set cores %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cores; return 0 } } -invalidcommand { bell } -justify center -state %s", param, ( no_tot && sim_num > 1 && param > 1 ) ? "normal" : "disabled" );
		cmd( "write_any $b.f6.e $cores" );
		cmd( "pack $b.f6.l $b.f6.e -side left -padx 2" );

		cmd( "ttk::frame $b.f7" );
		cmd( "ttk::checkbutton $b.f7.nores -text \"Skip generating results files\" -variable no_res -command { \
				if { $no_res && $no_tot } { \
					set no_tot 0; \
					$b.f5.l2 configure -style hl.TLabel; \
					$b.f6.e configure -state disabled; \
					if { [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
						$b.f5.l3 configure -text $tot_msg_warn \
					} else { \
						$b.f5.l3 configure -text \"\n\" \
					} \
				} \
			}", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
		cmd( "ttk::checkbutton $b.f7.notot -text \"Skip generating totals file\" -variable no_tot -command { \
				if { $no_res && $no_tot } { \
					set no_res 0 \
				}; \
				if { ! $no_tot } { \
					$b.f5.l2 configure -style hl.TLabel; \
					$b.f6.e configure -state disabled; \
					if { [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
						$b.f5.l3 configure -text $tot_msg_warn \
					} else { \
						$b.f5.l3 configure -text \"\n\" \
					} \
				} else { \
					if { %d > 1 && %d > 1 } { \
						$b.f6.e configure -state normal \
					}; \
					$b.f5.l2 configure -style dhl.TLabel; \
					$b.f5.l3 configure -text \"\n\" \
				} \
			}", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "", sim_num, param );
		cmd( "ttk::checkbutton $b.f7.dozip -text \"Generate zipped files\" -variable dozip -command { \
				if $dozip { \
					set zipExt .gz \
				} else { \
					set zipExt \"\" \
				}; \
				if { $simNum > 1 } { \
					$b.f4.w.l1.w configure -text \"$firstFile.$resExt$zipExt\"; \
					$b.f4.w.l2.w configure -text \"$lastFile.$resExt$zipExt\"; \
				} else { \
					$b.f4.w configure -text \"$firstFile.$resExt$zipExt\"; \
				}; \
				$b.f5.l2 configure -text \"$totFile.$totExt$zipExt\"; \
				if { [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
					$b.f5.l3 configure -text $tot_msg_warn \
				} else { \
					$b.f5.l3 configure -text \"\n\" \
				} \
			}", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
		cmd( "ttk::checkbutton $b.f7.docsv -text \"Comma-separated text format (.csv)\" -variable docsv -command { \
				if $docsv { set resExt csv; set totExt csv } { \
					set resExt res; \
					set totExt tot \
				}; \
				if { $simNum > 1 } { \
					$b.f4.w.l1.w configure -text \"$firstFile.$resExt$zipExt\"; \
					$b.f4.w.l2.w configure -text \"$lastFile.$resExt$zipExt\"; \
				} else { \
					$b.f4.w configure -text \"$firstFile.$resExt$zipExt\"; \
				}; \
				$b.f5.l2 configure -text \"$totFile.$totExt$zipExt\"; \
				if { [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
					$b.f5.l3 configure -text $tot_msg_warn \
				} else { \
					$b.f5.l3 configure -text \"\n\" \
				} \
			}", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
		cmd( "ttk::checkbutton $b.f7.dobar -text \"Show progress bar in logs\" -variable dobar" );
		cmd( "ttk::checkbutton $b.f7.doover -text \"Clear output path before run\" -variable doover -state %s", overwDir ? "normal" : "disabled" );
		cmd( "ttk::checkbutton $b.f7.tosave -text \"Update configuration file\" -variable overwConf -state %s", overwConf ? "normal" : "disabled" );
		cmd( "pack $b.f7.nores $b.f7.notot $b.f7.dozip $b.f7.docsv $b.f7.dobar $b.f7.doover $b.f7.tosave -anchor w" );

		cmd( "pack $b.f1 $b.f2 $b.f3 $b.f4 $b.f5 $b.f6 $b.f7 -padx 5 -pady 5" );

		cmd( "okhelpcancel $b b { set choice 1 } { LsdHelp menurun.html#batch } { set choice 2 }" );

		cmd( "showtop $b" );
		cmd( "mousewarpto $b.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set cores [ $b.f6.e get ]" );

		cmd( "destroytop .batch" );

		Tcl_UnlinkVar( inter, "no_res" );
		Tcl_UnlinkVar( inter, "no_tot" );
		Tcl_UnlinkVar( inter, "dobar" );
		Tcl_UnlinkVar( inter, "docsv" );
		Tcl_UnlinkVar( inter, "doover" );
		Tcl_UnlinkVar( inter, "dozip" );
		Tcl_UnlinkVar( inter, "overwConf" );

		if ( choice == 2 )
			break;

		if ( subDir )
			if ( ! create_res_dir( out_dir ) || ! results_alt_path( out_dir ) )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Subdirectory '%s' cannot be created\" -detail \"Check if the path is set READ-ONLY, or move your configuration file to a different location.\"", out_dir );
				break;
			}

		if ( overwDir && doover )
			clean_res_dir( out_dir );

		if ( sim_num > 1 && param > 1 && no_tot )				// parallel runs case
		{
			param = min( get_int( "cores" ), sim_num );
			param = min( max( param, 1 ), max_threads );		// parallel runs
			nature = max( max_threads / param, 1 );				// threads per run
		}
		else
		{
			param = 1;
			nature = max_threads;
		}

		for ( n = r; n->up != NULL; n = n->up );
		reset_blueprint( n );			// update blueprint to consider last changes

		if ( overwConf )				// save if needed
		{
			if ( ! save_configuration( ) )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s.lsd' cannot be saved\" -detail \"Check if the drive or the file is set READ-ONLY, or try to save to a different location.\"", simul_name	);
				break;
			}
			else
				unsaved_change( false );	// signal no unsaved change
		}

		// start the job
		cmd( "set oldpath [ pwd ]" );
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd $path" );

#ifdef _NP_

		snprintf( lab, MAX_PATH_LENGTH, "%s.log", simul_name );
		cmd( "catch { exec %s -f %s%s%s%s%s%s%s%s -l %s & }", nw_exe, struct_file, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", dobar ? " -b" : "", subDir ? " -o " : "", subDir ? out_dir : "", lab );
		run_logs.clear( );
		run_logs.push_back( lab );

#else

		plog( "\n\nProcessing parallel background run (threads=%d runs=%d)...", nature, param );
		run_parallel( false, nw_exe, simul_name, seed, sim_num, nature, param );

#endif

		show_logs( path, run_logs, true );

		cmd( "set path $oldpath" );
		cmd( "cd $path" );

	break;


	// Load network
	case 88:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and load one before trying to load a network structure file.\"" );
			break;
		}

		cmd( "set bah \"%s\"", simul_name );

		// make sure there is a path set
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		cmd( "set bah [ tk_getOpenFile -parent . -title \"Open Network Structure File\"	 -defaultextension \".net\" -initialdir \"$path\" -initialfile \"$bah.net\" -filetypes { { {Pajek network files} {.net} } { {All files} {*} } } ]" );
		choice = 0;
		cmd( "if { [ string length $bah ] > 0 && ! [ fn_spaces \"$bah\" . ] } { set netPath [ file dirname $bah ]; set netFile [ file tail $bah ]; set posExt [ string last . $netFile ]; if { $posExt >= 0 } { set netExt [ string range $netFile [ expr { $posExt + 1 } ] end ]; set netFile [ string range $netFile 0 [ expr { $posExt - 1 } ] ] } { set netExt \"\" } } { set choice 2 }" );
		if ( choice == 2 )
			break;

		lab1 = get_str( "netPath" );
		lab2 = get_str( "netFile" );
		lab3 = get_str( "netExt" );
		if ( strlen( lab2 ) == 0 )
			break;

		// try to read the object name from network file (in first comment line)
		snprintf( lab, MAX_PATH_LENGTH, "%s%s%s%s%s", lab1, foldersep( lab1 ), lab2, strlen( lab3 ) == 0 ? "" : ".", lab3 );
		strcpy( lab_old, "(none)" );			// no object name yet
		if ( ( f = fopen( lab, "r" ) ) )
		{
			fgets( ch, MAX_LINE_SIZE, f );		// get first line
			sscanf( ch, "%% %99s", lab_old );	// get first string after the comment char
			fclose( f );
		}
		else
		{
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid file or directory\" -detail \"Please check if an existing network structure file (Pajek .net format) was selected.\"" );
			break;
		}

		cmd( "set TT .objs" );
		cmd( "newtop $TT \"Load Network\" { set choice 2 }" );

		cmd( "ttk::frame $TT.l" );
		cmd( "ttk::label $TT.l.l -text \"Suggested object:\"" );
		cmd( "ttk::label $TT.l.n -style hl.TLabel -text \"%s\"", lab_old );
		cmd( "pack $TT.l.l $TT.l.n -padx 2" );

		cmd( "ttk::frame $TT.v" );
		cmd( "ttk::label $TT.v.l -justify center -text \"Object representing\nthe network nodes\"" );

		cmd( "ttk::frame $TT.v.t" );
		cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
		cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
		cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
		cmd( "mouse_wheel $TT.v.t.lb" );
		insert_object( "$TT.v.t.lb", root );
		cmd( "pack $TT.v.l $TT.v.t" );

		cmd( "pack $TT.l $TT.v -padx 5 -pady 5" );

		cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

		cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0; break }" );
		cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end; break }" );
		cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

		cmd( "showtop $TT" );

		cmd( "set cur 0" );
		if ( ! strcmp( lab_old, "(none)" ) )
		{
			if ( r != NULL )
				strcpyn( lab_old, r->label, MAX_ELEM_LENGTH );
			else
				strcpy( lab_old, "" );
		}
		cmd( "for { set i 0 } { $i < [ $TT.v.t.lb size ] } { incr i } { if [ string equal [ $TT.v.t.lb get $i ] %s ] { set cur $i; break } }", lab_old );
		cmd( "$TT.v.t.lb selection set $cur" );
		cmd( "focus $TT.v.t.lb" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set nodeObj [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ]" );
		cmd( "destroytop .objs" );

		if ( choice == 2 )
			break;

		lab4 = get_str( "nodeObj" );

		plog( "\nLoading network on object '%s' from file %s%s%s%s%s...\n", lab4, lab1, foldersep( lab1 ), lab2, strlen( lab3 ) == 0 ? "" : ".", lab3 );

		cur = root->search( lab4 );
		if ( cur != NULL && cur->up != NULL )
		{
			nLinks = cur->up->read_file_net( lab4, lab1, lab2, -1, lab3 );
			if ( nLinks == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid file or object\" -detail \"Please check the file contents for a valid Pajek network structure file (Pajek .net format) and make sure you select a valid object for attributing the network's nodes role.\"" );
				plog( "Error: No network links created\n" );
			}
			else
				plog( " %ld network links created\n", nLinks );
		}
		else
		{
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid object\" -detail \"Please make sure you select a valid object for attributing the network's nodes role.\"" );
			plog( "Error: No network links created\n" );
		}

	break;


	// Save network
	case 89:

		if ( ! struct_loaded || strlen( simul_name ) == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to save a network structure file.\"" );
			break;
		}

		cmd( "set TT .objs" );
		cmd( "newtop $TT \"Save Network\" { set choice 2 }" );

		cmd( "ttk::frame $TT.v" );
		cmd( "ttk::label $TT.v.l -justify center -text \"Object containing\nthe network nodes\"" );

		cmd( "ttk::frame $TT.v.t" );
		cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
		cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
		cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
		cmd( "mouse_wheel $TT.v.t.lb" );

		insert_object( "$TT.v.t.lb", root, true );
		cmd( "set numNets [ $TT.v.t.lb size ]" );
		if ( get_int( "numNets" ) == 0 )
		{
			cmd( "destroytop .objs" );
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No network object found\" -detail \"Please make sure there are objects set as network nodes before saving the network structure.\"" );
			break;
		}

		cmd( "pack $TT.v.l $TT.v.t" );

		cmd( "pack $TT.v -padx 5 -pady 5" );

		cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

		cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0; break }" );
		cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end; break }" );
		cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

		cmd( "showtop $TT" );

		cmd( "$TT.v.t.lb selection set 0" );
		cmd( "focus $TT.v.t.lb" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set nodeObj [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ]" );
		cmd( "destroytop .objs" );

		if ( choice == 2 )
			break;

		lab4 = get_str( "nodeObj" );
		cur = root->search( lab4 );
		if ( cur == NULL || cur->node == NULL || cur->up == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid object\" -detail \"Please make sure you select an object which is already a node of an existing network.\"" );
			break;
		}

		// make sure there is a path set
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		cmd( "set bah \"%s\"", simul_name );
		cmd( "set bah [ tk_getSaveFile -parent . -title \"Save Network Structure File\"	 -defaultextension \".net\" -initialdir \"$path\" -initialfile \"$bah.net\" -filetypes { { {Pajek network files} {.net} } } ]" );
		choice = 0;
		cmd( "if { [ string length $bah ] > 0 && ! [ fn_spaces \"$bah\" . ] } { set netPath [ file dirname $bah ]; set netFile [ file tail $bah ]; set posExt [ string last . $netFile ]; if { $posExt >= 0 } { set netExt [ string range $netFile [ expr { $posExt + 1 } ] end ]; set netFile [ string range $netFile 0 [ expr { $posExt - 1 } ] ] } { set netExt \"\" } } { set choice 2 }" );
		if ( choice == 2 )
			break;

		lab1 = get_str( "netPath" );
		lab2 = get_str( "netFile" );
		lab3 = get_str( "netExt" );
		if ( strlen( lab2 ) == 0 )
			break;

		plog( "\nSaving network on object '%s' to file %s%s%s%s%s...\n", lab4, lab1, foldersep( lab1 ), lab2, strlen( lab3 ) == 0 ? "" : ".", lab3 );

		nLinks = cur->up->write_file_net( lab4, lab1, lab2, -1 );
		if ( nLinks == 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid file or object\" -detail \"Please check the chosen directory/file for WRITE access and make sure you select a valid object for retrieving the network's nodes.\"" );
			plog( "Error: No network links saved\n" );
		}
		else
			plog( " %ld network links saved\n", nLinks );

	break;


	// Unload network
	case 93:

		if ( ! struct_loaded )
			break;

		if ( ! discard_change( ) )	// check for unsaved configuration changes
			break;

		cmd( "set TT .objs" );
		cmd( "newtop $TT \"Unload Network\" { set choice 2 }" );

		cmd( "ttk::frame $TT.v" );
		cmd( "ttk::label $TT.v.l -justify center -text \"Object containing\nthe network nodes\"" );

		cmd( "ttk::frame $TT.v.t" );
		cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
		cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
		cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
		cmd( "mouse_wheel $TT.v.t.lb" );

		insert_object( "$TT.v.t.lb", root, true );
		cmd( "set numNets [ $TT.v.t.lb size ]" );
		if ( get_int( "numNets" ) == 0 )
		{
			cmd( "destroytop .objs" );
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No network object found\" -detail \"Please make sure there are objects set as network nodes before unloading the network structure.\"" );
			break;
		}

		cmd( "pack $TT.v.l $TT.v.t" );

		cmd( "pack $TT.v -padx 5 -pady 5" );

		cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

		cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0 }" );
		cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end }" );
		cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

		cmd( "showtop $TT" );

		cmd( "$TT.v.t.lb selection set 0" );
		cmd( "focus $TT.v.t.lb" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set nodeObj [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ]" );
		cmd( "destroytop .objs" );

		if ( choice == 2 )
			break;

		lab4 = get_str( "nodeObj" );
		cur = root->search( lab4 );
		if ( cur == NULL || cur->node == NULL || cur->up == NULL )
		{
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid object\" -detail \"Please make sure you select an object which is already a node of an existing network.\"" );
			break;
		}

		plog( "\nRemoving network on object '%s'\n", lab4 );

		cur->up->delete_net( lab4 );

	break;


	// context-menu operation: execute the command in 'ctxMenuCmd'
	case 95:

		if ( get_str( "ctxMenuCmd" ) == NULL )
			break;

		cmd( "eval $ctxMenuCmd" );					// execute command

		cmd( "unset -nocomplain ctxMenuCmd" );
		redrawRoot = redrawStruc = true;			// force browser/structure redraw

	break;


	// toggle the state of the model structure windows, refresh window
	case 70:

		strWindowOn = strWindowOn ? 0 : 1;
		cmd( "set strWindowChk $strWindowOn" );
		cmd( "if { [ winfo exists .m.model ] } { .m.model entryconfig 15 -indicatoron $strWindowChk }" );
		redrawStruc = true;

		if ( strWindowOn )
			cmd( "tooltip::tooltip .bbar.struct \"Hide structure\"" );
		else
			cmd( "tooltip::tooltip .bbar.struct \"Show structure\"" );

	break;


	// refresh structure windows
	case 23:

		redrawStruc = true;

	break;


	// present parallel run log
	case 8:

#ifndef _NP_

		// destroy monitor thread
		if ( run_monitor.joinable( ) )
			run_monitor.join( );

		plog( "\n%s\n", run_log.c_str( ) );
		plog( "Finished parallel background run\n" );

#endif

	break;


	default:

		plog( "\nWarning: choice %d not recognized", choice );
	}

	choice = 0;
	return r;
}


/****************************************************
SHOW_SAVE
****************************************************/
void show_save( object *n )
{
	char out[ 3 * MAX_ELEM_LENGTH ];
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
	{
		if ( cv->save == 1 || cv->savei == 1 )
		{
			if ( cv->param == 1 )
				snprintf( out, 3 * MAX_ELEM_LENGTH, "Object: %s \tParameter:\t", n->label );
			else
				snprintf( out, 3 * MAX_ELEM_LENGTH, "Object: %s \tVariable :\t", n->label );
			if ( cv->savei == 1 )
			{
				if ( cv->save == 1 )
				   strcatn( out, " (memory and disk)", 3 * MAX_ELEM_LENGTH );
				else
				   strcatn( out, " (disk only)", 3 * MAX_ELEM_LENGTH );
			}
			plog( out );
			plog_tag( "%s\n", "highlight", cv->label );
			++lcount;
		}
	}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		show_save( co );
	}
}


/****************************************************
SHOW_OBSERVE
****************************************************/
void show_observe( object *n )
{
	bridge *cb;
	description *cd;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );
		if ( cd->observe=='y' )
		{
			if ( cv->param == 1 )
				plog( "Object: %s \tParameter:\t", n->label );
			else
				plog( "Object: %s \tVariable :\t", n->label );

			plog_tag( "%s (%lf)\n", "highlight", cv->label, cv->val[ 0 ] );
			++lcount;
		}
	}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		show_observe( co );
	}
}


/****************************************************
SHOW_INITIAL
****************************************************/
void show_initial( object *n )
{
	char buf_descr[ MAX_BUFF_SIZE ];
	bridge *cb;
	object *co;
	description *cd;
	variable *cv, *cv1;

	for ( cv = n->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );
		if ( cd->initial == 'y' )
		{
			if ( cv->param == 1 )
				plog( "Object: %s \tParameter:\t", n->label );
			if ( cv->param == 0 )
				plog( "Object: %s \tVariable :\t", n->label );
			if ( cv->param == 2 )
				plog( "Object: %s \tFunction :\t", n->label );

			++lcount;
			plog_tag( "%s \t", "highlight", cv->label );

			if ( cd->init == NULL || strlen( cd->init ) == 0 )
			{
				for ( co = n; co != NULL; co = co->hyper_next( co->label ) )
				{
					cv1 = co->search_var( NULL, cv->label );
					plog( " %g", cv1->val[ 0 ] );
				}
			}
			else
				plog( "%s", strtcl( buf_descr, cd->init, MAX_BUFF_SIZE ) );

			plog( "\n" );
		}
	}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head != NULL )
		{
			co = cb->head;
			show_initial( co );
		}
	}
}


/****************************************************
SHOW_PLOT
****************************************************/
void show_plot( object *n )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		if ( cv->plot )
		{
			if ( cv->param == 1 )
				plog( "Object: %s \tParameter:\t", n->label );
			if ( cv->param == 0 )
				plog( "Object: %s \tVariable :\t", n->label );
			if ( cv->param == 2 )
				plog( "Object: %s \tFunction :\t", n->label );
			plog_tag( "%s\n", "highlight", cv->label );
			lcount++;
		}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		show_plot( co );
	}
}


/****************************************************
SHOW_DEBUG
****************************************************/
void show_debug( object *n )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		if ( cv->deb_mode != 'n' )
		{
			if ( cv->param == 0 )
				plog( "Object: %s \tVariable:\t", n->label );
			if ( cv->param == 1 )
				plog( "Object: %s \tParameter:\t", n->label );
			if ( cv->param == 2 )
				plog( "Object: %s \tFunction:\t", n->label );

			plog_tag( "%s\t", "highlight", cv->label );

			switch ( cv->deb_mode )
			{
				default:
				case 'd':
					plog( "(debug)\n" );
					break;
				case 'w':
					plog( "(watch)\n" );
					break;
				case 'D':
					plog( "(debug and watch)\n" );
					break;
				case 'r':
					plog( "(watch write)\n" );
					break;
				case 'R':
					plog( "(debug and watch write)\n" );
			}

			lcount++;
		}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		show_debug( co );
	}
}


/****************************************************
SHOW_PARALLEL
****************************************************/
void show_parallel( object *n )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		if ( cv->parallel )
		{
			plog( "Object: %s \tVariable:\t", n->label );
			plog_tag( "%s\n", "highlight", cv->label );
			lcount++;
		}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		show_parallel( co );
	}
}


/****************************************************
SHOW_SPECIAL_UPDAT
****************************************************/
void show_special_updat( object *n )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		if ( cv->delay > 0 || cv->delay_range > 0 || cv->period > 1 || cv->period_range > 0 )
		{
			plog( "Object: %s \tVariable:\t", n->label );
			plog_tag( "%s\n", "highlight", cv->label );
			lcount++;
		}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		show_special_updat( co );
	}
}


/****************************************************
CLEAN_DEBUG
****************************************************/
void clean_debug( object *n )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		cv->deb_mode = 'n';

	for ( cb = n->b; cb != NULL; cb = cb->next )
		for ( co = cb->head; co != NULL; co = co->next )
			clean_debug( co );
}


/****************************************************
CLEAN_SAVE
****************************************************/
void clean_save( object *n )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
	{
		cv->save = 0;
		cv->savei = 0;
	}
	for ( cb = n->b; cb != NULL; cb = cb->next )
		for ( co = cb->head; co != NULL; co = co->next )
			clean_save( co );
}


/****************************************************
CLEAN_PLOT
****************************************************/
void clean_plot( object *n )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		cv->plot = false;

	for ( cb = n->b; cb != NULL; cb = cb->next )
		for ( co = cb->head; co != NULL; co = co->next )
			clean_plot( co );
}


/****************************************************
CLEAN_PARALLEL
****************************************************/
void clean_parallel( object *n )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		cv->parallel = false;

	for ( cb = n->b; cb != NULL; cb = cb->next )
		for ( co = cb->head; co != NULL; co = co->next )
			clean_parallel( co );
}


/****************************************************
WIPE_OUT
****************************************************/
void wipe_out( object *d )
{
	object *cur;
	variable *cv;

	cmd( "if [ info exists modObj ] { set pos [ lsearch -exact $modObj %s ]; if { $pos >= 0 } { set modObj [ lreplace $modObj $pos $pos ] } }", d->label );

	change_description( d->label );

	for ( cv = d->v; cv != NULL; cv = cv->next )
	{
		// remove from element lists
		cmd( "if [ info exists modElem ] { set pos [ lsearch -exact $modElem %s ]; if { $pos >= 0 } { set modElem [ lreplace $modElem $pos $pos ] } }", cv->label );
		cmd( "if [ info exists modVar ] { set pos [ lsearch -exact $modVar %s ]; if { $pos >= 0 } { set modVar [ lreplace $modVar $pos $pos ] } }", cv->label );
		cmd( "if [ info exists modPar ] { set pos [ lsearch -exact $modPar %s ]; if { $pos >= 0 } { set modPar [ lreplace $modPar $pos $pos ] } }", cv->label );
		cmd( "if [ info exists modFun ] { set pos [ lsearch -exact $modFun %s ]; if { $pos >= 0 } { set modFun [ lreplace $modFun $pos $pos ] } }", cv->label );

		change_description( cv->label );
	}

	cur = d->hyper_next( d->label );
	if ( cur != NULL )
		wipe_out( cur );

	delete_bridge( d );
}


/****************************************************
CHECK_LABEL
Control that the label lab does not already exist in the model
Also prevents invalid characters in the names
****************************************************/
int check_label( const char *lab, object *r )
{
	bridge *cb;
	object *cur;
	variable *cv;

	if ( ! valid_label( lab ) )
		return 2;				// invalid characters (incl. spaces)

	if ( ! strcmp( lab, r->label ) )
		return 1;

	for ( cv = r->v; cv != NULL; cv = cv->next )
		if ( ! strcmp( lab, cv->label ) )
			return 1;

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = blueprint->search( cb->blabel );
		else
			cur = cb->head;

		if ( check_label( lab, cur ) )
			return 1;
	}

	return 0;
}


/****************************************************
SET_SHORTCUTS
Define keyboard shortcuts to menu items
****************************************************/
void set_shortcuts( const char *window )
{
	cmd( "bind %s <Control-l> { set choice 17 }; bind %s <Control-L> { set choice 17 }", window, window	 );
	cmd( "bind %s <Control-s> { set choice 18 }; bind %s <Control-S> { set choice 18 }", window, window	 );
	cmd( "bind %s <Control-e> { set choice 20 }; bind %s <Control-E> { set choice 20 }", window, window	 );
	cmd( "bind %s <Control-q> { set choice 11 }; bind %s <Control-Q> { set choice 11 }", window, window	 );
	cmd( "bind %s <Control-v> { set param 0; set choice 2 }; bind %s <Control-V> { set param 0; set choice 2 }", window, window	 );
	cmd( "bind %s <Control-p> { set param 1; set choice 2 }; bind %s <Control-P> { set param 1; set choice 2 }", window, window	 );
	cmd( "bind %s <Control-n> { set param 2; set choice 2 }; bind %s <Control-N> { set param 2; set choice 2 }", window, window	 );
	cmd( "bind %s <Control-d> { set choice 3 }; bind %s <Control-D> { set choice 3 }", window, window  );
	cmd( "bind %s <Control-o> { set choice 19 }; bind %s <Control-O> { set choice 19 }", window, window	 );
	cmd( "bind %s <Control-i> { set choice 21 }; bind %s <Control-I> { set choice 21 }", window, window	 );
	cmd( "bind %s <Control-a> { set choice 26 }; bind %s <Control-A> { set choice 26 }", window, window	 );
	cmd( "bind %s <Control-r> { set choice 1 }; bind %s <Control-R> { set choice 1 }", window, window  );
	cmd( "bind %s <Control-m> { set choice 22 }; bind %s <Control-M> { set choice 22 }", window, window	 );
	cmd( "bind %s <Control-f> { set choice 50 }; bind %s <Control-F> { set choice 50 }", window, window	 );
	cmd( "bind %s <Control-u> { set choice 28 }; bind %s <Control-U> { set choice 28 }", window, window	 );
	cmd( "bind %s <Control-g> { set choice 30 }; bind %s <Control-G> { set choice 30 }", window, window	 );
	cmd( "bind %s <Control-b> { set choice 34 }; bind %s <Control-B> { set choice 34 }", window, window	 );
	cmd( "bind %s <Control-z> { set choice 37 }; bind %s <Control-Z> { set choice 37 }", window, window	 );
	cmd( "bind %s <Control-w> { set choice 38 }; bind %s <Control-W> { set choice 38 }", window, window	 );
	cmd( "bind %s <Control-Tab> { set choice 70 }", window	);
}


/****************************************************
CONTROL_TO_COMPUTE
****************************************************/
void control_to_compute( object *r, const char *lab )
{
	bridge *cb;
	object *cur;
	variable *cv;

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( ! check_save )
			return;

		if ( cv->save == 1 )
		{
			cmd( "set res [ ttk::messageBox -parent . -type okcancel -default ok -title Warning -icon warning -message \"Cannot save element\" -detail \"Element '%s' set to be saved but it will not be computed for the Analysis of Results, since object '%s' is not set to be computed.\n\nPress 'OK' to check for more disabled elements or 'Cancel' to proceed without further checking.\" ]", cv->label, lab );
			cmd( "if [ string equal $res cancel ] { set res 1 } { set res 0 }" );

			if ( get_bool( "res" ) )
				check_save = false;
		}
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = blueprint->search( cb->blabel );
		else
			cur = cb->head;

		control_to_compute( cur, lab );
	}
}


/****************************************************
INSERT_OBJECT
****************************************************/
void insert_object( const char *w, object *r, bool netOnly, object *above )
{
	bridge *cb;
	object *cur;

	if ( ( above == NULL || above->up == NULL || ( strcmp( r->label, above->label ) != 0 && strcmp( r->label, above->up->label ) != 0 ) ) &&
		 ( ! netOnly || r->node != NULL ) )
		cmd( "%s insert end %s", w, r->label );

	for ( cb = r->b; cb != NULL; cb = cb->next )
		if ( above == NULL || strcmp( cb->blabel, above->label ) != 0 )
		{
			if ( cb->head == NULL )
				cur = blueprint->search( cb->blabel );
			else
				cur = cb->head;

			insert_object( w, cur, netOnly, above );
		}
}


/****************************************************
SHIFT_VAR
****************************************************/
void shift_var( int direction, const char *vlab, object *r )
{
	variable *cv, *cv1 = NULL, *cv2 = NULL;

	if ( direction == -1 )
	{	// shift up
		if ( ! strcmp( vlab, r->v->label ) )
			return;		// variable already at the top

		if ( ! strcmp( vlab, r->v->next->label ) )
		{	// second var, must become the head of the chain
			cv = r->v->next->next;	// third
			cv1 = r->v;				// first
			r->v = r->v->next;		// shifted up
			r->v->next = cv1;
			cv1->next = cv;
			return;
		}

		for ( cv = r->v; cv != NULL; cv = cv->next )
		{
			if ( ! strcmp( vlab, cv->label ) )
			{
				cv2->next = cv;
				cv1->next = cv->next;
				cv->next = cv1;
				return;
			}

			cv2 = cv1;
			cv1 = cv;
		}
	}

	if ( direction == 1 )
	{	// move down
		if ( ! strcmp( vlab, r->v->label ) )
		{	// it's the first
			if ( r->v->next == NULL )
				return;				// it is unique

			cv = r->v;				// first
			cv1 = cv->next->next;	// third
			r->v = cv->next;		// first is former second
			r->v->next = cv;		// second is former first
			cv->next = cv1;			// second points to third
			return;
		}

		for ( cv = r->v; cv != NULL; cv = cv->next )
		{
			if ( ! strcmp( vlab,cv->label ) )
			{
				if ( cv->next == NULL )
					return;			// already at the end

				cv1->next = cv->next;
				cv->next = cv->next->next;
				cv1->next->next = cv;
				return;
			}

			cv1 = cv;
		}
	}
}


/****************************************************
SHIFT_DESC
****************************************************/
void shift_desc( int direction, const char *dlab, object *r )
{
	bridge *cb, *cb1 = NULL, *cb2 = NULL;

	if ( direction == -1 )
	{	// shift up
		if ( ! strcmp( dlab, r->b->blabel ) )
			return;		// object already at the top

		if ( ! strcmp( dlab, r->b->next->blabel ) )
		{	// second var, must become the head of the chain
			cb = r->b->next->next;	// third
			cb1 = r->b;				// first
			r->b = r->b->next;		// shifted up
			r->b->next = cb1;
			cb1->next = cb;
			return;
		}

		for ( cb = r->b; cb != NULL; cb = cb->next )
		{
			if ( ! strcmp( dlab, cb->blabel ) )
			{
				cb2->next = cb;
				cb1->next = cb->next;
				cb->next = cb1;
				return;
			}

			cb2 = cb1;
			cb1 = cb;
		}
	}

	if ( direction == 1 )
	{	//move down
		if ( ! strcmp( dlab, r->b->blabel ) )
		{	// it's the first
			if ( r->b->next == NULL)
				return;				// it is unique

			cb = r->b;				// first
			cb1 = cb->next->next;	// third
			r->b = cb->next;		// first is former second
			r->b->next = cb;		// second is former first
			cb->next = cb1;			// second points to third
			return;
		}

		for ( cb = r->b; cb != NULL; cb = cb->next )
		{
			if ( ! strcmp( dlab, cb->blabel ) )
			{
				if ( cb->next == NULL )
					return;			// already at the end

				cb1->next = cb->next;
				cb->next = cb->next->next;
				cb1->next->next = cb;
				return;
			}

			cb1 = cb;
		}
	}
}


/****************************************************
SORT_LISTBOX
****************************************************/
bool ascending_objects( const bridge &a, const bridge &b )
{ return ( strcmp( a.blabel, b.blabel ) < 0 ); }

bool descending_objects( const bridge &a, const bridge &b )
{ return ( strcmp( a.blabel, b.blabel ) > 0 ); }

bool ascending_variables( const variable &a, const variable &b )
{ return ( strcmp( a.label, b.label ) < 0 ); }

bool descending_variables( const variable &a, const variable &b )
{ return ( strcmp( a.label, b.label ) > 0 ); }

bool sort_listbox( int box, int order, object *r )
{
	bool first;

	// handle variable/parameter list
	if ( box == 1 )
	{
		if ( r->v == NULL || order < 0 || order > 5 )	// invalid sort?
			return false;

		variable *cv, *cv1 = NULL;
		list < variable > newv, newvV, newvP, newvF;
		list < variable > :: iterator it;

		// move LSD linked list of variables to a C++ linked list
		for ( cv = r->v; cv != NULL; cv = cv1 )
		{
			cv1 = cv->next;

			if ( order < 2 )		// no grouping?
				newv.push_back( *cv );
			else
				switch ( cv->param )
				{
					case 0:			// variable
						newvV.push_back( *cv );
						break;
					case 1:			// parameter
						newvP.push_back( *cv );
						break;
					case 2:			// function
						newvF.push_back( *cv );
				}

			delete cv;
		}

		if ( order < 2 )			// no grouping?
			if ( order == 0 )		// ascending order ?
				newv.sort( ascending_variables );
			else					// descending order
				newv.sort( descending_variables );
		else
		{
			switch ( order )
			{
				case 2:				// ascending order by type (par. first)
					newvV.sort( ascending_variables );
					newvP.sort( ascending_variables );
					newvF.sort( ascending_variables );
					newv.splice( newv.end( ), newvP );
					newv.splice( newv.end( ), newvF );
					newv.splice( newv.end( ), newvV );
					break;

				case 3:				// descending order by type (par. first)
					newvV.sort( descending_variables );
					newvP.sort( descending_variables );
					newvF.sort( descending_variables );
					newv.splice( newv.end( ), newvP );
					newv.splice( newv.end( ), newvF );
					newv.splice( newv.end( ), newvV );
					break;

				case 4:				// ascending order by type (var. first)
					newvV.sort( ascending_variables );
					newvP.sort( ascending_variables );
					newvF.sort( ascending_variables );
					newv.splice( newv.end( ), newvV );
					newv.splice( newv.end( ), newvF );
					newv.splice( newv.end( ), newvP );
					break;

				case 5:				// descending order by type (var. first)
					newvV.sort( descending_variables );
					newvP.sort( descending_variables );
					newvF.sort( descending_variables );
					newv.splice( newv.end( ), newvV );
					newv.splice( newv.end( ), newvF );
					newv.splice( newv.end( ), newvP );
					break;
			}
		}

		// rebuild LSD linked list from C++ list
		for ( first = true, it = newv.begin( ); it != newv.end( ); ++it )
		{
			cv = new variable( *it );
			if ( first )
			{
				r->v = cv;
				first = false;
			}
			else
				cv1->next = cv;
			cv1 = cv;
		}
		cv1->next = NULL;

		r->recreate_maps( );		// recreate the fast look-up maps

		return true;
	}

	// handle object list
	if ( box == 2 )
	{
		if ( r->b == NULL || order < 0 || order > 1 )	// invalid sort?
			return false;

		bridge *cb, *cb1 = NULL;
		list < bridge > newb;
		list < bridge > :: iterator it;

		// move LSD linked list of objects to a C++ linked list
		for ( cb = r->b; cb != NULL; cb = cb1 )
		{
			cb1 = cb->next;
			newb.push_back( *cb );
			cb->copy = true;		// prevent garbage collection
			delete cb;
		}

		if ( order == 0 )			// ascending order ?
			newb.sort( ascending_objects );
		else						// descending order
			newb.sort( descending_objects );

		// rebuild LSD linked list from C++ list
		for ( first = true, it = newb.begin( ); it != newb.end( ); ++it )
		{
			cb = new bridge( *it );
			if ( first )
			{
				r->b = cb;
				first = false;
			}
			else
				cb1->next = cb;
			cb1 = cb;
		}
		cb1->next = NULL;

		r->recreate_maps( );		// recreate the fast look-up maps

		return true;
	}

	return false;
}


/****************************************************
SENSITIVITY_TOO_LARGE
****************************************************/
bool sensitivity_too_large( long numSaPts )
{
	cmd( "set answer [ ttk::messageBox -parent . -type okcancel -icon warning -default cancel -title Warning -message \"Too many cases to perform sensitivity analysis\" -detail \"The required	 number (%ld) of configuration points to perform sensitivity analysis is likely too large to be processed in reasonable time.\n\nPress 'OK' if you want to continue anyway or 'Cancel' to abort the command now.\" ]; switch -- $answer { ok { set choice 0 } cancel { set choice 1 } }", numSaPts );

		return choice;
}


/****************************************************
SENSITIVITY_CLEAN
****************************************************/
bool sensitivity_clean_dir( const char *path )
{
	cmd( "set answer [ ttk::messageBox -parent . -type yesno -icon info -default yes -title \"Sensitivity Analysis\" -message \"Clean output path before proceeding?\" -detail \"The configuration files (.lsd) for sensitivity analysis will be created at:\n\n[ fn_break [ file nativename \"%s\" ] 40 ]\n\nThis subdirectory already contains LSD produced files. Click on 'Yes' to delete the existing files before proceeding or 'No' to just continue without deleting.\" ]; switch -- $answer { no { set choice 0 } yes { set choice 1 } }", path );

		return choice;
}


/****************************************************
SENSITIVITY_CREATED
****************************************************/
void sensitivity_created( const char *path, const char *sim_name, int findex )
{
	cmd( "ttk::messageBox -parent . -type ok -icon info -title \"Sensitivity Analysis\" -message \"Configuration files created\" -detail \"LSD has created configuration files (.lsd) for all the sensitivity analysis required points.\n\nTo run the analysis you have to start the processing of sensitivity configuration files by selecting 'Run'/'Create/Run Parallel Batch...' menu option.\n\nAlternatively, open a command prompt (terminal window) and execute the following command in the directory of the model:\n\n> lsdNW	 -f	 [ fn_break [ file nativename \"%s/%s\" ] 40 ]	-s	%d\"", path, sim_name, findex );
}


/****************************************************
SENSITIVITY_UNDEFINED
****************************************************/
void sensitivity_undefined( void )
{
	cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity analysis items not found\" -detail \"Before using this option you have to select at least one parameter or lagged variable initial value to perform the sensitivity analysis and inform the corresponding values to be explored.\n\nTo set the sensitivity analysis values (or ranges), use the 'Sensitivity Analysis' button in the 'Model'/'Change Element...' menu option (or the corresponding context menu option) and inform the values or range(s) using the syntax explained in the 'Sensitivity Analysis' entry window (it is possible to paste a list of values from the clipboard). You can repeat this procedure for each required parameter or initial value.\n\nSensitivity Analysis values are NOT saved in the standard LSD configuration file (.lsd) and if needed they MUST be saved in a LSD sensitivity analysis file (.sa) using the 'File'/'Save Sensitivity...' menu option.\"" );
}


/****************************************************
LOAD_PREV_CONFIGURATION
Restore sensitivity configuration
****************************************************/
bool load_prev_configuration( void )
{
	char *saFile = NULL;
	int lstFidx = findexSens;
	FILE *f;

	if ( sens_file != NULL )					// save SA file name if one is loaded
	{
		saFile = new char[ strlen( sens_file ) + 1 ];
		strcpy( saFile, sens_file );
	}

	if ( load_configuration( true ) != 0 )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be reloaded\" -detail \"Previously loaded configuration could not be restored. Check if LSD still has access to the model directory.\n\nCurrent configuration will be reset now.\"" );

		unload_configuration( true );			// full unload everything
		return false;
	}

	if ( saFile != NULL )						// restore SA configuration, if any
	{
		f = fopen( saFile, "rt" );
		if ( f == NULL || load_sensitivity( f ) != 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity analysis file cannot be reloaded\" -detail \"Previously loaded SA configuration could not be restored. Check if LSD still has access to the model directory.\n\nCurrent configuration will be reset now.\"" );
			return false;
		}

		if ( f != NULL )
			fclose( f );

		delete [ ] saFile;
	}

	findexSens = lstFidx;

	return true;
}


/****************************************************
SAVE_POS
Save user position in browser
****************************************************/
void save_pos( object *r )
{
	if ( ! eval_bool( "[ winfo exists .l.s.c.son_name ]" ) )
		return;										// browser not drawn yet

	// save the current object & cursor position for quick reload
	cmd( "set lastObj %s", r != NULL ? r->label : root->label );

	cmd( "if { ! [ string equal [ .l.s.c.son_name curselection ] \"\" ] } { \
				set lastList 2 \
			} else { \
				set lastList 1 \
			}" );

	cmd( "if { $lastList == 1 } { \
			set lastItem [ .l.v.c.var_name curselection ]; \
			set lastFirst [ lindex [ .l.v.c.var_name yview ] 0 ] \
		} else { \
			set lastItem [ .l.s.c.son_name curselection ]; \
			set lastFirst [ lindex [ .l.s.c.son_name yview ] 0 ] \
		}" );

	cmd( "if { $lastItem == \"\" } { set lastItem 0 }" );
}


/****************************************************
RESTORE_POS
Restore user position in browser
****************************************************/
object *restore_pos( object *r )
{
	object *cur;

	if ( r != NULL && eval_bool( "$lastObj ne \"\"" ) )
	{
		cur = root->search( get_str( "lastObj" ) );
		if ( cur != NULL )
		{
			cmd( "if [ info exists lastList ] { set listfocus $lastList }" );
			cmd( "if [ info exists lastItem ] { set itemfocus $lastItem }" );
			cmd( "if [ info exists lastFirst ] { set itemfirst $lastFirst }" );
			return cur;
		}
	}

	return r;
}


/****************************************************
NEED_RES_DIR
Evaluate if a separated results directory must be
created according to a set of criteria
****************************************************/
#define RES_AVOID_PATTERN "*.cpp *.h *.txt *.R *.o *.exe *.html"
bool need_res_dir( const char *path, const char *sim_name, char *buf, int buf_sz )
{
	bool newDir = false;

	cmd( "if { [ string length \"%s\" ] > 0 } { \
			set f [ file normalize \"%s/%s\" ]; \
		} else { \
			set f [ file normalize \"%s\" ]; \
		}", path, path, sim_name, sim_name );

	cmd( "set s \".*[ file tail $f ]_\\[0-9\\]+\\.lsd$\"" );
	cmd( "set f \"$f.lsd\"" );
	cmd( "set d [ file dirname $f ]" );

	// check if path is valid
	cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { set res 1 } { set res 0 }" );
	if ( get_bool( "res" ) )
	{
		// check if in the main model directory
		cmd( "if { $d eq [ file normalize \"%s\" ] } { set res 1 } { set res 0 }", exec_path );
		if ( get_bool( "res" ) )
			newDir = true;

		// check if we are in a code directory
		cmd( "set l [ glob -nocomplain -directory $d %s ]", RES_AVOID_PATTERN );
		cmd( "if { [ llength $l ] > 0 } { set res 1 } { set res 0 }" );
		if ( get_bool( "res" ) )
			newDir = true;

		// check if the only LSD configuration is the current one or sensitivity version
		cmd( "set l [ glob -nocomplain -directory $d *.lsd ]" );
		cmd( "set l [ lsearch -exact -all -inline -not $l $f ]" );
		cmd( "set l [ lsearch -regexp -all -inline -not $l $s ]" );

		cmd( "if { [ llength $l ] > 0 } { set res 1 } { set res 0 }" );
		if ( get_bool( "res" ) )
			newDir = true;

		if ( newDir )
			cmd( "set d \"$d/[ file tail \"%s\" ]\"", sim_name );
	}
	else
		cmd( "set d \"\"" );

	get_str( "d", buf, buf_sz );

	return newDir;
}


/****************************************************
CHECK_RES_DIR
Check if the results directory exists and
contains files to be deleted
****************************************************/
#define RES_CLEAR_PATTERN "*.res *.tot *.csv *.gz *.log *.bat *.pdf *.eps *.svg *.Rdata *.bak"
bool check_res_dir( const char *path, const char *sim_name )
{
	bool done;

	cmd( "set d \"%s\"", path );

	cmd( "if { [ file exists $d ] && [ file isdirectory $d ] && [ file normalize $d ] ne [ file normalize \"%s\" ] && [ llength [ glob -nocomplain -directory $d %s ] ] > 0 } { set res 1 } { set res 0 }", exec_path, RES_CLEAR_PATTERN );
	done = get_bool( "res" );

	if ( sim_name != NULL )
	{
		cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { \
				set l [ glob -nocomplain -directory $d *.lsd ]; \
				set n [ llength $l ]; \
				if { $n > 1 } { \
					set res 1 \
				} elseif { $n == 1 && [ file normalize [ lindex $l 0 ] ] ne [ file normalize \"$d/%s.lsd\" ] } { \
					set res 1 \
				} else { \
					set res 0 \
				} \
			}", clean_file( sim_name ) );

		done |= get_bool( "res" );
	}

	return done;
}


/****************************************************
CREATE_RES_DIR
Create the results directory, if not exists yet
****************************************************/
bool create_res_dir( const char *path )
{
	cmd( "set d \"%s\"", path );

	cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { set res 1 } { set res 0 }" );
	if ( ! get_bool( "res" ) )
	{
		cmd( "if { [ file exists $d ] } { catch { file delete -force $d } }" );
		cmd( "catch { file mkdir $d }" );
		cmd( "if { ! [ file exists $d ] || ! [ file isdirectory $d ] } { set res 1 } { set res 0 }" );
		if ( get_bool( "res" ) )
			return false;
	}

	return true;
}


/****************************************************
CLEAN_RES_DIR
Clear LSD produced files in the results directory,
if existent,
****************************************************/
void clean_res_dir( const char *path, const char *sim_name )
{
	cmd( "set d \"%s\"", path );

	cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { \
			set l [ glob -nocomplain -directory $d %s ]; \
			if { [ llength $l ] > 0 } { \
				catch { file delete -force {*}$l } \
			} \
		}", RES_CLEAR_PATTERN );

	if ( sim_name != NULL )
		cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { \
				set l [ glob -nocomplain -directory $d *.lsd ]; \
				foreach f $l { \
					if { [ file normalize $f ] ne [ file normalize \"$d/%s.lsd\" ] } { \
						catch { file delete -force $f } \
					} \
				} \
			}", clean_file( sim_name ) );
}


/****************************************************
UNSAVED_CHANGE
Read or set the UnsavedChange flag and update windows titles accordingly
****************************************************/
bool unsavedChange = false;		// control for unsaved changes in configuration
#define WND_NUM 10				// number of windows to update (in wndName)
const char *wndName[ ] = { ".", ".log", ".str", ".inid", ".inin", ".da", ".deb", ".lat", ".plt", ".dap" };

bool unsaved_change( bool val )
{
	if ( unsavedChange != val )
	{
		unsavedChange = val;

#ifndef _NW_
		char chgMark[ ] = "\0\0";
		chgMark[ 0 ] = unsavedChange ? '*' : ' ';

		// change all the possibly open (single) windows
		for ( int i = 0; i < WND_NUM; ++i )
		{
			cmd( "if [ winfo exist %s ] { wm title %s \"%s[ string range [ wm title %s ] 1 end ]\" }", wndName[ i ], wndName[ i ], chgMark, wndName[ i ]  );
		}
#endif
	}

	return unsavedChange;
}

bool unsaved_change( void )
{
	return unsavedChange;
}


/****************************************************
DISCARD_CHANGE
Ask user to discard changes in configuration, if applicable
Returns: 0: abort, 1: continue without saving
****************************************************/
bool discard_change( bool checkSense, bool senseOnly, const char title[ ] )
{
	// don't stop if simulation is running
	if ( running )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Cannot quit LSD\" -detail \"Cannot quit while simulation is running.\n\n Press 'OK' to continue simulation processing. If you really want to abort the simulation, press 'Stop' first.\"" );
		return false;
	}

	// nothing to save?
	if ( ! unsavedData && ! unsavedChange && ! unsavedSense )
		goto end_true;				// yes: simply discard configuration

	// no: ask for confirmation
	if ( ! senseOnly && unsavedData )
		cmd( "set question \"All data generated and not saved will be lost!\nDo you want to continue?\"" );
	else
		if ( ! senseOnly && unsavedChange )
		{
			if (  strlen( simul_name ) > 0 )
				cmd( "set question \"Recent changes to configuration '%s' are not saved!\nDo you want to discard and continue?\"", simul_name );
			else
				cmd( "set question \"Recent changes to current configuration are not saved!\nDo you want to discard and continue?\"" );
		}
		else						// there is unsaved sense data
		{
			if ( checkSense )
				cmd( "set question \"Recent changes to sensitivity data are not saved!\nDo you want to discard and continue?\"" );
			else
				goto end_true;		// checking sensitivity data is disabled
		}

	// must disable because of a bug in Tk when open dialog
	if ( ! brCovered )
	{
		cmd( ".l.s.c.son_name configure -state disabled" );
		cmd( ".l.v.c.var_name configure -state disabled" );
	}

	cmd( "if [ string equal [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Discard data?%s%s\" -detail $question ] yes ] { \
			set res 1 \
		} else { \
			set res 0 \
		}", strlen( title ) != 0 ? "\n\n" : "", title );

	if ( ! brCovered )
	{
		cmd( ".l.s.c.son_name configure -state normal" );
		cmd( ".l.v.c.var_name configure -state normal" );
	}

	if ( ! get_bool( "res" ) )
		return false;

	end_true:

	save_pos( currObj );	// save browser position in structure
	update_model_info( );	// save windows positions if appropriate

	return true;
}


/****************************************************
ABORT_RUN_THREADS
Confirm exiting when there are running threads
Returns: 0: cancel, 1: continue with exit
****************************************************/
bool abort_run_threads( void )
{

#ifndef _NP_

	int ans;

	// confirm aborting running parallel processes
	if ( parallel_monitor )
	{
		cmd( "switch [ ttk::messageBox -parent . -type yesnocancel -default yes -icon warning -title Warning -message \"Abort running simulation?\" -detail \"A set of parallel simulation runs is being executed in background. You may choose to interrupt it now, or let it to continue (results and log files will be produced in the configuration file's directory).\n\nChoose 'Yes' to abort before exiting, 'No' to exit without aborting, or 'Cancel' to just return to LSD.\" ] { \
				yes { set ans 2 } \
				no { set ans 1 } \
				cancel { set ans 0 } \
			}" );

		ans = get_int( "ans" );

		if ( ans == 2 )
			if ( ! stop_parallel( ) )
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Failed to abort running simulation\" -detail \"LSD is exiting but the parallel simulation runs will continue (results and log files will be produced in the configuration file's directory).\"" );

		if ( ans == 1 )
			detach_parallel( );

		if ( ans == 0 )
			return false;
		else
			return true;
	}

#endif

	return true;
}


/****************************************************
 TCL_ABORT_RUN_THREADS
 Entry point function for access from the Tcl interpreter
 ****************************************************/
int Tcl_abort_run_threads( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	if ( abort_run_threads( ) == 1 )
		Tcl_SetResult( inter, ( char * ) "ok", TCL_VOLATILE );
	else
		Tcl_SetResult( inter, ( char * ) "cancel", TCL_VOLATILE );

	return TCL_OK;
}


/****************************************************
TCL_GET_VAR_DESCR
Function to get variable description on
equation file(s) from Tcl
****************************************************/
int Tcl_get_var_descr( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ], desc[ MAX_BUFF_SIZE ];

	if ( argc != 2 )						// require 1 parameter: variable name
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || strlen( argv[ 1 ] ) == 0 )
		strcpy( desc, "" );				// empty name: do nothing
	else
	{
		sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
		get_var_descr( vname, desc, MAX_BUFF_SIZE );
	}

	Tcl_SetResult( inter, desc, TCL_VOLATILE );
	return TCL_OK;
}


/****************************************************
TCL_GET_VAR_CONF
Function to get variable configuration from Tcl
****************************************************/
int Tcl_get_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ], res[ 2 ];
	variable *cv;

	if ( argc != 3 )					// require 2 parameters: variable name and property
		return TCL_ERROR;

	if ( currObj == NULL || argv[ 1 ] == NULL || argv[ 2 ] == NULL ||
		 ! strcmp( argv[ 1 ], "(none)" ) )
		return TCL_ERROR;

	sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
	cv = currObj->search_var( NULL, vname );

	if ( cv == NULL )					// variable not found
		return TCL_ERROR;

	// get the appropriate value for variable
	res[ 1 ] = '\0';					// default is 1 char string array
	if ( ! strcmp( argv[ 2 ], "save" ) )
		res[ 0 ] = cv->save ? '1' : '0';
	else
		if ( ! strcmp( argv[ 2 ], "plot" ) )
			res[ 0 ] = cv->plot ? '1' : '0';
		else
			if ( ! strcmp( argv[ 2 ], "debug" ) )
				res[ 0 ] = ( cv->deb_mode == 'd' || cv->deb_mode == 'W' || cv->deb_mode == 'R' ) ? '1' : '0';
			else
				if ( ! strcmp( argv[ 2 ], "watch" ) )
					res[ 0 ] = ( cv->deb_mode == 'w' || cv->deb_mode == 'W' ) ? '1' : '0';
				else
					if ( ! strcmp( argv[ 2 ], "watch_write" ) )
						res[ 0 ] = ( cv->deb_mode == 'r' || cv->deb_mode == 'R' ) ? '1' : '0';
					else
						if ( ! strcmp( argv[ 2 ], "parallel" ) )
							res[ 0 ] = cv->parallel ? '1' : '0';
						else
							return TCL_ERROR;

	Tcl_SetResult( inter, res, TCL_VOLATILE );
	return TCL_OK;
}


/****************************************************
TCL_SET_VAR_CONF
Function to set variable configuration from Tcl
****************************************************/
int Tcl_set_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ];
	variable *cv;
	object *cur;

	if ( argc != 4 )					// require 3 parameters: variable name, property and value
		return TCL_ERROR;

	if ( currObj == NULL || argv[ 1 ] == NULL || argv[ 2 ] == NULL ||
		 argv[ 3 ] == NULL || ! strcmp( argv[ 1 ], "(none)" ) )
		return TCL_ERROR;

	sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
	cv = currObj->search_var( NULL, vname );

	if ( cv == NULL )					// variable not found
		return TCL_ERROR;

	// set the appropriate value for variable (all instances)
	for ( cur = currObj; cur != NULL; cur = cur->hyper_next( cur->label ) )
	{
		cv = cur->search_var( NULL, vname );
		if ( ! strcmp( argv[ 2 ], "save" ) )
			cv->save = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;
		else
			if ( ! strcmp( argv[ 2 ], "savei" ) )
				cv->savei = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;
			else
				if ( ! strcmp( argv[ 2 ], "plot" ) )
					cv->plot = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;
				else
					if ( ! strcmp( argv[ 2 ], "debug" ) )
					{
						if ( ! strcmp( argv[ 3 ], "1" ) )
						{
							if ( cv->deb_mode == 'n' )
								cv->deb_mode = 'd';
							else
								if ( cv->deb_mode == 'w' )
									cv->deb_mode = 'W';
								else
									if ( cv->deb_mode == 'r' )
										cv->deb_mode = 'R';
						}
						else
						{
							if ( cv->deb_mode == 'd' )
								cv->deb_mode = 'n';
							else
								if ( cv->deb_mode == 'W' )
									cv->deb_mode = 'w';
								else
									if ( cv->deb_mode == 'R' )
										cv->deb_mode = 'r';
						}
					}
					else
						if ( ! strcmp( argv[ 2 ], "watch" ) )
						{
							if ( ! strcmp( argv[ 3 ], "1" ) )
							{
								if ( cv->deb_mode == 'n' || cv->deb_mode == 'r' )
									cv->deb_mode = 'w';
								else
									if ( cv->deb_mode == 'd' || cv->deb_mode == 'R' )
										cv->deb_mode = 'W';
							}
							else
							{
								if ( cv->deb_mode == 'w' || cv->deb_mode == 'r' )
									cv->deb_mode = 'n';
								else
									if ( cv->deb_mode == 'W' || cv->deb_mode == 'R' )
										cv->deb_mode = 'd';
							}
						}
						else
							if ( ! strcmp( argv[ 2 ], "watch_write" ) )
							{
								if ( ! strcmp( argv[ 3 ], "1" ) )
								{
									if ( cv->deb_mode == 'n' || cv->deb_mode == 'w' )
										cv->deb_mode = 'r';
									else
										if ( cv->deb_mode == 'd' || cv->deb_mode == 'W' )
											cv->deb_mode = 'R';
								}
								else
								{
									if ( cv->deb_mode == 'r' )
										cv->deb_mode = 'n';
									else
										if ( cv->deb_mode == 'R' )
											cv->deb_mode = 'd';
								}
							}
							else
								if ( ! strcmp( argv[ 2 ], "parallel" ) )
									cv->parallel  = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;
								else
									return TCL_ERROR;
	}

	unsaved_change( true );				// signal unsaved change
	redrawReq = true;

	if ( ( ! strcmp( argv[ 2 ], "save" ) && cv->save ) ||
		 ( ! strcmp( argv[ 2 ], "savei" ) && cv->savei ) )
	{
		for ( cur = currObj; cur != NULL; cur = cur->up )
			if ( ! cur->to_compute )
			{
				cmd( "ttk::messageBox -parent . -type ok -title Warning -icon warning -message \"Cannot save element\" -detail \"Element '%s' set to be saved but it will not be computed for the Analysis of Results, since object '%s' is not set to be computed.\"", vname, cur->label );
				break;
			}
	}

	return TCL_OK;
}


/****************************************************
TCL_GET_OBJ_CONF
Function to get object configuration from Tcl
****************************************************/
int Tcl_get_obj_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ], res[ 2 ];
	object *cur;

	if ( argc != 3 )					// require 2 parameters: variable name and property
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || argv[ 2 ] == NULL || ! strcmp( argv[ 1 ], "(none)" ) )
		return TCL_ERROR;

	sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
	cur = root->search( vname );

	if ( cur == NULL )					// variable not found
		return TCL_ERROR;

	// get the appropriate value for variable
	res[ 1 ] = '\0';					// default is 1 char string array
	if ( ! strcmp( argv[ 2 ], "comp" ) )
		res[ 0 ] = cur->to_compute ? '1' : '0';
	else
		return TCL_ERROR;

	Tcl_SetResult( inter, res, TCL_VOLATILE );
	return TCL_OK;
}


/****************************************************
TCL_SET_OBJ_CONF
Function to set object configuration from Tcl
****************************************************/
int Tcl_set_obj_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ];
	object *cur, *cur1;

	if ( argc != 4 )					// require 3 parameters: variable name, property and value
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || argv[ 2 ] == NULL ||
		 argv[ 3 ] == NULL || ! strcmp( argv[ 1 ], "(none)" ) )
		return TCL_ERROR;

	sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
	cur = root->search( vname );

	if ( cur == NULL )					// variable not found
		return TCL_ERROR;

	// set the appropriate value for variable (all instances)
	for ( check_save = true, cur1 = cur; cur1 != NULL; cur1 = cur1->hyper_next( cur1->label ) )
		if ( ! strcmp( argv[ 2 ], "comp" ) )
		{
			cur1->to_compute = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;

			if ( ! cur1->to_compute && check_save )
			{
				// control for elements to save in objects to be not computed
				control_to_compute( cur, cur->label );
				check_save = false;		// do it just once
			}
		}
		else
			return TCL_ERROR;

	unsaved_change( true );				// signal unsaved change
	redrawReq = true;

	return TCL_OK;
}
