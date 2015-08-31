/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

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
#include <sys/stat.h>


#define DUAL_MONITOR true		// define this variable to better handle dual-monitor setups


Tcl_Interp *inter;
char msg[1024];		// old value (300) was too small (Tcl/Tk "invading" next vars) 
int choice;
int v_counter=0; //counter of the v[i] variables inserted in the equations
int shigh;			// syntax highlighting state (0, 1 or 2)

void copy(char *f1, char *f2);
Tcl_Interp *InterpInitWin(void);
void cmd(Tcl_Interp *inter, char *cc);
void cmd(Tcl_Interp *inter, const char cc[]) {cmd(inter, (char *)cc);};
int errormsg( char *lpszText,  char *lpszTitle);
int ModManMain(int argn, char **argv);
void color(int hiLev, long iniLin, long finLin);
void make_makefile(void);
void make_makefileNW(void);
char app_str[2][200];
void signal(char *s);
void create_compresult_window(void);
void delete_compresult_window( void );

#ifdef DUAL_MONITOR
// Main window constraints
char hsize[]="800";			// horizontal size in pixels
char vsize[]="600";			// vertical minimum size in pixels
char hmargin[]="20";		// horizontal right margin from the screen borders
char vmargin[]="20";		// vertical margins from the screen borders
char bordsize[]="4";		// width of windows borders
char tbarsize[]="80";		// size in pixels of bottom taskbar (exclusion area)
							// Windows 7+ = 82
#endif
								
int main(int argn, char **argv)
{
/*
Current attempt to get rid of the necessity to set the PATH variable under Windows
I will keep on working on this.

char *e, *loc, newpath[500], here[200];
int i;
e=getenv("PATH\0");
if(e!=(char *)NULL)
 {
  loc=strstr(e, "gnu/bin");
  if(loc==(char *)NULL)
   {strcpy(here, argv[0]);
    loc=strstr(here, "lmm");
    for(i=0; strcmp(here+i,loc); i++);
    here[i-1]='\0';
    strcat(here, "/gnu/bin");  // remember to check for _WIN64
    sprintf(newpath, "PATH=%s;%s",e,here);
    getenv(newpath);

   }
 }
*/

#ifndef TK_LOCAL_APPINIT
#define TK_LOCAL_APPINIT Tcl_AppInit    
#endif
    extern int TK_LOCAL_APPINIT _ANSI_ARGS_((Tcl_Interp *interp));
    
    /*
     * The following #if block allows you to change how Tcl finds the startup
     * script, prime the library or encoding paths, fiddle with the argv,
     * etc., without needing to rewrite Tk_Main()
     */
    
#ifdef TK_LOCAL_MAIN_HOOK
    extern int TK_LOCAL_MAIN_HOOK _ANSI_ARGS_((int *argc, char ***argv));
    TK_LOCAL_MAIN_HOOK(&argc, &argv);
#endif

//    Tk_Main(argc, argv, TK_LOCAL_APPINIT);
Tcl_FindExecutable(argv[0]);
ModManMain(argn, argv);
Tcl_Exit(0);
return 0;
}

/*********************************
INTERPINITWIN
Calls tclinit and tkinit, managing he errors
WARNING !!!
This function presumes the installation of a /gnu (or gnu64 for Win64) 
directory along the model's one. Tcl and Tk initialization files MUST be in
/gnu[64]/share/tcl8.X
/gnu[64]/share/tk8.X
*********************************/
Tcl_Interp *InterpInitWin(void)
{
Tcl_Interp *app;
char *s;
int res;
app=Tcl_CreateInterp();

//cmd(app, "puts $tcl_library");
//cmd(app, "source init.tcl");
//cmd(app, "source C:/Lsd5.5/gnu/lib/tk8.4/tk.tcl");  // remember to check for _WIN64

//Tcl_SetVar(app, "tcl_library", "C:/Lsd5.5/gnu/lib/tcl8.4", TCL_APPEND_VALUE);
/*
cmd(app, "puts $tcl_library");
return app;


    cmd(app, "set tk_library {gnu/lib/tk8.4}");
*/    
//cmd(app, "lappend tcl_libPath {gnu/lib/tcl8.4}");

if((res=Tcl_Init(app))!=TCL_OK)
 {
  char estring[255];
  sprintf(estring,"Tcl Error = %d : %s\n",res,app->result);
  errormsg(estring,NULL);
  exit(1);
 
 }

//cmd(app, "set env(DISPLAY) :0.0");
if((res=Tk_Init(app))!=TCL_OK)
 {
  errormsg( (char *)"Tk Initialization failed. Check directory structure:\nLSDHOME\\gnu[64]\\lib\\tk8.5\n", NULL);
  char estring[255];
  sprintf(estring,"Tk Error = %d : %s\n",res,app->result);
  errormsg(estring,NULL);
  exit(1);
 }

//cmd(app, "tk_messageBox -type ok -message \"[lindex [array get env PATH] 1]\"");
return app;
}

/****************************************************
CMD
****************************************************/
void cmd(Tcl_Interp *inter, char *cm)
{

int code;
FILE *ferr, *f;
/*
Tcl_Obj *o;
o=Tcl_NewStringObj(cm, strlen(cm));
code=Tcl_EvalObjEx(inter, o, TCL_EVAL_DIRECT);
if(code!=TCL_OK)
 {f=fopen("tk_err.err","a");
  sprintf(msg, "\n%s\n\n%s\n",cm, Tcl_GetStringResult(inter));
  fprintf(f,"%s", msg);
  fclose(f);
  //plog("\nTcl-Tk Error. See file tk_err.err\n");
 }


return;
*/

code=Tcl_Eval(inter, cm);

if(code!=TCL_OK && !strstr(cm,(char*)"exec a.bat")) // don't log model compilation errors
 {
  ferr=fopen("tk_err.err","a");
  sprintf(msg, "\nCommand:%s\nProduced message:\n%s\n-----\n",cm, inter->result);
  fprintf(ferr,"%s", msg);
  fclose(ferr);
//  sprintf(msg, "tk_messageBox -type ok -title \"Lmm error\" -message \"Error in a tk function\"");
//  cmd(inter, msg);
 }

}
int errormsg( char *lpszText,  char *lpszTitle)
{

printf("\n%s", (char *)lpszText);
exit(1);
}


void copy(char *f1, char *f2)
{
char m[600];

sprintf(m,"copy %s %s", f1, f2);
system(m);

}


/*************************************
ModManMain
/*************************************/
int ModManMain(int argn, char **argv)
{
int i, num, tosave, sourcefile, macro;
char str[500], str1[500], str2[500];
char *s;
FILE *f;

//Initialize the tcl interpreter,
inter=InterpInitWin();



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
  sprintf(str, "set filetoload %s",msg);
  cmd(inter, str);
  cmd(inter, "if {[file pathtype \"$filetoload\"]==\"absolute\"} {} {set filetoload \"[pwd]/$filetoload\"}");
 }

//cmd(inter, "set tk_strictMotif 1");

sprintf(msg, "%s",*argv);
if(!strncmp(msg, "//", 2))
 sprintf(str, "%s",msg+3);
else
 sprintf(str, "%s",msg);

Tcl_LinkVar(inter, "choice", (char *) &choice, TCL_LINK_INT);

sprintf(msg, "if {[file exists [file dirname \"[file nativename %s]\"]]==1} {cd [file dirname \"%s\"]; set choice 1} {cd [pwd]; set choice 0}",str, str);
cmd(inter, msg);
cmd(inter, "set RootLsd [pwd]");
s=(char *)Tcl_GetVar(inter, "RootLsd",0);
strcpy(msg, s);

cmd(inter, "lappend ml LSDROOT");
cmd(inter, "lappend ml $RootLsd");
cmd(inter, "array set env $ml");
cmd(inter, "set groupdir [pwd]");
cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {set DefaultWish wish; set DefaultTerminal xterm; set DefaultHtmlBrowser firefox; set DefaultFont Courier} {}");
cmd(inter, "if {$tcl_platform(os) == \"Windows NT\"} {set DefaultWish wish85.exe; set DefaultTerminal cmd; set DefaultHtmlBrowser open; set DefaultFont \"Courier New\"} {}");
cmd(inter, "if {$tcl_platform(os) == \"Darwin\"} {set DefaultWish wish8.5; set DefaultTerminal terminal; set DefaultHtmlBrowser open; set DefaultFont Courier} {}");

cmd(inter, "set Terminal $DefaultTerminal");
cmd(inter, "set HtmlBrowser $DefaultHtmlBrowser");
cmd(inter, "set fonttype $DefaultFont");
cmd(inter, "set wish $DefaultWish");
cmd(inter, "set LsdSrc src");

// Handles Windows 32 and 64-bit versions (at run time)
if((size_t)-1 > 0xffffffffUL)  // test for 64-bit address space
 cmd(inter, "if {$tcl_platform(os) == \"Windows NT\"} {set LsdGnu gnu64} {set LsdGnu gnu}");
else
 cmd(inter, "set LsdGnu gnu");
	
cmd(inter, "set choice [file exist $RootLsd/lmm_options.txt]");
if(choice==1)
 {
  cmd(inter, "set f [open $RootLsd/lmm_options.txt r]");
  cmd(inter, "gets $f Terminal");
  cmd(inter, "gets $f HtmlBrowser");
  cmd(inter, "gets $f fonttype");
  cmd(inter, "gets $f wish");
  cmd(inter, "gets $f LsdSrc");
	cmd( inter, "gets $f dim_character" );
	cmd( inter, "gets $f tabsize" );
	cmd( inter, "gets $f wrap" );
	cmd( inter, "gets $f shigh" );
	cmd( inter, "gets $f autoHide" );
  cmd(inter, "close $f" );
	// handle old options file
	cmd( inter, "if {$dim_character == \"\"} {set choice 0}" );
 }
// handle non-existent or old options file for new options
if ( choice != 1 )
 {
	cmd( inter, "set dim_character 0" );	// default font size (0=force auto-size)
	cmd( inter, "set tabsize 2" );		// default tab size
	cmd( inter, "set wrap 1" );			// default text wrapping mode (1=yes)
	cmd( inter, "set shigh 2" );			// default is full syntax highlighting
	cmd( inter, "set autoHide 1" );		// default is to auto hide LMM on run
  cmd(inter, "set f [open $RootLsd/lmm_options.txt w]");
  cmd(inter, "puts $f $Terminal");
  cmd(inter, "puts $f $HtmlBrowser");
  cmd(inter, "puts $f $fonttype");
  cmd(inter, "puts $f $wish");  
  cmd(inter, "puts $f $LsdSrc");
	cmd( inter, "puts $f $dim_character" );
	cmd( inter, "puts $f $tabsize" );
	cmd( inter, "puts $f $wrap" );
	cmd( inter, "puts $f $shigh" );
	cmd( inter, "puts $f $autoHide" );
  cmd(inter, "close $f");
 }
 

//cmd(inter, "tk_messageBox -type ok -message \"$env(LSDROOT)\\n[pwd]\"");
strcpy(msg, s);

cmd(inter, "if { [string first \" \" \"[pwd]\" ] >= 0  } {set choice 1} {set choice 0}");
if(choice==1)
 {
 cmd(inter, "wm iconify .");
 cmd(inter, "tk_messageBox -icon error -title \"Installation Error\" -type ok -message \"The Lsd directory is: '[pwd]'\n\nIt includes spaces, which makes impossible to compile and run Lsd model.\n\nThe Lsd directory must be located where there are no spaces in the full path name.\n\nMove all the Lsd directory in another directory.\n\nIf exists, delete the 'system_options.txt' file from the \\src directory. \"");
 exit(1);
 
 }

cmd(inter, "if { $tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
if(choice==1)
 {
  for(i=0; msg[i]!=(char)NULL; i++)
   {
   if(msg[i]=='/')
    msg[i]='\\';
   }
 }
//i=setenv("LSDROOT",msg,1);


cmd(inter, "if { [file exists $RootLsd/$LsdSrc/system_options.txt] == 1} {set choice 0} {set choice 1}");
if(choice==1)
 {cmd(inter, "if { $tcl_platform(platform) == \"windows\"} {set sysfile \"sysopt_windows.txt\"} { if { $tcl_platform(os) == \"Darwin\"} {set sysfile \"sysopt_mac.txt\"} {set sysfile \"sysopt_linux.txt\"}}");
    cmd(inter, "set f [open $RootLsd/$LsdSrc/system_options.txt w]");
    cmd(inter, "set f1 [open $RootLsd/$LsdSrc/$sysfile r]");
    //cmd(inter, "puts -nonewline $f \"LSDROOT=$RootLsd\\n\"");
    cmd(inter, "puts -nonewline $f [read $f1]");
    cmd(inter, "close $f");
    cmd(inter, "close $f1");    
 }
else
 {
  //control the Lsdroot
 /*REMOVED, DYNAMIC SETTING OF LSD ROOT INCLUDED IN THE MAKE_MAKEFILE
  cmd(inter, "set f [open $RootLsd/$LsdSrc/system_options.txt r]");
  cmd(inter, "set i -1");
  cmd(inter, "while {$i<0} { gets $f a; if { [eof $f] == 1} {set i 999} {set i [string first \"LSDROOT=\" $a] } }");
  cmd(inter, "if { $i == 999} {set choice 0 }  {set choice 1} ");
  if(choice==0)
   { cmd(inter, "tk_messageBox -icon error -title \"Installation Problem\" -type ok -message \"The system compilation options appear not compatible with the installation.\nUse 's' menu entry in menu 'Run' to fix the error, or the model compilation will cause errors.\"");
    choice=47;
   }
  else
   {
    cmd(inter, "set j [expr $i + 8]; set b [string range $a $j end]; set c [string tolower $b]");
    cmd(inter, "if { [string compare $c [string tolower [pwd]]] == 0} {set choice 1 } {set choice 0}");
    if(choice==0)
   { cmd(inter, "tk_messageBox -icon error -title \"Installation Problem\" -type ok -message \"The system compilation options appear not compatible with the installation:\n- current directory is '[pwd]'\n- recorded directory is '$b'\nUse 'System Compilation Options' menu entry in menu 'Run' to fix the error, or the model compilation will cause errors.\"");
    choice=47;
   }
     
   } 
 */
 } 

cmd(inter, "proc LsdHelp a {global HtmlBrowser; global tcl_platform; global RootLsd; set here [pwd]; cd $RootLsd; cd Manual; set f [open temp.html w]; puts $f \"<meta http-equiv=\\\"Refresh\\\" content=\\\"0;url=$a\\\">\"; close $f; set b \"temp.html\"; if {$tcl_platform(platform) == \"unix\"} {catch [exec $HtmlBrowser $b &]} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\" || $tcl_platform(osVersion) == \"5.2\" } {exec cmd.exe /c start $b &} { if {$tcl_platform(osVersion) == \"5.2\"} {cmd.exe /c $b &} {catch [exec openhelp.bat &]} }} {exec start $b &}}; cd $here }");

 
cmd(inter, "proc lmmraise {win ent} {wm focusmodel . active; if { $ent!=\"\"} {focus -force $ent} {focus -force $win}; wm focusmodel . passive}");

cmd(inter, "proc blocklmm w { wm transient $w [ winfo parent $w ]; raise $w; focus -force [focus -lastfor $w]; grab set $w }");
cmd(inter, "proc sblocklmm w { grab release $w; destroy $w; focus -force .f.t.t }");

#ifdef DUAL_MONITOR
/* procedures to adjust window positioning (settings for dual and single monitor setups). Three types for positioning:
	centerS: center over the primary display, only available if the parent window center is also in the primary display (if not, falback to centerW)
	centerW: center over the parent window (in any display)
	topleft: put over the top left corner of parent window (below menu bar)
*/
// check if window center is in primary display
cmd( inter, "proc primdisp w { if { [ winfo rootx $w ] > 0 && [ winfo rootx $w ] < [ winfo screenwidth $w ] && [ winfo rooty $w ] > 0 && [ winfo rooty $w ] < [ winfo screenheight $w ] } { return true } { return false } } " );
// compute x and y coordinates of new window according to the types
cmd( inter, "proc getx { w type } { switch $type { centerS { return [ expr [ winfo screenwidth $w ] / 2 - [ winfo reqwidth $w ] / 2 ] } centerW { return [ expr [ winfo rootx [ winfo parent $w ] ] + [ winfo width [ winfo parent $w ] ] / 2  - [ winfo reqwidth $w ] / 2 ] } topleft { return [ expr [ winfo x [ winfo parent $w ] ] + 5 ] } } }" );
cmd( inter, "proc gety { w type } { switch $type { centerS { return [ expr [ winfo screenheight $w ] / 2 - [ winfo reqheight $w ] / 2 ] } centerW { return [ expr [ winfo rooty [ winfo parent $w ] ] + [ winfo height [ winfo parent $w ] ] / 2  - [ winfo reqheight $w ] / 2 ] } topleft { return [ expr [ winfo y [ winfo parent $w ] ] + 50 ] } } }" );
// configure the window
cmd( inter, "proc setgeom { w { type centerW } } { wm withdraw $w; update idletasks; if { [ string equal $type centerS ] && ! [ primdisp [ winfo parent $w ] ] } { set type centerW }; set x [ getx $w $type ]; set y [ gety $w $type ]; wm geom $w +$x+$y; update; wm deiconify $w; wm transient $w [ winfo parent $w ] }" );
#else
// old centering procedure (doesn't work well for dual monitor), probably obsolete
cmd( inter, "proc setgeom { w { type window_center } } { wm withdraw $w; update idletasks; set x [ expr [ winfo screenwidth $w ] / 2 - [ winfo reqwidth $w ] / 2 - [ winfo vrootx [ winfo parent $w ] ] ]; set y [ expr [ winfo screenheight $w ] / 2 - [ winfo reqheight $w ] / 2 - [ winfo vrooty [ winfo parent $w ] ] ]; wm geom $w +$x+$y; update; wm deiconify $w }");
#endif

// procedures to adjust tab size according to font type and size and text wrapping
cmd( inter, "proc settab {w size font} { set tabwidth \"[ expr { $size * [ font measure \"$font\" 0 ] } ] left\"; $w conf -font \"$font\" -tabs $tabwidth -tabstyle wordprocessor }" );
cmd( inter, "proc setwrap {w wrap} { if { $wrap == 1 } { $w conf -wrap word } { $w conf -wrap none } }" );

cmd(inter, "wm title . \"LMM - Lsd Model Manager\"");
cmd(inter, "wm protocol . WM_DELETE_WINDOW { set choice 1 }");
cmd(inter, "bind . <Destroy> {set choice -1}");
cmd(inter, "set choice 0");
cmd(inter, "set recolor \"\"");
cmd(inter, "set docase 0");
cmd(inter, "set dirsearch \"-forwards\"");
cmd(inter, "set endsearch end");
cmd(inter, "set currentpos \"\"");
cmd(inter, "set currentdoc \"\"");
cmd(inter, "set v_num 0");
cmd(inter, "set macro 1");
cmd(inter, "set shigh_temp $shigh");
cmd(inter, "source $RootLsd/$LsdSrc/showmodel.tcl");
cmd(inter, "source $RootLsd/$LsdSrc/lst_mdl.tcl");
Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
Tcl_LinkVar(inter, "tosave", (char *) &tosave, TCL_LINK_INT);
Tcl_LinkVar(inter, "macro", (char *) &macro, TCL_LINK_INT);
macro=1;    
Tcl_LinkVar(inter, "shigh", (char *) &shigh, TCL_LINK_INT);
cmd(inter, "set shigh $shigh_temp");	// restore correct value


cmd(inter, "menu .m -tearoff 0");

cmd(inter, "set w .m.file");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label File -menu $w -underline 0");
cmd(inter, "$w add command -label \"New Text File\" -command { set choice 39} -underline 0");
cmd(inter, "$w add command -label \"Open...\" -command { set choice 15} -underline 0 -accelerator Ctrl+o");
cmd(inter, "$w add command -label \"Save\" -command { if {[string length $filename] > 0} {if { [file exist $dirname/$filename] == 1} {catch {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak}} {}; set f [open $dirname/$filename w];puts -nonewline $f [.f.t.t get 0.0 end]; close $f; set before [.f.t.t get 0.0 end]} {}} -underline 0 -accelerator Ctrl+s");
cmd(inter, "$w add command -label \"Save As...\" -command {set choice 4} -underline 5 -accelerator Ctrl+a");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"TkDiff...\" -command {set choice 57} -underline 0");

cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Options...\" -command { set choice 60} -underline 1");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Quit\" -command {; set choice 1} -underline 0 -accelerator Ctrl+q");

cmd(inter, "set w .m.edit");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Edit -menu $w -underline 0");
// make menu the same as ctrl-z/y (more color friendly)
cmd(inter, "$w add command -label \"Undo\" -command {catch {.f.t.t edit undo}} -underline 0 -accelerator Ctrl+z");
cmd(inter, "$w add command -label \"Redo\" -command {catch {.f.t.t edit redo}} -underline 2 -accelerator Ctrl+y");
cmd(inter, "$w add separator");
// collect information to focus recoloring
cmd(inter, "$w add command -label \"Cut\" -command {savCurIni; tk_textCut .f.t.t; if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd} -underline 1 -accelerator Ctrl+x");
cmd(inter, "$w add command -label \"Copy\" -command {tk_textCopy .f.t.t} -underline 0 -accelerator Ctrl+c");
cmd(inter, "$w add command -label \"Paste\" -command {savCurIni; tk_textPaste .f.t.t; if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd} -underline 0 -accelerator Ctrl+v");

//cmd(inter, "$w add command -label \"Cut\" -command {tk_textCut .f.t.t} -underline 1");
//cmd(inter, "$w add command -label \"Paste\" -command {tk_textPaste .f.t.t} -underline 0 -accelerator Ctrl+v");
//cmd(inter, "$w add command -label \"Undo\" -command {if {[llength $ud] ==0 } {} {lappend rd [.f.t.t get 0.0 \"end - 1 chars\"]; lappend rdi [.f.t.t index insert]; .f.t.t delete 0.0 end; .f.t.t insert 0.0 [lindex $ud end]; .f.t.t delete end; .f.t.t see [lindex $udi end]; .f.t.t mark set insert [lindex $udi end]; set ud [lreplace $ud end end]; set udi [lreplace $udi end end]; set choice 23}} -underline 0 -accelerator Ctrl+z");
//cmd(inter, "$w add command -label \"Redo\" -command {if {[llength $rd] ==0} {} {lappend ud [.f.t.t get 0.0 \"end -1 chars\"]; lappend udi [.f.t.t index insert]; .f.t.t delete 0.0 end; .f.t.t insert 0.0 [lindex $rd end]; .f.t.t delete end; .f.t.t see [lindex $rdi end]; .f.t.t mark set insert [lindex $rdi end]; set rd [lreplace $rd end end]; set rdi [lreplace $rdi end end]; set choice 23} } -underline 2 -accelerator Ctrl+y");

cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Find...\" -command {set choice 11} -underline 0 -accelerator Ctrl+f");
cmd(inter, "$w add command -label \"Find Again\" -command {set choice 12} -underline 5 -accelerator F3");
cmd(inter, "$w add command -label \"Replace...\" -command {set choice 21} -underline 0");
cmd(inter, "$w add command -label \"Goto Line...\" -command {set choice 10} -underline 5 -accelerator Ctrl+l");
cmd(inter, "$w add separator");

cmd(inter, "$w add command -label \"Match \\\{ \\}\" -command {set choice 17} -accelerator Ctrl+m");
cmd(inter, "$w add command -label \"Match \\\( \\)\" -command {set choice 32} -accelerator Ctrl+u");
cmd(inter, "$w add command -label \"Insert \\\{\" -command {.f.t.t insert insert \\\{} -accelerator Ctrl+\\\(");
cmd(inter, "$w add command -label \"Insert \\}\" -command {.f.t.t insert insert \\}} -accelerator Ctrl+\\)");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Indent Selection\" -command {set choice 42} -accelerator Ctrl+>");
cmd(inter, "$w add command -label \"De-indent Selection\" -command {set choice 43} -accelerator Ctrl+<");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Larger Font\" -command {incr dim_character 1; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"; settab .f.t.t $tabsize \"$a\"} -accelerator Ctrl+'+'");
cmd(inter, "$w add command -label \"Smaller Font\" -command {incr dim_character -1; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"; settab .f.t.t $tabsize \"$a\"} -accelerator Ctrl+'-'");
cmd(inter, "$w add command -label \"Change Font...\" -command {set choice 59} -underline 8");
cmd(inter, "$w add separator");
// add option to ajust syntax highlighting (word coloring)
cmd(inter, "$w add cascade -label \"Syntax Highlighting\" -menu $w.color -underline 0");
cmd(inter, "$w add check -label \"Wrap/Unwrap Text\" -variable wrap -command {setwrap .f.t.t $wrap} -underline 1 -accelerator Ctrl+w ");
cmd(inter, "$w add command -label \"Change Tab Size...\" -command {set choice 67} -underline 7");
cmd(inter, "$w add command -label \"Insert Lsd Scripts...\" -command {set choice 28} -underline 0 -accelerator Ctrl+i");

cmd(inter, "menu $w.color -tearoff 0");
cmd(inter, "$w.color add radio -label \" Full\" -variable shigh -value 2 -command {set choice 64} -underline 1 -accelerator \"Ctrl+;\"");
cmd(inter, "$w.color add radio -label \" Partial\" -variable shigh -value 1 -command {set choice 65} -underline 1 -accelerator Ctrl+,");
cmd(inter, "$w.color add radio -label \" None\" -variable shigh -value 0 -command {set choice 66} -underline 1 -accelerator Ctrl+.");

cmd(inter, "set w .m.model");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Model -menu $w -underline 0");
//cmd(inter, "$w add command -label \"New Model\" -underline 0 -command { set choice 14}");
//cmd(inter, "$w add command -label \"Copy Model\" -state disabled -underline 0 -command { set choice 41}");
cmd(inter, "$w add command -label \"Browse Models...\" -underline 0 -command {set choice 33} -accelerator Ctrl+b");
cmd(inter, "$w add command -label \"Compare Models...\" -underline 3 -command {set choice 61}");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Compile and Run Model...\" -state disabled -underline 0 -command {set choice 2} -accelerator Ctrl+r");
cmd(inter, "$w add command -label \"Compile Model\" -state disabled -underline 2 -command {set choice 6} -accelerator Ctrl+p");
cmd(inter, "$w add command -label \"GDB Debugger\" -state disabled -underline 0 -command {set choice 13} -accelerator Ctrl+g");
cmd(inter, "$w add command -label \"Create 'No Window' Version\" -underline 8 -state disabled -command {set choice 62}");
cmd(inter, "$w add command -label \"Model Info...\" -underline 6 -state disabled -command {set choice 44}");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Show Description\" -underline 5 -state disabled -command {set choice 5} -accelerator Ctrl+d");
cmd(inter, "$w add command -label \"Show Equations\" -state disabled -underline 5 -command {set choice 8} -accelerator Ctrl+e");
cmd(inter, "$w add command -label \"Show Makefile\" -state disabled -underline 7 -command { set choice 3}");
cmd(inter, "$w add command -label \"Show Compilation Results\" -underline 6 -state disabled -command {set choice 7}");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Model Compilation Options...\" -underline 2 -state disabled -command {set choice 48}");
cmd(inter, "$w add command -label \"System Compilation Options...\" -underline 0 -command {set choice 47}");
cmd(inter, "$w add separator");
cmd(inter, "$w add check -label \"Auto Hide LMM on Run\" -variable autoHide -underline 0");
cmd(inter, "$w add cascade -label \"Equations' Coding Style\" -underline 1 -menu $w.macro");

