/*************************************************************

	LSD 7.3 - December 2020
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
	See Readme.txt for copyright information of
	third parties' code used in LSD
	
 *************************************************************/

/*************************************************************
EDIT_DAT.CPP
Called by INTERF.CPP shows all the lagged variables and parameters
to be initialized for one object. Prepares the spread-sheet window
and the bindings. 

It can exits in three ways:
1) return to the calling function (either Browser or set object nunmber)
2) setall, calling the routine to set all values at once
3) move to set object number

This interface shows a maximum of 100 columns, though it allows to set all
the initial values of the model by using the setall options. In case
you need to individually observe and edit the data of objects not shown
by this function, use the Data Browse option.

The main functions contained in this file are:

- void edit_data( object *root, int *choice, char *obj_name )
Initialize the window, calls search_title and link_data, then wait for a
message from user.

- void search_title( object *root, char *tag, int *i, char *lab, int *incr )
It is a recursive routine. Scan the model structure looking for the object
of type as root and prepare the relative tag for any object. The tag is then
used by set_title to be printed as columns headers

- void set_title( object *c, char *lab, char *tag, int *incr );
prints the column headers

- void link_data( object *root, char *lab );
prints the line headers and create the cells, each linked to one variable value
of the model

- void clean_cell( object *root, char *tag, char *lab );
called before exiting, removes all the links between tcl variables and model
values
*************************************************************/

#include "decl.h"

// flags to avoid recursive usage (confusing and tk windows are not ready)
bool colOvflw;					// indicate columns overflow (>MAX_COLS)
bool iniShowOnce = false;		// prevent repeating warning on # of columns
bool in_edit_data = false;


