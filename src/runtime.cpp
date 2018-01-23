/*************************************************************

	LSD 7.0 - January 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/****************************************************
RUN_TIME.CPP contains initialization and management of run-time plotting


The functions contained here are:

- void prepare_plot(object *r, int id_sim)
Checks is there are LSD variables to plot. If not, returns immediately. Otherwise
initiliaze the run time globale variables. Namely, the vector of the labels for
the variables of plot. The plot window is initialized according to the id_sim name

- void count(object *r, int *i);
Recursive function that increments i of one for any variable to plot.

- void assign(object *r, int *i, char *lab);
Create a list of Variables to plot and create the list of labels (adding
the indexes if necessary) to be used in the plot.


- void init_plot(int i, int id_sim);
create the canvas for the plot, the lines, button, labels, etc.

- void plot_rt(variable *v)
the function used run time to plot the value of variable v

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

****************************************************/

#include "decl.h"

// better adjusts position for X11
char intval[ 100 ];				// string buffer
char **tp;						// labels of variables to plot in runtime
double ymin;
double ymax;
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
	variable *a;
	object *c;
	bridge *cb;

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
	variable *a;
	object *c, *c1;
	char cur_lab[ MAX_ELEM_LENGTH ];
	int j;
	bridge *cb;

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
	int i, j, k;
	
	if ( max_step > 500 )
		plot_step = 1;
	else
		plot_step = ( 500 / max_step );

	cmd( "set activeplot .plt%d", id_sim );

	cmd( "destroytop $activeplot" );

	cmd( "newtop $activeplot \"\" { set_c_var done_in 5 } \"\"" );
	cmd( "wm transient $activeplot ." );
	cmd( "wm title $activeplot \"%s%s(%d) - LSD Run-time Plot\"", unsaved_change() ? "*" : " ", simul_name, id_sim  );

	cmd( "bind $activeplot <s> { set_c_var done_in 1 }; bind $activeplot <S> { set_c_var done_in 1 }" );
	cmd( "bind $activeplot <f> { set_c_var done_in 2 }; bind $activeplot <F> { set_c_var done_in 2 }" );
	cmd( "bind $activeplot <d> { set_c_var done_in 3 }; bind $activeplot <D> { set_c_var done_in 3 }" );
	cmd( "bind $activeplot <o> { set_c_var done_in 4 }; bind $activeplot <O> { set_c_var done_in 4 }" );

	cmd( "frame $activeplot.c" );
	cmd( "frame $activeplot.c.c  " );
	cmd( "set p $activeplot.c.c.cn" );
	cmd( "scrollbar $activeplot.c.c.hscroll -orient horiz -command \"$p xview\"" );
	cmd( "canvas $p -width 500 -height 330 -relief sunken -background white -bd 2 -scrollregion {0 0 %d 340} -xscrollcommand \"$activeplot.c.c.hscroll set\" -yscrollincrement 1", max_step );
	cmd( "pack $activeplot.c.c.hscroll -side bottom -expand yes -fill x" );
	cmd( "$p create line 0 302 %d 302", (int)((double)max_step*plot_step) );
	cmd( "$p create line 0 151 %d 151 -fill grey60", (int)((double) max_step*plot_step) );
	cmd( "pack $p -expand yes -fill both -anchor w" );
	cmd( "$p xview moveto 0" );
	cmd( "canvas $activeplot.c.yscale -width 80 -height 370" );
	cmd( "pack $activeplot.c.yscale -anchor w -side left -anchor n -expand no" );
	cmd( "pack $activeplot.c.c -anchor w -side left -fill both -expand yes" );
	cmd( "pack $activeplot.c -anchor w -expand yes -fill both" );
	cmd( "mouse_wheel $p" );

	for ( i = 0; i < ( int ) ( ( double ) max_step * plot_step ); i += 100 )
	{
		cmd( "$p create line %d 0 %d 310 -fill grey60", i, i );
		cmd( "$p create text %d 310 -font {{MS Times New Roman} 10} -text %d -anchor nw", i, i/(int)plot_step );
	}
	
	cmd( "$activeplot.c.yscale create line 80 4 80 303" );
	cmd( "$activeplot.c.yscale create line 75 4 80 4" );
	cmd( "$activeplot.c.yscale create line 75 150 80 150" );
	cmd( "$activeplot.c.yscale create line 75 302 80 302" );
	cmd( "$activeplot.c.yscale create text 2 2 -font {{MS Times New Roman} 10} -anchor nw -text \"\" -tag ymax" );
	cmd( "$activeplot.c.yscale create text 2 150 -font {{MS Times New Roman} 10} -anchor w -text \"\" -tag medy" );
	cmd( "$activeplot.c.yscale create text 2 305 -font {{MS Times New Roman} 10} -anchor sw -text \"\" -tag ymin" );
	cmd( "set scrollB 0" );
	cmd( "checkbutton $activeplot.c.yscale.shift -text Scroll -variable scrollB -command { set_c_var done_in 8 }" );
	cmd( "button $activeplot.c.yscale.go -width 7 -text Center -command {set_c_var done_in 7}" );
	cmd( "$activeplot.c.c.cn conf -xscrollincrement 1" );

	cmd( "$activeplot.c.yscale create window 42 313 -window $activeplot.c.yscale.shift -anchor n" );
	cmd( "$activeplot.c.yscale create window 42 343 -window $activeplot.c.yscale.go -anchor n" );
	cmd( "canvas $activeplot.fond -height 50" );
	cmd( "pack $activeplot.fond -expand yes -fill both" );
	for ( i = 0, j = 0, k = 0; i < ( num < 18 ? num : 18 ); ++i )
	{
		cmd( "$activeplot.fond create text %d %d -font {{MS Times New Roman} 10} -anchor nw -text %s -fill $c%d", 5 + j * 100, k * 16, tp[i], i  );
		if ( j < 5 )
			++j;
		else
		{
			++k;
			j = 0;
		}
	}
	 
	i = id_sim * shift;				// calculate window shift position
	sprintf( intval,"%i",i );
	Tcl_SetVar( inter, "shift", intval, 0 );
	cmd( "set posXrt [ expr [ winfo x . ] + [ winfo width . ] + 2 * $bordsize + $hmargin + $corrX + $shift ]" );
	cmd( "set posYrt [ expr [ winfo y . ] + $corrY + $shift ]" );

	cmd( "showtop  $activeplot xy no no no $posXrt $posYrt" );
	if ( fast_mode > 0 )
	{
		cmd( "wm withdraw $activeplot" );
		cmd( "$activeplot.c.yscale.go conf -state disabled" );
		cmd( "$activeplot.c.yscale.shift conf -state disabled" );
	}

	cmd( "wm deiconify .log; raise .log; focus .log" );

	set_shortcuts_log( "$activeplot", "runtime.html" );
}