cmd(inter, "menu $w.macro -tearoff 0");
cmd(inter, "$w.macro add radio -label \" Use Lsd Macros\" -variable macro -value 1 -command {.m.help entryconf 1 -label \"Help on Macros for Lsd Equations\" -underline 6 -command {LsdHelp lsdfuncMacro.html}; set choice 68}");
cmd(inter, "$w.macro add radio -label \" Use Lsd C++\" -variable macro -value 0 -command {.m.help entryconf 1 -label \"Help on C++ for Lsd Equations\" -underline 8 -command {LsdHelp lsdfunc.html}; set choice 69}");


cmd(inter, "set w .m.help");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Help -menu $w -underline 0");
cmd(inter, "$w add command -label \"Help on LMM\" -underline 4 -command {LsdHelp \"LMM_help.html\"}");
if(macro==1)
  cmd(inter, "$w add command -label \"Help on Macros for Lsd Equations\" -underline 6 -command {LsdHelp lsdfuncMacro.html}");
else
  cmd(inter, "$w add command -label \"Help on C++ for Lsd Equations\" -underline 8 -command {LsdHelp lsdfunc.html}"); 

cmd(inter, "$w add separator");
//cmd(inter, "$w add command -label \"Tutorial 1 - LMM First users\" -underline 6 -command {LsdHelp Tutorial1.html}");
//cmd(inter, "$w add command -label \"Tutorial 2 - Using Lsd Models\" -underline 0 -command {LsdHelp ModelUsing.html}");
//cmd(inter, "$w add command -label \"Tutorial 3 - Writing Lsd Models\" -underline 6 -command {LsdHelp ModelWriting.html}");
cmd(inter, "$w add command -label \"Lsd Documentation\" -command {LsdHelp Lsd_Documentation.html}");


cmd( inter, "$w add command -label \"About LMM...\" -command { tk_messageBox -type ok -icon info -title \"About LMM\" -message \"Version 7.0 \n\nAugust 2015\" } -underline 0" ); 

cmd(inter, "frame .f");
cmd(inter, "frame .f.t -relief groove -bd 2");
cmd(inter, "scrollbar .f.t.vs -command \".f.t.t yview\"");
cmd(inter, "scrollbar .f.t.hs -orient horiz -command \".f.t.t xview\"");
cmd(inter, "text .f.t.t -height 2 -undo 1 -bg #fefefe -yscroll \".f.t.vs set\" -xscroll \".f.t.hs set\"");
cmd(inter, "set a [.f.t.t conf -font]");
cmd(inter, "set b [lindex $a 3]");
cmd(inter, "if {$dim_character == 0} {set dim_character [lindex $b 1]}");
//cmd(inter, "set fonttype [lindex $b 0]");
cmd(inter, "if { $dim_character == \"\"} {set dim_character 12} {}");

cmd(inter, "set a [list $fonttype $dim_character]");
// set preferred tab size and wrap option
cmd( inter, "settab .f.t.t $tabsize \"$a\"" );	// adjust tabs size to font type/size
cmd( inter, "setwrap .f.t.t $wrap" );		// adjust text wrap
//cmd(inter, "set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"");
//
// set syntax colors
cmd(inter, ".f.t.t tag configure comment1 -foreground green4");
cmd(inter, ".f.t.t tag configure comment2 -foreground green4");
cmd(inter, ".f.t.t tag configure str -foreground blue4");
cmd(inter, ".f.t.t tag configure cprep -foreground SaddleBrown");
cmd(inter, ".f.t.t tag configure lsdvar -foreground red4");
cmd(inter, ".f.t.t tag configure lsdmacro -foreground DodgerBlue4");
cmd(inter, ".f.t.t tag configure ctype -foreground DarkViolet");
cmd(inter, ".f.t.t tag configure ckword -foreground purple4");
cmd(inter, ".f.t.t configure -selectbackground gray");
cmd(inter, ".f.t.t configure -selectforeground black");

//


cmd(inter, "frame .f.hea -relief groove -bd 2");

cmd(inter, "frame .f.hea.grp");
cmd(inter, "label .f.hea.grp.tit -text \"Group: \" -fg red");
cmd(inter, "label .f.hea.grp.dat -text \"$modelgroup\"");
cmd(inter, "pack .f.hea.grp.tit .f.hea.grp.dat -side left");

cmd(inter, "frame .f.hea.mod");
cmd(inter, "label .f.hea.mod.tit -text \"Model: \" -fg red");
cmd(inter, "label .f.hea.mod.dat -text \"(No model)\"");
cmd(inter, "pack .f.hea.mod.tit .f.hea.mod.dat -side left");

cmd(inter, "frame .f.hea.ver");
cmd(inter, "label .f.hea.ver.tit -text \"Version: \" -fg red");
cmd(inter, "label .f.hea.ver.dat -text \"\"");
cmd(inter, "pack .f.hea.ver.tit .f.hea.ver.dat -side left");

cmd(inter, "frame .f.hea.file");
cmd(inter, "label .f.hea.file.tit -text \"File: \" -fg red");
cmd(inter, "label .f.hea.file.dat -text \"(No file)\"");
cmd(inter, "pack .f.hea.file.tit .f.hea.file.dat -side left");

cmd(inter, "frame .f.hea.line");
cmd(inter, "label .f.hea.line.line -relief sunk -width 12 -text \"[.f.t.t index insert]\"");
cmd(inter, "pack .f.hea.line.line -anchor e -expand no");
cmd(inter, "pack .f.hea.grp .f.hea.mod .f.hea.ver .f.hea.file .f.hea.line -side left -expand yes -fill x");

cmd(inter, "bind .f.hea.line.line <Button-1> {set choice 10}");
cmd(inter, "bind .f.hea.line.line <Button-2> {set choice 10}");
cmd(inter, "bind .f.hea.line.line <Button-3> {set choice 10}");

cmd(inter, "pack .f.hea -fill x");
cmd(inter, "pack .f.t -expand yes -fill both");
cmd(inter, "pack .f.t.vs -side right -fill y");
cmd(inter, "pack .f.t.t -expand yes -fill both");
cmd(inter, "pack .f.t.hs -fill x");


cmd(inter, "set dir [glob *]");
cmd(inter, "set num [llength $dir]");


cmd(inter, "bind . <Control-n> {tk_menuSetFocus .m.file}");

// procedures to save cursor environment before and after changes in text window for syntax coloring
cmd(inter, "proc savCurIni {} {global curSelIni curPosIni; set curSelIni [.f.t.t tag nextrange sel 1.0]; set curPosIni [.f.t.t index insert]; .f.t.t edit modified false}");
cmd(inter, "proc savCurFin {} {global curSelFin curPosFin; set curSelFin [.f.t.t tag nextrange sel 1.0]; set curPosFin [.f.t.t index insert]; .f.t.t edit modified false}");
cmd(inter, "proc updCurWnd {} {.f.hea.line.line conf -text [.f.t.t index insert]}");

// redefine bindings to better support new syntax highlight routine
cmd(inter, "bind .f.t.t <KeyPress> {savCurIni}");
cmd(inter, "bind .f.t.t <KeyRelease> {if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd}");
cmd(inter, "bind .f.t.t <ButtonPress> {savCurIni}");
cmd(inter, "bind .f.t.t <ButtonRelease> {if {[.f.t.t edit modified]} {savCurFin; set choice 23}; updCurWnd}");

/*
cmd(inter, "bind .f.t.t <KeyRelease> {.f.hea.line.line conf -text [.f.t.t index insert]}");
cmd(inter, "bind .f.t.t <ButtonRelease> {.f.hea.line.line conf -text [.f.t.t index insert]}");



cmd(inter, "bind .f.t.t <KeyPress> {if { [string compare $currentdoc [.f.t.t get 1.0 end] ]!=0 } {set recolor [lindex [.f.t.t tag nextrange sel 1.0] 0]; set undopos $currentpos; set currentpos [.f.t.t index insert]; set wholedoc \"$currentdoc\"; set currentdoc [.f.t.t get 1.0 end]; } {} }");
cmd(inter, "bind .f.t.t <KeyRelease> {.f.hea.line.line conf -text [.f.t.t index insert]; if { $recolor!=\"\"} {set choice 23; set recolor \"\"} {}}");
cmd(inter, "bind .f.t.t <KeyPress-Return> {.f.t.t tag remove comment2 [.f.t.t index insert] \"[.f.t.t index insert]+1 char\"}");
cmd(inter, "bind .f.t.t <KeyRelease-Delete> {.f.hea.line.line conf -text [.f.t.t index insert]; set choice 23}");
cmd(inter, "bind .f.t.t <KeyRelease-BackSpace> {.f.hea.line.line conf -text [.f.t.t index insert]; set choice 23}");
cmd(inter, "bind .f.t.t <KeyRelease-slash> {.f.hea.line.line conf -text [.f.t.t index insert]; set choice 23}");
cmd(inter, "bind .f.t.t <KeyRelease-asterisk> {.f.hea.line.line conf -text [.f.t.t index insert]; set choice 23}");
cmd(inter, "bind .f.t.t <KeyRelease-quotedbl> {.f.hea.line.line conf -text [.f.t.t index insert]; set choice 23}");
cmd(inter, "bind .f.t.t <KeyRelease-backslash> {.f.hea.line.line conf -text [.f.t.t index insert]; set choice 23}");*/


cmd(inter, "bind .f.t.t <Control-l> {set choice 10}");
cmd(inter, "bind .f.t.t <Control-w> {if {$wrap == 0} {set wrap 1} {set wrap 0}; setwrap .f.t.t $wrap}");

cmd(inter, "bind .f.t.t <Control-f> {set choice 11}");
cmd(inter, "bind .f.t.t <F3> {set choice 12}");
cmd(inter, "bind .f.t.t <Control-s> { if {[string length $filename] > 0} {if { [file exist $dirname/$filename] == 1} {catch {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak}} {}; set f [open $dirname/$filename w];puts -nonewline $f [.f.t.t get 0.0 end]; close $f; set before [.f.t.t get 0.0 end]} {}}"); 
cmd(inter, "bind .f.t.t <Control-a> { set choice 4}");
cmd(inter, "bind .f.t.t <Control-r> {set choice 2}");
cmd(inter, "bind .f.t.t <Control-e> {set choice 8}");
cmd(inter, "bind .f.t.t <Control-KeyRelease-o> {if {$tk_strictMotif == 0} {set a [.f.t.t index insert]; .f.t.t delete \"$a lineend\"} {}; set choice 15; break}");
cmd(inter, "bind .f.t.t <Control-q> {set choice 1}");
cmd(inter, "bind .f.t.t <Control-p> {set choice 6; break}");
cmd(inter, "bind .f.t.t <Control-u> {set choice 32}");
cmd(inter, "bind .f.t.t <Control-m> {set choice 17}");
cmd(inter, "bind .f.t.t <Control-g> {set choice 13}");
cmd(inter, "bind .f.t.t <Control-d> {set choice 5; break}");
cmd(inter, "bind .f.t.t <Control-b> {set choice 33; break}");



//cmd(inter, "bind .f.t.t <Control-c> {tk_textCopy .f.t.t}");
//cmd(inter, "bind .f.t.t <Control-v> {tk_textPaste .f.t.t}");

cmd(inter, "bind .f.t.t <Control-minus> {incr dim_character -2; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"}");

cmd(inter, "bind .f.t.t <Control-plus> {incr dim_character 2; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"}");

cmd(inter, "bind .f.t.t <Control-parenleft> {.f.t.t insert insert \\\{}");
cmd(inter, "bind .f.t.t <Control-parenright> {.f.t.t insert insert \\}}");
cmd(inter, "bind .f.t.t <Control-greater> {set choice 42}");
cmd(inter, "bind .f.t.t <Control-less> {set choice 43}");
cmd(inter, "bind .f.t.t <Control-semicolon> {set choice 64}");
cmd(inter, "bind .f.t.t <Control-comma> {set choice 65}");
cmd(inter, "bind .f.t.t <Control-period> {set choice 66}");
cmd(inter, "bind .f.t.t <Alt-q> {.m postcascade 0}");
cmd(inter, "if {\"$tcl_platform(platform)\" == \"unix\"} {bind .f.t.t <Control-Insert> {tk_textCopy .f.t.t}} {}");
cmd(inter, "if {\"$tcl_platform(platform)\" == \"unix\" && $tcl_platform(platform)!= \"Darwin\"} {bind .f.t.t <Control-c> {tk_textCopy .f.t.t}} {}");
cmd(inter, "if {\"$tcl_platform(platform)\" == \"unix\"} {bind .f.t.t <Shift-Insert> {tk_textPaste .f.t.t}} {}");
//cmd(inter, "if {\"$tcl_platform(platform)\" == \"unix\" && $tcl_platform(platform) != \"Darwin\"} {bind .f.t.t <Control-v> {.f.t.t yview scroll -1 pages; tk_textPaste .f.t.t}} {}");

cmd(inter, "bind .f.t.t <KeyPress-Return> {+set choice 16}");
//cmd(inter, "bind .f.t.t <KeyPress-Return> {+lappend ud [.f.t.t get 0.0 \"end - 1 chars\"]; lappend udi [.f.t.t index insert]}");
//cmd(inter, "bind .f.t.t <KeyPress-space> {+lappend ud [.f.t.t get 0.0 \"end - 1 chars\"]; lappend udi [.f.t.t index insert]}");
//cmd(inter, "bind .f.t.t <Control-z> {if {[llength $ud] ==0 } {} {lappend rd [.f.t.t get 0.0 \"end - 1 chars\"]; lappend rdi [.f.t.t index insert]; .f.t.t delete 0.0 end; .f.t.t insert 0.0 [lindex $ud end]; .f.t.t delete end; .f.t.t see [lindex $udi end]; .f.t.t mark set insert [lindex $udi end]; set ud [lreplace $ud end end]; set udi [lreplace $udi end end]; set choice 23}}");


//cmd(inter, "bind .f.t.t <Control-y> {if {[llength $rd] ==0} {} {lappend ud [.f.t.t get 0.0 \"end -1 chars\"]; lappend udi [.f.t.t index insert]; .f.t.t delete 0.0 end; .f.t.t insert 0.0 [lindex $rd end]; .f.t.t delete end; .f.t.t see [lindex $rdi end]; .f.t.t mark set insert [lindex $rdi end]; set rd [lreplace $rd end end]; set rdi [lreplace $rdi end end]; set choice 23} }");

cmd(inter, "bind .f.t.t <KeyRelease-space> {+.f.t.t edit separator}");
cmd(inter, "bind .f.t.t <Control-z> {catch {.f.t.t edit undo}}");
cmd(inter, "bind .f.t.t <Control-y> {catch {.f.t.t edit redo}}");
cmd(inter, "bind . <KeyPress-Insert> {# nothing}");



/*
POPUP manu
*/
cmd(inter, "menu .v -tearoff 0");

cmd(inter, "bind .f.t.t  <3> {%W mark set insert [.f.t.t index @%x,%y]; set vmenuInsert [.f.t.t index insert]; tk_popup .v %X %Y}");
cmd(inter, "bind .f.t.t  <2> {%W mark set insert [.f.t.t index @%x,%y]; set vmenuInsert [.f.t.t index insert]; tk_popup .v %X %Y}");
cmd(inter, ".v add command -label \"Copy\" -command {tk_textCopy .f.t.t}");
cmd(inter, ".v add command -label \"Cut\" -command {tk_textCut .f.t.t}");
cmd(inter, ".v add command -label \"Paste\" -command {tk_textPaste .f.t.t}");

cmd(inter, ".v add separator");
cmd(inter, ".v add cascade -label \"Lsd Script\" -menu .v.i");
cmd(inter, ".v add command -label \"Insert Lsd Script...\" -command {set choice 28}");
cmd(inter, ".v add command -label \"Indent Selection\" -command {set choice 42}");
cmd(inter, ".v add command -label \"De-indent Selection\" -command {set choice 43}");
cmd(inter, ".v add command -label \"Place a Break and Run gdb\" -command {set choice 58}");

cmd(inter, ".v add separator");
cmd(inter, ".v add command -label \"Find...\" -command {set choice 11}");
cmd(inter, ".v add command -label \"Match \\\{ \\}\" -command {set choice 17}");
cmd(inter, ".v add command -label \"Match \\\( \\)\" -command {set choice 32}");

if ( macro == 0 )
{
	cmd( inter, "menu .v.i -tearoff 0");
	cmd( inter, ".v.i add command -label \"Lsd equation\" -command {set choice 25} -accelerator Ctrl+E" );
	cmd( inter, ".v.i add command -label \"cal(...)\" -command {set choice 26} -accelerator Ctrl+V" );
	cmd( inter, ".v.i add command -label \"sum(...)\" -command {set choice 56} -accelerator Ctrl+U" );
	cmd( inter, ".v.i add command -label \"search_var_cond(...)\" -command {set choice 30} -accelerator Ctrl+S" );
	cmd( inter, ".v.i add command -label \"Search(...)\" -command {set choice 55} -accelerator Ctrl+A" );
	cmd( inter, ".v.i add command -label \"write(...)\" -command {set choice 29} -accelerator Ctrl+W" );
	cmd( inter, ".v.i add command -label \"for( ; ; )\" -command {set choice 27} -accelerator Ctrl+C" );
	cmd( inter, ".v.i add command -label \"lsdqsort(...)\" -command {set choice 31} -accelerator Ctrl+T" );
	cmd( inter, ".v.i add command -label \"Add a new object\" -command {set choice 52} -accelerator Ctrl+O" );
	cmd( inter, ".v.i add command -label \"Delete an object\" -command {set choice 53} -accelerator Ctrl+D" );
	cmd( inter, ".v.i add command -label \"Draw random an object\" -command {set choice 54} -accelerator Ctrl+N" );
	cmd( inter, ".v.i add command -label \"increment(...)\" -command {set choice 40} -accelerator Ctrl+I" );
	cmd( inter, ".v.i add command -label \"multiply(...)\" -command {set choice 45} -accelerator Ctrl+M" );
	cmd( inter, ".v.i add command -label \"Math operation\" -command {set choice 51} -accelerator Ctrl+H" );
}
else
{
	cmd( inter, "menu .v.i -tearoff 0");
	cmd( inter, ".v.i add command -label \"Lsd equation\" -command {set choice 25} -accelerator Ctrl+E" );
	cmd( inter, ".v.i add command -label \"V(...)\" -command {set choice 26} -accelerator Ctrl+V" );
	cmd( inter, ".v.i add command -label \"SUM(...)\" -command {set choice 56} -accelerator Ctrl+U" );
	cmd( inter, ".v.i add command -label \"SEARCH_CND(...)\" -command {set choice 30} -accelerator Ctrl+S" );
	cmd( inter, ".v.i add command -label \"SEARCH(...)\" -command {set choice 55} -accelerator Ctrl+A" );
	cmd( inter, ".v.i add command -label \"WRITE(...)\" -command {set choice 29} -accelerator Ctrl+W" );
	cmd( inter, ".v.i add command -label \"CYCLE(...)\" -command {set choice 27} -accelerator Ctrl+C" );
	cmd( inter, ".v.i add command -label \"SORT(...)\" -command {set choice 31} -accelerator Ctrl+T" );
	cmd( inter, ".v.i add command -label \"ADDOBJ(...)\" -command {set choice 52} -accelerator Ctrl+O" );
	cmd( inter, ".v.i add command -label \"DELETE(...))\" -command {set choice 53} -accelerator Ctrl+D" );
	cmd( inter, ".v.i add command -label \"RNDDRAW(...)\" -command {set choice 54} -accelerator Ctrl+N" );
	cmd( inter, ".v.i add command -label \"INCR(...)\" -command {set choice 40} -accelerator Ctrl+I" );
	cmd( inter, ".v.i add command -label \"MULT(...)\" -command {set choice 45} -accelerator Ctrl+M" );
	cmd( inter, ".v.i add command -label \"Math operation\" -command {set choice 51} -accelerator Ctrl+H" );
}

cmd(inter, "bind .f.t.t <Control-E> {set choice 25}");
cmd(inter, "bind .f.t.t <Control-V> {set choice 26}");
cmd(inter, "bind .f.t.t <Control-U> {set choice 56}");
cmd(inter, "bind .f.t.t <Control-A> {set choice 55}");
cmd(inter, "bind .f.t.t <Control-C> {set choice 27}");
cmd(inter, "bind .f.t.t <Control-i> {set choice 28; break}");
cmd(inter, "bind .f.t.t <Control-T> {set choice 31}");
cmd(inter, "bind .f.t.t <Control-I> {set choice 40}");
cmd(inter, "bind .f.t.t <Control-M> {set choice 45}");
cmd(inter, "bind .f.t.t <Control-S> {set choice 30}");
cmd(inter, "bind .f.t.t <Control-W> {set choice 29}");
cmd(inter, "bind .f.t.t <Control-O> {set choice 52}");
cmd(inter, "bind .f.t.t <Control-D> {set choice 53}");
cmd(inter, "bind .f.t.t <Control-N> {set choice 54}");
cmd(inter, "bind .f.t.t <Control-H> {set choice 51}");


cmd(inter, "bind .f.t.t <F1> {set choice 34}");



cmd(inter, "set textsearch \"\"");
cmd(inter, "set datasel \"\"");

cmd(inter, ". configure -menu .m");


cmd(inter, "pack .f -expand yes -fill both");
cmd(inter, "pack .f.t -expand yes -fill both");
cmd(inter, "pack .f.t.vs -side right -fill y");
cmd(inter, "pack .f.t.t -expand yes -fill both");
cmd(inter, "pack .f.t.hs -fill x");


cmd(inter, "set filename \"noname.txt\"");
cmd(inter, "set dirname [pwd]");
cmd(inter, "set modeldir \"[pwd]\"");
cmd(inter, "set groupdir [pwd]");

cmd(inter, ".f.t.t tag conf sel -foreground white");
cmd(inter, "set before [.f.t.t get 1.0 end]");
cmd(inter, "set a [wm maxsize .]");
cmd(inter, "set c \"[ expr [lindex $a 0] - 80]x[expr [lindex $a 1] - 105]+80+30\"");
cmd(inter, "wm geometry . $c");
// change window icon
cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {wm iconbitmap . -default $RootLsd/$LsdSrc/lmm.ico} {wm iconbitmap . @$RootLsd/$LsdSrc/lmm.xbm}");

if(argn>1)
 {sprintf(msg, "if {[file exists \"$filetoload\"] == 1} {set choice 0} {set choice -2}");
  cmd(inter, msg);
  if(choice == 0)
    {sprintf(msg, "set file [open \"$filetoload\"]");
     cmd(inter, msg);
     cmd(inter, ".f.t.t insert end [read $file]");
     cmd(inter, ".f.t.t edit reset");
     cmd(inter, "close $file");
     cmd(inter, ".f.t.t mark set insert 1.0");
     sprintf(msg, "set filename \"[file tail $filetoload]\"");
     cmd(inter, msg);
     cmd(inter, "set dirname [file dirname \"$filetoload\"]");
     cmd(inter, "set before [.f.t.t get 1.0 end]; .f.hea.file.dat conf -text \"$filename\"");
    cmd(inter, "wm title . \"$filename - LMM\"");          
     
     sprintf(msg, "set s [file extension \"$filetoload\"]" );
     cmd(inter, msg);
     s=(char *)Tcl_GetVar(inter, "s",0);
     choice=0;
     if(s[0]!='\0')
       {strcpy(str, s);
        if(!strcmp(str, ".cpp") || !strcmp(str, ".c") || !strcmp(str, ".C") || !strcmp(str, ".CPP") || !strcmp(str, ".Cpp") || !strcmp(str, ".c++") || !strcmp(str, ".C++")|| !strcmp(str, ".h")|| !strcmp(str, ".H"))
          {sourcefile=1;
//           cmd(inter, "set inicolor \"1.0\"");
//           cmd(inter, "set endcolor [.f.t.t index end]");
//           color(&num);
			color(shigh, 0, 0);			// set color types (all text)
          }
        else
          sourcefile=0;
       }

    }
   else
	cmd( inter, "tk_messageBox -parent .f.t.t -type ok -icon error -title \"Error\" -message \"File\\n$filetoload\\nnot found.\"");
   
  }
 else
  choice= -2; 
  




cmd(inter, "set tcl_nonwordchars \\\"");
cmd(inter, "focus -force .f.t.t");
//cmd(inter, "bind .f.t.t <Control-o> {; set choice 15}");
//cmd(inter, "wm focusmodel . active");
/*
cmd(inter, "set trash [bind .f.t.t]");
cmd(inter, ".f.t.t insert end \"$trash\"");
*/


cmd(inter, "set lfindcounter -1");
loop:


loop_help:
while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "set b [.f.t.t tag ranges sel]");
cmd(inter, "if {$b==\"\"} {} {set textsearch [.f.t.t get sel.first sel.last]}");

if(choice==-1)
 {
  Tcl_Exit(0);
  exit(0);
 }

cmd(inter, "if { [winfo exists .]==1} {set after [.f.t.t get 1.0 end]} { set after $before}");
cmd(inter, "if {[string compare $before $after] != 0} {set tosave 1} {set tosave 0}");
if( tosave==1 && (choice==2 || choice==15 || choice==1 || choice==13 || choice==14 ||choice==6 ||choice==8 ||choice==3 || choice==33||choice==5||choice==39||choice==41))
  {

//  cmd(inter, "set answer [tk_dialog .dia \"Save?\" \"Save the file $filename?\" \"\" 0 yes no cancel]");
  cmd(inter, "set answer [tk_messageBox -type yesnocancel -default yes -icon warning -title \"Save File?\" -message \"Recent changes to file '$filename' have not been saved. Do you want to save before continuing?\nNot doing so will not include recent changes to subsequent actions.\n\n- Yes: save the file and continue.\n- No: do not save and continue.\n- Cancel: do not save and return to editing.\"]");
  //cmd(inter, " if { $answer == \"yes\"} {set tk_strictMotif 0; set curfile [tk_getSaveFile -initialfile $filename -initialdir $dirname]; set tk_strictMotif 1; if { [string length $curfile] > 0} {set file [open $curfile w]; puts -nonewline $file [.f.t.t get 0.0 end]; close $file; set before [.f.t.t get 0.0 end]} {}} {if {$answer  == \"cancel\"} {set choice 0} {}}");
  
//cmd(inter, " if { $answer == \"yes\"} { set curfile [tk_getSaveFile -initialfile $filename -initialdir $dirname];  if { [string length $curfile] > 0} {set file [open $curfile w]; puts -nonewline $file [.f.t.t get 0.0 end]; close $file; set before [.f.t.t get 0.0 end]} {}} {if {$answer  == \"cancel\"} {set choice 0} {}}");  if(choice==0)
cmd(inter, " if { $answer == \"yes\"} {set curfile [file join $dirname $filename]; set file [open $curfile w]; puts -nonewline $file [.f.t.t get 0.0 end]; close $file; set before [.f.t.t get 0.0 end]} {if {$answer  == \"cancel\"} {set choice 0} {}}");  
if(choice==0)
  goto loop;

  }