/****************************************************
EDIT_DATA
****************************************************/
void edit_data( object *root, int *choice, char *obj_name )
{
	char *l , ch[ 2 * MAX_ELEM_LENGTH ], ch1[ MAX_ELEM_LENGTH ];
	int i, counter, lag;
	object *first;


	Tcl_LinkVar( inter, "lag", ( char * ) &lag, TCL_LINK_INT );

	cmd( "set cwidth 11" );
	cmd( "if { ! [ winfo exists .ini ] } { \
			newtop .ini; \
			set newIni 1 \
		} { \
			set newIni 0 \
		}" );

	cmd( "set position 1.0" );
	in_edit_data = true;

	*choice = 0;
	while ( *choice == 0 )
	{
		first = root->search( obj_name );

		cmd( "ttk::frame .ini.t" );		// top frame to pack
		
		// create single top frame to grid, where the initial values spreadsheet can be built
		cmd( "set g .ini.t.grid" );
		cmd( "ttk::frame $g" );
		cmd( "grid $g" );
		cmd( "grid rowconfigure $g 0 -weight 1" );
		cmd( "grid columnconfigure $g 0 -weight 1" );
		cmd( "grid propagate $g 0" );	// allow frame resizing later, after cells are created
		cmd( "set lastIniSz { 0 0 }" );	// handle first configuration
		cmd( "set iniDone 0" );
		
		// adjust spreadsheet size when toplevel window resizes
		cmd( "bind .ini <Configure> { \
				if { ! [ info exists iniConfRun ] } { \
					set iniConfRun 1; \
					set iniSz [ list [ winfo width .ini ] [ winfo height .ini ] ]; \
					if { $iniSz != $lastIniSz } { \
						update; \
						set lastIniSz $iniSz; \
						set canBbox [ $g.can bbox all ]; \
						if { $iniDone } { \
							set maxWid [ lindex $iniSz 0 ]; \
							set maxHgt [ expr [ lindex $iniSz 1 ] - 110 ] \
						} { \
							set maxWid [ expr [ winfo screenwidth .ini ] - [ getx .ini topleftW ] - 2 * $bordsize - $hmargin ]; \
							set maxHgt [ expr [ winfo screenheight .ini ] - [ gety .ini topleftW ] - 2 * $bordsize - $vmargin - $tbarsize - 110 ] \
						}; \
						set desWid [ expr min( max( [ lindex $canBbox 2 ] - [ lindex $canBbox 0 ] + [ winfo width $g.ys ], [ lindex $iniSz 0 ] ), $maxWid ) ]; \
						set desHgt [ expr min( max( [ lindex $canBbox 3 ] - [ lindex $canBbox 1 ] + [ winfo height $g.xs ], 40 ), $maxHgt ) ]; \
						$g configure -width $desWid -height $desHgt; \
						$g.can configure -scrollregion $canBbox \
					}; \
					unset iniConfRun \
				} \
			}" );

		// canvas to hold the initial values spreadsheet so it can be scrollable
		cmd( "ttk::canvas $g.can -yscrollcommand { .ini.t.grid.ys set } -xscrollcommand { .ini.t.grid.xs set } -entry 0 -dark $darkTheme" );
		cmd( "ttk::scrollbar $g.ys -command { .ini.t.grid.can yview }" );
		cmd( "ttk::scrollbar $g.xs -command { .ini.t.grid.can xview } -orient horizontal" );
		cmd( "grid $g.can $g.ys -sticky nsew" );
		cmd( "grid $g.xs -sticky ew" );
		cmd( "mouse_wheel $g.can" );
		
		// single frame in canvas to hold all spreadsheet cells
		cmd( "set w $g.can.f" );
		cmd( "ttk::frame $w" );
		cmd( "$g.can create window 0 0 -window $w -anchor nw" );
		cmd( "mouse_wheel $w" );
		
		// title row
		strncpy( ch1, obj_name, MAX_ELEM_LENGTH - 1 );
		ch1[ MAX_ELEM_LENGTH - 1 ] = '\0';
		cmd( "ttk::label $w.tit_empty -style boldSmall.TLabel -text %s", ch1 );
		cmd( "grid $w.tit_empty -sticky w -padx { 2 5 }" );
		cmd( "bind $w.tit_empty <Button-1> { set choice 4 }" );
		cmd( "mouse_wheel $w.tit_empty" );
		
		if ( ! in_set_obj )				// show only if not already recursing
			cmd( "bind $w.tit_empty <Enter> { set msg \"Click to edit '%s'\" }", ch1 );
			
		cmd( "bind $w.tit_empty <Leave> { set msg \"\" }" );
		
		cmd( "ttk::label $w.tit_typ -style hl.TLabel -text (Obj)" );
		cmd( "grid $w.tit_typ -row 0 -column 1 -padx 1" );
		cmd( "mouse_wheel $w.tit_typ" );

		// explore the tree searching for each instance of such object and create:
		// - titles
		// - entry cells linked to the values
		
		strcpy( ch, "" );
		i = 0;
		counter = 1;
		colOvflw = false;
		search_title( root, ch, &i, obj_name, &counter );
		link_data( root, obj_name );
		
		cmd( "set line_counter %d", counter );

		cmd( "pack .ini.t -expand 1 -fill both" );
		
		cmd( "set msg \"\"" );
		cmd( "ttk::label .ini.msg -width 45 -textvariable msg" );
		cmd( "ttk::label .ini.err -text \"\"" );
		cmd( "pack .ini.msg .ini.err -padx 5 -pady 5" );

		cmd( "donehelp .ini b { set choice 1 } { LsdHelp menudata_init.html }" );

		cmd( "bind .ini <KeyPress-Escape> { set choice 1 }" );
		cmd( "bind .ini <F1> { LsdHelp menudata_init.html }" );

		// show overflow warning just once per configuration but always indicate
		if ( colOvflw )
		{
			cmd( ".ini.err conf -text \"OBJECTS NOT SHOWN! (> %d)\" -style hl.TLabel", MAX_COLS );
			if ( ! iniShowOnce )
			{
				cmd( "update" );
				cmd( "ttk::messageBox -parent . -type ok -title Warning -icon warning -message \"Too many objects to edit\" -detail \"LSD Initial Values editor can show only the first %d objects' values. Please use the 'Set All' button to define values for objects beyond those.\" ", MAX_COLS );
				iniShowOnce = true;
			}
		}

		// reset title and destroy command because may be coming from set_obj_number
		cmd( "wm title .ini \"%s%s - LSD Initial Values Editor\"", unsaved_change( ) ? "*" : " ", simul_name );
		cmd( "wm protocol .ini WM_DELETE_WINDOW { set choice 1 }" );
		cmd( "if { $newIni } { showtop .ini topleftW 1 1; set newIni 0 }" );
		cmd( "wm minsize .ini $hsizeImin $vsizeImin" );
		cmd( "pack propagate .ini 0" );
		cmd( "set iniDone 1" );

		noredraw:
		
		cmd( "update" );
		cmd( "if [ info exists lastEditPosX ] { $g.can xview moveto $lastEditPosX; unset lastEditPosX }" );
		cmd( "if [ info exists lastEditPosY ] { $g.can yview moveto $lastEditPosY; unset lastEditPosY }" );
		cmd( "if { [ info exists lastFocus ] && $lastFocus != \"\" && [ winfo exists $lastFocus ] } { focus $lastFocus; $lastFocus selection range 0 end; unset lastFocus }" );

		// editor main command loop
		while ( *choice == 0 )
		{
			try
			{
				Tcl_DoOneEvent( 0 );
			}
			catch ( bad_alloc& ) 	// raise memory problems
			{
				throw;
			}
			catch ( ... )				// ignore the rest
			{
				goto noredraw;
			}
		}   

		// handle both resizing event and block object # setting while editing initial values
		if ( *choice == 5 || ( *choice == 4 && in_set_obj ) )		// avoid recursion
		{
			*choice = 0;
			goto noredraw;
		}

		cmd( "set lastEditPosX [ lindex [ $g.can xview ] 0 ]" );
		cmd( "set lastEditPosY [ lindex [ $g.can yview ] 0 ]" );
			
		// clean up
		strcpy( ch, "" );
		i = 0;
		clean_cell( root, ch, obj_name );
		cmd( "destroy .ini.t .ini.b .ini.msg .ini.err" );


		if ( *choice == 2 )
		{	
			l = ( char * ) Tcl_GetVar( inter, "var-S-A", 0 );
			strcpy( ch, l );
			
			*choice = 2;		// set data editor window parent
			set_all( choice, first, ch, lag );
			
			
			cmd( "bind .ini <KeyPress-Return> {}" );
			*choice = 0;
		}
		else
			cmd( "unset -nocomplain lastEditPosX lastEditPosY lastFocus" );
		
		if ( *choice == 4 )
		{ 
			*choice = 0;
			set_obj_number( root, choice );
			*choice = 0;
		}
	}

	in_edit_data = false;

	Tcl_UnlinkVar( inter, "lag");
}


