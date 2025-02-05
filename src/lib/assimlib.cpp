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
 ASSIM CONSTRUCTOR
 Add or update data assimilation settings for a model element
 *************************************************************/
lsd::assim::assim( const char *lab, simulation *_sim, const char *_csv, const char *_data_col_name, const char *_t_col_name, int _data_col_num, int _t_col_num )
{
	assim *ca;

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

	if ( da.elem == NULL )
		da.elem = this;
	else
	{
		for ( ca = da.elem; ca->next != NULL; ca = ca->next );
		ca->next = this;
	}
}


/*************************************************************
 ASSIM DESTRUCTOR
 Remove data assimilation settings for a model element
 *************************************************************/
lsd::assim::~assim( void )
{
	assim *ca, *pa;

	delete [ ] csv_file;
	delete [ ] data_col_name;
	delete [ ] label;
	delete [ ] t_col_name;
	delete [ ] val;

	if ( da.elem != NULL )
	{
		for ( ca = da.elem, pa = NULL; ca != this && ca != NULL; pa = ca, ca = ca->next );

		if ( ca == da.elem )
			da.elem = next;
		else
			if ( ca == this && pa != NULL )
				pa->next = next;
	}
}


/*************************************************************
 COUNT
 Count elements in data assimilation linked list
 *************************************************************/
int lsd::assimilation::count( void )
{
	assim *ca;
	int n;

	for ( ca = elem, n = 0; ca != NULL; ca = ca->next, ++n );

	return n;
}


/*************************************************************
 EMPTY
 Deallocate data assimilation settings memory
 *************************************************************/
void lsd::assimilation::empty( assim *ca )
{
	if ( ca == NULL )
	{
		if ( elem == NULL )
			return;

		ca = elem;
		elem = NULL;
	}

	if ( ca->next != NULL )
		empty( ca->next );

	delete ca;				// suicide
}


/*************************************************************
 SEARCH
 Find element in data assimilation linked list
 *************************************************************/
lsd::assim *lsd::assimilation::search( const char *lab )
{
	assim *ca;

	for ( ca = elem; ca != NULL; ca = ca->next )
		if ( ! strcmp( ca->label, lab ) )
			 break;

	return ca;
}


/*************************************************************
 SHOW
 Print elements in data assimilation linked list to log window
 *************************************************************/
void lsd::assimilation::show( void )
{
	assim *ca;

	sims[ 0 ]->plog( "\n\nVariables set for data assimilation:\n" );
	for ( ca = elem; ca != NULL; ca = ca->next )
	{
		sims[ 0 ]->plog( "Var: %s \t%s\t(col=", ca->label, ca->csv_file );

		if ( ca->data_col_name != NULL && strlen( ca->data_col_name ) != 0 )
			sims[ 0 ]->plog_tag( "'%s'", "highlight", ca->data_col_name );
		else
			sims[ 0 ]->plog_tag( "%d", "highlight", ca->data_col_num );

		if ( ( ca->t_col_name != NULL && strlen( ca->t_col_name ) != 0 ) || ca->t_col_num > 0 )
		{
			sims[ 0 ]->plog( " t_col=" );

			if ( ca->t_col_name != NULL && strlen( ca->t_col_name ) != 0 )
				sims[ 0 ]->plog_tag( "'%s'", "highlight", ca->t_col_name );
			else
				sims[ 0 ]->plog_tag( "%d", "highlight", ca->t_col_num );
		}

		sims[ 0 ]->plog( ")\n" );
	}
}


/*************************************************************
 LOAD_FILES
 Load all files containing data required for data assimilation
 *************************************************************/
bool lsd::assimilation::load_files( void )
{
	bool first;
	int i, j;
	assim *ca;

	// load assimilation data, if amy/proper
	if ( elem == NULL || disable )
		return false;

	if ( ( i = load_data( ) ) < count( ) )
	{
		if ( i == 0 )
			empty( );
		else
		{
			sims[ 0 ]->plog( "\nData for assimilation missing for:" );
			for ( ca = elem, first = true; ca != NULL; ca = ca->next )
				if ( ca->missing )
				{
					sims[ 0 ]->plog( "%s %s", first ? "" : ",", ca->label );
					first = false;
				}
		}

#ifndef _TERM_
		cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"Cannot load assimilation data\" -detail \"Part or all data for assimilation could not be retrieved from data files.\nPlease check your assimilation configuration.\"" );
#endif
	}

	if ( elem != NULL )
	{
		if ( ( j = load_cov( ) ) <= 0 )
		{
			if ( j == -1 )
				sims[ 0 ]->plog( "\nUnused data in covariance matrix ignored" );

			sims[ 0 ]->plog( "\nAssimilation data loaded for %d variables\n", i );
		}
		else
		{
			empty( );
			switch ( j )
			{
				case 1:
					sims[ 0 ]->plog( "\nInvalid covariance matrix file name" );
					break;

				case 2:
					sims[ 0 ]->plog( "\nInvalid covariance matrix file CSV format" );
					break;

				case 3:
					sims[ 0 ]->plog( "\nNon-symmetric covariance matrix (rows != columns)" );
					break;

				case 4:
					sims[ 0 ]->plog( "\nEmpty covariance matrix" );
					break;

				case 5:
					sims[ 0 ]->plog( "\nMissing variable(s) in covariance matrix" );
					break;

				case 6:
					sims[ 0 ]->plog( "\nMissing elements in covariance matrix" );
			}

#ifndef _TERM_
		cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"Cannot load assimilation covariance matrix\" -detail \"There was a problem loading the covariance matrix for data assimilation from file '%s'.\nCheck the Log window for details.\"", cov_file != NULL ? cov_file : "(none)" );
#endif
		}
	}

	if ( elem == NULL )
	{
		sims[ 0 ]->plog( "\nData assimilation configuration is invalid, ignoring\n" );
		return false;
	}
	else
		return true;
}


