/*************************************************************

	LSD 7.1 - December 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/*************************************************************
DRAW.CPP
Draws the graphical representation of the model. It is activated by
INTERF.CPP only in case a model is loaded.

The main functions contained in this file are:

- void show_graph( object *t )
initialize the canvas and calls show_obj for the root

- void draw_obj( object *top, object *sel, int level, int center, int from )
recursive function that according to the level of the object type sets the
distances among the objects. Rather rigid, but it should work nicely
for most of the model structures. It assigns all the labels (above and below the
symbols) and the writing bound to the mouse.

- void put_node( int x1, int y1, int x2, int y2, char *str, bool sel )
Draw the circle.

- void put_line( int x1, int y1, int x2, int y2 )
draw the line

- void put_text( char *str, char *num, int x, int y, char *str2 );
Draw the different texts and sets the bindings
*************************************************************/

#include "decl.h"

float hpan0lin = 0.276;			// Linux
float hpan0mac = 0.151;			// Mac
float hpan0win = 0.276;			// initial horizontal scroll % - Windows
float range_fact = 0.3;			// horizontal width exponential factor
int hcvsz = 1280;				// horizontal canvas size
int vcvsz = 800;				// vertical canvas size
int h0 = 170;					// initial horizontal position
int v0 = 10;					// initial vertical position
int range_init = 190;			// horizontal initial width (4 root sons)
int range_incr = 20;			// horizontal width increase step
int step_level = 20;			// vertical step
int n_size = 4;					// node size (diameter)
int v_margin = 4;				// labels margins to nodes

float level_factor[ MAX_LEVEL ];
int range;
int range_type;


/****************************************************
SHOW_GRAPH
****************************************************/
void show_graph( object *t )
{
	object *top;

	if ( ! struct_loaded || ! strWindowOn )		// model structure window is deactivated?
	{
		cmd( "destroytop .str" );
		return;
	}

	cmd( "set n_color white" );		// node color
	cmd( "set n_color_sel blue" );	// selected node color
	cmd( "set t_color red" );		// node name color
	cmd( "set l_color gray" );		// line color
	
	cmd( "set g .str" );
	for ( top = t; top->up != NULL; top = top->up );

	cmd( "set strExist [ winfo exists $g ]" );
	if ( ! strcmp( Tcl_GetVar( inter, "strExist", 0 ), "0" ) )		// build window only if needed
	{
		cmd( "newtop $g \"\" { set strWindowOn 0; set choice 70 } \"\"" );
		cmd( "wm transient $g ." );
		cmd( "sizetop $g" );

		cmd( "frame $g.f" );
		cmd( "scrollbar $g.f.vs -command \"$g.f.c yview\"" );
		cmd( "scrollbar $g.f.hs -orient horiz -command \"$g.f.c xview\"" );
		cmd( "canvas $g.f.c -width %d -height %d -yscrollcommand \"$g.f.vs set\" -xscrollcommand \"$g.f.hs set\" -scrollregion \"0 0 %d %d\"", hcvsz, vcvsz, hcvsz, vcvsz );

		cmd( "pack $g.f.vs -side right -fill y" );
		cmd( "pack $g.f.hs -side bottom -fill x" );
		cmd( "pack $g.f.c -expand yes -fill both" );
		cmd( "pack $g.f -expand yes -fill both" );
		cmd( "set hpan0win %f; set hpan0lin %f; set hpan0mac %f", hpan0win, hpan0lin, hpan0mac );
		cmd( "mouse_wheel $g.f.c" );
		cmd( "if [ string equal $tcl_platform(platform) windows ] { \
					$g.f.c xview moveto $hpan0win \
				} { \
					if [ string equal $tcl_platform(os) Darwin ] { \
						$g.f.c xview moveto $hpan0mac \
					} { \
						$g.f.c xview moveto $hpan0lin \
					} \
				}" );

		draw_obj( top, t, v0, h0, 0 );

		cmd( "bind $g.f.c <Button-1> { if [ info exists res_g ] { set choice_g 24 } }" );
		cmd( "bind $g.f.c <Button-2> { if [ info exists res_g ] { set res $res_g; set vname $res; set useCurrObj no; tk_popup $g.f.c.v %%X %%Y } }" );
		cmd( "bind $g.f.c <Button-3> { if [ info exists res_g ] { set res $res_g; set vname $res; set useCurrObj no; tk_popup $g.f.c.v %%X %%Y } }" );

		cmd( "menu $g.f.c.v -tearoff 0" );
		cmd( "$g.f.c.v add command -label \"Make Current\" -command { set choice 4 }" );
		cmd( "$g.f.c.v add command -label \"Insert Parent\" -command { set choice 32 }" );
		cmd( "$g.f.c.v add separator" );
		cmd( "$g.f.c.v add command -label Change -command { set choice 6 }" );
		cmd( "$g.f.c.v add command -label Rename -command { set choice 83 }" );
		cmd( "$g.f.c.v add command -label Number -command { set choice 33 }" );
		cmd( "$g.f.c.v add command -label Delete -command { set choice 74 }" );
		cmd( "$g.f.c.v add separator" );
		cmd( "$g.f.c.v add cascade -label Add -menu $g.f.c.v.a" );
		cmd( "$g.f.c.v add separator" );
		cmd( "$g.f.c.v add command -label \"Initial Values\" -command { set choice 21 }" );
		cmd( "$g.f.c.v add command -label \"Browse Data\" -command { set choice 34 }" );
		cmd( "menu $g.f.c.v.a -tearoff 0" );
		cmd( "$g.f.c.v.a add command -label Variable -command { set choice 2; set param 0 }" );
		cmd( "$g.f.c.v.a add command -label Parameter -command { set choice 2; set param 1 }" );
		cmd( "$g.f.c.v.a add command -label Function -command { set choice 2; set param 2 }" );
		cmd( "$g.f.c.v.a add command -label Object -command { set choice 3 }" );

		cmd( "showtop $g current yes yes no 0 0 b" );

		set_shortcuts( "$g", "graphrep.html" );
	}
	else	// or just update canvas
	{
		cmd( "$g.f.c delete all" );
		draw_obj( top, t, v0, h0, 0 );
	}

	cmd( "wm title $g \"%s%s - LSD Model Structure\"", unsaved_change() ? "*" : " ", simul_name );
}


