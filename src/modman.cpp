/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Silk icon set 1.3 by Mark James
http://www.famfamfam.com/lab/icons/silk 

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/

/*****
used up to 69 options included
*******/

/*****************************************************
This program is a front end for dealing with Lsd models code (running, compiling, editing, debugging 
Lsd model programs). See the manual for help on its use (really needed?)

IMPORTANT: this is _NOT_ a Lsd model, but the best I could produce of something similar to 
a programming environment for Lsd model programs.


This file can be compiled with the command line:

make -f <makefile>

There are several makefiles in Lsd root directory appropriate to different environments (Windows, Mac & Linux) and configurations (32 or 64-bit)

LMM starts in a quite weird way. If there is no parameter in the call used to start it, the only operation it does is to ... run a copy of itself followed by the parameter "kickstart". This trick is required because under Windows there are troubles launchins external package from a "first instance" of a program.

LMM reads all the directories that are not: Manual, gnu and src as model directories, where it expect to find certain files. At any given moment a model name is stored, together with its directory and the file shown. Any command is executed in a condition like this:
if(choice==x)
 do_this_and_that

and returned to the main cycle.

After each block the flow returns to "loop" where the main Tcl_DoOneEvent loop
sits.

The widget of importance are:
- .f.t.t is the main text editor
- .f.m is the frame containing the upper buttons, models list and help window


*****************************************************/

#include <tk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <new>

// LSD version strings, for About... boxes and code testing
#define _LSD_MAJOR_ 7
#define _LSD_MINOR_ 0
#define _LSD_VERSION_ "7.0"
#define _LSD_DATE_ __DATE__

// general buffer limits
#define TCL_BUFF_STR 3000		// standard Tcl buffer size (>1000)
#define MAX_PATH_LENGTH 500		// maximum path length
#define MAX_LINE_SIZE 1000		// max size of a text line to read from files (>999)

// user defined signals
#define SIGMEM NSIG + 1			// out of memory signal
#define SIGSTL NSIG + 2			// standard library exception signal

// auxiliary C procedures
int ModManMain( int argn, char **argv );
void cmd( const char *cm, ... );
void color( int hiLev, long iniLin, long finLin );
void create_compresult_window( void );
void delete_compresult_window( void );
void handle_signals( void );
void log_tcl_error( const char *cm, const char *message );
void make_makefile( void );
void make_makefileNW( void );
void signal_handler( int signum );

// global variables
Tcl_Interp *inter;
bool tk_ok = false;				// control for tk_ready to operate
char msg[ TCL_BUFF_STR ]; 
int choice;
int shigh;						// syntax highlighting state (0, 1 or 2)
int v_counter=0; 				//counter of the v[i] variables inserted in the equations


/*************************************
 MAIN
 *************************************/
int main( int argn, char **argv )
{
	int res;
	
	// register all signal handlers
	handle_signals( );

	try
	{
		res = ModManMain( argn, argv );
	}
	catch ( std::bad_alloc&  )	// out of memory conditions
	{
		signal_handler( SIGMEM );
	}
	catch ( std::exception& exc )// other known error conditions
	{
		sprintf( msg, "\nSTL exception of type: %s\n", exc.what( ) );
		signal_handler( SIGSTL );
	}
	catch ( ... )				// other unknown error conditions
	{
		abort( );				// raises a SIGABRT exception, tell user & close
	}
	
	Tcl_Exit( res );
	return res;
}


/*************************************
 CMD
 *************************************/
bool firstCall = true;

// enhanced cmd with embedded sprintf capabilities and integrated buffer underrun protection
void cmd( const char *cm, ... )
{
	char message[ TCL_BUFF_STR ];
	
	if ( strlen( cm ) >= TCL_BUFF_STR )
	{
		sprintf( message, "Tcl buffer overrun. Please increase TCL_BUFF_STR to at least %ld bytes.", strlen( cm ) );
		log_tcl_error( cm, message );
		if ( tk_ok )
			cmd( "tk_messageBox -type ok -title Error -icon warning -message \"Tcl buffer overrun (memory corrupted!)\" -detail \"Save your data and close LMM after pressing 'Ok'.\"" );
	}

	char buffer[ TCL_BUFF_STR ];
	va_list argptr;
	
	va_start( argptr, cm );
	int reqSz = vsnprintf( buffer, TCL_BUFF_STR, cm, argptr );
	va_end( argptr );
	
	if ( reqSz >= TCL_BUFF_STR )
	{
		sprintf( message, "Tcl buffer too small. Please increase TCL_BUFF_STR to at least %d bytes.", reqSz + 1 );
		log_tcl_error( cm, message );
		if ( tk_ok )
			cmd( "tk_messageBox -type ok -title Error -icon error -message \"Tcl buffer too small\" -detail \"Tcl/Tk command was canceled.\"" );
	}
	else
	{
		int code = Tcl_Eval( inter, buffer );

		if( code != TCL_OK )
			log_tcl_error( cm, Tcl_GetStringResult( inter ) );
	}
}


/*************************************
 LOG_TCL_ERROR
 *************************************/
void log_tcl_error( const char *cm, const char *message )
{
	FILE *f;
	time_t rawtime;
	struct tm *timeinfo;
	char ftime[ 80 ];

	f = fopen( "tk_err.err","a" );

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	strftime ( ftime, 80, "%F %T", timeinfo );

	if ( firstCall )
	{
		firstCall = false;
		fprintf( f,"\n\n====================> NEW TCL SESSION\n" );
	}
	fprintf( f, "\n(%s)\nCommand:\n%s\nMessage:\n%s\n-----\n", ftime, cm, message );
	fclose( f );
	
	if ( tk_ok )
		cmd( "tk_messageBox -type ok -title Error -icon error -message \"Tcl/Tk error\" -detail \"More information in file 'tk_err.err'.\"" );
}


/*************************************
 MODMANMAIN
 *************************************/
