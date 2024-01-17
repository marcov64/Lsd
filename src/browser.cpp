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
BROWSER.CPP
Contains the code that manages the LSD Browser, the main GUI
element. It performs:
- early initialization (Tcl/Tk GUI).
- the main cycle: browse a model, configure it, run simulation,
return to the browser, and so on.

The main functions contained here are:

- load_gui( argv )
Initializes the Tcl/Tk environment and passes control to the
LSD browser.

*************************************************************/

#include "LSD.h"


/*********************************
 LOAD_GUI
 *********************************/
int load_gui( const char **argv )
{
	char *str, cwd[ PATH_MAX ];
	const char *app;
	int i, j = 0, k = 0;
	object *r;
	FILE *f;

	// assume exec path is current path
	getcwd( cwd, PATH_MAX );
	set_exec( cwd, argv[ 0 ] );

	if ( exec_file == NULL || exec_path == NULL )
	{
		log_tcl_error( true, "Invalid LSD executable name or path", "Make sure the LSD directory is not too deep into the disk directory tree" );
		return 1;
	}

	for ( i = 1; argv[ i ] != NULL; i++ )
	{
		if ( argv[ i ][ 0 ] != '-' || ( argv[ i ][ 1 ] != 'f' && argv[ i ][ 1 ] != 'i' && argv[ i ][ 1 ] != 'c' ) )
		{
			log_tcl_error( true, "Command line parameters", "Invalid option, available options: -i TCL_DIRECTORY / -f MODEL_NAME / -c MAX_THREADS" );
			return 1;
		}

		if ( argv[ i ][ 1 ] == 'f' )
		{
			delete [ ] simul_name;
			simul_name = new char[ strlen( argv[ i + 1 ] ) + 5 ];
			str = new char[ strlen( argv[ i + 1 ] ) + 1 ];
			strcpy( simul_name, argv[ i + 1 ] );
			strcpy( str, argv[ i + 1 ] );
			strupr( str );

			if ( strlen( str ) > 0 && strstr( str, ".LSD" ) != NULL )
				simul_name[ strstr( str, ".LSD" ) - str ] = '\0';
			else
				strcpy( simul_name, "" );

			delete [ ] str;
			i++;
		}

		if ( argv[ i ][ 1 ] == 'i' )
		{
			strcpyn( tcl_dir, argv[ i + 1 ] + 2, MAX_PATH_LENGTH );
			i++;
		}

		// read -c parameter : max number of cores
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'c' )
		{
			sscanf( argv[ i + 1 ], "%d:%d", &j, &k );
			continue;
		}
	}

#ifndef _NP_

	if ( j > 0 && j < max_threads )
		max_threads = j;

