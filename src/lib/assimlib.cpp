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
 ELEMENT_DATA constructor
 Prepare container for element data produced by assimilation
 *************************************************************/
lsd::element_data::element_data( int _start, int _end, bool sav_fct, bool sav_dat )
{
	size = _end - _start + 1;

	// use C stdlib to be able to deallocate partial memory
	if ( ( anl = ( double * ) malloc( size * sizeof( double ) ) ) == NULL )
		raise( SIGMEM );

	for ( auto i = 0; i < size; ++i )
		anl[ i ] = NAN;

	if ( sav_fct )
	{
		if ( ( fct = ( double * ) malloc( size * sizeof( double ) ) ) == NULL )
			raise( SIGMEM );

		for ( auto i = 0; i < size; ++i )
			fct[ i ] = NAN;
	}

	if ( sav_dat )
	{
		if ( ( dat = ( double * ) malloc( size * sizeof( double ) ) ) == NULL )
			raise( SIGMEM );

		for ( auto i = 0; i < size; ++i )
			dat[ i ] = NAN;
	}

	cur_t = start = _start;
	end = _end;
}


/*************************************************************
 ~ELEMENT_DATA destructor
 Prepare container for element data produced by assimilation
 *************************************************************/
lsd::element_data::~element_data( void )
{
	free( anl );
	free( fct );
	free( dat );
}


/*************************************************************
 ASSIM constructor
 Add or update data assimilation settings for a model element
 *************************************************************/
