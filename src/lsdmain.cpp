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


/****************************************************
LSD_MAIN.CPP contains:
- early initialization (namely, of the Log windows)
- the main cycle: browse a model, run simulation, return to the browser.


The functions contained here are:

- void run(object *r)
Run the simulation model whose root is r. Running is not only the actual
simulation run, but also the initialization of result files. Of course, it has
also to manage the messages from user and from the model at run time.

- void print_title(object *root,char *tag, int counter, int *done);
Prepare variables to store saved data.

- Tcl_Interp *InterpInitWin(char *tcl_dir);
A function that manages to initialize the tcl interpreter. Guess the standard
functions are actually bugged, because of the difficulty to retrive the directory
for the tk library. Why is difficult only for tk and not for tcl, don't know.
But is a good thing, so that I can actually copy the tcl directory and make
the modifications

- void plog(char *m);
print  message string m in the Log screen.

Other functions used here, and the source files where are contained:

- object *create( object *r);
manage the browser. Its code is in INTERF.CPP

- object *skip_next_obj(object *t, int *count);
Contained in UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- object *go_brother(object *c);
Contained in UTIL.CPP. returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.


- void cmd(Tcl_Interp *inter, char *cc);
Contained in UTIL.CPP. Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

- void myexit(int v);
Exit function, which is customized on the operative system.

- FILE *search_str(char *name, char *str);
UTIL.CPP given a string name, returns the file corresponding to name, and the current
position of the file is just after str.


****************************************************/

#include "decl.h"
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#ifndef NO_WINDOW
#include <tk.h>
void cmd(Tcl_Interp *inter, char  const *cc);
Tcl_Interp *InterpInitWin(char *tcl_dir);
int Tcl_discard_change( ClientData, Tcl_Interp *, int, const char *[] );	// ask before discarding unsaved changes
int Tcl_get_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[] );
int Tcl_set_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[] );
Tcl_Interp *inter;
#endif

#ifdef LIBZ
int dozip = 1;				// compressed results file flag
#else
int dozip = 0;
#endif

// launch TkCon as an auxiliary window for debugging (comment to disable)
//#define TKCON

object *create( object *r);
void run(object *r);
void print_title(object *root);
object *skip_next_obj(object *t, int *count);
object *skip_next_obj(object *t);
object *go_brother(object *c);
void plog( char const *msg, char const *tag = "" );
void file_name( char *name);
void prepare_plot(object *r, int id_sim);
void create_logwindow(void);
void cover_browser( const char *, const char *, const char * );
void uncover_browser( void );
void myexit(int v);
FILE *search_str(char const *name, char const *str);
void set_lab_tit(variable *var);
void reset_end(object *r);
void close_sim(void);
int deb(object *r, object *c, char const *lab, double *res);
void analysis(int *choice);
void init_random(int seed);
void add_description(char const *lab, char const *type, char const *text);
void empty_cemetery(void);
void save_single(variable *vcv);
int load_configuration( object *, bool reload = false );
char *clean_file(char *);
char *clean_path(char *);
char *upload_eqfile(void);

extern bool fast;			// fast mode (log window)
extern double ymin;
extern double ymax;
extern long nodesSerial;

int t;
int quit=0;
int done_in;
int debug_flag;
int when_debug;
int stackinfo_flag=0;
int optimized=0;
int check_optimized=0;
int plot_flag=1;
double refresh=1.01;
char lsd_eq_file[ MAX_FILE_SIZE + 1 ];
int watch;
char msg[TCL_BUFF_STR];
int stack;
int identity=0;
int sim_num=1;
int cur_sim;
int cur_plt;
char ind[30];
int total_var=0;
int total_obj=0;
int choice;
int choice_g;
lsdstack *stacklog = NULL;
object *root = NULL;
object *blueprint = NULL;
variable *cemetery=NULL;
description *descr = NULL;
char *path = NULL;
char *alt_path = NULL;
char *simul_name = NULL;
char *struct_file = NULL;
char *eq_file=NULL;
char *equation_name = NULL;
char *exec_file = NULL;		// name of executable file
char *exec_path = NULL;		// path of executable file
char name_rep[1000];
char **tp;
int struct_loaded=0;
int running=0;
int actual_steps=0;
int counter;
int no_window=0;
int message_logged=0;
int no_more_memory=0;
int series_saved;
int findex, fend;
int batch_sequential=0;
char *sens_file=NULL;		// current sensitivity analysis file
long findexSens=0;			// index to sequential sensitivity configuration filenames
bool pause_run;				// pause running simulation
bool scroll;				// scroll state in current runtime plot
bool justAddedVar=false;	// control the selection of last added variable
bool unsavedSense = false;	// control for unsaved changes in sensitivity data
bool redrawRoot = true;		// control for redrawing root window (.)
bool save_alt_path = false;	// alternate save path flag

// flags for some program defaults
int seed = 1;				// random number generator initial seed
int max_step = 100;			// default number of simulation runs
int no_res = 0;				// produce intermediary .res files flag
int overwConf = 1;			// overwrite configuration on run flag
int ignore_eq_file = 1;		// flag to ignore equation file in configuration file
int add_to_tot = 1;			// flag to append results to existing totals file
int strWindowOn=1;			// control the presentation of the model structure window
int lattice_type=0;			// default lattice type (infrequent cells changes)
bool grandTotal = false;	// flag to produce or not grand total in batch processing
bool firstRes = true;		// flag to mark first results file (init grand total file)
bool unsavedData = false;	// flag unsaved simulation results
char nonavail[] = "NA";		// string for unavailable values (use R default)
// Main window constraints (even numbers only)
char hsize[] = "400";		// horizontal size in pixels (minimum)
char vsize[] = "620";		// vertical minimum size in pixels (2 x minimum)
char hmargin[] = "20";		// horizontal right margin from the screen borders
char vmargin[] = "20";		// vertical margins from the screen borders
// Log window tabs
char tabs[] = "5c 7.5c 10c 12.5c 15c 17.5c 20c";


