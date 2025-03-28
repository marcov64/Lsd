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

 - void simulation::run_simulation( int until_t, int until_run )
 Run the loaded simulation model. Running is not only the actual
 simulation run, but also the initialization of result files. Of
 course, it has also to manage the messages from user and from
 the model at run time.

 - bool object::alloc_save_mem( );
 Prepare variables to store saved data.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 RUN_SIMULATION (data assimilation)
 *************************************************************/
int lsd::assimilation::run_simulation( int until_t )
{
	clock_t start, last_update = clock( );
	int nstale, res = 0;

	// initialize data assimilation data structures
	if ( sims.size( ) == 0 || ! init( sims[ 0 ] ) )
		return 1;

	ref_sim->run = 1;
	until_t = until_t > 0 && until_t < ref_sim->last_t ? until_t : ref_sim->last_t;

	// cover browser & open run time plot window
#ifndef _TERM_
	if ( ref_sim->liblnk->runtime_start != NULL )
		ref_sim->liblnk->runtime_start( true );

	if ( ref_sim->liblnk->runtime_run_start != NULL )
		ref_sim->liblnk->runtime_run_start( true );

	if ( ref_sim->liblnk->enable_plot != NULL )
		ref_sim->liblnk->enable_plot( );
#endif

	ref_sim->plog( "\nData assimilation running (threads=%d)...", ref_sim->last_run );

	// control execution time
	start = clock( );

	// do the data assimilation forecast-analysis cycle
	for ( auto & dtime : time_var )
	{
		// stop if data time span is longer than simulation
		if ( ( next_t = dtime.first ) > until_t )
			break;

		// DA forecast step
		if ( ( nstale = dispatch_runs( run_sims, next_t, 1, true ) ) > 0 )
		{
			res = 3;
			break;
		}

		// DA analysis step
		if ( ( res = analysis( dtime.second, next_t ) ) != 0 )
			break;

#ifndef _TERM_
		// update run-time plot
		if ( ref_sim->liblnk->runtime_step != NULL )
			ref_sim->liblnk->runtime_step( true );

		// handle runtime button pressings after progress bar update
		if ( ref_sim->liblnk->progress_bar != NULL )
			ref_sim->liblnk->progress_bar( next_t, last_update );

		if ( ref_sim->liblnk->runtime_buttons != NULL && ( res = ref_sim->liblnk->runtime_buttons( ) ) != 0 )
			break;
#endif
	}

	// run remaining pure forecast periods, if any
	if ( res == 0 && next_t < until_t )
		if ( ( nstale = dispatch_runs( run_sims, until_t, 1, false ) ) > 0 )
			res = 3;

	if ( res == 0 )
	{
		for ( auto & sim : run_sims )
			if ( sim.eff_t != until_t )
				res = 4;

		ref_sim->eff_t = until_t;
		ref_sim->t = until_t + 1;
	}
	else
		ref_sim->eff_t = ref_sim->t = next_t;
	// close data assimilation run-time data structures
	finish( );

	ref_sim->plog( "\nData assimilation %s at time step %d (%.2f sec.)\n", ref_sim->quit == 2 ? "stopped" : "finished", ref_sim->t - 1, ( float ) ( clock( ) - start ) / CLOCKS_PER_SEC );

#ifndef _TERM_
	if ( ref_sim->liblnk->runtime_run_end != NULL )
		ref_sim->liblnk->runtime_run_end( );

	if ( ref_sim->liblnk->runtime_end != NULL )
		ref_sim->liblnk->runtime_end( );
#else
	ref_sim->save_results( true );
#endif

	return res;
}


/*************************************************************
 DISPATCH_RUNS
 *************************************************************/
