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
CONFGEN.CPP
Execute the lsd_confgen command line utility.

Generates new configurations from a base one.

The input CSV file defines the elements to change (parameters
or variables' initial conditions) in the rows and the different
configurations in the columns.

First column contain the element names and optional lag number
for variables. Lags, if specified, must be separated by spaces
from variable name and be always negative integers (-1:first lag,
-2:second,...). If a lag is not specified, it is assumed as
the first lag (-1). Subsequent columns contain the values to be
used for elements in each configuration file to be generated
(1 column = 1 configuration).

A first header (column names) row is compulsory and must contain
the same number of columns as the other rows but its values are
not used.

Example of a CSV file changing the value of one parameter (K),
two lagged values of a variable (A), and generating two
configurations:

Elem, Cfg1, Cfg2
K	, 1	  , 2
A	, 3	  , 4
A -2, 5	  , 6
*************************************************************/

#include "lib/libLSD.h"				// LSD library classes

#define SEP	",;\t"					// column separators to use

bool change_configuration( lsd::simulation *sim, int findex );
int load_confs_csv( char *config );

// objects which must be allocated in the heap
lsd::description desc;				// element description object

// global variables
char **vars = NULL;					// array of variables/parameters names
double **values = NULL;				// array of configuration values
int *lags = NULL;					// array of variables lags
int num_vars = 0;					// total variables/parameters to change

// command line strings
const char lsdCmdMsg[ ] = "This is the LSD Configuration Generator.";
const char lsdCmdDsc[ ] = "It creates new LSD configuration file(s) (.lsd) based on changed parameters\nor variables initial values described in a comma separated text file (.csv).\nEach changed element should take one line. First column must contain the\nparameter or variable name. Second (and additional) column(s) must contain\nthe values to apply in the new configuration. First line (header) is required\nand considered for the number of columns only. One configuration is generated\nfor each column with values, sequentially numbered.\n";
const char lsdCmdHlp[ ] = "Command line options:\n'-f FILENAME.lsd' the original configuration file to use as base\n'-c CONFIG.csv' comma separated text file with new configuration values\n'-o FILE_BASE_NAME' base name (no extension) to save new configuration file(s)\n'-q' produce configuration file(s) including descriptions\n";


/*************************************************************
 MAIN
 *************************************************************/
int main( int argn, const char **argv )
{
	bool quick = true;
	char *str, *sav_file, *val_file = NULL, *out_file = NULL;
	int i, confs, len;
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
			// read -c parameter : text configuration file name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'c' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				val_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( val_file, argv[ 1 + i ] );
				continue;
			}
			// read -o parameter : output base name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'o' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				out_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( out_file, argv[ 1 + i ] );
				continue;
			}
			// read -p parameter : do not produce totals .tot files
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'q' )
			{
				i--;					// no parameter for this option
				quick = false;
				continue;
			}

			fprintf( stderr, "\nOption '%c%c' not recognized.\n%s\n%s\n", argv[ i ][ 0 ], argv[ i ][ 1 ], lsdCmdMsg, lsdCmdHlp );
			lsd::lsd_exit( 3 );
		}
	}

	if ( sim.conf_name == NULL || strlen( sim.conf_name ) == 0 )
	{
		fprintf( stderr, "\nNo original configuration file provided.\n%s\nSpecify a -f FILENAME.lsd to use as a base for the new configuration files.\n\n", lsdCmdMsg );
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
		fprintf( stderr, "\nFile '%s' not found.\n%s\nSpecify an existing -f FILENAME.lsd base configuration file.\n\n", sim.conf_file, lsdCmdMsg );
		lsd::lsd_exit( 6 );
	}

	fclose( f );

	// default values file name
	if ( val_file == NULL )
	{
		len = strlen( sim.conf_name ) + 5;
		val_file = new char[ len ];
		snprintf( val_file, len, "%s.csv", sim.conf_name );
	}

	if ( strlen( val_file ) == 0 || ( f = fopen( val_file, "r" ) ) == NULL )
	{
		fprintf( stderr, "\nFile '%s' not found.\n%s\nSpecify an existing -c CONFIG.csv to use as the new configuration values.\n\n", val_file, lsdCmdMsg );
		lsd::lsd_exit( 7 );
	}

	fclose( f );

	// default output base name
	if ( out_file == NULL )
	{
		out_file = new char[ strlen( sim.conf_name ) + 1 ];
		strcpy( out_file, sim.conf_name );
	}

	if ( sim.load_configuration( true, NULL, 0 ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\n%s\nCheck if the file is a valid LSD configuration or regenerate it using the LSD Browser.\n\n", sim.conf_file, lsdCmdMsg );
		lsd::lsd_exit( 8 );
	}

	if ( ( confs = load_confs_csv( val_file ) ) == 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\n%s\nSpecify a -c CONFIG.csv with a valid comma separated format.\n\n", val_file, lsdCmdMsg );
		lsd::lsd_exit( 9 );
	}

	len = strlen( out_file ) + ( int ) log10( confs ) + 3;
	sav_file = new char [ len ];

	for ( i = 1; i <= confs; ++i )
	{
		if ( ! change_configuration( & sim, i ) )
		{
			fprintf( stderr, "\nInvalid parameter or variable name.\n%s\nCheck if the spelling of the names of parameters and variables is exactly the\nsame as in the original configuration.\n\n", lsdCmdMsg );
			lsd::lsd_exit( 10 );
		}

		if ( confs == 1 )
			strcpy( sav_file, out_file );
		else
			snprintf( sav_file, len, "%s_%d", out_file, i );

		if ( strcmp( sav_file, sim.conf_name ) == 0 )
		{
			fprintf( stderr, "\nInvalid output file name.\n%s\nCannot overwrite input configuration file '%s.lsd'.\n\n", lsdCmdMsg, sav_file );
			lsd::lsd_exit( 11 );
		}

		if ( ! sim.save_xml_configuration( ".", sav_file, NULL, 0, false, quick ) )
		{
			fprintf( stderr, "\nFile '%s.lsd' cannot be saved.\n%s\nCheck if the drive or the file is set READ-ONLY, change file name or\nselect a drive with write permission and try again.\n\n", sav_file, lsdCmdMsg  );
			lsd::lsd_exit( 12 );
		}
	}

	for ( i = 0; i < num_vars; ++i )
	{
		delete [ ] vars[ i ];
		delete [ ] values[ i ];
	}

	delete [ ] val_file;
	delete [ ] out_file;
	delete [ ] sav_file;
	delete [ ] vars;
	delete [ ] values;
	delete [ ] lags;

	lsd::lsd_exit( 0, true );

	return 0;
}


