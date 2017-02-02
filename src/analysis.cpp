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

/* 
Reached case 40
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

1) Plot the plot as indicated
2) Exit from data analysis
3) Brings in foreground the plot on which title it was double clicked
4) Load a new data file
5) Sort the labels of the variables
6) Copy the selection from the list of variables to the list of the variables
to plot
7) delete the variables selected in the list of the variables to plot
9) plot the cross-section plot

- void plot_tseries(int *choice);
Plot the plot with the variable indicated in the list of the variables
to plot. If the option says to use the automatic y scaling, it makes first
a scan of the file to spot the maximum and minimum y, then it plots. Otherwise,
it uses directly the values set by the users.

- void plot_cross(void);
Plot a plot of cross section data. The variables chosen are plotted along the times
steps chosen and, on request, they are sorted (descending or ascending) one one of
time steps. To choose which time to sort on, double-click on the time step.

- void set_cs_data(int *choice);
Interface used to determine the time steps to plot in the cross-section plots
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


- void cmd(char *cc);
Contained in UTIL.CPP. Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.


- void plog(char *m);
print  message string m in the Log screen. It is in LSDMAIN.CPP

- void myexit(int v);
Exit function, which is customized on the operative system.

****************************************************/

#include "decl.h"

// plot types
#define TSERIES	0
#define CRSSECT	1
#define GNUPLOT	2
#define LATTICE	3
#define HISTOGR	4

char filename[ MAX_PATH_LENGTH ];
double maxy, maxy2;
double miny, miny2;
double point_size;
int allblack;
int autom = true;
int autom_x = true;
int *cdata;
int cur_plot;
int dir;
int file_counter = 0;
int grid;
int gnu;
int line_point = 1;
int logs = false;		// log scale flag for the y-axis
int max_c;
int min_c;
int num_c;
int num_var;
int num_y2;
int nv;
int pdigits = 4;   		// precision parameter for labels in y scale
int plot_l[ MAX_PLOTS ];
int plot_nl[ MAX_PLOTS ];
int plot_w[ MAX_PLOTS ];
int res;
int time_cross = false;
int type_plot[ MAX_PLOTS ];
int watch = true;
int xy = false;
store *vs;


/***************************************************
ANALYSIS
****************************************************/
void analysis(int *choice)
{
*choice=0;
while( *choice == 0 )
	read_data( choice ); 	//Cases and Variables

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
char *app, *app1, *app2, dirname[MAX_PATH_LENGTH], str1[MAX_ELEM_LENGTH], str2[MAX_ELEM_LENGTH], str3[MAX_ELEM_LENGTH];
double *datum, compvalue=0;

cur_plot=0;
file_counter=0;
*choice=0;

cmd( "set daCwidth 36" );				     	// lists width (even number)

cmd( "if { ! [ info exists gpterm ] } { set gpooptions \"set ticslevel 0.0\"; set gpdgrid3d \"60,60,3\"; if { [ string equal $tcl_platform(platform) windows ] } { set gpterm windows } { set gpterm x11 } }" );

Tcl_LinkVar(inter, "cur_plot", (char *) &cur_plot, TCL_LINK_INT);
Tcl_LinkVar(inter, "nv", (char *) &nv, TCL_LINK_INT);

cover_browser( "Analysis of Results...", "Analysis of Results window is open", "Please exit Analysis of Results\nbefore using the Lsd Browser." );

cmd( "newtop .da \"%s%s - Lsd Analysis of Results\" { set choice 2 } \"\"", unsaved_change() ? "*" : " ", simul_name );

cmd( "menu .da.m -tearoff 0 -relief groove -bd 2" );

cmd( "set w .da.m.exit" );
cmd( ".da.m add cascade -label Exit -menu $w -underline 0" );
cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
cmd( "$w add command -label \"Quit and return to Browser\" -command {set choice 2} -underline 0 -accelerator Esc" );

cmd( "set w .da.m.gp" );
cmd( ".da.m add cascade -label Gnuplot -menu $w -underline 0" );
cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
cmd( "$w add command -label \"Gnuplot...\" -command {set choice 4} -underline 0 -accelerator Ctrl+G" );
cmd( "$w add command -label \"Gnuplot Options...\" -command {set choice 37} -underline 8" );

cmd( "set w .da.m.color" );
cmd( ".da.m add cascade -label Color -menu $w -underline 0" );
cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
cmd( "$w add command -label \"Set Colors...\" -command {set choice 21} -underline 0" );
cmd( "$w add command -label \"Set Default Colors\" -command {set choice 22} -underline 0" );

cmd( "set w .da.m.help" );
cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
cmd( ".da.m add cascade -label Help -menu $w -underline 0" );
cmd( "$w add command -label \"Help on Analysis of Result\" -command {set choice 41} -underline 0" );
cmd( "$w add command -label \"Model Report\" -command {set choice 44} -underline 0" );
cmd( "$w add separator" );
cmd( "$w add command -label \"About Lsd...\" -command { tk_messageBox -parent .da -type ok -icon info -title \"About Lsd\" -message \"Version %s (%s)\" -detail \"Platform: [ string totitle $tcl_platform(platform) ] ($tcl_platform(machine))\nOS: $tcl_platform(os) ($tcl_platform(osVersion))\nTcl/Tk: [ info patch ]\" } -underline 0", _LSD_VERSION_, _LSD_DATE_  ); 

cmd( ".da configure -menu .da.m" );

cmd( "set f .da.head" );
cmd( "frame $f" );
cmd( "label $f.lb -width [ expr $daCwidth - 3 ] -text \"Series Available\"" );
cmd( "label $f.pad -width 6" );
cmd( "label $f.ch -width [ expr $daCwidth - 3 ] -text \"Series Selected\"" );
cmd( "label $f.pl -width [ expr $daCwidth - 3 ] -text \"Plots\"" );
cmd( "pack $f.lb $f.pad $f.ch $f.pl -side left" );
cmd( "pack $f" );

cmd( "frame .da.vars" );

cmd( "set f .da.vars.lb" );
cmd( "frame $f" );
cmd( "scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "listbox $f.v -selectmode extended -width $daCwidth -yscroll \"$f.v_scroll set\"" );
cmd( "pack $f.v $f.v_scroll -side left -fill y" );

cmd( "mouse_wheel $f.v" );
cmd( "bind $f.v <Return> {.da.vars.b.in invoke}" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.lb.v <Return> }" );
cmd( "bind $f.v <Button-2> {.da.vars.lb.v selection clear 0 end;.da.vars.lb.v selection set @%%x,%%y; set res [selection get]; set choice 30}" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.lb.v <Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <KeyPress-space> {set res [.da.vars.lb.v get active]; set choice 30}; bind $f.v <KeyPress-space> {set res [.da.vars.lb.v get active]; set choice 30}" );

cmd( "bind $f.v <Shift-Button-2> {.da.vars.lb.v selection clear 0 end;.da.vars.lb.v selection set @%%x,%%y; set res [selection get]; set choice 16}" );
cmd( "bind $f.v <Shift-Button-3> { event generate .da.vars.lb.v <Shift-Button-2> -x %%x -y %%y }" );

cmd( "set f .da.vars.ch" );
cmd( "frame $f" );
cmd( "scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "listbox $f.v -selectmode extended -width $daCwidth -yscroll \"$f.v_scroll set\"" );
cmd( "pack $f.v $f.v_scroll -side left -fill y" );

cmd( "mouse_wheel $f.v" );
cmd( "bind $f.v <KeyPress-o> {.da.vars.b.out invoke}; bind $f.v <KeyPress-O> {.da.vars.b.out invoke}" );
cmd( "bind $f.v <Return> {.da.b.ts invoke}" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.ch.v <Return> }" );
cmd( "bind $f.v <Button-2> {.da.vars.ch.v selection clear 0 end;.da.vars.ch.v selection set @%%x,%%y; set res [selection get]; set choice 33}" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.ch.v <Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <KeyPress-space> {set res [.da.vars.ch.v get active]; set choice 33}; bind $f.v <KeyPress-space> {set res [.da.vars.ch.v get active]; set choice 33}" );

cmd( "set f .da.vars.pl" );
cmd( "frame $f" );
cmd( "scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "listbox $f.v -width $daCwidth -yscroll \"$f.v_scroll set\" -selectmode single" );
cmd( "pack $f.v $f.v_scroll -side left -fill y" );

cmd( "mouse_wheel $f.v" );
cmd( "bind $f.v <Return> {set it [selection get];set choice 3}" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.pl.v <Return> }" );
cmd( "bind $f.v <Button-2> {.da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set @%%x,%%y; set it [selection get]; set n_it [.da.vars.pl.v curselection]; set choice 20}" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.pl.v <Button-2> -x %%x -y %%y }" );
cmd( "bind .da <KeyPress-Delete> {set n_it [.da.vars.pl.v curselection]; if {$n_it != \"\" } {set it [selection get]; set choice 20} {}}" );

cmd( "frame .da.vars.b" );
cmd( "set f .da.vars.b" );
cmd( "button $f.in -width -6 -relief flat -overrelief groove -text \u25b6 -command {set choice 6}" );
cmd( "button $f.out -width -6 -relief flat -overrelief groove -state disabled -text \u25c0 -command {set choice 7}" );
cmd( "button $f.sort -width -6 -relief flat -overrelief groove -text \"Sort \u25b2\" -command {set choice 5} -underline 0" );
cmd( "button $f.sortdesc -width -6 -relief flat -overrelief groove -text \"Sort \u25bc\" -command {set choice 38} -underline 1" );
cmd( "button $f.sortend -width -6 -relief flat -overrelief groove -text \"Sort+\" -command {set choice 15} -underline 2" );
cmd( "button $f.unsort -width -6 -relief flat -overrelief groove -text \"Unsort\" -command {set choice 14} -underline 0" );
cmd( "button $f.search -width -6 -relief flat -overrelief groove -text Find... -command { set choice 39 } -underline 0" );
cmd( "button $f.add -width -6 -relief flat -overrelief groove -text \"Add...\" -command {set choice 24} -underline 0" );
cmd( "button $f.empty -width -6 -relief flat -overrelief groove -text Clear -command {set choice 8} -underline 0" );
cmd( "pack $f.in $f.out $f.sort $f.sortdesc $f.sortend $f.unsort $f.search $f.add $f.empty -padx 2 -pady 1 -fill y" );

cmd( "pack .da.vars.lb .da.vars.b .da.vars.ch .da.vars.pl -side left  -expand true -fill y" );
cmd( "pack .da.vars -expand true -fill y" );

cmd( "frame .da.com" );
cmd( "label .da.com.nvar -text \"Series = %d\" -width [ expr [ expr $daCwidth - 4 ] / 2 ]", num_var  );
cmd( "label .da.com.ncas -text \"Cases = %d\" -width [ expr [ expr $daCwidth - 4 ] / 2 ]", num_c  );
cmd( "label .da.com.pad -width 6" );
cmd( "label .da.com.selec -text \"Series = [ .da.vars.ch.v size ]\" -width [ expr $daCwidth - 3 ]" );
cmd( "label .da.com.plot -text \"Plots = [ .da.vars.pl.v size ]\" -width [ expr $daCwidth - 3 ]" );
cmd( "pack .da.com.nvar .da.com.ncas .da.com.pad .da.com.selec .da.com.plot -side left" );
cmd( "pack .da.com" );

num_c=1;
num_var=0;
if(actual_steps>0)
  insert_data_mem(root, &num_var, &num_c);

Tcl_LinkVar(inter, "auto", (char *) &autom, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "auto_x", (char *) &autom_x, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "minc", (char *) &min_c, TCL_LINK_INT);
Tcl_LinkVar(inter, "maxc", (char *) &max_c, TCL_LINK_INT);
Tcl_LinkVar(inter, "miny", (char *) &miny, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "maxy", (char *) &maxy, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "logs", (char *) &logs, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "allblack", (char *) &allblack, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "grid", (char *) &grid, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "point_size", (char *) &point_size, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "tc", (char *) &time_cross, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "line_point", (char *) &line_point, TCL_LINK_INT);
Tcl_LinkVar(inter, "xy", (char *) &xy, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "pdigits", (char *) &pdigits, TCL_LINK_INT);
Tcl_LinkVar(inter, "watch", (char *) &watch, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "gnu", (char *) &gnu, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "num_y2", (char *) &num_y2, TCL_LINK_INT);

min_c=1;
max_c=num_c;
miny=maxy=0;
gnu = false;
cmd( "set y2 0" );
cmd( "set allblack $grayscaleP" );
cmd( "set grid $gridP" );
cmd( "set point_size $pointsizeP" );
cmd( "set line_point $linemodeP" );
cmd( "set pdigits $pdigitsP" );

cmd( "frame .da.f" );
cmd( "frame .da.f.h" );
cmd( "frame .da.f.h.v" );

cmd( "frame .da.f.h.v.ft" );

cmd( "checkbutton .da.f.h.v.ft.auto -text \"Use all cases \" -variable auto_x -command {if {$auto_x==1} {.da.f.h.v.ft.to.mxc conf -state disabled; .da.f.h.v.ft.from.mnc conf -state disabled} {.da.f.h.v.ft.to.mxc conf -state normal; .da.f.h.v.ft.from.mnc conf -state normal}}" );

cmd( "frame .da.f.h.v.ft.from" );
cmd( "label .da.f.h.v.ft.from.minc -text \"From case\"" );
cmd( "entry .da.f.h.v.ft.from.mnc -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set minc %%P; return 1 } { %%W delete 0 end; %%W insert 0 $minc; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.ft.from.minc .da.f.h.v.ft.from.mnc -side left" );

cmd( "frame .da.f.h.v.ft.to" );
cmd( "label .da.f.h.v.ft.to.maxc -text \"to case\"" );
cmd( "entry .da.f.h.v.ft.to.mxc -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set maxc %%P; return 1 } { %%W delete 0 end; %%W insert 0 $maxc; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack  .da.f.h.v.ft.to.maxc .da.f.h.v.ft.to.mxc -side left" );

cmd( "pack .da.f.h.v.ft.auto .da.f.h.v.ft.from .da.f.h.v.ft.to -side left -ipadx 15" );

cmd( "frame .da.f.h.v.sc" );

cmd( "checkbutton .da.f.h.v.sc.auto -text \"Y self-scaling\" -variable auto -command {if {$auto==1} {.da.f.h.v.sc.max.max conf -state disabled; .da.f.h.v.sc.min.min conf -state disabled} {.da.f.h.v.sc.max.max conf -state normal; .da.f.h.v.sc.min.min conf -state normal}}" );

cmd( "frame .da.f.h.v.sc.min" );
cmd( "label .da.f.h.v.sc.min.lmin -text \"Min. Y\"" );
cmd( "entry .da.f.h.v.sc.min.min -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set miny %%P; return 1 } { %%W delete 0 end; %%W insert 0 $miny; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.sc.min.lmin .da.f.h.v.sc.min.min -side left" );

cmd( "frame .da.f.h.v.sc.max" );
cmd( "label .da.f.h.v.sc.max.lmax -text \"Max. Y\"" );
cmd( "entry .da.f.h.v.sc.max.max -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set maxy %%P; return 1 } { %%W delete 0 end; %%W insert 0 $maxy; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.sc.max.lmax .da.f.h.v.sc.max.max -side left" );

cmd( "pack .da.f.h.v.sc.auto .da.f.h.v.sc.min .da.f.h.v.sc.max -side left -ipadx 5" );

cmd( "frame .da.f.h.v.y2" );
cmd( "checkbutton .da.f.h.v.y2.logs -text \"Series in logs\" -variable logs" );
cmd( "checkbutton .da.f.h.v.y2.y2 -text \"Y2 axis\" -variable y2 -command {if {$y2==0} {.da.f.h.v.y2.f.e conf -state disabled} {.da.f.h.v.y2.f.e conf -state normal}}" );

cmd( "frame .da.f.h.v.y2.f" );
cmd( "label .da.f.h.v.y2.f.l -text \"First series in Y2 axis\"" );
cmd( "entry .da.f.h.v.y2.f.e -width 4 -validate focusout -vcmd { if [ string is integer %%P ] { set num_y2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num_y2; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.y2.f.l .da.f.h.v.y2.f.e -side left" );

cmd( "pack .da.f.h.v.y2.logs .da.f.h.v.y2.y2 .da.f.h.v.y2.f -side left -ipadx 7" );

cmd( "pack .da.f.h.v.ft .da.f.h.v.sc .da.f.h.v.y2" );

cmd( "frame .da.f.h.tc -relief groove -bd 2" );
cmd( "radiobutton .da.f.h.tc.time -text \"Time series\" -variable tc -value 0 -command { if { $xy == 0 } { .da.f.h.v.y2.y2 conf -state normal }; if { $xy == 1 } { .da.f.tit.lp.line config -state normal; set line_point $linemodeP } }" );
cmd( "radiobutton .da.f.h.tc.cross -text \"Cross-section\" -variable tc -value 1 -command { .da.f.h.v.y2.y2 deselect; .da.f.h.v.y2.y2 conf -state disabled; .da.f.h.v.y2.f.e conf -state disabled; if { $xy == 1 } { set line_point 2;  .da.f.tit.lp.line config -state disabled } }" );
cmd( "pack .da.f.h.tc.time .da.f.h.tc.cross -anchor w" );

cmd( "frame .da.f.h.xy -relief groove -bd 2" );
cmd( "radiobutton .da.f.h.xy.seq -text \"Sequence\" -variable xy -value 0 -command { if { $tc == 0 } { .da.f.h.v.y2.y2 conf -state normal } { .da.f.h.v.y2.y2 deselect; .da.f.h.v.y2.y2 conf -state disabled; .da.f.h.v.y2.f.e conf -state disabled }; set gnu 0; .da.f.tit.run.gnu conf -state disabled; .da.f.tit.run.watch conf -state normal; if { $tc == 1 } { .da.f.tit.lp.line config -state normal; set line_point $linemodeP } }" );
cmd( "radiobutton .da.f.h.xy.xy -text \"XY plot\" -variable xy -value 1 -command { .da.f.h.v.y2.y2 deselect; .da.f.h.v.y2.y2 conf -state disabled; .da.f.h.v.y2.f.e conf -state disabled; .da.f.tit.run.gnu conf -state normal; set gnu 1; .da.f.tit.run.watch conf -state disabled; if { $tc == 1 } { set line_point 2;  .da.f.tit.lp.line config -state disabled } }" );
cmd( "pack .da.f.h.xy.seq .da.f.h.xy.xy -anchor w" );

cmd( "pack .da.f.h.v .da.f.h.tc .da.f.h.xy -side left -padx 11" );

cmd( "frame .da.f.tit -relief groove -bd 2" );
cmd( "label .da.f.tit.l -text Title" );
cmd( "entry .da.f.tit.e -textvariable tit -width 36" );

cmd( "frame .da.f.tit.chk" );

cmd( "checkbutton .da.f.tit.chk.allblack -text \"No colors\" -variable allblack" );
cmd( "checkbutton .da.f.tit.chk.grid -text \"Grids\" -variable grid" );
cmd( "pack .da.f.tit.chk.allblack .da.f.tit.chk.grid -anchor w" );

cmd( "frame .da.f.tit.lp" );
cmd( "radiobutton .da.f.tit.lp.line -text \"Lines\" -variable line_point -value 1" );
cmd( "radiobutton .da.f.tit.lp.point -text \"Points\" -variable line_point -value 2" );
cmd( "pack .da.f.tit.lp.line .da.f.tit.lp.point -anchor w" );

cmd( "frame .da.f.tit.ps" );
cmd( "label .da.f.tit.ps.l -text \"Point size\"" );
cmd( "entry .da.f.tit.ps.e -width 4 -validate focusout -vcmd { if [ string is double %%P ] { set point_size %%P; return 1 } { %%W delete 0 end; %%W insert 0 $point_size; return 0 } } -invcmd { bell } -justify center" );
cmd( "pack .da.f.tit.ps.l .da.f.tit.ps.e" );

cmd( "frame .da.f.tit.run" );			// field for adjusting 
cmd( "checkbutton .da.f.tit.run.watch -text Watch -variable watch" );
cmd( "checkbutton .da.f.tit.run.gnu -text Gnuplot -variable gnu -state disabled" );
cmd( "pack .da.f.tit.run.watch .da.f.tit.run.gnu -anchor w" );

cmd( "frame .da.f.tit.pr" );			// field for adjusting y-axis precision
cmd( "label .da.f.tit.pr.l -text \"Precision\"" );
cmd( "entry .da.f.tit.pr.e -width 2 -validate focusout -vcmd { if [ string is integer %%P ] { set pdigits %%P; return 1 } { %%W delete 0 end; %%W insert 0 $pdigits; return 0 } } -invcmd { bell } -justify center" );
cmd( "pack .da.f.tit.pr.l .da.f.tit.pr.e" );

cmd( "pack .da.f.tit.l .da.f.tit.e .da.f.tit.chk .da.f.tit.run .da.f.tit.pr .da.f.tit.ps .da.f.tit.lp -side left -padx 5" );

cmd( "pack .da.f.h .da.f.tit" );
cmd( "pack .da.f" );

cmd( "frame .da.b" );
cmd( "button .da.b.ts -width -9 -text Plot -command {set choice 1} -underline 0" );
cmd( "button .da.b.dump -width -9 -text \"Save Plot\" -command {set fromPlot false; set choice 11} -underline 2" );
cmd( "button .da.b.sv -width -9 -text \"Save Data\" -command {set choice 10} -underline 3" );
cmd( "button .da.b.sp -width -9 -text \"Show Data\" -command {set choice 36} -underline 5" );
cmd( "button .da.b.st -width -9 -text Statistics -command {set choice 12} -underline 1" );
cmd( "button .da.b.fr -width -9 -text Histogram -command {set choice 32} -underline 0" );
cmd( "button .da.b.lat -width -9 -text Lattice -command {set choice 23} -underline 0" );

cmd( "pack .da.b.ts .da.b.dump .da.b.sv .da.b.sp .da.b.st .da.b.fr .da.b.lat -padx 10 -pady 10 -side left -expand no -fill none" );
cmd( "pack .da.b -side right -expand no -fill none" );

// top window shortcuts binding
cmd( "bind .da <KeyPress-Escape> {set choice 2}" );	// quit
cmd( "bind .da <Control-l> {set choice 23}; bind .da <Control-L> {set choice 23}" );	// plot lattice
cmd( "bind .da <Control-h> {set choice 32}; bind .da <Control-H> {set choice 32}" );	// plot histograms
cmd( "bind .da <Control-c> {set choice 8}; bind .da <Control-C> {set choice 8}" );	// empty (clear) selected series
cmd( "bind .da <Control-a> {set choice 24}; bind .da <Control-A> {set choice 24}" );	// insert new series
cmd( "bind .da <Control-i> {set choice 34}; bind .da <Control-I> {set choice 34}" );	// sort selected in inverse order
cmd( "bind .da <Control-f> { set choice 39 }; bind .da <Control-F> { set choice 39 }" );	// search first
cmd( "bind .da <F3> { set choice 40 }; bind .da <Control-n> { set choice 40 }; bind .da <Control-N> { set choice 40 }" );	// search next
cmd( "bind .da <Control-greater> { set choice 6 }" );	// insert series
cmd( "bind .da <Control-less> { set choice 7 }" );	// remove series
cmd( "bind .da <Control-s> { set choice 5 }; bind .da <Control-S> { set choice 5 }" );	// sort up
cmd( "bind .da <Control-o> { set choice 38 }; bind .da <Control-O> { set choice 38 }" );	// sort down
cmd( "bind .da <Control-r> { set choice 15 }; bind .da <Control-R> { set choice 15 }" );	// sort up nice (end)
cmd( "bind .da <Control-u> { set choice 14 }; bind .da <Control-U> { set choice 14 }" );	// un-sort
cmd( "bind .da <Control-g> { set choice 4 }; bind .da <Control-G> { set choice 4 }" );	// launch gnuplot
cmd( "bind .da <Control-p> { set choice 1 }; bind .da <Control-P> { set choice 1 }" );	// plot
cmd( "bind .da <Control-v> { set fromPlot false; set choice 11 }; bind .da <Control-V> { set fromPlot false; set choice 11 }" );	// save plot
cmd( "bind .da <Control-e> { set choice 10 }; bind .da <Control-E> { set choice 10 }" );	// save data
cmd( "bind .da <Control-d> { set choice 36 }; bind .da <Control-D> { set choice 36 }" );	// show data
cmd( "bind .da <Control-t> { set choice 12 }; bind .da <Control-D> { set choice 12 }" );	// statistics

// create special sort procedure to keep names starting with underline at the end
cmd( "proc comp_und { n1 n2 } { \
		if [ string equal $n1 $n2 ] { \
			return 0 \
		}; \
		if { ! [ expr { [ string index $n1 0 ] == \"_\" && [ string index $n2 0 ] == \"_\" } ] } { \
			if { [ string index $n1 0 ] == \"_\" } { \
				return 1 \
			}; \
			if { [ string index $n2 0 ] == \"_\" } { \
				return -1 \
			}; \
		}; \
		set listn [ lsort -dictionary [ list $n1 $n2 ] ]; \
		if [ string equal [ lindex $listn 0 ] $n1 ] { \
			return -1 \
		} { \
			return 1 \
		}; \
	}" );

// procedure to add a new plot to the plot list
cmd( "proc add_plot { plot name } { \
		.da.vars.pl.v insert end \"$plot) $name\"; \
		.da.vars.pl.v selection clear 0 end; \
		.da.vars.pl.v selection set end; \
		.da.vars.pl.v activate end; \
		.da.vars.pl.v see end \
	}" );

// grab focus when called from LSD Debugger
Tcl_SetVar( inter, "running", running ? "1" : "0", 0 );
cmd( "if $running { showtop .da overM 0 1 } { showtop .da overM 0 1 0 }" );

if(num_var==0)
  cmd( "tk_messageBox -parent .da -type ok -title \"Analysis of Results\" -icon info -message \"There are no series available\" -detail \"Click on button 'Add...' to load series from results files.\n\nIf you were looking for data after a simulation run, please make sure you have selected the series to be saved, or have not set the objects containing them to not be computed.\"" );  

// make a copy to allow insertion of new temporary variables
cmd( "if [ info exists ModElem ] { set DaModElem $ModElem } { set DaModElem [ list ] }" );

// main loop

there:

// sort the list of available variables
cmd( "if [ info exists DaModElem ] { set DaModElem [ lsort -unique -dictionary $DaModElem ] }" );

// enable/disable the remove series button in the series toolbar
cmd( "if { [ .da.vars.ch.v size ] > 0 } { .da.vars.b.out conf -state normal } { .da.vars.b.out conf -state disabled }" );

// reset first second scale series if option is disabled
cmd( "if { ! $y2 } { set num_y2 2 }" );

// update entry boxes with linked variables
cmd( "write_disabled .da.f.h.v.ft.from.mnc $minc" );
cmd( "write_disabled .da.f.h.v.ft.to.mxc $maxc" );
cmd( "write_disabled .da.f.h.v.sc.min.min [ format \"%%.[ expr $pdigits ]g\" $miny ]" );
cmd( "write_disabled .da.f.h.v.sc.max.max [ format \"%%.[ expr $pdigits ]g\" $maxy ]" );
cmd( "write_disabled .da.f.h.v.y2.f.e $num_y2" );
cmd( "write_any .da.f.tit.ps.e $point_size" ); 
cmd( "write_any .da.f.tit.pr.e $pdigits" ); 

// update on-screen statistics
cmd( ".da.com.nvar conf -text \"Series = %d\"", num_var );
cmd( ".da.com.ncas conf -text \"Cases = %d\"", num_c );
cmd( ".da.com.selec conf -text \"Series = [ .da.vars.ch.v size ]\"" );
cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );

// analysis command loop
while( ! *choice )
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
		goto there;
	}
}   

// update linked variables with values in entry boxes
cmd( "set minc [ .da.f.h.v.ft.from.mnc get ]" );
cmd( "set maxc [ .da.f.h.v.ft.to.mxc get ]" );
cmd( "set miny [ .da.f.h.v.sc.min.min get ]" );
cmd( "set maxy [ .da.f.h.v.sc.max.max get ]" );
cmd( "set num_y2 [ .da.f.h.v.y2.f.e get ]" );
cmd( "set point_size [ .da.f.tit.ps.e get ]" ); 
cmd( "set pdigits [ .da.f.tit.pr.e get ]" ); 

cmd( "set nv [ .da.vars.ch.v size ]" );

