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
 GETSAVED.CPP
 Execute the lsd_getsaved command line utility.

 Lists all variables being saved in a configuration.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes

#define SEP	",;\t"					// column separators to use

// command line strings
const char lsdCmdMsg[ ] = "This is the LSD Saved Variable Reader.";
const char lsdCmdDsc[ ] = "It reads a LSD configuration file (.lsd) and shows the variables/parameters\nbeing saved, optionally saving them in a comma separated text file (.csv).\n";
const char lsdCmdHlp[ ] = "Command line options:\n'-a' show all variables/parameters\n'-f FILENAME.lsd' the configuration file to use\n'-o OUTPUT.csv' name for the comma separated output text file\n";


/*************************************************************
 MAIN
 *************************************************************/
int main( int argn, const char **argv )
{
	bool all_var = false;
	char *sep, *str, *out_file = NULL;
	int i;
	FILE *f;

	// initialize LSD library and create master simulation object
	lsd::init_lib( );
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
			// read -o parameter : output file name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'o' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				out_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( out_file, argv[ 1 + i ] );
				continue;
			}
			// read -a parameter : show all variables/parameters
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'a' )
			{
				i--;					// no parameter for this option
				all_var = true;
				continue;
			}

			fprintf( stderr, "\nOption '%c%c' not recognized.\n%s\n%s\n", argv[ i ][ 0 ], argv[ i ][ 1 ], lsdCmdMsg, lsdCmdHlp );
			lsd::lsd_exit( 3 );
		}
	}

	if ( sim.conf_name == NULL || strlen( sim.conf_name ) == 0 )
	{
		fprintf( stderr, "\nNo configuration file provided.\n%s.\nSpecify a -f FILENAME.lsd to use for reading the saved variables (if any).\n\n", lsdCmdMsg );
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

	if ( sim.load_configuration( true, NULL, 1 ) != 0 )
	{
		fprintf( stderr, "\nFile '%s' is invalid.\n%s\nCheck if the file is a valid LSD configuration or regenerate it using the LSD Browser.\n\n", sim.conf_file, lsdCmdMsg );
		lsd::lsd_exit( 7 );
	}

	sim.root->count_save( & i );
	if ( ! all_var && i == 0 )
	{
		printf( "\n(no variable being saved)\n" );
		return 0;
	}

	if ( out_file != NULL && strlen( out_file ) != 0 )
	{
		f = fopen( out_file, "wt" );
		if ( f == NULL )
		{
			fprintf( stderr, "\nFile '%s' cannot be saved.\n%s\nCheck if the drive or the file is set READ-ONLY, change file name or\nselect a drive with write permission and try again.\n\n", out_file, lsdCmdMsg );
			lsd::lsd_exit( 8 );
		}

		sep = new char [ strlen( CSV_SEP ) + 1 ];
		strcpy( sep, CSV_SEP );

		// write .csv header
		fprintf( f, "Name%sType%sObject%sDescription\n", sep, sep, sep );
		sim.root->get_saved( f, sep, all_var );
		fclose( f );
	}
	else	// send to stdout
		sim.root->get_saved( stdout, "\t", all_var );

	delete [ ] out_file;

	lsd::lsd_exit( 0, true );

	return 0;
}