/*************************************************************
 LOAD_CONFS_CSV
 *************************************************************/
int load_confs_csv( char *config )
{
	char buf[ MAX_LINE_SIZE ], var[ MAX_ELEM_LENGTH ], *line, *tok;
	double value;
	int i, j, lag, num_confs;
	FILE *f = fopen( config, "r" );
	std::set< std::string > existing;

	if ( f == NULL )
		return 0;

	// free existing arrays
	for ( i = 0; i < num_vars; ++i )
	{
		delete [ ] vars[ i ];
		delete [ ] values[ i ];
	}
	delete [ ] vars;
	delete [ ] values;
	delete [ ] lags;
	vars = NULL;
	values = NULL;
	lags = NULL;

	// determine the number of columns
	fgets( buf, MAX_LINE_SIZE, f );
	line = buf;
	j = 0;
	do										// count columns
	{
		tok = strtok( line, SEP );
		line = NULL;
		if ( tok != NULL )					// finished?
			++j;
	}
	while ( tok != NULL );
	num_confs = j - 1;

	// determine the number of non-blank lines after header
	i = 0;
	do
	{
		strcpy( buf, "" );
		fgets( buf, MAX_LINE_SIZE, f );
		if ( strcmp( buf, "" ) )
		{
			tok = strtok( buf, SEP );
			sscanf( tok, " %99s", var );	// remove spaces
			if ( ! strcmp( var, "" ) )
				continue;					// no name, go next line
			// check if name already exists and abort if so
			if ( ! existing.insert( var ).second )
			{
				fprintf( stderr, "Duplicated parameter/variable name: %s\n", var );
				return 0;
			}
			++i;
		}
	}
	while ( ! feof( f ) );
	num_vars = i;

	if ( num_confs < 1 || num_vars < 1 )
		return 0;							// nothing to do

	vars = new char * [ num_vars ];			// array of string pointers
	values = new double * [ num_vars ];		// array of double arrays
	lags = new int [ num_vars ];			// array of integer arrays

	// reread the file, populating the arrays
	i = 0;
	rewind( f );
	fgets( buf, MAX_LINE_SIZE, f );			// discard header
	do
	{
		strcpy( buf, "" );
		fgets( buf, MAX_LINE_SIZE, f );
		if ( strcmp( buf, "" ) != 0 )
		{
			lag = -1;
			tok = strtok( buf, SEP );
			sscanf( tok, " %99s %u", var, & lag );	// get name & lags
			if ( ! strcmp( var, "" ) )
				continue;					// no name, go next line

			// save name & lag
			vars[ i ] = new char [ strlen( var ) + 1 ];
			strcpy( vars[ i ], var );
			lags[ i ] = lag;

			// save configuration values
			values[ i ] = new double [ num_confs ];
			for ( j = 0; j < num_confs; ++j )
			{
				tok = strtok( NULL, SEP );
				if ( tok != NULL && sscanf( tok, " %lf", & value ) > 0 )
					values[ i ][ j ] = value;
				else
					values[ i ][ j ] = NAN;
			}
			++i;
		}
	}
	while ( ! feof( f ) );

	return num_confs;
}