if(choice==1)
 {
  Tcl_Exit(0);
  exit(0);
 }

if(tosave==2)
 {choice=0;
  goto loop;
 } 
  
if(choice==-2)
{
/*
Initial stuff. Don't ask me why, it does not work if it is placed before the loop...
*/

choice=0;

#ifdef DUAL_MONITOR
Tcl_SetVar(inter, "hsize", hsize, 0);		// horizontal size in pixels
Tcl_SetVar(inter, "vsize", vsize, 0);		// vertical minimum size in pixels
Tcl_SetVar(inter, "hmargin", hmargin, 0);	// horizontal right margin from the screen borders
Tcl_SetVar(inter, "vmargin", vmargin, 0);	// vertical margins from the screen borders
Tcl_SetVar(inter, "bordsize", bordsize, 0);	// width of windows borders
Tcl_SetVar(inter, "tbarsize", tbarsize, 0);	// size in pixels of bottom taskbar (exclusion area)
cmd(inter, "if {[expr [winfo screenwidth .]] < ($hsize + 2*$bordsize)} {set w [expr [winfo screenwidth .] - 2*$bordsize]} {set w $hsize}");
cmd(inter, "set h [expr [winfo screenheight .] - $tbarsize - 2*$vmargin - 2*$bordsize]; if {$h < $vsize} {set h [expr [winfo screenheight .] - $tbarsize - 2*$bordsize]}");
cmd(inter, "if {[expr [winfo screenwidth .]] < ($hsize + 2*$bordsize + $hmargin)} {set x 0} {set x [expr [winfo screenwidth .] -$hmargin - $bordsize - $w]}");
cmd(inter, "set y [expr ([winfo screenheight .]-$tbarsize)/2 - $bordsize - $h/2]");
cmd(inter, "wm geom . [expr $w]x$h+$x+$y"); // main window geometry setting

//cmd(inter, "set sw [expr [winfo screenwidth .]]; set vx [expr [winfo vrootx .]]");
//cmd(inter, "set sh [expr [winfo screenheight .]]; set vy [expr [winfo vrooty .]]");
//cmd(inter, "tk_messageBox -type ok -message \"screenwidth=$sw\\nreqwidth=$w\\nvrootx=$vx\\nscreenheight=$sh\\nreqheight=$h\\nvrooty=$vy\\nx=$x\\ny=$y\"");
#endif

cmd(inter, "toplevel .a");
cmd(inter, "wm title .a \"Choose\"");
cmd( inter, "wm protocol .a WM_DELETE_WINDOW { set temp 0; .a.b.ok invoke }" );

cmd(inter, "frame .a.f -relief groove -bd 4");
cmd(inter, "set temp 33");
cmd(inter, "label .a.f.l -text \"Choose Action\"  -fg red");

cmd(inter, "radiobutton .a.f.r1 -variable temp -value 33 -text \"Browse Models\" -justify left -relief groove -anchor w");

cmd(inter, "radiobutton .a.f.r2 -variable temp -value 15 -text \"Open a text file.\" -justify left -relief groove -anchor w");

cmd(inter, "radiobutton .a.f.r3 -variable temp -value 0 -text \"Create a new text file.\" -justify left -relief groove -anchor w");


cmd(inter, "pack .a.f.l ");
cmd(inter, "pack .a.f.r1 .a.f.r2 .a.f.r3 -anchor w -fill x ");

cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "button .a.b.help -padx 20 -text Help -command {LsdHelp LMM_help.html#introduction}");
cmd(inter, "pack .a.b.ok .a.b.help -padx 1 -pady 5 -side left");
cmd(inter, "pack  .a.f .a.b -fill x");
cmd(inter, "bind .a <Return> {.a.b.ok invoke}");
cmd(inter, "bind .a <Escape> {set temp 0; .a.b.ok invoke}");
cmd( inter, "setgeom .a" );
cmd(inter, "set i 1; bind .a <Down> { if { $i < 3 } {incr i; .a.f.r$i invoke} {} }"); 
cmd(inter, "bind .a <Up> { if { $i > 1 } {incr i -1; .a.f.r$i invoke} {} }");
cmd(inter, "focus -force .a.f.r1");
cmd(inter, "blocklmm .a");
while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

cmd(inter, "set choice $temp");




goto loop;
}

if ( choice == 2 || choice == 6 )
 {
/*compile the model, invoking make*/
/*Run the model in the selection*/

bool run = ( choice == 2 ) ? true : false;
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {//this control is obsolete, since  model must be selected in order to arrive here
  cmd(inter, "tk_messageBox -title Error -icon error -type ok -default ok -message \"No model selected. Choose an existing model or create a new one.\"");
  choice=0;
  goto loop;
 }

  cmd(inter, "cd $modeldir");
  
  delete_compresult_window( );		// close any open compilation results window

  make_makefile();

  cmd(inter, "set fapp [file nativename $modeldir/makefile]");
  s=(char *)Tcl_GetVar(inter, "fapp",0);
  
  //register equation file's name
  f=fopen(s, "r");

  if(f==NULL)
   {
     f=fopen("makefile", "w");
     fclose(f);
    cmd(inter, "tk_messageBox -title Error -icon error -type ok -default ok -message \"File 'makefile' not found.\\nUse the Model Compilation Options, in menu Model, to create it.\"");

    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }
  fscanf(f, "%s", str);
  while(strncmp(str, "FUN=", 4) && fscanf(f, "%s", str)!=EOF);
  fclose(f);
  if(strncmp(str, "FUN=", 4)!=0)
   {
    cmd(inter, "tk_messageBox -type ok -title Error -icon error -message \"Makefile corrupted. Check Model and System Compilation options.\"");
    choice=0;
    goto loop;
   }
   sprintf(msg, "set fname %s.cpp", str+4);
   cmd(inter, msg);
  
  f=fopen(s, "r");

  if(f==NULL)
   {
     f=fopen("makefile", "w");
     fclose(f);
    cmd(inter, "tk_messageBox -title Error -icon error -type ok -default ok -message \"File 'makefile' not found.\\nUse the Model Compilation Options, in menu Model, to create it.\"");

    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }
  fscanf(f, "%s", str);
  while(strncmp(str, "TARGET=", 7) && fscanf(f, "%s", str)!=EOF);
  fclose(f);
  if(strncmp(str, "TARGET=", 7)!=0)
   {
    cmd(inter, "tk_messageBox -type ok -title Error -icon error -message \"Makefile corrupted. Check Model and System Compilation options.\"");
    choice=0;
    goto loop;
   }

  s = ( char * ) Tcl_GetVar( inter, "autoHide", 0 );	// get auto hide status

  cmd(inter, "set init_time [clock seconds]"); 
  cmd(inter, "toplevel .t");
  // change window icon
  cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap .t @$RootLsd/$LsdSrc/lmm.xbm} {}");
  cmd(inter, "wm title .t \"Please Wait\"");
  cmd(inter, "label .t.l1 -font {-weight bold} -text \"Making model...\"");
  if ( run )
	cmd(inter, "label .t.l2 -text \"The system is checking the files modified since the last compilation and recompiling as necessary.\nOn success the new model program will be launched and LMM will stay minimized.\nOn failure a text window will show the compiling error messages.\"");
  else
	cmd(inter, "label .t.l2 -text \"The system is checking the files modified since the last compilation and recompiling as necessary.\nOn failure a text window will show the compiling error messages.\"");
  cmd(inter, "pack .t.l1 .t.l2");
  if ( run && ! strcmp( s, "1" ) )	// auto hide LMM if appropriate
	cmd(inter, "wm iconify .");
  cmd(inter, "focus -force .t");
  cmd( inter, "setgeom .t" );
  cmd(inter, "blocklmm .t");
  cmd(inter, "update");  
  cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
  if(choice==0)
    cmd(inter, "set result \"[catch [exec make -fmakefile 2> makemessage.txt]]\""); 
  else
   {  

  cmd(inter, "set result -2.2");
  cmd(inter, "set file [open a.bat w]");
  cmd(inter, "puts -nonewline $file \"make -fmakefile 2> makemessage.txt\\n\"");
  cmd(inter, "close  $file");
  cmd(inter, "if { [file exists $RootLsd/$LsdSrc/system_options.txt] == 1} {set choice 0} {set choice 1}");
  sprintf(msg, "if { [file exists %s.exe]  == 1} {file rename -force %s.exe %sOld.exe} { }", str+7, str+7, str+7);
  cmd(inter, msg);
  cmd(inter, "if { [file exists $RootLsd/$LsdGnu/bin/crtend.o] == 1} { file copy $RootLsd/$LsdGnu/bin/crtend.o .;file copy $RootLsd/$LsdGnu/bin/crtbegin.o .;file copy $RootLsd/$LsdGnu/bin/crt2.o .} {}");

  cmd(inter, "catch [set result [catch [exec a.bat]] ]");
  //cmd(inter, "tkwait variable result");
  cmd(inter, "file delete a.bat");
  cmd(inter, "if { [file exists crtend.o] == 1} { file delete crtend.o;file delete crtbegin.o ;file delete crt2.o } {}");

   }
  cmd(inter, "sblocklmm .t");
  cmd(inter, "destroy .t");
//  cmd(inter, "wm deiconify .");  // only reopen if error
  cmd(inter, "update");
  
  cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set add_exe \".exe\"} {set add_exe \"\"}");
  
  cmd(inter, "if { [file size makemessage.txt]==0 } {set choice 0} {set choice 1}");
  if(choice==1)
  {
    cmd(inter, "set funtime [file mtime $fname]");
    sprintf(msg, "if { [file exist %s$add_exe] == 1 } {set exectime [file mtime %s$add_exe]} {set exectime $init_time}",str+7,str+7);
    cmd(inter, msg);
  
    //cmd(inter, "if {$funtime > $exectime } {set choice 1} {set choice 0}");
    cmd(inter, "if {$init_time < $exectime } {set choice 0} { }");
    //turn into 0 if the executable is newer than the compilation command, implying just warnings
  }
  

  if(choice==1)
   { //problem
   cmd(inter, "wm deiconify .");  // only reopen if error
   create_compresult_window();
/* 
Old message, offering to run an existing executable. Never used in 10 years, better to scrap it
   
  sprintf(msg, "if { [file exist %s$add_exe]==0 } {set choice 1} {set choice 0}", str+7);
  cmd(inter, msg);
   if(choice==1)
    {//the exe exists
   
   create_compresult_window();
   
   choice=0;
   cmd(inter, "toplevel .a");
   cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
   cmd(inter, "wm title .a \"Warning\"");
   cmd(inter, "label .a.l -text \"Compilation issued an error message, but a model program exists, probably generated with an version of the equation file.\\n.\\nDo you want to run the existing model program anyway?\"");
   cmd(inter, "pack .a.l");
   cmd(inter, "frame .a.b");
   cmd(inter, "button .a.b.help -padx 20 -text Help -command {LsdHelp LMM_help.html#run}");
   cmd(inter, "button .a.b.run -text Run -command {set choice 1}");
   cmd(inter, "button .a.b.norun -text \"Don't run\" -command {set choice 2}");
   cmd(inter, "pack .a.b.run .a.b.norun .a.b.help -side left -expand yes -fill x");
   cmd(inter, "pack .a.b -fill x");
   cmd(inter, "bind .a <Return> {.a.b.norun invoke}");
   cmd(inter, "bind .a <Escape> {.a.b.norun invoke}");
   choice=0;
   cmd(inter, "toplevel .a");
   cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
   cmd(inter, "wm title .a \"Warning\"");
   cmd(inter, "label .a.l -text \"Compilation failed. Read the compiler's message to fix the errors.\"");
   cmd(inter, "pack .a.l");
   cmd(inter, "frame .a.b");
 
   cmd(inter, "focus -force .a.b.norun");
   cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
   //cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
   cmd(inter, "blocklmm .a");
   while(choice==0)
    Tcl_DoOneEvent(0);
   cmd(inter, "sblocklmm .a");
   
      if(choice==2)
       {cmd(inter, "cd $RootLsd"); 
        choice=0;
        goto loop;
       } 
      
    }   
   else
    {//no exe file
    
   create_compresult_window();

   choice=0;
   cmd(inter, "toplevel .a");
   cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
   cmd(inter, "wm title .a \"Compilation Error\"");
   cmd(inter, "label .a.l -text \"Compilation issued an error message.\\nFix the errors and try again\"");
   cmd(inter, "pack .a.l");
   cmd(inter, "frame .a.b");
   cmd(inter, "button .a.b.help -padx 20 -text Help -command {LsdHelp LMM_help.html#run}");
   cmd(inter, "button .a.b.run -text Ok -command {set choice 1}");
   cmd(inter, "pack .a.b.run .a.b.help -side left -expand yes -fill x");
   cmd(inter, "pack .a.b -fill x");
   cmd(inter, "bind .a <Return> {.a.b.run invoke}");
   cmd(inter, "bind .a <Escape> {.a.b.run invoke}");
   cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
   //cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
   cmd(inter, "blocklmm .a");
   while(choice==0)
    Tcl_DoOneEvent(0);
   cmd(inter, "sblocklmm .a");
  
    } 
   /**************/ 
   }
  else
	if ( run )
    {//no problem
     strcpy(str1, str+7);
      cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\"} {set choice 4} {set choice 2}} {set choice 3}}");
      if(choice==1) //unix
       sprintf(msg,"exec ./%s &",str1);
      if(choice==2) //win2k
       sprintf(msg, "exec %s.exe &", str1); //Changed
      if(choice==3) //win 95/98
       sprintf(msg, "exec start %s.exe &", str1);
      if(choice==4)  //win NT
       sprintf(msg, "exec cmd /c start %s.exe &", str1);
       
      cmd(inter, msg);
      choice=0;
     } 
 cmd(inter, "cd $RootLsd");
 choice=0;
 cmd(inter, "focus -force .f.t.t");
 goto loop;
 }



if(choice==3)
 {
  /*Insert in the text window the makefile of the selected model*/

cmd(inter, ".f.t.t delete 0.0 end");
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {//this control is obsolete, since  model must be selected in order to arrive here
     cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"No model selected.\"");
  choice=0;
  goto loop;
 }

cmd(inter, "cd $modeldir");
make_makefile(); 
cmd(inter, "cd $RootLsd");
cmd(inter, "if { [file exists $modeldir/makefile]==1} {set choice 1} {set choice 0}");
if(choice==1)
 {
  cmd(inter, "set file [open $modeldir/makefile]");
  cmd(inter, ".f.t.t insert end [read -nonewline $file]");
  cmd(inter, ".f.t.t edit reset");
  cmd(inter, "close $file");
 } 
sourcefile=0; 
cmd(inter, "set before [.f.t.t get 1.0 end]");
cmd(inter, "set filename makefile");
cmd(inter, ".f.t.t mark set insert 1.0");
cmd(inter, ".f.hea.file.dat conf -text \"makefile\"");
cmd(inter, "wm title . \"Makefile - LMM\"");
cmd(inter, "tk_messageBox -title Warning -icon warning -type ok -message \"Direct changes to the 'makefile' will not affect compilation issued through LMM. Choose System Compilation options in Model Compilation Options (menu Model).\"");  
choice=0;
goto loop;
}

if(choice==4)
{
 /*Save the file currently shown*/


//cmd(inter, "set tk_strictMotif 0; set curfilename [tk_getSaveFile -initialfile $filename -initialdir $dirname]; set tk_strictMotif 1");
cmd(inter, "set curfilename [tk_getSaveFile -initialfile $filename -initialdir $dirname]");
s=(char *)Tcl_GetVar(inter, "curfilename",0);

if(s!=NULL && strcmp(s, ""))
 {
  cmd(inter, "if { [file exist $dirname/$filename] == 1} {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak} {}");
  cmd(inter, "set file [open $curfilename w]");
  cmd(inter, "puts -nonewline $file [.f.t.t get 0.0 end]");
  cmd(inter, "close $file");
  cmd(inter, "set before [.f.t.t get 0.0 end]");
  cmd(inter, "set dirname [file dirname $curfilename]");
  cmd(inter, "set filename [file tail $curfilename]");
  cmd(inter, ".f.hea.file.dat conf -text \"$filename\"");
  cmd(inter, "wm title . \"$filename - LMM\"");
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
  cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"No model selected.\"");
  choice=0;
  goto loop;
 }

  cmd(inter, ".f.t.t delete 0.0 end");
  cmd(inter, "if [file exists $modeldir/description.txt]==1 {set choice 1} {set choice 0}");
 if(choice==1)
  {cmd(inter, "set file [open $modeldir/description.txt]");
   cmd(inter, ".f.t.t insert end [read -nonewline $file]");
   cmd(inter, "close $file");
   cmd(inter, ".f.t.t edit reset");
   sourcefile=0;
   cmd(inter, ".f.t.t mark set insert 1.0");
   cmd(inter, "focus -force .f.t.t");
  }
  cmd(inter, "set before [.f.t.t get 1.0 end]");
  cmd(inter, "set filename description.txt");
  cmd(inter, ".f.hea.file.dat conf -text $filename");
  cmd(inter, "wm title . \"$filename - LMM\"");
  cmd(inter, "catch [unset -nocomplain ud]");
  cmd(inter, "catch [unset -nocomplain udi]");
  cmd(inter, "catch [unset -nocomplain rd]");
  cmd(inter, "catch [unset -nocomplain rdi]");
  cmd(inter, "lappend ud [.f.t.t get 0.0 end]");
  cmd(inter, "lappend udi [.f.t.t index insert]");
  
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
  cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"No model selected.\"");
  choice=0;
  goto loop;
 }

create_compresult_window();
choice=0;
goto loop;

  cmd(inter, "if [file exists $modeldir/makemessage.txt]==1 {set choice 1} {set choice 0}");
 if(choice==1)
  {
    
    create_compresult_window();
    
cmd(inter, "catch [unset -nocomplain ud]");
cmd(inter, "catch [unset -nocomplain udi]");
cmd(inter, "catch [unset -nocomplain rd]");
cmd(inter, "catch [unset -nocomplain rdi]");
cmd(inter, "lappend ud [.f.t.t get 0.0 end]");
cmd(inter, "lappend udi [.f.t.t index insert]");

  }
  else
  {

   cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"No compilation results.\"");
  choice=0;
  goto loop;

  }

  choice=0;
  goto loop;
 }

if(choice==8)
{
  /*Insert in the text window the equation file*/

cmd(inter, ".f.t.t delete 0.0 end");
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {
       cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"No model selected.\"");
  choice=0;
  goto loop;
 }
  cmd(inter, "cd $modeldir");
  

  make_makefile();

  cmd(inter, "set fapp [file nativename $modeldir/makefile]");
  s=(char *)Tcl_GetVar(inter, "fapp",0);
  
  f=fopen(s, "r");
  if(f==NULL)
   {
   
    cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"File 'makefile' not found.\\nAdd a makefile to model $modelname.\"");

    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }
  fscanf(f, "%s", str);
  while(strncmp(str, "FUN=", 4) && fscanf(f, "%s", str)!=EOF);    
  fclose(f);
  if(strncmp(str, "FUN=", 4)!=0)
   {
    cmd(inter, "tk_messageBox -type ok -title Error -icon error -message \"Makefile corrupted. Check Model and System Compilation options.\"");
    choice=0;
    goto loop;
   }
  


sprintf(msg, "set filename %s.cpp", str+4);
cmd(inter, msg);
cmd(inter, "cd $RootLsd");
s=(char *)Tcl_GetVar(inter, "filename",0);

if(s==NULL || !strcmp(s, ""))
 {
  cmd(inter, ".f.t.t delete 0.0 end");
  cmd(inter, "update");
  cmd(inter, "set filename \"\"");
  cmd(inter, "set dirname \"\"");
  cmd(inter, "set before [.f.t.t get 1.0 end]");  
  choice=0;
  goto loop;
 }
  cmd(inter, "focus -force .f.t.t");
  cmd(inter, "set dirname $modeldir");
  cmd(inter, "set file [open $modeldir/$filename]");
  cmd(inter, ".f.t.t insert end [read -nonewline $file]");
  cmd(inter, "close $file");
  cmd(inter, ".f.t.t edit reset");
  sourcefile=1;
  cmd(inter, "set before [.f.t.t get 1.0 end]");
//  cmd(inter, "set a [.f.t.t search \"if(!strcmp\" 1.0]");
//  cmd(inter, "if {$a !=\"\" } {.f.t.t yview $a;.f.t.t mark set insert \"$a + 12 lines\"} {}");

  cmd(inter, ".f.hea.file.dat conf -text \"$filename\"");
  cmd(inter, "wm title . \"$filename - LMM\"");
  cmd(inter, "update");
  sourcefile=1;
  choice=0;
//  cmd(inter, "set inicolor \"1.0\"");
//  cmd(inter, "set endcolor [.f.t.t index end]");
  cmd(inter, ".f.t.t tag add bc \"1.0\"");
  cmd(inter, ".f.t.t tag add fc \"1.0\"");
  cmd(inter, ".f.t.t mark set insert 1.0");
//  color(&num);
color(shigh, 0, 0);			// set color types (all text)
  
cmd(inter, "catch [unset -nocomplain ud]");
cmd(inter, "catch [unset -nocomplain udi]");
cmd(inter, "catch [unset -nocomplain rd]");
cmd(inter, "catch [unset -nocomplain rdi]");
cmd(inter, "lappend ud [.f.t.t get 0.0 end]");
cmd(inter, "lappend udi [.f.t.t index insert]");

  goto loop;
 }


if(choice==10)
{

/* Find a line in the text*/
cmd(inter, "if {[winfo exists .search_line]==1} {set choice 0; focus -force .search_line.e} {}");
if(choice==0)
 goto loop;

cmd(inter, "set line \"\"");
cmd(inter, "toplevel .search_line");
cmd(inter, "wm protocol .search_line WM_DELETE_WINDOW { }");
cmd(inter, "wm title .search_line \"Goto Line\"");
cmd(inter, "wm transient .search_line .");
cmd(inter, "label .search_line.l -text \"Type the line number\"");
cmd(inter, "entry .search_line.e -justify center -width 10 -textvariable line");
cmd(inter, "frame .search_line.b");
cmd(inter, "button .search_line.b.ok -padx 25 -text Ok -command {if {$line == \"\"} {.search_line.esc invoke} {.f.t.t see $line.0; .f.t.t tag remove sel 1.0 end; .f.t.t tag add sel $line.0 $line.500; .f.t.t mark set insert $line.0; .f.hea.line.line conf -text [.f.t.t index insert]; destroy .search_line; sblocklmm .search_line } }");
cmd(inter, "button .search_line.b.esc -padx 15 -text Cancel -command {sblocklmm .search_line}");
cmd(inter, "bind .search_line <KeyPress-Return> {.search_line.b.ok invoke}");
cmd(inter, "bind .search_line <KeyPress-Escape> {.search_line.b.esc invoke}");

cmd(inter, "pack .search_line.b.ok .search_line.b.esc -padx 1 -pady 5 -side left");
cmd(inter, "pack .search_line.l .search_line.e .search_line.b");
cmd( inter, "setgeom .search_line" );

cmd(inter, "focus -force .search_line.e");
cmd(inter, "blocklmm .search_line");
choice=0;
goto loop;
}

if(choice==11)
{
/* Find a text pattern in the text*/
cmd(inter, "if {[winfo exists .find]==1} {set choice 0; focus -force .find.e} {}");
if(choice==0)
 goto loop;

cmd(inter, "set curcounter $lfindcounter");
cmd(inter, "toplevel .find");
cmd(inter, "wm transient .find .");
cmd(inter, "wm protocol .find WM_DELETE_WINDOW { }");
cmd(inter, "wm title .find \"Search Text\"");

cmd(inter, "label .find.l -text \"Type the text to search\"");
cmd(inter, "entry .find.e -width 30 -textvariable textsearch");
cmd(inter, ".find.e selection range 0 end");
cmd(inter, "set docase 0");
cmd(inter, "checkbutton .find.c -text \"Case Sensitive\" -variable docase");
cmd(inter, "radiobutton .find.r1 -text \"Down\" -variable dirsearch -value \"-forwards\" -command {set endsearch end}");
cmd(inter, "radiobutton .find.r2 -text \"Up\" -variable dirsearch -value \"-backwards\" -command {set endsearch 1.0}" );

cmd(inter, "frame .find.b");
cmd(inter, "button .find.b.ok -padx 15 -text Search -command {incr lfindcounter; set curcounter $lfindcounter; lappend lfind \"$textsearch\"; if {$docase==1} {set case \"-exact\"} {set case \"-nocase\"}; .f.t.t tag remove sel 1.0 end; set cur [.f.t.t index insert]; set cur [.f.t.t search $dirsearch -count length $case -- \"$textsearch\" $cur $endsearch]; if {[string length $cur] > 0} {.f.t.t tag add sel $cur \"$cur + $length char\"; if {[string compare $endsearch end]==0} {} {set length 0}; .f.t.t mark set insert \"$cur + $length char\" ; update; .f.t.t see $cur; sblocklmm .find} {.find.e selection range 0 end; bell}}");

cmd(inter, "bind .find.e <Up> {if { $curcounter >= 0} {incr curcounter -1; set textsearch \"[lindex $lfind $curcounter]\"; .find.e selection range 0 end;} {}}");
cmd(inter, "bind .find.e <Down> {if { $curcounter <= $lfindcounter} {incr curcounter; set textsearch \"[lindex $lfind $curcounter]\"; .find.e selection range 0 end;} {}}");

cmd(inter, "button .find.b.esc -padx 15 -text Cancel -command {sblocklmm .find}");


cmd(inter, "bind .find <KeyPress-Return> {.find.b.ok invoke}");
cmd(inter, "bind .find <KeyPress-Escape> {.find.b.esc invoke}");

cmd(inter, "pack .find.b.ok .find.b.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .find.l .find.e .find.r1 .find.r2 .find.c -fill x");
cmd(inter, "pack .find.b");

cmd( inter, "setgeom .find" );
cmd(inter, "blocklmm .find");
cmd(inter, "focus -force .find.e");

choice=0;
goto loop;
}

if(choice==12)
{
/* Search again the same pattern in the text*/

cmd(inter, "if {$textsearch ==\"\"} {} {.f.t.t tag remove sel 1.0 end; set cur [.f.t.t index insert]; set cur [.f.t.t search -count length $dirsearch $case -- $textsearch $cur $endsearch]; if {[string length $cur] > 0} {.f.t.t tag add sel $cur \"$cur + $length char\";if {[string compare $endsearch end]==0} {} {set length 0}; .f.t.t mark set insert \"$cur + $length char\" ; update; .f.t.t see $cur; destroy .l; ;focus -force .f.t.t; } {}}");
choice=0;
goto loop;
}