int ModManMain( int argn, char **argv )
{
int i, num, sourcefile;
bool tosave = false, macro = true;
char str[MAX_LINE_SIZE+2*MAX_PATH_LENGTH], str1[2*MAX_PATH_LENGTH], str2[2*MAX_PATH_LENGTH];
char *s;
FILE *f;

// initialize the tcl interpreter
Tcl_FindExecutable( argv[0] );
inter = Tcl_CreateInterp( );
num = Tcl_Init( inter );
if ( num != TCL_OK )
{
	sprintf( msg, "Tcl initialization directories not found, check the Tcl/Tk installation  and configuration or reinstall Lsd\nTcl Error = %d : %s", num,  Tcl_GetStringResult( inter ) );
	log_tcl_error( "Create Tcl interpreter", msg );
	return 1;
}

// set variables and links in TCL interpreter
Tcl_SetVar( inter, "_LSD_VERSION_", _LSD_VERSION_, 0 );
Tcl_SetVar( inter, "_LSD_DATE_", _LSD_DATE_, 0 );
Tcl_LinkVar( inter, "choice", ( char * ) &choice, TCL_LINK_INT );

// test Tcl interpreter
cmd( "set choice 1234567890" );
Tcl_UpdateLinkedVar(inter, "choice" );
if ( choice != 1234567890 )
{
	log_tcl_error( "Test Tcl", "Tcl failed, check the Tcl/Tk installation and configuration or reinstall Lsd" );
	return 2;
}
	
// initialize & test the tk application
choice = 1;
num = Tk_Init( inter );
if ( num == TCL_OK )
	cmd( "if { ! [ catch { package present Tk 8.5 } ] && [ winfo exists . ] } { set choice 0 } { set choice 1 }" );
if ( choice )
{
	sprintf( msg, "Tk failed, check the Tcl/Tk installation (version 8.5+) and configuration or reinstall Lsd\nTcl Error = %d : %s", num,  Tcl_GetStringResult( inter ) );
	log_tcl_error( "Start Tk", msg );
	return 3;
}
tk_ok = true;
cmd( "tk appname lmm" );

cmd( "if { [string first \" \" \"[pwd]\" ] >= 0  } {set choice 1} {set choice 0}" );
if ( choice )
 {
 cmd( "tk_messageBox -icon error -title Error -type ok -message \"Installation error\" -detail \"The Lsd directory is: '[pwd]'\n\nIt includes spaces, which makes impossible to compile and run Lsd models.\nThe Lsd directory must be located where there are no spaces in the full path name.\nMove all the Lsd directory in another directory. If exists, delete the 'system_options.txt' file from the \\src directory.\"" );
 log_tcl_error( "Path check", "Lsd directory path includes spaces, move all the Lsd directory in another directory without spaces in the path" );
 return 4;
 }

if(argn>1)
 {
  for(i=0; argv[1][i]!=0; i++)
   {
    if(argv[1][i]=='\\')
     msg[i]='/';
    else
     msg[i]=argv[1][i];
   }
   msg[i]=argv[1][i];
  cmd( "set filetoload %s", msg );
  cmd( "if {[file pathtype \"$filetoload\"]==\"absolute\"} {} {set filetoload \"[pwd]/$filetoload\"}" );
 }

sprintf(msg, "%s",*argv);
if(!strncmp(msg, "//", 2))
 sprintf(str, "%s",msg+3);
else
 sprintf(str, "%s",msg);

cmd( "if {[file exists [file dirname \"[file nativename %s]\"]]==1} {cd [file dirname \"%s\"]; set choice 1} {cd [pwd]; set choice 0}", str, str );

// check if LSDROOT already exists and use it if so
cmd( "if [ info exists env(LSDROOT) ] { set RootLsd [ file normalize $env(LSDROOT) ]; if { ! [ file exists \"$RootLsd/src/decl.h\" ] } { unset RootLsd } }" );
cmd( "if { ! [ info exists RootLsd ] } { set RootLsd [ pwd ]; set env(LSDROOT) $RootLsd }" );
cmd( "set groupdir [pwd]" );
cmd( "if {$tcl_platform(platform) == \"unix\"} {set DefaultWish wish; set DefaultTerminal xterm; set DefaultHtmlBrowser firefox; set DefaultFont Courier} {}" );
cmd( "if {$tcl_platform(os) == \"Darwin\"} {set DefaultWish wish8.5; set DefaultTerminal terminal; set DefaultHtmlBrowser open; set DefaultFont Courier} {}" );
cmd( "if { [ string equal $tcl_platform(platform) windows ] && [ string equal $tcl_platform(machine) intel ] } { set DefaultWish wish85.exe; set DefaultTerminal cmd; set DefaultHtmlBrowser open; set DefaultFont \"Courier New\"}" );
cmd( "if { [ string equal $tcl_platform(platform) windows ] && [ string equal $tcl_platform(machine) amd64 ] } { set DefaultWish wish86.exe; set DefaultTerminal cmd; set DefaultHtmlBrowser open; set DefaultFont \"Courier New\"}" );

cmd( "set Terminal $DefaultTerminal" );
cmd( "set HtmlBrowser $DefaultHtmlBrowser" );
cmd( "set fonttype $DefaultFont" );
cmd( "set wish $DefaultWish" );
cmd( "set LsdSrc src" );

cmd( "if { [ string equal $tcl_platform(platform) windows ] && [ string equal $tcl_platform(machine) intel ] } {set LsdGnu gnu} {set LsdGnu gnu64}" );
	
cmd( "set choice [file exist $RootLsd/lmm_options.txt]" );
if(choice==1)
 {
  cmd( "set f [open $RootLsd/lmm_options.txt r]" );
  cmd( "gets $f Terminal" );
  cmd( "gets $f HtmlBrowser" );
  cmd( "gets $f fonttype" );
  cmd( "gets $f wish" );
  cmd( "gets $f LsdSrc" );
	cmd( "gets $f dim_character" );
	cmd( "gets $f tabsize" );
	cmd( "gets $f wrap" );
	cmd( "gets $f shigh" );
	cmd( "gets $f autoHide" );
	cmd( "gets $f showFileCmds" );
	cmd( "gets $f LsdNew" );
  cmd( "close $f" );
	// handle old options file
	cmd( "if {$dim_character == \"\" || $showFileCmds == \"\"} {set choice 0}" );
 }
// handle non-existent or old options file for new options
if ( choice != 1 )
 {
	cmd( "set dim_character 0" );	// default font size (0=force auto-size)
	cmd( "set tabsize 2" );			// default tab size
	cmd( "set wrap 1" );				// default text wrapping mode (1=yes)
	cmd( "set shigh 2" );			// default is full syntax highlighting
	cmd( "set autoHide 0" );			// default is to not auto hide LMM on run
	cmd( "set showFileCmds 0" );		// default is no text file commands in File menu
	cmd( "set LsdNew Work" );		// default new model subdirectory is "Work"
  cmd( "set f [open $RootLsd/lmm_options.txt w]" );
  cmd( "puts $f $Terminal" );
  cmd( "puts $f $HtmlBrowser" );
  cmd( "puts $f $fonttype" );
  cmd( "puts $f $wish" );  
  cmd( "puts $f $LsdSrc" );
	cmd( "puts $f $dim_character" );
	cmd( "puts $f $tabsize" );
	cmd( "puts $f $wrap" );
	cmd( "puts $f $shigh" );
	cmd( "puts $f $autoHide" );
	cmd( "puts $f $showFileCmds" );
	cmd( "puts $f $LsdNew" );
  cmd( "close $f" );
 }
 
cmd( "if { [file exists $RootLsd/$LsdSrc/system_options.txt] == 1} {set choice 0} {set choice 1}" );
if(choice==1)
 { //the src/system_options.txt file doesn't exists, so I invent it
	cmd( "if [ string equal $tcl_platform(platform) windows ] { if [ string equal $tcl_platform(machine) intel ] { set sysfile \"sysopt_win32.txt\" } { set sysfile \"sysopt_win64.txt\" } } { if [ string equal $tcl_platform(os) Darwin ] { set sysfile \"sysopt_mac.txt\" } { set sysfile \"sysopt_linux.txt\" } }" );
    cmd( "set f [open $RootLsd/$LsdSrc/system_options.txt w]" );
    cmd( "set f1 [open $RootLsd/$LsdSrc/$sysfile r]" );
    cmd( "puts -nonewline $f \"LSDROOT=$RootLsd\\n\"" );
    cmd( "puts -nonewline $f [read $f1]" );
    cmd( "close $f" );
    cmd( "close $f1" );    
 }

cmd( "set choice 0" );
cmd( "set recolor \"\"" );
cmd( "set docase 0" );
cmd( "set dirsearch \"-forwards\"" );
cmd( "set endsearch end" );
cmd( "set currentpos \"\"" );
cmd( "set currentdoc \"\"" );
cmd( "set v_num 0" );
cmd( "set shigh_temp $shigh" );
cmd( "set alignMode \"LMM\"" );

// procedures to adjust tab size according to font type and size and text wrapping
cmd( "proc settab {w size font} { set tabwidth \"[ expr { $size * [ font measure \"$font\" 0 ] } ] left\"; $w conf -font \"$font\" -tabs $tabwidth -tabstyle wordprocessor }" );
cmd( "proc setwrap {w wrap} { if { $wrap == 1 } { $w conf -wrap word } { $w conf -wrap none } }" );

cmd( "if [ file exists $RootLsd/$LsdSrc/showmodel.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/showmodel.tcl } ] != 0 } { set choice [ expr $choice + 1 ] } } { set choice [ expr $choice + 2 ] }" );
cmd( "if [ file exists $RootLsd/$LsdSrc/lst_mdl.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/lst_mdl.tcl } ] != 0 } { set choice [ expr $choice + 1 ] } } { set choice [ expr $choice + 2 ] }" );
cmd( "if [ file exists $RootLsd/$LsdSrc/defaults.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/defaults.tcl } ] != 0 } { set choice [ expr $choice + 1 ] } } { set choice [ expr $choice + 2 ] }" );
cmd( "if [ file exists $RootLsd/$LsdSrc/window.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/window.tcl } ] != 0 } { set choice [ expr $choice + 1 ] } } { set choice [ expr $choice + 2 ] }" );
cmd( "if [ file exists $RootLsd/$LsdSrc/ls2html.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/ls2html.tcl } ] != 0 } { set choice [ expr $choice + 1 ] } } { set choice [ expr $choice + 2 ] }" );
cmd( "if [ file exists $RootLsd/$LsdSrc/dblclick.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/dblclick.tcl } ] != 0 } { set choice [ expr $choice + 1 ] } } { set choice [ expr $choice + 2 ] }" );
if ( choice != 0 )
{
	cmd( "tk_messageBox -type ok -icon error -title Error -message \"File(s) missing or corrupted\" -detail \"Some critical Tcl files ($choice) are missing or corrupted.\nPlease check your installation and reinstall Lsd if the problem persists.\n\nLsd is aborting now.\"" );
	log_tcl_error( "Source files check", "Required Tcl/Tk source file(s) missing or corrupted, check the installation of Lsd and reinstall Lsd if the problem persists" );
	return 10 + choice;
}

Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
Tcl_LinkVar(inter, "tosave", (char *) &tosave, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "macro", (char *) &macro, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "shigh", (char *) &shigh, TCL_LINK_INT);
cmd( "set shigh $shigh_temp" );	// restore correct value

// set main window
cmd( "wm title . \"Lsd Model Manager - LMM\"" );
cmd( "wm protocol . WM_DELETE_WINDOW { set choice 1 }" );
cmd( ". configure -menu .m" );		// define here to avoid redimensining the window
cmd( "bind . <Destroy> { set choice 1 }" );
cmd( "icontop . lmm" );
cmd( "sizetop .lmm" );

// main menu
cmd( "menu .m -tearoff 0" );

cmd( "set w .m.file" );
cmd( "menu $w -tearoff 0" );
cmd( ".m add cascade -label File -menu $w -underline 0" );
cmd( "$w add command -label \"New Model...\" -underline 0 -command { set choice 14}" );
cmd( "$w add command -label \"Browse Models...\" -underline 0 -command {set choice 33} -accelerator Ctrl+b" );
cmd( "$w add command -label \"Save Model\" -underline 0 -state disabled -command { if {[string length $filename] > 0} {if { [file exist $dirname/$filename] == 1} {catch {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak}} {}; set f [open $dirname/$filename w];puts -nonewline $f [.f.t.t get 0.0 end]; close $f; set before [.f.t.t get 0.0 end]; set choice 999} } -underline 0 -accelerator Ctrl+s" );
cmd( "$w add command -label \"Save Model As...\" -state disabled -underline 0 -command { set choice 41} -underline 11 -accelerator Ctrl+a" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Compare Models...\" -underline 3 -command {set choice 61} -underline 0" );
cmd( "$w add command -label \"TkDiff...\" -command {set choice 57} -underline 0" );
cmd( "$w add separator" );

cmd( "if { $showFileCmds == 1 } { $w add command -label \"New Text File\" -command { set choice 39} -underline 4 }" );
cmd( "if { $showFileCmds == 1 } { $w add command -label \"Open Text File...\" -command { set choice 15} -underline 0 -accelerator Ctrl+o }" );
cmd( "if { $showFileCmds == 1 } { $w add command -label \"Save Text File\" -command { if {[string length $filename] > 0} {if { [file exist $dirname/$filename] == 1} {catch {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak}} {}; set f [open $dirname/$filename w];puts -nonewline $f [.f.t.t get 0.0 end]; close $f; set before [.f.t.t get 0.0 end]; set choice 999} {}} -underline 2 }" );
cmd( "if { $showFileCmds == 1 } { $w add command -label \"Save Text File As...\" -command {set choice 4} -underline 3 }" );
cmd( "if { $showFileCmds == 1 } { $w add separator }" );

cmd( "$w add command -label \"Options...\" -command { set choice 60} -underline 1" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Quit\" -command {set choice 1} -underline 0 -accelerator Ctrl+q" );

cmd( "set w .m.edit" );
cmd( "menu $w -tearoff 0" );
cmd( ".m add cascade -label Edit -menu $w -underline 0" );
// make menu the same as ctrl-z/y (more color friendly)
cmd( "$w add command -label \"Undo\" -command {catch {.f.t.t edit undo}} -underline 0 -accelerator Ctrl+z" );
cmd( "$w add command -label \"Redo\" -command {catch {.f.t.t edit redo}} -underline 2 -accelerator Ctrl+y" );
cmd( "$w add separator" );
// collect information to focus recoloring
cmd( "$w add command -label \"Cut\" -command {savCurIni; tk_textCut .f.t.t; if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd} -underline 1 -accelerator Ctrl+x" );
cmd( "$w add command -label \"Copy\" -command {tk_textCopy .f.t.t} -underline 0 -accelerator Ctrl+c" );
cmd( "$w add command -label \"Paste\" -command {savCurIni; tk_textPaste .f.t.t; if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd} -underline 0 -accelerator Ctrl+v" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Find...\" -command {set choice 11} -underline 0 -accelerator Ctrl+f" );
cmd( "$w add command -label \"Find Again\" -command {set choice 12} -underline 5 -accelerator F3" );
cmd( "$w add command -label \"Replace...\" -command {set choice 21} -underline 0" );
cmd( "$w add command -label \"Goto Line...\" -command {set choice 10} -underline 5 -accelerator Ctrl+l" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Match \\\{ \\}\" -command {set choice 17} -accelerator Ctrl+m" );
cmd( "$w add command -label \"Match \\\( \\)\" -command {set choice 32} -accelerator Ctrl+u" );
cmd( "$w add command -label \"Insert \\\{\" -command {.f.t.t insert insert \\\{} -accelerator Ctrl+\\\(" );
cmd( "$w add command -label \"Insert \\}\" -command {.f.t.t insert insert \\}} -accelerator Ctrl+\\)" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Indent Selection\" -command {set choice 42} -accelerator Ctrl+>" );
cmd( "$w add command -label \"De-indent Selection\" -command {set choice 43} -accelerator Ctrl+<" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Larger Font\" -command {incr dim_character 1; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"; settab .f.t.t $tabsize \"$a\"} -accelerator Ctrl+'+'" );
cmd( "$w add command -label \"Smaller Font\" -command {incr dim_character -1; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"; settab .f.t.t $tabsize \"$a\"} -accelerator Ctrl+'-'" );
cmd( "$w add command -label \"Change Font...\" -command {set choice 59} -underline 8" );
cmd( "$w add separator" );
// add option to ajust syntax highlighting (word coloring)
cmd( "$w add cascade -label \"Syntax Highlighting\" -menu $w.color -underline 0" );
cmd( "$w add check -label \"Wrap/Unwrap Text\" -variable wrap -command {setwrap .f.t.t $wrap} -underline 1 -accelerator Ctrl+w " );
cmd( "$w add command -label \"Change Tab Size...\" -command {set choice 67} -underline 7" );
cmd( "$w add command -label \"Insert Lsd Scripts...\" -command {set choice 28} -underline 0 -accelerator Ctrl+i" );

cmd( "menu $w.color -tearoff 0" );
cmd( "$w.color add radio -label \" Full\" -variable shigh -value 2 -command {set choice 64} -underline 1 -accelerator \"Ctrl+;\"" );
cmd( "$w.color add radio -label \" Partial\" -variable shigh -value 1 -command {set choice 65} -underline 1 -accelerator Ctrl+," );
cmd( "$w.color add radio -label \" None\" -variable shigh -value 0 -command {set choice 66} -underline 1 -accelerator Ctrl+." );

cmd( "set w .m.model" );
cmd( "menu $w -tearoff 0" );
cmd( ".m add cascade -label Model -menu $w -underline 0" );
cmd( "$w add command -label \"Compile and Run Model...\" -state disabled -underline 0 -command {set choice 2} -accelerator Ctrl+r" );
cmd( "$w add command -label \"Recompile Model\" -state disabled -underline 0 -command {set choice 6} -accelerator Ctrl+p" );
cmd( "$w add command -label \"GDB Debugger\" -state disabled -underline 0 -command {set choice 13} -accelerator Ctrl+g" );
cmd( "$w add command -label \"Create 'No Window' Version\" -underline 8 -state disabled -command {set choice 62}" );
cmd( "$w add command -label \"Model Info...\" -underline 6 -state disabled -command {set choice 44}" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Show Description\" -underline 5 -state disabled -command {set choice 5} -accelerator Ctrl+d" );
cmd( "$w add command -label \"Show Equations\" -state disabled -underline 5 -command {set choice 8} -accelerator Ctrl+e" );
cmd( "$w add command -label \"Show Makefile\" -state disabled -underline 7 -command { set choice 3}" );
cmd( "$w add command -label \"Show Compilation Results\" -underline 6 -state disabled -command {set choice 7}" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Model Compilation Options...\" -underline 4 -state disabled -command {set choice 48}" );
cmd( "$w add command -label \"System Compilation Options...\" -underline 0 -command {set choice 47}" );
cmd( "$w add separator" );
cmd( "$w add check -label \"Auto Hide LMM on Run\" -variable autoHide -underline 0" );
cmd( "$w add cascade -label \"Equations' Coding Style\" -underline 1 -menu $w.macro" );

cmd( "menu $w.macro -tearoff 0" );
cmd( "$w.macro add radio -label \" Use Lsd Macros\" -variable macro -value true -command {.m.help entryconf 1 -label \"Help on Macros for Lsd Equations\" -underline 6 -command {LsdHelp lsdfuncMacro.html}; set choice 68}" );
cmd( "$w.macro add radio -label \" Use Lsd C++\" -variable macro -value false -command {.m.help entryconf 1 -label \"Help on C++ for Lsd Equations\" -underline 8 -command {LsdHelp lsdfunc.html}; set choice 69}" );

cmd( "set w .m.help" );
cmd( "menu $w -tearoff 0" );
cmd( ".m add cascade -label Help -menu $w -underline 0" );
cmd( "$w add command -label \"Help on LMM\" -underline 4 -command {LsdHelp \"LMM_help.html\"}" );
if( macro )
  cmd( "$w add command -label \"Help on Macros for Lsd Equations\" -underline 6 -command {LsdHelp lsdfuncMacro.html}" );
else
  cmd( "$w add command -label \"Help on C++ for Lsd Equations\" -underline 8 -command {LsdHelp lsdfunc.html}" ); 

cmd( "$w add separator" );
//cmd( "$w add command -label \"Tutorial 1 - LMM First users\" -underline 6 -command {LsdHelp Tutorial1.html}" );
//cmd( "$w add command -label \"Tutorial 2 - Using Lsd Models\" -underline 0 -command {LsdHelp ModelUsing.html}" );
//cmd( "$w add command -label \"Tutorial 3 - Writing Lsd Models\" -underline 6 -command {LsdHelp ModelWriting.html}" );
cmd( "$w add command -label \"Lsd Documentation\" -command {LsdHelp Lsd_Documentation.html}" );
cmd( "$w add command -label \"About LMM...\" -command { tk_messageBox -parent . -type ok -icon info -title \"About LMM\" -message \"Version %s (%s)\" -detail \"Platform: [ string totitle $tcl_platform(platform) ] ($tcl_platform(machine))\nOS: $tcl_platform(os) ($tcl_platform(osVersion))\nTcl/Tk: [ info patch ]\" } -underline 0", _LSD_VERSION_, _LSD_DATE_  );


// Button bar
cmd( "frame .bbar -bd 2" );

cmd( "button .bbar.open -image openImg -relief $bRlf -overrelief $ovBrlf -command {set choice 33}" );
cmd( "button .bbar.save -image saveImg -relief $bRlf -overrelief $ovBrlf -command {if {[string length $filename] > 0} {if { [file exist $dirname/$filename] == 1} {catch {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak}} {}; set f [open $dirname/$filename w];puts -nonewline $f [.f.t.t get 0.0 end]; close $f; set before [.f.t.t get 0.0 end]; set choice 999}}" );
cmd( "button .bbar.undo -image undoImg -relief $bRlf -overrelief $ovBrlf -command {catch {.f.t.t edit undo}}" );
cmd( "button .bbar.redo -image redoImg -relief $bRlf -overrelief $ovBrlf -command {catch {.f.t.t edit redo}}" );
cmd( "button .bbar.cut -image cutImg -relief $bRlf -overrelief $ovBrlf -command {savCurIni; tk_textCut .f.t.t; if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd}" );
cmd( "button .bbar.copy -image copyImg -relief $bRlf -overrelief $ovBrlf -command {tk_textCopy .f.t.t}" );
cmd( "button .bbar.paste -image pasteImg -relief $bRlf -overrelief $ovBrlf -command {savCurIni; tk_textPaste .f.t.t; if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd}" );
cmd( "button .bbar.find -image findImg -relief $bRlf -overrelief $ovBrlf -command {set choice 11}" );
cmd( "button .bbar.replace -image replaceImg -relief $bRlf -overrelief $ovBrlf -command {set choice 21}" );
cmd( "button .bbar.indent -image indentImg -relief $bRlf -overrelief $ovBrlf -command {set choice 42}" );
cmd( "button .bbar.deindent -image deindentImg -relief $bRlf -overrelief $ovBrlf -command {set choice 43}" );
cmd( "button .bbar.comprun -image comprunImg -relief $bRlf -overrelief $ovBrlf -command {set choice 2}" );
cmd( "button .bbar.compile -image compileImg -relief $bRlf -overrelief $ovBrlf -command {set choice 6}" );
cmd( "button .bbar.info -image infoImg -relief $bRlf -overrelief $ovBrlf -command {set choice 44}" );
cmd( "button .bbar.descr -image descrImg -relief $bRlf -overrelief $ovBrlf -command {set choice 5}" );
cmd( "button .bbar.equation -image equationImg -relief $bRlf -overrelief $ovBrlf -command {set choice 8}" );
cmd( "button .bbar.set -image setImg -relief $bRlf -overrelief $ovBrlf -command {set choice 48}" );
cmd( "button .bbar.hide -image hideImg -relief $bRlf -overrelief $ovBrlf -command {set autoHide [ expr ! $autoHide ]}" );
cmd( "button .bbar.help -image helpImg -relief $bRlf -overrelief $ovBrlf -command {LsdHelp lsdfuncMacro.html}" );
cmd( "label .bbar.tip -textvariable ttip -font {Arial 8} -fg gray -width 30 -anchor w" );

cmd( "bind .bbar.open <Enter> {set ttip \"Browse models...\"}" );
cmd( "bind .bbar.open <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.save <Enter> {set ttip \"Save model\"}" );
cmd( "bind .bbar.save <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.undo <Enter> {set ttip \"Undo\"}" );
cmd( "bind .bbar.undo <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.redo <Enter> {set ttip \"Redo\"}" );
cmd( "bind .bbar.redo <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.cut <Enter> {set ttip \"Cut\"}" );
cmd( "bind .bbar.cut <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.copy <Enter> {set ttip \"Copy\"}" );
cmd( "bind .bbar.copy <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.paste <Enter> {set ttip \"Paste\"}" );
cmd( "bind .bbar.paste <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.find <Enter> {set ttip \"Find...\"}" );
cmd( "bind .bbar.find <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.replace <Enter> {set ttip \"Replace...\"}" );
cmd( "bind .bbar.replace <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.indent <Enter> {set ttip \"Indent selection\"}" );
cmd( "bind .bbar.indent <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.deindent <Enter> {set ttip \"De-indent selection\"}" );
cmd( "bind .bbar.deindent <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.comprun <Enter> {set ttip \"Compile and run model...\"}" );
cmd( "bind .bbar.comprun <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.compile <Enter> {set ttip \"Recompile model\"}" );
cmd( "bind .bbar.compile <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.info <Enter> {set ttip \"Model information...\"}" );
cmd( "bind .bbar.info <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.descr <Enter> {set ttip \"Show description\"}" );
cmd( "bind .bbar.descr <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.equation <Enter> {set ttip \"Show equations\"}" );
cmd( "bind .bbar.equation <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.set <Enter> {set ttip \"Model compilation options...\"}" );
cmd( "bind .bbar.set <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.hide <Enter> {if $autoHide {set ttip \"Not hide LMM on run\"} {set ttip \"Hide LMM on run\"}}" );
cmd( "bind .bbar.hide <Leave> {set ttip \"\"}" );
cmd( "bind .bbar.help <Enter> {set ttip \"Help on macros for Lsd equations\"}" );
cmd( "bind .bbar.help <Leave> {set ttip \"\"}" );

cmd( "pack .bbar.open .bbar.save .bbar.undo .bbar.redo .bbar.cut .bbar.copy .bbar.paste .bbar.find .bbar.replace .bbar.indent .bbar.deindent .bbar.comprun .bbar.compile .bbar.info .bbar.descr .bbar.equation .bbar.set .bbar.hide .bbar.help .bbar.tip -padx 3 -side left" );
cmd( "pack .bbar -anchor w -fill x" );

cmd( "frame .f -bd 2" );
cmd( "frame .f.t" );
cmd( "scrollbar .f.t.vs -command \".f.t.t yview\"" );
cmd( "scrollbar .f.t.hs -orient horiz -command \".f.t.t xview\"" );
cmd( "text .f.t.t -height 2 -undo 1 -autoseparators 1 -bg #fefefe -yscroll \".f.t.vs set\" -xscroll \".f.t.hs set\"" );
cmd( "set a [.f.t.t conf -font]" );
cmd( "set b [lindex $a 3]" );
cmd( "if {$dim_character == 0} {set dim_character [lindex $b 1]}" );
cmd( "if { $dim_character == \"\"} {set dim_character 12} {}" );
cmd( "set a [list $fonttype $dim_character]" );


// set preferred tab size and wrap option
cmd( "settab .f.t.t $tabsize \"$a\"" );	// adjust tabs size to font type/size
cmd( "setwrap .f.t.t $wrap" );		// adjust text wrap

// set syntax colors
cmd( ".f.t.t tag configure comment1 -foreground green4" );
cmd( ".f.t.t tag configure comment2 -foreground green4" );
cmd( ".f.t.t tag configure str -foreground blue4" );
cmd( ".f.t.t tag configure cprep -foreground SaddleBrown" );
cmd( ".f.t.t tag configure lsdvar -foreground red4" );
cmd( ".f.t.t tag configure lsdmacro -foreground DodgerBlue4" );
cmd( ".f.t.t tag configure ctype -foreground DarkViolet" );
cmd( ".f.t.t tag configure ckword -foreground purple4" );
cmd( ".f.t.t configure -selectbackground gray" );
cmd( ".f.t.t configure -selectforeground black" );

cmd( "frame .f.hea -relief groove -bd 2" );

cmd( "frame .f.hea.grp" );
cmd( "label .f.hea.grp.tit -text \"Group: \" -fg red" );
cmd( "label .f.hea.grp.dat -text \"$modelgroup\"" );
cmd( "pack .f.hea.grp.tit .f.hea.grp.dat -side left" );

cmd( "frame .f.hea.mod" );
cmd( "label .f.hea.mod.tit -text \"Model: \" -fg red" );
cmd( "label .f.hea.mod.dat -text \"(no model)\"" );
cmd( "pack .f.hea.mod.tit .f.hea.mod.dat -side left" );

cmd( "frame .f.hea.ver" );
cmd( "label .f.hea.ver.tit -text \"Version: \" -fg red" );
cmd( "label .f.hea.ver.dat -text \"\"" );
cmd( "pack .f.hea.ver.tit .f.hea.ver.dat -side left" );

cmd( "frame .f.hea.file" );
cmd( "label .f.hea.file.tit -text \"File: \" -fg red" );
cmd( "label .f.hea.file.dat -text \"(no file)\"" );
cmd( "pack .f.hea.file.tit .f.hea.file.dat -side left" );

cmd( "frame .f.hea.line" );
cmd( "label .f.hea.line.line -relief sunk -width 12 -text \"[.f.t.t index insert]\"" );
cmd( "pack .f.hea.line.line -anchor e -expand no" );
cmd( "pack .f.hea.grp .f.hea.mod .f.hea.ver .f.hea.file .f.hea.line -side left -expand yes -fill x" );

cmd( "bind .f.hea.line.line <Button-1> {set choice 10}" );
cmd( "bind .f.hea.line.line <Button-2> {set choice 10}" );
cmd( "bind .f.hea.line.line <Button-3> {set choice 10}" );

cmd( "pack .f.hea -fill x" );
cmd( "pack .f.t -expand yes -fill both" );
cmd( "pack .f.t.vs -side right -fill y" );
cmd( "pack .f.t.t -expand yes -fill both" );
cmd( "pack .f.t.hs -fill x" );

cmd( "set dir [glob *]" );
cmd( "set num [llength $dir]" );

cmd( "bind . <Control-n> {tk_menuSetFocus .m.file}; bind . <Control-N> {tk_menuSetFocus .m.file}" );

// procedures to save cursor environment before and after changes in text window for syntax coloring
cmd( "proc savCurIni {} {global curSelIni curPosIni; set curSelIni [.f.t.t tag nextrange sel 1.0]; set curPosIni [.f.t.t index insert]; .f.t.t edit modified false}" );
cmd( "proc savCurFin {} {global curSelFin curPosFin; set curSelFin [.f.t.t tag nextrange sel 1.0]; set curPosFin [.f.t.t index insert]; .f.t.t edit modified false}" );
cmd( "proc updCurWnd {} {.f.hea.line.line conf -text [.f.t.t index insert]}" );


// redefine bindings to better support new syntax highlight routine
cmd( "bind .f.t.t <KeyPress> {savCurIni}" );
cmd( "bind .f.t.t <KeyRelease> {if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd}" );
cmd( "bind .f.t.t <ButtonPress> {savCurIni}" );
cmd( "bind .f.t.t <ButtonRelease> {if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd}" );

cmd( "bind .f.t.t <Control-l> {set choice 10}; bind .f.t.t <Control-L> {set choice 10}" );
cmd( "bind .f.t.t <Control-w> {if {$wrap == 0} {set wrap 1} {set wrap 0}; setwrap .f.t.t $wrap}" );

cmd( "bind .f.t.t <Control-f> {set choice 11}; bind .f.t.t <Control-F> {set choice 11}" );
cmd( "bind .f.t.t <F3> {set choice 12}" );
cmd( "bind .f.t.t <Control-s> { if {[string length $filename] > 0} {if { [file exist $dirname/$filename] == 1} {catch {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak}} {}; set f [open $dirname/$filename w];puts -nonewline $f [.f.t.t get 0.0 end]; close $f; set before [.f.t.t get 0.0 end]; set choice 999} {}}" ); 
cmd( "bind .f.t.t <Control-a> { set choice 4}" );
cmd( "bind .f.t.t <Control-r> {set choice 2}; bind .f.t.t <Control-R> {set choice 2}" );
cmd( "bind .f.t.t <Control-e> {set choice 8}" );
cmd( "bind .f.t.t <Control-KeyRelease-o> {if {$tk_strictMotif == 0} {set a [.f.t.t index insert]; .f.t.t delete \"$a lineend\"} {}; set choice 15; break}" );
cmd( "bind .f.t.t <Control-q> {set choice 1}; bind .f.t.t <Control-Q> {set choice 1}" );
cmd( "bind .f.t.t <Control-p> {set choice 6; break}; bind .f.t.t <Control-P> {set choice 6; break}" );
cmd( "bind .f.t.t <Control-u> {set choice 32}" );
cmd( "bind .f.t.t <Control-m> {set choice 17}" );
cmd( "bind .f.t.t <Control-g> {set choice 13}; bind .f.t.t <Control-G> {set choice 13}" );
cmd( "bind .f.t.t <Control-d> {set choice 5; break}" );
cmd( "bind .f.t.t <Control-b> {set choice 33; break}; bind .f.t.t <Control-B> {set choice 33; break}" );
cmd( "bind .f.t.t <Control-minus> {incr dim_character -2; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"}" );
cmd( "bind .f.t.t <Control-plus> {incr dim_character 2; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"}" );
cmd( "bind .f.t.t <Control-parenleft> {.f.t.t insert insert \\\{}" );
cmd( "bind .f.t.t <Control-parenright> {.f.t.t insert insert \\}}" );
cmd( "bind .f.t.t <Control-greater> {set choice 42}" );
cmd( "bind .f.t.t <Control-less> {set choice 43}" );
cmd( "bind .f.t.t <Control-semicolon> {set choice 64}" );
cmd( "bind .f.t.t <Control-comma> {set choice 65}" );
cmd( "bind .f.t.t <Control-period> {set choice 66}" );
cmd( "bind .f.t.t <Alt-q> {.m postcascade 0}; bind .f.t.t <Alt-Q> {.m postcascade 0}" );
cmd( "if {\"$tcl_platform(platform)\" == \"unix\"} {bind .f.t.t <Control-Insert> {tk_textCopy .f.t.t}} {}" );
cmd( "if {\"$tcl_platform(platform)\" == \"unix\"} {bind .f.t.t <Shift-Insert> {tk_textPaste .f.t.t}} {}" );
cmd( "if { [ string equal $tcl_platform(platform) unix ] && ! [ string equal $tcl_platform(os) Darwin ] } { bind .f.t.t <Control-c> { tk_textCopy .f.t.t } }" );

cmd( "bind .f.t.t <KeyPress-Return> {+set choice 16}" );
cmd( "bind .f.t.t <KeyRelease-space> {+.f.t.t edit separator}" );
cmd( "bind .f.t.t <Control-z> {catch {.f.t.t edit undo}}; bind .f.t.t <Control-Z> {catch {.f.t.t edit undo}}" );
cmd( "bind .f.t.t <Control-y> {catch {.f.t.t edit redo}}; bind .f.t.t <Control-Y> {catch {.f.t.t edit redo}}" );
cmd( "bind . <KeyPress-Insert> {# nothing}" );

/*
POPUP menu
*/
cmd( "menu .v -tearoff 0" );

cmd( "bind .f.t.t  <3> {%%W mark set insert [.f.t.t index @%%x,%%y]; set vmenuInsert [.f.t.t index insert]; tk_popup .v %%X %%Y}" );
cmd( "bind .f.t.t  <2> {%%W mark set insert [.f.t.t index @%%x,%%y]; set vmenuInsert [.f.t.t index insert]; tk_popup .v %%X %%Y}" );
cmd( ".v add command -label \"Copy\" -command {tk_textCopy .f.t.t}" );
cmd( ".v add command -label \"Cut\" -command {tk_textCut .f.t.t}" );
cmd( ".v add command -label \"Paste\" -command {tk_textPaste .f.t.t}" );

cmd( ".v add separator" );
cmd( ".v add cascade -label \"Lsd Script\" -menu .v.i" );
cmd( ".v add command -label \"Insert Lsd Script...\" -command {set choice 28}" );
cmd( ".v add command -label \"Indent Selection\" -command {set choice 42}" );
cmd( ".v add command -label \"De-indent Selection\" -command {set choice 43}" );
cmd( ".v add command -label \"Place Break & Run GDB\" -command {set choice 58}" );

cmd( ".v add separator" );
cmd( ".v add command -label \"Find...\" -command {set choice 11}" );
cmd( ".v add command -label \"Match \\\{ \\}\" -command {set choice 17}" );
cmd( ".v add command -label \"Match \\\( \\)\" -command {set choice 32}" );

if ( ! macro )
{
	cmd( "menu .v.i -tearoff 0" );
	cmd( ".v.i add command -label \"Lsd equation\" -command {set choice 25} -accelerator Ctrl+E" );
	cmd( ".v.i add command -label \"cal(...)\" -command {set choice 26} -accelerator Ctrl+V" );
	cmd( ".v.i add command -label \"sum(...)\" -command {set choice 56} -accelerator Ctrl+U" );
	cmd( ".v.i add command -label \"search_var_cond(...)\" -command {set choice 30} -accelerator Ctrl+S" );
	cmd( ".v.i add command -label \"Search(...)\" -command {set choice 55} -accelerator Ctrl+A" );
	cmd( ".v.i add command -label \"write(...)\" -command {set choice 29} -accelerator Ctrl+W" );
	cmd( ".v.i add command -label \"for( ; ; )\" -command {set choice 27} -accelerator Ctrl+C" );
	cmd( ".v.i add command -label \"lsdqsort(...)\" -command {set choice 31} -accelerator Ctrl+T" );
	cmd( ".v.i add command -label \"Add a new object\" -command {set choice 52} -accelerator Ctrl+O" );
	cmd( ".v.i add command -label \"Delete an object\" -command {set choice 53} -accelerator Ctrl+D" );
	cmd( ".v.i add command -label \"Draw random an object\" -command {set choice 54} -accelerator Ctrl+N" );
	cmd( ".v.i add command -label \"increment(...)\" -command {set choice 40} -accelerator Ctrl+I" );
	cmd( ".v.i add command -label \"multiply(...)\" -command {set choice 45} -accelerator Ctrl+M" );
	cmd( ".v.i add command -label \"Math operation\" -command {set choice 51} -accelerator Ctrl+H" );
}
else
{
	cmd( "menu .v.i -tearoff 0" );
	cmd( ".v.i add command -label \"Lsd equation\" -command {set choice 25} -accelerator Ctrl+E" );
	cmd( ".v.i add command -label \"V(...)\" -command {set choice 26} -accelerator Ctrl+V" );
	cmd( ".v.i add command -label \"SUM(...)\" -command {set choice 56} -accelerator Ctrl+U" );
	cmd( ".v.i add command -label \"SEARCH_CND(...)\" -command {set choice 30} -accelerator Ctrl+S" );
	cmd( ".v.i add command -label \"SEARCH(...)\" -command {set choice 55} -accelerator Ctrl+A" );
	cmd( ".v.i add command -label \"WRITE(...)\" -command {set choice 29} -accelerator Ctrl+W" );
	cmd( ".v.i add command -label \"CYCLE(...)\" -command {set choice 27} -accelerator Ctrl+C" );
	cmd( ".v.i add command -label \"SORT(...)\" -command {set choice 31} -accelerator Ctrl+T" );
	cmd( ".v.i add command -label \"ADDOBJ(...)\" -command {set choice 52} -accelerator Ctrl+O" );
	cmd( ".v.i add command -label \"DELETE(...))\" -command {set choice 53} -accelerator Ctrl+D" );
	cmd( ".v.i add command -label \"RNDDRAW(...)\" -command {set choice 54} -accelerator Ctrl+N" );
	cmd( ".v.i add command -label \"INCR(...)\" -command {set choice 40} -accelerator Ctrl+I" );
	cmd( ".v.i add command -label \"MULT(...)\" -command {set choice 45} -accelerator Ctrl+M" );
	cmd( ".v.i add command -label \"Math operation\" -command {set choice 51} -accelerator Ctrl+H" );
}

cmd( "bind .f.t.t <Control-E> {set choice 25}" );
cmd( "bind .f.t.t <Control-V> {set choice 26}" );
cmd( "bind .f.t.t <Control-U> {set choice 56}" );
cmd( "bind .f.t.t <Control-A> {set choice 55}" );
cmd( "bind .f.t.t <Control-C> {set choice 27}" );
cmd( "bind .f.t.t <Control-i> {set choice 28; break}" );
cmd( "bind .f.t.t <Control-T> {set choice 31}" );
cmd( "bind .f.t.t <Control-I> {set choice 40}" );
cmd( "bind .f.t.t <Control-M> {set choice 45}" );
cmd( "bind .f.t.t <Control-S> {set choice 30}" );
cmd( "bind .f.t.t <Control-W> {set choice 29}" );
cmd( "bind .f.t.t <Control-O> {set choice 52}" );
cmd( "bind .f.t.t <Control-D> {set choice 53}" );
cmd( "bind .f.t.t <Control-N> {set choice 54}" );
cmd( "bind .f.t.t <Control-H> {set choice 51}" );

cmd( "bind .f.t.t <F1> {set choice 34}" );

// set advanced double-click behavior (dblclick.tcl) and also
// reset the word boundaries to fix weird double-click behavior on Windows
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

cmd( "set filename \"noname.txt\"" );
cmd( "set dirname [pwd]" );
cmd( "set modeldir \"[pwd]\"" );
cmd( "set groupdir [pwd]" );

cmd( ".f.t.t tag conf sel -foreground white" );
cmd( "set before [.f.t.t get 1.0 end]" );

if(argn>1)
 {cmd( "if {[file exists \"$filetoload\"] == 1} {set choice 0} {set choice -2}" );
  if(choice == 0)
    {cmd( "set file [open \"$filetoload\"]" );
     cmd( ".f.t.t insert end [read $file]" );
     cmd( ".f.t.t edit reset" );
     cmd( "close $file" );
     cmd( ".f.t.t mark set insert 1.0" );
     cmd( "set filename \"[file tail $filetoload]\"" );
     cmd( "set dirname [file dirname \"$filetoload\"]" );
     cmd( "set before [.f.t.t get 1.0 end]; .f.hea.file.dat conf -text \"$filename\"" );

     cmd( "set s [file extension \"$filetoload\"]" );
     s=(char *)Tcl_GetVar(inter, "s",0);
     choice=0;
     if(s[0]!='\0')
       {strcpy(str, s);
        if(!strcmp(str, ".cpp") || !strcmp(str, ".c") || !strcmp(str, ".C") || !strcmp(str, ".CPP") || !strcmp(str, ".Cpp") || !strcmp(str, ".c++") || !strcmp(str, ".C++") || !strcmp(str, ".h") || !strcmp(str, ".H") || !strcmp(str, ".hpp") || !strcmp(str, ".HPP") || !strcmp(str, ".Hpp"))
          {sourcefile=1;
			color(shigh, 0, 0);			// set color types (all text)
          }
        else
          sourcefile=0;
       }
    }
   else
	cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"File missing\" -detail \"File\\n$filetoload\\nnot found.\"" );
   
  }
 else
  choice= 33; 			// open model browser
  
cmd( "focus -force .f.t.t" );

cmd( "set lfindcounter -1" );


loop:

// indicate file save status in titlebar
if ( tosave )
	cmd( "wm title . \"*$filename - LMM\"" );
else
	cmd( "wm title . \" $filename - LMM\"" );

// main command loop
while( ! choice )
{
	try
	{
		Tcl_DoOneEvent( 0 );
	}
	catch ( std::bad_alloc& ) 	// raise memory problems
	{
		throw;
	}
	catch ( ... )				// ignore the rest
	{
		goto loop;
	}
}   

 
cmd( "set b [.f.t.t tag ranges sel]" );
cmd( "if {$b==\"\"} {} {set textsearch [.f.t.t get sel.first sel.last]}" );

// update the file changes status
cmd( "if [ winfo exists . ] { set after [ .f.t.t get 1.0 end] } { set after $before }" );
cmd( "if [ string compare $before $after ] { set tosave true } { set tosave false }" );

if( tosave && (choice==2 || choice==15 || choice==1 || choice==13 || choice==14 ||choice==6 ||choice==8 ||choice==3 || choice==33||choice==5||choice==39||choice==41))
  {

  cmd( "set answer [tk_messageBox -parent . -type yesnocancel -default yes -icon question -title Confirmation -message \"Save File?\" -detail \"Recent changes to file '$filename' have not been saved.\\n\\nDo you want to save before continuing?\nNot doing so will not include recent changes to subsequent actions.\n\n - Yes: save the file and continue.\n - No: do not save and continue.\n - Cancel: do not save and return to editing.\"]" );
  cmd( " if { $answer == yes} {set curfile [file join $dirname $filename]; set file [open $curfile w]; puts -nonewline $file [.f.t.t get 0.0 end]; close $file; set before [.f.t.t get 0.0 end]} {if [string equal $answer cancel] {set choice 0} {}}" );  
  if(choice==0)
  goto loop;

  }

  
if(choice==1)
  return 0;


