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
 COMMON.CPP
 Code common between LMM and LSD Browser. The basic set of
 common code used in DLL or no-window executables are stored
 COMMONLIB.CPP.
 *************************************************************/

#include "LSD.h"

#ifndef _NP_
namespace gui
{
	std::mutex lock_log_tcl_err;	// lock log_tcl_error for parallel access
}
#endif


/*************************************************************
 LSD_EXIT_GUI (DLL WRAPPER)
 exit LSD after the GUI is launched
 *************************************************************/
void gui::lsd_exit_gui( int v )
{
	if ( interp != NULL )
	{
		if ( tk_ok )
			cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { catch { destroy . } }" );

		Tcl_Finalize( );
	}

	lsd::lsd_exit( v );
}


/*************************************************************
 LOAD_LMM_OPTIONS
 *************************************************************/
bool gui::load_lmm_options( void )
{
	cmd( "set res [ file exists \"$RootLsd/$LMM_OPTIONS\" ]" );

	if ( get_bool( "res" ) )						// file exists?
	{
		cmd( "set f [ open \"$RootLsd/$LMM_OPTIONS\" r ]" );

		for ( int i = 0; i < LMM_OPTIONS_NUM; ++i )	// read parameters, returning 1 if incomplete
		{
			cmd( "gets $f %s", lmm_options[ i ] );
			cmd( "if { $%s == \"\" } { set res 0 }", lmm_options[ i ] );
		}

		cmd( "close $f" );
	}
	else
	{
		for ( int i = 0; i < LMM_OPTIONS_NUM; ++i )
			cmd( "set %s \"\"", lmm_options[ i ] );

		// fix now missing source directory name
		cmd( "if { $%s == \"\" } { set %s \"%s\" }", lmm_options[ 4 ], lmm_options[ 4 ], lmm_defaults[ 4 ] );
	}

	return get_bool( "res" );
}


/*************************************************************
 UPDATE_LMM_OPTIONS
 *************************************************************/
void gui::update_lmm_options( bool justLmmGeom )
{
	if ( justLmmGeom )
	{
		cmd( "set done 1" );
		cmd( "if { $restoreWin } { set curGeom [ geomtosave .lmm ]; if { $curGeom != \"\" && ! [ string equal $lmmGeom $curGeom ] } { set done 0 } }" );

		if ( get_bool( "done" ) )	// nothing to save?
			return;

		load_lmm_options( );				// if just saving window geometry, first reload from disk

		cmd( "set lmmGeom $curGeom" );
	}

	// save options to disk
	cmd( "set f [ open \"$RootLsd/$LMM_OPTIONS\" w ]" );

	// set undefined parameters to defaults
	for ( int i = 0; i < LMM_OPTIONS_NUM; ++i )
	{
		cmd( "if { ! [ info exists %s ] } { set %s \"\" }", lmm_options[ i ], lmm_options[ i ] );
		cmd( "if { $%s == \"\" } { set %s \"%s\" }", lmm_options[ i ], lmm_options[ i ], lmm_defaults[ i ] );
		cmd( "puts $f \"$%s\"", lmm_options[ i ] );
	}

	cmd( "close $f" );
}


/*************************************************************
 LOAD_MODEL_INFO
 *************************************************************/
bool gui::load_model_info( const char *path )
{
	cmd( "set res [ file exists \"%s/$MODEL_INFO\" ]", path );

	if ( get_bool( "res" ) )						// file exists?
	{
		cmd( "set f [ open \"%s/$MODEL_INFO\" r ]", path );

		for ( int i = 0; i < MODEL_INFO_NUM; ++i )	// read parameters, returning 1 if incomplete
		{
			cmd( "gets $f %s", model_info[ i ] );
			cmd( "if { $%s == \"\" } { set res 0 }", model_info[ i ] );
		}

		cmd( "close $f" );
	}

	return get_bool( "res" );
}


/*************************************************************
 UPDATE_MODEL_INFO
 *************************************************************/
void gui::update_model_info( bool fix )
{
	int i;

	// set undefined parameters to defaults
	if ( fix )
		for ( i = 0; i < MODEL_INFO_NUM; ++i )
		{
			cmd( "if { ! [ info exists %s ] } { set %s \"\" }", model_info[ i ], model_info[ i ] );
			cmd( "if { $%s == \"\" } { set %s \"%s\" }", model_info[ i ], model_info[ i ], model_defaults[ i ] );
		}

#ifndef _LMM_

	else
		// update existing windows positions
		for ( i = 0; i < LSD_WIN_NUM; ++i )
			cmd( "if { $restoreWin } { \
					set curGeom [ geomtosave .%s ]; \
					if { $curGeom != \"\" } { \
						set %s $curGeom \
					} \
				}", wnd_names[ i ], model_info[ i + 3 ] );

	// ensure model name is set
	cmd( "if { ! [ info exists modelName ] || $modelName eq \"\" || $modelName eq \"%s\" } { \
			set modelName [ string map -nocase { fun_ \"\" .cpp \"\" } \"%s\" ] \
		}", model_defaults[ 0 ], eq_file );

#endif

	// save info to disk
	cmd( "set f [ open \"$modelDir/$MODEL_INFO\" w ]" );

	// set undefined parameters to defaults before saving
	for ( i = 0; i < MODEL_INFO_NUM; ++i )
	{
		cmd( "if { ! [ info exists %s ] } { set %s \"\" }", model_info[ i ], model_info[ i ] );
		cmd( "if { $%s == \"\" } { set %s \"%s\" }", model_info[ i ], model_info[ i ], model_defaults[ i ] );
		cmd( "puts $f \"$%s\"", model_info[ i ] );
	}

	cmd( "close $f" );
}


/*************************************************************
 INIT_TCL_TK
 initializes the Tcl/Tk environment
 *************************************************************/
