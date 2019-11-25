/*************************************************************

	LSD 7.2 - December 2019
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
	
	cmd( "set g .str" );
	for ( top = t; top->up != NULL; top = top->up );

	cmd( "set strExist [ winfo exists $g ]" );
	if ( ! strcmp( Tcl_GetVar( inter, "strExist", 0 ), "0" ) )		// build window only if needed
	{
		cmd( "newtop $g \"\" { set strWindowOn 0; set choice 70 } \"\"" );
		cmd( "wm transient $g ." );
		cmd( "sizetop $g" );
	}
	else
		cmd( "destroy $g.f" );										// or just recreate canvas

	cmd( "wm title $g \"%s%s - LSD Model Structure\"", unsaved_change() ? "*" : " ", simul_name );
	
	cmd( "frame $g.f" );
	cmd( "canvas $g.f.c -xscrollincrement 1" );
	cmd( "pack $g.f.c -expand yes -fill both" );
	cmd( "pack $g.f -expand yes -fill both" );

	cmd( "showtop $g current yes yes no 0 0 b" );
	
	draw_obj( top, t );
	
	cmd( "set hrsizeM [ winfo width $g ]" );
	cmd( "set vrsizeM [ winfo height $g ]" );
	cmd( "$g.f.c xview scroll [ expr - int ( $hrsizeM / 2 ) ] units" );
	
	draw_buttons( );
	
	cmd( "bind $g.f.c <Configure> { if { $hrsizeM != [ winfo width .str ] || $vrsizeM != [ winfo height .str ] } { set choice_g 70 } }" );
	cmd( "bind $g.f.c <Button-1> { if [ info exists res_g ] { destroy .list; set choice_g 24 } }" );
	cmd( "bind $g.f.c <Button-2> { if [ info exists res_g ] { set res $res_g; set vname $res; set useCurrObj no; tk_popup .str.f.c.v %%X %%Y } }" );
	cmd( "bind $g.f.c <Button-3> { if [ info exists res_g ] { set res $res_g; set vname $res; set useCurrObj no; tk_popup .str.f.c.v %%X %%Y } }" );

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

	set_shortcuts( "$g", "graphrep.html" );

	cmd( "update" );
}


/****************************************************
DRAW_BUTTONS
****************************************************/
void draw_buttons( void )
{
	cmd( "set n [ scan [ $g.f.c bbox all ] \"%%d %%d %%d %%d\" x1 y1 x2 y2 ]" );
	cmd( "set cx1 [ $g.f.c canvasx 0 ]" );
	cmd( "set cy1 [ $g.f.c canvasy 0 ]" );
	cmd( "set cx2 [ $g.f.c canvasx [ winfo width $g ] ]" );
	cmd( "set cy2 [ $g.f.c canvasy [ winfo height $g ] ]" );
	
	cmd( "if { $n == 4 } { \
			set hratioM [ expr ( $cx2 - $cx1 ) / ( $x2 - $x1 + 2 * $borderM ) ]; \
			set vratioM [ expr ( $cy2 - $cy1 ) / ( $y2 - $y1 + 2 * $borderM ) ] \
		} { \
			set hratioM 1; \
			set vratioM 1 \
		}" );
	
	cmd( "button $g.f.c.hplus -text \"\u25B6\" -width $bsizeM -height 1 -command { \
			set hfactM [ round_N [ expr $hfactM + $rstepM ] 2 ]; \
			set choice_g 70 \
		}" );
	cmd( "button $g.f.c.hminus -text \"\u25C0\" -width $bsizeM -height 1 -command { \
			set hfactM [ round_N [ expr max( [ expr $hfactM - $rstepM ], $hfactMmin ) ] 2 ]; \
			set choice_g 70 \
		}" );
	cmd( "button $g.f.c.vplus -text \"\u25BC\" -width $bsizeM -height 1 -command { \
			set vfactM [ round_N [ expr $vfactM + $rstepM ] 2 ]; \
			set choice_g 70 \
		}" );
	cmd( "button $g.f.c.vminus -text \"\u25B2\" -width $bsizeM -height 1 -command { \
			set vfactM [ round_N [ expr max( [ expr $vfactM - $rstepM ], $vfactMmin ) ] 2 ]; \
			set choice_g 70 \
		}" );
	cmd( "button $g.f.c.auto -text \"A\" -width $bsizeM -height 1 -command { \
			set hfactM [ round_N [ expr $hfactM * $hratioM ] 2 ]; \
			set vfactM [ round_N [ expr $vfactM * $vratioM ] 2 ]; \
			set choice_g 70 \
		}" );
		
	cmd( "set colM [ expr $cx2 - $borderM ]" );
	cmd( "set rowM [ expr $cy2 - $borderM ]" );
	
	cmd( "$g.f.c create window $colM $rowM -window $g.f.c.auto" );
	cmd( "$g.f.c create window [ expr $colM - $bhstepM ] $rowM -window $g.f.c.hplus" );
	cmd( "$g.f.c create window [ expr $colM - 2 * $bhstepM ] $rowM -window $g.f.c.hminus" );
	cmd( "$g.f.c create window $colM [ expr $rowM - $bvstepM ] -window $g.f.c.vplus" );
	cmd( "$g.f.c create window $colM [ expr $rowM - 2 * $bvstepM ] -window $g.f.c.vminus" );
}


