/*************************************************************

	LSD 7.3 - December 2020
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/*************************************************************
RUN_TIME.CPP 
Contains initialization and management of run-time plotting

The main functions contained here are:

- void prepare_plot( object *r, int id_sim )
Checks is there are LSD variables to plot. If not, returns immediately. Otherwise
initiliaze the run time globale variables. Namely, the vector of the labels for
the variables of plot. The plot window is initialized according to the id_sim name

- void count( object *r, int *i );
Recursive function that increments i of one for any variable to plot.

- void assign( object *r, int *i, char *lab );
Create a list of Variables to plot and create the list of labels (adding
the indexes if necessary) to be used in the plot.


- void init_plot( int i, int id_sim );
create the canvas for the plot, the lines, button, labels, etc.

- void plot_rt( variable *v )
the function used run time to plot the value of variable v
*************************************************************/

#include "decl.h"

char intval[ 100 ];				// string buffer
double ymax;
double ymed;
double ymin;
double *old_val;
variable **list_var;


/**************************************
PREPARE_PLOT
**************************************/
void prepare_plot( object *r, int id_sim )
{
	int i = 0;
	char lab[ MAX_ELEM_LENGTH ];
	
	ymax = ymin = 0;
	strcpy( lab, "" );
	count( r, &i );
	if ( i == 0 )
		return;
	
	cmd( "unset -nocomplain tp" );
	list_var = new variable *[ i ];
	old_val = new double [ i ];
	i = 0;
	assign( r, &i, lab );
	init_plot( i, id_sim );
}


/**************************************
COUNT
**************************************/
void count( object *r, int *i )
{
	bridge *cb;
	object *c;
	variable *a;

	for ( a = r->v; a != NULL; a = a->next)
		if ( a->plot == 1 )
			*i = *i + 1;

	for ( cb = r->b; cb != NULL; cb = cb->next )
		for ( c = cb->head; c != NULL; c = c->next )
			count( c, i );
}


/**************************************
ASSIGN
**************************************/
void assign( object *r, int *i, char *lab )
{
	char cur_lab[ MAX_ELEM_LENGTH ];
	int j;
	bridge *cb;
	object *c, *c1;
	variable *a;
	
	for ( a = r->v; a != NULL; a = a->next )
		if ( a->plot == 1 )
		{
			list_var[ *i ] = a; 	// assigns the address of a to the list to plot
			sprintf( msg, "%s%s", a->label, lab );
			cmd( "lappend tp %s", msg );
			*i = *i + 1;
		}

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;
		
		c = cb->head;
		if ( c->next != NULL ) 		// multiple instances
			for ( j = 1, c1 = c; c1 != NULL; c1 = go_brother( c1 ), ++j )
			{
				sprintf( cur_lab, "%s_%d", lab, j );
				assign( c1, i, cur_lab );
			}
		else 						// unique instance
			assign( c, i, lab );
	}
}


