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
 INTERF.CPP
 Respond to the events in the main browser interfaces, that is
 the browser window GUI elements and all the menus.

 - object *operate( );
 takes the value of choice and operate the relative command on
 the object r. See the switch for the complete list of the
 available commands
 *************************************************************/

/*
cases used up to 97
cases free 40, 45, 48, 51
*/

#include "LSD.h"

namespace gui
{
	bool initVal = false;			// new variable initial setting going on
	int natBat = true;				// native (Windows/Linux) batch format flag (bool)
	int next_lag;					// new variable initial setting next lag to set
	lsd::object *initParent = NULL;	// parent of new variable initial setting
}


/*************************************************************
 OPERATE
 *************************************************************/
lsd::object *gui::operate( lsd::object *r )
{
	bool observe, initial, saveAs, delVar, renVar, table, subDir, overwDir;
	char deb_mode, *lab0, lab[ MAX_BUFF_SIZE ], lab_old[ 2 * MAX_PATH_LENGTH ], ch[ 2 * MAX_LINE_SIZE ], ch1[ MAX_ELEM_LENGTH ], NOLHfile[ MAX_PATH_LENGTH ], out_file[ MAX_PATH_LENGTH ], out_dir[ MAX_PATH_LENGTH ], term_exe[ MAX_PATH_LENGTH ], out_bat[ MAX_PATH_LENGTH ], win_dir[ MAX_PATH_LENGTH ], buf_descr[ MAX_BUFF_SIZE ];
	const char *lab1, *lab2, *lab3, *lab4;
	design *doe;
	double fracMC, fake = 0;
	int i, j, k, sl, num, param, save, plot, nature, numlag, lag, fSeq, ffirst, fnext, sizMC, varSA, savei, debug, watch, watch_write, parallel, temp[ 11 ], done = 0;
	long nlinks, ptsSa, maxMC;
	lsd::assimilation *ca;
	lsd::bridge *cb;
	lsd::description *cd;
	lsd::object *n, *cur, *cur1, *cur2;
	lsd::result *rf;			// pointer for results files (may be zipped or not)
	lsd::sensitivity *cs;
	lsd::variable *cv, *cv1;
	str_vecT logs;
	FILE *f;

	if ( ! redrawReq )
		redrawRoot = false;		// assume no browser redraw
	else
	{
		redrawRoot = true;		// handle pending async redraw request
		redrawReq = false;
	}

	switch ( choice )
	{
		// exit LSD
		case 11:

			if ( discard_change( ) && abort_run_threads( ) )
				lsd_exit_gui( 0 );

		break;


		// exit the browser and run the simulation
		case 1:

			if ( sim.conf_ok && strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration not saved\" -detail \"Please save your current configuration before trying to run the simulation.\"" );

				choice = 73;
				return r;
			}

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to run the simulation.\"" );
				break;
			}

			// warn about no variable/parameter being saved
			for ( n = r; n->up != NULL; n = n->up );
			sim.series_saved = 0;
			n->count_save( & sim.series_saved );
			if ( sim.series_saved == 0 )
			{
				cmd( "set answer [ ttk::messageBox -parent . -type okcancel -default ok -icon warning -title Warning -message \"No variable or parameter marked to be saved\" -detail \"If you proceed, there will be no data to be analyzed after the simulation is run. If this is not the intended behavior, please mark the variables and parameters to be saved before running the simulation.\" ]; switch -- $answer { ok { set choice 1 } cancel { set choice 2 } } " );
				if ( choice == 2 )
					break;
			}

			// warn missing debugger
			if ( ! sim.parallel_disable && sim.root->search_parallel( ) && ( sim.deb_t > 0 || sim.stack_info > 0 || sim.prof_aggr_time ) )
			{
				cmd( "set answer [ ttk::messageBox -parent . -title Warning -icon warning -type okcancel -default ok -message \"Debugger/profiler not available\" -detail \"Debugging in parallel mode is not supported, including stack profiling.\n\nPress 'OK' to proceed and disable parallel processing settings or 'Cancel' to return to LSD Browser.\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } }" );
				if ( choice == 2 )
					break;

				sim.parallel_disable = true;
			}

			// save the current object & cursor position for quick reload
			r->save_pos( );

			// only ask to overwrite configuration if there are changes
			overwConf = unsaved_change( ) ? true : false;

			// avoid showing dialog if configuration already saved and nothing to save to disk
			if ( ! overwConf && sim.last_run == 1 && ( sim.assim == NULL || sim.assim_disable ) )
				goto run;

			// remove any custom save path (save to current by default)
			sim.results_alt_path( "" );

			Tcl_LinkVar( interp, "no_res", ( char * ) & sim.no_res, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "no_tot", ( char * ) & sim.no_tot, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "add_to_tot", ( char * ) & sim.add_to_tot, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "docsv", ( char * ) & sim.docsv, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "doover", ( char * ) & doover, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "dozip", ( char * ) & sim.dozip, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "overwConf", ( char * ) & overwConf, TCL_LINK_BOOLEAN );

			cmd( "set firstFile \"%s_%d\"", sim.conf_name, sim.seed );
			cmd( "set lastFile \"%s_%d\"", sim.conf_name, sim.seed + sim.last_run - 1 );
			cmd( "set totFile \"%s\"", sim.conf_name );
			cmd( "set resExt %s", sim.docsv ? "csv" : "res" );
			cmd( "set totExt %s", sim.docsv ? "csv" : "tot" );
			cmd( "set zipExt \"%s\"", sim.dozip ? ".gz" : "" );
			cmd( "set tot_msg_warn \"(totals file already exists)\"" );

			cmd( "set T .run" );
			cmd( "newtop $T \"Run Simulation\" { set choice 2 }" );

			cmd( "ttk::frame $T.f1" );
			cmd( "ttk::label $T.f1.l -text \"Model configuration\"" );
			cmd( "ttk::label $T.f1.w -text \"%s\" -style hl.TLabel", sim.conf_name );
			cmd( "pack $T.f1.l $T.f1.w" );

			cmd( "ttk::frame $T.f2" );

			cmd( "ttk::frame $T.f2.t" );
			cmd( "ttk::label $T.f2.t.l -text \"Cases:\"" );
			cmd( "ttk::label $T.f2.t.w -text \"%d\" -style hl.TLabel", sim.last_t );
			cmd( "pack $T.f2.t.l $T.f2.t.w -side left -padx $_2" );

			if ( sim.assim == NULL || sim.assim_disable )	// regular run?
			{
				if ( sim.last_run == 1 )					// single run
				{
					subDir = overwDir = false;

					cmd( "pack $T.f2.t" );

					cmd( "ttk::label $T.f4 -text \"(results will be saved to memory only)\"" );

					cmd( "ttk::checkbutton $T.f6 -text \"Update configuration file\" -variable overwConf -state %s", overwConf ? "normal" : "disabled" );

					cmd( "pack $T.f1 $T.f2 $T.f4 $T.f6 -padx $_5 -pady $_5" );
				}
				else										// MC multi-run
				{
					// detect the need of a new save path and if it has results files
					subDir = need_res_dir( sim.conf_path, sim.conf_name, out_dir, MAX_PATH_LENGTH );
					overwDir = check_res_dir( out_dir );

					cmd( "ttk::frame $T.f2.n" );
					cmd( "ttk::label $T.f2.n.l -text \"Number of simulations:\"" );
					cmd( "ttk::label $T.f2.n.w -text \"%d\" -style hl.TLabel", sim.last_run );
					cmd( "pack $T.f2.n.l $T.f2.n.w -side left -padx $_2" );

					cmd( "pack $T.f2.t $T.f2.n" );

					cmd( "ttk::frame $T.f3" );
					cmd( "ttk::label $T.f3.l -text \"Output path\"" );
					cmd( "ttk::label $T.f3.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", out_dir );
					cmd( "pack $T.f3.l $T.f3.w" );

					cmd( "ttk::frame $T.f4" );
					cmd( "ttk::label $T.f4.l -text \"Results files\"" );

					cmd( "ttk::frame $T.f4.w" );

					cmd( "ttk::frame $T.f4.w.l1" );
					cmd( "ttk::label $T.f4.w.l1.l -text \"from:\"" );
					cmd( "ttk::label $T.f4.w.l1.w -style hl.TLabel -text \"$firstFile.$resExt$zipExt\"" );
					cmd( "pack $T.f4.w.l1.l $T.f4.w.l1.w -side left -padx $_2" );

					cmd( "ttk::frame $T.f4.w.l2" );
					cmd( "ttk::label $T.f4.w.l2.l -text \"to:\"" );
					cmd( "ttk::label $T.f4.w.l2.w -style hl.TLabel -text \"$lastFile.$resExt$zipExt\"" );
					cmd( "pack $T.f4.w.l2.l $T.f4.w.l2.w -side left -padx $_2" );

					cmd( "pack $T.f4.w.l1 $T.f4.w.l2" );

					cmd( "pack $T.f4.l $T.f4.w" );

					cmd( "set choice [ expr { ! $no_tot && [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } ]", out_dir, strlen( out_dir ) > 0 ? "/" : "" );

					cmd( "ttk::frame $T.f5" );
					cmd( "ttk::label $T.f5.l1 -text \"Totals file (last steps)\"" );
					cmd( "ttk::label $T.f5.l2 -style %s -text \"$totFile.$totExt$zipExt\"", choice ? "hl.TLabel" : "dhl.TLabel" );

					if ( choice )
						cmd( "ttk::label $T.f5.l3 -text $tot_msg_warn" );
					else
						cmd( "ttk::label $T.f5.l3 -text \"\"" );

					cmd( "pack $T.f5.l1 $T.f5.l2 $T.f5.l3" );

					sim.add_to_tot = ( choice ) ? sim.add_to_tot : false;

					cmd( "ttk::frame $T.f6" );
					cmd( "ttk::checkbutton $T.f6.a -text \"Append to existing totals file\" -variable add_to_tot -state %s -command { \
							if { $add_to_tot && $doover } { \
								set doover 0 \
							} \
						}", ( choice && ! sim.no_tot ) ? "normal" : "disabled" );
					cmd( "ttk::checkbutton $T.f6.b -text \"Skip generating results files\" -variable no_res" );
					cmd( "ttk::checkbutton $T.f6.b1 -text \"Skip generating totals file\" -variable no_tot -command { \
							if { ! $no_tot } { \
								$T.f5.l2 configure -style hl.TLabel; \
								if { [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
									$T.f5.l3 configure -text $tot_msg_warn; \
									$T.f6.a configure -state normal \
								} else { \
									$T.f5.l3 configure -text \"\"; \
								} \
							} else { \
								$T.f5.l2 configure -style dhl.TLabel; \
								$T.f5.l3 configure -text \"\"; \
								$T.f6.a configure -state disabled \
							} \
						}", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
					cmd( "ttk::checkbutton $T.f6.c -text \"Generate zipped files\" -variable dozip -command { \
						if $dozip { set zipExt \".gz\" } { \
							set zipExt \"\" }; \
							$T.f4.w.l1.w configure -text \"$firstFile.$resExt$zipExt\"; \
							$T.f4.w.l2.w configure -text \"$lastFile.$resExt$zipExt\"; \
							$T.f5.l2 configure -text \"$totFile.$totExt$zipExt\"; \
							if { ! $no_tot && [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
								$T.f5.l3 configure -text $tot_msg_warn; \
								$T.f6.a configure -state normal \
							} else { \
								$T.f5.l3 configure -text \"\"; \
								$T.f6.a configure -state disabled \
							} \
						}", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
					cmd( "ttk::checkbutton $T.f6.d -text \"Comma-separated text format (.csv)\" -variable docsv -command { \
						if $docsv { \
							set resExt csv; set totExt csv \
						} else { \
							set resExt res; \
							set totExt tot }; \
							$T.f4.w.l1.w configure -text \"$firstFile.$resExt$zipExt\"; \
							$T.f4.w.l2.w configure -text \"$lastFile.$resExt$zipExt\"; \
							$T.f5.l2 configure -text \"$totFile.$totExt$zipExt\"; \
							if { ! $no_tot && [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
								$T.f5.l3 configure -text $tot_msg_warn; \
								$T.f6.a configure -state normal \
							} else { \
								$T.f5.l3 configure -text \"\"; \
								$T.f6.a configure -state disabled \
							} \
						}", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
					cmd( "ttk::checkbutton $T.f6.o -text \"Clear output path before run\" -variable doover -state %s -command { \
							if { $add_to_tot && $doover } { \
								set add_to_tot 0 \
							} \
						}", overwDir ? "normal" : "disabled" );
					cmd( "ttk::checkbutton $T.f6.e -text \"Update configuration file\" -variable overwConf -state %s", overwConf ? "normal" : "disabled" );
					cmd( "pack $T.f6.a $T.f6.b $T.f6.b1 $T.f6.c $T.f6.d $T.f6.o $T.f6.e -anchor w" );

					cmd( "pack $T.f1 $T.f2 $T.f3 $T.f4 $T.f5 $T.f6 -padx $_5 -pady $_5" );
				}
			}
			else											// data assimilation
			{
				subDir = false;

				cmd( "pack $T.f2.t" );

				cmd( "ttk::label $T.f4 -text \"(results will be saved to memory only)\n Data Assimilation\"" );

				cmd( "ttk::checkbutton $T.f6 -text \"Update configuration file\" -variable overwConf -state %s", overwConf ? "normal" : "disabled" );

				cmd( "pack $T.f1 $T.f2 $T.f4 $T.f6 -padx $_5 -pady $_5" );
			}

			cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menurun.html#run } { set choice 2 }" );

			cmd( "showtop $T" );
			cmd( "mousewarpto $T.b.ok" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "destroytop .run" );

			Tcl_UnlinkVar( interp, "no_res" );
			Tcl_UnlinkVar( interp, "no_tot" );
			Tcl_UnlinkVar( interp, "add_to_tot" );
			Tcl_UnlinkVar( interp, "docsv" );
			Tcl_UnlinkVar( interp, "doover" );
			Tcl_UnlinkVar( interp, "dozip" );
			Tcl_UnlinkVar( interp, "overwConf" );

			if ( choice == 2 )
				break;

			if ( ( ! sim.no_res || ! sim.no_tot ) && subDir )
				if ( ! create_res_dir( out_dir ) || ! sim.results_alt_path( out_dir ) )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Subdirectory '%s' cannot be created\" -detail \"Check if the path is set READ-ONLY, or move your configuration file to a different location.\"", out_dir );
					break;
				}

			if ( overwDir && doover )
				clean_res_dir( out_dir );

			run:

			for ( n = r; n->up != NULL; n = n->up );
			sim.reset_blueprint( n );			// update blueprint to consider last changes

			if ( overwConf )					// save if needed
			{
				if ( ! save_xml_configuration_gui( ) )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s.lsd' cannot be saved\" -detail \"Check if the file is set READ-ONLY, or try to save to a different location.\"", sim.conf_name );
					break;
				}
				else
					unsaved_change( false );	// signal no unsaved change
			}

			pause_run = false;					// not paused
			done_in = 0;						// no run-time button pressed

			choice = 1;

			return n;


		// add an element to the current or the pointed object (defined in tcl $vname)
		case 2:

			// check if current or pointed object and save current if needed
			lab1 = get_str( "useCurrObj" );
			if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
			{
				lab1 = get_str( "vname" );
				if ( lab1 == NULL || ! strcmp( lab1, "" ) )
					break;
				sscanf( lab1, "%99s", lab_old );

				n = sim.root->search( lab_old );// set pointer to $vname
				if ( n == NULL )
					break;
				cur2 = r;
				r = n;
			}
			else
				cur2 = NULL;

			// read the lists of variables/functions, parameters and objects in model program
			// from disk, if needed, or just update the missing elements lists
			cmd( "if { [ llength $missVar ] == 0 || [ llength $missPar ] == 0 } { read_elem_file %s } { upd_miss_elem }", lsd::model_path );

			Tcl_LinkVar( interp, "done", ( char * ) &done, TCL_LINK_INT );
			Tcl_LinkVar( interp, "num", ( char * ) &num, TCL_LINK_INT );

			param = get_int( "param" );
			cmd( "set num 0" );
			cmd( "set lab \"\"" );
			cmd( "set initValEn 0" );

			cmd( "set T .addelem" );
			cmd( "newtop $T \"Add Element\" { set done 2 }" );

			switch ( param )
			{
				case 0:								// variable
					cmd( "ttk::frame $T.l" );
					cmd( "ttk::label $T.l.l1 -text \"New variable in object:\"" );
					cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
					cmd( "pack $T.l.l1 $T.l.l2 -side left -padx $_2" );

					cmd( "ttk::frame $T.f" );
					cmd( "ttk::label $T.f.lab_ent -text \"Variable name\"" );
					cmd( "ttk::label $T.f.lab_num -text \"Maximum lags\"" );
					cmd( "ttk::label $T.f.sp -width 5" );
					cmd( "ttk::combobox $T.f.ent_var -width 20 -textvariable lab -justify center -values $missVar" );
					cmd( "ttk::spinbox $T.f.ent_num -width 3 -from 0 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set num %%P; if { $num > 0 } { $T.b.x configure -state normal } { $T.b.x configure -state disabled }; return 1 } { %%W delete 0 end; %%W insert 0 $num; return 0 } } -command { if { [ $T.f.ent_num get ] > 0 } { $T.b.x configure -state normal } { $T.b.x configure -state disabled } } -invalidcommand { bell } -justify center" );
					cmd( "write_any $T.f.ent_num $num" );
					cmd( "pack $T.f.lab_ent $T.f.ent_var $T.f.sp $T.f.lab_num $T.f.ent_num -side left -padx $_2" );

					cmd( "tooltip::tooltip $T.f.ent_num \"Maximum lag used in equations\"" );

					cmd( "bind $T.f.ent_var <KeyRelease> { \
							if { %%N < 256 } { \
								set b [ .addelem.f.ent_var index insert ]; \
								set s [ .addelem.f.ent_var get ]; \
								set f [ lsearch -glob $missVar $s* ]; \
								if { $f !=-1 } { \
									set d [ lindex $missVar $f ]; \
									.addelem.f.ent_var delete 0 end; \
									.addelem.f.ent_var insert 0 $d; \
									.addelem.f.ent_var index $b; \
									.addelem.f.ent_var selection range $b end \
								} \
							} \
						}" );
					cmd( "bind $T.f.ent_var <<ComboboxSelected>> { \
							set s [ .addelem.f.ent_var get ]; \
							.addelem.d.f.text insert end \"[ get_var_descr $s ]\"; \
						}" );
					cmd( "bind $T.f.ent_var <KeyPress-Return> { event generate .addelem.f.ent_var <<ComboboxSelected>>; if { [ .addelem.f.ent_num get ] > 0 } { focus $T.b.x } { focus $T.b.ok } }" );
					cmd( "bind $T.f.ent_num <KeyPress-Return> { if { [ .addelem.f.ent_num get ] > 0 } { focus $T.b.x } { focus $T.b.ok } }" );
					cmd( "set help menumodel.html#AddAVar");
					break;

				case 2:								// function
					cmd( "ttk::frame $T.l" );
					cmd( "ttk::label $T.l.l1 -text \"New function in object:\"" );
					cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
					cmd( "pack $T.l.l1 $T.l.l2 -side left -padx $_2" );

					cmd( "ttk::frame $T.f" );
					cmd( "ttk::label $T.f.lab_ent -text \"Function name\"" );
					cmd( "ttk::combobox $T.f.ent_var -width 20 -textvariable lab -justify center -values $missVar" );
					cmd( "pack $T.f.lab_ent $T.f.ent_var -side left -padx $_2" );
					cmd( "bind $T.f.ent_var <KeyRelease> { \
							if { %%N < 256 } { \
								set b [ .addelem.f.ent_var index insert ]; \
								set s [ .addelem.f.ent_var get ]; \
								set f [ lsearch -glob $missVar $s* ]; \
								if { $f !=-1 } { \
									set d [ lindex $missVar $f ]; \
									.addelem.f.ent_var delete 0 end; \
									.addelem.f.ent_var insert 0 $d; \
									.addelem.f.ent_var index $b; \
									.addelem.f.ent_var selection range $b end \
								} \
							} \
						}" );
					cmd( "bind $T.f.ent_var <<ComboboxSelected>> { \
							set s [ .addelem.f.ent_var get ]; \
							.addelem.d.f.text insert end \"[ get_var_descr $s ]\"; \
						}" );
					cmd( "set help menumodel.html");
					cmd( "bind $T.f.ent_var <KeyPress-Return> { event generate .addelem.f.ent_var <<ComboboxSelected>>; focus $T.b.ok }" );

					break;

				case 1:								// parameter
					cmd( "ttk::frame $T.l" );
					cmd( "ttk::label $T.l.l1 -text \"New parameter in object:\"" );
					cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
					cmd( "pack $T.l.l1 $T.l.l2 -side left -padx $_2" );

					cmd( "ttk::frame $T.f" );
					cmd( "ttk::label $T.f.lab_ent -text \"Parameter name\"" );
					cmd( "ttk::combobox $T.f.ent_var -width 20 -textvariable lab -justify center -values $missPar" );
					cmd( "pack $T.f.lab_ent $T.f.ent_var -side left -padx $_2" );
					cmd( "bind $T.f.ent_var <KeyRelease> { \
							if { %%N < 256 } { \
								set b [ .addelem.f.ent_var index insert ]; \
								set s [ .addelem.f.ent_var get ]; \
								set f [ lsearch -glob $missPar $s* ]; \
								if { $f !=-1 } { \
									set d [ lindex $missPar $f ]; \
									.addelem.f.ent_var delete 0 end; \
									.addelem.f.ent_var insert 0 $d; \
									.addelem.f.ent_var index $b; \
									.addelem.f.ent_var selection range $b end \
								} \
							} \
						}" );
					cmd( "bind $T.f.ent_var <KeyPress-Return> { focus $T.b.x }" );
					cmd( "set help menumodel.html#AddAPar");
					cmd( "set initValEn 1" );
					break;

				default:
					done = 2;
					goto err_newelem;
			}

			cmd( "set w $T.d" );
			cmd( "ttk::frame $w" );
			cmd( "ttk::label $w.lab -text \"Description\"" );
			cmd( "ttk::frame $w.f" );
			cmd( "ttk::scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
			cmd( "ttk::text $w.f.text -wrap word -width 60 -height 6 -yscrollcommand \"$w.f.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
			cmd( "pack $w.f.yscroll -side right -fill y" );
			cmd( "pack $w.f.text -expand yes -fill both" );
			cmd( "mouse_wheel $w.f.text" );
			cmd( "pack $w.lab $w.f" );

			cmd( "pack $T.l $T.f $T.d -pady $_5" );

			cmd( "okXhelpcancel $T b \"Initial Values\" { set done 3 } { set done 1 } { LsdHelp $help } { set done 2 }" );

			cmd( "tooltip::tooltip $T.b.x \"Save and set initial value for element\"" );

			cmd( "if { ! $initValEn } { $T.b.x configure -state disabled }" );

			cmd( "showtop $T topleftW" );
			cmd( "focus $T.f.ent_var; $T.f.ent_var selection range 0 end" );

			here_newelem:

			if ( param == 0 )
				cmd( "write_any .addelem.f.ent_num $num" );

			while ( done == 0 )
				Tcl_DoOneEvent( 0 );

			if ( param == 0 )
				cmd( "set num [ .addelem.f.ent_num get ]" );

			initVal = ( done == 3 ) ? true : false;

			if ( done == 1 || done == 3 )
			{
				get_str( "lab", lab, MAX_ELEM_LENGTH );
				sl = strlen( lab );
				if ( sl != 0 )
				{
					for ( cur = r; cur->up != NULL; cur = cur->up );

					done = cur->check_label( lab );
					if ( done == 1 )
					{
						cmd( "ttk::messageBox -parent .addelem -title Error -icon error -type ok -message \"The name already exists in the model\" -detail \"Choose a different name and try again.\"" );
						cmd( "focus .addelem.f.ent_var; .addelem.f.ent_var selection range 0 end" );
						done = 0;
						goto here_newelem;
					}

					if ( done == 2 )
					{
						cmd( "ttk::messageBox -parent .addelem -title Error -icon error -type ok -message \"Invalid characters in name\" -detail \"Names must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces or other characters. Choose a different name and try again.\"" );
						cmd( "focus .addelem.f.ent_var; .addelem.f.ent_var selection range 0 end" );
						done = 0;
						goto here_newelem;
					}

					if ( done == 0 )
					{
						sim.add_description( lab, param, eval_str( "[ .addelem.d.f.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

						if ( param == 0 )
							cmd( "lappend modVar %s", lab );

						if ( param == 1 )
							cmd( "lappend modPar %s", lab );

						if ( param == 2 )
							cmd( "lappend modFun %s", lab );

						cmd( "lappend modElem %s", lab );

						for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
						{
							cv = cur->add_empty_var( lab );
							if ( param != 0 )
								num = 0;
							cv->val = new double[ num + 1 ];
							cv->save = 0;
							cv->param = param;
							cv->num_lag = num;
							cv->deb_mode = 'n';
							if ( ( param == 0 && num == 0 ) || param == 2 )
								cv->initialized = true;
							else
								cv->initialized = false;

							for ( i = 0; i < num + 1; ++i )
								cv->val[ i ] = 0;
						}

						initParent = r;

						// update focus memory
						cmd( "set listfocus 1; set itemfocus [ .l.v.c.var_name index end ]" );
						sim.conf_ok = true;		// some model structure loaded
						unsaved_change( true );		// signal unsaved change
						redrawRoot = redrawStruc = true;// force browser/structure redraw
					}
				}
				else
				{
					initVal = false;
					initParent = NULL;
				}
			}

			err_newelem:

			cmd( "destroytop .addelem" );

			if ( cur2 != NULL )						// restore original current object
				r = cur2;

			Tcl_UnlinkVar( interp, "done" );
			Tcl_UnlinkVar( interp, "num" );
			cmd( "unset done" );

			if ( initVal )
			{
				if ( param == 0 && num < 1 )
				{
					cmd( "ttk::messageBox -parent . -type ok -title Warning -icon warning -message \"Cannot set initial value\" -detail \"The variable '%s' was created with maximum lag equal to zero. No initial value is required.\"", lab );
					initVal = false;
					break;
				}

				cmd( "set vname %s", lab );
				next_lag = 0;						// lag to initialize
				choice = 77;						// change initial values for $vname
				return r;							// execute command
			}

		break;


		// Add a Descendent object to the current or the pointed object (defined in tcl $vname)
		// and assigns the number of its instances.
		case 3:

			// check if current or pointed object and save current if needed
			lab1 = get_str( "useCurrObj" );
			if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
			{
				lab1 = get_str( "vname" );
				if ( lab1 == NULL || ! strcmp( lab1, "" ) )
					break;
				sscanf( lab1, "%99s", lab_old );

				n = sim.root->search( lab_old );// set pointer to $vname
				if ( n == NULL )
					break;
				cur2 = r;
				r = n;
			}
			else
				cur2 = NULL;

			// read the lists of variables/functions, parameters and objects in model program
			// from disk, if needed, or just update the missing elements lists
			cmd( "if { [ llength $missObj ] == 0 } { read_elem_file %s } { upd_miss_elem }", lsd::model_path );

			Tcl_LinkVar( interp, "done", ( char * ) &done, TCL_LINK_INT );

			cmd( "set lab \"\"" );

			cmd( "set T .addobj" );
			cmd( "newtop $T \"Add Object\" { set done 2 }" );

			cmd( "ttk::frame $T.l" );
			cmd( "ttk::label $T.l.l1 -text \"New object descending from:\"" );
			cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
			cmd( "pack $T.l.l1 $T.l.l2 -side left -padx $_2" );

			cmd( "ttk::frame $T.f" );
			cmd( "ttk::label $T.f.lab_ent -text \"Object name\"" );
			cmd( "ttk::combobox $T.f.ent_var -width 20 -textvariable lab -justify center -values $missObj" );
			cmd( "pack $T.f.lab_ent $T.f.ent_var -side left -padx $_2" );
			cmd( "bind $T.f.ent_var <KeyPress-Return> {focus $T.b.ok}" );

			cmd( "set w $T.d" );
			cmd( "ttk::frame $w" );
			cmd( "ttk::label $w.lab -text \"Description\"" );
			cmd( "ttk::frame $w.f" );
			cmd( "ttk::scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
			cmd( "ttk::text $w.f.text -wrap word -width 60 -height 6 -yscrollcommand \"$w.f.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
			cmd( "pack $w.f.yscroll -side right -fill y" );
			cmd( "pack $w.f.text -expand yes -fill both" );
			cmd( "mouse_wheel $w.f.text" );
			cmd( "pack $w.lab $w.f" );

			cmd( "pack $T.l $T.f $w -pady $_5" );
			cmd( "okhelpcancel $T b { set done 1 } { LsdHelp menumodel.html#AddADesc } { set done 2 }" );

			cmd( "showtop $T topleftW" );
			cmd( "focus $T.f.ent_var; $T.f.ent_var selection range 0 end" );

			here_newobject:

			while ( done == 0 )
				Tcl_DoOneEvent( 0 );

			if ( done == 1 )
			{
				get_str( "lab", lab, MAX_ELEM_LENGTH );
				if ( strlen( lab ) == 0 )
					goto here_endobject;

				for ( cur = r; cur->up != NULL; cur = cur->up );

				done = cur->check_label( lab ); // check that the label does not exist already
				if ( done == 1 )
				{
					cmd( "ttk::messageBox -parent .addobj -title Error -icon error -type ok -message \"Name already exists in the model\" -detail \"Choose a different name and try again.\"" );
					cmd( "focus .addobj.f.ent_var; .addobj.f.ent_var selection range 0 end" );
					done = 0;
					goto here_newobject;
				}

				if ( done == 2 )
				{
					cmd( "ttk::messageBox -parent .addobj -title Error -icon error -type ok -message \"Invalid characters in name\" -detail \"Names must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces or other characters. Choose a different name and try again.\"" );
					cmd( "focus .addobj.f.ent_var; .addobj.f.ent_var selection range 0 end" );
					done = 0;
					goto here_newobject;
				}

				r->add_obj( lab, 1, true );

				sim.add_description( lab, 4, eval_str( "[ .addobj.d.f.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );
				cmd( "lappend modObj %s", lab );

				// update focus memory
				cmd( "set listfocus 2; set itemfocus [ .l.s.c.son_name index end ]; set itemfirst [ lindex [ .l.s.c.son_name yview ] 0 ]" );
				sim.conf_ok = true;		// some model structure loaded
				unsaved_change( true );		// signal unsaved change
				redrawRoot = redrawStruc = true;// force browser/structure redraw
			}

			here_endobject:

			cmd( "destroytop .addobj" );

			if ( cur2 != NULL )				// restore original current object
				r = cur2;

			Tcl_UnlinkVar( interp, "done" );
			cmd( "unset done" );

		break;


		// move object (defined in tcl $vname)
		case 32:

			// check if current or pointed object and save current if needed
			lab1 = get_str( "useCurrObj" );
			if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
			{
				lab1 = get_str( "vname" );
				if ( lab1 == NULL || ! strcmp( lab1, "" ) )
					break;
				sscanf( lab1, "%99s", lab_old );

				n = sim.root->search( lab_old );// set pointer to $vname
				if ( n == NULL )
					break;
				cur2 = r;
				r = n;
			}
			else
				cur2 = NULL;

			if ( r->up == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Cannot move 'Root' object\" -detail \"Consider, if appropriate, moving its descendants, one at a time.\"" );
				goto endmove;
			}

			cmd( "set TT .objs" );
			cmd( "newtop $TT \"Move\" { set choice 2 }" );

			cmd( "ttk::frame $TT.l" );
			cmd( "ttk::label $TT.l.l -text \"Object:\"" );
			cmd( "ttk::label $TT.l.n -style hl.TLabel -text \"%s\"", lab_old );
			cmd( "pack $TT.l.l $TT.l.n -side left -padx $_2" );

			cmd( "ttk::frame $TT.v" );
			cmd( "ttk::label $TT.v.l -text \"Move to\"" );

			cmd( "ttk::frame $TT.v.t" );
			cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
			cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
			cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
			cmd( "mouse_wheel $TT.v.t.lb" );
			sim.root->insert_object( "$TT.v.t.lb", false, r );
			cmd( "pack $TT.v.l $TT.v.t" );

			cmd( "pack $TT.l $TT.v -padx $_5 -pady $_5" );

			cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );// insert ok button

			cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0; break }" );
			cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end; break }" );
			cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

			cmd( "showtop $TT" );
			cmd( "$TT.v.t.lb selection set 0" );
			cmd( "focus $TT.v.t.lb" );

			choice = 0;

			cmd( "if { [ $TT.v.t.lb size ] == 0 } { ttk::messageBox -parent . -type ok -title Error -icon error -message \"Cannot move single 'Root' descendant\" -detail \"Consider, if appropriate, creating additional objects under 'Root' before moving this one.\"; set choice 2 }" );

			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "if { $choice != 2 } { set movelabel [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ] }" );
			cmd( "destroytop .objs" );

			if ( choice == 2 )
				goto endmove;

			lab1 = get_str( "movelabel" );
			if ( lab1 == NULL || strlen( lab1 ) == 0 )
				goto endmove;

			i = sim.hyper_count( r->up->label );
			j = sim.hyper_count( lab1 );

			if ( i != j )
			{
				cmd( "if { %d < %d } { set msg \"the last instance of '$vname' being replicated %d times\" } { set msg \"the last %d unmatched instances of '$vname' being deleted\" }", i, j, j - i, i - j );
				cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -title Warning -icon warning -message \"Different number of parents' instances\" -detail \"The original parent object '%s' has a different number of instances (%d) than the desired new parent '%s' (%d). Copying object '$vname' to parent '%s' will result in $msg.\" ]", r->up->label, i, lab1, j, lab1 );
				cmd( "switch $answer { yes { set choice 1 } no { set choice 2 } }" );

				if( choice == 2 )
					goto endmove;
			}

			sim.move_obj( lab_old, lab1 );

			unsaved_change( true );		// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw

			endmove:

			if ( cur2 != NULL )					// restore original current object
				r = cur2;

		break;


		// Move browser to show one of the descendant object (defined in tcl $vname)
		case 4:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) || ! strcmp( lab1, "(none)" ) )
				break;

			sscanf( lab1, "%99s", lab_old );

			n = sim.root->search( lab_old );
			if ( n == NULL )
				break;

			cmd( "set listfocus 2; set itemfocus 0" );

			choice = 0;
			redrawRoot = redrawStruc = true;	// force browser/structure redraw
			return n;


		// Move browser to show the parent object
		case 5:

			if ( r->up == NULL )
				return r;

			for ( i = 0, cb = r->up->b; cb->head != r; cb = cb->next, ++i );

			cmd( "set listfocus 2; set itemfocus %d", r->up->up == NULL ? i : i + 1 );

			choice = 0;
			redrawRoot = redrawStruc = true;	// force browser/structure redraw
			return r->up;


		// Edit current Object and give the option to disable the computation (defined in tcl $vname)
		case 6:

			cmd( "if $useCurrObj { set lab %s } { if [ info exists vname ] { set lab $vname } { set lab \"\" } }; set useCurrObj yes ", r->label  );
			lab1 = get_str( "lab" );

			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			// check if current or pointed object and save current if needed
			if ( strcmp( r->label, lab_old ) )	// check if not current variable
			{
				n = sim.root->search( lab_old );// set pointer to $vname
				if ( n == NULL )
					break;
				cur2 = r;
				r = n;
			}
			else
				cur2 = NULL;

			if ( ! strcmp( r->label, "Root" ) )	// cannot change Root
			{
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Cannot change Root\" -detail \"Please select an existing object or insert a new one before using this option.\"" );
				break;
			}

			cd = sim.search_description( lab_old );
			r->next_count( r, & num );

			cmd( "set to_compute %d", r->to_compute ? 1 : 0 );

			cmd( "set T .objprop" );
			cmd( "newtop $T \"Change Object\" { set choice 2 }" );

			cmd( "ttk::frame $T.h" );

			cmd( "ttk::frame $T.h.o" );
			cmd( "ttk::label $T.h.o.lab -text \"Object:\"" );
			cmd( "ttk::label $T.h.o.ent -style hl.TLabel -text $lab" );
			cmd( "pack $T.h.o.lab $T.h.o.ent -side left -padx $_2" );

			cmd( "ttk::frame $T.h.i" );
			cmd( "ttk::label $T.h.i.lab -text \"Number of instances:\"" );
			cmd( "ttk::label $T.h.i.ent -style hl.TLabel -text %d", num );
			cmd( "pack $T.h.i.lab $T.h.i.ent -side left -padx $_2" );

			cmd( "pack $T.h.o $T.h.i" );

			cmd( "ttk::frame $T.b0" );
			cmd( "ttk::button $T.b0.prop -width $butWid -text Rename -command { set useCurrObj yes; set choice 83 } -underline 0" );
			cmd( "ttk::button $T.b0.num -width $butWid -text Number -command { set useCurrObj yes; set choice 33 } -underline 0" );
			cmd( "ttk::button $T.b0.mov -width $butWid -text Move -command { set useCurrObj yes; set choice 32 } -underline 0" );
			cmd( "ttk::button $T.b0.del -width $butWid -text Delete -command { set choice 74 } -underline 0" );
			cmd( "pack $T.b0.prop $T.b0.num $T.b0.mov $T.b0.del -padx $butSpc -side left" );

			cmd( "tooltip::tooltip $T.b0.prop \"Change name\"" );
			cmd( "tooltip::tooltip $T.b0.num \"Change number of instances (copies)\"" );
			cmd( "tooltip::tooltip $T.b0.mov \"Move to another parent object\"" );
			cmd( "tooltip::tooltip $T.b0.del \"Remove object\"" );

			cmd( "ttk::frame $T.b1" );
			cmd( "ttk::checkbutton $T.b1.com -text \"Compute: force the computation of the variables in this object\" -variable to_compute -underline 1" );
			cmd( "pack $T.b1.com" );

			cmd( "set w $T.desc" );

			cmd( "ttk::frame $w" );
			cmd( "ttk::label $w.int -text Description -anchor center" );
			cmd( "ttk::frame $w.f" );
			cmd( "ttk::scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
			cmd( "ttk::text $w.f.text -wrap word -width 60 -height 10 -yscrollcommand \"$w.f.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
			cmd( "pack $w.f.yscroll -side right -fill y" );
			cmd( "pack $w.f.text -anchor w -expand yes -fill both" );
			cmd( "mouse_wheel $w.f.text" );

			cmd( "pack $w.int $w.f -fill x -expand yes" );

			cmd( "pack $T.h $T.b0 $T.b1 $w -pady $_5" );

			cmd( "bind $T <Control-r> \"$T.b0.prop invoke\"; bind $T <Control-R> \"$T.b0.prop invoke\"" );
			cmd( "bind $T <Control-n> \"$T.b0.num invoke\"; bind $T <Control-N> \"$T.b0.num invoke\"" );
			cmd( "bind $T <Control-m> \"$T.b0.mov invoke\"; bind $T <Control-M> \"$T.b0.mov invoke\"" );
			cmd( "bind $T <Control-d> \"$T.b0.del invoke\"; bind $T <Control-D> \"$T.b0.del invoke\"" );
			cmd( "bind $T <Control-o> \"$T.b1.com invoke\"; bind $T <Control-O> \"$T.b1.com invoke\"" );

			cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menumodel.html#ChangeObjName } { set choice 2 }" );

			cmd( "showtop $T topleftW" );
			cmd( "mousewarpto $T.b.ok" );

			cmd( "$w.f.text insert end \"%s\"", strtcl( buf_descr, cd->text, MAX_BUFF_SIZE ) );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			done = choice;

			if ( choice != 2 )
			{
				unsaved_change( true );		// signal unsaved change

				// save description changes
				sim.change_description( lab_old, NULL, -1, eval_str( "[ .objprop.desc.f.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

				cmd( "set choice $to_compute" );

				if ( choice != r->to_compute )
				{
					cur = sim.blueprint->search( r->label );
					if ( cur != NULL )
						cur->to_compute = choice;
					for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
						cur->to_compute = choice;
				}

				// control for elements to save in objects to be not computed
				check_save = true;
				if ( choice == 0 )
					r->control_to_compute( );
			}

			cmd( "destroytop .objprop" );

			redrawRoot = redrawStruc = true;			// force browser/structure redraw

			// dispatch chosen option
			if ( done > 2 )
			{
				cmd( "set vname $lab" );
				cmd( "set useCurrObj no" );
				choice = done;
			}
			else
				choice = 0;

			// avoid entering into descendant
			if ( cur2 != NULL )
				return cur2;
			else
				return r;

		break;


		// Delete object (defined in tcl $vname)
		case 74:
		// Rename object (defined in tcl $vname)
		case 83:

			nature = choice;

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			cur = sim.root->search( lab_old );// get pointer to vname
			if ( cur == NULL )
				break;

			if ( nature == 74 )		// delete
			{
				cmd( "set answer [ ttk::messageBox -parent . -title Confirmation -icon question -type yesno -default yes -message \"Delete object?\" -detail \"Press 'Yes' to confirm deleting '$vname'\n\nNote that all descendants will be also deleted!\" ]" );
				cmd( "switch $answer { yes { set choice 1 } no { set choice 2 } }" );
				if ( choice == 2 )
					break;

				r = cur->up;
				cur->wipe_out( );
			}
			else					// rename
			{
				cmd( "set T .chgnam" );
				cmd( "newtop $T \"Rename\" { set choice 2 }" );

				cmd( "ttk::frame $T.l" );
				cmd( "ttk::label $T.l.l -text \"Object:\"" );
				cmd( "ttk::label $T.l.n -style hl.TLabel -text \"$vname\"" );
				cmd( "pack $T.l.l $T.l.n -side left -padx $_2" );

				cmd( "ttk::frame $T.e" );
				cmd( "ttk::label $T.e.l -text \"New name\"" );
				cmd( "ttk::entry $T.e.e -width 20 -textvariable vname -justify center" );
				cmd( "pack $T.e.l $T.e.e -side left -padx $_2" );

				cmd( "pack $T.l $T.e -padx $_5 -pady $_5" );

				cmd( "okcancel $T b { set choice 1 } { set choice 2 }" );

				cmd( "bind $T.e.e <KeyPress-Return> { set choice 1 }" );

				cmd( "showtop $T" );
				cmd( "focus $T.e.e" );
				cmd( "$T.e.e selection range 0 end" );

				here_newname:

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				if ( choice == 1 )
				{
					lab1 = get_str( "vname" );
					if ( lab1 == NULL || ! strcmp( lab1, "" ) )
						break;
					sscanf( lab1, "%99s", lab );

					if ( strcmp( lab, r->label ) )
					{
						for ( cur1 = r; cur1->up != NULL; cur1 = cur1->up );

						done = cur1->check_label( lab );
						if ( done == 1 )
						{
							cmd( "ttk::messageBox -parent .chgnam -title Error -icon error -type ok -message \"The name already exists in the model\" -detail \"Choose a different name and try again.\"" );
							cmd( "focus .chgnam.e.e; .chgnam.e.e selection range 0 end" );
							goto here_newname;
						}

						if ( done == 2 )
						{
							cmd( "ttk::messageBox -parent .chgnam -title Error -icon error -type ok -message \"Invalid characters in name\" -detail \"Names must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
							cmd( "focus .chgnam.e.e; .chgnam.e.e selection range 0 end" );
							goto here_newname;
						}

						// update element list
						cmd( "if [ info exists modObj ] { set pos [ lsearch -exact $modObj %s ]; if { $pos >= 0 } { set modObj [ lreplace $modObj $pos $pos ] } }", cur->label	);
						cmd( "lappend modObj %s", lab );

						sim.change_description( cur->label, lab );
						cur->chg_lab( lab );
					}
					else
						break;
				}

				cmd( "destroytop .chgnam" );
			}

			if ( sim.root->v == NULL && sim.root->b == NULL )// if last object
			{
				unsaved_change( false );				// no unsaved change
				sim.conf_ok = false;					// no config loaded
			}
			else
				unsaved_change( true );					// signal unsaved change

			redrawRoot = redrawStruc = true;			// force browser/structure redraw

		break;


		// Edit variable (defined in tcl $vname) and set debug/saving/plot flags
		case 7:

			redrawRoot = redrawStruc = true;	// force browser/structure redraw

			cmd( "if { ! [ catch { set vname [ .l.v.c.var_name get [ .l.v.c.var_name curselection ] ] } ] && ! [ string equal $vname \"\" ] } { set choice 1 } { set choice 0 }" );
			if ( choice == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No element selected\" -detail \"Please select an element (variable, parameter) before using this option.\"" );
				break;
			}

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) || ! strcmp( lab1, "(none)" ) )
				break;

			sscanf( lab1, "%99s", lab_old );
			cv = r->search_var( NULL, lab_old );
			cd = sim.search_description( lab_old );

			Tcl_LinkVar( interp, "done", ( char * ) &done, TCL_LINK_INT );
			Tcl_LinkVar( interp, "save", ( char * ) &save, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "savei", ( char * ) &savei, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "plot", ( char * ) &plot, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "debug", ( char * ) &debug, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "watch", ( char * ) &watch, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "watch_write", ( char * ) &watch_write, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "parallel", ( char * ) &parallel, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "nature", ( char * ) &nature, TCL_LINK_BOOLEAN );

			save = cv->save;
			savei = cv->savei;
			plot = cv->plot;
			nature = cv->integer;
			debug = ( cv->deb_mode == 'd' || cv->deb_mode == 'W' || cv->deb_mode == 'R' ) ? 1 : 0;
			watch = ( cv->deb_mode == 'w' || cv->deb_mode == 'W' || cv->deb_mode == 'r' || cv->deb_mode == 'R' ) ? 1 : 0;
			watch_write = ( cv->deb_mode == 'r' || cv->deb_mode == 'R' ) ? 1 : 0;
			parallel = cv->parallel;

			cmd( "set observe %d", cd->observe ? 1 : 0 );
			cmd( "set initial %d", cd->initial ? 1 : 0 );
			cmd( "set vname %s", lab_old );

			if ( std::isnan( cv->max_val ) )
				cmd( "set vmax \"%s\"", NON_AVAILABLE );
			else
				cmd( "set vmax %g", cv->max_val );

			if ( std::isnan( cv->min_val ) )
				cmd( "set vmin \"%s\"", NON_AVAILABLE );
			else
				cmd( "set vmin %g", cv->min_val );

			cmd( "set T .chgelem" );
			cmd( "newtop $T \"Change Element\" { set done 2 }" );

			cmd( "ttk::frame $T.h" );

			cmd( "ttk::frame $T.h.o" );

			cmd( "ttk::frame $T.h.o.l" );

			if ( cv->param == 0 )
				cmd( "ttk::label $T.h.o.l.lab_ent -text \"Variable:\"" );
			if ( cv->param == 1 )
				cmd( "ttk::label $T.h.o.l.lab_ent -text \"Parameter:\"" );
			if ( cv->param == 2 )
				cmd( "ttk::label $T.h.o.l.lab_ent -text \"Function:\"" );

			cmd( "ttk::label $T.h.o.l.ent_var -style hl.TLabel -text $vname" );
			cmd( "pack $T.h.o.l.lab_ent $T.h.o.l.ent_var -side left -padx $_2" );

			cmd( "ttk::frame $T.h.o.o" );
			cmd( "ttk::label $T.h.o.o.l -text \"In object:\"" );
			cmd( "ttk::label $T.h.o.o.obj -style hl.TLabel -text \"%s\"", cv->up->label );
			cmd( "pack $T.h.o.o.l $T.h.o.o.obj -side left -padx $_2" );

			cmd( "pack $T.h.o.l $T.h.o.o -side left -padx $_5" );
			cmd( "pack $T.h.o" );

			if ( cv->num_lag > 0 || cv->param == 1 )
			{
				cmd( "ttk::frame $T.h.i" );
				cmd( "ttk::label $T.h.i.l -text \"Initial value%s%s:\"", cv->num_lag > 1 ? "s" : "", cv->up->next == NULL ? "" : " (first instance)" );

				if ( cv->initialized )
				{
					strcpy ( buf_descr, "" );

					j = ( cv->param == 1 ) ? 1 : std::min( cv->num_lag, 4 );
					for ( i = 0; i < j; ++i )
					{
						cmd( "ttk::frame $T.h.i.v%d", i );
						cmd( "ttk::label $T.h.i.v%d.val -style hl.TLabel -text \"%g\"", i, cv->val[ i ] );

						if ( j > 1 )
						{
							cmd( "ttk::label $T.h.i.v%d.lag -text \"(%d)\"", i, i + 1 );
							cmd( "pack $T.h.i.v%d.val $T.h.i.v%d.lag -side left", i, i );
						}
						else
							cmd( "pack $T.h.i.v%d.val", i );

						snprintf( lab, MAX_ELEM_LENGTH, " $T.h.i.v%d", i );
						lsd::strcatn( buf_descr, lab, MAX_BUFF_SIZE );
					}

					cmd( "pack $T.h.i.l %s -side left -padx $_1", buf_descr );
				}
				else
				{
					cmd( "ttk::label $T.h.i.val -style hl.TLabel -text \"(uninitialized)\"" );
					cmd( "pack $T.h.i.l $T.h.i.val -side left -padx $_2" );
				}

				cmd( "pack $T.h.i" );
			}

			if ( cv->param == 0 && ( cv->delay > 0 || cv->delay_range > 0 || cv->period > 1 || cv->period_range > 1 ) )
			{
				cmd( "ttk::frame $T.h.u" );

				if ( cv->delay > 0 )
				{
					cmd( "ttk::frame $T.h.u.d" );
					cmd( "ttk::label $T.h.u.d.l -text \"Initial updating delay:\"" );
					cmd( "ttk::label $T.h.u.d.v -style hl.TLabel -text \"%d\"", cv->delay );
					cmd( "pack $T.h.u.d.l $T.h.u.d.v -side left -padx $_2" );
					cmd( "pack $T.h.u.d" );
				}

				if ( cv->delay_range > 0 )
				{
					cmd( "ttk::frame $T.h.u.dr" );
					cmd( "ttk::label $T.h.u.dr.l -text \"Random updating delay range:\"" );
					cmd( "ttk::label $T.h.u.dr.v -style hl.TLabel -text \"%d\"", cv->delay_range );
					cmd( "pack $T.h.u.dr.l $T.h.u.dr.v -side left -padx $_2" );
					cmd( "pack $T.h.u.dr" );
				}

				if ( cv->period > 1 )
				{
					cmd( "ttk::frame $T.h.u.p" );
					cmd( "ttk::label $T.h.u.p.l -text \"Updating period:\"" );
					cmd( "ttk::label $T.h.u.p.v -style hl.TLabel -text \"%d\"", cv->period );
					cmd( "pack $T.h.u.p.l $T.h.u.p.v -side left -padx $_2" );
					cmd( "pack $T.h.u.p" );
				}

				if ( cv->period_range > 1 )
				{
					cmd( "ttk::frame $T.h.u.pr" );
					cmd( "ttk::label $T.h.u.pr.l -text \"Random updating period range:\"" );
					cmd( "ttk::label $T.h.u.pr.v -style hl.TLabel -text \"%d\"", cv->period_range );
					cmd( "pack $T.h.u.pr.l $T.h.u.pr.v -side left -padx $_2" );
					cmd( "pack $T.h.u.pr" );
				}

				cmd( "pack $T.h.u" );
			}

			cmd( "ttk::frame $T.b0" );
			cmd( "ttk::button $T.b0.prop -width $butWid -text Properties -command { set done 5 } -underline 1" );
			cmd( "ttk::button $T.b0.upd -width $butWid -text Updating -command { set done 14 } -underline 7" );
			cmd( "ttk::button $T.b0.mov -width $butWid -text Move -command { set done 13 } -underline 0" );
			cmd( "ttk::button $T.b0.del -width $butWid -text Delete -command { set done 10 } -underline 2" );

			if ( cv->param == 0 )
			{
				cmd( "bind $T <Control-g> \"$T.b0.upd invoke\"; bind $T <Control-G> \"$T.b0.upd invoke\"" );

				cmd( "pack $T.b0.prop $T.b0.upd $T.b0.mov $T.b0.del -padx $butSpc -side left -pady $_5" );
				cmd( "tooltip::tooltip $T.b0.upd \"Define special update timing\"" );
			}
			else
				cmd( "pack $T.b0.prop $T.b0.mov $T.b0.del -padx $butSpc -side left -pady $_5" );

			cmd( "tooltip::tooltip $T.b0.prop \"Change name, type or lags\"" );
			cmd( "tooltip::tooltip $T.b0.mov \"Move to another object\"" );
			cmd( "tooltip::tooltip $T.b0.del \"Remove element\"" );

			cmd( "ttk::frame $T.b1" );

			cmd( "ttk::frame $T.b1.l1" );
			cmd( "ttk::checkbutton $T.b1.l1.n -text Save -variable save -width 15 -underline 0 -command { \
					if { $save } { \
						.chgelem.b1.l1.i configure -state normal \
					} else { \
						set savei 0; \
						.chgelem.b1.l1.i configure -state disabled \
					} \
				}" );
			cmd( "ttk::checkbutton $T.b1.l1.i -text \"Save to file\" -variable savei -width 15 -underline 8" );
			cmd( "if { ! $save } { \
					set savei 0; \
					.chgelem.b1.l1.i configure -state disabled \
				}" );
			cmd( "ttk::checkbutton $T.b1.l1.plt -text \"Run-time plot\" -variable plot -width 15 -underline 9" );
			cmd( "pack $T.b1.l1.n $T.b1.l1.i $T.b1.l1.plt -side left -anchor w -padx $_10" );

			cmd( "ttk::frame $T.b1.l2" );
			cmd( "ttk::checkbutton $T.b1.l2.deb -text Debug -variable debug -width 15 -underline 0" );
			cmd( "ttk::checkbutton $T.b1.l2.w -text Watch -variable watch -width 15 -underline 0 -command { \
					if { $watch } { \
						.chgelem.b1.l2.ww configure -state normal \
					} else { \
						set watch_write 0; \
						.chgelem.b1.l2.ww configure -state disabled \
					} \
				}" );
			cmd( "ttk::checkbutton $T.b1.l2.ww -text \"Watch only writes\" -variable watch_write -width 15 -underline 4" );
			cmd( "if { ! $watch } { \
					set watch_write 0; \
					.chgelem.b1.l2.ww configure -state disabled \
				}" );
			cmd( "pack $T.b1.l2.deb $T.b1.l2.w $T.b1.l2.ww -side left -anchor w -padx $_10" );

			cmd( "ttk::frame $T.b1.l3" );
			cmd( "ttk::checkbutton $T.b1.l3.par -text Parallel -variable parallel -width 15" );
			cmd( "ttk::checkbutton $T.b1.l3.int -text Integer -variable nature -width 15" );
			cmd( "pack $T.b1.l3.par $T.b1.l3.int -side left -anchor w -padx $_10" );

			cmd( "pack $T.b1.l1 $T.b1.l2 $T.b1.l3 -anchor w -padx $_5" );

			cmd( "tooltip::tooltip $T.b1.l1.n \"Save the element series for analysis to memory or results file\"" );
			cmd( "tooltip::tooltip $T.b1.l1.i \"Save the element series for analysis to a separate file\"" );
			cmd( "tooltip::tooltip $T.b1.l1.plt \"Observe the element series during simulation execution\"" );
			cmd( "tooltip::tooltip $T.b1.l2.deb \"Trigger debugger after element is updated\"" );
			cmd( "tooltip::tooltip $T.b1.l2.w \"Trigger debugger when element is accessed\"" );
			cmd( "tooltip::tooltip $T.b1.l2.ww \"Trigger debugger only when element is modified\"" );
			cmd( "tooltip::tooltip $T.b1.l3.par \"Allow multi-object parallel updating of this element\"" );
			cmd( "tooltip::tooltip $T.b1.l3.int \"Element is integer (round to integer otherwise)\"" );

			switch ( cv->param )
			{
				case 1:
					cmd( ".chgelem.b1.l2.deb configure -state disabled" );
					cmd( ".chgelem.b1.l3.par configure -state disabled" );
					break;

				case 2:
					cmd( ".chgelem.b1.l2.w configure -state disabled" );
					cmd( ".chgelem.b1.l3.par configure -state disabled" );
			}

			cmd( "ttk::frame $T.b2" );
			cmd( "ttk::label $T.b2.l -text \"Include in documentation to be\"" );
			cmd( "ttk::checkbutton $T.b2.ini -text \"Initialized\" -variable initial -underline 0" );

			if ( cv->param != 1 && cv->num_lag == 0 )
				cmd( "$T.b2.ini configure -state disabled" );

			cmd( "ttk::checkbutton $T.b2.obs -text \"Observed\" -variable observe -underline 0" );

			if ( cv->param == 2 )
				cmd( "$T.b2.obs configure -state disabled" );

			cmd( "pack $T.b2.l $T.b2.obs $T.b2.ini -side left -padx $_5" );

			cmd( "ttk::frame $T.b3" );
			cmd( "ttk::frame $T.b3.min" );
			cmd( "ttk::label $T.b3.min.l -width 10 -anchor e -text \"Minimum\"" );
			cmd( "ttk::entry $T.b3.min.e -textvariable vmin -width 15 -justify center" );
			cmd( "tooltip::tooltip $T.b3.min.e \"Minimum value allowed for element (%s or blank for no limit)\"", NON_AVAILABLE );
			cmd( "pack $T.b3.min.l $T.b3.min.e -side left -anchor w -padx $_2 -pady $_2" );
			cmd( "ttk::frame $T.b3.max" );
			cmd( "ttk::label $T.b3.max.l -width 10 -anchor e -text \"Maximum\"" );
			cmd( "ttk::entry $T.b3.max.e -textvariable vmax -width 15 -justify center" );
			cmd( "tooltip::tooltip $T.b3.max.e \"Maximum value allowed for element (%s or blank for no limit)\"", NON_AVAILABLE );
			cmd( "pack $T.b3.max.l $T.b3.max.e -side left -anchor w -padx $_2 -pady $_2" );
			cmd( "pack $T.b3.min $T.b3.max -anchor w -side left -padx $_5" );

			cmd( "pack $T.h $T.b0 $T.b1 $T.b2 $T.b3 -pady $_5" );

			cmd( "set Td $T.desc" );
			cmd( "ttk::frame $Td" );

			cmd( "ttk::frame $Td.f" );
			cmd( "ttk::label $Td.f.int -text \"Description\"" );

			cmd( "ttk::frame $Td.f.desc" );
			cmd( "ttk::scrollbar $Td.f.desc.yscroll -command \"$Td.f.desc.text yview\"" );
			cmd( "ttk::text $Td.f.desc.text -wrap word -width 60 -height 6 -yscrollcommand \"$Td.f.desc.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
			cmd( "pack $Td.f.desc.yscroll -side right -fill y" );
			cmd( "pack $Td.f.desc.text -anchor w -expand yes -fill both" );
			cmd( "mouse_wheel $Td.f.desc.text" );

			cmd( "pack $Td.f.int $Td.f.desc" );

			cmd( "ttk::frame $Td.b" );
			cmd( "ttk::button $Td.b.eq -width [ expr { $butWid + 2 } ] -text Equation -command { set done 3 } -underline 1" );
			cmd( "ttk::button $Td.b.auto_doc -width [ expr { $butWid + 2 } ] -text \"Auto Desc.\" -command { set done 9 } -underline 0" );
			cmd( "ttk::button $Td.b.us -width [ expr { $butWid + 2 } ] -text \"Using Elem.\" -command { set done 4 } -underline 0" );
			cmd( "ttk::button $Td.b.using -width [ expr { $butWid + 2 } ] -text \"Elem. Used\" -command { set done 7 } -underline 0" );

			if ( ! strcmp( cd->type, "Parameter" ) )
				cmd( "pack $Td.b.auto_doc $Td.b.us -padx $butSpc -side left -pady $_5" );
			else
			{
				cmd( "pack $Td.b.eq $Td.b.auto_doc $Td.b.us $Td.b.using -padx $butSpc -side left -pady $_5" );
				cmd( "bind $T <Control-q> \"$Td.b.eq invoke\"; bind $T <Control-Q> \"$Td.b.eq invoke\"" );
				cmd( "bind $T <Control-e> \"$Td.b.using invoke\"; bind $T <Control-E> \"$Td.b.using invoke\"" );
			}

			cmd( "tooltip::tooltip $Td.b.eq \"Show variable's equation code\"" );
			cmd( "tooltip::tooltip $Td.b.auto_doc \"Get description from equation file\"" );
			cmd( "tooltip::tooltip $Td.b.us \"List all variables using this element\"" );
			cmd( "tooltip::tooltip $Td.b.using \"List all variables and parameters used\"" );

			if ( cv->param == 1 || cv->num_lag > 0 )
			{
				cmd( "ttk::frame $Td.i" );
				cmd( "ttk::label $Td.i.int -text \"Initial values description\"" );

				cmd( "ttk::frame $Td.i.desc" );
				cmd( "ttk::scrollbar $Td.i.desc.yscroll -command \"$Td.i.desc.text yview\"" );
				cmd( "ttk::text $Td.i.desc.text -wrap word -width 60 -height 3 -yscrollcommand \"$Td.i.desc.yscroll set\" -dark $darkTheme -style smallFixed.TText" );
				cmd( "pack $Td.i.desc.yscroll -side right -fill y" );
				cmd( "pack $Td.i.desc.text -anchor w -expand yes -fill both" );
				cmd( "mouse_wheel $Td.i.desc.text" );

				cmd( "pack $Td.i.int $Td.i.desc" );

				cmd( "ttk::frame $Td.b2" );
				cmd( "ttk::button $Td.b2.setall -width [ expr { $butWid + 2 } ] -text \"Initial Values\" -command { set done 11 } -underline 1" );
				cmd( "ttk::button $Td.b2.sens -width [ expr { $butWid + 2 } ] -text \"Sensitivity\" -command { set done 12 } -underline 5" );

				if ( cv->param == 0 )
				{
					cmd( "ttk::button $Td.b2.da -width [ expr { $butWid + 2 } ] -text \"Assimilation\" -command { set done 15 }" );
					cmd( "tooltip::tooltip $Td.b2.da \"Set data assimilation values for this element\"" );
					cmd( "pack $Td.b2.setall $Td.b2.sens $Td.b2.da -padx $butSpc -side left -pady $_5" );
				}
				else
					cmd( "pack $Td.b2.setall $Td.b2.sens -padx $butSpc -side left -pady $_5" );

				cmd( "pack $Td.f $Td.b $Td.i $Td.b2 -pady $_5" );

				cmd( "tooltip::tooltip $Td.b2.setall \"Set initial value(s) of this element\"" );
				cmd( "tooltip::tooltip $Td.b2.sens \"Set sensitivity analysis values for this element\"" );

				cmd( "bind $T <Control-n> \"$Td.b2.setall invoke\"; bind $T <Control-N> \"$Td.b2.setall invoke\"" );
				cmd( "bind $T <Control-t> \"$Td.b2.sens invoke\"; bind $T <Control-T> \"$Td.b2.sens invoke\"" );
			}
			else
				if ( cv->param == 0 )
				{
					cmd( "ttk::button $Td.da -width [ expr { $butWid + 2 } ] -text \"Assimilation\" -command { set done 15 }" );
					cmd( "tooltip::tooltip $Td.da \"Set data assimilation values for this element\"" );

					cmd( "pack $Td.f $Td.b $Td.da -pady $_5" );
				}
				else
					cmd( "pack $Td.f $Td.b -pady $_5" );

			cmd( "pack $Td -pady $_5" );

			cmd( "okhelpcancel $T b { set done 1 } { LsdHelp browser.html#changeelement } { set done 2 }" );

			cmd( "bind $T <Control-r> \"$T.b0.prop invoke\"; bind $T <Control-R> \"$T.b0.prop invoke\"" );
			cmd( "bind $T <Control-m> \"$T.b0.mov invoke\"; bind $T <Control-M> \"$T.b0.mov invoke\"" );
			cmd( "bind $T <Control-l> \"$T.b0.del invoke\"; bind $T <Control-L> \"$T.b0.del invoke\"" );
			cmd( "bind $T <Control-s> \"$T.b1.l1.n invoke\"; bind $T <Control-S> \"$T.b1.l1.n invoke\"" );
			cmd( "bind $T <Control-f> \"$T.b1.l1.i invoke\"; bind $T <Control-F> \"$T.b1.l1.i invoke\"" );
			cmd( "bind $T <Control-p> \"$T.b1.l1.plt invoke\"; bind $T <Control-P> \"$T.b1.l1.plt invoke\"" );
			cmd( "bind $T <Control-d> \"$T.b1.l2.deb invoke\"; bind $T <Control-D> \"$T.b1.l2.deb invoke\"" );
			cmd( "bind $T <Control-w> \"$T.b1.l2.w invoke\"; bind $T <Control-W> \"$T.b1.l2.w invoke\"" );
			cmd( "bind $T <Control-h> \"$T.b1.l2.ww invoke\"; bind $T <Control-H> \"$T.b1.l2.ww invoke\"" );
			cmd( "bind $T <Control-i> \"$T.b2.ini invoke\"; bind $T <Control-I> \"$T.b2.ini invoke\"" );
			cmd( "bind $T <Control-o> \"$T.b2.obs invoke\"; bind $T <Control-O> \"$T.b2.obs invoke\"" );
			cmd( "bind $T <Control-a> \"$Td.b.auto_doc invoke\"; bind $T <Control-A> \"$Td.b.auto_doc invoke\"" );
			cmd( "bind $T <Control-u> \"$Td.b.us invoke\"; bind $T <Control-U> \"$Td.b.us invoke\"" );

			cmd( "showtop $T topleftW" );
			cmd( "mousewarpto $T.b.ok" );

			cmd( "$Td.f.desc.text insert end \"%s\"", strtcl( buf_descr, cd->text, MAX_BUFF_SIZE ) );

			if ( cv->param == 1 || cv->num_lag > 0 )
				cmd( "$Td.i.desc.text insert end \"%s\"", strtcl( buf_descr, cd->init, MAX_BUFF_SIZE ) );

			cycle_var:

			done = 0;
			while ( done == 0 )
				Tcl_DoOneEvent( 0 );

			if ( done == 3 )
				show_eq( lab_old, ".chgelem" );

			if ( done == 4 )
				scan_used_lab( lab_old, ".chgelem" );

			if ( done == 7 )
				scan_using_lab( lab_old, ".chgelem" );

			choice = 0;

			if ( done == 9 )
			{
				sim.change_description( lab_old, NULL, -1, eval_str( "[ .chgelem.desc.f.desc.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

				auto_document( lab_old, "ALL", true );
				cmd( ".chgelem.desc.f.desc.text delete 1.0 end" );
				cmd( ".chgelem.desc.f.desc.text insert end \"%s\"", strtcl( buf_descr, cd->text, MAX_BUFF_SIZE ) );

				unsaved_change( true );		// signal unsaved change
			}

			if ( done == 7 || done == 4 || done == 3 || done == 9 )
			  goto cycle_var;

			if ( done == 2 || done == 8 )	// esc/cancel
			{
				redrawRoot = redrawStruc = false;	// no redraw necessary
				goto here_endelem;
			}
			else
			{
				cmd( "set choice $observe" );
				observe = choice ? true : false;
				cmd( "set choice $initial" );
				initial = choice ? true : false;
				cd->initial = initial;
				cd->observe = observe;

				if ( debug )
				{
					if ( watch_write )
						deb_mode = 'R';
					else
						if ( watch )
							deb_mode = 'W';
						else
							deb_mode = 'd';
				}
				else
				{
					if ( watch_write )
						deb_mode = 'r';
					else
						if ( watch )
							deb_mode = 'w';
						else
							deb_mode = 'n';
				}

				double vmax = get_double( "vmax", NULL, true ),
					   vmin = get_double( "vmin", NULL, true );

				if ( ! std::isnan( vmin ) && vmax < vmin )
				{
					cmd( "ttk::messageBox -parent .chgelem -type ok -title Warning -icon warning -message \"Invalid maximum value\" -detail \"Maximum element value '%g' is less than the minimum value '%g', discarding.\"", vmax, vmin );
					vmax = NAN;
				}

				for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
				{
					cv = cur->search_var( NULL, lab_old );
					cv->save = save;
					cv->savei = savei;
					cv->deb_mode = deb_mode;
					cv->plot = plot;
					cv->parallel = parallel;
					cv->observe = observe;
					cv->integer = nature;
					cv->max_val = vmax;
					cv->min_val = vmin;

					// ensure variable constraints are respected
					for ( i = 0; i < ( cv->param == 1 ? 1 : cv->num_lag ); ++i )
						cv->val[ i ] = cv->chk_val( cv->val[ i ] );
				}

				sim.change_description( lab_old, NULL, -1, eval_str( "[ .chgelem.desc.f.desc.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

				if ( cv->param == 1 || cv->num_lag > 0 )
					sim.change_description( lab_old, NULL, -1, NULL, eval_str( "[ .chgelem.desc.i.desc.text get 1.0 end ]", buf_descr, MAX_BUFF_SIZE ) );

				unsaved_change( true );		// signal unsaved change

				if ( save == 1 || savei == 1 )
					for ( cur = r; cur != NULL; cur = cur->up )
						if ( ! cur->to_compute )
							cmd( "ttk::messageBox -parent .chgelem -type ok -title Warning -icon warning -message \"Cannot save element\" -detail \"Element '%s' set to be saved but it will not be computed for the Analysis of Results, since object '%s' is not set to be computed.\"", lab_old, cur->label );
			}

			if ( done != 8 )
				choice = 0;
			else
				choice = 7;

			here_endelem:

			cmd( "destroytop .chgelem" );
			Tcl_UnlinkVar( interp, "done" );
			Tcl_UnlinkVar( interp, "save" );
			Tcl_UnlinkVar( interp, "savei" );
			Tcl_UnlinkVar( interp, "plot" );
			Tcl_UnlinkVar( interp, "debug" );
			Tcl_UnlinkVar( interp, "watch" );
			Tcl_UnlinkVar( interp, "watch_write" );
			Tcl_UnlinkVar( interp, "parallel" );
			Tcl_UnlinkVar( interp, "nature" );
			cmd( "unset done" );

			// options to be handled in a second run of the operate function
			switch ( done )
			{
				case 5:
					choice = 75;			// open properties box for $vname
					break;
				case 10:
					choice = 76;			// delete element in $vname
					break;
				case 11:
					choice = 77;			// change initial values for $vname
					break;
				case 12:
					choice = 78;			// change sensitivity values for $vname
					break;
				case 13:
					choice = 79;			// move element in $vname
					break;
				case 14:
					choice = 96;			// change updating scheme
					break;
				case 15:					// assimilation settings
					choice = 15;
					break;
				default:
					choice = 0;
			}

			if ( choice != 0 )
			{
				redrawRoot = redrawStruc = false;// no redraw yet
				return r;					// execute command
			}

		break;


		// edit variable/parameter (defined by tcl $vname) properties
		case 75:
		// delete variable/parameter (defined by tcl $vname)
		case 76:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );	// get var/par name in lab_old
			cv = r->search_var( NULL, lab_old );
			if ( cv == NULL )
				break;

			if ( choice == 76 )
			{
				delVar = renVar = true;

				cmd( "set answer [ ttk::messageBox -parent . -title Confirmation -icon question -type yesno -default yes -message \"Delete element?\" -detail \"Press 'Yes' to confirm deleting '$vname'\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );
				if ( choice == 1 )
					cmd( "set vname \"\"; set nature 3; set numlag 0" );	// configure to delete
				else
					goto here_endprop;
			}
			else
			{
				delVar = renVar = false;

				cmd( "set nature %d", cv->param );
				cmd( "if { $nature == 0 } { set numlag %d } { set numlag 0 }", cv->num_lag );

				cmd( "set T .prop" );
				cmd( "newtop $T \"Properties\" { set choice 2 }" );

				cmd( "ttk::frame $T.h" );
				cmd( "ttk::label $T.h.l1 -text \"Element:\"" );
				cmd( "ttk::label $T.h.l2 -text \"%s\" -style hl.TLabel", cv->label );
				cmd( "pack $T.h.l1 $T.h.l2 -side left -padx $_2" );

				cmd( "ttk::frame $T.n" );
				cmd( "ttk::label $T.n.var -text \"Name\"" );
				cmd( "ttk::entry $T.n.e -width 20 -textvariable vname -justify center" );
				cmd( "ttk::label $T.n.sp -width 2" );
				cmd( "ttk::label $T.n.l -text \"Lags\"" );
				cmd( "ttk::spinbox $T.n.lag -justify center -width 3 -from 0 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set numlag %%P; return 1 } { %%W delete 0 end; %%W insert 0 $numlag; return 0 } } -invalidcommand { bell }" );
				cmd( "$T.n.lag insert 0 $numlag" );
				cmd( "if { $nature != 0 } { $T.n.lag configure -state disabled }" );
				cmd( "pack $T.n.var $T.n.e $T.n.sp $T.n.l $T.n.lag -side left -padx $_2" );

				cmd( "tooltip::tooltip $T.n.lag \"Maximum lag used in equations\"" );

				cmd( "ttk::frame $T.v" );
				cmd( "ttk::label $T.v.l -text \"Type\"" );

				cmd( "ttk::frame $T.v.o -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
				cmd( "ttk::radiobutton $T.v.o.var -text Variable -variable nature -value 0 -underline 0 -command { $T.n.lag configure -state normal }" );
				cmd( "ttk::radiobutton $T.v.o.par -text Parameter -variable nature -value 1 -underline 0 -command { $T.n.lag configure -state disabled }" );
				cmd( "ttk::radiobutton $T.v.o.fun -text Function -variable nature -value 2 -underline 0 -command { $T.n.lag configure -state disabled }" );
				cmd( "pack	$T.v.o.var $T.v.o.par $T.v.o.fun -anchor w" );

				cmd( "pack $T.v.l $T.v.o" );

				cmd( "pack $T.h $T.n $T.v -padx $_5 -pady $_5" );

				cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menumodel.html#change_nature } { set choice 2 }" );

				cmd( "bind $T.n.e <KeyPress-Return> { set choice 1 }" );
				cmd( "bind $T <Control-v> { .prop.v.o.var invoke }; bind $T <Control-V> { .prop.v.o.var invoke }" );
				cmd( "bind $T <Control-p> { .prop.v.o.par invoke }; bind $T <Control-P> { .prop.v.o.par invoke }" );
				cmd( "bind $T <Control-f> { .prop.v.o.fun invoke }; bind $T <Control-F> { .prop.v.o.fun invoke }" );

				cmd( "showtop $T" );
				cmd( "mousewarpto $T.b.ok 0" );
				cmd( "$T.n.e selection range 0 end" );
				cmd( "focus $T.n.e" );

				choice = 0;
			}

			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "if [ winfo exists .prop ] { if { $nature == 0 } { set numlag [ .prop.n.lag get ] } }" );
			cmd( "destroytop .prop" );

			if ( choice == 2 )
				goto here_endprop;

			cmd( "set choice $nature" );
			nature = choice;

			cmd( "set choice $numlag" );
			numlag = choice;

			if ( ! delVar && ( nature != cv->param || numlag != cv->num_lag ) )
			{
				if ( nature != 1 && numlag == 0 )
					sim.change_description( lab_old, NULL, nature, NULL, "" );
				else
					sim.change_description( lab_old, NULL, nature );

				for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
				{
					cv = cur->search_var( NULL, lab_old );

					if ( cv == NULL )
						continue;

					double *old_val = cv->val;
					cv->val = new double[ numlag + 1 ];

					for ( i = 0; i <= numlag; ++i )
						cv->val[ i ] = 0;

					// avoid reseting initial values if not required
					if ( ( cv->param == 1 && numlag > 0 ) || ( nature == 1 && cv->num_lag > 0 ) )
						cv->val[ 0 ] = old_val[ 0 ];		// parameter <-> lagged variable
					else
						if ( cv->num_lag > 0 && numlag > 0 )// x-lags variable to y-lags variable?
							for ( i = 0; i < std::min( cv->num_lag, numlag ); ++i )
								cv->val[ i ] = old_val[ i ];

					delete [ ] old_val;
					cv->num_lag = numlag;
					cv->param = nature;

					if ( cv->param == 1 || cv->num_lag > 0 )
						cv->initialized = false;

					if ( cv->param != 0 )
					{
						cv->parallel = false;
						cv->period = 1;
						cv->delay = cv->delay_range = cv->period_range = 0;
					}
				}
			}

			lab1 = get_str( "vname" );
			if ( lab1 != NULL && strcmp( lab1, "" ) )
			{
				sscanf( lab1, "%99s", lab );			// new name in lab (empty if delete)
				if ( strcmp( lab, lab_old ) )			// check new name if different
					renVar = true;
			}
			else
				if ( delVar )
					strcpy( lab, "" );
				else
					goto here_endprop;

			if ( renVar )
			{
				if ( ! delVar )
				{
					for ( cur = r; cur->up != NULL; cur = cur->up );
					choice = cur->check_label( lab );

					if ( choice == 1 )
					{
						cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"The name already exists in the model\" -detail \"Choose a different name and try again.\"" );
						goto here_endprop;
					}
					if ( choice == 2 )
					{
						cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid characters in name\" -detail \"Names must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
						goto here_endprop;
					}
				}

				// remove from element lists
				cmd( "if [ info exists modElem ] { set pos [ lsearch -exact $modElem %s ]; if { $pos >= 0 } { set modElem [ lreplace $modElem $pos $pos ] } }", lab_old	 );
				cmd( "if [ info exists modPar ] { set pos [ lsearch -exact $modPar %s ]; if { $pos >= 0 } { set modPar [ lreplace $modPar $pos $pos ] } }", lab_old	 );
				cmd( "if [ info exists modVar ] { set pos [ lsearch -exact $modVar %s ]; if { $pos >= 0 } { set modVar [ lreplace $modVar $pos $pos ] } }", lab_old	 );
				cmd( "if [ info exists modFun ] { set pos [ lsearch -exact $modFun %s ]; if { $pos >= 0 } { set modFun [ lreplace $modFun $pos $pos ] } }", lab_old	 );

				if ( ! delVar )
				{
					// add to element lists
					cmd( "lappend modElem %s", lab );

					if ( cv->param == 0 )
						cmd( "lappend modVar %s", lab );
					if ( cv->param == 1 )
						cmd( "lappend modPar %s", lab );
					if ( cv->param == 2 )
						cmd( "lappend modFun %s", lab );

					sim.change_description( lab_old, lab );
				}

				for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
					if ( ! delVar )
						cur->chg_var_lab( lab_old, lab );
					else
						cur->delete_var( lab_old );
			}

			if ( sim.root->v == NULL && sim.root->b == NULL )// if last variable
			{
				unsaved_change( false );				// no unsaved change
				sim.conf_ok = false;					// no config loaded
			}
			else
				unsaved_change( true );					// signal unsaved change

			redrawRoot = redrawStruc = true;			// force browser/structure redraw

			here_endprop:

		break;


		// Move variable/parameter (defined by tcl $vname)
		case 79:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );	// get var/par name in lab_old

			cmd( "set TT .objs" );
			cmd( "newtop $TT \"Move\" { set choice 2 }" );

			cmd( "ttk::frame $TT.l" );
			cmd( "ttk::label $TT.l.l -text \"Element:\"" );
			cmd( "ttk::label $TT.l.n -style hl.TLabel -text \"%s\"", lab_old );
			cmd( "pack $TT.l.l $TT.l.n -side left -padx $_2" );

			cmd( "ttk::frame $TT.v" );
			cmd( "ttk::label $TT.v.l -text \"Move to\"" );

			cmd( "ttk::frame $TT.v.t" );
			cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
			cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
			cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
			cmd( "mouse_wheel $TT.v.t.lb" );
			sim.root->insert_object( "$TT.v.t.lb" );
			cmd( "pack $TT.v.l $TT.v.t" );

			cmd( "pack $TT.l $TT.v -padx $_5 -pady $_5" );

			cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

			cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0; break }" );
			cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end; break }" );
			cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

			cmd( "showtop $TT" );
			cmd( "$TT.v.t.lb selection set 0" );
			cmd( "focus $TT.v.t.lb" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set movelabel [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ]" );
			cmd( "destroytop .objs" );

			if ( choice == 2 )
				break;

			lab1 = get_str( "movelabel" );
			if ( lab1 == NULL || ! strcmp( lab1, r->label ) )		// same object?
				break;

			cv = r->search_var( NULL, lab_old );

			for ( cur = sim.root->search( lab1 ); cur != NULL; cur = cur->hyper_next( cur->label ) )
				cur->add_var_from_example( cv );

			for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
				cur->delete_var( lab_old );

			unsaved_change( true );		// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw

		break;


		// Change variable/parameter (defined by tcl $vname) initial values
		case 77:
		// Change variable/parameter (defined by tcl $vname) sensitivity values
		case 78:

			done = ( choice == 77 ) ? 1 : 2;

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );		// get var/par name in lab_old

			if ( initVal && initParent != NULL )
				cv = initParent->search_var( NULL, lab_old );// get var/par pointer
			else
				cv = r->search_var( NULL, lab_old );// get var/par pointer

			if ( cv == NULL )
				break;

			// do lag selection, if necessary, for initialization/sensitivity data entry
			lag = 0;								// lag option for the next cases (first lag)
			if ( ! initVal && ( cv->param == 0 || cv->param == 2 ) && cv->num_lag > 1 )
			{										// more than one lag to choose?
				cmd( "set lag \"1\"" );

				// confirm which lag to use
				cmd( "set T .lag" );
				cmd( "newtop $T \"Lag Selection\" { set choice 0 }" );

				cmd( "ttk::frame $T.i" );
				cmd( "ttk::label $T.i.l -text \"Use lag\"" );
				cmd( "ttk::spinbox $T.i.e -justify center -width 3 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 && $n <= %d } { set lag %%P; return 1 } { %%W delete 0 end; %%W insert 0 $lag; return 0 } } -invalidcommand { bell }", cv->num_lag, cv->num_lag );
				cmd( "$T.i.e insert 0 $lag" );
				cmd( "pack $T.i.l $T.i.e -side left -padx $_2" );

				cmd( "ttk::frame $T.o" );
				cmd( "ttk::label $T.o.l1 -text \"( valid values:\"" );
				cmd( "ttk::label $T.o.w1 -text 1 -style hl.TLabel" );
				cmd( "ttk::label $T.o.l2 -text to" );
				cmd( "ttk::label $T.o.w2 -text %d -style hl.TLabel", cv->num_lag );
				cmd( "ttk::label $T.o.l3 -text \")\"" );
				cmd( "pack $T.o.l1 $T.o.w1 $T.o.l2 $T.o.w2 $T.o.l3 -side left -padx $_2" );

				cmd( "pack $T.i $T.o -padx $_5 -pady $_5" );

				cmd( "okcancel $T b { set choice $lag } { set choice 0 }" );
				cmd( "bind $T <KeyPress-Return> { set choice $lag }" );

				cmd( "showtop $T" );
				cmd( "mousewarpto $T.b.ok 0" );

				choice = -1;
				while ( choice == -1 )			// wait for user action
					Tcl_DoOneEvent( 0 );

				cmd( "set lag [ .lag.i.e get ]" );
				cmd( "destroytop .lag" );

				if ( choice == 0 )
					break;

				cmd( "set choice $lag" );
				lag = abs( choice ) - 1;		// try to extract chosed lag

				// abort if necessary
				if ( lag < 0 || lag > ( cv->num_lag - 1 ) )
				{
					cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid lag selected\" -detail \"Select a valid lag value for the variable or change the number of lagged values for this variable.\"" );
					break;
				}
			}

			// initialize
			if ( done == 1 )
			{
				if ( initVal )					// running just after element creation?
				{
					lag = next_lag;
					cur = initParent;
				}
				else
					cur = r;

				cur->set_all( cv->label, lag );
				redrawRoot = true;				// redraw is needed to show new value tip

				if ( initVal )
				{
					if ( next_lag < ( cv->num_lag - 1 ) )
					{
						++next_lag;
						choice = 77;			// execute command again
						return r;
					}
					else
					{
						initVal = false;
						initParent = NULL;
						redrawStruc = true;		// redraw is needed to show new element
					}
				}
			}
			else								// edit sensitivity analysis data
			{
				if ( ( cs = sim.search_sensitivity( cv->label, lag ) ) == NULL )
					cs = new lsd::sensitivity( cv->label, & sim, cv->param, lag, cv->integer );

				i = cs->dataentry( );

				if ( i == 2 )					// data entry failed, no data?
					delete cs;
				else
					if ( i == 0 )
						unsavedSense = true;	// signal unsaved change
			}

		break;


		// add data assimilation settings to variable
		case 15:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );	// get var/par name in lab_old
			cv = r->search_var( NULL, lab_old );// get var/par pointer
			if ( cv == NULL )
				break;

			if ( ( ca = sim.search_assimilation( cv->label ) ) == NULL )
				ca = new lsd::assimilation( cv->label, & sim );

			i = ca->dataentry( );

			if ( i == 2 )
				delete ca;						// configuration failed, no data
			else
				if ( i == 0 )
					unsavedChange = true;		// signal unsaved change

		break;


		// change variable (defined by tcl $vname) updating scheme
		case 96:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );	// get var/par name in lab_old
			cv = r->search_var( NULL, lab_old );// get var/par pointer
			if ( cv == NULL )
				break;

			// save previous values to allow canceling operation
			temp[ 1 ] = cv->delay;
			temp[ 2 ] = cv->delay_range;
			temp[ 3 ] = cv->period;
			temp[ 4 ] = cv->period_range;

			Tcl_LinkVar( interp, "delay", ( char * ) & cv->delay, TCL_LINK_INT );
			Tcl_LinkVar( interp, "delay_range", ( char * ) & cv->delay_range, TCL_LINK_INT );
			Tcl_LinkVar( interp, "period", ( char * ) & cv->period, TCL_LINK_INT );
			Tcl_LinkVar( interp, "period_range", ( char * ) & cv->period_range, TCL_LINK_INT );

			cmd( "set T .updating" );
			cmd( "newtop $T \"Variable Updating\" { set choice 2 }" );

			cmd( "ttk::frame $T.h" );
			cmd( "ttk::label $T.h.l1 -text \"Variable:\"" );
			cmd( "ttk::label $T.h.l2 -text \"%s\" -style hl.TLabel", cv->label );
			cmd( "pack $T.h.l1 $T.h.l2 -side left -padx $_2" );

			cmd( "ttk::frame $T.f" );

			cmd( "ttk::frame $T.f.c" );
			cmd( "ttk::label $T.f.c.l2 -width 20 -anchor e -text \"Initial delay\"" );
			cmd( "ttk::spinbox $T.f.c.e2 -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set delay %%P; return 1 } { %%W delete 0 end; %%W insert 0 $delay; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.c.e2 insert 0 $delay" );
			cmd( "pack $T.f.c.l2 $T.f.c.e2 -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "ttk::frame $T.f.a" );
			cmd( "ttk::label $T.f.a.l -width 20 -anchor e -text \"Random delay range\"" );
			cmd( "ttk::spinbox $T.f.a.e -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set delay_range %%P; return 1 } { %%W delete 0 end; %%W insert 0 $delay_range; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.a.e insert 0 $delay_range" );
			cmd( "pack $T.f.a.l $T.f.a.e -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "ttk::frame $T.f.b" );
			cmd( "ttk::label $T.f.b.l1 -width 20 -anchor e -text \"Period\"" );
			cmd( "ttk::spinbox $T.f.b.e1 -width 7 -from 1 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set period %%P; return 1 } { %%W delete 0 end; %%W insert 0 $period; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.b.e1 insert 0 $period" );
			cmd( "pack $T.f.b.l1 $T.f.b.e1 -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "ttk::frame $T.f.d" );
			cmd( "ttk::label $T.f.d.l2 -width 20 -anchor e -text \"Random period range\"" );
			cmd( "ttk::spinbox $T.f.d.e2 -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set period_range %%P; return 1 } { %%W delete 0 end; %%W insert 0 $period_range; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.d.e2 insert 0 $period_range" );
			cmd( "pack $T.f.d.l2 $T.f.d.e2 -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "pack $T.f.c $T.f.a $T.f.b $T.f.d -anchor w" );

			cmd( "pack $T.h $T.f -padx $_5 -pady $_5" );

			cmd( "tooltip::tooltip $T.f.c \"First case (time step) to compute the variable\"" );
			cmd( "tooltip::tooltip $T.f.a \"Maximum case (time step) for uniform random first computation\"" );
			cmd( "tooltip::tooltip $T.f.b \"Period between computations of variable\"" );
			cmd( "tooltip::tooltip $T.f.d \"Maximum period for uniform random periodic computation\"" );

			cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp browser.html#updating } { set choice 2 }" );

			cmd( "bind $T.f.c.e2 <KeyPress-Return> { focus $T.f.a.e; $T.f.a.e selection range 0 end }" );
			cmd( "bind $T.f.a.e <KeyPress-Return> { focus $T.f.b.e1; $T.f.b.e1 selection range 0 end }" );
			cmd( "bind $T.f.b.e1 <KeyPress-Return> { focus $T.f.d.e2; $T.f.d.e2 selection range 0 end }" );
			cmd( "bind $T.f.d.e2 <KeyPress-Return> { focus $T.f.e.e2; $T.f.e.e2 selection range 0 end }" );

			cmd( "showtop $T" );
			cmd( "mousewarpto $T.b.ok 0" );
			cmd( "$T.f.c.e2 selection range 0 end" );
			cmd( "focus $T.f.c.e2" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set delay [ $T.f.c.e2 get ]" );
			cmd( "set delay_range [ $T.f.a.e get ]" );
			cmd( "set period [ $T.f.b.e1 get ]" );
			cmd( "set period_range [ $T.f.d.e2 get ]" );

			cmd( "destroytop $T" );

			if ( choice == 2 )	// Escape - revert previous values
			{
				cv->delay = temp[ 1 ];
				cv->delay_range = temp[ 2 ];
				cv->period = temp[ 3 ];
				cv->period_range = temp[ 4 ];
			}
			else
			// signal unsaved change if anything to be saved
				if ( temp[ 1 ] != cv->delay || temp[ 2 ] != cv->delay_range || temp[ 3 ] != cv->period || temp[ 4 ] != cv->period_range )
				{
					for ( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
					{
						cv1 = cur->search_var( NULL, lab_old );
						cv1->delay = cv->delay;
						cv1->delay_range = cv->delay_range;
						cv1->period = cv->period;
						cv1->period_range = cv->period_range;
					}

					unsaved_change( true );
					redrawRoot = true;
				}

			Tcl_UnlinkVar( interp, "delay" );
			Tcl_UnlinkVar( interp, "delay_range" );
			Tcl_UnlinkVar( interp, "period" );
			Tcl_UnlinkVar( interp, "period_range" );

		break;


		// Load a model
		case 17:
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "set path \"%s\"", sim.conf_path );
			else
				cmd( "set path \"%s\"", lsd::model_path );

			cmd( "cd $path" );

		// Reload model
		case 38:

			if ( discard_change( ) )	// unsaved configuration changes ?
				if ( ! open_configuration( r, choice == 38 ? true : false ) )
				{
					unload_configuration_gui( true );
					choice = 0;
					return sim.root;
				}

		break;


		// Save a model
		case 18:
		// Save a model as different name
		case 73:

			saveAs = ( choice == 73 || strlen( sim.conf_name ) == 0 ) ? true : false;

			if ( ! sim.conf_ok )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration to save\" -detail \"Create a configuration before saving.\"" );
				break;
			}

			Tcl_LinkVar( interp, "done", ( char * ) &done, TCL_LINK_INT );

			if ( sim.eff_t > 0 )
			{
				if ( sim.save_ok )
					cmd( "set answer [ ttk::messageBox -parent . -type okcancel -default cancel -icon warning -title Warning -message \"Configuration is the final state of a simulation run\" -detail \"Press 'OK' to save it anyway%s or 'Cancel' to abort saving.\" ]; switch -- $answer { ok { set done 1 } cancel { set done 2 } }", saveAs ? "" : " under a different name" );
				else
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration cannot be saved\" -detail \"Current configuration is the final state of a simulation run which has an incomplete structure that cannot be reliably saved.\n\nThis is due to the usage of USE_ZERO_INSTANCE macro, which allowed zero-instance objects in the current model structure.\"; set done 2" );

				if ( done == 2 )
				{
					Tcl_UnlinkVar( interp, "done" );
					cmd( "unset done" );
					break;
				}

				saveAs = true;		// require file name to save
			 }

			done = 0;
			cmd( "set res \"%s\"", strlen( sim.conf_name ) > 0 ? sim.conf_name : DEF_CONF_FILE );
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			if ( saveAs )			// only asks file name if instructed to or necessary
			{
				if ( sim.eff_t > 0 )
				{
					cmd( "set fn [ tk_getSaveFile -parent . -title \"Save Configuration File\" -defaultextension \".lsd\" -initialdir $path -filetypes { { {LSD model files} {.lsd} } } ]" );
					cmd( "if { [ string equal -nocase [ file normalize $fn ] [ file normalize \"$path/$res.lsd\" ] ] && [ ttk::messageBox -parent . -type okcancel -default cancel -icon warning -title Warning -message \"Overwrite existing configuration?\" -detail \"The original model configuration will be overwritten by the final state of the simulation run and, therefore, lost.\n\nPress 'OK' if you are sure or 'Cancel' to abort saving.\" ] eq \"cancel\" } { set fn \"\" }" );
				}
				else
					cmd( "set fn [ tk_getSaveFile -parent . -title \"Save Configuration File\" -defaultextension \".lsd\" -initialfile $res -initialdir $path -filetypes { { {LSD model files} {.lsd} } } ]" );

				cmd( "if { [ string length $fn ] > 0 && ! [ fn_spaces \"$fn\" . ] } { \
							set path [ file dirname $fn ]; \
							set fn [ string map -nocase [ list [ file extension $fn ] \"\" ] [ file tail $fn ] ]; \
						} else { \
							set done 2 \
						}" );

				if ( done == 2 )
					goto save_end;

				lab1 = get_str( "fn" );
				if ( strlen( lab1 ) == 0 )
					break;

				delete [ ] sim.conf_name;
				sim.conf_name = new char[ strlen( lab1 ) + 1 ];
				strcpy( sim.conf_name, lab1 );

				lab1 = get_str( "path" );
				delete [ ] sim.conf_path;
				sim.conf_path = new char[ strlen( lab1 ) + 1 ];
				strcpy( sim.conf_path, lab1 );

				delete [ ] sim.conf_file;
				sim.conf_file = new char[ strlen( sim.conf_path ) + strlen( sim.conf_name ) + 6 ];
				sprintf( sim.conf_file, "%s%s%s.lsd", sim.conf_path, strlen( sim.conf_path ) > 0 ? "/" : "", sim.conf_name );

				if ( strlen( lab1 ) > 0 )
					cmd( "cd $path" );

				redrawStruc = true;		// structure redraw because of titlebar
			}

			if ( ! save_xml_configuration_gui( ) )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s.lsd' cannot be saved\" -detail \"The model is NOT saved! Check if the drive or the file is set READ-ONLY, change file name or select a drive with write permission and try again.\"", sim.conf_name	);
			}
			else
				unsaved_change( false );					// signal no unsaved change

			save_end:
			Tcl_UnlinkVar( interp, "done" );
			cmd( "unset done" );

		break;


		// Unload the model
		case 20:

			if ( ! discard_change( ) )		// check for unsaved configuration changes
				break;

			unload_configuration_gui( true );

			r = sim.root;					// just an empty root exists

		break;


		// Edit Objects' numbers
		case 19:

			lsd::strcpyn( lab, r->label, MAX_BUFF_SIZE );

			choice = 0;
			sim.root->set_obj_number( );

			r = sim.root->search( lab );

		break;


		// Edit initial values for Objects currently selected or pointed by the browser (defined by tcl $vname)
		case 21:

			// check if current or pointed object and save current if needed
			lab1 = get_str( "useCurrObj" );
			if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
			{
				lab1 = get_str( "vname" );
				if ( lab1 == NULL || ! strcmp( lab1, "" ) )
					break;
				sscanf( lab1, "%99s", lab_old );

				n = sim.root->search( lab_old );// set pointer to $vname
				if ( n == NULL )
					break;
				cur2 = r;
				r = n;
			}
			else
				cur2 = NULL;

			for ( n = r; n->up != NULL; n = n->up );

			n->edit_data( r->label );

			redrawRoot = true;
			unsaved_change( true );			// signal unsaved change

			if ( cur2 != NULL )				// restore original current object
				r = cur2;

		break;


		// Simulation settings: sets seeds, number of steps, number of simulations
		case 22:

			// save previous values to allow canceling operation
			temp[ 1 ] = sim.last_run;
			temp[ 2 ] = sim.seed;
			temp[ 3 ] = sim.last_t;
			temp[ 4 ] = sim.deb_t;
			temp[ 5 ] = sim.stack_info;
			temp[ 6 ] = sim.prof_min_msecs;
			temp[ 7 ] = sim.prof_obs_only;
			temp[ 8 ] = sim.prof_aggr_time;
			temp[ 9 ] = sim.no_ptr_chk;
			temp[ 10 ] = sim.parallel_disable;

			Tcl_LinkVar( interp, "last_run", ( char * ) & sim.last_run, TCL_LINK_INT );
			Tcl_LinkVar( interp, "seed", ( char * ) & sim.seed, TCL_LINK_INT );
			Tcl_LinkVar( interp, "last_t", ( char * ) & sim.last_t, TCL_LINK_INT );
			Tcl_LinkVar( interp, "stack_info", ( char * ) & sim.stack_info, TCL_LINK_INT );
			Tcl_LinkVar( interp, "prof_min_msecs", ( char * ) & sim.prof_min_msecs, TCL_LINK_INT );
			Tcl_LinkVar( interp, "prof_obs_only", ( char * ) & sim.prof_obs_only, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "prof_aggr_time", ( char * ) & sim.prof_aggr_time, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "no_ptr_chk", ( char * ) & sim.no_ptr_chk, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "parallel_disable", ( char * ) & sim.parallel_disable, TCL_LINK_BOOLEAN );

			cmd( "set tw 30" );					// text label width
			cmd( "set T .simset" );
			cmd( "newtop $T \"Simulation Settings\" { set choice 2 }" );

			cmd( "ttk::frame $T.f" );

			cmd( "ttk::frame $T.f.c" );
			cmd( "ttk::label $T.f.c.l2 -width $tw -anchor e -text \"Simulation steps\"" );
			cmd( "ttk::spinbox $T.f.c.e2 -width 7 -from 1 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set last_t %%P; return 1 } { %%W delete 0 end; %%W insert 0 $last_t; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.c.e2 insert 0 $last_t" );
			cmd( "pack $T.f.c.l2 $T.f.c.e2 -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "ttk::frame $T.f.a" );
			cmd( "ttk::label $T.f.a.l -width $tw -anchor e -text \"Number of simulation runs\"" );
			cmd( "ttk::spinbox $T.f.a.e -width 7 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set last_run %%P; return 1 } { %%W delete 0 end; %%W insert 0 $last_run; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.a.e insert 0 $last_run" );
			cmd( "pack $T.f.a.l $T.f.a.e -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "ttk::frame $T.f.b" );
			cmd( "ttk::label $T.f.b.l1 -width $tw -anchor e -text \"Random numbers initial seed\"" );
			cmd( "ttk::spinbox $T.f.b.e1 -width 7 -from 1 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set seed %%P; return 1 } { %%W delete 0 end; %%W insert 0 $seed; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.b.e1 insert 0 $seed" );
			cmd( "pack $T.f.b.l1 $T.f.b.e1 -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "ttk::frame $T.f.d" );
			cmd( "ttk::label $T.f.d.l2 -width $tw -anchor e -text \"Start debugger at step (0:none)\"" );
			cmd( "ttk::spinbox $T.f.d.e2 -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set deb_t %%P; return 1 } { %%W delete 0 end; %%W insert 0 $deb_t; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.d.e2 insert 0 $deb_t" );
			cmd( "pack $T.f.d.l2 $T.f.d.e2 -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "ttk::frame $T.f.e" );
			cmd( "ttk::label $T.f.e.l2 -width $tw -anchor e -text \"Profile up to stack level (0:none)\"" );
			cmd( "ttk::spinbox $T.f.e.e2 -width 7 -from 0 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 && $n <= 99 } { set stack_info %%P; return 1 } { %%W delete 0 end; %%W insert 0 $stack_info; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.e.e2 insert 0 $stack_info" );
			cmd( "pack $T.f.e.l2 $T.f.e.e2 -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "ttk::frame $T.f.f" );
			cmd( "ttk::label $T.f.f.l2 -width $tw -anchor e -text \"Profile minimum time (0:all)\"" );
			cmd( "ttk::spinbox $T.f.f.e2 -width 7 -from 0 -to 99999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 0 } { set prof_min_msecs %%P; return 1 } { %%W delete 0 end; %%W insert 0 $prof_min_msecs; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "$T.f.f.e2 insert 0 $prof_min_msecs" );
			cmd( "pack $T.f.f.l2 $T.f.f.e2 -side left -anchor w -padx $_2 -pady $_2" );

			cmd( "pack $T.f.c $T.f.a $T.f.b $T.f.d $T.f.e $T.f.f -anchor w" );

			cmd( "ttk::frame $T.c" );

			cmd( "ttk::checkbutton $T.c.obs -text \"Profile observed variables only\" -variable prof_obs_only" );
			cmd( "ttk::checkbutton $T.c.aggr -text \"Show aggregated profiling times\" -variable prof_aggr_time" );
			cmd( "ttk::checkbutton $T.c.nchk -text \"Disable pointer checks\" -variable no_ptr_chk -state %s", lsd::no_pointer_check ? "disabled" : "normal" );

			cmd( "ttk::checkbutton $T.c.npar -text \"Disable parallel computation\" -variable parallel_disable" );
			if ( ! sim.root->search_parallel( ) || sim.max_threads < 2 )
				cmd( "$T.c.npar configure -state disabled" );

			cmd( "pack $T.c.obs $T.c.aggr $T.c.nchk $T.c.npar -anchor w" );

			cmd( "pack $T.f $T.c -padx $_5 -pady $_5" );

			cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menurun.html#simsetting } { set choice 2 }" );
			cmd( "bind $T.f.c.e2 <KeyPress-Return> { focus $T.f.a.e; $T.f.a.e selection range 0 end }" );
			cmd( "bind $T.f.a.e <KeyPress-Return> { focus $T.f.b.e1; $T.f.b.e1 selection range 0 end }" );
			cmd( "bind $T.f.b.e1 <KeyPress-Return> { focus $T.f.d.e2; $T.f.d.e2 selection range 0 end }" );
			cmd( "bind $T.f.d.e2 <KeyPress-Return> { focus $T.f.e.e2; $T.f.e.e2 selection range 0 end }" );
			cmd( "bind $T.f.e.e2 <KeyPress-Return> { focus $T.f.f.e2; $T.f.f.e2 selection range 0 end }" );
			cmd( "bind $T.f.f.e2 <KeyPress-Return>	{ focus $T.b.ok }" );

			cmd( "showtop $T" );
			cmd( "mousewarpto $T.b.ok 0" );
			cmd( "$T.f.c.e2 selection range 0 end" );
			cmd( "focus $T.f.c.e2" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set last_run [ .simset.f.a.e get ]" );
			cmd( "set seed [ .simset.f.b.e1 get ]" );
			cmd( "set last_t [ .simset.f.c.e2 get ]" );
			cmd( "set deb_t [ .simset.f.d.e2 get ]" );
			cmd( "set stack_info [ .simset.f.e.e2 get ]" );
			cmd( "set prof_min_msecs [ .simset.f.f.e2 get ]" );

			cmd( "destroytop $T" );

			if ( choice == 2 )	// Escape - revert previous values
			{
				sim.last_run = temp[ 1 ];
				sim.seed = ( unsigned ) temp[ 2 ];
				sim.last_t = temp[ 3 ];
				sim.deb_t = temp[ 4 ];
				sim.stack_info = temp[ 5 ];
				sim.prof_min_msecs = temp[ 6 ];
				sim.prof_obs_only = temp[ 7 ];
				sim.prof_aggr_time = temp[ 8 ];
				sim.no_ptr_chk = temp[ 9 ];
				sim.parallel_disable = temp[ 10 ];
			}
			else
				// signal unsaved change if anything to be saved
				if ( temp[ 1 ] != sim.last_run || ( unsigned ) temp[ 2 ] != sim.seed || temp[ 3 ] != sim.last_t || temp[ 4 ] != sim.deb_t || temp[ 5 ] != sim.stack_info || temp[ 6 ] != sim.prof_min_msecs || temp[ 7 ] != sim.prof_obs_only || temp[ 8 ] != sim.prof_aggr_time || temp[ 9 ] != sim.no_ptr_chk || temp[ 10 ] != sim.parallel_disable )
					unsaved_change( true );

			Tcl_UnlinkVar( interp, "last_run" );
			Tcl_UnlinkVar( interp, "seed" );
			Tcl_UnlinkVar( interp, "last_t" );
			Tcl_UnlinkVar( interp, "stack_info" );
			Tcl_UnlinkVar( interp, "prof_min_msecs" );
			Tcl_UnlinkVar( interp, "prof_obs_only" );
			Tcl_UnlinkVar( interp, "prof_aggr_time" );
			Tcl_UnlinkVar( interp, "no_ptr_chk" );
			Tcl_UnlinkVar( interp, "parallel_disable" );

		break;


		// assimilation settings: sets data assimilation realizations and covariance
		case 35:

			// check for data assimilation variables
			if ( sim.assim == NULL )
			{
				cmd( "ttk::messageBox -parent $T -type ok -icon error -title Error -message \"Data assimilation not configured\" -detail \"No variable is configured for data assimilation, changes here are only used if at least one variable is configured with data to be assimilated.\"" );
				break;
			}

			// save previous values to allow canceling operation
			temp[ 1 ] = sim.assim_disable;
			Tcl_LinkVar( interp, "assim_disable", ( char * ) & sim.assim_disable, TCL_LINK_INT );

			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			cmd( "set cov_file \"%s\"", sim.cov_file != NULL ? sim.cov_file : "" );
			cmd( "if { [ string first / $cov_file ] != -1 } { \
					set cov_file [ file nativename $cov_file ] \
				}" );

			cmd( "set T .assset" );
			cmd( "newtop $T \"Data Assimilation Settings\" { set choice 2 }" );

			cmd( "ttk::frame $T.csv" );
			cmd( "ttk::frame $T.csv.l" );
			cmd( "ttk::label $T.csv.l.l -text \"Covariance file (CSV only)\"" );
			cmd( "ttk::label $T.csv.l.pad -width 6" );
			cmd( "pack $T.csv.l.l $T.csv.l.pad -side left -padx $_5" );

			cmd( "ttk::frame $T.csv.file" );
			cmd( "ttk::entry $T.csv.file.e -width 40 -textvariable cov_file -justify center" );
			cmd( "ttk::button $T.csv.file.brw -text Browse -command { \
					set fn [ tk_getOpenFile -parent $T -title \"Select Data File\" -defaultextension \".csv\" -initialdir $path -filetypes { { {Comma-separated file} {.csv} } } ]; \
					if { [ string length $fn ] > 0 && ! [ fn_spaces $fn ] } { \
						set cov_file [ file normalize $fn ]; \
						if { [ string first [ file normalize $model_dir ] $cov_file ] == 0 } { \
							set cov_file [ string map [ list \"[ file normalize $model_dir ]/\" \"\" ] $cov_file ] \
						}; \
						if { [ string first / $cov_file ] != -1 } { \
							set cov_file [ file nativename $cov_file ] \
						} \
					} \
				}" );
			cmd( "pack $T.csv.file.e $T.csv.file.brw -side left -padx $_5" );

			cmd( "pack $T.csv.l $T.csv.file" );

			cmd( "ttk::frame $T.c" );
			cmd( "ttk::checkbutton $T.c.dis -text \"Disable data assimilation\" -variable assim_disable" );
			cmd( "pack $T.c.dis -anchor w" );

			cmd( "pack $T.csv $T.c -padx $_5 -pady $_10" );

			cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menurun.html#assimilation } { set choice 2 }" );

			cmd( "showtop $T centerW" );

			cmd( "mousewarpto $T.b.ok 0" );
			cmd( "$T.csv.file.e selection range 0 end" );
			cmd( "focus $T.csv.file.e" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			Tcl_UnlinkVar( interp, "assim_disable" );
			cmd( "destroytop $T" );

			if ( choice == 1 )
			{
				if ( strlen( get_str( "cov_file" ) ) > 0 )
				{
					cmd( "set cov_file [ string map {\\\\ /} $cov_file ]" );
					lab1 = get_str( "cov_file" );

					if ( sim.cov_file == NULL || strcmp( sim.cov_file, lab1 ) != 0 )
					{
						try
						{
							rapidcsv::Document csv;
							csv.Load( lab1, rapidcsv::LabelParams( 0, 0 ), rapidcsv::SeparatorParams( ',', true ), rapidcsv::ConverterParams( true, std::numeric_limits< long double >::quiet_NaN( ) ), rapidcsv::LineReaderParams( true, '#' ) );
						}
						catch ( ... )
						{
							cmd( "switch -- [ ttk::messageBox -parent . -type okcancel -default cancel -icon warning -title Warning -message \"Covariance file does not exist\" -detail \"If you want to add the covariance file later, press 'OK', or press 'Cancel' to abort changing assimilation settings.\" ] { ok { } cancel { set choice 2 } }" );
						}

						if ( choice != 2 )
						{
							delete [ ] sim.cov_file;
							sim.cov_file = new char [ strlen( lab1 ) + 1 ];
							strcpy( sim.cov_file, lab1 );
							unsaved_change( true );
						}
					}
				}

				if ( sim.assim_disable != temp[ 1 ] )
					unsaved_change( true );
			}
			else
				sim.assim_disable = temp[ 1 ];

		break;


		// Move browser to Object pointed on the graphical model structure map
		case 24:

			if ( res_g == NULL )
				break;

			n = sim.root->search( res_g );
			if ( n == NULL )
			{	// check if it is not a zero-instance object
				n = sim.blueprint->search( res_g );
				if ( n != NULL )
					cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Cannot show no-instance object\" -detail \"All instances of '%s' were deleted.\nSelect another object or reload your configuration and try again.\"", res_g );

				break;
			}

			if ( n != r )
			{
				redrawRoot = redrawStruc = true;		// force browser/structure redraw
				cmd( "set listfocus 1; set itemfocus 0" ); // point for first var in listbox
			}

			choice = 0;
			return n;


		// Enter the analysis of results module for Monte Carlo analysis
		case 12:
			// accept analysis after run only if MC data was just produced
			if ( sim.eff_t > 0 && sim.res_list.size( ) <= 1 )
			{
				cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid data for Monte Carlo analysis\" -detail \"Last simulation run did not produce adequate data to perform a Monte Carlo experiment analysis.\n\nPlease reload or unload your configuration and select the appropriate results files, or execute a multi-run configuration before using this option.\"" );
				break;
			}

			// check if MC results were not just created
			if ( sim.res_list.size( ) > 1 )
			{
				cmd( "set answer [ ttk::messageBox -parent . -type yesnocancel -icon question -default yes -title \"Results Available\" -message \"Use set of results last created?\" -detail \"A set of results files was previously created and can be used to perform the Monte Carlo experiment analysis.\n\nAny configuration or results not saved will be discarded.\n\nPress 'Yes' to confirm, 'No' to select a different set of files, or 'Cancel' to abort.\" ]; switch -- $answer { yes { set choice 1 } no { set choice 0 } cancel { set choice 2 } }" );

				if ( choice == 2 )
					break;

				if ( choice == 0 )
					sim.res_list.clear( );
			}
			else
				if ( ! discard_change( ) )		// check for unsaved configuration changes
					break;

			// remove existing results from memory before proceeding
			if ( ! open_configuration( r, true ) )
			{
				unload_configuration_gui( true );
				r = sim.root;
			}

			analysis( true );

		break;

		// Enter the analysis of results module
		case 26:

			analysis( false );

		break;


		// Change Equation File from which to take the code to show
		case 28:

			if ( ! sim.conf_ok )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create one before trying to change the equation file.\"" );
				break;
			}

			cmd( "set res %s", eq_file );

			cmd( "set res1 [ file tail [ tk_getOpenFile -parent . -title \"Select New Equation File\" -initialfile \"$res\" -initialdir \"%s\" -filetypes { { {LSD equation files} {.cpp} } { {All files} {*} } } ] ]", lsd::model_path );
			cmd( "if [ fn_spaces \"$res1\" . ] { set res1 \"\" } { set res1 [ file tail $res1 ] }" );

			lab1 = get_str( "res1" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%999s", eq_file );

			unsaved_change( true );		// signal unsaved change

		break;


		// Shortcut to show equation window
		case 29:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			show_eq( lab_old, "." );

		break;


		// Show variables to be saved
		case 39:

			i = 0;
			sim.root->count_save( & i );
			if ( i == 0 )
				plog( " \nNo variable or parameter saved." );
			else
			{
				plog( "\n\nVariables and parameters saved (%d):\n", i );
				sim.root->show_save( );
			}

		break;


		// Show variables to be observed
		case 42:

			plog( "\n\nVariables and parameters containing results:\n" );
			elem_count = 0;
			sim.root->show_observe( );
			if ( elem_count == 0 )
				plog( "(none)\n" );

		break;


		// Show variables to be initialized
		case 49:

			plog( "\n\nVariables and parameters relevant to initialize:\n" );
			elem_count = 0;
			sim.root->show_initial( );
			if ( elem_count == 0 )
				plog( "(none)\n" );

		break;


		// Show variables to be plot
		case 84:

			plog( "\n\nVariables and parameters to plot in run time:\n" );
			elem_count = 0;
			sim.root->show_plot( );
			if ( elem_count == 0 )
				plog( "(none)\n" );

		break;


		// Show elements to debug and watch
		case 85:

			plog( "\n\nVariables and parameters to debug and watch:\n" );
			elem_count = 0;
			sim.root->show_debug( );
			if ( elem_count == 0 )
				plog( "(none)\n" );

		break;


		// Show variables to parallelize
		case 86:

			plog( "\n\nMulti-object variables to run in parallel:\n" );
			elem_count = 0;
			sim.root->show_parallel( );
			if ( elem_count == 0 )
				plog( "(none)\n" );

		break;


		// Show variables with special updating
		case 97:

			plog( "\n\nVariables with special updating scheme:\n" );
			elem_count = 0;
			sim.root->show_special_updat( );
			if ( elem_count == 0 )
				plog( "(none)\n" );

		break;


		// elements/objects in configuration but unused in equation file
		case 56:

			// read the lists of variables/functions, parameters and objects in model program
			// from disk, if needed, or just update the missing elements lists
			cmd( "if { [ llength $unusVar ] == 0 || [ llength $unusFun ] == 0 || [ llength $unusPar ] == 0 || [ llength $unusObj ] == 0 } { read_elem_file %s } { upd_unus_elem }", lsd::model_path );

			plog( "\n\nElements/objects apparently unused/missing in equation file(s):\n" );

			cmd( "foreach var $unusVar { plog \"Variable :\t\"; plog \"$var\n\" highlight }" );
			cmd( "foreach fun $unusFun { plog \"Function :\t\"; plog \"$fun\n\" highlight }" );
			cmd( "foreach par $unusPar { plog \"Parameter:\t\"; plog \"$par\n\" highlight }" );
			cmd( "foreach obj $unusObj { plog \"Object	 :\t\"; plog \"$obj\n\" highlight }" );

			cmd( "set res [ expr { [ llength $unusVar ] + [ llength $unusFun ] + [ llength $unusPar ] + [ llength $unusObj ] } ]" );
			if ( get_int( "res" ) == 0 )
				plog( "(none)\n" );

		break;


		// Remove all the save flags
		case 30:

			cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Remove save flags?\" -detail \"Confirm the removal of all saving information. No data will be saved.\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );

			if ( choice == 1 )
			{
				sim.root->clean_save( );
				unsaved_change( true );				// signal unsaved change
				redrawRoot = redrawStruc = true;	// force browser/structure redraw
			}

		break;


		// Remove all the plot flags
		case 31:

			cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Remove plot flags?\" -detail \"Confirm the removal of all run-time plot information. No data will be plotted during run time.\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );

			if ( choice == 1 )
			{
				sim.root->clean_plot( );
				unsaved_change( true );				// signal unsaved change
				redrawRoot = redrawStruc = true;	// force browser/structure redraw
			}

		break;


		// Remove all the debug and watch flags
		case 27:

			cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Remove debug and watch flags?\" -detail \"Confirm the removal of all debugging and watching information. Debugger will not stop in any variable update, access or write.\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );

			if ( choice == 1 )
			{
				sim.root->clean_debug( );
				unsaved_change( true );				// signal unsaved change
				redrawRoot = redrawStruc = true;	// force browser/structure redraw
			}

		break;


		// Remove all the parallel flags
		case 87:

			cmd( "set answer [ ttk::messageBox -parent . -type yesno -default yes -icon question -title Confirmation -message \"Remove parallel flags?\" -detail \"Confirm the removal of all parallel processing information. No parallelization will be performed.\" ]; switch $answer { yes { set choice 1 } no { set choice 2 } }" );

			if ( choice == 1 )
			{
				sim.root->clean_parallel( );
				unsaved_change( true );				// signal unsaved change
				redrawRoot = redrawStruc = true;	// force browser/structure redraw
			}

		break;


		// Changes the number of instances of only the Object type shown
		// in the browser or the pointed object (defined in tcl $vname)
		case 33:

			// check if current or pointed object and save current if needed
			lab1 = get_str( "useCurrObj" );
			if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
			{
				lab1 = get_str( "vname" );
				if ( lab1 == NULL || ! strcmp( lab1, "" ) )
					break;
				sscanf( lab1, "%99s", lab_old );

				n = sim.root->search( lab_old );	// set pointer to $vname
				if ( n == NULL )
					break;
				cur2 = r;
				r = n;
			}
			else
				cur2 = NULL;

			if ( r->up == NULL )
			{
				cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Cannot create instances of 'Root' object\" -detail \"Consider, if necessary, to add a new object here and moving all descendants of 'Root' to it. this new object can, then, be multiplied in many instances.\"" );
				goto endinst;
			}

			r->next_count( r, & num );
			cmd( "set num %d", num );
			cmd( "set cfrom 1" );

			cmd( "set T .numinst" );
			cmd( "newtop $T \"Number of Instances\" { set choice 2 }" );

			cmd( "ttk::frame $T.l" );

			cmd( "ttk::label $T.l.l1 -text \"Object:\"" );
			cmd( "ttk::label $T.l.l2 -text \"%s\" -style hl.TLabel", r->label );
			cmd( "pack $T.l.l1 $T.l.l2 -side left" );

			cmd( "ttk::frame $T.e" );

			cmd( "ttk::frame $T.e.e" );
			cmd( "ttk::label $T.e.e.l -text \"Number of instances\"" );
			cmd( "ttk::spinbox $T.e.e.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set num %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( "pack $T.e.e.l $T.e.e.e -side left -padx $_2" );

			cmd( "ttk::label $T.e.l -text \"(all groups of this object will be affected)\"" );
			cmd( "pack $T.e.e $T.e.l" );

			cmd( "ttk::frame $T.cp" );
			cmd( "ttk::label $T.cp.l -text \"Copy from instance\"" );
			cmd( "ttk::spinbox $T.cp.e -width 5 -from 1 -to %d -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 && $n <= %d } { set cfrom %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cfrom; return 0 } } -invalidcommand { bell } -justify center", num, num );
			cmd( "ttk::button $T.cp.compute -width $butWid -text Compute -command { set choice 3; .numinst.cp.e selection range 0 end; focus .numinst.cp.e }" );
			cmd( "pack $T.cp.l $T.cp.e $T.cp.compute -side left -padx $_2" );

			cmd( "pack $T.l $T.e $T.cp -pady $_5 -padx $_5" );

			cmd( "okhelpcancel $T b { set choice 1 } { LsdHelp menudata_objn.html#this } { set choice 2 }" );
			cmd( "bind $T.e.e.e <Return> { set choice 1 }" );

			cmd( "showtop $T" );
			cmd( "mousewarpto $T.b.ok" );

			i = 1;

			objec_num:

			cmd( "write_any $T.e.e.e $num" );
			cmd( "write_any $T.cp.e $cfrom" );

			if ( i == 1 )
			{
				cmd( "$T.e.e.e selection range 0 end" );
				cmd( "focus $T.e.e.e" );
				i = 0;
			}

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set num [ $T.e.e.e get ]" );
			cmd( "set cfrom [ $T.cp.e get ]" );

			if ( choice == 3 )
			{
				k = r->compute_copyfrom( ".numinst" );
				if ( k > 0 )
					cmd( "set cfrom %d", k );

				goto objec_num;
			}

			cmd( "destroytop $T" );

			if ( choice == 2 )
				goto endinst;

			k = get_int( "cfrom" );
			num = get_int( "num" );
			for ( i = 0, cur = r->up; cur != NULL; ++i, cur = cur->up );

			change_obj_number( r, num, i, NULL, k );

			unsaved_change( true );				// signal unsaved change
			redrawRoot = redrawStruc = true;	// force browser/structure redraw

			endinst:

			if ( cur2 != NULL )					// restore original current object
				r = cur2;

		break;


		// Browse through the model instance by instance
		case 34:

			// check if current or pointed object and save current if needed
			lab1 = get_str( "useCurrObj" );
			if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
			{
				lab1 = get_str( "vname" );
				if ( lab1 == NULL || ! strcmp( lab1, "" ) )
					break;
				sscanf( lab1, "%99s", lab_old );

				n = sim.root->search( lab_old );// set pointer to $vname
				if ( n == NULL )
					break;
				cur2 = r;
				r = n;
			}
			else
				cur2 = NULL;

			r->debugger( NULL, NULL, &fake );

			if ( cur2 != NULL )					// restore original current object
				r = cur2;

		break;


		// Create model report
		case 36:

			sim.root->report( );

		break;


		// See model report
		case 44:

			show_report( "." );

		break;


		// Save result
		case 37:

			choice = 0;
			if ( sim.eff_t == 0 )
			{
				cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Simulation not run, nothing to save\" -detail \"Select menu option Run>Run before using this option.\"" );
				break;
			}

			Tcl_LinkVar( interp, "docsv", ( char * ) & sim.docsv, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "dozip", ( char * ) & sim.dozip, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "saveConf", ( char * ) & saveConf, TCL_LINK_BOOLEAN );

			time_t rawtime;
			time( &rawtime );
			struct tm *timeinfo;
			char ftime[80];
			timeinfo = localtime( &rawtime );
			strftime ( ftime, 80, "%Y%m%d-%H%M%S", timeinfo );

			cmd( "set lab \"%s_%s\"", strlen( sim.conf_name ) > 0 ? sim.conf_name : "results", ftime );

			// choose a name
			cmd( "newtop .n \"Save Results\" { set choice 2 }" );

			cmd( "ttk::frame .n.n" );
			cmd( "ttk::label .n.n.l -text \"Base name for file(s)\"" );
			cmd( "ttk::entry .n.n.e -width 30 -textvariable lab -justify center" );
			cmd( "pack .n.n.l .n.n.e" );

			cmd( "ttk::frame .n.do" );
			cmd( "ttk::checkbutton .n.do.zip -text \"Generate zipped results file\" -variable dozip" );
			cmd( "ttk::checkbutton .n.do.csv -text \"Comma-separated text format (.csv)\" -variable docsv" );
			cmd( "ttk::checkbutton .n.do.conf -text \"Save associated configuration\" -variable saveConf" );
			cmd( "pack .n.do.zip .n.do.csv .n.do.conf -anchor w" );

			cmd( "pack .n.n .n.do -padx $_5 -pady $_5" );

			cmd( "okcancel .n b { set choice 1 } { set choice 2 }" );
			cmd( "bind .n <KeyPress-Return> { set choice 1 }" );

			cmd( "showtop .n" );
			cmd( "mousewarpto .n.b.ok 0" );
			cmd( ".n.n.e selection range 0 end" );
			cmd( "focus .n.n.e" );

			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "if { [ string length lab ] == 0 } { set choice 2 }" );

			cmd( "destroytop .n" );

			Tcl_UnlinkVar( interp, "docsv" );
			Tcl_UnlinkVar( interp, "dozip" );
			Tcl_UnlinkVar( interp, "saveConf" );

			if ( choice == 2 )
				break;

			cmd( "focustop .log" );

			get_str( "lab", ch1, MAX_ELEM_LENGTH );

			if ( saveConf && strlen( sim.conf_name ) > 0 )
			{
				if ( strlen( sim.conf_path ) == 0 )
				{
					cmd( "file copy -force %s.lsd %s.lsd", sim.conf_name, ch1 );
					plog( "\nSaved configuration to file %s.lsd", ch1 );
				}
				else
				{
					cmd( "file copy -force %s/%s.lsd %s/%s.lsd", sim.conf_path, sim.conf_name, sim.conf_path, ch1 );
					plog( "\nSaved configuration to file %s/%s.lsd", sim.conf_path, ch1 );
				}
			}

			if ( strlen( sim.conf_path ) == 0 )
				snprintf( out_file, MAX_PATH_LENGTH, "%s.%s", ch1, sim.docsv ? "csv" : "res" );
			else
				snprintf( out_file, MAX_PATH_LENGTH, "%s/%s.%s", sim.conf_path, ch1, sim.docsv ? "csv" : "res" );

			if ( sim.dozip )
				lsd::strcatn( out_file, ".gz", MAX_PATH_LENGTH );

			plog( "\nSaving results to file %s... ", out_file );

			rf = new lsd::result( out_file, "wt", & sim, sim.dozip, sim.docsv );// create results file object
			rf->title( sim.root, 1 );					// write header
			rf->data( sim.root, 0, sim.eff_t );			// write all data
			delete rf;									// close file and delete object

			plog( "Done\n" );

			unsavedData = false;						// no unsaved simulation results

		break;


		// Help on lsd
		case 41:

			cmd( "LsdHelp LSD_quickhelp.html" );

		break;


		// Create automatically the elements descriptions
		case 43:

			if ( ! sim.conf_ok )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create one before trying to update descriptions.\"" );
				break;
			}

			cmd( "set answer [ttk::messageBox -parent . -message \"Replace existing descriptions?\" -detail \"Automatic data will replace any previous entered descriptions. Proceed?\" -type yesno -title Confirmation -icon question -default yes]" );
			cmd( "if { [ string compare $answer yes ] == 0 } { set choice 0 } { set choice 1 }" );

			if ( choice == 1 )
				break;

			cmd( "set x 1" );

			cmd( "newtop .warn \"Generate Descriptions\" { set choice 2 }" );

			cmd( "ttk::frame .warn.m" );
			cmd( "ttk::label .warn.m.l -text \"Elements to update\"" );

			cmd( "ttk::frame .warn.m.o -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
			cmd( "ttk::radiobutton .warn.m.o.var -text \"Only variables\" -variable x -value 1" );
			cmd( "ttk::radiobutton .warn.m.o.all -text \"All elements\" -variable x -value 2" );
			cmd( "pack .warn.m.o.var .warn.m.o.all -anchor w" );

			cmd( "pack .warn.m.l .warn.m.o" );

			cmd( "pack .warn.m -padx $_5 -pady $_5" );

			cmd( "okhelpcancel .warn b { set choice 1 } { LsdHelp menumodel.html#auto_docu } { set choice 2 }" );

			cmd( "showtop .warn" );
			cmd( "mousewarpto .warn.b.ok" );

			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "destroytop .warn" );

			if ( choice == 2 )
				break;

			cmd( "set choice $x" );
			if ( choice == 1 )
				auto_document( NULL, "" );
			else
				auto_document( NULL, "ALL" );

			unsaved_change( true );		// signal unsaved change

		break;


		// Show the vars./pars. vname is using
		case 46:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab );

			scan_using_lab( lab, "." );

		break;


		// Show the equations where vname is used
		case 47:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab );

			scan_used_lab( lab, "." );

		break;


		// find an object or element of the model
		case 50:

			if ( ! sim.conf_ok )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create one before trying to find elements.\"" );
				break;
			}

			cmd( "set bidi \"\"" );
			cmd( "set a [ lsort -dictionary [ concat Root $modObj $modElem ] ]" );

			cmd( "newtop .srch \"Find\" { set choice 2 }" );

			cmd( "ttk::frame .srch.i" );
			cmd( "ttk::label .srch.i.l -text \"Name\"" );
			cmd( "ttk::combobox .srch.i.e -width 20 -textvariable bidi -justify center -values $a" );
			cmd( "pack .srch.i.l .srch.i.e" );

			cmd( "ttk::label .srch.o -justify center -text \"(type the initial letters of the\nname, LSD will complete it)\"" );
			cmd( "pack .srch.i .srch.o -padx $_5 -pady $_5" );
			cmd( "pack .srch.i" );

			cmd( "okcancel .srch b { set choice 1 } { set choice 2 }" );

			cmd( "bind .srch.i.e <KeyPress-Return> { set choice 1; break }" );
			cmd( "bind .srch.i.e <KeyRelease> { \
					if { %%N < 256 } { \
						set b [ .srch.i.e index insert ]; \
						set s [ .srch.i.e get ]; \
						set f [ lsearch -glob $a $s* ]; \
						if { $f !=-1 } { \
							set d [ lindex $a $f ]; \
							.srch.i.e delete 0 end; \
							.srch.i.e insert 0 $d; \
							.srch.i.e index $b; \
							.srch.i.e selection range $b end \
						} \
					} \
				}" );

			cmd( "showtop .srch" );
			cmd( "focus .srch.i.e" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "destroytop .srch" );

			if ( choice == 2 )
				break;


		// Arrive here from the list of vars used (keep together with case 50!)
		case 55:

			cur = NULL;
			cv = NULL;
			if ( eval_bool( "\"$bidi\" eq \"Root\"" ) )
				cur = sim.root;
			else
				if ( eval_bool( "\"$bidi\" in $modObj" ) )
					cur = r->search( get_str( "bidi" ), false, false );
				else
					cv = r->search_var( r, get_str( "bidi" ), true );

			if ( cur != NULL )
			{
				cmd( "set listfocus 2; set itemfocus 0" );
				redrawRoot = redrawStruc = true;			// request browser redraw
				choice = 0;
				return cur;
			}
			else
				if ( cv != NULL )
				{
					for ( i = 0, cv1 = cv->up->v; cv1 != cv && cv1 != NULL;
						  cv1 = cv1->next, ++i );

					cmd( "set listfocus 1; set itemfocus %d", i );
					redrawRoot = redrawStruc = true;			// request browser redraw
					choice = 0;
					return cv->up;
				}
				else
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Name not found\" -detail \"Check the spelling of the name.\"" );

		break;


		// Restore configuration's equations in a new equation file
		case 52:
			/*
			Used to re-generate the equations used for the current configuration file
			*/

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to offload an equation file.\"" );
				break;
			}

			if ( ! strcmp( eq_txt, sim.conf_eq_txt ) )
			{
				cmd( "ttk::messageBox -parent . -title \"Offload Equations\" -icon info -message \"Nothing to do\" -detail \"There are no equations to be offloaded differing from the current equation file.\" -type ok" );
				break;
			}

			cmd( "set res1 fun_%s.cpp", sim.conf_name );
			cmd( "set bah [ tk_getSaveFile -parent . -title \"Save Equation File\" -defaultextension \".cpp\" -initialfile $res1 -initialdir \"%s\" -filetypes { { {LSD equation files} {.cpp} } { {All files} {*} } } ]", lsd::model_path );

			cmd( "if { [ string length $bah ] > 0 } { set choice 1; set res1 [ file tail $bah ] } { set choice 0 }" );
			if ( choice == 0 )
			  break;

			get_str( "res1", lab, MAX_PATH_LENGTH );
			if ( strlen( lab ) == 0 )
				break;

			if ( ( f = fopen( lab, "wb" ) ) != NULL )
			{
				fprintf( f, "%s", sim.conf_eq_txt );
				fclose( f );
				cmd( "ttk::messageBox -parent . -title \"Offload Equations\" -icon info -message \"Equation file '$res1' created\" -detail \"You need to create a new LSD model to use these equations, replacing the name of the equation file in LMM with the command 'Model Compilation Options' (menu Model).\" -type ok" );
			}
			else
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '$res1' cannot be saved\" -detail \"Check if the file already exists and is set READ-ONLY, or try to save to a different location.\"" );

		break;


		// Compare equation files
		case 53:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to compare equation files.\"" );
				break;
			}

			if ( strlen( sim.conf_eq_txt ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon Warning -title Warning -message \"No equations loaded\" -detail \"Please upload an equation file before trying to compare equation files.\"" );
				break;
			}

			cmd( "set tmpdir [ temp_dir ]" );
			lab1 = get_str( "tmpdir" );
			snprintf( lab_old, 2 * MAX_PATH_LENGTH, "%s/orig-eq_%s.tmp", lab1, sim.conf_name);

			if ( ( f = fopen( lab_old, "wb" ) ) != NULL )
			{
				fprintf( f, "%s", sim.conf_eq_txt );
				fclose( f );

				get_eqfile_name( lab, MAX_PATH_LENGTH );
				cmd( "open_diff %s %s %s %s.lsd", lab, lab_old, eq_file, sim.conf_name  );
			}
			else
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s' cannot be saved\" -detail \"Check if the file already exists and is set READ-ONLY.\"", lab_old );

		break;


		// Compare configuration files
		case 82:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 || strlen( sim.conf_file ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to compare configuration files.\"" );
				break;
			}

			// make sure there is a path set
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			cmd( "set res1 [ tk_getOpenFile -parent . -title \"Select Configuration File to Compare to\" -initialdir $path -filetypes { { {LSD configuration files} {.lsd} } } ]" );
			cmd( "set res2 [ file tail $res1 ]" );
			cmd( "if [ fn_spaces \"$res1\" . ] { set res1 \"\"; set res2 \"\" }" );

			lab1 = get_str( "res1" );
			lab2 = get_str( "res2" );
			if ( lab1 == NULL || lab2 == NULL || strlen ( lab1 ) == 0 || strlen ( lab2 ) == 0 )
				break;

			f = fopen( lab1, "r" );
			if ( f == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Cannot open file\" -detail \"Error opening file '%s'.\"", lab2 );
				break;
			}
			fclose( f );

			cmd( "set tmpdir [ temp_dir ]" );
			cmd( "file copy -force -- $res1 \"$tmpdir/ext-cfg.tmp\"" );
			cmd( "file copy -force -- %s \"$tmpdir/int-cfg.tmp\"", sim.conf_file );
			cmd( "open_diff \"$tmpdir/ext-cfg.tmp\" \"$tmpdir/int-cfg.tmp\" %s %s.lsd", lab2, sim.conf_name );

		break;


		// Toggle ignore eq. file controls
		case 54:

			cmd( "set choice $ignore_eq_file" );
			ignore_eq_file = choice;
			plog( "\n%s equation file\n", ignore_eq_file ? "Ignoring" : "Not ignoring" );

		break;


		// Generate Latex code
		case 57:
		case 92:

			table = ( choice == 57 ) ? true : false;

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create LaTex code.\"" );
				break;
			}

			snprintf( out_file, MAX_PATH_LENGTH, "%s%s%s_%s.tex", strlen( sim.conf_path ) > 0 ? sim.conf_path : "", strlen( sim.conf_path ) > 0 ? "/" : "", table ? "table" : "href", sim.conf_name );
			cmd( "set choice [ file exists %s ]", out_file );
			if ( choice == 1 )
			{
				cmd( "set answer [ ttk::messageBox -parent . -message \"File '%s' already exists\" -detail \"Please confirm overwriting it.\" -type okcancel -title Warning -icon warning -default ok ]", out_file );
				cmd( "if [ string equal $answer ok ] { set choice 0 } { set choice 1 }" );
				if ( choice == 1 )
					break;
			}

			if ( ( f = fopen( out_file, "wt" ) ) == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s' cannot be created\" -detail \"Check if the file already exists and is set READ-ONLY, or try to save to a different location.\"", out_file );
				break;
			}

			stop = false;
			cmd( "progressbox .ptex \"Creating LaTex\" \"LaTex code generation steps\" \"Step\" 6 { set stop true }" );

			sim.root->tex_report_head( f, table );
			cmd( "prgboxupdate .ptex 1" );

			if ( stop )
				goto end_latex;

			sim.root->tex_report_struct( f, table );
			cmd( "prgboxupdate .ptex 2" );

			if ( stop )
				goto end_latex;

			sim.root->tex_report_observe( f, table );
			cmd( "prgboxupdate .ptex 3" );

			if ( stop )
				goto end_latex;

			sim.root->tex_report_init( f, table );
			cmd( "prgboxupdate .ptex 4" );

			if ( stop )
				goto end_latex;

			sim.root->tex_report_initall( f, table );
			cmd( "prgboxupdate .ptex 5" );

			if ( stop )
				goto end_latex;

			sim.root->tex_report_end( f );
			cmd( "prgboxupdate .ptex 6" );

			end_latex:

			cmd( "destroytop .ptex" );

			fclose( f );

			if ( stop )
				remove( out_file );
			else
				plog( "\nLaTex code saved in file: %s\n", out_file );

		break;


		// Move variable up in the list box
		case 58:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			r->shift_var( -1, lab_old );

			unsaved_change( true );		// signal unsaved change
			redrawRoot = true;			// request browser redraw

		break;


		// Move variable down in the list box
		case 59:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			r->shift_var( 1, lab_old );

			unsaved_change( true );		// signal unsaved change
			redrawRoot = true;			// request browser redraw

		break;


		// Move object up in the list box
		case 60:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			r->shift_desc( -1, lab_old );

			unsaved_change( true );		// signal unsaved change
			redrawRoot = redrawStruc = true;	// request browser redraw

		break;


		// Move object down in the list box
		case 61:

			lab1 = get_str( "vname" );
			if ( lab1 == NULL || ! strcmp( lab1, "" ) )
				break;
			sscanf( lab1, "%99s", lab_old );

			r->shift_desc( 1, lab_old );

			unsaved_change( true );		// signal unsaved change
			redrawRoot = redrawStruc = true;	// request browser redraw

		break;


		// Sort current list box on the selected order
		case 94:
			cmd( "set choice $listfocus" );
			i = choice;
			cmd( "set choice $sort_order" );

			if ( r->sort_listbox( i, choice ) )
			{
				unsaved_change( true );		// signal unsaved change
				redrawRoot = true;			// request browser redraw
			}

		break;


		// Create parallel sensitivity analysis configuration
		case 62:

			if ( sim.sens != NULL )
			{
				if ( ! discard_change( false ) )	// unsaved configuration?
					break;

				varSA = sim.num_sensitivity_variables( );// number of variables to test
				plog( "\nNumber of elements for sensitivity analysis: %d", varSA );
				ptsSa = sim.num_sensitivity_points( );// total number of points in sensitivity space
				plog( "\nSensitivity analysis space size: %ld", ptsSa );

				// Prevent running into too big sensitivity spaces (high computation times)
				if ( ptsSa > std::max( 10, MAX_SENS_POINTS / 10 ) )
					// ask user before proceeding
					if ( sensitivity_too_large( ptsSa ) )
						break;

				for ( i = 1, cs = sim.sens; cs!=NULL; cs = cs->next )
					i *= cs->num_val;
				cur = sim.root->b->head;
				sim.root->add_n_objects2( cur->label, i - 1, cur );

				plog( "\nUpdating configuration... " );
				cmd( "focustop .log" );

				cur->sensitivity_parallel( sim.sens );

				plog( "Done\n" );

				unsaved_change( true );				// signal unsaved change
				redrawRoot = redrawStruc = true;	// force browser/structure redraw

				cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"Structure changed\" -detail \"LSD has changed your model structure, replicating the entire model for each sensitivity configuration. If you want to preserve your original configuration file, save your new configuration using a different name BEFORE running the model.\"" );
			}
			else
				sensitivity_undefined( );			// throw error

		break;


		// Create batch sensitivity analysis configuration
		case 63:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
				break;
			}

			if ( sim.sens != NULL )
			{
				if ( ! discard_change( false ) )	// unsaved configuration?
					break;

				varSA = sim.num_sensitivity_variables( );// number of variables to test
				plog( "\nNumber of elements for sensitivity analysis: %d", varSA );
				ptsSa = sim.num_sensitivity_points( );// total number of points in sensitivity space
				plog( "\nSensitivity analysis space size: %ld", ptsSa );

				// Prevent running into too big sensitivity spaces (high computation times)
				if ( ptsSa > MAX_SENS_POINTS )
					// ask user before proceeding
					if ( sensitivity_too_large( ptsSa ) )
						break;

				// detect the need of a new save path and create it if required
				if ( need_res_dir( sim.conf_path, sim.conf_name, sens_path, MAX_PATH_LENGTH ) )
					create_res_dir( sens_path );

				// ask to clean existing files before proceeding if required
				if ( check_res_dir( sens_path, sim.conf_name ) && sensitivity_clean_dir( sens_path ) )
					clean_res_dir( sens_path, sim.conf_name );

				// save the current object & cursor position for quick reload
				r->save_pos( );
				findexSens = 1;

				// create a design of experiment (DoE) for the sensitivity data
				cmd( "focustop .log" );

				stop = false;
				cmd( "progressbox .psa \"Creating DoE\" \"Creating configuration files\" \"File\"  %d { set stop true }", ptsSa );

				sensitivity_sequential( &findexSens, sim.sens, 1.0, sens_path );

				cmd( "destroytop .psa" );

				plog( "\nSensitivity analysis configurations produced: %d", findexSens - 1 );

				// if succeeded, explain user how to proceed
				if ( ! stop )
					sensitivity_created( sens_path, lsd::clean_file( sim.conf_name ), 1 );
				else
					findexSens = 0;					// don't consider for appending

				// now reload the previously existing configuration
				if ( ! load_prev_configuration( ) )
				{
					choice = 0;
					return sim.root;
				}

				// restore pointed object and variable
				r = r->restore_pos( );
			}
			else
				sensitivity_undefined( );			// throw error

		break;


		// Create Monte Carlo (MC) random sensitivity analysis sampling configuration (over user selected point values)
		case 71:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
				break;
			}

			if ( sim.sens != NULL )
			{
				if ( ! discard_change( false ) )	// unsaved configuration?
					break;

				varSA = sim.num_sensitivity_variables( );// number of variables to test
				plog( "\nNumber of elements for sensitivity analysis: %d", varSA );
				maxMC = sim.num_sensitivity_points( );// total number of points in sensitivity space
				plog( "\nSensitivity analysis space size: %ld", maxMC );

				// get the number of Monte Carlo samples to produce
				fracMC = 10;
				Tcl_LinkVar( interp, "fracMC", ( char * ) & fracMC, TCL_LINK_DOUBLE );

				// detect the need of a new save path
				subDir = need_res_dir( sim.conf_path, sim.conf_name, sens_path, MAX_PATH_LENGTH );

				cmd( "newtop .s \"MC Point Sampling\" { set choice 2 }" );

				cmd( "ttk::frame .s.p" );
				cmd( "ttk::label .s.p.l -text \"Output path\"" );
				cmd( "ttk::label .s.p.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", sens_path );
				cmd( "pack .s.p.l .s.p.w" );

				cmd( "ttk::frame .s.i" );
				cmd( "ttk::label .s.i.l -justify center -text \"Monte Carlo sample size as\n%% of sensitivity space size\n(0 to 100)\"" );
				cmd( "ttk::entry .s.i.e -width 5 -validate focusout -validatecommand { set n %%P; if { [ string is double -strict $n ] && $n > 0 && $n <= 100 } { set fracMC %%P; return 1 } { %%W delete 0 end; %%W insert 0 $fracMC; return 0 } } -invalidcommand { bell } -justify center" );
				cmd( ".s.i.e insert 0 $fracMC" );
				cmd( "pack .s.i.l .s.i.e" );

				cmd( "ttk::label .s.w -text \"(large samples are not recommended)\"" );

				cmd( "pack .s.p .s.i .s.w -padx $_5 -pady $_5" );

				cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menudata_sa.html#mcpoint } { set choice 2 }" );

				cmd( "showtop .s" );
				cmd( "mousewarpto .s.b.ok 0" );
				cmd( ".s.i.e selection range 0 end" );
				cmd( "focus .s.i.e" );

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "set fracMC [ .s.i.e get ]" );
				cmd( "destroytop .s" );
				Tcl_UnlinkVar( interp, "fracMC" );

				if ( choice == 2 )
					break;

				// Check if number is valid
				fracMC /= 100.0;
				if ( ( fracMC * maxMC ) < 1 || fracMC > 1.0 )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid sample size\" -detail \"Invalid Monte Carlo sample size to perform the sensitivity analysis. Select a number between 0%% and 100%% that produces at least one sample (in average).\"" );
					choice = 0;
					break;
				}

				// Prevent running into too big sensitivity space samples (high computation times)
				if ( ( fracMC * maxMC ) > MAX_SENS_POINTS )
					// ask user before proceeding
					if ( sensitivity_too_large( ( long ) ( fracMC * maxMC ) ) )
						break;

				// create a new save path if required
				if ( subDir )
					create_res_dir( sens_path );

				// ask to clean existing files before proceeding if required
				if ( check_res_dir( sens_path, sim.conf_name ) && sensitivity_clean_dir( sens_path ) )
					clean_res_dir( sens_path, sim.conf_name );

				// save the current object & cursor position for quick reload
				r->save_pos( );

				plog( "\nTarget sensitivity analysis sample size: %ld (%.1f%%)", ( long ) ( fracMC * maxMC ), 100 * fracMC );
				findexSens = 1;

				// create a design of experiment (DoE) for the sensitivity data
				cmd( "focustop .log" );

				stop = false;
				cmd( "progressbox .psa \"Creating DoE\" \"Creating configuration files\" \"File\" %ld { set stop true }", ( long ) ( fracMC * maxMC ) );

				sim.init_random( sim.seed );		// reset random number generator
				sensitivity_sequential( &findexSens, sim.sens, fracMC, sens_path );

				cmd( "destroytop .psa" );

				plog( "\nSensitivity analysis configurations produced: %d", findexSens - 1 );

				// if succeeded, explain user how to proceed
				if ( ! stop )
					sensitivity_created( sens_path, lsd::clean_file( sim.conf_name ), 1 );
				else
					findexSens = 0;					// don't consider for appending

				// now reload the previously existing configuration
				if ( ! load_prev_configuration( ) )
				{
					choice = 0;
					return sim.root;
				}

				// restore pointed object and variable
				r = r->restore_pos( );
			}
			else
				sensitivity_undefined( );			// throw error

		break;


		// Create Near Orthogonal Latin Hypercube (NOLH) sensitivity analysis sampling configuration
		case 72:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
				break;
			}

			if ( sim.sens != NULL )
			{
				if ( ! discard_change( false ) )	// unsaved configuration?
					break;

				varSA = sim.num_sensitivity_variables( );// number of variables to test
				plog( "\nNumber of elements for sensitivity analysis: %d", varSA );
				lab1 = NOLH_valid_tables( varSA, ch, 2 * MAX_LINE_SIZE );

				// detect the need of a new save path
				subDir = need_res_dir( sim.conf_path, sim.conf_name, sens_path, MAX_PATH_LENGTH );

				cmd( "set extdoe 0" );	// flag for using external DoE file
				cmd( "set NOLHfile \"NOLH.csv\"" );
				cmd( "set doeList [list %s]", lab1 );
				cmd( "set doesize [ lindex $doeList 0 ]" );	// minimum Doe size
				cmd( "set doeext 0" );	// flag for using extended number of samples

				cmd( "newtop .s \"NOLH Sampling\" { set choice 2 }" );

				cmd( "ttk::frame .s.p" );
				cmd( "ttk::label .s.p.l -text \"Output path\"" );
				cmd( "ttk::label .s.p.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", sens_path );
				cmd( "pack .s.p.l .s.p.w" );

				cmd( "ttk::frame .s.o" );
				cmd( "ttk::label .s.o.l1 -text \"NOLH table\"" );
				cmd( "ttk::combobox .s.o.c -width 15 -values $doeList -justify center -validate focusout -validatecommand { set n %%P; if { $n in $doeList } { set doesize %%P; return 1 } { %%W delete 0 end; %%W insert 0 $doesize; return 0 } } -invalidcommand { bell }" );
				cmd( "write_any .s.o.c $doesize" );
				cmd( "ttk::label .s.o.l2 -text \"(factors \u00D7 samples \u00D7 ext. samples)\"" );
				cmd( "pack .s.o.l1 .s.o.c .s.o.l2" );

				cmd( "ttk::checkbutton .s.e -text \"Extended number of samples\" -variable doeext" );
				if( varSA > 22 )
					cmd( ".s.e configure -state disabled" );

				cmd( "ttk::checkbutton .s.d -text \"External design file\" -variable extdoe -command { if { $extdoe == 1 } { .s.o.c configure -state disabled; .s.e configure -state disabled; .s.i.e configure -state normal; .s.i.e selection range 0 end; focus .s.i.e } { .s.o.c configure -state normal; .s.e configure -state normal; .s.i.e configure -state disabled } }" );

				cmd( "ttk::frame .s.i" );
				cmd( "ttk::label .s.i.l -text \"Design file name\"" );
				cmd( "ttk::entry .s.i.e -width 20 -justify center -textvariable NOLHfile -state disabled" );
				cmd( "ttk::label .s.i.w -justify center -text \"(file must be in the same folder\nas the configuration file; CSV\nformat with NO empty lines)\"" );
				cmd( "pack .s.i.l .s.i.e .s.i.w" );

				cmd( "pack .s.p .s.o .s.e .s.d .s.i -padx $_5 -pady $_5" );

				cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menudata_sa.html#nolh } { set choice 2 }" );

				cmd( "showtop .s" );
				cmd( "mousewarpto .s.b.ok" );

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "if { [ .s.o.c get ] in $doeList } { set doesize [ .s.o.c get ] } { bell }" );
				cmd( "destroytop .s" );

				if ( choice == 2 )
					break;

				// create a new save path if required
				if ( subDir )
					create_res_dir( sens_path );

				// ask to clean existing files before proceeding if required
				if ( check_res_dir( sens_path, sim.conf_name ) && sensitivity_clean_dir( sens_path ) )
					clean_res_dir( sens_path, sim.conf_name );

				if ( ! get_bool( "extdoe" ) )
					strcpy( NOLHfile, "" );
				else
					get_str( "NOLHfile", NOLHfile, MAX_PATH_LENGTH );

				num = ( sscanf( get_str( "doesize" ), "%d\u00D7", & j ) > 0 ) ? j : 0;

				// adjust an NOLH design of experiment (DoE) for the sensitivity data
				doe = new design( sim.sens, 1, NOLHfile, sens_path, 1, get_bool( "doeext" ) ? -1 : 0, num );

				if ( doe -> n == 0 )					// DoE configuration is not ok?
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration error\" -detail \"It was not possible to create a Non Orthogonal Latin Hypercube (NOLH) Design of Experiment (DoE) for the current sensitivity configuration. If the number of variables (factors) is large than 29, an external NOLH has to be provided in the file NOLH.csv (empty lines not allowed).\"" );

					if ( subDir )
						cmd( "catch { file delete -force \"%s\" }", sens_path );

					delete doe;
					break;
				}

				// Prevent running into too big sensitivity space samples (high computation times)
				if ( doe -> n > MAX_SENS_POINTS )
					// ask user before proceeding
					if ( sensitivity_too_large( doe -> n ) )
					{
						if ( subDir )
							cmd( "catch { file delete -force \"%s\" }", sens_path );

						delete doe;
						break;
					}

				// save the current object & cursor position for quick reload
				r->save_pos( );
				findexSens = 1;

				// create a design of experiment (DoE) for the sensitivity data
				cmd( "focustop .log" );

				sensitivity_doe( &findexSens, doe, sens_path );
				delete doe;

				// now reload the previously existing configuration
				if ( ! load_prev_configuration( ) )
				{
					choice = 0;
					return sim.root;
				}

				// restore pointed object and variable
				r = r->restore_pos( );

				if ( findexSens > 0 )
				{
					cmd( "set answer [ ttk::messageBox -parent . -title Confirmation -icon question -type yesno -default yes -message \"Create out-of-main-sample set of samples?\" -detail \"An out-of-sample set allows for better meta-model selection and fit-quality evaluation.\n\nPress 'Yes' to create a Monte Carlo sample now or 'No' otherwise.\" ]" );
					cmd( "switch $answer { yes { set choice 80 } no { set choice 0 } }" );

					if ( choice != 0 )
						return r;
				}
			}
			else
				sensitivity_undefined( );			// throw error

		break;


		// Create Monte Carlo (MC) random sensitivity analysis sampling configuration (over selected range values)
		case 80:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
				break;
			}

			if ( sim.sens != NULL )
			{
				if ( ! discard_change( false ) )	// unsaved configuration?
					break;

				varSA = sim.num_sensitivity_variables( );// number of variables to test
				plog( "\nNumber of elements for sensitivity analysis: %d", varSA );

				// get the number of Monte Carlo samples to produce
				sizMC = 10;
				Tcl_LinkVar( interp, "sizMC", ( char * ) & sizMC, TCL_LINK_INT );

				// detect the need of a new save path
				subDir = need_res_dir( sim.conf_path, sim.conf_name, sens_path, MAX_PATH_LENGTH );

				cmd( "set applst 1" );	// flag for appending to existing configuration files

				cmd( "newtop .s \"MC Range Sampling\" { set choice 2 }" );

				cmd( "ttk::frame .s.p" );
				cmd( "ttk::label .s.p.l -text \"Output path\"" );
				cmd( "ttk::label .s.p.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", sens_path );
				cmd( "pack .s.p.l .s.p.w" );

				cmd( "ttk::frame .s.i" );
				cmd( "ttk::label .s.i.l -justify center -text \"Monte Carlo sample size\nas number of samples\"" );
				cmd( "ttk::spinbox .s.i.e -width 5 -from 1 -to 9999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set sizMC %%P; return 1 } { %%W delete 0 end; %%W insert 0 $sizMC; return 0 } } -invalidcommand { bell } -justify center" );
				cmd( ".s.i.e insert 0 $sizMC" );
				cmd( "pack .s.i.l .s.i.e" );

				cmd( "ttk::checkbutton .s.c -text \"Append to existing configuration files\" -variable applst -state %s", findexSens > 1 ? "normal" : "disabled" );
				cmd( "pack .s.p .s.i .s.c -padx $_5 -pady $_5" );

				cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menudata_sa.html#mcrange } { set choice 2 }" );

				cmd( "showtop .s" );
				cmd( "mousewarpto .s.b.ok 0" );
				cmd( ".s.i.e selection range 0 end" );
				cmd( "focus .s.i.e" );

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "set sizMC [ .s.i.e get ]" );
				cmd( "destroytop .s" );
				Tcl_UnlinkVar( interp, "sizMC" );

				if ( choice == 2 )
					break;

				// Check if number is valid
				if ( sizMC < 1 )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid sample size\" -detail \"Invalid Monte Carlo sample size to perform the sensitivity analysis. Select at least one sample.\"" );
					choice = 0;
					break;
				}

				// Prevent running into too big sensitivity space samples (high computation times)
				if ( sizMC > MAX_SENS_POINTS )
					// ask user before proceeding
					if ( sensitivity_too_large( ( long ) sizMC ) )
						break;

				if ( findexSens < 1 || ( findexSens > 1 && ! get_bool( "applst" ) ) )
					findexSens = 1;

				// create a new save path if required
				if ( subDir )
					create_res_dir( sens_path );

				// ask to clean existing files before proceeding if required
				if ( findexSens == 1 && check_res_dir( sens_path, sim.conf_name ) && sensitivity_clean_dir( sens_path ) )
					clean_res_dir( sens_path, sim.conf_name );

				// save the current object & cursor position for quick reload
				r->save_pos( );

				// check if design file numbering should pick-up from previously generated files
				// adjust a design of experiment (DoE) for the sensitivity data
				doe = new design( sim.sens, 2, "", sens_path, findexSens, sizMC );
				sensitivity_doe( &findexSens, doe, sens_path );
				delete doe;

				// now reload the previously existing configuration
				if ( ! load_prev_configuration( ) )
				{
					choice = 0;
					return sim.root;
				}

				// restore pointed object and variable
				r = r->restore_pos( );
			}
			else
				sensitivity_undefined( );			// throw error

		break;


		// Create Elementary Effects (EE) sensitivity analysis sampling configuration (over selected range values)
		case 81:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to create a sensitivity analysis configuration.\"" );
				break;
			}

			if ( sim.sens != NULL )
			{
				if ( ! discard_change( false ) )	// unsaved configuration?
					break;

				varSA = sim.num_sensitivity_variables( );// number of variables to test
				plog( "\nNumber of elements for sensitivity analysis: %d", varSA );

				// get the number of Monte Carlo samples to produce
				int nLevels = 4, jumpSz = 2, nTraj = 10, nSampl = 100;
				Tcl_LinkVar( interp, "varSA", ( char * ) & varSA, TCL_LINK_INT );
				Tcl_LinkVar( interp, "nLevels", ( char * ) & nLevels, TCL_LINK_INT );
				Tcl_LinkVar( interp, "jumpSz", ( char * ) & jumpSz, TCL_LINK_INT );
				Tcl_LinkVar( interp, "nTraj", ( char * ) & nTraj, TCL_LINK_INT );
				Tcl_LinkVar( interp, "nSampl", ( char * ) & nSampl, TCL_LINK_INT );

				// detect the need of a new save path
				subDir = need_res_dir( sim.conf_path, sim.conf_name, sens_path, MAX_PATH_LENGTH );

				cmd( "newtop .s \"Elementary Effects Sampling\" { set choice 2 }" );

				cmd( "ttk::frame .s.o" );
				cmd( "ttk::label .s.o.l -text \"Output path\"" );
				cmd( "ttk::label .s.o.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", sens_path );
				cmd( "pack .s.o.l .s.o.w" );

				cmd( "ttk::frame .s.i" );
				cmd( "ttk::label .s.i.l1 -text \"Number of trajectories (r)\"" );
				cmd( "ttk::spinbox .s.i.e1 -width 5 -from 1 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set nTraj %%P; return 1 } { %%W delete 0 end; %%W insert 0 $nTraj; return 0 } } -invalidcommand { bell } -justify center" );
				cmd( ".s.i.e1 insert 0 $nTraj" );
				cmd( "ttk::label .s.i.l2 -text \"([ expr { $varSA + 1 } ]\u00D7r samples to create)\"" );
				cmd( "pack .s.i.l1 .s.i.e1 .s.i.l2" );

				cmd( "ttk::frame .s.p" );
				cmd( "ttk::label .s.p.l1 -text \"Trajectories pool size (M)\"" );
				cmd( "ttk::spinbox .s.p.e2 -width 5 -from 1 -to 999 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set nSampl %%P; return 1 } { %%W delete 0 end; %%W insert 0 $nSampl; return 0 } } -invalidcommand { bell } -justify center" );
				cmd( ".s.p.e2 insert 0 $nSampl" );
				cmd( "ttk::label .s.p.l2 -text \"(M > r enables optimization)\"" );
				cmd( "pack .s.p.l1 .s.p.e2 .s.p.l2" );

				cmd( "ttk::frame .s.l" );
				cmd( "ttk::label .s.l.l1 -text \"Number of levels (p)\"" );
				cmd( "ttk::spinbox .s.l.e3 -width 5 -from 1 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set nLevels %%P; return 1 } { %%W delete 0 end; %%W insert 0 $nLevels; return 0 } } -invalidcommand { bell } -justify center" );
				cmd( ".s.l.e3 insert 0 $nLevels" );
				cmd( "ttk::label .s.l.l2 -text \"(must be even)\"" );
				cmd( "pack .s.l.l1 .s.l.e3 .s.l.l2" );

				cmd( "ttk::frame .s.j" );
				cmd( "ttk::label .s.j.l1 -text \"Jump size\"" );
				cmd( "ttk::spinbox .s.j.e4 -width 5 -from 1 -to 99 -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set jumpSz %%P; return 1 } { %%W delete 0 end; %%W insert 0 $jumpSz; return 0 } } -invalidcommand { bell } -justify center" );
				cmd( ".s.j.e4 insert 0 $jumpSz" );
				cmd( "ttk::label .s.j.l2 -text \"( \u0394\u00D7(p - 1) )\"" );
				cmd( "pack .s.j.l1 .s.j.e4 .s.j.l2" );

				cmd( "ttk::label .s.t -justify center -text \"(for details on setting Elementary Effects\nsampling parameters see Morris (1991),\nCampolongo et al. (2007) and Ruano et al. (2012))\"" );

				cmd( "pack .s.o .s.i .s.p .s.l .s.j .s.t -padx $_5 -pady $_5" );

				cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menudata_sa.html#ee } { set choice 2 }" );

				cmd( "showtop .s" );
				cmd( "mousewarpto .s.b.ok 0" );
				cmd( ".s.i.e1 selection range 0 end" );
				cmd( "focus .s.i.e1" );

				choice = 0;
				while ( choice == 0 )
					Tcl_DoOneEvent( 0 );

				cmd( "set nTraj [ .s.i.e1 get ]" );
				cmd( "set nSampl [ .s.p.e2 get ]" );
				cmd( "set nLevels [ .s.l.e3 get ]" );
				cmd( "set jumpSz [ .s.j.e4 get ]" );
				cmd( "destroytop .s" );
				Tcl_UnlinkVar( interp, "varSA" );
				Tcl_UnlinkVar( interp, "nLevels" );
				Tcl_UnlinkVar( interp, "jumpSz" );
				Tcl_UnlinkVar( interp, "nTraj" );
				Tcl_UnlinkVar( interp, "nSampl" );

				if ( choice == 2 )
					break;

				// Check if numbers are valid
				if ( nLevels < 2 || nLevels % 2 != 0 || nTraj < 2 || nSampl < nTraj || jumpSz < 1 )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid configuration\" -detail \"Invalid Elementary Effects configuration to perform the sensitivity analysis. Check Morris (1991) and Campolongo et al. (2007) for details.\"" );
					choice = 0;
					break;
				}

				// Prevent running into too big sensitivity space samples (high computation times)
				if ( nTraj * ( varSA + 1 ) > MAX_SENS_POINTS )
					// ask user before proceeding
					if ( sensitivity_too_large( ( long ) ( nTraj * ( varSA + 1 ) ) ) )
						break;

				// create a new save path if required
				if ( subDir )
					create_res_dir( sens_path );

				// ask to clean existing files before proceeding if required
				if ( check_res_dir( sens_path, sim.conf_name ) && sensitivity_clean_dir( sens_path ) )
					clean_res_dir( sens_path, sim.conf_name );

				// save the current object & cursor position for quick reload
				r->save_pos( );
				findexSens = 1;

				// adjust a design of experiment (DoE) for the sensitivity data
				doe = new design( sim.sens, 3, "", sens_path, findexSens, nSampl, nLevels, jumpSz, nTraj );
				sensitivity_doe( &findexSens, doe, sens_path );
				delete doe;

				// now reload the previously existing configuration
				if ( ! load_prev_configuration( ) )
				{
					choice = 0;
					return sim.root;
				}

				// restore pointed object and variable
				r = r->restore_pos( );
			}
			else
				sensitivity_undefined( );			// throw error

		break;


		// import a sensitivity analysis configuration
		case 64:

			// check a model is already loaded
			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to import a sensitivity analysis configuration.\"" );
				break;
			}

			// check for existing sensitivity data loaded
			if ( sim.sens != NULL )
			{
				cmd( "set answer [ ttk::messageBox -parent . -type okcancel -icon warning -default ok -title Warning -message \"Sensitivity data already loaded\" -detail \"Press 'OK' if you want to discard the existing data before importing a new sensitivity configuration.\" ]; switch -- $answer { ok { set choice 1 } cancel { set choice 2 } }" );
				if ( choice == 2 )
					break;

				// empty sensitivity data
				sim.empty_sensitivity( );				// discard read data
				NOLH_clear( );							// deallocate DoE
				unsavedSense = false;					// nothing to save
				findexSens = 0;
			}

			// set default name and path to conf. file folder
			cmd( "set res \"%s\"", sim.conf_name );
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			// open dialog box to get file name & folder
			cmd( "set bah [ tk_getOpenFile -parent . -title \"Import Sensitivity Analysis File\" -defaultextension \".sa\" -initialfile \"$res\" -initialdir $path -filetypes { { {Sensitivity analysis files} {.sa} } } ]" );
			cmd( "if { [ string length $bah ] > 0 && ! [ fn_spaces \"$bah\" . ] } { set res $bah; set path [ file dirname $res ]; set res [ file tail $res ]; set last [ expr { [ string last .sa $res ] - 1 } ]; set res [ string range $res 0 $last ] } { set choice 2 }" );
			if ( choice == 2 )
				break;

			// form full name
			lab1 = get_str( "res" );
			lab2 = get_str( "path" );
			if ( sens_file != NULL )
				delete sens_file;
			sens_file = new char[ strlen( lab1 ) + strlen( lab2 ) + 5 ];
			sprintf( sens_file,"%s%s%s.sa", lab2, strlen( lab2 ) > 0 ? "/" : "", lab1 );

			// read sensitivity file (text mode)
			f = fopen( sens_file, "rt" );
			if ( f == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity Analysis file not found\"" );
				break;
			}

			if ( load_sensitivity( f ) != 0 )
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid sensitivity analysis file\" -detail \"Please check if you select a valid file or recreate your sensitivity analysis configuration.\"" );

			fclose( f );

		break;


		// export sensitivity analysis configuration
		case 65:

			// check a model is already loaded
			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to export a sensitivity analysis configuration.\"" );
				break;
			}

			// check for existing sensitivity data loaded
			if ( sim.sens == NULL )
			{
				sensitivity_undefined( );			// throw error
				break;
			}

			// default file name and path
			cmd( "set res %s", sim.conf_name );
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			// open dialog box to get file name & folder
			choice = 0;
			cmd( "set bah [ tk_getSaveFile -parent . -title \"Export Sensitivity Analysis File\" -defaultextension \".sa\" -initialfile $res -initialdir $path -filetypes { { {Sensitivity analysis files} {.sa} } } ]" );
			cmd( "if { [ string length $bah ] > 0 } { set path [ file dirname $bah ]; set res [ file tail $bah ]; set last [ expr { [ string last .sa $res ] - 1 } ]; set res [ string range $res 0 $last ] } { set choice 2 }" );
			if ( choice == 2 )
				break;

			// form full name
			lab1 = get_str( "res" );
			lab2 = get_str( "path" );
			if ( sens_file != NULL )
				delete sens_file;
			sens_file = new char[ strlen( lab1 ) + strlen( lab2 ) + 5 ];
			sprintf( sens_file,"%s%s%s.sa", lab2, strlen( lab2 ) > 0 ? "/" : "", lab1 );

			// write sensitivity file (text mode)
			f = fopen( sens_file, "wt" );  // use text mode for Windows better compatibility
			if ( f == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity analysis file not saved\" -detail \"Please check if the file name and path are valid.\"" );
				break;
			}

			if ( ! save_sensitivity( f ) )
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity analysis file cannot be saved\" -detail \"Check if the drive or the file is set READ-ONLY.\"" );

			fclose( f );
			unsavedSense = false;			// nothing to save

		break;


		// export configuration in legacy LSD format
		case 9:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration to export\" -detail \"Please load or create and load a configuration before trying to export to legacy LSD format.\"" );
				break;
			}

			// default file name
			cmd( "set res %s-legacy", sim.conf_name );

			// make sure there is a path set
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			// open dialog box to get file name & folder
			choice = 0;
			cmd( "set bah [ tk_getSaveFile -parent . -title \"Export Configuration in Legacy LSD Format\" -defaultextension \".csv\" -initialfile $res -initialdir $path -filetypes { { {LSD configuration files} {.lsd} } } ]" );
			cmd( "if { [ string length $bah ] > 0 } { set path [ file dirname $bah ]; set res [ file rootname [ file tail $bah ] ]; set ext [ file extension $bah ] } { set choice 2 }" );

			if ( choice == 2 )
				break;

			// write export file
			if ( ! sim.save_txt_configuration( get_str( "path" ), get_str( "res" ), get_str( "ext" ), eq_file, eq_txt ) )
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Legacy configuration file not saved\" -detail \"Please check if the file name and path are valid, or if the drive or the file is set READ-ONLY, or try to save to a different location.\"" );

		break;


		// export saved elements details
		case 91:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration to export\" -detail \"Please load or create and load a configuration before trying to export the details on the elements to save.\"" );
				break;
			}

			// warn about no variable being saved
			i = 0;
			sim.root->count_save( & i );
			if ( i == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"No variable or parameter marked to be saved\" -detail \"Please mark the variables and parameters to be saved before trying to export the details on the elements to save.\"" );
				break;
			}

			// default file name
			cmd( "set res %s-saved", sim.conf_name );

			// make sure there is a path set
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			// open dialog box to get file name & folder
			choice = 0;
			cmd( "set bah [ tk_getSaveFile -parent . -title \"Export Saved Elements Configuration as Comma-separated Text File\" -defaultextension \".csv\" -initialfile $res -initialdir $path -filetypes { { {Comma-separated files} {.csv} } } ]" );
			cmd( "if { [ string length $bah ] > 0 } { set path [ file dirname $bah ]; set res [ file tail $bah ] } { set choice 2 }" );
			if ( choice == 2 )
				break;

			// form full name
			lab1 = get_str( "res" );
			lab2 = get_str( "path" );
			snprintf( lab, MAX_PATH_LENGTH,"%s%s%s", lab2, strlen( lab2 ) > 0 ? "/" : "", lab1 );

			// write export file (text mode)
			f = fopen( lab, "wt" );	 // use text mode for Windows better compatibility
			if ( f == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Saved elements configuration file not saved\" -detail \"Please check if the file name and path are valid.\"" );
				break;
			}

			strcpy( ch, CSV_SEP );

			// write .csv header & content
			fprintf( f, "Name%sType%sObject%sDescription\n", ch, ch, ch );
			sim.root->get_saved( f, ch );
			fclose( f );

		break;


		// export sensitivity configuration as a .csv file
		case 90:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to save a sensitivity analysis configuration.\"" );
				break;
			}

			// check for existing sensitivity data loaded
			if ( sim.sens == NULL )
			{
				sensitivity_undefined( );			// throw error
				break;
			}

			// default file name
			cmd( "set res %s-limits", sim.conf_name );

			// make sure there is a path set
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			// open dialog box to get file name & folder
			choice = 0;
			cmd( "set bah [ tk_getSaveFile -parent . -title \"Export Sensitivity Limits as Comma-separated Text File\" -defaultextension \".csv\" -initialfile $res -initialdir $path -filetypes { { {Comma-separated files} {.csv} } } ]" );
			cmd( "if { [ string length $bah ] > 0 } { set path [ file dirname $bah ]; set res [ file tail $bah ] } { set choice 2 }" );
			if ( choice == 2 )
				break;

			// form full name
			lab1 = get_str( "res" );
			lab2 = get_str( "path" );
			snprintf( lab, MAX_PATH_LENGTH,"%s%s%s", lab2, strlen( lab2 ) > 0 ? "/" : "", lab1 );

			// write export file (text mode)
			f = fopen( lab, "wt" );	 // use text mode for Windows better compatibility
			if ( f == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity limits file not saved\" -detail \"Please check if the file name and path are valid.\"" );
				break;
			}

			// write .csv header
			strcpy( ch, CSV_SEP );
			fprintf( f, "Name%sType%sLag%sFormat%sValue%sMinimum%sMaximum%sDescription\n", ch, ch, ch, ch, ch, ch, ch );

			// write data
			r->get_sa_limits( f, ch );

			fclose( f );

		break;


		// Show sensitivity analysis configuration
		case 66:

			choice = 0;

			// check for existing sensitivity data loaded
			if ( sim.sens == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"There is no sensitivity data to show\"" );
				break;
			}

			// print data to log window
			for ( i = 0, cs = sim.sens; cs != NULL; cs = cs->next, ++i );
			plog( "\n\nVariables and parameters set for sensitivity analysis (%d):\n", i );
			for ( cs = sim.sens; cs != NULL; cs = cs->next )
			{
				if ( cs->param == 1 )
					plog( "Param: %s\\[%s\\]\t#%d:\t", cs->label, cs->integer ? "int" : "flt", cs->num_val );
				else
					plog( "Var: %s(-%d)\\[%s\\]\t#%d:\t", cs->label, cs->lag + 1, cs->integer ? "int" : "flt", cs->num_val );

				for ( i = 0; i < cs->num_val; ++i )
					plog_tag( "%g\t", "highlight", cs->val[ i ] );
				plog( "\n" );
			}

		break;


		// Remove sensitivity analysis configuration
		case 67:

			choice = 0;

			// check for existing sensitivity data loaded
			if ( sim.sens == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No sensitivity data to remove\"" );
				break;
			}

			if ( ! discard_change( true, true ) )	// unsaved configuration?
				break;

			// empty sensitivity data
			sim.empty_sensitivity( );				// discard read data
			NOLH_clear( );							// deallocate DoE
			plog( "\nSensitivity data removed.\n" );
			unsavedChange = true;
			unsavedSense = false;
			findexSens = 0;

		break;


		// Show variables for data assimilation
		case 16:

			choice = 0;

			// check for existing assimilation settings loaded
			if ( sim.assim == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"There is no data assimilation settings to show\"" );
				break;
			}

			// print data to log window
			plog( "\n\nVariables set for data assimilation:\n" );
			for ( ca = sim.assim; ca != NULL; ca = ca->next )
			{
				plog( "Var: %s \t%s\t(col=", ca->label, ca->csv_file );

				if ( ca->data_col_name != NULL && strlen( ca->data_col_name ) != 0 )
					plog_tag( "'%s'", "highlight", ca->data_col_name );
				else
					plog_tag( "%d", "highlight", ca->data_col_num );

				if ( ( ca->t_col_name != NULL && strlen( ca->t_col_name ) != 0 ) || ca->t_col_num > 0 )
				{
					plog( " t_col=" );

					if ( ca->t_col_name != NULL && strlen( ca->t_col_name ) != 0 )
						plog_tag( "'%s'", "highlight", ca->t_col_name );
					else
						plog_tag( "%d", "highlight", ca->t_col_num );
				}

				plog( ")\n" );
			}

		break;


		// Remove variables from data assimilation
		case 25:

			choice = 0;

			// check for existing assimilation settings loaded
			if ( sim.assim == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No data assimilation settings to remove\"" );
				break;
			}

			if ( ! discard_change( true, true ) )	// unsaved configuration?
				break;

			// empty data assimilation
			sim.empty_assimilation( );
			plog( "\nData assimilation settings removed.\n" );
			unsavedChange = true;

		break;


		// Create batch for multi-runs jobs and optionally run it
		case 68:

			// check for data assimilation
			if ( sim.assim != NULL && ! sim.assim_disable )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Data assimilation configured\" -detail \"The current configuration is set to perform data assimilation, which already uses parallel processing. Please use the non-parallel run option.\"" );
				break;
			}

			// check a model is already loaded
			if ( ! sim.conf_ok )
				findexSens = 0;						// no sensitivity created
			else
				if ( ! discard_change( false ) )	// unsaved configuration?
					break;

			// check for existing term executable
			snprintf( term_exe, MAX_PATH_LENGTH, "%s/%s", lsd::model_path, LSD_TERM );// form full executable name
			if ( platform == _WIN_ )
				lsd::strcatn( term_exe, ".exe", MAX_PATH_LENGTH );	// add Windows ending

			if ( ( f = fopen( term_exe, "rb" ) ) == NULL )
			{
				if ( ! make_terminal( ) )
					break;
			}
			else
				fclose( f );

			// check if terminal executable/lib files are older than running executable file
			if ( check_term_exec( term_exe ) )
			{
				cmd( "switch [ ttk::messageBox -parent . -title Warning -icon warning -type yesnocancel -default yes -message \"Recompile '%s'?\" -detail \"The existing terminal executable file ('%s') is older than the current executable.\n\nPress 'Yes' to recompile, 'No' continue anyway, or 'Cancel' to abort.\" ] { \
						yes { set choice 0 } \
						no { set choice 1 } \
						cancel { set choice 2 } \
					}", LSD_TERM, LSD_TERM );

				if ( choice == 2 )
					break;

				if ( choice == 0 )
					if ( ! make_terminal( ) )
						break;
			}

			// check if serial sensitivity configuration was just created
			choice = 0;
			if ( findexSens > 0 )
				cmd( "set answer [ ttk::messageBox -parent . -type yesnocancel -icon question -default yes -title \"Parallel Batch\" -message \"Configuration set available\" -detail \"A sequential sensitivity set of configuration files was just produced and can be used to create the batch.\n\nPress 'Yes' to confirm or 'No' to select a different set of files.\" ]; switch -- $answer { yes { set choice 1 } no { set choice 0 } cancel { set choice 2 } }" );
			if ( choice == 2 )
				break;

			// get configuration files to use
			if ( choice == 1 )							// use current configuration files
			{
				if ( strlen( sens_path ) == 0 || strlen( sim.conf_name ) == 0 )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Invalid simulation folder or name\" -detail \"Please try again.\"" );
					findexSens = 0;						// no sensitivity created
					break;
				}

				ffirst = fSeq = 1;
				fnext = findexSens;
				findexSens = 0;
				lsd::strcpyn( out_file, sim.conf_name, MAX_PATH_LENGTH );
				lsd::strcpyn( out_dir, sens_path, MAX_PATH_LENGTH );
				cmd( "set res \"%s\"", sim.conf_name );
				cmd( "set path \"%s\"", sim.conf_path );
			}
			else										// ask for first configuration file
			{
				cmd( "set answer [ ttk::messageBox -parent . -type yesnocancel -icon question -default yes -title \"Create Batch\" -message \"Select sequence of configuration files?\" -detail \"Press 'Yes' to choose the first file of the continuous sequence (format: 'name_NNN.lsd') or 'No' to select a different set of files (use 'Ctrl' to pick multiple files).\" ]; switch -- $answer { yes { set choice 1 } no { set choice 0 } cancel { set choice 2 } }" );
				if ( choice == 2 )
					break;
				else
					fSeq = choice;

				if ( fSeq && strlen( sim.conf_name ) > 0 )// default name
					cmd( "set res \"%s_1.lsd\"", sim.conf_name );
				else
					cmd( "set res \"\"" );

				cmd( "set path \"%s\"", sim.conf_path );
				if ( strlen( sim.conf_path ) > 0 )
					cmd( "cd $path" );

				// open dialog box to get file name & folder
				if ( fSeq )								// file sequence?
				{
					cmd( "set bah [ tk_getOpenFile -parent . -title \"Load First Configuration File\" -defaultextension \".lsd\" -initialfile $res -initialdir $path -filetypes { { {LSD model files} {.lsd} } } -multiple no ]" );
					cmd( "if { [ string length $bah ] > 0 && ! [ fn_spaces \"$bah\" . ] } { \
							set res $bah; \
							set path [ file dirname $res ]; \
							set res [ file tail $res ]; \
							set last [ expr { [ string last .lsd $res ] - 1 } ]; \
							set res [ string range $res 0 $last ]; \
							set numpos [ expr { [ string last _ $res ] + 1 } ]; \
							if { $numpos > 0 } { \
								set choice [ expr { [ string range $res $numpos end ] } ]; \
								set res [ string range $res 0 [ expr { $numpos - 2 } ] ] \
							} else { \
								plog \"\nInvalid file name for sequential set: $res\n\"; \
								set choice 0 \
							} \
						} else { \
							set choice 0 \
						}" );
					if ( choice == 0 )
						break;

					ffirst = choice;
					get_str( "res", out_file, MAX_PATH_LENGTH );
					get_str( "path", out_dir, MAX_PATH_LENGTH );
					f = NULL;
					do									// search for all sequential files
					{
						if ( strlen( out_dir ) == 0 )			// default path
							snprintf( lab, MAX_BUFF_SIZE, "%s_%d.lsd", out_file, choice++ );
						else
							snprintf( lab, MAX_BUFF_SIZE, "%s/%s_%d.lsd", out_dir, out_file, choice++ );

						if ( f != NULL )
							fclose( f );
						f = fopen( lab, "r" );
					}
					while ( f != NULL );

					fnext = choice - 1;
				}
				else									// bunch of files?
				{
					cmd( "set bah [ tk_getOpenFile -parent . -title \"Load Configuration Files\" -defaultextension \".lsd\" -initialdir $path -filetypes { { {LSD model files} {.lsd} } } -multiple yes ]" );
					cmd( "set choice [ llength $bah ]" );
					cmd( "if { $choice > 0 && ! [ fn_spaces [ lindex $bah 0 ] . 1 ] } { \
							set res [ lindex $bah 0 ]; \
							set path [ file dirname $res ]; \
							set res [ file tail $res ]; \
							set last [ expr { [ string last .lsd $res ] - 1 } ]; \
							set res [ string range $res 0 $last ]; \
							set numpos [ expr { [ string last _ $res ] + 1 } ]; \
							if { $numpos > 0 } { \
								set res [ string range $res 0 [ expr { $numpos - 2 } ] ] \
							} \
						}" );
					if ( choice == 0 )
						break;

					ffirst = 1;
					fnext = choice + 1;
					get_str( "path", out_dir, MAX_PATH_LENGTH );
				}
			}

			Tcl_LinkVar( interp, "natBat", ( char * ) & natBat, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "no_res", ( char * ) & sim.no_res, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "no_tot", ( char * ) & sim.no_tot, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "docsv", ( char * ) & sim.docsv, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "dozip", ( char * ) & sim.dozip, TCL_LINK_BOOLEAN );

			if ( sim.no_tot )
				sim.no_res = false;

			cmd( "set res2 $res" );
			cmd( "set cores %d", sim.max_threads );
			cmd( "set threads 1" );

			cmd( "newtop .s \"Parallel Batch\" { set choice 2 }" );

			cmd( "ttk::frame .s.u" );
			cmd( "ttk::label .s.u.l -text \"Output path\"" );
			cmd( "ttk::label .s.u.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", out_dir );
			cmd( "pack .s.u.l .s.u.w" );

			cmd( "ttk::frame .s.t" );
			cmd( "ttk::label .s.t.l -text \"Batch file base name\"" );
			cmd( "ttk::entry .s.t.e -width 20 -textvariable res2 -justify center" );
			cmd( "pack .s.t.l .s.t.e" );

			cmd( "ttk::frame .s.c" );
			cmd( "ttk::label .s.c.l -justify center -text \"Number of parallel\nLSD runs\"" );
			cmd( "ttk::spinbox .s.c.e -width 5 -from 1 -to 999 -justify center -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set cores %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cores; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".s.c.e insert 0 $cores" );
			cmd( "ttk::label .s.c.w -justify center -text \"(a number higher than the\nnumber of processors/cores\nis not recommended)\"" );
			cmd( "pack .s.c.l .s.c.e .s.c.w" );

			cmd( "ttk::frame .s.p" );
			cmd( "ttk::label .s.p.l -justify center -text \"Number of threads\nper LSD runs\"" );
			cmd( "ttk::spinbox .s.p.e -width 5 -from 1 -to 99 -justify center -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set threads %%P; return 1 } { %%W delete 0 end; %%W insert 0 $threads; return 0 } } -invalidcommand { bell } -justify center" );
			cmd( ".s.p.e insert 0 $threads" );
			cmd( "ttk::label .s.p.w -justify center -text \"(a number higher than 1\nis only useful when parallel\ncomputation is enabled)\"" );
			cmd( "pack .s.p.l .s.p.e .s.p.w" );

			cmd( "ttk::frame .s.o" );
			cmd( "ttk::checkbutton .s.o.nores -text \"Skip generating results files\" -variable no_res -command { \
						if { $no_res && $no_tot } { \
							set no_tot 0 \
						} \
					}" );
			cmd( "ttk::checkbutton .s.o.notot -text \"Skip generating totals files\" -variable no_tot -command { \
						if { $no_res && $no_tot } { \
							set no_res 0 \
						} \
					}" );
			cmd( "ttk::checkbutton .s.o.n -text \"Native batch format\" -variable natBat" );
			cmd( "ttk::checkbutton .s.o.dozip -text \"Generate zipped files\" -variable dozip" );
			cmd( "ttk::checkbutton .s.o.docsv -text \"Comma-separated text format (.csv)\" -variable docsv" );
			cmd( "pack .s.o.nores .s.o.notot .s.o.n .s.o.dozip .s.o.docsv -anchor w" );

			cmd( "pack .s.u .s.t .s.c .s.p .s.o -padx $_5 -pady $_5" );

			cmd( "okhelpcancel .s b { set choice 1 } { LsdHelp menurun.html#parallel } { set choice 2 }" );
			cmd( "bind .s.c.e <KeyPress-Return> { .s.b.ok invoke }" );

			cmd( "showtop .s" );
			cmd( "mousewarpto .s.b.ok 0" );
			cmd( ".s.c.e selection range 0 end" );
			cmd( "focus .s.c.e" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set cores [ .s.c.e get ]" );
			cmd( "set threads [ .s.p.e get ]" );

			cmd( "destroytop .s" );

			Tcl_UnlinkVar( interp, "natBat" );
			Tcl_UnlinkVar( interp, "no_res" );
			Tcl_UnlinkVar( interp, "no_tot" );
			Tcl_UnlinkVar( interp, "docsv" );
			Tcl_UnlinkVar( interp, "dozip" );

			if ( choice == 2 )
				break;

			param = get_int( "cores" );
			if ( param < 1 || param > SRV_MAX_CORES )
				param = std::min( sim.max_threads, SRV_MAX_CORES );

			nature = get_int( "threads" );
			if ( nature < 1 || nature > SRV_MAX_CORES )
				nature = std::min( sim.max_threads, SRV_MAX_CORES );

			get_str( "res2", out_bat, MAX_PATH_LENGTH );

			// select batch format & create batch file
			cmd( "if [ string equal $CurPlatform windows ] { if { $natBat == 1 } { set choice 1 } { set choice 2 } } { if { $natBat == 1 } { set choice 3 } { set choice 4 } }" );
			if ( fSeq )
				if ( choice == 1 || choice == 4 )
					snprintf( lab, MAX_BUFF_SIZE, "%s/%s_%d_%d.bat", out_dir, out_bat, ffirst, fnext - 1 );
				else
					snprintf( lab, MAX_BUFF_SIZE, "%s/%s_%d_%d.sh", out_dir, out_bat, ffirst, fnext - 1 );
			else
				if ( choice == 1 || choice == 4 )
					snprintf( lab, MAX_BUFF_SIZE, "%s/%s.bat", out_dir, out_bat );
				else
					snprintf( lab, MAX_BUFF_SIZE, "%s/%s.sh", out_dir, out_bat );

			f = fopen( lab, "wb" );						// binary mode to bypass CR/LF handling
			if ( f == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Batch file cannot be created\" -detail \"Check if LSD still has WRITE access to the model directory.\"" );
				findexSens = 0;							// no sensitivity created
				break;
			}

			if ( choice == 1 || choice == 4 )			// Windows header
			{
				// convert to Windows folder separators (\)
				for ( i = 0; ( unsigned ) i < strlen( term_exe ); ++i )
					if ( term_exe[ i ] == '/' )
						term_exe[ i ] = '\\';

				lsd::strcpyn( win_dir, out_dir, MAX_PATH_LENGTH );

				for ( i = 0; ( unsigned ) i < strlen( win_dir ); ++i )
					if ( win_dir[ i ] == '/' )
						win_dir[ i ]='\\';

				fprintf( f, "@echo off\nrem Batch generated by LSD\r\n" );
				fprintf( f, "echo Processing %d configuration files in up to %d parallel processes...\r\n", fnext - ffirst, param );
				fprintf( f, "if \"%%~1\"==\"\" (set LSD_EXEC=\"%s\") else (set LSD_EXEC=\"%%~1\")\r\n", term_exe );
				fprintf( f, "if \"%%~2\"==\"\" (set LSD_CONFIG_PATH=\"%s\") else (set LSD_CONFIG_PATH=\"%%~2\")\r\n", win_dir );
				fprintf( f, "set LSD_EXEC=%%LSD_EXEC:\"=%%\r\n" );
				fprintf( f, "set LSD_CONFIG_PATH=%%LSD_CONFIG_PATH:\"=%%\r\n" );
				fprintf( f, "echo LSD executable: %%LSD_EXEC%%\r\n" );
				fprintf( f, "echo Configuration path: %%LSD_CONFIG_PATH%%\r\n" );
				fprintf( f, "echo Use %s.bat LSD_EXEC CONFIG_PATH to change defaults\r\n", out_bat );
			}
			else										// Unix header
			{
				if ( ! natBat )							// Unix in Windows?
				{
					if ( strchr( term_exe, ':' ) != NULL )	// remove Windows drive letter
					{
						lsd::strcpyn( lab_old, strchr( term_exe, ':' ) + 1, 2 * MAX_PATH_LENGTH );
						lsd::strcpyn( term_exe, lab_old, MAX_PATH_LENGTH );
					}

					if ( strchr( out_dir, ':' ) != NULL )	// remove Windows drive letter
					{
						lsd::strcpyn( lab_old, strchr( out_dir, ':' ) + 1, 2 * MAX_PATH_LENGTH );
						lsd::strcpyn( out_dir, lab_old, MAX_PATH_LENGTH );
					}

					if ( ( lab0 = strstr( term_exe, ".exe" ) ) != NULL )	// remove Windows extension, if present
						lab0[ 0 ]='\0';
					else
						if ( ( lab0 = strstr( term_exe, ".EXE" ) ) != NULL )
							lab0[ 0 ]='\0';
				}

				// set background low priority in servers (cores/jobs > SRV_MIN_CORES)
				if ( nature > SRV_MIN_CORES || ( param > SRV_MIN_CORES && fnext - ffirst > SRV_MIN_CORES ) )
				{
					snprintf( lab_old, 2 * MAX_PATH_LENGTH, "nice %s", term_exe );
					lsd::strcpyn( term_exe, lab_old, MAX_PATH_LENGTH );
				}

				fprintf( f, "#!/bin/bash\n# Script generated by LSD\n" );
				fprintf( f, "echo \"Processing %d configuration files in up to %d parallel processes...\"\n", fnext - ffirst, param );
				fprintf( f, "if [ \"$1\" = \"\" ]; then LSD_EXEC=\"%s\"; else LSD_EXEC=\"$1\"; fi\n", term_exe );
				fprintf( f, "if [ \"$2\" = \"\" ]; then LSD_CONFIG_PATH=\"%s\"; else LSD_CONFIG_PATH=\"$2\"; fi\n", out_dir );
				fprintf( f, "echo \"LSD executable: $LSD_EXEC\"\n" );
				fprintf( f, "echo \"Configuration path: $LSD_CONFIG_PATH\"\n" );
				fprintf( f, "echo \"Use %s.sh LSD_EXEC CONFIG_PATH to change default paths\"\n", out_bat );
			}

			logs.clear( );

			if ( fSeq && ( fnext - ffirst ) > param )	// if possible, work in blocks
			{
				num = ( fnext - ffirst ) / param;		// base number of cases per core
				sl = ( fnext - ffirst ) % param;		// remaining cases per core
				for ( i = ffirst, j = 1; j <= param; ++j )	// allocates files by the number of cores
				{
					snprintf( lab_old, 2 * MAX_PATH_LENGTH, "%s_%d.log", out_file, j );
					logs.push_back( lab_old );

					if ( choice == 1 || choice == 4 )	// Windows
						fprintf( f, "start \"LSD Process %d\" /B \"%%LSD_EXEC%%\" -c %d -f \"%%LSD_CONFIG_PATH%%\\%s\" -s %d -e %d%s%s%s%s -l \"%%LSD_CONFIG_PATH%%\\%s\"\r\n", j, nature, out_file, i, j <= sl ? i + num : i + num - 1, sim.no_res ? " -r" : "", sim.no_tot ? " -p" : "", sim.docsv ? " -t" : "", sim.dozip ? "" : " -z", lab_old );
					else								// Unix
						fprintf( f, "$LSD_EXEC -c %d -f \"$LSD_CONFIG_PATH\"/%s -s %d -e %d%s%s%s%s -l \"$LSD_CONFIG_PATH\"/%s &\n", nature, out_file, i, j <= sl ? i + num : i + num - 1, sim.no_res ? " -r" : "", sim.no_tot ? " -p" : "", sim.docsv ? " -t" : "", sim.dozip ? "" : " -z", lab_old );

					j <= sl ? i += num + 1 : i += num;
				}
			}
			else										// if not, do one by one
			{
				for ( i = ffirst, j = 1; i < fnext; ++i, ++j )
				{
					if ( fSeq )
					{
						snprintf( lab_old, 2 * MAX_PATH_LENGTH, "%s_%d.log", out_file, i );

						if ( choice == 1 || choice == 4 )	// Windows
							fprintf( f, "start \"LSD Process %d\" /B \"%%LSD_EXEC%%\" -c %d -f \"%%LSD_CONFIG_PATH%%\\%s_%d.lsd\"%s%s%s%s -l \"%%LSD_CONFIG_PATH%%\\%s\"\r\n", j, nature, out_file, i, sim.no_res ? " -r" : "", sim.no_tot ? " -p" : "", sim.docsv ? " -t" : "", sim.dozip ? "" : " -z", lab_old );
						else								// Unix
							fprintf( f, "$LSD_EXEC -c %d -f \"$LSD_CONFIG_PATH\"/%s_%d.lsd%s%s%s%s -l \"$LSD_CONFIG_PATH\"/%s &\n", nature, out_file, i, sim.no_res ? " -r" : "", sim.no_tot ? " -p" : "", sim.docsv ? " -t" : "", sim.dozip ? "" : " -z", lab_old );
					}
					else
					{	// get the selected file names, one by one
						cmd( "set res3 [ lindex $bah %d ]; set res3 [ file tail $res3 ]; set last [ expr { [ string last .lsd $res3 ] - 1 } ]; set res3 [ string range $res3 0 $last ]", j - 1	);
						get_str( "res3", out_file, MAX_PATH_LENGTH - 4 );
						snprintf( lab_old, 2 * MAX_PATH_LENGTH, "%s.log", out_file );

						if ( choice == 1 || choice == 4 )	// Windows
							fprintf( f, "start \"LSD Process %d\" /B \"%%LSD_EXEC%%\" -c %d -f \"%%LSD_CONFIG_PATH%%\\%s.lsd\"%s%s%s%s -l \"%%LSD_CONFIG_PATH%%\\%s\"\r\n", j, nature, out_file, sim.no_res ? " -r" : "", sim.no_tot ? " -p" : "", sim.docsv ? " -t" : "", sim.dozip ? "" : " -z", lab_old );
						else								// Unix
							fprintf( f, "$LSD_EXEC -c %d -f \"$LSD_CONFIG_PATH\"/%s.lsd%s%s%s%s -l \"$LSD_CONFIG_PATH\"/%s &\n", nature, out_file, sim.no_res ? " -r" : "", sim.no_tot ? " -p" : "", sim.docsv ? " -t" : "", sim.dozip ? "" : " -z", lab_old );
					}

					logs.push_back( lab_old );
				}
			}

			if ( fSeq )
				if ( choice == 1 || choice == 4 )	// Windows closing
				{
					fprintf( f, "echo %d log files being generated: %s_1.log to %s_%d.log .\r\n", j - 1, out_file, out_file, j - 1 );
					fclose( f );
				}
				else								// Unix closing
				{
					fprintf( f, "echo \"%d log files being generated: %s_1.log to %s_%d.log .\"\n", j - 1, out_file, out_file, j - 1 );
					fclose( f );
					chmod( lab, ACCESSPERMS );		// set executable perms
				}
			else
				if ( choice == 1 || choice == 4 )	// Windows closing
				{
					fprintf( f, "echo %d log files being generated.\r\n", j - 1 );
					fclose( f );
				}
				else								// Unix closing
				{
					fprintf( f, "echo \"%d log files being generated.\"\n", j - 1 );
					fclose( f );
					chmod( lab, ACCESSPERMS );		// set executable perms
				}

			plog( "\nParallel batch file created: %s", lab );

			if ( ! natBat )
				break;

			// ask if script/batch should be executed right away
			cmd( "set answer [ ttk::messageBox -parent . -type yesno -icon question -default no -title \"Run Parallel Batch\" -message \"Run created batch?\" -detail \"The batch for running the configuration files was created.\n\nPress 'Yes' if you want to start the it as separated processes now.\" ]; switch -- $answer { yes { set choice 1 } no { set choice 2 } }" );
			if ( choice == 2 )
				break;

			// start the job
			cmd( "set oldpath [ pwd ]" );
			cmd( "set path \"%s\"", out_dir );
			if ( strlen( out_dir ) > 0 )
				cmd( "cd $path" );

			cmd( "catch { exec %s & }", lab );
			show_logs( out_dir, logs );

			cmd( "set path $oldpath" );
			cmd( "cd $path" );

		break;


		// Start terminal job as a separate background process
		case 69:

			// check if background are not being run already
			if ( sim.parallel_monitor )
			{
				cmd( "if { [ ttk::messageBox -parent . -type okcancel -default ok -icon warning -title Warning -message \"Abort running simulation?\" -detail \"A set of parallel simulation runs is being executed in background. You may choose to interrupt it now and proceed, or wait until it finishes before running a new one.\" ] eq \"ok\" } { set choice 1 } { set choice 0 }" );

				if ( choice == 0 )
					break;

				if ( ! sim.stop_parallel( ) )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Failed to abort running simulation\" -detail \"Please wait until the current parallel run finishes before trying to start a new one.\"" );
					break;
				}
			}

			// check a model is already loaded
			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 || strlen( sim.conf_file ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to start a parallel run.\"" );
				break;
			}

			// check for data assimilation
			if ( sim.assim != NULL && ! sim.assim_disable )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Data assimilation configured\" -detail \"The current configuration is set to perform data assimilation, which already uses parallel processing. Please use the non-parallel run option.\"" );
				break;
			}

			// check for existing terminal executable
			snprintf( term_exe, MAX_PATH_LENGTH, "%s/%s", lsd::model_path, LSD_TERM );// form full executable name
			if ( platform == _WIN_ )
				lsd::strcatn( term_exe, ".exe", MAX_PATH_LENGTH );	// add Windows ending

			if ( ( f = fopen( term_exe, "rb" ) ) == NULL )
			{
				if ( ! make_terminal( ) )
					break;
			}
			else
				fclose( f );

			// check if terminal executable/lib files are older than running executable file
			if ( check_term_exec( term_exe ) )
			{
				cmd( "switch [ ttk::messageBox -parent . -title Warning -icon warning -type yesnocancel -default yes -message \"Recompile '%s'?\" -detail \"The existing terminal executable file ('%s') is older than the current executable.\n\nPress 'Yes' to recompile, 'No' continue anyway, or 'Cancel' to abort.\" ] { \
						yes { set choice 0 } \
						no { set choice 1 } \
						cancel { set choice 2 } \
					}", LSD_TERM, LSD_TERM );

				if ( choice == 2 )
					break;

				if ( choice == 0 )
					if ( ! make_terminal( ) )
						break;
			}

			// remove any custom save path (save to current by default)
			sim.results_alt_path( "" );

			// detect the need of a new save path and if it has results files
			subDir = need_res_dir( sim.conf_path, sim.conf_name, out_dir, MAX_PATH_LENGTH );
			overwDir = check_res_dir( out_dir );

			Tcl_LinkVar( interp, "no_res", ( char * ) & sim.no_res, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "no_tot", ( char * ) & sim.no_tot, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "dobar", ( char * ) & sim.dobar, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "docsv", ( char * ) & sim.docsv, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "doover", ( char * ) & doover, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "dozip", ( char * ) & sim.dozip, TCL_LINK_BOOLEAN );
			Tcl_LinkVar( interp, "overwConf", ( char * ) & overwConf, TCL_LINK_BOOLEAN );

			// only ask to overwrite configuration if there are changes
			overwConf = unsaved_change( ) ? true : false;
			sim.add_to_tot = false;

			if ( sim.no_tot )
				sim.no_res = false;

			param = std::min( sim.last_run, sim.max_threads );

			cmd( "set simNum %d", sim.last_run );
			cmd( "set firstFile \"%s_%d\"", sim.conf_name, sim.seed );
			cmd( "set lastFile \"%s_%d\"", sim.conf_name, sim.seed + sim.last_run - 1 );
			cmd( "set totFile \"%s\"", sim.conf_name );
			cmd( "set resExt %s", sim.docsv ? "csv" : "res" );
			cmd( "set totExt %s", sim.docsv ? "csv" : "tot" );
			cmd( "set zipExt %s", sim.dozip ? ".gz" : "" );
			cmd( "set cores %d", param );
			cmd( "set tot_msg_warn \"(WARNING: existing totals file(s) in\noutput path may be overwritten)\"" );

			// confirm overwriting current configuration
			cmd( "set b .batch" );
			cmd( "newtop $b \"Parallel Run\" { set choice 2 }" );

			cmd( "ttk::frame $b.f1" );
			cmd( "ttk::label $b.f1.l -text \"Model configuration\"" );
			cmd( "ttk::label $b.f1.w -text \"%s\" -style hl.TLabel", sim.conf_name );
			cmd( "pack $b.f1.l $b.f1.w" );

			cmd( "ttk::frame $b.f2" );

			cmd( "ttk::frame $b.f2.t" );
			cmd( "ttk::label $b.f2.t.l -text \"Cases:\"" );
			cmd( "ttk::label $b.f2.t.w -text \"%d\" -style hl.TLabel", sim.last_t );
			cmd( "pack $b.f2.t.l $b.f2.t.w -side left -padx $_2" );

			cmd( "ttk::frame $b.f2.n" );
			cmd( "ttk::label $b.f2.n.l -text \"Number of simulations:\"" );
			cmd( "ttk::label $b.f2.n.w -text \"%d\" -style hl.TLabel", sim.last_run );
			cmd( "pack $b.f2.n.l $b.f2.n.w -side left -padx $_2" );
			cmd( "pack $b.f2.t $b.f2.n" );

			cmd( "ttk::frame $b.f3" );
			cmd( "ttk::label $b.f3.l -text \"Output path\"" );
			cmd( "ttk::label $b.f3.w -text [ fn_break [ file nativename \"%s\" ] 40 ] -justify center -style hl.TLabel", out_dir );
			cmd( "pack $b.f3.l $b.f3.w" );

			cmd( "ttk::frame $b.f4" );
			cmd( "ttk::label $b.f4.l -text \"Results file(s)\"" );

			if ( sim.last_run > 1 )	// multiple runs case
			{
				cmd( "ttk::frame $b.f4.w" );

				cmd( "ttk::frame $b.f4.w.l1" );
				cmd( "ttk::label $b.f4.w.l1.l -text \"from:\"" );
				cmd( "ttk::label $b.f4.w.l1.w -style hl.TLabel -text \"$firstFile.$resExt$zipExt\"" );
				cmd( "pack $b.f4.w.l1.l $b.f4.w.l1.w -side left -padx $_2" );

				cmd( "ttk::frame $b.f4.w.l2" );
				cmd( "ttk::label $b.f4.w.l2.l -text \"to:\"" );
				cmd( "ttk::label $b.f4.w.l2.w -style hl.TLabel -text \"$lastFile.$resExt$zipExt\"" );
				cmd( "pack $b.f4.w.l2.l $b.f4.w.l2.w -side left -padx $_2" );

				cmd( "pack $b.f4.w.l1 $b.f4.w.l2" );
			}
			else				// single run case
				cmd( "ttk::label $b.f4.w -style hl.TLabel -text \"$firstFile.$resExt$zipExt\"" );

			cmd( "pack $b.f4.l $b.f4.w" );

			cmd( "set choice [ expr { ! $no_tot && ( [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] ) } ]", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "" );

			cmd( "ttk::frame $b.f5" );
			cmd( "ttk::label $b.f5.l1 -text \"Totals file (last steps)\"" );
			cmd( "ttk::label $b.f5.l2 -style %s -text \"$totFile.$totExt$zipExt\"", choice ? "hl.TLabel" : "dhl.TLabel" );

			if ( choice )
				cmd( "ttk::label $b.f5.l3 -justify center -text $tot_msg_warn" );
			else
				cmd( "ttk::label $b.f5.l3 -justify center -text \"\n\"" );

			cmd( "pack $b.f5.l1 $b.f5.l2 $b.f5.l3" );

			cmd( "ttk::frame $b.f6" );
			cmd( "ttk::label $b.f6.l -text \"Parallel runs\"" );
			cmd( "ttk::spinbox $b.f6.e -width 5 -from 1 -to %d -justify center -validate focusout -validatecommand { set n %%P; if { [ string is integer -strict $n ] && $n >= 1 } { set cores %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cores; return 0 } } -invalidcommand { bell } -justify center -state %s", param, ( sim.no_tot && sim.last_run > 1 && param > 1 ) ? "normal" : "disabled" );
			cmd( "write_any $b.f6.e $cores" );
			cmd( "pack $b.f6.l $b.f6.e -side left -padx $_2" );

			cmd( "ttk::frame $b.f7" );
			cmd( "ttk::checkbutton $b.f7.nores -text \"Skip generating results files\" -variable no_res -command { \
					if { $no_res && $no_tot } { \
						set no_tot 0; \
						$b.f5.l2 configure -style hl.TLabel; \
						$b.f6.e configure -state disabled; \
						if { [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
							$b.f5.l3 configure -text $tot_msg_warn \
						} else { \
							$b.f5.l3 configure -text \"\n\" \
						} \
					} \
				}", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
			cmd( "ttk::checkbutton $b.f7.notot -text \"Skip generating totals file\" -variable no_tot -command { \
					if { $no_res && $no_tot } { \
						set no_res 0 \
					}; \
					if { ! $no_tot } { \
						$b.f5.l2 configure -style hl.TLabel; \
						$b.f6.e configure -state disabled; \
						if { [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
							$b.f5.l3 configure -text $tot_msg_warn \
						} else { \
							$b.f5.l3 configure -text \"\n\" \
						} \
					} else { \
						if { %d > 1 && %d > 1 } { \
							$b.f6.e configure -state normal \
						}; \
						$b.f5.l2 configure -style dhl.TLabel; \
						$b.f5.l3 configure -text \"\n\" \
					} \
				}", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "", sim.last_run, param );
			cmd( "ttk::checkbutton $b.f7.dozip -text \"Generate zipped files\" -variable dozip -command { \
					if $dozip { \
						set zipExt .gz \
					} else { \
						set zipExt \"\" \
					}; \
					if { $simNum > 1 } { \
						$b.f4.w.l1.w configure -text \"$firstFile.$resExt$zipExt\"; \
						$b.f4.w.l2.w configure -text \"$lastFile.$resExt$zipExt\"; \
					} else { \
						$b.f4.w configure -text \"$firstFile.$resExt$zipExt\"; \
					}; \
					$b.f5.l2 configure -text \"$totFile.$totExt$zipExt\"; \
					if { [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
						$b.f5.l3 configure -text $tot_msg_warn \
					} else { \
						$b.f5.l3 configure -text \"\n\" \
					} \
				}", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
			cmd( "ttk::checkbutton $b.f7.docsv -text \"Comma-separated text format (.csv)\" -variable docsv -command { \
					if $docsv { set resExt csv; set totExt csv } { \
						set resExt res; \
						set totExt tot \
					}; \
					if { $simNum > 1 } { \
						$b.f4.w.l1.w configure -text \"$firstFile.$resExt$zipExt\"; \
						$b.f4.w.l2.w configure -text \"$lastFile.$resExt$zipExt\"; \
					} else { \
						$b.f4.w configure -text \"$firstFile.$resExt$zipExt\"; \
					}; \
					$b.f5.l2 configure -text \"$totFile.$totExt$zipExt\"; \
					if { [ file exists \"%s%s$firstFile.$resExt$zipExt\" ] || [ file exists \"%s%s$totFile.$totExt$zipExt\" ] } { \
						$b.f5.l3 configure -text $tot_msg_warn \
					} else { \
						$b.f5.l3 configure -text \"\n\" \
					} \
				}", out_dir, strlen( out_dir ) > 0 ? "/" : "", out_dir, strlen( out_dir ) > 0 ? "/" : "" );
			cmd( "ttk::checkbutton $b.f7.dobar -text \"Show progress bar in logs\" -variable dobar" );
			cmd( "ttk::checkbutton $b.f7.doover -text \"Clear output path before run\" -variable doover -state %s", overwDir ? "normal" : "disabled" );
			cmd( "ttk::checkbutton $b.f7.tosave -text \"Update configuration file\" -variable overwConf -state %s", overwConf ? "normal" : "disabled" );
			cmd( "pack $b.f7.nores $b.f7.notot $b.f7.dozip $b.f7.docsv $b.f7.dobar $b.f7.doover $b.f7.tosave -anchor w" );

			cmd( "pack $b.f1 $b.f2 $b.f3 $b.f4 $b.f5 $b.f6 $b.f7 -padx $_5 -pady $_5" );

			cmd( "okhelpcancel $b b { set choice 1 } { LsdHelp menurun.html#batch } { set choice 2 }" );

			cmd( "showtop $b" );
			cmd( "mousewarpto $b.b.ok" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set cores [ $b.f6.e get ]" );

			cmd( "destroytop .batch" );

			Tcl_UnlinkVar( interp, "no_res" );
			Tcl_UnlinkVar( interp, "no_tot" );
			Tcl_UnlinkVar( interp, "dobar" );
			Tcl_UnlinkVar( interp, "docsv" );
			Tcl_UnlinkVar( interp, "doover" );
			Tcl_UnlinkVar( interp, "dozip" );
			Tcl_UnlinkVar( interp, "overwConf" );

			if ( choice == 2 )
				break;

			if ( subDir )
				if ( ! create_res_dir( out_dir ) || ! sim.results_alt_path( out_dir ) )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Subdirectory '%s' cannot be created\" -detail \"Check if the path is set READ-ONLY, or move your configuration file to a different location.\"", out_dir );
					break;
				}

			if ( overwDir && doover )
				clean_res_dir( out_dir );

			if ( sim.last_run > 1 && param > 1 && sim.no_tot )	// parallel runs case
			{
				param = std::min( get_int( "cores" ), sim.last_run );
				param = std::min( std::max( param, 1 ), sim.max_threads );// parallel runs
				nature = std::max( sim.max_threads / param, 1 );	// threads per run
			}
			else
			{
				param = 1;
				nature = sim.max_threads;
			}

			for ( n = r; n->up != NULL; n = n->up );
			sim.reset_blueprint( n );		// update blueprint to consider last changes

			if ( overwConf )				// save if needed
			{
				if ( ! save_xml_configuration_gui( ) )
				{
					cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"File '%s.lsd' cannot be saved\" -detail \"Check if the drive or the file is set READ-ONLY, or try to save to a different location.\"", sim.conf_name );
					break;
				}
				else
					unsaved_change( false );// signal no unsaved change
			}

			// start the job
			cmd( "set oldpath [ pwd ]" );
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			plog( "\n\nProcessing parallel background run (threads=%d runs=%d)...", nature, param );
			sim.run_parallel( false, term_exe, sim.conf_name, sim.seed, sim.last_run, nature, param );

			show_logs( sim.conf_path, sim.run_logs, true );

			cmd( "set path $oldpath" );
			cmd( "cd $path" );

		break;


		// import network
		case 88:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and load one before trying to import a network structure file.\"" );
				break;
			}

			cmd( "set bah \"%s\"", sim.conf_name );

			// make sure there is a path set
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			cmd( "set bah [ tk_getOpenFile -parent . -title \"Import Network Structure File\"	 -defaultextension \".net\" -initialdir $path -initialfile \"$bah.net\" -filetypes { { {Pajek network files} {.net} } { {All files} {*} } } ]" );
			choice = 0;
			cmd( "if { [ string length $bah ] > 0 && ! [ fn_spaces \"$bah\" . ] } { \
					set netPath [ file dirname $bah ]; \
					set netFile [ file tail $bah ]; \
					set posExt [ string last . $netFile ]; \
					if { $posExt >= 0 } { \
						set netExt [ string range $netFile [ expr { $posExt + 1 } ] end ]; \
						set netFile [ string range $netFile 0 [ expr { $posExt - 1 } ] ] \
					} { \
						set netExt \"\" \
					} \
				} { \
					set choice 2 \
				}" );

			if ( choice == 2 )
				break;

			lab1 = get_str( "netPath" );
			lab2 = get_str( "netFile" );
			lab3 = get_str( "netExt" );
			if ( strlen( lab2 ) == 0 )
				break;

			// try to read the object name from network file (in first comment line)
			snprintf( lab, MAX_PATH_LENGTH, "%s%s%s%s%s", lab1, foldersep( lab1 ), lab2, strlen( lab3 ) == 0 ? "" : ".", lab3 );
			strcpy( lab_old, "(none)" );			// no object name yet
			if ( ( f = fopen( lab, "r" ) ) )
			{
				fgets( ch, MAX_LINE_SIZE, f );		// get first line
				sscanf( ch, "%% %99s", lab_old );	// get first string after the comment char
				fclose( f );
			}
			else
			{
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid file or directory\" -detail \"Please check if an existing network structure file (Pajek .net format) was selected.\"" );
				break;
			}

			cmd( "set TT .objs" );
			cmd( "newtop $TT \"Import Network\" { set choice 2 }" );

			cmd( "ttk::frame $TT.l" );
			cmd( "ttk::label $TT.l.l -text \"Suggested object:\"" );
			cmd( "ttk::label $TT.l.n -style hl.TLabel -text \"%s\"", lab_old );
			cmd( "pack $TT.l.l $TT.l.n -padx $_2" );

			cmd( "ttk::frame $TT.v" );
			cmd( "ttk::label $TT.v.l -justify center -text \"Object representing\nthe network nodes\"" );

			cmd( "ttk::frame $TT.v.t" );
			cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
			cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
			cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
			cmd( "mouse_wheel $TT.v.t.lb" );
			sim.root->insert_object( "$TT.v.t.lb" );
			cmd( "pack $TT.v.l $TT.v.t" );

			cmd( "pack $TT.l $TT.v -padx $_5 -pady $_5" );

			cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

			cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0; break }" );
			cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end; break }" );
			cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

			cmd( "showtop $TT" );

			cmd( "set cur 0" );
			if ( ! strcmp( lab_old, "(none)" ) )
			{
				if ( r != NULL )
					lsd::strcpyn( lab_old, r->label, MAX_ELEM_LENGTH );
				else
					strcpy( lab_old, "" );
			}
			cmd( "for { set i 0 } { $i < [ $TT.v.t.lb size ] } { incr i } { if [ string equal [ $TT.v.t.lb get $i ] %s ] { set cur $i; break } }", lab_old );
			cmd( "$TT.v.t.lb selection set $cur" );
			cmd( "$TT.v.t.lb see $cur" );
			cmd( "focus $TT.v.t.lb" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set nodeObj [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ]" );
			cmd( "destroytop .objs" );

			if ( choice == 2 )
				break;

			lab4 = get_str( "nodeObj" );

			plog( "\nImporting network on object '%s' from file %s%s%s%s%s...\n", lab4, lab1, foldersep( lab1 ), lab2, strlen( lab3 ) == 0 ? "" : ".", lab3 );

			cur = sim.root->search( lab4 );
			if ( cur != NULL && cur->up != NULL )
			{
				nlinks = cur->up->read_file_net( lab4, lab1, lab2, -1, lab3 );
				if ( nlinks < 0 )
				{
					cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid file or object\" -detail \"Please check the file contents for a valid Pajek network structure file (Pajek .net format) and make sure you select a valid object for attributing the network's nodes role.\"" );
					plog( "\nError: Network file not imported\n" );
				}
				else
				{
					plog( "\n%ld network links imported\n", nlinks );
					redrawRoot = redrawStruc = true;			// force browser/structure redraw
				}
			}
			else
			{
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid object\" -detail \"Please make sure you select a valid object for attributing the network's nodes role.\"" );
				plog( "Error: Network file not imported\n" );
			}

		break;


		// export network
		case 89:

			if ( ! sim.conf_ok || strlen( sim.conf_name ) == 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No configuration loaded\" -detail \"Please load or create and save one before trying to export a network structure file.\"" );
				break;
			}

			cmd( "set TT .objs" );
			cmd( "newtop $TT \"Export Network\" { set choice 2 }" );

			cmd( "ttk::frame $TT.v" );
			cmd( "ttk::label $TT.v.l -justify center -text \"Object containing\nthe network nodes\"" );

			cmd( "ttk::frame $TT.v.t" );
			cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
			cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
			cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
			cmd( "mouse_wheel $TT.v.t.lb" );

			sim.root->insert_object( "$TT.v.t.lb", true );
			cmd( "set numNets [ $TT.v.t.lb size ]" );
			if ( get_int( "numNets" ) == 0 )
			{
				cmd( "destroytop .objs" );
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No network object found\" -detail \"Please make sure there are objects set as network nodes before exporting the network structure.\"" );
				break;
			}

			cmd( "pack $TT.v.l $TT.v.t" );

			cmd( "pack $TT.v -padx $_5 -pady $_5" );

			cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

			cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0; break }" );
			cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end; break }" );
			cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

			cmd( "showtop $TT" );

			cmd( "$TT.v.t.lb selection set 0" );
			cmd( "focus $TT.v.t.lb" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set nodeObj [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ]" );
			cmd( "destroytop .objs" );

			if ( choice == 2 )
				break;

			lab4 = get_str( "nodeObj" );
			cur = sim.root->search( lab4 );
			if ( cur == NULL || cur->node == NULL || cur->up == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid object\" -detail \"Please make sure you select an object which is already a node of an existing network.\"" );
				break;
			}

			// make sure there is a path set
			cmd( "set path \"%s\"", sim.conf_path );
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			cmd( "set bah \"%s-%s\"", sim.conf_name, lab4 );
			cmd( "set bah [ tk_getSaveFile -parent . -title \"Export Network Structure File\" -defaultextension \".net\" -initialdir $path -initialfile \"$bah.net\" -filetypes { { {Pajek network files} {.net} } } ]" );
			choice = 0;
			cmd( "if { [ string length $bah ] > 0 && ! [ fn_spaces \"$bah\" . ] } { \
					set netPath [ file dirname $bah ]; \
					set netFile [ file tail $bah ]; \
					set posExt [ string last . $netFile ]; \
					if { $posExt >= 0 } { \
						set netExt [ string range $netFile [ expr { $posExt + 1 } ] end ]; \
						set netFile [ string range $netFile 0 [ expr { $posExt - 1 } ] ] \
					} { \
						set netExt \"\" \
					} \
				} { \
					set choice 2 \
				}" );

			if ( choice == 2 )
				break;

			lab1 = get_str( "netPath" );
			lab2 = get_str( "netFile" );
			lab3 = get_str( "netExt" );
			if ( strlen( lab2 ) == 0 )
				break;

			plog( "\nExporting network on object '%s' to file %s%s%s%s%s...\n", lab4, lab1, foldersep( lab1 ), lab2, strlen( lab3 ) == 0 ? "" : ".", lab3 );

			nlinks = cur->up->write_file_net( lab4, lab1, lab2, -1 );
			if ( nlinks < 0 )
			{
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid file or object\" -detail \"Please check the chosen directory/file for WRITE access and make sure you select a valid object for retrieving the network's nodes.\"" );
				plog( "\nError: Network file not exported\n" );
			}
			else
				plog( "\n%ld network links exported\n", nlinks );

		break;


		// unload network
		case 93:

			if ( ! sim.conf_ok )
				break;

			if ( ! discard_change( ) )	// check for unsaved configuration changes
				break;

			cmd( "set TT .objs" );
			cmd( "newtop $TT \"Unload Network\" { set choice 2 }" );

			cmd( "ttk::frame $TT.v" );
			cmd( "ttk::label $TT.v.l -justify center -text \"Object containing\nthe network nodes\"" );

			cmd( "ttk::frame $TT.v.t" );
			cmd( "ttk::scrollbar $TT.v.t.v_scroll -command \"$TT.v.t.lb yview\"" );
			cmd( "ttk::listbox $TT.v.t.lb -width 25 -selectmode single -yscroll \"$TT.v.t.v_scroll set\" -dark $darkTheme" );
			cmd( "pack $TT.v.t.lb $TT.v.t.v_scroll -side left -fill y" );
			cmd( "mouse_wheel $TT.v.t.lb" );

			sim.root->insert_object( "$TT.v.t.lb", true );
			cmd( "set numNets [ $TT.v.t.lb size ]" );
			if ( get_int( "numNets" ) == 0 )
			{
				cmd( "destroytop .objs" );
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"No network object found\" -detail \"Please make sure there are objects set as network nodes before unloading the network structure.\"" );
				break;
			}

			cmd( "pack $TT.v.l $TT.v.t" );

			cmd( "pack $TT.v -padx $_5 -pady $_5" );

			cmd( "okcancel $TT b { set choice 1 } { set choice 2 }" );	// insert ok button

			cmd( "bind $TT.v.t.lb <Home> { selectinlist .objs.v.t.lb 0 }" );
			cmd( "bind $TT.v.t.lb <End> { selectinlist .objs.v.t.lb end }" );
			cmd( "bind $TT.v.t.lb <Double-1> { set choice 1 }" );

			cmd( "showtop $TT" );

			cmd( "$TT.v.t.lb selection set 0" );
			cmd( "focus $TT.v.t.lb" );

			choice = 0;
			while ( choice == 0 )
				Tcl_DoOneEvent( 0 );

			cmd( "set nodeObj [ .objs.v.t.lb get [ .objs.v.t.lb curselection ] ]" );
			cmd( "destroytop .objs" );

			if ( choice == 2 )
				break;

			lab4 = get_str( "nodeObj" );
			cur = sim.root->search( lab4 );
			if ( cur == NULL || cur->node == NULL || cur->up == NULL )
			{
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid object\" -detail \"Please make sure you select an object which is already a node of an existing network.\"" );
				break;
			}

			plog( "\nRemoving network on object '%s'\n", lab4 );

			cur->up->delete_net( lab4 );

		break;


		// context-menu operation: execute the command in 'ctxMenuCmd'
		case 95:

			if ( get_str( "ctxMenuCmd" ) == NULL )
				break;

			cmd( "eval $ctxMenuCmd" );					// execute command

			cmd( "unset -nocomplain ctxMenuCmd" );
			redrawRoot = redrawStruc = true;			// force browser/structure redraw

		break;


		// toggle the state of the model structure windows, refresh window
		case 70:

			str_wnd = str_wnd ? 0 : 1;
			cmd( "set strWindowChk $str_wnd" );
			cmd( "if { [ winfo exists .m.model ] } { .m.model entryconfig 15 -indicatoron $strWindowChk }" );
			redrawStruc = true;

			if ( str_wnd )
				cmd( "tooltip::tooltip .bbar.struct \"Hide structure\"" );
			else
				cmd( "tooltip::tooltip .bbar.struct \"Show structure\"" );

		break;


		// refresh structure windows
		case 23:

			redrawStruc = true;

		break;


		// present parallel run log
		case 8:

			// destroy monitor thread
			if ( sim.run_monitor.joinable( ) )
				sim.run_monitor.join( );

			plog( "\n%s\n", sim.run_log.c_str( ) );
			plog( "Finished parallel background run\n" );

		break;

		default:
			plog( "\nWarning: choice %d not recognized", choice );
	}

	choice = 0;
	return r;
}


