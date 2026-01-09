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
 LSDTERM.CPP
 The LSD terminal program entry point.

 This file can be compiled with the command:

  make [-f makefile]

 in the corresponding model directory.

 Relevant macros for conditional compilation (when defined):

 - _EQ_: user model equation file
 - _TERM_: terminal executable
 - _NT_: no signal trapping (better when debugging in GDB)
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes

#define LSD_TERM "lsd_term"			// LSD terminal executable name

int load_config( lsd::simulation & sim );
int parse_cmdline( int argn, const char **argv, lsd::simulation & sim, lsd::assimilation & da );

// objects which must be allocated in the heap
lsd::assimilation da;				// data assimilation object (unused)
lsd::simulation sim;				// single LSD simulation terminal instance

// command line strings
const char lsdCmdMsg[ ] = "This is the terminal version of LSD.";
const char lsdCmdHlp[ ] = "Command line options:\n'-f FILENAME.lsd [-s SEED] [-e RUNS] to run a single configuration file\n'-f FILE_BASE_NAME -s FIRST_NUM [-e LAST_NUM]' for batch sequential mode\n'-o PATH' to save result file(s) to a different subdirectory\n'-l FILENAME' to save all output to a (log) file\n'-t' to produce comma separated (.csv) text result file(s)\n'-r' for skipping the generation of intermediate result file(s)\n'-p' for skipping the generation of totals file\n'-g' for the generation of a single grand total file\n'-z' for preventing the generation of compressed result file(s)\n'-b' for showing a progress bar\n'-c MAX_THREADS[:MAX_RUNS]' to set maximum parallel threads/runs to use\n'-ai CL' to save data assimilation confidence intervals at CL level (%%)\n'-af' to save data assimilation forecasts\n'-ad' to save data assimilation observational data\n'-ac' to save data assimilation covariance/comedian matrix\n";


/*************************************************************
 MAIN
 *************************************************************/
int main( int argn, const char **argv )
{
	int res = -1;

	// initialize LSD library
	lsd::init_lib( & da );

#ifndef _NT_
	// register all signal handlers
	lsd::handle_signals( lsd::signal_handler );

	try
	{
#endif
		// assume exec path is included in file name, use CWD if not
		lsd::set_exec( NULL, argv[ 0 ] );

		if ( lsd::exec_file == NULL || strlen( lsd::exec_file ) == 0 || lsd::exec_path == NULL || strlen( lsd::exec_path ) == 0 )
		{
			fprintf( stderr, "\nInvalid LSD executable name or path.\n%s\nMake sure the LSD directory is not too deep into the disk directory tree (over %d chars).\n\n", lsdCmdMsg, PATH_MAX );
			lsd::lsd_exit( 5 );
		}

		// parse command line options
		res = parse_cmdline( argn, argv, sim, da );
		if ( res != 0 )
			lsd::lsd_exit( res );

		// load configuration
		res = load_config( sim );
		if ( res != 0 )
			lsd::lsd_exit( res );

		// check for data assimilation configuration and run it
		if ( ! da.disable && sim.last_run > 1 && da.count( 4 ) > 0 )
			res = da.run_simulation( 0 );
		else
			// if parallel execution is required, just run new instances & wait to finish
			if ( ! sim.batch_sequential && sim.last_run > 1 && sim.max_runs > 1 )
			{
				if ( sim.grand_total || ! sim.no_tot )
				{
					printf( "\n(Grand) total file(s) request ignored, running in parallel mode.\n" );
					sim.no_tot = true;
					sim.grand_total = false;
				}

				res = sim.run_parallel( true, argv[ 0 ], sim.conf_name, sim.seed, sim.last_run, sim.max_threads, sim.max_runs );
			}
			else
				// execute single simulation
				res = sim.run_simulation( 0, 0, false );

#ifndef _NT_
	}
	catch ( std::bad_alloc& exc )	// out of memory conditions
	{
		lsd::exception_handler( SIGMEM, exc.what( ) );
	}
	catch ( std::exception& exc )	// other known error conditions
	{
		lsd::exception_handler( SIGSTL, exc.what( ) );
	}
	catch ( ... )				// other unknown error conditions
	{
		abort( );				// raises a SIGABRT exception, tell user & close
	}
#endif

	lsd::lsd_exit( res, true );

	return res;
}