lsd::assim::assim( const char *_label, bool _param, bool _disable, bool _update, bool _data_obs, const char *_data_file, const char *_data_col_name, const char *_t_col_name, int _data_col_num, int _t_col_num, int _par_dist, double _par_n_sd, double _par_u_upp, double _par_u_low )
{
	assim *ca;
	variable *cv;

	if ( da == NULL )
		throw std::domain_error( "assimilation object not registered" );

	if ( _label != NULL )
	{
		label = new char [ strlen( _label ) + 1 ];
		strcpy( label, _label );

		if ( sims.size( ) > 0 && sims[ 0 ] != NULL )
			if ( ( cv = sims[ 0 ]->root->search_var( NULL, label, true ) ) != NULL )
			{
				parent = new char [ strlen( cv->up->label ) + 1 ];
				strcpy( parent, cv->up->label );
			}
	}

	param = _param;
	disable = _disable;
	update = _update;
	data_obs = _data_obs;

	if ( _data_file != NULL && strlen( _data_file ) > 0 )
	{
		data_file = new char [ strlen( _data_file ) + 1 ];
		strcpy( data_file, _data_file );

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

	par_dist = _par_dist;
	par_n_sd = _par_n_sd;
	par_u_upp = _par_u_upp;
	par_u_low = _par_u_low;

	no_data = param || disable || ! _data_obs || data_file == NULL || ( data_col_name == NULL && data_col_num < 1 );

	if ( da->elem == NULL )
		da->elem = this;
	else
	{
		for ( ca = da->elem; ca->next != NULL; ca = ca->next );
		ca->next = this;
	}
}


/*************************************************************
 INIT
 Initialize assimilation element before simulation running
 *************************************************************/
bool lsd::assim::init( void )
{
	object *cur;
	variable *cv;

	if ( disable )
		return true;

	if ( da == NULL || da->ref_sim == NULL )
		return false;

	da_data.clear( );

	// set the variable flags to current ones
	if ( parent != NULL && strlen( parent ) > 0 )
		cur = da->ref_sim->root->search( parent );
	else
		cur = NULL;

	if ( cur == NULL )
		cur = da->ref_sim->root;	// no parent hint, start from root

	if ( ( cv = cur->search_var( NULL, label, true ) ) == NULL )
		return false;

	param = cv->param;
	plot = cv->plot;
	save = cv->save || cv->savei;

	// count initial instances
	for ( inst_ini = 0; cv != NULL; ++inst_ini, cv = cv->hyper_next( ) );

	// add to map of assimilation elements
	da->elem_map[ label ] = this;

	return true;
}


/*************************************************************
 FINISH
 Finish assimilation element data collection after running
 *************************************************************/
void lsd::assim::finish( void )
{
	for ( auto inst : da_data )
		inst->end = inst->cur_t;
}


/*************************************************************
 ASSIM DESTRUCTOR
 Remove data assimilation settings for a model element
 *************************************************************/
lsd::assim::~assim( void )
{
	assim *ca, *pa;

	delete [ ] data_file;
	delete [ ] data_col_name;
	delete [ ] label;
	delete [ ] parent;
	delete [ ] t_col_name;

	for ( auto inst : da_data )
		delete inst;

	if ( da != NULL && da->elem != NULL )
	{
		for ( ca = da->elem, pa = NULL; ca != this && ca != NULL; pa = ca, ca = ca->next );

		if ( ca == da->elem )
			da->elem = next;
		else
			if ( ca == this && pa != NULL )
				pa->next = next;
	}
}


/*************************************************************
 INIT
 Initialize data assimilation structures before running
 *************************************************************/
bool lsd::assimilation::init( simulation *ref )
{
	int nrun, first;

	if ( ref == NULL || ( nrun = ref->last_run ) < 2 )
		return false;
	else
		ref_sim = ref;

	// initialize assimilation elements
	elem_map.clear( );
	for ( auto ca = elem; ca != NULL; ca = ca->next )
		if ( ! ca->init( ) )
			return false;

	// read data assimilation data from files
	if ( ! load_files( ref, ref_sim->last_t ) )
		return false;

	// set assimilation random number generator
	lib_prng.seed( ref->seed );

	// create simulation instances to produce realization forecasts
	first = sims.size( );
	for ( int i = 0, seed = ref->seed; i < nrun; ++i, ++seed )
	{
		auto sim = new simulation( ref->conf_name, ref->conf_path, 1 );

		// adjust instances' settings (random seed, single threaded)
		sim->seed = seed;
		sim->last_run = 1;
		sim->deb_t = 0;
		sim->stack_info = 0;
		sim->max_threads = 1;
		sim->parallel_disable = true;
		sim->prof_aggr_time = false;
		sim->no_ptr_chk = true;
		sim->results_alt_path( ref->alt_path );
		save_param( sim->root );
	}

	// vector of pointers to the ensemble realizations to run
	run_sims = sim_vecT( sims.begin( ) + first, sims.end( ) );

	return true;
}


/*************************************************************
 FINISH
 Finish data assimilation structures after running
 *************************************************************/
void lsd::assimilation::finish( void )
{
	for ( auto ca = elem; ca != NULL; ca = ca->next )
		ca->finish( );

	for ( auto sim : run_sims )
		delete sim;

	next_t = 0;
}


/*************************************************************
 ASSIMILATION DESTRUCTOR
 *************************************************************/
lsd::assimilation::~assimilation( void )
{
	empty( );
	delete [ ] dsp_file;
}


/*************************************************************
 RESET_INSTS
 Reset all the element instances' counters for AoR/saving
 *************************************************************/
void lsd::assimilation::reset_insts( assim *el )
{
	if ( el == NULL )
		el = elem;

	for ( ; el != NULL; el = el->next )
	{
		el->inst_idx = -1;

		for ( auto d : el->da_data )
			d->saved = false;
	}
}


/*************************************************************
 EMPTY
 Deallocate data assimilation elements settings memory
 *************************************************************/
void lsd::assimilation::empty( assim *el )
{
	if ( el == NULL )
	{
		if ( elem == NULL )
			return;

		el = elem;
		elem = NULL;
	}

	if ( el->next != NULL )
		empty( el->next );

	delete el;				// suicide
}


/*************************************************************
 COUNT
 Count elements in data assimilation linked list
 0: all elements
 1: enabled elements
 2: enabled parameters
 3: enabled variables
 4: enabled variables with data to load
 *************************************************************/
int lsd::assimilation::count( int what )
{
	assim *ca;
	int n;

	for ( ca = elem, n = 0; ca != NULL; ca = ca->next )
		switch ( what )
		{
			default:
			case 0:
				++n;
				break;

			case 1:
				if ( ! ca->disable )
					++n;
				break;

			case 2:
				if ( ! ca->disable && ca->param )
					++n;
				break;

			case 3:
				if ( ! ca->disable && ! ca->param )
					++n;
				break;

			case 4:
				if ( ! ca->disable && ! ca->param && ca->data_obs )
					++n;
		}

	return n;
}


/*************************************************************
 FIND
 Find data assimilation element using runtime (after init) map
 *************************************************************/
lsd::assim *lsd::assimilation::find( const char *lab )
{
	auto ca = da->elem_map.find( lab );

	if ( ca != da->elem_map.end( ) )
		return ca->second;

	return NULL;
}


/*************************************************************
 SEARCH
 Search element in data assimilation linked list
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
 SAVE_STATE_VARS
 Collect and save the current-step state variable/parameter
 instances for the simulation run
 *************************************************************/
void lsd::state_variables::save_state_vars( object *r )
{
	object *cur;
	st_vec.clear( );

	// find all instances of variables set and enabled for DA
	for ( auto ca = da->elem; ca != NULL; ca = ca->next )
		if ( ! ca->disable )
		{
			if ( ca->parent != NULL )		// try to start from parent
				cur = r->search( ca->parent );
			else
				cur = NULL;

			if ( cur == NULL )
				cur = r;					// no parent hint, start from root

			for ( auto cv = cur->search_var( NULL, ca->label, true ); cv != NULL; cv = cv->hyper_next( ) )
			{
				st_vec.emplace_back( cv );	// add to state vector of run
				ca->update_param( cv );		// handle parameter estimation
			}
		}
}


/*************************************************************
 ALIGN_STATE_VARS
 Ensure the state variable vectors in each run are consistent,
 making sure at a given position of the vector the same
 variable is referenced. If a variable has different number
 of instances among runs, there are two alternatives:
 . trim the number of instances kept in all vectors to the
 same as the state vector with the least instances, that is,
 unmatched instances are removed from the vectors of
 runs with more instances, and not considered for data
 assimilation; or
. insert virtual instances with the same mean or median
 as the rest of the ensemble, which may create artifacts in
 the covariance/comedian matrix, and so in the assimilation.
 *************************************************************/
#define DA_IDX ( sim->da_svars->idx )
#define DA_SV ( sim->da_svars->st_vec )

void lsd::assimilation::align_state_vars( void )
{
	strT lab;

	// reset vector indexes to first variable
	miss_inst.clear( );
	for ( auto sim : run_sims )
		DA_IDX = 0;

	// run over all base state variables
	for ( auto ca = elem; ca != NULL; ca = ca->next )
	{
		if ( ca->disable )
			continue;

		if ( align_trim )
		{
			// check for instance mismatches
			for ( auto missing = false; ! missing; )
			{
				// check if all runs have this instance
				for ( auto sim : run_sims )
					if ( DA_IDX >= DA_SV.size( ) || strcmp( DA_SV[ DA_IDX ]->label, ca->label ) != 0 )
					{
						missing = true;			// this run doesn't have instance
						break;
					}

				// update vector indexes, removing excess instances from all runs
				for ( auto sim : run_sims )
					if ( missing )
					{
						// one run missing the instance, remove all excess instances
						while ( true )			// remove all extra instances from var
							if ( DA_IDX < DA_SV.size( ) && strcmp( DA_SV[ DA_IDX ]->label, ca->label ) == 0 )
								DA_SV.erase( DA_SV.begin( ) + DA_IDX );
							else
								break;			// stop on first var after or last var
					}
					else						// all runs have this instance
						++DA_IDX;				// all aligned so far, check next var
			}
		}
		else
		{
			// check if state vector finish
			for ( auto done = false; ! done; )
			{
				// check if all runs have this instance
				for ( auto sim : run_sims )
				{	// if not, add virtual instance
					if ( DA_IDX >= DA_SV.size( ) || strcmp( DA_SV[ DA_IDX ]->label, ca->label ) != 0 )
					{
						if ( DA_IDX >= miss_inst.size( ) )
							miss_inst.resize( DA_IDX + 1, 0 );

						++miss_inst[ DA_IDX ];	// save missed instances in column
						DA_SV.insert( DA_SV.begin( ) + DA_IDX, NULL );
					}

					++DA_IDX;
				}

				// check if any run still has instances
				done = true;					// assume all instances done
				for ( auto sim : run_sims )
					if ( DA_IDX < DA_SV.size( ) && strcmp( DA_SV[ DA_IDX ]->label, ca->label ) == 0 )
					{
						done = false;			// except if a run still has inst.
						break;
					}
			}
		}
	}

	// save aligned forecasted variable names
	fctd_labs.clear( );
	for ( auto cv : run_sims[ 0 ]->da_svars->st_vec )
	{
		if ( cv != NULL )						// handle virtual instances
			lab = cv->label;

		fctd_labs.emplace_back( lab );
	}

	// ensure missing instance vector is consistent
	miss_inst.resize( fctd_labs.size( ), false );
}


/*************************************************************
 UPDATE_STATE_VARS
 Update the ensemble state variables (& free parameters) to
 incorporate the DA analysis estimates
 *************************************************************/
void lsd::assimilation::update_state_vars( const e_matT & x_a_e )
{
	int nobs = run_sims.size( );
	int nvar = fctd_labs.size( );
	variable *cv;

	for ( int j = 0; j < nvar; ++j )
		if ( elem_map[ fctd_labs[ j ] ]->update )
			for ( int i = 0; i < nobs; ++i )
			{
				cv = run_sims[ i ]->da_svars->st_vec[ j ];
				cv->val[ 0 ] = cv->chk_val( x_a_e( i, j ) );

				if ( cv->save || cv->savei )
					cv->data[ run_sims[ i ]->eff_t - cv->start ] = cv->val[ 0 ];
			}
}


/*************************************************************
 UPDATE_ASSIM_VARS
 Update the DA analysis variables in LSD
 *************************************************************/
void lsd::assimilation::update_assim_vars( const e_vecT & x_a, const e_vecT & x_f, const e_vecT & z, int t )
{
	assim *ca;
	size_t i;
	int nvar = x_a.size( );

	// saves each variable instance to the corresponding DA element storage
	for ( int j = 0; j < nvar; ++j )
	{
		ca = elem_map[ fctd_labs[ j ] ];

		if ( ! ca->save )
			continue;

		// find the proper instance to update
		for ( i = 0; i < ca->da_data.size( ); ++i )
			if ( ca->da_data[ i ]->cur_t < t )			// check existing slots
				break;

		if ( i == ca->da_data.size( ) )					// all used, create new
		{
			auto *slot = new element_data ( t, ref_sim->last_t, da->sav_fct, da->sav_dat );
			ca->da_data.emplace_back( slot );
		}

		ca->da_data[ i ]->anl[ t - ca->da_data[ i ]->start ] = x_a[ j ];

		if ( da->sav_fct )
			ca->da_data[ i ]->fct[ t - ca->da_data[ i ]->start ] = x_f[ j ];

		if ( da->sav_dat )
			ca->da_data[ i ]->dat[ t - ca->da_data[ i ]->start ] = z[ j ];

		ca->da_data[ i ]->cur_t = ca->da_data[ i ]->end = t;

	}
}


/*************************************************************
 SAVE_PARAM
 Save initial values of parameters being estimated
 *************************************************************/
void lsd::assimilation::save_param( object *r )
{
	// find all instances of parameters requiring saving initial values
	for ( auto ca = elem; ca != NULL; ca = ca->next )
		if ( ! ca->disable && ca->param && ca->update && ca->par_dist == 1 )
			for ( auto cv = r->search_var( NULL, ca->label, true ); cv != NULL; cv = cv->hyper_next( ) )
				cv->ini_val = cv->val[ 0 ];
}


/*************************************************************
 UPDATE_PARAM
 Update a parameter set for data assimilation
 *************************************************************/
void lsd::assim::update_param( variable *v )
{
	double a, b;
	if ( ! param || ! update || v == NULL || v->param != 1 || ! std::isfinite( v->val[ 0 ] ) )
		return;

	switch ( par_dist )
	{
		case 0:						// normal
			if ( par_n_sd > 0 )
			{
				std::normal_distribution < double > N( 0, par_n_sd );
				v->val[ 0 ] = v->chk_val( v->val[ 0 ] + N( lib_prng ) );
			}

			break;

		case 1:						// uniform
			a = v->ini_val - par_u_low;
			b = v->ini_val + par_u_upp;
			if ( a < b )
			{
				std::uniform_real_distribution < double > U( a, b );
				v->val[ 0 ] = v->chk_val( U( lib_prng ) );
			}
	}
}


/*************************************************************
 UPDATE_RUNTIME_PLOT
 Update the DA run-time plot window
 *************************************************************/
void lsd::assimilation::update_runtime_plot( int cur_t )
{
	if ( ref_sim->liblnk->plot_runtime == NULL )
		return;

	for ( auto ca = elem; ca != NULL; ca = ca->next )
		if ( ca->plot )
			// plot up to just the initial instances
			for ( auto i = 0; i <= ca->inst_ini; ++i )
			{
				if ( i >= ( int ) ca->da_data.size( ) )
					ref_sim->liblnk->plot_runtime( cur_t, NAN, NAN );
				else
					if ( ca->param == 1 || cur_t == ca->da_data[ i ]->start )
						ref_sim->liblnk->plot_runtime( cur_t, ca->da_data[ i ]->anl[ cur_t - ca->da_data[ i ]->start ], NAN );
					else
						ref_sim->liblnk->plot_runtime( cur_t, ca->da_data[ i ]->anl[ cur_t - ca->da_data[ i ]->start ], ca->da_data[ i ]->anl[ cur_t - ca->da_data[ i ]->start - 1 ] );
			}
}


/*************************************************************
 ANALYSIS
 Perform data assimilation analysis step
 *************************************************************/
 int lsd::assimilation::analysis( const ass_vecT & dvars, int cur_t )
{
	// reconcile/align the state vectors along all simulation runs
	align_state_vars( );

	int nobs = run_sims.size( );
	int nvar = fctd_labs.size( );
	e_matT K, x_a_e( nobs, nvar );

	// create the forecast ensemble matrix (N x L)
	const e_matT & x_f_ens = ensemble_forecast( );

	// produce the forecast ensemble location statistics (L x 1)
	const e_vecT & x_f = loc_stat( x_f_ens );

	// apply ensemble inflation if enabled
	const e_matT & x_f_e = ensemble_inflation( x_f_ens, x_f );

	// produce the forecast ensemble dispersion statistics (L x L)
	const e_matT & P_f = dsp_stat( x_f_e, x_f );

	// create the forward model matrix (P x L)
	const e_matT & H = forward_matrix( dvars );

	// create virtual observations (P x N)
	const e_vecT & z = data_obs( dvars, cur_t );
	const e_matT & z_e = virtual_obs( z, nobs );

	// apply the data assimilation algorithm
	switch ( algorithm )
	{
		case 0:							// EnKF
		{
			// do LU decomposition of the term to be inverted
			Eigen::FullPivLU< e_matT > lu( H * P_f * H.transpose( ) + dsp_mat );

			// try to compute the Kalman gain matrix
			if ( lu.isInvertible( ) )
				K = P_f * H.transpose( ) * lu.inverse( );
			else
				return 11;

			// finally, do the ensemble analysis
			for ( auto i = 0; i < nobs; ++i )
			{
				e_vecT x = x_f_e.row( i ).transpose( ) + K * ( z_e.row( i ).transpose( ) - H * x_f_e.row( i ).transpose( ) );
				x_a_e.row( i ) = x;
			}

			break;
		}

		case 1:							// ETPF
		{

		}
	}

	// update the state variables in simulation runs
	update_state_vars( x_a_e );

	// compute the MC analysis ensemble estimates & refresh run-time window
	const e_vecT & x_a = loc_stat( x_a_e );
	update_assim_vars( x_a, x_f, z, cur_t );
	update_runtime_plot( cur_t );

	return 0;
}


/*************************************************************
 MEDIAN (assimilation only)
 Compute the median on Eigen containers in-place
 It changes the original container order!
 *************************************************************/
template < class T >
double lsd::assimilation::median( T begin, T end )
{
	int s = 0;
	for ( auto it = begin; it != end; ++it, ++s );
	int n = s / 2;
	auto p = begin + n;
	std::nth_element( begin, p, end );

	if ( s % 2 != 0 )
		return *( begin + n );
	else
		return ( *std::max_element( begin, p ) + *( begin + n ) ) / 2.;
}


/*************************************************************
 LOC_STAT
 Compute the location statistic vector (mean or median)
 *************************************************************/
const e_vecT & lsd::assimilation::loc_stat( const e_matT & x )
{
	int nvar = x.cols( );
	static e_vecT x_bar( nvar );

	if ( med_stats )
		for ( int i = 0; i < nvar; ++i )
		{
			d_vecT fct( x.col( i ).begin( ), x.col( i ).end( ) );
			x_bar[ i ] = median( fct.begin( ), fct.end( ) );
		}
	else
		x_bar = x.colwise( ).mean( );

	return x_bar;
}


/*************************************************************
 DISP_STAT
 Compute the dispersion statistic matrix (covariance or comedian)
 *************************************************************/
const e_matT & lsd::assimilation::dsp_stat( const e_matT & x, const e_vecT & x_bar )
{
	int nvar = x.cols( );
	e_vecT d( nvar );

	static e_matT dsp = e_matT::Zero( nvar, nvar );

	if ( med_stats )
	{
		for ( int i = 0; i < nvar; ++i )
			for ( int j = i; j < nvar; ++j )		// symmetrical matrix
			{
				d = ( x.col( i ).array( ) - x_bar[ i ] ) * ( x.col( j ).array( ) - x_bar[ j ] );
				dsp( i, j ) = median( d.begin( ), d.end( ) );
				if ( i != j )
					dsp( j, i ) = dsp( i, j );		// not in main diagonal
			}
	}
	else
	{
		for ( auto row : x.rowwise( ) )
		{
			d = row.transpose( ) - x_bar;
			dsp += d * d.transpose( );
		}

		dsp /= x.rows( ) - 1.;
	}

	return dsp;
}


/*************************************************************
 ENSEMBLE_FORECAST
 Create the state forecast matrix for the simulation ensemble
 *************************************************************/
const e_matT & lsd::assimilation::ensemble_forecast( void )
{
	d_mapT loc;
	int nobs = run_sims.size( );
	int nvar = fctd_labs.size( );
	static e_matT x_f_ens( nobs, nvar );
	variable *sv;

	// compute average/median for columns missing instances
	if ( ! align_trim )
		for ( size_t j = 0; j < miss_inst.size( ); ++j )
			if ( miss_inst[ j ] > 0 )
			{
				d_vecT v;
				v.reserve( miss_inst[ j ] );

				// get all existing instance values
				for ( auto i = 0; i < nobs; ++i )
					if ( ( sv = run_sims[ i ]->da_svars->st_vec[ j ] ) != NULL && std::isfinite( sv->val[ 0 ] ) )
						v.emplace_back( sv->val[ 0 ] );

				// compute location statistic to be used in virtual instances
				if ( med_stats )
					loc[ j ] = median( v.begin( ), v.end( ) );
				else
					loc[ j ] = std::accumulate( v.begin( ), v.end( ), 0. ) / v.size( );
			}

	// fill matrix by rows/runs
	for ( auto i = 0; i < nobs; ++i )
		for ( auto j = 0; j < nvar; ++j )
			if ( ( sv = run_sims[ i ]->da_svars->st_vec[ j ] ) != NULL && std::isfinite( sv->val[ 0 ] ) )
				x_f_ens( i, j ) = sv->val[ 0 ];
			else
				if ( ! align_trim && loc.find( j ) != loc.end( ) )
					x_f_ens( i, j ) = loc[ j ];
				else
					x_f_ens( i, j ) = NAN;

	return x_f_ens;
}


/*************************************************************
 ENSEMBLE_INFLATION
 Create the state forecast matrix for the simulation ensemble
 *************************************************************/
const e_matT & lsd::assimilation::ensemble_inflation( const e_matT & x, const e_vecT & x_bar )
{
	if ( ! ens_infl )
		return( x );

	int nobs = x.rows( );
	int nvar = x.cols( );
	static e_matT x_f_e( nobs, nvar );

	for ( int i = 0; i < nobs; ++i )
	{
		e_vecT d = x.row( i ).transpose( ) - x_bar;
		x_f_e.row( i ) = x_bar + infl_fac * d;
	}

	return x_f_e;
}


/*************************************************************
 FORWARD_MATRIX
 Create the forward model matrix
 *************************************************************/
const e_matT & lsd::assimilation::forward_matrix( const ass_vecT & dvars )
{
	int ndvar = dvars.size( );
	int nfvar = fctd_labs.size( );
	static e_matT H = e_matT::Zero( ndvar, nfvar );
	b_vecT used_fvars( nfvar, false );

	// match data variables (no duplicates) to each forecast variable (incl. duplicates)
	for ( auto i = 0; i < ndvar; ++i )
	{
		i_vecT idx_fvars;
		bool found = false;
		for ( auto j = 0; j < nfvar; ++j )
		{
			if ( used_fvars[ j ] )	// avoid comparing already matched variables
				continue;

			if ( dvars[ i ]->label == fctd_labs[ j ] )
			{
				idx_fvars.emplace_back( j );
				used_fvars[ j ] = true;
				found = true;
			}
			else
				if ( found )		// finish sequence of forecast instances?
					break;			// next data variable
		}

		// weight the effect of multi-instance variables on data
		for ( auto j : idx_fvars )
			H( i, j ) = 1. / idx_fvars.size( );
	}

	return H;
}


/*************************************************************
 DATA_OBS
 Get current data observations
 *************************************************************/
const e_vecT & lsd::assimilation::data_obs( const ass_vecT & dvars, int cur_t )
{
	int nvar = dvars.size( );
	static e_vecT z( nvar );

	// collect observations available at current time
	for ( auto j = 0; j < nvar; ++j )
		z[ j ] = var_data[ dvars[ j ]->label ][ cur_t ];

	return z;
}

/*************************************************************
 VIRTUAL_OBS
 Create an ensemble of virtual observations
 *************************************************************/
const e_matT & lsd::assimilation::virtual_obs( const e_vecT & z, int nobs )
{
	int nvar = z.size( );
	static e_matT z_e( nobs, nvar );
	e_vecT y( nvar );

	std::normal_distribution < double > N( 0, 1 );

	// generate virtual observations
	for ( auto i = 0; i < nobs; ++i )
	{
		// create a random vector
		for ( auto j = 0; j < nvar; ++j )
			y[ j ] = N( lib_prng );

		z_e.row( i ) = z + dsp_chol * y;
	}

	return z_e;
}


/*************************************************************
 LOAD_OBS_DATA
 Load assimilation data from external data files
 *************************************************************/
int lsd::assimilation::load_obs_data( int last_t )
{
	struct assim_vars { ass_listT avl; int namrow = -1; };

	bool fexist;
	int vars_loaded = 0;
	size_t i;
	rapidcsv::Document csv;
	std::unordered_map < strT, assim_vars > fv;

	var_data.clear( );
	time_var.clear( );
	data_lab.clear( );

	// identify variables to be read and group them by data file
	for ( auto ca = elem; ca != NULL; ca = ca->next )
	{
		ca->no_data = true;

		if ( ca->param || ca->disable || ! ca->data_obs || ca->data_file == NULL || strlen( ca->data_file ) == 0 || ( ( ca->data_col_name == NULL || strlen( ca->data_col_name ) == 0 ) && ca->data_col_num < 1 ) )
			continue;

		fv[ ca->data_file ].avl.emplace_back( ca );

		if ( ( ca->data_col_name != NULL && strlen( ca->data_col_name ) > 0 ) ||
			 ( ca->t_col_name != NULL && strlen( ca->t_col_name ) > 0 ) )
			fv[ ca->data_file ].namrow = 0;
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

		// ignore variables without valid file
		if ( ! fexist )
			continue;

		// picks data from all variables in file
		for ( auto ca = cf->second.avl.begin( ); ca != cf->second.avl.end( ); ++ca )
		{
			d_vecT vdata;
			i_vecT vtime;
			d_mapT dtmap;

			try
			{
				if ( cf->second.namrow == 0 && ( *ca )->data_col_name != NULL )
					vdata = csv.GetColumn < double >( ( *ca )->data_col_name );
				else
					if ( ( *ca )->data_col_num > 0 )
						vdata = csv.GetColumn < double >( ( *ca )->data_col_num - 1 );
					else
						throw 1;

				if ( vdata.size( ) > 0 )
					( *ca )->no_data = false;
			}
			catch( ... ) { }

			// ignore variables with missing data
			if ( ! ( *ca )->no_data )
			{
				try
				{
					if ( cf->second.namrow == 0 && ( *ca )->t_col_name != NULL )
						vtime = csv.GetColumn < int >( ( *ca )->t_col_name );
					else
						if ( ( *ca )->t_col_num > 0 )
							vtime = csv.GetColumn < int >( ( *ca )->t_col_num - 1 );

					if ( vdata.size( ) != vtime.size( ) )
						throw 1;
				}
				catch( ... )
				{
					// create time reference from data
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

				// create var to data and time to var maps
				auto h = dtmap.end( );
				auto g = time_var.end( );
				for ( i = 0; i < vdata.size( ) && vtime[ i ] <= last_t; ++i )
				{
					h = dtmap.emplace_hint( h, vtime[ i ], vdata[ i ] );

					if ( time_var.find( vtime[ i ] ) == time_var.end( ) )
						g = time_var.emplace_hint( g, vtime[ i ], ass_vecT ( ) );

					time_var[ vtime[ i ] ].emplace_back( *ca );
				}

				var_data.emplace( ( *ca )->label, dtmap );
				data_lab.emplace_back( ( *ca )->label );

				++vars_loaded;
			}
		}
	}

	return vars_loaded;
}


/*************************************************************
 CALC_DSP_MAT
 Calculate dispersion (covariance or comedian) matrix for data
 assimilation from observational data, and try to perform
 Cholesky decomposition
 *************************************************************/
int lsd::assimilation::calc_dsp_mat( void )
{
	int res = 0;
	int nvar = data_lab.size( );
	e_matT cov_mat( nvar, nvar );				// contingency covariance matrix
	dsp_mat.resize( nvar, nvar );

	// handle each pair of variables independently to allow for different timings
	for ( auto i = 0; i < nvar; ++i )
	{
		auto x = var_data[ data_lab[ i ] ];

		for ( auto j = i; j < nvar; ++j )		// symmetrical matrix
		{
			auto y = var_data[ data_lab[ j ] ];

			e_vecT xv( x.size( ) );
			e_vecT yv( y.size( ) );

			int n = 0;
			for ( auto xt : x )
			{
				// look for times both values are defined
				auto yt = y.find( xt.first );
				if ( yt != y.end( ) )
				{
					xv[ n ] = xt.second;
					yv[ n ] = yt->second;
					++n;
				}
			}

			if ( n == 0 )						// no match
				dsp_mat( i, j ) = 0;
			else
			{
				xv.resize( n );
				yv.resize( n );

				// always compute the covariance matrix as a contingency
				e_vecT dx = xv.array( ) - xv.mean( );
				e_vecT dy = yv.array( ) - yv.mean( );
				cov_mat( i, j ) = dx.dot( dy ) / ( n - 1. );

				if ( med_stats )
				{
					double xmed = median( xv.begin( ), xv.end( ) );
					double ymed = median( yv.begin( ), yv.end( ) );
					e_vecT d = ( xv.array( ) - xmed ) * ( yv.array( ) - ymed );
					dsp_mat( i, j ) = median( d.begin( ), d.end( ) );
				}
				else
					dsp_mat( i, j ) = cov_mat( i, j );
			}

			if ( i != j )
			{
				cov_mat( j, i ) = cov_mat( i, j );	// not in main diagonal
				dsp_mat( j, i ) = dsp_mat( i, j );
			}
		}
	}

	if ( nvar == 0 || dsp_mat.isZero( ) )
		return 1;

	// check if matrix is positive definite
	e_vecT e = dsp_mat.selfadjointView< Eigen::Lower >( ).eigenvalues( );
	if ( ! std::all_of( e.begin( ), e.end( ), [ ] ( double x ) { return x > 0; } ) )
	{
		if ( med_stats )
		{
			// check if the covariance matrix can be used as contingency
			e_vecT f = cov_mat.selfadjointView< Eigen::Lower >( ).eigenvalues( );
			if ( ! std::all_of( f.begin( ), f.end( ), [ ] ( double x ) { return x > 0; } ) )
				return 3;

			dsp_mat = cov_mat;
			res = 2;
		}
		else
			return 3;
	}

	// Cholesky decomposition
	dsp_chol = dsp_mat.llt( ).matrixL( );

	return res;
}


/*************************************************************
 LOAD_DSP_MAT
 Load dispersion (covariance or comedian) matrix for data
 assimilation from external file, and try to perform
 Cholesky decomposition
 *************************************************************/
int lsd::assimilation::load_dsp_mat( simulation *sim )
{
	char *cpath, fname[ MAX_PATH_LENGTH ];
	int i, j, k, res = 0;
	assim *ca;
	rapidcsv::Document csv;
	std::unordered_set < strT > covnames;
	std::unordered_set < strT >::iterator it;
	str_vecT csvnames;

	dsp_mat.resize( 0, 0 );
	for ( ca = elem; ca != NULL; ca = ca->next )
		ca->cov_idx = -1;

	if ( dsp_file == NULL || strlen( dsp_file ) == 0 )
		return 1;

	cpath = sim == NULL ? NULL : sim->conf_path;
	if ( cpath != NULL && strlen( cpath ) > 0 )
		snprintf( fname, MAX_PATH_LENGTH, "%s/%s", cpath, dsp_file );
	else
		strcpyn( fname, dsp_file, MAX_PATH_LENGTH );

	// try to load matrix from file
	try
	{
		csv.Load( fname, rapidcsv::LabelParams( 0, 0 ), rapidcsv::SeparatorParams( ',', true ), rapidcsv::ConverterParams( true, std::numeric_limits< long double >::quiet_NaN( ) ), rapidcsv::LineReaderParams( true, '#' ) );
	}
	catch ( ... )
	{
		return 2;
	}

	// check if matrix is (can be made) symmetric
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
	for ( ca = elem, k = 0; ca != NULL; ca = ca->next )
	{
		if ( ca->no_data )
			continue;

		if ( ( it = covnames.find( ca->label ) ) != covnames.end( ) || ( ca->data_col_name != NULL && strlen( ca->data_col_name ) > 0 && ( it = covnames.find( ca->data_col_name ) ) != covnames.end( ) ) )
		{
			csvnames.emplace_back( *it );
			ca->cov_idx = k++;
		}
		else
			return 5;
	}

	// signal unused data (warning only)
	if ( covnames.size( ) > csvnames.size( ) )
		res = -1;

	// build proper matrix, discarding unused data
	dsp_mat.resize( k, k );
	for ( i = 0; i < k; ++i )
		for ( j = 0; j < k; ++j )
			try
			{
				dsp_mat( i, j ) = csv.GetCell < double > ( csvnames[ i ], csvnames[ j ] );
			}
			catch ( ... )
			{
				dsp_mat.resize( 0, 0 );
				return 6;
			}

	// check if matrix is positive definite
	e_vecT e = dsp_mat.selfadjointView< Eigen::Lower >( ).eigenvalues( );
	if ( ! std::all_of( e.begin( ), e.end( ), [ ] ( double x ) { return x > 0; } ) )
		return 7;

	// Cholesky decomposition
	dsp_chol = dsp_mat.llt( ).matrixL( );

	return res;
}


/*************************************************************
 LOAD_FILES
 Load all files containing data required for data assimilation
 *************************************************************/
bool lsd::assimilation::load_files( simulation *sim, int last_t )
{
	bool first;
	int i, j;
	assim *ca;

	// load assimilation data, if amy/proper
	if ( elem == NULL || disable )
		return false;

	if ( ( i = load_obs_data( last_t ) ) < count( 4 ) )
	{
		if ( i == 0 )
			plog_master( "\nNo data for assimilation found" );
		else
		{
			plog_master( "\nData for assimilation missing for:" );
			for ( ca = elem, first = true; ca != NULL; ca = ca->next )
				if ( ! ca->param && ! ca->disable && ca->data_obs && ca->no_data )
				{
					plog_master( "%s %s", first ? "" : ",", ca->label );
					first = false;
				}
		}

		cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"Cannot load assimilation data\" -detail \"Part or all data for assimilation could not be retrieved from data files.\nPlease check your assimilation configuration.\"" );
	}

	if ( i > 0 )
	{
		cmd( "set c %s", med_stats ? "comedian" : "covariance" );

		if ( use_dsp_file )
			if ( ( j = load_dsp_mat( sim ) ) <= 0 )
			{
				if ( j == -1 )
					plog_master( "\nUnused data in $c matrix ignored" );
			}
			else
			{
				empty( );
				switch ( j )
				{
					case 1:
						plog_master( "\nInvalid $c matrix file name" );
						break;

					case 2:
						plog_master( "\nInvalid $c matrix file CSV format" );
						break;

					case 3:
						plog_master( "\nNon-symmetric $c matrix (rows != columns)" );
						break;

					case 4:
						plog_master( "\nEmpty $c matrix" );
						break;

					case 5:
						plog_master( "\nMissing variable(s) in $c matrix" );
						break;

					case 6:
						plog_master( "\nMissing elements in $c matrix" );
						break;

					case 7:
						plog_master( "\nNon positive-definite $c matrix" );
				}

				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Cannot load assimilation $c matrix\" -detail \"There was a problem loading the $c matrix for data assimilation from file '%s'.\nCheck the Log window for details.\"", dsp_file != NULL ? dsp_file : "(none)" );
				return false;
			}
		else
		{
			switch ( j = calc_dsp_mat( ) )
			{
				case 1:
					plog_master( "\nNull $c matrix, virtual observations disabled" );
					break;

				case 2:
					plog_master( "\nNon definite-positive comedian matrix, using covariance matrix instead" );
					break;

				case 3:
					plog_master( "\nNon definite-positive $c matrix, virtual observations disable" );
			}

			if ( sav_dsp && dsp_mat.cols( ) > 0 && sim != NULL )
			{
				char fname[ MAX_PATH_LENGTH ];
				char *cpath = ( sim->alt_path == NULL || strlen ( sim->alt_path ) == 0 ) ? sim->conf_path : sim->alt_path;
				char *cname = sim->conf_name;

				if ( cname != NULL && strlen( cname ) > 0 )
				{
					int n = dsp_mat.cols( );
					rapidcsv::Document csv( "", rapidcsv::LabelParams( 0, 0 ) );

					// create empty CSV
					for ( auto i = 0; i < n; ++i )
					{
						csv.InsertColumn< double >( i, d_vecT ( ), data_lab[ i ] );
						csv.InsertRow< double >( i, d_vecT ( ), data_lab[ i ] );
					}

					// populate csv
					for ( auto i = 0; i < n; ++i )
						for ( auto j = i; j < n; ++j )		// symmetrical matrix
						{
							csv.SetCell( i, j, dsp_mat( i, j ) );
							csv.SetCell( j, i, dsp_mat( j, i ) );
						}

					if ( cpath != NULL && strlen( cpath ) > 0 )
						snprintf( fname, MAX_PATH_LENGTH, "%s/%s_data_co%c.csv", cpath, cname, ( med_stats && j == 0 ) ? 'm' : 'v' );
					else
						snprintf( fname, MAX_PATH_LENGTH, "%s_data_co%c.csv", cname, ( med_stats && j == 0 ) ? 'm' : 'v' );

					csv.Save( fname );
				}
			}
		}
	}
	else
	{
		plog_master( "\nData assimilation configuration is invalid, ignoring\n" );
		return false;
	}

	plog_master( "\nAssimilation data loaded for %d variables\n", i );

	return true;
}


