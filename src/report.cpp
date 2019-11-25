/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/*************************************************************
REPORT.CPP
This file contains the code for the report generating routines. 

The main function, (report) needs the pointer to the root of the model. 
It also needs the equation file name to be correctly set (if the file 
does not exist, the routine ask for it). The output of the routine is 
a html file containing the main information on the model. This file is 
meant to be used as basic structure to be filled with modellers' comments
in order to obtain the complete documentation of the model.

The report lists all the objects, variables and parameters, all linked with 
pointers, so that readers can easily jump hypertextually through the whole report.

- Each object is listed with indication of its ancestors.
- For each object, the set of variables and parameters is listed.
- For each parameter is listed the set of variables whose equations make use of its value.
- For each variables is listed the set of variables whose equations make use of its values,
and also the whole set of variables and parameters used in the its own equation.
*************************************************************/

#include "decl.h"

#define MAX_INIT 100
#define TEX_PAPER "a4paper"
#define TEX_LEFT 2.5
#define TEX_RIGHT 2.5
#define TEX_TOP 2.5
#define TEX_BOTTOM 2.5

bool table;
int code;
int desc;
int extra;
int fatto;
int file_error;
int init;
int lmenu;
int obs;
int pos;
int tab_lines;


/******************************
REPORT
*******************************/
void report( int *choice, object *r )
{
	char *app, ch;
	FILE *f, *frep;
	
	file_error = 0;

	if ( ! struct_loaded )
	{
		cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create one before trying to create a report.\"" );
		return;
	}

	sprintf( name_rep, "report_%s.html", simul_name );
	cmd( "set mrep %s", name_rep );
	cmd( "set choice [file exists $mrep]" );
	if ( *choice == 1 )
	{
		cmd( "set answer [tk_messageBox -parent . -message \"Model report already exists\" -detail \"Please confirm overwriting it.\" -type okcancel -title Warning -icon warning -default ok]" );
		cmd( "if {[string compare -nocase $answer ok] == 0} {set choice 0} {set choice 1}" );
		if ( *choice == 1 )
			return;
	}

	Tcl_LinkVar( inter, "code", ( char * ) &code, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( inter, "init", ( char * ) &init, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( inter, "desc", ( char * ) &desc, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( inter, "extra", ( char * ) &extra, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( inter, "obs", ( char * ) &obs, TCL_LINK_BOOLEAN );

	desc = true;
	obs = true;
	init = true;
	code = true;
	extra = false;
	cmd( "set reptit %s", simul_name );
	cmd( "set lmenu 1" );
	cmd( "set html2 1" );
	cmd( "set tit2 \"Comments\"" );
	cmd( "set file2 \"comments.txt\"" );

	cmd( "newtop .w \"Report Generation\" { set choice 3 }" );

	cmd( "frame .w.f" );
	cmd( "label .w.f.l -text \"Model title\"" );
	cmd( "entry .w.f.e -width 30 -textvariable reptit -justify center" );
	cmd( "pack .w.f.l .w.f.e -side left" );

	cmd( "frame .w.l" );
	cmd( "label .w.l.l -text \"Variables presentation\"" );

	cmd( "frame .w.l.opt -bd 2 -relief groove" );
	cmd( "radiobutton .w.l.opt.popup -text \"List boxes\" -variable lmenu -value 1" );
	cmd( "radiobutton .w.l.opt.text -text \"Text lists\" -variable lmenu -value 0" );
	cmd( "pack .w.l.opt.popup .w.l.opt.text -anchor w" );

	cmd( "pack .w.l.l .w.l.opt" );

	cmd( "frame .w.g" );
	cmd( "label .w.g.l -text \"Included sections\"" );

	cmd( "frame .w.g.opt" );
	cmd( "checkbutton .w.g.opt.desc -text \"Description\" -variable desc" );
	cmd( "checkbutton .w.g.opt.extra -text \"User section\" -variable extra -command { if { $extra } { .w.s.e2.file.new conf -state normal; .w.s.e2.h conf -state normal; .w.s.e2.header.tit conf -state normal; .w.s.e2.file.tit conf -state normal } { .w.s.e2.file.new conf -state disabled; .w.s.e2.h conf -state disabled; .w.s.e2.header.tit conf -state disabled; .w.s.e2.file.tit conf -state disabled } }" );
	cmd( "checkbutton .w.g.opt.obs -text \"Selected variables\" -variable obs" );
	cmd( "checkbutton .w.g.opt.init -text \"Initial values\" -variable init" );
	cmd( "checkbutton .w.g.opt.code -text \"Equations code\" -variable code" );
	cmd( "pack .w.g.opt.desc .w.g.opt.extra .w.g.opt.obs .w.g.opt.init .w.g.opt.code -anchor w" );

	cmd( "pack .w.g.l .w.g.opt" );

	cmd( "frame .w.s" );
	cmd( "label .w.s.lab -text \"User supplied section\"" );

	cmd( "frame .w.s.e2" );
	cmd( "checkbutton .w.s.e2.h -state disabled -text \"Use HTML tags\" -variable html2" );

	cmd( "frame .w.s.e2.header" );
	cmd( "label .w.s.e2.header.tlab -text \"Title\"" );
	cmd( "entry .w.s.e2.header.tit -width 30 -state disabled -textvariable tit2 -justify center" );
	cmd( "pack .w.s.e2.header.tlab .w.s.e2.header.tit -side left -padx 2" );

	cmd( "frame .w.s.e2.file" );
	cmd( "label .w.s.e2.file.tlab -text \"Get from file\"" );
	cmd( "entry .w.s.e2.file.tit -width 25 -state disabled -textvariable file2 -justify center" );
	cmd( "button .w.s.e2.file.new -width 5 -state disabled -text Search -command { set file2 [ tk_getOpenFile -parent .w -title \"Load Description File\" -filetypes {{{All files} {*}} } -initialdir \"%s\" ]; if [ fn_spaces \"$file2\" .w ] { set file2 \"\" } }", exec_path );
	cmd( "pack .w.s.e2.file.tlab .w.s.e2.file.tit .w.s.e2.file.new -side left -padx 2" );

	cmd( "pack .w.s.e2.h .w.s.e2.header .w.s.e2.file -padx 5 -pady 2" );

	cmd( "pack .w.s.lab .w.s.e2" );

	cmd( "pack .w.f .w.l .w.g .w.s -padx 5 -pady 5" );

	cmd( "okXhelpcancel .w b Search { set res [ tk_getSaveFile -parent .w -title \"Existing Report File\" -filetypes { { {HTML files} {.html} } } -initialdir \"%s\" ]; set choice 2 } { set choice 1 } { LsdHelp menumodel.html#createreport } { set choice 3 }", exec_path );

	cmd( "showtop .w topleftW" );

	here_create_report:

	*choice = 0;
	while ( *choice == 0 )
		Tcl_DoOneEvent( 0 );

	if ( *choice == 3 )
	{
		cmd( "destroytop .w" );
		goto end;
	}

	if ( *choice == 2 )
	{ 
		app = ( char * ) Tcl_GetVar( inter, "res", 0 );
		if ( app == NULL || strlen( app ) == 0 )
			goto here_create_report;
		
		strncpy( name_rep, app, MAX_PATH_LENGTH - 1 );
	}
		
	cmd( "destroytop .w" );

	while ( strlen( equation_name ) == 0 || ( f = fopen( equation_name, "r" ) ) == NULL )
	{
		cmd( "set answer [ tk_messageBox -parent . -type okcancel -default ok -icon error -title Error -message \"Equation file '%s' not found\" -detail \"Press 'OK' to select another file.\"]; if [ string equal $answer ok ] { set res [ file tail [ tk_getOpenFile -parent . -title \"Load Equation File\" -initialdir [pwd] -filetypes { { {LSD Equation Files} {.cpp} } { {All Files} {*} } } ] ]; if [ fn_spaces \"$res\" . ] { set res \"\" }; set choice 1 } { set choice 2 }", equation_name );

		if ( *choice == 1 )
		{
			app = ( char * ) Tcl_GetVar( inter, "res", 0 );
			if ( app != NULL && strlen( app ) > 0 )
				strncpy( equation_name, app, MAX_PATH_LENGTH - 1 );
		}

		if ( *choice == 2 )
			goto end;
	}

	fclose( f );
	cmd( "set choice $lmenu" );
	lmenu = *choice;

	frep = create_frames( name_rep );

	if ( frep == NULL )
	{
		cmd( "tk_messageBox -parent . -message \"Cannot write to disk\" -detail \"Please check if the model directory is not full or READ-ONLY.\" -type ok -title Error -icon error" );
		plog( "\nError writing report.\n" );
		goto end;
	}
		
	cmd( "wm deiconify .log; raise .log; focus .log" );
	plog( "\nWriting report. Please wait... " );

	fprintf( frep, "<HTML>\n<HEAD> <META NAME=\"Author\" CONTENT=\"Automatically generated by LSD - Laboratory for Simulation Development, copyright by Marco Valente\">\n" );
	fprintf( frep, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\">  <style> table {border-collapse: collapse} td, th {border: 1px solid #dddddd; padding: 8px;} tr:nth-child(even) {background-color: #dddddd;} </style> </HEAD> <BODY>" );

	app = ( char * ) Tcl_GetVar( inter, "reptit", 0 );

	fprintf( frep, "<TITLE>LSD Report - Model: \"%s\"</TITLE>", app );
	fprintf( frep, "<I>Automatically generated LSD report.</I><BR>" );
	fprintf( frep, "<A NAME=\"_TOP_\"><H1>Model: <I><U>%s</U></I></H1></A>", app );

	if ( desc || extra || obs )
		fprintf( frep, "<A NAME=\"_DESCRIPTION_\"><H2>Description</H2></A>" );

	if ( desc )
	{
		sprintf( msg, "%s/description.txt", exec_path );
		f = fopen( msg, "r" );
		if ( f != NULL )
		{
			for ( ch = fgetc( f ); ch != EOF; ch = fgetc( f ) )
				switch ( ch )
				{
					case '\n':
						fprintf( frep, "<br>\n" );
						break;
					case '<':
						fprintf( frep, "&lt;" );
						break;
					case '>':
						fprintf( frep, "&gt;" );
						break;
					default:
						fprintf( frep, "%c", ch );
						break;
				}
				
			fclose( f );
			fprintf( frep, "<BR><BR>" );
		}
		else
		{
			fprintf( frep, "No description file available.<BR>\n" );
			plog( "\nFile description.txt not found. Description section skipped... " );
		}
	}
	 
	if ( extra )
	{
		app = ( char * ) Tcl_GetVar( inter, "file2", 0 );
		if ( app == NULL )
			plog( "\nMissing file name for user section. Skipped... " );
		else
		{
			f = fopen( app, "r" );
			if ( f != NULL )
			{
				app = ( char * ) Tcl_GetVar( inter, "tit2", 0 );
				fprintf( frep, "<H3>%s</H3>", app );
			
				cmd( "set choice $html2" );
				for ( ch = fgetc( f ); ch != EOF; ch = fgetc( f ) )
				{
					if ( *choice == 0 )
					{
						switch ( ch )
						{
							case '\n':
								fprintf( frep, "<br>\n" );
								break;
							case '<':
								fprintf( frep, "&lt;" );
								break;
							case '>':
								fprintf( frep, "&gt;" );
								break;
							default:
								fprintf( frep, "%c", ch );
								break;
						}
					} 
					else
						fprintf( frep, "%c", ch ); 
				} 
			   
				fclose( f );
				fprintf( frep, "<BR><BR>" );
			}
			else
			{
				fprintf( frep, "User section file not available.<BR>\n" );
				plog( "\nFile %s not found. User section skipped... ", "", app );
			}
		}
	}

	if ( obs )
	{
		int begin = 1;
		show_rep_initial( frep, r, &begin, frep );
		
		begin = 1;
		show_rep_observe( frep, r, &begin, frep );
	}

	fprintf( frep, "<HR WIDTH=\"100%%\">\n" );

	fprintf( frep, "<A NAME=\"_MODEL_STRUCTURE_\"><H2>Model Structure</H2></A>\n" );
	fprintf( frep, "<H3>Object tree</H3>\n" );

	write_str( r, frep, 0, "" );
	write_list( frep, r, 1, "" );
	create_table_init( r, frep );

	if ( init )
	{
		fprintf( frep, "<HR WIDTH=\"100%%\">\n" );

		fprintf( frep, "<A NAME=\"_INITIALVALUES_\"><H2>Initial values</H2></A>\n" );
		fprintf( frep, "<H3>Object tree</H3>\n" );

		write_str( r, frep, 0, "_i_" );
		write_list( frep, r, 1, "_i_" );

		create_initial_values( r, frep );
	}

	if ( code )
	{
		fprintf( frep, "<HR WIDTH=\"100%%\">\n" );
		
		fprintf( frep, "<A NAME=\"_DETAILS_\"><H2>Equations code</H2></A>\n" );
		fprintf( frep, "<H3>Object tree</H3>\n" );

		write_str( r, frep, 0, "_d_" );
		write_list( frep, r, 1, "_d_" );

		cmd( "set app [ file tail \"%s\" ]", equation_name );
		app = ( char * ) Tcl_GetVar( inter, "app", 0 );
		fprintf( frep, "<BR><i>Equation file:</i> &nbsp;<TT><u>%s</u></TT><BR><BR>", app );

		write_obj( r, frep );
	}

	fprintf( frep,"</BODY> </HTML>" );
	fclose( frep );

	plog( "Done\nReport saved in file: %s\n", "", name_rep );
	cmd( "set namerep %s", name_rep );
	cmd( "LsdHtml $namerep" );

	end:

	Tcl_UnlinkVar( inter, "code" );
	Tcl_UnlinkVar( inter, "init" );
	Tcl_UnlinkVar( inter, "desc" );
	Tcl_UnlinkVar( inter, "extra" );
}


/**********************************
WRITE_OBJ
**********************************/
void write_obj( object *r, FILE *frep )
{
	int count;
	bridge *cb;
	object *cur, *cur2;
	variable *cv;

	for ( cur = r; cur != NULL; cur = skip_next_obj( cur, &count ) )
	{
		for ( count = 0, cv = cur->v; cv != NULL; cv = cv->next )
			if ( cv->param != 1 )
				count = 1;
	  
		if ( count != 0 )	// avoid no need for code, typically root or parameters only object
		{
			fprintf( frep, "<HR WIDTH=\"100%%\">\n" );
			
			fprintf( frep, "<H3><A NAME=\"_d_%s\">Object: &nbsp;<TT><U>%s</U></TT></A></H3>\n", cur->label, cur->label );
			
			if ( cur->up != NULL )
			{
				fprintf( frep,"<i>Contained in: &nbsp;</i>" );
				ancestors( cur, frep );
				fprintf( frep, "<BR>\n" );
			}
			 
			if ( cur->b != NULL )
			{
				fprintf( frep,"<i>Containing: &nbsp;</i>" );
				fprintf( frep, "<TT><A HREF=\"#%s\">%s</A></TT>", cur->b->blabel, cur->b->blabel );
				for ( cb = cur->b->next; cb != NULL; cb = cb->next )
					fprintf( frep, "<TT>,  <A HREF=\"#%s\">%s</A></TT>", cb->blabel, cb->blabel );
				fprintf( frep, "<BR>\n" );
			}
			
			fprintf( frep, "<BR>\n" );
			write_list( frep, cur, 0, "_d_" );
			
			for ( cv = cur->v; cv != NULL; cv = cv->next )
				write_var( cv, frep );
		}
	  
		if ( cur->b != NULL )
		{
			cur2 = cur->b->head;
			write_obj( cur2, frep );
		}
	}
}


/******************************
WRITE_VAR
*******************************/
void write_var( variable *v, FILE *frep )
{
	bool one;
	char *app, *fname, c1_lab[ 2 * MAX_LINE_SIZE ], c2_lab[ 2 * MAX_LINE_SIZE ], c3_lab[ 2 * MAX_LINE_SIZE ], updt_in[ MAX_ELEM_LENGTH + 1 ];
	int i, j, k, done, flag_begin, flag_string, flag_comm, flag_var, nfiles;
	object *cur;
	FILE *ffun ;

	cmd( "update" );

	fprintf( frep, "<HR WIDTH=\"100%%\">\n" );
		
	if ( v->param == 1 )
		fprintf( frep, "<A NAME=\"_d_%s\"><H4>Parameter: &nbsp;<TT><U>%s</U></TT></H4></A>", v->label, v->label );
	if ( v->param == 0 )
		fprintf( frep, "<A NAME=\"_d_%s\"><H4>Variable: &nbsp;<TT><U>%s</U></TT></H4></A>", v->label, v->label );
	if ( v->param == 2 )
		fprintf( frep, "<A NAME=\"_d_%s\"><H4>Function: &nbsp;<TT><U>%s</U></TT></H4></A>", v->label, v->label );

	fprintf( frep, "<I>Contained in: &nbsp;</I><A HREF=\"#%s\"><TT>%s</TT></A><BR>", v->up->label, v->up->label );
	 
	fprintf( frep, "<I>Used in: &nbsp;</I>" );

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", exec_path );
	cmd( "if { [ lsearch -exact $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", equation_name, equation_name );
	cmd( "set res [ llength $source_files ]" );
	get_int( "res", &nfiles );
		
	for ( one = false, k = 0; k < nfiles; ++k )
	{
		cmd( "set brr [ lindex $source_files %d ]", k );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );

		if ( ( ffun = fopen( fname, "r" ) ) == NULL )
		{
			if ( ++file_error < ERR_LIM )
				plog( "\nError opening file '%s': %s", "", fname, strerror( errno ) );
			continue;
		}

		while ( fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun ) != NULL )
		{
			if ( is_equation_header( c1_lab, c2_lab, updt_in ) )
			{
				done = contains( ffun, v->label, strlen( v->label ) );
		
				if ( done )
				{
					fprintf( frep, "<TT>%s<A HREF=\"#_d_%s\">%s</A></TT>", one ? ", " : "", c2_lab,  c2_lab );
					one = true;
				}
			}
		}

		fclose( ffun );
	}

	if ( ! one )
		fprintf( frep, "(never used)" );

	fprintf( frep, "<BR>\n" );

	if ( v->param == 0 || v->param == 2 )
	{
		fprintf( frep,"<I>Using: &nbsp;</I>" );
		for ( cur = v->up; cur->up != NULL; cur = cur->up ); // reach root
		fatto = 0;
		find_using( cur, v, frep );
		if ( fatto == 0 )
			fprintf( frep, "(none)" );
	} 

	fprintf( frep,"<BR>\n" );

	if ( v->param == 1 )
		return;

	for ( one = false, k = 0; ! one && k < nfiles; ++k )
	{
		cmd( "set brr [ lindex $source_files %d ]", k );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );

		if ( ( ffun = fopen( fname, "r" ) ) == NULL )
		{
			if ( ++file_error < ERR_LIM )
				plog( "\nError opening file '%s': %s", "", fname, strerror( errno ) );
			continue;
		}

		flag_comm = 0;

		while ( fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun ) != NULL  )
		{
			if ( is_equation_header( c1_lab, c2_lab, updt_in ) )
			{
				if ( ! macro )
					done = 0;
				else
					done = 1; 		// will never stop with {} only
			 
				if ( ! strcmp( c2_lab, v->label ) )
				{
					fatto = 0;
					one = true;
					 
					if ( k == 0 )
						fprintf( frep,"<BR><I>Equation code:</I><BR>\n" );
					else
						fprintf( frep,"<BR><I>Equation code</I> <TT>(%s)</TT>:<BR>\n", fname );
					
					fprintf( frep, "<BR>\n<TT>%s</TT>", c1_lab );
					 
					while ( done > 0 || fatto == 0 )
					{
						fatto = 1;
						fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun );
						strcpy( c3_lab, c1_lab );
						clean_spaces( c3_lab );
						
						// handle dummy equations without RESULT closing
						if ( eq_dum && ( ! strncmp( c1_lab, "EQUATION(", 9 ) || ! strncmp( c1_lab, "EQUATION_DUMMY(", 15 ) || ! strncmp( c1_lab, "FUNCTION(", 9 ) || ! strncmp( c1_lab, "MODELEND", 8 ) ) )
						{
							if ( strlen( updt_in ) > 0 )
								sprintf( c1_lab, "(DUMMY EQUATION: variable '%s' updated in '%s')", v->label, updt_in );
							else
								sprintf( c1_lab, "(DUMMY EQUATION: variable '%s' not updated here)", v->label );
								
							done = 0;
						}
						
						app = strstr( c1_lab, "{" );
						if ( app != NULL )
							done++;
						app = strstr( c1_lab, "}" );
						if ( app != NULL )
							done--;
						flag_string = flag_var = 0;
						fprintf( frep, "<BR><TT>" );
						
						if ( flag_comm == 1 )
							fprintf( frep, "<FONT COLOR=\"#009900\">" );
						
						flag_begin = 0;
						for ( j = 0; c1_lab[ j ] != '\0'; ++j )
						{
							if ( flag_comm == 0 && flag_string == 0 && c1_lab[ j ] == '\"' )
							{
								for ( i = j + 1; c1_lab[ i ] !='\"'; ++i )
								c2_lab[ i - j - 1 ] = c1_lab[ i ];
								c2_lab[ i - j - 1 ] = '\0';
								fprintf( frep, "\"<A HREF=\"#_d_%s\">", c2_lab );
								flag_string = 1;
								j++;
							}
							
							if ( flag_comm == 0 && flag_string == 1 && c1_lab[ j ] == '\"' )
							{
								fprintf( frep, "</A>\"" );
								flag_string = 0;
								j++ ;
							}
							
							if ( flag_comm == 0 && c1_lab[ j ] == '/' && c1_lab[ j + 1 ] == '*' )
							{
								fprintf( frep, "<FONT COLOR=\"#009900\">" );
								flag_comm = 1;
							}
								
							if ( flag_comm == 1 && c1_lab[ j ] == '/' && c1_lab[j - 1 ] == '*' )
							{
								fprintf( frep, "/</FONT>" );
								flag_comm = 0;
								j++;
							}
							
							if ( flag_comm == 0 && c1_lab[ j ] == '/' && c1_lab[ j + 1 ] == '/' )
							{
								fprintf( frep, "<FONT COLOR=\"#009900\">" );
								flag_comm = 2;
							}
							
							if ( flag_comm == 0 && c1_lab[ j ] == 'v' && c1_lab[ j + 1 ] == '[' )
							{
								fprintf( frep, "<FONT COLOR=\"#FF0000\">" );
								flag_var = 1;
							}
							
							if ( flag_comm == 0 && flag_var == 1 && c1_lab[ j ] == ']' )
							{
								fprintf( frep, "]</FONT>" );
								flag_var = 0;
								j++;
							}

							if ( c1_lab[ j ] == '<' || c1_lab[ j ] == '>' )
							{
								if ( c1_lab[ j ] == '<' )
									fprintf( frep, "&lt;" );
								else
									fprintf( frep, "&gt;" );
							}
							else
								if ( c1_lab[ j ] == ' ' && flag_begin == 0 )
									fprintf( frep, "&nbsp;" );
								else
									if ( c1_lab[ j ] != '\n' )
									{
										fprintf( frep, "%c", c1_lab[ j ] );
										flag_begin = 1;
									}
									else
										if ( flag_comm == 1 )
											fprintf( frep, "</FONT>" );
						}
						  
						if ( flag_comm == 2 )
						{
							fprintf( frep, "</FONT>" );
							flag_comm = 0;
						}
                        
						fprintf( frep, "</TT>\n" );
						
						if ( ! strncmp( c3_lab, "RESULT(", 7 ) && macro )
							done = 0; 		// force it to stop
					}
					  
					fprintf( frep, "</FONT><BR><BR>\n" );
					break; 
				}
			}
		}

		fclose( ffun );
	}

	if ( ! one )
		fprintf( frep,"<BR><I>Equation code</I>: (not available)<BR><BR>\n" );
}