if ( point_size <= 1 || point_size > 10 )
	point_size = 1.0;
if ( pdigits < 1 || pdigits > 8 )
	pdigits = 4;

if(*choice==1 && time_cross==1 && xy==0) //Plot cross section
 *choice=9;

if(*choice==1 && time_cross==0 && xy==1) //Plot XY
 *choice=17;

if(*choice==1 && time_cross==1 && xy==1) //Plot XY Cross section
 *choice=18;

if(*choice==12 && time_cross==1) //Statistics cross section
 *choice=13;

switch(*choice)
{

//exit
case 2:
cmd( "if { [ .da.vars.pl.v size ] != 0 } { set answer [ tk_messageBox -parent .da -type okcancel -title Confirmation -icon question -default ok -message \"Exit Analysis of Results?\" -detail \"All the plots and series created and not saved will be lost.\"] } { set answer ok }" );
app=(char *)Tcl_GetVar(inter, "answer",0);

cmd( "if {[string compare $answer ok] == 0} { } {set choice 0}" );
if(*choice==0)
  goto there;

delete[] vs;

cmd( "destroytop .da" );
uncover_browser( );
	
Tcl_UnlinkVar(inter, "auto");
Tcl_UnlinkVar(inter, "auto_x");
Tcl_UnlinkVar(inter, "minc");
Tcl_UnlinkVar(inter, "maxc");
Tcl_UnlinkVar(inter, "miny");
Tcl_UnlinkVar(inter, "maxy");
Tcl_UnlinkVar(inter, "logs");
Tcl_UnlinkVar(inter, "allblack");
Tcl_UnlinkVar(inter, "grid");
Tcl_UnlinkVar(inter,"point_size");
Tcl_UnlinkVar(inter,"tc");
Tcl_UnlinkVar(inter,"line_point");
Tcl_UnlinkVar(inter,"xy");
Tcl_UnlinkVar(inter, "pdigits");
Tcl_UnlinkVar(inter, "watch");
Tcl_UnlinkVar(inter, "gnu");
Tcl_UnlinkVar(inter, "num_y2");
Tcl_UnlinkVar(inter, "cur_plot");
Tcl_UnlinkVar(inter, "nv");

cmd( "catch [set a [glob -nocomplain plotxy_*]]" ); //remove directories
cmd( "catch [foreach b $a {catch [file delete -force $b]}]" );
return;
  
 
//Brutally shut down the system
case 35:
error_hard( msg, "Abort requested", "If error persists, please contact developers." );
myexit(20);


// MAIN BUTTONS OPTIONS

//Plot
case 1:
  cur_plot++;
  
  plot_tseries(choice);
  
  if(*choice==2) //plot aborted
   cur_plot--;
  else
   cmd( "add_plot $cur_plot $tit" );
  
  *choice=0;
  goto there;

  
//Plot the cross-section plot. That is, the vars selected form
//one series for each time step selected
case 9:
   set_cs_data(choice);
   
   if(*choice==2) //plot aborted
    {
    *choice=0;
    goto there;
    }

   cur_plot++;
	
   plot_cross(choice);

   if(*choice==2) //plot aborted
    cur_plot--;
   else
	cmd( "add_plot $cur_plot $tit" );
	
    *choice=0;
    goto there;

	
//Plot XY: plot_gnu or phase diagram, depending on how many series are selected
case 17:
  cur_plot++;
  
  if(nv>1)
    plot_gnu(choice);
  else
    plot_phase_diagram(choice);

  if(*choice==2)
   cur_plot--;
  else
   cmd( "add_plot $cur_plot $tit" );
  
  *choice=0;
  goto there;

  
//Plot_cs_xy
case 18:
  cur_plot++;

  plot_cs_xy(choice);

  if(*choice==2) //plot aborted
   cur_plot--;
  else
   cmd( "add_plot $cur_plot $tit" );
  
  *choice=0;
  goto there;

  
/*
plot a lattice. Data must be stored on a single time step organized for lines and columns in sequence
*/
case 23:
  cur_plot++;

  plot_lattice(choice);
  
  if(*choice==2) //plot aborted
   cur_plot--;
  else   
   cmd( "add_plot $cur_plot $tit" );
  
  *choice=0;
  goto there;


// plot histograms
case 32:
  cur_plot++;
  
  cmd( "set choice $tc" );
  if(*choice==0)
    histograms(choice);
  else
    histograms_cs(choice);
  
  if(*choice==2) //plot aborted
   cur_plot--;
  else   
   cmd( "add_plot $cur_plot $tit" );
    
  *choice=0;
  goto there;


//Print the data series in the log window
case 36: 
   plog_series(choice);
   cmd( "wm deiconify .log; raise .log .da" );
   *choice=0;
   goto there;
   
   
//show the equation for the selected variable
case 16:
cmd( "set a [split $res]; set b [lindex $a 0]" );
app=(char *)Tcl_GetVar(inter, "b", 0);
*choice = 2;	// point .da window as parent for the following window
show_eq(app, choice);
cmd( "raise .da .; focus .da" );
*choice=0;
goto there;


//Save data in the Vars. to plot listbox
case 10:
 save_datazip(choice);
 *choice=0;
 goto there;


//Statistics
case 12:
 statistics(choice);
 cmd( "wm deiconify .log; raise .log .da" );
 *choice=0;
 goto there;

 
//Statistics Cross Section
case 13:
 set_cs_data(choice);
 if(*choice!=2)
  {
   statistics_cross(choice);
   cmd( "wm deiconify .log; raise .log .da" );
  }
 *choice=0;
 goto there;


//create the postscript for a plot
case 11:
cmd( "if $fromPlot { set choice 1 } { set choice 0 }" );
if ( ! choice )
{
	cmd( "set choice [ .da.vars.pl.v size ]" );
	if ( *choice == 0 )		 // no plots to save
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No plot available\" -detail \"Place one or more series in the Series Selected listbox and select 'Plot' to produce a plot.\"" );
		goto there;
	}

	do
	{
		*choice=0;
		cmd( "set iti [.da.vars.pl.v curselection]" );
		cmd( "if {[string length $iti] == 0} {set choice 1}" );

		if(*choice==1)
		{
			*choice=0;
			cmd( "newtop .da.a \"Save Plot\" { set choice 2 } .da" );
			cmd( "label .da.a.l -text \"Select one plot from the \nPlots listbox before clicking 'Ok'\"" );
			cmd( "pack .da.a.l -pady 10 -padx 5" );
			cmd( "okcancel .da.a b { set choice 1 } { set choice 2 }" );
			cmd( "showtop .da.a centerW 0 0 0" );
			while(*choice==0)
				Tcl_DoOneEvent(0);
			cmd( "destroytop .da.a" );
		}
	}
	while ( *choice == 1 );

	if(*choice==2)
	   {*choice=0;
		goto there;
	   }

	*choice=0;
}

cmd( "set iti [.da.vars.pl.v curselection]" );
cmd( "set it [.da.vars.pl.v get $iti]" );
cmd( "scan $it %%d)%%s a b" );
cmd( "set choice [winfo exists .da.f.new$a]" );

if( *choice == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Cannot save plot\" -detail \"The plot was produced in Gnuplot. Please recreate the plot and use the Gnuplot tools (toolbar buttons) to save it.\"" );
	goto there;  
}

cmd( "newtop .da.file \"Save Plot\" { set choice 2 } .da" );

cmd( "frame .da.file.l -bd 2" );
cmd( "label .da.file.l.l1 -text \"Settings for plot\"" );
cmd( "label .da.file.l.l2 -fg red -text \"($a) $b\"" );
cmd( "pack .da.file.l.l1 .da.file.l.l2" );

cmd( "set cm \"color\"" );
cmd( "frame .da.file.col -relief groove -bd 2" );
cmd( "radiobutton .da.file.col.r1 -text \"Color\" -variable cm -value color" );
cmd( "radiobutton .da.file.col.r2 -text \"Grayscale\" -variable cm -value gray" );
cmd( "radiobutton .da.file.col.r3 -text \"Mono\" -variable cm -value mono" );
cmd( "pack .da.file.col.r1 .da.file.col.r2 .da.file.col.r3 -side left" );

cmd( "set res 0" );
cmd( "frame .da.file.pos -relief groove -bd 2" );
cmd( "radiobutton .da.file.pos.p1 -text \"Landscape\" -variable res -value 0" );
cmd( "radiobutton .da.file.pos.p2 -text \"Portrait\" -variable res -value 1" );
cmd( "pack .da.file.pos.p1 .da.file.pos.p2 -side left -ipadx 11" );

cmd( "set dim 270" );
cmd( "frame .da.file.dim -bd 2" );
cmd( "label .da.file.dim.l1 -text \"Dimension\"" );
cmd( "entry .da.file.dim.n -width 4 -validate focusout -vcmd { if [ string is integer %%P ] { set dim %%P; return 1 } { %%W delete 0 end; %%W insert 0 $dim; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.file.dim.n insert 0 $dim" );
cmd( "label .da.file.dim.l2 -text \"(mm@96DPI)\"" );
cmd( "pack .da.file.dim.l1 .da.file.dim.n .da.file.dim.l2 -side left" );

cmd( "set heightpost 1" );
cmd( "checkbutton .da.file.lab -text \"Include plot labels\" -variable heightpost -onvalue 1 -offvalue 0" );
cmd( "set choice $a" );
if ( plot_l[ *choice ] == plot_nl[ *choice ] )
	cmd( ".da.file.lab conf -state disabled" );

cmd( "pack .da.file.l .da.file.col .da.file.pos .da.file.dim .da.file.lab -pady 5 -padx 5" );
cmd( "okcancel .da.file b { set choice 1 } { set choice 2 }" );
cmd( "showtop .da.file centerW" );

*choice=0;
  while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set dim [ .da.file.dim.n get ]" ); 
cmd( "destroytop .da.file" );

if(*choice==2)
 {*choice=0;
  goto there;
 }

cmd( "set fn \"$b.eps\"" );
cmd( "set fn [ tk_getSaveFile -parent .da -title \"Save Plot File\" -defaultextension .eps -initialfile $fn -filetypes { { {Encapsulated Postscript files} {.eps} } { {All files} {*} } } ]; if { [string length $fn] == 0 } { set choice 2 }" );

if(*choice==2)
 {*choice=0;
  goto there;
 }

cmd( "set dd \"\"" );
cmd( "append dd $dim m" );
cmd( "set fn [file nativename $fn]" ); //return the name in the platform specific format

cmd( "set choice $a" );
if ( plot_l[ *choice ] == plot_nl[ *choice ] )	// no labels?
{
	if ( plot_l[ *choice ] > 0 )
		cmd( "set heightpost %d", plot_l[ *choice ] );
	else
	{
		cmd( "set str [.da.f.new$a.f.plots conf -height]" );
		cmd( "scan $str \"%%s %%s %%s %%s %%d\" trash1 trash2 trash3 trash4 heighpost" );
	} 
}
else
	cmd( "if { $heightpost == 1 } { set heightpost %d } { set heightpost %d }", plot_l[ *choice ], plot_nl[ *choice ] );

cmd( "if { ! [ info exists zoomLevel%d ] } { set zoomLevel%d 1.0 }", *choice, *choice );
cmd( ".da.f.new$a.f.plots postscript -x 0 -y 0 -height [ expr round( $heightpost * $zoomLevel%d ) ] -width [ expr round( %d * $zoomLevel%d ) ] -pagewidth $dd -rotate $res -colormode $cm -file \"$fn\"", *choice, plot_w[ *choice ], *choice );
cmd( "plog \"\nPlot saved: $fn\n\"" );

*choice = 0;
goto there;


// SELECTION BOX CONTEXT MENUS

//Use right button of the mouse to select all series with a given label
case 30:
Tcl_LinkVar(inter, "compvalue", (char *) &compvalue, TCL_LINK_DOUBLE);
cmd( "set a [split $res]" );
cmd( "set b [lindex $a 0]" );
cmd( "set c [lindex $a 1]" ); //get the tag value
cmd( "set i [llength [split $c {_}]]" );
cmd( "set ntag $i" );
cmd( "set ssys 2" );
cmd( "if { ! [info exist tvar] } {set tvar $maxc}" );
cmd( "if { ! [info exist cond] } {set cond 1}" );

cmd( "newtop .da.a \"Select Series\" { set choice 2 } .da" );

cmd( "frame .da.a.tit" );
cmd( "label .da.a.tit.l -text \"Select series with label\"" );
cmd( "label .da.a.tit.s -text \"$b\" -foreground red" );
cmd( "pack .da.a.tit.l .da.a.tit.s" );
cmd( "frame .da.a.q -relief groove -bd 2" );
cmd( "frame .da.a.q.f1" );
cmd( "radiobutton .da.a.q.f1.c -text \"Select all\" -variable ssys -value 2 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state disabled; .da.a.c.v.c.e conf -state disabled; .da.a.c.o.eq conf -state disabled; .da.a.c.o.dif conf -state disabled; .da.a.c.o.geq conf -state disabled; .da.a.c.o.g conf -state disabled; .da.a.c.o.seq conf -state disabled; .da.a.c.o.s conf -state disabled}" );
cmd( "bind .da.a.q.f1.c <Return> {.da.a.q.f1.c invoke; focus .da.a.b.ok}" );
cmd( "bind .da.a.q.f1.c <Down> {focus .da.a.q.f.c; .da.a.q.f.c invoke}" );
cmd( "pack .da.a.q.f1.c" );
cmd( "pack .da.a.q.f1 -anchor w" );
cmd( "frame .da.a.q.f" );
cmd( "radiobutton .da.a.q.f.c -text \"Select by series' tags\" -variable ssys -value 1 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state normal}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state disabled; .da.a.c.v.c.e conf -state disabled; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f.c <Up> {focus .da.a.q.f1.c; .da.a.q.f1.c invoke}" );
cmd( "bind .da.a.q.f.c <Return> {focus .da.a.q.f.l.e0; .da.a.q.f.l.e0 selection range 0 end}" );
cmd( "bind .da.a.q.f.c <Down> {focus .da.a.q.f3.s; .da.a.q.f3.s invoke}" );
cmd( "pack .da.a.q.f.c -anchor w" );
cmd( "frame .da.a.q.f.l" );
cmd( "for {set x 0} {$x<$i} {incr x} {if {$x > 0} {label .da.a.q.f.l.s$x -text \u2014}; entry .da.a.q.f.l.e$x -width 4 -textvariable v$x -justify center -state disabled}" );
cmd( "for { set x 0 } { $x < $i } { incr x } { if { $x > 0 } { pack .da.a.q.f.l.s$x -side left }; pack .da.a.q.f.l.e$x -side left; bind .da.a.q.f.l.e$x <Return> [ subst -nocommand { focus .da.a.q.f.l.e[ expr $x + 1 ]; .da.a.q.f.l.e[ expr $x + 1 ] selection range 0 end } ]; bind .da.a.q.f.l.e$x <KeyRelease> { .da.a.q.f.c invoke } }; incr x -1; bind .da.a.q.f.l.e$x <Return> { focus .da.a.b.ok }" );
cmd( "pack .da.a.q.f.l -anchor w -padx 25" );
cmd( "pack .da.a.q.f -anchor w" );
cmd( "frame .da.a.q.f3" );
cmd( "radiobutton .da.a.q.f3.s -text \"Select by series values\" -variable ssys -value 3 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state normal; .da.a.c.v.c.e conf -state normal; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f3.s <Up> {focus .da.a.q.f.c; .da.a.q.f.c invoke}" );
cmd( "bind .da.a.q.f3.s <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
cmd( "bind .da.a.q.f3.s <Down> {focus .da.a.q.f2.s; .da.a.q.f2.s invoke}" );
cmd( "pack .da.a.q.f3.s -anchor w" );
cmd( "pack .da.a.q.f3 -anchor w" );
cmd( "frame .da.a.q.f2" );
cmd( "radiobutton .da.a.q.f2.s -text \"Select by values from another series\" -variable ssys -value 4 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state normal; .da.a.c.v.t.e2 conf -state normal; .da.a.c.v.c.e conf -state normal; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f2.s <Up> {focus .da.a.q.f3.s; .da.a.q.f3.s invoke}" );
cmd( "bind .da.a.q.f2.s <Return> {focus .da.a.q.f2.f.e; .da.a.q.f2.f.e selection range 0 end}" );
cmd( "pack .da.a.q.f2.s -anchor w" );
cmd( "frame .da.a.q.f2.f" );
cmd( "label .da.a.q.f2.f.l -text \"Label\"" );
cmd( "entry .da.a.q.f2.f.e -width 17 -textvariable svar -justify center -state disabled" );
cmd( "bind .da.a.q.f2.f.e <KeyRelease> {if { %%N < 256 && [info exists DaModElem] } { set bb1 [.da.a.q.f2.f.e index insert]; set bc1 [.da.a.q.f2.f.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1 !=-1 } {set bd1 [lindex $DaModElem $bf1]; .da.a.q.f2.f.e delete 0 end; .da.a.q.f2.f.e insert 0 $bd1; .da.a.q.f2.f.e index $bb1; .da.a.q.f2.f.e selection range $bb1 end } } }" );
cmd( "bind .da.a.q.f2.f.e <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
cmd( "pack .da.a.q.f2.f.l .da.a.q.f2.f.e -anchor w -side left" );
cmd( "pack .da.a.q.f2.f -anchor w -padx 22" );
cmd( "pack .da.a.q.f2 -anchor w" );
cmd( "frame .da.a.c -relief groove -bd 2" );
cmd( "frame .da.a.c.o" );
cmd( "radiobutton .da.a.c.o.eq -text \"Equal (=)\" -variable cond -value 1 -state disabled" );
cmd( "radiobutton .da.a.c.o.dif -text \"Different (\u2260)\" -variable cond -value 0 -state disabled" );
cmd( "radiobutton .da.a.c.o.geq -text \"Larger or equal (\u2265)\" -variable cond -value 2 -state disabled" );
cmd( "radiobutton .da.a.c.o.g -text \"Larger (>)\" -variable cond -value 3 -state disabled" );
cmd( "radiobutton .da.a.c.o.seq -text \"Smaller or equal (\u2264)\" -variable cond -value 4 -state disabled" );
cmd( "radiobutton .da.a.c.o.s -text \"Smaller (<)\" -variable cond -value 5 -state disabled" );
cmd( "pack .da.a.c.o.eq .da.a.c.o.dif .da.a.c.o.geq .da.a.c.o.g .da.a.c.o.seq .da.a.c.o.s -anchor w" );
cmd( "frame .da.a.c.v" );
cmd( "frame .da.a.c.v.c" );
cmd( "label .da.a.c.v.c.l -text \"Comparison value\"" );
cmd( "entry .da.a.c.v.c.e -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set compvalue %%P; return 1 } { %%W delete 0 end; %%W insert 0 $compvalue; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any .da.a.c.v.c.e $compvalue" ); 
cmd( "bind .da.a.c.v.c.e <Return> {focus .da.a.c.v.t.e2; .da.a.c.v.t.e2 selection range 0 end }" );
cmd( "pack .da.a.c.v.c.l .da.a.c.v.c.e" );
cmd( "frame .da.a.c.v.t" );
cmd( "label .da.a.c.v.t.t -text \"Case\"" );
cmd( "entry .da.a.c.v.t.e2 -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set tvar %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tvar; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any .da.a.c.v.t.e2 $tvar" ); 
cmd( "bind .da.a.c.v.t.e2 <Return> {focus .da.a.b.ok}" );
cmd( "pack .da.a.c.v.t.t .da.a.c.v.t.e2" );
cmd( "pack .da.a.c.v.c .da.a.c.v.t -ipady 10" );
cmd( "pack .da.a.c.o .da.a.c.v -anchor w -side left -ipadx 5" );
cmd( "pack .da.a.tit .da.a.q .da.a.c -expand yes -fill x -padx 5 -pady 5" );

cmd( "okhelpcancel .da.a b { set choice 1 } { LsdHelp mdatares.html#batch_sel } { set choice 2 }" );
cmd( "showtop .da.a topleftW 0 0" );
cmd( "focus .da.a.q.f1.c" );

*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set tvar [ .da.a.c.v.t.e2 get ]" ); 
cmd( "set compvalue [ .da.a.c.v.c.e get ]" ); 
Tcl_UnlinkVar(inter, "compvalue");

if(*choice==2)
{
cmd( "destroytop .da.a" );
*choice=0;
goto there;
}

cmd( "if {[.da.vars.ch.v get 0] == \"\"} {set tit \"\"} {}" );
cmd( "set choice $ssys" );
if(*choice==2)
 {
 cmd( "set tot [.da.vars.lb.v get 0 end]" );
 cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} {  .da.vars.ch.v insert end \"$i\"  } {}}" );
 cmd( "if {\"$tit\" == \"\"} {set tit [.da.vars.ch.v get 0]} {}" );
 }
 
if(*choice== 1)
{
cmd( "set choice $cond" );
i=*choice;

*choice=-1;
cmd( "for {set x 0} {$x<$i} {incr x} {if {[.da.a.q.f.l.e$x get]!=\"\"} {set choice $x}}" );
if(*choice==-1)
{
cmd( "set tot [.da.vars.lb.v get 0 end]" );
cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} { .da.vars.ch.v insert end \"$i\"  } }" );
}
else
 {
cmd( "set tot [.da.vars.lb.v get 0 end]" );
cmd( "if { [info exist vcell]==1} {unset vcell} {}" );
cmd( "set choice $ntag" );
for(j=0; j<*choice; j++)
 cmd( "lappend vcell $v%d", j );

switch(i) 
{
case 0:
  cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] == [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0 } }; if $c { .da.vars.ch.v insert end \"$i\" } } }" );
 break;
case 1:
  cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] != [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
case 2:
   cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] > [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
case 3:
   cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] >= [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
case 4:
   cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] < [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
case 5:
   cmd( "foreach i $tot { if { [lindex [split $i] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] <= [lindex [split [lindex [split $i] 1] {_}] $x] } { set c 0} {} }; if { $c==1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
 break;
} 
}
*choice=0;
}