#endif

	// initialize tcl/tk and set global bidirectional variables
	init_tcl_tk( argv[ 0 ], "lsd" );
	Tcl_LinkVar( interp, "choice", ( char * ) &choice, TCL_LINK_INT );
	Tcl_LinkVar( interp, "choice_g", ( char * ) &choice_g, TCL_LINK_INT );
	Tcl_LinkVar( interp, "stop", ( char * ) &stop, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( interp, "debug_flag", ( char * ) &debug_flag, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( interp, "when_debug", ( char * ) &when_debug, TCL_LINK_INT );

	// set system defaults in tcl
	cmd( "set LMM_OPTIONS \"%s\"", LMM_OPTIONS );
	cmd( "set SYSTEM_OPTIONS \"%s\"", SYSTEM_OPTIONS );
	cmd( "set MODEL_OPTIONS \"%s\"", MODEL_OPTIONS );
	cmd( "set GROUP_INFO \"%s\"", GROUP_INFO );
	cmd( "set MODEL_INFO \"%s\"", MODEL_INFO );
	cmd( "set MODEL_INFO_NUM %d", MODEL_INFO_NUM );
	cmd( "set DESCRIPTION \"%s\"", DESCRIPTION );
	cmd( "set DATE_FMT \"%s\"", DATE_FMT );

	// check if exec file is in current path
	i = strlen( exec_path ) + strlen( exec_file ) + 1;
	str = new char[ i ];
	snprintf( str, i, "%s/%s", exec_path, exec_file );
	f = fopen( str, "r" );
	delete [ ] str;
	if ( f != NULL )
		fclose( f );

	// try to use exec_path to change to the model directory
	if ( f == NULL || strlen( exec_path ) == 0 || ! strcmp( exec_path, "/" ) )
	{	// try to get name from Tcl
		cmd( "if { [ info nameofexecutable ] != \"\" } { set path [ file dirname [ info nameofexecutable ] ] } { set path \"\" }" );
		app = get_str( "path" );
		if ( app != NULL && strlen( app ) > 0 )
		{
			delete [ ] exec_path;
			exec_path = new char[ strlen( app ) + 1 ];
			strcpy( exec_path, app );
		}
	}

	// check if directory is ok and if executable is inside a macOS package
	cmd( "set path [ file normalize \"%s\" ]", exec_path );
	cmd( "if { $tcl_platform(os) eq \"Darwin\" } { \
			set pathsplit [ file split \"$path\" ]; \
			if { [ lindex $pathsplit end ] eq \"MacOS\" && [ lindex $pathsplit end-1 ] eq \"Contents\" } { \
				set path [ file normalize \"$path/../../..\" ] \
			}; \
			unset pathsplit \
		}" );

	cmd( "set modelDir \"$path\"" );
	cmd( "cd \"$path\"" );
	app = get_str( "path" );
	delete [ ] path;
	path = new char[ strlen( app ) + 1 ];
	strcpy( path, app );
	delete [ ] exec_path;
	exec_path = new char[ strlen( app ) + 1 ];
	strcpy( exec_path, app );

	// check if LSDROOT already exists and use it if so, if not, search the current directory tree
	cmd( "if [ info exists env(LSDROOT) ] { set RootLsd [ file normalize $env(LSDROOT) ]; if { ! [ file exists \"$RootLsd/src/interf.cpp\" ] } { unset RootLsd } }" );

	// do some search for the right path to cope with Mac Acqua package
	choice = 0;
	cmd( "if { ! [ info exists RootLsd ] } { \
			set here [ pwd ]; \
			while { ! [ file exists \"src/interf.cpp\" ] && ! [ string equal [ pwd ] \"/\" ] && [ string length [ pwd ] ] > 3 } { \
				cd .. \
			}; \
			if [ file exists \"src/interf.cpp\" ] { \
				set RootLsd [ pwd ] \
			} { \
				set choice 1 \
			}; \
			cd $here; \
		}" );

	if ( choice )
	{
		log_tcl_error( false, "LSDROOT check", "LSDROOT not set, make sure the environment variable LSDROOT points to the directory where LSD is installed" );
		cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"LSDROOT not set\" -detail \"Please make sure the environment variable LSDROOT points to the directory where LSD is installed.\n\nLSD is aborting now.\"" );
		return 9;
	}

	cmd( "set env(LSDROOT) $RootLsd" );

	app = get_str( "RootLsd" );
	if ( app != NULL && strlen( app ) > 0 )
	{
		rootLsd = new char[ strlen( app ) + 1 ];
		strcpy( rootLsd, app );
		rootLsd = clean_path( rootLsd );
		cmd( "set RootLsd \"%s\"", rootLsd );
	}
	else
	{
		log_tcl_error( false, "LSD directory check", "Cannot locate LSD folder on disk, check the installation of LSD and reinstall LSD if the problem persists" );
		cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"LSD directory missing\" -detail \"Cannot locate the LSD installation folder on disk.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"" );
		return 9;
	}

	// load/check LMM configuration file
	i = load_lmm_options( );

	// load required Tcl/Tk data, procedures and packages (error coded by file/bit position)
	choice = 0;

	// load native Tk procedures for graphical user interface management
	cmd( "if [ file exists \"$RootLsd/$LsdSrc/gui.tcl\" ] { if [ catch { source \"$RootLsd/$LsdSrc/gui.tcl\" } err0x01 ] { set choice [ expr { $choice + %d } ] } } { set choice [ expr { $choice + %d } ] }", 0x0100, 0x01 );

	// load native Tcl procedures for general utilities
	cmd( "if [ file exists \"$RootLsd/$LsdSrc/file.tcl\" ] { if [ catch { source \"$RootLsd/$LsdSrc/file.tcl\" } err0x02 ] { set choice [ expr { $choice + %d } ] } } { set choice [ expr { $choice + %d } ] }", 0x0200, 0x02 );

	// load additional native Tcl procedures for external files handling
	cmd( "if [ file exists \"$RootLsd/$LsdSrc/util.tcl\" ] { if [ catch { source \"$RootLsd/$LsdSrc/util.tcl\" } err0x04 ] { set choice [ expr { $choice + %d } ] } } { set choice [ expr { $choice + %d } ] }", 0x0400, 0x04 );

	if ( choice != 0 )
	{
		log_tcl_error( false, "Source files check failed", "Required Tcl/Tk source file(s) missing or corrupted (0x%04x), check your installation and reinstall LSD if the problem persists\n\n0x01: %s\n\n0x02: %s\n\n0x04: %s", choice, get_str( "err0x01" ), get_str( "err0x02" ), get_str( "err0x04" ) );
		cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"File(s) missing or corrupted\" -detail \"Some critical Tcl files (0x%04x) are missing or corrupted.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"", choice );
		return 200 + choice;
	}

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
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Unsupported platform\" -detail \"Your computer operating system is not supported by this LSD version,\nyou may try an older version compatible with legacy systems\n(Windows 32-bit, Mac OS X, etc.)\n\nLSD is aborting now.\"", choice );
				return 200;
			}

	// fix non-existent or old options file for new options
	if ( i == 0 )
		update_lmm_options( );			// update config file

	// create a Tcl command that calls the C discard_change function before killing LSD
	Tcl_CreateCommand( interp, "discard_change", Tcl_discard_change, NULL, NULL );

	// Tcl command to check before exiting with running background threads
	Tcl_CreateCommand( interp, "abort_run_threads", Tcl_abort_run_threads, NULL, NULL );

	// create Tcl commands that get and set LSD object/variable properties
	Tcl_CreateCommand( interp, "get_obj_conf", Tcl_get_obj_conf, NULL, NULL );
	Tcl_CreateCommand( interp, "set_obj_conf", Tcl_set_obj_conf, NULL, NULL );
	Tcl_CreateCommand( interp, "get_var_conf", Tcl_get_var_conf, NULL, NULL );
	Tcl_CreateCommand( interp, "set_var_conf", Tcl_set_var_conf, NULL, NULL );

	// create a Tcl command to set a c variable when not in a Tcl idle loop
	Tcl_CreateCommand( interp, "set_c_var", Tcl_set_c_var, NULL, NULL );

	// create a Tcl command to get LSD variable description from equation file(s)
	Tcl_CreateCommand( interp, "get_var_descr", Tcl_get_var_descr, NULL, NULL );

	// create a Tcl command to set tooltip from LSD variable description
	Tcl_CreateCommand( interp, "set_ttip_descr", Tcl_set_ttip_descr, NULL, NULL );

	// create Tcl command to upload series data
	Tcl_CreateObjCommand( interp, "upload_series", Tcl_upload_series, NULL, NULL );

	// Tcl command to save message to LSD log
	Tcl_CreateCommand( interp, "log_tcl_error", Tcl_log_tcl_error, NULL, NULL );

	// Tcl global variables
	cmd( "set small_character [ expr { $dim_character - $deltaSize } ]" );
	cmd( "set gpterm \"\"" );

	// load/check model equation file
	read_eqfile_name( equation_name, MAX_PATH_LENGTH );
	eq_file = load_eqfile( );

	// load/check model information file and fix if required
	if ( ! load_model_info( exec_path ) )
		update_model_info( true );

	// check model configuration file
	if ( eval_bool( "[ info exists lastConf ] && [ file exists $lastConf ] && [ file isfile $lastConf ]" ) )
	{
		delete [ ] simul_name;
		cmd( "set fn [ string map -nocase [ list [ file extension $lastConf ] \"\" ] [ file tail $lastConf ] ]" );
		simul_name = new char[ eval_int( "[ string length $fn ]" ) + 1 ];
		strcpy( simul_name, get_str( "fn" ) );

		cmd( "set path [ file normalize [ file dirname $lastConf ] ]" );
		if ( eval_bool( "$path ne [ pwd ]" ) )
		{
			delete [ ] path;
			path = new char[ eval_int( "[ string length $path ]" ) + 1 ];
			strcpy( path, get_str( "path" ) );
			cmd( "cd $path" );
		}
	}

	// try to load model configuration file
	if ( strlen( simul_name ) > 0 )
	{
		struct_file = new char[ strlen( path ) + strlen( simul_name ) + 6 ];
		sprintf( struct_file, "%s%s%s.lsd", path, strlen( path ) > 0 ? "/" : "", simul_name );
		snprintf( name_rep, MAX_PATH_LENGTH, "report_%s.html", simul_name );

		i = open_configuration( ( r = NULL ), true );
	}
	else
		i = 0;

	// failed configuration
	if ( i == 0 )
	{
		delete [ ] simul_name;
		delete [ ] struct_file;
		simul_name = new char[ strlen( "" ) + 1 ];
		struct_file = new char[ strlen( "" ) + 1 ];
		strcpy( simul_name, "" );
		strcpy( struct_file, "" );
		strcpy( name_rep, "" );
		cmd( "cd \"%s\"", exec_path );
	}

	grandTotal = true;				// not in parallel mode: use .tot headers

	// configure main window
	cmd( ". configure -menu .m -background $colorsTheme(bg)" );
	cmd( "icontop . lsd" );
	cmd( "sizetop .lsd" );
	cmd( "setglobkeys ." );			// set global keys for main window
	cmd( "setstyles" );				// set ttk custom style
	cmd( "init_canvas_colors" );

	create_logwindow( );

	// set dynamic link library (DLL) call-back references
	inter = interp;
	liblnk.center_plot = & center_plot;
	liblnk.cmd_backend = & cmd_backend;
	liblnk.cover_browser = & cover_browser;
	liblnk.deb = & deb;
	liblnk.deb_log = & deb_log;
	liblnk.disable_plot = & disable_plot;
	liblnk.enable_plot = & enable_plot;
	liblnk.error_hard_helper = & error_hard_helper;
	liblnk.init_lattice_helper = & init_lattice_helper;
	liblnk.log_tcl_error = & log_tcl_error;
	liblnk.plog_backend = & plog_backend;
	liblnk.plot_rt = & plot_rt;
	liblnk.prepare_plot = & prepare_plot;
	liblnk.print_stack = & print_stack;
	liblnk.reset_plot = & reset_plot;
	liblnk.scroll_plot = & scroll_plot;
	liblnk.save_lattice_helper = & save_lattice_helper;
	liblnk.show_prof_aggr = & show_prof_aggr;
	liblnk.uncover_browser = & uncover_browser;
	liblnk.update_lattice_helper = & update_lattice_helper;

	while ( 1 )						// main GUI loop: create/edit configuration - run
	{
		create( );					// open LSD browser

		try
		{
			if ( ( i = run( ) ) != 0 )
				return i;
		}
		catch( int p )				// return point from error_hard() (in object.cpp)
		{
			if ( p != 919293 )		// check throw signature
				throw;
			quit = 0;
		}
		catch ( ... )				// send the rest upward
		{
			throw;
		}
	}

	Tcl_UnlinkVar( interp, "choice" );
	Tcl_UnlinkVar( interp, "choice_g" );
	Tcl_UnlinkVar( interp, "stop" );
	Tcl_UnlinkVar( interp, "debug_flag" );
	Tcl_UnlinkVar( interp, "when_debug" );

	set_env( false );

	return 0;
}
