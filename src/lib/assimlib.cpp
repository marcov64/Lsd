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
 ASSIMLIB.CPP
 Data assimilation code used in DLL or terminal executables.
 The remaining DA code is stored in SET_ALL.CPP.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 ASSIMILATION CONSTRUCTOR
 Add or update data assimilation settings for a model element
 *************************************************************/
lsd::assimilation::assimilation( const char *lab, simulation *_sim, const char *_csv, const char *_data_col_name, const char *_t_col_name, int _data_col_num, int _t_col_num )
{
	assimilation *ca;

	sim = _sim;

	if ( lab != NULL )
	{
		label = new char [ strlen( lab ) + 1 ];
		strcpy( label, lab );
	}

	if ( _csv != NULL && strlen( _csv ) > 0 )
	{
		csv_file = new char [ strlen( _csv ) + 1 ];
		strcpy( csv_file, _csv );

		if ( _data_col_name != NULL && strlen( _data_col_name ) > 0 )
		{
			data_col_name = new char [ strlen( _data_col_name ) + 1 ];
			strcpy( data_col_name, _data_col_name );
		}
		else
			data_col_num = _data_col_num;

		if ( _t_col_name != NULL && strlen( _t_col_name ) > 0 )
		{
			t_col_name = new char [ strlen( _t_col_name ) + 1 ];
			strcpy( t_col_name, _t_col_name );
		}
		else
			t_col_num = _t_col_num;
	}

	if ( sim->assim == NULL )
		sim->assim = this;
	else
	{
		for ( ca = sim->assim; ca->next != NULL; ca = ca->next );
		ca->next = this;
	}
}


/*************************************************************
 ASSIMILATION DESTRUCTOR
 Remove data assimilation settings for a model element
 *************************************************************/
lsd::assimilation::~assimilation( void )
{
	assimilation *ca, *pa;

	delete [ ] csv_file;
	delete [ ] data_col_name;
	delete [ ] label;
	delete [ ] t_col_name;
	delete [ ] val;

	if ( sim->assim != NULL )
	{
		for ( ca = sim->assim, pa = NULL; ca != this && ca != NULL; pa = ca, ca = ca->next );

		if ( ca == sim->assim )
			sim->assim = next;
		else
			if ( ca == this && pa != NULL )
				pa->next = next;
	}
}


/*************************************************************
 COUNT_ASSIMILATION
 Count elements in data assimilation linked list
 *************************************************************/
int lsd::simulation::count_assimilation( void )
{
	assimilation *ca;
	int n;

	for ( ca = assim, n = 0; ca != NULL; ca = ca->next, ++n );

	return n;
}


/*************************************************************
 EMPTY_ASSIMILATION
 Deallocate data assimilation settings memory
 *************************************************************/
void lsd::simulation::empty_assimilation( assimilation *ca )
{
	if ( ca == NULL )
	{
		if ( assim == NULL )
			return;

		ca = assim;
		assim = NULL;
	}

	if ( ca->next != NULL )
		empty_assimilation( ca->next );

	delete ca;				// suicide
}


/*************************************************************
 SEARCH_ASSIMILATION
 Find element in data assimilation linked list
 *************************************************************/
lsd::assimilation *lsd::simulation::search_assimilation( const char *lab )
{
	assimilation *ca;

	for ( ca = assim; ca != NULL; ca = ca->next )
		if ( ! strcmp( ca->label, lab ) )
			 break;

	return ca;
}


/*************************************************************
 LOAD_ASSIM_DATA
 Load assimilation data from external data files
 *************************************************************/
