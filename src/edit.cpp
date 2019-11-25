/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/*************************************************************
EDIT.CPP
This functions manage the computation, display and modification of
objects' number. Any call to these module starts by scanning the whole
model tree, counts the number of each type of objects and displays orderly
the information.

On request, it is possible to change these values, either for single "branches"
of the model or for the whole set of one type of Objects.
It can exit to return to the calling function (either the browser in INTERF.CPP
or the set initial values in EDIT.CPP) or going in setting initial values.

The main functions contained in this file are:

- void set_obj_number( object *r, int *choice )
The main function, called from the browser. Initialize the text widget and wait
the actions of the users to take place.

- void insert_obj_num( object *root, char *tag, char *indent, int counter, int *i, int *value );
Does the real job. Scan the model from root recursively and for each Object found
counts the number, prepare its index if the parent has multiple instances,
and set the indentation. Each label is bound to return a unique integer number
in case it is clicked. Such number is used as guide for the following function

- void edit_str( object *root, char *tag, int counter, int *i, int res, int *num, int *choice, int *done );
Explore recursively the model tree giving a unique number for every group of
objects encountered. When it finds the one clicked by user prepare the
window to accept a new value for the number of instances. Passes thie value
to the next function

- void chg_obj_num( object *c, int value, int all, int *choice );
Depending on all (the flag to modify all the values of that type in the model)
changes only the number of instances following c, or otherwise, every group of
intences of the type of c. If it has to increase the number of instances,
it does it directly. If it has to decrease, checks again all. If all is false,
it activate the routine below, otherwise, it eliminates directly the surplus

- void eliminate_obj( object *r, int actual, int desired , int *choice );
Ask the user whether he wants to eliminate the last object or to choose
individually the ones to eliminate. In this second case, it asks for a list
numbers. The list is as long as are the instances to eliminate. Each element
is the ordinal number of one instance to eliminate
*************************************************************/

#include "decl.h"

bool hid_level;
bool in_set_obj = false;		// avoid recursive usage (confusing and tk windows are not ready)
char lab_view[ MAX_ELEM_LENGTH ];
char tag_view[ MAX_ELEM_LENGTH ];
int level;
int lowest_level;
int max_depth;


