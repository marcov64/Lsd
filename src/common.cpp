/*************************************************************

	LSD 8.0 - May 2022
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 COMMON.CPP
 Code common between LMM and LSD Browser.

 Relevant flags (when defined):

 - _LMM_: Model Manager executable
 - _FUN_: user model equation file
 - _NW_: No Window executable
 - _NP_: no parallel (multi-task) processing
 - _NT_: no signal trapping (better when debugging in GDB)
 *************************************************************/

#include "common.h"

// Tcl/Tk-dependent modules
#ifndef _NW_

/*********************************
 LOAD_LMM_OPTIONS
 *********************************/
bool load_lmm_options( void )
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


/*********************************
 UPDATE_LMM_OPTIONS
 *********************************/
void update_lmm_options( bool justLmmGeom )
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


/*********************************
 LOAD_MODEL_INFO
 *********************************/
bool load_model_info( const char *path )
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


/*********************************
 UPDATE_MODEL_INFO
 *********************************/
void update_model_info( bool fix )
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
		}", model_defaults[ 0 ], equation_name );

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


/****************************************************
 INIT_TCL_TK
 initializes the Tcl/Tk environment
 ****************************************************/
void init_tcl_tk( const char *exec, const char *tcl_app_name )
{
	int num, res;

	if ( ! set_env( true ) )
	{
		log_tcl_error( false, "Set environment variables", "Environment variable setup failed, Tcl/Tk may be unavailable" );
		myexit( 2 );
	}

	// initialize the tcl/tk interpreter
	Tcl_FindExecutable( exec );
	inter = Tcl_CreateInterp( );
	num = Tcl_Init( inter );
	if ( num != TCL_OK )
	{
		log_tcl_error( false, "Create Tcl interpreter", "Tcl initialization directories not found, check the Tcl/Tk installation  and configuration or reinstall LSD\nTcl Error = %d : %s", num,  Tcl_GetStringResult( inter ) );
		myexit( 3 );
	}

	// set variables and links in TCL interpreter
	Tcl_SetVar( inter, "_LSD_VERSION_", _LSD_VERSION_, 0 );
	Tcl_SetVar( inter, "_LSD_DATE_", _LSD_DATE_, 0 );
	Tcl_LinkVar( inter, "res", ( char * ) &res, TCL_LINK_INT );

	// test Tcl interpreter
	cmd( "set res 1234567890" );
	Tcl_UpdateLinkedVar( inter, "res" );
	if ( res != 1234567890 )
	{
		log_tcl_error( false, "Test Tcl", "Tcl failed, check the Tcl/Tk installation and configuration or reinstall LSD" );
		myexit( 3 );
	}

	// initialize & test the tk application
	num = Tk_Init( inter );
	if ( num == TCL_OK )
		cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { set res 0 } { set res 1 }" );

	if ( num != TCL_OK || res )
	{
		log_tcl_error( false, "Start Tk", "Tk failed, check the Tcl/Tk installation (version 8.6+) and configuration or reinstall LSD\nTcl Error = %d : %s", num,  Tcl_GetStringResult( inter ) );
		myexit( 3 );
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
		myexit( 4 );
	}

	Tcl_UnlinkVar( inter, "res" );
}


/****************************************************
 SET_ENV
 sets the required environment variables
 it does nothing if the variables already exist
 and Windows PATH has a compiler in it
 ****************************************************/
bool set_env( bool set )
{
	bool res = true;
	char *lsd_root, *exec_path = NULL;
	static char *lsd_root_env = NULL, *tcl_lib_env = NULL, *path_env = NULL;

	if ( set )
	{
		lsd_root = getenv( "LSDROOT" );

		if ( lsd_root == NULL )
		{
			exec_path = new char[ MAX_PATH_LENGTH ];
			exec_path = getcwd( exec_path, MAX_PATH_LENGTH );
			exec_path = clean_path( exec_path );

			lsd_root = search_lsd_root( exec_path );

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
			lsd_root = clean_path( lsd_root );

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
				if ( run_system( TCL_FIND_EXE ) != 0 )
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
		delete [ ] exec_path;
	}
	else
	{
		delete [ ] tcl_lib_env;
		delete [ ] lsd_root_env;
		delete [ ] path_env;
	}

	return res;
}


/****************************************************
 SEARCH_LSD_ROOT
 searches LSD root directory upwards to the root
 ****************************************************/
char *search_lsd_root( char *start_path )
{
	bool miss;
	const char *files[ ] = LSD_MIN_FILES;
	char *file, *cur_dir, *last_dir, *orig_dir, *found = NULL;
	int i, st;
	struct stat info;

	cur_dir = new char[ MAX_PATH_LENGTH ];
	last_dir = new char[ MAX_PATH_LENGTH ];
	orig_dir = new char[ MAX_PATH_LENGTH ];
	orig_dir = getcwd( orig_dir, MAX_PATH_LENGTH );

	if ( orig_dir == NULL )
		goto err;

	if ( chdir( start_path ) )
		goto end;

	strcpy( last_dir, "" );

	do
	{
		cur_dir = getcwd( cur_dir, MAX_PATH_LENGTH );
		cur_dir = clean_path( cur_dir );

		if ( cur_dir == NULL || ! strcmp( cur_dir, last_dir ) )
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
			strcpyn( start_path, cur_dir, strlen( start_path ) + 1 );
			found = start_path;
			break;
		}

		strcpyn( last_dir, cur_dir, MAX_PATH_LENGTH );
	}
	while ( ! chdir( ".." ) );

	end:
	chdir( orig_dir );

	err:
	delete [ ] cur_dir;
	delete [ ] last_dir;
	delete [ ] orig_dir;

	return found;
}