/******************************
FIND_USING
*******************************/
void find_using( object *r, variable *v, FILE *frep )
{
	bool one;
	int count, done, i, nfiles;
	char *fname, c1_lab[ 2 * MAX_LINE_SIZE ], c2_lab[ 2 * MAX_LINE_SIZE ], updt_in[ MAX_ELEM_LENGTH + 1 ];
	object *cur;
	variable *cv;
	FILE *ffun;

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", exec_path );
	cmd( "if { [ lsearch -exact $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", equation_name, equation_name );
	cmd( "set res [ llength $source_files ]" );
	get_int( "res", &nfiles );
	
	// first check if variable has a dummy equation and abort if so
	for ( i = 0; i < nfiles; ++i )
	{
		cmd( "set brr [ lindex $source_files %d ]", i );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );
		
		if ( ( ffun = fopen( fname, "r" ) ) == NULL )
		{
			if ( ++file_error < ERR_LIM )
				plog( "\nError opening file '%s': %s", "", fname, strerror( errno ) );
			continue;
		}
		
		while ( fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun ) != NULL )
			if ( is_equation_header( c1_lab, c2_lab, updt_in ) )
				if ( eq_dum && ! strcmp( c2_lab, v->label ) )
				{
					fclose( ffun );
					return;
				}
				
		fclose( ffun );
	}
	
	// now search for all elements in all objects in all files
	for ( one = false, cur = r; cur != NULL; cur = skip_next_obj( cur, &count ) )
	{
		for ( cv = cur->v; cv != NULL; cv = cv->next )
		{
			for ( i = 0; i < nfiles; ++i )
			{
				cmd( "set brr [ lindex $source_files %d ]", i );
				cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
				fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );

				if ( ( ffun = fopen( fname, "r" ) ) == NULL )
				{
					if ( ++file_error < ERR_LIM )
						plog( "\nError opening file '%s': %s", "", fname, strerror( errno ) );
					continue;
				}

				while ( fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun ) != NULL )
				{
					if ( is_equation_header( c1_lab, c2_lab, updt_in ) )
					{
						done = 0;
						if ( ! strcmp( c2_lab, v->label ) )
							done = contains( ffun, cv->label, strlen( cv->label ) );
						if ( done )
						{
							if ( frep != NULL )
							{
								fprintf( frep, "<TT>%s<A HREF=\"#_d_%s\">%s</A></TT>", one ? ", " : "", cv->label, cv->label );
								one = true;
							}
							else
								cmd( "$list.l.l insert end %s", cv->label );
							fatto = 1;
						}
					}
				}
				
				fclose( ffun );
			}
		}

		if ( cur->b != NULL )
			find_using( cur->b->head, v, frep );
	}
}


