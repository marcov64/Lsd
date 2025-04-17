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
 ASSINSTANCE constructor
 Prepare container for element data produced by assimilation
 *************************************************************/
lsd::assinstance::assinstance( int _start, int _end, bool sav_fct, bool sav_obs )
{
	cur_t = start = _start;
	end = _end;

	int sz = end - start + 1;
	anl.assign( sz, NAN );

	if ( sav_fct )
		fct.assign( sz, NAN );

	if ( sav_obs )
		obs.assign( sz, NAN );

	if ( da->sav_ci )
	{
		anl_hi.assign( sz, NAN );
		anl_lo.assign( sz, NAN );

		if ( sav_fct )
		{
			fct_hi.assign( sz, NAN );
			fct_lo.assign( sz, NAN );
		}

		if ( sav_obs )
		{
			obs_hi.assign( sz, NAN );
			obs_lo.assign( sz, NAN );
		}
	}
}


/*************************************************************
 ASSIM constructor
 Add or update data assimilation settings for a model element
 *************************************************************/
lsd::assim::assim( const strT & _label, bool _param, bool _disable, bool _update, bool _data_obs, const strT & _data_file, const strT & _data_col_name, const strT & _t_col_name, int _data_col_num, int _t_col_num, int _par_dist, double _par_n_sd, double _par_u_upp, double _par_u_low )
{
	variable *cv;

	if ( da == NULL )
		throw std::domain_error( "assimilation object not registered" );

	if ( _label.size( ) > 0 )
	{
		label = _label;

		if ( sims.size( ) > 0 && sims[ 0 ] != NULL )
			if ( ( cv = sims[ 0 ]->root->search_var( NULL, label.c_str( ), true ) ) != NULL )
				parent = cv->up->attr->label;
	}

	param = _param;
	disable = _disable;
	update = _update;
	data_obs = _data_obs;

	if ( _data_file.size( ) > 0 )
	{
		data_file = _data_file;

		if ( _data_col_name.size( ) > 0 )
			data_col_name = _data_col_name;
		else
			data_col_num = _data_col_num;

		if ( _t_col_name.size( ) > 0 )
			t_col_name = _t_col_name;
		else
			t_col_num = _t_col_num;
	}

	par_dist = _par_dist;
	par_n_sd = _par_n_sd;
	par_u_upp = _par_u_upp;
	par_u_low = _par_u_low;

	no_data = param || disable || ! _data_obs || data_file.size( ) == 0 || ( data_col_name.size( ) == 0 && data_col_num < 1 );
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
	sum_erra_anl = sum_erra_fct = sum_erra_obs = 0;
	sum_err2_anl = sum_err2_fct = sum_err2_obs = 0;
	sum_n = sum_n_obs = 0;

	// set the variable flags to current ones
	if ( parent.size( ) > 0 )
		cur = da->ref_sim->root->search( parent.c_str( ) );
	else
		cur = NULL;

	if ( cur == NULL )
		cur = da->ref_sim->root;	// no parent hint, start from root

	if ( ( cv = cur->search_var( NULL, label.c_str( ), true ) ) == NULL )
		return false;

	param = cv->param;
	plot = cv->plot;

	if ( ( save = cv->attr->save ) )
		++( da->ref_sim->series_saved );

	// count initial instances
	for ( inst_ini = 0; cv != NULL; ++inst_ini, cv = cv->hyper_next( ) );

	return true;
}


/*************************************************************
 FINISH
 Finish assimilation element data collection after running
 *************************************************************/
void lsd::assim::finish( void )
{
	for ( auto & i : da_data )
		if ( i.end > i.cur_t )
		{
			i.end = i.cur_t;
			int sz = i.end - i.start + 1;
			i.anl.resize( sz );

			if ( da != NULL && da->sav_fct )
				i.fct.resize( sz );

			if ( da != NULL && da->sav_obs )
				i.obs.resize( sz );
		}
}


/*************************************************************
 INIT
 Initialize data assimilation structures before running
 *************************************************************/