if(choice==13 || choice==58)
{
 /*Run the model in the gdb debugger */
s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {
       cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"No model selected.\"");
  choice=0;
  goto loop;
 }

  
  cmd(inter, "cd $modeldir");
  if(choice==58)
   {
   //cmd(inter, "scan [.f.t.t index insert] %d.%d line col");
   cmd(inter, "scan $vmenuInsert %d.%d line col");
   cmd(inter, "set f [open break.txt w]; puts $f \"break $filename:$line\nrun\n\"; close $f");
   cmd(inter, "set cmdbreak \"--command=break.txt\"");
   
   }
  else
   cmd(inter, "set cmdbreak \"\""); 
  make_makefile();  
  cmd(inter, "set fapp [file nativename $modeldir/makefile]");
  s=(char *)Tcl_GetVar(inter, "fapp",0);
  
  f=fopen(s, "r");
  
  if(f==NULL)
   {
    cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"File 'makefile' for model '$modelname' not found.\"");
    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }
  fscanf(f, "%s", str);
  while(strncmp(str, "TARGET=", 7) && fscanf(f, "%s", str)!=EOF);
  if(strncmp(str, "TARGET=", 7)!=0)
   {
    cmd(inter, "tk_messageBox -type ok -title Error -icon error -message \"Makefile corrupted. Check Model and System Compilation options.\"");
    choice=0;
    goto loop;
   }
  

  if(f!=NULL && f!=(FILE *)EOF)
   {strcpy(str1, str+7);
    
   //cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"5.0\" || $tcl_platform(osVersion) == \"5.1\" || $tcl_platform(osVersion) == \"5.2\"} {set choice 5} {set choice 4}} {set choice 3}}");
cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} {set choice 5} {set choice 3}}");
   if(choice==3)
    {
     
    cmd(inter, "set SETDIR \"../$LsdGnu/share/gdbtcl\"");
//    sprintf(msg, "exec start /min gdb_bat %s $SETDIR &", str1); //Win 9x
    sprintf(msg, "exec start gdb_batw9x %s &", str1); //Win 9x    
	sprintf( str, "%s%s", str1, ".exe" );				// full executable file name
    }

   if(choice==4)
    {//WIndows NT case
    cmd(inter, "set SETDIR \"../$LsdGnu/share/gdbtcl\"");
    sprintf(msg, "exec cmd.exe /c start /min gdb_bat %s $SETDIR &", str1); //Win NT case
	sprintf( str, "%s%s", str1, ".exe" );				// full executable file name
    }
   if(choice==5)
    {//Windows 2000
     
//     cmd(inter, "set answer [tk_messageBox -type yesno -title \"Text-based GDB\" -icon question -message \"Use text-based GDB?\"]");
//     cmd(inter, "if {$answer == \"yes\"} {set nowin \"-nw\"} {set nowin \"\"}");
    cmd(inter, "set nowin \"\"");
    sprintf(msg, "set f [open run_gdb.bat w]; puts $f \"SET GDBTK_LIBRARY=$RootLsd/$LsdGnu/share/gdbtcl\\nstart gdb $nowin %s $cmdbreak &\\n\"; close $f",str1);
    cmd(inter, msg);
    sprintf(msg, "exec run_gdb &");
	sprintf( str, "%s%s", str1, ".exe" );				// full executable file name
     
    }

   if(choice==1)
     {
      sprintf(msg, "if {$cmdbreak==\"\"} {exec $Terminal -e gdb %s &} {exec $Terminal -e gdb $cmdbreak %s &}", str1, str1); //Unix case
	sprintf( str, "%s", str1 );				// full executable file name
     }
     
	// check if executable file is older than model file
	s = ( char * ) Tcl_GetVar( inter, "modeldir", 0 );
	sprintf( str1, "%s/%s", s, str );
	strcpy( str, s );
	s = ( char * ) Tcl_GetVar( inter, "filename", 0 );
	sprintf( str2, "%s/%s", str, s );

	// get OS info for files
	struct stat stExe, stMod;
	if ( stat( str, &stExe ) == 0 && stat( str1, &stMod ) == 0 )
	{
		if ( difftime( stExe.st_mtime, stMod.st_mtime ) < 0 )
		{
			cmd( inter, "set answer [t k_messageBox -title Warning -icon warning -type okcancel -default cancel -message \"The executable file is older than the last version of the model.\n\nPress 'Ok' to continue anyway or 'Cancel' to return to LMM. Please recompile the model to avoid this message.\"]; if { string equal -nocase $answer ok } { set choice 1 } { set choice 2 }" );
			if ( choice == 2 )
			{
				choice = 0;
				goto loop;
			}
			choice = 0;
		}
	}

	 cmd(inter, msg);
    cmd(inter, "cd $RootLsd"); 
   } 
  else
   {//executable not found
  cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"Executable not found. Compile the model before running it in the GDB debugger.\"");
  choice=0;
  goto loop;
    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }



choice =0;
goto loop;

}

if(choice==14)
{
/* Create a new model/group */

choice=0;


cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"New Model or Group\"");
cmd(inter, "wm transient .a .");
cmd(inter, "label .a.tit -text \"Current group:\\n$modelgroup\"");
cmd(inter, "frame .a.f -relief groove");

cmd(inter, "set temp 1");
cmd(inter, "radiobutton .a.f.r1 -variable temp -value 1 -text \"Create a new model in the current group\" -justify left -relief groove -anchor w");
cmd(inter, "radiobutton .a.f.r2 -variable temp -value 2 -text \"Create a new model in a new group\" -justify left -relief groove -anchor w");
cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "button .a.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.f.r1 .a.f.r2 -fill x");
cmd(inter, "pack .a.b.ok .a.b.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.tit .a.f -fill x");
cmd(inter, "pack .a.b");
cmd(inter, "bind .a <Return> {.a.b.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");
cmd(inter, "bind .a <Up> {.a.f.r1 invoke}");
cmd(inter, "bind .a <Down> {.a.f.r2 invoke}");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
while(choice==0)
 Tcl_DoOneEvent(0);


//operation cancelled
if(choice==2)
 {cmd(inter, "sblocklmm .a");
  choice=0;
  goto loop;
 }

cmd(inter, "sblocklmm .a");
cmd(inter, "set choice $temp");
if(choice==2)
{
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"New Group\"");
cmd(inter, "wm transient .a .");
cmd(inter, "label .a.tit -text \"Create a new group in group:\\n $modelgroup\"");

cmd(inter, "label .a.mname -text \"Insert new group name\"");
cmd(inter, "set mname \"New group\"");
cmd(inter, "entry .a.ename -width 30 -textvariable mname");
cmd(inter, "bind .a.ename <Return> {focus -force .a.edir; .a.edir selection range 0 end}");

cmd(inter, "label .a.mdir -text \"Insert a (non-existing) directory name\"");
cmd(inter, "set mdir \"newgroup\"");
cmd(inter, "entry .a.edir -width 30 -textvariable mdir");
cmd(inter, "bind .a.edir <Return> {focus -force .a.tdes}");

cmd(inter, "label .a.ldes -text \"Insert a short description of the group\"");
cmd(inter, "text .a.tdes -width 30 -heig 3");
cmd(inter, "bind .a.tdes <Control-e> {focus -force .a.b.ok}");

cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "button .a.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.b.ok .a.b.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.tit .a.mname .a.ename .a.mdir .a.edir .a.ldes .a.tdes");
cmd(inter, "pack .a.b");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.ename");
cmd(inter, ".a.ename selection range 0 end");


cmd(inter, "blocklmm .a");
here_newgroup:
choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);

cmd(inter, "if {[string length $mdir] == 0 || [string length $mname]==0} {set choice 2} {}");

//operation cancelled
if(choice==2)
 {cmd(inter, "sblocklmm .a");
  choice=0;
  goto loop;
 }
cmd(inter, "if {[llength [split $mdir]]>1} {set choice -1} {}");
if(choice==-1)
 {cmd(inter, "tk_messageBox -type ok -icon error -message \"Directory name must not contain spaces.\"");
  cmd(inter, "focus -force .a.edir");
  cmd(inter, ".a.edir selection range 0 end");
  goto here_newgroup;
 } 
//control for existing directory
cmd(inter, "if {[file exists $groupdir/$mdir] == 1} {tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create directory: $groupdir/$mdir\\n(possibly there is already such a directory).\\nCreation of new group aborted.\"; set choice 3} {}");
if(choice==3)
 {cmd(inter, "sblocklmm .a");
  choice=0;
  goto loop;
 } 


cmd(inter, "file mkdir $groupdir/$mdir");
cmd(inter, "cd $groupdir/$mdir");
cmd(inter, "set groupdir \"$groupdir/$mdir\"");

cmd(inter, "set f [open groupinfo.txt w]; puts -nonewline $f \"$mname\"; close $f");
cmd(inter, "set f [open description.txt w]; puts -nonewline $f \"[.a.tdes get 0.0 end]\"; close $f");
cmd(inter, "set modelgroup $modelgroup/$mname");

cmd(inter, "sblocklmm .a");
//end of creation of a new group
}
else
 cmd(inter, "cd $groupdir"); //if no group is created, move in the current group

//create a new model
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"New Model\"");
cmd(inter, "wm transient .a .");
cmd(inter, "label .a.tit -text \"Create a new model in group:\\n $modelgroup\"");

cmd(inter, "label .a.mname -text \"Insert new model name\"");
cmd(inter, "set mname \"New model\"");
cmd(inter, "entry .a.ename -width 30 -textvariable mname");
cmd(inter, "bind .a.ename <Return> {focus -force .a.ever; .a.ever selection range 0 end}");

cmd(inter, "label .a.mver -text \"Insert a version number\"");
cmd(inter, "set mver \"0.1\"");
cmd(inter, "entry .a.ever -width 30 -textvariable mver");
cmd(inter, "bind .a.ever <Return> {focus -force .a.edir; .a.edir selection range 0 end}");

cmd(inter, "label .a.mdir -text \"Insert a (non-existing) directory name\"");
cmd(inter, "set mdir \"New\"");
cmd(inter, "entry .a.edir -width 30 -textvariable mdir");
cmd(inter, "bind .a.edir <Return> {focus -force .a.b.ok}");

cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");
cmd(inter, "button .a.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.b.ok .a.b.esc -padx 10 -pady 10 -side left -fill x");

cmd(inter, "pack .a.tit .a.mname .a.ename .a.mver .a.ever .a.mdir .a.edir");
cmd(inter, "pack .a.b");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.ename");
cmd(inter, ".a.ename selection range 0 end");

loop_copy_new:
cmd(inter, "blocklmm .a");
while(choice==0)
 Tcl_DoOneEvent(0);

cmd(inter, "if {[string length $mdir] == 0 || [string length $mname]==0} {set choice 2} {}");
//operation cancelled
if(choice==2)
 {cmd(inter, "sblocklmm .a");
  choice=0;
  goto loop;
 }

cmd(inter, "if {[llength [split $mdir]]>1} {set choice -1} {}");
if(choice==-1)
 {cmd(inter, "tk_messageBox -type ok -icon error -message \"Directory name must not contain spaces.\"");
  cmd(inter, "focus -force .a.edir");
  cmd(inter, ".a.edir selection range 0 end");
  goto here_newgroup;
 } 

//control for existing directory
cmd(inter, "if {[file exists $mdir] == 1} {tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create directory: $mdir\"; set choice 3} {}");
if(choice==3)
 {choice=0;
  goto loop_copy_new;
 } 

//control for an existing model with the same name AND same version
cmd(inter, "set dir [glob *]");
cmd(inter, "set num [llength $dir]");
strcpy(str, " ");
for(i=0; i<num; i++)
 {
  sprintf(msg, "if {[file isdirectory [lindex $dir %d] ] == 1} {set curdir [lindex $dir %i]} {set curdir ___}",i, i);
  cmd(inter, msg);
  s=(char *)Tcl_GetVar(inter, "curdir",0);
  strcpy(str, s);
  if(strcmp(str,"___") && strcmp(str, "gnu") && strcmp(str, "src") && strcmp(str, "Manual") )
   {
    cmd(inter, "set ex [file exists $curdir/modelinfo.txt]");
    cmd(inter, "if { $ex == 0 } {set choice 0} {set choice 1}");
    if(choice==1)
    {
      cmd(inter, "set f [open $curdir/modelinfo.txt r]");
      cmd(inter, "set cname [gets $f]; set cver [gets $f];");
      cmd(inter, "close $f");
    } 
    else
      cmd(inter, "set cname $curdir; set cver \"0.1\"");
    cmd(inter, "set comp [string compare $cname $mname]");
    cmd(inter, "set comp1 [string compare $cver $mver]");
    cmd(inter, "if {$comp == 0 && $comp1 == 0} {set choice 3; set errdir $curdir} {}");
    cmd(inter, "if {$comp == 0 } {set choice 4; set errdir $curdir} {}");

   }

 }
if(choice==3)
 {cmd(inter, "tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create the new model '$mname' (ver. $mver) because it already exists (directory: $errdir).\"");
  choice=0;
  goto loop_copy_new;
 } 
if(choice==4)
 {
 choice=0;
 cmd(inter, "set answer [tk_messageBox -type yesno -title \"Warning\" -icon warning -message \"Warning: a model '$mname' already exists (ver. $mver). If you want the new model to inherit the same equations, data etc. of that model you should cancel this operation (choose 'No'), and use the 'Copy' command in the Model Browser. Press 'Yes' to continue creating a new (empty) model '$mname'.\"]");


  s=(char *)Tcl_GetVar(inter, "answer",0);

  cmd(inter, "if {[string compare $answer \"yes\"] == 0} {set choice 1} {set choice 0}");
  if(choice==0)
   {cmd(inter, "sblocklmm .a");
    goto loop;
   } 
 } 
 

//here we are: create a new empty model

cmd(inter, "set dirname $groupdir/$mdir");
cmd(inter, "set modeldir $groupdir/$mdir");
cmd(inter, "set modelname $mname");
cmd(inter, "set version $mver");
cmd(inter, ".f.hea.mod.dat conf -text \"$modelname\"");
cmd(inter, ".f.hea.ver.dat conf -text \"$version\"");
cmd(inter, ".f.hea.grp.dat conf -text \"$modelgroup\"");
cmd(inter, "sblocklmm .a");


cmd(inter, "file mkdir $dirname");


if(macro==1)
 cmd(inter, "set fun_base \"fun_baseMacro.cpp\"");
else 
 cmd(inter, "set fun_base \"fun_baseC.cpp\"");

cmd(inter, "file copy $RootLsd/$LsdSrc/$fun_base $modeldir"); 
 

cmd(inter, "cd $dirname");


//insert the empty equation file


cmd(inter, ".f.t.t delete 0.0 end");
cmd(inter, "set file [open $fun_base]");
cmd(inter, ".f.t.t insert end [read -nonewline $file]");
cmd(inter, "close $file");
cmd(inter, ".f.t.t edit reset");
cmd(inter, "cd $modeldir");
cmd(inter, "file delete $fun_base");
cmd(inter, "set f [open fun_$mdir.cpp w]");
cmd(inter, "puts -nonewline $f [.f.t.t get 0.0 end]");
cmd(inter, "close $f");
cmd(inter, "set filename fun_$mdir.cpp");
cmd(inter, ".f.hea.file.dat conf -text \"$filename\"");

//create the model_options.txt file
cmd(inter, "set dir [glob *.cpp]");
cmd(inter, "set b [lindex $dir 0]");
cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\nSWITCH_CC_LNK=\\n\"");
cmd(inter, "set f [open model_options.txt w]");
cmd(inter, "puts $f $a");
cmd(inter, "close $f");


//insert the modelinfo file, 
cmd(inter, "set before [.f.t.t get 1.0 end]");
cmd(inter, "set f [open modelinfo.txt w]");
cmd(inter, "puts $f \"$modelname\"");
cmd(inter, "puts $f \"$version\"");
cmd(inter, "set frmt \"%d %B, %Y\"");
cmd(inter, "puts $f \"[clock format [clock seconds] -format \"$frmt\"]\"");
cmd(inter, "close $f");
//cmd(inter, "if { $tcl_platform(platform) == \"windows\" } {file copy $RootLsd/$LsdGnu/bin/crt0.o $modeldir} {}");

cmd(inter, "cd $RootLsd");
cmd(inter, ".m.model entryconf 3 -state normal");
cmd(inter, ".m.model entryconf 4 -state normal");
cmd(inter, ".m.model entryconf 5 -state normal");
cmd(inter, ".m.model entryconf 6 -state normal");
cmd(inter, ".m.model entryconf 7 -state normal");
cmd(inter, ".m.model entryconf 9 -state normal");
cmd(inter, ".m.model entryconf 10 -state normal");
cmd(inter, ".m.model entryconf 11 -state normal");
cmd(inter, ".m.model entryconf 12 -state normal");
cmd(inter, ".m.model entryconf 14 -state normal");


cmd(inter, "tk_messageBox -type ok -title \"Model created\" -icon info -message \"New model '$mname' (ver. $mver) successfully created (directory: $dirname).\"");

choice=49;
goto loop;

}


if(choice==15)
{
/* open a text file*/

//cmd(inter, "set tk_strictMotif 0; set brr [tk_getOpenFile -initialdir $dirname]; set tk_strictMotif 1");
cmd(inter, "set brr [tk_getOpenFile -initialdir $dirname]");
cmd(inter, "if {[string length $brr] == 0} {set choice 0} {set choice 1}");
if(choice==0)
 goto loop;

cmd(inter, "if {[file exists $brr] == 1} {set choice 1} {set choice 0}");
if(choice==1)
 {
  cmd(inter, ".f.t.t delete 1.0 end; set file [open $brr]; .f.t.t insert end [read -nonewline $file]; .f.t.t edit reset; close $file; set before [.f.t.t get 1.0 end]; set dirname [file dirname $brr]; set filename [file tail $brr]; .f.hea.file.dat conf -text \"$filename\"");
    cmd(inter, "wm title . \"$filename - LMM\"");          

 }
else
 goto loop; 

cmd(inter, ".f.t.t mark set insert 1.0");
cmd(inter, "set s [file extension $filename]");
cmd(inter, "catch [unset -nocomplain ud]");
cmd(inter, "catch [unset -nocomplain udi]");
cmd(inter, "catch [unset -nocomplain rd]");
cmd(inter, "catch [unset -nocomplain rdi]");
cmd(inter, "lappend ud [.f.t.t get 0.0 end]");
cmd(inter, "lappend udi [.f.t.t index insert]");

s=(char *)Tcl_GetVar(inter, "s",0);
choice=0;
if(s[0]!='\0')
 {strcpy(str, s);
  if(!strcmp(str, ".cpp") || !strcmp(str, ".c") || !strcmp(str, ".C") || !strcmp(str, ".CPP") || !strcmp(str, ".Cpp") || !strcmp(str, ".c++") || !strcmp(str, ".C++") || !strcmp(str, ".h") || !strcmp(str, ".H"))
   {sourcefile=1;
//    cmd(inter, "set inicolor \"1.0\"");
//    cmd(inter, "set endcolor [.f.t.t index end]");
//    color(&num);
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
cmd(inter, "set in [.f.t.t index insert]");
cmd(inter, "scan $in %d.%d line col");
cmd(inter, "set line [expr $line -1]");
cmd(inter, "set s [.f.t.t get $line.0 $line.end]");
s=(char *)Tcl_GetVar(inter, "s",0);
for(i=0; s[i]==' '; i++)
  str[i]=' ';
if(i>0)
{str[i]=(char)NULL;
 sprintf(msg, ".f.t.t insert insert \"%s\"", str);
 cmd(inter, msg);
}
choice=0;
goto loop;

}

if(choice==17)
{
/* find matching parenthesis */

cmd(inter, "set sym [.f.t.t get insert]");
cmd(inter, "if {[string compare $sym \"\\{\"]!=0} {if {[string compare $sym \"\\}\"]!=0} {set choice 0} {set num -1; set direction -backwards; set fsign +; set sign \"\"; set terminal \"1.0\"}} {set num 1; set direction -forwards; set fsign -; set sign +; set terminal end}");

if(choice==0)
 goto loop;
cmd(inter, "set cur [.f.t.t index insert]");
cmd(inter, ".f.t.t tag add sel $cur \"$cur + 1char\"");
if (num>0)
  cmd(inter, "set cur [.f.t.t index \"insert + 1 char\"]");
while(num!=0 && choice!=0)
{
 cmd(inter, "set a [.f.t.t search $direction \"\\{\" $cur $terminal]");
 cmd(inter, "if {$a==\"\"} {set a [.f.t.t index $terminal]} {}");
 cmd(inter, "set b [.f.t.t search $direction \"\\}\" $cur $terminal]");
// cmd(inter, ".f.t.t insert end \"$a $b $cur\\n\"");
 cmd(inter, "if {$b==\"\"} {set b [.f.t.t index $terminal]} {}");
 cmd(inter, "if {$a==$b} {set choice 0} {}");
 if(choice==0)
  goto loop;
 if(num>0)
   cmd(inter, "if {[.f.t.t compare $a < $b]} {set num [expr $num + 1]; set cur [.f.t.t index \"$a+1char\"]} {set num [expr $num - 1]; set cur [.f.t.t index \"$b+1char\"]}");
 else
   cmd(inter, "if {[.f.t.t compare $a > $b]} {set num [expr $num + 1]; set cur [.f.t.t index $a]} {set num [expr $num - 1]; set cur [.f.t.t index $b]}");


}
choice=0;


cmd(inter, " if { [string compare $sign \"+\"]==0 } {.f.t.t tag add sel \"$cur - 1char\" $cur ; set num 1} {.f.t.t tag add sel $cur \"$cur + 1char\"; set num 0}");

cmd(inter, ".f.t.t see $cur");
goto loop;
}


/*
// It seems it's not used anymore
if(choice==19)
{
choice=0;
if(sourcefile==1)
  color(&num);

goto loop;
}



// It seems it's not used anymore
if(choice==22)
{
//reset the coloring around the insertion point after a deletion
if(sourcefile==0)
 {choice=0;
  goto loop;
 }
cmd(inter, "set a [lindex [.f.t.t nextrange sel 1.0] 0]");
cmd(inter, "if {$a==\"\"} {set choice 1} {set choice 0}");
if(choice==0)
 goto loop;

cmd(inter, ".f.t.t tag remove str 1.0 end");
cmd(inter, ".f.t.t tag remove comment1 1.0 end");
cmd(inter, ".f.t.t tag remove comment2 1.0 end");

cmd(inter, "set inicolor 1.0");
cmd(inter, "set endcolor end");
color(&num);
choice=0;
choice=0;
goto loop;
}

*/
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
  cmd(inter, "if {[lsearch -exact [.f.t.t tag names $curPosIni] comment1] != -1} {set curPosFin [.f.t.t search */ $curPosIni end]; set newPosIni [.f.t.t search -backwards /* $curPosIni 1.0]; set curPosIni $newPosIni} {if {[.f.t.t search -backwards */ $curPosIni \"$curPosIni linestart\"] != \"\"} {set comIni [.f.t.t search -backwards /* $curPosIni]; if {$comIni != \"\"} {set curPosIni $comIni}}; if {[.f.t.t search /* $curPosFin \"$curPosFin lineend\"] != \"\"} {set comFin [.f.t.t search */ $curPosFin]; if {$comFin != \"\"} {set curPosFin $comFin}}}");
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
  
/*  
  cmd(inter, "set inicolor [.f.t.t search -backwards * / [.f.t.t index insert] 1.0]");
  cmd(inter, "if {$inicolor==\"\"} {set inicolor 1.0} {set inicolor [.f.t.t index \"$inicolor + 2 char\"]}");
  cmd(inter, "set endcolor [.f.t.t search /* [.f.t.t index insert] end]");
  cmd(inter, "if {$endcolor==\"\"} {set endcolor end} {}");
   
  cmd(inter, ".f.t.t tag remove str $inicolor $endcolor");
  cmd(inter, ".f.t.t tag remove comment1 $inicolor $endcolor");
  cmd(inter, ".f.t.t tag remove comment2 $inicolor $endcolor");

//cmd(inter, "set inicolor 1.0");
//cmd(inter, "set endcolor end");
*/

//choice=0;
//color(&num);
color(shigh, prevLin, nextLin);
}
goto loop;

}

if(choice==21)
{
/* Find and replace a text pattern in the text*/
cmd(inter, "if {[winfo exists .l]==1} {set choice 0; focus -force .l.e} {}");
if(choice==0)
 goto loop;


cmd(inter, "set cur \"\"");
cmd(inter, "if { [winfo exists .l]==1} {.l.m.file invoke 1} { }");
cmd(inter, "toplevel .l");
cmd(inter, "wm protocol .l WM_DELETE_WINDOW { }");
cmd(inter, "wm transient .l .");
cmd(inter, "wm title .l \"Replace Text\"");
//cmd(inter, "set textsearch \"\"");
cmd(inter, "label .l.l -text \"Type the text to search\"");
cmd(inter, "entry .l.e -width 30 -textvariable textsearch");
cmd(inter, "label .l.r -text \"Type the text to replace\"");
cmd(inter, "entry .l.s -width 30 -textvariable textrepl");

cmd(inter, "radiobutton .l.r1 -text \"Down\" -variable dirsearch -value \"-forwards\" -command {set endsearch end}");
cmd(inter, "radiobutton .l.r2 -text \"Up\" -variable dirsearch -value \"-backwards\" -command {set endsearch 1.0}" );
cmd(inter, "checkbutton .l.c -text \"Case Sensitive\" -variable docase");

cmd(inter, "frame .l.b1");
cmd(inter, "button .l.b1.repl -padx 12 -state disabled -text Replace -command {if {[string length $cur] > 0} {.f.t.t delete $cur \"$cur + $length char\"; .f.t.t insert $cur \"$textrepl\"; if {[string compare $endsearch end]==0} {} {.f.t.t mark set insert $cur}; .l.ok invoke} {}}");
cmd(inter, "button .l.b1.all -padx 11 -state disabled -text \"Repl. All\" -command {set choice 4}");
cmd(inter, "frame .l.b2");
cmd(inter, "button .l.b2.ok -padx 15 -text Search -command {if { [string length \"$textsearch\"]==0} {} {.f.t.t tag remove found 1.0 end; if {$docase==1} {set case \"-exact\"} {set case -nocase}; .f.t.t tag remove sel 1.0 end; set cur [.f.t.t index insert]; set cur [.f.t.t search $dirsearch -count length $case -- $textsearch $cur $endsearch]; if {[string length $cur] > 0} {.f.t.t tag add found $cur \"$cur + $length char\"; if {[string compare $endsearch end]==0} {.f.t.t mark set insert \"$cur + $length char\" } {.f.t.t mark set insert $cur}; update; .f.t.t see $cur; .l.b1.repl conf -state normal; .l.b1.all conf -state normal} {.l.b1.all conf -state disabled; .l.b1.repl conf -state disabled}}}");
cmd(inter, "button .l.b2.esc -padx 15 -text Cancel -command {focus -force .f.t.t; set choice 5}");
cmd(inter, "bind .l <KeyPress-Return> {.l.b2.ok invoke}");
cmd(inter, "bind .l <KeyPress-Escape> {.l.b2.esc invoke}");

cmd(inter, "pack .l.b1.repl .l.b1.all -padx 10 -pady 5 -side left");
cmd(inter, "pack .l.b2.ok .l.b2.esc -padx 10 -pady 5 -side left");
cmd(inter, "pack .l.l .l.e .l.r .l.s .l.r1 .l.r2 .l.c");
cmd(inter, "pack .l.b1 .l.b2");

cmd( inter, "setgeom .l" );
cmd(inter, "focus -force .l.e");
cmd(inter, ".f.t.t tag conf found -background red -foreground white");
cmd(inter, "blocklmm .l");
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
  cmd(inter, "if {[string length $cur] > 0} {.f.t.t delete $cur \"$cur + $length char\"; .f.t.t see $cur; .f.t.t insert $cur \"$textrepl\"; if {[string compare $endsearch end]==0} {set cur [.f.t.t index \"$cur+$length char\"]} {}} {set choice 0}");
if(choice!=0)  
  cmd(inter, "set cur [.f.t.t search $dirsearch -count length $case -- $textsearch $cur $endsearch]"); 
  cmd(inter, "raise .l");
  cmd(inter, "focus -force .l");
  Tcl_DoOneEvent(0);  
  goto here;
 }

