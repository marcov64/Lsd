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


/* 
############################################################
Reached case 37
############################################################
*/

/****************************************************
ANALYSIS.CPP contains the routines to manage the data analysis module.


The functions contained here are:

- void analysis(int *choice)
Makes an initialization and then there is the main cycle in
read_data

- void read_data(int *choice);                            
Builds the managemen window, setting all the bindings, and enters in a cycle
from which can make 8 choices:

1) Plot the graph as indicated
2) Exit from data analysis
3) Brings in foreground the graph on which title it was double clicked
4) Load a new data file
5) Sort the labels of the variables
6) Copy the selection from the list of variables to the list of the variables
to plot
7) delete the variables selected in the list of the variables to plot
9) plot the cross-section graph

- void init_canvas(void);
Simply sets the colors for any integer number

- void plot(int *choice);
Plot the graph with the variable indicated in the list of the variables
to plot. If the option says to use the automatic y scaling, it makes first
a scan of the file to spot the maximum and minimum y, then it plots. Otherwise,
it uses directly the values set by the users.

- void plot_cross(void);
Plot a graph of cross section data. The variables chosen are plotted along the times
steps chosen and, on request, they are sorted (descending or ascending) one one of
time steps. To choose which time to sort on, double-click on the time step.

- void set_cs_data(int *choice);
Interface used to determine the time steps to plot in the cross-section graphs
and the possible sorting.

- void sort_cs_asc(char **s, double **v, int nv, int nt, int c);
Sort in ascending order the variables. Used in plot_cross.

- void sort_cs_desc(char **s, double **v, int nv, int nt, int c);

Sort in descending order the variables. Used in plot_cross.

Other functions used here:
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


- void plog(char *m);
print  message string m in the Log screen. It is in LSDMAIN.CPP

- void myexit(int v);
Exit function, which is customized on the operative system.

****************************************************/

//remove or comment the next line to compile without libz. It will not be possible to generate zipped result files.

#define LIBZ 
#define PI 3.141592654
#include <tk.h>
#include <unistd.h>
#include <float.h>


#include "decl.h"
#ifdef LIBZ
//#include "errno.h"
#include "zlib.h"


#endif

object *go_brother(object *c);
void cmd(Tcl_Interp *inter, char const *cc);
void plog(char const *msg);
void read_data(int *choice);
void init_canvas(void);
void plot(int *choice);
void plot_cross(int *choice);
void plot_gnu(int *choice);
void plot_cs_xy(int *choice);
void plot_phase_diagram(int *choice);
void plot_lattice(int *choice);
void histograms(int *choice);
void histograms_cs(int *choice);
void create_series(int *choice);
void create_maverag(int *choice);

void set_cs_data(int *choice);
void sort_cs_desc(char **s,char **t, double **v, int nv, int nt, int c);
void sort_cs_asc(char **s,char **t, double **v, int nv, int nt, int c);
void myexit(int v);

void save_data1(int *choice);
void save_datazip(int *choice);
void statistics(int *choice);
void frequencies(int *choice);
void statistics_cross(int *choice);
void insert_labels_mem(object *r, int *num_v, int *num_c);
void insert_store_mem(object *r, int *num_v);
void insert_data_mem(object *r, int *num_v, int *num_c);
void insert_labels_nosave(object *r,char * lab,  int *num_v);
void insert_store_nosave(object *r,char * lab,  int *num_v);
void insert_data_nosave(object *r, char * lab, int *num_v);
void set_window_size(void);
void insert_data_file(int *num_v, int *num_c);
void plog_series(int *choice);
void set_lab_tit(variable *var);

double *search_lab_tit_file(char *s,  char *t,int st, int en);
double *find_data(int id_series);
int shrink_gnufile(void);

int sort_labels_down(const void *a, const void *b);
void show_eq(char *lab, int *choice);
int cd(char *path);
void show_plot_gnu(int n, int *choice, int type);
object *skip_next_obj(object *t);

extern Tcl_Interp *inter;
extern object *root;

extern char *simul_name;
extern char name_rep[400];
extern int seed;
extern int done_in;


int num_var;
int num_c;
int min_c;
int max_c;
double miny;
double maxy;
double truemaxy;
int autom;
int autom_x;
extern int watch;
int res, dir;
int pdigits;   // precision parameter for labels in y scale
int logs;		// log scale flag for the y-axis
int data_infile=0;
int cur_plot=0;
int file_counter=0;
char filename[1000];
char **name_var;
extern char nonavail[];	// string for unavailable values
extern char msg[];

extern variable *cemetery;

extern int actual_steps;
FILE *debug;
int time_cross;
int line_point;
int grid;
int allblack;
double point_size;
int xy;

int type_graph[1000];
int graph_l[1000];
int graph_nl[1000];


struct store
{
char label[80];
int start;
int end;
char tag[20];
double *data;
int rank;
} *vs;
void sort_on_end(store *app);

/***************************************************
ANALYSIS
****************************************************/

void analysis(int *choice)
{
char *app;
cmd(inter, "if {[winfo exists $c]==1} {wm withdraw $c} {}");
cmd(inter, "wm iconify .log");

cmd(inter, "set a [info vars c0]");
cmd(inter, "if {$a==\"\" } {set choice 0} {set choice 1}");
if(*choice==0)
 init_canvas();
FILE *f;

*choice=0;
/***
if(actual_steps>0)
{
cmd(inter, "toplevel .ask");
cmd(inter, "wm transient .ask .");
sprintf(msg, "wm title .ask \"File or Simulation?\"");
cmd(inter, msg);
cmd(inter, "label .ask.l -text \"Use data from a previously saved file or from last simulation?\"");
cmd(inter, "frame .ask.b");
cmd(inter, "button .ask.b.file -text File -command {set choice 1}");
cmd(inter, "button .ask.b.sim -text Simulation -command {set choice 2}");
cmd(inter, "pack .ask.b.file .ask.b.sim -side left");
cmd(inter, "frame .ask.c");
cmd(inter, "button .ask.c.can -text Cancel -command {set choice 3}");
cmd(inter, "button .ask.c.help -text Help -command {LsdHelp menudata.html#analres}");
cmd(inter, "pack .ask.c.can .ask.c.help -side left");
cmd(inter, "pack .ask.l .ask.b .ask.c");
cmd(inter, "bind .ask <KeyPress-f> {.ask.b.file invoke}");
cmd(inter, "bind .ask <KeyPress-s> {.ask.b.sim invoke}");
cmd(inter, "bind .ask <KeyPress-Escape> {.ask.c.can invoke}");

cmd(inter, "set w .ask; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
cmd(inter, "focus .ask");
while(*choice==0)
 Tcl_DoOneEvent(0);

cmd(inter, "destroy .ask");
}
if(*choice==1 || actual_steps==0)
  {sprintf(msg, "set lab [tk_getOpenFile -initialdir [pwd] -filetypes {{{Lsd Result Files} {.res}} {{Lsd Total Files} {.tot}} {{All Files} {*}} }]");
   cmd(inter, msg);
  }

cmd(inter, "if {[string length lab] == 0} {set choice 3}");
if(*choice==1 || actual_steps==0)
 {data_infile=1;
  app=(char *)Tcl_GetVar(inter, "lab",0);
  strcpy(filename,app);
  f=fopen(filename, "r");

  if(f==NULL)
    {*choice=0;
     return;
    }
  else
    fclose(f);
  cmd(inter, "set path [file dirname $lab]");
  app=(char *)Tcl_GetVar(inter, "path",0);
  strcpy(msg,app);
  cd(msg);

 }
else
 if(*choice==2)
   data_infile=0;
 else
   data_infile=3;

*******/
data_infile=0;
if(data_infile!=3)
 *choice=0;

while(*choice==0)
{read_data(choice); //Cases and Variables

}
*choice=0;
}


/***************************************************
READ_DATA
****************************************************/
void read_data(int *choice)
{
FILE *f;
int rot, i, h, j, k, l, m, n, p, q, r;
store *app_store;
char *app, *app1, *app2, lab[60], dirname[300], str[200], str1[200], str2[200], str3[200], str4[200];
double *datum, compvalue=1;

cur_plot=0;
file_counter=0;
*choice=0;

cmd(inter, "bind . <Destroy> {set choice 35}");
cmd(inter, "bind .log <Destroy> {set choice 35}");


Tcl_LinkVar(inter, "cur_plot", (char *) &cur_plot, TCL_LINK_INT);
if(data_infile==1)
 sprintf(msg, "wm title . \"Data Analysis - File: %s\"", filename);
else
 sprintf(msg, "wm title . \"Data Analysis - Model : %s\"", simul_name);
cmd(inter, msg);

cmd(inter, "if {[info exist gpterm] == 1 } {} {set gpooptions \"set ticslevel 0.0\"; set gpdgrid3d \"60,60,3\";if { $tcl_platform(platform) == \"windows\"} {set gpterm \"windows\"} {set gpterm \"x11\"}}");
//cmd(inter, "wm resizable . 0 0");

*choice=0;

cmd(inter, "frame .f");
cmd(inter, "frame .f.vars");
cmd(inter, "set f .f.vars.lb");
cmd(inter, "frame $f");
cmd(inter, "label .f.vars.lb.l -text \"Series Available\"");
cmd(inter, "pack .f.vars.lb.l");
cmd(inter, "scrollbar $f.v_scroll -command \"$f.v yview\"");
cmd(inter, "listbox $f.v -selectmode extended -width 40 -yscroll \"$f.v_scroll set\" -height 12");

cmd(inter, "bind $f.v <Double-Button-1> {.f.vars.b.in invoke}");
cmd(inter, "bind $f.v <Return> {.f.vars.b.in invoke}");
cmd(inter, "bind $f.v <Button-3> {.f.vars.lb.v selection clear 0 end;.f.vars.lb.v selection set @%x,%y; set res [selection get]; set choice 30}");
cmd(inter, "bind $f.v <Button-2> {.f.vars.lb.v selection clear 0 end;.f.vars.lb.v selection set @%x,%y; set res [selection get]; set choice 30}");

cmd(inter, "bind $f.v <KeyPress-b> {set res [.f.vars.lb.v get active]; set choice 30}");


cmd(inter, "bind $f.v <Shift-Button-3> {.f.vars.lb.v selection clear 0 end;.f.vars.lb.v selection set @%x,%y; set res [selection get]; set choice 16}");
cmd(inter, "pack $f.v $f.v_scroll -side right -fill y");

cmd(inter, "set f .f.vars.ch");
cmd(inter, "frame $f");
cmd(inter, "label .f.vars.ch.l -text \"Series Selected\"");
cmd(inter, "pack .f.vars.ch.l");
cmd(inter, "scrollbar $f.v_scroll -command \"$f.v yview\"");
cmd(inter, "listbox $f.v -selectmode extended -width 40 -yscroll \"$f.v_scroll set\" -height 12");
cmd(inter, "bind $f.v <KeyPress-o> {.f.vars.b.out invoke}");
cmd(inter, "bind $f.v <Return> {.b.ts invoke}");

cmd(inter, "pack $f.v $f.v_scroll -side right -fill y");
cmd(inter, "bind $f.v <Double-Button-1> {.f.vars.b.out invoke}");
cmd(inter, "bind $f.v <Button-3> {.f.vars.ch.v selection clear 0 end;.f.vars.ch.v selection set @%x,%y; set res [selection get]; set choice 33}");
cmd(inter, "bind $f.v <Button-2> {.f.vars.ch.v selection clear 0 end;.f.vars.ch.v selection set @%x,%y; set res [selection get]; set choice 33}");

cmd(inter, "set f .f.vars.pl");
cmd(inter, "frame $f");
cmd(inter, "label $f.l -text \"Graphs\"");
cmd(inter, "pack $f.l");
cmd(inter, "scrollbar $f.v_scroll -command \"$f.v yview\"");
cmd(inter, "listbox $f.v -width 30 -yscroll \"$f.v_scroll set\" -height 12 -selectmode single");

cmd(inter, "pack $f.v $f.v_scroll -side right -fill y");
cmd(inter, "bind $f.v <Double-Button-1> {set it [selection get];set choice 3}");
cmd(inter, "bind .f.vars.pl.v <Button-3> {.f.vars.pl.v selection clear 0 end; .f.vars.pl.v selection set @%x,%y; set it [selection get]; set n_it [.f.vars.pl.v curselection]; set choice 20}");
cmd(inter, "bind .f.vars.pl.v <Button-2> {.f.vars.pl.v selection clear 0 end; .f.vars.pl.v selection set @%x,%y; set it [selection get]; set n_it [.f.vars.pl.v curselection]; set choice 20}");
cmd(inter, "bind . <KeyPress-Delete> {set n_it [.f.vars.pl.v curselection]; if {$n_it != \"\" } {set it [selection get]; set choice 20} {}}");

cmd(inter, "frame .f.vars.b");
cmd(inter, "set f .f.vars.b");
cmd(inter, "button $f.in -text > -command {set choice 6}");
cmd(inter, "button $f.out -state disabled -text < -command {set choice 7}");
cmd(inter, "button $f.sort -text Sort -command {set choice 5}");
cmd(inter, "button $f.unsort -text \"Un-sort\" -command {set choice 14}");
cmd(inter, "button $f.sortend -text \"Sort (End)\" -command {set choice 15}");
cmd(inter, "button $f.empty -text Clear -command {set choice 8}");

cmd(inter, "button $f.add -text \"Add series\" -command {set choice 24}");

cmd(inter, "pack $f.in $f.out $f.sort $f.sortend $f.unsort $f.add $f.empty -fill x");


cmd(inter, "pack .f.vars.lb .f.vars.b .f.vars.ch .f.vars.pl -side left");
cmd(inter, "pack .f.vars");

num_c=1;
num_var=0;
if(actual_steps>0)
{
if(data_infile==0)
  insert_data_mem(root, &num_var, &num_c);
else
  insert_data_file(&num_var, &num_c);
}
min_c=1;
max_c=num_c;
cmd(inter, "frame .f.com");
sprintf(msg, "label .f.com.nvar -text \"Series = %d\"",num_var);
cmd(inter,msg);
sprintf(msg, "label .f.com.ncas -text \"Cases = %d\"", num_c);
cmd(inter,msg);
cmd(inter, "pack .f.com.nvar .f.com.ncas -side left");
cmd(inter, "frame .f.ft");
cmd(inter, "label .f.ft.minc -text \"From case:\"");
Tcl_LinkVar(inter, "minc", (char *) &min_c, TCL_LINK_INT);
cmd(inter, "entry .f.ft.mnc -width 10 -relief sunken -textvariable minc -state disabled");

cmd(inter, "label .f.ft.maxc -text \" to case:\"");
Tcl_LinkVar(inter, "maxc", (char *) &max_c, TCL_LINK_INT);
cmd(inter, "entry .f.ft.mxc -width 10 -relief sunken -textvariable maxc -state disabled");
cmd(inter, "set auto_x 1");
autom_x=1;
Tcl_LinkVar(inter, "auto_x", (char *) &autom_x, TCL_LINK_INT);
cmd(inter, "checkbutton .f.ft.auto -text \"Use all cases \" -variable auto_x -command {if {$auto_x==1} {.f.ft.mxc conf -state disabled; .f.ft.mnc conf -state disabled} {.f.ft.mxc conf -state normal; .f.ft.mnc conf -state normal}}");

cmd(inter, "pack .f.ft.auto .f.ft.minc .f.ft.mnc .f.ft.maxc .f.ft.mxc -side left");

cmd(inter, "frame .f.sc");
cmd(inter, "set auto 1");
autom=1;
Tcl_LinkVar(inter, "auto", (char *) &autom, TCL_LINK_INT);

cmd(inter, "checkbutton .f.sc.auto -text \"Y Self-scaling\" -variable auto -command {if {$auto==1} {.f.sc.max conf -state disabled; .f.sc.min conf -state disabled} {.f.sc.max conf -state normal; .f.sc.min conf -state normal}}");
Tcl_LinkVar(inter, "maxy", (char *) &maxy, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "miny", (char *) &miny, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "logs", (char *) &logs, TCL_LINK_INT);

logs=0;
cmd(inter, "frame .f.y2");
cmd(inter, "checkbutton .f.y2.logs -text \"Series in logs\" -variable logs");
cmd(inter, "set y2 0");
cmd(inter, "checkbutton .f.y2.y2 -text \"Y2 axis\" -variable y2");
cmd(inter, "label .f.y2.l -text \"Num. of first series in Y2 axis\"");
cmd(inter, "set numy2 2");
cmd(inter, "entry .f.y2.e -textvariable numy2 -width 4");
miny=maxy=0;
cmd(inter, "label .f.sc.lmax -text \"Max. Y\"");
cmd(inter, "entry .f.sc.max -width 10 -relief sunken -textvariable maxy -state disabled");
cmd(inter, "label .f.sc.lmin -text \"Min. Y\"");
cmd(inter, "entry .f.sc.min -width 10 -relief sunken -textvariable miny -state disabled");
cmd(inter, "pack .f.sc.auto .f.sc.lmin .f.sc.min .f.sc.lmax .f.sc.max -side left");
cmd(inter, "pack .f.y2.logs .f.y2.y2 .f.y2.l .f.y2.e -side left");
cmd(inter, "pack .f.com .f.vars .f.ft .f.sc .f.y2");

allblack=0;
grid=0;
Tcl_LinkVar(inter, "allblack", (char *) &allblack, TCL_LINK_INT);
Tcl_LinkVar(inter, "grid", (char *) &grid, TCL_LINK_INT);
Tcl_LinkVar(inter, "point_size", (char *) &point_size, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "tc", (char *) &time_cross, TCL_LINK_INT);
Tcl_LinkVar(inter, "line_point", (char *) &line_point, TCL_LINK_INT);
Tcl_LinkVar(inter, "xy", (char *) &xy, TCL_LINK_INT);
Tcl_LinkVar(inter, "pdigits", (char *) &pdigits, TCL_LINK_INT);

point_size=1.0;
xy=0;
line_point=1;
time_cross=1;
pdigits=4;

cmd(inter, "frame .f.tit -relief groove -bd 2");
cmd(inter, "label .f.tit.l -text Title");
cmd(inter, "entry .f.tit.e -textvariable tit -width 50");
cmd(inter, "checkbutton .f.tit.allblack -text \"No Colors\" -variable allblack");
cmd(inter, "checkbutton .f.tit.grid -text \"Grids\" -variable grid");
cmd(inter, "frame .f.tit.lp");
cmd(inter, "radiobutton .f.tit.lp.line -text \"Lines\" -variable line_point -value 1");
cmd(inter, "radiobutton .f.tit.lp.point -text \"Points\" -variable line_point -value 2");
cmd(inter, "pack .f.tit.lp.line .f.tit.lp.point -anchor w");

cmd(inter, "frame .f.tit.ps");
cmd(inter, "label .f.tit.ps.l -text \"Point size\"");
cmd(inter, "entry .f.tit.ps.e -width 4 -textvariable point_size");
cmd(inter, "pack .f.tit.ps.l .f.tit.ps.e -anchor w");

cmd(inter, "frame .f.tit.pr");			// field for adjusting y-axis precision
cmd(inter, "label .f.tit.pr.l -text \"Precision\"");
cmd(inter, "entry .f.tit.pr.e -textvariable pdigits -width 2");
cmd(inter, "pack .f.tit.pr.l .f.tit.pr.e -anchor w");

cmd(inter, "pack .f.tit.l .f.tit.e .f.tit.allblack .f.tit.grid .f.tit.lp .f.tit.ps .f.tit.pr -side left");
cmd(inter, "pack .f.tit");


cmd(inter, "frame .b");
cmd(inter, "button .b.lat -text \"Lattice\" -command {set choice 23}");
cmd(inter, "button .b.ts -text \"Plot\" -command {set choice 1}");
cmd(inter, "button .b.sv -text \"Save Data\" -command {set choice 10}");
cmd(inter, "button .b.sp -text \"Print Data\" -command {set choice 36}");
cmd(inter, "button .b.st -text \"Statistics\" -command {set choice 12}");
cmd(inter, "button .b.fr -text \"Histograms\" -command {set choice 32}");
cmd(inter, "button .b.dump -text \"Postscript\" -command {set choice 11}");

cmd(inter, "frame .b.tc");
cmd(inter, "radiobutton .b.tc.time -text \"Time Series\" -variable tc -value 1");
cmd(inter, "radiobutton .b.tc.cross -text \"Cross Section\" -variable tc -value 2");
cmd(inter, "pack .b.tc.time .b.tc.cross -anchor w");

cmd(inter, "frame .b.xy");
cmd(inter, "radiobutton .b.xy.seq -text \"Sequence\" -variable xy -value 0");
cmd(inter, "radiobutton .b.xy.xy -text \"XY plot\" -variable xy -value 1");
//cmd(inter, "radiobutton .b.xy.pd -text \"Phase diagram\" -variable xy -value 2");
cmd(inter, "pack .b.xy.seq .b.xy.xy -anchor w");



Tcl_LinkVar(inter, "watch", (char *) &watch, TCL_LINK_INT);
cmd(inter, "checkbutton .b.watch -text Watch -variable watch");


cmd(inter, "pack .b.watch .b.ts .b.sv .b.sp .b.st .b.fr .b.dump .b.lat .b.tc .b.xy -side left");
cmd(inter, "pack .f .b");
cmd(inter, "set watch 1");
cmd(inter, "set auto 1");
//cmd(inter, "raise .");
cmd(inter, "focus .");

cmd(inter, "menu .m -tearoff 0 -relief groove -bd 2");

cmd(inter, "set w .m.exit");
cmd(inter, ".m add cascade -label Exit -menu $w -underline 0");
cmd(inter, "menu $w -tearoff 0 -relief groove -bd 2");
cmd(inter, "$w add command -label \"Exit Analysis of Result\" -command {set choice 2}");

cmd(inter, "set w .m.gp");
cmd(inter, ".m add cascade -label Gnuplot -menu $w -underline 0");
cmd(inter, "menu $w -tearoff 0 -relief groove -bd 2");
cmd(inter, "$w add command -label \"Gnuplot\" -command {if {$tcl_platform(platform) == \"unix\"} {exec xterm -e gnuplot &} {if {$tcl_platform(os) == \"Windows NT\"} {exec wgnuplot &} {exec start wgnuplot &}}}");
cmd(inter, "$w add command -label \"Gnuplot options\" -command {set choice 37}");


cmd(inter, "set w .m.color");
cmd(inter, ".m add cascade -label Color -menu $w -underline 0");
cmd(inter, "menu $w -tearoff 0 -relief groove -bd 2");
cmd(inter, "$w add command -label \"Default colors\" -command {set choice 22}");
cmd(inter, "$w add command -label \"Set colors\" -command {set choice 21}");

cmd(inter, "set w .m.help");
cmd(inter, "menu $w -tearoff 0 -relief groove -bd 2");
cmd(inter, ".m add cascade -label Help -menu $w -underline 0");
cmd(inter, "$w add command -label \"Analysis of Result - Help\" -command {set choice 41}");
cmd(inter, "$w add command -label \"Model Report\" -command {set choice 44}");

cmd(inter, "bind . <Control-x> {set choice 23}");
cmd(inter, "bind . <Control-z> {set choice 24}");
cmd(inter, "bind . <Control-h> {set choice 32}");
cmd(inter, "bind . <KeyPress-Escape> {set choice 2}");
cmd(inter, "bind . <Control-c> {.f.vars.b.empty invoke}"); 
cmd(inter, "bind . <Control-a> {set choice 24}");
cmd(inter, "bind . <Control-i> {set choice 34}");
if(num_var==0)
 {
  set_window_size();
  cmd(inter, "tk_messageBox -type ok -title \"No Series\" -message \"Apparently there are no series available from a recent simulation run.\\nClick on button \'Add Series\' to select series to analyse from previously saved files or current state of the model. \\nIf you expected data from a simulation that are not available you probably forgot to select series to save, or set the Objects containing them to be not computed.\"");  
 }
there :
set_window_size();
while(*choice==0)
 Tcl_DoOneEvent(0);


if(*choice==1 && time_cross==2 && xy==0) //Plot cross section
 *choice=9;

if(*choice==1 && time_cross==1 && xy==1) //Plot XY
 *choice=17;

if(*choice==1 && time_cross==2 && xy==1) //Plot XY Cross section
 *choice=18;

if(*choice==12 && time_cross==2) //Statistics cross section
 *choice=13;

if(pdigits<1 || pdigits>8)
	pdigits=4;

switch(*choice)
{

//Plot
case 1:
  *choice=1;
  cmd(inter, "set choice [.f.vars.ch.v size]");
  if(*choice==0) //No variables to plot defined
   {
   cmd(inter, "tk_messageBox -type ok -title \"No Series\" -message \"No series selected.\\nPlace some series in the Series Selected listbox.\"");
   goto there;
   }
  cur_plot++;
  cmd(inter, ".b.ts conf -text Stop");
  *choice=0;
  cmd(inter, "set exttit \"$cur_plot) $tit\"");
  plot(choice);
  cmd(inter, ".b.ts conf -text \"Plot\"");
  if(*choice==22) //plot aborted
    {
   cur_plot--;
   *choice=0;
   goto there;
   }
 

  *choice=0;

  cmd(inter, ".f.vars.pl.v insert end $exttit");
  type_graph[cur_plot]=0; //Lsd standard graph
  graph_l[cur_plot]=390; //height of graph with labels
  graph_nl[cur_plot]=325; //height of graph with labels  

  goto there;

//exit
case 2:
///plog("here 1\n");
cmd(inter, "set answer [tk_messageBox -type yesno -title \"Warning\" -message \"Do you really want to exit Analysis of Results?\\nAll the graphs created will be lost.\"]");
app=(char *)Tcl_GetVar(inter, "answer",0);

cmd(inter, "if {[string compare $answer \"yes\"] == 0} { } {set choice 0}");
if(*choice==0)
  goto there;
cmd(inter, "bind . <Control-x> {}");
cmd(inter, "bind . <Control-z> {}");
cmd(inter, "bind . <KeyPress-Escape> {}");
cmd(inter, "bind . <KeyPress-Delete> {}");

if(data_infile==1)
{
for(i=0; i<num_var; i++)
 delete[] vs[i].data;

}
delete[] vs;
  cmd(inter, "destroy .f .b .m");
  Tcl_UnlinkVar(inter, "minc");
  Tcl_UnlinkVar(inter, "maxc");
  Tcl_UnlinkVar(inter, "maxy");
  Tcl_UnlinkVar(inter, "miny");
  Tcl_UnlinkVar(inter, "auto");
  Tcl_UnlinkVar(inter, "watch");
  Tcl_UnlinkVar(inter, "allblack");
  Tcl_UnlinkVar(inter, "cur_plot");
Tcl_UnlinkVar(inter, "grid");
Tcl_UnlinkVar(inter,"point_size");
Tcl_UnlinkVar(inter,"tc");
Tcl_UnlinkVar(inter,"line_point");
Tcl_UnlinkVar(inter,"xy");

  cmd(inter, "catch [set a [glob -nocomplain plotxy_*]]"); //remove directories
  cmd(inter, "catch [foreach b $a {catch [file delete -force $b]}]");
  return;
//Raise the clicked graph
case 3:

  cmd(inter, "scan $it %d)%s a b");
  cmd(inter, "set ex [winfo exists .f.new$a]");
  *choice=0;
  cmd(inter, "if { $ex == 1 } {wm deiconify .f.new$a; raise .f.new$a} {set choice 1}");
  if(*choice==1)
   {
   getcwd(dirname, 300);
   cmd(inter, "set choice $a");
   sprintf(msg, "plotxy_%d",*choice);
   chdir(msg);
   cmd(inter, "set choice $a");
   show_plot_gnu(*choice, choice, 0);
   chdir(dirname);
   }

  miny=maxy=0;

  *choice=0;
  goto there;

// Sort
case 5:
  cmd(inter, "set a {}");
  cmd(inter, "set a [.f.vars.lb.v get 0 end]");
  cmd(inter, "set b {}");
  cmd(inter, "set b [lsort -dictionary $a]");
  cmd(inter, ".f.vars.lb.v delete 0 end");
  cmd(inter, "foreach i $b {.f.vars.lb.v insert end $i}");
  *choice=0;
  goto there;
  
case 34: //sort the selection in selected series list in inverse order
   cmd(inter, "set a [.f.vars.ch.v curselection]");
   cmd(inter, "set choice [llength $a]");
   if(*choice==0)
    goto there;
   cmd(inter, "set b {}");
   cmd(inter, "foreach i $a {lappend b [.f.vars.ch.v get $i]}");
   cmd(inter, "set c [lsort -decreasing -dictionary $b]");
   cmd(inter, "set d -1");
   cmd(inter, "foreach i $a {.f.vars.ch.v delete $i; .f.vars.ch.v insert $i [lindex $c [incr d]] }");
   *choice=0;
   goto there;
// Un-sort
case 36: //Print the data series in the log window
   
   plog_series(choice);
   cmd(inter, "wm deiconify .log");
   cmd(inter, "raise .log .");
   *choice=0;
   goto there;
case 37: //set options for gnuplot
cmd(inter, "toplevel .a");
cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Gnuplot options\"");
cmd(inter, "label .a.l -text \"Set options for gnuplot\" -fg red");
cmd(inter, "frame .a.t -relief groove -bd 2");
cmd(inter, "label .a.t.l -text \"Terminal \"");
cmd(inter, "entry .a.t.e -textvariable gpterm -width 12");
cmd(inter, "pack .a.t.l .a.t.e -side left -anchor w");
cmd(inter, "frame .a.d -relief groove -bd 2");
cmd(inter, "label .a.d.l -text \"Grid details \"");
cmd(inter, "entry .a.d.e -textvariable gpdgrid3d -width 12");
cmd(inter, "pack .a.d.l .a.d.e -side left -anchor w");
cmd(inter, "frame .a.o -relief groove -bd 2");
cmd(inter, "label .a.o.l -text \"Other options\"");
cmd(inter, "text .a.o.t -height 10 -width 30");
cmd(inter, ".a.o.t insert end \"$gpooptions\"");
cmd(inter, "pack .a.o.l .a.o.t");
cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.default -text \"Default\" -command {set choice 3}");
cmd(inter, "button .a.b.ok -text \" Ok \" -command {set choice 1}");
cmd(inter, "button .a.b.help -text \" Help \" -command {LsdHelp mdatares.html#gpoptions}");
cmd(inter, "pack .a.b.default .a.b.ok .a.b.help -side left");
cmd(inter, "pack .a.l .a.t .a.d .a.o .a.b -fill x");
//cmd(inter, "bind .a <Return> {.a.b.ok invoke}");
gpoptions :

*choice=0;

while(*choice==0)
	Tcl_DoOneEvent(0);
if(*choice==3)
 {
  cmd(inter, "set gpdgrid3d \"60,60,3\"");
  cmd(inter, "if { $tcl_platform(platform) == \"windows\"} {set gpterm \"windows\"} { set gpterm \"x11\"}");
  goto gpoptions;
 }
cmd(inter, "set gpooptions \"\[.a.o.t get 0.0 end]\""); 
cmd(inter, "destroy .a");
*choice=0;
goto there;
 
   
case 14:
   cmd(inter, ".f.vars.lb.v delete 0 end");
   for(i=0; i<num_var; i++)
    {
     sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", vs[i].label, vs[i].tag, vs[i].start, vs[i].end, vs[i].rank);
     cmd(inter, msg);
    }
   *choice=0;
   goto there;

// Sort on End
case 15:
   app_store=new store[num_var];
   for(i=0; i<num_var; i++)
     app_store[i]=vs[i];
   sort_on_end(app_store);
   cmd(inter, ".f.vars.lb.v delete 0 end");
   for(i=0; i<num_var; i++)
    {
     sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", app_store[i].label, app_store[i].tag, app_store[i].start, app_store[i].end, app_store[i].rank);
     cmd(inter, msg);
    }
    delete[] app_store;
   *choice=0;
   goto there;

// Insert the variables selected in the list of the variables to plot
case 6:
  cmd(inter, "set a {}");
  cmd(inter, "set a [.f.vars.lb.v curselection]");
  cmd(inter, "foreach i $a {.f.vars.ch.v insert end [.f.vars.lb.v get $i]}");
  cmd(inter, ".f.vars.b.out conf -state normal");
  *choice=0;
  cmd(inter, "set tit [.f.vars.ch.v get 0]");

  goto there;