/****************************************************
 CMD
 ****************************************************/
void cmd( const char *cm, ... )
{
	static bool bufdyn;
	static char *buffer, bufstat[ MAX_BUFF_SIZE ];
	static int reqsz, sz;
	static va_list argptr;

#ifndef _NP_
	// abort if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
		return;
#endif

	// abort if Tcl interpreter not initialized
	if ( inter == NULL )
	{
		fprintf( stderr, "\nTcl interpreter not initialized. Quitting LSD now.\n" );
		myexit( 24 );
	}

	buffer = bufstat;
	va_start( argptr, cm );
	reqsz = vsnprintf( buffer, MAX_BUFF_SIZE, cm, argptr );
	va_end( argptr );

	if ( reqsz < 0 )
	{
		log_tcl_error( true, "Invalid Tcl command", "Cannot expand command '%s...'", cm );
		return;
	}

	// handle very large commands
	if ( reqsz >= MAX_BUFF_SIZE )
	{
		buffer = new char[ reqsz + 1 ];
		va_start( argptr, cm );
		sz = vsnprintf( buffer, reqsz + 1, cm, argptr );
		va_end( argptr );

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

	if ( Tcl_Eval( inter, buffer ) != TCL_OK )
		log_tcl_error( true, cm, Tcl_GetStringResult( inter ) );

	if ( bufdyn )
		delete [ ] buffer;
}


/****************************************************
 LOG_TCL_ERROR
 ****************************************************/
void log_tcl_error( bool show, const char *cm, const char *message, ... )
{
	static char *err_path, ftime[ 80 ], fname[ MAX_PATH_LENGTH ], buffer[ MAX_BUFF_SIZE ];
	static struct tm *timeinfo;
	static time_t rawtime;
	static va_list argptr;
	static FILE *f;

	static bool firstCall = true;

#ifndef _NP_
	// abort if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
		return;
#endif

	va_start( argptr, message );
	vsnprintf( buffer, MAX_BUFF_SIZE, message, argptr );
	va_end( argptr );

#ifdef _LMM_
	err_path = rootLsd;
#else
	err_path = exec_path;
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


/****************************************************
 TCL_LOG_TCL_ERROR
 Entry point function for access from the Tcl interpreter
 ****************************************************/
int Tcl_log_tcl_error( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	if ( argc != 4 || argv[ 1 ] == NULL || argv[ 2 ] == NULL || argv[ 3 ] == NULL )	// require 3 parameters
		return TCL_ERROR;

	log_tcl_error( strcmp( argv[ 1 ], "0" ), argv[ 2 ], argv[ 3 ] );

	static char empty[ ] = "";
	Tcl_SetResult( inter, empty, TCL_VOLATILE );
	return TCL_OK;
}


/****************************************************
 SHOW_TCL_ERROR
 Show the error message to the user
 ****************************************************/
void show_tcl_error( const char *boxTitle, const char *errMsg, ... )
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


/****************************************************
 TCL_DISCARD_CHANGE
 Entry point function for access from the Tcl interpreter
 ****************************************************/
int Tcl_discard_change( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	if ( discard_change( ) == 1 )
		Tcl_SetResult( inter, ( char * ) "ok", TCL_VOLATILE );
	else
		Tcl_SetResult( inter, ( char * ) "cancel", TCL_VOLATILE );
	return TCL_OK;
}


/****************************************************
 VALID_LABEL
 ****************************************************/
bool valid_label( const char *lab )
{
	cmd( "if [ regexp {^[a-zA-Z_][a-zA-Z0-9_]*$} \"%s\" ] { set res 1 } { set res 0 }", lab );
	return get_bool( "res" );
}


/****************************************************
 EXISTS_VAR
 ****************************************************/
bool exists_var( const char *lab )
{
	cmd( "set res [ info exists \"%s\" ]", lab );
	return get_bool( "res" );
}


/****************************************************
 EXISTS_WINDOW
 ****************************************************/
bool exists_window( const char *lab )
{
	cmd( "set res [ winfo exists \"%s\" ]", lab );
	return get_bool( "res" );
}


/***************************************************
 GET_BOOL
 Set var to NULL to just get the Tcl value
 ***************************************************/
bool get_bool( const char *tcl_var, bool *var )
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

	if ( Tcl_GetBoolean( inter, strvar, & intvar ) != TCL_OK )
		if ( Tcl_GetInt( inter, strvar, & intvar ) != TCL_OK )
		{
			log_tcl_error( true, "Cannot convert to boolean", "Internal LSD error converting variable '%s' containing '%s'. If the problem persists, please contact developers", tcl_var, strvar );
			return false;
		}

	if ( var != NULL )
		*var = intvar ? true : false;

	return intvar ? true : false;
}


/***************************************************
 GET_INT
 Set var to NULL to just get the Tcl value
 ***************************************************/
int get_int( const char *tcl_var, int *var )
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

	if ( Tcl_GetInt( inter, strvar, & intvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot convert to integer", "Internal LSD error converting variable '%s' containing '%s'. If the problem persists, please contact developers", tcl_var, strvar );
		return 0;
	}

	if ( var != NULL )
		*var = intvar;

	return intvar;
}


/***************************************************
 GET_LONG
 Set var to NULL to just get the Tcl value
 ***************************************************/
long get_long( const char *tcl_var, long *var )
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


/***************************************************
 GET_DOUBLE
 Set var to NULL to just get the Tcl value
 ***************************************************/
double get_double( const char *tcl_var, double *var )
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

	if ( Tcl_GetDouble( inter, strvar, & dblvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot convert to double", "Internal LSD error converting variable '%s' containing '%s'. If the problem persists, please contact developers", tcl_var, strvar );
		return NAN;
	}

	if ( var != NULL )
		*var = dblvar;

	return dblvar;
}


/***************************************************
 GET_STR
 Set var to NULL to just get the Tcl pointer
 ***************************************************/
char *get_str( const char *tcl_var, char *var, int var_size )
{
	const char *strvar = Tcl_GetVar( inter, tcl_var, 0 );

	if ( strvar == NULL )
	{
		log_tcl_error( true, "Invalid Tcl variable name", "Internal LSD error searching for variable '%s'. If the problem persists, please contact developers", tcl_var );
		return var;
	}

	if ( var != NULL && var_size > 0 )
	{
		strcpyn( var, strvar, var_size );
		return var;
	}
	else
		return ( char * ) strvar;
}

const char *get_str( const char *tcl_var )
{
	return ( const char * ) get_str( tcl_var, NULL, 0 );
}


/***************************************************
 EQ_STR
 Compare if Tcl expression, evaluating it
 before comparison, is equal to C string
 ***************************************************/
bool expr_eq( const char *tcl_exp, const char *c_str )
{
	const char *strvar = eval_str( tcl_exp );

	if ( strvar != NULL && c_str != NULL )
		return strcmp( strvar, c_str ) == 0;
	else
		return false;
}


/***************************************************
 EVAL_STR
 Evaluate Tcl expression to C string
 ATTENTION: if var is NULL, the returned result
 string buffer is valid only until next Tcl invocation
 ***************************************************/
char *eval_str( const char *tcl_exp, char *var, int var_size )
{
	if ( Tcl_ExprString( inter, tcl_exp ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot evaluate to string", "Internal LSD error evaluating expression '%s'. If the problem persists, please contact developers", tcl_exp );
		return var;
	}

	if ( var != NULL && var_size > 0 )
	{
		strcpyn( var, Tcl_GetStringResult( inter ), var_size );
		return var;
	}
	else
		return ( char * ) Tcl_GetStringResult( inter );
}

const char *eval_str( const char *tcl_exp )
{
	return ( const char * ) eval_str( tcl_exp, NULL, 0 );
}


/***************************************************
 EVAL_BOOL
 Evaluate Tcl expression to C boolean
 ***************************************************/
bool eval_bool( const char *tcl_exp )
{
	int intvar;
	long longvar;

	if ( Tcl_ExprBoolean( inter, tcl_exp, & intvar ) == TCL_OK )
		return intvar ? true : false;

	if ( Tcl_ExprLong( inter, tcl_exp, & longvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot evaluate to boolean", "Internal LSD error evaluating expression '%s'. If the problem persists, please contact developers", tcl_exp );
		return false;
	}

	return longvar ? true : false;
}


/***************************************************
 EVAL_INT
 Evaluate Tcl expression to C integer
 ***************************************************/
int eval_int( const char *tcl_exp )
{
	return ( int ) eval_long( tcl_exp );
}


/***************************************************
 EVAL_LONG
 Evaluate Tcl expression to C long
 ***************************************************/
long eval_long( const char *tcl_exp )
{
	long longvar;

	if ( Tcl_ExprLong( inter, tcl_exp, & longvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot evaluate to long integer", "Internal LSD error evaluating expression '%s'. If the problem persists, please contact developers", tcl_exp );
		return 0;
	}

	return longvar;
}


/***************************************************
 EVAL_DOUBLE
 Evaluate Tcl expression to C double
 ***************************************************/
double eval_double( const char *tcl_exp )
{
	double dblvar;

	if ( Tcl_ExprDouble( inter, tcl_exp, & dblvar ) != TCL_OK )
	{
		log_tcl_error( true, "Cannot evaluate to double", "Internal LSD error evaluating expression '%s'. If the problem persists, please contact developers", tcl_exp );
		return NAN;
	}

	return dblvar;
}


/*********************************
 CHECK_OPTION_FILES
 check if model ans system option
 files exist and create them if not
 *********************************/
void check_option_files( bool sys )
{
	if ( ! sys && ! eval_bool( "[ file exists \"$modelDir/$MODEL_OPTIONS\" ]" ) && eval_bool( "$modelDir ne \"\"" ) && eval_bool( "$modelDir ne $RootLsd" ) )
	{
		cmd( "set dir [ glob -nocomplain \"$modelDir/fun_*.cpp\" ]" );
		cmd( "if { $dir ne \"\" } { set b [ file tail [ lindex $dir 0 ] ] } { set b \"fun_UNKNOWN.cpp\" }" );
		cmd( "set a \"# LSD options\nTARGET=$DefaultExe\nFUN=[ file rootname \"$b\" ]\n\n# Additional model files\nFUN_EXTRA=\n\n# Compiler options\nSWITCH_CC=-O0 -ggdb3\nSWITCH_CC_LNK=\"" );
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


/*********************************
 GET_FUN_NAME
 get current equation file name
 *********************************/
const char *get_fun_name( char *str, int str_sz, bool nw )
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


/*********************************
 USE_EIGEN
 detect if Eigen library is in use
 *********************************/
bool use_eigen( void )
{
	bool nfound = true;
	char full_name[ MAX_PATH_LENGTH ], buf[ 2 * MAX_PATH_LENGTH ];
	const char *fun_file, *path;
	FILE *f;

	path = get_str( "modelDir" );
	fun_file = get_fun_name( buf, 2 * MAX_PATH_LENGTH, true );

	if( path == NULL || fun_file == NULL )
		return false;

	snprintf( full_name, MAX_PATH_LENGTH, "%s/%s", path, fun_file );
	f = fopen( full_name, "r" );
	if( f == NULL )
		return false;

	while ( fgets( buf, 2 * MAX_PATH_LENGTH, f ) != NULL &&
			( nfound = strncmp( buf, EIGEN, strlen( EIGEN ) ) ) );

	fclose( f );

	if ( nfound )
		return false;

	return true;
}


/****************************************************
 MAKE_NO_WINDOW
 create a no-window command-line version of LSD
 ****************************************************/
const char *lsd_nw_src[ LSD_NW_NUM ] = LSD_NW_SRC;

bool make_no_window( void )
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
	cmd( "if { ! [ file exists \"$modelDir/$LsdSrc\" ] } { file mkdir \"$modelDir/$LsdSrc\" }" );

	for ( i = 0; i < LSD_NW_NUM; ++i )
		cmd( "file copy -force \"$RootLsd/$LsdSrc/%s\" \"$modelDir/$LsdSrc\"", lsd_nw_src[ i ] );

	// copy Eigen library files if in use, just once to save time
	if( use_eigen( ) )
		cmd( "if { ! [ file exists \"$modelDir/$LsdSrc/Eigen\" ] } { file copy -force \"$RootLsd/$LsdSrc/Eigen\" \"$modelDir/$LsdSrc\" }" );

	// create makefileNW and compile a local machine version of lsdNW
	return compile_run( false, true );
}


/*********************************
 MAKE_MAKEFILE
 create makefiles to compile LSD
 *********************************/
void make_makefile( bool nw )
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


/*********************************
 COMPILE_RUN
 compile LSD, GUI or command line
 and optionally execute it
 *********************************/
bool compile_run( int run_mode, bool nw )
{
	bool ret = false;
	char str[ 2 * MAX_PATH_LENGTH ];
	const char *s;
	int res, max_threads = 1;
	FILE *f;

	Tcl_LinkVar( inter, "res", ( char * ) &res, TCL_LINK_INT );

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

	if ( run_mode == 0 && ! nw )// delete existing object file if it's just compiling
	{							// to force recompilation
		cmd( "set oldObj \"[ file rootname [ lindex [ glob -nocomplain fun_*.cpp ] 0 ] ].o\"" );
		cmd( "if { [ file exists \"$oldObj\" ] } { file delete \"$oldObj\" }" );
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

	// get target exec name
	cmd( "set fapp [ file nativename \"$modelDir/makefile%s\" ]", nw ? "NW" : "" );
	f = fopen( get_str( "fapp" ), "r" );
	fscanf( f, "%1999s", str );
	while ( strncmp( str, "TARGET=", 7 ) && fscanf( f, "%1999s", str ) != EOF );
	fclose( f );
	if ( strncmp( str, "TARGET=", 7 ) != 0 )
	{
		cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Makefile%s corrupted\" -detail \"Check 'Model Options' and 'System Options' in LMM menu 'Model'.\"", nw ? "NW" : "" );
		goto end;
	}

	if ( nw )
	{
		cmd( "set mainExe %s", str + 7 );
		strcpy( str, "TARGET=lsdNW" );		// NW version use fixed name because of batches
	}

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
	max_threads = thread::hardware_concurrency( );

	// start compilation as a background task
	res = -1;
	cmd( "make_background %s %d %d %d", str + 7, max_threads, nw, true );

	// loop to wait compilation to finish or be aborted
	while ( res < 0 )
		Tcl_DoOneEvent( 0 );

	// close banner
	cmd( "destroytop .t" );
	Tcl_UnlinkVar( inter, "res" );

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
							cmd( "while { [ catch { exec -- ./%s & } result ] && $n > 0 } { incr n -1; after 50 }", str + 7 );
							break;

						case _MAC_:
							cmd( "while { [ catch { exec -- open -F -n ./%s.app & } result ] && $n > 0 } { incr n -1; after 50 }", str + 7 );
							break;

						case _WIN_:
							cmd( "while { [ catch { exec -- %s.exe & } result ] && $n > 0 } { incr n -1; after 50 }", str + 7 );
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

#else

void cmd( const char *cm, ... ) { }

#endif


/*************************************
 MAIN
 *************************************/
int main( int argn, const char **argv )
{
	int res = 0;

#ifndef _NT_
	// register all signal handlers
	handle_signals( signal_handler );

	try
	{
#endif
		res = lsdmain( argn, argv );

#ifndef _NT_
	}
	catch ( bad_alloc& exc )	// out of memory conditions
	{
		exception_handler( SIGMEM, exc.what( ) );
	}
	catch ( exception& exc )	// other known error conditions
	{
		exception_handler( SIGSTL, exc.what( ) );
	}
	catch ( ... )				// other unknown error conditions
	{
		abort( );				// raises a SIGABRT exception, tell user & close
	}

#endif

	myexit( res );
	return res;
}


#ifdef _WIN32

/****************************************************
 RUN_SYSTEM (Windows)
 executes run command in system without opening
 command-prompt window or activating STL mutexes
 spaces in path/file names are not supported
 ****************************************************/
int run_system( const char *cmd, int id )
{
	PROCESS_INFORMATION p_info;
	STARTUPINFO s_info;
	DWORD res;
	LPSTR c_line;

	memset( &s_info, 0, sizeof s_info );
	memset( &p_info, 0, sizeof p_info );
	s_info.cb = sizeof s_info;

	c_line = ( LPSTR ) malloc( strlen( cmd ) + 1 );
	strcpy( c_line, cmd );

	if ( ! CreateProcess( NULL, c_line, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, & s_info, & p_info ) )
	{
		free( c_line );
		return -1;
	}

#ifndef _NP_

	if ( id >= 0 && id < ( int ) run_pids.size( ) )
	{

		lock_guard < mutex > lock( lock_run_pids );
		run_pids[ id ] = p_info.hProcess;
	}

#endif

	WaitForSingleObject( p_info.hProcess, INFINITE );
	GetExitCodeProcess( p_info.hProcess, & res );
	CloseHandle( p_info.hProcess );
	CloseHandle( p_info.hThread );

	free( c_line );
	return res;
}


/****************************************************
 KILL_SYSTEM (Windows)
 stops a running command in system
 ****************************************************/
int kill_system( int id )
{

#ifndef _NP_

	DWORD res;

	if ( id >= 0 && id < ( int ) run_pids.size( ) &&
		 GetExitCodeProcess( run_pids[ id ], & res ) &&
		 res == STILL_ACTIVE &&
		 ! TerminateProcess( run_pids[ id ], 15 ) )
			return 0;

#endif

	return 1;
}

#else

extern char ** environ;

/****************************************************
 RUN_SYSTEM (Unix)
 executes run command in system without opening
 command-prompt window or activating STL mutexes
 spaces in path/file names are not supported
 ****************************************************/
int run_system( const char *cmd, int id )
{
	char **argv, **envp;
	int res;
	pid_t pid;
	wordexp_t p;

	if ( wordexp( cmd, & p, 0 ) != 0 )
		return -1;

	argv = p.we_wordv;
	envp = environ;

	pid = fork( );
	if ( pid == -1 )
	{
		wordfree( & p );
		return -1;
	}

	if ( pid == 0 )
	{
		execve( argv[ 0 ], argv, envp );
		exit ( errno );
	}
	else
	{

#ifndef _NP_

		if ( id >= 0 && id < ( int ) run_pids.size( ) )
		{
			lock_guard < mutex > lock( lock_run_pids );
			run_pids[ id ] = pid;
		}

#endif

		waitpid( pid, & res, 0 );
		wordfree( & p );

		if ( res == 0 )
			return 0;
		else
			return WEXITSTATUS( res );
	}
}


/****************************************************
 KILL_SYSTEM (Unix)
 stops a running command in system
 ****************************************************/
#define WAIT_TSECS 10
int kill_system( int id )
{

#ifndef _NP_

	int res, tsecs = 0;

	if ( id >= 0 && id < ( int ) run_pids.size( ) )
	{
		if ( kill( run_pids[ id ], SIGKILL ) == 0 )
		{
			while ( ( res = kill( run_pids[ id ], 0 ) ) == 0 &&
					tsecs++ < WAIT_TSECS )
				msleep( 100 );

			if ( res == 0 )
				return 0;
		}
		else
			if ( errno != ESRCH )
				return 0;
	}

#endif

	return 1;
}

#endif

/****************************************************
 CLEAN_FILE
 remove any path prefixes to filename, if present
 ****************************************************/
char *clean_file( const char *filename )
{
	if ( filename != NULL )
	{
		if ( strchr( filename, '/' ) != NULL )
			return ( char * ) strrchr( filename, '/' ) + 1;

		if ( strchr( filename, '\\' ) != NULL )
			return ( char * ) strrchr( filename, '\\' ) + 1;
	}

	return ( char * ) filename;
}


/****************************************************
 CLEAN_PATH
 remove cygwin/MSYS path prefixes, if present, and replace \ with /
 ****************************************************/
char *clean_path( char *filepath )
{
	int i, drvpos, pathpos;
	const int npref = 5;
	const char *pref[ npref ] = { "/cygdrive/", "/c/", "/d/", "/e/", "/f/" };

	if ( filepath == NULL )
		return NULL;

	char temp[ strlen( filepath ) + 1 ];
	strcpy( temp, "" );

	for ( i = 0; i < npref && strncmp( filepath, pref[ i ], strlen( pref[ i ] ) ); ++i );

	if ( i < npref )
	{
		if ( i == 0 )	// Cygwin
		{
			drvpos = strlen( pref[ i ] );				// drive letter position
			pathpos = drvpos + 1;						// path start
		}
		else			// MSYS
		{
			drvpos = 1;									// drive letter position
			pathpos = 2;								// path start
		}

		temp[ 0 ] = toupper( filepath[ drvpos ] );		// copy drive letter
		temp[ 1 ] = ':';								// insert ':' drive separator
		strcpyn( temp + 2, filepath + pathpos, strlen( filepath ) - 1 );
		strcpyn( filepath, temp, strlen( filepath ) + 1 );
	}

	for ( i = 0; i < ( int ) strlen( filepath ); ++i )
		if ( filepath[ i ] == '\\' )					// replace \ with /
			filepath[ i ] = '/';

	return filepath;
}


/***************************************************
 CLEAN_SPACES
 ***************************************************/
void clean_spaces( char *s )
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
	strcpyn( s, app, len + 1 );
}


/****************************************************
 WIN_PATH
 convert linux path to Windows default, replacing / with \
 ****************************************************/
string win_path( string filepath )
{
	string winpath;

	for ( auto c : filepath )
		if ( c == '/' )
			winpath.push_back( '\\' );
		else
			winpath.push_back( c );

	return winpath;
}


/****************************************************
 STRCATN
 Concatenate strings respecting total size of first one
 ****************************************************/
char *strcatn( char *d, const char *s, size_t dSz )
{
	if ( dSz <= 0 || d == NULL || strlen( d ) >= dSz - 1 || s == NULL || strlen( s ) == 0 )
		return d;

	return strncat( d, s, dSz - strlen( d ) - 1 );
}


/****************************************************
 STRCPYN
 Copy string respecting total size of destination one
 ****************************************************/
char *strcpyn( char *d, const char *s, size_t dSz )
{
	if ( dSz <= 0 || d == NULL || s == NULL )
		return d;

	if ( strlen( s ) > dSz - 1 )
	{
		strncpy( d, s, dSz - 1 );
		d[ dSz - 1 ] = '\0';
	}
	else
		strcpy( d, s );

	return d;
}


/****************************************************
 STRUPR
 convert string to upper case
 ****************************************************/
#ifndef GCCLIBS
char *strupr( char *s )
{
	if ( s == NULL )
		return NULL;

	for ( unsigned char *p = ( unsigned char * ) s; *p; ++p )
		*p = toupper( *p );

	return s;
}
#endif


/***************************************************
 STRWSP
 check for a string of just whitespace
 ***************************************************/
bool strwsp( const char *str )
{
	if ( str == NULL )
		return true;

	while ( isspace( ( unsigned char ) *str ) )
		++str;

	if ( *str == '\0' )			// all spaces?
		return true;

	return false;
}


/***************************************************
 STRCLN
 trim whitespace from the beginning/end of string
 and convert line ends to LF only (unix-like)
 ***************************************************/
int strcln( char *out, const char *str, int outSz )
{
	char buf[ strlen( str ) + 1 ];
	strlf( buf, str, strlen( str ) + 1 );
	return strtrim( out, buf, outSz );
}


/***************************************************
 STRLF
 replace CR-LF pairs with LF only on C string
 ***************************************************/
int strlf( char *out, const char *str, int outSz )
{
	int i, j;

	for ( i = j = 0; str[ j ] != '\0' && i < outSz - 1; ++j )
		if ( str[ j ] == '\r' )
		{
			if ( str[ j + 1 ] != '\n' )
				out[ i++ ] = '\n';
		}
		else
			out[ i++ ] = str[ j ];

	out[ i ] = '\0';

	return i;
}

/***************************************************
 STRTRIM
 trim whitespace from the beginning/end of string
 ***************************************************/
int strtrim( char *out, const char *str, int outSz )
{
	char *end;
	int size;

	if ( str == NULL || outSz <= 0 )
		return 0;

	while ( isspace( ( unsigned char ) *str ) )
		++str;

	if ( *str == '\0' )			// all spaces?
	{
		out[ 0 ] = '\0';
		return 1;
	}

	end = ( char * ) str + strlen( str ) - 1;
	while ( end > str && isspace( ( unsigned char ) *end ) )
		--end;
	++end;

	size = ( end - str ) < outSz ? ( end - str ) : outSz - 1;

	memcpy( out, str, size );
	out[ size ] = '\0';

	return size;
}


/***************************************************
 STRWRAP
 insert line breaks in string to wrap text at given width
 based on code from ulf.astrom@gmail.com
 ***************************************************/
int strwrap( char *out, const char *str, int outSz, int wid )
{
	int i, lines, tlen, len, pos, close_word, open_word;

	if ( str == NULL )
		return 0;

	tlen = strlen( str );

	if ( tlen == 0 || wid <= 0 || outSz <= 0 )
		return 0;

	lines = pos = 0;
	while ( pos < tlen && outSz > 0 )
	{
		if ( str[ pos ] == '\n' )
		{
			len = 0;
			goto end_line;
		}

		if ( pos + wid > tlen )
			wid = tlen - pos;

		len = wid;
		close_word = 0;
		while ( str[ pos + len + close_word ] != '\0' && ! isspace( str[ pos + len + close_word ] ) )
			++close_word;

		open_word = 0;
		while ( str[ pos + len ] != '\0' && ! isspace( str[ pos + len ] ) )
		{
			--len;
			++open_word;

			if ( open_word + close_word > wid * 0.8 )
			{
				len = wid;
				break;
			}
		}

		for ( i = 0; i < len; ++i )
		{
			if ( str[ pos + i ] == '\n' )
			{
				len = i;
				break;
			}
		}

		end_line:

		++lines;

		if ( len > outSz - 1 )
			len = outSz - 1;

		if ( len > 0 )
		{
			strncpy( out, str + pos, len );
			out += len;
			outSz -= len;
		}

		out[ 0 ] = '\n';
		++out;
		--outSz;

		if ( len == wid )
			--len;

		pos += len + 1;
	}

	*( out - 1 ) = '\0';

	return lines;
}


/***************************************************
 STRTCL
 convert a string to proper Tcl format
 ***************************************************/
char *strtcl( char *out, const char *text, int outSz )
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


/****************************************************
 MSLEEP
 stop execution for a given period
 ****************************************************/
void msleep( unsigned msec )
{
#ifdef _WIN32
	Sleep( msec );
#else
	usleep( msec * 1000 );
#endif
	return;
}


/****************************************************
 MYEXIT
 exit LSD
 ****************************************************/
void myexit( int v )
{
	fflush( stderr );
#ifndef _NP_
	// stop multi-thread workers, if needed/safe
	if ( worker_errors( ) == 0 )
		delete [ ] workers;
#endif

#ifndef _NW_
	if ( inter != NULL )
	{
		if ( tk_ok )
			cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { catch { destroy . } }" );

		Tcl_Finalize( );
	}
#endif
	exit( v );
}


/****************************************************
 SIGNAL_NAME
 ****************************************************/
const char *signal_name( int signum )
{
	int i;
	for ( i = 0; i < REG_SIG_NUM && signals[ i ] != signum; ++i );
	if ( i == REG_SIG_NUM )
		return "Unknow exception";
	return signal_names[ i ];
}


/****************************************************
 HANDLE_SIGNALS
 ****************************************************/
void handle_signals( void ( * handler )( int signum ) )
{
	for ( int i = 0; i < REG_SIG_NUM; ++i )
		signal( signals[ i ], handler );
}


/****************************************************
 SIGNAL_HANDLER
 handle critical system signals
 ****************************************************/
void signal_handler( int signum )
{
	exception_handler( signum, NULL );
}


/****************************************************
 EXCEPTION_HANDLER
 handle exceptions and system signals
 ****************************************************/
void exception_handler( int signum, const char *what )
{
	static char msg1[ MAX_LINE_SIZE ], msg2[ MAX_LINE_SIZE ], msg3[ MAX_LINE_SIZE ];

	switch ( signum )
	{
		case SIGINT:
		case SIGTERM:
#ifdef _NW_
			snprintf( msg1, MAX_LINE_SIZE, "SIGINT/SIGTERM (%s)", signal_name( signum ) );
			break;
#else
			cmd( "set choice 1" );	// regular quit (checking for save)
			return;
#endif
#ifdef SIGWINCH
		case SIGWINCH:
#ifndef _NW_
			cmd( "sizetop all" );	// readjust windows size/positions
			cmd( "update" );
#endif
			return;
#endif
		case SIGSTL:
			snprintf( msg1, MAX_LINE_SIZE, "SIGSTL (%s)", what != NULL && strlen( what ) > 0 ? what : "STL exception" );
			strcpyn( msg2, "Maybe an invalid math or data operation?\n	Check your standard C++ library calls' arguments", MAX_LINE_SIZE );
			break;

		case SIGMEM:
			snprintf( msg1, MAX_LINE_SIZE, "SIGMEM (%s)", what != NULL && strlen( what ) > 0 ? what : "Out of memory" );
			strcpyn( msg2, "Maybe too many series saved?\n	Try to reduce the number of series saved or the number of cases (time steps)", MAX_LINE_SIZE );
			break;

		case SIGABRT:
			snprintf( msg1, MAX_LINE_SIZE, "SIGABRT (%s)", signal_name( signum ) );
			strcpyn( msg2, "Maybe an invalid call to library or Tcl/Tk?", MAX_LINE_SIZE );
			break;

		case SIGFPE:
			snprintf( msg1, MAX_LINE_SIZE, "SIGFPE (%s)", signal_name( signum ) );
			strcpyn( msg2, "Maybe a division by 0 or similar?", MAX_LINE_SIZE );
		break;

		case SIGILL:
			snprintf( msg1, MAX_LINE_SIZE, "SIGILL (%s)", signal_name( signum ) );
			strcpyn( msg2, "Maybe executing data?", MAX_LINE_SIZE );
		break;

		case SIGSEGV:
			snprintf( msg1, MAX_LINE_SIZE, "SIGSEGV (%s)", signal_name( signum ) );
			strcpyn( msg2, "Maybe an invalid pointer?\n	 Also ensure no group of objects has zero elements.", MAX_LINE_SIZE );
		break;
		default:
			snprintf( msg1, MAX_LINE_SIZE, "Unknown signal (%s)", signal_name( signum ) );
			strcpy( msg2, "" );
	}

#ifndef _NW_

#ifndef _LMM_
	if ( ! user_exception )
#endif
	{
		strcpyn( msg2, "There is an internal LSD error\n  If error persists, please contact developers", MAX_LINE_SIZE );
		strcpyn( msg3, "LSD will close now...", MAX_LINE_SIZE );
	}
#ifndef _LMM_
	else
	{
		strcpyn( msg3, "Additional information may be obtained running the simulation using the 'Model'/'GDB Debugger' menu option", MAX_LINE_SIZE );
		if ( quit != 2 )
		{
			if ( ! parallel_mode && fast_mode == 0 && stacklog != NULL &&
				 stacklog->vs != NULL && stacklog->vs->label != NULL )
			{
				strcatn( msg3, "\n\nAttempting to open the LSD Debugger.\n\nLSD will close immediately after exiting the Debugger.", MAX_LINE_SIZE );
				plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s'", stacklog->vs->label );
				print_stack( );
			}
			else
			{
				strcatn( msg3, "\n\nPlease disable fast mode and parallel processing to get more information about the error.\n\nLSD will close now.", MAX_LINE_SIZE );
				plog( "\n\nAn unknown problem was detected while executing user's equations code" );
				plog( "\n\nWarning: %s active, cannot open LSD Debugger", parallel_mode ? "parallel preocessing" : "fast mode" );
			}

			quit = 2;
		}
	}
#endif

	if ( tk_ok )
		cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { \
			catch { ttk::messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s:\n  %s\n\n%s\" } \
			}", msg1, msg2, msg3 );

#ifndef _LMM_
	if ( user_exception )
	{
		if ( ! parallel_mode && fast_mode == 0 && stacklog != NULL &&
			 stacklog->vs != NULL && stacklog->vs->label != NULL )
		{
			double useless = -1;
			snprintf( msg3, MAX_LINE_SIZE, "%s (ERROR)", stacklog->vs->label );
			deb( stacklog->vs->up, NULL, msg3, & useless );
		}
	}
	else
#endif
		log_tcl_error( true, "FATAL ERROR", "System Signal received: %s", msg1 );

#else

	fprintf( stderr, "\nFATAL ERROR: System Signal received: %s\n", msg1 );

#endif

	myexit( -signum );				// abort program
}