int lsd::dispatch_runs( sim_vecT & run_sims, int until_t, int until_run, bool da_en )
{
	int nstale, nrun = 0;
	mtxT mtx;
	uniq_lT lock( mtx );

	for ( auto & sim : run_sims )
		if ( ! sim.sim_thread.joinable( ) && sim.conf_ok )
		{
			sim.sim_thread = thrT ( & lsd::simulation::run_simulation, & sim, until_t, until_run, da_en );
			sim.last_dispatch_time = sim.stale_time = 0;
			++nrun;
		}

	do
	{
		auto start = std::chrono::system_clock::now( );
		lsd::seq_end.wait_until( lock, start + std::chrono::seconds( MAX_SIM_SLEEP ) );

		nstale = 0;
		for ( auto & sim : run_sims )
		{
			if ( sim.sim_thread.joinable( ) && ! sim.running_seq && sim.eff_t > 0 )
			{
				sim.sim_thread.join( );
				--nrun;
			}
			else
				if ( sim.eff_t > sim.last_dispatch_time )
				{
					sim.last_dispatch_time = sim.eff_t;
					sim.stale_time = 0;
				}
				else
				{
					auto elapsed = std::chrono::duration_cast < std::chrono::seconds > ( std::chrono::system_clock::now( ) - start );
					sim.stale_time += elapsed.count( );
				}

			if ( sim.stale_time > MAX_STEP_TIMEOUT )
				++nstale;
		}
	}
	while ( nstale < nrun );

	return nstale;
}


/*************************************************************
 RUN_SIMULATION (regular)
 *************************************************************/
int lsd::simulation::run_simulation( int until_t, int until_run, bool da_en )
{
	int res = 0;
	static char bar_done[ 2 * BAR_DONE_SIZE ];
	static clock_t start_mc, start_run, last_update = clock( );
	static int perc_done, last_done;

	if ( ( until_run > 0 && until_run < run ) || ( until_t > 0 && until_t <= t &&
		 ( until_run <= 0 || ( until_run > 0 && until_run < run ) ) ) )
		goto end_run;				// already there, nothing to do

	if ( ! running_seq )			// if not already running sequential run set
		if ( ( res = init_new_seq( start_mc, bar_done, perc_done, last_done, da_en ) ) != 0 )
			goto end_run;

	// start loop controlling set of sequential simulation runs
	for ( ; quit != 2 && run <= last_run; ++run )
	{
		if ( ! running )			// if not already running single run
			if ( ( res = init_new_run( start_run, last_update, da_en ) ) != 0 )
				goto end_run;

		// start loop controlling a single simulation run
		for ( ; quit == 0 && t <= last_t; ++t )
		{
			// update the percentage done bar, if needed
			if ( dobar && liblnk != NULL )
			{
				update_bar( bar_done, perc_done, last_done, 2 * BAR_DONE_SIZE );
				perc_done = std::min( ( int ) ( 100 * ( ( run - 1 ) + ( double ) t / last_t ) / last_run ), 100 );
			}

#ifndef _TERM_
			// only update if simulation not paused
			if ( liblnk == NULL || liblnk->runtime_step == NULL || liblnk->runtime_step( false ) )
#endif
			{
				eff_t = t;
				root->update( true, false );// simulation step execution
			}

			// collect state variables if in data assimilation
			if ( da_en && t == da->next_t )
				da_svars.save_state_vars( root );

#ifndef _TERM_
			// handle runtime button pressings after progress bar update
			if ( liblnk != NULL && liblnk->progress_bar != NULL && liblnk->runtime_buttons != NULL )
			{
				liblnk->progress_bar( t, last_update );
				liblnk->runtime_buttons( );
			}
#endif
			// check if time to pause run (don't pause at last step)
			if ( until_t > 0 && t >= until_t && t + 1 <= last_t )
			{
				res = -2;			// interrupt
				goto end_run;
			}
		}	// end of time step

		// run user closing function, reporting error appropriately
		user_exception = true;
		_close_sim_( );
		::close_sim( );
		user_exception = false;
		running = false;

		// adjust simulation data to early stops and save variables to file
		root->reset_end( );

		if ( ! da_en )
		{
			if ( liblnk != NULL && liblnk->deb_log != NULL )
				liblnk->deb_log( false, 0 );// close debug log file, if any

			if ( dobar && on_bar && liblnk != NULL )
				update_bar( bar_done, perc_done, last_done, 2 * BAR_DONE_SIZE );

			if ( fast_mode < 2 )
				plog( "\nSimulation %d of %d %s at time step %d (%.2f sec.)\n", run, last_run, quit == 2 ? "stopped" : "finished", t - 1, ( float ) ( clock( ) - start_run ) / CLOCKS_PER_SEC );
		}

		if ( quit == 1 )			// multiple simulation runs need to reset quit
			quit = 0;

#ifndef _TERM_
		if ( liblnk != NULL && liblnk->runtime_run_end != NULL )
			liblnk->runtime_run_end( );
#endif

		if ( quit != 2 && ( last_run > 1 || liblnk == NULL || liblnk->runtime_run_end == NULL ) )
		{
			save_results( );		// save results for multiple runs, if any

			if ( run == last_run )	// last run?
				if ( ! next_batch( ) )// prepare next batch configuration, if any
					break;			// nothing else, finish
		}

		// check if time to pause run (don't pause at last run)
		if ( until_run > 0 && run >= until_run && run + 1 <= last_run )
		{
			res = -1;				// interrupt
			goto end_run;
		}
	}	// end of run

	if ( ! da_en && fast_mode == 2 )
		plog( "\nFinished processing configuration file(s) (%.2f sec.)\n", ( float ) ( clock( ) - start_mc ) / CLOCKS_PER_SEC );

#ifndef _TERM_
	if ( liblnk != NULL && liblnk->runtime_end != NULL )
		liblnk->runtime_end( );
#endif

	end_run:

	// set of sequential runs is finished
	quit = 0;						// ensure no error to handle
	running_seq = false;

	// stop multi-thread workers
	delete [ ] workers;
	workers = NULL;

	// wake dispatcher lock
	l_guardT lock( seq_end_lck );
	seq_end.notify_one( );

	return res;
}