cmd(inter, "sblocklmm .l");
cmd(inter, "focus -force .f.t.t");
cmd(inter, "destroy .l");
cmd(inter, ".f.t.t tag remove found 1.0 end");
color(shigh, 0, 0);				// reevaluate colors
choice=0;
goto loop;
}


if(choice==25 && macro==1)
{
/*
Insert a Lsd equation
*/

choice=0;

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert an Equation\"");

cmd(inter, "label .a.l1 -text \"Type below the label of the Variable\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.b.ok}");

cmd(inter, "frame .a.f -relief groove -bd 2");

cmd(inter, "set isfun 0");
cmd(inter, "radiobutton .a.f.r1 -variable isfun -value 0 -text \"Equation:\\nthe code will provide one value for every Variable of this type at each time step.\\nThe code is executed only once for each t, re-using the same value if requested many times.\\nEven if no other Variable requests this value, it the code is computed.\" -justify left -relief groove");
cmd(inter, "radiobutton .a.f.r2 -variable isfun -value 1 -text \"Function:\\nthe code is executed again any time this Variable is requested by other Variables' code,\\nbut it is not computed by default. This code is not computed if no Variable\\nneeds this Variable's value.\" -justify left -relief groove");
cmd(inter, "pack .a.f.r1 .a.f.r2 -anchor w ");


cmd(inter, "frame .a.b");	
cmd(inter, "button .a.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
cmd(inter, "button .a.b.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#equation}");
cmd(inter, "button .a.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");
cmd(inter, "pack .a.b.ok .a.b.help .a.b.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.label .a.f");
cmd(inter, "pack .a.b");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.label");
cmd(inter, ".a.label selection range 0 end");
cmd(inter, "blocklmm .a");
while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd(inter, "if {$isfun == 1} {set choice 3} {}");

cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

if(choice!=3)
  cmd(inter, ".f.t.t insert insert \"EQUATION(\\\"$v_label\\\")\\n\"");
else
  cmd(inter, ".f.t.t insert insert \"FUNCTION(\\\"$v_label\\\")\\n\"");

cmd(inter, ".f.t.t insert insert \"/*\\nComment\\n*/\\n\\n\"");
cmd(inter, ".f.t.t insert insert \"RESULT( )\\n\"");
cmd(inter, ".f.t.t mark set insert \"$a + 2 line\"");
cmd(inter, ".f.t.t tag add sel insert \"insert + 7 char\"");

v_counter=0;
cmd(inter, "set v_num 0");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==25 && macro==0)
{
/*
Insert a Lsd equation
*/
choice=0;


cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert an Equation\"");

cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Type below the label of the Variable\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.b.ok}");

cmd(inter, "frame .a.f -relief groove -bd 2");

cmd(inter, "set isfun 0");
cmd(inter, "radiobutton .a.f.r1 -variable isfun -value 0 -text \"Equation:\\nthe code will provide one value for every Variable of this type at each time step.\\nThe code is executed only once for each t, re-using the same value if requested many times.\\nEven if no other Variable requests this value, it the code is computed.\" -justify left -relief groove");
cmd(inter, "radiobutton .a.f.r2 -variable isfun -value 1 -text \"Function:\\nthe code is executed again any time this Variable is requested by other Variables' code,\\nbut it is not computed by default. This code is not computed if no Variable\\nneeds this Variable's value.\" -justify left -relief groove");
cmd(inter, "pack .a.f.r1 .a.f.r2 -anchor w ");


cmd(inter, "frame .a.b");	
cmd(inter, "button .a.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
cmd(inter, "button .a.b.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#equation}");
cmd(inter, "button .a.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");
cmd(inter, "pack .a.b.ok .a.b.help .a.b.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.label .a.f");
cmd(inter, "pack .a.b");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.label");
cmd(inter, ".a.label selection range 0 end");
cmd(inter, "blocklmm .a");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd(inter, "if {$isfun == 1} {set choice 3} {}");

cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, ".f.t.t insert insert \"if(!strcmp(label,\\\"$v_label\\\"))\\n\"");
cmd(inter, ".f.t.t insert insert \"{\\n\"");
cmd(inter, ".f.t.t insert insert \"/*\\nComment\\n*/\\n\"");
if(choice==3)
 {
 cmd(inter, ".f.t.t insert insert \"last_update--;//repeat the computation any time is requested\\n\"");
 cmd(inter, ".f.t.t insert insert \"if(c==NULL)//Avoids to be computed when the system activates the equation\\n\"");
 cmd(inter, ".f.t.t insert insert \"{\\n\"");
 cmd(inter, ".f.t.t insert insert \"res=-1;\\n\"");
 cmd(inter, ".f.t.t insert insert \"goto end;\\n\"");
 cmd(inter, ".f.t.t insert insert \"}\\n\"");
 }
cmd(inter, ".f.t.t insert insert \"\\n\"");
cmd(inter, ".f.t.t insert insert \"res=;\\n\"");
cmd(inter, ".f.t.t insert insert \"goto end;\\n}\\n\"");
cmd(inter, ".f.t.t mark set insert \"$a + 3 line\"");
cmd(inter, ".f.t.t tag add sel insert \"insert + 7 char\"");

v_counter=0;
cmd(inter, "set v_num 0");
cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==26 && macro==1)
{
/*
Insert a v[0]=p->cal("Var",0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'V(...)' Command\"");


cmd(inter, "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result\"");

sprintf(msg, "set v_num %d", v_counter);
cmd(inter, msg);
cmd(inter, "entry .a.v_num -width 2 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the variable to compute\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the lag to be used\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object to which request the computation\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#V}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");
cmd(inter, "blocklmm .a");
while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
 cmd(inter, "set a [.f.t.t index insert]");

//cmd(inter, "if {$v_num==\"\"} {.f.t.t insert insert \"$v_obj->cal(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->cal(\\\"$v_label\\\",$v_lag);\"; incr v_num}");
cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}");

cmd(inter, "if {$v_lag==0 && $v_obj==\"p\"} { .f.t.t insert insert \"V(\\\"$v_label\\\")\"} {}");
cmd(inter, "if {$v_lag!=0 && $v_obj==\"p\"} { .f.t.t insert insert \"VL(\\\"$v_label\\\",$v_lag)\"} {}");
cmd(inter, "if {$v_lag==0 && $v_obj!=\"p\"} { .f.t.t insert insert \"VS($v_obj,\\\"$v_label\\\")\"} {}");
cmd(inter, "if {$v_lag!=0 && $v_obj!=\"p\"} { .f.t.t insert insert \"VLS($v_obj,\\\"$v_label\\\",$v_lag)\"} {}");

//cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \";\\n\"} {}"); //no new line character
cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \";\"} {}");

cmd(inter, "if {$v_num==\"\"} { set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==26 && macro==0)
{
/*
Insert a v[0]=p->cal("Var",0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a 'cal'\"");
cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result\"");

sprintf(msg, "set v_num %d", v_counter);
cmd(inter, msg);
cmd(inter, "entry .a.v_num -width 2 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the variable to compute\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the lag to be used\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object to which request the computation\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#cal}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, "if {$v_num==\"\"} {.f.t.t insert insert \"$v_obj->cal(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->cal(\\\"$v_label\\\",$v_lag);\"; incr v_num}");
//cmd(inter, ".f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->cal(\\\"$v_label\\\",$v_lag);\"");
//cmd(inter, ".f.t.t mark set insert \"$a + 1 line\"");


cmd(inter, "if {$v_num==\"\"} { set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=num;

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==27 && macro==1)
{
/*
Insert a cycle for(cur=p->search("Label"); cur!=NULL; cur=go_brother(cur))

*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a 'CYCLE' Command\"");

cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Type below the label of the Object to cycle through\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.obj; .a.obj selection range 0 end}");


cmd(inter, "label .a.l2 -text \"Type below the cycling pointer\"");
cmd(inter, "set v_obj cur");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.par; .a.par selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the parent pointer\"");
cmd(inter, "set v_par p");
cmd(inter, "entry .a.par -width 6 -textvariable v_par");
cmd(inter, "bind .a.par <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#CYCLE}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.label .a.l2 .a.obj .a.l3 .a.par");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.label");
cmd(inter, ".a.label selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

//cmd(inter, ".f.t.t insert insert \"for($v_obj=$v_par->search(\\\"$v_label\\\"); $v_obj!=NULL; $v_obj=go_brother($v_obj) )\\n\"");
cmd(inter, "if { $v_par == \"p\"} {.f.t.t insert insert \"CYCLE($v_obj, \\\"$v_label\\\")\\n\"} {.f.t.t insert insert \"CYCLES($v_par, $v_obj, \\\"$v_label\\\")\\n\"}");
//cmd(inter, ".f.t.t insert insert \" {\\n  \\n }\\n\""); 
//Trying making automatic indent in cycles

cmd(inter, "set in [.f.t.t index insert]");
cmd(inter, "scan $in %d.%d line col");
cmd(inter, "set line [expr $line -1]");
cmd(inter, "set s [.f.t.t get $line.0 $line.end]");
s=(char *)Tcl_GetVar(inter, "s",0);
for(i=0; s[i]==' '; i++)
  str[i]=' ';
str[i]=' ';  
i++;  
if(i>0)
{str[i]=(char)NULL;
 sprintf(msg, ".f.t.t insert insert \"%s\"", str);
 cmd(inter, msg);
}
cmd(inter, ".f.t.t insert insert \"{\\n\"");

if(i>0)
{str[i]=(char)NULL;
 sprintf(msg, ".f.t.t insert insert \"%s \"", str);
 cmd(inter, msg);
 cmd(inter, "set b [.f.t.t index insert]");
}

if(i>0)
{str[i]=(char)NULL;
 sprintf(msg, ".f.t.t insert insert \"\\n%s\"", str);
 cmd(inter, msg);
}

cmd(inter, ".f.t.t insert insert \"}\\n\"");

//sprintf(msg, ".f.t.t mark set insert \"$a + 1 lines + %d char\"",i+2);
//cmd(inter, msg);
cmd(inter, ".f.t.t mark set insert \"$b\"");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;

}

if(choice==27 && macro==0)
{
/*
Insert a cycle for(cur=p->search("Label"); cur!=NULL; cur=go_brother(cur))

*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a 'for' Cycle\"");

cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Type below the label of the Object to cycle through\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.obj; .a.obj selection range 0 end}");


cmd(inter, "label .a.l2 -text \"Type below the cycling pointer\"");
cmd(inter, "set v_obj cur");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.par; .a.par selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the parent pointer\"");
cmd(inter, "set v_par p");
cmd(inter, "entry .a.par -width 6 -textvariable v_par");
cmd(inter, "bind .a.par <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#basicc}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.label .a.l2 .a.obj .a.l3 .a.par");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.label");
cmd(inter, ".a.label selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, ".f.t.t insert insert \"for($v_obj=$v_par->search(\\\"$v_label\\\"); $v_obj!=NULL; $v_obj=go_brother($v_obj) )\\n\"");
cmd(inter, ".f.t.t insert insert \" {\\n  \\n }\\n\"");
cmd(inter, ".f.t.t mark set insert \"$a + 2 line + 2 char\"");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;

}


if(choice==28 && macro==1)
{
/*
MACRO VERSION
Insert a Lsd script, to be used in Lsd equations' code
*/


choice=0;

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a Lsd Script\"");

cmd(inter, "wm transient .a .");
cmd(inter, "set res 26");
cmd(inter, "label .a.tit -text \"Choose one of the following options\nThe interface will request the necessary information\" -justify center");
cmd(inter, "frame .a.r -bd 2 -relief groove");
cmd(inter, "radiobutton .a.r.equ -text \"EQUATION/FUNCTION - insert a new Lsd equation\" -underline 0 -variable res -value 25");
cmd(inter, "radiobutton .a.r.cal -text \"V(...) - request the value of a Variable\" -underline 0 -variable res -value 26");
cmd(inter, "radiobutton .a.r.sum -text \"SUM - compute the sum of a Variable over a set of Objects\" -underline 1 -variable res -value 56");
cmd(inter, "radiobutton .a.r.sear -text \"SEARCH - search the first instance an Object type\" -underline 2 -variable res -value 55");
cmd(inter, "radiobutton .a.r.scnd -text \"SEARCH_CND - conditional search a specific Object in the model\" -underline 0 -variable res -value 30");
cmd(inter, "radiobutton .a.r.lqs -text \"SORT - sort a group of Objects\" -underline 3 -variable res -value 31");
cmd(inter, "radiobutton .a.r.addo -text \"ADDOBJ - add a new Object\" -underline 3 -variable res -value 52");
cmd(inter, "radiobutton .a.r.delo -text \"DELETE - delete an Object\" -underline 0 -variable res -value 53");
cmd(inter, "radiobutton .a.r.rndo -text \"RNDDRAW - draw an Object\" -underline 1 -variable res -value 54");
cmd(inter, "radiobutton .a.r.wri -text \"WRITE - overwrite a Variable or Parameter with a new value\" -underline 0 -variable res -value 29");
cmd(inter, "radiobutton .a.r.incr -text \"INCR - increment the value of a Parameter\" -underline 0 -variable res -value 40");
cmd(inter, "radiobutton .a.r.mult -text \"MULT - multiply the value of a Parameter\" -underline 0 -variable res -value 45");
cmd(inter, "radiobutton .a.r.for -text \"CYCLE - insert a cycle over a group of Objects\" -underline 0 -variable res -value 27");
cmd(inter, "radiobutton .a.r.math -text \"Insert a mathematical/statistical function\" -underline 12 -variable res -value 51");


cmd(inter, "bind .a <KeyPress-e> {.a.r.equ invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-v> {.a.r.cal invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-u> {.a.r.sum invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-s> {.a.r.scnd invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-a> {.a.r.sear invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-w> {.a.r.wri invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-c> {.a.r.for invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-t> {.a.r.lqs invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-o> {.a.r.addo invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-d> {.a.r.delo invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-i> {.a.r.incr invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-m> {.a.r.mult invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-h> {.a.r.math invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-n> {.a.r.rndo invoke; set choice 1}");

cmd(inter, "frame .a.f");
cmd(inter, "button .a.f.ok -padx 18 -text Insert -command {set choice 1}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp LMM_help.html#LsdScript}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");

cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.r.equ .a.r.cal .a.r.sum .a.r.sear .a.r.scnd .a.r.lqs .a.r.addo .a.r.delo .a.r.rndo .a.r.wri .a.r.incr .a.r.mult .a.r.for .a.r.math -anchor w");
cmd(inter, "pack .a.tit .a.r .a.f");
cmd(inter, "bind .a <Return> {.a.f.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.r.cal");


//cmd(inter, "bind . <Button-1> {raise .a; focus -force .a}");

cmd(inter, "blocklmm .a");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }

cmd(inter, "set choice $res");
goto loop;

}

if(choice==28 && macro==0)
{
/*
C++ VERSION
Insert a Lsd script, to be used in Lsd equations' code
*/


choice=0;

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a Lsd Script\"");

cmd(inter, "wm transient .a .");
cmd(inter, "set res 26");
cmd(inter, "label .a.tit -text \"Choose one of the following options. The interface will request the necessary information\" -justify center");
cmd(inter, "frame .a.r -bd 2 -relief groove");
cmd(inter, "radiobutton .a.r.equ -text \"EQUATION/FUNCTION - insert a new Lsd equation\" -underline 0 -variable res -value 25");
cmd(inter, "radiobutton .a.r.cal -text \"cal - request the value of a Variable\" -underline 0 -variable res -value 26");
cmd(inter, "radiobutton .a.r.sum -text \"sum - compute the sum of a Variable over a set of Objects\" -underline 1 -variable res -value 56");
cmd(inter, "radiobutton .a.r.sear -text \"search - search the first instance an Object type\" -underline 3 -variable res -value 55");
cmd(inter, "radiobutton .a.r.scnd -text \"search_var_cond - conditional search a specific Object in the model\" -underline 0 -variable res -value 30");
cmd(inter, "radiobutton .a.r.lqs -text \"lsdqsort - sort a group of Objects\" -underline 7 -variable res -value 31");
cmd(inter, "radiobutton .a.r.addo -text \"add_an_object - add a new Object\" -underline 0 -variable res -value 52");
cmd(inter, "radiobutton .a.r.delo -text \"delete_obj - delete an Object\" -underline 0 -variable res -value 53");
cmd(inter, "radiobutton .a.r.rndo -text \"draw_rnd - draw an Object\" -underline 6 -variable res -value 54");
cmd(inter, "radiobutton .a.r.wri -text \"write - overwrite a Variable or Parameter with a new value\" -underline 0 -variable res -value 29");
cmd(inter, "radiobutton .a.r.incr -text \"increment - increment the value of a Parameter\" -underline 0 -variable res -value 40");
cmd(inter, "radiobutton .a.r.mult -text \"multiply - multiply the value of a Parameter\" -underline 0 -variable res -value 45");
cmd(inter, "radiobutton .a.r.for -text \"for - insert a cycle over a group of Objects\" -underline 0 -variable res -value 27");
cmd(inter, "radiobutton .a.r.math -text \"Insert a mathematical/statistical function\" -underline 12 -variable res -value 51");


cmd(inter, "bind .a <KeyPress-e> {.a.r.equ invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-c> {.a.r.cal invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-u> {.a.r.sum invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-s> {.a.r.scnd invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-r> {.a.r.sear invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-w> {.a.r.wri invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-f> {.a.r.for invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-t> {.a.r.lqs invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-a> {.a.r.addo invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-d> {.a.r.delo invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-i> {.a.r.incr invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-m> {.a.r.mult invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-h> {.a.r.math invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-n> {.a.r.rndo invoke; set choice 1}");

cmd(inter, "frame .a.f");
cmd(inter, "button .a.f.ok -padx 18 -text Insert -command {set choice 1}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp LMM_help.html#LsdScript}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");

cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.r.equ .a.r.cal .a.r.sum .a.r.sear .a.r.scnd .a.r.lqs .a.r.addo .a.r.delo .a.r.rndo .a.r.wri .a.r.incr .a.r.mult .a.r.for .a.r.math -anchor w");
cmd(inter, "pack .a.tit .a.r .a.f");
cmd(inter, "bind .a <Return> {.a.f.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.r.cal");


//cmd(inter, "bind . <Button-1> {raise .a; focus -force .a}");


cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.f.ok");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 { 
  choice=0;

  goto loop;
 }


cmd(inter, "set choice $res");
goto loop;

}



if(choice==40 && macro==1)
{
/*
Insert a v[0]=p->increment("Var",0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert an 'INCR' Command\"");

cmd(inter, "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result after the increment\"");

sprintf(msg, "set v_num %d", v_counter);
cmd(inter, msg);

cmd(inter, "entry .a.v_num -width 2 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the variable to increase\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.val; .a.val selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the value to add\"");
cmd(inter, "set v_val 1");
cmd(inter, "entry .a.val -width 15 -textvariable v_val");
cmd(inter, "bind .a.val <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object containing the variable to increment\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#INCR}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, "if {$v_num==\"\" } {.f.t.t insert insert \"$v_obj->increment(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->increment(\\\"$v_label\\\",$v_val);\";incr v_num}");
cmd(inter, "if {$v_num!=\"\" } {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}");
cmd(inter, "if {$v_obj!=\"p\" } {.f.t.t insert insert \"INCRS($v_obj,\\\"$v_label\\\",$v_val)\"} {.f.t.t insert insert \"INCR(\\\"$v_label\\\",$v_val)\"}");
cmd(inter, ".f.t.t insert insert \";\\n\"");

cmd(inter, "if {$v_num==\"\" } {set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "unset -nocomplain v_num");
cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==40 && macro==0)
{
/*
Insert a v[0]=p->increment("Var",0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert an 'increment' Command\"");

cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result after the increment\"");

sprintf(msg, "set v_num %d", v_counter);
cmd(inter, msg);

cmd(inter, "entry .a.v_num -width 2 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the variable to increase\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.val; .a.val selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the value to add\"");
cmd(inter, "set v_val 0");
cmd(inter, "entry .a.val -width 15 -textvariable v_val");
cmd(inter, "bind .a.val <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object containing the variable to increment\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#increment}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, "if {$v_num==\"\" } {.f.t.t insert insert \"$v_obj->increment(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->increment(\\\"$v_label\\\",$v_val);\";incr v_num}");



cmd(inter, "if {$v_num==\"\" } {set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "unset v_num");
cmd(inter, "savCurIni");	// save data for recolor
goto loop;
}


if(choice==45 && macro==1)
{
/*
Insert a v[0]=p->multiply("Var",0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'MULT' Command\"");

cmd(inter, "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result after the multiplication\"");

sprintf(msg, "set v_num %d", v_counter);
cmd(inter, msg);

cmd(inter, "entry .a.v_num -width 2 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the variable to multiply\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.val; .a.val selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the value to multiply\"");
cmd(inter, "set v_val 0");
cmd(inter, "entry .a.val -width 15 -textvariable v_val");
cmd(inter, "bind .a.val <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object containing the variable to multiply\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");

	
cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#MULT}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, "if {$v_num==\"\" } {.f.t.t insert insert \"$v_obj->multiply(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->multiply(\\\"$v_label\\\",$v_val);\";incr v_num}");

cmd(inter, "if {$v_num!=\"\" } {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}");
cmd(inter, "if {$v_obj!=\"p\" } {.f.t.t insert insert \"MULTS($v_obj,\\\"$v_label\\\",$v_val)\"} {.f.t.t insert insert \"MULT(\\\"$v_label\\\",$v_val)\"}");
cmd(inter, ".f.t.t insert insert \";\\n\"");


cmd(inter, "if {$v_num==\"\" } {set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "unset v_num");
cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==45 && macro==0)
{
/*
Insert a v[0]=p->multiply("Var",0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a 'multiply' Command\"");

cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result after the multiplication\"");

sprintf(msg, "set v_num %d", v_counter);
cmd(inter, msg);

cmd(inter, "entry .a.v_num -width 2 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the variable to multiply\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.val; .a.val selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the value to multiply\"");
cmd(inter, "set v_val 0");
cmd(inter, "entry .a.val -width 15 -textvariable v_val");
cmd(inter, "bind .a.val <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object containing the variable to multiply\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");

	
cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#multiply}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, "if {$v_num==\"\" } {.f.t.t insert insert \"$v_obj->multiply(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->multiply(\\\"$v_label\\\",$v_val);\";incr v_num}");



cmd(inter, "if {$v_num==\"\" } {set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "unset v_num");
cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==29 && macro==1)
{
/*
Insert a p->write("Var",0,0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'WRITE' Command\"");

cmd(inter, ".f.t.t conf -state disabled");

cmd(inter, "label .a.l1 -text \"Type below the value to write\"");

cmd(inter, "set v_num 0");

cmd(inter, "entry .a.v_num -width 10 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Label of the Var. or Par. to overwrite\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Time step appearing as latest computation for the new value\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Object containing the Var or Par to write\"");
cmd(inter, "if { [catch {puts $v_obj}] == 1 } {set v_obj p} {}");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#WRITE}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, ".f.t.t insert insert \"$v_obj->write(\\\"$v_label\\\",$v_num, $v_lag);\"");
//cmd(inter, ".f.t.t mark set insert \"$a + 1 line\"");
cmd(inter, "if {$v_obj ==\"p\" && $v_lag == 0 } { .f.t.t insert insert \"WRITE(\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj ==\"p\" && $v_lag != 0 } { .f.t.t insert insert \"WRITEL(\\\"$v_label\\\",$v_num, $v_lag);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_lag == 0 } { .f.t.t insert insert \"WRITES($v_obj,\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_lag != 0 } { .f.t.t insert insert \"WRITELS($v_obj,\\\"$v_label\\\",$v_num, $v_lag);\"} {}");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==29 && macro==0)
{
/*
Insert a p->write("Var",0,0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a 'write' Command\"");

cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Type below the value to write\"");

cmd(inter, "set v_num 0");

cmd(inter, "entry .a.v_num -width 10 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the Var or Par to write\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the time step appearing as latest computation for the new value\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object containing the Var or Par to write\"");
cmd(inter, "if { [catch {puts $v_obj}] == 1 } {set v_obj p} {}");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#write}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, ".f.t.t insert insert \"$v_obj->write(\\\"$v_label\\\",$v_num, $v_lag);\"");
//cmd(inter, ".f.t.t mark set insert \"$a + 1 line\"");

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==30 && macro==0)
{
/*
Insert a cur=p->search_var_cond("Var",1,0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a 'search_var_cond' Command\"");

cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l0 -text \"Type below the target pointer in which to return the Object found\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.v_num; .a.v_num selection range 0 end}");

cmd(inter, "label .a.l1 -text \"Type below the value to search for\"");
cmd(inter, "set v_num 0");
cmd(inter, "entry .a.v_num -width 10 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the Var or Par to search for\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the lag to be used\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object from which to search\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#search_var_cond}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->search_var_cond(\\\"$v_label\\\",$v_num, $v_lag);\"");

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==30 && macro==1)
{
/*
Insert a cur=p->search_var_cond("Var",1,0);
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'SEARCH_CND' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the target pointer in which to return the Object found\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.v_num; .a.v_num selection range 0 end}");

cmd(inter, "label .a.l1 -text \"Type below the value to search for\"");
cmd(inter, "set v_num 0");
cmd(inter, "entry .a.v_num -width 10 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the Var or Par to search for\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the lag to be used\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object from which to search\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#SEARCH_CND}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->search_var_cond(\\\"$v_label\\\",$v_num, $v_lag);\"");

cmd(inter, "if {$v_obj ==\"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0=SEARCH_CND(\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj ==\"p\" && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0=SEARCH_CNDL(\\\"$v_label\\\",$v_num, $v_lag);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0=SEARCH_CNDS($v_obj,\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0=SEARCH_CNDLS($v_obj,\\\"$v_label\\\",$v_num, $v_lag);\"} {}");
cmd(inter, "destroy .a");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==31 && macro==1)
{
/*
Insert a p->lsdqsort("Object", "Variable", "DIRECTION");
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'SORT' Command\"");

cmd(inter, "label .a.l1 -text \"Type below the object containing the Objects to be sorted\"");
cmd(inter, "set v_obj1 p");
cmd(inter, "entry .a.obj1 -width 10 -textvariable v_obj1");
cmd(inter, "bind .a.obj1 <Return> {focus -force .a.obj0; .a.obj0 selection range 0 end}");

cmd(inter, "label .a.l0 -text \"Type below label of the Objects to be sorted\"");
cmd(inter, "set v_obj0 ObjectName");
cmd(inter, "entry .a.obj0 -width 20 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the Var or Par whose values are to be sorted\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.r_up}");

cmd(inter, "label .a.l3 -text \"Set the sorting direction\"");
cmd(inter, "set v_direction 1");
cmd(inter, "radiobutton .a.r_up -text Increasing -variable v_direction -value 1");
cmd(inter, "radiobutton .a.r_down -text Decreasing -variable v_direction -value 2");

cmd(inter, "bind .a.l3 <Return> {focus -force .a.f.ok}");



cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#SORT}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.obj1 .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.r_up .a.r_down");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj1");
cmd(inter, ".a.obj1 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }

cmd(inter, "set choice $v_direction");
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
/*
if(choice==1)
  cmd(inter, ".f.t.t insert insert \"$v_obj1->lsdqsort(\\\"$v_obj0\\\",\\\"$v_label\\\", \\\"UP\\\");\"");
else
  cmd(inter, ".f.t.t insert insert \"$v_obj1->lsdqsort(\\\"$v_obj0\\\",\\\"$v_label\\\", \\\"DOWN\\\");\"");
********/

if(choice==1)
  cmd(inter, "set direction \"UP\"");
else
  cmd(inter, "set direction \"DOWN\"");

cmd(inter, "if {$v_obj1 ==\"p\"} {.f.t.t insert insert \"SORT(\\\"$v_obj0\\\",\\\"$v_label\\\", \\\"$direction\\\");\"} {}");
cmd(inter, "if {$v_obj1 !=\"p\"} {.f.t.t insert insert \"SORTS($v_obj1,\\\"$v_obj0\\\",\\\"$v_label\\\", \\\"$direction\\\");\"} {}");

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==31 && macro==0)
{
/*
Insert a p->lsdqsort("Object", "Variable", "DIRECTION");
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"Insert a 'lsdqsort' Command\"");

cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Type below the object containing the Objects to be sorted\"");
cmd(inter, "set v_obj1 p");
cmd(inter, "entry .a.obj1 -width 10 -textvariable v_obj1");
cmd(inter, "bind .a.obj1 <Return> {focus -force .a.obj0; .a.obj0 selection range 0 end}");

cmd(inter, "label .a.l0 -text \"Type below label of the Objects to be sorted\"");
cmd(inter, "set v_obj0 ObjectName");
cmd(inter, "entry .a.obj0 -width 20 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the Var or Par whose values are to be sorted\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.r_up}");

cmd(inter, "label .a.l3 -text \"Set the sorting direction\"");
cmd(inter, "set v_direction 1");
cmd(inter, "radiobutton .a.r_up -text Increasing -variable v_direction -value 1");
cmd(inter, "radiobutton .a.r_down -text Decreasing -variable v_direction -value 2");

cmd(inter, "bind .a.l3 <Return> {focus -force .a.f.ok}");



cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#lsdqsort}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.obj1 .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.r_up .a.r_down");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj1");
cmd(inter, ".a.obj1 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }

cmd(inter, "set choice $v_direction");
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
if(choice==1)
  cmd(inter, ".f.t.t insert insert \"$v_obj1->lsdqsort(\\\"$v_obj0\\\",\\\"$v_label\\\", \\\"UP\\\");\"");
else
  cmd(inter, ".f.t.t insert insert \"$v_obj1->lsdqsort(\\\"$v_obj0\\\",\\\"$v_label\\\", \\\"DOWN\\\");\"");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==51)
{
/*
Insert a math function
*/
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a Math Operation\"");