/****************************************************
CONTAINS
 scans an equation checking if it contains anywhere the string
 lab between quotes. Returns 1 if found, and 0 otherwise.
 The file passed is moved to point to the next equation
 It correctly skip the commented text, either by // or by / * ... * /
****************************************************/
bool contains( FILE *f, char *lab, int len )
{
	bool found = false;
	int bra, start, i, j, got, comm = 0;
	char c1_lab[ MAX_LINE_SIZE ], pot[ MAX_LINE_SIZE ];
	
	if ( ! macro )
	{
		start = 1;
		bra = 1;
	}
	else
	{
		start = 0;
		bra = 2;
	}

	// for each line of the equation ...
	while ( ( bra > 1 || start == 1 ) && fgets( c1_lab, MAX_LINE_SIZE, f ) != NULL )
	{
		if ( comm == 1 )
			comm = 0;
		
		strcpy( pot, c1_lab );
		clean_spaces( pot );
		
		if ( ! strncmp( pot, "RESULT(", 7 ) )
			bra--;
		
		for ( i = 0; c1_lab[ i ] != 0; ++i ) // scans each character
		{
			if ( c1_lab[ i ] == '{' ) 		// if it is an open bracket
			{
				bra++;
				start = 0;
			}
			else
				if ( c1_lab[ i ] == '}' ) 	// if it is a closed bracket
					bra--;
				else
				{
					if ( c1_lab[ i ] == '/' && c1_lab[ i + 1 ] == '/' )
						comm = 1;
					else
						if ( c1_lab[ i ] == '/' && c1_lab[ i + 1 ] == '*' )
							comm = 2;
						else
							if ( comm == 2 && c1_lab[ i ] == '*' && c1_lab[ i + 1 ] == '/' )
								comm = 0;
							
					if ( comm == 0 && c1_lab[ i ] == '\"' && ! found ) // if it is a quote (supposedly open quotes)
					{
						for ( j = i + 1; c1_lab[ j ] != '\"'; ++j ) // copy the characters till the last quotes
							pot[ j - i - 1 ] = c1_lab[ j ];
							
						pot[ j - i - 1 ] = c1_lab[ j ];
		
						// scan the whole word, until a different char is not found
						got = 1;
						if ( pot[ 0 ] != '\"' ) 			// in case the eq. contains "" it gets fucked up..
							for ( j = 0; got == 1 && pot[ j ] != '\"' && lab + j != NULL; ++j )
								if ( pot[ j ] != lab[ j ])
									got = 0;
						for ( ; pot[ j ] != '\"'; ++j ); 	// finishes the word, until the closed quotes
						
						i = i + j + 1;
						
						if ( got == 1 && j == len )
							found = true;
					}
				}
		}
	}
	
	return found;
}