/**************************************
INIT_PLOT
**************************************/
void init_plot( int num, int id_sim )
{
	cmd( "if { %d > $hsizeR } { set plot_step 1 } { set plot_step [ expr $hsizeR / %d.0 ] }", max_step, max_step );
	
	cmd( "set scrollB 1" );
	scrollB = 1;

	cmd( "set activeplot .plt%d", id_sim );

	cmd( "destroytop $activeplot" );

	cmd( "newtop $activeplot \"\" { set_c_var done_in 5 } \"\"" );
	cmd( "wm transient $activeplot ." );
	cmd( "wm title $activeplot \"%s%s(%d) - LSD Run-time Plot\"", unsaved_change() ? "*" : " ", simul_name, id_sim  );
	
	cmd( "ttk::frame $activeplot.c" );
	
	// vertical scale values
	cmd( "ttk::canvas $activeplot.c.yscale -width $sclhsizeR -height [ expr $vsizeR + $sclvmarginR + $botvmarginR ] -entry 0 -dark $darkTheme" );

	cmd( "$activeplot.c.yscale create text $sclhsizeR [ expr max( $sclvmarginR, 10 ) ] -anchor e -justify right -text \"\" -fill $colorsTheme(dfg) -tag ymax" );
	cmd( "$activeplot.c.yscale create text $sclhsizeR [ expr $sclvmarginR + $vsizeR / 2 ] -anchor e -justify right -text \"\" -fill $colorsTheme(dfg) -tag medy" );
	cmd( "$activeplot.c.yscale create text $sclhsizeR [ expr $sclvmarginR + $vsizeR ] -anchor e -justify right -text \"\" -fill $colorsTheme(dfg) -tag ymin" );
	
	cmd( "pack $activeplot.c.yscale -side left -anchor nw" );
	
	// main canvas
	cmd( "ttk::frame $activeplot.c.c  " );
	cmd( "set p $activeplot.c.c.cn" );
	cmd( "ttk::scrollbar $activeplot.c.c.hscroll -orient horiz -command \"$p xview\"" );
	cmd( "ttk::canvas $p -width [ expr $hsizeR + 2 * $cvhmarginR ] -height [ expr $vsizeR + $sclvmarginR + $botvmarginR ] -scrollregion \"0 0 %d [ expr $vsizeR + $sclvmarginR + $botvmarginR ]\" -xscrollcommand \"$activeplot.c.c.hscroll set\" -xscrollincrement 1 -yscrollincrement 1 -dark $darkTheme", max_step );
	cmd( "pack $activeplot.c.c.hscroll -side bottom -expand yes -fill x" );
	cmd( "mouse_wheel $p" );
	
	// horizontal grid lines
	cmd( "for { set i 0 } { $i <= $vticksR } { incr i } { \
			if { $i > 0 && $i < $vticksR } { \
				set color $colorsTheme(bg) \
			} else { \
				set color $colorsTheme(dfg) \
			}; \
			$p create line [ expr $cvhmarginR - $ticmarginR ] [ expr $sclvmarginR + $vsizeR * $i / $vticksR ] [ expr $cvhmarginR ] [ expr $sclvmarginR + $vsizeR * $i / $vticksR ] -fill $colorsTheme(dfg); \
			$p create line [ expr $cvhmarginR ] [ expr $sclvmarginR + $vsizeR * $i / $vticksR ] [ expr $cvhmarginR + %d * $plot_step ] [ expr $sclvmarginR + $vsizeR * $i / $vticksR ] -fill $color \
		}", max_step );

	// vertical grid lines
	cmd( "set k [ expr $vsizeR + $sclvmarginR ]" );
	cmd( "for { set i 0; set j $cvhmarginR; set u -1 } { $j <= [ expr $cvhmarginR + %d * $plot_step ] } { incr i; set j [ expr $j + $hsizeR / $hticksR ] } { \
			if { $plot_step > 1 } { \
				set l [ expr %d * $i / $hticksR ] \
			} else { \
				set l [ expr $j - $cvhmarginR ] \
			}; \
			if { $j > $cvhmarginR && $j < [ expr $cvhmarginR + %d * $plot_step ] } { \
				set color $colorsTheme(bg) \
			} else { \
				set color $colorsTheme(dfg) \
			}; \
			$p create line $j $sclvmarginR $j $k -fill $color; \
			$p create line $j $k $j [ expr $k + $ticmarginR ] -fill  $colorsTheme(dfg); \
			if { $l > $u } { \
				$p create text $j [ expr $k + $ticmarginR ] -text $l -anchor n -fill $colorsTheme(dfg); \
				set u $l \
			} \
	}	", max_step, max_step, max_step );
	
	cmd( "pack $p -anchor nw" );
	cmd( "pack $activeplot.c.c -anchor nw" );
	cmd( "pack $activeplot.c -anchor nw" );
	cmd( "$p xview moveto 0" );
	
	// bottom part
	cmd( "ttk::canvas $activeplot.fond -width [ expr $sclhsizeR + $hsizeR + 2 * $cvhmarginR ] -height $botvsizeR -entry 0 -dark $darkTheme" );

	// controls
	cmd( "ttk::checkbutton $activeplot.fond.shift -text Scroll -variable scrollB -command { set_c_var done_in 8 }" );	
	cmd( "if [ string equal $CurPlatform windows ] { set goWid 7 } { set goWid 6 }" );
	cmd( "ttk::button $activeplot.fond.go -width $goWid -text Center -command { set_c_var done_in 7 }" );

	cmd( "$activeplot.fond create window [ expr $sclhsizeR / 2 ] [ expr $botvsizeR / 4 - 5 ] -window $activeplot.fond.shift" );
	cmd( "$activeplot.fond create window [ expr $sclhsizeR / 2 ] [ expr 3 * $botvsizeR / 4 - 2 ] -window $activeplot.fond.go" );
	
	// labels
	cmd( "for { set i 0; set j 0; set k 0 } { $i < [ expr min( %d, $linlabR * $lablinR ) ] } { incr i } { \
			$activeplot.fond create text [ expr $sclhsizeR + $sclvmarginR + $j * $hsizeR / $lablinR ] [ expr $k * $linvsizeR ] -anchor nw -text [ lindex $tp $i ] -fill [ set c$i ]; \
			if { $j < [ expr $lablinR - 1 ] } { \
				incr j \
			} else { \
				incr k; \
				set j 0 \
			} \
		}", num );
		
	cmd( "pack $activeplot.fond -expand yes -fill both -pady 7" );
	
	cmd( "set shiftW [ expr %d * $shiftR ]", id_sim - 1 );	// calculate window shift position
	cmd( "showtop  $activeplot righttoMshift no no no" );
	
	if ( fast_mode > 0 )
	{
		cmd( "wm withdraw $activeplot" );
		cmd( "$activeplot.fond.go conf -state disabled" );
		cmd( "$activeplot.fond.shift conf -state disabled" );
	}

	cmd( "focustop .log" );

	set_shortcuts_log( "$activeplot", "runtime.html" );
}