void gui::init_tcl_tk( const char *exec, const char *tcl_app_name )
{
	int num, res;

	if ( ! set_env( true ) )
	{
		log_tcl_error( false, "Set environment variables", "Environment variable setup failed, Tcl/Tk may be unavailable, or environment variable LSDROOT (if exists) does not point to the directory where LSD is installed" );
		lsd_exit_gui( 2 );
	}

	// initialize the tcl/tk interpreter
	Tcl_FindExecutable( exec );
	interp = Tcl_CreateInterp( );
	num = Tcl_Init( interp );
	if ( num != TCL_OK )
	{
		log_tcl_error( false, "Create Tcl interpreter", "Tcl initialization directories not found, check the Tcl/Tk installation  and configuration or reinstall LSD\nTcl Error = %d : %s", num,  Tcl_GetStringResult( interp ) );
		lsd_exit_gui( 3 );
	}

	// set variables and links in TCL interpreter
	Tcl_SetVar( interp, "_LSD_VERSION_", _LSD_VERSION_, 0 );
	Tcl_SetVar( interp, "_LSD_DATE_", _LSD_DATE_, 0 );
	Tcl_LinkVar( interp, "res", ( char * ) &res, TCL_LINK_INT );

	// test Tcl interpreter
	cmd( "set res 1234567890" );
	Tcl_UpdateLinkedVar( interp, "res" );
	if ( res != 1234567890 )
	{
		log_tcl_error( false, "Test Tcl", "Tcl failed, check the Tcl/Tk installation and configuration or reinstall LSD" );
		lsd_exit_gui( 3 );
	}

	// initialize & test the tk application
	num = Tk_Init( interp );
	if ( num == TCL_OK )
		cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { set res 0 } { set res 1 }" );

	if ( num != TCL_OK || res )
	{
		log_tcl_error( false, "Start Tk", "Tk failed, check the Tcl/Tk installation (version 8.6+) and configuration or reinstall LSD\nTcl Error = %d : %s", num,  Tcl_GetStringResult( interp ) );
		lsd_exit_gui( 3 );
	}

	cmd( "wm withdraw ." );
	cmd( "update idletasks" );
	cmd( "tk appname %s", tcl_app_name );
	tk_ok = true;

	// do not open/close terminal in mac
	if ( expr_eq( "$tcl_platform(os)", "Darwin" ) )
	{
		cmd( "catch { console hide }" );
		cmd( "set ::tk::mac::useCompatibilityMetrics 0" );	// disable Carbon compatibility

		// close console if open (usually only in Mac)
		cmd( "foreach i [ winfo interps ] { \
				if { ! [ string equal [ string range $i 0 2 ] lmm ] && ! [ string equal [ string range $i 0 2 ] lsd ] } { \
					send $i \"destroy .\" \
				} \
			}" );

		cmd( "update idletasks" );
	}

	// check installation directory for no spaces in name
	cmd( "if { [ string first \" \" \"[ pwd ]\" ] >= 0	} { set res 1 } { set res 0 }" );
	if ( res )
	{
		log_tcl_error( false, "Path check", "LSD directory path includes spaces, move all the LSD directory in another directory without spaces in the path" );
		cmd( "tk_messageBox -icon error -title Error -type ok -message \"Installation error\" -detail \"The LSD directory is: '[ pwd ]'\n\nIt includes spaces, which makes impossible to compile and run LSD models.\nThe LSD directory must be located where there are no spaces in the full path name.\nMove all the LSD directory in another directory. If it exists, delete the '%s' file from the sources (src) directory.\n\nLSD is aborting now.\"", SYSTEM_OPTIONS );
		lsd_exit_gui( 4 );
	}

	Tcl_UnlinkVar( interp, "res" );
}


/*************************************************************
 SET_ENV
 sets the required environment variables
 it does nothing if the variables already exist
 and Windows PATH has a compiler in it
 *************************************************************/
bool gui::set_env( bool set )
{
	bool res = true;
	char *lsd_root, cur_path[ PATH_MAX ];
	static char *lsd_root_env = NULL, *tcl_lib_env = NULL, *path_env = NULL;

	if ( set )
	{
		lsd_root = getenv( "LSDROOT" );

		if ( lsd_root == NULL )
		{
			if ( getcwd( cur_path, PATH_MAX ) != NULL )
				lsd_root = search_lsd_root( lsd::clean_path( cur_path ), PATH_MAX );

			if ( lsd_root != NULL )
			{
				delete [ ] lsd_root_env;
				lsd_root_env = new char[ strlen( "LSDROOT" ) + strlen( lsd_root ) + 2 ];
				sprintf( lsd_root_env, "LSDROOT=%s", lsd_root );

				res = ! ( bool ) putenv( lsd_root_env );
			}
			else
				res = false;
		}

#ifdef _WIN32
		char *file, *lsd_bin, *path;
		const char *compilers[ ] = WIN_COMP_PATH;
		int i, st;
		struct stat info;

		path = getenv( "PATH" );

		if ( lsd_root != NULL && getenv( TCL_LIB_VAR ) == NULL )
		{
			lsd_root = lsd::clean_path( lsd_root );

			file = new char[ strlen( lsd_root ) + strlen( TCL_LIB_PATH ) + strlen( TCL_LIB_INIT ) + 3 ];
			sprintf( file, "%s/%s/%s", lsd_root, TCL_LIB_PATH, TCL_LIB_INIT );
			st = stat( file, &info );
			delete [ ] file;

			if ( st == 0 )
			{
				delete [ ] tcl_lib_env;
				tcl_lib_env = new char[ strlen( TCL_LIB_VAR ) + strlen( lsd_root ) + strlen( TCL_LIB_PATH ) + 3 ];
				sprintf( tcl_lib_env, "%s=%s/%s", TCL_LIB_VAR, lsd_root, TCL_LIB_PATH );

				res = ! ( bool ) putenv( tcl_lib_env );
			}
			else
				if ( lsd::run_system( TCL_FIND_EXE ) != 0 )
					res = false;	// just stop if Tcl/Tk is not on path
		}

		if ( lsd_root != NULL && path != NULL )
		{
			// check if not already in path and add it in the adequate order
			lsd_bin = new char[ win_path( lsd_root ).size( ) + strlen( TCL_EXEC_PATH ) + 2 ];
			sprintf( lsd_bin, "%s\\%s", win_path( lsd_root ).c_str( ), TCL_EXEC_PATH );

			if ( strstr( path, lsd_bin ) == NULL )
			{
				// look for known 64-bit compilers
				for ( i = 0, st = 1; i < WIN_COMP_NUM; ++i )
					if ( strstr( path, compilers[ i ] ) != NULL )
						st = 0;

				delete [ ] path_env;
				path_env = new char[ strlen( path ) + strlen( lsd_bin ) + 7 ];

				if ( st == 0 )
					sprintf( path_env, "PATH=%s;%s", path, lsd_bin );
				else
					sprintf( path_env, "PATH=%s;%s", lsd_bin, path );

				putenv( path_env );
			}

			delete [ ] lsd_bin;
		}
#else
		res = true;					// do not stop on linux/mac
#endif
	}
	else
	{
		delete [ ] tcl_lib_env;
		delete [ ] lsd_root_env;
		delete [ ] path_env;
	}

	return res;
}


/*************************************************************
 SEARCH_LSD_ROOT
 searches LSD root directory upwards to the root
 *************************************************************/
char *gui::search_lsd_root( char *path, int pathSz )
{
	bool miss;
	const char *files[ ] = LSD_MIN_FILES;
	char *file, cur_dir[ PATH_MAX ], last_dir[ PATH_MAX ], orig_dir[ PATH_MAX ], *found = NULL;
	int i, st;
	struct stat info;

	if ( getcwd( orig_dir, PATH_MAX ) == NULL )
		return NULL;

	if ( chdir( path ) == -1 )
		goto end;

	strcpy( last_dir, "" );

	do
	{
		if ( getcwd( cur_dir, PATH_MAX ) == NULL || ! strcmp( lsd::clean_path( cur_dir ), last_dir ) )
			goto end;

		for ( i = 0, miss = false; i < LSD_MIN_NUM; ++i )
		{
			file = new char[ strlen( cur_dir ) + strlen( files[ i ] ) + 2 ];
			sprintf( file, "%s/%s", cur_dir, files[ i ] );
			st = stat( file, &info );
			delete [ ] file;

			if ( st != 0 )
			{
				miss = true;
				break;
			}
		}

		if ( ! miss )
		{
			lsd::strcpyn( path, cur_dir, pathSz );
			found = path;
			break;
		}

		lsd::strcpyn( last_dir, cur_dir, PATH_MAX );
	}
	while ( ! chdir( ".." ) );

	end:
	chdir( orig_dir );

	return found;
}


