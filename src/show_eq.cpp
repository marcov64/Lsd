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
SHOW_EQ.CPP 
Show one window containig the equation for the label clicked on.

Less simple as it seems, given that it has to deal with all weird characters
like parenthesis, quotes, brakets, that tk commands consider as special
characters. The basic trick is that it loads one line per time. If it finds
the name of the variable following the command "strcmp" than starts sending the
line to be printed. Lines are printed character per character, so that it can
deal with special characters. While printing it computes the number of parenthesis
open and closed, and when it meets the last parenthesis exits.

Everything is within just one single function:

- void show_eq( char *lab )

- void scan_used_lab( char *lab, int *choice )
Looks in the equation file whether the variable or parameter lab is contained
in some equations. It creates a window containing the list of the equations
using in any way the variable indicated. By clicking on the names in the
list the code for that variable is shown.
It is based on the recognition of the string lab between quotes, thus any function
is recognized.
*************************************************************/

#include "decl.h"


/****************************************************
SHOW_EQ
****************************************************/
void show_eq( char *lab, int *choice )
{
	bool done;
	char c1_lab[ MAX_LINE_SIZE ], c2_lab[ MAX_LINE_SIZE ], c3_lab[ MAX_LINE_SIZE ], full_name[ 2 * MAX_PATH_LENGTH ], updt_in[ MAX_ELEM_LENGTH + 1 ], *app, *fname;
	int i, k, bra, start, printing_var = 0, comment_line = 0, temp_var = 0;
	FILE *f1, *f2;

	cmd( "if [ string compare [ info command .eq_%s ] .eq_%s ] { set ex yes } { set ex no }", lab, lab );
	app= ( char * ) Tcl_GetVar( inter, "ex", 0 );
	strcpy( msg, app );
	if ( strcmp( msg, "yes" ) )
		return;

	// define the correct parent window
	cmd( "switch %d { 0 { set parWnd . } 1 { set parWnd .chgelem } 2 { set parWnd .da } 3 { set parWnd .deb } default { set parWnd . } }", *choice );

	start:
	
	fname = equation_name;
	sprintf( full_name, "%s/%s", exec_path, fname );
	if ( ( f1 = fopen( full_name, "r" ) ) == NULL )
	{
		cmd( "set answer [ ttk::messageBox -parent . -type okcancel -default ok -icon error -title Error -message \"Equation file not found\" -detail \"Check equation file name '%s' and press 'OK' to search it.\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } }", equation_name  );
		cmd( "if { $choice == 1 } { set res [ tk_getOpenFile -parent . -title \"Load Equation File\" -initialdir \"%s\" -filetypes { { {LSD Equation Files} {.cpp} } { {All Files} {*} } } ]; if [ fn_spaces \"$res\" . ] { set res \"\" } { set res [ file tail $res ] } }", exec_path );

		if ( *choice == 1 )
		{
			app = ( char * ) Tcl_GetVar( inter, "res", 0 );
			if ( app == NULL || strlen( app ) == 0 )
				return;
			
			strncpy( equation_name, app, MAX_PATH_LENGTH - 1 );
			
			goto start;
		}
		else
			return;
	}
	else
		fclose( f1 );

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", exec_path );
	cmd( "if { [ lsearch -exact $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", equation_name, equation_name );
	cmd( "set choice [ llength $source_files ]" );
	
	for ( done = false, k = 0; done == false && k < *choice; ++k )
	{
		cmd( "set brr [ lindex $source_files %d ]", k );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );
		if ( ( f2 = fopen( fname, "r" ) ) == NULL )
			continue;
		
		while ( ! done && fgets( c1_lab, MAX_LINE_SIZE - 1, f2 ) != NULL )
			if ( is_equation_header( c1_lab, c2_lab, updt_in ) )
				if ( ! strcmp( c2_lab, lab ) )
					done = true;
				
		if ( ! done )
			fclose( f2 );
	}

	if ( ! done )
	{
		cmd( "ttk::messageBox -parent $parWnd -type ok -icon error -title Error -message \"Equation not found\" -detail \"Equation for '%s' not found (check the spelling or equation file name).\"", lab  );
		return;
	}

	cmd( "if { [ string equal $parWnd . ] } { \
			set w .eq_%s \
		} else { \
			set w $parWnd.eq_%s \
		}", lab, lab );
	cmd( "set _W_%s $w", lab );
	cmd( "set s \"\"" );
	
	cmd( "newtop $w \"'%s' %s Equation (%s)\" \"destroytop $w; focus $parWnd\" $parWnd", lab, eq_dum ? "Dummy" : "", fname );

	cmd( "ttk::frame $w.f" );
	cmd( "ttk::scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
	cmd( "ttk::scrollbar $w.f.xscroll -orient horiz -command \"$w.f.text xview\"" );
	cmd( "ttk::text $w.f.text -wrap none -tabstyle wordprocessor -yscrollcommand \"$w.f.yscroll set\" -xscrollcommand \"$w.f.xscroll set\" -entry 0 -dark $darkTheme -style smallFixed.TText" );
	cmd( "mouse_wheel $w.f.text" );
	cmd( "settab $w.f.text $tabsize smallFixed.TText" );
	cmd( "pack $w.f.yscroll -side right -fill y" );
	cmd( "pack $w.f.xscroll -side bottom -fill x" );
	cmd( "pack $w.f.text -expand yes -fill both" );
	cmd( "pack $w.f -expand yes -fill both" );
	cmd( "findhelpdone $w b { \
			set W $_W_%s; \
			set cur1 [ $W.f.text index insert ]; \
			newtop $W.s \"\" { destroytop $W.s } $W; \
			ttk::label $W.s.l -text \"Find\"; \
			ttk::entry $W.s.e -textvariable s -justify center; \
			ttk::button $W.s.b -width $butWid -text OK -command { \
				destroytop $W.s; \
				set cur1 [ $W.f.text search -count length $s $cur1 ]; \
				if { [ string length $cur1 ] > 0 } { \
					$W.f.text tag remove sel 1.0 end; \
					$W.f.text tag add sel $cur1 \"$cur1 + $length char\"; \
					$W.f.text mark set insert \"$cur1 + $length char\"; \
					focus $W.f.text; \
					$W.f.text see $cur1; \
					update idletasks \
				} \
			}; \
			pack $W.s.l $W.s.e -padx 5; \
			pack $W.s.b -padx $butPad -pady $butPad -side right; \
			bind $W.s <KeyPress-Return> { $W.s.b invoke }; \
			showtop $W.s centerW; \
			$W.s.e selection range 0 end; \
			focus $W.s.e \
		} { LsdHelp equation.html } \"destroytop $w; focus $parWnd\"", lab  );
	cmd( "bind $w <Control-f> { $_W_%s.b.search invoke }; bind $w <Control-F> { $_W_%s.b.search invoke }", lab, lab );
	cmd( "bind $w <F3> { \
			set W $_W_%s; \
			set cur1 [ $W.f.text index insert ]; \
			set cur1 [ $W.f.text search -count length $s $cur1 ]; \
			if { [ string length $cur1 ] > 0 } { \
				$W.f.text tag remove sel 1.0 end; \
				$W.f.text tag add sel $cur1 \"$cur1 + $length char\"; \
				$W.f.text mark set insert \"$cur1 + $length char\"; \
				focus $W.f.text; \
				$W.f.text see $cur1; \
				update idletasks \
			} \
		}", lab );

	cmd( "bind $w.f.text <KeyPress-Prior> { $_W_%s.f.text yview scroll -1 pages }", lab );
	cmd( "bind $w.f.text <KeyPress-Next> { $_W_%s.f.text yview scroll 1 pages }", lab );
	cmd( "bind $w.f.text <KeyPress-Up> { $_W_%s.f.text yview scroll -1 units }", lab );
	cmd( "bind $w.f.text <KeyPress-Down> { $_W_%s.f.text yview scroll 1 units }", lab );
	cmd( "bind $w.f.text <KeyPress-Left> { $_W_%s.f.text xview scroll -1 units }", lab );
	cmd( "bind $w.f.text <KeyPress-Right> { $_W_%s.f.text xview scroll 1 units }", lab );

	cmd( "bind $w.f.text <Double-1> { \
			set W $_W_%s; \
			$W.f.text tag remove sel 1.0 end; \
			set a @%%x,%%y; \
			$W.f.text tag add sel \"$a wordstart\" \"$a wordend\"; \
			set res [ $W.f.text get sel.first sel.last ]; \
			set choice 29 \
		}", lab );

	cmd( "showtop $w centerW 1 1" );
	cmd( "mousewarpto $w.b.cancel" );

	cmd( "$w.f.text tag conf vars -foreground $colorsTheme(str)" );
	cmd( "$w.f.text tag conf comment_line -foreground $colorsTheme(comm)" );
	cmd( "$w.f.text tag conf temp_var -foreground $colorsTheme(vlsd)" );

	cmd( "set mytag \"\"" );

	if ( ! macro )
	{	// standard type of equations
		start = 1;
		bra = 1;
	}
	else
	{
		start = 0;
		bra = 2;
	}
	
	strcpy( c3_lab, c1_lab );						// save original first line
			
	do
	{	
		strcpy( c2_lab, c1_lab );
		clean_spaces( c2_lab );
		
		// handle dummy equations without RESULT closing
		if ( eq_dum && strcmp( c1_lab, c3_lab ) && ( ! strncmp( c2_lab, "EQUATION(", 9 ) || ! strncmp( c2_lab, "EQUATION_DUMMY(", 15 ) || ! strncmp( c2_lab, "FUNCTION(", 9 ) || ! strncmp( c2_lab, "MODELEND", 8 ) ) )
		{
			if ( strlen( updt_in ) > 0 )
				cmd( "$w.f.text insert end \"\n(DUMMY EQUATION: variable '%s' updated in '%s')\"", lab, updt_in );
			else
				cmd( "$w.f.text insert end \"\n(DUMMY EQUATION: variable '%s' not updated here)\"", lab );
				
			break;
		}
	
		if ( ! strncmp( c2_lab,"RESULT(", 7 ) )
			bra--;

		for ( i = 0; c1_lab[ i ] != 0; ++i )
		{
			if ( c1_lab[ i ] == '\r' ) 
			i++;
			if ( c1_lab[ i ] == '{' )
			{
				if ( bra != 1 )
					cmd( "$w.f.text insert end \"{\" $mytag" );
				else
					start = 0;
				bra++;
			}
			else
				if ( c1_lab[ i ] == '}' )
				{
					bra--;
					if ( bra > 1 )
						cmd( "$w.f.text insert end \"}\" $mytag " );
				}
				else
					if ( c1_lab[ i ] == '\\' )
						cmd( "$w.f.text insert end \\\\ $mytag" );
					else
						if ( c1_lab[ i ] == '[' )
							cmd( "$w.f.text insert end \\[ $mytag " );
						else
							if ( c1_lab[ i ] == ']' )
							{
								cmd( "$w.f.text insert end \\]  $mytag" );
								if ( temp_var == 1 )
								{
									temp_var = 0;
									cmd( "set mytag \"\"" );
								}
							}
							else
								if ( c1_lab[ i ] == '"' )
								{
									if ( printing_var == 1 && comment_line == 0 )
										cmd( "set mytag \"\"" );

									cmd( "$w.f.text insert end {\"} $mytag" );
									if ( printing_var == 0 && comment_line == 0 )
									{
										cmd( "set mytag \"vars\"" );
										printing_var = 1;
									}
									else
										printing_var = 0;
								}
								else
									if ( c1_lab[ i ] == '/' && c1_lab[ i + 1 ] == '/' && comment_line == 0 )
									{
										cmd( "set mytag comment_line" );
										comment_line = 1;
										cmd( "$w.f.text insert end \"//\" $mytag" );
										i++;
									}
									else
										if ( c1_lab[ i ] == '/' && c1_lab[ i + 1 ] == '*' && comment_line == 0 )
										{
											cmd( "set mytag comment_line" );
											comment_line = 2;
											cmd( "$w.f.text insert end \"/*\" $mytag" );
											i++;
										}
										else
											if ( c1_lab[ i ] == '*' && c1_lab[ i + 1 ] == '/' && comment_line == 2 )
											{
												comment_line = 0;
												cmd( "$w.f.text insert end \"*/\" $mytag" );
												i++;
												cmd( "set mytag \"\"" );
											}
											else
												if ( c1_lab[ i ] == 'v' && c1_lab[ i + 1 ] == '[' && comment_line == 0 )
												{
													temp_var = 1;
													cmd( "set mytag temp_var" );
													cmd( "$w.f.text insert end \"v\" $mytag" );
												}
												else
													if ( c1_lab[ i ] != '\n' )
														cmd( "$w.f.text insert end \"%c\"  $mytag", c1_lab[ i ] );
													else
													{
														cmd( "$w.f.text insert end \\n  $mytag" );
														if ( comment_line == 1 )
														{
															cmd( "set mytag \"\"" );
															comment_line = 0;
														}
													}
		}
	}
	while ( ( bra > 1 || start == 1 ) && fgets( c1_lab, MAX_LINE_SIZE, f2 ) != NULL  );
	
	fclose( f2 );

	cmd( "$w.f.text mark set insert 1.0" );
	cmd( "$w.f.text conf -state disabled" );
}