if ( choice == 2 || choice == 6 )
 {
/*compile the model, invoking make*/
/*Run the model in the selection*/

bool run = ( choice == 2 ) ? true : false;
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
  choice=0;
  goto loop;
 }

  cmd( "cd $modeldir" );
  
  delete_compresult_window( );		// close any open compilation results window

  if ( ! run )						// delete existing object file if it's just compiling
  {									// to force recompilation
	cmd( "set oldObj \"[ file rootname [ lindex [ glob *.cpp ] 0 ] ].o\"" );
	cmd( "if { [ file exists $oldObj ] } { file delete $oldObj }" );
  }
  
  make_makefile();

  cmd( "set fapp [file nativename $modeldir/makefile]" );
  s=(char *)Tcl_GetVar(inter, "fapp",0);
  f=fopen(s, "r");
  if(f==NULL)
   {
     cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"Makefile not created\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
	 goto end_run;
   }
  fscanf(f, "%999s", str);
  while(strncmp(str, "FUN=", 4) && fscanf(f, "%999s", str)!=EOF);
  fclose(f);
  if(strncmp(str, "FUN=", 4)!=0)
   {
     cmd( "tk_messageBox -parent . -type ok -title Error -icon error -message \"Makefile corrupted\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
	 goto end_run;
   }
  cmd( "set fname %s.cpp", str+4 );
  
  f=fopen(s, "r");
  fscanf(f, "%999s", str);
  while(strncmp(str, "TARGET=", 7) && fscanf(f, "%999s", str)!=EOF);
  fclose(f);
  if(strncmp(str, "TARGET=", 7)!=0)
   {
     cmd( "tk_messageBox -parent . -type ok -title Error -icon error -message \"Makefile corrupted\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
	 goto end_run;
   }

  choice = run;
  cmd( "set init_time [clock seconds]" ); 
  cmd( "if { ! $autoHide || ! $choice } { set parWnd .; set posWnd centerW } { set parWnd \"\"; set posWnd centerS }" );
  cmd( "newtop .t \"Please Wait\" \"\" $parWnd" );
  cmd( "label .t.l1 -font {-weight bold} -text \"Making model...\"" );
  if ( run )
	cmd( "label .t.l2 -text \"The system is checking the files modified since the last compilation and recompiling as necessary.\nOn success the new model program will be launched.\nOn failure a text window will show the compiling error messages.\"" );
  else
	cmd( "label .t.l2 -text \"The system is recompiling the model.\nOn failure a text window will show the compiling error messages.\"" );
  cmd( "pack .t.l1 .t.l2 -padx 5 -pady 5" );
  cmd( "showtop .t $posWnd" );
  
  s = ( char * ) Tcl_GetVar( inter, "autoHide", 0 );	// get auto hide status
  if ( run && ! strcmp( s, "1" ) )	// auto hide LMM if appropriate
	cmd( "wm iconify ." );

  cmd( "if { [ string equal $tcl_platform(platform) windows ] && ! [ string equal $tcl_platform(machine) amd64 ] } {set choice 1} {set choice 0}" );
  if(choice==0)
    cmd( "catch { exec make -f makefile 2> makemessage.txt } result" ); 
  else
   {  
  cmd( "set file [open make.bat w]" );
  cmd( "puts -nonewline $file \"make.exe -f makefile 2> makemessage.txt\\n\"" );
  cmd( "close  $file" );
  cmd( "if { [file exists $RootLsd/$LsdSrc/system_options.txt] == 1} {set choice 0} {set choice 1}" );
  cmd( "if { [file exists %s.exe]  == 1} {file rename -force %s.exe %sOld.exe} { }", str+7, str+7, str+7 );
  cmd( "if { [file exists $RootLsd/$LsdGnu/bin/crtend.o] == 1} { file copy -force $RootLsd/$LsdGnu/bin/crtend.o .;file copy -force $RootLsd/$LsdGnu/bin/crtbegin.o .;file copy -force $RootLsd/$LsdGnu/bin/crt2.o .} {}" );
  cmd( "catch { exec make.bat } result" );
  cmd( "file delete make.bat" );
  cmd( "if { [file exists crtend.o] == 1} { file delete crtend.o;file delete crtbegin.o ;file delete crt2.o } {}" );
   }
   
  cmd( "destroytop .t" );
  
  cmd( "if { [ file size makemessage.txt] == 0 } { file delete makemessage.txt; set choice 0 } { set choice 1 }" );
  if(choice==1)
  {
	cmd( "if { $tcl_platform(platform) == \"windows\"} {set add_exe \".exe\"} {set add_exe \"\"}" );  
    cmd( "set funtime [file mtime $fname]" );
    cmd( "if { [file exist %s$add_exe] == 1 } {set exectime [file mtime %s$add_exe]} {set exectime $init_time}", str+7,str+7 );
  
    cmd( "if {$init_time < $exectime } {set choice 0}" );
    //turn choice into 0 if the executable is newer than the compilation command, implying just warnings
  }

  if(choice==1)
   { //problem
   if ( run && ! strcmp( s, "1" ) )	// auto unhide LMM if appropriate
     cmd( "wm deiconify ." );  // only reopen if error
   create_compresult_window();
   }
  else
	if ( run )
    {//no problem
     strcpy(str1, str+7);
      cmd( "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\"} {set choice 4} {set choice 2}} {set choice 3}}" );
      if(choice==1) //unix
       cmd( "catch { exec ./%s & } result",str1 );
      if(choice==2) //win2k, XP, 7, 8, 10...
       cmd( "catch { exec %s.exe & } result", str1 ); //Changed
      if(choice==3) //win 95/98
       cmd( "catch { exec start %s.exe & } result", str1 );
      if(choice==4)  //win NT
       cmd( "catch { exec cmd /c start %s.exe & } result", str1 );
     } 
	 
 end_run:
 cmd( "cd $RootLsd" );
 choice=0;
 goto loop;
 }


if(choice==3)
 {
  /*Insert in the text window the makefile of the selected model*/

cmd( ".f.t.t delete 0.0 end" );
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
  choice=0;
  goto loop;
 }

cmd( "cd $modeldir" );
make_makefile(); 
cmd( "cd $RootLsd" );
cmd( "if { [file exists $modeldir/makefile]==1} {set choice 1} {set choice 0}" );
if(choice==1)
 {
  cmd( "set file [open $modeldir/makefile]" );
  cmd( ".f.t.t insert end [read -nonewline $file]" );
  cmd( ".f.t.t edit reset" );
  cmd( "close $file" );
 } 
sourcefile=0; 
cmd( "set before [.f.t.t get 1.0 end]" );
cmd( "set filename makefile" );
cmd( ".f.t.t mark set insert 1.0" );
cmd( ".f.hea.file.dat conf -text \"makefile\"" );
cmd( "wm title . \"Makefile - LMM\"" );
cmd( "tk_messageBox -parent . -title Warning -icon warning -type ok -message \"File 'makefile' changed\" -detail \"Direct changes to the 'makefile' will not affect compilation issued through LMM. Choose 'System Compilation Options' and 'Model Compilation Options' in menu Model.\"" );  
choice=0;
goto loop;
}

if(choice==4)
{
 /*Save the file currently shown*/


cmd( "set curfilename [tk_getSaveFile -parent . -title \"Save File\" -initialfile $filename -initialdir $dirname]" );
s=(char *)Tcl_GetVar(inter, "curfilename",0);

if(s!=NULL && strcmp(s, ""))
 {
  cmd( "if { [file exist $dirname/$filename] == 1} {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak} {}" );
  cmd( "set file [open $curfilename w]" );
  cmd( "puts -nonewline $file [.f.t.t get 0.0 end]" );
  cmd( "close $file" );
  cmd( "set before [.f.t.t get 0.0 end]" );
  cmd( "set dirname [file dirname $curfilename]" );
  cmd( "set filename [file tail $curfilename]" );
  cmd( ".f.hea.file.dat conf -text \"$filename\"" );
  cmd( "wm title . \"$filename - LMM\"" );
 }
 choice=0;
 goto loop;
}

if(choice==5 || choice==50)
{
 /*Load the description file*/
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \" Choose an existing model or create a new one.\"" );
  choice=0;
  goto loop;
 }

  cmd( "set filename description.txt" );
  
  cmd( ".f.t.t delete 0.0 end" );
  cmd( "set choice 0; if { [ file exists $modeldir/$filename ] } { set choice 1; if { [ file size $modeldir/$filename ] <= 2 } { set choice 0; file delete $modeldir/$filename } }" );
 if(choice==1)
  {cmd( "set file [open $modeldir/description.txt]" );
   cmd( ".f.t.t insert end [read -nonewline $file]" );
   cmd( "close $file" );
   cmd( "set before [.f.t.t get 1.0 end]" );
  }
  else		// if no description, ask if the user wants to create it or not
  {
	cmd( "set answer [ tk_messageBox -parent . -type yesno -default no -icon question -title \"Create Description\" -message \"Create a description file?\" -detail \"There is no valid description file ('description.txt') set for the model\n\nDo you want to create a description file now?\n\nPress 'No' to just show the equations file.\" ]" );
	cmd( " if [ string equal $answer yes ] { set choice 1 } { set choice 2 } " );
	if ( choice == 2 )
	{
		cmd( " set filename \"\" " );
		cmd( "set before [.f.t.t get 0.0 end]" );
		choice = 8;		// load equations file
		goto loop;
	}
	cmd( ".f.t.t insert end \"Model $modelname (ver. $version)\n\n(Enter the Model description text here)\n\n(PRESS CTRL+E TO EDIT EQUATIONS)\n\"" );
  }

  cmd( ".f.t.t edit reset" );
  sourcefile=0;
  cmd( ".f.t.t mark set insert 1.0" );
  cmd( "focus -force .f.t.t" );

  cmd( ".f.hea.file.dat conf -text $filename" );
  cmd( "wm title . \"$filename - LMM\"" );
  
  cmd( "catch [unset -nocomplain ud]" );
  cmd( "catch [unset -nocomplain udi]" );
  cmd( "catch [unset -nocomplain rd]" );
  cmd( "catch [unset -nocomplain rdi]" );
  cmd( "lappend ud [.f.t.t get 0.0 end]" );
  cmd( "lappend udi [.f.t.t index insert]" );
  
  if(choice==50)
    choice=46; //go to create makefile, after the model selection
  else
    choice=0;  //just relax
  goto loop;
}
 
if(choice==7)
 {
 /*Show compilation result*/

s=(char *)Tcl_GetVar(inter, "modelname",0);
if(s==NULL || !strcmp(s, ""))
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
  choice=0;
  goto loop;
 }

create_compresult_window();
choice=0;
goto loop;

  cmd( "if [file exists $modeldir/makemessage.txt]==1 {set choice 1} {set choice 0}" );
 if(choice==1)
  {
    
    create_compresult_window();
    
cmd( "catch [unset -nocomplain ud]" );
cmd( "catch [unset -nocomplain udi]" );
cmd( "catch [unset -nocomplain rd]" );
cmd( "catch [unset -nocomplain rdi]" );
cmd( "lappend ud [.f.t.t get 0.0 end]" );
cmd( "lappend udi [.f.t.t index insert]" );

  }
  else
  {

   cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No compilation results\"" );
  choice=0;
  goto loop;

  }

  choice=0;
  goto loop;
 }

if(choice==8)
{
  /*Insert in the text window the equation file*/

cmd( ".f.t.t delete 0.0 end" );
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
  choice=0;
  goto loop;
 }
  cmd( "cd $modeldir" );
  

  make_makefile();

  cmd( "set fapp [file nativename $modeldir/makefile]" );
  s=(char *)Tcl_GetVar(inter, "fapp",0);
  
  f=fopen(s, "r");
  if(f==NULL)
   {
   
    cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"File 'makefile' not found\" -detail \"Use the 'Model Compilation Options', in menu Model, to create it.\"" );

    choice=0;
    cmd( "cd $RootLsd" );
    goto loop;
   }
  fscanf(f, "%999s", str);
  while(strncmp(str, "FUN=", 4) && fscanf(f, "%999s", str)!=EOF);    
  fclose(f);
  if(strncmp(str, "FUN=", 4)!=0)
   {
    cmd( "tk_messageBox -parent . -type ok -title Error -icon error -message \"Makefile corrupted\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
    choice=0;
    goto loop;
   }

cmd( "set filename %s.cpp", str+4 );
cmd( "cd $RootLsd" );
s=(char *)Tcl_GetVar(inter, "filename",0);

if(s==NULL || !strcmp(s, ""))
 {
  cmd( ".f.t.t delete 0.0 end" );
  cmd( "update" );
  cmd( "set filename \"\"" );
  cmd( "set dirname \"\"" );
  cmd( "set before [.f.t.t get 1.0 end]" );  
  choice=0;
  goto loop;
 }
  cmd( "focus -force .f.t.t" );
  cmd( "set dirname $modeldir" );
  cmd( "set file [open $modeldir/$filename]" );
  cmd( ".f.t.t insert end [read -nonewline $file]" );
  cmd( "close $file" );
  cmd( ".f.t.t edit reset" );
  sourcefile=1;
  cmd( "set before [.f.t.t get 1.0 end]" );

  cmd( ".f.hea.file.dat conf -text \"$filename\"" );
  cmd( "wm title . \"$filename - LMM\"" );
  cmd( "update" );
  sourcefile=1;
  choice=0;
  cmd( ".f.t.t tag add bc \"1.0\"" );
  cmd( ".f.t.t tag add fc \"1.0\"" );
  cmd( ".f.t.t mark set insert 1.0" );
color(shigh, 0, 0);			// set color types (all text)
  
cmd( "catch [unset -nocomplain ud]" );
cmd( "catch [unset -nocomplain udi]" );
cmd( "catch [unset -nocomplain rd]" );
cmd( "catch [unset -nocomplain rdi]" );
cmd( "lappend ud [.f.t.t get 0.0 end]" );
cmd( "lappend udi [.f.t.t index insert]" );

  goto loop;
 }


if(choice==10)
{

/* Find a line in the text*/
cmd( "if {[winfo exists .search_line]==1} {set choice 0; focus .search_line.e} {}" );
if(choice==0)
 goto loop;

cmd( "set line \"\"" );
cmd( "newtop .search_line \"Goto Line\" { .search_line.b.esc invoke }" );

cmd( "label .search_line.l -text \"Type the line number\"" );
cmd( "entry .search_line.e -justify center -width 10 -textvariable line" );
cmd( "frame .search_line.b" );
cmd( "button .search_line.b.ok -width -9 -text Ok -command {if {$line == \"\"} {.search_line.esc invoke} {if [ string is integer $line ] { .f.t.t see $line.0; .f.t.t tag remove sel 1.0 end; .f.t.t tag add sel $line.0 $line.500; .f.t.t mark set insert $line.0; .f.hea.line.line conf -text [.f.t.t index insert]; destroytop .search_line } { destroytop .search_line; tk_messageBox -parent .search_line -type ok -title Error -icon error -message \"Invalid value\" -detail \"Please check that a valid integer is used.\" } } }" );
cmd( "button .search_line.b.esc -width -9 -text Cancel -command {destroytop .search_line}" );
cmd( "bind .search_line <KeyPress-Return> {.search_line.b.ok invoke}" );
cmd( "bind .search_line <KeyPress-Escape> {.search_line.b.esc invoke}" );

cmd( "pack .search_line.b.ok .search_line.b.esc -padx 1 -pady 5 -side left" );
cmd( "pack .search_line.l .search_line.e" );
cmd( "pack .search_line.b -side right" );

cmd( "showtop .search_line" );
cmd( "focus .search_line.e" );

choice=0;
goto loop;
}

if(choice==11)
{
/* Find a text pattern in the text*/
cmd( "if {[winfo exists .find]==1} {set choice 0; focus .find.e} {}" );
if(choice==0)
 goto loop;

cmd( "set curcounter $lfindcounter" );
cmd( "newtop .find \"Search Text\" { .find.b.esc invoke }" );

cmd( "label .find.l -text \"Type the text to search\"" );
cmd( "entry .find.e -width 30 -textvariable textsearch" );
cmd( ".find.e selection range 0 end" );
cmd( "set docase 0" );
cmd( "checkbutton .find.c -text \"Case sensitive\" -variable docase" );
cmd( "radiobutton .find.r1 -text \"Down\" -variable dirsearch -value \"-forwards\" -command {set endsearch end}" );
cmd( "radiobutton .find.r2 -text \"Up\" -variable dirsearch -value \"-backwards\" -command {set endsearch 1.0}" );

cmd( "frame .find.b" );
cmd( "button .find.b.ok -width -9 -text Search -command {incr lfindcounter; set curcounter $lfindcounter; lappend lfind \"$textsearch\"; if {$docase==1} {set case \"-exact\"} {set case \"-nocase\"}; .f.t.t tag remove sel 1.0 end; set cur [.f.t.t index insert]; set cur [.f.t.t search $dirsearch -count length $case -- \"$textsearch\" $cur $endsearch]; if {[string length $cur] > 0} {.f.t.t tag add sel $cur \"$cur + $length char\"; if {[string compare $endsearch end]==0} {} {set length 0}; .f.t.t mark set insert \"$cur + $length char\" ; update; .f.t.t see $cur; destroytop .find} {.find.e selection range 0 end; bell}}" );

cmd( "bind .find.e <Up> {if { $curcounter >= 0} {incr curcounter -1; set textsearch \"[lindex $lfind $curcounter]\"; .find.e selection range 0 end;} {}}" );
cmd( "bind .find.e <Down> {if { $curcounter <= $lfindcounter} {incr curcounter; set textsearch \"[lindex $lfind $curcounter]\"; .find.e selection range 0 end;} {}}" );

cmd( "button .find.b.esc -width -9 -text Cancel -command {destroytop .find}" );

cmd( "bind .find <KeyPress-Return> {.find.b.ok invoke}" );
cmd( "bind .find <KeyPress-Escape> {.find.b.esc invoke}" );

cmd( "pack .find.b.ok .find.b.esc -padx 10 -pady 10 -side left" );
cmd( "pack .find.l .find.e .find.r1 .find.r2 .find.c -fill x" );
cmd( "pack .find.b -side right" );

cmd( "showtop .find" );
cmd( "focus .find.e" );

choice=0;
goto loop;
}

if(choice==12)
{
/* Search again the same pattern in the text*/

cmd( "if {$textsearch ==\"\"} {} {.f.t.t tag remove sel 1.0 end; set cur [.f.t.t index insert]; set cur [.f.t.t search -count length $dirsearch $case -- $textsearch $cur $endsearch]; if {[string length $cur] > 0} {.f.t.t tag add sel $cur \"$cur + $length char\";if {[string compare $endsearch end]==0} {} {set length 0}; .f.t.t mark set insert \"$cur + $length char\" ; update; .f.t.t see $cur; destroytop .l } {}}" );
choice=0;
goto loop;
}


if(choice==13 || choice==58)
{
 /*Run the model in the gdb debugger */
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
  choice=0;
  goto loop;
 }
  
cmd( "cd $modeldir" );

if(choice==58)
 {
 cmd( "scan $vmenuInsert %%d.%%d line col" );
 cmd( "catch { set f [open break.txt w]; puts $f \"break $filename:$line\nrun\n\"; close $f }" );
 cmd( "set cmdbreak \"--command=break.txt\"" );
 
 }
else
 cmd( "set cmdbreak \"--args\"" ); 

make_makefile();  
cmd( "set fapp [file nativename $modeldir/makefile]" );
s=(char *)Tcl_GetVar(inter, "fapp",0);
f=fopen(s, "r");

if(f==NULL)
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"Makefile not created\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
  goto end_gdb;
 }
fscanf(f, "%999s", str);
while(strncmp(str, "TARGET=", 7) && fscanf(f, "%999s", str)!=EOF);
if(strncmp(str, "TARGET=", 7)!=0)
 {
  cmd( "tk_messageBox -parent . -type ok -title Error -icon error -message \"Makefile corrupted\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
  goto end_gdb;
 }

strcpy(str1, str+7);

cmd( "if [ string equal $tcl_platform(platform) unix ] { set choice 1 } { if [ string equal $tcl_platform(os) \"Windows NT\" ] { if [ string equal $tcl_platform(osVersion) \"4.0\" ] { set choice 4 } { set choice 2 } } { set choice 3 } }" );
if(choice==1)
 {//Unix
  sprintf( msg, "catch { exec $Terminal -e gdb $cmdbreak %s & } result", str1);
 }
if(choice==2)
 {//Windows 2000, XP, 7, 8, 10...
  strcat( str1, ".exe" );
  sprintf( msg, "catch { exec $Terminal /c gdb $cmdbreak %s & } result", str1);
 }
if(choice==3)
 {//Windows 95/98
  strcat( str1, ".exe" );
  cmd( "set f [open run_gdb.bat w]; puts $f \"start gdb %s $cmdbreak &\\n\"; close $f", str1 );
  sprintf(msg, "catch { exec start run_gdb & } result");    
 }
if(choice==4)
 {//Windows NT case
  strcat( str1, ".exe" );
  cmd( "set f [open run_gdb.bat w]; puts $f \"start gdb %s $cmdbreak &\\n\"; close $f", str1 );
  sprintf(msg, "catch { exec cmd.exe /c start /min run_gdb & } result"); 
 }

// check if executable file is older than model file
s = ( char * ) Tcl_GetVar( inter, "modeldir", 0 );
sprintf( str, "%s/%s", s, str1 );
strcpy( str1, s );
s = ( char * ) Tcl_GetVar( inter, "filename", 0 );
sprintf( str2, "%s/%s", str1, s );

// get OS info for files
struct stat stExe, stMod;
if ( stat( str, &stExe ) == 0 && stat( str2, &stMod ) == 0 )
{
	if ( difftime( stExe.st_mtime, stMod.st_mtime ) < 0 )
	{
		cmd( "set answer [tk_messageBox -parent . -title Warning -icon warning -type okcancel -default cancel -message \"Old executable file\" -detail \"The existing executable file is older than the last version of the model.\n\nPress 'Ok' to continue anyway or 'Cancel' to return to LMM. Please recompile the model to avoid this message.\"]; if [ string equal $answer ok ] { set choice 1 } { set choice 2 }" );
		if ( choice == 2 )
			goto end_gdb;
	}
}
else
{
	cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"Executable not found\" -detail \"Compile the model before running it in the GDB debugger.\"" );
	goto end_gdb;
}

cmd( msg );			// if all ok, run command

end_gdb:
cmd( "cd $RootLsd" );
choice =0;
goto loop;
}


if(choice==14)
{
/* Create a new model/group */

choice=0;

// prevent creating new groups in Lsd directory
cmd( "if { [ string equal $groupdir [pwd] ] && [ file exists \"$groupdir/$LsdNew/groupinfo.txt\" ] } \
				{	set groupdir \"$groupdir/$LsdNew\"; \
					set f [open $groupdir/groupinfo.txt r]; \
					set app \"[gets $f]\"; \
					close $f; \
					set modelgroup \"$modelgroup/$app\" }");

cmd( "newtop .a \"New Model or Group\" { .a.b.esc invoke }" );

cmd( "label .a.tit -text \"Current group:\\n$modelgroup\"" );
cmd( "frame .a.f -relief groove -bd 2" );

cmd( "set temp 1" );
cmd( "radiobutton .a.f.r1 -variable temp -value 1 -text \"Create a new model in the current group\" -justify left -anchor w" );
cmd( "radiobutton .a.f.r2 -variable temp -value 2 -text \"Create a new model in a new group\" -justify left -anchor w" );
cmd( "frame .a.b" );
cmd( "button .a.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .a.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack .a.f.r1 .a.f.r2 -fill x" );
cmd( "pack .a.b.ok .a.b.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.tit .a.f -fill x" );
cmd( "pack .a.b -side right" );
cmd( "bind .a <Return> {.a.b.ok invoke}" );
cmd( "bind .a <Escape> {.a.b.esc invoke}" );
cmd( "bind .a <Up> {.a.f.r1 invoke}" );
cmd( "bind .a <Down> {.a.f.r2 invoke}" );

cmd( "showtop .a" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );


//operation cancelled
if(choice==2)
 {
  choice=0;
  goto loop;
 }

cmd( "set choice $temp" );
if(choice==2)
{
cmd( "newtop .a \"New Group\" { .a.b.esc invoke }" );

cmd( "label .a.tit -text \"Create a new group in group:\\n $modelgroup\"" );

cmd( "label .a.mname -text \"Insert new group name\"" );
cmd( "set mname \"New group\"" );
cmd( "entry .a.ename -width 30 -textvariable mname" );
cmd( "bind .a.ename <Return> {focus .a.edir; .a.edir selection range 0 end}" );

cmd( "label .a.mdir -text \"Insert a (non-existing) directory name\"" );
cmd( "set mdir \"newgroup\"" );
cmd( "entry .a.edir -width 30 -textvariable mdir" );
cmd( "bind .a.edir <Return> {focus .a.tdes}" );

cmd( "label .a.ldes -text \"Insert a short description of the group\"" );
cmd( "text .a.tdes -width 30 -heig 3" );
cmd( "bind .a.tdes <Control-e> {focus .a.b.ok}; bind .a.tdes <Control-E> {focus .a.b.ok}" );

cmd( "frame .a.b" );
cmd( "button .a.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a <Return> {.a.b.ok invoke}" );
cmd( "button .a.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.b.esc invoke}" );
cmd( "pack .a.b.ok .a.b.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.tit .a.mname .a.ename .a.mdir .a.edir .a.ldes .a.tdes" );
cmd( "pack .a.b -side right" );

cmd( "showtop .a" );
cmd( "focus .a.ename" );
cmd( ".a.ename selection range 0 end" );

here_newgroup:
choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "if {[string length $mdir] == 0 || [string length $mname]==0} {set choice 2} {}" );

//operation cancelled
if(choice==2)
 {cmd( "destroytop .a" );
  choice=0;
  goto loop;
 }
cmd( "if {[llength [split $mdir]]>1} {set choice -1} {}" );
if(choice==-1)
 {cmd( "tk_messageBox -parent .a -type ok -title Error -icon error -message \"Space in path\" -detail \"Directory name must not contain spaces.\"" );
  cmd( "focus .a.edir" );
  cmd( ".a.edir selection range 0 end" );
  goto here_newgroup;
 } 
//control for existing directory
cmd( "if {[file exists $groupdir/$mdir] == 1} {tk_messageBox -parent .a -type ok -title Error -icon error -message \"Cannot create directory\" -detail \"$groupdir/$mdir\\n\\nPossibly there is already such a directory.\nCreation of new group aborted.\"; set choice 3} {}" );
if(choice==3)
 {cmd( "destroytop .a" );
  choice=0;
  goto loop;
 } 


cmd( "file mkdir $groupdir/$mdir" );
cmd( "cd $groupdir/$mdir" );
cmd( "set groupdir \"$groupdir/$mdir\"" );

cmd( "set f [open groupinfo.txt w]; puts -nonewline $f \"$mname\"; close $f" );
cmd( "set f [open description.txt w]; puts -nonewline $f \"[.a.tdes get 0.0 end]\"; close $f" );
cmd( "set modelgroup $modelgroup/$mname" );

cmd( "destroytop .a" );
//end of creation of a new group
}
else
 cmd( "cd $groupdir" ); //if no group is created, move in the current group

//create a new model
choice=0;
cmd( "newtop .a \"New Model\" { .a.b.esc invoke }" );

cmd( "label .a.tit -text \"Create a new model in group:\\n $modelgroup\"" );

cmd( "label .a.mname -text \"Insert new model name\"" );
cmd( "set mname \"New model\"" );
cmd( "entry .a.ename -width 30 -textvariable mname" );
cmd( "bind .a.ename <Return> {focus .a.ever; .a.ever selection range 0 end}" );

cmd( "label .a.mver -text \"Insert a version number\"" );
cmd( "set mver \"0.1\"" );
cmd( "entry .a.ever -width 30 -textvariable mver" );
cmd( "bind .a.ever <Return> {focus .a.edir; .a.edir selection range 0 end}" );

cmd( "label .a.mdir -text \"Insert a (non-existing) directory name\"" );
cmd( "set mdir \"New\"" );
cmd( "entry .a.edir -width 30 -textvariable mdir" );
cmd( "bind .a.edir <Return> {focus .a.b.ok}" );

cmd( "frame .a.b" );
cmd( "button .a.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.b.ok <Return> {.a.b.ok invoke}" );
cmd( "bind .a <Escape> {.a.b.esc invoke}" );
cmd( "button .a.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack .a.b.ok .a.b.esc -padx 10 -pady 10 -side left -fill x" );
cmd( "pack .a.tit .a.mname .a.ename .a.mver .a.ever .a.mdir .a.edir" );
cmd( "pack .a.b -side right" );

cmd( "showtop .a" );
cmd( "focus .a.ename" );
cmd( ".a.ename selection range 0 end" );

loop_copy_new:
while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "if {[string length $mdir] == 0 || [string length $mname]==0} {set choice 2} {}" );
//operation cancelled
if(choice==2)
 {cmd( "destroytop .a" );
  choice=0;
  goto loop;
 }

cmd( "if {[llength [split $mdir]]>1} {set choice -1} {}" );
if(choice==-1)
 {cmd( "tk_messageBox -parent .a -type ok -title Error -icon error -message \"Space in path\" -detail \"Directory name must not contain spaces.\"" );
  cmd( "focus .a.edir" );
  cmd( ".a.edir selection range 0 end" );
  goto here_newgroup;
 } 

//control for existing directory
cmd( "if {[file exists $mdir] == 1} {tk_messageBox -parent .a -type ok -title Error -icon error -message \"Cannot create directory\" -detail \"$mdir\\n\\nPossibly there is already such a directory.\"; set choice 3} {}" );
if(choice==3)
 {choice=0;
  goto loop_copy_new;
 } 

