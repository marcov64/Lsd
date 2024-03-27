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

 It is re-build any time the browser window changes. There are
 many actions that are commanded from the browser window,
 implemented as a switch in operate (INTERF.CPP).

 The main functions contained in this file are:

 - load_gui( argv )
 Initializes the Tcl/Tk environment and passes control to the
 LSD browser.

 - lsd::object *create( void )
 The main cycle for the Browser, from which it exits only to
 run a simulation or to quit the program. The cycle is just
 once call to browse followed by a call to operate.

 - int browse( );
 build the browser window and waits for an action (on the form
 of values for choice or choice_g different from 0)
 *************************************************************/

#include "LSD.h"

#define NUM_BAD_CHOICES ( sizeof( badChoices ) / sizeof( badChoices[ 0 ] ) )
#define NUM_REDO_CHOICES ( sizeof( redoChoices ) / sizeof( redoChoices[ 0 ] ) )

namespace gui
{
	// list of choices that are bad with existing run data
	int badChoices[ ] = { 1, 2, 3, 6, 7, 9, 19, 21, 22, 27, 28, 30, 31, 32, 33, 36, 43, 57, 58, 59, 62, 63, 64, 65, 68, 69, 71, 72, 74, 75, 76, 77, 78, 79, 80, 81, 83, 88, 90, 91, 92, 93, 94, 95, 96 };

	// list of choices that are run twice (called from another choice)
	int redoChoices[ ] = { 32, 33, 55, 73, 74, 75, 76, 77, 78, 79, 80, 83, 96 };

	// comparison functions
	int comp_ints ( const void *a, const void *b ) { return ( *( int * ) a - *( int * ) b ); }
}


/*************************************************************
 LOAD_GUI
 *************************************************************/
