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
LSDNW.CPP
The LSD No Window (terminal) program entry point.

This file can be compiled with the command:

 make -f makefileNW

in the corresponding model directory.

Relevant macros for conditional compilation (when defined):

- _FUN_: user model equation file
- _NW_: No Window executable
- _NP_: no parallel (multi-task) processing
- _NT_: no signal trapping (better when debugging in GDB)
*************************************************************/

#include "lib/libLSD.h"				// LSD library classes

int load_config( simulation & sim );
int parse_cmdline( int argn, const char **argv, simulation & sim );

const char lsdCmdMsg[ ] = "This is the No Window version of LSD.";
const char lsdCmdHlp[ ] = "Command line options:\n'-f FILENAME.lsd [-s SEED] [-e RUNS] to run a single configuration file\n'-f FILE_BASE_NAME -s FIRST_NUM [-e LAST_NUM]' for batch sequential mode\n'-o PATH' to save result file(s) to a different subdirectory\n'-l FILENAME' to save all output to a (log) file\n'-t' to produce comma separated (.csv) text result file(s)\n'-r' for skipping the generation of intermediate result file(s)\n'-p' for skipping the generation of totals file\n'-g' for the generation of a single grand total file\n'-z' for preventing the generation of compressed result file(s)\n'-b' for showing a progress bar\n'-c MAX_THREADS[:MAX_RUNS]' to set maximum parallel threads/runs to use\n";


/*************************************
 MAIN
 *************************************/
int main( int argn, const char **argv )
{
	char cwd[ PATH_MAX ];
	int res = -1;
	simulation sim;					// single LSD simulation terminal instance


#ifndef _NT_

	// register all signal handlers
	handle_signals( signal_handler );

	try
	{

#endif

		// set executable name and path
		getcwd( cwd, PATH_MAX );
		set_exec( cwd, argv[ 0 ] );

		if ( exec_file == NULL || exec_path == NULL )
		{
			fprintf( stderr, "\nInvalid LSD executable name or path.\n%s\nMake sure the LSD directory is not too deep into the disk directory tree (over %d chars).\n\n", lsdCmdMsg, PATH_MAX );
			lsd_exit( 5 );
		}

		// parse command line options
		res = parse_cmdline( argn, argv, sim );
		if ( res != 0 )
			lsd_exit( res );

		// load configuration
		res = load_config( sim );
		if ( res != 0 )
			lsd_exit( res );

#ifndef _NP_

		// if parallel execution is required, just run new instances & wait to finish
		if ( ! batch_sequential && sim.last_run > 1 && max_runs > 1 )
		{
			if ( grandTotal || ! no_tot )
			{
				printf( "\n(Grand) total file(s) request ignored, running in parallel mode.\n" );
				no_tot = true;
				grandTotal = false;
			}

			res = sim.run_parallel( true, argv[ 0 ], sim.conf_name, sim.seed, sim.last_run, max_threads, max_runs );
		}
		else

#endif
			// execute single simulation
			res = sim.run_simulation( );

#ifndef _NT_

	}
	catch ( std::bad_alloc& exc )	// out of memory conditions
	{
		exception_handler( SIGMEM, exc.what( ) );
	}
	catch ( std::exception& exc )	// other known error conditions
	{
		exception_handler( SIGSTL, exc.what( ) );
	}
	catch ( ... )				// other unknown error conditions
	{
		abort( );				// raises a SIGABRT exception, tell user & close
	}

#endif

	lsd_exit( res );
	return res;
}


/*********************************
 PARSE_CMDLINE
 *********************************/
