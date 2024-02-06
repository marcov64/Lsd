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
RUN.CPP
Contains the code to control the simulation run.

The main functions contained here are:

- void run( )
Run the loaded simulation model. Running is not only the actual
simulation run, but also the initialization of result files. Of
course, it has also to manage the messages from user and from the
model at run time.

- bool alloc_save_mem( );
Prepare variables to store saved data.
*************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*********************************
RUN
*********************************/
int run( void )
{
	bool batch_sequential_loop = false;
	char *path_out = NULL, *name_out, sep_out[ 2 ], fname[ MAX_PATH_LENGTH ], bar_done[ 2 * BAR_DONE_SIZE ];
	int i, perc_done, last_done;
	FILE *f;
	clock_t start, end, last_update;
	result *rf;				// pointer for results files (may be zipped or not)

#ifndef _NP_
	// check if there are parallel computing variables
	if ( parallel_disable || max_threads < 2 )
		parallel_mode = parallel_ready = false;
	else
	{
		parallel_mode = search_parallel( root );
		parallel_ready = true;
	}

	// start multi-thread workers
	if ( parallel_mode )
		workers = new worker[ max_threads ];
#else
	if ( search_parallel( root ) )
		plog( "\nWarning: parallel mode is not supported under current configuration\n" );
	parallel_mode = false;
#endif

#ifndef _NW_
	prof.clear( );			// reset profiling times

	if ( liblnk.cover_browser != NULL )
		liblnk.cover_browser( "Running...", "Use the buttons to control the simulation:\n\n'Stop' :  aborts the simulation\n'Pause' / 'Resume' :  pauses and resumes the simulation\n'Fast' :	accelerates the simulation by hiding information\n'Observe' :  presents more run-time information\n'Debug' :  triggers the debugger at flagged variables", true );
#else
	plog( "\nProcessing configuration file %s...\n", clean_file( struct_file ) );
#endif

	set_fast( 0 );			// should always start on OBSERVE and switch to FAST later
	res_list.clear( );		// empty list of saved results files
	strcpy( path_res, "" );	// and clear last saved path to results files

	// prepare progress bar
	on_bar = false;
	perc_done = 0;
	last_done = -1;
	strcpy( bar_done, "" );

	for ( i = 1, quit = 0; i <= sim_num && quit != 2; ++i )
	{
		running = true;		// signal simulation is running
		cur_sim = i;		// update the current run in the set of runs
		actual_steps = 0;	// no steps performed yet
		save_ok = true;		// valid structure to save

		empty_cemetery( );	// ensure that previous data are not erroneously mixed

#ifndef _NW_
		par_map.clear( );	// restart variable to parent name map for AoR

		if ( liblnk.prepare_plot != NULL )
			liblnk.prepare_plot( root, i );
#endif
		if ( fast_mode < 2 )
		{
			if ( parallel_mode )
				plog( "\nSimulation %d of %d running (seed=%d threads=%d)...", i, sim_num, seed, max_threads );
			else
				plog( "\nSimulation %d of %d running (seed=%d)...", i, sim_num, seed );
		}

		// if new batch configuration file, reload all except descriptions
		if ( batch_sequential_loop )
		{
			if ( load_configuration( true, NULL, 1 ) != 0 )
			{
#ifndef _NW_
				if ( liblnk.log_tcl_error != NULL )
					liblnk.log_tcl_error( true, "Load configuration", "Configuration file not found or corrupted" );

				cmd_gui( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be loaded\" -detail \"Check if LSD still has WRITE access to the configuration file '%s'.\nLSD will close now.\"", struct_file );
#else
				fprintf( stderr, "\nFile '%s' not found or corrupted.\n", struct_file );
#endif
				return 10;
			}
			batch_sequential_loop = false;
		}

		// if just another run seed, reload just structure & parameters
		if ( i > 1 )
			if ( load_configuration( true, NULL, 2 ) != 0 )
			{
#ifndef _NW_
				if ( liblnk.log_tcl_error != NULL )
					liblnk.log_tcl_error( true, "Load configuration", "Configuration file not found or corrupted" );

				cmd_gui( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be reloaded\" -detail \"Check if LSD still has WRITE access to the configuration file '%s'.\nLSD will close now.\"", struct_file );
#else
				fprintf( stderr, "\nFile '%s' not found or corrupted.\n", struct_file );
#endif
				return 10;
			}

		// build initial object list for user pointer checking
		if ( ! no_ptr_chk )
			build_obj_list( true );

		series_saved = 0;
		t = 1;

		if ( ! alloc_save_mem( root ) )
		{
#ifndef _NW_
			if ( liblnk.log_tcl_error != NULL )
				liblnk.log_tcl_error( true, "Memory allocation", "Not enough memory, too many series saved for the memory available" );

			cmd_gui( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Not enough memory\" -detail \"Too many series saved for the available memory. Memory insufficient for %d series over %d time steps. Reduce series to save and/or time steps.\nLSD will close now.\"", series_saved, max_step );
#else
			fprintf( stderr, "\nNot enough memory. Too many series saved for the memory available.\nMemory insufficient for %d series over %d time steps.\nReduce series to save and/or time steps.\n", series_saved, max_step );
#endif
			return 11;
		}

		// reset trace stack
		empty_stack( );

		// new random routine' initialization
		init_random( seed );

		// reset math error counters
		init_math_error( );

		seed++;
		error_hard_thread = false;
		worker_ready = true;
		worker_crashed = false;
		wait_delete = NULL;
		stack_info = 0;
		use_nan = false;
		no_search = false;
		no_search_up = false;
		start = last_update = clock( );

		for ( t = 1; quit == 0 && t <= max_step; ++t )
		{
			// update the percentage done bar, if needed
			if ( liblnk.cmd_backend == NULL && dobar )
				update_bar( bar_done, perc_done, last_done, 2 * BAR_DONE_SIZE );

#ifndef _NW_
			// only update if simulation not paused
			if ( liblnk.runtime_buttons == NULL || liblnk.runtime_step( t ) )
#endif
			{
				actual_steps = t;
				root->update( true, false );
			}

			perc_done = min( 100 * ( ( i - 1 ) + ( double ) t / max_step ) / sim_num, 100 );

#ifndef _NW_
			// handle runtime button pressings
			if ( liblnk.runtime_buttons != NULL )
				liblnk.runtime_buttons( i, t, last_update );
#endif
		}	// end of t

		unsavedData = true;			// flag unsaved simulation results
		running = false;
		end = clock( );

		if ( liblnk.deb_log != NULL )
			liblnk.deb_log( false, 0 );// close debug log file, if any

		if ( dobar && on_bar )
			update_bar( bar_done, perc_done, last_done, 2 * BAR_DONE_SIZE );

		if ( fast_mode < 2 )
			plog( "\nSimulation %d of %d %s at case %d (%.2f sec.)\n", i, sim_num, quit == 2 ? "stopped" : "finished", t - 1, ( float ) ( end - start ) / CLOCKS_PER_SEC );

		if ( quit == 1 )			// for multiple simulation runs you need to reset quit
			quit = 0;

#ifndef _NW_
		cmd_gui( ".p.b1.b configure -value %d", cur_sim );
		cmd_gui( ".p.b1.i configure -text \"Simulation: %d of %d ([ expr { int( 100 * %d / %d ) } ]%% done)\"", min( cur_sim + 1, sim_num ), sim_num, cur_sim, sim_num	);

		cmd_gui( "destroytop .deb" );
		cmd_gui( "update" );
#endif
		// run user closing function, reporting error appropriately
		user_exception = true;
		close_sim( );
		user_exception = false;

		reset_end( root );

		if ( quit != 2 && ( sim_num > 1 || liblnk.cmd_backend == NULL ) )
		{
			// save results for multiple simulation runs, if any
			if ( series_saved > 0 )
			{	// remove existing path, if any, from name in case of alternative output path
				char *alt_name = clean_file( simul_name );

				if ( save_alt_path )
				{
					path_out = alt_path;
					name_out = alt_name;
				}
				else
				{
					path_out = conf_path;
					name_out = simul_name;
				}

				if ( strlen( path_out ) == 0 )
					strcpy( sep_out, "" );
				else
					strcpy( sep_out, "/" );

				if ( ! no_res )
				{
					if ( ! batch_sequential )
						snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_%d.%s", path_out, sep_out, name_out, seed - 1, docsv ? "csv" : "res" );
					else
						snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_%d_%d.%s", path_out, sep_out, name_out, findex, seed - 1, docsv ? "csv" : "res" );

					if ( dozip )
						strcatn( fname, ".gz", MAX_PATH_LENGTH );

					res_list.push_back( fname );

					if ( fast_mode < 2 )
						plog( "Saving results to file %s... ", fname );

					rf = new result( fname, "wt", dozip, docsv );	// create results file object
					rf->title( root, 1 );						// write header
					rf->data( root, 0, actual_steps );			// write all data
					delete rf;									// close file and delete object

					if ( fast_mode < 2 )
						plog( "Done\n" );
				}

				if ( ! no_tot && ( liblnk.cmd_backend != NULL || max_runs == 1 ) )
				{
					if ( ! grandTotal || batch_sequential )		// generate partial total files?
					{
						if ( ! batch_sequential )
						  snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_%d_%d.%s", path_out, sep_out, name_out, seed - i, seed - 1 + sim_num - i, docsv ? "csv" : "tot" );
						else
						  snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_%d_%d_%d.%s", path_out, sep_out, name_out, findex, seed - i, seed - 1 + sim_num - i, docsv ? "csv" : "tot" );
					}
					else										// generate single grand total file
					{
						snprintf( fname, MAX_PATH_LENGTH, "%s%s%s.%s", path_out, sep_out, name_out, docsv ? "csv" : "tot" );
					}

					if ( dozip )
						strcatn( fname, ".gz", MAX_PATH_LENGTH );

					if ( fast_mode < 2 && i == sim_num )		// print only for last
						plog( "\nSaving totals to file %s... ", fname );

					if ( i == 1 && grandTotal && ! add_to_tot )
					{
						rf = new result( fname, "wt", dozip, docsv );// create results file object
						rf->title( root, 0 );					// write header
					}
					else
						rf = new result( fname, "a", dozip, docsv );// add results object to existing file

					rf->data( root, actual_steps );				// write current data data
					delete rf;									// close file and delete object

					if ( fast_mode < 2 && i == sim_num )		// print only for last
						plog( "Done\n" );
				}

				if ( i == sim_num )								// last run?
					strcpyn( path_res, path_out, MAX_PATH_LENGTH );
			}
			else
				if ( fast_mode < 2 )
					plog( "Nothing to save: no element selected\n" );

			if ( i == sim_num )									// last run?
			{
				if ( batch_sequential )							// last batch file?
				{
					findex++;									// try next file
					snprintf( fname, MAX_PATH_LENGTH, "%s_%d.lsd", simul_name, findex );
					delete [ ] struct_file;
					struct_file = new char[ strlen( fname ) + 1 ];
					strcpy( struct_file, fname );
					f = fopen( struct_file, "r" );
					if ( f == NULL || ( fend != 0 && findex > fend ) )// no more file to process
					{
						if ( f != NULL )
							fclose( f );
						if ( fast_mode < 2 )
							plog( "\nFinished processing %s\n", clean_file( struct_file ) );
						break;
					}

					plog( "\nProcessing configuration file %s...\n", clean_file( struct_file ) );
					fclose( f );								// process next file

					i = 0;										// force restarting run count
					batch_sequential_loop = true;				// force reloading configuration
				}
#ifdef _NW_
				else
					if ( fast_mode < 2 )
						plog( "\nFinished processing %s\n", clean_file( struct_file ) );
#endif
			}
		}
	}	// end of run

	if ( fast_mode == 2 )
		plog( "\nFinished processing configuration file(s)\n" );

#ifndef _NW_
	if ( liblnk.reset_plot != NULL )
		liblnk.reset_plot( );

	if ( liblnk.uncover_browser != NULL )
		liblnk.uncover_browser( );

	if ( liblnk.show_prof_aggr != NULL )
		liblnk.show_prof_aggr( );

	cmd_gui( "focustop .log" );
#endif

#ifndef _NP_
	// stop multi-thread workers
	delete [ ] workers;
	workers = NULL;
#endif

	quit = 0;

	return 0;
}


