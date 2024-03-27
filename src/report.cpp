/*************************************************************

	LSD 9.0 - January 2024
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 REPORT.CPP
 This file contains the code for the report generating
 routines.

 The main function, (report) needs the pointer to the root of
 the model. It also needs the equation file name to be
 correctly set (if  the file does not exist, the routine ask
 for it). The output of the routine is a html file containing
 the main information on the model. This file is meant to be
 used as basic structure to be filled with modelers'
 comments in order to obtain the complete documentation of
 the model.

 The report lists all the objects, variables and parameters,
 all linked with pointers, so that readers can easily jump
 hyper-textually through the whole report.

 - Each object is listed with indication of its ancestors.
 - For each object, the set of variables and parameters is
   listed.
 - For each parameter is listed the set of variables whose
   equations make use of its value.
 - For each variables is listed the set of variables whose
   equations make use of its values, and also the whole set
   of variables and parameters used in the its own equation.
 *************************************************************/

#include "LSD.h"

#define MAX_INIT 100
#define TEX_PAPER "a4paper"
#define TEX_LEFT 2.5
#define TEX_RIGHT 2.5
#define TEX_TOP 2.5
#define TEX_BOTTOM 2.5

namespace lsd
{
	bool table;
	char path_rep[ MAX_PATH_LENGTH ] = "";
	char tmp_rep[ MAX_BUFF_SIZE ];
	int detail;
	int desc;
	int extra;
	int file_error;
	int ini;
	int lmenu;
	int obs;
	int pos;
	int tab_lines;
}


/*************************************************************
 REPORT
 *************************************************************/
