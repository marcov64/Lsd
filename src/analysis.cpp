/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/*************************************************************
ANALYSIS.CPP 
Contains the routines to manage the data analysis module.

The main functions contained here are:

- void analysis( int *choice )
Makes an initialization and then there is the main cycle in
read_data

- void read_data( int *choice );                            
Builds the managemen window, setting all the bindings, and enters in a cycle
from which can make several choices:

1) Plot the plot as indicated
2) Exit from data analysis
3) Brings in foreground the plot on which title it was double clicked
4) Load a new data file
5) Sort the labels of the variables
6) Copy the selection from the list of variables to the list of the variables
to plot
7) delete the variables selected in the list of the variables to plot
9) plot the cross-section plot

- void plot_tseries( int *choice );
Plot the plot with the variable indicated in the list of the variables
to plot. If the option says to use the automatic y scaling, it makes first
a scan of the file to spot the maximum and minimum y, then it plots. Otherwise,
it uses directly the values set by the users.

- void plot_cross( void ) ;
Plot a plot of cross section data. The variables chosen are plotted along the times
steps chosen and, on request, they are sorted (descending or ascending) one one of
time steps. To choose which time to sort on, double-click on the time step.

- void set_cs_data( int *choice );
Interface used to determine the time steps to plot in the cross-section plots
and the possible sorting.

- void sort_cs_asc( char **s, double **v, int nv, int nt, int c );
Sort in ascending order the variables. Used in plot_cross.

- void sort_cs_desc( char **s, double **v, int nv, int nt, int c );

Sort in descending order the variables. Used in plot_cross.
*************************************************************/

/* 
used case 44
*/

#include "decl.h"

// plot types
#define TSERIES	0
#define CRSSECT	1
#define GNUPLOT	2
#define LATTICE	3
#define HISTOGR	4
#define HISTOCS	5

bool avgSmplMsg;
char filename[ MAX_PATH_LENGTH ];
double maxy, maxy2;
double miny, miny2;
double point_size;
int allblack;
int autom;
int autom_x;
int avgSmpl;
int *cdata;
int cur_plot;
int dir;
int file_counter;
int grid;
int gnu;
int line_point;
int logs;
int max_c;
int min_c;
int num_c;
int num_var;
int num_y2;
int nv;
int pdigits;
int plot_l[ MAX_PLOTS ];
int plot_nl[ MAX_PLOTS ];
int plot_w[ MAX_PLOTS ];
int res;
int time_cross;
int type_plot[ MAX_PLOTS ];
int watch;
int xy;
store *vs = NULL;


/***************************************************
ANALYSIS
****************************************************/
void analysis( int *choice )
{
	*choice = 0;
	while ( *choice == 0 )
		read_data( choice ); 	//Cases and Variables

	*choice = 0;
}


