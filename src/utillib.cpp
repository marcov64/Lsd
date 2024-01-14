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
UTILLIB.CPP
Contains the basic set of utilities used in DLL or no-window
executables. The remaining functions are stored in
UTIL.CPP.

The main functions contained in this file are:

- void plog( const char *m );
print  message string m in the Log screen or the console.

- void error_hard( const char *boxTitle, const char *boxText,
				   bool defQuit, const char *logFmt, ... );
print error messages to the log screen, console and error
file, recovering LSD configuration to allow for non-crashing
recovery.
*************************************************************/

#include "decl.h"


/*********************************
PLOG
Print message on the log window
*********************************/
void plog( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );
	plog_backend( cm, "", argptr );
	va_end( argptr );
}


/*********************************
PLOG_TAG
The optional tag parameter has to correspond to the log window existing tags
*********************************/
void plog_tag( const char *cm, const char *tag, ... )
{
	static va_list argptr;

	va_start( argptr, tag );
	plog_backend( cm, tag, argptr );
	va_end( argptr );
}


/*********************************
PLOG_BACKEND
Back-end to plog and plog_tag
*********************************/
#define NUM_TAGS 7
const char *tags[ NUM_TAGS ] = { "", "highlight", "table", "series", "prof1", "prof2", "bar" };

void plog_backend( const char *cm, const char *tag, va_list arg )
{
	static bool bufdyn;
	static char *buffer, *message, bufstat[ MAX_BUFF_SIZE ], msgstat[ MAX_BUFF_SIZE ];
	static bool tag_ok;
	static int i, j, reqsz, sz;
	static va_list argcpy;

#ifndef _NW_
	if ( ! tk_ok || ! log_ok )
		return;
#endif

#ifndef _NP_
	// abort if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
		return;
#endif

	buffer = bufstat;
	message = msgstat;
	va_copy( argcpy, arg );

	reqsz = vsnprintf( buffer, MAX_BUFF_SIZE, cm, arg );

	if ( reqsz < 0 )
	{
#ifndef _NW_
		log_tcl_error( true, "Invalid text message", "Cannot expand message '%s...'", cm );
#else
		fprintf( stderr, "\nCannot expand message '%s...'\n", cm );
#endif
		return;
	}

	// handle very large messages
	if ( reqsz >= MAX_BUFF_SIZE )
	{
		buffer = new char[ reqsz + 1 ];
		sz = vsnprintf( buffer, reqsz + 1, cm, argcpy );

		if ( reqsz < 0 || sz > reqsz )
		{
#ifndef _NW_
			log_tcl_error( true, "Invalid text message", "Cannot expand message '%s...'", cm );
#else
			fprintf( stderr, "\nCannot expand message '%s...'\n", cm );
#endif
			delete [ ] buffer;
			return;
		}

		message = new char[ reqsz + 1 ];
		bufdyn = true;
	}
	else
		bufdyn = false;

	va_end( argcpy );

	// remove invalid charaters and Tk control characters
	for ( i = 0, j = 0; buffer[ i ] != '\0' && j < reqsz; ++i )
		if ( ( isprint( buffer[ i ] ) || buffer[ i ] == '\n' ||
			   buffer[ i ] == '\r' || buffer[ i ] == '\t' ) &&
			 ! ( buffer[ i ] == '\"' ||
				 ( buffer[ i ] == '$' && buffer[ i + 1 ] != '$' ) ) )
			message[ j++ ] = buffer[ i ];
	message[ j ] = '\0';

#ifdef _NW_
	printf( "%s", message );
	fflush( stdout );
#else
	for ( tag_ok = false, i = 0; i < NUM_TAGS; ++i )
		if ( ! strcmp( tag, tags[ i ] ) )
			tag_ok = true;

	// handle the "bar" pseudo tag
	if ( strcmp( tag, "bar" ) )
		on_bar = false;

	if ( tag_ok )
	{
		cmd( "set log_ok 0" );
		cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { \
				catch { set log_ok [ winfo exists .log ] } \
			}" );
		cmd( "if $log_ok { .log.text.text.internal see [ .log.text.text.internal index insert ] }" );
		cmd( "if $log_ok { catch { .log.text.text.internal insert end \"%s\" %s } }", message, tag );
		cmd( "if $log_ok { .log.text.text.internal see end }" );
	}
	else
		plog( "\nError: invalid tag, message ignored:\n%s\n", message );
#endif

	message_logged = true;

	if ( bufdyn )
	{
		delete [ ] buffer;
		delete [ ] message;
	}
}


/***********
ERROR_HARD
Procedure called when an unrecoverable error occurs.
Information about the state of the simulation when the error
occurred is provided. Users can abort the program or analyze
the results collected up the latest time step available.
*************/
#ifndef _NP_
mutex error;
#endif