void lsd::object::report( void )
{
	bool html2;
	char ch, fname[ MAX_PATH_LENGTH ];
	const char *app;
	int step, elemDone;
	FILE *f, *frep;

	file_error = 0;

	if ( ! sim->conf_ok )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create one before trying to create a report.\"" );
		return;
	}

	snprintf( sim->rep_file, MAX_PATH_LENGTH, "report_%s.html", strlen( sim->conf_name ) > 0 ? sim->conf_name : "model" );

	cmd( "set mrep %s", sim->rep_file );
	cmd( "set res [ file exists $mrep ]" );
	if ( gui::get_bool( "res" ) )
	{
		cmd( "set answer [ ttk::messageBox -parent . -message \"Model report already exists\" -detail \"Please confirm overwriting it.\" -type okcancel -title Warning -icon warning -default ok ]" );
		cmd( "if { ! [ string compare -nocase $answer ok ] } { set res 0 } { set res 1 }" );
		if ( gui::get_bool( "res" ) )
			return;

		gui::eval_str( "[ pwd ]", path_rep, MAX_PATH_LENGTH );
	}
	else
		strcpyn( path_rep, sim->conf_path, MAX_PATH_LENGTH );

	Tcl_LinkVar( gui::interp, "detail", ( char * ) &detail, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( gui::interp, "ini", ( char * ) &ini, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( gui::interp, "desc", ( char * ) &desc, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( gui::interp, "extra", ( char * ) &extra, TCL_LINK_BOOLEAN );
	Tcl_LinkVar( gui::interp, "obs", ( char * ) &obs, TCL_LINK_BOOLEAN );

	desc = true;
	obs = true;
	ini = true;
	detail = true;
	extra = false;
	cmd( "set reptit \"%s\"", strlen( sim->conf_name ) > 0 ? sim->conf_name : NO_CONF_NAME );
	cmd( "set lmenu 1" );
	cmd( "set html2 1" );
	cmd( "set tit2 \"Comments\"" );
	cmd( "set file2 \"comments.txt\"" );

	cmd( "newtop .w \"Report Generation\" { set choice 3 }" );

	cmd( "ttk::frame .w.f" );
	cmd( "ttk::label .w.f.l -text \"Model title\"" );
	cmd( "ttk::entry .w.f.e -width 30 -textvariable reptit -justify center" );
	cmd( "pack .w.f.l .w.f.e -side left" );

	cmd( "ttk::frame .w.l" );
	cmd( "ttk::label .w.l.l -text \"Variables presentation\"" );

	cmd( "ttk::frame .w.l.opt -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton .w.l.opt.popup -text \"List boxes\" -variable lmenu -value 1" );
	cmd( "ttk::radiobutton .w.l.opt.text -text \"Text lists\" -variable lmenu -value 0" );
	cmd( "pack .w.l.opt.popup .w.l.opt.text -anchor w" );

	cmd( "pack .w.l.l .w.l.opt" );

	cmd( "ttk::frame .w.g" );
	cmd( "ttk::label .w.g.l -text \"Included sections\"" );

	cmd( "ttk::frame .w.g.opt" );
	cmd( "ttk::checkbutton .w.g.opt.desc -text \"Description\" -variable desc" );
	cmd( "ttk::checkbutton .w.g.opt.extra -text \"User section\" -variable extra -command { if { $extra } { .w.s.e2.file.new conf -state normal; .w.s.e2.h conf -state normal; .w.s.e2.header.tit conf -state normal; .w.s.e2.file.tit conf -state normal } { .w.s.e2.file.new conf -state disabled; .w.s.e2.h conf -state disabled; .w.s.e2.header.tit conf -state disabled; .w.s.e2.file.tit conf -state disabled } }" );
	cmd( "ttk::checkbutton .w.g.opt.obs -text \"Selected variables\" -variable obs" );
	cmd( "ttk::checkbutton .w.g.opt.ini -text \"Initial values\" -variable ini" );
	cmd( "ttk::checkbutton .w.g.opt.detail -text \"Object details\" -variable detail" );
	cmd( "pack .w.g.opt.desc .w.g.opt.extra .w.g.opt.obs .w.g.opt.ini .w.g.opt.detail -anchor w" );

	cmd( "pack .w.g.l .w.g.opt" );

	cmd( "ttk::frame .w.s" );
	cmd( "ttk::label .w.s.lab -text \"User supplied section\"" );

	cmd( "ttk::frame .w.s.e2" );
	cmd( "ttk::checkbutton .w.s.e2.h -state disabled -text \"Use HTML tags\" -variable html2" );

	cmd( "ttk::frame .w.s.e2.header" );
	cmd( "ttk::label .w.s.e2.header.tlab -text \"Title\"" );
	cmd( "ttk::entry .w.s.e2.header.tit -width 30 -state disabled -textvariable tit2 -justify center" );
	cmd( "pack .w.s.e2.header.tlab .w.s.e2.header.tit -side left -padx 2" );

	cmd( "ttk::frame .w.s.e2.file" );
	cmd( "ttk::label .w.s.e2.file.tlab -text \"Get from file\"" );
	cmd( "ttk::entry .w.s.e2.file.tit -width 25 -state disabled -textvariable file2 -justify center" );
	cmd( "ttk::button .w.s.e2.file.new -width 5 -state disabled -text Search -command { set file2 [ tk_getOpenFile -parent .w -title \"Load Description File\" -filetypes {{{All files} {*}} } -initialdir \"%s\" ]; if [ fn_spaces \"$file2\" .w ] { set file2 \"\" } }", model_path );
	cmd( "pack .w.s.e2.file.tlab .w.s.e2.file.tit .w.s.e2.file.new -side left -padx 2" );

	cmd( "pack .w.s.e2.h .w.s.e2.header .w.s.e2.file -padx 5 -pady 2" );

	cmd( "pack .w.s.lab .w.s.e2" );

	cmd( "pack .w.f .w.l .w.g .w.s -padx 5 -pady 5" );

	cmd( "okXhelpcancel .w b Search { set res [ tk_getSaveFile -parent .w -title \"Existing Report File\" -filetypes { { {HTML files} {.html} } } -initialdir \"%s\" ]; set choice 2 } { set choice 1 } { LsdHelp menumodel.html#createreport } { set choice 3 }", model_path );

	cmd( "showtop .w topleftW" );
	cmd( "mousewarpto .w.b.ok" );

	here_create_report:

	gui::choice = 0;
	while ( gui::choice == 0 )
		Tcl_DoOneEvent( 0 );

	if ( gui::choice == 3 )
	{
		cmd( "destroytop .w" );
		goto end;
	}

	if ( gui::choice == 2 )
	{
		app = gui::eval_str( "[ file tail \"$res\" ]" );
		if ( app == NULL || strlen( app ) == 0 )
			goto here_create_report;

		strcpyn( sim->rep_file, app, MAX_PATH_LENGTH );
		gui::eval_str( "[ file dirname  \"$res\" ]", path_rep, MAX_PATH_LENGTH );
	}

	cmd( "destroytop .w" );

	cmd( "set eqf [ file join \"%s\" \"%s\" ]", model_path, gui::eq_file );

	while ( strlen( gui::eq_file ) == 0 || ( f = fopen( gui::get_str( "eqf" ), "r" ) ) == NULL )
	{
		cmd( "set answer [ ttk::messageBox -parent . -type okcancel -default ok -icon error -title Error -message \"Equation file '$eqf' not found\" -detail \"Press 'OK' to select another file.\"]" );
		cmd( "if [ string equal $answer ok ] { \
				set eqf [ tk_getOpenFile -parent . -title \"Load Equation File\" -initialdir [ pwd ] -filetypes { { {LSD Equation Files} {.cpp} } { {All Files} {*} } } ]; \
				if [ fn_spaces \"$eqf\" . ] { \
					set eqf \"\" \
				}; \
				set res 1 \
			} { \
				set res 0 \
			}" );

		if ( gui::get_bool( "res" ) )
		{
			app = gui::get_str( "eqf" );
			if ( app != NULL && strlen( app ) > 0 )
				strcpyn( gui::eq_file, app, MAX_PATH_LENGTH );
		}
		else
			goto end;
	}

	fclose( f );
	cmd( "set l $lmenu" );
	lmenu = gui::get_int( "l" );

	frep = create_frames( path_rep, sim->rep_file );

	if ( frep == NULL )
	{
		cmd( "ttk::messageBox -parent . -message \"Cannot write to disk\" -detail \"Please check if the model directory is not full or READ-ONLY.\" -type ok -title Error -icon error" );
		goto end;
	}

	gui::stop = false;
	step = 1;
	cmd( "set nElem [ llength $modElem ]" );
	cmd( "progressbox .prep \"Creating Report\" \"Report generation steps\" \"Step\" %d { set stop true } \".\" \"Element\" $nElem", desc + extra + obs + ini + detail );


	fprintf( frep, "<HTML>\n<HEAD> <META NAME=\"Author\" CONTENT=\"Automatically generated by LSD - Laboratory for Simulation Development, copyright by Marco Valente\">\n" );
	fprintf( frep, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\">  <style> table {border-collapse: collapse} td, th {border: 1px solid #dddddd; padding: 8px;} tr:nth-child(even) {background-color: #dddddd;} </style> </HEAD> <BODY>" );

	app = gui::get_str( "reptit" );

	fprintf( frep, "<TITLE>LSD Report - Model: \"%s\"</TITLE>", app );
	fprintf( frep, "<I>Automatically generated LSD report.</I><BR>" );
	fprintf( frep, "<A NAME=\"_TOP_\"><H1>Model: <I><U>%s</U></I></H1></A>", app );

	if ( desc || extra || obs )
		fprintf( frep, "<A NAME=\"_DESCRIPTION_\"><H2>Description</H2></A>" );

	if ( desc )
	{
		snprintf( fname, MAX_PATH_LENGTH, "%s/description.txt", model_path );
		f = fopen( fname, "r" );
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
			gui::plog( "\nFile description.txt not found. Description section skipped... " );
		}

		cmd( "prgboxupdate .prep %d", step++ );
	}

	if ( gui::stop )
		goto end_report;

	if ( extra )
	{
		app = gui::get_str( "file2" );
		if ( app == NULL )
			gui::plog( "\nMissing file name for user section. Skipped... " );
		else
		{
			f = fopen( app, "r" );
			if ( f != NULL )
			{
				app = gui::get_str( "tit2" );
				fprintf( frep, "<H3>%s</H3>", app );

				html2 = gui::get_bool( "html2" );
				for ( ch = fgetc( f ); ch != EOF; ch = fgetc( f ) )
				{
					if ( ! html2 )
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
				gui::plog( "\nFile %s not found. User section skipped... ", app );
			}
		}

		cmd( "prgboxupdate .prep %d", step++ );
	}

	if ( gui::stop )
		goto end_report;

	if ( obs )
	{
		int begin = 1;
		show_rep_initial( frep, & begin, frep );

		begin = 1;
		show_rep_observe( frep, & begin, frep );

		cmd( "prgboxupdate .prep %d", step++ );
	}

	if ( gui::stop )
		goto end_report;

	fprintf( frep, "<HR WIDTH=\"100%%\">\n" );

	fprintf( frep, "<A NAME=\"_MODEL_STRUCTURE_\"><H2>Model Structure</H2></A>\n" );
	fprintf( frep, "<H3>Object tree</H3>\n" );

	write_str( frep, 0, "" );
	write_list( frep, true, "" );
	create_table_init( frep );

	if ( ini )
	{
		fprintf( frep, "<HR WIDTH=\"100%%\">\n" );

		fprintf( frep, "<A NAME=\"_INITIALVALUES_\"><H2>Initial values</H2></A>\n" );
		fprintf( frep, "<H3>Object tree</H3>\n" );

		write_str( frep, 0, "_i_" );
		write_list( frep, true, "_i_" );

		create_initial_values( frep );

		cmd( "prgboxupdate .prep %d", step++ );
	}

	if ( gui::stop )
		goto end_report;

	if ( detail )
	{
		fprintf( frep, "<HR WIDTH=\"100%%\">\n" );

		fprintf( frep, "<A NAME=\"_DETAILS_\"><H2>Object detail</H2></A>\n" );
		fprintf( frep, "<H3>Object tree</H3>\n" );

		write_str( frep, 0, "_d_" );
		write_list( frep, true, "_d_" );

		cmd( "set app [ file tail \"%s\" ]", gui::eq_file );
		app = gui::get_str( "app" );
		fprintf( frep, "<BR><i>Equation file:</i> &nbsp;<TT><u>%s</u></TT><BR><BR>", app );

		elemDone = 0;
		write_obj( frep, &elemDone );

		cmd( "prgboxupdate .prep %d %d", step, elemDone );
	}

	end_report:

	fprintf( frep,"</BODY> </HTML>" );
	fclose( frep );

	cmd( "destroytop .prep" );

	if ( gui::stop )
	{
		cmd( "set fullFileName [ file join \"%s\" \"%s\" ]", path_rep, sim->rep_file );
		remove( gui::get_str( "fullFileName" ) );
		cmd( "set fullFileName [ file join \"%s\" \"head_%s\" ]", path_rep, sim->rep_file );
		remove( gui::get_str( "fullFileName" ) );
		cmd( "set fullFileName [ file join \"%s\" \"body_%s\" ]", path_rep, sim->rep_file );
		remove( gui::get_str( "fullFileName" ) );
	}
	else
	{
		if ( strlen( path_rep ) > 0 )
			gui::plog( "\nReport saved in file: %s/%s\n", path_rep, sim->rep_file );
		else
			gui::plog( "\nReport saved in file: %s\n", sim->rep_file );

		cmd( "open_browser \"%s\" \"%s\"", path_rep, sim->rep_file );
	}

	end:

	Tcl_UnlinkVar( gui::interp, "detail" );
	Tcl_UnlinkVar( gui::interp, "ini" );
	Tcl_UnlinkVar( gui::interp, "desc" );
	Tcl_UnlinkVar( gui::interp, "extra" );
	Tcl_UnlinkVar( gui::interp, "obs" );
}


/*************************************************************
 SHOW_REPORT
 *************************************************************/
void gui::show_report( const char *par_wnd )
{
	cmd( "set res [ open_browser \"%s\" \"%s\" ]", lsd::path_rep, sim.rep_file );

	if ( ! get_bool( "res" ) )
	{
		cmd( "set answer [ ttk::messageBox -parent %s -message \"Model report not found\" -detail \"You may create a model report file from menu Model or press 'OK' to look for another report file.\" -type okcancel -title Warning -icon warning -default cancel ]", par_wnd );
		cmd( "set res [ string equal $answer ok ]" );
		if ( ! get_bool( "res" ) )
			return;

		if ( strlen( lsd::path_rep ) > 0 )
			cmd( "set fname [ tk_getOpenFile -parent %s -title \"Load Report File\" -defaultextension \".html\" -initialdir \"%s\" -filetypes { {{HTML files} {.html}} } ]", par_wnd, lsd::path_rep );
		else
			cmd( "set fname [ tk_getOpenFile -parent %s -title \"Load Report File\" -defaultextension \".html\" -filetypes { {{HTML files} {.html}} } ]", par_wnd );

		cmd( "if { $fname == \"\" || [ fn_spaces \"$fname\" %s ] } { set res 0 } { set res 1 }", par_wnd );
		if ( ! get_bool( "res" ) )
			return;

		cmd( "open_browser \"\" \"$fname\"" );
	}
}


/*************************************************************
 WRITE_OBJ
 *************************************************************/
void lsd::object::write_obj( FILE *frep, int *elemDone )
{
	int count;
	bridge *cb;
	object *cur;
	variable *cv;

	for ( count = 0, cur = this; cur != NULL && ! gui::stop; cur = next_count( cur, & count ) )
	{
		fprintf( frep, "<HR WIDTH=\"100%%\">\n" );

		fprintf( frep, "<H3><A NAME=\"_d_%s\">Object: &nbsp;<TT><U>%s</U></TT></A></H3>\n", cur->label, cur->label );

		if ( cur->up != NULL )
		{
			fprintf( frep,"<i>Contained in: &nbsp;</i>" );
			cur->ancestors( frep );
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
		cur->write_list( frep, false, "_d_" );

		for ( cv = cur->v; cv != NULL && ! gui::stop; cv = cv->next )
		{
			cv->write_var( frep );
			cmd( "prgboxupdate .prep \"\" %d", ( *elemDone )++ );
		}

		if ( cur->b != NULL && ! gui::stop )
			cur->b->head->write_obj( frep, elemDone );
	}
}


/*************************************************************
 WRITE_VAR
 *************************************************************/
void lsd::variable::write_var( FILE *frep )
{
	bool one, found;
	char *app, c1_lab[ 2 * MAX_LINE_SIZE ], c2_lab[ 2 * MAX_LINE_SIZE ], c3_lab[ 2 * MAX_LINE_SIZE ], updt_in[ MAX_ELEM_LENGTH ];
	const char *fname;
	int i, j, k, done, flag_begin, flag_string, flag_comm, flag_var, nfiles;
	object *cur;
	variable *cv;
	FILE *ffun ;

	cmd( "update" );

	fprintf( frep, "<HR WIDTH=\"100%%\">\n" );

	if ( param == 0 )
		fprintf( frep, "<A NAME=\"_d_%s\"><H4>Variable: &nbsp;<TT><U>%s</U></TT></H4></A>", label, label );
	if ( param == 1 )
		fprintf( frep, "<A NAME=\"_d_%s\"><H4>Parameter: &nbsp;<TT><U>%s</U></TT></H4></A>", label, label );
	if ( param == 2 )
		fprintf( frep, "<A NAME=\"_d_%s\"><H4>Function: &nbsp;<TT><U>%s</U></TT></H4></A>", label, label );

	if ( num_lag > 0 )
		fprintf( frep, "<I>Lags: &nbsp;</I>%d<BR>", num_lag );

	if ( integer )
		fprintf( frep, "<I>Integer: &nbsp;</I>yes<BR>" );

	if ( ! std::isnan( min_val ) )
		fprintf( frep, "<I>Minimum: &nbsp;</I>%g<BR>", min_val );

	if ( ! std::isnan( max_val ) )
		fprintf( frep, "<I>Maximum: &nbsp;</I>%g<BR>", max_val );

	fprintf( frep, "<I>Contained in: &nbsp;</I><A HREF=\"#%s\"><TT>%s</TT></A><BR>", up->label, up->label );

	fprintf( frep, "<I>Used in: &nbsp;</I>" );

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", model_path );
	cmd( "if { [ lsearch -exact -nocase $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", gui::eq_file, gui::eq_file );
	cmd( "set res [ llength $source_files ]" );
	nfiles = gui::get_int( "res" );

	for ( one = false, k = 0; k < nfiles; ++k )
	{
		cmd( "set brr [ lindex $source_files %d ]", k );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", model_path, model_path );
		fname = gui::get_str( "brr" );

		if ( ( ffun = fopen( fname, "r" ) ) == NULL )
		{
			if ( ++file_error < ERR_LIM )
				gui::plog( "\nError opening file '%s': %s", fname, strerror( errno ) );
			continue;
		}

		while ( fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun ) != NULL )
		{
			if ( gui::eq_header( c1_lab, c2_lab, updt_in ) )
			{
				done = gui::eq_contains( ffun, label, strlen( label ) );

				if ( done )
				{
					fprintf( frep, "<TT>%s<A HREF=\"#_d_%s\">%s</A></TT>", one ? ", " : "", c2_lab,	 c2_lab );
					one = true;
				}
			}
		}

		fclose( ffun );
	}

	if ( ! one )
		fprintf( frep, "(never used)" );

	fprintf( frep, "<BR>\n" );

	if ( param == 0 || param == 2 )
	{
		fprintf( frep,"<I>Using: &nbsp;</I>" );

		found = false;
		sim->root->find_using( this, frep, & found );

		if ( ! found )
			fprintf( frep, "(none)" );

		fprintf( frep, "<BR>\n" );
	}

	fprintf( frep, "<BR>\n" );

	// print initial values
	if ( param == 1 || ( param == 0 && num_lag > 0 ) )
	{
		fprintf( frep,"<I>Initial values (by instance): &nbsp;</I>" );

		if ( param == 1 || num_lag == 1 )
		{
			fprintf( frep, "%g", val[ 0 ] );

			for ( j = 1, cur = up->hyper_next( up->label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
			{
				cv = cur->search_var( cur, label );
				fprintf( frep, ", %g", cv->val[ 0 ] );
			}

			if ( j == MAX_INIT && cur != NULL )
				fprintf( frep, ", ..." );

			fprintf( frep, "<BR>\n" );
		}
		else
		{
			for ( i = 0; i < num_lag; ++i )
			{
				fprintf( frep, "<P style=\"margin-left:10px;\"><I>Lag %d:</I> %g", i + 1, val[ i ] );

				for ( j = 1, cur = up->hyper_next( up->label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
				{
					cv = cur->search_var( cur, label );
					fprintf( frep, ", %g", cv->val[ i ] );
				}

				if ( j == MAX_INIT && cur != NULL )
					fprintf( frep, ", ..." );
			}

			fprintf( frep, "</P>\n" );
		}

		fprintf( frep, "<BR>\n" );
	}

	if ( param == 1 )
		return;

	for ( one = false, k = 0; ! one && k < nfiles; ++k )
	{
		cmd( "set brr [ lindex $source_files %d ]", k );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", model_path, model_path );
		fname = gui::get_str( "brr" );

		if ( ( ffun = fopen( fname, "r" ) ) == NULL )
		{
			if ( ++file_error < ERR_LIM )
				gui::plog( "\nError opening file '%s': %s", fname, strerror( errno ) );
			continue;
		}

		flag_comm = 0;

		while ( fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun ) != NULL  )
		{
			if ( gui::eq_header( c1_lab, c2_lab, updt_in ) )
			{
				if ( ! gui::macro )
					done = 0;
				else
					done = 1;		// will never stop with {} only

				if ( ! strcmp( c2_lab, label ) )
				{
					found = false;
					one = true;

					if ( k == 0 )
						fprintf( frep,"<I>Equation code:</I><BR>\n" );
					else
						fprintf( frep,"<I>Equation code</I> <TT>(%s)</TT>:<BR>\n", fname );

					fprintf( frep, "<BR>\n<TT>%s</TT>", c1_lab );

					while ( done > 0 || ! found )
					{
						found = true;
						fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun );
						strcpyn( c3_lab, c1_lab, 2 * MAX_LINE_SIZE );
						gui::clean_spaces( c3_lab );

						// handle dummy equations without RESULT closing
						if ( gui::eq_dum && ( ! strncmp( c1_lab, "EQUATION(", 9 ) || ! strncmp( c1_lab, "EQUATION_DUMMY(", 15 ) || ! strncmp( c1_lab, "FUNCTION(", 9 ) || ! strncmp( c1_lab, "MODELEND", 8 ) ) )
						{
							if ( strlen( updt_in ) > 0 )
								snprintf( c1_lab, 2 * MAX_LINE_SIZE, "(DUMMY EQUATION: variable '%s' updated in '%s')", label, updt_in );
							else
								snprintf( c1_lab, 2 * MAX_LINE_SIZE, "(DUMMY EQUATION: variable '%s' not updated here)", label );

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

						if ( ! strncmp( c3_lab, "RESULT(", 7 ) && gui::macro )
							done = 0;		// force it to stop
					}

					fprintf( frep, "</FONT><BR><BR>\n" );
					break;
				}
			}
		}

		fclose( ffun );
	}

	if ( ! one )
		fprintf( frep,"<I>Equation code</I>: (not available)<BR><BR>\n" );
}


/*************************************************************
 FIND_USING
 *************************************************************/
void lsd::object::find_using( variable *v, FILE *frep, bool *found )
{
	bool one;
	int done, i, nfiles;
	char c1_lab[ 2 * MAX_LINE_SIZE ], c2_lab[ 2 * MAX_LINE_SIZE ], updt_in[ MAX_ELEM_LENGTH ];
	const char *fname;
	object *cur;
	variable *cv;
	FILE *ffun;

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", model_path );
	cmd( "if { [ lsearch -exact -nocase $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", gui::eq_file, gui::eq_file );
	cmd( "set res [ llength $source_files ]" );
	nfiles = gui::get_int( "res" );

	// first check if variable has a dummy equation and abort if so
	for ( i = 0; i < nfiles; ++i )
	{
		cmd( "set brr [ lindex $source_files %d ]", i );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", model_path, model_path );
		fname = gui::get_str( "brr" );

		if ( ( ffun = fopen( fname, "r" ) ) == NULL )
		{
			if ( ++file_error < ERR_LIM )
				gui::plog( "\nError opening file '%s': %s", fname, strerror( errno ) );
			continue;
		}

		while ( fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun ) != NULL )
			if ( gui::eq_header( c1_lab, c2_lab, updt_in ) )
				if ( gui::eq_dum && ! strcmp( c2_lab, v->label ) )
				{
					fclose( ffun );
					return;
				}

		fclose( ffun );
	}

	// now search for all elements in all objects in all files
	for ( one = false, cur = this; cur != NULL; cur = next_obj( cur ) )
	{
		for ( cv = cur->v; cv != NULL; cv = cv->next )
		{
			cmd( "set usingList [ list ]" );

			for ( i = 0; i < nfiles; ++i )
			{
				cmd( "set brr [ lindex $source_files %d ]", i );
				cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", model_path, model_path );
				fname = gui::get_str( "brr" );

				if ( ( ffun = fopen( fname, "r" ) ) == NULL )
				{
					if ( ++file_error < ERR_LIM )
						gui::plog( "\nError opening file '%s': %s", fname, strerror( errno ) );
					continue;
				}

				while ( fgets( c1_lab, 2 * MAX_LINE_SIZE, ffun ) != NULL )
				{
					if ( gui::eq_header( c1_lab, c2_lab, updt_in ) )
					{
						done = false;
						if ( ! strcmp( c2_lab, v->label ) )
							done = gui::eq_contains( ffun, cv->label, strlen( cv->label ) );
						if ( done )
						{
							// avoid duplicated variable equations
							cmd( "if { [ lsearch -exact $usingList %s ] >= 0 } { set res 1 } { set res 0 }", cv->label );
							if ( gui::get_int( "res", & done ) )
								continue;
							else
								cmd( "lappend usingList %s", cv->label );

							if ( frep != NULL )
							{
								fprintf( frep, "<TT>%s<A HREF=\"#_d_%s\">%s</A></TT>", one ? ", " : "", cv->label, cv->label );
								one = true;
							}
							else
								cmd( "$list.l.l insert end %s", cv->label );

							*found = true;
						}
					}
				}

				fclose( ffun );
			}
		}

		if ( cur->b != NULL )
			cur->b->head->find_using( v, frep, found );
	}
}


/*************************************************************
 EQ_CONTAINS
 scans an equation checking if it contains anywhere the string
 lab between quotes. Returns 1 if found, and 0 otherwise.
 The file passed is moved to point to the next equation
 It correctly skip the commented text, either by // or
 by / * ... * /
 *************************************************************/
bool gui::eq_contains( FILE *f, const char *lab, int len )
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

		lsd::strcpyn( pot, c1_lab, MAX_LINE_SIZE );
		clean_spaces( pot );

		if ( ! strncmp( pot, "RESULT(", 7 ) )
			bra--;

		for ( i = 0; c1_lab[ i ] != 0; ++i ) // scans each character
		{
			if ( c1_lab[ i ] == '{' )		// if it is an open bracket
			{
				bra++;
				start = 0;
			}
			else
				if ( c1_lab[ i ] == '}' )	// if it is a closed bracket
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
						if ( pot[ 0 ] != '\"' )				// in case the eq. contains "" it gets fucked up..
							for ( j = 0; got == 1 && pot[ j ] != '\"' && lab != NULL; ++j )
								if ( pot[ j ] != lab[ j ])
									got = 0;

						for ( ; pot[ j ] != '\"'; ++j );	// finishes the word, until the closed quotes

						i = i + j + 1;

						if ( got == 1 && j == len )
							found = true;
					}
				}
		}
	}

	return found;
}