//control for an existing model with the same name AND same version
cmd( "set dir [glob *]" );
cmd( "set num [llength $dir]" );
strcpy(str, " ");
for(i=0; i<num; i++)
 {
  cmd( "if {[file isdirectory [lindex $dir %d] ] == 1} {set curdir [lindex $dir %i]} {set curdir ___}", i, i );
  s=(char *)Tcl_GetVar(inter, "curdir",0);
  strcpy(str, s);
  if(strcmp(str,"___") && strcmp(str, "gnu")  && strcmp(str, "gnu64") && strcmp(str, "src") && strcmp(str, "Manual") && strcmp(str, "R") )
   {
    cmd( "set ex [file exists $curdir/modelinfo.txt]" );
    cmd( "if { $ex == 0 } {set choice 0} {set choice 1}" );
    if(choice==1)
    {
      cmd( "set f [open $curdir/modelinfo.txt r]" );
      cmd( "set cname [gets $f]; set cver [gets $f];" );
      cmd( "close $f" );
    } 
    else
      cmd( "set cname $curdir; set cver \"0.1\"" );
    cmd( "set comp [string compare $cname $mname]" );
    cmd( "set comp1 [string compare $cver $mver]" );
    cmd( "if {$comp == 0 && $comp1 == 0} {set choice 3; set errdir $curdir} {}" );
    cmd( "if {$comp == 0 } {set choice 4; set errdir $curdir} {}" );

   }

 }
if(choice==3)
 {cmd( "tk_messageBox -parent .a -type ok -title Error -icon error -message \"Cannot create model\" -detail \"Cannot create the new model '$mname' (ver. $mver) because it already exists (directory: $errdir).\"" );
  choice=0;
  goto loop_copy_new;
 } 
if(choice==4)
 {
 choice=0;
 cmd( "set answer [tk_messageBox -parent .a -type okcancel -title Warning -icon warning -default cancel -message \"Model already exists\" -detail \"A model named '$mname' already exists (ver. $mver).\\n\\nIf you want the new model to inherit the same equations, data etc. of that model you should cancel this operation, and use the 'Save Model As...' command. Or press 'Ok' to continue creating a new (empty) model '$mname'.\"]" );
  s=(char *)Tcl_GetVar(inter, "answer",0);

  cmd( "if {[string compare $answer ok] == 0} {set choice 1} {set choice 0}" );
  if(choice==0)
   {cmd( "destroytop .a" );
    goto loop;
   } 
 } 
 

//here we are: create a new empty model

cmd( "set dirname $groupdir/$mdir" );
cmd( "set modeldir $groupdir/$mdir" );
cmd( "set modelname $mname" );
cmd( "set version $mver" );
cmd( ".f.hea.mod.dat conf -text \"$modelname\"" );
cmd( ".f.hea.ver.dat conf -text \"$version\"" );
cmd( ".f.hea.grp.dat conf -text \"$modelgroup\"" );
cmd( "destroytop .a" );


cmd( "file mkdir $dirname" );


if( macro )
 cmd( "set fun_base \"fun_baseMacro.cpp\"" );
else 
 cmd( "set fun_base \"fun_baseC.cpp\"" );


//create the empty equation file
cmd( "file copy $RootLsd/$LsdSrc/$fun_base $modeldir/fun_$mdir.cpp" ); 
 

cmd( "cd $dirname" );

//create the model_options.txt file
cmd( "set dir [glob *.cpp]" );
cmd( "set b [lindex $dir 0]" );
cmd( "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-O3\\nSWITCH_CC_LNK=\\n\"" );
cmd( "set f [open model_options.txt w]" );
cmd( "puts $f $a" );
cmd( "close $f" );


//insert the modelinfo file, 
cmd( "set before [.f.t.t get 1.0 end]" );
cmd( "set f [open modelinfo.txt w]" );
cmd( "puts $f \"$modelname\"" );
cmd( "puts $f \"$version\"" );
cmd( "set frmt \"%d %%B, %%Y\"" );
cmd( "puts $f \"[clock format [clock seconds] -format \"$frmt\"]\"" );
cmd( "close $f" );

cmd( "cd $RootLsd" );

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
cmd( ".m.model entryconf 11 -state normal" );


cmd( "tk_messageBox -parent . -type ok -title \"Model Created\" -icon info -message \"Model '$mname' created\" -detail \"Version: $mver\nDirectory: $dirname\"" );

cmd( "set before [.f.t.t get 0.0 end]" ); //avoid to re-issue a warning for non saved files
choice=50;
goto loop;

}


if(choice==15)
{
/* open a text file*/

cmd( "set brr [tk_getOpenFile -parent . -title \"Load Text File\" -initialdir $dirname]" );
cmd( "if {[string length $brr] == 0} {set choice 0} {set choice 1}" );
if(choice==0)
 goto loop;

cmd( "if {[file exists $brr] == 1} {set choice 1} {set choice 0}" );
if(choice==1)
 {
  cmd( ".f.t.t delete 1.0 end; set file [open $brr]; .f.t.t insert end [read -nonewline $file]; .f.t.t edit reset; close $file; set before [.f.t.t get 1.0 end]; set dirname [file dirname $brr]; set filename [file tail $brr]; .f.hea.file.dat conf -text \"$filename\"" );
    cmd( "wm title . \"$filename - LMM\"" );          

 }
else
 goto loop; 

cmd( ".f.t.t mark set insert 1.0" );
cmd( "set s [file extension $filename]" );
cmd( "catch [unset -nocomplain ud]" );
cmd( "catch [unset -nocomplain udi]" );
cmd( "catch [unset -nocomplain rd]" );
cmd( "catch [unset -nocomplain rdi]" );
cmd( "lappend ud [.f.t.t get 0.0 end]" );
cmd( "lappend udi [.f.t.t index insert]" );

s=(char *)Tcl_GetVar(inter, "s",0);
choice=0;
if(s[0]!='\0')
 {strcpy(str, s);
  if(!strcmp(str, ".cpp") || !strcmp(str, ".c") || !strcmp(str, ".C") || !strcmp(str, ".CPP") || !strcmp(str, ".Cpp") || !strcmp(str, ".c++") || !strcmp(str, ".C++") || !strcmp(str, ".h") || !strcmp(str, ".H"))
   {sourcefile=1;
	color(shigh, 0, 0);			// set color types (all text)
   }
  else
   sourcefile=0;
 }
goto loop;
}


if(choice == 16)
{
/*insert automatic tabs */
cmd( "set in [.f.t.t index insert]" );
cmd( "scan $in %%d.%%d line col" );
cmd( "set line [expr $line -1]" );
cmd( "set s [.f.t.t get $line.0 $line.end]" );
s=(char *)Tcl_GetVar(inter, "s",0);
for(i=0; s[i]==' '; i++)
  str[i]=' ';
if(i>0)
{str[i]=(char)NULL;
 cmd( ".f.t.t insert insert \"%s\"", str );
}
choice=0;
goto loop;

}

if(choice==17)
{
/* find matching parenthesis */

cmd( "set sym [.f.t.t get insert]" );
cmd( "if {[string compare $sym \"\\{\"]!=0} {if {[string compare $sym \"\\}\"]!=0} {set choice 0} {set num -1; set direction -backwards; set fsign +; set sign \"\"; set terminal \"1.0\"}} {set num 1; set direction -forwards; set fsign -; set sign +; set terminal end}" );

if(choice==0)
 goto loop;
cmd( "set cur [.f.t.t index insert]" );
cmd( ".f.t.t tag add sel $cur \"$cur + 1char\"" );
if (num>0)
  cmd( "set cur [.f.t.t index \"insert + 1 char\"]" );
while(num!=0 && choice!=0)
{
 cmd( "set a [.f.t.t search $direction \"\\{\" $cur $terminal]" );
 cmd( "if {$a==\"\"} {set a [.f.t.t index $terminal]} {}" );
 cmd( "set b [.f.t.t search $direction \"\\}\" $cur $terminal]" );
 cmd( "if {$b==\"\"} {set b [.f.t.t index $terminal]} {}" );
 cmd( "if {$a==$b} {set choice 0} {}" );
 if(choice==0)
  goto loop;
 if(num>0)
   cmd( "if {[.f.t.t compare $a < $b]} {set num [expr $num + 1]; set cur [.f.t.t index \"$a+1char\"]} {set num [expr $num - 1]; set cur [.f.t.t index \"$b+1char\"]}" );
 else
   cmd( "if {[.f.t.t compare $a > $b]} {set num [expr $num + 1]; set cur [.f.t.t index $a]} {set num [expr $num - 1]; set cur [.f.t.t index $b]}" );


}
choice=0;


cmd( " if { [string compare $sign \"+\"]==0 } {.f.t.t tag add sel \"$cur - 1char\" $cur ; set num 1} {.f.t.t tag add sel $cur \"$cur + 1char\"; set num 0}" );

cmd( ".f.t.t see $cur" );
goto loop;
}


if(choice==23)
{ //reset colors after a sign for coloring

if(sourcefile==0)
 {choice=0;
  goto loop;
 }
else
{	
  choice=0;
  // text window not ready?
  if(Tcl_GetVar(inter,"curPosIni",0) == NULL)			
	  goto loop;
  // check if inside or close to multi-line comment and enlarge region appropriately
  cmd( "if {[lsearch -exact [.f.t.t tag names $curPosIni] comment1] != -1} {set curPosFin [.f.t.t search */ $curPosIni end]; set newPosIni [.f.t.t search -backwards /* $curPosIni 1.0]; set curPosIni $newPosIni} {if {[.f.t.t search -backwards */ $curPosIni \"$curPosIni linestart\"] != \"\"} {set comIni [.f.t.t search -backwards /* $curPosIni]; if {$comIni != \"\"} {set curPosIni $comIni}}; if {[.f.t.t search /* $curPosFin \"$curPosFin lineend\"] != \"\"} {set comFin [.f.t.t search */ $curPosFin]; if {$comFin != \"\"} {set curPosFin $comFin}}}" );
  // find the range of lines to reeval the coloring
  char *curPosIni=(char*)Tcl_GetVar(inter,"curPosIni",0); // position before insertion
  char *curPosFin=(char*)Tcl_GetVar(inter,"curPosFin",0); // position after insertion
  char *curSelIni=(char*)Tcl_GetVar(inter,"curSelIni",0); // selection before insertion
  char *curSelFin=(char*)Tcl_GetVar(inter,"curSelFin",0); // selection after insertion
  // collect all selection positions, before and after change
  float curPos[6];
  int nVal = 0;
  i = sscanf(curPosIni, "%f", &curPos[nVal]);
  nVal += i < 0 ? 0 : i;
  i = sscanf(curPosFin, "%f", &curPos[nVal]);
  nVal += i < 0 ? 0 : i;
  i = sscanf(curSelIni, "%f %f", &curPos[nVal], &curPos[nVal + 1]);
  nVal += i < 0 ? 0 : i;
  i = sscanf(curSelFin, "%f %f", &curPos[nVal], &curPos[nVal + 1]);
  nVal += i < 0 ? 0 : i;
  // find previous and next lines to select (worst case scenario)
  long prevLin = (long)floor(curPos[0]);
  long nextLin = (long)floor(curPos[0]) + 1;
  for(i = 1; i < nVal; i++)
  {
	  if(curPos[i] < prevLin) prevLin = (long)floor(curPos[i]);
	  if(curPos[i] + 1 > nextLin) nextLin = (long)floor(curPos[i]) + 1;
  }
  
color(shigh, prevLin, nextLin);
}
goto loop;

}

if(choice==21)
{
/* Find and replace a text pattern in the text*/
cmd( "if {[winfo exists .l]==1} {set choice 0; focus .l.e} {}" );
if(choice==0)
 goto loop;


cmd( "set cur \"\"" );
cmd( "newtop .l \"Replace Text\" { .l.b2.esc invoke }" );

cmd( "label .l.l -text \"Type the text to search\"" );
cmd( "entry .l.e -width 30 -textvariable textsearch" );
cmd( "label .l.r -text \"Type the text to replace\"" );
cmd( "entry .l.s -width 30 -textvariable textrepl" );

cmd( "radiobutton .l.r1 -text \"Down\" -variable dirsearch -value \"-forwards\" -command {set endsearch end}" );
cmd( "radiobutton .l.r2 -text \"Up\" -variable dirsearch -value \"-backwards\" -command {set endsearch 1.0}" );
cmd( "checkbutton .l.c -text \"Case sensitive\" -variable docase" );

cmd( "frame .l.b1" );
cmd( "button .l.b1.repl -width -9 -state disabled -text Replace -command {if {[string length $cur] > 0} {.f.t.t delete $cur \"$cur + $length char\"; .f.t.t insert $cur \"$textrepl\"; if {[string compare $endsearch end]==0} {} {.f.t.t mark set insert $cur}; .l.b2.ok invoke} {}}" );
cmd( "button .l.b1.all -width -9 -state disabled -text \"Repl. All\" -command {set choice 4}" );
cmd( "frame .l.b2" );
cmd( "button .l.b2.ok -width -9 -text Search -command {if { [string length \"$textsearch\"]==0} {} {.f.t.t tag remove found 1.0 end; if {$docase==1} {set case \"-exact\"} {set case -nocase}; .f.t.t tag remove sel 1.0 end; set cur [.f.t.t index insert]; set cur [.f.t.t search $dirsearch -count length $case -- $textsearch $cur $endsearch]; if {[string length $cur] > 0} {.f.t.t tag add found $cur \"$cur + $length char\"; if {[string compare $endsearch end]==0} {.f.t.t mark set insert \"$cur + $length char\" } {.f.t.t mark set insert $cur}; update; .f.t.t see $cur; .l.b1.repl conf -state normal; .l.b1.all conf -state normal} {.l.b1.all conf -state disabled; .l.b1.repl conf -state disabled}}}" );
cmd( "button .l.b2.esc -width -9 -text Cancel -command {focus -force .f.t.t; set choice 5}" );
cmd( "bind .l <KeyPress-Return> {.l.b2.ok invoke}" );
cmd( "bind .l <KeyPress-Escape> {.l.b2.esc invoke}" );
cmd( "pack .l.b1.repl .l.b1.all -padx 10 -pady 5 -side left" );
cmd( "pack .l.b2.ok .l.b2.esc -padx 10 -pady 5 -side left" );
cmd( "pack .l.l .l.e .l.r .l.s .l.r1 .l.r2 .l.c" );
cmd( "pack .l.b1" );
cmd( "pack .l.b2 -side right" );

cmd( "showtop .l" );
cmd( "focus .l.e" );
cmd( ".f.t.t tag conf found -background red -foreground white" );

choice=0;
here:
while(choice==0)
 Tcl_DoOneEvent(0);

if(choice==3)
  {choice=0;
   goto here;
  }

while(choice==4)
 { 
  cmd( "if {[string length $cur] > 0} {.f.t.t delete $cur \"$cur + $length char\"; .f.t.t see $cur; .f.t.t insert $cur \"$textrepl\"; if {[string compare $endsearch end]==0} {set cur [.f.t.t index \"$cur+$length char\"]} {}} {set choice 0}" );
if(choice!=0)  
  cmd( "set cur [.f.t.t search $dirsearch -count length $case -- $textsearch $cur $endsearch]" ); 
  cmd( "raise .l" );
  cmd( "focus .l" );
  Tcl_DoOneEvent(0);  
  goto here;
 }

cmd( "destroytop .l" );

cmd( ".f.t.t tag remove found 1.0 end" );
color(shigh, 0, 0);				// reevaluate colors
choice=0;
goto loop;
}