/***************************************************
READ_DATA
****************************************************/
void read_data( int *choice )
{
int i, h, j, k, l, m, p, r;
char *app, dirname[ MAX_PATH_LENGTH ], str1[ MAX_ELEM_LENGTH ], str2[ MAX_ELEM_LENGTH ], str3[ MAX_ELEM_LENGTH ];
double *datum, compvalue = 0;
store *app_store;
FILE *f;

cover_browser( "Analysis of Results...", "Analysis of Results window is open", "Please exit Analysis of Results\nbefore using the LSD Browser." );

Tcl_LinkVar( inter, "cur_plot", ( char * ) &cur_plot, TCL_LINK_INT );
Tcl_LinkVar( inter, "nv", ( char * ) &nv, TCL_LINK_INT );
Tcl_LinkVar( inter, "avgSmpl", ( char * ) &avgSmpl, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "auto", ( char * ) &autom, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "auto_x", ( char * ) &autom_x, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "minc", ( char * ) &min_c, TCL_LINK_INT );
Tcl_LinkVar( inter, "maxc", ( char * ) &max_c, TCL_LINK_INT );
Tcl_LinkVar( inter, "miny", ( char * ) &miny, TCL_LINK_DOUBLE );
Tcl_LinkVar( inter, "maxy", ( char * ) &maxy, TCL_LINK_DOUBLE );
Tcl_LinkVar( inter, "logs", ( char * ) &logs, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "allblack", ( char * ) &allblack, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "grid", ( char * ) &grid, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "point_size", ( char * ) &point_size, TCL_LINK_DOUBLE );
Tcl_LinkVar( inter, "tc", ( char * ) &time_cross, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "line_point", (char * ) &line_point, TCL_LINK_INT );
Tcl_LinkVar( inter, "xy", ( char * ) &xy, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "pdigits", ( char * ) &pdigits, TCL_LINK_INT );
Tcl_LinkVar( inter, "watch", ( char * ) &watch, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "gnu", ( char * ) &gnu, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "num_y2", ( char * ) &num_y2, TCL_LINK_INT );

avgSmplMsg = false;
logs = false;
cur_plot = 0;
file_counter = 0;
num_var = 0;
autom_x = true;
max_c = min_c = num_c = 1;
autom = true;
miny = maxy = 0;
time_cross = xy = false;
gnu = false;
watch = true;

cmd( "set y2 0" );
cmd( "set allblack $grayscaleP" );
cmd( "set grid $gridP" );
cmd( "set point_size $pointsizeP" );
cmd( "set line_point $linemodeP" );
cmd( "set pdigits $pdigitsP" );
cmd( "set avgSmpl $avgSmplP" );
cmd( "set list_times [ list ]" );
cmd( "set gpdgrid3d \"$gnuplotGrid3D\"" );
cmd( "set gpoptions \"$gnuplotOptions\"" );

cmd( "newtop .da \"%s%s - LSD Analysis of Results\" { set choice 2 } \"\"", unsaved_change() ? "*" : " ", simul_name );

cmd( "menu .da.m -tearoff 0" );

cmd( "set w .da.m.exit" );
cmd( ".da.m add cascade -label Exit -menu $w -underline 0" );
cmd( "menu $w -tearoff 0" );
cmd( "$w add command -label \"Quit and Return to Browser\" -command {set choice 2} -underline 0 -accelerator Esc" );

cmd( "set w .da.m.gp" );
cmd( ".da.m add cascade -label Gnuplot -menu $w -underline 0" );
cmd( "menu $w -tearoff 0" );
cmd( "$w add command -label \"Open...\" -command {set choice 4} -underline 5 -accelerator Ctrl+G" );
cmd( "$w add command -label \"Options...\" -command {set choice 37} -underline 8" );

cmd( "set w .da.m.opt" );
cmd( ".da.m add cascade -label Options -menu $w -underline 0" );
cmd( "menu $w -tearoff 0" );
cmd( "$w add command -label \"Colors...\" -command {set choice 21} -underline 0" );
cmd( "$w add command -label \"Plot Parameters...\" -command {set choice 22} -underline 0" );
cmd( "$w add command -label \"Lattice Parameters...\" -command {set choice 44} -underline 0" );
cmd( "$w add checkbutton -label \"Average Y Values\" -variable avgSmpl -underline 8" );

cmd( "set w .da.m.help" );
cmd( "menu $w -tearoff 0" );
cmd( ".da.m add cascade -label Help -menu $w -underline 0" );
cmd( "$w add command -label \"Help on Analysis of Results\" -underline 0 -accelerator F1 -command { set choice 41 }" );
cmd( "$w add command -label \"LSD Quick Help\" -underline 4 -command { LsdHelp LSD_quickhelp.html }" );
cmd( "$w add command -label \"LSD Documentation\" -underline 4 -command { LsdHelp LSD_documentation.html }" );
cmd( "$w add separator" );
cmd( "$w add command -label \"LMM Primer Tutorial\" -underline 4 -command { LsdHelp LMM_primer.html }" );
cmd( "$w add command -label \"Using LSD Models Tutorial\" -underline 0 -command { LsdHelp model_using.html }" );
cmd( "$w add command -label \"Writing LSD Models Tutorial\" -underline 0 -command { LsdHelp model_writing.html }" );
cmd( "$w add separator" );
cmd( "$w add command -label \"Model Report\" -underline 0 -command { set choice 43 }" );
cmd( "$w add separator" );
cmd( "$w add command -label \"About LSD...\" -underline 0 -command { LsdAbout {%s} {%s} .da }", _LSD_VERSION_, _LSD_DATE_  ); 

cmd( ".da configure -menu .da.m" );
cmd( "bind .da <F1> { .da.m.help invoke 0; break }" );

// adjust horizontal text space usage
cmd( "if { $tcl_platform(os) == \"Windows NT\" } { set pad -3 } { if { $tcl_platform(os) == \"Darwin\" } { set pad 1 } { set pad 2 } }" );

cmd( "set f .da.head" );
cmd( "frame $f" );
cmd( "label $f.lb -width [ expr $daCwid + $pad ] -text \"Series available\"" );
cmd( "label $f.pad -width 6" );
cmd( "label $f.ch -width [ expr $daCwid + $pad ] -text \"Series selected\"" );
cmd( "label $f.pl -width [ expr $daCwid + $pad ] -text \"Plots\"" );
cmd( "pack $f.lb $f.pad $f.ch $f.pl -side left" );
cmd( "pack $f" );

cmd( "frame .da.vars" );

cmd( "set f .da.vars.lb" );
cmd( "frame $f" );
cmd( "scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "listbox $f.v -selectmode extended -width $daCwid -yscroll \"$f.v_scroll set\"" );
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
cmd( "listbox $f.v -selectmode extended -width $daCwid -yscroll \"$f.v_scroll set\"" );
cmd( "pack $f.v $f.v_scroll -side left -fill y" );

cmd( "mouse_wheel $f.v" );
cmd( "bind $f.v <BackSpace> {.da.vars.b.out invoke}" );
cmd( "bind $f.v <Return> {.da.b.ts invoke}" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.ch.v <BackSpace> }" );
cmd( "bind $f.v <Button-2> {.da.vars.ch.v selection clear 0 end;.da.vars.ch.v selection set @%%x,%%y; set res [selection get]; set choice 33}" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.ch.v <Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <KeyPress-space> {set res [.da.vars.ch.v get active]; set choice 33}; bind $f.v <KeyPress-space> {set res [.da.vars.ch.v get active]; set choice 33}" );

cmd( "set f .da.vars.pl" );
cmd( "frame $f" );
cmd( "scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "listbox $f.v -width $daCwid -yscroll \"$f.v_scroll set\" -selectmode single" );
cmd( "pack $f.v $f.v_scroll -side left -fill y" );

cmd( "mouse_wheel $f.v" );
cmd( "bind $f.v <Return> {set it [selection get];set choice 3}" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.pl.v <Return> }" );
cmd( "bind $f.v <Button-2> {.da.vars.pl.v selection clear 0 end; .da.vars.pl.v selection set @%%x,%%y; set it [selection get]; set n_it [.da.vars.pl.v curselection]; set choice 20}" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.pl.v <Button-2> -x %%x -y %%y }" );
cmd( "bind .da <KeyPress-Delete> {set n_it [.da.vars.pl.v curselection]; if {$n_it != \"\" } {set it [selection get]; set choice 20} {}}" );

cmd( "frame .da.vars.b" );
cmd( "set f .da.vars.b" );
cmd( "button $f.in -width [ expr $butWid - 3 ] -relief flat -overrelief groove -text \u25b6 -command {set choice 6}" );
cmd( "button $f.out -width [ expr $butWid - 3 ] -relief flat -overrelief groove -state disabled -text \u25c0 -command {set choice 7}" );
cmd( "button $f.sort -width [ expr $butWid - 3 ] -relief flat -overrelief groove -text \"Sort \u25b2\" -command {set choice 5} -underline 0" );
cmd( "button $f.sortdesc -width [ expr $butWid - 3 ] -relief flat -overrelief groove -text \"Sort \u25bc\" -command {set choice 38} -underline 1" );
cmd( "button $f.sortend -width [ expr $butWid - 3 ] -relief flat -overrelief groove -text \"Sort+\" -command {set choice 15} -underline 2" );
cmd( "button $f.unsort -width [ expr $butWid - 3 ] -relief flat -overrelief groove -text \"Unsort\" -command {set choice 14} -underline 0" );
cmd( "button $f.search -width [ expr $butWid - 3 ] -relief flat -overrelief groove -text Find... -command { set choice 39 } -underline 0" );
cmd( "button $f.add -width [ expr $butWid - 3 ] -relief flat -overrelief groove -text \"Add...\" -command {set choice 24} -underline 0" );
cmd( "button $f.empty -width [ expr $butWid - 3 ] -relief flat -overrelief groove -text Clear -command {set choice 8} -underline 0" );
cmd( "pack $f.in $f.out $f.sort $f.sortdesc $f.sortend $f.unsort $f.search $f.add $f.empty -padx 2 -pady 1 -fill y" );

cmd( "pack .da.vars.lb .da.vars.b .da.vars.ch .da.vars.pl -side left  -expand true -fill y" );
cmd( "pack .da.vars -expand true -fill y" );

// add time series in memory
if ( actual_steps > 0 )
{
	insert_data_mem( root, &num_var, &num_c );
	max_c = num_c;
}

cmd( "frame .da.com" );
cmd( "label .da.com.nvar -text \"Series = %d\" -width [ expr ( $daCwid + 2 * int( $pad / 2 - 0.5 ) ) / 2 ]", num_var  );
cmd( "label .da.com.ncas -text \"Cases = %d\" -width [ expr ( $daCwid + 2 * int( $pad / 2 - 0.5 ) ) / 2 ]", num_c  );
cmd( "label .da.com.pad -width 6" );
cmd( "label .da.com.selec -text \"Series = [ .da.vars.ch.v size ]\" -width [ expr $daCwid + $pad ]" );
cmd( "label .da.com.plot -text \"Plots = [ .da.vars.pl.v size ]\" -width [ expr $daCwid + $pad ]" );
cmd( "pack .da.com.nvar .da.com.ncas .da.com.pad .da.com.selec .da.com.plot -side left" );
cmd( "pack .da.com" );

cmd( "frame .da.f" );
cmd( "frame .da.f.h" );
cmd( "frame .da.f.h.v" );

cmd( "frame .da.f.h.v.ft" );

cmd( "checkbutton .da.f.h.v.ft.auto -text \"Use all cases \" -variable auto_x -command {if {$auto_x == 1} {.da.f.h.v.ft.to.mxc conf -state disabled; .da.f.h.v.ft.from.mnc conf -state disabled} {.da.f.h.v.ft.to.mxc conf -state normal; .da.f.h.v.ft.from.mnc conf -state normal}}" );

cmd( "frame .da.f.h.v.ft.from" );
cmd( "label .da.f.h.v.ft.from.minc -text \"From case\"" );
cmd( "entry .da.f.h.v.ft.from.mnc -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set minc %%P; return 1 } { %%W delete 0 end; %%W insert 0 $minc; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.ft.from.minc .da.f.h.v.ft.from.mnc -side left" );

cmd( "frame .da.f.h.v.ft.to" );
cmd( "label .da.f.h.v.ft.to.maxc -text \"to case\"" );
cmd( "entry .da.f.h.v.ft.to.mxc -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set maxc %%P; return 1 } { %%W delete 0 end; %%W insert 0 $maxc; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack  .da.f.h.v.ft.to.maxc .da.f.h.v.ft.to.mxc -side left" );

cmd( "pack .da.f.h.v.ft.auto .da.f.h.v.ft.from .da.f.h.v.ft.to -side left -ipadx 15" );

cmd( "frame .da.f.h.v.sc" );

cmd( "checkbutton .da.f.h.v.sc.auto -text \"Y self-scaling\" -variable auto -command {if {$auto == 1} {.da.f.h.v.sc.max.max conf -state disabled; .da.f.h.v.sc.min.min conf -state disabled} {.da.f.h.v.sc.max.max conf -state normal; .da.f.h.v.sc.min.min conf -state normal}}" );

cmd( "frame .da.f.h.v.sc.min" );
cmd( "label .da.f.h.v.sc.min.lmin -text \"Min. Y\"" );
cmd( "entry .da.f.h.v.sc.min.min -width 10 -validate focusout -vcmd { if [ string is double -strict %%P ] { set miny %%P; return 1 } { %%W delete 0 end; %%W insert 0 $miny; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.sc.min.lmin .da.f.h.v.sc.min.min -side left" );

cmd( "frame .da.f.h.v.sc.max" );
cmd( "label .da.f.h.v.sc.max.lmax -text \"Max. Y\"" );
cmd( "entry .da.f.h.v.sc.max.max -width 10 -validate focusout -vcmd { if [ string is double -strict %%P ] { set maxy %%P; return 1 } { %%W delete 0 end; %%W insert 0 $maxy; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.sc.max.lmax .da.f.h.v.sc.max.max -side left" );

cmd( "pack .da.f.h.v.sc.auto .da.f.h.v.sc.min .da.f.h.v.sc.max -side left -ipadx 5" );

cmd( "frame .da.f.h.v.y2" );
cmd( "checkbutton .da.f.h.v.y2.logs -text \"Series in logs\" -variable logs" );
cmd( "checkbutton .da.f.h.v.y2.y2 -text \"Y2 axis\" -variable y2 -command {if {$y2 == 0} {.da.f.h.v.y2.f.e conf -state disabled} {.da.f.h.v.y2.f.e conf -state normal}}" );

cmd( "frame .da.f.h.v.y2.f" );
cmd( "label .da.f.h.v.y2.f.l -text \"First series in Y2 axis\"" );
cmd( "entry .da.f.h.v.y2.f.e -width 4 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set num_y2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num_y2; return 0 } } -invcmd { bell } -justify center -state disabled" );
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

cmd( "frame .da.f.tit" );
cmd( "label .da.f.tit.l -text Title" );
cmd( "entry .da.f.tit.e -textvariable tit -width 36 -justify center" );

cmd( "frame .da.f.tit.chk" );

cmd( "checkbutton .da.f.tit.chk.allblack -text \"No colors\" -variable allblack" );
cmd( "checkbutton .da.f.tit.chk.grid -text \"Grids\" -variable grid" );
cmd( "pack .da.f.tit.chk.allblack .da.f.tit.chk.grid -anchor w" );

cmd( "frame .da.f.tit.lp -relief groove -bd 2" );
cmd( "radiobutton .da.f.tit.lp.line -text \"Lines\" -variable line_point -value 1" );
cmd( "radiobutton .da.f.tit.lp.point -text \"Points\" -variable line_point -value 2" );
cmd( "pack .da.f.tit.lp.line .da.f.tit.lp.point -anchor w" );

cmd( "frame .da.f.tit.ps" );
cmd( "label .da.f.tit.ps.l -text \"Point size\"" );
cmd( "if [ string equal [ info tclversion ] 8.6 ] { ttk::spinbox .da.f.tit.ps.e -width 4 -from 0.2 -to 9.8 -increment 0.2 -validate focusout -validatecommand { if [ string is double -strict %%P ] { set point_size %%P; return 1 } { %%W delete 0 end; %%W insert 0 $point_size; return 0 } } -invalidcommand { bell } -justify center } { entry .da.f.tit.ps.e -width 4 -validate focusout -vcmd { if [ string is double -strict %%P ] { set point_size %%P; return 1 } { %%W delete 0 end; %%W insert 0 $point_size; return 0 } } -invcmd { bell } -justify center }" );
cmd( "pack .da.f.tit.ps.l .da.f.tit.ps.e" );

cmd( "frame .da.f.tit.run" );			// field for adjusting 
cmd( "checkbutton .da.f.tit.run.watch -text Watch -variable watch" );
cmd( "checkbutton .da.f.tit.run.gnu -text Gnuplot -variable gnu -state disabled" );
cmd( "pack .da.f.tit.run.watch .da.f.tit.run.gnu -anchor w" );

cmd( "frame .da.f.tit.pr" );			// field for adjusting y-axis precision
cmd( "label .da.f.tit.pr.l -text \"Precision\"" );
cmd( "if [ string equal [ info tclversion ] 8.6 ] { ttk::spinbox .da.f.tit.pr.e -width 2 -from 0 -to 9 -validate focusout -validatecommand { if [ string is integer -strict %%P ] { set pdigits %%P; return 1 } { %%W delete 0 end; %%W insert 0 $pdigits; return 0 } } -invalidcommand { bell } -justify center } { entry .da.f.tit.pr.e -width 2 -validate focusout -validatecommand { if [ string is integer -strict %%P ] { set pdigits %%P; return 1 } { %%W delete 0 end; %%W insert 0 $pdigits; return 0 } } -invalidcommand { bell } -justify center }" );
cmd( "pack .da.f.tit.pr.l .da.f.tit.pr.e" );

cmd( "pack .da.f.tit.l .da.f.tit.e .da.f.tit.chk .da.f.tit.run .da.f.tit.pr .da.f.tit.ps .da.f.tit.lp -side left -padx 5" );

cmd( "pack .da.f.h .da.f.tit" );
cmd( "pack .da.f" );

cmd( "frame .da.b" );
cmd( "button .da.b.ts -width $butWid -text Plot -command {set choice 1} -underline 0" );
cmd( "button .da.b.dump -width $butWid -text \"Save Plot\" -command {set fromPlot 0; set choice 11} -underline 2" );
cmd( "button .da.b.sv -width $butWid -text \"Save Data\" -command {set choice 10} -underline 3" );
cmd( "button .da.b.sp -width $butWid -text \"Show Data\" -command {set choice 36} -underline 5" );
cmd( "button .da.b.st -width $butWid -text Statistics -command {set choice 12} -underline 1" );
cmd( "button .da.b.fr -width $butWid -text Histogram -command {set choice 32} -underline 0" );
cmd( "button .da.b.lat -width $butWid -text Lattice -command {set choice 23} -underline 0" );

cmd( "pack .da.b.ts .da.b.dump .da.b.sv .da.b.sp .da.b.st .da.b.fr .da.b.lat -padx 10 -pady 10 -side left -expand no -fill none" );
cmd( "pack .da.b -side right -expand no -fill none" );

// top window shortcuts binding
cmd( "bind .da <KeyPress-Escape> {set choice 2}" );	// quit
cmd( "bind .da <Control-l> {set choice 23}; bind .da <Control-L> {set choice 23}" );	// plot lattice
cmd( "bind .da <Control-h> {set choice 32}; bind .da <Control-H> {set choice 32}" );	// plot histograms
cmd( "bind .da <Control-c> {set choice 8}; bind .da <Control-C> {set choice 8}" );	// empty (clear) selected series
cmd( "bind .da <Control-a> {set choice 24}; bind .da <Control-A> {set choice 24}" );	// insert new series
cmd( "bind .da <Control-i>  {set choice 34}; bind .da <Control-I> {set choice 34}" );	// sort selected in inverse order
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
cmd( "bind .da <Control-v> { set fromPlot 0; set choice 11 }; bind .da <Control-V> { set fromPlot 0; set choice 11 }" );	// save plot
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

if ( num_var == 0 )
  cmd( "tk_messageBox -parent .da -type ok -title \"Analysis of Results\" -icon info -message \"There are no series available\" -detail \"Click on button 'Add...' to load series from results files.\n\nIf you were looking for data after a simulation run, please make sure you have selected the series to be saved, or have not set the objects containing them to not be computed.\"" );  
else
{
	if ( sim_num > 1 )
	  cmd( "tk_messageBox -parent .da -type ok -title \"Analysis of Results\" -icon info -message \"Only series from last run are loaded\" -detail \"Click on button 'Add...' to load series from saved simulation results. You can use 'Ctrl' and 'Shift' keys to select multiple files at once. Avoid selecting the results file from last run, as data is already loaded and would be duplicated.\"" );  
	
	cmd( ".da.vars.lb.v selection set 0" );
	cmd( ".da.vars.lb.v activate 0" );
	cmd( ".da.vars.lb.v see 0" );
	cmd( "focus .da.vars.lb.v" );
}

// make a copy to allow insertion of new temporary variables
cmd( "if [ info exists modElem ] { set DaModElem $modElem } { set DaModElem [ list ] }" );

// main loop

while ( true )
{
	// sort the list of available variables
	cmd( "if [ info exists DaModElem ] { set DaModElem [ lsort -unique -dictionary $DaModElem ] }" );

	// enable/disable the remove series button in the series toolbar
	cmd( "if { [ .da.vars.ch.v size ] > 0 } { .da.vars.b.out conf -state normal } { .da.vars.b.out conf -state disabled }" );

	// reset first second scale series if option is disabled
	cmd( "if { ! $y2 } { set num_y2 2 }" );

	// update entry boxes with linked variables
	update_bounds( );
	cmd( "write_disabled .da.f.h.v.y2.f.e $num_y2" );
	cmd( "write_any .da.f.tit.ps.e $point_size" ); 
	cmd( "write_any .da.f.tit.pr.e $pdigits" ); 

	// update on-screen statistics
	cmd( ".da.com.nvar conf -text \"Series = %d\"", num_var );
	cmd( ".da.com.ncas conf -text \"Cases = %d\"", num_c );
	cmd( ".da.com.selec conf -text \"Series = [ .da.vars.ch.v size ]\"" );
	cmd( ".da.com.plot conf -text \"Plots = [ .da.vars.pl.v size ]\"" );

	// analysis command loop
	*choice = 0;
	while ( ! *choice )
	{
		try
		{
			Tcl_DoOneEvent( 0 );
		}
		catch ( bad_alloc& )		// raise memory problems
		{
			throw;
		}
		catch ( ... )				// ignore the rest
		{
			continue;
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
	
	// check options consistency and fix if necessary
	update_bounds( );
	if ( point_size <= 0 || point_size > 10 || ( gnu && point_size < 1 ) )
		point_size = 1.0;
	if ( pdigits < 1 || pdigits > 8 )
		pdigits = 4;

	if ( *choice == 1 && time_cross == 1 && xy == 0 )	// Plot cross section
		*choice = 9;

	if ( *choice == 1 && time_cross == 0 && xy == 1 )	// Plot XY
		*choice = 17;

	if ( *choice == 1 && time_cross == 1 && xy == 1 )	// Plot XY Cross section
		*choice = 18;

	if ( *choice == 12 && time_cross == 1 )				// Statistics cross section
		*choice = 13;

	switch ( *choice )
	{
		// Exit
		case 2:
			cmd( "if { [ .da.vars.pl.v size ] != 0 } { set answer [ tk_messageBox -parent .da -type okcancel -title Confirmation -icon question -default ok -message \"Exit Analysis of Results?\" -detail \"All the plots and series created and not saved will be lost.\"] } { set answer ok }" );
			app = ( char * ) Tcl_GetVar( inter, "answer", 0 );

			cmd( "if { [ string compare $answer ok ] == 0 } { } { set choice 0 }" );
			if ( *choice == 0 )
				break;

			delete [ ] vs;
			vs = NULL;

			cmd( "destroytop .da" );
			uncover_browser( );
				
			Tcl_UnlinkVar( inter, "auto" );
			Tcl_UnlinkVar( inter, "auto_x" );
			Tcl_UnlinkVar( inter, "minc" );
			Tcl_UnlinkVar( inter, "maxc" );
			Tcl_UnlinkVar( inter, "miny" );
			Tcl_UnlinkVar( inter, "maxy" );
			Tcl_UnlinkVar( inter, "logs" );
			Tcl_UnlinkVar( inter, "allblack" );
			Tcl_UnlinkVar( inter, "grid" );
			Tcl_UnlinkVar( inter, "point_size" );
			Tcl_UnlinkVar( inter, "tc" );
			Tcl_UnlinkVar( inter, "line_point" );
			Tcl_UnlinkVar( inter, "xy" );
			Tcl_UnlinkVar( inter, "pdigits" );
			Tcl_UnlinkVar( inter, "watch" );
			Tcl_UnlinkVar( inter, "gnu" );
			Tcl_UnlinkVar( inter, "num_y2" );
			Tcl_UnlinkVar( inter, "cur_plot" );
			Tcl_UnlinkVar( inter, "nv" );
			Tcl_UnlinkVar( inter, "avgSmpl" );

			cmd( "catch { set a [ glob -nocomplain plotxy_* ] }" ); // remove directories
			cmd( "foreach b $a { catch { file delete -force $b } }" );
			return;
		  
		 
		// MAIN BUTTONS OPTIONS

		// Plot
		case 1:
			cur_plot++;
			
			plot_tseries( choice );
			
			if ( *choice == 2 )		//plot aborted
				cur_plot--;
			else
				cmd( "add_plot $cur_plot $tit" );
			
			break;

		  
		// Plot the cross-section plot. That is, the vars selected from
		// one series for each time step selected
		case 9:
			set_cs_data( choice );
			
			if ( *choice == 2 )		//plot aborted
				break;
			
			cur_plot++;
				
			plot_cross( choice );
			
			if ( *choice == 2 )		//plot aborted
				cur_plot--;
			else
				cmd( "add_plot $cur_plot $tit" );
				
			break;

			
		// Plot XY: plot_gnu or phase diagram, depending on how many series are selected
		case 17:
			cur_plot++;
			
			if ( nv > 1 )
				plot_gnu( choice );
			else
				plot_phase_diagram( choice );
			
			if ( *choice == 2 )
				cur_plot--;
			else
				cmd( "add_plot $cur_plot $tit" );
			
			break;

		  
		// Plot_cs_xy
		case 18:
			cur_plot++;
			
			plot_cs_xy( choice );
			
			if ( *choice == 2 )		//plot aborted
				cur_plot--;
			else
				cmd( "add_plot $cur_plot $tit" );
			
			break;

		  
		// plot a lattice. 
		// Data must be stored on a single time step organized for lines and columns in sequence
		case 23:
			cur_plot++;
			
			plot_lattice( choice );
			
			if ( *choice == 2 )		//plot aborted
				cur_plot--;
			else   
				cmd( "add_plot $cur_plot $tit" );
			
			break;


		// plot histograms
		case 32:
			cur_plot++;
			
			cmd( "set choice $tc" );
			if ( *choice == 0 )
				histograms( choice );
			else
				histograms_cs( choice );
			
			if ( *choice == 2 )		//plot aborted
				cur_plot--;
			else   
				cmd( "add_plot $cur_plot $tit" );
				
			break;


		// Print the data series in the log window
		case 36: 
			plog_series( choice );
			cmd( "wm deiconify .log; raise .log .da" );
			
			break;
		   
		   
		// show the equation for the selected variable
		case 16:
			cmd( "set a [split $res]; set b [lindex $a 0]" );
			app = ( char * ) Tcl_GetVar( inter, "b", 0 );
			
			*choice = 2;	// point .da window as parent for the following window
			
			show_eq( app, choice );
			
			break;


		// Save data in the Vars. to plot listbox
		case 10:
			save_datazip( choice );
			
			break;


		// Statistics
		case 12:
			statistics( choice );
			cmd( "wm deiconify .log; raise .log .da" );
			
			break;

		 
		// Statistics Cross Section
		case 13:
			set_cs_data( choice );
			
			if ( *choice != 2 )
			{
				statistics_cross( choice );
				cmd( "wm deiconify .log; raise .log .da" );
			}
			
			break;


		// create the postscript for a plot
		case 11:
			cmd( "set choice $fromPlot" );
			if ( ! *choice )
			{
				cmd( "set choice [ .da.vars.pl.v size ]" );
				if ( *choice == 0 )		 // no plots to save
				{
					cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No plot available\" -detail \"Place one or more series in the Series Selected listbox and select 'Plot' to produce a plot.\"" );
					break;
				}

				do
				{
					cmd( "set iti [ .da.vars.pl.v curselection ]" );
					cmd( "if { [ string length $iti ] == 0 } { set choice 1 } { set choice 0 }" );

					if ( *choice == 1 )
					{
						*choice = 0;
						cmd( "newtop .da.a \"Save Plot\" { set choice 2 } .da" );
						cmd( "label .da.a.l -text \"Select one plot from the \nPlots listbox before clicking 'OK'\"" );
						cmd( "pack .da.a.l -pady 10 -padx 5" );
						cmd( "okhelpcancel .da.a b { set choice 1 } { LsdHelp menudata_res.html#postscript } { set choice 2 }" );
						cmd( "showtop .da.a centerW 0 0 0" );
						while ( *choice == 0 )
							Tcl_DoOneEvent( 0 );
						cmd( "destroytop .da.a" );
					}
				}
				while ( *choice == 1 );

				if ( *choice == 2 )
					break;

				*choice = 0;
				
				cmd( "set it [ .da.vars.pl.v get $iti ]" );
			}

			
			cmd( "set choice [ info exists it ]" );
			if ( ! *choice )
				break;
			
			cmd( "unset a" );
			cmd( "scan $it %%d)%%s a b" );
			cmd( "set choice [ info exists a ]" );
			if ( ! *choice )
				break;
			
			cmd( "set choice [ winfo exists .da.f.new$a ]" );

			if ( *choice == 0 )
			{
				cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Cannot save plot\" -detail \"The plot was produced in Gnuplot. Please reopen/create the plot and use the Gnuplot tools (toolbar buttons) to save it.\"" );
				break;  
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
			cmd( "frame .da.file.dim" );
			cmd( "label .da.file.dim.l1 -text \"Dimension\"" );
			cmd( "entry .da.file.dim.n -width 4 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set dim %%P; return 1 } { %%W delete 0 end; %%W insert 0 $dim; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.file.dim.n insert 0 $dim" );
			cmd( "label .da.file.dim.l2 -text \"( mm@96DPI)\"" );
			cmd( "pack .da.file.dim.l1 .da.file.dim.n .da.file.dim.l2 -side left" );

			cmd( "set heightpost 1" );
			cmd( "checkbutton .da.file.lab -text \"Include series legends\" -variable heightpost -onvalue 1 -offvalue 0" );
			cmd( "set choice $a" );
			if ( plot_l[ *choice ] == plot_nl[ *choice ] )
				cmd( ".da.file.lab conf -state disabled" );

			cmd( "pack .da.file.l .da.file.col .da.file.pos .da.file.dim .da.file.lab -pady 5 -padx 5" );
			cmd( "okhelpcancel .da.file b { set choice 1 } { LsdHelp menudata_res.html#postscript } { set choice 2 }" );
			cmd( "showtop .da.file" );

			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set dim [ .da.file.dim.n get ]" ); 
			cmd( "destroytop .da.file" );

			if ( *choice == 2 )
				break;

			// make sure there is a path set
			cmd( "set path \"%s\"", path );
			if ( strlen( path ) > 0 )
				cmd( "cd \"$path\"" );

			cmd( "set fn \"$b.eps\"" );
			cmd( "set fn [ tk_getSaveFile -parent .da -title \"Save Plot File\" -defaultextension .eps -initialfile $fn -initialdir \"$path\" -filetypes { { {Encapsulated Postscript files} {.eps} } { {All files} {*} } } ]; if { [string length $fn] == 0 } { set choice 2 }" );

			if ( *choice == 2 )
				break;

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
			cmd( ".da.f.new$a.f.plots postscript -x 0 -y 0 -height [ expr round( $heightpost * $zoomLevel%d) ] -width [ expr round( %d * $zoomLevel%d) ] -pagewidth $dd -rotate $res -colormode $cm -file \"$fn\"", *choice, plot_w[ *choice ], *choice );
			cmd( "plog \"\nPlot saved: $fn\n\"" );

			break;


		// SELECTION BOX CONTEXT MENUS

		// Use right button of the mouse to select all series with a given label
		case 30:
			Tcl_LinkVar( inter, "compvalue", ( char * ) &compvalue, TCL_LINK_DOUBLE );
			cmd( "set a [split $res]" );
			cmd( "set b [lindex $a 0]" );
			cmd( "set c [lindex $a 1 ]" ); //get the tag value
			cmd( "set i [llength [split $c {_}] ]" );
			cmd( "set ntag $i" );
			cmd( "set ssys 2" );
			cmd( "if { ! [ info exist tvar ] } {set tvar $maxc}" );
			cmd( "if { ! [ info exist cond] } {set cond 1}" );

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
			cmd( "bind .da.a.q.f2.f.e <KeyRelease> {if { %%N < 256 && [ info exists DaModElem] } { set bb1 [.da.a.q.f2.f.e index insert]; set bc1 [.da.a.q.f2.f.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1  != -1 } {set bd1 [lindex $DaModElem $bf1 ]; .da.a.q.f2.f.e delete 0 end; .da.a.q.f2.f.e insert 0 $bd1; .da.a.q.f2.f.e index $bb1; .da.a.q.f2.f.e selection range $bb1 end } } }" );
			cmd( "bind .da.a.q.f2.f.e <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
			cmd( "pack .da.a.q.f2.f.l .da.a.q.f2.f.e -anchor w -side left" );
			cmd( "pack .da.a.q.f2.f -anchor w -padx 22" );
			cmd( "pack .da.a.q.f2 -anchor w" );
			cmd( "frame .da.a.c -relief groove -bd 2" );
			cmd( "frame .da.a.c.o" );
			cmd( "radiobutton .da.a.c.o.eq -text \"Equal (=)\" -variable cond -value 1 -state disabled" );
			cmd( "radiobutton .da.a.c.o.dif -text \"Different (\u2260 )\" -variable cond -value 0 -state disabled" );
			cmd( "radiobutton .da.a.c.o.geq -text \"Larger or equal (\u2265)\" -variable cond -value 2 -state disabled" );
			cmd( "radiobutton .da.a.c.o.g -text \"Larger (>)\" -variable cond -value 3 -state disabled" );
			cmd( "radiobutton .da.a.c.o.seq -text \"Smaller or equal (\u2264)\" -variable cond -value 4 -state disabled" );
			cmd( "radiobutton .da.a.c.o.s -text \"Smaller (<)\" -variable cond -value 5 -state disabled" );
			cmd( "pack .da.a.c.o.eq .da.a.c.o.dif .da.a.c.o.geq .da.a.c.o.g .da.a.c.o.seq .da.a.c.o.s -anchor w" );
			cmd( "frame .da.a.c.v" );
			cmd( "frame .da.a.c.v.c" );
			cmd( "label .da.a.c.v.c.l -text \"Comparison value\"" );
			cmd( "entry .da.a.c.v.c.e -width 10 -validate focusout -vcmd { if [ string is double -strict %%P ] { set compvalue %%P; return 1 } { %%W delete 0 end; %%W insert 0 $compvalue; return 0 } } -invcmd { bell } -justify center -state disabled" );
			cmd( "write_any .da.a.c.v.c.e $compvalue" ); 
			cmd( "bind .da.a.c.v.c.e <Return> {focus .da.a.c.v.t.e2; .da.a.c.v.t.e2 selection range 0 end }" );
			cmd( "pack .da.a.c.v.c.l .da.a.c.v.c.e" );
			cmd( "frame .da.a.c.v.t" );
			cmd( "label .da.a.c.v.t.t -text \"Case\"" );
			cmd( "entry .da.a.c.v.t.e2 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set tvar %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tvar; return 0 } } -invcmd { bell } -justify center -state disabled" );
			cmd( "write_any .da.a.c.v.t.e2 $tvar" ); 
			cmd( "bind .da.a.c.v.t.e2 <Return> {focus .da.a.b.ok}" );
			cmd( "pack .da.a.c.v.t.t .da.a.c.v.t.e2" );
			cmd( "pack .da.a.c.v.c .da.a.c.v.t -ipady 10" );
			cmd( "pack .da.a.c.o .da.a.c.v -anchor w -side left -ipadx 5" );
			cmd( "pack .da.a.tit .da.a.q .da.a.c -expand yes -fill x -padx 5 -pady 5" );

			cmd( "okhelpcancel .da.a b { set choice 1 } { LsdHelp menudata_res.html#batch_sel } { set choice 2 }" );
			cmd( "showtop .da.a topleftW 0 0" );
			cmd( "focus .da.a.q.f1.c" );

			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set tvar [ .da.a.c.v.t.e2 get ]" ); 
			cmd( "set compvalue [ .da.a.c.v.c.e get ]" ); 
			Tcl_UnlinkVar( inter, "compvalue" );

			if ( *choice == 2 )
			{
				cmd( "destroytop .da.a" );
				break;
			}

			cmd( "if {[.da.vars.ch.v get 0] == \"\"} {set tit \"\"} {}" );
			cmd( "set choice $ssys" );
			if ( *choice == 2 )
			{
				cmd( "set tot [.da.vars.lb.v get 0 end]" );
				cmd( "foreach i $tot { if { [lindex [split $i ] 0] == \"$b\"} {  .da.vars.ch.v insert end \"$i\"  } {}}" );
				cmd( "if {\"$tit\" == \"\"} {set tit [.da.vars.ch.v get 0]} {}" );
			}
			 
			if ( *choice == 1 )
			{
				cmd( "set choice $cond" );
				i  = * choice;

				*choice = -1;
				cmd( "for {set x 0} {$x<$i} {incr x} {if {[.da.a.q.f.l.e$x get] != \"\"} {set choice $x}}" );
				if ( *choice == -1 )
				{
					cmd( "set tot [.da.vars.lb.v get 0 end]" );
					cmd( "foreach i $tot { if { [lindex [split $i ] 0] == \"$b\"} { .da.vars.ch.v insert end \"$i\"  } }" );
				}
				else
				{
					cmd( "set tot [.da.vars.lb.v get 0 end]" );
					cmd( "if { [ info exist vcell] == 1} {unset vcell} {}" );
					cmd( "set choice $ntag" );
					for ( j = 0; j < *choice; ++j )
						cmd( "lappend vcell $v%d", j );

					switch ( i )
					{
						case 0:
							cmd( "foreach i $tot { if { [lindex [split $i ] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] == [lindex [split [lindex [split $i ] 1 ] {_}] $x] } { set c 0 } }; if $c { .da.vars.ch.v insert end \"$i\" } } }" );
							break;
						case 1:
							cmd( "foreach i $tot { if { [lindex [split $i ] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] != [lindex [split [lindex [split $i ] 1 ] {_}] $x] } { set c 0} {} }; if { $c == 1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
							break;
						case 2:
							cmd( "foreach i $tot { if { [lindex [split $i ] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] > [lindex [split [lindex [split $i ] 1 ] {_}] $x] } { set c 0} {} }; if { $c == 1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
							break;
						case 3:
							cmd( "foreach i $tot { if { [lindex [split $i ] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] >= [lindex [split [lindex [split $i ] 1 ] {_}] $x] } { set c 0} {} }; if { $c == 1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
							break;
						case 4:
							cmd( "foreach i $tot { if { [lindex [split $i ] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] < [lindex [split [lindex [split $i ] 1 ] {_}] $x] } { set c 0} {} }; if { $c == 1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
							break;
						case 5:
						   cmd( "foreach i $tot { if { [lindex [split $i ] 0] == \"$b\" } { set c 1; for {set x 0} {$x<$ntag} {incr x} { if { [lindex $vcell $x] != \"\" && [lindex $vcell $x] <= [lindex [split [lindex [split $i ] 1 ] {_}] $x] } { set c 0} {} }; if { $c == 1 } {  .da.vars.ch.v insert end \"$i\"  } {}} {}}" );
					} 
				}
				
				*choice = 0;
			}

			if ( *choice == 3 || *choice == 4 )
			{
				l = *choice;
				cmd( "set choice $cond" );
				p = *choice;
				cmd( "set tot [.da.vars.lb.v get 0 end]" );
				cmd( "set choice [llength $tot]" );
				j = *choice;

				if ( l == 3 )
					app = ( char * ) Tcl_GetVar( inter, "b", 0 );
				else
					app = ( char * ) Tcl_GetVar( inter, "svar", 0 );

				strcpy( str3, app );

				cmd( "set choice $tvar" );
				h = *choice;

				for ( i = 0; i < j; ++i )
				{
					cmd( "set res [ lindex $tot %d ]", i );
					app = ( char * ) Tcl_GetVar( inter, "res", 0 );
					strcpy( msg, app );
					sscanf( msg, "%s %s (%d-%d) #%d", str1, str2, &l, &m, &k );
					if ( h >= l && h <= m && ! strcmp( str1, str3 ) )
					{
						datum = vs[ k ].data;
						if ( is_finite( datum[ h ] ) )		// ignore NaNs
						{
							r = 0;
							switch ( p )
							{
								case 0: 
									if ( datum[ h ] != compvalue )
										r = 1;
									break;
								case 1: 
									if ( datum[ h ] == compvalue )
										r = 1;
									break;
								case 2: 
									if ( datum[ h ] >= compvalue )
										r = 1;
									break;
								case 3: 
									if ( datum[ h ] > compvalue )
										r = 1;
									break;
								case 4: 
									if ( datum[ h ] <= compvalue )
										r = 1;
									break;
								case 5: 
									if ( datum[ h ] < compvalue )
										r = 1;
									break;
							}
							
							if ( r == 1 )
								cmd( ".da.vars.ch.v insert end $res" );
						}
					}
				}
			}

			cmd( "destroytop .da.a" );

			cmd( "if {\"$tit\" == \"\"} {set tit [.da.vars.ch.v get 0]} {}" );

			break;


		// Use right button of the mouse to remove series selected with different criteria
		case 33:
			cmd( "unset -nocomplain compvalue" );
			Tcl_LinkVar( inter, "compvalue", ( char * ) &compvalue, TCL_LINK_DOUBLE );
			cmd( "set a [split $res]" );
			cmd( "set b [lindex $a 0]" );
			cmd( "set c [lindex $a 1 ]" ); //get the tag value
			cmd( "set i [llength [split $c {_}] ]" );
			cmd( "set ntag $i" );
			cmd( "set ssys 2" );
			cmd( "if { ! [ info exist tvar ] } { set tvar $maxc }" );
			cmd( "if { ! [ info exist cond] } { set cond 1 }" );
			cmd( "if { ! [ info exist selOnly] } { set selOnly 0 }" );

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
			cmd( "bind .da.a.q.f2.f.e <KeyRelease> {if { %%N < 256 && [ info exists DaModElem] } { set bb1 [.da.a.q.f2.f.e index insert]; set bc1 [.da.a.q.f2.f.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1  != -1 } {set bd1 [lindex $DaModElem $bf1 ]; .da.a.q.f2.f.e delete 0 end; .da.a.q.f2.f.e insert 0 $bd1; .da.a.q.f2.f.e index $bb1; .da.a.q.f2.f.e selection range $bb1 end } } }" );
			cmd( "bind .da.a.q.f2.f.e <Return> {focus .da.a.c.v.c.e; .da.a.c.v.c.e selection range 0 end}" );
			cmd( "pack .da.a.q.f2.f.l .da.a.q.f2.f.e -anchor w -side left" );
			cmd( "pack .da.a.q.f2.f -anchor w -padx 22" );
			cmd( "pack .da.a.q.f2 -anchor w" );
			cmd( "frame .da.a.c -relief groove -bd 2" );
			cmd( "frame .da.a.c.o" );
			cmd( "radiobutton .da.a.c.o.eq -text \"Equal (=)\" -variable cond -value 1 -state disabled" );
			cmd( "radiobutton .da.a.c.o.dif -text \"Different (\u2260 )\" -variable cond -value 0 -state disabled" );
			cmd( "radiobutton .da.a.c.o.geq -text \"Larger or equal (\u2265)\" -variable cond -value 2 -state disabled" );
			cmd( "radiobutton .da.a.c.o.g -text \"Larger (>)\" -variable cond -value 3 -state disabled" );
			cmd( "radiobutton .da.a.c.o.seq -text \"Smaller or equal (\u2264)\" -variable cond -value 4 -state disabled" );
			cmd( "radiobutton .da.a.c.o.s -text \"Smaller (<)\" -variable cond -value 5 -state disabled" );
			cmd( "pack .da.a.c.o.eq .da.a.c.o.dif .da.a.c.o.geq .da.a.c.o.g .da.a.c.o.seq .da.a.c.o.s -anchor w" );
			cmd( "frame .da.a.c.v" );
			cmd( "frame .da.a.c.v.c" );
			cmd( "label .da.a.c.v.c.l -text \"Comparison value\"" );
			cmd( "entry .da.a.c.v.c.e -width 10 -validate focusout -vcmd { if [ string is double -strict %%P ] { set compvalue %%P; return 1 } { %%W delete 0 end; %%W insert 0 $compvalue; return 0 } } -invcmd { bell } -justify center -state disabled" );
			cmd( "write_any .da.a.c.v.c.e $compvalue" ); 
			cmd( "bind .da.a.c.v.c.e <Return> {focus .da.a.c.v.t.e2; .da.a.c.v.t.e2 selection range 0 end }" );
			cmd( "pack .da.a.c.v.c.l .da.a.c.v.c.e" );
			cmd( "frame .da.a.c.v.t" );
			cmd( "label .da.a.c.v.t.t -text \"Case\"" );
			cmd( "entry .da.a.c.v.t.e2 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set tvar %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tvar; return 0 } } -invcmd { bell } -justify center -state disabled" );
			cmd( "write_any .da.a.c.v.t.e2 $tvar" ); 
			cmd( "bind .da.a.c.v.t.e2 <Return> {focus .da.a.b.ok}" );
			cmd( "pack .da.a.c.v.t.t .da.a.c.v.t.e2" );
			cmd( "pack .da.a.c.v.c .da.a.c.v.t -ipady 10" );
			cmd( "pack .da.a.c.o .da.a.c.v -anchor w -side left -ipadx 5" );
			cmd( "frame .da.a.s" );
			cmd( "checkbutton .da.a.s.b -text \"Only mark items\" -variable selOnly" );
			cmd( "pack .da.a.s.b" );
			cmd( "pack .da.a.tit .da.a.q .da.a.c .da.a.s -expand yes -fill x -padx 5 -pady 5" );

			cmd( "okhelpcancel .da.a b { set choice 1 } { LsdHelp menudata_res.html#batch_sel } { set choice 2 }" );
			cmd( "showtop .da.a topleftW 0 0" );
			cmd( "focus .da.a.q.f1.c" );

			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set tvar [ .da.a.c.v.t.e2 get ]" ); 
			cmd( "set compvalue [ .da.a.c.v.c.e get ]" ); 
			Tcl_UnlinkVar( inter, "compvalue" );

			if ( *choice == 2 )
			{
				cmd( "destroytop .da.a" );
				break;
			}

			cmd( "set choice $ssys" );
			cmd( ".da.vars.ch.v selection clear 0 end" );

			if ( *choice == 2 )
			 {
				 cmd( "set tot [.da.vars.ch.v get 0 end]" );
				 cmd( "set myc 0; foreach i $tot { if { [lindex [split $i ] 0] == \"$b\"} {  .da.vars.ch.v selection set $myc  } {}; incr myc}" );
			 }
			 
			if ( *choice == 1 )
			{
				cmd( "set choice $cond" );
				i = *choice;

				*choice = -1;
				cmd( "for {set x 0} {$x<$i} {incr x} {if {[.da.a.q.f.l.e$x get] != \"\"} {set choice $x}}" );
				if ( *choice == -1 )
				{
					cmd( "set tot [.da.vars.ch.v get 0 end]" );
					cmd( "set myc 0; foreach i $tot { if { [lindex [split $i ] 0] == \"$b\"} { .da.vars.ch.v selection set $myc }; incr myc }" );
				}
				else
				{
					cmd( "set tot [.da.vars.ch.v get 0 end]" );
					cmd( "if { [ info exist vcell] == 1} {unset vcell} {}" );
					cmd( "set choice $ntag" );
					for ( j = 0; j < *choice; ++j )
						cmd( "lappend vcell $v%d", j );

					switch ( i )
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
					} 
				}
				
				*choice = 0;
			}

			if ( *choice == 3 || *choice == 4 )
			{
				l = *choice;
				cmd( "set choice $cond" );
				p = *choice;
				cmd( "set tot [.da.vars.ch.v get 0 end]" );
				cmd( "set choice [llength $tot]" );
				j = *choice;

				if ( l == 3 )
					app = ( char * ) Tcl_GetVar( inter, "b", 0 );
				else
					app = ( char * ) Tcl_GetVar( inter, "svar", 0 );

				strcpy( str3, app );

				cmd( "set choice $tvar" );
				h = *choice;

				for ( i = 0; i < j; ++i )
				{
					cmd( "set res [ lindex $tot %d ]", i );
					app = ( char * ) Tcl_GetVar( inter, "res", 0 );
					strcpy( msg, app );
					sscanf( msg, "%s %s (%d-%d) #%d", str1, str2, &l, &m, &k );
					if ( h >= l && h <= m && ! strcmp( str1, str3 ) )
					{
						datum = vs[ k ].data;
						if ( is_finite( datum[ h ] ) )		// ignore NaNs
						{
							r = 0;
							switch ( p )
							{
								case 0: 
									if ( datum[ h ] != compvalue )
										r = 1;
									break;
								case 1: 
									if ( datum[ h ] == compvalue )
										r = 1;
									break;
								case 2: 
									if ( datum[ h ] >= compvalue )
										r = 1;
									break;
								case 3: 
									if ( datum[ h ] > compvalue )
										r = 1;
									break;
								case 4: 
									if ( datum[ h ] <= compvalue )
										r = 1;
									break;
								case 5: 
									if ( datum[ h ] < compvalue )
										r = 1;
									break;
							}
							if ( r == 1 )
								cmd( ".da.vars.ch.v selection set %d", i );
						}
					}
				}
			}

			cmd( "destroytop .da.a" );

			cmd( "if { ! $selOnly } { set steps 0; foreach i [ .da.vars.ch.v curselection ] { .da.vars.ch.v delete [ expr $i - $steps ]; incr steps} } { if { [ llength [ .da.vars.ch.v curselection ] ] > 0 } { .da.vars.ch.v see [ lindex [ .da.vars.ch.v curselection ] 0 ] } }" );

			break;


		// PLOTS LIST CONTEXT ACTIONS

		// remove a plot
		case 20:
			cmd( "set answer [tk_messageBox -parent .da -type yesno -title Confirmation -message \"Delete plot?\" -detail \"Press 'Yes' to delete plot:\\n$tit\" -icon question -default yes]" );
			cmd( "if {[string compare $answer yes] == 0} { set choice 1} {set choice 0}" );
			if ( *choice == 0 )
				break;
			
			cmd( "scan $it %%d)%%s a b" );
			cmd( "set ex [winfo exists .da.f.new$a]" );
			cmd( "if { $ex == 1 } {destroytop .da.f.new$a; .da.vars.pl.v delete $n_it} {.da.vars.pl.v delete $n_it}" );

			break;


		// Raise the clicked plot
		case 3:
			cmd( "scan $it %%d)%%s a b" );
			cmd( "set ex [winfo exists .da.f.new$a]" );
			*choice = 0;
			cmd( "if { $ex == 1 } { wm deiconify .da.f.new$a; raise .da.f.new$a; focus .da.f.new$a } { set choice 1 }" );
			if ( *choice == 1 )
			{
				getcwd( dirname, MAX_PATH_LENGTH - 1 );
				cmd( "set choice $a" );
				sprintf( msg, "plotxy_%d", *choice );
				chdir( msg );
				cmd( "set choice $a" );
				show_plot_gnu( *choice, choice, 1, NULL, NULL );
				chdir( dirname );
			}
			
			miny = maxy = 0;
			
			break;

		  
		// SERIES TOOLBAR OPTIONS

		// Sort
		case 5:
			cmd( "set a [.da.vars.lb.v get 0 end]" );
			cmd( "set choice [llength $a]" );
			if ( *choice == 0 )
				break;
			
			cmd( "set b [lsort -command comp_und $a]" );		// use special sort procedure to keep underscores at the end
			cmd( ".da.vars.lb.v delete 0 end" );
			cmd( "foreach i $b {.da.vars.lb.v insert end $i}" );
			
			break;
		  
		  
		// Sort ( descending order)
		case 38:
			cmd( "set a [.da.vars.lb.v get 0 end]" );
			cmd( "set choice [llength $a]" );
			if ( *choice == 0 )
				break;
			
			cmd( "set b [lsort -decreasing -dictionary $a]" );
			cmd( ".da.vars.lb.v delete 0 end" );
			cmd( "foreach i $b {.da.vars.lb.v insert end $i}" );
			
			break;
		 

		// sort the selection in selected series list in inverse order
		case 34:
			cmd( "set a [.da.vars.ch.v curselection]" );
			cmd( "set choice [llength $a]" );
			if ( *choice == 0 )
				break;

			cmd( "set b {}" );
			cmd( "foreach i $a {lappend b [.da.vars.ch.v get $i ]}" );
			cmd( "set c [lsort -decreasing -dictionary $b]" );
			cmd( "set d -1" );
			cmd( "foreach i $a {.da.vars.ch.v delete $i; .da.vars.ch.v insert $i [lindex $c [ incr d] ] }" );
			
			break;

		   
		// Unsort
		case 14:
			cmd( "set a [.da.vars.lb.v get 0 end]" );
			cmd( "set choice [llength $a]" );
			if ( *choice == 0 )
				break;

			cmd( ".da.vars.lb.v delete 0 end" );
			for ( i = 0; i < num_var; ++i )
				cmd( ".da.vars.lb.v insert end \"%s %s (%d-%d) #%d\"", vs[ i ].label, vs[ i ].tag, vs[ i ].start, vs[ i ].end, vs[ i ].rank );
			
			break;

		   
		// Sort on End
		case 15:
			cmd( "set a [.da.vars.lb.v get 0 end]" );
			cmd( "set choice [llength $a]" );
			if ( *choice == 0 )
				break;

			app_store = new store[ num_var ];
			
			for ( i = 0; i < num_var; ++i )
				app_store[ i ] = vs[ i ];
			
			sort_on_end( app_store );
			cmd( ".da.vars.lb.v delete 0 end" );
			
			for ( i = 0; i < num_var; ++i )
				cmd( ".da.vars.lb.v insert end \"%s %s (%d-%d) #%d\"", app_store[ i ].label, app_store[ i ].tag, app_store[ i ].start, app_store[ i ].end, app_store[ i ].rank );
			
			delete [ ] app_store;
				
			break;

		   
		// Find first instance of series in the available series listbox
		case 39:
			cmd( "set a [.da.vars.lb.v get 0 end]" );
			cmd( "set choice [llength $a]" );
			if ( *choice == 0 )
				break;

			cmd( "set srchTxt \"\"; set srchInst 1" );
			cmd( "newtop .da.a \"Find Series\" { set choice 2 } .da" );
			cmd( "label .da.a.l -text \"Series name (or part)\"" );
			cmd( "entry .da.a.e -textvariable srchTxt -width 20 -justify center" );
			cmd( "bind .da.a.e <KeyRelease> {if { %%N < 256 && [ info exists DaModElem] } { set bb1 [.da.a.e index insert]; set bc1 [.da.a.e get]; set bf1 [lsearch -glob $DaModElem $bc1*]; if { $bf1  != -1 } { set bd1 [lindex $DaModElem $bf1 ]; .da.a.e delete 0 end; .da.a.e insert 0 $bd1; .da.a.e index $bb1; .da.a.e selection range $bb1 end } } }" );
			cmd( "label .da.a.n -text \"(finds first instance only,\nuse 'F3' or 'Ctrl+N' to find others)\"" );
			cmd( "pack .da.a.l .da.a.e .da.a.n -pady 5 -padx 5" );
			cmd( "okhelpcancel .da.a b  { set choice 1 } { LsdHelp menudata_res.html#find } { set choice 2 }" );
			cmd( "bind .da.a.e <Return> { set choice 1 }" );
			cmd( "bind .da.a.e <Escape> { set choice 2 }" );
			cmd( "showtop .da.a" );
			cmd( "focus .da.a.e" );

			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "destroytop .da.a" );

			if ( *choice == 1 )
			{
				cmd( "set choice [ string length $srchTxt ]" );
				if ( *choice == 0 )
					break;
				
				cmd( "set srchTxt [ string tolower $srchTxt ]" );
				cmd( "set a [ .da.vars.lb.v get 0 end ]; set i 0" );
				cmd( "foreach b $a { set choice [ string first $srchTxt [ string tolower [ lindex [ split $b ] 0 ] ] ]; if { $choice != -1 } { break } { set i [ expr $i + 1 ] } }" );
				if ( *choice != -1 )
					cmd( "focus .da.vars.lb.v; .da.vars.lb.v selection clear 0 end; .da.vars.lb.v selection set $i; .da.vars.lb.v activate $i; .da.vars.lb.v see $i" );
				else
					cmd( "tk_messageBox -parent .da -type ok -title \"Error\" -icon error -default ok -message \"Series not found\" -detail \"Check if the name was spelled correctly or use just part of the name. This command is case insensitive.\"" );
			}

			break;
		  
		  
		// Find subsequent instances of a series in the available series listbox
		case 40:
			cmd( "set choice [ string length $srchTxt ]" );
			if ( *choice == 0 )
				break;

			cmd( "set a [ .da.vars.lb.v get 0 end ]; set i 0; set inst 0" );
			cmd( "foreach b $a { set choice [ string first $srchTxt [ string tolower [ lindex [ split $b ] 0 ] ] ]; if { $choice != -1 } { set inst [ expr $inst + 1 ]; if { $inst > $srchInst } { set srchInst $inst; break } }; set i [ expr $i + 1 ] }" );
			if ( *choice != -1 )
				cmd( ".da.vars.lb.v selection clear 0 end; .da.vars.lb.v selection set $i; .da.vars.lb.v see $i" );
			else
				cmd( "tk_messageBox -parent .da -type ok -title \"Error\" -icon error -default ok -message \"Series not found\" -detail \"No additional instance of series found.\"" );

			break;

		  
		// Insert the variables selected in the list of the variables to plot
		case 6:
			cmd( "set a [.da.vars.lb.v curselection]" );
			cmd( "foreach i $a {.da.vars.ch.v insert end [.da.vars.lb.v get $i ]}" );
			cmd( "set tit [.da.vars.ch.v get 0]" );

			break;

		  
		// Remove the vars. selected from the variables to plot
		case 7:
			cmd( "set steps 0" );
			cmd( "foreach i [.da.vars.ch.v curselection] {.da.vars.ch.v delete [ expr $i - $steps]; incr steps}" );
			
			break;

		  
		// Remove all the variables from the list of vars to plot
		case 8:
			cmd( ".da.vars.ch.v delete 0 end" );

			break;


		// Insert new series ( from disk or combining existing series).
		case 24:
			if ( nv > 0 )
			{
				cmd( "newtop .da.s \"Choose Data Source\" { set choice 2 } .da" );
				cmd( "label .da.s.l -text \"Source of additional series\"" );

				cmd( "set bidi 4" );
				cmd( "frame .da.s.i -relief groove -bd 2" );
				cmd( "radiobutton .da.s.i.c -text \"Create new series from selected\" -variable bidi -value 4" );
				cmd( "radiobutton .da.s.i.a -text \"Moving average series from selected\" -variable bidi -value 5" );
				cmd( "radiobutton .da.s.i.f -text \"File( s) of saved results\" -variable bidi -value 1" );
				cmd( "pack .da.s.i.c .da.s.i.a .da.s.i.f -anchor w" );

				cmd( "pack .da.s.l .da.s.i -expand yes -fill x -pady 5 -padx 5" );
				cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#add_series } { set choice 2 }" );
				cmd( "showtop .da.s" );

				*choice = 0;
				while ( *choice == 0 )
				  Tcl_DoOneEvent( 0 );

				cmd( "destroytop .da.s" );

				if ( *choice == 2 )
					break;

				cmd( "set choice $bidi" );
				switch ( *choice )
				{
					case 4:
						*choice = 0;
						create_series( choice );
						*choice = 0;
						break;
					case 5:
						*choice = 0;
						create_maverag( choice );
						*choice = 0;
						break;
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
				const char extRes[ ] = ".res .res.gz";
				const char extTot[ ] = ".tot .tot.gz";
#else
				const char extRes[ ] = ".res";
				const char extTot[ ] = ".tot";
#endif 
			 
				// make sure there is a path set
				cmd( "set path \"%s\"", path );
				if ( strlen( path ) > 0 )
					cmd( "cd \"$path\"" );
			
				cmd( "set lab [tk_getOpenFile -parent .da -title \"Load Results File\" -multiple yes -initialdir \"$path\" -filetypes {{{LSD result files} {%s}} {{LSD total files} {%s}} {{All files} {*}} }]", extRes, extTot );
				cmd( "if { ! [ fn_spaces \"$lab\" .da 1 ] } { set choice [ llength $lab ] } { set choice 0 }" );
				if ( *choice == 0 )
					break; 			// no file selected
			
				h = *choice;
				
				for ( i = 0; i < h; ++i )  
				{
					cmd( "set datafile [lindex $lab %d]", i );
					app = ( char * ) Tcl_GetVar( inter, "datafile", 0 );
					strcpy( filename, app );
					
#ifdef LIBZ
					if ( strlen( filename ) > 3 && ! strcmp( &filename[ strlen( filename ) - 3 ], ".gz" ) )
						gz = true;
#endif
					
					f = fopen( filename, "r" );
				
					if ( f != NULL )
					{
						fclose( f );
						file_counter ++ ;
						insert_data_file( gz, &num_var, &num_c );
					}
					else
						plog( "\nError: could not open file: %s\n", "", filename );
				}
			}

			break;


		// MAIN MENU ACTIONS

		// open Gnuplot
		case 4:
			cmd( "open_gnuplot" );
			break;

			  
		// set options for gnuplot
		case 37: 
			cmd( "set sysTermTmp $sysTerm" );
			cmd( "set gptermTmp $gpterm" );
			cmd( "set gpdgrid3dTmp $gpdgrid3d" );
			
			cmd( "newtop .da.a \"Gnuplot Options\" { set choice 2 } .da" );
			cmd( "label .da.a.l -text \"Options for invoking Gnuplot\"" );

			cmd( "frame .da.a.st" );
			cmd( "label .da.a.st.l -text \"System terminal\"" );
			cmd( "entry .da.a.st.e -textvariable sysTermTmp -width 20 -justify center" );
			cmd( "pack .da.a.st.l .da.a.st.e -side left" );

			cmd( "frame .da.a.t" );
			cmd( "label .da.a.t.l -text \"Plot terminal (blank for default)\"" );
			cmd( "entry .da.a.t.e -textvariable gptermTmp -width 12 -justify center" );
			cmd( "pack .da.a.t.l .da.a.t.e -side left" );

			cmd( "frame .da.a.d" );
			cmd( "label .da.a.d.l -text \"3D grid configuration\"" );
			cmd( "entry .da.a.d.e -textvariable gpdgrid3dTmp -width 12 -justify center" );
			cmd( "pack .da.a.d.l .da.a.d.e -side left" );

			cmd( "frame .da.a.o" );
			cmd( "label .da.a.o.l -text \"Other options\"" );
			cmd( "text .da.a.o.t -undo 1 -height 10 -width 50 -font \"$font_small\"" );
			cmd( ".da.a.o.t insert end \"$gpoptions\"" );
			cmd( "pack .da.a.o.l .da.a.o.t" );

			cmd( "pack .da.a.l .da.a.st .da.a.t .da.a.d .da.a.o -pady 5 -padx 5" );
			cmd( "okXhelpcancel .da.a b  { Default } { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#gpoptions } { set choice 2 }" );

			cmd( "showtop .da.a" );

			gpoptions:
			
			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			if ( *choice == 3 )
			{
				cmd( "set sysTermTmp $systemTerm" );
				cmd( "set gptermTmp $gnuplotTerm" );
				cmd( "set gpdgrid3dTmp \"$gnuplotGrid3D\"" );
				cmd( ".da.a.o.t delete 1.0 end; .da.a.o.t insert end \"$gnuplotOptions\"" );
				goto gpoptions;
			}
			 
			if ( *choice == 1 )
			{
				cmd( "set sysTerm $sysTermTmp" );
				cmd( "set gpterm $gptermTmp" );
				cmd( "set gpdgrid3d $gpdgrid3dTmp" );
				cmd( "set gpoptions [.da.a.o.t get 0.0 end]" ); 
			}

			cmd( "destroytop .da.a" );
			
			break;

			
		// set colors
		case 21:		
			for ( i = 0; i < 20; ++i )
				cmd( "set current_c%d $c%d", i, i );

			cmd( "newtop .da.a \"Colors\" { set choice 2 } .da" );

			redraw_col:
			
			cmd( "label .da.a.l -text \"Pick the color to change\"" );

			cmd( "frame .da.a.o" );

			cmd( "frame .da.a.o.l1" );
			
			for ( i = 0; i < 10; ++i )
			{
				cmd( "frame .da.a.o.l1.c%d", i );
				cmd( "label .da.a.o.l1.c%d.n -text %d", i, i + 1 );
				cmd( "label .da.a.o.l1.c%d.c -text \"\u2588\u2588\u2588\u2588\u2588\u2588\" -fg $c%d", i, i );
				cmd( "bind .da.a.o.l1.c%d.c <Button-1> { set n_col %d; set col $c%d; set choice 4 }", i, i, i );
				cmd( "pack .da.a.o.l1.c%d.n .da.a.o.l1.c%d.c -side left", i, i );
				cmd( "pack .da.a.o.l1.c%d -anchor e", i );
			}
			 
			cmd( "frame .da.a.o.l2" );
			
			for ( i = 0; i < 10; ++i )
			{
				cmd( "frame .da.a.o.l2.c%d", i );
				cmd( "label .da.a.o.l2.c%d.n -text %d", i, i + 11 );
				cmd( "label .da.a.o.l2.c%d.c -text \"\u2588\u2588\u2588\u2588\u2588\u2588\" -fg $c%d", i, i + 10 );
				cmd( "bind .da.a.o.l2.c%d.c <Button-1> { set n_col %d; set col $c%d; set choice 4 }", i, i + 10, i + 10 );
				cmd( "pack .da.a.o.l2.c%d.n .da.a.o.l2.c%d.c -side left", i, i );
				cmd( "pack .da.a.o.l2.c%d -anchor e", i );
			}

			cmd( "pack .da.a.o.l1 .da.a.o.l2 -side left -pady 5 -padx 15" );
			cmd( "pack .da.a.l .da.a.o -pady 5 -padx 5" );

			cmd( "okXhelpcancel .da.a b Default { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#colors } { set choice 2 }" );
			cmd( "showtop .da.a" );

			set_col:
			
			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			switch ( *choice )
			{
				case 1:
					break;
				
				case 2:
					for ( i = 0; i < 20; ++i )
						cmd( "set c%d $current_c%d", i, i );
					break;
					
				case 3:
					cmd( "init_canvas_colors" );
					cmd( "destroy .da.a.l .da.a.o .da.a.b" );
					goto redraw_col;
					
				case 4:
					cmd( "set a [ tk_chooseColor -parent .da.a -title \"Choose Color\" -initialcolor $col ]" );
					cmd( "if { $a != \"\" } { set choice 1 } { set choice 0 }" );
					if ( *choice == 1 )
					{
						cmd( "set c$n_col $a" );
						cmd( "if { $n_col >= 10 } { set fr 2; set n_col [ expr $n_col - 10 ] } { set fr 1 }" );
						cmd( ".da.a.o.l$fr.c$n_col.c configure -fg $a" );
					}
					goto set_col;
			}

			cmd( "destroytop .da.a" );

			break;


		// set plot parameters
		case 22:			
			cmd( "if { ! [ info exists default_hsizeP ] } { set default_hsizeP $hsizeP }" );
			cmd( "if { ! [ info exists default_vsizeP ] } { set default_vsizeP $vsizeP }" );
			cmd( "if { ! [ info exists default_hsizePxy ] } { set default_hsizePxy $hsizePxy }" );
			cmd( "if { ! [ info exists default_vsizePxy ] } { set default_vsizePxy $vsizePxy }" );
			cmd( "if { ! [ info exists default_hmbordsizeP ] } { set default_hmbordsizeP $hmbordsizeP }" );
			cmd( "if { ! [ info exists default_tbordsizeP ] } { set default_tbordsizeP $tbordsizeP }" );
			cmd( "if { ! [ info exists default_bbordsizeP ] } { set default_bbordsizeP $bbordsizeP }" );
			cmd( "if { ! [ info exists default_hticksP ] } { set default_hticksP $hticksP }" );
			cmd( "if { ! [ info exists default_vticksP ] } { set default_vticksP $vticksP }" );
			cmd( "if { ! [ info exists default_fontP ] } { set default_fontP $fontP }" );
			cmd( "if { ! [ info exists default_smoothP ] } { set default_smoothP $smoothP }" );
			cmd( "if { ! [ info exists default_splstepsP ] } { set default_splstepsP $splstepsP }" );
				
			cmd( "newtop .da.s \"Plot Parameters\" { set choice 2 } .da" );

			cmd( "frame .da.s.x" );
			cmd( "label .da.s.x.l0 -text \"Plot width:\"" );
			cmd( "label .da.s.x.l1 -anchor e -text \"Sequence\"" );
			cmd( "set lx1 $hsizeP" ); 
			cmd( "entry .da.s.x.e1 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set lx1 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lx1; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.x.e1 insert 0 $lx1" ); 
			cmd( "label .da.s.x.l2 -anchor e -text \"  XY\"" );
			cmd( "set lx2 $hsizePxy" ); 
			cmd( "entry .da.s.x.e2 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set lx2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lx2; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.x.e2 insert 0 $lx2" ); 
			cmd( "pack .da.s.x.l0 .da.s.x.l1 .da.s.x.e1 .da.s.x.l2 .da.s.x.e2 -side left -padx 2 -pady 2" );

			cmd( "frame .da.s.y" );
			cmd( "label .da.s.y.l0 -anchor e -text \"Plot height:\"" );
			cmd( "label .da.s.y.l1 -anchor e -text \"Sequence\"" );
			cmd( "set ly1 $vsizeP" ); 
			cmd( "entry .da.s.y.e1 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set ly1 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ly1; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.y.e1 insert 0 $ly1" ); 
			cmd( "label .da.s.y.l2 -anchor e -text \"  XY\"" );
			cmd( "set ly2 $vsizePxy" ); 
			cmd( "entry .da.s.y.e2 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set ly2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ly2; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.y.e2 insert 0 $ly2" ); 
			cmd( "pack .da.s.y.l0 .da.s.y.l1 .da.s.y.e1 .da.s.y.l2 .da.s.y.e2 -side left -padx 2 -pady 2" );

			cmd( "frame .da.s.h" );
			cmd( "label .da.s.h.l -anchor e -text \"Horizontal borders\"" );
			cmd( "set hb $hmbordsizeP" ); 
			cmd( "entry .da.s.h.e -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set hb %%P; return 1 } { %%W delete 0 end; %%W insert 0 $hb; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.h.e insert 0 $hb" ); 
			cmd( "pack .da.s.h.l .da.s.h.e -side left -padx 2 -pady 2" );

			cmd( "frame .da.s.v" );
			cmd( "label .da.s.v.l0 -anchor e -text \"Vertical borders:\"" );
			cmd( "label .da.s.v.l1 -anchor e -text \"Top\"" );
			cmd( "set tb $tbordsizeP" ); 
			cmd( "entry .da.s.v.e1 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set tb %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tb; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.v.e1 insert 0 $tb" ); 
			cmd( "label .da.s.v.l2 -anchor e -text \"  Bottom\"" );
			cmd( "set bb $bbordsizeP" ); 
			cmd( "entry .da.s.v.e2 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set bb %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bb; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.v.e2 insert 0 $bb" ); 
			cmd( "pack .da.s.v.l0 .da.s.v.l1 .da.s.v.e1 .da.s.v.l2 .da.s.v.e2 -side left -padx 2 -pady 2" );

			cmd( "label .da.s.obs -text \"( all sizes are measured in screen pixels)\"" );

			cmd( "frame .da.s.t" );
			cmd( "label .da.s.t.l0 -anchor e -text \"Number of axis ticks:\"" );
			cmd( "label .da.s.t.l1 -anchor e -text \"Horizontal\"" );
			cmd( "set ht $hticksP" ); 
			cmd( "entry .da.s.t.e1 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set ht %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ht; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.t.e1 insert 0 $ht" ); 
			cmd( "label .da.s.t.l2 -anchor e -text \"  Vertical\"" );
			cmd( "set vt $vticksP" ); 
			cmd( "entry .da.s.t.e2 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set vt %%P; return 1 } { %%W delete 0 end; %%W insert 0 $vt; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.t.e2 insert 0 $vt" ); 
			cmd( "pack .da.s.t.l0 .da.s.t.l1 .da.s.t.e1 .da.s.t.l2 .da.s.t.e2 -side left -padx 2 -pady 2" );

			cmd( "frame .da.s.s" );
			cmd( "label .da.s.s.l1 -anchor e -text \"Smoothing\"" );
			cmd( "set sm $smoothP" ); 
			cmd( "ttk::combobox .da.s.s.e1 -textvariable sm -values [ list no yes raw ] -width 5 -justify center" );
			cmd( "label .da.s.s.l2 -anchor e -text \"  Spline segments\"" );
			cmd( "set ss $splstepsP" ); 
			cmd( "entry .da.s.s.e2 -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set ss %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ss; return 0 } } -invcmd { bell } -justify center -state disabled" );
			cmd( "if { $sm == \"raw\" } { .da.s.s.e2 configure -state normal }" );
			cmd( "write_any .da.s.s.e2 $ss" ); 
			cmd( "pack .da.s.s.l1 .da.s.s.e1 .da.s.s.l2 .da.s.s.e2 -side left -padx 2 -pady 2" );

			cmd( "frame .da.s.f" );
			cmd( "label .da.s.f.l -text \"Font name, size and style\"" );

			cmd( "frame .da.s.f.e" );
			cmd( "set ifont [ lindex $fontP 0 ]" ); 
			cmd( "ttk::combobox .da.s.f.e.font -textvariable ifont -values [ font families ] -width 15" );
			cmd( "set idim [ lindex $fontP 1 ]" ); 
			cmd( "ttk::combobox .da.s.f.e.dim -textvariable idim -values [ list 4 6 8 9 10 11 12 14 18 24 32 48 60 ] -width 3 -justify center" );
			cmd( "set istyle [ lindex $fontP 2 ]" ); 
			cmd( "ttk::combobox .da.s.f.e.sty -textvariable istyle -values [ list normal bold italic \"bold italic\" ] -width 10 -justify center" );
			cmd( "pack .da.s.f.e.font .da.s.f.e.dim .da.s.f.e.sty -padx 2 -side left" );

			cmd( "pack .da.s.f.l .da.s.f.e -side left -padx 2 -pady 2" );

			cmd( "pack .da.s.x .da.s.y .da.s.h .da.s.v .da.s.obs .da.s.t .da.s.s .da.s.f -padx 5 -pady 5" );

			cmd( "okXhelpcancel .da.s b Default { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#plotparameters } { set choice 2 }" );

			cmd( "bind .da.s.x.e1 <KeyPress-Return> { focus .da.s.x.e2; .da.s.x.e2 selection range 0 end }" );
			cmd( "bind .da.s.x.e2 <KeyPress-Return> { focus .da.s.y.e1; .da.s.y.e1 selection range 0 end }" );
			cmd( "bind .da.s.y.e1 <KeyPress-Return> { focus .da.s.y.e2; .da.s.y.e2 selection range 0 end }" );
			cmd( "bind .da.s.y.e2 <KeyPress-Return> { focus .da.s.h.e; .da.s.h.e selection range 0 end }" );
			cmd( "bind .da.s.h.e <KeyPress-Return> { focus .da.s.v.e1; .da.s.v.e1 selection range 0 end }" );
			cmd( "bind .da.s.v.e1 <KeyPress-Return> { focus .da.s.v.e2; .da.s.v.e2 selection range 0 end }" );
			cmd( "bind .da.s.v.e2 <KeyPress-Return> { focus .da.s.t.e1; .da.s.t.e1 selection range 0 end }" );
			cmd( "bind .da.s.t.e1 <KeyPress-Return> { focus .da.s.t.e2; .da.s.t.e2 selection range 0 end }" );
			cmd( "bind .da.s.t.e2 <KeyPress-Return> { set choice 1 }" );
			cmd( "bind .da.s.s.e1 <<ComboboxSelected>> { if { $sm == \"raw\" } { .da.s.s.e2 configure -state normal } { .da.s.s.e2 configure -state disabled } }" );

			cmd( "showtop .da.s" );
			cmd( "focus .da.s.x.e1; .da.s.x.e1 selection range 0 end" );

			set_plot:
			
			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			if ( *choice == 1 )
			{
				cmd( "set hsizeP [ .da.s.x.e1 get ]" ); 
				cmd( "set hsizePxy [ .da.s.x.e2 get ]" ); 
				cmd( "set vsizeP [ .da.s.y.e1 get ]" ); 
				cmd( "set vsizePxy [ .da.s.y.e2 get ]" ); 
				cmd( "set hmbordsizeP [ .da.s.h.e get ]" ); 
				cmd( "set tbordsizeP [ .da.s.v.e1 get ]" ); 
				cmd( "set bbordsizeP [ .da.s.v.e2 get ]" ); 
				cmd( "set hticksP [ .da.s.t.e1 get ]" ); 
				cmd( "set vticksP [ .da.s.t.e2 get ]" ); 
				cmd( "set smoothP $sm" ); 
				cmd( "set splstepsP [ .da.s.s.e2 get ]" ); 
				cmd( "set fontP [ list $ifont $idim $istyle ]" ); 
			}

			if ( *choice == 3 )
			{
				cmd( "write_any .da.s.x.e1  $default_hsizeP" );
				cmd( "write_any .da.s.x.e2  $default_hsizePxy" );
				cmd( "write_any .da.s.y.e1  $default_vsizeP" );
				cmd( "write_any .da.s.y.e2  $default_vsizePxy" );
				cmd( "write_any .da.s.h.e  $default_hmbordsizeP" );
				cmd( "write_any .da.s.v.e1  $default_tbordsizeP" );
				cmd( "write_any .da.s.v.e2  $default_bbordsizeP" );
				cmd( "write_any .da.s.t.e1  $default_hticksP" );
				cmd( "write_any .da.s.t.e2  $default_vticksP" );
				cmd( "set sm  $default_smoothP" );
				cmd( "write_any .da.s.s.e2  $default_splstepsP" );
				cmd( "set ifont [ lindex $default_fontP 0 ]" );
				cmd( "set idim [ lindex $default_fontP 1 ]" );
				cmd( "set istyle [ lindex $default_fontP 2 ]" );
				goto set_plot;
			}

			cmd( "destroytop .da.s" );

			break;

		// lattice parameters
		case 44:		
			cmd( "if { ! [ info exists default_hsizeLat ] } { set default_hsizeLat $hsizeLat }" );
			cmd( "if { ! [ info exists default_vsizeLat ] } { set default_vsizeLat $vsizeLat }" );
			cmd( "if { ! [ info exists default_cscaleLat ] } { set default_cscaleLat $cscaleLat }" );
				
			cmd( "newtop .da.s \"Lattice Parameters\" { set choice 2 } .da" );

			cmd( "frame .da.s.s" );
			cmd( "label .da.s.s.l -width 30 -anchor e -text \"Color scale\"" );
			cmd( "set cs $cscaleLat" ); 
			cmd( "entry .da.s.s.e -width 5 -validate focusout -vcmd { if [ string is double -strict %%P ] { set cs %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cs; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.s.e insert 0 $cs" ); 
			cmd( "pack .da.s.s.l .da.s.s.e -side left -anchor w -padx 2 -pady 2" );

			cmd( "frame .da.s.x" );
			cmd( "label .da.s.x.l -width 30 -anchor e -text \"Lattice width (pixels)\"" );
			cmd( "set lx $hsizeLat" ); 
			cmd( "entry .da.s.x.e -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set lx %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lx; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.x.e insert 0 $lx" ); 
			cmd( "pack .da.s.x.l .da.s.x.e -side left -anchor w -padx 2 -pady 2" );

			cmd( "frame .da.s.y" );
			cmd( "label .da.s.y.l -width 30 -anchor e -text \"Lattice heigth (pixels)\"" );
			cmd( "set ly $vsizeLat" ); 
			cmd( "entry .da.s.y.e -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set ly %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ly; return 0 } } -invcmd { bell } -justify center" );
			cmd( ".da.s.y.e insert 0 $ly" ); 
			cmd( "pack .da.s.y.l .da.s.y.e -side left -anchor w -padx 2 -pady 2" );

			cmd( "pack .da.s.s .da.s.x .da.s.y -anchor w -padx 5 -pady 5" );

			cmd( "okXhelpcancel .da.s b Default { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#latticeparameters } { set choice 2 }" );

			cmd( "bind .da.s.s.e <KeyPress-Return> { focus .da.s.x.e; .da.s.x.e selection range 0 end }" );
			cmd( "bind .da.s.x.e <KeyPress-Return> { focus .da.s.y.e; .da.s.y.e selection range 0 end }" );
			cmd( "bind .da.s.y.e <KeyPress-Return> { set choice 1 }" );

			cmd( "showtop .da.s" );
			cmd( "focus .da.s.s.e; .da.s.s.e selection range 0 end" );

			set_lattice:
			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			if ( *choice == 1 )
			{
				cmd( "set cscaleLat [ .da.s.s.e get ]" );
				cmd( "set hsizeLat [ .da.s.x.e get ]" );
				cmd( "set vsizeLat [ .da.s.y.e get ]" );
			}

			if ( *choice == 3 )
			{
				cmd( "write_any .da.s.s.e $default_cscaleLat" ); 
				cmd( "write_any .da.s.x.e $default_hsizeLat" ); 
				cmd( "write_any .da.s.y.e $default_vsizeLat" );
				goto set_lattice;
			}

			cmd( "destroytop .da.s" );

			break;


		// help on Analysis of Result
		case 41:
			cmd( "LsdHelp menudata_res.html" );
			
			break;


		// see model report
		case 43:
			sprintf( name_rep, "report_%s.html", simul_name );
			cmd( "set choice [file exists %s]", name_rep );
			if ( *choice == 0 )
			{
				cmd( "set answer [tk_messageBox -parent .da -message \"Model report not found\" -detail \"You may create a model report file from menu Model or press 'OK' to look for another HTML file.\" -type okcancel -title Warning -icon warning -default cancel]" );
				cmd( "if {[string compare -nocase $answer \"ok\"] == 0} {set choice 1} {set choice 0}" );
				if ( *choice == 0 )
					break;

				// make sure there is a path set
				cmd( "set path \"%s\"", path );
				if ( strlen( path ) > 0 )
					cmd( "cd \"$path\"" );

				cmd( "set fname [tk_getOpenFile -parent .da -title \"Load Report File\" -defaultextension \".html\" -initialdir \"$path\" -filetypes {{{HTML files} {.html}} {{All files} {*}} }]" );
				cmd( "if { $fname == \"\" || [ fn_spaces \"$fname\" .da ] } { set choice 0 } { set choice 1 }" );
				if ( *choice == 0 )
					break;
			}
			else
				cmd( "set fname %s", name_rep );

			if ( *choice == 1 ) // model report exists
				cmd( "LsdHtml $fname" );
			  
			break;


		// CANVAS OPTIONS

		// Edit labels
		case 26:
			cmd( "set itext [$ccanvas itemcget current -text]" );
			cmd( "set ifont [lindex [$ccanvas itemcget current -font] 0]" );
			cmd( "set idim [lindex [$ccanvas itemcget current -font] 1 ]" );
			cmd( "set istyle [lindex [$ccanvas itemcget current -font] 2 ]" );
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
			
			cmd( "okXhelpcancel $w b Delete { set itext \"\"; set choice 1 } { set choice 1 } { LsdHelp menudata_res.html#graph } { set choice 2 }" );
			
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
			
			if ( *choice == 1 )
			{
				cmd( "if { $itext==\"\" } {$ccanvas delete current; set choice 0}" );
				if ( *choice == 1 )
				{
					cmd( "$ccanvas itemconf current -text \"$itext\"" );
					cmd( "set ml [list $ifont $idim $istyle]" );
					cmd( "$ccanvas itemconf current -font \"$ml\"" );
					cmd( "$ccanvas itemconf current -fill $icolor" );  
				}
				
				cmd( "set choice $fontall" );
				if ( *choice == 1 )
					cmd( "$ccanvas itemconf text -font \"$ml\"" );
				cmd( "set choice $colorall" );
				if ( *choice == 1 )
					cmd( "$ccanvas itemconf text -fill $icolor" );
			} 
			
			cmd( "destroytop $w" ); 
			
			break;


		// New labels
		case 27:
			cmd( "set itext \"new text\"" );
			
			cmd( "set w $ccanvas.a" );
			cmd( "newtop $w \"New Labels\" { set choice 2 } $ccanvas" );
			cmd( "wm geometry $w +$LX+$LY" );
			
			cmd( "frame $w.l" );
			cmd( "label $w.l.t -text \"New label\"" );
			cmd( "entry $w.l.e -textvariable itext -width 30 -justify center" );
			cmd( "pack $w.l.t $w.l.e" );
			cmd( "pack $w.l -padx 5 -pady 5" );
			
			cmd( "okhelpcancel $w b { set choice 1 } { LsdHelp menudata_res.html#graph } { set choice 2 }" );
			
			cmd( "bind $w.l.e <Return> { $w.b.ok invoke}" );
			
			cmd( "showtop $w current" );
			cmd( "focus $w.l.e" );
			cmd( "$w.l.e selection range 0 end" );
			
			*choice = 0;
			while ( ! *choice )
				Tcl_DoOneEvent( 0 );
			
			cmd( "destroytop $w" ); 
			
			if ( *choice == 1 )
				cmd( "$ccanvas create text $hereX $hereY -text \"$itext\" -font $fontP -tags { text draw }" );
			
			break;


		// Edit line
		case 31:
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
			cmd( "entry $w.l.e -textvariable iwidth -width 5 -justify center -state disabled -validate focusout -vcmd { if [ string is double -strict %%P ] { set iwidth %%P; return 1 } { %%W delete 0 end; %%W insert 0 $iwidth; return 0 } } -invcmd { bell }" );
			cmd( "pack $w.l.t $w.l.e -padx 2 -side left" );
			
			cmd( "frame $w.c" );
			cmd( "label $w.c.l -text \"Color\"" );
			cmd( "button $w.c.color -width 5 -text Set -foreground white -background $icolor -command { set app [ tk_chooseColor -parent $w -title \"Line Color\" -initialcolor $icolor ]; if { $app != \"\" } { set icolor $app }; $w.c.color configure -background $icolor }" );
			cmd( "label $w.c.t -text \"   Dash pattern\"" );
			cmd( "ttk::combobox $w.c.e -textvariable idash -values [ list \"\" \". \" \"- \" \"-.\" \"-..\" ] -width 3 -justify center -state disabled" ); 
			cmd( "pack $w.c.l $w.c.color $w.c.t $w.c.e -padx 2 -side left" );
			
			cmd( "frame $w.d" );
			cmd( "label $w.d.t -text \"Line-end arrow( s)\"" );
			cmd( "ttk::combobox $w.d.e -textvariable iarrow -values [ list none first last both ] -width 7 -state disabled" );
			cmd( "pack $w.d.t $w.d.e -padx 2 -side left" );
			
			cmd( "frame $w.fall" );
			cmd( "checkbutton $w.fall.font -text \"Apply width to all line items\" -variable widthall -state disabled" );
			cmd( "checkbutton $w.fall.color -text \"Apply color to all line items\" -variable colorall" );
			cmd( "pack $w.fall.font $w.fall.color" );
			
			cmd( "pack $w.l $w.c $w.d $w.fall -padx 5 -pady 5" );
			
			cmd( "okXhelpcancel $w b Delete { set iwidth 0; set choice 1 } { set choice 1 } { LsdHelp menudata_res.html#graph } { set choice 2 }" );
			
			cmd( "bind $w.l.e <Return> { $w.b.ok invoke }" );
			cmd( "bind $w.c.e <Return> { $w.b.ok invoke }" );
			cmd( "bind $w.d.e <Return> { $w.b.ok invoke }" );
			
			cmd( "showtop $w current" );
			cmd( "focus $w.l.e" );
			cmd( "$w.l.e selection range 0 end" );
		 
			// enable most options for non-dotted lines
			cmd( "if { ! $dots } { $w.l.e  configure -state normal; $w.c.e  configure -state normal; $w.fall.font  configure -state normal }" );
			// enable dashes & arrows for drawing lines only
			cmd( "if $draw { $w.d.e  configure -state normal }" );

			*choice = 0;
			while ( ! *choice )
				Tcl_DoOneEvent( 0 );
			
			if ( *choice == 1 )
			{
				cmd( "if { $iwidth == 0 } { $ccanvas delete $cline; set choice 0 } { if $dots { set choice 2 } }" );
				if ( *choice == 1 )	// regular line made of lines & dots
				{	// avoid changing the dots parts in regular lines
					cmd( "$ccanvas dtag selected" );
					cmd( "$ccanvas addtag selected withtag \"$cline&&line\"" );
					cmd( "$ccanvas itemconf selected -width $iwidth" );
					cmd( "$ccanvas itemconf selected -dash \"$idash\"" );
					cmd( "$ccanvas itemconf selected -arrow $iarrow" );
					cmd( "$ccanvas dtag selected" );
					
					cmd( "$ccanvas itemconf $cline -fill $icolor" );  
				}
				if ( *choice == 2 )	// line of dots
					cmd( "$ccanvas itemconf $cline -fill $icolor" );  
				
				cmd( "set choice $widthall" );
				if ( *choice == 1 )
					cmd( "$ccanvas itemconf line -width $iwidth" );
				
				cmd( "set choice $colorall" );
				if ( *choice == 1 )
					cmd( "$ccanvas itemconf line -fill $icolor" );
			} 
			
			cmd( "destroytop $w" ); 
			
			break;


		// Insert a line item
		case 28: 
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
					
			break;


		// Edit bar
		case 42:
			cmd( "set iwidth [ $ccanvas itemcget current -width ]" );
			cmd( "set icolor1 [$ccanvas itemcget current -outline ]" );
			cmd( "set icolor2 [$ccanvas itemcget current -fill ]" );
			cmd( "set widthall 0" );
			cmd( "set colorall 0" );
			
			cmd( "set w $ccanvas.a" );
			cmd( "newtop $w \"Edit Bar\" { set choice 2 } $ccanvas" );
			cmd( "wm geometry $w +$LX+$LY" );
			
			cmd( "frame $w.c" );
			cmd( "label $w.c.l -text \"Fill color\"" );
			cmd( "button $w.c.color -width 5 -text Set -foreground white -background $icolor2 -command { set app [ tk_chooseColor -parent $w -title \"Fill Color\" -initialcolor $icolor2 ]; if { $app != \"\" } { set icolor2 $app }; $w.c.color configure -background $icolor2 }" );
			cmd( "pack $w.c.l $w.c.color -padx 2 -side left" );
			
			cmd( "frame $w.l" );
			cmd( "label $w.l.t -text \"Outline width\"" );
			cmd( "entry $w.l.e -textvariable iwidth -width 5 -justify center -validate focusout -vcmd { if [ string is double -strict %%P ] { set iwidth %%P; return 1 } { %%W delete 0 end; %%W insert 0 $iwidth; return 0 } } -invcmd { bell }" );
			cmd( "label $w.l.l -text \" color\"" );
			cmd( "button $w.l.color -width 5 -text Set -foreground white -background $icolor1 -command { set app [ tk_chooseColor -parent $w -title \"Outline Color\" -initialcolor $icolor1 ]; if { $app != \"\" } { set icolor1 $app }; $w.l.color configure -background $icolor1 }" );
			cmd( "pack $w.l.t $w.l.e $w.l.l $w.l.color -padx 2 -side left" );
			
			cmd( "frame $w.fall" );
			cmd( "checkbutton $w.fall.color -text \"Apply fill to all bar items\" -variable colorall" );
			cmd( "checkbutton $w.fall.font -text \"Apply outline to all bar items\" -variable widthall" );
			cmd( "pack $w.fall.color $w.fall.font" );
			
			cmd( "pack $w.c $w.l $w.fall -padx 5 -pady 5" );
			
			cmd( "okXhelpcancel $w b Delete { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#graph } { set choice 2 }" );
			
			cmd( "bind $w.l.e <Return> { $w.b.ok invoke }" );
			
			cmd( "showtop $w current" );
			cmd( "focus $w.l.e" );
			cmd( "$w.l.e selection range 0 end" );
			
			*choice = 0;
			while ( ! *choice )
				Tcl_DoOneEvent( 0 );
			
			if ( *choice == 1 )
			{
				cmd( "$ccanvas itemconf current -width $iwidth" );
				cmd( "$ccanvas itemconf current -outline $icolor1" );
				cmd( "$ccanvas itemconf current -fill $icolor2" );
				
				cmd( "set choice $widthall" );
				if ( *choice == 1 )
				{
					cmd( "$ccanvas itemconf bar -width $iwidth" );
					cmd( "$ccanvas itemconf bar -outline $icolor1" );
				}
				
				cmd( "set choice $colorall" );
				if ( *choice == 1 )
				cmd( "$ccanvas itemconf bar -fill $icolor2" );
			} 
			
			if ( *choice == 3 )
				cmd( "$ccanvas delete current" );
			
			cmd( "destroytop $w" ); 
			
			break;


		default:
			break;
	}
}
}


/***************************************************
 UPDATE_BOUNDS
 ****************************************************/
void update_bounds( void )
{
	if ( isfinite( miny ) )
		cmd( "write_any .da.f.h.v.sc.min.min [ format \"%%.[ expr $pdigits ]g\" $miny ]" );
	else
	{
		cmd( "write_any .da.f.h.v.sc.min.min -Infinity" );
		miny = 0;
	}
	
	if ( isfinite( maxy ) )
		cmd( "write_any .da.f.h.v.sc.max.max [ format \"%%.[ expr $pdigits ]g\" $maxy ]" );
	else
	{
		cmd( "write_any .da.f.h.v.sc.max.max Infinity" );
		maxy = 0;
	}
	
	if ( miny == 0 && maxy == 0 )
	{
		miny = -1;
		maxy = 1;
		cmd( "write_any .da.f.h.v.sc.min.min $miny" );
		cmd( "write_any .da.f.h.v.sc.max.max $maxy" );
	}
	
	if ( ! isfinite( miny2 ) )
		miny2 = 0;
	
	if ( ! isfinite( maxy2 ) )
		maxy2 = 0;		
	
	if ( miny2 == 0 && maxy2 == 0 )
	{
		miny2 = -1;
		maxy2 = 1;
	}
	
	if ( min_c < 1 )
		min_c = 1;
	
	if ( max_c <= min_c )
		max_c = min_c + 1;

	cmd( "write_any .da.f.h.v.ft.from.mnc $minc" );
	cmd( "write_any .da.f.h.v.ft.to.mxc $maxc" );
}
	

/***************************************************
 PLOT_TSERIES
 ****************************************************/
void plot_tseries( int *choice )
{
	bool y2on, done, stopErr = false;
	char *app, **str, **tag;
	double temp, **data, **logdata;
	int i, j, *start, *end, *id, logErrCnt = 0;

	if ( nv > 1000 )
	{
		cmd( "set answer [tk_messageBox -parent .da -type okcancel -title \"Too Many Series\" -icon warning -default ok -message \"You selected too many ($nv) series to plot\" -detail \"This may cause a crash of LSD, with the loss of all unsaved data.\nIf you continue the system may become slow, please be patient.\nPress 'OK' to continue anyway.\"]" );
		cmd( "if { [ string compare $answer ok ] == 0 } { set choice 1 } { set choice 2 }" );
		if ( *choice == 2 )
			return;
	}
	 
	if ( nv == 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}

	data = new double *[ nv ];
	logdata = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		logdata[ i ] = NULL;

		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
	  
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
				plog( "\nError: invalid data\n" );
	   
			if ( logs )			// apply log to the values to show "log scale" in the y-axis
			{
				logdata[ i ] = new double[ end[ i ] + 1 ];	// create space for the logged values
				for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
				if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )		// ignore NaNs
					logdata[ i ][ j ] = log( data[ i ][ j ] );
				else
				{
					logdata[ i ][ j ] = NAN;
					if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
						plog( "\nWarning: zero or negative values in log plot (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
					else
						if ( ! stopErr )
						{
							plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
							stopErr = true;
						}
				}
				
				data[ i ] = logdata[ i ];			// replace the data series
			}
		}
	}

	// handle case selection
	if ( autom_x || min_c >= max_c )
	{
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = max( start[ i ], 1 );

			if ( start[ i ] < min_c )
				min_c = max( start[ i ], 1 );
			
			if ( end[ i ] > max_c )
				max_c = end[ i ] > num_c ? num_c : end[ i ];
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
	if ( miny >= maxy )
		autom = true;

	if ( autom )
	{
		for ( done = false, i = 0; i < num_y2 - 1 && i < nv; ++i )
			for ( j = min_c; j <= max_c; ++j )
			{
				if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) )		// ignore NaNs
				{
					miny = maxy = data[ i ][ j ];
					done = true;
				}
				
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] < miny )		// ignore NaNs
					miny = data[ i ][ j ];
					
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] > maxy )		// ignore NaNs
					maxy = data[ i ][ j ];
			}
		
		// condition the max and min values 
		temp = lower_bound( miny, maxy, MARG, MARG_CONST, pdigits );
		maxy = upper_bound( miny, maxy, MARG, MARG_CONST, pdigits );
		miny = temp;
	}
	else
	{
		// condition the max and min values 
		temp = lower_bound( miny, maxy, 0, MARG_CONST, pdigits );
		maxy = upper_bound( miny, maxy, 0, MARG_CONST, pdigits );
		miny = temp;
	}

	// 2nd y axis is always automatic scaled
	for ( miny2 = maxy2 = 0, done = false, i = num_y2 - 1; i < nv; ++i )
		for ( j = min_c; j <= max_c; ++j )
		{
			if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) )		// ignore NaNs
			{
				miny2 = maxy2 = data[ i ][ j ];
				done = true;
			}
			
			if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] < miny2 )		// ignore NaNs
				miny2 = data[ i ][ j ];
				
			if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] > maxy2 )		// ignore NaNs
				maxy2 = data[ i ][ j ];
		}
		
	// check if not all invalid data
	if ( miny2 == 0 && maxy2 == 0 )
	{
		num_y2 = nv + 1;
		y2on = false;
	}
	else
	{			
		// condition the max and min values (2nd axis)
		temp = lower_bound( miny2, maxy2, MARG, MARG_CONST, pdigits );
		maxy2 = upper_bound( miny2, maxy2, MARG, MARG_CONST, pdigits );
		miny2 = temp;
	}
		
	update_bounds( );
	
	// plot all series
	plot( TSERIES, nv, data, start, end, id, str, tag, choice );

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/***************************************************
PLOT_CROSS
****************************************************/
void plot_cross( int *choice )
{
	bool first, stopErr = false;
	char *app, **str, **tag;
	double temp, **val, **data, **logdata;
	int i, j, k, nt, new_nv, *list_times, *pos, *start, *end, *id, *erase, logErrCnt = 0;

	cmd( "if [ info exists num_t ] { set nt $num_t } { set nt \"-1\" }" );
	get_int( "nt", & nt );
	
	if ( nv < 2 || nt <= 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series/time steps selected\" -detail \"Place at least two series in the Series Selected listbox and select at least one time step (case ).\"" );
		*choice = 2;
		return;
	}

	list_times = new int [ nt ];
	pos = new int [ nv ];
	val = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	erase = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];
	data = new double *[ nv ];
	logdata = new double *[ nv ];

	for ( i = 0; i < nt; ++i )
	{
		cmd( "set k [ lindex $list_times %d ]", i );
		get_int( "k", & k );
		list_times[ i ] = k;
	}
	
	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0, new_nv = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		logdata[ i ] = NULL;

		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// check if series has data for all CS selected cases
		for ( k = 0, erase[ i ] = 0; k < nt; ++k )
			if ( list_times[ k ] < start[ i ] || list_times[ k ] > end[ i ] )
			{
				erase[ i ] = 1;
				break;
			}
		
		// get series data and take logs if necessary
		if ( erase[ i ] == 0 )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
				plog( "\nError: invalid data\n" );
	   
			if ( logs )			// apply log to the values to show "log scale"
			{
				logdata[ new_nv ] = new double[ end[ i ] + 1 ];	// create space for the logged values
				for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
					if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )		// ignore NaNs
						logdata[ new_nv ][ j ] = log( data[ i ][ j ] );
					else
					{
						logdata[ new_nv ][ j ] = NAN;
						if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
							plog( "\nWarning: zero or negative values in log plot (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
						else
							if ( ! stopErr )
							{
								plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
								stopErr = true;
							}
					}
					
				data[ i ] = logdata[ new_nv ];		// replace the data series
			}
			
			val[ new_nv ] = new double[ nt ];
			new_nv++;
		}
		else
			data[ i ] = NULL;						// discard series not in all selected cases
	}
	 
	// organize useful/valid data in 'val' matrix and find max/mins
	if ( miny >= maxy )
		autom = true;

	for ( j = 0, first = true, i = 0; j < nv; ++j )
	{
		for ( k = 0; k < nt; ++k )
			if ( erase[ j ] == 0 && is_finite( data[ j ][ list_times[ k ] ] ) )		// ignore NaNs
			{
				val[ i ][ k ] = data[ j ][ list_times[ k ] ];
			
				// auto-find minimums and maximums
				if ( autom )
				{

					if ( first )  //The first value is assigned to both min and max
					{
						miny = maxy = val[ i ][ k ];
						first = false;
					}
					
					if ( miny > val[ i ][ k ] )
						miny = val[ i ][ k ];
					
					if ( maxy < val[ i ][ k ] )
						maxy = val[ i ][ k ];
				}
			}
			else
			{
				if ( ! erase[ j ] )	// mark NANs
					val[ i ][ k ] = NAN;
			}
		
		if ( erase[ j ] == 0 )
			++i;
	}

	// condition the max and min values 
	if ( autom )
	{
		temp = lower_bound( miny, maxy, MARG, MARG_CONST, pdigits );
		maxy = upper_bound( miny, maxy, MARG, MARG_CONST, pdigits );
		miny = temp;
	}
	else
	{
		temp = lower_bound( miny, maxy, 0, MARG_CONST, pdigits );
		maxy = upper_bound( miny, maxy, 0, MARG_CONST, pdigits );
		miny = temp;
	}
		
	update_bounds( );
	
	// sort series if required
	for ( k = 0; k < nt; ++k )		// find index to time reference
		if ( list_times[ k ] == res )
			break;

	if ( k < nt )
	{
		if ( dir == -1 )
			sort_cs_desc( str, tag, val, new_nv, nt, k );
		if ( dir == 1 )
			sort_cs_asc( str, tag, val, new_nv, nt, k );
	}

	// plot all series
	plot( CRSSECT, new_nv, val, list_times, &nt, id, str, tag, choice );

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
	}

	for ( i = 0; i < new_nv; ++i )
	{
		delete [ ] val[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}

	delete [ ] list_times;
	delete [ ] pos;
	delete [ ] val;
	delete [ ] erase;
	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/***************************************************
SET_CS_DATA
****************************************************/
void set_cs_data( int *choice )
{
	if ( nv < 2 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place two or more series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}
		
	Tcl_LinkVar( inter, "res", ( char * ) &res, TCL_LINK_INT );
	Tcl_LinkVar( inter, "dir", ( char * ) &dir, TCL_LINK_INT );

	cmd( "set bidi $maxc" );
	cmd( "set res $maxc" );
	cmd( "set dir 0" );
	cmd( "unset -nocomplain sfrom sto sskip" );
	
	cmd( "set list_times_new [ list ]" );
	cmd( "for { set i 0 } { $i < [ llength $list_times ] } { incr i } { \
			set x [ lindex $list_times $i ]; \
			if { $x >= $minc && $x <= $maxc } { \
				lappend list_times_new $x \
			} \
		}" );
	cmd( "set list_times $list_times_new" );

	cmd( "set p .da.s" );
	cmd( "newtop $p \"Cross Section Time Steps\" { set choice 2 } .da" );

	cmd( "frame $p.u" );

	cmd( "frame $p.u.i" );

	cmd( "frame $p.u.i.e" );
	cmd( "label $p.u.i.e.l -text \"Time step to add\"" );
	cmd( "entry $p.u.i.e.e -width 10 -validate focusout -vcmd { if { [ string is integer -strict %%P ] && %%P >= $minc && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
	cmd( "$p.u.i.e.e insert 0 $bidi" );
	cmd( "pack $p.u.i.e.l $p.u.i.e.e" );
	 
	cmd( "frame $p.u.i.lb" );
	cmd( "label $p.u.i.lb.l -text \"Selected time steps\"" );

	cmd( "frame $p.u.i.lb.lb" );
	cmd( "scrollbar $p.u.i.lb.lb.v_scroll -command \".da.s.u.i.lb.lb.lb yview\"" );
	cmd( "listbox $p.u.i.lb.lb.lb -listvariable list_times -width 8 -height 7 -selectmode extended -yscroll \".da.s.u.i.lb.lb.v_scroll set\"" );
	cmd( "mouse_wheel $p.u.i.lb.lb.lb" );
	cmd( "pack $p.u.i.lb.lb.lb $p.u.i.lb.lb.v_scroll -side left -fill y" );

	cmd( "pack $p.u.i.lb.l $p.u.i.lb.lb" );

	cmd( "pack $p.u.i.e $p.u.i.lb -padx 5 -pady 5" );

	cmd( "frame $p.u.s" );
	cmd( "label $p.u.s.l -text \"\nTime series order\"" );
	cmd( "pack $p.u.s.l" );

	cmd( "frame $p.u.s.b -relief groove -bd 2" );
	cmd( "radiobutton $p.u.s.b.nosort -text \"As selected\" -variable dir -value 0 -command { .da.s.u.s.r.e configure -state disabled; .da.s.u.i.e.e selection range 0 end; focus .da.s.u.i.e.e }" );
	cmd( "radiobutton $p.u.s.b.up -text \"Ascending order\" -variable dir -value 1 -command { set sel [ .da.s.u.i.lb.lb.lb curselection ]; if { [ llength $sel ] == 1 } { set res [ lindex $list_times $sel ] } { set res $maxc }; .da.s.u.s.r.e configure -state normal; .da.s.u.s.r.e delete 0 end; .da.s.u.s.r.e insert 0 $res; .da.s.u.s.r.e selection range 0 end; focus .da.s.u.s.r.e }" );
	cmd( "radiobutton $p.u.s.b.down -text \"Descending order\" -variable dir -value \"-1\" -command { set sel [ .da.s.u.i.lb.lb.lb curselection ]; if { [ llength $sel ] == 1 } { set res [ lindex $list_times $sel ] } { set res $maxc }; .da.s.u.s.r.e configure -state normal; .da.s.u.s.r.e delete 0 end; .da.s.u.s.r.e insert 0 $res; .da.s.u.s.r.e selection range 0 end; focus .da.s.u.s.r.e }" );
	cmd( "pack $p.u.s.b.nosort $p.u.s.b.up $p.u.s.b.down -padx 5 -anchor w" );
	cmd( "pack $p.u.s.b -padx 5" );

	cmd( "frame $p.u.s.r" );
	cmd( "label $p.u.s.r.l -text \"Time step reference\nfor series sorting\"" );
	cmd( "entry $p.u.s.r.e -width 10 -validate focusout -vcmd { if { [ string is integer -strict %%P ] && [ lsearch $list_times %%P ]  >= 0 } { set res %%P; return 1 } { %%W delete 0 end; %%W insert 0 $res; return 0 } } -invcmd { bell } -justify center -state disabled" );
	cmd( "write_disabled $p.u.s.r.e $res" );
	cmd( "pack $p.u.s.r.l $p.u.s.r.e" );
	cmd( "pack $p.u.s.r -pady 10" );

	cmd( "pack $p.u.i $p.u.s -side left -anchor n" );
	cmd( "pack $p.u -pady 5" );

	cmd( "bind $p.u.i.e.e <KeyPress-Return> { .da.s.fb.r1.x invoke }" );
	cmd( "bind $p.u.i.lb.lb.lb <Delete> { .da.s.fb.r1.y invoke }" );

	cmd( "bind $p.u.i.e.e <Control-f> { set sfrom [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }; bind $p.u.i.e.e <Control-F> { set sfrom [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }" );
	cmd( "bind $p.u.i.e.e <Control-t> { set sto [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }; bind $p.u.i.e.e <Control-T> { set sto [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }" );
	cmd( "bind $p.u.i.e.e <Control-s> { set sskip [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }; bind $p.u.i.e.e <Control-S> { set sskip [ .da.s.u.i.e.e get ]; .da.s.u.i.e.e selection range 0 end }" );
	cmd( "bind $p.u.i.e.e <Control-z> { \
			if { [ info exists sfrom ] && [ info exists sto ] && [ string is integer -strict $sfrom ] && [ string is integer -strict $sto ] && [ expr $sto - $sfrom ] > 0 } { \
				if { ! [ info exists sskip ] } { \
					set sskip 1 \
				}; \
				for { set x $sfrom } { $x <= $sto && $x <= $maxc } { incr x $sskip } { \
					.da.s.u.i.lb.lb.lb insert end $x \
				} \
			}; \
			.da.s.u.i.e.e selection range 0 end \
		}; bind $p.u.i.e.e <Control-Z> { \
			if { [ info exists sfrom ] && [ info exists sto ] && [ string is integer -strict $sfrom ] && [ string is integer -strict $sto ] && [ expr $sto - $sfrom ] > 0 } { \
				if { ! [ info exists sskip ] } { \
					set sskip 1 \
				}; \
				for { set x $sfrom } { $x <= $sto && $x <= $maxc } { incr x $sskip } { \
					.da.s.u.i.lb.lb.lb insert end $x \
				} \
			}; \
			.da.s.u.i.e.e selection range 0 end \
		}" );

	cmd( "XYZokhelpcancel $p fb Add Delete \"Delete All\" { \
			set a [ .da.s.u.i.e.e get ]; \
			if { [ lsearch $list_times $a ] < 0 && [ string is integer -strict $a ] && $a >= $minc && $a <= $maxc } { \
				.da.s.u.i.lb.lb.lb insert end $a; \
				.da.s.u.i.lb.lb.lb see end; \
				focus .da.s.u.i.e.e; \
				.da.s.u.i.e.e selection range 0 end; \
				.da.s.u.i.lb.lb.lb selection clear 0 end \
			} { \
				bell \
			} \
		} { \
			set sel [ .da.s.u.i.lb.lb.lb curselection ]; \
			if { [ llength $sel ] > 0 } { \
				.da.s.u.i.lb.lb.lb delete [ lindex $sel 0 ] [ lindex $sel [ expr [ llength $sel ] - 1 ] ]; \
				.da.s.u.i.e.e selection range 0 end; \
				focus .da.s.u.i.e.e \
			} \
		} { \
			.da.s.u.i.lb.lb.lb delete 0 end \
		} {\
			set choice 1 \
		} { \
			LsdHelp menudata_res.html#crosssection \
		} { \
			set choice 2 \
		}" );

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
	cmd( "if { $dir != 0 && [ lsearch $list_times $res ] < 0 } { tk_messageBox -parent .da.s -type ok -title Warning -icon warning -message \"Invalid time step reference selected\" -detail \"The selected time step reference is not one of the selected for the cross-section( s), no sorting will be performed.\"; set dir 0 }" );
	cmd( "set num_t [ llength $list_times ]" );
	*choice = 0;

	end:

	cmd( "destroytop .da.s" );

	Tcl_UnlinkVar( inter, "res" );
	Tcl_UnlinkVar( inter, "dir" );

	return;
}


/***************************************************
SORT_CS_DESC
****************************************************/
void sort_cs_desc( char **s,char **t, double **v, int nv, int nt, int c )
{
	int i, j, h;
	double dapp;
	char sapp[ MAX_ELEM_LENGTH ];

	for ( i = nv - 2; i >= 0; --i )
	{
		for ( j = i; j < nv - 1 && v[ j ][ c ] < v[ j + 1 ][ c ] ; ++j )
		{
			for ( h = 0; h < nt; ++h )
			{
				dapp = v[ j ][ h ];
				v[ j ][ h ] = v[ j + 1 ][ h ];
				v[ j + 1 ][ h ] = dapp;
			}
			
			strcpy( sapp, s[ j ] );
			strcpy( s[ j ], s[ j + 1 ] );
			strcpy( s[ j + 1 ], sapp );
		
			strcpy( sapp,t[ j ] );
			strcpy( t[ j ],t[ j + 1 ] );
			strcpy( t[ j + 1 ], sapp );
		}
	}
}


/***************************************************
SORT_CS_ASC
****************************************************/
void sort_cs_asc( char **s,char **t, double **v, int nv, int nt, int c )
{
	int i, j, h;
	double dapp;
	char sapp[ MAX_ELEM_LENGTH ];

	for ( i = nv-2; i >= 0; --i )
	{
		for ( j = i; j < nv - 1 && v[ j ][ c ] > v[ j + 1 ][ c ]; ++j )
		{
			for ( h = 0; h < nt; ++h )
			{
				dapp = v[ j ][ h ];
				v[ j ][ h ] = v[ j + 1 ][ h ];
				v[ j + 1 ][ h ] = dapp;
			}
			
			strcpy( sapp, s[ j ] );
			strcpy( s[ j ], s[ j + 1 ] );
			strcpy( s[ j + 1 ], sapp );
		
			strcpy( sapp,t[ j ] );
			strcpy( t[ j ],t[ j + 1 ] );
			strcpy( t[ j + 1 ], sapp );
		}
	}
}


/***************************************************
SEARCH_LAB_TIT
****************************************************/
double *search_lab_tit( object *r, char *s, char *t, int st, int en )
{
	object *cur;
	variable *cv;
	double *res = NULL;
	
	sprintf( msg, "%s_%s", s, t );
	for ( cur = r; cur != NULL && res == NULL; cur = cur->next )
	{ 
		for ( cv = cur->v; cv != NULL; cv = cv->next )
		{
			if ( cv->save == 1 && ! strcmp( cv->lab_tit, msg ) && st == cv->start )
			return cv->data;
		}
		
		if ( cur->b != NULL && cur->b->head != NULL )
			res = search_lab_tit( cur->b->head, s, t, st, en );
	}
	
	if ( res == NULL && r->up == NULL )
		for ( cv = cemetery; cv != NULL && res == NULL; cv = cv->next )
			if ( ! strcmp( cv->lab_tit, msg ) && st == cv->start )
				return cv->data;

	return res;
}


/***************************************************
INSERT_DATA_MEM
****************************************************/
void insert_data_mem( object *r, int *num_v, int *num_c )
{
	insert_labels_mem( r, num_v, num_c );
	vs = new store[ *num_v ];
	*num_v = 0;
	insert_store_mem( r, num_v );
}


/***************************************************
INSERT_LABELS_MEM
****************************************************/
void insert_labels_mem( object *r, int *num_v, int *num_c )
{
	object *cur;
	variable *cv;
	bridge *cb;

	for ( cv = r->v; cv != NULL; cv = cv->next )
		if ( cv->save )
			{
				set_lab_tit( cv );
				cmd( ".da.vars.lb.v insert end \"%s %s (%d-%d) #%d\"", cv->label, cv->lab_tit, cv->start, cv->end, *num_v );
				if ( cv->end > *num_c )
					*num_c = cv->end;
				*num_v += 1;
			}

	for ( cb = r->b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL && cb->head->to_compute )
			for ( cur = cb->head; cur != NULL; cur = cur->next )
				insert_labels_mem( cur, num_v, num_c );
	 
	if ( r->up == NULL )
		for ( cv = cemetery; cv != NULL; cv = cv->next )
		{  
			cmd( ".da.vars.lb.v insert end \"%s %s (%d-%d) #%d\"", cv->label, cv->lab_tit, cv->start, cv->end, *num_v );
			if ( cv->end > *num_c )
				*num_c = cv->end;
			*num_v += 1;
		}
}


/***************************************************
INSERT_STORE_MEM
****************************************************/
void insert_store_mem( object *r, int *num_v )
{
	object *cur;
	variable *cv;
	bridge *cb;

	for ( cv=r->v; cv != NULL; cv = cv->next )
		if ( cv->save )
		{
			set_lab_tit( cv );
			strcpy( vs[ *num_v ].label, cv->label );
			strcpy( vs[ *num_v ].tag, cv->lab_tit );
			vs[ *num_v ].start = cv->start;
			vs[ *num_v ].end = cv->end;
			vs[ *num_v ].rank = *num_v;
			vs[ *num_v ].data = cv->data;
			*num_v += 1;
		}
	  
	for ( cb = r->b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL && cb->head->to_compute )
			for ( cur = cb->head; cur != NULL; cur = cur->next )
				insert_store_mem( cur, num_v );
	 
	if ( r->up == NULL )
		for ( cv = cemetery; cv != NULL; cv = cv->next )
		{
			strcpy( vs[ *num_v ].label,cv->label );
			strcpy( vs[ *num_v ].tag,cv->lab_tit);
			vs[ *num_v ].start = cv->start;
			vs[ *num_v ].end = cv->end;
			vs[ *num_v ].rank = *num_v;
			vs[ *num_v ].data = cv->data;
			*num_v += 1;
		}
}


/***************************************************
INSERT_DATA_FILE
****************************************************/
void insert_data_file( bool gz, int *num_v, int *num_c )
{
	FILE *f = NULL;
#ifdef LIBZ
	gzFile fz = NULL;
#endif
	char ch, app_str[ 20 ], *tok, *linbuf;
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

	new_v = 0;
	if ( *num_v == 0 )
		plog( "\nResult data from file %s:\n", "", filename );
	else
		plog( "\nAdditional data file number %d.\nResult data from file %s:\n", "", file_counter, filename );  

	if ( ! gz )
		ch = ( char ) fgetc( f );
	else
	{
#ifdef LIBZ
		ch = ( char ) gzgetc( fz );
#endif
	}

	while ( ch != '\n' )
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

	new_c = -1;
	while ( ch != EOF )
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

	if ( *num_v == 0 )
	{
		vs = new store[ new_v ];
	}
	else
	{
		app = new store[ new_v + *num_v ];
		for ( i = 0; i < *num_v; ++i )
		{
			app[ i ] = vs[ i ];
			strcpy( app[ i ].label, vs[ i ].label );
			strcpy( app[ i ].tag, vs[ i ].tag);
		} 
		
		delete [ ] vs;
		vs = app;
	}  

	if ( ! gz )
		f = fopen( filename, "rt" );
	else
	{
#ifdef LIBZ
		fz = gzopen( filename, "rt" );
#endif
	}

	if ( *num_v>-1 )
		sprintf( app_str, " (file=%d)", file_counter );
	else
		strcpy( app_str, "" );

	linsiz = ( int ) max( linsiz, new_v * ( DBL_DIG + 4 ) ) + 1;
	linbuf = new char[ linsiz ];
	if ( linbuf == NULL )
	{
		plog( "\nError: not enough memory or invalid format, aborting file load.\n" );
		goto end;
	}

	// read header line
	if ( ! gz )
		fgets( linbuf, linsiz, f );		// buffers one entire line
	else
	{
#ifdef LIBZ
		gzgets( fz, linbuf, linsiz );
#endif
	}

	tok = strtok( linbuf , "\t" ); 		// prepares for parsing and get first one
	for ( i = *num_v; i < new_v + *num_v; ++i )
	{
		if ( tok == NULL )
		{
			plog( "\nError: invalid header, aborting file load.\n" );
			goto end;
		}
		
		sscanf( tok, "%s %s (%d %d)", vs[ i ].label, vs[ i ].tag, &( vs[ i ].start ), &( vs[ i ].end ) );	
		strcat( vs[ i ].label, "F" );
		sprintf( msg, "%d_%s", file_counter, vs[ i ].tag);
		strcpy( vs[ i ].tag, msg );
		vs[ i ].rank = i;

		if ( vs[ i ].start != -1 )
			cmd( ".da.vars.lb.v insert end \"%s %s (%d-%d) #%d %s\"", vs[ i ].label, vs[ i ].tag, vs[ i ].start, vs[ i ].end, i, app_str );
		else
		{
			cmd( ".da.vars.lb.v insert end \"%s %s (0-%d) #%d %s\"", vs[ i ].label, vs[ i ].tag, new_c-1, i, app_str );
			vs[ i ].start = 0;
			vs[ i ].end = new_c-1;
		}
	 
		cmd( "lappend DaModElem %s", vs[ i ].label );

		vs[ i ].data = new double[ new_c + 2 ];
	 
		tok = strtok( NULL, "\t" );			// get next token, if any
	}

	// read data lines
	for ( j = 0; j < new_c; ++j )
	{
		if ( ! gz )
			fgets( linbuf, linsiz, f );		// buffers one entire line
		else
		{
#ifdef LIBZ
			gzgets( fz, linbuf, linsiz );
#endif
		}
	 
		tok = strtok( linbuf , "\t" ); 		// prepares for parsing and get first one
		
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
				sscanf( tok, "%lf", &( vs[ i ].data[ j ] ) );
			
			tok = strtok( NULL, "\t" );			// get next token, if any
		}
	}

	*num_v += new_v;
	new_c--;
	if ( new_c > *num_c )
		*num_c = new_c;
	if ( new_c > max_c )
		max_c = new_c; 

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
void statistics( int *choice )
{
	bool stopErr = false;
	char *app, **str, **tag, str1[ 50 ], longmsg[ 300 ];
	double **data, **logdata, av, var, num, ymin, ymax, sig;
	int i, j, *start, *end, *id, logErrCnt = 0;

	if ( nv == 0 )			// no variables selected
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return;
	}

	data = new double *[ nv ];
	logdata = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		logdata[ i ] = NULL;
		
		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data; 
			if ( data[ i ] == NULL )
			plog( "\nError: invalid data\n" );
	   
			if ( logs )			// apply log to the values to show "log scale" in the y-axis
			{
				logdata[ i ] = new double[ end[ i ] + 1 ];	// create space for the logged values
				for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
					if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )	// ignore NaNs
						logdata[ i ][ j ] = log( data[ i ][ j ] );
					else
					{
						logdata[ i ][ j ] = NAN;
						if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
						{
							plog( "\nWarning: zero or negative values in log statistics (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
						}
						else
							if ( ! stopErr )
							{
								plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
								stopErr = true;
							}
					}
					
				data[ i ] = logdata[ i ];				// replace the data series
			}
		}
	}

	if ( logs )
		cmd( ".log.text.text.internal insert end \"\n\nTime series descriptive statistics (in log):\n\n\" tabel" );
	else
		cmd( ".log.text.text.internal insert end \"\n\nTime series descriptive statistics:\n\n\" tabel" );

	sprintf( str1, "%d Cases", max_c - min_c + 1 );
	sprintf( longmsg, "%-20s\tAverage\tStd.Dev.\tVar.\tMin.\tMax.\n", str1 );
	cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );

	for ( i = 0; i < nv; ++i )
	{
		ymin = DBL_MAX;
		ymax = - DBL_MAX;

		for ( av = var = num = 0, j = min_c; j <= max_c; ++j )
		{
			if ( j >= start[ i ] && j <= end[ i ] && is_finite( data[ i ][ j ] ) )	// ignore NaNs
			{
				if ( data[ i ][ j ] < ymin )
					ymin = data[ i ][ j ];
				
				if ( data[ i ][ j ] > ymax )
					ymax = data[ i ][ j ];
				
				av += data[ i ][ j ];
				var += data[ i ][ j ] * data[ i ][ j ];
				num++;
			}
		}

		if ( num > 1 )
		{
			av = av / num;
			var = fabs( var / num - av * av );
			sig = sqrt( var * num / ( num - 1 ) );
		}
		else
			var = sig = 0;
		
		if ( num > 0 )
		{
			sprintf( msg, "%s %s (%.*g)", str[ i ], tag[ i ], pdigits, num );
			sprintf( str1, "%-20s\t", msg );
			cmd( ".log.text.text.internal insert end \"%s\" tabel", str1 );
			
			
			sprintf( longmsg, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", pdigits, av, pdigits, sig, pdigits, var, pdigits, ymin, pdigits, ymax);
			cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );
		}
	}

	plog( "\n" );

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/************************
STATISTICS_CROSS
************************/
void statistics_cross( int *choice )
{
	bool first, stopErr = false;
	char *app, **str, **tag, str1[ 50 ], longmsg[ 300 ];
	double **data, **logdata, av, var, num, ymin = 0, ymax = 0, sig;
	int i, j, h, k, nt, *start, *end, *id, *list_times, logErrCnt = 0;

	Tcl_LinkVar( inter, "nt", ( char * ) &nt, TCL_LINK_INT );
	cmd( "if [ info exists num_t ] { set nt $num_t } { set nt \"-1\" }" );
	Tcl_UnlinkVar( inter, "nt" );

	if ( nv < 2 || nt <= 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place at least two series in the Series Selected listbox and select at least one time step (case ).\"" );
		*choice = 2;
		return;
	}

	// sets the list of cases to plot
	list_times = new int [ nt ];
	cmd( "set k 0" );
	Tcl_LinkVar( inter, "k", ( char * ) &k, TCL_LINK_INT );
	
	for ( i = 0; i < nt; ++i )    
	{
		cmd( "set k [ lindex $list_times %d ]", i );
		list_times[ i ] = k;
	}
	
	Tcl_UnlinkVar( inter, "k" );

	data = new double *[ nv ];
	logdata = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		
		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		data[ i ] = vs[ id[ i ] ].data;
		if ( data[ i ] == NULL )
			plog( "\nError: invalid data\n" );
	   
		if ( logs )			// apply log to the values to show "log scale" in the y-axis
		{
			logdata[ i ] = new double[ end[ i ] + 1 ];	// create space for the logged values
			for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
				if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )		// ignore NaNs
					logdata[ i ][ j ] = log( data[ i ][ j ] );
				else
				{
					logdata[ i ][ j ] = NAN;
					if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
						plog( "\nWarning: zero or negative values in log statistics (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
					else
						if ( ! stopErr )
						{
							plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
							stopErr = true;
						}
				}
				
			data[ i ] = logdata[ i ];				// replace the data series
		}
	}

	if ( logs )
		cmd( ".log.text.text.internal insert end \"\n\nCross-section descriptive statistics (in log):\n\n\" tabel" );
	else
		cmd( ".log.text.text.internal insert end \"\n\nCross-section descriptive statistics:\n\n\" tabel" );

	sprintf( str1, "%d Variables", nv );
	sprintf( longmsg, "%-20s\tAverage\tStd.Dev.\tVar.\tMin.\tMax.\n", str1 );
	cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );

	for ( j = 0; j < nt; ++j )
	{
		h = list_times[ j ];
		first = true;
		for ( av = var = num = 0, i = 0; i < nv; ++i )
		{
			if ( h >= start[ i ] && h <= end[ i ] && is_finite( data[ i ][ h ] ) )		// ignore NaNs
			{
				if ( first )
				{
					ymin = ymax = data[ i ][ h ];
					first = false;
				}
				
				if ( data[ i ][ h ] < ymin )
					ymin = data[ i ][ h ];
				
				if ( data[ i ][ h ] > ymax )
					ymax = data[ i ][ h ];
				
				av += data[ i ][ h ];
				num++;
				var += data[ i ][ h ]*data[ i ][ h ];
			}
		}
		
		if ( num > 1 )
		{
			av = av / num;
			var = var / num - av * av;
			sig = sqrt( var * num / ( num-1 ) );
		}
		else
			var = sig = 0;

		if ( num > 0 )
		{
			sprintf( str1, "Case %d (%.*g)\t", h, pdigits, num );
			cmd( ".log.text.text.internal insert end \"%s\" tabel", str1 );
			sprintf( longmsg, "%.*g\t%.*g\t%.*g\t%.*g\t%.*g\n", pdigits, av, pdigits, sig, pdigits, var, pdigits, ymin, pdigits, ymax );
			cmd( ".log.text.text.internal insert end \"%s\" tabel", longmsg );
		}
	}

	plog( "\n" );

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}

	delete [ ] list_times;
	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/************************
SORT_LABELS_DOWN
************************/
/*
Sorting function for presenting variables' labels in a nice way.
The variables are grouped according to:
	1) their label ( increasing: A first z last)
	2) time of their last occurrence ( decreasing: existing variable first)
   	3) time of their first occurrence ( increasing: first born first)
    4) LSD internal ID indexing system (used for the tag) ( increasing)
The function is complicated for the point 4) by the fact that the tag is recorded
in the labels as a single string using the underscore '_' as joining character.
*/
int sort_labels_down( const void *a, const void *b )
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

	delete [ ] a_str;
	delete [ ] b_str;

	if ( diff != 0 )
		return diff;
	else
		if ( ( ( store * ) a )->end != ( ( store * ) b)->end )
			return ( ( store * ) b )->end - ( ( store * ) a )->end;
		else
			if ( ( ( store * ) a)->start != ( ( store * ) b )->start )
				return ( ( store * ) a )->start - ( ( store * ) b )->start;
			else
				for ( counter = 0; ; )
				{
					a_int = atoi( ( ( store * ) a )->tag + counter );
					b_int = atoi( ( ( store * ) b )->tag + counter );
					
					if ( a_int != b_int )
						return a_int - b_int;
					
					while ( ( ( store * ) a )->tag[ counter ] != '_' )
						++counter;
					
					++counter;
				}
}