/*********************************
SET_FAST
*********************************/
void set_fast( int level )
{
	if ( level > 2 )
		level = 2;
	if ( level < 0 )
		level = 0;

#ifndef _NW_
	if ( level == 0 )
	{
		if ( liblnk.enable_plot != 0 )
			liblnk.enable_plot( );
	}
	else
		if ( liblnk.disable_plot != 0 )
			liblnk.disable_plot( );
#endif

	// remove the variables stack when switching to any fast mode
	if ( fast_mode == 0 && level > 0 )
	{
		if ( when_debug > 0 || stack_info > 0 || prof_aggr_time )
		{
			plog( "\nWarning: %s is active, fast mode command ignored", when_debug > 0 ? "debugging" : "profiling" );
			return;
		}

		empty_stack( );

		if ( liblnk.deb_log != NULL )
			liblnk.deb_log( false, 0 );
	}

	if ( fast_mode < 2 && level == 2 )
		plog( "\n" );

	fast_mode = level;
	fast = ( level == 0 ) ? false : true;
}


/*********************************
EMPTY_STACK
*********************************/
void empty_stack( void )
{
	if ( stack_log != NULL )
	{
		// remove stack allocation
		while ( stack_log->prev != NULL )
		{
			lsdstack *cur_stack = stack_log;
			stack_log = stack_log->prev;
			delete cur_stack;
		}
		// prepare for next run
		stack_log->next = NULL;
		stack_log->ns = 0;
		stack_log->vs = NULL;
		stack_level = 0;
	}
	else
	{
#ifndef _NW_
		if ( liblnk.log_tcl_error != NULL )
			liblnk.log_tcl_error( false, "Internal error", "LSD trace stack corrupted" );

		cmd_gui( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Internal LSD error\" -detail \"The LSD trace stack is corrupted.\nLSD will close now.\"" );
#else
		fprintf( stderr, "\nLSD trace stack corrupted.\n" );
#endif
		lsd_exit( 28 );
	}
}