/*************************************************************
 INIT_NEW_SEQ
 *************************************************************/
int lsd::simulation::init_new_seq( clock_t & start, char *bar_done, int & perc_done, int & last_done, bool da_en )
{
	int i;

	run = 1;					// first run in the sequence
	quit = 0;					// not marked for abortion

	// check if there are parallel computing variables
	if ( parallel_disable || max_threads < 2 )
		parallel_mode = parallel_ready = false;
	else
	{
		parallel_mode = root->search_parallel( );
		parallel_ready = true;
	}

	// start multi-thread workers
	if ( parallel_mode )
	{
		workers = new worker[ max_threads ];
		for ( i = 0; i < max_threads; ++i )
			workers[ i ].worker_thread = thrT( & lsd::worker::cal_worker, & workers[ i ] );
		}

	if ( ! da_en )
	{
#ifndef _TERM_
		if ( liblnk != NULL && liblnk->runtime_start != NULL )
			liblnk->runtime_start( false );
#else
		plog( "\nProcessing configuration file %s...\n", clean_file( conf_file ) );
#endif
		set_fast( 0 );				// should start on OBSERVE and switch to FAST later
	}
	else
		set_fast( 2 );

	res_list.clear( );			// empty list of saved results files
	strcpy( res_path, "" );		// and clear last saved path to results files

	// prepare progress bar
	on_bar = false;
	perc_done = 0;
	last_done = -1;
	strcpy( bar_done, "" );

	running_seq = true;

	// control execution time
	start = clock( );

	return 0;
}


/*************************************************************
 INIT_NEW_RUN
 *************************************************************/
int lsd::simulation::init_new_run( clock_t & start, clock_t & last_update, bool da_en )
{
	int i;

	t = 1;                  // first time step of run
	eff_t = 0;				// no steps performed yet
	save_ok = true;			// valid structure to save
#ifndef _TERM_
	if ( liblnk != NULL && liblnk->runtime_run_start != NULL )
		liblnk->runtime_run_start( false );
#endif
	if ( ! da_en && fast_mode < 2 )
	{
		if ( parallel_mode )
			plog( "\nSimulation %d of %d running (seed=%d threads=%d)...", run, last_run, seed, max_threads );
		else
			plog( "\nSimulation %d of %d running (seed=%d)...", run, last_run, seed );
	}

	// if new batch configuration file, reload all except descriptions
	if ( batch_loop )
	{
		batch_loop = false;
		i = load_configuration( true, NULL, 1 );
	}
	else
		// if just another run seed, reload just structure & parameters
		if ( run > 1 )
			i = load_configuration( true, NULL, 2 );
		else
			i = 0;			// use loaded configuration

	// abort if configuration cannot be loaded
	if ( i != 0 )
	{
#ifndef _TERM_
		if ( ! da_en )
		{
			if ( liblnk != NULL && liblnk->log_tcl_error != NULL )
				liblnk->log_tcl_error( true, "Load configuration", "Configuration file not found or corrupted" );

			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be reloaded\" -detail \"Check if LSD still has WRITE access to the configuration file '%s'.\nLSD will close now.\"", conf_file );
		}
#else
		fprintf( stderr, "\nFile '%s' not found or corrupted.\n", conf_file );
#endif
		return 10;
	}

	// pre-allocate memory to save all existing elements for the entire simulation
	running = true;
	series_saved = 0;
	if ( ! root->alloc_save_mem( ) )
	{
#ifndef _TERM_
		if ( ! da_en )
		{
			if ( liblnk != NULL && liblnk->log_tcl_error != NULL )
				liblnk->log_tcl_error( true, "Memory allocation", "Not enough memory, too many series saved for the memory available" );

			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Not enough memory\" -detail \"Too many series saved for the available memory. Memory insufficient for %d series over %d time steps. Reduce series to save and/or time steps.\nLSD will close now.\"", series_saved, last_t );
		}