/************************
SORT_ON_END
************************/
void sort_on_end( store *app )
{
	qsort( ( void * ) app, num_var, sizeof( vs[ 0 ] ), sort_labels_down );
}


/***************************************************
PLOT_GNU
Draws the XY plots, with the first series as X and the others as Y's
****************************************************/
void plot_gnu( int *choice )
{
	bool done, stopErr = false;
	char *app, **str, **tag, str1[ 50 ], str2[ 100 ], str3[ 10 ], dirname[ MAX_PATH_LENGTH ];
	double **data, **logdata;
	int i, j, box, ndim, gridd, *start, *end, *id, nanv = 0, logErrCnt = 0;
	FILE *f, *f2;

	if ( nv == 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}

	if ( nv > 2 )
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
		cmd( "label .da.s.d.l -text \"3D XY plane options\"" );

		cmd( "frame .da.s.d.o -relief groove -bd 2" );
		cmd( "if { ! [ info exists box ] } { set box 0 }" );
		cmd( "radiobutton .da.s.d.o.a -text \"Use 1st and 2nd series\" -variable box -value 0" );
		cmd( "radiobutton .da.s.d.o.c -text \"Use time and 1st series\" -variable box -value 2" );
		cmd( "radiobutton .da.s.d.o.b -text \"Use time and rank\" -variable box -value 1" );
		cmd( "pack .da.s.d.o.a .da.s.d.o.c .da.s.d.o.b -anchor w" );

		cmd( "pack .da.s.d.l .da.s.d.o" );

		cmd( "frame .da.s.o" );
		cmd( "checkbutton .da.s.o.g -text \"Use gridded data\" -variable gridd" );
		cmd( "checkbutton .da.s.o.p -text \"Render 3D surface\" -variable pm3d" );
		cmd( "pack .da.s.o.g .da.s.o.p" );

		cmd( "pack .da.s.t .da.s.d .da.s.o -padx 5 -pady 5" );

		cmd( "if { $ndim == 2 } { .da.s.d.o.a configure -state disabled; .da.s.d.o.c configure -state disabled; .da.s.d.o.b configure -state disabled; .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set box 0; set gridd 0; set pm3d 0 } { .da.s.d.o.a configure -state normal; .da.s.d.o.c configure -state normal; .da.s.d.o.b configure -state normal; .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );

		cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#3dTime } { set choice 2 }" );

		cmd( "showtop .da.s" );

		*choice = 0;
		while ( *choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "destroytop .da.s" );

		if ( *choice == 2 )
			return;

		cmd( "set choice $ndim" );
		ndim = *choice;
	}
	else
		ndim = 2; 

	data = new double *[ nv ];
	logdata = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		logdata[ i ] = NULL;
		
		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
				plog( "\nError: invalid data\n" );

			if ( logs )		// apply log to the values to show "log scale" in the y-axis
			{
				logdata[ i ] = new double[ end[ i ] + 1 ];	// create space for the logged values
				for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
					if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )	// ignore NaNs
						logdata[ i ][ j ] = log( data[ i ][ j ] );
					else
					{
						logdata[ i ][ j ] = NAN;
						if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
						{
							plog( "\nWarning: zero or negative values in log Gnuplot (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
						}
						else
							if ( ! stopErr )
							{
								plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
								stopErr = true;
							}
					}
					
				data[ i ] = logdata[ i ];				// replace the data series
			}
		}
		else
			nanv++; 
	}

	// handle case selection
	if ( autom_x || min_c >= max_c )
	{
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = max( start[ i ], 1 );
			
			if ( start[ i ] < min_c )
				min_c = max( start[ i ], 1 );
			
			if ( end[ i ] > max_c )
				max_c = end[ i ] > num_c ? num_c : end[ i ];
		}
	}

	// auto-find minimums and maximums
	if ( miny >= maxy )
		autom = true;

	if ( autom )
		for ( done = false, i = 1; i < nv; ++i )
			for ( j = min_c; j <= max_c; ++j )
			{
				if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) )	// ignore NaNs
				{
					miny = maxy = data[ i ][ j ];
					done = true;
				}
				
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] < miny )	// ignore NaNs
					miny = data[ i ][ j ];
					
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] > maxy )	// ignore NaNs
					maxy = data[ i ][ j ];
			}
	   
	cmd( "set dirxy plotxy_%d", cur_plot );
	cmd( "file mkdir $dirxy" );
	getcwd( dirname, MAX_PATH_LENGTH - 1 );
	sprintf( msg, "plotxy_%d", cur_plot );
	chdir( msg );
	f = fopen( "data.gp", "w" );
	fprintf( f, "#" );

	if ( nv > 2 )
	{
		cmd( "set choice $box" );
		box = *choice;
		cmd( "set choice $gridd" );
		gridd = *choice;
	}
	else
		box = gridd = 0;

	if ( box == 0 )
	{
		for ( i = 0; i < nv; ++i )
			if ( start[ i ] <= max_c && end[ i ] >= min_c )
				fprintf( f, "%s_%s\t", str[ i ], tag[ i ] ); 
	}    
	else
	{
		if ( gridd == 0 )
		{
			fprintf( f, "Time\t" );
			if ( box == 1 )
			{
				for ( i = 0; i < nv; ++i )
					if ( start[ i ] <= max_c && end[ i ] >= min_c )
						fprintf( f, "Var%d\t%s_%s\t", i, str[ i ], tag[ i ] );
			}
			else
			{
				for ( i = 0; i < nv; ++i )
					if ( start[ i ] <= max_c && end[ i ] >= min_c )
						fprintf( f, "%s_%s\t", str[ i ], tag[ i ] );   
			}
		}    
		else
			fprintf( f, "Time\tRank\tVal" );
	} 

	fprintf( f, "\n" );

	if ( box == 0 )
	{
		for ( j = min_c; j <= max_c; ++j )
		{
			for ( i = 0; i < nv; ++i )
				if ( start[ i ] <= max_c && end[ i ] >= min_c )
				{
					if ( j >= start[ i ] && i <= end[ i ] )
						fprintf( f, "%.*g\t", pdigits, data[ i ][ j ] );
					else
						fprintf( f, "nan\t" );  
				}
			fprintf( f, "\n" );
		}
	}
	else
	{
		if ( gridd == 0 )
		{	//not gridded
			if ( box == 1 )			// 3D with time and rank
			{
				for ( j = min_c; j <= max_c; ++j )
				{
					fprintf( f, "%d\t", j );
					for ( i = 0; i < nv; ++i )
						if ( start[ i ] <= max_c && end[ i ] >= min_c )
						{
							if ( j >= start[ i ] && i <= end[ i ] )
								fprintf( f, "%d\t%.*g\t", i + 1, pdigits, data[ i ][ j ] );
							else
								fprintf( f, "%d\tnan\t", i + 1 );        
						}
						
					fprintf( f, "\n" );
				}	
			}
			else					// 3D with time and 1st series
			{
				for ( j = min_c; j <= max_c; ++j )
				{
					fprintf( f, "%d\t", j );
					for ( i = 0; i < nv; ++i )
						if ( start[ i ] <= max_c && end[ i ] >= min_c )
						{
							if ( j >= start[ i ] && i <= end[ i ] ) 
								fprintf( f, "%.*g\t", pdigits, data[ i ][ j ] );
							else
								fprintf( f, "nan\t" );  
						} 
						
					fprintf( f, "\n" );
				}
			} 
		}
		else
		{	// gridded
			if ( box == 1 )			// 3D with time and rank
			{
				for ( i = 0; i < nv; ++i )
				{
					for ( j = min_c; j <= max_c; ++j )
						if ( start[ i ] <= max_c && end[ i ] >= min_c )
						{
							if ( j >= start[ i ] && i <= end[ i ] )
								fprintf( f, "%d\t%d\t%.*g\n", j, i + 1, pdigits, data[ i ][ j ] );
							else
								fprintf( f, "%d\t%d\tnan\n", j , i + 1 );         
						}
				}  
			}
			else					// 3D with time and 1st series
			{
				for ( i = 0; i < nv; ++i )
				{
					for ( j = min_c; j <= max_c; ++j )
						if ( start[ i ] <= max_c && end[ i ] >= min_c )
						{
							if ( j >= start[ i ] && i <= end[ i ] )
								fprintf( f, "%d\t%.*g\n", j, pdigits, data[ i ][ j ] );
							else
								fprintf( f, "%d\tnan\n", j );         
						}
				}  
			}
		} 
	} 

	fclose( f );

	*choice = 0;

	f = fopen( "gnuplot.lsd", "w" );
	f2 = fopen( "gnuplot.gp", "w" );

	fprintf( f, "set datafile missing \"nan\" \n" );		//handle NaNs
	fprintf( f2, "set datafile missing \"nan\" \n" );

	app = ( char * ) Tcl_GetVar( inter, "gpterm", 0 );
	fprintf( f, "set term tkcanvas\n" );
	if ( strlen( app ) > 0 )
		fprintf( f2, "set term %s\n", app );

	fprintf( f, "set output 'plot.file'\n" );

	if ( grid )
	{
		fprintf( f, "set grid\n" );
		fprintf( f2, "set grid\n" );
	}

	if ( line_point == 2 )
	{
		sprintf( str1, "set pointsize %lf\n", point_size );
		fprintf( f, "%s", str1 );
		fprintf( f2, "%s", str1 );
	}

	if ( ndim == 2 )
	{
		sprintf( msg, "set yrange [%.*g:%.*g]\n", pdigits, miny, pdigits, maxy );
		fprintf( f, "%s", msg );
		fprintf( f2, "%s", msg );
	} 

	if ( box == 0 )
		sprintf( msg, "set xlabel \"%s_%s\"\n", str[ 0 ], tag[ 0 ] );
	else
		sprintf( msg, "set xlabel \"Time\"\n" );  

	fprintf( f, "%s", msg );
	fprintf( f2, "%s", msg );

	if ( ndim > 2 )
	{
		if ( box == 0 )
			sprintf( msg, "set ylabel \"%s_%s\"\n", str[ 1 ], tag[ 1 ] );
		else
			sprintf( msg, "set ylabel \"Series\"\n" ); 
		
		fprintf( f, "%s", msg );
		fprintf( f2, "%s", msg );
	} 

	if ( line_point == 1 && ndim == 2 )
		sprintf( str1, "smooth csplines" );
	else
		if ( line_point == 1 && ndim > 2 )
			sprintf( str1, "with lines" );
		else
			strcpy( str1, "" ); 

	if ( ndim == 2 )
	{
		if ( allblack )
			sprintf( str3, " lt -1" );
		else
			strcpy( str3, "" );
	}
	else
	{
		cmd( "set choice $gridd" );
		if ( *choice == 1 )
		{
			app = ( char * ) Tcl_GetVar( inter, "gpdgrid3d", 0 );
			fprintf( f, "set dgrid3d %s\nset hidden3d\n", app );
			fprintf( f2, "set dgrid3d %s\nset hidden3d\n", app );
		}
		
		cmd( "set choice $pm3d" );
		if ( *choice == 1 )
		{
		   fprintf( f, "set pm3d\n" );
		   fprintf( f2, "set pm3d\n" );
		   sprintf( str1, "with pm3d " );
		}

		if ( allblack )
		{
			fprintf( f, "set palette gray\n" );
			fprintf( f2, "set palette gray\n" );
		}
	}

	app = ( char * ) Tcl_GetVar( inter, "gpoptions", 0 );
	fprintf( f, "%s\n", app );
	fprintf( f2, "%s\n", app );
	 
	if ( ndim == 2 ) 
	{
		sprintf( msg, "plot 'data.gp' using 1:2 %s t \"%s_%s\"", str1, str[ 1 ], tag[ 1 ] );
		
		if ( allblack )
			strcat( msg, str3);
		
		i = 2;
	} 
	else
	{
		if ( box == 0 )
		{
			sprintf( msg, "splot 'data.gp' using 1:2:3 %s t \"%s_%s\"", str1, str[ 2 ], tag[ 2 ] ); 
			i = 3;
		} 
		else
		{
			i = 1;  
			if ( box == 1 )
				sprintf( msg, "splot 'data.gp' using 1:2:3 %s t \"\"", str1 ); 
			else 
			{
				sprintf( msg, "splot 'data.gp' using 1:2:%d %s t \"\"", ( nv - nanv ) / 2 + 2, str1 );   
				i = 2;
			} 
		} 
	} 

	fprintf( f, "%s", msg );
	fprintf( f2, "%s", msg );

	for ( ; i < nv; ++i )
		if ( start[ i ] <= max_c && end[ i ] >= min_c )
		{
			if ( ndim == 2 )
				sprintf( str2, ", 'data.gp' using 1:%d %s t \"%s_%s\"", i + 1, str1, str[ i ], tag[ i ] );
			else
			{
				if ( box == 0 )
					sprintf( str2, ", 'data.gp' using 1:2:%d %s t \"%s_%s\"", i + 1, str1, str[ i ], tag[ i ] ); 
				else
					if ( gridd == 0 )
					{
						if ( box == 1 )
							sprintf( str2, ", 'data.gp' using 1:%d:%d %s t \"\"",2+2*i, 2*i+3, str1 ); 
						else
							if ( i <= nv / 2 )
								sprintf( str2, ", 'data.gp' using 1:%d:%d %s t \"\"", i + 1, ( nv-nanv) / 2+i + 1, str1 ); 
							else
								strcpy( str2, "" );     
					} 
					else
						strcpy( str2, "" );  
			}  

			if ( strlen( str2 ) > 0 && allblack )
				strcat( str2, str3 );
			
			fprintf( f, "%s", str2 );
			fprintf( f2, "%s", str2 );
		}

	fprintf( f, "\n" );
	fprintf( f2, "\n" );

	fclose( f );
	fclose( f2 );

	cmd( "if { [ info exists pm3d ] } { set choice $pm3d } { set choice 0 }" );
	if ( *choice != 0 )
		*choice = 2;	// require filled polygons, not supported by tkcanvas terminal
	else
		*choice = gnu;

	show_plot_gnu( cur_plot, choice, *choice, str, tag );

	chdir( dirname );
	
	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/*****************************************
PLOT_CS_XY
*****************************************/
void plot_cs_xy( int *choice )
{
	bool done, stopErr = false;
	char *app, **str, **tag, str1[ TCL_BUFF_STR ], str2[ 5 * MAX_ELEM_LENGTH ], str3[ MAX_ELEM_LENGTH ], dirname[ MAX_PATH_LENGTH ];
	double **data, **logdata, previous_row;
	int i, j, time_sel, block_length, ndim, *start, *end, *id, logErrCnt = 0;
	FILE *f, *f2;

	if ( nv < 2 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place at least two series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}

	data = new double *[ nv ];
	logdata = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		logdata[ i ] = NULL;
		
		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
				plog( "\nError: invalid data\n" );
	   
			if ( logs )		// apply log to the values to show "log scale" in the y-axis
			{
				logdata[ i ] = new double[ end[ i ] + 1 ];	// create space for the logged values
				for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
					if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )		// ignore NaNs
						logdata[ i ][ j ] = log( data[ i ][ j ] );
					else
					{
						logdata[ i ][ j ] = NAN;
						if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
							plog( "\nWarning: zero or negative values in log plot (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
						else
							if ( ! stopErr )
							{
								plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
								stopErr = true;
							}
					}
					
				data[ i ] = logdata[ i ];			// replace the data series
			}
		}
	}

	// handle case selection
	if ( autom_x || min_c >= max_c )
	{
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = max( start[ i ], 1 );
			
			if ( start[ i ] < min_c )
				min_c = max( start[ i ], 1 );
			
			if ( end[ i ] > max_c )
				max_c = end[ i ] > num_c ? num_c : end[ i ];
		}
	}

	// auto-find minimums and maximums
	if ( miny >= maxy )
		autom = true;
	
	if ( autom )
		for ( done = false, i = 1; i < nv; ++i )
			for ( j = min_c; j <= max_c; ++j )
			{
				if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) )	// ignore NaNs
				{
					miny = maxy = data[ i ][ j ];
					done = true;
				}
				
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] < miny )	// ignore NaNs
					miny = data[ i ][ j ];
					
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] > maxy )	// ignore NaNs
					maxy = data[ i ][ j ];
			}

	cmd( "set bidi %d", end[ 0 ] );

	cmd( "newtop .da.s \"XY Plot Options\" { set choice 2 } .da" );

	cmd( "frame .da.s.i" );
	cmd( "label .da.s.i.l -text \"Time step\"" );
	cmd( "entry .da.s.i.e -width 10 -validate focusout -vcmd { if { [ string is integer -strict %%P ] && %%P > 0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
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
	cmd( "checkbutton .da.s.o.p -text \"Render 3D surface\" -variable pm3d -anchor w" );
	cmd( "if { $ndim == 2 } { .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set gridd 0; set pm3d 0 } { .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );
	cmd( "pack .da.s.o.l .da.s.o.g .da.s.o.p" );

	cmd( "set choice $ndim" );
	if ( *choice == 2 )
		cmd( "set blength %d", ( int )( nv / 2 ) );
	else
		cmd( "set blength %d", ( int )( nv / 3 ) ); 

	cmd( "set numv 1" );
	cmd( "frame .da.s.v" );
	cmd( "label .da.s.v.l -text \"Number of dependent variables\"" );
	cmd( "entry .da.s.v.e -width 10 -validate focusout -vcmd { if { [ string is integer -strict %%P ] && %%P > 0 && %%P < %d } { set numv %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numv; return 0 } } -invcmd { bell } -justify center", nv );
	cmd( ".da.s.v.e insert 0 $numv" ); 
	cmd( "label .da.s.v.n -text \"Block length: $blength\"" );
	cmd( "pack .da.s.v.l .da.s.v.e .da.s.v.n" );

	cmd( "pack .da.s.i .da.s.d .da.s.o .da.s.v -padx 5 -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#3dCrossSection } { set choice 2 }" );

	cmd( "bind .da.s.v.e <KeyRelease> {set blength [ expr $nnvar / ($numv + $ndim-1 )]; .da.s.v.n conf -text \"Block length: $blength\"}" );
	cmd( "set nnvar %d", nv );
	cmd( "bind .da.s.v.e <Return> {set blength [ expr $nnvar / ($numv + $ndim-1 )]; .da.s.v.n conf -text \"Block length: $blength\"}" );
	cmd( "bind .da.s.v.e <Tab> {set blength [ expr $nnvar / ($numv + $ndim-1 )]; .da.s.v.n conf -text \"Block length: $blength\"}" );
	cmd( "bind .da.s.d.r.2d <Return> {set blength [ expr $nnvar / ($numv + $ndim-1 )]; .da.s.v.n conf -text \"Block length: $blength\"}" );
	cmd( "bind .da.s.d.r.2d <ButtonRelease-1> {set ndim 2; set blength [ expr $nnvar / ($numv + $ndim-1 )]; .da.s.v.n conf -text \"Block length: $blength\"}" );
	cmd( "bind .da.s.d.r.3d <ButtonRelease-1> {set ndim 3; set blength [ expr $nnvar / ($numv + $ndim-1 )]; .da.s.v.n conf -text \"Block length: $blength\"}" );
	cmd( "bind .da.s.d.r.2d <Down> {.da.s.d.r.3d invoke; focus .da.s.d.r.3d; set ndim 3; set blength [ expr $nnvar / ($numv + $ndim-1 )]; .da.s.v.n conf -text \"Block length: $blength\"}" );
	cmd( "bind .da.s.d.r.3d <Up> {.da.s.d.r.2d invoke; focus .da.s.d.r.2d; set ndim 2; set blength [ expr $nnvar / ($numv + $ndim-1 )]; .da.s.v.n conf -text \"Block length: $blength\"}" );
	cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
	cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );
	cmd( "bind .da.s.i.e <KeyPress-Return> {if {$ndim == 2} { focus .da.s.d.r.2d } {focus .da.s.d.r.3d}}" );
	cmd( "bind .da.s.d.r.2d <KeyPress-Return> {.da.s.v.e selection range 0 end; focus .da.s.v.e}" );
	cmd( "bind .da.s.d.r.3d <KeyPress-Return> {.da.s.v.e selection range 0 end; focus .da.s.v.e}" );
	cmd( "bind .da.s.v.e <KeyPress-Return> {focus .da.s.b.ok}" );

	cmd( "showtop .da.s" );

	cmd( "focus .da.s.i.e" );
	cmd( ".da.s.i.e selection range 0 end" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set bidi [ .da.s.i.e get ]" ); 
	cmd( "set numv [ .da.s.v.e get ]" ); 

	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
		goto end;

	cmd( "set choice $ndim" );
	ndim = *choice;

	cmd( "set choice $bidi" );
	time_sel = *choice;

	cmd( "set blength [ expr $nnvar / ($numv + $ndim-1 )]" );
	cmd( "set choice $blength" );

	block_length = *choice;
	*choice = 0;

	if ( block_length <= 0 || nv % block_length != 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid block length\" -detail \"Block length should be an exact divisor of the number of variables. You may also try a 3D plot.\"" );
		*choice = 2;
		goto end;
	}

	cmd( "set dirxy plotxy_%d", cur_plot );

	cmd( "file mkdir $dirxy" );
	getcwd( dirname, MAX_PATH_LENGTH - 1 );
	sprintf( msg, "plotxy_%d", cur_plot );
	chdir( msg );

	f = fopen( "data.gp", "w" );

	if ( start[ 0 ] == end[ 0 ] )  
		previous_row = data[ 0 ][ end[ 0 ] ];
	else
		previous_row = data[ 0 ][ time_sel ];  
	  
	for ( j = 0; j < block_length; ++j )
	{

		if ( start[ 0 ] == end[ 0 ] )  
		{
			if ( data[ j ][ end[ j ] ] != previous_row )
				previous_row = data[ j ][ end[ j ] ];
		}
		else
		{
			if ( data[ j ][ time_sel ] != previous_row )
				previous_row = data[ j ][ time_sel ];
		}
		 
		for ( i = 0; i < nv; i += block_length )
			if ( start[ i + j ] == end[ i + j ] )  
				fprintf( f, "%.*g\t", pdigits, data[ i + j ][ end[ i + j ] ] );
			else
				if ( start[ i + j ] <= max_c && end[ i + j ] >= min_c && start[ i + j ] <= time_sel && end[ i + j ] >= time_sel )
					fprintf( f, "%.*g\t", pdigits, data[ i + j ][ time_sel ] );

		fprintf( f, "\n" );
	}

	fclose( f );

	f = fopen( "gnuplot.lsd", "w" );
	f2 = fopen( "gnuplot.gp", "w" );

	fprintf( f, "set datafile missing \"nan\" \n" );		//handle NaNs
	fprintf( f2, "set datafile missing \"nan\" \n" );

	app = ( char * ) Tcl_GetVar( inter, "gpterm", 0 );
	fprintf( f, "set term tkcanvas\n" );
	if ( strlen( app ) > 0 )
		fprintf( f2, "set term %s\n", app );

	fprintf( f, "set output 'plot.file'\n" );

	if ( grid )
	{
		fprintf( f, "set grid\n" );
		fprintf( f2, "set grid\n" );
	}

	if ( line_point == 2 )
	{ 
		fprintf( f, "set pointsize %lf\n", point_size );
		fprintf( f2, "set pointsize %lf\n", point_size );
		
		if ( ndim == 2 )
			strcpy( str2, "" );
		else
			sprintf( str2, "with points " );       
	}
	else
	{
		if ( ndim == 2 )
			sprintf( str2, "smooth csplines " );
		else
			sprintf( str2, "with lines " ); 
	}

	sprintf( msg, "set xlabel \"%s_%s\"\n", str[ 0 ], tag[ 0 ] );
	fprintf( f, "%s", msg );
	fprintf( f2, "%s", msg );

	if ( ndim == 3 )
	{
		sprintf( msg, "set ylabel \"%s_%s\"\n", str[ block_length ], tag[ block_length ] );
		fprintf( f, "%s", msg );
		fprintf( f2, "%s", msg );
	} 

	if ( allblack )
		sprintf( str3, " lt -1" );
	else
		str3[ 0 ] = 0;

	if ( ndim == 3 )
	{
		cmd( "set choice $gridd" );
		if ( *choice == 1 )
		{
			app = ( char * ) Tcl_GetVar( inter, "gpdgrid3d", 0 );
			fprintf( f, "set dgrid3d %s\nset hidden3d\n", app );
			fprintf( f2, "set dgrid3d %s\nset hidden3d\n", app );
		}
		
		cmd( "set choice $pm3d" );
		if ( *choice == 1 )
		{
			sprintf( str2, "with pm3d " );
			fprintf( f, "set pm3d\n" );
			fprintf( f2, "set pm3d\n" );
		}

		if ( allblack )
		{
			strcpy( str3, "" );
			fprintf( f, "set palette gray\n" );
			fprintf( f2, "set palette gray\n" );
		} 
	}
	 
	if ( ndim == 2 )
	{
		sprintf( msg, "plot 'data.gp' using 1:2 %s t \"%s_%s(%d)\"", str2, str[ block_length ], tag[ block_length ], time_sel );
		
		if ( allblack )
			strcat( msg, str3);
		
		i = 2;			// init from the second variable
	} 
	else
	{
		sprintf( msg, "splot 'data.gp' using 1:2:3 %s t \"%s_%s(%d)\"", str2, str[2*block_length ], tag[2*block_length ], time_sel ); 
		
		i = 3;
	}  

	for ( ; i < nv / block_length; ++i )
	{
		j = i * block_length;
		
		if ( start[ j ] <= max_c && end[ j ] >= min_c )
		{
			if ( ndim == 2 )
			{
				sprintf( str1, ", 'data.gp' using 1:%d %s t \"%s_%s(%d)\"", i + 1, str2, str[ j ], tag[ j ], time_sel );
				
				if ( allblack )
					strcat( str1, str3);
			}  
			else
				sprintf( str1, ", 'data.gp' using 1:2:%d %s t \"%s_%s(%d)\"", i + 1, str2, str[ j ], tag[ j ], time_sel ); 
			
			strcat( msg, str1 );
		}
	}

	strcat( msg, "\n" );
	fprintf( f, "%s", msg );
	fprintf( f2, "%s", msg );

	fclose( f );
	fclose( f2 );

	cmd( "if { [ info exists pm3d ] } { set choice $pm3d } { set choice 0 }" );
	if ( *choice != 0 )
		*choice = 2;	// require filled polygons, not supported by tkcanvas terminal
	else
		*choice = gnu;

	show_plot_gnu( cur_plot, choice, *choice, str, tag );

	chdir( dirname );

	end:
	
	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/***************************************************
PLOT_PHASE_DIAGRAM
****************************************************/
void plot_phase_diagram( int *choice )
{
	bool done, stopErr = false;
	char *app, **str, **tag, str1[ 50 ], str2[ 100 ], str3[ 100 ], dirname[ MAX_PATH_LENGTH ];
	double **data, **logdata;
	int i, j, nlags, *start, *end, *id, logErrCnt = 0;
	FILE *f, *f2;

	if ( nv != 1 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"One and only one series must be selected.\"" );
		*choice = 2;
		return;
	}

	data = new double *[ nv ];
	logdata = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		logdata[ i ] = NULL;
		
		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
				plog( "\nError: invalid data\n" );
	   
			if ( logs )			// apply log to the values to show "log scale" in the y-axis
			{
				logdata[ i ] = new double[ end[ i ] + 1 ];	// create space for the logged values
				for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
					if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )		// ignore NaNs
						logdata[ i ][ j ] = log( data[ i ][ j ] );
					else
					{
						logdata[ i ][ j ] = NAN;
						if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
							plog( "\nWarning: zero or negative values in log plot (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
						else
							if ( ! stopErr )
							{
								plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
								stopErr = true;
							}
					}
					
				data[ i ] = logdata[ i ];				// replace the data series
			}
		}
	}

	// handle case selection
	if ( autom_x || min_c >= max_c )
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = max( start[ i ], 1 );
			
			if ( start[ i ] < min_c )
				min_c = max( start[ i ], 1 );
			
			if (end[ i ] > max_c )
				max_c = end[ i ] > num_c?num_c:end[ i ];
		}

	// auto-find minimums and maximums
	if ( miny >= maxy )
		autom = true;

	if ( autom )
		for ( done = false, i = 0; i < nv; ++i )
			for ( j = min_c; j <= max_c; ++j )
			{
				if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) )		// ignore NaNs
				{
					miny = maxy = data[ i ][ j ];
					done = true;
				}
				
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] < miny )		// ignore NaNs
					miny = data[ i ][ j ];
					
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j ] ) && data[ i ][ j ] > maxy )		// ignore NaNs
					maxy = data[ i ][ j ];
			}
		
	cmd( "newtop .da.s \"Lags Selection\" { set choice 2 } .da" );

	cmd( "frame .da.s.i" );
	cmd( "label .da.s.i.l -text \"Number of lags\"" );
	cmd( "set bidi 1" );
	cmd( "entry .da.s.i.e -width 10 -validate focusout -vcmd { if { [ string is integer -strict %%P ] && %%P > 0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e" );

	cmd( "if { ! [ info exists dia ] } { set dia 0 }" );
	cmd( "checkbutton .da.s.arrow -text \"Plot 45\u00B0 diagonal\" -variable dia" );
	cmd( "pack .da.s.i .da.s.arrow -padx 5 -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#phaseplot } { set choice 2 }" );

	cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
	cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set bidi [ .da.s.i.e get ]" ); 
	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
		goto end;

	cmd( "set dirxy plotxy_%d", cur_plot );

	cmd( "file mkdir $dirxy" );
	getcwd( dirname, MAX_PATH_LENGTH - 1 );
	sprintf( msg, "plotxy_%d", cur_plot );
	chdir( msg );

	cmd( "set choice $bidi" );
	nlags = *choice;

	f = fopen( "data.gp", "w" );

	fprintf( f, "#" );

	for ( i = 0; i <= nlags; ++i )
		if ( start[ 0 ] <= max_c && end[ 0 ] >= min_c )
			fprintf( f, "%s_%s(%d)\t", str[ 0 ], tag[ 0 ], i );
	 
	fprintf( f, "\n" );

	for ( j = min_c; j <= max_c - nlags; ++j )
	{
		for ( i = 0; i <= nlags; ++i )
			if ( start[ 0 ] <= max_c && end[ 0 ] >= min_c )
				fprintf( f, "%lf\t", data[ 0 ][ j + i ] );

		fprintf( f, "\n" );
	}

	fclose( f );

	*choice = 0;
	
	f = fopen( "gnuplot.lsd", "w" );
	f2 = fopen( "gnuplot.gp", "w" );
	fprintf( f, "set datafile missing \"nan\" \n" );		// handle NaNs
	fprintf( f2, "set datafile missing \"nan\" \n" );
	fprintf( f, "set term tkcanvas\n" );
	fprintf( f, "set output 'plot.file'\n" );

	if ( grid )
	{
		fprintf( f, "set grid\n" );
		fprintf( f2, "set grid\n" );
	}

	if ( line_point == 2 )
	{
		sprintf( str1, "set pointsize %lf\n", point_size );
		fprintf( f, "%s", str1 );
		fprintf( f2, "%s", str1 );
	}

	sprintf( msg, "set yrange [%.*g:%.*g]\n", pdigits, miny, pdigits, maxy );
	fprintf( f, "%s", msg );
	fprintf( f2, "%s", msg );
	sprintf( msg, "set xlabel \"%s_%s\"\n", str[ 0 ], tag[ 0 ] );
	fprintf( f, "%s", msg );
	fprintf( f2, "%s", msg );

	cmd( "set choice $dia" );
	if ( *choice == 1 )
	{
		fprintf( f, "set arrow from %.*g,%.*g to %.*g,%.*g lt -1\n", pdigits, miny, pdigits, miny, pdigits, maxy, pdigits, maxy );
		fprintf( f2, "set arrow from %.*g,%.*g to %.*g,%.*g lt -1\n", pdigits, miny, pdigits, miny, pdigits, maxy, pdigits, maxy );
	}

	if ( line_point == 1 )
		sprintf( str1, "smooth csplines" );
	else
		strcpy( str1, "" );

	if ( allblack )
		sprintf( str3, " lt -1" );
	else
		str3[ 0 ] = 0;

	sprintf( msg, "plot 'data.gp' using 1:2 %s t \"t + 1\"", str1 );

	if ( allblack )
		strcat( msg, str3 );

	for ( i = 2; i <= nlags; ++i )
		if ( start[ 0 ] <= max_c && end[ 0 ] >= min_c )
		{
			sprintf( str2, ", 'data.gp' using 1:%d %s t \"t+%d\"", i + 1, str1, i );
			strcat( msg, str2 );
			
			if ( allblack )
				strcat( msg, str3 );
		}

	strcat( msg, "\n" );
	fprintf( f, "%s", msg );
	fprintf( f2, "%s", msg );

	fclose( f );
	fclose( f2 );

	cmd( "set choice $gnu" );

	show_plot_gnu( cur_plot, choice, *choice, str, tag );

	chdir( dirname );

	end:

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}
	
	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
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
		if ( ! gnu )
			
	cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Unsupported options for the internal graphical window\" -detail \"Gnuplot automatically selected to support the chosen 3D plot options.\"" );
		gnu = true;
		type = 1;
	}

	cmd( "raise .da ." );
		
	if ( type == 1 )
	{	// plot with external gnuplot
		cmd( "set choice [ open_gnuplot gnuplot.gp \"Please check if you have selected an adequate configuration for the plot.\nIf the error persists please check if Gnuplot is installed and set up properly.\" ]" );
		
		if ( *choice != 0 )			// Gnuplot failed
		{
			*choice = 2;
			return;
		}
				
		type_plot[ n ] = -1; 		// external plot
		plot_w[ n ] = -1;
		plot_l[ n ] = -1;
		plot_nl[ n ] = -1;   
		
		*choice = 0;
		return;
	}

	// get graphical configuration from Tk ( file defaults.tcl )
	get_int( "hsizePxy", & hsize );			// 640
	get_int( "vsizePxy", & vsize );			// 450
	get_int( "sbordsizeP", & sbordsize );	// 0


	// generate tk canvas filling routine using Gnuplot
	cmd( "set choice [ open_gnuplot gnuplot.lsd \"Please check if you have selected an adequate configuration for the plot and if Gnuplot is set up properly.\nIf the error persists please check if Gnuplot is installed and set up properly.\" true ]" );

	if ( *choice != 0 )						// Gnuplot failed
	{
		*choice = 2;
		return;
	}

	if ( shrink_gnufile( ) != 0 )			// file conversion failed
	{
		*choice = 2;
		return;
	}
		
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
	if ( logs )
		cmd( "label $w.b.c.case.l -text \"log(X) value:\" -width 11 -anchor e" );
	else
		cmd( "label $w.b.c.case.l -text \"X value:\" -width 11 -anchor e" );
	cmd( "label $w.b.c.case.v -text \"\" -fg red -width 20 -anchor w" );
	cmd( "pack $w.b.c.case.l $w.b.c.case.v -side left -anchor w" );

	cmd( "frame $w.b.c.y" );
	if ( logs )
		cmd( "label $w.b.c.y.l -text \"log(Y) value:\" -width 11 -anchor e" );
	else
		cmd( "label $w.b.c.y.l -text \"Y value:\" -width 11 -anchor e" );
	cmd( "label $w.b.c.y.v1 -text \"\" -fg red -width 20 -anchor w" );
	cmd( "pack $w.b.c.y.l $w.b.c.y.v1 -side left -anchor w" );

	cmd( "pack $w.b.c.case $w.b.c.y -anchor w" );

	cmd( "frame $w.b.o" );
	cmd( "label $w.b.o.l1 -text \"Right-click: edit properties\"" );
	cmd( "label $w.b.o.l2 -text \"Shift-click: insert text\"" );
	cmd( "label $w.b.o.l3 -text \"Ctrl-click: insert line\"" );
	cmd( "pack $w.b.o.l1 $w.b.o.l2 $w.b.o.l3" );

	cmd( "frame $w.b.s" );
	cmd( "button $w.b.s.save -width $butWid -text Save -command { set it \"%d) $tit\"; set fromPlot 1; set choice 11 } -state disabled -underline 0", n );
	cmd( "button $w.b.s.gnu -width $butWid -text Gnuplot -command { \
			set oldpath [pwd]; \
			cd plotxy_%d; \
			open_gnuplot gnuplot.gp; \
			cd $oldpath; \
		} -state disabled -underline 0", n );
	cmd( "pack $w.b.s.save $w.b.s.gnu -pady 5" );

	cmd( "label $w.b.pad -width 6" );

	cmd( "frame $w.b.z" );
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
	cmd( "$p xview moveto 0; $p yview moveto 0" );
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
	cmd( "catch { gnuplot $p }" );
	cmd( "catch { rename gnuplot \"\" }" );

	// canvas plot limits (canvas & series)
	cmd( "set cmx [ expr [ winfo width $p ] - 2 * [ $p cget -border ] - 2 * [ $p cget -highlightthickness ] ]" );
	cmd( "if { $cmx <= 1 } { set cmx [ $p cget -width ] }" );
	cmd( "set cmy [ expr [ winfo height $p ] - 2 * [ $p cget -border ] - 2 * [ $p cget -highlightthickness ] ]" );
	cmd( "if { $cmy <= 1 } { set cmy [ $p cget -height ] }" );
	cmd( "unset -nocomplain lim rang" );
	cmd( "catch { set lim [ gnuplot_plotarea ] }" );
	cmd( "catch { set rang [ gnuplot_axisranges ] }" );
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

			if ( is_inf( lim[ i ] ) || is_nan( lim[ i ] ) || is_inf( rang[ i ] ) || is_nan( rang[ i ] ) )
				rang[ i ] = lim[ i ] = 0;
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

	// window bindings (return to Analysis, insert text, insert line )
	cmd( "bind $w <Escape> \"wm withdraw $w\"" );
	cmd( "bind $w <s> { $w.b.s.save invoke }; bind $w <S> { $w.b.s.save invoke }" );
	cmd( "bind $w <g> { $w.b.s.gnu invoke }; bind $w <G> { $w.b.s.gnu invoke }" );
	cmd( "bind $w <plus> { $w.b.z.b.p invoke }" );
	cmd( "bind $w <minus> { $w.b.z.b.m invoke }" );
	cmd( "bind $p <Double-Button-1> { wm deiconify .da; raise .da; focus .da }" );

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
void plot_lattice( int *choice )
{
	bool stopErr = false;
	char *app, **str, **tag;
	double val, color, cscale, **data, **logdata;
	int i, j, hi, le, first = 1, last, time, hsize, vsize, ncol, nlin, tot, *start, *end, *id, logErrCnt = 0;

	if ( nv == 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}

	if ( time_cross == 0 && nv > 1 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For time series lattices select only one series.\"" );
		*choice = 2;
		return;
	}

	if ( time_cross == 1 && nv < 2 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For cross-section lattices select at least two series.\"" );
		*choice = 2;
		return;
	}

	// lattice window size
	get_int( "hsizeLat", & hsize );			// 400
	get_int( "vsizeLat", & vsize );			// 400
	get_double( "cscaleLat", & cscale );	// 1.0

	// find column number suggestion
	tot = time_cross == 1 ? nv : max_c - min_c + 1;
	ncol = ( int ) max( sqrt( tot ), 1 );
	while ( tot % ncol != 0 && ncol > 0 )
		ncol--;

	data = new double *[ nv ];
	logdata = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	cmd( "newtop .da.s \"Lattice Options\" { set choice 2 } .da" );

	cmd( "frame .da.s.t" );
	cmd( "label .da.s.t.l -width 25 -anchor e -text \"Cross-section time step\"" );
	cmd( "set time %d", num_c );
	cmd( "entry .da.s.t.e -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set time %%P; return 1 } { %%W delete 0 end; %%W insert 0 $time; return 0 } } -invcmd { bell } -justify center" );
	cmd( ".da.s.t.e insert 0 $time" ); 
	cmd( "pack .da.s.t.l .da.s.t.e -side left -anchor w -padx 2 -pady 2" );

	cmd( "frame .da.s.i" );
	cmd( "label .da.s.i.l -width 25 -anchor e -text \"Number of data columns\"" );
	cmd( "set bidi %d", ncol );
	cmd( "entry .da.s.i.e -width 5 -validate focusout -vcmd { if { [ string is integer -strict %%P ] && %%P > 0 } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e -side left -anchor w -padx 2 -pady 2" );

	if ( time_cross == 1 )
	{
		cmd( "pack .da.s.t .da.s.i -anchor w -padx 5 -pady 5" );
		cmd( "bind .da.s.t.e <KeyPress-Return> {focus .da.s.i.e; .da.s.i.e selection range 0 end}" );
		cmd( "focus .da.s.t.e; .da.s.t.e selection range 0 end" );
	}
	else
	{
		cmd( "pack .da.s.i -anchor w -padx 5 -pady 5" );
		cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );
	}

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#lattice } { set choice 2 }" );

	cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );
		
	cmd( "showtop .da.s" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set bidi [ .da.s.i.e get ]" ); 
	cmd( "set time [ .da.s.t.e get ]" ); 
	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
		goto end1;

	// prepare data from selected series
	cmd( "set choice $time" );
	time = *choice;

	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];

		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
	  
		// check if series has data for all selected cases (cross-section only )
		if ( time_cross == 1 && ( time < start[ i ] || time > end[ i ] ) )
		{
			cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid time step\" -detail \"One or more of the series do not have a value associated to the selected cross-section time step.\"" );
			*choice = 2;
			
			for ( j = i + 1; j < nv; ++j )		// indicate non allocated positions
			{
				str[ j ] = tag[ j ] = NULL;
				logdata[ j ] = NULL;
			}
			
			goto end1;
		}
		
		// get series data and take logs if necessary
		data[ i ] = vs[ id[ i ] ].data;
		if ( data[ i ] == NULL )
			plog( "\nError: invalid data\n" );
	  
		if ( logs )			// apply log to the values to show "log scale"
		{
			logdata[ i ] = new double[ end[ i ] + 1 ];	// create space for the logged values
			for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
			if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )		// ignore NaNs
				logdata[ i ][ j ] = log( data[ i ][ j ] );
			else
			{
				logdata[ i ][ j ] = NAN;
				if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
					plog( "\nWarning: zero or negative values in log lattice (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
				else
					if ( ! stopErr )
					{
						plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
						stopErr = true;
					}
				}
				
			data[ i ] = logdata[ i ];				// replace the data series
		}
	}


	// define lattice configuration
	cmd( "set choice $bidi" );
	ncol = *choice;

	if ( time_cross == 0 )
	{
		if ( autom_x || min_c >= max_c )
		{
			first = max( start[ 0 ], 1 );
			last = end[ 0 ];
		}
		else
		{
			if ( min_c > max( start[ 0 ], 1 ) )
				first = min_c;
			else
				first = max( start[ 0 ], 1 );
			
			if ( max_c < end[ 0 ] )  
				last = max_c;
			else
				last = end[ 0 ];
		}
		
		for ( tot = 0, i = first; i <= last; ++i )	// count number of points excluding NaNs
			if ( ! is_nan( data[ 0 ][ i ] ) && is_finite( data[ 0 ][ i ] ) )
				tot++;
	}
	else
		tot = nv;

	nlin = tot / ncol;

	if ( nlin * ncol != tot )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of columns\" -detail \"The number of columns must be an exact divisor of the number (%d) of time steps (time series) or selected variables (cross section).\"", tot );
		*choice = 2;
		goto end2;
	}

	// draw lattice
	hi = vsize / nlin;
	le = hsize / ncol;

	cmd( "set w .da.f.new%d", cur_plot );
	cmd( "newtop $w $tit { wm withdraw .da.f.new%d } \"\"", cur_plot );

	cmd( "frame $w.f -width %d -height %d", ncol * le, nlin * hi );
	cmd( "pack $w.f" );

	cmd( "set p $w.f.plots" );
	cmd( "canvas $p -width %d -height %d -background white -relief flat", ncol * le, nlin * hi );
	cmd( "pack $p" );

	cmd( "showtop $w current yes yes no" );

	for ( j = 0; j < nlin; ++j )
		for ( i = 0; i < ncol; ++i )
		{
			val = time_cross == 1 ? data[ ncol * j + i ][ time ] : 
									data[ 0 ][ first + ncol * j + i ];
			color = max( 0, min( 1099, round( val * cscale ) ) );
			if ( is_nan( color ) || ! is_finite( color ) )
			  color = 0;

			cmd( "plot_bars $p %d %d %d %d p%d_%d $c%d %lf", 
			   i * le, j * hi, ( i + 1 ) * le, ( j + 1 ) * hi, j, i, 
			   ( int ) color, grid ? point_size : 0.0 );
		}

	// window bindings (return to Analysis, insert text, insert line )
	cmd( "bind $w <Escape> \"wm withdraw $w\"" );
	cmd( "bind $p <Double-Button-1> { wm deiconify .da; raise .da; focus .da }" );

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
			} elseif { [ lsearch $type bar ] >= 0 } { \
				set choice 42 \
			} elseif { [ lsearch $type text ] >= 0 } { \
				set choice 26 \
			} \
		}", cur_plot );
	cmd( "bind $p <Button-3> { event generate .da.f.new%d.f.plots <Button-2> -x %%x -y %%y }", cur_plot );

	// save plot info
	type_plot[ cur_plot ] = LATTICE;
	plot_w[ cur_plot ] = ncol * le;	// plot width
	plot_l[ cur_plot ] = nlin * hi;
	plot_nl[ cur_plot ] = nlin * hi;  

	end2:
	
	*choice = 0;

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}

	end1:

	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/***************************************************
HISTOGRAMS
****************************************************/
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

