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
 program. The basic set of utilities used in DLL or terminal
 executables are stored in UTILLIB.CPP.

 The main functions contained in this file are:

 - void plog_backend( const char *cm, ... );
 print  message string m in the Log screen.
 *************************************************************/

#include "LSD.h"

#define NUM_TAGS 7

namespace gui
{
	const char *tags[ NUM_TAGS ] = { "", "highlight", "table", "series", "prof1", "prof2", "bar" };
	mtxT lock_plog_backend;		// lock lock_plog_backend for parallel access
}


/*************************************************************
 PLOG
 Print message on the log window,
 if GUI is available, or console
 *************************************************************/
void gui::plog( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );
	plog_backend( cm, "", argptr );
	va_end( argptr );
}


/*************************************************************
 PLOG_TAG
 The optional tag parameter has to
 correspond to the log window
 existing tags, if GUI is available,
 or console
 *************************************************************/
void gui::plog_tag( const char *cm, const char *tag, ... )
{
	static va_list argptr;

	va_start( argptr, tag );
	plog_backend( cm, tag, argptr );
	va_end( argptr );
}


/*************************************************************
 PLOG_BACKEND
 Back-end to plog and plog_tag on
 log window
 *************************************************************/
void gui::plog_backend( const char *cm, const char *tag, va_list arg )
{
	static bool bufdyn;
	static char *buffer, *message, bufstat[ MAX_BUFF_SIZE ], msgstat[ MAX_BUFF_SIZE ];
	static bool tag_ok;
	static int i, j, reqsz, sz;
	static va_list argcpy;

	if ( ! tk_ok || ! log_ok )
		return;

	buffer = bufstat;
	message = msgstat;
	va_copy( argcpy, arg );
	reqsz = vsnprintf( buffer, MAX_BUFF_SIZE, cm, arg );

	l_guardT lock( lock_plog_backend );

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
		sim.on_bar = false;

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

	sim.message_logged = true;

	if ( bufdyn )
	{
		delete [ ] buffer;
		delete [ ] message;
	}
}


/*************************************************************
 PRINT_STACK
 Print the state of the stack in the log window.
 This tells the user which variable is computed
 because of other equations' request.
 *************************************************************/
void gui::print_stack( void )
{
	lsd::lsdstack *app;

	if ( sim.parallel_mode )
	{
		plog( "\n\nRunning in parallel mode, list of variables under computation not available\n(You may disable parallel computation in menu 'Run', 'Simulation Settings')\n" );
		return;
	}

	if ( sim.fast_mode > 0 )
	{
		plog( "\n\nRunning in fast mode, list of variables under computation not available\n(You may temporarily not use fast mode to get additional information)\n" );
		return;
	}

	plog( "\n\nList of variables currently under computation" );
	plog( "\n\nLevel\tVariable Label" );

	for ( app = sim.stack_log; app != NULL; app = app->prev )
		plog( "\n%d\t%s", app->n, app->label );

	plog( "\n\n(the zero-level variable is computed by the simulation manager, \nwhile possible other variables are triggered by the lower level ones\nbecause necessary for completing their computation)\n" );
}


/*************************************************************
 ERROR_HARD_HELPER
 Helper function to handle unrecoverable errors at the GUI.
 Users can abort the program or analyze the results collected
 up the latest time step available.
 *************************************************************/