bool lsd::assimilation::init( simulation *ref )
{
	int nrun;

	if ( ref == NULL || ( nrun = ref->last_run ) < 2 )
		return false;
	else
		ref_sim = ref;

	// initialize assimilation elements
	ref->series_saved = 0;
	elem_map.clear( );
	for ( auto ca = ass_elem.begin( ); ca != ass_elem.end( ); ++ca )
	{
		if ( ! ca->init( ) )
			return false;

		// add to map of assimilation elements
		da->elem_map[ ca->label ] = ca;
	}

	// read data assimilation data from files
	if ( ! load_files( ref, ref_sim->last_t ) )
		return false;

	// set assimilation random number generator
	lib_prng.seed( ref->seed );

	// create simulation instances to produce realization forecasts
	run_sims.clear( );
	run_sims.reserve( nrun );
	for ( int i = 0, seed = ref->seed; i < nrun; ++i, ++seed )
	{
		// construct loading configuration file
		run_sims.emplace_back( ref->conf_name, ref->conf_path, 1 );

		// adjust instances' settings (random seed, single threaded)
		run_sims[ i ].seed = seed;
		run_sims[ i ].last_run = 1;
		run_sims[ i ].deb_t = 0;
		run_sims[ i ].stack_info = 0;
		run_sims[ i ].max_threads = 1;
		run_sims[ i ].parallel_disable = true;
		run_sims[ i ].prof_aggr_time = false;
		run_sims[ i ].no_ptr_chk = true;
		run_sims[ i ].results_alt_path( ref->alt_path );
		save_param( run_sims[ i ].root );
	}

	// clear list of produced results files
	ref_sim->res_list.clear( );

	return true;
}


/*************************************************************
 FINISH
 Finish data assimilation structures after running
 *************************************************************/
void lsd::assimilation::finish( void )
{
	for ( auto & ca : ass_elem )
		ca.finish( );

	// collect produced results files
	for ( auto & sim : run_sims )
		for ( auto & fname : sim.res_list )
			ref_sim->res_list.push_back( fname );

	run_sims.clear( );
	next_t = 0;
}


/*************************************************************
 EMPTY_ASSIMILATION
 *************************************************************/
void lsd::empty_assimilation( void )
{
	if ( da == NULL )
		return;

	da->ass_elem.clear( );
	da->elem_map.clear( );
}


/*************************************************************
 RESET_INSTS
 Reset all the element instances' counters for AoR/saving
 *************************************************************/
void lsd::assimilation::reset_insts( void )
{
	for ( auto & ca : ass_elem )
	{
		ca.inst_idx = -1;

		for ( auto & i : ca.da_data )
			i.saved = false;
	}
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
	int n = 0;
	for ( auto & ca : ass_elem )
		switch ( what )
		{
			default:
			case 0:
				++n;
				break;

			case 1:
				if ( ! ca.disable )
					++n;
				break;

			case 2:
				if ( ! ca.disable && ca.param )
					++n;
				break;

			case 3:
				if ( ! ca.disable && ! ca.param )
					++n;
				break;

			case 4:
				if ( ! ca.disable && ! ca.param && ca.data_obs )
					++n;
		}

	return n;
}


/*************************************************************
 SEARCH
 Search element in data assimilation linked list
 *************************************************************/
lsd::ass_list_itT lsd::assimilation::search( const strT & lab )
{
	ass_list_itT ca;

	if ( da == NULL )
		return ass_elem.end( );

	for ( ca = ass_elem.begin( ); ca != ass_elem.end( ); ++ca )
		if ( ca->label == lab )
			 break;

	return ca;
}

lsd::ass_list_itT lsd::assimilation::search( const char *lab )
{
	return search( strT ( lab ) );
}

/*************************************************************
 SAVE_STATE_VARS
 Collect and save the current-step state variable/parameter
 instances for the simulation run
 *************************************************************/