bin *bins;	
int num_bins, cases, time_cs;
double mean, var;

void histograms( int *choice )
{
	char *app, **str, **tag;
	double mx = 0, mn = 0, step, a, lminy, lmaxy, *data, *logdata = NULL;
	int i, j, first, last, stat, start, end, id;

	int logErrCnt = 0;				// log errors counter to prevent excess messages
	bool norm, stopErr = false;

	if ( nv != 1 )
	{
		if ( nv == 0 )			// no variables selected
			cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one series in the Series Selected listbox.\"" );
		else
			cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For time series histograms select only one series.\"" );
		*choice = 2;
		return;
	} 
	 
	str = new char *[ 1 ];
	tag = new char *[ 1 ];
	str[ 0 ] = new char[ MAX_ELEM_LENGTH ];
	tag[ 0 ] = new char[ MAX_ELEM_LENGTH ];

	cmd( "set res [.da.vars.ch.v get 0]" );
	app = ( char * ) Tcl_GetVar( inter, "res", 0 );
	strcpy( msg, app );
	sscanf( msg, "%s %s (%d-%d) #%d", str[ 0 ], tag[ 0 ], &start, &end, &id );

	data = vs[ id ].data;
	if ( data == NULL )
		plog( "\nError: invalid data\n" );

	if ( logs )			// apply log to the values to show "log scale" in the y-axis
	{
		logdata = new double[ end + 1 ];		// create space for the logged values
		for ( j = start; j <= end; ++j )		// log everything possible
			if ( ! is_nan( data[ j ] ) && data[ j ] > 0.0 )	// ignore NaNs
				logdata[ j ] = log( data[ j ] );
			else
			{
				logdata[ j ] = NAN;
				if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
				{
					plog( "\nWarning: zero or negative values in log histogram (ignored)\nCase: %d", "", j );
				}
				else
					if ( ! stopErr )
					{
						plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
						stopErr = true;
					}
			}
		   
		data = logdata;							// replace the data series
	}

	if ( autom_x || min_c >= max_c )
	{
		first = max( start, 1 );
		last = end;
	}
	else
	{
		if ( min_c > max( start, 1 ) )
			first = min_c;
		else
			first = max( start, 1 );
		
		if ( max_c < end )  
			last = max_c;
		else
			last = end;
	}  

	for ( j = 0, i = first; i <= last; ++i )	// count number of points excluding NaNs
		if ( ! is_nan( data[ i ] ) && is_finite( data[ i ] ) )
			++j;

	cmd( "newtop .da.s \"Histogram Options\" { set choice 2 } .da" );

	cmd( "frame .da.s.i" );
	cmd( "label .da.s.i.l -text \"Number of classes/bins\"" );
	cmd( "set bidi %d", j < 25 ? j : 25 );
	cmd( "entry .da.s.i.e -width 5 -validate focusout -vcmd { if { [ string is integer -strict %%P ] && %%P > 0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e -side left -padx 2" );

	cmd( "set norm 0" );
	cmd( "checkbutton .da.s.norm -text \"Fit a Normal\" -variable norm" );
	cmd( "set stat 0" );
	cmd( "checkbutton .da.s.st -text \"Show statistics\" -variable stat" );
	cmd( "pack .da.s.i .da.s.norm .da.s.st -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#histogram } { set choice 2 }" );

	cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set bidi [ .da.s.i.e get ]" ); 
	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
		goto end;

	cmd( "set choice $bidi" );
	num_bins = *choice;

	mean = var = cases = 0;
	for ( i = first; i <= last; ++i )
	{
		if ( is_nan( data[ i ] ) || ! is_finite( data[ i ] ) )	// ignore NaNs
			continue;
			
		if ( i == first )
			mx = mn = data[ i ];
		else
		{
			if ( data[ i ] > mx )
				mx = data[ i ];
			else
				if ( data[ i ] < mn )
					mn = data[ i ];
		}  
		mean += data[ i ];
		var += data[ i ]*data[ i ];
		cases++;
	}

	if ( cases == 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid data\" -detail \"The selected series has no valid data for the chosen time step cases.\"" );
		*choice = 2;
		goto end;
	}

	if ( mx - mn <= 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid data\" -detail \"The selected series has no data variation for the chosen time step cases.\"" );
		*choice = 2;
		goto end;
	}

	mean = mean / cases;
	var = var / cases - mean * mean;

	bins = new bin[ num_bins ];
	for ( i = 0; i < num_bins; ++i )
	{
		bins[ i ].num = 0;
		bins[ i ].av = 0;
		bins[ i ].min = 0;
		bins[ i ].max = 0;
		bins[ i ].center = 0; 
	}
	 
	for ( i = first; i <= last; ++i )
	{
		if ( is_nan( data[ i ] ) || ! is_finite( data[ i ] ) )
			continue;

		a = floor( num_bins*( data[ i ] - mn ) / ( mx - mn ) );

		j = ( int ) a;
		if ( j == num_bins)
			j--;

		if ( bins[ j ].num == 0 )
			bins[ j ].min=bins[ j ].max = data[ i ];
		else
		{
			if ( bins[ j ].min > data[ i ] )
				bins[ j ].min = data[ i ];
			else
				if ( bins[ j ].max < data[ i ] )
					bins[ j ].max = data[ i ];
		}  
		
		bins[ j ].num++;   
		bins[ j ].av += data[ i ];
	} 

	a = ( mx - mn ) / ( num_bins - 1 );

	for ( i = 1; i < num_bins; ++i )
		if ( bins[ i ].num != 0 && bins[ i - 1 ].num != 0 && bins[ i ].min - bins[ i - 1 ].max < a )
			a = bins[ i ].min - bins[ i - 1 ].max;

	cmd( "set choice $stat" );
	stat = *choice;

	if ( stat == 1 )
		plog( "\nTime series histogram statistics\n #   Boundaries(center)\t\tMin.\tAverage\tMax.\tNum.\tFreq." );

	step = ( mx + a / 2 - ( mn - a / 2 ) ) / num_bins;
	lminy = cases;
	lmaxy = 0;

	for ( i = 0; i < num_bins; ++i )
	{
		if ( bins[ i ].num != 0 )
			bins[ i ].av /= bins[ i ].num;
		
		bins[ i ].lowb = mn - a / 2 + ( double ) i * step;
		bins[ i ].highb = mn - a / 2 + ( double ) ( i + 1 ) * step;
		bins[ i ].center = bins[ i ].highb / 2 + bins[ i ].lowb / 2;
		
		if ( stat == 1 )
			plog( "\n%3d: \\[%.*g, %.*g\\] (%.*g)\t\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g", "", i + 1, pdigits, mn - a / 2 + ( double ) i * step, pdigits, mn - a / 2 + ( double ) ( i + 1 ) * step, pdigits, mn - a / 2 + ( double ) i * step + step / 2, pdigits, bins[ i ].min, pdigits, bins[ i ].av, pdigits, bins[ i ].max, pdigits, bins[ i ].num, pdigits, bins[ i ].num / cases );
		
		if ( bins[ i ].num < lminy )
			lminy = bins[ i ].num;
		
		if ( bins[ i ].num > lmaxy )
			lmaxy = bins[ i ].num;
	}

	if ( stat == 1 )
	{
		plog( "\n\n" );
		cmd( "wm deiconify .log; raise .log .da" ); 
	}

	if ( autom || miny >= maxy )
	{
		maxy = lmaxy / cases;
		miny = lminy > 0 ? ( lminy - 1 ) / cases : 0;
		update_bounds( );
	}

	cmd( "set choice $norm" );
	norm = *choice ? true : false;

	// plot histogram
	plot( HISTOGR, &start, &end, str, tag, choice, norm );

	*choice = 0;

	delete [ ] bins;

	end:

	delete [ ] str[ 0 ];
	delete [ ] tag[ 0 ];
	delete [ ] str;
	delete [ ] tag;
	delete [ ] logdata;
}