/*************************************************************
 CMD
 *************************************************************/
void gui::cmd( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );
	cmd_backend( cm, argptr );
	va_end( argptr );
}


/*************************************************************
 CMD_BACKEND
 *************************************************************/
void gui::cmd_backend( const char *cm, va_list arg )
{
	static bool bufdyn;
	static char *buffer, bufstat[ MAX_BUFF_SIZE ];
	static int reqsz, sz;
	static va_list argcpy;

#ifndef _NP_
	// abort if not running in main LSD thread
	if ( std::this_thread::get_id( ) != lsd::main_thread )
		return;
#endif

	// abort if Tcl interpreter not initialized
	if ( interp == NULL )
	{
#ifdef _LMM_
		FILE *stderr_ptr = stderr;
#else
		FILE *stderr_ptr = lsd::stderr_ptr;
#endif
		fprintf( stderr_ptr, "\nTcl interpreter not initialized. Quitting LSD now.\n" );
		lsd_exit_gui( 24 );
	}

	buffer = bufstat;
	va_copy( argcpy, arg );
	reqsz = vsnprintf( buffer, MAX_BUFF_SIZE, cm, arg );

	if ( reqsz < 0 )
	{
		log_tcl_error( true, "Invalid Tcl command", "Cannot expand command '%s...'", cm );
		return;
	}

	// handle very large commands
	if ( reqsz >= MAX_BUFF_SIZE )
	{
		buffer = new char[ reqsz + 1 ];
		sz = vsnprintf( buffer, reqsz + 1, cm, argcpy );

		if ( reqsz < 0 || sz > reqsz )
		{
			log_tcl_error( true, "Invalid Tcl command", "Cannot expand command '%s...'", cm );
			delete [ ] buffer;
			return;
		}

		bufdyn = true;
	}
	else
		bufdyn = false;

	va_end( argcpy );

	if ( Tcl_Eval( interp, buffer ) != TCL_OK )
		log_tcl_error( true, cm, Tcl_GetStringResult( interp ) );

	if ( bufdyn )
		delete [ ] buffer;
}


/*************************************************************
 LOG_TCL_ERROR
 *************************************************************/
void gui::log_tcl_error( bool show, const char *cm, const char *message, ... )
{
	static char *err_path, ftime[ 80 ], fname[ MAX_PATH_LENGTH ], buffer[ MAX_BUFF_SIZE ];
	static struct tm *timeinfo;
	static time_t rawtime;
	static va_list argptr;
	static FILE *f;

	static bool firstCall = true;

#ifndef _NP_
	l_guardT lock( lock_log_tcl_err );
#endif

	va_start( argptr, message );
	vsnprintf( buffer, MAX_BUFF_SIZE, message, argptr );
	va_end( argptr );

#ifdef _LMM_
	err_path = lsd::root_lsd;
#else
	err_path = lsd::model_path;
#endif

	if ( err_path != NULL && strlen( err_path ) > 0 )
		snprintf( fname, MAX_PATH_LENGTH, "%s/%s", err_path, err_file );
	else
		snprintf( fname, MAX_PATH_LENGTH, "%s", err_file );

	f = fopen( fname, "a" );
	if ( f == NULL )
	{
		show_tcl_error( "Log file write error", "Cannot write to log file '%s'\nCheck disk and write permissions", fname );
		return;
	}

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	strftime ( ftime, 80, "%x %X", timeinfo );

	if ( firstCall )
	{
		firstCall = false;
		fprintf( f,"\n\n====================> NEW TCL SESSION\n" );
	}

	fprintf( f, "\n(%s)\nCommand:\n%s\nMessage:\n%s\n-----\n", ftime, cm, buffer );
	fclose( f );

	if ( show )
		show_tcl_error( "LSD error", "Internal LSD error. See file '%s'", fname );
}


/*************************************************************
 TCL_LOG_TCL_ERROR
 Entry point function for access from the Tcl interpreter
 *************************************************************/
int gui::Tcl_log_tcl_error( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	if ( argc != 4 || argv[ 1 ] == NULL || argv[ 2 ] == NULL || argv[ 3 ] == NULL )	// require 3 parameters
		return TCL_ERROR;

	log_tcl_error( strcmp( argv[ 1 ], "0" ), argv[ 2 ], argv[ 3 ] );

	static char empty[ ] = "";
	Tcl_SetResult( interp, empty, TCL_VOLATILE );
	return TCL_OK;
}


/*************************************************************
 SHOW_TCL_ERROR
 Show the error message to the user
 *************************************************************/
void gui::show_tcl_error( const char *boxTitle, const char *errMsg, ... )
{
	static char logText[ MAX_LINE_SIZE ];
	static va_list argptr;

	va_start( argptr, errMsg );
	vsnprintf( logText, MAX_LINE_SIZE, errMsg, argptr );
	va_end( argptr );

#ifdef _LMM_
	if ( tk_ok )
		cmd( "if { [ llength [ info procs ttk::messageBox ] ] > 0 } { \
				ttk::messageBox -type ok -title Error -icon error -message \"%s\" -detail \"%s.\" \
			} else { \
				tk_messageBox -type ok -title Error -icon error -message \"%s\" -detail \"%s.\" \
			}", boxTitle, logText, boxTitle, logText );
	else
		fprintf( stderr, "\n%s\n", logText );
#else
	plog( "\n%s\n", logText );
#endif
}


/*************************************************************
 TCL_DISCARD_CHANGE
 Entry point function for access from the Tcl interpreter
 *************************************************************/
extern bool discard_change( void );

int gui::Tcl_discard_change( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
#ifdef _LMM_
	if ( ::discard_change( ) == 1 )
#else
	if ( discard_change( ) == 1 )
#endif
		Tcl_SetResult( interp, ( char * ) "ok", TCL_VOLATILE );
	else
		Tcl_SetResult( interp, ( char * ) "cancel", TCL_VOLATILE );
	return TCL_OK;
}


/*************************************************************
 EXISTS_VAR
 *************************************************************/
bool gui::exists_var( const char *lab )
{
	cmd( "set res [ info exists \"%s\" ]", lab );
	return get_bool( "res" );
}


/*************************************************************
 EXISTS_WINDOW
 *************************************************************/
bool gui::exists_window( const char *lab )
{
	cmd( "set res [ winfo exists \"%s\" ]", lab );
	return get_bool( "res" );
}


/*************************************************************
 GET_BOOL
 Set var to NULL to just get the Tcl value
 *************************************************************/
bool gui::get_bool( const char *tcl_var, bool *var )
{
	const char *strvar;
	int intvar;

	strvar = get_str( tcl_var );
	if ( strvar == NULL )
	{
		if ( var != NULL )
			return *var;
		else
			return false;
	}

	if ( Tcl_GetBoolean( interp, strvar, & intvar ) != TCL_OK )
		if ( Tcl_GetInt( interp, strvar, & intvar ) != TCL_OK )
		{
			log_tcl_error( true, "Cannot convert to boolean", "Internal LSD error converting variable '%s' containing '%s'. If the problem persists, please contact developers", tcl_var, strvar );
			return false;
		}

	if ( var != NULL )
		*var = intvar ? true : false;

	return intvar ? true : false;
}