//Remove the vars. selected from the variables to plot
case 7:

  cmd(inter, "set steps 0");
  cmd(inter, "foreach i [.f.vars.ch.v curselection] {.f.vars.ch.v delete [expr $i - $steps]; incr steps}");
  cmd(inter, "if [.f.vars.ch.v size]==0 {$f.out conf -state disabled}");
  *choice=0;
  goto there;

//Remove all the variables from the list of vars to plot
case 8:
  cmd(inter, ".f.vars.ch.v delete 0 end");
  *choice=0;
  goto there;

//Plot the cross-section graph. That is, the vars selected form
//one series for each time step selected
case 9:
  *choice=1;
  cmd(inter, "set choice [.f.vars.ch.v size]");
  if(*choice==0) //No variables to plot defined
   goto there;

   set_cs_data(choice);
   if(*choice==2)
    {*choice=0;
     goto there;
    }
    cur_plot++;
//    cmd(inter, ".b.cs conf -state disabled");
    cmd(inter, "set exttit \"$cur_plot) $tit\"");
    *choice=0;
    plot_cross(choice);
//    cmd(inter, ".b.cs conf -state normal");

    *choice=0;

    cmd(inter, ".f.vars.pl.v insert end $exttit");
    type_graph[cur_plot]=0; //Lsd standard graph
    graph_l[cur_plot]=390; //height of graph with labels
    graph_nl[cur_plot]=325; //height of graph with labels  

    goto there;

//Brutally shut down the system
case 35:
myexit(0);

case 16:
//show the equation for the selected variable

cmd(inter, "set a [split $res]");
cmd(inter, "set b [lindex $a 0]");

app=(char *)Tcl_GetVar(inter, "b",0);

*choice=0;
show_eq(app, choice);
goto there;

//Shortcut to show equation
case 29:
app=(char *)Tcl_GetVar(inter, "res",0);
show_eq(app, choice);
*choice=0;
goto there;

case 41:
//help on Analysis of Result

cmd(inter, "LsdHelp mdatares.html");
*choice=0;
goto there;


case 44:
//see model report

sprintf(name_rep, "report_%s.html", simul_name);
sprintf(msg, "set choice [file exists %s]", name_rep);
cmd(inter, msg);
if(*choice == 0)
 {
  cmd(inter, "set answer [tk_messageBox -message \"Model report not found.\\nYou may create a model report file from menu Model.\\nDo you want to look for another HTML file?\" -type yesno -title Warning -icon warning]");
  cmd(inter, "if {[string compare $answer \"yes\"] == 0} {set choice 1} {set choice 0}");
 if(*choice == 0)
  goto there;
 cmd(inter, "set fname [tk_getOpenFile -title \"Load HTML File\" -defaultextension \".html\" -initialdir [pwd] -filetypes {{{HTML Files} {.html}} {{All Files} {*}} }]");
 cmd(inter, "if {$fname == \"\"} {set choice 0} {set choice 0}");
 if(*choice == 0)
  goto there;

 }
else
 {
 sprintf(msg, "set fname %s", name_rep);
 cmd(inter, msg);
 }


if(*choice==1) //model report exists
  cmd(inter, "LsdHtml $fname");
  
*choice=0;
goto there;


//Save data in the Vars. to plot listbox
case 10:
// save_data1(choice);
 save_datazip(choice);
 *choice=0;
 goto there;

//Statistics
case 12:
 statistics(choice);
 *choice=0;
 goto there;

//frequencies
case 25:
 frequencies(choice);
 *choice=0;
 goto there;

//Edit labels
case 26:
*choice=0;
 cmd(inter, "toplevel .a");
 cmd(inter, "wm title .a \"Edit label\"");
 cmd(inter, "wm geometry .a +$hereX+$hereY");
 cmd(inter, "wm transient .a $ccanvas");
 cmd(inter, "label .a.l -text \"New label for the selected label\"");
 cmd(inter, "set itext [$ccanvas itemcget selected -text]");
 cmd(inter, "entry .a.e -textvariable itext -width 30");
 cmd(inter, "frame .a.format -relief groove -bd 2");
 cmd(inter, "label .a.format.tit -text \"Font name, size, style and color\"");
 cmd(inter, "pack .a.format.tit");
 cmd(inter, "set ifont [lindex [$ccanvas itemcget selected -font] 0]");
 cmd(inter, "set idim [lindex [$ccanvas itemcget selected -font] 1]");
 cmd(inter, "set istyle [lindex [$ccanvas itemcget selected -font] 2]");
 cmd(inter, "frame .a.format.e");
 cmd(inter, "entry .a.format.e.font -textvariable ifont -width 30");
 cmd(inter, "entry .a.format.e.dim -textvariable idim -width 4");
  cmd(inter, "entry .a.format.e.sty -textvariable istyle -width 10");
 cmd(inter, "set icolor [$ccanvas itemcget selected -fill]");  
 cmd(inter, "button .a.format.e.color -text Color -background white -foreground $icolor -command {set app [tk_chooseColor -initialcolor $icolor]; if { $app != \"\"} {set icolor $app} {}; .a.format.e.color configure -foreground $icolor; focus -force .a.format.e.color}");

 cmd(inter, "bind .a.format.e.font <Return> {.a.b.ok invoke}");
 cmd(inter, "bind .a.format.e.dim <Return> {.a.b.ok invoke}");
 
 cmd(inter, "pack .a.format.e.font .a.format.e.dim .a.format.e.sty .a.format.e.color -side left");

 cmd(inter, "frame .a.format.fall");
 cmd(inter, "set fontall 0");
 cmd(inter, "set colorall 0");
 cmd(inter, "checkbutton .a.format.fall.font -text \"Apply font to all text items\" -variable fontall");
 cmd(inter, "checkbutton .a.format.fall.color -text \"Apply color to all text items\" -variable colorall");

 cmd(inter, "pack .a.format.fall.font .a.format.fall.color -anchor w");
 cmd(inter, "pack .a.format.e .a.format.fall -anchor w");

 cmd(inter, "frame .a.b");
 
 cmd(inter, "button .a.b.ok -text \" Ok \" -command {set choice 1}");
 cmd(inter, "button .a.b.esc -text \" Cancel \" -command {set choice 2}");
 cmd(inter, "pack .a.b.ok .a.b.esc -side left");
 cmd(inter, "pack .a.l .a.e .a.format .a.b");
 cmd(inter, ".a.e selection range 0 end");
 cmd(inter, "focus -force .a.e");
 cmd(inter, "bind .a.e <Return> {.a.b.ok invoke}");
 
  while(*choice==0)
	Tcl_DoOneEvent(0);

if(*choice==1)
 {
  cmd(inter, " if { $itext==\"\"} {$ccanvas delete selected; set choice 0} {}");
  if(*choice==1)
  {
   cmd(inter, "$ccanvas itemconf selected -text \"$itext\"");
   cmd(inter, "set ml [list $ifont $idim $istyle]");
   cmd(inter, "$ccanvas itemconf selected -font \"$ml\"");
   cmd(inter, "$ccanvas itemconf selected -fill $icolor");  
  }
  cmd(inter, "set choice $fontall");
  if(*choice==1)
   cmd(inter, "$ccanvas itemconf text -font \"$ml\"");
  cmd(inter, "set choice $colorall");
  if(*choice==1)
   cmd(inter, "$ccanvas itemconf text -fill $icolor");
 
 } 
 
cmd(inter, "$ccanvas dtag selected"); 
cmd(inter, "destroy .a"); 
cmd(inter, "focus -force $ccanvas");
 *choice=0;
 goto there;


//New labels
case 27:
*choice=0;
 cmd(inter, "toplevel .a");
 cmd(inter, "wm geometry .a +$hereX+$hereY");
 cmd(inter, "wm title .a \"Insert new labels\"");
 cmd(inter, "wm transient .a $ncanvas");
 cmd(inter, "label .a.l -text \"New label\"");
 cmd(inter, "set itext \"new text\"");
 cmd(inter, "entry .a.e -textvariable itext -width 30");
 cmd(inter, "frame .a.b");
 
 cmd(inter, "button .a.b.ok -text \" Ok \" -command {set choice 1}");
 cmd(inter, "button .a.b.esc -text \" Cancel \" -command {set choice 2}");
 cmd(inter, "pack .a.b.ok .a.b.esc -side left");
 cmd(inter, "pack .a.l .a.e .a.b");
 cmd(inter, ".a.e selection range 0 end");
 cmd(inter, "focus -force .a.e");
 cmd(inter, "bind .a.e <Return> {.a.b.ok invoke}");
 
  while(*choice==0)
	Tcl_DoOneEvent(0);
cmd(inter, "destroy .a"); 
if(*choice==2)
{

 *choice=0;
 goto there;
}

cmd(inter, "set choice $ncanvas");
i=*choice;
sprintf(msg, "set choice [.f.new%d.f.plots create text $LX $LY -text \"$itext\" -font {{Times} 10}]", i);
cmd(inter, msg);

  sprintf(msg, ".f.new%d.f.plots bind $choice <1> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; .f.new%d.f.plots raise current; set LX %%x; set LY %%y}", i,i,i,*choice,i);
  cmd(inter, msg);
  sprintf(msg, ".f.new%d.f.plots bind $choice <ButtonRelease-1> {.f.new%d.f.plots dtag selected}",i,i);
  cmd(inter, msg);
  sprintf(msg, ".f.new%d.f.plots bind $choice <B1-Motion> {.f.new%d.f.plots move selected [expr %%x-$LX] [expr %%y - $LY]; set LX %%x; set LY %%y}",i,i);
  cmd(inter, msg);
  sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",i,i,i, *choice,i);
  cmd(inter, msg);
  sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",i,i,i, *choice,i);
  cmd(inter, msg);
 *choice=0;
 goto there;

case 31:
/*
Edit line's color
*/
*choice=0;
/*
 cmd(inter, "toplevel .a");
 cmd(inter, "wm title .a \"Edit lines\"");
 cmd(inter, "wm geometry .a +$hereX+$hereY");
 cmd(inter, "wm transient .a $ccanvas");
 cmd(inter, "label .a.l -text \"New label for the selected series\"");
 cmd(inter, "set itext [$ccanvas itemcget selected -text]");
 cmd(inter, "entry .a.e -textvariable itext -width 30");
 cmd(inter, "frame .a.format -relief groove -bd 2");
 cmd(inter, "label .a.format.tit -text \"Font name and dim.\"");
 cmd(inter, "pack .a.format.tit");
 cmd(inter, "set ifont [lindex [$ccanvas itemcget selected -font] 0]");
 cmd(inter, "set idim [lindex [$ccanvas itemcget selected -font] 1]");
 
 cmd(inter, "entry .a.format.font -textvariable ifont -width 30");
 cmd(inter, "entry .a.format.dim -textvariable idim -width 4");
 cmd(inter, "bind .a.format.font <Return> {.a.b.ok invoke}");
 cmd(inter, "bind .a.format.dim <Return> {.a.b.ok invoke}");
 
 cmd(inter, "pack .a.format.font .a.format.dim -side left");
 cmd(inter, "set icolor [$ccanvas itemcget selected -fill]");
 cmd(inter, "button .a.color -text Color -background white -foreground $icolor -command {set app [tk_chooseColor -initialcolor $icolor]; if { $app != \"\"} {set icolor $app} {}; .a.color configure -foreground $icolor; focus -force .a.color}");
 cmd(inter, "frame .a.b");
 
 cmd(inter, "button .a.b.ok -text \" Ok \" -command {set choice 1}");
 cmd(inter, "button .a.b.esc -text \" Cancel \" -command {set choice 2}");
 cmd(inter, "pack .a.b.ok .a.b.esc -side left");
 cmd(inter, "pack .a.l .a.e .a.format .a.color .a.b");
 cmd(inter, ".a.e selection range 0 end");
 cmd(inter, "focus -force .a.e");
 cmd(inter, "bind .a.e <Return> {.a.b.ok invoke}");
 
  while(*choice==0)
	Tcl_DoOneEvent(0);
*/

cmd(inter, "set icolor [$ccanvas itemcget $cline -fill]");
cmd(inter, "set app [tk_chooseColor -initialcolor $icolor]; if { $app != \"\"} {set icolor $app} {}");
cmd(inter, "$ccanvas itemconfig $cline -fill $icolor");
cmd(inter, "focus -force $ccanvas");

goto there;
*choice=0;

case 28: 
/*insert a line item

NON ACTIVATED
*/
cmd(inter, "set choice $ncanvas");
sprintf(msg, "set c .f.new%d.f.plots", *choice);
cmd(inter, msg);

cmd(inter, "bind $c <B1-Motion> {$c delete selected; set ax %x; set ay %y; set cl [$c create line $px $py $ax $ay]; $c addtag selected withtag $cl}");
cmd(inter, "bind $c <ButtonRelease-1> {$c dtag selected}");

cmd(inter, "bind $c <1> {set px %x; set py %y; set ax [expr $px+1]; set ay [expr $py+1]; $c dtag selected; set cl [$c create line $px $py $ax $ay]; $c addtag selected withtag $cl }");
*choice=0;
goto there;



/*
Statistics Cross Section
*/
case 13:
  *choice=1;
  cmd(inter, "set choice [.f.vars.ch.v size]");
  if(*choice==0) //No variables to plot defined
   goto there;

   set_cs_data(choice);
   if(*choice==2)
    {*choice=0;
     goto there;
    }

 statistics_cross(choice);
 *choice=0;
 goto there;

case 11:
//create the postscript for a graph

exist_selection:
*choice=0;
  cmd(inter, "toplevel .w");
  cmd(inter, "wm title .w \"Save Lsd Graph\"");
  cmd(inter, "wm transient .w .");
  cmd(inter, "bind .w <Destroy> {set choice 2}");
  cmd(inter, "label .w.l -text \"Select one graph\"");
  cmd(inter, "button .w.o -text Ok -command {set choice 1}");
  cmd(inter, "button .w.c -text Cancel -command {set choice 2}");
  cmd(inter, "pack .w.l .w.o .w.c");
  cmd(inter, "focus -force .w.o");
  while(*choice==0)
	Tcl_DoOneEvent(0);

cmd(inter, "bind .w <Destroy> {}");
cmd(inter, "destroy .w");
if(*choice==2)
   {*choice=0;
    goto there;
   }
*choice=0;
cmd(inter, "set iti \"\"");
cmd(inter, "set iti [.f.vars.pl.v curselection]");
cmd(inter, "if {[string length $iti] == 0} {set choice 1}");
cmd(inter, "update");
if(*choice==1)
  goto exist_selection;


Tcl_LinkVar(inter, "res", (char *) &rot, TCL_LINK_INT);

cmd(inter, "set it [.f.vars.pl.v get $iti]");
cmd(inter, "scan $it %d)%s a b");

cmd(inter, "set choice [winfo exist .f.new$a]");
if(*choice == 0)
 {
  cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"The selected graph does not exist anymore.\nIf you closed the window of the graph, then re-create it.\nIf the graph is a high-quality scatter plot, use the gnuplot facilities to export it (use the right button of the mouse).\"");
  Tcl_UnlinkVar(inter, "res");
  goto there;  
 }
cmd(inter, "set cazzo $a");
cmd(inter, "toplevel .filename");
cmd(inter, "set r .filename");
cmd(inter, "wm title .filename \"Save Lsd Graph\"");
cmd(inter, "wm transient .filename .");
cmd(inter, "bind .filename <Destroy> {set choice 2}");
cmd(inter, "label .filename.l -text \"Save the graph\\n$it\"");
cmd(inter, "frame .filename.b");
cmd(inter, "button .filename.b.ok -text Ok -bd 2 -command {if {[string length $fn] > 0 } {set choice 1} {set choice 3} }");
cmd(inter, "button .filename.b.c -text Cancel -bd 2 -command {set choice 2}");
cmd(inter, "bind .filename <KeyPress-Return> {.filename.b.ok invoke} ");
cmd(inter, "bind .filename <KeyPress-Escape> {.filename.b.c invoke}");
cmd(inter, "bind .filename <Destroy> {set choice 2}");
cmd(inter, "button .filename.s -text \"Choose File\" -command {set fn1 [tk_getSaveFile -title \"Graph File Name\" -defaultextension \"eps\" -initialfile $fn -filetypes {{{Encapsulated Ps} {.eps}} {{All Files} {*}}}]; raise .filename ; if {[string length $fn1] == 0} {} {set fn $fn1}}");
//cmd(inter, "set fn \"Lsdplot.eps\"");
cmd(inter, "set fn \"$b.eps\"");
cmd(inter, "entry  .filename.e -width 40 -relief sunken -textvariable fn");
cmd(inter, "set cm \"color\"");
cmd(inter, "frame $r.r");
cmd(inter, "frame $r.r.col -bd 2");
cmd(inter, "radiobutton $r.r.col.r1 -text \"Color\" -variable cm -value color");
cmd(inter, "radiobutton $r.r.col.r2 -text \"Gray\" -variable cm -value gray");
cmd(inter, "pack $r.r.col.r1 $r.r.col.r2");

cmd(inter, "frame $r.r.pos -bd 2");
cmd(inter, "set res 1");
cmd(inter, "radiobutton $r.r.pos.p1 -text \"Landscape\" -variable res -value 1");
cmd(inter, "radiobutton $r.r.pos.p2 -text \"Portrait\" -variable res -value 2");
cmd(inter, "bind $r.r.pos.p1 <Button-1> {set dim 270}");
cmd(inter, "bind $r.r.pos.p2 <Button-1> {set dim 200}");

cmd(inter, "label $r.ldim -text \"Dimension (width in mm @96DPI)\"");
cmd(inter, "set dim 270");
cmd(inter, "entry $r.dim -width 4 -relief sunken -bd 2 -textvariable dim");
cmd(inter, "pack $r.r.pos.p1 $r.r.pos.p2");
cmd(inter, "pack $r.r.col $r.r.pos -side left");

cmd(inter, "set heightpost 390");
//cmd(inter, "checkbutton .filename.lab -text \"Include graph labels\" -variable heightpost -onvalue 390 -offvalue 325");
cmd(inter, "scan $it %d)%s a b");


cmd(inter, "checkbutton .filename.lab -text \"Include graph labels\" -variable heightpost -onvalue 1 -offvalue 0");
cmd(inter, "set choice $a");
if(graph_l[*choice]==graph_nl[*choice])
 cmd(inter, ".filename.lab conf -state disabled");

cmd(inter, "pack .filename.b.ok .filename.b.c -side left");
cmd(inter, "pack .filename.l .filename.lab .filename.r $r.ldim $r.dim .filename.e .filename.s .filename.b");
//cmd(inter, "raise .filename");
*choice=0;
  while(*choice==0)
	Tcl_DoOneEvent(0);

Tcl_UnlinkVar(inter, "res");

if(*choice==3)
 {cmd(inter, "bind .filename <Destroy> {}");
  cmd(inter, "destroy .filename");
  *choice=11;
  goto there;
 }
if(*choice==2)
 {*choice=0;
  cmd(inter, "bind .filename <Destroy> {}");
  cmd(inter, "destroy .filename");
  goto there;
 }

cmd(inter, "scan $it %d)%s a b");
cmd(inter, "set dd \"\"");
cmd(inter, "append dd $dim m");
cmd(inter, "set fn [file nativename $fn]"); //return the name in the platform specifi format

if(graph_l[*choice]==-1 )
 *choice=2;

if(*choice==1 )
{
cmd(inter, "set choice $a");
if(graph_l[*choice]==graph_nl[*choice])
 {
  if(graph_l[*choice]>0)
   {
    sprintf(msg, "set heightpost %d",graph_l[*choice]);
    cmd(inter, msg);
   } 
  else
   {
    cmd(inter, "set str [.f.new$a.f.plots conf -height]");
    cmd(inter," scan $str \"%s %s %s %s %d\" trash1 trash2 trash3 trash4 heighpost");

   } 
   
 }
else
 {
 sprintf(msg, "if {$heightpost == 1} {set heightpost %d} {set heightpost %d}", graph_l[*choice],graph_nl[*choice]);
 cmd(inter, msg);
 }  
if(rot==2)
  cmd(inter, ".f.new$a.f.plots postscript -height $heightpost -pagewidth $dd -colormode $cm -file \"$fn\"");
else
  cmd(inter, ".f.new$a.f.plots postscript -height $heightpost -pagewidth $dd -rotate true -colormode $cm -file \"$fn\"");
}
else
{
if(rot==2)
  cmd(inter, ".f.new$a.f.plots postscript -pagewidth $dd -colormode $cm -file \"$fn\"");
else
  cmd(inter, ".f.new$a.f.plots postscript -pagewidth $dd -rotate true -colormode $cm -file \"$fn\"");
}
  app=(char *)Tcl_GetVar(inter, "fn",0);

  cmd(inter, "bind .filename <Destroy> {}");
  cmd(inter, "destroy .filename");

 *choice=0;
 goto there;

//Plot_gnu AND phase diagram, depending on how many series are selected
case 17:
  *choice=1;
  cmd(inter, "set choice [.f.vars.ch.v size]");
  if(*choice==0) //No variables to plot defined
   { cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"No Series Selected.\\nPlace some of series available for the analysis\\nin the central listbox.\"");
    goto there;
   }
  cur_plot++;
  cmd(inter, ".b.ts conf -text Stop");

  cmd(inter, "set exttit \"$cur_plot) $tit\"");
  if(*choice>1)
  { *choice=0;
    plot_gnu(choice);
  }
  else
   {
   *choice=0;
   plot_phase_diagram(choice);
   }
  cmd(inter, ".b.ts conf -text \"Plot\"");
  if(*choice==2)
   {
   *choice=0;
   goto there;
   }

  cmd(inter, ".f.vars.pl.v insert end $exttit");
  type_graph[cur_plot]=1; //Gnuplot graph
  graph_l[cur_plot]=430; //height of graph with labels
  graph_nl[cur_plot]=430; //height of graph without labels  
  
  *choice=0;
  goto there;

//Plot_cs_xy
case 18:
  *choice=1;
  cmd(inter, "set choice [.f.vars.ch.v size]");
  if(*choice==0) //No variables to plot defined
   goto there;
  cur_plot++;
  cmd(inter, ".b.ts conf -text Stop");
  *choice=0;
  cmd(inter, "set exttit \"$cur_plot) $tit\"");
  plot_cs_xy(choice);
  cmd(inter, ".b.ts conf -text \"Plot\"");
  if(*choice==2)
    {*choice=0;
     goto there;
    }

  *choice=0;

  cmd(inter, ".f.vars.pl.v insert end $exttit");
  type_graph[cur_plot]=1; //gnuplot standard graph
  graph_l[cur_plot]=430; //height of graph with labels
  graph_nl[cur_plot]=430; //height of graph with labels  
  
  goto there;

//phase diagram //REMOVED, since P.D is activated elsewhere
case 19:
  *choice=1;
  cmd(inter, "set choice [.f.vars.ch.v size]");
  if(*choice==0) //No variables to plot defined
   goto there;
  cur_plot++;
  cmd(inter, ".b.ts conf -text Stop");
  *choice=0;
  cmd(inter, "set exttit \"$cur_plot) $tit\"");
  plot_phase_diagram(choice);
  cmd(inter, ".b.ts conf -text \"Plot\"");

  *choice=0;
  cmd(inter, ".f.vars.pl.v insert end $exttit");
  type_graph[cur_plot]=1; //Gnuplot graph
  graph_l[cur_plot]=430; //height of graph with labels
  graph_nl[cur_plot]=430; //height of graph with labels  

  goto there;

case 20:
//remove a graph
cmd(inter, "set answer [tk_messageBox -type yesno -title \"Delete Graph?\" -message \"Press Yes to delete graph:\\n$it\"]");
cmd(inter, "if {[string compare $answer \"yes\"] == 0} { set choice 1} {set choice 0}");
if(*choice==0)
 goto there;
cmd(inter, "scan $it %d)%s a b");
cmd(inter, "set ex [winfo exists .f.new$a]");
cmd(inter, "if { $ex == 1 } {destroy .f.new$a; .f.vars.pl.v delete $n_it} {.f.vars.pl.v delete $n_it}");
*choice=0;
goto there;


case 21:
//set personal colors
*choice=0;
cmd(inter, "toplevel .a -background white");
cmd(inter, "wm title .a \"\"");
cmd(inter, "wm transient .a .");

cmd(inter, "frame .a.l1");
cmd(inter, "frame .a.l2");
for(i=0; i<10; i++)
 {
  sprintf(msg, "label .a.l1.c%d -text \"%d) ___\" -fg $c%d  -bg white", i, i+1, i);
  cmd(inter, msg);
  sprintf(msg, "pack .a.l1.c%d -anchor e", i);
  cmd(inter, msg);
  sprintf(msg, "bind .a.l1.c%d <Button-1> {set n_col %d; set col $c%d; set choice 1}", i, i, i);
  cmd(inter, msg);
 }
for(i=0; i<10; i++)
 {
  sprintf(msg, "label .a.l2.c%d -text \"%d) ___\" -fg $c%d -bg white", i, i+11, i+10);
  cmd(inter, msg);
  sprintf(msg, "pack .a.l2.c%d -anchor e", i);
  cmd(inter, msg);
  sprintf(msg, "bind .a.l2.c%d <Button-1> {set n_col %d; set col $c%d; set choice 1}", i, i+10, i+10);
  cmd(inter, msg);
 }
cmd(inter, "pack .a.l1 .a.l2 -side left");
cmd(inter, "button .a.ok -text Ok -command {set choice 2}");
cmd(inter, "pack .a.ok -side bottom");
#ifndef DUAL_MONITOR
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
set_col:
  while(*choice==0)
	Tcl_DoOneEvent(0);

if(*choice==1)
 {

 cmd(inter, "set a [tk_chooseColor -initialcolor $col]");
 cmd(inter, "if {$a != \"\"} {set choice $n_col} {set choice -1}");
 if(*choice!=-1)
  {
   cmd(inter, "set c$n_col $a");
   cmd(inter, "if {$n_col >= 10 } {set fr 2; set n [expr $n_col-10]} {set fr 1; set n $n_col}");
   cmd(inter, ".a.l$fr.c$n conf -fg $a");
  }
 *choice=0;
// cmd(inter, "raise .a");
 goto set_col;
 }

*choice=0;
cmd(inter, "destroy .a");
goto there;


case 22:
//set default colors
init_canvas();
*choice=0;
goto there;

case 23:
/*
plot a lattice. Data must be stored on a single time step organized for lines and columns in
sequence

*/
  *choice=1;
  cmd(inter, "set choice [.f.vars.ch.v size]");
  if(*choice==0) //No variables to plot defined
   {
   cmd(inter, "tk_messageBox -type ok -title \"No Series\" -message \"No series selected.\\nPlace some series in the Series Selected listbox.\"");
   goto there;
   }
  cur_plot++;
  *choice=0;
  cmd(inter, "set exttit \"$cur_plot) $tit\"");
  plot_lattice(choice);
  cmd(inter, "destroy .s");
  if(*choice==0)
    {

    cmd(inter, ".f.vars.pl.v insert end $exttit");
    type_graph[cur_plot]=3; //Lattice canvas standard graph
    graph_l[cur_plot]=-1; //height of graph with labels
    graph_nl[cur_plot]=-1; //height of graph with labels  

    }
  else
    *choice=0;
  goto there;

case 24:
/*
Insert series not saved from the current model.
*/
  i=0;

cmd(inter, "set choice [.f.vars.ch.v size]");
cmd(inter, "toplevel .s");
cmd(inter, "wm title .s \"Choose data source\"");
cmd(inter, "wm transient .s .");
cmd(inter, "label .s.l -text \"Select the source of additional series\"");
cmd(inter, "set bidi 0");
cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "radiobutton .s.i.m -text \"Current model configuration\" -variable bidi -value 0");
cmd(inter, "bind .s.i.m <Down> {set bidi 1; focus -force .s.i.f}");

cmd(inter, "radiobutton .s.i.f -text \"File(s) of saved results\" -variable bidi -value 1");
if(*choice>0)
  cmd(inter, "bind .s.i.f <Down> {set bidi 5; focus -force .s.i.a}");
cmd(inter, "bind .s.i.f <Up> {set bidi 0; focus -force .s.i.m}");


cmd(inter, "radiobutton .s.i.a -text \"Create Mov.Average Series from selected\" -variable bidi -value 5");
cmd(inter, "bind .s.i.a <Down> {set bidi 4; focus -force .s.i.c}");
cmd(inter, "bind .s.i.a <Up> {set bidi 1; focus -force .s.i.f}");


cmd(inter, "radiobutton .s.i.c -text \"Create One Serie from selected series\" -variable bidi -value 4");
cmd(inter, "bind .s.i.c <Up> {set bidi 5; focus -force .s.i.a}");

if(*choice>0)
 cmd(inter, "pack .s.i.m .s.i.f .s.i.a .s.i.c -anchor w");
else
 cmd(inter, "pack .s.i.m .s.i.f  -anchor w");

cmd(inter, "pack .s.l .s.i");
cmd(inter, "button .s.ok -text Ok -command {set choice 1}");
cmd(inter, "button .s.esc -text Cancel -command {set choice 2}");
cmd(inter, "button .s.help -text \" Help \" -command {LsdHelp mdatares.html#add_series}");

cmd(inter, "pack .s.i .s.ok .s.esc .s.help");
cmd(inter, "bind .s <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
*choice=0;
cmd(inter, "focus -force .s.i.m");
  while(*choice==0)
	Tcl_DoOneEvent(0);
cmd(inter, "destroy .s");
if(*choice==2)
 {*choice=0;
  goto there;
 }

cmd(inter, "set choice $bidi");
if(*choice==4)
 {
  *choice=0;
  create_series(choice);
  *choice=0;
  goto there;
 }
if(*choice==5)
 {
  *choice=0;
  create_maverag(choice);
  *choice=0;
  goto there;
 } 
if(*choice==1 || *choice==3)
 {
 
  cmd(inter, "set lab [tk_getOpenFile -multiple yes -initialdir [pwd] -filetypes {{{Lsd Result Files} {.res}} {{Lsd Total Files} {.tot}} {{All Files} {*}} }]");
  cmd(inter, "set choice [llength $lab]");
  if(*choice==0 )
   {//no file selected
    //cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"No files matching pattern '$nfiles' found.\"");
    goto there; 
   }
  h=*choice;
  
  for(i=0; i<h; i++)  
  {
  sprintf(msg, "set datafile [lindex $lab %d]", i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "datafile",0);
  strcpy(filename,app);
  f=fopen(filename, "r");

  if(f!=NULL)
   {fclose(f);
    file_counter++;
    insert_data_file(&num_var, &num_c);
    sprintf(msg, ".f.com.nvar conf -text \"Series = %d\"",num_var);
    cmd(inter,msg);
    sprintf(msg, ".f.com.ncas conf -text \"Cases = %d\"", num_c);
    cmd(inter,msg);
   }
  }
  *choice=0;
  goto there;
 
 }
cmd(inter, "toplevel .s");
cmd(inter, "wm title .s \"Insert series\"");
cmd(inter, "wm transient .s .");
cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "label .s.i.l -text \"Type the label of the series to insert (self-completion)\"");
cmd(inter, "set bidi \"\"");
cmd(inter, "entry .s.i.e -textvariable bidi");
cmd(inter, "pack .s.i.l .s.i.e");
cmd(inter, "button .s.ok -text Ok -command {set choice 1}");
cmd(inter, "button .s.esc -text Cancel -command {set choice 2}");