void error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... )
{
	if ( quit == 2 )		// simulation already being stopped
		return;

	static char logText[ MAX_BUFF_SIZE ];
	static va_list argptr;

	va_start( argptr, logFmt );
	vsnprintf( logText, MAX_BUFF_SIZE, logFmt, argptr );
	va_end( argptr );

#ifndef _NP_
	// prevent concurrent use by more than one thread
	lock_guard < mutex > lock( error );

	// abort worker and park message if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
	{
		if ( ! error_hard_thread )	// handle just first error
		{
			error_hard_thread = true;
			strcpyn( error_hard_msg1, boxTitle, MAX_BUFF_SIZE );
			strcpyn( error_hard_msg2, logText, MAX_BUFF_SIZE );
			strcpyn( error_hard_msg3, boxText, MAX_BUFF_SIZE );
			throw 1;
		}
		else
			return;
	}
#endif

#ifndef _NW_
	if ( running )			// handle running events differently
	{
		cmd( "if [ winfo exists .deb ] { destroytop .deb }" );
		deb_log( false );	// close any open debug log file
		reset_plot( );		// show & disable run-time plot
		set_buttons_run( false );

		plog_tag( "\n\nError detected at case (time step): %d", "highlight", t );
		plog( "\n\nError: %s\nDetails: %s", boxTitle, logText );
		if ( ! parallel_mode && stack_log != NULL && stack_log->vs != NULL )
			plog( "\nOffending code contained in the equation for variable: '%s'", stack_log->vs->label );
		plog( "\nSuggestion: %s", boxText );
		print_stack( );
		cmd( "focustop .log" );
		cmd( "ttk::messageBox -parent . -title Error -type ok -icon error -message \"[ string totitle {%s} ]\" -detail \"[ string totitle {%s} ].\n\nMore details are available in the Log window.\n\nSimulation cannot continue.\"", boxTitle, boxText  );
	}
	else
	{
		plog( "\n\nError: %s\nDetails: %s", boxTitle, logText );
		plog( "\nSuggestion: %s\n", boxText );
		cmd( "ttk::messageBox -parent . -title Error -type ok -icon error -message \"[ string totitle {%s} ]\" -detail \"[ string totitle {%s} ].\n\nMore details are available in the Log window.\"", boxTitle, boxText  );
	}
#endif

	if ( ! running )
		return;

	quit = 2;				// do not continue simulation

#ifndef _NW_
	uncover_browser( );
	cmd( "focustop .log" );

	cmd( "set err %d", ( defQuit || worker_errors( ) ) > 0 ? 1 : 2 );

	cmd( "newtop .cazzo Error" );

	cmd( "ttk::frame .cazzo.t" );
	cmd( "ttk::label .cazzo.t.l -style hl.TLabel -text \"An error occurred during the simulation\"" );
	cmd( "pack .cazzo.t.l -pady 10" );
	cmd( "ttk::label .cazzo.t.l1 -justify center -text \"Information about the error is reported in the log window.\nPartial results are available in the LSD browser.\"" );
	cmd( "pack .cazzo.t.l1" );

	cmd( "ttk::frame .cazzo.e" );
	cmd( "ttk::label .cazzo.e.l -text \"Choose one option to continue\"" );

	cmd( "ttk::frame .cazzo.e.b -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton .cazzo.e.b.r -variable err -value 2 -text \"Return to LSD Browser to edit the model configuration\"" );
	cmd( "ttk::radiobutton .cazzo.e.b.d -variable err -value 3 -text \"Open LSD Debugger on the offending variable and object instance\"" );
	cmd( "ttk::radiobutton .cazzo.e.b.e -variable err -value 1 -text \"Quit LSD Browser to edit the model equations' code in LMM\"" );
	cmd( "pack .cazzo.e.b.r .cazzo.e.b.d .cazzo.e.b.e -anchor w" );

	cmd( "pack .cazzo.e.l .cazzo.e.b" );

	cmd( "pack .cazzo.t .cazzo.e -padx 5 -pady 5" );

	cmd( "okhelp .cazzo b { set choice 1 }  { LsdHelp debug.html#crash }" );

	cmd( "showtop .cazzo centerW" );
	cmd( "mousewarpto .cazzo.b.ok" );

	if ( parallel_mode || fast_mode != 0 )
		cmd( ".cazzo.e.b.d configure -state disabled" );

	if ( worker_errors( ) > 0 )
		cmd( ".cazzo.e.b.r configure -state disabled" );

	choice = 0;
	while ( choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "destroytop .cazzo" );

	int err = get_int( "err" );

	if ( err == 3 )
	{
		if ( ! parallel_mode && fast_mode == 0 && stack_log != NULL &&
			 stack_log->vs != NULL && stack_log->vs->label != NULL )
		{
			char err_msg[ MAX_LINE_SIZE ];
			double useless = -1;
			snprintf( err_msg, MAX_LINE_SIZE, "%s (ERROR)", stack_log->vs->label );
			deb( stack_log->vs->up, NULL, err_msg, & useless );
		}

		err = 2;
	}

	if ( err == 2 )
	{
		// do run( ) cleanup
		empty_stack( );
		unsavedData = true;				// flag unsaved simulation results
		running = false;

		// run user closing function, reporting error appropriately
		user_exception = true;
		close_sim( );
		user_exception = false;

		reset_end( root );
		uncover_browser( );

#ifndef _NP_
		// stop multi-thread workers
		delete [ ] workers;
		workers = NULL;
#endif
		throw ( int ) 919293;			// force end of run() (in lsdmain.cpp)
	}

	if ( err == 1 )
	{
		save_pos( currObj );			// save browser position in structure
		update_model_info( );			// save windows positions if appropriate
	}

#else

	fprintf( stderr, "\nError: %s\n(%s)\n", boxTitle, logText );
#endif

	myexit( 13 );
}