/******************************
WRITE_STR
*******************************/
void write_str( object *r, FILE *frep, int dep, char const *prefix )
{
	int len, i, j, trash;
	bridge *cb;
	object *cur;
	
	trash = 0;
	if ( r->up != NULL )
	{
		if ( r->up->b->head != r )
			for ( i = 0; i < dep; ++i )
			{
				if ( msg[ i ] == ' ' )
					fprintf( frep, "<TT>&nbsp;</TT>" );
				else
					fprintf( frep, "<TT>|</TT>" );
			}
			
		fprintf( frep, "<TT>&mdash;&gt;</TT>" );
		cur = skip_next_obj( r, &trash );
		if ( cur != NULL )
			msg[ dep ] = '|';
		else
			msg[ dep ] = ' ';
		
		msg[ ++dep ] = ' ';
		trash = 1;
	}

	fprintf( frep, "<TT><A HREF=\"#%s%s\">%s</A></TT>", prefix, r->label, r->label );
	len = strlen( r->label );
	for ( i = 0; i < len; ++i )
		msg[ dep + i + trash ] = ' ';
	dep = dep + len + trash;
	j = dep;
	
	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		cur = cb->head;
		write_str( cur, frep, j, prefix );
	}
	
	if ( r->b == NULL )
	{
		fprintf( frep, "<BR>\n" );
		for ( i = 0; i < dep; ++i )
			if ( msg[ i ] == ' ' )
				fprintf( frep, "<TT>&nbsp;</TT>" );
			else
				fprintf( frep, "<TT>|</TT>" );
			
		fprintf( frep, "<BR>\n" );
	}
}


/********************************
WRITE_LIST
*********************************/
void write_list( FILE *frep, object *root, int flag_all, char const *prefix )
{
	int num, i;
	char *app, s1[ 2 * MAX_ELEM_LENGTH ], s2[ MAX_ELEM_LENGTH ];

	Tcl_LinkVar( inter, "num", ( char * ) &num, TCL_LINK_INT );

	if ( flag_all == 1 ) 					// initial listing
		fprintf( frep, "<H3>Variables</H3>\n" );
	else
		fprintf( frep, "<i>Variables: &nbsp;</i>" );

	cmd( "lappend rawlist" ); 				// create the list if not existed
	cmd( "unset rawlist" ); 				// empty the list
	cmd( "lappend rawlist" ); 				// create a surely empty list

	if ( ! strcmp( prefix, "_i_" ) )
		fill_list_var( root, flag_all, 1 ); // insert only lagged variables
	else
		fill_list_var( root, flag_all, 0 );	// insert all the variables

	cmd( "set alphalist [lsort -dictionary $rawlist]" );
	cmd( "set num [llength $alphalist]" );
	
	// distinguish the case you are compiling the initial list of element (all) or for a single Object)
	if ( flag_all == 0 ) 
		sprintf( s1, "form_v_%s_%s", root->label, prefix );
	else
		sprintf( s1, "form_v_all_%s_%s", root->label, prefix );  

	if ( lmenu )
	{
		create_form( num, s1, prefix, frep ); 
		if ( num == 0 )
			fprintf( frep, "(none)<BR>\n" );
	}
	else
	{
		for ( i = 0; i < num; ++i )
		{
			cmd( "set app [ lindex $alphalist %d ]", i );
			app = ( char * ) Tcl_GetVar( inter, "app", 0 );
			strcpy( msg, app );
			sscanf( msg, "%s %s", s1, s2);
			fprintf( frep, "<TT><A HREF=\"#%s%s\">%s</A></TT>", prefix, s1, s1 );
			
			if ( i < num - 1 )
				fprintf( frep, "<TT>, </TT>" );
		}

		if ( num > 0 )
			fprintf( frep, "<BR>\n" );
		else
			fprintf( frep, "(none)<BR>\n" );
	}
	 
	if ( flag_all == 1 ) 					// initial listing
		fprintf( frep, "<H3>Parameters</H3>\n" );
	else
		fprintf( frep, "<i>Parameters: &nbsp;</i>" );

	cmd( "lappend rawlist" ); 				// create the list if not existed
	cmd( "unset rawlist" ); 				// empty the list
	cmd( "lappend rawlist" ); 				// create a surely empty list

	fill_list_par( root, flag_all );

	cmd( "set alphalist [lsort -dictionary $rawlist]" );
	cmd( "set num [llength $alphalist]" );
	
	//distinguish the case you are compiling the initial list of element (all) or for a single Object)
	if ( flag_all == 0 )
		sprintf( s1, "form_p_%s_%s", root->label, prefix );
	else
		sprintf( s1, "form_p_all_%s_%s", root->label, prefix ); 

	if ( lmenu )
	{
		create_form( num, s1, prefix, frep ); 
		if ( num == 0 )
			fprintf( frep, "(none)<BR>\n" );
	}
	else
	{
		for ( i = 0; i < num; ++i )
		{
			cmd( "set app [ lindex $alphalist %d ]", i );
			app = ( char * ) Tcl_GetVar( inter, "app", 0 );
			strcpy( msg, app );
			fprintf( frep, "<TT><A HREF=\"#%s%s\">%s</A></TT>",prefix, msg, msg );
			
			if ( i < num - 1 )
				fprintf( frep, "<TT>, </TT>" );
		}
	 
		if ( num > 0 )
			fprintf( frep, "<BR>\n" );
		else
			fprintf( frep, "(none)<BR>\n" );
	}

	Tcl_UnlinkVar( inter, "num" );
}

/********************************
FILL_LIST_VAR
*********************************/
void fill_list_var( object *r, int flag_all, int flag_init )
{
	bridge *cb;
	variable *cv;

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( ( cv->param == 0 || cv->param == 2 ) && ( flag_init == 0 || cv->num_lag > 0) )
		cmd( "lappend rawlist \"%s (%d)\"", cv->label, cv->num_lag );
	}
	
	if ( flag_all == 0 )
		return;
	
	for ( cb = r->b; cb != NULL; cb = cb->next )
		fill_list_var( cb->head, flag_all, flag_init );
}


/********************************
FILL_LIST_PAR
*********************************/
void fill_list_par( object *r, int flag_all )
{
	bridge *cb;
	variable *cv;

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( cv->param == 1 )
			cmd( "lappend rawlist \"%s\"", cv->label );
	}
	
	if ( flag_all == 0 )
		return;
	
	for ( cb = r->b; cb != NULL; cb = cb->next )
		fill_list_par( cb->head, flag_all);
}


