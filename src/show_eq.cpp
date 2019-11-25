/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
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
	FILE *f;

	cmd( "if [ string compare [ info command .eq_%s ] .eq_%s ] { set ex yes } { set ex no }", lab, lab );
	app= ( char * ) Tcl_GetVar( inter, "ex", 0 );
	strcpy( msg, app );
	if ( strcmp( msg, "yes" ) )
		return;

	// define the correct parent window
	cmd( "switch %d { 0 { set parWnd . } 1 { set parWnd .chgelem } 2 { set parWnd .da } 3 { set parWnd .deb } }", *choice );

	start:
	fname = equation_name;
	sprintf( full_name, "%s/%s", exec_path, fname );
	if ( ( f = fopen( full_name, "r" ) ) == NULL )
	{
		cmd( "set answer [ tk_messageBox -parent . -type okcancel -default ok -icon error -title Error -message \"Equation file not found\" -detail \"Check equation file name '%s' and press 'OK' to search it.\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } }", equation_name  );
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
		fclose( f );

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", exec_path );
	cmd( "if { [ lsearch -exact $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", equation_name, equation_name );
	cmd( "set choice [ llength $source_files ]" );
	
	for ( done = false, k = 0; done == false && k < *choice; ++k )
	{
		cmd( "set brr [ lindex $source_files %d ]", k );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );
		if ( ( f = fopen( fname, "r" ) ) == NULL )
			continue;
		
		while ( ! done && fgets( c1_lab, MAX_LINE_SIZE - 1, f ) != NULL )
			if ( is_equation_header( c1_lab, c2_lab, updt_in ) )
				if ( ! strcmp( c2_lab, lab ) )
					done = true;
	}

	if ( ! done )
	{
		if ( f != NULL )
			fclose( f );
		cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Equation not found\" -detail \"Equation for '%s' not found (check the spelling or equation file name).\"", lab  );
		return;
	}

	cmd( "set w .eq_%s", lab );
	cmd( "set s \"\"" );
	cmd( "newtop $w \"'%s' %s Equation (%s)\" { destroytop .eq_%s } $parWnd", lab, eq_dum ? "Dummy" : "", fname, lab );

	cmd( "frame $w.f" );
	cmd( "scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
	cmd( "scrollbar $w.f.xscroll -orient horiz -command \"$w.f.text xview\"" );
	cmd( "text $w.f.text -font \"$font_small\" -wrap none -tabstyle wordprocessor -yscrollcommand \"$w.f.yscroll set\" -xscrollcommand \"$w.f.xscroll set\"" );
	cmd( "settab $w.f.text $tabsize \"$font_small\"" );
	cmd( "pack $w.f.yscroll -side right -fill y" );
	cmd( "pack $w.f.xscroll -side bottom -fill x" );
	cmd( "pack $w.f.text -expand yes -fill both" );
	cmd( "pack $w.f -expand yes -fill both" );
	cmd( "findhelpdone $w b { \
			set W .eq_%s; \
			set cur1 [ $W.f.text index insert ]; \
			newtop $W.s \"\" { destroytop $W.s } $W; \
			label $W.s.l -text \"Find\"; \
			entry $W.s.e -textvariable s -justify center; \
			button $W.s.b -width $butWid -text OK -command { \
				destroytop $W.s; \
				set cur1 [ $W.f.text search -count length $s $cur1 ]; \
				if { [ string length $cur1 ] > 0 } { \
					$W.f.text tag remove sel 1.0 end; \
					$W.f.text tag add sel $cur1 \"$cur1 + $length char\"; \
					$W.f.text mark set insert \"$cur1 + $length char\"; \
					focus $W.f.text; \
					$W.f.text see $cur1; \
					update \
				} \
			}; \
			pack $W.s.l $W.s.e -padx 5; \
			pack $W.s.b -padx 10 -pady 10; \
			bind $W.s <KeyPress-Return> { $W.s.b invoke }; \
			showtop $W.s centerS; \
			$W.s.e selection range 0 end; \
			focus $W.s.e \
		} { LsdHelp equation.html } { destroytop .eq_%s }", lab, lab  );
	cmd( "bind .eq_%s <Control-f> { .eq_%s.b.search invoke }; bind .eq_%s <Control-F> { .eq_%s.b.search invoke }", lab, lab, lab, lab );
	cmd( "bind .eq_%s <F3> { \
			set W .eq_%s; \
			set cur1 [ $W.f.text index insert ]; \
			set cur1 [ $W.f.text search -count length $s $cur1 ]; \
			if { [ string length $cur1 ] > 0 } { \
				$W.f.text tag remove sel 1.0 end; \
				$W.f.text tag add sel $cur1 \"$cur1 + $length char\"; \
				$W.f.text mark set insert \"$cur1 + $length char\"; \
				focus $W.f.text; \
				$W.f.text see $cur1; \
				update \
			} \
		}", lab, lab );

	cmd( ".eq_%s.f.text tag conf vars -foreground blue4", lab );
	cmd( ".eq_%s.f.text tag conf comment_line -foreground green4", lab );
	cmd( ".eq_%s.f.text tag conf temp_var -foreground red4", lab );

	cmd( "set mytag \"\"" );

	if ( ! macro )
	{	//standard type of equations
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
				cmd( ".eq_%s.f.text insert end \"\n(DUMMY EQUATION: variable '%s' updated in '%s')\"", lab, lab, updt_in );
			else
				cmd( ".eq_%s.f.text insert end \"\n(DUMMY EQUATION: variable '%s' not updated here)\"", lab, lab );
				
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
					cmd( ".eq_%s.f.text insert end \"{\" $mytag", lab );
				else
					start = 0;
				bra++;
			}
			else
				if ( c1_lab[ i ] == '}' )
				{
					bra--;
					if ( bra > 1 )
						cmd( ".eq_%s.f.text insert end \"}\" $mytag ", lab );
				}
				else
					if ( c1_lab[ i ] == '\\' )
						cmd( ".eq_%s.f.text insert end \\\\ $mytag", lab );
					else
						if ( c1_lab[ i ] == '[' )
							cmd( ".eq_%s.f.text insert end \\[ $mytag ", lab );
						else
							if ( c1_lab[ i ] == ']' )
							{
								cmd( ".eq_%s.f.text insert end \\]  $mytag", lab );
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

									cmd( ".eq_%s.f.text insert end {\"} $mytag", lab );
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
										cmd( ".eq_%s.f.text insert end \"//\" $mytag", lab );
										i++;
									}
									else
										if ( c1_lab[ i ] == '/' && c1_lab[ i + 1 ] == '*' && comment_line == 0 )
										{
											cmd( "set mytag comment_line" );
											comment_line = 2;
											cmd( ".eq_%s.f.text insert end \"/*\" $mytag", lab );
											i++;
										}
										else
											if ( c1_lab[ i ] == '*' && c1_lab[ i + 1 ] == '/' && comment_line == 2 )
											{
												comment_line = 0;
												cmd( ".eq_%s.f.text insert end \"*/\" $mytag", lab );
												i++;
												cmd( "set mytag \"\"" );
											}
											else
												if ( c1_lab[ i ] == 'v' && c1_lab[ i + 1 ] == '[' && comment_line == 0 )
												{
													temp_var = 1;
													cmd( "set mytag temp_var" );
													cmd( ".eq_%s.f.text insert end \"v\" $mytag", lab );
												}
												else
													if ( c1_lab[ i ] != '\n' )
														cmd( ".eq_%s.f.text insert end \"%c\"  $mytag",lab, c1_lab[ i ] );
													else
													{
														cmd( ".eq_%s.f.text insert end \\n  $mytag", lab );
														if ( comment_line == 1 )
														{
															cmd( "set mytag \"\"" );
															comment_line = 0;
														}
													}
		}
	}
	while ( ( bra > 1 || start == 1 ) && fgets( c1_lab, MAX_LINE_SIZE, f ) != NULL  );
	
	fclose( f );

	cmd( ".eq_%s.f.text mark set insert 1.0", lab );

	cmd( "bind .eq_%s.f.text <KeyPress-Prior> {.eq_%s.f.text yview scroll -1 pages}", lab, lab );
	cmd( "bind .eq_%s.f.text <KeyPress-Next> {.eq_%s.f.text yview scroll 1 pages}", lab, lab );
	cmd( "bind .eq_%s.f.text <KeyPress-Up> {.eq_%s.f.text yview scroll -1 units}", lab, lab );
	cmd( "bind .eq_%s.f.text <KeyPress-Down> {.eq_%s.f.text yview scroll 1 units}", lab, lab );
	cmd( "bind .eq_%s.f.text <KeyPress-Left> {.eq_%s.f.text xview scroll -1 units}", lab, lab );
	cmd( "bind .eq_%s.f.text <KeyPress-Right> {.eq_%s.f.text xview scroll 1 units}", lab, lab );

	cmd( "bind .eq_%s.f.text <Double-1> {.eq_%s.f.text tag remove sel 1.0 end; set a @%%x,%%y; .eq_%s.f.text tag add sel \"$a wordstart\" \"$a wordend\"; set res [.eq_%s.f.text get sel.first sel.last]; set choice 29 }", lab, lab, lab, lab );

	cmd( "showtop $w centerS 1 1" );

	cmd( ".eq_%s.f.text conf -state disabled", lab );
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

	cmd( "set list .list_%s", lab );

	if ( ! no_window )
	{
		cmd( "if [ winfo exists $list ] { set choice 1 } { set choice 0 }" );
		if ( *choice == 1 )
			return;
		
		cmd( "newtop $list \"Used In\" { destroytop .list_%s }", lab  );

		cmd( "frame $list.lf " );
		cmd( "label $list.lf.l1 -text \"Equations using\"" );
		cmd( "label $list.lf.l2 -fg red -text \"%s\"", lab );
		cmd( "pack $list.lf.l1 $list.lf.l2" );

		cmd( "frame $list.l" );
		cmd( "scrollbar $list.l.v_scroll -command \".list_%s.l.l yview\"", lab );
		cmd( "listbox $list.l.l -width 25 -selectmode single -yscroll \".list_%s.l.v_scroll set\"", lab );
		cmd( "pack $list.l.l  $list.l.v_scroll -side left -fill y" );
		cmd( "mouse_wheel $list.l.l" );

		if ( caller != 1 )
			cmd( "label $list.l3 -text \"(double-click to\\nobserve the element)\"" );
		else
			cmd( "label $list.l3" );

		cmd( "pack $list.lf $list.l $list.l3 -padx 5 -pady 5 -expand yes -fill both" );

		cmd( "done $list b { destroytop .list_%s }", lab );		// done button
	}

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", exec_path );
	cmd( "if { [ lsearch -exact $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", equation_name, equation_name );
	cmd( "set res [ llength $source_files ]" );
	get_int( "res", & nfiles );
	
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
			cmd( "bind $list <Double-Button-1> {set bidi [ selection get ]; set done 8; set choice 55}" );
	}
	else
		cmd( "$list.l.l insert end \"(never used)\"" );

	cmd( "showtop $list" );
}