/****************************************************
SCAN_USED_LAB
****************************************************/
void scan_used_lab( char *lab, int *choice )
{
	bool exist, no_window;
	char c1_lab[ MAX_LINE_SIZE ], c2_lab[ MAX_LINE_SIZE ], *fname;
	int i, j, k, nfiles, done, caller = *choice;
	FILE *f;

	no_window = ( *choice == -1 ) ? true : false;

	// define the correct parent window
	cmd( "if { %d == 1 } { \
			set list .chgelem.listusing_%s \
		} else { \
			set list .listusing_%s \
		}", caller, lab, lab );

	if ( ! no_window )
	{
		cmd( "if [ winfo exists $list ] { set choice 1 } { set choice 0 }" );
		if ( *choice == 1 )
			return;
		
		cmd( "newtop $list \"Used In\" \"destroytop $list\""  );

		cmd( "ttk::frame $list.lf " );
		cmd( "ttk::label $list.lf.l1 -text \"Equations using\"" );
		cmd( "ttk::label $list.lf.l2 -style hl.TLabel -text \"%s\"", lab );
		cmd( "pack $list.lf.l1 $list.lf.l2" );

		cmd( "ttk::frame $list.l" );
		cmd( "ttk::scrollbar $list.l.v_scroll -command \"$list.l.l yview\"" );
		cmd( "ttk::listbox $list.l.l -width 25 -selectmode single -yscroll \"$list.l.v_scroll set\" -dark $darkTheme" );
		cmd( "pack $list.l.l  $list.l.v_scroll -side left -fill y" );
		cmd( "mouse_wheel $list.l.l" );

		cmd( "bind $list.l.l <Home> \"selectinlist $list.l.l 0; break\"" );
		cmd( "bind $list.l.l <End> \"selectinlist $list.l.l end; break\"" );

		if ( caller != 1 )
			cmd( "ttk::label $list.l3 -justify center -text \"(double-click to\nobserve the element)\"" );
		else
			cmd( "ttk::label $list.l3" );

		cmd( "pack $list.lf $list.l $list.l3 -padx 5 -pady 5 -expand yes -fill both" );

		cmd( "done $list b \"destroytop $list\"" );		// done button
	}

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", exec_path );
	cmd( "if { [ lsearch -exact $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", equation_name, equation_name );
	cmd( "set res [ llength $source_files ]" );
	nfiles = get_int( "res" );
	
	cmd( "unset -nocomplain list_used" );

	for ( exist = false, k = 0; k < nfiles; ++k )
	{
		cmd( "set brr [ lindex $source_files %d ]", k );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );
		
		if ( ( f = fopen( fname, "r" ) ) != NULL )
		{
			strcpy( c1_lab, "" );
			strcpy( c2_lab, "" );

			for ( done = 0; fgets( c1_lab, MAX_LINE_SIZE, f ) != NULL;  )
			{
				clean_spaces( c1_lab ); 	// eliminate the spaces
				
				for ( i = 0; c1_lab[ i ] != '"' && c1_lab[ i ] !=  '\0' ; ++i )
					c2_lab[ i ] = c1_lab[ i ];
				
				c2_lab[ i ] = '\0'; 			// close the string
				
				if ( ! strcmp( c2_lab, "if(!strcmp(label," ) || ! strcmp( c2_lab, "EQUATION(" ) || ! strcmp( c2_lab, "EQUATION_DUMMY(" ) || ! strcmp( c2_lab, "FUNCTION(" ) )
				{
					if ( ! strcmp( c2_lab, "if(!strcmp(label," ) )
						macro = false;
					else
						macro = true;
					
					for ( j = 0; c1_lab[ i + 1 + j ] != '"'; ++j )
						c2_lab[ j ] = c1_lab[ i + 1 + j ]; 	// prepare the c2_lab to store the var's label			
					c2_lab[ j ] = '\0';
					
					done = contains( f, lab, strlen( lab ) );
					if ( done == 1 )
					{
						if ( no_window )
							cmd( "lappend list_used %s", c2_lab );
						else
							cmd( "$list.l.l insert end %s", c2_lab );
						exist = true;
					}
				}
			}
			
			fclose( f );
		}
	}
	
	if ( no_window )
	{
		cmd( "if [ info exists list_used ] { set list_used [ join $list_used \", \" ] } { set list_used \"(never used)\" }" );
		return;
	}

	if ( exist )
	{
		if ( caller != 1 )
			cmd( "bind $list <Double-Button-1> { set bidi [ selection get ]; set done 8; set choice 55 }" );
	}
	else
		cmd( "$list.l.l insert end \"(never used)\"" );

	cmd( "showtop $list" );
	cmd( "mousewarpto $list.b.ok" );
}


