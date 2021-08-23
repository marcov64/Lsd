/*************************************************************

	LSD 8.0 - May 2021
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD
	
 *************************************************************/

/*************************************************************
MCSTATS.CPP
Execute the lsd_mcstats command line utility.

Generate Monte Carlo experiment statistic files.
*************************************************************/

#include <cfloat>
#include "common.h"

#define SEP	",;\t"			// column separators to use

// class to handle arrays for MC data
template < typename T >
class vector3D 
{
	public:
		vector3D( size_t d1 = 0, size_t d2 = 0, size_t d3 = 0, T const & t = T( ) ) :
				  d1( d1 ), d2( d2 ), d3( d3 ), data( d1 * d2 * d3, t ) { }

		T & operator ( ) ( size_t i, size_t j, size_t k )
		{
			return ( i < d1 && j < d2 && k < d3 ) ? data[ i * d2 * d3 + j * d3 + k ] 
												  : data.at( i * d2 * d3 + j * d3 + k );
		}

		T const & operator ( ) ( size_t i, size_t j, size_t k ) const 
		{
			return data[ i * d2 * d3 + j * d3 + k ];
		}

		void resize( size_t _d1 = 0, size_t _d2 = 0, size_t _d3 = 0 )
		{
			data.resize( _d1 * _d2 * _d3 );
			d1 = _d1;
			d2 = _d2;
			d3 = _d3;
		}
		
	private:
		size_t d1, d2, d3;
		vector < T > data;
};

template < typename T >
class vector2D 
{
	public:
		vector2D( size_t d1 = 0, size_t d2 = 0, T const & t = T( ) ) :
				  d1( d1 ), d2( d2 ), data( d1 * d2, t ) { }

		T & operator ( ) ( size_t i, size_t j )
		{
			return ( i < d1 && j < d2 ) ? data[ i * d2 + j ] 
										: data.at( i * d2 + j );
		}

		T const & operator ( ) ( size_t i, size_t j ) const 
		{
			return data[ i * d2 + j ];
		}

		void resize( size_t _d1 = 0, size_t _d2 = 0 )
		{
			data.resize( _d1 * _d2 );
			d1 = _d1;
			d2 = _d2;
		}
		
	private:
		size_t d1, d2;
		vector < T > data;
};

void save_csv( const char *base, const char *suffix, vector < string > header, vector2D < double > data, int rows, int cols );

// constant string arrays
const char *signal_names[ REG_SIG_NUM ] = REG_SIG_NAME;
const int signals[ REG_SIG_NUM ] = REG_SIG_CODE;

char nonavail[ ] = "NA";	// string for unavailable values (use R default)
char **in_files = NULL;		// input .csv files
char *out_file = NULL;		// output .csv file, if any

// command line strings
const char lsdCmdMsg[ ] = "This is the LSD Monte Carlo statistics generator.";
const char lsdCmdDsc[ ] = "It reads a set of CSV (comma separated values) result files (.csv)\nfrom a Monte Carlo (MC) experiment and generates four new CSV files\ncontaining the means, standard errors, maximums and minimum values\nfor each variable at each time step considering all the MC samples (runs).\n";
const char lsdCmdHlp[ ] = "Command line options:\n'-o OUTPUT' base name for the comma separated output text files\n'-f FILENAME1.csv FILENAME2.csv ...' the MC experiment result files to use\n";


/*********************************
 LSDMAIN
 *********************************/
