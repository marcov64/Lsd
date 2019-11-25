/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
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

- void deb_show( object *r )
fill in all the content of the object.
*************************************************************/

#include "decl.h"

lsdstack *asl = NULL;			// debug stack


/*******************************************
DEB
********************************************/
int deb( object *r, object *c, char const *lab, double *res, bool interact )
{
bool pre_running;
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

cmd( "set deb .deb" );
cmd( "if { ! [ winfo exists .deb ] } { if [ string equal $lab \"\" ] { set debTitle \"LSD Data Browser\" } { set debTitle \"LSD Debugger\" }; newtop .deb \"%s%s - $debTitle\" { set choice 7 } \"\"; set justCreated true }", unsaved_change() ? "*" : " ", simul_name  );

// avoid redrawing the menu if it already exists and is configured
cmd( "set existMenu [ winfo exists .deb.m ]" );
cmd( "set confMenu [ .deb cget -menu ]" );
if ( ! strcmp( Tcl_GetVar( inter, "existMenu", 0 ), "0" ) ||
	 strcmp( Tcl_GetVar( inter, "confMenu", 0 ), ".deb.m" ) )
{
	cmd( "destroy .deb.m" );
	cmd( "menu .deb.m -tearoff 0" );
	cmd( "set w .deb.m.exit" );
	cmd( ".deb.m add cascade -label Exit -menu $w -underline 0" );
	cmd( "menu $w -tearoff 0" );
	if ( mode == 3 )
		cmd( "$w add command -label \"Quit and Resume Simulation\" -command { set choice 7 } -underline 0 -accelerator Esc" );
	else
		cmd( "$w add command -label \"Quit and Return to Browser\" -command { set choice 7 } -underline 0 -accelerator Esc" );
	cmd( "set w .deb.m.help" );
	cmd( "menu $w -tearoff 0" );
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
	cmd( "bind .deb <F1> { .deb.m.help invoke 0; break }" );
}

// avoid redrawing the buttons if they already exist
cmd( "set existButtons [ expr [ winfo exists .deb.b ] ]" );
if ( ! strcmp( Tcl_GetVar( inter, "existButtons", 0 ), "0" ) )
{ 
	cmd( "destroy .deb.b" );

	cmd( "frame .deb.b -border 6" );
	
	// first row of buttons (always shown)
	cmd( "frame .deb.b.move" );

	cmd( "button .deb.b.move.up -width $butWid -text Up -command {set choice 3} -underline 0" );
	cmd( "button .deb.b.move.down -width $butWid -text Down -command {set choice 6} -underline 0" );
	cmd( "button .deb.b.move.prev -width $butWid -text Previous -command {set choice 12} -underline 0" );
	cmd( "button .deb.b.move.broth -width $butWid -text Next -command {set choice 4} -underline 0" );
	cmd( "button .deb.b.move.hypern -width $butWid -text \"Next Type\" -command {set choice 5} -underline 5" );
	cmd( "button .deb.b.move.last -width $butWid -text Last -command {set choice 14} -underline 0" );
	cmd( "button .deb.b.move.search -width $butWid -text Find -command {set choice 10} -underline 0" );
	cmd( "button .deb.b.move.hook -width $butWid -text Hooks -command {set choice 21} -underline 0" );
	cmd( "button .deb.b.move.net -width $butWid -text Network -command {set choice 22} -underline 3" );
	
	cmd( "pack .deb.b.move.up .deb.b.move.down .deb.b.move.prev .deb.b.move.broth .deb.b.move.hypern .deb.b.move.last .deb.b.move.search .deb.b.move.hook .deb.b.move.net -padx 8 -pady 5 -side left -expand no -fill none" );
	
	cmd( "pack .deb.b.move -expand no -fill none -anchor e" );
	
	cmd( "bind .deb <KeyPress-u> {.deb.b.move.up invoke}; bind .deb <KeyPress-U> {.deb.b.move.up invoke}" );
	cmd( "bind .deb <Up> {.deb.b.move.up invoke}" );
	cmd( "bind .deb <KeyPress-n> {.deb.b.move.broth invoke}; bind .deb <KeyPress-N> {.deb.b.move.broth invoke}" );
	cmd( "bind .deb <Right> {.deb.b.move.broth invoke}" );
	cmd( "bind .deb <KeyPress-t> {.deb.b.move.hypern invoke}; bind .deb <KeyPress-T> {.deb.b.move.hypern invoke}" );
	cmd( "bind .deb <KeyPress-l> {.deb.b.move.last invoke}; bind .deb <KeyPress-L> {.deb.b.move.last invoke}" );
	cmd( "bind .deb <KeyPress-d> {.deb.b.move.down invoke}; bind .deb <KeyPress-D> {.deb.b.move.down invoke}" );
	cmd( "bind .deb <Down> {.deb.b.move.down invoke}" );
	cmd( "bind .deb <KeyPress-h> {set choice 21}; bind .deb <KeyPress-H> {set choice 21}" );
	cmd( "bind .deb <KeyPress-w> {set choice 22}; bind .deb <KeyPress-W> {set choice 22}" );
	cmd( "bind .deb <KeyPress-f> {.deb.b.move.search invoke}; bind .deb <KeyPress-F> {.deb.b.move.search invoke}" );
	cmd( "bind .deb <KeyPress-p> {.deb.b.move.prev invoke}; bind .deb <KeyPress-P> {.deb.b.move.prev invoke}" );
	cmd( "bind .deb <Left> {.deb.b.move.prev invoke}" );
	cmd( "bind .deb <KeyPress-Escape> {set choice 7}" );

	// second row of buttons (if applicable)
	if ( mode == 1 || mode == 3 )
	{
		cmd( "set stack_flag %d", stack_info );
		
		cmd( "frame .deb.b.act" );
		
		if ( mode == 1 )
		{
			cmd( "button .deb.b.act.run -width $butWid -text Run -command {set choice 2; set_c_var done_in 0} -underline 0" );
			cmd( "button .deb.b.act.until -width $butWid -text Until -command {set choice 16; set_c_var done_in 0} -underline 3" );
			cmd( "button .deb.b.act.ok -width $butWid -text Step -command {set choice 1; set_c_var done_in 3} -underline 0" );
			cmd( "button .deb.b.act.call -width $butWid -text Caller -command {set choice 9} -underline 0" );
			cmd( "button .deb.b.act.prn_v -width $butWid -text \"v\\\[...\\]\" -command {set choice 15} -underline 0" );
			
			cmd( "bind .deb <KeyPress-r> {.deb.b.act.run invoke}; bind .deb <KeyPress-R> {.deb.b.act.run invoke}" );
			cmd( "bind .deb <KeyPress-i> {.deb.b.act.until invoke}; bind .deb <KeyPress-I> {.deb.b.act.until invoke}" );
			cmd( "bind .deb <KeyPress-s> {.deb.b.act.ok invoke}; bind .deb <KeyPress-S> {.deb.b.act.ok invoke}" );
			cmd( "bind .deb <KeyPress-c> {.deb.b.act.call invoke}; bind .deb <KeyPress-C> {.deb.b.act.call invoke}" );
			cmd( "bind .deb <KeyPress-v> {.deb.b.act.prn_v invoke}; bind .deb <KeyPress-V> {.deb.b.act.prn_v invoke}" );
		}
		
		cmd( "button .deb.b.act.an -width $butWid -text Analysis -command {set choice 11} -underline 0" );
		cmd( "button .deb.b.act.prn_stck -width $butWid -text Stack -command {set choice 13} -underline 4" );
		
		cmd( "bind .deb <KeyPress-a> {.deb.b.act.an invoke}; bind .deb <KeyPress-A> {.deb.b.act.an invoke}" );
		cmd( "bind .deb <KeyPress-k> {.deb.b.act.prn_stck invoke}; bind .deb <KeyPress-K> {.deb.b.act.prn_stck invoke}" );
		
		cmd( "frame .deb.b.act.stack" );
		cmd( "label .deb.b.act.stack.l -text \"Stack level\"" );
		cmd( "if [ string equal [ info tclversion ] 8.6 ] { ttk::spinbox .deb.b.act.stack.e -width 3 -from 0 -to 99 -validate focusout -validatecommand { if [ string is integer -strict %%P ] { set stack_flag %%P; return 1 } { %%W delete 0 end; %%W insert 0 $stack_flag; return 0 } } -invalidcommand { bell } -justify center } { entry .deb.b.act.stack.e -width 3 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set stack_flag %%P; return 1 } { %%W delete 0 end; %%W insert 0 $stack_flag; return 0 } } -invcmd { bell } -justify center }" );
		cmd( ".deb.b.act.stack.e insert 0 $stack_flag" ); 
		cmd( "pack .deb.b.act.stack.l .deb.b.act.stack.e -side left -pady 1 -expand no -fill none" );
		
		if ( mode == 1 )
			cmd( "pack .deb.b.act.run .deb.b.act.until .deb.b.act.ok .deb.b.act.call .deb.b.act.prn_v .deb.b.act.an .deb.b.act.prn_stck .deb.b.act.stack -padx 10 -pady 5 -side left -expand no -fill none" );
		else
			cmd( "pack .deb.b.act.an .deb.b.act.prn_stck .deb.b.act.stack -padx 10 -pady 5 -side left -expand no -fill none" );
	
		cmd( "pack .deb.b.act -expand no -fill none -anchor e" );
	}
}

app_res = *res;
Tcl_LinkVar( inter, "value", ( char * ) &app_res, TCL_LINK_DOUBLE );
cmd( "set value_change 0" );

choice = 0;

while ( choice == 0 )
{
	// if necessary, create the variable name and the time info bar
	if ( mode == 1 || mode == 4 )
	{
		cmd( "if { ! [ winfo exists .deb.v ] } { \
				frame .deb.v; \
				frame .deb.v.v1; \
				label .deb.v.v1.name1 -text \"Variable:\"; \
				label .deb.v.v1.name2 -width 20 -anchor w -fg red -text \"\"; \
				label .deb.v.v1.time1 -text \"Time step:\"; \
				label .deb.v.v1.time2 -width 5 -anchor w -fg red; \
				label .deb.v.v1.val1 -text \"Value \"; \
				entry .deb.v.v1.val2 -width 15 -validate key \
				-justify center -state disabled -vcmd { \
					if [ string is double -strict %%P ] { \
						set value %%P; \
						set value_change 1; \
						return 1 \
					} { \
						%%W delete 0 end; \
						if [ string is double -strict $value ] { \
							%%W insert 0 [ format \"%%g\" $value ] \
						} { \
							%%W insert 0 $value \
						}; \
						return 0 \
					} \
				}; \
				label .deb.v.v1.obs -text \"\"; \
				if { %d == 1 } { \
					pack .deb.v.v1.name1 .deb.v.v1.name2 .deb.v.v1.time1 .deb.v.v1.time2 .deb.v.v1.val1 .deb.v.v1.val2 .deb.v.v1.obs -side left; \
					bind .deb <KeyPress-g> { set choice 77 }; \
					bind .deb <KeyPress-G> { set choice 77 } \
				} { \
					pack .deb.v.v1.name1 .deb.v.v1.name2 .deb.v.v1.time1 .deb.v.v1.time2 -side left \
				} \
			}", mode );
		cmd( ".deb.v.v1.name2 conf -text \"%s\"", lab );
		Tcl_LinkVar( inter, "time", ( char * ) &t, TCL_LINK_INT );
		cmd( ".deb.v.v1.time2 conf -text \"$time      \"" );
		Tcl_UnlinkVar( inter, "time" );
	}

	deb_show( r );

	cmd( "pack .deb.b -side right -expand no -expand no -fill none" );

	cmd( "if $justCreated { showtop .deb topleftW 0 1; set justCreated false }" );

	cmd( "raise .deb; focus .deb" );
	
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

	debug_maincycle:

	if ( mode == 1 )
	{
		cmd( "write_any .deb.b.act.stack.e $stack_flag" ); 
		
		if ( interact )
		{	// write 3 time because of Tcl bug
			cmd( "if [ string is double -strict $value ] { \
					.deb.v.v1.val2 configure -state normal; \
					write_any .deb.v.v1.val2 [ format %%g $value ]; \
					write_any .deb.v.v1.val2 [ format %%g $value ]; \
					write_any .deb.v.v1.val2 [ format %%g $value ]; \
					.deb.v.v1.val2 selection range 0 end; \
					focus .deb.v.v1.val2; \
					bind .deb.v.v1.val2 <Return> { .deb.b.act.run invoke } \
			}" );
		}
		else
			cmd( "if [ string is double -strict $value ] { write_any .deb.v.v1.val2 [ format %%g $value ] }" ); 
	}

	// debug command loop
	while ( ! choice )
	{
		try
		{
			Tcl_DoOneEvent( 0 );
		}
		catch ( bad_alloc& ) 		// raise memory problems
		{
			throw;
		}
		catch ( ... )				// ignore the rest
		{
			goto debug_maincycle;
		}
	}   

	if ( mode == 1 )
	{
		cmd( "if [ string is double -strict [ .deb.v.v1.val2 get ] ] { set value [ .deb.v.v1.val2 get ] }" ); 
		cmd( "set stack_flag [ .deb.b.act.stack.e get ]" ); 
		cmd( "bind .deb <KeyPress-g> { }; bind .deb <KeyPress-G> { }" );
		i = choice;
		cmd( "set choice $stack_flag" );
		stack_info = choice;
		choice = i;
	} 

	switch ( choice )
	{	
		// Step
		case 1:
			if ( t >= max_step )
			{
				cmd( "destroytop .deb" );
				debug_flag = false;
			}
			break;

		// Run
		case 2:
			cmd( "destroytop .deb" );
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
				cmd( "set answer [ tk_messageBox -parent .deb -type okcancel -default cancel -icon warning -title Warning -message \"Stop simulation\" -detail \"Quitting the simulation run.\nPress 'OK' to confirm.\" ]; if [ string equal $answer ok ] { set choice 0 } { set choice 1 }" );
			} 
			else
				choice = 0; 

			if ( choice == 1 ) 
			{
				choice = deb( r, c, lab, res, interact );
				break;
			}

			cmd( "destroytop .deb" );
			choice = 1;

			// prevent changing run parameters when only data browse was called
			if ( mode == 1 )
			{
				quit = 1;
				debug_flag = false;
			}

			if ( mode == 3 )
				cmd( "wm deiconify .log; raise .log; focus .log" );

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

			cmd( "frame $e.n" );
			switch ( cv->param )
			{
				case 1:
					cmd( "label $e.n.l -text \"Parameter:\"" );
					break;

				case 2:
					cmd( "label $e.n.l -text \"Function:\"" );
					break;
					
				case 0:
					cmd( "label $e.n.l -text \"Variable:\"" );
					break;
			}
			cmd( "label $e.n.v -fg red -text $res" );
			cmd( "pack $e.n.l $e.n.v -side left -padx 2" );

			cmd( "frame $e.t" );
			cmd( "label $e.t.l -text \"Current time:\"" );
			cmd( "label $e.t.v -fg red -text $time" );
			cmd( "pack $e.t.l $e.t.v -side left -padx 2" );

			cmd( "frame $e.u" );
			cmd( "label $e.u.l -text \"Last update time:\"" );
			cmd( "label $e.u.v -fg red -text %d", cv->last_update );
			cmd( "pack $e.u.l $e.u.v -side left -padx 2" );

			cmd( "frame $e.x" );
			cmd( "label $e.x.l -text \"Next update time:\"" );
			cmd( "label $e.x.v -fg red -text %d", cv->next_update > 0 ? cv->next_update : cv->last_update < t ? t : t + 1 );
			cmd( "pack $e.x.l $e.x.v -side left -padx 2" );

			cmd( "frame $e.v" );
			for ( i = 0; i <= eff_lags; ++i )
			{
				cmd( "set val%d %g", i, cv->val[ i ] );
				app_values[ i ] = cv->val[ i ];
				sprintf( ch, "val%d", i );
				Tcl_LinkVar( inter, ch, ( char * ) &( app_values[ i ] ), TCL_LINK_DOUBLE );

				cmd( "frame $e.v.l$i" );

				if ( i == 0 )
					cmd( "label $e.v.l$i.l -text \"Value:\"" );
				else
					cmd( "label $e.v.l$i.l -text \"Lag $i:\"" );

				cmd( "entry $e.v.l%d.e -width 15 -validate focusout -vcmd { if [ string is double -strict %%P ] { set val%d %%P; return 1 } { %%W delete 0 end; %%W insert 0 $val%d; return 0 } } -invcmd { bell } -justify center", i, i, i );
				cmd( "$e.v.l%d.e insert 0 $val%d", i, i ); 

				cmd( "button $e.v.l$i.sa -width 5 -text \"Set All\" -command { set sa %i; set choice 10 }", i );
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
				
				cmd( "frame $e.d" );
				cmd( "checkbutton $e.d.deb -text \"Debug this instance only\" -variable debug" );
				cmd( "checkbutton $e.d.deball -text \"Debug all instances\" -variable debugall -command { if { $debugall == 1 } { set debug 1; set undebugall 0; .deb.stat.d.deb configure -state disabled } { set debug 0; set undebugall 1; .deb.stat.d.deb configure -state normal } }" );
				cmd( "pack $e.d.deb $e.d.deball" );

				cmd( "pack $e.v $e.d -pady 5 -padx 5" );	

				cmd( "frame $e.b1" );
				cmd( "button $e.b1.eq -width $butWid -text Equation -command { set choice 8 }" );
				cmd( "button $e.b1.cond -width $butWid -text \"Set Break\" -command { set choice 7 }" );
				cmd( "button $e.b1.exec -width $butWid -text Update -command { set choice 9 }" );
				cmd( "pack $e.b1.eq $e.b1.cond $e.b1.exec -padx 10 -side left" );
				cmd( "pack $e.b1" );	
			}

			cmd( "donehelp $e b { set choice 1 } { LsdHelp debug.html#content }" );
			cmd( "bind $e.v.l0.e <Return> { set choice 1 }" );

			cmd( "showtop $e" );
			cmd( "focus $e.v.l0.e" );
			cmd( "$e.v.l0.e selection range 0 end" );

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

				cmd( "frame $cb.l" );
				cmd( "label $cb.l.l -text \"Variable:\"" );
				cmd( "label $cb.l.n -fg red -text %s", cv->label );
				cmd( "pack $cb.l.l $cb.l.n -side left -padx 2" );

				cmd( "frame $cb.t" );
				cmd( "label $cb.t.l -text \"Type of conditional break\"" );

				cmd( "frame $cb.t.t -relief groove -bd 2" );
				cmd( "radiobutton $cb.t.t.none -text \"None\" -variable cond -value 0 -command { .deb.cbrk.v.e configure -state disabled }" );
				cmd( "radiobutton $cb.t.t.eq -text \"=\" -variable cond -value 1 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
				cmd( "radiobutton $cb.t.t.gr -text \">\" -variable cond -value 2 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
				cmd( "radiobutton $cb.t.t.le -text \"<\" -variable cond -value 3 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
				cmd( "pack $cb.t.t.none $cb.t.t.eq $cb.t.t.gr $cb.t.t.le -anchor w" );

				cmd( "pack $cb.t.l $cb.t.t" );

				cmd( "frame $cb.v" );
				cmd( "label $cb.v.l -text Value" );
				cmd( "entry $cb.v.e -width 10 -validate focusout -vcmd { if [ string is double -strict %%P ] { set cond_val %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cond_val; return 0 } } -invcmd { bell } -justify center -state disabled" );
				cmd( "write_any $cb.v.e $cond_val" ); 
				cmd( "if { $cond != 0 } { $cb.v.e configure -state normal; $cb.v.e selection range 0 end; focus $cb.v.e }" );
				cmd( "pack $cb.v.l $cb.v.e" );

				cmd( "pack $cb.l $cb.t $cb.v -padx 5 -pady 5" );

				cmd( "okhelpcancel $cb b { set choice 1 } { LsdHelp debug.html#cond } { set choice 2 }" );
				cmd( "bind $cb.v.e <Return> { set choice 1 }" );

				cmd( "showtop $cb" );

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "set cond_val [ $cb.v.e get ]" ); 

				cmd( "destroytop $cb" );

				if ( choice == 1 )
				{
					get_int( "cond", &cv->deb_cond );
					get_double( "cond_val", &cv->deb_cnd_val );
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
			Tcl_LinkVar( inter, "value_search", ( char * ) &value_search, TCL_LINK_DOUBLE );
			Tcl_LinkVar( inter, "condition", ( char * ) &cond, TCL_LINK_INT );
			cond = 0;
			choice = 0;
			value_search = 0;
			i = 1;

			cmd( "set s .deb.so" );
			cmd( "newtop $s \"Find Object\" { set choice 2 } .deb" );

			cmd( "frame $s.l" );
			cmd( "label $s.l.l -text \"Find object containing variable\"" );
			cmd( "entry $s.l.e -width 20 -justify center -textvariable en" );
			cmd( "bind $s.l.e <KeyRelease> { if { %%N < 256 && [ info exists modElem ] } { set bb1 [ .deb.so.l.e index insert ]; set bc1 [ .deb.so.l.e get ]; set bf1 [ lsearch -glob $modElem $bc1* ]; if { $bf1 !=-1 } { set bd1 [ lindex $modElem $bf1 ]; .deb.so.l.e delete 0 end; .deb.so.l.e insert 0 $bd1; .deb.so.l.e index $bb1; .deb.so.l.e selection range $bb1 end } } }" );
			cmd( "pack $s.l.l $s.l.e" );

			cmd( "frame $s.c" );
			cmd( "label $s.c.l -text \"Conditional to value\"" );

			cmd( "frame $s.c.cond -relief groove -bd 2" );
			cmd( "radiobutton $s.c.cond.any -text \"Any\" -variable condition -value 0 -command { .deb.so.v.e configure -state disabled }" );
			cmd( "radiobutton $s.c.cond.eq -text \"=\" -variable condition -value 1 -command { .deb.so.v.e configure -state normal }" );
			cmd( "radiobutton $s.c.cond.geq -text \u2265 -variable condition -value 2 -command { .deb.so.v.e configure -state normal }" );
			cmd( "radiobutton $s.c.cond.leq -text \u2264 -variable condition -value 3 -command { .deb.so.v.e configure -state normal }" );
			cmd( "radiobutton $s.c.cond.gr -text \">\" -variable condition -value 4 -command { .deb.so.v.e configure -state normal }" );
			cmd( "radiobutton $s.c.cond.le -text \"<\" -variable condition -value 5 -command { .deb.so.v.e configure -state normal }" );
			cmd( "radiobutton $s.c.cond.not -text \u2260 -variable condition -value 6 -command { .deb.so.v.e configure -state normal }" );
			cmd( "pack $s.c.cond.any $s.c.cond.eq $s.c.cond.geq $s.c.cond.leq $s.c.cond.gr $s.c.cond.le $s.c.cond.not -anchor w" );

			cmd( "pack $s.c.l $s.c.cond" );

			cmd( "frame $s.v" );
			cmd( "label $s.v.l -text Value" );
			cmd( "entry $s.v.e -width 10 -validate focusout -vcmd { if [ string is double -strict %%P ] { set value_search %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value_search; return 0 } } -invcmd { bell } -justify center -state disabled" );
			cmd( "write_any $s.v.e $value_search" ); 
			cmd( "pack $s.v.l $s.v.e" );

			cmd( "pack $s.l $s.c $s.v -padx 5 -pady 5" );

			cmd( "okhelpcancel $s b { set choice 1 } { LsdHelp debug.html#find } { set choice 2 }" );

			cmd( "bind $s.l.e <KeyPress-Return> { focus .deb.so.b.ok }" );
			cmd( "bind $s.v.e <KeyPress-Return> { focus .deb.so.b.ok }" );

			cmd( "showtop $s" );
			cmd( "focus $s.l.e" );
			cmd( "$s.l.e selection range 0 end" );

			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			if ( choice == 2 )
			{
				quit = 0;
				cmd( "destroytop $s" );
				Tcl_UnlinkVar( inter, "value_search" );
				Tcl_UnlinkVar( inter, "condition" );
				choice = 0;
				break;
			}
			 
			pre_running = running;
			running = false;

			cmd( "set value_search [ $s.v.e get ]" ); 
			ch1 = ( char * ) Tcl_GetVar( inter, "en", 0 );
			strcpy( ch, ch1);

			cur = NULL;
			switch ( cond )
			{
				case 0:
					cv = r->search_var( r, ch );
					if ( cv != NULL )
						cur = cv->up;
					break;
				 
				case 1:
					i = 0;
					cur2 = NULL;
					for ( cur1 = r; cur1 != NULL && i == 0; cur1 = cur1->up )
					{
						cv = cur1->search_var( r, ch );
						if ( cv == NULL )
							break;
					 
						cur = cv->up;

						for ( ; cur != NULL && i == 0; cur = cur->hyper_next( cur->label ) )
						{
							app_res = cur->search_var( cur, ch )->val[ 0 ];
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
					cv = r->search_var( r, ch );
					if ( cv != NULL )
						cur = cv->up;
					while ( cur != NULL && cur->cal( ch, 0 ) < value_search )
						cur = cur->hyper_next( cur->label );
					break;

				case 3:
					cv = r->search_var( r, ch );
					if ( cv != NULL )
						cur = cv->up;
					while ( cur != NULL && cur->cal( ch, 0 ) > value_search )
						cur = cur->hyper_next( cur->label );
					break;

				case 4:
					cv = r->search_var( r, ch );
					if ( cv != NULL )
						cur = cv->up;
					while ( cur != NULL && cur->cal( ch, 0 ) <= value_search )
						cur = cur->hyper_next( cur->label );
					break;

				case 5:
					cv = r->search_var( r, ch );
					if ( cv != NULL )
						cur = cv->up;
					while ( cur != NULL && cur->cal( ch, 0 ) >= value_search )
						cur = cur->hyper_next( cur->label );
					break;

				case 6:
					cv = r->search_var( r, ch );
					if ( cv != NULL )
						cur = cv->up;
					while ( cur != NULL && cur->cal( ch, 0 ) == value_search )
						cur = cur->hyper_next( cur->label );
					break;

				default:
					cur = NULL;
			}

			quit = 0; // If var is mispelled don't stop the simulation!
			cmd( "destroytop $s" );
			Tcl_UnlinkVar( inter, "value_search" );
			Tcl_UnlinkVar( inter, "condition" );

			if ( cur != NULL )
				choice = deb( cur, r, lab, res, interact );
			else
				choice = 0;

			running = pre_running;
			break;

		// Analysis
		case 11:
			actual_steps = t;
			for ( cur = r; cur->up != NULL; cur = cur->up );
			reset_end( cur );
			analysis( &choice );
			choice = 0;
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
			cmd( "wm deiconify .log; raise .log; focus .log" );
			choice = 0;
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
			choice = 0;
			break;

		// Until
		case 16:
			cmd( "set tdebug %d", t + 1 );

			cmd( "set t .deb.tdeb" );
			cmd( "newtop $t \"Run Until\" { set choice 2 } .deb" );

			cmd( "frame $t.t" );
			cmd( "label $t.t.l -text \"Run until time step\"" );
			cmd( "entry $t.t.val -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set tdebug %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tdebug; return 0 } } -invcmd { bell } -justify center" );
			cmd( "$t.t.val insert 0 $tdebug" ); 
			cmd( "pack $t.t.l $t.t.val" );

			cmd( "pack $t.t -padx 5 -pady 5" );

			cmd( "okhelpcancel $t b { set choice 1 } { LsdHelp debug.html#until } { set choice 2 }" );

			cmd( "bind $t.t.val <Return> { set choice 1 }" );

			cmd( "showtop $t" );
			cmd( "$t.t.val selection range 0 end" );
			cmd( "focus $t.t.val" );

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
			}
			else
				choice = 0;

			break;

		// change the object number of instances (click on level / object instance)
		case 17:
			if ( r->up != NULL )
				entry_new_objnum( r, &choice, "" );

			choice = 0;
			break;

		// Hooks
		case 21:
			j = r->hooks.size( );						// number of dynamic hooks
			
			if ( j > 0 )
			{
				cmd( "set hook 0" );
				
				cmd( "set hk .deb.hk" );
				
				cmd( "newtop $hk \"Choose Hook\" { set choice 2 } .deb" );
				
				cmd( "frame $hk.l" );
				cmd( "label $hk.l.l -text \"Object:\"" );
				cmd( "label $hk.l.n -fg red -text %s", r->label );
				cmd( "pack $hk.l.l $hk.l.n -side left -padx 2" );

				cmd( "frame $hk.t" );
				cmd( "label $hk.t.l -text \"Available hooks\"" );

				cmd( "frame $hk.t.t -relief groove -bd 2" );
				
				strcpy( ch, "pack" );
				
				for ( count = i = 0; i < j; ++i )
					if ( r->hooks[ i ] != NULL )
					{
						k = root->search_inst( r->hooks[ i ] );
						
						if ( k != 0 )
						{
							cmd( "radiobutton $hk.t.t.h%d -text \"Hook %d to %s (%d)\" -variable hook -value %d", i, i, r->hooks[ i ]->label, k, i );
							sprintf( ch2, " $hk.t.t.h%d", i );
							strncat( ch, ch2, 4 * MAX_ELEM_LENGTH - strlen( ch ) - 1 );
							++count;
						}
					}
				
				if ( r->hook != NULL )
				{
					k = root->search_inst( r->hook );
					
					if ( k != 0 )
					{
						cmd( "radiobutton $hk.t.t.h%d -text \"Static Hook to %s (%d)\" -variable hook -value %d", i, r->hook->label, k, i );
						sprintf( ch2, " $hk.t.t.h%d", i );
						strncat( ch, ch2, 4 * MAX_ELEM_LENGTH - strlen( ch ) - 1 );
						++count;
					}
				}
				
				if ( count == 0 )
				{
					cmd( "destroytop $hk" );
					cmd( "tk_messageBox -parent .deb -type ok -icon error -title Error -message \"Invalid hook pointer(s)\" -detail \"Check if your code is using valid (non-NULL) pointers to LSD objects or avoid using this option.\"" );
					choice = 0;
					break;
				}
				
				strcat( ch, " -anchor w" );
				cmd( ch );

				cmd( "pack $hk.t.l $hk.t.t" );

				cmd( "frame $hk.m" );
				cmd( "label $hk.m.l -text \"(invalid and NULL hooks not shown)\"" );
				cmd( "pack $hk.m.l" );
				
				cmd( "pack $hk.l $hk.t $hk.m -padx 5 -pady 5" );

				cmd( "okhelpcancel $hk b { set choice 1 } { LsdHelp debug.html#hooks } { set choice 2 }" );

				cmd( "showtop $hk" );
				
				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "destroytop $hk" );

				if ( choice == 1 )
				{
					get_int( "hook", &i );
					
					if ( i < j )
						choice = deb( r->hooks[ i ], c, lab, res, interact );
					else
						choice = deb( r->hook, c, lab, res, interact );
				}
				else
					choice = 0;
			}
			else
				if ( r->hook != NULL )
				{
					if ( root->search_inst( r->hook ) == 0 )
					{
						cmd( "tk_messageBox -parent .deb -type ok -icon error -title Error -message \"Invalid hook pointer\" -detail \"Check if your code is using valid pointers to LSD objects or avoid using this option.\"" );
						choice = 0;
						break;
					}
					
					choice = deb( r->hook, c, lab, res, interact );
				}
				else
					choice = 0;

			break;
					
		// Network
		case 22:
			show_neighbors( r, false );
			choice = 0;
			break;
			
		// double-click (change to) network node
		case 23:
			ch1 = ( char * ) Tcl_GetVar( inter, "nodeLab", 0 );
			get_long( "nodeId", & node );
			cur = root->search_node_net( ( const char * ) ch1, node );
			if ( cur != NULL )
				choice = deb( cur, c, lab, res, interact );
			else
				choice = 0;

			break;

		// double-click (change to) object pointer
		case 24:
			ch1 = ( char * ) Tcl_GetVar( inter, "objLab", 0 );
			get_int( "objNum", & i );
			cur = root->search( ( const char * ) ch1 );
			for ( j = 1; j != i && cur != NULL; ++j, cur = cur->hyper_next( ) );
			if ( cur != NULL )
				choice = deb( cur, c, lab, res, interact );
			else
				choice = 0;

			break;

		// right-click (set all) on multi-instanced parameter or variable
		case 29:
			ch1 = ( char * )Tcl_GetVar( inter, "res", 0 );
			strcpy( ch, ch1 );
			set_all( &choice, r, ch, 0 );
			choice = 0;
			break;
				
		// Model Report
		case 44:
			cmd( "set name_rep %s", name_rep );

			cmd( "set choice [ file exists $name_rep ]" );

			cmd( "if { $choice == 1 } { LsdHtml $name_rep }" );
			cmd( "if { $choice == 0 } { tk_messageBox -parent .deb -type ok -title Error -icon error -message \"Report file not available\" -detail \"You can create the report in menu Model.\" }" );
			choice = 0;
			break;

		// Debug variable under computation CTRL+G
		case 77: 
			if ( asl == NULL && stacklog != NULL )
			{
				asl = stacklog;
				plog( "\nVariable: %s", "", asl->label );
				if ( asl->vs != NULL && asl->vs->up != NULL )
					choice = deb( asl->vs->up, c, lab, res, interact );
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
				}
				else
				{
					asl = asl->next;
					plog( "\nVariable: %s", "", asl->label );
					if ( asl->vs != NULL && asl->vs->up != NULL )
						choice = deb( asl->vs->up, c, lab, res, interact );
				}
			}  
			break;  
			
		default:
			choice = 0;
	}
}

// only update if user typed a new valid value
cmd( "if { $value_change == 0 } { set value %lf }", *res );
*res = app_res;

non_var = false;

Tcl_UnlinkVar( inter, "value" );

return choice;
}


/*******************************************
DEB_SHOW
********************************************/
void deb_show( object *r )
{
	char ch[ 2 * MAX_ELEM_LENGTH ];
	variable *ap_v;
	int i;

	// fix the top frame before proceeding
	cmd( "if { ! [ winfo exists .deb.v ] } { frame .deb.v }" );
	cmd( "if { ! [ winfo exists .deb.v.v2 ] } { \
			frame .deb.v.v2; \
			label .deb.v.v2.obj -text \"Level & object instance: \"; \
			label .deb.v.v2.instance -fg red -text \"\"; \
			pack .deb.v.v2.obj .deb.v.v2.instance -side left; \
			if [ winfo exists .deb.v.v1 ] { \
				pack .deb.v.v1 .deb.v.v2 -padx 5 -pady 5 -anchor w \
			} { \
				pack .deb.v.v2 -padx 5 -pady 5 -anchor w \
			}; \
			pack .deb.v -anchor w -expand no -fill x \
		}" );

	if ( r->up != NULL )
	{
		cmd( "bind .deb.v.v2.obj <Button-1> {if { [winfo exist .deb.w] } { destroy .deb.w }; set choice 17}" );
		cmd( "bind .deb.v.v2.instance <Button-1> {if { [winfo exist .deb.w] } { destroy .deb.w }; set choice 17}" );
	}

	strcpy( ch, "" );
	attach_instance_number( ch, r );
	cmd( ".deb.v.v2.instance config -text \"%s\"", ch  );

	// adjust spacing to align labels with data and increase columns width to better fill window
	cmd( "if [ string equal $tcl_platform(platform) windows ] { set w1 20; set w2 25; set w3 10; set wwidth 100 }" );
	cmd( "if [ string equal $tcl_platform(platform) unix ] { set w1 20; set w2 24; set w3 10; set wwidth 100 }" );
	cmd( "if [ string equal $tcl_platform(os) Darwin ] { set w1 15; set w2 18; set w3 9; set wwidth 115 }" );

	cmd( "if { ! [ winfo exists .deb.tit ] } { \
			frame .deb.tit; \
			frame .deb.tit.h1; \
			label .deb.tit.h1.pad -width 1 -pady 0 -bd 0 ; \
			label .deb.tit.h1.name -text Variable -width $w1 -pady 0 -bd 0 -anchor w; \
			label .deb.tit.h1.val -text Value -width $w2 -pady 0 -bd 0 -fg red; \
			label .deb.tit.h1.last -text \"Last update\" -width $w3 -pady 0 -bd 0; \
			pack .deb.tit.h1.pad .deb.tit.h1.name .deb.tit.h1.val .deb.tit.h1.last -side left; \
			label .deb.tit.pad -pady 0 -bd 0 -text \u2009; \
			frame .deb.tit.h2; \
			label .deb.tit.h2.pad -width 1 -pady 0 -bd 0 ; \
			label .deb.tit.h2.name -text Variable -width $w1 -pady 0 -bd 0 -anchor w; \
			label .deb.tit.h2.val -text Value -width $w2 -pady 0 -bd 0 -fg red; \
			label .deb.tit.h2.last -text \"Last update\" -width $w3 -pady 0 -bd 0; \
			pack .deb.tit.h2.pad .deb.tit.h2.name .deb.tit.h2.val .deb.tit.h2.last -side left -anchor w; \
			pack .deb.tit.h1 .deb.tit.pad .deb.tit.h2 -expand no -side left; \
			pack .deb.tit -side top -anchor w -expand no -after .deb.v \
		}" );

	cmd( "if { ! [ winfo exists .deb.cc ] } { \
			frame .deb.cc; \
			scrollbar .deb.cc.scroll -command \".deb.cc.l yview\"; \
			pack .deb.cc.scroll -side right -fill y; \
			text .deb.cc.l -yscrollcommand \".deb.cc.scroll set\" -wrap none -width $wwidth -cursor arrow; \
			mouse_wheel .deb.cc.l; \
			pack .deb.cc.l -expand yes -fill both; \
			pack .deb.cc -expand yes -fill both\
		} { \
			.deb.cc.l configure -state normal\
		}" );

	cmd( ".deb.cc.l delete 1.0 end" );

	if ( r->v == NULL )
	{
		cmd( "label .deb.cc.l.no_var -text \"(no variables defined)\"" );
		cmd( ".deb.cc.l window create end -window .deb.cc.l.no_var" );
	}
	else
	{
		Tcl_LinkVar( inter, "i", ( char * ) &i, TCL_LINK_INT );

		for ( i = 1, ap_v = r->v; ap_v != NULL; ap_v = ap_v->next, ++i )
		{
			cmd( "set last %d", ap_v->last_update );
			cmd( "set val %g", ap_v->val[ 0 ] );
			cmd( "frame .deb.cc.l.e$i" );
			cmd( "label .deb.cc.l.e$i.pad1 -width 1 -pady 0 -bd 0" );
			cmd( "label .deb.cc.l.e$i.name -width $w1 -pady 0 -anchor w -bd 0 -text %s", ap_v->label );
			
			if ( is_nan( ap_v->val[ 0 ] ) )
				cmd( "label .deb.cc.l.e$i.val -width $w2 -pady 0 -bd 0 -fg red -text NAN" );
			else
				if ( is_inf( ap_v->val[ 0 ] ) )
					cmd( "label .deb.cc.l.e$i.val -width $w2 -pady 0 -bd 0 -fg red -text %sINFINITY", ap_v->val[ 0 ] < 0 ? "-" : "" );
				else
					if ( ap_v->val[ 0 ] != 0 && fabs( ap_v->val[ 0 ] ) < SIG_MIN )	// insignificant value?			
						cmd( "label .deb.cc.l.e$i.val -width $w2 -pady 0 -bd 0 -fg red -text ~0" );
					else
						cmd( "label .deb.cc.l.e$i.val -width $w2 -pady 0 -bd 0 -fg red -text $val" );
			
			if ( ap_v->param == 0 )
				cmd( "label .deb.cc.l.e$i.last -width $w3 -pady 0 -bd 0 -text $last" );
			if ( ap_v->param == 1 )
				cmd( "label .deb.cc.l.e$i.last -width $w3 -pady 0 -bd 0 -text (P)" );
			if ( ap_v->param == 2 )
				cmd( "label .deb.cc.l.e$i.last -width $w3 -pady 0 -bd 0 -text (F)" );
			
			if ( i % 2 == 0 )
			{
				cmd( "label .deb.cc.l.e$i.pad2 -pady 0 -bd 0 -bg white -text \u2009" );
			
				cmd( "pack .deb.cc.l.e$i.pad2 .deb.cc.l.e$i.pad1 .deb.cc.l.e$i.name .deb.cc.l.e$i.val .deb.cc.l.e$i.last -side left" );
			}
			else
				cmd( "pack .deb.cc.l.e$i.pad1 .deb.cc.l.e$i.name .deb.cc.l.e$i.val .deb.cc.l.e$i.last -side left" );

			cmd( "bind .deb.cc.l.e$i.name <Button-1> { set res %s; set lstDebPos [ .deb.cc.l index @%%x,%%y ]; set choice 8 }", ap_v->label );
			cmd( "bind .deb.cc.l.e$i.name <Button-2> { set res %s; set lstDebPos [ .deb.cc.l index @%%x,%%y ]; set choice 29 }", ap_v->label );
			cmd( "bind .deb.cc.l.e$i.name <Button-3> { event generate .deb.cc.l.e$i.name <Button-2> -x %%x -y %%y }" );

			cmd( ".deb.cc.l window create end -window .deb.cc.l.e$i" );
			if ( i % 2 == 0 )
				cmd( ".deb.cc.l insert end \\n" );
		}
	   
		cmd( "if [ info exists lstDebPos ] { .deb.cc.l see $lstDebPos; unset lstDebPos }" );
		
		Tcl_UnlinkVar( inter, "i" );
	}

	cmd( ".deb.cc.l configure -state disabled" );
}


/*******************************************
SHOW_TMP_VARS
********************************************/
void show_tmp_vars( object *r, bool update )
{
	bool valid;
	char i_names[ ] = { 'i', 'j', 'h', 'k' };
	int i, j, m, n = 0;
	netLink *curLnk = NULL;
	object *cur;
	
	cmd( "set in .deb.val" );
	cmd( "set existVal [ winfo exists $in ]" );
	if ( ! strcmp( Tcl_GetVar( inter, "existVal", 0 ), "0" ) )
	{
		cmd( "newtop $in \"v\\[...\\]\" { destroytop .deb.val } .deb" ); 

		cmd( "frame $in.l1" );
		cmd( "label $in.l1.l -text \"Name and instance\"" );
		cmd( "frame $in.l1.n" );
		cmd( "label $in.l1.n.name -foreground red" );
		cmd( "label $in.l1.n.sep -text |" );
		cmd( "label $in.l1.n.id -foreground red" );
		cmd( "pack $in.l1.n.name $in.l1.n.sep $in.l1.n.id -side left" );
		cmd( "pack $in.l1.l $in.l1.n" );
		
		cmd( "frame $in.l2" );
		cmd( "label $in.l2.id -width 6 -text Variable" );
		cmd( "label $in.l2.pad" );
		cmd( "label $in.l2.val -width 14 -fg red -text Value" );
		cmd( "pack $in.l2.id $in.l2.pad $in.l2.val -side left" );

		cmd( "pack $in.l1 $in.l2 -pady 2" );

		cmd( "frame $in.n" );
		cmd( "scrollbar $in.n.yscroll -command \"$in.n.t yview\"" );
		cmd( "pack $in.n.yscroll -side right -fill y" );
		cmd( "text $in.n.t -width 18 -height 27 -yscrollcommand \"$in.n.yscroll set\" -wrap none -cursor arrow" );
		cmd( "mouse_wheel $in.n.t" );
		cmd( "pack $in.n.t -expand yes -fill both" );
		cmd( "pack $in.n -expand yes -fill both" );
		
		cmd( "label $in.l3 -text \"(double-click name to\nchange to object)\"" );
		cmd( "pack $in.l3 -pady 5" );

		cmd( "showtop $in topleftW 0 1 0" );

		cmd( "$in.n.t tag configure bold -font [ font create -family TkDefaultFont -size $small_character -weight bold ]" );

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
	
	m = root->search_inst( r );
	cmd( "$in.l1.n.name configure -text \"%s\"", r->label == NULL ? "" : r->label );
	cmd( "$in.l1.n.id configure -text \"%d\"", m );
	
	Tcl_LinkVar( inter, "i", ( char * ) &i, TCL_LINK_INT );
	
	cmd( "$in.n.t insert end \"Temporary storage\n\" bold" );
	
	for ( i = 1, j = 0; j < 10; ++i, ++j )
	{
		cmd( "frame $in.n.t.n$i" );
		cmd( "label $in.n.t.n$i.var -width 6 -pady 0 -bd 0 -text \"v\\\[%d\\]\"", j );
		cmd( "label $in.n.t.n$i.pad -width 1 -pady 0 -bd 0" );
		
		if ( is_nan( d_values[ j ] ) )
			cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text NAN" );
		else
			if ( is_inf( d_values[ j ] ) )
				cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text %sINFINITY", d_values[ j ] < 0 ? "-" : "" );
			else
				if ( d_values[ j ] != 0 && fabs( d_values[ j ] ) < SIG_MIN )	// insignificant value?
					cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text ~0" );
				else
					cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text %g", d_values[ j ] );
				
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		cmd( "$in.n.t window create end -window $in.n.t.n$i" );
		cmd( "$in.n.t insert end \\n" );
	}
	
	cmd( "$in.n.t insert end \"Integer indexes\n\" bold" );
	
	for ( j = 0; j < ( int ) ( sizeof i_names / sizeof i_names[ 0 ] ); ++i, ++j )
	{
		cmd( "frame $in.n.t.n$i" );
		cmd( "label $in.n.t.n$i.var -width 6 -pady 0 -bd 0 -text \"%c\"", i_names[ j ] );
		cmd( "label $in.n.t.n$i.pad -width 1 -pady 0 -bd 0" );
		
		cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text %d", i_values[ j ] );

		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		cmd( "$in.n.t window create end -window $in.n.t.n$i" );
		cmd( "$in.n.t insert end \\n" );
	}
	
	cmd( "$in.n.t insert end \"Object pointers\n\" bold" );
	
	for ( j = 0; j < 10; ++i, ++j )
	{
		cmd( "frame $in.n.t.n$i" );
		cmd( "label $in.n.t.n$i.pad -width 1 -pady 0 -bd 0" );
		
		if ( j == 0 )
			cmd( "label $in.n.t.n$i.var -width 6 -pady 0 -bd 0 -text cur" );
		else
			cmd( "label $in.n.t.n$i.var -width 6 -pady 0 -bd 0 -text \"cur%d\"", j );
		
		valid = false;
		if ( o_values[ j ] == NULL )
			cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text NULL" );
		else
		{
			// search an object pointed by the pointer
			n = ( int ) root->search_inst( o_values[ j ] );
			if ( n > 0 && o_values[ j ]->label != NULL )
			{
				cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text \"%s(%d)\"", o_values[ j ]->label, n );
				valid = true;
			}
			else
				cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text \"(invalid)\"" );
		}
		
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		if ( valid )
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
		cmd( "frame $in.n.t.n$i" );
		cmd( "label $in.n.t.n$i.pad -width 1 -pady 0 -bd 0" );
		
		if ( j == 0 )
			cmd( "label $in.n.t.n$i.var -width 6 -pady 0 -bd 0 -text curl" );
		else
			cmd( "label $in.n.t.n$i.var -width 6 -pady 0 -bd 0 -text \"curl%d\"", j );
		
		if ( n_values[ j ] == NULL )
			cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text NULL" );
		else
		{
			// try search a link pointed by the pointer in current object only
			valid = false;
			if ( r->node != NULL )
			{
				for ( curLnk = r->node->first; curLnk != NULL; curLnk = curLnk->next )
					if ( curLnk == n_values[ j ] && curLnk->ptrTo != NULL && curLnk->ptrTo->label != NULL )
					{
						cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text \"%s(%ld)\"", curLnk->ptrTo->label, curLnk->serTo );
						valid = true;
						break;
					}
			}
			
			if ( ! valid )
				cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text \"(unknown)\"" );
		}
		
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		if ( valid && curLnk != NULL )
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
		cmd( "frame $in.n.t.n$i" );
		cmd( "label $in.n.t.n$i.pad -width 1 -pady 0 -bd 0" );

		if ( j < 0 )
		{
			cmd( "label $in.n.t.n$i.var -width 7 -pady 0 -bd 0 -text SHOOK" );
			cur = r->hook;	
		}
		else
		{
			cmd( "label $in.n.t.n$i.var -width 7 -pady 0 -bd 0 -text \"HOOK(%d)\"", j );
			cur = r->hooks[ j ];	
		}
		
		valid = false;
		if ( cur == NULL )
			cmd( "label $in.n.t.n$i.val -width 12 -pady 0 -bd 0 -foreground red -text NULL" );
		else
		{
			// search an object pointed by the hook
			n = ( int ) root->search_inst( cur );
			if ( n > 0 && cur->label != NULL )
			{
				cmd( "label $in.n.t.n$i.val -width 12 -pady 0 -bd 0 -foreground red -text \"%s(%d)\"", cur->label, n );
				valid = true;
			}
			else
				cmd( "label $in.n.t.n$i.val -width 12 -pady 0 -bd 0 -foreground red -text \"(invalid)\"" );
		}
		
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
		if ( valid )
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
		cmd( "frame $in.n.t.n$i" );
		cmd( "label $in.n.t.n$i.var -width 6 -pady 0 -bd 0 -text \"v\\\[%d\\]\"", j );
		cmd( "label $in.n.t.n$i.pad -width 1 -pady 0 -bd 0" );
		
		if ( is_nan( d_values[ j ] ) )
			cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text NAN" );
		else
			if ( is_inf( d_values[ j ] ) )
				cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text %sINFINITY", d_values[ j ] < 0 ? "-" : "" );
			else
				if ( d_values[ j ] != 0 && fabs( d_values[ j ] ) < SIG_MIN )	// insignificant value?
					cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text ~0" );
				else
					cmd( "label $in.n.t.n$i.val -width 13 -pady 0 -bd 0 -foreground red -text %g", d_values[ j ] );
				
		cmd( "pack $in.n.t.n$i.var $in.n.t.n$i.pad $in.n.t.n$i.val -side left" );
		
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

	cmd( "set n .deb.net" );
	cmd( "set existNet [ winfo exists $n ]" );
	if ( ! strcmp( Tcl_GetVar( inter, "existNet", 0 ), "0" ) )
	{
		cmd( "newtop $n \"Network\" { destroytop .deb.net } .deb" );
		
		cmd( "frame $n.l1" );
		cmd( "label $n.l1.l -text \"Node ID and name\"" );
		cmd( "frame $n.l1.n" );
		cmd( "label $n.l1.n.id -foreground red" );
		cmd( "label $n.l1.n.sep -text |" );
		cmd( "label $n.l1.n.name -foreground red" );
		cmd( "pack $n.l1.n.id $n.l1.n.sep $n.l1.n.name -side left" );
		cmd( "pack $n.l1.l $n.l1.n" );
		
		cmd( "frame $n.l2" );
		cmd( "label $n.l2.l -text \"Num. links out:\"" );
		cmd( "label $n.l2.n -foreground red" );
		cmd( "pack $n.l2.l $n.l2.n -side left" );
		
		cmd( "frame $n.l3" );
		cmd( "label $n.l3.l -text \"Outgoing links\"" );
		
		cmd( "frame $n.l3.h" );
		cmd( "label $n.l3.h.id -width 6 -text \"Dest. ID\"" );
		cmd( "label $n.l3.h.pad -width 2" );
		cmd( "label $n.l3.h.wght -width 12 -foreground red -text \"(Weight) \"" );
		cmd( "pack $n.l3.h.id $n.l3.h.pad $n.l3.h.wght -side left" );

		cmd( "pack $n.l3.l $n.l3.h" );

		cmd( "pack $n.l1 $n.l2 $n.l3 -pady 2" );

		cmd( "frame $n.n" );
		cmd( "scrollbar $n.n.yscroll -command \".deb.net.n.t yview\"" );
		cmd( "pack $n.n.yscroll -side right -fill y" );
		cmd( "text $n.n.t -width 18 -height 19 -yscrollcommand \"$n.n.yscroll set\" -wrap none -cursor arrow" );
		cmd( "mouse_wheel $n.n.t" );
		cmd( "pack $n.n.t -expand yes -fill both" );
		cmd( "pack $n.n -expand yes -fill both" );
		
		cmd( "label $n.l4 -text \"(double-click ID to\nchange to node)\"" );
		cmd( "pack $n.l4 -pady 5" );
		
		cmd( "showtop $n topleftW 0 1 0" );
		
		cmd( "if { ! [ winfo exists .deb.val ] } { align $n .deb } { align $n .deb.val }" );
	}
	else
		if ( update )
		{
			cmd( "$n.n.t configure -state normal" );
			cmd( "$n.n.t delete 1.0 end" );
		}
		else
		{
			cmd( "destroytop .deb.net" );
			return;
		}
	
	cmd( "$n.l1.n.id configure -text \"%ld\"", r->node->id );
	cmd( "$n.l1.n.name configure -text \"%s\"", r->node->name == NULL ? "" : r->node->name );
	cmd( "$n.l2.n configure -text %ld", r->node->nLinks );
	
	Tcl_LinkVar( inter, "i", ( char * ) &i, TCL_LINK_INT );
	
	for ( i = 1, curLnk = r->node->first; curLnk != NULL; curLnk = curLnk->next, ++i )
	{
		cmd( "frame $n.n.t.n$i" );
		cmd( "label $n.n.t.n$i.nodeto -width 6 -pady 0 -bd 0 -text %ld", curLnk->ptrTo->node->id );
		cmd( "label $n.n.t.n$i.pad -width 2 -pady 0 -bd 0" );
		if ( curLnk->weight != 0 )
			cmd( "label $n.n.t.n$i.weight -width 12 -pady 0 -bd 0 -foreground red -text %g", curLnk->weight );
		else
			cmd( "label $n.n.t.n$i.weight -width 12 -pady 0 -bd 0" );
		
		cmd( "pack $n.n.t.n$i.nodeto $n.n.t.n$i.pad $n.n.t.n$i.weight -side left" );
		
		cmd( "bind $n.n.t.n$i.nodeto <Double-Button-1> { set nodeId %ld; set nodeLab %s; set choice 23 }", curLnk->ptrTo->node->id, r->label );
		
		if ( curLnk->weight != 0 )
			cmd( "bind $n.n.t.n$i.weight <Double-Button-1> { set nodeId %ld; set nodeLab %s; set choice 23 }", curLnk->ptrTo->node->id, r->label );
		
		cmd( "$n.n.t window create end -window $n.n.t.n$i" );
		cmd( "$n.n.t insert end \\n" );
	}
	
	Tcl_UnlinkVar( inter, "i" );
	cmd( "$n.n.t configure -state disabled" );
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
