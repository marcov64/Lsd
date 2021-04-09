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
ANALYSIS.CPP 
Contains the routines to manage the data analysis module.

The main functions contained here are:

- void analysis( int *choice )
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
used case 47
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
bool first_run = true;
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
int first_c;
int num_c;
int num_var;
int num_y2;
int nv;
int pdigits;
int plot_l[ MAX_PLOTS ];
int plot_nl[ MAX_PLOTS ];
int plot_w[ MAX_PLOTS ];
int res;
int showInit;
int time_cross;
int type_plot[ MAX_PLOTS ];
int var_num;
int watch;
int xy;
store *vs = NULL;


/***************************************************
ANALYSIS
****************************************************/
void analysis( int *choice, bool mc )
{
bool gz;
char *app, dirname[ MAX_PATH_LENGTH + 1 ], str1[ MAX_ELEM_LENGTH + 1 ], str2[ MAX_ELEM_LENGTH + 1 ], str3[ MAX_ELEM_LENGTH + 1 ];
double *datum, compvalue;
int h, i, j, k, l, m, p, r;
vector < string > cur_var;
vector < vector < string > > var_names;
FILE *f;

cover_browser( "Analysis of Results...", "Please exit Analysis of Results\nbefore using the LSD Browser.", false );

cmd( "destroytop .dap" );

Tcl_LinkVar( inter, "cur_plot", ( char * ) &cur_plot, TCL_LINK_INT );
Tcl_LinkVar( inter, "nv", ( char * ) &nv, TCL_LINK_INT );
Tcl_LinkVar( inter, "avgSmpl", ( char * ) &avgSmpl, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "showInit", ( char * ) &showInit, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "auto", ( char * ) &autom, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "auto_x", ( char * ) &autom_x, TCL_LINK_BOOLEAN );
Tcl_LinkVar( inter, "firstc", ( char * ) &first_c, TCL_LINK_INT );
Tcl_LinkVar( inter, "numc", ( char * ) &num_c, TCL_LINK_INT );
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
max_c = min_c = num_c = first_c = 1;
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
cmd( "set showInit $showInitP" );
cmd( "set gpdgrid3d \"$gnuplotGrid3D\"" );
cmd( "set gpoptions \"$gnuplotOptions\"" );
cmd( "set moving 0" );
cmd( "set list_times [ list ]" );

cmd( "init_series .da.vars.lb.flt .da.vars.lb.f.v .da.vars.lb.bh.nvar .da.vars.lb.bh.ncas .da.vars.ch.f.v .da.vars.ch.bh.sel .da.vars.pl.f.v .da.vars.pl.bh.plot" );

cmd( "newtop .da \"%s%s - LSD Analysis of Results\" { set choice 2 } \"\"", unsaved_change( ) ? "*" : " ", simul_name );

// main menu
cmd( "ttk::menu .da.m -tearoff 0" );

cmd( "set w .da.m.exit" );
cmd( ".da.m add cascade -label Exit -menu $w -underline 0" );
cmd( "ttk::menu $w -tearoff 0" );
cmd( "$w add command -label \"Quit and Return to Browser\" -command { set choice 2 } -underline 0 -accelerator Esc" );

cmd( "set w .da.m.gp" );
cmd( ".da.m add cascade -label Gnuplot -menu $w -underline 0" );
cmd( "ttk::menu $w -tearoff 0" );
cmd( "$w add command -label \"Open...\" -command { set choice 4 } -underline 5 -accelerator Ctrl+G" );
cmd( "$w add command -label \"Options...\" -command { set choice 37 } -underline 8" );

cmd( "set w .da.m.opt" );
cmd( ".da.m add cascade -label Options -menu $w -underline 0" );
cmd( "ttk::menu $w -tearoff 0" );
cmd( "$w add command -label \"Colors...\" -command { set choice 21 } -underline 0" );
cmd( "$w add command -label \"Plot Parameters...\" -command { set choice 22 } -underline 0" );
cmd( "$w add command -label \"Lattice Parameters...\" -command { set choice 44 } -underline 0" );
cmd( "$w add checkbutton -label \"Average Y Values\" -variable avgSmpl -underline 8" );
cmd( "$w add checkbutton -label \"Show Initial Values\" -variable showInit -underline 5 -command { if { $showInit } { set a 0 } { set a 1 }; set minc [ expr { max( $firstc, $a ) } ]; write_any .da.f.h.v.ft.from.mnc $minc }" );

cmd( "set w .da.m.help" );
cmd( "ttk::menu $w -tearoff 0" );
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

// top listboxes band
cmd( "ttk::frame .da.vars" );

// available series listbox
cmd( "ttk::frame .da.vars.lb" );
cmd( "ttk::label .da.vars.lb.th -text \"Series available\" -style boldSmall.TLabel" );
cmd( "ttk::combobox .da.vars.lb.flt -state readonly -textvariable serPar -postcommand { .da.vars.lb.flt configure -values [ update_parent ] }" );
cmd( "pack .da.vars.lb.th .da.vars.lb.flt -fill x" );

cmd( "bind .da.vars.lb.flt <<ComboboxSelected>> { filter_series }" );

cmd( "set f .da.vars.lb.f" );
cmd( "ttk::frame $f" );
cmd( "ttk::listbox $f.v -selectmode extended -yscroll \"$f.v_scroll set\" -dark $darkTheme" );
cmd( "pack $f.v -side left -expand 1 -fill both" );
cmd( "ttk::scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "pack $f.v_scroll -side left -fill y" );
cmd( "pack $f -expand 1 -fill both" );

cmd( "mouse_wheel $f.v" );
cmd( "bind $f.v <Return> { .da.vars.b.in invoke; break }" );
cmd( "bind $f.v <space> { set res [ .da.vars.lb.f.v get active ]; set choice 30; break }" );
cmd( "bind $f.v <KeyRelease> { \
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
cmd( "bind $f.v <Home> { selectinlist .da.vars.lb.f.v 0; break }" );
cmd( "bind $f.v <End> { selectinlist .da.vars.lb.f.v end; break }" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.lb.f.v <Return> }" );
cmd( "bind $f.v <Button-2> { .da.vars.lb.f.v selection clear 0 end;.da.vars.lb.f.v selection set @%%x,%%y; set res [ selection get ]; set choice 30 } " );
cmd( "bind $f.v <Button-3> { event generate .da.vars.lb.f.v <Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <Shift-Button-2> { .da.vars.lb.f.v selection clear 0 end;.da.vars.lb.f.v selection set @%%x,%%y; set res [ selection get ]; set choice 16 }" );
cmd( "bind $f.v <Shift-Button-3> { event generate .da.vars.lb.f.v <Shift-Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <Control-Button-2> { .da.vars.lb.f.v selection clear 0 end;.da.vars.lb.f.v selection set @%%x,%%y; set res [ selection get ]; set choice 19 }" );
cmd( "bind $f.v <Control-Button-3> { event generate .da.vars.lb.f.v <Control-Button-2> -x %%x -y %%y }" );

cmd( "ttk::frame .da.vars.lb.bh" );
cmd( "ttk::label .da.vars.lb.bh.l1 -text \"Series =\"" );
cmd( "ttk::label .da.vars.lb.bh.nvar -width 6 -anchor w" );
cmd( "ttk::label .da.vars.lb.bh.pad" );
cmd( "ttk::label .da.vars.lb.bh.l2 -text \"Cases =\"" );
cmd( "ttk::label .da.vars.lb.bh.ncas -width 6 -anchor w" );
cmd( "pack .da.vars.lb.bh.l1 .da.vars.lb.bh.nvar .da.vars.lb.bh.pad .da.vars.lb.bh.l2 .da.vars.lb.bh.ncas -side left" );
cmd( "pack .da.vars.lb.bh" );

// vertical toolbar
cmd( "set f .da.vars.b" );
cmd( "ttk::frame $f" );
cmd( "ttk::label $f.pad1 -style boldSmall.TLabel" );
cmd( "ttk::button $f.in -width 6 -style Toolbutton -text \u25b6 -command { set choice 6 }" );
cmd( "ttk::button $f.out -width 6 -style Toolbutton -state disabled -text \u25c0 -command { set choice 7 }" );
cmd( "ttk::button $f.sort -width 6 -style Toolbutton -text \"Sort \u25b2\" -command { set choice 5 } -underline 0" );
cmd( "ttk::button $f.sortdesc -width 6 -style Toolbutton -text \"Sort \u25bc\" -command { set choice 38 } -underline 1" );
cmd( "ttk::button $f.sortend -width 6 -style Toolbutton -text \"Sort+\" -command { set choice 15 } -underline 2" );
cmd( "ttk::button $f.unsort -width 6 -style Toolbutton -text \"Unsort\" -command { set choice 14 } -underline 0" );
cmd( "ttk::button $f.search -width 6 -style Toolbutton -text Find... -command { set choice 39 } -underline 0" );
cmd( "ttk::button $f.add -width 6 -style Toolbutton -text \"Add...\" -command { set choice 24 } -underline 0" );
cmd( "ttk::button $f.empty -width 6 -style Toolbutton -text Clear -command { set choice 8 } -underline 0" );
cmd( "ttk::label $f.pad2" );
cmd( "pack $f.pad1 $f.in $f.out $f.sort $f.sortdesc $f.sortend $f.unsort $f.search $f.add $f.empty $f.pad2 -padx 2 -pady 1 -fill y" );

// selected series listbox
cmd( "ttk::frame .da.vars.ch" );
cmd( "ttk::label .da.vars.ch.th -text \"Series selected\" -style boldSmall.TLabel" );
cmd( "pack .da.vars.ch.th" );

cmd( "set f .da.vars.ch.f" );
cmd( "ttk::frame $f" );
cmd( "ttk::listbox $f.v -selectmode extended -yscroll \"$f.v_scroll set\" -dark $darkTheme" );
cmd( "pack $f.v -side left -expand 1 -fill both" );
cmd( "ttk::scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "pack $f.v_scroll -side left -fill y" );
cmd( "pack $f -expand 1 -fill both" );

cmd( "mouse_wheel $f.v" );
cmd( "bind $f.v <BackSpace> { .da.vars.b.out invoke; break }" );
cmd( "bind $f.v <Return> { .da.b.ts invoke; break }" );
cmd( "bind $f.v <space> { set res [ .da.vars.ch.f.v get active ]; set choice 33; break }" );
cmd( "bind $f.v <KeyRelease> { \
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
cmd( "bind $f.v <Home> { selectinlist .da.vars.ch.f.v 0; break }" );
cmd( "bind $f.v <End> { selectinlist .da.vars.ch.f.v end; break }" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.ch.f.v <BackSpace> }" );
cmd( "bind $f.v <Button-2> { .da.vars.ch.f.v selection clear 0 end;.da.vars.ch.f.v selection set @%%x,%%y; set res [ selection get ]; set choice 33 }" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.ch.f.v <Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <Shift-Button-2> { .da.vars.ch.f.v selection clear 0 end;.da.vars.ch.f.v selection set @%%x,%%y; set res [ selection get ]; set choice 16 }" );
cmd( "bind $f.v <Shift-Button-3> { event generate .da.vars.ch.f.v <Shift-Button-2> -x %%x -y %%y }" );
cmd( "bind $f.v <Control-Button-2> { .da.vars.ch.f.v selection clear 0 end;.da.vars.ch.f.v selection set @%%x,%%y; set res [ selection get ]; set choice 19 }" );
cmd( "bind $f.v <Control-Button-3> { event generate .da.vars.ch.f.v <Control-Button-2> -x %%x -y %%y }" );

cmd( "ttk::frame .da.vars.ch.bh" );
cmd( "ttk::label .da.vars.ch.bh.l -text \"Series =\"" );
cmd( "ttk::label .da.vars.ch.bh.sel -width 5 -anchor w" );
cmd( "pack .da.vars.ch.bh.l .da.vars.ch.bh.sel -side left" );
cmd( "pack .da.vars.ch.bh" );

// plots listbox
cmd( "ttk::frame .da.vars.pl" );
cmd( "ttk::label .da.vars.pl.th -text Plots -style boldSmall.TLabel" );
cmd( "pack .da.vars.pl.th" );

cmd( "set f .da.vars.pl.f" );
cmd( "ttk::frame $f" );
cmd( "ttk::listbox $f.v -selectmode single -yscroll \"$f.v_scroll set\" -dark $darkTheme" );
cmd( "pack $f.v -side left -expand 1 -fill both" );
cmd( "ttk::scrollbar $f.v_scroll -command \"$f.v yview\"" );
cmd( "pack $f.v_scroll -side left -fill y" );
cmd( "pack $f -expand 1 -fill both" );

cmd( "mouse_wheel $f.v" );
cmd( "bind $f.v <Return> { set it [ selection get ]; set choice 3; break }" );
cmd( "bind .da <Delete> { set n_it [ .da.vars.pl.f.v curselection ]; if { $n_it != \"\" } { set it [ selection get ]; set choice 20 }; break }" );
cmd( "bind $f.v <KeyRelease> { \
		if { ( %%s & 0x20004 ) != 0 } { \
			return \
		}; \
		set kk %%K; \
		if { [ string length $kk ] == 1 && [ string is digit -strict $kk ] } { \
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
cmd( "bind $f.v <Home> { selectinlist .da.vars.pl.f.v 0; break }" );
cmd( "bind $f.v <End> { selectinlist .da.vars.pl.f.v end; break }" );
cmd( "bind $f.v <Double-Button-1> { event generate .da.vars.pl.f.v <Return> }" );
cmd( "bind $f.v <Button-2> { .da.vars.pl.f.v selection clear 0 end; .da.vars.pl.f.v selection set @%%x,%%y; set it [ selection get ]; set n_it [ .da.vars.pl.f.v curselection ]; set choice 20 }" );
cmd( "bind $f.v <Button-3> { event generate .da.vars.pl.f.v <Button-2> -x %%x -y %%y }" );

cmd( "ttk::frame .da.vars.pl.bh" );
cmd( "ttk::label .da.vars.pl.bh.l -text \"Plots =\"" );
cmd( "ttk::label .da.vars.pl.bh.plot -width 4 -anchor w" );
cmd( "pack .da.vars.pl.bh.l .da.vars.pl.bh.plot -side left" );
cmd( "pack .da.vars.pl.bh" );

// pack listboxes and vertical toolbar together
cmd( "pack .da.vars.lb -side left -expand 1 -fill both" );
cmd( "pack .da.vars.b -side left -anchor n" );
cmd( "pack .da.vars.ch .da.vars.pl -side left -expand 1 -fill both" );
cmd( "pack .da.vars -pady 5 -expand 1 -fill both" );

// controls band
cmd( "ttk::frame .da.f" );

cmd( "ttk::frame .da.f.h" );				// first horizontal group of controls

cmd( "ttk::frame .da.f.h.v" );				// left options block

cmd( "ttk::frame .da.f.h.v.ft" );			// cases options

cmd( "ttk::checkbutton .da.f.h.v.ft.auto -text \"Use all cases \" -variable auto_x -command { if { $auto_x } { .da.f.h.v.ft.to.mxc conf -state disabled; .da.f.h.v.ft.from.mnc conf -state disabled } { .da.f.h.v.ft.to.mxc conf -state normal; .da.f.h.v.ft.from.mnc conf -state normal } }" );

cmd( "ttk::frame .da.f.h.v.ft.from" );
cmd( "ttk::label .da.f.h.v.ft.from.minc -text \"From case\"" );
cmd( "ttk::entry .da.f.h.v.ft.from.mnc -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 && $n <= $numc } { set minc %%P; return 1 } { %%W delete 0 end; %%W insert 0 $minc; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.ft.from.minc .da.f.h.v.ft.from.mnc -ipadx 5 -side left" );

cmd( "ttk::frame .da.f.h.v.ft.to" );
cmd( "ttk::label .da.f.h.v.ft.to.maxc -text \"to case\"" );
cmd( "ttk::entry .da.f.h.v.ft.to.mxc -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= $minc && $n <= $numc } { set maxc %%P; return 1 } { %%W delete 0 end; %%W insert 0 $maxc; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
cmd( "pack  .da.f.h.v.ft.to.maxc .da.f.h.v.ft.to.mxc -ipadx 5 -side left" );

cmd( "pack .da.f.h.v.ft.auto .da.f.h.v.ft.from .da.f.h.v.ft.to -side left -padx 5 -expand 1 -fill x" );

cmd( "ttk::frame .da.f.h.v.sc" );			// scaling/limits options

cmd( "ttk::checkbutton .da.f.h.v.sc.auto -text \"Y self-scaling\" -variable auto -command { if { $auto } { .da.f.h.v.sc.max.max conf -state disabled; .da.f.h.v.sc.min.min conf -state disabled } { .da.f.h.v.sc.max.max conf -state normal; .da.f.h.v.sc.min.min conf -state normal } }" );

cmd( "ttk::frame .da.f.h.v.sc.min" );
cmd( "ttk::label .da.f.h.v.sc.min.lmin -text \"Min. Y\"" );
cmd( "ttk::entry .da.f.h.v.sc.min.min -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set miny %%P; return 1 } { %%W delete 0 end; %%W insert 0 $miny; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.sc.min.lmin .da.f.h.v.sc.min.min -ipadx 5 -side left" );

cmd( "ttk::frame .da.f.h.v.sc.max" );
cmd( "ttk::label .da.f.h.v.sc.max.lmax -text \"Max. Y\"" );
cmd( "ttk::entry .da.f.h.v.sc.max.max -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set maxy %%P; return 1 } { %%W delete 0 end; %%W insert 0 $maxy; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.sc.max.lmax .da.f.h.v.sc.max.max -ipadx 5 -side left" );

cmd( "pack .da.f.h.v.sc.auto .da.f.h.v.sc.min .da.f.h.v.sc.max -side left -padx 5 -expand 1 -fill x" );

cmd( "ttk::frame .da.f.h.v.y2" );			// log and 2nd y axis
cmd( "ttk::checkbutton .da.f.h.v.y2.logs -text \"Series in logs\" -variable logs" );
cmd( "ttk::label .da.f.h.v.y2.pad" );
cmd( "ttk::checkbutton .da.f.h.v.y2.y2 -text \"Y2 axis\" -variable y2 -command { if { ! $y2 } { .da.f.h.v.y2.f.e conf -state disabled } { .da.f.h.v.y2.f.e conf -state normal } }" );

cmd( "ttk::frame .da.f.h.v.y2.f" );
cmd( "ttk::label .da.f.h.v.y2.f.l -text \"First series in Y2 axis\"" );
cmd( "ttk::spinbox .da.f.h.v.y2.f.e -width 4 -from 2 -to 999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n > 1 } { set num_y2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num_y2; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
cmd( "pack .da.f.h.v.y2.f.l .da.f.h.v.y2.f.e -ipadx 5 -side left" );

cmd( "pack .da.f.h.v.y2.logs .da.f.h.v.y2.pad .da.f.h.v.y2.y2 .da.f.h.v.y2.f -side left -padx 5 -expand 1 -fill x" );

cmd( "pack .da.f.h.v.ft .da.f.h.v.sc .da.f.h.v.y2 -anchor w -expand 1 -fill x" );

// right options block
cmd( "ttk::frame .da.f.h.tc -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
cmd( "ttk::radiobutton .da.f.h.tc.time -text \"Time series\" -variable tc -value 0 -command { if { $xy == 0 } { .da.f.h.v.y2.y2 conf -state normal }; if { $xy == 1 } { .da.f.tit.lp.line config -state normal; set line_point $linemodeP } }" );
cmd( "ttk::radiobutton .da.f.h.tc.cross -text \"Cross-section\" -variable tc -value 1 -command { set y2 0; .da.f.h.v.y2.y2 conf -state disabled; .da.f.h.v.y2.f.e conf -state disabled; if { $xy == 1 } { set line_point 2;  .da.f.tit.lp.line config -state disabled } }" );
cmd( "pack .da.f.h.tc.time .da.f.h.tc.cross -anchor w" );

cmd( "ttk::frame .da.f.h.xy -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
cmd( "ttk::radiobutton .da.f.h.xy.seq -text \"Sequence\" -variable xy -value 0 -command { if { $tc == 0 } { .da.f.h.v.y2.y2 conf -state normal } { set y2 0; .da.f.h.v.y2.y2 conf -state disabled; .da.f.h.v.y2.f.e conf -state disabled }; .da.f.tit.run.gnu conf -state disabled; .da.f.tit.run.watch conf -state normal; if { $tc == 1 } { .da.f.tit.lp.line config -state normal; set line_point $linemodeP } }" );
cmd( "ttk::radiobutton .da.f.h.xy.xy -text \"XY plot\" -variable xy -value 1 -command { set y2 0; .da.f.h.v.y2.y2 conf -state disabled; .da.f.h.v.y2.f.e conf -state disabled; .da.f.tit.run.gnu conf -state normal; .da.f.tit.run.watch conf -state disabled; if { $tc == 1 } { set line_point 2;  .da.f.tit.lp.line config -state disabled } }" );
cmd( "pack .da.f.h.xy.seq .da.f.h.xy.xy -anchor w" );

// pack first horizontal group of controls
cmd( "pack .da.f.h.v .da.f.h.tc .da.f.h.xy -padx 20 -side left -expand 1 -fill x" );

// second horizontal group of controls
cmd( "ttk::frame .da.f.tit" );

cmd( "ttk::frame .da.f.tit.t" );				// title box
cmd( "ttk::label .da.f.tit.t.l -text Title" );
cmd( "ttk::entry .da.f.tit.t.e -textvariable tit -width 35 -justify center" );
cmd( "pack .da.f.tit.t.l .da.f.tit.t.e -ipadx 5 -side left" );

cmd( "ttk::frame .da.f.tit.chk" );			// golor/grid options

cmd( "ttk::checkbutton .da.f.tit.chk.allblack -text \"No colors\" -variable allblack" );
cmd( "ttk::checkbutton .da.f.tit.chk.grid -text \"Grids\" -variable grid" );
cmd( "pack .da.f.tit.chk.allblack .da.f.tit.chk.grid -anchor w" );

cmd( "ttk::frame .da.f.tit.run" );			// watch/gnuplot options 
cmd( "ttk::checkbutton .da.f.tit.run.watch -text Watch -variable watch" );
cmd( "ttk::checkbutton .da.f.tit.run.gnu -text Gnuplot -variable gnu -state disabled" );
cmd( "pack .da.f.tit.run.watch .da.f.tit.run.gnu -anchor w" );

cmd( "ttk::frame .da.f.tit.pr" );			// precision positions
cmd( "ttk::label .da.f.tit.pr.l -text \"Precision\"" );
cmd( "ttk::spinbox .da.f.tit.pr.e -width 2 -from 0 -to 9 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 && $n <= 9 } { set pdigits %%P; return 1 } { %%W delete 0 end; %%W insert 0 $pdigits; return 0 } } -invalidcommand { bell } -justify center" );
cmd( "pack .da.f.tit.pr.l .da.f.tit.pr.e" );

cmd( "ttk::frame .da.f.tit.ps" );			// point size
cmd( "ttk::label .da.f.tit.ps.l -text \"Point size\"" );
cmd( "ttk::spinbox .da.f.tit.ps.e -width 4 -from 0.2 -to 9.8 -increment 0.2 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] && $n >= 0.2 && $n <= 9.8 } { set point_size %%P; return 1 } { %%W delete 0 end; %%W insert 0 $point_size; return 0 } } -invalidcommand { bell } -justify center" );
cmd( "pack .da.f.tit.ps.l .da.f.tit.ps.e" );

// line/points option
cmd( "ttk::frame .da.f.tit.lp -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
cmd( "ttk::radiobutton .da.f.tit.lp.line -text \"Lines\" -variable line_point -value 1" );
cmd( "ttk::radiobutton .da.f.tit.lp.point -text \"Points\" -variable line_point -value 2" );
cmd( "pack .da.f.tit.lp.line .da.f.tit.lp.point -anchor w" );

// pack second horizontal group of controls
cmd( "pack .da.f.tit.t .da.f.tit.chk .da.f.tit.run .da.f.tit.pr .da.f.tit.ps .da.f.tit.lp -padx 10 -pady 5 -side left -expand 1 -fill x" );

// pack controls band
cmd( "pack .da.f.h .da.f.tit" );
cmd( "pack .da.f -padx 5 -pady 5" );

// button bar
cmd( "ttk::frame .da.b" );
cmd( "ttk::button .da.b.ts -width [ expr { $butWid + 1 } ] -text Plot -command { set choice 1 } -underline 0" );
cmd( "ttk::button .da.b.dump -width [ expr { $butWid + 1 } ] -text \"Save Plot\" -command { set fromPlot 0; set choice 11 } -underline 2" );
cmd( "ttk::button .da.b.sv -width [ expr { $butWid + 1 } ] -text \"Save Data\" -command { set choice 10 } -underline 3" );
cmd( "ttk::button .da.b.sp -width [ expr { $butWid + 1 } ] -text \"Show Data\" -command { set choice 36 } -underline 5" );
cmd( "ttk::button .da.b.st -width [ expr { $butWid + 1 } ] -text Statistics -command { set choice 12 } -underline 1" );
cmd( "ttk::button .da.b.fr -width [ expr { $butWid + 1 } ] -text Histogram -command { set choice 32 } -underline 0" );
cmd( "ttk::button .da.b.lat -width [ expr { $butWid + 1 } ] -text Lattice -command { set choice 23 } -underline 0" );

cmd( "pack .da.b.ts .da.b.dump .da.b.sv .da.b.sp .da.b.st .da.b.fr .da.b.lat -padx $butSpc -side left" );
cmd( "pack .da.b -padx $butPad -pady $butPad -side right" );

// top window shortcuts binding
cmd( "bind .da <KeyPress-Escape> { set choice 2 }" );	// quit
cmd( "bind .da <Control-l> { set choice 23 }; bind .da <Control-L> { set choice 23 }" );	// plot lattice
cmd( "bind .da <Control-h> { set choice 32 }; bind .da <Control-H> { set choice 32 }" );	// plot histograms
cmd( "bind .da <Control-c> { set choice 8 }; bind .da <Control-C> { set choice 8 }" );	// empty (clear) selected series
cmd( "bind .da <Control-Shift-a> { set choice 24; break }; bind .da <Control-Shift-A> { set choice 24; break }" );	// insert new series
cmd( "bind .da <Control-i>  { set choice 34 }; bind .da <Control-I> { set choice 34 }" );	// sort selected in inverse order
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
cmd( "bind .da <Control-b> { set choice 45 }; bind .da <Control-B> { set choice 45 }" ); 	// add not saved series
cmd( "bind .da <Control-w> { set choice 46 }; bind .da <Control-W> { set choice 46 }" ); 	// create new series from existing
cmd( "bind .da <Control-m> { set choice 47 }; bind .da <Control-M> { set choice 47 }" ); 	// create moving average from existing

// grab focus when called from LSD Debugger
Tcl_SetVar( inter, "running", running ? "1" : "0", 0 );
cmd( "if $running { showtop .da overM } { showtop .da overM 1 1 0 }" );

// add time series in memory to listbox
if ( actual_steps > 0 )
{
	insert_data_mem( root, &num_var );
	
	min_c = max( first_c, showInit ? 0 : 1 );
	max_c = num_c;
} 
else
{	// create parent map from loaded but not run configuration
	par_map.clear( );
	create_par_map( root );
}

if ( ! mc && num_var == 0 )
{
	if ( first_run )
		cmd( "ttk::messageBox -parent .da -type ok -title \"Analysis of Results\" -icon info -message \"There are no series available\" -detail \"Click on button 'Add...' to load series from results files.\n\nIf you were looking for data after a simulation run, please make sure you have selected the series to be saved, or have not set the objects containing them to not be computed.\"" );
	
	first_run = false;
}
else
{
	if ( ! mc && sim_num > 1 )
	  cmd( "ttk::messageBox -parent .da -type ok -title \"Analysis of Results\" -icon info -message \"Only series from last run are loaded\" -detail \"Click on button 'Add...' to load series from saved simulation results. You can use 'Ctrl' and 'Shift' keys to select multiple files at once. Avoid selecting the results file from last run, as data is already loaded and would be duplicated.\"" );  
	
	cmd( "selectinlist .da.vars.lb.f.v 0 1" );
}

// main loop

while ( true )
{
	// sort the list of available variables
	cmd( "if [ info exists DaModElem ] { set DaModElem [ lsort -unique -dictionary $DaModElem ] }" );

	// enable/disable the buttons in the series toolbar and buttons
	cmd( "if { [ .da.vars.lb.f.v size ] > 0 } { \
			.da.vars.b.in conf -state normal; \
			.da.vars.b.sort conf -state normal; \
			.da.vars.b.sortdesc conf -state normal; \
			.da.vars.b.sortend conf -state normal; \
			.da.vars.b.unsort conf -state normal; \
			.da.vars.b.search conf -state normal \
		} { \
			.da.vars.b.in conf -state disabled; \
			.da.vars.b.sort conf -state disabled; \
			.da.vars.b.sortdesc conf -state disabled; \
			.da.vars.b.sortend conf -state disabled; \
			.da.vars.b.unsort conf -state disabled; \
			.da.vars.b.search conf -state disabled \
		}" );
	cmd( "if { [ .da.vars.ch.f.v size ] > 0 } { \
			.da.vars.b.out conf -state normal; \
			.da.vars.b.empty conf -state normal; \
			.da.f.tit.t.e conf -state normal; \
			.da.b.ts conf -state normal; \
			.da.b.dump conf -state normal; \
			.da.b.sv conf -state normal; \
			.da.b.sp conf -state normal; \
			.da.b.st conf -state normal; \
			.da.b.fr conf -state normal; \
			.da.b.lat conf -state normal; \
		} { \
			.da.vars.b.out conf -state disabled; \
			.da.vars.b.empty conf -state disabled; \
			.da.f.tit.t.e conf -state disabled; \
			.da.b.ts conf -state disabled; \
			.da.b.dump conf -state disabled; \
			.da.b.sv conf -state disabled; \
			.da.b.sp conf -state disabled; \
			.da.b.st conf -state disabled; \
			.da.b.fr conf -state disabled; \
			.da.b.lat conf -state disabled; \
		}" );

	// reset first second scale series if option is disabled
	cmd( "if { ! $y2 } { set num_y2 2 }" );

	// update entry boxes with linked variables
	update_bounds( );
	cmd( "write_disabled .da.f.h.v.y2.f.e $num_y2" );
	cmd( "write_any .da.f.tit.ps.e $point_size" ); 
	cmd( "write_any .da.f.tit.pr.e $pdigits" ); 

	// update series on-screen statistics
	cmd( "stat_series" );

	// analysis command loop (enter loading MC experiment if needed)
	*choice = mc ? 25 : 0;
	choice_g = 0;
	while ( ! *choice && ! choice_g )
		Tcl_DoOneEvent( 0 );

	// coming from the structure window
	if ( choice_g )
	{
		*choice = 29;					// change the parent filter
		cmd( "focus .da.vars.lb.f.v" );
	}

	// update linked variables with values in entry boxes
	cmd( "if [ string is double [ .da.f.h.v.sc.min.min get ] ] { set miny [ .da.f.h.v.sc.min.min get ] }" );
	cmd( "if [ string is double [ .da.f.h.v.sc.max.max get ] ] { set maxy [ .da.f.h.v.sc.max.max get ] }" );
	cmd( "if [ string is integer [ .da.f.h.v.ft.from.mnc get ] ] { set minc [ .da.f.h.v.ft.from.mnc get ] }" );
	cmd( "if [ string is integer [ .da.f.h.v.ft.to.mxc get ] ] { set maxc [ .da.f.h.v.ft.to.mxc get ] }" );
	cmd( "if [ string is double [ .da.f.h.v.y2.f.e get ] ] { set num_y2 [ .da.f.h.v.y2.f.e get ] }" );
	cmd( "if [ string is integer [ .da.f.tit.ps.e get ] ] { set point_size [ .da.f.tit.ps.e get ] }" ); 
	cmd( "if [ string is integer [ .da.f.tit.pr.e get ] ] { set pdigits [ .da.f.tit.pr.e get ] }" ); 
	
	cmd( "set nv [ .da.vars.ch.f.v size ]" );
	
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
			cmd( "if { [ .da.vars.pl.f.v size ] != 0 } { set answer [ ttk::messageBox -parent .da -type okcancel -title Confirmation -icon question -default ok -message \"Exit Analysis of Results?\" -detail \"All the plots and series created and not saved will be lost.\"] } { set answer ok }" );
			cmd( "if { ! [ string equal $answer ok ] } { set choice 0 }" );
			if ( *choice == 0 )
				break;

			delete [ ] vs;
			vs = NULL;

			cmd( "destroytop .dap" );
			cmd( "destroytop .da" );
			uncover_browser( );
				
			Tcl_UnlinkVar( inter, "auto" );
			Tcl_UnlinkVar( inter, "auto_x" );
			Tcl_UnlinkVar( inter, "firstc" );
			Tcl_UnlinkVar( inter, "numc" );
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
			Tcl_UnlinkVar( inter, "showInit" );

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
				cmd( "addtolist .da.vars.pl.f.v \"${cur_plot}) ${tit}\"" );
			
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
				cmd( "addtolist .da.vars.pl.f.v \"${cur_plot}) ${tit}\"" );
				
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
				cmd( "addtolist .da.vars.pl.f.v \"${cur_plot}) ${tit}\"" );
			
			break;

		  
		// Plot_cs_xy
		case 18:
			cur_plot++;
			
			plot_cs_xy( choice );
			
			if ( *choice == 2 )		//plot aborted
				cur_plot--;
			else
				cmd( "addtolist .da.vars.pl.f.v \"${cur_plot}) ${tit}\"" );
			
			break;

		  
		// plot a lattice. 
		// Data must be stored on a single time step organized for lines and columns in sequence
		case 23:
			cur_plot++;
			
			plot_lattice( choice );
			
			if ( *choice == 2 )		//plot aborted
				cur_plot--;
			else   
				cmd( "addtolist .da.vars.pl.f.v \"${cur_plot}) ${tit}\"" );
			
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
				cmd( "addtolist .da.vars.pl.f.v \"${cur_plot}) ${tit}\"" );
				
			break;


		// Print the data series in the log window
		case 36: 
			plog_series( choice );
			cmd( "focustop .log .da" );
			
			break;
		   
		   
		// show the equation for the selected element
		case 16:
			cmd( "set a [ split $res ]; set b [ lindex $a 0 ]" );
			app = ( char * ) Tcl_GetVar( inter, "b", 0 );
			
			*choice = 2;	// point .da window as parent for the following window
			show_eq( app, choice );
			
			break;


		// show the description for the selected element
		case 19:
			cmd( "set a [ split $res ]; set b [ lindex $a 0 ]" );
			app = ( char * ) Tcl_GetVar( inter, "b", 0 );
			
			*choice = 2;	// point .da window as parent for the following window
			show_descr( app, choice );
			
			break;


		// Save data in the Vars. to plot listbox
		case 10:
			save_datazip( choice );
			
			break;


		// Statistics
		case 12:
			statistics( choice );
			cmd( "focustop .log .da" );
			
			break;

		 
		// Statistics Cross Section
		case 13:
			set_cs_data( choice );
			
			if ( *choice != 2 )
			{
				statistics_cross( choice );
				cmd( "focustop .log .da" );
			}
			
			break;


		// save plot to file
		case 11:
			cmd( "set choice $fromPlot" );
			if ( ! *choice )
			{
				cmd( "set choice [ .da.vars.pl.f.v size ]" );
				if ( *choice == 0 )		 // no plots to save
				{
					cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No plot available\" -detail \"Place one or more series in the Series Selected listbox and select 'Plot' to produce a plot.\"" );
					break;
				}

				do
				{
					cmd( "set iti [ .da.vars.pl.f.v curselection ]" );
					cmd( "if { [ string length $iti ] == 0 } { set choice 1 } { set choice 0 }" );

					if ( *choice == 1 )
					{
						*choice = 0;
						cmd( "newtop .da.a \"Save Plot\" { set choice 2 } .da" );
						cmd( "ttk::label .da.a.l -justify center -text \"Select one plot from the \nPlots listbox before clicking 'OK'\"" );
						cmd( "pack .da.a.l -pady 10 -padx 5" );
						cmd( "okhelpcancel .da.a b { set choice 1 } { LsdHelp menudata_res.html#postscript } { set choice 2 }" );
						cmd( "showtop .da.a centerW 0 0 0" );
						cmd( "mousewarpto .da.a.b.ok" );
						
						while ( *choice == 0 )
							Tcl_DoOneEvent( 0 );
						cmd( "destroytop .da.a" );
					}
				}
				while ( *choice == 1 );

				if ( *choice == 2 )
					break;

				*choice = 0;
				
				cmd( "set it [ .da.vars.pl.f.v get $iti ]" );
			}
			
			cmd( "set choice [ info exists it ]" );
			if ( ! *choice )
				break;
			
			cmd( "unset a" );
			cmd( "scan $it %%d)%%s a b" );
			cmd( "set choice [ info exists a ]" );
			if ( ! *choice )
				break;
			
			cmd( "set choice [ winfo exists $daptab.tab$a ]" );

			if ( *choice == 0 )
			{
				cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Cannot save plot\" -detail \"The plot was produced in Gnuplot. Please reopen/create the plot and use the Gnuplot tools (toolbar buttons) to save it.\"" );
				break;  
			}

			cmd( "if { ! [ info exists pltSavFmt ] } { set pltSavFmt svg }" );
			cmd( "if { ! [ info exists pltSavCmod ] } { set pltSavCmod color }" );
			cmd( "if { ! [ info exists pltSavRes ] } { set pltSavRes 0 }" );
			cmd( "if { ! [ info exists pltSavDim ] } { set pltSavDim 270 }" );
			cmd( "if { ! [ info exists pltSavLeg ] } { set pltSavLeg 1 }" );
			
			cmd( "newtop .da.file \"Save Plot\" { set choice 2 } .da" );

			cmd( "ttk::frame .da.file.l" );
			cmd( "ttk::label .da.file.l.l1 -text \"Settings for plot\"" );
			cmd( "ttk::label .da.file.l.l2 -style hl.TLabel -text \"$a) $b\"" );
			cmd( "pack .da.file.l.l1 .da.file.l.l2" );

			cmd( "ttk::frame .da.file.opt" );
			
			cmd( "ttk::frame .da.file.opt.fmt -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			cmd( "ttk::radiobutton .da.file.opt.fmt.p1 -text SVG -variable pltSavFmt -value svg -command { \
					.da.file.opt.pos.p1 configure -state disabled; \
					.da.file.opt.pos.p2 configure -state disabled;  \
					.da.file.dim.n configure -state disabled  \
				}" );
			cmd( "ttk::radiobutton .da.file.opt.fmt.p2 -text Postscript -variable pltSavFmt -value eps -command { \
					.da.file.opt.pos.p1 configure -state normal; \
					.da.file.opt.pos.p2 configure -state normal;  \
					.da.file.dim.n configure -state normal  \
				}" );
			cmd( "pack .da.file.opt.fmt.p1 .da.file.opt.fmt.p2 -side left -ipadx 5" );
			
			cmd( "ttk::frame .da.file.opt.col -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			cmd( "ttk::radiobutton .da.file.opt.col.r1 -text Color -variable pltSavCmod -value color" );
			cmd( "ttk::radiobutton .da.file.opt.col.r2 -text Grayscale -variable pltSavCmod -value gray" );
			cmd( "ttk::radiobutton .da.file.opt.col.r3 -text Mono -variable pltSavCmod -value mono" );
			cmd( "pack .da.file.opt.col.r1 .da.file.opt.col.r2 .da.file.opt.col.r3 -side left -ipadx 5" );

			cmd( "ttk::frame .da.file.opt.pos -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			cmd( "ttk::radiobutton .da.file.opt.pos.p1 -text Landscape -variable pltSavRes -value 0" );
			cmd( "ttk::radiobutton .da.file.opt.pos.p2 -text Portrait -variable pltSavRes -value 1" );
			cmd( "pack .da.file.opt.pos.p1 .da.file.opt.pos.p2 -side left -ipadx 5" );
			
			cmd( "pack .da.file.opt.fmt .da.file.opt.col .da.file.opt.pos -pady 5" );

			cmd( "ttk::frame .da.file.dim" );
			cmd( "ttk::label .da.file.dim.l1 -text Dimension" );
			cmd( "ttk::entry .da.file.dim.n -width 4 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set pltSavDim %%P; return 1 } { %%W delete 0 end; %%W insert 0 $pltSavDim; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.file.dim.n insert 0 $pltSavDim" );
			cmd( "ttk::label .da.file.dim.l2 -text \"(mm@96DPI)\"" );
			cmd( "pack .da.file.dim.l1 .da.file.dim.n .da.file.dim.l2 -side left" );

			cmd( "ttk::checkbutton .da.file.lab -text \"Include series legends\" -variable pltSavLeg -onvalue 1 -offvalue 0" );
			
			cmd( "if [ string equal $pltSavFmt eps ] { \
					.da.file.opt.pos.p1 configure -state normal; \
					.da.file.opt.pos.p2 configure -state normal;  \
					.da.file.dim.n configure -state normal  \
				} else { \
					.da.file.opt.pos.p1 configure -state disabled; \
					.da.file.opt.pos.p2 configure -state disabled;  \
					.da.file.dim.n configure -state disabled  \
				}" );

			cmd( "set choice $a" );
			if ( plot_l[ *choice ] == plot_nl[ *choice ] )
				cmd( ".da.file.lab conf -state disabled" );

			cmd( "pack .da.file.l .da.file.opt .da.file.dim .da.file.lab -pady 5 -padx 5" );
			cmd( "okhelpcancel .da.file b { set choice 1 } { LsdHelp menudata_res.html#postscript } { set choice 2 }" );
			cmd( "showtop .da.file" );
			cmd( "mousewarpto .da.file.b.ok" );

			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set pltSavDim [ .da.file.dim.n get ]" ); 
			cmd( "destroytop .da.file" );

			if ( *choice == 2 )
				break;

			// make sure there is a path set
			cmd( "set path \"%s\"", path );
			if ( strlen( path ) > 0 )
				cmd( "cd \"$path\"" );

			cmd( "if [ string equal $pltSavFmt eps ] { \
					set t \"Encapsulated Postscript\" \
				} else { \
					set t \"Scalable Vector Graphics\" \
				}" );
					
			cmd( "set fn [ tk_getSaveFile -parent .da -title \"Save Plot File\" -defaultextension .$pltSavFmt -initialfile $b.$pltSavFmt -initialdir \"$path\" -filetypes { { {Scalable Vector Graphics} {.svg} } { {Encapsulated Postscript} {.eps} } { {All files} {*} } } -typevariable t ]; if { [ string length $fn ] == 0 } { set choice 2 }" );
			
			if ( *choice == 2 )
				break;

			cmd( "set dd ${pltSavDim}m" );
			cmd( "set fn [ file nativename $fn ]" );
			cmd( "set e [ string trimleft [ file extension $fn ] . ]" );
			cmd( "if { $e in [ list svg eps ] } { set pltSavFmt $e }" );
			
			cmd( "set bb [ $daptab.tab$a.c.f.plots bbox all ]" );
			cmd( "set x0 [ expr { [ lindex $bb 0 ] - 2 * $tbordsizeP } ]" );
			cmd( "set y0 [ expr { [ lindex $bb 1 ] - 2 * $tbordsizeP } ]" );
			cmd( "set x1 [ expr { [ lindex $bb 2 ] + 4 * $tbordsizeP } ]" );

			cmd( "set choice $pltSavLeg" );
			if ( *choice == 0 )				// remove legends, if any
			{
				// check for valid boundary values
				cmd( "set choice $a" );
				if ( plot_l[ *choice ] > 0 && plot_l[ *choice ] > plot_nl[ *choice ] )
				{
					cmd( "set y1 [ expr { %d + [ font metric \"$fontP\" -descent ] } ]", plot_nl[ *choice ] );
					cmd( "if [ info exists zoomLevel%d ] { \
						set y1 [ expr { $y1 * $zoomLevel%d } ] \
					}", *choice, *choice );
					
					cmd( "if [ string equal $pltSavFmt eps ] { \
							set y1 [ expr { $y1 + 2 * $tbordsizeP } ] \
						} else { \
							set y1 [ expr { $y1 + 4 * $tbordsizeP } ] \
						}" );
				}
			}
			else
				cmd( "set y1 [ expr { [ lindex $bb 3 ] + 4 * $tbordsizeP } ]" );
			
			cmd( "if [ string equal $pltSavFmt eps ] { \
					$daptab.tab$a.c.f.plots postscript -x $x0 -y $y0 -width [ expr { $x1 - $x0 } ] -height [ expr { $y1 - $y0 } ] -pagewidth $dd -rotate $pltSavRes -colormode $pltSavCmod -file \"$fn\" \
				} else { \
					canvas2svg $daptab.tab$a.c.f.plots \"$fn\" \"$x0 $y0 $x1 $y1\" $pltSavCmod %s \
				}", simul_name );
				
			cmd( "plog \"\nPlot saved: $fn\n\"" );

			break;


		// SELECTION BOX CONTEXT MENUS

		// Use right button of the mouse to select all series with a given label
		case 30:
			compvalue = 0;
			Tcl_LinkVar( inter, "compvalue", ( char * ) &compvalue, TCL_LINK_DOUBLE );
			cmd( "set a [ split $res ]" );
			cmd( "set b [ lindex $a 0 ]" );
			cmd( "set c [ lindex $a 1 ]" ); // get the tag value
			cmd( "set ntag [ llength [ split $c {_} ] ]" );
			cmd( "set ssys 2" );
			cmd( "if { ! [ info exist ca1 ] || ! [ string is integer -strict $ca1 ] } { set ca1 0 }" );
			cmd( "if { ! [ info exist ca2 ] || ! [ string is integer -strict $ca2 ] } { set ca2 $maxc }" );
			cmd( "if { ! [ info exist tvar ] || ! [ string is integer -strict $tvar ] } { set tvar $maxc }" );
			cmd( "if { ! [ info exist cond ] } { set cond 1 }" );

			cmd( "newtop .da.a \"Select Series\" { set choice 2 } .da" );

			cmd( "ttk::frame .da.a.tit" );
			cmd( "ttk::label .da.a.tit.l -text \"Select series with name\"" );
			cmd( "ttk::label .da.a.tit.s -text \"$b\" -style hl.TLabel" );
			cmd( "pack .da.a.tit.l .da.a.tit.s" );
			cmd( "ttk::frame .da.a.q -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			
			// select all
			cmd( "ttk::frame .da.a.q.f1" );
			cmd( "ttk::radiobutton .da.a.q.f1.c -text \"Select all\" -variable ssys -value 2 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state disabled \
					}; \
					.da.a.q.f2.f.e conf -state disabled; \
					.da.a.q.f4.l.e1 conf -state disabled; \
					.da.a.q.f4.l.e2 conf -state disabled; \
					.da.a.c.v.t.e2 conf -state disabled; \
					.da.a.c.v.c.e conf -state disabled; \
					.da.a.c.o.eq conf -state disabled; \
					.da.a.c.o.dif conf -state disabled; \
					.da.a.c.o.geq conf -state disabled; \
					.da.a.c.o.g conf -state disabled; \
					.da.a.c.o.seq conf -state disabled; \
					.da.a.c.o.s conf -state disabled \
				}" );
			cmd( "bind .da.a.q.f1.c <Return> { \
					.da.a.q.f1.c invoke; \
					focus .da.a.b.r2.ok \
				}" );
			cmd( "bind .da.a.q.f1.c <Down> { \
					focus .da.a.q.f.c; \
					.da.a.q.f.c invoke \
				}" );
			cmd( "pack .da.a.q.f1.c" );
			cmd( "pack .da.a.q.f1 -anchor w" );
			
			// select tags
			cmd( "ttk::frame .da.a.q.f" );
			cmd( "ttk::radiobutton .da.a.q.f.c -text \"Select by series tags\" -variable ssys -value 1 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state normal \
					}; \
					.da.a.q.f2.f.e conf -state disabled; \
					.da.a.q.f4.l.e1 conf -state disabled; \
					.da.a.q.f4.l.e2 conf -state disabled; \
					.da.a.c.v.t.e2 conf -state disabled; \
					.da.a.c.v.c.e conf -state disabled; \
					.da.a.c.o.eq conf -state normal; \
					.da.a.c.o.dif conf -state normal; \
					.da.a.c.o.geq conf -state normal; \
					.da.a.c.o.g conf -state normal; \
					.da.a.c.o.seq conf -state normal; \
					.da.a.c.o.s conf -state normal \
				}" );
			cmd( "bind .da.a.q.f.c <Up> { \
					focus .da.a.q.f1.c; \
					.da.a.q.f1.c invoke \
				}" );
			cmd( "bind .da.a.q.f.c <Return> { \
					focus .da.a.q.f.l.e0; \
					.da.a.q.f.l.e0 selection range 0 end \
				}" );
			cmd( "bind .da.a.q.f.c <Down> { \
					focus .da.a.q.f4.c; \
					.da.a.q.f4.c invoke \
				}" );
			cmd( "pack .da.a.q.f.c -anchor w" );
			cmd( "ttk::frame .da.a.q.f.l" );
			cmd( "for { set x 0 } { $x < $ntag } { incr x } { \
					if { $x > 0 } { \
						ttk::label .da.a.q.f.l.s$x -text \u2014 \
					}; \
					ttk::entry .da.a.q.f.l.e$x -width 4 -textvariable vtag($x) -justify center -state disabled \
				}" );
			cmd( "for { set x 0 } { $x < $ntag } { incr x } { \
					if { $x > 0 } { \
						pack .da.a.q.f.l.s$x -padx 2 -side left \
					}; \
					pack .da.a.q.f.l.e$x -side left; \
					bind .da.a.q.f.l.e$x <Return> [ subst -nocommand { focus .da.a.q.f.l.e[ expr { $x + 1 } ]; \
					.da.a.q.f.l.e[ expr { $x + 1 } ] selection range 0 end } ]; \
					bind .da.a.q.f.l.e$x <KeyRelease> { .da.a.q.f.c invoke } }; \
					incr x -1; \
					bind .da.a.q.f.l.e$x <Return> { focus .da.a.b.r2.ok }" );
			cmd( "pack .da.a.q.f.l -anchor w -padx 25" );
			cmd( "pack .da.a.q.f -anchor w" );
		
			// select cases
			cmd( "ttk::frame .da.a.q.f4" );
			cmd( "ttk::radiobutton .da.a.q.f4.c -text \"Select by series cases\" -variable ssys -value 5 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state disabled \
					}; \
					.da.a.q.f2.f.e conf -state disabled; \
					.da.a.q.f4.l.e1 conf -state normal; \
					.da.a.q.f4.l.e2 conf -state normal; \
					.da.a.c.v.t.e2 conf -state disabled; \
					.da.a.c.v.c.e conf -state disabled; \
					.da.a.c.o.eq conf -state disabled; \
					.da.a.c.o.dif conf -state disabled; \
					.da.a.c.o.geq conf -state disabled; \
					.da.a.c.o.g conf -state disabled; \
					.da.a.c.o.seq conf -state disabled; \
					.da.a.c.o.s conf -state disabled \
				}" );
			cmd( "bind .da.a.q.f4.c <Up> { \
					focus .da.a.q.f.c; \
					.da.a.q.f.c invoke \
				}" );
			cmd( "bind .da.a.q.f4.c <Return> { \
					.da.a.q.f4.c invoke; \
					focus .da.a.b.r2.ok \
				}" );
			cmd( "bind .da.a.q.f4.c <Down> { \
					focus .da.a.q.f3.s; \
					.da.a.q.f3.s invoke \
				}" );
			cmd( "pack .da.a.q.f4.c -anchor w" );
			
			cmd( "ttk::frame .da.a.q.f4.l" );
			cmd( "ttk::entry .da.a.q.f4.l.e1 -width 5 -textvariable ca1 -justify center -state disabled" );
			cmd( "ttk::label .da.a.q.f4.l.s -text to" );
			cmd( "ttk::entry .da.a.q.f4.l.e2 -width 5 -textvariable ca2 -justify center -state disabled" );
			cmd( "pack .da.a.q.f4.l.e1 .da.a.q.f4.l.s .da.a.q.f4.l.e2 -padx 2 -side left" );
			cmd( "bind .da.a.q.f4.l.e1 <Return> { \
					focus .da.a.q.f4.l.e2; \
					.da.a.q.f4.l.e2 selection range 0 end \
				}" );
			cmd( "bind .da.a.q.f4.l.e2 <Return> { focus .da.a.b.r2.ok }" );
			cmd( "pack .da.a.q.f4.l -anchor w -padx 25" );
			
			cmd( "pack .da.a.q.f4 -anchor w" );
			
			// select by values
			cmd( "ttk::frame .da.a.q.f3" );
			cmd( "ttk::radiobutton .da.a.q.f3.s -text \"Select by series values\" -variable ssys -value 3 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state disabled \
					}; \
					.da.a.q.f2.f.e conf -state disabled; \
					.da.a.q.f4.l.e1 conf -state disabled; \
					.da.a.q.f4.l.e2 conf -state disabled; \
					.da.a.c.v.t.e2 conf -state normal; \
					.da.a.c.v.c.e conf -state normal; \
					.da.a.c.o.eq conf -state normal; \
					.da.a.c.o.dif conf -state normal; \
					.da.a.c.o.geq conf -state normal; \
					.da.a.c.o.g conf -state normal; \
					.da.a.c.o.seq conf -state normal; \
					.da.a.c.o.s conf -state normal \
				}" );
			cmd( "bind .da.a.q.f3.s <Up> { \
					focus .da.a.q.f4.c; \
					.da.a.q.f4.c invoke \
				}" );
			cmd( "bind .da.a.q.f3.s <Return> { \
					focus .da.a.c.v.c.e; \
					.da.a.c.v.c.e selection range 0 end \
				}" );
			cmd( "bind .da.a.q.f3.s <Down> { \
					focus .da.a.q.f2.s; \
					.da.a.q.f2.s invoke \
				}" );
			cmd( "pack .da.a.q.f3.s -anchor w" );
			cmd( "pack .da.a.q.f3 -anchor w" );
			
			// select by values from other series
			cmd( "ttk::frame .da.a.q.f2" );
			cmd( "ttk::radiobutton .da.a.q.f2.s -text \"Select by values from another series\" -variable ssys -value 4 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state disabled \
					}; \
					.da.a.q.f2.f.e conf -state normal; \
					.da.a.q.f4.l.e1 conf -state disabled; \
					.da.a.q.f4.l.e2 conf -state disabled; \
					.da.a.c.v.t.e2 conf -state normal; \
					.da.a.c.v.c.e conf -state normal; \
					.da.a.c.o.eq conf -state normal; \
					.da.a.c.o.dif conf -state normal; \
					.da.a.c.o.geq conf -state normal; \
					.da.a.c.o.g conf -state normal; \
					.da.a.c.o.seq conf -state normal; \
					.da.a.c.o.s conf -state normal \
				}" );
			cmd( "bind .da.a.q.f2.s <Up> { \
					focus .da.a.q.f3.s; \
					.da.a.q.f3.s invoke \
				}" );
			cmd( "bind .da.a.q.f2.s <Return> { \
					focus .da.a.q.f2.f.e; \
					.da.a.q.f2.f.e selection range 0 end \
				}" );
			cmd( "pack .da.a.q.f2.s -anchor w" );
			cmd( "ttk::frame .da.a.q.f2.f" );
			cmd( "ttk::label .da.a.q.f2.f.l -text \"Name\"" );
			cmd( "ttk::entry .da.a.q.f2.f.e -width 17 -textvariable svar -justify center -state disabled" );
			cmd( "bind .da.a.q.f2.f.e <KeyRelease> { \
					if { %%N < 256 && [ info exists DaModElem ] } { \
						set bb1 [ .da.a.q.f2.f.e index insert ]; \
						set bc1 [ .da.a.q.f2.f.e get ]; \
						set bf1 [ lsearch -glob $DaModElem $bc1* ]; \
						if { $bf1  != -1 } { \
							set bd1 [ lindex $DaModElem $bf1 ]; \
							.da.a.q.f2.f.e delete 0 end; \
							.da.a.q.f2.f.e insert 0 $bd1; \
							.da.a.q.f2.f.e index $bb1; \
							.da.a.q.f2.f.e selection range $bb1 end \
						} \
					} \
				}" );
			cmd( "bind .da.a.q.f2.f.e <Return> { \
					focus .da.a.c.v.c.e; \
					.da.a.c.v.c.e selection range 0 end \
				}" );
			cmd( "pack .da.a.q.f2.f.l .da.a.q.f2.f.e -anchor w -side left" );
			cmd( "pack .da.a.q.f2.f -anchor w -padx 22" );
			cmd( "pack .da.a.q.f2 -anchor w" );
			
			cmd( "ttk::frame .da.a.c -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			cmd( "ttk::frame .da.a.c.o" );
			cmd( "ttk::radiobutton .da.a.c.o.eq -text \"Equal (=)\" -variable cond -value 1 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.dif -text \"Different (\u2260 )\" -variable cond -value 0 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.geq -text \"Larger or equal (\u2265)\" -variable cond -value 2 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.g -text \"Larger (>)\" -variable cond -value 3 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.seq -text \"Smaller or equal (\u2264)\" -variable cond -value 4 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.s -text \"Smaller (<)\" -variable cond -value 5 -state disabled" );
			cmd( "pack .da.a.c.o.eq .da.a.c.o.dif .da.a.c.o.geq .da.a.c.o.g .da.a.c.o.seq .da.a.c.o.s -anchor w" );
			cmd( "ttk::frame .da.a.c.v" );
			cmd( "ttk::frame .da.a.c.v.c" );
			cmd( "ttk::label .da.a.c.v.c.l -text \"Comparison value\"" );
			cmd( "ttk::entry .da.a.c.v.c.e -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set compvalue %%P; return 1 } { %%W delete 0 end; %%W insert 0 $compvalue; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
			cmd( "write_any .da.a.c.v.c.e $compvalue" ); 
			cmd( "bind .da.a.c.v.c.e <Return> {focus .da.a.c.v.t.e2; .da.a.c.v.t.e2 selection range 0 end }" );
			cmd( "pack .da.a.c.v.c.l .da.a.c.v.c.e" );
			cmd( "ttk::frame .da.a.c.v.t" );
			cmd( "ttk::label .da.a.c.v.t.t -text \"Case\"" );
			cmd( "ttk::entry .da.a.c.v.t.e2 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set tvar %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tvar; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
			cmd( "write_any .da.a.c.v.t.e2 $tvar" ); 
			cmd( "bind .da.a.c.v.t.e2 <Return> { focus .da.a.b.r2.ok }" );
			cmd( "pack .da.a.c.v.t.t .da.a.c.v.t.e2" );
			cmd( "pack .da.a.c.v.c .da.a.c.v.t -ipady 10" );
			cmd( "pack .da.a.c.o .da.a.c.v -anchor w -side left -ipadx 5" );
			cmd( "pack .da.a.tit .da.a.q .da.a.c -expand yes -fill x -padx 5 -pady 5" );

			cmd( "XYokhelpcancel .da.a b Description Equation { set choice 3 } { set choice 4 } { set choice 1 } { LsdHelp menudata_res.html#batch_sel } { set choice 2 }" );
			cmd( "showtop .da.a topleftW 0 0" );
			cmd( "mousewarpto .da.a.b.r2.ok" );

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
			
			if ( *choice == 3 )
			{
				cmd( "destroytop .da.a" );
				*choice = 2;	// point .da window as parent for the following window
				show_descr( ( char * ) Tcl_GetVar( inter, "b", 0 ), choice );
				break;
			}

			if ( *choice == 4 )
			{
				cmd( "destroytop .da.a" );
				*choice = 2;	// point .da window as parent for the following window
				show_eq( ( char * ) Tcl_GetVar( inter, "b", 0 ), choice );
				break;
			}

			cmd( "if { [ .da.vars.ch.f.v get 0 ] == \"\" } { set tit \"\" }" );
			cmd( "set choice $ssys" );
			
			// select all
			if ( *choice == 2 )
			{
				cmd( "set tot [ .da.vars.lb.f.v get 0 end ]" );
				cmd( "foreach i $tot { \
					if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
						insert_series .da.vars.ch.f.v \"$i\" \
					} \
				}" );
				cmd( "if { \"$tit\" == \"\" } { set tit [ .da.vars.ch.f.v get 0 ] }" );
			}
			 
			// select cases
			if ( *choice == 5 )
			{
				cmd( "if { ! [ string is integer -strict $ca1 ] } { set ca1 0 }" );
				cmd( "if { ! [ string is integer -strict $ca2 ] } { set ca2 $maxc }" );
				cmd( "set tot [ .da.vars.lb.f.v get 0 end ]" );
				cmd( "foreach i $tot { \
					if { [ lindex [ split $i ] 0 ] == \"$b\" && [ scan [ lindex [ split $i ] 2 ] \"(%%d-%%d)\" d e ] == 2 && $d >= $ca1 && $e <= $ca2 } { \
							insert_series .da.vars.ch.f.v \"$i\" \
					} \
				}" );
				cmd( "if { \"$tit\" == \"\" } { set tit [ .da.vars.ch.f.v get 0 ] }" );
			}
			 
			// select tags
			if ( *choice == 1 )
			{
				cmd( "set choice $cond" );
				i  = * choice;

				*choice = -1;
				cmd( "for { set x 0 } { $x < $ntag } { incr x } { \
						if { [ .da.a.q.f.l.e$x get ] != \"\" } { \
							set choice $x \
						} \
					}" );
					
				if ( *choice == -1 )
				{
					cmd( "set tot [ .da.vars.lb.f.v get 0 end ]" );
					cmd( "foreach i $tot { \
							if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
								insert_series .da.vars.ch.f.v \"$i\" \
							} \
						}" );
				}
				else
				{
					cmd( "set tot [ .da.vars.lb.f.v get 0 end ]" );
					cmd( "set vcell [ list ]" );
					cmd( "for { set x 0 } { $x < $ntag } { incr x } { \
							if { [ array exists vtag ] && [ info exists vtag($x) ] } { \
								lappend vcell $vtag($x) \
							} \
						}" );

					switch ( i )
					{
						case 0:
							cmd( "foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] == [ lindex [ split [ lindex [ split $i ] 1 ] {_} ] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											insert_series .da.vars.ch.f.v \"$i\" \
										} \
									} \
								}" );
							break;
						case 1:
							cmd( "foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] != [ lindex [ split [ lindex [ split $i ] 1 ] {_} ] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											insert_series .da.vars.ch.f.v \"$i\" \
										} \
									} \
								}" );
							break;
						case 2:
							cmd( "foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] > [ lindex [ split [ lindex [ split $i ] 1 ] {_} ] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											insert_series .da.vars.ch.f.v \"$i\" \
										} \
									} \
								}" );
							break;
						case 3:
							cmd( "foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] >= [ lindex [ split [ lindex [ split $i ] 1 ] {_} ] $x ] } { \
												set c 0\
											} \
										}; \
										if { $c == 1 } { \
											insert_series .da.vars.ch.f.v \"$i\" \
										} \
									} \
								}" );
							break;
						case 4:
							cmd( "foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] < [ lindex [ split [ lindex [ split $i ] 1 ] {_} ] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											insert_series .da.vars.ch.f.v \"$i\" \
										} \
									} \
								}" );
							break;
						case 5:
						   cmd( "foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] <= [ lindex [ split [ lindex [ split $i ] 1 ] {_} ] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											insert_series .da.vars.ch.f.v \"$i\" \
										} \
									} \
								}" );
					} 
				}
				
				*choice = 0;
			}

			// select by values or by values from other series
			if ( *choice == 3 || *choice == 4 )
			{
				l = *choice;
				cmd( "set choice $cond" );
				p = *choice;
				cmd( "set tot [ .da.vars.lb.f.v get 0 end ]" );
				cmd( "set choice [ llength $tot ]" );
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
						if ( is_finite( datum[ h - l ] ) )		// ignore NaNs
						{
							r = 0;
							switch ( p )
							{
								case 0: 
									if ( datum[ h - l ] != compvalue )
										r = 1;
									break;
								case 1: 
									if ( datum[ h - l ] == compvalue )
										r = 1;
									break;
								case 2: 
									if ( datum[ h - l ] >= compvalue )
										r = 1;
									break;
								case 3: 
									if ( datum[ h - l ] > compvalue )
										r = 1;
									break;
								case 4: 
									if ( datum[ h - l ] <= compvalue )
										r = 1;
									break;
								case 5: 
									if ( datum[ h - l ] < compvalue )
										r = 1;
									break;
							}
							
							if ( r == 1 )
								cmd( "insert_series .da.vars.ch.f.v $res" );
						}
					}
				}
			}

			cmd( "destroytop .da.a" );

			cmd( "if { \"$tit\" == \"\" } { set tit [ .da.vars.ch.f.v get 0 ] }" );

			break;


		// Use right button of the mouse to remove series selected with different criteria
		case 33:
			compvalue = 0;
			Tcl_LinkVar( inter, "compvalue", ( char * ) &compvalue, TCL_LINK_DOUBLE );
			cmd( "set a [ split $res ]" );
			cmd( "set b [ lindex $a 0 ]" );
			cmd( "set c [ lindex $a 1 ]" ); //get the tag value
			cmd( "set ntag [ llength [ split $c {_} ] ]" );
			cmd( "set ssys 2" );
			cmd( "if { ! [ info exist ca1 ] || ! [ string is integer -strict $ca1 ] } { set ca1 0 }" );
			cmd( "if { ! [ info exist ca2 ] || ! [ string is integer -strict $ca2 ] } { set ca2 $maxc }" );
			cmd( "if { ! [ info exist tvar ] || ! [ string is integer -strict $tvar ] } { set tvar $maxc }" );
			cmd( "if { ! [ info exist cond ] } { set cond 1 }" );
			cmd( "if { ! [ info exist selOnly ] } { set selOnly 0 }" );

			cmd( "newtop .da.a \"Unselect Series\" { set choice 2 } .da" );

			cmd( "ttk::frame .da.a.tit" );
			cmd( "ttk::label .da.a.tit.l -text \"Unselect series with name\"" );
			cmd( "ttk::label .da.a.tit.s -text \"$b\" -style hl.TLabel" );
			cmd( "pack .da.a.tit.l .da.a.tit.s" );
			cmd( "ttk::frame .da.a.q -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			
			// unselect all
			cmd( "ttk::frame .da.a.q.f1" );
			cmd( "ttk::radiobutton .da.a.q.f1.c -text \"Unselect all\" -variable ssys -value 2 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state disabled \
					}; \
					.da.a.q.f2.f.e conf -state disabled; \
					.da.a.q.f4.l.e1 conf -state disabled; \
					.da.a.q.f4.l.e2 conf -state disabled; \
					.da.a.c.v.t.e2 conf -state disabled; \
					.da.a.c.v.c.e conf -state disabled; \
					.da.a.c.o.eq conf -state disabled; \
					.da.a.c.o.dif conf -state disabled; \
					.da.a.c.o.geq conf -state disabled; \
					.da.a.c.o.g conf -state disabled; \
					.da.a.c.o.seq conf -state disabled; \
					.da.a.c.o.s conf -state disabled \
				}" );
			cmd( "bind .da.a.q.f1.c <Return> { \
					.da.a.q.f1.c invoke; \
					focus .da.a.b.r2.ok \
				}" );
			cmd( "bind .da.a.q.f1.c <Down> { \
					focus .da.a.q.f.c; \
					.da.a.q.f.c invoke \
				}" );
			cmd( "pack .da.a.q.f1.c" );
			cmd( "pack .da.a.q.f1 -anchor w" );
			
			// unselect tags
			cmd( "ttk::frame .da.a.q.f" );
			cmd( "ttk::radiobutton .da.a.q.f.c -text \"Unselect by series tags\" -variable ssys -value 1 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state normal \
					}; \
					.da.a.q.f2.f.e conf -state disabled; \
					.da.a.q.f4.l.e1 conf -state disabled; \
					.da.a.q.f4.l.e2 conf -state disabled; \
					.da.a.c.v.t.e2 conf -state disabled; \
					.da.a.c.v.c.e conf -state disabled; \
					.da.a.c.o.eq conf -state normal; \
					.da.a.c.o.dif conf -state normal; \
					.da.a.c.o.geq conf -state normal; \
					.da.a.c.o.g conf -state normal; \
					.da.a.c.o.seq conf -state normal; \
					.da.a.c.o.s conf -state normal \
				}" );
			cmd( "bind .da.a.q.f.c <Up> { \
					focus .da.a.q.f1.c; \
					.da.a.q.f1.c invoke \
				}" );
			cmd( "bind .da.a.q.f.c <Return> { \
					focus .da.a.q.f.l.e0; \
					.da.a.q.f.l.e0 selection range 0 end \
				}" );
			cmd( "bind .da.a.q.f.c <Down> { \
					focus .da.a.q.f4.c; \
					.da.a.q.f4.c invoke \
				}" );
			cmd( "pack .da.a.q.f.c -anchor w" );
			cmd( "ttk::frame .da.a.q.f.l" );
			cmd( "for { set x 0 } { $x < $ntag } { incr x } { \
					if { $x > 0 } { \
						ttk::label .da.a.q.f.l.s$x -text \u2014 \
					}; \
					ttk::entry .da.a.q.f.l.e$x -width 4 -textvariable vtag($x) -justify center -state disabled \
				}" );
			cmd( "for { set x 0 } { $x < $ntag } { incr x } { \
					if { $x > 0 } { \
						pack .da.a.q.f.l.s$x -padx 2 -side left \
					}; \
					pack .da.a.q.f.l.e$x -side left; \
					bind .da.a.q.f.l.e$x <Return> [ subst -nocommand { focus .da.a.q.f.l.e[ expr { $x + 1 } ]; \
					.da.a.q.f.l.e[ expr { $x + 1 } ] selection range 0 end } ]; \
					bind .da.a.q.f.l.e$x <KeyRelease> { .da.a.q.f.c invoke } }; \
					incr x -1; \
					bind .da.a.q.f.l.e$x <Return> { focus .da.a.b.r2.ok }" );
			cmd( "pack .da.a.q.f.l -anchor w -padx 25" );
			cmd( "pack .da.a.q.f -anchor w" );
		
			// unselect cases
			cmd( "ttk::frame .da.a.q.f4" );
			cmd( "ttk::radiobutton .da.a.q.f4.c -text \"Unselect by series cases\" -variable ssys -value 5 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state disabled \
					}; \
					.da.a.q.f2.f.e conf -state disabled; \
					.da.a.q.f4.l.e1 conf -state normal; \
					.da.a.q.f4.l.e2 conf -state normal; \
					.da.a.c.v.t.e2 conf -state disabled; \
					.da.a.c.v.c.e conf -state disabled; \
					.da.a.c.o.eq conf -state disabled; \
					.da.a.c.o.dif conf -state disabled; \
					.da.a.c.o.geq conf -state disabled; \
					.da.a.c.o.g conf -state disabled; \
					.da.a.c.o.seq conf -state disabled; \
					.da.a.c.o.s conf -state disabled \
				}" );
			cmd( "bind .da.a.q.f4.c <Up> { \
					focus .da.a.q.f.c; \
					.da.a.q.f.c invoke \
				}" );
			cmd( "bind .da.a.q.f4.c <Return> { \
					.da.a.q.f4.c invoke; \
					focus .da.a.b.r2.ok \
				}" );
			cmd( "bind .da.a.q.f4.c <Down> { \
					focus .da.a.q.f3.s; \
					.da.a.q.f3.s invoke \
				}" );
			cmd( "pack .da.a.q.f4.c -anchor w" );
			
			cmd( "ttk::frame .da.a.q.f4.l" );
			cmd( "ttk::entry .da.a.q.f4.l.e1 -width 5 -textvariable ca1 -justify center -state disabled" );
			cmd( "ttk::label .da.a.q.f4.l.s -text to" );
			cmd( "ttk::entry .da.a.q.f4.l.e2 -width 5 -textvariable ca2 -justify center -state disabled" );
			cmd( "pack .da.a.q.f4.l.e1 .da.a.q.f4.l.s .da.a.q.f4.l.e2 -padx 2 -side left" );
			cmd( "bind .da.a.q.f4.l.e1 <Return> { \
					focus .da.a.q.f4.l.e2; \
					.da.a.q.f4.l.e2 selection range 0 end \
				}" );
			cmd( "bind .da.a.q.f4.l.e2 <Return> { focus .da.a.b.r2.ok }" );
			cmd( "pack .da.a.q.f4.l -anchor w -padx 25" );
			
			cmd( "pack .da.a.q.f4 -anchor w" );
			
			// unselect by values
			cmd( "ttk::frame .da.a.q.f3" );
			cmd( "ttk::radiobutton .da.a.q.f3.s -text \"Unselect by series values\" -variable ssys -value 3 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state disabled \
					}; \
					.da.a.q.f2.f.e conf -state disabled; \
					.da.a.q.f4.l.e1 conf -state disabled; \
					.da.a.q.f4.l.e2 conf -state disabled; \
					.da.a.c.v.t.e2 conf -state normal; \
					.da.a.c.v.c.e conf -state normal; \
					.da.a.c.o.eq conf -state normal; \
					.da.a.c.o.dif conf -state normal; \
					.da.a.c.o.geq conf -state normal; \
					.da.a.c.o.g conf -state normal; \
					.da.a.c.o.seq conf -state normal; \
					.da.a.c.o.s conf -state normal \
				}" );
			cmd( "bind .da.a.q.f3.s <Up> { \
					focus .da.a.q.f4.c; \
					.da.a.q.f4.c invoke \
				}" );
			cmd( "bind .da.a.q.f3.s <Return> { \
					focus .da.a.c.v.c.e; \
					.da.a.c.v.c.e selection range 0 end \
				}" );
			cmd( "bind .da.a.q.f3.s <Down> { \
					focus .da.a.q.f2.s; \
					.da.a.q.f2.s invoke \
				}" );
			cmd( "pack .da.a.q.f3.s -anchor w" );
			cmd( "pack .da.a.q.f3 -anchor w" );
			
			// unselect by values from other series
			cmd( "ttk::frame .da.a.q.f2" );
			cmd( "ttk::radiobutton .da.a.q.f2.s -text \"Unselect by values from another series\" -variable ssys -value 4 -command { \
					for { set x 0 } { $x < $ntag } { incr x } { \
						.da.a.q.f.l.e$x conf -state disabled \
					}; \
					.da.a.q.f2.f.e conf -state normal; \
					.da.a.q.f4.l.e1 conf -state disabled; \
					.da.a.q.f4.l.e2 conf -state disabled; \
					.da.a.c.v.t.e2 conf -state normal; \
					.da.a.c.v.c.e conf -state normal; \
					.da.a.c.o.eq conf -state normal; \
					.da.a.c.o.dif conf -state normal; \
					.da.a.c.o.geq conf -state normal; \
					.da.a.c.o.g conf -state normal; \
					.da.a.c.o.seq conf -state normal; \
					.da.a.c.o.s conf -state normal \
				}" );
			cmd( "bind .da.a.q.f2.s <Up> { \
					focus .da.a.q.f3.s; \
					.da.a.q.f3.s invoke \
				}" );
			cmd( "bind .da.a.q.f2.s <Return> { \
					focus .da.a.q.f2.f.e; \
					.da.a.q.f2.f.e selection range 0 end \
				}" );
			cmd( "pack .da.a.q.f2.s -anchor w" );
			cmd( "ttk::frame .da.a.q.f2.f" );
			cmd( "ttk::label .da.a.q.f2.f.l -text \"Name\"" );
			cmd( "ttk::entry .da.a.q.f2.f.e -width 17 -textvariable svar -justify center -state disabled" );
			cmd( "bind .da.a.q.f2.f.e <KeyRelease> { \
					if { %%N < 256 && [ info exists DaModElem ] } { \
						set bb1 [ .da.a.q.f2.f.e index insert ]; \
						set bc1 [ .da.a.q.f2.f.e get ]; \
						set bf1 [ lsearch -glob $DaModElem $bc1* ]; \
						if { $bf1  != -1 } { \
							set bd1 [ lindex $DaModElem $bf1 ]; \
							.da.a.q.f2.f.e delete 0 end; \
							.da.a.q.f2.f.e insert 0 $bd1; \
							.da.a.q.f2.f.e index $bb1; \
							.da.a.q.f2.f.e selection range $bb1 end \
						} \
					} \
				}" );
			cmd( "bind .da.a.q.f2.f.e <Return> { \
					focus .da.a.c.v.c.e; \
					.da.a.c.v.c.e selection range 0 end \
				}" );
			cmd( "pack .da.a.q.f2.f.l .da.a.q.f2.f.e -anchor w -side left" );
			cmd( "pack .da.a.q.f2.f -anchor w -padx 22" );
			cmd( "pack .da.a.q.f2 -anchor w" );
			cmd( "ttk::frame .da.a.c -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			cmd( "ttk::frame .da.a.c.o" );
			cmd( "ttk::radiobutton .da.a.c.o.eq -text \"Equal (=)\" -variable cond -value 1 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.dif -text \"Different (\u2260 )\" -variable cond -value 0 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.geq -text \"Larger or equal (\u2265)\" -variable cond -value 2 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.g -text \"Larger (>)\" -variable cond -value 3 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.seq -text \"Smaller or equal (\u2264)\" -variable cond -value 4 -state disabled" );
			cmd( "ttk::radiobutton .da.a.c.o.s -text \"Smaller (<)\" -variable cond -value 5 -state disabled" );
			cmd( "pack .da.a.c.o.eq .da.a.c.o.dif .da.a.c.o.geq .da.a.c.o.g .da.a.c.o.seq .da.a.c.o.s -anchor w" );
			cmd( "ttk::frame .da.a.c.v" );
			cmd( "ttk::frame .da.a.c.v.c" );
			cmd( "ttk::label .da.a.c.v.c.l -text \"Comparison value\"" );
			cmd( "ttk::entry .da.a.c.v.c.e -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set compvalue %%P; return 1 } { %%W delete 0 end; %%W insert 0 $compvalue; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
			cmd( "write_any .da.a.c.v.c.e $compvalue" ); 
			cmd( "bind .da.a.c.v.c.e <Return> {focus .da.a.c.v.t.e2; .da.a.c.v.t.e2 selection range 0 end }" );
			cmd( "pack .da.a.c.v.c.l .da.a.c.v.c.e" );
			cmd( "ttk::frame .da.a.c.v.t" );
			cmd( "ttk::label .da.a.c.v.t.t -text \"Case\"" );
			cmd( "ttk::entry .da.a.c.v.t.e2 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set tvar %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tvar; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
			cmd( "write_any .da.a.c.v.t.e2 $tvar" ); 
			cmd( "bind .da.a.c.v.t.e2 <Return> { focus .da.a.b.r2.ok }" );
			cmd( "pack .da.a.c.v.t.t .da.a.c.v.t.e2" );
			cmd( "pack .da.a.c.v.c .da.a.c.v.t -ipady 10" );
			cmd( "pack .da.a.c.o .da.a.c.v -anchor w -side left -ipadx 5" );
			cmd( "ttk::frame .da.a.s" );
			cmd( "ttk::checkbutton .da.a.s.b -text \"Only mark items\" -variable selOnly" );
			cmd( "pack .da.a.s.b" );
			cmd( "pack .da.a.tit .da.a.q .da.a.c .da.a.s -expand yes -fill x -padx 5 -pady 5" );

			cmd( "XYokhelpcancel .da.a b Description Equation { set choice 3 } { set choice 4 } { set choice 1 } { LsdHelp menudata_res.html#batch_sel } { set choice 2 }" );
			cmd( "showtop .da.a topleftW 0 0" );
			cmd( "mousewarpto .da.a.b.r2.ok" );

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

			if ( *choice == 3 )
			{
				cmd( "destroytop .da.a" );
				*choice = 2;	// point .da window as parent for the following window
				show_descr( ( char * ) Tcl_GetVar( inter, "b", 0 ), choice );
				break;
			}

			if ( *choice == 4 )
			{
				cmd( "destroytop .da.a" );
				*choice = 2;	// point .da window as parent for the following window
				show_eq( ( char * ) Tcl_GetVar( inter, "b", 0 ), choice );
				break;
			}

			cmd( ".da.vars.ch.f.v selection clear 0 end" );
			cmd( "set choice $ssys" );

			// unselect all
			if ( *choice == 2 )
			{
				 cmd( "set tot [ .da.vars.ch.f.v get 0 end ]" );
				 cmd( "set myc 0" );
				 cmd( "foreach i $tot { \
						if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
							.da.vars.ch.f.v selection set $myc \
						}; \
						incr myc \
					}" );
			}
			 
			// unselect cases
			if ( *choice == 5 )
			{
				cmd( "if { ! [ string is integer -strict $ca1 ] } { set ca1 0 }" );
				cmd( "if { ! [ string is integer -strict $ca2 ] } { set ca2 $maxc }" );
				 cmd( "set tot [ .da.vars.ch.f.v get 0 end ]" );
				 cmd( "set myc 0" );
				 cmd( "foreach i $tot { \
					if { [ lindex [ split $i ] 0 ] == \"$b\" && [ scan [ lindex [ split $i ] 2 ] \"(%%d-%%d)\" d e ] == 2 && $d >= $ca1 && $e <= $ca2 } { \
						.da.vars.ch.f.v selection set $myc \
					}; \
					incr myc \
				}" );
			}
			 
			// unselect tags
			if ( *choice == 1 )
			{
				cmd( "set choice $cond" );
				i = *choice;

				*choice = -1;
				cmd( "for { set x 0 } { $x < $ntag } { incr x } { \
						if { [ .da.a.q.f.l.e$x get ] != \"\" } { \
							set choice $x \
						} \
					}" );
					
				if ( *choice == -1 )
				{
					cmd( "set tot [ .da.vars.ch.f.v get 0 end ]" );
					cmd( "set myc 0" ); 
					cmd( "foreach i $tot { \
							if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
								.da.vars.ch.f.v selection set $myc \
							}; \
							incr myc \
						}" );
				}
				else
				{
					cmd( "set tot [ .da.vars.ch.f.v get 0 end ]" );
					cmd( "set vcell [ list ]" );
					cmd( "set choice $ntag" );
					cmd( "for { set x 0 } { $x < $ntag } { incr x } { \
							if { [ array exists vtag ] && [ info exists vtag($x) ] } { \
								lappend vcell $vtag($x) \
							} \
						}" );

					switch ( i )
					{
						case 0:
							cmd( "set myc 0; \
								  foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] == [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											.da.vars.ch.f.v selection set $myc \
										} \
									}; \
									incr myc \
								}" );  
							break;
						case 1:
							cmd( "set myc 0; \
								  foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] != [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											.da.vars.ch.f.v selection set $myc \
										} \
									}; \
									incr myc \
								}" );  
							break;
						case 2:
							cmd( "set myc 0; \
								  foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] > [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											.da.vars.ch.f.v selection set $myc \
										} \
									}; \
									incr myc \
								}" );  
							break;
						case 3:
							cmd( "set myc 0; \
								  foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] >= [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											.da.vars.ch.f.v selection set $myc \
										} \
									}; \
									incr myc \
								}" );  
							break;
						case 4:
							cmd( "set myc 0; \
								  foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] < [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											.da.vars.ch.f.v selection set $myc \
										} \
									}; \
									incr myc \
								}" );  
							break;
						case 5:
							cmd( "set myc 0; \
								  foreach i $tot { \
									if { [ lindex [ split $i ] 0 ] == \"$b\" } { \
										set c 1; \
										for { set x 0 } { $x < $ntag } { incr x } { \
											if { [ lindex $vcell $x ] != \"\" && [ lindex $vcell $x ] <= [ lindex [ split [ lindex [ split $i ] 1 ] {_}] $x ] } { \
												set c 0 \
											} \
										}; \
										if { $c == 1 } { \
											.da.vars.ch.f.v selection set $myc \
										} \
									}; \
									incr myc \
								}" );  
					} 
				}
				
				*choice = 0;
			}

			// unselect by values or by values from other series
			if ( *choice == 3 || *choice == 4 )
			{
				l = *choice;
				cmd( "set choice $cond" );
				p = *choice;
				cmd( "set tot [ .da.vars.ch.f.v get 0 end ]" );
				cmd( "set choice [ llength $tot ]" );
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
						if ( is_finite( datum[ h - l ] ) )		// ignore NaNs
						{
							r = 0;
							switch ( p )
							{
								case 0: 
									if ( datum[ h - l ] != compvalue )
										r = 1;
									break;
								case 1: 
									if ( datum[ h - l ] == compvalue )
										r = 1;
									break;
								case 2: 
									if ( datum[ h - l ] >= compvalue )
										r = 1;
									break;
								case 3: 
									if ( datum[ h - l ] > compvalue )
										r = 1;
									break;
								case 4: 
									if ( datum[ h - l ] <= compvalue )
										r = 1;
									break;
								case 5: 
									if ( datum[ h - l ] < compvalue )
										r = 1;
									break;
							}
							if ( r == 1 )
								cmd( ".da.vars.ch.f.v selection set %d", i );
						}
					}
				}
			}

			cmd( "destroytop .da.a" );

			cmd( "if { ! $selOnly } { \
					set steps 0; \
					foreach i [ .da.vars.ch.f.v curselection ] { \
						.da.vars.ch.f.v delete [ expr { $i - $steps } ]; \
						incr steps \
					} \
				} { \
					if { [ llength [ .da.vars.ch.f.v curselection ] ] > 0 } { \
						.da.vars.ch.f.v see [ lindex [ .da.vars.ch.f.v curselection ] 0 ] \
					} \
				}" );

			break;


		// PLOTS LIST CONTEXT ACTIONS

		// remove a plot
		case 20:
			cmd( "set answer [ ttk::messageBox -parent .da -type yesno -title Confirmation -message \"Delete plot?\" -detail \"Press 'Yes' to delete plot:\\n$tit\" -icon question -default yes ]" );
			cmd( "if { [ string compare $answer yes ] == 0 } { set choice 1 } { set choice 0 }" );
			
			if ( *choice == 0 )
				break;
			
			cmd( "scan $it %%d) a" );
			cmd( "if [ winfo exists $daptab.tab$a ] { \
					if [ istoplevel $daptab.tab$a ] { \
						destroytop $daptab.tab$a \
					} else { \
						if { \"$daptab.tab$a\" in [ $daptab tabs ] } { \
							$daptab forget $daptab.tab$a; \
						}; \
						destroy $daptab.tab$a; \
						if { [ $daptab index end ] == 0 } { \
							wm withdraw [ winfo toplevel $daptab ] \
						} \
					} \
				}" );
			cmd( ".da.vars.pl.f.v delete $n_it" );
				
			update_more_tab( ".dap" );

			break;


		// Raise the clicked plot
		case 3:
			cmd( "scan $it %%d) a" );
			cmd( "if [ winfo exists $daptab.tab$a ] { \
					if [ istoplevel $daptab.tab$a ] { \
						focustop $daptab.tab$a \
					} else { \
						if { \"$daptab.tab$a\" ni [ $daptab tabs ] } { \
							$daptab forget 1; \
							$daptab add $daptab.tab$a -text [ string range [ lindex [ split $it ] 1 ] 0 %d ] \
						}; \
						$daptab select $daptab.tab$a; \
						focustop $daptab; \
						set choice 0 \
					} \
				} else { \
					set choice 1 \
				}", MAX_TAB_LEN - 1 );
				
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

		// sort
		case 5:
			cmd( "sort_series .da.vars.lb.f.v increasing" );
			break;
		  
		  
		// sort (descending order)
		case 38:
			cmd( "sort_series .da.vars.lb.f.v decreasing" );
			break;
		 

		// sort the selection in selected series list in inverse order
		case 34:
			cmd( "sort_series .da.vars.ch.f.v reverse" );
			break;

		   
		// unsort
		case 14:
			cmd( "sort_series .da.vars.lb.f.v none" );
			break;

		   
		// sort last updated on end
		case 15:
			cmd( "sort_series .da.vars.lb.f.v nice" );
			break;

		   
		// find first instance of series in the available series listbox
		case 39:
			cmd( "set choice [ llength $DaModElem ]" );
			if ( *choice == 0 )
				break;

			cmd( "set srchTxt \"\"; set srchInst 1" );
			cmd( "newtop .da.a \"Find Series\" { set choice 2 } .da" );
			
			cmd( "ttk::frame .da.a.v" );
			cmd( "ttk::label .da.a.v.l -text \"Series name (or part)\"" );
			cmd( "ttk::entry .da.a.v.e -textvariable srchTxt -width 20 -justify center" );
			cmd( "ttk::label .da.a.v.n -justify center -text \"(finds first instance only,\nuse 'F3' or 'Ctrl+N' to find others)\"" );

			cmd( "pack .da.a.v.l .da.a.v.e .da.a.v.n" );
			cmd( "pack .da.a.v -pady 5 -padx 5" );

			cmd( "bind .da.a.v.e <Return> { set choice 1 }" );
			cmd( "bind .da.a.v.e <Escape> { set choice 2 }" );
			cmd( "bind .da.a.v.e <KeyRelease> { \
					if { %%N < 256 && [ info exists DaModElem ] } { \
						set bb1 [ .da.a.v.e index insert ]; \
						set bc1 [ .da.a.v.e get ]; \
						set bf1 [ lsearch -glob $DaModElem $bc1* ]; \
						if { $bf1  != -1 } { \
							set bd1 [ lindex $DaModElem $bf1 ]; \
							.da.a.v.e delete 0 end; \
							.da.a.v.e insert 0 $bd1; \
							.da.a.v.e index $bb1; \
							.da.a.v.e selection range $bb1 end \
						} \
					} \
				}" );

			cmd( "okhelpcancel .da.a b  { set choice 1 } { LsdHelp menudata_res.html#find } { set choice 2 }" );
			cmd( "showtop .da.a" );
			cmd( "focus .da.a.v.e" );

			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "destroytop .da.a" );

			if ( *choice == 1 )
			{
				cmd( "set choice [ string length $srchTxt ]" );
				if ( *choice == 0 )
					break;
				
				cmd( "set choice [ search_series $srchTxt ]" );
				
				if ( *choice == 0 )
					cmd( "ttk::messageBox -parent .da -type ok -title \"Error\" -icon error -default ok -message \"Series not found\" -detail \"Check if the name was spelled correctly or use just part of the name. This command is case insensitive.\"" );
			}

			break;
		  
		  
		// find subsequent instances of a series in the available series listbox
		case 40:
			cmd( "set choice [ search_series ]" );

			if ( *choice == 0 )
				cmd( "ttk::messageBox -parent .da -type ok -title \"Warning\" -icon warning -default ok -message \"Series not found\" -detail \"No additional instance of series found.\"" );

			break;

		  
		// insert the variables selected in the list of the variables to plot
		case 6:
			cmd( "set a [ .da.vars.lb.f.v curselection ]" );
			cmd( "foreach i $a { insert_series .da.vars.ch.f.v \"[ .da.vars.lb.f.v get $i ]\" }" );
			cmd( "set tit [ .da.vars.ch.f.v get 0 ]" );

			break;

		  
		// remove the vars. selected from the variables to plot
		case 7:
			cmd( "set steps 0" );
			cmd( "foreach i [ .da.vars.ch.f.v curselection ] { .da.vars.ch.f.v delete [ expr { $i - $steps } ]; incr steps }" );
			cmd( "if { [ .da.vars.ch.f.v size ] == 0 } { set tit \"\" } { set tit [ .da.vars.ch.f.v get 0 ] }" );
			
			break;

		  
		// remove all the variables from the list of vars to plot
		case 8:
			cmd( ".da.vars.ch.f.v delete 0 end" );
			cmd( "set tit \"\"" );

			break;
			

		// add existing variables (no saved)
		case 45: 
			if ( *choice == 45 )
				cmd( "set bidi 0" );
			
		// insert MC series from disk
		case 25: 
			if ( *choice == 25 )
				cmd( "set bidi 3" );
			
		// add new series from existing ones
		case 46:
			if ( *choice == 46 )
				cmd( "set bidi 4" );
			
		// add moving average series from existing ones
		case 47: 
			if ( *choice == 47 )
				cmd( "set bidi 5" );
			
		// insert new series ( from disk or combining existing series).
		case 24:
			if ( *choice == 24 )
			{
				if ( num_var > 0 )
				{
					cmd( "newtop .da.s \"Choose Data Source\" { set choice 2 } .da" );
					cmd( "ttk::label .da.s.l -text \"Source of additional series\"" );

					cmd( "set bidi 4" );
					cmd( "ttk::frame .da.s.i -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
					cmd( "ttk::radiobutton .da.s.i.c -text \"Create new series from selected\" -underline 0 -variable bidi -value 4" );
					cmd( "ttk::radiobutton .da.s.i.a -text \"Moving average series from selected\" -underline 0 -variable bidi -value 5" );
					cmd( "ttk::radiobutton .da.s.i.e -text \"Existing unsaved element\" -underline 0 -variable bidi -value 0" );
					cmd( "ttk::radiobutton .da.s.i.f -text \"File(s) of saved results\" -underline 0 -variable bidi -value 1" );
					cmd( "pack .da.s.i.c .da.s.i.a .da.s.i.e .da.s.i.f -anchor w" );
					
					cmd( "bind .da.s <KeyPress-c> { set bidi 4 }; bind .da.s <KeyPress-C> { set bidi 4 }" );
					cmd( "bind .da.s <KeyPress-m> { set bidi 5 }; bind .da.s <KeyPress-M> { set bidi 5 }" );
					cmd( "bind .da.s <KeyPress-e> { set bidi 0 }; bind .da.s <KeyPress-E> { set bidi 0 }" );
					cmd( "bind .da.s <KeyPress-f> { set bidi 1 }; bind .da.s <KeyPress-F> { set bidi 1 }" );

					cmd( "pack .da.s.l .da.s.i -expand yes -fill x -pady 5 -padx 5" );
					
					if ( nv == 0 )
					{
						cmd( ".da.s.i.c configure -state disabled" );
						cmd( ".da.s.i.a configure -state disabled" );
						cmd( "set bidi 0" );
					}
				}
				else
				{
					cmd( "newtop .da.s \"Choose Data Source\" { set choice 2 } .da" );
					cmd( "ttk::label .da.s.l -text \"Source of additional series\"" );

					cmd( "set bidi 1" );
					cmd( "ttk::frame .da.s.i -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
					cmd( "ttk::radiobutton .da.s.i.f -text \"File(s) of saved results\" -variable bidi -value 1" );
					cmd( "ttk::radiobutton .da.s.i.m -text \"Files from Monte Carlo experiment\" -variable bidi -value 3" );
					cmd( "pack .da.s.i.f .da.s.i.m -anchor w" );

					cmd( "bind .da.s <KeyPress-f> { set bidi 1 }; bind .da.s <KeyPress-F> { set bidi 1 }" );
					cmd( "bind .da.s <KeyPress-m> { set bidi 3 }; bind .da.s <KeyPress-M> { set bidi 3 }" );

					cmd( "pack .da.s.l .da.s.i -expand yes -fill x -pady 5 -padx 5" );
				}

				cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#add_series } { set choice 2 }" );
				cmd( "showtop .da.s" );
				cmd( "mousewarpto .da.s.b.ok" );

				*choice = 0;
				while ( *choice == 0 )
				  Tcl_DoOneEvent( 0 );

				cmd( "destroytop .da.s" );

				if ( *choice == 2 )
					break;
			}
			
			// process the proper case
			cmd( "set choice $bidi" );
			switch ( *choice )
			{
				case 4:
					if ( create_series( choice, false, cur_var ) )
						cmd( "selectinlist .da.vars.lb.f.v end" );
					
					break;

				case 5:
					if ( create_maverag( choice ) )
						cmd( "selectinlist .da.vars.lb.f.v end" );
					
					break;
				
				case 0:
					if ( add_unsaved( choice ) )
						cmd( "selectinlist .da.vars.lb.f.v end" );
					
					break;
					
				case 3:
					mc = true;
					
					if ( logs )
						cmd( "ttk::messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

					cmd( "set confi 95" );
					cmd( "set bidi 1" );
					cmd( "set keepSeries 0" );

					cmd( "newtop .da.s \"Monte Carlo Series Options\" { set choice 2 } .da" );

					cmd( "ttk::frame .da.s.i" );
					cmd( "ttk::label .da.s.i.l -text \"Mode\"" );

					cmd( "ttk::frame .da.s.i.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
					cmd( "ttk::radiobutton .da.s.i.r.m -text \"Average only\" -variable bidi -value 1 -command { .da.s.ci.p configure -state disabled }" );
					cmd( "ttk::radiobutton .da.s.i.r.z -text \"Maximum and minimum\" -variable bidi -value 13 -command { .da.s.ci.p configure -state disabled }" );
					cmd( "ttk::radiobutton .da.s.i.r.x -text \"Average, maximum and minimum\" -variable bidi -value 15 -command { .da.s.ci.p configure -state disabled }" );
					cmd( "ttk::radiobutton .da.s.i.r.i -text \"Confidence interval\" -variable bidi -value 11 -command { .da.s.ci.p configure -state normal }" );
					cmd( "ttk::radiobutton .da.s.i.r.n -text \"Average and confidence interval\" -variable bidi -value 6 -command { .da.s.ci.p configure -state normal }" );
					cmd( "ttk::radiobutton .da.s.i.r.a -text \"All the above\" -variable bidi -value 16 -command { .da.s.ci.p configure -state normal }" );

					cmd( "pack .da.s.i.r.m .da.s.i.r.z .da.s.i.r.x .da.s.i.r.i .da.s.i.r.n .da.s.i.r.a -anchor w" );
					cmd( "pack .da.s.i.l .da.s.i.r" );

					cmd( "set a [ .da.vars.ch.f.v get 0 ]" );
					cmd( "set basename [ lindex [ split $a ] 0 ]" );
					cmd( "set tailname \"_avg\"" );
					cmd( "set vname $basename$tailname" );

					cmd( "ttk::frame .da.s.ci" );
					cmd( "ttk::label .da.s.ci.l -text \"Confidence level (%%)\"" );
					cmd( "ttk::entry .da.s.ci.p -width 3 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 80 && $n <= 99 } { set confi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $confi; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
					cmd( "write_disabled .da.s.ci.p $confi" ); 
					cmd( "pack .da.s.ci.l .da.s.ci.p" );
					
					cmd( "ttk::frame .da.s.s" );
					cmd( "ttk::checkbutton .da.s.s.k -text \"Keep original series\" -variable keepSeries" );
					cmd( "pack .da.s.s.k" );

					cmd( "pack .da.s.i .da.s.ci .da.s.s -padx 5 -pady 5" );

					cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#createmc } { set choice 2 }" );

					cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
					cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

					cmd( "showtop .da.s" );
					cmd( "mousewarpto .da.s.b.ok" );
					 
					*choice = 0;
					while ( *choice == 0 )
						Tcl_DoOneEvent( 0 );

					cmd( "if [ string is integer [ .da.s.ci.p get ] ] { set confi [ .da.s.ci.p get ] }" ); 
					cmd( "destroytop .da.s" );

					Tcl_UnlinkVar( inter, "confi" );

					if ( *choice == 2 )
						goto add_end;
					
				case 1:
					gz = false;
					const char extRes[ ] = ".res .res.gz";
					const char extTot[ ] = ".tot .tot.gz";

					// make sure there is a path set
					cmd( "set path \"%s\"", path );
					if ( strlen( path ) > 0 )
						cmd( "cd \"$path\"" );
				
					cmd( "set lab [ tk_getOpenFile -parent .da -title \"Load Results File%s\" -multiple yes -initialdir \"$path\" -filetypes {{{LSD result files} {%s}} {{LSD total files} {%s}} {{All files} {*}}} ]", mc ? "s" : "(s)", extRes, extTot );
					cmd( "if { ! [ fn_spaces \"$lab\" .da 1 ] } { set choice [ llength $lab ] } { set choice 0 }" );
					h = *choice;		// number of files
					
					if ( h == 0 )
						goto add_end; 	// no file selected
					
					if ( mc && h == 1 )
					{
						cmd( "ttk::messageBox -parent .da -type ok -icon error -title Error -message \"Invalid number of results files\" -detail \"Monte Carlo experiment requires two or more files. Please adjust the number of simulation runs properly and regenerate the files.\"" );
						plog( "\nError: invalid number of files\n" );
						goto add_end;
					}
				
					var_names.resize( h );
					
					if ( mc )
						k = get_int( "keepSeries" );
					else
						k = true;
				
					if ( h > 1 )
						cmd( "progressbox .da.pas \"Add Series\" \"Loading results files\" \"File\" %d { set stop true } .da \"Case\"", h );
					else
						cmd( "progressbox .da.pas \"Add Series\" \"Loading series\" \"\" 1 { set stop true } .da \"Case\"" );
					
					for ( i = 0, stop = false; i < h && ! stop; ++i )  
					{
						cmd( "set datafile [ lindex $lab %d ]", i );
						app = ( char * ) Tcl_GetVar( inter, "datafile", 0 );
						strcpy( filename, app );
						if ( strlen( filename ) > 3 && ! strcmp( &filename[ strlen( filename ) - 3 ], ".gz" ) )
							gz = true;

						f = fopen( filename, "r" );
					
						if ( f != NULL )
						{
							fclose( f );
							++file_counter;
							insert_data_file( gz, &num_var, &var_names[ i ], k );
							
							if ( h > 1 )
								cmd( "prgboxupdate .da.pas %d", i + 1 );
						}
						else
							plog( "\nError: could not open file: %s\n", "", filename );
					}
					
					cmd( "destroytop .da.pas" );
					
					if ( ! mc )
						goto add_clear;
					
					if ( stop )
					{
						delete [ ] vs;
						vs = NULL;
						num_var = max_c = file_counter = 0;
						goto add_clear;
					}
						
					plog( "\nCreating MC series... " );
					
					m = num_var;
					l = var_names[ 0 ].size( );	// number of series
					
					for ( i = 1; i < h; ++i )
					{
						if ( var_names[ i ].size( ) != ( unsigned ) l )
						{
							cmd( "ttk::messageBox -parent .da -type ok -icon error -title Error -message \"Invalid results files\" -detail \"The number of series in the files are not the same. Results files should come from the same set of simulation runs.\"" );
							plog( "Aborted\n" );
							
							if ( ! k )
							{
								delete [ ] vs;
								vs = NULL;
								num_var = max_c = file_counter = 0;
							}
							
							goto add_clear;
						}
						
						for ( j = 0; j < l; ++j )
						{
							if ( ! var_names[ i ][ j ].compare( var_names[ 0 ][ j ] ) )
							{
								cmd( "ttk::messageBox -parent .da -type ok -icon error -title Error -message \"Invalid results files\" -detail \"The series in the files are not the same or have different number of cases or instances. Variables from objects created during the simulation may not be handled properly. Results files should come from the same set of simulation runs.\"" );
								plog( "Aborted\n" );
								
								if ( ! k )
								{
									delete [ ] vs;
									vs = NULL;
									num_var = max_c = file_counter = 0;
								}
							
								goto add_clear;
							}
						}
					}
					
					if ( k )
						var_num = num_var;
					else
						var_num = 0;
					
					cmd( "progressbox .da.pas \"Add Series\" \"Creating MC Series\" \"Series\" %d { set stop true } .da", l );
						
					for ( j = 0, stop = false; j < l && ! stop; ++j )
					{
						cur_var.resize( h );
						
						for ( i = 0; i < h; ++i )
							cur_var[ i ] = var_names[ i ][ j ];
						
						cmd( "set vname [ lindex [ split \"%s\" ] 0 ]", var_names[ 0 ][ j ].c_str( ) );
						cmd( "set ftag [ string replace [ lindex [ split \"%s\" ] 1 ] 0 3 ]", var_names[ 0 ][ j ].c_str( ) );
						
						create_series( choice, true, cur_var );
						
						cmd( "prgboxupdate .da.pas %d", j + 1 );
					}
					
					cmd( "destroytop .da.pas" );
						
					if ( ! k && num_var > m )
					{
						store *vs_new = new store[ num_var - m ];
						for ( i = m, j = 0; i < num_var; ++i, ++j )
						{
							vs_new[ j ] = vs[ i ];
							strcpy( vs_new[ j ].label, vs[ i ].label );
							strcpy( vs_new[ j ].tag, vs[ i ].tag );
						} 
						
						delete [ ] vs;
						vs = vs_new;
						num_var -= m;
						file_counter = 0;
					}
					
					if ( stop )
						plog( "Interrupted\n" );
					else
						plog( "Done\n" );
					
					add_clear:
					
					var_names.clear( );
					cur_var.clear( );
					
					cmd( "selectinlist .da.vars.lb.f.v 0" );
			}
			
			add_end:
			
			mc = false;

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
			cmd( "ttk::label .da.a.l -text \"Options for invoking Gnuplot\"" );

			cmd( "ttk::frame .da.a.st" );
			cmd( "ttk::label .da.a.st.l -text \"System terminal\"" );
			cmd( "ttk::entry .da.a.st.e -textvariable sysTermTmp -width 20 -justify center" );
			cmd( "pack .da.a.st.l .da.a.st.e -side left" );

			cmd( "ttk::frame .da.a.t" );
			cmd( "ttk::label .da.a.t.l -text \"Plot terminal (blank for default)\"" );
			cmd( "ttk::entry .da.a.t.e -textvariable gptermTmp -width 12 -justify center" );
			cmd( "pack .da.a.t.l .da.a.t.e -side left" );

			cmd( "ttk::frame .da.a.d" );
			cmd( "ttk::label .da.a.d.l -text \"3D grid configuration\"" );
			cmd( "ttk::entry .da.a.d.e -textvariable gpdgrid3dTmp -width 12 -justify center" );
			cmd( "pack .da.a.d.l .da.a.d.e -side left" );

			cmd( "ttk::frame .da.a.o" );
			cmd( "ttk::label .da.a.o.l -text \"Other options\"" );
			cmd( "ttk::text .da.a.o.t -height 10 -width 50 -dark $darkTheme -style smallFixed.TText" );
			cmd( "pack .da.a.o.l .da.a.o.t" );

			cmd( "pack .da.a.l .da.a.st .da.a.t .da.a.d .da.a.o -pady 5 -padx 5" );
			cmd( "okXhelpcancel .da.a b  { Default } { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#gpoptions } { set choice 2 }" );

			cmd( "showtop .da.a" );
			cmd( "mousewarpto .da.a.b.ok" );

			cmd( ".da.a.o.t insert end \"$gpoptions\"" );
			cmd( "focus .da.a.o.t" );

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
				cmd( "set gpoptions [ .da.a.o.t get 0.0 end ]" ); 
			}

			cmd( "destroytop .da.a" );
			
			break;

			
		// set colors
		case 21:		
			for ( i = 0; i < 20; ++i )
				cmd( "set current_c%d $c%d", i, i );

			cmd( "newtop .da.a \"Colors\" { set choice 2 } .da" );

			redraw_col:
			
			cmd( "ttk::label .da.a.l -text \"Pick the color to change\"" );

			cmd( "ttk::frame .da.a.o" );

			cmd( "ttk::frame .da.a.o.l1" );
			
			for ( i = 0; i < 10; ++i )
			{
				cmd( "ttk::frame .da.a.o.l1.c%d", i );
				cmd( "ttk::label .da.a.o.l1.c%d.n -text %d", i, i + 1 );
				cmd( "ttk::style configure $c%d.TLabel -foreground $c%d", i, i );
				cmd( "ttk::label .da.a.o.l1.c%d.c -text \"\u2588\u2588\u2588\u2588\u2588\u2588\" -style $c%d.TLabel", i, i );
				cmd( "bind .da.a.o.l1.c%d.c <Button-1> { set n_col %d; set col $c%d; set choice 4 }", i, i, i );
				cmd( "pack .da.a.o.l1.c%d.n .da.a.o.l1.c%d.c -side left", i, i );
				cmd( "pack .da.a.o.l1.c%d -anchor e", i );
			}
			 
			cmd( "ttk::frame .da.a.o.l2" );
			
			for ( i = 0; i < 10; ++i )
			{
				cmd( "ttk::frame .da.a.o.l2.c%d", i );
				cmd( "ttk::label .da.a.o.l2.c%d.n -text %d", i, i + 11 );
				cmd( "ttk::style configure $c%d.TLabel -foreground $c%d", i + 10, i + 10 );
				cmd( "ttk::label .da.a.o.l2.c%d.c -text \"\u2588\u2588\u2588\u2588\u2588\u2588\" -style $c%d.TLabel", i, i + 10 );
				cmd( "bind .da.a.o.l2.c%d.c <Button-1> { set n_col %d; set col $c%d; set choice 4 }", i, i + 10, i + 10 );
				cmd( "pack .da.a.o.l2.c%d.n .da.a.o.l2.c%d.c -side left", i, i );
				cmd( "pack .da.a.o.l2.c%d -anchor e", i );
			}

			cmd( "pack .da.a.o.l1 .da.a.o.l2 -side left -pady 5 -padx 15" );
			cmd( "pack .da.a.l .da.a.o -pady 5 -padx 5" );

			cmd( "okXhelpcancel .da.a b Default { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#colors } { set choice 2 }" );
			cmd( "showtop .da.a" );
			cmd( "mousewarpto .da.a.b.ok" );

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
						cmd( "if { $n_col >= 10 } { \
								set fr 2; \
								set n_col [ expr { $n_col - 10 } ] \
							} { \
								set fr 1 \
							}" );
						cmd( "ttk::style configure $n_col.TLabel -foreground $a" );
						cmd( ".da.a.o.l$fr.c$n_col.c configure -style $n_col.Tlabel" );
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

			cmd( "ttk::frame .da.s.x" );
			cmd( "ttk::label .da.s.x.l0 -text \"Plot width:\"" );
			cmd( "ttk::label .da.s.x.l1 -anchor e -text \"Sequence\"" );
			cmd( "set lx1 $hsizeP" ); 
			cmd( "ttk::entry .da.s.x.e1 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set lx1 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lx1; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.x.e1 insert 0 $lx1" ); 
			cmd( "ttk::label .da.s.x.l2 -anchor e -text \"  XY\"" );
			cmd( "set lx2 $hsizePxy" ); 
			cmd( "ttk::entry .da.s.x.e2 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set lx2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lx2; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.x.e2 insert 0 $lx2" ); 
			cmd( "pack .da.s.x.l0 .da.s.x.l1 .da.s.x.e1 .da.s.x.l2 .da.s.x.e2 -side left -padx 2 -pady 2" );

			cmd( "ttk::frame .da.s.y" );
			cmd( "ttk::label .da.s.y.l0 -anchor e -text \"Plot height:\"" );
			cmd( "ttk::label .da.s.y.l1 -anchor e -text \"Sequence\"" );
			cmd( "set ly1 $vsizeP" ); 
			cmd( "ttk::entry .da.s.y.e1 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set ly1 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ly1; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.y.e1 insert 0 $ly1" ); 
			cmd( "ttk::label .da.s.y.l2 -anchor e -text \"  XY\"" );
			cmd( "set ly2 $vsizePxy" ); 
			cmd( "ttk::entry .da.s.y.e2 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set ly2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ly2; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.y.e2 insert 0 $ly2" ); 
			cmd( "pack .da.s.y.l0 .da.s.y.l1 .da.s.y.e1 .da.s.y.l2 .da.s.y.e2 -side left -padx 2 -pady 2" );

			cmd( "ttk::frame .da.s.h" );
			cmd( "ttk::label .da.s.h.l -anchor e -text \"Horizontal borders\"" );
			cmd( "set hb $hmbordsizeP" ); 
			cmd( "ttk::entry .da.s.h.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set hb %%P; return 1 } { %%W delete 0 end; %%W insert 0 $hb; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.h.e insert 0 $hb" ); 
			cmd( "pack .da.s.h.l .da.s.h.e -side left -padx 2 -pady 2" );

			cmd( "ttk::frame .da.s.v" );
			cmd( "ttk::label .da.s.v.l0 -anchor e -text \"Vertical borders:\"" );
			cmd( "ttk::label .da.s.v.l1 -anchor e -text \"Top\"" );
			cmd( "set tb $tbordsizeP" ); 
			cmd( "ttk::entry .da.s.v.e1 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set tb %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tb; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.v.e1 insert 0 $tb" ); 
			cmd( "ttk::label .da.s.v.l2 -anchor e -text \"  Bottom\"" );
			cmd( "set bb $bbordsizeP" ); 
			cmd( "ttk::entry .da.s.v.e2 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set bb %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bb; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.v.e2 insert 0 $bb" ); 
			cmd( "pack .da.s.v.l0 .da.s.v.l1 .da.s.v.e1 .da.s.v.l2 .da.s.v.e2 -side left -padx 2 -pady 2" );

			cmd( "ttk::label .da.s.obs -text \"( all sizes are measured in screen pixels)\"" );

			cmd( "ttk::frame .da.s.t" );
			cmd( "ttk::label .da.s.t.l0 -anchor e -text \"Number of axis ticks:\"" );
			cmd( "ttk::label .da.s.t.l1 -anchor e -text \"Horizontal\"" );
			cmd( "set ht $hticksP" ); 
			cmd( "ttk::entry .da.s.t.e1 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set ht %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ht; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.t.e1 insert 0 $ht" ); 
			cmd( "ttk::label .da.s.t.l2 -anchor e -text \"  Vertical\"" );
			cmd( "set vt $vticksP" ); 
			cmd( "ttk::entry .da.s.t.e2 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set vt %%P; return 1 } { %%W delete 0 end; %%W insert 0 $vt; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.t.e2 insert 0 $vt" ); 
			cmd( "pack .da.s.t.l0 .da.s.t.l1 .da.s.t.e1 .da.s.t.l2 .da.s.t.e2 -side left -padx 2 -pady 2" );

			cmd( "ttk::frame .da.s.s" );
			cmd( "ttk::label .da.s.s.l1 -anchor e -text \"Smoothing\"" );
			cmd( "set sm $smoothP" ); 
			cmd( "ttk::combobox .da.s.s.e1 -values [ list no yes raw ] -width 5 -justify center -validate focusout -validatecommand { set n %%P; if { $n in [ list no yes raw ] } { set sm %%P; return 1 } { %%W delete 0 end; %%W insert 0 $sm; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any .da.s.s.e1 $sm" );
			cmd( "ttk::label .da.s.s.l2 -anchor e -text \"  Spline segments\"" );
			cmd( "set ss $splstepsP" ); 
			cmd( "ttk::entry .da.s.s.e2 -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set ss %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ss; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
			cmd( "if { $sm == \"raw\" } { .da.s.s.e2 configure -state normal }" );
			cmd( "write_any .da.s.s.e2 $ss" ); 
			cmd( "pack .da.s.s.l1 .da.s.s.e1 .da.s.s.l2 .da.s.s.e2 -side left -padx 2 -pady 2" );

			cmd( "ttk::frame .da.s.f" );
			cmd( "ttk::label .da.s.f.l -text \"Font name, size and style\"" );

			cmd( "ttk::frame .da.s.f.e" );
			cmd( "set ifont [ lindex $fontP 0 ]" ); 
			cmd( "ttk::combobox .da.s.f.e.font -values [ lsort -dictionary [ font families ] ] -width 15 -justify center -validate focusout -validatecommand { set n %%P; if { $n in [ font families ] } { set ifont %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ifont; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any .da.s.f.e.font $ifont" );
			cmd( "set idim [ lindex $fontP 1 ]" ); 
			cmd( "ttk::combobox .da.s.f.e.dim -values [ list 4 6 8 9 10 11 12 14 18 24 32 48 60 ] -width 3 -justify center -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 4 && $n <= 60 } { set idim %%P; return 1 } { %%W delete 0 end; %%W insert 0 $idim; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any .da.s.f.e.dim $idim" );
			cmd( "set istyle [ lindex $fontP 2 ]" ); 
			cmd( "ttk::combobox .da.s.f.e.sty -values [ list normal bold italic \"bold italic\" ] -width 10 -justify center -validate focusout -validatecommand { set n %%P; if { $n in [ list normal bold italic \"bold italic\" ] } { set istyle %%P; return 1 } { %%W delete 0 end; %%W insert 0 $istyle; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any .da.s.f.e.sty $istyle" );
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
			cmd( "mousewarpto .da.s.b.ok" );

			set_plot:
			
			*choice = 0;
			while ( *choice == 0 )
				Tcl_DoOneEvent( 0 );

			if ( *choice == 1 )
			{
				cmd( "set hsizeP 		[ .da.s.x.e1 get ]" ); 
				cmd( "set hsizePxy 		[ .da.s.x.e2 get ]" ); 
				cmd( "set vsizeP 		[ .da.s.y.e1 get ]" ); 
				cmd( "set vsizePxy 		[ .da.s.y.e2 get ]" ); 
				cmd( "set hmbordsizeP 	[ .da.s.h.e get ]" ); 
				cmd( "set tbordsizeP	[ .da.s.v.e1 get ]" ); 
				cmd( "set bbordsizeP	[ .da.s.v.e2 get ]" ); 
				cmd( "set hticksP 		[ .da.s.t.e1 get ]" ); 
				cmd( "set vticksP 		[ .da.s.t.e2 get ]" ); 
				cmd( "set splstepsP 	[ .da.s.s.e2 get ]" ); 
				cmd( "set smoothP 		[ .da.s.s.e1 get ]" ); 
				cmd( "set fontP 		[ list [ .da.s.f.e.font get ] [ .da.s.f.e.dim get ] [ .da.s.f.e.sty get ] ]" );
				

			}

			if ( *choice == 3 )
			{
				cmd( "write_any .da.s.x.e1  $default_hsizeP" );
				cmd( "write_any .da.s.x.e2  $default_hsizePxy" );
				cmd( "write_any .da.s.y.e1  $default_vsizeP" );
				cmd( "write_any .da.s.y.e2  $default_vsizePxy" );
				cmd( "write_any .da.s.h.e   $default_hmbordsizeP" );
				cmd( "write_any .da.s.v.e1  $default_tbordsizeP" );
				cmd( "write_any .da.s.v.e2  $default_bbordsizeP" );
				cmd( "write_any .da.s.t.e1  $default_hticksP" );
				cmd( "write_any .da.s.t.e2  $default_vticksP" );
				cmd( "write_any .da.s.s.e1 	$default_smoothP" );
				cmd( "write_any .da.s.s.e2  $default_splstepsP" );
				cmd( "write_any .da.s.f.e.font 	[ lindex $default_fontP 0 ]" );
				cmd( "write_any .da.s.f.e.dim 	[ lindex $default_fontP 1 ]" );
				cmd( "write_any .da.s.f.e.sty 	[ lindex $default_fontP 2 ]" );
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

			cmd( "ttk::frame .da.s.s" );
			cmd( "ttk::label .da.s.s.l -width 30 -anchor e -text \"Color scale\"" );
			cmd( "set cs $cscaleLat" ); 
			cmd( "ttk::entry .da.s.s.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] && $n > 0 } { set cs %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cs; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.s.e insert 0 $cs" ); 
			cmd( "pack .da.s.s.l .da.s.s.e -side left -anchor w -padx 2 -pady 2" );

			cmd( "ttk::frame .da.s.x" );
			cmd( "ttk::label .da.s.x.l -width 30 -anchor e -text \"Lattice width (pixels)\"" );
			cmd( "set lx $hsizeLat" ); 
			cmd( "ttk::entry .da.s.x.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set lx %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lx; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.x.e insert 0 $lx" ); 
			cmd( "pack .da.s.x.l .da.s.x.e -side left -anchor w -padx 2 -pady 2" );

			cmd( "ttk::frame .da.s.y" );
			cmd( "ttk::label .da.s.y.l -width 30 -anchor e -text \"Lattice heigth (pixels)\"" );
			cmd( "set ly $vsizeLat" ); 
			cmd( "ttk::entry .da.s.y.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set ly %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ly; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".da.s.y.e insert 0 $ly" ); 
			cmd( "pack .da.s.y.l .da.s.y.e -side left -anchor w -padx 2 -pady 2" );

			cmd( "pack .da.s.s .da.s.x .da.s.y -anchor w -padx 5 -pady 5" );

			cmd( "okXhelpcancel .da.s b Default { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#latticeparameters } { set choice 2 }" );

			cmd( "bind .da.s.s.e <KeyPress-Return> { focus .da.s.x.e; .da.s.x.e selection range 0 end }" );
			cmd( "bind .da.s.x.e <KeyPress-Return> { focus .da.s.y.e; .da.s.y.e selection range 0 end }" );
			cmd( "bind .da.s.y.e <KeyPress-Return> { set choice 1 }" );

			cmd( "showtop .da.s" );
			cmd( "focus .da.s.s.e; .da.s.s.e selection range 0 end" );
			cmd( "mousewarpto .da.s.b.ok" );

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
			show_report( choice, ".da" );
			break;


		// CANVAS OPTIONS

		// Edit labels
		case 26:
			cmd( "set itext [ $ccanvas itemcget current -text ]" );
			cmd( "set ifont [ lindex [ $ccanvas itemcget current -font ] 0 ]" );
			cmd( "set idim [ lindex [ $ccanvas itemcget current -font ] 1 ]" );
			cmd( "set istyle [ lindex [ $ccanvas itemcget current -font ] 2 ]" );
			cmd( "set icolor [ $ccanvas itemcget current -fill ]" );  
			cmd( "set fontall 0" );
			cmd( "set colorall 0" );
			
			cmd( "set wid $ccanvas.a" );
			cmd( "newtop $wid \"Edit Text\" { set choice 2 } $ccanvas" );
			cmd( "wm geometry $wid +$LX+$LY" );
			
			cmd( "ttk::frame $wid.l" );
			cmd( "ttk::label $wid.l.t -text \"New text\"" );
			cmd( "ttk::entry $wid.l.e -textvariable itext -width 30 -justify center" );
			cmd( "pack $wid.l.t $wid.l.e" );
			
			cmd( "ttk::frame $wid.format" );
			cmd( "ttk::label $wid.format.tit -text \"Font name, size and style\"" );
			
			cmd( "ttk::frame $wid.format.e" );
			cmd( "ttk::combobox $wid.format.e.font -values [ lsort -dictionary [ font families ] ] -width 15 -justify center -validate focusout -validatecommand { set n %%P; if { $n in [ font families ] } { set ifont %%P; return 1 } { %%W delete 0 end; %%W insert 0 $ifont; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any $wid.format.e.font $ifont" );
			cmd( "ttk::combobox $wid.format.e.dim -values [ list 4 6 8 9 10 11 12 14 18 24 32 48 60 ] -width 3 -justify center -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 4 && $n <= 60 } { set idim %%P; return 1 } { %%W delete 0 end; %%W insert 0 $idim; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any $wid.format.e.dim $idim" );
			cmd( "ttk::combobox $wid.format.e.sty -values [ list normal bold italic \"bold italic\" ] -width 10 -justify center -validate focusout -validatecommand { set n %%P; if { $n in [ list normal bold italic \"bold italic\" ] } { set istyle %%P; return 1 } { %%W delete 0 end; %%W insert 0 $istyle; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any $wid.format.e.sty $istyle" );
			cmd( "pack $wid.format.e.font $wid.format.e.dim $wid.format.e.sty -padx 2 -side left" );
			
			cmd( "pack $wid.format.tit $wid.format.e" );
			
			cmd( "ttk::frame $wid.c" );
			cmd( "ttk::label $wid.c.l -text \"Text color\"" );
			cmd( "ttk::style configure icolor.TButton -foreground [ invert_color $icolor ] -background $icolor" );
			cmd( "ttk::button $wid.c.color -width 5 -text Set -style icolor.TButton -command { \
					set app [ tk_chooseColor -parent $wid -title \"Text Color\" -initialcolor $icolor ]; \
					if { $app != \"\" } { \
						set icolor $app \
					}; \
					ttk::style configure icolor.TButton -foreground [ invert_color $icolor ] -background $icolor \
				}" );
			cmd( "pack $wid.c.l $wid.c.color -padx 2 -side left" );
			
			cmd( "ttk::frame $wid.fall" );
			cmd( "ttk::checkbutton $wid.fall.font -text \"Apply font to all text items\" -variable fontall" );
			cmd( "ttk::checkbutton $wid.fall.color -text \"Apply color to all text items\" -variable colorall" );
			cmd( "pack $wid.fall.font $wid.fall.color" );
			
			cmd( "pack $wid.l $wid.format $wid.c $wid.fall -padx 5 -pady 5" );
			
			cmd( "okXhelpcancel $wid b Delete { set itext \"\"; set choice 1 } { set choice 1 } { LsdHelp menudata_res.html#graph } { set choice 2 }" );
			
			cmd( "bind $wid.l.e <Return> { $wid.b.ok invoke }" );
			cmd( "bind $wid.format.e.font <Return> { $wid.b.ok invoke }" );
			cmd( "bind $wid.format.e.dim <Return> { $wid.b.ok invoke }" );
			cmd( "bind $wid.format.e.sty <Return> { $wid.b.ok invoke }" );
			
			cmd( "showtop $wid current" );
			cmd( "focus $wid.l.e" );
			cmd( "$wid.l.e selection range 0 end" );
			cmd( "mousewarpto $wid.b.ok" );
			
			*choice = 0;
			while ( ! *choice )
				Tcl_DoOneEvent( 0 );
			
			if ( *choice == 1 )
			{
				cmd( "if { $itext==\"\" } { $ccanvas delete $curitem; set choice 0 }" );
				if ( *choice == 1 )
				{
					cmd( "$ccanvas itemconf $curitem -text \"$itext\"" );
					cmd( "set ml [ list [ $wid.format.e.font get ] [ $wid.format.e.dim get ] [ $wid.format.e.sty get ] ]" );
					cmd( "if [ catch { $ccanvas itemconf $curitem -font \"$ml\" } ] { bell }" );
					cmd( "if [ catch { $ccanvas itemconf $curitem -fill $icolor } ] { bell }" );  
				}
				
				cmd( "set choice $fontall" );
				if ( *choice == 1 )
					cmd( "if [ catch { $ccanvas itemconf text -font \"$ml\" } ] { bell }" );
				cmd( "set choice $colorall" );
				if ( *choice == 1 )
					cmd( "if [ catch { $ccanvas itemconf text -fill $icolor } ] { bell }" );
			} 
			
			cmd( "destroytop $wid" ); 
			
			break;


		// New labels
		case 27:
			cmd( "set itext \"new text\"" );
			
			cmd( "set wid $ccanvas.a" );
			cmd( "newtop $wid \"New Text\" { set choice 2 } $ccanvas" );
			cmd( "wm geometry $wid +$LX+$LY" );
			
			cmd( "ttk::frame $wid.l" );
			cmd( "ttk::label $wid.l.t -text \"New text\"" );
			cmd( "ttk::entry $wid.l.e -textvariable itext -width 30 -justify center" );
			cmd( "pack $wid.l.t $wid.l.e" );
			cmd( "pack $wid.l -padx 5 -pady 5" );
			
			cmd( "okhelpcancel $wid b { set choice 1 } { LsdHelp menudata_res.html#graph } { set choice 2 }" );
			
			cmd( "bind $wid.l.e <Return> { $wid.b.ok invoke}" );
			
			cmd( "showtop $wid current" );
			cmd( "focus $wid.l.e" );
			cmd( "$wid.l.e selection range 0 end" );
			
			*choice = 0;
			while ( ! *choice )
				Tcl_DoOneEvent( 0 );
			
			cmd( "destroytop $wid" ); 
			
			if ( *choice == 1 )
				cmd( "$ccanvas create text $hereX $hereY -text \"$itext\" -fill $colorsTheme(fg) -font $fontP -tags { text draw }" );
			
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
			cmd( "set icolor [ $ccanvas itemcget $cline -fill ]" );
			cmd( "set widthall 0" );
			cmd( "set colorall 0" );
			
			cmd( "set wid $ccanvas.a" );
			cmd( "newtop $wid \"Edit Line\" { set choice 2 } $ccanvas" );
			cmd( "wm geometry $wid +$LX+$LY" );
			
			cmd( "ttk::frame $wid.l" );
			cmd( "ttk::label $wid.l.t -text \"Width\"" );
			cmd( "ttk::spinbox $wid.l.e -textvariable iwidth -width 5 -from 1.0 -to 10.0 -justify center -state disabled -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] && $n > 0 && $n < 100 } { set iwidth %%P; return 1 } { %%W delete 0 end; %%W insert 0 $iwidth; return 0 } } -invalidcommand { bell }" );
			cmd( "pack $wid.l.t $wid.l.e -padx 2 -side left" );
			
			cmd( "ttk::frame $wid.c" );
			cmd( "ttk::label $wid.c.l -text \"Color\"" );
			cmd( "ttk::style configure icolor.TButton -foreground [ invert_color $icolor ] -background $icolor" );
			cmd( "ttk::button $wid.c.color -width 5 -text Set -style icolor.TButton -command { \
					set app [ tk_chooseColor -parent $wid -title \"Line Color\" -initialcolor $icolor ]; \
					if { $app != \"\" } { \
						set icolor $app \
					}; \
					ttk::style configure icolor.TButton -foreground [ invert_color $icolor ] -background $icolor \
				}" );
			cmd( "ttk::label $wid.c.pad -width 3" );
			cmd( "ttk::label $wid.c.t -text \"Dash pattern\"" );
			cmd( "ttk::combobox $wid.c.e -values [ list \"\" \". \" \"- \" \"-.\" \"-..\" ] -width 3 -justify center -state disabled -validate focusout -validatecommand { set n %%P; if { $n in [ list \"\" \". \" \"- \" \"-.\" \"-..\" ] } { set idash %%P; return 1 } { %%W delete 0 end; %%W insert 0 $idash; return 0 } } -invalidcommand { bell }" ); 
			cmd( "write_any $wid.c.e $idash" );
			cmd( "pack $wid.c.l $wid.c.color $wid.c.pad $wid.c.t $wid.c.e -padx 2 -side left" );
			
			cmd( "ttk::frame $wid.d" );
			cmd( "ttk::label $wid.d.t -text \"Line-end arrow( s)\"" );
			cmd( "ttk::combobox $wid.d.e -values [ list none first last both ] -width 7 -justify center -state disabled -validate focusout -validatecommand { set n %%P; if { $n in [ list none first last both ] } { set iarrow %%P; return 1 } { %%W delete 0 end; %%W insert 0 $iarrow; return 0 } } -invalidcommand { bell }" );
			cmd( "write_any $wid.d.e $iarrow" );
			cmd( "pack $wid.d.t $wid.d.e -padx 2 -side left" );
			
			cmd( "ttk::frame $wid.fall" );
			cmd( "ttk::checkbutton $wid.fall.font -text \"Apply width to all line items\" -variable widthall -state disabled" );
			cmd( "ttk::checkbutton $wid.fall.color -text \"Apply color to all line items\" -variable colorall" );
			cmd( "pack $wid.fall.font $wid.fall.color" );
			
			cmd( "pack $wid.l $wid.c $wid.d $wid.fall -padx 5 -pady 5" );
			
			cmd( "okXhelpcancel $wid b Delete { set iwidth 0; set choice 1 } { set choice 1 } { LsdHelp menudata_res.html#graph } { set choice 2 }" );
			
			cmd( "bind $wid.l.e <Return> { $wid.b.ok invoke }" );
			cmd( "bind $wid.c.e <Return> { $wid.b.ok invoke }" );
			cmd( "bind $wid.d.e <Return> { $wid.b.ok invoke }" );
			
			cmd( "showtop $wid current" );
			cmd( "focus $wid.l.e" );
			cmd( "$wid.l.e selection range 0 end" );
			cmd( "mousewarpto $wid.b.ok" );
		 
			// enable most options for non-dotted lines
			cmd( "if { ! $dots } { $wid.l.e  configure -state normal; $wid.c.e  configure -state normal; $wid.fall.font  configure -state normal }" );
			// enable dashes & arrows for drawing lines only
			cmd( "if $draw { $wid.d.e  configure -state normal }" );

			*choice = 0;
			while ( ! *choice )
				Tcl_DoOneEvent( 0 );
			
			if ( *choice == 1 )
			{
				cmd( "if $draw { \
						if { $iwidth == 0 } { \
							$ccanvas delete $curitem; \
							set choice 0 \
						} { \
							set choice 3 \
						} \
					} { \
						if { $iwidth == 0 } { \
							$ccanvas delete $cline; \
							set choice 0 \
						} { \
							if $dots { \
								set choice 2 \
							} { \
								set choice 1 \
							} \
						} \
					}" );
				
				if ( *choice == 1 )	// regular line made of lines & dots
				{	// avoid changing the dots parts in regular lines
					cmd( "$ccanvas dtag selected" );
					cmd( "$ccanvas addtag selected withtag \"$cline&&line\"" );
					cmd( "if [ catch { $ccanvas itemconf selected -width [ $wid.l.e get ] } ] { bell }" );
					cmd( "if [ catch { $ccanvas itemconf selected -dash \"[ $wid.c.e get ]\" } ] { bell }" );
					cmd( "if [ catch { $ccanvas itemconf selected -arrow [ $wid.d.e get ] } ] { bell }" );
					cmd( "$ccanvas dtag selected" );
					
					cmd( "$ccanvas itemconf $cline -fill $icolor" );  
				}
				
				if ( *choice == 2 )	// line of dots
					cmd( "$ccanvas itemconf $cline -fill $icolor" );  
				
				if ( *choice == 3 )	// draw line
				{
					cmd( "if [ catch { $ccanvas itemconf $curitem -width [ $wid.l.e get ] } ] { bell }" );
					cmd( "if [ catch { $ccanvas itemconf $curitem -dash \"[ $wid.c.e get ]\" } ] { bell }" );
					cmd( "if [ catch { $ccanvas itemconf $curitem -arrow [ $wid.d.e get ] } ] { bell }" );
					
					cmd( "$ccanvas itemconf $curitem -fill $icolor" );  
				}
				
				cmd( "set choice $widthall" );
				if ( *choice == 1 )
					cmd( "if [ catch { $ccanvas itemconf line -width $iwidth } ] { bell }" );
				
				cmd( "set choice $colorall" );
				if ( *choice == 1 )
					cmd( "if [ catch { $ccanvas itemconf line -fill $icolor } ] { bell }" );
			} 
			
			cmd( "destroytop $wid" ); 
			
			break;


		// Insert a line item
		case 28: 
			cmd( "set choice $ncanvas" );
			i = *choice;
			
			cmd( "bind $ccanvas <B1-Motion> { \
					set ccanvas $daptab.tab%d.c.f.plots; \
					if [ info exists cl ] { \
						$ccanvas delete $cl\
					}; \
					set ax [ $ccanvas canvasx %%x ]; \
					set ay [ $ccanvas canvasy %%y ]; \
					set cl [ $ccanvas create line $hereX $hereY $ax $ay -fill $colorsTheme(fg) -tags { line draw selected } ] \
				}", i );
			
			cmd( "bind $ccanvas <Shift-B1-Motion> { \
					set ccanvas $daptab.tab%d.c.f.plots; \
					if [ info exists cl ] { \
						$ccanvas delete $cl\
					}; \
					set ax [ $ccanvas canvasx %%x ]; \
					set ay [ $ccanvas canvasy %%y ]; \
					if { [ expr { abs( $ax - $hereX ) } ] > [ expr { abs( $ay - $hereY ) } ] } { \
						set ay $hereY \
					} { \
						set ax $hereX \
					}; \
					set cl [ $ccanvas create line $hereX $hereY $ax $ay -fill $colorsTheme(fg) -tags { line draw selected } ] \
				}", i );
				
			cmd( "bind $ccanvas <ButtonRelease-1> { \
					set ccanvas $daptab.tab%d.c.f.plots; \
					$ccanvas dtag selected; \
					bind $ccanvas <B1-Motion> {\
						set ccanvas $daptab.tab%d.c.f.plots; \
						if $moving { \
							$ccanvas move current [ expr { [ $ccanvas canvasx %%%%x ] - $hereX } ] \
								[ expr { [ $ccanvas canvasy %%%%y ] - $hereY } ]; \
							set hereX [ $ccanvas canvasx %%%%x ]; \
							set hereY [ $ccanvas canvasy %%%%y ] \
						} \
					}; \
					bind $ccanvas <Shift-B1-Motion> { }; \
					bind $ccanvas <ButtonRelease-1> { set moving false }; \
					if [ info exists cl ] { \
						$ccanvas bind $cl <1> \" \
							set ccanvas $daptab.tab%d.c.f.plots; \
							$ccanvas dtag selected; \
							$ccanvas addtag selected withtag $cl; \
							$ccanvas raise selected; \
							set hereX %%%%x; \
							set hereY %%%%y \
						\"; \
						$ccanvas bind $cl <ButtonRelease-1> { \
							$daptab.tab%d.c.f.plots dtag selected \
						}; \
						$ccanvas bind $cl <B1-Motion> { \
							set ax %%%%x; \
							set ay %%%%y; \
							$daptab.tab%d.c.f.plots move selected [ expr { $ax - $hereX } ] [ expr { $ay - $hereY } ]; \
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
			cmd( "set icolor1 [ $ccanvas itemcget current -outline ]" );
			cmd( "set icolor2 [ $ccanvas itemcget current -fill ]" );
			cmd( "set widthall 0" );
			cmd( "set colorall 0" );
			
			cmd( "if { ! [ string is double -strict $iwidth ] } { set iwidth 0.0 }" );
			
			cmd( "set wid $ccanvas.a" );
			cmd( "newtop $wid \"Edit Bar\" { set choice 2 } $ccanvas" );
			cmd( "wm geometry $wid +$LX+$LY" );
			
			cmd( "ttk::frame $wid.c" );
			cmd( "ttk::label $wid.c.l -text \"Fill color\"" );
			cmd( "ttk::style configure icolor2.TButton -foreground [ invert_color $icolor2 ] -background $icolor2" );
			cmd( "ttk::button $wid.c.color -width 5 -text Set -style icolor2.TButton -command { \
					set app [ tk_chooseColor -parent $wid -title \"Fill Color\" -initialcolor $icolor2 ]; \
					if { $app != \"\" } { \
						set icolor2 $app \
					}; \
					ttk::style configure icolor2.TButton -foreground [ invert_color $icolor2 ] -background $icolor2 \
				}" );
			cmd( "pack $wid.c.l $wid.c.color -padx 2 -side left" );
			
			cmd( "ttk::frame $wid.l" );
			cmd( "ttk::label $wid.l.t -text \"Outline width\"" );
			cmd( "ttk::spinbox $wid.l.e -textvariable iwidth -width 5 -from 0.0 -to 10.0 -justify center -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] && $n >= 0 } { set iwidth %%P; return 1 } { %%W delete 0 end; %%W insert 0 $iwidth; return 0 } } -invalidcommand { bell }" );
			cmd( "ttk::label $wid.l.l -text \" color\"" );
			cmd( "ttk::style configure icolor1.TButton -foreground [ invert_color $icolor1 ] -background $icolor1" );
			cmd( "ttk::button $wid.l.color -width 5 -text Set -style icolor1.TButton -command { \
					set app [ tk_chooseColor -parent $wid -title \"Outline Color\" -initialcolor $icolor1 ]; \
					if { $app != \"\" } { \
						set icolor1 $app \
					}; \
					ttk::style configure icolor1.TButton -foreground [ invert_color $icolor1 ] -background $icolor1 \
				}" );
			cmd( "pack $wid.l.t $wid.l.e $wid.l.l $wid.l.color -padx 2 -side left" );
			
			cmd( "ttk::frame $wid.fall" );
			cmd( "ttk::checkbutton $wid.fall.color -text \"Apply fill to all bar items\" -variable colorall" );
			cmd( "ttk::checkbutton $wid.fall.font -text \"Apply outline to all bar items\" -variable widthall" );
			cmd( "pack $wid.fall.color $wid.fall.font" );
			
			cmd( "pack $wid.c $wid.l $wid.fall -padx 5 -pady 5" );
			
			cmd( "okXhelpcancel $wid b Delete { set choice 3 } { set choice 1 } { LsdHelp menudata_res.html#graph } { set choice 2 }" );
			
			cmd( "bind $wid.l.e <Return> { $wid.b.ok invoke }" );
			
			cmd( "showtop $wid current" );
			cmd( "focus $wid.l.e" );
			cmd( "$wid.l.e selection range 0 end" );
			cmd( "mousewarpto $wid.b.ok" );
			
			*choice = 0;
			while ( ! *choice )
				Tcl_DoOneEvent( 0 );
			
			if ( *choice == 1 )
			{
				cmd( "$ccanvas itemconf $curitem -width $iwidth" );
				cmd( "$ccanvas itemconf $curitem -outline $icolor1" );
				cmd( "$ccanvas itemconf $curitem -fill $icolor2" );
				
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
				cmd( "$ccanvas delete $curitem" );
			
			cmd( "destroytop $wid" ); 
			
			break;
			

		// MODEL STRUCTURE ACTIONS

		// select parent filter
		case 29:
			cmd( "filter_series $res_g" );
			break;

		default:
			break;
	}
	
	cmd( "update" );
}
}


/***************************************************
 UPDATE_BOUNDS
 ****************************************************/
void update_bounds( void )
{
	if ( isfinite( miny ) )
		cmd( "write_any .da.f.h.v.sc.min.min [ format \"%%.${pdigits}g\" $miny ]" );
	else
	{
		cmd( "write_any .da.f.h.v.sc.min.min -Infinity" );
		miny = 0;
	}
	
	if ( isfinite( maxy ) )
		cmd( "write_any .da.f.h.v.sc.max.max [ format \"%%.${pdigits}g\" $maxy ]" );
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
	
	if ( min_c < first_c )
		min_c = max( first_c, showInit ? 0 : 1 );
	
	if ( max_c <= min_c )
	{
		max_c = min_c + 1;
		
		if ( max_c > num_c && num_c > 0 )
		{
			max_c = num_c;
			min_c = num_c - 1;
		}
	}

	cmd( "write_any .da.f.h.v.ft.from.mnc $minc" );
	cmd( "write_any .da.f.h.v.ft.to.mxc $maxc" );
}
	

/***************************************************
 PLOT_TSERIES
 ****************************************************/
void plot_tseries( int *choice )
{
	bool y2on, done;
	char *app, **str, **tag;
	double temp, **data;
	int i, j, *start, *end, *id;

	if ( nv > 1000 )
	{
		cmd( "set answer [ ttk::messageBox -parent .da -type okcancel -title \"Too Many Series\" -icon warning -default ok -message \"You selected too many ($nv) series to plot\" -detail \"This may cause a crash of LSD, with the loss of all unsaved data.\nIf you continue the system may become slow, please be patient.\nPress 'OK' to continue anyway.\" ]" );
		cmd( "if { [ string compare $answer ok ] == 0 } { set choice 1 } { set choice 2 }" );
		if ( *choice == 2 )
			return;
	}
	 
	if ( nv == 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;

		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
	  
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
			{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}
	   
			if ( logs )			// apply log to the values to show "log scale" in the y-axis
				data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "plot" );
		}
	}

	// handle case selection
	if ( autom_x || min_c >= max_c )
	{
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = max( start[ i ], showInit ? 0 : 1 );

			if ( start[ i ] < min_c )
				min_c = max( start[ i ], showInit ? 0 : 1 );
			
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
				if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) )		// ignore NaNs
				{
					miny = maxy = data[ i ][ j - start[ i ] ];
					done = true;
				}
				
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] < miny )		// ignore NaNs
					miny = data[ i ][ j - start[ i ] ];
					
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] > maxy )		// ignore NaNs
					maxy = data[ i ][ j - start[ i ] ];
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
			if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) )		// ignore NaNs
			{
				miny2 = maxy2 = data[ i ][ j - start[ i ] ];
				done = true;
			}
			
			if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] < miny2 )		// ignore NaNs
				miny2 = data[ i ][ j - start[ i ] ];
				
			if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] > maxy2 )		// ignore NaNs
				maxy2 = data[ i ][ j - start[ i ] ];
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
			delete [ ] data[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
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
	bool first;
	char *app, **str, **tag;
	double temp, **val, **data;
	int i, j, k, nt, new_nv, *list_times, *pos, *start, *end, *id, *erase;

	cmd( "if [ info exists num_t ] { set nt $num_t } { set nt \"-1\" }" );
	nt = get_int( "nt" );
	
	if ( nv < 2 || nt <= 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series/cases selected\" -detail \"Place at least two series in the Series Selected listbox and select at least one case (time step).\"" );
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

	for ( i = 0; i < nt; ++i )
	{
		cmd( "set k [ lindex $list_times %d ]", i );
		k = get_int( "k" );
		list_times[ i ] = k;
	}
	
	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0, new_nv = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;

		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
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
			{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}
	   
			if ( logs )			// apply log to the values to show "log scale"
				data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "plot" );
			
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
			if ( erase[ j ] == 0 && is_finite( data[ j ][ list_times[ k ] - start[ j ] ] ) )		// ignore NaNs
			{
				val[ i ][ k ] = data[ j ][ list_times[ k ] - start[ j ] ];
			
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
			delete [ ] data[ i ];
	}

	delete [ ] list_times;
	delete [ ] pos;
	delete [ ] val;
	delete [ ] erase;
	delete [ ] str;
	delete [ ] tag;
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
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place two or more series in the Series Selected listbox.\"" );
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
	cmd( "newtop $p \"Cross Section Cases\" { set choice 2 } .da" );

	cmd( "ttk::frame $p.u" );

	cmd( "ttk::frame $p.u.i" );

	cmd( "ttk::frame $p.u.i.e" );
	cmd( "ttk::label $p.u.i.e.l -text \"Case to add\"" );
	cmd( "ttk::entry $p.u.i.e.e -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= $minc && $n <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "$p.u.i.e.e insert 0 $bidi" );
	cmd( "pack $p.u.i.e.l $p.u.i.e.e" );
	 
	cmd( "ttk::frame $p.u.i.lb" );
	cmd( "ttk::label $p.u.i.lb.l -text \"Selected cases\"" );

	cmd( "ttk::frame $p.u.i.lb.lb" );
	cmd( "ttk::scrollbar $p.u.i.lb.lb.v_scroll -command \".da.s.u.i.lb.lb.lb yview\"" );
	cmd( "ttk::listbox $p.u.i.lb.lb.lb -listvariable list_times -width 8 -height 7 -selectmode extended -yscroll \".da.s.u.i.lb.lb.v_scroll set\" -dark $darkTheme" );
	cmd( "mouse_wheel $p.u.i.lb.lb.lb" );
	cmd( "pack $p.u.i.lb.lb.lb $p.u.i.lb.lb.v_scroll -side left -fill y" );

	cmd( "pack $p.u.i.lb.l $p.u.i.lb.lb" );

	cmd( "pack $p.u.i.e $p.u.i.lb -padx 5 -pady 5" );

	cmd( "ttk::frame $p.u.s" );
	cmd( "ttk::label $p.u.s.l -text \"\nTime series order\"" );
	cmd( "pack $p.u.s.l" );

	cmd( "ttk::frame $p.u.s.b -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton $p.u.s.b.nosort -text \"As selected\" -variable dir -value 0 -command { .da.s.u.s.r.e configure -state disabled; .da.s.u.i.e.e selection range 0 end; focus .da.s.u.i.e.e }" );
	cmd( "ttk::radiobutton $p.u.s.b.up -text \"Ascending order\" -variable dir -value 1 -command { set sel [ .da.s.u.i.lb.lb.lb curselection ]; if { [ llength $sel ] == 1 } { set res [ lindex $list_times $sel ] } { set res $maxc }; .da.s.u.s.r.e configure -state normal; .da.s.u.s.r.e delete 0 end; .da.s.u.s.r.e insert 0 $res; .da.s.u.s.r.e selection range 0 end; focus .da.s.u.s.r.e }" );
	cmd( "ttk::radiobutton $p.u.s.b.down -text \"Descending order\" -variable dir -value \"-1\" -command { set sel [ .da.s.u.i.lb.lb.lb curselection ]; if { [ llength $sel ] == 1 } { set res [ lindex $list_times $sel ] } { set res $maxc }; .da.s.u.s.r.e configure -state normal; .da.s.u.s.r.e delete 0 end; .da.s.u.s.r.e insert 0 $res; .da.s.u.s.r.e selection range 0 end; focus .da.s.u.s.r.e }" );
	cmd( "pack $p.u.s.b.nosort $p.u.s.b.up $p.u.s.b.down -anchor w" );
	cmd( "pack $p.u.s.b -padx 5" );

	cmd( "ttk::frame $p.u.s.r" );
	cmd( "ttk::label $p.u.s.r.l -justify center -text \"Case reference\nfor series sorting\"" );
	cmd( "ttk::entry $p.u.s.r.e -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && ( $n in $list_times ) } { set res %%P; return 1 } { %%W delete 0 end; %%W insert 0 $res; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
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
			if { [ info exists sfrom ] && [ info exists sto ] && [ string is integer -strict $sfrom ] && [ string is integer -strict $sto ] && [ expr { $sto - $sfrom } ] > 0 } { \
				if { ! [ info exists sskip ] } { \
					set sskip 1 \
				}; \
				for { set x $sfrom } { $x <= $sto && $x <= $maxc } { incr x $sskip } { \
					.da.s.u.i.lb.lb.lb insert end $x \
				} \
			}; \
			.da.s.u.i.e.e selection range 0 end \
		}; bind $p.u.i.e.e <Control-Z> { \
			if { [ info exists sfrom ] && [ info exists sto ] && [ string is integer -strict $sfrom ] && [ string is integer -strict $sto ] && [ expr { $sto - $sfrom } ] > 0 } { \
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
				.da.s.u.i.lb.lb.lb delete [ lindex $sel 0 ] [ lindex $sel [ expr { [ llength $sel ] - 1 } ] ]; \
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
	cmd( "mousewarpto $p.fb.ok" );

	*choice = 0;
	while ( ! *choice )
		Tcl_DoOneEvent( 0 );

	if ( *choice == 2 )
		goto end;

	cmd( "if { [ .da.s.u.i.lb.lb.lb size ] == 0 } { ttk::messageBox -parent .da.s -type ok -title Error -icon error -message \"No case selected\" -detail \"At least one case (time step) must be selected. Please try again.\"; set choice 2 }" );

	if ( *choice == 2 )
		goto end;

	cmd( "set res [ $p.u.s.r.e get ]" );
	cmd( "if { $dir != 0 && [ lsearch $list_times $res ] < 0 } { ttk::messageBox -parent .da.s -type ok -title Warning -icon warning -message \"Invalid case reference selected\" -detail \"The selected case (time step) reference is not one of the selected for the cross-section(s), no sorting will be performed.\"; set dir 0 }" );
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
LOG_DATA
****************************************************/
double *log_data( double *data, int start, int end, int ser, const char *err_msg )
{
	bool stopErr;
	double *logdata;
	int i, errCnt;
	
	logdata = new double [ end - start + 1 ];
	
	for ( stopErr = false, errCnt = 0, i = 0; i <= end - start; ++i )
		if ( ! is_nan( data[ i ] ) && data[ i ] > 0.0 )
			logdata[ i ] = log( data[ i ] );
		else
		{
			logdata[ i ] = NAN;
			if ( i > 0 && ++errCnt < ERR_LIM )	// prevent slow down due to I/O
				if ( ser >= 0 )
					plog( "\nWarning: zero or negative values in log %s (ignored)\n         Series: %d, Case: %d", "", err_msg, ser + 1, start + i );
				else
					plog( "\nWarning: zero or negative values in log %s (ignored)\n         Case: %d", "", err_msg, start + i );
			else
				if ( i > 0 && ! stopErr )
				{
					plog( "\nWarning: too many zero or negative values, stop reporting...\n" );
					stopErr = true;
				}
		}
	
	return logdata;
}


/***************************************************
INSERT_DATA_MEM
****************************************************/
void insert_data_mem( object *r, int *num_v, char *lab )
{
	int i, ini_v = *num_v;
	
	insert_labels_mem( r, num_v, lab );
	cmd( "update_parent" );
	
	store *vs_new = new store[ *num_v ];
	
	for ( i = 0; i < ini_v; ++i )
	{
		vs_new[ i ] = vs[ i ];
		strcpy( vs_new[ i ].label, vs[ i ].label );
		strcpy( vs_new[ i ].tag, vs[ i ].tag );
	}
	
	delete [ ] vs;
	vs = vs_new;
	
	insert_store_mem( r, &ini_v, lab );
	
	if ( *num_v != ini_v )
	{
		error_hard( "invalid number of series", 
					"internal problem in LSD", 
					"if error persists, please contact developers",
					true );
		myexit( 18 );
	}
}


/***************************************************
CREATE_PAR_MAP
****************************************************/
void create_par_map( object *r )
{
	bridge *cb;
	object *cur;
	variable *cv;
	
	for ( cv = r->v; cv != NULL; cv = cv->next )
		par_map.insert( make_pair < string, string > ( cv->label, r->label ) );		

	for ( cb = r->b; cb != NULL; cb = cb->next )
		for ( cur = cb->head; cur != NULL; cur = go_brother( cur ) )
			create_par_map( cur );
}


/***************************************************
INSERT_LABELS_MEM
****************************************************/
void insert_labels_mem( object *r, int *num_v, char *lab )
{
	bool found;
	char tag_pref[ 3 ];
	object *cur;
	variable *cv;
	bridge *cb;

	for ( found = false, cv = r->v; cv != NULL; cv = cv->next )
		if ( ( lab == NULL && cv->save ) || ( lab != NULL && ! strcmp( cv->label, lab ) ) )
		{
			if ( cv->save )
				strcpy( tag_pref, "" );
			else
			{
				found = true;
				strcpy( tag_pref, "U_" );
				cv->start = cv->last_update - cv->num_lag;
				cv->end = cv->last_update;
			}
			
			set_lab_tit( cv );
			cmd( "add_series \"%s %s%s (%d-%d) #%d\" %s", cv->label, tag_pref, cv->lab_tit, cv->start, cv->end, *num_v, cv->up->label );
			
			if ( cv->end > num_c )
				num_c = cv->end;
			
			if ( cv->start < first_c )
				first_c = cv->start;
			
			*num_v += 1;
		}
	
	for ( cb = r->b; cb != NULL && ! found; cb = cb->next )
		if ( cb->head != NULL && cb->head->to_compute )
			for ( cur = cb->head; cur != NULL; cur = cur->next )
				insert_labels_mem( cur, num_v, lab );
	 
	if ( r->up == NULL && lab == NULL )
		for ( cv = cemetery; cv != NULL; cv = cv->next )
		{  
			cmd( "add_series \"%s %s (%d-%d) #%d\" %s", cv->label, cv->lab_tit, cv->start, cv->end, *num_v, par_map[ cv->label ].c_str( ) );
			
			if ( cv->end > num_c )
				num_c = cv->end;
			
			if ( cv->start < first_c )
				first_c = cv->start;
			
			*num_v += 1;
		}
}


/***************************************************
INSERT_STORE_MEM
****************************************************/
void insert_store_mem( object *r, int *num_v, char *lab )
{
	bool found;
	char tag_pref[3];
	int i;
	object *cur;
	variable *cv;
	bridge *cb;

	for ( found = false, cv = r->v; cv != NULL; cv = cv->next )
		if ( ( lab == NULL && cv->save ) || ( lab != NULL && ! strcmp( cv->label, lab ) ) )
		{
			if ( cv->save )
				strcpy( tag_pref, "" );
			else
			{
				found = true;
				strcpy( tag_pref, "U_" );
				
				// use C stdlib to be able to deallocate memory for deleted objects
				if ( cv->data == NULL )
					cv->data = ( double * ) malloc( ( cv->num_lag + 1 ) * sizeof( double ) );
				
				if ( cv->data == NULL )
				{
					error_hard( "cannot allocate memory for unsaved series", 
								"out of memory", 
								"if there is memory available and the error persists,\nplease contact developers",
								true );
					myexit( 19 );
				}	
				
				for ( i = 0; i <= cv->num_lag; ++i )
					cv->data[ i ] = cv->val[ cv->num_lag - i ];
			}
			
			set_lab_tit( cv );
			strcpy( vs[ *num_v ].label, cv->label );
			sprintf( vs[ *num_v ].tag, "%s%s", tag_pref, cv->lab_tit );
			vs[ *num_v ].start = cv->start;
			vs[ *num_v ].end = cv->end;
			vs[ *num_v ].rank = *num_v;
			vs[ *num_v ].data = cv->data;
			
			*num_v += 1;
		}
	  
	for ( cb = r->b; cb != NULL && ! found; cb = cb->next )
		if ( cb->head != NULL && cb->head->to_compute )
			for ( cur = cb->head; cur != NULL; cur = cur->next )
				insert_store_mem( cur, num_v, lab );
	 
	if ( r->up == NULL && lab == NULL )
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
void insert_data_file( bool gz, int *num_v, vector < string > *var_names, bool keep_vars )
{
	FILE *f = NULL;
	gzFile fz = NULL;
	char ch, *tok, *linbuf, *tag;
	int i, j, new_v, new_c;
	bool header = false;
	long linsiz = 1;
	store *app;

	if ( ! gz )
		f = fopen( filename, "rt" );
	else
		fz = gzopen( filename, "rt" );

	new_v = 0;
	plog( "\nResults data from file %s (F_%d) ", "", filename, file_counter );

	if ( ! gz )
		ch = ( char ) fgetc( f );
	else
		ch = ( char ) gzgetc( fz );

	while ( ch != '\n' )
	{
		if ( ! gz )
			ch = ( char ) fgetc( f );
		else
			ch = ( char ) gzgetc( fz );

		if ( ch == '\t' )
			new_v += 1;
	   
		if ( ch == '(' )		// check for header-less .tot files
			header = true;
		
		linsiz++;				// count line size
	}

	if ( ! header )
	{
		plog( "\nError: invalid header, aborting file load\nCheck if '.tot' files where created with the -g option\n" );
		goto end;
	}

	if ( ! gz )
		fclose( f );
	else
		gzclose( fz );

	plog( "%d series", "",  new_v );

	if ( ! gz )
		f = fopen( filename, "rt" );
	else
		fz = gzopen( filename, "rt" );

	new_c = -1;
	while ( ch != EOF )
	{
		if ( ! gz )
			ch = ( char ) fgetc( f );
		else
			ch = ( char ) gzgetc( fz );

		if ( ch == '\n' )
			++new_c;
	}

	if ( ! gz )
		fclose( f );
	else
		gzclose( fz );

	cmd( ".da.pas.main.p2.scale configure -maximum %d", new_c - 1 );
	cmd( "update idletasks" );

	if ( *num_v == 0 )
		vs = new store[ new_v ];
	else
	{
		app = new store[ new_v + *num_v ];
		for ( i = 0; i < *num_v; ++i )
		{
			app[ i ] = vs[ i ];
			strcpy( app[ i ].label, vs[ i ].label );
			strcpy( app[ i ].tag, vs[ i ].tag );
		} 
		
		delete [ ] vs;
		vs = app;
	}  

	if ( ! gz )
		f = fopen( filename, "rt" );
	else
		fz = gzopen( filename, "rt" );

	linsiz = ( int ) max( linsiz, new_v * ( DBL_DIG + 4 ) ) + 1;
	linbuf = new char[ linsiz ];
	if ( linbuf == NULL )
	{
		plog( "\nError: not enough memory or invalid format, aborting file load\n" );
		goto end;
	}

	// read header line
	if ( ! gz )
		fgets( linbuf, linsiz, f );		// buffers one entire line
	else
		gzgets( fz, linbuf, linsiz );

	tok = strtok( linbuf , "\t" ); 		// prepares for parsing and get first one
	for ( i = *num_v; i < new_v + *num_v; ++i )
	{
		if ( tok == NULL )
		{
			plog( "\nError: invalid header, aborting file load\n" );
			goto end;
		}
		
		sscanf( tok, "%s %s (%d %d)", vs[ i ].label, vs[ i ].tag, &( vs[ i ].start ), &( vs[ i ].end ) );	
		vs[ i ].rank = i;

		tag = new char [ strlen( vs[ i ].tag ) + 10 ];
		sprintf( tag, "F_%d_%s", file_counter, vs[ i ].tag );
		strncpy( vs[ i ].tag, tag, MAX_ELEM_LENGTH );
		delete [ ] tag;

		if ( vs[ i ].start != -1 )
			sprintf( msg, "%s %s (%d-%d) #%d", vs[ i ].label, vs[ i ].tag, vs[ i ].start, vs[ i ].end, i );
		else
		{
			sprintf( msg, "%s %s (0-%d) #%d", vs[ i ].label, vs[ i ].tag, new_c - 1, i );
			vs[ i ].start = 0;
			vs[ i ].end = new_c - 1;
			first_c = 0;
		}
		
		var_names->push_back( msg );
		vs[ i ].data = new double[ vs[ i ].end - vs[ i ].start + 1 ];
	 
		if ( keep_vars )
		{
			if ( par_map.find( vs[ i ].label ) == par_map.end( ) )
				cmd( "add_series \"%s\" %s", msg, filename );
			else
				cmd( "add_series \"%s\" %s", msg, par_map[ vs[ i ].label ].c_str( ) );
		}
	 
		tok = strtok( NULL, "\t" );			// get next token, if any
	}
	
	cmd( "update_parent" );

	// read data lines
	for ( first_c = 1, j = 0; j < new_c; ++j )
	{
		if ( ! gz )
			fgets( linbuf, linsiz, f );		// buffers one entire line
		else
			gzgets( fz, linbuf, linsiz );
	 
		tok = strtok( linbuf , "\t" ); 		// prepares for parsing and get first one
		
		for ( i = *num_v; i < new_v + *num_v; ++i )
		{
			if ( tok == NULL )
			{
				plog( "\nError: invalid data, aborting file load\n" );
				num_c += ( j > 0 ? j - 1 : 0 ) > num_c ? ( j > 0 ? j - 1 : 0 ) : 0;
				goto end;
			}
			
	  		// ignore not started / already ended series' column
			if ( j >= vs[ i ].start && j <= vs[ i ].end )
			{
				if ( ! strcmp( tok, nonavail ) )// it's a non-available observation
					vs[ i ].data[ j - vs[ i ].start ] = NAN;
				else
				{
					sscanf( tok, "%lf", &( vs[ i ].data[ j - vs[ i ].start ] ) );
					
					if ( j == 0 )			// at least one lagged variable?
						first_c = 0;
				}
			}
			
			tok = strtok( NULL, "\t" );		// get next token, if any
		}
		
		if ( ( j + 2 ) % 10 == 0 )
			cmd( "prgboxupdate .da.pas \"\" %d", j + 1 );
	}

	*num_v += new_v;
	new_c--;
	if ( new_c > num_c )
		num_c = new_c;
	if ( new_c > max_c )
		max_c = new_c; 
	
	min_c = max( first_c, showInit ? 0 : 1 );

	end:

	if ( ! gz )
		fclose( f );
	else
		gzclose( fz );
}


/************************
STATISTICS
************************/
void statistics( int *choice )
{
	char *app, **str, **tag, str1[ 50 ], longmsg[ 300 ];
	double **data, av, var, num, ymin, ymax, sig;
	int i, j, *start, *end, *id;

	if ( nv == 0 )			// no variables selected
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return;
	}

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		
		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data; 
			if ( data[ i ] == NULL )
			{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}
	   
			if ( logs )			// apply log to the values to show "log scale" in the y-axis
				data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "statistics" );
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
			if ( j >= start[ i ] && j <= end[ i ] && is_finite( data[ i ][ j - start[ i ] ] ) )	// ignore NaNs
			{
				if ( data[ i ][ j - start[ i ] ] < ymin )
					ymin = data[ i ][ j - start[ i ] ];
				
				if ( data[ i ][ j - start[ i ] ] > ymax )
					ymax = data[ i ][ j - start[ i ] ];
				
				av += data[ i ][ j - start[ i ] ];
				var += data[ i ][ j - start[ i ] ] * data[ i ][ j - start[ i ] ];
				
				++num;
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
			delete [ ] data[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
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
	bool first;
	char *app, **str, **tag, str1[ 50 ], longmsg[ 300 ];
	double **data, av, var, num, ymin = 0, ymax = 0, sig;
	int i, j, h, k, nt, *start, *end, *id, *list_times;

	Tcl_LinkVar( inter, "nt", ( char * ) &nt, TCL_LINK_INT );
	cmd( "if [ info exists num_t ] { set nt $num_t } { set nt \"-1\" }" );
	Tcl_UnlinkVar( inter, "nt" );

	if ( nv < 2 || nt <= 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place at least two series in the Series Selected listbox and select at least one case (time step).\"" );
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
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		
		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		data[ i ] = vs[ id[ i ] ].data;
		if ( data[ i ] == NULL )
		{
			error_hard( "invalid series data", 
						"internal problem in LSD", 
						"if error persists, please contact developers",
						true );
			myexit( 18 );
		}
	   
		if ( logs )			// apply log to the values to show "log scale" in the y-axis
			data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "statistics" );
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
			if ( h >= start[ i ] && h <= end[ i ] && is_finite( data[ i ][ h - start[ i ] ] ) )		// ignore NaNs
			{
				if ( first )
				{
					ymin = ymax = data[ i ][ h - start[ i ] ];
					first = false;
				}
				
				if ( data[ i ][ h - start[ i ] ] < ymin )
					ymin = data[ i ][ h - start[ i ] ];
				
				if ( data[ i ][ h - start[ i ] ] > ymax )
					ymax = data[ i ][ h - start[ i ] ];
				
				av += data[ i ][ h - start[ i ] ];
				var += data[ i ][ h - start[ i ] ] * data[ i ][ h - start[ i ] ];
				
				++num;
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
			delete [ ] data[ i ];
	}

	delete [ ] list_times;
	delete [ ] str;
	delete [ ] tag;
	delete [ ] data;
	delete [ ] start;
	delete [ ] end;
	delete [ ] id;
}


/***************************************************
PLOT_GNU
Draws the XY plots, with the first series as X and the others as Y's
****************************************************/
void plot_gnu( int *choice )
{
	bool done;
	char *app, **str, **tag, str1[ 50 ], str2[ 100 ], str3[ 10 ], dirname[ MAX_PATH_LENGTH ];
	double **data;
	int i, j, box, ndim, gridd, *start, *end, *id, nanv = 0;
	FILE *f, *f2;

	if ( nv == 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}

	if ( nv > 2 )
	{
		cmd( "newtop .da.s \"XY Plot Options\" { set choice 2 } .da" );

		cmd( "ttk::frame .da.s.t" );
		cmd( "ttk::label .da.s.t.l -text \"Plot type\"" );

		cmd( "ttk::frame .da.s.t.d -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "if { ! [ info exists ndim ] } { set ndim 2 }" );
		cmd( "ttk::radiobutton .da.s.t.d.2d -text \"2D plot\" -variable ndim -value 2 -command { .da.s.d.o.a configure -state disabled; .da.s.d.o.c configure -state disabled; .da.s.d.o.b configure -state disabled; .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set box 0; set gridd 0; set pm3d 0 }" );
		cmd( "ttk::radiobutton .da.s.t.d.3d -text \"3D plot \" -variable ndim -value 3 -command { .da.s.d.o.a configure -state normal; .da.s.d.o.c configure -state normal; .da.s.d.o.b configure -state normal; .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );
		cmd( "pack .da.s.t.d.2d .da.s.t.d.3d -anchor w" );

		cmd( "pack .da.s.t.l .da.s.t.d" );

		cmd( "ttk::frame .da.s.d" );
		cmd( "ttk::label .da.s.d.l -text \"3D XY plane options\"" );

		cmd( "ttk::frame .da.s.d.o -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "if { ! [ info exists box ] } { set box 0 }" );
		cmd( "ttk::radiobutton .da.s.d.o.a -text \"Use 1st and 2nd series\" -variable box -value 0" );
		cmd( "ttk::radiobutton .da.s.d.o.c -text \"Use time and 1st series\" -variable box -value 2" );
		cmd( "ttk::radiobutton .da.s.d.o.b -text \"Use time and rank\" -variable box -value 1" );
		cmd( "pack .da.s.d.o.a .da.s.d.o.c .da.s.d.o.b -anchor w" );

		cmd( "pack .da.s.d.l .da.s.d.o" );

		cmd( "ttk::frame .da.s.o" );
		cmd( "ttk::checkbutton .da.s.o.g -text \"Use gridded data\" -variable gridd" );
		cmd( "ttk::checkbutton .da.s.o.p -text \"Render 3D surface\" -variable pm3d" );
		cmd( "pack .da.s.o.g .da.s.o.p" );

		cmd( "pack .da.s.t .da.s.d .da.s.o -padx 5 -pady 5" );

		cmd( "if { $ndim == 2 } { .da.s.d.o.a configure -state disabled; .da.s.d.o.c configure -state disabled; .da.s.d.o.b configure -state disabled; .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set box 0; set gridd 0; set pm3d 0 } { .da.s.d.o.a configure -state normal; .da.s.d.o.c configure -state normal; .da.s.d.o.b configure -state normal; .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );

		cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#3dTime } { set choice 2 }" );

		cmd( "showtop .da.s" );
		cmd( "mousewarpto .da.s.b.ok" );

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
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		
		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
			{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}

			if ( logs )		// apply log to the values to show "log scale" in the y-axis
				data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "Gnuplot" );
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
				min_c = max_c = max( start[ i ], showInit ? 0 : 1 );
			
			if ( start[ i ] < min_c )
				min_c = max( start[ i ], showInit ? 0 : 1 );
			
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
				if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) )	// ignore NaNs
				{
					miny = maxy = data[ i ][ j - start[ i ] ];
					done = true;
				}
				
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] < miny )	// ignore NaNs
					miny = data[ i ][ j - start[ i ] ];
					
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] > maxy )	// ignore NaNs
					maxy = data[ i ][ j - start[ i ] ];
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
						fprintf( f, "%.*g\t", pdigits, data[ i ][ j - start[ i ] ] );
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
								fprintf( f, "%d\t%.*g\t", i + 1, pdigits, data[ i ][ j - start[ i ] ] );
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
								fprintf( f, "%.*g\t", pdigits, data[ i ][ j - start[ i ] ] );
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
								fprintf( f, "%d\t%d\t%.*g\n", j, i + 1, pdigits, data[ i ][ j - start[ i ] ] );
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
								fprintf( f, "%d\t%.*g\n", j, pdigits, data[ i ][ j - start[ i ] ] );
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
							sprintf( str2, ", 'data.gp' using 1:%d:%d %s t \"\"", 2 + 2 * i, 2 * i + 3, str1 ); 
						else
							if ( i <= nv / 2 )
								sprintf( str2, ", 'data.gp' using 1:%d:%d %s t \"\"", i + 1, ( nv - nanv ) / 2 + i + 1, str1 ); 
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
			delete [ ] data[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
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
	bool done;
	char *app, **str, **tag, str1[ TCL_BUFF_STR ], str2[ 5 * MAX_ELEM_LENGTH ], str3[ MAX_ELEM_LENGTH ], dirname[ MAX_PATH_LENGTH ];
	double **data, previous_row;
	int i, j, time_sel, block_length, ndim, *start, *end, *id;
	FILE *f, *f2;

	if ( nv < 2 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Not enough series selected\" -detail \"Place at least two series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		
		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
			{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}
	   
			if ( logs )		// apply log to the values to show "log scale" in the y-axis
				data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "plot" );
		}
	}

	// handle case selection
	if ( autom_x || min_c >= max_c )
	{
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = max( start[ i ], showInit ? 0 : 1 );
			
			if ( start[ i ] < min_c )
				min_c = max( start[ i ], showInit ? 0 : 1 );
			
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
				if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) )	// ignore NaNs
				{
					miny = maxy = data[ i ][ j - start[ i ] ];
					done = true;
				}
				
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] < miny )	// ignore NaNs
					miny = data[ i ][ j - start[ i ] ];
					
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] > maxy )	// ignore NaNs
					maxy = data[ i ][ j - start[ i ] ];
			}

	cmd( "set bidi %d", end[ 0 ] );

	cmd( "newtop .da.s \"XY Plot Options\" { set choice 2 } .da" );

	cmd( "ttk::frame .da.s.i" );
	cmd( "ttk::label .da.s.i.l -text \"Case\"" );
	cmd( "ttk::entry .da.s.i.e -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e" );

	cmd( "ttk::frame .da.s.d" );
	cmd( "ttk::label .da.s.d.l -text \"Type\"" );

	cmd( "ttk::frame .da.s.d.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "if { ! [ info exists ndim ] } { set ndim 2 }" );
	cmd( "ttk::radiobutton .da.s.d.r.2d -text \"2D plot\" -variable ndim -value 2 -command  { .da.s.o.g configure -state disabled; .da.s.o.p configure -state disabled; set gridd 0; set pm3d 0 }" );
	cmd( "ttk::radiobutton .da.s.d.r.3d -text \"3D plot\" -variable ndim -value 3 -command  { .da.s.o.g configure -state normal; .da.s.o.p configure -state normal }" );
	cmd( "pack .da.s.d.r.2d .da.s.d.r.3d -anchor w" );

	cmd( "pack .da.s.d.l .da.s.d.r" );

	cmd( "ttk::frame .da.s.o" );
	cmd( "ttk::label .da.s.o.l -text \"3D options\"" );
	cmd( "ttk::frame .da.s.o.opt" );
	cmd( "ttk::checkbutton .da.s.o.opt.g -text \"Use gridded data\" -variable gridd" );
	cmd( "ttk::checkbutton .da.s.o.opt.p -text \"Render 3D surface\" -variable pm3d" );
	cmd( "if { $ndim == 2 } { .da.s.o.opt.g configure -state disabled; .da.s.o.opt.p configure -state disabled; set gridd 0; set pm3d 0 } { .da.s.o.opt.g configure -state normal; .da.s.o.opt.p configure -state normal }" );
	cmd( "pack .da.s.o.opt.g .da.s.o.opt.p -anchor w" );
	cmd( "pack .da.s.o.l .da.s.o.opt" );

	cmd( "set choice $ndim" );
	if ( *choice == 2 )
		cmd( "set blength %d", ( int )( nv / 2 ) );
	else
		cmd( "set blength %d", ( int )( nv / 3 ) ); 

	cmd( "set numv 1" );
	cmd( "ttk::frame .da.s.v" );
	cmd( "ttk::label .da.s.v.l -text \"Dependent variables\"" );
	cmd( "ttk::spinbox .da.s.v.e -width 5 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= %d } { set numv %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numv; return 0 } } -invalidcommand { bell } -justify center", nv, nv );
	cmd( ".da.s.v.e insert 0 $numv" ); 
	cmd( "ttk::label .da.s.v.n -text \"Block length: $blength\"" );
	cmd( "pack .da.s.v.l .da.s.v.e .da.s.v.n" );

	cmd( "pack .da.s.i .da.s.d .da.s.o .da.s.v -padx 5 -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#3dCrossSection } { set choice 2 }" );

	cmd( "bind .da.s.v.e <KeyRelease> { set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]; .da.s.v.n conf -text \"Block length: $blength\" }" );
	cmd( "set nnvar %d", nv );
	cmd( "bind .da.s.v.e <Return> { set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]; .da.s.v.n conf -text \"Block length: $blength\" }" );
	cmd( "bind .da.s.v.e <Tab> { set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]; .da.s.v.n conf -text \"Block length: $blength\" }" );
	cmd( "bind .da.s.d.r.2d <Return> { set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]; .da.s.v.n conf -text \"Block length: $blength\" }" );
	cmd( "bind .da.s.d.r.2d <ButtonRelease-1> { set ndim 2; set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]; .da.s.v.n conf -text \"Block length: $blength\" }" );
	cmd( "bind .da.s.d.r.3d <ButtonRelease-1> { set ndim 3; set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]; .da.s.v.n conf -text \"Block length: $blength\" }" );
	cmd( "bind .da.s.d.r.2d <Down> { .da.s.d.r.3d invoke; focus .da.s.d.r.3d; set ndim 3; set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]; .da.s.v.n conf -text \"Block length: $blength\" }" );
	cmd( "bind .da.s.d.r.3d <Up> { .da.s.d.r.2d invoke; focus .da.s.d.r.2d; set ndim 2; set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]; .da.s.v.n conf -text \"Block length: $blength\" }" );
	cmd( "bind .da.s <KeyPress-Return> { set choice 1 }" );
	cmd( "bind .da.s <KeyPress-Escape> { set choice 2 }" );
	cmd( "bind .da.s.i.e <KeyPress-Return> { if { $ndim == 2 } { focus .da.s.d.r.2d } { focus .da.s.d.r.3d } }" );
	cmd( "bind .da.s.d.r.2d <KeyPress-Return> { .da.s.v.e selection range 0 end; focus .da.s.v.e }" );
	cmd( "bind .da.s.d.r.3d <KeyPress-Return> { .da.s.v.e selection range 0 end; focus .da.s.v.e }" );
	cmd( "bind .da.s.v.e <KeyPress-Return> { focus .da.s.b.ok }" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.i.e" );
	cmd( ".da.s.i.e selection range 0 end" );
	cmd( "mousewarpto .da.s.b.ok" );

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

	cmd( "set blength [ expr { $nnvar / ( $numv + $ndim - 1 ) } ]" );
	cmd( "set choice $blength" );

	block_length = *choice;
	*choice = 0;

	if ( block_length <= 0 || nv % block_length != 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid block length\" -detail \"Block length should be an exact divisor of the number of variables. You may also try a 3D plot.\"" );
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
		previous_row = data[ 0 ][ end[ 0 ] - start[ 0 ] ];
	else
		previous_row = data[ 0 ][ time_sel - start[ 0 ] ];  
	  
	for ( j = 0; j < block_length; ++j )
	{

		if ( start[ 0 ] == end[ 0 ] )  
		{
			if ( data[ j ][ end[ j ] - start[ j ] ] != previous_row )
				previous_row = data[ j ][ end[ j ] - start[ j ] ];
		}
		else
		{
			if ( data[ j ][ time_sel - start[ j ] ] != previous_row )
				previous_row = data[ j ][ time_sel - start[ j ] ];
		}
		 
		for ( i = 0; i < nv; i += block_length )
			if ( start[ i + j ] == end[ i + j ] )  
				fprintf( f, "%.*g\t", pdigits, data[ i + j ][ end[ i + j ] - start[ i + j ] ] );
			else
				if ( start[ i + j ] <= max_c && end[ i + j ] >= min_c && start[ i + j ] <= time_sel && end[ i + j ] >= time_sel )
					fprintf( f, "%.*g\t", pdigits, data[ i + j ][ time_sel - start[ i + j ] ] );

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
		sprintf( msg, "splot 'data.gp' using 1:2:3 %s t \"%s_%s(%d)\"", str2, str[ 2 * block_length ], tag[ 2 * block_length ], time_sel ); 
		
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
			delete [ ] data[ i ];
	}

	delete [ ] str;
	delete [ ] tag;
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
	bool done;
	char *app, **str, **tag, str1[ 50 ], str2[ 100 ], str3[ 100 ], dirname[ MAX_PATH_LENGTH ];
	double **data;
	int i, j, nlags, *start, *end, *id;
	FILE *f, *f2;

	if ( nv != 1 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"One and only one series must be selected.\"" );
		*choice = 2;
		return;
	}

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		
		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		// get series data and take logs if necessary
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
			{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}
	   
			if ( logs )			// apply log to the values to show "log scale" in the y-axis
				data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "plot" );
		}
	}

	// handle case selection
	if ( autom_x || min_c >= max_c )
		for ( i = 0; i < nv; ++i )
		{
			if ( i == 0 )
				min_c = max_c = max( start[ i ], showInit ? 0 : 1 );
			
			if ( start[ i ] < min_c )
				min_c = max( start[ i ], showInit ? 0 : 1 );
			
			if ( end[ i ] > max_c )
				max_c = end[ i ] > num_c ? num_c : end[ i ];
		}

	// auto-find minimums and maximums
	if ( miny >= maxy )
		autom = true;

	if ( autom )
		for ( done = false, i = 0; i < nv; ++i )
			for ( j = min_c; j <= max_c; ++j )
			{
				if ( ! done && start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) )		// ignore NaNs
				{
					miny = maxy = data[ i ][ j - start[ i ] ];
					done = true;
				}
				
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] < miny )		// ignore NaNs
					miny = data[ i ][ j - start[ i ] ];
					
				if ( start[ i ] <= j && end[ i ] >= j && is_finite( data[ i ][ j - start[ i ] ] ) && data[ i ][ j - start[ i ] ] > maxy )		// ignore NaNs
					maxy = data[ i ][ j - start[ i ] ];
			}
		
	cmd( "newtop .da.s \"Lag Selection\" { set choice 2 } .da" );

	cmd( "ttk::frame .da.s.i" );
	cmd( "ttk::label .da.s.i.l -text \"Lags\"" );
	cmd( "set bidi 1" );
	cmd( "ttk::spinbox .da.s.i.e -width 5 -from 1 -to $maxc -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e" );

	cmd( "if { ! [ info exists dia ] } { set dia 0 }" );
	cmd( "ttk::checkbutton .da.s.arrow -text \"Plot 45\u00B0 diagonal\" -variable dia" );
	cmd( "pack .da.s.i .da.s.arrow -padx 5 -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#phaseplot } { set choice 2 }" );

	cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
	cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );
	cmd( "mousewarpto .da.s.b.ok" );

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
				fprintf( f, "%lf\t", data[ 0 ][ j + i - start[ 0 ] ] );

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
			delete [ ] data[ i ];
	}
	
	delete [ ] str;
	delete [ ] tag;
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
			
	cmd( "ttk::messageBox -parent .da -type ok -icon warning -title Warning -message \"Unsupported options for the internal graphical window\" -detail \"Gnuplot automatically selected to support the chosen 3D plot options.\"" );
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
	hsize = get_int( "hsizePxy" );			// 640
	vsize = get_int( "vsizePxy" );			// 450
	sbordsize = get_int( "sbordsizeP" );	// 0


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

	// create plot notepad window, tab & canvas
	add_da_plot_tab( ".dap", n );    		// plot tab window

	cmd( "set p $w.f.plots" );				// plot canvas

	cmd( "ttk::frame $w.f" );
	cmd( "ttk::scrollbar $w.f.vs -command \"$p yview\"" );
	cmd( "ttk::scrollbar $w.f.hs -orient horiz -command \"$p xview\"" );
	cmd( "pack $w.f.vs -side right -fill y" );
	cmd( "pack $w.f.hs -side bottom -fill x" );

	cmd( "ttk::canvas $p -width %d -height %d -yscrollcommand \"$w.f.vs set\" -xscrollcommand \"$w.f.hs set\" -scrollregion \"%d %d %d %d\" -dark $darkTheme", 
			hsize, vsize, - sbordsize, - sbordsize, hsize + sbordsize, vsize + sbordsize  );

	cmd( "pack $p -expand yes -fill both" );
	cmd( "pack $w.f -expand yes -fill both" );
	cmd( "mouse_wheel $p" );

	// add buttons bottom bar
	cmd( "ttk::frame $w.b" );

	cmd( "ttk::frame $w.b.c" );
	
	cmd( "ttk::frame $w.b.c.case" );
	
	cmd( "set labWid 15; set datWid 30" );

	if ( logs )
		cmd( "ttk::label $w.b.c.case.l -text \"log(X) value:\" -width $labWid -anchor e" );
	else
		cmd( "ttk::label $w.b.c.case.l -text \"X value:\" -width $labWid -anchor e" );
	cmd( "ttk::label $w.b.c.case.v -text \"\" -style hl.TLabel -width $datWid -anchor w" );
	cmd( "pack $w.b.c.case.l $w.b.c.case.v -side left -anchor w" );

	cmd( "ttk::frame $w.b.c.y" );
	if ( logs )
		cmd( "ttk::label $w.b.c.y.l -text \"log(Y) value:\" -width $labWid -anchor e" );
	else
		cmd( "ttk::label $w.b.c.y.l -text \"Y value:\" -width $labWid -anchor e" );
	cmd( "ttk::label $w.b.c.y.v1 -text \"\" -style hl.TLabel -width $datWid -anchor w" );
	cmd( "pack $w.b.c.y.l $w.b.c.y.v1 -side left -anchor w" );

	cmd( "pack $w.b.c.case $w.b.c.y -anchor w" );

	cmd( "ttk::frame $w.b.o" );
	cmd( "ttk::label $w.b.o.l1 -text \"Alt-click: properties\"" );
	cmd( "ttk::label $w.b.o.l2 -text \"Shift-click: add text\"" );
	cmd( "ttk::label $w.b.o.l3 -text \"Ctrl-click: add line\"" );
	cmd( "pack $w.b.o.l1 $w.b.o.l2 $w.b.o.l3" );

	cmd( "ttk::frame $w.b.s" );
	cmd( "ttk::button $w.b.s.save -width $butWid -text Save -state disabled -command { \
			set it \"%d) %s\"; \
			set fromPlot 1; \
			set choice 11 \
		}", n, ( char * ) Tcl_GetVar( inter, "tit", 0 ) );
	cmd( "ttk::button $w.b.s.gnu -width $butWid -text Gnuplot -state disabled -command { \
			set oldpath [pwd]; \
			cd plotxy_%d; \
			open_gnuplot gnuplot.gp; \
			cd $oldpath; \
		}", n );
	cmd( "pack $w.b.s.save $w.b.s.gnu -pady 4" );

	cmd( "ttk::label $w.b.pad -width 6" );

	cmd( "ttk::frame $w.b.z" );
	cmd( "ttk::label $w.b.z.l -text Zoom" );

	cmd( "ttk::frame $w.b.z.b" );
	cmd( "ttk::button $w.b.z.b.p -width 3 -text + -command { scale_canvas $daptab.tab%d.c.f.plots \"+\" zoomLevel%d } -state disabled", n, n );
	cmd( "ttk::button $w.b.z.b.m -width 3 -text - -command { scale_canvas $daptab.tab%d.c.f.plots \"-\" zoomLevel%d } -state disabled", n, n  );
	cmd( "pack $w.b.z.b.p $w.b.z.b.m -pady 4" );

	cmd( "pack  $w.b.z.l $w.b.z.b -side left -padx 2" );

	cmd( "pack $w.b.c $w.b.o $w.b.pad $w.b.s $w.b.z -padx 10 -pady 5 -side left" );
	cmd( "pack $w.b -side right" );

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
	cmd( "set cmx [ expr { [ winfo width $p ] - 2 * [ $p cget -border ] - 2 * [ $p cget -highlightthickness ] } ]" );
	cmd( "if { $cmx <= 1 } { set cmx [ $p cget -width ] }" );
	cmd( "set cmy [ expr { [ winfo height $p ] - 2 * [ $p cget -border ] - 2 * [ $p cget -highlightthickness ] } ]" );
	cmd( "if { $cmy <= 1 } { set cmy [ $p cget -height ] }" );
	cmd( "unset -nocomplain lim rang" );
	cmd( "catch { set lim [ gnuplot_plotarea ] }" );
	cmd( "catch { set rang [ gnuplot_axisranges ] }" );
	cmd( "if { [ info exists lim ] && [ info exists rang ] } { set choice 1 } { set choice 0 }" );
	
	if ( *choice == 1 )
	{
		cmd( "set res [ expr { int( $cmx * [ lindex $lim 0 ] / 1000.0 ) } ]" );
		lim[ 0 ] = get_int( "res" );
		cmd( "set res [ expr { int( $cmx * [ lindex $lim 1 ] / 1000.0 ) } ]" );
		lim[ 1 ] = get_int( "res" );
		cmd( "set res [ expr { int( $cmy * [ lindex $lim 2 ] / 1000.0 ) } ]" );
		lim[ 2 ] = get_int( "res" );
		cmd( "set res [ expr { int( $cmy * [ lindex $lim 3 ] / 1000.0 ) } ]" );
		lim[ 3 ] = get_int( "res" );
		
		for ( i = 0; i < 4; ++i )
		{	
			cmd( "set res [ lindex $rang %d ]", i );
			rang[ i ] = get_double( "res" );

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
			set w $daptab.tab%d.c; \
			set llim [ expr { %d * $zoom } ]; \
			set rlim [ expr { %d * $zoom } ]; \
			set tlim [ expr { %d * $zoom } ]; \
			set blim [ expr { %d * $zoom } ]; \
			set cx [ $w.f.plots canvasx %%x ]; \
			set cy [ $w.f.plots canvasy %%y ]; \
			if { $cx >= $llim && $cx <= $rlim && $cy >= $tlim && $cy <= $blim && [ expr { $rlim - $llim } ] > 0 && [ expr { $blim - $tlim } ] > 0 } { \
				$w.b.c.case.v configure -text [ format \"%%%%.${pdigits}g\" [ expr { ( $cx - $llim ) * ( %lf - %lf ) / ( $rlim - $llim ) + %lf } ] ]; \
				$w.b.c.y.v1 configure -text [ format \"%%%%.${pdigits}g\" [ expr { ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf } ] ]; \
			} \
		}", n, n, n, lim[ 0 ], lim[ 1 ], lim[ 2 ], lim[ 3 ], rang[ 1 ], rang[ 0 ], rang[ 0 ], rang[ 3 ], rang[ 2 ], rang[ 2 ] );

	// create context menu and common bindings
	canvas_binds( n );
	
	cmd( "update idletasks" );

	// save plot info
	type_plot[ n ] = GNUPLOT; 	// Gnuplot plot
	plot_w[ n ] = hsize;		// plot width
	plot_l[ n ] = vsize; 		// height of plot with labels
	plot_nl[ n ] = vsize; 		// height of plot without labels   
	 
	*choice = 0;
}


/***************************************************
PLOT_LATTICE
****************************************************/
void plot_lattice( int *choice )
{
	char *app, **str, **tag;
	double val, color, cscale, **data;
	int i, j, hi, le, first = 1, last, time, hsize, vsize, ncol, nlin, tot, *start, *end, *id;

	if ( nv == 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		*choice = 2;
		return;
	}

	if ( time_cross == 0 && nv > 1 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For time series lattices select only one series.\"" );
		*choice = 2;
		return;
	}

	if ( time_cross == 1 && nv < 2 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For cross-section lattices select at least two series.\"" );
		*choice = 2;
		return;
	}

	// lattice window size
	hsize = get_int( "hsizeLat" );			// 420
	vsize = get_int( "vsizeLat" );			// 420
	cscale = get_double( "cscaleLat" );		// 1.0

	// find column number suggestion
	tot = time_cross == 1 ? nv : max_c - min_c + 1;
	ncol = ( int ) max( sqrt( tot ), 1 );
	while ( tot % ncol != 0 && ncol > 0 )
		ncol--;

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	cmd( "set time %d", num_c );
	cmd( "set bidi %d", ncol );
	cmd( "set cscale [ format %%.2f %lf ]", cscale );

	cmd( "newtop .da.s \"Lattice Options\" { set choice 2 } .da" );

	cmd( "ttk::frame .da.s.t" );
	cmd( "ttk::label .da.s.t.l -width 22 -anchor e -text \"Cross-section case\"" );
	cmd( "ttk::entry .da.s.t.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= $numc } { set time %%P; return 1 } { %%W delete 0 end; %%W insert 0 $time; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( ".da.s.t.e insert 0 $time" ); 
	cmd( "pack .da.s.t.l .da.s.t.e -side left -anchor w -padx 2 -pady 2" );

	cmd( "ttk::frame .da.s.i" );
	cmd( "ttk::label .da.s.i.l -width 22 -anchor e -text \"Data columns\"" );
	cmd( "ttk::spinbox .da.s.i.e -width 5 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invalidcommand { bell } -justify center", num_c );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e -side left -anchor w -padx 2 -pady 2" );

	cmd( "ttk::frame .da.s.s" );
	cmd( "ttk::label .da.s.s.l -width 22 -anchor e -text \"Color scale\"" );
	cmd( "ttk::entry .da.s.s.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] && $n > 0 } { set cscale %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cscale; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( ".da.s.s.e insert 0 $cscale" ); 
	cmd( "pack .da.s.s.l .da.s.s.e -side left -anchor w -padx 2 -pady 2" );

	if ( time_cross == 1 )
	{
		cmd( "pack .da.s.t .da.s.i .da.s.s -anchor w -padx 5 -pady 5" );
		cmd( "bind .da.s.t.e <KeyPress-Return> { focus .da.s.i.e; .da.s.i.e selection range 0 end }" );
		cmd( "focus .da.s.t.e; .da.s.t.e selection range 0 end" );
	}
	else
	{
		cmd( "pack .da.s.i .da.s.s -anchor w -padx 5 -pady 5" );
		cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );
	}

	cmd( "bind .da.s.i.e <KeyPress-Return> { focus .da.s.s.e; .da.s.s.e selection range 0 end }" );
	cmd( "bind .da.s.s.e <KeyPress-Return> { set choice 1 }" );
		
	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#lattice } { set choice 2 }" );

	cmd( "showtop .da.s" );
	cmd( "mousewarpto .da.s.b.ok" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set time [ .da.s.t.e get ]" ); 
	cmd( "set bidi [ .da.s.i.e get ]" ); 
	cmd( "set cscale [ .da.s.s.e get ]" ); 
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

		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
	  
		// check if series has data for all selected cases (cross-section only )
		if ( time_cross == 1 && ( time < start[ i ] || time > end[ i ] ) )
		{
			cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid case\" -detail \"One or more of the series do not have a value associated to the selected cross-section case (time step).\"" );
			*choice = 2;
			
			for ( j = i + 1; j < nv; ++j )		// indicate non allocated positions
				str[ j ] = tag[ j ] = NULL;
			
			goto end1;
		}
		
		// get series data and take logs if necessary
		data[ i ] = vs[ id[ i ] ].data;
		if ( data[ i ] == NULL )
		{
			error_hard( "invalid series data", 
						"internal problem in LSD", 
						"if error persists, please contact developers",
						true );
			myexit( 18 );
		}
	  
		if ( logs )			// apply log to the values to show "log scale"
			data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "lattice" );
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
			if ( min_c > max( start[ 0 ], showInit ? 0 : 1 ) )
				first = min_c;
			else
				first = max( start[ 0 ], showInit ? 0 : 1 );
			
			if ( max_c < end[ 0 ] )  
				last = max_c;
			else
				last = end[ 0 ];
		}
		
		for ( tot = 0, i = first; i <= last; ++i )	// count number of points excluding NaNs
			if ( ! is_nan( data[ 0 ][ i - start[ 0 ] ] ) && is_finite( data[ 0 ][ i - start[ 0 ] ] ) )
				tot++;
	}
	else
		tot = nv;

	nlin = tot / ncol;

	if ( nlin * ncol != tot )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of columns\" -detail \"The number of columns must be an exact divisor of the number (%d) of cases/time steps (time series) or selected variables (cross section).\"", tot );
		*choice = 2;
		goto end2;
	}

	// draw lattice
	hi = vsize / nlin;
	le = hsize / ncol;
	cscale = get_double( "cscale" );

	add_da_plot_tab( ".dap", cur_plot );    // plot tab window

	cmd( "ttk::frame $w.f -width %d -height %d", ncol * le + 1, nlin * hi + 1 );
	cmd( "set p $w.f.plots" );
	cmd( "ttk::canvas $p -width %d -height %d -entry 0 -dark $darkTheme", ncol * le + 1, nlin * hi + 1 );
	cmd( "pack $p -anchor c" );
	cmd( "pack $w.f" );
	
	// add buttons bottom bar
	cmd( "ttk::frame $w.b -width %d", ncol * le + 1 );

	cmd( "ttk::frame $w.b.o" );
	cmd( "ttk::label $w.b.o.l1 -text \"Alt-click: properties\"" );
	cmd( "ttk::label $w.b.o.l2 -text \"Shift-click: add text\"" );
	cmd( "ttk::label $w.b.o.l3 -text \"Ctrl-click: add line\"" );
	cmd( "pack $w.b.o.l1 $w.b.o.l2 $w.b.o.l3" );

	cmd( "ttk::frame $w.b.s" );
	cmd( "ttk::button $w.b.s.save -width $butWid -text Save -command { \
			set it \"%d) %s\"; \
			set fromPlot 1; \
			set choice 11 \
		}", cur_plot, ( char * ) Tcl_GetVar( inter, "tit", 0 ) );
	cmd( "ttk::button $w.b.s.det -width $butWid -text Detach -command { \
			detach_tab $daptab tab%d c.b.s.det .da %d \
		}", cur_plot, MAX_TAB_LEN - 1 );
	cmd( "pack $w.b.s.save $w.b.s.det -pady 3" );
	
	cmd( "ttk::label $w.b.pad -width 30" );

	cmd( "pack $w.b.o $w.b.pad $w.b.s -padx 10 -pady 5 -side left" );
	cmd( "pack $w.b" );

	for ( j = 0; j < nlin; ++j )
		for ( i = 0; i < ncol; ++i )
		{
			val = time_cross == 1 ? data[ ncol * j + i ][ time - start[ ncol * j + i ] ] : 
									data[ 0 ][ first + ncol * j + i - start[ 0 ] ];
			color = max( 0, min( 1099, round( val * cscale ) ) );
			if ( is_nan( color ) || ! is_finite( color ) )
			  color = 0;

			cmd( "plot_bars $p %d %d %d %d p%d_%d $c%d %lf", 
			   i * le, j * hi, ( i + 1 ) * le, ( j + 1 ) * hi, j, i, 
			   ( int ) color, grid ? point_size : 0.0 );
		}

	// create context menu and common bindings
	canvas_binds( cur_plot );
	
	cmd( "update idletasks" );

	// save plot info
	type_plot[ cur_plot ] = LATTICE;
	plot_w[ cur_plot ] = ncol * le + 1;	// plot width
	plot_l[ cur_plot ] = nlin * hi + 1;
	plot_nl[ cur_plot ] = nlin * hi + 1;  

	end2:
	
	*choice = 0;

	for ( i = 0; i < nv; ++i )
	{
		delete [ ] str[ i ];
		delete [ ] tag[ i ];
		
		if ( logs )
			delete [ ] data[ i ];
	}

	end1:

	delete [ ] str;
	delete [ ] tag;
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
	bool norm;
	char *app, **str, **tag;
	double mx = 0, mn = 0, step, a, lminy, lmaxy, *data;
	int i, j, first, last, stat, start, end, id;

	if ( nv != 1 )
	{
		if ( nv == 0 )			// no variables selected
			cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one series in the Series Selected listbox.\"" );
		else
			cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For time series histograms select only one series.\"" );
		*choice = 2;
		return;
	} 
	 
	str = new char *[ 1 ];
	tag = new char *[ 1 ];
	str[ 0 ] = new char[ MAX_ELEM_LENGTH ];
	tag[ 0 ] = new char[ MAX_ELEM_LENGTH ];

	cmd( "set res [ .da.vars.ch.f.v get 0 ]" );
	app = ( char * ) Tcl_GetVar( inter, "res", 0 );
	strcpy( msg, app );
	sscanf( msg, "%s %s (%d-%d) #%d", str[ 0 ], tag[ 0 ], &start, &end, &id );

	data = vs[ id ].data;
	if ( data == NULL )
	{
		error_hard( "invalid series data", 
					"internal problem in LSD", 
					"if error persists, please contact developers",
					true );
		myexit( 18 );
	}

	if ( logs )			// apply log to the values to show "log scale" in the y-axis
		data = log_data( data, start, end, -1, "histogram" );

	if ( autom_x || min_c >= max_c )
	{
		first = max( start, showInit ? 0 : 1 );
		last = end;
	}
	else
	{
		if ( min_c > max( start, showInit ? 0 : 1 ) )
			first = min_c;
		else
			first = max( start, showInit ? 0 : 1 );
		
		if ( max_c < end )  
			last = max_c;
		else
			last = end;
	}  

	for ( j = 0, i = first; i <= last; ++i )	// count number of points excluding NaNs
		if ( ! is_nan( data[ i - start ] ) && is_finite( data[ i - start ] ) )
			++j;

	cmd( "set bidi %d", j < 25 ? j : 25 );
	cmd( "set norm 0" );
	cmd( "set stat 0" );
	
	cmd( "newtop .da.s \"Histogram Options\" { set choice 2 } .da" );

	cmd( "ttk::frame .da.s.i" );
	cmd( "ttk::label .da.s.i.l -text \"Classes/bins\"" );
	cmd( "ttk::spinbox .da.s.i.e -width 5 -from $minc -to $maxc -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= $minc && $n <= $maxc } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e -side left -padx 2" );

	cmd( "ttk::frame .da.s.o" );
	cmd( "ttk::checkbutton .da.s.o.norm -text \"Fit a Normal\" -variable norm" );
	cmd( "ttk::checkbutton .da.s.o.st -text \"Show statistics\" -variable stat" );
	cmd( "pack .da.s.o.norm .da.s.o.st -anchor w" );

	cmd( "pack .da.s.i .da.s.o -pady 10" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#histogram } { set choice 2 }" );

	cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.i.e; .da.s.i.e selection range 0 end" );
	cmd( "mousewarpto .da.s.b.ok" );

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
		if ( is_nan( data[ i - start ] ) || ! is_finite( data[ i - start ] ) )	// ignore NaNs
			continue;
			
		if ( i == first )
			mx = mn = data[ i - start ];
		else
		{
			if ( data[ i - start ] > mx )
				mx = data[ i - start ];
			else
				if ( data[ i - start ] < mn )
					mn = data[ i - start ];
		}  
		
		mean += data[ i - start ];
		var += data[ i - start ] * data[ i - start ];
		
		++cases;
	}

	if ( cases == 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid data\" -detail \"The selected series has no valid data for the chosen cases (time steps).\"" );
		*choice = 2;
		goto end;
	}

	if ( mx - mn <= 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid data\" -detail \"The selected series has no data variation for the chosen cases (time steps).\"" );
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
		if ( is_nan( data[ i - start ] ) || ! is_finite( data[ i - start ] ) )
			continue;

		a = floor( num_bins*( data[ i - start ] - mn ) / ( mx - mn ) );

		j = ( int ) a;
		if ( j == num_bins)
			j--;

		if ( bins[ j ].num == 0 )
			bins[ j ].min = bins[ j ].max = data[ i - start ];
		else
		{
			if ( bins[ j ].min > data[ i - start ] )
				bins[ j ].min = data[ i - start ];
			else
				if ( bins[ j ].max < data[ i - start ] )
					bins[ j ].max = data[ i - start ];
		}  
		
		bins[ j ].av += data[ i - start ];
		bins[ j ].num++;   
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
		cmd( "focustop .log .da" ); 
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

	if ( logs )
		delete [ ] data;
	
	delete [ ] str[ 0 ];
	delete [ ] tag[ 0 ];
	delete [ ] str;
	delete [ ] tag;
}