/*************************************************************
 LOAD_DATA
 Load assimilation data from external data files
 *************************************************************/
int lsd::assimilation::load_data( void )
{
	struct assim_vars { ass_listT avl; int namrow = -1; };

	bool fexist;
	int i, vars_loaded = 0;
	rapidcsv::Document csv;
	std::unordered_map < strT, assim_vars > fv;

	data.clear( );
	time.clear( );

	for ( auto ca = elem; ca != NULL; ca = ca->next )
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
				d_vecT vdata;
				i_vecT vtime;
				dbl_mapT dtmap;

				try
				{
					if ( cf->second.namrow == 0 && ( *cv )->data_col_name != NULL )
						vdata = csv.GetColumn < double >( ( *cv )->data_col_name );
					else
						if ( ( *cv )->data_col_num > 0 )
							vdata = csv.GetColumn < double >( ( *cv )->data_col_num - 1 );
						else
							throw 1;

					( *cv )->missing = false;
				}
				catch( ... )
				{
					( *cv )->missing = true;
				}

				if ( ! ( *cv )->missing && vdata.size( ) > 0 )
				{
					try
					{
						if ( cf->second.namrow == 0 && ( *cv )->t_col_name != NULL )
							vtime = csv.GetColumn < int >( ( *cv )->t_col_name );
						else
							if ( ( *cv )->t_col_num > 0 )
								vtime = csv.GetColumn < int >( ( *cv )->t_col_num - 1 );

						if ( vdata.size( ) != vtime.size( ) )
							throw 1;
					}
					catch( ... )
					{
						if ( vdata.size( ) > vtime.size( ) )
						{
							int tini = 1, tsz = vtime.size( );

							if ( tsz > 0 )
								tini = vtime[ tsz - 1 ] + 1;

							vtime.resize( vdata.size( ) );
							std::iota( vtime.begin( ) + tsz, vtime.end( ), tini );
						}
						else
							vtime.resize( vdata.size( ) );
					}

					ass_listT empty;
					auto h = dtmap.end( );
					auto g = time.end( );
					for ( i = 0; i < ( int ) vdata.size( ); ++i )
					{
						h = dtmap.emplace_hint( h, vtime[ i ], vdata[ i ] );

						if ( time.find( vtime[ i ] ) == time.end( ) )
							g = time.emplace_hint( g, vtime[ i ], empty );

						time[ vtime[ i ] ].emplace_back( *cv );
					}

					data.emplace( ( *cv )->label, dtmap );

					++vars_loaded;
				}
			}
	}

	return vars_loaded;
}


/*************************************************************
 LOAD_COV
 Load covariance matrix for data assimilation from external file
 *************************************************************/
int lsd::assimilation::load_cov( void )
{
	char *cpath, fname[ MAX_PATH_LENGTH ];
	int i, j, res = 0;
	assim *ca;
	rapidcsv::Document csv;
	std::unordered_set < strT > covnames;
	std::unordered_set < strT >::iterator it;
	str_vecT csvnames;

	if ( cov_file == NULL || strlen( cov_file ) == 0 )
		return 1;

	cpath = sims[ 0 ]->conf_path;
	if ( cpath != NULL && strlen( cpath ) > 0 )
		snprintf( fname, MAX_PATH_LENGTH, "%s/%s", cpath, cov_file );
	else
		strcpyn( fname, cov_file, MAX_PATH_LENGTH );

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
	for ( ca = elem, i = 0; ca != NULL; ca = ca->next, ++i )
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
	cov_mat.resize( i, i );

	for ( i = 0; i < ( int ) csvnames.size( ); ++i )
		for ( j = 0; j < ( int ) csvnames.size( ); ++j )
			try
			{
				cov_mat( i, j ) = csv.GetCell < double > ( csvnames[ i ], csvnames[ j ] );
			}
			catch ( ... )
			{
				cov_mat.resize( 0, 0 );
				return 6;
			}

	return res;
}