/*************************************************************
 GET_INT
 Set var to NULL to just get the Tcl value
 *************************************************************/
int gui::get_int( const char *tcl_var, int *var )
{
	const char *strvar;
	int intvar;

	strvar = get_str( tcl_var );
	if ( strvar == NULL )
	{
		if ( var != NULL )
			return *var;
		else
			return 0;
	}

	if ( Tcl_GetInt( interp, strvar, & intvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot convert to integer", "Internal LSD error converting variable '%s' containing '%s'. If the problem persists, please contact developers", tcl_var, strvar );
		return 0;
	}

	if ( var != NULL )
		*var = intvar;

	return intvar;
}


/*************************************************************
 GET_LONG
 Set var to NULL to just get the Tcl value
 *************************************************************/
long gui::get_long( const char *tcl_var, long *var )
{
	const char *strvar;
	long longvar;

	strvar = get_str( tcl_var );
	if ( strvar == NULL )
	{
		if ( var != NULL )
			return *var;
		else
			return 0;
	}

	if ( sscanf( strvar, "%ld", & longvar ) != 1 )
	{
		log_tcl_error( true, "Cannot convert to long", "Internal LSD error converting variable '%s' containing '%s'. If the problem persists, please contact developers", tcl_var, strvar );
		return 0;
	}

	if ( var != NULL )
		*var = longvar;

	return longvar;
}


/*************************************************************
 GET_DOUBLE
 Set var to NULL to just get the Tcl value
 *************************************************************/
double gui::get_double( const char *tcl_var, double *var, bool no_error )
{
	const char *strvar;
	double dblvar;

	strvar = get_str( tcl_var );
	if ( strvar == NULL )
	{
		if ( var != NULL )
			return *var;
		else
			return NAN;
	}

	if ( Tcl_GetDouble( interp, strvar, & dblvar ) != TCL_OK )
	{
		if ( ! no_error )
			log_tcl_error( true, "Cannot convert to double", "Internal LSD error converting variable '%s' containing '%s'. If the problem persists, please contact developers", tcl_var, strvar );
		return NAN;
	}

	if ( var != NULL )
		*var = dblvar;

	return dblvar;
}


/*************************************************************
 GET_STR
 Set var to NULL to just get the Tcl pointer
 *************************************************************/
char *gui::get_str( const char *tcl_var, char *var, int var_size )
{
	const char *strvar = Tcl_GetVar( interp, tcl_var, 0 );

	if ( strvar == NULL )
	{
		log_tcl_error( true, "Invalid Tcl variable name", "Internal LSD error searching for variable '%s'. If the problem persists, please contact developers", tcl_var );
		return var;
	}

	if ( var != NULL && var_size > 0 )
	{
		lsd::strcpyn( var, strvar, var_size );
		return var;
	}
	else
		return ( char * ) strvar;
}

const char *gui::get_str( const char *tcl_var )
{
	return ( const char * ) get_str( tcl_var, NULL, 0 );
}


/*************************************************************
 EQ_STR
 Compare if Tcl expression, evaluating it
 before comparison, is equal to C string
 *************************************************************/
bool gui::expr_eq( const char *tcl_exp, const char *c_str )
{
	const char *strvar = eval_str( tcl_exp );

	if ( strvar != NULL && c_str != NULL )
		return strcmp( strvar, c_str ) == 0;
	else
		return false;
}


/*************************************************************
 EVAL_STR
 Evaluate Tcl expression to C string
 ATTENTION: if var is NULL, the returned result
 string buffer is valid only until next Tcl invocation
 *************************************************************/
char *gui::eval_str( const char *tcl_exp, char *var, int var_size )
{
	if ( Tcl_ExprString( interp, tcl_exp ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot evaluate to string", "Internal LSD error evaluating expression '%s'. If the problem persists, please contact developers", tcl_exp );
		return var;
	}

	if ( var != NULL && var_size > 0 )
	{
		lsd::strcpyn( var, Tcl_GetStringResult( interp ), var_size );
		return var;
	}
	else
		return ( char * ) Tcl_GetStringResult( interp );
}

const char *gui::eval_str( const char *tcl_exp )
{
	return ( const char * ) eval_str( tcl_exp, NULL, 0 );
}


/*************************************************************
 EVAL_BOOL
 Evaluate Tcl expression to C boolean
 *************************************************************/
bool gui::eval_bool( const char *tcl_exp )
{
	int intvar;
	long longvar;

	if ( Tcl_ExprBoolean( interp, tcl_exp, & intvar ) == TCL_OK )
		return intvar ? true : false;

	if ( Tcl_ExprLong( interp, tcl_exp, & longvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot evaluate to boolean", "Internal LSD error evaluating expression '%s'. If the problem persists, please contact developers", tcl_exp );
		return false;
	}

	return longvar ? true : false;
}


/*************************************************************
 EVAL_INT
 Evaluate Tcl expression to C integer
 *************************************************************/
int gui::eval_int( const char *tcl_exp )
{
	return ( int ) eval_long( tcl_exp );
}


/*************************************************************
 EVAL_LONG
 Evaluate Tcl expression to C long
 *************************************************************/
long gui::eval_long( const char *tcl_exp )
{
	long longvar;

	if ( Tcl_ExprLong( interp, tcl_exp, & longvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot evaluate to long integer", "Internal LSD error evaluating expression '%s'. If the problem persists, please contact developers", tcl_exp );
		return 0;
	}

	return longvar;
}


/*************************************************************
 EVAL_DOUBLE
 Evaluate Tcl expression to C double
 *************************************************************/
double gui::eval_double( const char *tcl_exp )
{
	double dblvar;

	if ( Tcl_ExprDouble( interp, tcl_exp, & dblvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot evaluate to double", "Internal LSD error evaluating expression '%s'. If the problem persists, please contact developers", tcl_exp );
		return NAN;
	}

	return dblvar;
}


/*************************************************************
 CHECK_OPTION_FILES
 check if model and system option
 files exist and create them if not
 *************************************************************/
void gui::check_option_files( bool sys )
{
	if ( ! sys && ! eval_bool( "[ file exists \"$modelDir/$MODEL_OPTIONS\" ]" ) && eval_bool( "$modelDir ne \"\"" ) && eval_bool( "$modelDir ne $RootLsd" ) )
	{
		cmd( "set dir [ glob -nocomplain \"$modelDir/fun_*.cpp\" ]" );
		cmd( "if { $dir ne \"\" } { set b [ file tail [ lindex $dir 0 ] ] } { set b \"fun_UNKNOWN.cpp\" }" );
		cmd( "set a \"# LSD options\nTARGET=$DefaultExe\nFUN=[ file rootname \"$b\" ]\nPRECOMPILED=true\n\n# Additional model files\nFUN_EXTRA=\n\n# Compiler options\nSWITCH_CC=-O0 -ggdb3\nSWITCH_CC_LNK=\"" );
		cmd( "set f [ open \"$modelDir/$MODEL_OPTIONS\" w ]" );
		cmd( "puts $f $a" );
		cmd( "close $f" );
	}

	if ( ! eval_bool( "[ file exists \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\" ]" ) )
	{
		cmd( "if [ string equal $tcl_platform(platform) windows ] { \
				set sysfile \"system_options-windows.txt\" \
			} elseif { [ string equal $tcl_platform(os) Darwin ] } { \
				set sysfile \"system_options-mac.txt\" \
			} else { \
				set sysfile \"system_options-linux.txt\" \
			}" );
		cmd( "set f [ open \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\" w ]" );
		cmd( "set f1 [ open \"$RootLsd/$LsdSrc/$sysfile\" r ]" );
		cmd( "puts $f \"# LSD options\"" );
		cmd( "puts $f \"LSDROOT=$RootLsd\"" );
		cmd( "puts $f \"SRC=$LsdSrc\n\"" );
		cmd( "puts $f [ string trim [ read $f1 ] ]" );
		cmd( "close $f" );
		cmd( "close $f1" );
	}
}


/*************************************************************
 GET_FUN_NAME
 get current equation file name
 *************************************************************/
const char *gui::get_fun_name( char *str, int str_sz, bool nw )
{
	char buf[ MAX_PATH_LENGTH ];
	FILE *f;

	make_makefile( nw );

	cmd( "set fapp [ file nativename \"$modelDir/makefile%s\" ]", nw ? "NW" : "" );
	f = fopen( get_str( "fapp" ), "r" );
	if ( f == NULL )
		goto error;

	do
		fgets( str, str_sz, f );
	while ( strncmp( str, "FUN=", 4 ) && ! feof( f ) );

	fclose( f );

	if ( strncmp( str, "FUN=", 4 ) != 0 )
		goto error;

	sscanf( str + 4, "%994s", buf );
	snprintf( str, str_sz, "%s.cpp", buf );

	return str;

error:
	cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Makefile not found or corrupted\" -detail \"Please check 'Model Options' and 'System Options' in LMM menu 'Model'.\"" );
	return NULL;
}


/*************************************************************
 GET_TARGET_NAME
 get current executable file name
 *************************************************************/
const char *gui::get_target_name( char *str, int str_sz, bool nw )
{
	char buf[ MAX_PATH_LENGTH ], buf1[ MAX_PATH_LENGTH ];
	FILE *f;

	if ( nw )					// NW version use fixed name because of batches
	{
		snprintf( str, str_sz, "lsdNW%s", platform == _WIN_ ? ".exe" : "" );
		return str;
	}

	make_makefile( nw );

	cmd( "set fapp [ file nativename \"$modelDir/makefile%s\" ]", nw ? "NW" : "" );
	f = fopen( get_str( "fapp" ), "r" );
	if ( f == NULL )
		goto error;

	do
		fgets( str, str_sz, f );
	while ( strncmp( str, "TARGET=", 7 ) && ! feof( f ) );

	fclose( f );

	if ( strncmp( str, "TARGET=", 7 ) != 0 )
		goto error;

	sscanf( str + 7, "%994s", buf );
	lsd::strcpyn( buf1, buf, MAX_PATH_LENGTH );

	if ( strcmp( strupr( buf1 ), "LSD" ) == 0 )
		strcpy( buf, "LSD" );			// LSD default target is case insensitive

	snprintf( str, str_sz, "%s%s", buf, platform == _WIN_ ? ".exe" : "" );

	return str;

error:
	cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Makefile not found or corrupted\" -detail \"Please check 'Model Options' and 'System Options' in LMM menu 'Model'.\"" );
	return NULL;
}


/*************************************************************
 GET_PRECOMPILED_FLAG
 get current executable pre-
 compilation flag
 *************************************************************/
bool gui::get_precompiled_flag( const char *exec, bool nw )
{
	bool deftarg = true, precomp = true;		// defaults if settings are missing
	char buf[ MAX_PATH_LENGTH ], buf1[ MAX_PATH_LENGTH ];
	FILE *f;

	if ( ! nw )
	{
		// non default executable name - cannot use precompiled code
		lsd::strcpyn( buf, exec, MAX_PATH_LENGTH );

		if ( platform == _WIN_ )
			strupr( buf );

		if ( strcmp( buf, platform == _WIN_ ? "LSD.EXE" : "LSD" ) != 0 )
			deftarg = false;
	}

	cmd( "set fapp [ file nativename \"$modelDir/makefile\" ]" );
	f = fopen( get_str( "fapp" ), "r" );
	if ( f == NULL )
		goto error;

	do
		fgets( buf, MAX_PATH_LENGTH, f );
	while ( strncmp( buf, "PRECOMPILED=", 12 ) && ! feof( f ) );

	fclose( f );

	if ( deftarg && strncmp( buf, "PRECOMPILED=", 12 ) != 0 )
		return precomp;

	sscanf( buf + 12, "%989s", buf1 );
	strupr( buf1 );

	if ( strncmp( buf1, "FALSE", MAX_PATH_LENGTH ) == 0 ||
		 strncmp( buf1, "NO", MAX_PATH_LENGTH ) == 0 ||
		 strncmp( buf1, "0", MAX_PATH_LENGTH ) == 0 )
		precomp = false;

	if ( ! deftarg && precomp )
	{
		cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Cannot use pre-compiled code\" -detail \"Non-default TARGET name '[ file rootname %s ]' cannot be used together with the pre-compiled code option (PRECOMPILED = true).\n\nPlease adjust your model options to avoid this message.\"", exec );
		precomp = false;
	}

	return precomp;

error:
	cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Makefile not found or corrupted\" -detail \"Please check 'Model Options' and 'System Options' in LMM menu 'Model'.\"" );
	return NULL;
}


/*************************************************************
 MAKE_NO_WINDOW
 create a no-window command-line version of LSD
 *************************************************************/
bool gui::make_no_window( void )
{
	int i;

	cmd( "set ans 1" );
	cmd( "if { \"[ check_sys_opt ]\" ne \"\" } { \
			if { [ ttk::messageBox -parent . -icon warning -title Warning -type yesno -default no -message \"Invalid system options detected\" -detail \"The current LSD configuration is invalid for your platform. To fix it, please use LMM menu option 'Model>System Options', press the 'Default' button, and then 'OK'.\n\nDo you want to proceed anyway?\" ] == no } { \
				set ans 0 \
			} \
		}" );
	if ( get_int( "ans" ) == 0 )
		return false;

	// copy the base LSD source files to distribution directory
	cmd( "if { ! [ file exists \"$modelDir/$LsdSrc\" ] } { \
			file mkdir \"$modelDir/$LsdSrc\" \
		}" );

	for ( i = 0; i < LSD_NW_NUM; ++i )
		cmd( "file copy -force \"$RootLsd/$LsdSrc/%s\" \"$modelDir/$LsdSrc\"", lsd_nw_src[ i ] );

	// copy LSD library files always
	cmd( "if { ! [ file exists \"$modelDir/$LsdSrc/lib\" ] } { \
			file mkdir \"$modelDir/$LsdSrc/lib\" \
		}" );

	cmd( "foreach f [ glob -nocomplain -directory \"$RootLsd/$LsdSrc/lib\" * ] { \
			file copy -force $f \"$modelDir/$LsdSrc/lib\" \
		}" );

	// copy 3rd-party C++ libraries just once
	cmd( "if { ! [ file exists \"$modelDir/$LsdSrc/clib\" ] } { \
			file copy -force \"$RootLsd/$LsdSrc/clib\" \"$modelDir/$LsdSrc\" \
		}" );

	// create makefileNW and compile a local machine version of lsdNW
	return compile_run( false, true );
}


/*************************************************************
 MAKE_MAKEFILE
 create makefiles to compile LSD
 *************************************************************/
void gui::make_makefile( bool nw )
{
	check_option_files( );

	cmd( "set f [ open \"$modelDir/$MODEL_OPTIONS\" r ]" );
	cmd( "set a [ string trim [ read $f ] ]" );
	cmd( "close $f" );

	cmd( "set f [ open \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\" r ]" );
	cmd( "set d [ string trim [ read $f ] ]" );
	cmd( "close $f" );

	cmd( "set f [ open \"$RootLsd/$LsdSrc/makefile-%s.txt\" r ]", nw ? "NW" : get_str( "CurPlatform" ) );

	cmd( "set b [ string trim [ read $f ] ]" );
	cmd( "close $f" );

	cmd( "set c \"# Model compilation options\\n$a\\n\\n# System compilation options\\n$d\\n\\n# Body of makefile%s (from makefile_%s.txt)\\n$b\"", nw ? "NW" : "", nw ? "NW" : get_str( "CurPlatform" ) );
	cmd( "set f [ open \"$modelDir/makefile%s\" w ]", nw ? "NW" : "" );
	cmd( "puts $f $c" );
	cmd( "close $f" );
}


/*************************************************************
 COMPILE_RUN
 compile LSD, GUI or command line
 and optionally execute it
 *************************************************************/
bool gui::compile_run( int run_mode, bool nw )
{
	bool precompiled, ret = false;
	char str[ 2 * MAX_PATH_LENGTH ];
	const char *s;
	int res, max_threads = 1;
	FILE *f;

	Tcl_LinkVar( interp, "res", ( char * ) &res, TCL_LINK_INT );

	cmd( "set oldpath [ pwd ]" );
	cmd( "cd \"$modelDir\"" );

#ifdef _LMM_

	cmd( "destroytop .mm" );	// close any open compilation results window

	s = get_str( "modelName" );
	if ( s == NULL || ! strcmp( s, "" ) )
	{
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
		goto end;
	}

#endif

	// get source name
	s = get_fun_name( str, 2 * MAX_PATH_LENGTH, nw );
	if ( s == NULL || ! strcmp( s, "" ) || ( f = fopen( s, "r" ) ) == NULL )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Equation file not found\" -detail \"File '%s' is no longer available in directory '$modelDir'.\" ", s );
		goto end;
	}
	else
		fclose( f );

	cmd( "set fname \"%s\"", s );

	// get executable name
	cmd( "set mainExe %s", get_target_name( str, 2 * MAX_PATH_LENGTH ) );

	if ( nw )
		get_target_name( str, 2 * MAX_PATH_LENGTH, nw );