/***************************************************
HISTOGRAMS CS
****************************************************/
void histograms_cs( int *choice )
{
	bool norm;
	char *app, **str, **tag;
	double mx = 0, mn = 0, step, a, lminy, lmaxy, **data;
	int i, j, stat, active_v, *start, *end, *id;

	if ( nv < 2 )
	{
		if ( nv == 0 )			// no variables selected
			cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place two or more series in the Series Selected listbox.\"" );
		else
			cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid number of series\" -detail \"For cross-section histograms select at least two series.\"" );
		
		*choice = 2;
		return;
	}

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	// prepare data from selected series
	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		
		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		data[ i ] = vs[ id[ i ] ].data;
		if ( data[ i ] == NULL )
		{
			error_hard( "invalid series data", 
						"internal problem in LSD", 
						"if error persists, please contact developers",
						true );
			myexit( 18 );
		}
	  
		if ( logs )			// apply log to the values to show "log scale" in the y-axis
			data[ i ] = log_data( data[ i ], start[ i ], end[ i ], i, "histogram" );
	}

	cmd( "set time %d", end[ 0 ] );
	cmd( "set bidi %d", nv < 25 ? nv : 25 );
	cmd( "set norm 0" );
	cmd( "set stat 0" );
	
	cmd( "newtop .da.s \"Histogram Options\" { set choice 2 } .da" );

	cmd( "ttk::frame .da.s.t" );
	cmd( "ttk::label .da.s.t.l -text \"Cross-section case\"" );
	cmd( "ttk::entry .da.s.t.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 && $n <= $numc } { set time %%P; return 1 } { %%W delete 0 end; %%W insert 0 $time; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( ".da.s.t.e insert 0 $time" ); 
	cmd( "pack .da.s.t.l .da.s.t.e -side left -padx 2" );

	cmd( "ttk::frame .da.s.i" );
	cmd( "ttk::label .da.s.i.l -text \"Classes/bins\"" );
	cmd( "ttk::spinbox .da.s.i.e -width 5 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= %d } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invalidcommand { bell } -justify center", nv, nv );
	cmd( ".da.s.i.e insert 0 $bidi" ); 
	cmd( "pack .da.s.i.l .da.s.i.e -side left -padx 2" );

	cmd( "ttk::frame .da.s.o" );
	cmd( "ttk::checkbutton .da.s.o.norm -text \"Fit a Normal\" -variable norm" );
	cmd( "ttk::checkbutton .da.s.o.st -text \"Show statistics\" -variable stat" );
	cmd( "pack .da.s.o.norm .da.s.o.st -anchor w" );
	
	cmd( "pack .da.s.t .da.s.i .da.s.o -pady 10" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#histogram } { set choice 2 }" );

	cmd( "bind .da.s.t.e <Return> {focus .da.s.i.e; .da.s.i.e selection range 0 end}" );
	cmd( "bind .da.s.i.e <KeyPress-Return> {set choice 1}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.t.e; .da.s.t.e selection range 0 end" );
	cmd( "mousewarpto .da.s.b.ok" );

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
		if ( start[ i ] <= time_cs && end[ i ] >= time_cs && is_finite( data[ i ][ time_cs - start[ i ] ] ) )		// ignore NaNs
		{
			if ( active_v == 0 )
				mx = mn = data[ i ][ time_cs - start[ i ] ];
			else
			{
				if ( data[ i ][ time_cs - start[ i ] ] > mx )
					mx = data[ i ][ time_cs - start[ i ] ];
				else
					if ( data[ i ][ time_cs - start[ i ] ] < mn )
						mn = data[ i ][ time_cs - start[ i ] ];
			}  
			
			mean += data[ i ][ time_cs - start[ i ] ];
			var += data[ i ][ time_cs - start[ i ] ] * data[ i ][ time_cs - start[ i ] ];
			
			++cases;
			++active_v;
		}

	if ( cases == 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Invalid data\" -detail \"The selected series have no valid data in the chosen cases (time steps).\"" );
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
		if ( start[ i ] > time_cs || end[ i ] < time_cs || ! is_finite( data[ i ][ time_cs - start[ i ] ] ) )
			continue;

		a = floor( num_bins * ( data[ i ][ time_cs - start[ i ] ] - mn ) / ( mx - mn ) );
			
		j = ( int ) a;
		if ( j == num_bins )
			--j;
		
		if ( bins[ j ].num == 0 )
			bins[ j ].min=bins[ j ].max = data[ i ][ time_cs - start[ i ] ];
		else
		{
			if ( bins[ j ].min > data[ i ][ time_cs - start[ i ] ] )
				bins[ j ].min = data[ i ][ time_cs - start[ i ] ];
			else
				if ( bins[ j ].max < data[ i ][ time_cs - start[ i ] ] )
					bins[ j ].max = data[ i ][ time_cs - start[ i ] ];
		}
		
		bins[ j ].num++;   
		bins[ j ].av += data[ i ][ time_cs - start[ i ] ];
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
		cmd( "focustop .log .da" ); 
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
			delete [ ] data[ i ];
	}
	
	delete [ ] str;
	delete [ ] tag; 
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

// define MC series parent names for AoR
const char *mc_par[ ] = { "(MC)", "meanMC", "maxMC", "minMC", "varMC", "sumMC", "meanMC", "countMC", "sdMC", "prodMC", "invMC", "ci+MC", "ci-MC", "maxMC", "(MC)", "meanMC", "meanMC" };
		
bool create_series( int *choice, bool mc, vector < string > var_names )
{
	bool first, done = true;
	char *lapp, **str, **tag;
	double nmax = 0, nmin = 0, nmean, nvar, nn, sum, prod, thflt, z_crit, **data;
	int i, j, k, flt, cs_long, type_series, new_series, sel_series, confi, *start, *end, *id;
	store *app;

	if ( ! mc )
	{
		if ( nv == 0 )
		{
			cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
			return false;
		}

		if ( logs )
			cmd( "ttk::messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

		cmd( "set flt 0" );
		cmd( "set thflt 0" );
		cmd( "set bido 1" );
		cmd( "set bidi 1" );
		cmd( "set ftag 1" );
		cmd( "set confi 95" );

		cmd( "newtop .da.s \"New Series Options\" { set choice 2 } .da" );

		cmd( "ttk::frame .da.s.o" );
		cmd( "ttk::label .da.s.o.l -text \"Aggregation mode\"" );

		cmd( "ttk::frame .da.s.o.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .da.s.o.r.m -text \"Calculate over series (same # of cases)\" -variable bido -value 1" );
		cmd( "ttk::radiobutton .da.s.o.r.f -text \"Calculate over cases (# cases = # of series)\" -variable bido -value 2" );
		cmd( "pack .da.s.o.r.m .da.s.o.r.f -anchor w" );

		cmd( "pack .da.s.o.l .da.s.o.r" );

		cmd( "ttk::frame .da.s.f" );
		cmd( "ttk::label .da.s.f.l -text \"Filtering\"" );

		cmd( "ttk::frame .da.s.f.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .da.s.f.r.n -text \"Use all values\" -variable flt -value 0 -command { .da.s.f.t.th configure -state disabled }" );
		cmd( "ttk::radiobutton .da.s.f.r.s -text \"Ignore small values\" -variable flt -value 1 -command { .da.s.f.t.th configure -state normal }" );
		cmd( "ttk::radiobutton .da.s.f.r.b -text \"Ignore large values\" -variable flt -value 2 -command { .da.s.f.t.th configure -state normal }" );
		cmd( "pack .da.s.f.r.n .da.s.f.r.s .da.s.f.r.b -anchor w" );

		cmd( "ttk::frame .da.s.f.t" );
		cmd( "ttk::label .da.s.f.t.l -text \"Threshold\"" );
		cmd( "ttk::entry .da.s.f.t.th -width 10 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] } { set thflt %%P; return 1 } { %%W delete 0 end; %%W insert 0 $thflt; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
		cmd( "write_disabled .da.s.f.t.th $thflt" ); 
		cmd( "pack .da.s.f.t.l .da.s.f.t.th -side left -padx 2" );

		cmd( "pack .da.s.f.l .da.s.f.r .da.s.f.t" );

		cmd( "ttk::frame .da.s.i" );
		cmd( "ttk::label .da.s.i.l -text \"Operation\"" );

		cmd( "ttk::frame .da.s.i.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .da.s.i.r.m -text \"Average\" -variable bidi -value 1 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_avg\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end }" );
		cmd( "ttk::radiobutton .da.s.i.r.z -text \"Sum\" -variable bidi -value 5 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_sum\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end  }" );
		cmd( "ttk::radiobutton .da.s.i.r.x -text \"Product\" -variable bidi -value 9 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_prd\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end  }" );
		cmd( "ttk::radiobutton .da.s.i.r.i -text \"Invert\" -variable bidi -value 10 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_inv\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end  }" );
		cmd( "ttk::radiobutton .da.s.i.r.n -text \"Count\" -variable bidi -value 7 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_num\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end }" );
		cmd( "ttk::radiobutton .da.s.i.r.f -text \"Maximum\" -variable bidi -value 2 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_max\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end }" );
		cmd( "ttk::radiobutton .da.s.i.r.t -text \"Minimum\" -variable bidi -value 3 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_min\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end }" );
		cmd( "ttk::radiobutton .da.s.i.r.c -text \"Variance\" -variable bidi -value 4 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_var\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end }" );
		cmd( "ttk::radiobutton .da.s.i.r.s -text \"Standard deviation\" -variable bidi -value 8 -command { .da.s.i.r.ci.p configure -state disabled; set tailname \"_sd\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end }" );

		cmd( "ttk::frame .da.s.i.r.ci" );
		cmd( "ttk::radiobutton .da.s.i.r.ci.c -text \"Confidence interval (3 series)\" -variable bidi -value 6 -command { .da.s.i.r.ci.p configure -state normal; set tailname \"\"; set vname $basename$tailname; .da.s.n.nv selection range 0 end }" );
		cmd( "ttk::label .da.s.i.r.ci.x -text @" );
		cmd( "ttk::label .da.s.i.r.ci.perc -text %%" );
		cmd( "ttk::spinbox .da.s.i.r.ci.p -width 5 -from 80 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 80 && $n <= 99 } { set confi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $confi; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
		cmd( "write_disabled .da.s.i.r.ci.p $confi" ); 
		cmd( "pack .da.s.i.r.ci.c .da.s.i.r.ci.x .da.s.i.r.ci.p .da.s.i.r.ci.perc -side left" );

		cmd( "pack .da.s.i.r.m .da.s.i.r.z .da.s.i.r.x .da.s.i.r.i .da.s.i.r.n .da.s.i.r.f .da.s.i.r.t .da.s.i.r.c .da.s.i.r.s .da.s.i.r.ci -anchor w" );
		cmd( "pack .da.s.i.l .da.s.i.r" );

		cmd( "set a [ .da.vars.ch.f.v get 0 ]" );
		cmd( "set basename [ lindex [ split $a ] 0 ]" );
		cmd( "set tailname \"_avg\"" );
		cmd( "set vname $basename$tailname" );

		cmd( "ttk::frame .da.s.n" );
		cmd( "ttk::label .da.s.n.lnv -text \"New series name\"" );
		cmd( "ttk::entry .da.s.n.nv -width 30 -textvariable vname -justify center" );
		cmd( "pack .da.s.n.lnv .da.s.n.nv" );

		cmd( "ttk::frame .da.s.t" );
		cmd( "ttk::label .da.s.t.tnv -text \"New series tag\"" );
		cmd( "ttk::entry .da.s.t.tv -width 20 -textvariable ftag -justify center" );
		cmd( "pack .da.s.t.tnv .da.s.t.tv" );

		cmd( "pack .da.s.o .da.s.f .da.s.i .da.s.n .da.s.t -padx 5 -pady 5" );

		cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#createselection } { set choice 2 }" );

		cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
		cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

		cmd( "showtop .da.s" );
		cmd( "focus .da.s.n.nv" );
		cmd( ".da.s.n.nv selection range 0 end" );
		cmd( "mousewarpto .da.s.b.ok" );
		 
		*choice = 0;
		while ( *choice == 0 )
			Tcl_DoOneEvent( 0 );

		cmd( "if [ string is double [ .da.s.f.t.th get ] ] { set thflt [ .da.s.f.t.th get ] }" ); 
		cmd( "if [ string is integer [ .da.s.i.r.ci.p get ] ] { set confi [ .da.s.i.r.ci.p get ] }" ); 
		cmd( "destroytop .da.s" );

		if ( *choice == 2 )
		{
			*choice = 0;
			return false;
		}
		
		flt = get_int( "flt" );
		thflt = get_double( "thflt" );
		cs_long = get_int( "bido" );
		sel_series = nv;
		var_num = num_var;
	}
	else
	{
		flt = 0;
		thflt = 0;
		cs_long = 1;
		sel_series = var_names.size( );
	}
	
	confi = get_int( "confi" );
	type_series = get_int( "bidi" );
	new_series = 1;
	z_crit = 0;
	
	// set option specific parameters
	switch ( type_series )
	{
		case 6:							// avg (6), ci+ (11), ci- (12)
			new_series = 3;
			
			// first series to produce
			cmd( "set basename $vname; set tailname \"_avg\"; set vname $basename$tailname" );
			
			// get the critical value to the chosen confidence level
			z_crit = z_star[ ( int ) max( min( confi, 99 ), 80 ) - 80 ];
			
			break;
			
		case 11:						// ci+ (11), ci- (12)
			new_series = 2;
			cmd( "set basename $vname; set tailname \"_ci+\"; set vname $basename$tailname" );
			z_crit = z_star[ ( int ) max( min( confi, 99 ), 80 ) - 80 ];
			break;
			
		case 13:						// max (13), min (3)
			new_series = 2;
			cmd( "set basename $vname; set tailname \"_max\"; set vname $basename$tailname" );
			break;
		
		case 15:						// avg (15), max (2), min (3)
			new_series = 3;
			cmd( "set basename $vname; set tailname \"_avg\"; set vname $basename$tailname" );
			break;
		
		case 16:						// avg (16), ci+ (11), ci- (12), max (2), min (3)
			new_series = 5;
			cmd( "set basename $vname; set tailname \"_avg\"; set vname $basename$tailname" );
			z_crit = z_star[ ( int ) max( min( confi, 99 ), 80 ) - 80 ];
			break;
	}
	
	data = new double *[ sel_series ];
	start = new int [ sel_series ];
	end = new int [ sel_series ];
	id = new int [ sel_series ];
	str = new char *[ sel_series ];
	tag = new char *[ sel_series ];

	// allocate space for the new series
	app = new store[ num_var + new_series ];
	for ( i = 0; i < num_var; ++i )
	{
		app[ i ] = vs[ i ];
		strcpy( app[ i ].label, vs[ i ].label );
		strcpy( app[ i ].tag, vs[ i ].tag );
	} 

	delete [ ] vs;
	vs = app;

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	for ( i = 0; i < sel_series; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;

		if ( mc )
			strcpy( msg, var_names[ i ].c_str( ) );
		else
		{
			cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
			lapp = ( char * ) Tcl_GetVar( inter, "res", 0 );
			strcpy( msg, lapp );
		}
		
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
			{
				cmd( "destroytop .da.pas" );
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}
		}
	}

	if ( autom_x || min_c >= max_c )
	{
		// differently from normal, pick just cases covering all series
		min_c = max( start[ 0 ], showInit ? 0 : 1 );
		max_c = end[ 0 ];
		for ( i = 1; i < sel_series; ++i )
		{
			if ( start[ i ] > min_c )
				min_c = start[ i ];
			if ( end[ i ] < max_c )
				max_c = end[ i ];
		}
		
		if ( min_c >= max_c )
		{
			cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Series cases do not overlap\" -detail \"Two or more series in the Series Selected listbox have no common cases (time steps). Please use manual case selection if this is the desired behavior.\"" );
			
			done = false;
			goto end_new_series;
		}
	}

	// handle creation of multiple series
	for ( k = 0; k < new_series; ++k, ++num_var, ++var_num )
	{
		lapp = ( char * ) Tcl_GetVar( inter, "vname", 0 );
		strcpy( vs[ num_var ].label, lapp );
		lapp = ( char * ) Tcl_GetVar( inter, "ftag", 0 );
		sprintf( vs[ num_var ].tag, "%s_%s", mc ? "MC" : "C", lapp );
		vs[ num_var ].rank = var_num;
			
		if ( cs_long == 1 )									// compute over series?
		{
			vs[ num_var ].data = new double[ max_c - min_c + 1 ];
			vs[ num_var ].end = max_c;
			vs[ num_var ].start = min_c;

			for ( i = min_c; i <= max_c; ++i )
			{
				nn = nvar = sum = prod = 0;
				first = true;
				for ( j = 0; j < sel_series; ++j )
				{
					if ( i >= start[ j ] && i <= end[ j ] && is_finite( data[ j ][ i - start[ j ] ] ) && ( flt == 0 || ( flt == 1 && data[ j ][ i - start[ j ] ] > thflt ) || ( flt == 2 && data[ j ][ i - start[ j ] ] < thflt ) ) )		// ignore NaNs
					{
						sum += data[ j ][ i - start[ j ] ];
						nvar += data[ j ][ i - start[ j ] ] * data[ j ][ i - start[ j ] ];
						
						++nn;
						
						if ( first )
						{
							nmin = nmax = prod = data[ j ][ i - start[ j ] ];
							first = false;
						}
						else
						{
							if ( nmin > data[ j ][ i - start[ j ] ] )
								nmin = data[ j ][ i - start[ j ] ];
							if ( nmax < data[ j ][ i - start[ j ] ] )
								nmax = data[ j ][ i - start[ j ] ];
							prod *= data[ j ][ i - start[ j ] ];
						} 
					}
				}

				if ( nn == 0 )	// not a single valid value?
					nn = nmean = nvar = nmin = nmax = sum = prod = NAN;
				else
				{
					nmean = sum / nn;
					nvar /= nn;
					nvar -= nmean * nmean;
				}
			   
				if ( type_series == 1 || type_series == 6 || type_series == 15 || type_series == 16 )
					vs[ num_var ].data[ i - min_c ] = nmean;
				if ( type_series == 2 || type_series == 13 )
					vs[ num_var ].data[ i - min_c ] = nmax;
				if ( type_series == 3 )
					vs[ num_var ].data[ i - min_c ] = nmin;
				if ( type_series == 4 )
					vs[ num_var ].data[ i - min_c ] = nvar;
				if ( type_series == 5 )
					vs[ num_var ].data[ i - min_c ] = sum;
				if ( type_series == 7 )
					vs[ num_var ].data[ i - min_c ] = nn;
				if ( type_series == 8 )
					vs[ num_var ].data[ i - min_c ] = sqrt( nvar );
				if ( type_series == 9 )
					vs[ num_var ].data[ i - min_c ] = prod;
				if ( type_series == 10 )
				{
					if ( nmean != 0 )
						vs[ num_var ].data[ i - min_c ] = 1 / nmean;
					else
						vs[ num_var ].data[ i - min_c ] = NAN;
				}
				if ( type_series == 11 )
					vs[ num_var ].data[ i - min_c ] = nmean + z_crit * sqrt( nvar ) / sqrt( nn );
				if ( type_series == 12 )
					vs[ num_var ].data[ i - min_c ] = nmean - z_crit * sqrt( nvar ) / sqrt( nn );
			}
		} 
		else												// compute over cases
		{
			vs[ num_var ].data = new double[ sel_series ];
			vs[ num_var ].end = sel_series - 1;
			vs[ num_var ].start = 0;

			for ( j = 0; j < sel_series; ++j )
			{
				nn = nvar = sum = prod = 0;
				first = true;
				for ( i = min_c; i <= max_c; ++i )
				{
					if ( i >= start[ j ] && i <= end[ j ] && is_finite( data[ j ][ i - start[ j ] ] ) && ( flt == 0 || ( flt == 1 && data[ j ][ i - start[ j ] ] > thflt ) || ( flt == 2 && data[ j ][ i - start[ j ] ] < thflt ) ) )
					{
						sum += data[ j ][ i - start[ j ] ];
						nvar += data[ j ][ i - start[ j ] ] * data[ j ][ i - start[ j ] ];
						
						++nn;
						
						if ( first )
						{
							nmin = nmax = prod = data[ j ][ i - start[ j ] ];
							first = false;
						}
						else
						{
							if ( nmin > data[ j ][ i - start[ j ] ] )
								nmin = data[ j ][ i - start[ j ] ];
							if ( nmax < data[ j ][ i - start[ j ] ] )
								nmax = data[ j ][ i - start[ j ] ];
							prod *= data[ j ][ i - start[ j ] ];
						} 
					}
				}

				if ( nn == 0 )	// not a single valid value?
					nn = nmean = nvar = nmin = nmax = sum = prod = NAN;
				else
				{
					nmean = sum / nn;
					nvar /= nn;
					nvar -= nmean * nmean;
				}
				  
				if ( type_series == 1 || type_series == 6 || type_series == 15 || type_series == 16 )
					vs[ num_var ].data[ j ] = nmean;
				if ( type_series == 2 || type_series == 13 )
					vs[ num_var ].data[ j ] = nmax;
				if ( type_series == 3 )
					vs[ num_var ].data[ j ] = nmin;
				if ( type_series == 4 )
					vs[ num_var ].data[ j ] = nvar;
				if ( type_series == 5 )
					vs[ num_var ].data[ j ] = sum;
				if ( type_series == 7 )
					vs[ num_var ].data[ j ] = nn;
				if ( type_series == 8 )
					vs[ num_var ].data[ j ] = sqrt( nvar );
				if ( type_series == 9 )
					vs[ num_var ].data[ j ] = prod;
				if ( type_series == 10 )
				{
					if ( nmean != 0 )
						vs[ num_var ].data[ j ] = 1 / nmean;
					else
						vs[ num_var ].data[ j ] = NAN;
				}
				if ( type_series == 11 )
					vs[ num_var ].data[ j ] = nmean + z_crit * sqrt( nvar ) / sqrt( nn );
				if ( type_series == 12 )
					vs[ num_var ].data[ j ] = nmean - z_crit * sqrt( nvar ) / sqrt( nn );
					
			}
		}
		
		if ( mc && new_series == 1 && par_map.find( vs[ num_var ].label ) != par_map.end( ) )
			cmd( "add_series \"%s %s (%d-%d) #%d\" %s", vs[ num_var ].label, vs[ num_var ].tag, vs[ num_var ].start, vs[ num_var ].end, vs[ num_var ].rank, par_map[ vs[ num_var ].label ].c_str( ) ); 
		else
			cmd( "add_series \"%s %s (%d-%d) #%d\" %s", vs[ num_var ].label, vs[ num_var ].tag, vs[ num_var ].start, vs[ num_var ].end, vs[ num_var ].rank, mc ? mc_par[ type_series ] : "(added)" ); 

		// define next series options for multiple series
		switch ( type_series )
		{
			case 6:					// avg (6), ci+ (11), ci- (12)
			case 16:				// avg (16), ci+ (11), ci- (12), max (2), min (3)
				type_series = 11;
				cmd( "set tailname \"_ci+\"; set vname $basename$tailname" ); 
				break;
			
			case 11:				// ci+ (11), ci- (12)
				type_series = 12;
				cmd( "set tailname \"_ci-\"; set vname $basename$tailname" ); 
				break;
			
			case 12:				// ci- (12), max (2), min (3)
			case 15:				// avg (15), max (2), min (3)
				type_series = 2;
				cmd( "set tailname \"_max\"; set vname $basename$tailname" ); 
				break;
			
			case 2:					// max (2), min (3)
			case 13:				// max (13), min (3)
				type_series = 3;
				cmd( "set tailname \"_min\"; set vname $basename$tailname" ); 
		}
	}
	
	cmd( "update_parent" );

	end_new_series:
	
	for ( i = 0; i < sel_series; ++i )
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
	
	return done;
}


/***************************************************
CREATE_MAVERAG
****************************************************/
bool create_maverag( int *choice )
{
	char *lapp, **str, **tag;
	double xapp, **data;
	int h, i, j, k, flt, ma_type, *start, *end, *id;
	store *app;

	if ( nv == 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return false;
	}

	if ( logs )
		cmd( "ttk::messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

	cmd( "set bido 4" );
	cmd( "set ma_type 0" );
	
	cmd( "newtop .da.s \"Moving Average Period\" { set choice 2 } .da" );

	cmd( "ttk::frame .da.s.o" );
	cmd( "ttk::label .da.s.o.l -text \"Period (cases)\"" );
	cmd( "ttk::spinbox .da.s.o.th -width 5 -from 1 -to $numc -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= $numc } { set bido %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bido; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( ".da.s.o.th insert 0 $bido" ); 
	cmd( "pack .da.s.o.l .da.s.o.th" );
	
	cmd( "ttk::frame .da.s.t -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton .da.s.t.s -variable ma_type -value 0 -text \"Simple moving average\"" );
	cmd( "ttk::radiobutton .da.s.t.c -variable ma_type -value 1 -text \"Central moving average\"" );
	cmd( "pack .da.s.t.s .da.s.t.c -anchor w" );

	cmd( "pack .da.s.o .da.s.t -padx 5 -pady 5" );

	cmd( "okhelpcancel .da.s b { set choice 1 } { LsdHelp menudata_res.html#createmavg } { set choice 2 }" );

	cmd( "bind .da.s <KeyPress-Return> {set choice 1}" );
	cmd( "bind .da.s <KeyPress-Escape> {set choice 2}" );

	cmd( "showtop .da.s" );
	cmd( "focus .da.s.o.th" );
	cmd( ".da.s.o.th selection range 0 end" );
	cmd( "mousewarpto .da.s.b.ok" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set bido [ .da.s.o.th get ]" ); 
	cmd( "destroytop .da.s" );

	if ( *choice == 2 )
	{
		*choice = 0;
		return false;
	}

	flt = get_int( "bido" );
	ma_type = get_int( "ma_type" );
	
	// adjust to odd number, if required
	if ( flt < 2 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -icon error -title Error -message \"Invalid moving average period\" -detail \"Please choose a period larger than one case (time step).\"" );
		*choice = 0;
		return false;
	}

	if ( ma_type == 1 && flt % 2 == 0 )
	{
		++flt;
		plog( "\nRounding up the period to %d for an odd number of cases (time steps)\n", "", flt );
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
		strcpy( app[ i ].tag, vs[ i ].tag );
	} 
	
	delete [ ] vs;
	vs = app;

	if ( autom_x )
	{
		min_c = max( first_c, showInit ? 0 : 1 );
		max_c = num_c;
	}

	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;

		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		lapp = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, lapp );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );

		sprintf( vs[ num_var + i ].label, "%s_%cma%d", str[ i ], ma_type == 0 ? 's' : 'c', flt );
		sprintf( vs[ num_var + i ].tag, "C_%s", tag[ i ] );
		vs[ num_var + i ].start = ( ma_type == 0 ) ? start[ i ] + flt - 1 : start[ i ];
		vs[ num_var + i ].end = end[ i ];
		vs[ num_var + i ].rank = num_var + i;
		vs[ num_var + i ].data = new double[ vs[ num_var + i ].end - vs[ num_var + i ].start + 1 ];
		
		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
			{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}

			if ( ma_type == 0 )		// simple moving average
			{
				for ( k = start[ i ] + flt - 1; k <= end[ i ]; ++k )
				{
					for ( xapp = 0, h = 0, j = k - flt + 1; j <= k; ++j )
						if ( is_finite( data[ i ][ j - start[ i ] ] ) )		// not a NaN?
						{
							xapp += data[ i ][ j - start[ i ] ];
							++h;
						}
						
					if ( h == 0 )
						xapp = NAN;
					else
						xapp /= h;
					
					vs[ num_var + i ].data[ k - vs[ num_var + i ].start ] = xapp;
				}
			}
			else					// central moving average
			{
				// average of first period in data
				for ( xapp = 0, h = 0, j = start[ i ]; j < start[ i ] + flt; ++j )
					if ( is_finite( data[ i ][ j - start[ i ] ] ) )		// not a NaN?
					{
						xapp += data[ i ][ j - start[ i ] ];
						h++;
					}
					
				if ( h == 0 )		// no observation before first?
					xapp = NAN;
				else
					xapp /= h;

				for ( j = start[ i ]; j < start[ i ] + ( flt - 1 ) / 2; ++j )
					vs[ num_var + i ].data[ j - vs[ num_var + i ].start ] = xapp;

				for ( ; j < end[ i ] - ( flt - 1 ) / 2; ++j )
				{
					if ( is_finite( data[ i ][ j - ( flt - 1 ) / 2 - start[ i ] ] ) && is_finite( data[ i ][ j + ( flt - 1 ) / 2 - start[ i ] ] ) )
						xapp = xapp - data[ i ][ j - ( flt - 1 ) / 2 - start[ i ] ] / flt + data[ i ][ j + ( flt - 1 ) / 2 - start[ i ] ] / flt;
					else
						xapp = NAN;
					
					vs[ num_var + i ].data[ j - vs[ num_var + i ].start ] = xapp;
				}
				
				for ( ; j <= end[ i ]; ++j )
					vs[ num_var + i ].data[ j - vs[ num_var + i ].start ] = xapp;     
			}
		}
		
		cmd( "add_series \"%s %s (%d-%d) #%d\" \"(added)\"", vs[ num_var + i ].label, vs[ num_var + i ].tag, vs[ num_var + i ].start, vs[ num_var + i ].end, vs[ num_var + i ].rank ); 
	}

	cmd( "update_parent" );
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
	
	return true;
}