cmd(inter, "pack .s.i .s.ok .s.esc");
cmd(inter, "bind .s.i.e <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
*choice=0;
cmd(inter, "focus .s.i.e");
cmd(inter, "bind .s.i.e <KeyRelease> {if { %N < 256} { set b [.s.i.e index insert]; set c [.s.i.e get]; set f [lsearch -glob $ModElem $c*]; if { $f !=-1 } {set d [lindex $ModElem $f]; .s.i.e delete 0 end; .s.i.e insert 0 $d; .s.i.e index $b; .s.i.e selection range $b end } { } } { } }");
  while(*choice==0)
	Tcl_DoOneEvent(0);
cmd(inter, "destroy .s");
if(*choice==2)
 {*choice=0;
  goto there;
 }
cmd(inter, "toplevel .s");
cmd(inter, "wm transient .s .");
cmd(inter, "label .s.l -text \"Inserting new series\"");
cmd(inter, "pack .s.l");
app=(char *)Tcl_GetVar(inter, "bidi",0);
strcpy(lab,app);
insert_data_nosave(root, lab, &i);
cmd(inter, "destroy .s");
cmd(inter, "set tit [.f.vars.ch.v get 0]");
*choice=0;
goto there;

case 30:
/*
Use right button of the mouse to move all series 
with a given label
*/

cmd(inter, "if {[info exist compvalue] == 1 } {} {set compvalue 1}");
Tcl_LinkVar(inter, "compvalue", (char *) &compvalue, TCL_LINK_DOUBLE);
cmd(inter, "set a [split $res]");
cmd(inter, "set b [lindex $a 0]");
cmd(inter, "set c [lindex $a 1]"); //get the tag value
cmd(inter, "set i [llength [split $c {_}]]");
cmd(inter, "set ntag $i");
cmd(inter, "set ssys 2");
cmd(inter, "toplevel .a");
cmd(inter, "wm title .a \"Add a batch of items\"");
cmd(inter, "wm transient .a .");

cmd(inter, "frame .a.tit  -relief groove -bd 2");
cmd(inter, "label .a.tit.l -text \"Select series with label : \"");
cmd(inter, "label .a.tit.s -text \"$b\" -foreground red");
cmd(inter, "pack .a.tit.l .a.tit.s -side left");

cmd(inter, "frame .a.f1 -relief groove -bd 2");
cmd(inter, "radiobutton .a.f1.c -text \"Select all the series\" -variable ssys -value 2");
cmd(inter, "bind .a.f1.c <Down> {focus -force .a.f.c; .a.f.c invoke}");
cmd(inter, "bind .a.f1.c <Return> {.a.f1.c invoke; focus -force .a.b.ok}");
cmd(inter, "pack .a.f1.c -anchor w");


cmd(inter, "frame .a.f2 -relief groove -bd 2");
cmd(inter, "radiobutton .a.f2.s -text \"Select for values of another series\" -variable ssys -value 3");
cmd(inter, "bind .a.f2.s <Up> {focus -force .a.f.c; .a.f.c invoke}");
cmd(inter, "bind .a.f2.s <Return> {focus -force .a.f2.f.e; .a.f2.f.e selection range 0 end}");
cmd(inter, "frame .a.f2.f");
cmd(inter, "label .a.f2.f.l -text \"Label: \"");
cmd(inter, "entry .a.f2.f.e -width 20 -relief sunken -textvariable svar");
cmd(inter, "bind .a.f2.f.e <KeyRelease> {if { %N < 256} { set bb1 [.a.f2.f.e index insert]; set bc1 [.a.f2.f.e get]; set bf1 [lsearch -glob $ModElem $bc1*]; if { $bf1 !=-1 } {set bd1 [lindex $ModElem $bf1]; .a.f2.f.e delete 0 end; .a.f2.f.e insert 0 $bd1; .a.f2.f.e index $bb1; .a.f2.f.e selection range $bb1 end } { } } { } }");
cmd(inter, "bind .a.f2.f.e <Return> {focus -force .a.f2.f.e1; .a.f2.f.e1 selection range 0 end}");
cmd(inter, "label .a.f2.f.t -text \"Time step: \"");
cmd(inter, "if { [info exist tvar]==1} {} {set tvar 0}");
cmd(inter, "entry .a.f2.f.e1 -width 7 -relief sunken -textvariable tvar");
cmd(inter, "bind .a.f2.f.e1 <Return> {focus -force .a.f2.c.e; .a.f2.c.e selection range 0 end }");
cmd(inter, "pack .a.f2.f.l .a.f2.f.e .a.f2.f.t .a.f2.f.e1 -side left");
cmd(inter, "frame .a.f2.c");
cmd(inter, "label .a.f2.c.l -text \"Comparison value: \"");
cmd(inter, "entry .a.f2.c.e -width 12 -relief sunken -textvariable compvalue");
cmd(inter, "bind .a.f2.c.e <Return> {focus -force .a.b.ok}");
cmd(inter, "pack .a.f2.c.l .a.f2.c.e -side left");
cmd(inter, "pack .a.f2.s .a.f2.f .a.f2.c -anchor w");


cmd(inter, "frame .a.f -relief groove -bd 2");
cmd(inter, "radiobutton .a.f.c -text \"Select for series' tags\" -variable ssys -value 1");
cmd(inter, "bind .a.f.c <Up> {focus -force .a.f1.c; .a.f1.c invoke}");
cmd(inter, "bind .a.f.c <Down> {focus -force .a.f2.s; .a.f2.s invoke}");
cmd(inter, "pack .a.f.c -anchor w");
cmd(inter, "for {set x 0} {$x<$i} {incr x} {	if {$x > 0} {label .a.f.s$x -text \" - \"} {}; entry .a.f.e$x -width 4 -relief sunken -textvariable v$x}");
cmd(inter, "for {set x 0} {$x<$i} {incr x} {	if {$x > 0} {pack .a.f.s$x -side left} {}; pack .a.f.e$x -side left; bind .a.f.e$x <Return> {focus -force .a.b.ok}}");


cmd(inter, "frame .a.c -relief groove -bd 2");
cmd(inter, "label .a.c.l -text \"Set the condition to meet\"");
cmd(inter, "radiobutton .a.c.eq -text \"Equal to: =\" -variable cond -value 1");
cmd(inter, "radiobutton .a.c.geq -text \"Larger or equal to: >=\" -variable cond -value 2");
cmd(inter, "radiobutton .a.c.g -text \"Larger: >\" -variable cond -value 3");
cmd(inter, "radiobutton .a.c.seq -text \"Smaller or equal to <=\" -variable cond -value 4");
cmd(inter, "radiobutton .a.c.s -text \"Smaller: <\" -variable cond -value 5");
cmd(inter, "pack .a.c.l .a.c.eq .a.c.geq .a.c.g .a.c.seq .a.c.s -anchor w");
cmd(inter, "set cond 1");


cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -text \" Ok \" -command {set choice 1}");

cmd(inter, "button .a.b.esc -text \" Cancel \" -command {set choice 2}");
cmd(inter, "button .a.b.help -text \" Help \" -command {LsdHelp mdatares.html#batch_sel}");
cmd(inter, "pack .a.b.ok .a.b.esc .a.b.help -side left");

cmd(inter, "pack .a.tit .a.f1 .a.f .a.f2 .a.c -anchor w -expand yes -fill x");
cmd(inter, "pack .a.b");
*choice=0;

cmd(inter, "focus -force .a.f1.c; .a.f.e0 selection range 0 end");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");

  while(*choice==0)
	Tcl_DoOneEvent(0);

Tcl_UnlinkVar(inter, "compvalue");
if(*choice==2)
{
*choice=0;

cmd(inter, "destroy .a");
goto there;

}

cmd(inter, "if {[.f.vars.ch.v get 0] == \"\"} {set tit \"\"} {}");
cmd(inter, "set choice $ssys");
if(*choice==2)
 {
 cmd(inter, "destroy .a");
 cmd(inter, "set tot [.f.vars.lb.v get 0 end]");
 cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} {  .f.vars.ch.v insert end \"$i\"  } {}}");
 cmd(inter, "if {\"$tit\" == \"\"} {set tit [.f.vars.ch.v get 0]} {}");
 cmd(inter, ".f.vars.b.out conf -state normal");
 *choice=0;
 goto there;
 }
 
if(*choice== 1)
{
cmd(inter, "set choice $cond");
i=*choice;

cmd(inter, ".f.vars.b.out conf -state normal");
*choice=-1;
cmd(inter, "for {set x 0} {$x<$i} {incr x} {	if {[.a.f.e$x get]!=\"\"} {set choice $x; set xval [.a.f.e$x get]} {}}");
if(*choice==-1)
{
cmd(inter, "set tot [.f.vars.lb.v get 0 end]");
cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} {  .f.vars.ch.v insert end \"$i\"  } {}}");
}
else
 {
// plog("I want all the $b variables with $xval in position $choice\n");
/*
NEW CODE (12 - 11 - 2007)
allowed to insert multiple conditions for each digit in the tag. Selection values from the cells are copied into 
a tcl list and recalled with lindex.

The cases scan each entry with the proper label and remove those entries that have a non-null entry in a cell not satisfying the indicated condition.

*/
 cmd(inter, "set tot [.f.vars.lb.v get 0 end]");
cmd(inter, "if { [info exist vcell]==1} {unset vcell} {}");
cmd(inter, "set choice $ntag");
for(j=0; j<*choice; j++)
 {
 sprintf(msg, "lappend vcell $v%d", j);
 cmd(inter, msg);
 }

switch(i) 
{
case 1:
// cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] == $xval } {  .f.vars.ch.v insert end \"$i\"  } {}}");

  cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] != [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .f.vars.ch.v insert end \"$i\"  } {}} {}}");
 break;
case 2:
// cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] >= $xval } {  .f.vars.ch.v insert end \"$i\"  } {}}");
   cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] > [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .f.vars.ch.v insert end \"$i\"  } {}} {}}");
 break;
case 3:
 //cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] > $xval } {  .f.vars.ch.v insert end \"$i\"  } {}}");
   cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] >= [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .f.vars.ch.v insert end \"$i\"  } {}} {}}");
 break;
case 4:
 //cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] <= $xval } {  .f.vars.ch.v insert end \"$i\"  } {}}");
   cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] < [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .f.vars.ch.v insert end \"$i\"  } {}} {}}");
 break;
case 5:
 //cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] < $xval } {  .f.vars.ch.v insert end \"$i\"  } {}}");
   cmd(inter, "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] <= [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .f.vars.ch.v insert end \"$i\"  } {}} {}}");
 break;
} 
 
 
 }
 
 
cmd(inter, "if {\"$tit\" == \"\"} {set tit [.f.vars.ch.v get 0]} {}");
cmd(inter, ".f.vars.b.out conf -state normal");
cmd(inter, "destroy .a");
*choice=0;
goto there;
}


if(*choice==3)
{

cmd(inter, ".f.vars.b.out conf -state normal");
cmd(inter, "set choice $cond");
p=*choice;
cmd(inter, "set tot [.f.vars.lb.v get 0 end]");
//cmd(inter, "set tot1 [.f.vars.lb.v get 0 end]");
cmd(inter, "set choice [llength $tot]");
j=*choice;
app=(char *)Tcl_GetVar(inter, "svar",0);
strcpy(str3,app);

cmd(inter, "set choice $tvar");
h=*choice;

for(i=0; i<j; i++)
 {

  sprintf(msg, "set res [lindex $tot %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str1, str2, &l, &m, &k);
  if(h>=l && h<=m && !strcmp(str1, str3))
   {
   datum=find_data(k);
   r=0;
   if(!isnan(datum[h]))		// ignore NaNs
    switch(p)
    {
    case 1: if(datum[h]==compvalue) r=1;
    break;
    case 2: if(datum[h]>=compvalue) r=1;
     break;
    case 3: if(datum[h] > compvalue) r=1;
     break;
    case 4: if(datum[h]<=compvalue) r=1;
     break;
    case 5: if(datum[h]<=compvalue) r=1;
     break;
        
    }
    if(r==1)
     { 
      cmd(inter, "set templ $tot");
      sprintf(msg, "set choice [lsearch $templ \"$b %s *\"]", str2);
      cmd(inter, msg);

      while(*choice>=0)
       {
       cmd(inter, ".f.vars.ch.v insert end [lindex $templ $choice]");
       cmd(inter, "set templ [lreplace $templ $choice $choice]");
       cmd(inter, msg);
       
       }
      
     /*
      sprintf(msg, "set found [lsearch $tot \"$b %s*\"]", str2);
      cmd(inter, msg);
      cmd(inter, "set choice $found ");
      if(*choice>=0)
       {
        sprintf(msg, "set iddu [lsearch $tot \"$b %s*\"]",str2);
        cmd(inter, msg);
        cmd(inter, ".f.vars.ch.v insert end [.f.vars.lb.v get $iddu]");
        
       }
      */ 
     }
   }

 }
cmd(inter, "if {\"$tit\" == \"\"} {set tit [.f.vars.ch.v get 0]} {}");
cmd(inter, "destroy .a");

}








*choice=0;
goto there;

case 33:
/*
Use right button of the mouse to remove series selected
with different criteria
*/

Tcl_LinkVar(inter, "compvalue", (char *) &compvalue, TCL_LINK_DOUBLE);
cmd(inter, "set a [split $res]");
cmd(inter, "set b [lindex $a 0]");
cmd(inter, "set c [lindex $a 1]"); //get the tag value
cmd(inter, "set i [llength [split $c {_}]]");
cmd(inter, "set ssys 2");
cmd(inter, "toplevel .a");
cmd(inter, "wm title .a \"Remove a batch of items\"");
cmd(inter, "wm transient .a .");

cmd(inter, "frame .a.tit  -relief groove -bd 2");
cmd(inter, "label .a.tit.l -text \"Select series with label : \"");
cmd(inter, "label .a.tit.s -text \"$b\" -foreground red");
cmd(inter, "pack .a.tit.l .a.tit.s -side left");

cmd(inter, "frame .a.f1 -relief groove -bd 2");
cmd(inter, "radiobutton .a.f1.c -text \"Select all the series\" -variable ssys -value 2");
cmd(inter, "bind .a.f1.c <Down> {focus -force .a.f.c; .a.f.c invoke}");
cmd(inter, "bind .a.f1.c <Return> {.a.f1.c invoke; focus -force .a.b.ok}");
cmd(inter, "pack .a.f1.c -anchor w");


cmd(inter, "frame .a.f2 -relief groove -bd 2");
cmd(inter, "radiobutton .a.f2.s -text \"Select for values of another series\" -variable ssys -value 3");
cmd(inter, "bind .a.f2.s <Up> {focus -force .a.f.c; .a.f.c invoke}");
cmd(inter, "bind .a.f2.s <Return> {focus -force .a.f2.f.e; .a.f2.f.e selection range 0 end}");
cmd(inter, "frame .a.f2.f");
cmd(inter, "label .a.f2.f.l -text \"Label: \"");
cmd(inter, "entry .a.f2.f.e -width 20 -relief sunken -textvariable svar");
cmd(inter, "bind .a.f2.f.e <KeyRelease> {if { %N < 256} { set bb1 [.a.f2.f.e index insert]; set bc1 [.a.f2.f.e get]; set bf1 [lsearch -glob $ModElem $bc1*]; if { $bf1 !=-1 } {set bd1 [lindex $ModElem $bf1]; .a.f2.f.e delete 0 end; .a.f2.f.e insert 0 $bd1; .a.f2.f.e index $bb1; .a.f2.f.e selection range $bb1 end } { } } { } }");
cmd(inter, "bind .a.f2.f.e <Return> {focus -force .a.f2.f.e1; .a.f2.f.e1 selection range 0 end}");
cmd(inter, "label .a.f2.f.t -text \"Time step: \"");
cmd(inter, "if { [info exist tvar]==1} {} {set tvar 0}");
cmd(inter, "entry .a.f2.f.e1 -width 7 -relief sunken -textvariable tvar");
cmd(inter, "bind .a.f2.f.e1 <Return> {focus -force .a.f2.c.e; .a.f2.c.e selection range 0 end }");
cmd(inter, "pack .a.f2.f.l .a.f2.f.e .a.f2.f.t .a.f2.f.e1 -side left");
cmd(inter, "frame .a.f2.c");
cmd(inter, "label .a.f2.c.l -text \"Comparison value: \"");
cmd(inter, "if {[info exist comvalue]==1} {} {set compvalue 1}");
cmd(inter, "entry .a.f2.c.e -width 12 -relief sunken -textvariable compvalue");
cmd(inter, "bind .a.f2.c.e <Return> {focus -force .a.b.ok}");
cmd(inter, "pack .a.f2.c.l .a.f2.c.e -side left");
cmd(inter, "pack .a.f2.s .a.f2.f .a.f2.c -anchor w");


cmd(inter, "frame .a.f -relief groove -bd 2");
cmd(inter, "radiobutton .a.f.c -text \"Select for series' tags\" -variable ssys -value 1");
cmd(inter, "bind .a.f.c <Up> {focus -force .a.f1.c; .a.f1.c invoke}");
cmd(inter, "bind .a.f.c <Down> {focus -force .a.f2.s; .a.f2.s invoke}");
cmd(inter, "pack .a.f.c -anchor w");
cmd(inter, "for {set x 0} {$x<$i} {incr x} {	if {$x > 0} {label .a.f.s$x -text \" - \"} {}; entry .a.f.e$x -width 4 -relief sunken -textvariable v$x}");
cmd(inter, "for {set x 0} {$x<$i} {incr x} {	if {$x > 0} {pack .a.f.s$x -side left} {}; pack .a.f.e$x -side left; bind .a.f.e$x <Return> {focus -force .a.b.ok}}");


cmd(inter, "frame .a.c -relief groove -bd 2");
cmd(inter, "label .a.c.l -text \"Set the condition to meet\"");
cmd(inter, "radiobutton .a.c.eq -text \"Equal to: =\" -variable cond -value 1");
cmd(inter, "radiobutton .a.c.geq -text \"Larger or equal to: >=\" -variable cond -value 2");
cmd(inter, "radiobutton .a.c.g -text \"Larger: >\" -variable cond -value 3");
cmd(inter, "radiobutton .a.c.seq -text \"Smaller or equal to <=\" -variable cond -value 4");
cmd(inter, "radiobutton .a.c.s -text \"Smaller: <\" -variable cond -value 5");
cmd(inter, "pack .a.c.l .a.c.eq .a.c.geq .a.c.g .a.c.seq .a.c.s -anchor w");
cmd(inter, "set cond 1");


cmd(inter, "frame .a.b");
cmd(inter, "button .a.b.ok -text \" Ok \" -command {set choice 1}");

cmd(inter, "button .a.b.esc -text \" Cancel \" -command {set choice 2}");
cmd(inter, "button .a.b.help -text \" Help \" -command {LsdHelp mdatares.html#batch_sel}");
cmd(inter, "pack .a.b.ok .a.b.esc .a.b.help -side left");

cmd(inter, "pack .a.tit .a.f1 .a.f .a.f2 .a.c -anchor w -expand yes -fill x");
cmd(inter, "pack .a.b");
*choice=0;

cmd(inter, "focus -force .a.f1.c; .a.f.e0 selection range 0 end");
cmd(inter, "bind .a.b.ok <Return> {.a.b.ok invoke}");
cmd(inter, "bind .a <Escape> {.a.b.esc invoke}");

  while(*choice==0)
	Tcl_DoOneEvent(0);

if(*choice==2)
{
*choice=0;

cmd(inter, "destroy .a");
goto there;

}

//cmd(inter, "if {[.f.vars.ch.v get 0] == \"\"} {set tit \"\"} {}");
cmd(inter, "set choice $ssys");
if(*choice==2)
 {
 cmd(inter, "destroy .a");
 cmd(inter, "set tot [.f.vars.ch.v get 0 end]");
 cmd(inter, "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} {  .f.vars.ch.v selection set $myc  } {}; incr myc}");
// cmd(inter, "if {\"$tit\" == \"\"} {set tit [.f.vars.ch.v get 0]} {}");
 cmd(inter, ".f.vars.b.out conf -state normal");
 *choice=0;
 goto there;
 }
 
if(*choice== 1)
{
cmd(inter, "set choice $cond");
i=*choice;

cmd(inter, ".f.vars.b.out conf -state normal");
*choice=-1;
cmd(inter, "for {set x 0} {$x<$i} {incr x} {	if {[.a.f.e$x get]!=\"\"} {set choice $x; set xval [.a.f.e$x get]} {}}");
if(*choice==-1)
{
cmd(inter, "set tot [.f.vars.ch.v get 0 end]");
cmd(inter, "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} {  .f.vars.ch.v selection set $myc } {}; incr myc}");
}
else
 {
// plog("I want all the $b variables with $xval in position $choice\n");


cmd(inter, "set tot [.f.vars.ch.v get 0 end]");
/*
 TO BE UPDATED along the lines for series' selection. Too boring, will do it, eventually.
 
 
cmd(inter, "if { [info exist vcell]==1} {unset vcell} {}");
cmd(inter, "set choice $ntag");
for(j=0; j<*choice; j++)
 {
 sprintf(msg, "lappend vcell $v%d", j);
 cmd(inter, msg);
 }
*/
switch(i) 
{
case 1:
//  cmd(inter, "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} {.log.text.text insert end \"$x - [lindex $vcell $x] - [lindex [split [lindex [split $i] 1] {_}] $x]\n \"; if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] != [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .f.vars.ch.v selection set $myc; incr myc  } {}} {}}");
 cmd(inter, "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] == $xval } {  .f.vars.ch.v selection set $myc  } {}; incr myc}");
 break;
case 2:
 cmd(inter, "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] >= $xval } {  .f.vars.ch.v selection set $myc  } {}; incr myc}");
 break;
case 3:
 cmd(inter, "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] > $xval } {  .f.vars.ch.v selection set $myc  } {}; incr myc}");
 break;
case 4:
 cmd(inter, "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] <= $xval } {  .f.vars.ch.v selection set $myc  } {}; incr myc}");
 break;
case 5:
 cmd(inter, "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\" && [lindex [split [lindex [split $i] 1] {_}] $choice] < $xval } {  .f.vars.ch.v selection set $myc  } {}; incr myc}");
 break;
} 
 
 
 }
 
 
//cmd(inter, "if {\"$tit\" == \"\"} {set tit [.f.vars.ch.v get 0]} {}");
cmd(inter, ".f.vars.b.out conf -state normal");
cmd(inter, "destroy .a");
*choice=0;
goto there;
}


if(*choice==3)
{

cmd(inter, ".f.vars.b.out conf -state normal");
cmd(inter, "set choice $cond");
p=*choice;
cmd(inter, "set tot [.f.vars.ch.v get 0 end]");
//cmd(inter, "set tot1 [.f.vars.lb.v get 0 end]");
cmd(inter, "set choice [llength $tot]");
j=*choice;
app=(char *)Tcl_GetVar(inter, "svar",0);
strcpy(str3,app);

cmd(inter, "set choice $tvar");
h=*choice;

for(i=0; i<j; i++)
 {

  sprintf(msg, "set res [lindex $tot %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str1, str2, &l, &m, &k);
  if(h>=l && h<=m && !strcmp(str1, str3))
   {
   datum=find_data(k);
   r=0;
   if(!isnan(datum[h]))		// ignore NaNs
    switch(p)
    {
    case 1: if(datum[h]==compvalue) r=1;
    break;
    case 2: if(datum[h]>=compvalue) r=1;
     break;
    case 3: if(datum[h] > compvalue) r=1;
     break;
    case 4: if(datum[h]<=compvalue) r=1;
     break;
    case 5: if(datum[h]<=compvalue) r=1;
     break;
        
    }
    if(r==1)
     { 
      sprintf(msg, "set found [lsearch $tot \"$b %s*\"]", str2);
      cmd(inter, msg);
      cmd(inter, "set choice $found ");
      if(*choice>=0)
       {
        sprintf(msg, "set iddu [lsearch $tot \"$b %s*\"]",str2);
        cmd(inter, msg);
//        cmd(inter, ".f.vars.ch.v insert end [.f.vars.lb.v get $iddu]");
        cmd(inter, ".f.vars.ch.v selection set $iddu");
        
       }
     }
   }

 }
//cmd(inter, "if {\"$tit\" == \"\"} {set tit [.f.vars.ch.v get 0]} {}");
cmd(inter, "destroy .a");
Tcl_UnlinkVar(inter, "compvalue");
}








*choice=0;
goto there;



case 32:
*choice=0;
cur_plot++;
cmd(inter, "set exttit \"$cur_plot) $tit\"");
cmd(inter, ".f.vars.pl.v insert end $exttit");
cmd(inter, "set choice $tc");
if(*choice==1)
 {*choice=0;
  histograms(choice);
 } 
else
 {*choice=0;
  histograms_cs(choice);
 } 

*choice=0;

type_graph[cur_plot]=0; //Lsd standard graph
graph_l[cur_plot]=325; //height of graph with labels
graph_nl[cur_plot]=325; //height of graph with labels  

goto there;


default:
*choice=0;
 goto there;
}

}

/***************************************************
PLOT
****************************************************/
void plot(int *choice)
{

char *app;
char **str, **tag;
int idseries;


int i, nv, j, k, *start, *end, done, doney2, color,  numy2;
double x1,x01, *y1, x2, x02, *y2,  cminy, cmaxy, miny2, maxy2, truemaxy2;
double step;
Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");
double **data,**logdata;

if(nv>1000)
 {
  cmd(inter, "set answer [tk_messageBox -type yesno -message \"You selected $nv series to be plotted. So many series may cause a crash of the Lsd model program, with the loss of all data.\nIf you continue the system may become extremely slow.\nDo you want to continue anyway?\"]" );
  *choice=0;
  cmd(inter, "if {[string compare $answer \"yes\"] == 0} {set choice 1 } {set choice 22}");
  if(*choice==22)
   return;
  
 }
if(nv==0)
 return;

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];


str=new char *[nv];
tag=new char *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[100];
  tag[i]=new char[100];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {

   data[i]=find_data(idseries);
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!isnan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 sprintf(msg,"\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n",i+1,j);
		 plog(msg);
	   }
	 data[i]=logdata[i];				// replace the data series
   }
  }

 }


y1=new double[nv];
y2=new double[nv];

if(autom_x==1||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}


cmd(inter, "set choice $y2");
if(*choice==1)
 {
  cmd(inter, "set choice $numy2");
  numy2=*choice;
  if(numy2>nv||numy2<2)
   numy2=nv+3;
 }
else
 numy2=nv+3; 
 
numy2--;
 
if(autom==1||miny>=maxy)
{

miny2=miny;
maxy2=maxy;
for(done=doney2=0, i=0; i<nv; i++)
 {
  if(i<numy2)
  {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && !isnan(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
  }
  else
  {
  for(j=min_c; j<=max_c; j++)
   {
    if(doney2==0 && start[i]<=j && end[i]>=j && !isnan(data[i][j]))		// ignore NaNs
     {miny2=maxy2=data[i][j];
      doney2=1;
     }
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]<miny2 )		// ignore NaNs
     miny2=data[i][j];
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]>maxy2 )		// ignore NaNs
     maxy2=data[i][j];

   }
  
  }
   
 }


} //End for finding min-max


if(miny==maxy) //To avoid divisions by zero for constant values
 {if(miny==0)
   {maxy=0.1;
    miny=-0.1;
   }
  else
   {if(miny>0)
    {miny*=0.9;
     maxy*=1.1;
    }
    else
    {miny*=1.1;
     maxy*=0.9;
    }

   }
 }


if(miny2==maxy2) //To avoid divisions by zero for constant values
 {if(miny2==0)
   {maxy2=0.1;
    miny2=-0.1;
   }
  else
   {if(miny2>0)
    {miny2*=0.9;
     maxy2*=1.1;
    }
    else
    {miny2*=1.1;
     maxy2*=0.9;
    }

   }
 }

sprintf(msg, "set miny %lf", miny);
cmd(inter, msg);

sprintf(msg, "set maxy %lf", maxy);
cmd(inter, msg);
sprintf(msg, "set minc %d", min_c);
cmd(inter, msg);
sprintf(msg, "set maxc %d", max_c);
cmd(inter, msg);
truemaxy=maxy;
maxy+=(maxy-miny)/100; //The f... Windows does not plot the very firt pixels...

if(numy2!=nv+2)
 {truemaxy2=maxy2;
  maxy2+=(maxy2-miny2)/100;
 } 
sprintf(msg, "set p .f.new%d", cur_plot);
cmd(inter, msg);

cmd(inter, "toplevel $p");
cmd(inter, "wm title $p $tit");
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/lsd.xbm} {}");
cmd(inter, "bind $p <Double-Button-1> { raise .; focus -force .b.ts}");
sprintf(msg, "wm protocol $p WM_DELETE_WINDOW { wm withdraw .f.new%d}", cur_plot);
cmd(inter, msg);
cmd(inter, "frame $p.f -width 640 -height 430");
cmd(inter, "pack $p.f");

//Reset p to point to the canvas (shit...)
cmd(inter, "set p $p.f.plots");

cmd(inter, "canvas $p -width 640 -height 430 -background white -relief flat");
cmd(inter, "pack $p");

cmd(inter, "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p");

if(grid==1)
{
cmd(inter, "$p create line 40 0 40 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 190 0 190 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 340 0 340 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 490 0 490 310 -fill grey60 -width 1p -tag p");
}
else
{
cmd(inter, "$p create line 640 295 640 305  -width 1 -tag p");
cmd(inter, "$p create line 190 295 190 305 -width 1 -tag p");
cmd(inter, "$p create line 340 295 340 305 -width 1 -tag p");
cmd(inter, "$p create line 490 295 490 305 -width 1 -tag p");
}


sprintf(msg, "set choice [$p create text 40 312 -font {Times 10 normal} -anchor nw -text %d -tag {p text}]",min_c);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 190 312 -font {Times 10 normal} -anchor n -text %d -tag {p text}]",min_c+(max_c-min_c)/4);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 340 312 -font {Times 10 normal} -anchor n -text %d -tag {p  text}]",min_c+(max_c-min_c)/2);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 490 312 -font {Times 10 normal} -anchor n -text %d -tag {p text}]",min_c+(max_c-min_c)*3/4);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 635 312 -font {Times 10 normal} -anchor ne -text %d -tag {p  text}]",max_c);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


cmd(inter, "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p");

if(grid==1)
{
cmd(inter, "$p create line 38 300 640 300 -fill grey60 -tag p");
cmd(inter, "$p create line 38 225 640 225 -fill grey60 -tag p");
cmd(inter, "$p create line 38 150 640 150 -fill grey60 -tag p");
cmd(inter, "$p create line 38 75 640 75 -fill grey60 -tag p");
cmd(inter, "$p create line 38 0 640 0 -fill grey60 -tag p");
}
else
{
//cmd(inter, "$p create line 35 300 45 300  -tag p");
cmd(inter, "$p create line 35 225 45 225  -tag p");
cmd(inter, "$p create line 35 150 45 150  -tag p");
cmd(inter, "$p create line 35 75 45 75  -tag p");
cmd(inter, "$p create line 35 2 45 2 -tag p");
}