/****************************************************
DRAW_OBJ
****************************************************/
void draw_obj( object *t, object *sel, int level, int center, int from )
{
	bool sp_upd;
	char str[ MAX_LINE_SIZE ], ch[ TCL_BUFF_STR ], ch1[ MAX_ELEM_LENGTH ];
	int i, j, k, step_type, begin, count;
	object *cur;
	variable *cv;
	bridge *cb;

	// element list to appear on the left of the window
	cmd( "set list_%s \"\"", t->label );

	if ( t->v == NULL )
		cmd( "append list_%s \"(no elements)\"", t->label );

	// floating variable list
	for ( cv = t->v; cv != NULL; cv = cv->next )
	{
		sprintf( ch,"append list_%s \"%s", t->label, cv->label );
		
		// special updating scheme?
		if ( cv->param == 0 && ( cv->delay > 0 || cv->delay_range > 0 || cv->period > 1 || cv->period_range > 0 ) )
			sp_upd = true;
		else
			sp_upd = false;

		// set flags string
		cmd( "set varFlags \"%s%s%s%s%s\"", ( cv->save || cv->savei ) ? "+" : "", cv->plot ? "*" : "", cv->debug == 'd' ? "!" : "", cv->parallel ? "&" : "", sp_upd ? "§" : "" );
				
		if ( cv->param == 1 )
			sprintf( str," (P$varFlags)\n\"" );
		else
		{
			if ( cv->num_lag == 0 )
				sprintf( str, " (%s$varFlags)\n\"", ( cv->param == 0 ) ? "V" : "F" );
			else
				sprintf( str, " (%s_%d$varFlags)\n\"", ( cv->param == 0 ) ? "V" : "F", cv->num_lag );
		}
		
		strcat( ch, str );
		cmd( ch );
	}

	// draw node only if it is not the root
	if ( t->up != NULL )
	{ 	// compute number of groups of this type
		sprintf( ch, "%s", t->label );
		strcpy( ch1, "" );
		
		for ( cur = t; cur != NULL ; cur = cur->hyper_next( ) )
		{
			if ( strlen( ch1 ) >= 9 )
			{
				strcat( ch1, "\u2026" );
				break;
			}
			
			skip_next_obj( cur, &count );
			sprintf( str, "%s%d", strlen( ch1 ) ? " " : "", count );
			strcat( ch1, str );
			
			for ( ; cur->next != NULL; cur = cur->next ); // reaches the last object of this group
		}
		
		if ( t->up->up != NULL )
			put_line( from, level - step_level + v_margin + n_size / 2, center, level + n_size / 2 );
		put_node( center - n_size / 2, level + v_margin - n_size / 2, center + n_size / 2, level + v_margin + n_size / 2, t->label, t == sel ? true : false );
		put_text( ch, ch1, center, level, t->label );
	}

	// count the number of son object types
	for ( i = 0, cb = t->b; cb != NULL; ++i, cb = cb->next );
	
	// find current tree depth
	for ( j = 0, cur = t; cur->up != NULL; ++j, cur = cur->up );
	
	// limit the maximum depth of plot
	if ( j >= MAX_LEVEL )
		return;
	
	// root? adjust tree begin
	if ( j == 0 )		
	{
		level -= step_level;
		
		// set the default horizontal scale factor per level
		for ( k = 0; k < MAX_LEVEL; ++ k )
			level_factor[ k ] = 1.0;

		// pick hand set scale factors
		switch ( i )
		{
			case 0:
			case 1:
				level_factor[ 0 ] = 0.4;
				level_factor[ 1 ] = 0.1;
				level_factor[ 2 ] = 0.5;
				level_factor[ 3 ] = 1.0;
				break;
			case 2:
				level_factor[ 0 ] = 0.5;
				level_factor[ 1 ] = 0.8;
				level_factor[ 2 ] = 0.6;
				level_factor[ 3 ] = 0.5;
				break;
			case 3:
				level_factor[ 0 ] = 0.6;
				level_factor[ 1 ] = 1.1;
				level_factor[ 2 ] = 0.4;
				level_factor[ 3 ] = 0.5;
				break;
			case 4:
				level_factor[ 0 ] = 0.7;
				level_factor[ 1 ] = 1.2;
				level_factor[ 2 ] = 0.3;
				level_factor[ 3 ] = 0.4;
				break;
			case 5:
				level_factor[ 0 ] = 0.7;
				level_factor[ 1 ] = 1.3;
				level_factor[ 2 ] = 0.2;
				level_factor[ 3 ] = 0.4;
				break;
			default:
				level_factor[ 0 ] = 0.9 + ( i - 5.0 ) / 10;
				level_factor[ 1 ] = 1.2 + ( i - 5.0 ) / 20;
				level_factor[ 2 ] = 0.3 + ( i - 5.0 ) / 30;
				level_factor[ 3 ] = 0.5 + ( i - 5.0 ) / 40;
		}
		
		range_type = ( int ) floor( level_factor[ 0 ] * range_init );
	}
	else
	{
		// reduce object type width at each level
		range_type = ( int ) floor( fabs( level_factor[ j ] * range_init / pow( 2, j + range_fact ) - pow( range_init * 2 / 3, 1 / j ) + 1 ) );
	}
	
	if ( i <= 1 )					// single object type son?
	{
		begin = center;				// adjust to print below parent
		step_type = 0;
	}
	else
	{
		begin = center - range_type / 2;
		step_type = range_type / ( i - 1 );
	}

	// draw sons
	for ( i = begin, cb = t->b; cb != NULL; i += step_type, cb = cb->next )
		if ( cb->head != NULL )
			draw_obj( cb->head, sel, level + step_level, i, center );
}