/****************************************************
SCAN_USING_LAB
****************************************************/
void scan_using_lab( char *lab, int *choice )
{
	bool found = false;
	int caller = *choice;
	variable *cv;

	// define the correct parent window
	cmd( "if { %d == 1 } { \
			set list .chgelem.listusing_%s \
		} else { \
			set list .listusing_%s \
		}", caller, lab, lab );

	cmd( "if [ winfo exists $list ] { set choice 1 } { set choice 0 }" );
	if ( *choice == 1 )
		return;

	cmd( "newtop $list \"Using\" \"destroytop $list\"" );

	cmd( "ttk::frame $list.lf " );
	cmd( "ttk::label $list.lf.l1 -justify center -text \"Elements used in\"" );
	cmd( "ttk::label $list.lf.l2 -style hl.TLabel -text \"%s\"", lab );
	cmd( "pack $list.lf.l1 $list.lf.l2" );

	cmd( "ttk::frame $list.l" );
	cmd( "ttk::scrollbar $list.l.v_scroll -command \"$list.l.l yview\"" );
	cmd( "ttk::listbox $list.l.l -width 25 -selectmode single -yscroll \"$list.l.v_scroll set\" -dark $darkTheme" );
	cmd( "pack $list.l.l $list.l.v_scroll -side left -fill y" );
	cmd( "mouse_wheel $list.l.l" );

	cmd( "bind $list.l.l <Home> \"selectinlist $list.l.l 0; break\"" );
	cmd( "bind $list.l.l <End> \"selectinlist $list.l.l end; break\"" );

	if ( caller != 1 )
		cmd( "ttk::label $list.l3 -justify center -text \"(double-click to\nobserve the element)\"" );
	else
		cmd( "ttk::label $list.l3" );

	cmd( "pack $list.lf $list.l $list.l3 -padx 5 -pady 5 -expand yes -fill both" );

	cmd( "done $list b \"destroytop $list\"" );		// done button

	cv = root->search_var( root, lab );
	find_using( root, cv, NULL, & found );
	
	cmd( "set choice [ $list.l.l size ]" );
	if ( *choice != 0 )
	{
		if ( caller != 1 )
			cmd( "bind $list <Double-Button-1> {set bidi [selection get]; set choice 55; set done 8}" );
	}
	else
		cmd( "$list.l.l insert end \"(none)\"" );

	cmd( "showtop $list" );
	cmd( "mousewarpto $list.b.ok" );
}