sprintf(msg, "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,miny);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p  text}]",pdigits,(miny+(truemaxy-miny)/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,(miny+(truemaxy-miny)/2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,(miny+(truemaxy-miny)*3/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]",pdigits,(truemaxy));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

if(numy2!=nv+2)
 {
sprintf(msg, "set choice [$p create text 4 312 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]",pdigits,miny2);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 237 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]",pdigits,(miny2+(truemaxy2-miny2)/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 162 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]",pdigits,(miny2+(truemaxy2-miny2)/2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 87 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]",pdigits,(miny2+(truemaxy2-miny2)*3/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 16 -font {Times 10 normal} -anchor nw -text (%.*g) -tag {p text}]",pdigits,(truemaxy2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
 
 }
cmd(inter, "set xlabel 4");
cmd(inter, "set ylabel 0");
for(i=0, j=4, k=0, color=0; i<nv && k<4; i++)
 { if(start[i]<=max_c && end[i]>=min_c)
  {
  sprintf(msg, "set app [font measure {Times 10 normal} \"%s_%s\"]", str[i],tag[i]);
  cmd(inter, msg);

  cmd(inter, "if {[expr $xlabel + $app]>600} {set xlabel 4; set ylabel [expr $ylabel + 1]} {}");
  cmd(inter, "set app1 [expr 330+14*$ylabel]");
  sprintf(msg, "$p create text $xlabel $app1 -font {Times 10 normal} -anchor nw -text %s_%s -tag {txt%d text} ", str[i], tag[i], i,i);
  cmd(inter, msg);
  sprintf(msg, "$p addtag p%d withtag txt%d",i,i);
  cmd(inter, msg); 
  
  /**/
//add the mobility and editability of the labels
  sprintf(msg, "$p bind txt%d <1> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag current; .f.new%d.f.plots raise current; set LX %%x; set LY %%y}",i, cur_plot, cur_plot, cur_plot);
  cmd(inter, msg);
  sprintf(msg, "$p bind txt%d <ButtonRelease-1> {.f.new%d.f.plots dtag selected}",i, cur_plot);
  cmd(inter, msg);
  sprintf(msg, "$p bind txt%d <B1-Motion> {.f.new%d.f.plots move selected [expr %%x-$LX] [expr %%y - $LY]; set LX %%x; set LY %%y}", i, cur_plot);
  cmd(inter, msg);
  sprintf(msg, "$p bind txt%d <Button-3> {set ccanvas .f.new%d.f.plots; .f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag txt%d; set hereX %%X ; set hereY %%y;set choice 26}", i, cur_plot, cur_plot, cur_plot,i);
  cmd(inter, msg);
/*  */
  cmd(inter, "set xlabel [expr $xlabel+$app+4]");
  cmd(inter, "set choice $ylabel");
  k=*choice;
  if(allblack==0)
  {
  sprintf(msg, "$p itemconf p%d -fill $c%d",i,color);
  cmd(inter, msg);
  }
  color++;
  }
 }

*choice=0;

if(i<nv)
  { sprintf(msg, "set choice [$p create text $xlabel %d -font {Times 10 normal} -anchor nw -text (more) -tag {p%d text}]", 330+k*14, i);
    cmd(inter, msg);
    sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
    cmd(inter, msg);

  }

cmd(inter, "update");

step=600/(double)(max_c-min_c);


for(k=0; k<nv; k++)
  y2[k]=y1[k]=0;

for(x01=0,x1=40,i=min_c ;x1==40 ;i++ ) //, and record the first to be used
 {
  for(k=0; k<nv; k++)
   {if(start[k]<=i && end[k]>=i && !isnan(data[k][i]))		// ignore NaNs
      y1[k]+=data[k][i];
   }
  x1=40+(int)((double)(1+i)*step);
  x01++;
 }

x1=40;


for(k=0; k<nv; k++)
  {
  if(k<numy2)
   {
   y1[k]/=x01;
   if(y1[k]<miny)
    y1[k]=miny;
	 y1[k]=300-((y1[k]-miny)/(maxy-miny))*300;
   }
  else
   {
   y1[k]/=x01;
   if(y1[k]<miny2)
    y1[k]=miny2;
	 y1[k]=300-((y1[k]-miny2)/(maxy2-miny2))*300;
   }
   

  }

for(x02=0; i<=max_c; i++)
{
 x2=40+(int)((double)(1+i-x01-min_c)*step);
 x02++;                     //counter for the average when many values occupy one point
	for(k=0, color=-1; k<nv; k++)
	{
    if(k<numy2)
     {
     cminy=miny;
     cmaxy=maxy;
     }
    else
     {
     cminy=miny2;
     cmaxy=maxy2;
     }
      
    color++;
    if( (start[k]<i) && (end[k]>=i) && !isnan(data[k][i]))		// ignore NaNs
	  {y2[k]+=data[k][i];
		if(x1!=x2 )
		{y2[k]/=x02;
       if(y2[k]<cminy)
         y2[k]=cminy;
		 y2[k]=(300-((y2[k]-cminy)/(cmaxy-cminy))*300);
       if(allblack==0)
       {
       if(line_point==1)
        {//plog("\nPoint size $point_size\n");
         sprintf(msg,"$p create line %d %d %d %d -width $point_size -tag p%d -fill $c%d",(int)x1,(int)y1[k],(int)x2,(int)y2[k], k, color);
		   cmd(inter, msg);
        }
       else
        {
//         sprintf(msg,"$p create oval %d %d %d %d -tag p%d -width 1 -fill $c%d -outline $c%d ",(int)x1,(int)y1[k],(int)x1,(int)y1[k], k, color, color);
         sprintf(msg,"$p create oval %d %d %d %d -tag p%d -width 1 -fill $c%d -outline $c%d ",(int)x1 -(int)(point_size-1),(int)y1[k]-(int)(point_size-1),(int)x1+(int)(point_size-1),(int)y1[k]+(int)(point_size-1), k, color, color);
		   cmd(inter, msg);
         sprintf(msg,"$p create oval %d %d %d %d -tag p%d -width 1 -fill $c%d -outline $c%d ",(int)x2-(int)(point_size-1),(int)y2[k]-(int)(point_size-1),(int)x2+(int)(point_size-1),(int)y2[k]+(int)(point_size-1), k, color, color);
		   cmd(inter, msg);

        }

       }
       else
       {
		 sprintf(msg,"$p create line %d %d %d %d -tag p%d -width $point_size",(int)x1,(int)y1[k],(int)x2,(int)y2[k], k);
		 cmd(inter, msg);
       }

//       color++;
		 y1[k]=y2[k];
		 y2[k]=0;
//   if(watch==1)
//	   cmd(inter, "update");

		 }
	  }
     else
      if(start[k]==i && !isnan(data[k][i]))		// ignore NaNs
       { //series entrying after the min_c

        y1[k]=(300-((data[k][i]-cminy)/(cmaxy-cminy))*300);
//        y2[k]=y1[k]*(x02-1);
        if(x1==x2)
          y2[k]=data[k][i]*x02;
       }


   if(*choice==1) //Button STOP pressed
    {//j=num_var;
     i=max_c;
    }
   
  
  }
 if(watch==1)
	cmd(inter, "update");
 if(x1!=x2 )
 { x1=x2;
	x02=0;
 }
 }

 sprintf(msg, "%d", i);
cmd(inter, "$p create text 200 420 -font {Times 10 normal} -text \"Case num:\" -anchor w ");
if(logs)
 cmd(inter, "$p create text 360 420 -font {Times 10 normal} -text \"log(Y) value: \" -anchor w ");
else
 cmd(inter, "$p create text 380 420 -font {Times 10 normal} -text \"Y value: \" -anchor w ");

sprintf(msg, "set p .f.new%d$tit", cur_plot);

sprintf(msg, "bind $p <Motion> {.f.new%d.f.plots delete xpos%d; .f.new%d.f.plots delete ypos%d ;if {%%x>39 && %%y<301} {.f.new%d.f.plots create text 290 420 -font {Times 10 normal} -text [expr (%%x-40)*(%d-%d)/600+%d] -anchor w -tag xpos%d; .f.new%d.f.plots create text 490 420 -font {Times 10 normal} -text [expr double(round((10**$pdigits)*((300.0-%%y)*(%lf-%lf)/300+%lf)))/(10**$pdigits)] -tag ypos%d}}",cur_plot, cur_plot, cur_plot, cur_plot, cur_plot, max_c, min_c, min_c,cur_plot,cur_plot, maxy, miny, miny, cur_plot);
cmd(inter, msg);

if(numy2!=nv+2)
{
sprintf(msg, "bind $p <Shift-Motion> {.f.new%d.f.plots delete xpos%d; .f.new%d.f.plots delete ypos%d ;if {%%x>39 && %%y<301} {.f.new%d.f.plots create text 290 420 -font {Times 10 normal} -text [expr (%%x-40)*(%d-%d)/600+%d] -anchor w -tag xpos%d; .f.new%d.f.plots create text 490 420 -font {Times 10 normal} -text [expr double(round((10**$pdigits)*((300.0-%%y)*(%lf-%lf)/300+%lf)))/(10**$pdigits)] -tag ypos%d}}",cur_plot, cur_plot, cur_plot, cur_plot, cur_plot, max_c, min_c, min_c,cur_plot,cur_plot, maxy2, miny2, miny2, cur_plot);
cmd(inter, msg);
}
for(i=0; i<nv; i++)
{
sprintf(msg, "$p bind p%d <Enter> {.f.new%d.f.plots create text 5 420 -font {Times 10 normal} -text \"%s_%s\" -tag p%d -tag pp -anchor w}",i,cur_plot, str[i], tag[i],i);
cmd(inter, msg);
sprintf(msg, "$p bind p%d <Leave> {.f.new%d.f.plots delete pp}",i,cur_plot);
cmd(inter, msg);
sprintf(msg, "$p dtag txt%i p%d",i,i);
cmd(inter, msg);
sprintf(msg, "$p bind txt%d <Enter> {.f.new%d.f.plots create text 5 420 -font {Times 10 normal} -text \"%s_%s\" -tag p%d -tag pp -anchor w}",i,cur_plot, str[i], tag[i],i);
cmd(inter, msg);
sprintf(msg, "$p bind txt%d <Leave> {.f.new%d.f.plots delete pp}",i,cur_plot);
cmd(inter, msg);

sprintf(msg, "$p bind p%d <Button-3> {set ccanvas .f.new%d.f.plots; set cline p%d; set hereX %%X; set hereY %%Y; set choice 31}",i,cur_plot,i);
cmd(inter, msg);
sprintf(msg, "$p bind p%d <Button-2> {set ccanvas .f.new%d.f.plots; set cline p%d; set hereX %%X; set hereY %%Y; set choice 31}",i,cur_plot,i);
cmd(inter, msg);

}

sprintf(msg, "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}",cur_plot);

//cmd(inter, "bind $c <B1-Motion> {$c delete selected; set ax %x; set ay %y; set cl [$c create line $px $py $ax $ay]; $c addtag selected withtag $cl}");
//cmd(inter, "bind $c <ButtonRelease-1> {$c dtag selected}");

//cmd(inter, "bind $c <Control-Button-1> {set px %x; set py %y; set ax [expr $px+1]; set ay [expr $py+1]; $c dtag selected; set cl [$c create line $px $py $ax $ay]; $c addtag selected withtag $cl }");
*choice=0;

cmd(inter, msg);
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }
delete[] str;
delete[] tag;
delete[] y1;
delete[] y2;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;

}


/***************************************************
PLOT_CROSS
****************************************************/
void plot_cross(int *choice)
{

int idseries;

char *app;
char **str;
char **tag;

int *start, *end, app1, app2, new_nv;
int *erase;
FILE *f;
int i, nv, j, *pos, k, nt, *list_times, h,  first;
double x1, y01, x2,  y02;
double val1,  step, **val, **data, **logdata, truemaxy;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");

Tcl_LinkVar(inter, "nt", (char *) &nt, TCL_LINK_INT);
cmd(inter, "set nt $num_t");
Tcl_UnlinkVar(inter, "nt");


if(nv<2 || nt==0)
 return;

list_times=new int[nt];
cmd(inter, "set k 0");
Tcl_LinkVar(inter, "k", (char *) &k, TCL_LINK_INT);

for(i=0; i<nt; i++)    //Sets the list of cases to plot
 {sprintf(msg, "set k [lindex $list_times %d]", i);
  cmd(inter, msg);
  list_times[i]=k;
 }
Tcl_UnlinkVar(inter, "k");

pos=new int[nv];
val=new double *[nv];
start=new int[nv];
end=new int[nv];
erase = new int[nv];
str=new char *[nv];
tag=new char *[nv];
/*
for(i=0; i<nv;i++)
 {str[i]=new char[30];
  tag[i]=new char[30];
  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d)", str[i], tag[i], &start[i], &end[i]);

 }
*/
data=new double *[nv];
logdata=new double *[nv];
for(i=0, new_nv=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
//  sscanf(msg, "%s %s (%d - %d)", str[i], tag[i], &start[i], &end[i]);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  for(k=0, erase[i]=0; k<nt; k++)
    if(list_times[k]<start[i] || list_times[k]>end[i])
        {erase[i]=1;
         k=nt;
        }
  if(erase[i]==0)
   {
    data[i]=find_data(idseries);
    if(data[i]==NULL)
      plog("Shit\n");
   
    if(logs)			// apply log to the values to show "log scale"
    {
	  logdata[new_nv]=new double[end[i]+1];	// create space for the logged values
      for(j=start[i];j<=end[i];j++)		// log everything possible
	  if(!isnan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	  else
	  {
		 logdata[i][j]=NAN;
		 sprintf(msg,"\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n",i+1,j);
		 plog(msg);
	  }
	  
	  data[i]=logdata[new_nv];				// replace the data series
    }
    val[new_nv]=new double[nt];
    new_nv++;
   }
  else
   data[i]=NULL;
 }
/*********************
 } //end of loading data

REMOVED
************************/
  for(j=0, first=1, i=0; j<nv; j++)
  {
	for(k=0; k<nt; k++)
	{if(erase[j]==0 && !isnan(data[j][list_times[k]]))		// ignore NaNs
	  {val[i][k]=data[j][list_times[k]];
     if(first==1)  //The first value is assigned to both min and max
      {miny=maxy=val[i][k];
       first=0;
      }
     if(miny>val[i][k])
      miny=val[i][k];
     if(maxy<val[i][k])
      maxy=val[i][k];

     }//End if of erase[]

	} //end for over times
    if(erase[j]==0)
     i++;
  } //End for (j on num_var)

delete[] data;


//}//end soft-data

if(dir==-1)
  sort_cs_desc(str, tag, val, new_nv, nt, res);
if(dir==1)
  sort_cs_asc(str, tag, val, new_nv, nt, res);

if(miny==maxy)
 {miny*=0.75; //To avoid divisions by zero for constant values
  maxy*=1.25;
 }


sprintf(msg, "set miny %lf", miny);
cmd(inter, msg);
sprintf(msg, "set maxy %lf", maxy);
cmd(inter, msg);

truemaxy=maxy;
maxy+=(maxy-miny)/100; //The f... windows does not plot the very firt pixels...

sprintf(msg, "set p .f.new%d", cur_plot);
cmd(inter, msg);
cmd(inter, "toplevel $p");
cmd(inter, "wm title $p $tit");
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/lsd.xbm} {}");

cmd(inter, "bind $p <Double-Button-1> {raise .; focus -force .b.ts}");
cmd(inter, "frame $p.f -width 640 -height 430");
cmd(inter, "pack $p.f");

//Reset p to point to the canvas (shit...)
cmd(inter, "set p $p.f.plots");

cmd(inter, "canvas $p -width 640 -height 430 -relief flat -background white");
cmd(inter, "pack $p");

cmd(inter, "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p");
if(grid==1)
{
cmd(inter, "$p create line 40 0 40 310 -fill grey60 -tag p");
cmd(inter, "$p create line 190 0 190 310 -fill grey60 -tag p");
cmd(inter, "$p create line 340 0 340 310 -fill grey60 -tag p");
cmd(inter, "$p create line 490 0 490 310 -fill grey60 -tag p");
}
else
{
cmd(inter, "$p create line 640 295 640 305  -width 1 -tag p");
cmd(inter, "$p create line 190 295 190 305 -width 1 -tag p");
cmd(inter, "$p create line 340 295 340 305 -width 1 -tag p");
cmd(inter, "$p create line 490 295 490 305 -width 1 -tag p");
}


/*
Numbers of Variables printed in the X axis.
REMOVED!!!

sprintf(msg, "$p create text 40 312 -font {Times 10 normal} -anchor nw -text %d -tag p",0);
cmd(inter, msg);
sprintf(msg, "$p create text 190 312 -font {Times 10 normal} -anchor n -text %d -tag p",new_nv/4);
cmd(inter, msg);
sprintf(msg, "$p create text 340 312 -font {Times 10 normal} -anchor n -text %d -tag p",new_nv/2);
cmd(inter, msg);
sprintf(msg, "$p create text 490 312 -font {Times 10 normal} -anchor n -text %d -tag p",new_nv*3/4);
cmd(inter, msg);
sprintf(msg, "$p create text 635 312 -font {Times 10 normal} -anchor ne -text %d -tag p",new_nv);
cmd(inter, msg);
*/
sprintf(msg, "set app [$p create text 40 312 -font {Times 10 normal} -anchor nw -text \"%d series used\" -tag {p  text}]",new_nv);
cmd(inter, msg);
cmd(inter, "set choice $app");

//add the mobility and editability of the labels
sprintf(msg, "$p bind %d <1> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag current; .f.new%d.f.plots raise current; set LX %%x; set LY %%y}",*choice, cur_plot, cur_plot, cur_plot);
cmd(inter, msg);
sprintf(msg, "$p bind %d <ButtonRelease-1> {.f.new%d.f.plots dtag selected}",*choice, cur_plot);
cmd(inter, msg);
sprintf(msg, "$p bind %d <B1-Motion> {.f.new%d.f.plots move selected [expr %%x-$LX] [expr %%y - $LY]; set LX %%x; set LY %%y}", *choice, cur_plot);
cmd(inter, msg);
sprintf(msg, "$p bind %d <Button-3> {set ccanvas .f.new%d.f.plots; .f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set hereX %%X ; set hereY %%y;set choice 26}", *choice, cur_plot, cur_plot, cur_plot,*choice);
cmd(inter, msg);

cmd(inter, "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p");

if(grid==1)
{
cmd(inter, "$p create line 38 300 640 300 -fill grey60 -tag p");
cmd(inter, "$p create line 38 225 640 225 -fill grey60 -tag p");
cmd(inter, "$p create line 38 150 640 150 -fill grey60 -tag p");
cmd(inter, "$p create line 38 75 640 75 -fill grey60 -tag p");
cmd(inter, "$p create line 38 0 640 0 -fill grey60 -tag p");
}
else
{
//cmd(inter, "$p create line 35 300 45 300  -tag p");
cmd(inter, "$p create line 35 225 45 225  -tag p");
cmd(inter, "$p create line 35 150 45 150  -tag p");
cmd(inter, "$p create line 35 75 45 75  -tag p");
cmd(inter, "$p create line 35 2 45 2 -tag p");
}

sprintf(msg, "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,miny);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,(miny+(truemaxy-miny)/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,(miny+(truemaxy-miny)/2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,(miny+(truemaxy-miny)*3/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]",pdigits,(truemaxy));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


cmd(inter, "update");


step=600/(double)(new_nv-1);
for(i=0 ; i<new_nv-1; i++)
 {
  for(k=j=0; j<i || erase[k]==1; k++) //find the index for the labels
    if(erase[k]==0)
     j++;
  x1=40+(int)( (double)(i*step));
  x2=40+(int)( (double)((i+1)*step));
  for(j=0; j<nt; j++)
   {
    y01=300-((val[i][j]-miny)/(maxy-miny))*300;
    y02=300-((val[i+1][j]-miny)/(maxy-miny))*300;

   if(allblack==0)
    {// draw colorful lines
    if(line_point==1)
      {
     	 sprintf(msg,"$p create line %d %d %d %d  -width $point_size -tag {p%d v%d_%d} -fill $c%d",(int)x1,(int)y01,(int)x2,(int)y02, j,i,j, j);
   	 cmd(inter, msg);
      }
    else
      {
     	 sprintf(msg,"$p create oval %d %d %d %d -tag {p%d v%d_%d} -fill $c%d -outline $c%d ",(int)x1,(int)y01,(int)x1,(int)y01, j,i,j, j, j);
   	 cmd(inter, msg);
     	 sprintf(msg,"$p create oval %d %d %d %d -tag {p%d v%d_%d} -fill $c%d -outline $c%d ",(int)x2,(int)y02,(int)x2,(int)y02, j,i,j, j, j);
   	 cmd(inter, msg);
      }
    }
    else
     {//draw only black lines
     if(line_point==1)
      {
     	 sprintf(msg,"$p create line %d %d %d %d  -width $point_size -tag {p%d v%d_%d}",(int)x1,(int)y01,(int)x2,(int)y02, j,i,j);
   	 cmd(inter, msg);
      }
    else
      {
//      sprintf(msg,"if { [$p find overlapping  %d %d %d %d ] == \"\" } {$p create oval %d %d %d %d -tag p%d -tag v%d_%d} {}",(int)x1,(int)y01,(int)x1,(int)y01,(int)x1,(int)y01,(int)x1,(int)y01, j,i,j);
     	 sprintf(msg,"$p create oval %d %d %d %d -tag {p%d v%d_%d}",(int)x1,(int)y01,(int)x1,(int)y01, j,i,j);
   	 cmd(inter, msg);
//    sprintf(msg,"if { [$p find overlapping  %d %d %d %d ] == \"\" } {$p create oval %d %d %d %d -tag p%d -tag v%d_%d} {}",(int)x2,(int)y02,(int)x2,(int)y02,(int)x2,(int)y02,(int)x2,(int)y02, j,i,j);   
     	 sprintf(msg,"$p create oval %d %d %d %d -tag {p%d v%d_%d}",(int)x2,(int)y02,(int)x2,(int)y02, j,i,j);
   	 cmd(inter, msg);
      }
     }
    sprintf(msg, "$p bind v%d_%d <Enter> {.f.new%d.f.plots create text 70 400 -font {Times 10 normal} -text \"%s_%s in Order %d at time %d\" -tag pp -anchor w}",i,j,cur_plot, str[k], tag[k], i+1, list_times[j]);
    cmd(inter, msg);
    sprintf(msg, "$p bind v%d_%d <Leave> {.f.new%d.f.plots delete pp}", i, j, cur_plot);
    cmd(inter, msg);
   }

 }

 for(i=0; i<nt ; i++)
 {
  sprintf(msg, "$p bind p%d <Button-3> {set ccanvas .f.new%d.f.plots; set cline p%d; set hereX %%X; set hereY %%Y; set choice 31}",i,cur_plot,i);
  cmd(inter, msg);
    sprintf(msg, "$p bind p%d <Button-2> {set ccanvas .f.new%d.f.plots; set cline p%d; set hereX %%X; set hereY %%Y; set choice 31}",i,cur_plot,i);
  cmd(inter, msg);

 }  
 for(i=0, j=0, k=0; i<nt && i<20; i++)
 {
  sprintf(msg, "set app [$p create text %d %d -font {Times 10 normal} -anchor nw -text \"Time = [lindex $list_times %d]\" -tag {p%d text}]",4+j*100, 330+k*12, i, i);
  cmd(inter, msg);
  cmd(inter, "set choice $app");
//add the mobility and editability of the labels
sprintf(msg, "$p bind %d <1> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag current; .f.new%d.f.plots raise current; set LX %%x; set LY %%y}",*choice, cur_plot, cur_plot, cur_plot);
cmd(inter, msg);
sprintf(msg, "$p bind %d <ButtonRelease-1> {.f.new%d.f.plots dtag selected}",*choice, cur_plot);
cmd(inter, msg);
sprintf(msg, "$p bind %d <B1-Motion> {.f.new%d.f.plots move selected [expr %%x-$LX] [expr %%y - $LY]; set LX %%x; set LY %%y}", *choice, cur_plot);
cmd(inter, msg);
sprintf(msg, "$p bind %d <Button-3> {set ccanvas .f.new%d.f.plots; .f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set hereX %%X ; set hereY %%y;set choice 26}", *choice, cur_plot, cur_plot, cur_plot,*choice);
cmd(inter, msg);
sprintf(msg, "$p bind %d <Button-2> {set ccanvas .f.new%d.f.plots; .f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set hereX %%X ; set hereY %%y;set choice 26}", *choice, cur_plot, cur_plot, cur_plot,*choice);
cmd(inter, msg);
  
  if(j<5)
	j++;
  else
	{k++;
	 j=0;
	}

  if(allblack==0)
  {
  sprintf(msg, "$p itemconf p%d -fill $c%d",i,i);
  cmd(inter, msg);
  }


 }


cmd(inter, "set lab \"\"");

  cmd(inter, "$p create text 5 400 -font {Times 10 normal} -text \"X value: \" -anchor w ");
if(logs)
  cmd(inter, "$p create text 5 420 -font {Times 10 normal} -text \"log(Y) value: \" -anchor w ");
else
  cmd(inter, "$p create text 5 420 -font {Times 10 normal} -text \"Y value: \" -anchor w ");

sprintf(msg, "set p .f.new%d$tit", cur_plot);


//app=(char *)Tcl_GetVar(inter, "tit",0);
//strcpy(str1, app);

sprintf(msg, "bind $p <Motion> {.f.new%d.f.plots delete ypos%d ;if {%%x>39 && %%y<301} { .f.new%d.f.plots create text 130 420 -font {Times 10 normal} -text [expr double(round((10**$pdigits)*((300.0-%%y)*(%lf-%lf)/300+%lf)))/(10**$pdigits)] -tag ypos%d}}", cur_plot, cur_plot,cur_plot, maxy, miny, miny, cur_plot);
cmd(inter, msg);


sprintf(msg, "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}",cur_plot);
cmd(inter, msg);

for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
 }
for(i=0; i<new_nv; i++)
{
  delete[] val[i];
  if(logs)
    delete[] logdata[i];
}
delete[] str;
delete[] tag;
delete[] pos;
delete[] val;
delete[] start;
delete[] end;
delete[] erase;
delete[] logdata;
}


/***************************************************
INIT_CANVAS
****************************************************/
void init_canvas(void)
{
int i;
cmd(inter, "set c0 black");
cmd(inter, "set c1 red");
cmd(inter, "set c2 green");
cmd(inter, "set c3 #d0d000");
cmd(inter, "set c4 #fb46bc");
cmd(inter, "set c5 blue");
cmd(inter, "set c6 DeepSkyBlue1");
cmd(inter, "set c7 grey40");
cmd(inter, "set c8 PaleTurquoise2");
cmd(inter, "set c9 cyan");
cmd(inter, "set c10 aquamarine1");
cmd(inter, "set c11 DarkSeaGreen1");
cmd(inter, "set c12 chartreuse1");
cmd(inter, "set c13 OliveDrab");
cmd(inter, "set c14 khaki3");
cmd(inter, "set c15 LightGoldenrod4");
cmd(inter, "set c16 sienna1");
cmd(inter, "set c17 chocolate4");
cmd(inter, "set c18 firebrick3");
cmd(inter, "set c19 orange1");
cmd(inter, "set c20 salmon3");
cmd(inter, "set c1000 white");
 for(i=21; i<1000; i++)
  {sprintf(msg, "set c%d black", i);
   cmd(inter, msg);
  }

for(i=0; i<100; i++)
  {sprintf(msg, "set c1%03d grey%d", i,i);
   cmd(inter, msg);
  }
  
cmd(inter, "set c1000 white");  
}

/***************************************************
SET_CS_DATA
****************************************************/
void set_cs_data(int *choice)
{

cmd(inter, "toplevel .s");
cmd(inter, "wm protocol .s WM_DELETE_WINDOW {set choice 2}");
cmd(inter, "wm transient .s .");
cmd(inter, "set p .s");
cmd(inter, "wm title $p \"Select Cases for Cross section analysis\"");
//cmd(inter, "raise $p");
//cmd(inter, "frame $p.rd");

Tcl_LinkVar(inter, "res", (char *) &res, TCL_LINK_INT);
Tcl_LinkVar(inter, "dir", (char *) &dir, TCL_LINK_INT);

*choice=0;
res=1;

switch(res)
{
case 1:
*choice=0;
/********/
cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "label .s.i.l -text \"Insert time steps to use\"");
cmd(inter, "entry .s.i.e -textvariable bidi");
cmd(inter, "label .s.i.l1 -text \"Time steps selected\"");
cmd(inter, "listbox .s.i.lb");
cmd(inter, "set bidi $maxc");
cmd(inter, "set res 0");
cmd(inter, "set dir 0");
cmd(inter, "set count 0");
cmd(inter, "set sfrom -1");
cmd(inter, "set sto -1");
cmd(inter, "pack .s.i.l .s.i.e .s.i.l1 .s.i.lb");

cmd(inter, "frame .s.fb -relief groove -bd 2");
cmd(inter, "set p .s.fb");

cmd(inter, "bind .s.i.e <KeyPress-Return> {$p.add invoke}");
cmd(inter, "button $p.add -text Add -command {.s.i.lb insert end $bidi; incr count 1; focus .s.i.e; .s.i.e selection range 0 end; .s.i.lb selection set end }");
cmd(inter, "button $p.del -text Delete -command {.s.i.lb delete [.s.i.lb curselection]; incr count -1; focus .s.i.e; .s.i.e selection range 0 end }");
cmd(inter, "button $p.can -text Abort -command {set choice 2}");
cmd(inter, "button $p.end -text Continue -command {set choice 1}");
cmd(inter, "button $p.help -text Help -command {LsdHelp mdatares.html#crosssection}");
cmd(inter, "pack $p.add $p.del $p.end $p.can $p.help -expand yes -fill x -anchor n");

cmd(inter, "frame .s.s -relief groove -bd 2");
cmd(inter, "label .s.s.l -text \"Use series in the same order as selected\"");
cmd(inter, "button .s.s.up -text \"Sort Ascending\" -command {set res [.s.i.lb curselection]; .s.s.l config -text \"User series according to increasing values at time: [selection get]\"; set dir 1}");
cmd(inter, "button .s.s.down -text \"Sort Descending\" -command {set res [.s.i.lb curselection]; .s.s.l config -text \"User series according to decreasing values at time: [selection get]\"; set dir -1}");
cmd(inter, "button .s.s.nosort -text \"No Sort\" -command {.s.s.l config -text \"Use series in the same order as selected\"; set dir 0}");
cmd(inter, "pack .s.s.l .s.s.up .s.s.down .s.s.nosort -anchor n");

cmd(inter, "pack .s.i .s.fb .s.s -side left -anchor n");
cmd(inter, "focus .s.i.e; .s.i.e selection range 0 end");
cmd(inter, "set sfrom $bidi");
cmd(inter, "set sto $bidi");
cmd(inter, "set sskip 1");

cmd(inter, "bind .s <KeyPress-c> {set choice 1}");
cmd(inter, "bind .s <Control-f> {set sfrom $bidi}");
cmd(inter, "bind .s <Control-t> {set sto $bidi}");
cmd(inter, "bind .s <Control-x> {set sskip $bidi}");
cmd(inter, "bind .s <Control-z> { if { [expr $sto - $sfrom] > 0 } {for {set x $sfrom} {$x<$sto} {incr x $sskip} {	.s.i.lb insert end $x} } {}}");

while(*choice==0)
  Tcl_DoOneEvent(0);

if(*choice==2)
 goto end;

cmd(inter, "if { [.s.i.lb size] == 0 } {tk_messageBox -type ok -title \"Warning\" -message \"No time step has been selected.\\nNo graph will be created\";set choice 2} { }");
cmd(inter, "set num_t [.s.i.lb size]");
cmd(inter, "set list_times [.s.i.lb get 0 end]");


end:
Tcl_UnlinkVar(inter, "res");
Tcl_UnlinkVar(inter, "dir");

cmd(inter, "destroy .s");
return;

}

}

//v[Variables][Time]
/***************************************************
SORT_CS_DESC
****************************************************/
void sort_cs_desc(char **s,char **t, double **v, int nv, int nt, int c)
{
int i, j, h;
double dapp;
char sapp[50];


for(i=nv-2; i>=0; i--)
 {
 for(j=i; j<nv-1 && v[j][c]<v[j+1][c] ; j++)
  {
   for(h=0;h<nt; h++)
    {
     dapp=v[j][h];
     v[j][h]=v[j+1][h];
     v[j+1][h]=dapp;
    }
   strcpy(sapp,s[j]);
   strcpy(s[j],s[j+1]);
   strcpy(s[j+1],sapp);

   strcpy(sapp,t[j]);
   strcpy(t[j],t[j+1]);
   strcpy(t[j+1],sapp);

//   printf("%d",j);
  }

 }
}

/***************************************************
SORT_CS_ASC
****************************************************/
void sort_cs_asc(char **s,char **t, double **v, int nv, int nt, int c)
{
int i, j, h;
double dapp;
char sapp[50];


for(i=nv-2; i>=0; i--)
 {
 for(j=i; j<nv-1 && v[j][c]>v[j+1][c]; j++)
  {
   for(h=0;h<nt; h++)
    {
     dapp=v[j][h];
     v[j][h]=v[j+1][h];
     v[j+1][h]=dapp;
    }
   strcpy(sapp,s[j]);
   strcpy(s[j],s[j+1]);
   strcpy(s[j+1],sapp);

   strcpy(sapp,t[j]);
   strcpy(t[j],t[j+1]);
   strcpy(t[j+1],sapp);

  }

 }
}