/*************************************************************
 SHOW
 Print elements in data assimilation linked list to log window
 *************************************************************/
void lsd::assimilation::show( void )
{
	assim *ca;

	plog_master( "\n\nVariables set for data assimilation (#=not updated / *=disabled):\n" );
	for ( ca = elem; ca != NULL; ca = ca->next )
	{
		plog_master( "%s: %s", ca->param ? "Par" : "Var", ca->label );
		plog_tag_master( "%s%s", "highlight", ! ca->update ? "#" : "", ca->disable ? "*" : "" );

		if ( ! ca->param )
		{
			if ( ca->data_obs )
			{
				plog_master( " \t%s\t(col=", ca->data_file != NULL ? ca->data_file : "" );

				if ( ca->data_col_name != NULL && strlen( ca->data_col_name ) != 0 )
					plog_tag_master( "'%s'", "highlight", ca->data_col_name );
				else
					plog_tag_master( "%d", "highlight", ca->data_col_num );

				if ( ( ca->t_col_name != NULL && strlen( ca->t_col_name ) != 0 ) || ca->t_col_num > 0 )
				{
					plog_master( " t_col=" );

					if ( ca->t_col_name != NULL && strlen( ca->t_col_name ) != 0 )
						plog_tag_master( "'%s'", "highlight", ca->t_col_name );
					else
						plog_tag_master( "%d", "highlight", ca->t_col_num );
				}

				plog_master( ")" );
			}
		}
		else
		{
			if ( ca->par_dist == 0 )
			{
				plog_master( " \tNorm(sd=" );
				plog_tag_master( "%.4g", "highlight", ca->par_n_sd );
			}
			else
			{
				plog_master( " \tUnif(up=" );
				plog_tag_master( "%.3g", "highlight", ca->par_u_upp );
				plog_master( " lo=" );
				plog_tag_master( "%.3g", "highlight", ca->par_u_low );
			}

			plog_master( ")" );
		}

		plog_master( "\n" );
	}
}