/*************************************************************
 WRITE_STR
 *************************************************************/
void lsd::object::write_str( FILE *frep, int dep, const char *prefix )
{
	int len, i, j, count = 0;
	bridge *cb;

	if ( up != NULL )
	{
		if ( up->b->head != this )
			for ( i = 0; i < dep; ++i )
			{
				if ( tmp_rep[ i ] == ' ' )
					fprintf( frep, "<TT>&nbsp;</TT>" );
				else
					fprintf( frep, "<TT>|</TT>" );
			}

		fprintf( frep, "<TT>&mdash;&gt;</TT>" );

		if ( next_count( this, & count ) != NULL )
			tmp_rep[ dep ] = '|';
		else
			tmp_rep[ dep ] = ' ';

		tmp_rep[ ++dep ] = ' ';
		count = 1;
	}

	fprintf( frep, "<TT><A HREF=\"#%s%s\">%s</A></TT>", prefix, label, label );
	len = strlen( label );
	for ( i = 0; i < len; ++i )
		tmp_rep[ dep + i + count ] = ' ';
	dep = dep + len + count;
	j = dep;

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->write_str( frep, j, prefix );

	if ( b == NULL )
	{
		fprintf( frep, "<BR>\n" );
		for ( i = 0; i < dep; ++i )
			if ( tmp_rep[ i ] == ' ' )
				fprintf( frep, "<TT>&nbsp;</TT>" );
			else
				fprintf( frep, "<TT>|</TT>" );

		fprintf( frep, "<BR>\n" );
	}
}