/***************************************************
SEARCH_LAB_TIT
****************************************************/
double *search_lab_tit(object *r, char *s, char *t, int st, int en)
{
object *cur;
variable *cv;
double *res=NULL;
sprintf(msg, "%s_%s", s, t);
for(cur=r; cur!=NULL &&res==NULL; cur=cur->next)
 { for(cv=cur->v; cv!=NULL; cv=cv->next)
    {
     if(cv->save==1 && !strcmp(cv->lab_tit, msg)  && st==cv->start )
      return cv->data;
    }
  if(cur->b!=NULL)
   res=search_lab_tit(cur->b->head, s, t, st, en);
 }
if(res==NULL &&r->up==NULL)
 for(cv=cemetery; cv!=NULL &&res==NULL; cv=cv->next)
  {if(!strcmp(cv->lab_tit, msg) && st==cv->start)
     return cv->data;
  }
return res;
}


double *find_data(int id)
{
int i;

double *d, app;

return vs[id].data;

sprintf(msg, "tk_messageBox -type ok -message \"System error: series %d not found. To continue the very first series is returned.\"", id);
cmd(inter, msg);
return vs[0].data;
}


/***************************************************
SEARCH_LAB_TIT_FILE
****************************************************/
double *search_lab_tit_file(char *s, char *t, int st, int en)
{
int i, done, app_st, app_en, j;
char str[50], tag[30];
double *d, app;
FILE *f;

for(i=0; i<num_var ; i++)
 { if(!strcmp(s, vs[i].label) && !strcmp(t, vs[i].tag) && vs[i].start==st && vs[i].end==en)
    return vs[i].data;

 }

cmd(inter, "tk_messageBox -type ok -title \"Data not found\" -message \"Bad crash...\"");
exit(0);
f=fopen(filename, "r");
for(i=0, done=0; i<num_var; i++)
 {fscanf(f, "%s %s (%d %d)\t", str, tag, &app_st, &app_en);
  if(!strcmp(s, str) && !strcmp(tag, t ) && (app_st==st || app_st==-1))
    done=i;

 }


d=new double[num_c+1];

if(d==NULL)
 plog("Ecco");

fscanf(f, "\n");
for(i=0; i<=num_c; i++)
  for(j=0; j<num_var; j++)
   if(j==done)
    fscanf(f,"%lf",&d[i+1]);
   else
    fscanf(f, "%lf", &app);
fclose(f);

return d;
}



/***************************************************
INSERT_LABELS_NOSAVE
****************************************************/
void insert_labels_nosave(object *r, char *lab,  int *num_v)
{
object *cur;
variable *cv;
bridge *cb;
int flag=0;

for(cur=r; cur!=NULL; cur=cur->next)
 {
  for(cv=cur->v; cv!=NULL; cv=cv->next)
   {
   if(cv->save==0 && !strcmp(cv->label,lab))
    {
     set_lab_tit(cv);
     sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, 0, 0, num_var + (*num_v));
     cmd(inter, msg);
     sprintf(msg, ".f.vars.ch.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, 0, 0, num_var + (*num_v));
     cmd(inter, msg);

     *num_v+=1;
     flag=1;
    }
   } 
  //if(cur->b !=NULL && cur->b->head!=NULL && flag==0)
  for(cb=cur->b; cb!=NULL && cb->head!=NULL && flag==0; cb=cb->next) 
   insert_labels_nosave(cb->head, lab, num_v);

 }
}

/***************************************************
insert_data_nosave
****************************************************/
void insert_data_nosave(object *r, char *lab, int *num_v)
{
store *app;
int i;
insert_labels_nosave(r,lab, num_v);
app=new store[num_var+ *num_v];
for(i=0; i<num_var; i++)
 {app[i]=vs[i];
  strcpy(app[i].label, vs[i].label);
  strcpy(app[i].tag, vs[i].tag);
 } 
delete[] vs;
vs=app;

*num_v=num_var;
insert_store_nosave(r,lab, num_v);
num_var=*num_v;


sprintf(msg, ".f.com.nvar conf -text \"Series = %d\"",num_var);
cmd(inter,msg);
}

/***************************************************
INSERT_STORE_NOSAVE
****************************************************/
void insert_store_nosave(object *r, char *lab, int *num_v)
{
object *cur;
variable *cv;
bridge *cb;

int flag=0;
for(cur=r; cur!=NULL; cur=cur->next)
 {for(cv=cur->v; cv!=NULL; cv=cv->next)
   if(cv->save==0 && !strcmp(cv->label, lab))
    {
     strcpy(vs[*num_v].label,cv->label);
     set_lab_tit(cv);
     strcpy(vs[*num_v].tag,cv->lab_tit);
     vs[*num_v].start=0;
     vs[*num_v].end=0;
     vs[*num_v].rank=*num_v;
     vs[*num_v].data=&(cv->val[0]);
     *num_v+=1;
     flag=1;
    }
//  if(cur->b!=NULL && cur->b->head!=NULL && flag==0)
  for(cb=cur->b; cb!=NULL && cb->head!=NULL && flag==0; cb=cb->next) 
   insert_store_nosave(cb->head,lab, num_v);

 }
}


/***************************************************
INSERT_LABELS_MEM
****************************************************/
void insert_labels_mem(object *r, int *num_v, int *num_c)
{
object *cur;
variable *cv;
bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
   if(cv->save)
    {
//     sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit+strlen(cv->label)+1, cv->start, cv->end, *num_v); //this uses the tags embdedded in lab_tit
     set_lab_tit(cv);
     sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, cv->start, cv->end, *num_v);
     if(cv->end>*num_c)
       *num_c=cv->end;
     cmd(inter, msg);
     *num_v+=1;
    }
//for(cur=r->son; cur!=NULL; )
for(cb=r->b; cb!=NULL; cb=cb->next)
 {cur=cb->head;
 if(cur->to_compute==1)
   {
   for(cur=cb->head; cur!=NULL; cur=cur->next)
     {
     insert_labels_mem(cur, num_v, num_c);
     
     }
   }  
 }
if(r->up==NULL)
 for(cv=cemetery; cv!=NULL; cv=cv->next)
  {  sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, cv->start, cv->end, *num_v);
     if(cv->end>*num_c)
       *num_c=cv->end;
     cmd(inter, msg);
     *num_v+=1;

  }
}

/***************************************************
insert_data_mem
****************************************************/
void insert_data_mem(object *r, int *num_v, int *num_c)
{


insert_labels_mem(r,num_v, num_c);
vs=new store[*num_v];
*num_v=0;
insert_store_mem(r, num_v);

}

/***************************************************
INSERT_STORE_MEM
****************************************************/
void insert_store_mem(object *r, int *num_v)
{
object *cur;
variable *cv;
bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
 if(cv->save)
  {
   set_lab_tit(cv);
   strcpy(vs[*num_v].label,cv->label);
   strcpy(vs[*num_v].tag,cv->lab_tit);
   vs[*num_v].start=cv->start;
   vs[*num_v].end=cv->end;
   vs[*num_v].rank=*num_v;
   vs[*num_v].data=cv->data;
   *num_v+=1;
  }
for(cb=r->b; cb!=NULL; cb=cb->next)
 {cur=cb->head;
 if(cur->to_compute==1)
   {
   for(cur=cb->head; cur!=NULL; cur=cur->next)
     {
      insert_store_mem(cur, num_v);
      
     }
   }
 }    
 
if(r->up==NULL)
 for(cv=cemetery; cv!=NULL; cv=cv->next)
  {
     strcpy(vs[*num_v].label,cv->label);
     strcpy(vs[*num_v].tag,cv->lab_tit);
     vs[*num_v].start=cv->start;
     vs[*num_v].end=cv->end;
     vs[*num_v].rank=*num_v;
     vs[*num_v].data=cv->data;
     *num_v+=1;

  }
}


/***************************************************
INSERT_data_FILE
****************************************************/
void insert_data_file(int *num_v, int *num_c)
{
FILE *f;
char ch, label[60], tag[60], app_str[20];
int i, j, new_v, new_c;
store *app;

f=fopen(filename, "r");
if(f==NULL)
 plog("\nFile not opened");
ch=(char)fgetc(f);
new_v=0;
if(*num_v==0)
  sprintf(msg, "\nResult data from file %s:\n", filename);
else
  sprintf(msg, "\nAdditional data file number %d.\nResult data from file %s:\n", file_counter, filename);  
plog(msg);
//cmd(inter, "raise .log");
//cmd(inter, "update");
while(ch!='\n')
 if( (ch=(char)fgetc(f))=='\t')
   new_v+=1;
fclose(f);
sprintf(msg, "- %d Variables\n",  new_v);
plog(msg);
cmd(inter, "update");
f=fopen(filename, "r");

new_c=-1;
while(ch!=EOF)
 if( (ch=(char)fgetc(f))=='\n')
   new_c+=1;
fclose(f);
sprintf(msg, "- %d Cases\nLoading...", new_c);
plog(msg);
cmd(inter, "update");
if(*num_v==0)
 {
  vs=new store[new_v];
 }
else
 {
 app=new store[new_v+ *num_v];
 for(i=0; i<*num_v; i++)
  {app[i]=vs[i];
   strcpy(app[i].label, vs[i].label);
   strcpy(app[i].tag, vs[i].tag);
  } 
 delete[] vs;
 vs=app;
 }  
f=fopen(filename, "r");

if(*num_v>-1)
 sprintf(app_str, " (file=%d)",file_counter);
else
 sprintf(app_str, "");
for(i=*num_v; i<*num_v+new_v; i++)
 {
  fscanf(f, "%s %s (%d %d)\t", vs[i].label, vs[i].tag, &(vs[i].start), &(vs[i].end));
  strcat(vs[i].label, "F");
  sprintf(msg, "%d_%s", file_counter, vs[i].tag);
  strcpy(vs[i].tag, msg);
  vs[i].rank=i;

 if(vs[i].start!=-1)
   sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d %s\"", vs[i].label, vs[i].tag, vs[i].start, vs[i].end, i, app_str);
 else
   {sprintf(msg, ".f.vars.lb.v insert end \"%s %s (0 - %d) # %d %s\"", vs[i].label, vs[i].tag, new_c-1, i, app_str);
    vs[i].start=0;
    vs[i].end=new_c-1;
   }
 cmd(inter, msg);
 vs[i].data=new double[new_c+2];
 }

long linsiz=new_v*(DBL_DIG+4)+1;
if(linsiz>INT_MAX)
 linsiz=INT_MAX;	// prevents too big buffers for 32-bit arch
char *tok,*linbuf=new char[linsiz];
const char *sep=" \t,;|";	// token separators are spaces, tabs, commas etc.

for(j=0; j<=new_c; j++)
{
 fgets(linbuf,linsiz,f);	// buffers one entire line
 tok=strtok(linbuf,sep); // prepares for parsing and get first one
 for(i=*num_v; i<new_v+*num_v; i++)
 {
    //fscanf(f,"%lf",&(vs[i].data[j]));      // can't handle n/a's
	if(tok==NULL)				// get to end of line too early
	  break;
	if(!strcmp(tok,nonavail))	// it's a non-available observation
	  vs[i].data[j]=NAN;
	else
	  sscanf(tok,"%lf",&(vs[i].data[j]));
 	tok=strtok(NULL,sep);		// get next token, if any
 }
}
fclose(f);
*num_v+=new_v;
new_c--;
if(new_c > *num_c)
 *num_c=new_c;
if(new_c > max_c)
 max_c=new_c; 
}

/************************
SAVE_DATA1
************************/
void save_data1(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[100], delimiter[10], misval[10];
int i, nv, j, *start, *end, typelab, numcol, del, fr, gp, type_res;
double **data;
FILE *fsave;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");

*choice=0;

if(nv==0)
 return;

if(logs)
  cmd(inter, "tk_messageBox -type ok -icon warning -title \"Warning\" -message \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"");

data=new double *[nv];
start=new int[nv];
end=new int[nv];


str=new char *[nv];
tag=new char *[nv];

max_c=min_c=0;

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);

  data[i]=find_data(idseries);
  if(max_c<end[i])
   max_c=end[i];
 }


//Variables' Name in first column
fr=1;
strcpy(misval,nonavail);
Tcl_LinkVar(inter, "typelab", (char *) &typelab, TCL_LINK_INT);
typelab=4;
cmd(inter, "toplevel .lab");
cmd(inter, "wm title .lab \"Saving Data\"");
cmd(inter, "wm transient .lab . ");
cmd(inter, "frame .lab.f");
cmd(inter, "radiobutton .lab.f.lsd -text \"Lsd Result File\" -variable typelab -value 3");
cmd(inter, "radiobutton .lab.f.nolsd -text \"Text File\" -variable typelab -value 4");
cmd(inter ,"button .lab.ok -text Proceed -command {set choice 1}");
cmd(inter, "button .lab.help -text Help -command {LsdHelp mdatares.html#save}");
cmd(inter ,"button .lab.esc -text Cancel -command {set choice 2}");

cmd(inter, "pack .lab.f.lsd .lab.f.nolsd -anchor w");
cmd(inter, "pack .lab.f .lab.ok .lab.help .lab.esc");
cmd(inter, "bind .lab <Return> {.lab.ok invoke}");
cmd(inter, "bind .lab <Escape> {.lab.esc invoke}");
cmd(inter, "focus .lab");
#ifndef DUAL_MONITOR
cmd(inter, "set w .lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
while(*choice==0)
 Tcl_DoOneEvent(0);
if(*choice==2)
 goto end;
type_res=typelab;

*choice=0;
cmd(inter, "destroy .lab.f .lab.ok .lab.esc .lab.help");

 


if(typelab==4)
{
Tcl_LinkVar(inter, "deli", (char *) &del, TCL_LINK_INT);
Tcl_LinkVar(inter, "numcol", (char *) &numcol, TCL_LINK_INT);
Tcl_LinkVar(inter, "fr", (char *) &fr, TCL_LINK_INT);

typelab=1;
cmd(inter, "frame .lab.f -relief groove -bd 2");
cmd(inter, "label .lab.f.tit -text \"Labels to use\" -foreground red");

cmd(inter, "radiobutton .lab.f.orig -text Original -variable typelab -value 1");
cmd(inter, "radiobutton .lab.f.new -text \"New Names\" -variable typelab -value 2");
cmd(inter, "set newlab \"\"");
cmd(inter, "entry .lab.f.en -textvariable newlab");
cmd(inter, "set gp 0");
cmd(inter, "checkbutton .lab.f.gp -text \"Add #\" -variable gp");
cmd(inter, "bind .lab.f.en <FocusIn> {.lab.f.new invoke}");
cmd(inter, "pack .lab.f.tit .lab.f.orig .lab.f.new .lab.f.en .lab.f.gp -anchor w");
cmd(inter, "frame .lab.d -relief groove -bd 2");
cmd(inter, "label .lab.d.tit -text \"Columns delimiter\" -foreground red");

cmd(inter, "frame .lab.d.r");
del=1;
cmd(inter, "radiobutton .lab.d.r.tab -text \"Tab Delimited\" -variable deli -value 1");
cmd(inter, "radiobutton .lab.d.r.oth -text \"Other Delimiter\" -variable deli -value 2");
cmd(inter, "set delimiter \"\"");
cmd(inter, "entry .lab.d.r.del -textvariable delimiter");
cmd(inter, "bind .lab.d.r.del <FocusIn> {.lab.d.r.oth invoke}");

cmd(inter, "radiobutton .lab.d.r.col -text \"Fixed Length Columns\" -variable deli -value 3");
numcol=16;
cmd(inter, "entry .lab.d.r.ecol -textvariable numcol");
cmd(inter, "bind .lab.d.r.ecol <FocusIn> {.lab.d.r.col invoke}");


cmd(inter, "pack .lab.d.r.tab .lab.d.r.oth .lab.d.r.del .lab.d.r.col .lab.d.r.ecol -anchor w");
cmd(inter, "pack .lab.d.tit .lab.d.r -anchor w");

cmd(inter, "frame .lab.gen -relief groove -bd 2");
cmd(inter, "label .lab.gen.tit -text \"General Options\" -foreground red");
cmd(inter, "checkbutton .lab.gen.fr -text \"Variables in First Row\" -variable fr");
cmd(inter, "label .lab.gen.miss -text \"Missing Values\"");
cmd(inter, "set misval \"n/a\"");
cmd(inter, "entry .lab.gen.mis_val -textvariable misval");
cmd(inter, "pack .lab.gen.tit .lab.gen.fr .lab.gen.miss .lab.gen.mis_val -anchor w");
cmd(inter, "button .lab.help -text Help -command {LsdHelp mdatares.html#save}");
cmd(inter ,"button .lab.esc -text Cancel -command {set choice 2}");

cmd(inter ,"button .lab.ok -text Proceed -command {set choice 1}");
cmd(inter, "pack .lab.f .lab.d .lab.gen .lab.ok .lab.esc .lab.help -fill x");
*choice=0;
cmd(inter, "focus .lab");
cmd(inter, "bind .lab <KeyPress-Return> {.lab.ok invoke}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
//cmd(inter, "raise .lab");
while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==2)
 goto end;

cmd(inter, "set choice $gp");
gp=*choice;

*choice=0;

app=(char *)Tcl_GetVar(inter, "misval", 0);
strcpy(misval,app);

}

if(type_res==4)
  sprintf(msg, "set bah [tk_getSaveFile -title \"Data File Name\" -initialdir [pwd] -defaultextension \"txt\" -filetypes {{{Text Files} {.txt}} {{Lsd Result Files} {.res}} {{All Files} {*}} }]");
else
  sprintf(msg, "set bah [tk_getSaveFile -title \"Data File Name\"  -initialdir [pwd] -defaultextension \"res\" -filetypes {{{Lsd Result Files} {.res}} {{Text Files} {.txt}} {{All Files} {*}} }]");
cmd(inter, msg);
app=(char *)Tcl_GetVar(inter, "bah",0);
strcpy(msg, app);

if(strlen(msg)==0)
 goto end;
fsave=fopen(msg, "wt");  // use text mode for Windows better compatibility


if(del!=3) //Delimited files
{if(del==2)
 {app=(char *)Tcl_GetVar(inter, "delimiter",0);
  if(strlen(app)==0)
    strcpy(delimiter,"\t");
  else
    strcpy(delimiter,app);
 }
 else
  strcpy(delimiter, "\t");
}
if(fr==1)
{
if(del!=3)
{
switch(typelab)
{
case 1:   //Original labels
if(gp==1)
 fprintf(fsave, "#");
for(i=0; i<nv; i++)
  fprintf(fsave, "%s_%s%s", str[i], tag[i], delimiter);
break;

case 2: //New names for labels
  app=(char *)Tcl_GetVar(inter, "newlab",0);
  if(strlen(app)==0)
    strcpy(msg,"Var");
  else
    strcpy(msg,app);
if(gp==1)
 fprintf(fsave, "#");
 for(i=0; i<nv; i++)
   fprintf(fsave, "%s%d%s", msg, i, delimiter);
 break;

case 3:
for(i=0; i<nv; i++) //Lsd result files
  fprintf(fsave, "%s %s (%d %d)\t", str[i], tag[i], start[i], end[i]);
break;
 } //end switch delimiter
} //and of delimiter header writing
else //header for Column text files
  {
if(gp==1)
 fprintf(fsave, "#");

  strcpy(str1, "                                                                                ");
   for(i=0; i<nv; i++)
    {sprintf(msg, "%s_%s", str[i], tag[i]);
     if(strlen(msg)<=numcol)
      strcat(msg, str1);
     msg[numcol]=(char)NULL;
    fprintf(fsave,"%s", msg);
    }
  }

fprintf(fsave,"\n");
}//end of header writing

if(del!=3) //data delimited writing
{
for(j=min_c; j<=max_c; j++)
 {for(i=0; i<nv; i++)
   {if(j>=start[i] && j<=end[i] && !isnan(data[i][j]))		// save NaN as n/a
      fprintf(fsave, "%g%s", data[i][j], delimiter);
    else
      fprintf(fsave, "%s%s", misval, delimiter);
   }
  fprintf(fsave,"\n");
 }

}
else //column data writing
{ strcpy(str1, "00000000000000000000000000000000000000000000000000000000000000000000000000000000");
for(j=min_c; j<=max_c; j++)
 {for(i=0; i<nv; i++)
   {
   if(j>=start[i] && j<=end[i] && !isnan(data[i][j]))
   {
      sprintf(msg, "%.10lf", data[i][j]);
	  strcat(msg, str1);
	  msg[numcol]=(char)NULL;
   }
   else
      sprintf(msg, "%s", misval);		// save NaN as n/a
   fprintf(fsave, "%s", msg);
   }
  fprintf(fsave,"\n");
 }

}
fclose(fsave);

end:
cmd(inter, "destroy .lab");
Tcl_UnlinkVar(inter, "typelab");
Tcl_UnlinkVar(inter, "numcol");
Tcl_UnlinkVar(inter, "deli");
Tcl_UnlinkVar(inter, "fr");
Tcl_UnlinkVar(inter, "misval");


for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
 }
delete[] str;
delete[] tag;
delete[] data;
delete[] start;
delete[] end;
*choice=0;

} //end of save_data1


/************************
STATISTICS
************************/
void statistics(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[50], longmsg[180];
int i, nv, j, *start, *end;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");
double **data,**logdata, av, var, num, ymin, ymax, sig;


data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];

if(nv==0)
 return;

str=new char *[nv];
tag=new char *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries); 
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!isnan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 sprintf(msg,"\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n",i+1,j);
		 plog(msg);
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }

 }

if(logs)
 sprintf(msg, ".log.text.text insert end \"\n\nTime series Descriptive Stats (in log).\n\" tabel");
else
 sprintf(msg, ".log.text.text insert end \"\n\nTime series Descriptive Stats.\n\" tabel");
cmd(inter, msg);

sprintf(str1, "%d Cases", max_c-min_c+1);
sprintf(longmsg, "%-20s\tAverage\tVar.\tMin\tMax\tSigma\n", str1);
sprintf(msg, ".log.text.text insert end \"%s\" tabel", longmsg);
cmd(inter, msg);

for(i=0; i<nv; i++)
{
 ymin=10000000;
 ymax=-10000000;

 for(av=var=num=0, j=min_c; j<=max_c; j++)
  {
  if(j>=start[i] && j<=end[i] && !isnan(data[i][j]))		// ignore NaNs
  {
//   if(j==start[i])
//     ymin=ymax=data[i][j]; DOES NOT WORK IN CASE OF LIMITED RANGE
   if(data[i][j]<ymin)
    ymin=data[i][j];
   if(data[i][j]>ymax)
    ymax=data[i][j];
   av+=data[i][j];
   num++;
   var+=data[i][j]*data[i][j];
  }
  }

 if(num>1)
 {
  av=av/num;
  var=fabs(var/num-av*av);
  sig=sqrt(var*num/(num-1));
 }
 else
  var=sig=0;
 if(num>0)
 {
 sprintf(msg, "%s %s (%.*g)", str[i], tag[i], pdigits, num);
 sprintf(str1, "%-20s\t", msg);
 sprintf(msg, ".log.text.text insert end \"%s\" tabel", str1);
 cmd(inter, msg);


 sprintf(longmsg, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", pdigits, av, pdigits, var, pdigits, ymin, pdigits, ymax, pdigits, sig);
sprintf(msg, ".log.text.text insert end \"%s\" tabel", longmsg);
cmd(inter, msg);


 }
}
plog("\n");
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }
delete[] str;
delete[] tag;

delete[] logdata;
delete[] data;
/*
else
 {for(i=0; i<nv; i++)
   if(start[i]<=i && end[i]>=i)
    delete data[i];
  delete data;
 }
*/
delete[] start;
delete[] end;


} //end Statistics


struct fr
{
double *v;
int *freq;
};

/************************
FREQUENCIES
************************/
void frequencies(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[50], longmsg[180];
int i, nv, j, h,k, *start, *end;
bool first;

fr *freq;

int *num_freq;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");
double **data, av, var, num, ymin, ymax, sig;


data=new double *[nv];
start=new int[nv];
end=new int[nv];

if(nv==0)
 return;

str=new char *[nv];
tag=new char *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

freq=new fr[nv];
num_freq=new int[nv];

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];
  num_freq[i]=1;
  
  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {

   data[i]=find_data(idseries);   
   freq[i].freq=new int[num_c];
   freq[i].v=new double[num_c];
   for(h=0; h<num_c; h++)
    freq[i].freq[h]=0;
   }

 }




for(i=0; i<nv; i++)
{
 first=true;
 //for each var
 for(j=min_c; j<=max_c; j++)
  {//for each case
  if(j>=start[i] && j<=end[i] && !isnan(data[i][j]))		// ignore NaNs
  {

  if(first)
   {//very first case
    freq[i].v[0]=data[i][j];
    freq[i].freq[0]=1;
	first=false;
   }
  else 
   {
   for(h=0; h<j-start[i]; h++)
    {//check all previous cases
     if(data[i][j]==freq[i].v[h])
      {//if alread exist, increase the frequency
      freq[i].freq[h]++;
      h=j-start[i]+1;
      }
      
    }
    if(h==j-start[i])
     {//no previous case found
      k=num_freq[i]++; //increase the cases counter
      freq[i].v[k]=data[i][j]; //add the new case value
      freq[i].freq[k]=1; //set its frequency
     }
   }
   

  }
  }

sprintf(msg, ".log.text.text insert end \"\nFreq. for %s_%s\nVal.\tNum.\n\" ", str[i], tag[i]);
cmd(inter, msg);
for(h=0; h<num_freq[i]; h++)
 {
 sprintf(msg, ".log.text.text insert end \"%.*g\t%d\n\"", pdigits, freq[i].v[h], freq[i].freq[h]);
 cmd(inter, msg);
 
 }

 }


for(i=0; i<nv; i++)
 {freq[i].freq;
  freq[i].v;
  delete[] str[i];
  delete[] tag[i];
 }
delete[] freq;
delete[] num_freq; 
delete[] str;
delete[] tag;

delete[] data;
delete[] start;
delete[] end;


} //end Statistics


/************************
STATISTICS_CROSS
************************/
void statistics_cross(int *choice)
{
int idseries;
char *app;
char **str, **tag;
char str1[50], longmsg[180];
int i, nv, j, *start, *end, nt, *list_times, h, k;
double **data,**logdata, av, var, num, ymin, ymax, sig;
bool first;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");

Tcl_LinkVar(inter, "nt", (char *) &nt, TCL_LINK_INT);
cmd(inter, "set nt $num_t");
Tcl_UnlinkVar(inter, "nt");


if(nv<2 || nt==0)
 return;

list_times=new int[nt];
cmd(inter, "set k 0");
Tcl_LinkVar(inter, "k", (char *) &k, TCL_LINK_INT);

for(i=0; i<nt; i++)    //Sets the list of cases to plot
 {sprintf(msg, "set k [lindex $list_times %d]", i);
  cmd(inter, msg);
  list_times[i]=k;
 }
Tcl_UnlinkVar(inter, "k");

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];

if(nv==0)
 return;

str=new char *[nv];
tag=new char *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  data[i]=find_data(idseries); 
/*
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries); 
   }
 */  
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!isnan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 sprintf(msg,"\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n",i+1,j);
		 plog(msg);
	   }
	 data[i]=logdata[i];				// replace the data series
   }
 }

if(logs)
 sprintf(msg, ".log.text.text insert end \"\n\nCross Section Descriptive Stats (in log).\n\" tabel");
else
 sprintf(msg, ".log.text.text insert end \"\n\nCross Section Descriptive Stats.\n\" tabel");
cmd(inter, msg);

sprintf(str1, "%d Variables",nv);
sprintf(longmsg, "%-20s\tAverage\tVar.\tMin\tMax\tSigma\n", str1);
sprintf(msg, ".log.text.text insert end \"%s\" tabel", longmsg);
cmd(inter, msg);

for(j=0; j<nt; j++)
{h=list_times[j];
 first=true;
 for(av=var=num=0, i=0; i<nv; i++)
  {
  if(h>=start[i] && h<=end[i] && !isnan(data[i][h]))		// ignore NaNs
  {
  if(first)
  {
     ymin=ymax=data[i][h];
	 first=false;
  }
   if(data[i][h]<ymin)
    ymin=data[i][h];
   if(data[i][h]>ymax)
    ymax=data[i][h];
   av+=data[i][h];
   num++;
   var+=data[i][h]*data[i][h];
  }
 }

 if(num>1)
 {av=av/num;
 var=var/num-av*av;
 sig=sqrt(var*num/(num-1));
 }
 else
  var=sig=0;
 if(num>0)
 {
 sprintf(str1, "Case %d (%.*g)\t",h, pdigits, num);
 sprintf(msg, ".log.text.text insert end \"%s\" tabel", str1 );
 cmd(inter, msg);

 sprintf(longmsg, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", pdigits, av, pdigits, var, pdigits, ymin, pdigits, ymax, pdigits, sig);
 sprintf(msg, ".log.text.text insert end \"%s\" tabel", longmsg );
 cmd(inter, msg);
 }
}
plog("\n");
delete[] list_times;
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }
delete[] str;
delete[] tag;
delete[] logdata;
delete[] data;
delete[] start;
delete[] end;


} //end Statistics_cross


void sort_on_end(store *app)
{

qsort((void *)app, num_var, sizeof(vs[0]), sort_labels_down);

}
/*
Sorting function for presenting variables' labels in a nice way.
The variables are grouped according to:
1) their label (increasing: A first z last)
	2) time of their last occurrence (decreasing: existing variable first)
   	3) time of their first occurrence (increasing: first born first)
      	4) lsd internal ID indexing system (used for the tag) (increasing)
The function is complicated for the point 4) by the fact that the tag is recorded
in the labels as a single string using the underscore '_' as joining character.

*/
int sort_labels_down(const void *a, const void *b)
{
int a_int, b_int, counter;
int diff;
diff=strcmp(((store *)a)->label, ((store *)b)->label);
if(diff!=0)
 return diff;
else
 if(((store *)a)->end != ((store *)b)->end)
  return ((store *)b)->end - ((store *)a)->end;
 else
  if(((store *)a)->start != ((store *)b)->start)
    return ((store *)a)->start - ((store *)b)->start;
  else
    {
     for(counter=0; ; )
      {
       a_int=atoi(((store *)a)->tag+counter);
       b_int=atoi(((store *)b)->tag+counter);
       if(a_int!=b_int)
        return a_int-b_int;
       while(((store *)a)->tag[counter]!='_')
        counter++;
       counter++;
      }
    }
}


/***************************************************
PLOT_GNU
Draws the XY graphs, with the first series as X and the others as Y's
****************************************************/
void plot_gnu(int *choice)
{

char *app;
char **str, **tag;
char str1[50], str2[100], str3[10], dirname[300];
FILE *f, *f2;
double **data,**logdata;

int i, nv,nanv=0, j, k, *start, *end, done, box;
int idseries, ndim, gridd;

cmd(inter, "set choice [.f.vars.ch.v size]");
nv=*choice;
*choice=0;
if(nv==0)
 {
 cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"No series selected.\\nBring some series in the 'Vars. to plot' listbox.\"");
 return;
 }

if(nv>2)
 {
 cmd(inter, "toplevel .s");
 cmd(inter, "wm title .s \"2D or 3D?\"");
 cmd(inter, "wm transient .s .");

 cmd(inter, "label .s.l -text \"Choose the type of graph\"");

 cmd(inter, "frame .s.d -relief groove -bd 2");
 cmd(inter, "set ndim 2");
 cmd(inter, "radiobutton .s.d.2d -text \"2D graph\" -variable ndim -value 2");
 cmd(inter, "radiobutton .s.d.3d -text \"3D graph \" -variable ndim -value 3");

 cmd(inter, "pack .s.d.2d .s.d.3d -expand yes -fill x -anchor w");

cmd(inter, "frame .s.o -relief groove -bd 2");

cmd(inter, "label .s.o.l -text \"Select 3D options\"");

cmd(inter, "if { [info exist box]==1} {} {set box 0}");
cmd(inter, "radiobutton .s.o.a -text \"Use 1st and 2nd vars as plane\" -variable box -value 0 -anchor w");
cmd(inter, "radiobutton .s.o.c -text \"Use time and 1st var. as plane\" -variable box -value 2 -anchor w");
cmd(inter, "radiobutton .s.o.b -text \"Use time and rank as plane\" -variable box -value 1 -anchor w");
cmd(inter, "checkbutton .s.o.g -text \"Use gridded data\" -variable gridd -anchor w");
cmd(inter, "checkbutton .s.o.p -text \"Draw palette-mapped surface\" -variable pm3d -anchor w");

cmd(inter, "pack .s.o.l .s.o.a .s.o.c .s.o.b .s.o.g .s.o.p -expand yes -fill x -anchor w");

cmd(inter, "frame .s.w -relief groove -bd 2");
cmd(inter, "label .s.w.l -text \"Select window type\"");
cmd(inter, "set wind 1");
cmd(inter, "radiobutton .s.w.g -text \"Lsd window\" -variable wind -value 1 -anchor w");
cmd(inter, "radiobutton .s.w.p -text \"Gnuplot window\" -variable wind -value 2 -anchor w");
cmd(inter, "pack .s.w.l .s.w.g .s.w.p -expand yes -fill x -anchor w");

 cmd(inter, "frame .s.b");
 cmd(inter, "set p .s.b");
 cmd(inter, "button $p.ok -text Ok -command {set choice 1}");
 cmd(inter, "button $p.can -text Cancel -command {set choice 2}");
 cmd(inter, "button $p.help -text Help -command {LsdHelp mdatares.html#3dTime}");
 cmd(inter, "pack $p.ok $p.can $p.help -side left -expand yes -fill x");
 
 cmd(inter, "bind .s.b.ok <Return> {.s.b.ok invoke}");

 cmd(inter, "pack .s.l .s.d .s.o .s.w .s.b -expand yes -fill x");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w"); 
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w"); 
#endif
*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd(inter, "destroy .s");

if(*choice==2)
 return;
cmd(inter, "set choice $ndim");
ndim=*choice;
 }
