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

- void insert_obj_num( object *r, char *tag, char *indent, int counter, int *idx, int *value );
Does the real job. Scan the model from root recursively and for each Object found
counts the number, prepare its index if the parent has multiple instances,
and set the indentation. Each label is bound to return a unique integer number
in case it is clicked. Such number is used as guide for the following function

- void edit_str( object *r, char *tag, int *idx, int res, int *choice, int *done );
Explore recursively the model tree giving a unique number for every group of
objects encountered. When it finds the one clicked by user prepare the
window to accept a new value for the number of instances. Passes this value
to the next function

- void chg_obj_num( object **c, int value, int all, int *choice );
Depending on all (the flag to modify all the values of that type in the model)
changes only the number of instances following c, or otherwise, every group of
instances of the type of c. If it has to increase the number of instances,
it does it directly. If it has to decrease, checks again all. If all is false,
it activate the routine below, otherwise, it eliminates directly the surplus

- void eliminate_obj( object **c, int actual, int desired , int *choice );
Ask the user whether he wants to eliminate the last object or to choose
individually the ones to eliminate. In this second case, it asks for a list
numbers. The list is as long as are the instances to eliminate. Each element
is the ordinal number of one instance to eliminate
*************************************************************/

#include "decl.h"

bool hid_level;
int level;
int lowest_level;
int max_depth;