int lsd::simulation::load_assim_data( void )
{
	struct assim_vars { ass_listT avl; int namrow = -1; };

	bool fexist;
	int i, vars_loaded = 0;
	rapidcsv::Document csv;
	std::unordered_map < strT, assim_vars > fv;

	for ( auto ca = assim; ca != NULL; ca = ca->next )
	{
		fv[ ca->csv_file ].avl.emplace_back( ca );

		if ( ( ca->data_col_name != NULL && strlen( ca->data_col_name ) > 0 ) ||
			 ( ca->t_col_name != NULL && strlen( ca->t_col_name ) > 0 ) )
			fv[ ca->csv_file ].namrow = 0;
	}

	for ( auto cf = fv.begin( ); cf != fv.end( ); ++cf )
	{
		try
		{
			csv.Load( cf->first, rapidcsv::LabelParams( cf->second.namrow, -1 ), rapidcsv::SeparatorParams( ',', true ), rapidcsv::ConverterParams( true, std::numeric_limits< long double >::quiet_NaN( ) ), rapidcsv::LineReaderParams( true, '#' ) );
			fexist = true;
		}
		catch ( ... )
		{
			fexist = false;
		}

		if ( fexist )
			for ( auto cv = cf->second.avl.begin( ); cv != cf->second.avl.end( ); ++cv )
			{
				d_vecT data;
				i_vecT time;
				dbl_mapT dtmap;

				try
				{
					if ( cf->second.namrow == 0 && ( *cv )->data_col_name != NULL )
						data = csv.GetColumn < double >( ( *cv )->data_col_name );
					else
						if ( ( *cv )->data_col_num > 0 )
							data = csv.GetColumn < double >( ( *cv )->data_col_num - 1 );
						else
							throw 1;

					( *cv )->missing = false;
				}
				catch( ... )
				{
					( *cv )->missing = true;
				}

				if ( ! ( *cv )->missing && data.size( ) > 0 )
				{
					try
					{
						if ( cf->second.namrow == 0 && ( *cv )->t_col_name != NULL )
							time = csv.GetColumn < int >( ( *cv )->t_col_name );
						else
							if ( ( *cv )->t_col_num > 0 )
								time = csv.GetColumn < int >( ( *cv )->t_col_num - 1 );

						if ( data.size( ) != time.size( ) )
							throw 1;
					}
					catch( ... )
					{
						if ( data.size( ) > time.size( ) )
						{
							int tini = 1, tsz = time.size( );

							if ( tsz > 0 )
								tini = time[ tsz - 1 ] + 1;

							time.resize( data.size( ) );
							std::iota( time.begin( ) + tsz, time.end( ), tini );
						}
						else
							time.resize( data.size( ) );
					}

					ass_listT empty;
					auto h = dtmap.end( );
					auto g = assim_time.end( );
					for ( i = 0; i < ( int ) data.size( ); ++i )
					{
						h = dtmap.emplace_hint( h, time[ i ], data[ i ] );

						if ( assim_time.find( time[ i ] ) == assim_time.end( ) )
							g = assim_time.emplace_hint( g, time[ i ], empty );

						assim_time[ time[ i ] ].emplace_back( *cv );
					}

					assim_data.emplace( ( *cv )->label, dtmap );

					++vars_loaded;
				}
			}
	}

	return vars_loaded;
}


/*************************************************************
 LOAD_ASSIM_COV
 Load covariance matrix for data assimilation from external file
 *************************************************************/
int lsd::simulation::load_assim_cov( void )
{
	char fname[ MAX_PATH_LENGTH ];
	int i, j, res = 0;
	assimilation *ca;
	rapidcsv::Document csv;
	std::unordered_set < strT > covnames;
	std::unordered_set < strT >::iterator it;
	str_vecT csvnames;

	if ( cov_file == NULL || strlen( cov_file ) == 0 )
		return 1;

	snprintf( fname, MAX_PATH_LENGTH, "%s%s%s", conf_path, strlen( conf_path ) > 0 ? "/" : "", cov_file );

	// load matrix from file
	try
	{
		csv.Load( fname, rapidcsv::LabelParams( 0, 0 ), rapidcsv::SeparatorParams( ',', true ), rapidcsv::ConverterParams( true, std::numeric_limits< long double >::quiet_NaN( ) ), rapidcsv::LineReaderParams( true, '#' ) );
	}
	catch ( ... )
	{
		return 2;
	}

	// check if matrix is (can be) symmetric
	auto cnames = csv.GetColumnNames( );
	auto rnames = csv.GetRowNames( );
	std::sort( cnames.begin( ), cnames.end( ) );
	std::sort( rnames.begin( ), rnames.end( ) );

	if ( cnames != rnames )
		return 3;

	// ignore empty matrix
	covnames.insert( cnames.begin( ), cnames.end( ) );
	if ( covnames.size( ) == 0 )
		return 4;

	// check if all information is available
	for ( ca = assim, i = 0; ca != NULL; ca = ca->next, ++i )
	{
		if ( ( it = covnames.find( ca->label ) ) != covnames.end( ) || ( ca->data_col_name != NULL && ( it = covnames.find( ca->data_col_name ) ) != covnames.end( ) ) )
		{
			csvnames.emplace_back( *it );
			ca->cov_idx = i;
		}
		else
			return 5;
	}

	// signal unused data
	if ( covnames.size( ) > csvnames.size( ) )
		res = -1;

	// build proper matrix, discarding unused data
	assim_cov.resize( i, i );

	for ( i = 0; i < ( int ) csvnames.size( ); ++i )
		for ( j = 0; j < ( int ) csvnames.size( ); ++j )
			try
			{
				assim_cov( i, j ) = csv.GetCell < double > ( csvnames[ i ], csvnames[ j ] );
			}
			catch ( ... )
			{
				assim_cov.resize( 0, 0 );
				return 6;
			}

	return res;
}