if ( choice == 25 && macro )
{
/*
Insert a Lsd equation
*/

choice=0;

cmd( "newtop .a \"Insert an Equation\" { .a.b.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the label of the variable\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.b.ok}" );

cmd( "frame .a.f -relief groove -bd 2" );

cmd( "set isfun 0" );
cmd( "radiobutton .a.f.r1 -variable isfun -value 0 -text \"Equation:\\nthe code will provide one value for every variable of this type at each time step.\\nThe code is executed only once for each t, re-using the same value if requested many times.\\nEven if no other variable requests this value, it the code is computed.\" -justify left -relief groove" );
cmd( "radiobutton .a.f.r2 -variable isfun -value 1 -text \"Function:\\nthe code is executed again any time this variable is requested by other variables' code,\\nbut it is not computed by default. This code is not computed if no variable\\nneeds this variable's value.\" -justify left -relief groove" );
cmd( "pack .a.f.r1 .a.f.r2 -anchor w " );


cmd( "frame .a.b" );	
cmd( "button .a.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.b.ok <Return> {.a.b.ok invoke}" );
cmd( "button .a.b.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#equation}" );
cmd( "button .a.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.b.esc invoke}" );
cmd( "pack .a.b.ok .a.b.help .a.b.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.label .a.f" );
cmd( "pack .a.b -side right" );

cmd( "showtop .a" );
cmd( "focus .a.label" );
cmd( ".a.label selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd( "if {$isfun == 1} {set choice 3} {}" );

cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

if(choice!=3)
  cmd( ".f.t.t insert insert \"EQUATION(\\\"$v_label\\\")\\n\"" );
else
  cmd( ".f.t.t insert insert \"FUNCTION(\\\"$v_label\\\")\\n\"" );

cmd( ".f.t.t insert insert \"/*\\nComment\\n*/\\n\\n\"" );
cmd( ".f.t.t insert insert \"RESULT( )\\n\"" );
cmd( ".f.t.t mark set insert \"$a + 2 line\"" );
cmd( ".f.t.t tag add sel insert \"insert + 7 char\"" );

v_counter=0;
cmd( "set v_num 0" );


cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 25 && ! macro )
{
/*
Insert a Lsd equation
*/
choice=0;


cmd( "newtop .a \"Insert an Equation\" { .a.b.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the label of the variable\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.b.ok}" );

cmd( "frame .a.f -relief groove -bd 2" );

cmd( "set isfun 0" );
cmd( "radiobutton .a.f.r1 -variable isfun -value 0 -text \"Equation:\\nthe code will provide one value for every variable of this type at each time step.\\nThe code is executed only once for each t, re-using the same value if requested many times.\\nEven if no other variable requests this value, it the code is computed.\" -justify left -relief groove" );
cmd( "radiobutton .a.f.r2 -variable isfun -value 1 -text \"Function:\\nthe code is executed again any time this variable is requested by other variables' code,\\nbut it is not computed by default. This code is not computed if no variable\\nneeds this variable's value.\" -justify left -relief groove" );
cmd( "pack .a.f.r1 .a.f.r2 -anchor w " );


cmd( "frame .a.b" );	
cmd( "button .a.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.b.ok <Return> {.a.b.ok invoke}" );
cmd( "button .a.b.help -width -9 -text Help -command {LsdHelp lsdfunc.html#equation}" );
cmd( "button .a.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.b.esc invoke}" );
cmd( "pack .a.b.ok .a.b.help .a.b.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.label .a.f" );
cmd( "pack .a.b -side right" );

cmd( "showtop .a" );
cmd( "focus .a.label" );
cmd( ".a.label selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd( "if {$isfun == 1} {set choice 3} {}" );

cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( ".f.t.t insert insert \"if(!strcmp(label,\\\"$v_label\\\"))\\n\"" );
cmd( ".f.t.t insert insert \"{\\n\"" );
cmd( ".f.t.t insert insert \"/*\\nComment\\n*/\\n\"" );
if(choice==3)
 {
 cmd( ".f.t.t insert insert \"last_update--;//repeat the computation any time is requested\\n\"" );
 cmd( ".f.t.t insert insert \"if(c==NULL)//Avoids to be computed when the system activates the equation\\n\"" );
 cmd( ".f.t.t insert insert \"{\\n\"" );
 cmd( ".f.t.t insert insert \"res=-1;\\n\"" );
 cmd( ".f.t.t insert insert \"goto end;\\n\"" );
 cmd( ".f.t.t insert insert \"}\\n\"" );
 }
cmd( ".f.t.t insert insert \"\\n\"" );
cmd( ".f.t.t insert insert \"res=;\\n\"" );
cmd( ".f.t.t insert insert \"goto end;\\n}\\n\"" );
cmd( ".f.t.t mark set insert \"$a + 3 line\"" );
cmd( ".f.t.t tag add sel insert \"insert + 7 char\"" );

v_counter=0;
cmd( "set v_num 0" );
cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 26 && macro )
{
/*
Insert a v[0]=p->cal("Var",0);
*/
choice=0;
cmd( "newtop .a \"Insert a 'V(...)' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result\"" );

cmd( "set v_num %d", v_counter );
cmd( "entry .a.v_num -width 2 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the variable to compute\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the lag to be used\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object to which request the computation\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#V}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
 cmd( "set a [.f.t.t index insert]" );

cmd( "if {$v_num!=\"\" && [string is integer $v_num]} {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}" );

cmd( "if {$v_lag==0 && $v_obj==\"p\"} { .f.t.t insert insert \"V(\\\"$v_label\\\")\"} {}" );
cmd( "if {$v_lag!=0 && $v_obj==\"p\"} { .f.t.t insert insert \"VL(\\\"$v_label\\\",$v_lag)\"} {}" );
cmd( "if {$v_lag==0 && $v_obj!=\"p\"} { .f.t.t insert insert \"VS($v_obj,\\\"$v_label\\\")\"} {}" );
cmd( "if {$v_lag!=0 && $v_obj!=\"p\" && [string is integer $v_lag]} { .f.t.t insert insert \"VLS($v_obj,\\\"$v_label\\\",$v_lag)\"} {}" );

cmd( "if {$v_num!=\"\"} {.f.t.t insert insert \";\"} {}" );

cmd( "if {$v_num==\"\"} { set num -1} {set num $v_num}" );
if(num!=-1)
 v_counter=++num;

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 26 && ! macro )
{
/*
Insert a v[0]=p->cal("Var",0);
*/
choice=0;
cmd( "newtop .a \"Insert a 'cal'\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result\"" );

cmd( "set v_num %d", v_counter );
cmd( "entry .a.v_num -width 2 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the variable to compute\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the lag to be used\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object to which request the computation\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#cal}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

cmd( "if {$v_num==\"\" && [string is integer $v_num] && [string is integer $v_lag]} {.f.t.t insert insert \"$v_obj->cal(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->cal(\\\"$v_label\\\",$v_lag);\"; incr v_num}" );

cmd( "if {$v_num==\"\"} { set num -1} {set num $v_num}" );
if(num!=-1)
 v_counter=num;

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 27 && macro )
{
/*
Insert a cycle for(cur=p->search("Label"); cur!=NULL; cur=go_brother(cur))

*/
choice=0;
cmd( "newtop .a \"Insert a 'CYCLE' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the label of the object to cycle through\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.obj; .a.obj selection range 0 end}" );


cmd( "label .a.l2 -text \"Type below the cycling pointer\"" );
cmd( "set v_obj cur" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.par; .a.par selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the parent pointer\"" );
cmd( "set v_par p" );
cmd( "entry .a.par -width 6 -textvariable v_par -justify center" );
cmd( "bind .a.par <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#CYCLE}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.label .a.l2 .a.obj .a.l3 .a.par" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.label" );
cmd( ".a.label selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

cmd( "if { $v_par == \"p\"} {.f.t.t insert insert \"CYCLE($v_obj,\\\"$v_label\\\")\\n\"} {.f.t.t insert insert \"CYCLES($v_par, $v_obj,\\\"$v_label\\\")\\n\"}" );
//cmd( ".f.t.t insert insert \" {\\n  \\n }\\n\"" ); 
//Trying making automatic indent in cycles

cmd( "set in [.f.t.t index insert]" );
cmd( "scan $in %%d.%%d line col" );
cmd( "set line [expr $line -1]" );
cmd( "set s [.f.t.t get $line.0 $line.end]" );
s=(char *)Tcl_GetVar(inter, "s",0);
for(i=0; s[i]==' '; i++)
  str[i]=' ';
str[i]=' ';  
i++;  
if(i>0)
{str[i]=(char)NULL;
 cmd( ".f.t.t insert insert \"%s\"", str );
}
cmd( ".f.t.t insert insert \"{\\n\"" );

if(i>0)
{str[i]=(char)NULL;
 cmd( ".f.t.t insert insert \"%s \"", str );
 cmd( "set b [.f.t.t index insert]" );
}

if(i>0)
{str[i]=(char)NULL;
 cmd( ".f.t.t insert insert \"\\n%s\"", str );
}

cmd( ".f.t.t insert insert \"}\\n\"" );
cmd( ".f.t.t mark set insert \"$b\"" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;

}

if ( choice == 27 && ! macro )
{
/*
Insert a cycle for(cur=p->search("Label"); cur!=NULL; cur=go_brother(cur))

*/
choice=0;
cmd( "newtop .a \"Insert a 'for' Cycle\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the label of the object to cycle through\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.obj; .a.obj selection range 0 end}" );


cmd( "label .a.l2 -text \"Type below the cycling pointer\"" );
cmd( "set v_obj cur" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.par; .a.par selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the parent pointer\"" );
cmd( "set v_par p" );
cmd( "entry .a.par -width 6 -textvariable v_par -justify center" );
cmd( "bind .a.par <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#basicc}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.label .a.l2 .a.obj .a.l3 .a.par" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.label" );
cmd( ".a.label selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

cmd( ".f.t.t insert insert \"for($v_obj=$v_par->search(\\\"$v_label\\\");$v_obj!=NULL;$v_obj=go_brother($v_obj))\\n\"" );
cmd( ".f.t.t insert insert \" {\\n  \\n }\\n\"" );
cmd( ".f.t.t mark set insert \"$a + 2 line + 2 char\"" );


cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;

}


if ( choice == 28 && macro )
{
/*
MACRO VERSION
Insert a Lsd script, to be used in Lsd equations' code
*/


choice=0;

cmd( "newtop .a \"Insert a Lsd Script\" { .a.f.esc invoke }" );

cmd( "set res 26" );
cmd( "label .a.tit -text \"Choose one of the following options\nThe interface will request the necessary information\" -justify center" );
cmd( "frame .a.r -bd 2 -relief groove" );
cmd( "radiobutton .a.r.equ -text \"EQUATION/FUNCTION - insert a new Lsd equation\" -underline 0 -variable res -value 25" );
cmd( "radiobutton .a.r.cal -text \"V(...) - request the value of a variable\" -underline 0 -variable res -value 26" );
cmd( "radiobutton .a.r.sum -text \"SUM - compute the sum of a variable over a set of objects\" -underline 1 -variable res -value 56" );
cmd( "radiobutton .a.r.sear -text \"SEARCH - search the first instance an object type\" -underline 2 -variable res -value 55" );
cmd( "radiobutton .a.r.scnd -text \"SEARCH_CND - conditional search a specific object in the model\" -underline 0 -variable res -value 30" );
cmd( "radiobutton .a.r.lqs -text \"SORT - sort a group of objects\" -underline 3 -variable res -value 31" );
cmd( "radiobutton .a.r.addo -text \"ADDOBJ - add a new object\" -underline 3 -variable res -value 52" );
cmd( "radiobutton .a.r.delo -text \"DELETE - delete an object\" -underline 0 -variable res -value 53" );
cmd( "radiobutton .a.r.rndo -text \"RNDDRAW - draw an object\" -underline 1 -variable res -value 54" );
cmd( "radiobutton .a.r.wri -text \"WRITE - overwrite a variable or parameter with a new value\" -underline 0 -variable res -value 29" );
cmd( "radiobutton .a.r.incr -text \"INCR - increment the value of a parameter\" -underline 0 -variable res -value 40" );
cmd( "radiobutton .a.r.mult -text \"MULT - multiply the value of a parameter\" -underline 0 -variable res -value 45" );
cmd( "radiobutton .a.r.for -text \"CYCLE - insert a cycle over a group of objects\" -underline 0 -variable res -value 27" );
cmd( "radiobutton .a.r.math -text \"Insert a mathematical/statistical function\" -underline 12 -variable res -value 51" );


cmd( "bind .a <KeyPress-e> {.a.r.equ invoke; set choice 1}; bind .a <KeyPress-E> {.a.r.equ invoke; set choice 1}" );
cmd( "bind .a <KeyPress-v> {.a.r.cal invoke; set choice 1}; bind .a <KeyPress-V> {.a.r.cal invoke; set choice 1}" );
cmd( "bind .a <KeyPress-u> {.a.r.sum invoke; set choice 1}; bind .a <KeyPress-U> {.a.r.sum invoke; set choice 1}" );
cmd( "bind .a <KeyPress-s> {.a.r.scnd invoke; set choice 1}; bind .a <KeyPress-S> {.a.r.scnd invoke; set choice 1}" );
cmd( "bind .a <KeyPress-a> {.a.r.sear invoke; set choice 1}; bind .a <KeyPress-A> {.a.r.sear invoke; set choice 1}" );
cmd( "bind .a <KeyPress-w> {.a.r.wri invoke; set choice 1}; bind .a <KeyPress-W> {.a.r.wri invoke; set choice 1}" );
cmd( "bind .a <KeyPress-c> {.a.r.for invoke; set choice 1}; bind .a <KeyPress-C> {.a.r.for invoke; set choice 1}" );
cmd( "bind .a <KeyPress-t> {.a.r.lqs invoke; set choice 1}; bind .a <KeyPress-T> {.a.r.lqs invoke; set choice 1}" );
cmd( "bind .a <KeyPress-o> {.a.r.addo invoke; set choice 1}; bind .a <KeyPress-O> {.a.r.addo invoke; set choice 1}" );
cmd( "bind .a <KeyPress-d> {.a.r.delo invoke; set choice 1}; bind .a <KeyPress-D> {.a.r.delo invoke; set choice 1}" );
cmd( "bind .a <KeyPress-i> {.a.r.incr invoke; set choice 1}; bind .a <KeyPress-I> {.a.r.incr invoke; set choice 1}" );
cmd( "bind .a <KeyPress-m> {.a.r.mult invoke; set choice 1}; bind .a <KeyPress-M> {.a.r.mult invoke; set choice 1}" );
cmd( "bind .a <KeyPress-h> {.a.r.math invoke; set choice 1}; bind .a <KeyPress-H> {.a.r.math invoke; set choice 1}" );
cmd( "bind .a <KeyPress-n> {.a.r.rndo invoke; set choice 1}; bind .a <KeyPress-N> {.a.r.rndo invoke; set choice 1}" );

cmd( "frame .a.f" );
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp LMM_help.html#LsdScript}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );

cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.r.equ .a.r.cal .a.r.sum .a.r.sear .a.r.scnd .a.r.lqs .a.r.addo .a.r.delo .a.r.rndo .a.r.wri .a.r.incr .a.r.mult .a.r.for .a.r.math -anchor w" );
cmd( "pack .a.tit .a.r" );
cmd( "pack .a.f -side right" );
cmd( "bind .a <Return> {.a.f.ok invoke}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );

cmd( "showtop .a" );
cmd( "focus .a.r.cal" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }

cmd( "set choice $res" );
goto loop;

}

if ( choice == 28 && ! macro )
{
/*
C++ VERSION
Insert a Lsd script, to be used in Lsd equations' code
*/


choice=0;

cmd( "newtop .a \"Insert a Lsd Script\" { .a.f.esc invoke }" );

cmd( "set res 26" );
cmd( "label .a.tit -text \"Choose one of the following options. The interface will request the necessary information\" -justify center" );
cmd( "frame .a.r -bd 2 -relief groove" );
cmd( "radiobutton .a.r.equ -text \"EQUATION/FUNCTION - insert a new Lsd equation\" -underline 0 -variable res -value 25" );
cmd( "radiobutton .a.r.cal -text \"cal - request the value of a variable\" -underline 0 -variable res -value 26" );
cmd( "radiobutton .a.r.sum -text \"sum - compute the sum of a variable over a set of objects\" -underline 1 -variable res -value 56" );
cmd( "radiobutton .a.r.sear -text \"search - search the first instance an object type\" -underline 3 -variable res -value 55" );
cmd( "radiobutton .a.r.scnd -text \"search_var_cond - conditional search a specific object in the model\" -underline 0 -variable res -value 30" );
cmd( "radiobutton .a.r.lqs -text \"lsdqsort - sort a group of objects\" -underline 7 -variable res -value 31" );
cmd( "radiobutton .a.r.addo -text \"add_n_objects2 - add new objects\" -underline 0 -variable res -value 52" );
cmd( "radiobutton .a.r.delo -text \"delete_obj - delete an object\" -underline 0 -variable res -value 53" );
cmd( "radiobutton .a.r.rndo -text \"draw_rnd - draw an object\" -underline 6 -variable res -value 54" );
cmd( "radiobutton .a.r.wri -text \"write - overwrite a variable or parameter with a new value\" -underline 0 -variable res -value 29" );
cmd( "radiobutton .a.r.incr -text \"increment - increment the value of a parameter\" -underline 0 -variable res -value 40" );
cmd( "radiobutton .a.r.mult -text \"multiply - multiply the value of a parameter\" -underline 0 -variable res -value 45" );
cmd( "radiobutton .a.r.for -text \"for - insert a cycle over a group of objects\" -underline 0 -variable res -value 27" );
cmd( "radiobutton .a.r.math -text \"Insert a mathematical/statistical function\" -underline 12 -variable res -value 51" );

cmd( "bind .a <KeyPress-e> {.a.r.equ invoke; set choice 1}; bind .a <KeyPress-E> {.a.r.equ invoke; set choice 1}" );
cmd( "bind .a <KeyPress-c> {.a.r.cal invoke; set choice 1}; bind .a <KeyPress-C> {.a.r.cal invoke; set choice 1}" );
cmd( "bind .a <KeyPress-u> {.a.r.sum invoke; set choice 1}; bind .a <KeyPress-U> {.a.r.sum invoke; set choice 1}" );
cmd( "bind .a <KeyPress-s> {.a.r.scnd invoke; set choice 1}; bind .a <KeyPress-S> {.a.r.scnd invoke; set choice 1}" );
cmd( "bind .a <KeyPress-r> {.a.r.sear invoke; set choice 1}; bind .a <KeyPress-R> {.a.r.sear invoke; set choice 1}" );
cmd( "bind .a <KeyPress-w> {.a.r.wri invoke; set choice 1}; bind .a <KeyPress-W> {.a.r.wri invoke; set choice 1}" );
cmd( "bind .a <KeyPress-f> {.a.r.for invoke; set choice 1}; bind .a <KeyPress-F> {.a.r.for invoke; set choice 1}" );
cmd( "bind .a <KeyPress-t> {.a.r.lqs invoke; set choice 1}; bind .a <KeyPress-T> {.a.r.lqs invoke; set choice 1}" );
cmd( "bind .a <KeyPress-a> {.a.r.addo invoke; set choice 1}; bind .a <KeyPress-A> {.a.r.addo invoke; set choice 1}" );
cmd( "bind .a <KeyPress-d> {.a.r.delo invoke; set choice 1}; bind .a <KeyPress-D> {.a.r.delo invoke; set choice 1}" );
cmd( "bind .a <KeyPress-i> {.a.r.incr invoke; set choice 1}; bind .a <KeyPress-I> {.a.r.incr invoke; set choice 1}" );
cmd( "bind .a <KeyPress-m> {.a.r.mult invoke; set choice 1}; bind .a <KeyPress-M> {.a.r.mult invoke; set choice 1}" );
cmd( "bind .a <KeyPress-h> {.a.r.math invoke; set choice 1}; bind .a <KeyPress-H> {.a.r.math invoke; set choice 1}" );
cmd( "bind .a <KeyPress-n> {.a.r.rndo invoke; set choice 1}; bind .a <KeyPress-N> {.a.r.rndo invoke; set choice 1}" );

cmd( "frame .a.f" );
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp LMM_help.html#LsdScript}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );

cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.r.equ .a.r.cal .a.r.sum .a.r.sear .a.r.scnd .a.r.lqs .a.r.addo .a.r.delo .a.r.rndo .a.r.wri .a.r.incr .a.r.mult .a.r.for .a.r.math -anchor w" );
cmd( "pack .a.tit .a.r" );
cmd( "pack .a.f -side right" );
cmd( "bind .a <Return> {.a.f.ok invoke}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );

cmd( "showtop .a" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 { 
  choice=0;

  goto loop;
 }


cmd( "set choice $res" );
goto loop;

}



if ( choice == 40 && macro )
{
/*
Insert a v[0]=p->increment("Var",0);
*/
choice=0;
cmd( "newtop .a \"Insert an 'INCR' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result after the increment\"" );

cmd( "set v_num %d", v_counter );

cmd( "entry .a.v_num -width 2 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the variable to increase\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.val; .a.val selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the value to add\"" );
cmd( "set v_val 1" );
cmd( "entry .a.val -width 15 -textvariable v_val -justify center" );
cmd( "bind .a.val <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object containing the variable to increment\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#INCR}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {$v_num!=\"\" && [string is integer $v_num]} {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}" );
cmd( "if {$v_obj!=\"p\"} {.f.t.t insert insert \"INCRS($v_obj,\\\"$v_label\\\",$v_val)\"} {.f.t.t insert insert \"INCR(\\\"$v_label\\\",$v_val)\"}" );
cmd( ".f.t.t insert insert \";\\n\"" );

cmd( "if {$v_num==\"\" } {set num -1} {set num $v_num}" );
if(num!=-1)
 v_counter=++num;

cmd( "unset -nocomplain v_num" );
cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 40 && ! macro )
{
/*
Insert a v[0]=p->increment("Var",0);
*/
choice=0;
cmd( "newtop .a \"Insert an 'increment' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result after the increment\"" );

cmd( "set v_num %d", v_counter );

cmd( "entry .a.v_num -width 2 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the variable to increase\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.val; .a.val selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the value to add\"" );
cmd( "set v_val 0" );
cmd( "entry .a.val -width 15 -textvariable v_val -justify center" );
cmd( "bind .a.val <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object containing the variable to increment\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#increment}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {$v_num==\"\" && [string is integer $v_num]} {.f.t.t insert insert \"$v_obj->increment(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->increment(\\\"$v_label\\\",$v_val);\";incr v_num}" );

cmd( "if {$v_num==\"\" } {set num -1} {set num $v_num}" );
if(num!=-1)
 v_counter=++num;

cmd( "unset v_num" );
cmd( "savCurIni" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 45 && macro )
{
/*
Insert a v[0]=p->multiply("Var",0);
*/
choice=0;
cmd( "newtop .a \"Insert a 'MULT' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result after the multiplication\"" );

cmd( "set v_num %d", v_counter );

cmd( "entry .a.v_num -width 2 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the variable to multiply\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.val; .a.val selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the value to multiply\"" );
cmd( "set v_val 0" );
cmd( "entry .a.val -width 15 -textvariable v_val -justify center" );
cmd( "bind .a.val <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object containing the variable to multiply\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );

	
cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#MULT}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );

cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

cmd( "if {$v_num!=\"\" && [string is integer $v_num]} {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}" );
cmd( "if {$v_obj!=\"p\"} {.f.t.t insert insert \"MULTS($v_obj,\\\"$v_label\\\",$v_val)\"} {.f.t.t insert insert \"MULT(\\\"$v_label\\\",$v_val)\"}" );
cmd( ".f.t.t insert insert \";\\n\"" );

cmd( "if {$v_num==\"\" } {set num -1} {set num $v_num}" );
if(num!=-1)
 v_counter=++num;

cmd( "unset v_num" );
cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 45 && ! macro )
{
/*
Insert a v[0]=p->multiply("Var",0);
*/
choice=0;
cmd( "newtop .a \"Insert a 'multiply' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result after the multiplication\"" );

cmd( "set v_num %d", v_counter );

cmd( "entry .a.v_num -width 2 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the variable to multiply\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.val; .a.val selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the value to multiply\"" );
cmd( "set v_val 0" );
cmd( "entry .a.val -width 15 -textvariable v_val -justify center" );
cmd( "bind .a.val <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object containing the variable to multiply\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );

	
cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#multiply}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {$v_num==\"\" && [string is integer $v_num]} {.f.t.t insert insert \"$v_obj->multiply(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->multiply(\\\"$v_label\\\",$v_val);\";incr v_num}" );

cmd( "if {$v_num==\"\" } {set num -1} {set num $v_num}" );
if(num!=-1)
 v_counter=++num;

cmd( "unset v_num" );
cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 29 && macro )
{
/*
Insert a p->write("Var",0,0);
*/
choice=0;
cmd( "newtop .a \"Insert a 'WRITE' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the value to write\"" );

cmd( "set v_num 0" );

cmd( "entry .a.v_num -width 15 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Label of the var. or par. to overwrite\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Time step appearing as latest computation for the new value\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Object containing the var. or par. to write\"" );
cmd( "if { [catch {puts $v_obj}] == 1 } {set v_obj p} {}" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#WRITE}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {$v_obj ==\"p\" && $v_lag == 0} { .f.t.t insert insert \"WRITE(\\\"$v_label\\\",$v_num);\"} {}" );
cmd( "if {$v_obj ==\"p\" && [string is integer $v_lag] && $v_lag != 0} { .f.t.t insert insert \"WRITEL(\\\"$v_label\\\",$v_num, $v_lag);\"} {}" );
cmd( "if {$v_obj !=\"p\" && $v_lag == 0} { .f.t.t insert insert \"WRITES($v_obj,\\\"$v_label\\\",$v_num);\"} {}" );
cmd( "if {$v_obj !=\"p\" && [string is integer $v_lag] && $v_lag != 0} { .f.t.t insert insert \"WRITELS($v_obj,\\\"$v_label\\\",$v_num, $v_lag);\"} {}" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 29 && ! macro )
{
/*
Insert a p->write("Var",0,0);
*/
choice=0;
cmd( "newtop .a \"Insert a 'write' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the value to write\"" );

cmd( "set v_num 0" );

cmd( "entry .a.v_num -width 15 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the var. or par. to write\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the time step appearing as latest computation for the new value\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object containing the var. or par. to write\"" );
cmd( "if { [catch {puts $v_obj}] == 1 } {set v_obj p} {}" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#write}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {[string is integer $v_lag]} {.f.t.t insert insert \"$v_obj->write(\\\"$v_label\\\",$v_num,$v_lag);\"}" );
cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 30 && ! macro )
{
/*
Insert a cur=p->search_var_cond("Var",1,0);
*/
choice=0;
cmd( "newtop .a \"Insert a 'search_var_cond' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the target pointer in which to return the object found\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.v_num; .a.v_num selection range 0 end}" );

cmd( "label .a.l1 -text \"Type below the value to search for\"" );
cmd( "set v_num 0" );
cmd( "entry .a.v_num -width 15 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the var. or par. to search for\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the lag to be used\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object from which to search\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#search_var_cond}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {[string is integer $v_lag]} {.f.t.t insert insert \"$v_obj0=$v_obj->search_var_cond(\\\"$v_label\\\",$v_num,$v_lag);\"}" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 30 && macro )
{
/*
Insert a cur=p->search_var_cond("Var",1,0);
*/
choice=0;
cmd( "newtop .a \"Insert a 'SEARCH_CND' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the target pointer in which to return the object found\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.v_num; .a.v_num selection range 0 end}" );

cmd( "label .a.l1 -text \"Type below the value to search for\"" );
cmd( "set v_num 0" );
cmd( "entry .a.v_num -width 15 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the var. or par. to search for\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the lag to be used\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object from which to search\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#SEARCH_CND}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {$v_obj ==\"p\" && $v_lag == 0} { .f.t.t insert insert \"$v_obj0=SEARCH_CND(\\\"$v_label\\\",$v_num);\"} {}" );
cmd( "if {$v_obj ==\"p\" && [string is integer $v_lag] && $v_lag != 0} { .f.t.t insert insert \"$v_obj0=SEARCH_CNDL(\\\"$v_label\\\",$v_num, $v_lag);\"} {}" );
cmd( "if {$v_obj !=\"p\" && $v_lag == 0} { .f.t.t insert insert \"$v_obj0=SEARCH_CNDS($v_obj,\\\"$v_label\\\",$v_num);\"} {}" );
cmd( "if {$v_obj !=\"p\" && [string is integer $v_lag] && $v_lag != 0} { .f.t.t insert insert \"$v_obj0=SEARCH_CNDLS($v_obj,\\\"$v_label\\\",$v_num, $v_lag);\"} {}" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 31 && macro )
{
/*
Insert a p->lsdqsort("Object", "Variable", "DIRECTION");
*/
choice=0;
cmd( "newtop .a \"Insert a 'SORT' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the object containing the objects to be sorted\"" );
cmd( "set v_obj1 p" );
cmd( "entry .a.obj1 -width 6 -textvariable v_obj1 -justify center" );
cmd( "bind .a.obj1 <Return> {focus .a.obj0; .a.obj0 selection range 0 end}" );

cmd( "label .a.l0 -text \"Type below label of the objects to be sorted\"" );
cmd( "set v_obj0 ObjectName" );
cmd( "entry .a.obj0 -width 30 -textvariable v_obj0" );
cmd( "bind .a.obj0 <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the var. or par. whose values are to be sorted\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.r_up}" );

cmd( "label .a.l3 -text \"Set the sorting direction\"" );
cmd( "set v_direction 1" );
cmd( "radiobutton .a.r_up -text Increasing -variable v_direction -value 1" );
cmd( "radiobutton .a.r_down -text Decreasing -variable v_direction -value 2" );

cmd( "bind .a.l3 <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#SORT}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.obj1 .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.r_up .a.r_down" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj1" );
cmd( ".a.obj1 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }

cmd( "set choice $v_direction" );
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

if ( choice == 1)
  cmd( "set direction \"UP\"" );
else
  cmd( "set direction \"DOWN\"" );

cmd( "if {$v_obj1 ==\"p\"} {.f.t.t insert insert \"SORT(\\\"$v_obj0\\\",\\\"$v_label\\\",\\\"$direction\\\");\"} {}" );
cmd( "if {$v_obj1 !=\"p\"} {.f.t.t insert insert \"SORTS($v_obj1,\\\"$v_obj0\\\",\\\"$v_label\\\",\\\"$direction\\\");\"} {}" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 31 && ! macro )
{
/*
Insert a p->lsdqsort("Object", "Variable", "DIRECTION");
*/
choice=0;
cmd( "newtop .a \"Insert a 'lsdqsort' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the object containing the objects to be sorted\"" );
cmd( "set v_obj1 p" );
cmd( "entry .a.obj1 -width 6 -textvariable v_obj1 -justify center" );
cmd( "bind .a.obj1 <Return> {focus .a.obj0; .a.obj0 selection range 0 end}" );

cmd( "label .a.l0 -text \"Type below label of the objects to be sorted\"" );
cmd( "set v_obj0 ObjectName" );
cmd( "entry .a.obj0 -width 30 -textvariable v_obj0" );
cmd( "bind .a.obj0 <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the var. or par. whose values are to be sorted\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.r_up}" );

cmd( "label .a.l3 -text \"Set the sorting direction\"" );
cmd( "set v_direction 1" );
cmd( "radiobutton .a.r_up -text Increasing -variable v_direction -value 1" );
cmd( "radiobutton .a.r_down -text Decreasing -variable v_direction -value 2" );

cmd( "bind .a.l3 <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#lsdqsort}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.obj1 .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.r_up .a.r_down" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj1" );
cmd( ".a.obj1 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }

cmd( "set choice $v_direction" );
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
if(choice==1)
  cmd( ".f.t.t insert insert \"$v_obj1->lsdqsort(\\\"$v_obj0\\\",\\\"$v_label\\\",\\\"UP\\\");\"" );
else
  cmd( ".f.t.t insert insert \"$v_obj1->lsdqsort(\\\"$v_obj0\\\",\\\"$v_label\\\",\\\"DOWN\\\");\"" );


cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==51)
{
/*
Insert a math function
*/
cmd( "newtop .a \"Insert a Math Operation\" { .a.f.esc invoke }" );

cmd( "set value1 \"0\"; set value2 \"1\"; set res 1; set str \"UNIFORM($value1,$value2)\"" );
cmd( "label .a.l1 -text \"Minimum\"" );
cmd( "entry .a.e1 -justify center -textvariable value1" );
cmd( "label .a.l2 -text \"Maximum\"" );
cmd( "entry .a.e2 -justify center -textvariable value2" );
cmd( "pack .a.l1 .a.e1 .a.l2 .a.e2" );

cmd( "radiobutton .a.r1 -text \"Uniform random draw\" -variable res -value 1 -command {.a.l1 conf -text Minimum; .a.l2 conf -text Maximum -state normal; set str \"UNIFORM($value1,$value2)\"}" );
cmd( "radiobutton .a.r2 -text \"Normal random draw\" -variable res -value 2 -command {.a.l1 conf -text Mean; .a.l2 conf -text Variance -state normal; set str \"norm($value1,$value2)\"}" );
cmd( "radiobutton .a.r3 -text \"Integer uniform random draw\" -variable res -value 3 -command {.a.l1 conf -text Minimum; .a.l2 conf -text Maximum -state normal; set str \"rnd_integer($value1,$value2)\"}" );
cmd( "radiobutton .a.r4 -text \"Poisson random draw\" -variable res -value 4 -command {.a.l1 conf -text Mean; .a.l2 conf -text (unused) -state disabled; set str \"poisson($value1)\"}" );
cmd( "radiobutton .a.r5 -text \"Gamma random draw\" -variable res -value 5 -command {.a.l1 conf -text Mean; .a.l2 conf -text (unused) -state disabled; set str \"gamma($value1)\"}" );
cmd( "radiobutton .a.r6 -text \"Absolute value\" -variable res -value 6 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused) -state disabled; set str \"abs($value1)\"}" );
cmd( "radiobutton .a.r7 -text \"Minimum value\" -variable res -value 7 -command {.a.l1 conf -text \"Value 1\"; .a.l2 conf -text \"Value 2\" -state normal; set str \"min($value1,$value2)\"}" );
cmd( "radiobutton .a.r8 -text \"Maximum value\" -variable res -value 8 -command {.a.l1 conf -text \"Value 1\"; .a.l2 conf -text \"Value 2\" -state normal; set str \"max($value1,$value2)\"}" );
cmd( "radiobutton .a.r9 -text \"Round closest integer\" -variable res -value 9 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused) -state disabled; set str \"round($value1)\"}" );
cmd( "radiobutton .a.r10 -text \"Exponential\" -variable res -value 10 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused) -state disabled; set str \"exp($value1)\"}" );
cmd( "radiobutton .a.r11 -text \"Logarithm\" -variable res -value 11 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused) -state disabled; set str \"log($value1)\"}" );
cmd( "radiobutton .a.r12 -text \"Square root\" -variable res -value 12 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused) -state disabled; set str \"sqrt($value1)\"}" );
cmd( "radiobutton .a.r13 -text \"Power\" -variable res -value 13 -command {.a.l1 conf -text \"Base\"; .a.l2 conf -text \"Exponent\" -state normal; set str \"pow($value1,$value2)\"}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#rnd}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left" );
cmd( "pack .a.r1 .a.r2 .a.r3 .a.r4 .a.r5 .a.r6 .a.r7 .a.r8 .a.r9 .a.r10 .a.r11 .a.r12 .a.r13 -anchor w" );
cmd( "pack .a.f -side right" );
choice=0;

cmd( "showtop .a" );
cmd( "focus .a.e1; .a.e1 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd( "set choice $res" );
switch(choice)
{

case 1: cmd( "set str \"UNIFORM($value1,$value2)\"" );
break;
case 2: cmd( "set str \"norm($value1,$value2)\"" );
break;
case 3: cmd( "set str \"rnd_integer($value1,$value2)\"" );
break;
case 4: cmd( "set str \"poisson($value1)\"" );
break;
case 5: cmd( "set str \"gamma($value1)\"" );
break;
case 6: cmd( "set str \"abs($value1)\"" );
break;
case 7: cmd( "set str \"min($value1,$value2)\"" );
break;
case 8: cmd( "set str \"max($value1,$value2)\"" );
break;
case 9: cmd( "set str \"round($value1)\"" );
break;
case 10: cmd( "set str \"exp($value1)\"" );
break;
case 11: cmd( "set str \"log($value1)\"" );
break;
case 12: cmd( "set str \"sqrt($value1)\"" );
break;
case 13: cmd( "set str \"pow($value1,$value2)\"" );
break;

default: break;
}

cmd( "savCurIni" );	// save data for recolor
cmd( ".f.t.t insert insert \"$str\"" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 52 && macro )
{
/*
Insert a add_an_obj;
*/
here_addobj:

cmd( "newtop .a \"Insert an 'ADDOBJ' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the target pointer in which to return the new object created\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the object to create\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.numobj; .a.numobj selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the number of objects to create\"" );
cmd( "set numobj \"1\"" );
cmd( "entry .a.numobj -width 6 -textvariable numobj -justify center" );
cmd( "bind .a.numobj <Return> {focus .a.v_num; .a.v_num selection range 0 end}" );

cmd( "label .a.l1 -text \"Type below the pointer to an example object to copy its initialization, if any.\"" );
cmd( "set v_num \"cur\"" );
cmd( "entry .a.v_num -width 6 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the parent object containing the new object(s)\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#ADDOBJ}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.numobj .a.l1 .a.v_num .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );
 
choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );
 
 
if(choice==2)
 {
  
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if { $numobj == \"1\"} {set choice 1} {set choice 0}" );
cmd( "if {$v_obj0 != \"\"} {.f.t.t insert insert $v_obj0; .f.t.t insert insert =} {}" );

if(choice ==1)
{
//cmd( ".f.t.t insert insert \"$v_obj0 \"" );
cmd( "if {$v_obj ==\"p\" && $v_num==\"\" } { .f.t.t insert insert \"ADDOBJ(\\\"$v_label\\\");\"} {}" );
cmd( "if {$v_obj ==\"p\" && $v_num!=\"\" && [string is integer $v_num]} { .f.t.t insert insert \"ADDOBJ_EX(\\\"$v_label\\\",$v_num);\"} {}" );
cmd( "if {$v_obj !=\"p\" && $v_num == \"\" } { .f.t.t insert insert \"ADDOBJS($v_obj,\\\"$v_label\\\");\"} {}" );
cmd( "if {$v_obj !=\"p\" && $v_num != \"\" && [string is integer $v_num]} { .f.t.t insert insert \"ADDOBJS_EX($v_obj,\\\"$v_label\\\",$v_num);\"} {}" );

}
else
{

cmd( "if {$v_obj ==\"p\" && $v_num!=\"\" && [string is integer $v_num] && [string is integer $numobj]} { .f.t.t insert insert \"ADDNOBJ_EX(\\\"$v_label\\\",$numobj, $v_num);\"; set choice -3} {}" );
cmd( "if {$v_obj !=\"p\" && $v_num!= \"\" && [string is integer $v_num] && [string is integer $numobj]} { .f.t.t insert insert \"ADDNOBJS_EX($v_obj,\\\"$v_label\\\",$numobj,$v_num);\"; set choice -3} {}" );
cmd( "if {$v_obj ==\"p\" && $v_num==\"\" && [string is integer $numobj]} { .f.t.t insert insert \"ADDNOBJ(\\\"$v_label\\\",$numobj);\"; set choice -3} {}" );
cmd( "if {$v_obj !=\"p\" && $v_num== \"\" && [string is integer $numobj]} { .f.t.t insert insert \"ADDNOBJS($v_obj,\\\"$v_label\\\",$numobj);\"; set choice -3} {}" );

}


cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 52 && ! macro )
{
/*
Insert a add_an_obj;
*/
choice=0;
cmd( "newtop .a \"Insert an 'add_n_objects2' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the target pointer in which to return the new object created\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.v_num; .a.v_num selection range 0 end}" );

cmd( "label .a.l1 -text \"Type below the pointer to an example object, if available\"" );
cmd( "set v_num \"\"" );
cmd( "entry .a.v_num -width 6 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the object to create\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.obj; .a.obj selection range 0 end}" );


cmd( "label .a.l3 -text \"Type below the number of objects to create\"" );
cmd( "set numobj \"1\"" );
cmd( "entry .a.numobj -width 6 -textvariable numobj -justify center" );
cmd( "bind .a.numobj <Return> {focus .a.v_num; .a.v_num selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the parent object where to add the new object\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#add_an_object}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.numobj .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

cmd( "if {$v_num==\"\" && [string is integer $numobj]} { .f.t.t insert insert \"$v_obj0=$v_obj->add_n_objects2(\\\"$v_label\\\",$numobj);\\n\"}" );
cmd( "if {$v_num!=\"\" && [string is integer $v_num] && [string is integer $numobj]} {.f.t.t insert insert \"$v_obj0=$v_obj->add_n_objects2(\\\"$v_label\\\",$numobj,$v_num);\\n\"}" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 53 && macro )
{
/*
Insert a delete_obj;
*/
choice=0;
cmd( "newtop .a \"Insert a 'DELETE' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the pointer of the object to delete\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#DELETE}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left" );
cmd( "pack .a.l0 .a.obj0" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );


cmd( ".f.t.t insert insert \"DELETE($v_obj0);\\n\"" );


cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 53 && ! macro )
{
/*
Insert a delete_obj;
*/
choice=0;
cmd( "newtop .a \"Insert a 'delete_obj' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the pointer of the object to delete\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#delete_obj}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left" );
cmd( "pack .a.l0 .a.obj0" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );


cmd( ".f.t.t insert insert \"$v_obj0->delete_obj();\\n\"" );


cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 54 && macro )
{
/*
Insert a RNDDRAW
*/
choice=0;
cmd( "newtop .a \"Insert a 'RNDDRAW' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the target pointer in which to return the object drawn\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.v_num; .a.v_num selection range 0 end}" );

cmd( "label .a.l1 -text \"Type below the label of the objects to draw\"" );
cmd( "set v_num object" );
cmd( "entry .a.v_num -width 6 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the var. or par. to be used as proxies for probabilities\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the lag to be used\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.tot; .a.tot selection range 0 end}" );

cmd( "label .a.l31 -text \"Type below the sum over all values of the var. or par., if available\"" );
cmd( "set v_tot \"\"" );
cmd( "entry .a.tot -width 15 -textvariable v_tot -justify center" );
cmd( "bind .a.tot <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object from which to search\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#RNDDRAW}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l31 .a.tot .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {$v_tot == \"\"} {set choice 1} {set choice 2}" );

if(choice==1)
 {
  cmd( "if {$v_obj ==\"p\" && $v_lag == 0 && $v_label != \"\"} { .f.t.t insert insert \"$v_obj0=RNDDRAW(\\\"$v_num\\\",\\\"$v_label\\\");\\n\"} {}" );
  cmd( "if {$v_obj ==\"p\" && $v_lag == 0 && $v_label == \"\"} { .f.t.t insert insert \"$v_obj0=RNDDRAWFAIR(\\\"$v_num\\\");\\n\"} {}" );  
  cmd( "if {$v_obj ==\"p\" && $v_lag != 0 && [string is integer $v_lag]} { .f.t.t insert insert \"$v_obj0=RNDDRAWL(\\\"$v_num\\\",\\\"$v_label\\\", $v_lag);\\n\"} {}" );
  cmd( "if {$v_obj !=\"p\" && $v_lag == 0 && $v_label != \"\" } { .f.t.t insert insert \"$v_obj0=RNDDRAWS($v_obj,\\\"$v_num\\\",\\\"$v_label\\\");\\n\"} {}" );
  cmd( "if {$v_obj !=\"p\" && $v_lag == 0 && $v_label == \"\" } { .f.t.t insert insert \"$v_obj0=RNDDRAWFAIRS($v_obj,\\\"$v_num\\\");\\n\"} {}" );  
  cmd( "if {$v_obj !=\"p\" && $v_lag != 0 && [string is integer $v_lag]} { .f.t.t insert insert \"$v_obj0=RNDDRAWLS($v_obj,\\\"$v_num\\\",\\\"$v_label\\\", $v_lag);\\n\"} {}" );
  
 }
else
 {
  cmd( "if {$v_obj ==\"p\" && $v_lag == 0} { .f.t.t insert insert \"$v_obj0=RNDDRAWTOT(\\\"$v_num\\\",\\\"$v_label\\\", $v_tot);\\n\"} {}" );
  cmd( "if {$v_obj ==\"p\" && $v_lag != 0 && [string is integer $v_lag]} { .f.t.t insert insert \"$v_obj0=RNDDRAWTOTL(\\\"$v_num\\\",\\\"$v_label\\\", $v_lag, $v_tot);\\n\"} {}" );
  cmd( "if {$v_obj !=\"p\" && $v_lag == 0} { .f.t.t insert insert \"$v_obj0=RNDDRAWTOTS($v_obj,\\\"$v_num\\\",\\\"$v_label\\\", $v_tot);\\n\"} {}" );
  cmd( "if {$v_obj !=\"p\" && $v_lag != 0 && [string is integer $v_lag]} { .f.t.t insert insert \"$v_obj0=RNDDRAWTOTLS($v_obj,\\\"$v_num\\\",\\\"$v_label\\\", $v_lag, $v_tot);\\n\"} {}" );
 }

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 54 && ! macro )
{
/*
Insert a RNDDRAW
*/
choice=0;
cmd( "newtop .a \"Insert a 'draw_rnd' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the target pointer in which to return the object drawn\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.v_num; .a.v_num selection range 0 end}" );

cmd( "label .a.l1 -text \"Type below the label of the objects to draw\"" );
cmd( "set v_num Object" );
cmd( "entry .a.v_num -width 6 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the var. or par. to be used as proxies for probabilities\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the lag to be used\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.tot; .a.tot selection range 0 end}" );

cmd( "label .a.l31 -text \"Type below the sum over all values of the var. or par., if available\"" );
cmd( "set v_tot \"\"" );
cmd( "entry .a.tot -width 15 -textvariable v_tot -justify center" );
cmd( "bind .a.tot <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object from which to search\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#draw_rnd}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l31 .a.tot .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );
cmd( "if {$v_tot == \"\"} {set choice 1} {set choice 2}" );

if(choice==1)
  cmd( "if {[string is integer $v_lag]} {.f.t.t insert insert \"$v_obj0=$v_obj->draw_rnd(\\\"$v_num\\\",\\\"$v_label\\\",$v_lag);\\n\"}" );
else
  cmd( "if {[string is integer $v_lag]} {.f.t.t insert insert \"$v_obj0=$v_obj->draw_rnd(\\\"$v_num\\\",\\\"$v_label\\\",$v_lag,$v_tot);\\n\"}" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 55 && macro )
{
/*
Insert a SEARCH;
*/
choice=0;
cmd( "newtop .a \"Insert a 'SEARCH' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the target pointer where to return the found object\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.lab; .a.lab selection range 0 end}" );

cmd( "label .a.l1 -text \"Type below the label of the object to search\"" );
cmd( "set v_lab Object" );
cmd( "entry .a.lab -width 20 -textvariable v_lab" );
cmd( "bind .a.lab <Return> {focus .a.obj1; .a.obj1 selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the pointer of the parent object where to start the search\"" );
cmd( "set v_obj1 p" );
cmd( "entry .a.obj1 -width 6 -textvariable v_obj1 -justify center" );
cmd( "bind .a.obj1 <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#SEARCH}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l0 .a.obj0 .a.l1 .a.lab .a.l2 .a.obj1" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

cmd( "if { $v_obj1 == \"p\" } {.f.t.t insert insert \"$v_obj0=SEARCH(\\\"$v_lab\\\");\\n\"} {.f.t.t insert insert \"$v_obj0=SEARCHS($v_obj1,\\\"$v_lab\\\");\\n\"}" );


cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 55 && ! macro )
{
/*
Insert a SEARCH;
*/
choice=0;
cmd( "newtop .a \"Insert a 'search' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l0 -text \"Type below the target pointer where to return the found object\"" );
cmd( "set v_obj0 cur" );
cmd( "entry .a.obj0 -width 6 -textvariable v_obj0 -justify center" );
cmd( "bind .a.obj0 <Return> {focus .a.lab; .a.lab selection range 0 end}" );

cmd( "label .a.l1 -text \"Type below the label of the object to search\"" );
cmd( "set v_lab Object" );
cmd( "entry .a.lab -width 20 -textvariable v_lab" );
cmd( "bind .a.lab <Return> {focus .a.obj1; .a.obj1 selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the pointer of the parent object where to start the search\"" );
cmd( "set v_obj1 p" );
cmd( "entry .a.obj1 -width 6 -textvariable v_obj1 -justify center" );
cmd( "bind .a.obj1 <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#search}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l0 .a.obj0 .a.l1 .a.lab .a.l2 .a.obj1" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.obj0" );
cmd( ".a.obj0 selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

cmd( ".f.t.t insert insert \"$v_obj0=$v_obj1->search(\\\"$v_lab\\\");\\n\"" );

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if ( choice == 56 && macro )
{
/*
Insert a SUM;
*/
choice=0;
cmd( "newtop .a \"Insert a 'SUM' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result\"" );

cmd( "set v_num %d", v_counter );
cmd( "entry .a.v_num -width 2 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the variable to sum\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the lag to be used\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object to which request the computation\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfuncMacro.html#SUM}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

//cmd( "if {$v_num==\"\"} {.f.t.t insert insert \"$v_obj->cal(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->cal(\\\"$v_label\\\",$v_lag);\"; incr v_num}" );
cmd( "if {$v_num!=\"\" && [string is integer $v_num]} {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}" );

cmd( "if {$v_lag==0 && $v_obj==\"p\"} { .f.t.t insert insert \"SUM(\\\"$v_label\\\")\"} {}" );
cmd( "if {$v_lag!=0 && [string is integer $v_lag] && $v_obj==\"p\"} { .f.t.t insert insert \"SUML(\\\"$v_label\\\",$v_lag)\"} {}" );
cmd( "if {$v_lag==0 && $v_obj!=\"p\"} { .f.t.t insert insert \"SUMS($v_obj,\\\"$v_label\\\")\"} {}" );
cmd( "if {$v_lag!=0 && [string is integer $v_lag] && $v_obj!=\"p\"} { .f.t.t insert insert \"SUMLS($v_obj,\\\"$v_label\\\",$v_lag)\"} {}" );

cmd( "if {$v_num!=\"\"} {.f.t.t insert insert \";\\n\"} {}" );

cmd( "if {$v_num==\"\"} { set num -1} {set num $v_num}" );
if(num!=-1)
 v_counter=++num;

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if ( choice == 56 && ! macro )
{
/*
Insert a p->sum;
*/
choice=0;
cmd( "newtop .a \"Insert a 'sum' Command\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result\"" );

cmd( "set v_num %d", v_counter );
cmd( "entry .a.v_num -width 2 -textvariable v_num -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.label; .a.label selection range 0 end}" );

cmd( "label .a.l2 -text \"Type below the label of the variable to sum\"" );
cmd( "set v_label Label" );
cmd( "entry .a.label -width 30 -textvariable v_label" );
cmd( "bind .a.label <Return> {focus .a.lag; .a.lag selection range 0 end}" );

cmd( "label .a.l3 -text \"Type below the lag to be used\"" );
cmd( "set v_lag 0" );
cmd( "entry .a.lag -width 2 -textvariable v_lag -justify center" );
cmd( "bind .a.lag <Return> {focus .a.obj; .a.obj selection range 0 end}" );

cmd( "label .a.l4 -text \"Type below the object to which request the computation\"" );
cmd( "set v_obj p" );
cmd( "entry .a.obj -width 6 -textvariable v_obj -justify center" );
cmd( "bind .a.obj <Return> {focus .a.f.ok}" );

cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp lsdfunc.html#sum}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "savCurIni" );	// save data for recolor
cmd( "set a [.f.t.t index insert]" );

cmd( "if {$v_num!=\"\" && [string is integer $v_num] && [string is integer $v_lag]} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->sum(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"$v_obj->sum(\\\"$v_label\\\",$v_lag);\\n\"}" );

cmd( "if {$v_num!=\"\"} {.f.t.t insert insert \";\\n\"} {}" );

cmd( "if {$v_num==\"\"} { set num -1} {set num $v_num}" );
if(num!=-1)
 v_counter=++num;

cmd( "savCurFin; updCurWnd" );	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==32)
{
/* find matching parenthesis */

cmd( "set sym [.f.t.t get insert]" );
cmd( "if {[string compare $sym \"\\(\"]!=0} { \
        if {[string compare $sym \"\\)\"]!=0} \
            {set choice 0} \
            {set num -1; set direction -backwards; set fsign +; set sign \"\"; set terminal \"1.0\"}\
      } \
      {set num 1; set direction -forwards; set fsign -; set sign +; set terminal end}" );

if(choice==0)
 goto loop;
cmd( "set cur [.f.t.t index insert]" );
cmd( ".f.t.t tag add sel $cur \"$cur + 1char\"" );
if (num>0)
  cmd( "set cur [.f.t.t index \"insert + 1 char\"]" );
while(num!=0 && choice!=0)
{
 cmd( "set a [.f.t.t search $direction \"\\(\" $cur $terminal]" );
 cmd( "if {$a==\"\"} {set a [.f.t.t index $terminal]} {}" );
 cmd( "set b [.f.t.t search $direction \"\\)\" $cur $terminal]" );
// cmd( ".f.t.t insert end \"$a $b $cur\\n\"" );
 cmd( "if {$b==\"\"} {set b [.f.t.t index $terminal]} {}" );
 cmd( "if {$a==$b} {set choice 0} {}" );
 if(choice==0)
  goto loop;
 if(num>0)
   cmd( "if {[.f.t.t compare $a < $b]} {set num [expr $num + 1]; set cur [.f.t.t index \"$a+1char\"]} {set num [expr $num - 1]; set cur [.f.t.t index \"$b+1char\"]}" );
 else
   cmd( "if {[.f.t.t compare $a > $b]} {set num [expr $num + 1]; set cur [.f.t.t index $a]} {set num [expr $num - 1]; set cur [.f.t.t index $b]}" );


}
choice=0;


cmd( " if { [string compare $sign \"+\"]==0 } {.f.t.t tag add sel \"$cur - 1char\" $cur ; set num 1} {.f.t.t tag add sel $cur \"$cur + 1char\"; set num 0}" );

cmd( ".f.t.t see $cur" );
goto loop;
}


if(choice==33)
{
/* Select a model*/

choice=0;
Tcl_LinkVar(inter, "choiceSM", (char *) &num, TCL_LINK_INT);
num=0;
cmd( "showmodel $groupdir" );
cmd( "focus .l" );

while(choice==0 && num==0)
 Tcl_DoOneEvent(0);

cmd( "if {[winfo exists .l]==1} {destroytop .l; bind .f.t.t <Enter> {}} {}" );
choice=num;

Tcl_UnlinkVar(inter, "choiceSM");
if(choice==2 || choice==0)
 {choice=0;
  goto loop;
 }
cmd( "set groupdir [lindex $lrn 0]" ); //the group dir is the same for every element
if(choice == 14)
 { //create a new model/group
 goto loop;
 }

cmd( "set modelname [lindex $lmn $result]" );
cmd( "set version [lindex $lver $result]" );
cmd( "set modeldir [lindex $ldn $result]" );
cmd( "set dirname $modeldir" );

cmd( ".f.hea.grp.dat conf -text \"$modelgroup\"" );
cmd( ".f.hea.mod.dat conf -text \"$modelname\"" );
cmd( ".f.hea.ver.dat conf -text \"$version\"" );

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
cmd( ".m.model entryconf 11 -state normal" );

choice=50;		// load description file
goto loop;
}


if(choice==41)
{
/************
 create a new version of the current model
*************/
choice=0;


cmd( "newtop .a \"Save Model As...\" { .a.b.esc invoke }" );

cmd( "label .a.tit -text \"Create a new version of model '$modelname' (ver. $version)\"" );

cmd( "label .a.mname -text \"Insert new model name\"" );
cmd( "set mname $modelname" );
cmd( "entry .a.ename -width 30 -textvariable mname" );
cmd( "bind .a.ename <Return> {.a.ever selection range 0 end; focus .a.ever}" ); 

cmd( "label .a.mver -text \"Insert new version number\"" );
cmd( "set mver $version" );
cmd( "entry .a.ever -width 30 -textvariable mver" );
cmd( "bind .a.ever <Return> {.a.edir selection range 0 end; focus .a.edir}" ); 

cmd( "label .a.mdir -text \"Insert new directory name\"" );
cmd( "set mdir $dirname" );
cmd( "entry .a.edir -width 30 -textvariable mdir" );
cmd( "bind .a.edir <Return> {focus .a.b.ok}" ); 

cmd( "frame .a.b" );
cmd( "button .a.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .a.b.help -width -9 -text Help -command {LsdHelp LMM_help.html#copy}" );
cmd( "button .a.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.b.esc invoke}" );
cmd( "pack .a.b.ok .a.b.help .a.b.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.mname .a.ename .a.mver .a.ever .a.mdir .a.edir" );
cmd( "pack .a.b -side right" );

cmd( "showtop .a" );
cmd( "focus .a.ename" );
cmd( ".a.ename selection range 0 end" );

loop_copy:

while(choice==0)
 Tcl_DoOneEvent(0);

//operation cancelled
if(choice==2)
 {
  cmd( "destroytop .a" );  
  choice=0;
  goto loop;
 }

//control for existing directory
cmd( "if {[file exists $mdir] == 1} {tk_messageBox -parent .a -type ok -title Error -icon error -message \"Cannot create directory\" -detail \"$mdir.\\n\\nChoose a different name.\"; set choice 3} {}" );
if(choice==3)
 {cmd( ".a.edir selection range 0 end" );
  cmd( "focus .a.edir" );
  choice=0;
  goto loop_copy;
 } 

//control for an existing model with the same name AND same version
cmd( "set dir [glob *]" );
cmd( "set num [llength $dir]" );
strcpy(str, " ");
for(i=0; i<num && choice!=3; i++)
 {
  cmd( "if {[file isdirectory [lindex $dir %d] ] == 1} {set curdir [lindex $dir %i]} {set curdir ___}", i, i );
  s=(char *)Tcl_GetVar(inter, "curdir",0);
  strcpy(str, s);
  if(strcmp(str,"___") && strcmp(str, "gnu") && strcmp(str, "src") && strcmp(str, "Manual") )
   {

    cmd( "set ex [file exists $curdir/modelinfo.txt]" );
    cmd( "if { $ex == 0 } {set choice 0} {set choice 1}" );
    if(choice==1)
    {
      cmd( "set f [open $curdir/modelinfo.txt r]" );
      cmd( "set cname [gets $f]; set cver [gets $f];" );
      cmd( "close $f" );
    }
    else
      cmd( "set cname $curdir; set cver \"0.1\"" );
    cmd( "set comp [string compare $cname $mname]" );
    cmd( "set comp1 [string compare $cver $mver]" );
    cmd( "if {$comp == 0 & $comp1 == 0} {set choice 3; set errdir $curdir} {}" );

   }

 }
if(choice==3)
 {cmd( "tk_messageBox -parent .a -type ok -title Error -icon error -message \"Cannot create new model\" -detail \"The model '$mname' already exists (directory: $errdir).\"" );
  cmd( ".a.ename selection range 0 end" );
  cmd( "focus .a.ename" );
  choice=0;
  goto loop_copy;
 } 

//here we are: create a new copycat model
cmd( "file copy $dirname $mdir" );
cmd( "set dirname $mdir" );
cmd( "set modeldir $mdir" );
cmd( "set modelname $mname" );
cmd( "set version $mver" );

cmd( ".f.hea.mod.dat conf -text \"$modelname\"" );
cmd( ".f.hea.ver.dat conf -text \"$version\"" );
cmd( "destroytop .a" );
cmd( "set ex [file exists $dirname/modelinfo.txt]" );
cmd( "if { $ex == 0 } {set choice 0} {set choice 1}" );
if(choice==1)
  cmd( "file delete $dirname/modelinfo.txt" );
cmd( "set f [open $dirname/modelinfo.txt w]" );
cmd( "puts $f \"$modelname\"" );
cmd( "puts $f \"$version\"" );
cmd( "set frmt \"%d %%B, %%Y\"" );
cmd( "puts $f \"[clock format [clock seconds] -format \"$frmt\"]\"" );
cmd( "close $f" );
cmd( "tk_messageBox -parent . -type ok -title \"Save Model As...\" -icon info -message \"Model '$mname' created\" -detail \"Version: $mver\nDirectory: $dirname\"" );

choice=49;
goto loop;
}

if(choice==42)
 {
 /************
Increase indent
 *************/
choice=0;
cmd( "set in [.f.t.t tag range sel]" );
cmd( "if { [string length $in] == 0 } {set choice 0} {set choice 1}" );
if(choice==0)
 goto loop;
cmd( "scan $in \"%%d.%%d %%d.%%d\" line1 col1 line2 col2" );
cmd( "set num $line1" );
i=num;
cmd( "set num $line2" );

for( ;i<=num; i++)
 {
 cmd( ".f.t.t insert %d.0 \" \"", i );
 }
 

choice=0;
goto loop;

}

if(choice==43)
 {
 /************
Decrease indent
 *************/
choice=0;
cmd( "set in [.f.t.t tag range sel]" );
cmd( "if { [string length $in] == 0 } {set choice 0} {set choice 1}" );
if(choice==0)
 goto loop;
cmd( "scan $in \"%%d.%%d %%d.%%d\" line1 col1 line2 col2" );
cmd( "set num $line1" );
i=num;
cmd( "set num $line2" );

for( ;i<=num; i++)
 {
 cmd( "set c [.f.t.t get %d.0]", i );
 cmd( "if { $c == \" \" } {set choice 1} {set choice 0}" );
 if(choice==1)
  { cmd( ".f.t.t delete %d.0 ", i );
  }  
 }
 
choice=0;
goto loop;

}

if(choice==44)
{
/*
Show end edit model info
*/

if(s==NULL || !strcmp(s, ""))
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
  choice=0;
  goto loop;
 }

cmd( "set ex [file exists $modeldir/modelinfo.txt]" );
cmd( "set choice $ex" );
if(choice==0)
  cmd( "tk_messageBox -parent . -type ok -title Error -icon error -message \"Cannot find file for model info\" -detail \"Please, check the date of creation.\"" );
  
cmd( "newtop .a \"Model Info\" { .a.b.ok invoke }" );

cmd( "frame .a.c" );

cmd( "label .a.c.n -text \"Model Name\"" );
cmd( "entry .a.c.en -width 40 -textvariable modelname" );

cmd( "label .a.c.d -text \"Model Directory\"" );
cmd( "set complete_dir [file nativename [file join [pwd] $modeldir]]" );
cmd( "entry .a.c.ed -width 40 -state disabled -textvariable complete_dir" );

cmd( "label .a.c.v -text Version" );
cmd( "entry .a.c.ev -width 40 -textvariable version" );


cmd( "label .a.c.date -text \"Date Creation\"" );
if(choice==1)
{
  cmd( "set f [open $modeldir/modelinfo.txt r]" );
  cmd( "gets $f; gets $f; set date [gets $f]" ); 
  cmd( "close $f" );
}

if(choice==1)
 cmd( "entry .a.c.edate -width 40 -state disabled -textvariable date" );
else
  cmd( "entry .a.c.edate -width 40 -textvariable date" );

cmd( "cd $modeldir" );
make_makefile();
cmd( "set fapp [file nativename $modeldir/makefile]" );
s=(char *)Tcl_GetVar(inter, "fapp",0);

f=fopen(s, "r");


if(f==NULL)
 {
  cmd( "tk_messageBox -parent .a -title Error -icon error -type ok -message \"File 'makefile' not found\" -detail \"Use the 'Model Compilation Options', in menu Model, to create it.\"" );
  choice=0;
  cmd( "cd $RootLsd" );
  cmd( "if { [winfo exists .a] == 1} {destroytop .a} {}" );
  goto loop;
 }
fscanf(f, "%999s", str);
while(strncmp(str, "FUN=", 4) && fscanf(f, "%999s", str)!=EOF);
fclose(f);
if(strncmp(str, "FUN=", 4)!=0)
 {
  cmd( "tk_messageBox -parent .a -type ok -title Error -icon error -message \"Makefile corrupted\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
  choice=0;
  goto loop;
 }

cmd( "set eqname %s.cpp", str+4 );

cmd( "cd $RootLsd" );

cmd( "label .a.c.last -text \"Date Last Modification (equations only)\"" );
cmd( "set frmt \"%d %%B, %%Y\"" );
cmd( "set last \"[clock format [file mtime $modeldir/$eqname] -format \"$frmt\"]\"" );
cmd( "entry .a.c.elast -width 40 -state disabled -textvariable last" );


cmd( "pack .a.c.n .a.c.en .a.c.v .a.c.ev .a.c.date .a.c.edate .a.c.last .a.c.elast .a.c.d .a.c.ed" );

cmd( "frame .a.b" );
cmd( "button .a.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .a.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack .a.b.ok .a.b.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.c" );
cmd( "pack .a.b -side right" );
cmd( "bind .a <Escape> {set choice 2}" );
cmd( "bind .a.c.en <Return> {focus .a.c.ev}" );
cmd( "bind .a.c.ev <Return> {focus .a.b.ok}" );
cmd( "bind .a.b.ok <Return> {.a.b.ok invoke}" );
choice=0;

cmd( "showtop .a" );
cmd( "focus .a.c.en" );

while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==1)
 {
  cmd( "set f [open $modeldir/modelinfo.txt w]" );
  cmd( "puts $f \"$modelname\"" );
  cmd( "puts $f \"$version\"" );
  cmd( "puts $f \"$date\"" );
  cmd( "close $f" );
  cmd( ".f.hea.mod.dat conf -text \"$modelname\"" );
  cmd( ".f.hea.ver.dat conf -text \"$version\"" );
  
 }



choice=0;
goto loop;
 

}

if(choice==39)
 {
 /************
 create a new file
 *************/
 cmd( ".f.t.t delete 1.0 end" );
 cmd( "set before [.f.t.t get 1.0 end]" );
 cmd( "set filename noname.txt" );
 cmd( "set dirname [pwd]" );
 cmd( ".f.t.t mark set insert 1.0" );
 cmd( ".f.hea.file.dat conf -text \"noname.txt\"" );
 cmd( "wm title . \"nomame.txt - LMM\"" );
 cmd( "catch [unset -nocomplain ud]" );
 cmd( "catch [unset -nocomplain udi]" );
 cmd( "catch [unset -nocomplain rd]" );
 cmd( "catch [unset -nocomplain rdi]" );
 cmd( "lappend ud [.f.t.t get 0.0 end]" );
 cmd( "lappend udi [.f.t.t index insert]" );
 
 choice=0;
 goto loop;
 }

if(choice==46 || choice==49)
{
/*
Create the makefile
*/
cmd( "cd $modeldir" );

cmd( "set choice [file exists model_options.txt]" );
if(choice==0)
 {//the model_options.txt file does not exists, probably an old version
   cmd( "set dir [glob *.cpp]" );
   cmd( "set b [lindex $dir 0]" );
   cmd( "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-O3\\nSWITCH_CC_LNK=\\n\"" );
   cmd( "set f [open model_options.txt w]" );
   cmd( "puts -nonewline $f $a" );
   cmd( "close $f" );
 }
choice=0; 
cmd( "set f [open model_options.txt r]" );
cmd( "set a [read -nonewline $f]" );
cmd( "close $f" );

cmd( "cd $RootLsd" );

cmd( "set choice [file exists $LsdSrc/system_options.txt]" );
if(choice==0)
 { //the src/system_options.txt file doesn't exists, so I invent it
	cmd( "if [ string equal $tcl_platform(platform) windows ] { if [ string equal $tcl_platform(machine) intel ] { set sysfile \"sysopt_win32.txt\" } { set sysfile \"sysopt_win64.txt\" } } { if [ string equal $tcl_platform(os) Darwin ] { set sysfile \"sysopt_mac.txt\" } { set sysfile \"sysopt_linux.txt\" } }" );
    cmd( "set f [open $RootLsd/$LsdSrc/system_options.txt w]" );
    cmd( "set f1 [open $RootLsd/$LsdSrc/$sysfile r]" );
    cmd( "puts -nonewline $f \"LSDROOT=$RootLsd\\n\"" );
    cmd( "puts -nonewline $f [read $f1]" );
    cmd( "close $f" );
    cmd( "close $f1" );    
 }
choice=0; 

cmd( "set f [open $LsdSrc/system_options.txt r]" );
cmd( "set d [read -nonewline $f]" );
cmd( "close $f" );

cmd( "set f [open $LsdSrc/makefile_base.txt r]" );
cmd( "set b [read -nonewline $f]" );
cmd( "close $f" );

cmd( "cd $modeldir" );
cmd( "set c \"# Model compilation options\\n$a\\n\\n# System compilation option\\n$d\\n\\n# body of the makefile (in $LsdSrc/makefile_base.txt)\\n$b\"" );
cmd( "set f [open makefile w]" );
cmd( "puts -nonewline $f $c" );
cmd( "close $f" );

cmd( "cd $RootLsd" );
if(choice==46)
  choice=0; //just create the makefile
if(choice==49) //after this show the description file (and a model is created)
  choice=50;
  
goto loop;

} 



if(choice==47)
{
/*
System Compilation Options
*/

choice=0;
cmd( "set choice [file exists $LsdSrc/system_options.txt]" );

if(choice==1)
 {
  cmd( "set f [open $LsdSrc/system_options.txt r]" );
  cmd( "set a [read -nonewline $f]" );
  cmd( "close $f" );
  choice=0;
 }
else
  cmd( "set a \"\"" );


cmd( "newtop .l \"System Compilation Options\" { .l.t.d.b.esc invoke }" );

cmd( "frame .l.t" );

cmd( "scrollbar .l.t.yscroll -command \".l.t.text yview\"" );
cmd( "text .l.t.text -wrap word -font {Times 10 normal} -width 60 -height 16 -relief sunken -yscrollcommand \".l.t.yscroll set\"" );

cmd( "frame .l.t.d" );
cmd( "frame .l.t.d.os" );

cmd( "if [ string equal $tcl_platform(machine) intel ] { button .l.t.d.os.win -width -15 -text \"Default Windows x32\" -command { .l.t.text delete 1.0 end; set file [ open $RootLsd/$LsdSrc/sysopt_win32.txt r ]; set a [ read -nonewline $file ]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\" } } { button .l.t.d.os.win -width -15 -text \"Default Windows x64\" -command { .l.t.text delete 1.0 end; set file [ open $RootLsd/$LsdSrc/sysopt_win64.txt r ]; set a [ read -nonewline $file ]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\" } } " ); 
cmd( "button .l.t.d.os.lin -width -15 -text \"Default Linux\" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_linux.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}" );
cmd( "button .l.t.d.os.mac -width -15 -text \"Default Mac OSX\" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_mac.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}" ); 
cmd( "pack .l.t.d.os.win .l.t.d.os.lin .l.t.d.os.mac -padx 1 -pady 5 -side left" );

cmd( "frame .l.t.d.b" );
cmd( "button .l.t.d.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .l.t.d.b.help -width -9 -text Help -command {LsdHelp LMM_help.html#compilation_options}" );
cmd( "button .l.t.d.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack .l.t.d.b.ok .l.t.d.b.help .l.t.d.b.esc -padx 10 -pady 5 -side left" );
cmd( "pack .l.t.yscroll -side right -fill y" );
cmd( "pack .l.t.d.os" );
cmd( "pack .l.t.d.b -side right" );
cmd( "pack .l.t.text .l.t.d -expand yes -fill both" );
cmd( "pack .l.t" );

cmd( ".l.t.text insert end $a" );

//cmd( "bind .l <KeyPress-Return> {set choice 1}" );
cmd( "bind .l <KeyPress-Escape> {set choice 2}" );

cmd( "showtop .l" );
cmd( "focus .l.t.text" );

while(choice==0)
 Tcl_DoOneEvent(0);

if(choice==1)
 {
  cmd( "set f [open $LsdSrc/system_options.txt w]" );
  cmd( "puts -nonewline $f [.l.t.text get 1.0 end]" );
  cmd( "close $f" );
  choice=46; //go to create makefile
 }
else
 choice=0; 

cmd( "destroytop .l" );

goto loop;

} 

if(choice==48)
{
/*
Model Compilation Options
*/

if(s==NULL || !strcmp(s, ""))
 {
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"No model selected\" -detail \"Choose an existing model or create a new one.\"" );
  choice=0;
  goto loop;
 }

cmd( "cd $modeldir" );
s = ( char * ) Tcl_GetVar( inter, "modeldir", 0 );
strcpy( str, s );
strcat( str, "/model_options.txt" );
choice = 0;
f = fopen( str, "r" );
if ( f != NULL )
{
	while ( fscanf( f, "%499s", str ) != EOF && strncmp( str, "FUN=", 4 ) );    
	fclose( f );
	if ( ! strncmp( str, "FUN=", 4 ) )
	{
		strcat( str, ".cpp");
		Tcl_SetVar( inter, "b", str + 4, 0 );
		choice = 1;
	}
}

if ( choice == 0 )
{
	cmd( "set dir [glob fun_*.cpp]" );
	cmd( "set b [lindex $dir 0]" );
}

cmd( "set gcc_base \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC_LNK=\"" );
cmd( "if { [ string equal $tcl_platform(platform) windows ] && [ string equal $tcl_platform(machine) intel ] } { set gcc_par \"SWITCH_CC=-march=pentium-mmx -mtune=prescott\" } { set gcc_par \"SWITCH_CC=-march=native\" }" );
cmd( "set gcc_conf \"$gcc_base\\n$gcc_par\"" );
cmd( "set gcc_deb \"-O0 -g\"" );
cmd( "set gcc_opt \"-O3\"" );

if(choice==1)
 {
  cmd( "set f [open model_options.txt r]" );
  cmd( "set a [read -nonewline $f]" );
  cmd( "close $f" );
  choice=0;
 }
else
  {
   cmd( "tk_messageBox -parent . -type ok -title Warning -icon warning -message \"Model compilation options not found or corrupted\" -detail \"The system will use default values.\"" );
   cmd( "set a \"$gcc_conf $gcc_opt\\n\"" );
   cmd( "set f [open model_options.txt w]" );
   cmd( "puts -nonewline $f $a" );
   cmd( "close $f" );

  }
cmd( "newtop .l \"Model Compilation Options\" { .l.t.d.b.esc invoke }" );

cmd( "frame .l.t" );

cmd( "scrollbar .l.t.yscroll -command \".l.t.text yview\"" );
cmd( "text .l.t.text -wrap word -font {Times 10 normal} -width 60 -height 10 -relief sunken -yscrollcommand \".l.t.yscroll set\"" );

cmd( "frame .l.t.d" );
cmd( "frame .l.t.d.opt" );
cmd( "if { ! [ info exists debug ] } { set debug 0 }; checkbutton .l.t.d.opt.debug -text Debug -variable debug -command { .l.t.d.opt.def invoke }" );

cmd( "button .l.t.d.opt.def -width -15 -text \"Default Values\" -command { if { $debug == 0 } { set default \"$gcc_conf $gcc_opt\\n\" } { set default \"$gcc_conf $gcc_deb\\n\" }; .l.t.text delete 1.0 end; .l.t.text insert end \"$default\" }" );

cmd( "button .l.t.d.opt.cle -width -15 -text \"Clean Pre-Compiled Files\" -command { if { [ catch { glob $RootLsd/$LsdSrc/*.o } objs ] == 0 } { foreach i $objs { catch { file delete -force $i } } } }" );

cmd( "pack .l.t.d.opt.debug .l.t.d.opt.def .l.t.d.opt.cle -padx 10 -pady 5 -side left" );

cmd( "frame .l.t.d.b" );
cmd( "button .l.t.d.b.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .l.t.d.b.help -width -9 -text Help -command {LsdHelp LMM_help.html#compilation_options}" );
cmd( "button .l.t.d.b.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "pack .l.t.d.b.ok .l.t.d.b.help .l.t.d.b.esc -padx 10 -pady 5 -side left" );
cmd( "pack .l.t.d.opt" );
cmd( "pack .l.t.d.b -side right" );
cmd( "pack .l.t.yscroll -side right -fill y" );
cmd( "pack .l.t.text .l.t.d -expand yes -fill both" );
cmd( "pack .l.t" );
cmd( "bind .l <KeyPress-Return> {set choice 1}" );
cmd( "bind .l <KeyPress-Escape> {set choice 2}" );
cmd( ".l.t.text insert end $a" );

cmd( "showtop .l" );
cmd( "focus .l.t.text" );

while(choice==0)
 Tcl_DoOneEvent(0);

if(choice==1)
 {
  cmd( "set f [open model_options.txt w]" );
  cmd( "puts -nonewline $f [.l.t.text get 1.0 end]" );
  cmd( "close $f" );
  choice=46; //go to create makefile
 }
else
 choice=0; 

cmd( "cd $RootLsd" );

cmd( "destroytop .l" );

goto loop;

} 
if(choice==57)
{
//launch tkdiff



cmd( "if {$tcl_platform(platform) == \"unix\"} {if {$tcl_platform(os) == \"Darwin\" } {set choice 1} {set choice 1} } {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\"} {set choice 4} {set choice 2}} {set choice 3}}" );
if(choice==1) //unix
 cmd( "exec $wish $LsdSrc/tkdiff.tcl &" );
if(choice==2) //win2k
 cmd( "exec $wish $LsdSrc/tkdiff.tcl &" ); //Changed
if(choice==3) //win 95/98
 cmd( "exec start $wish $LsdSrc/tkdiff.tcl &" );
if(choice==4)  //win NT
 cmd( "exec cmd /c start $wish $LsdSrc/tkdiff.tcl &" );
 
choice=0;
goto loop;

}

if(choice==59)
{
//Change font

cmd( "newtop .a \"Change Font\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Enter the font name you wish to use\"" );

cmd( "entry .a.v_num -width 30 -textvariable fonttype" );
cmd( "bind .a.v_num <Return> {focus .a.f.ok}" );


cmd( "frame .a.f" );	
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp LMM_help.html#changefont}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left" );
cmd( "pack .a.l1 .a.v_num" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( "focus .a.v_num" );
cmd( ".a.v_num selection range 0 end" );

choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd( "set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"; settab .f.t.t $tabsize \"$a\"" );



choice=0;
goto loop;

}


if(choice==60)
{
//LMM system options

cmd( "set temp_var1 $Terminal" );
cmd( "set temp_var2 $HtmlBrowser" );
cmd( "set temp_var3 $fonttype" );
cmd( "set temp_var4 $wish" );
cmd( "set temp_var5 $LsdSrc" );
cmd( "set temp_var6 $dim_character" );
cmd( "set temp_var7 $tabsize" );
cmd( "set temp_var8 $wrap" );
cmd( "set temp_var9 $shigh" );
cmd( "set temp_var10 $autoHide" );
cmd( "set temp_var11 $showFileCmds" );
cmd( "set temp_var12 $LsdNew" );

cmd( "newtop .a \"LMM Options\" { .a.f2.esc invoke }" );

cmd( "frame .a.num" );
cmd( "label .a.num.l -text \"Terminal for the GDB debugger\"" );
cmd( "entry .a.num.v -width 25 -textvariable temp_var1" );
cmd( "pack .a.num.l .a.num.v" );
cmd( "bind .a.num.v <Return> {focus .a.num2.v; .a.num2.v selection range 0 end}" );

cmd( "frame .a.num2" );
cmd( "label .a.num2.l -text \"HTML browser for help pages\"" );
cmd( "entry .a.num2.v -width 25 -textvariable temp_var2" );
cmd( "pack .a.num2.l .a.num2.v" );
cmd( "bind .a.num2.v <Return> {focus .a.num4.v; .a.num4.v selection range 0 end}" );

cmd( "frame .a.num4" );
cmd( "label .a.num4.l -text \"Wish program\"" );
cmd( "entry .a.num4.v -width 25 -textvariable temp_var4" );
cmd( "pack .a.num4.l .a.num4.v" );
cmd( "bind .a.num4.v <Return> {focus .a.num12.v; .a.num12.v selection range 0 end}" );

cmd( "frame .a.num12" );
cmd( "label .a.num12.l -text \"New models subdirectory\"" );
cmd( "entry .a.num12.v -width 25 -textvariable temp_var12" );
cmd( "pack .a.num12.l .a.num12.v" );
cmd( "bind .a.num12.v <Return> {focus .a.num5.v; .a.num5.v selection range 0 end}" );

cmd( "frame .a.num5" );
cmd( "label .a.num5.l -text \"Source code subdirectory\"" );
cmd( "entry .a.num5.v -width 25 -textvariable temp_var5" );
cmd( "pack .a.num5.l .a.num5.v" );
cmd( "bind .a.num5.v <Return> {focus .a.num3.v; .a.num3.v selection range 0 end}" );

cmd( "frame .a.num3" );
cmd( "label .a.num3.l -text \"Font family\"" );
cmd( "entry .a.num3.v -width 25 -textvariable temp_var3" );
cmd( "pack .a.num3.l .a.num3.v" );
cmd( "bind .a.num3.v <Return> {focus .a.num6.v; .a.num6.v selection range 0 end}" );

cmd( "frame .a.num6" );
cmd( "label .a.num6.l -text \"Font size (points)\"" );
cmd( "entry .a.num6.v -width 3 -textvariable temp_var6 -justify center" );
cmd( "pack .a.num6.l .a.num6.v" );
cmd( "bind .a.num6.v <Return> {focus .a.num7.v; .a.num7.v selection range 0 end}" );

cmd( "frame .a.num7" );
cmd( "label .a.num7.l -text \"Tab size (characters)\"" );
cmd( "entry .a.num7.v -width 2 -textvariable temp_var7 -justify center" );
cmd( "pack .a.num7.l .a.num7.v" );
cmd( "bind .a.num7.v <Return> {focus .a.num9.v3}" );

cmd( "frame .a.num9" );
cmd( "label .a.num9.l -text \"Syntax highlights\"" );
cmd( "pack .a.num9.l" );
cmd( "radiobutton .a.num9.v1 -variable temp_var9 -value 0 -text None" );
cmd( "radiobutton .a.num9.v2 -variable temp_var9 -value 1 -text Partial" );
cmd( "radiobutton .a.num9.v3 -variable temp_var9 -value 2 -text Full" );
cmd( "pack .a.num9.v1 .a.num9.v2 .a.num9.v3 -side left" );
cmd( "bind .a.num9.v1 <Return> { focus .a.v_num8 }" );
cmd( "bind .a.num9.v2 <Return> { focus .a.v_num8 }" );
cmd( "bind .a.num9.v3 <Return> { focus .a.v_num8 }" );

cmd( "checkbutton .a.v_num8 -variable temp_var8 -text \"Wrap text\"" );
cmd( "bind .a.v_num8 <Return> { focus .a.v_num10 }" );

cmd( "checkbutton .a.v_num10 -variable temp_var10 -text \"Auto hide on run\"" );
cmd( "bind .a.v_num10 <Return> { focus .a.v_num11 }" );

cmd( "checkbutton .a.v_num11 -variable temp_var11 -text \"Show text file commands\"" );
cmd( "bind .a.v_num11 <Return> { focus .a.f2.ok }" );

cmd( "pack .a.num .a.num2 .a.num4 .a.num12 .a.num5 .a.num3 .a.num6 .a.num7 .a.num9 .a.v_num8 .a.v_num10 .a.v_num11 -padx 5 -pady 5" );

cmd( "frame .a.f1" );
cmd( "button .a.f1.def -width -9 -text Default -command {set temp_var1 $DefaultTerminal; set temp_var2 $DefaultHtmlBrowser; set temp_var3 $DefaultFont; set temp_var5 src; set temp_var6 12; set temp_var7 2; set temp_var8 1; set temp_var9 2; set temp_var10 0; set temp_var11 0; set temp_var12 Work}" );
cmd( "button .a.f1.help -width -9 -text Help -command {LsdHelp LMM_help.html#SystemOpt}" );
cmd( "pack .a.f1.def .a.f1.help -padx 10 -side left" );

cmd( "frame .a.f2" );
cmd( "button .a.f2.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "button .a.f2.esc -width -9 -text Cancel -command {set choice 2}" );

cmd( "pack .a.f2.ok .a.f2.esc -padx 10 -pady 10 -side left" );
cmd( "pack .a.f1" );
cmd( "pack .a.f2" );
cmd( "bind .a.f2.ok <Return> {.a.f2.ok invoke}" );
cmd( "bind .a <Escape> {.a.f2.esc invoke}" );

cmd( "showtop .a" );
cmd( "focus .a.num.v" );
cmd( ".a.num.v selection range 0 end" );

choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==1)
 {
 cmd( "if { $showFileCmds != $temp_var11 } { tk_messageBox -parent . -icon warning -title Warning -type ok -message \"Restart required\" -detail \"Restart required after configuration changes. Only after LMM is closed and restarted the changes in the menu configuration will be used.\" }" );

 cmd( "set Terminal $temp_var1" );
 cmd( "set HtmlBrowser $temp_var2" );
 cmd( "set fonttype $temp_var3" );
 cmd( "set wish $temp_var4" );
 cmd( "set LsdSrc $temp_var5" );
 cmd( "if [string is integer $temp_var6] {set dim_character $temp_var6}" );
 cmd( "if [string is integer $temp_var7] {set tabsize $temp_var7}" );
 cmd( "set wrap $temp_var8" );
 cmd( "set shigh $temp_var9" );
 cmd( "set autoHide $temp_var10" );
 cmd( "set showFileCmds $temp_var11" );
 cmd( "set LsdNew $temp_var12" );
 
 cmd( "set a [list $fonttype $dim_character]" );
 cmd( ".f.t.t conf -font \"$a\"" );
 cmd( "settab .f.t.t $tabsize \"$a\"" );	// adjust tabs size to font type/size
 cmd( "setwrap .f.t.t $wrap" );			// adjust text wrap
 color(shigh, 0, 0);							// set color highlights (all text)
cmd( "set f [open $RootLsd/lmm_options.txt w]" );
 cmd( "puts -nonewline $f  \"$Terminal\n\"" );
 cmd( "puts $f $HtmlBrowser" );
 cmd( "puts $f $fonttype" );
 cmd( "puts $f $wish" );
 cmd( "puts $f $LsdSrc" );
 cmd( "puts $f $dim_character" );
 cmd( "puts $f $tabsize" );
 cmd( "puts $f $wrap" );
 cmd( "puts $f $shigh" );
 cmd( "puts $f $autoHide" );
 cmd( "puts $f $showFileCmds" );
 cmd( "puts $f $LsdNew" );
 cmd( "close $f" );
 }

choice=0;
goto loop;
}


if(choice==61)
{
// start tkdiff with model selection
choice=0;

cmd( "chs_mdl" );
while(choice==0)
 Tcl_DoOneEvent(0);

if(choice==-1)
 {
  choice=0;
  goto loop;

 }
 
cmd( "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\"} {set choice 4} {set choice 2}} {set choice 3}}" );
if(choice==1) //unix
 cmd( "exec $wish src/tkdiff.tcl [file join $d1 $f1] [file join $d2 $f2] &" );
if(choice==2) //win2k
 cmd( "exec $wish src/tkdiff.tcl [file join $d1 $f1] [file join $d2 $f2]  &" ); //Changed
if(choice==3) //win 95/98
 cmd( "exec start $wish src/tkdiff.tcl [file join $d1 $f1] [file join $d2 $f2]  &" );
if(choice==4)  //win NT
 cmd( "exec cmd /c start $wish src/tkdiff.tcl [file join $d1 $f1] [file join $d2 $f2]  &" );
 
choice=0;
goto loop;
}


if(choice==62)
{
/*
Generate the no window distribution
*/
cmd( "cd $modeldir" );

make_makefileNW();

cmd( "if { ! [ file exists src ] } { file mkdir src }" );

cmd( "file copy -force $RootLsd/$LsdSrc/lsdmain.cpp src" );
cmd( "file copy -force $RootLsd/$LsdSrc/main_gnuwin.cpp src" );
cmd( "file copy -force $RootLsd/$LsdSrc/file.cpp src" );
cmd( "file copy -force $RootLsd/$LsdSrc/variab.cpp src" );
cmd( "file copy -force $RootLsd/$LsdSrc/object.cpp src" );
cmd( "file copy -force $RootLsd/$LsdSrc/report.cpp src" );
cmd( "file copy -force $RootLsd/$LsdSrc/util.cpp src" );
cmd( "file copy -force $RootLsd/$LsdSrc/nets.cpp src" );
cmd( "file copy -force $RootLsd/$LsdSrc/fun_head.h src" );
cmd( "file copy -force $RootLsd/$LsdSrc/decl.h src" );
cmd( "file copy -force $RootLsd/$LsdSrc/system_options.txt src" );

cmd( "set f [open src/choose.h w]" );
cmd( "puts -nonewline $f \"#define NO_WINDOW\\n\"" );
cmd( "close $f" );

// Compile a local machine version of lsd_gnuNW
cmd( "set fapp [file nativename $modeldir/makefileNW]" );
s=(char *)Tcl_GetVar(inter, "fapp",0);
f=fopen(s, "r");
if(f==NULL)
{
  cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"Makefile not created\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
  goto end_compNW;
}
fscanf(f, "%999s", str);
while(strncmp(str, "FUN=", 4) && fscanf(f, "%999s", str)!=EOF);
fclose(f);
if(strncmp(str, "FUN=", 4)!=0)
{
  cmd( "tk_messageBox -parent . -type ok -title Error -icon error -message \"Makefile corrupted\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
  goto end_compNW;
}
cmd( "set fname %s.cpp", str+4 );

f=fopen(s, "r");
fscanf(f, "%999s", str);
while(strncmp(str, "TARGET=", 7) && fscanf(f, "%999s", str)!=EOF);
fclose(f);
if(strncmp(str, "TARGET=", 7)!=0)
{
  cmd( "tk_messageBox -parent . -type ok -title Error -icon error -message \"Makefile corrupted\" -detail \"Check 'Model Compilation Options' and 'System Compilation Options' in menu Model.\"" );
  goto end_compNW;
}

strcat(str,"NW");

cmd( "set init_time [clock seconds]" ); 
cmd( "newtop .t \"Please Wait\"" );
cmd( "label .t.l1 -font {-weight bold} -text \"Making non-graphical version of model...\"" );
cmd( "label .t.l2 -text \"The executable 'lsd_gnuNW' for this system is being created.\nThe make file 'makefileNW' and the 'src' folder are being created\nin the model folder and can be used to recompile the\n'No Window' version in other systems.\"" );
cmd( "pack .t.l1 .t.l2 -padx 5 -pady 5" );
cmd( "showtop .t centerW" );

cmd( "if { [ string equal $tcl_platform(platform) windows ] && ! [ string equal $tcl_platform(machine) amd64 ] } {set choice 1} {set choice 0}" );
if(choice==0)
  cmd( "catch { exec make -f makefileNW 2> makemessage.txt } result" ); 
else
{  
  cmd( "set file [open make.bat w]" );
  cmd( "puts -nonewline $file \"make.exe -fmakefileNW 2> makemessage.txt\\n\"" );
  cmd( "close  $file" );
  cmd( "if { [file exists $RootLsd/$LsdSrc/system_options.txt] == 1} {set choice 0} {set choice 1}" );
  cmd( "if { [file exists %s.exe]  == 1} {file rename -force %s.exe %sOld.exe} { }", str+7, str+7, str+7 );
  cmd( "if { [file exists $RootLsd/$LsdGnu/bin/crtend.o] == 1} { file copy -force $RootLsd/$LsdGnu/bin/crtend.o .;file copy -force $RootLsd/$LsdGnu/bin/crtbegin.o .;file copy -force $RootLsd/$LsdGnu/bin/crt2.o .} {}" );
  cmd( "catch { exec make.bat } result" );
  cmd( "file delete make.bat" );
  cmd( "if { [file exists crtend.o] == 1} { file delete crtend.o;file delete crtbegin.o ;file delete crt2.o } {}" );
}

cmd( "destroytop .t" );

cmd( "if { [ file size makemessage.txt] == 0 } { file delete makemessage.txt; set choice 0 } { set choice 1 }" );
if(choice==1)
{
  cmd( "if { $tcl_platform(platform) == \"windows\"} {set add_exe \".exe\"} {set add_exe \"\"}" );
  cmd( "set funtime [file mtime $fname]" );
  cmd( "if { [file exist %s$add_exe] == 1 } {set exectime [file mtime %s$add_exe]} {set exectime $init_time}", str+7,str+7 );
  cmd( "if {$init_time < $exectime } {set choice 0} { }" );
  //turn choice into 0 if the executable is newer than the compilation command, implying just warnings
}

if(choice==1)
  cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Problem generating 'No Window' version\" -detail \"Probably there is a problem with your model.\\n\\nBefore using this option, make sure you are able to run the model with the 'Model'/'Compile and Run Model' option without errors. The error list is in the file 'makemessage.txt'.\"" );
else
  cmd( "tk_messageBox -parent . -type ok -icon info -title \"No Window Version\" -message \"Compilation successful\" -detail \"LMM has created a non-graphical version of the model, to be transported on any system endowed with a GCC compiler and standard libraries.\\n\\nA local system version of the executable 'lsd_gnuNW' was also generated in your current model folder and is ready to use in this computer.\\n\\nTo move the model in another system copy the content of the model's directory:\\n$modeldir\\nincluding also its new subdirectory 'src'.\\n\\nTo create a 'No Window' version of the model program follow these steps, to be executed within the directory of the model:\\n- compile with the command 'make -f makefileNW'\\n- run the model with the command 'lsd_gnuNW -f mymodelconf.lsd'\\n- the simulation will run automatically saving the results (for the variables indicated in the conf. file) in Lsd result files named after the seed generator used.\"" );

end_compNW:
cmd( "cd $RootLsd" );
choice=0;
goto loop;
}

if(choice==64)
{
/*
Full syntax highlighting
*/
shigh=2;
Tcl_UpdateLinkedVar(inter, "shigh");
color(shigh, 0, 0);			// set color types (all text)

choice=0;
goto loop;
}

if(choice==65)
{
/*
Partial syntax highlighting
*/
shigh=1;
Tcl_UpdateLinkedVar(inter, "shigh");
// select entire text to be color adjusted
color(shigh, 0, 0);			// set color types (all text)

choice=0;
goto loop;
}

if(choice==66)
{
/*
No syntax highlighting
*/
shigh=0;
Tcl_UpdateLinkedVar(inter, "shigh");
color(shigh, 0, 0);			// set color types (all text)

choice=0;
goto loop;
}

if(choice==67)
{
//Change tab size

cmd( "newtop .a \"Change Tab Size\" { .a.f.esc invoke }" );

cmd( "label .a.l1 -text \"Enter the tab size (characters)\"" );
cmd( "entry .a.v_num -justify center -width 10 -textvariable tabsize -justify center" );
cmd( "bind .a.v_num <Return> {focus .a.f.ok}" );
cmd( "frame .a.f" );
cmd( "button .a.f.ok -width -9 -text Ok -command {set choice 1}" );
cmd( "bind .a.f.ok <Return> {.a.f.ok invoke}" );
cmd( "button .a.f.help -width -9 -text Help -command {LsdHelp LMM_help.html#changetab}" );
cmd( "button .a.f.esc -width -9 -text Cancel -command {set choice 2}" );
cmd( "bind .a <Escape> {.a.f.esc invoke}" );
cmd( "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left" );
cmd( "pack .a.l1 .a.v_num" );
cmd( "pack .a.f -side right" );

cmd( "showtop .a" );
cmd( ".a.v_num selection range 0 end" );
cmd( "focus .a.v_num" );

choice=0;
while(choice==0)
	Tcl_DoOneEvent(0);

cmd( "destroytop .a" );

if(choice==2)
{
	choice=0;
	goto loop;
}
cmd( "if [string is integer $tabsize] {settab .f.t.t $tabsize \"[.f.t.t cget -font]\"}" );
choice=0;
goto loop;
}

if ( choice == 68 )
{	// Adjust context menu for LSD macros
	cmd( "destroy .v.i" );
	cmd( "menu .v.i -tearoff 0" );
	cmd( ".v.i add command -label \"Lsd equation\" -command {set choice 25} -accelerator Ctrl+E" );
	cmd( ".v.i add command -label \"V(...)\" -command {set choice 26} -accelerator Ctrl+V" );
	cmd( ".v.i add command -label \"SUM(...)\" -command {set choice 56} -accelerator Ctrl+U" );
	cmd( ".v.i add command -label \"SEARCH_CND(...)\" -command {set choice 30} -accelerator Ctrl+S" );
	cmd( ".v.i add command -label \"SEARCH(...)\" -command {set choice 55} -accelerator Ctrl+A" );
	cmd( ".v.i add command -label \"WRITE(...)\" -command {set choice 29} -accelerator Ctrl+W" );
	cmd( ".v.i add command -label \"CYCLE(...)\" -command {set choice 27} -accelerator Ctrl+C" );
	cmd( ".v.i add command -label \"SORT(...)\" -command {set choice 31} -accelerator Ctrl+T" );
	cmd( ".v.i add command -label \"ADDOBJ(...)\" -command {set choice 52} -accelerator Ctrl+O" );
	cmd( ".v.i add command -label \"DELETE(...))\" -command {set choice 53} -accelerator Ctrl+D" );
	cmd( ".v.i add command -label \"RNDDRAW(...)\" -command {set choice 54} -accelerator Ctrl+N" );
	cmd( ".v.i add command -label \"INCR(...)\" -command {set choice 40} -accelerator Ctrl+I" );
	cmd( ".v.i add command -label \"MULT(...)\" -command {set choice 45} -accelerator Ctrl+M" );
	cmd( ".v.i add command -label \"Math operation\" -command {set choice 51} -accelerator Ctrl+H" );
	choice = 0;
	goto loop;
}

if ( choice == 69 )
{	// Adjust context menu for LSD C++
	cmd( "destroy .v.i" );
	cmd( "menu .v.i -tearoff 0" );
	cmd( ".v.i add command -label \"Lsd equation\" -command {set choice 25} -accelerator Ctrl+E" );
	cmd( ".v.i add command -label \"cal(...)\" -command {set choice 26} -accelerator Ctrl+V" );
	cmd( ".v.i add command -label \"sum(...)\" -command {set choice 56} -accelerator Ctrl+U" );
	cmd( ".v.i add command -label \"search_var_cond(...)\" -command {set choice 30} -accelerator Ctrl+S" );
	cmd( ".v.i add command -label \"search(...)\" -command {set choice 55} -accelerator Ctrl+A" );
	cmd( ".v.i add command -label \"write(...)\" -command {set choice 29} -accelerator Ctrl+W" );
	cmd( ".v.i add command -label \"for( ; ; )\" -command {set choice 27} -accelerator Ctrl+C" );
	cmd( ".v.i add command -label \"lsdqsort(...)\" -command {set choice 31} -accelerator Ctrl+T" );
	cmd( ".v.i add command -label \"add_n_objects2\" -command {set choice 52} -accelerator Ctrl+O" );
	cmd( ".v.i add command -label \"delete_obj\" -command {set choice 53} -accelerator Ctrl+D" );
	cmd( ".v.i add command -label \"draw_rnd\" -command {set choice 54} -accelerator Ctrl+N" );
	cmd( ".v.i add command -label \"increment(...)\" -command {set choice 40} -accelerator Ctrl+I" );
	cmd( ".v.i add command -label \"multiply(...)\" -command {set choice 45} -accelerator Ctrl+M" );
	cmd( ".v.i add command -label \"Math operation\" -command {set choice 51} -accelerator Ctrl+H" );
	choice = 0;
	goto loop;
}

if(choice!=1)
 {choice=0;
 goto loop;
 }

Tcl_UnlinkVar(inter, "choice");
Tcl_UnlinkVar(inter, "num");
Tcl_UnlinkVar(inter, "tosave");
Tcl_UnlinkVar(inter, "macro");
Tcl_UnlinkVar(inter, "shigh");

return 0;
}

// data structures for color syntax (used by color/rm_color)
struct hit
{
	int type, count;
	long iniLin, iniCol;
	char previous, next;
};
// color types (0-n) to Tk tags mapping
const char *cTypes[] = {"comment1", "comment2", "cprep", "str", "lsdvar", "lsdmacro", "ctype", "ckword"};
// regular expressions identifying colored text types
 const char *cRegex[] = {
	"/\[*].*\[*]/",		// each item define one different color
	"//.*",
	"^(\\s)*#\[^/]*",
	"\\\"\[^\\\"]*\\\"",
	"v\\[\[0-9]{1,2}]|cur(l)?\[0-9]?",
	"MODEL(BEGIN|END)|(END_)?EQUATION|FUNCTION|RESULT|ABORT|DEBUG(_AT)?|CURRENT|V[LS]*(_CHEAT)?|V(S)?_(NODEID)?(NODENAME)?(WEIGHT)?|SUM|SUM[LS]*|STAT(S)?(_NET)?(_NODE)?|WHTAVE[LS]*|INCR(S)?|MULT(S)?|CYCLE(S)?(_LINK)?|CYCLE_SAFE(S)?|MAX[LS]*|WRITE[LS]*(_NODEID)?(_NODENAME)?(_WEIGHT)?|SEARCH_CND[LS]*|SEARCH(S)?(_NET)?(_LINK)?|TSEARCHS(_INI)?|SORT[S2]*|ADD(N)?OBJ(S)?(_EX)?|DELETE|RND|UNIFORM|RNDDRAW(FAIR)?(TOT)?[LS]*(_NET)?|PARAMETER|INTERACT(S)?|rnd_integer|norm|poisson|gamma|abs|min|max|round|exp|log|sqrt|pow|(init)?(update)?(save)?_lattice|NETWORK(S)?(_INI)?(_LOAD)?(_SAVE)?(_DEL)?(_SNAP)?|SHUFFLE(S)?|ADDLINK[WS]*|DELETELINK|LINK(TO|FROM)",
	"auto|const|double|float|int|short|struct|unsigned|long|signed|void|enum|register|volatile|char|extern|static|union|asm|bool|explicit|template|typename|class|friend|private|inline|public|virtual|mutable|protected|wchar_t",
	"break|continue|else|for|switch|case|default|goto|sizeof|typedef|do|if|return|while|dynamic_cast|namespace|reinterpret_cast|try|new|static_cast|typeid|catch|false|operator|this|using|throw|delete|true|const_cast|cin|endl|iomanip|main|npos|std|cout|include|iostream|NULL|string"
};

// count words in a string (used by color)
int strwrds(char string[])
{
	int i = 0, words = 0;
	char lastC = '\0';
	if(string == NULL) return 0;
	while(isspace(string[i])) i++;
	if(string[i] == '\0') return 0;
	for( ; string[i] != '\0'; lastC = string[i++])
		if(isspace(string[i]) && ! isspace(lastC)) words++;
	if(isspace(lastC)) return words;
	else return words + 1;
}

// map syntax highlight level to the number of color types to use
#define ITEM_COUNT( ptrArray )  ( sizeof( ptrArray ) / sizeof( ptrArray[0] ) )
int map_color(int hiLev)
{
	if(hiLev == 0)
		return 0;
	if(hiLev == 1)
		return 4;
	if(ITEM_COUNT(cTypes) > ITEM_COUNT(cRegex))
		return ITEM_COUNT(cRegex);
	return ITEM_COUNT(cTypes);
}

// compare function for qsort to compare different color hits (used by color)
int comphit(const void *p1, const void *p2)
{
	if(((hit*)p1)->iniLin < ((hit*)p2)->iniLin) return -1;
	if(((hit*)p1)->iniLin > ((hit*)p2)->iniLin) return 1;
	if(((hit*)p1)->iniCol < ((hit*)p2)->iniCol) return -1;
	if(((hit*)p1)->iniCol > ((hit*)p2)->iniCol) return 1;
	if(((hit*)p1)->type < ((hit*)p2)->type) return -1;
	if(((hit*)p1)->type > ((hit*)p2)->type) return 1;
	return 0;
}

/* New color routine, using new tcl/tk 8.5 search for all feature */
#define TOT_COLOR ITEM_COUNT( cTypes )
void color(int hiLev, long iniLin, long finLin)
{
int i, maxColor, newCnt;
long j, k, tsize = 0, curLin = 0, curCol = 0, newLin, newCol, size[TOT_COLOR];
char type, *pcount, *ppos, *count[TOT_COLOR], *pos[TOT_COLOR], finStr[16], *s;
struct hit *hits;

// prepare parameters
maxColor = map_color(hiLev);	// convert option to # of color types
if(finLin == 0)			// convert code 0 for end of text
	sprintf(finStr, "end");
else
	sprintf(finStr, "%ld.end", finLin);

// remove color tags
for(i = 0; i < TOT_COLOR; i++)
{
	cmd( ".f.t.t tag remove %s %ld.0 %s", cTypes[i], iniLin == 0 ? 1 : iniLin, finStr );
}

// find & copy all occurrence types to arrays of C strings
for(i = 0; i < maxColor; i++)
{
	// locate all occurrences of each color group
	Tcl_UnsetVar(inter, "ccount", 0);
	if(!strcmp(cTypes[i], "comment1"))	// multi line search element?
		cmd( "set pos [.f.t.t search -regexp -all -nolinestop -count ccount -- {%s} %ld.0 %s]", cRegex[i], iniLin == 0 ? 1 : iniLin, finStr);
	else
		cmd( "set pos [.f.t.t search -regexp -all -count ccount -- {%s} %ld.0 %s]", cRegex[i], iniLin == 0 ? 1 : iniLin, finStr );

	// check number of ocurrences
	pcount = (char*)Tcl_GetVar(inter, "ccount", 0);
	size[i] = strwrds(pcount);
	if(size[i] == 0)				// nothing to do?
		continue;
	tsize += size[i];

	// do intermediate store in C memory
	count[i] = (char*)calloc(strlen(pcount) + 1, sizeof(char));
	strcpy(count[i], pcount);
	ppos = (char*)Tcl_GetVar(inter, "pos", 0);
	pos[i] = (char*)calloc(strlen(ppos) + 1, sizeof(char));
	strcpy(pos[i], ppos); 
}
if(tsize == 0)
	return;							// nothing to do

// organize all occurrences in a single array of C numbers (struct hit)
hits = (hit*)calloc(tsize, sizeof(hit));
for(i = 0, k = 0; i < maxColor; i++)
{
	if(size[i] == 0)				// nothing to do?
		continue;
	pcount = (char*)count[i] - 1;
	ppos = (char*)pos[i] - 1;
	for(j = 0; j < size[i] && k < tsize; j++, k++)
	{
		hits[k].type = i;
		s = strtok(pcount + 1, " \t");
		hits[k].count = atoi(s);
		pcount = s + strlen(s);
		s = strtok(ppos + 1, " \t");
		sscanf(strtok(s, " \t"), "%ld.%ld", &hits[k].iniLin, &hits[k].iniCol);
		ppos = s + strlen(s);
	}
	free(count[i]);
	free(pos[i]);
}

// Sort the single list for processing
qsort((void *)hits, tsize, sizeof(hit), comphit);

// process each occurrence, if applicable
Tcl_LinkVar(inter, "lin", (char*)&newLin, TCL_LINK_LONG | TCL_LINK_READ_ONLY);
Tcl_LinkVar(inter, "col", (char*)&newCol, TCL_LINK_LONG | TCL_LINK_READ_ONLY);
Tcl_LinkVar(inter, "cnt", (char*)&newCnt, TCL_LINK_INT | TCL_LINK_READ_ONLY);
for(k = 0; k < tsize; k++)
	// skip occurrences inside other occurrence
	if(hits[k].iniLin > curLin || (hits[k].iniLin == curLin && hits[k].iniCol >= curCol))
	{
		newLin = hits[k].iniLin;
		newCol = hits[k].iniCol;
		newCnt = hits[k].count;
		cmd( "set end [.f.t.t index \"$lin.$col + $cnt char\"]" );
		// treats each type of color case properly
		if(hits[k].type < 4)			// non token?
			cmd( ".f.t.t tag add %s $lin.$col $end", cTypes[hits[k].type]);
		else							// token - should not be inside another word
			cmd( "if {[regexp {\\w} [.f.t.t get \"$lin.$col - 1 any chars\"]]==0 && [regexp {\\w} [.f.t.t get $end]]==0} {.f.t.t tag add %s $lin.$col $end}", cTypes[hits[k].type] );
		// next search position
		ppos = (char*)Tcl_GetVar(inter, "end", 0);
		sscanf(ppos, "%ld.%ld", &curLin, &curCol);
	}
Tcl_UnlinkVar(inter, "lin");
Tcl_UnlinkVar(inter, "col");
Tcl_UnlinkVar(inter, "cnt");
free(hits);
}


void make_makefile(void)
{
cmd( "set choice [file exists model_options.txt]" );
if(choice==0)
 {//the model_options.txt file does not exists, probably an old version
   cmd( "set dir [glob *.cpp]" );
   cmd( "set b [lindex $dir 0]" );
   cmd( "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-O3\\nSWITCH_CC_LNK=\\n\"" );
   cmd( "set f [open model_options.txt w]" );
   cmd( "puts -nonewline $f $a" );
   cmd( "close $f" );
 }
choice=0; 
cmd( "set f [open model_options.txt r]" );
cmd( "set a [read -nonewline $f]" );
cmd( "close $f" );

cmd( "set choice [file exists $RootLsd/$LsdSrc/system_options.txt]" );
if(choice==0)
 { //the src/system_options.txt file doesn't exists, so I invent it
	cmd( "if [ string equal $tcl_platform(platform) windows ] { if [ string equal $tcl_platform(machine) intel ] { set sysfile \"sysopt_win32.txt\" } { set sysfile \"sysopt_win64.txt\" } } { if [ string equal $tcl_platform(os) Darwin ] { set sysfile \"sysopt_mac.txt\" } { set sysfile \"sysopt_linux.txt\" } }" );
    cmd( "set f [open $RootLsd/$LsdSrc/system_options.txt w]" );
    cmd( "set f1 [open $RootLsd/$LsdSrc/$sysfile r]" );
    cmd( "puts -nonewline $f \"LSDROOT=$RootLsd\\n\"" );
    cmd( "puts -nonewline $f [read $f1]" );
    cmd( "close $f" );
    cmd( "close $f1" );    
 }
choice=0; 

cmd( "set f [open $RootLsd/$LsdSrc/system_options.txt r]" );
cmd( "set d [read -nonewline $f]" );
cmd( "close $f" );

cmd( "set f [open $RootLsd/$LsdSrc/makefile_base.txt r]" );
cmd( "set b [read -nonewline $f]" );
cmd( "close $f" );


cmd( "set c \"# Model compilation options\\n$a\\n\\n# System compilation option\\n$d\\nLSDROOT=$RootLsd\\n\\n# body of the makefile (in src/makefile_base.txt)\\n$b\"" );
cmd( "set f [open makefile w]" );
cmd( "puts -nonewline $f $c" );
cmd( "close $f" );
cmd( "update" );
}


void make_makefileNW(void)
{

cmd( "set choice [file exists model_options.txt]" );
if(choice==0)
 {//the model_options.txt file does not exists, probably an old version
   cmd( "set dir [glob *.cpp]" );
   cmd( "set b [lindex $dir 0]" );
   cmd( "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-O3\\nSWITCH_CC_LNK=\\n\"" );
   cmd( "set f [open model_options.txt w]" );
   cmd( "puts -nonewline $f $a" );
   cmd( "close $f" );
 }
choice=0; 
cmd( "set f [open model_options.txt r]" );
cmd( "set a [read -nonewline $f]" );
cmd( "close $f" );

cmd( "set choice [file exists $RootLsd/$LsdSrc/system_options.txt]" );
if(choice==0)
 { //the src/system_options.txt file doesn't exists, so I invent it
	cmd( "if [ string equal $tcl_platform(platform) windows ] { if [ string equal $tcl_platform(machine) intel ] { set sysfile \"sysopt_win32.txt\" } { set sysfile \"sysopt_win64.txt\" } } { if [ string equal $tcl_platform(os) Darwin ] { set sysfile \"sysopt_mac.txt\" } { set sysfile \"sysopt_linux.txt\" } }" );
    cmd( "set f [open $RootLsd/$LsdSrc/system_options.txt w]" );
    cmd( "set f1 [open $RootLsd/$LsdSrc/$sysfile r]" );
    cmd( "puts -nonewline $f \"LSDROOT=$RootLsd\\n\"" );
    cmd( "puts -nonewline $f [read $f1]" );
    cmd( "close $f" );
    cmd( "close $f1" );    
 }
choice=0; 

cmd( "set f [open $RootLsd/$LsdSrc/system_options.txt r]" );
cmd( "set d [read -nonewline $f]" );
cmd( "close $f" );

cmd( "if { [ string equal $tcl_platform(platform) windows ] && [ string equal $tcl_platform(machine) intel ] } { set fnameNW $RootLsd/$LsdSrc/makefile_baseNW32.txt } { set fnameNW $RootLsd/$LsdSrc/makefile_baseNW.txt }" );
cmd( "set f [open $fnameNW r]" );
cmd( "set b [read -nonewline $f]" );
cmd( "close $f" );

cmd( "set c \"# Model compilation options\\n$a\\n\\n# System compilation option\\n$d\\n\\n# body of the makefile (in src/makefile_base.txt)\\n$b\"" );
cmd( "set f [open makefileNW w]" );
cmd( "puts -nonewline $f $c" );
cmd( "close $f" );
cmd( "update" );
}


void create_compresult_window(void)
{

delete_compresult_window();

cmd( "set cerr 0.0" );

cmd( "newtop .mm \"Compilation Errors\" { destroytop .mm } \"\"" );

cmd( "label .mm.lab -justify left -text \"- Each error is indicated by the file name and line number where it has been identified.\n- Check the relative file and search on the indicated line number, considering that the error may have occurred in the previous line.\n- Fix first errors at the beginning of the list, since the following errors may be due to previous ones.\n- Check the 'Readme.txt' in Lsd installation directory for information on particular problems.\"" );
cmd( "pack .mm.lab" );

cmd( "text .mm.t -yscrollcommand \".mm.yscroll set\" -wrap word; scrollbar .mm.yscroll -command \".mm.t yview\"" );

cmd( "pack .mm.yscroll -side right -fill y; pack .mm.t -expand yes -fill both" );

cmd( "frame .mm.b" );

cmd( "set error \"error\"" );				// error string to be searched
cmd( "button .mm.b.ferr -width -9 -text \"Next Error\" -command {set errtemp [.mm.t search -nocase -regexp -count errlen -- $error $cerr end]; if { [string length $errtemp] == 0} {} { set cerr \"$errtemp + $errlen ch\"; .mm.t mark set insert $cerr; .mm.t tag remove sel 1.0 end; .mm.t tag add sel \"$errtemp linestart\" \"$errtemp lineend\"; .mm.t see $errtemp;} }" );

cmd( "button .mm.b.perr -width -9 -text \"Previous Error\" -command {set errtemp [ .mm.t search -nocase -regexp -count errlen -backward -- $error $cerr 0.0];  if { [string length $errtemp] == 0} {} { set cerr \"$errtemp - 1ch\"; .mm.t mark set insert $errtemp; .mm.t tag remove sel 1.0 end; .mm.t tag add sel \"$errtemp linestart\" \"$errtemp lineend\"; .mm.t see $errtemp} }" );

cmd( "pack .mm.b.perr .mm.b.ferr -padx 10 -pady 5 -expand yes -fill x -side left" );
cmd( "pack .mm.b -expand yes -fill x" );
cmd( "button .mm.close -width -9 -text Done -command {destroytop .mm}" );
cmd( "pack .mm.close -padx 10 -pady 10 -side right" );
cmd( "bind .mm <Escape> {destroytop .mm}" );

cmd( "showtop .mm topleftS no no no" );
cmd( "focus .mm.t" );

cmd( "set file [open $modeldir/makemessage.txt]" );
cmd( ".mm.t insert end [read $file]" );
cmd( "close $file" );
cmd( ".mm.t mark set insert \"1.0\";" );
cmd( "set errtemp [.mm.t search -nocase -regexp -count errlen -- $error $cerr end]; if { [string length $errtemp] == 0} {} { set cerr \"$errtemp + $errlen ch\"; .mm.t mark set insert $cerr; .mm.t tag remove sel 1.0 end; .mm.t tag add sel \"$errtemp linestart\" \"$errtemp lineend\"; .mm.t see $errtemp;}" );
}

// delete any previously open compilation results window
void delete_compresult_window( void )
{
	cmd( "set a [winfo exists .mm]" );
	cmd( "if { $a == 1 } { destroytop .mm } { }" );
}


/*********************************
 HANDLE_SIGNALS
 *********************************/
// provide support to the old 32-bit gcc compiler
#ifdef SIGSYS
#define NUM_SIG 11
int signals[ NUM_SIG ] = { SIGINT, SIGQUIT, SIGTERM, SIGWINCH, SIGABRT, SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGSYS, SIGXFSZ };
#else
#define NUM_SIG 6
int signals[ NUM_SIG ] = { SIGINT, SIGTERM, SIGABRT, SIGFPE, SIGILL, SIGSEGV };
const char *signal_names[ NUM_SIG ] = { "", "", "", "Floating-point exception", "Illegal instruction", "Segmentation violation", };
const char *strsignal( int signum ) 
{ 
	int i;
	for ( i = 0; i < NUM_SIG && signals[ i ] != signum; ++i );
	if ( i == NUM_SIG )
		return "Unknow exception";
	return signal_names[ i ];
}
#endif

void handle_signals( void )
{
	for ( int i = 0; i < NUM_SIG; ++i )
		signal( signals[ i ], signal_handler );  
}


/*********************************
 SIGNAL_HANDLER
 *********************************/
// handle critical system signals
void signal_handler( int signum )
{
	char msg2[ MAX_LINE_SIZE ];

	switch( signum )
	{
#ifdef SIGQUIT
		case SIGQUIT:
#endif
		case SIGINT:
		case SIGTERM:
			choice = 1;				// regular quit (checking for save)
			return;
#ifdef SIGWINCH
		case SIGWINCH:
			cmd( "sizetop all" );	// readjust windows size/positions
			cmd( "update idletasks" );
			return;
#endif
		case SIGMEM:
			sprintf( msg, "Out of memory" );
			break;
			
		case SIGSTL:
			break;
			
		case SIGABRT:
		case SIGFPE:
		case SIGILL:
		case SIGSEGV:
#ifdef SIGBUS
		case SIGBUS:
		case SIGSYS:
		case SIGXFSZ:
#endif
		default:
			strcpy( msg, strsignal( signum ) );
			break;			
	}
	
	cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s\n\nLMM will close now.\"", msg );
	sprintf( msg2, "System Signal received: %s", msg );
	log_tcl_error( "FATAL ERROR", msg2 );
	Tcl_Exit( -signum );			// abort program
}
