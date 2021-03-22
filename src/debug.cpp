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
DEBUG.CPP
Builds and manages the debug window. 

This window appears under two conditions:
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

The main functions contained in this file are:

- int deb( object *r, object *c, char *lab, double *res, bool interact )
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

- void deb_show( object *r, const char *hl_var )
fill in all the content of the object.
*************************************************************/

#include "decl.h"

lsdstack *asl = NULL;			// debug stack
object *debLstObj;				// last object shown


/*******************************************
DEB
********************************************/
int deb( object *r, object *c, char const *lab, double *res, bool interact, const char *hl_var )
{
	bool pre_running, redraw;
	char ch[ 4 * MAX_ELEM_LENGTH ], *ch1, ch2[ MAX_ELEM_LENGTH ];
	int i, j, k, count, cond, eff_lags;
	double value_search, app_res, *app_values;
	long node;
	object *cur, *cur1, *cur2;
	bridge *cb, *cb1;
	variable *cv, *cv1;

	// define the presentation mode ( 1 = normal debug, 2 = data browse, 3 = pause debug, 4 = error )
	int mode = ( lab == NULL ) ? 2 : ( ! strcmp( lab, "Paused by User" ) ) ? 3 : ( strstr( lab, "(ERROR)" ) != NULL ) ? 4 : 1; 
	Tcl_SetVar( inter, "lab", lab, 0 );

	if ( mode == 2 )
		cover_browser( "Data Browser...", "Please exit Data Browser\nbefore using the LSD Browser.", false );

	set_buttons_run( false );

	cmd( "set deb .deb" );
	cmd( "if { ! [ winfo exists .deb ] } { \
			if [ string equal $lab \"\" ] { \
				set debTitle \"LSD Data Browser\" \
			} { \
				set debTitle \"LSD Debugger\" \
			}; \
			newtop .deb \"%s%s - $debTitle\" { set choice 7 } \"\"; \
			set newDeb true \
		}", unsaved_change() ? "*" : " ", simul_name  );

	// avoid redrawing the menu if it already exists and is configured
	cmd( "set existMenu [ winfo exists .deb.m ]" );
	cmd( "set confMenu [ .deb cget -menu ]" );
	if ( ! strcmp( Tcl_GetVar( inter, "existMenu", 0 ), "0" ) ||
		 strcmp( Tcl_GetVar( inter, "confMenu", 0 ), ".deb.m" ) )
	{
		cmd( "destroy .deb.m" );
		cmd( "ttk::menu .deb.m -tearoff 0" );
		cmd( "set w .deb.m.exit" );
		cmd( ".deb.m add cascade -label Exit -menu $w -underline 0" );
		cmd( "ttk::menu $w -tearoff 0" );
		if ( mode == 3 )
			cmd( "$w add command -label \"Quit and Resume Simulation\" -command { set choice 7 } -underline 0 -accelerator Esc" );
		else
			cmd( "$w add command -label \"Quit and Return to Browser\" -command { set choice 7 } -underline 0 -accelerator Esc" );
		cmd( "set w .deb.m.find" );
		cmd( "ttk::menu $w -tearoff 0" );
		cmd( ".deb.m add cascade -label Find -menu $w -underline 0" );
		cmd( "$w add command -label \"Find Element...\" -underline 0 -accelerator F -command { set choice 18 }" );
		cmd( "$w add command -label \"Find Object Containing...\" -underline 0 -accelerator Ctrl+F -command { set choice 10 }" );
		cmd( "$w add separator" );
		cmd( "$w add command -label \"Clear Highlighted\" -underline 0 -accelerator Del -command { set choice 19 }" );
		cmd( "set w .deb.m.help" );
		cmd( "ttk::menu $w -tearoff 0" );
		cmd( ".deb.m add cascade -label Help -menu $w -underline 0" );
		cmd( "$w add command -label \"Help on Debugger\" -underline 0 -accelerator F1 -command { LsdHelp debug.html }" );
		cmd( "$w add command -label \"LSD Quick Help\" -underline 4 -command { LsdHelp LSD_quickhelp.html }" );
		cmd( "$w add command -label \"LSD Documentation\" -underline 4 -command { LsdHelp LSD_documentation.html }" );
		cmd( "$w add separator" );
		cmd( "$w add command -label \"LMM Primer Tutorial\" -underline 4 -command { LsdHelp LMM_primer.html }" );
		cmd( "$w add command -label \"Using LSD Models Tutorial\" -underline 0 -command { LsdHelp model_using.html }" );
		cmd( "$w add command -label \"Writing LSD Models Tutorial\" -underline 0 -command { LsdHelp model_writing.html }" );
		cmd( "$w add separator" );
		cmd( "$w add command -label \"Model Report\" -underline 0 -command { set choice 44 }" );
		cmd( "$w add separator" );
		cmd( "$w add command -label \"About LSD...\" -underline 0 -command { LsdAbout {%s} {%s} .deb }", _LSD_VERSION_, _LSD_DATE_ ); 
		cmd( ".deb configure -menu .deb.m" );
		
		cmd( "bind .deb <Control-f> { set choice 10 }; bind .deb <Control-F> { set choice 10 }" );
		cmd( "bind .deb <Delete> { set choice 19 }" );
		cmd( "bind .deb <F1> { .deb.m.help invoke 0; break }" );
	}

	// avoid redrawing the buttons if they already exist
	cmd( "set existButtons [ expr [ winfo exists .deb.b ] ]" );
	if ( ! strcmp( Tcl_GetVar( inter, "existButtons", 0 ), "0" ) )
	{ 
		cmd( "if [ string equal $CurPlatform mac ] { \
				set butWidD [ expr $butWid - 1 ] \
			} { \
				set butWidD $butWid \
			}" );
		cmd( "destroy .deb.b" );

		cmd( "ttk::frame .deb.b" );
		
		// first row of buttons (always shown)
		cmd( "ttk::frame .deb.b.move" );

		cmd( "ttk::button .deb.b.move.up -width $butWidD -text Up -command { set choice 3 } -underline 0" );
		cmd( "ttk::button .deb.b.move.down -width $butWidD -text Down -command { set choice 6 } -underline 0" );
		cmd( "ttk::button .deb.b.move.prev -width $butWidD -text Previous -command { set choice 12 } -underline 0" );
		cmd( "ttk::button .deb.b.move.broth -width $butWidD -text Next -command { set choice 4 } -underline 0" );
		cmd( "ttk::button .deb.b.move.hypern -width $butWidD -text \"Next Type\" -command { set choice 5 } -underline 5" );
		cmd( "ttk::button .deb.b.move.last -width $butWidD -text Last -command { set choice 14 } -underline 0" );
		cmd( "ttk::button .deb.b.move.search -width $butWidD -text Find -command { set choice 18 } -underline 0" );
		cmd( "ttk::button .deb.b.move.hook -width $butWidD -text Hooks -command { set choice 21 } -underline 0" );
		cmd( "ttk::button .deb.b.move.net -width $butWidD -text Network -command { set choice 22 } -underline 3" );
		
		cmd( "pack .deb.b.move.up .deb.b.move.down .deb.b.move.prev .deb.b.move.broth .deb.b.move.hypern .deb.b.move.last .deb.b.move.search .deb.b.move.hook .deb.b.move.net -padx $butSpc -side left" );
		
		cmd( "bind .deb <KeyPress-u> { .deb.b.move.up invoke }; bind .deb <KeyPress-U> { .deb.b.move.up invoke }" );
		cmd( "bind .deb <Up> { .deb.b.move.up invoke }" );
		cmd( "bind .deb <KeyPress-n> { .deb.b.move.broth invoke }; bind .deb <KeyPress-N> { .deb.b.move.broth invoke }" );
		cmd( "bind .deb <Right> { .deb.b.move.broth invoke }" );
		cmd( "bind .deb <KeyPress-t> { .deb.b.move.hypern invoke }; bind .deb <KeyPress-T> { .deb.b.move.hypern invoke }" );
		cmd( "bind .deb <KeyPress-l> { .deb.b.move.last invoke} ; bind .deb <KeyPress-L> { .deb.b.move.last invoke }" );
		cmd( "bind .deb <KeyPress-d> { .deb.b.move.down invoke }; bind .deb <KeyPress-D> { .deb.b.move.down invoke }" );
		cmd( "bind .deb <Down> { .deb.b.move.down invoke }" );
		cmd( "bind .deb <KeyPress-h> { set choice 21 }; bind .deb <KeyPress-H> { set choice 21 }" );
		cmd( "bind .deb <KeyPress-w> { set choice 22 }; bind .deb <KeyPress-W> { set choice 22 }" );
		cmd( "bind .deb <KeyPress-f> { .deb.b.move.search invoke }; bind .deb <KeyPress-F> { .deb.b.move.search invoke }" );
		cmd( "bind .deb <KeyPress-p> { .deb.b.move.prev invoke }; bind .deb <KeyPress-P> { .deb.b.move.prev invoke }" );
		cmd( "bind .deb <Left> { .deb.b.move.prev invoke }" );
		cmd( "bind .deb <KeyPress-Escape> { set choice 7 }" );

		// second row of buttons (if applicable)
		if ( mode == 1 || mode == 3 )
		{
			cmd( "set stack_flag %d", stack_info );
			
			cmd( "ttk::frame .deb.b.act" );
			
			if ( mode == 1 )
			{
				cmd( "ttk::button .deb.b.act.run -width $butWidD -text Run -command { set choice 2; set_c_var done_in 0 } -underline 0" );
				cmd( "ttk::button .deb.b.act.until -width $butWidD -text Until -command { set choice 16; set_c_var done_in 0 } -underline 3" );
				cmd( "ttk::button .deb.b.act.ok -width $butWidD -text Step -command { set choice 1; set_c_var done_in 3 } -underline 0" );
				cmd( "ttk::button .deb.b.act.call -width $butWidD -text Caller -command { set choice 9 } -underline 0" );
				cmd( "ttk::button .deb.b.act.prn_v -width $butWidD -text \"v\\\[...\\]\" -command { set choice 15 } -underline 0" );
				
				cmd( "bind .deb <KeyPress-r> { .deb.b.act.run invoke }; bind .deb <KeyPress-R> { .deb.b.act.run invoke }; bind .deb <F5> { .deb.b.act.run invoke }" );
				cmd( "bind .deb <KeyPress-i> { .deb.b.act.until invoke }; bind .deb <KeyPress-I> { .deb.b.act.until invoke }; bind .deb <F7> { .deb.b.act.until invoke }" );
				cmd( "bind .deb <KeyPress-s> { .deb.b.act.ok invoke }; bind .deb <KeyPress-S> { .deb.b.act.ok invoke }; bind .deb <F8> { .deb.b.act.ok invoke }" );
				cmd( "bind .deb <KeyPress-c> { .deb.b.act.call invoke }; bind .deb <KeyPress-C> { .deb.b.act.call invoke }; bind .deb <F6> { .deb.b.act.call invoke }" );
				cmd( "bind .deb <KeyPress-v> { .deb.b.act.prn_v invoke }; bind .deb <KeyPress-V> { .deb.b.act.prn_v invoke }" );
			}
			
			cmd( "ttk::button .deb.b.act.an -width $butWidD -text Analysis -command { set choice 11 } -underline 0" );
			cmd( "ttk::button .deb.b.act.prn_stck -width $butWidD -text Stack -command { set choice 13 } -underline 4" );
			
			cmd( "bind .deb <KeyPress-a> { .deb.b.act.an invoke }; bind .deb <KeyPress-A> { .deb.b.act.an invoke }" );
			cmd( "bind .deb <KeyPress-k> { .deb.b.act.prn_stck invoke }; bind .deb <KeyPress-K> { .deb.b.act.prn_stck invoke }" );
			
			cmd( "ttk::frame .deb.b.act.stack" );
			cmd( "ttk::label .deb.b.act.stack.l -text \"Stack level\"" );
			cmd( "ttk::spinbox .deb.b.act.stack.e -width 3 -from 0 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 && $n <= 99 } { set stack_flag %%P; return 1 } { %%W delete 0 end; %%W insert 0 $stack_flag; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".deb.b.act.stack.e insert 0 $stack_flag" ); 
			cmd( "pack .deb.b.act.stack.l .deb.b.act.stack.e -padx 2 -side left" );
			
			if ( mode == 1 )
				cmd( "pack .deb.b.act.run .deb.b.act.until .deb.b.act.ok .deb.b.act.call .deb.b.act.prn_v .deb.b.act.an .deb.b.act.prn_stck .deb.b.act.stack -padx $butSpc -side left" );
			else
				cmd( "pack .deb.b.act.an .deb.b.act.prn_stck .deb.b.act.stack -padx $butSpc -side left" );
		

			cmd( "ttk::frame .deb.b.pad" );
			
			cmd( "pack .deb.b.move -anchor e" );
			cmd( "pack .deb.b.pad -pady $butSpc -anchor e" );
			cmd( "pack .deb.b.act -anchor e" );
		}
		else
			cmd( "pack .deb.b.move -anchor e" );		
	}

	app_res = *res;
	Tcl_LinkVar( inter, "value", ( char * ) &app_res, TCL_LINK_DOUBLE );
	cmd( "set value_change 0" );

	redraw = true;
	choice = 0;

	while ( choice == 0 )
	{
		if ( redraw )
		{
			// if necessary, create the variable name and the time info bar
			if ( mode == 1 || mode == 4 )
			{
				cmd( "if { ! [ winfo exists .deb.v ] } { \
						ttk::frame .deb.v; \
						ttk::frame .deb.v.v1; \
						ttk::label .deb.v.v1.name1 -text \"Variable:\"; \
						ttk::label .deb.v.v1.name2 -width 20 -anchor w -style hl.TLabel -text \"\"; \
						ttk::label .deb.v.v1.time1 -text \"Time step:\"; \
						ttk::label .deb.v.v1.time2 -width 5 -anchor w -style hl.TLabel; \
						ttk::label .deb.v.v1.val1 -text \"Value \"; \
						ttk::entry .deb.v.v1.val2 -width 15 -justify center -state disabled -validate key -validatecommand { \
							set n %%P; \
							if [ regexp {^[0-9eE.+-]*$} \"$n\" ] { \
								set value_temp $n; \
								set value_change 1; \
								return 1 \
							} { \
								%%W delete 0 end; \
								if { $value_change } { \
									%%W insert 0 $value_temp \
								} { \
									%%W insert 0 $value \
								}; \
								return 0 \
							} \
						}; \
						bind .deb.v.v1.val2 <Left> { \
							set c [ .deb.v.v1.val2 index insert ]; \
							if { $c > 0 } { \
								incr c -1; \
								.deb.v.v1.val2 icursor $c \
							}; \
							break \
						}; \
						bind .deb.v.v1.val2 <Right> { \
							set c [ .deb.v.v1.val2 index insert ]; \
							if { $c < [ string length [ .deb.v.v1.val2 get ] ] } { \
								incr c; \
								.deb.v.v1.val2 icursor $c \
							}; \
							break \
						}; \
						ttk::label .deb.v.v1.obs -text \"\"; \
						if { %d == 1 } { \
							pack .deb.v.v1.name1 .deb.v.v1.name2 .deb.v.v1.time1 .deb.v.v1.time2 .deb.v.v1.val1 .deb.v.v1.val2 .deb.v.v1.obs -side left; \
							bind .deb <KeyPress-g> { set choice 77 }; \
							bind .deb <KeyPress-G> { set choice 77 } \
						} { \
							pack .deb.v.v1.name1 .deb.v.v1.name2 .deb.v.v1.time1 .deb.v.v1.time2 -side left \
						} \
					}", mode );

				cmd( ".deb.v.v1.name2 conf -text \"%s\"", lab );
				cmd( ".deb.v.v1.time2 conf -text \"%d      \"", t );
			}

			// populate the element list
			deb_show( r, hl_var );
			debLstObj = r;

			cmd( "pack .deb.b -padx $butPad -pady $butPad -side right -after .deb.cc" );

			cmd( "if { $newDeb } { showtop .deb topleftW; set newDeb false } { focustop .deb }" );
			cmd( "set debDone 1" );
			cmd( "event generate .deb <Configure>" );	// resize canvas as window is mapped now

			// update variable label field
			cmd( "if [ winfo exists .deb.v.v1.name1 ] { \
					if { %d == 0 } { \
						.deb.v.v1.name1 configure -text \"Variable:\" \
					} { \
						.deb.v.v1.name1 configure -text \"Message:\" \
					} \
				} ", non_var ? 1 : 0 );

			// update observations field
			cmd( "if [ winfo exists .deb.v.v1.obs ] { \
					if { %d == 0 } { \
						.deb.v.v1.obs configure -text \"     (enter new value to change variable)\" \
					} { \
						.deb.v.v1.obs configure -text \"     (enter value and click Run or press Enter to continue)\" \
					} \
				} ", non_var ? 1 : 0 );

			// disable or enable the caller button
			if ( mode == 1 )
			{
				if( c == NULL )
					cmd( ".deb.b.act.call configure -state disabled" );
				else
					cmd( ".deb.b.act.call configure -state normal" );
			}
				
			// disable or enable the hook button
			if( r->hook == NULL && r->hooks.size( ) == 0 )
				cmd( ".deb.b.move.hook configure -state disabled" );
			else
				cmd( ".deb.b.move.hook configure -state normal" );
				
			// update the temporary variables watch window
			cmd( "set existVal [ winfo exists .deb.val ]" );
			if ( ! strcmp( Tcl_GetVar( inter, "existVal", 0 ), "1" ) )
					show_tmp_vars( r, true );

			// remove or update the network window
			if ( r->node == NULL )
			{
				cmd( "destroytop .deb.net" );
				cmd( ".deb.b.move.net configure -state disabled" );
			}
			else
			{
				cmd( "set existNet [ winfo exists .deb.net ]" );
				if ( ! strcmp( Tcl_GetVar( inter, "existNet", 0 ), "1" ) )
					show_neighbors( r, true );
				cmd( ".deb.b.move.net configure -state normal" );
			}
			
			ch[ 0 ] = '\0';
			attach_instance_number( ch, r );

			asl = NULL;

			if ( mode == 1 )
			{
				cmd( "write_any .deb.b.act.stack.e $stack_flag" ); 
				
				if ( interact )
					cmd( "if { ! $value_change } { \
							.deb.v.v1.val2 configure -state normal; \
							.deb.v.v1.val2 delete 0 end; \
							catch { .deb.v.v1.val2 insert 0 [ format %%g $value ] }; \
							bind .deb.v.v1.val2 <Return> { .deb.b.act.run invoke } \
						}" );
				else
					cmd( "catch { write_any .deb.v.v1.val2 [ format %%g $value ] }" ); 
			}

			// resize the scrollbar if needed and ajust position
			cmd( "set debDone 2" );
			cmd( "event generate .deb <Configure>" );
			cmd( "if { $lastHl != \"\" } { \
					if { $hlPos < [ lindex [ .deb.cc.grid.can yview ] 0 ] + 0.1 || $hlPos > [ lindex [ .deb.cc.grid.can yview ] 1 ] - 0.1 } { \
						.deb.cc.grid.can yview moveto $hlPos \
					} \
				} elseif [ info exists lstDebPos ] { \
					$g.can yview moveto [ lindex $lstDebPos 0 ]; \
				}" );
			cmd( "unset -nocomplain lstDebPos" );
		}
		
		redraw = true;
		
		// debug command loop
		while ( ! choice )
			Tcl_DoOneEvent( 0 );

		if ( mode == 1 )
		{
			cmd( "bind .deb <KeyPress-g> { }; bind .deb <KeyPress-G> { }" );
			cmd( "set stack_flag [ .deb.b.act.stack.e get ]" ); 
			stack_info = get_int( "stack_flag" );
			
			cmd( "if { $value_change } { \
					if [ string is double -strict [ .deb.v.v1.val2 get ] ] { \
						set value [ .deb.v.v1.val2 get ] \
					} { \
						catch { write_any .deb.v.v1.val2 [ format %%g $value ] }; \
						ttk::messageBox -parent .deb -type ok -icon error -title Error -message \"Invalid value\" -detail \"The entered value cannot be converted to a floating point number (with a dot decimal separator). The new value was ignored.\" \
					}; \
					set value_change 0 \
				}" );
		} 

		switch ( choice )
		{
			// Step
			case 1:
				if ( t >= max_step )
				{
					cmd( "destroytop .deb" );
					set_buttons_run( true );
					debug_flag = false;
				}
				break;

			// Run
			case 2:
				cmd( "destroytop .deb" );
				set_buttons_run( true );
				if ( ! non_var )
					debug_flag = false;

				break;

			// Up
			case 3: 
				if ( r->up != NULL )
					choice = deb( r->up, c, lab, res, interact );
				else
					choice = 0;
				break;

			// Next
			case 4: 
				if ( r->next != NULL )
					choice = deb( r->next, c, lab, res, interact );
				else
				{
					cur = skip_next_obj( r );
					if ( cur != NULL )
						choice = deb( cur, c, lab, res, interact );
					else 
						choice = 0;
				} 
				break;

			// Next Type
			case 5:
				cur = skip_next_obj( r, &count );
				if ( cur != NULL )
					choice = deb( cur, c, lab, res, interact );
				else
					choice = 0;

				break;
				
			// Down
			case 6:
				// handle zero instanced objects
				for ( cb = r->b; cb != NULL; cb = cb->next )
					if ( cb->head != NULL )
					{
						choice = deb( cb->head, c, lab, res, interact );
						break;
					}
					
				if ( cb == NULL )
					choice = 0;
				
				break;
				
			// Quit
			case 7:
				if ( mode == 1 )
				{
					cmd( "set answer [ ttk::messageBox -parent .deb -type okcancel -default cancel -icon warning -title Warning -message \"Stop simulation\" -detail \"Quitting the simulation run.\nPress 'OK' to confirm.\" ]; if [ string equal $answer ok ] { set choice 0 } { set choice 1 }" );
				} 
				else
					choice = 0; 

				if ( choice == 1 ) 
				{
					choice = deb( r, c, lab, res, interact );
					break;
				}

				cmd( "destroytop .deb" );
				set_buttons_run( true );
				choice = 1;
				
				switch ( mode )
				{
					case 1:		// prevent changing run parameters when only data browse was called
						quit = 1;
						debug_flag = false;
						break;
						
					case 2:
						uncover_browser( );
						break;

					case 3:
						cmd( "focustop ." );
				}

				break;
				
			// element change (click on parameter/variable)
			case 8:
				ch1 = ( char * ) Tcl_GetVar( inter, "res", 0 );
				Tcl_LinkVar( inter, "debug", ( char * ) &count, TCL_LINK_INT );
				Tcl_LinkVar( inter, "time", ( char * ) &t, TCL_LINK_INT );
				Tcl_LinkVar( inter, "i", ( char * ) &i, TCL_LINK_INT );

				cv = r->search_var( NULL, ch1 );
				i = cv->last_update;
				count = ( cv->debug == 'd' ) ? 1 : 0;
				eff_lags = ( cv->last_update >= cv->num_lag ) ? cv->num_lag : cv->num_lag - 1;
				app_values = new double[ eff_lags + 1 ];
				cmd( "set debugall 0" );
				cmd( "set undebugall 0" );

				cmd( "set e .deb.stat" );
				cmd( "newtop $e \"Element Status\" { set choice 1 } .deb" );

				cmd( "ttk::frame $e.n" );
				switch ( cv->param )
				{
					case 1:
						cmd( "ttk::label $e.n.l -text \"Parameter:\"" );
						break;

					case 2:
						cmd( "ttk::label $e.n.l -text \"Function:\"" );
						break;
						
					case 0:
						cmd( "ttk::label $e.n.l -text \"Variable:\"" );
						break;
				}
				cmd( "ttk::label $e.n.v -style hl.TLabel -text $res" );
				cmd( "pack $e.n.l $e.n.v -side left -padx 2" );

				cmd( "ttk::frame $e.t" );
				cmd( "ttk::label $e.t.l -text \"Current time:\"" );
				cmd( "ttk::label $e.t.v -style hl.TLabel -text $time" );
				cmd( "pack $e.t.l $e.t.v -side left -padx 2" );

				cmd( "ttk::frame $e.u" );
				cmd( "ttk::label $e.u.l -text \"Last update time:\"" );
				cmd( "ttk::label $e.u.v -style hl.TLabel -text %d", cv->last_update );
				cmd( "pack $e.u.l $e.u.v -side left -padx 2" );

				cmd( "ttk::frame $e.x" );
				cmd( "ttk::label $e.x.l -text \"Next update time:\"" );
				cmd( "ttk::label $e.x.v -style hl.TLabel -text %d", cv->next_update > 0 ? cv->next_update : cv->last_update < t ? t : t + 1 );
				cmd( "pack $e.x.l $e.x.v -side left -padx 2" );

				cmd( "ttk::frame $e.v" );
				for ( i = 0; i <= eff_lags; ++i )
				{
					cmd( "set val%d %g", i, cv->val[ i ] );
					app_values[ i ] = cv->val[ i ];
					sprintf( ch, "val%d", i );
					Tcl_LinkVar( inter, ch, ( char * ) &( app_values[ i ] ), TCL_LINK_DOUBLE );

					cmd( "ttk::frame $e.v.l$i" );

					if ( i == 0 )
						cmd( "ttk::label $e.v.l$i.l -text \"Value:\"" );
					else
						cmd( "ttk::label $e.v.l$i.l -text \"Lag $i:\"" );

					cmd( "ttk::entry $e.v.l%d.e -width 15 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set val%d %%P; return 1 } { %%W delete 0 end; %%W insert 0 $val%d; return 0 } } -invalidcommand { bell } -justify center", i, i, i );
					cmd( "$e.v.l%d.e insert 0 $val%d", i, i ); 

					cmd( "ttk::button $e.v.l$i.sa -width 5 -text \"Set All\" -command { set sa %i; set choice 10 }", i );
					cmd( "pack $e.v.l$i.l $e.v.l$i.e $e.v.l$i.sa -side left -padx 2" );
					cmd( "pack $e.v.l$i" );
				}

				if ( cv->param == 1 )
				{
					cmd( "pack $e.n $e.t" );
					cmd( "pack $e.v -pady 5 -padx 5" );
				}
				else
				{
					if ( cv->param == 0 )
						cmd( "pack $e.n $e.t $e.u $e.x" );
					else
						cmd( "pack $e.n $e.t $e.u" );
					
					cmd( "ttk::frame $e.d" );
					cmd( "ttk::checkbutton $e.d.deb -text \"Debug this instance only\" -variable debug" );
					cmd( "ttk::checkbutton $e.d.deball -text \"Debug all instances\" -variable debugall -command { if { $debugall == 1 } { set debug 1; set undebugall 0; .deb.stat.d.deb configure -state disabled } { set debug 0; set undebugall 1; .deb.stat.d.deb configure -state normal } }" );
					cmd( "pack $e.d.deb $e.d.deball" );

					cmd( "pack $e.v $e.d -pady 5 -padx 5" );	

					cmd( "ttk::frame $e.b1" );
					cmd( "ttk::button $e.b1.eq -width $butWid -text Equation -command { set choice 8 }" );
					cmd( "ttk::button $e.b1.cond -width $butWid -text \"Set Break\" -command { set choice 7 }" );
					cmd( "ttk::button $e.b1.exec -width $butWid -text Update -command { set choice 9 }" );
					cmd( "pack $e.b1.eq $e.b1.cond $e.b1.exec -padx $butSpc -side left" );
					cmd( "pack $e.b1 -padx $butPad" );	
				}

				cmd( "donehelp $e b { set choice 1 } { LsdHelp debug.html#content }" );
				cmd( "bind $e.v.l0.e <Return> { set choice 1 }" );

				cmd( "showtop $e" );
				cmd( "focus $e.v.l0.e" );
				cmd( "$e.v.l0.e selection range 0 end" );
				cmd( "mousewarpto $e.b.ok" );

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				cv->data_loaded='+';

				for ( i = 0; i <= eff_lags; ++i )
				{
					cmd( "set val%d [ $e.v.l%d.e get ]", i, i ); 

					cv->val[ i ] = app_values[ i ];
					sprintf( ch, "val%d",i);

					Tcl_UnlinkVar( inter, ch);
					cmd( "unset val$i" );
				}
				delete [ ] app_values;
				Tcl_UnlinkVar( inter, "i");

				cmd( "destroytop $e" );

				cv->debug = ( count == 1 ) ? 'd' : 'n';
				Tcl_UnlinkVar( inter, "debug" );
				count = choice;

				cmd( "if { $debugall || $undebugall } { set choice 1 } { set choice 0 }" );
				if ( choice == 1 )
					for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
					{
						cv1 = cur->search_var( cur, cv->label );
						cv1->debug = cv->debug;
					}
				 
				choice = count;
				if ( choice == 7 )
				{
					cmd( "set cond %d", cv->deb_cond );
					cmd( "set cond_val %4g", cv->deb_cnd_val );

					cmd( "set cb .deb.cbrk" );

					cmd( "newtop $cb \"Conditional Break\" { set choice 2 } .deb" );

					cmd( "ttk::frame $cb.l" );
					cmd( "ttk::label $cb.l.l -text \"Variable:\"" );
					cmd( "ttk::label $cb.l.n -style hl.TLabel -text %s", cv->label );
					cmd( "pack $cb.l.l $cb.l.n -side left -padx 2" );

					cmd( "ttk::frame $cb.t" );
					cmd( "ttk::label $cb.t.l -text \"Type of break\"" );

					cmd( "ttk::frame $cb.t.t -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
					cmd( "ttk::radiobutton $cb.t.t.none -text \"None\" -variable cond -value 0 -command { .deb.cbrk.v.e configure -state disabled }" );
					cmd( "ttk::radiobutton $cb.t.t.eq -text \"=\" -variable cond -value 1 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
					cmd( "ttk::radiobutton $cb.t.t.gr -text \">\" -variable cond -value 2 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
					cmd( "ttk::radiobutton $cb.t.t.le -text \"<\" -variable cond -value 3 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
					cmd( "pack $cb.t.t.none $cb.t.t.eq $cb.t.t.gr $cb.t.t.le -anchor w" );

					cmd( "pack $cb.t.l $cb.t.t" );

					cmd( "ttk::frame $cb.v" );
					cmd( "ttk::label $cb.v.l -text Value" );
					cmd( "ttk::entry $cb.v.e -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set cond_val %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cond_val; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
					cmd( "write_any $cb.v.e $cond_val" ); 
					cmd( "if { $cond != 0 } { $cb.v.e configure -state normal; $cb.v.e selection range 0 end; focus $cb.v.e }" );
					cmd( "pack $cb.v.l $cb.v.e" );

					cmd( "pack $cb.l $cb.t $cb.v -padx 5 -pady 5" );

					cmd( "okhelpcancel $cb b { set choice 1 } { LsdHelp debug.html#cond } { set choice 2 }" );
					cmd( "bind $cb.v.e <Return> { set choice 1 }" );

					cmd( "showtop $cb" );
					cmd( "mousewarpto $cb.b.ok" );

					choice = 0;
					while ( choice == 0 )
						Tcl_DoOneEvent( 0 );

					cmd( "set cond_val [ $cb.v.e get ]" ); 

					cmd( "destroytop $cb" );

					if ( choice == 1 )
					{
						cv->deb_cond = get_int( "cond" );
						cv->deb_cnd_val = get_double( "cond_val" );
					}
				}

				if ( choice == 8 )
				{
					choice = 3;	// point .deb window as parent for the following window
					show_eq( cv->label, &choice );
					choice = 8;
				}

				if ( choice == 9 )
				{
					cur = cv->up;
					cur->cal( cv->label, 0 );
				}

				if ( choice == 10 )
				{
					ch1=( char * ) Tcl_GetVar( inter, "res", 0 );
					strcpy( ch, ch1 );

					cmd( "set choice $sa" );
					i = choice;
					choice = 0;

					set_all( &choice, r, ch, i );
				} 

				choice = 0;
				break;

			// Caller
			case 9:
				if ( c != NULL )
					choice = deb( c, r, lab, res, interact );
				else
					choice = 0;
				break;
				
			// Search
			case 10:
				cmd( "if { ! [ info exists modElem ] || [ llength $modElem ] == 0 } { \
						ttk::messageBox -parent .deb -type ok -icon warning -title Warning -message \"No configuration loaded\" -detail \"Please load or create one before trying to find elements.\"; \
						set choice 0 \
					}" );
					
				if ( choice == 0 )
					break;
			
				Tcl_LinkVar( inter, "value_search", ( char * ) &value_search, TCL_LINK_DOUBLE );
				Tcl_LinkVar( inter, "condition", ( char * ) &cond, TCL_LINK_INT );
				cond = 0;
				choice = 0;
				value_search = 0;
				i = 1;

				cmd( "set bidi \"\"" );

				cmd( "set s .deb.so" );
				cmd( "newtop $s \"Find Object\" { set choice 2 } .deb" );

				cmd( "ttk::frame $s.l" );
				cmd( "ttk::label $s.l.l -text \"Contained element\"" );
				cmd( "ttk::combobox $s.l.e -width 20 -textvariable bidi -justify center -values $modElem" );
				cmd( "ttk::label $s.l.o -justify center -text \"(type the initial letters of the\nname, LSD will complete it)\"" );
				cmd( "pack $s.l.l $s.l.e $s.l.o" );

				cmd( "ttk::frame $s.c" );
				cmd( "ttk::label $s.c.l -text \"Element value\"" );

				cmd( "ttk::frame $s.c.cond -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
				cmd( "ttk::radiobutton $s.c.cond.any -text \"Any\" -variable condition -value 0 -command { .deb.so.v.e configure -state disabled }" );
				cmd( "ttk::radiobutton $s.c.cond.eq -text \"=\" -variable condition -value 1 -command { .deb.so.v.e configure -state normal }" );
				cmd( "ttk::radiobutton $s.c.cond.geq -text \u2265 -variable condition -value 2 -command { .deb.so.v.e configure -state normal }" );
				cmd( "ttk::radiobutton $s.c.cond.leq -text \u2264 -variable condition -value 3 -command { .deb.so.v.e configure -state normal }" );
				cmd( "ttk::radiobutton $s.c.cond.gr -text \">\" -variable condition -value 4 -command { .deb.so.v.e configure -state normal }" );
				cmd( "ttk::radiobutton $s.c.cond.le -text \"<\" -variable condition -value 5 -command { .deb.so.v.e configure -state normal }" );
				cmd( "ttk::radiobutton $s.c.cond.not -text \u2260 -variable condition -value 6 -command { .deb.so.v.e configure -state normal }" );
				cmd( "pack $s.c.cond.any $s.c.cond.eq $s.c.cond.geq $s.c.cond.leq $s.c.cond.gr $s.c.cond.le $s.c.cond.not -anchor w" );

				cmd( "pack $s.c.l $s.c.cond" );

				cmd( "ttk::frame $s.v" );
				cmd( "ttk::label $s.v.l -text Value" );
				cmd( "ttk::entry $s.v.e -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set value_search %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value_search; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
				cmd( "write_any $s.v.e $value_search" ); 
				cmd( "pack $s.v.l $s.v.e" );

				cmd( "pack $s.l $s.c $s.v -padx 5 -pady 5" );

				cmd( "okhelpcancel $s b { set choice 1 } { LsdHelp debug.html#find } { set choice 2 }" );

				cmd( "bind $s.v.e <KeyPress-Return> { focus .deb.so.b.ok }" );
				cmd( "bind $s.l.e <KeyPress-Return> { focus .deb.so.b.ok; break }" );
				cmd( "bind $s.l.e <KeyRelease> { \
						if { %%N < 256 && [ info exists modElem ] } { \
							set b [ .deb.so.l.e index insert ]; \
							set a [ .deb.so.l.e get ]; \
							set f [ lsearch -glob $modElem $a* ]; \
							if { $f !=-1 } { \
								set d [ lindex $modElem $f ]; \
								.deb.so.l.e delete 0 end; \
								.deb.so.l.e insert 0 $d; \
								.deb.so.l.e index $b; \
								.deb.so.l.e selection range $b end \
							} \
						} \
					}" );

				cmd( "showtop $s" );
				cmd( "focus $s.l.e" );
				cmd( "$s.l.e selection range 0 end" );

				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				if ( choice == 1 )
					cmd( "if { [ lsearch -exact $modElem $bidi ] < 0 } { \
							ttk::messageBox -parent .deb.so -type ok -icon error -title Error -message \"Variable or parameter not found\" -detail \"No element in any object with the name provided was found. Check the spelling of the element name.\"; \
							set choice 2 \
						}" );
					
				if ( choice == 2 )
				{
					cmd( "destroytop .deb.so" );
					Tcl_UnlinkVar( inter, "value_search" );
					Tcl_UnlinkVar( inter, "condition" );
					
					choice = 0;
					redraw = false;
					break;
				}
				 
				pre_running = running;
				running = false;

				cmd( "set value_search [ .deb.so.v.e get ]" ); 
				ch1 = ( char * ) Tcl_GetVar( inter, "bidi", 0 );
				strcpy( ch, ch1);

				cur = NULL;
				switch ( cond )
				{
					case 0:
						cv = r->search_var( r, ch, true );
						if ( cv != NULL )
							cur = cv->up;
						break;
					 
					case 1:
						i = 0;
						cur2 = NULL;
						for ( cur1 = r; cur1 != NULL && i == 0; cur1 = cur1->up )
						{
							cv = cur1->search_var( r, ch, true );
							if ( cv == NULL )
								break;
						 
							cur = cv->up;

							for ( ; cur != NULL && i == 0; cur = cur->hyper_next( cur->label ) )
							{
								app_res = cur->search_var( cur, ch, true )->val[ 0 ];
								if ( app_res == value_search )
								{
									cur2 = cur;
									i = 1;
								} 
							}
						}
						cur = cur2;
						break;

					case 2:
						cv = r->search_var( r, ch, true );
						if ( cv != NULL )
							cur = cv->up;
						while ( cur != NULL && cur->cal( ch, 0 ) < value_search )
							cur = cur->hyper_next( cur->label );
						break;

					case 3:
						cv = r->search_var( r, ch, true );
						if ( cv != NULL )
							cur = cv->up;
						while ( cur != NULL && cur->cal( ch, 0 ) > value_search )
							cur = cur->hyper_next( cur->label );
						break;

					case 4:
						cv = r->search_var( r, ch, true );
						if ( cv != NULL )
							cur = cv->up;
						while ( cur != NULL && cur->cal( ch, 0 ) <= value_search )
							cur = cur->hyper_next( cur->label );
						break;

					case 5:
						cv = r->search_var( r, ch, true );
						if ( cv != NULL )
							cur = cv->up;
						while ( cur != NULL && cur->cal( ch, 0 ) >= value_search )
							cur = cur->hyper_next( cur->label );
						break;

					case 6:
						cv = r->search_var( r, ch, true );
						if ( cv != NULL )
							cur = cv->up;
						while ( cur != NULL && cur->cal( ch, 0 ) == value_search )
							cur = cur->hyper_next( cur->label );
						break;

					default:
						cur = NULL;
				}

				quit = 0;	// if name is mispelled don't stop the simulation!
				cmd( "destroytop .deb.so" );
				Tcl_UnlinkVar( inter, "value_search" );
				Tcl_UnlinkVar( inter, "condition" );

				if ( cur != NULL )
					choice = deb( cur, r, lab, res, interact, ch );
				else
				{
					cmd( "ttk::messageBox -parent .deb -type ok -icon error -title Error -message \"Variable or parameter not found\" -detail \"No object containing an element satisfying the condition provided could be found.\"" );

					choice = 0;
					redraw = false;
					break;
				}

				running = pre_running;
				break;

			// Analysis
			case 11:
				for ( cur = r; cur->up != NULL; cur = cur->up );
				reset_end( cur );
				analysis( &choice );
				cmd( "focustop .deb" );
				
				choice = 0;
				redraw = false;
				break;

			// Previous
			case 12:
				if ( r->up == NULL )
				{
					choice = 0;
					break;
				}
				
				for ( cb1 = NULL, cb = r->up->b; strcmp( r->label, cb->blabel ); cb1 = cb, cb = cb->next );
				
				if ( cb->head != NULL )
				{
					cur = cb->head;
					
					if ( cur == r )
					{
						if ( cb1 != NULL && cb1->head != NULL )
						{
							for ( cur = cb1->head; cur->next != NULL; cur = cur->next );
							choice = deb( cur, c, lab, res, interact );
							break;
						}
						else 
							choice = 0;
						break;
					}
					  
					for ( ; cur->next != r; cur = cur->next );
					choice = deb( cur, c, lab, res, interact );
				}
				
				break;
				
				
			// Print Stack
			case 13:
				print_stack( );
				cmd( "focustop .log" );
				
				choice = 0;
				redraw = false;
				break;

			// Last
			case 14: 
				for ( cur = r, cur1 = NULL; cur != NULL; cur = cur->next )
					cur1 = cur;
				choice = deb( cur1, c, lab, res, interact );
				break;

			// show v[...] variables 
			case 15:
				show_tmp_vars( r, false );
				cmd( "focustop .deb" );

				choice = 0;
				redraw = false;
				break;

			// Until
			case 16:
				cmd( "set tdebug %d", t + 1 );

				cmd( "set t .deb.tdeb" );
				cmd( "newtop $t \"Run Until\" { set choice 2 } .deb" );

				cmd( "ttk::frame $t.t" );
				cmd( "ttk::label $t.t.l -text \"Run until time step\"" );
				cmd( "ttk::entry $t.t.val -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set tdebug %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tdebug; return 0 } } -invalidcommand { bell } -justify center" );
				cmd( "$t.t.val insert 0 $tdebug" ); 
				cmd( "pack $t.t.l $t.t.val" );

				cmd( "pack $t.t -padx 5 -pady 5" );

				cmd( "okhelpcancel $t b { set choice 1 } { LsdHelp debug.html#until } { set choice 2 }" );

				cmd( "bind $t.t.val <Return> { set choice 1 }" );

				cmd( "showtop $t" );
				cmd( "$t.t.val selection range 0 end" );
				cmd( "focus $t.t.val" );
				cmd( "mousewarpto $t.b.ok" );

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "set tdebug [ $t.t.val get ]" ); 
				cmd( "destroytop $t" );

				if ( choice == 1 )
				{
					// restart execution
					choice = 2;
					debug_flag = false;
					cmd( "if { $tdebug > %d } { set when_debug $tdebug } { set when_debug %d }", t, t + 1 );
					cmd( "destroytop .deb" );
					set_buttons_run( true );
				}
				else
					choice = 0;

				break;

			// change the object number of instances (click on level / object instance)
			case 17:
				if ( r->up != NULL )
					entry_new_objnum( r, "", &choice );

				choice = 0;
				break;
				
			// find element
			case 18:
			
				cmd( "if { [ llength $curElem ] == 0 } { \
						ttk::messageBox -parent .deb -type ok -icon warning -title Warning -message \"Empty object\" -detail \"The current object has no element to be found.\"; \
						set choice 0 \
					}" );
					
				if ( choice == 0 )
					break;
			
				cmd( "set bidi \"\"" );

				cmd( "newtop .deb.sv \"Find Element\" { set choice 2 }" );

				cmd( "ttk::frame .deb.sv.i" );
				cmd( "ttk::label .deb.sv.i.l -text \"Element name\"" );
				cmd( "ttk::combobox .deb.sv.i.e -width 20 -textvariable bidi -justify center -values $curElem" );
				cmd( "pack .deb.sv.i.l .deb.sv.i.e" );

				cmd( "ttk::label .deb.sv.o -justify center -text \"(type the initial letters of the\nname, LSD will complete it)\"" );
				cmd( "pack .deb.sv.i .deb.sv.o -padx 5 -pady 5" );
				cmd( "pack .deb.sv.i" );

				cmd( "okcancel .deb.sv b { set choice 1 } { set choice 2 }" );

				cmd( "bind .deb.sv.i.e <KeyPress-Return> { set choice 1; break }" );
				cmd( "bind .deb.sv.i.e <KeyRelease> { \
						if { %%N < 256 && [ info exists curElem ] } { \
							set b [ .deb.sv.i.e index insert ]; \
							set a [ .deb.sv.i.e get ]; \
							set f [ lsearch -glob $curElem $a* ]; \
							if { $f !=-1 } { \
								set d [ lindex $curElem $f ]; \
								.deb.sv.i.e delete 0 end; \
								.deb.sv.i.e insert 0 $d; \
								.deb.sv.i.e index $b; \
								.deb.sv.i.e selection range $b end \
							} \
						} \
					}" );

				cmd( "showtop .deb.sv" );
				cmd( "focus .deb.sv.i.e" );

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				if ( choice == 1 )
					cmd( "if { [ lsearch -exact $curElem $bidi ] < 0 } { \
							ttk::messageBox -parent .deb.sv -type ok -icon error -title Error -message \"Variable or parameter not found\" -detail \"Check the spelling of the element name.\"; \
							set choice 2 \
						}" );
					
				cmd( "destroytop .deb.sv" );

				if ( choice == 2 )
				{
					choice = 0;
					redraw = false;
					break;	
				}	

				pre_running = running;
				running = false;

				ch1 = ( char * ) Tcl_GetVar( inter, "bidi", 0 );
				choice = deb( r, c, lab, res, interact, ch1 );

				running = pre_running;
				break;			

			// clear find selection
			case 19:
				cmd( "if [ winfo exists $lastHl ] { \
						$lastHl.name configure -style TLabel; \
						$lastHl.val configure -style hl.TLabel; \
						$lastHl.last configure -style TLabel; \
						set lastHl \"\" \
					}" );

				choice = 0;
				redraw = false;
				break;			

			// Hooks
			case 21:
				j = r->hooks.size( );						// number of dynamic hooks
				
				if ( j > 0 )
				{
					vector < bool > checked( j + 1, false );
					
					cmd( "set hook 0" );
					
					cmd( "set hk .deb.hk" );
					
					cmd( "newtop $hk \"Choose Hook\" { set choice 2 } .deb" );
					
					cmd( "ttk::frame $hk.l" );
					cmd( "ttk::label $hk.l.l -text \"Object:\"" );
					cmd( "ttk::label $hk.l.n -style hl.TLabel -text %s", r->label );
					cmd( "pack $hk.l.l $hk.l.n -side left -padx 2" );

					cmd( "ttk::frame $hk.t" );
					cmd( "ttk::label $hk.t.l -text \"Available hooks\"" );

					cmd( "ttk::frame $hk.t.t -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
					
					strcpy( ch, "pack" );
					
					for ( i = 0; i < j; ++i )
						if ( r->hooks[ i ] != NULL )
						{
							k = root->search_inst( r->hooks[ i ], false );
							
							if ( k != 0 )
							{
								if ( k > 0 )
								{
									cmd( "ttk::radiobutton $hk.t.t.h%d -text \"Hook %d to %s (%d)\" -variable hook -value %d", i, i, r->hooks[ i ]->label, k, i );
									checked[ i ] = true;
								}
								else
									cmd( "ttk::radiobutton $hk.t.t.h%d -text \"Hook %d to (unchecked)\" -variable hook -value %d", i, i, i );
									
								sprintf( ch2, " $hk.t.t.h%d", i );
								strncat( ch, ch2, 4 * MAX_ELEM_LENGTH - strlen( ch ) - 1 );
							}
						}
					
					if ( r->hook != NULL )
					{
						k = root->search_inst( r->hook, false );
						
						if ( k != 0 )
						{
							if ( k > 0 )
							{
								cmd( "ttk::radiobutton $hk.t.t.h%d -text \"Static Hook to %s (%d)\" -variable hook -value %d", i, r->hook->label, k, i );
									checked[ i ] = true;
							}
							else
								cmd( "ttk::radiobutton $hk.t.t.h%d -text \"Static Hook to (unchecked)\" -variable hook -value %d", i, i );
							
							sprintf( ch2, " $hk.t.t.h%d", i );
							strncat( ch, ch2, 4 * MAX_ELEM_LENGTH - strlen( ch ) - 1 );
						}
					}
					
					strcat( ch, " -anchor w" );
					cmd( ch );

					cmd( "pack $hk.t.l $hk.t.t" );

					cmd( "ttk::frame $hk.m" );
					cmd( "ttk::label $hk.m.l -text \"(invalid and NULL hooks not shown)\"" );
					cmd( "pack $hk.m.l" );
					
					cmd( "pack $hk.l $hk.t $hk.m -padx 5 -pady 5" );

					cmd( "okhelpcancel $hk b { set choice 1 } { LsdHelp debug.html#hooks } { set choice 2 }" );

					cmd( "showtop $hk" );
					
					cmd( "mousewarpto $hk.b.ok" );

					choice = 0;
					while ( choice == 0 )
						Tcl_DoOneEvent( 0 );

					cmd( "destroytop $hk" );

					if ( choice == 1 )
					{
						i = get_int( "hook" );
						
						if ( ! checked[ i ] )
						{
							cmd( "if [ string equal [ ttk::messageBox -parent .deb -type okcancel -icon warning -title Warning -default cancel -message \"Cannot check hook pointer\" -detail \"Cannot check if hook points to a valid object. LSD may crash if jumping to an invalid hook pointer.\" ] ok ] { set choice 1 } { set choice 0 }" );
							
							if ( choice == 0 )
								break;
						}						
						
						if ( i < j )
							choice = deb( r->hooks[ i ], c, lab, res, interact );
						else
							choice = deb( r->hook, c, lab, res, interact );
					}
					else
					{
						choice = 0;
						redraw = false;
					}
				}
				else
					if ( r->hook != NULL )
					{
						k = root->search_inst( r->hook, false );
						
						if ( k == 0 )
						{
							cmd( "ttk::messageBox -parent .deb -type ok -icon error -title Error -message \"Invalid hook pointer\" -detail \"Check if your code is using valid pointers to LSD objects or avoid using this option.\"" );
							
							choice = 0;
							redraw = false;
							break;
						}
						
						if ( k < 0 )
						{
							cmd( "if [ string equal [ ttk::messageBox -parent .deb -type okcancel -icon warning -title Warning -default cancel -message \"Cannot check hook pointer\" -detail \"Cannot check if hook points to a valid object. LSD may crash if jumping to an invalid hook pointer.\" ] ok ] { set choice 1 } { set choice 0 }" );
							
							if ( choice == 0 )
							{
								redraw = false;
								break;
							}
						}
						
						choice = deb( r->hook, c, lab, res, interact );
					}
					else
					{
						choice = 0;
						redraw = false;
					}
					
				break;
						
			// Network
			case 22:
				show_neighbors( r, false );
				cmd( "focustop .deb" );
				
				choice = 0;
				redraw = false;
				break;
				
			// double-click (change to) network node
			case 23:
				ch1 = ( char * ) Tcl_GetVar( inter, "nodeLab", 0 );
				node = get_long( "nodeId" );
				cur = root->search_node_net( ( const char * ) ch1, node );
				if ( cur != NULL )
					choice = deb( cur, c, lab, res, interact );
				else
				{
					choice = 0;
					redraw = false;
				}
				
				break;

			// double-click (change to) object pointer
			case 24:
				ch1 = ( char * ) Tcl_GetVar( inter, "objLab", 0 );
				i = get_int( "objNum" );
				cur = root->search( ( const char * ) ch1 );
				for ( j = 1; j != i && cur != NULL; ++j, cur = cur->hyper_next( ) );
				if ( cur != NULL )
					choice = deb( cur, c, lab, res, interact );
				else
				{
					choice = 0;
					redraw = false;
				}
				
				break;

			// right-click (set all) on multi-instanced parameter or variable
			case 29:
				ch1 = ( char * )Tcl_GetVar( inter, "res", 0 );
				strcpy( ch, ch1 );
				set_all( &choice, r, ch, 0 );
				
				choice = 0;
				break;
					
			// model Report
			case 44:
				show_report( &choice, ".deb" );
				
				choice = 0;
				redraw = false;
				break;

			// Debug variable under computation CTRL+G
			case 77: 
				if ( asl == NULL && stacklog != NULL )
				{
					asl = stacklog;
					plog( "\nVariable: %s", "", asl->label );
					if ( asl->vs != NULL && asl->vs->up != NULL )
						choice = deb( asl->vs->up, c, lab, res, interact );
					else
					{
						choice = 0;
						redraw = false;
					}
				}
				else
				{
					if ( asl->next == NULL )
					{
						while ( asl->prev->prev != NULL )
							asl = asl->prev;
						plog( "\nVariable: %s", "", asl->label );
						if ( asl->vs != NULL && asl->vs->up != NULL )
							choice = deb( asl->vs->up, c, lab, res, interact );
						else
						{
							choice = 0;
							redraw = false;
						}
					}
					else
					{
						asl = asl->next;
						plog( "\nVariable: %s", "", asl->label );
						if ( asl->vs != NULL && asl->vs->up != NULL )
							choice = deb( asl->vs->up, c, lab, res, interact );
						else
						{
							choice = 0;
							redraw = false;
						}
					}
				}  
				break;  
				
			default:
				redraw = false;
				choice = 0;
		}
	}

	*res = app_res;
	non_var = false;

	Tcl_UnlinkVar( inter, "value" );

	return choice;
}


/*******************************************
DEB_SHOW
********************************************/
void deb_show( object *r, const char *hl_var )
{
	char ch[ 2 * MAX_ELEM_LENGTH ];
	variable *ap_v;
	int i;

	// fix the top frame before proceeding
	cmd( "if { ! [ winfo exists .deb.v ] } { \
			ttk::frame .deb.v \
		}" );
	cmd( "if { ! [ winfo exists .deb.v.v2 ] } { \
			ttk::frame .deb.v.v2; \
			ttk::label .deb.v.v2.obj -text \"Level:object(instance): \"; \
			ttk::label .deb.v.v2.instance -style hl.TLabel -text \"\"; \
			pack .deb.v.v2.obj .deb.v.v2.instance -side left; \
			if [ winfo exists .deb.v.v1 ] { \
				pack .deb.v.v1 .deb.v.v2 -padx 5 -pady 5 -anchor w \
			} { \
				pack .deb.v.v2 -padx 5 -pady 5 -anchor w \
			}; \
			pack .deb.v -anchor w \
		}" );

	if ( r->up != NULL )
	{
		cmd( "bind .deb.v.v2.obj <Button-1> { \
				if [ winfo exists .deb.w ] { \
					destroy .deb.w \
				}; \
				set choice 17 \
			}" );
		cmd( "bind .deb.v.v2.instance <Button-1> { \
				if [ winfo exists .deb.w ] { \
					destroy .deb.w \
				}; \
				set choice 17 \
			}" );
	}

	strcpy( ch, "" );
	attach_instance_number( ch, r );
	cmd( ".deb.v.v2.instance config -text \"%s\"", ch  );

	cmd( "if { ! [ winfo exists .deb.tit ] } { \
			set fntSz [ expr [ font metrics [ ttk::style lookup boldSmall.TLabel -font ] -linespace ] + 2 ]; \
			ttk::frame .deb.tit -height [ expr $fntSz + $vspcszD ]; \
			ttk::label .deb.tit.name1 -style boldSmall.TLabel -text Variable -anchor w; \
			ttk::label .deb.tit.val1 -style hlBoldSmall.TLabel -text Value; \
			ttk::label .deb.tit.last1 -style boldSmall.TLabel -text \"Updated\"; \
			ttk::label .deb.tit.pad -style boldSmall.TLabel; \
			ttk::label .deb.tit.name2 -style boldSmall.TLabel -text Variable -anchor w; \
			ttk::label .deb.tit.val2 -style hlBoldSmall.TLabel -text Value; \
			ttk::label .deb.tit.last2 -style boldSmall.TLabel -text \"Updated\"; \
			placeline { .deb.tit.name1 .deb.tit.val1 .deb.tit.last1 .deb.tit.pad .deb.tit.name2 .deb.tit.val2 .deb.tit.last2 } [ list $hnamshD $hvalshD $hupdshD $hpadshD $hnamshD $hvalshD $hupdshD ] 0 $fntSz; \
			pack .deb.tit -anchor w -fill x -after .deb.v \
		}" );

	// create single top frame to grid, where the values table can be built
	// and a canvas to hold the table so it can be scrollable
	cmd( "set g .deb.cc.grid" );
	cmd( "if [ winfo exists .deb.cc ] { \
			$g.can delete all; \
			if { ! %d } { \
				$g.can yview moveto 0; \
				unset -nocomplain lstDebPos \
			} \
		} { \
			ttk::frame .deb.cc; \
			ttk::frame $g; \
			grid $g; \
			grid rowconfigure $g 0 -weight 1; \
			grid columnconfigure $g 0 -weight 1; \
			grid propagate $g 0; \
			set lastDebSz { 0 0 }; \
			set debDone 0; \
			set fntWid [ font measure [ ttk::style lookup TLabel -font active TkDefaultFont ] 0 ]; \
			set hcharszD [ expr int( ( $hsizeDmin - 15 ) / $fntWid ) ]; \
			set hnamszD [ expr round( $hnamshD * $hcharszD ) ]; \
			set hvalszD [ expr round( $hvalshD * $hcharszD ) ]; \
			set hupdszD [ expr round( $hupdshD * $hcharszD ) ]; \
			set hpadszD [ expr round( $hpadshD * $hcharszD ) - 1 ]; \
			ttk::canvas $g.can -yscrollcommand { .deb.cc.grid.scroll set } -entry 0 -dark $darkTheme; \
			ttk::scrollbar $g.scroll -command { .deb.cc.grid.can yview }; \
			grid $g.can $g.scroll -sticky nsew; \
			mouse_wheel $g.can; \
			pack .deb.cc -anchor w -expand 1 -fill both -after .deb.tit; \
			bind .deb <Configure> { \
				if { ! [ info exists debConfRun ] } { \
					set debConfRun 1; \
					set debSz [ list [ winfo width .deb ] [ winfo height .deb ] ]; \
					if { $debSz != $lastDebSz || ( $debDone == 1 && ! [ info exists debButHgt ] ) } { \
						update; \
						set lastDebSz $debSz; \
						set canBbox [ $g.can bbox all ]; \
						if { $debDone == 1 } { \
							set desWid [ winfo width .deb.cc ]; \
							set desHgt [ winfo height .deb.cc ]; \
							if { ! [ info exists debButHgt ] } { \
								set debButHgt [ expr [ lindex $debSz 1 ] - $desHgt ]; \
							} elseif { $debButHgt > [ expr [ lindex $debSz 1 ] - $desHgt ] } { \
								set desHgt [ expr [ lindex $debSz 1 ] - $debButHgt ] \
							}; \
							$g configure -width $desWid -height $desHgt; \
							$g.can configure -scrollregion $canBbox \
						} { \
							set desWid [ expr max( [ lindex $canBbox 2 ] - [ lindex $canBbox 0 ], 400 ) ]; \
							set desHgt [ expr max( [ lindex $canBbox 3 ] - [ lindex $canBbox 1 ], 250 ) ]; \
							unset -nocomplain debButHgt \
						}; \
					} elseif { $debDone == 2 } { \
						set debDone 1; \
						$g.can configure -scrollregion [ $g.can bbox all ] \
					}; \
					unset debConfRun \
				} \
			} \
		}", r == debLstObj ? 1 : 0 );

	cmd( "set lastHl \"\"" );
	cmd( "set curElem [ list ]" );
	cmd( "array unset debElem" );
	
	if ( r->v == NULL )
	{
		cmd( "$g.can create text 0 0" );	// reference to position message
		cmd( "$g.can create text [ expr ( $hsizeDmin - 10 ) / 2 ] [ expr $vsizeDmin / 3 ] -text \"(no elements in object)\" -font [ ttk::style lookup boldSmall.TLabel -font ] -fill $colorsTheme(fg)" );
	}
	else
	{
		Tcl_LinkVar( inter, "i", ( char * ) &i, TCL_LINK_INT );

		// single frame ($w=.deb.cc.grid.can.f) in canvas to hold all cells
		cmd( "set w $g.can.f" );
		cmd( "destroy $w" );
		cmd( "ttk::frame $w" );
		cmd( "$g.can create window 0 0 -window $w -anchor nw" );
		
		for ( i = 1, ap_v = r->v; ap_v != NULL; ap_v = ap_v->next, ++i )
		{
			cmd( "set debElem(%s) [ list $i $w.e$i ]", ap_v->label );

			cmd( "set last %d", ap_v->last_update );
			cmd( "set val %g", ap_v->val[ 0 ] );
			cmd( "ttk::frame $w.e$i" );
			cmd( "ttk::label $w.e$i.name -width $hnamszD -anchor w -text %s", ap_v->label );
			
			if ( is_nan( ap_v->val[ 0 ] ) )
				cmd( "ttk::label $w.e$i.val -width $hvalszD -style hl.TLabel -text NAN" );
			else
				if ( is_inf( ap_v->val[ 0 ] ) )
					cmd( "ttk::label $w.e$i.val -width $hvalszD -style hl.TLabel -text %sINFINITY", ap_v->val[ 0 ] < 0 ? "-" : "" );
				else
					if ( ap_v->val[ 0 ] != 0 && fabs( ap_v->val[ 0 ] ) < SIG_MIN )	// insignificant value?			
						cmd( "ttk::label $w.e$i.val -width $hvalszD -style hl.TLabel -text ~0" );
					else
						cmd( "ttk::label $w.e$i.val -width $hvalszD -style hl.TLabel -text $val" );
			
			if ( ap_v->param == 0 )
				cmd( "ttk::label $w.e$i.last -width $hupdszD -text $last" );
			if ( ap_v->param == 1 )
				cmd( "ttk::label $w.e$i.last -width $hupdszD -text (P)" );
			if ( ap_v->param == 2 )
				cmd( "ttk::label $w.e$i.last -width $hupdszD -text (F)" );
			
			if ( i % 2 == 0 )
			{
				cmd( "ttk::label $w.e$i.pad -width $hpadszD" );
			
				cmd( "grid $w.e$i.pad $w.e$i.name $w.e$i.val $w.e$i.last" );
				cmd( "grid $w.e$i -column 1 -row [ expr int( ( $i - 1 ) / 2 ) ]" );
				cmd( "mouse_wheel $w.e$i.pad" );
			}
			else
			{
				cmd( "grid $w.e$i.name $w.e$i.val $w.e$i.last" );
				cmd( "grid $w.e$i" );
			}

			cmd( "mouse_wheel $w.e$i.name" );
			cmd( "mouse_wheel $w.e$i.val" );
			cmd( "mouse_wheel $w.e$i.last" );

			cmd( "bind $w.e$i.name <Button-1> { set res %s; set lstDebPos [ .deb.cc.grid.can yview ]; set choice 8 }", ap_v->label );
			cmd( "bind $w.e$i.name <Button-2> { set res %s; set lstDebPos [ .deb.cc.grid.can yview ]; set choice 29 }", ap_v->label );
			cmd( "bind $w.e$i.name <Button-3> { event generate .deb.cc.grid.can.f.e$i.name <Button-2> -x %%x -y %%y }" );
		}

		cmd( "set debConfChg 1" );
		cmd( "event generate .deb <Configure>" );
		
		cmd( "set curElem [ lsort [ array names debElem ] ]" );
		cmd( "if { [ string length \"%s\" ] > 0 && [ lsearch -exact $curElem %s ] >= 0 } { \
				set lastHl [ lindex $debElem(%s) 1 ]; \
				set hlPos [ expr ceil( [ lindex $debElem(%s) 0 ] / 2 ) / ceil( [ array size debElem ] / 2 ) ]; \
				$lastHl.name configure -style sel.TLabel; \
				$lastHl.val configure -style selHl.TLabel; \
				$lastHl.last configure -style sel.TLabel \
			} { \
				set lastHl \"\"; \
			}", hl_var, hl_var, hl_var, hl_var );
		
		Tcl_UnlinkVar( inter, "i" );
	}
}


/*******************************************
SHOW_TMP_VARS
********************************************/
void show_tmp_vars( object *r, bool update )
{
	char i_names[ ] = { 'i', 'j', 'h', 'k' };
	int i, j, m, n;
	netLink *curLnk = NULL;
	object *cur;
	
	cmd( "set in .deb.val" );
	cmd( "set existVal [ winfo exists $in ]" );
	if ( ! strcmp( Tcl_GetVar( inter, "existVal", 0 ), "0" ) )
	{
		cmd( "newtop $in \"v\\[...\\]\" { destroytop .deb.val } .deb" ); 

		cmd( "ttk::frame $in.l1" );
		cmd( "ttk::label $in.l1.l -text \"Name and instance\"" );
		cmd( "ttk::frame $in.l1.n" );
		cmd( "ttk::label $in.l1.n.name -style hl.TLabel" );
		cmd( "ttk::label $in.l1.n.sep -text |" );
		cmd( "ttk::label $in.l1.n.id -style hl.TLabel" );
		cmd( "pack $in.l1.n.name $in.l1.n.sep $in.l1.n.id -side left" );
		cmd( "pack $in.l1.l $in.l1.n" );
		
		cmd( "ttk::frame $in.l2" );
		cmd( "ttk::label $in.l2.id -width 6 -text Variable" );
		cmd( "ttk::label $in.l2.pad" );
		cmd( "ttk::label $in.l2.val -width 14 -style hl.TLabel -text Value" );
		cmd( "pack $in.l2.id $in.l2.pad $in.l2.val -side left" );

		cmd( "pack $in.l1 $in.l2 -pady 2" );

		cmd( "ttk::frame $in.n" );
		cmd( "ttk::scrollbar $in.n.yscroll -command \"$in.n.t yview\"" );
		cmd( "pack $in.n.yscroll -side right -fill y" );
		cmd( "ttk::text $in.n.t -width 18 -height 15 -yscrollcommand \"$in.n.yscroll set\" -wrap none -entry 0 -dark $darkTheme" );
		cmd( "mouse_wheel $in.n.t" );
		cmd( "pack $in.n.t -expand yes -fill both" );
		cmd( "pack $in.n -expand yes -fill both" );
		
		cmd( "ttk::label $in.l3 -justify center -text \"(double-click name to\nchange to object)\"" );
		cmd( "pack $in.l3 -pady 5" );

		cmd( "showtop $in topleftW 0 1 0" );
		cmd( "wm minsize $in [ winfo reqwidth $in ] [ expr $vsizeDmin + $vmenusize ]" );
		cmd( "wm geometry $in [ winfo reqwidth $in ]x[ expr [ winfo height .deb ] + $vmenusize ]" );

		cmd( "$in.n.t tag configure bold -font [ ttk::style lookup boldSmallProp.TText -font ]" );

		cmd( "if { ! [ winfo exists .deb.net ] } { align $in .deb } { align $in .deb.net }" );
	}
	else
		if ( update )
		{
			
			cmd( "$in.n.t configure -state normal" );
			cmd( "$in.n.t delete 1.0 end" );
		}
		else
		{
			cmd( "destroytop .deb.val" );
			return;
		}
	
	m = root->search_inst( r, true );
	cmd( "$in.l1.n.name configure -text \"%s\"", r->label == NULL ? "" : r->label );
	cmd( "$in.l1.n.id configure -text \"%d\"", m );
	
	Tcl_LinkVar( inter, "i", ( char * ) &i, TCL_LINK_INT );
	
	cmd( "$in.n.t insert end \"Temporary storage\n\" bold" );
	
	for ( i = 1, j = 0; j < 10; ++i, ++j )
	{
		cmd( "ttk::frame $in.n.t.n$i" );
		cmd( "ttk::label $in.n.t.n$i.var -width 6 -text \"v\\\[%d\\]\"", j );
		cmd( "ttk::label $in.n.t.n$i.pad -width 1" );
		
		if ( is_nan( d_values[ j ] ) )
			cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text NAN" );
		else
			if ( is_inf( d_values[ j ] ) )
				cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text %sINFINITY", d_values[ j ] < 0 ? "-" : "" );
			else
				if ( d_values[ j ] != 0 && fabs( d_values[ j ] ) < SIG_MIN )	// insignificant value?
					cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text ~0" );
				else
					cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text %g", d_values[ j ] );
				
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		cmd( "mouse_wheel $in.n.t.n$i.var" );
		cmd( "mouse_wheel $in.n.t.n$i.pad" );
		cmd( "mouse_wheel $in.n.t.n$i.val" );
		
		cmd( "$in.n.t window create end -window $in.n.t.n$i" );
		cmd( "$in.n.t insert end \\n" );
	}
	
	cmd( "$in.n.t insert end \"Integer indexes\n\" bold" );
	
	for ( j = 0; j < ( int ) ( sizeof i_names / sizeof i_names[ 0 ] ); ++i, ++j )
	{
		cmd( "ttk::frame $in.n.t.n$i" );
		cmd( "ttk::label $in.n.t.n$i.var -width 6 -text \"%c\"", i_names[ j ] );
		cmd( "ttk::label $in.n.t.n$i.pad -width 1" );
		
		cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text %d", i_values[ j ] );

		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		cmd( "mouse_wheel $in.n.t.n$i.var" );
		cmd( "mouse_wheel $in.n.t.n$i.pad" );
		cmd( "mouse_wheel $in.n.t.n$i.val" );
		
		cmd( "$in.n.t window create end -window $in.n.t.n$i" );
		cmd( "$in.n.t insert end \\n" );
	}
	
	cmd( "$in.n.t insert end \"Object pointers\n\" bold" );
	
	for ( j = 0; j < 10; ++i, ++j )
	{
		cmd( "ttk::frame $in.n.t.n$i" );
		cmd( "ttk::label $in.n.t.n$i.pad -width 1" );
		
		if ( j == 0 )
			cmd( "ttk::label $in.n.t.n$i.var -width 6 -text cur" );
		else
			cmd( "ttk::label $in.n.t.n$i.var -width 6 -text \"cur%d\"", j );
		
		n = 0;
		if ( o_values[ j ] == NULL )
			cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text NULL" );
		else
		{
			// search an object pointed by the pointer
			n = ( int ) root->search_inst( o_values[ j ], false );
			
			if ( n > 0 && o_values[ j ]->label != NULL )
				cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text \"%s(%d)\"", o_values[ j ]->label, n );
			else
				if ( n < 0 )
					cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text \"(unchecked)\"" );
				else
					cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text \"(invalid)\"" );
		}
		
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		cmd( "mouse_wheel $in.n.t.n$i.var" );
		cmd( "mouse_wheel $in.n.t.n$i.pad" );
		cmd( "mouse_wheel $in.n.t.n$i.val" );
		
		if ( n > 0 )
		{
			cmd( "bind $in.n.t.n$i.var <Double-Button-1> { set objLab %s; set objNum %d; set choice 24 }", o_values[ j ]->label, n );
			cmd( "bind $in.n.t.n$i.val <Double-Button-1> { set objLab %s; set objNum %d; set choice 24 }", o_values[ j ]->label, n );
		}
		
		cmd( "$in.n.t window create end -window $in.n.t.n$i" );
		cmd( "$in.n.t insert end \\n" );
	}
	
	cmd( "$in.n.t insert end \"Network link pointers\n\" bold" );
	
	for ( j = 0; j < 10; ++i, ++j )
	{
		cmd( "ttk::frame $in.n.t.n$i" );
		cmd( "ttk::label $in.n.t.n$i.pad -width 1" );
		
		if ( j == 0 )
			cmd( "ttk::label $in.n.t.n$i.var -width 6 -text curl" );
		else
			cmd( "ttk::label $in.n.t.n$i.var -width 6 -text \"curl%d\"", j );
		
		if ( n_values[ j ] == NULL )
			cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text NULL" );
		else
		{
			// try search a link pointed by the pointer in current object only
			n = 0;
			if ( r->node != NULL )
			{
				for ( curLnk = r->node->first; curLnk != NULL; curLnk = curLnk->next )
					if ( curLnk == n_values[ j ] && curLnk->ptrTo != NULL && curLnk->ptrTo->label != NULL )
					{
						cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text \"%s(%ld)\"", curLnk->ptrTo->label, curLnk->serTo );
						n = 1;
						break;
					}
			}
			
			if ( n == 0 )
				cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text \"(unknown)\"" );
		}
		
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		cmd( "mouse_wheel $in.n.t.n$i.var" );
		cmd( "mouse_wheel $in.n.t.n$i.pad" );
		cmd( "mouse_wheel $in.n.t.n$i.val" );
		
		if ( n > 0 && curLnk != NULL )
		{
			cmd( "bind $in.n.t.n$i.var <Double-Button-1> { set nodeId %ld; set nodeLab %s; set choice 23 }", curLnk->ptrTo->node->id, curLnk->ptrTo->label );
			cmd( "bind $in.n.t.n$i.val <Double-Button-1> { set nodeId %ld; set nodeLab %s; set choice 23 }", curLnk->ptrTo->node->id, curLnk->ptrTo->label );
		}
		
		cmd( "$in.n.t window create end -window $in.n.t.n$i" );
		cmd( "$in.n.t insert end \\n" );
	}
	
	cmd( "$in.n.t insert end \"Object hook pointers\n\" bold" );
	
	for ( j = -1; j < ( int ) r->hooks.size( ); ++i, ++j )
	{
		cmd( "ttk::frame $in.n.t.n$i" );
		cmd( "ttk::label $in.n.t.n$i.pad -width 1" );

		if ( j < 0 )
		{
			cmd( "ttk::label $in.n.t.n$i.var -width 7 -text SHOOK" );
			cur = r->hook;	
		}
		else
		{
			cmd( "ttk::label $in.n.t.n$i.var -width 7 -text \"HOOK(%d)\"", j );
			cur = r->hooks[ j ];	
		}
		
		n = 0;
		if ( cur == NULL )
			cmd( "ttk::label $in.n.t.n$i.val -width 12 -style hl.TLabel -text NULL" );
		else
		{
			// search an object pointed by the hook
			n = ( int ) root->search_inst( cur, false );
			
			if ( n > 0 && cur->label != NULL )
				cmd( "ttk::label $in.n.t.n$i.val -width 12 -style hl.TLabel -text \"%s(%d)\"", cur->label, n );
			else
				if ( n < 0 )
					cmd( "ttk::label $in.n.t.n$i.val -width 12 -style hl.TLabel -text \"(unchecked)\"" );
				else
					cmd( "ttk::label $in.n.t.n$i.val -width 12 -style hl.TLabel -text \"(invalid)\"" );
		}
		
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		cmd( "mouse_wheel $in.n.t.n$i.var" );
		cmd( "mouse_wheel $in.n.t.n$i.pad" );
		cmd( "mouse_wheel $in.n.t.n$i.val" );
		
		if ( n > 0 )
		{
			cmd( "bind $in.n.t.n$i.var <Double-Button-1> { set objLab %s; set objNum %d; set choice 24 }", cur->label, n );
			cmd( "bind $in.n.t.n$i.val <Double-Button-1> { set objLab %s; set objNum %d; set choice 24 }", cur->label, n );
		}
		
		cmd( "$in.n.t window create end -window $in.n.t.n$i" );
		cmd( "$in.n.t insert end \\n" );
	}

	cmd( "$in.n.t insert end \"More temporary storage\n\" bold" );
	
	for ( j = 10; j < min( 100, USER_D_VARS ); ++i, ++j )
	{
		cmd( "ttk::frame $in.n.t.n$i" );
		cmd( "ttk::label $in.n.t.n$i.var -width 6 -text \"v\\\[%d\\]\"", j );
		cmd( "ttk::label $in.n.t.n$i.pad -width 1" );
		
		if ( is_nan( d_values[ j ] ) )
			cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text NAN" );
		else
			if ( is_inf( d_values[ j ] ) )
				cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text %sINFINITY", d_values[ j ] < 0 ? "-" : "" );
			else
				if ( d_values[ j ] != 0 && fabs( d_values[ j ] ) < SIG_MIN )	// insignificant value?
					cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text ~0" );
				else
					cmd( "ttk::label $in.n.t.n$i.val -width 13 -style hl.TLabel -text %g", d_values[ j ] );
				
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		cmd( "mouse_wheel $in.n.t.n$i.var" );
		cmd( "mouse_wheel $in.n.t.n$i.pad" );
		cmd( "mouse_wheel $in.n.t.n$i.val" );
		
		cmd( "$in.n.t window create end -window $in.n.t.n$i" );
		cmd( "$in.n.t insert end \\n" );
	}
	
	Tcl_UnlinkVar( inter, "i" );
	cmd( "$in.n.t configure -state disabled" );
} 


/*******************************************
SHOW_NEIGHBORS
********************************************/
void show_neighbors( object *r, bool update )
{
	int i;
	netLink *curLnk;
	
	if ( r->node == NULL )
		return;

	cmd( "set N .deb.net" );
	cmd( "set existNet [ winfo exists $N ]" );
	if ( ! strcmp( Tcl_GetVar( inter, "existNet", 0 ), "0" ) )
	{
		cmd( "newtop $N \"Network\" { destroytop .deb.net } .deb" );
		
		cmd( "ttk::frame $N.l1" );
		cmd( "ttk::label $N.l1.l -text \"Node ID and name\"" );
		cmd( "ttk::frame $N.l1.n" );
		cmd( "ttk::label $N.l1.n.id -style hl.TLabel" );
		cmd( "ttk::label $N.l1.n.sep -text |" );
		cmd( "ttk::label $N.l1.n.name -style hl.TLabel" );
		cmd( "pack $N.l1.n.id $N.l1.n.sep $N.l1.n.name -side left" );
		cmd( "pack $N.l1.l $N.l1.n" );
		
		cmd( "ttk::frame $N.l2" );
		cmd( "ttk::label $N.l2.l -text \"Num. links out:\"" );
		cmd( "ttk::label $N.l2.n -style hl.TLabel" );
		cmd( "pack $N.l2.l $N.l2.n -side left" );
		
		cmd( "ttk::frame $N.l3" );
		cmd( "ttk::label $N.l3.l -text \"Outgoing links\"" );
		
		cmd( "ttk::frame $N.l3.h" );
		cmd( "ttk::label $N.l3.h.id -width 6 -text \"Dest. ID\"" );
		cmd( "ttk::label $N.l3.h.pad -width 2" );
		cmd( "ttk::label $N.l3.h.wght -width 12 -style hl.TLabel -text \"(Weight) \"" );
		cmd( "pack $N.l3.h.id $N.l3.h.pad $N.l3.h.wght -side left" );

		cmd( "pack $N.l3.l $N.l3.h" );

		cmd( "pack $N.l1 $N.l2 $N.l3 -pady 2" );

		cmd( "ttk::frame $N.n" );
		cmd( "ttk::scrollbar $N.n.yscroll -command \".deb.net.n.t yview\"" );
		cmd( "pack $N.n.yscroll -side right -fill y" );
		cmd( "ttk::text $N.n.t -width 18 -height 15 -yscrollcommand \"$N.n.yscroll set\" -wrap none -entry 0 -dark $darkTheme" );
		cmd( "mouse_wheel $N.n.t" );
		cmd( "pack $N.n.t -expand yes -fill both" );
		cmd( "pack $N.n -expand yes -fill both" );
		
		cmd( "ttk::label $N.l4 -justify center -text \"(double-click ID to\nchange to node)\"" );
		cmd( "pack $N.l4 -pady 5" );
		
		cmd( "showtop $N topleftW 0 1 0" );
		cmd( "wm minsize $N [ winfo reqwidth $N ] [ expr $vsizeDmin + $vmenusize ]" );
		cmd( "wm geometry $N [ winfo reqwidth $N ]x[ expr [ winfo height .deb ] + $vmenusize ]" );
		
		cmd( "if { ! [ winfo exists .deb.val ] } { align $N .deb } { align $N .deb.val }" );
	}
	else
		if ( update )
		{
			cmd( "$N.n.t configure -state normal" );
			cmd( "$N.n.t delete 1.0 end" );
		}
		else
		{
			cmd( "destroytop .deb.net" );
			return;
		}
	
	cmd( "$N.l1.n.id configure -text \"%ld\"", r->node->id );
	cmd( "$N.l1.n.name configure -text \"%s\"", r->node->name == NULL ? "" : r->node->name );
	cmd( "$N.l2.n configure -text %ld", r->node->nLinks );
	
	Tcl_LinkVar( inter, "i", ( char * ) &i, TCL_LINK_INT );
	
	for ( i = 1, curLnk = r->node->first; curLnk != NULL; curLnk = curLnk->next, ++i )
	{
		cmd( "ttk::frame $N.n.t.n$i" );
		cmd( "ttk::label $N.n.t.n$i.nodeto -width 6 -text %ld", curLnk->ptrTo->node->id );
		cmd( "ttk::label $N.n.t.n$i.pad -width 2" );
		if ( curLnk->weight != 0 )
			cmd( "ttk::label $N.n.t.n$i.weight -width 12 -style hl.TLabel -text %g", curLnk->weight );
		else
			cmd( "ttk::label $N.n.t.n$i.weight -width 12" );
		
		cmd( "pack $N.n.t.n$i.nodeto $N.n.t.n$i.pad $N.n.t.n$i.weight -side left" );
		
		cmd( "mouse_wheel $N.n.t.n$i.nodeto" );
		cmd( "mouse_wheel $N.n.t.n$i.pad" );
		cmd( "mouse_wheel $N.n.t.n$i.weight" );
		
		cmd( "bind $N.n.t.n$i.nodeto <Double-Button-1> { set nodeId %ld; set nodeLab %s; set choice 23 }", curLnk->ptrTo->node->id, r->label );
		
		if ( curLnk->weight != 0 )
			cmd( "bind $N.n.t.n$i.weight <Double-Button-1> { set nodeId %ld; set nodeLab %s; set choice 23 }", curLnk->ptrTo->node->id, r->label );
		
		cmd( "$N.n.t window create end -window $N.n.t.n$i" );
		cmd( "$N.n.t insert end \\n" );
	}
	
	Tcl_UnlinkVar( inter, "i" );
	cmd( "$N.n.t configure -state disabled" );
}	


/*******************************************
ATTACH_INSTANCE_NUMBER
********************************************/
int depth;

void attach_instance_number( char *ch, object *r )
{
	object *cur;
	int i = 1, j = 1;

	if ( r == NULL )
		return;
	
	attach_instance_number( ch, r->up );

	if ( r->up != NULL )
		for ( cur = r->up->search( r->label ); cur != NULL; cur = go_brother( cur ) )
		{
			if ( cur == r )
			j = i;
			i++;
		}

	if ( r->up == NULL )
		sprintf( msg, "%d:%s (1/1) ", depth = 1, r->label );
	else
		sprintf( msg, " |  %d:%s (%d/%d) ", ++depth, r->label, j, i - 1 );
	strncat( ch, msg, 2 * MAX_ELEM_LENGTH - 1 - strlen( ch ) );
}
