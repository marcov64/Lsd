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
UTIL.CPP
Contains a set of utilities for different parts of the
program. The basic set of utilities used in DLL or no-window
executables are stored in UTILLIB.CPP.

The main functions contained in this file are:

- void plog_backend( const char *cm, ... );
print  message string m in the Log screen.

- void cmd( const char *cc, ... );
Standard routine to send the message string cc to the TCL
interpreter in order to execute a command for the graphical
interfaces.
*************************************************************/

#include "decl.h"


/*********************************
PLOG_BACKEND
Back-end to plog and plog_tag on
log window
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

	if ( ! tk_ok || ! log_ok )
		return;

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
		log_tcl_error( true, "Invalid text message", "Cannot expand message '%s...'", cm );
		return;
	}

	// handle very large messages
	if ( reqsz >= MAX_BUFF_SIZE )
	{
		buffer = new char[ reqsz + 1 ];
		sz = vsnprintf( buffer, reqsz + 1, cm, argcpy );

		if ( reqsz < 0 || sz > reqsz )
		{
			log_tcl_error( true, "Invalid text message", "Cannot expand message '%s...'", cm );
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

	message_logged = true;

	if ( bufdyn )
	{
		delete [ ] buffer;
		delete [ ] message;
	}
}


/****************************
PRINT_STACK
Print the state of the stack in the log window.
This tells the user which variable is computed
because of other equations' request.
*****************************/
void print_stack( void )
{
	lsdstack *app;

	if ( parallel_mode )
	{
		plog( "\n\nRunning in parallel mode, list of variables under computation not available\n(You may disable parallel computation in menu 'Run', 'Simulation Settings')\n" );
		return;
	}

	if ( fast_mode > 0 )
	{
		plog( "\n\nRunning in fast mode, list of variables under computation not available\n(You may temporarily not use fast mode to get additional information)\n" );
		return;
	}

	plog( "\n\nList of variables currently under computation" );
	plog( "\n\nLevel\tVariable Label" );

	for ( app = stack_log; app != NULL; app = app->prev )
		plog( "\n%d\t%s", app->ns, app->label );

	plog( "\n\n(the zero-level variable is computed by the simulation manager, \nwhile possible other variables are triggered by the lower level ones\nbecause necessary for completing their computation)\n" );
}


/*************************************************************
ERROR_HARD_HELPER
Helper function to handle unrecoverable errors at the GUI.
Users can abort the program or analyze the results collected
up the latest time step available.
*************************************************************/
void error_hard_helper( const char *boxTitle, const char *boxText, const char *logText, bool defQuit )
{
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

	if ( ! running )
		return;

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
}


/***************************************************
FMT_TTIP_DESCR
***************************************************/
char *fmt_ttip_descr( char *out, description *d, int outSz, bool init )
{
	char out1[ outSz ];

	if ( out == NULL || outSz <= 0 )
		return NULL;

	if ( has_descr_text ( d ) )
		strcln( out, d->text, outSz );
	else
		out[ 0 ] = '\0';

	if ( init && d != NULL && d->init != NULL && strlen( d->init ) > 0 )
	{
		if ( strlen( out ) > 0 )
			strcatn( out, "\n\u2500\u2500\u2500\n", outSz );

		strcln( out1, d->init, outSz );
		strcatn( out, out1, outSz );
	}

	if ( strlen( out ) > 0 )
	{
		strwrap( out1, out, outSz - 1, 60 );
		strtcl( out, out1, outSz - 1 );
	}

	return out;
}


/***************************************************
SET_TTIP_DESCR
***************************************************/
void set_ttip_descr( const char *w, const char *lab, int it, bool init )
{
	char desc[ MAX_LINE_SIZE + 1 ];
	description *cd;

	// add tooltip only if element has description
	cd = search_description( lab, false );
	if ( cd != NULL && strlen( fmt_ttip_descr( desc, cd, MAX_LINE_SIZE + 1, init ) ) > 0 )
	{
		if ( it >= 0 )			// listbox/canvas?
			cmd( "tooltip::tooltip %s -item %d \"%s\"", w, it, desc );
		else
			cmd( "tooltip::tooltip %s \"%s\"", w, desc );
	}
}


/***************************************************
TCL_SET_TTIP_DESCR
***************************************************/
int Tcl_set_ttip_descr( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	int it, init;

	if ( argc < 3 || argc > 5 )		// require 4 parameters: widget name, variable name text, item number (opt) and init text flag (opt)
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || argv[ 2 ] == NULL ||
		 ! strcmp( argv[ 1 ], "" ) || ! strcmp( argv[ 2 ], "" ) )
		return TCL_ERROR;

	if ( argc < 4 || argv[ 3 ] == NULL || sscanf( argv[ 3 ], "%d", & it ) == 0 )
		it = -1;

	if ( argc < 5 || argv[ 4 ] == NULL || sscanf( argv[ 4 ], "%d", & init ) == 0 || init < 0 || init > 1 )
		init = 1;

	set_ttip_descr( argv[ 1 ], argv[ 2 ], it, init ? true : false );

	return TCL_OK;
}