int lsdmain( int argn, const char **argv )
{
	char ch, *linbuf, *tok;
	double val, sum, sumsq, maxv, minv;
	int i, j, k, n, sz, linsz = 0, rows = 0, cols = 0, files = 0;
	vector < string > vars, cur_vars;
	FILE *f;
	
	if ( argn < 2 )
	{
		fprintf( stderr, "\n%s\n%s\n%s\n", lsdCmdMsg, lsdCmdDsc, lsdCmdHlp );
		myexit( 1 );
	}
	else
	{
		in_files = new char * [ argn ];
		
		for ( i = 1; i < argn; )
		{
			// read -f parameter : result files
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'f' )
			{
				while ( 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 && ! ( strlen( argv[ 1 + i ] ) == 2 && argv[ 1 + i ][ 0 ] == '-' ) )
				{
					in_files[ files ] = new char[ strlen( argv[ 1 + i ] ) + 1 ];
					strcpy( in_files[ files ], argv[ 1 + i ] );
					++i;
					++files;
				}
				
				++i;
				continue;
			}
			
			// read -o parameter : output file name
			if ( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == 'o' && 1 + i < argn && strlen( argv[ 1 + i ] ) > 0 )
			{
				out_file = new char[ strlen( argv[ 1 + i ] ) + 1 ];
				strcpy( out_file, argv[ 1 + i ] );
				i += 2;
				continue;
			}

			fprintf( stderr, "\nOption '%c%c' not recognized.\n%s\n%s\n", argv[ i ][ 0 ], argv[ i ][ 1 ], lsdCmdMsg, lsdCmdHlp );
			myexit( 2 );
		}
	}

	if ( files < 2 )
	{
		fprintf( stderr, "\nInsufficient result files provided.\n%s.\nSpecify '-f FILENAME1.csv FILENAME2.csv ...' to provide at least 2 result files.\n\n", lsdCmdMsg );
		myexit( 3 );
	}

	for ( i = 0; i < files; ++i )
	{
		f = fopen( in_files[ i ], "rt" );
		if ( f == NULL )
		{
			fprintf( stderr, "\nFile '%s' not found.\n%s\nSpecify existing '-f FILENAME1.csv FILENAME2.csv ...' result files.\n\n", in_files[ i ], lsdCmdMsg );
			myexit( 4 );
		}
		
		// determine number of rows and columns from results file
		j = 0;
		while ( ! feof( f ) )
		{
			for ( ch = k = 0; ch != '\n' && ch != EOF; ++k )
				ch = ( char ) fgetc( f );
			
			if ( k > 0 && ch != EOF )
				++j;
			
			linsz = max( linsz, k );
		}
		
		fclose( f );
		
		if ( i == 0 )
			rows = j;
		
		if ( j < 2 || j != rows )
		{
			fprintf( stderr, "\nInvalid file rows (%s).\n%s.\nFiles must have same number of rows (>1) and columns (>0).\n\n", in_files[ i ], lsdCmdMsg );
			myexit( 5 );
		}
	}

	if ( out_file == NULL || strlen( out_file ) == 0 )
	{
		fprintf( stderr, "\nNo base name to MC files provided.\n%s.\nSpecify '-o OUTPUT' to provide a base name.\n\n", lsdCmdMsg );
		myexit( 6 );
	}

	linbuf = new char[ linsz + 2 ];
	vector3D < double > mcdata( 0, 0, 0, 0 );
	
	for ( i = 0; i < files; ++i )
	{
		f = fopen( in_files[ i ], "rt" );
		
		// read header line
		cur_vars.clear( );
		fgets( linbuf, linsz + 1, f );
		tok = strtok( linbuf, SEP ); // prepares parsing / get first variable name
		for ( j = 0; tok != NULL; ++j )
		{
			sz = strlen( tok ) + 1;
			char out[ sz ];
			strtrim( out, tok, sz );
			cur_vars.push_back( out );
			tok = strtok( NULL, SEP );
		}
		
		if ( i == 0 )
		{
			cols = j;
			vars = cur_vars;
			mcdata.resize( files, rows - 1, cols );
		}
		
		if ( j < 1 || j != cols || cur_vars != vars )
		{
			fprintf( stderr, "\nInvalid file header (%s).\n%s.\nFiles must have same number of rows (>1) and columns (>0).\n\n", in_files[ i ], lsdCmdMsg );
			myexit( 7 );
		}
		
		// read data lines
		for ( j = 0; j < rows - 1 && ! feof( f ); ++j )
		{
			fgets( linbuf, linsz + 1, f );
			tok = strtok( linbuf, SEP );
			for ( k = 0; k < cols && tok != NULL; ++k )
			{
				sz = strlen( tok ) + 1;
				char out[ sz ];
				strtrim( out, tok, sz );
				if ( ! strcmp( out, nonavail ) )
					val = NAN;
				else
					if ( sscanf( out, "%lf", & val ) == 0 )
					{
						fprintf( stderr, "\nInvalid file values (%s).\n%s.\nFiles must have same number of rows (>1) and columns (>0).\n\n", in_files[ i ], lsdCmdMsg );
						myexit( 8 );
					}
				
				mcdata( i, j, k ) = val;
				tok = strtok( NULL, SEP );
			}
			
			if ( k < cols || tok != NULL )
			{
				fprintf( stderr, "\nInvalid file columns (%s).\n%s.\nFiles must have same number of rows (>1) and columns (>0).\n\n", in_files[ i ], lsdCmdMsg );
				myexit( 9 );
			}
		}
		
		if ( j < rows - 1 )
		{
			fprintf( stderr, "\nInvalid file rows (%s).\n%s.\nFiles must have same number of rows (>1) and columns (>0).\n\n", in_files[ i ], lsdCmdMsg );
			myexit( 10 );
		}
		
		fclose( f );
	}
	
	// compute MC statistics
	vector2D < double > me( rows - 1, cols ), se( rows - 1, cols ), mx( rows - 1, cols ), mn( rows - 1, cols );
	
	for ( j = 0; j < rows - 1; ++j )
		for ( k = 0; k < cols; ++k )
		{
			n = sum = sumsq = 0;
			maxv = DBL_MIN;
			minv = DBL_MAX;
			for ( i = 0; i < files; ++i )
				if ( isfinite( mcdata( i, j, k ) ) )
				{
					sum += mcdata( i, j, k );
					sumsq += pow( mcdata( i, j, k ), 2 );
					maxv = max( maxv, mcdata( i, j, k ) );
					minv = min( minv, mcdata( i, j, k ) );
					++n;
				}
			
			if ( n > 0 )
			{
				me( j, k ) = sum / n;
				me( j, k ) = fabs( me( j, k ) ) > 2 * DBL_MIN ? me( j, k ) : 0;
				se( j, k ) = sqrt( ( n * sumsq - pow( sum, 2 ) ) / ( n * ( n - 1 ) ) ) / sqrt( n );
				se( j, k ) = fabs( se( j, k ) ) > 2 * DBL_MIN ? se( j, k ) : 0;
				mx( j, k ) = fabs( maxv ) > 2 * DBL_MIN ? maxv : 0;
				mn( j, k ) = fabs( minv ) > 2 * DBL_MIN ? minv : 0;
			}
			else
				me( j, k ) = se( j, k ) = mx( j, k ) = mn( j, k ) = NAN;
		}
	
	// create MC files
	save_csv( out_file, "mean", vars, me, rows, cols );
	save_csv( out_file, "se", vars, se, rows, cols );
	save_csv( out_file, "max", vars, mx, rows, cols );
	save_csv( out_file, "min", vars, mn, rows, cols );

	for ( i = 0; i < files; ++i )
		delete [ ] in_files[ i ];
	
	delete [ ] in_files;
	delete [ ] out_file;
	delete [ ] linbuf;

	return 0;
}


/***************************************************
 SAVE_CSV
 save table to CSV file
 ***************************************************/
void save_csv( const char *base, const char *suffix, vector < string > header, vector2D < double > data, int rows, int cols )
{
	char fn[ strlen( base ) + strlen( suffix ) + 6 ];
	int i, j, k;
	FILE *f;
	
	sprintf( fn, "%s_%s.csv", base, suffix );
	f = fopen( fn, "wt" );
	if ( f == NULL )
	{
		fprintf( stderr, "\nFile '%s' cannot be created.\n%s\nCheck if base name is correct.\n\n", fn, lsdCmdMsg );
		myexit( 11 );
	}
	
	for ( i = 0; i < ( int ) header.size( ); ++ i )
		fprintf( f, "%s%s", i > 0 ? "," : "", header[ i ].c_str( ) );
	
	fprintf( f, "\n" );
	
	for ( j = 0; j < rows - 1; ++j )
	{
		for ( k = 0; k < cols; ++k )
			if ( isfinite( data( j, k ) ) )
				fprintf( f, "%s%g", k > 0 ? "," : "", data( j, k ) );
			else
				fprintf( f, "%s%s", k > 0 ? "," : "", nonavail );
		
		fprintf( f, "\n" );
	}

	fclose( f );
}
	