#ifdef _LMM_
	if ( run_mode == 0 && ! nw )// delete existing object file if it's just compiling
	{							// to force recompilation

		cmd( "set oldObj \"[ temp_dir ]/[ file rootname $mainExe ]/[ file tail $modelDir ]/[ file rootname [ lindex [ glob -nocomplain fun_*.cpp ] 0 ] ].o\"" );
		cmd( "if { [ file exists \"$oldObj\" ] } { file delete \"$oldObj\" }" );
	}

	precompiled = get_precompiled_flag( str, nw );
#else
	precompiled = get_precompiled_flag( str, true );
#endif

	if ( ! nw && precompiled )	// remove old unused executables
		cmd( "if { [ file exists %s ] } { file delete %s }", str, str );

	// show compilation banner
	cmd( "if { ( [ info exists autoHide ] && ! $autoHide ) || %d == 0 } { \
			set parWnd .; \
			set posWnd centerW \
		} else { \
			set parWnd \"\"; \
			set posWnd centerS \
		}", run_mode );

	cmd( "newtop .t \"Please Wait\" \"\" $parWnd" );

	if ( nw )
		cmd( "ttk::label .t.l1 -style bold.TLabel -justify center -text \"Compiling 'No Window' model...\"" );
	else
		cmd( "ttk::label .t.l1 -style bold.TLabel -justify center -text \"Compiling model...\"" );

	if ( run_mode != 0 )
		cmd( "ttk::label .t.l2 -justify center -text \"Just recompiling equation file(s) changes.\nOn success, the %s will be launched.\nOn failure, a new window will show the compilation errors.\"", run_mode != 2 ? "new model program" : "debugger" );
	else
		if ( nw )