/****************************************************
SEARCH_TITLE
****************************************************/
void search_title( object *root, char *tag, int *i, char *lab, int *incr )
{
	char ch[ 2 * MAX_ELEM_LENGTH ];
	int multi, counter;
	bridge *cb;
	object *c, *cur;

	set_title( root, lab, tag, incr );

	for ( cb = root->b, counter = 1; cb != NULL; cb = cb->next, counter = 1 )
	{  
		if ( cb->head == NULL )
			continue;
		
		c = cb->head;
		*i = *i + 1;
		
		if ( c->next != NULL )
			multi=1;
		else
			multi = 0;
		
		for ( cur = c; cur != NULL; ++counter, cur = go_brother( cur ) )
		{
			if ( multi == 1 )
				if ( strlen( tag ) != 0 )
					sprintf( ch, "%s-%d", tag, counter );
				else
					sprintf( ch, "%d", counter );
			else
				sprintf( ch, "%s", tag );
	 
			if ( *incr <= MAX_COLS )
				search_title( cur, ch, i, lab, incr );

		}
	}
}


/****************************************************
SET_TITLE
****************************************************/
void set_title( object *c, char *lab, char *tag, int *incr )
{
	char ch1[ MAX_ELEM_LENGTH ], ch2[ MAX_ELEM_LENGTH ];

	if ( ! strcmp( c->label, lab ) )
	{
		ch1[ MAX_ELEM_LENGTH - 1 ] = '\0';
		strncpy( ch1, c->label, MAX_ELEM_LENGTH - 1 );
		
		if ( strlen( tag ) != 0 )
		{  
			strncpy( ch2, tag, MAX_ELEM_LENGTH - 1 );
			ch2[ MAX_ELEM_LENGTH - 1 ] = '\0';
		}
		else
			strcpy( ch2, "  " );

		cmd( "set titheader_%d \"%s\"", *incr , ch2 );

		cmd( "ttk::label $w.c%d_tit -text ${titheader_%d} -style boldSmall.TLabel", *incr ,*incr );
		cmd( "grid $w.c%d_tit -row 0 -column [ expr 2 + %d ] ", *incr, *incr );
		cmd( "mouse_wheel $w.c%d_tit", *incr );
		
		if ( strlen( tag ) == 0 )
			cmd( "set tag_%d \"\"", *incr );
		else
			cmd( "set tag_%d %s", *incr, tag );
		
		++( *incr );
	}
}