/********************************
CREATE_TABLE_INIT
Create recursively the help table for an Object
*********************************/
void create_table_init( object *r, FILE *frep )
{
	int i;
	bridge *cb;
	description *cd;
	variable *cv;

	fprintf( frep, "<HR WIDTH=\"100%%\">\n" );
	 
	fprintf( frep, "<H3><A NAME=\"%s\">Object: &nbsp;<TT><U>%s</U></TT></A></H3>", r->label, r->label );

	if ( r->up != NULL )
	{
		fprintf( frep, "<i>Contained in: &nbsp;</i>" ); 
		ancestors( r, frep );
		fprintf( frep, "<BR>\n" );
	}

	if ( r->b != NULL )
	{
		fprintf( frep, "<i>Containing: &nbsp;</i>" );
		fprintf( frep, "<TT><a HREF=\"#%s\">%s</a></TT>", r->b->blabel, r->b->blabel );
		for ( cb = r->b->next ; cb != NULL; cb = cb->next )
			fprintf( frep, "<TT>,  <a HREF=\"#%s\">%s</a></TT>", cb->blabel, cb->blabel );
		fprintf( frep, "<BR>\n" );
	}

	fprintf( frep, "<BR>\n" );
	write_list( frep, r, 0, "" );

	cd = search_description( r->label );
	if ( cd == NULL )
	{
		add_description( r->label, "Object", "(no description available)" );
		plog( "\nWarning: description for '%s' not found. New one created.", "", r->label );
		cd = search_description( r->label );
	} 

	if ( cd != NULL && cd->text != NULL && ! strstr( cd->text, "(no description available)" ) )
	{
		fprintf( frep, "<i>Description:</i><BR>\n" );
		
		for ( i = 0; cd->text[ i ] != '\0'; ++i )
		{
			switch ( cd->text[ i ])
			{
				case '\n':
					fprintf( frep, "<br>\n" );
					break;
				case '<':
					fprintf( frep, "&lt;" );
					break;
				case '>':
					fprintf( frep, "&gt;" );
					break;
				default:
					fprintf( frep, "%c", cd->text[ i ] );
					break;
			}
		}
		
		fprintf( frep, "<BR>\n" );
	}

	fprintf( frep, "<BR>\n" );

	if ( r->v != NULL )
	{
		fprintf( frep, "<table BORDER>" );
		fprintf( frep, "<tr>" );
		fprintf( frep, "<td><center><i>Element</i></center></td>" );
		fprintf( frep, "<td><center><i>Lags</i></center></td>\n" );
		fprintf( frep, "<td><center><i>Description and initial values comments</i></center></td>\n" );
		fprintf( frep, "</tr>" );
	
		for ( cv = r->v; cv != NULL; cv = cv->next )
		{
			fprintf( frep, "<tr VALIGN=TOP>" );

			if ( cv->param == 1 )
			{
				fprintf( frep, "<td><a NAME=\"%s\"><A HREF=\"#_d_%s\"><TT>%s</TT></A></a></td>\n", cv->label, cv->label, cv->label );
				fprintf( frep, "<td><A HREF=\"#_i_%s\">Par.</A></td>\n", cv->label );
			}
			else
			{
				fprintf( frep, "<td><a NAME=\"%s\"><A HREF=\"#_d_%s\"><TT>%s</TT></A></a></td>\n", cv->label, cv->label, cv->label );
				if ( cv->num_lag > 0 )
					fprintf( frep, "<td><A HREF=\"#_i_%s\">%d</A></td>\n", cv->label, cv->num_lag );
				else
					fprintf( frep, "<td></td>\n" );
			}
	   
			cd = search_description( cv->label );
			if ( cd == NULL )
			{
				if ( cv->param == 0 )
					add_description( cv->label, "Variable", "(no description available)" );
				if ( cv->param == 1 )
					add_description( cv->label, "Parameter", "(no description available)" );  
				if ( cv->param == 2 )
					add_description( cv->label, "Function", "(no description available)" );  
				plog( "\nWarning: description for '%s' not found. New one created.", "", cv->label );
				cd = search_description( cv->label );
			} 

			fprintf( frep, "<td> " );
			bool desc_text = false;
			if ( cd->text != NULL && strlen( cd->text ) > 0 && ! strstr( cd->text, "(no description available)" ) )
				for ( i = 0; cd->text[ i ] != '\0'; ++i )
				{
					desc_text = true;
					switch ( cd->text[ i ])
					{
						case '\n':
							fprintf( frep, "<br>\n" );
							desc_text = false;
							break;
						case '<':
							fprintf( frep, "&lt;" );
							break;
						case '>':
							fprintf( frep, "&gt;" );
							break;
						default:
							fprintf( frep, "%c", cd->text[ i ] );
							break;
					}
				}
	  
			if ( cv->param == 1 || cv->num_lag > 0 )
				if ( cd->init != NULL && strlen( cd->init ) > 0 )
				{
					if ( desc_text )
						fprintf( frep, "<br>\n" );
		
					for ( i = 0; cd->init[ i ] != '\0'; ++i )
						switch ( cd->init[ i ])
						{
							case '\n':
								fprintf( frep, "<br>\n" );
								break;
							case '<':
								fprintf( frep, "&lt;" );
								break;
							case '>':
								fprintf( frep, "&gt;" );
								break;
							default:
								fprintf( frep, "%c", cd->init[ i ] );
								break;
						}
				}
	 
			fprintf( frep, "</td></tr>\n" );
		}
		
		fprintf( frep, "</table><br>\n" );
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
		create_table_init( cb->head, frep );
}


/********************************
CREATE_INITIAL_VALUES
Create recursively the help table for the initial values of an Object
*********************************/
void create_initial_values( object *r, FILE *frep )
{
	int count = 0, i, j;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	for ( cv = r->v; cv != NULL; cv = cv->next )
		if ( cv->param == 1 || cv->num_lag > 0 )
			count = 1;

	if ( count == 0 )
	{	// no need for initialization, typically root
		for ( cb = r->b; cb != NULL; cb = cb->next )
			create_initial_values( cb->head, frep );
		return;
	}  
	 
	for ( count = 0, cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
		count++;

	fprintf( frep, "<HR WIDTH=\"100%%\">\n" );
	fprintf( frep, "<H3><A NAME=\"%s\">Object: &nbsp;<TT><U>%s</U></TT></A></H3>", r->label, r->label );
	fprintf( frep, "<i>Instances number: &nbsp</i>%d<BR>", count );
	fprintf( frep, "<i>Instances group(s): &nbsp;</i>" ); 

	for ( i = 0, cur = r; cur != NULL; )
	{ 
		count = 1;
		while ( cur->next != NULL && ! strcmp( cur->label, ( cur->next )->label ) )
		{
			++count;
			cur = cur->next;
		}
		
		fprintf( frep, "%d ", count );
		cur = cur->hyper_next( cur->label );
		++i;
		
		if ( i > MAX_INIT )
		{
			fprintf( frep, "..." );
			cur = NULL;
		}
	}
	
	fprintf( frep, "<BR><BR>\n" ); 
	fprintf( frep, "<table BORDER>" );
	fprintf( frep, "<tr>" );
	fprintf( frep, "<td><center><i>Element</i></center></td>\n" );
	fprintf( frep, "<td><center><i>Lag</i></center></td>\n" );
	fprintf( frep, "<td><center><i>Initial values (by instance)</i></center></td>\n" );
	fprintf( frep, "</tr>" );

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( cv->param == 1 )
		{
			fprintf( frep, "<tr VALIGN=TOP>" );
			fprintf( frep, "<td><a NAME=\"_i_%s\"><A HREF=\"#%s\"><TT>%s</TT></A></a></td>\n", cv->label, cv->label, cv->label );
			fprintf( frep, "<td>Par.</td>\n" );
			fprintf( frep, "<td>%g", cv->val[ 0 ] );
			
			for ( j = 1, cur = r->hyper_next( r->label ); cur !=NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
			{
				cv1 = cur->search_var( cur, cv->label );
				fprintf( frep, ", %g", cv1->val[ 0 ] );
			}
			
			if ( j == MAX_INIT && cur != NULL )
				fprintf( frep, ", ..." );

			fprintf( frep, "</td></tr>\n" );
		}   
		else
		{
			for ( i = 0; i < cv->num_lag; ++i )
			{
				fprintf( frep, "<tr VALIGN=TOP>" );
				fprintf( frep, "<td><a NAME=\"_i_%s\"><A HREF=\"#%s\"><TT>%s</TT></A></a></td>\n", cv->label, cv->label, cv->label );
				fprintf( frep, "<td>%d</td>\n", i + 1 );
				fprintf( frep, "<td>%g", cv->val[ i ] );
				
				for ( j = 1, cur = r->hyper_next( r->label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
				{
					cv1 = cur->search_var( cur, cv->label );
					fprintf( frep, ", %g", cv1->val[ i ] );
				}
				
				if ( j == MAX_INIT && cur != NULL )
					fprintf( frep, ", ..." );
			  
				fprintf( frep, "</td></tr>\n" );
			}
		}
	}
	
	fprintf( frep, "</table><BR>\n" );

	for ( cb = r->b; cb != NULL; cb = cb->next )
		create_initial_values( cb->head, frep );
}


/********************************
IS_EQUATION_HEADER
Squeeze the spaces out of line and returns 1 if the line is an equation header,
placing the Variable label in Var
*********************************/
bool is_equation_header( char *raw_line, char *var, char *updt_in )
{
	bool header;
	int i, j;
	char *line = new char[ strlen( raw_line ) + 1 ];
	
	eq_dum = false;
	strcpy( line, raw_line );
	clean_spaces( line );

	if ( ! strncmp( line, "if(!strcmp(label,", 17 ) || ! strncmp( line, "EQUATION(", 9 ) || ! strncmp( line, "EQUATION_DUMMY(", 9 ) || ! strncmp( line, "FUNCTION(", 9 ) )
	{
		header = true;
		
		if ( ! strncmp( line, "if(!strcmp(label,", 17 ) )
			macro = false;
		else
			macro = true;
		
		if ( ! strncmp( line, "EQUATION_DUMMY(", 15 ) )
			eq_dum = true;
				
		for ( i = 0; line[ i ] != '"' && line[ i ] != '\0' && i < MAX_LINE_SIZE; ++i );
		
		for ( j = ++i; line[ i ] != '"' && line[ i ] != '\0' && i < MAX_LINE_SIZE; ++i )
			var[ i - j ] = line[ i ];
		var[ i - j ] = '\0';
		
		if ( eq_dum )
		{
			for ( ++i; line[ i ] != '"' && line[ i ] != '\0' && i < MAX_LINE_SIZE; ++i );
			
			if ( line[ i ] == '"' && line[ i + 1 ] != '"' )	// non-empty name?
			{
				for ( j = 0, ++i; line[ i ] != '"' && line[ i ] != '\0' && j < MAX_LINE_SIZE; ++i, ++j )
					updt_in[ j ] = line[ i ];
				updt_in[ j ] = '\0';
			}
			else
				strcpy( updt_in, "" );
		}
		else
			strcpy( updt_in, "" );
	}
	else
		header = false;

	delete [ ] line;
	
	return header;
}


/************
 ANCESTORS 
 ************/
void ancestors( object *r, FILE *f, bool html )
{
	if ( r->up != NULL )
	{
	  ancestors( r->up, f, html );
	  if ( r->up->up == NULL )
		if ( html )
		  fprintf( f, "<TT><A HREF=\"#%s\">%s</A></TT>", r->up->label, r->up->label );
		else
		  fprintf( f, "\\lsd{%s}", r->up->label );
	  else
		if ( html )
		  fprintf( f, "<TT>&mdash;&gt;<A HREF=\"#%s\">%s</A></TT>", r->up->label, r->up->label );
		else
		  fprintf( f, "$\\rightarrow$\\lsd{%s}", r->up->label );
	}
}


/************
 CREATE_FRAMES 
 ************/
FILE *create_frames( char *t )
{
	FILE *f;

	f = fopen( t, "w" );
	if ( f == NULL )
		return NULL;

	fprintf( f, "<html> <head> <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\"> <meta name=\"AUTHOR\" content=\"Marco Valente\"> </head>\n" );
	fprintf( f, "<FRAMESET framespacing=0 border=1 rows=\"30,*\"> <FRAME NAME=\"testa\" SRC=\"head_%s\"",t );

	fprintf( f, "frameborder=0 MARGINHEIGHT=\"0\" MARGINWIDTH=\"0\" SCROLLING=\"NO\" NORESIZE> <FRAME NAME=\"body\" SRC=\"body_%s\" frameborder=0 MARGINHEIGHT=\"0\" MARGINWIDTH=\"0\" SCROLLING=\"AUTO\"> </FRAMESET> <NOFRAMES>  <p>Use a frame-enabled browser</p> </NOFRAMES> </html>",t );

	fclose( f );

	sprintf( msg, "head_%s", t );
	f = fopen( msg, "w" );
	if ( f == NULL )
		return NULL;

	fprintf( f, "<html> <head> </head > <body bgcolor=\"#D6A9FE\"> <center>" );
	fprintf( f, "<a href=\"body_%s#_TOP_\" target=body>Top</a> &nbsp", t );
	if ( desc || extra )
		fprintf( f, "<a href=\"body_%s#_DESCRIPTION_\" target=body>Description</a> &nbsp", t );
	fprintf( f, "<a href=\"body_%s#_MODEL_STRUCTURE_\" target=body>Structure</a> &nbsp", t );
	if ( init )
		fprintf( f, "<a href=\"body_%s#_INITIALVALUES_\" target=body>Initialization</a> &nbsp", t );
	if ( code )
		fprintf( f, "<a href=\"body_%s#_DETAILS_\" target=body>Code</a> &nbsp", t );

	fprintf( f, "</body> </html>" );
	fclose( f );

	sprintf( msg, "body_%s", t );
	f = fopen( msg, "w" );
	return f;
}


/************
CREATE LIST FORMS FOR LABELS. 
**********/
void create_form( int num, char const *title, char const *prefix, FILE *frep )
{
	int i;
	char s1[ 2 * MAX_ELEM_LENGTH ], s2[ 2 * MAX_ELEM_LENGTH ], *app;

	if ( num == 0 )
		return;

	sprintf( msg, "<form name=\"%s\" method=\"POST\">\n", title );
	fprintf( frep,"%s",msg );
	sprintf( msg, "<select name=\"entry\" class=\"form\">\n" );
	fprintf( frep,"%s", msg );

	for ( i = 0; i < num; ++i )
	{
		cmd( "set app [ lindex $alphalist %d ]", i );
		app = ( char * ) Tcl_GetVar( inter, "app", 0 );
		strcpy( msg, app );
		sscanf( msg, "%s %s", s1, s2 );
		fprintf( frep, "<option value=\"#%s%s\">%s</option>\n", prefix, s1, s1 );
	}
	
	fprintf( frep, "</select>\n" );
	fprintf( frep, "<INPUT TYPE=button VALUE=\"Show\" onClick=\"location.href = document.%s.entry.options[document.%s.entry.selectedIndex].value\">\n", title, title );
	fprintf( frep, "</form>" );
}


/****************************************************
SHOW_REP_OBSERVE
****************************************************/
void show_rep_observe( FILE *f, object *n, int *begin, FILE *frep )
{
	int i;
	bridge *cb;
	description *cd;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );
		if (cd == NULL )
		{
			if ( cv->param == 0 )
				add_description( cv->label, "Variable", "(no description available)" );
			if ( cv->param == 1 )
				add_description( cv->label, "Parameter", "(no description available)" );  
			if ( cv->param == 2 )
				add_description( cv->label, "Function", "(no description available)" );  
			plog( "\nWarning: description for '%s' not found. New one created.", "", cv->label );
			cd = search_description( cv->label );
		} 

		if ( cd->observe == 'y' )
		{
			if ( *begin == 1 )
			{
				*begin = 0;
				table = true;
				fprintf( f,"<H3>Relevant elements to observe</H3>\n" );
				
				fprintf( f, "<table BORDER>" );
				fprintf( f, "<tr>" );
				fprintf( f, "<td><center><i>Element</i></center></td>\n" );
				fprintf( f, "<td><center><i>Object</i></center></td>" );
				fprintf( f, "<td><center><i>Type</i></center></td>\n" );
				fprintf( f, "<td><center><i>Description</i></center></td>\n" );    
				fprintf( f, "</tr>" );
			}
			
			fprintf( f, "<tr VALIGN=TOP>" );
		 
			fprintf( f, "<td><TT><A HREF=\"#%s\">%s</A></TT></td>", cv->label, cv->label );
			fprintf( f, "<td><TT><A HREF=\"#%s\">%s</A></TT></td>", n->label, n->label );
			
			if ( cv->param == 1 )
				fprintf( f, "<td>Parameter</td>" );
			if ( cv->param == 0 )
				fprintf( f, "<td>Variable</td>" );
			if ( cv->param == 2 )
				fprintf( f, "<td>Function</td>" );
			
			fprintf( f, "<td>" ); 
			
			if ( cd->text != NULL && strlen( cd->text ) > 0 && ! strstr( cd->text, "(no description available)" ) )
				for ( i = 0; cd->text[ i ] != '\0'; ++i )
				{
					switch ( cd->text[ i ] )
					{
						case '\n':
							fprintf( f, "<br>\n" );
							break;
						case '<':
							fprintf( f, "&lt;" );
							break;
						case '>':
							fprintf( f, "&gt;" );
							break;
						default:
							fprintf( f, "%c", cd->text[ i ] );
							break;
					}
				}
		 
			fprintf( f, "</td></tr>\n" );    
		}
	}

for ( cb = n->b; cb != NULL; cb = cb->next )
	show_rep_observe( f, cb->head, begin, frep );

if ( n->up == NULL && table )
	fprintf( f, "</table><BR>\n" );
}


/****************************************************
 SHOW_REP_INITIAL
 ****************************************************/
void show_rep_initial( FILE *f, object *n, int *begin, FILE *frep )
{
	int i;
	bridge *cb;
	description *cd;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );
		if ( cd == NULL )
		{
			if ( cv->param == 0 )
				add_description( cv->label, "Variable", "(no description available)" );
			if ( cv->param == 1 )
				add_description( cv->label, "Parameter", "(no description available)" );  
			if ( cv->param == 2 )
				add_description( cv->label, "Function", "(no description available)" );  
			
			plog( "\nWarning: description for '%s' not found. New one created.", "", cv->label );
			
			cd = search_description( cv->label );
		} 

		if ( cd->initial == 'y' )
		{
			if ( *begin == 1 )
			{
				*begin = 0;
				table = true;
				fprintf( f,"<H3>Relevant elements to initialize</H3>\n" );
				
				fprintf( f, "<table BORDER>" );
				fprintf( f, "<tr>" );
				fprintf( f, "<td><center><i>Element</i></center></td>\n" );
				fprintf( f, "<td><center><i>Object</i></center></td>" );
				fprintf( f, "<td><center><i>Type</i></center></td>\n" );
				fprintf( f, "<td><center><i>Description and initial values comments</i></center></td>\n" );    
				fprintf( f, "</tr>" );
			}
			
			fprintf( f, "<tr VALIGN=TOP>" );
			fprintf( f, "<td><TT><A HREF=\"#%s\">%s</A></TT></td>", cv->label, cv->label );
			fprintf( f, "<td><TT><A HREF=\"#%s\">%s</A></TT></td>", n->label, n->label );
			
			if ( cv->param == 1 )
				fprintf( f, "<td>Parameter</td>" );
			if ( cv->param == 0 )
				fprintf( f, "<td>Variable</td>" );
			if ( cv->param == 2 )
				fprintf( f, "<td>Function</td>" );
				
			fprintf( f, "<td>" ); 
			
			bool desc_text = false;
			if ( cd->text != NULL && strlen( cd->text ) > 0 && ! strstr( cd->text, "(no description available)" ) )
				for ( i = 0; cd->text[ i ] != '\0'; ++i )
				{
					desc_text = true;
					switch ( cd->text[ i ] )
					{
						case '\n':
							fprintf( f, "<br>\n" );
							desc_text = false;
							break;
						case '<':
							fprintf( f, "&lt;" );
							break;
						case '>':
							fprintf( f, "&gt;" );
							break;
						default:
							fprintf( f, "%c", cd->text[ i ] );
							break;
					}
				}
				
			if ( ( cv->param == 1 || cv->num_lag > 0 ) && cd->init != NULL && strlen( cd->init ) > 0 )
			{
				if ( desc_text )
					fprintf( frep, "<br>\n" );
				 
				for ( i = 0; cd->init[ i ] != '\0'; ++i )
				{
					switch ( cd->init[ i ] )
					{
						case '\n':
							fprintf( f, "<br>\n" );
							break;
						case '<':
							fprintf( f, "&lt;" );
							break;
						case '>':
							fprintf( f, "&gt;" );
							break;
						default:
							fprintf( f, "%c", cd->init[ i ] );
							break;
					}
				}
			}
			  
			fprintf( f, " <A HREF=\"#_i_%s\">(Show initial values)</A>",  cv->label );
			fprintf( f, "</td></tr>\n" );    
		}
	}

	for ( cb = n->b; cb != NULL; cb = cb->next )
		show_rep_initial( f, cb->head, begin, frep );

	if ( n->up == NULL && table )
		fprintf( f, "</table><BR>\n" );
}


