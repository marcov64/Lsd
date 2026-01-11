/*************************************************************

	LSD 9.0 - January 2026
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 PARALLEL.CPP
 Contains the functions to run models is parallel using the
 operating system to manage the runs.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 RUN_PARALLEL_EXEC
 *************************************************************/
void lsd::simulation::run_parallel_exec( bool term, int id, strT cmd )
{
	int res;

	res = run_system( cmd.c_str( ), this, id );

	l_guardT lock( run_status_lck );
	run_status[ id ] = res;
}


/*************************************************************
 RUN_PARALLEL
 *************************************************************/
#define INISTAT -1234
int lsd::simulation::run_parallel( bool term, const char *exec, const char *simname, int fseed, int runs, int thrrun, int parruns )
{
	char *alt_name, *def_path;
	int i, j, k, num, sl;

	if ( nsim != 0 )						// only first sim object can run OS parallel
		return -1;

	if ( strlen( conf_path ) > 0 )
		def_path = conf_path;
	else
		def_path = exec_path;

	int path_len = save_alt ? strlen( alt_path ) : strlen( def_path );
	int name_len = strlen( simname ) + ( int ) log10( fseed + runs ) + 2;
	int dest_len = path_len + 5;
	int log_len = path_len + name_len + 6;
	int res_len = path_len + name_len + 9;
	int cmd_len = strlen( exec ) + 2 * ( path_len + name_len ) + 50;
	char dest_path[ dest_len ], log_file[ log_len ], res_file[ res_len ], cmd[ cmd_len ];

	alt_name = clean_file( simname );

	if ( save_alt )
		snprintf( dest_path, path_len + 5, " -o %s", alt_path );
	else
		strcpy( dest_path, "" );

	run_logs.clear( );
	run_pids.clear( );
	run_status.clear( );
	run_threads.clear( );
	run_results.clear( );
	parallel_abort = false;

	if ( runs > parruns )				// more than one run per thread?
	{
		num = runs / parruns;			// base number of cases per thread
		sl = runs % parruns;			// remaining cases per thread

		// allocate runs by thread
		for ( i = fseed, j = 1; j <= parruns; ++j )
		{
			// log file name
			snprintf( log_file, log_len, "%s%s%s_%d.log", save_alt ? alt_path : def_path, strlen( save_alt ? alt_path : def_path ) > 0 ? "/" : "", save_alt ? alt_name : simname, j );
			run_logs.push_back( log_file );

			// results file names
			for ( k = i; k < i + num + ( j <= sl ? 1 : 0 ); ++k )
			{
				snprintf( res_file, res_len, "%s%s%s_%d.%s", save_alt ? alt_path : def_path, strlen( save_alt ? alt_path : def_path ) > 0 ? "/" : "", save_alt ? alt_name : simname, k, docsv ? "csv" : "res" );

				if ( dozip )
					strcatn( res_file, ".gz", res_len );

				if ( ! no_res )
					run_results.push_back( res_file );
			}

			// command line
			snprintf( cmd, cmd_len, "%s -c %d -f %s.lsd -s %d -e %d%s%s%s%s%s%s -l %s", exec, thrrun, simname, i, j <= sl ? num + 1 : num, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", dobar ? " -b" : "", dest_path, log_file );

			run_pids.resize( run_pids.size( ) + 1 );
			run_status.push_back( INISTAT );
			run_threads.push_back( thrT ( & lsd::simulation::run_parallel_exec, this, term, run_status.size( ) - 1, strT( cmd ) ) );

			j <= sl ? i += num + 1 : i += num;
		}
	}
	else								// just one run per thread
	{
		for ( i = fseed, j = 1; i < fseed + runs; ++i, ++j )
		{
			// log file name
			snprintf( log_file, log_len, "%s%s%s_%d.log", save_alt ? alt_path : def_path, strlen( save_alt ? alt_path : def_path ) > 0 ? "/" : "", save_alt ? alt_name : simname, i );
			run_logs.push_back( log_file );

			// results file name
			snprintf( res_file, res_len, "%s%s%s_%d.%s", save_alt ? alt_path : def_path, strlen( save_alt ? alt_path : def_path ) > 0 ? "/" : "", save_alt ? alt_name : simname, i, docsv ? "csv" : "res" );

			if ( dozip )
				strcatn( res_file, ".gz", res_len );

			if ( ! no_res )
				run_results.push_back( res_file );

			// command line
			snprintf( cmd, cmd_len, "%s -c %d -f %s.lsd -s %d -e 1%s%s%s%s%s%s -l %s", exec, thrrun, simname, i, no_res ? " -r" : "", no_tot ? " -p" : "", docsv ? " -t" : "", dozip ? "" : " -z", dobar ? " -b" : "", dest_path, log_file );

			run_pids.resize( run_pids.size( ) + 1 );
			run_status.push_back( INISTAT );
			run_threads.push_back( thrT( & lsd::simulation::run_parallel_exec, this, term, run_status.size( ) - 1, strT( cmd ) ) );
		}
	}

	if ( term )
	{
		// create an overall progress bar, using the average progress of threads
		if ( dobar )
		{
			bool abort = false;
			sl = -1;

			printf( "\n" );

			do
			{
				msleep( 1000 );

				num = monitor_logs( );
				if ( num < 0 )
				{
					num = - num;
					abort = true;
				}

				if ( sims.size( ) > 0 )
					update_bar( NULL, num, sl, 2 * BAR_DONE_SIZE );
			}
			while ( num < 100 && ! abort );

			printf( "\n" );
		}

		for ( auto & thr : run_threads )
			if ( thr.joinable( ) )
				thr.join( );

		log_parallel( term );

		i = 0;
		for ( int status : run_status )
			if ( status != 0 )
			{
				i = status;
				break;
			}

		return i;
	}
	else
		run_monitor = thrT( & lsd::simulation::monitor_parallel, this, term );

	return 0;
}


/*************************************************************
 MONITOR_LOGS
 *************************************************************/
int lsd::simulation::monitor_logs( void )
{
	int i, j, k, last, len, thr, threads, n = 0, finished = 0, sum = 0;
	char *log = NULL, tok[ 4 ];
	FILE *f;

	// check if threads are still running
	threads = run_status.size( );
	for ( thr = 0; thr < threads; ++thr )
		if ( run_status[ thr ] != INISTAT )
			++finished;

	thr = 0;
	for ( strT & logn : run_logs )
	{
		// consider just running threads except if all threads are stopped
		if ( run_status[ thr++ ] != INISTAT && finished < threads )
			continue;

		if ( ( f = fopen( logn.c_str( ), "rb" ) ) == NULL )
			continue;

		// read file content at once
		fseek( f, 0, SEEK_END );
		len = ftell( f );
		log = new char [ len + 1 ];
		rewind( f );
		fread( ( void * ) log, sizeof log[ 0 ], len, f );
		log[ len ] = '\0';
		fclose( f );

		for ( i = len - 1; i >= 0; --i )	// move backwards in the log
			if ( strstr( log + i, "\n0%" ) != NULL )  // it is start of bar?
			{
				last = strrchr( log + i, '%' ) - ( log + i );	// end of bar
				for ( j = last; j > 0 && ( log + i )[ j ] != '.'; --j ); // last n% in bar

				if ( j > 0 )		// ignore the first '0%' in bar
				{
					++j;
					strncpy( tok, log + i + j, std::min( last - j, 3 ) );
					tok[ std::min( last - j, 3 ) ] = '\0';
					if ( sscanf( tok, "%d", & k ) == 1 )
					{
						if ( finished < threads )
						{
							sum += k;
							++n;
						}
						else		// all threads stopped, pick the more advanced
						{
							sum = std::max( sum, k );
							n = 1;
						}
					}
				}

				break;				// just consider last bar in log
			}

		delete [ ] log;
	}

	// no thread running, signal it
	if ( finished == threads )
		return n == 0 ? -1 : - sum / n;
	else
		return n == 0 ? 0 : sum / n;	// rounded-down average
}


/*************************************************************
 STOP_PARALLEL
 *************************************************************/
#define WAIT_SECS 5
bool lsd::simulation::stop_parallel( void )
{
	int id, res = 0, secs = 0;

	parallel_abort = true;

	if ( ! parallel_monitor )
		return true;

	for ( id = 0; id < ( int ) run_pids.size( ); ++id )
		res += kill_system( this, id );

	if ( res < ( int ) run_pids.size( ) )
		return false;

	while ( parallel_monitor && secs++ < WAIT_SECS )
		msleep( 1000 );

	for ( strT & results : run_results )
		remove( results.c_str( ) );

	for ( strT & log : run_logs )
		remove( log.c_str( ) );

	run_results.clear( );
	run_logs.clear( );

	if ( parallel_monitor )
		return false;

	if ( run_monitor.joinable( ) )
		run_monitor.join( );

#ifndef _TERM_
	plog( "\nParallel background run aborted!\n" );
#endif

	return true;
}


/*************************************************************
 DETACH_PARALLEL
 *************************************************************/
void lsd::simulation::detach_parallel( void )
{
	parallel_abort = true;

	for ( auto & thr : run_threads )
		if ( thr.joinable( ) )
			thr.detach( );

	if ( run_monitor.joinable( ) )
		run_monitor.detach( );
}


/*************************************************************
 MONITOR_PARALLEL
 *************************************************************/
void lsd::simulation::monitor_parallel( bool term )
{
	parallel_monitor = true;

	for ( auto & thr : run_threads )
		if ( thr.joinable( ) )
			thr.join( );

	log_parallel( term );

	parallel_monitor = false;
}


/*************************************************************
 LOG_PARALLEL
 Consolidate a set of parallel-run logs
 *************************************************************/
void lsd::simulation::log_parallel( bool term )
{
	char buf[ MAX_LINE_SIZE ];
	FILE *f;

	run_log.clear( );

	if ( parallel_abort )
		return;
	else
	{
		l_guardT lock( run_logs_lck );

		for ( strT & log : run_logs )
		{
			f = fopen( log.c_str( ), "r" );
			if ( f == NULL )
			{
				snprintf( buf, MAX_LINE_SIZE, "\nCannot read '%s', consolidated log is incomplete", log.c_str( ) );
				run_log.append( buf );
				continue;
			}

			while ( fgets( buf, MAX_LINE_SIZE, f ) != NULL )
				run_log.append( buf );

			fclose( f );
			remove( log.c_str( ) );
		}

		run_logs.clear( );
	}

	if ( term )
	{
		puts( run_log.c_str( ) );
		return;
	}

#ifndef _TERM_
	while ( ! idle_loop )
		msleep( 100 );

	res_list = run_results;

	if ( liblnk != NULL )
		*( liblnk->choice ) = 8;
#endif

}