/*************************************************************
 SENSITIVITY_TOO_LARGE
 *************************************************************/
bool gui::sensitivity_too_large( long numSaPts )
{
	cmd( "set answer [ ttk::messageBox -parent . -type okcancel -icon warning -default cancel -title Warning -message \"Too many cases to perform sensitivity analysis\" -detail \"The required	 number (%ld) of configuration points to perform sensitivity analysis is likely too large to be processed in reasonable time.\n\nPress 'OK' if you want to continue anyway or 'Cancel' to abort the command now.\" ]; switch -- $answer { ok { set choice 0 } cancel { set choice 1 } }", numSaPts );

		return choice;
}


/*************************************************************
 SENSITIVITY_CLEAN
 *************************************************************/
bool gui::sensitivity_clean_dir( const char *path )
{
	cmd( "set answer [ ttk::messageBox -parent . -type yesno -icon info -default yes -title \"Sensitivity Analysis\" -message \"Clean output path before proceeding?\" -detail \"The configuration files (.lsd) for sensitivity analysis will be created at:\n\n[ fn_break [ file nativename \"%s\" ] 40 ]\n\nThis subdirectory already contains LSD produced files. Click on 'Yes' to delete the existing files before proceeding or 'No' to just continue without deleting.\" ]; switch -- $answer { no { set choice 0 } yes { set choice 1 } }", path );

		return choice;
}