void gui::error_hard_helper( const char *boxTitle, const char *boxText, const char *logText, bool defQuit )
{
	if ( sim.running )			// handle running events differently
	{
		cmd( "if [ winfo exists .deb ] { destroytop .deb }" );
		deb_log( false, 0 );// close any open debug log file
		reset_plot( );		// show & disable run-time plot
		set_buttons_run( false );

		plog_tag( "\n\nError detected at time step: %d", "highlight", sim.t );
		plog( "\n\nError: %s\nDetails: %s", boxTitle, logText );
		if ( ! sim.parallel_mode && sim.stack_log != NULL && sim.stack_log->v != NULL )
			plog( "\nOffending code contained in the equation for variable: '%s'", sim.stack_log->v->attr->label );
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

	if ( ! sim.running )
		return;

	uncover_browser( );
	cmd( "focustop .log" );

	cmd( "set err %d", ( defQuit || sim.worker_errors( ) ) > 0 ? 1 : 2 );

	cmd( "newtop .cazzo Error" );

	cmd( "ttk::frame .cazzo.t" );
	cmd( "ttk::label .cazzo.t.l -style hl.TLabel -text \"An error occurred during the simulation\"" );
	cmd( "pack .cazzo.t.l -pady $_10" );
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

	cmd( "pack .cazzo.t .cazzo.e -padx $_5 -pady $_5" );

	cmd( "okhelp .cazzo b { set choice 1 }  { LsdHelp debug.html#crash }" );

	cmd( "showtop .cazzo centerW" );
	cmd( "mousewarpto .cazzo.b.ok" );

	if ( sim.parallel_mode || sim.fast_mode != 0 )
		cmd( ".cazzo.e.b.d configure -state disabled" );

	if ( sim.worker_errors( ) > 0 )
		cmd( ".cazzo.e.b.r configure -state disabled" );

	choice = 0;
	while ( choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "destroytop .cazzo" );

	int err = get_int( "err" );

	if ( err == 3 )
	{
		if ( ! sim.parallel_mode && sim.fast_mode == 0 && sim.stack_log != NULL &&
			 sim.stack_log->v != NULL && sim.stack_log->v->attr != NULL && sim.stack_log->v->attr->label != NULL )
		{
			char err_msg[ MAX_LINE_SIZE ];
			double useless = -1;
			snprintf( err_msg, MAX_LINE_SIZE, "%s (ERROR)", sim.stack_log->v->attr->label );
			sim.stack_log->v->up->debugger( NULL, err_msg, & useless );
		}

		err = 2;
	}

	if ( err == 2 )
	{
		// do run( ) cleanup
		sim.empty_stack( );
		sim.running = sim.running_seq = false;
		unsavedData = true;				// flag unsaved simulation results

		// run user closing function, reporting error appropriately
		sim.user_exception = true;
		sim._close_sim_( );
		::close_sim( );
		sim.user_exception = false;

		sim.root->reset_end( );
		uncover_browser( );

		// stop multi-thread workers
		delete [ ] sim.workers;
		sim.workers = NULL;

		throw ( int ) 919293;			// force end of run() (in lsdmain.cpp)
	}

	if ( err == 1 )
	{
		if ( curr_obj != NULL )
			curr_obj->save_pos( );		// save browser position in structure

		update_model_options( );		// save windows positions if appropriate
	}
}


/*************************************************************
 TCL_SET_C_VAR
 Function to set a c variable when
 not in a Tcl idle loop (hardcoded
 vars only)
 *************************************************************/
int gui::Tcl_set_c_var( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ];
	int value;

	if ( argc != 3 )					// require 2 parameters: variable name and value
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || argv[ 2 ] == NULL )
		return TCL_ERROR;

	if ( ! sscanf( argv[ 1 ], "%99s", vname ) )	// remove unwanted spaces
		return TCL_ERROR;

	// set the appropriate variable (hardcoded in an else-if chain)
	if ( ! strcmp( vname, "done_in" ) )
	{
		if ( ! sscanf( argv[ 2 ], "%d", &value ) )	// transform to integer
			return TCL_ERROR;

		done_in = value;
	}
	else
		return TCL_ERROR;

	return TCL_OK;
}


/*************************************************************
 FMT_TTIP_DESCR
 *************************************************************/
char *gui::fmt_ttip_descr( char *out, lsd::descr & d, int outSz, bool init )
{
	char out1[ outSz ];

	if ( out == NULL || outSz <= 0 )
		return NULL;

	if ( d.has_descr_text ( ) )
		lsd::strcln( out, d.text, outSz );
	else
		out[ 0 ] = '\0';

	if ( init && d.init != NULL && strlen( d.init ) > 0 )
	{
		if ( strlen( out ) > 0 )
			lsd::strcatn( out, "\n\u2500\u2500\u2500\n", outSz );

		lsd::strcln( out1, d.init, outSz );
		lsd::strcatn( out, out1, outSz );
	}

	if ( strlen( out ) > 0 )
	{
		lsd::strwrap( out1, out, outSz - 1, 60 );
		strtcl( out, out1, outSz - 1 );
	}

	return out;
}