if ( *choice == 3 || *choice == 4 )
{
l = *choice;
cmd( "set choice $cond" );
p=*choice;
cmd( "set tot [.da.vars.lb.v get 0 end]" );
cmd( "set choice [llength $tot]" );
j=*choice;

if ( l == 3 )
	app = ( char * ) Tcl_GetVar( inter, "b", 0 );
else
	app = ( char * ) Tcl_GetVar( inter, "svar", 0 );

strcpy(str3,app);

cmd( "set choice $tvar" );
h=*choice;

for(i=0; i<j; i++)
 {
  cmd( "set res [lindex $tot %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str1, str2, &l, &m, &k);
  if(h>=l && h<=m && !strcmp(str1, str3))
   {
   datum = vs[ k ].data;
   r=0;
   if(is_finite(datum[h]))		// ignore NaNs
    switch(p)
    {
    case 0: if(datum[h]!=compvalue) r=1;
    break;
    case 1: if(datum[h]==compvalue) r=1;
    break;
    case 2: if(datum[h]>=compvalue) r=1;
     break;
    case 3: if(datum[h] > compvalue) r=1;
     break;
    case 4: if(datum[h]<=compvalue) r=1;
     break;
    case 5: if(datum[h] < compvalue) r=1;
     break;
        
    }
    if(r==1)
     { 
      cmd( "set templ $tot" );
      cmd( "set choice [lsearch $templ \"$b %s *\"]", str2 );

      while(*choice>=0)
       {
       cmd( ".da.vars.ch.v insert end [lindex $templ $choice]" );
       cmd( "set templ [lreplace $templ $choice $choice]" );
       }
     }
   }
 }
}

cmd( "destroytop .da.a" );

cmd( "if {\"$tit\" == \"\"} {set tit [.da.vars.ch.v get 0]} {}" );

*choice=0;
goto there;


//Use right button of the mouse to remove series selected with different criteria
case 33:
cmd( "unset -nocomplain compvalue" );
Tcl_LinkVar(inter, "compvalue", (char *) &compvalue, TCL_LINK_DOUBLE);
cmd( "set a [split $res]" );
cmd( "set b [lindex $a 0]" );
cmd( "set c [lindex $a 1]" ); //get the tag value
cmd( "set i [llength [split $c {_}]]" );
cmd( "set ntag $i" );
cmd( "set ssys 2" );
cmd( "if { ! [info exist tvar] } { set tvar $maxc }" );
cmd( "if { ! [info exist cond] } { set cond 1 }" );
cmd( "if { ! [info exist selOnly] } { set selOnly 0 }" );

cmd( "newtop .da.a \"Unselect Series\" { set choice 2 } .da" );

cmd( "frame .da.a.tit" );
cmd( "label .da.a.tit.l -text \"Unselect series with label\"" );
cmd( "label .da.a.tit.s -text \"$b\" -foreground red" );
cmd( "pack .da.a.tit.l .da.a.tit.s" );
cmd( "frame .da.a.q -relief groove -bd 2" );
cmd( "frame .da.a.q.f1" );
cmd( "radiobutton .da.a.q.f1.c -text \"Unselect all\" -variable ssys -value 2 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state disabled; .da.a.c.v.c.e conf -state disabled; .da.a.c.o.eq conf -state disabled; .da.a.c.o.dif conf -state disabled; .da.a.c.o.geq conf -state disabled; .da.a.c.o.g conf -state disabled; .da.a.c.o.seq conf -state disabled; .da.a.c.o.s conf -state disabled}" );
cmd( "bind .da.a.q.f1.c <Return> {.da.a.q.f1.c invoke; focus .da.a.b.ok}" );
cmd( "bind .da.a.q.f1.c <Down> {focus .da.a.q.f.c; .da.a.q.f.c invoke}" );
cmd( "pack .da.a.q.f1.c" );
cmd( "pack .da.a.q.f1 -anchor w" );
cmd( "frame .da.a.q.f" );
cmd( "radiobutton .da.a.q.f.c -text \"Unselect by series' tags\" -variable ssys -value 1 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state normal}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state disabled; .da.a.c.v.c.e conf -state disabled; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f.c <Up> {focus .da.a.q.f1.c; .da.a.q.f1.c invoke}" );
cmd( "bind .da.a.q.f.c <Return> {focus .da.a.q.f.l.e0; .da.a.q.f.l.e0 selection range 0 end}" );
cmd( "bind .da.a.q.f.c <Down> {focus .da.a.q.f3.s; .da.a.q.f3.s invoke}" );
cmd( "pack .da.a.q.f.c -anchor w" );
cmd( "frame .da.a.q.f.l" );
cmd( "for {set x 0} {$x<$i} {incr x} {if {$x > 0} {label .da.a.q.f.l.s$x -text \u2014}; entry .da.a.q.f.l.e$x -width 4 -textvariable v$x -justify center -state disabled}" );
cmd( "for { set x 0 } { $x < $i } { incr x } { if { $x > 0 } { pack .da.a.q.f.l.s$x -side left }; pack .da.a.q.f.l.e$x -side left; bind .da.a.q.f.l.e$x <Return> [ subst -nocommand { focus .da.a.q.f.l.e[ expr $x + 1 ]; .da.a.q.f.l.e[ expr $x + 1 ] selection range 0 end } ]; bind .da.a.q.f.l.e$x <KeyRelease> { .da.a.q.f.c invoke } }; incr x -1; bind .da.a.q.f.l.e$x <Return> { focus .da.a.b.ok }" );
cmd( "pack .da.a.q.f.l -anchor w -padx 25" );
cmd( "pack .da.a.q.f -anchor w" );
cmd( "frame .da.a.q.f3" );
cmd( "radiobutton .da.a.q.f3.s -text \"Unselect by series values\" -variable ssys -value 3 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state disabled; .da.a.c.v.t.e2 conf -state normal; .da.a.c.v.c.e conf -state normal; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f3.s <Up> {focus .da.a.q.f.c; .da.a.q.f.c invoke}" );
cmd( "bind .da.a.q.f3.s <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
cmd( "bind .da.a.q.f3.s <Down> {focus .da.a.q.f2.s; .da.a.q.f2.s invoke}" );
cmd( "pack .da.a.q.f3.s -anchor w" );
cmd( "pack .da.a.q.f3 -anchor w" );
cmd( "frame .da.a.q.f2" );
cmd( "radiobutton .da.a.q.f2.s -text \"Unselect by values from another series\" -variable ssys -value 4 -command {for {set x 0} {$x<$i} {incr x} {.da.a.q.f.l.e$x conf -state disabled}; .da.a.q.f2.f.e conf -state normal; .da.a.c.v.t.e2 conf -state normal; .da.a.c.v.c.e conf -state normal; .da.a.c.o.eq conf -state normal; .da.a.c.o.dif conf -state normal; .da.a.c.o.geq conf -state normal; .da.a.c.o.g conf -state normal; .da.a.c.o.seq conf -state normal; .da.a.c.o.s conf -state normal}" );
cmd( "bind .da.a.q.f2.s <Up> {focus .da.a.q.f3.s; .da.a.q.f3.s invoke}" );
cmd( "bind .da.a.q.f2.s <Return> {focus .da.a.q.f2.f.e; .da.a.q.f2.f.e selection range 0 end}" );
cmd( "pack .da.a.q.f2.s -anchor w" );
cmd( "frame .da.a.q.f2.f" );
cmd( "label .da.a.q.f2.f.l -text \"Label\"" );
cmd( "entry .da.a.q.f2.f.e -width 17 -textvariable svar -justify center -state disabled" );
cmd( "bind .da.a.q.f2.f.e <KeyRelease> {if { %%N < 256 && [info exists DaModElem] } { set bb1 [.da.a.q.f2.f.e index insert]; set bc1 [.da.a.q.f2.f.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1 !=-1 } {set bd1 [lindex $DaModElem $bf1]; .da.a.q.f2.f.e delete 0 end; .da.a.q.f2.f.e insert 0 $bd1; .da.a.q.f2.f.e index $bb1; .da.a.q.f2.f.e selection range $bb1 end } } }" );
cmd( "bind .da.a.q.f2.f.e <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
cmd( "pack .da.a.q.f2.f.l .da.a.q.f2.f.e -anchor w -side left" );
cmd( "pack .da.a.q.f2.f -anchor w -padx 22" );
cmd( "pack .da.a.q.f2 -anchor w" );
cmd( "frame .da.a.c -relief groove -bd 2" );
cmd( "frame .da.a.c.o" );
cmd( "radiobutton .da.a.c.o.eq -text \"Equal (=)\" -variable cond -value 1 -state disabled" );
cmd( "radiobutton .da.a.c.o.dif -text \"Different (\u2260)\" -variable cond -value 0 -state disabled" );
cmd( "radiobutton .da.a.c.o.geq -text \"Larger or equal (\u2265)\" -variable cond -value 2 -state disabled" );
cmd( "radiobutton .da.a.c.o.g -text \"Larger (>)\" -variable cond -value 3 -state disabled" );
cmd( "radiobutton .da.a.c.o.seq -text \"Smaller or equal (\u2264)\" -variable cond -value 4 -state disabled" );
cmd( "radiobutton .da.a.c.o.s -text \"Smaller (<)\" -variable cond -value 5 -state disabled" );
cmd( "pack .da.a.c.o.eq .da.a.c.o.dif .da.a.c.o.geq .da.a.c.o.g .da.a.c.o.seq .da.a.c.o.s -anchor w" );
cmd( "frame .da.a.c.v" );
cmd( "frame .da.a.c.v.c" );
cmd( "label .da.a.c.v.c.l -text \"Comparison value\"" );
cmd( "entry .da.a.c.v.c.e -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set compvalue %%P; return 1 } { %%W delete 0 end; %%W insert 0 $compvalue; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any .da.a.c.v.c.e $compvalue" ); 
cmd( "bind .da.a.c.v.c.e <Return> {focus .da.a.c.v.t.e2; .da.a.c.v.t.e2 selection range 0 end }" );
cmd( "pack .da.a.c.v.c.l .da.a.c.v.c.e" );
cmd( "frame .da.a.c.v.t" );
cmd( "label .da.a.c.v.t.t -text \"Case\"" );
cmd( "entry .da.a.c.v.t.e2 -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set tvar %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tvar; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any .da.a.c.v.t.e2 $tvar" ); 
cmd( "bind .da.a.c.v.t.e2 <Return> {focus .da.a.b.ok}" );
cmd( "pack .da.a.c.v.t.t .da.a.c.v.t.e2" );
cmd( "pack .da.a.c.v.c .da.a.c.v.t -ipady 10" );
cmd( "pack .da.a.c.o .da.a.c.v -anchor w -side left -ipadx 5" );
cmd( "frame .da.a.s" );
cmd( "checkbutton .da.a.s.b -text \"Only mark items\" -variable selOnly" );
cmd( "pack .da.a.s.b" );
cmd( "pack .da.a.tit .da.a.q .da.a.c .da.a.s -expand yes -fill x -padx 5 -pady 5" );

cmd( "okhelpcancel .da.a b { set choice 1 } { LsdHelp mdatares.html#batch_sel } { set choice 2 }" );
cmd( "showtop .da.a topleftW 0 0" );
cmd( "focus .da.a.q.f1.c" );

*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set tvar [ .da.a.c.v.t.e2 get ]" ); 
cmd( "set compvalue [ .da.a.c.v.c.e get ]" ); 
Tcl_UnlinkVar(inter, "compvalue");

if(*choice==2)
{
cmd( "destroytop .da.a" );
*choice=0;
goto there;
}

cmd( "set choice $ssys" );
cmd( ".da.vars.ch.v selection clear 0 end" );

if(*choice==2)
 {
 cmd( "set tot [.da.vars.ch.v get 0 end]" );
 cmd( "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} {  .da.vars.ch.v selection set $myc  } {}; incr myc}" );
 }
 
if(*choice== 1)
{
cmd( "set choice $cond" );
i=*choice;

*choice=-1;
cmd( "for {set x 0} {$x<$i} {incr x} {if {[.da.a.q.f.l.e$x get]!=\"\"} {set choice $x}}" );
if(*choice==-1)
{
cmd( "set tot [.da.vars.ch.v get 0 end]" );
cmd( "set myc 0; foreach i $tot { if { [lindex [split $i] 0] == \"$b\"} { .da.vars.ch.v selection set $myc }; incr myc }" );
}
else
 {
cmd( "set tot [.da.vars.ch.v get 0 end]" );
cmd( "if { [info exist vcell]==1} {unset vcell} {}" );
cmd( "set choice $ntag" );
for(j=0; j<*choice; j++)
 cmd( "lappend vcell $v%d", j );

switch(i) 
{
case 0:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] == [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 1:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] != [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 2:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] > [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 3:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] >= [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 4:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] < [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
case 5:
 cmd( "set myc 0; foreach i $tot { if { [ lindex [ split $i ] 0 ] == \"$b\" } { set c 1; for { set x 0 } { $x < $ntag } { incr x } { if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] <= [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { set c 0 } }; if $c { .da.vars.ch.v selection set $myc } }; incr myc }" );  
 break;
} 
}
*choice=0;
}

if ( *choice == 3 || *choice == 4 )
{
l = *choice;
cmd( "set choice $cond" );
p=*choice;
cmd( "set tot [.da.vars.ch.v get 0 end]" );
cmd( "set choice [llength $tot]" );
j=*choice;

if ( l == 3 )
	app = ( char * ) Tcl_GetVar( inter, "b", 0 );
else
	app = ( char * ) Tcl_GetVar( inter, "svar", 0 );

strcpy(str3,app);

cmd( "set choice $tvar" );
h=*choice;

for(i=0; i<j; i++)
 {
  cmd( "set res [lindex $tot %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str1, str2, &l, &m, &k);
  if(h>=l && h<=m && !strcmp(str1, str3))
   {
   datum = vs[ k ].data;
   r=0;
   if(is_finite(datum[h]))		// ignore NaNs
    switch(p)
    {
    case 0: if(datum[h]!=compvalue) r=1;
    break;
    case 1: if(datum[h]==compvalue) r=1;
    break;
    case 2: if(datum[h]>=compvalue) r=1;
     break;
    case 3: if(datum[h] > compvalue) r=1;
     break;
    case 4: if(datum[h]<=compvalue) r=1;
     break;
    case 5: if(datum[h] < compvalue) r=1;
     break;
        
    }
    if(r==1)
     { 
      cmd( "set templ $tot" );
      cmd( "set choice [lsearch $templ \"$b %s *\"]", str2 );

      while(*choice>=0)
       {
       cmd( ".da.vars.ch.v selection set $choice" );
       cmd( "set templ [lreplace $templ $choice $choice]" );
       }
     }
   }
 }
}

cmd( "destroytop .da.a" );

cmd( "if { ! $selOnly } { set steps 0; foreach i [ .da.vars.ch.v curselection ] { .da.vars.ch.v delete [ expr $i - $steps ]; incr steps} } { if { [ llength [ .da.vars.ch.v curselection ] ] > 0 } { .da.vars.ch.v see [ lindex [ .da.vars.ch.v curselection ] 0 ] } }" );

*choice=0;
goto there;


// PLOTS LIST CONTEXT ACTIONS

//remove a plot
case 20:
cmd( "set answer [tk_messageBox -parent .da -type yesno -title Confirmation -message \"Delete plot?\" -detail \"Press 'Yes' to delete plot:\\n$tit\" -icon question -default yes]" );
cmd( "if {[string compare $answer yes] == 0} { set choice 1} {set choice 0}" );
if(*choice==0)
 goto there;
cmd( "scan $it %%d)%%s a b" );
cmd( "set ex [winfo exists .da.f.new$a]" );
cmd( "if { $ex == 1 } {destroytop .da.f.new$a; .da.vars.pl.v delete $n_it} {.da.vars.pl.v delete $n_it}" );

*choice=0;
goto there;


//Raise the clicked plot
case 3:
  cmd( "scan $it %%d)%%s a b" );
  cmd( "set ex [winfo exists .da.f.new$a]" );
  *choice=0;
  cmd( "if { $ex == 1 } { wm deiconify .da.f.new$a; raise .da.f.new$a; focus .da.f.new$a } { set choice 1 }" );
  if(*choice==1)
   {
   getcwd(dirname, MAX_PATH_LENGTH - 1);
   cmd( "set choice $a" );
   sprintf(msg, "plotxy_%d",*choice);
   chdir(msg);
   cmd( "set choice $a" );
   show_plot_gnu(*choice, choice, 1, NULL, NULL);
   chdir(dirname);
   }

  miny=maxy=0;

  *choice=0;
  goto there;

  
// SERIES TOOLBAR OPTIONS

// Sort
case 5:
  cmd( "set a [.da.vars.lb.v get 0 end]" );
  cmd( "set choice [llength $a]" );
  if(*choice==0)
    goto there;
  cmd( "set b {}" );
  cmd( "set b [lsort -command comp_und $a]" );		// use special sort procedure to keep underscores at the end
  cmd( ".da.vars.lb.v delete 0 end" );
  cmd( "foreach i $b {.da.vars.lb.v insert end $i}" );
  *choice=0;
  goto there;
  
  
// Sort (descending order)
case 38:
  cmd( "set a [.da.vars.lb.v get 0 end]" );
  cmd( "set choice [llength $a]" );
  if(*choice==0)
    goto there;
  cmd( "set b {}" );
  cmd( "set b [lsort -decreasing -dictionary $a]" );
  cmd( ".da.vars.lb.v delete 0 end" );
  cmd( "foreach i $b {.da.vars.lb.v insert end $i}" );
  *choice=0;
  goto there;
 

//sort the selection in selected series list in inverse order
case 34:
   cmd( "set a [.da.vars.ch.v curselection]" );
   cmd( "set choice [llength $a]" );
   if(*choice==0)
    goto there;
   cmd( "set b {}" );
   cmd( "foreach i $a {lappend b [.da.vars.ch.v get $i]}" );
   cmd( "set c [lsort -decreasing -dictionary $b]" );
   cmd( "set d -1" );
   cmd( "foreach i $a {.da.vars.ch.v delete $i; .da.vars.ch.v insert $i [lindex $c [incr d]] }" );
   *choice=0;
   goto there;

   
// Unsort
case 14:
   cmd( "set a [.da.vars.lb.v get 0 end]" );
   cmd( "set choice [llength $a]" );
   if(*choice==0)
    goto there;
   cmd( ".da.vars.lb.v delete 0 end" );
   for(i=0; i<num_var; i++)
     cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", vs[i].label, vs[i].tag, vs[i].start, vs[i].end, vs[i].rank );
   *choice=0;
   goto there;

   
// Sort on End
case 15:
   cmd( "set a [.da.vars.lb.v get 0 end]" );
   cmd( "set choice [llength $a]" );
   if(*choice==0)
    goto there;
   app_store=new store[num_var];
   for(i=0; i<num_var; i++)
     app_store[i]=vs[i];
   sort_on_end(app_store);
   cmd( ".da.vars.lb.v delete 0 end" );
   for(i=0; i<num_var; i++)
    cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", app_store[i].label, app_store[i].tag, app_store[i].start, app_store[i].end, app_store[i].rank );
    delete[] app_store;
   *choice=0;
   goto there;

   
// Find first instance of series in the available series listbox
case 39:
cmd( "set a [.da.vars.lb.v get 0 end]" );
cmd( "set choice [llength $a]" );
if(*choice==0)
 goto there;

cmd( "set srchTxt \"\"; set srchInst 1" );
cmd( "newtop .da.a \"Find Series\" { set choice 2 } .da" );
cmd( "label .da.a.l -text \"Series name (or part)\"" );
cmd( "entry .da.a.e -textvariable srchTxt -width 20 -justify center" );
cmd( "bind .da.a.e <KeyRelease> {if { %%N < 256 && [info exists DaModElem] } { set bb1 [.da.a.e index insert]; set bc1 [.da.a.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1 !=-1 } {set bd1 [lindex $DaModElem $bf1]; .da.a.e delete 0 end; .da.a.e insert 0 $bd1; .da.a.e index $bb1; .da.a.e selection range $bb1 end } } }" );
cmd( "label .da.a.n -text \"(finds first instance only,\nuse 'F3' or 'Ctrl+N' to find others)\"" );
cmd( "pack .da.a.l .da.a.e .da.a.n -pady 5 -padx 5" );
cmd( "okcancel .da.a b  { set choice 1 } { set choice 2 }" );
cmd( "bind .da.a.e <Return> { set choice 1 }" );
cmd( "bind .da.a.e <Escape> { set choice 2 }" );
cmd( "showtop .da.a centerW" );
cmd( "focus .da.a.e" );

*choice = 0;
while ( *choice == 0 )
	Tcl_DoOneEvent( 0 );

cmd( "destroytop .da.a" );

if ( *choice == 1 )
{
	cmd( "set choice [ string length $srchTxt ]" );
	if ( *choice == 0 )
		goto there;
	
	cmd( "set srchTxt [ string tolower $srchTxt ]" );
	cmd( "set a [ .da.vars.lb.v get 0 end ]; set i 0" );
	cmd( "foreach b $a { set choice [ string first $srchTxt [ string tolower [ lindex [ split $b ] 0 ] ] ]; if { $choice != -1 } { break } { set i [ expr $i + 1 ] } }" );
	if ( *choice != -1 )
	cmd( ".da.vars.lb.v selection clear 0 end; .da.vars.lb.v selection set $i; .da.vars.lb.v see $i" );
	else
		cmd( "tk_messageBox -parent .da -type ok -title \"Error\" -icon error -default ok -message \"Series not found\" -detail \"Check if the name was spelled correctly or use just part of the name. This command is case insensitive.\"" );
}

*choice=0;
goto there;
  
  
// Find subsequent instances of a series in the available series listbox
case 40:
cmd( "set choice [ string length $srchTxt ]" );
if ( *choice == 0 )
	goto there;

cmd( "set a [ .da.vars.lb.v get 0 end ]; set i 0; set inst 0" );
cmd( "foreach b $a { set choice [ string first $srchTxt [ string tolower [ lindex [ split $b ] 0 ] ] ]; if { $choice != -1 } { set inst [ expr $inst + 1 ]; if { $inst > $srchInst } { set srchInst $inst; break } }; set i [ expr $i + 1 ] }" );
if ( *choice != -1 )
	cmd( ".da.vars.lb.v selection clear 0 end; .da.vars.lb.v selection set $i; .da.vars.lb.v see $i" );
else
	cmd( "tk_messageBox -parent .da -type ok -title \"Error\" -icon error -default ok -message \"Series not found\" -detail \"No additional instance of series found.\"" );

*choice=0;
goto there;

  
// Insert the variables selected in the list of the variables to plot
case 6:
  cmd( "set a [.da.vars.lb.v curselection]" );
  cmd( "foreach i $a {.da.vars.ch.v insert end [.da.vars.lb.v get $i]}" );
  *choice=0;
  cmd( "set tit [.da.vars.ch.v get 0]" );

  goto there;

  
//Remove the vars. selected from the variables to plot
case 7:

  cmd( "set steps 0" );
  cmd( "foreach i [.da.vars.ch.v curselection] {.da.vars.ch.v delete [expr $i - $steps]; incr steps}" );
  *choice=0;

  goto there;

  
//Remove all the variables from the list of vars to plot
case 8:
  cmd( ".da.vars.ch.v delete 0 end" );
  *choice=0;

  goto there;


//Insert new series (from disk or combining existing series).
case 24:
if ( nv > 0 )
{
	cmd( "newtop .da.s \"Choose Data Source\" { set choice 2 } .da" );
	cmd( "label .da.s.l -text \"Source of additional series\"" );

	cmd( "set bidi 4" );
	cmd( "frame .da.s.i -relief groove -bd 2" );
	cmd( "radiobutton .da.s.i.c -text \"Create new series from selected\" -variable bidi -value 4" );
	cmd( "radiobutton .da.s.i.a -text \"Moving average series from selected\" -variable bidi -value 5" );
	cmd( "radiobutton .da.s.i.f -text \"File(s) of saved results\" -variable bidi -value 1" );
	cmd( "pack .da.s.i.c .da.s.i.a .da.s.i.f -anchor w" );

	cmd( "pack .da.s.l .da.s.i -expand yes -fill x -pady 5 -padx 5" );
	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#add_series } { set choice 2 }" );
	cmd( "showtop .da.s centerW" );

	*choice = 0;
	while ( *choice == 0 )
	  Tcl_DoOneEvent( 0 );

	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
	{
		*choice = 0;
		goto there;
	}

	cmd( "set choice $bidi" );
	switch ( *choice )
	{
		case 4:
			*choice = 0;
			create_series( choice );
			*choice = 0;
			goto there;
		case 5:
			*choice = 0;
			create_maverag( choice );
			*choice = 0;
			goto there;
		case 1:
			*choice = 1;
	}
}
else
	*choice = 1;

if ( *choice == 1 )
{
	bool gz = false;
#ifdef LIBZ
	const char extRes[] = ".res .res.gz";
	const char extTot[] = ".tot .tot.gz";
#else
	const char extRes[] = ".res";
	const char extTot[] = ".tot";
#endif 
 
  cmd( "set lab [tk_getOpenFile -parent .da -title \"Load Results File\" -multiple yes -initialdir [pwd] -filetypes {{{Lsd result files} {%s}} {{Lsd total files} {%s}} {{All files} {*}} }]", extRes, extTot );
  
  cmd( "set choice [llength $lab]" );
  if(*choice==0 )
   {//no file selected
    goto there; 
   }
  h=*choice;
  
  for(i=0; i<h; i++)  
  {
  cmd( "set datafile [lindex $lab %d]", i );
  app=(char *)Tcl_GetVar(inter, "datafile",0);
  strcpy(filename,app);
  
#ifdef LIBZ
  if ( strlen( filename ) > 3 && ! strcmp( &filename[ strlen( filename ) - 3 ], ".gz" ) )
	  gz = true;
#endif
  
  f = fopen( filename, "r" );

  if ( f != NULL )
   {
	fclose(f);
    file_counter++;
    insert_data_file(gz, &num_var, &num_c);
    }
   else
	plog( "\nError: could not open file: %s\n", "", filename );
  }
}

*choice=0;
goto there;


// MAIN MENU ACTIONS

// open Gnuplot
case 4:
	cmd( "if { $tcl_platform(platform) == \"unix\" } { \
			set answer [ catch { exec xterm -e gnuplot & } ] \
		} { \
			if { $tcl_platform(os) == \"Windows NT\" } { \
				set answer [ catch { exec wgnuplot & } ] \
			} { \
				set answer [ catch { exec start wgnuplot & } ] \
			} \
		}; \
		if { $answer != 0 } { \
			tk_messageBox \
				-parent .da \
				-type ok \
				-icon error \
				-title Error \
				-message \"Gnuplot failed to launch\" \
				-detail \"Gnuplot returned error '$answer' during setup.\nPlease check if Gnuplot is set up properly.\" \
		}" );
	*choice=0;
	goto there;

  
//set options for gnuplot
case 37: 
cmd( "newtop .da.a \"Gnuplot Options\" { set choice 2 } .da" );
cmd( "label .da.a.l -text \"Options for invoking Gnuplot\"" );

cmd( "set gptermTmp $gpterm" );
cmd( "frame .da.a.t -bd 2" );
cmd( "label .da.a.t.l -text \"Terminal\"" );
cmd( "entry .da.a.t.e -textvariable gptermTmp -width 12 -justify center" );
cmd( "pack .da.a.t.l .da.a.t.e -side left" );

cmd( "set gpdgrid3dTmp $gpdgrid3d" );
cmd( "frame .da.a.d -bd 2" );
cmd( "label .da.a.d.l -text \"3D grid configuration\"" );
cmd( "entry .da.a.d.e -textvariable gpdgrid3dTmp -width 12 -justify center" );
cmd( "pack .da.a.d.l .da.a.d.e -side left" );

cmd( "frame .da.a.o -relief groove -bd 2" );
cmd( "label .da.a.o.l -text \"Other options\"" );
cmd( "text .da.a.o.t -height 10 -width 45" );
cmd( ".da.a.o.t insert end \"$gpooptions\"" );
cmd( "pack .da.a.o.l .da.a.o.t" );

cmd( "pack .da.a.l .da.a.t .da.a.d .da.a.o -pady 5 -padx 5" );
cmd( "okXhelpcancel .da.a b  { Default } { set choice 3 } { set choice 1 } { LsdHelp mdatares.html#gpoptions } { set choice 2 }" );

cmd( "showtop .da.a centerW" );

gpoptions:
*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);

if(*choice==3)
 {
  cmd( "set gpdgrid3dTmp \"60,60,3\"" );
  cmd( ".da.a.o.t delete 1.0 end; .da.a.o.t insert end \"set ticslevel 0.0\"" );
  cmd( "if { $tcl_platform(platform) == \"windows\"} {set gptermTmp \"windows\"} { set gptermTmp \"x11\"}" );
  goto gpoptions;
 }
 
if ( *choice == 1 )
{
	cmd( "set gpterm $gptermTmp" );
	cmd( "set gpdgrid3d $gpdgrid3dTmp" );
	cmd( "set gpooptions [.da.a.o.t get 0.0 end]" ); 
}

cmd( "destroytop .da.a" );
*choice=0;
goto there;
 
 
case 21:		//set personal colors

cmd( "newtop .da.a \"Set Colors\" { set choice 2 } .da" );

cmd( "label .da.a.l -text \"Pick the color to change\"" );

cmd( "frame .da.a.o" );

cmd( "frame .da.a.o.l1" );
for ( i = 0; i < 10; ++i )
{
	cmd( "frame .da.a.o.l1.c%d", i );
	cmd( "label .da.a.o.l1.c%d.n -text %d", i, i + 1 );
	cmd( "label .da.a.o.l1.c%d.c -text \"\u2588\u2588\u2588\u2588\" -fg $c%d", i, i );
	cmd( "bind .da.a.o.l1.c%d.c <Button-1> { set n_col %d; set col $c%d; set choice 1 }", i, i, i );
	cmd( "pack .da.a.o.l1.c%d.n .da.a.o.l1.c%d.c -side left", i, i );
	cmd( "pack .da.a.o.l1.c%d -anchor e", i );
}
 
cmd( "frame .da.a.o.l2" );
for ( i = 0; i < 10; ++i )
{
	cmd( "frame .da.a.o.l2.c%d", i );
	cmd( "label .da.a.o.l2.c%d.n -text %d", i, i + 11 );
	cmd( "label .da.a.o.l2.c%d.c -text \"\u2588\u2588\u2588\u2588\" -fg $c%d", i, i + 10 );
	cmd( "bind .da.a.o.l2.c%d.c <Button-1> { set n_col %d; set col $c%d; set choice 1 }", i, i + 10, i + 10 );
	cmd( "pack .da.a.o.l2.c%d.n .da.a.o.l2.c%d.c -side left", i, i );
	cmd( "pack .da.a.o.l2.c%d -anchor e", i );
}

cmd( "pack .da.a.o.l1 .da.a.o.l2 -side left -pady 5 -padx 5" );
cmd( "pack .da.a.l .da.a.o -pady 5 -padx 5" );

cmd( "done .da.a b { set choice 2 }" );
cmd( "showtop .da.a centerW" );

set_col:
*choice = 0;
while ( *choice == 0 )
	Tcl_DoOneEvent( 0 );

if ( *choice == 1 )
{
	cmd( "set a [ tk_chooseColor -parent .da.a -title \"Choose Color\" -initialcolor $col ]" );
	cmd( "if { $a != \"\" } { set choice 1 } { set choice 0 }" );
	if ( *choice == 1 )
	{
		cmd( "set c$n_col $a" );
		cmd( "if { $n_col >= 10 } { set fr 2; set n_col [expr $n_col - 10 ] } { set fr 1 }" );
		cmd( ".da.a.o.l$fr.c$n_col.c configure -fg $a" );
	}
	goto set_col;
}

cmd( "destroytop .da.a" );

*choice = 0;
goto there;


case 22:
//set default colors
cmd( "init_canvas_colors" );
*choice=0;
goto there;


//help on Analysis of Result
case 41:
cmd( "LsdHelp mdatares.html" );
*choice=0;
goto there;


case 44:
//see model report

sprintf(name_rep, "report_%s.html", simul_name);
cmd( "set choice [file exists %s]", name_rep );
if(*choice == 0)
 {
  cmd( "set answer [tk_messageBox -parent .da -message \"Model report not found\" -detail \"You may create a model report file from menu Model or press 'Ok' to look for another HTML file.\" -type okcancel -title Warning -icon warning -default cancel]" );
  cmd( "if {[string compare -nocase $answer \"ok\"] == 0} {set choice 1} {set choice 0}" );
 if(*choice == 0)
  goto there;
 cmd( "set fname [tk_getOpenFile -parent .da -title \"Load Report File\" -defaultextension \".html\" -initialdir [pwd] -filetypes {{{HTML files} {.html}} {{All files} {*}} }]" );
 cmd( "if {$fname == \"\"} {set choice 0} {set choice 0}" );
 if(*choice == 0)
  goto there;

 }
else
 cmd( "set fname %s", name_rep );

if(*choice==1) //model report exists
  cmd( "LsdHtml $fname" );
  
*choice=0;
goto there;


// CANVAS OPTIONS

//Edit labels
case 26:
 cmd( "set itext [$ccanvas itemcget current -text]" );
 cmd( "set ifont [lindex [$ccanvas itemcget current -font] 0]" );
 cmd( "set idim [lindex [$ccanvas itemcget current -font] 1]" );
 cmd( "set istyle [lindex [$ccanvas itemcget current -font] 2]" );
 cmd( "set icolor [$ccanvas itemcget current -fill]" );  
 cmd( "set fontall 0" );
 cmd( "set colorall 0" );
 
 cmd( "set w $ccanvas.a" );
 cmd( "newtop $w \"Edit Label\" { set choice 2 } $ccanvas" );
 cmd( "wm geometry $w +$LX+$LY" );

 cmd( "frame $w.l" );
 cmd( "label $w.l.t -text \"New label\"" );
 cmd( "entry $w.l.e -textvariable itext -width 30 -justify center" );
 cmd( "pack $w.l.t $w.l.e" );
 
 cmd( "frame $w.format" );
 cmd( "label $w.format.tit -text \"Font name, size and style\"" );
 
 cmd( "frame $w.format.e" );
 cmd( "ttk::combobox $w.format.e.font -textvariable ifont -values [ font families ] -width 15" );
 cmd( "ttk::combobox $w.format.e.dim -textvariable idim -values [ list 4 6 8 10 11 12 14 18 24 32 48 60 ] -width 3 -justify center" );
 cmd( "ttk::combobox $w.format.e.sty -textvariable istyle -values [ list normal bold italic \"bold italic\" ] -width 10 -justify center" );
 cmd( "pack $w.format.e.font $w.format.e.dim $w.format.e.sty -padx 2 -side left" );
 
 cmd( "pack $w.format.tit $w.format.e" );

 cmd( "frame $w.c" );
 cmd( "label $w.c.l -text \"Text color\"" );
 cmd( "button $w.c.color -width 5 -text Set -foreground white -background $icolor -command { set app [ tk_chooseColor -parent $w -title \"Text Color\" -initialcolor $icolor ]; if { $app != \"\" } { set icolor $app }; $w.c.color configure -background $icolor }" );
 cmd( "pack $w.c.l $w.c.color -padx 2 -side left" );
 
 cmd( "frame $w.fall" );
 cmd( "checkbutton $w.fall.font -text \"Apply font to all text items\" -variable fontall" );
 cmd( "checkbutton $w.fall.color -text \"Apply color to all text items\" -variable colorall" );
 cmd( "pack $w.fall.font $w.fall.color" );

 cmd( "pack $w.l $w.format $w.c $w.fall -padx 5 -pady 5" );

 cmd( "okXcancel $w b Delete { set itext \"\"; set choice 1 } { set choice 1 } { set choice 2 }" );
 
 cmd( "bind $w.l.e <Return> { $w.b.ok invoke }" );
 cmd( "bind $w.format.e.font <Return> { $w.b.ok invoke }" );
 cmd( "bind $w.format.e.dim <Return> { $w.b.ok invoke }" );
 cmd( "bind $w.format.e.sty <Return> { $w.b.ok invoke }" );
 
 cmd( "showtop $w current" );
 cmd( "focus $w.l.e" );
 cmd( "$w.l.e selection range 0 end" );
 
 *choice = 0;
 while ( ! *choice )
	Tcl_DoOneEvent( 0 );

 if(*choice==1)
 {
  cmd( "if { $itext==\"\" } {$ccanvas delete current; set choice 0}" );
  if(*choice==1)
  {
   cmd( "$ccanvas itemconf current -text \"$itext\"" );
   cmd( "set ml [list $ifont $idim $istyle]" );
   cmd( "$ccanvas itemconf current -font \"$ml\"" );
   cmd( "$ccanvas itemconf current -fill $icolor" );  
  }
  cmd( "set choice $fontall" );
  if(*choice==1)
   cmd( "$ccanvas itemconf text -font \"$ml\"" );
  cmd( "set choice $colorall" );
  if(*choice==1)
   cmd( "$ccanvas itemconf text -fill $icolor" );
 } 
 
 cmd( "destroytop $w" ); 
 
 *choice=0;
 goto there;


//New labels
case 27:
 cmd( "set itext \"new text\"" );
 
 cmd( "set w $ccanvas.a" );
 cmd( "newtop $w \"New Labels\" { set choice 2 } $ccanvas" );
 cmd( "wm geometry $w +$LX+$LY" );
 
 cmd( "frame $w.l" );
 cmd( "label $w.l.t -text \"New label\"" );
 cmd( "entry $w.l.e -textvariable itext -width 30" );
 cmd( "pack $w.l.t $w.l.e" );
 cmd( "pack $w.l -padx 5 -pady 5" );

 cmd( "okcancel $w b { set choice 1 } { set choice 2 }" );
 
 cmd( "bind $w.l.e <Return> { $w.b.ok invoke}" );
 
 cmd( "showtop $w current" );
 cmd( "focus $w.l.e" );
 cmd( "$w.l.e selection range 0 end" );
 
 *choice = 0;
 while ( ! *choice )
	Tcl_DoOneEvent( 0 );

 cmd( "destroytop $w" ); 
 
 if(*choice==1)
	cmd( "$ccanvas create text $hereX $hereY -text \"$itext\" -font $fontP -tags { text draw }" );

 *choice=0;
 goto there;

 
case 31:
/*
Edit line
*/
 cmd( "if { [ lsearch $type dots ] >= 0 } { \
			set dots true; \
			set iwidth \"\"; \
			set iarrow \"\"; \
			set idash \"\" \
		} { \
			set dots false; \
			set iwidth [ $ccanvas itemcget $cline -width ]; \
			set iarrow [ $ccanvas itemcget $cline -arrow ]; \
			set idash [ $ccanvas itemcget $cline -dash ] \
		}" );
 cmd( "set icolor [$ccanvas itemcget $cline -fill ]" );
 cmd( "set widthall 0" );
 cmd( "set colorall 0" );
 
 cmd( "set w $ccanvas.a" );
 cmd( "newtop $w \"Edit Line\" { set choice 2 } $ccanvas" );
 cmd( "wm geometry $w +$LX+$LY" );

 cmd( "frame $w.l" );
 cmd( "label $w.l.t -text \"Width\"" );
 cmd( "entry $w.l.e -textvariable iwidth -width 5 -justify center -state disabled -validate focusout -vcmd { if [ string is double %%P ] { set iwidth %%P; return 1 } { %%W delete 0 end; %%W insert 0 $iwidth; return 0 } } -invcmd { bell }" );
 cmd( "pack $w.l.t $w.l.e -padx 2 -side left" );
 
 cmd( "frame $w.c" );
 cmd( "label $w.c.l -text \"Color\"" );
 cmd( "button $w.c.color -width 5 -text Set -foreground white -background $icolor -command { set app [ tk_chooseColor -parent $w -title \"Line Color\" -initialcolor $icolor ]; if { $app != \"\" } { set icolor $app }; $w.c.color configure -background $icolor }" );
 cmd( "label $w.c.t -text \"   Dash pattern\"" );
 cmd( "ttk::combobox $w.c.e -textvariable idash -values [ list \"\" \". \" \"- \" \"-.\" \"-..\" ] -width 3 -justify center -state disabled" ); 
 cmd( "pack $w.c.l $w.c.color $w.c.t $w.c.e -padx 2 -side left" );
 
 cmd( "frame $w.d" );
 cmd( "label $w.d.t -text \"Line-end arrow(s)\"" );
 cmd( "ttk::combobox $w.d.e -textvariable iarrow -values [ list none first last both ] -width 7 -state disabled" );
 cmd( "pack $w.d.t $w.d.e -padx 2 -side left" );
 
 cmd( "frame $w.fall" );
 cmd( "checkbutton $w.fall.font -text \"Apply width to all line items\" -variable widthall -state disabled" );
 cmd( "checkbutton $w.fall.color -text \"Apply color to all line items\" -variable colorall" );
 cmd( "pack $w.fall.font $w.fall.color" );

 cmd( "pack $w.l $w.c $w.d $w.fall -padx 5 -pady 5" );

 cmd( "okXcancel $w b Delete { set iwidth 0; set choice 1 } { set choice 1 } { set choice 2 }" );
 
 cmd( "bind $w.l.e <Return> { $w.b.ok invoke }" );
 cmd( "bind $w.c.e <Return> { $w.b.ok invoke }" );
 cmd( "bind $w.d.e <Return> { $w.b.ok invoke }" );
 
 cmd( "showtop $w current" );
 cmd( "focus $w.l.e" );
 cmd( "$w.l.e selection range 0 end" );
 
// enable most options for non-dotted lines
cmd( "if { ! $dots } { $w.l.e  configure -state normal; $w.c.e  configure -state normal; $w.fall.font  configure -state normal }");
// enable dashes & arrows for drawing lines only
cmd( "if $draw { $w.d.e  configure -state normal }");

 *choice = 0;
 while ( ! *choice )
	Tcl_DoOneEvent( 0 );

 if(*choice==1)
 {
  cmd( "if { $iwidth==0 } { $ccanvas delete $cline; set choice 0 } { if $dots { set choice 2 } }" );
  if(*choice==1)	// regular line made of lines & dots
  {// avoid changing the dots parts in regular lines
   cmd( "$ccanvas dtag selected" );
   cmd( "$ccanvas addtag selected withtag \"$cline&&line\"" );
   cmd( "$ccanvas itemconf selected -width $iwidth" );
   cmd( "$ccanvas itemconf selected -dash \"$idash\"" );
   cmd( "$ccanvas itemconf selected -arrow $iarrow" );
   cmd( "$ccanvas dtag selected" );
   
   cmd( "$ccanvas itemconf $cline -fill $icolor" );  
  }
  if(*choice==2)	// line of dots
   cmd( "$ccanvas itemconf $cline -fill $icolor" );  

  cmd( "set choice $widthall" );
  if(*choice==1)
   cmd( "$ccanvas itemconf line -width $iwidth" );
  cmd( "set choice $colorall" );
  if(*choice==1)
   cmd( "$ccanvas itemconf line -fill $icolor" );
 } 
 
 cmd( "destroytop $w" ); 

 *choice=0;
 goto there;


case 28: 
/*
Insert a line item
*/
 cmd( "set choice $ncanvas" );
 i = *choice;
 
 cmd( "bind $ccanvas <B1-Motion> { \
		set ccanvas .da.f.new%d.f.plots; \
		if [ info exists cl ] { \
			$ccanvas delete $cl\
		}; \
		set ax [ $ccanvas canvasx %%x ]; \
		set ay [ $ccanvas canvasy %%y ]; \
		set cl [ $ccanvas create line $hereX $hereY $ax $ay -tags { line draw selected } ] \
	}", i );

 cmd( "bind $ccanvas <Shift-B1-Motion> { \
		set ccanvas .da.f.new%d.f.plots; \
		if [ info exists cl ] { \
			$ccanvas delete $cl\
		}; \
		set ax [ $ccanvas canvasx %%x ]; \
		set ay [ $ccanvas canvasy %%y ]; \
		if { [ expr abs( $ax - $hereX ) ] > [ expr abs( $ay - $hereY ) ] } { \
			set ay $hereY \
		} { \
			set ax $hereX \
		}; \
		set cl [ $ccanvas create line $hereX $hereY $ax $ay -tags { line draw selected } ] \
	}", i );
	
 cmd( "bind $ccanvas <ButtonRelease-1> { \
		set ccanvas .da.f.new%d.f.plots; \
		$ccanvas dtag selected; \
		bind $ccanvas <B1-Motion> {\
			set ccanvas .da.f.new%d.f.plots; \
			if $moving { \
				$ccanvas move current [ expr [ $ccanvas canvasx %%%%x ] - $hereX ] \
					[ expr [ $ccanvas canvasy %%%%y ] - $hereY ]; \
				set hereX [ $ccanvas canvasx %%%%x ]; \
				set hereY [ $ccanvas canvasy %%%%y ] \
			} \
		}; \
		bind $ccanvas <Shift-B1-Motion> { }; \
		bind $ccanvas <ButtonRelease-1> { set moving false }; \
		if [ info exists cl ] { \
			$ccanvas bind $cl <1> \" \
				set ccanvas .da.f.new%d.f.plots; \
				$ccanvas dtag selected; \
				$ccanvas addtag selected withtag $cl; \
				$ccanvas raise selected; \
				set hereX %%%%x; \
				set hereY %%%%y \
			\"; \
			$ccanvas bind $cl <ButtonRelease-1> { \
				.da.f.new%d.f.plots dtag selected \
			}; \
			$ccanvas bind $cl <B1-Motion> { \
				set ax %%%%x; \
				set ay %%%%y; \
				.da.f.new%d.f.plots move selected [ expr $ax - $hereX ] [ expr $ay - $hereY ]; \
				set hereX $ax; \
				set hereY $ay \
			}; \
			unset cl \
		} \
	}", i, i, i, i, i );
		
 *choice=0;
 goto there;


default:
 *choice=0;
 goto there;
}

}

/***************************************************
 PLOT_TSERIES
 ****************************************************/
void plot_tseries(int *choice)
{

char *app;
char **str, **tag;
int idseries;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool y2on, stopErr = false;

int i, j, k, *start, *end, done, doney2;
double **data,**logdata;

if(nv>1000)
 {
  cmd( "set answer [tk_messageBox -parent .da -type okcancel -title \"Too Many Series\" -icon warning -default ok -message \"You selected too many ($nv) series to plot\" -detail \"So many series may cause a crash of the Lsd model program, with the loss of all data.\nIf you continue the system may become slow, please be patient.\nPress 'Ok' to continue anyway.\"]" );
  cmd( "if {[string compare $answer ok] == 0} {set choice 1 } {set choice 2}" );
  if(*choice==2)
   return;
 }
 
if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	*choice = 2;
	return;
}

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

// prepare data from selected series
for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  
  // get series data and take logs if necessary
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
    data[ i ] = vs[ idseries ].data;
    if(data[i]==NULL)
      plog("\nError: invalid data\n");
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
  }
 }