/***************************************************
SET_LAB_TIT
Ensure that all objects on top of the variables have the counter updated,
and then writes the lab_tit field.
lab_tit indicates the position of the object containing the variables in the model.
***************************************************/
void set_lab_tit( variable *var )
{
	bool first = true;
	char app[ MAX_LINE_SIZE ], app1[ 2 * MAX_LINE_SIZE ];
	object *cur;

	if ( var->up->up == NULL )
	{
		// this is the Root of the model
		if ( var->lab_tit != NULL )
			return; 					// already done in the past

		var->lab_tit = new char[ strlen( "R" ) + 1 ];
		strcpy( var->lab_tit, "R" );

		return;
	}

	for ( cur = var->up; cur->up != NULL; cur = cur->up )
	{
		// find the bridge containing the variable
		set_tit_counter( cur );
		if ( ! first )
			snprintf( app1, 2 * MAX_LINE_SIZE, "%d_%s", cur->acounter, app );
		else
		{
			first = false;
			snprintf( app1, 2 * MAX_LINE_SIZE, "%d", cur->acounter );
		}

		strcpyn( app, app1, MAX_LINE_SIZE );
	}

	if ( var->lab_tit != NULL )
		delete [ ] var->lab_tit;

	var->lab_tit = new char[ strlen( app ) + 1 ];
	strcpy( var->lab_tit, app );
}


/***************************************************
SET_TIT_COUNTER
***************************************************/
void set_tit_counter( object *o )
{
	int i;
	bridge *cb;
	object *cur;

	if ( o->up == NULL )
		return;

	set_tit_counter( o->up );

	// find the bridge which contains the object
	cb = o->up->search_bridge( o->label );

	if ( cb->counter_updated )
		return;

	for ( cur = cb->head, i = 1; cur != NULL; cur = cur->next, ++i )
		if ( cur->lstCntUpd < t )		// don't update more than once per period
		{								// to avoid deletions to change counters
			cur->acounter = i;
			cur->lstCntUpd = t;
		}

	cb->counter_updated = true;
}


/*****************************************************************************
SET_BLUEPRINT
copy the naked structure of the model into another object, called blueprint,
to be used for adding objects without example
******************************************************************************/
void set_blueprint( object *container, object *r )
{
	bridge *cb, *cb1;
	object *cur, *cur1;
	variable *cv;

	if ( r == NULL )
		return;

	for ( cv = r->v; cv != NULL; cv = cv->next )
		container->add_var_from_example( cv );

	delete [ ] container->label;

	container->label = new char[ strlen( r->label ) + 1 ];
	strcpy( container->label, r->label );

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;

		cur1 = cb->head;
		container->add_obj( cur1->label );

		for ( cb1 = container->b; strcmp( cb1->blabel, cb->blabel ); cb1 = cb1->next );

		cur = cb1->head;
		set_blueprint( cur, cur1 );
	}
}


/*****************************************************************************
EMPTY_BLUEPRINT
remove the current blueprint
******************************************************************************/
void empty_blueprint( void )
{
	if ( blueprint == NULL )
		return;

	blueprint->empty( );
	blueprint->delete_obj( );
	blueprint = NULL;
}


/*****************************************************************************
RESET_BLUEPRINT
reset the current blueprint
******************************************************************************/
void reset_blueprint( object *r )
{
	empty_blueprint( );
	blueprint = new object;
	blueprint->init( NULL, "Root" );
	set_blueprint( blueprint, r );
}