/****************************************************
CLEAN_CELL
****************************************************/
void clean_cell( object *root, char *tag, char *lab )
{
	char ch1[ 2 * MAX_ELEM_LENGTH ];
	int j, i;
	object *cur;
	variable *cv;
	
	cur = root->search( lab );
	
	for ( i = 1; i <= MAX_COLS && cur != NULL; cur = cur->hyper_next( lab ), ++i )
	{
		for ( cv = cur->v; cv != NULL; cv = cv->next )
		{
			if ( cv->param == 1 )
			{ 
				sprintf( ch1,"p%s_%d", cv->label, i );
				cmd( "set %s [ $w.c%d_v%sp get ]", ch1, i, cv->label );
				Tcl_UnlinkVar( inter, ch1 );
			}
			else
			{ 
				for ( j = 0; j < cv->num_lag; ++j )
				{
					sprintf( ch1,"v%s_%d_%d", cv->label, i, j );
					cmd( "set %s [ $w.c%d_v%s_%d get ]", ch1, i, cv->label, j );
					Tcl_UnlinkVar( inter, ch1 );
				}
			}
		}
	}	
}


/****************************************************
LINK_DATA
****************************************************/
void link_data( object *root, char *lab )
{
	int i, j, k;
	bool lastFocus = false;
	char previous[ MAX_ELEM_LENGTH + 20 ], ch1[ MAX_ELEM_LENGTH ];
	object *cur, *cur1;
	variable *cv, *cv1;

	// scroll large tables to show selected cell
	cmd( "if { [ info procs iniselcell ] == \"\" } { \
			proc iniselcell { cell } { \
				global g; \
				focus $cell; \
				$cell selection range 0 end; \
				canvassee $g.can $cell \
			} \
		}" );
		
	cur1 = root->search( lab );
	strcpy( previous, "" );
	
	for ( cv1 = cur1->v, j = 0, k = 1; cv1 != NULL; )
	{
		if ( cv1->param == 1 )
		{ 
			strncpy( ch1, cv1->label, MAX_ELEM_LENGTH - 1 );
			ch1[ MAX_ELEM_LENGTH - 1 ] = '\0';
			
			cmd( "ttk::label $w.tit_t%s -text %s", cv1->label, ch1 );
			cmd( "grid $w.tit_t%s -row %d -sticky w -padx { 2 5 }", cv1->label, k );
			cmd( "mouse_wheel $w.tit_t%s", cv1->label );
			cmd( "bind $w.tit_t%s <Enter> { set msg \"Parameter '%s' in '%s'\" }", cv1->label, cv1->label, cur1->label );
			cmd( "bind $w.tit_t%s <Leave> { set msg \"\" }", cv1->label );
			cmd( "ttk::label $w.typ_t%s -text (P) -style hl.TLabel", cv1->label );
			cmd( "grid $w.typ_t%s -row %d -column 1 -padx 1", cv1->label, k );
			cmd( "mouse_wheel $w.typ_t%s", cv1->label );
			cmd( "ttk::button $w.t%s -text \"Set All\" -width -1 -takefocus 0 -style small.TButton -command { set choice 2; set var-S-A %s; set lag %d; set position $w.tit_t%s; set lastFocus [ focus -displayof $w ] }", cv1->label, cv1->label, j, cv1->label );
			cmd( "grid $w.t%s -row %d -column 2", cv1->label, k );
			cmd( "mouse_wheel $w.t%s", cv1->label );
		}
		else
		{ 
			if ( j < cv1->num_lag )
			{
				strncpy( ch1, cv1->label, MAX_ELEM_LENGTH - 1 );
				ch1[ MAX_ELEM_LENGTH - 1 ] = '\0';
				
				cmd( "ttk::label $w.tit_t%s_%d -text %s", cv1->label, j, ch1 );
				cmd( "grid $w.tit_t%s_%d -row %d -sticky w -padx { 2 5 }", cv1->label, j, k );
				cmd( "bind $w.tit_t%s_%d <Enter> { set msg \"Variable '%s' (lag %d) in '%s'\" }", cv1->label, j, cv1->label, j + 1, cur1->label );
				cmd( "bind $w.tit_t%s_%d <Leave> { set msg \"\" }", cv1->label, j );
				cmd( "mouse_wheel $w.tit_t%s_%d", cv1->label, j );
				cmd( "ttk::label $w.typ_t%s_%d -text (V_%d) -style hl.TLabel", cv1->label, j, j + 1 );
				cmd( "grid $w.typ_t%s_%d -row %d -column 1 -padx 1", cv1->label, j, k );
				cmd( "mouse_wheel $w.typ_t%s_%d", cv1->label, j );
				cmd( "ttk::button $w.t%s_%d -text \"Set All\" -width -1 -takefocus 0 -style small.TButton -command { set choice 2; set var-S-A %s; set lag %d; set position $w.tit_t%s_%d; set lastFocus [ focus -displayof $w ] }", cv1->label, j, cv1->label, j, cv1->label, j );
				cmd( "grid $w.t%s_%d -row %d -column 2", cv1->label, j, k );
				cmd( "mouse_wheel $w.t%s_%d", cv1->label, j );
			}
		}

		for ( cur = cur1, i = 1; i <= MAX_COLS && cur != NULL; cur = cur->hyper_next( lab ) , ++i )
		{
			cv = cur->search_var( cur, cv1->label );
			cv->data_loaded = '+';
			
			if ( cv->param == 1 )
			{ 
				sprintf( ch1, "p%s_%d", cv->label, i );
				Tcl_LinkVar( inter, ch1, ( char * ) &( cv->val[ 0 ] ), TCL_LINK_DOUBLE );
				
				cmd( "ttk::entry $w.c%d_v%sp -width $cwidth -justify center -validate focusout -validatecommand { set n %%P; if [ string is double -strict $n ] { set p%s_%d $n; return 1 } { %%W delete 0 end; %%W insert 0 ${p%s_%d}; return 0 } } -invalidcommand { bell }", i, cv->label, cv->label, i, cv->label, i, cv->label, i );
				cmd( "$w.c%d_v%sp insert 0 [ format %%.4g ${p%s_%d} ]", i, cv->label, cv->label, i );
				cmd( "grid $w.c%d_v%sp -row %d -column [ expr 2 + %d ] -padx 1", i, cv->label, k, i );
				cmd( "mouse_wheel $w.c%d_v%sp", i, cv->label );
				
				cmd( "bind $w.c%d_v%sp <Button-1> { iniselcell $w.c%d_v%sp; break }", i, cv->label, i, cv->label );
				
				if ( strlen( previous ) != 0 )
				{
					cmd( "bind %s <Return> { iniselcell $w.c%d_v%sp }", previous, i, cv->label );
					cmd( "bind $w.c%d_v%sp <Shift-Return> { iniselcell %s }", i, cv->label, previous );
					cmd( "bind %s <Tab> { iniselcell $w.c%d_v%sp }", previous, i, cv->label );
					cmd( "bind $w.c%d_v%sp <Shift-Tab> { iniselcell %s }", i, cv->label, previous );
					cmd( "bind %s <Down> { iniselcell $w.c%d_v%sp }", previous, i, cv->label );
					cmd( "bind $w.c%d_v%sp <Up> { iniselcell %s }", i, cv->label, previous );
				}
				else
				{
					cmd( "bind $w.c%d_v%sp <Shift-Return> { break }", i, cv->label );
					cmd( "bind %s <Tab> { break }", previous );
					cmd( "bind $w.c%d_v%sp <Shift-Tab> { break }", i, cv->label );
				}
				
				cmd( "bind $w.c%d_v%sp <FocusIn> { \
						if { $tag_%d != \"\" } { \
							set t \" (instance $tag_%d)\" \
						} { \
							set t \"\" \
						}; \
						set msg \"Parameter '%s'$t\" \
					}", i, cv->label, i, i, cv->label );
				cmd( "bind $w.c%d_v%sp <FocusOut> { set msg \"\" }", i, cv->label );
				sprintf( previous, "$w.c%d_v%sp", i, cv->label );
				
				if ( ! lastFocus )
				{
					cmd( "set lastFocus $w.c%d_v%sp", i, cv->label );
					lastFocus = true;
				}
			}
			else
			{ 
				if ( j < cv->num_lag )
				{
					sprintf( ch1, "v%s_%d_%d", cv->label, i, j );
					Tcl_LinkVar( inter, ch1, ( char * ) &( cv->val[ j ] ), TCL_LINK_DOUBLE );
					
					cmd( "ttk::entry $w.c%d_v%s_%d -width $cwidth -justify center -validate focusout -validatecommand { set n %%P; if [ string is double -strict $n ] { set v%s_%d_%d $n; return 1 } { %%W delete 0 end; %%W insert 0 ${v%s_%d_%d}; return 0 } } -invalidcommand { bell }", i, cv->label, j, cv->label, i, j, cv->label, i, j, cv->label, i, j );
					cmd( "$w.c%d_v%s_%d insert 0 [ format %%.4g ${v%s_%d_%d} ]", i, cv->label, j, cv->label, i, j );
					cmd( "grid $w.c%d_v%s_%d -row %d -column [ expr 2 + %d ] -padx 1", i, cv->label, j, k, i );
					cmd( "mouse_wheel $w.c%d_v%s_%d", i, cv->label, j );

					cmd( "bind  $w.c%d_v%s_%d <Button-1> { iniselcell $w.c%d_v%s_%d; break }", i, cv->label, j, i, cv->label, j );
					
					if ( strlen( previous ) != 0 )
					{
						cmd( "bind %s <Return> { iniselcell $w.c%d_v%s_%d }", previous, i, cv->label, j );
						cmd( "bind  $w.c%d_v%s_%d <Shift-Return> { iniselcell %s }", i, cv->label, j, previous );
						cmd( "bind %s <Tab> { iniselcell $w.c%d_v%s_%d }", previous, i, cv->label, j );
						cmd( "bind  $w.c%d_v%s_%d <Shift-Tab> { iniselcell %s }", i, cv->label, j, previous );
						cmd( "bind %s <Down> { iniselcell $w.c%d_v%s_%d }", previous, i, cv->label, j );
						cmd( "bind  $w.c%d_v%s_%d <Up> { iniselcell %s }", i, cv->label, j, previous );
					}
					else
					{
						cmd( "bind  $w.c%d_v%s_%d <Shift-Return> { break }", i, cv->label, j );
						cmd( "bind %s <Tab> { break }", previous );
						cmd( "bind  $w.c%d_v%s_%d <Shift-Tab> { break }", i, cv->label, j );
					}
					
					cmd( "bind $w.c%d_v%s_%d <FocusIn> { \
							if { $tag_%d != \"\" } { \
								set t \" (instance $tag_%d)\" \
							} { \
								set t \"\" \
							}; \
							set msg \"Variable '%s' (lag %d)$t\" \
						}", i, cv->label, j, i, i, cv->label, j + 1 );
					cmd( "bind $w.c%d_v%s_%d <FocusOut> { set msg \"\" }", i, cv->label, j );
					sprintf( previous, "$w.c%d_v%s_%d", i, cv->label, j );
					
					if ( ! lastFocus )
					{
						cmd( "set lastFocus $w.c%d_v%s_%d", i, cv->label, j );
						lastFocus = true;
					}
				}
			}
		}
	  
		// missing binds for last cell
		if ( strlen( previous ) != 0 )
		{
			cmd( "bind %s <Return> { break }", previous );
			cmd( "bind %s <Tab> { break }", previous );
			cmd( "bind %s <Down> { break }", previous );
		}
		
		// indicate columns overflow (>MAX_COLS)
		if ( ! colOvflw && cur != NULL )
			colOvflw = true;

		// set flag of data loaded also to not shown pars.
		for ( ; cur != NULL; cur = cur->hyper_next( lab ) )
		{
			cv = cur->search_var( cur, cv1->label );
			cv->data_loaded = '+';
		}
		
		if ( cv1->param == 1 || cv1->num_lag > 0 )
			++k;
		
		if ( cv1->param == 0 && j + 1 < cv1->num_lag )
			++j;
		else
		{
			cv1 = cv1->next;
			j = 0;
		}
	}
}