// handle case selection
if(autom_x||min_c>=max_c)
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

// handle 2nd y-axis scale
cmd( "set choice $y2" );
y2on = *choice;

if ( y2on )
{
  if ( num_y2 > nv || num_y2 < 2 )
  {
   num_y2 = nv + 1;
   y2on = false;
  }
}
else
 num_y2 = nv + 1; 
 
// auto-find minimums and maximums
if ( miny>=maxy )
	autom = true;
if ( autom )
{
miny2=miny;
maxy2=maxy;
for(done=doney2=0, i=0; i<nv; i++)
 {
  if ( i < num_y2 - 1 )
  {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];
   }
  }
  else
  {
  for(j=min_c; j<=max_c; j++)
   {
    if(doney2==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny2=maxy2=data[i][j];
      doney2=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny2 )		// ignore NaNs
     miny2=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy2 )		// ignore NaNs
     maxy2=data[i][j];
   }
  }
 }
} //End for finding min-max

//To avoid divisions by zero for constant values
if(miny==maxy)
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

cmd( "write_disabled .da.f.h.v.sc.min.min $miny" );
cmd( "write_disabled .da.f.h.v.sc.max.max $maxy" );
cmd( "write_disabled .da.f.h.v.ft.from.mnc $minc" );
cmd( "write_disabled .da.f.h.v.ft.to.mxc $maxc" );

// plot all series
plot( 0, nv, data, start, end, str, tag, choice );	// standard Lsd TS plot (type=0)

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
PLOT_CROSS
****************************************************/
void plot_cross(int *choice)
{

int idseries;

char *app;
char **str;
char **tag;
int *start, *end, new_nv;
int *erase;
FILE *f;
int i, j, *pos, k, nt, *list_times, h,  first;
double **val, **data, **logdata;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

Tcl_LinkVar(inter, "nt", (char *) &nt, TCL_LINK_INT);
cmd( "if [ info exists num_t ] { set nt $num_t } { set nt \"-1\" }" );
Tcl_UnlinkVar(inter, "nt");

if(nv<2 || nt<=0)
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place at least two series in the Series Selected listbox and select at least one one time step (case).\"" );
	*choice = 2;
	return;
}

//Sets the list of cases to plot
list_times=new int[nt];
Tcl_LinkVar(inter, "k", (char *) &k, TCL_LINK_INT);
for(i=0; i<nt; i++)
 {
  cmd( "set k [lindex $list_times %d]", i );
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
data=new double *[nv];
logdata=new double *[nv];

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

// prepare data from selected series
for(i=0, new_nv=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  
  // check if series has data for all CS selected cases
  for(k=0, erase[i]=0; k<nt; k++)
    if(list_times[k]<start[i] || list_times[k]>end[i])
        {
		 erase[i]=1;
         break;
        }
	
  // get series data and take logs if necessary
  if(erase[i]==0)
   {
    data[ i ] = vs[ idseries ].data;
    if(data[i]==NULL)
      plog("\nError: invalid data\n");
   
    if(logs)			// apply log to the values to show "log scale"
    {
	  logdata[new_nv]=new double[end[i]+1];	// create space for the logged values
      for(j=start[i];j<=end[i];j++)		// log everything possible
	  if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	  else
	  {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	  }
	  data[i]=logdata[new_nv];				// replace the data series
    }
    val[new_nv]=new double[nt];
    new_nv++;
   }
  else
   data[i]=NULL;			// discard series not in all selected cases
 }
 
 // organize useful/valid data in 'val' matrix and find max/mins
 if ( miny>=maxy )
	 autom = true;
 for(j=0, first=1, i=0; j<nv; j++)
  {
	for(k=0; k<nt; k++)
	{
	 if(erase[j]==0 && is_finite(data[j][list_times[k]]))		// ignore NaNs
	  {
		val[i][k]=data[j][list_times[k]];
		
		// auto-find minimums and maximums
		if ( autom )
		{

			if(first)  //The first value is assigned to both min and max
			{
				miny=maxy=val[i][k];
				first=false;
			}
			if(miny>val[i][k])
				miny=val[i][k];
			if(maxy<val[i][k])
				maxy=val[i][k];
		}
     }
	 else
	 {
		 if ( ! erase[ j ] )	// mark NANs
			val[ i ][ k ] = NAN;
	 }//End if of erase[]

	} //end for over times
    if(erase[j]==0)
     i++;
  } //End for (j on num_var)

//To avoid divisions by zero for constant values
if(miny==maxy)
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
 
cmd( "write_disabled .da.f.h.v.sc.min.min $miny" );
cmd( "write_disabled .da.f.h.v.sc.max.max $maxy" );

// sort series if required
for ( k = 0; k < nt; ++k )		// find index to time reference
	if ( list_times[ k ] == res )
		break;

if ( k < nt )
{
	if(dir==-1)
	  sort_cs_desc(str, tag, val, new_nv, nt, k);
	if(dir==1)
	  sort_cs_asc(str, tag, val, new_nv, nt, k);
}

// plot all series
plot( 1, new_nv, val, list_times, &nt, str, tag, choice );	// standard Lsd CS plot (type=1)

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
delete[] list_times;
delete[] str;
delete[] tag;
delete[] pos;
delete[] val;
delete[] start;
delete[] end;
delete[] erase;
delete[] logdata;
delete[] data;
}


/***************************************************
SET_CS_DATA
****************************************************/
void set_cs_data(int *choice)
{

if ( nv < 2 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place two or more series in the Series Selected listbox.\"" );
	*choice = 2;
	return;
}
	
Tcl_LinkVar(inter, "res", (char *) &res, TCL_LINK_INT);
Tcl_LinkVar(inter, "dir", (char *) &dir, TCL_LINK_INT);
cmd( "set bidi $maxc" );
cmd( "set res $maxc" );
cmd( "set dir 0" );
cmd( "set list_times [ list ]" );
cmd( "unset -nocomplain sfrom sto sskip" );

cmd( "set p .da.s" );
cmd( "newtop $p \"Cross Section Time Steps\" { set choice 2 } .da" );

cmd( "frame $p.u" );

cmd( "frame $p.u.i" );

cmd( "frame $p.u.i.e" );
cmd( "label $p.u.i.e.l -text \"Time step to add\"" );
cmd( "entry $p.u.i.e.e -width 10 -validate focusout -vcmd { if { [ string is integer %%P ] && %%P >=0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( "$p.u.i.e.e insert 0 $bidi" );
cmd( "pack $p.u.i.e.l $p.u.i.e.e" );
 
cmd( "frame $p.u.i.lb" );
cmd( "label $p.u.i.lb.l -text \"Selected time steps\"" );

cmd( "frame $p.u.i.lb.lb" );
cmd( "scrollbar $p.u.i.lb.lb.v_scroll -command \".da.s.u.i.lb.lb.lb yview\"" );
cmd( "listbox $p.u.i.lb.lb.lb -listvariable list_times -width 8 -height 7 -justify center -selectmode extended -yscroll \".da.s.u.i.lb.lb.v_scroll set\"" );
cmd( "mouse_wheel $p.u.i.lb.lb.lb" );
cmd( "pack $p.u.i.lb.lb.lb $p.u.i.lb.lb.v_scroll -side left -fill y" );

cmd( "pack $p.u.i.lb.l $p.u.i.lb.lb" );

cmd( "pack $p.u.i.e $p.u.i.lb -padx 5 -pady 5" );

cmd( "frame $p.u.s" );
cmd( "label $p.u.s.l -text \"\nTime series order\"" );
cmd( "pack $p.u.s.l" );

cmd( "frame $p.u.s.b -relief groove -bd 2" );
cmd( "radiobutton $p.u.s.b.nosort -width -9 -text \"As selected\" -variable dir -value 0 -command { .da.s.u.s.r.e configure -state disabled; .da.s.u.i.e.e selection range 0 end; focus .da.s.u.i.e.e }" );
cmd( "radiobutton $p.u.s.b.up -text \"Ascending order\" -variable dir -value 1 -command { set sel [ .da.s.u.i.lb.lb.lb curselection ]; if { [ llength $sel ] == 1 } { set res [ lindex $list_times $sel ] } { set res $maxc }; .da.s.u.s.r.e configure -state normal; .da.s.u.s.r.e delete 0 end; .da.s.u.s.r.e insert 0 $res; .da.s.u.s.r.e selection range 0 end; focus .da.s.u.s.r.e }" );
cmd( "radiobutton $p.u.s.b.down -text \"Descending order\" -variable dir -value \"-1\" -command { set sel [ .da.s.u.i.lb.lb.lb curselection ]; if { [ llength $sel ] == 1 } { set res [ lindex $list_times $sel ] } { set res $maxc }; .da.s.u.s.r.e configure -state normal; .da.s.u.s.r.e delete 0 end; .da.s.u.s.r.e insert 0 $res; .da.s.u.s.r.e selection range 0 end; focus .da.s.u.s.r.e }" );
cmd( "pack $p.u.s.b.nosort $p.u.s.b.up $p.u.s.b.down -padx 5 -anchor w" );
cmd( "pack $p.u.s.b -padx 5" );

cmd( "frame $p.u.s.r" );
cmd( "label $p.u.s.r.l -text \"Time step reference\nfor series sorting\"" );
cmd( "entry $p.u.s.r.e -width 10 -validate focusout -vcmd { if { [ string is integer %%P ] && [ lsearch $list_times %%P ] >=0 } { set res %%P; return 1 } { %%W delete 0 end; %%W insert 0 $res; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_disabled $p.u.s.r.e $res" );
cmd( "pack $p.u.s.r.l $p.u.s.r.e" );
cmd( "pack $p.u.s.r -pady 10" );

cmd( "pack $p.u.i $p.u.s -side left -anchor n" );
cmd( "pack $p.u -pady 5");

cmd( "bind $p.u.i.e.e <KeyPress-Return> { .da.s.fb.r1.x invoke }" );

cmd( "bind $p.u.i.e.e <Control-f> { set sfrom [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }; bind $p.u.i.e.e <Control-F> { set sfrom [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }" );
cmd( "bind $p.u.i.e.e <Control-t> { set sto [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }; bind $p.u.i.e.e <Control-T> { set sto [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }" );
cmd( "bind $p.u.i.e.e <Control-s> { set sskip [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }; bind $p.u.i.e.e <Control-S> { set sskip [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }" );
cmd( "bind $p.u.i.e.e <Control-z> { if { [ info exists sfrom ] && [ info exists sto ] && [ string is integer $sfrom ] && [ string is integer $sto ] && [ expr $sto - $sfrom ] > 0 } { if { ! [ info exists sskip ] } { set sskip 1 }; for { set x $sfrom } { $x <= $sto && $x <= $maxc } { incr x $sskip } { .da.s.u.i.lb.lb.lb insert end $x } }; .da.s.u.i.e.e selection range 0 end }; bind $p.u.i.e.e <Control-Z> { if { [ info exists sfrom ] && [ info exists sto ] && [ string is integer $sfrom ] && [ string is integer $sto ] && [ expr $sto - $sfrom ] > 0 } { if { ! [ info exists sskip ] } { set sskip 1 }; for { set x $sfrom } { $x <= $sto && $x <= $maxc } { incr x $sskip } { .da.s.u.i.lb.lb.lb insert end $x } }; .da.s.u.i.e.e selection range 0 end }" );

cmd( "XYokhelpcancel $p fb Add Delete { set a [ .da.s.u.i.e.e get ]; if { [ lsearch $list_times $a ] < 0 && [ string is integer $a ] && $a >= 0 && $a <= $maxc } { .da.s.u.i.lb.lb.lb insert end $a; focus .da.s.u.i.e.e; .da.s.u.i.e.e selection range 0 end; .da.s.u.i.lb.lb.lb selection set end } { bell } } { set sel [ .da.s.u.i.lb.lb.lb curselection ]; if { [ llength $sel ] > 0 } { .da.s.u.i.lb.lb.lb delete [ lindex $sel 0 ] [ lindex $sel [ expr [ llength $sel ] - 1 ] ]; .da.s.u.i.e.e selection range 0 end; focus .da.s.u.i.e.e } } { set choice 1 } { LsdHelp mdatares.html#crosssection } { set choice 2 }" );

cmd( "showtop $p centerW no no yes 0 0 .da.s.fb.r1.add" );
cmd( ".da.s.u.i.e.e selection range 0 end; focus .da.s.u.i.e.e" );

*choice = 0;
while ( ! *choice )
	Tcl_DoOneEvent( 0 );

if ( *choice == 2 )
	goto end;

cmd( "if { [.da.s.u.i.lb.lb.lb size] == 0 } { tk_messageBox -parent .da.s -type ok -title Error -icon error -message \"No time step selected\" -detail \"At least one case/time step must be selected. Please try again.\"; set choice 2 }" );

if ( *choice == 2 )
	goto end;

cmd( "set res [ $p.u.s.r.e get ]" );
cmd( "if { $dir != 0 && [ lsearch $list_times $res ] < 0 } { tk_messageBox -parent .da.s -type ok -title Warning -icon warning -message \"Invalid time step reference selected\" -detail \"The selected time step reference is not one of the selected for the cross-section(s), no sorting will be performed.\"; set dir 0 }" );
cmd( "set num_t [ llength $list_times ]" );
*choice = 0;

end:
cmd( "destroytop .da.s" );
Tcl_UnlinkVar(inter, "res");
Tcl_UnlinkVar(inter, "dir");

return;
}


/***************************************************
SORT_CS_DESC
****************************************************/
void sort_cs_desc(char **s,char **t, double **v, int nv, int nt, int c)
{
int i, j, h;
double dapp;
char sapp[MAX_ELEM_LENGTH];

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
char sapp[MAX_ELEM_LENGTH];

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
     set_lab_tit(cv);
     cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, cv->start, cv->end, *num_v );
     if(cv->end>*num_c)
       *num_c=cv->end;
     *num_v+=1;
    }

for(cb=r->b; cb!=NULL; cb=cb->next)
 {cur=cb->head;
 if(cur->to_compute==1)
   {
   for(cur=cb->head; cur!=NULL; cur=cur->next)
     insert_labels_mem(cur, num_v, num_c);
   }  
 }
 