/*************************************************************
 SET_TTIP_DESCR
 *************************************************************/
void gui::set_ttip_descr( const char *w, const char *lab, int it, bool init )
{
	char dsc[ MAX_LINE_SIZE + 1 ];

	// add tooltip only if element has description
	auto cd = desc.search_descr( lab );
	if ( cd != NULL && strlen( fmt_ttip_descr( dsc, *cd, MAX_LINE_SIZE + 1, init ) ) > 0 )
	{
		if ( it >= 0 )			// listbox/canvas?
			cmd( "tooltip::tooltip %s -item %d \"%s\"", w, it, dsc );
		else
			cmd( "tooltip::tooltip %s \"%s\"", w, dsc );
	}
}


/*************************************************************
 TCL_SET_TTIP_DESCR
 *************************************************************/
int gui::Tcl_set_ttip_descr( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
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


/*************************************************************
 AUTO_DOCUMENT
 *************************************************************/
void gui::auto_document( const char *lab, const char *which, bool append )
{
	bool var;
	char str1[ MAX_LINE_SIZE ], app[ 10 * MAX_LINE_SIZE ], text[ 2 * MAX_BUFF_SIZE ];

	for ( auto & cd : desc.elem )
	{
		app[ 0 ] = '\0';
		if ( ( lab == NULL && ( ! strcmp( which, "ALL" ) || ! strcmp( cd.type, "Variable" ) || ! strcmp( cd.type, "Function" ) ) ) || ( lab != NULL && ! strcmp( lab, cd.label ) ) )
		{	// for each description
			if ( ( ! strcmp( cd.type, "Variable") ) == 1 || ( ! strcmp( cd.type, "Function" ) ) == 1 )
			{ 	// if it is a Variable
				var = true;
				get_var_descr( cd.label, app, 10 * MAX_LINE_SIZE );
			}
			else
				var = false;

			return_where_used( cd.label, str1, MAX_LINE_SIZE );
			if ( ( append || ! var ) && cd.has_descr_text ( ) )
				if ( lsd::strwsp( cd.text ) )
					snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n'%s' appears in the equation for: %s", app, cd.label, str1 );
				else
					snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n%s\n'%s' appears in the equation for: %s", cd.text, app, cd.label, str1 );
			else
				snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n'%s' appears in the equation for: %s", app, cd.label, str1 );

			delete [ ] cd.text;
			cd.text = new char[ strlen( text ) + 1 ];
			strcpy( cd.text, text );
		} 					// end of the label to document
	}						// end of the for (desc)
}


/*************************************************************
 RETURN_WHERE_USED
 *************************************************************/
void gui::return_where_used( char *lab, char *s, int sz )
{
	const char *app;

	scan_used_lab( lab, "" );	// make scan without window
	app = get_str( "list_used" );

	if ( app != NULL )
		lsd::strcpyn( s, app, sz );
	else
		strcpy( s, "" );
}


/*************************************************************
 GET_VAR_DESCR
 *************************************************************/
void gui::get_var_descr( const char *lab, char *desc, int descr_len )
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
						done = 0; 		// beginning of a multi-line comment
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
						done = 2; 		// beginning of a single-line comment
						i += 2;

						while( str1[ i ] == '/' )
							++i;		// skip extra slashes
					}
				}

				if ( done == 0 ) 		// we are in a multi-line comment
					if ( str1[ i ] == '*' && str1[ i + 1 ] == '/' )
						done = 1;

				if ( done == 2 )		// we are in a single-line comment
					if ( str1[ i ] == '\n' )
						done = 1;

				if ( done == 0 || done == 2 )
					if ( str1[ i ] != '\r' )
						str2[ j++ ] = str1[ i ];

				if ( j >= descr_len - 2 )
					done = 1;
			}
		}

		fclose( f );
	}

	str2[ j ] = '\0';
	lsd::strcln( desc, str2, descr_len );
}


/*************************************************************
 SEARCH_ALL_SOURCES
 *************************************************************/
