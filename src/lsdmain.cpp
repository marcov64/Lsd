/***************************************************
****************************************************
LSD 6.4 - January 2015
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

#include "choose.h"

#ifndef NO_WINDOW
#include <tk.h>
void cmd(Tcl_Interp *inter, char  const *cc);
Tcl_Interp *InterpInitWin(char *tcl_dir);

Tcl_Interp *inter;
#endif

#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "decl.h"
#include <time.h>
#include <signal.h>

int t;
int quit=0;
int done_in;
int debug_flag;
int when_debug;
int stackinfo_flag=0;
int optimized=0;
int check_optimized=0;
int add_to_tot;
int plot_flag=1;
double refresh=1.01;
char *eq_file=NULL;
char lsd_eq_file[200000];
int lattice_type=2;
int watch;
char msg[1000];

int stack;
int identity=0;
int sim_num=1;

int seed=1;
long idum;

int max_step=100;
int cur_plt;
char ind[30];
variable *cemetery=NULL;

int total_var=0;
int total_obj=0;
int choice;
int choice_g;

extern double ymin;
extern double ymax;
extern long nodesSerial;
extern char nonavail[]="NA";	// string for unavailable values (use R default)

int errormsg(char const *lpszText,  char const *lpszTitle);
object *create( object *r);
void run(object *r);
void print_title(object *root);
object *skip_next_obj(object *t, int *count);
object *skip_next_obj(object *t);
object *go_brother(object *c);

void plog(char const *m);


void file_name( char *name);
void print_title_obj( object *root, int index);
void prepare_plot(object *r, int id_sim);
void myexit(int v);
FILE *search_str(char const *name, char const *str);
void set_lab_tit(variable *var);
void reset_end(object *r);
void save_title_result(FILE *f, object *r, int header );
void save_result(FILE *f, object *r, int i);
void close_sim(void);
double ran1(long *idum);
int deb(object *r, object *c, char *lab, double *res);
void run_no_window(void);
void analysis(int *choice);
void init_random(int seed);
void add_description(char const *lab, char const *type, char const *text);
void add_description(char const *lab, char const *type, char const *text);
void empty_cemetery(void);
void control_bridge(object *r);
void save_single(variable *vcv);
void delete_mn(mnode *mn);
void scan_mn(object *c);
char *clean_file(char *);
char *clean_path(char *);

char *upload_eqfile(void);
lsdstack *stacklog;

object *root;
object *blueprint;

description *descr=NULL;

char *simul_name;
char *struct_file;
char *equation_name;
char *exec_file;		// name of executable file
char *exec_path;		// path of executable file
char name_rep[400];

char *path;

char **tp;
variable **list;
object **mylist=NULL;
int llist;
int struct_loaded=0;
int posiziona;
int running;
int actual_steps=0;
int counter;
int no_window=0;
int no_res=0;
int message_logged=0;
int no_more_memory=0;
int series_saved;
int findex, fend;
int batch_sequential=0;
int fast=0;		// make fast persistent across runs
#ifdef DUAL_MONITOR
// Main window constraints
char hsize[]="400";			// horizontal size in pixels
char vsize[]="620";			// vertical minimum size in pixels
char hmargin[]="20";		// horizontal right margin from the screen borders
char vmargin[]="20";		// vertical margins from the screen borders
#endif

/*********************************
LSD MAIN
*********************************/
int lsdmain(int argn, char **argv)
{
char tcl_dir[100], str[500], *lsdroot;

int i, p=0, len, done;
FILE *f;

blueprint=NULL;
simul_name=new char[30];
path=new char[300];
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
  printf("\nLsd NoWindow version. This is the no window version of Lsd. Specify '-f filename.lsd' to run a single configuration file, or '-f simul_name -s 1' for batch sequential simulation mode (requires configuration files: simul_name_1.lsd, simul_name_2.lsd, etc).\n");
  exit(0);
 }
else
 {
 for(i=1; i<argn; i+=2)
 {
 done=0;
 if(argv[i][0]=='-' && argv[i][1]=='f' )
  {
   done=1;
   delete[] simul_name;
   simul_name=new char[strlen(argv[1+i])+1];
   strcpy(simul_name,argv[1+i]);
  }
 if(argv[i][0]=='-' && argv[i][1]=='s' )
  {done=1;
 	 findex=atoi(argv[i+1]);
  batch_sequential=1;   
  }
 if(argv[i][0]=='-' && argv[i][1]=='e' )	// read -e parameter : last sequential file to process
  {done=1;
 	 fend=atoi(argv[i+1]);
  }

  
  if(done==0)
   {  printf("\nOption '%s' not recognized. Lsd NoWindow version. This is the no window version of Lsd. Specify '-f filename.lsd' to run a single configuration file, or '-f simul_name -s 1' for batch sequential simulation mode (requires configuration files: simul_name_1.lsd, simul_name_2.lsd, etc).\n", argv[1]);
    exit(0);
   }
  }
 } 
if(batch_sequential==0)
 {
 struct_file=new char[strlen(simul_name)+1];
 sprintf(struct_file, "%s", simul_name);
 simul_name[strlen(simul_name)-4]='\0';
  } 