else
 ndim=2; 

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {

  data[i]=find_data(idseries); 

   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!isnan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 sprintf(msg,"\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n",i+1,j);
		 plog(msg);
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }
  else
   nanv++; 

 }

if(autom_x==1||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

if(autom==1||miny>=maxy)
{

for(done=0, i=1; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && !isnan(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
 }


} //End for finding min-max

   
sprintf(msg, "set dirxy plotxy_%d", cur_plot);
cmd(inter, msg);

cmd(inter, "file mkdir $dirxy");

getcwd(dirname, 300);
sprintf(msg, "plotxy_%d", cur_plot);
chdir(msg);
f=fopen("data.gp","w");
fprintf(f,"#");
if(nv>2)
{
  cmd(inter, "set choice $box");
  box=*choice;
  cmd(inter, "set choice $gridd");
  gridd=*choice;
}
else
  box=gridd=0;

if(box==0)
 {
 for(i=0; i<nv; i++)
  if(start[i]<=max_c && end[i]>=min_c)
     fprintf(f,"%s_%s\t", str[i], tag[i]);
     
 }    
else
 {
 if(gridd==0)
 {
  fprintf(f,"Time\t");
  if(box==1)
   {
  for(i=0; i<nv; i++)
   if(start[i]<=max_c && end[i]>=min_c)
     fprintf(f,"Var%d\t%s_%s\t",i, str[i], tag[i]);
   }
  else
   {
  for(i=0; i<nv; i++)
   if(start[i]<=max_c && end[i]>=min_c)
     fprintf(f,"%s_%s\t", str[i], tag[i]);   
   }
 }    
 else
 {
  fprintf(f,"Time\tRank\tVal");
 }    
 
 } 
fprintf(f,"\n");
if(box==0)
{
for(j=min_c; j<=max_c; j++)
 {
 for(i=0; i<nv; i++)
  if(start[i]<=max_c && end[i]>=min_c)
    {
    if(j>=start[i] && i<=end[i])
      fprintf(f,"%lf\t", data[i][j]);
    else
      fprintf(f,"nan\t");  
    
    }
 fprintf(f,"\n");
 }
}
else
{
//3D with time and rank
if(gridd==0)
 {//not gridded
 if(box==1)
 {
 for(j=min_c; j<=max_c; j++)
  {fprintf(f,"%d\t",j);
   for(i=0; i<nv; i++)
    if(start[i]<=max_c && end[i]>=min_c)
      {
      if(j>=start[i] && i<=end[i])
       fprintf(f,"%d\t%lf\t",i+1, data[i][j]);
      else
        fprintf(f,"%d\tnan\t", i+1);        
      }
   fprintf(f,"\n");
  }
 }
 else
 {
 for(j=min_c; j<=max_c; j++)
  {fprintf(f,"%d\t",j);
   for(i=0; i<nv; i++)
    if(start[i]<=max_c && end[i]>=min_c)
      {
      if(j>=start[i] && i<=end[i]) 
        fprintf(f,"%lf\t",data[i][j]);
      else
        fprintf(f,"nan\t");  
      } 
   fprintf(f,"\n");
  }
 
 } 
 }
else
 {//gridded
  for(i=0; i<nv; i++)
   {
    for(j=min_c; j<=max_c; j++)
    if(start[i]<=max_c && end[i]>=min_c)
     {
     if(j>=start[i] && i<=end[i])
       fprintf(f,"%d\t%d\t%lf\n",j,i+1,data[i][j]);
     else
       fprintf(f,"%d\t%d\tnan\n",j,i+1);         
     }
   }  
 } 
} 
fclose(f);

*choice=0;



f=fopen("gnuplot.lsd","w");
f2=fopen("gnuplot.gp","w");

fprintf(f, "set datafile missing \"nan\" \n");		//handle NaNs
fprintf(f2, "set datafile missing \"nan\" \n");
app=(char *)Tcl_GetVar(inter, "gpterm",0);

fprintf(f,"set term tkcanvas\n");
fprintf(f2,"set term %s\n", app);
fprintf(f,"set output 'plot.file'\n");

if(grid==1)
 {fprintf(f, "set grid\n");
  fprintf(f2, "set grid\n");
 }
if(line_point==2)
 {sprintf(str1,"set pointsize %lf\n", point_size);
  fprintf(f,"%s", str1);
  fprintf(f2,"%s", str1);
 }
if(ndim==2)
{
 sprintf(msg, "set yrange [%lf:%lf]\n", miny, maxy);
 fprintf(f, "%s",msg);
 fprintf(f2, "%s",msg);
} 

if(box==0)
  sprintf(msg, "set xlabel \"%s_%s\"\n", str[0], tag[0]);
else
  sprintf(msg, "set xlabel \"Time\"\n");  
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

if(ndim>2)
{
if(box==0)
 sprintf(msg, "set ylabel \"%s_%s\"\n", str[1], tag[1]);
else
  sprintf(msg, "set ylabel \"Series\"\n"); 
 fprintf(f, "%s",msg);
 fprintf(f2, "%s",msg);
} 

if(line_point==1 && ndim==2)
 sprintf(str1, "smooth csplines");
else
 if(line_point==1 && ndim>2)
  sprintf(str1, "with lines");
 else
  sprintf(str1, ""); 

if(ndim==2)
{
if(allblack==1 )
 sprintf(str3, " lt -1");
else
 sprintf(str3, "");
}
else
 {
  cmd(inter, "set choice $gridd");
 if(*choice==1)
  {
   app=(char *)Tcl_GetVar(inter, "gpdgrid3d",0);
   fprintf(f, "set dgrid3d %s\nset hidden3d\n", app);
   fprintf(f2, "set dgrid3d %s\nset hidden3d\n",app);
  }
 cmd(inter, "set choice $pm3d");
 if(*choice==1)
  {
   fprintf(f, "set pm3d\n");
   fprintf(f2, "set pm3d\n");
   sprintf(str1, "with pm3d ");
  }

  if(allblack==1)
   {
   fprintf(f, "set palette gray\n");
   fprintf(f2, "set palette gray\n");
   
   }
    
 }

app=(char *)Tcl_GetVar(inter, "gpooptions",0);
fprintf(f,"%s\n", app);
fprintf(f2,"%s\n", app);
 
if(ndim==2) 
 {
  sprintf(msg,"plot 'data.gp' using 1:2 %s t \"%s(%s)\"", str1, str[1], tag[1]);
  if(allblack==1 )
   strcat(msg, str3);
  i=2;
 } 
else
 {

  if(box==0)
    {sprintf(msg,"splot 'data.gp' using 1:2:3 %s t \"%s(%s)\"", str1, str[2], tag[2]); 
     i=3;
    } 
  else
   {
    i=1;  
    if(box==1)
      sprintf(msg,"splot 'data.gp' using 1:2:3 %s t \"\"", str1); 
    else 
      {
       sprintf(msg,"splot 'data.gp' using 1:2:%d %s t \"\"",(nv-nanv)/2+2, str1);   
       i=2;
      } 
   } 
 } 

fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

for( ; i<nv; i++)
 if(start[i]<=max_c && end[i]>=min_c)
  {
   if(ndim==2)
    sprintf(str2,", 'data.gp' using 1:%d %s t \"%s(%s)\"",i+1, str1, str[i], tag[i]);
   else
    {if(box==0 )
      sprintf(str2,", 'data.gp' using 1:2:%d %s t \"%s(%s)\"",i+1, str1, str[i], tag[i]); 
     else
      if(gridd==0)
        {
         if(box==1)
           sprintf(str2,", 'data.gp' using 1:%d:%d %s t \"\"",2+2*i, 2*i+3, str1); 
         else
          if(i<=nv/2)
           sprintf(str2,", 'data.gp' using 1:%d:%d %s t \"\"",i+1, (nv-nanv)/2+i+1, str1); 
          else
           strcpy(str2,"");     
        } 
      else
       strcpy(str2,"");  
    }  

   if(strlen(str2)>0 && allblack==1 )
     strcat(str2, str3);
   fprintf(f, "%s",str2);
   fprintf(f2, "%s",str2);


  }

fprintf(f, "\n");
fprintf(f2, "\n");

fprintf(f2, "pause -1 \"Close graph %d)\"\n", cur_plot);

fclose(f);
fclose(f2);

if(nv>2)
 cmd(inter, "set choice $wind");
else
 *choice=0;

show_plot_gnu(cur_plot, choice, *choice);

chdir(dirname);

end:
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }

delete[] str;
delete[] tag;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;

}

/*
PLOT_CS_XY
*/
void plot_cs_xy(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[500], str2[500], str3[10], dirname[300];
FILE *f, *f2;
double **data,**logdata;
int i, nv, j, k, *start, *end, done, color;
int time_sel, block_length, ndim;

double previous_row;
cmd(inter, "set choice [.f.vars.ch.v size]");
nv=*choice;
*choice=0;
if(nv==0)
 {
 cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"No series selected.\\nBring some series in the 'Vars. to plot' listbox.\"");
 return;
 }

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];


str=new char *[nv];
tag=new char *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {
  data[i]=find_data(idseries); 
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!isnan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 sprintf(msg,"\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n",i+1,j);
		 plog(msg);
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }

 }



if(autom_x==1||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}


if(autom==1||miny>=maxy)
{

for(done=0, i=1; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && !isnan(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
 }

} //End for finding min-max

*choice=0;
/********/
sprintf(msg, "set bidi %d", end[0]);
cmd(inter, msg);

cmd(inter, "toplevel .s");
cmd(inter, "wm title .s \"Options\"");
cmd(inter, "wm transient .s .");
cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "label .s.i.l -text \"Insert time step to use\"");
cmd(inter, "entry .s.i.e -textvariable bidi");
cmd(inter, "pack .s.i.l .s.i.e -expand yes -fill x");

cmd(inter, "frame .s.d -relief groove -bd 2");

cmd(inter, "label .s.d.l -text \"Select the type of graph\"");
cmd(inter, "radiobutton .s.d.2d -text \"2D graph\" -variable ndim -value 2");
cmd(inter, "radiobutton .s.d.3d -text \"3D graph\" -variable ndim -value 3");

cmd(inter, "pack .s.d.l .s.d.2d .s.d.3d -expand yes -fill x -anchor w");

cmd(inter, "frame .s.o -relief groove -bd 2");

cmd(inter, "label .s.o.l -text \"Select 3D options\"");
cmd(inter, "checkbutton .s.o.g -text \"Use gridded data\" -variable gridd -anchor w");
cmd(inter, "checkbutton .s.o.p -text \"Draw palette-mapped surface\" -variable pm3d -anchor w");

cmd(inter, "pack .s.o.l .s.o.g .s.o.p -expand yes -fill x -anchor w");

cmd(inter, "set ndim 2");

cmd(inter, "set choice $ndim");
if(*choice==2)
 sprintf(msg, "set blength %d", (int)(nv/2));
else
 sprintf(msg, "set blength %d", (int)(nv/3));

cmd(inter, msg); 

cmd(inter, "set numv 1");
cmd(inter, "frame .s.v -relief groove -bd 2");
cmd(inter, "label .s.v.l -text \"Number of dependent variables:\"");
cmd(inter, "entry .s.v.e -textvariable numv");
cmd(inter, "label .s.v.n -text \"Num. of points: $blength\"");
cmd(inter, "pack .s.v.l .s.v.e .s.v.n -expand yes -fill x");
cmd(inter, "bind .s.v.e <KeyRelease> {set blength [expr $nnvar / ($numv + $ndim-1)]; .s.v.n conf -text \"Num. of points: $blength\"}");
sprintf(msg, "set nnvar %d", nv);
cmd(inter, msg);
cmd(inter, "bind .s.v.e <Return> {set blength [expr $nnvar / ($numv + $ndim-1)]; .s.v.n conf -text \"Num. of points: $blength\"}");
cmd(inter, "bind .s.v.e <Tab> {set blength [expr $nnvar / ($numv + $ndim-1)]; .s.v.n conf -text \"Num. of points: $blength\"}");
cmd(inter, "bind .s.d.2d <Return> {set blength [expr $nnvar / ($numv + $ndim-1)]; .s.v.n conf -text \"Num. of points: $blength\"}");
cmd(inter, "bind .s.d.2d <ButtonRelease-1> {set ndim 2; set blength [expr $nnvar / ($numv + $ndim-1)]; .s.v.n conf -text \"Num. of points: $blength\"}");
cmd(inter, "bind .s.d.3d <ButtonRelease-1> {set ndim 3; set blength [expr $nnvar / ($numv + $ndim-1)]; .s.v.n conf -text \"Num. of points: $blength\"}");
cmd(inter, "bind .s.d.2d <Down> {.s.d.3d invoke; focus -force .s.d.3d; set ndim 3; set blength [expr $nnvar / ($numv + $ndim-1)]; .s.v.n conf -text \"Num. of points: $blength\"}");
cmd(inter, "bind .s.d.3d <Up> {.s.d.2d invoke; focus -force .s.d.2d; set ndim 2; set blength [expr $nnvar / ($numv + $ndim-1)]; .s.v.n conf -text \"Num. of points: $blength\"}");


cmd(inter, "frame .s.l -relief groove -bd 2");
cmd(inter, "label .s.l.l -text \"Select the block length\"");
cmd(inter, "pack .s.l.l ");

cmd(inter, "frame .s.w -relief groove -bd 2");
cmd(inter, "label .s.w.l -text \"Select window type\"");
cmd(inter, "set wind 1");
cmd(inter, "radiobutton .s.w.g -text \"Lsd window\" -variable wind -value 1 -anchor w");
cmd(inter, "radiobutton .s.w.p -text \"Gnuplot window\" -variable wind -value 2 -anchor w");
cmd(inter, "pack .s.w.l .s.w.g .s.w.p -expand yes -fill x -anchor w");


cmd(inter, "frame .s.b");
cmd(inter, "set p .s.b");
cmd(inter, "button $p.ok -text Ok -command {set choice 1}");
cmd(inter, "button $p.can -text Cancel -command {set choice 2}");
cmd(inter, "button $p.help -text Help -command {LsdHelp mdatares.html#3dCrossSection}");
cmd(inter, "pack $p.ok $p.can $p.help -side left -expand yes -fill x");

cmd(inter, "bind .s.b.ok <Return> {.s.b.ok invoke}");

cmd(inter, "pack .s.i .s.d .s.o .s.v .s.w .s.b -expand yes -fill x");
cmd(inter, "focus .s.i.e; .s.i.e selection range 0 end");

//cmd(inter, "bind .s <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
cmd(inter, " .s.i.e selection range 0 end");
cmd(inter, "bind .s.i.e <KeyPress-Return> {if {$ndim == 2} { focus -force .s.d.2d } {focus -force .s.d.3d}}");
cmd(inter, "bind .s.d.2d <KeyPress-Return> {.s.v.e selection range 0 end; focus -force .s.v.e}");
cmd(inter, "bind .s.d.3d <KeyPress-Return> {.s.v.e selection range 0 end; focus -force .s.v.e}");

cmd(inter, "bind .s.v.e <KeyPress-Return> {focus -force .s.b.ok}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd(inter, "destroy .s");

if(*choice==2)
 goto end;

cmd(inter, "set choice $ndim");
ndim=*choice;

cmd(inter, "set choice $bidi");
time_sel=*choice;

cmd(inter, "set blength [expr $nnvar / ($numv + $ndim-1)]");
cmd(inter, "set choice $blength");
block_length=*choice;
*choice=0;

cmd(inter, "destroy .s");

if(nv%block_length!=0)
 {
  cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Block length is incorrect. It should be an exact divisor of the number of Variables.\"");
  goto end;
 }

//here we are
sprintf(msg, "set dirxy plotxy_%d", cur_plot);
cmd(inter, msg);

cmd(inter, "file mkdir $dirxy");
getcwd(dirname, 300);
sprintf(msg, "plotxy_%d",cur_plot);
chdir(msg);

f=fopen("data.gp","w");
if(start[0] == end[0])  
  previous_row=data[0][end[0]];
else
  previous_row=data[0][time_sel];  
  
for(j=0; j<block_length; j++)
 {

if(start[0] == end[0])  
 {
  if(data[j][end[j]]!=previous_row)
   {
    // fprintf(f,"\n");
     previous_row=data[j][end[j]];
   }
 }
else
 {
  if(data[j][time_sel]!=previous_row)
   {
     //fprintf(f,"\n");
     previous_row=data[j][time_sel];
   }
 }
 

 for(i=0; i<nv; i+=block_length)
  {
  if(start[i+j] == end[i+j])  
     fprintf(f,"%lf\t", data[i+j][end[i+j]]);
  else
   {
    if(start[i+j]<=max_c && end[i+j]>=min_c)
      fprintf(f,"%lf\t", data[i+j][time_sel]);
   }
  } 
 fprintf(f,"\n");
 }
fclose(f);

f=fopen("gnuplot.lsd","w");
f2=fopen("gnuplot.gp","w");

fprintf(f, "set datafile missing \"nan\" \n");		//handle NaNs
fprintf(f2, "set datafile missing \"nan\" \n");
app=(char *)Tcl_GetVar(inter, "gpterm",0);
fprintf(f2,"set term %s\n", app);
fprintf(f,"set term tkcanvas\n");
fprintf(f,"set output 'plot.file'\n");

if(grid==1)
 {
  fprintf(f, "set grid\n");
  fprintf(f2, "set grid\n");
 }
if(line_point==2)
 { fprintf(f, "set pointsize %lf\n", point_size);
   fprintf(f2, "set pointsize %lf\n", point_size);
   if(ndim==2)
    sprintf(str2, "");
   else
    sprintf(str2, "with points ");       
 }
else
 {
 if(ndim==2)
  sprintf(str2, "smooth csplines ");
 else
   sprintf(str2, "with lines "); 
 }

sprintf(msg, "set xlabel \"%s_%s\"\n", str[0], tag[0]);
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

if(ndim==3)
 {
 sprintf(msg, "set ylabel \"%s_%s\"\n", str[block_length], tag[block_length]);
 fprintf(f, "%s",msg);
 fprintf(f2, "%s",msg);
 } 

if(allblack==1)
 sprintf(str3, " lt -1");
else
  str3[0]=0;
 //sprintf(str3, "");

if(ndim==3)
 {
// fprintf(f, "set dgrid3d 30,30,4\nset hidden3d\nset pm3d\n");
// fprintf(f2, "set dgrid3d 30,30,4\nset hidden3d\nset pm3d\n");
 cmd(inter, "set choice $gridd");
 if(*choice==1)
  {
   app=(char *)Tcl_GetVar(inter, "gpdgrid3d",0);
   fprintf(f, "set dgrid3d %s\nset hidden3d\n", app);
   fprintf(f2, "set dgrid3d %s\nset hidden3d\n", app);
  }
 cmd(inter, "set choice $pm3d");
 if(*choice==1)
  {sprintf(str2, "with pm3d ");
   fprintf(f, "set pm3d\n");
   fprintf(f2, "set pm3d\n");
  }

 if(allblack==1)
  {
   sprintf(str3, "");
  fprintf(f, "set palette gray\n");
  fprintf(f2, "set palette gray\n");
   
  } 

  
 }
if(ndim==2)
 {
  sprintf(msg,"plot 'data.gp' using 1:2 %s t \"%s_%s(%d)\"",str2, str[block_length], tag[block_length], time_sel);
  if(allblack==1 )
   strcat(msg, str3);
  i=2; //init from the second variable
 } 
else
 {
  sprintf(msg,"splot 'data.gp' using 1:2:3 %s t \"%s_%s(%d)\"",str2, str[2*block_length], tag[2*block_length], time_sel); 
 i=3;
 }  
for(; i<nv/block_length; i++)
 {j=i*block_length;
 if(start[j]<=max_c && end[j]>=min_c)
  {
   if(ndim==2)
    {
     sprintf(str1,", 'data.gp' using 1:%d %s t \"%s_%s(%d)\"",i+1, str2, str[j], tag[j], time_sel);
     if(allblack==1 )
      strcat(str1, str3);
    }  
   else
    sprintf(str1,", 'data.gp' using 1:2:%d %s t \"%s_%s(%d)\"",i+1, str2, str[j], tag[j], time_sel); 
   strcat(msg,str1);

  }
 }
strcat(msg,"\n");
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

fprintf(f2, "pause -1 \"(Close graph %d)\"\n", cur_plot);
fclose(f);
fclose(f2);
cmd(inter, "set choice $wind");
show_plot_gnu(cur_plot, choice, *choice);
chdir(dirname);
end:
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }

delete[] str;
delete[] tag;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;

}


/***************************************************
PLOT_PHASE_DIAGRAM
****************************************************/
void plot_phase_diagram(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[50], str2[100], str3[100], dirname[300];
FILE *f, *f2;
double **data,**logdata;

int i, nv, j, k, *start, *end, done, nlags;


cmd(inter, "set choice [.f.vars.ch.v size]");
if(*choice!=1)
 {
 cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Wrong number of series.\\nOne and only one series must be selected\"");
 return;
 }

nv=1; //ridiculous, but I am recycling the code

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries);
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!isnan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 sprintf(msg,"\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n",i+1,j);
		 plog(msg);
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }

 }



if(autom_x==1||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}


if(autom==1||miny>=maxy)
{

for(done=0, i=0; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && !isnan(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && !isnan(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
 }


} //End for finding min-max

cmd(inter, "set bidi 1");
cmd(inter, "toplevel .s");
cmd(inter, "wm transient .s .");
cmd(inter, "wm title .s \"Lags to plot\"");
cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "label .s.i.l -text \"Insert number of lags\"");
cmd(inter, "entry .s.i.e -textvariable bidi");
cmd(inter, "set dia 0");
cmd(inter, "checkbutton .s.i.arrow -text \"Diagonal\" -variable dia");
cmd(inter, "pack .s.i.l .s.i.e .s.i.arrow");

cmd(inter, "frame .s.w -relief groove -bd 2");
cmd(inter, "label .s.w.l -text \"Select window type\"");
cmd(inter, "set wind 1");
cmd(inter, "radiobutton .s.w.g -text \"Lsd window\" -variable wind -value 1 -anchor w");
cmd(inter, "radiobutton .s.w.p -text \"Gnuplot window\" -variable wind -value 2 -anchor w");
cmd(inter, "pack .s.w.l .s.w.g .s.w.p -expand yes -fill x");

cmd(inter, "frame .s.b");
cmd(inter, "set p .s.b");
cmd(inter, "button $p.ok -text Ok -command {set choice 1}");
cmd(inter, "button $p.can -text Cancel -command {set choice 2}");
cmd(inter, "button $p.help -text Help -command {LsdHelp mdatares.html#plot}");
cmd(inter, "pack $p.ok $p.can $p.help -side left -expand yes -fill x");



cmd(inter, "pack .s.i .s.w .s.b");
cmd(inter, "focus .s.i.e; .s.i.e selection range 0 end");

cmd(inter, "bind .s <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
cmd(inter, " .s.i.e selection range 0 end");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd(inter, "destroy .s");
if(*choice==2)
 goto end;
sprintf(msg, "set dirxy plotxy_%d", cur_plot);
cmd(inter, msg);

cmd(inter, "file mkdir $dirxy");
getcwd(dirname, 300);
sprintf(msg, "plotxy_%d",cur_plot);
chdir(msg);


cmd(inter, "set choice $bidi");
nlags=*choice;

f=fopen("data.gp","w");
fprintf(f,"#");
for(i=0; i<=nlags; i++)
  if(start[0]<=max_c && end[0]>=min_c)
     fprintf(f,"%s_%s(%d)\t", str[0], tag[0], i);
fprintf(f,"\n");
for(j=min_c; j<=max_c-nlags; j++)
 {
 for(i=0; i<=nlags; i++)
  if(start[0]<=max_c && end[0]>=min_c)
    fprintf(f,"%lf\t", data[0][j+i]);
 fprintf(f,"\n");
 }
fclose(f);

*choice=0;
f=fopen("gnuplot.lsd","w");
f2=fopen("gnuplot.gp","w");
fprintf(f, "set datafile missing \"nan\" \n");		//handle NaNs
fprintf(f2, "set datafile missing \"nan\" \n");
fprintf(f,"set term tkcanvas\n");
fprintf(f,"set output 'plot.file'\n");

if(grid==1)
 {fprintf(f, "set grid\n");
  fprintf(f2, "set grid\n");
 }
if(line_point==2)
 {sprintf(str1,"set pointsize %lf\n", point_size);
  fprintf(f,"%s", str1);
  fprintf(f2,"%s", str1);
 }
sprintf(msg, "set yrange [%lf:%lf]\n", miny, maxy);
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);
sprintf(msg, "set xlabel \"%s_%s\"\n", str[0], tag[0]);
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

cmd(inter, "set choice $dia");
if(*choice==1)
 {
  fprintf(f,"set arrow from %lf,%lf to %lf,%lf lt -1\n",miny, miny, maxy, maxy);
  fprintf(f2,"set arrow from %lf,%lf to %lf,%lf lt -1\n",miny, miny, maxy, maxy);
 }
if(line_point==1)
 sprintf(str1, "smooth csplines");
else
 sprintf(str1, "");

if(allblack==1)
 sprintf(str3, " lt -1");
else
 str3[0]=0;
 //sprintf(str3, "");

 sprintf(msg,"plot 'data.gp' using 1:2 %s t \"t+1\"", str1 );
 if(allblack==1 )
  strcat(msg, str3);
for(i=2; i<=nlags; i++)
 if(start[0]<=max_c && end[0]>=min_c)
  {sprintf(str2,", 'data.gp' using 1:%d %s t \"t+%d\"",i+1, str1, i);
   strcat(msg,str2);
   if(allblack==1 )
    strcat(msg, str3);

  }

 strcat(msg,"\n");
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

fprintf(f2, "pause -1 \"Close graph %d)\"\n", cur_plot);

fclose(f);
fclose(f2);
cmd(inter, "set choice $wind");
show_plot_gnu(cur_plot, choice, *choice);
chdir(dirname);
end:
for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
  if(logs)
    delete[] logdata[i];
 }

delete[] str;
delete[] tag;

delete[] logdata;
delete[] data;
delete[] start;
delete[] end;

}

void show_plot_gnu(int n, int *choice, int type)
{

char dirname[200];

cmd(inter, "if { $tcl_platform(platform)==\"unix\" } {set choice 1 } {set choice 0} ");
if(*choice==1)
  sprintf(dirname, "plotxy_%d/", cur_plot);
else
  sprintf(dirname, "plotxy_%d\\", cur_plot);



//cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {exec gnuplot gnuplot.lsd} {if {$tcl_platform(os) == \"Windows NT\"} { if { $tcl_platform(os) == \"4.0\" } {exec cmd /c start wgnuplot gnuplot.lsd} {exec wgnuplot gnuplot.lsd} } {exec start wgnuplot gnuplot.lsd}}");
cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {set choice 1} {if {$tcl_platform(os) == \"Windows NT\"} { if { $tcl_platform(os) == \"4.0\" } {set choice 2} {set choice 3} } {set choice 4}}");

switch (*choice)
{
//unix
case 1: sprintf(msg, "exec gnuplot gnuplot.lsd");
break;
//Win NT
case 2: sprintf(msg, "exec cmd /c start wgnuplot gnuplot.lsd");
break;
//Win 2K
case 3: sprintf(msg, "exec wgnuplot gnuplot.lsd");
break;
//Win 95/98/ME
case 4: sprintf(msg, "exec start wgnuplot gnuplot.lsd");
break;


}

cmd(inter, msg);
sprintf(msg, "set p .f.new%d", n);
cmd(inter, msg);

cmd(inter, "toplevel $p");
cmd(inter, "wm title $p $tit");
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/lsd.xbm} {}");
cmd(inter, "bind $p <Double-Button-1> {raise .}");
cmd(inter, "frame $p.f");
cmd(inter, "canvas $p.f.plots -height 430 -width 640 -bg white");

cmd(inter, "pack $p.f.plots");
cmd(inter, "pack $p.f");
cmd(inter, "update");
//cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Step 2\\n\"");
shrink_gnufile();
cmd(inter, "file delete plot.file; file rename plot_clean.file plot.file");

//cmd(inter, "file stat plot.file stat_data");
//cmd(inter, "if { $stat_data(size) > 500000} {set choice 1} {set choice 0}");
if(type==0)
{
  cmd(inter, "set answer [tk_messageBox -type yesno -title \"Option\" -message \"The requested graph may be generated with higher quality by using Gnuplot external from Lsd.\\nPress 'Yes' to generate a low-quality graph within Lsd or 'No' to generate a high-quality graph with Gnuplot.\"]");
  cmd(inter, "if {[string compare $answer \"yes\"] == 0} { set choice 1} {set choice 0}");
  if(*choice ==0)
   type=2;
  else
   type=1;
}   
   



if(type==2)
 {//plot with external gnuplot
 cmd(inter, "destroy $p");
   cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {exec xterm -e gnuplot gnuplot.gp &} {if {$tcl_platform(os) == \"Windows NT\"} {exec wgnuplot gnuplot.gp &} {exec start wgnuplot gnuplot.gp &}}");
 *choice=0;
 return;
 }



cmd(inter, "source plot.file");

cmd(inter, "update");
cmd(inter, "update idletasks");
cmd(inter, "set choice 0");

// cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Step 3\\n\"");
  cmd(inter, "catch [gnuplot $p.f.plots]");
cmd(inter, "catch [rename gnuplot \"\"]");
// cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Step 4\\n\"");

}

void plot_lattice(int *choice)
{

char *app;

char str1[50], str2[100], str3[100];
FILE *f, *f2;
double **data;

int i, nv, j, hi, le, done, nlags, ncol, nlin, end;


cmd(inter, "set choice [.f.vars.ch.v size]");
/*
if(*choice!=1)
 {
 cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Wrong number of series.\\nOne and only one series must be selected\"");
 return;
 }
*/
nv=*choice;
data=new double *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }


cmd(inter, "set res [.f.vars.ch.v get 0]");
cmd(inter, "scan $res \"%s %s (%d - %d) # %d\" a b c d choice");
end=vs[*choice].end;

for(i=0; i<nv; i++)
 {

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  cmd(inter, "scan $res \"%s %s (%d - %d) # %d\" a b c d choice");
  data[i]=vs[*choice].data;

 }



cmd(inter, "set bidi 1");
cmd(inter, "toplevel .s");
cmd(inter, "wm title .s \"Insert\"");
cmd(inter, "wm transient .s .");
cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "label .s.i.l -text \"Insert number of columns\"");
cmd(inter, "entry .s.i.e -textvariable bidi");
cmd(inter, "pack .s.i.l .s.i.e");
sprintf(msg, "set time %d", end);
cmd(inter, msg);
cmd(inter, "frame .s.t -relief groove -bd 2");
cmd(inter, "label .s.t.l -text \"Insert time step to use\"");
cmd(inter, "entry .s.t.e -textvariable time");
cmd(inter, "pack .s.t.l .s.t.e");