if(r->up==NULL)
 for(cv=cemetery; cv!=NULL; cv=cv->next)
  {  
	 cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d\"", cv->label, cv->lab_tit, cv->start, cv->end, *num_v );
     if(cv->end>*num_c)
       *num_c=cv->end;
     *num_v+=1;
  }
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
 {
 cur=cb->head;
 if(cur->to_compute==1)
   for(cur=cb->head; cur!=NULL; cur=cur->next)
      insert_store_mem(cur, num_v);
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
void insert_data_file( bool gz, int *num_v, int *num_c )
{
FILE *f;
#ifdef LIBZ
gzFile fz;
#endif
char ch, label[MAX_ELEM_LENGTH], tag[MAX_ELEM_LENGTH], app_str[20], *tok, *linbuf;
int i, j, new_v, new_c;
bool header = false;
long linsiz = 1;
store *app;

if ( ! gz )
	f = fopen( filename, "rt" );
else
{
#ifdef LIBZ
	fz = gzopen( filename, "rt" );
#endif
}

new_v=0;
if(*num_v==0)
  plog( "\nResult data from file %s:\n", "", filename);
else
  plog( "\nAdditional data file number %d.\nResult data from file %s:\n", "", file_counter, filename);  

if ( ! gz )
	ch = ( char ) fgetc( f );
else
{
#ifdef LIBZ
	ch = ( char ) gzgetc( fz );
#endif
}

while( ch != '\n' )
{
	if ( ! gz )
		ch = ( char ) fgetc( f );
	else
	{
#ifdef LIBZ
		ch = ( char ) gzgetc( fz );
#endif
	}
	if ( ch == '\t' )
	   new_v += 1;
   
	if ( ch == '(' )		// check for header-less .tot files
		header = true;
	
	linsiz++;				// count line size
}

if ( ! header )
{
	plog( "\nError: invalid header, aborting file load.\nCheck if '.tot' files where created with the -g option.\n" );
	goto end;
}

if ( ! gz )
	fclose( f );
else
{
#ifdef LIBZ
	gzclose( fz );
#endif
}

plog( "- %d Variables\n", "",  new_v );

cmd( "update" );

if ( ! gz )
	f = fopen( filename, "rt" );
else
{
#ifdef LIBZ
	fz = gzopen( filename, "rt" );
#endif
}

new_c=-1;
while( ch != EOF )
{
	if ( ! gz )
		ch = ( char ) fgetc( f );
	else
	{
#ifdef LIBZ
		ch = ( char ) gzgetc( fz );
#endif
	}
	if ( ch == '\n' )
		new_c += 1;
}

if ( ! gz )
	fclose( f );
else
{
#ifdef LIBZ
	gzclose( fz );
#endif
}

plog( "- %d Cases\nLoading...", "", new_c );

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

if ( ! gz )
	f = fopen( filename, "rt" );
else
{
#ifdef LIBZ
	fz = gzopen( filename, "rt" );
#endif
}

if(*num_v>-1)
 sprintf(app_str, " (file=%d)",file_counter);
else
 strcpy(app_str, "");

linsiz = max( linsiz, new_v * ( DBL_DIG + 4 ) ) + 1;
linbuf = new char[ linsiz ];
if ( linbuf == NULL )
{
	plog( "\nError: not enough memory or invalid format, aborting file load.\n" );
	goto end;
}

// read header line
if ( ! gz )
	fgets( linbuf, linsiz, f );	// buffers one entire line
else
{
#ifdef LIBZ
	gzgets( fz, linbuf, linsiz );
#endif
}

tok = strtok( linbuf , "\t" ); // prepares for parsing and get first one
for ( i = *num_v; i < new_v + *num_v; ++i )
{
  if ( tok == NULL )
  {
	  plog( "\nError: invalid header, aborting file load.\n");
	  goto end;
  }
  
  sscanf( tok, "%s %s (%d %d)", vs[i].label, vs[i].tag, &( vs[i].start ), &( vs[i].end ) );	
  strcat(vs[i].label, "F");
  sprintf(msg, "%d_%s", file_counter, vs[i].tag);
  strcpy(vs[i].tag, msg);
  vs[i].rank=i;

 if(vs[i].start!=-1)
   cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d %s\"", vs[i].label, vs[i].tag, vs[i].start, vs[i].end, i, app_str );
 else
   {
	cmd( ".da.vars.lb.v insert end \"%s %s (0 - %d) # %d %s\"", vs[i].label, vs[i].tag, new_c-1, i, app_str );
    vs[i].start=0;
    vs[i].end=new_c-1;
   }
 
 cmd( "lappend DaModElem %s", vs[i].label );

 vs[i].data=new double[new_c+2];
 
 tok=strtok( NULL, "\t" );		// get next token, if any
}

// read data lines
for(j=0; j<new_c; j++)
{
 if ( ! gz )
	fgets( linbuf, linsiz, f );	// buffers one entire line
 else
 {
#ifdef LIBZ
	gzgets( fz, linbuf, linsiz );
#endif
 }
 
 tok = strtok( linbuf , "\t" ); // prepares for parsing and get first one
 for ( i = *num_v; i < new_v + *num_v; ++i )
 {
	if ( tok == NULL )
	{
	  plog( "\nError: invalid data, aborting file load.\n" );
	  *num_c += ( j > 0 ? j - 1 : 0 ) > *num_c ? ( j > 0 ? j - 1 : 0 ) : 0;
	  goto end;
	}
  
	if ( ! strcmp( tok, nonavail ) )	// it's a non-available observation
		vs[ i ].data[ j ] = NAN;
	else
		sscanf( tok,"%lf", &( vs[ i ].data[ j ] ) );
	
	tok=strtok( NULL, "\t" );		// get next token, if any
 }
}

*num_v+=new_v;
new_c--;
if(new_c > *num_c)
 *num_c=new_c;
if(new_c > max_c)
 max_c=new_c; 

plog( " Done\n" );

end:

if ( ! gz )
	fclose( f );
else
{
#ifdef LIBZ
	gzclose( fz );
#endif
}

}


/************************
STATISTICS
************************/
void statistics(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[50], longmsg[300];
int i, j, *start, *end;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;
double **data,**logdata, av, var, num, ymin, ymax, sig;

if ( nv == 0 )			// no variables selected
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

// prepare data from selected series
for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  
  // get series data and take logs if necessary
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
    data[ i ] = vs[ idseries ].data; 
    if(data[i]==NULL)
      plog("\nError: invalid data\n");
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
  }
 }

if(logs)
 cmd( ".log.text.text.internal insert end \"\n\nTime series descriptive statistics (in log)\n\" tabel" );
else
 cmd( ".log.text.text.internal insert end \"\n\nTime series descriptive statistics\n\" tabel" );

sprintf(str1, "%d Cases", max_c-min_c+1);
sprintf(longmsg, "%-20s\tAverage\tStd.Dev.\tVar.\tMin\tMax\n", str1);
cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );

for(i=0; i<nv; i++)
{
 ymin = DBL_MAX;
 ymax = - DBL_MAX;

 for(av=var=num=0, j=min_c; j<=max_c; j++)
  {
  if(j>=start[i] && j<=end[i] && is_finite(data[i][j]))		// ignore NaNs
  {
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
 cmd( ".log.text.text.internal insert end \"%s\" tabel", str1 );


 sprintf(longmsg, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", pdigits, av, pdigits, sig, pdigits, var, pdigits, ymin, pdigits, ymax);
 cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );


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
char str1[50], longmsg[300];
int i, j, *start, *end, nt, *list_times, h, k;
double **data,**logdata, av, var, num, ymin, ymax, sig;
bool first;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

Tcl_LinkVar(inter, "nt", (char *) &nt, TCL_LINK_INT);
cmd( "if [ info exists num_t ] { set nt $num_t } { set nt \"-1\" }" );
Tcl_UnlinkVar(inter, "nt");

if(nv<2 || nt<=0)
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place at least two series in the Series Selected listbox and select at least one time step (case).\"" );
	*choice = 2;
	return;
}

//Sets the list of cases to plot
list_times=new int[nt];
cmd( "set k 0" );
Tcl_LinkVar(inter, "k", (char *) &k, TCL_LINK_INT);
for(i=0; i<nt; i++)    
 {
  cmd( "set k [lindex $list_times %d]", i );
  list_times[i]=k;
 }
Tcl_UnlinkVar(inter, "k");

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

// prepare data from selected series
for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  data[ i ] = vs[ idseries ].data;
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
 }

if(logs)
 cmd( ".log.text.text.internal insert end \"\n\nCross-section descriptive statistics (in log)\n\" tabel" );
else
 cmd( ".log.text.text.internal insert end \"\n\nCross-section descriptive statistics\n\" tabel" );

sprintf(str1, "%d Variables",nv);
sprintf(longmsg, "%-20s\tAverage\tStd.Dev.\tVar.\tMin\tMax\n", str1);
cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );

for(j=0; j<nt; j++)
{h=list_times[j];
 first=true;
 for(av=var=num=0, i=0; i<nv; i++)
  {
  if(h>=start[i] && h<=end[i] && is_finite(data[i][h]))		// ignore NaNs
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
 cmd( ".log.text.text.internal insert end \"%s\" tabel", str1 );

 sprintf(longmsg, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", pdigits, av, pdigits, sig, pdigits, var, pdigits, ymin, pdigits, ymax);
 cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );
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


/************************
SORT_ON_END
************************/
void sort_on_end(store *app)
{
	qsort((void *)app, num_var, sizeof(vs[0]), sort_labels_down);
}


/************************
SORT_LABELS_DOWN
************************/
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

// convert labels to lowercase for comparison
int a_sz = strlen( ( ( store * ) a )->label );
int b_sz = strlen( ( ( store * ) b )->label );
char *a_str = new char [ a_sz + 1 ];
char *b_str = new char [ b_sz + 1 ];
strcpy( a_str, ( ( store * ) a )->label );
strcpy( b_str, ( ( store * ) b )->label );
for ( int i = 0; i < a_sz; ++i )
	a_str[ i ] = tolower( a_str[ i ] );
for ( int i = 0; i < b_sz; ++i )
	b_str[ i ] = tolower( b_str[ i ] );
// make names started with a underscore go to the end
if ( a_str[ 0 ] == '_' )
	a_str[ 0 ] = '~';
if ( b_str[ 0 ] == '_' )
	b_str[ 0 ] = '~';

diff = strcmp( a_str, b_str );

delete [] a_str;
delete [] b_str;

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
Draws the XY plots, with the first series as X and the others as Y's
****************************************************/
void plot_gnu(int *choice)
{
char *app;
char **str, **tag;
char str1[50], str2[100], str3[10], dirname[MAX_PATH_LENGTH];
FILE *f, *f2;
double **data,**logdata;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

int i, nanv=0, j, k, *start, *end, done, box;
int idseries, ndim, gridd;

if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	*choice = 2;
	return;
}

if(nv>2)
 {
 cmd( "newtop .da.s \"XY Plot Options\" { set choice 2 } .da" );

 cmd( "frame .da.s.t" );
 cmd( "label .da.s.t.l -text \"Plot type\"" );

 cmd( "frame .da.s.t.d -relief groove -bd 2" );
 cmd( "if { ! [ info exists ndim ] } { set ndim 2 }" );
 cmd( "radiobutton .da.s.t.d.2d -text \"2D plot\" -variable ndim -value 2 -command { .da.s.d.o.a configure -state disabled; .da.s.d.o.c configure -state disabled; .da.s.d.o.b configure -state disabled; .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set box 0; set gridd 0; set pm3d 0 }" );
 cmd( "radiobutton .da.s.t.d.3d -text \"3D plot \" -variable ndim -value 3 -command { .da.s.d.o.a configure -state normal; .da.s.d.o.c configure -state normal; .da.s.d.o.b configure -state normal; .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );
 cmd( "pack .da.s.t.d.2d .da.s.t.d.3d -anchor w" );

 cmd( "pack .da.s.t.l .da.s.t.d" );

 cmd( "frame .da.s.d" );
 cmd( "label .da.s.d.l -text \"3D options\"" );

 cmd( "frame .da.s.d.o -relief groove -bd 2" );
 cmd( "if { ! [ info exists box ] } { set box 0 }" );
 cmd( "radiobutton .da.s.d.o.a -text \"Use 1st and 2nd vars. as plane\" -variable box -value 0" );
 cmd( "radiobutton .da.s.d.o.c -text \"Use time and 1st var. as plane\" -variable box -value 2" );
 cmd( "radiobutton .da.s.d.o.b -text \"Use time and rank as plane\" -variable box -value 1" );
 cmd( "pack .da.s.d.o.a .da.s.d.o.c .da.s.d.o.b -anchor w" );

 cmd( "pack .da.s.d.l .da.s.d.o" );

 cmd( "frame .da.s.o" );
 cmd( "checkbutton .da.s.o.g -text \"Use gridded data\" -variable gridd" );
 cmd( "checkbutton .da.s.o.p -text \"Draw palette-mapped surface\" -variable pm3d" );
 cmd( "pack .da.s.o.g .da.s.o.p" );

 cmd( "pack .da.s.t .da.s.d .da.s.o -padx 5 -pady 5" );
 
 cmd( "if { $ndim == 2 } { .da.s.d.o.a configure -state disabled; .da.s.d.o.c configure -state disabled; .da.s.d.o.b configure -state disabled; .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set box 0; set gridd 0; set pm3d 0 } { .da.s.d.o.a configure -state normal; .da.s.d.o.c configure -state normal; .da.s.d.o.b configure -state normal; .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );

 cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#3dTime } { set choice 2 }" );

 cmd( "showtop .da.s centerW" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "destroytop .da.s" );

if(*choice==2)
 return;

cmd( "set choice $ndim" );
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
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

// prepare data from selected series
for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  
  // get series data and take logs if necessary
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
    data[ i ] = vs[ idseries ].data;
    if(data[i]==NULL)
      plog("\nError: invalid data\n");

   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }
  else
   nanv++; 
 }

// handle case selection
if(autom_x||min_c>=max_c)
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

// auto-find minimums and maximums
if ( miny>=maxy )
	autom = true;
if ( autom )
{
for(done=0, i=1; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
 }
} //End for finding min-max
   
cmd( "set dirxy plotxy_%d", cur_plot );

cmd( "file mkdir $dirxy" );

getcwd(dirname, MAX_PATH_LENGTH-1);
sprintf(msg, "plotxy_%d", cur_plot);
chdir(msg);
f=fopen("data.gp","w");
fprintf(f,"#");
if(nv>2)
{
  cmd( "set choice $box" );
  box=*choice;
  cmd( "set choice $gridd" );
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
      fprintf(f,"%.*g\t", pdigits, data[i][j]);
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
       fprintf(f,"%d\t%.*g\t", i+1, pdigits, data[i][j]);
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
        fprintf(f,"%.*g\t", pdigits, data[i][j]);
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
       fprintf(f,"%d\t%d\t%.*g\n", j, i+1, pdigits, data[i][j]);
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

if(grid)
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
 sprintf(msg, "set yrange [%.*g:%.*g]\n", pdigits, miny, pdigits, maxy);
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
  strcpy(str1, ""); 

if(ndim==2)
{
if(allblack)
 sprintf(str3, " lt -1");
else
 strcpy(str3, "");
}
else
 {
  cmd( "set choice $gridd" );
 if(*choice==1)
  {
   app=(char *)Tcl_GetVar(inter, "gpdgrid3d",0);
   fprintf(f, "set dgrid3d %s\nset hidden3d\n", app);
   fprintf(f2, "set dgrid3d %s\nset hidden3d\n",app);
  }
 cmd( "set choice $pm3d" );
 if(*choice==1)
  {
   fprintf(f, "set pm3d\n");
   fprintf(f2, "set pm3d\n");
   sprintf(str1, "with pm3d ");
  }

  if(allblack)
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
  sprintf(msg,"plot 'data.gp' using 1:2 %s t \"%s_%s\"", str1, str[1], tag[1]);
  if(allblack)
   strcat(msg, str3);
  i=2;
 } 
else
 {

  if(box==0)
    {sprintf(msg,"splot 'data.gp' using 1:2:3 %s t \"%s_%s\"", str1, str[2], tag[2]); 
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
    sprintf(str2,", 'data.gp' using 1:%d %s t \"%s_%s\"",i+1, str1, str[i], tag[i]);
   else
    {if(box==0 )
      sprintf(str2,", 'data.gp' using 1:2:%d %s t \"%s_%s\"",i+1, str1, str[i], tag[i]); 
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

   if(strlen(str2)>0 && allblack)
     strcat(str2, str3);
   fprintf(f, "%s",str2);
   fprintf(f2, "%s",str2);
  }

fprintf(f, "\n");
fprintf(f2, "\n");

fprintf(f2, "pause -1 \"Close plot %d\"\n", cur_plot);

fclose(f);
fclose(f2);

cmd( "if { [ info exists pm3d ] } { set choice $pm3d } { set choice 0 }" );
if ( *choice != 0 )
 *choice = 2;	// require filled polygons, not supported by tkcanvas terminal
else
 *choice = gnu;

show_plot_gnu(cur_plot, choice, *choice, str, tag);

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


/*****************************************
PLOT_CS_XY
*****************************************/
void plot_cs_xy(int *choice)
{

int idseries;
char *app;
char **str, **tag;
char str1[5*MAX_ELEM_LENGTH], str2[5*MAX_ELEM_LENGTH], str3[10], dirname[MAX_PATH_LENGTH];
FILE *f, *f2;
double **data,**logdata, previous_row;
int i, j, k, *start, *end, done, color;
int time_sel, block_length, ndim;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

if ( nv < 2 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place at least two series in the Series Selected listbox.\"" );
	*choice = 2;
	return;
}

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

// prepare data from selected series
for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  
  // get series data and take logs if necessary
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
    data[ i ] = vs[ idseries ].data;
    if(data[i]==NULL)
      plog("\nError: invalid data\n");
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
  }
 }

// handle case selection
if(autom_x||min_c>=max_c)
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

// auto-find minimums and maximums
if ( miny>=maxy )
	autom = true;
if ( autom )
{
for(done=0, i=1; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];
   }
 }
} //End for finding min-max

cmd( "set bidi %d", end[0] );

cmd( "newtop .da.s \"XY Plot Options\" { set choice 2 } .da" );

cmd( "frame .da.s.i" );
cmd( "label .da.s.i.l -text \"Time step\"" );
cmd( "entry .da.s.i.e -width 10 -validate focusout -vcmd { if { [ string is integer %%P ] && %%P > 0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "pack .da.s.i.l .da.s.i.e" );

cmd( "frame .da.s.d" );
cmd( "label .da.s.d.l -text \"Type\"" );

cmd( "frame .da.s.d.r -relief groove -bd 2" );
cmd( "if { ! [ info exists ndim ] } { set ndim 2 }" );
cmd( "radiobutton .da.s.d.r.2d -text \"2D plot\" -variable ndim -value 2 -command  { .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set gridd 0; set pm3d 0 }" );
cmd( "radiobutton .da.s.d.r.3d -text \"3D plot\" -variable ndim -value 3 -command  { .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );
cmd( "pack .da.s.d.r.2d .da.s.d.r.3d -anchor w" );

cmd( "pack .da.s.d.l .da.s.d.r" );

cmd( "frame .da.s.o" );
cmd( "label .da.s.o.l -text \"3D options\"" );
cmd( "checkbutton .da.s.o.g -text \"Use gridded data\" -variable gridd -anchor w" );
cmd( "checkbutton .da.s.o.p -text \"Draw palette-mapped surface\" -variable pm3d -anchor w" );
cmd( "if { $ndim == 2 } { .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set gridd 0; set pm3d 0 } { .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );
cmd( "pack .da.s.o.l .da.s.o.g .da.s.o.p" );

cmd( "set choice $ndim" );
if(*choice==2)
 cmd( "set blength %d", (int)(nv/2) );
else
 cmd( "set blength %d", (int)(nv/3) ); 

cmd( "set numv 1" );
cmd( "frame .da.s.v" );
cmd( "label .da.s.v.l -text \"Number of dependent variables\"" );
cmd( "entry .da.s.v.e -width 10 -validate focusout -vcmd { if { [ string is integer %%P ] && %%P > 0 && %%P < %d } { set numv %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numv; return 0 } } -invcmd { bell } -justify center", nv );
cmd( ".da.s.v.e insert 0 $numv" ); 
cmd( "label .da.s.v.n -text \"Block length: $blength\"" );
cmd( "pack .da.s.v.l .da.s.v.e .da.s.v.n" );

cmd( "pack .da.s.i .da.s.d .da.s.o .da.s.v -padx 5 -pady 5" );

cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#3dCrossSection } { set choice 2 }" );

cmd( "bind .da.s.v.e <KeyRelease> {set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Block length: $blength\"}" );
cmd( "set nnvar %d", nv );
cmd( "bind .da.s.v.e <Return> {set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Block length: $blength\"}" );
cmd( "bind .da.s.v.e <Tab> {set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Block length: $blength\"}" );
cmd( "bind .da.s.d.r.2d <Return> {set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Block length: $blength\"}" );
cmd( "bind .da.s.d.r.2d <ButtonRelease-1> {set ndim 2; set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Block length: $blength\"}" );
cmd( "bind .da.s.d.r.3d <ButtonRelease-1> {set ndim 3; set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Block length: $blength\"}" );
cmd( "bind .da.s.d.r.2d <Down> {.da.s.d.r.3d invoke; focus .da.s.d.r.3d; set ndim 3; set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Block length: $blength\"}" );
cmd( "bind .da.s.d.r.3d <Up> {.da.s.d.r.2d invoke; focus .da.s.d.r.2d; set ndim 2; set blength [expr $nnvar / ($numv + $ndim-1)]; .da.s.v.n conf -text \"Block length: $blength\"}" );
cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( "bind .da.s.i.e <KeyPress-Return> {if {$ndim == 2} { focus .da.s.d.r.2d } {focus .da.s.d.r.3d}}" );
cmd( "bind .da.s.d.r.2d <KeyPress-Return> {.da.s.v.e selection range 0 end; focus .da.s.v.e}" );
cmd( "bind .da.s.d.r.3d <KeyPress-Return> {.da.s.v.e selection range 0 end; focus .da.s.v.e}" );
cmd( "bind .da.s.v.e <KeyPress-Return> {focus .da.s.b.ok}" );

cmd( "showtop .da.s centerW" );

cmd( "focus .da.s.i.e" );
cmd( ".da.s.i.e selection range 0 end" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "set numv [ .da.s.v.e get ]" ); 

cmd( "destroytop .da.s" );

if(*choice==2)
 goto end;

cmd( "set choice $ndim" );
ndim=*choice;

cmd( "set choice $bidi" );
time_sel=*choice;

cmd( "set blength [expr $nnvar / ($numv + $ndim-1)]" );
cmd( "set choice $blength" );
block_length=*choice;
*choice=0;

if ( block_length <= 0 || nv % block_length != 0 )
 {
  cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid block length\" -detail \"Block length should be an exact divisor of the number of variables. You may also try a 3D plot.\"" );
  *choice=2;
  goto end;
 }

//here we are
cmd( "set dirxy plotxy_%d", cur_plot );

cmd( "file mkdir $dirxy" );
getcwd(dirname, MAX_PATH_LENGTH-1);
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
     previous_row=data[j][end[j]];
   }
 }
else
 {
  if(data[j][time_sel]!=previous_row)
   {
    previous_row=data[j][time_sel];
   }
 }
 
 for(i=0; i<nv; i+=block_length)
  {
  if(start[i+j] == end[i+j])  
     fprintf(f,"%.*g\t", pdigits, data[i+j][end[i+j]]);
  else
   {
    if(start[i+j]<=max_c && end[i+j]>=min_c)
      fprintf(f,"%.*g\t", pdigits, data[i+j][time_sel]);
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

if(grid)
 {
  fprintf(f, "set grid\n");
  fprintf(f2, "set grid\n");
 }
if(line_point==2)
 { fprintf(f, "set pointsize %lf\n", point_size);
   fprintf(f2, "set pointsize %lf\n", point_size);
   if(ndim==2)
    strcpy(str2, "");
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

if(allblack)
 sprintf(str3, " lt -1");
else
  str3[0]=0;

if(ndim==3)
 {
 cmd( "set choice $gridd" );
 if(*choice==1)
  {
   app=(char *)Tcl_GetVar(inter, "gpdgrid3d",0);
   fprintf(f, "set dgrid3d %s\nset hidden3d\n", app);
   fprintf(f2, "set dgrid3d %s\nset hidden3d\n", app);
  }
 cmd( "set choice $pm3d" );
 if(*choice==1)
  {sprintf(str2, "with pm3d ");
   fprintf(f, "set pm3d\n");
   fprintf(f2, "set pm3d\n");
  }

 if(allblack)
  {
   strcpy(str3, "");
  fprintf(f, "set palette gray\n");
  fprintf(f2, "set palette gray\n");
  } 
 }
 
if(ndim==2)
 {
  sprintf(msg,"plot 'data.gp' using 1:2 %s t \"%s_%s(%d)\"",str2, str[block_length], tag[block_length], time_sel);
  if(allblack)
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
     if(allblack)
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

fprintf(f2, "pause -1 \"Close plot %d\"\n", cur_plot);
fclose(f);
fclose(f2);

cmd( "if { [ info exists pm3d ] } { set choice $pm3d } { set choice 0 }" );
if ( *choice != 0 )
 *choice = 2;	// require filled polygons, not supported by tkcanvas terminal
else
 *choice = gnu;

show_plot_gnu(cur_plot, choice, *choice, str, tag);
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
char str1[50], str2[100], str3[100], dirname[MAX_PATH_LENGTH];
FILE *f, *f2;
double **data,**logdata;

int i, j, k, *start, *end, done, nlags;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

if(nv!=1)
 {
 cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"One and only one series must be selected.\"" );
 *choice = 2;
 return;
 }

nv=1; //ridiculous, but I am recycling the code

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

// prepare data from selected series
for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  
  // get series data and take logs if necessary
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[ i ] = vs[ idseries ].data;
    if(data[i]==NULL)
      plog("\nError: invalid data\n");
   
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
   }
 }

// handle case selection
if(autom_x||min_c>=max_c)
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

// auto-find minimums and maximums
if ( miny>=maxy )
	autom = true;
if ( autom )
{
for(done=0, i=0; i<nv; i++)
 {
  for(j=min_c; j<=max_c; j++)
   {
    if(done==0 && start[i]<=j && end[i]>=j && is_finite(data[i][j]))		// ignore NaNs
     {miny=maxy=data[i][j];
      done=1;
     }
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]<miny )		// ignore NaNs
     miny=data[i][j];
    if(start[i]<=j && end[i]>=j && is_finite(data[i][j]) && data[i][j]>maxy )		// ignore NaNs
     maxy=data[i][j];

   }
 }
} //End for finding min-max

cmd( "newtop .da.s \"Lags Selection\" { set choice 2 } .da" );

cmd( "frame .da.s.i" );
cmd( "label .da.s.i.l -text \"Number of lags\"" );
cmd( "set bidi 1" );
cmd( "entry .da.s.i.e -width 10 -validate focusout -vcmd { if { [ string is integer %%P ] && %%P > 0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "pack .da.s.i.l .da.s.i.e" );

cmd( "if { ! [ info exists dia ] } { set dia 0 }" );
cmd( "checkbutton .da.s.arrow -text \"Plot 45\u00B0 diagonal\" -variable dia" );
cmd( "pack .da.s.i .da.s.arrow -padx 5 -pady 5" );

cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#plot } { set choice 2 }" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

cmd( "showtop .da.s centerW" );
cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "destroytop .da.s" );

if(*choice==2)
 goto end;
cmd( "set dirxy plotxy_%d", cur_plot );

cmd( "file mkdir $dirxy" );
getcwd(dirname, MAX_PATH_LENGTH-1);
sprintf(msg, "plotxy_%d",cur_plot);
chdir(msg);

cmd( "set choice $bidi" );
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

if(grid)
 {fprintf(f, "set grid\n");
  fprintf(f2, "set grid\n");
 }
if(line_point==2)
 {sprintf(str1,"set pointsize %lf\n", point_size);
  fprintf(f,"%s", str1);
  fprintf(f2,"%s", str1);
 }
sprintf(msg, "set yrange [%.*g:%.*g]\n", pdigits, miny, pdigits, maxy);
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);
sprintf(msg, "set xlabel \"%s_%s\"\n", str[0], tag[0]);
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

cmd( "set choice $dia" );
if(*choice==1)
 {
  fprintf(f,"set arrow from %.*g,%.*g to %.*g,%.*g lt -1\n", pdigits, miny, pdigits, miny, pdigits, maxy, pdigits, maxy);
  fprintf(f2,"set arrow from %.*g,%.*g to %.*g,%.*g lt -1\n", pdigits, miny, pdigits, miny, pdigits, maxy, pdigits, maxy);
 }
if(line_point==1)
 sprintf(str1, "smooth csplines");
else
 strcpy(str1, "");

if(allblack)
 sprintf(str3, " lt -1");
else
 str3[0]=0;

 sprintf(msg,"plot 'data.gp' using 1:2 %s t \"t+1\"", str1 );
 if(allblack)
  strcat(msg, str3);
for(i=2; i<=nlags; i++)
 if(start[0]<=max_c && end[0]>=min_c)
  {sprintf(str2,", 'data.gp' using 1:%d %s t \"t+%d\"",i+1, str1, i);
   strcat(msg,str2);
   if(allblack)
    strcat(msg, str3);
  }

 strcat(msg,"\n");
fprintf(f, "%s",msg);
fprintf(f2, "%s",msg);

fprintf(f2, "pause -1 \"Close plot %d\"\n", cur_plot);

fclose(f);
fclose(f2);
cmd( "set choice $gnu" );
show_plot_gnu(cur_plot, choice, *choice, str, tag);
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
SHOW_PLOT_GNU
****************************************************/
void show_plot_gnu( int n, int *choice, int type, char **str, char **tag )
{
int i, hsize, vsize, sbordsize, lim[ 4 ];
double rang[ 4 ];

if ( type == 2 )
{
	cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Unsupported options for the internal graphical window\" -detail \"Gnuplot automatically selected to support the chosen 3D plot options.\"" );
	gnu = true;
	type = 1;
}

cmd( "raise .da ." );
	