/*************************************************************
 CHANGE_CONFIGURATION
 *************************************************************/
bool change_configuration( lsd::simulation *sim, int findex )
{
	char *lab;
	int i, lag;
	lsd::object *cur;
	lsd::variable *cv;

	if ( sim == NULL || sim->root == NULL || findex < 1 || findex > num_vars )
		return false;

	for ( i = 0; i < num_vars; ++i )
	{
		// handle pseudo-parameters
		if ( ! strcmp( vars[ i ], "_timeSteps_" ) )
		{
			sim->last_t = ( int ) std::max( 1., std::round( values[ i ][ findex - 1 ] ) );
			continue;
		}

		if ( ! strcmp( vars[ i ], "_numRuns_" ) )
		{
			sim->last_run = ( int ) std::max( 1., std::round( values[ i ][ findex - 1 ] ) );
			continue;
		}

		if ( ! strcmp( vars[ i ], "_rndSeed_" ) )
		{
			sim->seed = ( int ) std::max( 1., std::round( values[ i ][ findex - 1 ] ) );
			continue;
		}

		// it's a regular parameter/variable
		cv = sim->root->search_var( NULL, vars[ i ] );// get first instance
		if ( cv == NULL )
		{
			fprintf( stderr, "Parameter/variable not found: %s\n", vars[ i ] );
			return false;
		}

		lab = cv->up->attr->label;					// container object label
		for ( cur = cv->up; cur != NULL; cur = cur->hyper_next( lab ) )
		{											// update all instances
			cv = cur->search_var( NULL, vars[ i ] );
			if ( cv == NULL )
			{
				fprintf( stderr, "Corrupted LSD configuration file.\n" );
				return false;
			}
			if ( values[ i ][ findex - 1 ] == values[ i ][ findex - 1 ] )	// test NAN
			{
				if ( cv->param == 1 )
					lag = 0;						// parameters have no lag
				else
				{
					if ( lags[ i ] < 0 )
						lag = 0;					// fix invalid/undefined (default=1) lags
					else
						lag = lags[ i ] - 1;

					if ( lag > cv->attr->num_lag - 1 )
					{
						fprintf( stderr, "Variable not lagged or invalid lag: %s:%d\n", vars[ i ], lag + 1 );
						return false;
					}
				}

				cv->attr->initialized = true;
				cv->val[ lag ] = values[ i ][ findex - 1 ];
			}
		}
	}

	return true;
}