/***************************************************
SET_OBJ_NUMBER
****************************************************/
void set_obj_number( object *r, int *choice )
{
	char ch[ 2 * MAX_ELEM_LENGTH ], *l;
	int i, num, res, count, done;

	Tcl_LinkVar( inter, "val", ( char * ) &count, TCL_LINK_INT );
	Tcl_LinkVar( inter, "i", ( char * ) &i, TCL_LINK_INT );
	Tcl_LinkVar( inter, "num", ( char * ) &num, TCL_LINK_INT );
	Tcl_LinkVar( inter, "result", ( char * ) &res, TCL_LINK_INT );
	Tcl_LinkVar( inter, "max_depth", ( char * ) &max_depth, TCL_LINK_INT );

	cmd( "set ini .ini" );
	cmd( "if { ! [ winfo exists .ini ] } { newtop .ini; showtop .ini topleftW 1 1 1 $hsizeN $vsizeN } { resizetop .ini $hsizeN $vsizeN }" );

	in_set_obj = true;
	strcpy( lab_view, "" );
	strcpy( tag_view, "" );
	level = lowest_level = 1;
	max_depth = 0;							// start with all levels open
	
	while ( *choice == 0 )
	{
		// reset title and destroy command because may be coming from edit_data
		cmd( "settop .ini \"%s%s - LSD Object Number Editor\" { set choice 1; set result -1 }", unsaved_change() ? "*" : " ", simul_name );
	  
		cmd( "frame .ini.obj" );
		cmd( "pack .ini.obj -fill both -expand yes" );
		cmd( "set b .ini.obj" );
		cmd( "scrollbar $b.scroll -command \"$b.list yview\"" );
		cmd( "scrollbar $b.scrollh -command \"$b.list xview\" -orient horizontal" );
		cmd( "set t $b.list" );
		cmd( "text $t -yscrollcommand \"$b.scroll set\" -xscrollcommand \"$b.scrollh set\" -wrap none -cursor arrow" );

		strcpy( ch, "" );
		i = 0;
		hid_level = false;
		insert_obj_num( r,  ch, "", 1, &i, &count );
		if ( max_depth < 1 )
		{
			hid_level = false;
			max_depth = lowest_level - 1;	// start with all levels open
		}

		cmd( "pack $b.scroll -side right -fill y" );
		cmd( "pack $b.scrollh -side bottom -fill x" );  
		cmd( "pack $b.list -fill both -expand yes" );
		cmd( "pack $b" );
		cmd( "set msg \"\"" );
		cmd( "label .ini.msglab -textvariable msg" );
		cmd( "pack .ini.msglab -pady 5" );
	  
		cmd( "frame .ini.f" );
		cmd( "label .ini.f.tmd -text \"Show level:\"" );
		cmd( "label .ini.f.emd -width 2 -fg red -text \"$max_depth \"" );
		cmd( "button .ini.f.mn -width 2 -text \"-\" -command { if { $max_depth > 1 } { incr max_depth -1; set choice 4 } }" );
		cmd( "button .ini.f.pl -width 2 -text \"+\" -command { if { %d } { incr max_depth; set choice 4 } }", hid_level );
		cmd( "pack .ini.f.tmd .ini.f.emd .ini.f.mn .ini.f.pl -side left" );
	
		cmd( "pack .ini.f -anchor e -padx 10 -pady 5" );
		cmd( "donehelp .ini fb { set choice 1; set result -1 } { LsdHelp menudata_objn.html }" );
	  
		cmd( "$t configure -state disabled" );
		cmd( "bind .ini <minus> { .ini.f.mn invoke }" );
		cmd( "bind .ini <plus> { .ini.f.pl invoke }" );
		cmd( "bind .ini <F1> { LsdHelp menudata_objn.html }" );

		if ( strlen( lab_view ) > 0 )
			cmd( "if { [ info exists toview ] && [ winfo exists $toview ] } { $t see $toview }" );

		noredraw:

		// editor command loop
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

		if ( *choice == 3 && in_edit_data )		// avoid recursion
		{
			*choice = 0;
			goto noredraw;
		}

		if ( *choice == 2 )
		{
			i = 0;
			done = 0;
			edit_str( r, ch, 1, &i, res, &num, choice, &done );
			*choice = 2;
		}

		cmd( "destroy $b .ini.msglab .ini.f .ini.fb" );

		strcpy( ch, "" );
		i = 0;
		done = 0;
		switch ( *choice )
		{
			case 1: 
				break;
				
			case 2: 
			case 4: 
				*choice = 0;
				break;
				  
			case 3: 
				l = ( char * ) Tcl_GetVar( inter, "obj_name", 0 );
				strcpy( ch, l );
				edit_data( r, choice, ch );
				*choice = 0;
				break;
				
			default: 
				plog( "\nChoice not recognized" );
				*choice = 0;
		}
	}

	in_set_obj = false;

	Tcl_UnlinkVar( inter, "i" );
	Tcl_UnlinkVar( inter, "val" );
	Tcl_UnlinkVar( inter, "result" );
	Tcl_UnlinkVar( inter, "num" );
	Tcl_UnlinkVar( inter, "max_depth" );

	cmd( "unset result num choice val i" );
}