/*************************************************************
 WRITE_LIST
 *************************************************************/
void lsd::object::write_list( FILE *frep, bool show_all, const char *prefix )
{
	int num, i;
	char s1[ 2 * MAX_ELEM_LENGTH ], s2[ MAX_ELEM_LENGTH ];

	Tcl_LinkVar( gui::interp, "num", ( char * ) &num, TCL_LINK_INT );

	if ( show_all )							// initial listing?
		fprintf( frep, "<H3>Variables</H3>\n" );
	else
		fprintf( frep, "<i>Variables: &nbsp;</i>" );

	cmd( "set rawlist [ list ]" );			// create an empty list

	if ( strcmp( prefix, "_i_" ) == 0 )
		fill_list_var( show_all, true ); 	// insert only lagged variables
	else
		fill_list_var( show_all, false );	// insert all the variables

	cmd( "set alphalist [ lsort -dictionary $rawlist ]" );
	cmd( "set num [ llength $alphalist ]" );

	// distinguish the case you are compiling the initial list of element (all) or for a single Object)
	if ( ! show_all )
		snprintf( s1, 2 * MAX_ELEM_LENGTH, "form_v_%s_%s", label, prefix );
	else
		snprintf( s1, 2 * MAX_ELEM_LENGTH, "form_v_all_%s_%s", label, prefix );

	if ( lmenu )
		create_form( num, s1, prefix, frep );
	else
		for ( i = 0; i < num; ++i )
		{
			cmd( "set app [ lindex $alphalist %d ]", i );
			sscanf( gui::get_str( "app" ), "%s %s", s1, s2);
			fprintf( frep, "<TT><A HREF=\"#%s%s\">%s</A></TT>", prefix, s1, s1 );

			if ( i < num - 1 )
				fprintf( frep, "<TT>, </TT>" );
		}

	if ( num == 0 )
		fprintf( frep, "(none)<BR>\n" );
	else
		fprintf( frep, "<BR>\n" );

	if ( show_all )							// initial listing
		fprintf( frep, "<H3>Parameters</H3>\n" );
	else
		fprintf( frep, "<i>Parameters: &nbsp;</i>" );

	cmd( "lappend rawlist" );				// create the list if not existed
	cmd( "unset rawlist" );					// empty the list
	cmd( "lappend rawlist" );				// create a surely empty list

	fill_list_par( show_all );

	cmd( "set alphalist [ lsort -dictionary $rawlist ]" );
	cmd( "set num [ llength $alphalist ]" );

	// distinguish the case you are compiling the initial list of element (all) or for a single Object)
	if ( ! show_all )
		snprintf( s1, 2 * MAX_ELEM_LENGTH, "form_p_%s_%s", label, prefix );
	else
		snprintf( s1, 2 * MAX_ELEM_LENGTH, "form_p_all_%s_%s", label, prefix );

	if ( lmenu )
		create_form( num, s1, prefix, frep );
	else
		for ( i = 0; i < num; ++i )
		{
			cmd( "set app [ lindex $alphalist %d ]", i );
			fprintf( frep, "<TT><A HREF=\"#%s%s\">%s</A></TT>", prefix, gui::get_str( "app" ), gui::get_str( "app" ) );

			if ( i < num - 1 )
				fprintf( frep, "<TT>, </TT>" );
		}

	if ( num == 0 )
		fprintf( frep, "(none)<BR>\n" );
	else
		fprintf( frep, "<BR>\n" );

	if ( ! show_all )
		fprintf( frep, "<BR>\n" );

	Tcl_UnlinkVar( gui::interp, "num" );
}