/*********************************
ALLOC_SAVE_MEM
*********************************/
bool alloc_save_mem( object *r )
{
	int toquit = quit;
	bridge *cb;
	object *cur;
	variable *cv;

	// for each variable set the data saving support
	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( ( cv->num_lag > 0 || cv->param == 1 ) && ! cv->initialized )
		{
			error_hard( "required initialization values missing",
						"select the object and choose menu 'Data'/'Initial Values'",
						false,
						"%s '%s' in object '%s' has not been initialized", cv->param == 1 ? "parameter" : "variable", cv->label, r->label );
			toquit = 2;
		}

		cv->last_update = 0;

		// choose next update step for special updating variables
		if ( cv->delay > 0 || cv->delay_range > 0 )
		{
			cv->next_update = cv->delay;
			if ( cv->delay_range > 0 )
				cv->next_update += rnd_int( 0, cv->delay_range );
		}

		if ( cv->save || cv->savei )
			alloc_save_var( cv );

#ifndef _NW_
		// variable to parent name map for AoR
		par_map.insert( make_pair < string, string > ( cv->label, r->label ) );
#endif
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
		for ( cur = cb->head; cur != NULL && quit != 2; cur = go_brother( cur ) )
			alloc_save_mem( cur );

	if ( quit != 2 )
		quit = toquit;

	return ! no_more_memory;
}