#ifdef _LMM_
			cmd( "ttk::label .t.l2 -justify center -text \"Creating command-line model program ('lsdNW').\nOn success, the model directory can be also ported to any computer.\nOn failure, a new window will show the compilation errors.\"" );
#else
			cmd( "ttk::label .t.l2 -justify center -text \"Creating updated command-line model program ('lsdNW').\nOn success, the requested operation will continue.\"" );
#endif
		else
			cmd( "ttk::label .t.l2 -justify center -text \"Recompiling the entire model program.\nOn success, the new program will NOT be launched.\nOn failure, a new window will show the compilation errors.\"" );

	cmd( "pack .t.l1 .t.l2 -padx 5 -pady 5" );
	cmd( "cancel .t b { set res 2 }");
	cmd( "showtop .t $posWnd" );

#ifdef _LMM_

	// minimize LMM if required
	cmd( "set res $autoHide" );				// get auto hide status
	if ( res && run_mode != 0 )				// hide LMM?
		cmd( "wm iconify ." );

#endif

	// number of cores for make parallelization
	max_threads = std::thread::hardware_concurrency( );

	// start compilation as a background task
	res = -1;
	cmd( "make_background %s %d %d %d ", str, max_threads, nw, precompiled );

	// loop to wait compilation to finish or be aborted
	while ( res < 0 )
		Tcl_DoOneEvent( 0 );

	// close banner
	cmd( "destroytop .t" );
	Tcl_UnlinkVar( interp, "res" );

	ret = false;

	if ( res == 2 )
	{
		cmd( "catch { close $makePipe }" );
		cmd( "if [ file exists make.bat ] { catch { file delete make.bat } }" );
		cmd( "if { [ winfo exists .f.t.t ] } { \
				focustop .f.t.t \
			} else { \
				focustop . \
			}" );
		goto end;
	}