int parse_cmdline( int argn, const char **argv, simulation & sim )
{
	int i, j = 0, k = 0;

	// set default/start-up parameters to preserve compatibility
	dozip = true;
	dobar = docsv = no_res = no_tot = grandTotal = false;
	findex = -1;							// no default
	fend = 0;								// no file number limit

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
			sim.conf_name = new char[ strlen( argv[ 1 + i ] ) + 1 ];
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
			sim.log_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
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
			sscanf( argv[ i + 1 ], "%d", & findex );
			continue;
		}
		// read -e parameter : last sequential file to process
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'e' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
		{
			sscanf( argv[ i + 1 ], "%d", & fend );
			continue;
		}
		// read -t parameter : produce .csv text results files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 't' )
		{
			i--;					// no parameter for this option
			docsv = true;
			continue;
		}
		// read -r parameter : do not produce intermediate .res files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'r' )
		{
			i--;					// no parameter for this option
			no_res = true;
			continue;
		}
		// read -p parameter : do not produce totals .tot files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'p' )
		{
			i--;					// no parameter for this option
			no_tot = true;
			continue;
		}
		// read -g parameter : create grand total file (batch only)
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'g' )
		{
			i--;					// no parameter for this option
			grandTotal = true;
			printf( "\nGrand total file requested ('-g'), don't run another instance of 'lsdNW' in this folder!\n" );
			continue;
		}
		// read -z parameter : don't create compressed result files
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'z' )
		{
			i--;					// no parameter for this option
			dozip = false;
			continue;
		}
		// read -b parameter : show a progress bar
		if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'b' )
		{
			i--;					// no parameter for this option
			dobar = true;
			continue;
		}

		fprintf( stderr, "\nOption '%c%c' not recognized.\n%s\n%s\n", argv[ i ][ 0 ], argv[ i ][ 1 ], lsdCmdMsg, lsdCmdHlp );
		return 6;
	}

#ifndef _NP_

	if ( k > 0 )
		max_runs = min( k, max_threads );
	else
	{
		max_runs = 1;

		if ( j > 0 )
			max_threads = j;
	}

	if ( max_runs > 1 )
		max_threads = max( min( j, max_threads / max_runs ), 1 );

#else

	if ( k != 0 )
		printf( "\nMulti-run request ignored, running in sequential mode.\n" );

#endif

	return 0;
}


/*********************************
 LOAD_CONFIGURATION
 *********************************/
int load_config( simulation & sim )
{
	char *str;
	FILE *f;

	str = new char[ strlen( sim.conf_name ) + 1 ];
	strcpy( str, sim.conf_name );
	strupr( str );

	if ( strlen( str ) == 0 )
	{
		fprintf( stderr, "\nOption '-f' required, no configuration file(s).\n%s\n%s\n", lsdCmdMsg, lsdCmdHlp );
		return 6;
	}

	if ( strstr( str, ".LSD" ) == NULL )
	{
		batch_sequential = true;

		if ( findex < 0 || fend < 0 || fend < findex )
		{
			fprintf( stderr, "\nInvalid -s and/or -e values.\n%s\n%s\n", lsdCmdMsg, lsdCmdHlp );
			return 6;
		}

		sim.conf_file = new char[ strlen( sim.conf_name ) + ( int ) log10( findex ) + 7 ];
		sprintf( sim.conf_file, "%s_%d.lsd", sim.conf_name, findex );
	}
	else
	{
		batch_sequential = false;
		sim.conf_file = new char[ strlen( sim.conf_name ) + 1 ];
		strcpy( sim.conf_file, sim.conf_name );
		sim.conf_name[ strstr( str, ".LSD" ) - str ] = '\0';
	}

	delete [ ] str;

	if ( ( f = fopen( sim.conf_file, "r" ) ) == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\nThis is the no window version of LSD.\nSpecify a -f FILENAME.lsd to run a simulation or -f FILE_BASE_NAME -s 1 for\nbatch sequential simulation mode (requires configuration files:\nFILE_BASE_NAME_1.lsd, FILE_BASE_NAME_2.lsd, etc).\n\n", sim.conf_file );
		return 7;
	}

	fclose( f );

	if ( sim.load_configuration( true, NULL, 1 ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\nThis is the no window version of LSD.\nCheck if the file is a valid LSD configuration or regenerate it using the\nLSD Browser.\n\n", sim.conf_file );
		return 8;
	}

	if ( ! batch_sequential )
	{
		if ( findex > 0 )
			sim.seed = findex;

		if ( fend > 0 )
			sim.last_run = fend;
	}

	if ( sim.log_file != NULL )
	{
		if ( sim.save_alt && strncmp( sim.log_file, sim.alt_path, strlen( sim.alt_path ) ) != 0 )
		{
			str = sim.log_file;
			sim.log_file = new char[ strlen( sim.alt_path ) + strlen( str ) + 2 ];
			sprintf( sim.log_file, "%s/%s", sim.alt_path, str );
			delete [ ] str;
		}

		if ( ( f = fopen( sim.log_file , "w+" ) ) == NULL )
			printf( "\nCannot create log file '%s', using stdout.\n", sim.log_file );
		else
		{
			dup2( fileno( f ), STDOUT_FILENO );
			dup2( fileno( f ), STDERR_FILENO );
			fclose( f );
		}
	}

	return 0;
}