/*********************************
LSD MAIN
*********************************/
int lsdmain(int argn, char **argv)
{
char tcl_dir[1000], str[1000], *lsdroot;
int i, len, done;
FILE *f;

blueprint=NULL;
simul_name=new char[300];
path=new char[1000];
equation_name=new char[300];
exec_file=clean_file(argv[0]);	// global pointer to the name of executable file
exec_path=clean_path(getcwd(NULL, 0));	// global pointer to path of executable file

strcpy(path, "");
strcpy(tcl_dir, "");
strcpy(simul_name, "Sim1");
strcpy(equation_name,"fun.cpp");
#ifdef NO_WINDOW

findex=1;
fend=0;		// no file number limit

if(argn<3)
 {
  printf("\nThis is the No Window version of Lsd. Command line options:\n'-f FILENAME.lsd' to run a single configuration file\n'-f FILE_BASE_NAME -s FIRST_NUM [-e LAST_NUM]' for batch sequential mode\n'-o PATH' to save result file(s) to a different subdirectory\n'-r' for skipping the generation of intermediate result file(s)\n'-g' for the generation of a single grand total file\n'-z' for preventing the generation of compressed result file(s)\n");
  myexit(1);
 }
else
 {
 for(i=1; i<argn; i+=2)
 {
 if(argv[i][0]=='-' && argv[i][1]=='f' )
  {
   delete[] simul_name;
   simul_name=new char[strlen(argv[1+i])+1];
   strcpy(simul_name,argv[1+i]);
   continue;
  }
 if(argv[i][0]=='-' && argv[i][1]=='s' )
  {
 	 findex=atoi(argv[i+1]);
  batch_sequential=1;   
   continue;
  }
 if(argv[i][0]=='-' && argv[i][1]=='e' )	// read -e parameter : last sequential file to process
  {
 	 fend=atoi(argv[i+1]);
   continue;
  }
 if( argv[i][0] == '-' && argv[i][1] == 'r' )	// read -r parameter : do not produce intermediate .res files
 {
	i--; 	// no parameter for this option
	no_res = 1;
    continue;
 }
 if( argv[i][0] == '-' && argv[i][1] == 'g' )	// read -g parameter : create grand total file (batch only)
 {
	i--; 	// no parameter for this option
	grandTotal = true;
	printf( "Grand total file requested ('-g'), please don't run another instance of Lsd_gnuNW in this folder!\n" );
	continue;
 }
 if( argv[i][0] == '-' && argv[i][1] == 'z' )	// read -g parameter : don't create compressed result files
 {
	i--; 	// no parameter for this option
	dozip = false;
	continue;
 }
 if( argv[i][0] == '-' && argv[i][1] == 'o' )	// change the path for the output of result files
 {
	delete [ ] alt_path;
	alt_path = new char [ strlen( argv[ 1 + i ] ) + 1 ];
	strcpy( alt_path, argv[ 1 + i ] );
	save_alt_path = true;
	continue;
 }
  
  printf("\nOption '%c%c' not recognized.\nThis is the No Window version of Lsd. Command line options:\n'-f FILENAME.lsd' to run a single configuration file\n'-f FILE_BASE_NAME -s FIRST_NUM [-e LAST_NUM]' for batch sequential mode\n'-o PATH' to save result file(s) to a different subdirectory\n'-r' for skipping the generation of intermediate result file(s)\n'-g' for the generation of a single grand total file\n'-z' for preventing the generation of compressed result file(s)\n", argv[i][0], argv[i][1]);
  myexit(2);
  }
 } 

#else 
for(i=1; argv[i]!=NULL; i++)
{if(argv[i][0]!='-' || (argv[i][1]!='f' && argv[i][1]!='i') )
  {printf(msg,"\nInvalid option: %s\nAvailable options:\n-i tcl_directory\n-f model_name\n", argv[i]);
	myexit(1);
  }
 if(argv[i][1]=='f')
	{delete[] simul_name;
	 simul_name=new char[strlen(argv[i+1])+3];
	 strcpy(simul_name,argv[i+1]);
    len=strlen(simul_name);
    if(len>4 && !strcmp(".lsd",simul_name+len-4) )
     *(simul_name+len-4)=(char)NULL;
    i++;
	}
 if(argv[i][1]=='i')
	{
   strcpy(tcl_dir,argv[i+1]+2);
   i++;
  } 
}

grandTotal = true;				// not in parallel mode: use .tot headers
struct_file=new char[strlen(simul_name)+5];
sprintf(struct_file, "%s.lsd", simul_name);
#endif

// register critical signal handlers
void signal_handler(int);
signal(SIGFPE, signal_handler);  
signal(SIGILL, signal_handler);  
signal(SIGSEGV, signal_handler);  

root=new object;
root->init(NULL, "Root");
add_description("Root", "Object", "(no description available)");
blueprint=new object;
blueprint->init(NULL, "Root");

#ifdef NO_WINDOW
no_window=1;

if ( batch_sequential )
 {
 struct_file=new char[strlen(simul_name)+1];
 sprintf(struct_file, "%s", simul_name);
 simul_name[strlen(simul_name)-4]='\0';
 } 
else
 {
  sprintf(msg, "%s_%d.lsd", simul_name,findex);
  struct_file=new char[strlen(msg)+1];
  strcpy(struct_file,msg);
 }
 
f=fopen(struct_file, "r");
if(f==NULL)
 {
  printf("\nFile %s not found.\nThis is the no window version of Lsd. Specify a -f filename.lsd to run a simulation or -f simul_name -s 1 for batch sequential simulation mode (requires configuration files: simul_name_1.lsd, simul_name_2.lsd, etc).\n", struct_file);
  myexit(3);
 }
fclose(f);
struct_loaded = true;

if ( load_configuration( root, false ) != 0 )
{
	printf( "\nFile %s is invalid.\nThis is the no window version of Lsd. Check if the file is a valid Lsd configuration or regenerate it using the Lsd Browser.\n", struct_file );
	myexit(3);
}

#else 
Tcl_FindExecutable(argv[0]); 
inter=InterpInitWin(tcl_dir);
if(inter==NULL)
 myexit(4);

eq_file=upload_eqfile();
lsd_eq_file[0]=(char)NULL;

// use the new exec_path variable to change to the model directory
sprintf(msg, "cd \"%s\"", exec_path);
cmd(inter, msg);

Tcl_LinkVar(inter, "choice", (char *) &choice, TCL_LINK_INT);
Tcl_LinkVar(inter, "debug_flag", (char *) &debug_flag, TCL_LINK_INT);
Tcl_LinkVar(inter, "when_debug", (char *) &when_debug, TCL_LINK_INT);

cmd(inter, "if { [string first \" \" \"[pwd]\" ] >= 0  } {set debug_flag 1} {set debug_flag 0}");
if(debug_flag==1)
 {
 cmd(inter, "tk_messageBox -parent . -title Error -icon error -type ok -message \"Spaces in file path\" -detail \"The directory containing the model is:\n[pwd]\nIt appears to include spaces. This will make impossible to compile and run Lsd model. The Lsd directory must be located where there are no spaces in the full path name.\nMove all the Lsd directory and delete the 'system_options.txt' file from the \\src directory.\n\nLsd is aborting now.\"");
 myexit(5);
 
 }

cmd(inter, "tk appname browser");
// check if LSDROOT already exists and use it if so, if not, search the current directory tree
cmd( inter, "if [ info exists env(LSDROOT) ] { set RootLsd $env(LSDROOT); if { ! [ file exists \"$RootLsd/src/interf.cpp\" ] } { unset RootLsd } }" );
cmd( inter, "if { ! [ info exists RootLsd ] } { set here [ pwd ]; while { ! [ file exists \"src/interf.cpp\" ] && [ string length [ pwd ] ] > 3 } { cd .. }; if [ file exists \"src/interf.cpp\" ] { set RootLsd [ pwd ] } { set RootLsd \"\" }; cd $here; set env(LSDROOT) $RootLsd }" );
lsdroot = ( char * ) Tcl_GetVar( inter, "RootLsd", 0 );
if ( lsdroot == NULL || strlen( lsdroot ) == 0 )
 {
 cmd(inter, "tk_messageBox -parent . -title Error -icon error -type ok -message \"LSDROOT not set\" -detail \"Please make sure the environment variable LSDROOT points to the directory where Lsd is installed.\n\nLsd is aborting now.\"");
 myexit(6);
 }
strcpy( str, lsdroot );
lsdroot = ( char * ) calloc( strlen( str ) + 1, sizeof ( char ) );
strcpy( lsdroot, str );
sprintf(msg, "set RootLsd \"%s\"",lsdroot);
len=strlen(msg);
for(i=0; i<len; i++)
 if(msg[i]=='\\')
  msg[i]='/';
cmd(inter, msg);

Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);
cmd(inter, "set done [file exist $RootLsd/lmm_options.txt]");
if(done==1)
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
  cmd(inter, "tk_messageBox -parent . -title Warning -icon warning -type ok -message \"Could not locate LMM system options\" -detail \"It may be impossible to open help files and compare the equation files. Any other functionality will work normally. When possible set in LMM the system options in menu File.\"");
 }