#ifdef _LMM_

	if ( res == 0 )							// compilation failure?
	{
		cmd( "set res $autoHide" );			// get auto hide status
		if ( run_mode != 0 && res )			// auto unhide LMM if necessary
			cmd( "focustop .f.t.t" );		// only reopen if error
		show_comp_result( nw );				// show errors
	}
	else
	{
		if ( nw )
			cmd( "ttk::messageBox -parent . -type ok -icon info -title \"'No Window' Model\" -message \"Compilation successful\" -detail \"A non-graphical, command-line model program was created.\n\nThe executable 'lsdNW\\[.exe\\]' for this computer was generated in your model directory. It can be ported to any computer with a GCC-compatible compiler, like a high-performance server.\n\nTo port the model, copy the entire model directory:\n\n[ fn_break [ file nativename \"$modelDir\" ] 40 ]\n\nto another computer (including the subdirectory '$LsdSrc'). After the copy, use the following steps to use it:\n\n- open the command-line terminal/shell\n- change to the copied model directory ('cd')\n- recompile with the command:\n\nmake -f makefileNW\n\n- run the model program with a preexisting model configuration file ('.lsd' extension) using the command:\n\n./lsdNW -f CONF_NAME.lsd\n\n(you may have to remove the './' in Windows)\n\nSimulations run in the command-line will save the results into files with '.res\\[.gz\\]' and '.tot\\[.gz\\]' extensions.\"" );
		else
		{
			if ( run_mode != 0 )				// no problem - execute
			{
				// create the element list file in background and try to open 10 times every 50 ms
				cmd( "after 0 { create_elem_file $modelDir }" );
				cmd( "update" );

				if ( run_mode == 1 )			// run executable directly (not debugger)
				{
					cmd( "set n 10" );
					cmd( "set result \"\"" );

					switch ( platform )
					{
						case _LIN_:
							cmd( "while { [ catch { exec -- %s/%s & } result ] && $n > 0 } { incr n -1; after 50 }", precompiled ? lsd::root_lsd : ".", str );
							break;

						case _MAC_:
							cmd( "while { [ catch { exec -- open -F -n %s/%s.app & } result ] && $n > 0 } { incr n -1; after 50 }", precompiled ? lsd::root_lsd : ".", str );
							break;

						case _WIN_:
							cmd( "while { [ catch { exec -- [ file nativename \"%s/%s\" ] & } result ] && $n > 0 } { incr n -1; after 50 }", precompiled ? lsd::root_lsd : ".", str );
							break;
					}
				}
			}
			else
				cmd( "create_elem_file $modelDir" );
		}

		ret = true;
	}

#else

	if ( res == 0 )							// compilation failure?
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Compilation failed\" -detail \"The command-line model program	('lsdNW') could not be compiled, likely due to a syntax problem.\n\nPlease go to LMM,  choose menu 'Model'/'Generate 'No Window' Version' to recompile, and check the Compilation Errors window for details on the problem(s).\"" );
	}
	else
		ret = true;

#endif

	// update no-window executable time if not recompiled
	if ( nw && ret )
		cmd( "if { [ file exists $mainExe ] && [ file exists $targetExe ] && [ file mtime $mainExe ] > [ file mtime $targetExe ] } { \
				file mtime $targetExe [ file mtime $mainExe ] \
			}" );

end:

	cmd( "cd \"$oldpath\"" );

	return ret;
}


/*************************************************************
 SHOW_COMP_RESULT
 *************************************************************/