#else 
for(i=1; argv[i]!=NULL; i++)
{if(argv[i][0]!='-' || (argv[i][1]!='f' && argv[i][1]!='i') )
  {sprintf(msg,"Illegal option: %s\n\nAvailable options :\n-itcl_directory\n-fmodel_name\n\nContinue anyway?", argv[i]);
	errormsg(msg, "Lsd Warning");
  }
 if(argv[i][1]=='f')
	{delete[] simul_name;
	 simul_name=new char[strlen(argv[i+1])+3];
	 strcpy(simul_name,argv[i+1]);
    len=strlen(simul_name);
    if(len>4 && !strcmp(".lsd",simul_name+len-4) )
     *(simul_name+len-4)=(char)NULL;
    i++;
    p=1;
	}
 if(argv[i][1]=='i')
	{
   strcpy(tcl_dir,argv[i+1]+2);
   i++;
  } 

}

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
blueprint=new object;
blueprint->init(NULL, "Root");


#ifdef NO_WINDOW
no_window=1;

if(batch_sequential==0)
  f=fopen(struct_file, "r");
else
 {
  sprintf(msg, "%s_%d.lsd", simul_name,findex);
  struct_file=new char[strlen(msg)+1];
  strcpy(struct_file,msg);
  f=fopen(struct_file, "r");
 }
if(f==NULL)
 {
  printf("\nFile %s not found. This is the no window version of Lsd. Specify a -f filename.lsd to run a simulation or -f simul_name -s 1 for batch sequential simulation mode (requires configuration files: simul_name_1.lsd, simul_name_2.lsd, etc).\n",struct_file);
  exit(0);
 }
fclose(f);
#else 
Tcl_FindExecutable(argv[0]); 
inter=InterpInitWin(tcl_dir);
eq_file=upload_eqfile();
lsd_eq_file[0]=(char)NULL;
/* this does not work with cygwin
cmd(inter, "pwd");
sprintf(msg, "%s",*argv);
if(!strncmp(msg, "//", 2))
 sprintf(str, "%s",msg+3);
else
 sprintf(str, "%s",msg);



sprintf(msg, "if {[file exists [file dirname \"%s\"]]==1} {cd [file dirname \"%s\"]} {cd [pwd]}",str, str);
cmd(inter, msg);

cmd(inter, "pwd");
*/
// use the new exec_path variable to change to the model directory
sprintf(msg, "cd \"%s\"", exec_path);
cmd(inter, msg);

if(inter==NULL)
 myexit(1);

Tcl_LinkVar(inter, "choice", (char *) &choice, TCL_LINK_INT);
Tcl_LinkVar(inter, "debug_flag", (char *) &debug_flag, TCL_LINK_INT);
//Tcl_LinkVar(inter, "stack_flag", (char *) &stackinfo_flag, TCL_LINK_INT);
Tcl_LinkVar(inter, "when_debug", (char *) &when_debug, TCL_LINK_INT);

cmd(inter, "if { [string first \" \" \"[pwd]\" ] >= 0  } {set debug_flag 1} {set debug_flag 0}");
if(debug_flag==1)
 {
 cmd(inter, "tk_messageBox -icon error -type ok -message \"The directory containing the model is:\n[pwd]\n It appears to include spaces. This will make impossible to compile and run Lsd model. The Lsd directory must be located where there are no spaces in the full path name.\nMove all the Lsd directory and delete the 'system_options.txt' file from the \\src directory. \"");
 exit(1);
 
 }

cmd(inter, "tk appname browser");
cmd(inter, "set RootLsd [pwd]");
lsdroot=getenv("LSDROOT");
if(lsdroot==NULL)
 {
 cmd(inter, "tk_messageBox -icon error -type ok -message \"LSDROOT not set. Program aborted.\"");
 exit(1);
 }
sprintf(msg, "set RootLsd \"%s\"",lsdroot);
len=strlen(msg);
for(i=0; i<len; i++)
 if(msg[i]=='\\')
  msg[i]='/';
cmd(inter, msg);

/** WORKS
cmd(inter, "proc LsdHelp a {global tcl_platform; global RootLsd; set here [pwd]; cd $RootLsd; cd Manual; set f [open temp.html w]; puts $f \"<meta http-equiv=\\\"Refresh\\\" content=\\\"0;url=$a\\\">\"; close $f; set b \"temp.html\"; if {$tcl_platform(platform) == \"unix\"} {exec konqueror $b &} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\" || $tcl_platform(osVersion) == \"5.1\" || $tcl_platform(osVersion) == \"5.0\" } {exec cmd.exe /c start $b &} {catch [exec open.bat &] }} {exec command.com /c start $b &}}; cd $here }");

********/

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
  cmd(inter, "tk_messageBox -title Warning -type ok -message \"The system could not locate the LMM system options.\\nIt may be impossible to open help files and compare the equation files. Any other functionality will work normally. When possible set in LMM the system options in menu File.\"");
 }

Tcl_UnlinkVar(inter, "done");
//cmd(inter, "proc LsdHelp a {global HtmlBrowser; global tcl_platform; global RootLsd; set here [pwd];  set f [open $RootLsd/Manual/temp.html w]; puts $f \"<meta http-equiv=\\\"Refresh\\\" content=\\\"0;url=$a\\\">\"; close $f; set b \"[file nativename $RootLsd/Manual/temp.html]\"; if {$tcl_platform(platform) == \"unix\"} {exec $HtmlBrowser $b &} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\" || $tcl_platform(osVersion) == \"5.1\" || $tcl_platform(osVersion) == \"5.0\" } {exec cmd.exe /c start $b &} {catch [exec open.bat &] }} {exec start $b &}} }");