/***************************************************
INSERT_OBJ_NUM
****************************************************/
void insert_obj_num( object *root, char const *tag, char const *ind, int counter, int *i, int *value )
{
	char ch[ 2 * MAX_ELEM_LENGTH ], indent[ 30 ];
	object *c, *cur;
	int num = 0;
	bridge *cb; 

	strcpy( ch, tag );
	strcpy( indent, ind );

	if ( root->up != NULL )
	  strcat( indent, "  +  " );

	for ( cb = root->b, counter = 1; cb != NULL; cb = cb->next, counter = 1 )
	{  
		if ( cb->head == NULL )
			continue;
		
		*i += 1;
		c = cb->head;
		for ( cur = c, num = 0; cur != NULL; cur = cur->next, ++num );

		// just update if already exists
		cmd( "set val [ winfo exists $t.val$i ]" );
		
		if ( *value )
		{
			*value = num;
			cmd( "$t.val$i configure -text $val" );
		}
		else
		{
			*value = num;
			
			cmd( "label $t.lab$i -text \"%s %s \" -foreground red -bg white -justify left", indent, c->label );
			cmd( "pack $t.lab$i -anchor w" );

			if ( strlen( tag ) != 0 )
				cmd( "label $t.tag$i -text \" in %s %s \" -bg white", (c->up)->label, tag );
			else
				cmd( "label $t.tag$i -text \" \" -bg white" );

			cmd( "pack $t.tag$i -anchor w" );

			cmd( "set count$i $val" );
			cmd( "label $t.val$i -text $val -width 5 -relief raised" );
			cmd( "pack $t.val$i" );
			cmd( "$t window create end -window $t.lab$i" );
			cmd( "$t window create end -window $t.tag$i" );
			cmd( "$t window create end -window $t.val$i" );
			
			if ( strlen( lab_view ) > 0 )
				if ( ! strcmp( lab_view, c->label ) && ! strcmp( tag_view, tag ) )
					cmd( "set toview \"$t.val$i\"" );

			if ( level >= max_depth && c->b != NULL )
				hid_level = true;

			cmd( "$t insert end \\n" );
			cmd( "bind $t.val$i <Button-1> { set result %d; set num $count%d; set toview \"$t.val%d\"; set choice 2 }", *i, *i, *i );
			cmd( "bind $t.lab$i <Button-1> { set obj_name %s; set choice 3 }", c->label );
		   
			if ( ! in_edit_data )				// show only if not already recursing
				cmd( "bind $t.lab$i <Enter> { set msg \"Click to edit values for '%s'\" }", c->label );
			cmd( "bind $t.lab$i <Leave> { set msg \"\" }" );
		}
		
		Tcl_DoOneEvent( 0 );

		if ( max_depth < 1 || level < max_depth )
		{
			for ( cur = c; cur != NULL; ++counter, cur=go_brother( cur ) )
			{
				++level;
				lowest_level = level > lowest_level ? level : lowest_level;
				
				if ( strlen( tag ) != 0 )
					sprintf( ch, "%d - %s %s", counter, ( c->up )->label,  tag );
				else
					sprintf( ch, "%d", counter );
				
				insert_obj_num( cur, ch, indent, counter, i, value );
				
				--level;
			}
		}
	}
}