/***************************************************
HISTOGRAMS CS
****************************************************/
void histograms_cs( int *choice )
{
	bool norm, stopErr = false;
	char *app, **str, **tag;
	double mx = 0, mn = 0, step, a, lminy, lmaxy, **data, **logdata;
	int i, j, stat, active_v, *start, *end, *id, logErrCnt = 0;

	if ( nv < 2 )
	{
		if ( nv == 0 )			// no variables selected
			cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place two or more series in the Series Selected listbox.\"" );
		else
			cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For cross-section histograms select at least two series.\"" );
		
		*choice = 2;
		return;
	}

	data = new double *[ nv ];
	logdata = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		
		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		data[ i ] = vs[ id[ i ] ].data;
		if ( data[ i ] == NULL )
			plog( "\nError: invalid data\n" );
	  
		if ( logs )			// apply log to the values to show "log scale" in the y-axis
		{
			logdata[ i ] = new double[ end[ i ] + 1 ];	// create space for the logged values
			for ( j = start[ i ]; j <= end[ i ]; ++j )		// log everything possible
				if ( ! is_nan( data[ i ][ j ] ) && data[ i ][ j ] > 0.0 )		// ignore NaNs
					logdata[ i ][ j ] = log( data[ i ][ j ] );
				else
				{
					logdata[ i ][ j ] = NAN;
					if ( ++logErrCnt < ERR_LIM )	// prevent slow down due to I/O
						plog( "\nWarning: zero or negative values in log histogram (ignored)\n         Series: %d, Case: %d", "", i + 1, j );
					else
						if ( ! stopErr )
						{
							plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
							stopErr = true;
						}
				}
				
			data[ i ] = logdata[ i ];				// replace the data series
		}
	}

	cmd( "newtop .da.s \"Histogram Options\" { set choice 2 } .da" );

	cmd( "frame .da.s.t" );
	cmd( "label .da.s.t.l -text \"Cross-section time step\"" );
	cmd( "set time %d", end[ 0 ] );
	cmd( "entry .da.s.t.e -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set time %%P; return 1 } { %%W delete 0 end; %%W insert 0 $time; return 0 } } -invcmd { bell } -justify center" );
	cmd( ".da.s.t.e insert 0 $time" ); 
	cmd( "pack .da.s.t.l .da.s.t.e -side left -padx 2" );

	cmd( "frame .da.s.i" );
	cmd( "label .da.s.i.l -text \"Number of classes/bins\"" );
	cmd( "set bidi %d", nv < 25 ? nv : 25 );
	cmd( "entry .da.s.i.e -width 5 -validate focusout -vcmd { if { [ string is integer -strict %%P ] && %%P > 0 && %%P <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invcmd { bell } -justify center" );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e -side left -padx 2" );

	cmd( "set norm 0" );
	cmd( "checkbutton .da.s.norm -text \"Fit a Normal\" -variable norm" );
	cmd( "set stat 0" );
	cmd( "checkbutton .da.s.st -text \"Show statistics\" -variable stat" );
	cmd( "pack .da.s.t .da.s.i .da.s.norm .da.s.st -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#histogram } { set choice 2 }" );

	cmd( "bind .da.s.t.e <Return> {focus .da.s.i.e; .da.s.i.e selection range 0 end}" );
	cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.t.e; .da.s.t.e selection range 0 end" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set bidi [ .da.s.i.e get ]" ); 
	cmd( "set time [ .da.s.t.e get ]" ); 
	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
		goto end;

	cmd( "set choice $bidi" );
	num_bins = *choice;
	cmd( "set choice $time" );
	time_cs = *choice;

	mean = var = cases = 0;
	active_v = 0;
	for ( i = 0; i < nv; ++i )
		if ( start[ i ] <= time_cs && end[ i ] >= time_cs && is_finite( data[ i ][ time_cs ] ) )		// ignore NaNs
		{
			if ( active_v == 0 )
				mx = mn = data[ i ][ time_cs ];
			else
			{
				if ( data[ i ][ time_cs ] > mx )
					mx = data[ i ][ time_cs ];
				else
					if ( data[ i ][ time_cs ] < mn )
						mn = data[ i ][ time_cs ];
			}  
			
			mean += data[ i ][ time_cs ];
			var += data[ i ][ time_cs ]*data[ i ][ time_cs ];
			cases++;
			active_v++;
		}

	if ( cases == 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"Invalid data\" -detail \"The selected series have no valid data in the chosen time step.\"" );
		*choice = 2;
		goto end;
	}

	mean = mean / cases;
	var = var / cases - mean * mean;

	bins = new bin[ num_bins ];
	for ( i = 0; i < num_bins; ++i )
	{
		bins[ i ].num = 0;
		bins[ i ].av = 0;
		bins[ i ].min = 0;
		bins[ i ].max = 0;  
		bins[ i ].center = 0; 
	}
	 
	for ( i = 0; i < nv; ++i )
	{
		if ( start[ i ] > time_cs || end[ i ] < time_cs || ! is_finite( data[ i ][ time_cs ] ) )
			continue;

		a = floor( num_bins * ( data[ i ][ time_cs ] - mn ) / ( mx - mn ) );
			
		j = ( int ) a;
		if ( j == num_bins )
			--j;
		
		if ( bins[ j ].num == 0 )
			bins[ j ].min=bins[ j ].max = data[ i ][ time_cs ];
		else
		{
			if ( bins[ j ].min > data[ i ][ time_cs ] )
				bins[ j ].min = data[ i ][ time_cs ];
			else
				if ( bins[ j ].max < data[ i ][ time_cs ] )
					bins[ j ].max = data[ i ][ time_cs ];
		}
		
		bins[ j ].num++;   
		bins[ j ].av += data[ i ][ time_cs ];
	} 

	a = ( mx - mn ) / ( num_bins - 1 );
	for ( i = 1; i < num_bins; ++i )
		if ( bins[ i ].num != 0 && bins[ i - 1 ].num != 0 && bins[ i ].min - bins[ i - 1 ].max < a )
			a = bins[ i ].min - bins[ i - 1 ].max;

	cmd( "set choice $stat" );
	stat = *choice;

	if ( stat == 1 )
		plog( "\nCross-section histogram statistics\n#    Boundaries(center)\t\tMin\tAve\tMax\tNum.\tFreq." );

	step = ( mx + a / 2 - ( mn - a / 2 ) ) / num_bins;
	lminy = active_v;
	lmaxy = 0;

	for ( i = 0; i < num_bins; ++i )
	{
		if ( bins[ i ].num != 0 )
			bins[ i ].av /= bins[ i ].num;
		bins[ i ].lowb = mn - a / 2 + ( double ) i * step;
		bins[ i ].highb = mn - a / 2 + ( double ) ( i + 1 ) * step;
		bins[ i ].center = bins[ i ].highb / 2 + bins[ i ].lowb / 2;
		
		if ( stat == 1 )
			plog( "\n%3d: \\[%.*g, %.*g\\] (%.*g)\t\t%.*g\t%.*g\t%.*g\t%.*g\t%.*g", "", i + 1, pdigits, mn - a / 2 + ( double ) i * step, pdigits, mn - a / 2 + ( double ) ( i + 1 ) * step, pdigits, mn - a / 2 + ( double ) i * step + step / 2, pdigits, bins[ i ].min, pdigits, bins[ i ].av, pdigits, bins[ i ].max, pdigits, bins[ i ].num, pdigits, bins[ i ].num / cases );
		
		if ( bins[ i ].num < lminy )
			lminy = bins[ i ].num;
		if ( bins[ i ].num > lmaxy )
			lmaxy = bins[ i ].num;
	}

	if ( stat == 1 )
	{
		plog( "\n\n" );
		cmd( "wm deiconify .log; raise .log .da" ); 
	}

	if ( autom || miny >= maxy )
	{
		maxy = lmaxy / cases;
		miny = lminy > 0 ? ( lminy - 1 ) / cases : 0;
		update_bounds( );
	}

	cmd( "set choice $norm" );
	norm = *choice ? true : false;

	// plot histogram
	plot( HISTOCS, start, end, str, tag, choice, norm );

	*choice = 0;

	delete [ ] bins;

	end:

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] logdata[ i ];
	}
	
	delete [ ] str;
	delete [ ] tag; 
	delete [ ] logdata;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/***************************************************
CREATE_SERIES
****************************************************/
// Confidence level  0.80      0.81      0.82      0.83      0.84      0.85      0.86      0.87      0.88      0.89      0.90      0.91      0.92      0.93      0.94      0.95       0.96      0.97      0.98      0.99
double z_star[ ] = { 1.281552, 1.310579, 1.340755, 1.372204, 1.405072, 1.439531, 1.475791, 1.514102, 1.554774, 1.598193, 1.644854, 1.695398, 1.750686, 1.811911, 1.880794, 1.959964,  2.053749, 2.170090, 2.326348, 2.575829 };