#else
		fprintf( stderr, "\nNot enough memory. Too many series saved for the memory available.\nMemory insufficient for %d series over %d time steps.\nReduce series to save and/or time steps.\n", series_saved, last_t );
#endif
		return 11;
	}

	// build initial object list for user pointer checking
	if ( ! no_ptr_chk )
		build_obj_list( true );

	// reset cemetery, trace stack and simulation control
	empty_cemetery( );
	empty_stack( );
	stack_info = 0;
	error_hard_thread = false;
	worker_ready = true;
	worker_crashed = false;
	wait_delete = NULL;

	// new random routine' initialization
	init_random( seed );
	seed++;

	// reset math error counters and defaults
	init_math_error( );
	use_nan = false;
	no_search = false;
	no_search_up = false;

	// control execution time
	start = last_update = clock( );

	return 0;
}


/*************************************************************
 SAVE_RESULTS
 *************************************************************/
void lsd::simulation::save_results( bool da_en )
{
	char *path_out, *name_out, sep_out[ 2 ], fname[ MAX_PATH_LENGTH ];
	result *rf;				// pointer for results files (may be zipped or not)

	if ( series_saved == 0 )
	{
		if ( fast_mode < 2 )
			plog( "Nothing to save: no element selected\n" );

		return;
	}

	// remove existing path, if any, from name in case of alternative output path
	char *alt_name = clean_file( conf_name );

	if ( save_alt )
	{
		path_out = alt_path;
		name_out = alt_name;
	}
	else
	{
		path_out = conf_path;
		name_out = conf_name;
	}

	if ( strlen( path_out ) == 0 )
		strcpy( sep_out, "" );
	else
		strcpy( sep_out, "/" );

	if ( ! no_res )
	{
		if ( da_en )
			snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_da_%d_%d.%s", path_out, sep_out, name_out, seed, seed + last_run - 1, docsv ? "csv" : "res" );
		else
		if ( ! batch_sequential )
			snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_%d.%s", path_out, sep_out, name_out, seed - 1, docsv ? "csv" : "res" );
		else
			snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_%d_%d.%s", path_out, sep_out, name_out, findex, seed - 1, docsv ? "csv" : "res" );

		if ( dozip )
			strcatn( fname, ".gz", MAX_PATH_LENGTH );

		res_list.push_back( fname );

		if ( fast_mode < 2 )
			plog( "Saving results to file %s... ", fname );

		rf = new result( fname, "wt", this, dozip, docsv );// create results file object
		rf->title( root, 1 );						// write header
		rf->data( root, 0, eff_t );					// write all data
		delete rf;									// close file and delete object

		if ( fast_mode < 2 )
			plog( "Done\n" );
	}

	if ( ! da_en && ! no_tot && ( ( liblnk != NULL && liblnk->runtime_run_end != NULL ) || max_runs == 1 ) )
	{
		if ( ! grand_total || batch_sequential )	// generate partial total files?
		{
			if ( ! batch_sequential )
			  snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_%d_%d.%s", path_out, sep_out, name_out, seed - run, seed - 1 + last_run - run, docsv ? "csv" : "tot" );
			else
			  snprintf( fname, MAX_PATH_LENGTH, "%s%s%s_%d_%d_%d.%s", path_out, sep_out, name_out, findex, seed - run, seed - 1 + last_run - run, docsv ? "csv" : "tot" );
		}
		else										// generate single grand total file
		{
			snprintf( fname, MAX_PATH_LENGTH, "%s%s%s.%s", path_out, sep_out, name_out, docsv ? "csv" : "tot" );
		}

		if ( dozip )
			strcatn( fname, ".gz", MAX_PATH_LENGTH );

		if ( fast_mode < 2 && run == last_run )		// print only for last
			plog( "\nSaving totals to file %s... ", fname );

		if ( run == 1 && grand_total && ! add_to_tot )
		{
			rf = new result( fname, "wt", this, dozip, docsv );// create results file object
			rf->title( root, 0 );					// write header
		}
		else
			rf = new result( fname, "a", this, dozip, docsv );// add results object to existing file

		rf->data( root, eff_t );					// write current data data
		delete rf;									// close file and delete object

		if ( fast_mode < 2 && run == last_run )		// print only for last
			plog( "Done\n" );
	}

	if ( run == last_run )							// last run?
		strcpyn( res_path, path_out, MAX_PATH_LENGTH );
		
	delete [ ] alt_name;
}