if(type==1)
{//plot with external gnuplot
   cmd( "if {$tcl_platform(platform) == \"unix\"} { set choice [ catch { exec xterm -e gnuplot gnuplot.gp & } ] } {if {$tcl_platform(os) == \"Windows NT\"} { set choice [ catch { exec wgnuplot gnuplot.gp & } ] } { set choice [ catch { exec start wgnuplot gnuplot.gp & } ] } }" );
   
   if ( *choice != 0 )			// Gnuplot failed
   {
		cmd( "tk_messageBox -parent .da -type ok -icon error -title Error -message \"Gnuplot returned error '$choice'\" -detail \"Please check if you have selected an adequate configuration for the plot.\"" );
		*choice=2;
		return;
   }
      	
   type_plot[ n ] = -1; //external plot
   plot_w[ n ] = -1;
   plot_l[ n ] = -1;
   plot_nl[ n ] = -1;   
   
   *choice = 0;
   return;
}

// get graphical configuration from Tk (file defaults.tcl)
get_int( "hsizePxy", & hsize );			// 640
get_int( "vsizePxy", & vsize );			// 450
get_int( "sbordsizeP", & sbordsize );	// 0


// generate tk canvas filling routine using Gnuplot
cmd( "if { $tcl_platform(platform) == \"unix\" } { set choice [ catch { exec xterm -e gnuplot gnuplot.lsd } ] } { if { $tcl_platform(os) == \"Windows NT\" } { set choice [ catch { exec wgnuplot gnuplot.lsd } ] } { set choice [ catch { exec start wgnuplot gnuplot.lsd } ] } }" );
if ( *choice != 0 )			// Gnuplot failed
{
	cmd( "tk_messageBox -parent .da -type ok -icon error -title Error -message \"Gnuplot returned error '$choice'\" -detail \"Please check if you have selected an adequate configuration for the plot and if Gnuplot is set up properly.\"" );
	*choice=2;
	return;
}
shrink_gnufile( );
cmd( "file delete plot.file; file rename plot_clean.file plot.file" );

// create plot window & canvas
cmd( "set w .da.f.new%d", n );			// plot window
cmd( "set p $w.f.plots" );				// plot canvas

cmd( "newtop $w $tit \"wm withdraw $w\" \"\"" );

cmd( "frame $w.f" );
cmd( "scrollbar $w.f.vs -command \"$p yview\"" );
cmd( "scrollbar $w.f.hs -orient horiz -command \"$p xview\"" );
cmd( "pack $w.f.vs -side right -fill y" );
cmd( "pack $w.f.hs -side bottom -fill x" );

cmd( "canvas $p -width %d -height %d -background white -relief flat -yscrollcommand \"$w.f.vs set\" -xscrollcommand \"$w.f.hs set\" -scrollregion \"%d %d %d %d\" -relief flat -highlightthickness 0", 
		hsize, vsize, - sbordsize, - sbordsize, hsize + sbordsize, vsize + sbordsize  );

cmd( "pack $p -expand yes -fill both" );
cmd( "pack $w.f -expand yes -fill both" );

// add buttons bottom bar
cmd( "frame $w.b" );

cmd( "frame $w.b.c" );

cmd( "frame $w.b.c.case" );
if( logs )
	cmd( "label $w.b.c.case.l -text \"log(X) value:\" -width 11 -anchor e" );
else
	cmd( "label $w.b.c.case.l -text \"X value:\" -width 11 -anchor e" );
cmd( "label $w.b.c.case.v -text \"\" -fg red -width 20 -anchor w" );
cmd( "pack $w.b.c.case.l $w.b.c.case.v -side left -anchor w" );

cmd( "frame $w.b.c.y" );
if( logs )
	cmd( "label $w.b.c.y.l -text \"log(Y) value:\" -width 11 -anchor e" );
else
	cmd( "label $w.b.c.y.l -text \"Y value:\" -width 11 -anchor e" );
cmd( "label $w.b.c.y.v1 -text \"\" -fg red -width 20 -anchor w" );
cmd( "pack $w.b.c.y.l $w.b.c.y.v1 -side left -anchor w");

cmd( "pack $w.b.c.case $w.b.c.y -anchor w" );

cmd( "frame $w.b.o" );
cmd( "label $w.b.o.l1 -text \"Right-click: edit properties\"" );
cmd( "label $w.b.o.l2 -text \"Shift-click: insert text\"" );
cmd( "label $w.b.o.l3 -text \"Ctrl-click: insert line\"" );
cmd( "pack $w.b.o.l1 $w.b.o.l2 $w.b.o.l3" );

cmd( "frame $w.b.s" );
cmd( "button $w.b.s.save -width 9 -text Save -command { set it \"%d) $tit\"; set fromPlot true; set choice 11 } -state disabled -underline 0", n );
cmd( "button $w.b.s.gnu -width 9 -text Gnuplot -command { set oldpath [pwd]; cd plotxy_%d; if { $tcl_platform(platform) == \"unix\" } { set choice [ catch { exec xterm -e gnuplot gnuplot.gp & } ] } { if { $tcl_platform(os) == \"Windows NT\" } { set choice [ catch { exec wgnuplot gnuplot.gp & } ] } { set choice [ catch { exec start wgnuplot gnuplot.gp & } ] } }; cd $oldpath; if { $choice != 0 } { tk_messageBox -parent .da.f.new%d -type ok -icon error -title Error -message \"Gnuplot returned error '$choice'\" -detail \"Please check if Gnuplot is set up properly.\" } } -state disabled -underline 0", n, n );
cmd( "pack $w.b.s.save $w.b.s.gnu -pady 5" );

cmd( "label $w.b.pad -width 6" );

cmd( "frame $w.b.z -bd 2 -relief groove" );
cmd( "label $w.b.z.l -text Zoom" );

cmd( "frame $w.b.z.b" );
cmd( "button $w.b.z.b.p -width 3 -text + -command { scale_canvas .da.f.new%d.f.plots \"+\" zoomLevel%d } -state disabled", n, n );
cmd( "button $w.b.z.b.m -width 3 -text - -command { scale_canvas .da.f.new%d.f.plots \"-\" zoomLevel%d } -state disabled", n, n  );
cmd( "pack $w.b.z.b.p $w.b.z.b.m" );

cmd( "pack  $w.b.z.l $w.b.z.b -side left -padx 2 -pady 2" );

cmd( "pack $w.b.c $w.b.o $w.b.pad $w.b.s $w.b.z -expand no -padx 10 -pady 5 -side left" );
cmd( "pack $w.b -side right -expand no" );

cmd( "mouse_wheel $p" );

cmd( "showtop $w current yes yes no" ); 
cmd( "$p xview moveto 0; $p yview moveto 0");
cmd( "set zoomLevel%d 1.0", n );

// create list with the series names+tags in Tk
if ( str != NULL && tag != NULL )
{
	cmd( "set series%d [ list ]", n );
	for ( i = 0; i < nv; ++i )
		cmd( "lappend series%d \"%s_%s\"", n, str[ i ], tag[ i ] );
}

// draw canvas
cmd( "source plot.file" );
cmd( "catch [ gnuplot $p ]" );
cmd( "catch [ rename gnuplot \"\" ]" );

// canvas plot limits (canvas & series)
cmd( "set cmx [ expr [ winfo width $p ] - 2 * [ $p cget -border ] - 2 * [ $p cget -highlightthickness ] ]" );
cmd( "if { $cmx <= 1 } { set cmx [ $p cget -width ] }" );
cmd( "set cmy [ expr [ winfo height $p ] - 2 * [ $p cget -border ] - 2 * [ $p cget -highlightthickness ] ]" );
cmd( "if { $cmy <= 1 } { set cmy [ $p cget -height ] }" );
cmd( "unset -nocomplain lim rang" );
cmd( "catch [ set lim [ gnuplot_plotarea ] ]" );
cmd( "catch [ set rang [ gnuplot_axisranges ] ]" );
cmd( "if { [ info exists lim ] && [ info exists rang ] } { set choice 1 } { set choice 0 }" );
if ( *choice == 1 )
{
	cmd( "set res [ expr int( $cmx * [ lindex $lim 0 ] / 1000.0 ) ]" );
	get_int( "res", &lim[ 0 ] );
	cmd( "set res [ expr int( $cmx * [ lindex $lim 1 ] / 1000.0 ) ]" );
	get_int( "res", &lim[ 1 ] );
	cmd( "set res [ expr int( $cmy * [ lindex $lim 2 ] / 1000.0 ) ]" );
	get_int( "res", &lim[ 2 ] );
	cmd( "set res [ expr int( $cmy * [ lindex $lim 3 ] / 1000.0 ) ]" );
	get_int( "res", &lim[ 3 ] );
	for ( i = 0; i < 4; ++i )
	{	
		cmd( "set res [ lindex $rang %d ]", i );
		get_double( "res", &rang[ i ] );
	}
}
else
	for ( i = 0; i < 4; ++i )
		rang[ i ] = lim[ i ] = 0;

// enable buttons
cmd( "$w.b.s.save configure -state normal" );
cmd( "$w.b.s.gnu configure -state normal" );
cmd( "$w.b.z.b.p configure -state normal" );
cmd( "$w.b.z.b.m configure -state normal" );

// update cursor indicators at bottom window
cmd( "bind $p <Motion> { \
		set zoom $zoomLevel%d; \
		set series $series%d; \
		set w .da.f.new%d; \
		set llim [ expr %d * $zoom ]; \
		set rlim [ expr %d * $zoom ]; \
		set tlim [ expr %d * $zoom ]; \
		set blim [ expr %d * $zoom ]; \
		set cx [ $w.f.plots canvasx %%x ]; \
		set cy [ $w.f.plots canvasy %%y ]; \
		if { $cx >= $llim && $cx <= $rlim && $cy >= $tlim && $cy <= $blim && [ expr $rlim - $llim ] > 0 && [ expr $blim - $tlim ] > 0 } { \
			$w.b.c.case.v configure -text [ format \"%%%%.[ expr $pdigits ]g\" [ expr ( $cx - $llim ) * ( %lf - %lf ) / ( $rlim - $llim ) + %lf ] ]; \
			$w.b.c.y.v1 configure -text [ format \"%%%%.[ expr $pdigits ]g\" [ expr ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf ] ]; \
		} \
	}", n, n, n, lim[ 0 ], lim[ 1 ], lim[ 2 ], lim[ 3 ], rang[ 1 ], rang[ 0 ], rang[ 0 ], rang[ 3 ], rang[ 2 ], rang[ 2 ] );

// window bindings (return to Analysis, insert text, insert line)
cmd( "bind $w <Escape> \"wm withdraw $w\"" );
cmd( "bind $w <s> { $w.b.s.save invoke }; bind $w <S> { $w.b.s.save invoke }" );
cmd( "bind $w <g> { $w.b.s.gnu invoke }; bind $w <G> { $w.b.s.gnu invoke }" );
cmd( "bind $w <plus> { $w.b.z.b.p invoke }" );
cmd( "bind $w <minus> { $w.b.z.b.m invoke }" );
cmd( "bind $p <Double-Button-1> { raise .da }" );

cmd( "bind $p <Shift-1> { \
		set ccanvas .da.f.new%d.f.plots; \
		set LX %%X; \
		set LY %%Y; \
		set hereX [ $ccanvas canvasx %%x ]; \
		set hereY [ $ccanvas canvasy %%y ]; \
		set choice 27 \
	}", n );
	
cmd( "bind $p <Control-1> { \
		set ccanvas .da.f.new%d.f.plots; \
		set ncanvas %d; \
		set hereX [ $ccanvas canvasx %%x ]; \
		set hereY [ $ccanvas canvasy %%y ]; \
		unset -nocomplain cl; \
		set choice 28 \
	}", n, n );

// moving and editing lines and text
cmd( "bind $p <Button-1> { \
		set ccanvas .da.f.new%d.f.plots; \
		set type [ $ccanvas gettags current ]; \
		if { [ lsearch -regexp $type (draw|legend) ] >= 0 } { \
			set moving true; \
			set hereX [ $ccanvas canvasx %%x ]; \
			set hereY [ $ccanvas canvasy %%y ]; \
			$ccanvas raise current \
		} { \
			set moving false \
		} \
	}", cur_plot );
cmd( "bind $p <B1-Motion> { \
		set ccanvas .da.f.new%d.f.plots; \
		if $moving { \
			$ccanvas move current [ expr [ $ccanvas canvasx %%x ] - $hereX ] \
				[ expr [ $ccanvas canvasy %%y ] - $hereY ]; \
			set hereX [ $ccanvas canvasx %%x ]; \
			set hereY [ $ccanvas canvasy %%y ] \
		} \
	}", cur_plot );
cmd( "bind $p <ButtonRelease-1> { set moving false }" );

cmd( "bind $p <Button-2> { \
		set ccanvas .da.f.new%d.f.plots; \
		set LX %%X; set LY %%Y; \
		set type [ $ccanvas gettags current ]; \
		if { [ lsearch $type line ] >= 0 || [ lsearch $type dots ] >= 0 } { \
			if { [ lsearch $type series ] >= 0 } { \
				set cline [ lindex $type 0 ]; \
				set draw false \
			} { \
				set cline current; \
				set draw true \
			}; \
			set choice 31 \
		} { \
			if { [ lsearch $type text ] >= 0 } { \
				set choice 26 \
			} \
		} \
	}", cur_plot );
cmd( "bind $p <Button-3> { event generate .da.f.new%d.f.plots <Button-2> -x %%x -y %%y }", cur_plot );

// save plot info
type_plot[ n ] = GNUPLOT; //Gnuplot plot
plot_w[ n ] = hsize;	// plot width
plot_l[ n ] = vsize; 	//height of plot with labels
plot_nl[ n ] = vsize; 	//height of plot without labels   
 
*choice = 0;
}


/***************************************************
PLOT_LATTICE
****************************************************/
void plot_lattice(int *choice)
{

FILE *f, *f2;
double **data;
int i, j, hi, le, done, nlags, ncol, nlin, end;

if ( nv == 0 )		 // no plots to save
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No plot available\" -detail \"Place one or more series in the Series Selected listbox and select 'Plot' to produce a plot.\"" );
	*choice = 2;
	return;
}

data=new double *[nv];
if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

cmd( "set res [.da.vars.ch.v get 0]" );
cmd( "scan $res \"%%s %%s (%%d - %%d) # %%d\" a b c d choice" );
end=vs[nv].end;

for(i=0; i<nv; i++)
 {
  cmd( "set res [.da.vars.ch.v get %d]", i );
  cmd( "scan $res \"%%s %%s (%%d - %%d) # %%d\" a b c d choice" );
  data[i]=vs[nv].data;
 }

cmd( "newtop .da.s \"Lattice Options\" { set choice 2 } .da" );

cmd( "frame .da.s.i" );
cmd( "label .da.s.i.l -text \"Number of columns\"" );
cmd( "set bidi 1" );
cmd( "entry .da.s.i.e -width 10 -validate focusout -vcmd { if { [ string is integer %%P ] && %%P > 0 } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "pack .da.s.i.l .da.s.i.e" );

cmd( "frame .da.s.t" );
cmd( "label .da.s.t.l -text \"Time step\"" );
cmd( "set time %d", end );
cmd( "entry .da.s.t.e -width 10 -validate focusout -vcmd { if [ string is integer %%P ] { set time %%P; return 1 } { %%W delete 0 end; %%W insert 0 $time; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.t.e insert 0 $time" ); 
cmd( "pack .da.s.t.l .da.s.t.e" );

cmd( "frame .da.s.x" );
cmd( "label .da.s.x.l -text \"Lattice width\"" );
cmd( "set lx 400" );
cmd( "entry .da.s.x.e -width 10 -validate focusout -vcmd { if [ string is integer %%P ] { set lx %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lx; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.x.e insert 0 $lx" ); 
cmd( "pack .da.s.x.l .da.s.x.e" );

cmd( "frame .da.s.y" );
cmd( "label .da.s.y.l -text \"Lattice heigth\"" );
cmd( "set ly 400" );
cmd( "entry .da.s.y.e -width 10 -validate focusout -vcmd { if [ string is integer %%P ] { set ly %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ly; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.y.e insert 0 $ly" ); 
cmd( "pack .da.s.y.l .da.s.y.e" );

cmd( "pack .da.s.t .da.s.i .da.s.x .da.s.y -padx 5 -pady 5" );

cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#lattice } { set choice 2 }" );

cmd( "bind .da.s.t.e <KeyPress-Return> {focus .da.s.i.e; .da.s.i.e selection range 0 end}" );
cmd( "bind .da.s.i.e <KeyPress-Return> {focus .da.s.x.e; .da.s.x.e selection range 0 end}" );
cmd( "bind .da.s.x.e <KeyPress-Return> {focus .da.s.y.e; .da.s.y.e selection range 0 end}" );
cmd( "bind .da.s.y.e <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

cmd( "showtop .da.s centerW" );
cmd( "focus .da.s.t.e; .da.s.t.e selection range 0 end" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "set time [ .da.s.t.e get ]" ); 
cmd( "set lx [ .da.s.x.e get ]" ); 
cmd( "set ly [ .da.s.y.e get ]" ); 
cmd( "destroytop .da.s" );

if(*choice==2)
 goto end;

cmd( "set choice $time" );
nlags=*choice;
cmd( "set choice $bidi" );
ncol=*choice;

nlin=nv/ncol;
if(nlin*ncol!=nv)
 {
 cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of columns\"" );
 *choice=2;
 goto end;
 }

cmd( "set choice $ly" );
hi=*choice/nlin;
cmd( "set choice $lx" );
le=*choice/ncol;

cmd( "set w .da.f.new%d", cur_plot );
cmd( "newtop $w $tit { wm withdraw .da.f.new%d } \"\"", cur_plot );

cmd( "frame $w.f -width %d -height %d", ncol*le, nlin*hi );
cmd( "pack $w.f" );

//Reset p to point to the canvas (shit...)
cmd( "set p $w.f.plots" );
cmd( "canvas $p -width %d -height %d -background white -relief flat", ncol*le, nlin*hi );
cmd( "pack $p" );

cmd( "bind $w <Double-Button-1> {raise .da}" );

cmd( "showtop $w current yes yes no" );

if(!grid)
{
for(j=0; j<nlin; j++)
 for(i=0; i<ncol; i++)
  cmd( "$p create poly %d %d %d %d %d %d %d %d -fill $c%d", i*le, j*hi, i*le, (j+1)*hi, (i+1)*le, (j+1)*hi, (i+1)*le, j*hi, (int)data[ncol*j+i][nlags] );
}
else
{
for(j=0; j<nlin; j++)
 for(i=0; i<ncol; i++)
  cmd( "$p create poly %d %d %d %d %d %d %d %d -fill $c%d -outline black", i*le, j*hi, i*le, (j+1)*hi, (i+1)*le, (j+1)*hi, (i+1)*le, j*hi, (int)data[ncol*j+i][nlags] );
}

type_plot[ cur_plot ] = LATTICE;
plot_w[ cur_plot ] = ncol * le;	// plot width
plot_l[ cur_plot ] = nlin * hi;
plot_nl[ cur_plot ] = nlin * hi;  

end:
*choice=0;

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
     cmd( "lappend series_bylab \"%s (Obj. %s)\"",cv->label, cur->label );
     cmd( "lappend series_byobj \"(Obj. %s) %s\"", cur->label, cv->label );

    }
  if(cur->son!=NULL)
   add_list(cur->son);

 }

}

void list_series(object *r)
{
cmd( "set a [info vars series_bylab]" );
cmd( "if {$a==\"\" } {} {unset series_bylab; unset series_byobj}" );
add_list(r);
cmd( "set temp [lsort -dictionary -integer $series_bylab]" );
cmd( "unset series_bylab" );
cmd( "set series_bylab $temp" );
cmd( "set temp [lsort -dictionary -integer $series_byobj]" );
cmd( "unset series_byobj" );
cmd( "set series_byobj $temp" );

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
char str[MAX_ELEM_LENGTH];
char tag[MAX_ELEM_LENGTH];

int start, end;
int i, num_bin, j, first, last, stat;
int x1, x2, y1,y2;
double ap, mx,mn, *data, step, a, b, s, lminy, miny2, truemaxy, truemaxy2, average, sigma, tot, totnorm;
bin *cl;

if ( nv != 1 )
 {
	if ( nv == 0 )			// no variables selected
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one series in the Series Selected listbox.\"" );
	else
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For time series histograms select only one series.\"" );
	*choice=2;
	return;
 } 
 
cmd( "set res [.da.vars.ch.v get 0]" );
app=(char *)Tcl_GetVar(inter, "res",0);
strcpy(msg,app);
sscanf(msg, "%s %s (%d - %d) # %d", str, tag, &start, &end, &idseries);

data = vs[ idseries ].data;
if(autom_x)
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
 if(is_finite(data[i]))				// ignore NaNs
  tot++;

cmd( "newtop .da.s \"Histogram Options\" { set choice 2 } .da" );

cmd( "frame .da.s.i" );
cmd( "label .da.s.i.l -text \"Number of classes/bins\"" );
cmd( "set bidi %d", 100<tot?100:(int)tot );
cmd( "entry .da.s.i.e -width 10 -validate focusout -vcmd { if { [ string is integer %%P ] && %%P > 0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "pack .da.s.i.l .da.s.i.e" );

cmd( "set norm 0" );
cmd( "checkbutton .da.s.norm -text \"Interpolate a Normal\" -variable norm" );
cmd( "set stat 0" );
cmd( "checkbutton .da.s.st -text \"Print statistics in Log window\" -variable stat" );
cmd( "pack .da.s.i .da.s.norm .da.s.st -padx 5 -pady 5" );

cmd( "pack .da.s.i" );

cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#seq_xy } { set choice 2 }" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );

cmd( "showtop .da.s centerW" );
cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "destroytop .da.s" );

if(*choice==2)
 return;

cmd( "set choice $bidi" );
num_bin=*choice;
cl=new bin[num_bin];
ap=(double)num_bin;

tot=average=sigma=0;
for(i=first; i<=last; i++)
 {
 if(!is_finite(data[i]))		// ignore NaNs
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
  if(!is_finite(data[i]))		// ignore NaNs
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

cmd( "set choice $stat" );
stat = *choice;

if(stat==1)
 plog("\n\n#    Boundaries(center)\t\tMin\tAve\tMax\tNum.\tFreq.");
step=(mx+a/2-(mn-a/2))/(ap);

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
 plog( "\n%3d: %.*g<=X<%.*g (%.*g)\t\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g", "", i+1, pdigits, mn-a/2+(double)(i)*step, pdigits, mn -a/2 + (double)(i+1)*step, pdigits, mn -a/2 +(double)(i)*step +step/2, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max, pdigits, cl[i].num, pdigits, cl[i].num/(tot) );
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
 cmd( "wm deiconify .log; raise .log .da" ); 
if(!autom && miny<maxy)
 {
  lminy=miny;
  miny2=lminy/tot;
  truemaxy=maxy;
  truemaxy2=maxy/tot;
 }
else
 {
  cmd( ".da.f.h.v.sc.max.max conf -state normal" );
  cmd( ".da.f.h.v.sc.min.min conf -state normal" );  
  maxy=truemaxy;
  miny=lminy;
  cmd( "update" );
  cmd( ".da.f.h.v.sc.max.max conf -state disabled" );
  cmd( ".da.f.h.v.sc.min.min conf -state disabled" );  
 }


cmd( "set w .da.f.new%d", cur_plot );

cmd( "newtop $w $tit { wm withdraw .da.f.new%d } \"\"", cur_plot );

cmd( "frame $w.f -width 640 -height 430" );
cmd( "pack $w.f" );

//Reset p to point to the canvas (shit...)
cmd( "set p $w.f.plots" );
cmd( "canvas $p -width 640 -height 430 -background white -relief flat" );
cmd( "pack $p" );

cmd( "bind $w <Double-Button-1> {raise .da; focus .da.b.ts}" );

cmd( "showtop $w current yes yes no" );

cmd( "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 40 0 40 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 190 0 190 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 340 0 340 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 490 0 490 310 -fill grey60 -width 1p -tag p" );
}
else
{
cmd( "$p create line 640 295 640 305  -width 1 -tag p" );
cmd( "$p create line 190 295 190 305 -width 1 -tag p" );
cmd( "$p create line 340 295 340 305 -width 1 -tag p" );
cmd( "$p create line 490 295 490 305 -width 1 -tag p" );
}

cmd( "set choice [$p create text 40 312 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,cl[0].lowb );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

i=num_bin/4;
cmd( "set choice [$p create text 190 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

if(num_bin%2==0)
 a=cl[num_bin/2].lowb;
else
 a=cl[(num_bin-1)/2].center; 
i=num_bin/2;
cmd( "set choice [$p create text 340 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p  text}]", pdigits,a );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

i=(int)((double)(num_bin)*.75);
cmd( "set choice [$p create text 490 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 635 312 -font {Times 10 normal} -anchor ne -text %.*g -tag {p  text}]", pdigits,cl[num_bin-1].highb );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 38 300 640 300 -fill grey60 -tag p" );
cmd( "$p create line 38 225 640 225 -fill grey60 -tag p" );
cmd( "$p create line 38 150 640 150 -fill grey60 -tag p" );
cmd( "$p create line 38 75 640 75 -fill grey60 -tag p" );
cmd( "$p create line 38 0 640 0 -fill grey60 -tag p" );
}
else
{
cmd( "$p create line 35 225 45 225  -tag p" );
cmd( "$p create line 35 150 45 150  -tag p" );
cmd( "$p create line 35 75 45 75  -tag p" );
cmd( "$p create line 35 2 45 2 -tag p" );
}

cmd( "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,lminy );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p  text}]", pdigits,(lminy+(truemaxy-lminy)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,(truemaxy) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 312 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,miny2 );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 237 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 162 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );


cmd( "set choice [$p create text 4 87 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 16 -font {Times 10 normal} -anchor nw -text (%.*g) -tag {p text}]", pdigits,(truemaxy2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

*choice=0;
for(i=0; i<num_bin; i++)
 {
  x1=40+(int)(600* (cl[i].lowb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  x2=40+(int)(600* (cl[i].highb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  y1=(int)(300-((cl[i].num-lminy)/(truemaxy-lminy))*300);
  y2=300;

  if(y1<301)
  {//in case of user-definde miny
   cmd( "$p create rect %d %d %d %d -tag p%d -width 1 -fill $c%d",x1,y1,x2,y2, i, 1 );
   cmd( "$p bind p%d <Enter> {.da.f.new%d.f.plots delete xpos; .da.f.new%d.f.plots create text 0 390 -font {Times 10 normal} -text \"Class %d, %.*g <= X < %.*g, (center=%.*g)\\nContains %.*g units (%.*g perc.). Actual values: min=%.*g, av.=%.*g, max=%.*g  \" -anchor w -tag xpos}", i,  cur_plot, cur_plot, i, pdigits, cl[i].lowb, pdigits, cl[i].highb, pdigits, cl[i].center, pdigits, cl[i].num, pdigits, 100*cl[i].num/tot, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max  );
   cmd( "$p bind p%d <Leave> {.da.f.new%d.f.plots delete xpos}", i, cur_plot );
  }
  if(*choice==1) //Button STOP pressed
     i=num_bin;
 if(watch)
	cmd( "update" );
 }

/***
just to check that the sum is one
*/
cmd( "set choice $norm" );
if(*choice==1)
{
 totnorm=0;
 for(i=0; i<num_bin; i++)
  {
   a=cl[i].lowb;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*M_PI*sigma));
   s=cl[i].highb;
   ap=exp(-(s-average)*(s-average)/(2*sigma))/(sqrt(2*M_PI*sigma));
   totnorm+=(b+ap)/2;
  }
  
 /*******/
 for(i=0; i<num_bin-1; i++)
  {
   a=cl[i].center;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*M_PI*sigma));
   b=b*tot/totnorm;
   b=300-((b-lminy)/(truemaxy-lminy))*300;    
   a=cl[i+1].center;  
   s=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*M_PI*sigma));
   s=s*tot/totnorm;
   s=300-((s-lminy)/(truemaxy-lminy))*300;  
   
   x1=40+(int)(600* (cl[i].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
   x2=40+(int)(600* (cl[i+1].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
 
   cmd( "$p create line %d %d %d %d",x1,(int)b, x2, (int)s );
  }
} 
    
cmd( "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}", cur_plot );
cmd( "bind $p <Control-1> { set ncanvas %d; set px %%x; set py %%y; set ax [expr $px+1]; set ay [expr $py+1]; .da.f.new%d.f.plots dtag selected; set cl [.da.f.new%d.f.plots create line $px $py $ax $ay]; .da.f.new%d.f.plots addtag selected withtag $cl; set choice 28 }", cur_plot, cur_plot, cur_plot, cur_plot );

type_plot[ cur_plot ] = HISTOGR;
plot_w[ cur_plot ] = 640;	// plot width
plot_l[ cur_plot ] = 325; //height of plot with labels
plot_nl[ cur_plot ] = 325; //height of plot with labels  
 
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
int i, num_bin, j, first, last, stat, time, active_v;
int x1, x2, y1,y2;
double ap, mx,mn,  step, a, b, s, lminy, miny2, truemaxy, truemaxy2, average, sigma, tot, totnorm;
bin *cl;

int logErrCnt = 0;				// log errors counter to prevent excess messages
bool stopErr = false;

if ( nv < 2 )
{
	if ( nv == 0 )			// no variables selected
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place two or more series in the Series Selected listbox.\"" );
	else
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For Cross Section histograms select at least two series.\"" );
	*choice=2;
	return;
}

data=new double *[nv];
logdata=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  data[ i ] = vs[ idseries ].data;
  
   if(logs)			// apply log to the values to show "log scale" in the y-axis
   {
	 logdata[i]=new double[end[i]+1];	// create space for the logged values
     for(j=start[i];j<=end[i];j++)		// log everything possible
	   if(!is_nan(data[i][j]) && data[i][j]>0.0)		// ignore NaNs
		 logdata[i][j]=log(data[i][j]);
	   else
	   {
		 logdata[i][j]=NAN;
		 if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
		 {
			plog( "\nWarning: zero or negative values in log plot (set to NaN)\n         Series: %d, Case: %d", "", i + 1, j );
		 }
		 else
			if ( ! stopErr )
			{
				plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
				stopErr = true;
			}
	   }
	 data[i]=logdata[i];				// replace the data series
   }
 }

cmd( "set bidi %d", 100<nv?100:nv );
cmd( "set time %d", end[0] );

cmd( "newtop .da.s \"Histogram Options\" { set choice 2 } .da" );

cmd( "frame .da.s.t" );
cmd( "label .da.s.t.l -text \"Time step\"" );
cmd( "entry .da.s.t.e -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set time %%P; return 1 } { %%W delete 0 end; %%W insert 0 $time; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.t.e insert 0 $time" ); 
cmd( "pack .da.s.t.l .da.s.t.e" );
cmd( "bind .da.s.t.e <Return> {focus .da.s.i.e; .da.s.i.e selection range 0 end}" );

cmd( "frame .da.s.i" );
cmd( "label .da.s.i.l -text \"Number of classes/bins\"" );
cmd( "entry .da.s.i.e -width 5 -validate focusout -vcmd { if { [ string is integer %%P ] && %%P > 0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.i.e insert 0 $bidi" ); 
cmd( "pack .da.s.i.l .da.s.i.e" );

cmd( "set norm 0" );
cmd( "checkbutton .da.s.norm -text \"Interpolate a Normal\" -variable norm" );
cmd( "set stat 0" );
cmd( "checkbutton .da.s.st -text \"Print statistics in Log window\" -variable stat" );

cmd( "pack .da.s.t .da.s.i .da.s.norm .da.s.st -padx 5 -pady 5" );

cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#seq_xy } { set choice 2 }" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );

cmd( "showtop .da.s centerW" );
cmd( "focus .da.s.t.e; .da.s.t.e selection range 0 end" );

*choice=0;
while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "set bidi [ .da.s.i.e get ]" ); 
cmd( "set time [ .da.s.t.e get ]" ); 
cmd( "destroytop .da.s" );

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
 
 return;
 }