cmd(inter, "set lx 400");
cmd(inter, "frame .s.x -relief groove -bd 2");
cmd(inter, "label .s.x.l -text \"Lattice width\"");
cmd(inter, "entry .s.x.e -textvariable lx");
cmd(inter, "pack .s.x.l .s.x.e");

cmd(inter, "set ly 400");
cmd(inter, "frame .s.y -relief groove -bd 2");
cmd(inter, "label .s.y.l -text \"Lattice heigth\"");
cmd(inter, "entry .s.y.e -textvariable ly");
cmd(inter, "pack .s.y.l .s.y.e");


cmd(inter, "frame .s.b");
cmd(inter, "set p .s.b");
cmd(inter, "button $p.ok -text Ok -command {set choice 1}");
cmd(inter, "button $p.can -text Cancel -command {set choice 2}");
cmd(inter, "button $p.help -text Help -command {LsdHelp mdatares.html#lattice}");
cmd(inter, "pack $p.ok $p.can $p.help -side left -expand yes -fill x");



cmd(inter, "pack .s.t .s.i .s.x .s.y .s.b");
cmd(inter, "focus .s.t.e; .s.t.e selection range 0 end");


cmd(inter, "bind .s.t.e <KeyPress-Return> {focus .s.i.e; .s.i.e selection range 0 end}");
cmd(inter, "bind .s.i.e <KeyPress-Return> {focus .s.x.e; .s.x.e selection range 0 end}");
cmd(inter, "bind .s.x.e <KeyPress-Return> {focus .s.y.e; .s.y.e selection range 0 end}");
cmd(inter, "bind .s.y.e <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd(inter, "destroy .s");
if(*choice==2)
 {
 cur_plot--;
 *choice=1;
 goto end;
 }

cmd(inter, "set choice $time");
nlags=*choice;
cmd(inter, "set choice $bidi");
ncol=*choice;

nlin=nv/ncol;
if(nlin*ncol!=nv)
 {
 cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Wrong number of columns.\"");
 cur_plot--;
 *choice=1;
 goto end;
 }

cmd(inter, "set choice $ly");
hi=*choice/nlin;
cmd(inter, "set choice $lx");
le=*choice/ncol;

sprintf(msg, "set p .f.new%d", cur_plot);
cmd(inter, msg);
cmd(inter, "toplevel $p");
cmd(inter, "wm title $p $tit");
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/lsd.xbm} {}");
cmd(inter, "bind $p <Double-Button-1> {raise .}");
sprintf(msg,"frame $p.f -width %d -height %d", ncol*le, nlin*hi);
cmd(inter, msg);


cmd(inter, "pack $p.f");

//Reset p to point to the canvas (shit...)
cmd(inter, "set p $p.f.plots");


sprintf(msg, "canvas $p -width %d -height %d -background white -relief flat", ncol*le, nlin*hi);
cmd(inter, msg);
cmd(inter, "pack $p");

if(grid==0)
{
for(j=0; j<nlin; j++)
 for(i=0; i<ncol; i++)
  {
  sprintf(msg, "$p create poly %d %d %d %d %d %d %d %d -fill $c%d", i*le, j*hi, i*le, (j+1)*hi, (i+1)*le, (j+1)*hi, (i+1)*le, j*hi, (int)data[ncol*j+i][nlags]);
  cmd(inter, msg);
  }
}
else
{
for(j=0; j<nlin; j++)
 for(i=0; i<ncol; i++)
  {
  sprintf(msg, "$p create poly %d %d %d %d %d %d %d %d -fill $c%d -outline black", i*le, j*hi, i*le, (j+1)*hi, (i+1)*le, (j+1)*hi, (i+1)*le, j*hi, (int)data[ncol*j+i][nlags]);
  cmd(inter, msg);
  }
}

*choice=0;
end:


delete[] data;

}


/*********
WORK IN PROGRESS
object *skip_next_obj(object *t, int *count);

void add_list(object *r)
{
int i=0;
object *cur;
variable *cv;

for(cur=r; cur!=NULL; cur=skip_next_obj(object cur, &i))
 {for(cv=cur->v; cv!=NULL; cv=cv->next)
    {
     sprintf(msg, "lappend series_bylab \"%s (Obj. %s)\"",cv->label, cur->label);
     cmd(inter, msg);
     sprintf(msg, "lappend series_byobj \"(Obj. %s) %s\"", cur->label, cv->label);
     cmd(inter, msg);

    }
  if(cur->son!=NULL)
   add_list(cur->son);

 }

}

void list_series(object *r)
{
cmd(inter, "set a [info vars series_bylab]");
cmd(inter, "if {$a==\"\" } {} {unset series_bylab; unset series_byobj}");
add_list(r);
cmd(inter, "set temp [lsort -dictionary -integer $series_bylab]");
cmd(inter, "unset series_bylab");
cmd(inter, "set series_bylab $temp");
cmd(inter, "set temp [lsort -dictionary -integer $series_byobj]");
cmd(inter, "unset series_byobj");
cmd(inter, "set series_byobj $temp");

}
*********/



struct bin
{
double num;
double min;
double max;
double center;
double av;
double lowb;
double highb;
};	

/***************************************************
HISTOGRAMS
****************************************************/
void histograms(int *choice)
{

int idseries;
char *app;
char str[100];
char tag[100];

int start, end;
int i, num_bin, j, first, last, stat;
int x1, x2, y1,y2;
double ap, mx,mn, *data, step, a, b, s, lminy, miny2, truemaxy, truemaxy2, average, sigma, tot, totnorm;
bin *cl;

cmd(inter, "set choice [.f.vars.ch.v size]");
if(*choice!=1)
 {
  cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Wrong number of series.\\nFor Time Series histograms select only one series.\"");
  cur_plot--;
  sprintf(msg, ".f.vars.pl.v delete %d",cur_plot);
  cmd(inter, msg);
  return;
 } 
cmd(inter, "set res [.f.vars.ch.v get 0]");
app=(char *)Tcl_GetVar(inter, "res",0);
strcpy(msg,app);
sscanf(msg, "%s %s (%d - %d) # %d", str, tag, &start, &end, &idseries);

data=find_data(idseries);
if(autom_x==1)
 {first=start;
  last=end;
 }
else
 {
 if(min_c>start)
   first=min_c;
 else
   first=start;
 if(max_c<end)  
   last=max_c;
 else
   last=end;
 if(first>=last) 
  {
   first=start;
   last=end;
  }
 }  

for(tot=0, i=first; i<=last; i++)	// count number of points excluding NaNs
 if(!isnan(data[i]))				// ignore NaNs
  tot++;

*choice=0;
/********/
//sprintf(msg, "set bidi %d", 100<last-first?100:last-first);
sprintf(msg, "set bidi %d", 100<tot?100:(int)tot);
cmd(inter, msg);

cmd(inter, "toplevel .s");
cmd(inter, "wm title .s \"Number of classes\"");
cmd(inter, "wm transient .s .");
cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "label .s.i.l -text \"Insert the number of classes to use\"");
cmd(inter, "entry .s.i.e -textvariable bidi");
cmd(inter, "set norm 0");
cmd(inter, "checkbutton .s.i.norm -text \"Interpolate a Normal\" -variable norm");
cmd(inter, "set stat 0");
cmd(inter, "checkbutton .s.i.st -text \"Print statistics in Log window\" -variable stat");
cmd(inter, "pack .s.i.l .s.i.e .s.i.norm .s.i.st -anchor w");





cmd(inter, "frame .s.b");
cmd(inter, "set p .s.b");
cmd(inter, "button $p.ok -text Ok -command {set choice 1}");
cmd(inter, "button $p.can -text Cancel -command {set choice 2}");
cmd(inter, "button $p.help -text Help -command {LsdHelp mdatares.html#seq_xy}");
cmd(inter, "pack $p.ok $p.can $p.help -side left -expand yes -fill x");



cmd(inter, "pack .s.i .s.b");
cmd(inter, "focus .s.i.e; .s.i.e selection range 0 end");

//cmd(inter, "bind .s <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
cmd(inter, " .s.i.e selection range 0 end");
cmd(inter, "bind .s.i.e <KeyPress-Return> {set choice 1}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd(inter, "destroy .s");

if(*choice==2)
 {
 cur_plot--;
 sprintf(msg, ".f.vars.pl.v delete %d",cur_plot);
 cmd(inter, msg);
 return;
 }
 


cmd(inter, "set choice $bidi");
num_bin=*choice;
cl=new bin[num_bin];
ap=(double)num_bin;

tot=average=sigma=0;
for(i=first; i<=last; i++)
 {
 if(isnan(data[i]))		// ignore NaNs
  continue;
 if(i==first)
  mx=mn=data[i];
 else
  {
  if(data[i]>mx)
   mx=data[i];
  else
   if(data[i]<mn)
    mn=data[i];
  }  
  average+=data[i];
  sigma+=data[i]*data[i];
  tot++;
 }
average=average/tot;
sigma=sigma/tot-average*average;

for(i=0; i<num_bin; i++)
 {
 cl[i].num=0;
 cl[i].av=0;
 cl[i].min=0;
 cl[i].max=0;
 cl[i].center=0; 
  
 }
 

step=(mx-mn)/(ap); 
for(i=first; i<=last; i++)
 {
  if(isnan(data[i]))		// ignore NaNs
   continue;
  a=floor( ap*(data[i]-mn)/(mx-mn));
  s=ap*(data[i]-mn)/(mx-mn);
    
  j=(int)a;
  if(j==num_bin)
   j--;

  if(cl[j].num==0)
   {cl[j].min=cl[j].max=data[i];
   }
  else
   {
   if(cl[j].min>data[i])
    cl[j].min=data[i];
   else
   if(cl[j].max<data[i])
    cl[j].max=data[i];
   }  
  cl[j].num++;   
  cl[j].av+=data[i];
 } 

a=(mx-mn)/(ap-1);
for(i=1; i<num_bin; i++)
 {
  if(cl[i].num!=0 && cl[i-1].num!=0 && cl[i].min-cl[i-1].max<a)
    a=cl[i].min-cl[i-1].max;
 }


cmd(inter, "set choice $stat");
stat = *choice;

if(stat==1)
 plog("\n\n#    Boundaries(center)\t\tMin\tAve\tMax\tNum.\tFreq.");
step=(mx+a/2-(mn-a/2))/(ap);

//lminy=last-first;
lminy=tot;		// consider only non-NaNs
truemaxy=0;
for(i=0; i<num_bin; i++)
 {if(cl[i].num!=0)
    cl[i].av/=cl[i].num;
  cl[i].lowb=mn-a/2+(double)(i)*step;
  cl[i].highb=mn -a/2 + (double)(i+1)*step;
  cl[i].center=  cl[i].highb/2+  cl[i].lowb/2;
 if(stat==1)
 {
 sprintf(msg, "\n%3d: %.*g<=X<%.*g (%.*g)\t\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g", i+1, pdigits, mn-a/2+(double)(i)*step, pdigits, mn -a/2 + (double)(i+1)*step, pdigits, mn -a/2 +(double)(i)*step +step/2, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max, pdigits, cl[i].num, pdigits, cl[i].num/(tot));
 plog(msg);
 }
 if(cl[i].num<lminy)
  {
   lminy=cl[i].num;
   miny2=cl[i].num/tot;
  } 
 if(cl[i].num>truemaxy)
  {
   truemaxy=cl[i].num;
   truemaxy2=cl[i].num/tot;
  } 
 
 }

if(stat==1)
 cmd(inter, "wm deiconify .log; raise .log ."); 
if(autom==0 && miny<maxy)
 {
  lminy=miny;
  miny2=lminy/tot;
  truemaxy=maxy;
  truemaxy2=maxy/tot;
 }
else
 {
  cmd(inter, ".f.sc.max conf -state normal");
  cmd(inter, ".f.sc.min conf -state normal");  
  maxy=truemaxy;
  miny=lminy;
  cmd(inter, "update");
  cmd(inter, ".f.sc.max conf -state disabled");
  cmd(inter, ".f.sc.min conf -state disabled");  
 }


sprintf(msg, "set p .f.new%d", cur_plot);
cmd(inter, msg);

cmd(inter, "toplevel $p");
cmd(inter, "wm title $p $tit");
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/lsd.xbm} {}");
cmd(inter, "bind $p <Double-Button-1> {raise .; focus -force .b.ts}");
cmd(inter, "frame $p.f -width 640 -height 430");
cmd(inter, "pack $p.f");

//Reset p to point to the canvas (shit...)
cmd(inter, "set p $p.f.plots");

cmd(inter, "canvas $p -width 640 -height 430 -background white -relief flat");
cmd(inter, "pack $p");

cmd(inter, "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p");

if(grid==1)
{
cmd(inter, "$p create line 40 0 40 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 190 0 190 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 340 0 340 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 490 0 490 310 -fill grey60 -width 1p -tag p");
}
else
{
cmd(inter, "$p create line 640 295 640 305  -width 1 -tag p");
cmd(inter, "$p create line 190 295 190 305 -width 1 -tag p");
cmd(inter, "$p create line 340 295 340 305 -width 1 -tag p");
cmd(inter, "$p create line 490 295 490 305 -width 1 -tag p");
}


sprintf(msg, "set choice [$p create text 40 312 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,cl[0].lowb);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

i=num_bin/4;
sprintf(msg, "set choice [$p create text 190 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


if(num_bin%2==0)
 a=cl[num_bin/2].lowb;
else
 a=cl[(num_bin-1)/2].center; 
i=num_bin/2;
sprintf(msg, "set choice [$p create text 340 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p  text}]", pdigits,a);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


i=(int)((double)(num_bin)*.75);
sprintf(msg, "set choice [$p create text 490 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 635 312 -font {Times 10 normal} -anchor ne -text %.*g -tag {p  text}]", pdigits,cl[num_bin-1].highb);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);



cmd(inter, "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p");

if(grid==1)
{
cmd(inter, "$p create line 38 300 640 300 -fill grey60 -tag p");
cmd(inter, "$p create line 38 225 640 225 -fill grey60 -tag p");
cmd(inter, "$p create line 38 150 640 150 -fill grey60 -tag p");
cmd(inter, "$p create line 38 75 640 75 -fill grey60 -tag p");
cmd(inter, "$p create line 38 0 640 0 -fill grey60 -tag p");
}
else
{
//cmd(inter, "$p create line 35 300 45 300  -tag p");
cmd(inter, "$p create line 35 225 45 225  -tag p");
cmd(inter, "$p create line 35 150 45 150  -tag p");
cmd(inter, "$p create line 35 75 45 75  -tag p");
cmd(inter, "$p create line 35 2 45 2 -tag p");
}

sprintf(msg, "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,lminy);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p  text}]", pdigits,(lminy+(truemaxy-lminy)/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)/2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)*3/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,(truemaxy));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 4 312 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,miny2);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 4 237 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 4 162 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


sprintf(msg, "set choice [$p create text 4 87 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)*3/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 16 -font {Times 10 normal} -anchor nw -text (%.*g) -tag {p text}]", pdigits,(truemaxy2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);



*choice=0;
for(i=0; i<num_bin; i++)
 {
  x1=40+(int)(600* (cl[i].lowb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  x2=40+(int)(600* (cl[i].highb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  y1=(int)(300-((cl[i].num-lminy)/(truemaxy-lminy))*300);
  y2=300;

  if(y1<301)
  {//in case of user-definde miny
  sprintf(msg,"$p create rect %d %d %d %d -tag p%d -width 1 -fill $c%d",x1,y1,x2,y2, i, 1);
  cmd(inter, msg);
sprintf(msg, "$p bind p%d <Enter> {.f.new%d.f.plots delete xpos; .f.new%d.f.plots create text 0 390 -font {Times 10 normal} -text \"Class %d, %.*g <= X < %.*g, (center=%.*g)\\nContains %.*g units (%.*g perc.). Actual values: min=%.*g, av.=%.*g, max=%.*g  \" -anchor w -tag xpos}",i,  cur_plot, cur_plot, i, pdigits, cl[i].lowb, pdigits, cl[i].highb, pdigits, cl[i].center, pdigits, cl[i].num, pdigits, 100*cl[i].num/tot, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max );

//sprintf(msg, "$p bind p%d <Enter> {.f.new%d.f.plots delete xpos; .f.new%d.f.plots create text 0 390 -font {Times 10 normal} -text \"Class %d, %g <= X < %g, min=%g center=%g max=%g.\\nContains %g units (%g perc.)\" -anchor w -tag xpos}",i,  cur_plot, cur_plot, i, cl[i].lowb, cl[i].highb, cl[i].min, cl[i].center, cl[i].max, cl[i].num, 100*cl[i].num/tot);

cmd(inter, msg);
sprintf(msg, "$p bind p%d <Leave> {.f.new%d.f.plots delete xpos}",i, cur_plot);
cmd(inter, msg);
  }
  if(*choice==1) //Button STOP pressed
    {
     i=num_bin;
    }
 if(watch==1)
	cmd(inter, "update");
  
 }

/***
just to check that the sum is one
*/
cmd(inter, "set choice $norm");
if(*choice==1)
{
 totnorm=0;
 for(i=0; i<num_bin; i++)
  {
   a=cl[i].lowb;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   s=cl[i].highb;
   ap=exp(-(s-average)*(s-average)/(2*sigma))/(sqrt(2*PI*sigma));
   totnorm+=(b+ap)/2;
 //  sprintf(msg, "\nn=%g, f=%g", (b+ap)/2,cl[i].num/tot);
 //  plog(msg);
  }
  
 /*******/
 for(i=0; i<num_bin-1; i++)
  {
   a=cl[i].center;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   b=b*tot/totnorm;
   b=300-((b-lminy)/(truemaxy-lminy))*300;    
   a=cl[i+1].center;  
   s=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   s=s*tot/totnorm;
   s=300-((s-lminy)/(truemaxy-lminy))*300;  
   
   x1=40+(int)(600* (cl[i].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
   x2=40+(int)(600* (cl[i+1].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
 
   sprintf(msg, "$p create line %d %d %d %d",x1,(int)b, x2, (int)s );
   cmd(inter, msg);
 
  }
} 
    


//cmd(inter, "$p create text 200 420 -font {Times 10 normal} -text \"Class num:\" -anchor w ");
//cmd(inter, "$p create text 380 420 -font {Times 10 normal} -text \"Freq: \" -anchor w ");


sprintf(msg, "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}",cur_plot);
cmd(inter, msg);
*choice=0;

delete[] cl;
}

/***************************************************
HISTOGRAMS CS
****************************************************/
void histograms_cs(int *choice)
{

int idseries;
char *app;
char **str;
char **tag;
double **data,**logdata;
int *start, *end;
int i, num_bin, j, first, last, stat, nv, time, active_v;
int x1, x2, y1,y2;
double ap, mx,mn,  step, a, b, s, lminy, miny2, truemaxy, truemaxy2, average, sigma, tot, totnorm;
bin *cl;

cmd(inter, "set choice [.f.vars.ch.v size]");
nv=*choice;
if(nv<2)
{
  cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Wrong number of series.\\nFor Cross Section histograms select more than 2 series.\"");
  cur_plot--;
  sprintf(msg, ".f.vars.pl.v delete %d",cur_plot);
  cmd(inter, msg);
}
data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];
if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  data[i]=find_data(idseries);
  
  /*****************
  IT IS CS, SO THE TIME MUST BE DECIDED YET
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries);
   }
  *********************/
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!isnan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 sprintf(msg,"\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d\n",i+1,j);
		 plog(msg);
	   }
	 data[i]=logdata[i];				// replace the data series
   }
 }





*choice=0;
/********/
sprintf(msg, "set bidi %d", 100<nv?100:nv);
cmd(inter, msg);

sprintf(msg, "set time %d", end[0]);
cmd(inter, msg);

cmd(inter, "toplevel .s");
cmd(inter, "wm title .s \"Number of classes\"");
cmd(inter, "wm transient .s .");
cmd(inter, "frame .s.t -relief groove -bd 2");
cmd(inter, "label .s.t.l -text \"Insert the time step to use\"");
cmd(inter, "entry .s.t.e -textvariable time");
cmd(inter, "bind .s.t.e <Return> {focus -force .s.i.e; .s.i.e selection range 0 end}");
cmd(inter, "pack .s.t.l .s.t.e -anchor w");

cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "label .s.i.l -text \"Insert the number of classes to use\"");
cmd(inter, "entry .s.i.e -textvariable bidi");
cmd(inter, "set norm 0");
cmd(inter, "checkbutton .s.i.norm -text \"Interpolate a Normal\" -variable norm");
cmd(inter, "set stat 0");
cmd(inter, "checkbutton .s.i.st -text \"Print statistics in Log window\" -variable stat");
cmd(inter, "pack .s.i.l .s.i.e .s.i.norm .s.i.st -anchor w");




cmd(inter, "frame .s.b");
cmd(inter, "set p .s.b");
cmd(inter, "button $p.ok -text Ok -command {set choice 1}");
cmd(inter, "button $p.can -text Cancel -command {set choice 2}");
cmd(inter, "button $p.help -text Help -command {LsdHelp mdatares.html#seq_xy}");
cmd(inter, "pack $p.ok $p.can $p.help -side left -expand yes -fill x");



cmd(inter, "pack .s.t .s.i .s.b");
cmd(inter, "focus .s.t.e; .s.t.e selection range 0 end");

//cmd(inter, "bind .s <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");

cmd(inter, "bind .s.i.e <KeyPress-Return> {set choice 1}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd(inter, "destroy .s");

if(*choice==2)
 {
 delete[] start;
 delete[] end;
 delete[] data;
 for(i=0; i<nv; i++)
  {
  delete[] str[i];
  delete[] tag[i];
  
  if(logs)
    delete[] logdata[i]; 
  }
 delete[] logdata;
 delete[] str;
 delete[] tag; 
 cur_plot--;
 sprintf(msg, ".f.vars.pl.v delete %d",cur_plot);
 cmd(inter, msg);
 
 return;
 }


cmd(inter, "set choice $bidi");
num_bin=*choice;
cmd(inter, "set choice $time");
time=*choice;

cl=new bin[num_bin];
ap=(double)num_bin;

tot=average=sigma=0;
active_v=0;
if(autom==1)
{//min and max fixed automatically
for(i=0; i<nv; i++)
 {
 if(start[i]<=time && end[i]>=time && !isnan(data[i][time]))		// ignore NaNs
 {
 if(active_v==0)
  mx=mn=data[i][time];
 else
  {
  if(data[i][time]>mx)
   mx=data[i][time];
  else
   if(data[i][time]<mn)
    mn=data[i][time];
  }  
  average+=data[i][time];
  sigma+=data[i][time]*data[i][time];
  tot++;
  active_v++;
  }
 }
}
else
{//min and max set by the user
mx=maxy;
mn=miny;
for(i=0; i<nv; i++)
 {
 if(start[i]<=time && end[i]>=time && !isnan(data[i][time]) && data[i][time]>=mn && data[i][time]<=mx)		// ignore NaNs
 {
  average+=data[i][time];
  sigma+=data[i][time]*data[i][time];
  tot++;
  active_v++;
  }
 }

}
if(tot==0)
	tot=1;
average=average/tot;
sigma=sigma/tot-average*average;

for(i=0; i<num_bin; i++)
 {
 cl[i].num=0;
 cl[i].av=0;
 cl[i].min=0;
 cl[i].max=0;  
 cl[i].center=0; 
 }
 

step=(mx-mn)/(ap); 
for(i=0; i<nv; i++)
 {
  if(start[i]<=time && end[i]>=time && !isnan(data[i][time]) && data[i][time]>=mn && data[i][time]<=mx)		// ignore NaNs
  {
  a=floor( ap*(data[i][time]-mn)/(mx-mn));
  s=ap*(data[i][time]-mn)/(mx-mn);
    
  j=(int)a;
  if(j==num_bin)
   j--;

  if(cl[j].num==0)
   {cl[j].min=cl[j].max=data[i][time];
   }
  else
   {
   if(cl[j].min>data[i][time])
    cl[j].min=data[i][time];
   else
   if(cl[j].max<data[i][time])
    cl[j].max=data[i][time];
   }  
  cl[j].num++;   
  cl[j].av+=data[i][time];
  
  }
 } 

a=(mx-mn)/(ap-1);
for(i=1; i<num_bin; i++)
 {
  if(cl[i].num!=0 && cl[i-1].num!=0 && cl[i].min-cl[i-1].max<a)
    a=cl[i].min-cl[i-1].max;
 }


cmd(inter, "set choice $stat");
stat = *choice;

if(stat==1)
 plog("\n\n#    Boundaries(center)\t\tMin\tAve\tMax\tNum.\tFreq.");
step=(mx+a/2-(mn-a/2))/(ap);

lminy=active_v;
truemaxy=0;
for(i=0; i<num_bin; i++)
 {
 if(cl[i].num!=0)
    cl[i].av/=cl[i].num;
  cl[i].lowb=mn-a/2+(double)(i)*step;
  cl[i].highb=mn -a/2 + (double)(i+1)*step;
  cl[i].center=  cl[i].highb/2+  cl[i].lowb/2;
 if(stat==1)
 {
 sprintf(msg, "\n%3d %.*g<=X<%.*g (%.*g)\t\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g", i+1, pdigits, mn-a/2+(double)(i)*step, pdigits, mn -a/2 + (double)(i+1)*step, pdigits, mn -a/2 +(double)(i)*step +step/2, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max, pdigits, cl[i].num, pdigits, cl[i].num/(tot));
 plog(msg);
 }
 if(cl[i].num<lminy)
  {
   lminy=cl[i].num;
   miny2=cl[i].num/tot;
  } 
 if(cl[i].num>truemaxy)
  {
   truemaxy=cl[i].num;
   truemaxy2=cl[i].num/tot;
  } 
 
 }

if(stat==1)
 cmd(inter, "wm deiconify .log; raise .log ."); 
if(2==1 && autom==0 && miny<maxy)
 {
  lminy=miny;
  miny2=lminy/tot;
  truemaxy=maxy;
  truemaxy2=maxy/tot;
 }
else
 {
  cmd(inter, ".f.sc.max conf -state normal");
  cmd(inter, ".f.sc.min conf -state normal");  
  maxy=truemaxy;
  miny=lminy;
  cmd(inter, "update");
  cmd(inter, ".f.sc.max conf -state disabled");
  cmd(inter, ".f.sc.min conf -state disabled");  
 }


sprintf(msg, "set p .f.new%d", cur_plot);
cmd(inter, msg);

cmd(inter, "toplevel $p");
cmd(inter, "wm title $p $tit");
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap $p @$RootLsd/$LsdSrc/lsd.xbm} {}");
cmd(inter, "bind $p <Double-Button-1> {raise .; focus -force .b.ts}");
cmd(inter, "frame $p.f -width 640 -height 430");
cmd(inter, "pack $p.f");

//Reset p to point to the canvas (shit...)
cmd(inter, "set p $p.f.plots");

cmd(inter, "canvas $p -width 640 -height 430 -background white -relief flat");
cmd(inter, "pack $p");

cmd(inter, "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p");

if(grid==1)
{
cmd(inter, "$p create line 40 0 40 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 190 0 190 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 340 0 340 310 -fill grey60 -width 1p -tag p");
cmd(inter, "$p create line 490 0 490 310 -fill grey60 -width 1p -tag p");
}
else
{
cmd(inter, "$p create line 640 295 640 305  -width 1 -tag p");
cmd(inter, "$p create line 190 295 190 305 -width 1 -tag p");
cmd(inter, "$p create line 340 295 340 305 -width 1 -tag p");
cmd(inter, "$p create line 490 295 490 305 -width 1 -tag p");
}


sprintf(msg, "set choice [$p create text 40 312 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]",pdigits,cl[0].lowb);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

i=num_bin/4;
sprintf(msg, "set choice [$p create text 190 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]",pdigits,cl[i].center);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


if(num_bin%2==0)
 a=cl[num_bin/2].lowb;
else
 a=cl[(num_bin-1)/2].center; 
i=num_bin/2;
sprintf(msg, "set choice [$p create text 340 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p  text}]",pdigits,a);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

i=(int)((double)(num_bin)*.75);
sprintf(msg, "set choice [$p create text 490 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]",pdigits,cl[i].center);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 635 312 -font {Times 10 normal} -anchor ne -text %.*g -tag {p  text}]",pdigits,cl[num_bin-1].highb);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);



cmd(inter, "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p");

if(grid==1)
{
cmd(inter, "$p create line 38 300 640 300 -fill grey60 -tag p");
cmd(inter, "$p create line 38 225 640 225 -fill grey60 -tag p");
cmd(inter, "$p create line 38 150 640 150 -fill grey60 -tag p");
cmd(inter, "$p create line 38 75 640 75 -fill grey60 -tag p");
cmd(inter, "$p create line 38 0 640 0 -fill grey60 -tag p");
}
else
{
//cmd(inter, "$p create line 35 300 45 300  -tag p");
cmd(inter, "$p create line 35 225 45 225  -tag p");
cmd(inter, "$p create line 35 150 45 150  -tag p");
cmd(inter, "$p create line 35 75 45 75  -tag p");
cmd(inter, "$p create line 35 2 45 2 -tag p");
}

sprintf(msg, "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,lminy);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p  text}]",pdigits,(lminy+(truemaxy-lminy)/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,(lminy+(truemaxy-lminy)/2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]",pdigits,(lminy+(truemaxy-lminy)*3/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]",pdigits,(truemaxy));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 312 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]",pdigits,miny2);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 237 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]",pdigits,(miny2+(truemaxy2-miny2)/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 162 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]",pdigits,(miny2+(truemaxy2-miny2)/2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 87 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]",pdigits,(miny2+(truemaxy2-miny2)*3/4));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);

sprintf(msg, "set choice [$p create text 4 16 -font {Times 10 normal} -anchor nw -text (%.*g) -tag {p text}]",pdigits,(truemaxy2));
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-3> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);
sprintf(msg, ".f.new%d.f.plots bind $choice <Button-2> {.f.new%d.f.plots dtag selected; .f.new%d.f.plots addtag selected withtag %d; set ccanvas .f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}",cur_plot,cur_plot,cur_plot, *choice,cur_plot);
cmd(inter, msg);