cmd(inter, "proc LsdHelp a {global HtmlBrowser; global tcl_platform; global RootLsd; set here [pwd];  set f [open $RootLsd/Manual/temp.html w]; puts $f \"<meta http-equiv=\\\"Refresh\\\" content=\\\"0;url=$a\\\">\"; close $f; set b \"[file nativename $RootLsd/Manual/temp.html]\"; if {$tcl_platform(platform) == \"unix\"} {exec $HtmlBrowser $b &} {exec cmd.exe /c start $b &}}");


//cmd(inter, "proc LsdHtml a {global HtmlBrowser; global tcl_platform;  set f [open temp.html w]; puts $f \"<meta http-equiv=\\\"Refresh\\\" content=\\\"0;url=$a\\\">\"; close $f; set b \"temp.html\"; if {$tcl_platform(platform) == \"unix\"} {exec $HtmlBrowser $b &} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\" || $tcl_platform(osVersion) == \"5.1\" || $tcl_platform(osVersion) == \"5.0\"  } {exec cmd.exe /c start $b &} {catch [exec open.bat &] }} {exec start $b &}}}");

cmd(inter, "proc LsdHtml a {global HtmlBrowser; global tcl_platform;  set f [open temp.html w]; puts $f \"<meta http-equiv=\\\"Refresh\\\" content=\\\"0;url=$a\\\">\"; close $f; set b \"temp.html\"; if {$tcl_platform(platform) == \"unix\"} {exec $HtmlBrowser $b &} {exec cmd.exe /c start $b &}}");


cmd(inter, "proc LsdTkDiff {a b} {global tcl_platform; global RootLsd; global wish; global LsdSrc; if {$tcl_platform(platform) == \"unix\"} {exec $wish $RootLsd/$LsdSrc/tkdiffb.tcl $a $b &} {if {$tcl_platform(os) == \"Windows NT\"} {if {$tcl_platform(osVersion) == \"4.0\" } {exec cmd /c start $wish $RootLsd/$LsdSrc/tkdiffb.tcl $a $b &} {exec $wish $RootLsd/$LsdSrc/tkdiffb.tcl $a $b &} } {exec start $wish $RootLsd/$LsdSrc/tkdiffb.tcl $a $b &}}}");

// set window icon
cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {wm iconbitmap . -default $RootLsd/$LsdSrc/lsd.ico} {wm iconbitmap . @$RootLsd/$LsdSrc/lsd.xbm}");
// set position parameters now
#ifdef DUAL_MONITOR
Tcl_SetVar(inter, "widthB", hsize, 0);		// horizontal size in pixels
Tcl_SetVar(inter, "heightB", vsize, 0);		// vertical minimum size in pixels
Tcl_SetVar(inter, "posX", hmargin, 0);		// horizontal right margin from the screen borders
Tcl_SetVar(inter, "posY", vmargin, 0);		// vertical margins from the screen borders
cmd(inter, "wm geometry . \"[expr $widthB]x$heightB+$posX+$posY\"");
#endif

cmd(inter, "label .l -text \"Starting Lsd\"");
cmd(inter, "pack .l");
cmd(inter, "wm protocol . WM_DELETE_WINDOW {set message [tk_messageBox -title \"Exit?\" -type yesno -message \"Do you really want to kill Lsd?\"]; if {$message==\"yes\"} {exit} {}}"); 
cmd(inter, "update");

add_description("Root", "Object", "(no description available)");
sprintf(name_rep, "modelreport.html");

f=fopen("makefile", "r");
if(f!=NULL)
 {
 fscanf(f, "%s", msg);
 while(strncmp(msg, "FUN=", 4) && f!=(FILE *)EOF)
    fscanf(f, "%s", msg);
 fclose(f);
 sprintf(equation_name, "%s.cpp",msg+4);
 }



cmd(inter, "destroy .l");
#endif

if(p==1)
{
f=fopen(struct_file, "r");
if(f!=NULL)
{struct_loaded;
 root->load_struct(f);
 fclose(f);
 f=NULL;
 root->load_param(struct_file, 1, f);
 f=search_str(struct_file, "SIM_NUM");
 if(f!=NULL)
  fscanf(f, "%d", &sim_num);
 fclose(f);
 f=search_str(struct_file, "SEED");
 if(f!=NULL)
  fscanf(f, "%ld", &idum);
 fclose(f);
 f=search_str(struct_file, "MAX_STEP");
 if(f!=NULL)
   fscanf(f, "%d", &max_step);
 fclose(f);
 f=search_str(struct_file, "EQUATION");
 if(f!=NULL)
   fgets(msg, 200, f);
 delete[] equation_name;
 equation_name=new char[strlen(msg)+1];
 strcpy(equation_name, msg+1);
 if(equation_name[strlen(equation_name)-1]=='\n')
   equation_name[strlen(equation_name)-1]=(char)NULL;
 if(equation_name[strlen(equation_name)-1]=='\n')
   equation_name[strlen(equation_name)-1]=(char)NULL;
  

 fclose(f);
}
}

stacklog = new lsdstack;
stacklog->next=NULL;
stacklog->prev=NULL;
stacklog->ns=0;
stacklog->vs=NULL;
strcpy(stacklog->label, "Lsd Simulation Manager");