/****************************************************
 TEX_STRCPY
 ****************************************************/
void tex_strcpy( char *&s1, char* s2 )
{
	int i, j;
	
	// handle active underscores and other special chars in label references
	for ( i = 0, j = 0; s2[ j ] != '\0'; ++j )
		if ( s2[ j ] == '_' || s2[ j ] == '^' || s2[ j ] == '$' || s2[ j ] == '&' )
			++i;
		
	if ( i > 0 )
	{
		delete [ ] s1;
		s1 = new char[ 2 * strlen( s2 ) + 1 + i * 6 ];
	}
	
	for ( i = 0, j = 0; s2[ j ] != '\0'; ++i, ++j )
		switch ( s2[ j ] )
		{
			case '\n':
			case '>':
			case '<':
			case '\\':
			case '}':
			case '{':
			case '~':
			case '%':
			case '#':
				--i;		// simply remove char
				break;
			case '_':
			case '^':
			case '$':
			case '&':
				memcpy( s1 + i, "\\string", 7 );
				i += 7;
			default:
				s1[ i ] = s2[ j ]; 
		}
		
	s1[ i ] = '\0';
}


/****************************************************
 TEX_FPRINTF
 ****************************************************/
void tex_fprintf( FILE *f, char* text )
{
	bool newline = false;
	
	for ( int i = 0; text[ i ] != '\0'; ++i )
		switch ( text[ i ] )
		{
			case '\n':
				if ( ! newline )
					fprintf( f, "\\newline " );
				newline = true;
				break;
			case '>':
				fprintf( f, "\\textgreater " ); 
				newline = false;
				break;
			case '<':
				fprintf( f, "\\textless " ); 
				newline = false;
				break;
			case '\\':
				fprintf( f, "/" ); 
				newline = false;
				break;
			case '_':
			case '^':
			case '~':
			case '}':
			case '{':
			case '%':
			case '$':
			case '#':
			case '&':
				fprintf( f, "\\" ); 
			default:
				fprintf( f, "%c", text[ i ] ); 
				newline = false;
		}
}

				
/****************************************************
 TEX_REPORT_HEAD
 ****************************************************/