/*************************************************************
 NEXT_BATCH
 *************************************************************/
bool lsd::simulation::next_batch( void )
{
	char fname[ MAX_PATH_LENGTH ];
	FILE *f;

	if ( batch_sequential )			// last batch file?
	{
		// try reading next file
		snprintf( fname, MAX_PATH_LENGTH, "%s_%d.lsd", conf_name, ++findex );
		delete [ ] conf_file;
		conf_file = new char[ strlen( fname ) + 1 ];
		strcpy( conf_file, fname );
		f = fopen( conf_file, "r" );

		if ( f == NULL || ( fend != 0 && findex > fend ) )// no more file to process
		{
			if ( f != NULL )
				fclose( f );

			if ( fast_mode < 2 )
				plog( "\nFinished processing %s\n", clean_file( conf_file ) );

			return false;
		}

		plog( "\nProcessing configuration file %s...\n", clean_file( conf_file ) );
		fclose( f );				// process next file

		run = 0;					// force restarting run count
		batch_loop = true;			// force reloading configuration

		return true;
	}
#ifdef _TERM_
	else
		if ( fast_mode < 2 )
			plog( "\nFinished processing %s\n", clean_file( conf_file ) );
#endif
	return false;
}


/*************************************************************
 SET_FAST
 *************************************************************/
void lsd::simulation::set_fast( int level )
{
	if ( level > 2 )
		level = 2;
	if ( level < 0 )
		level = 0;

#ifndef _TERM_
	if ( level == 0 )
	{
		if ( liblnk != NULL && liblnk->enable_plot != NULL )
			liblnk->enable_plot( );
	}
	else
		if ( liblnk != NULL && liblnk->disable_plot != NULL )
			liblnk->disable_plot( );
#endif

	// remove the variables stack when switching to any fast mode
	if ( fast_mode == 0 && level > 0 )
	{
		if ( deb_t > 0 || stack_info > 0 || prof_aggr_time )
		{
			plog( "\nWarning: %s is active, fast mode command ignored", deb_t > 0 ? "debugging" : "profiling" );
			return;
		}

		empty_stack( );

		if ( liblnk != NULL && liblnk->deb_log != NULL )
			liblnk->deb_log( false, 0 );
	}

	if ( this == sims[ 0 ] && fast_mode < 2 && level == 2 )
		plog( "\n" );

	fast_mode = level;
	fast = ( level == 0 ) ? false : true;
}


/*************************************************************
 EMPTY_STACK
 *************************************************************/
void lsd::simulation::empty_stack( void )
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
		stack_log->n = 0;
		stack_log->v = NULL;
		stack_level = 0;
	}
	else
	{
#ifndef _TERM_
		if ( liblnk != NULL && liblnk->log_tcl_error != NULL )
			liblnk->log_tcl_error( false, "Internal error", "LSD trace stack corrupted" );

		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Internal LSD error\" -detail \"The LSD trace stack is corrupted.\nLSD will close now.\"" );
#else
		fprintf( stderr, "\nLSD trace stack corrupted.\n" );
#endif
		lsd_exit( 28 );
	}
}


/*************************************************************
 ALLOC_SAVE_MEM
 *************************************************************/