#ifndef NO_WINDOW
#ifndef DUAL_MONITOR
cmd(inter, "set widthB 400");
cmd(inter, "set heightB 454");
cmd(inter, "set posX 50");
cmd(inter, "set posY 46");
#endif
cmd(inter, "source $RootLsd/$LsdSrc/align.tcl");
while(1)
{
root=create( root);
cmd(inter, "wm iconify .");

no_more_memory=0;
series_saved=0;
try {
run(root);
}

/**/
catch(int p)
 {
 cmd(inter, "puts here");
 }
/**/ 
cmd(inter, "wm deiconify .");
cmd(inter, "destroy .l");
}

#else
 run(root);
 empty_cemetery();
 root->empty();
 blueprint->empty();
#endif 

}


/*********************************
RUN
*********************************/
void run(object *root)
{
int i, j, done=0;
bool batch_sequential_loop=false; // indicates second or higher iteration of a batch
char ch[120], nf[300];
FILE *f;
double app=0;
clock_t start, end;

#ifndef NO_WINDOW
Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);
Tcl_LinkVar(inter, "done_in", (char *) &done_in, TCL_LINK_INT);
#endif

done_in=done=0;
quit=0;

 
#ifndef NO_WINDOW 
Tcl_UnlinkVar(inter, "done");
Tcl_LinkVar(inter, "posiziona", (char *) &posiziona, TCL_LINK_INT);
#else
    sprintf(msg, "\nProcessing configuration file %s ...\n",struct_file);
    plog(msg);
   	f=fopen(struct_file, "r");
    root->load_struct(f);
	  struct_loaded=1;
    fscanf(f, "%s", msg); //should be DATA
	  root->load_param(struct_file, 1,f);
    fscanf(f, "%s",msg); //should be SIM_NUM 
	  fscanf(f, "%d", &sim_num);
    fscanf(f, "%s",msg); //should be SEED
	  fscanf(f, "%d", &seed);
    fscanf(f, "%s",msg); //should be MAX_STEP
    fscanf(f, "%d", &max_step);
    fclose(f);
#endif

for(i=1; i<=sim_num && quit!=2; i++)
{

empty_cemetery(); //ensure that previous data are not erroneously mixed (sorry Nadia!)
#ifndef NO_WINDOW
prepare_plot(root, i);

if(done_in==2 && cur_plt>0)
 cmd(inter, "if {[winfo exists $activeplot]==1} {wm iconify $activeplot} {}");
#else
sprintf(msg, "\nSimulation %d running ...\n", i);
plog(msg);
#endif

// deb(root->son, NULL, "stop 1", &app);
if(i>1 || batch_sequential_loop)
{
 root->empty();
 root->init(NULL, "Root");
 blueprint->empty();
 nodesSerial=0;			// restart network nodes serial counter
// if(batch_sequential==1)
//  {
//   sprintf(msg, "%s_%d.lsd",simul_name,findex);
//   delete[] struct_file;
//   struct_file=new char[strlen(msg)+1];
//   strcpy(struct_file,msg);
//  } 
 f=fopen(struct_file, "r");
 if(f==NULL)
  {
   sprintf(msg, "\nFile %s not found.\n",struct_file);
   plog(msg);
   exit(0);
  }
 root->load_struct(f);
 fscanf(f, "%s", msg); //should be DATA
 root->load_param(struct_file, 1, f);
 fclose(f);
}

strcpy(ch, "");

series_saved=0;
print_title(root);
if(no_more_memory==1)
 {
#ifndef NO_WINDOW 
 root->empty();
 root->init(NULL, "Root");
 blueprint->empty();
 f=fopen(struct_file, "r");
 root->load_struct(f);
 fclose(f);
 f=NULL;
 root->load_param(struct_file, 1, f);
  sprintf(msg, "tk_messageBox -type ok -icon warning -title \"Not enough memory\" -message \"Too many series saved for the memory available.\nMemory sufficient for %d series over %d time steps.\nReduce series to save and/or time steps.\"", series_saved, max_step);
 cmd(inter, msg);
#else
 printf("Not enough memory. Too many series saved for the memory available.\nMemory sufficient for %d series over %d time steps.\nReduce series to save and/or time steps.", series_saved, max_step);
 exit(0);
 #endif
 return;
 }
 
// deb(root->son, NULL, "stop 3", &app);
//new random routine' initialization
init_random(seed);

seed++;
stack=0;

#ifndef NO_WINDOW 
cmd(inter, "bind .log <Destroy> {set done_in 35}");
cmd(inter, "if {[winfo exists $c]==1} {bind $c <Destroy> {set done_in 35}} {}");
#endif
posiziona=0;
running=1;
actual_steps=0;
start = clock();

for(t=1; quit==0 && t<=max_step;t++ )
 {
// control_bridge(root);
  if(when_debug==t)
    debug_flag=1;
  cur_plt=0;
  root->update();

#ifndef NO_WINDOW 
switch(done_in)
{
case 0:

 if(fast==0)
  {
  if(cur_plt==0)
   {sprintf(msg,"\nSim. %d step %d done",i,t);
	 plog(msg);
   }
  else
   {switch(posiziona)
      {
      
      case 2: 
        sprintf(msg,"if { [winfo exist .plt%d]} {$activeplot.c.c.cn xview scroll 1 units} {}",i);
        cmd(inter, msg);
       
      break;
       
      case 1:  
       
       
       sprintf(msg, "set newpos [expr %lf - [expr 250 / %lf]]", (double)t/(double)max_step, (double)max_step);
       cmd(inter, msg);
       sprintf(msg,"if { [winfo exist .plt%d]} {$activeplot.c.c.cn xview moveto $newpos} {}", i);
       //sprintf(msg,"$activeplot.c.c.cn xview moveto %lf", (double)t/(double)max_step);
       cmd(inter, msg);
       cmd(inter, "set posiziona $oldposiziona");
       break;
      default:
      break; 
      }
   }
   }
break;

case 2:
 fast=1;
 debug_flag=0;
 cmd(inter, "set a [split [winfo children .] ]");
 cmd(inter, " foreach i $a {if [string match .plt* $i] {wm iconify $i}}");
// sprintf(msg, "if { [winfo exist .plt%d]} {wm iconify .plt%d} {}", i, i);
// cmd(inter, msg);
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.go conf -state disabled} {}",i, i);
 cmd(inter, msg);
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.shift conf -state disabled} {}", i, i);
 cmd(inter, msg);
 
 done_in=0;
 break;

case 4:
 fast=0;
 cmd(inter, "set a [split [winfo children .] ]");
 cmd(inter, " foreach i $a {if [string match .plt* $i] {wm deiconify $i}}");
// sprintf(msg, "if { [winfo exist .plt%d]} {wm deiconify .plt%d} {}", i, i);
// cmd(inter, msg);
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.go conf -state normal} {}",i, i);
 cmd(inter, msg);
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.shift conf -state normal} {}",i, i);
 cmd(inter, msg);

 done_in=0;
 break;
 