Tcl_UnlinkVar(inter, "done");

// create a Tcl command that calls the C discard_change function before killing Lsd
Tcl_CreateCommand( inter, "discard_change", Tcl_discard_change, NULL, NULL );

// create Tcl commands that get and set LSD variable properties
Tcl_CreateCommand( inter, "get_var_conf", Tcl_get_var_conf, NULL, NULL );
Tcl_CreateCommand( inter, "set_var_conf", Tcl_set_var_conf, NULL, NULL );

// set window icon
cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {wm iconbitmap . -default $RootLsd/$LsdSrc/icons/lsd.ico} {wm iconbitmap . @$RootLsd/$LsdSrc/icons/lsd.xbm}");
// set position parameters now
Tcl_SetVar(inter, "widthB", hsize, 0);		// horizontal size in pixels
Tcl_SetVar(inter, "heightB", vsize, 0);		// vertical minimum size in pixels
Tcl_SetVar(inter, "posX", hmargin, 0);		// horizontal right margin from the screen borders
Tcl_SetVar(inter, "posY", vmargin, 0);		// vertical margins from the screen borders
cmd(inter, "wm geometry . \"[expr $widthB]x$heightB+$posX+$posY\"");
cmd( inter, "wm minsize . [ expr $widthB ] [ expr $heightB / 2 ]" );

cmd(inter, "label .l -text \"Starting Lsd\"");
cmd(inter, "pack .l");
cmd( inter, "wm protocol . WM_DELETE_WINDOW { if { [ discard_change ] == \"ok\" } { exit } { } }" ); 
cmd(inter, "update");

sprintf(name_rep, "modelreport.html");

f=fopen("makefile", "r");
if(f!=NULL)
 {
 fscanf(f, "%99s", msg);
 while(strncmp(msg, "FUN=", 4) && f!=(FILE *)EOF)
    fscanf(f, "%99s", msg);
 fclose(f);
 sprintf(equation_name, "%s.cpp",msg+4);
 }

create_logwindow( );
cmd(inter, "destroy .l");
#endif

stacklog = new lsdstack;
stacklog->next=NULL;
stacklog->prev=NULL;
stacklog->ns=0;
stacklog->vs=NULL;
strcpy(stacklog->label, "Lsd Simulation Manager");

#ifndef NO_WINDOW
cmd( inter, "if [ file exists $RootLsd/$LsdSrc/align.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/align.tcl } ] == 0 } { set choice 0 } { set choice 1 } } { set choice 2 }; if { $choice != 0 } { tk_messageBox -parent . -type ok -icon error -title Error -message \"File 'src/align.tcl' missing or corrupted\" -detail \"Please check your installation and reinstall Lsd if required.\n\nLsd is aborting now.\" }" );
if ( choice != 0 )
	myexit( 7 + choice );