/*************************************************************
 SENSITIVITY_CREATED
 *************************************************************/
void gui::sensitivity_created( const char *path, const char *sim_name, int findex )
{
	cmd( "ttk::messageBox -parent . -type ok -icon info -title \"Sensitivity Analysis\" -message \"Configuration files created\" -detail \"LSD has created configuration files (.lsd) for all the sensitivity analysis required points.\n\nTo run the analysis you have to start the processing of sensitivity configuration files by selecting 'Run'/'Create/Run Parallel Batch...' menu option.\n\nAlternatively, open a command prompt (terminal window) and execute the following command in the directory of the model:\n\n> %s -f	 [ fn_break [ file nativename \"%s/%s\" ] 40 ]	-s	%d\"", LSD_TERM, path, sim_name, findex );
}


/*************************************************************
 SENSITIVITY_UNDEFINED
 *************************************************************/
void gui::sensitivity_undefined( void )
{
	cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity analysis items not found\" -detail \"Before using this option you have to select at least one parameter or lagged variable initial value to perform the sensitivity analysis and inform the corresponding values to be explored.\n\nTo set the sensitivity analysis values (or ranges), use the 'Sensitivity Analysis' button in the 'Model'/'Change Element...' menu option (or the corresponding context menu option) and inform the values or range(s) using the syntax explained in the 'Sensitivity Analysis' entry window (it is possible to paste a list of values from the clipboard). You can repeat this procedure for each required parameter or initial value.\n\nSensitivity Analysis values are NOT saved in the standard LSD configuration file (.lsd) and if needed they MUST be saved in a LSD sensitivity analysis file (.sa) using the 'File'/'Save Sensitivity...' menu option.\"" );
}