/****************************************************
SEARCH_ALL_SOURCES
****************************************************/
FILE *search_all_sources( char *str )
{
	char got[ MAX_LINE_SIZE ];
	const char *fname;
	int i, j, nfiles;
	FILE *f;

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", exec_path );
	cmd( "if { [ lsearch -exact $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", equation_name, equation_name );
	cmd( "set res [ llength $source_files ]" );
	nfiles = get_int( "res" );

	for ( i = 0; i < nfiles; ++i )
	{
		cmd( "set brr [ lindex $source_files %d ]", i );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = get_str( "brr" );
		if ( ( f = fopen( fname, "r" ) ) == NULL )
			continue;

		fgets( got, MAX_LINE_SIZE, f );
		clean_spaces( got );
		for ( j = 0; strncmp( got, str, strlen( str ) ) && j < MAX_FILE_TRY; ++j )
		{
			if ( fgets( got, MAX_LINE_SIZE, f ) == NULL )
				break;
			clean_spaces( got );
		}

		if ( ! strncmp( got, str, strlen( str ) ) )
			return f;

		fclose( f );
	}

	return NULL;
}


/***************************************************
RETURN_WHERE_USED
***************************************************/
void return_where_used( char *lab, char *s, int sz )
{
	const char *app;

	scan_used_lab( lab, "" );	// make scan without window
	app = get_str( "list_used" );

	if ( app != NULL )
		strcpyn( s, app, sz );
	else
		strcpy( s, "" );
}


/***************************************************
GET_VAR_DESCR
***************************************************/
void get_var_descr( const char *lab, char *desc, int descr_len )
{
	char str[ 2 * MAX_ELEM_LENGTH ], str1[ MAX_LINE_SIZE ], str2[ descr_len ];
	int i, j = 0, done = -1;
	FILE *f;

	snprintf( str, 2 * MAX_ELEM_LENGTH, "EQUATION(\"%s\")", lab );
	f = search_all_sources( str );

	if ( f == NULL )
	{
		snprintf( str, 2 * MAX_ELEM_LENGTH, "EQUATION_DUMMY(\"%s\",", lab );
		f = search_all_sources( str );
	}

	if ( f == NULL )
	{
		snprintf( str, 2 * MAX_ELEM_LENGTH, "FUNCTION(\"%s\")", lab );
		f = search_all_sources( str );
	}

	if ( f == NULL )
	{
		snprintf( str, 2 * MAX_ELEM_LENGTH, "if (!strcmp(label,\"%s\"))", lab );
		f = search_all_sources( str );
	}

	if ( f != NULL )
	{
		while ( done != 1 )
		{
			fgets( str1, MAX_LINE_SIZE, f );

			for ( i = 0; str1[ i ] != '\0' && done != 1; ++i )
			{
				if ( done == -1 ) 		// no comment found yet
				{
					if ( isalpha( str1[ i ]) != 0 ) 	// no comment exists
						done = 1;

					if ( str1[ i ] == '/' && str1[ i + 1 ] == '*' )
					{
						done = 0; 		// beginning of a multiline comment
						i += 2;

						// discard initial empty line
						while ( str1[ i ] == '\r' && str1[ i + 1 ] == '\n' )
							i += 2;
						while ( str1[ i ] == '\n' )
							++i;
						if ( str1[ i ] == '\0' )
							break;
					}

					if ( str1[ i ] == '/' && str1[ i + 1 ] == '/' )
					{
						done = 2; 		// beginning of a single line comment
						i += 2;
					}
				}

				if ( done == 0 ) 		// we are in a comment
					if ( str1[ i ] == '*' && str1[ i + 1 ] == '/' )
						done = 1;

				if ( done == 0 || done == 2 )
					if ( str1[ i ] != '\r' )
						str2[ j++ ] = str1[ i ];

				if ( done == 2 && str1[ i ] == '\n' )
					done = -1;

				if ( j >= descr_len - 2 )
					done = 1;
			}
		}

		fclose( f );
	}

	str2[ j ] = '\0';
	strcln( desc, str2, descr_len );
}


/***************************************************
AUTO_DOCUMENT
***************************************************/
void auto_document( const char *lab, const char *which, bool append )
{
	bool var;
	char str1[ MAX_LINE_SIZE ], app[ 10 * MAX_LINE_SIZE ], text[ 2 * MAX_BUFF_SIZE ];
	description *cd;

	for ( cd = descr; cd != NULL; cd = cd->next )
	{
		app[ 0 ] = '\0';
		if ( ( lab == NULL && ( ! strcmp( which, "ALL" ) || ! strcmp( cd->type, "Variable" ) || ! strcmp( cd->type, "Function" ) ) ) || ( lab != NULL && ! strcmp( lab, cd->label ) ) )
		{	// for each description
			if ( ( ! strcmp( cd->type, "Variable") ) == 1 || ( ! strcmp( cd->type, "Function" ) ) == 1 )
			{ 	// if it is a Variable
				var = true;
				get_var_descr( cd->label, app, 10 * MAX_LINE_SIZE );
			}
			else
				var = false;

			return_where_used( cd->label, str1, MAX_LINE_SIZE );
			if ( ( append || ! var ) && has_descr_text ( cd ) )
				if ( strwsp( cd->text ) )
					snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n'%s' appears in the equation for: %s", app, cd->label, str1 );
				else
					snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n%s\n'%s' appears in the equation for: %s", cd->text, app, cd->label, str1 );
			else
				snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n'%s' appears in the equation for: %s", app, cd->label, str1 );

			delete [ ] cd->text;
			cd->text = new char[ strlen( text ) + 1 ];
			strcpy( cd->text, text );
		} 					// end of the label to document
	}						// end of the for (desc)
}


/****************************************************
COUNT_SAVE
****************************************************/
void count_save( object *n, int *count )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		if ( cv->save == 1 || cv->savei == 1 )
			( *count )++;

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		count_save( co, count );
	}
}
