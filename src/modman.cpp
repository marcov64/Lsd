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
MODMAN.CPP
This program is a front end for dealing with LSD models code (running, compiling, editing,
debugging LSD model programs). See the manual for help on its use.

IMPORTANT: this is _NOT_ a LSD model, but the best I could produce of something similar to
a programming environment for LSD model programs.

This file can be compiled with the command line:

make -f <makefile>

There are several makefiles in LSD root directory appropriate to different environments
(Windows, Mac & Linux) and configurations (32 or 64-bit).

LMM starts in a quite weird way. If there is no parameter in the call used to start it, the only
operation it does is to ... run a copy of itself followed by the parameter "kickstart". This trick
is required because under Windows there are troubles launching external package from a "first
instance" of a program.

LMM reads all the directories that are not: Manual, gnu, installer, LMM.app, lwi, Rpkg and src as model
directories, where it expect to find certain files. At any given moment a model name is stored,
together with its directory and the file shown.

Any internal command is executed in a condition like this:

if ( choice == x )
 do_this_and_that

and returned to the main cycle. After each block the flow returns to "loop" where the main
Tcl_DoOneEvent loop sits.

The widget of importance are:
- .f.t.t is the main text editor
- .f.m is the frame containing the upper buttons, models list and help window
*************************************************************/

/*****
used up to 88 options
*******/

// common definitions for LMM and LSD
#include "common.h"

// auxiliary C procedures
bool compile_run( bool run, bool nw = false );
bool is_source_file( const char *fname );
bool use_eigen( void );			// check is Eigen library is in use
char *get_fun_name( char *str, bool nw = false );
void check_option_files( bool sys = false );
void color( int hiLev, long iniLin, long finLin );
void create_compresult_window( bool nw = false );
void make_makefile( bool nw = false );

// global variables
bool tk_ok = false;				// control for tk_ready to operate
bool sourcefile = false;		// current file type
char *exec_path = NULL;			// path of executable file
char *rootLsd = NULL;			// path of LSD root directory
char err_file[ ] = "LMM.err";	// error log file name
char msg[ TCL_BUFF_STR ] = "";	// auxiliary Tcl buffer
int choice;						// Tcl menu control variable
int platform = 0;				// OS platform (1=Linux, 2=Mac, 3=Windows)
int shigh;						// syntax highlighting state (0, 1 or 2)
int tosave = false;				// modified file flag
Tcl_Interp *inter = NULL;		// Tcl standard interpreter pointer