case 1:
  sprintf(msg, "\nSimulation stopped at t = %d", t);
  plog(msg);
  quit=2;
  done_in=0;
break;

case 3:
debug_flag=1;
done_in=0;
break;

case 35:
myexit(0);
break;

case 6:

Tcl_LinkVar(inter, "app_refresh", (char *) &app, TCL_LINK_DOUBLE);

cmd(inter, "set app_refresh $refresh");
Tcl_UnlinkVar(inter, "done");
if(app<2 && app > 1)
 refresh=app;
done_in=0;
break; 

// plot window DELETE_WINDOW button handler
case 5:
 cmd(inter, " if { [winfo exists .plt%d] == 1} {wm deiconify .plt%d} {}");
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.go conf -state disabled} {}",i, i);
 cmd(inter, msg);
 sprintf(msg, "if { [winfo exist .plt%d]} {.plt%d.c.yscale.shift conf -state disabled} {}", i, i);
 cmd(inter, msg);
 sprintf(msg, "if { [winfo exist .plt%d]} {destroy .plt%d} {}", i, i);
 cmd(inter, msg);
 sprintf(msg, "\nSimulation stopped at t = %d", t);
 plog(msg);
 quit=2;
 done_in=0;
break;
}

cmd(inter, "update");
#endif
 }//end of for t


actual_steps=t-1;

running=0;
if(quit==1) //For multiple simulation runs you need to reset quit
 quit=0;
/**************** WORKS ONLY FOR WINDOWS VERSION */
end=clock();
if(strlen(path)>0 || no_window==1)
  sprintf(msg, "\nSimulation %d finished (%2g sec.)",i,(float)(( end - start) /(float)CLOCKS_PER_SEC), simul_name, seed-1);
else
  sprintf(msg, "\nSimulation %d finished (%2g sec.)",i,(float)( end - start) /CLOCKS_PER_SEC, simul_name, seed-1);
 plog(msg);
/****************************************************/
//sprintf(msg, "\nSimulation %d finished\n",i);
//plog(msg);

#ifndef NO_WINDOW 
cmd(inter, "update");
// allow for run time plot window destruction
cmd(inter, "if {[info exists activeplot] == 1} {if {[winfo exists $activeplot] == 1} {wm protocol $activeplot WM_DELETE_WINDOW \"\"}}");
#endif

close_sim();
plog("\nResetting Variables with the end value. Wait... ");


#ifndef NO_WINDOW 
cmd(inter, "update");
#endif
//if(t<max_step+1) //cannot work, if users used the debug resetting the end.
reset_end(root);
scan_mn(root);
plog("Done.\n");