/*************************************************************
 FILL_LIST_VAR
 *************************************************************/
void lsd::object::fill_list_var( bool show_all, bool lag_only )
{
	bridge *cb;
	variable *cv;

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		if ( ( cv->param == 0 || cv->param == 2 ) && ( ! lag_only || cv->num_lag > 0 ) )
		cmd( "lappend rawlist \"%s (%d)\"", cv->label, cv->num_lag );
	}

	if ( ! show_all )
		return;

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->fill_list_var( show_all, lag_only );
}


/*************************************************************
 FILL_LIST_PAR
 *************************************************************/
void lsd::object::fill_list_par( bool show_all )
{
	bridge *cb;
	variable *cv;

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		if ( cv->param == 1 )
			cmd( "lappend rawlist \"%s\"", cv->label );
	}

	if ( ! show_all )
		return;

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->fill_list_par( show_all );
}


/*************************************************************
 CREATE_TABLE_INIT
 Create recursively the help table for an Object
 *************************************************************/
void lsd::object::create_table_init( FILE *frep )
{
	char min_val[ 32 ], max_val[ 32 ];
	int i;
	bridge *cb;
	description *cd;
	variable *cv;

	fprintf( frep, "<HR WIDTH=\"100%%\">\n" );

	fprintf( frep, "<H3><A NAME=\"%s\">Object: &nbsp;<TT><U>%s</U></TT></A></H3>", label, label );

	if ( up != NULL )
	{
		fprintf( frep, "<i>Contained in: &nbsp;</i>" );
		ancestors( frep );
		fprintf( frep, "<BR>\n" );
	}

	if ( b != NULL )
	{
		fprintf( frep, "<i>Containing: &nbsp;</i>" );
		fprintf( frep, "<TT><a HREF=\"#%s\">%s</a></TT>", b->blabel, b->blabel );
		for ( cb = b->next ; cb != NULL; cb = cb->next )
			fprintf( frep, "<TT>,  <a HREF=\"#%s\">%s</a></TT>", cb->blabel, cb->blabel );
		fprintf( frep, "<BR>\n" );
	}

	fprintf( frep, "<BR>\n" );
	write_list( frep, false, "" );

	cd = sim->search_description( label );
	if ( cd->has_descr_text ( ) )
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

	if ( v != NULL )
	{
		fprintf( frep, "<table BORDER>" );
		fprintf( frep, "<tr>" );
		fprintf( frep, "<td><center><i>Element</i></center></td>" );
		fprintf( frep, "<td><center><i>Lags</i></center></td>\n" );
		fprintf( frep, "<td><center><i>Int.</i></center></td>\n" );
		fprintf( frep, "<td><center><i>Min.</i></center></td>\n" );
		fprintf( frep, "<td><center><i>Max.</i></center></td>\n" );
		fprintf( frep, "<td><center><i>Description and initial values comments</i></center></td>\n" );
		fprintf( frep, "</tr>" );

		for ( cv = v; cv != NULL; cv = cv->next )
		{
			fprintf( frep, "<tr VALIGN=TOP>" );
			fprintf( frep, "<td><a NAME=\"%s\"><A HREF=\"#_d_%s\"><TT>%s</TT></A></a></td>\n", cv->label, cv->label, cv->label );

			if ( cv->param == 1 )
				fprintf( frep, "<td><A HREF=\"#_i_%s\">Par.</A></td>\n", cv->label );
			else
				if ( cv->num_lag > 0 )
					fprintf( frep, "<td><A HREF=\"#_i_%s\">%d</A></td>\n", cv->label, cv->num_lag );
				else
					fprintf( frep, "<td></td>\n" );

			if ( std::isnan( cv->min_val ) )
				strcpy( min_val, "" );
			else
				snprintf( min_val, 32, "%g", cv->min_val );

			if ( std::isnan( cv->max_val ) )
				strcpy( max_val, "" );
			else
				snprintf( max_val, 32, "%g", cv->max_val );

			fprintf( frep, "<td>%s</td><td>%s</td><td>%s</td>\n", cv->integer ? "yes" : "", min_val, max_val );

			cd = sim->search_description( cv->label );

			fprintf( frep, "<td> " );
			bool desc_text = false;
			if ( cd->has_descr_text ( ) )
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

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->create_table_init( frep );
}


/*************************************************************
 CREATE_INITIAL_VALUES
 Create recursively the help table
 for the initial values of an Object
 *************************************************************/
void lsd::object::create_initial_values( FILE *frep )
{
	int count = 0, i, j;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	for ( cv = v; cv != NULL; cv = cv->next )
		if ( cv->param == 1 || cv->num_lag > 0 )
			count = 1;

	if ( count == 0 )
	{	// no need for initialization, typically root
		for ( cb = b; cb != NULL; cb = cb->next )
			cb->head->create_initial_values( frep );
		return;
	}

	for ( count = 0, cur = this; cur != NULL; cur = cur->hyper_next( cur->label ) )
		count++;

	fprintf( frep, "<HR WIDTH=\"100%%\">\n" );
	fprintf( frep, "<H3><A NAME=\"%s\">Object: &nbsp;<TT><U>%s</U></TT></A></H3>", label, label );
	fprintf( frep, "<i>Instance number: &nbsp</i>%d<BR>", count );
	fprintf( frep, "<i>Instance group(s): &nbsp;</i>" );

	for ( i = 0, cur = this; cur != NULL; )
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

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		if ( cv->param == 1 )
		{
			fprintf( frep, "<tr VALIGN=TOP>" );
			fprintf( frep, "<td><a NAME=\"_i_%s\"><A HREF=\"#%s\"><TT>%s</TT></A></a></td>\n", cv->label, cv->label, cv->label );
			fprintf( frep, "<td>Par.</td>\n" );
			fprintf( frep, "<td>%g", cv->val[ 0 ] );

			for ( j = 1, cur = hyper_next( label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
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

				for ( j = 1, cur = hyper_next( label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
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

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->create_initial_values( frep );
}


/*************************************************************
 IS_EQUATION_HEADER
 Squeeze the spaces out of line and
 returns 1 if the line is an equation
 header, placing the Variable label
 in Var
 *************************************************************/
bool gui::eq_header( const char *raw_line, char *var, char *updt_in )
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


/*************************************************************
 CREATE_FRAMES
 *************************************************************/
FILE *lsd::object::create_frames( const char *dest_path, const char *fname )
{
	FILE *f;

	cmd( "set fullFileName [ file join \"%s\" \"%s\" ]", dest_path, fname );
	f = fopen( gui::get_str( "fullFileName" ), "w" );
	if ( f == NULL )
		return NULL;

	fprintf( f, "<html> <head> <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\"> <meta name=\"AUTHOR\" content=\"Marco Valente\"> </head>\n" );
	fprintf( f, "<FRAMESET framespacing=0 border=1 rows=\"30,*\"> <FRAME NAME=\"testa\" SRC=\"head_%s\"", fname );

	fprintf( f, "frameborder=0 MARGINHEIGHT=\"0\" MARGINWIDTH=\"0\" SCROLLING=\"NO\" NORESIZE> <FRAME NAME=\"body\" SRC=\"body_%s\" frameborder=0 MARGINHEIGHT=\"0\" MARGINWIDTH=\"0\" SCROLLING=\"AUTO\"> </FRAMESET> <NOFRAMES>  <p>Use a frame-enabled browser</p> </NOFRAMES> </html>", fname );

	fclose( f );

	cmd( "set fullFileName [ file join \"%s\" \"head_%s\" ]", dest_path, fname );
	f = fopen( gui::get_str( "fullFileName" ), "w" );
	if ( f == NULL )
		return NULL;

	fprintf( f, "<html> <head> </head > <body bgcolor=\"#D6A9FE\"> <center>" );
	fprintf( f, "<a href=\"body_%s#_TOP_\" target=body>Top</a> &nbsp", fname );
	if ( desc || extra )
		fprintf( f, "<a href=\"body_%s#_DESCRIPTION_\" target=body>Description</a> &nbsp", fname );
	fprintf( f, "<a href=\"body_%s#_MODEL_STRUCTURE_\" target=body>Structure</a> &nbsp", fname );
	if ( ini )
		fprintf( f, "<a href=\"body_%s#_INITIALVALUES_\" target=body>Initialization</a> &nbsp", fname );
	if ( detail )
		fprintf( f, "<a href=\"body_%s#_DETAILS_\" target=body>Code</a> &nbsp", fname );

	fprintf( f, "</body> </html>" );
	fclose( f );

	cmd( "set fullFileName [ file join \"%s\" \"body_%s\" ]", dest_path, fname );
	f = fopen( gui::get_str( "fullFileName" ), "w" );

	return f;
}


/*************************************************************
 CREATE_FORM
 create list forms for labels.
 *************************************************************/
void lsd::object::create_form( int num, const char *title, const char *prefix, FILE *frep )
{
	int i;
	char s1[ 2 * MAX_ELEM_LENGTH ], s2[ 2 * MAX_ELEM_LENGTH ];

	if ( num == 0 )
		return;

	fprintf( frep, "<form name=\"%s\" method=\"POST\">\n", title );
	fprintf( frep, "<select name=\"entry\" class=\"form\">\n" );

	for ( i = 0; i < num; ++i )
	{
		cmd( "set app [ lindex $alphalist %d ]", i );
		sscanf( gui::get_str( "app" ), "%s %s", s1, s2 );
		fprintf( frep, "<option value=\"#%s%s\">%s</option>\n", prefix, s1, s1 );
	}

	fprintf( frep, "</select>\n" );
	fprintf( frep, "<INPUT TYPE=button VALUE=\"Show\" onClick=\"location.href = document.%s.entry.options[document.%s.entry.selectedIndex].value\">\n", title, title );
	fprintf( frep, "</form>" );
}


/*************************************************************
 SHOW_REP_OBSERVE
 *************************************************************/
void lsd::object::show_rep_observe( FILE *f, int *begin, FILE *frep )
{
	int i;
	bridge *cb;
	description *cd;
	variable *cv;

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		cd = sim->search_description( cv->label );
		if ( cd->observe )
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
			fprintf( f, "<td><TT><A HREF=\"#%s\">%s</A></TT></td>", label, label );

			if ( cv->param == 1 )
				fprintf( f, "<td>Parameter</td>" );
			if ( cv->param == 0 )
				fprintf( f, "<td>Variable</td>" );
			if ( cv->param == 2 )
				fprintf( f, "<td>Function</td>" );

			fprintf( f, "<td>" );

			if ( cd->has_descr_text ( ) )
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

for ( cb = b; cb != NULL; cb = cb->next )
	cb->head->show_rep_observe( f, begin, frep );

if ( up == NULL && table )
	fprintf( f, "</table><BR>\n" );
}


/*************************************************************
 SHOW_REP_INITIAL
 *************************************************************/
void lsd::object::show_rep_initial( FILE *f, int *begin, FILE *frep )
{
	int i;
	bridge *cb;
	description *cd;
	variable *cv;

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		cd = sim->search_description( cv->label );
		if ( cd->initial )
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
			fprintf( f, "<td><TT><A HREF=\"#%s\">%s</A></TT></td>", label, label );

			if ( cv->param == 1 )
				fprintf( f, "<td>Parameter</td>" );
			if ( cv->param == 0 )
				fprintf( f, "<td>Variable</td>" );
			if ( cv->param == 2 )
				fprintf( f, "<td>Function</td>" );

			fprintf( f, "<td>" );

			bool desc_text = false;
			if ( cd->has_descr_text ( ) )
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

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->show_rep_initial( f, begin, frep );

	if ( up == NULL && table )
		fprintf( f, "</table><BR>\n" );
}


/*************************************************************
 TEX_STRCPY
 *************************************************************/
char *tex_strcpy( char *&out, char *in )
{
	int i, j;

	// handle active underscores and other special chars in label references
	for ( i = 0, j = 0; in[ j ] != '\0'; ++j )
		if ( in[ j ] == '_' || in[ j ] == '^' || in[ j ] == '$' || in[ j ] == '&' )
			++i;

	if ( i > 0 )
	{
		delete [ ] out;
		out = new char[ 2 * strlen( in ) + 1 + i * 6 ];
	}

	for ( i = 0, j = 0; in[ j ] != '\0'; ++i, ++j )
		switch ( in[ j ] )
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
				memcpy( out + i, "\\string", 7 );
				i += 7;
			default:
				out[ i ] = in[ j ];
		}

	out[ i ] = '\0';

	return out;
}


/*************************************************************
 TEX_FPRINTF
 *************************************************************/
void lsd::object::tex_fprintf( FILE *f, char* text )
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


/*************************************************************
 ANCESTORS
 *************************************************************/
void lsd::object::ancestors( FILE *f, bool html )
{
	char *ol;

	if ( up != NULL )
	{
		up->ancestors( f, html );
		ol = new char[ 2 * strlen( up->label ) + 1 ];
		tex_strcpy( ol, up->label );

		if ( up->up == NULL )
			if ( html )
				fprintf( f, "<TT><A HREF=\"#%s\">%s</A></TT>", up->label, up->label );
			else
				fprintf( f, "\\hrf{%s}{%s}", ol, up->label );
		else
			if ( html )
				fprintf( f, "<TT>&mdash;&gt;<A HREF=\"#%s\">%s</A></TT>", up->label, up->label );
			else
				fprintf( f, "$\\rightarrow$\\hrf{%s}{%s}", ol, up->label );

		delete [ ] ol;
	}
}


/*************************************************************
 TEX_REPORT_HEAD
 *************************************************************/
void lsd::object::tex_report_head( FILE *f, bool table )
{
	fprintf( f, "\\documentclass{article}\n\n" );
	fprintf( f, "\\usepackage[%s,left=%fcm,right=%fcm,top=%fcm,bottom=%fcm]{geometry}\n", TEX_PAPER, TEX_LEFT, TEX_RIGHT, TEX_TOP, TEX_BOTTOM );
	fprintf( f, "\\usepackage{color}\n" );
	fprintf( f, "\\usepackage[colorlinks=true,linkcolor=blue,pdfborder={0 0 0}]{hyperref}\n\n" );

	if ( table )
		fprintf( f, "\\usepackage{longtable}\n\n" );

	fprintf( f, "\\newcommand{\\hrf}[2] {\\hyperref[#1]{\\texttt{\\color{blue}{\\detokenize{#2}}}}}\n" );
	fprintf( f, "\\setlength{\\parindent}{0cm}\n\n" );

	fprintf( f, "\\title{Model: %s}\n", strlen( sim->conf_name ) > 0 ? sim->conf_name : NO_CONF_NAME );
	fprintf( f, "\\author{Automatically generated LSD report}\n" );
	fprintf( f, "\\date{}\n\n" );

	fprintf( f, "\\begin{document}\n\n" );
	fprintf( f, "\\maketitle\n\n" );
}


/*************************************************************
 TEX_REPORT_STRUCT
 *************************************************************/
void lsd::object::tex_report_struct( FILE *f, bool table )
{
	char *ol, *vl, min_val[ 32 ], max_val[ 32 ];
	bridge *cb;
	description *cd;
	variable *cv;

	if ( up == NULL )
		fprintf( f, "\\section{Model Structure}\n\n" );

	ol = new char[ 2 * strlen( label ) + 1 ];
	tex_strcpy( ol, label );
	fprintf( f, "\\subsection{Object: %s} \\label{%s}\n\n", label, ol );
	delete [ ] ol;

	if ( up != NULL )
	{
		fprintf( f,"\\emph{Contained in:} " );
		ancestors( f, false );
		fprintf( f,"\n\n" );
	}

	if ( b != NULL )
	{
		ol = new char[ 2 * strlen( b->blabel ) + 1 ];
		tex_strcpy( ol, b->blabel );
		fprintf( f,"\\emph{Containing:} \\hrf{%s}{%s}", ol, b->blabel );
		delete [ ] ol;

		for ( cb = b->next; cb != NULL; cb = cb->next )
		{
			ol = new char[ 2 * strlen( cb->blabel ) + 1 ];
			tex_strcpy( ol, cb->blabel );
			fprintf( f, ",	\\hrf{%s}{%s}", ol, cb->blabel );
			delete [ ] ol;
		}

		fprintf( f, "\n\n" );
	}

	cd = sim->search_description( label );
	if ( cd->has_descr_text ( ) )
		fprintf( f, "\\emph{Description:}\n\\detokenize{%s}\n\n", cd->text );

	if ( v != NULL )
	{
		if ( ! table )
			fprintf( f,"\\emph{Contained elements:}\n\n" );
		else
			fprintf( f, "\\begin{longtable}{*{6}{|l}|p{7cm}|}\n	 \\hline\n	\\textbf{Element} & \\textbf{Type} & \\textbf{Lags} & \\textbf{Int.} & \\textbf{Min.} & \\textbf{Max.} & \\textbf{Description and initial values comments} \\\\ \n	 \\hline \\endhead\n  \\multicolumn{7}{r}{\\textit{Continued on next page...}} \\\\ \n	\\endfoot\n	 \\endlastfoot\n" );
	}

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		vl = new char[ 2 * strlen( cv->label ) + 1 ];
		tex_strcpy( vl, cv->label );

		if ( ! table )
		{
			fprintf( f, "  %s \\label{%s}", cv->label, vl );

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
			fprintf( f, "  \\hrf{%s_init}{%s} \\label{%s}", vl, cv->label, vl );

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

			if ( std::isnan( cv->min_val ) )
				strcpy( min_val, "" );
			else
				snprintf( min_val, 32, "%g", cv->min_val );

			if ( std::isnan( cv->max_val ) )
				strcpy( max_val, "" );
			else
				snprintf( max_val, 32, "%g", cv->max_val );

			fprintf( f, "%s & %s & %s & ", cv->integer ? "yes" : "", min_val, max_val );
		}

		delete [ ] vl;
		bool desc_text = false;
		cd = sim->search_description( cv->label );
		if ( cd->has_descr_text ( ) )
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

	if ( v != NULL )
	{
		if ( table )
			fprintf( f, "\\end{longtable}\n\n" );
		else
			fprintf( f, "\n" );
	}

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->tex_report_struct( f, table );
}


/*************************************************************
 TEX_REPORT_OBSERVE
 *************************************************************/
void lsd::object::tex_report_observe( FILE *f, bool table )
{
	char *ol, *vl;
	bridge *cb;
	description *cd;
	variable *cv;

	if ( up == NULL )
	{
		fprintf( f, "\\section{Relevant elements to observe}\n\n" );
		tab_lines = 0;
		if ( table )
			fprintf( f, "\\begin{longtable}{*{3}{|l}|p{9cm}|}\n	 \\hline\n	\\textbf{Element} & \\textbf{Object} & \\textbf{Type} & \\textbf{Description} \\\\ \n  \\hline \\endhead\n	\\multicolumn{4}{r}{\\textit{Continued on next page...}} \\\\ \n  \\endfoot\n  \\endlastfoot\n" );
	}

	ol = new char[ 2 * strlen( label ) + 1 ];
	tex_strcpy( ol, label );

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		cd = sim->search_description( cv->label );
		if ( cd->observe )
		{
			vl = new char[ 2 * strlen( cv->label ) + 1 ];
			tex_strcpy( vl, cv->label );

			if ( ! table )
				fprintf( f, "\\hrf{%s}{%s} (object \\hrf{%s}{%s}) \\newline \n", vl, cv->label, ol, label );
			else
			{
				fprintf( f, "  \\hrf{%s}{%s} & \\hrf{%s}{%s} & ", vl, cv->label, ol, label );
				if ( cv->param == 0 )
					fprintf( f, "Variable & " );
				if ( cv->param == 1 )
					fprintf( f, "Parameter & " );
				if ( cv->param == 2 )
					fprintf( f, "Function & " );
				if ( cd->has_descr_text ( ) )
					tex_fprintf( f, cd->text );
				fprintf( f, "\\\\ \n  \\hline \n" );
			}

			++tab_lines;

			delete [ ] vl;
		}
	}

	delete [ ] ol;

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->tex_report_observe( f, table );

	if ( up == NULL )
	{
		if ( table )
		{
			if ( tab_lines == 0 )
				fprintf( f, "  \\multicolumn{4}{|c|}{(none selected)} \\\\ \n  \\hline \n" );
			fprintf( f, "\\end{longtable}\n\n" );
		}
		else
			if ( tab_lines == 0 )
				fprintf( f, "(none selected) \\newline \n" );
			else
				fprintf( f, "\n" );
	}
}


/*************************************************************
 TEX_REPORT_INIT
 *************************************************************/
void lsd::object::tex_report_init( FILE *f, bool table )
{
	char *ol, *vl;
	bridge *cb;
	description *cd;
	variable *cv;

	if ( up == NULL )
	{
		fprintf( f, "\\section{Relevant elements to initialize}\n\n" );
		tab_lines = 0;
		if ( table )
			fprintf( f, "\\begin{longtable}{*{3}{|l}|p{9cm}|}\n	 \\hline\n	\\textbf{Element} & \\textbf{Object} & \\textbf{Type} & \\textbf{Description and initial values comments} \\\\ \n  \\hline \\endhead\n	\\multicolumn{4}{r}{\\textit{Continued on next page...}} \\\\ \n  \\endfoot\n  \\endlastfoot\n" );
	}

	ol = new char[ 2 * strlen( label ) + 1 ];
	tex_strcpy( ol, label );

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		cd = sim->search_description( cv->label );
		if ( cd->initial )
		{
			vl = new char[ 2 * strlen( cv->label ) + 1 ];
			tex_strcpy( vl, cv->label );

			if ( ! table )
				fprintf( f, "\\hrf{%s}{%s} (object \\hrf{%s}{%s}) \\newline \n", vl, cv->label, ol, label );
			else
			{
				fprintf( f, "  \\hrf{%s_init}{%s} & \\hrf{%s}{%s} & ", vl, cv->label, ol, label );
				if ( cv->param == 0 )
					fprintf( f, "Variable & " );
				if ( cv->param == 1 )
					fprintf( f, "Parameter & " );
				if ( cv->param == 2 )
					fprintf( f, "Function & " );

				bool desc_text = false;
				if ( cd->has_descr_text ( ) )
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

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->tex_report_init( f, table );

	if ( up == NULL )
	{
		if ( table )
		{
			if ( tab_lines == 0 )
				fprintf( f, "  \\multicolumn{4}{|c|}{(none selected)} \\\\ \n  \\hline \n" );
			fprintf( f, "\\end{longtable}\n\n" );
		}
		else
			if ( tab_lines == 0 )
				fprintf( f, "(none selected) \\newline \n" );
			else
				fprintf( f, "\n" );
	}
}


/*************************************************************
 TEX_REPORT_INITALL
 *************************************************************/
void lsd::object::tex_report_initall( FILE *f, bool table )
{
	int i, j;
	char *ol, *vl;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	if ( ! table )
		return;

	if ( up == NULL )
	{
		fprintf( f, "\\section{Initial values}\n\n" );
		fprintf( f, "\\begin{longtable}{*{3}{|l}|p{9cm}|}\n	 \\hline\n	\\textbf{Object} & \\textbf{Element} & \\textbf{Lag} & \\textbf{Initial values (by instance)} \\\\ \n  \\hline \\endhead\n	\\multicolumn{4}{r}{\\textit{Continued on next page...}} \\\\ \n  \\endfoot\n  \\endlastfoot\n" );
	}

	ol = new char[ 2 * strlen( label ) + 1 ];
	tex_strcpy( ol, label );

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		vl = new char[ 2 * strlen( cv->label ) + 1 ];
		tex_strcpy( vl, cv->label );

		if ( cv->param == 1 )
		{
			fprintf( f, "  \\hrf{%s}{%s} & \\hrf{%s}{%s} \\label{%s_init} & & %g", ol, label, vl, cv->label, vl, cv->val[ 0 ] );
			for ( j = 1, cur = hyper_next( label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
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
				fprintf( f, "  \\hrf{%s}{%s} & \\hrf{%s}{%s} \\label{%s_init} & %d & %g", ol, label, vl, cv->label, vl, i + 1, cv->val[ i ] );
				for ( j = 1, cur = hyper_next( label ); cur != NULL && j < MAX_INIT; cur = cur->hyper_next( cur->label ), ++j )
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

	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->tex_report_initall( f, table );

	if ( up == NULL )
		fprintf( f, "\\end{longtable}\n\n" );
}


/*************************************************************
 TEX_REPORT_END
 *************************************************************/
void lsd::object::tex_report_end( FILE *f )
{
	fprintf( f, "\\end{document}\n" );
}
