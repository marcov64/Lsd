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
 COMMON.CPP 
 Code common between LMM and LSD Browser.
 
 Relevant flags (when defined):
 
 - LMM: Model Manager executable
 - FUN: user model equation file
 - NW: No Window executable
 - NP: no parallel (multi-task) processing
 - NT: no signal trapping (better when debugging in GDB)
 *************************************************************/

#include "common.h"

// Tcl/Tk-dependent modules
#ifndef NW

/*********************************
 LOAD_LMM_OPTIONS
 *********************************/
bool load_lmm_options( void )
{
	cmd( "set choice [ file exists \"$RootLsd/$LMM_OPTIONS\" ]" );
	
	if ( choice == 1 )								// file exists?
	{
		cmd( "set f [ open \"$RootLsd/$LMM_OPTIONS\" r ]" );
		
		for ( int i = 0; i < LMM_OPTIONS_NUM; ++i )	// read parameters, returning 1 if incomplete
		{
			cmd( "gets $f %s", lmm_options[ i ] );
			cmd( "if { $%s == \"\" } { set choice 0 }", lmm_options[ i ] );
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
	
	return choice;
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

		if ( atoi( Tcl_GetVar( inter, "done", 0 ) ) )	// nothing to save?
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
	cmd( "set choice [ file exists \"%s/$MODEL_INFO\" ]", path );

	if ( choice == 1 )							// file exists?
	{
		cmd( "set f [ open \"%s/$MODEL_INFO\" r ]", path );

		for ( int i = 0; i < MODEL_INFO_NUM; ++i )	// read parameters, returning 1 if incomplete
		{
			cmd( "gets $f %s", model_info[ i ] );
			cmd( "if { $%s == \"\" } { set choice 0 }", model_info[ i ] );
		}

		cmd( "close $f" );
	}

	return choice;
}


/*********************************
 UPDATE_MODEL_INFO
 *********************************/
void update_model_info( void )
{
	int i;
	
#ifdef LMM
	// set undefined parameters to defaults
	for ( i = 0; i < MODEL_INFO_NUM; ++i )
	{
		cmd( "if { ! [ info exists %s ] } { set %s \"\" }", model_info[ i ], model_info[ i ] );
		cmd( "if { $%s == \"\" } { set %s \"%s\" }", model_info[ i ], model_info[ i ], model_defaults[ i ] );
	}
#else
	// update existing windows positions
	for ( i = 0; i < LSD_WIN_NUM; ++i )
		cmd( "if { $restoreWin } { set curGeom [ geomtosave .%s ]; if { $curGeom != \"\" } { set %s $curGeom } }", wnd_names[ i ], model_info[ i + 3 ] );

	// ensure model name is set
	cmd( "if { ! [ info exists modelName ] } { set modelName \"\" }" );
	cmd( "if { $modelName == \"\" } { regsub \"fun_\" \"%s\" \"\" modelName; regsub \".cpp\" \"$modelName\" \"\" modelName }", equation_name );
	cmd( "set modelDir \"%s\"", exec_path );
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
	char *s;
	int num, res;
	
	if ( ! set_env( true ) )
	{
		log_tcl_error( "Set environment variables", "Environment variable setup failed, Tcl/Tk may be unavailable" );
		myexit( 2 );
	}
	
	// initialize the tcl/tk interpreter
	Tcl_FindExecutable( exec ); 
	inter = Tcl_CreateInterp( );
	num = Tcl_Init( inter );
	if ( num != TCL_OK )
	{
		sprintf( msg, "Tcl initialization directories not found, check the Tcl/Tk installation  and configuration or reinstall LSD\nTcl Error = %d : %s", num,  Tcl_GetStringResult( inter ) );
		log_tcl_error( "Create Tcl interpreter", msg );
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
		log_tcl_error( "Test Tcl", "Tcl failed, check the Tcl/Tk installation and configuration or reinstall LSD" );
		myexit( 3 );
	}

	// initialize & test the tk application
	num = Tk_Init( inter );
	if ( num == TCL_OK )
		cmd( "if { ! [ catch { package present Tk 8.6 } ] && [ winfo exists . ] } { set res 0 } { set res 1 }" );
	
	if ( num != TCL_OK || res )
	{
		sprintf( msg, "Tk failed, check the Tcl/Tk installation (version 8.6+) and configuration or reinstall LSD\nTcl Error = %d : %s", num,  Tcl_GetStringResult( inter ) );
		log_tcl_error( "Start Tk", msg );
		myexit( 3 );
	}
	
	cmd( "wm withdraw ." );
	cmd( "update idletasks" );
	cmd( "tk appname %s", tcl_app_name );
	tk_ok = true;

	// do not open/close terminal in mac
	s = ( char * ) Tcl_GetVar( inter, "tcl_platform(os)", 0 );
	if ( ! strcmp( s, "Darwin") )
	{
		cmd( "console hide" );
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
	cmd( "if { [ string first \" \" \"[ pwd ]\" ] >= 0  } { set res 1 } { set res 0 }" );
	if ( res )
	{
		log_tcl_error( "Path check", "LSD directory path includes spaces, move all the LSD directory in another directory without spaces in the path" );
		cmd( "ttk::messageBox -icon error -title Error -type ok -message \"Installation error\" -detail \"The LSD directory is: '[ pwd ]'\n\nIt includes spaces, which makes impossible to compile and run LSD models.\nThe LSD directory must be located where there are no spaces in the full path name.\nMove all the LSD directory in another directory. If it exists, delete the '%s' file from the sources (src) directory.\n\nLSD is aborting now.\"", SYSTEM_OPTIONS );
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
			exec_path = new char[ MAX_PATH_LENGTH + 1 ];
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
				if ( system( TCL_FIND_EXE ) != 0 )
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

	cur_dir = new char[ MAX_PATH_LENGTH + 1 ];
	last_dir = new char[ MAX_PATH_LENGTH + 1 ];
	orig_dir = new char[ MAX_PATH_LENGTH + 1 ];
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
			strncpy( start_path, cur_dir, strlen( start_path ) );
			found = start_path;
			break;
		}
		
		strcpy( last_dir, cur_dir );
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
bool firstCall = true;

void cmd( const char *cm, ... )
{
#ifndef NP
	// abort if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
		return;
#endif
	
	// abort if Tcl interpreter not initialized
	if ( inter == NULL )
	{
		printf( "\nTcl interpreter not initialized. Quitting LSD now.\n" );
		myexit( 24 );
	}
	
	int bufsz = strlen( cm ) + TCL_BUFF_STR;
	char buffer[ bufsz + 1 ], message[ MAX_LINE_SIZE ];
	va_list argptr;
	
	va_start( argptr, cm );
	int reqsz = vsnprintf( buffer, bufsz, cm, argptr );
	va_end( argptr );
	
	if ( reqsz > bufsz )
	{
		sprintf( message, "Tcl buffer overrun. Please increase TCL_BUFF_STR to at least %d bytes.", reqsz );
		log_tcl_error( cm, message );
		if ( tk_ok )
			cmd( "ttk::messageBox -type ok -title Error -icon error -message \"Tcl buffer overrun\" -detail \"Tcl/Tk command failed.\"" );
	}
	else
	{
		int code = Tcl_Eval( inter, buffer );

		if ( code != TCL_OK )
		{
			log_tcl_error( cm, Tcl_GetStringResult( inter ) );
#ifdef LMM
			if ( tk_ok )
				cmd( "ttk::messageBox -type ok -title Error -icon error -message \"Tcl error\" -detail \"More information in file '%s/%s'.\"", rootLsd, err_file );
#endif
		}
	}
}


/****************************************************
 LOG_TCL_ERROR
 ****************************************************/
void log_tcl_error( const char *cm, const char *message )
{
	FILE *f;
	time_t rawtime;
	struct tm *timeinfo;
	char *err_path, ftime[ 80 ];
	
#ifdef LMM
	err_path = rootLsd;
#else
	err_path = exec_path;
#endif

	char fname[ ( err_path != NULL ? strlen( err_path ) : 0 ) + strlen( err_file ) + 2 ];

	if ( err_path != NULL && strlen( err_path ) > 0 )
		sprintf( fname, "%s/%s", err_path, err_file );
	else
		sprintf( fname, "%s", err_file );

	f = fopen( fname, "a" );
	if ( f == NULL )
	{
#ifdef LMM
		if ( tk_ok )
			cmd( "ttk::messageBox -type ok -title Error -icon error -message \"Log file write error\" -detail \"Cannot write to log file: '%s/%s'\nCheck write permissions.\"", err_path, err_file );
		else
			printf( "\nCannot write to log file '%s/%s'.\nCheck write permissions\n", err_path, err_file );
#else
		plog( "\nCannot write to log file '%s/%s'.\nCheck write permissions\n", "", err_path, err_file );
#endif
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
	fprintf( f, "\n(%s)\nCommand:\n%s\nMessage:\n%s\n-----\n", ftime, cm, message );
	fclose( f );
	
#ifndef LMM
	plog( "\nInternal LSD error. See file '%s'\n", "", fname );
#endif
}


/****************************************************
 TCL_LOG_TCL_ERROR
 Entry point function for access from the Tcl interpreter
 ****************************************************/
int Tcl_log_tcl_error( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	if ( argc != 3 || argv[ 1 ] == NULL || argv[ 2 ] == NULL )	// require 2 parameters
		return TCL_ERROR;

	log_tcl_error( argv[ 1 ], argv[ 2 ] );
		
	char empty[ ] = "";
	Tcl_SetResult( inter, empty, TCL_VOLATILE );
	return TCL_OK;		
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
	Tcl_SetVar( inter, "lab", lab, 0 );
	cmd( "if [ regexp {^[a-zA-Z_][a-zA-Z0-9_]*$} $lab ] { set answer 1 } { set answer 0 }" );
	const char *answer = Tcl_GetVar( inter, "answer", 0 );
	
	if ( *answer == '0' )
		return false;
	else
		return true;
}


/***************************************************
 GET_BOOL
 ***************************************************/
bool get_bool( const char *tcl_var, bool *var )
{
	char *string;
	int intvar;
	
	string = ( char * ) Tcl_GetVar( inter, tcl_var, 0 );
	if ( string == NULL )
	{
		log_tcl_error( "Invalid Tcl variable name", "Internal LSD error (get_bool). If the problem persists, please contact developers" );
		return *var;
	}
		
	sscanf( string, "%d", &intvar );
	if ( var != NULL )
		*var = intvar ? true : false;
	
	return intvar ? true : false;
}


/***************************************************
 GET_INT
 ***************************************************/
int get_int( const char *tcl_var, int *var )
{
	char *string;
	int intvar;
	
	string = ( char * ) Tcl_GetVar( inter, tcl_var, 0 );
	if ( string == NULL )
	{
		log_tcl_error( "Invalid Tcl variable name", "Internal LSD error (get_int). If the problem persists, please contact developers" );
		return *var;
	}
		
	sscanf( string, "%d", &intvar );
	if ( var != NULL )
		*var = intvar;
	
	return intvar;
}


/***************************************************
 GET_LONG
 ***************************************************/
long get_long( const char *tcl_var, long *var )
{
	char *string;
	long longvar;
	
	string = ( char * ) Tcl_GetVar( inter, tcl_var, 0 );
	if ( string == NULL )
	{
		log_tcl_error( "Invalid Tcl variable name", "Internal LSD error (get_long). If the problem persists, please contact developers" );
		return *var;
	}
		
	sscanf( string, "%ld", &longvar );
	if ( var != NULL )
		*var = longvar;
	
	return longvar;
}


/***************************************************
 GET_DOUBLE
 ***************************************************/
double get_double( const char *tcl_var, double *var )
{
	char *string;
	double dblvar;
	
	string = ( char * ) Tcl_GetVar( inter, tcl_var, 0 );
	if ( string == NULL )
	{
		log_tcl_error( "Invalid Tcl variable name", "Internal LSD error (get_double). If the problem persists, please contact developers" );
		return *var;
	}
		
	sscanf( string, "%lf", &dblvar );
	if ( var != NULL )
		*var = dblvar;
	
	return dblvar;
}


/***************************************************
 GET_STR
 ***************************************************/
char *get_str( const char *tcl_var, char *var, int var_size )
{
	char *strvar;
	
	strvar = ( char * ) Tcl_GetVar( inter, tcl_var, 0 );
	if ( strvar == NULL )
	{
		log_tcl_error( "Invalid Tcl variable name", "Internal LSD error (get_str). If the problem persists, please contact developers" );
		return var;
	}
		
	if ( var != NULL )
	{
		if ( var_size > 0 )
			strncpy( var, strvar, var_size - 1 );
		else
			strcpy( var, strvar );
			
		return var;
	}
	else
		return strvar;
}

#else

void cmd( const char *cm, ... ) { }

#endif


/*************************************
 MAIN
 *************************************/
int main( int argn, char **argv )
{
	int res = 0;

#ifndef NT
	// register all signal handlers
	handle_signals( signal_handler );

	try
	{
#endif
		res = lsdmain( argn, argv );
		
#ifndef NT
	}
	catch ( bad_alloc& )	// out of memory conditions
	{
		signal_handler( SIGMEM );
	}
	catch ( exception& exc )// other known error conditions
	{
		sprintf( msg, "\nSTL exception of type: %s\n", exc.what( ) );
		signal_handler( SIGSTL );
	}
	catch ( ... )				// other unknown error conditions
	{
		abort( );				// raises a SIGABRT exception, tell user & close
	}

#endif

	myexit( res );
	return res;
}


/****************************************************
 CLEAN_FILE
 remove any path prefixes to filename, if present
 ****************************************************/
char *clean_file( char *filename )
{
	if ( filename != NULL )
	{
		if ( strchr( filename, '/' ) != NULL )
			return strrchr( filename, '/' ) + 1;
		
		if ( strchr( filename, '\\' ) != NULL )
			return strrchr( filename, '\\' ) + 1;
	}
	
	return filename;
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
		strcpy( temp + 2, filepath + pathpos );			// copy removing prefix
		strcpy( filepath, temp );
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
	strcpy( s, app );
}


/***************************************************
 CLEAN_NEWLINES
 ***************************************************/
void clean_newlines( char *s )
{
	int i, len;
	
	if ( s == NULL )
		return;
	
	len = strlen( s );

	char d[ len + 1 ];

	for ( i = 0; s[ i ] == '\n' && i < len; ++i );

	strcpy( d, s + i );
	strcpy( s, d );

	for ( i = strlen( s ) - 1; s[ i ] == '\n' && i >= 0; --i );
	
	s[ i + 1 ] = '\0';
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
 STR_UPR
 convert string to upper case
 ****************************************************/
char *str_upr( char *s )
{
	unsigned char *p = ( unsigned char * ) s;

	if ( s == NULL )
		return NULL;
	
	while ( *p )
	{
		*p = toupper( *p );
		++p;
	}

	return s;
}


/****************************************************
 MSLEEP
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
#ifndef NP
	// stop multi-thread workers, if needed
	delete [ ] workers;
#endif

#ifndef NW
	if ( inter != NULL )
	{
		cmd( "if { ! [ catch { package present Tk } ] } { destroy . }" );
		cmd( "catch { LsdExit }" );
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
 ****************************************************/
// handle critical system signals
void signal_handler( int signum )
{
	char msg2[ MAX_LINE_SIZE ], msg3[ MAX_LINE_SIZE ];
	double useless = -1;
	
	switch ( signum )
	{
		case SIGINT:
		case SIGTERM:
#ifdef NW
			sprintf( msg, "SIGINT/SIGTERM (%s)", signal_name( signum ) );
			break;
#else
			choice = 1;				// regular quit (checking for save)
			return;
#endif
#ifdef SIGWINCH
		case SIGWINCH:
#ifndef NW
			cmd( "sizetop all" );	// readjust windows size/positions
			cmd( "update idletasks" );
#endif
			return;
#endif
		case SIGSTL:
			break;
			
		case SIGMEM:
			sprintf( msg, "SIGMEM (Out of memory)" );
			strcpy( msg2, "Maybe too many series saved?\n  Try to reduce the number of series saved or the number of time steps" );
			break;
			
		case SIGABRT:
			sprintf( msg, "SIGABRT (%s)", signal_name( signum ) );
			strcpy( msg2, "Maybe an invalid call to library or Tcl/Tk?" );		
			break;

		case SIGFPE:
			sprintf( msg, "SIGFPE (%s)", signal_name( signum ) );
			strcpy( msg2, "Maybe a division by 0 or similar?" );
		break;
		
		case SIGILL:
			sprintf( msg, "SIGILL (%s)", signal_name( signum ) );
			strcpy( msg2, "Maybe executing data?" );		
		break;
		
		case SIGSEGV:
			sprintf( msg, "SIGSEGV (%s)", signal_name( signum ) );
			strcpy( msg2, "Maybe an invalid pointer?\n  Also ensure no group of objects has zero elements." );		
		break;
		default:
			sprintf( msg, "Unknown signal (%s)", signal_name( signum ) );
			strcpy( msg2, "" );			
	}

#ifndef NW

#ifndef LMM
	if ( ! user_exception )
#endif
	{
		strcpy( msg2, "There is an internal LSD error\n  If error persists, please contact developers" );
		strcpy( msg3, "LSD will close now..." );
	}
#ifndef LMM
	else
	{
		strcpy( msg3, "Additional information may be obtained running the simulation using the 'Model'/'GDB Debugger' menu option" );
		if ( quit != 2 )
		{
			if ( ! parallel_mode && fast_mode == 0 && stacklog != NULL && 
				 stacklog->vs != NULL && stacklog->vs->label != NULL )
			{
				strcat( msg3, "\n\nAttempting to open the LSD Debugger.\n\nLSD will close immediately after exiting the Debugger." );
				plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s'", "", stacklog->vs->label );
				print_stack( );				
			}
			else
			{
				strcat( msg3, "\n\nPlease disable fast mode and parallel processing to get more information about the error.\n\nLSD will close now." );
				plog( "\n\nAn unknown problem was detected while executing user's equations code" );
				plog( "\n\nWarning: %s active, cannot open LSD Debugger", "", parallel_mode ? "parallel preocessing" : "fast mode" );
			}
				
			quit = 2;
		}
	}
#endif
	
	cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s:\n  %s\n\n%s\"", msg, msg2, msg3 );
	
#ifndef LMM
	if ( user_exception )
	{
		if ( ! parallel_mode && fast_mode == 0 && stacklog != NULL && 
			 stacklog->vs != NULL && stacklog->vs->label != NULL )
		{
			sprintf( msg3, "%s (ERROR)", stacklog->vs->label );
			deb( stacklog->vs->up, NULL, msg3, &useless );
		}
	}
	else
#endif
	{
		sprintf( msg3, "System Signal received: %s", msg );
		log_tcl_error( "FATAL ERROR", msg3 );
	}
	
#else
	
	fprintf( stderr, "\nFATAL ERROR: System Signal received: %s\n", msg );
	
#endif

	myexit( -signum );				// abort program
}