/*************************************************************
 PARSE_CMDLINE
 *************************************************************/
int parse_cmdline( int argn, const char **argv, lsd::simulation & sim, lsd::assimilation & da )
{
	int i, j = 0, k = 0;

	// set default/start-up parameters to preserve compatibility
	sim.dozip = true;
	sim.dobar = sim.docsv = sim.no_res = sim.no_tot = sim.grand_total = false;
	sim.findex = -1;						// no default
	sim.fend = 0;							// no file number limit

	if ( argn < 3 )
	{
		fprintf( stderr, "\nNo configuration to run.\n%s\n%s\n", lsdCmdMsg, lsdCmdHlp );
		return 5;
	}

	for ( i = 1; i < argn; i += 2 )
	{
		// read -f parameter : file name or base name
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'f' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
		{
			delete [ ] sim.conf_name;
			sim.conf_name = new char [ strlen( argv[ 1 + i ] ) + 1 ];
			strcpy( sim.conf_name, argv[ 1 + i ] );
			continue;
		}
		// read -o parameter : change the path for the output of result files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'o' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
		{
			sim.results_alt_path( argv[ 1 + i ] );
			continue;
		}
		// read -l parameter : save all output to a (log) file
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'l' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
		{
			delete [ ] sim.log_file;
			sim.log_file = new char [ strlen( argv[ 1 + i ] ) + 1 ];
			strcpy( sim.log_file, argv[ 1 + i ] );
			continue;
		}
		// read -c parameter : max number of cores
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'c' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
		{
			sscanf( argv[ i + 1 ], "%d:%d", &j, &k );
			continue;
		}
		// read -s parameter : first sequential file to process
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 's' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
		{
			sscanf( argv[ i + 1 ], "%d", & sim.findex );
			continue;
		}
		// read -e parameter : last sequential file to process
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'e' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
		{
			sscanf( argv[ i + 1 ], "%d", & sim.fend );
			continue;
		}
		// read -t parameter : produce .csv text results files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 't' )
		{
			i--;					// no parameter for this option
			sim.docsv = true;
			continue;
		}
		// read -r parameter : do not produce intermediate .res files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'r' )
		{
			i--;					// no parameter for this option
			sim.no_res = true;
			continue;
		}
		// read -p parameter : do not produce totals .tot files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'p' )
		{
			i--;					// no parameter for this option
			sim.no_tot = true;
			continue;
		}
		// read -g parameter : create grand total file (batch only)
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'g' )
		{
			i--;					// no parameter for this option
			sim.grand_total = true;
			printf( "\nGrand total file requested ('-g'), don't run another instance of '%s' in this folder!\n", LSD_TERM );
			continue;
		}
		// read -z parameter : don't create compressed result files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'z' )
		{
			i--;					// no parameter for this option
			sim.dozip = false;
			continue;
		}
		// read -b parameter : show a progress bar
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'b' )
		{
			i--;					// no parameter for this option
			sim.dobar = true;
			continue;
		}
		// read -ai parameter : save assimilation confidence intervals
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'a' && argv[ i ][ 2 ] == 'i' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
		{
			sscanf( argv[ i + 1 ], "%lf", & da.conf_lev );
			continue;
		}
		// read -af parameter : save assimilation forecast
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'a' && argv[ i ][ 2 ] == 'f' )
		{
			i--;					// no parameter for this option
			da.sav_fct = true;
			continue;
		}
		// read -ad parameter : save assimilation data
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'a' && argv[ i ][ 2 ] == 'd' )
		{
			i--;					// no parameter for this option
			da.sav_obs = true;
			continue;
		}
		// read -ac parameter : save assimilation covariance/comedian matrix
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'a' && argv[ i ][ 2 ] == 'c' )
		{
			i--;					// no parameter for this option
			da.sav_dsp = true;
			continue;
		}

		fprintf( stderr, "\nOption '%c%c' not recognized.\n%s\n%s\n", argv[ i ][ 0 ], argv[ i ][ 1 ], lsdCmdMsg, lsdCmdHlp );
		return 6;
	}

	if ( k > 0 )
		sim.max_runs = std::min( k, sim.max_threads );
	else
	{
		sim.max_runs = 1;

		if ( j > 0 )
			sim.max_threads = j;
	}

	if ( sim.max_runs > 1 )
		sim.max_threads = std::max( std::min( j, sim.max_threads / sim.max_runs ), 1 );

	return 0;
}


