/***************************************************
****************************************************
LSD 6.3 - May 2014
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/


/*****
used up to 62 options included
*******/

/* TEST UNDO:

Set uscounter=-1
Command to store:
bind .t <KeyPress-Return> {lappend ud [.t get 0.0 "end - 1 chars"]; lappend udi [.t index insert]; incr udcounter}
<<repeat for space and any other word terminal of choice>>

Command to retrieve:
bind .t <Control-z> {if {$udcounter < 0 } {} {.t delete 0.0 end; .t insert 0.0 [lindex $ud $udcounter]; .t delete end; .t see [lindex $udi $udcounter]; .t mark set insert [lindex $udi $udcounter]; incr udcounter -1}}

bind .t <Control-z> {if {[llength $ud] ==0 } {} {lappend rd [.t get 0.0 "end - 1 chars"]; lappend rdi [.t index insert]; .t delete 0.0 end; .t insert 0.0 [lindex $ud end]; .t delete end; .t see [lindex $udi end]; .t mark set insert [lindex $udi end]; set ud [lreplace $ud end end]; set udi [lreplace $udi end end]}}

bind .t <Control-y> {if {[llength $rd] ==0} {} {lappend ud [.t get 0.0 "end -1 chars"]; lappend udi [.t index insert]; .t delete 0.0 end; .t insert 0.0 [lindex $rd end]; .t delete end; .t see [lindex $rdi end]; .t mark set insert [lindex $rdi end]; set rd [lreplace $rd end end]; set rdi [lreplace $rdi end end]} }

- Adjust for controlling of negative udcounter
- Adjust the "see", probably to replace with yview


ERROR! The "lappend" command increases always the list. When an "undo" is done, , the list should be shortened.
********************/

/*****************************************************
This program is a front end for dealing with Lsd models code (running, compiling, editing, debugging 
Lsd model programs). See the manual for help on its use (really needed?)

IMPORTANT: this is _NOT_ a Lsd model, but the best I could produce of something similar to 
a programming environment for Lsd model programs.


This file can be compiled with the command line:

gcc -g src\modman.cpp -Lgnu/lib -ltcl83 -ltk83 -Ignu/include -o lmm -Wl,--subsystem,windows

or

gcc -g modman.cpp -ltcl8.3 -ltk8.3 -o lmm

for Windows or Unix systems respectively (changes may be required depending on your installation directories.


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




Tcl_Interp *inter;
char msg[300];
int choice;
int v_counter=0; //counter of the v[i] variables inserted in the equations

void copy(char *f1, char *f2);
Tcl_Interp *InterpInitWin(void);
void cmd(Tcl_Interp *inter, char *cc);
void cmd(Tcl_Interp *inter, const char cc[]) {cmd(inter, (char *)cc);};
int errormsg( char *lpszText,  char *lpszTitle);
int ModManMain(int argn, char **argv);
void color(int *num);
void make_makefile(void);
void make_makefileNW(void);
char app_str[2][200];
void signal(char *s);
void create_compresult_window(void);

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
    strcat(here, "/gnu/bin");
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
This function presumes the installation of a /gnu directory along
the model's one. Tcl and Tk initialization files MUST be in
/gnu/share/tcl8.0
/gnu/share/tk8.0
*********************************/
Tcl_Interp *InterpInitWin(void)
{
Tcl_Interp *app;
char *s;
int res;
app=Tcl_CreateInterp();

//cmd(app, "puts $tcl_library");
//cmd(app, "source init.tcl");
//cmd(app, "source C:/Lsd5.5/gnu/lib/tk8.4/tk.tcl");

//Tcl_SetVar(app, "tcl_library", "C:/Lsd5.5/gnu/lib/tcl8.4", TCL_APPEND_VALUE);
/*
cmd(app, "puts $tcl_library");
return app;


    cmd(app, "set tk_library {gnu/lib/tk8.4}");
*/    
//cmd(app, "lappend tcl_libPath {gnu/lib/tcl8.4}");

if(Tcl_Init(app)!=TCL_OK)
 {
  exit(1);
 
 }

//cmd(app, "set env(DISPLAY) :0.0");
if(Tk_Init(app)!=TCL_OK)
 {
  errormsg( (char *)"Tk Initialization failed. Check directory structure:\nLSDHOME\\lib\\share\\tk8.0\n", NULL);
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

if(code!=TCL_OK)
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
char str[500], str1[50];
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
cmd(inter, "if {$tcl_platform(os) == \"Windows NT\"} {set DefaultWish wish; set DefaultTerminal cmd; set DefaultHtmlBrowser open; set DefaultFont Courier} {}");
cmd(inter, "if {$tcl_platform(os) == \"Darwin\"} {set DefaultWish wish8.5; set DefaultTerminal terminal; set DefaultHtmlBrowser open; set DefaultFont Courier} {}");

cmd(inter, "set Terminal $DefaultTerminal");
cmd(inter, "set HtmlBrowser $DefaultHtmlBrowser");
cmd(inter, "set fonttype $DefaultFont");
cmd(inter, "set wish $DefaultWish");
cmd(inter, "set LsdSrc src");

cmd(inter, "set choice [file exist $RootLsd/lmm_options.txt]");
if(choice==1)
 {
  cmd(inter, "set f [open $RootLsd/lmm_options.txt r]");
  cmd(inter, "gets $f Terminal");
  cmd(inter, "gets $f HtmlBrowser");
  cmd(inter, "gets $f fonttype");
  cmd(inter, "gets $f wish");
  cmd(inter, "gets $f LsdSrc");
  cmd(inter, "close $f");
 }
else
 {
  cmd(inter, "set f [open $RootLsd/lmm_options.txt w]");
  cmd(inter, "puts $f $Terminal");
  cmd(inter, "puts $f $HtmlBrowser");
  cmd(inter, "puts $f $fonttype");
  cmd(inter, "puts $f $wish");  
  cmd(inter, "puts $f $LsdSrc");
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
 {cmd(inter, "if { $tcl_platform(platform) == \"windows\"} {set sysfile \"sysopt_windows.txt\"} { if { $tcl_platform(os) == \"\"} {set sysfile \"sysopt_mac.txt\"} {set sysfile \"sysopt_linux.txt\"}}");
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

//cmd(inter, "proc blocklmm w {set curfoc [focus -displayof .]; tk_messageBox -message \"My focus is\n$curfoc\nLast for focus is\n[focus -displayof $w]\"; bind $w <Leave> {set curfoc [focus -displayof $w]};  bind .f.t.t <1> {focus -force $curfoc};  wm transient $w .;.f.t.t conf -state disabled}");
//tk_messageBox -message \"My focus is\n[focus -lastfor $w]\nLast for focus is\n[focus -displayof $w]\";
cmd(inter, "proc blocklmm w {  bind .f.t.t <1> { raise $w .; focus -force [focus -lastfor $w]};  wm transient $w .;bind . <Escape> {sblocklmm $w}; bind . <1> {sblocklmm $w}; .f.t.t conf -state disabled}");
cmd(inter, "proc sblocklmm w {bind .f.t.t <1> {}; .f.t.t conf -state normal; destroy $w; focus -force .f.t.t; bind .f.t.t <Escape> {} }");
cmd(inter, "wm title . \"LMM - Lsd Model Manager\"");
cmd(inter, "wm protocol . WM_DELETE_WINDOW {set choice 1}");
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
cmd(inter, "source $RootLsd/$LsdSrc/showmodel.tcl");
cmd(inter, "source $RootLsd/$LsdSrc/lst_mdl.tcl");
Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
Tcl_LinkVar(inter, "tosave", (char *) &tosave, TCL_LINK_INT);
Tcl_LinkVar(inter, "macro", (char *) &macro, TCL_LINK_INT);
macro=1;    
  

cmd(inter, "menu .m -tearoff 0");

cmd(inter, "set w .m.file");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label File -menu $w -underline 0");
cmd(inter, "$w add command -label \"New text file\" -command { set choice 39} -underline 0");
cmd(inter, "$w add command -label \"Open File\" -command { set choice 15} -underline 0 -accelerator Control+o");
cmd(inter, "$w add command -label \"Save\" -command { if {[string length $filename] > 0} {if { [file exist $dirname/$filename] == 1} {catch {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak}} {}; set f [open $dirname/$filename w];puts -nonewline $f [.f.t.t get 0.0 end]; close $f; set before [.f.t.t get 0.0 end]} {}} -underline 0 -accelerator Control+s");
cmd(inter, "$w add command -label \"Save as\" -command {set choice 4} -underline 5 -accelerator Control+a");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Quit\" -command {; set choice 1} -underline 0 -accelerator Control+q");

//cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {$w add command -label \"System Options\" -command { set choice 60} } {}");

cmd(inter, "$w add command -label \"System Options\" -command { set choice 60} ");

cmd(inter, "set w .m.edit");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Edit -menu $w -underline 0");
cmd(inter, "$w add command -label \"Copy\" -command {tk_textCopy .f.t.t} -underline 0 -accelerator Control+c");
cmd(inter, "$w add command -label \"Cut\" -command {tk_textCut .f.t.t} -underline 1");
cmd(inter, "$w add command -label \"Paste\" -command {tk_textPaste .f.t.t} -underline 0 -accelerator Control+v");
cmd(inter, "$w add command -label \"Undo\" -command {if {[llength $ud] ==0 } {} {lappend rd [.f.t.t get 0.0 \"end - 1 chars\"]; lappend rdi [.f.t.t index insert]; .f.t.t delete 0.0 end; .f.t.t insert 0.0 [lindex $ud end]; .f.t.t delete end; .f.t.t see [lindex $udi end]; .f.t.t mark set insert [lindex $udi end]; set ud [lreplace $ud end end]; set udi [lreplace $udi end end]; set choice 23}} -underline 0 -accelerator Control+z");
cmd(inter, "$w add command -label \"Redo\" -command {if {[llength $rd] ==0} {} {lappend ud [.f.t.t get 0.0 \"end -1 chars\"]; lappend udi [.f.t.t index insert]; .f.t.t delete 0.0 end; .f.t.t insert 0.0 [lindex $rd end]; .f.t.t delete end; .f.t.t see [lindex $rdi end]; .f.t.t mark set insert [lindex $rdi end]; set rd [lreplace $rd end end]; set rdi [lreplace $rdi end end]; set choice 23} } -underline 2 -accelerator Control+y");

cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Goto Line\" -command {set choice 10} -underline 5 -accelerator Control+l");
cmd(inter, "$w add command -label \"Find\" -command {set choice 11} -underline 0 -accelerator Control+f");
cmd(inter, "$w add command -label \"Find Again\" -command {set choice 12} -underline 5 -accelerator F3");
cmd(inter, "$w add command -label \"Replace\" -command {set choice 21} -underline 0");
cmd(inter, "$w add separator");

cmd(inter, "$w add command -label \"Match \\\{ \\}\" -command {set choice 17} -underline 0 -accelerator Control+m");
cmd(inter, "$w add command -label \"Match \\\( \\)\" -command {set choice 32} -underline 0 -accelerator Control+p");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Insert \\\{\" -command {.f.t.t insert insert \\\{} -accelerator Control+\\\(");
cmd(inter, "$w add command -label \"Insert \\}\" -command {.f.t.t insert insert \\}} -accelerator Control+\\)");
cmd(inter, "$w add command -label \"Insert Lsd scripts\" -command {set choice 28} -underline 0 -accelerator Control+i");
cmd(inter, "$w add separator");
cmd(inter, "$w add check -label \" Wrap/Unwrap\" -variable wrap -command {if {$wrap == 1} {.f.t.t conf -wrap word } {.f.t.t conf -wrap none}} -underline 1 -accelerator Control+w ");
cmd(inter, "$w add command -label \"Larger Font\" -command {incr dim_character 1; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"} -accelerator Control+'+'");
cmd(inter, "$w add command -label \"Smaller Font\" -command {incr dim_character -1; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"} -accelerator Control+'-'");
cmd(inter, "$w add command -label \"Change fonts\" -command {set choice 59} ");
cmd(inter, "$w add command -label \"Indent Selection\" -command {set choice 42} -accelerator Control+>");
cmd(inter, "$w add command -label \"De-indent Selection\" -command {set choice 43} -accelerator Control+<");

cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"TkDiff\" -command {set choice 57}");
cmd(inter, "$w add command -label \"Compare models\" -command {set choice 61}");
//cmd(inter, "$w add command -label \"LMM Options\" -command {set choice 63}");


cmd(inter, "set w .m.model");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Model -menu $w -underline 0");
cmd(inter, "$w add cascade -label \"Equations' Coding Style\" -menu $w.macro");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Browse Models\" -underline 0 -command {set choice 33}");
//cmd(inter, "$w add command -label \"New Model\" -underline 0 -command { set choice 14}");
//cmd(inter, "$w add command -label \"Copy Model\" -state disabled -underline 0 -command { set choice 41}");
cmd(inter, "$w add separator");
//cmd(inter, "$w add command -label \"Compile\" -state disabled -underline 2 -command {set choice 6}");
cmd(inter, "$w add command -label \"Compile and Run Model\" -state disabled -underline 0 -command {set choice 2} -accelerator Control+r");
cmd(inter, "$w add command -label \"gdb Debug\" -state disabled -underline 0 -command {set choice 13}");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Show Equations File\" -state disabled -underline 5 -command {set choice 8} -accelerator Control+e");
cmd(inter, "$w add command -label \"Show Makefile\" -state disabled -underline 7 -command { set choice 3}");
cmd(inter, "$w add command -label \"Show Compilation Results\" -underline 6 -state disabled -command {set choice 7}");
cmd(inter, "$w add command -label \"Show Description\" -underline 1 -state disabled -command {set choice 5}");
cmd(inter, "$w add command -label \"Model Info\" -underline 6 -state disabled -command {set choice 44}");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"System Compilation Options\" -command {set choice 47}");
cmd(inter, "$w add command -label \"Model Compilation Options\" -state disabled -command {set choice 48}");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Generate 'NO WINDOW' makefile\" -command {set choice 62}");