/***************************************************
SET_OBJ_NUMBER
****************************************************/
void set_obj_number( object *r, int *choice )
{
	bool notShown = true;
	char lab[ MAX_ELEM_LENGTH + 1 ];
	int idx, res, count, done;

	Tcl_LinkVar( inter, "idx", ( char * ) &idx, TCL_LINK_INT );
	Tcl_LinkVar( inter, "val", ( char * ) &count, TCL_LINK_INT );
	Tcl_LinkVar( inter, "result", ( char * ) &res, TCL_LINK_INT );
	Tcl_LinkVar( inter, "hid_level", ( char * ) &hid_level, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( inter, "max_depth", ( char * ) &max_depth, TCL_LINK_INT );

	level = lowest_level = 1;
	max_depth = 0;							// start with all levels open
	
	cmd( "newtop .inin \"%s%s - LSD Object Number Editor\" { set choice 1; set result -1 }", unsaved_change( ) ? "*" : " ", simul_name );

	cmd( "ttk::frame .inin.obj" );
	cmd( "set f .inin.obj" );
	cmd( "ttk::scrollbar $f.scroll -command \"$f.list yview\"" );
	cmd( "ttk::scrollbar $f.scrollh -command \"$f.list xview\" -orient horizontal" );
	cmd( "set t $f.list" );
	cmd( "ttk::text $t -yscrollcommand \"$f.scroll set\" -xscrollcommand \"$f.scrollh set\" -wrap none -entry 0 -dark $darkTheme" );

	cmd( "pack $f.scroll -side right -fill y" );
	cmd( "pack $f.scrollh -side bottom -fill x" );  
	cmd( "pack $f.list -fill both -expand yes" );
	cmd( "pack $f -fill both -expand yes" );
	cmd( "mouse_wheel $t" );

	cmd( "set ininMsg \"\"" );
	cmd( "ttk::label .inin.msglab -textvariable ininMsg" );
	cmd( "pack .inin.msglab -pady 5" );
  
	cmd( "ttk::frame .inin.l" );
	cmd( "ttk::label .inin.l.tmd -text \"Show level:\"" );
	cmd( "ttk::label .inin.l.emd -width 2 -style hl.TLabel" );
	cmd( "ttk::button .inin.l.mn -width 2 -text \"-\" -command { if { $max_depth > 1 } { incr max_depth -1; set choice 4 } }" );
	cmd( "ttk::button .inin.l.pl -width 2 -text \"+\" -command { if { $hid_level } { incr max_depth; set choice 4 } }" );
	cmd( "pack .inin.l.tmd .inin.l.emd .inin.l.mn .inin.l.pl -side left" );
	cmd( "pack .inin.l -anchor e -padx 10 -pady 5" );

	cmd( "donehelp .inin b { set choice 1; set result -1 } { LsdHelp menudata_objn.html }" );
	  
	cmd( "bind .inin <minus> { .inin.l.mn invoke }" );
	cmd( "bind .inin <plus> { .inin.l.pl invoke }" );
	cmd( "bind .inin <Escape> { set choice 1 }" );
	cmd( "bind .inin <F1> { LsdHelp menudata_objn.html }" );

	while ( true )
	{
		cmd( "$t configure -state normal" );
		cmd( "$t delete 0.0 end" );
		cmd( "set ininMsg \"\"" );
		
		idx = 0;
		hid_level = false;
		cmd( "set ininWid 0" );
		cmd( "set ininHgt 0" );
		insert_obj_num( r, "", "", &idx, &count );
		
		if ( notShown )
		{
			cmd( "showtop .inin topleftW 1 1 1 [ expr max( $ininWid + 10, $hsizeNmin ) ] [ expr max( $ininHgt + 110, $vsizeNmin ) ]" );
			cmd( "wm minsize .inin $hsizeNmin $vsizeNmin" );
			notShown = false;
		}
		
		cmd( "$t configure -state disabled" );
		
		if ( max_depth < 1 )
		{
			hid_level = false;
			max_depth = lowest_level - 1;	// start with all levels open
		}

		cmd( ".inin.l.emd configure -text \"$max_depth \"" );
		cmd( "if [ info exists ininLastY ] { $t yview moveto $ininLastY; unset ininLastY }" );
		
		noredraw:

		// editor command loop
		*choice = 0;
		while ( ! *choice )
			Tcl_DoOneEvent( 0 );
		
		if ( *choice == 1 )
			break;
		
		cmd( "set ininLastY [ lindex [ $t yview ] 0 ]" );

		if ( *choice == 2 )
		{
			idx = 0;
			done = 0;
			edit_str( r, "", &idx, res, choice, &done );
			*choice = 2;
			
			if ( done == 2 )
				goto noredraw;
		}

		if ( *choice == 3 )
		{
			if ( Tcl_GetVar( inter, "obj_name", 0 ) != NULL )
			{
				strncpy( lab, ( char * ) Tcl_GetVar( inter, "obj_name", 0 ), MAX_ELEM_LENGTH );
				edit_data( r, choice, lab );
			}
			goto noredraw;
		}
	}

	cmd( "destroytop .inin" );

	Tcl_UnlinkVar( inter, "idx" );
	Tcl_UnlinkVar( inter, "val" );
	Tcl_UnlinkVar( inter, "result" );
	Tcl_UnlinkVar( inter, "hid_level" );
	Tcl_UnlinkVar( inter, "max_depth" );
}


/***************************************************
INSERT_OBJ_NUM
****************************************************/
void insert_obj_num( object *r, const char *tag, const char *ind, int *idx, int *count )
{
	char sInd[ ] = "    \u2219    ", newTag[ strlen( tag ) + MAX_ELEM_LENGTH + 15 ], newInd[ strlen( ind ) + strlen( sInd ) + 1 ];
	int i, num;
	bridge *cb; 
	object *cur;

	strcpy( newTag, tag );
	strcpy( newInd, ind );

	if ( r->up != NULL )
		strcat( newInd, sInd );

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{  
		if ( cb->head == NULL )
			continue;
		
		*idx += 1;
		
		for ( cur = cb->head, num = 0; cur != NULL; cur = cur->next, ++num );

		// just update if already exists
		cmd( "set val [ winfo exists $t.val$idx ]" );
		
		if ( *count )
		{
			*count = num;
			cmd( "$t.val$idx configure -text $val" );
		}
		else
		{
			*count = num;
			
			cmd( "ttk::button $t.but$idx -text Change -width -1 -style small.TButton -command { set result %d; set num $count%d; set choice 2 }", *idx, *idx );
			cmd( "pack $t.but$idx" );

			cmd( "ttk::label $t.ind$idx -text \"   %s   \"", newInd );
			cmd( "pack $t.ind$idx" );

			cmd( "ttk::label $t.lab$idx -style boldSmall.TLabel -text \"%s  \"", cb->head->label );
			cmd( "pack $t.lab$idx" );

			cmd( "set count$idx $val" );
			cmd( "ttk::label $t.val$idx -style hlBoldSmall.TLabel -text \"($val instance%s)\"", num == 1 ? "" : "s" );
			cmd( "pack $t.val$idx" );
			
			if ( cb->head->up != NULL && strlen( tag ) != 0 )
				cmd( "ttk::label $t.tag$idx -text \"  in   %s %s \"", cb->head->up->label, tag );
			else
				cmd( "ttk::label $t.tag$idx" );

			cmd( "pack $t.tag$idx" );

			cmd( "$t window create end -window $t.but$idx" );
			cmd( "$t window create end -window $t.ind$idx" );
			cmd( "$t window create end -window $t.lab$idx" );
			cmd( "$t window create end -window $t.val$idx" );
			cmd( "$t window create end -window $t.tag$idx" );
			cmd( "mouse_wheel $t.but$idx" );
			cmd( "mouse_wheel $t.ind$idx" );
			cmd( "mouse_wheel $t.lab$idx" );
			cmd( "mouse_wheel $t.tag$idx" );
			cmd( "mouse_wheel $t.val$idx" );
			
			cmd( "$t insert end \\n" );
			
			cmd( "set ininWid [ expr max( $ininWid, [ winfo reqwidth $t.but$idx ] + [ winfo reqwidth $t.ind$idx ] + [ winfo reqwidth $t.lab$idx ] + [ winfo reqwidth $t.val$idx ] + [ winfo reqwidth $t.tag$idx ] ) ]" );
			cmd( "set ininHgt [ expr $ininHgt + max( [ winfo reqheight $t.but$idx ], [ winfo reqheight $t.ind$idx ], [ winfo reqheight $t.lab$idx ], [ winfo reqheight $t.val$idx ], [ winfo reqheight $t.tag$idx ] ) ]" );

			if ( level >= max_depth && cb->head->b != NULL )
				hid_level = true;

			cmd( "bind $t.lab$idx <Button-1> { set obj_name %s; set choice 3 }", cb->head->label );
			cmd( "bind $t.ind$idx <Enter> { set ininMsg \"'%s' at level %d\" }", cb->head->label, level );
			cmd( "bind $t.ind$idx <Leave> { set ininMsg \"\" }" );
			cmd( "bind $t.but$idx <Enter> { set ininMsg \"Change number of instances '%s'\" }", cb->head->label );
			cmd( "bind $t.but$idx <Leave> { set ininMsg \"\" }" );
			cmd( "bind $t.lab$idx <Enter> { set ininMsg \"Click to change initial values of '%s'\" }", cb->head->label );
			cmd( "bind $t.lab$idx <Leave> { set ininMsg \"\" }" );
		}
		
		if ( max_depth < 1 || level < max_depth )
		{
			for ( i = 1, cur = cb->head; cur != NULL; ++i, cur = go_brother( cur ) )
			{
				++level;
				lowest_level = level > lowest_level ? level : lowest_level;
				
				if ( strlen( tag ) != 0 && cb->head->up != NULL )
					sprintf( newTag, "#%d - %s %s", i, cb->head->up->label, tag );
				else
					sprintf( newTag, "#%d", i );
				
				insert_obj_num( cur, newTag, newInd, idx, count );
				
				--level;
			}
		}
	}
}


/***************************************************
EDIT_STR
****************************************************/
void edit_str( object *r, const char *tag, int *idx, int res, int *choice, int *done )
{
	char newTag[ strlen( tag ) + 10 ];
	int i;
	bridge *cb;
	object *cur;

	strcpy( newTag, tag );
	
	for ( cb = r->b; cb != NULL && *done == 0; cb = cb->next )
	{ 
		if ( cb->head == NULL )
			continue;
		
		*idx += 1;
		
		if ( *idx == res )
			*done = entry_new_objnum( cb->head, tag, choice );

		for ( i = 1, cur = cb->head; cur != NULL && *done == 0; ++i, cur = go_brother( cur ) )
		{
			if ( strlen( tag ) != 0 )
				sprintf( newTag, "%s-%d", tag, i );
			else
				sprintf( newTag, "%d", i );
			
			if ( level < max_depth )
			{
				level++;
				edit_str( cur, newTag, idx, res, choice, done );
				level--;
			} 
		}
	}
}


/***************************************************
ENTRY_NEW_OBJNUM
****************************************************/
int entry_new_objnum( object *c, const char *tag, int *choice )
{  
	int i, j, k, num, cfrom, max_level;
	object *cur, *first;
	
	if ( c->up == NULL )
		return 2;

	skip_next_obj( c->up->search( c->label ), &num );
	cmd( "set num %d", num );
	cmd( "set conf 0" );
	cmd( "set cfrom 1" );

	cmd( "if [ winfo exists .inin ] { set p .inin } { set p . }" );
	cmd( "if { $p != \".\" } { set T $p.numinst } { set T .numinst }" );
	cmd( "newtop $T \"Number of Instances\" { set conf 1; set choice 2 } $p" );

	cmd( "ttk::frame $T.l" );

	cmd( "ttk::frame $T.l.n1" );
	cmd( "ttk::label $T.l.n1.l1 -text \"Object:\"" );
	cmd( "ttk::label $T.l.n1.l2 -text \"%s\" -style hl.TLabel", c->label );
	cmd( "pack $T.l.n1.l1 $T.l.n1.l2 -side left" );

	cmd( "ttk::frame $T.l.n2" );
	cmd( "ttk::label $T.l.n2.l1 -text \"Contained in:\"" );
	cmd( "ttk::label $T.l.n2.l2 -style hl.TLabel -text \"%s %s\"", c->up->label, tag );
	cmd( "pack $T.l.n2.l1 $T.l.n2.l2 -side left" );

	cmd( "pack $T.l.n1 $T.l.n2" );

	cmd( "ttk::frame $T.e" );
	
	cmd( "ttk::label $T.e.l -text \"Number of instances\"" );
	cmd( "ttk::spinbox $T.e.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= 9999 } { set num %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num; return 0 } } -invalidcommand { bell } -justify center" );
	cmd( "pack $T.e.l $T.e.e -side left -padx 2" );

	cmd( "ttk::frame $T.cp" );
	cmd( "ttk::label $T.cp.l -text \"Copy from instance\"" );
	cmd( "ttk::spinbox $T.cp.e -width 5 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= %d } { set cfrom %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cfrom; return 0 } } -invalidcommand { bell } -justify center", num, num );
	cmd( "ttk::button $T.cp.compute -width $butWid -text Compute -command \"set conf 1; set choice 3; $T.cp.e selection range 0 end; focus $T.cp.e\"" );
	cmd( "pack $T.cp.l $T.cp.e $T.cp.compute -side left -padx 2" );

	cmd( "ttk::frame $T.ef" );
	cmd( "ttk::label $T.ef.l -text \"Modify groups\"" );

	cmd( "ttk::frame $T.ef.g -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	
	for ( j = 1, cur = c->up; cur->up != NULL; cur = cur->up, j++ )
	{
		if ( j == 1 )
		{
			first = cur->up->search( cur->label );
			for ( k = 1; first != cur; first = go_brother( first ), ++k );
			cmd( "set affect 1.%d", k );
			cmd( "ttk::radiobutton $T.ef.g.r1 -text \"This group of '%s' contained in '%s' #%d\" -variable affect -value 1.%d", c->label, cur->label, k, k );
		}
		else
		{
			first = cur->up->search( cur->label );
			for ( k = 1; first != cur; first = go_brother( first ), ++k );
			cmd( "ttk::radiobutton $T.ef.g.r%d -text \"All groups of '%s' contained in '%s' #%d\" -variable affect -value %d.%d", j, c->label, cur->label, k, j, k );
		}
		cmd( "pack $T.ef.g.r%d -anchor w", j );
	}
	
	cmd( "ttk::radiobutton $T.ef.g.r%d -text \"All groups of '%s' in the model\" -variable affect -value %d.1", j, c->label, j );
	cmd( "pack $T.ef.g.r%d -anchor w", j );

	max_level = j;
	if ( j == 1 ) // we are dealing with root's descendants
		cmd( "set affect 1.1" );

	cmd( "pack $T.ef.l $T.ef.g" );
	   
	cmd( "pack $T.l $T.e $T.cp $T.ef -pady 5 -padx 5" );

	cmd( "okhelpcancel $T b { set conf 1; set choice 1 } { LsdHelp menudata_objn.html#modifyNumberObj } { set conf 1; set choice 2 }" );
	cmd( "bind $T.e.e <Return> { set conf 1; set choice 1 }" );

	cmd( "showtop $T" );
	cmd( "mousewarpto $T.b.ok" );

	j = 1;
	 
	objec_num:

	cmd( "write_any $T.e.e $num" ); 
	cmd( "write_any $T.cp.e $cfrom" ); 

	if ( j == 1 )
	{
		cmd( "$T.e.e selection range 0 end" );
		cmd( "focus $T.e.e" );
		j = 0;
	}

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );
		  
	cmd( "set num [ $T.e.e get ]" ); 
	cmd( "set cfrom [ $T.cp.e get ]" ); 

	k = *choice;
	cmd( "set choice $conf" );
	if ( *choice == 0 )
		goto objec_num;
	else
		*choice = k;  

	if ( *choice == 3 )
	{
		k = compute_copyfrom( c, choice, "$T" );
		if ( k > 0 )
			cmd( "set cfrom %d", k );
		
		cmd( "set conf 0" );
		goto objec_num;
	} 

	cmd( "destroytop $T" );  
			 
	if ( *choice == 2 )
		return 2;
	
	cfrom = get_int( "cfrom" );
	num = get_int( "num" );
	
	cmd( "set choice [ lindex [ split $affect . ] 0 ]" );
	j = *choice;
	cmd( "set choice [ lindex [ split $affect . ] 1 ]" );
	k = *choice;

	int affected[ max_level + 1 ];
	for ( i = 1; i <= max_level; ++i )
		affected[ i ] = ( i == j ) ? k : -1;

	chg_obj_num( &c, num, j, affected, choice, cfrom );

	unsaved_change( true );				// signal unsaved change
	redrawRoot = redrawStruc = true;	// update list boxes & structure

	return 1;
}


/***************************************************
COMPUTE_COPYFROM
****************************************************/
int compute_copyfrom( object *c, int *choice, const char *parWnd )
{
	object *cur, *cur1, *cur2, *cur3;
	int i, j, k, h, res;

	if ( c == NULL || c->up == NULL )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Element in Root object\" -detail \"The Root object is always single-instanced, so any element contained in it has only one instance.\"" );
		return 1;
	}

	cmd( "set conf 0" );

	cmd( "set cc %s.compcopy", parWnd );
	cmd( "newtop $cc \"Instance Number\" { set cconf 1; set choice 1 } %s", parWnd );

	cmd( "ttk::frame $cc.l" );
	cmd( "ttk::label $cc.l.l -justify center -text \"Determine the effective instance number of '%s'\nby computing the instance numbers of the containing objects.\nPress 'Done' to use the number and continue.\"", c->label );
	cmd( "pack $cc.l.l" );

	cmd( "ttk::frame $cc.f" );

	for ( i = 1, j = 1, cur = c; cur->up != NULL; cur = cur->up, ++j ) 
	{
		cmd( "ttk::frame $cc.f.f%d", j );
		cmd( "ttk::label $cc.f.f%d.l -text \"Instance # of '%s'\"", j, cur->label );
		cmd( "ttk::entry $cc.f.f%d.e -width 5 -textvariable num%d -justify center", j, j );
		cmd( "pack $cc.f.f%d.l $cc.f.f%d.e -side left -padx 2", j, j );

		for ( i = 1, cur1 = cur->up->search( cur->label ); cur1 != cur; cur1 = cur1->next, ++i );

		cmd( "set num%d %d", j, i );
	}
	  
	cmd( "focus $cc.f.f%d.e; $cc.f.f%d.e selection range 0 end", j - 1, j - 1 );

	for ( --j, cur = c; cur->up != NULL; cur = cur->up, --j )
	{	// pack in inverse order
		cmd( "pack $cc.f.f%d -pady 2 -anchor e", j );
		cmd( "bind $cc.f.f%d.e <Return> \"focus $cc.f.f%d.e; $cc.f.f%d.e selection range 0 end\"", j, j - 1, j - 1 );
	}

	cmd( "bind $cc.f.f1.e <Return> \"focus $cc.b.comp\"" );

	cmd( "ttk::frame $cc.r" );
	cmd( "ttk::label $cc.r.l -text \"Global instance number:\"" );
	cmd( "ttk::label $cc.r.res -style hl.TLabel -text %d", i );
	cmd( "pack $cc.r.l $cc.r.res -side left -padx 2" );

	cmd( "pack $cc.l $cc.f $cc.r -pady 5 -padx 5" );

	cmd( "comphelpdone $cc b { set cconf 1; set choice 2 } { LsdHelp menudata_objn.html#compute } { set cconf 1; set choice 1 }" );

	cmd( "showtop $cc" );
	cmd( "mousewarpto $cc.b.comp" );

	res = i;

	ccompute:
	
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
	   
	cfrom:

	cmd( "set ccfrom 0" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	i = *choice;
	cmd( "set choice $cconf" );
	if ( *choice == 0 )
		goto cfrom;
	else
		*choice = i;  

	if ( *choice == 2 )
		goto ccompute;

	cmd( "destroytop $cc" );
	
	return res;
}