void create_series( int *choice )
{
	bool first;
	char *lapp, **str, **tag;
	double nmax = 0, nmin = 0, nmean, nvar, nn, thflt, z_crit, **data;
	int i, j, k, flt, cs_long, type_series, new_series, confi, *start, *end, *id;
	store *app;

	if ( nv == 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return;
	}

	if ( logs )
		cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

	Tcl_LinkVar( inter, "thflt", ( char * ) &thflt, TCL_LINK_DOUBLE );
	Tcl_LinkVar( inter, "confi", ( char * ) &confi, TCL_LINK_INT );

	thflt = 0;
	confi = 95;
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
	cmd( "entry .da.s.f.t.th -width 10 -validate focusout -vcmd { if [ string is double -strict %%P ] { set thflt %%P; return 1 } { %%W delete 0 end; %%W insert 0 $thflt; return 0 } } -invcmd { bell } -justify center -state disabled" );
	cmd( "write_disabled .da.s.f.t.th $thflt" ); 
	cmd( "pack .da.s.f.t.l .da.s.f.t.th -side left -padx 2" );

	cmd( "pack .da.s.f.l .da.s.f.r .da.s.f.t" );

	cmd( "frame .da.s.i" );
	cmd( "label .da.s.i.l -text \"Type of series\"" );

	cmd( "frame .da.s.i.r -relief groove -bd 2" );
	cmd( "radiobutton .da.s.i.r.m -text \"Average\" -variable bidi -value 1 -command { .da.s.i.r.ci.p configure -state disabled; set headname \"Avg\"; set vname $headname$basename; .da.s.n.nv selection range 0 end }" );
	cmd( "radiobutton .da.s.i.r.z -text \"Sum\" -variable bidi -value 5 -command { .da.s.i.r.ci.p configure -state disabled; set headname \"Sum\"; set vname $headname$basename; .da.s.n.nv selection range 0 end  }" );
	cmd( "radiobutton .da.s.i.r.n -text \"Count\" -variable bidi -value 7 -command { .da.s.i.r.ci.p configure -state disabled; set headname \"Num\"; set vname $headname$basename; .da.s.n.nv selection range 0 end }" );
	cmd( "radiobutton .da.s.i.r.f -text \"Maximum\" -variable bidi -value 2 -command { .da.s.i.r.ci.p configure -state disabled; set headname \"Max\"; set vname $headname$basename; .da.s.n.nv selection range 0 end }" );
	cmd( "radiobutton .da.s.i.r.t -text \"Minimum\" -variable bidi -value 3 -command { .da.s.i.r.ci.p configure -state disabled; set headname \"Min\"; set vname $headname$basename; .da.s.n.nv selection range 0 end }" );
	cmd( "radiobutton .da.s.i.r.c -text \"Variance\" -variable bidi -value 4 -command { .da.s.i.r.ci.p configure -state disabled; set headname \"Var\"; set vname $headname$basename; .da.s.n.nv selection range 0 end }" );
	cmd( "radiobutton .da.s.i.r.s -text \"Standard deviation\" -variable bidi -value 8 -command { .da.s.i.r.ci.p configure -state disabled; set headname \"SD\"; set vname $headname$basename; .da.s.n.nv selection range 0 end }" );

	cmd( "frame .da.s.i.r.ci" );
	cmd( "radiobutton .da.s.i.r.ci.c -text \"Confidence interval (3 series)\" -variable bidi -value 6 -command { .da.s.i.r.ci.p configure -state normal; set headname \"\"; set vname $headname$basename; .da.s.n.nv selection range 0 end }" );
	cmd( "label .da.s.i.r.ci.x -text @" );
	cmd( "label .da.s.i.r.ci.perc -text %%" );
	cmd( "entry .da.s.i.r.ci.p -width 3 -validate focusout -vcmd { if { [ string is integer -strict \"%%P\" ] && %%P >= 80 && %%P <= 99 } { set confi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $confi; return 0 } } -invcmd { bell } -justify center -state disabled" );
	cmd( "write_disabled .da.s.i.r.ci.p $confi" ); 
	cmd( "pack .da.s.i.r.ci.c .da.s.i.r.ci.x .da.s.i.r.ci.p .da.s.i.r.ci.perc -side left" );

	cmd( "pack .da.s.i.r.m .da.s.i.r.z .da.s.i.r.n .da.s.i.r.f .da.s.i.r.t .da.s.i.r.c .da.s.i.r.s .da.s.i.r.ci -anchor w" );
	cmd( "pack .da.s.i.l .da.s.i.r" );

	cmd( "set a [ .da.vars.ch.v get 0 ]" );
	cmd( "set basename [ lindex [ split $a ] 0 ]" );
	cmd( "set headname \"Avg\"" );
	cmd( "set vname $headname$basename" );

	cmd( "frame .da.s.n" );
	cmd( "label .da.s.n.lnv -text \"New series label\"" );
	cmd( "entry .da.s.n.nv -width 30 -textvariable vname -justify center" );
	cmd( "pack .da.s.n.lnv .da.s.n.nv" );

	cmd( "frame .da.s.t" );
	cmd( "label .da.s.t.tnv -text \"New series tag\"" );
	cmd( "entry .da.s.t.tv -width 20 -textvariable vtag -justify center" );
	cmd( "pack .da.s.t.tnv .da.s.t.tv" );

	cmd( "pack .da.s.o .da.s.f .da.s.i .da.s.n .da.s.t -padx 5 -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#createselection } { set choice 2 }" );

	cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
	cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.n.nv" );
	cmd( ".da.s.n.nv selection range 0 end" );
	 
	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set thflt [ .da.s.f.t.th get ]" ); 
	cmd( "set confi [ .da.s.i.r.ci.p get ]" ); 
	cmd( "destroytop .da.s" );

	Tcl_UnlinkVar( inter, "thflt" );
	Tcl_UnlinkVar( inter, "confi" );

	if ( *choice == 2 )
	{
		*choice = 0;
		return;
	}
	
	cmd( "set choice $flt" );
	flt = *choice;
	cmd( "set choice $bido" );
	cs_long = *choice;
	cmd( "set choice $bidi" );
	type_series = *choice;
	
	// set option specific parameters
	if ( type_series == 6 )
	{
		new_series = 3;
		
		// first series to produce
		cmd( "set basename $vname; set headname \"Avg\"; set vname $headname$basename" );
		
		// get the critical value to the chosen confidence level
		z_crit = z_star[ ( int ) max( min( confi, 99 ), 80 ) - 80 ];	
	}
	else
	{
		new_series = 1;
		z_crit = 0;
	}
	
	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	// allocate space for the new series
	app = new store[ num_var + new_series ];
	for ( i = 0; i < num_var; ++i )
	{
		app[ i ] = vs[ i ];
		strcpy( app[ i ].label, vs[ i ].label );
		strcpy( app[ i ].tag, vs[ i ].tag);
	} 

	delete [ ] vs;
	vs = app;

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;

		cmd( "set res [.da.vars.ch.v get %d]", i );
		lapp = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg,lapp );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
				plog( "\nError: invalid data\n" );
		}
	}

	if ( autom_x || min_c >= max_c )
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = max( start[ i ], 1 );
			if ( start[ i ] < min_c )
				min_c = max( start[ i ], 1 );
			if ( end[ i ] > max_c )
				max_c = end[ i ] > num_c ? num_c : end[ i ];
		}

	// handle creation of multiple series
	for ( k = 0; k < new_series; ++k, ++num_var )
	{
		if ( cs_long == 1 )									// compute over series?
		{
			vs[ num_var ].data = new double[ max_c + 2 ];
			
			lapp = ( char * ) Tcl_GetVar( inter, "vname", 0 );
			strcpy( vs[ num_var ].label, lapp );
			vs[ num_var ].end = max_c;
			vs[ num_var ].start = min_c;
			lapp = ( char * ) Tcl_GetVar( inter, "vtag", 0 );
			strcpy( vs[ num_var ].tag, lapp );
			vs[ num_var ].rank = num_var;

			for ( i = min_c; i <= max_c; ++i )
			{
				nmean = nn = nvar = 0;
				first = true;
				for ( j = 0; j < nv; ++j )
				{
					if ( i >= start[ j ] && i <= end[ j ] && is_finite( data[ j ][ i ] ) && ( flt == 0 || ( flt == 1 && data[ j ][ i ] > thflt) || ( flt == 2 && data[ j ][ i ] < thflt) ) )		// ignore NaNs
					{
						nmean += data[ j ][ i ];
						nvar += data[ j ][ i ] * data[ j ][ i ];
						nn++;
						if ( first)
						{
							nmin = nmax = data[ j ][ i ];
							first = false;
						}
						else
						{
							if ( nmin>data[ j ][ i ] )
								nmin = data[ j ][ i ];
							if ( nmax<data[ j ][ i ] )
								nmax = data[ j ][ i ];
						} 
					}
				}

				if ( nn == 0 )	// not a single valid value?
					nn = nmean = nvar = nmin = nmax = NAN;
				else
				{
					nmean /= nn;
					nvar /= nn;
					nvar -= nmean * nmean;
				}
			   
				if ( type_series == 1 || type_series == 6 )
					vs[ num_var ].data[ i ] = nmean;
				if ( type_series == 2 )
					vs[ num_var ].data[ i ] = nmax;
				if ( type_series == 3 )
					vs[ num_var ].data[ i ] = nmin;
				if ( type_series == 4 )
					vs[ num_var ].data[ i ] = nvar;
				if ( type_series == 5 )
					vs[ num_var ].data[ i ] = nmean * nn;
				if ( type_series == 7 )
					vs[ num_var ].data[ i ] = nn;
				if ( type_series == 8 )
					vs[ num_var ].data[ i ] = sqrt( nvar );
				if ( type_series == 11 )
					vs[ num_var ].data[ i ] = nmean + z_crit * sqrt( nvar ) / sqrt( nn );
				if ( type_series == 12 )
					vs[ num_var ].data[ i ] = nmean - z_crit * sqrt( nvar ) / sqrt( nn );
					
			}
			cmd( ".da.vars.lb.v insert end \"%s %s (%d-%d) #%d (created)\"", vs[ num_var ].label, vs[ num_var ].tag, min_c, max_c, num_var ); 

			cmd( "lappend DaModElem %s", vs[ num_var ].label  );
		} 
		else												// compute over cases
		{
			vs[ num_var ].data = new double[ nv ];
			lapp = ( char * ) Tcl_GetVar( inter, "vname", 0 );
			strcpy( vs[ num_var ].label, lapp );
			vs[ num_var ].end=nv - 1;
			vs[ num_var ].start = 0;
			lapp = ( char * ) Tcl_GetVar( inter, "vtag", 0 );
			strcpy( vs[ num_var ].tag, lapp );
			vs[ num_var ].rank = num_var;

			for ( j = 0; j < nv; ++j )
			{
				nmean = nn = nvar = 0;
				first = true;
				for ( i = min_c; i <= max_c; ++i )
				{
					if ( i >= start[ j ] && i <= end[ j ] && is_finite( data[ j ][ i ] ) && ( flt == 0 || ( flt == 1 && data[ j ][ i ] > thflt) || ( flt == 2 && data[ j ][ i ] < thflt) ) )
					{
						nmean += data[ j ][ i ];
						nvar += data[ j ][ i ]*data[ j ][ i ];
						nn++;
						if ( first)
						{
							nmin = nmax = data[ j ][ i ];
							first = false;
						}
						else
						{
							if ( nmin>data[ j ][ i ] )
								nmin = data[ j ][ i ];
							if ( nmax<data[ j ][ i ] )
								nmax = data[ j ][ i ];
						} 
					}
			   }

			   if ( nn == 0 )	// not a single valid value?
					nn = nmean = nvar = nmin = nmax = NAN;
			   else
			   {
					nmean /= nn;
					nvar /= nn;
					nvar -= nmean * nmean;
			   }
				  
				if ( type_series == 1 || type_series == 6 )
					vs[ num_var ].data[ j ] = nmean;
				if ( type_series == 2 )
					vs[ num_var ].data[ j ] = nmax;
				if ( type_series == 3 )
					vs[ num_var ].data[ j ] = nmin;
				if ( type_series == 4 )
					vs[ num_var ].data[ j ] = nvar;
				if ( type_series == 5 )
					vs[ num_var ].data[ j ] = nmean * nn;
				if ( type_series == 7 )
					vs[ num_var ].data[ j ] = nn;
				if ( type_series == 8 )
					vs[ num_var ].data[ i ] = sqrt( nvar );
				if ( type_series == 11 )
					vs[ num_var ].data[ i ] = nmean + z_crit * sqrt( nvar ) / sqrt( nn );
				if ( type_series == 12 )
					vs[ num_var ].data[ i ] = nmean - z_crit * sqrt( nvar ) / sqrt( nn );
					
			 }

			cmd( ".da.vars.lb.v insert end \"%s %s (%d-%d) #%d (created)\"", vs[ num_var ].label, vs[ num_var ].tag, 0, nv - 1, num_var ); 

			cmd( "lappend DaModElem %s", vs[ num_var ].label  );
		}
		
		// define next series options for multiple series (must be reverse order!)
		if ( type_series == 11 )
		{
			type_series = 12;
			cmd( "set headname \"CILo\"; set vname $headname$basename" ); 
		}
		
		if ( type_series == 6 )
		{
			type_series = 11;
			cmd( "set headname \"CIUp\"; set vname $headname$basename" ); 
		}
	}

	cmd( ".da.vars.lb.v see end" );
	
	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
	}
	
	delete [ ] str;
	delete [ ] tag; 
	delete [ ] data;	
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/***************************************************
CREATE_MAVERAG
****************************************************/
void create_maverag( int *choice )
{
	char *lapp, **str, **tag;
	double xapp, **data;
	int h, i, j, k, flt, ma_type, *start, *end, *id;
	store *app;

	if ( nv == 0 )
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return;
	}

	if ( logs )
		cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

	cmd( "set bido 4" );
	cmd( "set ma_type 0" );
	
	cmd( "newtop .da.s \"Moving Average Period\" { set choice 2 } .da" );

	cmd( "frame .da.s.o" );
	cmd( "label .da.s.o.l -text \"Number of time steps\"" );
	cmd( "entry .da.s.o.th -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set bido %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bido; return 0 } } -invcmd { bell } -justify center" );
	cmd( ".da.s.o.th insert 0 $bido" ); 
	cmd( "pack .da.s.o.l .da.s.o.th" );
	
	cmd( "frame .da.s.t -bd 2 -relief groove" );
	cmd( "radiobutton .da.s.t.s -variable ma_type -value 0 -text \"Simple moving average\"" );
	cmd( "radiobutton .da.s.t.c -variable ma_type -value 1 -text \"Central moving average\"" );
	cmd( "pack .da.s.t.s .da.s.t.c" );

	cmd( "pack .da.s.o .da.s.t -padx 5 -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#createmavg } { set choice 2 }" );

	cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
	cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.o.th" );
	cmd( ".da.s.o.th selection range 0 end" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set bido [ .da.s.o.th get ]" ); 
	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
	{
		*choice = 0;
		return;
	}

	get_int( "bido", &flt );
	get_int( "ma_type", &ma_type );
	
	// adjust to odd number, if required
	if ( flt < 2 )
	{
		cmd( "tk_messageBox -parent .da -type ok -icon error -title Error -message \"Invalid moving average period\" -detail \"Please choose a period larger than one time step.\"" );
		*choice = 0;
		return;
	}

	if ( ma_type == 1 && flt % 2 == 0 )
	{
		++flt;
		plog( "\nRounding up the period to %d for an odd number of time steps\n", "", flt );
	}

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	app = new store[ nv + num_var ];
	for ( i = 0; i < num_var; ++i )
	{
		app[ i ] = vs[ i ];
		strcpy( app[ i ].label, vs[ i ].label );
		strcpy( app[ i ].tag, vs[ i ].tag);
	} 
	
	delete [ ] vs;
	vs = app;

	if ( autom_x )
	{
		min_c = 1;
		max_c = num_c;
	}

	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;

		cmd( "set res [.da.vars.ch.v get %d]", i );
		lapp = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, lapp );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );

		sprintf( msg, "%cMA%d_%s", ma_type == 0 ? 'S' : 'C', flt, str[ i ] );
		strcpy( vs[ num_var + i ].label, msg );
		strcpy( vs[ num_var + i ].tag,tag[ i ] );
		vs[ num_var + i ].start = ( ma_type == 0 ) ? start[ i ] + flt - 1 : start[ i ];
		vs[ num_var + i ].end = end[ i ];
		vs[ num_var + i ].rank = num_var + i;
		vs[ num_var + i ].data = new double[ max_c + 2 ];
		
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
				plog( "\nError: invalid data\n" );

			if ( ma_type == 0 )		// simple moving average
			{
				for ( k = start[ i ] + flt - 1; k <= end[ i ]; ++k )
				{
					for ( xapp = 0, h = 0, j = k - flt + 1; j <= k; ++j )
						if ( is_finite( data[ i ][ j ] ) )		// not a NaN?
						{
							xapp += data[ i ][ j ];
							h++;
						}
						
					if ( h == 0 )
						xapp = NAN;
					else
						xapp /= h;
					
					vs[ num_var + i ].data[ k ] = xapp;
				}
			}
			else					// central moving average
			{
				// average of first period in data
				for ( xapp = 0, h = 0, j = start[ i ]; j < start[ i ] + flt; ++j )
					if ( is_finite( data[ i ][ j ] ) )		// not a NaN?
					{
						xapp += data[ i ][ j ];
						h++;
					}
					
				if ( h == 0 )		// no observation before first?
					xapp = NAN;
				else
					xapp /= h;

				for ( j = start[ i ]; j < start[ i ] + ( flt - 1 ) / 2; ++j )
					vs[ num_var + i ].data[ j ] = xapp;

				for ( ; j < end[ i ] - ( flt - 1 ) / 2; ++j )
				{
					if ( is_finite( data[ i ][ j - ( flt - 1 ) / 2 ] ) && is_finite( data[ i ][ j + ( flt - 1 ) / 2 ] ) )
						xapp = xapp - data[ i ][ j - ( flt - 1 ) / 2 ] / flt + data[ i ][ j + ( flt - 1 ) / 2 ] / flt;
					else
						xapp = NAN;
					
					vs[ num_var + i ].data[ j ] = xapp;
				}
				
				for ( ; j <= end[ i ]; ++j )
					vs[ num_var + i ].data[ j ] = xapp;     
			}
		}
		
		cmd( ".da.vars.lb.v insert end \"%s %s (%d-%d) #%d (created)\"", vs[ num_var + i ].label, vs[ num_var + i ].tag, vs[ num_var + i ].start, vs[ num_var + i ].end, num_var + i ); 

		cmd( "lappend DaModElem %s", vs[ num_var + i ].label );
	}

	cmd( ".da.vars.lb.v see end" );
	num_var += nv; 

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
	}
	
	delete [ ] str;
	delete [ ] tag; 
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/************************
 SAVE_DATAzip
 ************************/