FILE *gui::search_all_sources( char *str )
{
	char got[ MAX_LINE_SIZE ];
	const char *fname;
	int i, j, nfiles;
	FILE *f;

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", lsd::model_path );
	cmd( "set res [ llength $source_files ]" );
	nfiles = get_int( "res" );

	for ( i = 0; i < nfiles; ++i )
	{
		cmd( "set brr [ lindex $source_files %d ]", i );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", lsd::model_path, lsd::model_path );
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


/*************************************************************
 TCL_GET_VAR_DESCR
 Function to get variable description on
 equation file(s) from Tcl
 *************************************************************/
int gui::Tcl_get_var_descr( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
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

	Tcl_SetResult( interp, desc, TCL_VOLATILE );
	return TCL_OK;
}


/*************************************************************
 TCL_GET_VAR_CONF
 Function to get variable configuration from Tcl
 *************************************************************/
int gui::Tcl_get_var_conf( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ], res[ 2 ];
	lsd::variable *cv;

	if ( argc != 3 )					// require 2 parameters: variable name and property
		return TCL_ERROR;

	if ( curr_obj == NULL || argv[ 1 ] == NULL || argv[ 2 ] == NULL ||
		 ! strcmp( argv[ 1 ], "(none)" ) )
		return TCL_ERROR;

	sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
	cv = curr_obj->search_var( NULL, vname );

	if ( cv == NULL )					// variable not found
		return TCL_ERROR;

	// get the appropriate value for variable
	res[ 1 ] = '\0';					// default is 1 char string array
	if ( ! strcmp( argv[ 2 ], "save" ) )
		res[ 0 ] = cv->attr->save ? '1' : '0';
	else
		if ( ! strcmp( argv[ 2 ], "parallel" ) )
			res[ 0 ] = cv->attr->parallel ? '1' : '0';
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
							return TCL_ERROR;

	Tcl_SetResult( interp, res, TCL_VOLATILE );
	return TCL_OK;
}


/*************************************************************
 TCL_SET_VAR_CONF
 Function to set variable configuration from Tcl
 *************************************************************/
int gui::Tcl_set_var_conf( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ];
	lsd::object *cur;
	lsd::variable *cv;

	if ( argc != 4 )					// require 3 parameters: variable name, property and value
		return TCL_ERROR;

	if ( curr_obj == NULL || argv[ 1 ] == NULL || argv[ 2 ] == NULL ||
		 argv[ 3 ] == NULL || ! strcmp( argv[ 1 ], "(none)" ) )
		return TCL_ERROR;

	sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
	cv = curr_obj->search_var( NULL, vname );

	if ( cv == NULL )					// variable not found
		return TCL_ERROR;

	// set the appropriate value for variable (static attributes)
	if ( ! strcmp( argv[ 2 ], "save" ) )
		cv->attr->save = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;
	else
		if ( ! strcmp( argv[ 2 ], "savei" ) )
			cv->attr->savei = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;
		else
			if ( ! strcmp( argv[ 2 ], "parallel" ) )
				cv->attr->parallel  = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;
			else
				// set the appropriate value for variable (all instances)
				for ( cur = curr_obj; cur != NULL; cur = cur->hyper_next( cur->label ) )
				{
					cv = cur->search_var( NULL, vname );

					if ( ! strcmp( argv[ 2 ], "plot" ) )
						cv->plot = ( ! strcmp( argv[ 3 ], "1" ) ) ? true : false;
					else
						if ( ! strcmp( argv[ 2 ], "debug" ) )
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
						else
							if ( ! strcmp( argv[ 2 ], "watch" ) )
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
							else
								if ( ! strcmp( argv[ 2 ], "watch_write" ) )
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
								else
									return TCL_ERROR;
					}

	unsaved_change( true );				// signal unsaved change
	redrawReq = true;

	if ( ( ! strcmp( argv[ 2 ], "save" ) && cv->attr->save ) ||
		 ( ! strcmp( argv[ 2 ], "savei" ) && cv->attr->savei ) )
	{
		for ( cur = curr_obj; cur != NULL; cur = cur->up )
			if ( ! cur->to_compute )
			{
				cmd( "ttk::messageBox -parent . -type ok -title Warning -icon warning -message \"Cannot save element\" -detail \"Element '%s' set to be saved but it will not be computed for the Analysis of Results, since object '%s' is not set to be computed.\"", vname, cur->label );
				break;
			}
	}

	return TCL_OK;
}