/****************************************************
SCAN_USING_LAB
****************************************************/
void scan_using_lab( char *lab, int *choice )
{
	int caller = *choice;
	variable *cv;

	cmd( "set list .listusing_%s", lab );

	cmd( "if [ winfo exists $list ] { set choice 1 } { set choice 0 }" );
	if ( *choice == 1 )
		return;

	cmd( "newtop $list \"Using\" { destroytop .listusing_%s }", lab  );

	cmd( "frame $list.lf " );
	cmd( "label $list.lf.l1 -justify center -text \"Elements used in\"" );
	cmd( "label $list.lf.l2 -fg red -text \"%s\"", lab );
	cmd( "pack $list.lf.l1 $list.lf.l2" );

	cmd( "frame $list.l" );
	cmd( "scrollbar $list.l.v_scroll -command \".listusing_%s.l.l yview\"", lab );
	cmd( "listbox $list.l.l -width 25 -selectmode single -yscroll \".listusing_%s.l.v_scroll set\"", lab );
	cmd( "pack $list.l.l $list.l.v_scroll -side left -fill y" );
	cmd( "mouse_wheel $list.l.l" );

	if ( caller != 1 )
		cmd( "label $list.l3 -text \"(double-click to\\nobserve the element)\"" );
	else
		cmd( "label $list.l3" );

	cmd( "pack $list.lf $list.l $list.l3 -padx 5 -pady 5 -expand yes -fill both" );

	cmd( "done $list b { destroytop .listusing_%s }", lab );		// done button

	cv = root->search_var( root, lab );
	find_using( root, cv, NULL );
	
	cmd( "set choice [ $list.l.l size ]" );
	if ( *choice != 0 )
	{
		if ( caller != 1 )
			cmd( "bind $list <Double-Button-1> {set bidi [selection get]; set choice 55; set done 8}" );
	}
	else
		cmd( "$list.l.l insert end \"(none)\"" );

	cmd( "showtop $list" );
}