/****************************************************
SHOW_DESCR
****************************************************/
void show_descr( char *lab, int *choice )
{
	char buf_descr[ TCL_BUFF_STR + 1 ];
	description *cd;
	variable *cv;
	
	// define the correct parent window
	cmd( "switch %d { 0 { set parWnd . } 1 { set parWnd .chgelem } 2 { set parWnd .da } 3 { set parWnd .deb } default { set parWnd . } }", *choice );
	
	cv = root->search_var( NULL, lab );
	if ( cv == NULL )
		return;
	
	cd = search_description( lab );

	cmd( "if { [ string equal $parWnd . ] } { \
			set w .desc_%s \
		} else { \
			set w $parWnd.desc_%s \
		}", lab, lab );

	cmd( "newtop $w \"Element '%s' in Object '%s'\" \"destroytop $w; focus $parWnd\" $parWnd", lab, cv->up->label );

	cmd( "ttk::frame $w.f" );
	cmd( "ttk::label $w.f.l -text Description" );
	cmd( "ttk::frame $w.f.d" );
	cmd( "ttk::scrollbar $w.f.d.yscroll -command \"$w.f.d.text yview\"" );
	cmd( "ttk::text $w.f.d.text -wrap word -width 60 -height 8 -yscrollcommand \"$w.f.d.yscroll set\" -entry 0 -dark $darkTheme -style smallFixed.TText" );
	cmd( "mouse_wheel $w.f.d.text" );
	cmd( "pack $w.f.d.yscroll -side right -fill y" );
	cmd( "pack $w.f.d.text -expand yes -fill both" );
	cmd( "pack $w.f.l $w.f.d" );
	cmd( "pack $w.f -expand yes -fill both -pady 5" );
	
	if ( ( cv->param == 1 || cv->num_lag > 0 ) && cd->init != NULL )
	{
		cmd( "ttk::frame $w.i" );
		cmd( "ttk::label $w.i.l -text \"Initial values\"" );
		cmd( "ttk::frame $w.i.d" );
		cmd( "ttk::scrollbar $w.i.d.yscroll -command \"$w.i.d.text yview\"" );
		cmd( "ttk::text $w.i.d.text -wrap word -width 60 -height 3 -yscrollcommand \"$w.i.d.yscroll set\" -entry 0 -dark $darkTheme -style smallFixed.TText" );
		cmd( "mouse_wheel $w.i.d.text" );
		cmd( "pack $w.i.d.yscroll -side right -fill y" );
		cmd( "pack $w.i.d.text -expand yes -fill both" );
		cmd( "pack $w.i.l $w.i.d" );
		cmd( "pack $w.i -expand yes -fill both -pady 5" );
	}
	
	cmd( "done $w b \"destroytop $w; focus $parWnd\"" );

	cmd( "showtop $w centerW 1 1" );
	cmd( "mousewarpto $w.b.ok" );
	
	cmd( "$w.f.d.text insert end \"%s\"", strtcl( buf_descr, cd->text, TCL_BUFF_STR ) );
	cmd( "$w.f.d.text conf -state disabled" );

	if ( ( cv->param == 1 || cv->num_lag > 0 ) && cd->init != NULL )
	{
		cmd( "$w.i.d.text insert end \"%s\"", strtcl( buf_descr, cd->init, TCL_BUFF_STR ) );
		cmd( "$w.i.d.text conf -state disabled" );
	}
}