/***************************************************
COMPUTE_COPYFROM
****************************************************/
int compute_copyfrom( object *c, int *choice )
{
	object *cur, *cur1, *cur2, *cur3;
	int i, j, k, h, res;

	if ( c == NULL || c->up == NULL )
	{
		cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Element in Root object\" -detail \"The Root object is always single-instanced, so any element contained in it has only one instance.\"" );
		return 1;
	}

	cmd( "set conf 0" );

	cmd( "set cc .compcopy" );
	cmd( "newtop $cc \"Instance Number\" { set cconf 1; set choice 1 }" );

	cmd( "frame $cc.l" );
	cmd( "label $cc.l.l -text \"Determine the effective instance number of '%s' \\nby computing the instance numbers of the containing objects.\\nPress 'Done' to use the number and continue.\"", c->label );
	cmd( "pack $cc.l.l" );

	cmd( "frame $cc.f" );

	for ( i = 1, j = 1, cur = c; cur->up != NULL; cur = cur->up, ++j ) 
	{
		cmd( "frame $cc.f.f%d", j );
		cmd( "label $cc.f.f%d.l -text \"Instance number of '%s'\"", j, cur->label );
		cmd( "entry $cc.f.f%d.e -width 5 -textvariable num%d -justify center", j, j );
		cmd( "pack $cc.f.f%d.l $cc.f.f%d.e -side left -padx 2", j, j );

		for ( i = 1, cur1 = cur->up->search( cur->label ); cur1 != cur; cur1 = cur1->next, ++i );

		cmd( "set num%d %d", j, i );
	}
	  
	cmd( "focus $cc.f.f%d.e; $cc.f.f%d.e selection range 0 end", j - 1, j - 1 );

	for ( --j, cur = c; cur->up != NULL; cur = cur->up, --j )
	{//pack in inverse order
		cmd( "pack $cc.f.f%d", j );
		cmd( "bind $cc.f.f%d.e <Return> { focus .compcopy.f.f%d.e; .compcopy.f.f%d.e selection range 0 end}", j, j - 1, j - 1 );
	}

	cmd( "bind $cc.f.f1.e <Return> \"focus $cc.b.com\"" );

	cmd( "frame $cc.r" );
	cmd( "label $cc.r.l -text \"Global instance number:\"" );
	cmd( "label $cc.r.res -fg red -text %d", i );
	cmd( "pack $cc.r.l $cc.r.res -side left -padx 2" );

	cmd( "pack $cc.l $cc.f $cc.r -pady 5 -padx 5" );

	cmd( "comphelpdone $cc b { set cconf 1; set choice 2 } { LsdHelp menudata_objn.html#compute } { set cconf 1; set choice 1 }" );

	cmd( "showtop $cc" );

	res = i;

	here_ccompute:
	
	for ( cur = c->up; cur->up != NULL; cur = cur->up ); //cur is root
	cur = cur->search( c->label ); //find the first

	for ( i = 0, k = 0, cur3 = NULL; k == 0 && cur != NULL ; cur3 = cur, cur = cur->hyper_next( c->label ), ++i )
	{
		k = 1;
		for ( j = 1, cur1 = cur; cur1->up != NULL; cur1 = cur1->up, ++j )
		{
			cmd( "if [ string is integer -strict $num%d ] { set choice $num%d } { set choice -1 }", j, j );
			if ( *choice < 0 )
				break;

			for ( h = 1, cur2 = cur1->up->search( cur1->label ); cur2 != cur1; cur2 = cur2->next, ++h );   
			if ( cur2->next == NULL && *choice > h )
				*choice = h;
			if ( h < *choice )
			{
				k = 0;
				break;
			}
		}
	}

	res = i;
	
	// reset possibly erroneous values
	for ( j = 1, cur2 = cur3; cur2 != NULL && cur2->up != NULL; cur2 = cur2->up, ++j ) 
	{
		for ( i = 1, cur1 = cur2->up->search( cur2->label ); cur1 != cur2; cur1 = cur1->next, ++i );

		cmd( "set num%d %d", j, i );
	}

	cmd( "$cc.r.res configure -text %d", res ); 
	   
	here_cfrom:

	cmd( "set ccfrom 0" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	i = *choice;
	cmd( "set choice $cconf" );
	if ( *choice == 0 )
		goto here_cfrom;
	else
		*choice = i;  

	if ( *choice == 2 )
		goto here_ccompute;

	cmd( "destroytop $cc" );
	return res;
}