/*********************************
ALLOC_SAVE_VAR
*********************************/
bool alloc_save_var( variable *v )
{
	bool prev_state = no_more_memory;

	if ( ! running )
		return true;

	if ( ! no_more_memory )
	{
		if ( v->num_lag > 0 || v->param == 1 )
			v->start = t - 1;
		else
			v->start = t;

		v->end = max_step;

		// use C stdlib to be able to deallocate memory for deleted objects
		free( v->data );
		v->data = ( double * ) malloc( ( v->end - v->start + 1 ) * sizeof( double ) );

		if( v->data == NULL )
		{
			no_more_memory = true;
			v->save = v->savei = false;
			v->start = v->end = 0;

			if ( no_more_memory != prev_state )
			{
				set_lab_tit( v );
				plog( "\nWarning: cannot allocate memory for saving '%s %s' (object '%s')\n Subsequent series will not be saved\n", v->label, v->lab_tit, v->up->label );
			}
		}
		else
		{
			if ( v->num_lag > 0	 || v->param == 1 )
				v->data[ 0 ] = v->val[ 0 ];

			++series_saved;
		}
	}
	else
		v->save = v->savei = false;

	return ! no_more_memory;
}


/*********************************
RESET_END
*********************************/
void reset_end( object *r )
{
	bridge *cb;
	object *cur;
	variable *cv;

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( cv->save )
			cv->end = t - 1;
		if ( cv->savei == 1 )
			save_single( cv );
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		cur = cb->head;
		if ( cur != NULL && cur->to_compute )
			for ( ; cur != NULL; cur = go_brother( cur ) )
				reset_end( cur );
	}
}