/***************************************************
CHG_OBJ_NUM
****************************************************/
void chg_obj_num( object **c, int value, int level, int affected[ ], int *choice, int cfrom )
{
	int i, num;
	object *cur, *cur1, *cur2, *first, *last, *pivot;

	for ( cur = *c; cur->up != NULL; cur = cur->up );		// go to root
	
	// select the object example
	for ( first = cur->search( ( *c )->label ), i = 1; i < cfrom && first != NULL; first = first->hyper_next( first->label ), ++i );
	
	if ( first == NULL )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Object instance not found\" -detail \"Instance %d of object '%s' not found.\"", cfrom, ( *c )->label );
		return;
	}
	 
	// select the pivot 
	for ( i = 0, pivot = *c; i < level; ++i )
		pivot = pivot->up;
	
	// select the first object of the type to change under pivot
	cur = pivot->search( ( *c )->label );
	  
	while ( cur != NULL )
	{	// as long as necessary
		if ( affected == NULL || check_affected( cur, pivot, level, affected ) == 1 )
		{
			skip_next_obj( cur, &num ); 	// count the existing objects
			
			if ( num <= value )             
				// add objects
				cur->up->add_n_objects2( first->label, value - num, first ); //add the necessary num of objects
			else
			{ 	// remove objects
				if ( level == 1 ) 	// you have the option to choose the items to be removed, only if you operate on one group
				{
					eliminate_obj( &cur, num, value, choice );
					*c = cur;
				} 
				else
				{	// remove automatically the excess of objects
					for ( i = 1, cur1 = cur; i < value; ++i, cur1 = cur1->next );
					while ( go_brother( cur1 ) != NULL )
					{ 
						cur2 = cur1->next->next;
						cur1->next->delete_obj( );
						cur1->next = cur2;
					}
				}
			}
		}
		
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


/***************************************************
ELIMINATE_OBJ
****************************************************/
void eliminate_obj( object **c, int actual, int desired, int *choice )
{
	int i, idx2, val2, last, *del;
	object *cur, *cur1;

	cmd( "if [ winfo exists .inin ] { set p .inin } { set p . }" );
	cmd( "if { $p != \".\" } { set d $p.delobj } { set d .delobj }" );
	cmd( "newtop $d \"Delete Instances\" { set choice 3 } $p" );
	cmd( "set conf 0" );

	cmd( "ttk::frame $d.l" );
	cmd( "ttk::label $d.l.l1 -text \"Object:\"" );
	cmd( "ttk::label $d.l.l2 -style hl.TLabel -text \"%s\"", ( *c )->label );
	cmd( "pack $d.l.l1 $d.l.l2 -side left" );

	cmd( "ttk::frame $d.t" );
	cmd( "ttk::label $d.t.txt1 -text \"Do you want to delete the last\"" );

	cmd( "ttk::label $d.t.txt2 -text \"%d instances(s)\" -style hl.TLabel", actual - desired );
	cmd( "ttk::label $d.t.txt3 -text \"or you want to choose them?\"" );
	cmd( "pack $d.t.txt1 $d.t.txt2 $d.t.txt3" );
	cmd( "pack $d.l $d.t -padx 5 -pady 5" );

	cmd( "ttk::frame $d.b" );
	cmd( "ttk::button $d.b.last -width $butWid -text Last -command { set choice 1 }" );
	cmd( "ttk::button $d.b.choose -width $butWid -text Choose -command { set choice 2 }" );
	cmd( "pack $d.b.last $d.b.choose -padx 10 -side left" );
	cmd( "pack $d.b" );

	cmd( "helpcancel $d b2 { LsdHelp menudata_objn.html#pick_remove } { set choice 3 }" );

	cmd( "showtop $d" );
	cmd( "mousewarpto $d.b.last" );

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "destroytop $d" );
	
	if ( *choice == 3 )
		return;

	if ( *choice == 1 )
	{
		for ( i = 1, cur = *c; i < desired && cur != NULL; ++i, cur = go_brother( cur ) );
		for ( ; go_brother( cur ) != NULL; )
			go_brother( cur )->delete_obj( );
	}
	else
	{ 
		Tcl_LinkVar( inter, "val2", ( char * ) &val2, TCL_LINK_INT );
		Tcl_LinkVar( inter, "idx2", ( char * ) &idx2, TCL_LINK_INT );
		del = new int[ actual - desired ];
		cmd( "set conf 0" );

		cmd( "newtop $d \"Choose Instances\" { set choice 2 } $p" );

		cmd( "ttk::frame $d.l" );
		cmd( "ttk::label $d.l.l1 -text \"Object:\"" );
		cmd( "ttk::label $d.l.l2 -style hl.TLabel -text \"%s\"", ( *c )->label );
		cmd( "pack $d.l.l1 $d.l.l2 -side left" );

		cmd( "ttk::frame $d.t" );
		cmd( "ttk::label $d.t.tit -text \"Instance to delete\"", ( *c )->label );
		cmd( "ttk::spinbox $d.t.e -width 6 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= %d } { set val2 %%P; return 1 } { %%W delete 0 end; %%W insert 0 $val2; return 0 } } -invalidcommand { bell } -justify center", actual, actual );
		cmd( "ttk::label $d.t.tit1 -text \"\"" );
		cmd( "pack $d.t.tit $d.t.e $d.t.tit1" );
		cmd( "pack $d.l $d.t -padx 5 -pady 5" );

		cmd( "okhelpcancel $d b { set choice 1 } { LsdHelp menudata_objn.html#pick_remove } { set choice 2 }" );
		cmd( "bind $d.t.e <Return> { set choice 1 }" );

		cmd( "showtop $d" );
		cmd( "focus $d.t.e" );
		cmd( "$d.t.e selection range 0 end" );

		val2 = 1;
		for ( last = 0, idx2 = 1; idx2 <= actual - desired && last <= actual; ++idx2 )
		{
			do
			{
				cmd( "$d.t.tit1 configure -text \"([ expr $idx2 - 1 ] instance(s) done, %d to do)\"", actual - desired - idx2 + 1 );
				cmd( "write_any $d.t.e $val2" );
				cmd( "$d.t.e selection range 0 end" );
				cmd( "focus $d.t.e" );

				*choice = 0;
				while ( *choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "set val2 [ $d.t.e get ]" ); 

				if ( *choice == 2 )
					goto end;

				for ( i = 0; i < idx2 - 1; ++i )
					if ( del[ i ] == val2 )
						continue;
					
				del[ idx2 - 1 ] = val2;
			}
			while ( last >= val2 );
			
			last = val2;
			++val2;
		}

		for ( idx2 = 1, val2 = 0, cur = *c, cur1 = go_brother( cur ); cur != NULL && idx2 <= actual && val2 < actual - desired; ++idx2, cur = cur1, cur1 = go_brother( cur ) )
			if ( idx2 == del[ val2 ] )
			{
				if ( cur == *c )
					*c = go_brother( cur );
				
				cur->delete_obj( );
				++val2;      
			} 

		end:
		
		cmd( "destroytop $d" );
		Tcl_UnlinkVar( inter, "val2" );
		Tcl_UnlinkVar( inter, "idx2" );
		*choice = 0;
	}
}


/***************************************************
CHECK_AFFECTED
****************************************************/
int check_affected( object *c, object *pivot, int level, int affected[ ] )
{
	int i, j, res;
	object *cur, *cur1;

	for ( i = 1, res = 1, cur = c->up; i <= level && res == 1; ++i, cur = cur->up )
	{
		// don't check if it is in Root or if there is no constraint
		if ( affected[ i ] != -1 && cur->up != NULL )	
		{
			cur1 = cur->up->search( cur->label );
			for ( j = 1; cur1 != cur; cur1 = cur1->next, ++j );	// find the id of cur
			if ( j != affected[ i ] )
				res = 0;
		}
	}
	
	return res;
}