cmd(inter, "menu $w.macro -tearoff 0");
cmd(inter, "$w.macro add radio -label \" Use Lsd Macros\" -variable macro -value 1 -command {.m.help entryconf 1 -label \"Help on Macros for Lsd Equations\" -underline 6 -command {LsdHelp lsdfuncMacro.html}}");
cmd(inter, "$w.macro add radio -label \" Use Lsd C++\" -variable macro -value 0 -command {.m.help entryconf 1 -label \"Help on C++ for Lsd Equations\" -underline 8 -command {LsdHelp lsdfunc.html}}");


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
cmd(inter, "$w add command -label \"Lsd documentation\" -command {LsdHelp Lsd_Documentation.html}");

cmd(inter, "$w add command -label \"About LMM + Lsd\" -command {if { [winfo exists .about]==1} {destroy .about } {}; toplevel .about; label .about.l -text \"Version 6.3 \n\nMay 2014\n\n\"; button .about.ok -text \"Ok\" -command {destroy .about}; pack .about.l .about.ok; wm title .about \"\"}"); 


cmd(inter, "frame .f");
cmd(inter, "frame .f.t -relief groove -bd 2");
cmd(inter, "scrollbar .f.t.vs -command \".f.t.t yview\"");
cmd(inter, "scrollbar .f.t.hs -orient horiz -command \".f.t.t xview\"");
cmd(inter, "text .f.t.t -height 2 -tabs 20 -undo 1 -wrap none -bg #fefefe -yscroll \".f.t.vs set\" -xscroll \".f.t.hs set\"");
cmd(inter, "set a [.f.t.t conf -font]");
cmd(inter, "set b [lindex $a 3]");
cmd(inter, "set dim_character [lindex $b 1]");
//cmd(inter, "set fonttype [lindex $b 0]");
cmd(inter, "if { $dim_character == \"\"} {set dim_character 12} {}");

cmd(inter, "set a [list $fonttype $dim_character]");
cmd(inter, ".f.t.t conf -font \"$a\"");
//cmd(inter, "set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"");
//
cmd(inter, ".f.t.t tag configure comment1 -foreground #00aa00");
cmd(inter, ".f.t.t tag configure comment2 -foreground #00aa00");
cmd(inter, ".f.t.t tag configure str -foreground blue");
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


cmd(inter, "pack .f.hea -fill x");
cmd(inter, "pack .f.t -expand yes -fill both");
cmd(inter, "pack .f.t.vs -side right -fill y");
cmd(inter, "pack .f.t.t -expand yes -fill both");
cmd(inter, "pack .f.t.hs -fill x");


cmd(inter, "set dir [glob *]");
cmd(inter, "set num [llength $dir]");



cmd(inter, "set wrap 0");

cmd(inter, "bind . <Control-n> {tk_menuSetFocus .m.file}");

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
cmd(inter, "bind .f.t.t <KeyRelease-backslash> {.f.hea.line.line conf -text [.f.t.t index insert]; set choice 23}");


cmd(inter, "bind .f.t.t <Control-l> {set choice 10}");
cmd(inter, "bind .f.t.t <Control-w> {if {$wrap == 0} {.f.t.t conf -wrap word; set wrap 1} {.f.t.t conf -wrap none; set wrap 0 }}");

cmd(inter, "bind .f.t.t <Control-f> {set choice 11}");
cmd(inter, "bind .f.t.t <F3> {set choice 12}");
cmd(inter, "bind .f.t.t <Control-s> { if {[string length $filename] > 0} {if { [file exist $dirname/$filename] == 1} {catch {file copy -force $dirname/$filename $dirname/[file rootname $filename].bak}} {}; set f [open $dirname/$filename w];puts -nonewline $f [.f.t.t get 0.0 end]; close $f; set before [.f.t.t get 0.0 end]} {}}"); 
cmd(inter, "bind .f.t.t <Control-a> { set choice 4}");
cmd(inter, "bind .f.t.t <Control-r> {set choice 2}");
cmd(inter, "bind .f.t.t <Control-e> {set choice 8}");
cmd(inter, "bind .f.t.t <Control-KeyRelease-o> {if {$tk_strictMotif == 0} {set a [.f.t.t index insert]; .f.t.t delete \"$a lineend\"} {}; set choice 15; break}");
cmd(inter, "bind .f.t.t <Control-q> {set choice 1}");
cmd(inter, "bind .f.t.t <Control-p> {set choice 32}");
cmd(inter, "bind .f.t.t <Control-u> {set choice 32}");
cmd(inter, "bind .f.t.t <Control-m> {set choice 17}");



//cmd(inter, "bind .f.t.t <Control-c> {tk_textCopy .f.t.t}");
//cmd(inter, "bind .f.t.t <Control-v> {tk_textPaste .f.t.t}");

cmd(inter, "bind .f.t.t <Control-minus> {incr dim_character -2; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"}");

cmd(inter, "bind .f.t.t <Control-plus> {incr dim_character 2; set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"}");

cmd(inter, "bind .f.t.t <Control-parenleft> {.f.t.t insert insert \\\{}");
cmd(inter, "bind .f.t.t <Control-parenright> {.f.t.t insert insert \\}}");
cmd(inter, "bind .f.t.t <Control-greater> {set choice 42}");
cmd(inter, "bind .f.t.t <Control-less> {set choice 43}");
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
POPUP manu, to be completed
*/
cmd(inter, "menu .v -tearoff 0");

cmd(inter, "bind .f.t.t  <3> {%W mark set insert [.f.t.t index @%x,%y]; set vmenuInsert [.f.t.t index insert]; tk_popup .v %X %Y}");
cmd(inter, "bind .f.t.t  <2> {%W mark set insert [.f.t.t index @%x,%y]; set vmenuInsert [.f.t.t index insert]; tk_popup .v %X %Y}");
cmd(inter, ".v add command -label \"Copy\" -command {tk_textCopy .f.t.t}");
cmd(inter, ".v add command -label \"Cut\" -command {tk_textCut .f.t.t}");
cmd(inter, ".v add command -label \"Paste\" -command {tk_textPaste .f.t.t}");

cmd(inter, ".v add separator");
cmd(inter, ".v add cascade -label \"Insert Lsd Script \" -command {set choice 28}");
cmd(inter, ".v add command -label \"Indent Selection\" -command {set choice 42}");
cmd(inter, ".v add command -label \"De-indent Selection\" -command {set choice 43}");
cmd(inter, ".v add command -label \"Place a break and run gdb\" -command {set choice 58}");

cmd(inter, ".v add separator");
cmd(inter, ".v add command -label \"Find\" -command {set choice 11}");
cmd(inter, ".v add command -label \"Match \\\{ \\}\" -command {set choice 17}");
cmd(inter, ".v add command -label \"Match \\\( \\)\" -command {set choice 32}");


cmd(inter, "menu .v.i -tearoff 0");
cmd(inter, ".v.i add command -label \"Lsd Equation\" -command {set choice 25}");
cmd(inter, ".v.i add command -label \"cal(...)\" -command {set choice 26}");
cmd(inter, ".v.i add command -label \"SUM\" -command {set choice 56}");
cmd(inter, ".v.i add command -label \"Math Operation\" -command {set choice 51}");

cmd(inter, ".v.i add command -label \"write(...)\" -command {set choice 29}");
cmd(inter, ".v.i add command -label \"increment(...)\" -command {set choice 40}");
cmd(inter, ".v.i add command -label \"multiply(...)\" -command {set choice 45}");
cmd(inter, ".v.i add command -label \"lsdqsort(...)\" -command {set choice 31}");
cmd(inter, ".v.i add command -label \"Add a new Object\" -command {set choice 52}");
cmd(inter, ".v.i add command -label \"Delete an Object\" -command {set choice 53}");
cmd(inter, ".v.i add command -label \"Draw Rnd. an Object\" -command {set choice 54}");
cmd(inter, ".v.i add command -label \"Search\" -command {set choice 55}");
cmd(inter, ".v.i add command -label \"search_var_cond(...)\" -command {set choice 30}");
cmd(inter, ".v.i add command -label \"for( ; ; )\" -command {set choice 27}");

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
cmd(inter, "bind .f.t.t <Control-P> {set choice 51}");


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
    cmd(inter, "wm title . \"LMM - $filename\"");          
     
     sprintf(msg, "set s [file extension \"$filetoload\"]" );
     cmd(inter, msg);
     s=(char *)Tcl_GetVar(inter, "s",0);
     choice=0;
     if(s[0]!='\0')
       {strcpy(str, s);
        if(!strcmp(str, ".cpp") || !strcmp(str, ".c") || !strcmp(str, ".C") || !strcmp(str, ".CPP") || !strcmp(str, ".Cpp") || !strcmp(str, ".c++") || !strcmp(str, ".C++")|| !strcmp(str, ".h")|| !strcmp(str, ".H"))
          {sourcefile=1;
           cmd(inter, "set inicolor \"1.0\"");
           cmd(inter, "set endcolor [.f.t.t index end]");
           color(&num);
          }
        else
          sourcefile=0;
       }

    }
   else
    {
    cmd(inter, "toplevel .a");
    cmd(inter, "label .a.l1 -text \"File:\"");
    cmd(inter, "label .a.l2 -text \"$filetoload\" -foreground red");
    cmd(inter, "label .a.l3 -text \"not found\"");
    cmd(inter, "button .a.ok -text Continue -command {set choice -2}");
    cmd(inter, "pack .a.l1 .a.l2 .a.l3 .a.ok");
    choice=0;
    cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
    cmd(inter, "focus -force .a.ok");
    cmd(inter, "blocklmm .a");
    while(choice==0)
     Tcl_DoOneEvent(0);
    cmd(inter, "sblocklmm .a");
    
    //cmd(inter, "tk_messageBox -type yesno -title \"Warning\" -message \"File\\n$filetoload\\nnot found.\"");
    } 
   
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
if( tosave==1 && (choice==2 || choice==15 || choice==1 ||choice==14 ||choice==6 ||choice==8 ||choice==3 || choice==33||choice==5||choice==39||choice==41))
  {

//  cmd(inter, "set answer [tk_dialog .dia \"Save?\" \"Save the file $filename?\" \"\" 0 yes no cancel]");
  cmd(inter, "set answer [tk_messageBox -type yesnocancel -title \"Save file?\" -message \"Recent changes to file '$filename' have not been saved. Do you want to save before continuing?\nNot doing so will not include recent changes to subsequent actions.\n\n- Yes: save the file and continue.\n- No: do not save and continue.\n- Cancel: do not save and return to editing.\"]");
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

cmd(inter, "toplevel .a");
cmd(inter, "wm title .a \"Choose\"");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "frame .a.f -relief groove -bd 4");
cmd(inter, "set temp 33");
cmd(inter, "label .a.f.l -text \"Choose Action\"  -fg red");