cmd( inter, "if [ file exists $RootLsd/$LsdSrc/ls2html.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/ls2html.tcl } ] == 0 } { set choice 0 } { set choice 1 } } { set choice 2 }; if { $choice != 0 } { tk_messageBox -parent . -type ok -icon error -title Error -message \"File 'src/ls2html.tcl' missing or corrupted\" -detail \"Please check your installation and reinstall Lsd if required.\n\nLsd is aborting now.\" }" );
if ( choice != 0 )
	myexit( 7 + choice );

#ifdef TKCON
// launch TkCon as an auxiliary window for debugging
cmd( inter, "if [ file exists $RootLsd/$LsdSrc/tkcon.tcl ] { if { [ catch { source $RootLsd/$LsdSrc/tkcon.tcl } ] == 0 } { package require tkcon; set tkcon::PRIV(showOnStartup) 0; set tkcon::PRIV(root) .console; set tkcon::PRIV(protocol) {tkcon hide}; set tkcon::OPT(exec) \"\"; tkcon::Init; tkcon title \"Tcl/Tk Debug Console\"; tkcon show } { tk_messageBox -parent . -type ok -icon error -title Error -message \"File 'src/tkcon.tcl' missing or corrupted\" -detail \"Please check your installation and reinstall Lsd if required.\n\nLsd is continuing with no debug console.\" } }" );
#endif

while(1)
{
root=create( root);
no_more_memory=0;
series_saved=0;
try {
run(root);
}

/**/
catch(int p)
 {
 while(stacklog->prev!=NULL)
   stacklog=stacklog->prev;
 stack=0; 
 }
/**/ 
}

#else
 run(root);
#endif 

empty_cemetery();
blueprint->empty();
root->empty();
delete blueprint;
delete root;
delete stacklog;
delete [ ] struct_file;
delete [ ] equation_name;
delete [ ] path;
delete [ ] simul_name;

return 0;
}


/*********************************
RUN
*********************************/
void run(object *root)
{
int i, j, done=0;
bool batch_sequential_loop=false; // indicates second or higher iteration of a batch
char ch[300], nf[300];
FILE *f;
result *rf;					// pointer for results files (may be zipped or not)
double app=0;
clock_t start, end;

done_in=done=0;
quit=0;
 
#ifndef NO_WINDOW
Tcl_LinkVar(inter, "done_in", (char *) &done_in, TCL_LINK_INT);
Tcl_SetVar(inter, "done", "0", 0);

cover_browser( "Running...", "The simulation is being executed", "Use the Lsd Log window buttons to interact during execution:\n\n'Stop' :  stops the simulation\n'Pause' :  pauses and resumes the simulation\n'Fast' :  accelerates the simulation by hiding information\n'Observe' :  presents more run time information\n'Debug' :  trigger the debugger at a flagged variable" );
cmd( inter, "wm deiconify .log; raise .log; focus .log" );

#else
sprintf(msg, "\nProcessing configuration file %s ...\n",struct_file);
plog(msg);
#endif

for(i=1; i<=sim_num && quit!=2; i++)
{
cur_sim = i;	 //Update the global variable holding information on the current run in the set of runs
empty_cemetery(); //ensure that previous data are not erroneously mixed (sorry Nadia!)

#ifndef NO_WINDOW
prepare_plot(root, i);
#endif

sprintf(msg, "\nSimulation %d running...", i);
plog(msg);

// if new batch configuration file, reload all
if ( batch_sequential_loop )
{
	if ( load_configuration( root, false ) != 0 )
	{
#ifndef NO_WINDOW 
		cmd( inter, "tk_messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be loaded\" -detail \"Check if Lsd still has WRITE access to the model directory.\nLsd will close now.\"" );
#else
		sprintf(msg, "\nFile %s not found or corrupted", struct_file);
		plog(msg);
#endif
		myexit(9);
	}
	batch_sequential_loop = false;
}

// if just another run seed, reload just structure & parameters
if ( i > 1 )
	if ( load_configuration( root, true ) != 0 )
	{
#ifndef NO_WINDOW 
		cmd( inter, "tk_messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be reloaded\" -detail \"Check if Lsd still has WRITE access to the model directory.\nLsd will close now.\"" );
#else
		sprintf(msg, "\nFile %s not found or corrupted", struct_file);
		plog(msg);
#endif
		myexit(9);
	}

strcpy(ch, "");
series_saved=0;

print_title(root);
if(no_more_memory==1)
 {
#ifndef NO_WINDOW 
 sprintf(msg, "tk_messageBox -parent . -type ok -icon error -title Error -message \"Not enough memory\" -detail \"Too many series saved for the available memory. Memory insufficient for %d series over %d time steps. Reduce series to save and/or time steps.\nLsd will close now.\"", series_saved, max_step);
 cmd(inter, msg);
#else
 sprintf(msg, "\nNot enough memory. Too many series saved for the memory available.\nMemory insufficient for %d series over %d time steps.\nReduce series to save and/or time steps.\n", series_saved, max_step);
 plog(msg);
#endif
 myexit(10);
 }
 
//new random routine' initialization
init_random(seed);

seed++;
stack=0;

scroll = false;
pause_run = false;
done_in = 0;
debug_flag = 0;
running = 1;
actual_steps = 0;
start = clock();

for(t=1; quit==0 && t<=max_step;t++)
{
	
// adjust "clock" backwards if simulation is paused
if ( pause_run )
	t--;

#ifndef NO_WINDOW 
if(when_debug==t)
{
  debug_flag=1;
  cmd( inter, "if [ winfo exists .deb ] { wm deiconify .deb; raise .deb; focus -force .deb; update idletasks }" );
}
#endif

cur_plt=0;

// only update if simulation not paused
if ( ! pause_run )
	root->update();

#ifndef NO_WINDOW 
switch( done_in )
{
case 0:
 if ( ! pause_run && ! fast )
  {
	if(cur_plt==0)
	{
		sprintf(msg,"\nSim. %d step %d done",i,t);
		plog(msg);
	}
  }
break;

case 1:			// Stop button in Log window / s/S key in Runtime window
  if ( pause_run )
	cmd( inter, "wm title .log \"$origLogTit\"" );
  sprintf(msg, "\nSimulation stopped at t = %d", t);
  plog(msg);
  quit=2;
break;

case 2:			// Fast button in Log window / f/F key in Runtime window
 fast = true;
 debug_flag=0;
 cmd(inter, "set a [split [winfo children .] ]");
 cmd(inter, " foreach i $a {if [string match .plt* $i] {wm iconify $i}}");
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.go conf -state disabled} {}",i, i);
 cmd(inter, msg);
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.shift conf -state disabled} {}", i, i);
 cmd(inter, msg);
break;

case 3:			// Debug button in Log window / d/D key in Runtime window
if ( ! pause_run )
{
	debug_flag=1;
	cmd( inter, "if [ winfo exists .deb ] { wm deiconify .deb; raise .deb; focus -force .deb }" );
}
else			// if paused, just call the data browser
{
	double useless = 0;
	deb( root, NULL, "Paused by User", &useless );
}
	
break;

case 4:			// Observe button in Log window / o/O key in Runtime window
 fast = false;
 cmd(inter, "set a [split [winfo children .] ]");
 cmd(inter, " foreach i $a {if [string match .plt* $i] {wm deiconify $i; raise $i}}");
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.go conf -state normal} {}",i, i);
 cmd(inter, msg);
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.shift conf -state normal} {}",i, i);
 cmd(inter, msg);