if(sim_num>1 || no_window==1) //Save results for multiple simulation runs
{

if(no_res==0)
{
if(batch_sequential==0)
 sprintf(msg, "\nSaving results in file %s_%d.res",simul_name, seed-1);
else
 sprintf(msg, "\nSaving results in file %s_%d_%d.res",simul_name, findex, seed-1);

plog(msg);
#ifndef NO_WINDOW 
cmd(inter, "update");
#endif

if(batch_sequential==0)
 sprintf(msg, "%s_%d.res", simul_name, seed-1);
else
 sprintf(msg, "%s_%d_%d.res", simul_name, findex, seed-1);

f=fopen(msg,"wt");  // use text mode for Windows better compatibility
save_title_result(f, root, 1);
fprintf(f, "\n");
for(j=0; j<=actual_steps; j++)
  {save_result(f,root, j);
   fprintf(f, "\n");
  }
fclose(f);

if(batch_sequential==0)
 sprintf(msg, "\nResults saved in file %s_%d.res",simul_name, seed-1);
else
 sprintf(msg, "\nResults saved in file %s_%d_%d.res",simul_name, findex, seed-1);

plog(msg);
}
#ifndef NO_WINDOW 
cmd(inter, "update");
#else
printf("\n");
#endif

if(batch_sequential==0)
 sprintf(msg, "%s_%d_%d.tot", simul_name, seed-i, seed-1+sim_num-i);
else
 sprintf(msg, "%s_%d_%d_%d.tot", simul_name, findex, seed-i, seed-1+sim_num-i);

if(i==1 && add_to_tot==0)
 {
 f=fopen(msg,"wt");  // use text mode for Windows better compatibility
 save_title_result(f, root, 0);
 fprintf(f, "\n");
 }
else
 f=fopen(msg,"a");
save_result(f,root, actual_steps);
fprintf(f, "\n");
fclose(f);
//if(batch_sequential==1)
// findex+=1;
 if(batch_sequential==1 && i==sim_num)  // last run of current batch file?
 {
   findex++;							// try next file
   sprintf(msg, "%s_%d.lsd",simul_name,findex);
   delete[] struct_file;
   struct_file=new char[strlen(msg)+1];
   strcpy(struct_file,msg);
   f=fopen(struct_file, "r");			
   if(f==NULL || (fend!=0 && findex>fend))  // no more file to process
   {
	 if(f!=NULL) fclose(f);
     sprintf(msg, "\nFinished processing %s.\n",simul_name);
     plog(msg);
     break;
   }
   sprintf(msg, "\n\nProcessing configuration file %s ...\n",struct_file);
   plog(msg);
   fclose(f);  // process next file
   i=0;   // force restarting run count
   batch_sequential_loop=true;
  } 
}
}
/*
if(strlen(path)>0)
  sprintf(msg, "%s/%s_.lsd", path, simul_name);
else
  sprintf(msg, "%s_.lsd", simul_name);
f=fopen(msg, "w");
strcpy(msg, "");

root->save_struct(f,msg);
fprintf(f, "\nDATA\n");
root->save_param(f);
fprintf(f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d\nEQUATION %s\n", sim_num, seed, max_step, equation_name);
fclose(f);

root->empty();
root->init(NULL, "Root");
if(strlen(path)>0)
 sprintf(msg, "%s/%s.lsd",path, simul_name);
else
 sprintf(msg, "%s.lsd",simul_name);

f=fopen(msg, "r");
root->load_struct(f);
fclose(f);
root->load_param(msg, 1);
*/

#ifndef NO_WINDOW 
Tcl_UnlinkVar(inter, "done_in");
Tcl_UnlinkVar(inter, "posiziona");
#endif
quit=0;
}


/*********************************
PRINT_TITLE
*********************************/

void print_title(object *root)
{
char ch[20];
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
    /*fprintf(f,"%s%s\t", var->label,tag);
     sprintf(msg,"%s%s", var->label,tag); */
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
	  {sprintf(msg,"\nData for %s in Object %s not loaded\n", var->label, root->label);
		plog(msg);
		plog("Use the Data Editor to set its values\n");
     if(var->param==1)
       sprintf(msg, "tk_messageBox -type ok -icon warning -title \"Data not loaded\" -message \"The simulation cannot start because parameter:\n'%s' (Object '%s')\nhas not been initialized.\n\nMove the browser to show object '%s' and choose menu Data-Init.Values.\"", var->label, root->label, root->label);
     else
       sprintf(msg, "tk_messageBox -type ok -icon warning -title \"Data not loaded\" -message \"The simulation cannot start because a lagged value for variable:\n'%s' (Object '%s')\nhas not been initialized.\n\nMove the browser to show object '%s' and choose menu Data-Init.Values.\"", var->label, root->label, root->label);
     #ifndef NO_WINDOW   
     cmd(inter, msg);  
     #endif
		toquit=2;
	  }
 }


//for(c=root->son, counter=1; c!=NULL; c=skip_next_obj(c, &num), counter=1)
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
PRINT_TITLE_OBJ
Esperimental routine, not used in this version. Prints the results
in different files, one for each instance of an object
*********************************/

void print_title_obj( object *root, int index)
{
variable *cv;
FILE *f;
char ch[80];
int num;
object *cur;
bridge *cb;

//for(cur=root->son; cur!=NULL; cur=skip_next_obj(cur, &num))
for(cb=root->b; cb!=NULL; cb=cb->next)
{strcpy(ch,"");
 cur=cb->head;
 for(cv=cur->v; cv!=NULL; cv=cv->next)
 {if(cv->save==1)
   {sprintf(ch,"%s_%d.res", cur->label, index);
	 break;
	}
 }
 if(strlen(ch)!=0)
 {

 sprintf(ch, "%s_1.res",root->label);
 f=fopen(ch, "wt");  // use text mode for Windows better compatibility

 for(cv=cur->v; cv!=NULL; cv=cv->next)
  {if(cv->save==1)
   fprintf(f,"%s\t",cv->label);
  }
 fprintf(f,"\n");
 fclose(f);
 }
 print_title_obj(cur, index);
}
}



/*********************************
PLOG
has some problems, because the log window tends to interpret the message
as tcl/tk commands
*********************************/

void plog(char const *cm)
{
char app[1000];

#ifndef NO_WINDOW 
sprintf(app, ".log.text.text insert end \"%s\"", cm);

cmd(inter, app);
cmd(inter, ".log.text.text see end");
//cmd(inter, "raise .log");
//cmd(inter, "update");
#else
 printf("\n%s", cm);
#endif 
message_logged=1;

}