/****************************************************
DRAW_OBJ
****************************************************/
void draw_obj( object *t, object *sel, int level, int center, int from, bool zeroinst )
{
	bool sp_upd, fit_wid;
	char str[ MAX_LINE_SIZE ], ch[ TCL_BUFF_STR ], ch1[ MAX_ELEM_LENGTH ];
	double h_fact, v_fact, range_fact;
	int h, i, j, k, step_level, step_type, begin, count, max_wid, range_init, range_incr;
	object *cur;
	variable *cv;
	bridge *cb;

	get_double( "hfactM", &h_fact );
	get_double( "vfactM", &v_fact );
	get_double( "rfactM", &range_fact );
	get_int( "rinitM", &range_init );
	get_int( "rincrM", &range_incr );
	get_int( "vstepM", &step_level );
	step_level = round( step_level * v_fact );
	
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
		cmd( "set varFlags \"%s%s%s%s%s\"", ( cv->save || cv->savei ) ? "+" : "", cv->plot ? "*" : "", cv->debug == 'd' ? "!" : "", cv->parallel ? "&" : "", sp_upd ? "\u00A7" : "" );
				
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

	// find current tree depth
	for ( j = 0, cur = t; cur->up != NULL; ++j, cur = cur->up );
	
	// draw node only if it is not the root
	if ( t->up != NULL )
	{
		sprintf( ch, "%s", t->label );
		strcpy( ch1, "" );
		
		// count number of brothers and define maximum width for number string
		for ( k = 0, cb = t->up->b; cb != NULL; ++k, cb = cb->next );
		
		switch ( j )
		{
			case 1:								// first line objects
				max_wid = 15 * h_fact;
				break;
			case 2:								// second line objects
				max_wid = min( 15 * h_fact, 80 * h_fact / k );
				break;
			default:							// all other lines
				max_wid = min( 7 * h_fact, 10 * h_fact / k );
		}
		
	
		// format number string
		if ( zeroinst )
		{
			strcpy( ch1, "0" );
			
			// if parent is multi-instanced, add ellipsis to the zero
			if ( t->up->up != NULL )
			{
				// must search out of the blueprint, where we are now
				// may get the wrong parent if the parent is replicated somewhere
				cur = root->search( t->up->up->label );
				if ( cur != NULL )
				{
					cb = cur->search_bridge( t->up->label );
					for ( k = 0, cur = cb->head; cur != NULL; ++k, cur = cur->next );
				
					if ( k > 1 )				// handle multi-instanced parents
						strcat( ch1, "\u2026" );
				}
			}
		}
		else									// compute number of groups of this type
		{
			fit_wid = true;
			for ( h = 0, cur = t; cur != NULL ; ++h, cur = cur->hyper_next( ) )
			{
				if ( strlen( ch1 ) >= ( unsigned ) max_wid )
				{
					strcat( ch1, "\u2026" );
					fit_wid = false;
					break;
				}
				
				skip_next_obj( cur, &count );
				sprintf( str, "%s%d", strlen( ch1 ) > 0 ? " " : "", count );
				strcat( ch1, str );
				
				for ( ; cur->next != NULL; cur = cur->next ); // reaches the last object of this group
			}
			
			// count number of instances of parent and check for zero instances
			if ( fit_wid && t->up->up != NULL )	// first level cannot have multiple zero instances
			{
				cb = t->up->up->search_bridge( t->up->label );
				for ( k = 0, cur = cb->head; cur != NULL; ++k, cur = cur->next );
			
				if ( h < k )					// found zero instanced object?
					strcat( ch1, "\u2026" );
			}
		}
		
		if ( t->up->up != NULL )
			put_line( from, level, center );
		
		put_node( center, level, t->label, t == sel ? true : false );
		put_text( ch, ch1, center, level, t->label );
	}
	else
	{
		cmd( "$g.f.c delete all" );
		get_int( "borderM", &level );
		center = 0;
	}
	
	// limit the maximum depth of plot
	if ( j >= MAX_LEVEL )
		return;
	
	// count the number of son object types
	for ( i = 0, cb = t->b; cb != NULL; ++i, cb = cb->next );
	
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
		
		range_type = round( level_factor[ 0 ] * range_init * h_fact );
	}
	else
	{
		// reduce object type width at each level
		range_type = round( fabs( level_factor[ j ] * range_init / pow( 2, j + range_fact ) - pow( range_init * 2 / 3, 1 / j ) + 1 ) * h_fact );
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
			draw_obj( cb->head, sel, level + step_level, i, center, zeroinst );
		else
		{	// try to draw zero instance objects
			cur = blueprint->search( cb->blabel );
			if ( cur != NULL )
				draw_obj( cur, sel, level + step_level, i, center, true );
		}
}