bool lsd::object::alloc_save_mem( void )
{
	int i;
	bridge *cb;
	object *cur;
	variable *cv;

	// for each variable set the data saving support
	for ( cv = v; cv != NULL; cv = cv->next )
	{
		if ( cv->attr->num_lag > 0 || cv->param == 1 )
		{
			if ( ! cv->attr->initialized )
			{
				sim->error_hard( "required initialization values missing",
								 "select the object and choose menu 'Data'/'Initial Values'",
								 false,
								 "%s '%s' in object '%s' has not been initialized",
								 cv->param == 1 ? "parameter" : "variable", cv->attr->label, attr->label );
				goto error;
			}

			// ensure variable constraints are respected
			for ( i = 0; i < ( cv->param == 1 ? 1 : cv->attr->num_lag ); ++i )
				cv->val[ i ] = cv->chk_val( cv->val[ i ] );
		}

		cv->last_update = 0;

		// choose next update step for special updating variables
		if ( cv->attr->delay > 0 || cv->attr->delay_range > 0 )
		{
			cv->next_update = cv->attr->delay;
			if ( cv->attr->delay_range > 0 )
				cv->next_update += sim->rnd_int( 0, cv->attr->delay_range );
		}

		if ( cv->attr->save || cv->attr->savei )
			if ( ! cv->alloc_save_var( ) )
				goto error;

#ifndef _TERM_
		// variable to parent name map for AoR (only in GUI mode)
		if ( sim->liblnk != NULL )
			sim->par_map.insert( std::make_pair < strT, strT > ( cv->attr->label, attr->label ) );
#endif
	}

	for ( cb = b; cb != NULL; cb = cb->next )
		for ( cur = cb->head; cur != NULL && sim->quit != 2; cur = BROTHER( cur ) )
			if ( ! cur->alloc_save_mem( ) )
				goto error;

	return true;

	error:

	sim->quit = 2;

	return false;
}


/*************************************************************
 ALLOC_SAVE_VAR
 *************************************************************/
bool lsd::variable::alloc_save_var( void )
{
	if ( ! up->sim->running )
	{
		data = NULL;
		start = end = 0;
		return true;
	}

	if ( attr->num_lag > 0 || param == 1 )
		start = up->sim->t - 1;
	else
		start = up->sim->t;

	end = up->sim->last_t;

	// use C stdlib to be able to deallocate memory for deleted objects
	free( data );
	data = ( double * ) malloc( ( end - start + 1 ) * sizeof( double ) );

	if( data == NULL )
	{
		raise( SIGMEM );
		return false;
	}
	else
	{
		if ( attr->num_lag > 0 || param == 1 )
			data[ 0 ] = val[ 0 ];

		++( up->sim->series_saved );
		return true;
	}
}


/*************************************************************
 RESET_END
 *************************************************************/
void lsd::object::reset_end( void )
{
	bridge *cb;
	object *cur;
	variable *cv;

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		if ( cv->attr->save )
			cv->end = sim->eff_t;

		if ( cv->attr->savei == 1 )
			cv->save_single( );
	}

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		cur = cb->head;
		if ( cur != NULL && cur->to_compute )
			for ( ; cur != NULL; cur = BROTHER( cur ) )
				cur->reset_end( );
	}
}


/*************************************************************
 UPDATE_BAR
 *************************************************************/
void lsd::simulation::update_bar( char *bar, int done, int & last_done, int bar_sz )
{
	char perc[ MAX_ELEM_LENGTH ];
	int p;

	done = std::min ( done, 100 );
	last_done = std::min ( last_done, 100 );

	if ( nsim != 0 || done <= last_done || last_done == 100 )
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


/*************************************************************
 RESULTS_ALT_PATH
 simple tool to allow changing
 where results are saved.
 *************************************************************/
bool lsd::simulation::results_alt_path( const char *altPath )
{
	if ( save_alt )
	{
		delete [ ] alt_path;
		alt_path = NULL;
	}

	if ( altPath == NULL || strlen( altPath ) == 0 )
	{
		save_alt = false;
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
			save_alt = true;
			return true;
		}
	}

	delete [ ] alt_path;
	alt_path = NULL;
	save_alt = false;

	plog( "\nWarning: could not open results directory '%s', ignoring.\n", altPath );

	return false;
}