cmd(inter, "set value1 \"0\"; set value2 \"1\"; set res 1; set str \"UNIFORM($value1,$value2)\"");
cmd(inter, "label .a.l1 -text \"Minimum\"");
cmd(inter, "entry .a.e1 -justify center -textvariable value1");
//cmd(inter, "a.e1 selection range 0 end");
cmd(inter, "label .a.l2 -text \"Maximum\"");
cmd(inter, "entry .a.e2 -justify center -textvariable value2");
cmd(inter, "pack .a.l1 .a.e1 .a.l2 .a.e2");

cmd(inter, "radiobutton .a.r1 -text \"Uniform Random Draw\" -variable res -value 1 -command {.a.l1 conf -text Minimum; .a.l2 conf -text Maximum; set str \"UNIFORM($value1,$value2)\"}");
cmd(inter, "radiobutton .a.r2 -text \"Normal Random Draw\" -variable res -value 2 -command {.a.l1 conf -text Mean; .a.l2 conf -text Variance; set str \"norm($value1,$value2)\"}");
cmd(inter, "radiobutton .a.r3 -text \"Integer Uniform Random Draw\" -variable res -value 3 -command {.a.l1 conf -text Minimum; .a.l2 conf -text Maximum; set str \"rnd_integer($value1, $value2)\"}");
cmd(inter, "radiobutton .a.r4 -text \"Poisson Draw\" -variable res -value 4 -command {.a.l1 conf -text Mean; .a.l2 conf -text (unused); set str \"poisson($value1)\"}");
cmd(inter, "radiobutton .a.r5 -text \"Gamma Random Draw\" -variable res -value 5 -command {.a.l1 conf -text Mean; .a.l2 conf -text (unused); set str \"gamma($value1)\"}");
cmd(inter, "radiobutton .a.r6 -text \"Absolute Value\" -variable res -value 6 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused); set str \"abs($value1)\"}");
cmd(inter, "radiobutton .a.r7 -text \"Minimum Value\" -variable res -value 7 -command {.a.l1 conf -text \"Value 1\"; .a.l2 conf -text \"Value 2\"; set str \"min($value1,$value2)\"}");
cmd(inter, "radiobutton .a.r8 -text \"Maximum Value\" -variable res -value 8 -command {.a.l1 conf -text \"Value 1\"; .a.l2 conf -text \"Value 2\"; set str \"max($value1,$value2)\"}");
cmd(inter, "radiobutton .a.r9 -text \"Round closest integer\" -variable res -value 9 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused); set str \"round($value1)\"}");
cmd(inter, "radiobutton .a.r10 -text \"Exponential\" -variable res -value 10 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused); set str \"exp($value1)\"}");
cmd(inter, "radiobutton .a.r11 -text \"Logarithm\" -variable res -value 11 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused); set str \"log($value1)\"}");
cmd(inter, "radiobutton .a.r12 -text \"Square root\" -variable res -value 12 -command {.a.l1 conf -text Value; .a.l2 conf -text (unused); set str \"sqrt($value1)\"}");
cmd(inter, "radiobutton .a.r13 -text \"Power\" -variable res -value 13 -command {.a.l1 conf -text \"Base\"; .a.l2 conf -text \"Exponent\"; set str \"pow($value1,$value2)\"}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 18 -text Insert -command {set choice 1}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#rnd}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left");
cmd(inter, "pack .a.r1 .a.r2 .a.r3 .a.r4 .a.r5 .a.r6 .a.r7 .a.r8 .a.r9 .a.r10 .a.r11 .a.r12 .a.r13 -anchor w");
cmd(inter, "pack .a.f");
choice=0;

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.e1; .a.e1 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 { 
  choice=0;
  goto loop;
 }
cmd(inter, "set choice $res");
switch(choice)
{

case 1: cmd(inter, "set str \"UNIFORM($value1,$value2)\"");
break;
case 2: cmd(inter, "set str \"norm($value1,$value2)\"");
break;
case 3: cmd(inter, "set str \"rnd_integer($value1, $value2)\"");
break;
case 4: cmd(inter, "set str \"poisson($value1)\"");
break;
case 5: cmd(inter, "set str \"gamma($value1)\"");
break;
case 6: cmd(inter, "set str \"abs($value1)\"");
break;
case 7: cmd(inter, "set str \"min($value1,$value2)\"");
break;
case 8: cmd(inter, "set str \"max($value1,$value2)\"");
break;
case 9: cmd(inter, "set str \"round($value1)\"");
break;
case 10: cmd(inter, "set str \"exp($value1)\"");
break;
case 11: cmd(inter, "set str \"log($value1)\"");
break;
case 12: cmd(inter, "set str \"sqrt($value1)\"");
break;
case 13: cmd(inter, "set str \"pow($value1,$value2)\"");
break;

default: break;
}

cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, ".f.t.t insert insert \"$str\"");

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==52 && macro==1)
{
/*
Insert a add_an_obj;
*/
here_addobj:

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert an 'ADDOBJ' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the target pointer in which to return the new Object created\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.label; .a.label selection range 0 end}");


cmd(inter, "label .a.l2 -text \"Type below the label of the Object to create\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.numobj; .a.numobj selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the number of Objects to create\"");
cmd(inter, "set numobj \"1\"");
cmd(inter, "entry .a.numobj -width 6 -textvariable numobj");
cmd(inter, "bind .a.numobj <Return> {focus -force .a.v_num; .a.v_num selection range 0 end}");

cmd(inter, "label .a.l1 -text \"Type below the pointer to an example Object to copy its initialization, if any.\"");
cmd(inter, "set v_num \"cur\"");
cmd(inter, "entry .a.v_num -width 10 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the parent Object containing the new Object(s)\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#ADDOBJ}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.numobj .a.l1 .a.v_num .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");

cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");
 


choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);
 
 
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, "if { $numobj == \"1\"} {set choice 1} {set choice 0}");
cmd(inter, "if {$v_obj0 != \"\"} {.f.t.t insert insert $v_obj0; .f.t.t insert insert =} {}");

if(choice ==1)
{
cmd(inter, "sblocklmm .a");
//cmd(inter, ".f.t.t insert insert \"$v_obj0 \"");
cmd(inter, "if {$v_obj ==\"p\" && $v_num==\"\" } { .f.t.t insert insert \"ADDOBJ(\\\"$v_label\\\");\"} {}");
cmd(inter, "if {$v_obj ==\"p\" && $v_num!=\"\" } { .f.t.t insert insert \"ADDOBJ_EX(\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_num == \"\" } { .f.t.t insert insert \"ADDOBJS($v_obj,\\\"$v_label\\\");\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_num != \"\" } { .f.t.t insert insert \"ADDOBJS_EX($v_obj,\\\"$v_label\\\",$v_num);\"} {}");

}
else
{

cmd(inter, "if {$v_obj ==\"p\" && $v_num!=\"\" } { .f.t.t insert insert \"ADDNOBJ_EX(\\\"$v_label\\\",$numobj, $v_num);\"; set choice -3} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_num!= \"\" } { .f.t.t insert insert \"ADDNOBJS_EX($v_obj,\\\"$v_label\\\",$numobj,$v_num);\"; set choice -3} {}");
cmd(inter, "if {$v_obj ==\"p\" && $v_num==\"\" } { .f.t.t insert insert \"ADDNOBJ(\\\"$v_label\\\",$numobj);\"; set choice -3} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_num== \"\" } { .f.t.t insert insert \"ADDNOBJS($v_obj,\\\"$v_label\\\",$numobj);\"; set choice -3} {}");

/*
if(choice!=-3)
 {
 cmd(inter, "tk_messageBox -type ok -title Error -message \"Error: missing information\\nYou need to provide the pointer to an example object when adding more than 1 new objects.\"");
 choice=-3;
 goto here_addobj;
 }
*/  
}



cmd(inter, "sblocklmm .a");
cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==52 && macro==0)
{
/*
Insert a add_an_obj;
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert an 'add_an_object' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the target pointer in which to return the new Object created\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.v_num; .a.v_num selection range 0 end}");

cmd(inter, "label .a.l1 -text \"Type below the pointer to an example Object, if available\"");
cmd(inter, "set v_num \"\"");
cmd(inter, "entry .a.v_num -width 10 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the Object to create\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.obj; .a.obj selection range 0 end}");


cmd(inter, "label .a.l4 -text \"Type below the parent Object where to add the new Object\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#add_an_object}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, "if {$v_num==\"\" } { .f.t.t insert insert \"$v_obj0=$v_obj->add_an_object(\\\"$v_label\\\");\\n\"} {.f.t.t insert insert \"$v_obj0=$v_obj->add_an_object(\\\"$v_label\\\",$v_num);\\n\"}");

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==53 && macro==1)
{
/*
Insert a delete_obj;
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'DELETE' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the pointer of the Object to delete\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#DELETE}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left");

cmd(inter, "pack .a.l0 .a.obj0");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");


cmd(inter, ".f.t.t insert insert \"DELETE($v_obj0);\\n\"");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==53 && macro==0)
{
/*
Insert a delete_obj;
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'delete_obj' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the pointer of the Object to delete\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#delete_obj}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left");

cmd(inter, "pack .a.l0 .a.obj0");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");


cmd(inter, ".f.t.t insert insert \"$v_obj0->delete_obj();\\n\"");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==54 && macro==1)
{
/*
Insert a RNDDRAW
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'RNDDRAW' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the target pointer in which to return the Object drawn\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.v_num; .a.v_num selection range 0 end}");

cmd(inter, "label .a.l1 -text \"Type below the label of the Objects to draw\"");
cmd(inter, "set v_num Object");
cmd(inter, "entry .a.v_num -width 10 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the Var or Par to be used as proxies for probabilities\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the lag to be used\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.tot; .a.tot selection range 0 end}");

cmd(inter, "label .a.l31 -text \"Type below the sum over all values of the Var or Par, if available\"");
cmd(inter, "set v_tot \"\"");
cmd(inter, "entry .a.tot -width 9 -textvariable v_tot");
cmd(inter, "bind .a.tot <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object from which to search\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#RNDDRAW}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l31 .a.tot .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->search_var_cond(\\\"$v_label\\\",$v_num, $v_lag);\"");
cmd(inter, "if {$v_tot == \"\"} {set choice 1} {set choice 2}");

if(choice==1)
 {
  cmd(inter, "if {$v_obj ==\"p\" && $v_lag == 0 && $v_label != \"\"} { .f.t.t insert insert \"$v_obj0=RNDDRAW(\\\"$v_num\\\",\\\"$v_label\\\");\\n\"} {}");
  cmd(inter, "if {$v_obj ==\"p\" && $v_lag == 0 && $v_label == \"\"} { .f.t.t insert insert \"$v_obj0=RNDDRAWFAIR(\\\"$v_num\\\");\\n\"} {}");  
  cmd(inter, "if {$v_obj ==\"p\" && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0=RNDDRAWL(\\\"$v_num\\\",\\\"$v_label\\\", $v_lag);\\n\"} {}");
  cmd(inter, "if {$v_obj !=\"p\" && $v_lag == 0 && $v_label != \"\" } { .f.t.t insert insert \"$v_obj0=RNDDRAWS($v_obj,\\\"$v_num\\\",\\\"$v_label\\\");\\n\"} {}");
  cmd(inter, "if {$v_obj !=\"p\" && $v_lag == 0 && $v_label == \"\" } { .f.t.t insert insert \"$v_obj0=RNDDRAWFAIRS($v_obj,\\\"$v_num\\\");\\n\"} {}");  
  cmd(inter, "if {$v_obj !=\"p\" && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0=RNDDRAWLS($v_obj,\\\"$v_num\\\",\\\"$v_label\\\", $v_lag);\\n\"} {}");
  
 }
else
 {
  cmd(inter, "if {$v_obj ==\"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0=RNDDRAWTOT(\\\"$v_num\\\",\\\"$v_label\\\", $v_tot);\\n\"} {}");
  cmd(inter, "if {$v_obj ==\"p\" && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0=RNDDRAWTOTL(\\\"$v_num\\\",\\\"$v_label\\\", $v_lag, $v_tot);\\n\"} {}");
  cmd(inter, "if {$v_obj !=\"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0=RNDDRAWTOTS($v_obj,\\\"$v_num\\\",\\\"$v_label\\\", $v_tot);\\n\"} {}");
  cmd(inter, "if {$v_obj !=\"p\" && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0=RNDDRAWTOTLS($v_obj,\\\"$v_num\\\",\\\"$v_label\\\", $v_lag, $v_tot);\\n\"} {}");
  

 }


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==54 && macro==0)
{
/*
Insert a RNDDRAW
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'draw_rnd' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the target pointer in which to return the Object drawn\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.v_num; .a.v_num selection range 0 end}");

cmd(inter, "label .a.l1 -text \"Type below the label of the Objects to draw\"");
cmd(inter, "set v_num Object");
cmd(inter, "entry .a.v_num -width 10 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the Var or Par to be used as proxies for probabilities\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the lag to be used\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.tot; .a.tot selection range 0 end}");

cmd(inter, "label .a.l31 -text \"Type below the sum over all values of the Var or Par, if available\"");
cmd(inter, "set v_tot \"\"");
cmd(inter, "entry .a.tot -width 9 -textvariable v_tot");
cmd(inter, "bind .a.tot <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object from which to search\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#draw_rnd}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l31 .a.tot .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->search_var_cond(\\\"$v_label\\\",$v_num, $v_lag);\"");
cmd(inter, "if {$v_tot == \"\"} {set choice 1} {set choice 2}");

if(choice==1)
  cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->draw_rnd(\\\"$v_num\\\",\\\"$v_label\\\", $v_lag);\\n\"");
else
  cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->draw_rnd(\\\"$v_num\\\",\\\"$v_label\\\", $v_lag, $v_tot);\\n\"");

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}



if(choice==55&& macro==1)
{
/*
Insert a SEARCH;
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'SEARCH' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the target pointer where to return the found Object\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.lab; .a.lab selection range 0 end}");

cmd(inter, "label .a.l1 -text \"Type below the label of the Object to search\"");
cmd(inter, "set v_lab Object");
cmd(inter, "entry .a.lab -width 20 -textvariable v_lab");
cmd(inter, "bind .a.lab <Return> {focus -force .a.obj1; .a.obj1 selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the pointer of the parent Object where to start the search\"");
cmd(inter, "set v_obj1 p");
cmd(inter, "entry .a.obj1 -width 6 -textvariable v_obj1");
cmd(inter, "bind .a.obj1 <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#SEARCH}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.lab .a.l2 .a.obj1");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, "if { $v_obj1 == \"p\" } {.f.t.t insert insert \"$v_obj0=SEARCH(\\\"$v_lab\\\");\\n\"} {.f.t.t insert insert \"$v_obj0=SEARCHS($v_obj1,\\\"$v_lab\\\");\\n\"}");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==55&& macro==0)
{
/*
Insert a SEARCH;
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'search' Command\"");

cmd(inter, "label .a.l0 -text \"Type below the target pointer where to return the found Object\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.lab; .a.lab selection range 0 end}");

cmd(inter, "label .a.l1 -text \"Type below the label of the Object to search\"");
cmd(inter, "set v_lab Object");
cmd(inter, "entry .a.lab -width 20 -textvariable v_lab");
cmd(inter, "bind .a.lab <Return> {focus -force .a.obj1; .a.obj1 selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the pointer of the parent Object where to start the search\"");
cmd(inter, "set v_obj1 p");
cmd(inter, "entry .a.obj1 -width 6 -textvariable v_obj1");
cmd(inter, "bind .a.obj1 <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#search}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.lab .a.l2 .a.obj1");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.obj0");
cmd(inter, ".a.obj0 selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj1->search(\\\"$v_lab\\\");\\n\"");


cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==56 && macro==1)
{
/*
Insert a SUM;
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'SUM' Command\"");
cmd(inter, "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result\"");

sprintf(msg, "set v_num %d", v_counter);
cmd(inter, msg);
cmd(inter, "entry .a.v_num -width 2 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the variable to sum\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the lag to be used\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object to which request the computation\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfuncMacro.html#SUM}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

//cmd(inter, "if {$v_num==\"\"} {.f.t.t insert insert \"$v_obj->cal(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->cal(\\\"$v_label\\\",$v_lag);\"; incr v_num}");
cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}");

cmd(inter, "if {$v_lag==0 && $v_obj==\"p\"} { .f.t.t insert insert \"SUM(\\\"$v_label\\\")\"} {}");
cmd(inter, "if {$v_lag!=0 && $v_obj==\"p\"} { .f.t.t insert insert \"SUML(\\\"$v_label\\\",$v_lag)\"} {}");
cmd(inter, "if {$v_lag==0 && $v_obj!=\"p\"} { .f.t.t insert insert \"SUMS($v_obj,\\\"$v_label\\\")\"} {}");
cmd(inter, "if {$v_lag!=0 && $v_obj!=\"p\"} { .f.t.t insert insert \"SUMLS($v_obj,\\\"$v_label\\\",$v_lag)\"} {}");

cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \";\\n\"} {}");

cmd(inter, "if {$v_num==\"\"} { set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}

if(choice==56 && macro==0)
{
/*
Insert a p->sum;
*/
choice=0;
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Insert a 'sum' Command\"");
cmd(inter, "label .a.l1 -text \"Type below the number v\\\[x\\] to which assign the result\"");

sprintf(msg, "set v_num %d", v_counter);
cmd(inter, msg);
cmd(inter, "entry .a.v_num -width 2 -textvariable v_num");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.label; .a.label selection range 0 end}");

cmd(inter, "label .a.l2 -text \"Type below the label of the variable to sum\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.lag; .a.lag selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Type below the lag to be used\"");
cmd(inter, "set v_lag 0");
cmd(inter, "entry .a.lag -width 2 -textvariable v_lag");
cmd(inter, "bind .a.lag <Return> {focus -force .a.obj; .a.obj selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Type below the object to which request the computation\"");
cmd(inter, "set v_obj p");
cmd(inter, "entry .a.obj -width 6 -textvariable v_obj");
cmd(inter, "bind .a.obj <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp lsdfunc.html#sum}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 10 -pady 10 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj");
cmd(inter, "pack .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");

while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "savCurIni");	// save data for recolor
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->sum(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"$v_obj->sum(\\\"$v_label\\\",$v_lag);\\n\"}");

cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \";\\n\"} {}");

cmd(inter, "if {$v_num==\"\"} { set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "savCurFin; updCurWnd");	// save data for recolor
choice=23;	// do syntax coloring
goto loop;
}


if(choice==32)
{
/* find matching parenthesis */

cmd(inter, "set sym [.f.t.t get insert]");
cmd(inter, "if {[string compare $sym \"\\(\"]!=0} \
              {\
              if {[string compare $sym \"\\)\"]!=0} \
   {set choice 0} \
   {set num -1; set direction -backwards; set fsign +; set sign \"\"; set terminal \"1.0\"}\
              } \
   {set num 1; set direction -forwards; set fsign -; set sign +; set terminal end}");

if(choice==0)
 goto loop;
cmd(inter, "set cur [.f.t.t index insert]");
cmd(inter, ".f.t.t tag add sel $cur \"$cur + 1char\"");
if (num>0)
  cmd(inter, "set cur [.f.t.t index \"insert + 1 char\"]");
while(num!=0 && choice!=0)
{
 cmd(inter, "set a [.f.t.t search $direction \"\\(\" $cur $terminal]");
 cmd(inter, "if {$a==\"\"} {set a [.f.t.t index $terminal]} {}");
 cmd(inter, "set b [.f.t.t search $direction \"\\)\" $cur $terminal]");
// cmd(inter, ".f.t.t insert end \"$a $b $cur\\n\"");
 cmd(inter, "if {$b==\"\"} {set b [.f.t.t index $terminal]} {}");
 cmd(inter, "if {$a==$b} {set choice 0} {}");
 if(choice==0)
  goto loop;
 if(num>0)
   cmd(inter, "if {[.f.t.t compare $a < $b]} {set num [expr $num + 1]; set cur [.f.t.t index \"$a+1char\"]} {set num [expr $num - 1]; set cur [.f.t.t index \"$b+1char\"]}");
 else
   cmd(inter, "if {[.f.t.t compare $a > $b]} {set num [expr $num + 1]; set cur [.f.t.t index $a]} {set num [expr $num - 1]; set cur [.f.t.t index $b]}");


}
choice=0;


cmd(inter, " if { [string compare $sign \"+\"]==0 } {.f.t.t tag add sel \"$cur - 1char\" $cur ; set num 1} {.f.t.t tag add sel $cur \"$cur + 1char\"; set num 0}");

cmd(inter, ".f.t.t see $cur");
goto loop;
}


if(choice==33)
{
/* Select a model*/

choice=0;
Tcl_LinkVar(inter, "choiceSM", (char *) &num, TCL_LINK_INT);
num=0;
cmd(inter, "showmodel $groupdir");

while(choice==0 && num==0)
 Tcl_DoOneEvent(0);

cmd(inter, "if {[winfo exists .l]==1} {destroy .l; bind .f.t.t <Enter> {}} {}");
choice=num;

Tcl_UnlinkVar(inter, "choiceSM");
if(choice==2 || choice==0)
 {choice=0;
  goto loop;
 }
cmd(inter, "set groupdir [lindex $lrn 0]"); //the group dir is the same for every element
if(choice == 14)
 { //create a new model/group
 goto loop;
 }
if( choice==4)
 {
  cmd(inter, "if { [lindex $lmn $result] == \"..\" } {set choice 33} {}");
  if(choice==33)
   {
   cmd(inter, "tk_messageBox -type ok -title \"Error\" -icon error -message \"You cannot remove this.\"");
   goto loop;
   }
  cmd(inter, "if { [lindex $group $result] == 1} {set message \"Group\\n[lindex $lmn $result] (dir. [lindex $ldn $result])\\n is going to be deleted.\\nConfirm?\"; set answer [tk_messageBox -icon question -type yesno -title \"Remove group?\" -message $message]} {set message \"Model\\n[lindex $lmn $result] ver. [lindex $lver $result] (dir. [lindex $ldn $result])\\n is going to be deleted.\\n Confirm?\"; set answer [tk_messageBox -icon question -type yesno -title \"Remove model?\" -message $message]}");
  cmd(inter, "if {$answer == \"yes\"} {file delete -force [lindex $ldn $result]} {}");
  cmd(inter, "set groupdir [lindex $lrn $result]");
  choice=33;
  goto loop;
 } 
cmd(inter, "set modelname [lindex $lmn $result]");
cmd(inter, "set version [lindex $lver $result]");
cmd(inter, "set modeldir [lindex $ldn $result]");
cmd(inter, "set dirname $modeldir");
cmd(inter, "set filename \"description.txt\"");

cmd(inter, ".f.t.t delete 0.0 end");
cmd(inter, "if { [file exists $modeldir/description.txt] == 1} {set f [open $modeldir/description.txt]; .f.t.t insert end \"[read -nonewline $f]\"; .f.t.t edit reset; close $f} {.f.t.t insert end \"Model $modelname (ver. $version)\\n\"; .f.t.t edit reset}");

cmd(inter, ".f.hea.file.dat conf -text \"$filename\"");
cmd(inter, ".f.hea.grp.dat conf -text \"$modelgroup\"");
cmd(inter, ".f.hea.mod.dat conf -text \"$modelname\"");
cmd(inter, ".f.hea.ver.dat conf -text \"$version\"");
cmd(inter, ".m.model entryconf 3 -state normal");
cmd(inter, ".m.model entryconf 4 -state normal");
cmd(inter, ".m.model entryconf 5 -state normal");
cmd(inter, ".m.model entryconf 6 -state normal");
cmd(inter, ".m.model entryconf 7 -state normal");
cmd(inter, ".m.model entryconf 9 -state normal");
cmd(inter, ".m.model entryconf 10 -state normal");
cmd(inter, ".m.model entryconf 11 -state normal");
cmd(inter, ".m.model entryconf 12 -state normal");
cmd(inter, ".m.model entryconf 14 -state normal");

cmd(inter, "sblocklmm .l");
cmd(inter, "set before [.f.t.t get 0.0 end]"); //avoid to re-issue a warning for non saved files
choice=50;
goto loop;
}