/****************************************************
PUT_NODE
****************************************************/
void put_node( int x1, int y1, int x2, int y2, char *str, bool sel )
{
	cmd( "$g.f.c create oval %d.m %d.m %d.m %d.m -fill $%s -outline $l_color -tags node -tags %s", x1, y1, x2, y2, sel ? "n_color_sel" : "n_color", str );
}


/****************************************************
PUT_LINE
****************************************************/
void put_line( int x1, int y1, int x2, int y2 )
{
    cmd( "$g.f.c create line %d.m %d.m %d.m %d.m -fill $l_color -tags node", x1, y1, x2, y2 );
}


/****************************************************
PUT_TEXT
****************************************************/
void put_text( char *str, char *n, int x, int y, char *str2 )
{
	cmd( "$g.f.c create text %d.m %d.m -text \"%s\" -fill $t_color -tags node -tags %s", x, y - 1, str, str2 );

	// text for node numerosity (handle single "1" differently to displace from line)
	if ( ! strcmp( n, "1" ) )
		cmd( "$g.f.c create text %.1lfm %d.m -text \"%s\" -tags node -tags %s", x - 0.5, y + 2 * v_margin + 1, n, str2 );
	else
		cmd( "$g.f.c create text %d.m %d.m -text \"%s\" -tags node -tags %s", x, y + 2 * v_margin + 1, n, str2 );

	cmd( "$g.f.c bind %s <Enter> { set res_g %s; if [winfo exists .list] { destroy .list }; toplevel .list; wm transient .list $g; wm title .list \"\"; wm protocol .list WM_DELETE_WINDOW { }; frame .list.h; label .list.h.l -text \"Object:\"; label .list.h.n -fg red -text \"%s\"; pack .list.h.l .list.h.n -side left -padx 2; label .list.l -text \"$list_%s\" -justify left; pack .list.h .list.l; align .list $g }", str2, str2, str2, str2 );

	cmd( "$g.f.c bind %s <Leave> { if [ info exists res_g ] { unset res_g }; destroy .list}", str2 );
}
