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
 GETLIMITS.CPP
 Executes the lsd_getlimits command line utility.

 Lists all initial values ranges and configuration.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes

// limits and description for simulation settings
#define MIN_STEP 1
#define MAX_STEP 100000
#define DESC_STEP "Number of time steps to perform the simulation"
#define MIN_RUNS 1
#define MAX_RUNS 100000
#define DESC_RUNS "Number of times to repeat the simulation (Monte Carlo experiment)"
#define MIN_SEED 1
#define MAX_SEED 100000
#define DESC_SEED "First seed to be used to initialize the pseudorandom number generator"

#define SEP	",;\t"					// column separators to use

// objects which must be allocated in the heap
lsd::description desc;				// element description object

// command line strings
const char lsdCmdMsg[ ] = "This is the LSD Initial Values Range Reader.";
const char lsdCmdDsc[ ] = "It reads a LSD configuration file (.lsd) and a LSD sensitivity analysis file\n(.sa) and shows the ranges used for variables/parameters being analyzed,\noptionally saving them in a comma separated text file (.csv).\n";
const char lsdCmdHlp[ ] = "Command line options:\n'-f FILENAME.lsd' the configuration file to use\n'-s FILENAME.sa' the sensitivity analysis file to use\n'-o OUTPUT.csv' name for the comma separated output text file\n";


/*************************************************************
 MAIN
 *************************************************************/