cmd(inter, "radiobutton .a.f.r1 -variable temp -value 33 -text \"Browse Models\" -justify left -relief groove -anchor w");

cmd(inter, "radiobutton .a.f.r2 -variable temp -value 15 -text \"Open a text file.\" -justify left -relief groove -anchor w");

cmd(inter, "radiobutton .a.f.r3 -variable temp -value 0 -text \"Create a new text file.\" -justify left -relief groove -anchor w");


cmd(inter, "pack .a.f.l ");
cmd(inter, "pack .a.f.r1 .a.f.r2 .a.f.r3 -anchor w -fill x ");

cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.help -text Help -command {LsdHelp LMM_help.html#introduction}");
cmd(inter, "button .a.b.ok -text Ok -command {set choice 1}");
cmd(inter, "pack .a.b.ok .a.b.help -side left");
cmd(inter, "pack  .a.f -fill x -expand yes");
cmd(inter, "pack .a.b");
cmd(inter, "bind .a <Return> {.a.b.ok invoke}");
cmd(inter, "bind .a <Escape> {set temp 0; .a.b.ok invoke}");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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

if(choice==2)
 {
/*Run the model in the selection*/


s=(char *)Tcl_GetVar(inter, "modelname",0);

if(s==NULL || !strcmp(s, ""))
 {//this control is obsolete, since  model must be selected in order to arrive here
  cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"No model selected. Choose an existing model or create a new one.\"");
  choice=0;
  goto loop;
 }

  cmd(inter, "cd $modeldir");
  

  make_makefile();

  cmd(inter, "set fapp [file nativename $modeldir/makefile]");
  s=(char *)Tcl_GetVar(inter, "fapp",0);
  
  //register equation file's name
  f=fopen(s, "r");

  if(f==NULL)
   {
     f=fopen("makefile", "w");
     fclose(f);
    cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"File 'makefile' not found.\\nUse the Model Compilation Options, in menu Model, to create it.\"");

    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }
  fscanf(f, "%s", str);
  while(strncmp(str, "FUN=", 4) && fscanf(f, "%s", str)!=EOF);
  fclose(f);
  if(strncmp(str, "FUN=", 4)!=0)
   {
    cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Makefile corrupted. Check Model and System Compilation options.\"");
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
    cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"File 'makefile' not found.\\nUse the Model Compilation Options, in menu Model, to create it.\"");

    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }
  fscanf(f, "%s", str);
  while(strncmp(str, "TARGET=", 7) && fscanf(f, "%s", str)!=EOF);
  fclose(f);
  if(strncmp(str, "TARGET=", 7)!=0)
   {
    cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Makefile corrupted. Check Model and System Compilation options.\"");
    choice=0;
    goto loop;
   }

  
  cmd(inter, "set init_time [clock seconds]"); 
  cmd(inter, "toplevel .t");
  cmd(inter, "wm title .t \"Wait\"");
  cmd(inter, "label .t.l -text \"Making model.\nThe system is checking the files modified since the last compilation and recompiling as necessary.\nOn success the new model program will be launched.\nOn failure a text window will show the compiling error messages.\"");
  cmd(inter, "pack .t.l");
  cmd(inter, "wm iconify .");
  cmd(inter, "focus -force .t.l");
  cmd(inter, "grab set .t.l");
    cmd(inter, "set w .t; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
  cmd(inter, "if { [file exists $RootLsd/gnu/bin/crtend.o] == 1} { file copy $RootLsd/gnu/bin/crtend.o .;file copy $RootLsd/gnu/bin/crtbegin.o .;file copy $RootLsd/gnu/bin/crt2.o .} {}");

  cmd(inter, "catch [set result [catch [exec a.bat]] ]");
  //cmd(inter, "tkwait variable result");
  cmd(inter, "file delete a.bat");
  cmd(inter, "if { [file exists crtend.o] == 1} { file delete crtend.o;file delete crtbegin.o ;file delete crt2.o } {}");

   }
  cmd(inter, "destroy .t");
  cmd(inter, "wm deiconify .");
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
   cmd(inter, "button .a.b.help -text Help -command {LsdHelp LMM_help.html#run}");
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
   cmd(inter, "button .a.b.help -text Help -command {LsdHelp LMM_help.html#run}");
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
     cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"No model selected.\"");
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
cmd(inter, "wm title . \"LMM - makefile\"");
cmd(inter, "tk_messageBox -title Warning -type ok -default ok -message \"Direct changes to the 'makefile' will not affect compilation issued through LMM. Choose System Compilation options an Model Compilation Options (menu Model).\"");  
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
  cmd(inter, "wm title . \"LMM - $filename\"");
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
  cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"No model selected.\"");
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
   cmd(inter, ".f.t.t conf -wrap word");
   cmd(inter, "set wrap 1");
  }
  cmd(inter, "set before [.f.t.t get 1.0 end]");
  cmd(inter, "set filename description.txt");
  cmd(inter, ".f.hea.file.dat conf -text $filename");
  cmd(inter, "wm title . \"LMM - $filename\"");
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
 
if(choice==6)
 {
 /*compile the model, invoking make*/
s=(char *)Tcl_GetVar(inter, "modelname",0);
if(s==NULL || !strcmp(s, ""))
 {
  cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"No model selected.\"");
  choice=0;
  goto loop;
 }

cmd(inter, "cd $modeldir");
make_makefile();
cmd(inter, "if { [file exists makefile] == 1 } {set choice 1} {set choice 0}");
if(choice==0)
 {
      cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"File 'makefile' not found.\"");
  choice=0;
  goto loop;
 
 }

  cmd(inter, "toplevel .t");
  cmd(inter, "wm title .t \"Wait\"");
  cmd(inter, "label .t.l -text \"Making model.\nThe system is checking the files modified since the last compilation and recompiling as necessary.\nAt the end, the output of the process will appear in the text window and in the file makemessage.txt\"");
  cmd(inter, "pack .t.l");
  cmd(inter, "wm iconify .");
  cmd(inter, "focus -force .t.l");
  cmd(inter, "grab set .t.l");
  cmd(inter, "set w .t; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
  //cmd(inter, "set w .t; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
  
  cmd(inter, "if {[file isdirectory ../gnu/bin ] == 1} {set a [../gnu/bin/make]} {set a \"make\"}");

  cmd(inter, "catch [exec $a 2> makemessage.txt]"); 
  cmd(inter, "destroy .t");
  cmd(inter, "wm deiconify .");
  cmd(inter, "if { [file size makemessage.txt]==0 } {set choice 0} {set choice 1}");
  if(choice==0)
   {
    cmd(inter, "set answer [tk_messageBox -type yesno -message \"Compilation succeeded.\\nRun the model now?\" -title \"Model Compiled\"]"); 
  s=(char *)Tcl_GetVar(inter, "answer",0);

  cmd(inter, "if {[string compare $answer \"yes\"] == 0} {set choice 2} {set choice 0}");
  cmd(inter, "cd $RootLsd");
  goto loop;
  }
//continue only if there are messages   
  create_compresult_window();
  cmd(inter, "cd $RootLsd");

  cmd(inter, "tk_messageBox -type ok -message \"Compilation issued messages, possibly errors.\\nSee the compilation results.\" -title \"Model Compiled\""); 


  choice =0;
  goto loop;
 }

if(choice==7)
 {
 /*Show compilation result*/

s=(char *)Tcl_GetVar(inter, "modelname",0);
if(s==NULL || !strcmp(s, ""))
 {
  cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"No model selected.\"");
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

   cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"No compilation results.\"");
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
       cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"No model selected.\"");
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
   
    cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"File 'makefile' not found.\\nAdd a makefile to model $modelname\"");

    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }
  fscanf(f, "%s", str);
  while(strncmp(str, "FUN=", 4) && fscanf(f, "%s", str)!=EOF);    
  fclose(f);
  if(strncmp(str, "FUN=", 4)!=0)
   {
    cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Makefile corrupted. Check Model and System Compilation options.\"");
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
  cmd(inter, "set inicolor \"1.0\"");
  cmd(inter, "set endcolor [.f.t.t index end]");
  cmd(inter, ".f.t.t tag add bc \"1.0\"");
  cmd(inter, ".f.t.t tag add fc \"1.0\"");
  cmd(inter, ".f.t.t mark set insert 1.0");
  color(&num);
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
cmd(inter, "label .search_line.l -text \"Type the line number\"");
cmd(inter, "entry .search_line.e -width 30 -textvariable line");
cmd(inter, "button .search_line.ok -text Ok -command {if {$line == \"\"} {.search_line.esc invoke} {.f.t.t see $line.0; .f.t.t tag remove sel 1.0 end; .f.t.t tag add sel $line.0 $line.500; .f.t.t mark set insert $line.0; .f.hea.line.line conf -text [.f.t.t index insert]; destroy .search_line; sblocklmm .search_line } }");
cmd(inter, "button .search_line.esc -text Close -command {sblocklmm .search_line}");
cmd(inter, "bind .search_line <KeyPress-Return> {.search_line.ok invoke}");
cmd(inter, "bind .search_line <KeyPress-Escape> {.search_line.esc invoke}");