void lsd::asstatevars::save_state_vars( object *r )
{
	object *cur;
	st_vec.clear( );

	if ( da == NULL )
		return;

	// find all instances of variables set and enabled for DA
	for ( auto & ca : da->ass_elem )
		if ( ! ca.disable )
		{
			if ( ca.parent.size( ) > 0 )	// try to start from parent
				cur = r->search( ca.parent.c_str( ) );
			else
				cur = NULL;

			if ( cur == NULL )
				cur = r;					// no parent hint, start from root

			for ( auto cv = cur->search_var( NULL, ca.label.c_str( ), true ); cv != NULL; cv = cv->hyper_next( ) )
			{
				st_vec.emplace_back( cv );	// add to state vector of run
				ca.update_param( cv );		// handle parameter estimation
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
#define DA_IDX ( sim.da_svars.idx )
#define DA_SV ( sim.da_svars.st_vec )

void lsd::assimilation::align_state_vars( void )
{
	strT lab;

	// reset vector indexes to first variable
	miss_inst.clear( );
	for ( auto & sim : run_sims )
		DA_IDX = 0;

	// run over all base state variables
	for ( auto & ca : ass_elem )
	{
		if ( ca.disable )
			continue;

		if ( align_trim )
		{
			// look for instance mismatches
			for ( auto missing = false; ! missing; )
			{
				// check if all runs have this instance
				for ( auto & sim : run_sims )
					if ( DA_IDX >= DA_SV.size( ) || ca.label != DA_SV[ DA_IDX ]->attr->label )
					{
						missing = true;			// this run doesn't have instance
						break;
					}

				// update vector indexes, removing excess instances from all runs
				for ( auto & sim : run_sims )
					if ( missing )
						// one run missing the instance, remove all excess instances
						while ( true )			// remove all extra instances of var
							if ( DA_IDX < DA_SV.size( ) && ca.label == DA_SV[ DA_IDX ]->attr->label )
								DA_SV.erase( DA_SV.begin( ) + DA_IDX );
							else
								break;			// stop on first var after or last var
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
				for ( auto & sim : run_sims )
				{	// if not, add virtual instance
					if ( DA_IDX >= DA_SV.size( ) || ca.label != DA_SV[ DA_IDX ]->attr->label )
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
				for ( auto & sim : run_sims )
					if ( DA_IDX < DA_SV.size( ) && ca.label == DA_SV[ DA_IDX ]->attr->label )
					{
						done = false;			// except if a run still has inst.
						break;
					}
			}
		}
	}

	// save aligned forecasted variable names
	fct_labs.clear( );
	for ( auto & cv : run_sims[ 0 ].da_svars.st_vec )
	{
		if ( cv != NULL )						// handle virtual instances
			lab = cv->attr->label;

		fct_labs.emplace_back( lab );
	}

	// ensure missing instance vector is consistent
	miss_inst.resize( fct_labs.size( ), false );
}


/*************************************************************
 UPDATE_STATE_VARS
 Update the ensemble state variables (& free parameters) to
 incorporate the DA analysis estimates
 *************************************************************/
void lsd::assimilation::update_state_vars( const e_matT & x_a_e )
{
	int nobs = run_sims.size( );
	int nvar = fct_labs.size( );
	variable *cv;

	for ( int j = 0; j < nvar; ++j )
		if ( elem_map[ fct_labs[ j ] ]->update )
			for ( int i = 0; i < nobs; ++i )
			{
				cv = run_sims[ i ].da_svars.st_vec[ j ];
				cv->val[ 0 ] = cv->chk_val( x_a_e( i, j ) );

				if ( cv->attr->save || cv->attr->savei )
					cv->data[ run_sims[ i ].eff_t - cv->start ] = cv->val[ 0 ];
			}
}


/*************************************************************
 UPDATE_ASSIM_VARS
 Update the DA analysis variables in LSD
 *************************************************************/
void lsd::assimilation::update_assim_vars( const e_vecT & x_a, const e_vecT & x_f, const e_vecT & z, const e_matT & x_a_e, const e_matT & x_f_e, const e_matT & z_e, int t )
{
	e_matT x_a_ci, x_f_ci, z_ci;
	int obs_idx;
	size_t i, nvar = fct_labs.size( );

	// compute confidence intervals for DA elements
	if ( da->sav_ci )
	{
		x_a_ci = ci_stat( x_a_e, x_a );
		x_f_ci = ci_stat( x_f_e, x_f );
		z_ci = ci_stat( z_e, z );
	}

	// saves each variable instance to the corresponding DA element storage
	for ( auto j = 0; j < nvar; ++j )
	{
		auto & ca = *elem_map[ fct_labs[ j ] ];
		auto co = obs_labs_map.find( fct_labs[ j ] );

		if ( co != obs_labs_map.end( ) )
			obs_idx = co->second;
		else
			obs_idx = -1;

		if ( obs_idx >= 0 )
		{
			// update error accumulators
			ca.sum_erra_anl += std::abs( x_a[ j ] - z[ obs_idx ] );
			ca.sum_erra_fct += std::abs( x_f[ j ] - z[ obs_idx ] );
			ca.sum_err2_anl += std::pow( x_a[ j ] - z[ obs_idx ], 2 );
			ca.sum_err2_fct += std::pow( x_f[ j ] - z[ obs_idx ], 2 );
			++ ca.sum_n;

			for ( auto k = 0; k < z_e.rows( ); ++k )
			{
				ca.sum_erra_obs += std::abs( z_e( k, obs_idx ) - z[ obs_idx ] );
				ca.sum_err2_obs += std::pow( z_e( k, obs_idx ) - z[ obs_idx ], 2 );
				++ ca.sum_n_obs;
			}
		}

		if ( ! ca.save )
			continue;

		// find the proper instance to update
		for ( i = 0; i < ca.da_data.size( ); ++i )
			if ( ca.da_data[ i ].cur_t < t )			// check existing slots
				break;

		if ( i == ca.da_data.size( ) )					// all used, create new
			ca.da_data.emplace_back( t, ref_sim->last_t, da->sav_fct, da->sav_obs && obs_idx >= 0 );

		ca.da_data[ i ].anl[ t - ca.da_data[ i ].start ] = x_a[ j ];

		if ( da->sav_fct )
			ca.da_data[ i ].fct[ t - ca.da_data[ i ].start ] = x_f[ j ];

		if ( da->sav_obs && obs_idx >= 0 )
			ca.da_data[ i ].obs[ t - ca.da_data[ i ].start ] = z[ obs_idx ];

		if ( da->sav_ci )
		{
			ca.da_data[ i ].anl_hi[ t - ca.da_data[ i ].start ] = x_a_ci( 0, j );
			ca.da_data[ i ].anl_lo[ t - ca.da_data[ i ].start ] = x_a_ci( 1, j );

			if ( da->sav_fct )
			{
				ca.da_data[ i ].fct_hi[ t - ca.da_data[ i ].start ] = x_f_ci( 0, j );
				ca.da_data[ i ].fct_lo[ t - ca.da_data[ i ].start ] = x_f_ci( 1, j );
			}

			if ( da->sav_obs && obs_idx >= 0 )
			{
				ca.da_data[ i ].obs_hi[ t - ca.da_data[ i ].start ] = z_ci( 0, obs_idx );
				ca.da_data[ i ].obs_lo[ t - ca.da_data[ i ].start ] = z_ci( 1, obs_idx );
			}
		}

		ca.da_data[ i ].cur_t = ca.da_data[ i ].end = t;
	}
}


/*************************************************************
 SAVE_PARAM
 Save initial values of parameters being estimated
 *************************************************************/
void lsd::assimilation::save_param( object *r )
{
	// find all instances of parameters requiring saving initial values
	for ( auto & ca : ass_elem )
		if ( ! ca.disable && ca.param && ca.update && ca.par_dist == 1 )
			for ( auto cv = r->search_var( NULL, ca.label.c_str( ), true ); cv != NULL; cv = cv->hyper_next( ) )
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
void lsd::assimilation::update_runtime_plot( int t )
{
	double cur_val_anl, cur_val_fct, cur_val_obs, last_val_anl, last_val_fct, last_val_obs;

	if ( ref_sim->liblnk == NULL || ref_sim->liblnk->plot_runtime == NULL )
		return;

	for ( auto & ca : ass_elem )
		if ( ca.plot )
			// plot up to just the initial instances
			for ( auto i = 0; i < ca.inst_ini; ++i )
			{
				if ( i >= ( int ) ca.da_data.size( ) )
					cur_val_anl = cur_val_fct = cur_val_obs = last_val_anl = last_val_fct = last_val_obs = NAN;
				else
				{
					cur_val_anl = ca.da_data[ i ].anl[ t - ca.da_data[ i ].start ];
					cur_val_fct = sav_fct ? ca.da_data[ i ].fct[ t - ca.da_data[ i ].start ] : NAN;
					cur_val_obs = sav_obs ? ca.da_data[ i ].obs[ t - ca.da_data[ i ].start ] : NAN;

					if ( ca.param == 1 || t <= ca.da_data[ i ].start )
						last_val_anl = last_val_fct = last_val_obs = NAN;
					else
					{
						last_val_anl = ca.da_data[ i ].anl[ t - ca.da_data[ i ].start - 1 ];
						last_val_fct = sav_fct ? ca.da_data[ i ].fct[ t - ca.da_data[ i ].start - 1 ] : NAN;
						last_val_obs = sav_obs ? ca.da_data[ i ].obs[ t - ca.da_data[ i ].start - 1 ] : NAN;
					}
				}

				ref_sim->liblnk->plot_runtime( NULL, t, cur_val_anl, last_val_anl );

				if ( sav_fct )
					ref_sim->liblnk->plot_runtime( NULL, t, cur_val_fct, last_val_fct );

				if ( sav_obs )
					ref_sim->liblnk->plot_runtime( NULL, t, cur_val_obs, last_val_obs );
			}
}


/*************************************************************
 ANALYSIS
 Perform data assimilation analysis step
 *************************************************************/
 int lsd::assimilation::analysis( const ass_vecT & dvars, int t )
{
	// reconcile/align the state vectors along all simulation runs
	align_state_vars( );

	int nobs = run_sims.size( );
	int nvar = fct_labs.size( );
	e_matT K, x_a_e( nobs, nvar );

	// create the forecast ensemble matrix (N x L)
	const e_matT & x_f_ens = ensemble_forecast( );

	// produce the forecast ensemble location statistics (L x 1)
	e_vecT x_f = loc_stat( x_f_ens );

	// apply ensemble inflation if enabled
	const e_matT & x_f_e = ensemble_inflation( x_f_ens, x_f );

	// produce the forecast ensemble dispersion statistics (L x L)
	const e_matT & P_f = dsp_stat( x_f_e, x_f );

	// create the forward model matrix (P x L)
	const e_matT & H = forward_matrix( dvars );

	// create virtual observations (N x P)
	const e_vecT & z = data_obs( dvars, t );
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
	e_vecT x_a = loc_stat( x_a_e );
	update_assim_vars( x_a, x_f, z, x_a_e, x_f_e, z_e, t );

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
e_vecT lsd::assimilation::loc_stat( const e_matT & x )
{
	int nvar = x.cols( );
	e_vecT x_bar( nvar );

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
 CI_STAT
 Compute the confidence interval matrix (for mean or median)
 *************************************************************/
e_matT lsd::assimilation::ci_stat( const e_matT & x, const e_vecT & x_bar )
{
	int nobs = x.rows( ), nvar = x.cols( );
	e_matT x_ci( 2, nvar );

	if ( sav_ci )
	{
		double l, t_crit = ref_sim->t_star( nobs - 1, conf_lev );
		e_vecT d( nobs );

		for ( auto i = 0; i < nvar; ++i )
		{
			if ( med_stats )
			{
				d = ( x.col( i ).array( ) - x_bar[ i ] ).abs( );
				double mad = median( d.begin( ), d.end( ) );
				l = t_crit * 1.4826 * mad / std::sqrt( nobs );
			}
			else
			{
				d = x.col( i ).array( ) - x_bar[ i ];
				double sd = std::sqrt( d.dot( d.transpose( ) ) / ( x.rows( ) - 1. ) );
				l = t_crit * sd / std::sqrt( nobs );
			}

			x_ci( 0, i ) = x_bar[ i ] + l;
			x_ci( 1, i ) = x_bar[ i ] - l;
		}
	}

	return x_ci;
}


/*************************************************************
 PLOG_STATS
 Present final DA statistics in the log window
 *************************************************************/
void lsd::assimilation::plog_stats( void )
{
	if ( ref_sim == NULL )
		return;

	ref_sim->plog( "\nData assimilation statistics (runs=%d observations=%d)\n", ref_sim->last_run, time_var.size( ) );
	ref_sim->plog( "\n               \tAnalysis\t\tForecast\t\tVirtual observations" );
	ref_sim->plog( "\nElement        \tRMSE\tMAE\tRMSE\tMAE\tRMSE\tMAE\tSamples" );

	for ( auto & lab : fct_labs )
	{
		auto & ca = *elem_map[ lab ];

		if ( ca.sum_n == 0 )
			continue;

		double rmse_anl = std::sqrt( ca.sum_err2_anl / ca.sum_n  );
		double rmse_fct = std::sqrt( ca.sum_err2_fct / ca.sum_n  );
		double rmse_obs = std::sqrt( ca.sum_err2_obs / ca.sum_n_obs );
		double mae_anl = ca.sum_erra_anl / ca.sum_n;
		double mae_fct = ca.sum_erra_fct / ca.sum_n;
		double mae_obs = ca.sum_erra_obs / ca.sum_n_obs;

		if ( ca.sum_n_obs > 0 )
			ref_sim->plog( "\n%-15s\t%.4g\t%.4g\t%.4g\t%.4g\t%.4g\t%.4g\t%d/%d", lab.c_str( ), rmse_anl, mae_anl, rmse_fct, mae_fct, rmse_obs, mae_obs, ca.sum_n, ca.sum_n_obs );
		else
			ref_sim->plog( "\n%-15s\t%.4g\t%.4g\t%.4g\t%.4g\t     \t     \t%d", lab.c_str( ), rmse_anl, mae_anl, rmse_fct, mae_fct, ca.sum_n );
	}

	ref_sim->plog( "\n" );
}


/*************************************************************
 DSP_STAT
 Compute the dispersion statistic matrix (covariance or comedian)
 *************************************************************/
const e_matT & lsd::assimilation::dsp_stat( const e_matT & x, const e_vecT & x_bar )
{
	int nvar = x.cols( );
	e_vecT d( nvar );
	static e_matT dsp;

	dsp = e_matT::Zero( nvar, nvar );

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
		for ( auto & row : x.rowwise( ) )
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
	int nvar = fct_labs.size( );
	static e_matT x_f_ens;
	variable *sv;

	x_f_ens.resize( nobs, nvar );

	// compute average/median for columns missing instances
	if ( ! align_trim )
		for ( size_t j = 0; j < miss_inst.size( ); ++j )
			if ( miss_inst[ j ] > 0 )
			{
				d_vecT v;
				v.reserve( miss_inst[ j ] );

				// get all existing instance values
				for ( auto i = 0; i < nobs; ++i )
					if ( ( sv = run_sims[ i ].da_svars.st_vec[ j ] ) != NULL && std::isfinite( sv->val[ 0 ] ) )
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
			if ( ( sv = run_sims[ i ].da_svars.st_vec[ j ] ) != NULL && std::isfinite( sv->val[ 0 ] ) )
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
	static e_matT x_f_e;

	x_f_e.resize( nobs, nvar );

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
	int nfvar = fct_labs.size( );
	static e_matT H;
	b_vecT used_fvars( nfvar, false );

	H = e_matT::Zero( ndvar, nfvar );

	// match data variables (no duplicates) to each forecast variable (incl. duplicates)
	for ( auto i = 0; i < ndvar; ++i )
	{
		i_vecT idx_fvars;
		bool found = false;
		for ( auto j = 0; j < nfvar; ++j )
		{
			if ( used_fvars[ j ] )	// avoid comparing already matched variables
				continue;

			if ( dvars[ i ]->label == fct_labs[ j ] )
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
const e_vecT & lsd::assimilation::data_obs( const ass_vecT & dvars, int t )
{
	int nvar = dvars.size( );
	static e_vecT z;

	z.resize( nvar );

	// collect observations available at current time
	for ( auto j = 0; j < nvar; ++j )
		z[ j ] = var_data[ dvars[ j ]->label ][ t ];

	return z;
}

/*************************************************************
 VIRTUAL_OBS
 Create an ensemble of virtual observations
 *************************************************************/
const e_matT & lsd::assimilation::virtual_obs( const e_vecT & z, int nobs )
{
	int nvar = z.size( );
	static e_matT z_e;
	e_vecT y( nvar );

	z_e.resize( nobs, nvar );
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
	struct assim_vars { asp_listT avl; int namrow = -1; };

	bool fexist;
	int vars_loaded = 0;
	size_t i;
	rapidcsv::Document csv;
	std::unordered_map < strT, assim_vars > fv;

	var_data.clear( );
	time_var.clear( );
	obs_labs.clear( );
	obs_labs_map.clear( );

	// identify variables to be read and group them by data file
	for ( auto & ca : ass_elem )
	{
		ca.no_data = true;

		if ( ca.param || ca.disable || ! ca.data_obs || ca.data_file.size( ) == 0 || ( ca.data_col_name.size( ) == 0 && ca.data_col_num < 1 ) )
			continue;

		fv[ ca.data_file ].avl.emplace_back( & ca );

		if ( ca.data_col_name.size( ) > 0 || ca.t_col_name.size( ) > 0 )
			fv[ ca.data_file ].namrow = 0;
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
				if ( cf->second.namrow == 0 && ( *ca )->data_col_name.size( ) > 0 )
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
					if ( cf->second.namrow == 0 && ( *ca )->t_col_name.size( ) > 0 )
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
				obs_labs.emplace_back( ( *ca )->label );
				obs_labs_map.emplace( ( *ca )->label, vars_loaded );

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
	int nvar = obs_labs.size( );
	e_matT cov_mat( nvar, nvar );				// contingency covariance matrix
	dsp_mat.resize( nvar, nvar );

	// handle each pair of variables independently to allow for different timings
	for ( auto i = 0; i < nvar; ++i )
	{
		auto x = var_data[ obs_labs[ i ] ];

		for ( auto j = i; j < nvar; ++j )		// symmetrical matrix
		{
			auto y = var_data[ obs_labs[ j ] ];

			e_vecT xv( x.size( ) );
			e_vecT yv( y.size( ) );

			int n = 0;
			for ( auto & xt : x )
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
int lsd::assimilation::load_dsp_mat( simulation *sim, strT & missing )
{
	char *cpath, fname[ MAX_PATH_LENGTH ];
	int i, j, k = 0, res = 0;
	rapidcsv::Document csv;
	std::unordered_set < strT > covnames;
	std::unordered_set < strT >::iterator it;
	str_vecT csvnames;

	missing = "";

	dsp_mat.resize( 0, 0 );
	for ( auto & ca : ass_elem )
		ca.cov_idx = -1;

	if ( dsp_file.size( ) == 0 )
		return 1;

	cpath = sim == NULL ? NULL : sim->conf_path;
	if ( cpath != NULL && strlen( cpath ) > 0 )
		snprintf( fname, MAX_PATH_LENGTH, "%s/%s", cpath, dsp_file.c_str( ) );
	else
		strcpyn( fname, dsp_file.c_str( ), MAX_PATH_LENGTH );

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
	for ( auto & ca : ass_elem )
	{
		if ( ca.no_data )
			continue;

		if ( ( it = covnames.find( ca.label ) ) != covnames.end( ) || ( ca.data_col_name.size( ) > 0 && ( it = covnames.find( ca.data_col_name ) ) != covnames.end( ) ) )
		{
			csvnames.emplace_back( *it );
			ca.cov_idx = k++;
		}
		else
		{
			missing = ca.label;
			return 5;
		}
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
				missing = csvnames[ i ];
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
	bool first = true;
	int i, j;
	strT missing;

	// load assimilation data, if amy/proper
	if ( ass_elem.size( ) == 0 || disable )
		return false;

	if ( ( i = load_obs_data( last_t ) ) < count( 4 ) )
	{
		if ( i == 0 )
			plog_master( "\nNo data for assimilation found" );
		else
		{
			plog_master( "\nData for assimilation missing for:" );
			for ( auto & ca : ass_elem )
				if ( ! ca.param && ! ca.disable && ca.data_obs && ca.no_data )
				{
					plog_master( "%s %s", first ? "" : ",", ca.label.c_str( ) );
					first = false;
				}
		}

		cmd( "ttk::messageBox -parent . -type ok -icon warning -title Warning -message \"Cannot load assimilation data\" -detail \"Part or all data for assimilation could not be retrieved from data files.\nPlease check your assimilation configuration.\"" );
	}

	if ( i > 0 )
	{
		cmd( "set c %s", med_stats ? "comedian" : "covariance" );

		if ( use_dsp_file )
			if ( ( j = load_dsp_mat( sim, missing ) ) <= 0 )
			{
				if ( j == -1 )
					plog_master( "\nUnused data in $$c matrix ignored" );
			}
			else
			{
				empty_assimilation( );
				switch ( j )
				{
					case 1:
						plog_master( "\nInvalid $$c matrix file name\n" );
						break;

					case 2:
						plog_master( "\nInvalid $$c matrix file CSV format\n" );
						break;

					case 3:
						plog_master( "\nNon-symmetric $$c matrix (rows != columns)\n" );
						break;

					case 4:
						plog_master( "\nEmpty $$c matrix\n" );
						break;

					case 5:
						plog_master( "\nMissing element in $$c matrix: %s\n", missing.c_str( ) );
						break;

					case 6:
						plog_master( "\nMissing row/column in $$c matrix: %s\n", missing.c_str( ) );
						break;

					case 7:
						plog_master( "\nNon positive-definite $$c matrix\n" );
				}

				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Cannot load assimilation $c matrix\" -detail \"There was a problem loading the $c matrix for data assimilation from file '%s'.\nCheck the Log window for details.\"", dsp_file != "" ? dsp_file.c_str( ) : "(none)" );
				return false;
			}
		else
		{
			switch ( j = calc_dsp_mat( ) )
			{
				case 1:
					plog_master( "\nNull $$c matrix, virtual observations disabled" );
					break;

				case 2:
					plog_master( "\nNon definite-positive comedian matrix, using covariance matrix instead" );
					break;

				case 3:
					plog_master( "\nNon definite-positive $$c matrix, virtual observations disable" );
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
						csv.InsertColumn< double >( i, d_vecT ( ), obs_labs[ i ] );
						csv.InsertRow< double >( i, d_vecT ( ), obs_labs[ i ] );
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

	plog_master( "\nAssimilation data loaded for %d variables", i );

	return true;
}


/*************************************************************
 SHOW
 Print elements in data assimilation linked list to log window
 *************************************************************/
void lsd::assimilation::show( void )
{
	plog_master( "\n\nVariables set for data assimilation (#=not updated / *=disabled):\n" );
	for ( auto & ca : ass_elem )
	{
		plog_master( "%s: %s", ca.param ? "Par" : "Var", ca.label.c_str( ) );
		plog_tag_master( "%s%s", "highlight", ! ca.update ? "#" : "", ca.disable ? "*" : "" );

		if ( ! ca.param )
		{
			if ( ca.data_obs )
			{
				plog_master( " \t%s\t(col=", ca.data_file.c_str( ) );

				if ( ca.data_col_name.size( ) > 0 )
					plog_tag_master( "'%s'", "highlight", ca.data_col_name.c_str( ) );
				else
					plog_tag_master( "%d", "highlight", ca.data_col_num );

				if ( ca.t_col_name.size( ) > 0 || ca.t_col_num > 0 )
				{
					plog_master( " t_col=" );

					if ( ca.t_col_name.size( ) > 0 )
						plog_tag_master( "'%s'", "highlight", ca.t_col_name.c_str( ) );
					else
						plog_tag_master( "%d", "highlight", ca.t_col_num );
				}

				plog_master( ")" );
			}
		}
		else
		{
			if ( ca.par_dist == 0 )
			{
				plog_master( " \tNorm(sd=" );
				plog_tag_master( "%.4g", "highlight", ca.par_n_sd );
			}
			else
			{
				plog_master( " \tUnif(up=" );
				plog_tag_master( "%.3g", "highlight", ca.par_u_upp );
				plog_master( " lo=" );
				plog_tag_master( "%.3g", "highlight", ca.par_u_low );
			}

			plog_master( ")" );
		}

		plog_master( "\n" );
	}
}