void tex_report_head( FILE *f, bool table )
{
	fprintf( f, "\\documentclass{article}\n\n" );
	fprintf( f, "\\usepackage[%s,left=%fcm,right=%fcm,top=%fcm,bottom=%fcm]{geometry}\n", TEX_PAPER, TEX_LEFT, TEX_RIGHT, TEX_TOP, TEX_BOTTOM );
	
	if ( table )
	{
		fprintf( f, "\\usepackage{color}\n" );
		fprintf( f, "\\usepackage{longtable}\n" );
		fprintf( f, "\\usepackage{tabu}\n\n" );
		fprintf( f, "\\newcommand{\\lsd}[1] {\\texttt{\\color{blue}{\\detokenize{#1}}}}\n\n" );
	}
	else
	{
		fprintf( f, "\\usepackage[colorlinks=true,linkcolor=blue,pdfborder={0 0 0}]{hyperref}\n\n" );
		fprintf( f, "\\newcommand{\\lsd}[1] {\\texttt{\\detokenize{#1}}}\n" );
		fprintf( f, "\\newcommand{\\hr}[1] {\\hrf{#1}{#1}}\n" );
		fprintf( f, "\\newcommand{\\hrf}[2] {\\hyperref[#1]{\\texttt{\\detokenize{#2}}}}\n" );
	}
	
	fprintf( f, "\\setlength{\\parindent}{0cm}\n\n" );

	fprintf( f, "\\title{Model: \\lsd{%s}}\n", simul_name );
	fprintf( f, "\\author{Automatically generated LSD report}\n" );
	fprintf( f, "\\date{}\n\n" );

	fprintf( f, "\\begin{document}\n\n" );
	fprintf( f, "\\maketitle\n\n" );
}
	  

/****************************************************
 TEX_REPORT_STRUCT
 ****************************************************/
void tex_report_struct( object *r, FILE *f, bool table )
{
	char *ol, *vl;
	bridge *cb;
	description *cd;
	variable *cv;

	if ( r->up == NULL )
		fprintf( f, "\\section{Model Structure}\n\n" );

	ol = new char[ 2 * strlen( r->label ) + 1 ];
	tex_strcpy( ol, r->label );
	fprintf( f, "\\subsection{Object: \\lsd{%s}} \\label{%s}\n\n", r->label, ol );
	delete [ ] ol;
	
	if ( r->up != NULL )
	{
		fprintf( f,"\\emph{Contained in:} " );
		ancestors( r, f, false );
		fprintf( f,"\n\n" );
	}

	if ( r->b != NULL )
	{
		fprintf( f,"\\emph{Containing:} \\lsd{%s}", r->b->blabel );
		for ( cb = r->b->next; cb != NULL; cb = cb->next )
			fprintf( f, ",  \\lsd{%s}", cb->blabel );
		fprintf( f, "\n\n" );
	}

	cd = search_description( r->label );
	if ( cd->text != NULL && strlen( cd->text ) > 0 && ! strstr( cd->text, "(no description available)" ) )
		fprintf( f, "\\emph{Description:}\n\n\\detokenize{%s}\n\n", cd->text );

	if ( r->v != NULL )
	{
		if ( ! table )
			fprintf( f,"\\emph{Contained elements:}\n\n" );	
		else
			fprintf( f, "\\begin{longtabu} to \\textwidth {|l|l|l|X|}\n  \\hline\n  \\textbf{Element} & \\textbf{Type} & \\textbf{Lags} & \\textbf{Description and initial values comments} \\\\ \n  \\hline \\endhead\n  \\multicolumn{4}{r}{\\textit{Continued on next page...}} \\\\ \n  \\endfoot\n  \\endlastfoot\n" );
	}

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		vl = new char[ 2 * strlen( cv->label ) + 1 ];
		tex_strcpy( vl, cv->label );
		fprintf( f, "  \\lsd{%s} \\label{%s}", cv->label , vl );
		delete [ ] vl;
		
		if ( ! table )
		{
			if ( cv->param == 0 )
				fprintf( f, " (variable" );
			if ( cv->param == 1 )
				fprintf( f, " (parameter" );
			if ( cv->param == 2 )
				fprintf( f, " (function" );
			if ( cv->param == 1 || cv->num_lag == 0 )
				fprintf( f, ")" );
			else
				fprintf( f, ", %d lag%s)", cv->num_lag, cv->num_lag == 1 ? "" : "s" );
		}
		else
		{
			if ( cv->param == 0 )
				fprintf( f, " & Variable & " );
			if ( cv->param == 1 )
				fprintf( f, " & Parameter & " );
			if ( cv->param == 2 )
				fprintf( f, " & Function & " );
			if ( cv->param == 1 || cv->num_lag == 0 )
				fprintf( f, "& " );
			else
				fprintf( f, "%d & ", cv->num_lag );
		}
		
		bool desc_text = false;
		cd = search_description( cv->label );
		if ( cd->text != NULL && strlen( cd->text ) > 0 && ! strstr( cd->text, "(no description available)" ) )
		{
			if ( ! table )
				fprintf( f, ": " );
			desc_text = true;
			tex_fprintf( f, cd->text );
		}
	  
		if ( table && ( cv->param == 1 || cv->num_lag > 0 ) && cd->init != NULL && strlen( cd->init ) > 0 )
		{
			if ( desc_text )
				fprintf( f, "\\newline " );
			tex_fprintf( f, cd->init );
		}

		if ( ! table )
			fprintf( f, "\\newline \n" ); 
		else
			fprintf( f, "\\\\ \n  \\hline \n" ); 
	}

	if ( r->v != NULL )
	{
		if ( table )
			fprintf( f, "\\end{longtabu}\n\n" );
		else
			fprintf( f, "\n" );
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
		tex_report_struct( cb->head, f, table );
}