int gui::load_gui( const char **argv )
{
	char *str, cwd[ PATH_MAX ];
	const char *app, *app1;
	int i, j = 0, k = 0;
	lsd::object *r;
	FILE *f;

	// assume exec path is current path
	getcwd( cwd, PATH_MAX );
	lsd::set_exec( cwd, argv[ 0 ] );

	if ( lsd::exec_file == NULL || lsd::exec_path == NULL )
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
			delete [ ] sim.conf_name;
			sim.conf_name = new char[ strlen( argv[ i + 1 ] ) + 5 ];
			str = new char[ strlen( argv[ i + 1 ] ) + 1 ];
			strcpy( sim.conf_name, argv[ i + 1 ] );
			strcpy( str, argv[ i + 1 ] );
			strupr( str );

			if ( strlen( str ) > 0 && strstr( str, ".LSD" ) != NULL )
				sim.conf_name[ strstr( str, ".LSD" ) - str ] = '\0';
			else
				strcpy( sim.conf_name, "" );

			delete [ ] str;
			i++;
		}

		if ( argv[ i ][ 1 ] == 'i' )
		{
			lsd::strcpyn( tcl_dir, argv[ i + 1 ] + 2, MAX_PATH_LENGTH );
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
	if ( j > 0 && j < sim.max_threads )
		sim.max_threads = j;
#endif

	// initialize tcl/tk and set global bidirectional variables
	init_tcl_tk( argv[ 0 ], "lsd" );

	// global links between C and tcl variables
	Tcl_LinkVar( interp, "choice", ( char * ) & choice, TCL_LINK_INT );
	Tcl_LinkVar( interp, "choice_g", ( char * ) & choice_g, TCL_LINK_INT );
	Tcl_LinkVar( interp, "stop", ( char * ) & stop, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( interp, "deb_set", ( char * ) & sim.deb_set, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( interp, "deb_t", ( char * ) & sim.deb_t, TCL_LINK_INT );

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
	i = strlen( lsd::exec_path ) + strlen( lsd::exec_file ) + 2;
	str = new char[ i ];
	snprintf( str, i, "%s/%s", lsd::exec_path, lsd::exec_file );
	f = fopen( str, "r" );
	delete [ ] str;
	if ( f != NULL )
		fclose( f );

	// try to use exec_path to change to the model directory
	if ( f == NULL || strlen( lsd::exec_path ) == 0 || ! strcmp( lsd::exec_path, "/" ) )
	{	// try to get name from Tcl
		cmd( "if { [ info nameofexecutable ] != \"\" } { \
				set path [ file dirname [ info nameofexecutable ] ]; \
				set exec [ file rootname [ info nameofexecutable ] ] \
			} { \
				set path \"\"; \
				set exec \"\" \
			}" );

		app = get_str( "path" );
		app1 = get_str( "exec" );
		if ( app != NULL && app1 != NULL && strlen( app1 ) > 0 )
			lsd::set_exec( app, app1 );
	}

	// check if executable is inside a macOS package
	cmd( "set path [ file normalize \"%s\" ]", lsd::exec_path );
	cmd( "if { $tcl_platform(os) eq \"Darwin\" } { \
			set pathsplit [ file split \"$path\" ]; \
			if { [ lindex $pathsplit end ] eq \"MacOS\" && [ lindex $pathsplit end-1 ] eq \"Contents\" } { \
				set path [ file normalize \"$path/../../..\" ] \
			}; \
			unset pathsplit \
		}" );

	// only use the exec path if not already in a model directory
	cmd( "if { [ file exists $MODEL_OPTIONS ] } { \
			set modelDir \"[ pwd ]\" \
		} { \
			set modelDir \"$path\"; \
		}" );

	cmd( "cd \"$modelDir\"" );
	app = get_str( "modelDir" );

	delete [ ] sim.conf_path;
	delete [ ] lsd::model_path;
	sim.conf_path = new char[ strlen( app ) + 1 ];
	lsd::model_path = new char[ strlen( app ) + 1 ];
	strcpy( sim.conf_path, app );
	strcpy( lsd::model_path, app );

	// check if LSDROOT already exists and use it if so, if not, search the current directory tree
	cmd( "if [ info exists env(LSDROOT) ] { set RootLsd [ file normalize $env(LSDROOT) ]; if { ! [ file exists \"$RootLsd/src/interf.cpp\" ] } { unset RootLsd } }" );

	// do some search for the right path to cope with Mac Acqua package
	choice = 0;
	cmd( "if { ! [ info exists RootLsd ] } { \
			set here [ pwd ]; \
			while { ! [ file exists \"src/interf.cpp\" ] && ! [ string equal [ pwd ] \"/\" ] && [ string length [ pwd ] ] > 3 } { \
				cd .. \
			}; \
			if [ file exists \"src/LSD.h\" ] { \
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
		lsd::root_lsd = new char[ strlen( app ) + 1 ];
		strcpy( lsd::root_lsd, app );
		lsd::root_lsd = lsd::clean_path( lsd::root_lsd );
		cmd( "set RootLsd \"%s\"", lsd::root_lsd );
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
	read_eqfile_name( eq_file, MAX_PATH_LENGTH );
	eq_txt = load_eqfile( );

	// load/check model information file and fix if required
	if ( ! load_model_info( lsd::model_path ) )
		update_model_info( true );

	// check model configuration file
	if ( eval_bool( "[ info exists lastConf ] && [ file exists $lastConf ] && [ file isfile $lastConf ]" ) )
	{
		delete [ ] sim.conf_name;
		cmd( "set fn [ string map -nocase [ list [ file extension $lastConf ] \"\" ] [ file tail $lastConf ] ]" );
		sim.conf_name = new char[ eval_int( "[ string length $fn ]" ) + 1 ];
		strcpy( sim.conf_name, get_str( "fn" ) );

		cmd( "set path [ file normalize [ file dirname $lastConf ] ]" );
		if ( eval_bool( "$path ne [ pwd ]" ) )
		{
			delete [ ] sim.conf_path;
			sim.conf_path = new char[ eval_int( "[ string length $path ]" ) + 1 ];
			strcpy( sim.conf_path, get_str( "path" ) );
			cmd( "cd $path" );
		}
	}

	// set dynamic link library (DLL) call-back references
	sim.inter = interp;
	sim.liblnk = new lsd::dlliblinkage;

	sim.liblnk->choice = & choice;
	sim.liblnk->cmd_backend = & cmd_backend;
	sim.liblnk->cover_browser = & cover_browser;
	sim.liblnk->debugger = & lsd::object::debugger;
	sim.liblnk->deb_log = & deb_log;
	sim.liblnk->disable_plot = & disable_plot;
	sim.liblnk->enable_plot = & enable_plot;
	sim.liblnk->error_hard_helper = & error_hard_helper;
	sim.liblnk->init_lattice_helper = & init_lattice_helper;
	sim.liblnk->log_tcl_error = & log_tcl_error;
	sim.liblnk->plog_backend = & plog_backend;
	sim.liblnk->plot_runtime = & lsd::variable::plot_runtime;
	sim.liblnk->print_stack = & print_stack;
	sim.liblnk->runtime_buttons = & runtime_buttons;
	sim.liblnk->runtime_end = & runtime_end;
	sim.liblnk->runtime_run_start = & runtime_run_start;
	sim.liblnk->runtime_run_end = & runtime_run_end;
	sim.liblnk->runtime_start = & runtime_start;
	sim.liblnk->runtime_step = & runtime_step;
	sim.liblnk->save_lattice_helper = & save_lattice_helper;
	sim.liblnk->update_lattice_helper = & update_lattice_helper;

	// try to load model configuration file
	if ( strlen( sim.conf_name ) > 0 )
	{
		sim.conf_file = new char[ strlen( sim.conf_path ) + strlen( sim.conf_name ) + 6 ];
		sprintf( sim.conf_file, "%s%s%s.lsd", sim.conf_path, strlen( sim.conf_path ) > 0 ? "/" : "", sim.conf_name );
		snprintf( sim.rep_file, MAX_PATH_LENGTH, "report_%s.html", sim.conf_name );

		i = open_configuration( r = NULL, true );
	}
	else
		i = 0;

	// failed configuration
	if ( i == 0 )
	{
		delete [ ] sim.conf_name;
		delete [ ] sim.conf_file;
		sim.conf_name = new char[ strlen( "" ) + 1 ];
		sim.conf_file = new char[ strlen( "" ) + 1 ];
		strcpy( sim.conf_name, "" );
		strcpy( sim.conf_file, "" );
		strcpy( sim.rep_file, "" );
		cmd( "cd \"%s\"", lsd::model_path );
	}

	// configure main window
	cmd( ". configure -menu .m -background $colorsTheme(bg)" );
	cmd( "icontop . lsd" );
	cmd( "sizetop .lsd" );
	cmd( "setglobkeys ." );			// set global keys for main window
	cmd( "setstyles" );				// set ttk custom style
	cmd( "init_canvas_colors" );

	create_logwindow( );

	while ( true )					// main GUI loop: create/edit configuration - run
	{
		create( );					// open LSD browser

		try
		{
			if ( ( i = sim.run_simulation( ) ) != 0 )
				break;
			else
				unsavedData = true;	// flag unsaved simulation results
		}
		catch( int p )				// return point from error_hard() (in object.cpp)
		{
			if ( p != 919293 )		// check throw signature
				throw;
			sim.quit = 0;
		}
		catch ( ... )				// send the rest upward
		{
			throw;
		}
	}

	delete sim.liblnk;

	Tcl_UnlinkVar( interp, "choice" );
	Tcl_UnlinkVar( interp, "choice_g" );
	Tcl_UnlinkVar( interp, "stop" );
	Tcl_UnlinkVar( interp, "deb_set" );
	Tcl_UnlinkVar( interp, "deb_t" );

	set_env( false );

	return 100 + i;
}


/*************************************************************
 CREATE
 *************************************************************/
void gui::create( void )
{
	lsd::object *r;

	Tcl_LinkVar( interp, "strWindowOn", ( char * ) &strWindowOn, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( interp, "eff_t", ( char * ) &sim.eff_t, TCL_LINK_INT );

	// sort the list of choices with existing run data to use later
	qsort( badChoices, NUM_BAD_CHOICES, sizeof ( int ), comp_ints );
	qsort( redoChoices, NUM_REDO_CHOICES, sizeof ( int ), comp_ints );

	cmd( "set ignore_eq_file %d", ignore_eq_file ? 1 : 0  );
	cmd( "set listfocus 1" );
	cmd( "set prevlistfocus 0" );
	cmd( "set itemfocus 0" );
	cmd( "set itemfirst 0" );
	cmd( "set c \"\"" );

	// restore previous object and cursor position in browser, if any
	r = sim.root->restore_pos( );
	redrawRoot = redrawStruc = true;	// browser/ structure redraw when drawing the first time
	choice_g = choice = 0;

	// main cycle
	while ( choice != 1 )
	{
		cmd( "wm title . \"%s%s - LSD Browser\"", unsaved_change( ) ? "*" : " ", strlen( sim.conf_name ) > 0 ? sim.conf_name : NO_CONF_NAME );
		cmd( "wm title .log \"%s%s - LSD Log\"", unsaved_change( ) ? "*" : " ", strlen( sim.conf_name ) > 0 ? sim.conf_name : NO_CONF_NAME );

		// find root and minimally check the configuration
		if ( sim.conf_ok && sim.root->v == NULL && sim.root->b == NULL )
		{
			sim.error_hard( "corrupted configuration file or internal problem in LSD",
							"if error persists, please contact developers",
							false,
							"invalid model configuration loaded" );
			unload_configuration_gui( true );
			r = sim.root;
		}

		if ( sim.message_logged )
		{
			cmd( "focustop .log" );
			sim.message_logged = false;
		}

		// browse only if not running two-cycle operations
		if ( bsearch( & choice, redoChoices, NUM_REDO_CHOICES, sizeof ( int ), comp_ints ) == NULL )
			choice = browse( r );

		// check if configuration was just reloaded
		if ( choice < 0 )
		{
			choice = - choice;
			r = currObj;				// restore pointed object
		}

		r = operate( r );
	}

	Tcl_UnlinkVar( interp, "strWindowOn" );
	Tcl_UnlinkVar( interp, "eff_t" );
}


/*************************************************************
 BROWSE
 *************************************************************/
int gui::browse( lsd::object *r )
{
	bool done, sp_upd;
	int i, num;
	lsd::bridge *cb;
	lsd::variable *cv;

	currObj = r;			// global pointer to C Tcl routines

	// main LSD window - avoids redrawing if not required
	if ( redrawRoot )
	{
		cmd( "destroy .t .l" );
		cmd( "ttk::frame .l" );

		cmd( "ttk::frame .l.v" );

		cmd( "ttk::frame .l.v.c" );
		cmd( "ttk::scrollbar .l.v.c.v_scroll -command \".l.v.c.var_name yview\"" );
		cmd( "ttk::listbox .l.v.c.var_name -selectmode browse -yscroll \".l.v.c.v_scroll set\" -dark $darkTheme" );
		cmd( "mouse_wheel .l.v.c.var_name" );
		cmd( "tooltip::tooltip clear .l.v.c.var_name*" );

		// populate the variables panel
		if ( r->v == NULL )
			cmd( ".l.v.c.var_name insert end \"(none)\"; set nVar 0" );
		else
		{
			for ( cv = r->v, i = 0; cv != NULL; cv = cv->next, ++i )
			{
				// special updating scheme?
				if ( cv->param == 0 && ( cv->delay > 0 || cv->delay_range > 0 || cv->period > 1 || cv->period_range > 0 ) )
					sp_upd = true;
				else
					sp_upd = false;

				// set flags string
				cmd( "set varFlags \"%s%s%s%s%s%s\"", ( cv->save || cv->savei ) ? "+" : "", cv->plot ? "*" : "", ( cv->deb_mode == 'd' || cv->deb_mode == 'W' || cv->deb_mode == 'R' ) ? "!" : "", ( cv->deb_mode == 'w' || cv->deb_mode == 'W' ) ? "?" : "", ( cv->deb_mode == 'r' || cv->deb_mode == 'R' ) ? "\u00BF" : "", cv->parallel ? "&" : "", sp_upd ? "\u00A7" : "" );

				// add elements to the listbox
				if ( cv->param == 0 )
				{
					if ( cv->num_lag == 0 )
					{
						cmd( ".l.v.c.var_name insert end \"%s (V$varFlags)\"", cv->label );
						cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(var)", i );
					}
					else
					{
						cmd( ".l.v.c.var_name insert end \"%s (V_%d$varFlags)\"", cv->label, cv->num_lag );
						cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(lvar)", i );
					}
				}

				if ( cv->param == 1 )
				{
					cmd( ".l.v.c.var_name insert end \"%s (P$varFlags)\"", cv->label );
					cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(par)", i );
				}

				if ( cv->param == 2 )
				{
					if ( cv->num_lag == 0 )
					{
						cmd( " .l.v.c.var_name insert end \"%s (F$varFlags)\"", cv->label );
						cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(fun)", i );
					}
					else
					{
						cmd( ".l.v.c.var_name insert end \"%s (F_%d$varFlags)\"", cv->label, cv->num_lag );
						cmd( ".l.v.c.var_name itemconf %d -fg $colorsTheme(lfun)", i );
					}
				}

				set_ttip_descr( ".l.v.c.var_name", cv->label, i );
			}

			cmd( "set nVar [ .l.v.c.var_name size ]" );
		}

		cmd( "ttk::label .l.v.lab -text \"Variables & parameters ($nVar)\"" );

		// variables panel context menu (right mouse button)
		cmd( "ttk::menu .l.v.c.var_name.v -tearoff 0" );
		cmd( ".l.v.c.var_name.v add command -label Change -accelerator Enter -command { set choice 7 }" );	// entryconfig 0
		cmd( ".l.v.c.var_name.v add command -label Properties -accelerator F2 -command { set choice 75 }" );	// entryconfig 1
		cmd( ".l.v.c.var_name.v add command -label \"Updating (\u00A7)\" -state disabled -command { set choice 96 }" );	// entryconfig 2
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 3
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Save (+)\" -variable save -accelerator F5 -command { set ctxMenuCmd \"set_var_conf $vname save $save\"; set choice 95 }" );	// entryconfig 4
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Run Plot (*)\" -variable plot -accelerator F6 -command { set ctxMenuCmd \"set_var_conf $vname plot $plot\"; set choice 95 }" );	// entryconfig 5
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Debug (!)\" -state disabled -variable debug -accelerator F7 -command { set ctxMenuCmd \"set_var_conf $vname debug $debug\"; set choice 95 }" );	// entryconfig 6
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Watch (?)\" -state disabled -variable watch -accelerator F8 -command { set watch_write 0; set ctxMenuCmd \"set_var_conf $vname watch $watch\"; set choice 95 }" );	// entryconfig 7
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Watch Write (\u00BF)\" -state disabled -variable watch_write -accelerator F9 -command { set watch 0; set ctxMenuCmd \"set_var_conf $vname watch_write $watch_write\"; set choice 95 }" );	// entryconfig 8
		cmd( ".l.v.c.var_name.v add checkbutton -label \"Parallel (&)\" -state disabled -variable parallel -accelerator F10 -command { set ctxMenuCmd \"set_var_conf $vname parallel $parallel\"; set choice 95 }" );	// entryconfig 9
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 10
		cmd( ".l.v.c.var_name.v add command -label \"Move Up\" -accelerator \"Ctrl+\u2191\" -state disabled -command { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { $itemfocus > 0 } { incr itemfocus -1 }; set choice 58 }" );	// entryconfig 11
		cmd( ".l.v.c.var_name.v add command -label \"Move Down\" -accelerator \"Ctrl+\u2193\" -state disabled -command { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { $itemfocus < [ expr { [ .l.v.c.var_name size ] - 1 } ] } { incr itemfocus }; set choice 59 }" );	// entryconfig 12
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 13
		cmd( ".l.v.c.var_name.v add command -label Move -command { set choice 79 }" );	// entryconfig 14
		cmd( ".l.v.c.var_name.v add command -label Delete -accelerator Del -command { set choice 76 }" );	// entryconfig 15
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 16
		cmd( ".l.v.c.var_name.v add command -label Equation -state disabled -command { set choice 29 }" );	// entryconfig 17
		cmd( ".l.v.c.var_name.v add command -label Using -state disabled -command { set choice 46 }" );	// entryconfig 18
		cmd( ".l.v.c.var_name.v add command -label \"Used In\" -state disabled -command { set choice 47 }" );	// entryconfig 19
		cmd( ".l.v.c.var_name.v add separator" );	// entryconfig 20
		cmd( ".l.v.c.var_name.v add command -label \"Initial Values\" -state disabled -command { set choice 77 }" );	// entryconfig 21
		cmd( ".l.v.c.var_name.v add command -label Sensitivity -state disabled -command { set choice 78 }" );	// entryconfig 22

		// variables panel bindings
		if ( r->v != NULL )
		{
			cmd( "bind .l.v.c.var_name <Return> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 7 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Double-Button-1> { \
					set dblclk 1; \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						after idle { set choice 7 } \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Button-2> { \
					.l.v.c.var_name selection clear 0 end; \
					.l.v.c.var_name selection set @%%x,%%y; \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						.l.v.c.var_name.v entryconfig 2 -state normal; \
						.l.v.c.var_name.v entryconfig 6 -state normal; \
						.l.v.c.var_name.v entryconfig 7 -state normal; \
						.l.v.c.var_name.v entryconfig 8 -state normal; \
						.l.v.c.var_name.v entryconfig 9 -state normal; \
						.l.v.c.var_name.v entryconfig 11 -state normal; \
						.l.v.c.var_name.v entryconfig 12 -state normal; \
						.l.v.c.var_name.v entryconfig 17 -state normal; \
						.l.v.c.var_name.v entryconfig 18 -state normal; \
						.l.v.c.var_name.v entryconfig 19 -state normal; \
						.l.v.c.var_name.v entryconfig 21 -state normal; \
						.l.v.c.var_name.v entryconfig 22 -state normal; \
						set save [ get_var_conf $vname save ]; \
						set plot [ get_var_conf $vname plot ]; \
						set debug [ get_var_conf $vname debug ]; \
						set watch [ get_var_conf $vname watch ]; \
						set watch_write [ get_var_conf $vname watch_write ]; \
						set parallel [ get_var_conf $vname parallel ]; \
						if [ string equal $color $colorsTheme(var) ] { \
							.l.v.c.var_name.v entryconfig 21 -state disabled; \
							.l.v.c.var_name.v entryconfig 22 -state disabled; \
						} elseif [ string equal $color $colorsTheme(par) ] { \
							.l.v.c.var_name.v entryconfig 2 -state disabled; \
							.l.v.c.var_name.v entryconfig 6 -state disabled; \
							.l.v.c.var_name.v entryconfig 9 -state disabled; \
							.l.v.c.var_name.v entryconfig 17 -state disabled; \
							.l.v.c.var_name.v entryconfig 18 -state disabled \
						} elseif [ string equal $color $colorsTheme(lfun) ] { \
							.l.v.c.var_name.v entryconfig 2 -state disabled; \
							.l.v.c.var_name.v entryconfig 7 -state disabled; \
							.l.v.c.var_name.v entryconfig 8 -state disabled; \
							.l.v.c.var_name.v entryconfig 9 -state disabled \
						} elseif [ string equal $color $colorsTheme(fun) ] { \
							.l.v.c.var_name.v entryconfig 2 -state disabled; \
							.l.v.c.var_name.v entryconfig 7 -state disabled; \
							.l.v.c.var_name.v entryconfig 8 -state disabled; \
							.l.v.c.var_name.v entryconfig 9 -state disabled; \
							.l.v.c.var_name.v entryconfig 21 -state disabled; \
							.l.v.c.var_name.v entryconfig 22 -state disabled; \
						}; \
						if { $itemfocus == 0 } { \
							.l.v.c.var_name.v entryconfig 11 -state disabled \
						}; \
						if { $itemfocus == [ expr { [ .l.v.c.var_name size ] - 1 } ] } { \
							.l.v.c.var_name.v entryconfig 12 -state disabled \
						}; \
						tk_popup .l.v.c.var_name.v %%X %%Y \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Button-3> { \
					event generate .l.v.c.var_name <Button-2> -x %%x -y %%y \
				}" );
			cmd( "bind .l.v.c.var_name <Control-Up> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					if { $itemfocus > 0 } { \
						incr itemfocus -1 \
					}; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 58 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Control-Down> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					if { $itemfocus < [ expr { [ .l.v.c.var_name size ] - 1 } ] } { \
						incr itemfocus \
					}; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 59 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Delete> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 76 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <F2> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 75 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <F3> { \
					set listfocus 1; \
					set sort_order 0; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <F4> { \
					set listfocus 1; \
					set sort_order 1; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <Shift-F3> { \
					set listfocus 1; \
					set sort_order 2; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <Shift-F4> { \
					set listfocus 1; \
					set sort_order 3; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <Control-F3> { \
					set listfocus 1; \
					set sort_order 4; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <Control-F4> { \
					set listfocus 1; \
					set sort_order 5; \
					set choice 94 \
				}" );
			cmd( "bind .l.v.c.var_name <F5> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $eff_t == 0 } { \
						set save [ expr { ! [ get_var_conf $vname save ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname save $save\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <plus> { \
					event generate .l.v.c.var_name <F5> \
				}" );
			cmd( "bind .l.v.c.var_name <F6> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $eff_t == 0 } { \
						set plot [ expr { ! [ get_var_conf $vname plot ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname plot $plot\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <asterisk> { \
					event generate .l.v.c.var_name <F6> \
				}" );
			cmd( "bind .l.v.c.var_name <F7> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $eff_t == 0 && ! [ string equal $color $colorsTheme(par) ] } { \
						set debug [ expr { ! [ get_var_conf $vname debug ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname debug $debug\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <exclam> { \
					event generate .l.v.c.var_name <F7> \
				}" );
			cmd( "bind .l.v.c.var_name <F8> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $eff_t == 0 && ! [ string equal $color $colorsTheme(lfun) ] && ! [ string equal $color $colorsTheme(fun) ] } { \
						set watch_write 0; \
						set watch [ expr { ! [ get_var_conf $vname watch ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname watch $watch\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <question> { \
					event generate .l.v.c.var_name <F8> \
				}" );
			cmd( "bind .l.v.c.var_name <F9> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $eff_t == 0 && ! [ string equal $color $colorsTheme(lfun) ] && ! [ string equal $color $colorsTheme(fun) ] } { \
						set watch 0; \
						set watch_write [ expr { ! [ get_var_conf $vname watch_write ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname watch_write $watch_write\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <questiondown> { \
					event generate .l.v.c.var_name <F9> \
				}" );
			cmd( "bind .l.v.c.var_name <F11> { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; \
					if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $eff_t == 0 && ! [ string equal $color $colorsTheme(par) ] && ! [ string equal $color $colorsTheme(lfun) ] && ! [ string equal $color $colorsTheme(fun) ] } { \
						set parallel [ expr { ! [ get_var_conf $vname parallel ] } ]; \
						set ctxMenuCmd \"set_var_conf $vname parallel $parallel\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <ampersand> { \
					event generate .l.v.c.var_name <F11> \
				}" );
			cmd( "bind .l.v.c.var_name <KeyRelease> { \
					if { ( %%s & 0x20004 ) != 0 } { \
						return \
					}; \
					set kk %%K; \
					if { [ string equal $kk underscore ] || ( [ string length $kk ] == 1 && [ string is alpha -strict $kk ] ) } { \
						if [ string equal $kk underscore ] { \
							set kk _ \
						}; \
						set ll %%W; \
						set ff [ lsearch -start [ expr { [ $ll curselection ] + 1 } ] -nocase [ $ll get 0 end ] \"${kk}*\" ]; \
						if { $ff == -1 } { \
							set ff [ lsearch -start 0 -nocase [ $ll get 0 end ] \"${kk}*\" ] \
						}; \
						if { $ff >= 0 } { \
							selectinlist $ll $ff \
						} \
					} \
				}" );
			cmd( "bind .l.v.c.var_name <Home> { \
					selectinlist .l.v.c.var_name 0; \
					break \
				}" );
			cmd( "bind .l.v.c.var_name <End> { \
					selectinlist .l.v.c.var_name end; \
					break \
				}" );
		}

		cmd( "bind .l.v.c.var_name <Button-1> { \
				set dblclk 0; \
				after 200; \
				if { ! $dblclk } { \
					set listfocus 1; \
					set itemfocus [ .l.v.c.var_name curselection ]; \
					set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ]; \
					upd_menu_visib \
				} \
			}" );
		cmd( "bind .l.v.c.var_name <Left> { \
				focus .l.s.c.son_name; \
				set listfocus 2; \
				set itemfocus 0; \
				selectinlist .l.s.c.son_name 0; \
				upd_menu_visib \
			}" );

		cmd( "ttk::frame .l.s" );

		cmd( "ttk::frame .l.s.c" );
		cmd( "ttk::scrollbar .l.s.c.v_scroll -command \".l.s.c.son_name yview\"" );
		cmd( "ttk::listbox .l.s.c.son_name -selectmode browse -yscroll \".l.s.c.v_scroll set\" -dark $darkTheme" );
		cmd( "mouse_wheel .l.s.c.son_name" );
		cmd( "tooltip::tooltip clear .l.s.c.son_name*" );

		if ( r->up != NULL )
		{
			cmd( ".l.s.c.son_name insert end \"$upSymbol\"" );
			cmd( "tooltip::tooltip .l.s.c.son_name -item 0 \"%s\"", r->up->label );
			i = 1;
		}
		else
			i = 0;

		cmd( "set upObjItem %d", i );

		if ( r->up == NULL && r->b == NULL )
			cmd( ".l.s.c.son_name insert end \"(none)\"" );
		else
		{
			// populate the objects panel
			for ( cb = r->b; cb != NULL; cb = cb->next, ++i )
			{
				if ( cb->head != NULL )
				{
					cb->head->next_count( cb->head, & num );
					done = cb->head->to_compute;
				}
				else
				{
					num = 0;
					done = true;
				}

				cmd( ".l.s.c.son_name insert end \"%s (#%d%s)\"", cb->blabel, num, done ? "" : "-" );
				cmd( ".l.s.c.son_name itemconf %d -fg $colorsTheme(obj)", i );

				set_ttip_descr( ".l.s.c.son_name", cb->blabel, i );
			}
		}

		cmd( "ttk::label .l.s.lab -text \"Descending objects ([ expr { %d - $upObjItem } ])\"", i );

		// objects panel context menu (right mouse button)
		cmd( "ttk::menu .l.s.c.son_name.v -tearoff 0" );
		cmd( ".l.s.c.son_name.v add command -label \"Select\" -accelerator Enter -command { set choice 4 }" );	// entryconfig 0
		cmd( ".l.s.c.son_name.v add command -label \"Parent\" -accelerator Back -command { set choice 5 }" );	// entryconfig 1
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 2
		cmd( ".l.s.c.son_name.v add command -label \"Move Up\" -accelerator \"Ctrl+\u2191\" -state disabled -command { set listfocus 2; set itemfocus [ .l.s.c.son_name curselection ]; if { $itemfocus > 0 } { incr itemfocus -1 }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 60 } }" );	// entryconfig 3
		cmd( ".l.s.c.son_name.v add command -label \"Move Down\" -accelerator \"Ctrl+\u2193\" -state disabled -command { set listfocus 2; set itemfocus [ .l.s.c.son_name curselection ]; if { $itemfocus < [ expr { [ .l.s.c.son_name size ] - 1 } ] } { incr itemfocus }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 61 } }" );	// entryconfig 4
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 5
		cmd( ".l.s.c.son_name.v add command -label Change -accelerator \"Ctrl+Enter\" -command { set choice 6 }" );	// entryconfig 6
		cmd( ".l.s.c.son_name.v add command -label Rename -accelerator F2 -command { set choice 83 }" );	// entryconfig 7
		cmd( ".l.s.c.son_name.v add command -label Number -command { set choice 33 }" );	// entryconfig 8
		cmd( ".l.s.c.son_name.v add command -label Move -command { set choice 32 }" );	// entryconfig 9
		cmd( ".l.s.c.son_name.v add command -label Delete -accelerator Del -command { set choice 74 }" );	// entryconfig 10
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 11
		cmd( ".l.s.c.son_name.v add cascade -label Add -menu .l.s.c.son_name.v.a" );	// entryconfig 12=14
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 13
		cmd( ".l.s.c.son_name.v add checkbutton -label \"Not Compute (-)\" -variable nocomp -accelerator F5 -command { set ctxMenuCmd \"set_obj_conf $vname comp [ expr { ! $nocomp } ]\"; set choice 95 }" );	// entryconfig 14
		cmd( ".l.s.c.son_name.v add separator" );	// entryconfig 15
		cmd( ".l.s.c.son_name.v add command -label \"Initial Values\" -accelerator \"Ctrl+I\" -command { set choice 21 }" );	// entryconfig 16
		cmd( ".l.s.c.son_name.v add command -label \"Browse Data\" -accelerator \"Ctrl+B\" -command { set choice 34 }" );	// entryconfig 17
		cmd( "ttk::menu .l.s.c.son_name.v.a -tearoff 0" );
		cmd( ".l.s.c.son_name.v.a add command -label Variable -accelerator \"Ctrl+V\" -command { set choice 2; set param 0 }" );
		cmd( ".l.s.c.son_name.v.a add command -label Parameter -accelerator \"Ctrl+P\" -command { set choice 2; set param 1 }" );
		cmd( ".l.s.c.son_name.v.a add command -label Function -accelerator \"Ctrl+N\" -command { set choice 2; set param 2 }" );
		cmd( ".l.s.c.son_name.v.a add command -label Object -accelerator \"Ctrl+D\" -command { set choice 3 }" );

		// objects panel bindings
		if ( r->up != NULL || r->b != NULL )
		{
			cmd( "bind .l.s.c.son_name <Return> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { $upObjItem && $itemfocus == 0 } { \
						set choice 5 \
					} { \
						if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
							set choice 4 \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Control-Return> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set useCurrObj no; \
						set choice 6 \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Double-Button-1> { \
					set dblclk 1; \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { $upObjItem && $itemfocus == 0 } { \
						set choice 5 \
					} { \
						if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
							after idle { set choice 4 } \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Button-2> { \
					.l.s.c.son_name selection clear 0 end; \
					.l.s.c.son_name selection set @%%x,%%y; \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set useCurrObj no; \
						set nocomp [ expr { ! [ get_obj_conf $vname comp ] } ]; \
						if { $itemfocus == 0 } { \
							.l.s.c.son_name.v entryconfig 3 -state disabled \
						} { \
							.l.s.c.son_name.v entryconfig 3 -state normal \
						}; \
						if { $itemfocus == [ expr { [ .l.s.c.son_name size ] - 1 } ] } { \
							.l.s.c.son_name.v entryconfig 4 -state disabled \
						} { \
							.l.s.c.son_name.v entryconfig 4 -state normal \
						}; \
						tk_popup .l.s.c.son_name.v %%X %%Y \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Button-3> { \
					event generate .l.s.c.son_name <Button-2> -x %%x -y %%y \
				}" );
			cmd( "bind .l.s.c.son_name <Control-Up> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) } { \
						if { $itemfocus > 0 } { \
							incr itemfocus -1 \
						}; \
						if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
							set choice 60 \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Control-Down> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) } { \
						if { $itemfocus < [ expr { [ .l.s.c.son_name size ] - 1 } ] } { \
							incr itemfocus \
						}; \
						if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
							set choice 61 \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Delete> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 74 \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <F2> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { \
						set choice 83 \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <F3> { \
					set listfocus 2; \
					set sort_order 0; \
					set choice 94 \
				}" );
			cmd( "bind .l.s.c.son_name <F4> { \
					set listfocus 2; \
					set sort_order 1; \
					set choice 94 \
				}" );
			cmd( "bind .l.s.c.son_name <F5> { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					if { ! ( $upObjItem && $itemfocus == 0 ) && ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] && $eff_t == 0 } { \
						set nocomp [ expr { ! [ get_obj_conf $vname comp ] } ]; \
						set ctxMenuCmd \"set_obj_conf $vname comp $nocomp\"; \
						set choice 95 \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <minus> { \
					event generate .l.s.c.son_name <F5> \
				}" );
			cmd( "bind .l.s.c.son_name <KeyRelease> { \
					if { ( %%s & 0x20004 ) != 0 } { \
						return \
					}; \
					set kk %%K; \
					if { [ string equal $kk underscore ] || ( [ string length $kk ] == 1 && [ string is alpha -strict $kk ] ) } { \
						if [ string equal $kk underscore ] { \
							set kk _ \
						}; \
						set ll %%W; \
						set ff [ lsearch -start [ expr { [ $ll curselection ] + 1 } ] -nocase [ $ll get 0 end ] \"${kk}*\" ]; \
						if { $ff == -1 } { \
							set ff [ lsearch -start 0 -nocase [ $ll get 0 end ] \"${kk}*\" ] \
						}; \
						if { $ff >= 0 } { \
							selectinlist $ll $ff \
						} \
					} \
				}" );
			cmd( "bind .l.s.c.son_name <Home> { \
					selectinlist .l.s.c.son_name 0; \
					break \
				}" );
			cmd( "bind .l.s.c.son_name <End> { \
					selectinlist .l.s.c.son_name end; \
					break \
				}" );
		}

		cmd( "bind .l.s.c.son_name <BackSpace> { set choice 5 }" );

		cmd( "bind .l.s.c.son_name <Button-1> { \
				set dblclk 0; \
				after 200; \
				if { ! $dblclk } { \
					set listfocus 2; \
					set itemfocus [ .l.s.c.son_name curselection ]; \
					set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]; \
					upd_menu_visib \
				} \
			}" );
		cmd( "bind .l.s.c.son_name <Right> { \
				focus .l.v.c.var_name; \
				set listfocus 1; \
				set itemfocus 0; \
				selectinlist .l.v.c.var_name 0; \
				upd_menu_visib \
			}" );

		// navigation (top) panel
		cmd( "ttk::frame .l.p" );

		cmd( "ttk::frame .l.p.up_name" );
		cmd( "ttk::label .l.p.up_name.d -text \"Parent object:\" -width 15 -anchor w" );
		if ( r->up != NULL )
		{
			cmd( "ttk::label .l.p.up_name.n -text \" %s \" -anchor w -style hl.TLabel", r->up->label );
			cmd( "bind . <KeyPress-u> { set itemfocus 0; set choice 5 }; bind . <KeyPress-U> { set itemfocus 0; set choice 5 }" );
		}
		else
			cmd( "ttk::label .l.p.up_name.n -anchor w -text \"\"" );

		cmd( "pack .l.p.up_name.d .l.p.up_name.n -side left" );
		cmd( "pack .l.p.up_name -padx 5 -anchor w" );

		cmd( "ttk::frame .l.p.tit" );
		cmd( "ttk::label .l.p.tit.lab -text \"Current object:\" -width 15 -anchor w" );
		cmd( "ttk::button .l.p.tit.but -width -1 -text \" %s \" -style hlBold.Toolbutton %s", r->label, r->up == NULL ? "" : "-command { set choice 6 }" );

		if ( r->up != NULL )
			cmd( "tooltip::tooltip .l.p.tit.but \"Change...\"" );
		else
			cmd( ".l.p.tit.but configure -state disabled" );

		cmd( "pack .l.p.tit.lab .l.p.tit.but -side left" );
		cmd( "pack .l.p.tit -padx 5 -anchor w" );

		// main menu - avoid redrawing the menu if it already exists and is configured
		if ( ! exists_window( ".m" ) || ! expr_eq( "[ . cget -menu ]", ".m" ) )
		{
			cmd( "destroy .m" );
			cmd( "ttk::menu .m -tearoff 0" );

			cmd( "set w .m.file" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label File -menu $w -underline 0" );
			cmd( "$w add command -label \"Load...\" -underline 0 -accelerator Ctrl+L -command { set choice 17 }" );
			cmd( "$w add command -label Reload -underline 0 -accelerator Ctrl+W -command { set choice 38 }" );
			cmd( "$w add command -label Save -underline 0 -accelerator Ctrl+S -command { set choice 18 }" );
			cmd( "$w add command -label \"Save As...\" -underline 5 -command { set choice 73 }" );

			cmd( "$w add command -label Unload -underline 0 -accelerator Ctrl+E -command { set choice 20 }" );
			cmd( "$w add command -label \"Compare...\" -underline 0 -command { set choice 82 }" );
			cmd( "$w add command -label \"Export Legacy...\" -underline 9 -command { set choice 9 }" );
			cmd( "$w add command -label \"Export Saved Elements...\" -underline 1 -command { set choice 91 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Save Results...\" -underline 2 -accelerator Ctrl+Z -command { set choice 37 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Unload Sensitivity\" -underline 11 -command { set choice 67 }" );
			cmd( "$w add command -label \"Import Sensitivity...\" -underline 3 -command { set choice 64 }" );
			cmd( "$w add command -label \"Export Sensitivity...\" -underline 6 -command { set choice 65 }" );
			cmd( "$w add command -label \"Export Sensitivity Limits...\" -underline 2 -command { set choice 90 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Unload Network\" -underline 3 -command { set choice 93 }" );
			cmd( "$w add command -label \"Import Network...\" -underline 5 -command { set choice 88 }" );
			cmd( "$w add command -label \"Export Network...\" -underline 8 -command { set choice 89 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Set Equation File...\" -underline 2 -accelerator Ctrl+U -command { set choice 28 }" );
			cmd( "$w add command -label \"Restore Equation File...\" -underline 1 -command { set choice 52 }" );
			cmd( "$w add command -label \"Compare Equation Files...\" -underline 2 -command { set choice 53 }" );

			cmd( "$w add separator" );

			cmd( "$w add command -label Quit -underline 0 -accelerator Ctrl+Q -command { set choice 11 }" );

			cmd( "set w .m.model" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label Model -menu $w -underline 0" );
			cmd( "$w add command -label \"Add Variable...\" -underline 4 -accelerator Ctrl+V -command { set param 0; set choice 2 }" );	// entryconfig 0
			cmd( "$w add command -label \"Add Parameter...\" -underline 4 -accelerator Ctrl+P -command { set param 1; set choice 2 }" );	// entryconfig 1
			cmd( "$w add command -label \"Add Function...\" -underline 5 -accelerator Ctrl+N -command { set param 2; set choice 2 }" );	// entryconfig 2
			cmd( "$w add command -label \"Add Object...\" -underline 4 -accelerator Ctrl+D -command { set choice 3 }" );	// entryconfig 3

			cmd( "$w add separator" );	// entryconfig 4

			cmd( "$w add command -label \"Change Element...\" -underline 0 -accelerator Enter -command { set useCurrObj yes; set choice 7 }" );	// entryconfig 5
			cmd( "$w add command -label \"Change Object...\" -underline 7 -accelerator Ctrl+Enter -command { set useCurrObj yes; set choice 6 }" );	// entryconfig 6
			cmd( "$w add command -label \"Find...\" -underline 0 -accelerator Ctrl+F -command { set choice 50 }" );	// entryconfig 7

			cmd( "$w add cascade -label \"Sort\" -underline 0 -menu $w.sort" );	// entryconfig 8

			cmd( "$w add separator" );	// entryconfig 9

			cmd( "$w add command -label \"Create Model Report...\" -underline 7 -command { set choice 36 }" );	// entryconfig 10
			cmd( "$w add command -label \"Create LaTex Tables\" -underline 9 -command { set choice 57 }" );	// entryconfig 11
			cmd( "$w add command -label \"Create LaTex References\" -underline 13 -command { set choice 92 }" );	// entryconfig 12
			cmd( "$w add command -label \"Import Descriptions\" -underline 0 -command { set choice 43 }" );	// entryconfig 13

			cmd( "$w add separator" );	// entryconfig 14

			cmd( "set strWindowChk $strWindowOn" );
			cmd( "$w add checkbutton -label \"Enable Structure Window\" -underline 17 -accelerator Ctrl+Tab -variable strWindowChk -command { set choice 70 }" );	// entryconfig 15
			cmd( "$w add checkbutton -label \"Ignore Equation File\" -underline 0 -variable ignore_eq_file -command { set choice 54 }" );	// entryconfig 16

			cmd( "set w .m.model.sort" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( "$w add command -label \"Ascending (alphabetic only)\" -underline 0 -accelerator F3 -command { set sort_order 0; set choice 94 }" );
			cmd( "$w add command -label \"Descending (alphabetic only)\" -underline 0 -accelerator F4 -command { set sort_order 1; set choice 94 }" );
			cmd( "$w add command -label \"Ascending (parameters first)\" -underline 11 -accelerator Shift+F3 -command { set sort_order 2; set choice 94 }" );
			cmd( "$w add command -label \"Descending (parameters first)\" -underline 18 -accelerator Shift+F4 -command { set sort_order 3; set choice 94 }" );
			cmd( "$w add command -label \"Ascending (variables first)\" -underline 11 -accelerator Ctrl+F3 -command { set sort_order 4; set choice 94 }" );
			cmd( "$w add command -label \"Descending (variables first)\" -underline 17 -accelerator Ctrl+F4 -command { set sort_order 5; set choice 94 }" );

			cmd( "set w .m.data" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label Data -menu $w -underline 0" );
			cmd( "$w add command -label \"Initial Values...\" -command { set choice 21 } -underline 0 -accelerator Ctrl+I" );
			cmd( "$w add command -label \"Numbers of Objects....\" -command { set choice 19 } -accelerator Ctrl+O -underline 0" );

			cmd( "$w add separator" );

			cmd( "$w add cascade -label \"Sensitivity Analysis\" -underline 0 -menu $w.setsens" );

			cmd( "$w add separator" );

			cmd( "$w add command -label \"Analysis of Results...\" -command { set choice 26 } -underline 0 -accelerator Ctrl+A" );
			cmd( "$w add command -label \"Analysis of MC Experiment...\" -command { set choice 12 } -underline 0" );
			cmd( "$w add command -label \"Data Browse...\" -command { set choice 34 } -underline 5 -accelerator Ctrl+B" );

			cmd( "set w .m.data.setsens" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( "$w add command -label \"Full (online)\" -underline 0 -command { set choice 62 }" );
			cmd( "$w add command -label \"Full (batch)\" -underline 6 -command { set choice 63 }" );
			cmd( "$w add command -label \"MC Point Sampling (batch)...\" -underline 0 -command { set choice 71 }" );
			cmd( "$w add command -label \"MC Range Sampling (batch)...\" -underline 3 -command { set choice 80 }" );
			cmd( "$w add command -label \"EE Sampling (batch)...\" -underline 0 -command { set choice 81 }" );
			cmd( "$w add command -label \"NOLH Sampling (batch)...\" -underline 0 -command { set choice 72 }" );

			cmd( "set w .m.run" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label Run -menu $w -underline 0" );
			cmd( "$w add command -label Run -underline 0 -accelerator Ctrl+R -command { set choice 1 }" );
			cmd( "$w add command -label \"Parallel Run...\" -underline 0 -command { set choice 69 }" );
			cmd( "$w add command -label \"Parallel Batch...\" -underline 9 -command { set choice 68 }" );
			cmd( "$w add separator" );
			cmd( "$w add command -label \"Simulation Settings...\" -underline 2 -accelerator Ctrl+M -command { set choice 22 }" );

			cmd( "$w add separator" );

			cmd( "$w add cascade -label \"Show Elements to\" -underline 17 -menu $w.show" );
			cmd( "$w add cascade -label \"Remove Flags to\" -underline 15 -menu $w.rem" );

			cmd( "set w .m.run.show" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( "$w add command -label Save -underline 0 -command { set choice 39 }" );
			cmd( "$w add command -label \"Run-time Plot\" -underline 0 -command { set choice 84 }" );
			cmd( "$w add command -label \"Debug and Watch\" -underline 0 -command { set choice 85 }" );
			cmd( "$w add command -label Initialize -underline 0 -command { set choice 49 }" );
			cmd( "$w add command -label Observe -underline 0 -command { set choice 42 }" );
			cmd( "$w add command -label Parallelize -underline 0 -command { set choice 86 }" );
			cmd( "$w add command -label \"Special Updating\" -underline 8 -command { set choice 97 }" );
			cmd( "$w add command -label \"Sensitivity Analysis\" -underline 1 -command { set choice 66 }" );
			cmd( "$w add command -label Unused -underline 1 -command { set choice 56 }" );

			cmd( "set w .m.run.rem" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( "$w add command -label Save -underline 0 -accelerator Ctrl+G -command { set choice 30 }" );
			cmd( "$w add command -label \"Run-time Plot\" -underline 0 -command { set choice 31 }" );
			cmd( "$w add command -label \"Debug and Watch\" -underline 0 -accelerator Ctrl+F -command { set choice 27 }" );
			cmd( "$w add command -label Parallelize -underline 0 -command { set choice 87 }" );

			cmd( "set w .m.help" );
			cmd( "ttk::menu $w -tearoff 0" );
			cmd( ".m add cascade -label Help -menu $w -underline 0" );
			cmd( "$w add command -label \"Help on Browser\" -underline 0 -accelerator F1 -command { LsdHelp browser.html }" );
			cmd( "$w add command -label \"LSD Quick Help\" -underline 4 -command { LsdHelp LSD_quickhelp.html }" );
			cmd( "$w add command -label \"LSD Documentation\" -underline 4 -command { LsdHelp LSD_documentation.html }" );
			cmd( "$w add separator" );
			cmd( "$w add command -label \"LMM Primer Tutorial\" -underline 4 -command { LsdHelp LMM_primer.html }" );
			cmd( "$w add command -label \"Using LSD Models Tutorial\" -underline 0 -command { LsdHelp model_using.html }" );
			cmd( "$w add command -label \"Writing LSD Models Tutorial\" -underline 0 -command { LsdHelp model_writing.html }" );
			cmd( "$w add separator" );
			cmd( "if { ! [ string equal $CurPlatform windows ] } { $w add command -label \"Set Browser\" -command { set choice 48 } -underline 0 }" );
			cmd( "$w add command -label \"Model Report\" -underline 0 -command { set choice 44 }" );
			cmd( "$w add separator" );
			cmd( "$w add command -label \"About LSD...\" -underline 0 -command { LsdAbout {%s} {%s} }", _LSD_VERSION_, _LSD_DATE_  );

			// set shortcuts on open windows
			cmd( "bind . <F1> { LsdHelp browser.html }" );
			set_shortcuts( "." );
			set_shortcuts( ".log" );
		}

		// Button bar
		if ( ! exists_window( ".bbar" ) )
		{
			cmd( "ttk::frame .bbar" );

			cmd( "ttk::button .bbar.open -image openImg -style Toolbutton -command { set choice 17 }" );
			cmd( "ttk::button .bbar.reload -image reloadImg -style Toolbutton -command { set choice 38 }" );
			cmd( "ttk::button .bbar.save -image saveImg -style Toolbutton -command { set choice 18 }" );
			cmd( "ttk::button .bbar.struct -image structImg -style Toolbutton -command { set choice 70 }" );
			cmd( "ttk::button .bbar.find -image findImg -style Toolbutton -command { set choice 50 }" );
			cmd( "ttk::button .bbar.addvar -image addvarImg -style Toolbutton -command { set param 0; set choice 2 }" );
			cmd( "ttk::button .bbar.addpar -image addparImg -style Toolbutton -command { set param 1; set choice 2 }" );
			cmd( "ttk::button .bbar.addobj -image addobjImg -style Toolbutton -command { set choice 3 }" );
			cmd( "ttk::button .bbar.init -image initImg -style Toolbutton -command { set choice 21 }" );
			cmd( "ttk::button .bbar.number -image numberImg -style Toolbutton -command { set choice 19 }" );
			cmd( "ttk::button .bbar.set -image setImg -style Toolbutton -command { set choice 22 }" );
			cmd( "ttk::button .bbar.run -image runImg -style Toolbutton -command { set choice 1 }" );
			cmd( "ttk::button .bbar.data -image dataImg -style Toolbutton -command { set choice 34 }" );
			cmd( "ttk::button .bbar.result -image resultImg -style Toolbutton -command { set choice 26 }" );

			cmd( "tooltip::tooltip .bbar.open \"Load...\"" );
			cmd( "tooltip::tooltip .bbar.reload \"Reload\"" );
			cmd( "tooltip::tooltip .bbar.save \"Save\"" );
			cmd( "if { $strWindowOn } { \
					tooltip::tooltip .bbar.struct \"Hide Structure\" \
				} else { \
					tooltip::tooltip .bbar.struct \"Show Sstructure\" \
				}" );
			cmd( "tooltip::tooltip .bbar.find \"Find...\"" );
			cmd( "tooltip::tooltip .bbar.addvar \"Add Variable...\"" );
			cmd( "tooltip::tooltip .bbar.addpar \"Add Parameter...\"" );
			cmd( "tooltip::tooltip .bbar.addobj \"Add Object...\"" );
			cmd( "tooltip::tooltip .bbar.init \"Initial Values...\"" );
			cmd( "tooltip::tooltip .bbar.number \"Number of Objects...\"" );
			cmd( "tooltip::tooltip .bbar.set \"Settings...\"" );
			cmd( "tooltip::tooltip .bbar.run \"Run\"" );
			cmd( "tooltip::tooltip .bbar.data \"Data Browse...\"" );
			cmd( "tooltip::tooltip .bbar.result \"Analysis of Results...\"" );

			cmd( "pack .bbar.open .bbar.reload .bbar.save .bbar.struct .bbar.find .bbar.addvar .bbar.addpar .bbar.addobj .bbar.init .bbar.number .bbar.set .bbar.run .bbar.data .bbar.result -side left" );
			cmd( "pack .bbar -padx 3 -anchor w -fill x" );
		}

		cmd( "pack .l.v.lab" );
		cmd( "pack .l.v.c.v_scroll -side right -fill y" );
		cmd( "pack .l.v.c.var_name -fill both -expand yes" );
		cmd( "pack .l.v.c -fill both -expand yes" );

		cmd( "pack .l.s.lab" );
		cmd( "pack .l.s.c.v_scroll -side right -fill y" );
		cmd( "pack .l.s.c.son_name -fill both -expand yes" );
		cmd( "pack .l.s.c -fill both -expand yes" );

		cmd( "pack .l.p.up_name .l.p.tit" );
		cmd( "pack .l.p -pady 3 -fill x" );

		cmd( "pack .l.s .l.v -side left -fill both -expand yes" );

		cmd( "pack .l -fill both -expand yes" );
	}

	cmd( "settop . no { if { [ discard_change ] eq \"ok\" && [ abort_run_threads ] eq \"ok\" } { exit } } no yes" );

	if ( redrawStruc )
	{
		r->show_graph( );
		redrawStruc = false;
	}

	main_cycle:

	// update element lists removing duplicates and sorting
	cmd( "if [ info exists modObj ] { set modObj [ lsort -dictionary -unique $modObj ] }" );
	cmd( "if [ info exists modElem ] { set modElem [ lsort -dictionary -unique $modElem ] }" );
	cmd( "if [ info exists modVar ] { set modVar [ lsort -dictionary -unique $modVar ] }" );
	cmd( "if [ info exists modPar ] { set modPar [ lsort -dictionary -unique $modPar ] }" );
	cmd( "if [ info exists modFun ] { set modFun [ lsort -dictionary -unique $modFun ] }" );

	// restore correct selection on list boxes
	cmd( "if { $listfocus == 1 } { \
			if { [ .l.v.c.var_name size ] == 0 || ! [ string is integer -strict $itemfocus ] } { \
				set itemfocus 0 \
			} { \
				if { $itemfocus >= [ .l.v.c.var_name size ] } { \
					set itemfocus [ expr { [ .l.v.c.var_name size ] - 1 } ] \
				} \
			}; \
			if { [ lindex [ .l.s.c.son_name yview ] 0 ] != $itemfirst } { \
				.l.v.c.var_name yview moveto $itemfirst \
			}; \
			if { [ .l.v.c.var_name curselection ] != $itemfocus } { \
				.l.v.c.var_name selection set $itemfocus; \
				if { $itemfocus < [ expr { [ lindex [ .l.v.c.var_name yview ] 0 ] * [ .l.v.c.var_name size ] } ] || $itemfocus >= [ expr { [ lindex [ .l.v.c.var_name yview ] 1 ] * [ .l.v.c.var_name size ] } ] } { \
					.l.v.c.var_name see $itemfocus \
				}; \
				set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ] \
			}; \
			if { [ .l.v.c.var_name index active ] != $itemfocus } { \
				selectinlist .l.v.c.var_name $itemfocus \
			}; \
			focus .l.v.c.var_name; \
		}" );
	cmd( "if { $listfocus == 2 } { \
			if { [ .l.s.c.son_name size ] == 0 || ! [ string is integer -strict $itemfocus ] } { \
				set itemfocus 0 \
			} { \
				if { $itemfocus >= [ .l.s.c.son_name size ] } { \
					set itemfocus [ expr { [ .l.s.c.son_name size ] - 1 } ] \
				} \
			}; \
			if { [ lindex [ .l.s.c.son_name yview ] 0 ] != $itemfirst } { \
				.l.s.c.son_name yview moveto $itemfirst \
			}; \
			if { [ .l.s.c.son_name curselection ] != $itemfocus } { \
				.l.s.c.son_name selection set $itemfocus; \
				if { $itemfocus < [ expr { [ lindex [ .l.s.c.son_name yview ] 0 ] * [ .l.s.c.son_name size ] } ] || $itemfocus >= [ expr { [ lindex [ .l.s.c.son_name yview ] 1 ] * [ .l.s.c.son_name size ] } ] } { \
					.l.s.c.son_name see $itemfocus \
				}; \
				set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ] \
			}; \
			if { [ .l.s.c.son_name index active ] != $itemfocus } { \
				selectinlist .l.s.c.son_name $itemfocus \
			}; \
			focus .l.s.c.son_name; \
		}" );

	cmd( "upd_menu_visib" );		// update active menu options
	cmd( "set useCurrObj yes" );	// flag to select among the current or the clicked object

	choice = choice_g = 0;
	sim.idle_loop = true;

	// main command loop
	while ( ! choice && ! choice_g )
		Tcl_DoOneEvent( 0 );

	sim.idle_loop = false;

	// coming from the structure window
	if ( choice_g )
	{
		choice = choice_g;
		choice_g = 0;
		res_g = exists_var( "res_g" ) ? get_str( "res_g" ) : NULL;
		cmd( "focus .l.v.c.var_name" );
	}

	// update focus memory
	cmd( "if { [ .l.v.c.var_name curselection ] != \"\" } { \
			set listfocus 1; \
			set itemfocus [ .l.v.c.var_name curselection ]; \
			set itemfirst [ lindex [ .l.v.c.var_name yview ] 0 ] \
		}" );
	cmd( "if { [ .l.s.c.son_name curselection ] != \"\" } { \
			set listfocus 2; \
			set itemfocus [ .l.s.c.son_name curselection ]; \
			set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ] \
		}" );

	// if simulation was started, check to see if operation is valid
	if ( sim.running || sim.eff_t > 0 )
		// search the sorted list of choices that are bad with existing run data
		if ( bsearch( & choice, badChoices, NUM_BAD_CHOICES, sizeof ( int ), comp_ints ) != NULL )
		{
			if ( discard_change( true, false, "Invalid command after a simulation run." ) )	// for sure there are changes, just get the pop-up
			{
				if ( open_configuration( r, true ) )
					choice = - choice;		// signal the reload
				else
					choice = 20;			// reload failed, unload configuration
			}
			else
			{
				choice = 0;
				goto main_cycle;
			}
		}

	return choice;
}


/*************************************************************
 RUNTIME_START
 Updates GUI at the start of a set of simulation runs
 *************************************************************/
void gui::runtime_start( void )
{
	sim.prof_times.clear( );		// reset profiling times

	cover_browser( "Running...", "Use the buttons to control the simulation:\n\n'Stop' :  aborts the simulation\n'Pause' / 'Resume' :  pauses and resumes the simulation\n'Fast' :	accelerates the simulation by hiding information\n'Observe' :  presents more run-time information\n'Debug' :  triggers the debugger at flagged variables", true );
}


/*************************************************************
 RUNTIME_END
 Updates GUI at the end of a set of simulation runs
 *************************************************************/
void gui::runtime_end( void )
{
	reset_plot( );
	uncover_browser( );
	show_prof_aggr( );
	cmd( "focustop .log" );
}


/*************************************************************
 RUNTIME_RUN_START
 Updates GUI at the start of each simulation run
 Prepare run-time plots and clear AoR maps
 *************************************************************/
void gui::runtime_run_start( void )
{
	sim.root->prepare_plot( sim.run );
	sim.par_map.clear( );			// restart variable to parent name map for AoR
}


/*************************************************************
 RUNTIME_RUN_END
 Updates GUI at the end of each simulation run
 Updates the GUI elements
 *************************************************************/
void gui::runtime_run_end( void )
{
	cmd( ".p.b1.b configure -value %d", sim.run );
	cmd( ".p.b1.i configure -text \"Simulation: %d of %d ([ expr { int( 100 * %d / %d ) } ]%% done)\"",
		 std::min( sim.run + 1, sim.last_run ), sim.last_run, sim.run, sim.last_run );
	cmd( "destroytop .deb" );
	cmd( "update" );
}


/*************************************************************
 RUNTIME_STEP
 Updates GUI at the start of each time step
 Checks if debug must be invoked and if simulation
 is paused (return FALSE) or not (TRUE)
 *************************************************************/
bool gui::runtime_step( void )
{
	cur_plt = 0;			// restart runtime variable color cycle

	if ( pause_run )		// adjust "clock" backwards if simulation is paused
		--sim.t;

	if ( sim.t == sim.deb_t )// activate degugger if it's time
	{
		sim.deb_set = true;
		cmd( "focustop .deb" );
	}
	else
		sim.deb_set = false;

	return ! pause_run;		// only update variables if simulation not paused
}


/*************************************************************
 RUNTIME_BUTTONS
 Handle active buttons during simulation execution
 at the end of each time step
 *************************************************************/
void gui::runtime_buttons( clock_t &last_update )
{
	switch ( done_in )
	{
		case 1:			// Stop button / s/S key
			if ( pause_run )
			{
				cmd( "wm title .log \"$origLogTit\"" );
				cmd( ".b.r2.pause conf -text Pause" );
			}

			sim.quit = 2;
			break;

		case 2:			// Fast button / f/F key
			sim.set_fast( 1 );
			sim.deb_set = false;
			break;

		case 3:			// Debug button / d/D key
			if ( ! pause_run )
			{
				sim.deb_t = sim.t + 1;
				sim.deb_set = true;
				cmd( "focustop .deb" );
			}
			else		// if paused, just call the data browser
			{
				double useless = 0;
				sim.root->debugger( NULL, "Paused by User", &useless, false, "" );
			}

			break;

		case 4:			// Observe button / o/O key
			sim.set_fast( 0 );
			break;

		// runtime plot events
		case 7:			// center button
			center_plot( );
			break;

		case 8:			// scroll checkbox
			scrollB = ! scrollB;
			break;

		case 9:			// pause simulation
			pause_run = ! pause_run;
			if ( pause_run )
			{
				cmd( "set origLogTit [ wm title .log ]; wm title .log \"$origLogTit (PAUSED)\"" );
				plog( "\nSimulation %d of %d paused at case %d", sim.run, sim.last_run, sim.t );
				cmd( ".b.r2.pause conf -text Resume" );
			}
			else
			{
				cmd( "wm title .log \"$origLogTit\"" );
				plog( "\nSimulation %d of %d resumed at case %d", sim.run, sim.last_run, sim.t );
				cmd( ".b.r2.pause conf -text Pause" );
			}
	}

	done_in = 0;

	if ( sim.run == 1 && sim.t == 1 )
		enable_plot( );	// show run time plot if still enabled

	scroll_plot( );		// perform scrolling if enabled

	if ( ( ( float ) clock( ) - last_update ) / CLOCKS_PER_SEC > UPD_PER && exists_window( ".p" ) )
	{
		cmd( ".p.b2.b configure -value %d", sim.t );
		cmd( ".p.b2.i configure -text \"Case: %d of %d ([ expr { int( 100 * %d / %d ) } ]%% done)\"", std::min( sim.t + 1, sim.last_t ), sim.last_t, sim.t, sim.last_t );
		cmd( "update" );
		last_update = clock( );
	}
}


/*************************************************************
 SAVE_POS
 Save user position in browser
 *************************************************************/
void lsd::object::save_pos( void )
{
	if ( ! gui::eval_bool( "[ winfo exists .l.s.c.son_name ]" ) )
		return;				// browser not drawn yet

	// save the current object & cursor position for quick reload
	cmd( "set lastObj %s", label );

	cmd( "if { ! [ string equal [ .l.s.c.son_name curselection ] \"\" ] } { \
				set lastList 2 \
			} else { \
				set lastList 1 \
			}" );

	cmd( "if { $lastList == 1 } { \
			set lastItem [ .l.v.c.var_name curselection ]; \
			set lastFirst [ lindex [ .l.v.c.var_name yview ] 0 ] \
		} else { \
			set lastItem [ .l.s.c.son_name curselection ]; \
			set lastFirst [ lindex [ .l.s.c.son_name yview ] 0 ] \
		}" );

	cmd( "if { $lastItem == \"\" } { set lastItem 0 }" );
}


/*************************************************************
 RESTORE_POS
 Restore user position in browser
 *************************************************************/
lsd::object *lsd::object::restore_pos( void )
{
	object *cur;

	if ( gui::eval_bool( "$lastObj ne \"\"" ) && ( cur = search( gui::get_str( "lastObj" ) ) ) != NULL )
	{
		cmd( "if [ info exists lastList ] { set listfocus $lastList }" );
		cmd( "if [ info exists lastItem ] { set itemfocus $lastItem }" );
		cmd( "if [ info exists lastFirst ] { set itemfirst $lastFirst }" );
		return cur;
	}

	return this;
}


/*************************************************************
 SET_SHORTCUTS
 Define keyboard shortcuts to menu items
 *************************************************************/
void gui::set_shortcuts( const char *window )
{
	cmd( "bind %s <Control-l> { set choice 17 }; bind %s <Control-L> { set choice 17 }", window, window	 );
	cmd( "bind %s <Control-s> { set choice 18 }; bind %s <Control-S> { set choice 18 }", window, window	 );
	cmd( "bind %s <Control-e> { set choice 20 }; bind %s <Control-E> { set choice 20 }", window, window	 );
	cmd( "bind %s <Control-q> { set choice 11 }; bind %s <Control-Q> { set choice 11 }", window, window	 );
	cmd( "bind %s <Control-v> { set param 0; set choice 2 }; bind %s <Control-V> { set param 0; set choice 2 }", window, window	 );
	cmd( "bind %s <Control-p> { set param 1; set choice 2 }; bind %s <Control-P> { set param 1; set choice 2 }", window, window	 );
	cmd( "bind %s <Control-n> { set param 2; set choice 2 }; bind %s <Control-N> { set param 2; set choice 2 }", window, window	 );
	cmd( "bind %s <Control-d> { set choice 3 }; bind %s <Control-D> { set choice 3 }", window, window  );
	cmd( "bind %s <Control-o> { set choice 19 }; bind %s <Control-O> { set choice 19 }", window, window	 );
	cmd( "bind %s <Control-i> { set choice 21 }; bind %s <Control-I> { set choice 21 }", window, window	 );
	cmd( "bind %s <Control-a> { set choice 26 }; bind %s <Control-A> { set choice 26 }", window, window	 );
	cmd( "bind %s <Control-r> { set choice 1 }; bind %s <Control-R> { set choice 1 }", window, window  );
	cmd( "bind %s <Control-m> { set choice 22 }; bind %s <Control-M> { set choice 22 }", window, window	 );
	cmd( "bind %s <Control-f> { set choice 50 }; bind %s <Control-F> { set choice 50 }", window, window	 );
	cmd( "bind %s <Control-u> { set choice 28 }; bind %s <Control-U> { set choice 28 }", window, window	 );
	cmd( "bind %s <Control-g> { set choice 30 }; bind %s <Control-G> { set choice 30 }", window, window	 );
	cmd( "bind %s <Control-b> { set choice 34 }; bind %s <Control-B> { set choice 34 }", window, window	 );
	cmd( "bind %s <Control-z> { set choice 37 }; bind %s <Control-Z> { set choice 37 }", window, window	 );
	cmd( "bind %s <Control-w> { set choice 38 }; bind %s <Control-W> { set choice 38 }", window, window	 );
	cmd( "bind %s <Control-Tab> { set choice 70 }", window	);
}


/*************************************************************
 INSERT_OBJECT
 *************************************************************/
void lsd::object::insert_object( const char *w, bool netOnly, object *above )
{
	bridge *cb;
	object *cur;

	if ( ( above == NULL || above->up == NULL || ( strcmp( label, above->label ) != 0 && strcmp( label, above->up->label ) != 0 ) ) &&
		 ( ! netOnly || node != NULL ) )
		cmd( "%s insert end %s", w, label );

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( above == NULL || strcmp( cb->blabel, above->label ) != 0 )
		{
			if ( cb->head == NULL )
				cur = sim->blueprint->search( cb->blabel );
			else
				cur = cb->head;

			cur->insert_object( w, netOnly, above );
		}
}


/*************************************************************
 WIPE_OUT
 *************************************************************/
void lsd::object::wipe_out( void )
{
	object *cur;
	variable *cv;

	cmd( "if [ info exists modObj ] { set pos [ lsearch -exact $modObj %s ]; if { $pos >= 0 } { set modObj [ lreplace $modObj $pos $pos ] } }", label );

	sim->change_description( label );

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		// remove from element lists
		cmd( "if [ info exists modElem ] { set pos [ lsearch -exact $modElem %s ]; if { $pos >= 0 } { set modElem [ lreplace $modElem $pos $pos ] } }", cv->label );
		cmd( "if [ info exists modVar ] { set pos [ lsearch -exact $modVar %s ]; if { $pos >= 0 } { set modVar [ lreplace $modVar $pos $pos ] } }", cv->label );
		cmd( "if [ info exists modPar ] { set pos [ lsearch -exact $modPar %s ]; if { $pos >= 0 } { set modPar [ lreplace $modPar $pos $pos ] } }", cv->label );
		cmd( "if [ info exists modFun ] { set pos [ lsearch -exact $modFun %s ]; if { $pos >= 0 } { set modFun [ lreplace $modFun $pos $pos ] } }", cv->label );

		sim->change_description( cv->label );
	}

	cur = hyper_next( label );
	if ( cur != NULL )
		cur->wipe_out( );

	delete_bridge( );
	delete this;
}


/*************************************************************
 SHIFT_VAR
 *************************************************************/
 void lsd::object::shift_var( int direction, const char *vlab )
{
	variable *cv, *cv1 = NULL, *cv2 = NULL;

	if ( direction == -1 )
	{	// shift up
		if ( ! strcmp( vlab, v->label ) )
			return;		// variable already at the top

		if ( ! strcmp( vlab, v->next->label ) )
		{	// second var, must become the head of the chain
			cv = v->next->next;		// third
			cv1 = v;				// first
			v = v->next;			// shifted up
			v->next = cv1;
			cv1->next = cv;
			return;
		}

		for ( cv = v; cv != NULL; cv = cv->next )
		{
			if ( ! strcmp( vlab, cv->label ) )
			{
				cv2->next = cv;
				cv1->next = cv->next;
				cv->next = cv1;
				return;
			}

			cv2 = cv1;
			cv1 = cv;
		}
	}

	if ( direction == 1 )
	{	// move down
		if ( ! strcmp( vlab, v->label ) )
		{	// it's the first
			if ( v->next == NULL )
				return;				// it is unique

			cv = v;					// first
			cv1 = cv->next->next;	// third
			v = cv->next;			// first is former second
			v->next = cv;			// second is former first
			cv->next = cv1;			// second points to third
			return;
		}

		for ( cv = v; cv != NULL; cv = cv->next )
		{
			if ( ! strcmp( vlab,cv->label ) )
			{
				if ( cv->next == NULL )
					return;			// already at the end

				cv1->next = cv->next;
				cv->next = cv->next->next;
				cv1->next->next = cv;
				return;
			}

			cv1 = cv;
		}
	}
}


/*************************************************************
 SHIFT_DESC
 *************************************************************/
void lsd::object::shift_desc( int direction, const char *dlab )
{
	bridge *cb, *cb1 = NULL, *cb2 = NULL;

	if ( direction == -1 )
	{	// shift up
		if ( ! strcmp( dlab, b->blabel ) )
			return;		// object already at the top

		if ( ! strcmp( dlab, b->next->blabel ) )
		{	// second var, must become the head of the chain
			cb = b->next->next;		// third
			cb1 = b;				// first
			b = b->next;			// shifted up
			b->next = cb1;
			cb1->next = cb;
			return;
		}

		for ( cb = b; cb != NULL; cb = cb->next )
		{
			if ( ! strcmp( dlab, cb->blabel ) )
			{
				cb2->next = cb;
				cb1->next = cb->next;
				cb->next = cb1;
				return;
			}

			cb2 = cb1;
			cb1 = cb;
		}
	}

	if ( direction == 1 )
	{	//move down
		if ( ! strcmp( dlab, b->blabel ) )
		{	// it's the first
			if ( b->next == NULL)
				return;				// it is unique

			cb = b;					// first
			cb1 = cb->next->next;	// third
			b = cb->next;			// first is former second
			b->next = cb;			// second is former first
			cb->next = cb1;			// second points to third
			return;
		}

		for ( cb = b; cb != NULL; cb = cb->next )
		{
			if ( ! strcmp( dlab, cb->blabel ) )
			{
				if ( cb->next == NULL )
					return;			// already at the end

				cb1->next = cb->next;
				cb->next = cb->next->next;
				cb1->next->next = cb;
				return;
			}

			cb1 = cb;
		}
	}
}


/*************************************************************
 ASCENDING/DESCENDING_OBJECTS/VARIABLES
 comparison functions for sorting objects and variables
 *************************************************************/
namespace lsd
{
	bool ascending_objects( const bridge &a, const bridge &b ) { return ( strcmp( a.blabel, b.blabel ) < 0 ); }
	bool descending_objects( const bridge &a, const bridge &b ) { return ( strcmp( a.blabel, b.blabel ) > 0 ); }
	bool ascending_variables( const variable &a, const variable &b ) { return ( strcmp( a.label, b.label ) < 0 ); }
	bool descending_variables( const variable &a, const variable &b ) { return ( strcmp( a.label, b.label ) > 0 ); }
}


/*************************************************************
 SORT_LISTBOX
 *************************************************************/
bool lsd::object::sort_listbox( int box, int order )
{
	bool first;

	// handle variable/parameter list
	if ( box == 1 )
	{
		if ( v == NULL || order < 0 || order > 5 )	// invalid sort?
			return false;

		variable *cv, *cv1 = NULL;
		std::list < variable > newv, newvV, newvP, newvF;
		std::list < variable > :: iterator it;

		// move LSD linked list of variables to a C++ linked list
		for ( cv = v; cv != NULL; cv = cv1 )
		{
			cv1 = cv->next;

			if ( order < 2 )		// no grouping?
				newv.push_back( *cv );
			else
				switch ( cv->param )
				{
					case 0:			// variable
						newvV.push_back( *cv );
						break;
					case 1:			// parameter
						newvP.push_back( *cv );
						break;
					case 2:			// function
						newvF.push_back( *cv );
				}

			delete cv;
		}

		if ( order < 2 )			// no grouping?
			if ( order == 0 )		// ascending order ?
				newv.sort( ascending_variables );
			else					// descending order
				newv.sort( descending_variables );
		else
		{
			switch ( order )
			{
				case 2:				// ascending order by type (par. first)
					newvV.sort( ascending_variables );
					newvP.sort( ascending_variables );
					newvF.sort( ascending_variables );
					newv.splice( newv.end( ), newvP );
					newv.splice( newv.end( ), newvF );
					newv.splice( newv.end( ), newvV );
					break;

				case 3:				// descending order by type (par. first)
					newvV.sort( descending_variables );
					newvP.sort( descending_variables );
					newvF.sort( descending_variables );
					newv.splice( newv.end( ), newvP );
					newv.splice( newv.end( ), newvF );
					newv.splice( newv.end( ), newvV );
					break;

				case 4:				// ascending order by type (var. first)
					newvV.sort( ascending_variables );
					newvP.sort( ascending_variables );
					newvF.sort( ascending_variables );
					newv.splice( newv.end( ), newvV );
					newv.splice( newv.end( ), newvF );
					newv.splice( newv.end( ), newvP );
					break;

				case 5:				// descending order by type (var. first)
					newvV.sort( descending_variables );
					newvP.sort( descending_variables );
					newvF.sort( descending_variables );
					newv.splice( newv.end( ), newvV );
					newv.splice( newv.end( ), newvF );
					newv.splice( newv.end( ), newvP );
					break;
			}
		}

		// rebuild LSD linked list from C++ list
		for ( first = true, it = newv.begin( ); it != newv.end( ); ++it )
		{
			cv = new variable( *it );
			if ( first )
			{
				v = cv;
				first = false;
			}
			else
				cv1->next = cv;
			cv1 = cv;
		}
		cv1->next = NULL;

		recreate_maps( );		// recreate the fast look-up maps

		return true;
	}

	// handle object list
	if ( box == 2 )
	{
		if ( b == NULL || order < 0 || order > 1 )	// invalid sort?
			return false;

		bridge *cb, *cb1 = NULL;
		std::list < bridge > newb;
		std::list < bridge > :: iterator it;

		// move LSD linked list of objects to a C++ linked list
		for ( cb = b; cb != NULL; cb = cb1 )
		{
			cb1 = cb->next;
			newb.push_back( *cb );
			cb->copy = true;		// prevent garbage collection
			delete cb;
		}

		if ( order == 0 )			// ascending order ?
			newb.sort( ascending_objects );
		else						// descending order
			newb.sort( descending_objects );

		// rebuild LSD linked list from C++ list
		for ( first = true, it = newb.begin( ); it != newb.end( ); ++it )
		{
			cb = new bridge( *it );
			if ( first )
			{
				b = cb;
				first = false;
			}
			else
				cb1->next = cb;
			cb1 = cb;
		}

		cb1->next = NULL;

		recreate_maps( );		// recreate the fast look-up maps

		return true;
	}

	return false;
}


/*************************************************************
 UNSAVED_CHANGE
 Read or set the unsaved change flag and update
 windows titles accordingly
 *************************************************************/
bool gui::unsaved_change( bool val )
{
	if ( unsavedChange != val )
	{
		unsavedChange = val;

		char chgMark[ ] = "\0\0";
		chgMark[ 0 ] = unsavedChange ? '*' : ' ';

		// change all the possibly open (single) windows
		for ( int i = 0; i < TK_WIN_NUM; ++i )
		{
			cmd( "if [ winfo exist %s ] { wm title %s \"%s[ string range [ wm title %s ] 1 end ]\" }", tk_wnd_names[ i ], tk_wnd_names[ i ], chgMark, tk_wnd_names[ i ]  );
		}
	}

	return unsavedChange;
}

bool gui::unsaved_change( void )
{
	return unsavedChange;
}


/*************************************************************
 DISCARD_CHANGE
 Ask user to discard changes in configuration, if applicable
 Returns: 0: abort, 1: continue without saving
 *************************************************************/
bool gui::discard_change( bool checkSense, bool senseOnly, const char title[ ] )
{
	// don't stop if simulation is running
	if ( sim.running )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Cannot quit LSD\" -detail \"Cannot quit while simulation is running.\n\n Press 'OK' to continue simulation processing. If you really want to abort the simulation, press 'Stop' first.\"" );
		return false;
	}

	// nothing to save?
	if ( ! unsavedData && ! unsavedChange && ! unsavedSense )
		goto end_true;				// yes: simply discard configuration

	// no: ask for confirmation
	if ( ! senseOnly && unsavedData )
		cmd( "set question \"All data generated and not saved will be lost!\nDo you want to continue?\"" );
	else
		if ( ! senseOnly && unsavedChange )
		{
			if (  strlen( sim.conf_name ) > 0 )
				cmd( "set question \"Recent changes to configuration '%s' are not saved!\nDo you want to discard and continue?\"", sim.conf_name );
			else
				cmd( "set question \"Recent changes to current configuration are not saved!\nDo you want to discard and continue?\"" );
		}
		else						// there is unsaved sensitivity data
		{
			if ( checkSense )
				cmd( "set question \"Recent changes to sensitivity data are not saved!\nDo you want to discard and continue?\"" );
			else
				goto end_true;		// checking sensitivity data is disabled
		}

	// must disable because of a bug in Tk when open dialog
	if ( ! brCovered )
	{
		cmd( ".l.s.c.son_name configure -state disabled" );
		cmd( ".l.v.c.var_name configure -state disabled" );
	}

	cmd( "if [ string equal [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Discard data?%s%s\" -detail $question ] yes ] { \
			set res 1 \
		} else { \
			set res 0 \
		}", strlen( title ) != 0 ? "\n\n" : "", title );

	if ( ! brCovered )
	{
		cmd( ".l.s.c.son_name configure -state normal" );
		cmd( ".l.v.c.var_name configure -state normal" );
	}

	if ( ! get_bool( "res" ) )
		return false;

	end_true:

	if ( currObj != NULL )
		currObj->save_pos( );	// save browser position in structure

	update_model_info( );		// save windows positions if appropriate

	return true;
}


/*************************************************************
 ABORT_RUN_THREADS
 Confirm exiting when there are running threads
 Returns: 0: cancel, 1: continue with exit
 *************************************************************/
bool gui::abort_run_threads( void )
{
#ifndef _NP_
	int res;

	// confirm aborting running parallel processes
	if ( sim.parallel_monitor )
	{
		cmd( "switch [ ttk::messageBox -parent . -type yesnocancel -default yes -icon warning -title Warning -message \"Abort running simulation?\" -detail \"A set of parallel simulation runs is being executed in background. You may choose to interrupt it now, or let it to continue (results and log files will be produced in the configuration file's directory).\n\nChoose 'Yes' to abort before exiting, 'No' to exit without aborting, or 'Cancel' to just return to LSD.\" ] { \
				yes { set res 2 } \
				no { set res 1 } \
				cancel { set res 0 } \
			}" );

		res = get_int( "res" );

		if ( res == 2 )
			if ( ! sim.stop_parallel( ) )
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Failed to abort running simulation\" -detail \"LSD is exiting but the parallel simulation runs will continue (results and log files will be produced in the configuration file's directory).\"" );

		if ( res == 1 )
			sim.detach_parallel( );

		if ( res == 0 )
			return false;
		else
			return true;
	}
#endif

	return true;
}


/*************************************************************
 TCL_ABORT_RUN_THREADS
 Entry point function for access from the Tcl
 interpreter
 *************************************************************/
int gui::Tcl_abort_run_threads( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] )
{
	if ( abort_run_threads( ) == 1 )
		Tcl_SetResult( interp, ( char * ) "ok", TCL_VOLATILE );
	else
		Tcl_SetResult( interp, ( char * ) "cancel", TCL_VOLATILE );

	return TCL_OK;
}