// constant string arrays
const char *lmm_options[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_NAME;
const char *lmm_defaults[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_DEFAULT;
const char *model_info[ MODEL_INFO_NUM ] = MODEL_INFO_NAME;
const char *model_defaults[ MODEL_INFO_NUM ] = MODEL_INFO_DEFAULT;
const char *lsd_dir[ LSD_DIR_NUM ] = LSD_DIR_NAME;
const char *signal_names[ REG_SIG_NUM ] = REG_SIG_NAME;
const char *wnd_names[ LSD_WIN_NUM ] = LSD_WIN_NAME;
const int signals[ REG_SIG_NUM ] = REG_SIG_CODE;


/*************************************
 LSDMAIN
 *************************************/
int lsdmain( int argn, char **argv )
{
	bool found, recolor = false;
	int i, j, num, recolor_all = 0, v_counter = 0;
	char *s, str[ 5 * MAX_PATH_LENGTH ], str1[ 2 * MAX_PATH_LENGTH ], str2[ 6 * MAX_PATH_LENGTH ];
	FILE *f;

	// initialize tcl/tk and set global bidirectional variables
	init_tcl_tk( argv[ 0 ], "lmm" );
	Tcl_LinkVar( inter, "num", ( char * ) &num, TCL_LINK_INT );
	Tcl_LinkVar( inter, "shigh", ( char * ) &shigh, TCL_LINK_INT );
	Tcl_LinkVar( inter, "choice", ( char * ) &choice, TCL_LINK_INT );
	Tcl_LinkVar( inter, "tosave", ( char * ) &tosave, TCL_LINK_BOOLEAN);
	Tcl_LinkVar( inter, "recolor_all", ( char * ) &recolor_all, TCL_LINK_BOOLEAN);

	// set system defaults in tcl
	cmd( "set LMM_OPTIONS \"%s\"", LMM_OPTIONS );
	cmd( "set SYSTEM_OPTIONS \"%s\"", SYSTEM_OPTIONS );
	cmd( "set MODEL_OPTIONS \"%s\"", MODEL_OPTIONS );
	cmd( "set GROUP_INFO \"%s\"", GROUP_INFO );
	cmd( "set MODEL_INFO \"%s\"", MODEL_INFO );
	cmd( "set MODEL_INFO_NUM %d", MODEL_INFO_NUM );
	cmd( "set DESCRIPTION \"%s\"", DESCRIPTION );
	cmd( "set DATE_FMT \"%s\"", DATE_FMT );

	// try to open text file if name is provided in the command line
	if ( argn > 1 )
	{
		for ( i = 0; argv[ 1 ][ i ] != '\0'; ++i )
		{
			if ( argv[ 1 ][ i ] == '\\' )
				msg[ i ] = '/';
			else
				msg[ i ] = argv[ 1 ][ i ];
		}
		msg[ i ] = '\0';
		cmd( "set filetoload \"%s\"", msg );
		cmd( "if { ! [ file pathtype \"$filetoload\" ] == \"absolute\" } { set filetoload \"[ pwd ]/$filetoload\" }" );
	}

	// prepare to use exec path to find LSD directory
	cmd( "if { [ info nameofexecutable ] != \"\" } { set path [ file dirname [ info nameofexecutable ] ] } { set path \"[ pwd ]\" }" );
	s = ( char * ) Tcl_GetVar( inter, "path", 0 );
	if ( s != NULL && strlen( s ) > 0 )
	{
		exec_path = new char[ strlen( s ) + 1 ];
		strcpy( exec_path, s );
		exec_path = clean_path( exec_path );
	}
	else
	{
		log_tcl_error( "LMM executable check", "Cannot locate LSD executable on disk, check the installation of LSD and reinstall LSD if the problem persists" );
		cmd( "ttk::messageBox -type ok -icon error -title Error -message \"LMM executable not found\" -detail \"Cannot locate the LMM executable folder on disk.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"" );
		return 5;
	}

	// check if LSDROOT environment variable exists and use it if so
	cmd( "if [ info exists env(LSDROOT) ] { set RootLsd [ file normalize $env(LSDROOT) ]; if [ file exists \"$RootLsd/Manual/LMM.html\" ] { cd \"$RootLsd\"; set choice 0 } { set choice 1 } } { set choice 1 }" );

	if ( choice )
	{
		choice = 0;
		cmd( "set RootLsd [ file normalize \"%s\" ]", exec_path );
		// check if directory is ok and if executable is inside a macOS package
		cmd( "if [ file exists \"$RootLsd/Manual/LMM.html\" ] { \
				cd \"$RootLsd\" \
			} { \
				if [ file exists \"$RootLsd/../../../Manual/LMM.html\" ] { \
					cd \"$RootLsd/../../..\"; \
					set RootLsd \"[ pwd ]\" \
				} { \
					unset -nocomplain RootLsd; \
					set choice 1 \
				} \
			}" );
		if ( choice )
		{
			log_tcl_error( "Source files check", "Required LSD source file(s) missing or corrupted, check the installation of LSD and reinstall LSD if the problem persists" );
			cmd( "ttk::messageBox -type ok -icon error -title Error -message \"File(s) missing or corrupted\" -detail \"Some critical LSD files or folders are missing or corrupted.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"" );
			return 6;
		}
		
		cmd( "set env(LSDROOT) $RootLsd" );
	}

	s =  ( char * ) Tcl_GetVar( inter, "RootLsd", 0 );
	if ( s != NULL && strlen( s ) > 0 )
	{
		rootLsd = new char[ strlen( s ) + 1 ];
		strcpy( rootLsd, s );
		rootLsd = clean_path( rootLsd );
		cmd( "set RootLsd \"%s\"", rootLsd );
	}
	else
	{
		log_tcl_error( "LSD directory check", "Cannot locate LSD folder on disk, check the installation of LSD and reinstall LSD if the problem persists" );
		cmd( "ttk::messageBox -type ok -icon error -title Error -message \"LSD directory missing\" -detail \"Cannot locate the LSD installation folder on disk.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"" );
		return 7;
	}
		
	// load/check configuration files
	i = load_lmm_options( );
	check_option_files( true );

	// load required Tcl/Tk data, procedures and packages (error coded by file/bit position)
	choice = 0;

	// load native Tk procedures for graphical user interface management
	cmd( "if [ file exists \"$RootLsd/$LsdSrc/gui.tcl\" ] { if [ catch { source \"$RootLsd/$LsdSrc/gui.tcl\" } err0x01 ] { set choice [ expr $choice + %d ] } } { set choice [ expr $choice + %d ] }", 0x0100, 0x01 );

	// load native Tcl procedures for external files handling
	cmd( "if [ file exists \"$RootLsd/$LsdSrc/file.tcl\" ] { if [ catch { source \"$RootLsd/$LsdSrc/file.tcl\" } err0x02 ] { set choice [ expr $choice + %d ] } } { set choice [ expr $choice + %d ] }", 0x0200, 0x02 );

	// load native Tcl procedures for general utilities
	cmd( "if [ file exists \"$RootLsd/$LsdSrc/util.tcl\" ] { if [ catch { source \"$RootLsd/$LsdSrc/util.tcl\" } err0x04 ] { set choice [ expr $choice + %d ] } } { set choice [ expr $choice + %d ] }", 0x0400, 0x04 );

	// load the native model browser module
	cmd( "if [ file exists \"$RootLsd/$LsdSrc/model.tcl\" ] { if [ catch { source \"$RootLsd/$LsdSrc/model.tcl\" } err0x08 ] { set choice [ expr $choice + %d ] } } { set choice [ expr $choice + %d ] }", 0x0800, 0x08 );

	if ( choice != 0 )
	{
		char *err0x01 = ( char * ) Tcl_GetVar( inter, "err0x01", 0 );
		char *err0x02 = ( char * ) Tcl_GetVar( inter, "err0x02", 0 );
		char *err0x04 = ( char * ) Tcl_GetVar( inter, "err0x04", 0 );
		char *err0x08 = ( char * ) Tcl_GetVar( inter, "err0x08", 0 );
		snprintf( msg, TCL_BUFF_STR - 1, "Required Tcl/Tk source file(s) missing or corrupted (0x%04x), check your installation and reinstall LSD if the problem persists\n\n0x01: %s\n\n0x02: %s\n\n0x04: %s\n\n0x08: %s", choice, err0x01, err0x02, err0x04, err0x08 );
		log_tcl_error( "Source files check failed", msg );
		cmd( "ttk::messageBox -type ok -icon error -title Error -message \"File(s) missing or corrupted\" -detail \"Some critical Tcl files (0x%04x) are missing or corrupted.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is aborting now.\"", choice );
		return 10 + choice;
	}

	s = ( char * ) Tcl_GetVar( inter, "CurPlatform", 0 );
	if ( ! strcmp( s, "linux" ) )
		platform = LINUX;
	else
		if ( ! strcmp( s, "mac" ) )
			platform = MAC;
		else
			if ( ! strcmp( s, "windows" ) )
				platform = WINDOWS;
			else
			{
				log_tcl_error( "Unsupported platform", "Your computer operating system is not supported by this LSD version, you may try an older version compatible with legacy systems (Windows 32-bit, Mac OS X, etc.)" );
				cmd( "ttk::messageBox -type ok -icon error -title Error -message \"Unsupported platform\" -detail \"Your computer operating system is not supported by this LSD version,\nyou may try an older version compatible with legacy systems\n(Windows 32-bit, Mac OS X, etc.)\n\nLSD is aborting now.\"", choice );
				return 10;
			}

	// create a Tcl command that calls the C discard_change function before killing LMM
	Tcl_CreateCommand( inter, "discard_change", Tcl_discard_change, NULL, NULL );

	// Tcl command to save message to LSD log
	Tcl_CreateCommand( inter, "log_tcl_error", Tcl_log_tcl_error, NULL, NULL );

	// fix non-existent or old options file for new options
	if ( i == 0 )
		update_lmm_options(  ); 		// update config file

	// Tcl global variables
	cmd( "set choice 0" );
	cmd( "set recolor \"\"" );
	cmd( "set docase 1" );
	cmd( "set dirsearch \"-forwards\"" );
	cmd( "set lfind [ list ]");
	cmd( "set lfindsize 0" );
	cmd( "set endsearch end" );
	cmd( "set currentpos \"\"" );
	cmd( "set currentdoc \"\"" );
	cmd( "set v_num 0" );
	cmd( "set alignMode \"LMM\"" );
	cmd( "set MakeExe \"$DefaultMakeExe\"" );
	cmd( "set small_character [ expr $dim_character - $deltaSize ]" );

	// configure main window
	cmd( ". configure -menu .m -background $colorsTheme(bg)" );
	cmd( "icontop . lmm" );
	cmd( "sizetop .lmm" );
	cmd( "setglobkeys ." );				// set global keys for main window
	cmd( "setstyles" );					// set ttk custom style

	// main menu
	cmd( "ttk::menu .m -tearoff 0" );

	cmd( "set w .m.file" );
	cmd( "ttk::menu $w -tearoff 0" );
	cmd( ".m add cascade -label File -menu $w -underline 0" );
	cmd( "$w add command -label \"New Model/Group...\" -underline 0 -command { set choice 14 }" );	// entryconfig 0
	cmd( "$w add command -label \"Browse Models...\" -underline 0 -command { set choice 33 } -accelerator Ctrl+b" );	// entryconfig 1
	cmd( "$w add command -label \"Save Model\" -underline 0 -state disabled -command { \
			if { [ string length \"$filename\" ] > 0 } { \
				if [ file exist \"$dirname/$filename\" ] { \
					catch { \
						file copy -force \"$dirname/$filename\" \"$dirname/[ file rootname \"$filename\" ].bak\" \
					} \
				}; \
				set f [ open \"$dirname/$filename\" w ]; \
				puts -nonewline $f [ .f.t.t get 0.0 end ]; \
				close $f; \
				set before [ .f.t.t get 0.0 end ]; \
				set choice 999 \
			} \
		} -underline 0 -accelerator Ctrl+s" );	// entryconfig 2
	cmd( "$w add command -label \"Save Model As...\" -state disabled -underline 0 -command { set choice 41 } -underline 11 -accelerator Ctrl+a" );	// entryconfig 3
	cmd( "$w add separator" );	// entryconfig 4
	cmd( "$w add command -label \"Compare Models...\" -underline 3 -command { set choice 61 } -underline 0" );		// entryconfig 5
	cmd( "$w add separator" );		// entryconfig 6

	cmd( "if { $showFileCmds } { $w add command -label \"New Text File\" -command { set choice 39 } -underline 4 }" );		// entryconfig (7)
	cmd( "if { $showFileCmds } { $w add command -label \"Open Text File...\" -command { set choice 15 } -underline 0 -accelerator Ctrl+o }" );		// entryconfig (8)
	cmd( "if { $showFileCmds == 1 } { \
			$w add command -label \"Save Text File\" -command { \
				if { [ string length \"$filename\" ] > 0 } { \
					if [ file exist \"$dirname/$filename\" ] { \
						catch { \
							file copy -force \"$dirname/$filename\" \"$dirname/[ file rootname \"$filename\" ].bak\" \
						} \
					}; \
					set f [ open \"$dirname/$filename\" w ]; \
					puts -nonewline $f [ .f.t.t get 0.0 end ]; \
					close $f; \
					set before [ .f.t.t get 0.0 end ]; \
					set choice 999 \
				} \
			} -underline 2 }" );		// entryconfig (9)
	cmd( "if { $showFileCmds } { $w add command -label \"Save Text File As...\" -command { set choice 4 } -underline 3 }" );		// entryconfig (10)
	cmd( "if { $showFileCmds } { $w add separator }" );		// entryconfig (11)

	cmd( "$w add command -label \"Options...\" -command { set choice 60 } -underline 1" );		// entryconfig 7/(12)
	cmd( "$w add separator" );		// entryconfig 8/(13)
	cmd( "$w add command -label \"Quit\" -command { set choice 1 } -underline 0 -accelerator Ctrl+q" );		// entryconfig 4/(14)

	cmd( "set w .m.edit" );
	cmd( "ttk::menu $w -tearoff 0" );
	cmd( ".m add cascade -label Edit -menu $w -underline 0" );

	// make menu the same as ctrl-z/y (more color friendly)
	cmd( "$w add command -label Undo -command { \
			catch { \
				.f.t.t edit undo; \
				set recolor_all 1 \
			} \
		} -underline 0 -accelerator Ctrl+z" );	// entryconfig 0
	cmd( "$w add command -label Redo -command { \
			catch { \
				.f.t.t edit redo; \
				set recolor_all 1 \
			} \
		} -underline 2 -accelerator Ctrl+y" );	// entryconfig 1
	cmd( "$w add separator" );	// entryconfig 2
	// collect information to focus recoloring
	cmd( "$w add command -label Cut -command { \
			sav_cur_ini; \
			tk_textCut .f.t.t; \
			upd_color \
		} -underline 1 -accelerator Ctrl+x" );	// entryconfig 3
	cmd( "$w add command -label Copy -command { tk_textCopy .f.t.t } -underline 0 -accelerator Ctrl+c" );	// entryconfig 4
	cmd( "$w add command -label Paste -command { \
			sav_cur_ini; \
			tk_textPaste .f.t.t; \
			upd_color \
		} -underline 0 -accelerator Ctrl+v" );	// entryconfig 5
	cmd( "$w add command -label Delete -command { \
			sav_cur_ini; \
			if [ string equal [ .f.t.t tag ranges sel ] \"\" ] { \
				.f.t.t delete insert \
			} { \
				.f.t.t delete sel.first sel.last \
			}; \
			upd_color \
		} -accelerator Del" );	// entryconfig 6
	cmd( "$w add separator" );	// entryconfig 7
	cmd( "$w add command -label \"Find...\" -command { set choice 11 } -underline 0 -accelerator Ctrl+f" );	// entryconfig 8
	cmd( "$w add command -label \"Find Again\" -command { set choice 12 } -underline 5 -accelerator F3" );	// entryconfig 9
	cmd( "$w add command -label \"Find Backwards\" -command { set choice 88 } -underline 5 -accelerator Ctrl+F3" );	// entryconfig 10
	cmd( "$w add command -label \"Replace...\" -command { set choice 21 } -underline 0" );	// entryconfig 11
	cmd( "$w add command -label \"Go to Line...\" -command { set choice 10 } -underline 6 -accelerator Ctrl+l" );	// entryconfig 12
	cmd( "$w add separator" );	// entryconfig 13
	cmd( "$w add command -label \"Match \\\{ \\}\" -command { set choice 17 } -accelerator Ctrl+m" );	// entryconfig 14
	cmd( "$w add command -label \"Match \\\( \\)\" -command { set choice 32 } -accelerator Ctrl+u" );	// entryconfig 15
	cmd( "$w add command -label \"Insert \\\{\" -command { .f.t.t insert insert \\\{ } -accelerator Ctrl+\\\(" );	// entryconfig 16
	cmd( "$w add command -label \"Insert \\}\" -command { .f.t.t insert insert \\} } -accelerator Ctrl+\\)" );	// entryconfig 17
	cmd( "$w add separator" );	// entryconfig 18
	cmd( "$w add command -label \"Indent\" -command { set choice 42 } -accelerator Ctrl+>" );	// entryconfig 19
	cmd( "$w add command -label \"De-indent\" -command { set choice 43 } -accelerator Ctrl+<" );	// entryconfig 20
	cmd( "$w add separator" );	// entryconfig 21
	cmd( "$w add command -label \"Larger Font\" -command { \
			if { $dim_character < 60 } { \
				incr dim_character 1; \
				ttk::style configure fixed.TText -font [ font create -family \"$fonttype\" -size $dim_character ]; \
				settab .f.t.t $tabsize fixed.TText \
			} \
		} -accelerator \"Ctrl +\"" );	// entryconfig 22
	cmd( "$w add command -label \"Smaller Font\" -command { \
			if { $dim_character > 4 } { \
				incr dim_character -1; \
				ttk::style configure fixed.TText -font [ font create -family \"$fonttype\" -size $dim_character ]; \
				settab .f.t.t $tabsize fixed.TText \
			} \
		} -accelerator \"Ctrl -\"" );	// entryconfig 23
	cmd( "$w add separator" );	// entryconfig 24
	// add option to ajust syntax highlighting (word coloring)
	cmd( "$w add check -label \"Wrap/Unwrap\" -variable wrap -command { setwrap .f.t.t $wrap } -underline 1 -accelerator Ctrl+w " );
	cmd( "$w add command -label \"Insert LSD Macro...\" -command { set choice 28 } -underline 0 -accelerator Ctrl+i" );

	cmd( "set w .m.model" );
	cmd( "ttk::menu $w -tearoff 0" );
	cmd( ".m add cascade -label Model -menu $w -underline 0" );
	cmd( "$w add command -label \"Compile and Run...\" -state disabled -underline 0 -command { set choice 2 } -accelerator F5" );	// entryconfig 0
	cmd( "$w add command -label \"Recompile\" -state disabled -underline 0 -command { set choice 6 } -accelerator F6" );	// entryconfig 1
	cmd( "$w add command -label \"[ string toupper $DbgExe ] Debugger\" -state disabled -underline 0 -command { set choice 13 } -accelerator F7" );	// entryconfig 2
	cmd( "$w add command -label \"Create 'No Window' Version\" -underline 8 -state disabled -command { set choice 62 }" );	// entryconfig 3
	cmd( "$w add command -label \"Model Info...\" -underline 6 -state disabled -command { set choice 44 }" );	// entryconfig 4
	cmd( "$w add separator" );	// entryconfig 5
	cmd( "$w add command -label \"Show Description\" -underline 5 -state disabled -command { set choice 5 } -accelerator Ctrl+d" );	// entryconfig 6
	cmd( "$w add command -label \"Show Equations\" -state disabled -underline 5 -command { set choice 8 } -accelerator Ctrl+e" );	// entryconfig 7
	cmd( "$w add command -label \"Show Extra Files...\" -state disabled -underline 6 -command { set choice 70 } -accelerator Ctrl+j" );	// entryconfig 8
	cmd( "$w add command -label \"Show Make File\" -state disabled -underline 7 -command { set choice 3 }" );	// entryconfig 9
	cmd( "$w add command -label \"Show Compilation Errors\" -underline 6 -state disabled -command { set choice 7 }" );	// entryconfig 10
	cmd( "$w add separator" );	// entryconfig 11
	cmd( "$w add command -label \"Model Options...\" -underline 4 -state disabled -command { set choice 48 }" );	// entryconfig 12
	cmd( "$w add command -label \"System Options...\" -underline 0 -command { set choice 47 }" );	// entryconfig 13
	cmd( "$w add separator" );	// entryconfig 14
	cmd( "$w add check -label \"Auto-hide LMM\" -variable autoHide -underline 0" );	// entryconfig 15

	cmd( "set w .m.help" );
	cmd( "ttk::menu $w -tearoff 0" );
	cmd( ".m add cascade -label Help -menu $w -underline 0" );
	cmd( "$w add command -label \"Help on LMM\" -underline 0 -accelerator F1 -command { LsdHelp LMM.html }" );
	cmd( "$w add command -label \"Help on Macros for LSD Equations\" -underline 8 -command { LsdHelp LSD_macros.html }" );
	cmd( "$w add command -label \"LSD Documentation\" -underline 4 -command { LsdHelp LSD_documentation.html }" );
	cmd( "$w add separator" );
	cmd( "$w add command -label \"LMM Primer Tutorial\" -underline 4 -command { LsdHelp LMM_primer.html }" );
	cmd( "$w add command -label \"Using LSD Models Tutorial\" -underline 0 -command { LsdHelp model_using.html }" );
	cmd( "$w add command -label \"Writing LSD Models Tutorial\" -underline 0 -command { LsdHelp model_writing.html }" );
	cmd( "$w add separator" );
	cmd( "$w add command -label \"About LSD...\" -underline 0 -command { LsdAbout {%s} {%s} }", _LSD_VERSION_, _LSD_DATE_  );

	// Button bar
	cmd( "ttk::frame .bbar" );

	cmd( "ttk::button .bbar.open -image openImg -style Toolbutton -command { set choice 33 }" );
	cmd( "ttk::button .bbar.save -image saveImg -style Toolbutton -command { .m.file invoke 2 }" );
	cmd( "ttk::button .bbar.undo -image undoImg -style Toolbutton -command { .m.edit invoke 0 }" );
	cmd( "ttk::button .bbar.redo -image redoImg -style Toolbutton -command { .m.edit invoke 1 }" );
	cmd( "ttk::button .bbar.cut -image cutImg -style Toolbutton -command { .m.edit invoke 3 }" );
	cmd( "ttk::button .bbar.copy -image copyImg -style Toolbutton -command { .m.edit invoke 4 }" );
	cmd( "ttk::button .bbar.paste -image pasteImg -style Toolbutton -command { .m.edit invoke 5 }" );
	cmd( "ttk::button .bbar.find -image findImg -style Toolbutton -command { set choice 11 }" );
	cmd( "ttk::button .bbar.replace -image replaceImg -style Toolbutton -command { set choice 21 }" );
	cmd( "ttk::button .bbar.indent -image indentImg -style Toolbutton -command { set choice 42 }" );
	cmd( "ttk::button .bbar.deindent -image deindentImg -style Toolbutton -command { set choice 43 }" );
	cmd( "ttk::button .bbar.wrap -image wrapImg -style Toolbutton -command { \
			if { $wrap == 0 } { \
				set wrap 1 \
			} { \
				set wrap 0 \
			}; \
			setwrap .f.t.t $wrap \
		}" );
	cmd( "ttk::button .bbar.compile -image compileImg -style Toolbutton -command { set choice 6 }" );
	cmd( "ttk::button .bbar.comprun -image comprunImg -style Toolbutton -command { set choice 2 }" );
	cmd( "ttk::button .bbar.gdb -image gdbImg -style Toolbutton -command { set choice 13 }" );
	cmd( "ttk::button .bbar.info -image infoImg -style Toolbutton -command { set choice 44 }" );
	cmd( "ttk::button .bbar.descr -image descrImg -style Toolbutton -command { set choice 5 }" );
	cmd( "ttk::button .bbar.equation -image equationImg -style Toolbutton -command { set choice 8 }" );
	cmd( "ttk::button .bbar.extra -image extraImg -style Toolbutton -command { set choice 70 }" );
	cmd( "ttk::button .bbar.set -image setImg -style Toolbutton -command { set choice 48 }" );
	cmd( "ttk::button .bbar.hide -image hideImg -style Toolbutton -command { set autoHide [ expr ! $autoHide ] }" );
	cmd( "ttk::button .bbar.help -image helpImg -style Toolbutton -command { LsdHelp LSD_macros.html }" );
	cmd( "ttk::label .bbar.tip -textvariable ttip -width 30 -style graySmall.TLabel -anchor w" );

	cmd( "bind .bbar.open <Enter> { set ttip \"Browse models...\" }" );
	cmd( "bind .bbar.open <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.save <Enter> { set ttip \"Save model\" }" );
	cmd( "bind .bbar.save <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.undo <Enter> { set ttip \"Undo\" }" );
	cmd( "bind .bbar.undo <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.redo <Enter> { set ttip \"Redo\" }" );
	cmd( "bind .bbar.redo <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.cut <Enter> { set ttip \"Cut\" }" );
	cmd( "bind .bbar.cut <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.copy <Enter> { set ttip \"Copy\" }" );
	cmd( "bind .bbar.copy <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.paste <Enter> { set ttip \"Paste\" }" );
	cmd( "bind .bbar.paste <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.find <Enter> { set ttip \"Find...\" }" );
	cmd( "bind .bbar.find <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.replace <Enter> { set ttip \"Replace...\" }" );
	cmd( "bind .bbar.replace <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.indent <Enter> { set ttip \"Indent selection\" }" );
	cmd( "bind .bbar.indent <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.deindent <Enter> { set ttip \"De-indent selection\" }" );
	cmd( "bind .bbar.deindent <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.wrap <Enter> { set ttip \"Wrap lines\" }" );
	cmd( "bind .bbar.wrap <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.compile <Enter> { set ttip \"Recompile model\" }" );
	cmd( "bind .bbar.compile <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.comprun <Enter> { set ttip \"Compile and run model...\" }" );
	cmd( "bind .bbar.comprun <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.gdb <Enter> { set ttip \"Run in [ string toupper $DbgExe ] debugger\" }" );
	cmd( "bind .bbar.gdb <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.info <Enter> { set ttip \"Model information...\" }" );
	cmd( "bind .bbar.info <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.descr <Enter> { set ttip \"Show description\" }" );
	cmd( "bind .bbar.descr <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.equation <Enter> { set ttip \"Show equations\" }" );
	cmd( "bind .bbar.equation <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.extra <Enter> { set ttip \"Show extra files...\" }" );
	cmd( "bind .bbar.extra <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.set <Enter> { set ttip \"Model compilation options...\" }" );
	cmd( "bind .bbar.set <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.hide <Enter> { \
			if $autoHide { \
				set ttip \"Not auto-hide LMM\" \
			} { \
				set ttip \"Auto-hide LMM\" \
			} \
		}" );
	cmd( "bind .bbar.hide <Leave> { set ttip \"\" }" );
	cmd( "bind .bbar.help <Enter> { set ttip \"Help on macros for LSD equations\" }" );
	cmd( "bind .bbar.help <Leave> { set ttip \"\" }" );

	cmd( "pack .bbar.open .bbar.save .bbar.undo .bbar.redo .bbar.cut .bbar.copy .bbar.paste .bbar.find .bbar.replace .bbar.indent .bbar.deindent .bbar.wrap .bbar.compile .bbar.comprun .bbar.gdb .bbar.info .bbar.descr .bbar.equation .bbar.extra .bbar.set .bbar.hide .bbar.help .bbar.tip -side left" );
	cmd( "pack .bbar -padx 3 -anchor w -fill x" );

	cmd( "ttk::frame .f" );
	cmd( "ttk::frame .f.t" );
	cmd( "ttk::scrollbar .f.t.vs -command \".f.t.t yview\"" );
	cmd( "ttk::scrollbar .f.t.hs -orient horiz -command \".f.t.t xview\"" );
	cmd( "ttk::text .f.t.t -autoseparators 1 -yscroll \".f.t.vs set\" -xscroll \".f.t.hs set\" -dark $darkTheme -style fixed.TText" );
	cmd( "mouse_wheel .f.t.t" );

	// set preferred tab size and wrap option
	cmd( "settab .f.t.t $tabsize fixed.TText" );	// adjust tabs size to font type/size
	cmd( "setwrap .f.t.t $wrap" );		// adjust text wrap

	// set colors
	cmd( ".f.t.t tag configure comment1 -foreground $colorsTheme(comm)" );
	cmd( ".f.t.t tag configure comment2 -foreground $colorsTheme(comm)" );
	cmd( ".f.t.t tag configure str -foreground $colorsTheme(str)" );
	cmd( ".f.t.t tag configure cprep -foreground $colorsTheme(prep)" );
	cmd( ".f.t.t tag configure ctype -foreground $colorsTheme(type)" );
	cmd( ".f.t.t tag configure ckword -foreground $colorsTheme(kwrd)" );
	cmd( ".f.t.t tag configure lsdvar -foreground $colorsTheme(vlsd)" );
	cmd( ".f.t.t tag configure lsdmacro -foreground $colorsTheme(mlsd)" );

	cmd( "ttk::frame .f.hea" );

	cmd( "ttk::frame .f.hea.info" );

	cmd( "ttk::frame .f.hea.info.grp" );
	cmd( "ttk::label .f.hea.info.grp.tit -text \"Group: \"" );
	cmd( "ttk::label .f.hea.info.grp.dat -text \"$modelGroup\" -style hl.TLabel" );
	cmd( "pack .f.hea.info.grp.tit .f.hea.info.grp.dat -side left" );

	cmd( "ttk::label .f.hea.info.pad1 -width 2" );

	cmd( "ttk::frame .f.hea.info.mod" );
	cmd( "ttk::label .f.hea.info.mod.tit -text \"Model: \"" );
	cmd( "ttk::label .f.hea.info.mod.dat -text \"(no model)\" -style hl.TLabel" );
	cmd( "pack .f.hea.info.mod.tit .f.hea.info.mod.dat -side left" );

	cmd( "ttk::label .f.hea.info.pad2 -width 2" );

	cmd( "ttk::frame .f.hea.info.ver" );
	cmd( "ttk::label .f.hea.info.ver.tit -text \"Version: \"" );
	cmd( "ttk::label .f.hea.info.ver.dat -text \"\" -style hl.TLabel" );
	cmd( "pack .f.hea.info.ver.tit .f.hea.info.ver.dat -side left" );

	cmd( "ttk::label .f.hea.info.pad3 -width 2" );

	cmd( "ttk::frame .f.hea.info.file" );
	cmd( "ttk::label .f.hea.info.file.tit -text \"File: \"" );
	cmd( "ttk::label .f.hea.info.file.dat -text \"(no file)\" -style hl.TLabel" );
	cmd( "pack .f.hea.info.file.tit .f.hea.info.file.dat -side left" );

	cmd( "pack .f.hea.info.grp .f.hea.info.pad1 .f.hea.info.mod .f.hea.info.pad2 .f.hea.info.ver .f.hea.info.pad3 .f.hea.info.file -side left" );

	cmd( "pack .f.hea.info -side left -anchor w -expand yes" );

	cmd( "ttk::frame .f.hea.cur" );

	cmd( "ttk::frame .f.hea.cur.line" );
	cmd( "ttk::label .f.hea.cur.line.ln1 -text \"Ln:\"" );
	cmd( "ttk::label .f.hea.cur.line.ln2 -width 4 -style hlBoldSmall.TLabel -text 1" );
	cmd( "pack .f.hea.cur.line.ln1 .f.hea.cur.line.ln2 -side left" );

	cmd( "ttk::frame .f.hea.cur.col" );
	cmd( "ttk::label .f.hea.cur.col.col1 -text \"Col:\"" );
	cmd( "ttk::label .f.hea.cur.col.col2 -width 3 -style hlBoldSmall.TLabel -text 1" );
	cmd( "pack .f.hea.cur.col.col1 .f.hea.cur.col.col2 -side left" );

	cmd( "pack .f.hea.cur.line .f.hea.cur.col -side left" );

	cmd( "pack .f.hea.cur -side left -anchor e" );

	cmd( "pack .f.hea -padx 5 -pady 3 -fill x" );

	cmd( "pack .f.t -expand yes -fill both" );
	cmd( "pack .f.t.vs -side right -fill y" );
	cmd( "pack .f.t.t -expand yes -fill both" );
	cmd( "pack .f.t.hs -fill x" );

	// redefine bindings to better support new syntax highlight routine
	cmd( "bind .f.t.t <KeyPress> { sav_cur_ini }" );
	cmd( "bind .f.t.t <KeyRelease> { upd_color }" );
	cmd( "bind .f.t.t <ButtonPress> { sav_cur_ini }" );
	cmd( "bind .f.t.t <ButtonRelease> { upd_color }" );

	// button and key specific bindings
	cmd( "bind .f.hea.cur.line.ln1 <Button-1> { set choice 10 }" );
	cmd( "bind .f.hea.cur.line.ln2 <Button-1> { set choice 10 }" );
	cmd( "bind .f.hea.cur.col.col1 <Button-1> { set choice 10 }" );
	cmd( "bind .f.hea.cur.col.col2 <Button-1> { set choice 10 }" );

	cmd( "bind . <F1> { .m.help invoke 0; break }" );
	cmd( "bind . <Insert> { }" );

	cmd( "bind .f.t.t <F3> { set choice 12 }" );
	cmd( "bind .f.t.t <Control-F3> { set choice 88 }" );

	cmd( "bind .f.t.t <Control-l> { set choice 10 }; bind .f.t.t <Control-L> { set choice 10 }" );
	cmd( "bind .f.t.t <Control-w> { \
			if { $wrap == 0 } { \
				set wrap 1 \
			} { \
				set wrap 0 \
			}; \
			setwrap .f.t.t $wrap \
		}" );

	cmd( "bind .f.t.t <Control-f> { set choice 11 }; bind .f.t.t <Control-F> { set choice 11 }" );
	cmd( "bind .f.t.t <Control-s> { .m.file invoke 2 }" );
	cmd( "bind .f.t.t <Control-a> { set choice 4 }" );
	cmd( "bind .f.t.t <Control-r> { set choice 2 }; bind .f.t.t <Control-R> { set choice 2 }; bind .f.t.t <F5> { set choice 2 }" );
	cmd( "bind .f.t.t <Control-p> { set choice 6; break }; bind .f.t.t <Control-P> { set choice 6; break }; bind .f.t.t <F6> { set choice 6 }" );
	cmd( "bind .f.t.t <Control-g> { set choice 13 }; bind .f.t.t <Control-G> { set choice 13 }; bind .f.t.t <F7> { set choice 13 }" );
	cmd( "bind .f.t.t <Control-e> { set choice 8 }" );
	cmd( "bind .f.t.t <Control-j> { set choice 70 }" );
	cmd( "bind .f.t.t <Control-o> { if { $showFileCmds } { set choice 8; break } { break } }" );
	cmd( "bind .f.t.t <Control-q> { set choice 1 }; bind .f.t.t <Control-Q> { set choice 1 }" );
	cmd( "bind .f.t.t <Control-u> { set choice 32 }" );
	cmd( "bind .f.t.t <Control-m> { set choice 17 }" );
	cmd( "bind .f.t.t <Control-d> { set choice 5; break }" );
	cmd( "bind .f.t.t <Control-b> { set choice 33; break }; bind .f.t.t <Control-B> { set choice 33; break }" );
	cmd( "bind .f.t.t <Control-minus> { .m.edit invoke 23 }" );
	cmd( "bind .f.t.t <Control-plus> { .m.edit invoke 22 }" );
	cmd( "bind .f.t.t <Control-parenleft> { .f.t.t insert insert \\{ }" );
	cmd( "bind .f.t.t <Control-parenright> { .f.t.t insert insert \\} }" );
	cmd( "bind .f.t.t <Control-greater> { set choice 42 }; bind .f.t.t <Control-period> { set choice 42 }" );
	cmd( "bind .f.t.t <Control-less> { set choice 43 }; bind .f.t.t <Control-comma> { set choice 43 }" );
	cmd( "bind .f.t.t <Alt-q> { .m postcascade 0 }; bind .f.t.t <Alt-Q> { .m postcascade 0 }" );

	// redefine the default Tk bindings adding "break" to avoid further event processing
	cmd( "bind .f.t.t <Control-z> { .m.edit invoke 0; break }; bind .f.t.t <Control-Z> { event generate .f.t.t <Control-z> }" );
	cmd( "bind .f.t.t <Control-y> { .m.edit invoke 1; break }; bind .f.t.t <Control-Y> { event generate .f.t.t <Control-y> }" );
	cmd( "bind .f.t.t <Control-x> { .m.edit invoke 3; break }; bind .f.t.t <Control-X> { event generate .f.t.t <Control-x> }" );
	cmd( "bind .f.t.t <Control-c> { .m.edit invoke 4; break }" );
	cmd( "bind .f.t.t <Control-v> { .m.edit invoke 5; break }" );
	cmd( "bind .f.t.t <Delete> { .m.edit invoke 6; break }" );
	cmd( "bind .f.t.t <BackSpace> { \
			sav_cur_ini; \
			if [ string equal [ .f.t.t tag ranges sel ] \"\" ] { \
				if { ! [ string equal [ .f.t.t index insert ] 1.0 ] } { \
					.f.t.t delete insert-1c \
				} \
			} { \
				.f.t.t delete sel.first sel.last \
			}; \
			if [ .f.t.t edit modified ] { \
				sav_cur_end; \
				set choice 23 \
			}; \
			upd_color; \
			break \
		}" );
	cmd( "if { ! [ string equal $CurPlatform windows ] } { bind .f.t.t <Control-Insert> { .m.edit invoke 4; break } }" );
	cmd( "if { ! [ string equal $CurPlatform windows ] } { bind .f.t.t <Shift-Insert> { .m.edit invoke 5; break } }" );

	cmd( "bind .f.t.t <KeyPress-Return> {+set choice 16}" );
	cmd( "bind .f.t.t <KeyRelease-space> {+.f.t.t edit separator}" );

	// context menu
	cmd( "ttk::menu .v -tearoff 0" );

	cmd( "bind .f.t.t <3> { \
			%%W mark set insert [ .f.t.t index @%%x,%%y ]; \
			set vmenuInsert [ .f.t.t index insert ]; \
			tk_popup .v %%X %%Y \
		}" );
	cmd( "bind .f.t.t <2> { \
			%%W mark set insert [ .f.t.t index @%%x,%%y ]; \
			set vmenuInsert [ .f.t.t index insert ]; \
			tk_popup .v %%X %%Y \
		}" );
	cmd( ".v add command -label \"Copy\" -command { .m.edit invoke 4 }" );
	cmd( ".v add command -label \"Cut\" -command { .m.edit invoke 3 }" );
	cmd( ".v add command -label \"Paste\" -command { .m.edit invoke 5 }" );

	cmd( ".v add separator" );
	cmd( ".v add cascade -label \"LSD Macro\" -menu .v.i" );
	cmd( ".v add command -label \"Indent Selection\" -command { set choice 42 }" );
	cmd( ".v add command -label \"De-indent Selection\" -command { set choice 43 }" );
	cmd( ".v add command -label \"Place Break & Run [ string toupper $DbgExe ]\" -command { set choice 58 }" );

	cmd( ".v add separator" );
	cmd( ".v add command -label \"Find...\" -command { set choice 11 }" );
	cmd( ".v add command -label \"Match \\\{ \\}\" -command { set choice 17 }" );
	cmd( ".v add command -label \"Match \\\( \\)\" -command { set choice 32 }" );

	cmd( "ttk::menu .v.i -tearoff 0" );
	cmd( ".v.i add command -label \"EQUATION\" -command { set choice 25 } -accelerator Ctrl+E" );
	cmd( ".v.i add command -label \"V(...)\" -command { set choice 26 } -accelerator Ctrl+V" );
	cmd( ".v.i add command -label \"CYCLE(...)\" -command { set choice 27 } -accelerator Ctrl+C" );
	cmd( ".v.i add command -label \"SUM(...)\" -command { set choice 56 } -accelerator Ctrl+U" );
	cmd( ".v.i add command -label \"INCR(...)\" -command { set choice 40 } -accelerator Ctrl+I" );
	cmd( ".v.i add command -label \"MULT(...)\" -command { set choice 45 } -accelerator Ctrl+M" );
	cmd( ".v.i add command -label \"SEARCH(...)\" -command { set choice 55 } -accelerator Ctrl+A" );
	cmd( ".v.i add command -label \"SEARCH_CND(...)\" -command { set choice 30 } -accelerator Ctrl+S" );
	cmd( ".v.i add command -label \"SORT(...)\" -command { set choice 31 } -accelerator Ctrl+T" );
	cmd( ".v.i add command -label \"RNDDRAW(...)\" -command { set choice 54 } -accelerator Ctrl+N" );
	cmd( ".v.i add command -label \"WRITE(...)\" -command { set choice 29 } -accelerator Ctrl+W" );
	cmd( ".v.i add command -label \"ADDOBJ(...)\" -command { set choice 52 } -accelerator Ctrl+O" );
	cmd( ".v.i add command -label \"DELETE(...)\" -command { set choice 53 } -accelerator Ctrl+D" );
	cmd( ".v.i add command -label \"Network macros\" -command { set choice 72 } -accelerator Ctrl+K" );
	cmd( ".v.i add command -label \"Math functions\" -command { set choice 51 } -accelerator Ctrl+H" );

	cmd( "bind .f.t.t <Control-E> { set choice 25 }" );
	cmd( "bind .f.t.t <Control-V> { set choice 26; break }" );
	cmd( "bind .f.t.t <Control-U> { set choice 56 }" );
	cmd( "bind .f.t.t <Control-A> { set choice 55 }" );
	cmd( "bind .f.t.t <Control-C> { set choice 27; break }" );
	cmd( "bind .f.t.t <Control-i> { set choice 28; break }" );
	cmd( "bind .f.t.t <Control-T> { set choice 31 }" );
	cmd( "bind .f.t.t <Control-I> { set choice 40 }" );
	cmd( "bind .f.t.t <Control-M> { set choice 45 }" );
	cmd( "bind .f.t.t <Control-S> { set choice 30 }" );
	cmd( "bind .f.t.t <Control-W> { set choice 29 }" );
	cmd( "bind .f.t.t <Control-O> { set choice 52 }" );
	cmd( "bind .f.t.t <Control-D> { set choice 53 }" );
	cmd( "bind .f.t.t <Control-N> { set choice 54 }" );
	cmd( "bind .f.t.t <Control-H> { set choice 51 }" );
	cmd( "bind .f.t.t <Control-K> { set choice 72 }" );

	cmd( "bind .f.t.t <F1> { set choice 34 }" );

	cmd( "setDoubleclickBinding .f.t.t" );
	cmd( "set tcl_wordchars {\\w}" );
	cmd( "set tcl_nonwordchars {\\W}" );
	cmd( "set textsearch \"\"" );
	cmd( "set datasel \"\"" );

	cmd( "pack .f -expand yes -fill both" );
	cmd( "pack .f.t -expand yes -fill both" );
	cmd( "pack .f.t.vs -side right -fill y" );
	cmd( "pack .f.t.t -expand yes -fill both" );
	cmd( "pack .f.t.hs -fill x" );

	cmd( "set filename \"(no name)\"" );
	cmd( "set dirname \"[ pwd ]\"" );
	cmd( "set modelDir \"[ pwd ]\"" );
	cmd( "set groupdir \"[ pwd ]\"" );

	cmd( ".f.t.t tag remove sel 1.0 end" );
	cmd( ".f.t.t mark set insert 1.0" );
	cmd( "set before [ .f.t.t get 1.0 end ]" );

	if ( argn > 1 )
	{
		cmd( "if [ file exists \"$filetoload\" ] { set choice 0 } { set choice -2 }" );
		if ( choice == 0 )
		{
			cmd( "set file [ open \"$filetoload\" ]" );
			cmd( ".f.t.t insert end [ read $file ]" );
			cmd( ".f.t.t edit reset" );
			cmd( "close $file" );
			cmd( ".f.t.t mark set insert 1.0" );
			cmd( "set filename \"[ file tail \"$filetoload\" ]\"" );
			cmd( "set dirname [ file dirname \"$filetoload\" ]" );
			cmd( "set before [ .f.t.t get 1.0 end ]" );
			cmd( ".f.hea.info.file.dat conf -text \"$filename\"" );

			sourcefile = recolor_all = is_source_file( ( char * ) Tcl_GetVar( inter, "filetoload", 0 ) );
		}
		else
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File missing\" -detail \"File '$filetoload' not found.\"" );
	}
	else
		choice = 33; 				// open model browser

	cmd( "settop . no { set choice 1 } no yes" );
	cmd( "focus .f.t.t" );
	cmd( "set keepfocus 0" );

	loop:

	cmd( "if { ! $keepfocus } { focus .f.t.t; update } { set keepfocus 0 }" );

	// update file save status in titlebar and cursor position in info bar
	cmd( "update_title_bar" );

	// start recolor if needed
	if ( recolor_all )				// all text?
	{
		cmd( "sav_cur_ini; sav_cur_end; upd_cursor" );	// save data for recolor
		color( shigh, 0, 0 );		// set color types (all text)
	}
	else
		if ( recolor )				// just around cursor?
			cmd( "upd_color 1" );

	recolor_all = recolor = false;

	// main command loop
	while ( ! choice )
	{
		try
		{
			Tcl_DoOneEvent( 0 );
		}
		catch ( bad_alloc& ) 		// raise memory problems
		{
			throw;
		}
		catch ( ... )				// ignore the rest
		{
			goto loop;
		}
	}

	// update file save status in titlebar
	cmd( "update_title_bar" );

	// verify if saving before command is necessary
	if ( choice == 1 || choice == 2 || choice == 3 || choice == 5 || choice == 6 || choice == 8 || choice == 13 || choice == 14 || choice == 15 || choice == 33 || choice == 39 || choice == 41 || choice == 58 || choice == 71 )
		if ( ! discard_change( ) )
			goto loop;

	// save data for recolor if there is not a pending recolor
	if ( choice != 23 )
		cmd( "sav_cur_ini" );

	// start evaluating the executed command

	// exit LMM
	if ( choice == 1 )
	{
		update_lmm_options( true );	// update window position, if required
		return 0;
	}

	// compile the model, invoking make
	// Run the model
	if ( choice == 2 || choice == 6 )
	{
		cmd( "if { \"[ check_sys_opt ]\" != \"\" } { if { [ ttk::messageBox -parent . -icon warning -title Warning -type yesno -default no -message \"Invalid system options detected\" -detail \"The current LSD configuration is invalid for your platform. To fix it, please use menu option 'Model>System Options', press the 'Default' button, and then 'OK'.\n\nDo you want to proceed anyway?\" ] == no } { set choice 0 } }" );

		if ( choice != 0 )
		{
			compile_run( choice == 2 ? true : false );
			choice = 0;
		}

		goto loop;
	}

	/* Insert in the text window the makefile of the selected model */
	if ( choice == 3 )
	{
		cmd( ".f.t.t delete 0.0 end" );
		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );

		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
			choice = 0;
			goto loop;
		}

		cmd( "cd \"$modelDir\"" );
		make_makefile( );
		cmd( "cd \"$RootLsd\"" );
		cmd( "if { [ file exists \"$modelDir/makefile\" ] } { set choice 1 } { set choice 0 }" );
		if ( choice == 1 )
		{
			cmd( "set file [ open \"$modelDir/makefile\" r ]" );
			cmd( ".f.t.t insert end [ read -nonewline $file ]" );
			cmd( ".f.t.t edit reset" );
			cmd( "close $file" );
		}

		sourcefile = 0;

		cmd( "set before [ .f.t.t get 1.0 end ]" );
		cmd( "set filename makefile" );
		cmd( ".f.t.t mark set insert 1.0" );
		cmd( ".f.hea.info.file.dat conf -text \"makefile\"" );
		cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Makefile should not be changed\" -detail \"Direct changes to the 'makefile' will not affect compilation issued through LMM. Please check 'Model Options' and 'System Options' in menu 'Model' to change compilation options.\"" );

		choice = 0;
		goto loop;
	}

	/* Save the file currently shown */
	if ( choice == 4 )
	{
		cmd( "set curfilename [tk_getSaveFile -parent . -title \"Save File\" -initialfile $filename -initialdir $dirname]" );
		s = ( char * ) Tcl_GetVar( inter, "curfilename", 0 );

		if ( s != NULL && strcmp( s, "" ) )
		{
			cmd( "if [ file exist \"$dirname/$filename\" ] { file copy -force \"$dirname/$filename\" \"$dirname/[file rootname \"$filename\"].bak\" }" );
			cmd( "set file [ open \"$curfilename\" w ]" );
			cmd( "puts -nonewline $file [ .f.t.t get 0.0 end ]" );
			cmd( "close $file" );
			cmd( "set before [ .f.t.t get 0.0 end ]" );
			cmd( "set dirname [ file dirname \"$curfilename\" ]" );
			cmd( "set filename [ file tail \"$curfilename\" ]" );
			cmd( ".f.hea.info.file.dat conf -text \"$filename\"" );
		}

		choice = 0;
		goto loop;
	}

	/* Load the description file */
	if ( choice == 5 || choice == 50 )
	{
		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );

		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
			choice = 0;
			goto loop;
		}

		cmd( "set dirname \"$modelDir\"" );
		cmd( "set filename $DESCRIPTION" );

		cmd( ".f.t.t delete 0.0 end" );
		cmd( "set choice 0; if { [ file exists \"$dirname/$filename\" ] } { set choice 1; if { [ file size \"$dirname/$filename\" ] <= 2 } { set choice 0; file delete \"$dirname/$filename\" } }" );
		if ( choice == 1 )
		{
			cmd( "set file [ open \"$dirname/$DESCRIPTION\" r ]" );
			cmd( ".f.t.t insert end [ read -nonewline $file ]" );
			cmd( "close $file" );
			cmd( "set before [ .f.t.t get 1.0 end ]" );
		}
		else		// if no description, ask if the user wants to create it or not
		{
			cmd( "set answer [ ttk::messageBox -parent . -type yesno -default no -icon question -title \"Create Description\" -message \"Create a description file?\" -detail \"There is no valid description file ('$DESCRIPTION') set for the model\n\nDo you want to create a description file now?\n\nPress 'No' to just show the equations file.\" ]" );
			cmd( " if [ string equal $answer yes ] { set choice 1 } { set choice 2 } " );
			if ( choice == 2 )
			{
				cmd( " set filename \"\" " );
				cmd( "set before [ .f.t.t get 0.0 end ]" );
				choice = 8;		// load equations file
				goto loop;
			}
			cmd( ".f.t.t insert end \"Model $modelName (ver. $modelVersion)\n\n(Enter the Model description text here)\n\n(PRESS CTRL+E TO EDIT EQUATIONS)\n\"" );
		}

		sourcefile = 0;
		
		cmd( ".f.t.t edit reset" );
		cmd( ".f.t.t mark set insert 1.0" );
		cmd( ".f.hea.info.file.dat conf -text \"$filename\"" );

		cmd( "unset -nocomplain ud udi rd rdi" );
		cmd( "lappend ud [ .f.t.t get 0.0 end ]" );
		cmd( "lappend udi [ .f.t.t index insert ]" );

		if ( choice == 50 )
			choice = 46; 			// go to create makefile, after the model selection
		else
			choice = 0;

		goto loop;
	}

	/* Show compilation result */
	if ( choice == 7 )
	{
		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );
		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
			choice = 0;
			goto loop;
		}

		create_compresult_window( );
		choice = 0;
		goto loop;
	}

	/* Insert in the text window the main equation file */
	if ( choice == 8 )
	{
		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );

		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
			choice = 0;
			goto loop;
		}

		s = get_fun_name( str );
		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid equation file name\" -detail \"Check the 'FUN' field in menu 'Model', 'Model Options' for a valid equation file name.\"" );
			choice = 0;
			goto loop;
		}

		cmd( "set oldfile \"$filename\"" );
		cmd( "set olddir \"$dirname\"" );
		cmd( "set filename \"%s\"", s );
		cmd( "set dirname \"$modelDir\"" );
		cmd( "if [ file exist \"$dirname/$filename\" ] { \
				set file [ open \"$dirname/$filename\" r ]; \
				.f.t.t delete 1.0 end; \
				.f.t.t insert end [ read -nonewline $file ]; \
				close $file; \
				.f.t.t edit reset; \
				.f.t.t tag remove sel 1.0 end; \
				set choice 1 \
			} { \
				set filename \"$oldfile\"; \
				set dirname \"$olddir\"; \
				ttk::messageBox -parent . -title Error -icon error -type ok -message \"Equation file not found\" -detail \"If equation file has been renamed, update the 'FUN' field in menu 'Model', 'Model Options'.\"; \
				set choice 0 \
			}" );
		cmd( "unset -nocomplain oldfile olddir" );

		if ( ! choice )
			goto loop;

		// handle the opening of files from the compilation error window
		cmd( "if { [ info exists errfil ] && [ string equal \"$errfil\" \"[ file normalize \"$modelDir/$filename\" ]\" ] && [ info exists errlin ] && [ string is integer -strict $errlin ] } { \
				.f.t.t tag add sel $errlin.0 $errlin.end; \
				if { [ info exists errcol ] && $errcol != \"\" && [ string is integer -strict $errcol ] } { \
					.f.t.t see $errlin.$errcol; \
					.f.t.t mark set insert $errlin.$errcol \
				} else { \
					.f.t.t see $errlin.0; \
					.f.t.t mark set insert $errlin.0 \
				} \
			} else { \
				.f.t.t mark set insert 1.0 \
			}" );
		cmd( "upd_cursor" );

		cmd( "set before [ .f.t.t get 1.0 end ]" );
		cmd( ".f.hea.info.file.dat conf -text \"$filename\"" );
		cmd( ".f.t.t tag add bc \"1.0\"" );
		cmd( ".f.t.t tag add fc \"1.0\"" );
		cmd( "unset -nocomplain ud udi rd rdi" );
		cmd( "lappend ud [ .f.t.t get 0.0 end ]" );
		cmd( "lappend udi [ .f.t.t index insert ]" );

		sourcefile = 1;
		recolor_all = true;
		choice = 0;
		goto loop;
	}

	/* Find a line in the text */
	if ( choice == 10 )
	{
		cmd( "set line \"\"" );

		cmd( "newtop .search_line \"Go To\" { .search_line.b.cancel invoke }" );

		cmd( "ttk::frame .search_line.l" );
		cmd( "ttk::label .search_line.l.l -text \"Line number\"" );
		cmd( "ttk::entry .search_line.l.e -justify center -width 10" );
		cmd( "pack .search_line.l.l .search_line.l.e" );

		cmd( "pack .search_line.l -padx 5 -pady 5" );

		cmd( "okcancel .search_line b { \
				set line [ .search_line.l.e get ]; \
				if { $line != \"\" && [ string is integer -strict $line ] && $line >= 0 } { \
					.f.t.t tag remove sel 1.0 end; \
					.f.t.t see $line.0; \
					.f.t.t mark set insert $line.0; \
					.f.t.t tag add sel $line.0 $line.end; \
					upd_cursor \
				} { \
					bell \
				}; \
				destroytop .search_line; \
				focus .f.t.t; \
				set keepfocus 0 \
			} { \
				destroytop .search_line; \
				focus .f.t.t; \
				set keepfocus 0 \
			}" );

		cmd( "bind .search_line.l.e <KeyPress-Return> { .search_line.b.ok invoke }" );

		cmd( "showtop .search_line" );
		cmd( "focus .search_line.l.e" );
		cmd( "set keepfocus 1" );

		choice = 0;
		goto loop;
	}

	/* Find a text pattern in the text */
	if ( choice == 11 )
	{
		cmd( "if [ string equal $dirsearch \"-backwards\" ] { set endsearch 1.0 } { set endsearch end }" );
		cmd( "set curcounter $lfindsize" );
		cmd( "if { ! [ string equal [ .f.t.t tag ranges sel ] \"\" ] } { set textsearch [ .f.t.t get sel.first sel.last ] } { set textsearch \"\" }" );

		cmd( "newtop .find \"Find\" { .find.b.cancel invoke }" );

		cmd( "ttk::frame .find.l" );
		cmd( "ttk::label .find.l.l -text \"Text to find\"" );
		cmd( "ttk::entry .find.l.e -width 25 -textvariable textsearch -justify center" );
		cmd( "pack .find.l.l .find.l.e" );

		cmd( "ttk::frame .find.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .find.r.r2 -text \"Up\" -variable dirsearch -value \"-backwards\" -command { set endsearch 1.0 }" );
		cmd( "ttk::radiobutton .find.r.r1 -text \"Down\" -variable dirsearch -value \"-forwards\" -command { set endsearch end }" );
		cmd( "pack .find.r.r2 .find.r.r1 -side left" );

		cmd( "ttk::checkbutton .find.c -text \"Case sensitive\" -variable docase" );

		cmd( "pack .find.l .find.r .find.c -padx 5 -pady 5" );

		cmd( "Xcancel .find b Find { \
				if { $textsearch != \"\" } { \
					incr lfindsize; \
					set curcounter [ expr $lfindsize - 1 ]; \
					lappend lfind \"$textsearch\"; \
					.f.t.t tag remove sel 1.0 end; \
					set cur [ .f.t.t index insert ]; \
					if { $docase } { \
						set cur [ .f.t.t search -count length $dirsearch -- \"$textsearch\" $cur $endsearch ] \
					} else { \
						set cur [ .f.t.t search -count length -nocase $dirsearch -- \"$textsearch\" $cur $endsearch ] \
					}; \
					if { [ string length $cur ] > 0 } { \
						.f.t.t tag add sel $cur \"$cur + $length char\"; \
						if { [ string compare $endsearch end ] != 0 } { \
							set length 0 \
						}; \
						.f.t.t mark set insert \"$cur + $length char\" ; \
						.f.t.t see $cur; \
						upd_cursor; \
						destroytop .find; \
						focus .f.t.t; \
						set keepfocus 0; \
						update \
					} else { \
						.find.l.e selection range 0 end; \
						bell \
					} \
				} { \
					bell \
				} \
			} { \
				destroytop .find; \
				focus .f.t.t; \
				set keepfocus 0 \
			}" );

		cmd( "bind .find.l.e <Up> { \
				if { $lfindsize > 0 && $curcounter > 0 } { \
					incr curcounter -1; \
					set textsearch \"[ lindex $lfind $curcounter ]\"; \
					.find.l.e selection range 0 end \
				} \
			}" );
		cmd( "bind .find.l.e <Down> { \
				if { $lfindsize > 0 && $curcounter < [ expr $lfindsize - 1 ] } { \
					incr curcounter; \
					set textsearch \"[ lindex $lfind $curcounter ]\"; \
					.find.l.e selection range 0 end \
				} \
			}" );
		cmd( "bind .find.l.e <KeyPress-Return> { .find.b.ok invoke }" );
		cmd( "bind .find.r.r1 <KeyPress-Return> { .find.b.ok invoke }" );
		cmd( "bind .find.r.r2 <KeyPress-Return> { .find.b.ok invoke }" );
		cmd( "bind .find.c <KeyPress-Return> { .find.b.ok invoke }" );

		cmd( "showtop .find" );
		cmd( ".find.l.e selection range 0 end" );
		cmd( "focus .find.l.e" );
		cmd( "set keepfocus 1" );

		choice = 0;
		goto loop;
	}

	/* Search again the same pattern in the text */
	if ( choice == 12 || choice == 88 )
	{
		if ( choice == 12 )
			cmd( "set dirsearch \"-forwards\"; set endsearch end" );
		else
			cmd( "set dirsearch \"-backwards\"; set endsearch 1.0" );

		cmd( "if { $textsearch != \"\" } { \
				.f.t.t tag remove sel 1.0 end; \
				set cur [ .f.t.t index insert ]; \
					if { $docase } { \
						set cur [ .f.t.t search -count length $dirsearch -- \"$textsearch\" $cur $endsearch ] \
					} else { \
						set cur [ .f.t.t search -count length -nocase $dirsearch -- \"$textsearch\" $cur $endsearch ] \
					}; \
					if { [ string length $cur ] > 0 } { \
					.f.t.t tag add sel $cur \"$cur + $length char\"; \
					if { [ string compare $endsearch end ] != 0 } { \
						set length 0 \
					}; \
					.f.t.t mark set insert \"$cur + $length char\"; \
					.f.t.t see $cur; \
					upd_cursor; \
					update \
				} else { \
					bell \
				} \
			}" );

		choice = 0;
		goto loop;
	}

	/* Find and replace a text pattern in the text */
	if ( choice == 21 )
	{
		cmd( "if [ string equal $dirsearch \"-backwards\" ] { set endsearch 1.0 } { set endsearch end }" );
		cmd( "set curcounter $lfindsize" );
		cmd( "set cur \"\"" );
		cmd( "if { ! [ string equal [ .f.t.t tag ranges sel ] \"\" ] } { set textsearch [ .f.t.t get sel.first sel.last ] } { set textsearch \"\" }" );

		cmd( "newtop .l \"Replace\" { .l.b2.cancel invoke }" );

		cmd( "ttk::frame .l.l" );
		cmd( "ttk::label .l.l.l -text \"Text to find\"" );
		cmd( "ttk::entry .l.l.e -width 25 -textvariable textsearch -justify center" );
		cmd( "pack .l.l.l .l.l.e" );

		cmd( "ttk::frame .l.p" );
		cmd( "ttk::label .l.p.l -text \"Text to replace\"" );
		cmd( "ttk::entry .l.p.e -width 25 -textvariable textrepl -justify center" );
		cmd( "pack .l.p.l .l.p.e" );

		cmd( "ttk::frame .l.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .l.r.r2 -text \"Up\" -variable dirsearch -value \"-backwards\" -command { set endsearch 1.0 }" );
		cmd( "ttk::radiobutton .l.r.r1 -text \"Down\" -variable dirsearch -value \"-forwards\" -command { set endsearch end }" );
		cmd( "pack .l.r.r2 .l.r.r1 -side left" );

		cmd( "ttk::checkbutton .l.c -text \"Case sensitive\" -variable docase" );

		cmd( "pack .l.l .l.p .l.r .l.c -padx 5 -pady 5" );

		cmd( "ttk::frame .l.pad" );
		cmd( "pack .l.pad -pady 5" );
		
		cmd( "ttk::frame .l.b1" );
		cmd( "ttk::button .l.b1.repl -width $butWid -state disabled -text Replace -command { \
				if { [ string length $cur ] > 0 } { \
					.f.t.t delete $cur \"$cur + $length char\"; \
					.f.t.t insert $cur \"$textrepl\"; \
					if { [ string compare $endsearch end ] != 0 } { \
						.f.t.t mark set insert $cur; \
						upd_cursor \
					}; \
					.l.b2.ok invoke \
				} \
			}" );
		cmd( "ttk::button .l.b1.all -width $butWid -state disabled -text \"Repl. All\" -command { set choice 4 }" );
		cmd( "pack .l.b1.repl .l.b1.all -padx $butSpc -side left" );

		cmd( "pack .l.b1 -padx $butPad -anchor e" );

		cmd( "Xcancel .l b2 Find { \
				if { $textsearch != \"\" } { \
					incr lfindsize; \
					set curcounter [ expr $lfindsize - 1 ]; \
					lappend lfind \"$textsearch\"; \
					.f.t.t tag remove found 1.0 end; \
					.f.t.t tag remove sel 1.0 end; \
					set cur [ .f.t.t index insert ]; \
					if { $docase } { \
						set cur [ .f.t.t search -count length $dirsearch -- \"$textsearch\" $cur $endsearch ] \
					} else { \
						set cur [ .f.t.t search -count length -nocase $dirsearch -- \"$textsearch\" $cur $endsearch ] \
					}; \
					if { [ string length $cur ] > 0 } { \
						.f.t.t tag add found $cur \"$cur + $length char\"; \
						if { [ string compare $endsearch end ] == 0 } { \
							.f.t.t mark set insert \"$cur + $length char\" \
						} else { \
							.f.t.t mark set insert $cur \
						}; \
						.f.t.t see $cur; \
						upd_cursor; \
						update; \
						.l.b1.repl conf -state normal; \
						.l.b1.all conf -state normal \
					} else { \
						.l.b1.all conf -state disabled; \
						.l.b1.repl conf -state disabled \
					} \
				} { \
					bell \
				} \
			} { \
				set choice 5 \
			}" );

		cmd( "bind .l.l.e <Up> { \
				if { $lfindsize > 0 && $curcounter > 0 } { \
					incr curcounter -1; \
					set textsearch \"[ lindex $lfind $curcounter ]\"; \
					.l.l.e selection range 0 end \
				} \
			}" );
		cmd( "bind .l.l.e <Down> { \
				if { $lfindsize > 0 && $curcounter < [ expr $lfindsize - 1 ] } { \
					incr curcounter; \
					set textsearch \"[ lindex $lfind $curcounter ]\"; \
					.l.l.e selection range 0 end \
				} \
			}" );
		cmd( "bind .l.l.e <KeyPress-Return> { .l.b2.ok invoke }" );
		cmd( "bind .l.p.e <KeyPress-Return> { .l.b2.ok invoke }" );
		cmd( "bind .l.r.r1 <KeyPress-Return> { .l.b2.ok invoke }" );
		cmd( "bind .l.r.r2 <KeyPress-Return> { .l.b2.ok invoke }" );
		cmd( "bind .l.c <KeyPress-Return> { .l.b2.ok invoke }" );
		cmd( "bind .l.b1.repl <KeyPress-Return> { .l.b1.repl invoke }" );
		cmd( "bind .l.b1.all <KeyPress-Return> { .l.b1.all invoke }" );

		cmd( "showtop .l" );
		cmd( ".l.l.e selection range 0 end" );
		cmd( ".f.t.t tag conf found -background $colorsTheme(sbg) -foreground $colorsTheme(sfg)" );
		cmd( "focus .l.l.e" );

		choice = 0;

		here:
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		if ( choice == 3 )
		{
			choice = 0;
			goto here;
		}

		while ( choice == 4 )
		{
			cmd( "if { [ string length $cur ] > 0 } { \
				.f.t.t delete $cur \"$cur + $length char\"; \
				.f.t.t see $cur; \
				.f.t.t insert $cur \"$textrepl\"; \
				if { [ string compare $endsearch end ] == 0 } { \
					set cur [ .f.t.t index \"$cur+$length char\" ] \
				} \
			} { \
				set choice 0 \
			}" );

			if ( choice != 0 )
				cmd( "if { $docase } { \
							set cur [ .f.t.t search -count length $dirsearch -- \"$textsearch\" $cur $endsearch ] \
						} else { \
							set cur [ .f.t.t search -count length -nocase $dirsearch -- \"$textsearch\" $cur $endsearch ] \
						}" );

			cmd( "focustop .l" );
			Tcl_DoOneEvent( 0 );
			goto here;
		}

		cmd( "destroytop .l" );
		cmd( ".f.t.t tag remove found 1.0 end" );

		recolor_all = true;
		choice = 0;
		goto loop;
	}

	// Run the model in the gdb debugger
	if ( choice == 13 || choice == 58 )
	{
		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );

		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
			choice = 0;
			goto loop;
		}

		cmd( "cd \"$modelDir\"" );

		if ( choice == 58 )
		{
			cmd( "scan $vmenuInsert %%d.%%d line col" );
			cmd( "if [ string equal -nocase $DbgExe lldb ] { set breakExt lldb; set breakTxt \"breakpoint set -f $dirname/$filename -l$line\nrun\n\" } { set breakExt gdb; set breakTxt \"break $dirname/$filename:$line\nrun\n\" }" );
			cmd( "catch { set f [ open break.$breakExt w ]; puts $f $breakTxt; close $f }" );

			cmd( "if [ string equal -nocase $DbgExe lldb ] { set cmdbreak \"-sbreak.lldb\" } { set cmdbreak \"--command=break.gdb\" }" );
		}
		else
			cmd( "if [ string equal -nocase $DbgExe gdb ] { set cmdbreak \"--args\" } { set cmdbreak \"\" }" );

		make_makefile( );
		cmd( "set fapp [ file nativename \"$modelDir/makefile\" ]" );
		s = ( char * ) Tcl_GetVar( inter, "fapp", 0 );
		f = fopen( s, "r" );
		if ( f == NULL )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Makefile not created\" -detail \"Please check 'Model Options' and 'System Options' in menu 'Model'.\"" );
			goto end_gdb;
		}
		fscanf( f, "%999s", str );
		while ( strncmp( str, "TARGET=", 7 ) && fscanf( f, "%999s", str ) != EOF );
		if ( strncmp(str, "TARGET=", 7) != 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Makefile corrupted\" -detail \"Please check 'Model Options' and 'System Options' in menu 'Model'.\"" );
			goto end_gdb;
		}

		strcpy( str1, str + 7 );

		switch( platform )
		{
			case LINUX:
				sprintf( msg, "catch { exec $sysTerm -e $DbgExe $cmdbreak %s & } result", str1 );
				break;

			case MAC:
				cmd( "if [ string equal $cmdbreak \"--args\" ] { set cmdbreak \"\" }" );
				sprintf( msg, "catch { exec osascript -e \"tell application \\\"$sysTerm\\\" to do script \\\"cd $dirname; clear; $DbgExe $cmdbreak -f %s.app/Contents/MacOS/%s\\\"\" & } result", str1, str1 );
				break;

			case WINDOWS:
				strcat( str1, ".exe" );
				sprintf( msg, "catch { exec $sysTerm /c $DbgExe $cmdbreak %s & } result", str1 );
				break;

			default:
				goto end_gdb;
		}

		// check if executable file is older than model file
		s = get_fun_name( str );
		if ( s == NULL || ! strcmp( s, "" ) )
			goto end_gdb;
		strncpy( str, s, 999 );
		s = ( char * ) Tcl_GetVar( inter, "modelDir", 0 );
		if ( s != NULL && strcmp( s, "" ) )
		{
			sprintf( str2, "%s/%s", s, str );
			
			if ( platform == MAC )
				sprintf( str, "%s/%s.app/Contents/MacOS/%s", s, str1, str1 );
			else
				sprintf( str, "%s/%s", s, str1 );
		}

		// get OS info for files
		struct stat stExe, stMod;
		if ( stat( str, &stExe ) == 0 && stat( str2, &stMod ) == 0 )
		{
			if ( difftime( stExe.st_mtime, stMod.st_mtime ) < 0 )
			{
				cmd( "set answer [ ttk::messageBox -parent . -title Warning -icon warning -type okcancel -default cancel -message \"Old executable file\" -detail \"The existing executable file is older than the last version of the model.\n\nPress 'OK' to continue anyway or 'Cancel' to return to LMM. Please recompile the model to avoid this message.\" ]; if [ string equal $answer ok ] { set choice 1 } { set choice 2 }" );
				if ( choice == 2 )
					goto end_gdb;
			}
		}
		else
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Executable not found\" -detail \"Compile the model before running it in the [ string toupper $DbgExe ] debugger.\"" );
			goto end_gdb;
		}

		cmd( msg );					// if all ok, run debug command

		end_gdb:
		cmd( "cd \"$RootLsd\"" );
		choice = 0;
		goto loop;
	}

	/* create a new model/group */
	if ( choice == 14 )
	{
		cmd( "destroytop .mm" );	// close compilation results, if open

		// prevent creating new groups in LSD directory
		cmd( "if { [ string equal $groupdir [ pwd ] ] && [ file exists \"$groupdir/$LsdNew/$GROUP_INFO\" ] } \
				{	set answer [ ttk::messageBox -parent . -type okcancel -title Warning \
					-icon warning -default ok -message \"Invalid parent group\" \
					-detail \"Cannot create group/model in the Root group. Press 'OK' to change to the '$LsdNew' group before proceeding.\" ]; \
					if [ string equal $answer ok ] { \
						set groupdir \"$groupdir/$LsdNew\"; \
						set f [ open \"$groupdir/$GROUP_INFO\" r ]; \
						set modelGroup \"[ gets $f ]\"; \
						close $f; \
						.f.hea.info.grp.dat conf -text \"$modelGroup\"; \
						set choice 1 \
					} else { \
						set choice 0 \
					} \
				}" );
		if ( choice == 0 )
			goto loop;

		cmd( "set temp 1" );

		cmd( "newtop .a \"New Model\" { set choice 2 }" );

		cmd( "ttk::frame .a.tit" );
		cmd( "ttk::label .a.tit.l -text \"Current group:\"" );
		cmd( "ttk::label .a.tit.n -style hl.TLabel -text \"$modelGroup\"" );
		cmd( "pack .a.tit.l .a.tit.n" );

		cmd( "ttk::frame .a.f -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .a.f.r1 -variable temp -value 1 -text \"Create a new model in the current group\"" );
		cmd( "ttk::radiobutton .a.f.r2 -variable temp -value 2 -text \"Create a new model in a new group\"" );
		cmd( "pack .a.f.r1 .a.f.r2 -anchor w" );

		cmd( "pack .a.tit .a.f -padx 5 -pady 5" );

		cmd( "okcancel .a b { set choice 1 } { set choice 2 }" );

		cmd( "bind .a.f <KeyPress-Return> { .a.b.ok invoke }" );
		cmd( "bind .a <Up> { .a.f.r1 invoke }" );
		cmd( "bind .a <Down> { .a.f.r2 invoke }" );

		cmd( "showtop .a" );
		cmd( "mousewarpto .a.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set choice $temp" );
		if ( choice == 2 )
		{
			cmd( "set mname \"New group\"" );
			cmd( "set mdir \"newgroup\"" );

			cmd( "newtop .a \"New Group\" { set choice 2 }" );

			cmd( "ttk::frame .a.tit" );
			cmd( "ttk::label .a.tit.l -text \"Current group:\"" );
			cmd( "ttk::label .a.tit.n -style hl.TLabel -text \"$modelGroup\"" );
			cmd( "pack .a.tit.l .a.tit.n" );

			cmd( "ttk::frame .a.mname" );
			cmd( "ttk::label .a.mname.l -text \"New group name\"" );
			cmd( "ttk::entry .a.mname.e -width 25 -textvariable mname -justify center" );
			cmd( "pack .a.mname.l .a.mname.e" );

			cmd( "ttk::frame .a.mdir" );
			cmd( "ttk::label .a.mdir.l -text \"New (non-existing) subdirectory name\"" );
			cmd( "ttk::entry .a.mdir.e -width 35 -textvariable mdir -justify center" );
			cmd( "pack .a.mdir.l .a.mdir.e" );

			cmd( "ttk::frame .a.tdes" );
			cmd( "ttk::label .a.tdes.l -text \"Group description\"" );
			cmd( "set a [ list \"$fonttype\" $small_character ]" );
			cmd( "ttk::text .a.tdes.e -width 60 -height 15 -dark $darkTheme -style smallFixed.TText" );
			cmd( "pack .a.tdes.l .a.tdes.e" );

			cmd( "pack .a.tit .a.mname .a.mdir .a.tdes -padx 5 -pady 5" );

			cmd( "okcancel .a b { set choice 1 } { set choice 2 }" );
			cmd( "bind .a.mname.e <Return> { focus .a.mdir.e; .a.mdir.e selection range 0 end }" );
			cmd( "bind .a.mdir.e <Return> { focus .a.tdes.e }" );

			cmd( "showtop .a" );
			cmd( ".a.mname.e selection range 0 end" );
			cmd( "focus .a.mname.e" );

			here_newgroup:

			choice = 0;

			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "if { [ string length $mdir ] == 0 || [ string length $mname ] == 0 } { set choice 2 }" );

			if ( choice == 2 )
			{
				cmd( "destroytop .a" );
				choice = 0;
				goto loop;
			}

			cmd( "if { [ llength [ split $mdir ] ] > 1 } { set choice -1 }" );
			if ( choice == -1 )
			{
				cmd( "ttk::messageBox -parent .a -type ok -title Error -icon error -message \"Space in path\" -detail \"Directory name must not contain spaces, please try a new name.\"" );
				cmd( "focus .a.mdir.e" );
				cmd( ".a.mdir.e selection range 0 end" );
				goto here_newgroup;
			}

			// control for existing directory
			cmd( "if [ file exists \"$groupdir/$mdir\" ] { ttk::messageBox -parent .a -type ok -title Error -icon error -message \"Cannot create directory\" -detail \"$groupdir/$mdir\\n\\nPossibly there is already such a directory, please try a new directory.\"; set choice 3 }" );
			if ( choice == 3 )
			{
				cmd( "focus .a.mdir.e" );
				cmd( ".a.mdir.e selection range 0 end" );
				goto here_newgroup;
			}

			cmd( "file mkdir \"$groupdir/$mdir\"" );
			cmd( "cd \"$groupdir/$mdir\"" );
			cmd( "set groupdir \"$groupdir/$mdir\"" );
			cmd( "set f [ open $GROUP_INFO w ]; puts -nonewline $f \"$mname\"; close $f" );
			cmd( "set f [ open $DESCRIPTION w ]; puts -nonewline $f \"[ .a.tdes.e get 0.0 end ]\"; close $f" );
			cmd( "set modelGroup \"$mname\"" );

			cmd( "destroytop .a" );
			//end of creation of a new group
		}
		else
			cmd( "cd \"$groupdir\"" );	// if no group is created, move in the current group

		// create a new model
		cmd( "set mname \"New model\"" );
		cmd( "set mver \"1.0\"" );
		cmd( "set mdir \"newmodel\"" );


		cmd( "newtop .a \"New Model\" { set choice 2 }" );

		cmd( "ttk::frame .a.tit" );
		cmd( "ttk::label .a.tit.l -text \"Current group:\"" );
		cmd( "ttk::label .a.tit.n -style hl.TLabel -text \"$modelGroup\"" );
		cmd( "pack .a.tit.l .a.tit.n" );

		cmd( "ttk::frame .a.mname" );
		cmd( "ttk::label .a.mname.l -text \"New model name\"" );
		cmd( "ttk::entry .a.mname.e -width 25 -textvariable mname -justify center" );
		cmd( "pack .a.mname.l .a.mname.e" );

		cmd( "ttk::frame .a.mver" );
		cmd( "ttk::label .a.mver.l -text \"Version\"" );
		cmd( "ttk::entry .a.mver.e -width 10 -textvariable mver -justify center" );
		cmd( "pack .a.mver.l .a.mver.e" );

		cmd( "ttk::frame .a.mdir" );
		cmd( "ttk::label .a.mdir.l -text \"New (non-existing) subdirectory name\"" );
		cmd( "ttk::entry .a.mdir.e -width 35 -textvariable mdir -justify center" );
		cmd( "pack .a.mdir.l .a.mdir.e" );

		cmd( "pack .a.tit .a.mname .a.mver .a.mdir -padx 5 -pady 5" );

		cmd( "okcancel .a b { set choice 1 } { set choice 2 }" );
		cmd( "bind .a.mname.e <Return> { focus .a.mver.e; .a.mver.e selection range 0 end }" );
		cmd( "bind .a.mver.e <Return> { focus .a.mdir.e; .a.mdir.e selection range 0 end }" );
		cmd( "bind .a.mdir.e <Return> { focus .a.b.ok }" );

		cmd( "showtop .a" );
		cmd( ".a.mname.e selection range 0 end" );
		cmd( "focus .a.mname.e" );

		loop_copy_new:

		choice = 0;

		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "if { [ string length $mdir ] == 0 || [ string length $mname ] == 0 } { set choice 2 }" );
		if ( choice == 2 )
		{
			cmd( "destroytop .a" );
			cmd( "set modelName \"\"" );
			cmd( "set modelVersion \"\"" );
			choice = 0;
			goto loop;
		}

		cmd( "if { [ llength [ split $mdir ] ] > 1 } { set choice -1 }" );
		if ( choice == -1 )
		{
			cmd( "ttk::messageBox -parent .a -type ok -title Error -icon error -message \"Space in path\" -detail \"Directory name must not contain spaces, please try a new name.\"" );
		   cmd( "focus .a.mdir.e" );
		   cmd( ".a.mdir.e selection range 0 end" );
		   goto loop_copy_new;
		}

		// control for existing directory
		cmd( "if [ file exists \"$mdir\" ] { ttk::messageBox -parent .a -type ok -title Error -icon error -message \"Cannot create directory\" -detail \"$groupdir/$mdir\\n\\nPossibly there is already such a directory, please try a new directory.\"; set choice 3 }" );
		if ( choice == 3 )
		{
			cmd( "focus .a.mdir.e" );
			cmd( ".a.mdir.e selection range 0 end" );
			goto loop_copy_new;
		}

		// control for an existing model with the same name AND same version
		cmd( "set dir [ glob -nocomplain * ]" );
		cmd( "set num [ llength $dir ]" );
		strcpy(str, " ");

		for ( i = 0; i < num; ++i )
		{
			cmd( "if [ file isdirectory [ lindex $dir %d ] ] { set curdir [ lindex $dir %i ] } { set curdir ___ }", i, i );
			s = ( char * ) Tcl_GetVar( inter, "curdir", 0 );
			strncpy( str, s, 499 );

			// check for invalid directories (LSD managed)
			for ( found = false, j = 0; j < LSD_DIR_NUM; ++j )
				if ( ! strcmp( str, lsd_dir[ j ] ) )
					found = true;

			if ( ! found )
			{
				if ( ! load_model_info( str ) )
					cmd( "set modelName $curdir; set modelVersion \"1.0\"" );

				cmd( "set comp [ string compare $modelName $mname ]" );
				cmd( "set comp1 [ string compare $modelVersion $mver ]" );
				cmd( "if { $comp == 0 && $comp1 == 0 } { set choice 3 }" );
				cmd( "if { $comp == 0 } { set choice 4 }" );
			}
		}

		if ( choice == 3 )
		{
			cmd( "ttk::messageBox -parent .a -type ok -title Error -icon error -message \"Model already exists\" -detail \"Cannot create the new model '$mname' (ver. $mver) because it already exists (directory: $curdir).\"" );
			cmd( ".a.mname.e selection range 0 end" );
			cmd( "focus .a.mname.e" );
			goto loop_copy_new;
		}

		if ( choice == 4 )
		{
			choice = 0;
			cmd( "set answer [ ttk::messageBox -parent .a -type okcancel -title Warning -icon warning -default cancel -message \"Model already exists\" -detail \"A model named '$mname' (ver. $mver) already exists in directory: $curdir.\\n\\nIf you want the new model to inherit the same equations, data etc. of that model you may cancel this operation, and use the 'Save Model As...' command. Or press 'OK' to continue creating a new (empty) model '$mname'.\" ]" );
			s = ( char * ) Tcl_GetVar( inter, "answer", 0 );

			cmd( "if { ! [ string compare $answer ok ] } { set choice 1 } { set choice 0 }" );
			if ( choice == 0 )
			{
				cmd( "destroytop .a" );
				cmd( "set modelName \"\"" );
				cmd( "set modelVersion \"\"" );
				goto loop;
			}
		}

		cmd( "destroytop .a" );

		// create a new empty model
		cmd( "set dirname $groupdir/$mdir" );
		cmd( "set modelDir $groupdir/$mdir" );
		cmd( "set modelName $mname" );
		cmd( "set modelVersion $mver" );
		cmd( "set modelDate \"\"" );
		cmd( ".f.hea.info.mod.dat conf -text \"$modelName\"" );
		cmd( ".f.hea.info.ver.dat conf -text \"$modelVersion\"" );
		cmd( ".f.hea.info.grp.dat conf -text \"$modelGroup\"" );

		cmd( "file mkdir \"$dirname\"" );

		// create the empty equation file
		cmd( "file copy \"$RootLsd/$LsdSrc/fun_base.cpp\" \"$modelDir/fun_$mdir.cpp\"" );

		// create the model options and info files
		check_option_files( );
		update_model_info( );

		cmd( ".m.file entryconf 2 -state normal" );
		cmd( ".m.file entryconf 3 -state normal" );
		cmd( ".m.model entryconf 0 -state normal" );
		cmd( ".m.model entryconf 1 -state normal" );
		cmd( ".m.model entryconf 2 -state normal" );
		cmd( ".m.model entryconf 3 -state normal" );
		cmd( ".m.model entryconf 4 -state normal" );
		cmd( ".m.model entryconf 6 -state normal" );
		cmd( ".m.model entryconf 7 -state normal" );
		cmd( ".m.model entryconf 8 -state normal" );
		cmd( ".m.model entryconf 9 -state normal" );
		cmd( ".m.model entryconf 10 -state normal" );
		cmd( ".m.model entryconf 12 -state normal" );

		cmd( "ttk::messageBox -parent . -type ok -title \"New Model\" -icon info -message \"Model '$modelName' created\" -detail \"Version: $modelVersion\nDirectory: $modelDir\"" );

		cmd( "set before [ .f.t.t get 1.0 end ]" ); //avoid to re-issue a warning for non saved files

		choice = 50;
		goto loop;
	}

	/* open a text file */
	if ( choice == 15 || choice == 71 )
	{
		if ( choice == 15 )
		{
			cmd( "set brr [ tk_getOpenFile -parent . -title \"Load Text File\" -initialdir $dirname ]" );
			cmd( "if { [ string length $brr ] == 0 } { set choice 0 } { set choice 1 }" );
			if ( choice == 0 )
				goto loop;
		}

		cmd( "if [ file exists \"$brr\" ] { set choice 1 } { set choice 0 }" );
		if ( choice == 0 )
			goto loop;

		cmd( ".f.t.t delete 1.0 end" );
		cmd( "set dirname [ file dirname \"$brr\" ]" );
		cmd( "set filename [ file tail \"$brr\" ]" );
		cmd( "set file [ open \"$brr\" r ]" );
		cmd( ".f.t.t insert end [ read -nonewline $file ]" );
		cmd( "close $file" );
		cmd( ".f.t.t edit reset" );
		cmd( ".f.t.t tag remove sel 1.0 end" );

		// handle the opening of files from the compilation error window
		cmd( "if { [ info exists errfil ] && [ string equal \"$errfil\" \"[ file normalize \"$dirname/$filename\" ]\" ] && [ info exists errlin ] && [ string is integer -strict $errlin ] } { \
				.f.t.t tag add sel $errlin.0 $errlin.end; \
				if { [ info exists errcol ] && $errcol != \"\" && [ string is integer -strict $errcol ] } { \
					.f.t.t see $errlin.$errcol; \
					.f.t.t mark set insert $errlin.$errcol \
				} else { \
					.f.t.t see $errlin.0; \
					.f.t.t mark set insert $errlin.0 \
				} \
			} else { \
				.f.t.t mark set insert 1.0 \
			}" );
		cmd( "upd_cursor" );

		cmd( "set before [ .f.t.t get 1.0 end ]" );
		cmd( ".f.hea.info.file.dat conf -text \"$filename\"" );
		
		sourcefile = recolor_all = is_source_file( ( char * ) Tcl_GetVar( inter, "filename", 0 ) );
		
		if ( sourcefile )
		{
			cmd( ".f.t.t tag add bc \"1.0\"" );
			cmd( ".f.t.t tag add fc \"1.0\"" );
		}

		cmd( "unset -nocomplain ud udi rd rdi" );
		cmd( "lappend ud [ .f.t.t get 0.0 end ]" );
		cmd( "lappend udi [ .f.t.t index insert ]" );

		choice = 0;
		goto loop;
	}

	/* insert automatic tabs */
	if ( choice == 16 )
	{
		cmd( "set in [ .f.t.t index insert ]" );
		cmd( "scan $in %%d.%%d line col" );
		cmd( "set line [ expr $line - 1 ]" );
		cmd( "set s [ .f.t.t get $line.0 $line.end ]" );

		s = ( char * ) Tcl_GetVar( inter, "s", 0 );
		for ( i = 0; s[ i ] == ' ' || s[ i ] == '\t'; ++i )
		  str[ i ] = s[ i ];

		if ( i > 0 )
		{
			str[ i ] = '\0';
			cmd( ".f.t.t insert insert \"%s\"", str );
		}

		choice = 0;
		goto loop;
	}

	/* find matching parenthesis */
	if ( choice == 17)
	{
		cmd( "set sym [ .f.t.t get insert ]" );
		cmd( "if { [ string compare $sym \"\\{\"] != 0 } { if { [ string compare $sym \"\\}\" ] != 0 } { set choice 0 } { set num -1; set direction -backwards; set fsign +; set sign \"\"; set terminal \"1.0\" } } { set num 1; set direction -forwards; set fsign -; set sign +; set terminal end }" );

		if ( choice == 0 )
			goto loop;
		cmd( "set cur [ .f.t.t index insert ]" );
		cmd( ".f.t.t tag add sel $cur \"$cur + 1char\"" );
		if ( num > 0 )
			cmd( "set cur [ .f.t.t index \"insert + 1 char\" ]" );

		while ( num != 0 && choice != 0 )
		{
			cmd( "set a [ .f.t.t search $direction \"\\{\" $cur $terminal ]" );
			cmd( "if { $a == \"\" } { set a [ .f.t.t index $terminal ] }" );
			cmd( "set b [ .f.t.t search $direction \"\\}\" $cur $terminal ]" );
			cmd( "if { $b == \"\" } { set b [.f.t.t index $terminal] }" );
			cmd( "if { $a == $b } { set choice 0 }" );
			if ( choice == 0 )
				goto loop;

			if ( num > 0 )
				cmd( "if [ .f.t.t compare $a < $b ] { set num [ expr $num + 1 ]; set cur [ .f.t.t index \"$a+1char\" ] } { set num [ expr $num - 1 ]; set cur [ .f.t.t index \"$b+1char\" ] }" );
			else
				cmd( "if [ .f.t.t compare $a > $b ] { set num [ expr $num + 1 ]; set cur [ .f.t.t index $a ] } { set num [ expr $num - 1 ]; set cur [ .f.t.t index $b ] }" );


		}

		choice = 0;

		cmd( " if { ! [ string compare $sign \"+\" ] } { .f.t.t tag add sel \"$cur - 1char\" $cur ; set num 1 } { .f.t.t tag add sel $cur \"$cur + 1char\"; set num 0 }" );

		cmd( ".f.t.t see $cur" );
		goto loop;
	}

	// Insert a LSD macro, to be used in LSD equations' code
	if ( choice == 28 )
	{
		cmd( "set res 26" );

		cmd( "newtop .a \"Insert LSD macro\" { set choice 2 }" );

		cmd( "ttk::label .a.tit -text \"Available LSD macros\"" );

		cmd( "ttk::frame .a.r -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.r.equ -text \"EQUATION - insert a new LSD equation\" -underline 0 -variable res -value 25" );
		cmd( "ttk::radiobutton .a.r.cal -text \"V(...) - request the value of a variable\" -underline 0 -variable res -value 26" );
		cmd( "ttk::radiobutton .a.r.for -text \"CYCLE - insert a cycle over a group of objects\" -underline 0 -variable res -value 27" );
		cmd( "ttk::radiobutton .a.r.sum -text \"SUM - compute the sum of a variable over a set of objects\" -underline 1 -variable res -value 56" );
		cmd( "ttk::radiobutton .a.r.incr -text \"INCR - increment the value of a parameter\" -underline 0 -variable res -value 40" );
		cmd( "ttk::radiobutton .a.r.mult -text \"MULT - multiply the value of a parameter\" -underline 0 -variable res -value 45" );
		cmd( "ttk::radiobutton .a.r.sear -text \"SEARCH - search the first instance an object type\" -underline 2 -variable res -value 55" );
		cmd( "ttk::radiobutton .a.r.scnd -text \"SEARCH_CND - conditional search a specific object\" -underline 0 -variable res -value 30" );
		cmd( "ttk::radiobutton .a.r.lqs -text \"SORT - sort a group of objects\" -underline 3 -variable res -value 31" );
		cmd( "ttk::radiobutton .a.r.rndo -text \"RNDDRAW - draw an object\" -underline 1 -variable res -value 54" );
		cmd( "ttk::radiobutton .a.r.wri -text \"WRITE - overwrite a variable or parameter with a new value\" -underline 0 -variable res -value 29" );
		cmd( "ttk::radiobutton .a.r.addo -text \"ADDOBJ - add a new object\" -underline 3 -variable res -value 52" );
		cmd( "ttk::radiobutton .a.r.delo -text \"DELETE - delete an object\" -underline 0 -variable res -value 53" );
		cmd( "ttk::radiobutton .a.r.net -text \"Network macros\" -underline 6 -variable res -value 72" );
		cmd( "ttk::radiobutton .a.r.math -text \"Mathematical functions\" -underline 12 -variable res -value 51" );

		cmd( "pack .a.r.equ .a.r.cal .a.r.for .a.r.sum .a.r.incr .a.r.mult .a.r.sear .a.r.scnd .a.r.lqs .a.r.rndo .a.r.wri .a.r.addo .a.r.delo .a.r.net .a.r.math -anchor w" );
		cmd( "pack .a.tit .a.r -padx 5 -pady 5" );

		cmd( "okhelpcancel .a b { set choice 1 } { LsdHelp LMM.html#LsdScript } { set choice 2 }" );

		cmd( "bind .a <KeyPress-e> { .a.r.equ invoke; set choice 1 }; bind .a <KeyPress-E> { .a.r.equ invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-v> { .a.r.cal invoke; set choice 1 }; bind .a <KeyPress-V> { .a.r.cal invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-u> { .a.r.sum invoke; set choice 1 }; bind .a <KeyPress-U> { .a.r.sum invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-s> { .a.r.scnd invoke; set choice 1 }; bind .a <KeyPress-S> { .a.r.scnd invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-a> { .a.r.sear invoke; set choice 1 }; bind .a <KeyPress-A> { .a.r.sear invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-w> { .a.r.wri invoke; set choice 1 }; bind .a <KeyPress-W> { .a.r.wri invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-c> { .a.r.for invoke; set choice 1 }; bind .a <KeyPress-C> { .a.r.for invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-t> { .a.r.lqs invoke; set choice 1 }; bind .a <KeyPress-T> { .a.r.lqs invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-o> { .a.r.addo invoke; set choice 1 }; bind .a <KeyPress-O> { .a.r.addo invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-d> { .a.r.delo invoke; set choice 1 }; bind .a <KeyPress-D> { .a.r.delo invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-i> { .a.r.incr invoke; set choice 1 }; bind .a <KeyPress-I> { .a.r.incr invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-m> { .a.r.mult invoke; set choice 1 }; bind .a <KeyPress-M> { .a.r.mult invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-h> { .a.r.math invoke; set choice 1 }; bind .a <KeyPress-H> { .a.r.math invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-k> { .a.r.net invoke; set choice 1 }; bind .a <KeyPress-K> { .a.r.net invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-n> { .a.r.rndo invoke; set choice 1 }; bind .a <KeyPress-N> { .a.r.rndo invoke; set choice 1 }" );
		cmd( "bind .a <Return> { .a.b.ok invoke }" );

		cmd( "showtop .a" );
		cmd( "focus .a.r.cal" );
		cmd( "mousewarpto .a.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set choice $res" );
		goto loop;
	}

	// insert a math function
	if ( choice == 51 )
	{
		cmd( "set value1 \"0\"" );
		cmd( "set value2 \"1\"" );
		cmd( "set res 9" );
		cmd( "set str {uniform($value1, $value2)}" );

		cmd( "newtop .a \"Math Functions\" { set choice 2 }" );

		cmd( "ttk::frame .a.e" );
		cmd( "ttk::label .a.e.l -text \"Function parameter(s)\"" );

		cmd( "ttk::frame .a.e.e" );

		cmd( "ttk::frame .a.e.e.e1" );
		cmd( "ttk::label .a.e.e.e1.l -text Minimum" );
		cmd( "ttk::entry .a.e.e.e1.e -width 20 -justify center -textvariable value1" );
		cmd( "bind .a.e.e.e1.e <Return> { if [ string equal [ .a.e.e.e2.e cget -state ] normal ] { focus .a.e.e.e2.e; .a.e.e.e2.e selection range 0 end } { .a.b.ok invoke } }" );
		cmd( "pack .a.e.e.e1.l .a.e.e.e1.e" );

		cmd( "ttk::frame .a.e.e.e2" );
		cmd( "ttk::label .a.e.e.e2.l -text Maximum" );
		cmd( "ttk::entry .a.e.e.e2.e -width 20 -justify center -textvariable value2" );
		cmd( "bind .a.e.e.e2.e <Return> { .a.b.ok invoke }" );
		cmd( "pack .a.e.e.e2.l .a.e.e.e2.e" );

		cmd( "pack .a.e.e.e1 .a.e.e.e2 -padx 5 -pady 5" );

		cmd( "pack .a.e.l .a.e.e" );

		cmd( "ttk::frame .a.f -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.f.r1 -text \"Square root\" -variable res -value 1 -command { .a.e.e.e1.l conf -text Value; .a.e.e.e2.l conf -text (unused); .a.e.e.e2.e conf -state disabled; set str {sqrt($value1)} }" );
		cmd( "ttk::radiobutton .a.f.r2 -text \"Power\" -variable res -value 2 -command { .a.e.e.e1.l conf -text \"Base\"; .a.e.e.e2.l conf -text \"Exponent\"; .a.e.e.e2.e conf -state normal; set str {pow($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r3 -text \"Exponential\" -variable res -value 3 -command { .a.e.e.e1.l conf -text Value; .a.e.e.e2.l conf -text (unused); .a.e.e.e2.e conf -state disabled; set str {exp($value1)} }" );
		cmd( "ttk::radiobutton .a.f.r4 -text \"Logarithm\" -variable res -value 4 -command { .a.e.e.e1.l conf -text Value; .a.e.e.e2.l conf -text (unused); .a.e.e.e2.e conf -state disabled; set str {log($value1)} }" );
		cmd( "ttk::radiobutton .a.f.r5 -text \"Absolute value\" -variable res -value 5 -command { .a.e.e.e1.l conf -text Value; .a.e.e.e2.l conf -text (unused); .a.e.e.e2.e conf -state disabled; set str {abs($value1)} }" );
		cmd( "ttk::radiobutton .a.f.r6 -text \"Minimum value\" -variable res -value 6 -command { .a.e.e.e1.l conf -text \"Value 1\"; .a.e.e.e2.l conf -text \"Value 2\"; .a.e.e.e2.e conf -state normal; set str {min($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r7 -text \"Maximum value\" -variable res -value 7 -command { .a.e.e.e1.l conf -text \"Value 1\"; .a.e.e.e2.l conf -text \"Value 2\"; .a.e.e.e2.e conf -state normal; set str {max($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r8 -text \"Round to closest integer\" -variable res -value 8 -command { .a.e.e.e1.l conf -text Value; .a.e.e.e2.l conf -text (unused); .a.e.e.e2.e conf -state disabled; set str {round($value1)} }" );
		cmd( "ttk::radiobutton .a.f.r9 -text \"Uniform random draw\" -variable res -value 9 -command { .a.e.e.e1.l conf -text Minimum; .a.e.e.e2.l conf -text Maximum; .a.e.e.e2.e conf -state normal; set str {uniform($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r10 -text \"Integer uniform random draw\" -variable res -value 10 -command { .a.e.e.e1.l conf -text Minimum; .a.e.e.e2.l conf -text Maximum; .a.e.e.e2.e conf -state normal; set str {uniform_int($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r11 -text \"Bernoulli random draw\" -variable res -value 11 -command { .a.e.e.e1.l conf -text Probability; .a.e.e.e2.l conf -text (unused); .a.e.e.e2.e conf -state disabled; set str {bernoulli($value1)} }" );
		cmd( "ttk::radiobutton .a.f.r12 -text \"Poisson random draw\" -variable res -value 12 -command { .a.e.e.e1.l conf -text Mean; .a.e.e.e2.l conf -text (unused); .a.e.e.e2.e conf -state disabled; set str {poisson($value1)} }" );
		cmd( "ttk::radiobutton .a.f.r13 -text \"Normal random draw\" -variable res -value 13 -command { .a.e.e.e1.l conf -text Mean; .a.e.e.e2.l conf -text \"Std. deviation\"; .a.e.e.e2.e conf -state normal; set str {norm($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r14 -text \"Lognormal random draw\" -variable res -value 14 -command { .a.e.e.e1.l conf -text Mean; .a.e.e.e2.l conf -text \"Std. deviation\"; .a.e.e.e2.e conf -state normal; set str {lnorm($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r15 -text \"Beta random draw\" -variable res -value 15 -command { .a.e.e.e1.l conf -text Alpha; .a.e.e.e2.l conf -text Beta; .a.e.e.e2.e conf -state normal; set str {beta($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r16 -text \"Gamma random draw\" -variable res -value 16 -command { .a.e.e.e1.l conf -text Alpha; .a.e.e.e2.l conf -text Beta; .a.e.e.e2.e conf -state normal; set str {gamma($value1, $value2)} }" );
		cmd( "ttk::radiobutton .a.f.r17 -text \"Pareto random draw\" -variable res -value 17 -command { .a.e.e.e1.l conf -text Mean; .a.e.e.e2.l conf -text Alpha; .a.e.e.e2.e conf -state normal; set str {pareto($value1, $value2)} }" );

		cmd( "pack .a.f.r1 .a.f.r2 .a.f.r3 .a.f.r4 .a.f.r5 .a.f.r6 .a.f.r7 .a.f.r8 .a.f.r9 .a.f.r10 .a.f.r11 .a.f.r12 .a.f.r13 .a.f.r14 .a.f.r15 .a.f.r16 .a.f.r17 -anchor w" );

		cmd( "ttk::label .a.more -text \"(see Help for more functions/distributions)\"" );

		cmd( "pack .a.e .a.f .a.more -padx 5 -pady 5" );

		cmd( "okhelpcancel .a b { set choice 1 } { LsdHelp LSD_macros.html#Math } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.e.e.e1.e" );
		cmd( ".a.e.e.e1.e selection range 0 end" );
		cmd( "mousewarpto .a.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( ".f.t.t insert insert [ subst $str ] }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a LSD equation
	if ( choice == 25 )
	{
		cmd( "set v_label \"\"" );
		cmd( "set isfun 0" );

		cmd( "newtop .a \"Insert Equation\" { set choice 2 }" );

		cmd( "ttk::frame .a.label" );
		cmd( "ttk::label .a.label.l -text \"Variable/function name\"" );
		cmd( "ttk::entry .a.label.n -width 20 -textvariable v_label -justify center" );
		cmd( "pack .a.label.l .a.label.n" );

		cmd( "bind .a.label.n <Return> { focus .a.b.ok }" );
		cmd( "pack .a.label -padx 5 -pady 5" );

		cmd( "okhelpcancel .a b { set choice 1 } { LsdHelp LSD_macros.html#EQUATION } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( ".a.label.n selection range 0 end" );
		cmd( "focus .a.label.n" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( ".f.t.t insert insert \"EQUATION(\\\"$v_label\\\")\\n\"" );
		cmd( ".f.t.t insert insert \"/*\\nComment\\n*/\\n\\n\"" );
		cmd( ".f.t.t insert insert \"RESULT( )\\n\"" );
		cmd( ".f.t.t mark set insert \"$a + 2 line\"" );
		cmd( ".f.t.t tag add sel insert \"insert + 7 char\"" );

		v_counter = 0;
		
		cmd( ".f.t.t see insert" );

		recolor_all = true;
		choice = 0;
		goto loop;
	}

	// insert a V macro
	if ( choice == 26 )
	{
		cmd( "set v_num %d", v_counter );
		cmd( "set v_label \"\"" );
		cmd( "set v_lag 0" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'V(...)' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Number v\\\[x\\] to assign to\"" );
		cmd( "ttk::entry .a.v.e -width 2 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Variable or parameter to retrieve\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.l.e; .a.l.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.l" );
		cmd( "ttk::label .a.l.l -text \"Lag to use\"" );
		cmd( "ttk::entry .a.l.e -width 2 -textvariable v_lag -justify center" );
		cmd( "bind .a.l.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.l.l .a.l.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Parent object\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.v .a.n .a.l .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#V } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.v.e" );
		cmd( ".a.v.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"v\\\[$v_num\\] = \" }" );

		cmd( "if { $v_lag == 0 && $v_obj == \"p\" } { .f.t.t insert insert \"V(\\\"$v_label\\\")\" }" );
		cmd( "if { $v_lag != 0 && $v_obj == \"p\" } { .f.t.t insert insert \"VL(\\\"$v_label\\\", $v_lag)\" }" );
		cmd( "if { $v_lag == 0 && $v_obj != \"p\" } { .f.t.t insert insert \"VS($v_obj, \\\"$v_label\\\")\" }" );
		cmd( "if { $v_lag != 0 && $v_obj != \"p\" && [ string is integer -strict $v_lag ] } { .f.t.t insert insert \"VLS($v_obj, \\\"$v_label\\\", $v_lag)\" }" );

		cmd( "if { $v_num != \"\" } { .f.t.t insert insert \";\" }" );

		cmd( "if { $v_num == \"\" } { set num -1 } { set num $v_num }" );
		
		if ( num != -1 )
			v_counter = ++num;

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a CYCLE macro
	if ( choice == 27 )
	{
		cmd( "set v_label \"\"" );
		cmd( "set v_obj cur" );
		cmd( "set v_par p" );

		cmd( "newtop .a \"Insert 'CYCLE' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Object to cycle through\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.c.e; .a.c.e selection range 0 end }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Cycling pointer\"" );
		cmd( "ttk::entry .a.c.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.c.e <Return> { focus .a.p.e; .a.p.e selection range 0 end }" );
		cmd( "pack .a.c.l .a.c.e" );

		cmd( "ttk::frame .a.p" );
		cmd( "ttk::label .a.p.l -text \"Parent object\"" );
		cmd( "ttk::entry .a.p.e -width 25 -textvariable v_par -justify center" );
		cmd( "bind .a.p.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.p.l .a.p.e" );

		cmd( "pack .a.o .a.c .a.p -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#CYCLE } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.o.e" );
		cmd( ".a.o.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_par == \"p\" } { .f.t.t insert insert \"CYCLE($v_obj, \\\"$v_label\\\")\\n\" } { .f.t.t insert insert \"CYCLES($v_par, $v_obj, \\\"$v_label\\\")\\n\" }" );

		cmd( "set in [ .f.t.t index insert ]" );
		cmd( "scan $in %%d.%%d line col" );
		cmd( "set line [ expr $line -1 ]" );
		cmd( "set s [ .f.t.t get $line.0 $line.end ]" );
		s = ( char * ) Tcl_GetVar( inter, "s", 0 );
		for ( i = 0; s[ i ] == ' ' || s[ i ] == '\t'; ++i )
			str[ i ] = s[ i ];
		if ( i > 0 )
		{
			str[ i ] = '\0';
			cmd( ".f.t.t insert insert \"%s\"", str );
			cmd( ".f.t.t insert insert \"{\\n\"" );
			cmd( ".f.t.t insert insert \"%s\\t\"", str );
			cmd( "set b [ .f.t.t index insert ]" );
			cmd( ".f.t.t insert insert \"\\n%s\"", str );
		}
		else
		{
			cmd( ".f.t.t insert insert \"{\\n\"" );
			cmd( ".f.t.t insert insert \"\\t\"" );
			cmd( "set b [ .f.t.t index insert ]" );
			cmd( ".f.t.t insert insert \"\\n\"" );
		}

		cmd( ".f.t.t insert insert \"}\\n\"" );
		cmd( ".f.t.t mark set insert \"$b\"" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a INCR macro
	if ( choice == 40 )
	{
		cmd( "set v_num %d", v_counter );
		cmd( "set v_label \"\"" );
		cmd( "set v_val 1" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'INCR' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Number v\\\[x\\] to assign the result after increment\"" );
		cmd( "ttk::entry .a.v.e -width 2 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Variable to increase\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.a.e; .a.a.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.a" );
		cmd( "ttk::label .a.a.l -text \"Value to add\"" );
		cmd( "ttk::entry .a.a.e -width 15 -textvariable v_val -justify center" );
		cmd( "bind .a.a.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.a.l .a.a.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Parent object\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.v .a.n .a.a .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#INCR } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.v.e" );
		cmd( ".a.v.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"v\\\[$v_num\\] = \" }" );
		cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"INCRS($v_obj, \\\"$v_label\\\", $v_val)\" } { .f.t.t insert insert \"INCR(\\\"$v_label\\\", $v_val)\" }" );

		cmd( "if { $v_num != \"\" } { .f.t.t insert insert \";\" }" );

		cmd( "if { $v_num == \"\" } { set num -1 } { set num $v_num }" );
		
		if ( num != -1 )
			v_counter = ++num;

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a MULT macro
	if ( choice == 45 )
	{
		cmd( "set v_num %d", v_counter );
		cmd( "set v_label \"\"" );
		cmd( "set v_val 1" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'MULT' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Number v\\\[x\\] to assign the result after multiplication\"" );
		cmd( "ttk::entry .a.v.e -width 2 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Variable to multiply\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.a.e; .a.a.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.a" );
		cmd( "ttk::label .a.a.l -text \"Value to multiply\"" );
		cmd( "ttk::entry .a.a.e -width 15 -textvariable v_val -justify center" );
		cmd( "bind .a.a.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.a.l .a.a.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Parent object\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.v .a.n .a.a .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#MULT } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.v.e" );
		cmd( ".a.v.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"v\\\[$v_num\\] = \" }" );
		cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"MULTS($v_obj, \\\"$v_label\\\", $v_val)\" } { .f.t.t insert insert \"MULT(\\\"$v_label\\\", $v_val)\" }" );

		cmd( "if { $v_num != \"\" } { .f.t.t insert insert \";\" }" );

		cmd( "if { $v_num == \"\" } { set num -1 } { set num $v_num }" );
		
		if ( num != -1 )
			v_counter = ++num;

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a WRITE macro
	if ( choice == 29 )
	{
		cmd( "set v_num 0" );
		cmd( "set v_label \"\"" );
		cmd( "set v_lag T" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'WRITE' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Value to write\"" );
		cmd( "ttk::entry .a.v.e -width 15 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Variable or parameter to write\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.l.e; .a.l.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.l" );
		cmd( "ttk::label .a.l.l -text \"Time step appearing as latest computation\"" );
		cmd( "ttk::entry .a.l.e -width 15 -textvariable v_lag -justify center" );
		cmd( "bind .a.l.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.l.l .a.l.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Parent object\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.v .a.n .a.l .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#WRITE } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.v.e" );
		cmd( ".a.v.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_obj == \"p\" && ( $v_lag == \"\" || [ string equal -nocase $v_lag t ] ) } { .f.t.t insert insert \"WRITE(\\\"$v_label\\\", $v_num);\" }" );
		cmd( "if { $v_obj == \"p\" && $v_lag != \"\" && ! [ string equal -nocase $v_lag t ] } { .f.t.t insert insert \"WRITEL(\\\"$v_label\\\", $v_num, $v_lag);\" }" );
		cmd( "if { $v_obj != \"p\" && ( $v_lag == \"\" || [ string equal -nocase $v_lag t ] ) } { .f.t.t insert insert \"WRITES($v_obj, \\\"$v_label\\\", $v_num);\" }" );
		cmd( "if { $v_obj != \"p\" && $v_lag != \"\" && ! [ string equal -nocase $v_lag t ] } { .f.t.t insert insert \"WRITELS($v_obj, \\\"$v_label\\\", $v_num, $v_lag);\" }" );

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a SEARCH_CND macro
	if ( choice == 30 )
	{
		cmd( "set v_obj0 cur" );
		cmd( "set v_num 0" );
		cmd( "set v_label \"\"" );
		cmd( "set v_lag 0" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'SEARCH_CND' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Pointer to return the object found\"" );
		cmd( "ttk::entry .a.d.e -width 6 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Value to search for\"" );
		cmd( "ttk::entry .a.v.e -width 15 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Variable or parameter to search\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.l.e; .a.l.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.l" );
		cmd( "ttk::label .a.l.l -text \"Lag to use\"" );
		cmd( "ttk::entry .a.l.e -width 2 -textvariable v_lag -justify center" );
		cmd( "bind .a.l.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.l.l .a.l.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Object from which to search\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.v .a.n .a.l .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#SEARCH_CND } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_obj == \"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0 = SEARCH_CND(\\\"$v_label\\\", $v_num);\" }" );
		cmd( "if { $v_obj == \"p\" && [ string is integer -strict $v_lag ] && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0 = SEARCH_CNDL(\\\"$v_label\\\", $v_num, $v_lag);\" }" );
		cmd( "if { $v_obj != \"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0 = SEARCH_CNDS($v_obj, \\\"$v_label\\\", $v_num);\" }" );
		cmd( "if { $v_obj != \"p\" && [ string is integer -strict $v_lag ] && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0 = SEARCH_CNDLS($v_obj, \\\"$v_label\\\", $v_num, $v_lag);\" }" );

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a SORT macro
	if ( choice == 31 )
	{
		cmd( "set v_obj p" );
		cmd( "set v_obj0 \"\"" );
		cmd( "set v_label \"\"" );
		cmd( "set v_direction 1" );

		cmd( "newtop .a \"Insert 'SORT' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Object to be sorted\"" );
		cmd( "ttk::entry .a.d.e -width 25 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Variable or parameter for sorting\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.s.u }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.s" );
		cmd( "ttk::label .a.s.l -text \"Sorting direction\"" );
		cmd( "ttk::radiobutton .a.s.u -text Increasing -variable v_direction -value 1" );
		cmd( "ttk::radiobutton .a.s.d -text Decreasing -variable v_direction -value 2" );
		cmd( "bind .a.s <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.s.l .a.s.u .a.s.d" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Parent object\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.n .a.s .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#SORT } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set choice $v_direction" );
		cmd( "set a [ .f.t.t index insert ]" );

		if ( choice == 1)
		  cmd( "set direction \"UP\"" );
		else
		  cmd( "set direction \"DOWN\"" );

		cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"SORT(\\\"$v_obj0\\\", \\\"$v_label\\\", \\\"$direction\\\");\" }" );
		cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"SORTS($v_obj, \\\"$v_obj0\\\", \\\"$v_label\\\", \\\"$direction\\\");\" }" );

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a ADDOBJ macro
	if ( choice == 52 )
	{
		cmd( "set v_obj0 cur" );
		cmd( "set v_label \"\"" );
		cmd( "set numobj \"1\"" );
		cmd( "set v_num \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'ADDOBJ' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Pointer to return the object created\"" );
		cmd( "ttk::entry .a.d.e -width 6 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Object to create\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.x.e; .a.x.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.x" );
		cmd( "ttk::label .a.x.l -text \"Number of objects to create\"" );
		cmd( "ttk::entry .a.x.e -width 6 -textvariable numobj -justify center" );
		cmd( "bind .a.x.e <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "pack .a.x.l .a.x.e" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Example object (if available)\"" );
		cmd( "ttk::entry .a.v.e -width 25 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Parent object for new object(s)\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.n .a.x .a.v .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#ADDOBJ } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $numobj == \"1\" } { set choice 1 } { set choice 0 }" );
		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \"$v_obj0 = \" }" );

		if ( choice  == 1 )
		{
		cmd( "if { $v_obj == \"p\" && $v_num == \"\" } { .f.t.t insert insert \"ADDOBJ(\\\"$v_label\\\");\" }" );
		cmd( "if { $v_obj == \"p\" && $v_num != \"\" } { .f.t.t insert insert \"ADDOBJ_EX(\\\"$v_label\\\", $v_num);\" }" );
		cmd( "if { $v_obj != \"p\" && $v_num == \"\" } { .f.t.t insert insert \"ADDOBJS($v_obj, \\\"$v_label\\\");\" }" );
		cmd( "if { $v_obj != \"p\" && $v_num != \"\" } { .f.t.t insert insert \"ADDOBJ_EXS($v_obj, \\\"$v_label\\\", $v_num);\" }" );
		}
		else
		{
		cmd( "if { $v_obj == \"p\" && $v_num != \"\" } { .f.t.t insert insert \"ADDNOBJ_EX(\\\"$v_label\\\", $numobj, $v_num);\"; set choice -3 }" );
		cmd( "if { $v_obj != \"p\" && $v_num != \"\" } { .f.t.t insert insert \"ADDNOBJ_EXS($v_obj, \\\"$v_label\\\", $numobj, $v_num);\"; set choice -3 }" );
		cmd( "if { $v_obj == \"p\" && $v_num == \"\" } { .f.t.t insert insert \"ADDNOBJ(\\\"$v_label\\\", $numobj);\"; set choice -3 }" );
		cmd( "if { $v_obj != \"p\" && $v_num == \"\" } { .f.t.t insert insert \"ADDNOBJS($v_obj, \\\"$v_label\\\", $numobj);\"; set choice -3 }" );
		}

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a DELETE macro
	if ( choice == 53 )
	{
		cmd( "set v_obj0 cur" );

		cmd( "newtop .a \"Insert 'DELETE' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Object to delete\"" );
		cmd( "ttk::entry .a.d.e -width 25 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "pack .a.d -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#DELETE } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( ".f.t.t insert insert \"DELETE($v_obj0);\"" );

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a RNDDRAW macro
	if ( choice == 54 )
	{
		cmd( "set v_obj0 cur" );
		cmd( "set v_num \"\"" );
		cmd( "set v_label \"\"" );
		cmd( "set v_lag 0" );
		cmd( "set v_tot \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'RNDDRAW' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Pointer to return the object found\"" );
		cmd( "ttk::entry .a.d.e -width 6 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Object to draw\"" );
		cmd( "ttk::entry .a.v.e -width 25 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Variable or parameter defining probabilities\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.l.e; .a.l.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.l" );
		cmd( "ttk::label .a.l.l -text \"Lag to use\"" );
		cmd( "ttk::entry .a.l.e -width 2 -textvariable v_lag -justify center" );
		cmd( "bind .a.l.e <Return> { focus .a.t.e; .a.t.e selection range 0 end }" );
		cmd( "pack .a.l.l .a.l.e" );

		cmd( "ttk::frame .a.t" );
		cmd( "ttk::label .a.t.l -text \"Sum over all values (if available)\"" );
		cmd( "ttk::entry .a.t.e -width 15 -textvariable v_tot -justify center" );
		cmd( "bind .a.t.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.t.l .a.t.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Object from which to search\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.v .a.n .a.l .a.t .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#RNDDRAW } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_tot == \"\" } { set choice 1 } { set choice 2 }" );

		if ( choice == 1 )
		 {
		  cmd( "if { $v_obj == \"p\" && $v_lag == 0 && $v_label != \"\" } { .f.t.t insert insert \"$v_obj0 = RNDDRAW(\\\"$v_num\\\", \\\"$v_label\\\");\" }" );
		  cmd( "if { $v_obj == \"p\" && $v_lag == 0 && $v_label == \"\" } { .f.t.t insert insert \"$v_obj0 = RNDDRAW_FAIR(\\\"$v_num\\\");\" }" );
		  cmd( "if { $v_obj == \"p\" && $v_lag != 0 && [ string is integer -strict $v_lag ] } { .f.t.t insert insert \"$v_obj0 = RNDDRAWL(\\\"$v_num\\\", \\\"$v_label\\\", $v_lag);\" }" );
		  cmd( "if { $v_obj != \"p\" && $v_lag == 0 && $v_label != \"\" } { .f.t.t insert insert \"$v_obj0 = RNDDRAWS($v_obj, \\\"$v_num\\\", \\\"$v_label\\\");\" }" );
		  cmd( "if { $v_obj != \"p\" && $v_lag == 0 && $v_label == \"\" } { .f.t.t insert insert \"$v_obj0 = RNDDRAW_FAIRS($v_obj, \\\"$v_num\\\");\" }" );
		  cmd( "if { $v_obj != \"p\" && $v_lag != 0 && [ string is integer -strict $v_lag ] } { .f.t.t insert insert \"$v_obj0 = RNDDRAWLS($v_obj, \\\"$v_num\\\", \\\"$v_label\\\", $v_lag);\" }" );
		 }
		else
		 {
		  cmd( "if { $v_obj == \"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0 = RNDDRAWTOT(\\\"$v_num\\\", \\\"$v_label\\\", $v_tot);\" }" );
		  cmd( "if { $v_obj == \"p\" && $v_lag != 0 && [ string is integer -strict $v_lag ] } { .f.t.t insert insert \"$v_obj0 = RNDDRAW_TOTL(\\\"$v_num\\\", \\\"$v_label\\\", $v_lag, $v_tot);\" }" );
		  cmd( "if { $v_obj != \"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0 = RNDDRAWTOTS($v_obj, \\\"$v_num\\\", \\\"$v_label\\\", $v_tot);\" }" );
		  cmd( "if { $v_obj != \"p\" && $v_lag != 0 && [ string is integer -strict $v_lag ] } { .f.t.t insert insert \"$v_obj0 = RNDDRAW_TOTLS($v_obj, \\\"$v_num\\\", \\\"$v_label\\\", $v_lag, $v_tot);\" }" );
		 }

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a SEARCH macro
	if ( choice == 55 )
	{
		cmd( "set v_obj0 cur" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'SEARCH' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Pointer to return the object found\"" );
		cmd( "ttk::entry .a.d.e -width 6 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Object to search\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Object from which to search\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LSD_macros.html#SEARCH } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"$v_obj0 = SEARCH(\\\"$v_label\\\");\" } { .f.t.t insert insert \"$v_obj0 = SEARCHS($v_obj, \\\"$v_label\\\");\" }" );

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a SUM macro
	if ( choice == 56 )
	{
		cmd( "set v_num %d", v_counter );
		cmd( "set v_label \"\"" );
		cmd( "set v_lag 0" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'SUM' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Number v\\\[x\\] to assign the result\"" );
		cmd( "ttk::entry .a.v.e -width 2 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Variable or parameter to sum\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.l.e; .a.l.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.l" );
		cmd( "ttk::label .a.l.l -text \"Lag to use\"" );
		cmd( "ttk::entry .a.l.e -width 2 -textvariable v_lag -justify center" );
		cmd( "bind .a.l.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.l.l .a.l.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Parent object\"" );
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.v .a.n .a.l .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#SUM } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.v.e" );
		cmd( ".a.v.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"v\\\[$v_num\\] = \" }" );

		cmd( "if { $v_lag == 0 && $v_obj == \"p\" } { .f.t.t insert insert \"SUM(\\\"$v_label\\\")\" }" );
		cmd( "if { $v_lag != 0 && [ string is integer -strict $v_lag ] && $v_obj == \"p\" } { .f.t.t insert insert \"SUML(\\\"$v_label\\\", $v_lag)\" }" );
		cmd( "if { $v_lag == 0 && $v_obj != \"p\" } { .f.t.t insert insert \"SUMS($v_obj, \\\"$v_label\\\")\" }" );
		cmd( "if { $v_lag != 0 && [ string is integer -strict $v_lag ] && $v_obj != \"p\" } { .f.t.t insert insert \"SUMLS($v_obj, \\\"$v_label\\\", $v_lag)\" }" );

		cmd( "if { $v_num != \"\" } { .f.t.t insert insert \";\" }" );

		cmd( "if { $v_num == \"\" } { set num -1 } { set num $v_num }" );
		
		if ( num != -1 )
			v_counter = ++num;

		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// insert a network macro
	if ( choice == 72 )
	{
		cmd( "set res 73");

		cmd( "newtop .a \"Network Macros\" { set choice 2 }" );

		cmd( "ttk::label .a.tit -text \"Available LSD network macros\"" );

		cmd( "ttk::frame .a.f -borderwidth 1 -relief solid" );

		cmd( "ttk::radiobutton .a.f.r1 -text \"INIT - create a new network\" -variable res -value 73 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r2 -text \"LOAD - load a network from a file\" -variable res -value 74 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r3 -text \"SAVE - save a network to a file\" -variable res -value 75 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r4 -text \"SNAP - add a network snapshot to a file\" -variable res -value 76 -underline 1" );
		cmd( "ttk::radiobutton .a.f.r5 -text \"ADD - add a node or link\" -variable res -value 77 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r6 -text \"V - get the values of a node or link\" -variable res -value 78 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r7 -text \"WRITE - set the values of a node or link\" -variable res -value 79 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r8 -text \"CYCLE - cycle through links\" -variable res -value 80 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r9 -text \"SEARCH - search for a node or link\" -variable res -value 81 -underline 1" );
		cmd( "ttk::radiobutton .a.f.r10 -text \"LINK - get objects connected by link\" -variable res -value 82 -underline 3" );
		cmd( "ttk::radiobutton .a.f.r11 -text \"SHUFFLE - shuffle nodes in a network\" -variable res -value 83 -underline 1" );
		cmd( "ttk::radiobutton .a.f.r12 -text \"RNDDRAW - random draw a node or link\" -variable res -value 84 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r13 -text \"DELETE - delete a network, node or link\" -variable res -value 85 -underline 0" );
		cmd( "ttk::radiobutton .a.f.r14 -text \"STAT - statistics about a network or node\" -variable res -value 86 -underline 1" );

		cmd( "pack .a.f.r1 .a.f.r2 .a.f.r3 .a.f.r4 .a.f.r5 .a.f.r6 .a.f.r7 .a.f.r8 .a.f.r9 .a.f.r10 .a.f.r11 .a.f.r12 .a.f.r13 .a.f.r14 -anchor w" );

		cmd( "pack .a.tit .a.f -padx 5 -pady 5" );

		cmd( "okhelpcancel .a b { set choice 1 } { LsdHelp LSD_macros.html#Networks } { set choice 2 }" );

		cmd( "bind .a <KeyPress-i> { .a.f.r1  invoke; set choice 1 }; bind .a <KeyPress-I> { .a.f.r1  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-l> { .a.f.r2  invoke; set choice 1 }; bind .a <KeyPress-L> { .a.f.r2  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-s> { .a.f.r3  invoke; set choice 1 }; bind .a <KeyPress-S> { .a.f.r3  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-n> { .a.f.r4  invoke; set choice 1 }; bind .a <KeyPress-N> { .a.f.r4  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-a> { .a.f.r5  invoke; set choice 1 }; bind .a <KeyPress-A> { .a.f.r5  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-v> { .a.f.r6  invoke; set choice 1 }; bind .a <KeyPress-V> { .a.f.r6  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-w> { .a.f.r7  invoke; set choice 1 }; bind .a <KeyPress-W> { .a.f.r7  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-c> { .a.f.r8  invoke; set choice 1 }; bind .a <KeyPress-C> { .a.f.r8  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-e> { .a.f.r9  invoke; set choice 1 }; bind .a <KeyPress-E> { .a.f.r9  invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-k> { .a.f.r10 invoke; set choice 1 }; bind .a <KeyPress-K> { .a.f.r10 invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-h> { .a.f.r11 invoke; set choice 1 }; bind .a <KeyPress-H> { .a.f.r11 invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-r> { .a.f.r12 invoke; set choice 1 }; bind .a <KeyPress-R> { .a.f.r12 invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-d> { .a.f.r13 invoke; set choice 1 }; bind .a <KeyPress-D> { .a.f.r13 invoke; set choice 1 }" );
		cmd( "bind .a <KeyPress-t> { .a.f.r14 invoke; set choice 1 }; bind .a <KeyPress-T> { .a.f.r14.a.r.net invoke; set choice 1 }" );
		cmd( "bind .a <Return> { .a.b.ok invoke }" );

		cmd( "showtop .a" );
		cmd( "focus .a.f.r1" );
		cmd( "mousewarpto .a.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set choice $res" );		// execute selected macro

		goto loop;
	}

	// create a new network
	if ( choice == 73 )
	{
		cmd( "set netListLong [ list \"Disconnected (no links)\" \"Fully connected\" \"Star (undirected)\" \"Circle (undirected)\" \"Lattice (directed)\" \"Random (directed)\" \"Random (undirected)\" \"Uniform random (directed)\" \"Renyi-Erdos random (undirected)\" \"Small World (undirected)\" \"Scale-free (undirected)\" ]");
		cmd( "set netListShort [ list \"DISCONNECTED\" \"CONNECTED\" \"STAR\" \"CIRCLE\" \"LATTICE\" \"RANDOM-DIR\" \"RANDOM-UNDIR\" \"UNIFORM\" \"RENYI-ERDOS\" \"SMALL-WORLD\" \"SCALE-FREE\" ]");
		cmd( "set netListPar [ list [ list ] [ list ] [ list ] [ list \"Number of neighbors\" ] [ list \"Number of columns\" \"Neighbors (4 or 8)\" ] [ list \"Number of links\" ] [ list \"Number of links (2 per connection)\" ] [ list \"Out-degree\" ] [ list \"Out-degree\" ] [ list \"Out-degree\" \"Reconnection probability\" ] [ list \"Out-degree\" \"Power-law degree\" ] ]");

		cmd( "set v_net [ lindex $netListLong 0 ]" );
		cmd( "set v_label \"\"" );
		cmd( "set v_num 2" );
		cmd( "set v_obj p" );
		cmd( "set v_par1 \"\"" );
		cmd( "set v_par2 \"\"" );

		cmd( "newtop .a \"Insert 'INIT_NET' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Type of network to create\"" );
		cmd( "ttk::combobox .a.d.e -width 30 -textvariable v_net -justify center -values $netListLong" );
		cmd( "bind .a.d.e <<ComboboxSelected>> { set a [ lindex [ lindex $netListPar [ .a.d.e current ] ] 0 ]; if { $a == \"\" } { set a \"(unused)\"; .a.p1.e configure -state disabled } { .a.p1.e configure -state normal }; .a.p1.l configure -text $a; set a [ lindex [ lindex $netListPar [ .a.d.e current ] ] 1 ]; if { $a == \"\" } { set a \"(unused)\"; .a.p2.e configure -state disabled } { .a.p2.e configure -state normal }; .a.p2.l configure -text $a }" );
		cmd( "bind .a.d.e <Return> { focus .a.x.e; .a.x.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.x" );
		cmd( "ttk::label .a.x.l -text \"Number of nodes\"" );
		cmd( "ttk::entry .a.x.e -width 6 -textvariable v_num -justify center" );
		cmd( "bind .a.x.e <Return> { focus .a.p1.e; .a.p1.e selection range 0 end }" );
		cmd( "pack .a.x.l .a.x.e" );

		cmd( "ttk::frame .a.p1" );
		cmd( "ttk::label .a.p1.l -text \"(unused)\"" );
		cmd( "ttk::entry .a.p1.e -width 6 -textvariable v_par1 -justify center -state disabled" );
		cmd( "bind .a.p1.e <Return> { focus .a.p2.e; .a.p2.e selection range 0 end }" );
		cmd( "pack .a.p1.l .a.p1.e" );

		cmd( "ttk::frame .a.p2" );
		cmd( "ttk::label .a.p2.l -text \"(unused)\"" );
		cmd( "ttk::entry .a.p2.e -width 6 -textvariable v_par2 -justify center -state disabled" );
		cmd( "bind .a.p2.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.p2.l .a.p2.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Network node object name\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Network nodes parent object\"" );
		cmd( "ttk::entry .a.o.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.x .a.p1 .a.p2 .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#INIT_NET } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set res [ .a.d.e current ]" );
		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_par1 == \"\" } { set v_par1 0 }" );
		cmd( "if { $v_par2 == \"\" } { set v_par2 0 }" );

		cmd( "if { $v_obj == \"p\" && [ llength [ lindex $netListPar $res ] ] == 2 && [ string is integer -strict $v_num ] && [ string is integer -strict $v_par1 ] && [ string is double -strict $v_par2 ] } { .f.t.t insert insert \"INIT_NET(\\\"$v_label\\\", \\\"[ lindex $netListShort $res ]\\\", $v_num, $v_par1, $v_par2);\" }" );
		cmd( "if { $v_obj == \"p\" && [ llength [ lindex $netListPar $res ] ] == 1 && [ string is integer -strict $v_num ] && [ string is integer -strict $v_par1 ] } { .f.t.t insert insert \"INIT_NET(\\\"$v_label\\\", \\\"[ lindex $netListShort $res ]\\\", $v_num, $v_par1);\" }" );
		cmd( "if { $v_obj == \"p\" && [ llength [ lindex $netListPar $res ] ] == 0 && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"INIT_NET(\\\"$v_label\\\", \\\"[ lindex $netListShort $res ]\\\", $v_num);\" }" );

		cmd( "if { $v_obj != \"p\" && [ llength [ lindex $netListPar $res ] ] == 2 && [ string is integer -strict $v_num ] && [ string is integer -strict $v_par1 ] && [ string is double -strict $v_par2 ] } { .f.t.t insert insert \"INIT_NETS($v_obj, \\\"$v_label\\\", \\\"[ lindex $netListShort $res ]\\\", $v_num, $v_par1, $v_par2);\" }" );
		cmd( "if { $v_obj != \"p\" && [ llength [ lindex $netListPar $res ] ] == 1 && [ string is integer -strict $v_num ] && [ string is integer -strict $v_par1 ] } { .f.t.t insert insert \"INIT_NETS($v_obj, \\\"$v_label\\\", \\\"[ lindex $netListShort $res ]\\\", $v_num, $v_par1);\" }" );
		cmd( "if { $v_obj != \"p\" && [ llength [ lindex $netListPar $res ] ] == 0 && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"INIT_NETS($v_obj, \\\"$v_label\\\", \\\"[ lindex $netListShort $res ]\\\", $v_num);\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// load a network from a file
	if ( choice == 74 )
	{
		cmd( "set v_net \"\"" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'LOAD_NET' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Network file name (without .net Pajek extension)\"" );
		cmd( "ttk::entry .a.d.e -width 25 -textvariable v_net -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Network node object name\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Network nodes parent object\"" );
		cmd( "ttk::entry .a.o.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#LOAD_NET } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"LOAD_NET(\\\"$v_label\\\", \\\"$v_net\\\");\" }" );
		cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"LOAD_NETS($v_obj, \\\"$v_label\\\", \\\"$v_net\\\");\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// save a network to a file
	if ( choice == 75 )
	{
		cmd( "set v_net \"\"" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'SAVE_NET' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Network file name (without .net Pajek extension)\"" );
		cmd( "ttk::entry .a.d.e -width 25 -textvariable v_net -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Network node object name\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Network nodes parent object\"" );
		cmd( "ttk::entry .a.o.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#SAVE_NET } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"SAVE_NET(\\\"$v_label\\\", \\\"$v_net\\\");\" }" );
		cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"SAVE_NETS($v_obj, \\\"$v_label\\\", \\\"$v_net\\\");\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// add a network snapshot to a file
	if ( choice == 76 )
	{
		cmd( "set v_net \"\"" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'SNAP_NET' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Network file name (without .paj Pajek extension)\"" );
		cmd( "ttk::entry .a.d.e -width 25 -textvariable v_net -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Network node object name\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Network nodes parent object\"" );
		cmd( "ttk::entry .a.o.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.d .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#SNAP_NET } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"SNAP_NET(\\\"$v_label\\\", \\\"$v_net\\\");\" }" );
		cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"SNAP_NETS($v_obj, \\\"$v_label\\\", \\\"$v_net\\\");\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// add a node or link
	if ( choice == 77 )
	{
		cmd( "set v_type 0" );
		cmd( "set v_obj0 cur" );
		cmd( "set v_num \"\"" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'ADDNODE/LINK' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Add to the network a\"" );

		cmd( "ttk::frame .a.c.b -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.c.b.e -text Node -width 6 -variable v_type -value 0 -command { write_any .a.d.e cur; .a.i.l configure -text \"Unique ID number\"; .a.n.l configure -text \"Node name (optional)\"; write_any .a.n.e \"\"; .a.o.l configure -text \"Object for new network node\" }" );
		cmd( "ttk::radiobutton .a.c.b.f -text Link -width 6 -variable v_type -value 1 -command { write_any .a.d.e curl; .a.i.l configure -text \"Link weight\"; .a.n.l configure -text \"Destination node object\"; write_any .a.n.e cur; .a.o.l configure -text \"Origin node object\" }" );
		cmd( "bind .a.c.b.e <Return> { focus .a.d.e; .a.d.e selection range 0 end }" );
		cmd( "bind .a.c.b.f <Return> { focus .a.d.e; .a.d.e selection range 0 end }" );
		cmd( "pack .a.c.b.e .a.c.b.f -side left" );

		cmd( "pack .a.c.l .a.c.b" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Pointer to return the object created\"" );
		cmd( "ttk::entry .a.d.e -width 6 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.i.e; .a.i.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.i" );
		cmd( "ttk::label .a.i.l" ); 			// ID or weight
		cmd( "ttk::entry .a.i.e -width 6 -textvariable v_num -justify center" );
		cmd( "bind .a.i.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.i.l .a.i.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l" );			// name or destination
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l" );			// object or origin object
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.c .a.d .a.i .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#ADDNODE } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( ".a.c.b.e invoke" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "set choice $v_type" );
		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \"$v_obj0 = \" }" );

		if ( choice == 0 )
		{
			cmd( "if { $v_obj == \"p\" && $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"ADDNODE($v_num, \\\"$v_label\\\")\" }" );
			cmd( "if { $v_obj != \"p\" && $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"ADDNODES($v_obj, $v_num, \\\"$v_label\\\")\" }" );
		}
		else
		{
			cmd( "if { $v_obj == \"p\" && $v_num == \"\" } { .f.t.t insert insert \"ADDLINK($v_label)\" }" );
			cmd( "if { $v_obj == \"p\" && $v_num != \"\" && [ string is double -strict $v_num ] } { .f.t.t insert insert \"ADDLINKW($v_label, $v_num)\" }" );
			cmd( "if { $v_obj != \"p\" && $v_num == \"\" } { .f.t.t insert insert \"ADDLINKS($v_obj, $v_label)\" }" );
			cmd( "if { $v_obj != \"p\" && $v_num != \"\" && [ string is double -strict $v_num ] } { .f.t.t insert insert \"ADDLINKWS($v_obj, $v_label, $v_num)\" }" );
		}

		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \";\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// get the values of a node or link
	if ( choice == 78 )
	{
		cmd( "set v_type 0" );
		cmd( "set v_num %d", v_counter );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'V_NODE/LINK' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Get the value of\"" );

		cmd( "ttk::frame .a.c.b -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.c.b.e -text \"Node ID\" -width 8 -variable v_type -value 0 -command { .a.v.l configure -text \"Number v\\\[x\\] to assign to\"; write_any .a.v.e %d; .a.o.l configure -text \"Object node\"; write_any .a.o.e p }", v_counter );
		cmd( "ttk::radiobutton .a.c.b.f -text \"Node name\" -width 8 -variable v_type -value 1 -command { .a.v.l configure -text \"char  pointer to assign to\"; write_any .a.v.e \"\"; .a.o.l configure -text \"Object node\"; write_any .a.o.e p }" );
		cmd( "ttk::radiobutton .a.c.b.g -text \"Link weight\" -width 8 -variable v_type -value 2 -command { .a.v.l configure -text \"Number v\\\[x\\] to assign to\"; write_any .a.v.e %d; .a.o.l configure -text \"Link pointer\"; write_any .a.o.e curl }", v_counter );
		cmd( "bind .a.c.b.e <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "bind .a.c.b.f <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "bind .a.c.b.g <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "pack .a.c.b.e .a.c.b.f .a.c.b.g -side left" );

		cmd( "pack .a.c.l .a.c.b" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l" );			// v[x] or char pointer name
		cmd( "ttk::entry .a.v.e -width 15 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l" );			// object node or link
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.c .a.v .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#V_NODEID } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( ".a.c.b.e invoke" );
		cmd( "focus .a.v.e" );
		cmd( ".a.v.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "set choice $v_type" );

		switch ( choice )
		{
			case 0:
				cmd( "if { $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"v\\\[$v_num\\] = \" }" );
				cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"V_NODEID()\" }" );
				cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"V_NODEIDS($v_obj)\" }" );

				cmd( "if { $v_num == \"\" } { set num -1 } { set num $v_num }" );
				
				if ( num != -1 )
					v_counter = ++num;
				break;

			case 1:
				cmd( "if { $v_num != \"\" } { .f.t.t insert insert \"$v_num = \" }" );
				cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"V_NODENAME()\" }" );
				cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"V_NODENAMES($v_obj)\" }" );
				break;

			case 2:
				cmd( "if { $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"v\\\[$v_num\\] = \" }" );
				cmd( ".f.t.t insert insert \"V_LINK($v_obj)\"" );

				cmd( "if { $v_num == \"\" } { set num -1 } { set num $v_num }" );
				
				if ( num != -1 )
					v_counter = ++num;
				break;

			default:
				break;
		}

		cmd( "if { $v_num != \"\" } { .f.t.t insert insert \";\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// write values to a node or link
	if ( choice == 79 )
	{
		cmd( "set v_type 0" );
		cmd( "set v_num \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'WRITE_NODE/LINK' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Write a value to\"" );

		cmd( "ttk::frame .a.c.b -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.c.b.e -text \"Node ID\" -width 8 -variable v_type -value 0 -command { .a.v.l configure -text \"Node ID to set\"; .a.o.l configure -text \"Object node\"; write_any .a.o.e p }", v_counter );
		cmd( "ttk::radiobutton .a.c.b.f -text \"Node name\" -width 8 -variable v_type -value 1 -command { .a.v.l configure -text \"Node name to set\"; .a.o.l configure -text \"Object node\"; write_any .a.o.e p }" );
		cmd( "ttk::radiobutton .a.c.b.g -text \"Link weight\" -width 8 -variable v_type -value 2 -command { .a.v.l configure -text \"Link weight to set\"; .a.o.l configure -text \"Link pointer\"; write_any .a.o.e curl }", v_counter );
		cmd( "bind .a.c.b.e <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "bind .a.c.b.f <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "bind .a.c.b.g <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "pack .a.c.b.e .a.c.b.f .a.c.b.g -side left" );

		cmd( "pack .a.c.l .a.c.b" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l" );			// ID, name, weight
		cmd( "ttk::entry .a.v.e -width 25 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l" );			// object node or link
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.c .a.v .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#WRITE_NODEID } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( ".a.c.b.e invoke" );
		cmd( "focus .a.v.e" );
		cmd( ".a.v.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "set choice $v_type" );

		switch ( choice )
		{
			case 0:
				cmd( "if { $v_obj == \"p\" && $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"WRITE_NODEID($v_num);\" }" );
				cmd( "if { $v_obj != \"p\" && $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"WRITE_NODEIDS($v_obj, $v_num);\" }" );
				break;

			case 1:
				cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"WRITE_NODENAME(\\\"$v_num\\\");\" }" );
				cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"WRITE_NODENAMES($v_obj, \\\"$v_num\\\");\" }" );
				break;

			case 2:
				cmd( "if { $v_num != \"\" && [ string is double -strict $v_num ] } { .f.t.t insert insert \"WRITE_LINK($v_obj, $v_num);\" }" );
				break;

			default:
				break;
		}
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// cycle through links
	if ( choice == 80 )
	{
		cmd( "set v_obj curl" );
		cmd( "set v_par p" );

		cmd( "newtop .a \"Insert 'CYCLE_LINK' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Cycling link pointer\"" );
		cmd( "ttk::entry .a.c.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.c.e <Return> { focus .a.p.e; .a.p.e selection range 0 end }" );
		cmd( "pack .a.c.l .a.c.e" );

		cmd( "ttk::frame .a.p" );
		cmd( "ttk::label .a.p.l -text \"Parent node object\"" );
		cmd( "ttk::entry .a.p.e -width 25 -textvariable v_par -justify center" );
		cmd( "bind .a.p.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.p.l .a.p.e" );

		cmd( "pack .a.c .a.p -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#CYCLE_LINK } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.c.e" );
		cmd( ".a.c.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"CYCLE_LINK($v_obj)\\n\" }" );
		cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"CYCLE_LINKS($v_par, $v_obj)\\n\" }" );

		cmd( "set in [ .f.t.t index insert ]" );
		cmd( "scan $in %%d.%%d line col" );
		cmd( "set line [ expr $line -1 ]" );
		cmd( "set s [ .f.t.t get $line.0 $line.end ]" );
		s = ( char * ) Tcl_GetVar( inter, "s", 0 );
		for ( i = 0; s[ i ] == ' ' || s[ i ] == '\t'; ++i )
			str[ i ] = s[ i ];
		if ( i > 0 )
		{
			str[ i ] = '\0';
			cmd( ".f.t.t insert insert \"%s\"", str );
			cmd( ".f.t.t insert insert \"{\\n\"" );
			cmd( ".f.t.t insert insert \"%s\\t\"", str );
			cmd( "set b [ .f.t.t index insert ]" );
			cmd( ".f.t.t insert insert \"\\n%s\"", str );
		}
		else
		{
			cmd( ".f.t.t insert insert \"{\\n\"" );
			cmd( ".f.t.t insert insert \"\\t\"" );
			cmd( "set b [ .f.t.t index insert ]" );
			cmd( ".f.t.t insert insert \"\\n\"" );
		}

		cmd( ".f.t.t insert insert \"}\\n\"" );
		cmd( ".f.t.t mark set insert \"$b\"" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// search for a node or link
	if ( choice == 81 )
	{
		cmd( "set v_obj0 cur" );
		cmd( "set v_num 0" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'SEARCH_NODE/LINK' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Search for a\"" );

		cmd( "ttk::frame .a.c.b -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.c.b.e -text Node -width 6 -variable v_type -value 0 -command { write_any .a.d.e cur; .a.n.l configure -text \"Network node object name\"; .a.n.e configure -state normal; .a.o.l configure -text \"Network nodes parent object\" }" );
		cmd( "ttk::radiobutton .a.c.b.f -text Link -width 6 -variable v_type -value 1 -command { write_any .a.d.e curl; .a.n.l configure -text \"(unused)\"; write_any .a.n.e \"\"; .a.n.e configure -state disabled; .a.o.l configure -text \"Node object\" }" );
		cmd( "bind .a.c.b.e <Return> { focus .a.d.e; .a.d.e selection range 0 end }" );
		cmd( "bind .a.c.b.f <Return> { focus .a.d.e; .a.d.e selection range 0 end }" );
		cmd( "pack .a.c.b.e .a.c.b.f -side left" );

		cmd( "pack .a.c.l .a.c.b" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Pointer to return the element found\"" );
		cmd( "ttk::entry .a.d.e -width 6 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"ID to search for\"" );
		cmd( "ttk::entry .a.v.e -width 15 -textvariable v_num -justify center" );
		cmd( "bind .a.v.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l" );			// object name or unused
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l" );			// parent object or node object
		cmd( "ttk::entry .a.o.e -width 25 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.c .a.d .a.v .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LSD_macros.html#SEARCH_NODE } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( ".a.c.b.e invoke" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "set choice $v_type" );
		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \"$v_obj0 = \" }" );

		if ( choice == 0 )
		{
			cmd( "if { $v_obj == \"p\" && $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"SEARCH_NODE(\\\"$v_label\\\", $v_num)\" }" );
			cmd( "if { $v_obj != \"p\" && $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"SEARCH_NODES($v_obj, \\\"$v_label\\\", $v_num)\" }" );
		}
		else
		{
			cmd( "if { $v_obj == \"p\" && $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"SEARCH_LINK($v_num)\" }" );
			cmd( "if { $v_obj != \"p\" && $v_num != \"\" && [ string is integer -strict $v_num ] } { .f.t.t insert insert \"SEARCH_LINKS($v_obj, $v_num)\" }" );
		}

		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \";\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// get objects connected by link
	if ( choice == 82 )
	{
		cmd( "set v_type 0" );
		cmd( "set v_obj0 cur" );
		cmd( "set v_obj curl" );

		cmd( "newtop .a \"Insert 'LINKTO/FROM' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Get the node object\"" );

		cmd( "ttk::frame .a.c.b -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.c.b.e -text \"Link points to\" -width 12 -variable v_type -value 0" );
		cmd( "ttk::radiobutton .a.c.b.f -text \"Link points from\" -width 12 -variable v_type -value 1" );
		cmd( "bind .a.c.b.e <Return> { focus .a.d.e; .a.d.e selection range 0 end }" );
		cmd( "bind .a.c.b.f <Return> { focus .a.d.e; .a.d.e selection range 0 end }" );
		cmd( "pack .a.c.b.e .a.c.b.f -side left" );

		cmd( "pack .a.c.l .a.c.b" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Pointer to return the node pointed\"" );
		cmd( "ttk::entry .a.d.e -width 6 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.v.e; .a.v.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.v" );
		cmd( "ttk::label .a.v.l -text \"Link pointer\"" );
		cmd( "ttk::entry .a.v.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.v.e <Return>  { focus .a.f.ok }" );
		cmd( "pack .a.v.l .a.v.e" );

		cmd( "pack .a.c .a.d .a.v -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#LINKTO } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "set choice $v_type" );
		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \"$v_obj0 = \" }" );

		if ( choice == 0 )
			cmd( ".f.t.t insert insert \"LINKTO($v_obj)\" }" );
		else
			cmd( ".f.t.t insert insert \"LINKFROM($v_obj)\" }" );

		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \";\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// shuffle nodes in a network
	if ( choice == 83 )
	{
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'SHUFFLE_NET' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l -text \"Network node object name\"" );
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l -text \"Network nodes parent object\"" );
		cmd( "ttk::entry .a.o.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#SHUFFLE_NET } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.n.e" );
		cmd( ".a.n.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"SHUFFLE_NET(\\\"$v_label\\\");\" }" );
		cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"SHUFFLE_NETS($v_obj, \\\"$v_label\\\");\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// random draw a node or link
	if ( choice == 84 )
	{
		cmd( "set v_type 0" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );
		cmd( "set v_obj0 cur" );

		cmd( "newtop .a \"Insert 'RNDDRAW_NODE/LINK' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Randomly draw a\"" );

		cmd( "ttk::frame .a.c.b -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.c.b.e -text Node -width 6 -variable v_type -value 0 -command { write_any .a.d.e cur; .a.n.l configure -text \"Network node object name\"; .a.n.e configure -state normal; .a.o.l configure -text \"Network nodes parent object\" }" );
		cmd( "ttk::radiobutton .a.c.b.f -text Link -width 6 -variable v_type -value 1 -command { write_any .a.d.e curl; .a.n.l configure -text \"(unused)\"; write_any .a.n.e \"\"; .a.n.e configure -state disabled; .a.o.l configure -text \"Network node object\" }" );
		cmd( "bind .a.c.b.e <Return> { focus .a.d.e; .a.d.e selection range 0 end }" );
		cmd( "bind .a.c.b.f <Return> { focus .a.d.e; .a.d.e selection range 0 end }" );
		cmd( "pack .a.c.b.e .a.c.b.f -side left" );

		cmd( "pack .a.c.l .a.c.b" );

		cmd( "ttk::frame .a.d" );
		cmd( "ttk::label .a.d.l -text \"Pointer to return the element drawn\"" );
		cmd( "ttk::entry .a.d.e -width 6 -textvariable v_obj0 -justify center" );
		cmd( "bind .a.d.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.d.l .a.d.e" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l" );			// network name or unused
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l" );			// parent object or node object
		cmd( "ttk::entry .a.o.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.c .a.d .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#RNDDRAW_NODE } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( ".a.c.b.e invoke" );
		cmd( "focus .a.d.e" );
		cmd( ".a.d.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "set choice $v_type" );
		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \"$v_obj0 = \" }" );

		if ( choice == 0 )
		{
			cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"RNDDRAW_NODE(\\\"$v_label\\\")\" }" );
			cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"RNDDRAW_NODES($v_obj, \\\"$v_label\\\")\" }" );
		}
		else
		{
			cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"RNDDRAW_LINK()\" }" );
			cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"RNDDRAW_LINKS($v_obj)\" }" );
		}

		cmd( "if { $v_obj0 != \"\" } { .f.t.t insert insert \";\" }" );
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// delete a network, node or link
	if ( choice == 85 )
	{
		cmd( "set v_type 0" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'DELETE_NET/NODE/LINK' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Delete a\"" );

		cmd( "ttk::frame .a.c.b -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.c.b.e -text Network -width 6 -variable v_type -value 0 -command { .a.n.l configure -text \"Network node object name\"; .a.n.e configure -state normal; .a.o.l configure -text \"Network nodes parent object\"; write_any .a.o.e p }" );
		cmd( "ttk::radiobutton .a.c.b.f -text Node -width 6 -variable v_type -value 1 -command { .a.n.l configure -text \"(unused)\"; write_any .a.n.e \"\"; .a.n.e configure -state disabled; .a.o.l configure -text \"Node object\"; write_any .a.o.e p }" );
		cmd( "ttk::radiobutton .a.c.b.g -text Link -width 6 -variable v_type -value 2 -command { .a.n.l configure -text \"(unused)\"; write_any .a.n.e \"\"; .a.n.e configure -state disabled; .a.o.l configure -text \"Link pointer\"; write_any .a.o.e curl }" );
		cmd( "bind .a.c.b.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "bind .a.c.b.f <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "bind .a.c.b.g <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.c.b.e .a.c.b.f .a.c.b.g -side left" );

		cmd( "pack .a.c.l .a.c.b" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l" );			// object name or unused
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l" );			// parent object or node object
		cmd( "ttk::entry .a.o.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.c .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#DELETE_NET } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( ".a.c.b.e invoke" );
		cmd( "focus .a.n.e" );
		cmd( ".a.n.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "set choice $v_type" );

		switch ( choice )
		{
			case 0:
				cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"DELETE_NET(\\\"$v_label\\\");\" }" );
				cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"DELETE_NETS($v_obj, \\\"$v_label\\\");\" }" );
				break;

			case 1:
				cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"DELETE_NODE();\" }" );
				cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"DELETE_NODES($v_obj);\" }" );
				break;

			case 2:
				cmd( ".f.t.t insert insert \"DELETE_LINK($v_obj);\"" );
				break;

			default:
				break;
		}
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// statistics about a network or node
	if ( choice == 86 )
	{
		cmd( "set v_type 0" );
		cmd( "set v_label \"\"" );
		cmd( "set v_obj p" );

		cmd( "newtop .a \"Insert 'STAT_NET/NODE' Command\" { set choice 2 }" );

		cmd( "ttk::frame .a.c" );
		cmd( "ttk::label .a.c.l -text \"Get statistics from a\"" );

		cmd( "ttk::frame .a.c.b -borderwidth 1 -relief solid" );
		cmd( "ttk::radiobutton .a.c.b.e -text Network -width 6 -variable v_type -value 0 -command { .a.n.l configure -text \"Network node object name\"; .a.n.e configure -state normal; .a.o.l configure -text \"Network nodes parent object\"; write_any .a.o.e p }" );
		cmd( "ttk::radiobutton .a.c.b.f -text Node -width 6 -variable v_type -value 1 -command { .a.n.l configure -text \"(unused)\"; write_any .a.n.e \"\"; .a.n.e configure -state disabled; .a.o.l configure -text \"Node object\"; write_any .a.o.e p }" );
		cmd( "bind .a.c.b.e <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "bind .a.c.b.f <Return> { focus .a.n.e; .a.n.e selection range 0 end }" );
		cmd( "pack .a.c.b.e .a.c.b.f -side left" );

		cmd( "pack .a.c.l .a.c.b" );

		cmd( "ttk::frame .a.n" );
		cmd( "ttk::label .a.n.l" );			// object name or unused
		cmd( "ttk::entry .a.n.e -width 25 -textvariable v_label -justify center" );
		cmd( "bind .a.n.e <Return> { focus .a.o.e; .a.o.e selection range 0 end }" );
		cmd( "pack .a.n.l .a.n.e" );

		cmd( "ttk::frame .a.o" );
		cmd( "ttk::label .a.o.l" );			// parent object or node object
		cmd( "ttk::entry .a.o.e -width 6 -textvariable v_obj -justify center" );
		cmd( "bind .a.o.e <Return> { focus .a.f.ok }" );
		cmd( "pack .a.o.l .a.o.e" );

		cmd( "pack .a.c .a.n .a.o -padx 5 -pady 5" );

		cmd( "okhelpcancel .a f { set choice 1 } { LsdHelp LSD_macros.html#STAT_NET } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( ".a.c.b.e invoke" );
		cmd( "focus .a.n.e" );
		cmd( ".a.n.e selection range 0 end" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 2 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set a [ .f.t.t index insert ]" );

		cmd( "set choice $v_type" );

		if ( choice == 0 )
		{
			cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"STAT_NET(\\\"$v_label\\\");\" }" );
			cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"STAT_NETS($v_obj, \\\"$v_label\\\");\" }" );
		}
		else
		{
				cmd( "if { $v_obj == \"p\" } { .f.t.t insert insert \"STAT_NODE();\" }" );
				cmd( "if { $v_obj != \"p\" } { .f.t.t insert insert \"STAT_NODES($v_obj);\" }" );
		}
		cmd( ".f.t.t see insert" );

		recolor = true;		// trigger recoloring
		goto loop;
	}

	// find matching parenthesis
	if ( choice == 32 )
	{
		cmd( "set sym [ .f.t.t get insert ]" );
		cmd( "if { [ string compare $sym \"\\(\" ] != 0 } { \
				if { [ string compare $sym \"\\)\"] != 0 } { \
					set choice 0 \
				} { \
					set num -1; \
					set direction -backwards; \
					set fsign +; \
					set sign \"\"; \
					set terminal \"1.0\" \
				} \
			} { \
				set num 1; \
				set direction -forwards; \
				set fsign -; \
				set sign +; \
				set terminal end \
			}" );

		if ( choice == 0 )
			goto loop;

		cmd( "set cur [ .f.t.t index insert ]" );
		cmd( ".f.t.t tag add sel $cur \"$cur + 1char\"" );
		if ( num > 0 )
			cmd( "set cur [.f.t.t index \"insert + 1 char\"]" );

		while ( num != 0 && choice != 0 )
		{
			cmd( "set a [ .f.t.t search $direction \"\\(\" $cur $terminal ]" );
			cmd( "if { $a == \"\" } { set a [ .f.t.t index $terminal ] }" );
			cmd( "set b [ .f.t.t search $direction \"\\)\" $cur $terminal ]" );
			cmd( "if { $b == \"\" } { set b [ .f.t.t index $terminal ] }" );
			cmd( "if { $a == $b } { set choice 0 }" );
			if ( choice == 0 )
				goto loop;
			if ( num > 0 )
				cmd( "if [ .f.t.t compare $a < $b ] { set num [ expr $num + 1 ]; set cur [ .f.t.t index \"$a+1char\" ] } { set num [ expr $num - 1 ]; set cur [ .f.t.t index \"$b+1char\" ] }" );
			else
				cmd( "if [ .f.t.t compare $a > $b ] { set num [ expr $num + 1 ]; set cur [ .f.t.t index $a ] } { set num [ expr $num - 1 ]; set cur [ .f.t.t index $b ] }" );
		}

		choice = 0;

		cmd( " if { ! [ string compare $sign \"+\" ] } { .f.t.t tag add sel \"$cur - 1char\" $cur ; set num 1 } { .f.t.t tag add sel $cur \"$cur + 1char\"; set num 0 }" );

		cmd( ".f.t.t see $cur" );
		goto loop;
	}

	// select a model
	if ( choice == 33 )
	{
		cmd( "destroytop .mm" );					// close compilation results, if open

		Tcl_LinkVar( inter, "choiceSM", ( char * ) & num, TCL_LINK_INT );
		num = 0;

		cmd( "showmodel $groupdir" );

		while ( num == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .l" );
		cmd( "bind .f.t.t <Enter> { }" );
		cmd( "focustop .f.t.t" );

		choice = num;
		Tcl_UnlinkVar( inter, "choiceSM" );

		if ( choice == 2 || choice == 0 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "set groupdir [ lindex $lrn 0 ]" );	// the group dir is the same for every element
		if ( choice == 14 )
			goto loop; 								// create a new model/group

		cmd( "set modelDir [ lindex $ldn $result ]" );
		cmd( "set dirname $modelDir" );

		load_model_info( ( char * ) Tcl_GetVar( inter, "modelDir", 0 ) );

		cmd( ".f.hea.info.grp.dat conf -text \"$modelGroup\"" );
		cmd( ".f.hea.info.mod.dat conf -text \"$modelName\"" );
		cmd( ".f.hea.info.ver.dat conf -text \"$modelVersion\"" );

		cmd( ".m.file entryconf 2 -state normal" );
		cmd( ".m.file entryconf 3 -state normal" );
		cmd( ".m.model entryconf 0 -state normal" );
		cmd( ".m.model entryconf 1 -state normal" );
		cmd( ".m.model entryconf 2 -state normal" );
		cmd( ".m.model entryconf 3 -state normal" );
		cmd( ".m.model entryconf 4 -state normal" );
		cmd( ".m.model entryconf 6 -state normal" );
		cmd( ".m.model entryconf 7 -state normal" );
		cmd( ".m.model entryconf 8 -state normal" );
		cmd( ".m.model entryconf 9 -state normal" );
		cmd( ".m.model entryconf 10 -state normal" );
		cmd( ".m.model entryconf 12 -state normal" );

		choice = 50;								// load description file
		goto loop;
	}

	// create a new version of the current model
	if ( choice == 41 )
	{
		cmd( "set oldModelName $modelName" );
		cmd( "set oldModelVersion $modelVersion" );
		cmd( "set mname $modelName" );
		cmd( "set mver $modelVersion" );
		cmd( "set mdir $dirname" );

		cmd( "newtop .a \"Save Model As...\" { set choice 2 }" );

		cmd( "ttk::frame .a.tit" );
		cmd( "ttk::label .a.tit.l -text \"Original model:\"" );

		cmd( "ttk::frame .a.tit.n" );
		cmd( "ttk::label .a.tit.n.n -style hl.TLabel -text \"$modelName\"" );
		cmd( "ttk::label .a.tit.n.l1 -text \"( version\"" );
		cmd( "ttk::label .a.tit.n.v -style hl.TLabel -text \"$modelVersion\"" );
		cmd( "ttk::label .a.tit.n.l2 -text \")\"" );
		cmd( "pack .a.tit.n.n .a.tit.n.l1 .a.tit.n.v .a.tit.n.l2 -side left" );

		cmd( "pack .a.tit.l .a.tit.n" );

		cmd( "ttk::frame .a.mname" );
		cmd( "ttk::label .a.mname.l -text \"New model name\"" );
		cmd( "ttk::entry .a.mname.e -width 25 -textvariable mname -justify center" );
		cmd( "pack .a.mname.l .a.mname.e" );

		cmd( "ttk::frame .a.mver" );
		cmd( "ttk::label .a.mver.l -text \"Version\"" );
		cmd( "ttk::entry .a.mver.e -width 10 -textvariable mver -justify center" );
		cmd( "pack .a.mver.l .a.mver.e" );

		cmd( "ttk::frame .a.mdir" );
		cmd( "ttk::label .a.mdir.l -text \"New (non-existing) directory name\"" );
		cmd( "ttk::entry .a.mdir.e -width 35 -textvariable mdir -justify center" );
		cmd( "pack .a.mdir.l .a.mdir.e" );

		cmd( "pack .a.tit .a.mname .a.mver .a.mdir -padx 5 -pady 5" );

		cmd( "okhelpcancel .a b { set choice 1 } { LsdHelp LMM.html#copy } { set choice 2 }" );
		cmd( "bind .a.mname.e <Return> { focus .a.mver.e; .a.mver.e selection range 0 end }" );
		cmd( "bind .a.mver.e <Return> { focus .a.mdir.e; .a.mdir.e selection range 0 end }" );
		cmd( "bind .a.mdir.e <Return> { focus .a.b.ok }" );

		cmd( "showtop .a" );
		cmd( ".a.mname.e selection range 0 end" );
		cmd( "focus .a.mname.e" );

		loop_copy:

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		if ( choice == 2 )
		{
			cmd( "destroytop .a" );
			cmd( "set modelName $oldModelName" );
			cmd( "set modelVersion $oldModelVersion" );
			choice = 0;
			goto loop;
		}

		cmd( "if { [ llength [ split $mdir ] ] > 1 } { set choice -1 }" );
		if ( choice == -1 )
		{
			cmd( "ttk::messageBox -parent .a -type ok -title Error -icon error -message \"Space in path\" -detail \"Directory name must not contain spaces, please try a new name.\"" );
			cmd( "focus .a.mdir.e" );
			cmd( ".a.mdir.e selection range 0 end" );
			goto loop_copy;
		}

		// control for existing directory
		cmd( "if [ file exists \"$mdir\" ] { ttk::messageBox -parent .a -type ok -title Error -icon error -message \"Cannot create directory\" -detail \"$groupdir/$mdir\\n\\nPossibly there is already such a directory, please try a new directory.\"; set choice 3 }" );
		if ( choice == 3 )
		{
			cmd( "focus .a.mdir.e" );
			cmd( ".a.mdir.e selection range 0 end" );
			goto loop_copy;
		}

		// control for an existing model with the same name AND same version
		cmd( "set dir [ glob -nocomplain * ]" );
		cmd( "set num [ llength $dir ]" );
		strcpy( str, " " );
		for ( i = 0; i < num && choice != 3; ++i )
		{
			cmd( "if [ file isdirectory [ lindex $dir %d ] ] { set curdir [ lindex $dir %i ] } { set curdir ___ }", i, i );
			s = ( char * ) Tcl_GetVar( inter, "curdir", 0 );
			strncpy( str, s, 499 );

			// check for invalid directories (LSD managed)
			for ( found = false, j = 0; j < LSD_DIR_NUM; ++j )
				if ( ! strcmp( str, lsd_dir[ j ] ) )
					found = true;

			if ( ! found )
			{
				if ( ! load_model_info( str ) )
					cmd( "set modelName $curdir; set modelVersion \"1.0\"" );

				cmd( "set comp [ string compare $modelName $mname ]" );
				cmd( "set comp1 [ string compare $modelVersion $mver ]" );
				cmd( "if { $comp == 0 && $comp1 == 0 } { set choice 3 }" );
			}
		}

		if ( choice == 3 )
		{
			cmd( "ttk::messageBox -parent .a -type ok -title Error -icon error -message \"Model already exists\" -detail \"Cannot create the new model '$mname' (ver. $mver) because it already exists (directory: $curdir).\"" );
			cmd( ".a.mname.e selection range 0 end" );
			cmd( "focus .a.mname.e" );
			goto loop_copy;
		}

		cmd( "destroytop .a" );

		// create a new copycat model
		cmd( "file copy \"$dirname\" \"$mdir\"" );
		cmd( "set dirname \"$mdir\"" );
		cmd( "set modelDir \"$mdir\"" );
		cmd( "set modelName \"$mname\"" );
		cmd( "set modelVersion \"$mver\"" );
		cmd( "set modelDate \"\"" );
		cmd( ".f.hea.info.mod.dat conf -text \"$modelName\"" );
		cmd( ".f.hea.info.ver.dat conf -text \"$modelVersion\"" );

		// create the model info file
		update_model_info( );

		cmd( "ttk::messageBox -parent . -type ok -title \"Save Model As...\" -icon info -message \"Model '$modelName' created\" -detail \"Version: $modelVersion\nDirectory: $modelDir\"" );

		choice = 49;
		goto loop;
	}

	// increase indent
	if ( choice == 42 )
	{
		cmd( "set in [ .f.t.t tag range sel ]" );
		cmd( "if { [ string length $in ] == 0 } { set choice 0 } { set choice 1 }" );
		if ( choice == 0 )
			goto loop;

		cmd( "scan $in \"%%d.%%d %%d.%%d\" line1 col1 line2 col2" );
		cmd( "set num $line1" );
		i = num;
		cmd( "set num $line2" );

		for ( ; i <= num; ++i )
			cmd( ".f.t.t insert %d.0 \" \"", i );

		choice = 0;
		goto loop;
	}

	// decrease indent
	if ( choice == 43 )
	{
		cmd( "set in [ .f.t.t tag range sel ]" );
		cmd( "if { [ string length $in ] == 0 } { set choice 0 } { set choice 1 }" );
		if ( choice == 0 )
			goto loop;

		cmd( "scan $in \"%%d.%%d %%d.%%d\" line1 col1 line2 col2" );
		cmd( "set num $line1" );
		i = num;
		cmd( "set num $line2" );

		for ( ; i <= num; ++i )
		{
			cmd( "set c [.f.t.t get %d.0]", i );
			cmd( "if { $c == \" \" } { set choice 1 } { set choice 0 }" );
			if ( choice == 1 )
				cmd( ".f.t.t delete %d.0 ", i );
		}

		choice = 0;
		goto loop;
	}

	// show and edit model info
	if ( choice == 44 )
	{
		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );
		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
			choice = 0;
			goto loop;
		}

		if ( ! load_model_info( ( char * ) Tcl_GetVar( inter, "modelDir", 0 ) ) )
			update_model_info( );			// recreate the model info file

		cmd( "set mname $modelName" );
		cmd( "set mver $modelVersion" );
		cmd( "set mdate $modelDate" );

		cmd( "set complete_dir [ file nativename [ file join [ pwd ] \"$modelDir\" ] ]" );

		s = get_fun_name( str );
		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "set eqname \"\"" );
			cmd( "set edate \"\"" );
		}
		else
		{
			cmd( "set eqname \"%s\"", s );
			cmd( "if [ file exists \"$modelDir/$eqname\" ] { set edate \"[ clock format [ file mtime \"$modelDir/$eqname\" ] -format \"$DATE_FMT\" ]\" } { set edate \"\" }" );
		}

		cmd( "newtop .a \"Model Info\" { set choice 2 }" );

		cmd( "ttk::frame .a.mname" );
		cmd( "ttk::label .a.mname.l -text \"Model name\"" );
		cmd( "ttk::entry .a.mname.e -width 25 -textvariable mname -justify center" );
		cmd( "pack .a.mname.l .a.mname.e" );

		cmd( "ttk::frame .a.mver" );
		cmd( "ttk::label .a.mver.l -text \"Version\"" );
		cmd( "ttk::entry .a.mver.e -width 10 -textvariable mver -justify center" );
		cmd( "pack .a.mver.l .a.mver.e" );

		cmd( "ttk::frame .a.mdir" );
		cmd( "ttk::label .a.mdir.l -text \"Model home directory\"" );
		cmd( "ttk::entry .a.mdir.e -width 35 -state disabled -textvariable complete_dir -justify center" );
		cmd( "pack .a.mdir.l .a.mdir.e" );

		cmd( "ttk::frame .a.date" );
		cmd( "ttk::label .a.date.l -text \"Creation date\"" );
		cmd( "ttk::entry .a.date.e -width 20 -textvariable mdate -justify center" );
		cmd( "pack .a.date.l .a.date.e" );

		cmd( "ttk::frame .a.edate" );
		cmd( "ttk::label .a.edate.l -text \"Modification date (equations)\"" );
		cmd( "ttk::entry .a.edate.e -width 20 -state disabled -textvariable edate -justify center" );
		cmd( "pack .a.edate.l .a.edate.e" );

		cmd( "pack .a.mname .a.mver .a.mdir .a.date .a.edate -padx 5 -pady 5" );

		cmd( "okcancel .a b { set choice 1 } { set choice 2 }" );
		cmd( "bind .a.mname.e <Return> { focus .a.mver.e; .a.mver.e selection range 0 end }" );
		cmd( "bind .a.mver.e <Return> { focus .a.b.ok }" );

		cmd( "showtop .a" );
		cmd( "mousewarpto .a.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .a" );

		if ( choice == 1 )
		{
			cmd( "set modelName $mname" );
			cmd( "set modelVersion $mver" );
			cmd( "if { [ string is print -strict $mdate ] } { set modelDate \"$mdate\" } { set modelDate \"[ current_date ]\" }" );

			cmd( ".f.hea.info.mod.dat conf -text \"$modelName\"" );
			cmd( ".f.hea.info.ver.dat conf -text \"$modelVersion\"" );

			// update the model info file
			update_model_info( );
		}

		choice = 0;
		goto loop;
	}

	// create a new file
	if ( choice == 39 )
	{
		cmd( ".f.t.t delete 1.0 end" );
		cmd( "set before [ .f.t.t get 1.0 end ]" );
		cmd( "set filename newfile.txt" );
		cmd( "set dirname [ pwd ]" );
		cmd( ".f.t.t mark set insert 1.0" );
		cmd( ".f.hea.info.file.dat conf -text \"$filename\"" );
		cmd( "unset -nocomplain ud udi rd rdi" );
		cmd( "lappend ud [ .f.t.t get 0.0 end ]" );
		cmd( "lappend udi [ .f.t.t index insert ]" );

		choice = 0;
		goto loop;
	}

	// create the makefile
	if ( choice == 46 || choice == 49 )
	{
		make_makefile( );

		if ( choice == 46 )
			choice = 0;		//just create the makefile
		if ( choice == 49 )	//after this show the description file (and a model is created)
			choice = 50;

		goto loop;
	}

	// System Options
	if ( choice == 47 )
	{
		cmd( "set choice [ file exists \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\" ]" );
		if ( choice == 1 )
		{
			cmd( "set f [ open \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\" r ]" );
			cmd( "set a [ read -nonewline $f ]" );
			cmd( "close $f" );
			choice = 0;
		}
		else
			cmd( "set a \"\"" );

		cmd( "newtop .l \"System Options\" { set choice 2 }" );

		cmd( "ttk::frame .l.t" );
		cmd( "ttk::scrollbar .l.t.yscroll -command \".l.t.text yview\"" );
		cmd( "ttk::text .l.t.text -wrap word -width 70 -height 20 -yscrollcommand \".l.t.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
		cmd( ".l.t.text insert end $a" );
		cmd( "pack .l.t.yscroll -side right -fill y" );
		cmd( "pack .l.t.text" );
		cmd( "mouse_wheel .l.t.text" );

		cmd( "ttk::frame .l.d" );
		cmd( "ttk::label .l.d.msg -text [ check_sys_opt ] -style hl.TLabel" );
		cmd( "pack .l.d.msg" );

		cmd( "pack .l.t .l.d" );

		cmd( "okXhelpcancel .l b Default { \
				.l.t.text delete 1.0 end; \
				if [ file exists \"$RootLsd/$LsdSrc/system_options-$CurPlatform.txt\" ] { \
					set file [ open \"$RootLsd/$LsdSrc/system_options-$CurPlatform.txt\" r ]; \
					set a [ read -nonewline $file ]; \
					close $file \
				} { \
					set a \"File $DefaultSysOpt is missing\nPlease reinstall LSD\" \
				}; \
				.l.t.text insert end \"# LSD options\n\"; \
				.l.t.text insert end \"LSDROOT=$RootLsd\n\"; \
				.l.t.text insert end \"SRC=$LsdSrc\n\n\"; \
				.l.t.text insert end \"$a\"; \
				.l.d.msg configure -text \"\"; \
				set objs [ glob -nocomplain -directory \"$RootLsd/$LsdSrc\" *.o *.gch ]; \
				foreach i $objs { \
					catch { \
						file delete -force \"$i\" \
					} \
				}; \
				if { ! [ string equal -nocase \"$modelDir\" \"$RootLsd\" ] } { \
					set objs [ glob -nocomplain -directory \"$modelDir\" *.o src makefile* makemessage.txt lsd* *.exe* *.app ]; \
					foreach i $objs { \
						catch { \
							file delete -force \"$i\" \
						} \
					} \
				} \
			} { set choice 1 } { LsdHelp LMM.html#compilation_options } { set choice 2 }" );

		cmd( "showtop .l" );
		cmd( "focus .l.t.text" );
		cmd( "mousewarpto .l.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		if ( choice == 1 )
		{
			cmd( "set f [ open \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\" w ]" );
			cmd( "puts -nonewline $f [ .l.t.text get 1.0 end ]" );
			cmd( "close $f" );
			choice = 46; 	//go to create makefile
		}
		else
			choice = 0;

		cmd( "destroytop .l" );

		goto loop;
	}

	// Model Options
	if ( choice == 48 )
	{
		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );
		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
			choice = 0;
			goto loop;
		}

		s = get_fun_name( str );
		if ( s == NULL || ! strcmp( s, "" ) )
			check_option_files( );

		cmd( "cd \"$modelDir\"" );

		cmd( "set b \"%s\"", s );
		cmd( "set f [ open $MODEL_OPTIONS r ]" );
		cmd( "set a [ read -nonewline $f ]" );
		cmd( "close $f" );

		cmd( "set gcc_conf \"# LSD options\nTARGET=$DefaultExe\nFUN=[ file rootname \"$b\" ]\n\n# Additional model files\nFUN_EXTRA=\n\n# Compiler options\nSWITCH_CC=\"" );
		cmd( "set gcc_deb_nopt \"-O0\"" );
		cmd( "set gcc_deb \"$gcc_conf$gcc_deb_nopt -ggdb3\nSWITCH_CC_LNK=\"" );
		cmd( "set gcc_opt \"$gcc_conf -O3\nSWITCH_CC_LNK=\"" );

		cmd( "set pos [ string first \"SWITCH_CC=\" $a ]" );
		cmd( "if { $pos == -1 } { \
				set choice 0 \
			} { \
				if { [ string first \" -g\" $a $pos ] == -1 } { \
					set debug 0 \
				} { \
					set debug 1 \
				} \
			}" );

		cmd( "newtop .l \"Model Options\" { set choice 2 }" );

		cmd( "ttk::frame .l.t" );
		cmd( "ttk::scrollbar .l.t.yscroll -command \".l.t.text yview\"" );
		cmd( "ttk::text .l.t.text -wrap word -width 70 -height 16 -yscrollcommand \".l.t.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
		cmd( ".l.t.text insert end $a" );
		cmd( "pack .l.t.yscroll -side right -fill y" );
		cmd( "pack .l.t.text" );
		cmd( "pack .l.t" );
		cmd( "mouse_wheel .l.t.text" );

		cmd( "ttk::frame .l.pad" );
		cmd( "pack .l.pad -pady 5" );

		cmd( "ttk::frame .l.d" );

		cmd( "ttk::frame .l.d.opt" );
		cmd( "ttk::checkbutton .l.d.opt.debug -text Debug -variable debug -command { \
				set a [.l.t.text get 1.0 end]; \
				set pos [ string first \"SWITCH_CC=\" $a ]; \
				if { $pos == -1 } { \
					.l.d.opt.def invoke \
				} else { \
					if $debug { \
						if { [ string first \" -g\" $a $pos ] != -1 } { \
							return \
						}; \
						set pos1 [ string first \"\n\" $a $pos ]; \
						if { $pos1 == -1 } { \
							set a \"$a -ggdb3\" \
						} else { \
							set a [ string replace $a $pos1 $pos1 \" -ggdb3\n\" ] \
						}; \
						set pos2 [ string first \"-O\" $a $pos ]; \
						if { $pos2 != -1 } { \
							set a [ string replace $a $pos2 $pos2+2 $gcc_deb_nopt ] \
						} \
					} else { \
						set pos1 [ string first \" -ggdb3\" $a $pos ]; \
						if { $pos1 != -1 } { \
							set a [ string replace $a $pos1 $pos1+6 \"\" ] \
						} else { \
							set pos1 [ string first \" -ggdb\" $a $pos ]; \
							if { $pos1 != -1 } { \
								set a [ string replace $a $pos1 $pos1+5 \"\" ] \
							} else { \
								set pos1 [ string first \" -g\" $a $pos ]; \
								if { $pos1 != -1 } { \
									set a [ string replace $a $pos1 $pos1+2 \"\" ] \
								} else { \
									return \
								} \
							} \
						}; \
						set pos2 [ string first \"-O\" $a $pos ]; \
						if { $pos2 != -1 } { \
							set a [ string replace $a $pos2 $pos2+2 \"-O3\" ] \
						} \
					}; \
					.l.t.text delete 1.0 end; \
					.l.t.text insert end \"[ string trim $a ]\n\" \
				} \
			}" );
		cmd( "ttk::button .l.d.opt.ext -width $butWid -text \"Add Extra\" -command { \
				set a [.l.t.text get 1.0 end]; \
				set pos [ string first \"FUN_EXTRA=\" $a ]; \
				if { $pos == -1 } { \
					.l.d.opt.def invoke; \
					set a [.l.t.text get 1.0 end]; \
					set pos [ string first \"FUN_EXTRA=\" $a ]; \
				}; \
				set fun_extra [ tk_getOpenFile -parent .l -title \"Select Additional Source Files\" -multiple yes -initialdir \"$modelDir\" -filetypes { { {C++ header files} {.h .hpp .h++} } { {C++ source files} {.c .cpp .c++} } { {All files} {*} } } ]; \
				if { $fun_extra == \"\" } { \
					return \
				}; \
				set extra_files [ list ]; \
				foreach x $fun_extra { \
					set dirlen [ string length $modelDir ]; \
					if { [ string equal -length $dirlen $modelDir $x ] } { \
						if { [ string index $x $dirlen ] == \"/\" || [ string index $x $dirlen ] == \"\\\\\" } {  \
							incr dirlen; \
						}; \
						lappend extra_files [ string range $x $dirlen end ] \
					} else { \
						lappend extra_files $x \
					} \
				}; \
				set pos1 [ string first \"\n\" $a $pos ]; \
				if { $pos1 == -1 } { \
					set a \"$a $extra_files\" \
				} else { \
					set a [ string replace $a $pos1 $pos1 \" $extra_files\n\" ] \
				}; \
				.l.t.text delete 1.0 end; \
				.l.t.text insert end \"[ string trim $a ]\n\" \
			}" );
		cmd( "ttk::button .l.d.opt.def -width $butWid -text \"Default\" -command { \
				if { $debug == 0 } { \
					set default \"$gcc_opt\" \
				} else { \
					set default \"$gcc_deb\" \
				}; \
				.l.t.text delete 1.0 end; \
				.l.t.text insert end \"$default\" \
			}" );
		cmd( "ttk::button .l.d.opt.cle -width $butWid -text \"Clean Obj.\" -command { \
				set objs [ glob -nocomplain -directory \"$RootLsd/$LsdSrc\" *.o *.gch ]; \
				foreach i $objs { \
					catch { \
						file delete -force \"$i\" \
					} \
				}; \
				set objs [ glob -nocomplain -directory \"$modelDir\" *.o src break.gdb makefile* makemessage.txt make.bat elements.txt lsd* *.exe *.app *.bak *.err ]; \
				foreach i $objs { \
					catch { \
						file delete -force \"$i\" \
					} \
				} \
			}" );
		cmd( "pack .l.d.opt.debug .l.d.opt.ext .l.d.opt.def .l.d.opt.cle -padx $butSpc -side left" );

		cmd( "pack .l.d.opt -padx $butPad" );

		cmd( "pack .l.d -anchor e" );

		cmd( "okhelpcancel .l b { set choice 1 } { LsdHelp LMM.html#model_options } { set choice 2 }" );

		cmd( "bind .l.d.opt.ext <KeyPress-Return> { .l.d.opt.ext invoke }" );
		cmd( "bind .l.d.opt.def <KeyPress-Return> { .l.d.opt.def invoke }" );
		cmd( "bind .l.d.opt.cle <KeyPress-Return> { .l.d.opt.cle invoke }" );


		cmd( "showtop .l" );
		cmd( "focus .l.t.text" );
		cmd( "mousewarpto .l.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		if ( choice == 1 )
		{
			cmd( "set f [ open $MODEL_OPTIONS w ]" );
			cmd( "puts -nonewline $f [.l.t.text get 1.0 end]" );
			cmd( "close $f" );
			choice = 46;		//go to create makefile
		}
		else
			choice = 0;

		cmd( "cd \"$RootLsd\"" );

		cmd( "destroytop .l" );

		goto loop;
	}

	// start diff with model selection
	if ( choice == 61 )
	{
		cmd( "destroytop .mm" );	// close compilation results, if open

		cmd( "set eqname \"\"" );
		cmd( "set complete_dir \"\"" );

		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );
		if ( s != NULL && strcmp( s, "" ) )
		{
			if ( ! load_model_info( ( char * ) Tcl_GetVar( inter, "modelDir", 0 ) ) )
				update_model_info( );			// recreate the model info file

			s = get_fun_name( str );
			if ( s != NULL && strcmp( s, "" ) )
			{
				cmd( "set eqname \"%s\"", s );
				cmd( "set complete_dir [ file nativename [ file join [ pwd ] \"$modelDir\" ] ]" );
			}
		}

		cmd( "choose_models $complete_dir $eqname" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		if ( choice == -1 )
		{
			choice = 0;
			goto loop;
		}

		cmd( "open_diff [ file join \"$d1\" \"$f1\" ] [ file join \"$d2\" \"$f2\" ] \"$f1\" \"$f2\"" );

		choice = 0;
		goto loop;
	}

	// LMM options
	if ( choice == 60 )
	{
		cmd( "updateTheme" );
		
		for ( i = 1; i <= LMM_OPTIONS_NUM; ++i )
		{
			cmd( "set temp_var%d \"$%s\"", i, lmm_options[ i - 1 ] );
			cmd( "set default_var%d \"%s\"", i, lmm_defaults[ i - 1 ] );
		}
		
		cmd( "set temp_var16 \"[ dict get $themeToName $temp_var16 ]\"" );
		cmd( "set default_var16 \"[ dict get $themeToName $default_var16 ]\"" );
		
		cmd( "newtop .a \"Options\" { set choice 2 }" );
		
		cmd( "ttk::frame .a.f" );

		cmd( "ttk::frame .a.f.c1" );					// column 1

		cmd( "ttk::frame .a.f.c1.num" );
		cmd( "ttk::label .a.f.c1.num.l -text \"System terminal\"" );
		cmd( "ttk::entry .a.f.c1.num.v -width 25 -textvariable temp_var1 -justify center" );
		cmd( "pack .a.f.c1.num.l .a.f.c1.num.v" );
		cmd( "bind .a.f.c1.num.v <Return> { focus .a.f.c1.num13.v; .a.f.c1.num13.v selection range 0 end }" );

		cmd( "ttk::frame .a.f.c1.num13" );
		cmd( "ttk::label .a.f.c1.num13.l -text \"Debugger\"" );
		cmd( "ttk::entry .a.f.c1.num13.v -width 25 -textvariable temp_var13 -justify center" );
		cmd( "pack .a.f.c1.num13.l .a.f.c1.num13.v" );
		cmd( "bind .a.f.c1.num13.v <Return> { focus .a.f.c1.num2.v; .a.f.c1.num2.v selection range 0 end }" );

		cmd( "ttk::frame .a.f.c1.num2" );
		cmd( "ttk::label .a.f.c1.num2.l -text \"HTML browser\"" );
		cmd( "ttk::entry .a.f.c1.num2.v -width 25 -textvariable temp_var2 -justify center" );
		cmd( "pack .a.f.c1.num2.l .a.f.c1.num2.v" );
		cmd( "bind .a.f.c1.num2.v <Return> { focus .a.f.c1.num4.v; .a.f.c1.num4.v selection range 0 end }" );

		cmd( "ttk::frame .a.f.c1.num4" );
		cmd( "ttk::label .a.f.c1.num4.l -text \"Tcl/Tk Wish\"" );
		cmd( "ttk::entry .a.f.c1.num4.v -width 25 -textvariable temp_var4 -justify center" );
		cmd( "pack .a.f.c1.num4.l .a.f.c1.num4.v" );
		cmd( "bind .a.f.c1.num4.v <Return> { focus .a.f.c1.num12.v; .a.f.c1.num12.v selection range 0 end }" );

		cmd( "ttk::frame .a.f.c1.num12" );
		cmd( "ttk::label .a.f.c1.num12.l -text \"New models subdirectory\"" );
		cmd( "ttk::entry .a.f.c1.num12.v -width 25 -textvariable temp_var12 -justify center" );
		cmd( "pack .a.f.c1.num12.l .a.f.c1.num12.v" );
		cmd( "bind .a.f.c1.num12.v <Return> { focus .a.f.c1.num5.v; .a.f.c1.num5.v selection range 0 end }" );

		cmd( "ttk::frame .a.f.c1.num5" );
		cmd( "ttk::label .a.f.c1.num5.l -text \"Source code subdirectory\"" );
		cmd( "ttk::entry .a.f.c1.num5.v -width 25 -textvariable temp_var5 -justify center" );
		cmd( "pack .a.f.c1.num5.l .a.f.c1.num5.v" );
		cmd( "bind .a.f.c1.num5.v <Return> { focus .a.f.c2.num16.v; .a.f.c2.num16.v selection range 0 end }" );

		cmd( "pack .a.f.c1.num .a.f.c1.num13 .a.f.c1.num2 .a.f.c1.num4 .a.f.c1.num12 .a.f.c1.num5 -padx 5 -pady 5" );

		cmd( "pack .a.f.c1 -padx 10 -side left" );
		
		cmd( "ttk::frame .a.f.c2" );					// column 2

		cmd( "ttk::frame .a.f.c2.num16" );
		cmd( "ttk::label .a.f.c2.num16.l -text \"Interface theme\"" );
		cmd( "ttk::combobox .a.f.c2.num16.v -justify center -values $themeNames -width 22 -validate focusout -validatecommand { set n %%P; if { $n in $themeNames } { set temp_var16 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $temp_var16; return 0 } } -invalidcommand { bell }" );
		cmd( "write_any .a.f.c2.num16.v $temp_var16" );
		cmd( "pack .a.f.c2.num16.l .a.f.c2.num16.v" );
		cmd( "bind .a.f.c2.num16.v <Return> { focus .a.f.c2.num3.f.v; .a.f.c2.num7.v selection range 0 end }" );
		
		cmd( "ttk::frame .a.f.c2.num3" );
		cmd( "ttk::label .a.f.c2.num3.l -text \"Font name and size\"" );
		cmd( "ttk::frame .a.f.c2.num3.f" );
		cmd( "ttk::combobox .a.f.c2.num3.f.v -justify center -values [ lsort -dictionary [ font families ] ] -width 15 -validate focusout -validatecommand { set n %%P; if { $n in [ font families ] } { set temp_var3 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $temp_var3; return 0 } } -invalidcommand { bell }" );
		cmd( "write_any .a.f.c2.num3.f.v $temp_var3" );
		cmd( "ttk::combobox .a.f.c2.num3.f.s -justify center -values [ list 4 6 8 9 10 11 12 14 18 24 32 48 60 ] -width 3 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 4 && $n <= 60 } { set temp_var6 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $temp_var6; return 0 } } -invalidcommand { bell }" );
		cmd( "write_any .a.f.c2.num3.f.s $temp_var6" );
		cmd( "pack .a.f.c2.num3.f.v .a.f.c2.num3.f.s -side left" );
		cmd( "pack .a.f.c2.num3.l .a.f.c2.num3.f" );
		cmd( "bind .a.f.c2.num3.f.v <Return> { focus .a.f.c2.num3.f.s; .a.f.c2.num3.f.s selection range 0 end }" );
		cmd( "bind .a.f.c2.num3.f.s <Return> { focus .a.f.c2.num7.v; .a.f.c2.num7.v selection range 0 end }" );

		cmd( "ttk::frame .a.f.c2.num7" );
		cmd( "ttk::label .a.f.c2.num7.l -text \"Tab size (characters)\"" );
		cmd( "ttk::spinbox .a.f.c2.num7.v -justify center -width 3 -from 1 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= 99 } { set temp_var7 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $temp_var7; return 0 } } -invalidcommand { bell }" );
		cmd( "write_any .a.f.c2.num7.v $temp_var7" );
		cmd( "pack .a.f.c2.num7.l .a.f.c2.num7.v" );
		cmd( "bind .a.f.c2.num7.v <Return> { focus .a.f.c2.num9.r.v3 }" );

		cmd( "ttk::frame .a.f.c2.num9" );
		cmd( "ttk::label .a.f.c2.num9.l -text \"Syntax highlights\"" );

		cmd( "ttk::frame .a.f.c2.num9.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .a.f.c2.num9.r.v1 -variable temp_var9 -value 0 -text None" );
		cmd( "ttk::radiobutton .a.f.c2.num9.r.v2 -variable temp_var9 -value 1 -text Partial" );
		cmd( "ttk::radiobutton .a.f.c2.num9.r.v3 -variable temp_var9 -value 2 -text Full" );
		cmd( "pack .a.f.c2.num9.r.v1 .a.f.c2.num9.r.v2 .a.f.c2.num9.r.v3 -side left" );

		cmd( "pack .a.f.c2.num9.l .a.f.c2.num9.r" );
		cmd( "bind .a.f.c2.num9.r.v1 <Return> { focus .a.f.c2.num8.v_num8 }" );
		cmd( "bind .a.f.c2.num9.r.v2 <Return> { focus .a.f.c2.num8.v_num8 }" );
		cmd( "bind .a.f.c2.num9.r.v3 <Return> { focus .a.f.c2.num8.v_num8 }" );

		cmd( "ttk::frame .a.f.c2.num8" );
		cmd( "ttk::checkbutton .a.f.c2.num8.v_num8 -variable temp_var8 -text \"Wrap text\"" );
		cmd( "ttk::checkbutton .a.f.c2.num8.v_num10 -variable temp_var10 -text \"Auto-hide on run\"" );
		cmd( "ttk::checkbutton .a.f.c2.num8.v_num11 -variable temp_var11 -text \"Show text file commands\"" );
		cmd( "ttk::checkbutton .a.f.c2.num8.v_num14 -variable temp_var14 -text \"Restore window positions\"" );
		cmd( "pack .a.f.c2.num8.v_num8 .a.f.c2.num8.v_num10 .a.f.c2.num8.v_num11 .a.f.c2.num8.v_num14 -anchor w" );
		cmd( "bind .a.f.c2.num8.v_num8 <Return> { focus .a.f.c2.num8.v_num10 }" );
		cmd( "bind .a.f.c2.num8.v_num10 <Return> { focus .a.f.c2.num8.v_num11 }" );
		cmd( "bind .a.f.c2.num8.v_num11 <Return> { focus .a.f.c2.num8.v_num14 }" );
		cmd( "bind .a.f.c2.num8.v_num14 <Return> { focus .a.b.ok }" );

		cmd( "pack .a.f.c2.num16 .a.f.c2.num3 .a.f.c2.num7 .a.f.c2.num9 .a.f.c2.num8 -padx 5 -pady 5" );

		cmd( "pack .a.f.c2 -padx 10 -side left" );
		
		cmd( "pack .a.f" );
		
		cmd( "proc set_defaults { } { \
				set ::temp_var1 \"$::default_var1\"; \
				set ::temp_var2 \"$::default_var2\"; \
				set ::temp_var3 \"$::default_var3\"; \
				set ::temp_var4 \"$::default_var4\"; \
				set ::temp_var5 \"$::default_var5\"; \
				set ::temp_var6 \"$::default_var6\"; \
				write_any .a.f.c2.num7.v \"$::default_var7\"; \
				set ::temp_var8 \"$::default_var8\"; \
				set ::temp_var9 \"$::default_var9\"; \
				set ::temp_var10 \"$::default_var10\"; \
				set ::temp_var11 \"$::default_var11\"; \
				set ::temp_var12 \"$::default_var12\"; \
				set ::temp_var13 \"$::default_var13\"; \
				set ::temp_var14 \"$::default_var14\"; \
				set ::temp_var15 \"$::default_var15\"; \
				set ::temp_var16 \"$::default_var16\" \
			}" );

		cmd( "okXhelpcancel .a b Default { set_defaults } { set choice 1 } { LsdHelp LMM.html#SystemOpt } { set choice 2 }" );

		cmd( "showtop .a" );
		cmd( "focus .a.f.c1.num.v" );
		cmd( ".a.f.c1.num.v selection range 0 end" );
		cmd( "mousewarpto .a.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set temp_var7 [ .a.f.c2.num7.v get ]" );

		cmd( "destroytop .a" );

		if ( choice == 1 )
		{
			cmd( "if { ! ( \"$temp_var3\" in [ font families ] ) } { set temp_var3 \"$fonttype\"; bell }" );
			cmd( "if { ! [ string is integer -strict $temp_var6 ] || $temp_var6 < 4 || $temp_var6 > 60 } { set temp_var6 $dim_character; bell }" );
			cmd( "if { ! [ string is integer -strict $temp_var7 ] || $temp_var7 < 1 || $temp_var7 > 99 } { set temp_var7 $tabsize; bell }" );
			cmd( "if { ! ( \"$temp_var16\" in $themeNames ) } { set temp_var16 \"$lsdTheme\"; bell } { set temp_var16 [ dict get $nameToTheme $temp_var16 ] }" );

			cmd( "if { $showFileCmds != $temp_var11 || $lsdTheme != $temp_var16 } { \
					ttk::messageBox -parent . -icon warning -title Warning -type ok -message \"LMM restart required\" -detail \"Please restart LMM for changes to be applied.\" \
				}" );

			for ( i = 1; i <= LMM_OPTIONS_NUM; ++i )
				cmd( "set %s \"$temp_var%d\"", lmm_options[ i - 1 ], i );

			update_lmm_options(  ); 				// update config file

			// adjust text styles and apply
			cmd( "ttk::style configure fixed.TText -font [ font create -family \"$fonttype\" -size $dim_character ]" );
			cmd( "ttk::style configure smallFixed.TText -font [ font create -family \"$fonttype\" -size $small_character ]" );
			cmd( "settab .f.t.t $tabsize fixed.TText" );	// adjust tabs size to font type/size
			cmd( "setwrap .f.t.t $wrap" );			// adjust text wrap
			recolor_all = true;
		}

		choice = 0;
		goto loop;
	}

	// generate the no window distribution
	if ( choice == 62 )
	{
		cmd( "if { \"[ check_sys_opt ]\" != \"\" } { if { [ ttk::messageBox -parent . -icon warning -title Warning -type yesno -default no -message \"Invalid system options detected\" -detail \"The current LSD configuration is invalid for your platform. To fix it, please use menu option 'Model>System Options', press the 'Default' button, and then 'OK'.\n\nDo you want to proceed anyway?\" ] == no } { set choice 0 } }" );
		if ( choice == 0 )
			goto loop;

		// copy the base LSD source files to distribution directory
		cmd( "if { ! [ file exists \"$modelDir/$LsdSrc\" ] } { file mkdir \"$modelDir/$LsdSrc\" }" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/lsdmain.cpp\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/common.cpp\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/file.cpp\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/nets.cpp\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/object.cpp\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/util.cpp\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/variab.cpp\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/check.h\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/common.h\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/decl.h\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/fun_head.h\" \"$modelDir/$LsdSrc\"" );
		cmd( "file copy -force \"$RootLsd/$LsdSrc/fun_head_fast.h\" \"$modelDir/$LsdSrc\"" );

		// copy Eigen library files if in use, just once to save time
		if( use_eigen( ) )
			cmd( "if { ! [ file exists \"$modelDir/$LsdSrc/Eigen\" ] } { file copy -force \"$RootLsd/$LsdSrc/Eigen\" \"$modelDir/$LsdSrc\" }" );

		// create makefileNW and compile a local machine version of lsdNW
		compile_run( false, true );

		choice = 0;
		goto loop;
	}

	// Adjust context menu for LSD macros
	if ( choice == 68 )
	{
		cmd( "destroy .v.i" );
		cmd( "ttk::menu .v.i -tearoff 0" );
		cmd( ".v.i add command -label \"EQUATION\" -command { set choice 25 } -accelerator Ctrl+E" );
		cmd( ".v.i add command -label \"V(...)\" -command { set choice 26 } -accelerator Ctrl+V" );
		cmd( ".v.i add command -label \"CYCLE(...)\" -command { set choice 27 } -accelerator Ctrl+C" );
		cmd( ".v.i add command -label \"SUM(...)\" -command { set choice 56 } -accelerator Ctrl+U" );
		cmd( ".v.i add command -label \"INCR(...)\" -command { set choice 40 } -accelerator Ctrl+I" );
		cmd( ".v.i add command -label \"MULT(...)\" -command { set choice 45 } -accelerator Ctrl+M" );
		cmd( ".v.i add command -label \"SEARCH(...)\" -command { set choice 55 } -accelerator Ctrl+A" );
		cmd( ".v.i add command -label \"SEARCH_CND(...)\" -command { set choice 30 } -accelerator Ctrl+S" );
		cmd( ".v.i add command -label \"SORT(...)\" -command { set choice 31 } -accelerator Ctrl+T" );
		cmd( ".v.i add command -label \"RNDDRAW(...)\" -command { set choice 54 } -accelerator Ctrl+N" );
		cmd( ".v.i add command -label \"WRITE(...)\" -command { set choice 29 } -accelerator Ctrl+W" );
		cmd( ".v.i add command -label \"ADDOBJ(...)\" -command { set choice 52 } -accelerator Ctrl+O" );
		cmd( ".v.i add command -label \"DELETE(...)\" -command { set choice 53 } -accelerator Ctrl+D" );
		cmd( ".v.i add command -label \"Network macros\" -command { set choice 72 } -accelerator Ctrl+K" );
		cmd( ".v.i add command -label \"Math functions\" -command { set choice 51 } -accelerator Ctrl+H" );
		choice = 0;
		goto loop;
	}

	// Adjust context menu for LSD C++
	if ( choice == 69 )
	{
		cmd( "destroy .v.i" );
		cmd( "ttk::menu .v.i -tearoff 0" );
		cmd( ".v.i add command -label \"LSD equation/function\" -command { set choice 25 } -accelerator Ctrl+E" );
		cmd( ".v.i add command -label \"cal(...)\" -command { set choice 26 } -accelerator Ctrl+V" );
		cmd( ".v.i add command -label \"for ( ; ; )\" -command { set choice 27 } -accelerator Ctrl+C" );
		cmd( ".v.i add command -label \"sum(...)\" -command { set choice 56 } -accelerator Ctrl+U" );
		cmd( ".v.i add command -label \"increment(...)\" -command { set choice 40 } -accelerator Ctrl+I" );
		cmd( ".v.i add command -label \"multiply(...)\" -command { set choice 45 } -accelerator Ctrl+M" );
		cmd( ".v.i add command -label \"search(...)\" -command { set choice 55 } -accelerator Ctrl+A" );
		cmd( ".v.i add command -label \"search_var_cond(...)\" -command { set choice 30 } -accelerator Ctrl+S" );
		cmd( ".v.i add command -label \"lsdqsort(...)\" -command { set choice 31 } -accelerator Ctrl+T" );
		cmd( ".v.i add command -label \"draw_rnd\" -command { set choice 54 } -accelerator Ctrl+N" );
		cmd( ".v.i add command -label \"write(...)\" -command { set choice 29 } -accelerator Ctrl+W" );
		cmd( ".v.i add command -label \"add_n_objects2\" -command { set choice 52 } -accelerator Ctrl+O" );
		cmd( ".v.i add command -label \"delete_obj\" -command { set choice 53 } -accelerator Ctrl+D" );
		cmd( ".v.i add command -label \"Math functions\" -command { set choice 51 } -accelerator Ctrl+H" );
		choice = 0;
		goto loop;
	}

	// Show extra source files
	if ( choice == 70 )
	{
		s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );
		if ( s == NULL || ! strcmp( s, "" ) )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
			choice = 0;
			goto loop;
		}

		// Create model options file if it doesn't exist
		cmd( "set choice [ file exists \"$modelDir/$MODEL_OPTIONS\" ]" );
		if ( choice == 0 )
			make_makefile( );

		choice = 0;
		cmd( "set fapp [ file nativename \"$modelDir/$MODEL_OPTIONS\" ]" );
		s = ( char * ) Tcl_GetVar( inter, "fapp", 0 );
		if ( s == NULL || ( f = fopen( s, "r" ) ) == NULL )
		{
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Makefile not created\" -detail \"Please check 'Model Options' and 'System Options' in menu 'Model'.\"" );
			goto loop;
		}

		while ( fgets( str, MAX_LINE_SIZE - 1, f ) != NULL && strncmp( str, "FUN_EXTRA=", 10 ) );
		fclose( f );
		if ( strncmp( str, "FUN_EXTRA=", 10 ) || sscanf( str + 10, "%s", str1 ) < 1 )
		{
			cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"No extra files defined\" -detail \"Open 'Model Options' in menu 'Model' and include all extra files names in the line starting with 'FUN_EXTRA='. Add the names after the '=' character and separate them with spaces or use 'Add Extra' button to select one or more files.\n\nIf there is no 'FUN_EXTRA=' line, press 'Default' button first.\"" );
			goto loop;
		}
		i = strlen( str ) - 1;
		str[ i ] = '\0';				// remove LF
		if ( str[ --i ] == '\r' )
			str[ i ] = '\0';			// remove CR (Windows)

		cmd( "set fun_extra [ split [ string trim \"%s\" ] \" \t\" ]", str + 10 );
		cmd( "set extra_files [ list ]" );
		cmd( "foreach x $fun_extra { if { [ string trim $x ] != \"\" && ( [ file exists \"$x\" ] || [ file exists \"$modelDir/$x\" ] ) } { lappend extra_files \"$x\" } }" );
		cmd( "set brr \"\"" );
		cmd( "set e .extra" );

		cmd( "newtop $e \"Extra Files\" { set choice 2 }"  );

		cmd( "ttk::frame $e.lf " );
		cmd( "ttk::label $e.lf.l1 -justify center -text \"Show additional\nsource files for model:\"" );
		cmd( "ttk::label $e.lf.l2 -style hl.TLabel -text \"$modelName\"" );
		cmd( "pack $e.lf.l1 $e.lf.l2" );

		cmd( "ttk::frame $e.l" );
		cmd( "ttk::scrollbar $e.l.v_scroll -command \"$e.l.l yview\"" );
		cmd( "ttk::listbox $e.l.l -listvariable extra_files -width 30 -height 15 -selectmode single -yscroll \"$e.l.v_scroll set\" -dark $darkTheme" );
		cmd( "pack $e.l.l $e.l.v_scroll -side left -fill y" );
		cmd( "mouse_wheel $e.l.l" );
		
		cmd( "bind $e.l.l <Home> { selectinlist .extra.l.l 0 }" );
		cmd( "bind $e.l.l <End> { selectinlist .extra.l.l end }" );

		cmd( "set choice [ $e.l.l size ]" );
		if ( choice > 0 )
			cmd( "bind $e.l.l <Double-Button-1> { set brr [ .extra.l.l curselection ]; set choice 1 }" );
		else
			cmd( "$e.l.l insert end \"(none)\"" );

		cmd( "ttk::label $e.l3 -text \"(double-click to show the file)\"" );
		cmd( "ttk::label $e.l4 -justify center -text \"Extra files are added using\n'Model Options' in menu 'Model'\"" );

		cmd( "pack $e.lf $e.l $e.l3 $e.l4 -pady 5 -padx 5" );

		cmd( "okcancel $e b { set choice 1 } { set choice 2 }" );

		cmd( "showtop $e" );
		cmd( "mousewarpto $e.b.ok" );

		choice = 0;
		while ( choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "set i [ $e.l.l curselection ]" );
		cmd( "destroytop $e" );

		cmd( "if { $i == \"\" } { set brr \"\" } { set brr [ lindex $extra_files $i ] }" );
		s = ( char * ) Tcl_GetVar( inter, "brr", 0 );

		if ( choice == 1 && strlen( s ) > 0 )
		{
			cmd( "if { ! [ file exists \"$brr\" ] && [ file exists \"$modelDir/$brr\" ] } { set brr \"$modelDir/$brr\" }" );
			choice = 71;
		}
		else
			choice = 0;

		goto loop;
	}

	// show the file/line/column containing error from compilation results window
	if ( choice == 87 )
	{
		// check if file exists and normalize name for comparisons
		cmd( "if { [ file exists \"$errfil\" ] } { \
				set errfil \"[ file normalize \"$errfil\" ]\" \
			} elseif { $errfil != \"\" && [ file exists \"$modelDir/$errfil\" ] } { \
				set errfil \"[ file normalize \"$modelDir/$errfil\" ]\" \
			} else { \
				set errfil \"\" \
			}" );

		cmd( "if { $errfil == \"\" } { \
					set choice 0 \
				} { \
					if [ string equal -nocase [ file tail \"$errfil\" ] \"fun_head.h\" ] { \
						ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Error in LSD macro expansion\" -detail \"Please check the offending file and line in the description following the error message.\"; \
						set choice 0 \
					} else { \
						set choice 1 \
					} \
				}" );
		if ( choice == 0 )
			goto loop;				// insufficient data to show error

		// check if file is already loaded
		cmd( "if { [ string equal \"$errfil\" \"[ file normalize \"$dirname/$filename\" ]\" ] } { \
				set choice 1 \
			} { \
				set choice 0 \
			}" );

		if ( choice == 0 )
		{
			// check if main equation file is not the current file
			s = get_fun_name( str );
			if ( s != NULL && strlen( s ) > 0 )
				cmd( "if [ string equal \"$errfil\" \"[ file normalize \"$modelDir/%s\" ]\" ] { set choice 8 }", s );		// open main equation file

			// try to open an extra file defined by the user
			if ( choice == 0 )
			{	// open the configuration file
				cmd( "set fapp [ file nativename \"$modelDir/$MODEL_OPTIONS\" ]" );
				s = ( char * ) Tcl_GetVar( inter, "fapp", 0 );
				if ( s == NULL || strlen( s ) == 0 || ( f = fopen( s, "r" ) ) == NULL )
				{
					cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Makefile not created\" -detail \"Please check 'Model Options' and 'System Options' in menu 'Model' and then try again.\"" );
					goto loop;
				}
				else
					fclose( f );

				// search in all source files (except main, already done)
				cmd( "set source_files [ get_source_files $modelDir ]" );
				cmd( "if { [ llength $source_files ] > 1 } { set fun_extra [ lreplace $source_files 0 0 ]; set choice [ llength $fun_extra ] } { set choice 0 }" );

				if ( choice > 0 )
				{	// search error file in the extra files list
					cmd( "foreach x $fun_extra { \
							set x \"[ string trim $x ]\"; \
							if { $x != \"\" } { \
								if { [ file exists \"$x\" ] } { \
									set x \"[ file normalize \"$x\" ]\" \
								} elseif { [ file exists \"$modelDir/$x\" ] } { \
									set x \"[ file normalize \"$modelDir/$x\" ]\" \
								} else { \
									set x \"\" \
								}; \
								if { $x != \"\" && [ string equal \"$errfil\" \"$x\" ] } { \
									set choice 71; \
									break \
								} \
							} \
						}" );
				}

				if ( choice == 0 )
				{
					cmd( "ttk::messageBox -parent .mm -title Warning -icon warning -type ok -message \"File not tracked\" -detail \"LMM is able to show error only in your main equation file or in extra files explicitly included.\n\nIf you want to track additional extra files, please open 'Model Options' in menu 'Model' and include all (additional) extra files names at the end of the line starting with 'FUN_EXTRA='. Add the names after the '=' character and separate them with spaces or use 'Add Extra' button to select one or more files.\n\nIf there is no 'FUN_EXTRA=' line, press 'Default' button first.\"" );
					goto loop;				// file not available
				}
			}
		}

		// show the error position on screen
		cmd( "focustop .f.t.t" );

		if ( choice == 1 )
		{	// file already loaded, just point error
			cmd( "if { [ info exists errlin ] && $errlin != \"\" && [ string is integer -strict $errlin ] } { \
					.f.t.t tag remove sel 1.0 end; \
					.f.t.t tag add sel $errlin.0 $errlin.end; \
					if { [ info exists errcol ] && $errcol != \"\" && [ string is integer -strict $errcol ] } { \
						.f.t.t see $errlin.[ expr $errcol - 1 ]; \
						.f.t.t mark set insert $errlin.[ expr $errcol - 1 ] \
					} else { \
						.f.t.t see $errlin.0; \
						.f.t.t mark set insert $errlin.0 \
					}; \
					upd_cursor \
				}" );

			choice = 0;						// nothing else to do
		}
		else
			// load the found extra file as correct "choice" is already set
			cmd( "set brr \"$errfil\"" );

		goto loop;
	}

	// reset colors after a sign for recoloring
	if ( choice == 23 )
	{
		if ( sourcefile != 0 )
		{
			// text window not ready?
			if ( Tcl_GetVar( inter, "curPosIni", 0 ) == NULL || Tcl_GetVar( inter, "curPosFin", 0 ) == NULL || strlen( Tcl_GetVar( inter, "curPosIni", 0 ) ) == 0 || strlen( Tcl_GetVar( inter, "curPosFin", 0 ) ) == 0 )
				goto loop;

			// check if inside or close to multi-line comment and enlarge region appropriately
			cmd( "if { [ lsearch -exact [ .f.t.t tag names $curPosIni ] comment1 ] != -1 } { \
						set curPosFin [ .f.t.t search */ $curPosIni end ]; \
						set newPosIni [ .f.t.t search -backwards /* $curPosIni 1.0 ]; \
						set curPosIni $newPosIni \
					} else { \
						if { \"$curPosIni linestart\" != \"\" && [ .f.t.t search -backwards */ $curPosIni \"$curPosIni linestart\" ] != \"\" } { \
							set comIni [ .f.t.t search -backwards /* $curPosIni ]; \
							if { $comIni != \"\" } { \
								set curPosIni $comIni \
							} \
						}; \
						if { \"$curPosFin lineend\" != \"\" && [ .f.t.t search /* $curPosFin \"$curPosFin lineend\" ] != \"\" } { \
							set comFin [ .f.t.t search */ $curPosFin ]; \
							if { $comFin != \"\" } { \
								set curPosFin $comFin \
							} \
						} \
					}" );

			// find the range of lines to reeval the coloring
			char *curPosIni=( char* ) Tcl_GetVar( inter,"curPosIni", 0 ); // position before insertion
			char *curPosFin=( char* ) Tcl_GetVar( inter,"curPosFin", 0 ); // position after insertion
			char *curSelIni=( char* ) Tcl_GetVar( inter,"curSelIni", 0 ); // selection before insertion
			char *curSelFin=( char* ) Tcl_GetVar( inter,"curSelFin", 0 ); // selection after insertion

			// collect all selection positions, before and after change
			float curPos[ 6 ];
			int nVal = 0;
			i = sscanf( curPosIni, "%f", &curPos[ nVal ] );
			nVal += i < 0 ? 0 : i;
			i = sscanf( curPosFin, "%f", &curPos[ nVal ] );
			nVal += i < 0 ? 0 : i;
			i = sscanf( curSelIni, "%f %f", &curPos[ nVal ], &curPos[ nVal + 1 ] );
			nVal += i < 0 ? 0 : i;
			i = sscanf( curSelFin, "%f %f", &curPos[ nVal ], &curPos[ nVal + 1 ] );
			nVal += i < 0 ? 0 : i;

			// find previous and next lines to select (worst case scenario)
			long prevLin = ( long ) floor( curPos[ 0 ] );
			long nextLin = ( long ) floor( curPos[ 0 ] ) + 1;
			for ( i = 1; i < nVal; ++i )
			{
				if ( curPos[ i ] < prevLin )
					prevLin = ( long ) floor( curPos[ i ] );
				if ( curPos[ i ] + 1 > nextLin )
					nextLin = ( long ) floor( curPos[ i ] ) + 1;
			}

			color( shigh, prevLin, nextLin );
		}

		choice = 0;
		goto loop;
	}

	// ignore invalid command
	if ( choice != 1 )
	{
		choice = 0;
		goto loop;
	}

	Tcl_UnlinkVar( inter, "num");
	Tcl_UnlinkVar( inter, "shigh");
	Tcl_UnlinkVar( inter, "choice");
	Tcl_UnlinkVar( inter, "tosave");
	Tcl_UnlinkVar( inter, "recolor_all");

	set_env( false );
	
	delete [ ] rootLsd;
	delete [ ] exec_path;

	return 0;
}


/*********************************
 IS_SOURCE_FILE
 *********************************/
bool is_source_file( const char *fname )
{
	char *ext;
	
	cmd( "set ext \"[ file extension \"%s\" ]\"", fname );
	ext = ( char * ) Tcl_GetVar( inter, "ext", 0 );

	return ! strcmp( ext, ".cpp" ) || ! strcmp( ext, ".c" )   || ! strcmp( ext, ".C" )   || \
		   ! strcmp( ext, ".CPP" ) || ! strcmp( ext, ".Cpp" ) || ! strcmp( ext, ".c++" ) || \
		   ! strcmp( ext, ".C++" ) || ! strcmp( ext, ".h" )   || ! strcmp( ext, ".H" )   || \
		   ! strcmp( ext, ".hpp" ) || ! strcmp( ext, ".HPP" ) || ! strcmp( ext, ".Hpp" );
}


/*********************************
 COLOR
 *********************************/
// data structures for color syntax (used by color/rm_color)
struct hit
{
	int type, count;
	long iniLin, iniCol;
	char previous, next;
};
// color types (0-n) to Tk tags mapping
const char *cTypes[ ] = { "comment1", "comment2", "cprep", "str", "lsdvar", "lsdmacro", "ctype", "ckword" };
// regular expressions identifying colored text types
const char *cRegex[ ] = {
	"/\[*].*\[*]/",		// each item define one different color
	"//.*",
	"^(\\s)*#\[^/]*",
	"\\\"\[^\\\"]*\\\"",
	"v\\[\[0-9]{1,3}]|curl?\[1-9]?|root|up|next|hook",
	"MODEL(BEGIN|END)|(END_)?EQUATION(_DUMMY)?|FUNCTION|RESULT|ABORT|DEBUG_(START|STOP)(_AT)?|CURRENT|VL?S?|V_(CHEATL?S?|NODEIDS?|NODENAMES?|LINKS?|EXTS?|LAT)|SUM(_CND)?L?S?|COUNT(_ALL|_CNDL?|_ALL_CNDL?|_HOOK)?S?|STAT(_CND)?L?S?|STAT_(NETS?|NODES?)|(WHT)?AVE(_CND)?L?S?|MED(_CND)?L?S?|PERC(_CND)?L?S?|SD(_CND)?L?S?|INCRS?|MULTS?|CYCLES?|CYCLE_(EXTS?|LINKS?)|CYCLE2?3?_SAFES?|MAX(_CND)?L?S?|MIN(_CND)?L?S?|HOOKS?|SHOOKS?|WRITEL?L?S?|WRITE_(NODEIDS?|NODENAMES?|LINK|EXTS?|ARG_EXTS?|LAT|HOOKS?|SHOOKS?)|SEARCH(_CNDL?|_INST|_NODE|_LINK)?S?|SEARCHS?|TSEARCH(_CND)?S?|SORT2?S?|ADDN?OBJL?S?|ADDN?OBJ_EXL?S?|ADD(NODES?|LINKW?S?|EXTS?|EXT_INITS?|HOOKS?)|DELETE|DELETE_(EXTS?|NETS?|NODES?|LINKS?)|DELETINGS?|RND|RND_(GENERATOR|SEED|SETSEED)|RNDDRAWL?S?|RNDDRAW_(FAIRS?|TOTL?S?|NODES?|LINKS?)|DRAWPROB_(NODES?|LINK)|PARAMETER|INTERACTS?|P?LOG|INIT_(TSEARCH(_CND)?T?S?|NETS?|LAT)|LOAD_NETS?|SAVE_(NETS?|LAT)|(SNAP|SHUFFLE)_NETS?|LINK(TO|FROM)|EXTS?|(P|DO|EXEC)_EXTS?|(USE|NO)_NAN|(USE|NO)_POINTER_CHECK|(USE|NO)_SAVED|(USE|NO)_SEARCH|(USE|NO)_ZERO_INSTANCE|PATH|CONFIG|(LAST_)?T|SLEEP|FAST(_FULL)?|OBSERVE|LAST_CALCS?|RECALCS?|UPDATE(S|_RECS?)?|DEFAULT_RESULT|THIS|NEXTS?|(GRAND)?PARENTS?|UP|DOWN|RUN|abs|min|max|round(_digits)?|(sq|cb)rt|pow|exp|log(10)?|fact|(t|l)?gamma|a?sin|a?cos|a?tan|pi|is_(finite|inf|nan)|uniform(_int)?|l?norm(cdf)?|poisson(cdf)?|beta(cdf)?|alapl(cdf)?|unifcdf|gammacdf|close_sim",
	"auto|const|double|float|int|short|struct|unsigned|long|signed|void|enum|volatile|char|extern|static|union|asm|bool|explicit|template|typename|class|friend|private|inline|public|virtual|mutable|protected|wchar_t",
	"break|continue|else|for|switch|case|default|goto|sizeof|typedef|do|if|return|while|dynamic_cast|namespace|reinterpret_cast|try|new|static_cast|typeid|catch|false|operator|this|using|throw|delete|true|const_cast|cin|endl|iomanip|main|npos|std|cout|include|iostream|NULL|string"
};

// count words in a string (used by color)
int strwrds( char string[ ] )
{
	int i = 0, words = 0;
	char lastC = '\0';
	
	if ( string == NULL ) 
		return 0;
	
	while ( isspace( string[ i ] ) ) 
		++i;
	
	if ( string[ i ] == '\0' ) 
		return 0;
	
	for ( ; string[ i ] != '\0'; lastC = string[ i++ ] )
		if ( isspace( string[ i ] ) && ! isspace( lastC ) ) 
			words++;
		
	if ( isspace( lastC ) ) 
		return words;

	return words + 1;
}

// map syntax highlight level to the number of color types to use
#define ITEM_COUNT( ptrArray )  ( sizeof( ptrArray ) / sizeof( ptrArray[0] ) )
int map_color( int hiLev )
{
	if ( ! sourcefile || hiLev == 0 )
		return 0;
	
	if ( hiLev == 1 )
		return 4;
	
	if ( ITEM_COUNT( cTypes ) > ITEM_COUNT( cRegex ) )
		return ITEM_COUNT( cRegex );
	
	return ITEM_COUNT( cTypes );
}

// compare function for qsort to compare different color hits (used by color)
int comphit(const void *p1, const void *p2)
{
	if ( ( ( hit * ) p1 )->iniLin < ( ( hit * ) p2 )->iniLin ) 
		return -1;
	
	if ( ( ( hit * ) p1 )->iniLin > ( ( hit * ) p2 )->iniLin ) 
		return 1;
	
	if ( ( ( hit * ) p1 )->iniCol < ( ( hit * ) p2 )->iniCol ) 
		return -1;
	
	if ( ( ( hit * ) p1 )->iniCol > ( ( hit * ) p2 )->iniCol ) 
		return 1;
	
	if ( ( ( hit * ) p1 )->type < ( ( hit * ) p2 )->type ) 
		return -1;
	
	if ( ( ( hit * ) p1 )->type > ( ( hit * ) p2 )->type ) 
		return 1;
	
	return 0;
}

// color routine
#define TOT_COLOR ITEM_COUNT( cTypes )
void color( int hiLev, long iniLin, long finLin )
{
	char *pcount, *ppos, *count[ TOT_COLOR ], *pos[ TOT_COLOR ], finStr[ 16 ], *s;
	int i, maxColor, newCnt;
	long j, k, tsize = 0, curLin = 0, curCol = 0, newLin, newCol, size[ TOT_COLOR ];
	struct hit *hits;

	// prepare parameters
	maxColor = map_color( hiLev );	// convert option to # of color types
	if ( finLin == 0 )			// convert code 0 for end of text
		sprintf( finStr, "end");
	else
		sprintf( finStr, "%ld.end", finLin );

	// remove color tags
	for ( i = 0; ( unsigned ) i < TOT_COLOR; ++i )
		cmd( ".f.t.t tag remove %s %ld.0 %s", cTypes[ i ], iniLin == 0 ? 1 : iniLin, finStr );

	// find & copy all occurrence types to arrays of C strings
	for ( i = 0; i < maxColor; ++i )
	{
		// locate all occurrences of each color group
		Tcl_UnsetVar( inter, "ccount", 0 );
		if ( ! strcmp( cTypes[ i ], "comment1" ) )	// multi line search element?
			cmd( "set pos [.f.t.t search -regexp -all -nolinestop -count ccount -- {%s} %ld.0 %s]", cRegex[ i ], iniLin == 0 ? 1 : iniLin, finStr );
		else
			cmd( "set pos [.f.t.t search -regexp -all -count ccount -- {%s} %ld.0 %s]", cRegex[ i ], iniLin == 0 ? 1 : iniLin, finStr );

		// check number of ocurrences
		pcount = ( char * ) Tcl_GetVar( inter, "ccount", 0 );
		size[ i ] = strwrds(pcount);
		if (size[ i ] == 0)				// nothing to do?
			continue;
		tsize += size[ i ];

		// do intermediate store in C memory
		count[ i ] = ( char * ) calloc( strlen( pcount ) + 1, sizeof( char ) );
		strcpy(count[ i ], pcount);
		ppos = ( char * ) Tcl_GetVar( inter, "pos", 0 );
		pos[ i ] = ( char * ) calloc( strlen( ppos ) + 1, sizeof( char ) );
		strcpy(pos[ i ], ppos);
	}
	if ( tsize == 0 )
		return;							// nothing to do

	// organize all occurrences in a single array of C numbers (struct hit)
	hits = ( hit * ) calloc( tsize, sizeof( hit ) );
	for ( i = 0, k = 0; i < maxColor; ++i )
	{
		if ( size[ i ] == 0 )			// nothing to do?
			continue;
		pcount = ( char* ) count[ i ] - 1;
		ppos = ( char* ) pos[ i ] - 1;
		for ( j = 0; j < size[ i ] && k < tsize; j++, ++k )
		{
			hits[ k ].type = i;
			s = strtok( pcount + 1, " \t" );
			hits[ k ].count = atoi( s );
			pcount = s + strlen( s );
			s = strtok( ppos + 1, " \t" );
			sscanf( strtok( s, " \t" ), "%ld.%ld", &hits[ k ].iniLin, &hits[ k ].iniCol );
			ppos = s + strlen( s );
		}
		free( count[ i ] );
		free( pos[ i ] );
	}

	// Sort the single list for processing
	qsort( ( void * ) hits, tsize, sizeof( hit ), comphit );

	// process each occurrence, if applicable
	Tcl_LinkVar( inter, "lin", ( char * ) &newLin, TCL_LINK_LONG | TCL_LINK_READ_ONLY );
	Tcl_LinkVar( inter, "col", ( char * ) &newCol, TCL_LINK_LONG | TCL_LINK_READ_ONLY );
	Tcl_LinkVar( inter, "cnt", ( char * ) &newCnt, TCL_LINK_INT | TCL_LINK_READ_ONLY );

	for (k = 0; k < tsize; ++k )
		// skip occurrences inside other occurrence
		if ( hits[ k ].iniLin > curLin || ( hits[ k ].iniLin == curLin && hits[ k ].iniCol >= curCol ) )
		{
			newLin = hits[ k ].iniLin;
			newCol = hits[ k ].iniCol;
			newCnt = hits[ k ].count;
			cmd( "set end [.f.t.t index \"$lin.$col + $cnt char\"]" );
			// treats each type of color case properly
			if ( hits[ k ].type < 4 )		// non token?
				cmd( ".f.t.t tag add %s $lin.$col $end", cTypes[ hits[ k ].type ] );
			else							// token - should not be inside another word
				cmd( "if { [ regexp {\\w} [ .f.t.t get \"$lin.$col - 1 any chars\" ] ] == 0 && [ regexp {\\w} [ .f.t.t get $end ] ] == 0 } { .f.t.t tag add %s $lin.$col $end }", cTypes[ hits[ k ].type ] );
			// next search position
			ppos = ( char * ) Tcl_GetVar( inter, "end", 0 );
			sscanf( ppos, "%ld.%ld", &curLin, &curCol );
		}

	Tcl_UnlinkVar( inter, "lin");
	Tcl_UnlinkVar( inter, "col");
	Tcl_UnlinkVar( inter, "cnt");
	free( hits );
}


/*********************************
 MAKE_MAKEFILE
 *********************************/
void make_makefile( bool nw )
{
	check_option_files( );

	cmd( "set f [ open \"$modelDir/$MODEL_OPTIONS\" r ]" );
	cmd( "set a [ read -nonewline $f ]" );
	cmd( "close $f" );

	cmd( "set f [ open \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\" r ]" );
	cmd( "set d [ read -nonewline $f ]" );
	cmd( "close $f" );

	cmd( "set f [ open \"$RootLsd/$LsdSrc/makefile-%s.txt\" r ]", nw ? "NW" : ( char * ) Tcl_GetVar( inter, "CurPlatform", 0 ) );
	
	cmd( "set b [ read -nonewline $f ]" );
	cmd( "close $f" );

	cmd( "set c \"# Model compilation options\\n$a\\n\\n# System compilation options\\n$d\\nLSDROOT=$RootLsd\\n\\n# Body of makefile%s (from makefile_%s.txt)\\n$b\"", nw ? "NW" : "", nw ? "NW" : ( char * ) Tcl_GetVar( inter, "CurPlatform", 0 ) );
	cmd( "set f [ open \"$modelDir/makefile%s\" w ]", nw ? "NW" : "" );
	cmd( "puts -nonewline $f $c" );
	cmd( "close $f" );
}


/*********************************
 CHECK_OPTION_FILES
 *********************************/
void check_option_files( bool sys )
{
	int exists;
	Tcl_LinkVar( inter, "exists", ( char * ) &exists, TCL_LINK_BOOLEAN );

	if ( ! sys )
	{
		cmd( "set exists [ file exists \"$modelDir/$MODEL_OPTIONS\" ]" );

		if ( ! exists )
		{
			cmd( "set dir [ glob -nocomplain \"$modelDir/fun_*.cpp\" ]" );
			cmd( "if { $dir != \"\" } { set b [ file tail [ lindex $dir 0 ] ] } { set b \"fun_UNKNOWN.cpp\" }" );
			cmd( "set a \"# LSD options\nTARGET=$DefaultExe\nFUN=[ file rootname \"$b\" ]\n\n# Additional model files\nFUN_EXTRA=\n\n# Compiler options\nSWITCH_CC=-O0 -ggdb3\nSWITCH_CC_LNK=\"" );
			cmd( "set f [ open \"$modelDir/$MODEL_OPTIONS\" w ]" );
			cmd( "puts -nonewline $f $a" );
			cmd( "close $f" );
		}
	}

	cmd( "set exists [ file exists \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\"]" );
	if ( ! exists )
	{
		cmd( "if [ string equal $tcl_platform(platform) windows ] { set sysfile \"system_options-windows.txt\" } elseif [ string equal $tcl_platform(os) Darwin ] { set sysfile \"system_options-mac.txt\" } else { set sysfile \"system_options-linux.txt\" }" );
		cmd( "set f [ open \"$RootLsd/$LsdSrc/$SYSTEM_OPTIONS\" w ]" );
		cmd( "set f1 [ open \"$RootLsd/$LsdSrc/$sysfile\" r ]" );
		cmd( "puts -nonewline $f \"# LSD options\n\"" );
		cmd( "puts -nonewline $f \"LSDROOT=$RootLsd\n\"" );
		cmd( "puts -nonewline $f \"SRC=$LsdSrc\n\n\"" );
		cmd( "puts -nonewline $f [ read $f1 ]" );
		cmd( "close $f" );
		cmd( "close $f1" );
	}

	Tcl_UnlinkVar( inter, "exists" );
}


/*********************************
 GET_FUN_NAME
 *********************************/
char *get_fun_name( char *str, bool nw )
{
	char *s, buf[ MAX_PATH_LENGTH ];
	FILE *f;

	make_makefile( nw );

	cmd( "set fapp [ file nativename \"$modelDir/makefile%s\" ]", nw ? "NW" : "" );
	s = ( char * ) Tcl_GetVar( inter, "fapp", 0 );

	f = fopen( s, "r" );
	if ( f == NULL )
		goto error;

	fgets( str, MAX_LINE_SIZE - 1, f );
	while ( strncmp( str, "FUN=", 4 ) && ! feof( f ) )
		fgets( str, MAX_LINE_SIZE - 1, f );

	fclose( f );

	if ( strncmp( str, "FUN=", 4 ) != 0 )
		goto error;

	sscanf( str + 4, "%499s", buf );
	sprintf( str, "%s.cpp", buf );
	return str;

error:
	cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Makefile not found or corrupted\" -detail \"Please check 'Model Options' and 'System Options' in menu 'Model'.\"" );
	return NULL;
}


/*********************************
 USE_EIGEN
 *********************************/

bool use_eigen( void )
{
	bool nfound = true;
	char *path, *fun_file, full_name[ MAX_PATH_LENGTH + 1 ], buffer[ 2 * MAX_PATH_LENGTH ];
	FILE *f;

	path = ( char * ) Tcl_GetVar( inter, "modelDir", 0 );
	fun_file = get_fun_name( buffer, true );

	if( path == NULL || fun_file == NULL )
		return false;

	snprintf( full_name, MAX_PATH_LENGTH, "%s/%s", path, fun_file );
	f = fopen( full_name, "r" );
	if( f == NULL )
		return false;

	while ( fgets( buffer, 2 * MAX_PATH_LENGTH - 1, f ) != NULL &&
			( nfound = strncmp( buffer, EIGEN, strlen( EIGEN ) ) ) );

	fclose( f );

	if ( nfound )
		return false;

	return true;
}


/*********************************
 COMPILE_RUN
 *********************************/
bool compile_run( bool run, bool nw )
{
	bool ret = false;
	char *s, str[ 2 * MAX_PATH_LENGTH ];
	int res, max_threads = 1;
	FILE *f;

	Tcl_LinkVar( inter, "res", ( char * ) &res, TCL_LINK_INT );

	cmd( "destroytop .mm" );	// close any open compilation results window
	cmd( "cd \"$modelDir\"" );

	s = ( char * ) Tcl_GetVar( inter, "modelName", 0 );
	if ( s == NULL || ! strcmp( s, "" ) )
	{
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
		goto end;
	}

	if ( ! run && ! nw )			// delete existing object file if it's just compiling
	{								// to force recompilation
		cmd( "set oldObj \"[ file rootname [ lindex [ glob -nocomplain fun_*.cpp ] 0 ] ].o\"" );
		cmd( "if { [ file exists \"$oldObj\" ] } { file delete \"$oldObj\" }" );
	}

	// get source name
	s = get_fun_name( str, nw );
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
	s = ( char * )Tcl_GetVar( inter, "fapp", 0 );
	f = fopen( s, "r" );
	fscanf( f, "%999s", str );
	while ( strncmp( str, "TARGET=", 7 ) && fscanf( f, "%999s", str ) != EOF );
	fclose( f );
	if ( strncmp( str, "TARGET=", 7 ) != 0 )
	{
		cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Makefile%s corrupted\" -detail \"Check 'Model Options' and 'System Options' in menu 'Model'.\"", nw ? "NW" : "" );
		goto end;
	}
	if ( nw )
		strcpy( str, "TARGET=lsdNW" );		// NW version use fixed name because of batches

	// show compilation banner
	cmd( "if { ! $autoHide || ! %d } { set parWnd .; set posWnd centerW } { set parWnd \"\"; set posWnd centerS }", run );
	cmd( "newtop .t \"Please Wait\" \"\" $parWnd" );

	if ( nw )
		cmd( "ttk::label .t.l1 -style bold.TLabel -justify center -text \"Compiling 'No Window' model...\"" );
	else
		cmd( "ttk::label .t.l1 -style bold.TLabel -justify center -text \"Compiling model...\"" );

	if ( run )
		cmd( "ttk::label .t.l2 -justify center -text \"Just recompiling equation file(s) changes.\nOn success, the new model program will be launched.\nOn failure, a new window will show the compilation errors.\"" );
	else
		if ( nw )
			cmd( "ttk::label .t.l2 -justify center -text \"Creating command-line model program  ('lsdNW').\nOn success, the model directory can be also ported to any computer.\nOn failure, a new window will show the compilation errors.\"" );
		else
			cmd( "ttk::label .t.l2 -justify center -text \"Recompiling the entire model program.\nOn success, the new program will NOT be launched.\nOn failure, a new window will show the compilation errors.\"" );

	cmd( "pack .t.l1 .t.l2 -padx 5 -pady 5" );
	cmd( "cancel .t b { set res 2 }");
	cmd( "showtop .t $posWnd" );

	// minimize LMM if required
	cmd( "set res $autoHide" );		// get auto hide status
	if ( res && run )				// hide LMM?
		cmd( "wm iconify ." );

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

	if ( res == 2 )
	{
		cmd( "catch { close $makePipe }" );
		cmd( "if [ file exists make.bat ] { file delete make.bat }" );
		cmd( "focustop .f.t.t" );
		goto end;
	}

	if ( res == 0 )							// compilation failure?
	{
		cmd( "set res $autoHide" );			// get auto hide status
		if ( run && res )					// auto unhide LMM if necessary
			cmd( "focustop .f.t.t" );  		// only reopen if error
		create_compresult_window( nw );		// show errors
	}
	else
	{
		if ( nw )
			cmd( "ttk::messageBox -parent . -type ok -icon info -title \"'No Window' Model\" -message \"Compilation successful\" -detail \"A non-graphical, command-line model program was created.\n\nThe executable 'lsdNW\\[.exe\\]' for this computer was generated in your model directory. It can be ported to any computer with a GCC-compatible compiler, like a high-performance server.\n\nTo port the model, copy the entire model directory:\n\n$modelDir\n\nto another computer (including the subdirectory '$LsdSrc'). After the copy, use the following steps to use the model program in the new computer:\n\n- open the command-line terminal/shell\n- change to the copied model directory ('cd')\n- recompile with the command:\n\nmake -f makefileNW\n\n- run the model program with a preexisting model configuration file ('.lsd' extension) using the command:\n\n./lsdNW -f CONF_NAME.lsd\n\n(you may have to remove the './' in Windows)\n\nSimulations run in the command-line will save the results into files with '.res\\[.gz\\]' and '.tot\\[.gz\\]' extensions.\n\nSee LSD documentation for further details.\"" );

		if ( run )							// no problem - execute
		{
			// create the element list file in background
			cmd( "after 0 { create_elem_file $modelDir }; update" );

			switch ( platform )
			{
				case LINUX:
					cmd( "catch { exec ./%s & } result", str + 7 );
					break;
					
				case MAC:
					cmd( "catch { exec open -F -n ./%s.app & } result", str + 7 );
					break;
					
				case WINDOWS:
					cmd( "catch { exec %s.exe & } result", str + 7 );
					break;
			}
		}
	}

	ret = true;
end:
	cmd( "cd \"$RootLsd\"" );
	Tcl_UnlinkVar( inter, "res" );
	return ret;
}


/*********************************
 CREATE_COMPRESULT_WINDOW
 *********************************/
void create_compresult_window( bool nw )
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

	cmd( "ttk::frame .mm.b" );

	cmd( "ttk::button .mm.b.perr -width [ expr $butWid + 4 ] -text \"Previous Error\" -underline 0 -command { \
			focus .mm.t.t; \
			set start \"$cerr linestart\"; \
			set errtemp [ .mm.t.t search -nocase -regexp -count errlen -backward -- $error $start 1.0];  \
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
				if { $errfil != \"\" && [ llength $errdat ] > $idxfil && [ string is integer -strict [ lindex $errdat $idxfil ] ] } { \
					set errlin [ lindex $errdat $idxfil ] \
				} else { \
					set errlin  \"\" \
				}; \
				incr idxfil; \
				if { $errfil != \"\" && [ llength $errdat ] > $idxfil && [ string is integer -strict [ lindex $errdat $idxfil ] ] } { \
					set errcol [ lindex $errdat $idxfil ] \
				} else { \
					set errcol \"\" \
				}; \
				.mm.i.f.n configure -text $errfil; \
				.mm.i.l.n configure -text $errlin; \
				.mm.i.c.n configure -text $errcol; \
			} \
		}" );
	cmd( "ttk::button .mm.b.gerr -width [ expr $butWid + 4 ] -text \"Go to Error\" -underline 0 -command { set choice 87 }" );
	cmd( "ttk::button .mm.b.ferr -width [ expr $butWid + 4 ] -text \"Next Error\" -underline 0 -command { \
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
				if { $errfil != \"\" && [ llength $errdat ] > $idxfil && [ string is integer -strict [ lindex $errdat $idxfil ] ] } { \
					set errlin [ lindex $errdat $idxfil ] \
				} else { \
					set errlin  \"\" \
				}; \
				incr idxfil; \
				if { $errfil != \"\" && [ llength $errdat ] > $idxfil && [ string is integer -strict [ lindex $errdat $idxfil ] ] } { \
					set errcol [ lindex $errdat $idxfil ] \
				} else { \
					set errcol \"\" \
				}; \
				.mm.i.f.n configure -text $errfil; \
				.mm.i.l.n configure -text $errlin; \
				.mm.i.c.n configure -text $errcol; \
			} \
		}" );
	cmd( "ttk::button .mm.b.close -width [ expr $butWid + 4 ] -text Done -underline 0 -command { unset -nocomplain errfil errlin errcol; destroytop .mm; focustop .f.t.t; set keepfocus 0 }" );
	cmd( "pack .mm.b.perr .mm.b.gerr .mm.b.ferr .mm.b.close -padx $butSpc -expand yes -fill x -side left" );
	cmd( "pack .mm.b -padx $butPad -pady $butPad -side right" );

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
	cmd( "mousewarpto .mm.b.gerr" );

	cmd( "if [ file exists \"$modelDir/makemessage.txt\" ] { set file [open \"$modelDir/makemessage.txt\"]; .mm.t.t insert end [read $file]; close $file } { .mm.t.t insert end \"(no compilation errors)\" }" );
	cmd( ".mm.t.t mark set insert \"1.0\"" );
	cmd( ".mm.b.ferr invoke" );

	cmd( ".mm.t.t configure -state disabled" );
	cmd( "focustop .mm.t.t" );
	cmd( "set keepfocus 1" );
	cmd( "update" );
}


/****************************************************
DISCARD_CHANGE
Ask user to discard changes in edited file, if applicable
Returns: 0: abort, 1: continue
****************************************************/
bool discard_change( void )
{
	if ( ! tosave )
		return true;					// yes: simply discard configuration

	// ask for confirmation
	cmd( "set answer [ ttk::messageBox -parent . -type yesnocancel -default yes -icon question -title Confirmation -message \"Save current file?\" -detail \"Recent changes to file '$filename' have not been saved.\\n\\nDo you want to save before continuing?\nNot doing so will not include recent changes to subsequent actions.\n\n - Yes: save the file and continue.\n - No: do not save and continue.\n - Cancel: do not save and return to editing.\" ]" );
	cmd( "if [ string equal $answer yes ] { \
			set curfile [ file join \"$dirname\" \"$filename\" ]; \
			set file [ open \"$curfile\" w ]; \
			puts -nonewline $file [ .f.t.t get 0.0 end ]; \
			close $file; \
			set before [ .f.t.t get 0.0 end ]; \
			update_title_bar; \
			set ans 1; \
		} elseif [ string equal $answer cancel ] { \
			set ans 0; \
			set choice 0 \
		} { \
			set ans 1 \
		}" );

	const char *ans = Tcl_GetVar( inter, "ans", 0 );
	if ( atoi( ans ) == 0 )
		return false;

	return true;
}