cmd( "set choice $bidi" );
num_bin=*choice;
cmd( "set choice $time" );
time=*choice;

cl=new bin[num_bin];
ap=(double)num_bin;

tot=average=sigma=0;
active_v=0;

if(autom)
{//min and max fixed automatically
for(i=0; i<nv; i++)
 {
 if(start[i]<=time && end[i]>=time && is_finite(data[i][time]))		// ignore NaNs
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
 if(start[i]<=time && end[i]>=time && is_finite(data[i][time]) && data[i][time]>=mn && data[i][time]<=mx)		// ignore NaNs
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
  if(start[i]<=time && end[i]>=time && is_finite(data[i][time]) && data[i][time]>=mn && data[i][time]<=mx)		// ignore NaNs
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

cmd( "set choice $stat" );
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
 plog( "\n%3d %.*g<=X<%.*g (%.*g)\t\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g", "", i+1, pdigits, mn-a/2+(double)(i)*step, pdigits, mn -a/2 + (double)(i+1)*step, pdigits, mn -a/2 +(double)(i)*step +step/2, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max, pdigits, cl[i].num, pdigits, cl[i].num/(tot) );
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
 cmd( "wm deiconify .log; raise .log .da" ); 
if(!autom && miny<maxy)
 {
  lminy=miny;
  miny2=lminy/tot;
  truemaxy=maxy;
  truemaxy2=maxy/tot;
 }
else
 {
  cmd( ".da.f.h.v.sc.max.max conf -state normal" );
  cmd( ".da.f.h.v.sc.min.min conf -state normal" );  
  maxy=truemaxy;
  miny=lminy;
  cmd( "update" );
  cmd( ".da.f.h.v.sc.max.max conf -state disabled" );
  cmd( ".da.f.h.v.sc.min.min conf -state disabled" );  
 }

cmd( "set w .da.f.new%d", cur_plot );
cmd( "newtop $w $tit { wm withdraw .da.f.new%d } \"\"", cur_plot );

cmd( "frame $w.f -width 640 -height 430" );
cmd( "pack $w.f" );

//Reset p to point to the canvas (shit...)
cmd( "set p $w.f.plots" );
cmd( "canvas $p -width 640 -height 430 -background white -relief flat" );
cmd( "pack $p" );

cmd( "bind $w <Double-Button-1> {raise .da; focus .da.b.ts}" );
cmd( "showtop $w current yes yes no" );

cmd( "$p create line 40 300 640 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 40 0 40 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 190 0 190 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 340 0 340 310 -fill grey60 -width 1p -tag p" );
cmd( "$p create line 490 0 490 310 -fill grey60 -width 1p -tag p" );
}
else
{
cmd( "$p create line 640 295 640 305  -width 1 -tag p" );
cmd( "$p create line 190 295 190 305 -width 1 -tag p" );
cmd( "$p create line 340 295 340 305 -width 1 -tag p" );
cmd( "$p create line 490 295 490 305 -width 1 -tag p" );
}

cmd( "set choice [$p create text 40 312 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,cl[0].lowb );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

i=num_bin/4;
cmd( "set choice [$p create text 190 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

if(num_bin%2==0)
 a=cl[num_bin/2].lowb;
else
 a=cl[(num_bin-1)/2].center; 
i=num_bin/2;
cmd( "set choice [$p create text 340 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p  text}]", pdigits,a );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

i=(int)((double)(num_bin)*.75);
cmd( "set choice [$p create text 490 312 -font {Times 10 normal} -anchor n -text %.*g -tag {p text}]", pdigits,cl[i].center );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 635 312 -font {Times 10 normal} -anchor ne -text %.*g -tag {p  text}]", pdigits,cl[num_bin-1].highb );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "$p create line 40 0 40 300 -width 1 -fill grey50 -tag p" );

if(grid)
{
cmd( "$p create line 38 300 640 300 -fill grey60 -tag p" );
cmd( "$p create line 38 225 640 225 -fill grey60 -tag p" );
cmd( "$p create line 38 150 640 150 -fill grey60 -tag p" );
cmd( "$p create line 38 75 640 75 -fill grey60 -tag p" );
cmd( "$p create line 38 0 640 0 -fill grey60 -tag p" );
}
else
{
cmd( "$p create line 35 225 45 225  -tag p" );
cmd( "$p create line 35 150 45 150  -tag p" );
cmd( "$p create line 35 75 45 75  -tag p" );
cmd( "$p create line 35 2 45 2 -tag p" );
}

cmd( "set choice [$p create text 4 300 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,lminy );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 225 -font {Times 10 normal} -anchor sw -text %.*g -tag {p  text}]", pdigits,(lminy+(truemaxy-lminy)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 150 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 75 -font {Times 10 normal} -anchor sw -text %.*g -tag {p text}]", pdigits,(lminy+(truemaxy-lminy)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 4 -font {Times 10 normal} -anchor nw -text %.*g -tag {p text}]", pdigits,(truemaxy) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 312 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,miny2 );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 237 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 162 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)/2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 87 -font {Times 10 normal} -anchor sw -text (%.*g) -tag {p text}]", pdigits,(miny2+(truemaxy2-miny2)*3/4) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

cmd( "set choice [$p create text 4 16 -font {Times 10 normal} -anchor nw -text (%.*g) -tag {p text}]", pdigits,(truemaxy2) );
cmd( ".da.f.new%d.f.plots bind $choice <Button-3> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );
cmd( ".da.f.new%d.f.plots bind $choice <Button-2> {.da.f.new%d.f.plots dtag selected; .da.f.new%d.f.plots addtag selected withtag %d; set ccanvas .da.f.new%d.f.plots; set hereX %%X ; set hereY %%y; set choice 26}", cur_plot,cur_plot,cur_plot, *choice,cur_plot );

*choice=0;
for(i=0; i<num_bin; i++)
 {
  x1=40+(int)(600* (cl[i].lowb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  x2=40+(int)(600* (cl[i].highb-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
  y1=(int)(300-((cl[i].num-lminy)/(truemaxy-lminy))*300);
  y2=300;

  if(y1<301)
  {//in case of user-definde miny
  cmd( "$p create rect %d %d %d %d -tag p%d -width 1 -fill $c%d",x1,y1,x2,y2, i, 1 );
  cmd( "$p bind p%d <Enter> {.da.f.new%d.f.plots delete xpos; .da.f.new%d.f.plots create text 0 390 -font {Times 10 normal} -text \"Class %d, %.*g <= X < %.*g, (center=%.*g)\\nContains %.*g units (%.*g perc.). Actual values: min=%.*g, av.=%.*g, max=%.*g  \" -anchor w -tag xpos}",i,  cur_plot, cur_plot, i, pdigits, cl[i].lowb, pdigits, cl[i].highb, pdigits, cl[i].center, pdigits, cl[i].num, pdigits, 100*cl[i].num/tot, pdigits, cl[i].min, pdigits, cl[i].av, pdigits, cl[i].max );
  cmd( "$p bind p%d <Leave> {.da.f.new%d.f.plots delete xpos}",i, cur_plot );
  }
  if(*choice==1) //Button STOP pressed
     i=num_bin;
 if(watch)
	cmd( "update" );
 }

/***
just to check that the sum is one
*/
cmd( "set choice $norm" );
if(*choice==1)
{
 totnorm=0;
 for(i=0; i<num_bin; i++)
  {
   a=cl[i].lowb;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*M_PI*sigma));
   s=cl[i].highb;
   ap=exp(-(s-average)*(s-average)/(2*sigma))/(sqrt(2*M_PI*sigma));
   totnorm+=(b+ap)/2;
  }
  
 /*******/
 for(i=0; i<num_bin-1; i++)
  {
   a=cl[i].center;
   b=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*M_PI*sigma));
   b=b*tot/totnorm;
   b=300-((b-lminy)/(truemaxy-lminy))*300;    
   a=cl[i+1].center;  
   s=exp(-(a-average)*(a-average)/(2*sigma))/(sqrt(2*M_PI*sigma));
   s=s*tot/totnorm;
   s=300-((s-lminy)/(truemaxy-lminy))*300;  
   
   x1=40+(int)(600* (cl[i].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
   x2=40+(int)(600* (cl[i+1].center-cl[0].lowb)/(cl[num_bin-1].highb-cl[0].lowb));
 
   cmd( "$p create line %d %d %d %d",x1,(int)b, x2, (int)s );
  }
} 
    
cmd( "bind $p <Shift-1> {set ncanvas %d; set LX %%x; set LY %%y; set hereX %%X ; set hereY %%y; set choice 27}", cur_plot );
cmd( "bind $p <Control-1> { set ncanvas %d; set px %%x; set py %%y; set ax [expr $px+1]; set ay [expr $py+1]; .da.f.new%d.f.plots dtag selected; set cl [.da.f.new%d.f.plots create line $px $py $ax $ay]; .da.f.new%d.f.plots addtag selected withtag $cl; set choice 28 }", cur_plot, cur_plot, cur_plot, cur_plot );

type_plot[ cur_plot ] = HISTOGR;
plot_w[ cur_plot ] = 640;	// plot width
plot_l[ cur_plot ] = 325; //height of plot with labels
plot_nl[ cur_plot ] = 325; //height of plot with labels  
 
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


/***************************************************
CREATE_SERIES
****************************************************/
void create_series(int *choice)
{
int i, j, k, *start, *end, idseries, flt;
double nmax, nmin, nmean, nvar, nn, thflt, confi;
double step;
bool first;
char *lapp, **str, **tag;
store *app;

if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

double **data;

if(logs)
  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

Tcl_LinkVar(inter, "thflt", (char *) &thflt, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "confi", (char *) &confi, TCL_LINK_DOUBLE);

thflt=0;
confi=1.96;
cmd( "set flt 0" );
cmd( "set bido 1" );
cmd( "set bidi 1" );
cmd( "set vtag 1" );

cmd( "newtop .da.s \"New Series Options\" { set choice 2 } .da" );

cmd( "frame .da.s.o" );
cmd( "label .da.s.o.l -text \"Aggregation mode\"" );

cmd( "frame .da.s.o.r -relief groove -bd 2" );
cmd( "radiobutton .da.s.o.r.m -text \"Calculate over series (same # of cases)\" -variable bido -value 1" );
cmd( "radiobutton .da.s.o.r.f -text \"Calculate over cases (# cases = # of series)\" -variable bido -value 2" );
cmd( "pack .da.s.o.r.m .da.s.o.r.f -anchor w" );

cmd( "pack .da.s.o.l .da.s.o.r" );

cmd( "frame .da.s.f" );
cmd( "label .da.s.f.l -text \"Filtering\"" );

cmd( "frame .da.s.f.r -relief groove -bd 2" );
cmd( "radiobutton .da.s.f.r.n -text \"Use all values\" -variable flt -value 0 -command { .da.s.f.t.th configure -state disabled }" );
cmd( "radiobutton .da.s.f.r.s -text \"Ignore small values\" -variable flt -value 1 -command { .da.s.f.t.th configure -state normal }" );
cmd( "radiobutton .da.s.f.r.b -text \"Ignore large values\" -variable flt -value 2 -command { .da.s.f.t.th configure -state normal }" );
cmd( "pack .da.s.f.r.n .da.s.f.r.s .da.s.f.r.b  -anchor w" );

cmd( "frame .da.s.f.t" );
cmd( "label .da.s.f.t.l -text \"Threshold\"" );
cmd( "entry .da.s.f.t.th -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set thflt %%P; return 1 } { %%W delete 0 end; %%W insert 0 $thflt; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_disabled .da.s.f.th $thflt" ); 
cmd( "pack .da.s.f.t.l .da.s.f.t.th -side left -padx 2" );

cmd( "pack .da.s.f.l .da.s.f.r .da.s.f.t" );

cmd( "frame .da.s.i" );
cmd( "label .da.s.i.l -text \"Type of series\"" );

cmd( "frame .da.s.i.r -relief groove -bd 2" );
cmd( "radiobutton .da.s.i.r.m -text \"Average\" -variable bidi -command {set headname \"Avg\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 1 -command { .da.s.i.r.ci.p configure -state disabled }" );
cmd( "radiobutton .da.s.i.r.z -text \"Sum\" -variable bidi -command {set headname \"Sum\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 5 -command { .da.s.i.r.ci.p configure -state disabled }" );
cmd( "radiobutton .da.s.i.r.f -text \"Maximum\" -variable bidi -command {set headname \"Max\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 2 -command { .da.s.i.r.ci.p configure -state disabled }" );
cmd( "radiobutton .da.s.i.r.t -text \"Minimum\" -variable bidi -command {set headname \"Min\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 3 -command { .da.s.i.r.ci.p configure -state disabled }" );
cmd( "radiobutton .da.s.i.r.c -text \"Variance\" -variable bidi -command {set headname \"Var\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 4 -command { .da.s.i.r.ci.p configure -state disabled }" );

cmd( "frame .da.s.i.ci" );
cmd( "radiobutton .da.s.i.r.ci.c -text \"Std. Dev.\" -variable bidi -command {set headname \"CI\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 6 -command { .da.s.i.r.ci.p configure -state normal }" );
cmd( "label .da.s.i.r.ci.x -text \u00D7" );
cmd( "entry .da.s.i.r.ci.p -width 5 -validate focusout -vcmd { if [ string is double %%P ] { set confi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $confi; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( ".da.s.i.r.ci.p insert 0 $confi" ); 
cmd( "pack .da.s.i.r.ci.c .da.s.i.r.ci.x .da.s.i.ci.r.p -side left" );

cmd( "radiobutton .da.s.i.n -text \"Count\" -variable bidi -command {set headname \"Num\"; set vname $headname$basename; .da.s.nv selection range 0 end} -value 7 -command { .da.s.i.r.ci.p configure -state disabled }" );
cmd( "pack .da.s.i.l .da.s.i.m .da.s.i.z .da.s.i.f .da.s.i.t .da.s.i.c .da.s.i.ci .da.s.i.n -anchor w" );

cmd( "set a [.da.vars.ch.v get 0]" );
cmd( "set basename [lindex [split $a] 0]" );
cmd( "set headname \"Avg\"" );
cmd( "set vname $headname$basename" );

cmd( "frame .da.s.n" );
cmd( "label .da.s.n.lnv -text \"New series label\"" );
cmd( "entry .da.s.n.nv -width 30 -textvariable vname" );
cmd( "pack .da.s.n.lnv .da.s.n.nv" );

cmd( "frame .da.s.t" );
cmd( "label .da.s.t.tnv -text \"New series tag\"" );
cmd( "entry .da.s.t.tv -width 20 -textvariable vtag" );
cmd( "pack .da.s.t.tnv .da.s.t.tv" );

cmd( "pack .da.s.o .da.s.f .da.s.i .da.s.n .da.s.t -padx 5 -pady 5" );

cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#seq_xy } { set choice 2 }" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

cmd( "showtop .da.s centerW" );
cmd( "focus .da.s.n.nv" );
cmd( ".da.s.n.nv selection range 0 end" );
 
*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set thflt [ .da.s.f.th get ]" ); 
cmd( "set confi [ .da.s.i.ci.p get ]" ); 
Tcl_UnlinkVar(inter,"thflt");
Tcl_UnlinkVar(inter,"confi");
cmd( "destroytop .da.s" );

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

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  lapp=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,lapp);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
    data[ i ] = vs[ idseries ].data;
 }

if(autom_x||min_c>=max_c)
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

cmd( "set choice $flt" );
flt=*choice;

cmd( "set choice $bido" );

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

cmd( "set choice $bidi" );

for(i=min_c; i<=max_c; i++)
 {nmean=nn=nvar=0;
  first=true;
  for(j=0; j<nv; j++)
   {
    if(i>=start[j] && i<=end[j] && is_finite(data[j][i]) && (flt==0 || (flt==1 && data[j][i]>thflt) || (flt==2 && data[j][i]<thflt) ))		// ignore NaNs
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
cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var].label, vs[num_var].tag, min_c, max_c, num_var ); 

cmd( "lappend DaModElem %s", vs[num_var].label  );
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

cmd( "set choice $bidi" );

for(j=0; j<nv; j++)
 {nmean=nn=nvar=0;
  first=true;
  for(i=min_c; i<=max_c; i++)
   {
    if(i>=start[j] && i<=end[j] && is_finite(data[j][i]) && (flt==0 || (flt==1 && data[j][i]>thflt) || (flt==2 && data[j][i]<thflt) ) )
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

cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var].label, vs[num_var].tag, 0, nv-1, num_var ); 

cmd( "lappend DaModElem %s", vs[num_var].label  );
}
cmd( ".da.vars.lb.v see end" );
num_var++; 

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
int i, j, h, *start, *end, flt, idseries;
double xapp;
double step;

char *lapp, **str, **tag;
store *app;

if ( nv == 0 )
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

double **data;

if(logs)
  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

cmd( "newtop .da.s \"Moving Average Options\" { set choice 2 } .da" );

cmd( "frame .da.s.o" );
cmd( "label .da.s.o.l -text \"Number of (odd) periods\"" );
cmd( "set bido 10" );
cmd( "entry .da.s.o.th -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set bido %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bido; return 0 } } -invcmd { bell } -justify center" );
cmd( ".da.s.o.th insert 0 $bido" ); 
cmd( "pack .da.s.o.l .da.s.o.th" );

cmd( "pack .da.s.o -padx 5 -pady 5" );

cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp mdatares.html#create_maverag } { set choice 2 }" );

cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

cmd( "showtop .da.s centerW" );
cmd( "focus .da.s.o.th" );
cmd( ".da.s.o.th selection range 0 end" );

*choice=0;
  while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( "set bido [ .da.s.o.th get ]" ); 
cmd( "destroytop .da.s" );

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

if(autom_x)
 {min_c=1;
  max_c=num_c;
 }

cmd( "set choice $bido" );
flt=*choice;
if((flt%2)==0)
 flt++;

if(flt<2)
  {*choice=0;
  return;
 }


for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
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
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   {
   data[ i ] = vs[ idseries ].data;
   xapp=0;

   for(h=0, j=start[i]; j<start[i]+flt;j++)
	   if(is_finite(data[i][j]))		// not a NaN?
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
	 if(is_finite(data[i][j-(flt-1)/2]) && is_finite(data[i][j+(flt-1)/2]))
		xapp=xapp-data[i][j-(flt-1)/2]/(double)flt+data[i][j+(flt-1)/2]/(double)flt;
	 else
		xapp=NAN;
     vs[num_var+i].data[j]=xapp;
    }
   for(   ; j<end[i]; j++)
     vs[num_var+i].data[j]=xapp;     
   }
  cmd( ".da.vars.lb.v insert end \"%s %s (%d - %d) # %d (created)\"", vs[num_var+i].label, vs[num_var+i].tag, vs[num_var+i].start, vs[num_var+i].end, num_var+i ); 
  
  cmd( "lappend DaModElem %s", vs[num_var+i].label );
 }

cmd( ".da.vars.lb.v see end" );
num_var+=nv; 

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
int idseries, memstep;
char *app;
char **str, **tag;
char str1[100], delimiter[10], misval[10];
int i, j, *start, *end, typelab, numcol, del, fr, gp, type_res;
double **data, *dd, *ddstart;
const char *descr, *ext;
const char descrRes[] = "Lsd Result File";
const char descrTxt[] = "Text File";
const char extResZip[] = ".res.gz";
const char extTxtZip[] = ".txt.gz";
const char extRes[] = ".res";
const char extTxt[] = ".txt";
FILE *fsave;
#ifdef LIBZ
gzFile fsavez;
#else
FILE *fsavez; 
#endif 

if ( nv == 0 )			// no variables selected
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

if(logs)
  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

data=new double *[nv];
start=new int[nv];
end=new int[nv];
str=new char *[nv];
tag=new char *[nv];

max_c=min_c=0;

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  app=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,app);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  data[ i ] = vs[ idseries ].data;

  if(max_c<end[i])
   max_c=end[i];
 }

Tcl_LinkVar(inter, "dozip", (char *)&dozip, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "typelab", (char *) &typelab, TCL_LINK_INT);
Tcl_LinkVar(inter, "deli", (char *) &del, TCL_LINK_INT);
Tcl_LinkVar(inter, "numcol", (char *) &numcol, TCL_LINK_INT);
Tcl_LinkVar(inter, "fr", (char *) &fr, TCL_LINK_INT);

//Variables' Name in first column
strcpy(misval,nonavail);
fr=1;
typelab=3;
cmd( "newtop .da.lab \"Data Save Options\" { set choice 2 } .da" );

cmd( "label .da.lab.l -text \"File format\"" );

cmd( "frame .da.lab.f -relief groove -bd 2" );
cmd( "radiobutton .da.lab.f.lsd -text \"Lsd results file\" -variable typelab -value 3" );
cmd( "radiobutton .da.lab.f.nolsd -text \"Text file\" -variable typelab -value 4" );
cmd( "pack .da.lab.f.lsd .da.lab.f.nolsd -anchor w" );

cmd( "checkbutton .da.lab.dozip -text \"Generate zipped file\" -variable dozip" );

#ifdef LIBZ
cmd( "pack .da.lab.l .da.lab.f .da.lab.dozip -padx 5 -pady 5" );
#else
cmd( "pack .da.lab.l .da.lab.f -padx 5 -pady 5" );
#endif

cmd( "okhelpcancel .da.lab b { set choice 1 } { LsdHelp mdatares.html#save } { set choice 2 }" );

cmd( "bind .da.lab <Return> {.da.lab.ok invoke}" );
cmd( "bind .da.lab <Escape> {.da.lab.esc invoke}" );

cmd( "showtop .da.lab centerW" );

*choice=0;
while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==2)
 goto end;
type_res=typelab;

*choice=0;
cmd( "destroytop .da.lab" );

if(typelab==4)
{
cmd( "newtop .da.lab \"Data Save Options\" { set choice 2 } .da" );

cmd( "frame .da.lab.f" );
cmd( "label .da.lab.f.tit -text \"Labels to use\"" );

cmd( "frame .da.lab.f.r -relief groove -bd 2" );
typelab=1;
cmd( "radiobutton .da.lab.f.r.orig -text Original -variable typelab -value 1 -command { .da.lab.f.en configure -state disabled }" );
cmd( "radiobutton .da.lab.f.r.new -text \"New names\" -variable typelab -value 2 -command { .da.lab.f.en configure -state normal }" );
cmd( "pack .da.lab.f.r.orig .da.lab.f.r.new -anchor w" );

cmd( "set newlab \"\"" );
cmd( "entry .da.lab.f.en -width 20 -textvariable newlab -state disabled" );
cmd( "set gp 0" );
cmd( "checkbutton .da.lab.f.gp -text \"Add #\" -variable gp" );
cmd( "bind .da.lab.f.en <FocusIn> {.da.lab.f.new invoke}" );
cmd( "pack .da.lab.f.tit .da.lab.f.r .da.lab.f.en .da.lab.f.gp" );

cmd( "frame .da.lab.d" );
cmd( "label .da.lab.d.tit -text \"Columns delimiter\"" );
cmd( "frame .da.lab.d.r -relief groove -bd 2" );
del=1;
cmd( "radiobutton .da.lab.d.r.tab -text \"Tab delimited\" -variable deli -value 1 -command { .da.lab.d.r.del configure -state disabled; .da.lab.d.r.ecol configure -state disabled }" );
cmd( "radiobutton .da.lab.d.r.oth -text \"Other delimiter\" -variable deli -value 2 -command { .da.lab.d.r.del configure -state normal; .da.lab.d.r.ecol configure -state disabled }" );
cmd( "set delimiter \"\"" );
cmd( "entry -width 3 .da.lab.d.r.del -textvariable delimiter -justify center -state disabled" );
cmd( "bind .da.lab.d.r.del <FocusIn> {.da.lab.d.r.oth invoke}" );
cmd( "radiobutton .da.lab.d.r.col -text \"Fixed length columns\" -variable deli -value 3 -command { .da.lab.d.r.del configure -state disabled; .da.lab.d.r.ecol configure -state normal }" );
numcol=16;
cmd( "entry .da.lab.d.r.ecol -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set numcol %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numcol; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_disabled .da.lab.d.r.ecol $numcol" ); 
cmd( "bind .da.lab.d.r.ecol <FocusIn> {.da.lab.d.r.col invoke}" );
cmd( "pack .da.lab.d.r.tab .da.lab.d.r.oth .da.lab.d.r.del .da.lab.d.r.col .da.lab.d.r.ecol -anchor w" );

cmd( "pack .da.lab.d.tit .da.lab.d.r" );

cmd( "frame .da.lab.gen" );
cmd( "label .da.lab.gen.miss -text \"Missing values\"" );
cmd( "set misval \"n/a\"" );
cmd( "entry .da.lab.gen.mis_val -width 5 -textvariable misval -justify center" );
cmd( "pack .da.lab.gen.miss .da.lab.gen.mis_val" );

cmd( "checkbutton .da.lab.fr -text \"Names in first row\" -variable fr" );

cmd( "pack .da.lab.f .da.lab.d .da.lab.gen .da.lab.fr -padx 5 -pady 5" );

cmd( "okhelpcancel .da.lab b { set choice 1 } { LsdHelp mdatares.html#save } { set choice 2 }" );

cmd( "bind .da.lab <KeyPress-Return> {.da.lab.ok invoke}" );

cmd( "showtop .da.lab centerW" );

*choice=0;
while(*choice==0)
 Tcl_DoOneEvent(0);

cmd( "set numcol [ .da.lab.d.r.ecol get ]" ); 

if(*choice==2)
 goto end;

cmd( "set choice $gp" );
gp=*choice;

*choice=0;

app=(char *)Tcl_GetVar(inter, "misval", 0);
strcpy(misval,app);
}