cmd(inter, "pack .search_line.l .search_line.e .search_line.ok .search_line.esc -fill y");
cmd(inter, "set w .search_line; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
//cmd(inter, "set w .search_line; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
  

cmd(inter, "focus -force .search_line.e");
cmd(inter, "wm transient .search_line .");
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
cmd(inter, "wm protocol .find WM_DELETE_WINDOW { }");
cmd(inter, "wm title .find \"Search Text\"");

cmd(inter, "label .find.l -text \"Type the text to search\"");
cmd(inter, "entry .find.e -width 30 -textvariable textsearch");
cmd(inter, ".find.e selection range 0 end");
cmd(inter, "set docase 0");
cmd(inter, "checkbutton .find.c -text \"Case Sensitive\" -variable docase");
cmd(inter, "radiobutton .find.r1 -text \"Down\" -variable dirsearch -value \"-forwards\" -command {set endsearch end}");
cmd(inter, "radiobutton .find.r2 -text \"Up\" -variable dirsearch -value \"-backwards\" -command {set endsearch 1.0}" );

cmd(inter, "button .find.ok -text Ok -command {incr lfindcounter; set curcounter $lfindcounter; lappend lfind \"$textsearch\"; if {$docase==1} {set case \"-exact\"} {set case \"-nocase\"}; .f.t.t tag remove sel 1.0 end; set cur [.f.t.t index insert]; set cur [.f.t.t search $dirsearch -count length $case -- \"$textsearch\" $cur $endsearch]; if {[string length $cur] > 0} {.f.t.t tag add sel $cur \"$cur + $length char\"; if {[string compare $endsearch end]==0} {} {set length 0}; .f.t.t mark set insert \"$cur + $length char\" ; update; .f.t.t see $cur; sblocklmm .find} {.find.e selection range 0 end; bell}}");

cmd(inter, "bind .find.e <Up> {if { $curcounter >= 0} {incr curcounter -1; set textsearch \"[lindex $lfind $curcounter]\"; .find.e selection range 0 end;} {}}");
cmd(inter, "bind .find.e <Down> {if { $curcounter <= $lfindcounter} {incr curcounter; set textsearch \"[lindex $lfind $curcounter]\"; .find.e selection range 0 end;} {}}");

cmd(inter, "button .find.esc -text Esc -command {sblocklmm .find}");


cmd(inter, "bind .find <KeyPress-Return> {.find.ok invoke}");
cmd(inter, "bind .find <KeyPress-Escape> {.find.esc invoke}");

cmd(inter, "pack .find.l .find.e .find.r1 .find.r2 .find.c .find.ok .find.esc");

//cmd(inter, "set w .l; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
 cmd(inter, "set w .find; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
       cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"No model selected.\"");
  choice=0;
  goto loop;
 }

  
  cmd(inter, "cd $modeldir");
  if(choice==58)
   {
   //cmd(inter, "scan [.f.t.t index insert] %d.%d line col");
   cmd(inter, "scan $vmenuInsert %d.%d line col");
   cmd(inter, "set f [open break.txt w]; puts $f \"break $filename:$line\\n\"; close $f");
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
    cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"File 'makefile' for model $modelname not found.\"");
    choice=0;
    cmd(inter, "cd $RootLsd");
    goto loop;
   }
  fscanf(f, "%s", str);
  while(strncmp(str, "TARGET=", 7) && fscanf(f, "%s", str)!=EOF);
  if(strncmp(str, "TARGET=", 7)!=0)
   {
    cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Makefile corrupted. Check Model and System Compilation options.\"");
    choice=0;
    goto loop;
   }
  

  if(f!=NULL && f!=(FILE *)EOF)
   {strcpy(str1, str+7);
    
   //cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"5.0\" || $tcl_platform(osVersion) == \"5.1\" || $tcl_platform(osVersion) == \"5.2\"} {set choice 5} {set choice 4}} {set choice 3}}");
cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} {set choice 5} {set choice 3}}");
   if(choice==3)
    {
     
    cmd(inter, "set SETDIR \"../gnu/share/gdbtcl\"");
//    sprintf(msg, "exec start /min gdb_bat %s $SETDIR &", str1); //Win 9x
    sprintf(msg, "exec start gdb_batw9x %s &", str1); //Win 9x    
    }

   if(choice==4)
    {//WIndows NT case
    cmd(inter, "set SETDIR \"../gnu/share/gdbtcl\"");
    sprintf(msg, "exec cmd.exe /c start /min gdb_bat %s $SETDIR &", str1); //Win NT case
    }
   if(choice==5)
    {//Windows 2000
     
//     cmd(inter, "set answer [tk_messageBox -type yesno -title \"Text-based GDB\" -icon question -message \"Use text-based GDB?\"]");
//     cmd(inter, "if {$answer == \"yes\"} {set nowin \"-nw\"} {set nowin \"\"}");
    cmd(inter, "set nowin \"\"");
    sprintf(msg, "set f [open run_gdb.bat w]; puts $f \"SET GDBTK_LIBRARY=$RootLsd/gnu/share/gdbtcl\\nstart gdb $nowin %s $cmdbreak &\\n\"; close $f",str1);
    cmd(inter, msg);
    sprintf(msg, "exec run_gdb &");
     
    }

   if(choice==1)
     {
      sprintf(msg, "if {$cmdbreak==\"\"} {exec $Terminal -e gdb %s &} {exec $Terminal -e gdb $cmdbreak %s &}", str1, str1); //Unix case
     }
     
    cmd(inter, msg);
    cmd(inter, "cd $RootLsd"); 
   } 
  else
   {//executable not found
  cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"Executable not found. Compile the model before running it in the GDB debugger.\"");
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
cmd(inter, "wm title .a \"New model or group?\"");
cmd(inter, "label .a.tit -text \"Currrent group:\\n$modelgroup\"");
cmd(inter, "frame .a.f -relief groove");

cmd(inter, "set temp 1");
cmd(inter, "radiobutton .a.f.r1 -variable temp -value 1 -text \"Create a new model in the current group\" -justify left -relief groove -anchor w");
cmd(inter, "radiobutton .a.f.r2 -variable temp -value 2 -text \"Create a new model in a new group\" -justify left -relief groove -anchor w");
cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -text \" Ok \" -command {set choice 1}");
cmd(inter, "button .a.b.esc -text \" Cancel \" -command {set choice 2}");
cmd(inter, "pack .a.f.r1 .a.f.r2 -anchor w -fill x");
cmd(inter, "pack .a.b.ok .a.b.esc -side left");
cmd(inter, "pack .a.tit .a.f .a.b");
cmd(inter, "bind .a <Return> {.a.b.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");
cmd(inter, "bind .a <Up> {.a.f.r1 invoke}");
cmd(inter, "bind .a <Down> {.a.f.r2 invoke}");

cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "wm title .a \"New group\"");
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
cmd(inter, "button .a.b.ok -text Ok -command {set choice 1}");
cmd(inter, "button .a.b.esc -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.b.ok .a.b.esc -side left -fill x");

cmd(inter, "pack .a.tit .a.mname .a.ename .a.mdir .a.edir .a.ldes .a.tdes -anchor w");
cmd(inter, "pack .a.b");

cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "if {[file exists $groupdir/$mdir] == 1} {tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create directory: $groupdir/$mdir\\(possibly there is already such a directory).\\nCreation of new group aborted\"; set choice 3} {}");
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
cmd(inter, "wm title .a \"New model\"");
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
cmd(inter, "button .a.b.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");
cmd(inter, "button .a.b.esc -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.b.ok .a.b.esc -side left -fill x");

cmd(inter, "pack .a.tit .a.mname .a.ename .a.mver .a.ever .a.mdir .a.edir -anchor w");
cmd(inter, "pack .a.b");



cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
 {cmd(inter, "tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create the new model $mname (ver. $mver) because it already exists (directory: $errdir)\"");
  choice=0;
  goto loop_copy_new;
 } 
if(choice==4)
 {
 choice=0;
 cmd(inter, "set answer [tk_messageBox -type yesno -title \"Warning\" -icon warning -message \"Warning: a model $mname already exists (ver. $mver). If you want the new model to inherit the same equations, data etc. of that model you should cancel this operation (choose 'No'), and use the 'Copy' command in the models' Browser. Press 'Yes' to continue creating a new (empty) model '$mname'.\"]");


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
cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\n\"");
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
//cmd(inter, "if { $tcl_platform(platform) == \"windows\" } {file copy $RootLsd/gnu/bin/crt0.o $modeldir} {}");

cmd(inter, "cd $RootLsd");
cmd(inter, ".m.model entryconf 4 -state normal");
cmd(inter, ".m.model entryconf 5 -state normal");
cmd(inter, ".m.model entryconf 7 -state normal");	
cmd(inter, ".m.model entryconf 8 -state normal");
cmd(inter, ".m.model entryconf 9 -state normal");
cmd(inter, ".m.model entryconf 10 -state normal");
cmd(inter, ".m.model entryconf 11 -state normal");
cmd(inter, ".m.model entryconf 13 -state normal");
cmd(inter, ".m.model entryconf 14 -state normal");