break;
 
// plot window DELETE_WINDOW button handler
case 5:
 if ( pause_run )
	cmd( inter, "wm title .log \"$origLogTit\"" );
 sprintf(msg, "if { [winfo exist .plt%d]} {destroy .plt%d} {}", i, i);
 cmd(inter, msg);
 sprintf(msg, "\nSimulation stopped at t = %d", t);
 plog(msg);
 quit=2;
break;

case 6:
 Tcl_LinkVar(inter, "app_refresh", (char *) &app, TCL_LINK_DOUBLE);
 cmd(inter, "set app_refresh $refresh");
 Tcl_UnlinkVar(inter, "app_refresh");
 if(app<2 && app > 1)
  refresh=app;
break; 

// runtime plot events
case 7:  		// Center button
 sprintf(msg, "set newpos [expr %lf - [expr 250 / %lf]]", (double)t/(double)max_step, (double)max_step);
 cmd(inter, msg);
 sprintf(msg,"if { [winfo exist .plt%d]} {$activeplot.c.c.cn xview moveto $newpos} {}", i);
 cmd(inter, msg);
break;

case 8: 		// Scroll checkbox
 scroll = ! scroll;
break;

case 9: 		// Pause simulation
 pause_run = ! pause_run;
 if ( pause_run )
 {
	cmd( inter, "set origLogTit [ wm title .log ]; wm title .log \"$origLogTit (PAUSED)\"" );
	sprintf( msg, "\nSimulation paused at t = %d", t );
	plog( msg );
	cmd(inter, "bind .log <KeyPress-p> { }; bind .log <KeyPress-P> { }");
	cmd( inter, ".log.but.pause conf -text Resume" );
	cmd(inter, "bind .log <KeyPress-r> { .log.but.pause invoke }; bind .log <KeyPress-R> { .log.but.pause invoke }");
 }
 else
 {
	cmd( inter, "wm title .log \"$origLogTit\"" );
	plog( "\nSimulation resumed" );
	cmd(inter, "bind .log <KeyPress-r> { }; bind .log <KeyPress-R> { }");
	cmd( inter, ".log.but.pause conf -text Pause" );
	cmd(inter, "bind .log <KeyPress-p> { .log.but.pause invoke }; bind .log <KeyPress-P> { .log.but.pause invoke }");
 }
break;

case 35:
 myexit(11);
break;

default:
break;
}

// perform scrolling if enabled
if ( ! pause_run && scroll )
{
	sprintf(msg,"if { [winfo exist .plt%d]} {$activeplot.c.c.cn xview scroll 1 units} {}",i);
	cmd(inter, msg);
}

done_in = 0;
cmd(inter, "update");
#endif
}//end of for t

actual_steps=t-1;
unsavedData = true;						// flag unsaved simulation results
running=0;
end=clock();

if(quit==1) 			//For multiple simulation runs you need to reset quit
 quit=0;

sprintf(msg, "\nSimulation %d finished (%2g sec.)\n",i,(float)(end - start) /CLOCKS_PER_SEC);
plog(msg);

#ifndef NO_WINDOW 
cmd( inter, "update" );
// allow for run time plot window destruction
sprintf( msg, "if [ winfo exists .plt%d ] { wm protocol .plt%d WM_DELETE_WINDOW \"\"; .plt%d.c.yscale.go conf -state disabled; .plt%d.c.yscale.shift conf -state disabled }", i, i, i, i );
cmd( inter, msg);
#endif

close_sim();
reset_end(root);
root->emptyturbo();