/****************************************************
 TEX_REPORT_OBSERVE
 ****************************************************/
void tex_report_observe( object *r, FILE *f, bool table )
{
	char *ol, *vl;
	bridge *cb;
	description *cd;
	variable *cv;

	if ( r->up == NULL )
	{
		fprintf( f, "\\section{Relevant elements to observe}\n\n" );
		tab_lines = 0;
		if ( table )
			fprintf( f, "\\begin{longtabu} to \\textwidth {|l|l|l|X|}\n  \\hline\n  \\textbf{Element} & \\textbf{Object} & \\textbf{Type} & \\textbf{Description} \\\\ \n  \\hline \\endhead\n  \\multicolumn{4}{r}{\\textit{Continued on next page...}} \\\\ \n  \\endfoot\n  \\endlastfoot\n" );
	}

	ol = new char[ 2 * strlen( r->label ) + 1 ];
	tex_strcpy( ol, r->label );
			
	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );
		if ( cd->observe == 'y' )
		{
			vl = new char[ 2 * strlen( cv->label ) + 1 ];
			tex_strcpy( vl, cv->label );
			
			if ( ! table )
				fprintf( f, "\\hrf{%s}{%s} (object \\hrf{%s}{%s}) \\newline \n", vl, cv->label, ol, r->label );
			else
			{
				fprintf( f, "  \\lsd{%s} & \\lsd{%s} & ", cv->label, r->label );
				if ( cv->param == 0 )
					fprintf( f, "Variable & " );
				if ( cv->param == 1 )
					fprintf( f, "Parameter & " );
				if ( cv->param == 2 )
					fprintf( f, "Function & " );
				if ( cd->text != NULL && strlen( cd->text ) > 0 && ! strstr( cd->text, "(no description available)" ) )
					tex_fprintf( f, cd->text );
				fprintf( f, "\\\\ \n  \\hline \n" ); 
			} 
			
			++tab_lines;
		
			delete [ ] vl;
		}
	}

	delete [ ] ol;
			
	for ( cb = r->b; cb != NULL; cb = cb->next )
		tex_report_observe( cb->head, f, table );

	if ( r->up == NULL )
	{
		if ( table )
		{
			if ( tab_lines == 0 )
				fprintf( f, "  \\multicolumn{4}{|c|}{(none selected)} \\\\ \n  \\hline \n" );
			fprintf( f, "\\end{longtabu}\n\n" );
		}
		else
			if ( tab_lines == 0 )
				fprintf( f, "(none selected) \\newline \n" );
			else
				fprintf( f, "\n" );
	}
}


/****************************************************
 TEX_REPORT_INIT
 ****************************************************/
void tex_report_init( object *r, FILE *f, bool table )
{
	char *ol, *vl;
	bridge *cb;
	description *cd;
	variable *cv;

	if ( r->up == NULL )
	{
		fprintf( f, "\\section{Relevant elements to initialize}\n\n" );
		tab_lines = 0;
		if ( table )
			fprintf( f, "\\begin{longtabu} to \\textwidth {|l|l|l|X|}\n  \\hline\n  \\textbf{Element} & \\textbf{Object} & \\textbf{Type} & \\textbf{Description and initial values comments} \\\\ \n  \\hline \\endhead\n  \\multicolumn{4}{r}{\\textit{Continued on next page...}} \\\\ \n  \\endfoot\n  \\endlastfoot\n" );
	}

	ol = new char[ 2 * strlen( r->label ) + 1 ];
	tex_strcpy( ol, r->label );
			
	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );
		if ( cd->initial == 'y' )
		{
			vl = new char[ 2 * strlen( cv->label ) + 1 ];
			tex_strcpy( vl, cv->label );
			
			if ( ! table )
				fprintf( f, "\\hrf{%s}{%s} (object \\hrf{%s}{%s}) \\newline \n", vl, cv->label, ol, r->label );
			else
			{
				fprintf( f, "  \\lsd{%s} & \\lsd{%s} & ", cv->label, r->label );
				if ( cv->param == 0 )
					fprintf( f, "Variable & " );
				if ( cv->param == 1 )
					fprintf( f, "Parameter & " );
				if ( cv->param == 2 )
					fprintf( f, "Function & " );
		  
				bool desc_text = false;
				if ( cd->text != NULL && strlen( cd->text ) > 0 && ! strstr( cd->text, "(no description available)" ) )
				{
					desc_text = true;
					tex_fprintf( f, cd->text );
				}
				
				if ( ( cv->param == 1 || cv->num_lag > 0 ) && cd->init != NULL && strlen( cd->init ) > 0 )
				{
					if ( desc_text )
						fprintf( f, "\\newline " );
					tex_fprintf( f, cd->init );
				}
				fprintf( f, "\\\\ \n  \\hline \n" ); 
			}
		
			++tab_lines;
			
			delete [ ] vl;
		} 
	}

	delete [ ] ol;
			
	for ( cb = r->b; cb != NULL; cb = cb->next )
		tex_report_init( cb->head, f, table );

	if ( r->up == NULL )
	{
		if ( table )
		{
			if ( tab_lines == 0 )
				fprintf( f, "  \\multicolumn{4}{|c|}{(none selected)} \\\\ \n  \\hline \n" );
			fprintf( f, "\\end{longtabu}\n\n" );
		}
		else
			if ( tab_lines == 0 )
				fprintf( f, "(none selected) \\newline \n" );
			else
				fprintf( f, "\n" );
	}
}


/****************************************************
 TEX_REPORT_INITALL
 ****************************************************/
void tex_report_initall( object *r, FILE *f, bool table )
{
	int i, j;
	char *ol, *vl;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	if ( ! table )
		return;

	if ( r->up == NULL )
	{
		fprintf( f, "\\section{Initial values}\n\n" );
		fprintf( f, "\\begin{longtabu} to \\textwidth {|l|l|l|X|}\n  \\hline\n  \\textbf{Object} & \\textbf{Element} & \\textbf{Lag} & \\textbf{Initial values (by instance)} \\\\ \n  \\hline \\endhead\n  \\multicolumn{4}{r}{\\textit{Continued on next page...}} \\\\ \n  \\endfoot\n  \\endlastfoot\n" );
	}

	ol = new char[ 2 * strlen( r->label ) + 1 ];
	tex_strcpy( ol, r->label );
			
	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		vl = new char[ 2 * strlen( cv->label ) + 1 ];
		tex_strcpy( vl, cv->label );
		
		if ( cv->param == 1 )
		{
			fprintf( f, "  \\lsd{%s} & \\lsd{%s} & & %g", ol, vl, cv->val[ 0 ] );
			for ( j = 1, cur = r->hyper_next( r->label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
			{
				cv1 = cur->search_var( cur, cv->label );
				fprintf( f, ", %g", cv1->val[ 0 ] );
			}
			if ( j == MAX_INIT && cur != NULL )
				fprintf( f, ", ..." );
			  
			fprintf( f, " \\\\ \n  \\hline \n" );
		}
		else
		{
			for ( i = 0; i < cv->num_lag; ++i )
			{
				fprintf( f, "  \\lsd{%s} & \\lsd{%s} & %d & %g", ol, vl, i + 1, cv->val[ i ] );
				for ( j = 1, cur = r->hyper_next( r->label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
				{
					cv1 = cur->search_var( cur, cv->label );
					fprintf( f, ", %g", cv1->val[ i ] );
				}
				if ( j == MAX_INIT && cur != NULL )
					fprintf( f, ", ..." );
				  
				fprintf( f, " \\\\ \n  \\hline \n" );
			}
		}
		
		delete [ ] vl;
	}

	delete [ ] ol;
			
	for ( cb = r->b; cb != NULL; cb = cb->next )
		tex_report_initall( cb->head, f, table );

	if ( r->up == NULL )
		fprintf( f, "\\end{longtabu}\n\n" );
}


/****************************************************
 TEX_REPORT_END
 ****************************************************/
void tex_report_end( FILE *f )
{
	fprintf( f, "\\end{document}\n" );
}