/*************************************************************
 TCL_GET_OBJ_CONF
 Function to get object configuration from Tcl
 *************************************************************/
int gui::Tcl_get_obj_conf( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ], res[ 2 ];
	lsd::object *cur;

	if ( argc != 3 )					// require 2 parameters: variable name and property
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || argv[ 2 ] == NULL || ! strcmp( argv[ 1 ], "(none)" ) )
		return TCL_ERROR;

	sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
	cur = sim.root->search( vname );

	if ( cur == NULL )					// variable not found
		return TCL_ERROR;

	// get the appropriate value for variable
	res[ 1 ] = '\0';					// default is 1 char string array
	if ( ! strcmp( argv[ 2 ], "comp" ) )
		res[ 0 ] = cur->to_compute ? '1' : '0';
	else
		return TCL_ERROR;

	Tcl_SetResult( interp, res, TCL_VOLATILE );
	return TCL_OK;
}


/*************************************************************
 TCL_SET_OBJ_CONF
 Function to set object configuration from Tcl
 *************************************************************/
int gui::Tcl_set_obj_conf( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	char vname[ MAX_ELEM_LENGTH ];
	lsd::object *cur, *cur1;

	if ( argc != 4 )					// require 3 parameters: variable name, property and value
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || argv[ 2 ] == NULL ||
		 argv[ 3 ] == NULL || ! strcmp( argv[ 1 ], "(none)" ) )
		return TCL_ERROR;

	sscanf( argv[ 1 ], "%99s", vname );	// remove unwanted spaces
	cur = sim.root->search( vname );

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
				cur->control_to_compute( );
				check_save = false;		// do it just once
			}
		}
		else
			return TCL_ERROR;

	unsaved_change( true );				// signal unsaved change
	redrawReq = true;

	return TCL_OK;
}


/*************************************************************
 CHECK_TERM_EXEC
 Check if terminal executable/lib files are older than
 running executable file
 *************************************************************/
bool gui::check_term_exec( const char *term_exe )
{
	char exe[ MAX_PATH_LENGTH ], lib[ MAX_PATH_LENGTH ];
	struct stat stTermExe, stLib, stExe;

	if ( strlen( lsd::lib_path ) > 0 )
		snprintf( lib, MAX_PATH_LENGTH, "%s/%s", lsd::lib_path, lsd::lib_file );// full lib name
	else
		lsd::strcpyn( lib, lsd::lib_file, MAX_PATH_LENGTH );

	if ( strlen( lsd::exec_path ) > 0 )
		snprintf( exe, MAX_PATH_LENGTH, "%s/%s", lsd::exec_path, lsd::exec_file );// full exe name
	else
		lsd::strcpyn( exe, lsd::exec_file, MAX_PATH_LENGTH );

	// get OS info for files
	if ( stat( term_exe, &stTermExe ) == 0 && ( stat( lib, &stLib ) == 0 || ( stat( lib, &stExe ) == 0 ) ) )
		if ( ( stat( lib, &stLib ) == 0 && difftime( stTermExe.st_mtime, stLib.st_mtime ) < 0 ) ||
			 ( stat( lib, &stExe ) == 0 && difftime( stTermExe.st_mtime, stExe.st_mtime ) < 0 ) )
			return true;

	return false;
}


/*************************************************************
 CHECK_LABEL
 Control that the label lab does not already exist
 in the model. Also prevents invalid characters in
 the names.
 *************************************************************/
int lsd::object::check_label( const char *lab )
{
	object *cur;

	if ( ! valid_label( lab ) )
		return 2;				// invalid characters (incl. spaces)

	if ( ! strcmp( lab, label ) )
		return 1;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( ! strcmp( lab, cv->attr->label ) )
			return 1;

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;

		if ( cur->check_label( lab ) )
			return 1;
	}

	return 0;
}