if ( type_res == 4 )
{
	descr = descrTxt;
	if ( ! dozip )
		ext = extTxt;
	else
		ext = extTxtZip;
}
else
{
	descr = descrRes;
	if ( ! dozip )
		ext = extRes;
	else
		ext = extResZip;
}

cmd( "set bah [tk_getSaveFile -parent .da -title \"Save Data File\" -initialdir [pwd] -defaultextension \"%s\" -filetypes {{{%s} {%s}} {{All files} {*}} }]", ext, descr, ext );
app=(char *)Tcl_GetVar(inter, "bah",0);
strcpy(msg, app);

if(strlen(msg)==0)
 goto end;
if(dozip==1) 
 {
 #ifdef LIBZ
  fsavez=gzopen(msg, "wt");
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
#ifdef LIBZ
for(j=min_c; j<=max_c; j++)
 {
  for(i=0; i<nv; i++)
   {if(j>=start[i] && j<=end[i] && !is_nan(data[i][j]))		// write NaN as n/a
      gzprintf(fsavez, "%g%s", data[i][j], delimiter);
    else
      gzprintf(fsavez, "%s%s", misval, delimiter);
      
   }
  gzprintf(fsavez,"\n");
 }
#endif 
}
else
{
for(j=min_c; j<=max_c; j++)
 {for(i=0; i<nv; i++)
   {if(j>=start[i] && j<=end[i] && !is_nan(data[i][j]))		// write NaN as n/a
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
   if(j>=start[i] && j<=end[i] && !is_nan(data[i][j]))		// write NaN as n/a
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
cmd( "destroytop .da.lab" );
Tcl_UnlinkVar(inter, "typelab");
Tcl_UnlinkVar(inter, "dozip");
Tcl_UnlinkVar(inter, "numcol");
Tcl_UnlinkVar(inter, "deli");
Tcl_UnlinkVar(inter, "fr");

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


/************************
 PLOG_SERIES
 ************************/
void plog_series(int *choice)
{
int i, j, k, *start, *end, idseries, flt;
double nmax, nmin, nmean, nvar, nn, thflt;
double step;

char *lapp, **str, **tag;
store *app;

if ( nv == 0 )			// no variables selected
{
	cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
	return;
}

double **data;

if(logs)
  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

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

for(i=0; i<nv; i++)
 {str[i]=new char[MAX_ELEM_LENGTH];
  tag[i]=new char[MAX_ELEM_LENGTH];

  cmd( "set res [.da.vars.ch.v get %d]", i );
  lapp=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(msg,lapp);
  sscanf(msg, "%s %s (%d - %d) # %d", str[i], tag[i], &start[i], &end[i], &idseries);
  if(autom_x ||(start[i]<=max_c && end[i]>=min_c))
   data[ i ] = vs[ idseries ].data;
 }

if(autom_x||min_c>=max_c)
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

plog( "\n\nTime series data\n" );
plog( "t\t%s_%s", "series", str[0], tag[0]);

for(i=1; i<nv; i++)
  plog( "\t%s_%s", "series", str[i], tag[i]);
plog( "\n" );

for(i=min_c; i<=max_c; i++)
 {
 if ( ! is_nan( data[0][i] ) && start[0] <= i )
	plog( "%d\t%.*g", "series", i, pdigits, data[0][i] );
 else
	plog( "%d\t%s", "series", i, nonavail );		// write NaN as n/a

 for(j=1; j<nv; j++)
   {
   if ( ! is_nan( data[j][i] ) && start[j] <= i )
	plog( "\t%.*g", "series", pdigits, data[j][i] );
   else
	plog( "\t%s", "series", nonavail );		// write NaN as n/a
   }
 plog( "\n" );
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


/*******************************************************
 MIN_HBORDER
 
 calculate horizontal borders required for legends 
 *******************************************************/
int min_hborder( int *choice, int pdigits, double miny, double maxy )
{
	int vticks, largest = 0;
	
	cmd( "set choice $vticksP" );
	vticks = *choice;
	
	for ( int i = 0; i < vticks + 2; ++i ) // search for the longest legend
	{
		cmd( "set choice [ font measure $fontP \"%.*g\" ]", pdigits, miny + ( vticks + 1 - i ) * ( maxy - miny ) / ( vticks + 1 ) );
		if ( *choice > largest )
			largest = *choice;
	}
	
	return largest;
}

 
 /*****************************************
 PLOT
 
 Effectively create the plot in canvas
 *****************************************/
void plot( int type, int nv, double **data, int *start, int *end, char **str, char **tag, int *choice )
{
	// create canvas for the plot
	int h, i, j, k, color, hsize, vsize, hbordsize, tbordsize, bbordsize, sbordsize, htmargin, vtmargin, hticks, vticks, lheight, hcanvas, vcanvas, hscroll, vscroll, *htick, *vtick, nLine, endCase, iniCase;
	char *txtCase, *txtLine, txtLab[ 2 * MAX_ELEM_LENGTH ];
	double x1, x2, *y, yVal, cminy, cmaxy, step;
	bool first, tOk, y2on = ( type == TSERIES ) ? ( num_y2 <= nv ? true : false ) : false;
	
	// define type-specific parameters
	switch ( type )
	{
		case TSERIES:
			txtCase = ( char * ) "Case number";
			txtLine = ( char * ) "Series";
			nLine = nv;
			iniCase = min_c;
			endCase = max_c;
			break;
			
		case CRSSECT:
			txtCase = ( char * ) "Series";
			txtLine = ( char * ) "Cross-section";
			nLine = *end;
			iniCase = 0;
			endCase = nv - 1;
			break;
	}
	
	// get graphical configuration from Tk (file defaults.tcl)
	get_int( "hsizeP", & hsize );			// 600
	get_int( "vsizeP", & vsize );			// 300
	get_int( "hmbordsizeP", & hbordsize );	// 40
	get_int( "tbordsizeP", & tbordsize );	// 5
	get_int( "bbordsizeP", & bbordsize );	// 90
	get_int( "sbordsizeP", & sbordsize );	// 0
	get_int( "htmarginP", & htmargin );		// 4
	get_int( "vtmarginP", & vtmargin );		// 5
	get_int( "hticksP", & hticks );			// 3
	get_int( "vticksP", & vticks );			// 3
	get_int( "lheightP", & lheight );		// 15

	// calculate horizontal borders required for legends
	h = min_hborder( choice, pdigits, miny, maxy );
	if ( y2on )								// repeat for 2nd y axis
		h = max( k, min_hborder( choice, pdigits, miny2, maxy2 ) );
		
	// include margins and tick size (if present)
	hbordsize = max( hbordsize, h ) + 2 * htmargin + ( grid ? 0 : 5 );
	cmd( "set hbordsizeP %d", hbordsize );
	
	// initial canvas size
	hcanvas = hsize + hbordsize * ( y2on ? 2 : 1 ) + 
				( y2on ? 0 : ( max_c < 1000 ? 10 : 20 ) );
	vcanvas = vsize + tbordsize + bbordsize;
	hscroll = hcanvas + sbordsize;
	vscroll = vcanvas + sbordsize;
	
	// create plot window & canvas
	cmd( "set w .da.f.new%d", cur_plot );	// plot window
	cmd( "set p $w.f.plots" );				// plot canvas
	
	cmd( "newtop $w $tit \"wm withdraw $w\" \"\"" );

	cmd( "frame $w.f" );
	cmd( "scrollbar $w.f.vs -command \"$p yview\"" );
	cmd( "scrollbar $w.f.hs -orient horiz -command \"$p xview\"" );
	cmd( "pack $w.f.vs -side right -fill y" );
	cmd( "pack $w.f.hs -side bottom -fill x" );

	cmd( "canvas $p -width %d -height %d -background white -relief flat -yscrollcommand \"$w.f.vs set\" -xscrollcommand \"$w.f.hs set\" -scrollregion \"%d %d %d %d\" -relief flat -highlightthickness 0", 
		hcanvas, vcanvas, - sbordsize, - sbordsize, hcanvas + sbordsize, vcanvas + sbordsize  );
	cmd( "pack $p -expand yes -fill both" );
	cmd( "pack $w.f -expand yes -fill both" );
	
	// add buttons bottom bar
	cmd( "frame $w.b" );

	cmd( "frame $w.b.c" );

	cmd( "frame $w.b.c.case" );
	cmd( "label $w.b.c.case.l -text \"%s:\" -width 11 -anchor e", txtCase );
	cmd( "label $w.b.c.case.v -text \"\" -fg red -width 20 -anchor w" );
	cmd( "pack $w.b.c.case.l $w.b.c.case.v -side left -anchor w" );

	cmd( "frame $w.b.c.y" );
	if( logs )
		cmd( "label $w.b.c.y.l -text \"log(Y) value:\" -width 11 -anchor e" );
	else
		cmd( "label $w.b.c.y.l -text \"Y value:\" -width 11 -anchor e" );
	cmd( "label $w.b.c.y.v1 -text \"\" -fg red -width 10 -anchor w" );
	cmd( "label $w.b.c.y.v2 -text \"\" -fg red -width 10 -anchor w" );
	cmd( "pack $w.b.c.y.l $w.b.c.y.v1 $w.b.c.y.v2 -side left -anchor w");

	cmd( "frame $w.b.c.var" );
	cmd( "label $w.b.c.var.l -text \"%s:\" -width 11 -anchor e", txtLine );
	cmd( "label $w.b.c.var.v -text \"\" -fg red -width 20 -anchor w" );
	cmd( "pack $w.b.c.var.l $w.b.c.var.v -side left -anchor w" );

	cmd( "pack $w.b.c.case $w.b.c.y $w.b.c.var -anchor w" );

	cmd( "frame $w.b.o" );
	cmd( "label $w.b.o.l1 -text \"Right-click: edit properties\"" );
	cmd( "label $w.b.o.l2 -text \"Shift-click: insert text\"" );
	cmd( "label $w.b.o.l3 -text \"Ctrl-click: insert line\"" );
	cmd( "pack $w.b.o.l1 $w.b.o.l2 $w.b.o.l3" );

	cmd( "frame $w.b.s" );
	cmd( "button $w.b.s.save -width 9 -text Save -command { set it \"$cur_plot) $tit\"; set fromPlot true; set choice 11 } -state disabled -underline 0" );
	cmd( "button $w.b.s.stop -width 9 -text Stop -command { set choice 2 } -state disabled -underline 0" );
	cmd( "pack $w.b.s.save $w.b.s.stop -pady 5" );

	cmd( "label $w.b.pad -width 6" );

	cmd( "frame $w.b.z -bd 2 -relief groove" );
	cmd( "label $w.b.z.l -text Zoom" );

	cmd( "frame $w.b.z.b" );
	cmd( "button $w.b.z.b.p -width 3 -text + -command { scale_canvas .da.f.new%d.f.plots \"+\" zoomLevel%d } -state disabled", cur_plot, cur_plot );
	cmd( "button $w.b.z.b.m -width 3 -text - -command { scale_canvas .da.f.new%d.f.plots \"-\" zoomLevel%d } -state disabled", cur_plot, cur_plot  );
	cmd( "pack $w.b.z.b.p $w.b.z.b.m" );

	cmd( "pack  $w.b.z.l $w.b.z.b -side left -padx 2 -pady 2" );

	cmd( "pack $w.b.c $w.b.o $w.b.pad $w.b.s $w.b.z -expand no -padx 10 -pady 5 -side left" );
	cmd( "pack $w.b -side right -expand no" );

	cmd( "mouse_wheel $p" );
	
	if ( watch )
	{	
		cmd( "$w.b.s.stop configure -state normal" );
		cmd( "bind $w <s> { $w.b.s.stop invoke }; bind $w <S> { $w.b.s.stop invoke }" );
		cmd( "bind $w <Escape> { $w.b.s.stop invoke }" );
	}

	cmd( "showtop $w current yes yes no" );
	cmd( "$p xview moveto 0; $p yview moveto 0");
	cmd( "set zoomLevel%d 1.0", cur_plot );
	
	// create list with the series names+tags in Tk
	cmd( "set series%d [ list ]", cur_plot );
	for ( i = 0; i < nv; ++i )
		cmd( "lappend series%d \"%s_%s\"", cur_plot, str[ i ], tag[ i ] );
		
	// axis lines, ticks & grid
	cmd( "canvas_axis $p %d %d %d", type, grid, y2on );

	// x-axis values
	switch ( type )
	{
		case TSERIES:
			for ( i = 0; i < hticks + 2; ++i )
				cmd( "$p create text %d %d -font $fontP -anchor n -text %d -tag { p text }", hbordsize + ( int ) round( i * ( double ) hsize / ( hticks + 1 ) ), vsize + lheight - 3, min_c + ( int ) floor( i * ( double ) ( max_c - min_c ) / ( hticks + 1 ) ) );
			break;
			
		case CRSSECT:
			cmd( "$p create text %d %d -font $fontP -anchor nw -text \"%d series\" -tag { p text }", hbordsize, vsize + lheight - 3, nv );
			break;
	}

	// y-axis values
	for ( i = 0; i < vticks + 2; ++i )
	{
		cmd( "$p create text %d %d -font $fontP -anchor e -text %.*g -tag { p text }", hbordsize - htmargin - ( grid ? 0 : 5), tbordsize + ( int ) round( i * ( double ) vsize / ( vticks + 1 ) ), pdigits, miny + ( vticks + 1 - i ) * ( maxy - miny ) / ( vticks + 1 ) );
		
		// second y-axis series values (if any)
		if ( y2on )
			cmd( "$p create text %d %d -font $fontP -anchor w -text %.*g -tag { p text }", hbordsize + hsize + htmargin + ( grid ? 0 : 5), tbordsize + ( int ) round( i * ( double ) vsize / ( vticks + 1 ) ), pdigits, miny2 + ( vticks + 1 - i ) * ( maxy2 - miny2 ) / ( vticks + 1 ) );
	}

	// series labels
	cmd( "set xlabel $htmarginP" ); 
	cmd( "set ylabel [ expr $tbordsizeP + $vsizeP + 2 * $lheightP ]" );
	color = allblack ? 1001 : 0;		// select gray scale or color range
	
	for ( i = 0; i < nLine; ++i, ++color )
	{
		switch ( type )
		{
			case TSERIES:
				if ( start[ i ] <= max_c && end[ i ] >= min_c )
					tOk = true;
				else
					tOk = false;
				sprintf( txtLab, "%s_%s", str[ i ], tag[ i ] );
				break;
			case CRSSECT:
				sprintf( txtLab, "t = %d ", start[ i ] );
				tOk = true;
				break;
		}
		
		if ( tOk )
		{
			cmd( "set app [ font measure $fontP \"%s\"]", txtLab );
			cmd( "if { [expr $xlabel + $app] > %d } { set xlabel %d; incr ylabel %d }", hcanvas - htmargin, htmargin, lheight );
			get_int( "ylabel", & k );
			if ( k > tbordsize + vsize + bbordsize - 2 * lheight )
				break;

			cmd( "$p create text $xlabel $ylabel -font $fontP -anchor nw -text \"%s\" -tag { txt%d text legend } -fill $c%d", txtLab, i, ( color < 1100 ) ? color : 0 );
			cmd( "set xlabel [ expr $xlabel + $app + $htmarginP ]" );
		}
	}

	if ( i < nLine )
		cmd( "$p create text $xlabel $ylabel -font $fontP -anchor nw -text (more...)" );

	cmd( "update" );

	// create data structure to hold image to be presented
	y = new double[ nLine ];		// cross section consolidated "intra" step average value
	int pdataX[ hsize + 1 ];		// visible time step values (max)
	for ( i = 0; i < hsize + 1; ++i )
		pdataX[ i ] = -1;			// mark all as non plot
	int **pdataY = new int *[ nLine ];	// matrix of visual average scaled/rounded values
	for ( k = 0; k < nLine; ++k )
	{
		pdataY[ k ] = new int[ hsize + 1 ];
		for ( i = 0; i < hsize + 1; ++i )
			pdataY[ k ][ i ] = -1;	// mark all as non plot
	}
	
	// calculate screen plot values for all series
	x1 = hbordsize - 1;
	step = hsize / ( double ) ( endCase - iniCase );
	for ( h = 0, j = 0, i = iniCase; i <= endCase; ++i )
	{
		// move the x-axis pointer in discrete steps
		x2 = x1 + ( 1 + h ) * step;
		h++;            	// counter for the average when many values occupy one x point
		
		// fix initial x point if scale is too coarse
		if ( i == iniCase && x2 - x1 > 1 )
			x2 = x1 + 1;
		
		// moved to another canvas step?
		bool xnext = ( floor( x1 ) != floor ( x2 ) );

		// process each curve (series or cross-section)
		for ( k = 0; k < nLine; ++k, ++color )	
		{
			switch ( type )
			{
				case TSERIES:
					yVal = data[ k ][ i ];
					if ( start[ k ] < i && end[ k ] >= i )
						tOk = true;
					else
						tOk = false;
					break;
					
				case CRSSECT:
					yVal = data[ i ][ k ];
					tOk = true;
					break;
			}

			// pick the right limits according to the y-axis of series
			if ( ! y2on || k < num_y2 - 1 )
			{
				cminy = miny;
				cmaxy = maxy;
			}
			else
			{
				cminy = miny2;
				cmaxy = maxy2;
			}
		  
			if ( is_finite( yVal ) )
			{
				if ( tOk )
				{
					y[ k ] += yVal;
					if ( xnext )
					{	// average "intra" steps
						y[ k ] /= h;		
						// constrain to canvas virtual limits
						y[ k ] = min( max( y[ k ], cminy ), cmaxy );
						// scale to the canvas physical y range
						y[ k ] = round( tbordsize + vsize * ( 1 - ( y[ k ] - cminy ) / ( cmaxy - cminy ) ) );
						// save to visual vertical line buffer
						pdataY[ k ][ j ] = round( y[ k ] );
						// restart averaging
						y[ k ] = 0;
					}
				}
				else
				{
					if ( start[ k ] == i )
					{ 	//series entering after x1
						if ( ! xnext )					// "intra" step?
							y[ k ] = yVal * h;			// suppose from the beginning of x1
						else
						{	// just plot as usual
							y[ k ] = yVal;
							y[ k ] = min( max( y[ k ], cminy ), cmaxy );
							y[ k ] = round( tbordsize + vsize * ( 1 - ( y[ k ] - cminy ) / ( cmaxy - cminy ) ) );
							pdataY[ k ][ j ] = round( y[ k ] );
							y[ k ] = 0;
						}
					}
				}
			}
		}
		
		if ( xnext )
		{ 
			x1 = x2;
			h = 0;			// restart averaging (more than 1 step per canvas step)
			pdataX[ j ] = floor( x2 );
			if ( ++j > hsize )	// buffer full?
				break;
		}
	}

	// transfer data to Tcl and plot it
	cdata = pdataX;					// send series x values to Tcl
	cmd( "get_series %d pdataX", j );
	
	color = allblack ? 1001 : 0;	// select gray scale or color range
		
	*choice = 0;
	for ( k = 0; k < nLine; ++k, ++color )
	{
		cdata = pdataY[ k ];		// send series y values to Tcl
		cmd( "get_series %d pdataY", j );
		
		int colorPlot = ( color < 1100 ) ? color : 0;	// use black if out of colors

		if( line_point == 1 )
			cmd( "plot_line $p $pdataX $pdataY p%d $c%d %lf", k, colorPlot, point_size );
		else
			cmd( "plot_points $p $pdataX $pdataY p%d $c%d %lf", k, colorPlot, point_size );
		
		if ( watch )
			cmd( "update" );
		if	( *choice == 2 ) 		// button STOP pressed
			break;  
	}
	
	// garbage collection
	cmd( "unset pdataX pdataY");
	for ( i = 0; i < nLine; ++i )
		delete [] pdataY[ i ];
	delete [] pdataY;
	delete [] y;
	
	if	( *choice == 2 ) 			// Button STOP pressed
	{
		cmd( "destroytop $w" );
		return;
	}
	
	// enable/disable buttons
	cmd( "$w.b.s.save configure -state normal" );
	cmd( "$w.b.s.stop configure -state disabled" );
	cmd( "$w.b.z.b.p configure -state normal" );
	cmd( "$w.b.z.b.m configure -state normal" );

	// raise axis, legends & draws to the front and lower grid to the back
	cmd( "$p raise p" );
	cmd( "$p lower g" );
	cmd( "if { [ $p find withtag draw ] != \"\" } { $p raise draw }" );
	
	// update cursor indicators at bottom window
	if ( y2on )
		cmd( "bind $p <Motion> { \
				set zoom $zoomLevel%d; \
				set w .da.f.new%d; \
				set llim [ expr %d * $zoom ]; \
				set rlim [ expr %d * $zoom ]; \
				set tlim [ expr %d * $zoom ]; \
				set blim [ expr %d * $zoom ]; \
				set cx [ $w.f.plots canvasx %%x ]; \
				set cy [ $w.f.plots canvasy %%y ]; \
				if { $cx >= $llim && $cx <= $rlim && $cy >= $tlim && $cy <= $blim } { \
					$w.b.c.case.v configure -text [ expr int( ( $cx - $llim ) * ( %d - %d ) / ( $rlim - $llim ) + %d ) ]; \
					$w.b.c.y.v1 configure -text [ format \"%%%%.[ expr $pdigits ]g\" [ expr ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf ] ]; \
					$w.b.c.y.v2 configure -text [ format \"%%%%.[ expr $pdigits ]g\" [ expr ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf ] ] \
				} \
			}", cur_plot, cur_plot, hbordsize, hbordsize + hsize, tbordsize, tbordsize + vsize, endCase, iniCase, iniCase, maxy, miny, miny, maxy2, miny2, miny2 );
	else
		cmd( "bind $p <Motion> { \
				set type %d; \
				set zoom $zoomLevel%d; \
				set series $series%d; \
				set w .da.f.new%d; \
				set llim [ expr %d * $zoom ]; \
				set rlim [ expr %d * $zoom ]; \
				set tlim [ expr %d * $zoom ]; \
				set blim [ expr %d * $zoom ]; \
				set cx [ $w.f.plots canvasx %%x ]; \
				set cy [ $w.f.plots canvasy %%y ]; \
				if { $cx >= $llim && $cx <= $rlim && $cy >= $tlim && $cy <= $blim } { \
					set case  [ expr int( ( $cx - $llim ) * ( %d - %d ) / ( $rlim - $llim ) + %d ) ]; \
					if { $type == 0 } { \
						$w.b.c.case.v configure -text $case \
					} elseif { $type == 1 } { \
						$w.b.c.case.v configure -text \"[ lindex $series $case ] (#[ expr $case + 1 ])\" \
					}; \
					$w.b.c.y.v1 configure -text [ format \"%%%%.[ expr $pdigits ]g\" [ expr ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf ] ]; \
				} \
			}", type, cur_plot, cur_plot, cur_plot, hbordsize, hbordsize + hsize, tbordsize, tbordsize + vsize, endCase, iniCase, iniCase, maxy, miny, miny );

	for ( i = 0; i < nLine; ++i )
	{
		switch ( type )
		{
			case TSERIES:
				sprintf( txtLab, "%s_%s", str[ i ], tag[ i ] );
				break;
			case CRSSECT:
				sprintf( txtLab, "t = %d ", start[ i ] );
				break;
		}
			
		cmd( "$p bind p%d <Enter> { .da.f.new%d.b.c.var.v configure -text \"%s\" }", i, cur_plot, txtLab );
		cmd( "$p bind p%d <Leave> { .da.f.new%d.b.c.var.v configure -text \"\" }", i, cur_plot );

		cmd( "$p bind txt%d <Enter> { .da.f.new%d.b.c.var.v configure -text \"%s\" }", i,cur_plot, txtLab );
		cmd( "$p bind txt%d <Leave> { .da.f.new%d.b.c.var.v configure -text \"\" }", i, cur_plot );
	}

	// window bindings (return to Analysis, insert text, insert line)
	cmd( "bind $w <Escape> \"wm withdraw $w\"" );
	cmd( "bind $w <s> { $w.b.s.save invoke }; bind $w <S> { $w.b.s.save invoke }" );
	cmd( "bind $w <plus> { $w.b.z.b.p invoke }" );
	cmd( "bind $w <minus> { $w.b.z.b.m invoke }" );
	cmd( "bind $p <Double-Button-1> { raise .da }" );
	
	cmd( "bind $p <Shift-1> { \
			set ccanvas .da.f.new%d.f.plots; \
			set LX %%X; \
			set LY %%Y; \
			set hereX [ $ccanvas canvasx %%x ]; \
			set hereY [ $ccanvas canvasy %%y ]; \
			set choice 27 \
		}", cur_plot );
		
	cmd( "bind $p <Control-1> { \
			set ccanvas .da.f.new%d.f.plots; \
			set ncanvas %d; \
			set hereX [ $ccanvas canvasx %%x ]; \
			set hereY [ $ccanvas canvasy %%y ]; \
			unset -nocomplain cl; \
			set choice 28 \
		}", cur_plot, cur_plot );

	// moving and editing lines and text
	cmd( "bind $p <Button-1> { \
			set ccanvas .da.f.new%d.f.plots; \
			set type [ $ccanvas gettags current ]; \
			if { [ lsearch -regexp $type (draw|legend) ] >= 0 } { \
				set moving true; \
				set hereX [ $ccanvas canvasx %%x ]; \
				set hereY [ $ccanvas canvasy %%y ]; \
				$ccanvas raise current \
			} { \
				set moving false \
			} \
		}", cur_plot );
	cmd( "bind $p <B1-Motion> { \
			set ccanvas .da.f.new%d.f.plots; \
			if $moving { \
				$ccanvas move current [ expr [ $ccanvas canvasx %%x ] - $hereX ] \
					[ expr [ $ccanvas canvasy %%y ] - $hereY ]; \
				set hereX [ $ccanvas canvasx %%x ]; \
				set hereY [ $ccanvas canvasy %%y ] \
			} \
		}", cur_plot );
	cmd( "bind $p <ButtonRelease-1> { set moving false }" );
	
	cmd( "bind $p <Button-2> { \
			set ccanvas .da.f.new%d.f.plots; \
			set LX %%X; set LY %%Y; \
			set type [ $ccanvas gettags current ]; \
			if { [ lsearch $type line ] >= 0 || [ lsearch $type dots ] >= 0 } { \
				if { [ lsearch $type series ] >= 0 } { \
					set cline [ lindex $type 0 ]; \
					set draw false \
				} { \
					set cline current; \
					set draw true \
				}; \
				set choice 31 \
			} { \
				if { [ lsearch $type text ] >= 0 } { \
					set choice 26 \
				} \
			} \
		}", cur_plot );
	cmd( "bind $p <Button-3> { event generate .da.f.new%d.f.plots <Button-2> -x %%x -y %%y }", cur_plot );
	
	// save plot info
	type_plot[ cur_plot ] = type; 					// plot type
	plot_w[ cur_plot ] = hcanvas;					// plot width
	plot_nl[ cur_plot ] = tbordsize + vsize + lheight ; // height of plot without labels  
	cmd( "set choice $ylabel" );
	plot_l[ cur_plot ] = *choice + lheight; 		// height of plot with labels

	*choice = 0;
}

/*****************************
 TCL_UPLOAD_SERIES
 
 data transfer routine from C to Tcl 
 *****************************/
int Tcl_upload_series( ClientData cd, Tcl_Interp *inter, int oc, Tcl_Obj *CONST ov[] )
{
	int size, *data;

	if ( oc != 3 ) 
	{
		Tcl_WrongNumArgs( inter, 1, ov, "size data");
		return TCL_ERROR;
	}

	if ( Tcl_GetIntFromObj( inter, ov[ 1 ], &size ) != TCL_OK )
		return TCL_ERROR;
	
	data = ( int * ) Tcl_GetByteArrayFromObj( ov[ 2 ], NULL );
	if ( data == NULL )
		return TCL_ERROR;

	Tcl_InvalidateStringRep( ov[ 2 ] );

	for ( int i = 0; i < size; ++i )
		data[ i ] = cdata[ i ];

	return TCL_OK;
}