if(choice==41)
{
/************
 create a new version of the current model
*************/
choice=0;


cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm title .a \"Copy Model\"");
cmd(inter, "label .a.tit -text \"Create a new version of model '$modelname' (ver. $version)\"");

cmd(inter, "label .a.mname -text \"Insert new model name\"");
cmd(inter, "set mname $modelname");
cmd(inter, "entry .a.ename -width 30 -textvariable mname");
cmd(inter, "bind .a.ename <Return> {.a.ever selection range 0 end; focus -force .a.ever}"); 

cmd(inter, "label .a.mver -text \"Insert new version number\"");
cmd(inter, "set mver $version");
cmd(inter, "entry .a.ever -width 30 -textvariable mver");
cmd(inter, "bind .a.ever <Return> {.a.edir selection range 0 end; focus -force .a.edir}"); 

cmd(inter, "label .a.mdir -text \"Insert directory name\"");
cmd(inter, "set mdir $dirname");
cmd(inter, "entry .a.edir -width 30 -textvariable mdir");
cmd(inter, "bind .a.edir <Return> {focus -force .a.b.ok}"); 

cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "button .a.b.help -padx 20 -text Help -command {LsdHelp LMM_help.html#copy}");
cmd(inter, "button .a.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.b.ok .a.b.help .a.b.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.mname .a.ename .a.mver .a.ever .a.mdir .a.edir");
cmd(inter, "pack .a.b");

cmd( inter, "setgeom .a" );
cmd(inter, "blocklmm .a");
cmd(inter, ".a.ename selection range 0 end");
cmd(inter, "focus -force .a.ename");

loop_copy:


while(choice==0)
 Tcl_DoOneEvent(0);


//operation cancelled
if(choice==2)
 {
  cmd(inter, "sblocklmm .a");  
  choice=0;
  goto loop;
 }

//control for existing directory
cmd(inter, "if {[file exists $mdir] == 1} {tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create directory: $mdir.\\nChoose a different name.\"; set choice 3} {}");
if(choice==3)
 {cmd(inter, ".a.edir selection range 0 end");
  cmd(inter, "focus -force .a.edir");
  choice=0;
  goto loop_copy;
 } 


//control for an existing model with the same name AND same version
cmd(inter, "set dir [glob *]");
cmd(inter, "set num [llength $dir]");
strcpy(str, " ");
for(i=0; i<num && choice!=3; i++)
 {
  sprintf(msg, "if {[file isdirectory [lindex $dir %d] ] == 1} {set curdir [lindex $dir %i]} {set curdir ___}",i, i);
  cmd(inter, msg);
  s=(char *)Tcl_GetVar(inter, "curdir",0);
  strcpy(str, s);
  if(strcmp(str,"___") && strcmp(str, "gnu") && strcmp(str, "src") && strcmp(str, "Manual") )
   {

    cmd(inter, "set ex [file exists $curdir/modelinfo.txt]");
    cmd(inter, "if { $ex == 0 } {set choice 0} {set choice 1}");
    if(choice==1)
    {
      cmd(inter, "set f [open $curdir/modelinfo.txt r]");
      cmd(inter, "set cname [gets $f]; set cver [gets $f];");
      cmd(inter, "close $f");
    }
    else
      cmd(inter, "set cname $curdir; set cver \"0.1\"");
    cmd(inter, "set comp [string compare $cname $mname]");
    cmd(inter, "set comp1 [string compare $cver $mver]");
    cmd(inter, "if {$comp == 0 & $comp1 == 0} {set choice 3; set errdir $curdir} {}");

   }

 }
if(choice==3)
 {cmd(inter, "tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create the new model '$mname' (ver. $mver) because it already exists (directory: $errdir).\"");
  cmd(inter, ".a.ename selection range 0 end");
  cmd(inter, "focus -force .a.ename");
  choice=0;
  goto loop_copy;
 } 

//here we are: create a new copycat model
cmd(inter, "file copy $dirname $mdir");
cmd(inter, "set dirname $mdir");
cmd(inter, "set modeldir $mdir");
cmd(inter, "set modelname $mname");
cmd(inter, "set version $mver");

cmd(inter, ".f.hea.mod.dat conf -text \"$modelname\"");
cmd(inter, ".f.hea.ver.dat conf -text \"$version\"");
cmd(inter, "sblocklmm .a");
cmd(inter, "set ex [file exists $dirname/modelinfo.txt]");
cmd(inter, "if { $ex == 0 } {set choice 0} {set choice 1}");
if(choice==1)
  cmd(inter, "file delete $dirname/modelinfo.txt");
cmd(inter, "set f [open $dirname/modelinfo.txt w]");
cmd(inter, "puts $f \"$modelname\"");
cmd(inter, "puts $f \"$version\"");
cmd(inter, "set frmt \"%d %B, %Y\"");
cmd(inter, "puts $f \"[clock format [clock seconds] -format \"$frmt\"]\"");
cmd(inter, "close $f");
cmd(inter, "tk_messageBox -type ok -title \"Model Copied\" -icon info -message \"New model '$mname' (ver. $mver) successfully created (directory: $dirname).\"");

choice=49;
goto loop;
}

if(choice==42)
 {
 /************
Increase indent
 *************/
choice=0;
cmd(inter, "set in [.f.t.t tag range sel]");
cmd(inter, "if { [string length $in] == 0 } {set choice 0} {set choice 1}");
if(choice==0)
 goto loop;
cmd(inter, "scan $in \"%d.%d %d.%d\" line1 col1 line2 col2");
cmd(inter, "set num $line1");
i=num;
cmd(inter, "set num $line2");

for( ;i<=num; i++)
 {
 sprintf(msg, ".f.t.t insert %d.0 \" \"", i);
 cmd(inter, msg);
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
cmd(inter, "set in [.f.t.t tag range sel]");
cmd(inter, "if { [string length $in] == 0 } {set choice 0} {set choice 1}");
if(choice==0)
 goto loop;
cmd(inter, "scan $in \"%d.%d %d.%d\" line1 col1 line2 col2");
cmd(inter, "set num $line1");
i=num;
cmd(inter, "set num $line2");

for( ;i<=num; i++)
 {
 sprintf(msg, "set c [.f.t.t get %d.0]", i);
 cmd(inter, msg);
 cmd(inter, "if { $c == \" \" } {set choice 1} {set choice 0}");
 if(choice==1)
  { sprintf(msg, ".f.t.t delete %d.0 ", i);
    cmd(inter, msg);
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
choice=0;

cmd(inter, "set ex [file exists $modeldir/modelinfo.txt]");
cmd(inter, "set choice $ex");
if(choice==0)
  cmd(inter, "tk_messageBox -type ok -title Error -icon error -message \"Cannot find file for model info.\\nPlease, check the date of creation.\"");



  
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Model Info\"");
cmd(inter, "frame .a.c");

cmd(inter, "label .a.c.n -text \"Model Name\"");
cmd(inter, "entry .a.c.en -width 40 -textvariable modelname");

cmd(inter, "label .a.c.d -text \"Model Directory\"");
cmd(inter, "set complete_dir [file nativename [file join [pwd] $modeldir]]");
cmd(inter, "entry .a.c.ed -width 40 -state disabled -textvariable complete_dir");

cmd(inter, "label .a.c.v -text Version");
cmd(inter, "entry .a.c.ev -width 40 -textvariable version");


cmd(inter, "label .a.c.date -text \"Date Creation\"");
if(choice==1)
{
  cmd(inter, "set f [open $modeldir/modelinfo.txt r]");
  cmd(inter, "gets $f; gets $f; set date [gets $f]"); 
  cmd(inter, "close $f");
}

if(choice==1)
 cmd(inter, "entry .a.c.edate -width 40 -state disabled -textvariable date");
else
  cmd(inter, "entry .a.c.edate -width 40 -textvariable date");

cmd(inter, "cd $modeldir");
make_makefile();
cmd(inter, "set fapp [file nativename $modeldir/makefile]");
s=(char *)Tcl_GetVar(inter, "fapp",0);

f=fopen(s, "r");


if(f==NULL)
 {
  cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"File 'makefile' not found.\\nAdd a makefile to model '$modelname' ([pwd]).\"");
  choice=0;
  cmd(inter, "cd $RootLsd");
  cmd(inter, "if { [winfo exists .a] == 1} {destroy .a} {}");
  goto loop;
 }
fscanf(f, "%s", str);
while(strncmp(str, "FUN=", 4) && fscanf(f, "%s", str)!=EOF);
fclose(f);
if(strncmp(str, "FUN=", 4)!=0)
 {
  cmd(inter, "tk_messageBox -type ok -title Error -icon error -message \"Makefile corrupted. Check Model and System Compilation options.\"");
  choice=0;
  goto loop;
 }



sprintf(msg, "set eqname %s.cpp", str+4);
cmd(inter, msg);

cmd(inter, "cd $RootLsd");

cmd(inter, "label .a.c.last -text \"Date Last Modification (equations only)\"");
cmd(inter, "set frmt \"%d %B, %Y\"");
cmd(inter, "set last \"[clock format [file mtime $modeldir/$eqname] -format \"$frmt\"]\"");
cmd(inter, "entry .a.c.elast -width 40 -state disabled -textvariable last");


cmd(inter, "pack .a.c.n .a.c.en .a.c.v .a.c.ev .a.c.date .a.c.edate .a.c.last .a.c.elast .a.c.d .a.c.ed");

cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "button .a.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.b.ok .a.b.esc -padx 10 -pady 10 -side left");

cmd(inter, "pack .a.c .a.b");
cmd(inter, "bind .a <Escape> {set choice 2}");
cmd(inter, "bind .a.c.en <Return> {focus -force .a.c.ev}");
cmd(inter, "bind .a.c.ev <Return> {focus -force .a.b.ok}");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
choice=0;

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.c.en");
cmd(inter, "blocklmm .a");
while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");

if(choice==1)
 {
  cmd(inter, "set f [open $modeldir/modelinfo.txt w]");
  cmd(inter, "puts $f \"$modelname\"");
  cmd(inter, "puts $f \"$version\"");
  cmd(inter, "puts $f \"$date\"");
  cmd(inter, "close $f");
  cmd(inter, ".f.hea.mod.dat conf -text \"$modelname\"");
  cmd(inter, ".f.hea.ver.dat conf -text \"$version\"");
  
 }



choice=0;
goto loop;
 

}

if(choice==39)
 {
 /************
 create a new file
 *************/
 cmd(inter, ".f.t.t delete 1.0 end");
 cmd(inter, "set before [.f.t.t get 1.0 end]");
 cmd(inter, "set filename noname.txt");
 cmd(inter, "set dirname [pwd]");
 cmd(inter, ".f.t.t mark set insert 1.0");
 cmd(inter, ".f.hea.file.dat conf -text \"noname.txt\"");
 cmd(inter, "wm title . \"nomame.txt - LMM\"");
 cmd(inter, "catch [unset -nocomplain ud]");
 cmd(inter, "catch [unset -nocomplain udi]");
 cmd(inter, "catch [unset -nocomplain rd]");
 cmd(inter, "catch [unset -nocomplain rdi]");
 cmd(inter, "lappend ud [.f.t.t get 0.0 end]");
 cmd(inter, "lappend udi [.f.t.t index insert]");
 
 choice=0;
 goto loop;
 }

if(choice==46 || choice==49)
{
/*
Create the makefile
*/
cmd(inter, "cd $modeldir");

cmd(inter, "set choice [file exists model_options.txt]");
if(choice==0)
 {//the model_options.txt file does not exists, probably an old version
   cmd(inter, "set dir [glob *.cpp]");
   cmd(inter, "set b [lindex $dir 0]");
   cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\nSWITCH_CC_LNK=\\n\"");
   cmd(inter, "set f [open model_options.txt w]");
   cmd(inter, "puts -nonewline $f $a");
   cmd(inter, "close $f");
 }
choice=0; 
cmd(inter, "set f [open model_options.txt r]");
cmd(inter, "set a [read -nonewline $f]");
cmd(inter, "close $f");

cmd(inter, "cd $RootLsd");

cmd(inter, "set choice [file exists $LsdSrc/system_options.txt]");
if(choice==0)
 { //the src/system_options.txt file doesn't exists, so I invent it
   cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {file copy $RootLsd/$LsdSrc/sysopt_windows.txt $RootLsd/$LsdSrc/system_options.txt} {file copy $RootLsd/$LsdSrc/sysopt_linux.txt $RootLsd/$LsdSrc/system_options.txt}");
   cmd(inter, "set f [open model_options.txt w]");
   cmd(inter, "puts -nonewline $f $d");
   cmd(inter, "close $f");
 }
choice=0; 

cmd(inter, "set f [open $LsdSrc/system_options.txt r]");
cmd(inter, "set d [read -nonewline $f]");
cmd(inter, "close $f");

cmd(inter, "set f [open $LsdSrc/makefile_base.txt r]");
cmd(inter, "set b [read -nonewline $f]");
cmd(inter, "close $f");

cmd(inter, "cd $modeldir");
cmd(inter, "set c \"# Model compilation options\\n$a\\n\\n# System compilation option\\n$d\\n\\n# body of the makefile (in $LsdSrc/makefile_base.txt)\\n$b\"");
cmd(inter, "set f [open makefile w]");
cmd(inter, "puts -nonewline $f $c");
cmd(inter, "close $f");

cmd(inter, "cd $RootLsd");
if(choice==46)
  choice=0; //just create the makefile
if(choice==49) //after this show the equation file (and a model is created)
  choice=8;
  
goto loop;

} 



if(choice==47)
{
/*
System Compilation Options
*/

choice=0;
cmd(inter, "set choice [file exists $LsdSrc/system_options.txt]");

if(choice==1)
 {
  cmd(inter, "set f [open $LsdSrc/system_options.txt r]");
  cmd(inter, "set a [read -nonewline $f]");
  cmd(inter, "close $f");
  choice=0;
 }
else
  cmd(inter, "set a \"\"");


cmd(inter, "if { [winfo exists .l]==1} {tk_messageBox -type ok; .l.m.file invoke 1; } { }"); 
cmd(inter, "toplevel .l");
cmd(inter, "wm protocol .l WM_DELETE_WINDOW { }");
cmd(inter, "wm title .l \"System Compilation Options\"");

cmd(inter, "wm transient .l .");
cmd(inter, "frame .l.t");

cmd(inter, "scrollbar .l.t.yscroll -command \".l.t.text yview\"");
cmd(inter, "text .l.t.text -wrap word -font {Times 10 normal} -width 60 -height 16 -relief sunken -yscrollcommand \".l.t.yscroll set\"");

if((size_t)-1 > 0xffffffffUL)  // test for Windows 64-bit 
  cmd(inter, "set win_default \"TCL_VERSION=85\\nTK_VERSION=85\\nLSDROOT=[pwd]\\nDUMMY=mwindows\\nPATH_TCL_LIB=\\$(LSDROOT)/$LsdGnu/lib\\nPATH_TK_LIB=\\$(LSDROOT)/$LsdGnu/lib\\nPATH_TK_HEADER=\\$(LSDROOT)/$LsdGnu/include\\nPATH_TCL_HEADER=\\$(LSDROOT)/$LsdGnu/include\\nPATH_LIB=\\$(LSDROOT)/$LsdGnu/lib\\nINCLUDE_LIB=-I\\$(LSDROOT)/$LsdGnu/include\\nCC=g++\\nSRC=src\\nEXTRA_PAR=-lz\\nSSWITCH_CC=-march=native -mtune= native -O3\\n\"");
else
  cmd(inter, "set win_default \"TCL_VERSION=85\\nTK_VERSION=85\\nLSDROOT=[pwd]\\nDUMMY=mwindows\\nPATH_TCL_LIB=\\$(LSDROOT)/$LsdGnu/lib\\nPATH_TK_LIB=\\$(LSDROOT)/$LsdGnu/lib\\nPATH_TK_HEADER=\\$(LSDROOT)/$LsdGnu/include\\nPATH_TCL_HEADER=\\$(LSDROOT)/$LsdGnu/include\\nPATH_LIB=\\$(LSDROOT)/$LsdGnu/lib\\nINCLUDE_LIB=-I\\$(LSDROOT)/$LsdGnu/include\\nCC=g++\\nSRC=src\\nEXTRA_PAR=-lz\\nSSWITCH_CC=-O2\\n\"");

cmd(inter, "set lin_default \"TCL_VERSION=8.5\\nTK_VERSION=8.5\\nLSDROOT=[pwd]\\nDUMMY=\\nPATH_TCL_LIB=.\\nPATH_TK_LIB=.\\nPATH_TK_HEADER=\\nPATH_TCL_HEADER=\\nPATH_LIB=.\\nINCLUDE_LIB=\\nCC=g++\\nSRC=src\\nEXTRA_PAR=-lz\\nSSWITCH_CC=-O2\\n\"");

cmd(inter, "frame .l.t.d");
cmd(inter, "frame .l.t.d.os");

if((size_t)-1 > 0xffffffffUL)  // test for Windows 64-bit 
  cmd(inter, "button .l.t.d.os.win -padx 2 -text \"Default Windows x64\" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_windows64.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}"); 
else
  cmd(inter, "button .l.t.d.os.win -padx 12 -text \"Default Windows\" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_windows.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}"); 


cmd(inter, "button .l.t.d.os.lin -padx 22 -text \"Default Linux\" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_linux.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}");
cmd(inter, "button .l.t.d.os.mac -padx 12 -text \"Default Mac OSX\" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_mac.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}"); 
cmd(inter, "pack .l.t.d.os.win .l.t.d.os.lin .l.t.d.os.mac -padx 1 -pady 5 -side left");

cmd(inter, "frame .l.t.d.b");
cmd(inter, "button .l.t.d.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "button .l.t.d.b.help -padx 20 -text Help -command {LsdHelp LMM_help.html#compilation_options}");
cmd(inter, "button .l.t.d.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .l.t.d.b.ok .l.t.d.b.help .l.t.d.b.esc -padx 10 -pady 5 -side left");
cmd(inter, "pack .l.t.yscroll -side right -fill y");
cmd(inter, "pack .l.t.d.os .l.t.d.b");
cmd(inter, "pack .l.t.text .l.t.d -expand yes -fill both");
cmd(inter, "pack .l.t");

cmd(inter, ".l.t.text insert end $a");

//cmd(inter, "bind .l <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .l <KeyPress-Escape> {set choice 2}");

cmd( inter, "setgeom .l" );
cmd(inter, "focus -force .l.t.text");
cmd(inter, "blocklmm .l");
while(choice==0)
 Tcl_DoOneEvent(0);


if(choice==1)
 {
  cmd(inter, "set f [open $LsdSrc/system_options.txt w]");
  cmd(inter, "puts -nonewline $f [.l.t.text get 1.0 end]");
  cmd(inter, "close $f");
  choice=46; //go to create makefile
 }
else
 choice=0; 
cmd(inter, "sblocklmm .l");

goto loop;

} 

if(choice==48)
{
/*
Model Compilation Options
*/

cmd(inter, "cd $modeldir");
choice=0;
cmd(inter, "set choice [file exists model_options.txt]");

cmd(inter, "set dir [glob *.cpp]");
cmd(inter, "set b [lindex $dir 0]");
if(choice==1)
 {
  cmd(inter, "set f [open model_options.txt r]");
  cmd(inter, "set a [read -nonewline $f]");
  cmd(inter, "close $f");
  choice=0;
 }
else
  {
   cmd(inter, "tk_messageBox -type ok -icon warning -message \"Model compilation options not found. The system will use default values.\" -title \"Warning\"");
   cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\nSWITCH_CC_LNK=\\n\"");
   cmd(inter, "set f [open model_options.txt w]");
   cmd(inter, "puts -nonewline $f $a");
   cmd(inter, "close $f");

  }
cmd(inter, "if { [winfo exists .l]==1} {.l.m.file invoke 1} { }"); 
cmd(inter, "toplevel .l");
cmd(inter, "wm protocol .l WM_DELETE_WINDOW { }");
cmd(inter, "wm title .l \"Model Compilation Options\"");

cmd(inter, "wm transient .l .");

cmd(inter, "frame .l.t");

cmd(inter, "scrollbar .l.t.yscroll -command \".l.t.text yview\"");
cmd(inter, "text .l.t.text -wrap word -font {Times 10 normal} -width 60 -height 10 -relief sunken -yscrollcommand \".l.t.yscroll set\"");

cmd(inter, "set default \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\nSWITCH_CC_LNK=\\n\"");


cmd(inter, "frame .l.t.d");
cmd(inter, "frame .l.t.d.opt");
cmd(inter, "button .l.t.d.opt.def -padx 30 -text \"Default Values\" -command {.l.t.text delete 1.0 end; .l.t.text insert end \"$default\"}");

cmd(inter, "button .l.t.d.opt.cle -padx 2 -text \"Clean Pre-Compiled Files\" -command { if { [ catch { glob $RootLsd/$LsdSrc/*.o } objs ] == 0 } { foreach i $objs { catch { file delete -force $i } } } }");

cmd(inter, "pack .l.t.d.opt.def .l.t.d.opt.cle -padx 10 -pady 5 -side left");

cmd(inter, "frame .l.t.d.b");
cmd(inter, "button .l.t.d.b.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "button .l.t.d.b.help -padx 20 -text Help -command {LsdHelp LMM_help.html#compilation_options}");
cmd(inter, "button .l.t.d.b.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .l.t.d.b.ok .l.t.d.b.help .l.t.d.b.esc -padx 10 -pady 5 -side left");
cmd(inter, "pack .l.t.d.opt .l.t.d.b");

cmd(inter, "pack .l.t.yscroll -side right -fill y");
cmd(inter, "pack .l.t.text .l.t.d -expand yes -fill both");
cmd(inter, "pack .l.t");
cmd(inter, "bind .l <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .l <KeyPress-Escape> {set choice 2}");
cmd(inter, ".l.t.text insert end $a");

cmd( inter, "setgeom .l" );
cmd(inter, "blocklmm .l");
cmd(inter, "focus -force .l.t.text");

cmd(inter, "set cazzo 0");
while(choice==0)
 Tcl_DoOneEvent(0);


if(choice==1)
 {
  cmd(inter, "set f [open model_options.txt w]");
  cmd(inter, "puts -nonewline $f [.l.t.text get 1.0 end]");
  cmd(inter, "close $f");
  choice=46; //go to create makefile
 }
else
 choice=0; 
//cmd(inter, "set choice $cazzo");
cmd(inter, "sblocklmm .l");
cmd(inter, "cd $RootLsd");

goto loop;

} 
if(choice==57)
{
//launch tkdiff



cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {if {$tcl_platform(os) == \"Darwin\" } {set choice 1} {set choice 1} } {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\"} {set choice 4} {set choice 2}} {set choice 3}}");
if(choice==1) //unix
 sprintf(msg,"exec $wish $LsdSrc/tkdiffb.tcl &");
if(choice==2) //win2k
 sprintf(msg, "exec $wish $LsdSrc/tkdiffb.tcl &"); //Changed
if(choice==3) //win 95/98
 sprintf(msg, "exec start $wish $LsdSrc/tkdiffb.tcl &");
if(choice==4)  //win NT
 sprintf(msg, "exec cmd /c start $wish $LsdSrc/tkdiffb.tcl &");
 
cmd(inter, msg);
choice=0;
goto loop;

}

if(choice==59)
{
//Change font

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Change Font\"");
cmd(inter, "label .a.l1 -text \"Enter the font name you wish to use\"");

cmd(inter, "entry .a.v_num -width 30 -textvariable fonttype");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp LMM_help.html#changefont}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");
cmd(inter, "blocklmm .a");
choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
 {
  choice=0;
  goto loop;
 }
cmd(inter, "set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"; settab .f.t.t $tabsize \"$a\"");



choice=0;
goto loop;

}


if(choice==60)
{
//LMM system options

cmd(inter, "set temp_var1 $Terminal");
cmd(inter, "set temp_var2 $HtmlBrowser");
cmd(inter, "set temp_var3 $fonttype");
cmd(inter, "set temp_var4 $wish");
cmd(inter, "set temp_var5 $LsdSrc");
cmd(inter, "set temp_var6 $dim_character");
cmd(inter, "set temp_var7 $tabsize");
cmd(inter, "set temp_var8 $wrap");
cmd(inter, "set temp_var9 $shigh");
cmd(inter, "set temp_var10 $autoHide");

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"LMM Options\"");
cmd(inter, "wm transient .a .");
cmd(inter, "label .a.l1 -text \"Terminal to use for the GDB debugger\"");
cmd(inter, "entry .a.v_num -width 30 -textvariable temp_var1");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.v_num2; .a.v_num2 selection range 0 end}");

cmd(inter, "label .a.l2 -text \"HTML Browser to use for help pages.\"");
cmd(inter, "entry .a.v_num2 -width 30 -textvariable temp_var2");
cmd(inter, "bind .a.v_num2 <Return> {focus -force .a.v_num4; .a.v_num4 selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Wish program\"");
cmd(inter, "entry .a.v_num4 -width 30 -textvariable temp_var4");
cmd(inter, "bind .a.v_num4 <Return> {focus -force .a.v_num5; .a.v_num4 selection range 0 end}");

cmd(inter, "label .a.l5 -text \"Source code subdirectory\"");
cmd(inter, "entry .a.v_num5 -width 30 -textvariable temp_var5");
cmd(inter, "bind .a.v_num5 <Return> {focus -force .a.v_num3; .a.v_num3 selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Font family\"");
cmd(inter, "entry .a.v_num3 -width 30 -textvariable temp_var3");
cmd(inter, "bind .a.v_num3 <Return> {focus -force .a.v_num6; .a.v_num6 selection range 0 end}");

cmd(inter, "label .a.l6 -text \"Font size (points)\"");
cmd(inter, "entry .a.v_num6 -width 30 -textvariable temp_var6");
cmd(inter, "bind .a.v_num6 <Return> {focus -force .a.v_num7; .a.v_num7 selection range 0 end}");

cmd(inter, "label .a.l7 -text \"Tab size (characters))\"");
cmd(inter, "entry .a.v_num7 -width 30 -textvariable temp_var7");
cmd(inter, "bind .a.v_num7 <Return> {focus -force .a.v_num8; .a.v_num8 selection range 0 end}");

cmd(inter, "label .a.l8 -text \"Wrap text (0:no/1:yes)\"");
cmd(inter, "entry .a.v_num8 -width 30 -textvariable temp_var8");
cmd(inter, "bind .a.v_num8 <Return> {focus -force .a.v_num9; .a.v_num9 selection range 0 end}");

cmd(inter, "label .a.l9 -text \"Syntax highlights (0:no/1:part./2:full)\"");
cmd(inter, "entry .a.v_num9 -width 30 -textvariable temp_var9");
cmd(inter, "bind .a.v_num9 <Return> {focus -force .a.v_num10; .a.v_num10 selection range 0 end}");

cmd(inter, "label .a.l10 -text \"Auto hide on run (0:no/1:yes)\"");
cmd(inter, "entry .a.v_num10 -width 30 -textvariable temp_var10");
cmd(inter, "bind .a.v_num10 <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f1");
cmd(inter, "button .a.f1.def -padx 14 -text Default -command {set temp_var1 $DefaultTerminal; set temp_var2 $DefaultHtmlBrowser; set temp_var3 $DefaultFont; set temp_var5 src; set temp_var6 12; set temp_var7 2; set temp_var8 1; set temp_var9 2; set temp_var10 1}");
cmd(inter, "button .a.f1.help -padx 20 -text Help -command {LsdHelp LMM_help.html#SystemOpt}");
cmd(inter, "pack .a.f1.def .a.f1.help -padx 10 -pady 5 -side left");

cmd(inter, "frame .a.f2");
cmd(inter, "button .a.f2.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "button .a.f2.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.f2.ok .a.f2.esc -padx 10 -pady 5 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.v_num2 .a.l4 .a.v_num4 .a.l5 .a.v_num5 .a.l3 .a.v_num3 .a.l6 .a.v_num6 .a.l7 .a.v_num7 .a.l8 .a.v_num8 .a.l9 .a.v_num9 .a.l10 .a.v_num10 .a.f1 .a.f2");
cmd(inter, "bind .a.f2.ok <Return> {.a.f2.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.f2.esc invoke}");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");
cmd(inter, "blocklmm .a");
choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==1)
 {
 cmd(inter, "set Terminal $temp_var1");
 cmd(inter, "set HtmlBrowser $temp_var2");
 cmd(inter, "set fonttype $temp_var3");
 cmd(inter, "set wish $temp_var4");
 cmd(inter, "set LsdSrc $temp_var5");
 cmd(inter, "set dim_character $temp_var6");
 cmd(inter, "set tabsize $temp_var7");
 cmd(inter, "set wrap $temp_var8");
 cmd(inter, "set shigh $temp_var9");
 cmd(inter, "set autoHide $temp_var10");
 
 cmd(inter, "set a [list $fonttype $dim_character]");
 cmd(inter, ".f.t.t conf -font \"$a\"");
 cmd( inter, "settab .f.t.t $tabsize \"$a\"" );	// adjust tabs size to font type/size
 cmd( inter, "setwrap .f.t.t $wrap" );			// adjust text wrap
 color(shigh, 0, 0);							// set color highlights (all text)
cmd(inter, "set f [open $RootLsd/lmm_options.txt w]");
 cmd(inter, "puts -nonewline $f  \"$Terminal\n\"");
 cmd(inter, "puts $f $HtmlBrowser");
 cmd(inter,  "puts $f $fonttype");
 cmd(inter,  "puts $f $wish");
 cmd(inter, "puts $f $LsdSrc");
 cmd(inter, "puts $f $dim_character");
 cmd(inter, "puts $f $tabsize");
 cmd(inter, "puts $f $wrap");
 cmd(inter, "puts $f $shigh");
 cmd(inter, "puts $f $autoHide");
 cmd(inter, "close $f");
 }

choice=0;
goto loop;


}

if(choice==61)
{
// start tkdiff with model selection
choice=0;

cmd(inter, "chs_mdl");
while(choice==0)
 Tcl_DoOneEvent(0);

if(choice==-1)
 {
  choice=0;
  goto loop;

 }
 
cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\"} {set choice 4} {set choice 2}} {set choice 3}}");
if(choice==1) //unix
 sprintf(msg,"exec $wish src/tkdiffb.tcl [file join $d1 $f1] [file join $d2 $f2] &");
if(choice==2) //win2k
 sprintf(msg, "exec $wish src/tkdiffb.tcl [file join $d1 $f1] [file join $d2 $f2]  &"); //Changed
if(choice==3) //win 95/98
 sprintf(msg, "exec start $wish src/tkdiffb.tcl [file join $d1 $f1] [file join $d2 $f2]  &");
if(choice==4)  //win NT
 sprintf(msg, "exec cmd /c start $wish src/tkdiffb.tcl [file join $d1 $f1] [file join $d2 $f2]  &");
 
cmd(inter, msg);
choice=0;
goto loop;
}