/*************************************************************
 CONTROL_TO_COMPUTE
 *************************************************************/
void lsd::object::control_to_compute( void )
{
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
	{
		if ( ! gui::check_save )
			return;

		if ( cv->attr->save == 1 )
		{
			cmd( "set res [ ttk::messageBox -parent . -type okcancel -default ok -title Warning -icon warning -message \"Cannot save element\" -detail \"Element '%s' set to be saved but it will not be computed for the Analysis of Results, since object '%s' is not set to be computed.\n\nPress 'OK' to check for more disabled elements or 'Cancel' to proceed without further checking.\" ]", cv->attr->label, label );
			cmd( "if [ string equal $res cancel ] { set res 1 } { set res 0 }" );

			if ( gui::get_bool( "res" ) )
				gui::check_save = false;
		}
	}

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;

		cur->control_to_compute( );
	}
}


/*************************************************************
 COUNT_SAVE
 *************************************************************/
void lsd::object::count_save( int *count )
{
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->attr->save == 1 || cv->attr->savei == 1 )
			( *count )++;

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;
		cur->count_save( count );
	}
}


/*************************************************************
 SHOW_SAVE
 *************************************************************/
void lsd::object::show_save( void )
{
	char out[ 3 * MAX_ELEM_LENGTH ];
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
	{
		if ( cv->attr->save == 1 || cv->attr->savei == 1 )
		{
			if ( cv->param == 1 )
				snprintf( out, 3 * MAX_ELEM_LENGTH, "Object: %s \tParameter:\t", label );
			else
				snprintf( out, 3 * MAX_ELEM_LENGTH, "Object: %s \tVariable :\t", label );
			if ( cv->attr->savei == 1 )
			{
				if ( cv->attr->save == 1 )
				   strcatn( out, " (memory and disk)", 3 * MAX_ELEM_LENGTH );
				else
				   strcatn( out, " (disk only)", 3 * MAX_ELEM_LENGTH );
			}
			gui::plog( out );
			gui::plog_tag( "%s\n", "highlight", cv->attr->label );
			++gui::elem_count;
		}
	}

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;
		cur->show_save( );
	}
}


/*************************************************************
 CLEAN_SAVE
 *************************************************************/
void lsd::object::clean_save( void )
{
	for ( auto cv = v; cv != NULL; cv = cv->next )
		cv->attr->save = cv->attr->savei = false;

	for ( auto cb = b; cb != NULL; cb = cb->next )
		for ( auto cur = cb->head; cur != NULL; cur = cur->next )
			cur->clean_save( );
}


/*************************************************************
 SHOW_PLOT
 *************************************************************/
void lsd::object::show_plot( void )
{
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->plot )
		{
			if ( cv->param == 1 )
				gui::plog( "Object: %s \tParameter:\t", label );
			if ( cv->param == 0 )
				gui::plog( "Object: %s \tVariable :\t", label );
			if ( cv->param == 2 )
				gui::plog( "Object: %s \tFunction :\t", label );

			gui::plog_tag( "%s\n", "highlight", cv->attr->label );
			++gui::elem_count;
		}

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;

		cur->show_plot( );
	}
}


/*************************************************************
 CLEAN_PLOT
 *************************************************************/
void lsd::object::clean_plot( void )
{
	for ( auto cv = v; cv != NULL; cv = cv->next )
		cv->plot = false;

	for ( auto cb = b; cb != NULL; cb = cb->next )
		for ( auto cur = cb->head; cur != NULL; cur = cur->next )
			cur->clean_plot( );
}


/*************************************************************
 SHOW_DEBUG
 *************************************************************/
void lsd::object::show_debug( void )
{
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->deb_mode != 'n' )
		{
			if ( cv->param == 0 )
				gui::plog( "Object: %s \tVariable:\t", label );
			if ( cv->param == 1 )
				gui::plog( "Object: %s \tParameter:\t", label );
			if ( cv->param == 2 )
				gui::plog( "Object: %s \tFunction:\t", label );

			gui::plog_tag( "%s\t", "highlight", cv->attr->label );

			switch ( cv->deb_mode )
			{
				default:
				case 'd':
					gui::plog( "(debug)\n" );
					break;
				case 'w':
					gui::plog( "(watch)\n" );
					break;
				case 'D':
					gui::plog( "(debug and watch)\n" );
					break;
				case 'r':
					gui::plog( "(watch write)\n" );
					break;
				case 'R':
					gui::plog( "(debug and watch write)\n" );
			}

			++gui::elem_count;
		}

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;
		cur->show_debug( );
	}
}


