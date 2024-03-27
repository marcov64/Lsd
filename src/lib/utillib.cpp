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

 - void plog( const char *m, ... );
 print  message string m in the Log screen or the console.

 - void error_hard( const char *boxTitle, const char *boxText,
 				   bool defQuit, const char *logFmt, ... );
 print error messages to the log screen, console and error
 file, recovering LSD configuration to allow for non-crashing
 recovery.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 PLOG
 Print message on the log window,
 if GUI is available, or console
 *************************************************************/
void lsd::simulation::plog( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );

	if ( liblnk != NULL )
		liblnk->plog_backend( cm, "", argptr );
	else
		plog_terminal( cm, argptr );

	va_end( argptr );
}


/*************************************************************
 PLOG_TAG
 The optional tag parameter has to
 correspond to the log window
 existing tags, if GUI is available,
 or console
 *************************************************************/
void lsd::simulation::plog_tag( const char *cm, const char *tag, ... )
{
	static va_list argptr;

	va_start( argptr, tag );

	if ( liblnk != NULL )
		liblnk->plog_backend( cm, tag, argptr );
	else
		plog_terminal( cm, argptr );

	va_end( argptr );
}


/*************************************************************
 PLOG_TERMINAL
 Back-end to plog and plog_tag on
 console
 *************************************************************/
void lsd::simulation::plog_terminal( const char *cm, va_list arg )
{
	static bool bufdyn;
	static char *buffer, *message, bufstat[ MAX_BUFF_SIZE ], msgstat[ MAX_BUFF_SIZE ];
	static int i, j, reqsz, sz;
	static va_list argcpy;

	buffer = bufstat;
	message = msgstat;
	va_copy( argcpy, arg );

	reqsz = vsnprintf( buffer, MAX_BUFF_SIZE, cm, arg );

#ifndef _NP_
	l_guardT lock( plog_term_lck );
#endif

	if ( reqsz < 0 )
	{
		fprintf( stderr_ptr, "\nCannot expand message '%s...'\n", cm );
		return;
	}

	// handle very large messages
	if ( reqsz >= MAX_BUFF_SIZE )
	{
		buffer = new char[ reqsz + 1 ];
		sz = vsnprintf( buffer, reqsz + 1, cm, argcpy );

		if ( reqsz < 0 || sz > reqsz )
		{
			fprintf( stderr_ptr, "\nCannot expand message '%s...'\n", cm );
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

	fprintf( stdout_ptr, "%s", message );
	fflush( stdout_ptr );

	if ( sims.size( ) > 0 )
		sims[ 0 ]->message_logged = true;

	if ( bufdyn )
	{
		delete [ ] buffer;
		delete [ ] message;
	}
}


/*************************************************************
 ERROR_HARD
 Procedure called when an unrecoverable error occurs.
 Information about the state of the simulation when the error
 occurred is provided. Users can abort the program or analyze
 the results collected up the latest time step available.
 *************************************************************/
void lsd::simulation::error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... )
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
	l_guardT lock( error_lck );

	// abort worker and park message if not running in main LSD thread
	if ( std::this_thread::get_id( ) != main_thread )
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

	quit = 2;				// do not continue simulation

	if ( liblnk != NULL && liblnk->error_hard_helper != NULL )
		liblnk->error_hard_helper( boxTitle, boxText, logText, defQuit );
	else
		fprintf( stderr, "\nError: %s\n(%s)\n", boxTitle, logText );

	lsd_exit( 13 );
}


/*************************************************************
 SET_LAB_TIT
 Ensure that all objects on top of the variables
 have the counter updated, and then writes the
 lab_tit field.
 lab_tit indicates the position of the object
 containing the variables in the model.
 *************************************************************/
void lsd::variable::set_lab_tit( void )
{
	bool first = true;
	char app[ MAX_LINE_SIZE ], app1[ 2 * MAX_LINE_SIZE ];
	object *cur;

	if ( up->up == NULL )
	{
		// this is the Root of the model
		if ( lab_tit != NULL )
			return;						// already done in the past

		lab_tit = new char[ strlen( "R" ) + 1 ];
		strcpy( lab_tit, "R" );

		return;
	}

	for ( cur = up; cur->up != NULL; cur = cur->up )
	{
		// find the bridge containing the variable
		cur->set_tit_counter( );

		if ( ! first )
			snprintf( app1, 2 * MAX_LINE_SIZE, "%d_%s", cur->acounter, app );
		else
		{
			first = false;
			snprintf( app1, 2 * MAX_LINE_SIZE, "%d", cur->acounter );
		}

		strcpyn( app, app1, MAX_LINE_SIZE );
	}

	if ( lab_tit != NULL )
		delete [ ] lab_tit;

	lab_tit = new char[ strlen( app ) + 1 ];
	strcpy( lab_tit, app );
}


/*************************************************************
 SET_TIT_COUNTER
 *************************************************************/
void lsd::object::set_tit_counter( void )
{
	int i;
	bridge *cb;
	object *cur;

	if ( up == NULL )
		return;

	up->set_tit_counter( );

	// find the bridge which contains the object
	cb = up->search_bridge( label );

	if ( cb->counter_updated )
		return;

	for ( cur = cb->head, i = 1; cur != NULL; cur = cur->next, ++i )
		if ( cur->lstCntUpd < sim->t )	// don't update more than once per period
		{								// to avoid deletions to change counters
			cur->acounter = i;
			cur->lstCntUpd = sim->t;
		}

	cb->counter_updated = true;
}


/*************************************************************
 SET_BLUEPRINT
 copy the naked structure of the model into another object,
 called blueprint, to be used for adding objects without
 example
 *************************************************************/
void lsd::object::set_blueprint( object *container )
{
	bridge *cb, *cb1;
	object *cur, *cur1;
	variable *cv;

	for ( cv = v; cv != NULL; cv = cv->next )
		container->add_var_from_example( cv );

	delete [ ] container->label;

	container->label = new char[ strlen( label ) + 1 ];
	strcpy( container->label, label );

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;

		cur1 = cb->head;
		container->add_obj( cur1->label );

		for ( cb1 = container->b; strcmp( cb1->blabel, cb->blabel ); cb1 = cb1->next );

		cur = cb1->head;
		cur1->set_blueprint( cur );
	}
}


/*************************************************************
 EMPTY_BLUEPRINT
 remove the current blueprint
 *************************************************************/
void lsd::simulation::empty_blueprint( void )
{
	if ( blueprint == NULL )
		return;

	blueprint->empty( );
	blueprint->delete_obj( );
	blueprint = NULL;
}


/*************************************************************
 RESET_BLUEPRINT
 reset the current blueprint
 *************************************************************/
void lsd::simulation::reset_blueprint( object *r )
{
	empty_blueprint( );
	blueprint = new object;
	blueprint->init( NULL, this, "Root" );

	if ( r != NULL )
		r->set_blueprint( blueprint );
}


/*************************************************************
 SEARCH_PARALLEL
 *************************************************************/
bool lsd::object::search_parallel( void )
{
	bridge *cb;
	variable *cv;

	// search among the variables
	for ( cv = v; cv != NULL; cv=cv->next )
		if ( cv->parallel )
			return true;

	// search among descendants
	for ( cb = b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			if ( cb->head->search_parallel( ) )
				return true;

	return false;
}
