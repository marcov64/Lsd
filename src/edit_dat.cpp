/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
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
int set_focus;


/****************************************************
EDIT_DATA
****************************************************/
void edit_data( object *root, int *choice, char *obj_name )
{
	char *l , ch[ 2 * MAX_ELEM_LENGTH ], ch1[ MAX_ELEM_LENGTH ];
	int i, counter, lag;
	object *first;

	cmd( "if {$tcl_platform(os) == \"Darwin\"} {set cwidth 9; set cbd 2 } {set cwidth 8; set cbd 2}" );

	Tcl_LinkVar( inter, "lag", ( char * ) &lag, TCL_LINK_INT );

	cmd( "if { ! [ info exists autoWidth ] } { set autoWidth 1 }" );
	cmd( "if { ! [ winfo exists .ini ] } { newtop .ini; showtop .ini topleftW 1 1 1 $hsizeI $vsizeI } { if { ! $autoWidth } { resizetop $hsizeI $vsizeI } }" );

	cmd( "set position 1.0" );
	in_edit_data = true;

	*choice = 0;
	while ( *choice == 0 )
	{
		// reset title and destroy command because may be coming from set_obj_number
		cmd( "settop .ini \"%s%s - LSD Initial Values Editor\" { set choice 1 }", unsaved_change() ? "*" : " ", simul_name  );

		first = root->search( obj_name );

		cmd( "frame .ini.b" );
		cmd( "set w .ini.b.tx" );
		cmd( "scrollbar .ini.b.ys -command \".ini.b.tx yview\"" );
		cmd( "scrollbar .ini.b.xs -command \".ini.b.tx xview\" -orient horizontal" );
		cmd( "text $w -yscrollcommand \".ini.b.ys set\" -xscrollcommand \".ini.b.xs set\" -wrap none" );
		cmd( ".ini.b.tx conf -cursor arrow" );
		
		strncpy( ch1, obj_name, MAX_ELEM_LENGTH - 1 );
		ch1[ MAX_ELEM_LENGTH - 1 ] = '\0';
		cmd( "label $w.tit_empty -width 32 -relief raised -text \"Object: %-17s \" -borderwidth 4", ch1 );
		cmd( "bind $w.tit_empty <Button-1> {set choice 4}" );
		
		if ( ! in_set_obj )				// show only if not already recursing
			cmd( "bind $w.tit_empty <Enter> {set msg \"Click to edit number of instances\"}" );
			
		cmd( "bind $w.tit_empty <Leave> {set msg \"\"}" );
		cmd( "$w window create end -window $w.tit_empty" );

		strcpy( ch, "" );
		i = 0;
		counter = 1;
		colOvflw = false;
		search_title( root, ch, &i, obj_name, &counter );
		cmd( "$w insert end \\n" );

		// explore the tree searching for each instance of such object and create:
		// - titles
		// - entry cells linked to the values
		
		set_focus = 0;
		link_data( root, obj_name );
		
		cmd( "pack .ini.b.ys -side right -fill y" );
		cmd( "pack .ini.b.xs -side bottom -fill x" );
		cmd( "pack .ini.b.tx -expand yes -fill both" );
		cmd( "pack .ini.b  -expand yes -fill both" );

		cmd( "label .ini.msg -textvariable msg" );
		cmd( "pack .ini.msg -pady 5" );

		cmd( "frame .ini.st" );
		cmd( "label .ini.st.err -text \"\"" );
		cmd( "label .ini.st.pad -text \"         \"" );
		cmd( "checkbutton .ini.st.aw -text \"Automatic width\" -variable autoWidth -command { set choice 5 }" );
		cmd( "pack .ini.st.err .ini.st.pad .ini.st.aw -side left" );
		cmd( "pack .ini.st -anchor e -padx 10 -pady 5" );

		cmd( "donehelp .ini boh { set choice 1 } { LsdHelp menudata_init.html }" );

		cmd( "$w configure -state disabled" );

		if ( set_focus == 1 )
			cmd( "focus $initial_focus; $initial_focus selection range 0 end" );

		cmd( "bind .ini <KeyPress-Escape> {set choice 1}" );
		cmd( "bind .ini <F1> { LsdHelp menudata_init.html }" );

		// show overflow warning just once per configuration but always indicate
		if ( colOvflw )
		{
			cmd( ".ini.st.err conf -text \"OBJECTS NOT SHOWN! (> %d)\" -fg red", MAX_COLS );
			if ( ! iniShowOnce )
			{
				cmd( "update; tk_messageBox -parent .ini -type ok -title Warning -icon warning -message \"Too many objects to edit\" -detail \"LSD Initial Values editor can show only the first %d objects' values. Please use the 'Set All' button to define values for objects beyond those.\" ", MAX_COLS );
				iniShowOnce = true;
			}
		}

		noredraw:
		
		cmd( "if [ info exists lastEditPos ] { $w yview moveto $lastEditPos; unset lastEditPos }" );
		cmd( "if { [ info exists lastInitialFocus ] && [ winfo exists $lastInitialFocus ] } { focus $lastInitialFocus; $lastInitialFocus selection range 0 end; unset lastInitialFocus }" );

		cmd( "if $autoWidth { resizetop .ini [ expr ( 40 + %d * ( $cwidth + 1 ) ) * [ font measure TkTextFont -displayof .ini 0 ] ] }", counter );

		// editor main command loop
		while ( ! *choice )
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

		cmd( "set lastEditPos [ lindex [ $w yview ] 0 ]" );
			
		// clean up
		strcpy( ch, "" );
		i = 0;
		clean_cell( root, ch, obj_name );
		cmd( "destroy .ini.b .ini.boh .ini.msg .ini.st" );


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
			cmd( "unset -nocomplain lastEditPos lastInitialFocus" );
		
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

		cmd( "set %d_titheader \"%s\"", *incr ,ch2 );

		cmd( "entry $w.c%d_tit -width $cwidth -bd $cbd -relief raised -justify center -textvariable \"%d_titheader\" -state readonly", *incr ,*incr );
		cmd( "$w window create end -window $w.c%d_tit", *incr );
		
		if ( strlen(tag) == 0 )
			cmd( "set tag_%d \" \"", *incr );
		else
			cmd( "set tag_%d %s", *incr, tag );
		
		*incr = *incr + 1;
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
	int i, j;
	char previous[ MAX_ELEM_LENGTH + 20 ], ch1[ MAX_ELEM_LENGTH ];
	object *cur, *cur1;
	variable *cv, *cv1;

	cur1 = root->search( lab );
	strcpy( previous, "" );
	
	for ( cv1 = cur1->v, j = 0; cv1 != NULL; )
	{
		if ( cv1->param == 1 )
		{ 
			strncpy( ch1, cv1->label, MAX_ELEM_LENGTH - 1 );
			ch1[ MAX_ELEM_LENGTH - 1 ] = '\0';
			cmd( "label $w.tit_t%s -anchor w -width 25 -text \"Par: %-25s\" -borderwidth 4", cv1->label, ch1 );
			cmd( "$w window create end -window $w.tit_t%s", cv1->label );
			cmd( "bind $w.tit_t%s <Enter> { set msg \"Parameter '%s'\" }", cv1->label, cv1->label );
			cmd( "bind $w.tit_t%s <Leave> { set msg \" \" }", cv1->label );
			cmd( "button $w.b%s_%d -text \"Set All\" -pady 0m -padx 1m -command { set choice 2; set var-S-A %s; set lag %d; set position $w.tit_t%s; set lastInitialFocus $w.c1_v%sp }", cv1->label, j, cv1->label, j, cv1->label, cv1->label );
			cmd( "$w window create end -window $w.b%s_%d", cv1->label, j );
		}
		else
		{ 
			if ( j < cv1->num_lag )
			{
				strncpy( ch1, cv1->label, MAX_ELEM_LENGTH - 1 );
				ch1[ MAX_ELEM_LENGTH - 1 ] = '\0';
				cmd( "label $w.tit_t%s_%d -anchor w -width 25 -text \"Var: %-20s (-%d)\" -borderwidth 4", cv1->label, j, ch1, j + 1 );
				cmd( "$w window create end -window $w.tit_t%s_%d", cv1->label, j );
				cmd( "bind $w.tit_t%s_%d <Enter> { set msg \"Variable '%s' with lag %d\" }", cv1->label, j, cv1->label, j + 1 );
				cmd( "bind $w.tit_t%s_%d <Leave> { set msg \" \" }", cv1->label, j );
				cmd( "button $w.b%s_%d -text \"Set All\" -pady 0m -padx 1m -command { set choice 2; set var-S-A %s; set lag %d; set position $w.tit_t%s_%d; set lastInitialFocus $w.c1_v%s_0 }", cv1->label, j, cv1->label, j, cv1->label, j, cv1->label );
				cmd( "$w window create end -window $w.b%s_%d", cv1->label, j );
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
				
				cmd( "entry $w.c%d_v%sp -width $cwidth -bd $cbd -validate focusout -vcmd {if [string is double -strict %%P] {set p%s_%d %%P; return 1} {%%W delete 0 end; %%W insert 0 $p%s_%d; return 0}} -invcmd {bell} -justify center", i, cv->label, cv->label, i, cv->label, i  );
				cmd( "$w.c%d_v%sp insert 0 $p%s_%d", i, cv->label, cv->label, i );
				
				if ( set_focus == 0 )
				{
					cmd( "set initial_focus $w.c%d_v%sp", i, cv->label );
					set_focus = 1;
				}
				
				cmd( "$w window create end -window $w.c%d_v%sp", i, cv->label );
				
				if ( strlen( previous ) != 0 )
				{
					cmd( "bind %s <KeyPress-Return> {focus $w.c%d_v%sp; $w.c%d_v%sp selection range 0 end; $w see $w.c%d_v%sp}", previous, i, cv->label, i, cv->label, i, cv->label );
					cmd( "bind %s <KeyPress-Down> {focus $w.c%d_v%sp; $w.c%d_v%sp selection range 0 end; $w see $w.c%d_v%sp}", previous, i, cv->label, i, cv->label, i, cv->label );
					cmd( "bind $w.c%d_v%sp <KeyPress-Up> {focus %s; %s selection range 0 end; $w see %s}", i, cv->label, previous, previous, previous );
				}
				
				cmd( "bind $w.c%d_v%sp <FocusIn> {set msg \"Inserting parameter '%s' in '%s' $tag_%d\"}", i,cv->label,cv->label,cur1->label,i );
				cmd( "bind $w.c%d_v%sp <FocusOut> {set msg \" \"}", i, cv->label );
				sprintf( previous, "$w.c%d_v%sp", i, cv->label );
			}
			else
			{ 
				if ( j < cv->num_lag )
				{
					sprintf( ch1, "v%s_%d_%d", cv->label, i, j );
					Tcl_LinkVar( inter, ch1, ( char * ) &( cv->val[ j ] ), TCL_LINK_DOUBLE );
					
					cmd( "entry $w.c%d_v%s_%d -width $cwidth -bd $cbd -validate focusout -vcmd {if [string is double -strict %%P] {set v%s_%d_%d %%P; return 1} {%%W delete 0 end; %%W insert 0 $v%s_%d_%d; return 0}} -invcmd {bell} -justify center", i, cv->label, j, cv->label, i, j, cv->label, i, j );
					cmd( "$w.c%d_v%s_%d insert 0 $v%s_%d_%d", i, cv->label, j, cv->label, i, j );
					
					if ( set_focus == 0 )
					{
						cmd( "set initial_focus $w.c%d_v%s_%d", i, cv->label, j );
						set_focus = 1;
					}

					cmd( "$w window create end -window $w.c%d_v%s_%d", i, cv->label, j );
					if ( strlen( previous ) != 0 )
					{
						cmd( "bind %s <KeyPress-Return> {focus $w.c%d_v%s_%d; $w.c%d_v%s_%d selection range 0 end; $w see  $w.c%d_v%s_%d}", previous, i, cv->label, j, i, cv->label, j, i, cv->label, j );
						cmd( "bind %s <KeyPress-Down> {focus $w.c%d_v%s_%d; $w.c%d_v%s_%d selection range 0 end; $w see  $w.c%d_v%s_%d}", previous, i, cv->label, j, i, cv->label, j, i, cv->label, j );
						cmd( "bind  $w.c%d_v%s_%d <KeyPress-Up> {focus %s; %s selection range 0 end; $w see  %s}", i, cv->label, j, previous, previous, previous );
					}
					
					cmd( "bind $w.c%d_v%s_%d <FocusIn> {set msg \"Inserting variable '%s' (lag %d) in '%s' $tag_%d\"}", i, cv->label, j, cv->label, j + 1, cur1->label, i );
					cmd( "bind $w.c%d_v%s_%d <FocusOut> {set msg \" \"}", i, cv->label, j );
					sprintf( previous, "$w.c%d_v%s_%d", i, cv->label, j );
				}
			}
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
			cmd( "$w insert end \\n" );
		
		if ( cv1->param == 0 && j + 1 < cv1->num_lag )
			++j;
		else
		{
			cv1 = cv1->next;
			j = 0;
		}
	}
}