/*************************************************************
 CLEAN_DEBUG
 *************************************************************/
void lsd::object::clean_debug( void )
{
	for ( auto cv = v; cv != NULL; cv = cv->next )
		cv->deb_mode = 'n';

	for ( auto cb = b; cb != NULL; cb = cb->next )
		for ( auto cur = cb->head; cur != NULL; cur = cur->next )
			cur->clean_debug( );
}


/*************************************************************
 SHOW_PARALLEL
 *************************************************************/
void lsd::object::show_parallel( void )
{
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->attr->parallel )
		{
			gui::plog( "Object: %s \tVariable:\t", label );
			gui::plog_tag( "%s\n", "highlight", cv->attr->label );
			++gui::elem_count;
		}

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;
		cur->show_parallel( );
	}
}


/*************************************************************
 CLEAN_PARALLEL
 *************************************************************/
void lsd::object::clean_parallel( void )
{
	for ( auto cv = v; cv != NULL; cv = cv->next )
		cv->attr->parallel = false;

	for ( auto cb = b; cb != NULL; cb = cb->next )
		for ( auto cur = cb->head; cur != NULL; cur = cur->next )
			cur->clean_parallel( );
}


/*************************************************************
 SHOW_OBSERVE
 *************************************************************/
void lsd::object::show_observe( void )
{
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
	{
		auto cd = desc != NULL ? desc->search_descr( cv->attr->label ) : NULL;
		if ( cd != NULL && cd->observe )
		{
			if ( cv->param == 1 )
				gui::plog( "Object: %s \tParameter:\t", label );
			else
				gui::plog( "Object: %s \tVariable :\t", label );

			gui::plog_tag( "%s (%lf)\n", "highlight", cv->attr->label, cv->val[ 0 ] );
			++gui::elem_count;
		}
	}

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;
		cur->show_observe( );
	}
}


/*************************************************************
 SHOW_INITIAL
 *************************************************************/
void lsd::object::show_initial( void )
{
	char buf_descr[ MAX_BUFF_SIZE ];

	for ( auto cv = v; cv != NULL; cv = cv->next )
	{
		auto cd = desc != NULL ? desc->search_descr( cv->attr->label ) : NULL;
		if ( cd != NULL && cd->initial )
		{
			if ( cv->param == 1 )
				gui::plog( "Object: %s \tParameter:\t", label );
			if ( cv->param == 0 )
				gui::plog( "Object: %s \tVariable :\t", label );
			if ( cv->param == 2 )
				gui::plog( "Object: %s \tFunction :\t", label );

			++gui::elem_count;
			gui::plog_tag( "%s \t", "highlight", cv->attr->label );

			if ( cd->init == NULL || strlen( cd->init ) == 0 )
			{
				for ( auto cur = this; cur != NULL; cur = cur->hyper_next( cur->label ) )
				{
					auto cv1 = cur->search_var( NULL, cv->attr->label );
					gui::plog( " %g", cv1->val[ 0 ] );
				}
			}
			else
				gui::plog( "%s", gui::strtcl( buf_descr, cd->init, MAX_BUFF_SIZE ) );

			gui::plog( "\n" );
		}
	}

	for ( auto cb = b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			cb->head->show_initial( );
}


/*************************************************************
 SHOW_SPECIAL_UPDAT
 *************************************************************/
void lsd::object::show_special_updat( void )
{
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->attr->delay > 0 || cv->attr->delay_range > 0 || cv->attr->period > 1 || cv->attr->period_range > 0 )
		{
			gui::plog( "Object: %s \tVariable:\t", label );
			gui::plog_tag( "%s\n", "highlight", cv->attr->label );
			++gui::elem_count;
		}

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->label );
		else
			cur = cb->head;
		cur->show_special_updat( );
	}
}