/*********************************
INTERPINITWIN
Calls tclinit and tkinit, managing he errors
WARNING !!!
This function presumes the installation of a /gnu directory along
the model's one. Tcl and Tk initialization files MUST be in
/gnu/share/tcl8.0
/gnu/share/tk8.0

!!!!!!!!!!!!!!!!!! NEWER: UPDATE TO 8.3 !!!!!!!!!!!!!!!!!!!!!!
/gnu/lib/tcl8.3
/gnu/lib/tk8.3

*********************************/


#ifndef NO_WINDOW 

Tcl_Interp *InterpInitWin(char *tcl_dir)
{
Tcl_Interp *app;
char *s, *lsdroot;
FILE *f;

app=Tcl_CreateInterp();

if(Tcl_Init(app)!=TCL_OK)
 {
  lsdroot=getenv("LSDROOT");
  if(lsdroot==NULL)
   {f=fopen("tk_err.err","wt");  // use text mode for Windows better compatibility
    fprintf(f,"Environmental variable 'LSDROOT' is not set, probably because you are using Win95/98/ME and no environmental space is available.\nTo fix the problem see in the LMM manual in the section concerning the compilation errors.");
    fclose(f);
    cmd(app, "set a [pwd]");
    exit(1);
   }
   
  sprintf(msg, "set tcl_library {%s/gnu/lib/tcl8.3}", lsdroot);
  cmd(app, msg);
  sprintf(msg, "set tk_library {%s/gnu/lib/tk8.3}", lsdroot);
  cmd(app, msg);
  if(Tcl_Init(app)!=TCL_OK)
   {f=fopen("tk_err.err","wt");  // use text mode for Windows better compatibility
    fprintf(f,"Tcl/Tk initialization directories not found. Check the installation of Tcl/Tk.\n%s", Tcl_GetStringResult(app));
    fclose(f);
    exit(1);
   }



 }


if(Tk_Init(app)!=TCL_OK)
   {f=fopen("tk_err.err","wt");  // use text mode for Windows better compatibility
    fprintf(f,"Tcl/Tk initialization directories not found. Check the installation of Tcl/Tk.\n%s", Tcl_GetStringResult(app));
    fclose(f);
    exit(1);
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




void run_no_window(void)
{
int j, choice;
FILE *f;
char ch[10];

root=new object;
root->init(NULL, "Root");
f=fopen(struct_file, "r");
if(f!=NULL)
{struct_loaded;
 root->load_struct(f);
 fclose(f);
 f=NULL;
 root->load_param(struct_file, 1,f);
 f=search_str(struct_file, "SIM_NUM");
 if(f!=NULL)
  fscanf(f, "%d", &sim_num);
 fclose(f);
 f=search_str(struct_file, "SEED");
 if(f!=NULL)
  fscanf(f, "%ld", &idum);
 fclose(f);
 f=search_str(struct_file, "MAX_STEP");
 if(f!=NULL)
   fscanf(f, "%d", &max_step);
 fclose(f);
 f=search_str(struct_file, "EQUATION");
 if(f!=NULL)
   fgets(msg, 200, f);
 delete[] equation_name;
 equation_name=new char[strlen(msg)+1];
 strcpy(equation_name, msg+1);
 if(equation_name[strlen(equation_name)-1]=='\n')
   equation_name[strlen(equation_name)-1]=(char)NULL;

 fclose(f);

stacklog = new lsdstack;
stacklog->next=NULL;
stacklog->prev=NULL;
stacklog->ns=0;

idum=-seed; //new random routine' initialization
ran1(&idum);

seed++;
stack=0;
posiziona=0;
running=1;
actual_steps=0;
strcpy(ch, "");
j=0;
print_title(root);

for(t=1; quit==0 && t<=max_step;t++ )
  {
  //printf("\n%d",t);
  root->update();
  }
/*
inter=InterpInitWin(NULL);
Tcl_LinkVar(inter, "choice", (char *) &choice, TCL_LINK_INT);
cmd(inter, "set c");
analysis(choice);
*/
printf("\nFinished simulation. Saving results...\n");
sprintf(msg, "%s_%d.res", simul_name, seed-1);
f=fopen(msg,"wt");  // use text mode for Windows better compatibility
save_title_result(f, root, 1);
fprintf(f, "\n");
for(j=1; j<=actual_steps; j++)
  {save_result(f,root, j);
   fprintf(f, "\n");
  }
fclose(f);
exit(0);

}
}


#ifndef NO_WINDOW
void create_logwindow(void)
{
cmd(inter, "toplevel .log");
// change window icon
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap .log @$RootLsd/$LsdSrc/lsd.xbm} {}");
cmd(inter, "wm protocol .log WM_DELETE_WINDOW {set message [tk_messageBox -title \"Exit?\" -type yesno -icon warning -message \"Do you really want to kill Lsd?\"]; if {$message==\"yes\"} {exit} {}}"); 
cmd(inter, "set w .log.text");
cmd(inter, "frame $w");
cmd(inter, "wm title .log \"Log\"");
cmd(inter, "scrollbar $w.scroll -command \"$w.text yview\"");
cmd(inter, "scrollbar $w.scrollx -command \"$w.text xview\" -orient hor");
cmd(inter, "text $w.text -relief sunken -yscrollcommand \"$w.scroll set\" -xscrollcommand \"$w.scrollx set\" -wrap none");
//cmd(inter, "$w.text configure -tabs {3.5c 5.5c 7.5c 9.5c 11.5c}");
cmd(inter, "$w.text configure -tabs {5c 7.5c 10c 12.5c 15c 17.5c 20c}"); // fix misalignment
cmd(inter, "$w.text tag configure highlight -foreground red");
cmd(inter, "$w.text tag configure tabel");

cmd(inter, "pack $w.scroll -side right -fill y");
cmd(inter, "pack $w.text -expand yes -fill both");
cmd(inter, "pack $w.scrollx -side bottom -fill x");
cmd(inter, "pack $w -expand yes -fill both");
cmd(inter, "bind .log <KeyPress-s> {.log.but.stop invoke}");
cmd(inter, "bind .log <KeyPress-f> {.log.but.speed invoke}");
cmd(inter, "bind .log <KeyPress-o> {.log.but.obs invoke}");
cmd(inter, "bind .log <KeyPress-d> {.log.but.deb invoke}");
cmd(inter, "bind .log <KeyPress-h> {.log.but.help invoke}");
cmd(inter, "bind .log <KeyPress-Escape> {focus -force .}");

cmd(inter, "set w .log.but");
cmd(inter, "frame $w");
cmd(inter, "button $w.stop -text Stop -command {set done_in 1}");
cmd(inter, "button $w.speed -text Fast -command {set done_in 2}");
cmd(inter, "button $w.obs -text Observe -command {set done_in 4}");
cmd(inter, "button $w.deb -text Debug -command {set done_in 3}");
cmd(inter, "button $w.help -text Help -command {LsdHelp Log.html}");
cmd(inter, "button $w.copy -text Copy -command {tk_textCopy .log.text.text}");

cmd(inter, "pack $w.stop $w.speed $w.obs $w.deb $w.help $w.copy -side left");
cmd(inter, "pack $w");
#ifdef DUAL_MONITOR
// better adjusts position for X11
cmd(inter, "update idletasks");
cmd(inter, "set posXLog [expr [winfo screenwidth .log] - $posX - [winfo reqwidth .log]]");
cmd(inter, "set posYLog [expr [winfo screenheight .log] - 4 * $posY - [winfo reqheight .log]]");
cmd(inter, "wm geometry .log +$posXLog+$posYLog; update idletasks");	
#else
//cmd(inter, "set posX [winfo x .]");
cmd(inter, "set posXLog [expr $posX + 340]");
//cmd(inter, "set posY [winfo y .]");
cmd(inter, "wm geometry .log +$posXLog+$posY");
#endif

}

#endif

void control_bridge(object *r)
{
bridge *cb;
object *cur;
int i=0;

plog("\nAttempted control bridge (lsdmain.cpp). Stopped.\n");
return;

for(cb=r->b; cb!=NULL; cb=cb->next)
 {
 cur=r->search(cb->head->label);
 if(cur!=cb->head)
  {
  sprintf(msg, "\nWrong bridge:\\nObject %s\\nDesc. %s\\n",r->label, cb->head->label);
  plog(msg);
  }
 }
for(cur=r->b->head; cur!=NULL; cur=skip_next_obj(cur, &i) )
 {i=0;
 for(cb=r->b; cb!=NULL; cb=cb->next)
  if(cb->head==cur)
   i=1;
 if(i==0)
 {
  sprintf(msg, "\nError in bridges: Object %s has no bridge\\n",cur->label);
  plog(msg);
 } 
   
 }
for(cur=r->b->head; cur!=NULL; cur=cur->next )
 control_bridge(cur);
}


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

void scan_mn(object *r)
{
bridge *cb;
object *cur;
for(cb=r->b; cb!=NULL; cb=cb->next)
 {
  if(cb->mn!=NULL)
   {delete_mn(cb->mn);
    cb->mn=NULL;
   } 
  for(cur=cb->head; cur!=NULL; cur=cur->next)
    scan_mn(cur);
 }
}

// remove any path prefixes to filename, if present
char *clean_file(char *filename)
{
	if(strchr(filename, '/') != NULL)
		return strrchr(filename, '/') + 1;
	if(strchr(filename, '\\') != NULL)
		return strrchr(filename, '\\') + 1;
	return filename;
}

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

// handle critical system signals
void signal_handler(int signum)
{
	switch(signum)
	{
		case SIGFPE:
			sprintf(msg, "SIGFPE (Floating-Point Exception):\n  Maybe a division by 0 or similar?");
		break;
		
		case SIGILL:
			sprintf(msg, "SIGILL (Illegal Instruction):\n  Maybe executing data?");		
		break;
		
		case SIGSEGV:
			sprintf(msg, "SIGSEGV (Segmentation Violation):\n  Maybe an invalid pointer?");		
		break;
		
		default:
			sprintf(msg, "Unknown signal");			
	}
#ifdef NO_WINDOW
	printf("FATAL ERROR: System Signal received:\n %s\nLsd is aborting...", msg);
#else
	char msg2[1000];
	sprintf(msg2, "tk_messageBox -icon error -type ok -message \"FATAL ERROR: System Signal received:\n\n %s\n\nAdditional information can be obtained running the simulation using the 'Model'/'gdb debug' option.\n\nLsd is aborting...\"", msg);
	cmd(inter, msg2);
#endif

	exit(signum);			// abort program
}