if ( sim_num > 1 || no_window ) //Save results for multiple simulation runs
{

if ( ! no_res )
{
if ( ! batch_sequential )
  sprintf( msg, "%s%s%s_%d.res", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", simul_name, seed - 1 );
else
  sprintf( msg, "%s%s%s_%d_%d.res", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", simul_name, findex, seed - 1 );

sprintf( ch, "Saving results in file %s%s... ", msg, dozip ? ".gz" : "" );
plog ( ch );

rf = new result( msg, "wt", dozip );				// create results file object
rf->title( root, 1 );							// write header
rf->data( root, 0, actual_steps );				// write all data
delete rf;										// close file and delete object

plog("Done\n");
}

if ( ! grandTotal || batch_sequential )			// generate partial total files?
{
	if ( ! batch_sequential )
	  sprintf( msg, "%s%s%s_%d_%d.tot", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", simul_name, seed - i, seed - 1 + sim_num - i );
	else
	  sprintf( msg, "%s%s%s_%d_%d_%d.tot", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", simul_name, findex, seed - i, seed - 1 + sim_num - i );
}
else											// generate single grand total file
{
	sprintf( msg, "%s%s%s.tot", save_alt_path ? alt_path : path, strlen( save_alt_path ? alt_path : path ) > 0 ? "/" : "", simul_name );
}

if ( i == sim_num )								// print only for last
{
	sprintf( ch, "\nSaving totals in file %s%s... ", msg, dozip ? ".gz" : "" );
	plog( ch );
}

if ( ( i == 1 && ! add_to_tot ) || ( grandTotal && firstRes ) )
{
	rf = new result( msg, "wt", dozip );		// create results file object
	rf->title( root, 0 );						// write header
	firstRes = false;
}
else
	rf = new result( msg, "a", dozip );			// add results object to existing file

rf->data( root, actual_steps );					// write current data data
delete rf;										// close file and delete object

if ( i == sim_num )								// print only for last
	plog( "Done\n" );

if ( batch_sequential && i == sim_num)  		// last run of current batch file?
 {
   findex++;									// try next file
   sprintf(msg, "%s_%d.lsd",simul_name,findex);
   delete[] struct_file;
   struct_file=new char[strlen(msg)+1];
   strcpy(struct_file,msg);
   f=fopen(struct_file, "r");			
   if(f==NULL || (fend!=0 && findex>fend))  	// no more file to process
   {
	 if(f!=NULL) fclose(f);
     sprintf(msg, "\nFinished processing %s.\n",simul_name);
     plog(msg);
     break;
   }
   sprintf(msg, "\nProcessing configuration file %s ...\n",struct_file);
   plog(msg);
   fclose(f);  									// process next file
   struct_loaded = true;
   i=0;   										// force restarting run count
   batch_sequential_loop=true;
 } 
}
}

#ifndef NO_WINDOW 
uncover_browser( );
cmd( inter, "wm deiconify .log; raise .log; focus .log" );
Tcl_UnlinkVar(inter, "done_in");
#endif
quit=0;
}


/*********************************
PRINT_TITLE
*********************************/

void print_title(object *root)
{
object *c, *cur;
variable *var;
int num=0, multi, toquit;
bridge *cb;


toquit=quit;
//for each variable set the data saving support
for(var=root->v; var!=NULL; var=var->next)
 {
 var->last_update=0;

	if( (var->save || var->savei) && no_more_memory==0)
	 {
     if(var->num_lag>0 || var->param==1)
       var->start=0;
     else
       var->start=1;
     var->end=max_step;
     if(var->data!=NULL)
      delete[] var->data;
    try {
     var->data=new double[max_step+1];
     series_saved++;
     if(var->num_lag>0  || var->param==1)
      var->data[0]=var->val[0];
     }
    
    catch(...) {
     set_lab_tit(var);
       sprintf(msg, "\nNot enough memory.\nData for %s and subsequent series will not be saved.\n",var->lab_tit);
        plog(msg);
        var->save=0;
        no_more_memory=1;
       }
    }
   else
    {
     if(no_more_memory==1)
      var->save=0;
    }
	if(var->data_loaded=='-')
	  {sprintf(msg,"\nData for %s in object %s not loaded\n", var->label, root->label);
		plog(msg);
		plog("Use the Initial Values editor to set its values\n");
     #ifndef NO_WINDOW   
     if(var->param==1)
       sprintf(msg, "tk_messageBox -parent . -type ok -icon error -title Error -message \"Run aborted\" -detail \"The simulation cannot start because parameter:\n'%s' (object '%s')\nhas not been initialized.\nUse the browser to show object '%s' and choose menu 'Data'/'Initìal Values'.\"", var->label, root->label, root->label);
     else
       sprintf(msg, "tk_messageBox -parent . -type ok -icon error -title Error -message \"Run aborted\" -detail \"The simulation cannot start because a lagged value for variable:\n'%s' (object '%s')\nhas not been initialized.\nUse the browser to show object '%s' and choose menu 'Data'/'Init.Values'.\"", var->label, root->label, root->label);
     cmd(inter, msg);  
     #endif
		toquit=2;
	  }
 }

for(cb=root->b, counter=1; cb!=NULL; cb=cb->next, counter=1)
 {for(cur=cb->head; cur!=NULL && quit!=2; cur=go_brother(cur))
   {
   print_title(cur);

   }
  }
if(quit!=2)
 quit=toquit;
}


/*********************************
PLOG
has some problems, because the log window tends to interpret the message
as tcl/tk commands
The optional tag parameter has to correspond to the log window existing tags
*********************************/

void plog( char const *cm, char const *tag )
{
char app[TCL_BUFF_STR];

#ifndef NO_WINDOW 
sprintf( app, ".log.text.text.internal insert end \"%s\" %s", cm, tag );
cmd(inter, app);
cmd(inter, ".log.text.text.internal see end");
cmd(inter, "update idletasks");

#else
printf("%s", cm);
fflush(stdout);
#endif 
message_logged=1;

}

/*********************************
INTERPINITWIN
Calls tclinit and tkinit, managing he errors
WARNING !!!
This function presumes the installation of a /gnu directory along
the model's one. Tcl and Tk initialization files MUST be in
/gnu[64]/lib/tcl8.X
/gnu[64]/lib/tk8.X

*********************************/

#ifndef NO_WINDOW 

Tcl_Interp *InterpInitWin(char *tcl_dir)
{
Tcl_Interp *app;
char *s;
FILE *f;

app=Tcl_CreateInterp();

if(Tcl_Init(app)!=TCL_OK)
   {f=fopen("tk_err.err","wt");  // use text mode for Windows better compatibility
    fprintf(f,"Tcl/Tk initialization directories not found. Check the installation of Tcl/Tk.\n%s", Tcl_GetStringResult(app));
    fclose(f);
    myexit(12);
   }
   
if(Tk_Init(app)!=TCL_OK)
   {f=fopen("tk_err.err","wt");  // use text mode for Windows better compatibility
    fprintf(f,"Tcl/Tk initialization directories not found. Check the installation of Tcl/Tk.\n%s", Tcl_GetStringResult(app));
    fclose(f);
    myexit(13);
   }

return app;
}
#endif

void reset_end(object *r)
{
object *cur;
variable *cv;
bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
  { if(cv->save)
      {
       cv->end=t-1;
      } 
   if(cv->savei==1)
       save_single(cv);

  } 

for(cb=r->b; cb!=NULL; cb=cb->next)
 {cur=cb->head;
 if(cur->to_compute==1)   
   {
   for(; cur!=NULL;cur=go_brother(cur) )
     reset_end(cur);
   }  
 }
}


/*********************************
CREATE_LOG_WINDOW
*********************************/

#ifndef NO_WINDOW
void create_logwindow(void)
{
cmd(inter, "toplevel .log");
cmd(inter, "wm title .log \"Lsd Log\"");
cmd( inter, "wm protocol .log WM_DELETE_WINDOW { if { [ discard_change ] == \"ok\" } { exit } { } }" ); 
cmd( inter, "wm group .log ." );
// change window icon
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap .log @$RootLsd/$LsdSrc/icons/lsd.xbm} {}");

cmd(inter, "set w .log.text");
cmd(inter, "frame $w");
cmd(inter, "scrollbar $w.scroll -command \"$w.text yview\"");
cmd(inter, "scrollbar $w.scrollx -command \"$w.text xview\" -orient hor");
cmd(inter, "text $w.text -relief sunken -yscrollcommand \"$w.scroll set\" -xscrollcommand \"$w.scrollx set\" -wrap none");
sprintf( msg, "$w.text configure -tabs {%s}", tabs );
cmd( inter, msg );

// Log window tags
cmd(inter, "$w.text tag configure highlight -foreground red");
cmd(inter, "$w.text tag configure tabel");
cmd(inter, "$w.text tag configure series -tabs {2c 5c 8c}");

cmd(inter, "pack $w.scroll -side right -fill y");
cmd(inter, "pack $w.text -expand yes -fill both");
cmd(inter, "pack $w.scrollx -side bottom -fill x");
cmd(inter, "pack $w -expand yes -fill both");
cmd(inter, "bind .log <KeyPress-s> {.log.but.stop invoke}; bind .log <KeyPress-S> {.log.but.stop invoke}");
cmd(inter, "bind .log <KeyPress-p> {.log.but.pause invoke}; bind .log <KeyPress-P> {.log.but.pause invoke}");
cmd(inter, "bind .log <KeyPress-f> {.log.but.speed invoke}; bind .log <KeyPress-F> {.log.but.speed invoke}");
cmd(inter, "bind .log <KeyPress-o> {.log.but.obs invoke}; bind .log <KeyPress-O> {.log.but.obs invoke}");
cmd(inter, "bind .log <KeyPress-d> {.log.but.deb invoke}; bind .log <KeyPress-D> {.log.but.deb invoke}");
cmd(inter, "bind .log <KeyPress-h> {.log.but.help invoke}; bind .log <KeyPress-H> {.log.but.help invoke}");
cmd(inter, "bind .log <KeyPress-c> {.log.but.copy invoke}; bind .log <KeyPress-C> {.log.but.copy invoke}");
cmd(inter, "bind .log <Control-c> {.log.but.copy invoke}; bind .log <Control-C> {.log.but.copy invoke}");
cmd(inter, "bind .log <KeyPress-Escape> {focus -force .}");

cmd(inter, "set w .log.but");
cmd(inter, "frame $w");
cmd(inter, "button $w.stop -width -9 -text Stop -command {set done_in 1} -underline 0");
cmd(inter, "button $w.pause -width -9 -text Pause -command {set done_in 9} -underline 0");
cmd(inter, "button $w.speed -width -9 -text Fast -command {set done_in 2} -underline 0");
cmd(inter, "button $w.obs -width -9 -text Observe -command {set done_in 4} -underline 0");
cmd(inter, "button $w.deb -width -9 -text Debug -command {set done_in 3} -underline 0");
cmd(inter, "button $w.help -width -9 -text Help -command {LsdHelp Log.html} -underline 0");
cmd(inter, "button $w.copy -width -9 -text Copy -command {tk_textCopy .log.text.text} -underline 0");

cmd(inter, "pack $w.stop $w.pause $w.speed $w.obs $w.deb $w.copy $w.help -padx 10 -pady 10 -side left");
cmd(inter, "pack $w -side right");

cmd(inter, "update idletasks");
cmd(inter, "set posXLog [expr [winfo screenwidth .log] - $posX - [winfo reqwidth .log]]");
cmd(inter, "set posYLog [expr [winfo screenheight .log] - 4 * $posY - [winfo reqheight .log]]");
cmd(inter, "wm geometry .log +$posXLog+$posYLog");	
cmd(inter, "wm minsize .log [ winfo width .log ] [ winfo height .log ]");	

// replace text widget default insert, delete and replace bindings, preventing the user to change it
cmd( inter, "rename .log.text.text .log.text.text.internal" );
cmd( inter, "proc .log.text.text { args } { switch -exact -- [lindex $args 0] { insert { } delete { } replace { } default { return [ eval .log.text.text.internal $args] } } }" );

// a Tcl/Tk version of plog
cmd( inter, "proc plog cm { .log.text.text.internal insert end $cm }" );
}


/*********************************
COVER_BROWSER
*********************************/
bool brCovered = false;

void cover_browser( const char *text1, const char *text2, const char *text3 )
{
	if ( brCovered )		// ignore if already covered
		return;
		
	cmd(inter, "if [ winfo exists .model_str ] { wm withdraw .model_str }");
	cmd( inter, "disable_window \"\" m bbar l" );		// disable main window
	cmd( inter, "set origMainTit [ wm title . ]; wm title . \"$origMainTit (DISABLED)\"" );
	cmd( inter, "newtop .t [ wm title . ]" );
	cmd( inter, "if { $tcl_platform(platform) != \"windows\" } { wm iconbitmap .t @$RootLsd/$LsdSrc/icons/lmm.xbm } {}" );
	sprintf( msg, "label .t.l1 -font {-weight bold} -text \"%s\"", text1 );
	cmd( inter, msg );
	sprintf( msg, "label .t.l2 -text \"\n%s\"", text2 );
	cmd( inter, msg );
	cmd( inter, "label .t.l3 -fg red -text \"\nInteraction with the Lsd Browser is now disabled\"" );
	sprintf( msg, "label .t.l4 -justify left -text \"\n%s\"", text3 );
	cmd( inter, msg );
	cmd( inter, "pack .t.l1 .t.l2 .t.l3 .t.l4 -expand yes -fill y" );
	cmd( inter, "showtop .t coverW no no no" );
	cmd( inter, "update" );
	
	brCovered = true;
}


/*********************************
UNCOVER_BROWSER
*********************************/

void uncover_browser( void )
{
	if ( ! brCovered )		// ignore if not covered
		return;

	cmd( inter, "if [ winfo exist .t ] { destroytop .t }"  );
	cmd( inter, "wm title . $origMainTit" );
	cmd( inter, "enable_window \"\" m bbar l" );	// enable main window
	cmd( inter, "if { [ string equal [ wm state . ] normal ] && [ winfo exist .model_str ] && ! [ string equal [ wm state .model_str ] normal ] } { wm deiconify .model_str; lower .model_str }" );
	cmd( inter, "update" );
	
	brCovered = false;
}
#endif


/*********************************
RESULTS_ALT_PATH
*********************************/
//Simple tool to allow changing where results are saved.
void results_alt_path(const char * altPath_)
{
	if ( save_alt_path )
		delete [ ] alt_path;

	if ( strlen( altPath_ ) == 0 )
	{
		save_alt_path = false;
		return;
	}
	  
	alt_path = new char[ strlen( altPath_ ) + 1 ];
	if ( sprintf( alt_path, "%s", altPath_ ) > 0 )
		save_alt_path = true;
	else
		delete [ ] alt_path;
}


/*********************************
SAVE_SINGLE
*********************************/

void save_single(variable *vcv)
{
FILE *f;
int i;

set_lab_tit(vcv);
sprintf(msg, "%s_%s-%d_%d_seed-%d.res", vcv->label, vcv->lab_tit, vcv->start,vcv->end,seed-1);
f=fopen(msg, "wt");  // use text mode for Windows better compatibility

fprintf(f, "%s %s (%d %d)\t\n",vcv->label, vcv->lab_tit, vcv->start, vcv->end);

for(i=0; i<=t-1; i++)
 {
  if(i>=vcv->start && i <=vcv->end && !isnan(vcv->data[i]))		// save NaN as n/a
    fprintf(f,"%lf\t\n",vcv->data[i]);
  else
    fprintf(f,"%s\t\n", nonavail);
  }
  
fclose(f); 
}


/*********************************
CLEAN_FILE
*********************************/
// remove any path prefixes to filename, if present
char *clean_file(char *filename)
{
	if(strchr(filename, '/') != NULL)
		return strrchr(filename, '/') + 1;
	if(strchr(filename, '\\') != NULL)
		return strrchr(filename, '\\') + 1;
	return filename;
}


/*********************************
CLEAN_PATH
*********************************/
// remove cygwin path prefix, if present, and replace \ with /
char *clean_path(char *filepath)
{
	int i, len=strlen("/cygdrive/");
	if(!strncmp(filepath, "/cygdrive/", len))
	{
		char *temp=new char[strlen(filepath) + 1];
		temp[0]=toupper(filepath[len]);				// copy drive letter
		temp[1]=':';								// insert ':'
		strcpy(temp + 2, filepath + len + 1);		// copy removing prefix
		strcpy(filepath, temp);
		delete[] temp;
	}
	
	len=strlen(filepath);
	for(i=0; i<len; i++)
		if(filepath[i]=='\\')	// replace \ with /
			filepath[i]='/';
			
	return filepath;
}


/*********************************
SIGNAL_HANDLER
*********************************/
// handle critical system signals
void signal_handler(int signum)
{
	char msg2[TCL_BUFF_STR];
	double useless = -1;
	
	switch(signum)
	{
		case SIGFPE:
			sprintf(msg, "SIGFPE (Floating-Point Exception):\n  Maybe a division by 0 or similar?");
		break;
		
		case SIGILL:
			sprintf(msg, "SIGILL (Illegal Instruction):\n  Maybe executing data?");		
		break;
		
		case SIGSEGV:
			sprintf(msg, "SIGSEGV (Segmentation Violation):\n  Maybe an invalid pointer?\nAlso ensure no group of objects has zero elements.");		
		break;
		
		default:
			sprintf(msg, "Unknown signal");			
	}
	
#ifdef NO_WINDOW
	sprintf(msg2, "FATAL ERROR: System Signal received:\n %s\nLsd is aborting...", msg);
	plog(msg2);
#else
	sprintf(msg2, "tk_messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s\n\nAdditional information can be obtained running the simulation using the 'Model'/'GDB Debugger' menu option.\n\nAttempting to open the Lsd Debugger (Lsd will close immediately after exiting the Debugger)...\"", msg);
	cmd(inter, msg2);
	sprintf( msg2, "Error in equation for '%s'", stacklog->vs->label );
	deb( stacklog->vs->up, NULL, msg2, &useless );
#endif

	myexit(-signum);			// abort program
}