/*************************************************************
 LOAD_CONFIGURATION
 *************************************************************/
int load_config( lsd::simulation & sim )
{
	char *str;
	FILE *f;

	if ( sim.conf_name == NULL || strlen( sim.conf_name ) == 0 )
	{
		fprintf( stderr, "\nOption '-f' required, no configuration file(s).\n%s\n%s\n", lsdCmdMsg, lsdCmdHlp );
		return 6;
	}

	str = new char [ strlen( sim.conf_name ) + 1 ];
	strcpy( str, sim.conf_name );
	lsd::strupr( str );

	if ( strstr( str, ".LSD" ) == NULL )
	{
		sim.batch_sequential = true;

		if ( sim.findex < 0 || sim.fend < 0 || sim.fend < sim.findex )
		{
			fprintf( stderr, "\nInvalid -s and/or -e values.\n%s\n%s\n", lsdCmdMsg, lsdCmdHlp );
			return 6;
		}

		sim.conf_file = new char [ strlen( sim.conf_name ) + ( int ) log10( sim.findex ) + 7 ];
		sprintf( sim.conf_file, "%s_%d.lsd", sim.conf_name, sim.findex );
	}
	else
	{
		sim.batch_sequential = false;
		sim.conf_file = new char [ strlen( sim.conf_name ) + 1 ];
		strcpy( sim.conf_file, sim.conf_name );
		sim.conf_name[ strstr( str, ".LSD" ) - str ] = '\0';
	}

	delete [ ] str;

	if ( ( f = fopen( sim.conf_file, "r" ) ) == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\nThis is the terminal version of LSD.\nSpecify a -f FILENAME.lsd to run a simulation or -f FILE_BASE_NAME -s 1 for\nbatch sequential simulation mode (requires configuration files:\nFILE_BASE_NAME_1.lsd, FILE_BASE_NAME_2.lsd, etc).\n\n", sim.conf_file );
		return 7;
	}

	fclose( f );

	if ( sim.load_configuration( true, NULL, 1 ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\nThis is the terminal version of LSD.\nCheck if the file is a valid LSD configuration or regenerate it using the\nLSD Browser.\n\n", sim.conf_file );
		return 8;
	}

	if ( ! sim.batch_sequential )
	{
		if ( sim.findex > 0 )
			sim.seed = sim.findex;

		if ( sim.fend > 0 )
			sim.last_run = sim.fend;
	}

	if ( sim.log_file != NULL )
	{
		if ( sim.save_alt && strncmp( sim.log_file, sim.alt_path, strlen( sim.alt_path ) ) != 0 )
		{
			str = sim.log_file;
			sim.log_file = new char [ strlen( sim.alt_path ) + strlen( str ) + 2 ];
			sprintf( sim.log_file, "%s/%s", sim.alt_path, str );
			delete [ ] str;
		}

		if ( ( sim.log_file_ptr = fopen( sim.log_file , "w+" ) ) == NULL )
			printf( "\nCannot create log file '%s', using stdout.\n", sim.log_file );
		else
		{
			lsd::stdout_ptr = lsd::stderr_ptr = sim.log_file_ptr;
			dup2( fileno( sim.log_file_ptr ), STDOUT_FILENO );
			dup2( fileno( sim.log_file_ptr ), STDERR_FILENO );
		}
	}

	return 0;
}
