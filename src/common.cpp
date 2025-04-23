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
 common code used in DLL or terminal executables are stored
 COMMONLIB.CPP.
 *************************************************************/

#include "LSD.h"

namespace gui
{
	mtxT lock_log_tcl_err;			// lock log_tcl_error for parallel access
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
	else
		tcl_ok = true;

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
		tcl_ok = false;
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
	else
		tk_ok = true;

	cmd( "wm withdraw ." );
	cmd( "update idletasks" );
	cmd( "tk appname %s", tcl_app_name );

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
		cmd( "tk_messageBox -icon error -title Error -type ok -message \"Installation error\" -detail \"The LSD directory is\n\n[ pwd ]\n\nIt includes spaces, which makes impossible to compile and run LSD models.\nThe LSD directory must be located where there are no spaces in the full path name.\n\nPlease reinstall LSD in a proper directory.\nLSD is aborting now.\"" );
		lsd_exit_gui( 4 );
	}

	Tcl_UnlinkVar( interp, "res" );
}


/*************************************************************
 INIT_LSD_ENV
 initialize LSD path and environment variables
 *************************************************************/
int gui::init_lsd_env( const char **argv )
{
	char *str;
	const char *app, *app1;
	int i;
	FILE *f;

	// set system defaults in tcl
	cmd( "set LMM_TXT_OPTIONS \"%s\"", LMM_TXT_OPTIONS );
	cmd( "set LSD_XML_CONFIG \"%s\"", LSD_XML_CONFIG );
	cmd( "set SYSTEM_TXT_OPTIONS \"%s\"", SYSTEM_TXT_OPTIONS );
	cmd( "set MODEL_TXT_OPTIONS \"%s\"", MODEL_TXT_OPTIONS );
	cmd( "set MODEL_XML_CONFIG \"%s\"", MODEL_XML_CONFIG );
	cmd( "set MODEL_TXT_INFO \"%s\"", MODEL_TXT_INFO );
	cmd( "set GROUP_TXT_INFO \"%s\"", GROUP_TXT_INFO );
	cmd( "set GROUP_XML_CONFIG \"%s\"", GROUP_XML_CONFIG );
	cmd( "set DESCRIPTION \"%s\"", DESCRIPTION );
	cmd( "set DATE_FMT \"%s\"", DATE_FMT );

	// assume exec path is included in file name, use CWD if not
	lsd::set_exec( NULL, argv[ 0 ] );

	if ( lsd::exec_file == NULL || strlen( lsd::exec_file ) == 0 || lsd::exec_path == NULL || strlen( lsd::exec_path ) == 0 )
	{
		log_tcl_error( true, "Invalid LSD executable name or path", "Make sure the LSD directory is not too deep into the disk directory tree" );
		return 1;
	}

	// check if exec file is in current path
	i = strlen( lsd::exec_path ) + strlen( lsd::exec_file ) + 2;
	str = new char[ i ];
	snprintf( str, i, "%s%s%s", lsd::exec_path, strlen( lsd::exec_path ) > 0 ? "/" : "", lsd::exec_file );
	f = fopen( str, "r" );
	delete [ ] str;
	if ( f != NULL )
		fclose( f );

	// try to recover from failures
	if ( f == NULL || strlen( lsd::exec_path ) == 0 || ! strcmp( lsd::exec_path, "/" ) )
	{	// try to get exec name from Tcl
		cmd( "if { [ info nameofexecutable ] ne \"\" } { \
				set path [ file dirname [ info nameofexecutable ] ]; \
				set exec [ file rootname [ info nameofexecutable ] ] \
			} { \
				set path [ pwd ]; \
				set exec \"\" \
			}" );

		app = get_str( "path" );
		app1 = get_str( "exec" );
		if ( app != NULL && app1 != NULL && strlen( app1 ) > 0 )
			lsd::set_exec( app, app1 );
		else
		{
			gui::log_tcl_error( false, "LSD executable check", "Cannot locate LSD executable on disk, check the installation of LSD and reinstall LSD if the problem persists" );
			cmd( "tk_messageBox -type ok -icon error -title Error -message \"LSD executable not found\" -detail \"Cannot locate the LSD executable folder on disk.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"" );
			return 5;
		}
	}

	// check if executable is inside a macOS package
	cmd( "set path [ file normalize \"%s\" ]", lsd::exec_path );
	cmd( "if { $tcl_platform(os) eq \"Darwin\" } { \
			set pathsplit [ file split $path ]; \
			if { [ lindex $pathsplit end ] eq \"MacOS\" && [ lindex $pathsplit end-1 ] eq \"Contents\" } { \
				set path [ file normalize \"$path/../../..\" ] \
			}; \
			unset pathsplit \
		}" );

#ifndef _LMM_
	// only use the exec path if not already in a model directory
	cmd( "if { [ file exists $MODEL_TXT_OPTIONS ] || [ file exists $MODEL_XML_CONFIG ] } { \
			set model_dir [ pwd ]; \
			set path [ pwd ] \
		} { \
			set model_dir $path; \
		}" );

	cmd( "cd $model_dir" );
	cmd( "set a [ file normalize $model_dir ]" );
	app = get_str( "a" );

	if ( app != NULL && strlen( app ) > 0 )
	{
		delete [ ] sim.conf_path;
		delete [ ] lsd::model_path;
		sim.conf_path = new char[ strlen( app ) + 1 ];
		lsd::model_path = new char[ strlen( app ) + 1 ];
		strcpy( sim.conf_path, app );
		strcpy( lsd::model_path, app );
	}
	else
	{
		log_tcl_error( false, "LSD model directory check", "Cannot locate LSD model folder on disk, check your model directory, or recreate the model" );
		cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"LSD model directory missing\" -detail \"Cannot locate the LSD model folder on disk.\nPlease check your model directory and recreate your model if the problem persists.\n\nLSD is aborting now.\"" );
		return 6;
	}
#endif

	cmd( "set lsd_src \"%s\"", DEFAULT_SRC_DIR );	// default initial source dir
	cmd( "set lsd_example \"%s\"", DEFAULT_EXAMPLE_DIR );// examples group dir
	cmd( "set lsd_trash \"%s\"", DEFAULT_TRASH_DIR );// trash bin group dir

	// check if LSDROOT environment variable exists and use it if so
	cmd( "if { [ info exists env(LSDROOT) ] } { \
			set lsd_root [ file normalize $env(LSDROOT) ] \
		} { \
			set lsd_root [ file normalize %s ] \
		}", lsd::exec_path );
	cmd( "if [ file exists \"$lsd_root/$lsd_src/LSD.h\" ] { \
			set res 0 \
		} { \
			set res 1 \
		}" );

	// do some search for the right path to cope with macOS package
	if ( get_bool( "res" ) )
	{
		cmd( "set here [ pwd ]" );
		cmd( "while { ! [ file exists \"$lsd_src/LSD.h\" ] && ! [ string equal [ pwd ] \"/\" ] && [ string length [ pwd ] ] > 3 } { \
				cd .. \
			}" );
		cmd( "if [ file exists \"$lsd_src/LSD.h\" ] { \
				set lsd_root [ pwd ]; \
				cd $here; \
				set res 0 \
			} { \
				set res 1 \
			}" );
		cmd( "unset here" );

		if ( get_bool( "res" ) )
		{
#ifdef _LMM_
			log_tcl_error( false, "Source files check", "Required LSD source file(s) missing or corrupted, check the installation of LSD and reinstall LSD if the problem persists" );
			cmd( "tk_messageBox -type ok -icon error -title Error -message \"File(s) missing or corrupted\" -detail \"Some critical LSD files or folders are missing or corrupted.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"" );
#else
			log_tcl_error( false, "LSDROOT check", "LSDROOT not set, make sure the environment variable LSDROOT points to the directory where LSD is installed" );
			cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"LSDROOT not set\" -detail \"Please make sure the environment variable LSDROOT points to the directory where LSD is installed.\n\nLSD is aborting now.\"" );
#endif
			return 5;
		}

		cmd( "set env(LSDROOT) \"$lsd_root\"" );
	}

	app = get_str( "lsd_root" );
	if ( app != NULL && strlen( app ) > 0 )
	{
		lsd::root_lsd = lsd::clean_path( app );
		cmd( "set lsd_root [ file normalize \"%s\" ]", lsd::root_lsd );
	}
	else
	{
		log_tcl_error( false, "LSD directory check", "Cannot locate LSD folder on disk, check the installation of LSD and reinstall LSD if the problem persists" );
		cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"LSD directory missing\" -detail \"Cannot locate the LSD installation folder on disk.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"" );
		return 1;
	}

	// set default LSD XML configuration path (same as LSDROOT if in home directory)
	cmd( "if { [ string first [ file normalize ~ ] $lsd_root ] == 0 } { \
				set cfgDir $lsd_root \
			} { \
				if { $tcl_platform(os) eq \"Windows NT\" } { \
					set cfgDir [ file normalize \"~/AppData/Local/LSD\" ] \
				} { \
					if { [ info exists env(XDG_CONFIG_HOME) ] && $env(XDG_CONFIG_HOME) ne \"\" } { \
						set cfgDir [ file normalize \"$env(XDG_CONFIG_HOME)/LSD\" ] \
					} { \
						set cfgDir [ file normalize \"~/.config/LSD\" ] \
					} \
				} \
			}" );

	app = get_str( "cfgDir" );
	lsd::strcpyn( cfg_path, app, MAX_PATH_LENGTH );

	// create directory if it doesn't exist and check it
	cmd( "file mkdir $cfgDir" );
	cmd( "if { [ file exists $cfgDir ] && [ file isdirectory $cfgDir ] } { \
			set res 0 \
		} { \
			set res 1 \
		}" );

	if ( get_bool( "res" ) )
	{
		log_tcl_error( false, "Configuration directory check", "Cannot locate or create LSD configuration folder on disk, check the installation of LSD and reinstall LSD if the problem persists" );
		cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"Cannot create LSD configuration directory\" -detail \"Cannot create or access the LSD configuration folder on disk\n\n%s\n\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"", get_str( "cfgDir" ) );
		return 1;
	}

#ifdef _LMM_
	// change path to the LSD root directory in LMM
	cmd( "cd $lsd_root" );
#endif

	return 0;
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
	char *lsdroot, *path, cur_path[ PATH_MAX ];
	static char *lsdroot_env = NULL, *tcl_lib_env = NULL, *path_env = NULL;

	if ( set )
	{
		lsdroot = getenv( "LSDROOT" );

		if ( lsdroot == NULL )
		{
			if ( getcwd( cur_path, PATH_MAX ) != NULL )
			{
				path = lsd::clean_path( cur_path );
				lsd::strcpyn( cur_path, path, PATH_MAX );
				delete [ ] path;
				lsdroot = search_lsdroot( cur_path, PATH_MAX );
			}

			if ( lsdroot != NULL )
			{
				delete [ ] lsdroot_env;
				lsdroot_env = new char[ strlen( "LSDROOT" ) + strlen( lsdroot ) + 2 ];
				sprintf( lsdroot_env, "LSDROOT=%s", lsdroot );

				res = ! ( bool ) putenv( lsdroot_env );
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

		if ( lsdroot != NULL && getenv( TCL_LIB_VAR ) == NULL )
		{
			lsdroot = lsd::clean_path( lsdroot );

			file = new char[ strlen( lsdroot ) + strlen( TCL_LIB_PATH ) + strlen( TCL_LIB_INIT ) + 3 ];
			sprintf( file, "%s/%s/%s", lsdroot, TCL_LIB_PATH, TCL_LIB_INIT );
			st = stat( file, &info );
			delete [ ] file;

			if ( st == 0 )
			{
				delete [ ] tcl_lib_env;
				tcl_lib_env = new char[ strlen( TCL_LIB_VAR ) + strlen( lsdroot ) + strlen( TCL_LIB_PATH ) + 3 ];
				sprintf( tcl_lib_env, "%s=%s/%s", TCL_LIB_VAR, lsdroot, TCL_LIB_PATH );

				res = ! ( bool ) putenv( tcl_lib_env );
			}
			else
				if ( lsd::run_system( TCL_FIND_EXE ) != 0 )
					res = false;	// just stop if Tcl/Tk is not on path
		}

		if ( lsdroot != NULL && path != NULL )
		{
			// check if not already in path and add it in the adequate order
			lsd_bin = new char[ win_path( lsdroot ).size( ) + strlen( TCL_EXEC_PATH ) + 2 ];
			sprintf( lsd_bin, "%s\\%s", win_path( lsdroot ).c_str( ), TCL_EXEC_PATH );

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
		delete [ ] lsdroot_env;
		delete [ ] path_env;
	}

	return res;
}


/*************************************************************
 SEARCH_LSD_ROOT
 searches LSD root directory upwards to the root
 *************************************************************/
char *gui::search_lsdroot( char *path, int pathSz )
{
	bool miss, eq;
	const char *files[ ] = LSD_MIN_FILES;
	char *file, *dir, cur_dir[ PATH_MAX ], last_dir[ PATH_MAX ], orig_dir[ PATH_MAX ], src_dir[ 2 * PATH_MAX ], *found = NULL;
	int i, st;
	struct stat info;

	if ( getcwd( orig_dir, PATH_MAX ) == NULL )
		return NULL;

	if ( pathSz <= 0 || path == NULL || chdir( path ) == -1 )
		goto end;

	strcpy( last_dir, "" );

	do
	{
		if ( getcwd( cur_dir, PATH_MAX ) == NULL )
			goto end;

		dir = lsd::clean_path( cur_dir );
		eq = strcmp( dir, last_dir ) == 0;
		delete [ ] dir;

		if ( eq )
			goto end;

		snprintf( src_dir, 2 * PATH_MAX, "%s/%s", cur_dir, DEFAULT_SRC_DIR );
		for ( i = 0, miss = false; i < LSD_MIN_NUM; ++i )
		{
			file = new char[ strlen( src_dir ) + strlen( files[ i ] ) + 2 ];
			sprintf( file, "%s/%s", src_dir, files[ i ] );
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
 SET_PLATFORM
 set and check to OS platform
 requires that gui.tcl script has been executed before
 *************************************************************/
int gui::set_platform( void )
{
	const char *app;

	app = get_str( "CurPlatform" );
	if ( ! strcmp( app, "linux" ) )
		platform = _LIN_;
	else
		if ( ! strcmp( app, "mac" ) )
			platform = _MAC_;
		else
			if ( ! strcmp( app, "windows" ) )
				platform = _WIN_;
			else
			{
				log_tcl_error( false, "Unsupported platform", "Your computer operating system is not supported by this LSD version, you may try an older version compatible with legacy systems (Windows 32-bit, Mac OS X, etc.)" );
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Unsupported platform\" -detail \"Your computer operating system is not supported by this LSD version,\nyou may try an older version compatible with legacy systems\n(Windows 32-bit, Mac OS X, etc.)\n\nLSD is aborting now.\"" );
				return 7;
			}

	return 0;
}


/*************************************************************
 RESET_MAKE_OPTIONS
 check if model and system options
 are set and reset to defaults if not
 *************************************************************/
void gui::reset_make_options( int which )
{
	const char *s;

	// model makefile options
	if ( which != 1 && eval_bool( "$model_dir ne \"\"" ) && eval_bool( "$model_dir ne $lsd_root" ) )
	{
		cmd( "set a [ glob -nocomplain \"$model_dir/fun_*.cpp\" ]" );
		cmd( "if { $a ne \"\" } { \
				set b [ file tail [ lindex $a 0 ] ] \
			} { \
				set b \"fun_UNKNOWN.cpp\" \
			}" );
		cmd( "set model_make \"# LSD options\nTARGET=$DefaultExe\nFUN=[ file rootname \"$b\" ]\nPRECOMPILED=true\n\n# Additional model files\nFUN_EXTRA=\n\n# Compiler options\nSWITCH_CC=-O0 -ggdb3\nSWITCH_CC_LNK=\"" );

		if ( ( s = get_str( "model_make" ) ) != NULL )
		{
			delete [ ] model_make;
			model_make = new char [ strlen( s ) + 1 ];
			strcpy( model_make, s );
		}
	}

	// system makefile options
	if ( which != 2 )
	{
        cmd( "if [ string equal $tcl_platform(platform) windows ] { \
                set sysfile \"[ file rootname $SYSTEM_TXT_OPTIONS ]-windows.txt\" \
            } elseif { [ string equal $tcl_platform(os) Darwin ] } { \
                set sysfile \"[ file rootname $SYSTEM_TXT_OPTIONS ]-mac.txt\" \
            } else { \
                set sysfile \"[ file rootname $SYSTEM_TXT_OPTIONS ]-linux.txt\" \
            }" );

		cmd( "set f [ open \"$lsd_root/$lsd_src/$sysfile\" r ]" );
		cmd( "set system_make \"# LSD options\n\"" );
		cmd( "append system_make \"LSDROOT=$lsd_root\n\"" );
		cmd( "append system_make \"SRC=$lsd_src\n\n\"" );
		cmd( "append system_make [ string trim [ read $f ] ]" );
		cmd( "close $f" );

		if ( ( s = get_str( "system_make" ) ) != NULL )
		{
			delete [ ] system_make;
			system_make = new char [ strlen( s ) + 1 ];
			strcpy( system_make, s );
		}
	}
}


/*************************************************************
 LOAD_LSD_OPTIONS
 *************************************************************/
#define PUGI_LOAD_OPTIONS pugi::parse_default | \
						  pugi::parse_declaration | \
						  pugi::parse_doctype | \
						  pugi::parse_trim_pcdata

void gui::load_lsd_options( void )
{
	char fName[ MAX_PATH_LENGTH ];
	const char *s, *path = cfg_path;				// default path
	int i;
	x_docT sysCfg;

	// ensure defaults are loaded
	cmd( "if { ! [ info exists CurPlatform ] } { \
			source \"$lsd_root/$lsd_src/defaults.tcl\" \
		}" );

	cmd( "if [ file exists \"$cfgDir/$LSD_XML_CONFIG\" ] { \
			set res 1 \
		} elseif [ file exists \"$lsd_root/$LSD_XML_CONFIG\" ] { \
			set res 2 \
		} elseif [ file exists \"$cfgDir/$LMM_TXT_OPTIONS\" ] { \
			set f [ open \"$cfgDir/$LMM_TXT_OPTIONS\" r ]; \
			set res 3 \
		} elseif [ file exists \"$lsd_root/$LMM_TXT_OPTIONS\" ] { \
			set f [ open \"$lsd_root/$LMM_TXT_OPTIONS\" r ]; \
			set res 3 \
		} else { \
			set res 0 \
		}" );

	if ( ( i = get_int( "res" ) ) == 2 )
		path = lsd::root_lsd;						// xml format in LSD root

	// load legacy configuration (one-time migration)
	if ( i == 3 )
	{
		for ( i = 0; i < LMM_OPTIONS_NUM; ++i )		// read all parameters
		{
			cmd( "gets $f %s", lmm_options[ i ] );
			cmd( "if { $%s eq \"#\" } { set %s \"\" }", lmm_options[ i ], lmm_options[ i ] );
		}

		cmd( "close $f" );
	}

	snprintf( fName, MAX_PATH_LENGTH, "%s%s%s", path, strlen( path ) > 0 ? "/" : "", LSD_XML_CONFIG );

	// try to load XML file
	if ( sysCfg.load_file( fName, PUGI_LOAD_OPTIONS ).status != pugi::status_ok )
	{
		update_lsd_options( );						// rebuild configuration file
		sysCfg.load_file( fName, PUGI_LOAD_OPTIONS );
	}

	// load XML file structure
	x_nodeT lsdNode = sysCfg.document_element( );	// LSD top element
	x_nodeT sysNode = lsdNode.child( "system" );	// load system config.
	x_nodeT lmmNode = sysNode.child( "LMM" );		// LMM configuration
	x_nodeT setNode = lmmNode.child( "settings" );	// LMM settings
	x_nodeT geoNode = lmmNode.child( "geometry" );	// LMM current geometry

	// load LMM settings
	for ( i = 0; i < LMM_OPTIONS_NUM; ++i )
	{
		cmd( "set s \"%s\"", lmm_defaults[ i ] );
		s = get_str( "s" );
		if ( strcmp( s, "#" ) == 0 )
			s = "";									// handle empty items

		switch ( lmm_types[ i ] )
		{
			case 'a':								// attribute
				cmd( "set %s \"%s\"", lmm_options[ i ], setNode.attribute( lmm_options[ i ] ).as_string( s ) );
				break;

			case 'p':								// PCDATA text node
				cmd( "set %s \"%s\"", lmm_options[ i ], setNode.child( lmm_options[ i ] ).text( ).as_string( s ) );
				break;

			case 'g':								// geometry
				cmd( "set %s \"%s\"", lmm_options[ i ], geoNode.text( ).as_string( s ) );
				break;
		}
	}

	// load system makefile options
	x_nodeT makNode = sysNode.child( "makefile" );	// makefile configuration
	s = makNode.text( ).as_string( );
	if ( strlen( s ) == 0 )
	{
		// try to read legacy files
		if ( eval_bool( "[ file exists \"$cfgDir/$SYSTEM_TXT_OPTIONS\" ]" ) )
			cmd( "set sysfile \"$cfgDir/$SYSTEM_TXT_OPTIONS\"" );
		else
			if ( eval_bool( "[ file exists \"$lsd_root/$lsd_src/$SYSTEM_TXT_OPTIONS\" ]" ) )
				cmd( "set sysfile \"$lsd_root/$lsd_src/$SYSTEM_TXT_OPTIONS\"" );
			else	// if not, use default settings for platform
			{
				reset_make_options( 1 );
				cmd( "set sysfile \"\"" );
			}

		if ( strlen( get_str( "sysfile" ) ) > 0 )
		{
			cmd( "set f [ open $sysfile r ]" );
			cmd( "set system_make [ string trim [ read $f ] ]" );
			cmd( "close $f" );
			if ( ( s = get_str( "system_make" ) ) != NULL )
			{
				delete [ ] system_make;
				system_make = new char [ strlen( s ) + 1 ];
				strcpy( system_make, s );
			}
		}

		update_lsd_options( );						// update configuration file
	}
	else
	{
		delete [ ] system_make;
		system_make = lsd::strdecdata( NULL, s );
		cmd( "set system_make {%s}", system_make );
	}

#ifdef _LMM_
	// fix new model directory if needed
	if ( eval_bool( "[ file normalize $group_new ] eq \"/\" || [ file normalize $group_new ] eq $lsd_root || ( ! [ file exists $group_new ] && [ catch { file mkdir $group_new } ] )" ) )
	{
		cmd( "set group_new \"%s\"", DEFAULT_GROUP_DIR );
		if ( ! eval_bool( "[ file exists $group_new ]" ) )
			cmd( "catch { file mkdir $group_new }" );
	}

	if ( ! eval_bool( "[ file exists \"$group_new/$GROUP_TXT_INFO\" ] || [ file exists \"$group_new/$GROUP_XML_CONFIG\" ]" ) )
	{
		cmd( "set_group_setting [ file normalize $group_new ] name \"Work in Progress\"" );
		cmd( "set_group_setting [ file normalize $group_new ] description \"Models under development.\"" );
	}

	// load previous model
	x_nodeT modNode = lmmNode.child( "model" );		// LMM current model
	cmd( "set m {%s}", modNode.child( "path" ).text( ).as_string( ) );
	if ( eval_bool( "[ file exists \"$m/$MODEL_XML_CONFIG\" ]" ) )
	{
		cmd( "set g [ file normalize \"$m/..\" ]" );
		if ( eval_bool( "[ file exists \"$g/$GROUP_TXT_INFO\" ] || [ file exists \"$g/$GROUP_XML_CONFIG\" ]" ) )
		{
			cmd( "set model_dir $m" );
			cmd( "set group_dir $g" );
		}
	}

	cmd( "set f {%s}", modNode.child( "file" ).text( ).as_string( ) );
	if ( eval_bool( "[ file exists $f ] && [ file isfile $f ]" ) )
	{
		cmd( "set file_dir [ file normalize [ file dirname $f ] ]" );
		cmd( "set file_name [ file tail $f ]" );
	}
#endif
}


/*************************************************************
 UPDATE_LSD_OPTIONS
 *************************************************************/
void gui::update_lsd_options( bool save_settings )
{
	bool save_attr;
	char fName[ MAX_PATH_LENGTH ];
	const char *s;
	x_attrT attr;
	x_docT sysCfg;
	x_nodeT child;

	// update current geometry if no saving just settings
	if ( ! save_settings )
		cmd( "if { $restore_geom } { \
				set curGeom [ geomtosave .lmm ]; \
				if { $curGeom != \"\" && ! [ string equal $lmm_geom $curGeom ] } { \
					set lmm_geom $curGeom \
				} \
			}" );

	// try to load existing XML as base
	snprintf( fName, MAX_PATH_LENGTH, "%s%s%s", cfg_path, strlen( cfg_path ) > 0 ? "/" : "", LSD_XML_CONFIG );

	if ( sysCfg.load_file( fName, PUGI_LOAD_OPTIONS ).status != pugi::status_ok )
		sysCfg.reset( );							// recreate XML structure

	// try to read configuration nodes
	x_nodeT lsdNode = sysCfg.document_element( );	// LSD top element
	x_nodeT sysNode = lsdNode.child( "system" );	// load system config.
	x_nodeT lmmNode = sysNode.child( "LMM" );		// LMM configuration
	x_nodeT setNode = lmmNode.child( "settings" );	// LMM settings
	x_nodeT geoNode = lmmNode.child( "geometry" );	// LMM current geometry

	// recreate XML structure if wrong format XML
	if ( strcmp( lsdNode.name( ), "LSD" ) != 0 || strcmp( sysNode.name( ), "system" ) != 0 )
	{
		sysCfg.reset( );							// recreate all
		x_nodeT typeNode = sysCfg.append_child( pugi::node_declaration );
		typeNode.append_attribute( "version" ) = "1.0";
		typeNode.append_attribute( "encoding" ) = "ISO-8859-1";
		typeNode.append_attribute( "standalone" ) = "yes";
		sysCfg.append_child( pugi::node_doctype ).set_value( "LSD [\n \
		<!ELEMENT LSD (system)>\n \
		<!ELEMENT system (LMM, makefile)>\n \
		<!ELEMENT LMM (settings, geometry, model?)>\n \
		<!ELEMENT settings (#PCDATA)>\n \
		<!ELEMENT geometry (#PCDATA)>\n \
		<!ELEMENT model (#PCDATA)>\n \
		<!ELEMENT makefile (#PCDATA)>\n]" );
		lsdNode = sysCfg.append_child( "LSD" );
		sysNode = lsdNode.append_child( "system" );
	}

	// add/update system configuration attributes
	if ( ( attr = sysNode.attribute( "version" ) ) == NULL )
		attr = sysNode.append_attribute( "version" );

	attr = "1.0";

	if ( ( attr = sysNode.attribute( "description" ) ) == NULL )
		attr = sysNode.append_attribute( "description" );

	attr = "LSD system configuration file";

	// add/update LMM settings and geometry (legacy configuration content)
	if ( lmmNode.empty( ) )
		lmmNode = sysNode.append_child( "LMM" );

	if ( setNode.empty( ) )
		setNode = lmmNode.append_child( "settings" );

	if ( geoNode.empty( ) )
		geoNode = lmmNode.append_child( "geometry" );

	for ( int i = 0; i < LMM_OPTIONS_NUM; ++i )
	{
		// set undefined parameters to defaults
		cmd( "if { ! [ info exists %s ] } { set %s \"\" }", lmm_options[ i ], lmm_options[ i ] );
		cmd( "if { $%s eq \"\" && \"%s\" ne \"#\" } { set %s \"%s\" }", lmm_options[ i ], lmm_defaults[ i ], lmm_options[ i ], lmm_defaults[ i ] );

		s = get_str( lmm_options[ i ] );
		if ( strcmp( s, "#" ) == 0 )
			s = "";									// handle empty items

		save_attr = save_settings;					// save default

		switch ( lmm_types[ i ] )
		{
			case 'a':								// attribute
				if ( ( attr = setNode.attribute( lmm_options[ i ] ) ) == NULL )
				{
					attr = setNode.append_attribute( lmm_options[ i ] );
					save_attr = true;				// save because missing
				}

				if ( save_attr )
					attr = s;

				break;

			case 'p':								// PCDATA text node
				if ( ( child = setNode.child( lmm_options[ i ] ) ) == NULL )
				{
					child = setNode.append_child( lmm_options[ i ] );
					save_attr = true;				// save because missing
				}

				if ( save_attr )
					child.text( ) = s;

				break;

			case 'g':								// geometry
				geoNode.text( ) = s;
				break;
		}
	}

#ifdef _LMM_
	// save current model info
	if ( ! save_settings && exists_var( "model_name" ) && strcmp( get_str( "model_name" ), "(no model)" ) != 0 && exists_var( "model_dir" ) && eval_bool( "[ file exists $model_dir ] && [ file isdirectory $model_dir ]" ) )
	{
		x_nodeT modNode = lmmNode.child( "model" );	// LMM current model
		if ( modNode.empty( ) )
			modNode = lmmNode.append_child( "model" );

		if ( exists_var( "model_group" ) )
		{
			if ( ( attr = modNode.attribute( "group" ) ) == NULL )
				attr = modNode.append_attribute( "group" );

			attr = get_str( "model_group" );
		}

		if ( exists_var( "model_name" ) )
		{
			if ( ( attr = modNode.attribute( "name" ) ) == NULL )
				attr = modNode.append_attribute( "name" );

			attr = get_str( "model_name" );
		}

		if ( exists_var( "model_version" ) )
		{
			if ( ( attr = modNode.attribute( "version" ) ) == NULL )
				attr = modNode.append_attribute( "version" );

			attr = get_str( "model_version" );
		}

		if ( ( child = modNode.child( "path" ) ) == NULL )
			child = modNode.append_child( "path" );

		child.text( ) = get_str( "model_dir" );

		if ( exists_var( "file_dir" ) && exists_var( "file_name" ) )
		{
			if ( ( child = modNode.child( "file" ) ) == NULL )
				child = modNode.append_child( "file" );

			child.text( ) = eval_str( "[ file normalize \"$file_dir/$file_name\" ]" );
		}
	}
	else
		lmmNode.remove_child( "model" );
#endif

	// save system makefile options
	x_nodeT makNode = sysNode.child( "makefile" );	// makefile configuration
	if ( makNode.empty( ) )
		makNode = sysNode.append_child( "makefile" );

	if ( ( child = makNode.first_child( ) ) == NULL )
		child = makNode.append_child( pugi::node_cdata );

	if ( system_make != NULL && strlen( system_make ) > 0 )
	{
		s = lsd::strencdata( NULL, system_make );
		child.set_value( s );
		delete [ ] s;
	}

	// save to file
	if ( ! sysCfg.save_file( fName ) )
	{
		gui::log_tcl_error( false, "Cannot save LSD configuration", "LSD configuration file cannot be saved to the user directory.\nnCheck if the user home directory is not set READ-ONLY or if it has enough space, and try again" );
		cmd( "tk_messageBox -type ok -icon error -title Error -message \"Cannot save LSD configuration\" -detail \"File '%s' cannot be saved to directory\n\n%s\n\nCheck if the user home directory is not set READ-ONLY or if it is not full, and try again.\"", LSD_XML_CONFIG, cfg_path );
	}
}


/*************************************************************
 LOAD_MODEL_OPTIONS
 *************************************************************/
bool gui::load_model_options( const char *path, bool fix )
{
	char fName[ MAX_PATH_LENGTH ];
	const char *s;
	int i;
	x_docT modCfg;

	if ( path == NULL && exists_var( "model_dir" ) )
		path = get_str( "model_dir" );

	if ( path != NULL )
	{
		cmd( "set model_dir \"%s\"", path );
		cmd( "set a [ file normalize $model_dir ]" );
		path = get_str( "a" );
	}

	if ( path == NULL || ! eval_bool( "[ file exists $model_dir ] && [ file isdirectory $model_dir ]" ) )
	{
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Cannot load configuration\" -detail \"Choose an existing model or create a new one.\"" );
		return false;
	}

	cmd( "if [ file exists \"%s/$MODEL_XML_CONFIG\" ] { \
			set res 1 \
		} elseif [ file exists \"%s/$MODEL_TXT_INFO\" ] { \
			set f [ open \"%s/$MODEL_TXT_INFO\" r ]; \
			set res 2 \
		} else { \
			set res 0 \
		}", path, path, path );

	if ( ( i = get_int( "res" ) ) == 0 && ! fix )
		return false;

	// load legacy configuration (one-time migration)
	if ( i == 2 )
	{
		for ( i = 0; i < MODEL_OPTIONS_NUM; ++i )	// read all parameters
		{
			cmd( "gets $f %s", model_options[ i ] );
			cmd( "if { $%s eq \"#\" } { set %s \"\" }", model_options[ i ], model_options[ i ] );
		}

		cmd( "close $f" );

		// load corresponding legacy makefile, if any
		load_legacy_makefile( path );
	}

	snprintf( fName, MAX_PATH_LENGTH, "%s%s%s", path, strlen( path ) > 0 ? "/" : "", MODEL_XML_CONFIG );

	// try to load XML file
	if ( modCfg.load_file( fName, PUGI_LOAD_OPTIONS ).status != pugi::status_ok )
	{
		update_model_options( true );				// rebuild configuration file
		modCfg.load_file( fName, PUGI_LOAD_OPTIONS );
	}

	// try to read configuration nodes
	x_nodeT lsdNode = modCfg.document_element( );	// LSD top element
	x_nodeT modNode = lsdNode.child( "model" );		// load model data
	x_nodeT setNode = modNode.child( "settings" );	// model settings
	x_nodeT geoNode = modNode.child( "geometry" );	// model current geometry
	x_nodeT cfgNode = modNode.child( "configuration" );// model current config.

	// load model settings
	for ( i = 0; i < MODEL_OPTIONS_NUM; ++i )
	{
		cmd( "set s \"%s\"", model_defaults[ i ] );
		s = get_str( "s" );
		if ( strcmp( s, "#" ) == 0 )
			s = "";									// handle empty items

		switch ( model_types[ i ] )
		{
			case 's':								// setting node
				cmd( "set %s \"%s\"", model_options[ i ], setNode.child( model_options[ i ] ).text( ).as_string( s ) );
				break;

			case 'g':								// geometry
				cmd( "set %s \"%s\"", model_options[ i ], geoNode.child( model_options[ i ] ).text( ).as_string( s ) );
				break;

			case 'a':								// configuration attribute
				cmd( "set %s \"%s\"", model_options[ i ], cfgNode.attribute( model_options[ i ] ).as_string( s ) );
				break;

			case 'c':								// configuration node
				cmd( "set %s \"%s\"", model_options[ i ], cfgNode.child( model_options[ i ] ).text( ).as_string( s ) );
				break;
		}
	}

	// load model makefile options
	x_nodeT makNode = modNode.child( "makefile" );	// makefile configuration
	s = makNode.text( ).as_string( );
	if ( strlen( s ) == 0 )
	{
		if ( ! load_legacy_makefile( path ) )		// try to read legacy file
			reset_make_options( 2 );				// use default settings

		update_model_options( );					// update configuration file
	}
	else
	{
		delete [ ] model_make;
		model_make = lsd::strdecdata( NULL, s );
		cmd( "set model_make {%s}", model_make );
	}

	return true;
}


/*************************************************************
 LOAD_LEGACY_MAKEFILE
 *************************************************************/
bool gui::load_legacy_makefile( const char *path )
{
	const char *s;

	delete [ ] model_make;
	model_make = NULL;
	cmd( "set model_make \"\"" );

	cmd( "if { [ file exists \"%s/$MODEL_TXT_OPTIONS\" ] } { \
			set sysfile \"%s/$MODEL_TXT_OPTIONS\" \
		} { \
			set sysfile \"\" \
		}", path, path );

	if ( strlen( get_str( "sysfile" ) ) > 0 )
	{
		cmd( "set f [ open $sysfile r ]" );
		cmd( "set model_make [ string trim [ read $f ] ]" );
		cmd( "close $f" );
		if ( ( s = get_str( "model_make" ) ) != NULL )
		{
			model_make = new char [ strlen( s ) + 1 ];
			strcpy( model_make, s );
			return true;
		}
	}

	return false;
}


/*************************************************************
 UPDATE_MODEL_OPTIONS
 *************************************************************/
void gui::update_model_options( bool fix )
{
	char fName[ MAX_PATH_LENGTH ];
	const char *s;
	int i;
	x_attrT attr;
	x_docT modCfg;
	x_nodeT child;

	// ensure defaults are loaded
	cmd( "if { ! [ info exists CurPlatform ] } { \
			source \"$lsd_root/$lsd_src/defaults.tcl\" \
		}" );

	// set undefined parameters to defaults
	if ( fix )
		for ( i = 0; i < MODEL_OPTIONS_NUM; ++i )
		{
			cmd( "if { ! [ info exists %s ] } { set %s \"\" }", model_options[ i ], model_options[ i ] );
			cmd( "if { $%s eq \"\" && \"%s\" ne \"#\" } { set %s \"%s\" }", model_options[ i ], model_defaults[ i ], model_options[ i ], model_defaults[ i ] );
		}

#ifndef _LMM_
	else
		// update existing windows positions
		for ( i = 0; i < LSD_WIN_NUM; ++i )
			cmd( "if { $restore_geom } { \
					set curGeom [ geomtosave .%s ]; \
					if { $curGeom != \"\" } { \
						set %s $curGeom \
					} \
				}", wnd_names[ i ], model_options[ i + 3 ] );

	// ensure model name is set
	cmd( "if { ! [ info exists model_name ] || $model_name eq \"\" || $model_name eq \"%s\" } { \
			set model_name [ string map -nocase { fun_ \"\" .cpp \"\" } \"%s\" ] \
		}", model_defaults[ 0 ], eq_file );
#endif

	if ( exists_var( "model_dir" ) )
	{
		cmd( "set a [ file normalize $model_dir ]" );
		s = get_str( "a" );
	}
	else
		s = NULL;

	if ( s == NULL || ! exists_var( "model_name" ) || strlen( get_str( "model_name" ) ) == 0 || strcmp( get_str( "model_name" ), "(no model)" ) == 0 || ! exists_var( "model_version" ) || ! exists_var( "model_date" ) )
	{
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Cannot save configuration\" -detail \"Choose an existing model or create a new one.\"" );
		return;
	}

	// try to load existing XML as base
	snprintf( fName, MAX_PATH_LENGTH, "%s%s%s", s, strlen( s ) > 0 ? "/" : "", MODEL_XML_CONFIG );

	if ( modCfg.load_file( fName, PUGI_LOAD_OPTIONS ).status != pugi::status_ok )
		modCfg.reset( );								// recreate XML structure

	// try to read configuration nodes
	x_nodeT lsdNode = modCfg.document_element( );		// LSD top element
	x_nodeT modNode = lsdNode.child( "model" );			// load model data
	x_nodeT setNode = modNode.child( "settings" );		// model settings
	x_nodeT geoNode = modNode.child( "geometry" );		// model current geometry
	x_nodeT cfgNode = modNode.child( "configuration" );	// model current config.

	// recreate XML structure if wrong format XML
	if ( strcmp( lsdNode.name( ), "LSD" ) != 0 || strcmp( modNode.name( ), "model" ) != 0 )
	{
		modCfg.reset( );								// recreate all
		x_nodeT typeNode = modCfg.append_child( pugi::node_declaration );
		typeNode.append_attribute( "version" ) = "1.0";
		typeNode.append_attribute( "encoding" ) = "ISO-8859-1";
		typeNode.append_attribute( "standalone" ) = "yes";
		modCfg.append_child( pugi::node_doctype ).set_value( "LSD [\n \
		<!ELEMENT LSD (model)>\n \
		<!ELEMENT model (settings, geometry, configuration?, makefile)>\n \
		<!ELEMENT settings (#PCDATA)>\n \
		<!ELEMENT geometry (#PCDATA)>\n \
		<!ELEMENT configuration (#PCDATA)>\n \
		<!ELEMENT makefile (#PCDATA)>\n]" );
		lsdNode = modCfg.append_child( "LSD" );
		modNode = lsdNode.append_child( "model" );
	}

	// add/update model configuration attributes
	if ( ( attr = modNode.attribute( "version" ) ) == NULL )
		attr = modNode.append_attribute( "version" );

	attr = "1.0";

	if ( ( attr = modNode.attribute( "description" ) ) == NULL )
		attr = modNode.append_attribute( "description" );

	attr = "LSD model configuration file";

	// add/update model settings and geometry (legacy configuration content)
	if ( setNode.empty( ) )
		setNode = modNode.append_child( "settings" );

	if ( geoNode.empty( ) )
		geoNode = modNode.append_child( "geometry" );

	if ( cfgNode.empty( ) && exists_var( "last_conf" ) && eval_bool( "$last_conf ne \"\" && $last_conf ne \"#\"" ) )
		cfgNode = modNode.append_child( "configuration" );

	for ( i = 0; i < MODEL_OPTIONS_NUM; ++i )
	{
		cmd( "if { ! [ info exists %s ] } { set %s \"\" }", model_options[ i ], model_options[ i ] );
		cmd( "if { $%s eq \"\" && \"%s\" ne \"#\" } { set %s \"%s\" }", model_options[ i ], model_defaults[ i ], model_options[ i ], model_defaults[ i ] );

		s = get_str( model_options[ i ] );
		if ( strcmp( s, "#" ) == 0 )
			s = "";									// handle empty items

		switch ( model_types[ i ] )
		{
			case 's':								// setting node
				if ( ( child = setNode.child( model_options[ i ] ) ) == NULL )
					child = setNode.append_child( model_options[ i ] );

				child.text( ) = s;
				break;

			case 'g':								// geometry node
				if ( ( child = geoNode.child( model_options[ i ] ) ) == NULL )
					child = geoNode.append_child( model_options[ i ] );

				child.text( ) = s;
				break;

			case 'a':								// configuration attribute
				if ( cfgNode.empty( ) )
					break;

				if ( ( attr = cfgNode.attribute( model_options[ i ] ) ) == NULL )
					attr = cfgNode.append_attribute( model_options[ i ] );

				attr = s;
				break;

			case 'c':								// configuration node
				if ( cfgNode.empty( ) )
					break;

				if ( ( child = cfgNode.child( model_options[ i ] ) ) == NULL )
					child = cfgNode.append_child( model_options[ i ] );

				child.text( ) = s;
				break;
		}
	}

	// save additonal model info
	if ( exists_var( "model_dir" ) )
	{
		if ( ( child = setNode.child( "model_path" ) ) == NULL )
			child = setNode.append_child( "model_path" );

		child.text( ).set( get_str( "model_dir" ) );
	}

	// save model makefile options
	x_nodeT makNode = modNode.child( "makefile" );	// LMM configuration
	if ( makNode.empty( ) )
		makNode = modNode.append_child( "makefile" );

	if ( ( child = makNode.first_child( ) ) == NULL )
		child = makNode.append_child( pugi::node_cdata );

	if ( model_make != NULL && strlen( model_make ) > 0 )
	{
		s = lsd::strencdata( NULL, model_make );
		child.text( ) = s;
		delete [ ] s;
	}

	// save to file
	if ( ! modCfg.save_file( fName ) )
	{
		gui::log_tcl_error( false, "Cannot save model configuration", "Model configuration file cannot be saved to the model directory.\nnCheck if the model home directory is not set READ-ONLY or if it has enough space, and try again" );
		cmd( "tk_messageBox -type ok -icon error -title Error -message \"Cannot save model configuration\" -detail \"File '%s' cannot be saved to directory\n\n[ file nativename [ file normalize $model_dir ] ]\n\nCheck if the user model directory is not set READ-ONLY or if it is not full, and try again.\"", MODEL_XML_CONFIG );
	}
}


/*************************************************************
 TCL_GET_MODEL_SETTING
 Entry point function for access from the Tcl interpreter
 to get the specified setting for a given model folder
 containing the proper XML configuration file
 *************************************************************/
int gui::Tcl_get_model_setting( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	char fName[ MAX_PATH_LENGTH ], line[ MAX_LINE_SIZE ], set_val[ MAX_LINE_SIZE ] = "";
	const char *dir;
	int setID, i;
	x_docT modCfg;
	FILE *f;

	if ( argc != 3 || argv[ 1 ] == NULL || argv[ 2 ] == NULL || strlen( argv[ 1 ] ) == 0 || strlen( argv[ 2 ] ) == 0 )// require 2 param.
		return TCL_ERROR;

	cmd( "set f [ file normalize \"%s\" ]", argv[ 1 ] );
	dir = get_str( "f" );
	if ( ! eval_bool( "[ file exists $f ] || ! [ file isdirectory $f ]" ) )
		return TCL_ERROR;

	for ( setID = 0; setID < MODEL_OPTIONS_NUM; ++setID )
		if ( strcmp( argv[ 2 ], model_options[ setID ] ) == 0 )
			break;

	if ( setID == MODEL_OPTIONS_NUM )
		return TCL_ERROR;

	snprintf( fName, MAX_PATH_LENGTH, "%s/%s", dir, MODEL_XML_CONFIG );
	if ( modCfg.load_file( fName, PUGI_LOAD_OPTIONS ).status == pugi::status_ok )
	{
		x_nodeT lsdNode = modCfg.document_element( );	// LSD top element
		x_nodeT modNode = lsdNode.child( "model" );		// load model data
		x_nodeT setNode = modNode.child( "settings" );	// model settings

		if ( strcmp( lsdNode.name( ), "LSD" ) == 0 && setNode.child( argv[ 2 ] ) != NULL )
			lsd::strcpyn( set_val, setNode.child( argv[ 2 ] ).text( ).as_string( ), MAX_LINE_SIZE );
		else
			std::remove( fName );						// remove corrupt file
	}

	if ( strlen( set_val ) == 0 )
	{
		snprintf( fName, MAX_PATH_LENGTH, "%s/%s", dir, MODEL_TXT_INFO );
		if ( ( f = fopen( fName, "r" ) ) != NULL )
		{
			for ( i = 0; i <= setID && fgets( line, MAX_LINE_SIZE, f ) != NULL; ++i )
				if ( i == setID )
					lsd::strtrim( set_val, line, MAX_LINE_SIZE );

			fclose( f );
		}
	}

	Tcl_SetResult( interp, set_val, TCL_VOLATILE );
	return TCL_OK;
}


/*************************************************************
 TCL_SET_MODEL_SETTING
 Entry point function for access from the Tcl interpreter
 to set the specified setting for a given model folder
 containing the proper XML configuration file
 *************************************************************/
int gui::Tcl_set_model_setting( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	char fName[ MAX_PATH_LENGTH ], line[ MAX_LINE_SIZE ], buf[ MAX_BUFF_SIZE ] = "";
	const char *dir;
	int  setID, i;
	x_docT modCfg;
	x_nodeT child;
	FILE *f;

	if ( argc != 4 || argv[ 1 ] == NULL || argv[ 2 ] == NULL || argv[ 3 ] == NULL || strlen( argv[ 1 ] ) == 0 || strlen( argv[ 2 ] ) == 0 || strlen( argv[ 3 ] ) == 0 )// require 3 param.
		return TCL_ERROR;

	cmd( "set f [ file normalize \"%s\" ]", argv[ 1 ] );
	dir = get_str( "f" );
	if ( ! eval_bool( "[ file exists $f ] || ! [ file isdirectory $f ]" ) )
		return TCL_ERROR;

	for ( setID = 0; setID < MODEL_OPTIONS_NUM; ++setID )
		if ( strcmp( argv[ 2 ], model_options[ setID ] ) == 0 )
			break;

	if ( setID == MODEL_OPTIONS_NUM )
		return TCL_ERROR;

	snprintf( fName, MAX_PATH_LENGTH, "%s/%s", dir, MODEL_XML_CONFIG );
	if ( modCfg.load_file( fName, PUGI_LOAD_OPTIONS ).status == pugi::status_ok )
	{
		x_nodeT lsdNode = modCfg.document_element( );	// LSD top element
		x_nodeT modNode = lsdNode.child( "model" );		// load model data
		x_nodeT setNode = modNode.child( "settings" );	// model settings

		if ( strcmp( lsdNode.name( ), "LSD" ) != 0 || strcmp( modNode.name( ), "model" ) != 0 || strcmp( setNode.name( ), "settings" ) != 0 )
			std::remove( fName );						// remove corrupt file
		else
		{
			if ( ( child = setNode.child( argv[ 2 ] ) ) == NULL )
				child = setNode.append_child( argv[ 2 ] );

			child.text( ) = argv[ 3 ];
			modCfg.save_file( fName );
		}
	}

	snprintf( fName, MAX_PATH_LENGTH, "%s/%s", dir, MODEL_TXT_INFO );
	if ( ( f = fopen( fName, "r" ) ) != NULL )
	{
		for ( i = 0; i < MODEL_OPTIONS_NUM && fgets( line, MAX_LINE_SIZE, f ) != NULL; ++i )
			if ( i != setID )
				lsd::strcatn( buf, line, MAX_BUFF_SIZE );
			else
			{
				lsd::strcatn( buf, argv[ 3 ], MAX_BUFF_SIZE );
				lsd::strcatn( buf, "\n", MAX_BUFF_SIZE );
			}

		fclose( f );

		if ( i >= setID )								// ignore corrupt file
		{
			f = fopen( fName, "w" );
			fputs( buf, f );
			fclose( f );
		}
	}

	return TCL_OK;
}


/*************************************************************
 TCL_GET_GROUP_SETTING
 Entry point function for access from the Tcl interpreter
 to get the specified setting for a given group folder
 containing the proper XML configuration file
 *************************************************************/
int gui::Tcl_get_group_setting( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	bool rebuild_xml = true;
	char fNameXML[ MAX_PATH_LENGTH ], fNameTXT[ MAX_PATH_LENGTH ], line[ MAX_LINE_SIZE ], buf[ MAX_BUFF_SIZE ], desc[ MAX_BUFF_SIZE ] = "", set_val[ MAX_BUFF_SIZE ] = "";
	const char *dir;
	int setID, i;
	x_docT grpCfg;
	x_nodeT child, dscNode, grpNode, lsdNode;
 	FILE *f;

	if ( argc != 3 || argv[ 1 ] == NULL || argv[ 2 ] == NULL || strlen( argv[ 1 ] ) == 0 || strlen( argv[ 2 ] ) == 0 )// require 2 param.
		return TCL_ERROR;

	for ( setID = 0; setID < GROUP_OPTIONS_NUM; ++setID )
		if ( strcmp( argv[ 2 ], group_options[ setID ] ) == 0 )
			break;

	if ( setID == GROUP_OPTIONS_NUM )
		return TCL_ERROR;

	cmd( "set f [ file normalize \"%s\" ]", argv[ 1 ] );
	dir = get_str( "f" );
	if ( strcmp( dir, get_str( "::lsd_root") ) == 0 || strcmp( dir, eval_str( "[ file dirname [ file normalize $::group_new ] ]" ) ) == 0 )
	{
		if ( strcmp( argv[ 2 ], "name" ) == 0 )
			lsd::strcpyn( set_val, get_str( "::rootname" ), MAX_BUFF_SIZE );

		if ( strcmp( argv[ 2 ], "description" ) == 0 )
			snprintf( set_val, MAX_BUFF_SIZE, "%s group.\n\nAll groups are descendants of this group.", get_str( "::rootname" ) );

		goto end;
	}

	snprintf( fNameXML, MAX_PATH_LENGTH, "%s/%s", dir, GROUP_XML_CONFIG );
	if ( grpCfg.load_file( fNameXML, PUGI_LOAD_OPTIONS ).status == pugi::status_ok )
	{
		lsdNode = grpCfg.document_element( );			// LSD top element
		grpNode = lsdNode.child( "group" );				// load group data

		if ( strcmp( lsdNode.name( ), "LSD" ) == 0 && strcmp( grpNode.name( ), "group" ) == 0 )
		{
			rebuild_xml = false;
			switch ( group_types[ setID ] )
			{
				case 'p':								// PCDATA text node
					lsd::strcpyn( set_val, grpNode.child( argv[ 2 ] ).text( ).as_string( group_defaults[ setID ] ), MAX_BUFF_SIZE );
					break;

				case 'c':								// CDATA text node
					lsd::strdecdata( set_val, grpNode.child( argv[ 2 ] ).first_child( ).text( ).as_string( group_defaults[ setID ] ), MAX_BUFF_SIZE );
					break;
			}
		}
	}

	// (re)create XML file using legacy file information
	if ( rebuild_xml )
	{
		grpCfg.reset( );
		x_nodeT typeNode = grpCfg.append_child( pugi::node_declaration );
		typeNode.append_attribute( "version" ) = "1.0";
		typeNode.append_attribute( "encoding" ) = "ISO-8859-1";
		typeNode.append_attribute( "standalone" ) = "yes";
		grpCfg.append_child( pugi::node_doctype ).set_value( "LSD [\n \
		<!ELEMENT LSD (model)>\n \
		<!ELEMENT group (name, description)>\n \
		<!ELEMENT name (#PCDATA)>\n \
		<!ELEMENT description (#PCDATA)>\n]" );
		lsdNode = grpCfg.append_child( "LSD" );
		grpNode = lsdNode.append_child( "group" );

		str_vecT grpOptions( GROUP_OPTIONS_NUM, "" );
		snprintf( fNameTXT, MAX_PATH_LENGTH, "%s/%s", dir, GROUP_TXT_INFO );
		if ( ( f = fopen( fNameTXT, "r" ) ) != NULL )
		{
			for ( i = 0; i < GROUP_OPTIONS_NUM && fgets( line, MAX_LINE_SIZE, f ) != NULL; ++i )
				if ( strcmp( group_options[ i ], "description" ) != 0 )
				{
					lsd::strtrim( buf, line, MAX_LINE_SIZE );
					grpOptions[ i ] = buf;

					if ( i == setID )
						strcpy( set_val, buf );
				}

			fclose( f );
		}

		snprintf( fNameTXT, MAX_PATH_LENGTH, "%s/%s", dir, DESCRIPTION );
		if ( ( f = fopen( fNameTXT, "r" ) ) != NULL )
		{
			i = fread( ( void * ) buf, sizeof ( char ), MAX_BUFF_SIZE - 1, f );
			fclose( f );

			if ( i > 0 )
			{
				buf[ i ] = '\0';
				lsd::strtrim( desc, buf, MAX_BUFF_SIZE );

				if ( strcmp( argv[ 2 ], "description" ) == 0 )
					strcpy( set_val, desc );
			}
		}

		for ( i = 0; i < GROUP_OPTIONS_NUM; ++i )
		{
			if ( strcmp( group_options[ i ], "description" ) == 0 )
				grpOptions[ i ] = desc;

			if ( i == 0 )
			{
				if ( grpOptions[ 0 ].size( ) == 0 )
				{
					cmd( "set f [ file tail \"%s\" ]", dir );
					grpOptions[ 0 ] = get_str( "f" );
				}
			}
			else
				if ( grpOptions[ i ].size( ) == 0 )
					grpOptions[ i ] = group_defaults[ i ];

			switch ( group_types[ i ] )
			{
				case 'p':
					grpNode.append_child( group_options[ i ] ).text( ) = grpOptions[ i ].c_str( );
					break;

				case 'c':
					child = grpNode.append_child( group_options[ i ] );
					child = child.append_child( pugi::node_cdata );
					child.text( ) = grpOptions[ i ].c_str( );
					break;
			}
		}

		grpCfg.save_file( fNameXML );
	}

	end:

	Tcl_SetResult( interp, set_val, TCL_VOLATILE );
	return TCL_OK;
}


/*************************************************************
 TCL_SET_GROUP_SETTING
 Entry point function for access from the Tcl interpreter
 to set the specified setting for a given group folder
 containing the proper XML configuration file
 *************************************************************/
int gui::Tcl_set_group_setting( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	bool rebuild_xml = true;
	char fName[ MAX_PATH_LENGTH ], line[ MAX_LINE_SIZE ], buf[ MAX_BUFF_SIZE ];
	const char *dir;
	int setID, i;
	x_docT grpCfg;
	x_nodeT child;
	FILE *f;

	if ( argc != 4 || argv[ 1 ] == NULL || argv[ 2 ] == NULL || argv[ 3 ] == NULL || strlen( argv[ 1 ] ) == 0 || strlen( argv[ 2 ] ) == 0 || strlen( argv[ 3 ] ) == 0 )// require 3 param.
		return TCL_ERROR;

	cmd( "set f [ file normalize \"%s\" ]", argv[ 1 ] );
	dir = get_str( "f" );
	if ( strcmp( dir, get_str( "::lsd_root" ) ) == 0 )
		return TCL_ERROR;

	for ( setID = 0; setID < GROUP_OPTIONS_NUM; ++setID )
		if ( strcmp( argv[ 2 ], group_options[ setID ] ) == 0 )
			break;

	if ( setID == GROUP_OPTIONS_NUM )
		return TCL_ERROR;

	snprintf( fName, MAX_PATH_LENGTH, "%s/%s", dir, GROUP_XML_CONFIG );
	if ( grpCfg.load_file( fName, PUGI_LOAD_OPTIONS ).status == pugi::status_ok )
	{
		x_nodeT lsdNode = grpCfg.document_element( );	// LSD top element
		x_nodeT grpNode = lsdNode.child( "group" );		// load group data

		if ( strcmp( lsdNode.name( ), "LSD" ) == 0 && strcmp( grpNode.name( ), "group" ) == 0 )
		{
			rebuild_xml = false;
			switch ( group_types[ setID ] )
			{
				case 'p':								// PCDATA text node
					if ( ( child = grpNode.child( argv[ 2 ] ) ) == NULL )
						child = grpNode.append_child( argv[ 2 ] );

					child.text( ) = argv[ 3 ];
					break;

				case 'c':								// CDATA text node
					if ( ( child = grpNode.child( argv[ 2 ] ) ) == NULL )
						child = grpNode.append_child( argv[ 2 ] );

					if ( child.first_child( ) == NULL )
						child = grpNode.append_child( pugi::node_cdata );
					else
						child = child.first_child( );

					lsd::strtrim( buf, argv[ 3 ], MAX_BUFF_SIZE );
					char *desc = lsd::strencdata( NULL, argv[ 3 ] );
					child.text( ) = desc;
					delete [ ] desc;
					break;
			}

			grpCfg.save_file( fName );
		}
	}

	if ( strcmp( argv[ 2 ], "description" ) != 0 )
	{
		strcpy( buf, "" );
		snprintf( fName, MAX_PATH_LENGTH, "%s/%s", dir, GROUP_TXT_INFO );
		if ( ( f = fopen( fName, "r" ) ) != NULL )
		{
			for ( i = 0; i < GROUP_OPTIONS_NUM && fgets( line, MAX_LINE_SIZE, f ) != NULL; ++i )
				if ( i != setID )
					lsd::strcatn( buf, line, MAX_BUFF_SIZE );
				else
				{
					lsd::strcatn( buf, argv[ 3 ], MAX_BUFF_SIZE );
					lsd::strcatn( buf, "\n", MAX_BUFF_SIZE );
				}

			fclose( f );

			if ( i >= setID )							// ignore corrupt file
			{
				if ( ( f = fopen( fName, "w" ) ) != NULL )
				{
					fputs( buf, f );
					fclose( f );
				}
			}
		}
	}
	else
	{
		snprintf( fName, MAX_PATH_LENGTH, "%s/%s", dir, DESCRIPTION );
		if ( ( f = fopen( fName, "r" ) ) != NULL )		// update only if exists
		{
			fclose( f );

			if ( ( f = fopen( fName, "w" ) ) != NULL )
			{
				lsd::strtrim( buf, argv[ 3 ], MAX_BUFF_SIZE );
				fputs( buf, f );
				fclose( f );
			}
		}
	}

	if ( rebuild_xml )
	{
		cmd( "get_group_setting \"%s\" %s", argv[ 1 ], group_options[ 0 ] );
		cmd( "set_group_setting \"%s\" %s \"%s\"", argv[ 1 ], argv[ 2 ], argv[ 3 ] );
	}

	return TCL_OK;
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

	// LSD is exiting, do nothing
	if ( tcl_exit )
		return;

#ifndef _LMM_
	// abort if not running in main LSD thread
	if ( std::this_thread::get_id( ) != lsd::main_thread )
		return;
#endif

	// abort if Tcl interpreter not initialized
	if ( interp == NULL || ! tcl_ok )
	{
#ifndef _LMM_
		FILE *stderr_ptr = lsd::stderr_ptr;
#else
		FILE *stderr_ptr = stderr;
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
	static bool firstCall = true;
	static char *err_path, ftime[ 80 ], fname[ MAX_PATH_LENGTH ], buffer[ MAX_BUFF_SIZE ];
	static struct tm *timeinfo;
	static time_t rawtime;
	static va_list argptr;
	static FILE *f;

	l_guardT lock( lock_log_tcl_err );

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
 LSD_EXIT_GUI (DLL WRAPPER)
 exit LSD after the GUI is launched
 *************************************************************/
void gui::lsd_exit_gui( int v, bool clean )
{
#ifndef _LMM_
	choice = 11;

	delete sim.liblnk;
	delete [ ] eq_txt;
#endif

	delete [ ] model_make;
	delete [ ] system_make;

	if ( interp != NULL )
	{
		if ( tk_ok )
			cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { \
					catch { destroy . } \
				}" );

		tk_ok = false;

		if ( tcl_ok )
			Tcl_Finalize( );

		tcl_exit = true;
		tcl_ok = false;
	}

	set_env( false );

	lsd::lsd_exit( v, clean );
}


/*************************************************************
 TCL_LSD_EXIT_GUI
 Entry point function for access from the Tcl interpreter
 *************************************************************/
int gui::Tcl_lsd_exit_gui( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	int v, clean;

	if ( argc != 2 && argc != 3 )	// require 1/2 parameters: exit_value, and clean flag
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || sscanf( argv[ 1 ], "%d", & v ) != 1 )
		return TCL_ERROR;

	if ( argc == 3 && argv[ 2 ] != NULL && sscanf( argv[ 2 ], "%d", & clean ) == 1 )
		lsd_exit_gui( v, clean );
	else
		lsd_exit_gui( v );

	Tcl_SetResult( interp, ( char * ) "ok", TCL_VOLATILE );
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
 GET_MAKE_VAR
 Get the named variable from a make-formated string buffer
 *************************************************************/
const char *gui::get_make_var( const char *var, const char *buf, char *dest, int sz )
{
	strT buffer, pattern;
	std::regex regex;
	std::smatch match;

	buffer = buf;
	pattern = "(^|\n)[ \t]*";
	pattern	+= var;
	pattern += "[ \t]*=[ \t]*(.*)[ \t]*(?=\n|$)";
	regex = pattern;

	if ( ! std::regex_search( buffer, match, regex ) )
		return NULL;

	snprintf( dest, sz, "%s", match.str( 2 ).c_str( ) );

	return dest;
}


/*************************************************************
 GET_EQFILE_NAME
 Get the file name of the current equation file
 *************************************************************/
const char *gui::get_eqfile_name( char *s, int sz )
{
	if ( get_make_var( "FUN", model_make, s, sz ) == NULL || strlen( s ) == 0 )
	{
		cmd( "ttk::messageBox -parent . -type ok -title -title Error -icon error -message \"Configuration corrupted\" -detail \"Please check 'Model Options' and 'System Options' in LMM menu 'Model'.\"" );

		return NULL;
	}

	lsd::strcatn( s, ".cpp", sz );

	return s;
}


/*************************************************************
 GET_TARGET_NAME
 get current executable file name
 *************************************************************/
const char *gui::get_target_name( char *str, int str_sz, bool term )
{
	char buf[ MAX_PATH_LENGTH ], buf1[ MAX_PATH_LENGTH ];
	FILE *f;

	make_makefile( term );

	if ( term )					// term version use fixed name because of batches
	{
		snprintf( str, str_sz, "%s%s", LSD_TERM, platform == _WIN_ ? ".exe" : "" );
		return str;
	}

	cmd( "set fapp [ file normalize \"$model_dir/makefile%s\" ]", term ? "" : ".gui" );
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

	if ( strcmp( lsd::strupr( buf1 ), "LSD" ) == 0 )
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
bool gui::get_precompiled_flag( const char *exec, bool term )
{
	bool deftarg = true, precomp = true;		// defaults if settings are missing
	char buf[ MAX_PATH_LENGTH ], buf1[ MAX_PATH_LENGTH ];
	FILE *f;

	if ( ! term )
	{
		// non default executable name - cannot use precompiled code
		lsd::strcpyn( buf, exec, MAX_PATH_LENGTH );

		if ( platform == _WIN_ )
			lsd::strupr( buf );

		if ( strcmp( buf, platform == _WIN_ ? "LSD.EXE" : "LSD" ) != 0 )
			deftarg = false;
	}

	cmd( "set fapp [ file normalize \"$model_dir/makefile.gui\" ]" );
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
	lsd::strupr( buf1 );

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
	return false;
}


/*************************************************************
 MAKE_TERMINAL
 create a terminal version of LSD
 *************************************************************/
bool gui::make_terminal( void )
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
	cmd( "if { ! [ file exists \"$model_dir/$lsd_src\" ] } { \
			file mkdir \"$model_dir/$lsd_src\" \
		}" );

	for ( i = 0; i < LSD_TERM_NUM; ++i )
		cmd( "file copy -force \"$lsd_root/$lsd_src/%s\" \"$model_dir/$lsd_src\"", lsd_term_src[ i ] );

	// copy LSD library files always
	cmd( "if { ! [ file exists \"$model_dir/$lsd_src/lib\" ] } { \
			file mkdir \"$model_dir/$lsd_src/lib\" \
		}" );

	cmd( "foreach f [ glob -nocomplain -directory \"$lsd_root/$lsd_src/lib\" * ] { \
			file copy -force $f \"$model_dir/$lsd_src/lib\" \
		}" );

	// copy 3rd-party C++ libraries just once
	cmd( "if { ! [ file exists \"$model_dir/$lsd_src/clib\" ] } { \
			file copy -force \"$lsd_root/$lsd_src/clib\" \"$model_dir/$lsd_src\" \
		}" );

	// create makefile and compile a local machine version of LSD_TERM
	return compile_run( false, true );
}


/*************************************************************
 MAKE_MAKEFILE
 create makefiles to compile LSD
 *************************************************************/
void gui::make_makefile( bool term )
{
	if ( system_make == NULL || strlen( system_make ) == 0 )
		load_lsd_options( );

	if ( model_make == NULL || strlen( model_make ) == 0 )
	{
		cmd( "set fapp [ file normalize $model_dir ]" );
		load_model_options( get_str( "fapp" ) );
	}

	cmd( "set f [ open \"$lsd_root/$lsd_src/makefile-%s.txt\" r ]", term ? "term" : "gui" );
	cmd( "set b [ string trim [ read $f ] ]" );
	cmd( "close $f" );

	cmd( "set f [ open \"$model_dir/makefile%s\" w ]", term ? "" : ".gui" );
	cmd( "puts $f \"# Model compilation options\n\n%s\n\"", model_make );
	cmd( "puts $f {# System compilation options\n\n%s\n}", system_make );
	cmd( "puts $f \"# Body of makefile%s (from makefile-%s.txt)\n\n$b\"", term ? "" : ".gui", term ? "term" : "gui" );
	cmd( "close $f" );
}


/*************************************************************
 COMPILE_RUN
 compile LSD, GUI or command line
 and optionally execute it
 *************************************************************/
bool gui::compile_run( int run_mode, bool term )
{
	bool precompiled, ret = false;
	char str[ 2 * MAX_PATH_LENGTH ];
	const char *s;
	int res, max_threads = 1;
	FILE *f;

	Tcl_LinkVar( interp, "res", ( char * ) &res, TCL_LINK_INT );

	cmd( "set oldpath [ pwd ]" );
	cmd( "cd $model_dir" );

#ifdef _LMM_
	cmd( "destroytop .mm" );	// close any open compilation results window

	if ( ( s = get_str( "model_name" ) ) == NULL || ! strcmp( s, "" ) )
	{
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
		goto end;
	}
#endif

	// get source name
	if ( ( s = get_eqfile_name( str, 2 * MAX_PATH_LENGTH ) ) == NULL || ( f = fopen( s, "r" ) ) == NULL )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Equation file not found\" -detail \"File '%s' is no longer available in directory\n\n'[ file nativename $model_dir ]'\" ", s );
		goto end;
	}
	else
		fclose( f );

	cmd( "set fname \"%s\"", s );

	// get executable name
	cmd( "set mainExe %s", get_target_name( str, 2 * MAX_PATH_LENGTH ) );

	if ( term )
		get_target_name( str, 2 * MAX_PATH_LENGTH, term );

#ifdef _LMM_
	if ( run_mode == 0 && ! term )// delete existing object file if it's just compiling
	{							  // to force recompilation

		cmd( "set oldObj \"[ temp_dir ]/[ file rootname $mainExe ]/[ file tail $model_dir ]/[ file rootname [ lindex [ glob -nocomplain fun_*.cpp ] 0 ] ].o\"" );
		cmd( "if { [ file exists \"$oldObj\" ] } { file delete \"$oldObj\" }" );
	}

	precompiled = get_precompiled_flag( str, term );
#else
	precompiled = get_precompiled_flag( str, true );
#endif

	if ( ! term && precompiled )	// remove old unused executables
		cmd( "if { [ file exists %s ] } { file delete %s }", str, str );

	// show compilation banner
	cmd( "if { ( [ info exists auto_hide ] && ! $auto_hide ) || %d == 0 } { \
			set parWnd .; \
			set posWnd centerW \
		} else { \
			set parWnd \"\"; \
			set posWnd centerS \
		}", run_mode );

	cmd( "newtop .t \"Please Wait\" \"\" $parWnd" );

	if ( term )
		cmd( "ttk::label .t.l1 -style bold.TLabel -justify center -text \"Compiling terminal model executable...\"" );
	else
		cmd( "ttk::label .t.l1 -style bold.TLabel -justify center -text \"Compiling model...\"" );

	if ( run_mode != 0 )
		cmd( "ttk::label .t.l2 -justify center -text \"Just recompiling equation file(s) changes.\nOn success, the %s will be launched.\nOn failure, a new window will show the compilation errors.\"", run_mode != 2 ? "new model program" : "debugger" );
	else
		if ( term )
#ifdef _LMM_
			cmd( "ttk::label .t.l2 -justify center -text \"Creating terminal model executable ('%s').\nOn success, the model directory can be also ported to any computer.\nOn failure, a new window will show the compilation errors.\"", LSD_TERM );
#else
			cmd( "ttk::label .t.l2 -justify center -text \"Creating updated terminal model executable ('%s').\nOn success, the requested operation will continue.\"", LSD_TERM );
#endif
		else
			cmd( "ttk::label .t.l2 -justify center -text \"Recompiling the entire model program.\nOn success, the new program will NOT be launched.\nOn failure, a new window will show the compilation errors.\"" );

	cmd( "pack .t.l1 .t.l2 -padx $_5 -pady $_5" );
	cmd( "cancel .t b { set res 2 }");
	cmd( "showtop .t $posWnd" );

#ifdef _LMM_

	// minimize LMM if required
	cmd( "set res $auto_hide" );			// get auto hide status
	if ( res && run_mode != 0 )				// hide LMM?
		cmd( "wm iconify ." );

#endif

	// number of cores for make parallelization
	max_threads = thrT::hardware_concurrency( );

	// start compilation as a background task
	res = -1;
	cmd( "make_background %s %d %d %d ", str, max_threads, term, precompiled );

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
		cmd( "set res $auto_hide" );		// get auto hide status
		if ( run_mode != 0 && res )			// auto unhide LMM if necessary
			cmd( "focustop .f.t.t" );		// only reopen if error
		show_comp_result( term );			// show errors
	}
	else
	{
		if ( term )
			cmd( "ttk::messageBox -parent . -type ok -icon info -title \"Terminal Model Executable\" -message \"Compilation successful\" -detail \"A non-graphical, command-line model program was created.\n\nThe executable '%s\\[.exe\\]' for this computer was generated in your model directory. It can be ported to any computer with a GCC-compatible compiler, like a high-performance server.\n\nTo port the model, copy the entire model directory:\n\n[ fn_break [ file nativename \"$model_dir\" ] 40 ]\n\nto another computer (including the subdirectory '$lsd_src'). After the copy, use the following steps to use it:\n\n- open the command-line terminal/shell\n- change to the copied model directory ('cd')\n- recompile with the command:\n\nmake\n\n- run the model program with a preexisting model configuration file ('.lsd' extension) using the command:\n\n./%s -f CONF_NAME.lsd\n\n(you may have to remove the './' in Windows)\n\nSimulations run in the command-line will save the results into files with '.res\\[.gz\\]' and '.tot\\[.gz\\]' extensions.\"", LSD_TERM, LSD_TERM );
		else
		{
			if ( run_mode != 0 )				// no problem - execute
			{
				// create the element list file in background and try to open 10 times every 50 ms
				cmd( "after 0 { create_elem_file $model_dir }" );
				cmd( "update" );

				if ( run_mode == 1 )			// run executable directly (not debugger)
				{
					cmd( "set n 10" );
					cmd( "set result \"\"" );

					switch ( platform )
					{
						case _LIN_:
							cmd( "while { [ catch { exec -- sh -c \"LD_LIBRARY_PATH=[ pwd ] %s/%s\" & } result ] && $n > 0 } { incr n -1; after 50 }", precompiled ? lsd::root_lsd : ".", str );
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
				cmd( "create_elem_file $model_dir" );
		}

		ret = true;
	}

#else

	if ( res == 0 )							// compilation failure?
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Compilation failed\" -detail \"The terminal model executable ('%s') could not be compiled, likely due to a syntax problem.\n\nPlease go to LMM, choose menu 'Model'/'Create Terminal Executable' to recompile, and check the Compilation Errors window for details on the problem(s).\"", LSD_TERM );
	}
	else
		ret = true;

#endif

	// update terminal executable time if not recompiled
	if ( term && ret )
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
void gui::show_comp_result( bool term )
{
	cmd( "set cerr 1.0" );						// search start position in file
	cmd( "set error \" error:\"" );				// error string to be searched
	cmd( "set errfil \"\"" );
	cmd( "set errlin \"\"" );
	cmd( "set errcol \"\"" );

	cmd( "newtop .mm \"Compilation Errors%s\" { .mm.b.close invoke } \"\"", term ? " (Terminal executable)" : "" );

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

	cmd( "pack .mm.i.f .mm.i.l .mm.i.c -padx $_10 -pady $_5 -side left" );
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

	cmd( "showtop .mm lefttoW yes yes no" );
	cmd( "mousewarpto .mm.b.gerr 0" );

	cmd( "if [ file exists \"$model_dir/makemessage.txt\" ] { set file [ open \"$model_dir/makemessage.txt\" ]; .mm.t.t insert end [ read -nonewline $file ]; close $file } { .mm.t.t insert end \"(no compilation errors)\" }" );
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
strT gui::win_path( strT filepath )
{
	strT winpath;

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