/***************************************************
ENTRY_NEW_OBJNUM
****************************************************/
void entry_new_objnum( object *c, int *choice, char const *tag )
{  
	object *cur, *first;
	int cfrom, j, max_level, affect, k, *pippo, num;

	for ( num = 0, cur = c->up->search( c->label ); cur != NULL; cur = go_brother( cur ), ++num );

	strcpy( lab_view, c->label );
	strcpy( tag_view, tag );

	cmd( "set conf 0" );
	cmd( "set num %d", num );
	cmd( "set cfrom 1" );

	cmd( "set n .numobj" );
	cmd( "newtop $n \"Number of Instances\" { set conf 1; set choice 2 }" );

	cmd( "frame $n.l" );

	cmd( "frame $n.l.n1" );
	cmd( "label $n.l.n1.l1 -text \"Object:\"" );
	cmd( "label $n.l.n1.l2 -fg red -text \"%s\"", c->label );
	cmd( "pack $n.l.n1.l1 $n.l.n1.l2 -side left" );

	cmd( "frame $n.l.n2" );
	cmd( "label $n.l.n2.l1 -text \"Contained in:\"" );
	cmd( "label $n.l.n2.l2 -fg red -text \"%s %s\"", c->up->label, tag );
	cmd( "pack $n.l.n2.l1 $n.l.n2.l2 -side left" );

	cmd( "pack $n.l.n1 $n.l.n2" );

	cmd( "frame $n.e" );
	cmd( "label $n.e.l -text \"Number of instances\"" );
	cmd( "if [ string equal [ info tclversion ] 8.6 ] { ttk::spinbox $n.e.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { if [ string is integer -strict %%P ] { set num %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num; return 0 } } -invalidcommand { bell } -justify center } { entry $n.e.e -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set num %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num; return 0 } } -invcmd { bell } -justify center }" );
	cmd( "pack $n.e.l $n.e.e -side left -padx 2" );

	cmd( "frame $n.cp" );
	cmd( "label $n.cp.l -text \"Copy from instance\"" );
	cmd( "if [ string equal [ info tclversion ] 8.6 ] { ttk::spinbox $n.cp.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { if [ string is integer -strict %%P ] { set cfrom %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cfrom; return 0 } } -invalidcommand { bell } -justify center } { entry $n.cp.e -width 5 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set cfrom %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cfrom; return 0 } } -invcmd { bell } -justify center }" );
	cmd( "button $n.cp.compute -width 7 -text Compute -command { set conf 1; set choice 3; .numobj.cp.e selection range 0 end; focus .numobj.cp.e }" );
	cmd( "pack $n.cp.l $n.cp.e $n.cp.compute -side left -padx 2" );

	cmd( "frame $n.ef" );
	cmd( "label $n.ef.l -text \"Modify groups\"" );

	cmd( "frame $n.ef.g -relief groove -bd 2" );
	
	for ( j = 1, cur = c->up; cur->up != NULL; cur = cur->up, j++ )
	{
		if ( j == 1 )
		{
			first = cur->up->search( cur->label );
			for ( k = 1; first != cur; first = go_brother( first ), ++k );
			cmd( "set affect 1.%d", k );
			cmd( "radiobutton $n.ef.g.r1 -text \"This group of '%s' contained in '%s' #%d\" -variable affect -value 1.%d", c->label, cur->label, k, k );
		}
		else
		{
			first = cur->up->search( cur->label );
			for ( k = 1; first != cur; first = go_brother( first ), ++k );
			cmd( "radiobutton $n.ef.g.r%d -text \"All groups of '%s' contained in '%s' #%d\" -variable affect -value %d.%d", j, c->label, cur->label, k, j, k );
		}
		cmd( "pack $n.ef.g.r%d -anchor w", j );
	}
	
	cmd( "radiobutton $n.ef.g.r%d -text \"All groups of '%s' in the model\" -variable affect -value %d.1", j, c->label, j );
	cmd( "pack $n.ef.g.r%d -anchor w", j );

	max_level = j;
	if ( j == 1 ) // we are dealing with root's descendants
		cmd( "set affect 1.1" );

	cmd( "pack $n.ef.l $n.ef.g" );
	   
	cmd( "pack $n.l $n.e $n.cp $n.ef -pady 5 -padx 5" );

	cmd( "okhelpcancel $n b { set conf 1; set choice 1 } { LsdHelp menudata_objn.html#modifyNumberObj } { set conf 1; set choice 2 }" );
	cmd( "bind $n.e.e <KeyPress-Return> { set conf 1; set choice 1 }" );

	cmd( "showtop $n" );

	j = 1;
	*choice = 0;
	 
	here_objec_num:

	cmd( "write_any .numobj.e.e $num" ); 
	cmd( "write_any .numobj.cp.e $cfrom" ); 

	if ( j == 1 )
	{
		cmd( ".numobj.e.e selection range 0 end" );
		cmd( "focus .numobj.e.e" );
		j = 0;
	}

	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );
		  
	cmd( "set num [ .numobj.e.e get ]" ); 
	cmd( "set cfrom [ .numobj.cp.e get ]" ); 

	k = *choice;

	cmd( "set choice $conf" );
	if ( *choice == 0 )
		goto here_objec_num;
	else
		*choice=k;  

	if ( *choice == 3 )
	{
		*choice = 0;
		k = compute_copyfrom( c, choice );
		if ( k > 0 )
			cmd( "set cfrom %d", k );
		cmd( "set conf 0" );
		*choice = 0;
		goto here_objec_num;
	} 

	cmd( "destroytop $n" );  
			 
	if ( *choice == 2 )
		return;

	cmd( "set choice $cfrom" );  
	cfrom = *choice;
	cmd( "set choice [ lindex [ split $affect . ] 0 ]" );
	affect = *choice;
	cmd( "set choice [ lindex [ split $affect . ] 1 ]" );
	k = *choice;

	pippo = new int[ max_level + 1 ];
	for ( j = 1; j <= max_level; ++j )
		pippo[ j ] = ( j == affect ) ? k : -1;

	cmd( "set choice $num" );
	chg_obj_num( &c, *choice, affect, pippo, choice, cfrom );

	redrawRoot = true;			// update list boxes

	delete [ ] pippo;
}