void gui::show_comp_result( bool nw )
{
	cmd( "set cerr 1.0" );						// search start position in file
	cmd( "set error \" error:\"" );				// error string to be searched
	cmd( "set errfil \"\"" );
	cmd( "set errlin \"\"" );
	cmd( "set errcol \"\"" );

	cmd( "newtop .mm \"Compilation Errors%s\" { .mm.b.close invoke } \"\"", nw ? " (No Window Version)" : "" );

	cmd( "ttk::label .mm.lab -justify left -text \"- Each error is indicated by the file name and line number where it has been identified.\n- Click on 'Go to Error' to open the equation file on the indicated line.\n- Consider that the error may have been originated in the previous lines.\n- Start fixing errors at the beginning of the list, subsequent errors may be due to previous ones.\"" );
	cmd( "pack .mm.lab" );

	cmd( "ttk::frame .mm.t" );
	cmd( "ttk::scrollbar .mm.t.yscroll -command \".mm.t.t yview\"" );
	cmd( "ttk::text .mm.t.t -yscrollcommand \".mm.t.yscroll set\" -wrap word -entry 0 -dark $darkTheme -style smallFixed.TText" );
	cmd( "pack .mm.t.yscroll -side right -fill y" );
	cmd( "pack .mm.t.t -expand yes -fill both" );
	cmd( "mouse_wheel .mm.t.t" );

	cmd( "pack .mm.t -expand yes -fill both" );

	cmd( "ttk::frame .mm.i" );

	cmd( "ttk::frame .mm.i.f" );
	cmd( "ttk::label .mm.i.f.l -text \"File:\"" );
	cmd( "ttk::label .mm.i.f.n -anchor w -width 50 -style hl.TLabel" );
	cmd( "pack .mm.i.f.l .mm.i.f.n -side left" );

	cmd( "ttk::frame .mm.i.l" );
	cmd( "ttk::label .mm.i.l.l -text \"Line:\"" );
	cmd( "ttk::label .mm.i.l.n -anchor w -width 5 -style hl.TLabel" );
	cmd( "pack .mm.i.l.l .mm.i.l.n -side left" );

	cmd( "ttk::frame .mm.i.c" );
	cmd( "ttk::label .mm.i.c.l -text \"Column:\"" );
	cmd( "ttk::label .mm.i.c.n -anchor w -width 5 -style hl.TLabel" );
	cmd( "pack .mm.i.c.l .mm.i.c.n -side left" );

	cmd( "pack .mm.i.f .mm.i.l .mm.i.c -padx 10 -pady 5 -side left" );
	cmd( "pack .mm.i" );

	cmd( "tooltip::tooltip .mm.i \"File, line and column of error\"" );

	cmd( "ttk::frame .mm.b" );

	cmd( "ttk::button .mm.b.perr -width [ expr { $butWid + 4 } ] -text \"Previous Error\" -underline 0 -command { \
			focus .mm.t.t; \
			set start \"$cerr linestart\"; \
			set errtemp [ .mm.t.t search -nocase -regexp -count errlen -backward -- $error $start 1.0];	 \
			if { [ string length $errtemp ] != 0 } { \
				set cerr $errtemp; \
				.mm.t.t mark set insert $errtemp; \
				.mm.t.t tag remove sel 1.0 end; \
				.mm.t.t tag add sel \"$errtemp linestart\" \"$errtemp lineend\"; \
				.mm.t.t see $errtemp; \
				set errdat [ split [ .mm.t.t get \"$errtemp linestart\" \"$errtemp lineend\" ] : ]; \
				if { [ string length [ lindex $errdat 0 ] ] == 1 } { \
					set errfil \"[ lindex $errdat 0 ]:[ lindex $errdat 1 ]\"; \
					set idxfil 2 \
				} else { \
					set errfil \"[ lindex $errdat 0 ]\"; \
					set idxfil 1 \
				}; \
				if { $errfil ne \"\" && [ llength $errdat ] > $idxfil && [ string is integer -strict [ lindex $errdat $idxfil ] ] } { \
					set errlin [ lindex $errdat $idxfil ] \
				} else { \
					set errlin	\"\" \
				}; \
				incr idxfil; \
				if { $errfil ne \"\" && [ llength $errdat ] > $idxfil && [ string is integer -strict [ lindex $errdat $idxfil ] ] } { \
					set errcol [ lindex $errdat $idxfil ] \
				} else { \
					set errcol \"\" \
				}; \
				.mm.i.f.n configure -text $errfil; \
				.mm.i.l.n configure -text $errlin; \
				.mm.i.c.n configure -text $errcol; \
			} \
		}" );
	cmd( "ttk::button .mm.b.gerr -width [ expr { $butWid + 4 } ] -text \"Go to Error\" -underline 0 -command { set choice 87 }" );
	cmd( "ttk::button .mm.b.ferr -width [ expr { $butWid + 4 } ] -text \"Next Error\" -underline 0 -command { \
			focus .mm.t.t; \
			if { ! [ string equal $cerr 1.0 ] } { \
				set start \"$cerr lineend\" \
			} else { \
				set start 1.0 \
			}; \
			set errtemp [ .mm.t.t search -nocase -regexp -count errlen -- $error $start end ]; \
			if { [ string length $errtemp ] != 0 } { \
				set cerr $errtemp; \
				.mm.t.t mark set insert \"$errtemp + $errlen ch\"; \
				.mm.t.t tag remove sel 1.0 end; \
				.mm.t.t tag add sel \"$errtemp linestart\" \"$errtemp lineend\"; \
				.mm.t.t see $errtemp; \
				set errdat [ split [ .mm.t.t get \"$errtemp linestart\" \"$errtemp lineend\" ] : ]; \
				if { [ string length [ lindex $errdat 0 ] ] == 1 } { \
					set errfil \"[ lindex $errdat 0 ]:[ lindex $errdat 1 ]\"; \
					set idxfil 2 \
				} else { \
					set errfil \"[ lindex $errdat 0 ]\"; \
					set idxfil 1 \
				}; \
				if { $errfil ne \"\" && [ llength $errdat ] > $idxfil && [ string is integer -strict [ lindex $errdat $idxfil ] ] } { \
					set errlin [ lindex $errdat $idxfil ] \
				} else { \
					set errlin	\"\" \
				}; \
				incr idxfil; \
				if { $errfil ne \"\" && [ llength $errdat ] > $idxfil && [ string is integer -strict [ lindex $errdat $idxfil ] ] } { \
					set errcol [ lindex $errdat $idxfil ] \
				} else { \
					set errcol \"\" \
				}; \
				.mm.i.f.n configure -text $errfil; \
				.mm.i.l.n configure -text $errlin; \
				.mm.i.c.n configure -text $errcol; \
			} \
		}" );
	cmd( "ttk::button .mm.b.close -width [ expr { $butWid + 4 } ] -text Done -underline 0 -command { unset -nocomplain errfil errlin errcol; destroytop .mm; focustop .f.t.t; set keepfocus 0 }" );
	cmd( "pack .mm.b.perr .mm.b.gerr .mm.b.ferr .mm.b.close -padx $butSpc -expand yes -fill x -side left" );
	cmd( "pack .mm.b -padx $butPad -pady $butPad -side right" );

	cmd( "tooltip::tooltip .mm.b.perr \"Show previous error line\"" );
	cmd( "tooltip::tooltip .mm.b.gerr \"Edit error line in LMM\"" );
	cmd( "tooltip::tooltip .mm.b.ferr \"Show next error line\"" );
	cmd( "tooltip::tooltip .mm.b.close \"Close this window\"" );

	cmd( "bind .mm <p> { .mm.b.perr invoke }; bind .mm <P> { .mm.b.perr invoke }" );
	cmd( "bind .mm.t.t <Up> { .mm.b.perr invoke; break }" );
	cmd( "bind .mm.t.t <Left> { .mm.b.perr invoke; break }" );
	cmd( "bind .mm <g> { .mm.b.gerr invoke }; bind .mm <G> { .mm.b.gerr invoke }" );
	cmd( "bind .mm <n> { .mm.b.ferr invoke }; bind .mm <N> { .mm.b.ferr invoke }" );
	cmd( "bind .mm.t.t <Down> { .mm.b.ferr invoke; break }" );
	cmd( "bind .mm.t.t <Right> { .mm.b.ferr invoke; break }" );
	cmd( "bind .mm <d> { .mm.b.close invoke }; bind .mm <D> { .mm.b.close invoke }" );
	cmd( "bind .mm.t.t <KeyPress-Return> { .mm.b.gerr invoke }" );
	cmd( "bind .mm <KeyPress-Escape> { .mm.b.close invoke }" );
	cmd( "bind .mm.b.perr <KeyPress-Return> { .mm.b.perr invoke }" );
	cmd( "bind .mm.b.gerr <KeyPress-Return> { .mm.b.gerr invoke }" );
	cmd( "bind .mm.b.ferr <KeyPress-Return> { .mm.b.ferr invoke }" );
	cmd( "bind .mm.b.close <KeyPress-Return> { .mm.b.close invoke }" );

	cmd( "showtop .mm lefttoW no no no" );
	cmd( "mousewarpto .mm.b.gerr 0" );

	cmd( "if [ file exists \"$modelDir/makemessage.txt\" ] { set file [ open \"$modelDir/makemessage.txt\" ]; .mm.t.t insert end [ read -nonewline $file ]; close $file } { .mm.t.t insert end \"(no compilation errors)\" }" );
	cmd( ".mm.t.t mark set insert \"1.0\"" );
	cmd( ".mm.b.ferr invoke" );

	cmd( ".mm.t.t configure -state disabled" );
	cmd( "focustop .mm.t.t" );
	cmd( "set keepfocus 1" );
	cmd( "update" );
}


/*************************************************************
 CLEAN_SPACES
 *************************************************************/
void gui::clean_spaces( char *s )
{
	int i, j, len;

	if ( s == NULL )
		return;

	len = strlen( s );
	char app[ len + 1 ];
	app[ len ] = '\0';

	for ( j = 0, i = 0; s[ i ] != '\0' && i < len; ++i )
		switch ( s[ i ] )
		{
			case ' ':
			case '\t':
				break;

			default:
				app[ j++ ] = s[ i ];
				break;
		}

	app[ j ] = '\0';
	lsd::strcpyn( s, app, len + 1 );
}


/*************************************************************
 WIN_PATH
 convert linux path to Windows default, replacing / with \
 *************************************************************/
std::string gui::win_path( std::string filepath )
{
	std::string winpath;

	for ( auto c : filepath )
		if ( c == '/' )
			winpath.push_back( '\\' );
		else
			winpath.push_back( c );

	return winpath;
}


/*************************************************************
 STRTCL
 convert a string to proper Tcl format
 *************************************************************/
char *gui::strtcl( char *out, const char *text, int outSz )
{
	int i, j;

	if ( out == NULL )
		return NULL;

	if ( text == NULL )
		j = 0;
	else
	{
		for ( i = j = 0; text[ i ] != '\0' && j < outSz - 1; ++i )
		{
			if ( text[ i ] == '\r' && text[ i + 1 ] == '\n' )
				continue;				// convert CR-LF to LF

			if ( text[ i ] != '[' && text[ i ] != ']' && text[ i ] != '{' && text[ i ] != '}' && text[ i ] != '\"' && text[ i ] != '\\' && text[ i ] != '$' )
				out[ j++ ] = text[ i ];
			else
			{
				out[ j++ ] = '\\';
				out[ j++ ] = text[ i ];
			}
		}

		for ( i = 1; i <= j && isspace( ( unsigned char ) out[ j - i ] ); ++i )
			out[ j - i ] = '\0';
	}

	out[ j ] = '\0';

	return out;
}