cmd(inter, "tk_messageBox -type ok -title \"Model created\" -message \"New model $mname (ver. $mver) successfully created (directory: $dirname)\"");

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
    cmd(inter, "wm title . \"LMM - $filename\"");          

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
  if(!strcmp(str, ".cpp") || !strcmp(str, ".c") || !strcmp(str, ".C") || !strcmp(str, ".CPP") || !strcmp(str, ".Cpp") || !strcmp(str, ".c++") || !strcmp(str, ".C++"))
   {sourcefile=1;
    cmd(inter, "set inicolor \"1.0\"");
    cmd(inter, "set endcolor [.f.t.t index end]");
    color(&num);
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


if(choice==19)
{
choice=0;
if(sourcefile==1)
  color(&num);

goto loop;
}



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

if(choice==23)
{ //reset colors after a sign for coloring

if(sourcefile==0)
 {choice=0;
  goto loop;
 }
  cmd(inter, "set inicolor [.f.t.t search -backwards */ [.f.t.t index insert] 1.0]");
  cmd(inter, "if {$inicolor==\"\"} {set inicolor 1.0} {set inicolor [.f.t.t index \"$inicolor + 2 char\"]}");
  cmd(inter, "set endcolor [.f.t.t search /* [.f.t.t index insert] end]");
  cmd(inter, "if {$endcolor==\"\"} {set endcolor end} {}");
   
  cmd(inter, ".f.t.t tag remove str $inicolor $endcolor");
  cmd(inter, ".f.t.t tag remove comment1 $inicolor $endcolor");
  cmd(inter, ".f.t.t tag remove comment2 $inicolor $endcolor");

//cmd(inter, "set inicolor 1.0");
//cmd(inter, "set endcolor end");
choice=0;
color(&num);

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
cmd(inter, "grab set .l");
cmd(inter, "wm title .l \"Replace Text\"");
//cmd(inter, "set textsearch \"\"");
cmd(inter, "label .l.l -text \"Type the text to search\"");
cmd(inter, "entry .l.e -width 30 -textvariable textsearch");
cmd(inter, "label .l.r -text \"Type the text to replace\"");
cmd(inter, "entry .l.s -width 30 -textvariable textrepl");

cmd(inter, "radiobutton .l.r1 -text \"Down\" -variable dirsearch -value \"-forwards\" -command {set endsearch end}");
cmd(inter, "radiobutton .l.r2 -text \"Up\" -variable dirsearch -value \"-backwards\" -command {set endsearch 1.0}" );
cmd(inter, "checkbutton .l.c -text \"Case Sensitive\" -variable docase");

cmd(inter, "button .l.ok -text Search -command {if { [string length \"$textsearch\"]==0} {} {.f.t.t tag remove found 1.0 end; if {$docase==1} {set case \"-exact\"} {set case -nocase}; .f.t.t tag remove sel 1.0 end; set cur [.f.t.t index insert]; set cur [.f.t.t search $dirsearch -count length $case -- $textsearch $cur $endsearch]; if {[string length $cur] > 0} {.f.t.t tag add found $cur \"$cur + $length char\"; if {[string compare $endsearch end]==0} {.f.t.t mark set insert \"$cur + $length char\" } {.f.t.t mark set insert $cur}; update; .f.t.t see $cur; .l.repl conf -state normal; .l.all conf -state normal} {.l.all conf -state disabled; .l.repl conf -state disabled}}}");

cmd(inter, "button .l.repl -state disabled -text Replace -command {if {[string length $cur] > 0} {.f.t.t delete $cur \"$cur + $length char\"; .f.t.t insert $cur \"$textrepl\"; if {[string compare $endsearch end]==0} {} {.f.t.t mark set insert $cur}; .l.ok invoke} {}}");
cmd(inter, "button .l.all -state disabled -text \"Repl. All\" -command {set choice 4}");
cmd(inter, "button .l.esc -text Close -command {focus -force .f.t.t; set choice 5}");
cmd(inter, "bind .l <KeyPress-Return> {.l.ok invoke}");
cmd(inter, "bind .l <KeyPress-Escape> {.l.esc invoke}");


cmd(inter, "pack .l.l .l.e .l.r .l.s .l.r1 .l.r2 .l.c .l.ok .l.repl .l.all .l.esc");


//cmd(inter, "set w .l; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .l; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
cmd(inter, "focus -force .l.e");
cmd(inter, ".f.t.t tag conf found -background red -foreground white");
cmd(inter, "wm transient .l .");
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

cmd(inter, "grab release .l");
cmd(inter, "focus -force .f.t.t");
cmd(inter, "destroy .l");
cmd(inter, ".f.t.t tag remove found 1.0 end");
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

cmd(inter, "wm title .a \"Insert an equation\"");

cmd(inter, "label .a.l1 -text \"Type below the label of the Variable\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.b.ok}");

cmd(inter, "frame .a.f -relief groove -bd 2");

cmd(inter, "set isfun 0");
//cmd(inter, "radiobutton .a.f.r1 -variable isfun -value 0 -text \"Equation:\\nthe code will provide one value for every Variable of this type at each time step.\\nThe code is executed only once for each t, re-using the same value if requested many times.\\nEven if no other Variable requests this value, it the code is computed.\" -justify left -relief groove");
//cmd(inter, "radiobutton .a.f.r2 -variable isfun -value 1 -text \"Function:\\nthe code is executed again any time this Variable is requested by other Variables' code,\\nbut it is not computed by default. This code is not computed if no Variable\\nneeds this Variable's value.\" -justify left -relief groove");
//cmd(inter, "pack .a.f.r1 .a.f.r2 -anchor w -fill x ");


cmd(inter, "frame .a.b");	
cmd(inter, "button .a.b.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
cmd(inter, "button .a.b.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");
cmd(inter, "button .a.b.help -text Help -command {LsdHelp lsdfuncMacro.html#equation}");
cmd(inter, "pack .a.b.ok .a.b.help .a.b.esc -side left");
cmd(inter, "pack .a.l1 .a.label .a.f .a.b");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");




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


choice=0;
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
cmd(inter, "wm title .a \"Insert an equation\"");

cmd(inter, "label .a.l1 -text \"Type below the label of the Variable\"");
cmd(inter, "set v_label Label");
cmd(inter, "entry .a.label -width 30 -textvariable v_label");
cmd(inter, "bind .a.label <Return> {focus -force .a.b.ok}");

cmd(inter, "frame .a.f -relief groove -bd 2");

cmd(inter, "set isfun 0");
cmd(inter, "radiobutton .a.f.r1 -variable isfun -value 0 -text \"Equation:\\nthe code will provide one value for every Variable of this type at each time step.\\nThe code is executed only once for each t, re-using the same value if requested many times.\\nEven if no other Variable requests this value, it the code is computed.\" -justify left -relief groove");
cmd(inter, "radiobutton .a.f.r2 -variable isfun -value 1 -text \"Function:\\nthe code is executed again any time this Variable is requested by other Variables' code,\\nbut it is not computed by default. This code is not computed if no Variable\\nneeds this Variable's value.\" -justify left -relief groove");
cmd(inter, "pack .a.f.r1 .a.f.r2 -anchor w -fill x ");


cmd(inter, "frame .a.b");	
cmd(inter, "button .a.b.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
cmd(inter, "button .a.b.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");
cmd(inter, "button .a.b.help -text Help -command {LsdHelp lsdfunc.html#equation}");
cmd(inter, "pack .a.b.ok .a.b.help .a.b.esc -side left");
cmd(inter, "pack .a.l1 .a.label .a.f .a.b");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");


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
choice=0;
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

cmd(inter, "wm title .a \"Insert a 'V(...)' command\"");


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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#V}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");


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

choice=0;
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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#cal}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, "if {$v_num==\"\"} {.f.t.t insert insert \"$v_obj->cal(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->cal(\\\"$v_label\\\",$v_lag);\"; incr v_num}");
//cmd(inter, ".f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->cal(\\\"$v_label\\\",$v_lag);\"");
//cmd(inter, ".f.t.t mark set insert \"$a + 1 line\"");


cmd(inter, "if {$v_num==\"\"} { set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=num;

choice=0;
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
cmd(inter, "wm title .a \"Insert a 'CYCLE' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#CYCLE}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.label .a.l2 .a.obj .a.l3 .a.par .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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


choice=0;
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
cmd(inter, "wm title .a \"Insert a cycle for\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#basicc}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.label .a.l2 .a.obj .a.l3 .a.par .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, ".f.t.t insert insert \"for($v_obj=$v_par->search(\\\"$v_label\\\"); $v_obj!=NULL; $v_obj=go_brother($v_obj) )\\n\"");
cmd(inter, ".f.t.t insert insert \" {\\n  \\n }\\n\"");
cmd(inter, ".f.t.t mark set insert \"$a + 2 line + 2 char\"");


choice=0;
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
cmd(inter, "wm title .a \"Insert a Lsd script\"");

cmd(inter, "set res 26");
cmd(inter, "label .a.tit1 -text \"Insert Lsd Script\" -foreground #ff0000");
cmd(inter, "label .a.tit2 -text \"Choose one of the following options. The interface will request the necessary information\" -justify left");
cmd(inter, "frame .a.r -bd 2 -relief groove");
cmd(inter, "radiobutton .a.r.equ -text \"EQUATION/FUNCTION - insert a new Lsd equation\" -underline 0 -variable res -value 25");
cmd(inter, "radiobutton .a.r.cal -text \"V(...)- request the value of a Var\" -underline 0 -variable res -value 26");
cmd(inter, "radiobutton .a.r.sum -text \"SUM - Compute the sum of a Var over a set of Obj.\" -underline 1 -variable res -value 56");
cmd(inter, "radiobutton .a.r.scnd -text \"SEARCH_CND - conditional search a specific Object in the model\" -underline 0 -variable res -value 30");
cmd(inter, "radiobutton .a.r.sear -text \"SEARCH - search the first instance an Object type\" -underline 2 -variable res -value 55");
cmd(inter, "radiobutton .a.r.wri -text \"WRITE - overwrite a Variable or Parameter with a new value\" -underline 0 -variable res -value 29");
cmd(inter, "radiobutton .a.r.for -text \"CYCLE - insert a cycle over a group of Objects\" -underline 0 -variable res -value 27");
cmd(inter, "radiobutton .a.r.lqs -text \"SORT - sort a group of Objects\" -underline 3 -variable res -value 31");
cmd(inter, "radiobutton .a.r.addo -text \"ADDOBJ - Add a new Object\" -underline 3 -variable res -value 52");
cmd(inter, "radiobutton .a.r.delo -text \"DELETE - Delete an Object\" -underline 0 -variable res -value 53");
cmd(inter, "radiobutton .a.r.rndo -text \"RNDDRAW - Draw an Object\" -underline 1 -variable res -value 54");
cmd(inter, "radiobutton .a.r.incr -text \"INCR - increment the value of a Parameter\" -underline 0 -variable res -value 40");
cmd(inter, "radiobutton .a.r.mult -text \"MULT - multiply the value of a Parameter\" -underline 0 -variable res -value 45");
cmd(inter, "radiobutton .a.r.math -text \"Insert A Math/Prob. function\" -underline 14 -variable res -value 51");


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
cmd(inter, "bind .a <KeyPress-p> {.a.r.math invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-n> {.a.r.rndo invoke; set choice 1}");

cmd(inter, "frame .a.f");
cmd(inter, "button .a.f.ok -text Insert -command {set choice 1}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp LMM_help.html#LsdScript}");

cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.r.equ .a.r.cal .a.r.sum .a.r.sear .a.r.scnd .a.r.lqs .a.r.addo .a.r.delo .a.r.rndo .a.r.wri .a.r.incr .a.r.mult .a.r.for .a.r.math -anchor w");
cmd(inter, "pack .a.tit1 .a.tit2 .a.r .a.f");
cmd(inter, "bind .a <Return> {.a.f.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
cmd(inter, "focus -force .a.f.ok");


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
cmd(inter, "wm title .a \"Insert a Lsd script\"");

cmd(inter, "set res 26");
cmd(inter, "label .a.tit1 -text \"Insert Lsd Script\" -foreground #ff0000");
cmd(inter, "label .a.tit2 -text \"Choose one of the following options. The interface will request the necessary information\" -justify left");
cmd(inter, "frame .a.r -bd 2 -relief groove");
cmd(inter, "radiobutton .a.r.equ -text \"EQUATION/FUNCTION - insert a new Lsd equation\" -underline 0 -variable res -value 25");
cmd(inter, "radiobutton .a.r.cal -text \"cal - request the value of a Var\" -underline 0 -variable res -value 26");
cmd(inter, "radiobutton .a.r.sum -text \"sum - Compute the sum of a Var over a set of Obj.\" -underline 1 -variable res -value 56");
cmd(inter, "radiobutton .a.r.scnd -text \"search_var_cond - conditional search a specific Object in the model\" -underline 0 -variable res -value 30");
cmd(inter, "radiobutton .a.r.sear -text \"search - search the first instance an Object type\" -underline 3 -variable res -value 55");
cmd(inter, "radiobutton .a.r.wri -text \"write - overwrite a Variable or Parameter with a new value\" -underline 0 -variable res -value 29");
cmd(inter, "radiobutton .a.r.for -text \"for - insert a cycle over a group of Objects\" -underline 0 -variable res -value 27");
cmd(inter, "radiobutton .a.r.lqs -text \"lsdqsort - sort a group of Objects\" -underline 7 -variable res -value 31");
cmd(inter, "radiobutton .a.r.addo -text \"add_an_object - Add a new Object\" -underline 0 -variable res -value 52");
cmd(inter, "radiobutton .a.r.delo -text \"delete_obj - Delete an Object\" -underline 0 -variable res -value 53");
cmd(inter, "radiobutton .a.r.rndo -text \"draw_rnd - Draw an Object\" -underline 6 -variable res -value 54");
cmd(inter, "radiobutton .a.r.incr -text \"increment - increment the value of a Parameter\" -underline 0 -variable res -value 40");
cmd(inter, "radiobutton .a.r.mult -text \"multiply - multiply the value of a Parameter\" -underline 0 -variable res -value 45");
cmd(inter, "radiobutton .a.r.math -text \"Insert A Math/Prob. function\" -underline 14 -variable res -value 51");


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
cmd(inter, "bind .a <KeyPress-p> {.a.r.math invoke; set choice 1}");
cmd(inter, "bind .a <KeyPress-n> {.a.r.rndo invoke; set choice 1}");

cmd(inter, "frame .a.f");
cmd(inter, "button .a.f.ok -text Insert -command {set choice 1}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp LMM_help.html#LsdScript}");

cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.r.equ .a.r.cal .a.r.sum .a.r.sear .a.r.scnd .a.r.lqs .a.r.addo .a.r.delo .a.r.rndo .a.r.wri .a.r.incr .a.r.mult .a.r.for .a.r.math -anchor w");
cmd(inter, "pack .a.tit1 .a.tit2 .a.r .a.f");
cmd(inter, "bind .a <Return> {.a.f.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
cmd(inter, "focus -force .a.f.ok");


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

cmd(inter, "wm title .a \"Insert a 'INCR' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#INCR}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, "if {$v_num==\"\" } {.f.t.t insert insert \"$v_obj->increment(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->increment(\\\"$v_label\\\",$v_val);\";incr v_num}");
cmd(inter, "if {$v_num!=\"\" } {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}");
cmd(inter, "if {$v_obj!=\"p\" } {.f.t.t insert insert \"INCRS($v_obj,\\\"$v_label\\\",$v_val)\"} {.f.t.t insert insert \"INCR(\\\"$v_label\\\",$v_val)\"}");
cmd(inter, ".f.t.t insert insert \";\\n\"");

cmd(inter, "if {$v_num==\"\" } {set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "unset -nocomplain v_num");
choice=0;
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
cmd(inter, "wm title .a \"Insert a 'increment'\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#increment}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, "if {$v_num==\"\" } {.f.t.t insert insert \"$v_obj->increment(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->increment(\\\"$v_label\\\",$v_val);\";incr v_num}");



cmd(inter, "if {$v_num==\"\" } {set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "unset v_num");
choice=0;
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

cmd(inter, "wm title .a \"Insert a 'MULT' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#MULT}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, "if {$v_num==\"\" } {.f.t.t insert insert \"$v_obj->multiply(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->multiply(\\\"$v_label\\\",$v_val);\";incr v_num}");

cmd(inter, "if {$v_num!=\"\" } {.f.t.t insert insert \"v\\\[$v_num\\]=\"} {}");
cmd(inter, "if {$v_obj!=\"p\" } {.f.t.t insert insert \"MULTS($v_obj,\\\"$v_label\\\",$v_val)\"} {.f.t.t insert insert \"MULT(\\\"$v_label\\\",$v_val)\"}");
cmd(inter, ".f.t.t insert insert \";\\n\"");


cmd(inter, "if {$v_num==\"\" } {set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "unset v_num");
choice=0;
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
cmd(inter, "wm title .a \"Insert a 'multiply'\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#multiply}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.val .a.l4 .a.obj .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, "if {$v_num==\"\" } {.f.t.t insert insert \"$v_obj->multiply(\\\"$v_label\\\",$v_val);\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->multiply(\\\"$v_label\\\",$v_val);\";incr v_num}");



cmd(inter, "if {$v_num==\"\" } {set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

cmd(inter, "unset v_num");
choice=0;
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

cmd(inter, "wm title .a \"Insert a 'WRITE' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#WRITE}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, ".f.t.t insert insert \"$v_obj->write(\\\"$v_label\\\",$v_num, $v_lag);\"");
//cmd(inter, ".f.t.t mark set insert \"$a + 1 line\"");
cmd(inter, "if {$v_obj ==\"p\" && $v_lag == 0 } { .f.t.t insert insert \"WRITE(\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj ==\"p\" && $v_lag != 0 } { .f.t.t insert insert \"WRITEL(\\\"$v_label\\\",$v_num, $v_lag);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_lag == 0 } { .f.t.t insert insert \"WRITES($v_obj,\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_lag != 0 } { .f.t.t insert insert \"WRITELS($v_obj,\\\"$v_label\\\",$v_num, $v_lag);\"} {}");


choice=0;
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
cmd(inter, "wm title .a \"Insert a 'write' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#write}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, ".f.t.t insert insert \"$v_obj->write(\\\"$v_label\\\",$v_num, $v_lag);\"");
//cmd(inter, ".f.t.t mark set insert \"$a + 1 line\"");

choice=0;
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
cmd(inter, "wm title .a \"Insert a 'search_var_cond' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#search_var_cond}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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
cmd(inter, "set a [.f.t.t index insert]");
cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->search_var_cond(\\\"$v_label\\\",$v_num, $v_lag);\"");

choice=0;
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

cmd(inter, "wm title .a \"Insert a 'SEARCH_CND' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#SEARCH_CND}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->search_var_cond(\\\"$v_label\\\",$v_num, $v_lag);\"");

cmd(inter, "if {$v_obj ==\"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0=SEARCH_CND(\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj ==\"p\" && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0=SEARCH_CNDL(\\\"$v_label\\\",$v_num, $v_lag);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_lag == 0 } { .f.t.t insert insert \"$v_obj0=SEARCH_CNDS($v_obj,\\\"$v_label\\\",$v_num);\"} {}");
cmd(inter, "if {$v_obj !=\"p\" && $v_lag != 0 } { .f.t.t insert insert \"$v_obj0=SEARCH_CNDLS($v_obj,\\\"$v_label\\\",$v_num, $v_lag);\"} {}");
cmd(inter, "destroy .a");


choice=0;
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

cmd(inter, "wm title .a \"Insert a 'SORT' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#SORT}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.obj1 .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.r_up .a.r_down .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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

choice=0;
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
cmd(inter, "wm title .a \"Insert a 'lsdqsort' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#lsdqsort}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.obj1 .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.r_up .a.r_down .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
if(choice==1)
  cmd(inter, ".f.t.t insert insert \"$v_obj1->lsdqsort(\\\"$v_obj0\\\",\\\"$v_label\\\", \\\"UP\\\");\"");
else
  cmd(inter, ".f.t.t insert insert \"$v_obj1->lsdqsort(\\\"$v_obj0\\\",\\\"$v_label\\\", \\\"DOWN\\\");\"");


choice=0;
goto loop;
}


if(choice==51)
{
/*
Insert a math function
*/
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm title .a \"Insert a math operation\"");

cmd(inter, "set value1 \"0\"; set value2 \"1\"; set res 1; set str \"UNIFORM($value1,$value2)\"");
cmd(inter, "label .a.l1 -text \"Minimum\"");
cmd(inter, "entry .a.e1 -textvariable value1");
//cmd(inter, "a.e1 selection range 0 end");
cmd(inter, "label .a.l2 -text \"Maximum\"");
cmd(inter, "entry .a.e2 -textvariable value2");
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

cmd(inter, "button .a.ok -text Ok -command {set choice 1}");
cmd(inter, "button .a.help -text Help -command {LsdHelp lsdfuncMacro.html#rnd}");
cmd(inter, "button .a.can -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.r1 .a.r2 .a.r3 .a.r4 .a.r5 .a.r6 .a.r7 .a.r8 .a.r9 .a.r10 .a.r11 .a.r12 .a.r13 -anchor w");
cmd(inter, "pack .a.ok .a.help .a.can");
choice=0;
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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

cmd(inter, ".f.t.t insert insert \"$str\"");

choice=0;
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

cmd(inter, "wm title .a \"Insert a 'ADDOBJ' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#ADDOBJ}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l2 .a.label .a.l3 .a.numobj .a.l1 .a.v_num .a.l4 .a.obj .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
choice=0;
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

cmd(inter, "wm title .a \"Insert a 'add_an_object' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#add_an_object}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l4 .a.obj .a.f");



cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, "if {$v_num==\"\" } { .f.t.t insert insert \"$v_obj0=$v_obj->add_an_object(\\\"$v_label\\\");\\n\"} {.f.t.t insert insert \"$v_obj0=$v_obj->add_an_object(\\\"$v_label\\\",$v_num);\\n\"}");

choice=0;
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

cmd(inter, "wm title .a \"Insert a 'DELETE' command\"");

cmd(inter, "label .a.l0 -text \"Type below the pointer of the Object to delete\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#DELETE}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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
cmd(inter, "set a [.f.t.t index insert]");


cmd(inter, ".f.t.t insert insert \"DELETE($v_obj0);\\n\"");


choice=0;
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

cmd(inter, "wm title .a \"Insert a 'delete_obj' command\"");

cmd(inter, "label .a.l0 -text \"Type below the pointer of the Object to delete\"");
cmd(inter, "set v_obj0 cur");
cmd(inter, "entry .a.obj0 -width 6 -textvariable v_obj0");
cmd(inter, "bind .a.obj0 <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#delete_obj}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");


cmd(inter, ".f.t.t insert insert \"$v_obj0->delete_obj();\\n\"");


choice=0;
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

cmd(inter, "wm title .a \"Insert a 'RNDDRAW' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#RNDDRAW}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l31 .a.tot .a.l4 .a.obj .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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


choice=0;
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

cmd(inter, "wm title .a \"Insert a 'draw_rnd' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#draw_rnd}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l31 .a.tot .a.l4 .a.obj .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");
//cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->search_var_cond(\\\"$v_label\\\",$v_num, $v_lag);\"");
cmd(inter, "if {$v_tot == \"\"} {set choice 1} {set choice 2}");

if(choice==1)
  cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->draw_rnd(\\\"$v_num\\\",\\\"$v_label\\\", $v_lag);\\n\"");
else
  cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj->draw_rnd(\\\"$v_num\\\",\\\"$v_label\\\", $v_lag, $v_tot);\\n\"");

choice=0;
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

cmd(inter, "wm title .a \"Insert a 'SEARCH' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#SEARCH}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.lab .a.l2 .a.obj1 .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, "if { $v_obj1 == \"p\" } {.f.t.t insert insert \"$v_obj0=SEARCH(\\\"$v_lab\\\");\\n\"} {.f.t.t insert insert \"$v_obj0=SEARCHS($v_obj1,\\\"$v_lab\\\");\\n\"}");


choice=0;
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

cmd(inter, "wm title .a \"Insert a 'search' command\"");

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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#search}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");

cmd(inter, "pack .a.l0 .a.obj0 .a.l1 .a.lab .a.l2 .a.obj1 .a.f");


//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, ".f.t.t insert insert \"$v_obj0=$v_obj1->search(\\\"$v_lab\\\");\\n\"");


choice=0;
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

cmd(inter, "wm title .a \"Insert a 'SUM'\"");
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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#SUM}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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

choice=0;
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

cmd(inter, "wm title .a \"Insert a 'sum'\"");
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
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfunc.html#sum}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.label .a.l3 .a.lag .a.l4 .a.obj .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "set a [.f.t.t index insert]");

cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \"v\\\[$v_num\\]=$v_obj->sum(\\\"$v_label\\\",$v_lag);\"} {.f.t.t insert insert \"$v_obj->sum(\\\"$v_label\\\",$v_lag);\\n\"}");

cmd(inter, "if {$v_num!=\"\"} {.f.t.t insert insert \";\\n\"} {}");

cmd(inter, "if {$v_num==\"\"} { set num -1} {set num $v_num}");
if(num!=-1)
 v_counter=++num;

choice=0;
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
   cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"You cannot remove this.\"");
   goto loop;
   }
  cmd(inter, "if { [lindex $group $result] == 1} {set message \"Group\\n[lindex $lmn $result] (dir. [lindex $ldn $result])\\n is going to be deleted. Confirm?\"; set answer [tk_messageBox -icon question -type yesno -title \"Remove group?\" -message $message]} {set message \"Model\\n[lindex $lmn $result] ver. [lindex $lver $result] (dir. [lindex $ldn $result])\\n is going to be deleted. Confirm?\"; set answer [tk_messageBox -icon question -type yesno -title \"Remove model?\" -message $message]}");
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
cmd(inter, ".m.model entryconf 4 -state normal");
cmd(inter, ".m.model entryconf 5 -state normal");
cmd(inter, ".m.model entryconf 7 -state normal");	
cmd(inter, ".m.model entryconf 8 -state normal");
cmd(inter, ".m.model entryconf 9 -state normal");
cmd(inter, ".m.model entryconf 10 -state normal");
cmd(inter, ".m.model entryconf 11 -state normal");
cmd(inter, ".m.model entryconf 13 -state normal");
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

cmd(inter, "wm title .a \"Copy model\"");
cmd(inter, "label .a.tit -text \"Create a new version of model $modelname (ver. $version)\"");

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
cmd(inter, "button .a.b.ok -text Ok -command {set choice 1}");
cmd(inter, "button .a.b.help -text Help -command {LsdHelp LMM_help.html#copy}");
cmd(inter, "button .a.b.esc -text Cancel -command {set choice 2}");
cmd(inter, "pack .a.b.ok .a.b.help .a.b.esc -side left -fill x");

cmd(inter, "pack .a.mname .a.ename .a.mver .a.ever .a.mdir .a.edir -anchor w");
cmd(inter, "pack .a.b");

cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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
cmd(inter, "if {[file exists $mdir] == 1} {tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create directory: $mdir.\\nChoose a different name\"; set choice 3} {}");
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
 {cmd(inter, "tk_messageBox -type ok -title \"Error\" -icon error -message \"Cannot create the new model $mname (ver. $mver) because it already exists (directory: $errdir)\"");
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
cmd(inter, "tk_messageBox -type ok -title \"Model copied\" -message \"New model $mname (ver. $mver) successfully created (directory: $dirname)\"");

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
  cmd(inter, "tk_messageBox -type ok -title \"Warning\" -message \"Cannot find file for model info.\\nPlease, enter the date of creation.\"");



  
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");

cmd(inter, "wm title .a \"Model Info\"");
cmd(inter, "frame .a.c");

cmd(inter, "label .a.c.n -text \"Model Name\"");
cmd(inter, "entry .a.c.en -width 40 -textvariable modelname");

cmd(inter, "label .a.c.d -text \"Model Directory\"");
cmd(inter, "set complete_dir [file nativename [file join [pwd] $modeldir]]");
cmd(inter, "entry .a.c.ed -width 40 -state disabled -textvariable complete_dir");

cmd(inter, "label .a.c.v -text \"Version\"");
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
  cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"File 'makefile' not found.\\nAdd a makefile to model $modelname ([pwd])\"");
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
  cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Makefile corrupted. Check Model and System Compilation options.\"");
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


cmd(inter, "pack .a.c.n .a.c.en .a.c.v .a.c.ev .a.c.date .a.c.edate .a.c.last .a.c.elast .a.c.d .a.c.ed -anchor w");

cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -text \"Ok\" -command {set choice 1}");
cmd(inter, "button .a.b.esc -text \"Cancel\" -command {set choice 2}");
cmd(inter, "pack .a.b.ok .a.b.esc -side left");

cmd(inter, "pack .a.c .a.b");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
cmd(inter, "bind .a <Escape> {set choice 2}");

cmd(inter, "focus -force .a.c.en");
cmd(inter, "bind .a.c.en <Return> {focus -force .a.c.ev}");
cmd(inter, "bind .a.c.ev <Return> {focus -force .a.b.ok}");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
choice=0;
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
 cmd(inter, "wm title . \"LMM - nomame.txt\"");
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
   cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\n\"");
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
cmd(inter, "wm title .l \"System Compilation's Options\"");

cmd(inter, "frame .l.t");

cmd(inter, "scrollbar .l.t.yscroll -command \".l.t.text yview\"");
cmd(inter, "text .l.t.text -wrap word -font {Times 10 normal} -width 60 -height 16 -relief sunken -yscrollcommand \".l.t.yscroll set\"");


cmd(inter, "set win_default \"TCL_VERSION=8\\nTK_VERSION=84\\nLSDROOT=[pwd]\\nDUMMY=mwindows\\nPATH_TCL_LIB=\\$(LSDROOT)/gnu/lib\\nPATH_TK_LIB=\\$(LSDROOT)/gnu/lib\\nPATH_TK_HEADER=\\$(LSDROOT)/gnu/include\\nPATH_TCL_HEADER=\\$(LSDROOT)/gnu/include\\nPATH_LIB=\\$(LSDROOT)/gnu/lib\\nINCLUDE_LIB=-I\\$(LSDROOT)/gnu/include\\nCC=gcc\\nSRC=src\\nEXTRA_PAR=-lz\\nSSWITCH_CC=-O2\\n\"");
cmd(inter, "set lin_default \"TCL_VERSION=8.4\\nTK_VERSION=8.4\\nLSDROOT=[pwd]\\nDUMMY=\\nPATH_TCL_LIB=.\\nPATH_TK_LIB=.\\nPATH_TK_HEADER=\\nPATH_TCL_HEADER=\\nPATH_LIB=.\\nINCLUDE_LIB=\\nCC=g++\\nSRC=src\\nEXTRA_PAR=-lz\\nSSWITCH_CC=-O2\\n\"");

cmd(inter, "frame .l.t.d -relief groove -bd 2");
cmd(inter, "button .l.t.d.win -text \" Default Windows \" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_windows.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}"); 


cmd(inter, "button .l.t.d.lin -text \" Default Linux \" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_linux.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}");
cmd(inter, "button .l.t.d.mac -text \" Default Mac OS X \" -command {.l.t.text delete 1.0 end; set file [open $RootLsd/$LsdSrc/sysopt_mac.txt r]; set a [read -nonewline $file]; close $file; .l.t.text insert end \"LSDROOT=[pwd]\\n\"; .l.t.text insert end \"$a\"}"); 
cmd(inter, "pack .l.t.d.win .l.t.d.lin .l.t.d.mac -expand yes -side left");

cmd(inter, "frame .l.t.b");
cmd(inter, "button .l.t.b.ok -text Ok -command {set choice 1}");
cmd(inter, "button .l.t.b.esc -text Cancel -command {set choice 2}");
cmd(inter, "button .l.t.b.help -text Help -command {LsdHelp LMM_help.html#compilation_options}");
cmd(inter, "pack .l.t.b.ok .l.t.b.help .l.t.b.esc -expand yes -side left");
cmd(inter, "pack .l.t.yscroll -side right -fill y");
cmd(inter, "pack .l.t.text .l.t.d .l.t.b -expand yes -fill both");
cmd(inter, "pack .l.t");

cmd(inter, ".l.t.text insert end $a");

//cmd(inter, "bind .l <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .l <KeyPress-Escape> {set choice 2}");
cmd(inter, "set w .l; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
   cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\n\"");
   cmd(inter, "set f [open model_options.txt w]");
   cmd(inter, "puts -nonewline $f $a");
   cmd(inter, "close $f");

  }
cmd(inter, "if { [winfo exists .l]==1} {.l.m.file invoke 1} { }"); 
cmd(inter, "toplevel .l");
cmd(inter, "wm protocol .l WM_DELETE_WINDOW { }");
cmd(inter, "wm title .l \"Model Compilation's Options\"");


cmd(inter, "frame .l.t");

cmd(inter, "scrollbar .l.t.yscroll -command \".l.t.text yview\"");
cmd(inter, "text .l.t.text -wrap word -font {Times 10 normal} -width 60 -height 10 -relief sunken -yscrollcommand \".l.t.yscroll set\"");

cmd(inter, "set default \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\n\"");


cmd(inter, "frame .l.t.d -relief groove -bd 2");
cmd(inter, "button .l.t.d.def -text \" Default Values \" -command {.l.t.text delete 1.0 end; .l.t.text insert end \"$default\"}");

//cmd(inter, "button .l.t.d.cle -text \" Clean pre-compiled files \" -command { set mapp [pwd]; cd $RootLsd; cd src; catch [set app [glob *.o]]; set cazzo [llength $app];  catch [foreach i [glob $RootLsd/$LsdSrc/*.o] {catch [file delete -force $i]}]; cd $mapp}");
cmd(inter, "button .l.t.d.cle -text \" Clean pre-compiled files \" -command { catch [foreach i [glob $RootLsd/$LsdSrc/*.o] {wm title .l $i; catch [file delete -force $i]}]}");

cmd(inter, "pack .l.t.d.def -expand yes -side left");

cmd(inter, "frame .l.t.b");
cmd(inter, "button .l.t.b.ok -text Ok -command {set choice 1}");
cmd(inter, "button .l.t.b.esc -text Cancel -command {set choice 2}");
cmd(inter, "button .l.t.b.help -text Help -command {LsdHelp LMM_help.html#compilation_options}");

cmd(inter, "pack .l.t.b.ok .l.t.b.help .l.t.b.esc -expand yes -side left");
cmd(inter, "pack .l.t.yscroll -side right -fill y");
cmd(inter, "pack .l.t.text .l.t.d .l.t.b -expand yes -fill both");
cmd(inter, "pack .l.t");
//cmd(inter, "bind .l <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .l <KeyPress-Escape> {set choice 2}");
cmd(inter, ".l.t.text insert end $a");
cmd(inter, "set w .l; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
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
cmd(inter, "label .a.l1 -text \"Enter the font name you wish to use\"");

cmd(inter, "entry .a.v_num -width 30 -textvariable fonttype");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp LMM_help.html#changefont}");
cmd(inter, "pack .a.f.ok .a.f.help .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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
cmd(inter, "set a [list $fonttype $dim_character]; .f.t.t conf -font \"$a\"");



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

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm title .a \"LMM Options\"");
cmd(inter, "label .a.l1 -text \"Terminal to use for the GDB debugger\"");
cmd(inter, "entry .a.v_num -width 30 -textvariable temp_var1");
cmd(inter, "bind .a.v_num <Return> {focus -force .a.v_num2; .a.v_num2 selection range 0 end}");

cmd(inter, "label .a.l2 -text \"HTML Browser to use for help pages.\"");
cmd(inter, "entry .a.v_num2 -width 30 -textvariable temp_var2");
cmd(inter, "bind .a.v_num2 <Return> {focus -force .a.v_num3; .a.v_num3 selection range 0 end}");

cmd(inter, "label .a.l3 -text \"Font family\"");
cmd(inter, "entry .a.v_num3 -width 30 -textvariable temp_var3");
cmd(inter, "bind .a.v_num3 <Return> {focus -force .a.v_num4; .a.v_num4 selection range 0 end}");

cmd(inter, "label .a.l4 -text \"Wish\"");
cmd(inter, "entry .a.v_num4 -width 30 -textvariable temp_var4");
cmd(inter, "bind .a.v_num4 <Return> {focus -force .a.v_num5; .a.v_num4 selection range 0 end}");

cmd(inter, "label .a.l5 -text \"Wish\"");
cmd(inter, "entry .a.v_num5 -width 30 -textvariable temp_var5");
cmd(inter, "bind .a.v_num5 <Return> {focus -force .a.f.ok}");

cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Esc -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp LMM_help.html#SystemOpt}");
cmd(inter, "button .a.f.def -text Default -command {set temp_var1 $DefaultTerminal; set temp_var2 $DefaultHtmlBrowser; set temp_var3 $DefaultFont; set temp_var5 src}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");

cmd(inter, "pack .a.f.ok .a.f.help .a.f.def .a.f.esc -side left");
cmd(inter, "pack .a.l1 .a.v_num .a.l2 .a.v_num2  .a.l3 .a.v_num3 .a.l4 .a.v_num4 .a.l5 .a.v_num5 .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");

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
 
 cmd(inter, "set a [list $fonttype $dim_character]");
 cmd(inter, ".f.t.t conf -font \"$a\"");
 cmd(inter, "set f [open $RootLsd/lmm_options.txt w]");
 cmd(inter, "puts -nonewline $f  \"$Terminal\n\"");
 cmd(inter, "puts $f $HtmlBrowser");
 cmd(inter,  "puts $f $fonttype");
 cmd(inter,  "puts $f $wish");
 cmd(inter, "puts $f $LsdSrc");
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
cmd(inter, "set lfile {$RootLsd/$LsdSrc/lsdmain.cpp $RootLsd/$LsdSrc/main_gnuwin.cpp $RootLsd/$LsdSrc/file.cpp $RootLsd/$LsdSrc/util.cpp $RootLsd/$LsdSrc/variab.cpp $RootLsd/$LsdSrc/object.cpp $RootLsd/$LsdSrc/lsdmain.cpp $RootLsd/$LsdSrc/report.cpp $RootLsd/$LsdSrc/fun_head.h $RootLsd/$LsdSrc/decl.h } ");

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

cmd(inter, "file copy -force $RootLsd/$LsdSrc/fun_head.h src");
cmd(inter, "file copy -force $RootLsd/$LsdSrc/decl.h src");

cmd(inter, "file copy -force $RootLsd/$LsdSrc/system_options.txt src");

cmd(inter, "set f [open src/choose.h w]");
cmd(inter, "puts -nonewline $f \"#define NO_WINDOW\\n\"");
cmd(inter, "close $f");

cmd(inter, "cd $RootLsd"); 
cmd(inter, "tk_messageBox -type ok -icon info -title \"Info\" -message \"LMM has created a non-graphical version of the model, to be transported on any system endowed with a GCC compiler and standard libraries.\\n\\nTo move the model in another system copy the content of the model's directory:\\n$modeldir\\nincluding also its new subdirectory 'src'.\\n\\nTo create a 'no window' version of the model program follow these steps, to be executed within the directory of the model:\\n- compile with the command 'make -f makefileNW'\\n- run the model with the command 'lsd_gnu -f mymodelconf.lsd'\\n- the simulation will run automatically saving the results (for the variables indicated in the conf. file) in LSD result files named after the seed generator used.\"");
choice=0;
goto loop;

}
if(choice!=1)
 {choice=0;
 goto loop;
 }

Tcl_UnlinkVar(inter, "choice");
Tcl_UnlinkVar(inter, "num");
Tcl_UnlinkVar(inter, "tosave");

Tcl_Exit(0);

exit(0);

}



void color(int *num)
{
int i=0;
char *s;
Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);
cmd(inter, "set previ $inicolor");
cmd(inter, "set beg $inicolor");

while(i==0)
 {
Tcl_DoOneEvent(0);
cmd(inter, "set beg $previ");
cmd(inter, "set a [.f.t.t search \"//\" $previ $endcolor]");
cmd(inter, "set b [.f.t.t search \"/*\" $previ $endcolor]");
cmd(inter, "set c [.f.t.t search \"\\\"\" $previ $endcolor]");
cmd(inter, "if {$a==\"\"} {set a [.f.t.t index $endcolor]} {}");
cmd(inter, "if {$b==\"\"} {set b [.f.t.t index $endcolor]} {}");
cmd(inter, "if {$c==\"\"} {set c [.f.t.t index $endcolor]} {}");
cmd(inter, "if {$a==$b && $b==$c} {set i 1} {}");
if(i==1)
 {i=0;
 return;
 }
cmd(inter, "while {$c != \"\" && [string compare [.f.t.t get \"$c-1char\"] \"\\\\\"]==0} {set c [.f.t.t search \"\\\"\" \"$c+1char\" $endcolor]}");
cmd(inter, "if {$c == \"\" } {set c $endcolor} {}");
cmd(inter, "if {[.f.t.t compare $a < $b]} {if {[.f.t.t compare $a < $c]} {set previ $a; set i 2} {set previ $c; set i 4}} {if {[.f.t.t compare $b < $c]} {set previ $b; set i 3} {set previ $c; set i 4}}");
cmd(inter, "if {[string compare $previ \"\"]==0} {set i 1} {}");
cmd(inter, "scan $previ %d.%d num trash");
/*
s=(char *)Tcl_GetVar(inter, "beg",0);
printf("%s",s);

s=(char *)Tcl_GetVar(inter, "previ",0);
printf("%s",s);
*/
 if(i==2)
 {
  //comment of type //
  cmd(inter, "scan $previ %d.%d line trash");
  //cmd(inter, ".f.t.t tag remove str $previ \"$line.end+1char\"");
  //cmd(inter, ".f.t.t tag remove comment1 $previ \"$line.end+1char\"");
  cmd(inter, ".f.t.t tag add comment2 $previ \"$line.end+1char\"");
  cmd(inter, "set previ [.f.t.t index \"$line.end+1char\"]");
  cmd(inter, "if {[string compare $previ \"\"]==0} {set previ $endcolor; set i 1} {set i 0;}");
 }
if(i==3)
  {
  /*comment of the type /*  ********** */

  cmd(inter, "set fin [.f.t.t search \"*/\" $previ end]");
  cmd(inter, "if {[string compare $fin \"\"]==0} {set fin end; set i 1} {set i 0}");
  //cmd(inter, ".f.t.t tag remove str $previ \"$fin + 2 char\"");
  //cmd(inter, ".f.t.t tag remove comment2 $previ \"$fin + 2 char\"");
  cmd(inter, ".f.t.t tag add comment1 $previ \"$fin + 2 char\"");
  cmd(inter, "set previ [lindex [.f.t.t tag nextrange comment1 $previ] 1]");
  cmd(inter, "if {[string compare $previ \"\"]==0} {set previ end; set i 1} {set i 0;}");


  }
if(i==4)
 { //in case of strings

  cmd(inter, "scan $previ %d.%d line trash");
  cmd(inter, "set fin [.f.t.t search \"\\\"\" \"$previ+1char\" $endcolor]");
  cmd(inter, "if { [string length $fin] == 0} {set fin $line.end} {while {[string length $fin]>0 && [string compare [.f.t.t get \"$fin - 1char\"] \"\\\\\"]==0} {set fin [.f.t.t search \"\\\"\" \"$fin+1char\" $endcolor]; }}");
  cmd(inter, "if {$fin == \"\"} {set fin $line.end} {}");

  cmd(inter, "if {[.f.t.t compare $fin > $line.end]==1} {set fin $line.end} {}");
  cmd(inter, ".f.t.t tag remove comment1 $previ \"$fin + 1 char\"");
  cmd(inter, ".f.t.t tag remove comment2 $previ \"$fin + 1 char\"");
  cmd(inter, ".f.t.t tag add str $previ \"$fin + 1 char\"");
//   cmd(inter, "if {[string compare $fin \"\"]==0} {set fin $endcolor; set i 1} {set i 0}");
  cmd(inter, "set previ \"$fin+1 char\"");
  i=0;
 }

cmd(inter, "if {[.f.t.t compare $previ >= $endcolor]==1} {set i 1} {}");

}

Tcl_UnlinkVar(inter, "i");
}


/*
Create the makefile for a model, merging:
1) model_options.txt
2) src/system_options.txt
3) src/makefile_base.txt
*


void make_makefile(void)
{
FILE *sys, *modopt, *bas, *mak;
char line[200];

cmd(inter, "set here [pwd]");
cmd(inter, "cd $RootLsd");
sys=fopen("src/system_options.txt", "r");
bas=fopen("src/makefile_base.txt", "r");
cmd(inter, "cd $here");
cmd(inter, "pwd");
modopt=fopen("model_options.txt", "r");


if(sys==NULL || modopt==NULL || bas==NULL)
 {
  cmd(inter, "tk_messageBox -type ok -message \"Cannot open one of the file required for 'makefile' creation.\\n\""); 
  if(sys!=NULL)
   fclose(sys);
  if(modopt!=NULL)
   fclose(modopt);
  if(bas!=NULL)
   fclose(bas);

  return;
     
 }

mak=fopen("makefile", "w");
if(mak==NULL)
 {
 cmd(inter, "tk_messageBox -type ok -message \"Cannot create the makefile.\""); 
 cmd(inter, "cd $RootLsd");
 return;

 }  


while(fgets(line, 200, modopt)!=(char *)NULL)
 fputs(line, mak);

while(fgets(line, 200, sys)!=(char *)NULL)
 fputs(line, mak);

while(fgets(line, 200, bas)!=(char *)NULL)
 fputs(line, mak);

fclose(sys);
fclose(modopt);
fclose(mak);
fclose(bas);

 
}
*************/


void make_makefile(void)
{

 

cmd(inter, "set choice [file exists model_options.txt]");
if(choice==0)
 {//the model_options.txt file does not exists, probably an old version
   cmd(inter, "set dir [glob *.cpp]");
   cmd(inter, "set b [lindex $dir 0]");
   cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\n\"");
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
//cmd(inter, "if {$tcl_platform(platform) == \"windows\" && [file exists crt0.o] == 0} { file copy $RootLsd/gnu/bin/crt0.o .} {}");
}





void make_makefileNW(void)
{

 
cmd(inter, "set choice [file exists model_options.txt]");
if(choice==0)
 {//the model_options.txt file does not exists, probably an old version
   cmd(inter, "set dir [glob *.cpp]");
   cmd(inter, "set b [lindex $dir 0]");
   cmd(inter, "set a \"TARGET=lsd_gnu\\nFUN=[file rootname $b]\\nSWITCH_CC=-g\\n\"");
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
//cmd(inter, "if {$tcl_platform(platform) == \"windows\" && [file exists crt0.o] == 0} { file copy $RootLsd/gnu/bin/crt0.o .} {}");
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
cmd(inter, "label .mm.lab1 -text \"Compilation errors\" -fg red");

cmd(inter, "label .mm.lab -justify left -text \"- Each error is indicated by the file name and line number where it has been identified.\n- Check the relative file and search on the indicated line number, considering that the error may have occurred in the previous line.\n- Fix first errors at the beginning of the list, since the following errors may be due to previous ones.\"");
cmd(inter, "pack .mm.lab1 .mm.lab");

cmd(inter, "text .mm.t -yscrollcommand \".mm.yscroll set\" -wrap word; scrollbar .mm.yscroll -command \".mm.t yview\"");


cmd(inter, "pack .mm.yscroll -side right -fill y; pack .mm.t -expand yes -fill both; wm geom .mm -0+0; wm title .mm \"Compilation Errors List\"");


cmd(inter, "frame .mm.b -relief groove -bd 2");

cmd(inter, "button .mm.b.ferr -text \" Next error \" -command {set errtemp [.mm.t search -- error $cerr end]; if { [string length $errtemp] == 0} {} { set cerr \"$errtemp +1ch\";  .mm.t mark set insert $cerr; .mm.t tag remove sel 1.0 end; .mm.t tag add sel $errtemp \"$errtemp + 5ch\";  .mm.t see $cerr} }");

cmd(inter, "button .mm.b.perr -text \" Previous error \" -command {set errtemp [ .mm.t search -backward -- error $cerr 0.0];  if { [string length $errtemp] == 0} {} { set cerr \"$errtemp -1ch\";  .mm.t mark set insert $cerr; .mm.t tag remove sel 1.0 end; .mm.t tag add sel $errtemp \"$errtemp + 5ch\"; .mm.t see $cerr} }");

cmd(inter, "pack .mm.b.ferr .mm.b.perr -expand yes -fill x -side left");
cmd(inter, "pack .mm.b -expand yes -fill x");
cmd(inter, "button .mm.close -text \" Close \" -command {destroy .mm}");
cmd(inter, "pack .mm.close -side bottom");

cmd(inter, "bind .mm <Escape> {destroy .mm}");
cmd(inter, "focus -force .mm");
cmd(inter, "set file [open $modeldir/makemessage.txt]");
cmd(inter, ".mm.t insert end [read $file]");
cmd(inter, "close $file");
cmd(inter, ".mm.t mark set insert 1.0");
}


