/***************************************************
EDIT_STR
****************************************************/
void edit_str( object *root, char *tag, int counter, int *i, int res, int *num, int *choice, int *done )
{
	char ch[ 2 * MAX_ELEM_LENGTH ];
	object *c, *cur;
	int multi = 0;
	bridge *cb;

	strcpy( ch, tag );
	for ( cb = root->b, counter = 1; cb != NULL && *done == 0; cb = cb->next, counter = 1 )
	{ 
		if ( cb->head == NULL )
			continue;
		
		c = cb->head; 
		*i += 1;
		if ( *i == res )
		{
			entry_new_objnum( c, choice, tag );
			*done = 1;
		}

		if ( go_brother( c ) != NULL )
			multi = 1;
		else
			multi = 0;
		
		for ( cur = c; cur != NULL && *done == 0; ++counter, cur = go_brother( cur ) )
		{
			if ( multi == 1 || multi == 0 )
				if ( strlen( tag ) != 0 )
					sprintf( ch, "%s-%d", tag, counter );
				else
					sprintf( ch, "%d", counter );
			else
				sprintf( ch, "%s", tag );
			if ( level < max_depth )
			{
				level++;
				edit_str( cur, ch, counter, i, res, num, choice, done );
				level--;
			} 
		}
	}
}