int numcol = 16;

void save_datazip( int *choice )
{
	char *app, **str, **tag, delimiter[ 10 ], misval[ 10 ], labprefix[ MAX_ELEM_LENGTH ];
	const char *descr, *ext;
	double **data;
	int i, j, fr, typelab, del, type_res, *start, *end, *id, headprefix = 0;
	FILE *fsave = NULL;

#ifdef LIBZ
	gzFile fsavez = NULL;
#else
	FILE *fsavez = NULL; 
#endif 

	const char str0[ ] = "00000000000000000000000000000000000000000000000000000000000000000000000000000000";
	const char strsp[ ] = "                                                                                ";
	const char descrRes[ ] = "LSD Result File";
	const char descrTxt[ ] = "Text File";
	const char extResZip[ ] = ".res.gz";
	const char extTxtZip[ ] = ".txt.gz";
	const char extRes[ ] = ".res";
	const char extTxt[ ] = ".txt";

	if ( nv == 0 )			// no variables selected
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return;
	}

	if ( logs )
	  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	max_c = min_c = 0;

	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		
		cmd( "set res [.da.vars.ch.v get %d]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		data[ i ] = vs[ id[ i ] ].data;
		if ( data[ i ] == NULL )
			plog( "\nError: invalid data\n" );
		
		if ( max_c < end[ i ] )
			max_c = end[ i ];
	}

	Tcl_LinkVar( inter, "fr", ( char * ) &fr, TCL_LINK_BOOLEAN);
	Tcl_LinkVar( inter, "dozip", ( char * ) &dozip, TCL_LINK_BOOLEAN);
	Tcl_LinkVar( inter, "typelab", ( char * ) &typelab, TCL_LINK_INT );
	Tcl_LinkVar( inter, "deli", ( char * ) &del, TCL_LINK_INT );
	Tcl_LinkVar( inter, "numcol", ( char * ) &numcol, TCL_LINK_INT );

	strncpy( misval, nonavail, 9 );
	typelab = 3;
	fr = 1;
	del = 1;

	cmd( "newtop .da.lab \"Data Save Options\" { set choice 2 } .da" );

	cmd( "frame .da.lab.f" );
	cmd( "label .da.lab.f.l -text \"File format\"" );

	cmd( "frame .da.lab.f.t -relief groove -bd 2" );
	cmd( "radiobutton .da.lab.f.t.lsd -text \"LSD results file\" -variable typelab -value 3" );
	cmd( "radiobutton .da.lab.f.t.nolsd -text \"Text file\" -variable typelab -value 4" );
	cmd( "pack .da.lab.f.t.lsd .da.lab.f.t.nolsd -anchor w" );

	cmd( "pack .da.lab.f.l .da.lab.f.t" );

	cmd( "checkbutton .da.lab.dozip -text \"Generate zipped file\" -variable dozip" );

#ifdef LIBZ
	cmd( "pack .da.lab.f .da.lab.dozip -padx 5 -pady 5" );
#else
	cmd( "pack .da.lab.f -padx 5 -pady 5" );
#endif

	cmd( "okhelpcancel .da.lab b { set choice 1 } { LsdHelp menudata_res.html#save } { set choice 2 }" );

	cmd( "bind .da.lab <Return> {.da.lab.ok invoke}" );
	cmd( "bind .da.lab <Escape> {.da.lab.esc invoke}" );

	cmd( "showtop .da.lab" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	if ( *choice == 2 )
		goto end;

	*choice = 0;
	type_res = typelab;
	cmd( "destroytop .da.lab" );

	if ( typelab == 4 )
	{
		typelab = 1;
		cmd( "if { ! [ info exists labprefix ] } { set labprefix \"V\" }" );
		cmd( "if { ! [ info exists headprefix ] } { set headprefix 0 }" );
		cmd( "if { ! [ info exists delimiter ] } { set delimiter \"%s\" }", CSV_SEP );
		cmd( "if { ! [ info exists misval ] } { set misval \"%s\" }", nonavail );

		cmd( "newtop .da.lab \"Data Save Options\" { set choice 2 } .da" );

		cmd( "checkbutton .da.lab.fr -text \"Header in first row\" -variable fr -command { if { $fr == 0 } { .da.lab.gp configure -state disabled; .da.lab.f.r.orig configure -state disabled; .da.lab.f.r.new configure -state disabled; .da.lab.d.r.tab configure -state disabled; .da.lab.d.r.oth configure -state disabled; .da.lab.d.r.col configure -state disabled; .da.lab.gen.mis_val configure -state disabled; if { $typelab == 2 } { .da.lab.n.en configure -state disabled }; if { $deli == 2 } { .da.lab.c.del configure -state disabled }; if { $deli == 3 } { .da.lab.e.ecol configure -state disabled } } else { .da.lab.gp configure -state normal; .da.lab.f.r.orig configure -state normal; .da.lab.f.r.new configure -state normal; .da.lab.d.r.tab configure -state normal; .da.lab.d.r.oth configure -state normal; .da.lab.d.r.col configure -state normal; .da.lab.gen.mis_val configure -state normal; if { $typelab == 2 } { .da.lab.n.en configure -state normal }; if { $deli == 2 } { .da.lab.c.del configure -state normal }; if { $deli == 3 } { .da.lab.e.ecol configure -state normal } } }" );
		cmd( "checkbutton .da.lab.gp -text \"Prefix header with #\" -variable headprefix" );

		cmd( "frame .da.lab.f" );
		cmd( "label .da.lab.f.tit -text \"First row labels\"" );

		cmd( "frame .da.lab.f.r -relief groove -bd 2" );
		cmd( "radiobutton .da.lab.f.r.orig -text Original -variable typelab -value 1 -command { .da.lab.n.en configure -state disabled }" );
		cmd( "radiobutton .da.lab.f.r.new -text Numeric -variable typelab -value 2 -command { .da.lab.n.en configure -state normal }" );
		cmd( "pack .da.lab.f.r.orig .da.lab.f.r.new -anchor w" );

		cmd( "pack .da.lab.f.tit .da.lab.f.r" );

		cmd( "frame .da.lab.n" );
		cmd( "label .da.lab.n.l -text \"Numeric labels prefix\"" );
		cmd( "entry .da.lab.n.en -width 10 -justify center -textvariable labprefix -state disabled" );
		cmd( "pack .da.lab.n.l .da.lab.n.en" );

		cmd( "frame .da.lab.d" );
		cmd( "label .da.lab.d.tit -text \"Columns delimiter\"" );

		cmd( "frame .da.lab.d.r -relief groove -bd 2" );
		cmd( "radiobutton .da.lab.d.r.tab -text Tabs -variable deli -value 1 -command { .da.lab.c.del configure -state disabled; .da.lab.e.ecol configure -state disabled }" );
		cmd( "radiobutton .da.lab.d.r.oth -text Custom -variable deli -value 2 -command { .da.lab.c.del configure -state normal; .da.lab.e.ecol configure -state disabled }" );
		cmd( "radiobutton .da.lab.d.r.col -text \"Fixed width\" -variable deli -value 3 -command { .da.lab.c.del configure -state disabled; .da.lab.e.ecol configure -state normal }" );
		cmd( "pack .da.lab.d.r.tab .da.lab.d.r.oth .da.lab.d.r.col -anchor w" );

		cmd( "pack .da.lab.d.tit .da.lab.d.r" );

		cmd( "frame .da.lab.c" );
		cmd( "label .da.lab.c.l -text \"Custom delimiter\"" );
		cmd( "entry .da.lab.c.del -width 3 -textvariable delimiter -justify center -state disabled" );
		cmd( "pack .da.lab.c.l .da.lab.c.del" );

		cmd( "frame .da.lab.e" );
		cmd( "label .da.lab.e.l -text \"Column width (10-80 chars)\"" );
		cmd( "entry .da.lab.e.ecol -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set numcol %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numcol; return 0 } } -invcmd { bell } -justify center -state disabled" );
		cmd( "write_disabled .da.lab.e.ecol $numcol" );
		cmd( "pack .da.lab.e.l .da.lab.e.ecol" );

		cmd( "frame .da.lab.gen" );
		cmd( "label .da.lab.gen.miss -text \"Missing values\"" );
		cmd( "entry .da.lab.gen.mis_val -width 5 -textvariable misval -justify center" );
		cmd( "pack .da.lab.gen.miss .da.lab.gen.mis_val" );

		cmd( "pack .da.lab.fr .da.lab.gp .da.lab.f .da.lab.n .da.lab.d .da.lab.c .da.lab.e .da.lab.gen -padx 5 -pady 5" );

		cmd( "okhelpcancel .da.lab b { set choice 1 } { LsdHelp menudata_res.html#save } { set choice 2 }" );

		cmd( "showtop .da.lab" );

		*choice = 0;
		while ( *choice == 0 )
			Tcl_DoOneEvent( 0 );

		if ( *choice == 2 )
			goto end;

		cmd( "set numcol [ .da.lab.e.ecol get ]" ); 
		cmd( "set choice $headprefix" );
		
		headprefix = *choice;
		app = ( char * ) Tcl_GetVar( inter, "misval", 0 );
		strncpy( misval, app, 9 );

		*choice = 0;
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

	// make sure there is a path set
	cmd( "set path \"%s\"", path );
	if ( strlen( path ) > 0 )
		cmd( "cd \"$path\"" );

	cmd( "set bah [tk_getSaveFile -parent .da -title \"Save Data File\" -initialdir \"$path\" -defaultextension \"%s\" -filetypes {{{%s} {%s}} {{All files} {*}} }]", ext, descr, ext );
	app = ( char * ) Tcl_GetVar( inter, "bah", 0 );
	strcpy( msg, app );

	if ( strlen( msg ) == 0 )
		goto end;

	if ( dozip == 1 ) 
	{
#ifdef LIBZ
		fsavez = gzopen( msg, "wt" );
#endif 
	} 
	else
		fsave = fopen( msg, "wt" );  // use text mode for Windows better compatibility

	if ( del != 3 ) //Delimited files
	{
		if ( del == 2 )
		{
			app = ( char * ) Tcl_GetVar( inter, "delimiter", 0 );
			
			if ( strlen( app ) == 0 )
				strcpy( delimiter, "\t" );
			else
				strncpy( delimiter, app, 9 );
		}
		else
			strcpy( delimiter, "\t" );
	}

	if ( typelab == 2 )
	{
		app = ( char * ) Tcl_GetVar( inter, "labprefix", 0 );
		
		if ( strlen( app ) == 0 )
			strcpy( labprefix, "V" );
		else
			strncpy( labprefix, app, MAX_ELEM_LENGTH - 1 );
	}

	numcol = ( int ) max( 10, min( numcol, 80 ) );

	if ( fr == 1 )
	{
		if ( del != 3 )
		{
			switch ( typelab )
			{
				case 1:   				// Original labels
					if ( headprefix == 1 )
					{
						if ( dozip == 1 ) 
						{
#ifdef LIBZ
							gzprintf( fsavez, "#" );
#endif
						} 
						else
							fprintf( fsave, "#" );
					}  
					 
					for ( i = 0; i < nv; ++i )
					{
						if ( dozip == 1 ) 
						{
#ifdef LIBZ 
							gzprintf( fsavez, "%s_%s", str[ i ], tag[ i ] );
						   
							if ( i < nv - 1 )  
								gzprintf( fsavez, "%s", delimiter );
#endif 
						} 
						else
						{
							fprintf( fsave, "%s_%s", str[ i ], tag[ i ] ); 
						   
							if ( i < nv - 1 )  
								fprintf( fsave, "%s", delimiter );
						} 
					}  
					
					break;

				case 2: 				// New names for labels
					if ( headprefix == 1 )
					{
						if ( dozip == 1 ) 
						{
#ifdef LIBZ
							gzprintf( fsavez, "#" );
#endif
						} 
						else
							fprintf( fsave, "#" );
					}  
				 
					for ( i = 0; i < nv; ++i )
					{
						if ( dozip == 1 ) 
						{
#ifdef LIBZ
							gzprintf( fsavez, "%s%d", labprefix, i );
					   
							if ( i < nv - 1 )  
								gzprintf( fsavez, "%s", delimiter );
#endif
						} 
						else
						{
							fprintf( fsave, "%s%d", labprefix, i );  
						   
							if ( i < nv - 1 )  
								fprintf( fsave, "%s", delimiter );
						}
					}   
					
					break;

				case 3: 				// LSD result files
					for ( i = 0; i < nv; ++i )
					{
						if ( dozip == 1 ) 
						{
#ifdef LIBZ 
							gzprintf( fsavez, "%s %s (%d %d)\t", str[ i ], tag[ i ], start[ i ], end[ i ] );
#endif 
						} 
						else
							fprintf( fsave, "%s %s (%d %d)\t", str[ i ], tag[ i ], start[ i ], end[ i ] );  
					}  
					
					break;
			}
		}
		else 							// header for fixed Column text files
		{
			if ( headprefix == 1 )
			{
				if ( dozip == 1 ) 
				{
#ifdef LIBZ 
					gzprintf( fsavez, "#" );
#endif 
				} 
				else
					fprintf( fsave, "#" );  
			}    

			for ( i = 0; i < nv; ++i )
			{
				if ( typelab == 2 )
					sprintf( msg, "%s%d", labprefix, i + 1 );
				else
					sprintf( msg, "%s_%s", str[ i ], tag[ i ] );
					
				if ( strlen( msg ) < ( unsigned ) numcol )
					strcat( msg, strsp );
				
				if ( i == 0 && headprefix == 1 )
					msg[ numcol - 1 ] = '\0';
				else
					msg[ numcol ] = '\0';
					
				if ( dozip == 1 ) 
				{
#ifdef LIBZ
					gzprintf( fsavez, "%s", msg );
#endif
				} 
				else
					fprintf( fsave, "%s", msg ); 
			}
		}

		if ( dozip == 1 ) 
		{
#ifdef LIBZ 
			gzprintf( fsavez, "\n" );
#endif 
		} 
		else
			fprintf( fsave, "\n" );  
	}

	if ( del != 3 ) 					// data delimited writing
	{
		if ( dozip == 1 )
		{
#ifdef LIBZ
			for ( j = min_c; j <= max_c; ++j )
			{
				for ( i = 0; i < nv; ++i )
				{
					if ( j >= start[ i ] && j <= end[ i ] && ! is_nan( data[ i ][ j ] ) )		// write NaN as n/a
						gzprintf( fsavez, "%.*G", SIG_DIG, data[ i ][ j ] );
					else
						gzprintf( fsavez, "%s", misval );
			  
					if ( typelab == 3 || i < nv - 1 )  
						gzprintf( fsavez, "%s", delimiter );
				}
				
				gzprintf( fsavez, "\n" );
			}
#endif 
		}
		else
		{
			for ( j = min_c; j <= max_c; ++j )
			{
				for ( i = 0; i < nv; ++i )
				{
					if ( j >= start[ i ] && j <= end[ i ] && ! is_nan( data[ i ][ j ] ) )		// write NaN as n/a
						fprintf( fsave, "%.*G", SIG_DIG, data[ i ][ j ] );  
					else
						fprintf( fsave, "%s", misval );  
			  
					if ( typelab == 3 || i < nv - 1 )  
						fprintf( fsave, "%s", delimiter );
				}
			   
				fprintf( fsave, "\n" );  
			}
		}
	}
	else 								//fixed column data writing
	{
		for ( j = min_c; j <= max_c; ++j )
		{
			for ( i = 0; i < nv; ++i )
			{
				if ( j >= start[ i ] && j <= end[ i ] && ! is_nan( data[ i ][ j ] ) )		// write NaN as n/a
				{
					sprintf( msg, "%.*G", ( int ) min( numcol - 6, SIG_DIG ), data[ i ][ j ] );
					strcat( msg, str0 );
					msg[ numcol ] = '\0';
				}
				else
				{
					sprintf( msg, "%s", misval );
					strcat( msg, strsp );
					msg[ numcol ] = '\0';
				}
				
				if ( dozip == 1 ) 
				{
#ifdef LIBZ
					gzprintf( fsavez, "%s", msg );
#endif 
				}
				else
					fprintf( fsave, "%s", msg );  
			}
			
			if ( dozip == 1 ) 
			{
#ifdef LIBZ
				gzprintf( fsavez, "\n" );
#endif
			} 
			else
				fprintf( fsave, "\n" );  
		}
	}

	if ( dozip == 1 ) 
	{
#ifdef LIBZ
		gzclose( fsavez);
#endif
	}
	else
		fclose( fsave );  

	end:

	cmd( "destroytop .da.lab" );
	Tcl_UnlinkVar( inter, "typelab" );
	Tcl_UnlinkVar( inter, "dozip" );
	Tcl_UnlinkVar( inter, "numcol" );
	Tcl_UnlinkVar( inter, "deli" );
	Tcl_UnlinkVar( inter, "fr" );
	
	*choice = 0;

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/************************
 PLOG_SERIES
 ************************/
void plog_series( int *choice )
{
	char *lapp, **str, **tag;
	double **data;
	int i, j, *start, *end, *id;
	store *app;

	if ( nv == 0 )			// no variables selected
	{
		cmd( "tk_messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return;
	}

	if ( logs )
	  cmd( "tk_messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	app = new store[ 1 + num_var ];

	for ( i = 0; i < num_var; ++i )
	{
		app[ i ] = vs[ i ];
		strcpy( app[ i ].label, vs[ i ].label );
		strcpy( app[ i ].tag, vs[ i ].tag);
	}
	 
	delete [ ] vs;
	vs = app;

	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		
		cmd( "set res [.da.vars.ch.v get %d]", i );
		lapp = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg,lapp );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
				plog( "\nError: invalid data\n" );
		}
	}

	if ( autom_x || min_c >= max_c )
	{
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = start[ i ];
			
			if ( start[ i ] < min_c )
				min_c = start[ i ];
			
			if ( end[ i ] > max_c )
			max_c = end[ i ] > num_c ? num_c : end[ i ];
		}
	}

	plog( "\n\nTime series data\n" );
	plog( "t\t%s_%s", "series", str[ 0 ], tag[ 0 ] );

	for ( i = 1; i < nv; ++i )
		plog( "\t%s_%s", "series", str[ i ], tag[ i ] );

	plog( "\n" );

	for ( i = min_c; i <= max_c; ++i )
	{
		if ( start[ 0 ] <= i && end[ 0 ] >= i && ! is_nan( data[ 0 ][ i ] ) )
			plog( "%d\t%.*g", "series", i, pdigits, data[ 0 ][ i ] );
		else
			plog( "%d\t%s", "series", i, nonavail );		// write NaN as n/a
		
		for ( j = 1; j < nv; ++j )
		{
			if ( start[ j ] <= i && end[ j ] >= i && ! is_nan( data[ j ][ i ] ) )
				plog( "\t%.*g", "series", pdigits, data[ j ][ i ] );
			else
				plog( "\t%s", "series", nonavail );		// write NaN as n/a
		}
		
		plog( "\n" );
	}

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
	}

	delete [ ] str;
	delete [ ] tag; 
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}    


 /*****************************************
 PLOT (curves)
 Effectively create the plot in canvas
 *****************************************/