/****************************************************
PUT_NODE
****************************************************/
void put_node( int x, int y, char *str, bool sel )
{
	cmd( "$g.f.c create oval [ expr %d - $nsizeM / 2 ] [ expr %d + $vmarginM - $nsizeM / 2 ] [ expr %d + $nsizeM / 2 ] [ expr %d + $vmarginM + $nsizeM / 2 ]  -fill $%s -outline $lcolorM -tags %s", x, y, x, y, sel ? "ncolorMsel" : "ncolorM", str );
}


/****************************************************
PUT_LINE
****************************************************/
void put_line( int x1, int y1, int x2 )
{
    cmd( "$g.f.c create line %d [ expr round ( %d - $vstepM * $vfactM + $vmarginM + $nsizeM / 2 ) ] %d [ expr round ( %d + $nsizeM / 2 ) ] -fill $lcolorM", x1, y1, x2, y1 );
}


/****************************************************
PUT_TEXT
****************************************************/
void put_text( char *str, char *n, int x, int y, char *str2 )
{
	cmd( "$g.f.c create text %d %d -text \"%s\" -fill $tcolorM -tags %s", x, y - 1, str, str2 );

	// text for node numerosity (handle single "1" differently to displace from line)
	if ( ! strcmp( n, "1" ) )
		cmd( "$g.f.c create text [ expr %d - 2 ] [ expr %d + 2 * $vmarginM + 1 ] -text \"%s\" -tags %s", x, y, n, str2 );
	else
		cmd( "$g.f.c create text %d [ expr %d + 2 * $vmarginM + 1 ] -text \"%s\" -tags %s", x, y, n, str2 );

	cmd( "$g.f.c bind %s <Enter> { set res_g %s; if [ winfo exists .list ] { destroy .list }; toplevel .list; wm transient .list $g; wm title .list \"\"; wm protocol .list WM_DELETE_WINDOW { }; frame .list.h; label .list.h.l -text \"Object:\"; label .list.h.n -fg red -text \"%s\"; pack .list.h.l .list.h.n -side left -padx 2; label .list.l -text \"$list_%s\" -justify left; pack .list.h .list.l; align .list $g }", str2, str2, str2, str2 );

	cmd( "$g.f.c bind %s <Leave> { if [ info exists res_g ] { unset res_g }; destroy .list}", str2 );
}