*choice=0;
for(i=0; i<num_bin; i++)
 {
  x1=40+(int)(600* (cl[i].lowb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  x2=40+(int)(600* (cl[i].highb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  y1=(int)(300-((cl[i].num-lminy)/(truemaxy-lminy))*300);
  y2=300;

  if(y1<301)
  {//in case of user-definde miny
  sprintf(msg,"$p create rect %d %d %d %d -tag p%d -width 1 -fill $c%d",x1,y1,x2,y2, i, 1);
  cmd(inter, msg);
//sprintf(msg, "$p bind p%d <Enter> {.f.new%d.f.plots delete xpos; .f.new%d.f.plots create text 0 390 -font {Times 10 normal} -text \"Class %d, %g <= X < %g, min=%g center=%g max=%g.\\nContains %g units (%g perc.)\" -anchor w -tag xpos}",i,  cur_plot, cur_plot, i, cl[i].lowb, cl[i].highb, cl[i].min, cl[i].center, cl[i].max, cl[i].num, 100*cl[i].num/tot);
sprintf(msg, "$p bind p%d <Enter> {.f.new%d.f.plots delete xpos; .f.new%d.f.plots create text 0 390 -font {Times 10 normal} -text \"Class %d, %.*g <= X < %.*g, (center=%.*g)\\nContains %.*g units (%.*g perc.). Actual values: min=%.*g, av.=%.*g, max=%.*g  \" -anchor w -tag xpos}",i,  cur_plot, cur_plot, i, pdigits, cl[i].lowb, pdigits, cl[i].highb, pdigits, cl[i].center, pdigits, cl[i].num, pdigits, 100*cl[i].num/tot, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max);


cmd(inter, msg);
sprintf(msg, "$p bind p%d <Leave> {.f.new%d.f.plots delete xpos}",i, cur_plot);
cmd(inter, msg);
  }
  if(*choice==1) //Button STOP pressed
    {
     i=num_bin;
    }
 if(watch==1)
	cmd(inter, "update");
  
 }

/***
just to check that the sum is one
*/
cmd(inter, "set choice $norm");
if(*choice==1)
{
 totnorm=0;
 for(i=0; i<num_bin; i++)
  {
   a=cl[i].lowb;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   s=cl[i].highb;
   ap=exp(-(s-average)*(s-average)/(2*sigma))/(sqrt(2*PI*sigma));
   totnorm+=(b+ap)/2;
 //  sprintf(msg, "\nn=%g, f=%g", (b+ap)/2,cl[i].num/tot);
 //  plog(msg);
  }
  
 /*******/
 for(i=0; i<num_bin-1; i++)
  {
   a=cl[i].center;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   b=b*tot/totnorm;
   b=300-((b-lminy)/(truemaxy-lminy))*300;    
   a=cl[i+1].center;  
   s=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*PI*sigma));
   s=s*tot/totnorm;
   s=300-((s-lminy)/(truemaxy-lminy))*300;  
   
   x1=40+(int)(600* (cl[i].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
   x2=40+(int)(600* (cl[i+1].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
 
   sprintf(msg, "$p create line %d %d %d %d",x1,(int)b, x2, (int)s );
   cmd(inter, msg);
 
  }
} 
    


//cmd(inter, "$p create text 200 420 -font {Times 10 normal} -text \"Class num:\" -anchor w ");
//cmd(inter, "$p create text 380 420 -font {Times 10 normal} -text \"Freq: \" -anchor w ");


sprintf(msg, "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}",cur_plot);
cmd(inter, msg);
*choice=0;

delete[] cl;
delete[] start;
delete[] end;
delete[] data;
for(i=0; i<nv; i++)
 {
 delete[] str[i];
 delete[] tag[i];
 if(logs)
   delete[] logdata[i];
 }
delete[] logdata;
delete[] str;
delete[] tag; 

}

void create_series(int *choice)
{


int i, nv, j, k, *start, *end, idseries, flt;
double nmax, nmin, nmean, nvar, nn, thflt, confi;
double step;
bool first;

thflt=0;
char *lapp, **str, **tag;
store *app;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");
double **data;

if(nv==0)
 return;

if(logs)
  cmd(inter, "tk_messageBox -type ok -icon warning -title \"Warning\" -message \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"");

Tcl_LinkVar(inter, "thflt", (char *) &thflt, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "confi", (char *) &confi, TCL_LINK_DOUBLE);


cmd(inter, "toplevel .s");
cmd(inter, "wm title .s \"Select elaborations\"");
cmd(inter, "wm transient .s .");

cmd(inter, "frame .s.o -relief groove -bd 2");
cmd(inter, "label .s.o.l -text \"Type of scanning\" -fg red");
cmd(inter, "pack .s.o.l");
cmd(inter, "set bido 1");
cmd(inter, "radiobutton .s.o.m -text \"Compute over series (same # of cases)\" -variable bido -value 1");
cmd(inter, "radiobutton .s.o.f -text \"Compute over cases (# cases = # of series)\" -variable bido -value 2");
cmd(inter, "pack .s.o.m .s.o.f -anchor w");

cmd(inter, "pack .s.o");

cmd(inter, "frame .s.f -relief groove -bd 2");
cmd(inter, "label .s.f.l -text \"Filtering\" -fg red");
cmd(inter, "pack .s.f.l");
cmd(inter, "set flt 0");
cmd(inter, "radiobutton .s.f.n -text \"Use all the data\" -variable flt -value 0");
cmd(inter, "radiobutton .s.f.s -text \"Ignore small values\" -variable flt -value 1");
cmd(inter, "radiobutton .s.f.b -text \"Ignore large values\" -variable flt -value 2");
cmd(inter, "entry .s.f.th -width 21 -textvariable thflt");
cmd(inter, "pack .s.f.n .s.f.s .s.f.b .s.f.th  -anchor w");

cmd(inter, "pack .s.f");

cmd(inter, "set bidi 0");
cmd(inter, "frame .s.i -relief groove -bd 2");
/**/
cmd(inter, "label .s.i.l -text \"Type of series to create\" -fg red");

cmd(inter, "radiobutton .s.i.m -text \"Average\" -variable bidi -command {set headname \"Av\"; set vname $headname$basename; .s.nv selection range 0 end} -value 1");
cmd(inter, "radiobutton .s.i.z -text \"Sum\" -variable bidi -command {set headname \"Sum\"; set vname $headname$basename; .s.nv selection range 0 end} -value 5");
cmd(inter, "radiobutton .s.i.f -text \"Maximum\" -variable bidi -command {set headname \"Max\"; set vname $headname$basename; .s.nv selection range 0 end} -value 2");
cmd(inter, "radiobutton .s.i.t -text \"Minimum\" -variable bidi -command {set headname \"Min\"; set vname $headname$basename; .s.nv selection range 0 end} -value 3");
cmd(inter, "radiobutton .s.i.c -text \"Variance\" -variable bidi -command {set headname \"Var\"; set vname $headname$basename; .s.nv selection range 0 end} -value 4");
cmd(inter, "frame .s.i.ci");
cmd(inter, "radiobutton .s.i.ci.c -text \"StdDev\" -variable bidi -command {set headname \"CI\"; set vname $headname$basename; .s.nv selection range 0 end} -value 6");
cmd(inter, "label .s.i.ci.x -text \"x\"");
confi=1.96;
cmd(inter, "entry .s.i.ci.p -width 4 -textvariable confi");
cmd(inter, "pack .s.i.ci.c .s.i.ci.x .s.i.ci.p -side left");
/*
cmd(inter, "frame .s.i.ma");
cmd(inter, "radiobutton .s.i.ma.ma -text \"Mov. Average\" -variable bidi -command {set headname \"MovA\"; set vname $headname$basename; .s.nv selection range 0 end} -value 8");
cmd(inter, "label .s.i.ma.x -text \" range \"");
confi=1.96;
cmd(inter, "entry .s.i.ma.p -width 4 -textvariable confi");
cmd(inter, "pack .s.i.ma.ma .s.i.ma.x .s.i.ma.p -side left");
*/

cmd(inter, "radiobutton .s.i.n -text \"Count\" -variable bidi -command {set headname \"Num\"; set vname $headname$basename; .s.nv selection range 0 end} -value 7");

cmd(inter, "pack .s.i.l .s.i.m .s.i.z .s.i.f .s.i.t .s.i.c .s.i.ci .s.i.n -anchor w");
/**/
cmd(inter, "pack .s.i");


//cmd(inter, "set vname newvar");
cmd(inter, "set a [.f.vars.ch.v get 0]");
cmd(inter, "set basename [lindex [split $a] 0]");
cmd(inter, "set headname \"Av\"");
cmd(inter, "set vname $headname$basename");

cmd(inter, "label .s.lnv -text \"New series label\" -fg red");
cmd(inter, "entry .s.nv -width 40 -textvariable vname");
cmd(inter, "pack .s.lnv .s.nv");

cmd(inter, "set vtag 1");
cmd(inter, "label .s.tnv -text \"New series tag\" -fg red");
cmd(inter, "entry .s.tv -width 20 -textvariable vtag");
cmd(inter, "pack .s.tnv .s.tv");

cmd(inter, "button .s.ok -text Ok -command {set choice 1}");
cmd(inter, "button .s.esc -text Cancel -command {set choice 2}");
cmd(inter, "button .s.help -text \" Help \" -command {LsdHelp mdatares.html#create_series}");
cmd(inter, "pack .s.i .s.ok .s.help .s.esc");
cmd(inter, "bind .s <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
*choice=0;
cmd(inter, "set bidi 1");
cmd(inter, "focus -force .s.nv");
 cmd(inter, ".s.nv selection range 0 end");
  while(*choice==0)
	Tcl_DoOneEvent(0);

Tcl_UnlinkVar(inter,"thflt");
Tcl_UnlinkVar(inter,"confi");
cmd(inter, "destroy .s");
if(*choice==2)
 {*choice=0;
  return;
 }


data=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

 app=new store[1+ num_var];
 for(i=0; i<num_var; i++)
  {app[i]=vs[i];
   strcpy(app[i].label, vs[i].label);
   strcpy(app[i].tag, vs[i].tag);
  } 
 delete[] vs;
 vs=app;

if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  lapp=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,lapp);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {

   data[i]=find_data(idseries);
   }

 }



if(autom_x==1||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

cmd(inter, "set choice $flt");
flt=*choice;



cmd(inter, "set choice $bido");

if(*choice==1)
{//compute over series
vs[num_var].data = new double[max_c+2];
lapp=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(msg,lapp);
strcpy(vs[num_var].label, msg);
vs[num_var].end=max_c;
vs[num_var].start=min_c;
lapp=(char *)Tcl_GetVar(inter, "vtag",0);
strcpy(msg,lapp);
strcpy(vs[num_var].tag, msg);
vs[num_var].rank=num_var;

cmd(inter, "set choice $bidi");

for(i=min_c; i<=max_c; i++)
 {nmean=nn=nvar=0;
  first=true;
  for(j=0; j<nv; j++)
   {
    if(i>=start[j] && i<=end[j] && !isnan(data[j][i]) && (flt==0 || (flt==1 && data[j][i]>thflt) || (flt==2 && data[j][i]<thflt) ))		// ignore NaNs
    {
    nmean+=data[j][i];
    nvar+=data[j][i]*data[j][i];
    nn++;
    if(first)
	{
     nmin=nmax=data[j][i];
	 first=false;
	}
    else
     {
     if(nmin>data[j][i])
      nmin=data[j][i];
     if(nmax<data[j][i])
      nmax=data[j][i];
      
     } 
    }
   }

   if(nn==0)	// not a single valid value?
   {
		nn=nmean=nvar=nmin=nmax=NAN;
   }
   else
   {
		nmean/=nn;
		nvar/=nn;
		nvar-=nmean*nmean;
   }
   
if(*choice==1)
 vs[num_var].data[i]=nmean;
if(*choice==5)
 vs[num_var].data[i]=nmean*nn;
 
if(*choice==2)
 vs[num_var].data[i]=nmax;
if(*choice==3)
 vs[num_var].data[i]=nmin;
if(*choice==4)
 vs[num_var].data[i]=nvar;
if(*choice==7)
 vs[num_var].data[i]=nn;
if(*choice==6)
 vs[num_var].data[i]=sqrt(nvar)*confi;


 }
sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var].label, vs[num_var].tag, min_c, max_c, num_var); 
cmd(inter, msg);

} 
else
{
//compute over cases
vs[num_var].data = new double[nv];
lapp=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(msg,lapp);
strcpy(vs[num_var].label, msg);
vs[num_var].end=nv-1;
vs[num_var].start=0;
lapp=(char *)Tcl_GetVar(inter, "vtag",0);
strcpy(msg,lapp);
strcpy(vs[num_var].tag, msg);
vs[num_var].rank=num_var;

cmd(inter, "set choice $bidi");

for(j=0; j<nv; j++)
 {nmean=nn=nvar=0;
  first=true;
  for(i=min_c; i<=max_c; i++)
   {
    if(i>=start[j] && i<=end[j] && !isnan(data[j][i]) && (flt==0 || (flt==1 && data[j][i]>thflt) || (flt==2 && data[j][i]<thflt) ) )
    {
    nmean+=data[j][i];
    nvar+=data[j][i]*data[j][i];
    nn++;
    if(first)
	{
     nmin=nmax=data[j][i];
	 first=false;
	}
    else
     {
     if(nmin>data[j][i])
      nmin=data[j][i];
     if(nmax<data[j][i])
      nmax=data[j][i];
      
     } 
    }
   }

   if(nn==0)	// not a single valid value?
   {
		nn=nmean=nvar=nmin=nmax=NAN;
   }
   else
   {
		nmean/=nn;
		nvar/=nn;
		nvar-=nmean*nmean;
   }
      
if(*choice==1)
 vs[num_var].data[j]=nmean;
if(*choice==5)
 vs[num_var].data[j]=nmean*nn;
if(*choice==2)
 vs[num_var].data[j]=nmax;
if(*choice==3)
 vs[num_var].data[j]=nmin;
if(*choice==4)
 vs[num_var].data[j]=nvar;
if(*choice==7)
 vs[num_var].data[j]=nn;
if(*choice==6)
 vs[num_var].data[j]=sqrt(nvar)*confi;

 }

sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var].label, vs[num_var].tag, 0, nv-1, num_var); 
cmd(inter, msg);

}
cmd(inter, ".f.vars.lb.v see end");
num_var++; 
sprintf(msg, ".f.com.nvar conf -text \"Series = %d\"",num_var);
cmd(inter,msg);

delete[] start;
delete[] end;
delete[] data;
for(i=0; i<nv; i++)
 {
 delete[] str[i];
 delete[] tag[i];
 }
delete[] str;
delete[] tag; 


}



void create_maverag(int *choice)
{


int i, nv, j, h, *start, *end, flt, idseries;
double xapp;
double step;

char *lapp, **str, **tag;
store *app;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");
double **data;

if(nv==0)
 return;

if(logs)
  cmd(inter, "tk_messageBox -type ok -icon warning -title \"Warning\" -message \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"");

cmd(inter, "toplevel .s");
cmd(inter, "wm title .s \"Mov.Av. Range\"");
cmd(inter, "wm transient .s .");

cmd(inter, "frame .s.o -relief groove -bd 2");
cmd(inter, "label .s.o.l -text \"Set # of (odd) periods\" -fg red");
cmd(inter, "pack .s.o.l");
cmd(inter, "set bido 10");
cmd(inter, "entry .s.o.th -width 6 -textvariable bido");
cmd(inter, "pack .s.o.th");

cmd(inter, "pack .s.o");


cmd(inter, "button .s.ok -text Ok -command {set choice 1}");
cmd(inter, "button .s.esc -text Cancel -command {set choice 2}");
cmd(inter, "button .s.help -text \" Help \" -command {LsdHelp mdatares.html#create_maverag}");
cmd(inter, "pack .s.ok .s.help .s.esc");
cmd(inter, "bind .s <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
*choice=0;
cmd(inter, "focus -force .s.o.th");
cmd(inter, ".s.o.th selection range 0 end");
  while(*choice==0)
	Tcl_DoOneEvent(0);

cmd(inter, "destroy .s");
if(*choice==2)
 {*choice=0;
  return;
 }


data=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

 app=new store[nv + num_var];
 for(i=0; i<num_var; i++)
  {app[i]=vs[i];
   strcpy(app[i].label, vs[i].label);
   strcpy(app[i].tag, vs[i].tag);
  } 
 delete[] vs;
 vs=app;

if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

cmd(inter, "set choice $bido");
flt=*choice;
if((flt%2)==0)
 flt++;

if(flt<2)
  {*choice=0;
  return;
 }


for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  lapp=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,lapp);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  
  sprintf(msg, "MA%d_%s", flt, str[i]);
  strcpy(vs[num_var+i].label, msg);
  strcpy(vs[num_var+i].tag,tag[i]);
  vs[num_var+i].start=start[i];
  vs[num_var+i].end=end[i];
  vs[num_var+i].rank=num_var+i;
  vs[num_var+i].data = new double[max_c+2];
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[i]=find_data(idseries);
   xapp=0;

   for(h=0, j=start[i]; j<start[i]+flt;j++)
//     xapp+=data[i][j]/(double)flt;
	   if(!isnan(data[i][j]))		// not a NaN?
	   {
		   xapp+=data[i][j];
		   h++;
	   }
	if(h==0)	// no observation?
		xapp=NAN;
	else
		xapp/=(double)h;
	
   for(j=start[i]; j<start[i]+(flt-1)/2; j++)
     vs[num_var+i].data[j]=xapp;

   for(   ; j<end[i]-(flt-1)/2; j++)
    {
	 if(!isnan(data[i][j-(flt-1)/2]) && !isnan(data[i][j+(flt-1)/2]))
		xapp=xapp-data[i][j-(flt-1)/2]/(double)flt+data[i][j+(flt-1)/2]/(double)flt;
	 else
		xapp=NAN;
     vs[num_var+i].data[j]=xapp;
    }
   for(   ; j<end[i]; j++)
     vs[num_var+i].data[j]=xapp;     
   }
  sprintf(msg, ".f.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var+i].label, vs[num_var+i].tag,       
  vs[num_var+i].start, vs[num_var+i].end, num_var+i); 
  cmd(inter, msg); 
 }

cmd(inter, ".f.vars.lb.v see end");
num_var+=nv; 
sprintf(msg, ".f.com.nvar conf -text \"Series = %d\"",num_var);
cmd(inter,msg);

delete[] start;
delete[] end;
delete[] data;
for(i=0; i<nv; i++)
 {
 delete[] str[i];
 delete[] tag[i];
 }
delete[] str;
delete[] tag; 

}

/************************
SAVE_DATAzip
************************/
void save_datazip(int *choice)
{

int idseries, dozip=0, memstep;
char *app;
char **str, **tag;
char str1[100], delimiter[10], misval[10];
int i, nv, j, *start, *end, typelab, numcol, del, fr, gp, type_res;
double **data, *dd, *ddstart;

#ifdef LIBZ
 gzFile fsavez;
#else
 FILE *fsavez; 
#endif 
FILE *fsave;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");

*choice=0;

if(nv==0)
 return;

if(logs)
  cmd(inter, "tk_messageBox -type ok -icon warning -title \"Warning\" -message \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"");

data=new double *[nv];
start=new int[nv];
end=new int[nv];


str=new char *[nv];
tag=new char *[nv];

max_c=min_c=0;

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);

  data[i]=find_data(idseries);
  if(max_c<end[i])
   max_c=end[i];
 }


//Variables' Name in first column
fr=1;
strcpy(misval,nonavail);
Tcl_LinkVar(inter, "typelab", (char *) &typelab, TCL_LINK_INT);
typelab=4;
cmd(inter, "toplevel .lab");
cmd(inter, "wm title .lab \"Saving Data\"");
cmd(inter, "wm transient .lab . ");
cmd(inter, "frame .lab.f -relief groove -bd 2");
cmd(inter, "radiobutton .lab.f.lsd -text \"Lsd Result File\" -variable typelab -value 3");
cmd(inter, "radiobutton .lab.f.nolsd -text \"Text File\" -variable typelab -value 4");
cmd(inter, "set dozip 0");
cmd(inter, "checkbutton .lab.dozip -text \"Generate zipped file\" -variable dozip");

cmd(inter ,"button .lab.ok -text Proceed -command {set choice 1}");
cmd(inter, "button .lab.help -text Help -command {LsdHelp mdatares.html#save}");
cmd(inter ,"button .lab.esc -text Cancel -command {set choice 2}");

cmd(inter, "pack .lab.f.lsd .lab.f.nolsd -anchor w");
#ifdef LIBZ
cmd(inter, "pack .lab.f  .lab.dozip .lab.ok .lab.help .lab.esc");
#else
cmd(inter, "pack .lab.f  .lab.ok .lab.help .lab.esc");
#endif
cmd(inter, "bind .lab <Return> {.lab.ok invoke}");
cmd(inter, "bind .lab <Escape> {.lab.esc invoke}");
cmd(inter, "focus .lab");
#ifndef DUAL_MONITOR
cmd(inter, "set w .lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==2)
 goto end;
type_res=typelab;

cmd(inter, "set choice $dozip");
dozip=*choice;

*choice=0;
cmd(inter, "destroy .lab.f .lab.ok .lab.esc .lab.help");

 


if(typelab==4)
{
Tcl_LinkVar(inter, "deli", (char *) &del, TCL_LINK_INT);
Tcl_LinkVar(inter, "numcol", (char *) &numcol, TCL_LINK_INT);
Tcl_LinkVar(inter, "fr", (char *) &fr, TCL_LINK_INT);

typelab=1;
cmd(inter, "frame .lab.f -relief groove -bd 2");
cmd(inter, "label .lab.f.tit -text \"Labels to use\" -foreground red");

cmd(inter, "radiobutton .lab.f.orig -text Original -variable typelab -value 1");
cmd(inter, "radiobutton .lab.f.new -text \"New Names\" -variable typelab -value 2");
cmd(inter, "set newlab \"\"");
cmd(inter, "entry .lab.f.en -textvariable newlab");
cmd(inter, "set gp 0");
cmd(inter, "checkbutton .lab.f.gp -text \"Add #\" -variable gp");
cmd(inter, "bind .lab.f.en <FocusIn> {.lab.f.new invoke}");
cmd(inter, "pack .lab.f.tit .lab.f.orig .lab.f.new .lab.f.en .lab.f.gp -anchor w");
cmd(inter, "frame .lab.d -relief groove -bd 2");
cmd(inter, "label .lab.d.tit -text \"Columns delimiter\" -foreground red");

cmd(inter, "frame .lab.d.r");
del=1;
cmd(inter, "radiobutton .lab.d.r.tab -text \"Tab Delimited\" -variable deli -value 1");
cmd(inter, "radiobutton .lab.d.r.oth -text \"Other Delimiter\" -variable deli -value 2");
cmd(inter, "set delimiter \"\"");
cmd(inter, "entry .lab.d.r.del -textvariable delimiter");
cmd(inter, "bind .lab.d.r.del <FocusIn> {.lab.d.r.oth invoke}");

cmd(inter, "radiobutton .lab.d.r.col -text \"Fixed Length Columns\" -variable deli -value 3");
numcol=16;
cmd(inter, "entry .lab.d.r.ecol -textvariable numcol");
cmd(inter, "bind .lab.d.r.ecol <FocusIn> {.lab.d.r.col invoke}");


cmd(inter, "pack .lab.d.r.tab .lab.d.r.oth .lab.d.r.del .lab.d.r.col .lab.d.r.ecol -anchor w");
cmd(inter, "pack .lab.d.tit .lab.d.r -anchor w");

cmd(inter, "frame .lab.gen -relief groove -bd 2");
cmd(inter, "label .lab.gen.tit -text \"General Options\" -foreground red");
cmd(inter, "checkbutton .lab.gen.fr -text \"Variables in First Row\" -variable fr");
cmd(inter, "label .lab.gen.miss -text \"Missing Values\"");
cmd(inter, "set misval \"n/a\"");
cmd(inter, "entry .lab.gen.mis_val -textvariable misval");
cmd(inter, "pack .lab.gen.tit .lab.gen.fr .lab.gen.miss .lab.gen.mis_val -anchor w");
cmd(inter, "button .lab.help -text Help -command {LsdHelp mdatares.html#save}");
cmd(inter ,"button .lab.esc -text Cancel -command {set choice 2}");

cmd(inter ,"button .lab.ok -text Proceed -command {set choice 1}");
cmd(inter, "pack .lab.f .lab.d .lab.gen .lab.ok .lab.esc .lab.help -fill x");
*choice=0;
cmd(inter, "focus .lab");
cmd(inter, "bind .lab <KeyPress-Return> {.lab.ok invoke}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .lab; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
//cmd(inter, "raise .lab");
while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==2)
 goto end;

cmd(inter, "set choice $gp");
gp=*choice;

*choice=0;

app=(char *)Tcl_GetVar(inter, "misval", 0);
strcpy(misval,app);

}

if(type_res==4)
  sprintf(msg, "set bah [tk_getSaveFile -title \"Data File Name\" -initialdir [pwd] -defaultextension \"txt\" -filetypes {{{Text Files} {.txt}} {{Lsd Result Files} {.res}} {{All Files} {*}} }]");
else
  sprintf(msg, "set bah [tk_getSaveFile -title \"Data File Name\"  -initialdir [pwd] -defaultextension \"res\" -filetypes {{{Lsd Result Files} {.res}} {{Text Files} {.txt}} {{All Files} {*}} }]");
cmd(inter, msg);
app=(char *)Tcl_GetVar(inter, "bah",0);
strcpy(msg, app);

if(strlen(msg)==0)
 goto end;
if(dozip==1) 
 {
 strcat(msg, ".gz");
 #ifdef LIBZ
  fsavez=gzopen(msg, "wb");
 #endif 

   
 } 
else
 fsave=fopen(msg, "wt");  // use text mode for Windows better compatibility


if(del!=3) //Delimited files
{if(del==2)
 {app=(char *)Tcl_GetVar(inter, "delimiter",0);
  if(strlen(app)==0)
    strcpy(delimiter,"\t");
  else
    strcpy(delimiter,app);
 }
 else
  strcpy(delimiter, "\t");
}
if(fr==1)
{
if(del!=3)
{
switch(typelab)
{
case 1:   //Original labels
if(gp==1)
 {if(dozip==1) 
    {
     #ifdef LIBZ
     gzprintf(fsavez, "#");
     #endif
    } 
  else
    fprintf(fsave, "#");
 }   
for(i=0; i<nv; i++)
  {
   if(dozip==1) 
    {
    #ifdef LIBZ 
     gzprintf(fsavez, "%s_%s%s", str[i], tag[i], delimiter);
    #endif 
    } 
   else
    fprintf(fsave, "%s_%s%s", str[i], tag[i], delimiter); 
  }  
break;

case 2: //New names for labels
  app=(char *)Tcl_GetVar(inter, "newlab",0);
  if(strlen(app)==0)
    strcpy(msg,"Var");
  else
    strcpy(msg,app);
if(gp==1)
 {if(dozip==1) 
   {
    #ifdef LIBZ
    gzprintf(fsavez, "#");
    #endif
   } 
  else
   fprintf(fsave, "#");
 }  
 for(i=0; i<nv; i++)
   {if(dozip==1) 
      {
       #ifdef LIBZ
       gzprintf(fsavez, "%s%d%s", msg, i, delimiter);
       #endif
      } 
    else
      fprintf(fsave, "%s%d%s", msg, i, delimiter);  
   }   
 break;

case 3:
for(i=0; i<nv; i++) //Lsd result files
  {if(dozip==1) 
     {
     #ifdef LIBZ 
      gzprintf(fsavez, "%s %s (%d %d)\t", str[i], tag[i], start[i], end[i]);
     #endif 
     } 
   else
     fprintf(fsave, "%s %s (%d %d)\t", str[i], tag[i], start[i], end[i]);  
  }   
break;
 } //end switch delimiter
} //and of delimiter header writing
else //header for Column text files
  {
if(gp==1)
 {
  if(dozip==1) 
    {
    #ifdef LIBZ 
     gzprintf(fsavez, "#");
    #endif 
    } 
  else
    fprintf(fsave, "#");  
 }    

  strcpy(str1, "                                                                                ");
   for(i=0; i<nv; i++)
    {sprintf(msg, "%s_%s", str[i], tag[i]);
     if(strlen(msg)<=numcol)
      strcat(msg, str1);
     msg[numcol]=(char)NULL;
    if(dozip==1) 
     {
      #ifdef LIBZ
      gzprintf(fsavez,"%s", msg);
      #endif
     } 
    else
     fprintf(fsave,"%s", msg); 
    }
  }

if(dozip==1) 
  {
  #ifdef LIBZ 
   gzprintf(fsavez,"\n");
  #endif 
  } 
else
  fprintf(fsave,"\n");  
}//end of header writing

if(del!=3) //data delimited writing
{

if(dozip==1)
{
/**********/
//WORKS

#ifdef LIBZ
for(j=min_c; j<=max_c; j++)
 {
  for(i=0; i<nv; i++)
   {if(j>=start[i] && j<=end[i] && !isnan(data[i][j]))		// write NaN as n/a
      gzprintf(fsavez, "%g%s", data[i][j], delimiter);
    else
      gzprintf(fsavez, "%s%s", misval, delimiter);
      
   }
  gzprintf(fsavez,"\n");
 }
#endif 
/**********************

if(nv>1)
 memstep=&data[1][0]-&data[0][0];
else
 memstep=0; 
ddstart=&data[0][min_c]; 
for(j=min_c; j<=max_c; j++)
 {
 dd=ddstart;
  for(i=0; i<nv; i++)
   {if(j>=start[i] && j<=end[i])
      gzprintf(fsavez, "%g%s", *dd, delimiter);
    else
      gzprintf(fsavez, "%s%s", misval, delimiter);
      
    dd+=memstep;  
   }
  ddstart+=1; 
  gzprintf(fsavez,"\n");
 }
/**********************/
}
else
{
for(j=min_c; j<=max_c; j++)
 {for(i=0; i<nv; i++)
   {if(j>=start[i] && j<=end[i] && !isnan(data[i][j]))		// write NaN as n/a
      fprintf(fsave, "%g%s", data[i][j], delimiter);  
    else
      fprintf(fsave, "%s%s", misval, delimiter);  
   }
   fprintf(fsave,"\n");  
 }
}
}
else //column data writing
{ strcpy(str1, "00000000000000000000000000000000000000000000000000000000000000000000000000000000");
for(j=min_c; j<=max_c; j++)
 {for(i=0; i<nv; i++)
   {
   if(j>=start[i] && j<=end[i] && !isnan(data[i][j]))		// write NaN as n/a
   {
      sprintf(msg, "%.10lf", data[i][j]);
	  strcat(msg, str1);
	  msg[numcol]=(char)NULL;
   }
   else
      sprintf(msg, "%s", misval);
   if(dozip==1) 
     {
     #ifdef LIBZ
      gzprintf(fsavez, "%s", msg);
     #endif 
     }
   else
     fprintf(fsave, "%s", msg);  
   }
  if(dozip==1) 
    {
     #ifdef LIBZ
     gzprintf(fsavez,"\n");
     #endif
    } 
   else
    fprintf(fsave,"\n");  
 }

}
if(dozip==1) 
  {
  #ifdef LIBZ
  gzclose(fsavez);
  #endif
  }
else
  fclose(fsave);  

end:
cmd(inter, "destroy .lab");
Tcl_UnlinkVar(inter, "typelab");
Tcl_UnlinkVar(inter, "numcol");
Tcl_UnlinkVar(inter, "deli");
Tcl_UnlinkVar(inter, "fr");
Tcl_UnlinkVar(inter, "misval");


for(i=0; i<nv; i++)
 {delete[] str[i];
  delete[] tag[i];
 }
delete[] str;
delete[] tag;
delete[] data;
delete[] start;
delete[] end;
*choice=0;

} //end of save_datazip

void plog_series(int *choice)
{


int i, nv, j, k, *start, *end, idseries, flt;
double nmax, nmin, nmean, nvar, nn, thflt;
double step;

char *lapp, **str, **tag;
store *app;

Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);
cmd(inter, "set nv [.f.vars.ch.v size]");
Tcl_UnlinkVar(inter, "nv");
double **data;

if(nv==0)
 return;

if(logs)
  cmd(inter, "tk_messageBox -type ok -icon warning -title \"Warning\" -message \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"");

data=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

 app=new store[1+ num_var];
 for(i=0; i<num_var; i++)
  {app[i]=vs[i];
   strcpy(app[i].label, vs[i].label);
   strcpy(app[i].tag, vs[i].tag);
  } 
 delete[] vs;
 vs=app;

if(autom_x==1)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[50];
  tag[i]=new char[50];

  sprintf(msg, "set res [.f.vars.ch.v get %d]",i);
  cmd(inter, msg);
  lapp=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,lapp);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x==1 ||(start[i]<=max_c && end[i]>=min_c))
   {
   
   data[i]=find_data(idseries);
   }

 }



if(autom_x==1||min_c>=max_c)
{

for(i=0; i<nv; i++)
 {if(i==0)
   min_c=max_c=start[i];
  if(start[i]<min_c)
   min_c=start[i];
  if(end[i]>max_c)
   max_c=end[i]>num_c?num_c:end[i];
 }
}

sprintf(msg, "\n\n%s_%s", str[0], tag[0]);
plog(msg); 

for(i=1; i<nv; i++)
 {
  sprintf(msg, "\t%s_%s", str[i], tag[i]);
  plog(msg); 
 }
plog("\n");


for(i=min_c; i<=max_c; i++)
 {
 if(!isnan(data[0][i]))		// write NaN as n/a
	sprintf(msg, "%lf", data[0][i]);
 else
	sprintf(msg, "%s", nonavail);
 plog(msg);
 for(j=1; j<nv; j++)
   {
   if(!isnan(data[j][i]))		// write NaN as n/a
	sprintf(msg, "\t%lf", data[j][i]);
   else
	sprintf(msg, "%s", nonavail);
   plog(msg);
   
   }
 plog("\n");
 }

delete[] start;
delete[] end;
delete[] data;
for(i=0; i<nv; i++)
 {
 delete[] str[i];
 delete[] tag[i];
 }
delete[] str;
delete[] tag; 

}    