void plot( int type, int nv, double **data, int *start, int *end, int *id, char **str, char **tag, int *choice )
{
	int h, i, j, k, color, hsize, vsize, hbordsize, tbordsize, lheight, hcanvas, vcanvas, nLine, endCase, iniCase;
	double x1, x2, *y, yVal, cminy, cmaxy, step;
	char txtLab[ 2 * MAX_ELEM_LENGTH ];
	bool tOk = false, y2on = ( type == TSERIES ) ? ( num_y2 <= nv ? true : false ) : false;
	
	// define type-specific parameters
	switch ( type )
	{
		case TSERIES:
			nLine = nv;
			iniCase = min_c;
			endCase = max_c;
			break;
			
		case CRSSECT:
			nLine = *end;
			iniCase = 1;
			endCase = nv - 1;
			break;
			
		default:
			return;					// invalid type, do nothing
	}
	
	// get graphical configuration from Tk ( file defaults.tcl )
	get_int( "hsizeP", & hsize );			// 600
	get_int( "vsizeP", & vsize );			// 300
	get_int( "tbordsizeP", & tbordsize );	// 5

	// select gray scale or color range				
	color = allblack ? 1001 : 0;
	
	// alert once about plotting over more timesteps than plot window pixels
	step = hsize / ( double ) ( endCase - iniCase );
	if ( avgSmpl && ! avgSmplMsg && step < 1 )
	{
		if ( type == TSERIES )
			cmd( "set answer [ tk_messageBox -parent .da -title Warning -icon warning -type yesno -default yes -message \"Disable Y values averaging?\" -detail \"The number of time steps to plot is larger than the physical plot width. To compute the Y values, LSD averages data from multiple time steps.\n\nPress 'Yes' to disable Y values averaging or 'No' otherwise\n(this configuration can be also changed in menu 'Options').\"]" );
		else
			cmd( "set answer [ tk_messageBox -parent .da -title Warning -icon warning -type yesno -default yes -message \"Disable series values averaging?\" -detail \"The number of series to plot is larger than the physical plot width. To compute the presented values, LSD averages data from multiple series.\n\nPress 'Yes' to disable Y values averaging or 'No' otherwise\n(this configuration can be also changed in menu 'Options').\"]" );
			
		cmd( "switch $answer { yes { set avgSmpl 0 } no { } }" );
		avgSmplMsg = true;
	}

	// create data structure to hold image to be presented
	y = new double[ nLine ];		// cross section consolidated "intra" step average value
	int pdataX[ hsize + 1 ];		// visible time step values ( max)
	for ( i = 0; i < hsize + 1; ++i )
		pdataX[ i ] = -1;			// mark all as non plot
	int **pdataY = new int *[ nLine ];	// matrix of visual average scaled/rounded values
	for ( k = 0; k < nLine; ++k )
	{
		pdataY[ k ] = new int [ hsize + 1 ];
		for ( i = 0; i < hsize + 1; ++i )
			pdataY[ k ][ i ] = -1;	// mark all as non plot
	}
	
	// create the window and the canvas
	plot_canvas( type, nv, start, end, str, tag, choice );
	
	// get graphical configuration real canvas
	get_int( "hcanvasP", & hcanvas );
	get_int( "vcanvasP", & vcanvas );
	get_int( "hbordsizeP", & hbordsize );
	get_int( "lheightP", & lheight );

	// calculate screen plot values for all series
	x1 = hbordsize - 1;
	for ( h = 0, j = 0, i = iniCase; i <= endCase; ++i )
	{
		// move the x-axis pointer in discrete steps
		x2 = x1 + ( 1 + h ) * step;
		++h;            	// counter for the average when many values occupy one x point
		
		// fix initial x point if scale is too coarse
		if ( i == iniCase && x2 - x1 > 1 )
			x2 = x1 + 1;
		
		// moved to another canvas step?
		bool xnext = ( floor( x1 ) != floor ( x2 ) );

		// process each curve (series or cross-section)
		for ( k = 0; k < nLine; ++k, ++color )	
		{		
			yVal = NAN;
			
			switch ( type )
			{
				case TSERIES:
					// ignore series not started/finished
					if ( data[ k ] == NULL )
						continue;
					
					if ( i > 1 && start[ k ] < i && end[ k ] >= i )
					{
						yVal = data[ k ][ i ];
						tOk = true;
					}
					else
					{
						if ( start[ k ] == i || ( i <= 1 && start[ k ] <= 0 ) )
							yVal = data[ k ][ i ];
						
						tOk = false;
					}
					
					break;
					
				case CRSSECT:
					// ignore series not started/finished
					if ( data[ i ] == NULL )
						continue;
					
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
					{	
						if ( avgSmpl )					// average "intra" steps ?
							y[ k ] /= h;
						else
							y[ k ] = yVal;
							
						// constrain to canvas virtual limits
						y[ k ] = min( max( y[ k ], cminy ), cmaxy );
						// scale to the canvas physical y range
						y[ k ] = round( tbordsize + vsize * ( 1 - ( y[ k ] - cminy ) / ( cmaxy - cminy ) ) );
						// save to visual vertical line buffer
						pdataY[ k ][ j ] = ( int ) round( y[ k ] );
						// restart averaging
						y[ k ] = 0;
					}
				}
				else
				{
					if ( start[ k ] == i || ( i <= 1 && start[ k ] <= 0 ) )
					{ 	// series entering after x1
						if ( ! xnext )					// "intra" step?
							y[ k ] = yVal * h;			// suppose from the beginning of x1
						else
						{	// just plot as usual
							y[ k ] = yVal;
							y[ k ] = min( max( y[ k ], cminy ), cmaxy );
							y[ k ] = round( tbordsize + vsize * ( 1 - ( y[ k ] - cminy ) / ( cmaxy - cminy ) ) );
							pdataY[ k ][ j ] = ( int ) round( y[ k ] );
							y[ k ] = 0;
						}
					}
				}
			}
		}
		
		if ( xnext )
		{ 
			x1 = x2;
			h = 0;			// restart averaging ( more than 1 step per canvas step )
			pdataX[ j ] = ( int ) floor( x2 );
			if ( ++j > hsize )	// buffer full?
				break;
		}
	}

	// transfer data to Tcl and plot it
	cdata = pdataX;					// send series x values to Tcl
	cmd( "get_series %d pdataX", j );
	
	color = allblack ? 1001 : 0;	// select gray scale or color range
		
	for ( *choice = k = 0; k < nLine; ++k, ++color )
	{
		cdata = pdataY[ k ];		// send series y values to Tcl
		cmd( "unset -nocomplain pdataY" );
		cmd( "get_series %d pdataY", j );
		
		int colorPlot = ( color < 1100 ) ? color : 0;	// use black if out of colors

		if ( line_point == 1 )
			cmd( "plot_line $p $pdataX $pdataY p%d $c%d %lf", k, colorPlot, point_size );
		else
			cmd( "plot_points $p $pdataX $pdataY p%d $c%d %lf", k, colorPlot, point_size );
		
		if ( watch )
			cmd( "update" );
		if	( *choice == 2 ) 		// button STOP pressed
			break;  
	}
	
	// garbage collection
	cmd( "unset pdataX pdataY" );
	for ( i = 0; i < nLine; ++i )
		delete [ ] pdataY[ i ];
	delete [ ] pdataY;
	delete [ ] y;
	
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
					$w.b.c.case.v configure -text [ expr int( ( $cx - $llim ) * ( %d - %d) / ( $rlim - $llim ) + %d) ]; \
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
					set case  [ expr int( ( $cx - $llim ) * ( %d - %d) / ( $rlim - $llim ) + %d) ]; \
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
				sprintf( txtLab, "%s_%s (#%d)", str[ i ], tag[ i ], id[ i ] );
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

	// window bindings (return to Analysis, insert text, insert line )
	cmd( "bind $w <Escape> \"wm withdraw $w\"" );
	cmd( "bind $w <s> { $w.b.s.save invoke }; bind $w <S> { $w.b.s.save invoke }" );
	cmd( "bind $w <plus> { $w.b.z.b.p invoke }" );
	cmd( "bind $w <minus> { $w.b.z.b.m invoke }" );
	cmd( "bind $p <Double-Button-1> { wm deiconify .da; raise .da; focus .da }" );
	
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
	plot_l[ cur_plot ] = vcanvas; 					// height of plot with labels

	*choice = 0;
}


 /*****************************************
 PLOT (histogram)
 Effectively create the histogram in canvas
 *****************************************/
void plot( int type, int *start, int *end, char **str, char **tag, int *choice, bool norm )
{
	int i, x1, x2, y1, y2, color, hsize, vsize, hbordsize, tbordsize, lheight, hcanvas, vcanvas;
	char txtLab[ 2 * MAX_ELEM_LENGTH ];
	
	if ( type != HISTOGR && type != HISTOCS )
		return;								// invalid type, do nothing
	
	// create the window and the canvas
	plot_canvas( type, 1, start, end, str, tag, choice );
	
	// get graphical configuration from Tk ( file defaults.tcl )
	get_int( "hsizeP", & hsize );			// 600
	get_int( "vsizeP", & vsize );			// 300
	get_int( "hcanvasP", & hcanvas );
	get_int( "vcanvasP", & vcanvas );
	get_int( "tbordsizeP", & tbordsize );	// 5
	get_int( "hbordsizeP", & hbordsize );
	get_int( "lheightP", & lheight );		// 15

	// select gray or color			
	color = allblack ? 1001 : 0;	

	// plot histogram
	*choice = 0;
	for ( i = 0; i < num_bins; ++i )
	{
		if ( line_point == 1 )
		{
			x1 = hbordsize + ( int ) floor( hsize * ( bins[ i ].lowb - bins[ 0 ].lowb ) / ( bins[ num_bins - 1 ].highb - bins[ 0 ].lowb ) );
			x2 = hbordsize + ( int ) floor( hsize * ( bins[ i ].highb - bins[ 0 ].lowb ) / ( bins[ num_bins - 1 ].highb - bins[ 0 ].lowb ) );
			y1 = ( int ) min( max( tbordsize + vsize - floor( vsize * ( bins[ i ].num / cases - miny ) / ( maxy - miny ) ), tbordsize ), tbordsize + vsize );
			y2 = tbordsize + vsize;

			cmd( "plot_bars $p %d %d %d %d p%d $c%d %lf", x1, y1, x2, y2, i, color + 1, point_size );
		}
		else
		{
			x1 = hbordsize + ( int ) floor( hsize * ( bins[ i ].center - bins[ 0 ].lowb ) / ( bins[ num_bins - 1 ].highb - bins[ 0 ].lowb ) );
			y1 = tbordsize + vsize - ( int ) floor( vsize * ( bins[ i ].num / cases - miny ) / ( maxy - miny ) );
			if ( y1 <= tbordsize + vsize && y1 >= tbordsize )
				cmd( "plot_points $p %d %d p%d $c%d %lf", x1, y1, i, color, point_size );
		}
			
		if ( watch )
			cmd( "update" );
		if ( *choice == 2 ) 		// button STOP pressed
			break;  
	}

	if	( *choice == 2 ) 			// Button STOP pressed
	{
		cmd( "destroytop $w" );
		return;
	}

	if ( norm && var > 0 )
	{
		double a, b, s, tot_norm = 0;
		
		for ( i = 0; i < num_bins; ++i )
		{
			a = bins[ i ].lowb;
			b = exp( - ( a - mean ) * ( a - mean ) / ( 2 * var ) ) / 
				( sqrt( 2 * M_PI * var ) );
			a = bins[ i ].highb;
			s = exp( - ( a - mean ) * ( a - mean ) / ( 2 * var ) ) / 
				 ( sqrt( 2 * M_PI * var ) );
			tot_norm += ( b + s ) / 2;
		}

		for ( i = 0; i < num_bins; ++i )
		{
			a = bins[ i ].center;  
			b = exp( - ( a - mean ) * ( a - mean ) / ( 2 * var ) ) / 
				( sqrt( 2 * M_PI * var ) );
			b /= tot_norm;
			y2 = ( int ) min( max( tbordsize + vsize - round( vsize * ( b - miny ) / ( maxy - miny ) ), 
								   tbordsize ), 
							  tbordsize + vsize );  

			x2 = hbordsize + ( int ) round( hsize * ( bins[ i ].center - bins[ 0 ].lowb ) / 
											( bins[ num_bins - 1 ].highb - bins[ 0 ].lowb ) );
									
			if ( i > 0 && ( y1 > tbordsize || y2 > tbordsize ) && 
				 ( y1 < tbordsize + vsize || y2 < tbordsize + vsize ) )
				cmd( "plot_line $p { %d %d } { %d %d } p%d $c%d %lf", 
					 x1, x2, y1, y2, num_bins, color + 2, point_size );
			
			x1 = x2;
			y1 = y2;
		}
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
				$w.b.c.case.v configure -text [ expr min( 1 + int( ( $cx - $llim ) * %d / ( $rlim - $llim ) ), %d) ]; \
				$w.b.c.y.v1 configure -text [ format \"%%%%.[ expr $pdigits ]g\" [ expr ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf ] ]; \
				$w.b.c.y.v2 configure -text \"( n=[ expr int( ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf ) ] )\" \
			} \
		}", cur_plot, cur_plot, hbordsize, hbordsize + hsize, tbordsize, tbordsize + vsize, num_bins, num_bins, maxy, miny, miny, maxy * cases, miny * cases, miny * cases );

	for ( i = 0; i < num_bins; ++i )
	{
		switch ( type )
		{
			case HISTOGR:
				sprintf( txtLab, "n=%d \u03bc=%.*g \\[%.*g,%.*g\\]", ( int ) bins[ i ].num, pdigits, bins[ i ].av, pdigits, bins[ i ].min, pdigits, bins[ i ].max );
				break;
				
			case HISTOCS:
				sprintf( txtLab, "n=%d \u03bc=%.*g \\[%.*g,%.*g\\]", ( int ) bins[ i ].num, pdigits, bins[ i ].av, pdigits, bins[ i ].min, pdigits, bins[ i ].max );
				break;
		}
			
		cmd( "$p bind p%d <Enter> { .da.f.new%d.b.c.var.v configure -text \"%s\" }", i, cur_plot, txtLab );
		cmd( "$p bind p%d <Leave> { .da.f.new%d.b.c.var.v configure -text \"\" }", i, cur_plot );

		cmd( "$p bind txt%d <Enter> { .da.f.new%d.b.c.var.v configure -text \"%s\" }", i,cur_plot, txtLab );
		cmd( "$p bind txt%d <Leave> { .da.f.new%d.b.c.var.v configure -text \"\" }", i, cur_plot );
	}

	// window bindings (return to Analysis, insert text, insert line )
	cmd( "bind $w <Escape> \"wm withdraw $w\"" );
	cmd( "bind $w <s> { $w.b.s.save invoke }; bind $w <S> { $w.b.s.save invoke }" );
	cmd( "bind $w <plus> { $w.b.z.b.p invoke }" );
	cmd( "bind $w <minus> { $w.b.z.b.m invoke }" );
	cmd( "bind $p <Double-Button-1> { wm deiconify .da; raise .da; focus .da }" );
	
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
			} elseif { [ lsearch $type bar ] >= 0 } { \
				set choice 42 \
			} elseif { [ lsearch $type text ] >= 0 } { \
				set choice 26 \
			} \
		}", cur_plot );
	cmd( "bind $p <Button-3> { event generate .da.f.new%d.f.plots <Button-2> -x %%x -y %%y }", cur_plot );

	// save plot info
	type_plot[ cur_plot ] = type; 					// plot type
	plot_w[ cur_plot ] = hcanvas;					// plot width
	plot_nl[ cur_plot ] = tbordsize + vsize + lheight; // height of plot  
	plot_l[ cur_plot ] = vcanvas; 					// height of plot with labels

	*choice = 0;
}


/*****************************
 PLOT_CANVAS
 create the plot window & canvas 
 *****************************/
void plot_canvas( int type, int nv, int *start, int *end, char **str, char **tag, int *choice )
{
	bool tOk, y2on;
	char *txtValue, *txtCase, *txtLine, txtLab[ 2 * MAX_ELEM_LENGTH ];
	int h, i, color, hsize, vsize, hbordsize, tbordsize, bbordsize, sbordsize, htmargin, vtmargin, hticks, vticks, lheight, hcanvas, vcanvas, nLine;
	double yVal, cminy2, cmaxy2;
	
	// get graphical configuration from Tk ( file defaults.tcl )
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

	// define type-specific parameters
	switch ( type )
	{
		case TSERIES:
			txtCase = ( char * ) "Case number";
			txtValue = ( char * ) ( logs ? "log(Y) value" : "Y value" );
			txtLine = ( char * ) "Series";
			nLine = nv;
			y2on = num_y2 <= nv ? true : false;
			cminy2 = miny2;
			cmaxy2 = maxy2;
			break;
			
		case CRSSECT:
			txtCase = ( char * ) "Series";
			txtValue = ( char * ) ( logs ? "log(Y) value" : "Y value" );
			txtLine = ( char * ) "Cross-section";
			nLine = *end;
			bbordsize = nLine < 10 ? 4 * lheight : bbordsize;
			y2on = false;
			cminy2 = miny2;
			cmaxy2 = maxy2;
			break;
			
		case HISTOGR:
			txtCase = ( char * ) "Bin number";
			txtValue = ( char * ) "Frequency";
			txtLine = ( char * ) ( logs ? "Bin log data" : "Bin data" );
			nLine = 0;
			bbordsize = 2 * lheight;
			y2on = true;
			cminy2 = miny * cases;
			cmaxy2 = maxy * cases;
			break;
			
		case HISTOCS:
			txtCase = ( char * ) "Bin number";
			txtValue = ( char * ) "Frequency";
			txtLine = ( char * ) ( logs ? "Bin log data" : "Bin data" );
			nLine = 1;
			bbordsize = 4 * lheight;
			y2on = true;
			cminy2 = miny * cases;
			cmaxy2 = maxy * cases;
			break;
			
		default:
			return;							// invalid type, do nothing
	}
	
	// calculate horizontal borders required for legends
	h = min_hborder( choice, pdigits, miny, maxy );
	if ( y2on )								// repeat for 2nd y axis
		h = ( int ) max( h, min_hborder( choice, pdigits, cminy2, cmaxy2 ) );
		
	// include margins and tick size ( if present)
	hbordsize = ( int ) max( hbordsize, h ) + 2 * htmargin + 5;
	cmd( "set hbordsizeP %d", hbordsize );
	
	// initial canvas size
	hcanvas = hsize + hbordsize * ( y2on ? 2 : 1 ) + 
				( y2on ? 0 : ( max_c < 1000 ? 10 : 20 ) );
	vcanvas = vsize + tbordsize + bbordsize;
	cmd( "set hcanvasP %d; set vcanvasP %d", hcanvas, vcanvas );
	
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
	
	// adjust horizontal text space usage
	if ( y2on )							
		cmd( "set datWid 26; if { $tcl_platform(os) == \"Windows NT\" } { set pad1 6; set pad2 10 } { set pad1 0; set pad2 5 }" );
	else
		cmd( "set datWid 20; if { $tcl_platform(os) == \"Windows NT\" } { set pad1 6; set pad2 10 } { set pad1 0; set pad2 5 }" );
	
	// adjust vertical text adjustment
	cmd( "if [ string equal $tcl_platform(platform) unix ] { if [ string equal $tcl_platform(os) Darwin ] { set pad3 3 } { set pad3 0 } } { set pad3 -3 }" );
	
	cmd( "frame $w.b.c.case" );
	cmd( "label $w.b.c.case.l -text \"%s:\" -width 11 -anchor e", txtCase );
	cmd( "label $w.b.c.case.v -text \"\" -fg red -width $datWid -anchor w" );
	cmd( "pack $w.b.c.case.l $w.b.c.case.v -side left -anchor w" );

	cmd( "frame $w.b.c.y" );
	cmd( "label $w.b.c.y.l -text \"%s:\" -width 11 -anchor e", txtValue );
	cmd( "label $w.b.c.y.v1 -text \"\" -fg red -width [ expr $datWid / 2 ] -anchor w" );
	cmd( "label $w.b.c.y.v2 -text \"\" -fg red -width [ expr $datWid / 2 ] -anchor w" );
	cmd( "pack $w.b.c.y.l $w.b.c.y.v1 $w.b.c.y.v2 -side left -anchor w" );

	cmd( "frame $w.b.c.var" );
	cmd( "label $w.b.c.var.l -text \"%s:\" -width 11 -anchor e", txtLine );
	cmd( "label $w.b.c.var.v -text \"\" -fg red -width $datWid -anchor w" );
	cmd( "pack $w.b.c.var.l $w.b.c.var.v -side left -anchor w" );

	cmd( "pack $w.b.c.case $w.b.c.y $w.b.c.var -anchor w" );

	cmd( "frame $w.b.o" );
	cmd( "label $w.b.o.l1 -text \"Right-click: edit properties\"" );
	cmd( "label $w.b.o.l2 -text \"Shift-click: insert text\"" );
	cmd( "label $w.b.o.l3 -text \"Ctrl-click: insert line\"" );
	cmd( "pack $w.b.o.l1 $w.b.o.l2 $w.b.o.l3" );

	cmd( "frame $w.b.s" );
	cmd( "button $w.b.s.save -width $butWid -text Save -command { set it \"$cur_plot) $tit\"; set fromPlot 1; set choice 11 } -state disabled -underline 0" );
	cmd( "button $w.b.s.stop -width $butWid -text Stop -command { set choice 2 } -state disabled -underline 0" );
	cmd( "pack $w.b.s.save $w.b.s.stop -pady 5" );

	cmd( "frame $w.b.z" );
	cmd( "label $w.b.z.l -text Zoom" );

	cmd( "frame $w.b.z.b" );
	cmd( "button $w.b.z.b.p -width 3 -text + -command { scale_canvas .da.f.new%d.f.plots \"+\" zoomLevel%d } -state disabled", cur_plot, cur_plot );
	cmd( "button $w.b.z.b.m -width 3 -text - -command { scale_canvas .da.f.new%d.f.plots \"-\" zoomLevel%d } -state disabled", cur_plot, cur_plot  );
	cmd( "pack $w.b.z.b.p $w.b.z.b.m" );

	cmd( "pack  $w.b.z.l $w.b.z.b -side left -padx 2 -pady 2" );

	cmd( "label $w.b.pad -width $pad1" );

	cmd( "pack $w.b.c $w.b.o $w.b.pad $w.b.s $w.b.z -expand no -padx $pad2 -pady 5 -side left" );
	cmd( "pack $w.b -side right -expand no" );

	cmd( "mouse_wheel $p" );
	
	if ( watch )
	{	
		cmd( "$w.b.s.stop configure -state normal" );
		cmd( "bind $w <s> { $w.b.s.stop invoke }; bind $w <S> { $w.b.s.stop invoke }" );
		cmd( "bind $w <Escape> { $w.b.s.stop invoke }" );
	}

	cmd( "showtop $w current yes yes no" );
	
	// hack to bring the new plot to the foreground during debugging in macOS
	cmd( "if { $running && [ string equal [ tk windowingsystem ] aqua ] } { \
			wm transient $w .da; \
			wm transient $w \
		}" );
	
	cmd( "$p xview moveto 0; $p yview moveto 0" );
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
				cmd( "$p create text %d [ expr %d + $pad3 ] -font $fontP -anchor n -text %d -tag { p text }", hbordsize + ( int ) round( i * ( double ) hsize / ( hticks + 1 ) ), vsize + lheight, min_c + ( int ) floor( i * ( double ) ( max_c - min_c ) / ( hticks + 1 ) ) );
			break;
			
		case CRSSECT:
			cmd( "$p create text %d [ expr %d + $pad3 ] -font $fontP -anchor nw -text \"%d series\" -tag { p text }", hbordsize, vsize + lheight, nv );
			break;
			
		case HISTOGR:
		case HISTOCS:
			for ( i = 0; i < hticks + 2; ++i )
				cmd( "$p create text %d [ expr %d + $pad3 ] -font $fontP -anchor n -text %.*g -tag { p text }", hbordsize + ( int ) round( i * ( double ) hsize / ( hticks + 1 ) ), vsize + lheight, pdigits, bins[ 0 ].lowb + i * ( bins[ num_bins - 1 ].highb - bins[ 0 ].lowb ) / ( hticks + 1 ) );
			break;			
	}

	// y-axis values
	for ( i = 0; i < vticks + 2; ++i )
	{
		yVal = miny + ( vticks + 1 - i ) * ( maxy - miny ) / ( vticks + 1 );
		yVal = ( fabs( yVal ) < ( maxy - miny ) * MARG ) ? 0 : yVal;
		
		cmd( "$p create text %d %d -font $fontP -anchor e -text %.*g -tag { p text }", hbordsize - htmargin - 5, tbordsize + ( int ) round( i * ( double ) vsize / ( vticks + 1 ) ), pdigits, yVal );
		
		// second y-axis series values ( if any )
		if ( y2on )
		{
			yVal = cminy2 + ( vticks + 1 - i ) * ( cmaxy2 - cminy2 ) / ( vticks + 1 );
			yVal = ( fabs( yVal ) < ( cmaxy2 - cminy2 ) * MARG ) ? 0 : yVal;
		
			cmd( "$p create text %d %d -font $fontP -anchor w -text %.*g -tag { p text }", hbordsize + hsize + htmargin + 5, tbordsize + ( int ) round( i * ( double ) vsize / ( vticks + 1 ) ), pdigits, yVal );
		}
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
				
			case HISTOCS:
				sprintf( txtLab, "t = %d ", time_cs );
				tOk = true;
				break;
				
			default:
				tOk = false;			// don't print anything 
				break;
		}
		
		if ( tOk )
		{
			cmd( "set app [ font measure $fontP \"%s\"]", txtLab );
			cmd( "if { [ expr $xlabel + $app] > %d } { set xlabel %d; incr ylabel %d }", hcanvas - htmargin, htmargin, lheight );
			get_int( "ylabel", & h );
			if ( h > tbordsize + vsize + bbordsize - 2 * lheight )
				break;
			cmd( "$p create text $xlabel $ylabel -font $fontP -anchor nw -text \"%s\" -tag { txt%d text legend } -fill $c%d", txtLab, i, ( color < 1100 ) ? color : 0 );
			cmd( "set xlabel [ expr $xlabel + $app + $htmarginP ]" );
		}
	}

	if ( i < nLine )
		cmd( "$p create text $xlabel $ylabel -font $fontP -anchor nw -text \"(%d more...)\"", nLine - i );

	cmd( "update" );	
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

 
/*****************************
 TCL_UPLOAD_SERIES
 data transfer routine from C to Tcl 
 *****************************/
int Tcl_upload_series( ClientData cd, Tcl_Interp *inter, int oc, Tcl_Obj *CONST ov[ ] )
{
	int size, *data;

	if ( oc != 3 ) 
	{
		Tcl_WrongNumArgs( inter, 1, ov, "size data" );
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


/***************************************************
SHRINK_GNUFILE
Prepare gnuplot file
***************************************************/
struct s
{
	int x;
	struct s *son;
	struct s *next;
} d;

int store( struct s *c, int x4 )
{
	struct s *app, *prev;

	for ( prev = NULL, app = c; app != NULL; app = app->next )
		if ( app->x == x4 )
			return 0;
		else
			prev = app; 
	
	if ( app == NULL && prev != NULL )
	{
		prev->next = new struct s;
		app = prev->next;
		app->x = x4;
		app->next = NULL;
		app->son = NULL;

		return 1;
	}

	error_hard( "invalid data structure",
				"internal problem in LSD", 
				"if error persists, please contact developers",
				true );
	myexit( 14 );
	
	return 0;
}

int store( struct s *c, int x3, int x4 )
{
	struct s *app, *prev;

	for ( prev = NULL, app = c; app != NULL; app = app->next )
		if ( app->x == x3 )
			return store( app->son, x4 );
		else
			prev = app;
	
	if ( app == NULL && prev != NULL )
	{
		prev->next = new struct s;
		app = prev->next;
		app->x = x3;
		app->next = NULL;
		
		app->son = new struct s;
		app = app->son;
		app->x = x4;
		app->next = NULL;
		app->son = NULL;
	
		return 1;
	 }

	error_hard( "invalid data structure",
				"internal problem in LSD", 
				"if error persists, please contact developers",
				true );
	myexit( 15 );
	
	return 0;
}

int store( struct s *c, int x2, int x3, int x4 )
{
	struct s *app, *prev;

	for ( prev = NULL, app = c; app != NULL; app = app->next )
		if ( app->x == x2 )
			return store( app->son, x3, x4 );
		else
			prev = app;
		
	if ( app == NULL && prev != NULL )
	{
		prev->next = new struct s;
		app = prev->next;
		app->x = x2;
		app->next = NULL;
		
		app->son = new struct s;
		app = app->son;
		app->x = x3;
		app->next = NULL;
		
		app->son = new struct s;
		app = app->son;
		app->x = x4;
		app->next = NULL;
		app->son = NULL;
	
		return 1;
	}

	error_hard( "invalid data structure",
				"internal problem in LSD", 
				"if error persists, please contact developers",
				true );
	myexit( 16 );
	
	return 0;
}

int store( int x1, int x2, int x3, int x4 )
{
	struct s *app, *prev;

	for ( prev = NULL, app = &d; app != NULL; app = app->next )
		if ( app->x == x1 )
			return store( app->son, x2, x3, x4 );
		else
			prev = app;
	
	if ( app == NULL && prev != NULL )
	{
		prev->next = new struct s;
		app = prev->next;
		app->x = x1;
		app->next = NULL;
		
		app->son = new struct s;
		app = app->son;
		app->x = x2;
		app->next = NULL;
		
		app->son = new struct s;
		app = app->son;
		app->x = x3;
		app->next = NULL;
		
		app->son = new struct s;
		app = app->son;
		app->x = x4;
		app->next = NULL;
		app->son = NULL;
		
		return 1;
	}

	error_hard( "invalid data structure", 
				"internal problem in LSD", 
				"if error persists, please contact developers",
				true );
	myexit( 17 );
	
	return 0;
}

void free_storage( struct s *c )
{
	if ( c->next != NULL )
		free_storage( c->next );
	
	if ( c->son != NULL )
		free_storage( c->son );

	delete c;
}

int shrink_gnufile( void ) 
{
	char str[ 2 * MAX_ELEM_LENGTH ], str1[ 2 * MAX_ELEM_LENGTH ], str2[ 2 * MAX_ELEM_LENGTH ], str3[ 2 * MAX_ELEM_LENGTH ], str4[ 2 * MAX_ELEM_LENGTH ];

	int x1, x2, x3, x4, i, j, h = 0, count = 0;
	FILE *f, *f1;

	d.son = NULL;
	d.next = NULL;
	d.x = -1;

	// wait some time for the file to be ready in macOS
	while ( ( f = fopen( "plot.file", "r" ) ) == NULL && count++ < 10 )
		msleep( 1000 );
	
	if ( f == NULL )
	{
		cmd( "tk_messageBox -parent .da -title Error -icon error -type ok -message \"Cannot open plot\" -detail \"The plot file produced by Gnuplot is not available.\nPlease check if you have selected an adequate configuration for the plot.\nIf the error persists please check if Gnuplot is installed and set up properly.\"" );
		return 1;
	}

	f1 = fopen( "plot_clean.file", "w" );
	if ( f == NULL )
	{
		cmd( "tk_messageBox -parent .da -title Error -icon error -type ok -message \"Cannot create plot file\" -detail \"Please check if the drive or the directory is not set READ-ONLY or full\nand try again\"" );
		return 2;
	}

	while ( fgets( str, 2 * MAX_ELEM_LENGTH, f ) != NULL )
	{
		if ( h++ == 1 )
		fprintf( f1, "set font \"{$::fontP}\"\n" );
		sscanf( str, "%s %s", str1, str2 );
		if ( ! strcmp( str1, "$can" ) && ! strcmp( str2, "create" ) )
		{
			i = strcspn( str, "[" );
			j = strcspn( str, "]" );
			strncpy( str1, str + i, j - i + 1 );
			str1[ j - i + 1 ] = '\0';
			sscanf( str1, "[expr $cmx * %d /1000]", &x1 );
		
			i = strcspn( str + j + 1, "[" );
			i += j + 1;
			j = strcspn( str + i + 1, "]" );
			j += i + 1;
			strncpy( str2, str + i, j - i + 1 );
			str2[ j - i + 1 ]='\0';
			sscanf( str2, "[expr $cmy * %d /1000]", &x2 );
		
			i = strcspn( str + j + 1, "[" );
			i += j + 1;
			j = strcspn( str + i + 1, "]" );
			j += i + 1;
			strncpy( str3, str + i, j - i + 1 );
			str3[ j - i + 1 ] = '\0';
			sscanf( str3, "[expr $cmx * %d /1000]", &x3 );
		
			i = strcspn( str + j + 1, "[" );
			i += j + 1;
			j = strcspn( str + i + 1, "]" );
			j += i + 1;
			strncpy( str4, str + i, j - i + 1 );
			str4[ j - i + 1 ] = '\0';
			sscanf( str4, "[expr $cmy * %d /1000]", &x4 );
			
			// if new data are stored, then add it to the cleaned file
			if ( store( x1, x2, x3, x4 ) == 1 )
				fprintf( f1, "%s", str );
		}
		else
			fprintf( f1, "%s", str );

	}
	fclose( f );
	fclose( f1 );

	if ( d.next != NULL )
		free_storage( d.next );
	if ( d.son != NULL )
		free_storage( d.son );

	return 0;
}