/**************************************
PLOT_RT
**************************************/
void plot_rt( variable *v )
{
	int y1, y2;
	double dy, step, value;
	
	// limit the number of run-time plot variables
	if ( cur_plt > 100 )
		return;

	if ( ymax == ymin ) 		// very initial setting
	{ 
		if ( v->val[ 0 ] > 0 )
		{
			ymax = v->val[ 0 ] * 1.001;
			ymin = v->val[ 0 ];
		}
		else
		{
			ymax = v->val[ 0 ] * 0.009;
			ymin = v->val[ 0 ];
		}
		
		if ( ymax == ymin )		// it must be zero...
		{
			ymax = 0.0001;
//     		ymin = -0.001; 		// most of the times, if initial vars are zero means that they are supposed to grow
		}

		cmd( "$activeplot.c.yscale itemconf ymax -text %.4g", ymax );
		cmd( "$activeplot.c.yscale itemconf ymin -text %.4g", ymin );
		cmd( "$activeplot.c.yscale itemconf medy -text %.4g", ( ymax - ymin ) / 2 + ymin );
	}
	
	if ( v->val[ 0 ] >= ymax )
	{
		if ( v->val[ 0 ] >= 0 )
			step = 1.1;
		else
			step = 0.9;
	  
		double scale = ( ymax - ymin ) / ( v->val[ 0 ] * step - ymin );
		cmd( "$activeplot.c.c.cn scale punto 0 300 1 %lf", scale  < 0.01 ? 0.01 : scale  );
		ymax=v->val[0]*step;
		cmd( "$activeplot.c.yscale itemconf ymax -text %.4g", ymax );
		cmd( "$activeplot.c.yscale itemconf medy -text %.4g", ( ymax - ymin ) / 2 + ymin );
	}

	if ( v->val[ 0 ] <= ymin )
	{
		if ( v->val[ 0 ] > 0 )
			step = 0.9;
		else
			step = 1.1;
		value = min( v->val[ 0 ] * step, ymin - ( ymax - ymin ) / 300 );

		double scale = ( ymax - ymin ) / ( ymax - value );
		cmd( "$activeplot.c.c.cn scale punto 0 0 1 %lf", scale < 0.01 ? 0.01 : scale  );
		ymin = value;
		cmd( "$activeplot.c.yscale itemconf ymin -text %.4g", ymin );
		cmd( "$activeplot.c.yscale itemconf medy -text %.4g", ( ymax - ymin ) / 2 + ymin );
	}

	if ( t == 1 )
	{
		old_val[ cur_plt ] = v->val[ 0 ];
		++cur_plt;
		return;
	}

	dy = ( 300 - ( ( v->val[ 0 ] - ymin ) / ( ymax - ymin ) ) * 300 );
	y1 = ( int ) dy;
	dy = ( 300 -( ( old_val[ cur_plt ] - ymin ) / ( ymax - ymin ) ) * 300 );
	y2 = ( int ) dy;
	old_val[ cur_plt ] = v->val[ 0 ];

	cmd( "$activeplot.c.c.cn create line %d %d %d %d -tag punto -fill $c%d", ( t - 1 ) * ( int ) plot_step, y2, t * ( int ) plot_step, y1, cur_plt );
	++cur_plt;
}
