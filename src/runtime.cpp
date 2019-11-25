/*************************************************************

	LSD 7.2 - December 2019
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

int width = 500;				// runtime plot area dimensions
int height = 300;
int b_height = 48;
int s_width = 75;
int h_margin = 8;
int t_margin = 3;
int b_margin = 30;
int h_ticks = 5;
int v_ticks = 2;
int tick = 5;
int lab_lin = 5;				// labels per line
int lin_lab = 3;				// lines of label
int lin_height = 18;
int p_digits = 3;				// precision digits
char intval[ 100 ];				// string buffer
char **tp;						// labels of variables to plot in runtime
double ymax;
double ymed;
double ymin;
double *old_val;
double plot_step;
int shift = 20;					// new window shift
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
	
	tp = new char *[ i ];
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
			tp[ *i ] = new char[ strlen( msg ) + 1 ];
			strcpy( tp[ *i ], msg );
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
	int i, j, k, l;
	
	plot_step = ( max_step > width ) ? 1 : width / ( double ) max_step;
	cmd( "set scrollB 0" );

	cmd( "set activeplot .plt%d", id_sim );

	cmd( "destroytop $activeplot" );

	cmd( "newtop $activeplot \"\" { set_c_var done_in 5 } \"\"" );
	cmd( "wm transient $activeplot ." );
	cmd( "wm title $activeplot \"%s%s(%d) - LSD Run-time Plot\"", unsaved_change() ? "*" : " ", simul_name, id_sim  );
	
	cmd( "frame $activeplot.c" );
	
	// vertical scale values
	cmd( "canvas $activeplot.c.yscale -width %d -height %d", s_width, height + t_margin + b_margin );

	cmd( "$activeplot.c.yscale create text %d %d -anchor e -justify right -text \"\" -tag ymax", s_width, ( int ) max( t_margin, 10 ) );
	cmd( "$activeplot.c.yscale create text %d %d -anchor e -justify right -text \"\" -tag medy", s_width, t_margin + height / 2 );
	cmd( "$activeplot.c.yscale create text %d %d -anchor e -justify right -text \"\" -tag ymin", s_width, t_margin + height );
	
	cmd( "pack $activeplot.c.yscale -side left -anchor nw" );
	
	// main canvas
	cmd( "frame $activeplot.c.c  " );
	cmd( "set p $activeplot.c.c.cn" );
	cmd( "scrollbar $activeplot.c.c.hscroll -orient horiz -command \"$p xview\"" );
	cmd( "canvas $p -width %d -height %d -bg white -scrollregion {0 0 %d %d} -xscrollcommand \"$activeplot.c.c.hscroll set\" -xscrollincrement 1 -yscrollincrement 1", width + 2 * h_margin, height + t_margin + b_margin, max_step, height + t_margin + b_margin );
	cmd( "pack $activeplot.c.c.hscroll -side bottom -expand yes -fill x" );
	
	// horizontal grid lines
	for ( i = 0; i <= v_ticks; ++i )
	{
		cmd( "$p create line %d %d %d %d -fill grey60", h_margin - tick, t_margin + height * i / v_ticks, ( int )( h_margin + max_step * plot_step ), t_margin + height * i / v_ticks );
	}

	// vertical grid lines
	for ( i = 0, j = h_margin; j <= h_margin + max_step * plot_step; ++i, j += width / h_ticks )
	{
		k = height + t_margin + tick;
		l = ( plot_step > 1 ) ? max_step * i / h_ticks : j - h_margin;
		cmd( "$p create line %d %d %d %d -fill grey60", j, t_margin, j, k );
		cmd( "$p create text %d %d -text %d -anchor n", j, k + tick, l );
	}
	
	cmd( "pack $p -anchor nw" );
	cmd( "pack $activeplot.c.c -anchor nw" );
	cmd( "pack $activeplot.c -anchor nw" );
	cmd( "$p xview moveto 0" );
	cmd( "mouse_wheel $p" );
	
	// bottom part
	cmd( "canvas $activeplot.fond -width %d -height %d", s_width + width + 2 * h_margin, b_height );

	// controls
	cmd( "checkbutton $activeplot.fond.shift -text Scroll -variable scrollB -command { set_c_var done_in 8 }" );	
	cmd( "if { $tcl_platform(platform) == \"windows\" } { set goWid 7 } { set goWid 5 }" );
	cmd( "button $activeplot.fond.go -width $goWid -text Center -command { set halfCanvas %d; set_c_var done_in 7 }", width / 2 );

	cmd( "$activeplot.fond create window %d %d -window $activeplot.fond.shift", s_width / 2, b_height / 4 - 5 );
	cmd( "$activeplot.fond create window %d %d -window $activeplot.fond.go", s_width / 2, 3 * b_height / 4 );
	
	// labels
	for ( i = 0, j = 0, k = 0; i < ( num < lin_lab * lab_lin ? num : lin_lab * lab_lin ); ++i )
	{
		cmd( "$activeplot.fond create text %d %d -anchor nw -text %s -fill $c%d", s_width + t_margin + j * width / lab_lin, k * lin_height, tp[ i ], i  );
		if ( j < lab_lin - 1 )
			++j;
		else
		{
			++k;
			j = 0;
		}
	}
	cmd( "pack $activeplot.fond -expand yes -fill both -pady 7" );
	
	// calculate window shift position
	i = id_sim * shift;
	sprintf( intval,"%i",i );
	Tcl_SetVar( inter, "shift", intval, 0 );
	cmd( "set posXrt [ expr [ winfo x . ] + [ winfo width . ] + 2 * $bordsize + $hmargin + $corrX + $shift ]" );
	cmd( "set posYrt [ expr [ winfo y . ] + $corrY + $shift ]" );

	cmd( "showtop  $activeplot xy no no no $posXrt $posYrt" );
	
	if ( fast_mode > 0 )
	{
		cmd( "wm withdraw $activeplot" );
		cmd( "$activeplot.fond.go conf -state disabled" );
		cmd( "$activeplot.fond.shift conf -state disabled" );
	}

	cmd( "wm deiconify .log; raise .log; focus .log" );

	set_shortcuts_log( "$activeplot", "runtime.html" );
}


/**************************************
PLOT_RT
**************************************/
void plot_rt( variable *v )
{
	bool relabel = false;
	int x1, x2, y1, y2;
	double value, scale, zero_lim;
	
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
		
		cmd( "$activeplot.c.c.cn scale punto 0 %d 1 %lf", height, scale  < 0.01 ? 0.01 : scale  );
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

	x1 = ( int ) floor( h_margin + t * plot_step );
	x2 = ( int ) floor( h_margin + ( t - 1 ) * plot_step );
	y1 = ( int ) floor( t_margin + ( height - ( ( v->val[ 0 ] - ymin ) / ( ymax - ymin ) ) * height ) );
	y2 = ( int ) floor( t_margin + ( height - ( ( old_val[ cur_plt ] - ymin ) / ( ymax - ymin ) ) * height ) );
	old_val[ cur_plt ] = v->val[ 0 ];

	cmd( "$activeplot.c.c.cn create line %d %d %d %d -tag punto -fill $c%d", x2, y2, x1, y1, cur_plt );
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