/***************************************************
ADD_UNSAVED
****************************************************/
bool add_unsaved( int *choice )
{
	char *lab;
	
	if ( actual_steps == 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"Simulation not run\" -detail \"Select menu option Run>Run before using this option.\"" );
		return false;
	}

	cmd( "set bidi \"\"" );
	
	cmd( "set unSavElem { }" );
	cmd( "foreach b $modElem { \
			if { [ lsearch $DaModElem $b ] < 0 } { \
				lappend unSavElem $b \
			} \
		}" );
	
	cmd( "newtop .da.s \"Add Unsaved Element\" { set choice 2 } .da" );
	
	cmd( "ttk::frame .da.s.i" );
	cmd( "ttk::label .da.s.i.l -text \"Element name (or part)\"" );
	cmd( "ttk::combobox .da.s.i.e -width 20 -justify center -values $unSavElem -validate focusout -validatecommand { set n %%P; if { $n in $unSavElem } { set bidi %%P; return 1 } { %%W delete 0 end; %%W insert 0 $bidi; return 0 } } -invalidcommand { bell }" );
	cmd( "write_any .da.s.i.e $bidi" );
	cmd( "pack .da.s.i.l .da.s.i.e" );
	cmd( "pack .da.s.i -pady 5 -padx 5" );
	
	cmd( "bind .da.s.i.e <Return> { set choice 1 }" );
	cmd( "bind .da.s.i.e <Escape> { set choice 2 }" );
	cmd( "bind .da.s.i.e <KeyRelease> { \
			if { %%N < 256 && [ info exists unSavElem ] } { \
				set b [ .da.s.i.e index insert ]; \
				set s [ .da.s.i.e get ]; \
				set f [ lsearch -glob $unSavElem $s* ]; \
				if { $f !=-1 } { \
					set d [ lindex $unSavElem $f ]; \
					.da.s.i.e delete 0 end; \
					.da.s.i.e insert 0 $d; \
					.da.s.i.e index $b; \
					.da.s.i.e selection range $b end \
				} \
			} \
		}" );

	cmd( "okhelpcancel .da.s b  { set choice 1 } { LsdHelp menudata_res.html#add_series } { set choice 2 }" );
	cmd( "showtop .da.s" );
	cmd( "focus .da.s.i.e" );
	
	*choice = 0;
	while( *choice == 0 )
		Tcl_DoOneEvent( 0 );
	
	cmd( "if { [ .da.s.i.e get ] in $unSavElem } { set bidi [ .da.s.i.e get ] } { bell }" );
	cmd( "destroytop .da.s" );
	
	if( *choice == 2 )
		return false;
	
	cmd( "set choice [ lsearch $modElem $bidi ]" );
	if( *choice < 0 )
	{
		cmd( "ttk::messageBox -parent .da -type ok -icon error -title Error -message \"Invalid element name\" -detail \"There is no element in the model structure with the given name.\"" );
		return false;
	}
	
	lab = ( char * ) Tcl_GetVar( inter, "bidi", 0 );
	insert_data_mem( root, &num_var, lab );
	
	return true;
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

	gzFile fsavez = NULL;

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
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return;
	}

	if ( logs )
	  cmd( "ttk::messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

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
		
		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );
		
		data[ i ] = vs[ id[ i ] ].data;
		if ( data[ i ] == NULL )
		{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
		}
		
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

	cmd( "ttk::frame .da.lab.f" );
	cmd( "ttk::label .da.lab.f.l -text \"File format\"" );

	cmd( "ttk::frame .da.lab.f.t -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton .da.lab.f.t.lsd -text \"LSD results file\" -variable typelab -value 3" );
	cmd( "ttk::radiobutton .da.lab.f.t.nolsd -text \"Text file\" -variable typelab -value 4" );
	cmd( "pack .da.lab.f.t.lsd .da.lab.f.t.nolsd -anchor w" );

	cmd( "pack .da.lab.f.l .da.lab.f.t" );

	cmd( "ttk::checkbutton .da.lab.dozip -text \"Generate zipped file\" -variable dozip" );

	cmd( "pack .da.lab.f .da.lab.dozip -padx 5 -pady 5" );

	cmd( "okhelpcancel .da.lab b { set choice 1 } { LsdHelp menudata_res.html#save } { set choice 2 }" );

	cmd( "bind .da.lab <Return> {.da.lab.ok invoke}" );
	cmd( "bind .da.lab <Escape> {.da.lab.esc invoke}" );

	cmd( "showtop .da.lab" );
	cmd( "mousewarpto .da.lab.b.ok" );

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

		cmd( "ttk::checkbutton .da.lab.fr -text \"Header in first row\" -variable fr -command { if { $fr == 0 } { .da.lab.gp configure -state disabled; .da.lab.f.r.orig configure -state disabled; .da.lab.f.r.new configure -state disabled; .da.lab.d.r.tab configure -state disabled; .da.lab.d.r.oth configure -state disabled; .da.lab.d.r.col configure -state disabled; .da.lab.gen.mis_val configure -state disabled; if { $typelab == 2 } { .da.lab.n.en configure -state disabled }; if { $deli == 2 } { .da.lab.c.del configure -state disabled }; if { $deli == 3 } { .da.lab.e.ecol configure -state disabled } } else { .da.lab.gp configure -state normal; .da.lab.f.r.orig configure -state normal; .da.lab.f.r.new configure -state normal; .da.lab.d.r.tab configure -state normal; .da.lab.d.r.oth configure -state normal; .da.lab.d.r.col configure -state normal; .da.lab.gen.mis_val configure -state normal; if { $typelab == 2 } { .da.lab.n.en configure -state normal }; if { $deli == 2 } { .da.lab.c.del configure -state normal }; if { $deli == 3 } { .da.lab.e.ecol configure -state normal } } }" );
		cmd( "ttk::checkbutton .da.lab.gp -text \"Prefix header with #\" -variable headprefix" );

		cmd( "ttk::frame .da.lab.f" );
		cmd( "ttk::label .da.lab.f.tit -text \"First row labels\"" );

		cmd( "ttk::frame .da.lab.f.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .da.lab.f.r.orig -text Original -variable typelab -value 1 -command { .da.lab.n.en configure -state disabled }" );
		cmd( "ttk::radiobutton .da.lab.f.r.new -text Numeric -variable typelab -value 2 -command { .da.lab.n.en configure -state normal }" );
		cmd( "pack .da.lab.f.r.orig .da.lab.f.r.new -anchor w" );

		cmd( "pack .da.lab.f.tit .da.lab.f.r" );

		cmd( "ttk::frame .da.lab.n" );
		cmd( "ttk::label .da.lab.n.l -text \"Numeric labels prefix\"" );
		cmd( "ttk::entry .da.lab.n.en -width 10 -justify center -textvariable labprefix -state disabled" );
		cmd( "pack .da.lab.n.l .da.lab.n.en" );

		cmd( "ttk::frame .da.lab.d" );
		cmd( "ttk::label .da.lab.d.tit -text \"Columns delimiter\"" );

		cmd( "ttk::frame .da.lab.d.r -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
		cmd( "ttk::radiobutton .da.lab.d.r.tab -text Tabs -variable deli -value 1 -command { .da.lab.c.del configure -state disabled; .da.lab.e.ecol configure -state disabled }" );
		cmd( "ttk::radiobutton .da.lab.d.r.oth -text Custom -variable deli -value 2 -command { .da.lab.c.del configure -state normal; .da.lab.e.ecol configure -state disabled }" );
		cmd( "ttk::radiobutton .da.lab.d.r.col -text \"Fixed width\" -variable deli -value 3 -command { .da.lab.c.del configure -state disabled; .da.lab.e.ecol configure -state normal }" );
		cmd( "pack .da.lab.d.r.tab .da.lab.d.r.oth .da.lab.d.r.col -anchor w" );

		cmd( "pack .da.lab.d.tit .da.lab.d.r" );

		cmd( "ttk::frame .da.lab.c" );
		cmd( "ttk::label .da.lab.c.l -text \"Custom delimiter\"" );
		cmd( "ttk::entry .da.lab.c.del -width 3 -textvariable delimiter -justify center -state disabled" );
		cmd( "pack .da.lab.c.l .da.lab.c.del" );

		cmd( "ttk::frame .da.lab.e" );
		cmd( "ttk::label .da.lab.e.l -text \"Column width (chars)\"" );
		cmd( "ttk::spinbox .da.lab.e.ecol -width 5 -from 10 -to 80 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 10 && $n <= 80 } { set numcol %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numcol; return 0 } } -invalidcommand { bell } -justify center -state disabled" );
		cmd( "write_disabled .da.lab.e.ecol $numcol" );
		cmd( "pack .da.lab.e.l .da.lab.e.ecol" );

		cmd( "ttk::frame .da.lab.gen" );
		cmd( "ttk::label .da.lab.gen.miss -text \"Missing values\"" );
		cmd( "ttk::entry .da.lab.gen.mis_val -width 5 -textvariable misval -justify center" );
		cmd( "pack .da.lab.gen.miss .da.lab.gen.mis_val" );

		cmd( "pack .da.lab.fr .da.lab.gp .da.lab.f .da.lab.n .da.lab.d .da.lab.c .da.lab.e .da.lab.gen -padx 5 -pady 5" );

		cmd( "okhelpcancel .da.lab b { set choice 1 } { LsdHelp menudata_res.html#save } { set choice 2 }" );

		cmd( "showtop .da.lab" );
		cmd( "mousewarpto .da.lab.b.ok" );

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

	cmd( "set bah [ tk_getSaveFile -parent .da -title \"Save Data File\" -initialdir \"$path\" -defaultextension \"%s\" -filetypes { { {%s} {%s} } { {All files}  {*} }  } ]", ext, descr, ext );
	app = ( char * ) Tcl_GetVar( inter, "bah", 0 );
	strcpy( msg, app );

	if ( strlen( msg ) == 0 )
		goto end;

	if ( dozip == 1 ) 
		fsavez = gzopen( msg, "wt" );
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
							gzprintf( fsavez, "#" );
						else
							fprintf( fsave, "#" );
					}  
					 
					for ( i = 0; i < nv; ++i )
					{
						if ( dozip == 1 ) 
						{
							gzprintf( fsavez, "%s_%s", str[ i ], tag[ i ] );
						   
							if ( i < nv - 1 )  
								gzprintf( fsavez, "%s", delimiter );
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
							gzprintf( fsavez, "#" );
						else
							fprintf( fsave, "#" );
					}  
				 
					for ( i = 0; i < nv; ++i )
					{
						if ( dozip == 1 ) 
						{
							gzprintf( fsavez, "%s%d", labprefix, i );
					   
							if ( i < nv - 1 )  
								gzprintf( fsavez, "%s", delimiter );
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
							gzprintf( fsavez, "%s %s (%d %d)\t", str[ i ], tag[ i ], start[ i ], end[ i ] );
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
					gzprintf( fsavez, "#" );
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
					gzprintf( fsavez, "%s", msg );
				else
					fprintf( fsave, "%s", msg ); 
			}
		}

		if ( dozip == 1 ) 
			gzprintf( fsavez, "\n" );
		else
			fprintf( fsave, "\n" );  
	}

	if ( del != 3 ) 					// data delimited writing
	{
		if ( dozip == 1 )
		{
			for ( j = min_c; j <= max_c; ++j )
			{
				for ( i = 0; i < nv; ++i )
				{
					if ( j >= start[ i ] && j <= end[ i ] && ! is_nan( data[ i ][ j - start[ i ] ] ) )		// write NaN as n/a
						gzprintf( fsavez, "%.*G", SIG_DIG, data[ i ][ j - start[ i ] ] );
					else
						gzprintf( fsavez, "%s", misval );
			  
					if ( typelab == 3 || i < nv - 1 )  
						gzprintf( fsavez, "%s", delimiter );
				}
				
				gzprintf( fsavez, "\n" );
			}
		}
		else
		{
			for ( j = min_c; j <= max_c; ++j )
			{
				for ( i = 0; i < nv; ++i )
				{
					if ( j >= start[ i ] && j <= end[ i ] && ! is_nan( data[ i ][ j - start[ i ] ] ) )		// write NaN as n/a
						fprintf( fsave, "%.*G", SIG_DIG, data[ i ][ j - start[ i ] ] );  
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
				if ( j >= start[ i ] && j <= end[ i ] && ! is_nan( data[ i ][ j - start[ i ] ] ) )		// write NaN as n/a
				{
					sprintf( msg, "%.*G", ( int ) min( numcol - 6, SIG_DIG ), data[ i ][ j - start[ i ] ] );
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
					gzprintf( fsavez, "%s", msg );
				else
					fprintf( fsave, "%s", msg );  
			}
			
			if ( dozip == 1 ) 
				gzprintf( fsavez, "\n" );
			else
				fprintf( fsave, "\n" );  
		}
	}

	if ( dozip == 1 ) 
		gzclose( fsavez);
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

	if ( nv == 0 )			// no variables selected
	{
		cmd( "ttk::messageBox -parent .da -type ok -title Error -icon error -message \"No series selected\" -detail \"Place one or more series in the Series Selected listbox.\"" );
		return;
	}

	if ( logs )
	  cmd( "ttk::messageBox -parent .da -type ok -icon warning -title Warning -message \"Series in logs not allowed\" -detail \"The option 'Series in logs' is checked but it does not affect the data produced by this command.\"" );

	data = new double *[ nv ];
	start = new int [ nv ];
	end = new int [ nv ];
	id = new int [ nv ];
	str = new char *[ nv ];
	tag = new char *[ nv ];

	for ( i = 0; i < nv; ++i )
	{
		str[ i ] = new char[ MAX_ELEM_LENGTH ];
		tag[ i ] = new char[ MAX_ELEM_LENGTH ];
		data[ i ] = NULL;
		
		cmd( "set res [ .da.vars.ch.f.v get %d ]", i );
		lapp = ( char * ) Tcl_GetVar( inter, "res", 0 );
		sscanf( lapp, "%s %s (%d-%d) #%d", str[ i ], tag[ i ], &start[ i ], &end[ i ], &id[ i ] );

		if ( autom_x || ( start[ i ] <= max_c && end[ i ] >= min_c ) )
		{
			data[ i ] = vs[ id[ i ] ].data;
			if ( data[ i ] == NULL )
			{
				error_hard( "invalid series data", 
							"internal problem in LSD", 
							"if error persists, please contact developers",
							true );
				myexit( 18 );
			}
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
		if ( start[ 0 ] <= i && end[ 0 ] >= i && ! is_nan( data[ 0 ][ i - start[ 0 ] ] ) )
			plog( "%d\t%.*g", "series", i, pdigits, data[ 0 ][ i - start[ 0 ] ] );
		else
			plog( "%d\t%s", "series", i, nonavail );		// write NaN as n/a
		
		for ( j = 1; j < nv; ++j )
		{
			if ( start[ j ] <= i && end[ j ] >= i && ! is_nan( data[ j ][ i - start[ j ] ] ) )
				plog( "\t%.*g", "series", pdigits, data[ j ][ i - start[ j ] ] );
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
			iniCase = 0;
			endCase = nv - 1;
			break;
			
		default:
			return;					// invalid type, do nothing
	}
	
	// get graphical configuration from Tk ( file defaults.tcl )
	hsize = get_int( "hsizeP" );			// 600
	vsize = get_int( "vsizeP" );			// 300
	tbordsize = get_int( "tbordsizeP" );	// 5

	// select gray scale or color range				
	color = allblack ? 1001 : 0;
	
	// alert once about plotting over more timesteps than plot window pixels
	step = hsize / ( double ) ( endCase - iniCase );
	if ( avgSmpl && ! avgSmplMsg && step < 1 )
	{
		if ( type == TSERIES )
			cmd( "set answer [ ttk::messageBox -parent .da -title Warning -icon warning -type yesno -default yes -message \"Disable Y values averaging?\" -detail \"The number of cases (time steps) to plot is larger than the physical plot width. To compute the Y values, LSD averages data from multiple cases.\n\nPress 'Yes' to disable Y values averaging or 'No' otherwise\n(this configuration can be also changed in menu 'Options').\"]" );
		else
			cmd( "set answer [ ttk::messageBox -parent .da -title Warning -icon warning -type yesno -default yes -message \"Disable series values averaging?\" -detail \"The number of series to plot is larger than the physical plot width. To compute the presented values, LSD averages data from multiple series.\n\nPress 'Yes' to disable Y values averaging or 'No' otherwise\n(this configuration can be also changed in menu 'Options').\"]" );
			
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
		y[ k ] = 0;
		pdataY[ k ] = new int [ hsize + 1 ];
		for ( i = 0; i < hsize + 1; ++i )
			pdataY[ k ][ i ] = -1;	// mark all as non plot
	}
	
	// create the window and the canvas
	plot_canvas( type, nv, start, end, str, tag, choice );
	
	// get graphical configuration real canvas
	hcanvas = get_int( "hcanvasP" );
	vcanvas = get_int( "vcanvasP" );
	hbordsize = get_int( "hbordsizeP" );
	lheight = get_int( "lheightP" );

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
					
					if ( i > min( iniCase, 1 ) && start[ k ] < i && end[ k ] >= i )
					{
						yVal = data[ k ][ i - start[ k ] ];
						tOk = true;
					}
					else
					{
						if ( start[ k ] == i || ( i <= 1 && start[ k ] <= 0 && end[ k ] >= i ) )
							yVal = data[ k ][ i - start[ k ] ];
						
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
							
						if ( line_point == 1 || ( y[ k ] >= cminy && y[ k ] <= cmaxy ) )
						{
							// constrain to canvas virtual limits
							y[ k ] = min( max( y[ k ], cminy ), cmaxy );
							// scale to the canvas physical y range and save to visual vertical line buffer
							pdataY[ k ][ j ] = ( int ) round( tbordsize + vsize * ( 1 - ( y[ k ] - cminy ) / ( cmaxy - cminy ) ) );
						}
						
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
							if ( line_point == 1 || ( yVal >= cminy && yVal <= cmaxy ) )
							{
								y[ k ] = min( max( yVal, cminy ), cmaxy );
								pdataY[ k ][ j ] = ( int ) round( tbordsize + vsize * ( 1 - ( y[ k ] - cminy ) / ( cmaxy - cminy ) ) );
							}
							
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
		
		int colorPlot = ( color < 1100 ) ? color : 0;	// use default foreground if out of colors

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
		cmd( "$daptab forget $daptab.tab%d", cur_plot );
		cmd( "destroy $daptab.tab%d", cur_plot );
		cmd( "if { [ $daptab index end ] == 0 } { \
				wm withdraw [ winfo toplevel $daptab ] \
			}" );
		return;
	}
	
	// enable/disable buttons
	cmd( "$w.b.s.save configure -state normal" );
	cmd( "$w.b.s.det configure -state normal -text Detach" );
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
				set w $daptab.tab%d.c; \
				set llim [ expr { %d * $zoom } ]; \
				set rlim [ expr { %d * $zoom } ]; \
				set tlim [ expr { %d * $zoom } ]; \
				set blim [ expr { %d * $zoom } ]; \
				set cx [ $w.f.plots canvasx %%x ]; \
				set cy [ $w.f.plots canvasy %%y ]; \
				if { $cx >= $llim && $cx <= $rlim && $cy >= $tlim && $cy <= $blim } { \
					$w.b.c.case.v configure -text [ expr { int( ( $cx - $llim ) * ( %d - %d) / ( $rlim - $llim ) + %d) } ]; \
					$w.b.c.y.v1 configure -text [ format \"%%%%.${pdigits}g\" [ expr { ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf } ] ]; \
					$w.b.c.y.v2 configure -text [ format \"%%%%.${pdigits}g\" [ expr { ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf } ] ] \
				} \
			}", cur_plot, cur_plot, hbordsize, hbordsize + hsize, tbordsize, tbordsize + vsize, endCase, iniCase, iniCase, maxy, miny, miny, maxy2, miny2, miny2 );
	else
		cmd( "bind $p <Motion> { \
				set type %d; \
				set zoom $zoomLevel%d; \
				set series $series%d; \
				set w $daptab.tab%d.c; \
				set llim [ expr { %d * $zoom } ]; \
				set rlim [ expr { %d * $zoom } ]; \
				set tlim [ expr { %d * $zoom } ]; \
				set blim [ expr { %d * $zoom } ]; \
				set cx [ $w.f.plots canvasx %%x ]; \
				set cy [ $w.f.plots canvasy %%y ]; \
				if { $cx >= $llim && $cx <= $rlim && $cy >= $tlim && $cy <= $blim } { \
					set case  [ expr { int( ( $cx - $llim ) * ( %d - %d) / ( $rlim - $llim ) + %d) } ]; \
					if { $type == 0 } { \
						$w.b.c.case.v configure -text $case \
					} elseif { $type == 1 } { \
						$w.b.c.case.v configure -text \"[ lindex $series $case ] (#[ expr { $case + 1 } ])\" \
					}; \
					$w.b.c.y.v1 configure -text [ format \"%%%%.${pdigits}g\" [ expr { ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf } ] ]; \
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
			
		cmd( "$p bind p%d <Enter> { $daptab.tab%d.c.b.c.var.v configure -text \"%s\" }", i, cur_plot, txtLab );
		cmd( "$p bind p%d <Leave> { $daptab.tab%d.c.b.c.var.v configure -text \"\" }", i, cur_plot );

		cmd( "$p bind txt%d <Enter> { $daptab.tab%d.c.b.c.var.v configure -text \"%s\" }", i,cur_plot, txtLab );
		cmd( "$p bind txt%d <Leave> { $daptab.tab%d.c.b.c.var.v configure -text \"\" }", i, cur_plot );
	}

	// save plot info
	type_plot[ cur_plot ] = type; 					// plot type
	plot_w[ cur_plot ] = hcanvas;					// plot width
	plot_nl[ cur_plot ] = tbordsize + vsize + lheight; // height of plot without labels  
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
	hsize = get_int( "hsizeP" );			// 600
	vsize = get_int( "vsizeP" );			// 300
	hcanvas = get_int( "hcanvasP" );
	vcanvas = get_int( "vcanvasP" );
	tbordsize = get_int( "tbordsizeP" );	// 5
	hbordsize = get_int( "hbordsizeP" );
	lheight = get_int( "lheightP");			// 15

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
		cmd( "$daptab forget $daptab.tab%d", cur_plot );
		cmd( "destroy $daptab.tab%d", cur_plot );
		cmd( "if { [ $daptab index end ] == 0 } { \
				wm withdraw [ winfo toplevel $daptab ] \
			}" );
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
	cmd( "$w.b.s.det configure -state normal -text Detach" );
	cmd( "$w.b.z.b.p configure -state normal" );
	cmd( "$w.b.z.b.m configure -state normal" );

	// raise axis, legends & draws to the front and lower grid to the back
	cmd( "$p raise p" );
	cmd( "$p lower g" );
	cmd( "if { [ $p find withtag draw ] != \"\" } { $p raise draw }" );
	
	// update cursor indicators at bottom window
	cmd( "bind $p <Motion> { \
			set zoom $zoomLevel%d; \
			set w $daptab.tab%d.c; \
			set llim [ expr { %d * $zoom } ]; \
			set rlim [ expr { %d * $zoom } ]; \
			set tlim [ expr { %d * $zoom } ]; \
			set blim [ expr { %d * $zoom } ]; \
			set cx [ $w.f.plots canvasx %%x ]; \
			set cy [ $w.f.plots canvasy %%y ]; \
			if { $cx >= $llim && $cx <= $rlim && $cy >= $tlim && $cy <= $blim } { \
				$w.b.c.case.v configure -text [ expr { min( 1 + int( ( $cx - $llim ) * %d / ( $rlim - $llim ) ), %d ) } ]; \
				$w.b.c.y.v1 configure -text [ format \"%%%%.${pdigits}g\" [ expr { ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf } ] ]; \
				$w.b.c.y.v2 configure -text \"( n=[ expr { int( ( $blim - $cy ) * ( %lf - %lf ) / ( $blim - $tlim ) + %lf ) } ] )\" \
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
			
		cmd( "$p bind p%d <Enter> { $daptab.tab%d.c.b.c.var.v configure -text \"%s\" }", i, cur_plot, txtLab );
		cmd( "$p bind p%d <Leave> { $daptab.tab%d.c.b.c.var.v configure -text \"\" }", i, cur_plot );

		cmd( "$p bind txt%d <Enter> { $daptab.tab%d.c.b.c.var.v configure -text \"%s\" }", i,cur_plot, txtLab );
		cmd( "$p bind txt%d <Leave> { $daptab.tab%d.c.b.c.var.v configure -text \"\" }", i, cur_plot );
	}

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
	int h, i, color, hsize, vsize, hbordsize, tbordsize, bbordsize, sbordsize, htmargin, hticks, vticks, lheight, hcanvas, vcanvas, nLine;
	double yVal, cminy2, cmaxy2;
	
	// get graphical configuration from Tk ( file defaults.tcl )
	hsize = get_int( "hsizeP" );			// 600
	vsize = get_int( "vsizeP" );			// 300
	hbordsize = get_int( "hmbordsizeP" );	// 40
	tbordsize = get_int( "tbordsizeP"	);	// 5
	bbordsize = get_int( "bbordsizeP"	);	// 90
	sbordsize = get_int( "sbordsizeP"	);	// 0
	htmargin = get_int( "htmarginP" );		// 4
	hticks = get_int( "hticksP" );			// 3
	vticks = get_int( "vticksP" );			// 3
	lheight = get_int( "lheightP" );		// 15

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
		
	// include margins and tick size (if present)
	hbordsize = ( int ) max( hbordsize, h ) + 2 * htmargin + 5;
	cmd( "set hbordsizeP %d", hbordsize );
	
	// initial canvas size
	hcanvas = hsize + hbordsize * ( y2on ? 2 : 1 ) + 
				( y2on ? 0 : ( max_c < 1000 ? 10 : 20 ) );
	vcanvas = vsize + tbordsize + bbordsize;
	cmd( "set hcanvasP %d; set vcanvasP %d", hcanvas, vcanvas );
	
	// create plot notepad window, tab & canvas
	add_da_plot_tab( ".dap", cur_plot );    // plot tab window
	
	cmd( "set p $w.f.plots" );				// plot canvas
	
	cmd( "ttk::frame $w.f" );
	cmd( "ttk::scrollbar $w.f.vs -command \"$p yview\"" );
	cmd( "ttk::scrollbar $w.f.hs -orient horiz -command \"$p xview\"" );
	cmd( "pack $w.f.vs -side right -fill y" );
	cmd( "pack $w.f.hs -side bottom -fill x" );

	cmd( "ttk::canvas $p -width %d -height %d -yscrollcommand \"$w.f.vs set\" -xscrollcommand \"$w.f.hs set\" -scrollregion \"%d %d %d %d\" -dark $darkTheme", 
		hcanvas, vcanvas, - sbordsize, - sbordsize, hcanvas + sbordsize, vcanvas + sbordsize  );
	cmd( "pack $p -expand yes -fill both" );
	cmd( "pack $w.f -expand yes -fill both" );
	cmd( "mouse_wheel $p" );
		
	// add buttons bottom bar
	cmd( "ttk::frame $w.b" );

	cmd( "ttk::frame $w.b.c" );
	
	// adjust horizontal text space usage
	if ( y2on )							
		cmd( "set labWid 15; set datWid 30; if [ string equal $CurPlatform windows ] { set pad1 6; set pad2 10 } { set pad1 0; set pad2 5 }" );
	else
		cmd( "set labWid 15; set datWid 25; if [ string equal $CurPlatform windows ] { set pad1 6; set pad2 10 } { set pad1 0; set pad2 5 }" );
	
	// adjust vertical text adjustment
	cmd( "if [ string equal $CurPlatform linux ] { set pad3 0 } { if [ string equal $CurPlatform mac ] { set pad3 3 } { set pad3 -3 } }" );
	
	cmd( "ttk::frame $w.b.c.case" );
	cmd( "ttk::label $w.b.c.case.l -text \"%s:\" -width $labWid -anchor e", txtCase );
	cmd( "ttk::label $w.b.c.case.v -text \"\" -style hl.TLabel -width $datWid -anchor w" );
	cmd( "pack $w.b.c.case.l $w.b.c.case.v -side left -anchor w" );

	cmd( "ttk::frame $w.b.c.y" );
	cmd( "ttk::label $w.b.c.y.l -text \"%s:\" -width $labWid -anchor e", txtValue );
	cmd( "ttk::label $w.b.c.y.v1 -text \"\" -style hl.TLabel -width [ expr { $datWid / 2 } ] -anchor w" );
	cmd( "ttk::label $w.b.c.y.v2 -text \"\" -style hl.TLabel -width [ expr { $datWid / 2 } ] -anchor w" );
	cmd( "pack $w.b.c.y.l $w.b.c.y.v1 $w.b.c.y.v2 -side left -anchor w" );

	cmd( "ttk::frame $w.b.c.var" );
	cmd( "ttk::label $w.b.c.var.l -text \"%s:\" -width $labWid -anchor e", txtLine );
	cmd( "ttk::label $w.b.c.var.v -text \"\" -style hl.TLabel -width $datWid -anchor w" );
	cmd( "pack $w.b.c.var.l $w.b.c.var.v -side left -anchor w" );

	cmd( "pack $w.b.c.case $w.b.c.y $w.b.c.var -anchor w" );

	cmd( "ttk::frame $w.b.o" );
	cmd( "ttk::label $w.b.o.l1 -text \"Alt-click: properties\"" );
	cmd( "ttk::label $w.b.o.l2 -text \"Shift-click: add text\"" );
	cmd( "ttk::label $w.b.o.l3 -text \"Ctrl-click: add line\"" );
	cmd( "pack $w.b.o.l1 $w.b.o.l2 $w.b.o.l3" );

	cmd( "ttk::frame $w.b.s" );
	cmd( "ttk::button $w.b.s.save -width $butWid -text Save -state disabled -command { \
			set it \"%d) %s\"; \
			set fromPlot 1; \
			set choice 11 \
		}", cur_plot, ( char * ) Tcl_GetVar( inter, "tit", 0 ) );
	cmd( "ttk::button $w.b.s.det -width $butWid -text Detach -state disabled -command { \
			if { [ $daptab.tab%d.c.b.s.det cget -text ] eq \"Stop\" } { \
				set choice 2 \
			} else { \
				detach_tab $daptab tab%d c.b.s.det .da %d \
			} \
		}", cur_plot, cur_plot, MAX_TAB_LEN - 1	);
	cmd( "pack $w.b.s.save $w.b.s.det -pady 4" );

	cmd( "ttk::frame $w.b.z" );
	cmd( "ttk::label $w.b.z.l -text Zoom" );

	cmd( "ttk::frame $w.b.z.b" );
	cmd( "ttk::button $w.b.z.b.p -width 3 -text + -command { scale_canvas $daptab.tab%d.c.f.plots \"+\" zoomLevel%d } -state disabled", cur_plot, cur_plot );
	cmd( "ttk::button $w.b.z.b.m -width 3 -text - -command { scale_canvas $daptab.tab%d.c.f.plots \"-\" zoomLevel%d } -state disabled", cur_plot, cur_plot  );
	cmd( "pack $w.b.z.b.p $w.b.z.b.m -pady 4" );

	cmd( "pack  $w.b.z.l $w.b.z.b -side left -padx 2" );

	cmd( "ttk::label $w.b.pad -width $pad1" );

	cmd( "pack $w.b.c $w.b.o $w.b.pad $w.b.s $w.b.z -padx $pad2 -pady 5 -side left" );
	cmd( "pack $w.b -side right" );

	if ( watch )
		cmd( "$w.b.s.det configure -state normal -text Stop" );

	// hack to bring the new plot to the foreground during debugging in macOS
	cmd( "if { $running && [ string equal [ tk windowingsystem ] aqua ] } { \
			wm transient .dap \
		}" );
	
	cmd( "$p xview moveto 0; $p yview moveto 0" );
	cmd( "set zoomLevel%d 1.0", cur_plot );
	
	// create list with the series names+tags in Tk
	cmd( "set series%d [ list ]", cur_plot );
	for ( i = 0; i < nv; ++i )
		cmd( "lappend series%d \"%s_%s\"", cur_plot, str[ i ], tag[ i ] );
		
	// axis lines, ticks & grid (adjust ticks for few horizontal cases)
	if ( type == TSERIES && ( max_c - min_c ) / ( hticks + 1 ) < 10 )
		for ( ; hticks > 0; --hticks )
			if ( ( max_c - min_c ) % ( hticks + 1 ) == 0 )
				break;
			
	cmd( "canvas_axis $p %d %d %d %d", type, grid, hticks, y2on );

	// x-axis values
	switch ( type )
	{
		case TSERIES:
			for ( i = 0; i < hticks + 2; ++i )
				cmd( "$p create text %d [ expr { %d + $pad3 } ] -fill $colorsTheme(dfg) -font $fontP -anchor n -text %d -tag { p text }", hbordsize + ( int ) round( i * ( double ) hsize / ( hticks + 1 ) ), vsize + lheight, min_c + ( int ) floor( i * ( double ) ( max_c - min_c ) / ( hticks + 1 ) ) );
			break;
			
		case CRSSECT:
			cmd( "$p create text %d [ expr { %d + $pad3 } ] -fill $colorsTheme(dfg) -font $fontP -anchor nw -text \"%d series\" -tag { p text }", hbordsize, vsize + lheight, nv );
			break;
			
		case HISTOGR:
		case HISTOCS:
			for ( i = 0; i < hticks + 2; ++i )
				cmd( "$p create text %d [ expr { %d + $pad3 } ] -fill $colorsTheme(dfg) -font $fontP -anchor n -text %.*g -tag { p text }", hbordsize + ( int ) round( i * ( double ) hsize / ( hticks + 1 ) ), vsize + lheight, pdigits, bins[ 0 ].lowb + i * ( bins[ num_bins - 1 ].highb - bins[ 0 ].lowb ) / ( hticks + 1 ) );
			break;			
	}

	// y-axis values
	for ( i = 0; i < vticks + 2; ++i )
	{
		yVal = miny + ( vticks + 1 - i ) * ( maxy - miny ) / ( vticks + 1 );
		yVal = ( fabs( yVal ) < ( maxy - miny ) * MARG ) ? 0 : yVal;
		
		cmd( "$p create text %d %d -fill $colorsTheme(dfg) -font $fontP -anchor e -text %.*g -tag { p text }", hbordsize - htmargin - 5, tbordsize + ( int ) round( i * ( double ) vsize / ( vticks + 1 ) ), pdigits, yVal );
		
		// second y-axis series values ( if any )
		if ( y2on )
		{
			yVal = cminy2 + ( vticks + 1 - i ) * ( cmaxy2 - cminy2 ) / ( vticks + 1 );
			yVal = ( fabs( yVal ) < ( cmaxy2 - cminy2 ) * MARG ) ? 0 : yVal;
		
			cmd( "$p create text %d %d -fill $colorsTheme(dfg) -font $fontP -anchor w -text %.*g -tag { p text }", hbordsize + hsize + htmargin + 5, tbordsize + ( int ) round( i * ( double ) vsize / ( vticks + 1 ) ), pdigits, yVal );
		}
	}

	// series labels
	cmd( "set xlabel $htmarginP" ); 
	cmd( "set ylabel [ expr { $tbordsizeP + $vsizeP + 2 * $lheightP } ]" );
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
			cmd( "if { [ expr { $xlabel + $app } ] > %d } { set xlabel %d; incr ylabel %d }", hcanvas - htmargin, htmargin, lheight );
			h = get_int( "ylabel" );
			if ( h > tbordsize + vsize + bbordsize - 2 * lheight )
				break;
			cmd( "$p create text $xlabel $ylabel -font $fontP -anchor nw -text \"%s\" -tag { txt%d text legend } -fill $c%d", txtLab, i, ( color < 1100 ) ? color : 0 );
			cmd( "set xlabel [ expr { $xlabel + $app + $htmarginP } ]" );
		}
	}

	if ( i < nLine )
		cmd( "$p create text $xlabel $ylabel -fill $colorsTheme(fg) -font $fontP -anchor nw -text \"(%d more...)\"", nLine - i );

	// create context menu and common bindings
	canvas_binds( cur_plot );
	
	cmd( "update idletasks" );	
}


/*******************************************************
 CANVAS_BINDS
 create canvas context menu and common bindings 
 *******************************************************/
void canvas_binds( int n )
{
	cmd( "set p $daptab.tab%d.c.f.plots", n );		// plot canvas

	// context menu
	cmd( "ttk::menu $p.v -tearoff 0" );
	cmd( "$p.v add command -label Properties -command { event generate $daptab.tab%d.c.f.plots <Alt-1> -x $menuX -y $menuY }", n );
	cmd( "$p.v add command -label Delete -command { \
			set ccanvas $daptab.tab%d.c.f.plots; \
			if { $menuSel ne \"\" } { \
				$ccanvas delete $menuSel \
			} \
		}", n );
	cmd( "$p.v add command -label \"Add Text\" -command { event generate $daptab.tab%d.c.f.plots <Shift-1> -x $menuX -y $menuY }", n );
	cmd( "$p.v add separator" );
	cmd( "$p.v add command -label Help -accelerator F1 -command { LsdHelp menudata_res.html#graph }" );
	
	// canvas bindings
	cmd( "bind $p <Button-2> { \
			set ccanvas $daptab.tab%d.c.f.plots; \
			set menuX %%x; \
			set menuY %%y; \
			set menuSel \"\"; \
			set type [ $ccanvas gettags current ]; \
			set curitem [ $ccanvas find withtag current ]; \
			if { [ lsearch $type line ] >= 0 || [ lsearch $type dots ] >= 0 } { \
				if { [ lsearch $type series ] >= 0 } { \
					set menuSel [ lindex $type 0 ] \
				} { \
					set menuSel $curitem \
				} \
			} elseif { [ lsearch $type bar ] >= 0 || [ lsearch $type text ] >= 0 } { \
				set menuSel $curitem \
			}; \
			if { $menuSel eq \"\" } { \
				$ccanvas.v entryconfig 0 -state disabled; \
				$ccanvas.v entryconfig 1 -state disabled \
			} { \
				$ccanvas.v entryconfig 0 -state normal; \
				$ccanvas.v entryconfig 1 -state normal \
			}; \
			tk_popup $ccanvas.v %%X %%Y \
		}", n );
	cmd( "bind $p <Button-3> { event generate $daptab.tab%d.c.f.plots <Button-2> -x %%x -y %%y }", n );
	
	cmd( "bind $p <Double-Button-1> { focustop .da }" );
	
	cmd( "bind $p <Alt-1> { \
			set ccanvas $daptab.tab%d.c.f.plots; \
			set LX %%X; set LY %%Y; \
			set type [ $ccanvas gettags current ]; \
			set curitem [ $ccanvas find withtag current ]; \
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
		}", n );

	cmd( "bind $p <Shift-1> { \
			set ccanvas $daptab.tab%d.c.f.plots; \
			set LX %%X; \
			set LY %%Y; \
			set hereX [ $ccanvas canvasx %%x ]; \
			set hereY [ $ccanvas canvasy %%y ]; \
			set choice 27 \
		}", n );
		
	cmd( "bind $p <Control-1> { \
			set ccanvas $daptab.tab%d.c.f.plots; \
			set ncanvas %d; \
			set hereX [ $ccanvas canvasx %%x ]; \
			set hereY [ $ccanvas canvasy %%y ]; \
			unset -nocomplain cl; \
			set choice 28 \
		}", n, n );

	cmd( "bind $p <Button-1> { \
			set ccanvas $daptab.tab%d.c.f.plots; \
			set type [ $ccanvas gettags current ]; \
			if { [ lsearch -regexp $type (draw|legend) ] >= 0 } { \
				set moving true; \
				set hereX [ $ccanvas canvasx %%x ]; \
				set hereY [ $ccanvas canvasy %%y ]; \
				$ccanvas raise current \
			} { \
				set moving false \
			} \
		}", n );
	cmd( "bind $p <B1-Motion> { \
			set ccanvas $daptab.tab%d.c.f.plots; \
			if $moving { \
				$ccanvas move current [ expr { [ $ccanvas canvasx %%x ] - $hereX } ] \
					[ expr { [ $ccanvas canvasy %%y ] - $hereY } ]; \
				set hereX [ $ccanvas canvasx %%x ]; \
				set hereY [ $ccanvas canvasy %%y ] \
			} \
		}", n );
	cmd( "bind $p <ButtonRelease-1> { set moving false }" );
	
}


/*******************************************************
 ADD_DA_PLOT_TAB
 add new plot tab to data analysis notepad
 *******************************************************/
void add_da_plot_tab( const char *w, int id_plot )
{
	int n;
		
	// create notebook, if not exists
	cmd( "set w %s", w );
	cmd( "set daptab $w.pad" );
	cmd( "if { ! [ winfo exists $daptab ] } { \
			newtop $w \"%s%s - LSD Plots\" \"wm withdraw $w\" \"\"; \
			wm transient $w .da; \
			ttk::notebook $daptab; \
			pack $daptab -expand yes -fill both; \
			ttk::notebook::enableTraversal $daptab; \
			showtop $w; \
			bind $w <F1> { LsdHelp menudata_res.html#graph }; \
			bind $w <Escape> \"wm withdraw $w\"; \
		} else { \
			settop $w \
		}", unsaved_change( ) ? "*" : " ", simul_name );
		
	// create tab frame with heading
	cmd( "set t $daptab.tab%d", id_plot );
	cmd( "if [ winfo exists $t ] { \
			if { $t in [ $daptab  tabs ] } { \
				$daptab forget $t \
			}; \
			destroy $t \
		}" );
	cmd( "ttk::labelframe $t -text \"%d) $tit\"", id_plot );
	cmd( "pack $t" );

	// create content frame (for moving when detaching)
	cmd( "set w $t.c" );
	cmd( "ttk::frame $w" );
	cmd( "pack $w -expand yes -fill both" );
	
	cmd( "set tt [ string range [ lindex [ split $tit ] 0 ] 0 %d ]", MAX_TAB_LEN - 1 );
	
	cmd( "set n [ .da.vars.pl.f.v size ]" );
	n = get_int( "n" ) + 1;
	
	if ( n <= MAX_PLOT_TABS )
	{
		cmd( "$daptab add $t -text \"$tt\"" );
		cmd( "$daptab select $t" );
		
		cmd( "if { \"$daptab.more\" in [ $daptab tabs ] } { \
				$daptab forget $daptab.more; \
				destroy $daptab.more \
			}" );
	}
	
	if ( n == MAX_PLOT_TABS + 1 )
	{
		cmd( "ttk::frame $daptab.more" );
		cmd( "pack $daptab.more" );
		cmd( "$daptab insert 0 $daptab.more -text \"More...\" -underline 0" );
	}
	
	if ( n > MAX_PLOT_TABS ) 
	{
		cmd( "$daptab forget 1" );
		cmd( "$daptab add $t -text \"$tt\"" );
		cmd( "$daptab select $t" );
		
		cmd( "set addplot \"%d) $tit\"", id_plot );
		update_more_tab( w, true );
	}
}


/*******************************************************
 UPDATE_MORE_TAB
 update the plots index tab, if it exists
 *******************************************************/
void update_more_tab( const char *w, bool adding )
{
	char *tt;
	int i, j, k, m, n, cols;
	
	if ( platform == MAC )
		cols = 3;
	else
		cols = 4;
	
	cmd( "if { [ info exists daptab ] && [ winfo exists $daptab.more ] } { \
			set _tmp 1 \
		} else { \
			set _tmp 0 \
		}" );
		
	if ( ! get_bool( "_tmp" ) )
		return;
	
	cmd( "set plotlist [ .da.vars.pl.f.v get 0 end ]" );
	
	if ( adding )
		cmd( "lappend plotlist \"$addplot\"" );
	
	cmd( "set n [ llength $plotlist ]" );
	n = get_int( "n" );
	
	cmd( "destroy $daptab.more.b");
	cmd( "ttk::frame $daptab.more.b");
	
	for ( i = 0; cols * i + 1 <= n; ++i )
	{
		cmd( "ttk::frame $daptab.more.b.l%d", i );
		
		for ( j = 1; j <= cols && cols * i + j <= n; ++j )
		{
			k = cols * i + j;
			
			cmd( "set t [ lindex $plotlist %d ]", k - 1 );
			cmd( "set m [ string trim [ lindex [ split $t ] 0 ] \")\" ]" );
			cmd( "set tt [ string range [ lindex [ split $t ] 1 ] 0 %d ]", MAX_TAB_LEN - 1 );
			m = get_int( "m" );
			tt = get_str( "tt" );
			
			cmd( "ttk::button $daptab.more.b.l%d.b%d -width -1 -text $t -command { \
					set _w $daptab.tab%d; \
					if [ istoplevel $_w ] { \
						focustop $_w \
					} else { \
						if { $_w ni [ $daptab tabs ] } { \
							if { [ $daptab index end ] >= %d } { \
								$daptab forget 1 \
							}; \
							$daptab add $_w -text \"%s\" \
						}; \
						$daptab select $_w \
					} \
				}", i, k, m, MAX_PLOT_TABS, tt );
			cmd( "pack $daptab.more.b.l%d.b%d -side left -padx 2", i, k );
		}
		
		cmd( "pack $daptab.more.b.l%d -anchor w -pady 2", i );
	}
	
	cmd( "pack $daptab.more.b -padx 20 -pady 20" );
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
		cmd( "ttk::messageBox -parent .da -title Error -icon error -type ok -message \"Cannot open plot\" -detail \"The plot file produced by Gnuplot is not available.\nPlease check if you have selected an adequate configuration for the plot.\nIf the error persists please check if Gnuplot is installed and set up properly.\"" );
		return 1;
	}

	f1 = fopen( "plot_clean.file", "w" );
	if ( f == NULL )
	{
		cmd( "ttk::messageBox -parent .da -title Error -icon error -type ok -message \"Cannot create plot file\" -detail \"Please check if the drive or the directory is not set READ-ONLY or full\nand try again\"" );
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
			sscanf( str1, "[ expr { $cmx * %d / 1000 } ]", &x1 );
		
			i = strcspn( str + j + 1, "[" );
			i += j + 1;
			j = strcspn( str + i + 1, "]" );
			j += i + 1;
			strncpy( str2, str + i, j - i + 1 );
			str2[ j - i + 1 ]='\0';
			sscanf( str2, "[ expr { $cmy * %d / 1000 } ]", &x2 );
		
			i = strcspn( str + j + 1, "[" );
			i += j + 1;
			j = strcspn( str + i + 1, "]" );
			j += i + 1;
			strncpy( str3, str + i, j - i + 1 );
			str3[ j - i + 1 ] = '\0';
			sscanf( str3, "[ expr { $cmx * %d / 1000 } ]", &x3 );
		
			i = strcspn( str + j + 1, "[" );
			i += j + 1;
			j = strcspn( str + i + 1, "]" );
			j += i + 1;
			strncpy( str4, str + i, j - i + 1 );
			str4[ j - i + 1 ] = '\0';
			sscanf( str4, "[ expr { $cmy * %d / 1000 } ]", &x4 );
			
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