/***************************************************
ELIMINATE_OBJ
****************************************************/
void eliminate_obj( object **r, int actual, int desired , int *choice )
{
	int i, j, *del, value, last;
	object *cur, *app;

	cmd( "set d .delobj" );
	cmd( "newtop $d \"Delete Instances\" { set choice 3 }" );
	cmd( "set conf 0" );

	cmd( "frame $d.l" );
	cmd( "label $d.l.l1 -text \"Object:\"" );
	cmd( "label $d.l.l2 -fg red -text \"%s\"", ( *r )->label );
	cmd( "pack $d.l.l1 $d.l.l2 -side left" );

	cmd( "frame $d.t" );
	cmd( "label $d.t.txt1 -text \"Do you want to delete the last\"" );

	cmd( "label $d.t.txt2 -text \"%d instances(s)\" -fg red", actual-desired );
	cmd( "label $d.t.txt3 -text \"or you want to choose them?\"" );
	cmd( "pack $d.t.txt1 $d.t.txt2 $d.t.txt3" );
	cmd( "pack $d.l $d.t -padx 5 -pady 5" );

	cmd( "frame $d.b" );
	cmd( "button $d.b.last -width $butWid -text Last -command { set choice 1 }" );
	cmd( "button $d.b.choose -width $butWid -text Choose -command { set choice 2 }" );
	cmd( "pack $d.b.last $d.b.choose -padx 10 -side left" );
	cmd( "pack $d.b" );

	cmd( "helpcancel $d b2 { LsdHelp menudata_objn.html#pick_remove } { set choice 3 }" );

	cmd( "showtop $d" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "destroytop $d" );
	
	if ( *choice == 3 )
		return;

	if ( *choice == 1 )
	{
		for ( i = 1, cur = *r; i < desired && cur != NULL; ++i, cur = go_brother( cur ) );
		for ( ; go_brother( cur ) != NULL; )
			go_brother( cur )->delete_obj( );
	}
	else
	{ 
		Tcl_LinkVar( inter, "value", ( char * ) &value, TCL_LINK_INT );
		Tcl_LinkVar( inter, "j", ( char * ) &j, TCL_LINK_INT );
		del = new int[ actual - desired ];
		cmd( "set conf 0" );

		cmd( "newtop $d \"Choose Instances\" { set choice 2 }" );

		cmd( "frame $d.l" );
		cmd( "label $d.l.l1 -text \"Object:\"" );
		cmd( "label $d.l.l2 -fg red -text \"%s\"", ( *r )->label );
		cmd( "pack $d.l.l1 $d.l.l2 -side left" );

		cmd( "frame $d.t" );
		cmd( "label $d.t.tit -text \"Instance to delete\"", (*r)->label );
		cmd( "if [ string equal [ info tclversion ] 8.6 ] { ttk::spinbox $d.t.e -width 6 -from 1 -to 9999 -validate focusout -validatecommand { if [ string is integer -strict %%P ] { set value %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value; return 0 } } -invalidcommand { bell } -justify center } { entry $d.t.e -width 6 -validate focusout -vcmd { if [ string is integer -strict %%P ] { set value %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value; return 0 } } -invcmd { bell } -justify center }" );
		cmd( "label $d.t.tit1 -text \"\"" );
		cmd( "pack $d.t.tit $d.t.e $d.t.tit1" );
		cmd( "pack $d.l $d.t -padx 5 -pady 5" );

		cmd( "okhelpcancel $d b { set choice 1 } { LsdHelp menudata_objn.html#pick_remove } { set choice 2 }" );
		cmd( "bind $d.t.e <KeyPress-Return> { set choice 1 }" );

		cmd( "showtop $d" );
		cmd( "focus $d.t.e" );
		cmd( "$d.t.e selection range 0 end" );

		value = 1;
		for ( last = 0, j = 1; j <= actual-desired && last <= actual; ++j )
		{
			do
			{
				cmd( "$d.t.tit1 configure -text \"([ expr $j - 1 ] instance(s) done, %d to do)\"", actual - desired - j + 1 );
				cmd( "write_any $d.t.e $value" ); 
				cmd( "$d.t.e selection range 0 end" );
				cmd( "focus $d.t.e" );

				*choice = 0;
				while ( *choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "set value [ $d.t.e get ]" ); 

				if ( *choice == 2 )
					goto end;

				del[ j - 1 ] = value;
			}
			while ( last >= value );
			last = value;
			++value;
		}

		for ( j = 1, value = 0, cur = *r, app = go_brother( cur ); cur != NULL && j <= actual && value < actual - desired; ++j, cur = app, app = go_brother( cur ) )
			if ( j == del[ value ] )
			{
				if ( cur == *r )
					*r = go_brother( cur );
				cur->delete_obj( );
				++value;      
			} 

		end:
		
		cmd( "destroytop $d" );
		Tcl_UnlinkVar( inter, "value" );
		Tcl_UnlinkVar( inter, "j" );
		*choice = 0;
	}
}