/**************************************
PLOT_RT
**************************************/
void plot_rt( variable *v )
{
	bool relabel = false;
	int height, p_digits;
	double value, scale, zero_lim;
	
	height = get_int( "vsizeR", &height );
	p_digits = get_int( "pdigitsR", &p_digits );
	
	// limit the number of run-time plot variables
	if ( cur_plt > 100 )
		return;

	if ( ymax == ymin ) 		// very initial setting
	{ 
		if ( v->val[ 0 ] > 0 )
			ymax = round_digits( v->val[ 0 ] * ( 1 + MARG ), p_digits );
		else
			ymax = round_digits( v->val[ 0 ] * ( 1 - MARG ), p_digits );
		
		ymin = round_digits( v->val[ 0 ], p_digits );
		
		if ( ymax == ymin )
			ymax += MARG;
		
		relabel = true;
	}
	
	if ( v->val[ 0 ] >= ymax )
	{
		value = v->val[ 0 ] * ( v->val[ 0 ] > 0 ? 1 + MARG_CONST : 1 - MARG_CONST );
		value = round_digits( value, p_digits );
	  
		scale = ( ymax - ymin ) / ( value - ymin );
		ymax = value;	
		
		relabel = true;
		
		cmd( "$activeplot.c.c.cn scale punto 0 $vsizeR 1 %lf", scale  < 0.01 ? 0.01 : scale  );
	}

	if ( v->val[ 0 ] <= ymin )
	{
		value = v->val[ 0 ] * ( v->val[ 0 ] > 0 ? 1 - MARG_CONST : 1 + MARG_CONST );
		value = min( value, ymin - ( ymax - ymin ) / height );
		value = round_digits( value, p_digits );

		scale = ( ymax - ymin ) / ( ymax - value );
		ymin = value;
		
		relabel = true;
		
		cmd( "$activeplot.c.c.cn scale punto 0 0 1 %lf", scale < 0.01 ? 0.01 : scale  );
	}

	if ( relabel )
	{
		ymed = round_digits( ( ymax - ymin ) / 2 + ymin, p_digits );
		zero_lim = ( ymax - ymin ) * MARG;
		
		cmd( "$activeplot.c.yscale itemconf ymax -text %.*g", p_digits, fabs( ymax ) < zero_lim ? 0 : ymax );
		cmd( "$activeplot.c.yscale itemconf medy -text %.*g", p_digits, fabs( ymed ) < zero_lim ? 0 : ymed );
		cmd( "$activeplot.c.yscale itemconf ymin -text %.*g", p_digits, fabs( ymin ) < zero_lim ? 0 : ymin );
	}
		
	if ( t == 1 )
	{
		old_val[ cur_plt ] = v->val[ 0 ];
		++cur_plt;
		return;
	}

	cmd( "set x1 [ expr floor( $cvhmarginR + %d * $plot_step ) ]", t );
	cmd( "set x2 [ expr floor( $cvhmarginR + ( %d - 1 ) * $plot_step ) ]", t );
	cmd( "set y1 [ expr floor( $sclvmarginR + ( $vsizeR - ( ( %lf - %lf ) / ( %lf - %lf ) ) * $vsizeR ) ) ]", v->val[ 0 ], ymin, ymax, ymin );
	cmd( "set y2 [ expr floor( $sclvmarginR + ( $vsizeR - ( ( %lf - %lf ) / ( %lf - %lf ) ) * $vsizeR ) ) ]", old_val[ cur_plt ], ymin, ymax, ymin );

	cmd( "$activeplot.c.c.cn create line $x2 $y2 $x1 $y1 -tag punto -fill $c%d", cur_plt );
	
	old_val[ cur_plt ] = v->val[ 0 ];
	++cur_plt;
}


/**************************************
RESET_PLOT
**************************************/
void reset_plot( int run )
{
	// allow for run-time plot window destruction
	cmd( "if [ winfo exists .plt%d ] { wm protocol .plt%d WM_DELETE_WINDOW \"\"; .plt%d.fond.go conf -state disabled; .plt%d.fond.shift conf -state disabled }", 
		 run, run, run, run );
}