/*********************************
RESULTS_ALT_PATH
simple tool to allow changing where results are saved.
*********************************/
bool results_alt_path( const char *altPath )
{
	if ( save_alt_path )
	{
		delete [ ] alt_path;
		alt_path = NULL;
	}

	if ( strlen( altPath ) == 0 )
	{
		save_alt_path = false;
		return false;
	}

	alt_path = new char[ strlen( altPath ) + 1 ];
	if ( sprintf( alt_path, "%s", altPath ) > 0 )
	{
		int lstChr = strlen( alt_path ) - 1;
		if ( alt_path[ lstChr ] == '\\' || alt_path[ lstChr ] == '/' )
			alt_path[ lstChr ] = '\0';

		struct stat sb;
		if ( stat( alt_path, &sb ) == 0 && S_ISDIR( sb.st_mode ) )
		{
			save_alt_path = true;
			return true;
		}
	}

	delete [ ] alt_path;
	alt_path = NULL;
	save_alt_path = false;

	plog( "\nWarning: could not open results directory '%s', ignoring.\n", altPath );

	return false;
}


/*********************************
UPDATE_BAR
*********************************/
void update_bar( char *bar, int done, int & last_done, int bar_sz )
{
	char perc[ MAX_ELEM_LENGTH ];
	int p;

	done = min ( done, 100 );
	last_done = min ( last_done, 100 );

	if ( done <= last_done || last_done == 100 )
		return;

	for ( p = last_done + 1; p <= done; ++p )
		if ( p % 10 == 0 )
		{
			snprintf( perc, MAX_ELEM_LENGTH, "%d%%", p );

			if ( bar != NULL )
				strcatn( bar, perc, bar_sz );

			// check if continuing existing bar or starting a new one
			if ( on_bar || bar == NULL )
				plog_tag( "%s", "bar", perc );
			else
			{
				on_bar = true;
				plog_tag( "\n%s", "bar", bar );
			}
		}
		else
			if ( p % ( 100 / ( BAR_DONE_SIZE - 33 ) ) == 0 )
			{
				if ( bar != NULL )
					strcatn( bar, ".", bar_sz );

				if ( on_bar || bar == NULL )
					plog( ".", "bar" );
				else
				{
					on_bar = true;
					plog_tag( "\n%s", "bar", bar );
				}
			}

	last_done = done;
}