/***************************************************
CHECK_PIPPO
****************************************************/
int check_pippo( object *c, object *pivot, int level, int pippo[ ] )
{
	int res = 1, i, j;
	object *cur, *cur1;

	for ( cur = c->up, i = 1; i <= level && res == 1; ++i, cur = cur->up )
	{
		// don't check if it is in Root or if there is no constraint
		if ( pippo[ i ] != -1 && cur->up != NULL )	
		{
			cur1 = cur->up->search( cur->label );
			for ( j = 1; cur1 != cur; cur1 = cur1->next, ++j );	// find the id of cur
			if ( j != pippo[ i ] )
				res = 0;
		}
	}
	
	return res;
}


/***************************************************
CHG_OBJ_NUM
****************************************************/
void chg_obj_num( object **c, int value, int level, int pippo[ ], int *choice, int cfrom )
{
	object *cur, *cur1, *last, *app, *first, *pivot;
	int cazzo, i;

	for ( cur = *c; cur->up != NULL; cur = cur->up); //goto root
	
	// select the object example
	for ( first = cur->search( ( *c )->label), i = 1; i < cfrom && first != NULL; first = first->hyper_next( first->label ), ++i );
	if ( first == NULL )
	{
		cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Object instance not found\" -detail \"Instance %d of object '%s' not found.\"", cfrom, ( *c )->label );
		return;
	}
	 
	// select the pivot 
	for ( i = 0, pivot = *c; i < level; ++i )
		pivot = pivot->up;
	
	// select the first object of the type to change under pivot
	cur = pivot->search( ( *c )->label );
	  
	while ( cur != NULL )
	{	// as long as necessary
		if ( pippo == NULL || check_pippo( cur, pivot, level, pippo ) == 1 )
		{
			skip_next_obj( cur, &cazzo ); 	// count the existing objects
			
			if ( cazzo <= value )             
				// add objects
				cur->up->add_n_objects2(first->label,value-cazzo,first); //add the necessary num of objects
			else
			{ 	// remove objects
				if ( level == 1 ) 	// you have the option to choose the items to be removed, only if you operate on one group
				{
					eliminate_obj( &cur, cazzo, value, choice );
					*c = cur;
				} 
				else
				{	// remove automatically the excess of objects
					for ( i = 1, cur1 = cur; i < value; ++i, cur1 = cur1->next );
					while ( go_brother( cur1 ) != NULL )
					{ 
						app = cur1->next->next;
						cur1->next->empty( );
						delete cur1->next;
						cur1->next = app;
					}
				}
			}
		}// end check_pippo
		
		for ( last = NULL, cur1 = cur; cur1 != NULL; cur1 = go_brother( cur1 ) )
			last = cur1 ; 	// skip the just updated group of objects
		
		if ( last == NULL )
			cur = NULL;
		else
		{
			cur = last->hyper_next( cur->label );	// first copy of the object to change after the just adjusted bunch

			if ( level > 0 && cur != NULL )
			{	//search the next pivot
				for ( cur1 = cur->up, i = 1; i < level; ++i, cur1 = cur1->up );	// hopefully, cur1 is of type pivot
				
				// a new set of objects to change has been found, but descends from another pivo
				if ( cur1 != pivot )
					cur = NULL;
			}
			else
				cur = NULL;
		}
	}
}