int main( int argn, const char **argv )
{
	bool meta_par_in[ META_PAR_NUM ];
	char *sep, *str, *sens_file = NULL, *out_file = NULL;
	int i;
	FILE *f;

	// initialize LSD library and create master simulation object
	lsd::init_lib( & desc );
	lsd::simulation sim;				// must be defined AFTER init_lib!

	// assume exec path is included in file name, use CWD if not
	lsd::set_exec( NULL, argv[ 0 ] );

	if ( lsd::exec_file == NULL || strlen( lsd::exec_file ) == 0 || lsd::exec_path == NULL || strlen( lsd::exec_path ) == 0 )
	{
		fprintf( stderr, "\nInvalid executable name or path.\n%s\nMake sure the directory is not too deep into the disk directory tree (over %d chars).\n\n", lsdCmdMsg, PATH_MAX );
		lsd::lsd_exit( 1 );
	}

	if ( argn < 3 )
	{
		fprintf( stderr, "\n%s\n%s\n%s\n", lsdCmdMsg, lsdCmdDsc, lsdCmdHlp );
		lsd::lsd_exit( 2 );
	}
	else
	{
		for ( i = 1; i < argn; i += 2 )
		{
			// read -f parameter : original configuration file
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'f' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				delete [ ] sim.conf_name;
				sim.conf_name = new char [ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( sim.conf_name, argv[ 1 + i ] );
				continue;
			}
			// read -s parameter : sensitivity file name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 's' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				sens_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( sens_file, argv[ 1 + i ] );
				continue;
			}
			// read -o parameter : output file name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'o' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				out_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( out_file, argv[ 1 + i ] );
				continue;
			}

			fprintf( stderr, "\nOption '%c%c' not recognized.\n%s\n%s\n", argv[ i ][ 0 ], argv[ i ][ 1 ], lsdCmdMsg, lsdCmdHlp );
			lsd::lsd_exit( 2 );
		}
	}

	if ( sim.conf_name == NULL || strlen( sim.conf_name ) == 0 )
	{
		fprintf( stderr, "\nNo original configuration file provided.\n%s\nSpecify a -f FILENAME.lsd to use for reading the saved variables (if any).\n\n", lsdCmdMsg );
		lsd::lsd_exit( 4 );
	}

	str = new char [ strlen( sim.conf_name ) + 1 ];
	strcpy( str, sim.conf_name );
	lsd::strupr( str );

	if ( strstr( str, ".LSD" ) == NULL || strlen( str ) <= 4 )
	{
		fprintf( stderr, "\nInvalid configuration file provided.\n%s.\nSpecify a -f FILENAME.lsd to use for reading the saved variables (if any).\n\n", lsdCmdMsg );
		lsd::lsd_exit( 5 );
	}

	sim.conf_file = new char [ strlen( sim.conf_name ) + 1 ];
	strcpy( sim.conf_file, sim.conf_name );
	sim.conf_name[ strstr( str, ".LSD" ) - str ] = '\0';

	delete [ ] str;

	if ( ( f = fopen( sim.conf_file, "r" ) ) == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\n%s\nSpecify an existing -f FILENAME.lsd configuration file.\n\n", sim.conf_file, lsdCmdMsg );
		lsd::lsd_exit( 6 );
	}

	fclose( f );
	f = NULL;

	if ( sim.load_configuration( true, NULL, 0 ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\n%s\nCheck if the file is a valid LSD configuration or regenerate it using the LSD Browser.\n\n", sim.conf_file, lsdCmdMsg );
		lsd::lsd_exit( 7 );
	}

	if ( ( sens_file == NULL || strlen( sens_file ) == 0 ) && sim.sens == NULL )
	{
		fprintf( stderr, "\nNo sensitivity analysis file provided.\n%s\nSpecify a -s FILENAME.sa to use for reading the values limits (if any).\n\n", lsdCmdMsg );
		lsd::lsd_exit( 8 );
	}

	// read sensitivity file
	if ( sens_file != NULL && strlen( sens_file ) > 0 && ( f = fopen( sens_file, "rt" ) ) == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\n%s\nSpecify an existing -s FILENAME.sa sensitivity analysis file.\n\n", sens_file, lsdCmdMsg );
		lsd::lsd_exit( 9 );
	}

	if ( f != NULL && sim.load_txt_sensitivity( f ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\n%s\nCheck if the file is a valid LSD sensitivity analysis configuration or regenerate it using the LSD Browser.\n\n", sens_file, lsdCmdMsg	 );
		fclose( f );
		lsd::lsd_exit( 10 );
	}

	if ( f != NULL )
		fclose( f );

	if ( sim.sens == NULL )
	{
		fprintf( stderr, "\nNo sensitivity analysis data available.\n%s\nSpecify a -s FILENAME.sa to use for reading the values limits (if any).\n\n", lsdCmdMsg );
		lsd::lsd_exit( 11 );
	}

	if ( out_file != NULL && strlen( out_file ) != 0 )
	{
		f = fopen( out_file, "wt" );
		if ( f == NULL )
		{
			fprintf( stderr, "\nFile '%s' cannot be saved.\n%s\nCheck if the drive or the file is set READ-ONLY, change file name or\nselect a drive with write permission and try again.\n\n", out_file, lsdCmdMsg	 );
			lsd::lsd_exit( 12 );
		}

		sep = new char [ strlen( CSV_SEP ) + 1 ];
		strcpy( sep, CSV_SEP );

		// write .csv header
		fprintf( f, "Name%sType%sLag%sFormat%sValue%sMinimum%sMaximum%sDescription\n", sep, sep, sep, sep, sep, sep, sep );

		// write all parameters and initial conditions
		sim.root->get_sa_limits( f, sep, meta_par_in );

		// write simulation setting, if not already set
		if ( ! meta_par_in[ 0 ] )
			fprintf( f, "_timeSteps_%ssetting%s0%sinteger%s%d%s%d%s%d%s\"%s\"\n",
					 sep, sep, sep, sep, sim.last_t, sep, MIN_STEP, sep, MAX_STEP, sep, DESC_STEP );

		if ( ! meta_par_in[ 1 ] )
			fprintf( f, "_numRuns_%ssetting%s0%sinteger%s%d%s%d%s%d%s\"%s\"\n",
					 sep, sep, sep, sep, sim.last_run, sep, MIN_RUNS, sep, MAX_RUNS, sep, DESC_RUNS );

		if ( ! meta_par_in[ 2 ] )
			fprintf( f, "_rndSeed_%ssetting%s0%sinteger%s%d%s%d%s%d%s\"%s\"\n",
					 sep, sep, sep, sep, sim.seed, sep, MIN_SEED, sep, MAX_SEED, sep, DESC_SEED );

		fclose( f );
	}
	else	// send to stdout
	{
		sep = new char [ 2 ];
		strcpy( sep, "\t" );

		sim.root->get_sa_limits( stdout, sep, meta_par_in );

		if ( ! meta_par_in[ 0 ] )
			fprintf( stdout, "_timeSteps_%ssetting%s0%sinteger%s%d%s%d%s%d%s\"%s\"\n",
					 sep, sep, sep, sep, sim.last_t, sep, MIN_STEP, sep, MAX_STEP, sep, DESC_STEP );

		if ( ! meta_par_in[ 1 ] )
			fprintf( stdout, "_numRuns_%ssetting%s0%sinteger%s%d%s%d%s%d%s\"%s\"\n",
					 sep, sep, sep, sep, sim.last_run, sep, MIN_RUNS, sep, MAX_RUNS, sep, DESC_RUNS );

		if ( ! meta_par_in[ 2 ] )
			fprintf( stdout, "_rndSeed_%ssetting%s0%sinteger%s%d%s%d%s%d%s\"%s\"\n",
					 sep, sep, sep, sep, sim.seed, sep, MIN_SEED, sep, MAX_SEED, sep, DESC_SEED );
	}

	delete [ ] out_file;
	delete [ ] sens_file;

	lsd::lsd_exit( 0, true );

	return 0;
}