if(choice==62)
{
/*
Generate the no window distribution
*/
cmd(inter, "cd $modeldir");

make_makefileNW();

cmd(inter, "if { [file exist src] ==1 } {set choice 1} {file mkdir src; set choice 0}");

//cmd(inter, "set lfile [glob $RootLsd/$LsdSrc/*.cpp]");
cmd(inter, "set lfile {$RootLsd/$LsdSrc/lsdmain.cpp $RootLsd/$LsdSrc/main_gnuwin.cpp $RootLsd/$LsdSrc/file.cpp $RootLsd/$LsdSrc/util.cpp $RootLsd/$LsdSrc/variab.cpp $RootLsd/$LsdSrc/object.cpp $RootLsd/$LsdSrc/nets.cpp $RootLsd/$LsdSrc/report.cpp $RootLsd/$LsdSrc/fun_head.h $RootLsd/$LsdSrc/decl.h } ");

//cmd(inter, "foreach i $lfile {file copy -force \"$i\" src}");

//cmd(inter, "set lfile [glob $RootLsd/$LsdSrc/*.h]");
//cmd(inter, "foreach i $lfile {file copy -force $i src}");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/lsdmain.cpp src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/main_gnuwin.cpp src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/file.cpp src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/variab.cpp src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/object.cpp src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/report.cpp src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/util.cpp src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/nets.cpp src");

cmd(inter, "file copy -force $RootLsd/$LsdSrc/fun_head.h src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/decl.h src");

cmd(inter, "file copy -force $RootLsd/$LsdSrc/system_options.txt src");

cmd(inter, "set f [open src/choose.h w]");
cmd(inter, "puts -nonewline $f \"#define NO_WINDOW\\n\"");
cmd(inter, "close $f");

cmd(inter, "cd $RootLsd"); 

// Compile a local machine version of lsd_gnuNW
cmd(inter, "set fapp [file nativename $modeldir/makefileNW]");
s=(char *)Tcl_GetVar(inter, "fapp",0);
f=fopen(s, "r");
if(f==NULL)
  goto loop;
fscanf(f, "%s", str);
while(strncmp(str, "FUN=", 4) && fscanf(f, "%s", str)!=EOF);
fclose(f);
if(strncmp(str, "FUN=", 4)!=0)
{
  choice=0;
  goto loop;
}
sprintf(msg, "set fname %s.cpp", str+4);

f=fopen(s, "r");
fscanf(f, "%s", str);
while(strncmp(str, "TARGET=", 7) && fscanf(f, "%s", str)!=EOF);
fclose(f);
if(strncmp(str, "TARGET=", 7)!=0)
{
  choice=0;
  goto loop;
}
strcat(str,"NW");
cmd(inter, msg);
cmd(inter, "set init_time [clock seconds]"); 
cmd(inter, "toplevel .t");
// change window icon
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap .t @$RootLsd/$LsdSrc/lmm.xbm} {}");
cmd(inter, "wm title .t \"Please Wait\"");
cmd(inter, "label .t.l1 -font {-weight bold} -text \"Making non-graphical version of model...\"");
cmd(inter, "label .t.l2 -text \"The executable 'lsd_gnuNW' for this system is being created.\nThe make file 'makefileNW' and the 'src' folder are being created\nin the model folder and can be used to recompile the\n'No Window' version in other systems.\"");
cmd(inter, "pack .t.l1 .t.l2");
cmd(inter, "focus -force .t");
cmd( inter, "setgeom .t" );
cmd(inter, "blocklmm .t");
cmd(inter, "update");  

cmd(inter, "cd $modeldir");
cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1;set add_exe \".exe\"} {set choice 0;set add_exe \"\"}");
if(choice==0)
  cmd(inter, "set result \"[catch [exec make -fmakefileNW 2> makemessage.txt]]\""); 
else
{  
  cmd(inter, "set result -2.2");
  cmd(inter, "set file [open a.bat w]");
  cmd(inter, "puts -nonewline $file \"make -fmakefileNW 2> makemessage.txt\\n\"");
  cmd(inter, "close  $file");
  cmd(inter, "if { [file exists $RootLsd/$LsdSrc/system_options.txt] == 1} {set choice 0} {set choice 1}");
  sprintf(msg, "if { [file exists %s.exe]  == 1} {file rename -force %s.exe %sOld.exe} { }", str+7, str+7, str+7);
  cmd(inter, msg);
  cmd(inter, "if { [file exists $RootLsd/$LsdGnu/bin/crtend.o] == 1} { file copy $RootLsd/$LsdGnu/bin/crtend.o .;file copy $RootLsd/$LsdGnu/bin/crtbegin.o .;file copy $RootLsd/$LsdGnu/bin/crt2.o .} {}");
  cmd(inter, "catch [set result [catch [exec a.bat]] ]");
  cmd(inter, "file delete a.bat");
  cmd(inter, "if { [file exists crtend.o] == 1} { file delete crtend.o;file delete crtbegin.o ;file delete crt2.o } {}");
}
cmd(inter, "sblocklmm .t");
cmd(inter, "destroy .t");
cmd(inter, "update");

cmd(inter, "if { [file size makemessage.txt]==0 } {set choice 0} {set choice 1}");
if(choice==1)
{
  cmd(inter, "set funtime [file mtime $fname]");
  sprintf(msg, "if { [file exist %s$add_exe] == 1 } {set exectime [file mtime %s$add_exe]} {set exectime $init_time}",str+7,str+7);
  cmd(inter, msg);
  cmd(inter, "if {$init_time < $exectime } {set choice 0} { }");
  //turn into 0 if the executable is newer than the compilation command, implying just warnings
}
cmd(inter, "cd $RootLsd");
if(choice==1)
  cmd(inter, "tk_messageBox -type ok -icon error -title Error -message \"Problem generating 'No Window' version, probably there is a problem with your model.\\n\\nBefore using this option, make sure you are able to run the model with the 'Model'/'Compile and Run Model' option without errors. The error list is in the file 'makemessage.txt'.\"");
else
  cmd(inter, "tk_messageBox -type ok -icon info -title \"Info\" -message \"LMM has created a non-graphical version of the model, to be transported on any system endowed with a GCC compiler and standard libraries.\\n\\nA local system version of the executable 'lsd_gnuNW' was also generated in your current model folder and is ready to use in this computer.\\n\\nTo move the model in another system copy the content of the model's directory:\\n$modeldir\\nincluding also its new subdirectory 'src'.\\n\\nTo create a 'No Window' version of the model program follow these steps, to be executed within the directory of the model:\\n- compile with the command 'make -f makefileNW'\\n- run the model with the command 'lsd_gnuNW -f mymodelconf.lsd'\\n- the simulation will run automatically saving the results (for the variables indicated in the conf. file) in LSD result files named after the seed generator used.\"");
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

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Change Tab Size\"");
cmd(inter, "label .a.l1 -text \"Enter the Tab size (in characters)\"");
cmd(inter, "entry .a.v_num -justify center -width 10 -textvariable tabsize");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.f.ok}");
cmd(inter, "frame .a.f");
cmd(inter, "button .a.f.ok -padx 25 -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.help -padx 20 -text Help -command {LsdHelp LMM_help.html#changetab}");
cmd(inter, "button .a.f.esc -padx 15 -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -padx 1 -pady 5 -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.f");

cmd( inter, "setgeom .a" );
cmd(inter, "focus -force .a.v_num");
cmd(inter, ".a.v_num selection range 0 end");
cmd(inter, "blocklmm .a");
choice=0;
while(choice==0)
	Tcl_DoOneEvent(0);
cmd(inter, "sblocklmm .a");
if(choice==2)
{
	choice=0;
	goto loop;
}
cmd(inter, "settab .f.t.t $tabsize \"[.f.t.t cget -font]\"");
choice=0;
goto loop;
}

if ( choice == 68 )
{	// Adjust context menu for LSD macros
	cmd( inter, "destroy .v.i");
	cmd( inter, "menu .v.i -tearoff 0");
	cmd( inter, ".v.i add command -label \"Lsd equation\" -command {set choice 25} -accelerator Ctrl+E" );
	cmd( inter, ".v.i add command -label \"V(...)\" -command {set choice 26} -accelerator Ctrl+V" );
	cmd( inter, ".v.i add command -label \"SUM(...)\" -command {set choice 56} -accelerator Ctrl+U" );
	cmd( inter, ".v.i add command -label \"SEARCH_CND(...)\" -command {set choice 30} -accelerator Ctrl+S" );
	cmd( inter, ".v.i add command -label \"SEARCH(...)\" -command {set choice 55} -accelerator Ctrl+A" );
	cmd( inter, ".v.i add command -label \"WRITE(...)\" -command {set choice 29} -accelerator Ctrl+W" );
	cmd( inter, ".v.i add command -label \"CYCLE(...)\" -command {set choice 27} -accelerator Ctrl+C" );
	cmd( inter, ".v.i add command -label \"SORT(...)\" -command {set choice 31} -accelerator Ctrl+T" );
	cmd( inter, ".v.i add command -label \"ADDOBJ(...)\" -command {set choice 52} -accelerator Ctrl+O" );
	cmd( inter, ".v.i add command -label \"DELETE(...))\" -command {set choice 53} -accelerator Ctrl+D" );
	cmd( inter, ".v.i add command -label \"RNDDRAW(...)\" -command {set choice 54} -accelerator Ctrl+N" );
	cmd( inter, ".v.i add command -label \"INCR(...)\" -command {set choice 40} -accelerator Ctrl+I" );
	cmd( inter, ".v.i add command -label \"MULT(...)\" -command {set choice 45} -accelerator Ctrl+M" );
	cmd( inter, ".v.i add command -label \"Math operation\" -command {set choice 51} -accelerator Ctrl+H" );
	choice = 0;
	goto loop;
}

if ( choice == 69 )
{	// Adjust context menu for LSD C++
	cmd( inter, "destroy .v.i");
	cmd( inter, "menu .v.i -tearoff 0");
	cmd( inter, ".v.i add command -label \"Lsd equation\" -command {set choice 25} -accelerator Ctrl+E" );
	cmd( inter, ".v.i add command -label \"cal(...)\" -command {set choice 26} -accelerator Ctrl+V" );
	cmd( inter, ".v.i add command -label \"sum(...)\" -command {set choice 56} -accelerator Ctrl+U" );
	cmd( inter, ".v.i add command -label \"search_var_cond(...)\" -command {set choice 30} -accelerator Ctrl+S" );
	cmd( inter, ".v.i add command -label \"Search(...)\" -command {set choice 55} -accelerator Ctrl+A" );
	cmd( inter, ".v.i add command -label \"write(...)\" -command {set choice 29} -accelerator Ctrl+W" );
	cmd( inter, ".v.i add command -label \"for( ; ; )\" -command {set choice 27} -accelerator Ctrl+C" );
	cmd( inter, ".v.i add command -label \"lsdqsort(...)\" -command {set choice 31} -accelerator Ctrl+T" );
	cmd( inter, ".v.i add command -label \"Add a new object\" -command {set choice 52} -accelerator Ctrl+O" );
	cmd( inter, ".v.i add command -label \"Delete an object\" -command {set choice 53} -accelerator Ctrl+D" );
	cmd( inter, ".v.i add command -label \"Draw random an object\" -command {set choice 54} -accelerator Ctrl+N" );
	cmd( inter, ".v.i add command -label \"increment(...)\" -command {set choice 40} -accelerator Ctrl+I" );
	cmd( inter, ".v.i add command -label \"multiply(...)\" -command {set choice 45} -accelerator Ctrl+M" );
	cmd( inter, ".v.i add command -label \"Math operation\" -command {set choice 51} -accelerator Ctrl+H" );
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
Tcl_UnlinkVar(inter, "shigh");

Tcl_Exit(0);

exit(0);

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
	"MODEL(BEGIN|END)|(END_)?EQUATION|FUNCTION|RESULT|DEBUG(_AT)?|CURRENT|V[LS]*(_CHEAT)?|V(S)?_(NODEID)?(NODENAME)?(WEIGHT)?|SUM|SUM[LS]*|STAT(S)?(_NET)?(_NODE)?|WHTAVE[LS]*|INCR(S)?|MULT(S)?|CYCLE(S)?(_LINK)?|CYCLE_SAFE(S)?|MAX[LS]*|WRITE[LS]*(_NODEID)?(_NODENAME)?(_WEIGHT)?|SEARCH_CND[LS]*|SEARCH(S)?(_NET)?(_LINK)?|TSEARCHS(_INI)?|SORT[S2]*|ADD(N)?OBJ(S)?(_EX)?|DELETE|RND|UNIFORM|RNDDRAW(FAIR)?(TOT)?[LS]*(_NET)?|PARAMETER|INTERACT(S)?|rnd_integer|norm|poisson|gamma|init_lattice|update_lattice|NETWORK(S)?(_INI)?(_LOAD)?(_SAVE)?|SHUFFLE(S)?|ADDLINK[WS]*|DELETELINK|LINK(TO|FROM)",
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
	sprintf(msg, ".f.t.t tag remove %s %ld.0 %s", cTypes[i], iniLin == 0 ? 1 : iniLin, finStr);
	cmd(inter, msg);
}

// find & copy all occurrence types to arrays of C strings
for(i = 0; i < maxColor; i++)
{
	// locate all occurrences of each color group
	Tcl_UnsetVar(inter, "ccount", 0);
	if(!strcmp(cTypes[i], "comment1"))	// multi line search element?
		sprintf(msg, "set pos [.f.t.t search -regexp -all -nolinestop -count ccount -- {%s} %ld.0 %s]", cRegex[i], iniLin == 0 ? 1 : iniLin, finStr);
	else
		sprintf(msg, "set pos [.f.t.t search -regexp -all -count ccount -- {%s} %ld.0 %s]", cRegex[i], iniLin == 0 ? 1 : iniLin, finStr);
	cmd(inter, msg);

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
		cmd(inter, "set end [.f.t.t index \"$lin.$col + $cnt char\"]");
		// treats each type of color case properly
		if(hits[k].type < 4)			// non token?
			sprintf(msg, ".f.t.t tag add %s $lin.$col $end", cTypes[hits[k].type]);
		else							// token - should not be inside another word
			sprintf(msg, "if {[regexp {\\w} [.f.t.t get \"$lin.$col - 1 any chars\"]]==0 && [regexp {\\w} [.f.t.t get $end]]==0} {.f.t.t tag add %s $lin.$col $end}", cTypes[hits[k].type]);
		cmd(inter, msg);
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

 

cmd(inter, "set choice [file exists model_options.txt]");
if(choice==0)
 {//the model_options.txt file does not exists, probably an old version
   cmd(inter, "set dir [glob *.cpp]");
   cmd(inter, "set b [lindex $dir 0]");
   cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\nSWITCH_CC_LNK=\\n\"");
   cmd(inter, "set f [open model_options.txt w]");
   cmd(inter, "puts -nonewline $f $a");
   cmd(inter, "close $f");
 }
choice=0; 
cmd(inter, "set f [open model_options.txt r]");
cmd(inter, "set a [read -nonewline $f]");
cmd(inter, "close $f");



cmd(inter, "set choice [file exists $RootLsd/$LsdSrc/system_options.txt]");
if(choice==0)
 { //the src/system_options.txt file doesn't exists, so I invent it
   cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {file copy $RootLsd/$LsdSrc/sysopt_windows.txt $RootLsd/$LsdSrc/system_options.txt} {file copy $RootLsd/$LsdSrc/sysopt_linux.txt $RootLsd/$LsdSrc/system_options.txt}");
 }
choice=0; 

cmd(inter, "set f [open $RootLsd/$LsdSrc/system_options.txt r]");
cmd(inter, "set d [read -nonewline $f]");
cmd(inter, "close $f");

cmd(inter, "set f [open $RootLsd/$LsdSrc/makefile_base.txt r]");
cmd(inter, "set b [read -nonewline $f]");
cmd(inter, "close $f");


cmd(inter, "set c \"# Model compilation options\\n$a\\n\\n# System compilation option\\n$d\\nLSDROOT=$RootLsd\\n\\n# body of the makefile (in src/makefile_base.txt)\\n$b\"");
cmd(inter, "set f [open makefile w]");
cmd(inter, "puts -nonewline $f $c");
cmd(inter, "close $f");
cmd(inter, "update");
//cmd(inter, "if {$tcl_platform(platform) == \"windows\" && [file exists crt0.o] == 0} { file copy $RootLsd/$LsdGnu/bin/crt0.o .} {}");
}





void make_makefileNW(void)
{

 
cmd(inter, "set choice [file exists model_options.txt]");
if(choice==0)
 {//the model_options.txt file does not exists, probably an old version
   cmd(inter, "set dir [glob *.cpp]");
   cmd(inter, "set b [lindex $dir 0]");
   cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\nSWITCH_CC_LNK=\\n\"");
   cmd(inter, "set f [open model_options.txt w]");
   cmd(inter, "puts -nonewline $f $a");
   cmd(inter, "close $f");
 }
choice=0; 
cmd(inter, "set f [open model_options.txt r]");
cmd(inter, "set a [read -nonewline $f]");
cmd(inter, "close $f");



cmd(inter, "set choice [file exists $RootLsd/$LsdSrc/system_options.txt]");
if(choice==0)
 { //the src/system_options.txt file doesn't exists, so I invent it
   cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {file copy $RootLsd/$LsdSrc/sysopt_windows.txt $RootLsd/$LsdSrc/system_options.txt} {file copy $RootLsd/$LsdSrc/sysopt_linux.txt $RootLsd/$LsdSrc/system_options.txt}");
 }
choice=0; 

cmd(inter, "set f [open $RootLsd/$LsdSrc/system_options.txt r]");
cmd(inter, "set d [read -nonewline $f]");
cmd(inter, "close $f");

cmd(inter, "set f [open $RootLsd/$LsdSrc/makefile_baseNW.txt r]");
cmd(inter, "set b [read -nonewline $f]");
cmd(inter, "close $f");


cmd(inter, "set c \"# Model compilation options\\n$a\\n\\n# System compilation option\\n$d\\n\\n# body of the makefile (in src/makefile_base.txt)\\n$b\"");
cmd(inter, "set f [open makefileNW w]");
cmd(inter, "puts -nonewline $f $c");
cmd(inter, "close $f");
cmd(inter, "update");
//cmd(inter, "if {$tcl_platform(platform) == \"windows\" && [file exists crt0.o] == 0} { file copy $RootLsd/$LsdGnu/bin/crt0.o .} {}");
}



void signal(char *s)
{
FILE *f;
f=fopen("signal.txt", "w");
fprintf(f, "%s",s);
fclose(f);
}





void create_compresult_window(void)
{

cmd(inter, "set a [winfo exists .mm]");
cmd(inter, "if {$a==1} {destroy .mm} {}");

cmd(inter, "set cerr 0.0");
cmd(inter, "toplevel .mm");
// change window icon
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap .mm @$RootLsd/$LsdSrc/lmm.xbm} {}");
cmd(inter, "label .mm.lab -justify left -text \"- Each error is indicated by the file name and line number where it has been identified.\n- Check the relative file and search on the indicated line number, considering that the error may have occurred in the previous line.\n- Fix first errors at the beginning of the list, since the following errors may be due to previous ones.\"");
cmd(inter, "pack .mm.lab");

cmd(inter, "text .mm.t -yscrollcommand \".mm.yscroll set\" -wrap word; scrollbar .mm.yscroll -command \".mm.t yview\"");

cmd(inter, "pack .mm.yscroll -side right -fill y; pack .mm.t -expand yes -fill both; wm geom .mm \"+$hmargin+$vmargin\"; wm title .mm \"Compilation Errors\"");

cmd(inter, "frame .mm.b");

cmd(inter, "set error \"error\"");				// error string to be searched
cmd(inter, "button .mm.b.ferr -text \" Next error \" -command {set errtemp [.mm.t search -nocase -regexp -count errlen -- $error $cerr end]; if { [string length $errtemp] == 0} {} { set cerr \"$errtemp + $errlen ch\"; .mm.t mark set insert $cerr; .mm.t tag remove sel 1.0 end; .mm.t tag add sel \"$errtemp linestart\" \"$errtemp lineend\"; .mm.t see $errtemp;} }");

cmd(inter, "button .mm.b.perr -text \" Previous error \" -command {set errtemp [ .mm.t search -nocase -regexp -count errlen -backward -- $error $cerr 0.0];  if { [string length $errtemp] == 0} {} { set cerr \"$errtemp - 1ch\"; .mm.t mark set insert $errtemp; .mm.t tag remove sel 1.0 end; .mm.t tag add sel \"$errtemp linestart\" \"$errtemp lineend\"; .mm.t see $errtemp} }");

cmd(inter, "pack .mm.b.perr .mm.b.ferr -padx 10 -pady 5 -expand yes -fill x -side left");
cmd(inter, "pack .mm.b -expand yes -fill x");
cmd(inter, "button .mm.close -padx 15 -text Cancel -command {destroy .mm}");
cmd(inter, "pack .mm.close -pady 5 -side bottom");

cmd(inter, "bind .mm <Escape> {destroy .mm}");
cmd(inter, "focus -force .mm.t");
cmd(inter, "set file [open $modeldir/makemessage.txt]");
cmd(inter, ".mm.t insert end [read $file]");
cmd(inter, "close $file");
cmd(inter, ".mm.t mark set insert \"1.0\";");
cmd(inter, "set errtemp [.mm.t search -nocase -regexp -count errlen -- $error $cerr end]; if { [string length $errtemp] == 0} {} { set cerr \"$errtemp + $errlen ch\"; .mm.t mark set insert $cerr; .mm.t tag remove sel 1.0 end; .mm.t tag add sel \"$errtemp linestart\" \"$errtemp lineend\"; .mm.t see $errtemp;}");
}

// delete any previously open compilation results window
void delete_compresult_window( void )
{
	cmd( inter, "set a [winfo exists .mm]" );
	cmd( inter, "if { $a == 1 } { destroy .mm } { }" );
}
