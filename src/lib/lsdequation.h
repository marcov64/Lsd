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
 LSDEQUATION.H
 This file contains all the macros required by the LSD
 model's equation file.
 *************************************************************/

#define LSDEQUATION

#define _EQ_				// comment this line to access internal LSD functions
#include "lib/check.h"		// macro check support code

// enable pointer checking to protect users (medium overhead) if not disabled
#ifndef NO_POINTER_CHECK
	const bool lsd::no_pointer_check = false;

	#define CHK_PTR_NOP( O ) if ( _chk_ptr_( O ) ) _bad_ptr_void_( O, __FILE__, __LINE__ );
	#define CHK_PTR_CHR( O ) _chk_ptr_( O ) ? _bad_ptr_chr_( O, __FILE__, __LINE__ ) :
	#define CHK_PTR_DBL( O ) _chk_ptr_( O ) ? _bad_ptr_dbl_( O, __FILE__, __LINE__ ) :
	#define CHK_PTR_LNK( O ) _chk_ptr_( O ) ? _bad_ptr_lnk_( O, __FILE__, __LINE__ ) :
	#define CHK_PTR_OBJ( O ) _chk_ptr_( O ) ? _bad_ptr_obj_( O, __FILE__, __LINE__ ) :
	#define CHK_PTR_POBJ( O ) _chk_ptr_( O ) || _chk_ptr_( O->up ) ? _bad_ptr_obj_( O, __FILE__, __LINE__ ) :
	#define CHK_PTR_VOID( O ) _chk_ptr_( O ) ? _bad_ptr_void_( O, __FILE__, __LINE__ ) :
	#define CHK_OBJ_OBJ( O ) _chk_obj_( O ) ? _bad_ptr_obj_( O, __FILE__, __LINE__ ) :
	#define CHK_HK_OBJ( O, X ) _chk_hook_( O, X ) ? _no_hook_obj_( O, X, __FILE__, __LINE__ ) :
#else
	const bool lsd::no_pointer_check = true;

	#define CHK_PTR_NOP( O )
	#define CHK_PTR_CHR( O )
	#define CHK_PTR_DBL( O )
	#define CHK_PTR_LNK( O )
	#define CHK_PTR_OBJ( O )
	#define CHK_PTR_POBJ( O )
	#define CHK_PTR_VOID( O )
	#define CHK_OBJ_OBJ( O )
	#define CHK_HK_OBJ( O, X )

	#ifdef NO_POINTER_CHECK
		#undef NO_POINTER_CHECK
	#endif
#endif

// initialize pointers to NULL to protect users (small overhead) if not disabled
#ifndef NO_POINTER_INIT
	const bool lsd::no_pointer_init = false;

	#define INIT_POINTERS \
		h = i = j = k = 0; \
		cur = cur1 = cur2 = cur3 = cur4 = cur5 = cur6 = cur7 = cur8 = cur9 = cyccur = cyccur2 = cyccur3 = NULL; \
		curl = curl1 = curl2 = curl3 = curl4 = curl5 = curl6 = curl7 = curl8 = curl9 = NULL; \
		f = NULL;
	#define CHK_LNK_DBL( O ) O == NULL ? _nul_lnk_dbl_( __FILE__, __LINE__ ) :
	#define CHK_LNK_OBJ( O ) O == NULL ? _nul_lnk_obj_( __FILE__, __LINE__ ) :
	#define CHK_LNK_VOID( O ) O == NULL ? _nul_lnk_void_( __FILE__, __LINE__ ) :
	#define CHK_NODE_CHR( O ) O->node == NULL ? _no_node_chr_( O->attr->label, __FILE__, __LINE__ ) :
	#define CHK_NODE_DBL( O ) O->node == NULL ? _no_node_dbl_( O->attr->label, __FILE__, __LINE__ ) :
#else
	const bool lsd::no_pointer_init = true;

	#define INIT_POINTERS
	#define CHK_LNK_DBL( O )
	#define CHK_LNK_OBJ( O )
	#define CHK_LNK_VOID( O )
	#define CHK_NODE_CHR( O )
	#define CHK_NODE_DBL( O )
#endif

// user defined variables for all equations (to be defined in equation file)
#ifndef EQ_USER_VARS
	#define EQ_USER_VARS
#endif

// debugger probe variables
#ifndef _TERM_
	#define DEBUG_CODE \
		if ( _sim_->deb_set ) \
		{ \
			for ( int n = 0; n < USER_D_VARS; ++n ) \
				_d_values_[ n ] = v[ n ]; \
			_i_values_[ 0 ] = i; \
			_i_values_[ 1 ] = j; \
			_i_values_[ 2 ] = h; \
			_i_values_[ 3 ] = k; \
			_o_values_[ 0 ] = cur; \
			_o_values_[ 1 ] = cur1; \
			_o_values_[ 2 ] = cur2; \
			_o_values_[ 3 ] = cur3; \
			_o_values_[ 4 ] = cur4; \
			_o_values_[ 5 ] = cur5; \
			_o_values_[ 6 ] = cur6; \
			_o_values_[ 7 ] = cur7; \
			_o_values_[ 8 ] = cur8; \
			_o_values_[ 9 ] = cur9; \
			_n_values_[ 0 ] = curl; \
			_n_values_[ 1 ] = curl1; \
			_n_values_[ 2 ] = curl2; \
			_n_values_[ 3 ] = curl3; \
			_n_values_[ 4 ] = curl4; \
			_n_values_[ 5 ] = curl5; \
			_n_values_[ 6 ] = curl6; \
			_n_values_[ 7 ] = curl7; \
			_n_values_[ 8 ] = curl8; \
			_n_values_[ 9 ] = curl9; \
			_f_values_[ 0 ] = f; \
		};
#else
	#define DEBUG_CODE
#endif

// create map for fast equation look-up
#define MODELBEGIN \
	double lsd::equation::_fun_( variable *v, object *caller ) \
	{ \
		if ( _sim_->quit == 2 ) \
			return v->val[ 0 ]; \
		if ( v->attr->eq_func == NULL ) \
			v->attr->eq_func = _chk_eq_( v->attr->label ); \
		return v->chk_res( ( v->attr->eq_func )( v, caller ) ); \
	} \
	void lsd::equation::_init_map_( ) \
	{ \
		_eq_map_ = \
		{

#define MODELEND \
		}; \
	}

#define EQUATION( X ) \
	{ strT( X ), [ this ]( const variable *_v_, object *_c_ ) -> double \
		{ \
			object *_p_ = _v_->up; \
			int h, i, j, k; \
			double v[ USER_D_VARS ]; \
			object *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cyccur, *cyccur2, *cyccur3; \
			netlink *curl, *curl1, *curl2, *curl3, *curl4, *curl5, *curl6, *curl7, *curl8, *curl9; \
			FILE *f; \
			INIT_POINTERS \
			EQ_USER_VARS

#define RESULT( X ) \
			; \
			DEBUG_CODE \
			return X; \
		} \
	},

#define END_EQUATION( X ) \
	{ \
		DEBUG_CODE \
		return X; \
	}

#define EQUATION_DUMMY( X, Y ) \
	{ strT( X ), [ ]( const variable *_v_, object *_c_ ) -> double \
		{ \
			return ( ( variable * ) _v_ )->chk_dummy( Y ); \
		} \
	},

// simulation close code
#ifndef LEGACY_CODE
#define CLOSEBEGIN \
	void lsd::equation::_close_sim_( void ) \
	{

#define CLOSEEND \
	} \
	void close_sim( void ) { }
#endif

// macros to handle user-defined C functions to be used in equations
#ifdef LSDCFUNINIT
	#undef CFUN_DBL
	#undef CFUN_INT
	#undef CFUN_OBJ
	#undef CFUN_VOID
	#define CFUN_DBL( N, ... ) double lsd::equation::N( lsd::object *_p_, const lsd::variable *_v_, lsd::object *_c_ __VA_OPT__( , ) __VA_ARGS__ )
	#define CFUN_INT( N, ... ) int lsd::equation::N( lsd::object *_p_, const lsd::variable *_v_, lsd::object *_c_ __VA_OPT__( , ) __VA_ARGS__ )
	#define CFUN_OBJ( N, ... ) lsd::object *lsd::equation::N( lsd::object *_p_, const lsd::variable *_v_, lsd::object *_c_ __VA_OPT__( , ) __VA_ARGS__ )
	#define CFUN_VOID( N, ... ) void lsd::equation::N( lsd::object *_p_, const lsd::variable *_v_, lsd::object *_c_ __VA_OPT__( , ) __VA_ARGS__ )
	#define CFUN( N, ... ) lsd::equation::N( _p_, _v_, _c_ __VA_OPT__( , ) __VA_ARGS__ )
	#define CFUNS( O, N, ... ) lsd::equation::N( CHK_PTR_OBJ( O ) O, _v_, _c_ __VA_OPT__( , ) __VA_ARGS__ )
#endif

// LSD macros
#define pi M_PI
#define abs( x ) std::fabs( x )
#define exp( x ) std::exp( x )
#define log( x ) std::log( x )
#define log10( x ) std::log10( x )
#define max( x, y ) std::fmax( x, y )
#define min( x, y ) std::fmin( x, y )
#define pow( x, y ) std::pow( x, y )
#define round( x ) std::round( x )
#define sin( x ) std::sin( x )
#define cos( x ) std::cos( x )
#define tan( x ) std::tan( x )
#define asin( x ) std::asin( x )
#define acos( x ) std::acos( x )
#define atan( x ) std::atan( x )
#define sqrt( x ) std::sqrt( x )
#define tgamma( x ) std::tgamma( x )
#define lgamma( x ) std::lgamma( x )
#define is_finite( x ) std::isfinite( x )
#define is_inf( x ) std::isinf( x )
#define is_nan( x ) std::isnan( x )

#define UP "UP"
#define DOWN "DOWN"

#define ABORT _quit_( 1 )
#define FAST _fast_( 1 )
#define FAST_FULL _fast_( 2 )
#define OBSERVE _fast_( 0 )
#define PARAMETER _param_( _v_, 1 )

#define NO_NAN _use_nan_( false )
#define USE_NAN _use_nan_( true )
#define NO_POINTER_CHECK _use_pointer_check_( false )
#define USE_POINTER_CHECK _use_pointer_check_( true )
#define NO_SAVED _no_saved_( true )
#define USE_SAVED _no_saved_( false )
#define NO_SEARCH _no_search_( true )
#define USE_SEARCH _no_search_( false )
#define NO_SEARCH_UP _no_search_up_( true )
#define USE_SEARCH_UP _no_search_up_( false )
#define NO_ZERO_INSTANCE _no_zero_inst_( true )
#define USE_ZERO_INSTANCE _no_zero_inst_( false )

#define RND _ran1_( )
#define RND_SEED _seed_( -1 )
#define RND_SETSEED( X ) _seed_( ( unsigned ) X )
#define RND_GENERATOR( X ) _random_( ( unsigned ) X )
#define SLEEP( X ) _msleep_( X )

#define DEBUG_START _debug_( true, 0 )
#define DEBUG_START_AT( X ) _debug_( true, ( unsigned ) X )
#define DEBUG_STOP _debug_( false, 0 )
#define DEBUG_STOP_AT( X ) _debug_( false, ( unsigned ) X )

#define LOG( ... ) _plog_( false, __VA_ARGS__ )
#define PLOG( ... ) _plog_( true, __VA_ARGS__ )

#define NAME ( ( const char * ) _p_->attr->label )
#define NAMES( O ) ( _chk_ptr_( O ) ? NULL : ( const char * ) O->attr->label )
#define CONFIG _conf_name_( )
#define PATH _conf_path_( )

#define CURRENT _current_( _v_ )
#define T _t_( )
#define LAST_T _last_t_( )
#define RUN _run_( )
#define LAST_RUN _last_run_( )
#define LAST_CALC( X ) ( _p_->last_cal( X ) )
#define LAST_CALCS( O, X ) ( CHK_PTR_DBL( O ) O->last_cal( X ) )

#define ROOT _root_( )
#define THIS _p_
#define CALLER _c_
#define NEXT ( _p_->next )
#define NEXTS( O ) ( CHK_PTR_OBJ( O ) O->next )
#define PARENT ( _p_->up )
#define PARENTS( O ) ( CHK_PTR_OBJ( O ) O->up )
#define GRANDPARENT ( CHK_PTR_POBJ( _p_ ) _p_->up->up )
#define GRANDPARENTS( O ) ( CHK_PTR_POBJ( O ) O->up->up )

#define RECALC( X ) ( _p_->recal( X ) )
#define RECALCS( O, X ) ( CHK_PTR_DBL( O ) O->recal( X ) )
#define UPDATE ( _p_->update( false, true ) )
#define UPDATES( O ) ( CHK_PTR_VOID( O ) O->update( false, true ) )
#define UPDATE_REC ( _p_->update( true, true ) )
#define UPDATE_RECS( O ) ( CHK_PTR_VOID( O ) O->update( true, true ) )

#define V( X ) ( _p_->cal( _p_, X, 0 ) )
#define VL( X, L ) ( _p_->cal( _p_, X, L ) )
#define VS( O, X ) ( CHK_PTR_DBL( O ) O->cal( O, X, 0 ) )
#define VLS( O, X, L ) ( CHK_PTR_DBL( O ) O->cal( O, X, L ) )

#define MAVE( X, P ) ( _p_->mav( _p_, X, P, 0 ) )
#define MAVEL( X, P, L ) ( _p_->mav( _p_, X, P, L ) )
#define MAVES( O, X, P ) ( CHK_PTR_DBL( O ) O->mav( O, X, P, 0 ) )
#define MAVELS( O, X, P, L ) ( CHK_PTR_DBL( O ) O->mav( O, X, P, L ) )

#define WHTMAVE( X, P, W ) ( _p_->mav( _p_, X, P, W, 0 ) )
#define WHTMAVEL( X, P, W, L ) ( _p_->mav( _p_, X, P, W, L ) )
#define WHTMAVES( O, X, P, W ) ( CHK_PTR_DBL( O ) O->mav( O, X, P, W, 0 ) )
#define WHTMAVELS( O, X, P, W, L ) ( CHK_PTR_DBL( O ) O->mav( O, X, P, W, L ) )

#define SUM( X ) ( _p_->sum( X, 0, false, "", "", 0. ) )
#define SUML( X, L ) ( _p_->sum( X, L, false, "", "", 0. ) )
#define SUMS( O, X ) ( CHK_PTR_DBL( O ) O->sum( X, 0, false, "", "", 0. ) )
#define SUMLS( O, X, L ) ( CHK_PTR_DBL( O ) O->sum( X, L, false, "", "", 0. ) )
#define SUM_CND( X, T, R, V ) ( _p_->sum( X, 0, true, T, R, V ) )
#define SUM_CNDL( X, T, R, V, L ) ( _p_->sum( X, L, true, T, R, V ) )
#define SUM_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->sum( X, 0, true, T, R, V ) )
#define SUM_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->sum( X, L, true, T, R, V ) )

#define MAX( X ) ( _p_->overall_max( X, 0, false, "", "", 0. ) )
#define MAXL( X, L ) ( _p_->overall_max( X, L, false, "", "", 0. ) )
#define MAXS( O, X ) ( CHK_PTR_DBL( O ) O->overall_max( X, 0, false, "", "", 0. ) )
#define MAXLS( O, X, L ) ( CHK_PTR_DBL( O ) O->overall_max( X, L, false, "", "", 0. ) )
#define MAX_CND( X, T, R, V ) ( _p_->overall_max( X, 0, true, T, R, V ) )
#define MAX_CNDL( X, T, R, V, L ) ( _p_->overall_max( X, L, true, T, R, V ) )
#define MAX_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->overall_max( X, 0, true, T, R, V ) )
#define MAX_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->overall_max( X, L, true, T, R, V ) )

#define MIN( X ) ( _p_->overall_min( X, 0, false, "", "", 0. ) )
#define MINL( X, L ) ( _p_->overall_min( X, L, false, "", "", 0. ) )
#define MINS( O, X ) ( CHK_PTR_DBL( O ) O->overall_min( X, 0, false, "", "", 0. ) )
#define MINLS( O, X, L ) ( CHK_PTR_DBL( O ) O->overall_min( X, L, false, "", "", 0. ) )
#define MIN_CND( X, T, R, V ) ( _p_->overall_min( X, 0, true, T, R, V ) )
#define MIN_CNDL( X, T, R, V, L ) ( _p_->overall_min( X, L, true, T, R, V ) )
#define MIN_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->overall_min( X, 0, true, T, R, V ) )
#define MIN_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->overall_min( X, L, true, T, R, V ) )

#define AVE( X ) ( _p_->av( X, 0, false, "", "", 0. ) )
#define AVEL( X, L ) ( _p_->av( X, L, false, "", "", 0. ) )
#define AVES( O, X ) ( CHK_PTR_DBL( O ) O->av( X, 0, false, "", "", 0. ) )
#define AVELS( O, X, L ) ( CHK_PTR_DBL( O ) O->av( X, L, false, "", "", 0. ) )
#define AVE_CND( X, T, R, V ) ( _p_->av( X, 0, true, T, R, V ) )
#define AVE_CNDL( X, T, R, V, L ) ( _p_->av( X, L, true, T, R, V ) )
#define AVE_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->av( X, 0, true, T, R, V ) )
#define AVE_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->av( X, L, true, T, R, V ) )

#define WHTAVE( X, Y ) ( _p_->whg_av( X, Y, 0, false, "", "", 0. ) )
#define WHTAVEL( X, Y, L ) ( _p_->whg_av( X, Y, L, false, "", "", 0. ) )
#define WHTAVES( O, X, Y ) ( CHK_PTR_DBL( O ) O->whg_av( X, Y, 0, false, "", "", 0. ) )
#define WHTAVELS( O, X, Y, L ) ( CHK_PTR_DBL( O ) O->whg_av( X, Y, L, false, "", "", 0. ) )
#define WHTAVE_CND( X, Y, T, R, V ) ( _p_->whg_av( X, Y, 0, true, T, R, V ) )
#define WHTAVE_CNDL( X, Y, T, R, V, L ) ( _p_->whg_av( X, Y, L, true, T, R, V ) )
#define WHTAVE_CNDS( O, X, Y, T, R, V ) ( CHK_PTR_DBL( O ) O->whg_av( X, Y, 0, true, T, R, V ) )
#define WHTAVE_CNDLS( O, X, Y, T, R, V, L ) ( CHK_PTR_DBL( O ) O->whg_av( X, Y, L, true, T, R, V ) )

#define MED( X ) ( _p_->med( X, 0, false, "", "", 0. ) )
#define MEDL( X, L ) ( _p_->med( X, L, false, "", "", 0. ) )
#define MEDS( O, X ) ( CHK_PTR_DBL( O ) O->med( X, 0, false, "", "", 0. ) )
#define MEDLS( O, X, L ) ( CHK_PTR_DBL( O ) O->med( X, L, false, "", "", 0. ) )
#define MED_CND( X, T, R, V ) ( _p_->med( X, 0, true, T, R, V ) )
#define MED_CNDL( X, T, R, V, L ) ( _p_->med( X, L, true, T, R, V ) )
#define MED_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->med( X, 0, true, T, R, V ) )
#define MED_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->med( X, L, true, T, R, V ) )

#define PERC( X, P ) ( _p_->perc( X, P, 0, false, "", "", 0. ) )
#define PERCL( X, P, L ) ( _p_->perc( X, P, L, false, "", "", 0. ) )
#define PERCS( O, X, P ) ( CHK_PTR_DBL( O ) O->perc( X, P, 0, false, "", "", 0. ) )
#define PERCLS( O, X, P, L ) ( CHK_PTR_DBL( O ) O->perc( X, P, L, false, "", "", 0. ) )
#define PERC_CND( X, P, T, R, V ) ( _p_->perc( X, P, 0, true, T, R, V ) )
#define PERC_CNDL( X, P, T, R, V, L ) ( _p_->perc( X, P, L, true, T, R, V ) )
#define PERC_CNDS( O, X, P, T, R, V ) ( CHK_PTR_DBL( O ) O->perc( X, P, 0, true, T, R, V ) )
#define PERC_CNDLS( O, X, P, T, R, V, L ) ( CHK_PTR_DBL( O ) O->perc( X, P, L, true, T, R, V ) )

#define SD( X ) ( _p_->sd( X, 0, false, "", "", 0. ) )
#define SDL( X, L ) ( _p_->sd( X, L, false, "", "", 0. ) )
#define SDS( O, X ) ( CHK_PTR_DBL( O ) O->sd( X, 0, false, "", "", 0. ) )
#define SDLS( O, X, L ) ( CHK_PTR_DBL( O ) O->sd( X, L, false, "", "", 0. ) )
#define SD_CND( X, T, R, V ) ( _p_->sd( X, 0, true, T, R, V ) )
#define SD_CNDL( X, T, R, V, L ) ( _p_->sd( X, L, true, T, R, V ) )
#define SD_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->sd( X, 0, true, T, R, V ) )
#define SD_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->sd( X, L, true, T, R, V ) )

#define COUNT( X ) ( _p_->count( X, 0, false, "", "", 0. ) )
#define COUNTS( O, X ) ( CHK_PTR_DBL( O ) O->count( X, 0, false, "", "", 0. ) )
#define COUNT_CND( X, T, R, V ) ( _p_->count( X, 0, true, T, R, V ) )
#define COUNT_CNDL( X, T, R, V, L ) ( _p_->count( X, L, true, T, R, V ) )
#define COUNT_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->count( X, 0, true, T, R, V ) )
#define COUNT_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->count( X, L, true, T, R, V ) )

#define COUNT_ALL( X ) ( _p_->count_all( X, 0, false, "", "", 0. ) )
#define COUNT_ALLS( O, X ) ( CHK_PTR_DBL( O ) O->count_all( X, 0, false, "", "", 0. ) )
#define COUNT_ALL_CND( X, T, R, V ) ( _p_->count_all( X, 0, true, T, R, V ) )
#define COUNT_ALL_CNDL( X, T, R, V, L ) ( _p_->count_all( X, L, true, T, R, V ) )
#define COUNT_ALL_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->count_all( X, 0, true, T, R, V ) )
#define COUNT_ALL_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->count_all( X, L, true, T, R, V ) )

#define STAT( X ) ( _p_->stat( X, v, 0, false, "", "", 0. ) )
#define STATL( X, L ) ( _p_->stat( X, v, L, false, "", "", 0. ) )
#define STATS( O, X ) ( CHK_PTR_DBL( O ) O->stat( X, v, 0, false, "", "", 0. ) )
#define STATLS( O, X, L ) ( CHK_PTR_DBL( O ) O->stat( X, v, L, false, "", "", 0. ) )
#define STAT_CND( X, T, R, V ) ( _p_->stat( X, v, 0, true, T, R, V ) )
#define STAT_CNDL( X, T, R, V, L ) ( _p_->stat( X, v, L, true, T, R, V ) )
#define STAT_CNDS( O, X, T, R, V ) ( CHK_PTR_DBL( O ) O->stat( X, v, 0, true, T, R, V ) )
#define STAT_CNDLS( O, X, T, R, V, L ) ( CHK_PTR_DBL( O ) O->stat( X, v, L, true, T, R, V ) )

#define INTERACT( X, Y ) ( _p_->interact( X, Y, v, i, j, h, k, cur, cur1, cur2, cur3, cur4, cur5, cur6, cur7, cur8, cur9, curl, curl1, curl2, curl3, curl4, curl5, curl6, curl7, curl8, curl9, f ) )
#define INTERACTS( O, X, Y ) ( CHK_PTR_DBL( O ) O->interact( X, Y, v, i, j, h, k, cur, cur1, cur2, cur3, cur4, cur5, cur6, cur7, cur8, cur9, curl, curl1, curl2, curl3, curl4, curl5, curl6, curl7, curl8, curl9, f ) )

#define SEARCH( X ) ( _p_->search( X, false ) )
#define SEARCHS( O, X ) ( CHK_PTR_OBJ( O ) O->search( X, false ) )
#define SEARCH_CND( X, Y ) ( _p_->search_var_cond( X, Y, 0 ) )
#define SEARCH_CNDL( X, Y, L ) ( _p_->search_var_cond( X, Y, L ) )
#define SEARCH_CNDS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->search_var_cond( X, Y, 0 ) )
#define SEARCH_CNDLS( O, X, Y, L ) ( CHK_PTR_OBJ( O ) O->search_var_cond( X, Y, L ) )

#define SEARCH_INST( X ) ( _p_->search_inst( X, true ) )
#define SEARCH_INSTS( O, X ) ( CHK_PTR_DBL( O ) O->search_inst( X, true ) )

#define RNDDRAW( X, Y ) ( _p_->draw_rnd( X, Y, 0 ) )
#define RNDDRAWL( X, Y, L ) ( _p_->draw_rnd( X, Y, L ) )
#define RNDDRAWS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->draw_rnd( X, Y, 0 ) )
#define RNDDRAWLS( O, X, Y, L ) ( CHK_PTR_OBJ( O ) O->draw_rnd( X, Y, L ) )

#define RNDDRAW_FAIR( X ) ( _p_->draw_rnd( X ) )
#define RNDDRAW_FAIRS( O, X ) ( CHK_PTR_OBJ( O ) O->draw_rnd( X ) )
#define RNDDRAW_TOT( X, Y, Z ) ( _p_->draw_rnd( X, Y, 0, Z ) )
#define RNDDRAW_TOTL( X, Y, L, Z ) ( _p_->draw_rnd( X, Y, L, Z ) )
#define RNDDRAW_TOTS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->draw_rnd( X, Y, 0, Z ) )
#define RNDDRAW_TOTLS( O, X, Y, L, Z ) ( CHK_PTR_OBJ( O ) O->draw_rnd( X, Y, L, Z ) )

#define WRITE( X, Y ) ( _p_->write( X, Y, T, 0 ) )
#define WRITEL( X, Y, L ) ( _p_->write( X, Y, L, 0 ) )
#define WRITELL( X, Y, Z, L ) ( _p_->write( X, Y, Z, L ) )
#define WRITES( O, X, Y ) ( CHK_PTR_DBL( O ) O->write( X, Y, T, 0 ) )
#define WRITELS( O, X, Y, L ) ( CHK_PTR_DBL( O ) O->write( X, Y, L, 0 ) )
#define WRITELLS( O, X, Y, Z, L ) ( CHK_PTR_DBL( O ) O->write( X, Y, Z, L ) )

#define INCR( X, Y ) ( _p_->increment( X, Y ) )
#define INCRS( O, X, Y ) ( CHK_PTR_DBL( O ) O->increment( X, Y ) )

#define MULT( X, Y ) ( _p_->multiply( X, Y ) )
#define MULTS( O, X, Y ) ( CHK_PTR_DBL( O ) O->multiply( X, Y ) )

#define ADDOBJ( X ) ( _p_->add_n_objects2( X, 1, -1 ) )
#define ADDOBJL( X, L ) ( _p_->add_n_objects2( X, 1, L ) )
#define ADDOBJS( O, X ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( X, 1, -1 ) )
#define ADDOBJLS( O, X, L ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( X, 1, L ) )
#define ADDNOBJ( X, N ) ( _p_->add_n_objects2( X, N, -1 ) )
#define ADDNOBJL( X, N, L ) ( _p_->add_n_objects2( X, N, L ) )
#define ADDNOBJS( O, X, N ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( X, N, -1 ) )
#define ADDNOBJLS( O, X, N, L ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( X, N, L ) )

#define ADDOBJ_EX( X, E ) ( _p_->add_n_objects2( X, 1, E, -1 ) )
#define ADDOBJ_EXL( X, E, L ) ( _p_->add_n_objects2( X, 1, E, L ) )
#define ADDOBJ_EXS( O, X, E ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( X, 1, E, -1 ) )
#define ADDOBJ_EXLS( O, X, E, L ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( X, 1, E, L ) )
#define ADDNOBJ_EX( X, N, E ) ( _p_->add_n_objects2( X, N, E, -1 ) )
#define ADDNOBJ_EXL( X, N, E, L ) ( _p_->add_n_objects2( X, N, E, L ) )
#define ADDNOBJ_EXS( O, X, N, E ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( X, N, E, -1 ) )
#define ADDNOBJ_EXLS( O, X, N, E, L ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( X, N, E, L ) )

#define DELETE( O ) ( CHK_PTR_VOID( O ) O->delete_obj( _v_ ) )
#define DELETING ( _p_->to_delete( ) )
#define DELETINGS( O ) ( CHK_PTR_DBL( O ) O->to_delete( ) )

#define SORT( X, Y, D ) ( _p_->lsdqsort( X, Y, D, 0 ) )
#define SORTL( X, Y, D, L ) ( _p_->lsdqsort( X, Y, D, L ) )
#define SORTS( O, X, Y, D ) ( CHK_PTR_OBJ( O ) O->lsdqsort( X, Y, D, 0 ) )
#define SORTLS( O, X, Y, D, L ) ( CHK_PTR_OBJ( O ) O->lsdqsort( X, Y, D, L ) )

#define SORT2( X, Y, Z, D ) ( _p_->lsdqsort( X, Y, Z, D , 0 ) )
#define SORT2L( X, Y, Z, D, L ) ( _p_->lsdqsort( X, Y, Z, D, L ) )
#define SORT2S( O, X, Y, Z, D ) ( CHK_PTR_OBJ( O ) O->lsdqsort( X, Y, Z, D, 0 ) )
#define SORT2LS( O, X, Y, Z, D, L ) ( CHK_PTR_OBJ( O ) O->lsdqsort( X, Y, Z, D, L ) )

#define HOOK( N ) ( CHK_HK_OBJ( _p_, N ) _p_->hooks[ N ] )
#define HOOKS( O, N ) ( CHK_PTR_OBJ( O ) CHK_HK_OBJ( O, N ) O->hooks[ N ] )
#define SHOOK ( _p_->hook )
#define SHOOKS( O ) ( CHK_PTR_OBJ( O ) O->hook )

#define WRITE_HOOK( N, X ) ( CHK_HK_OBJ( _p_, N ) CHK_OBJ_OBJ( X ) _p_->hooks[ N ] = X )
#define WRITE_HOOKS( O, N, X ) ( CHK_PTR_OBJ( O ) CHK_HK_OBJ( O, N ) CHK_OBJ_OBJ( X ) O->hooks[ N ] = X )
#define WRITE_SHOOK( X ) ( CHK_OBJ_OBJ( X ) _p_->hook = X )
#define WRITE_SHOOKS( O, X ) ( CHK_PTR_OBJ( O ) CHK_OBJ_OBJ( X ) O->hook = X )

#define ADDHOOK( N ) ( _p_->hooks.resize( ( unsigned ) N ) )
#define ADDHOOKS( O, N ) ( CHK_PTR_VOID( O ) O->hooks.resize( ( unsigned ) N ) )

#define COUNT_HOOK ( _p_->hooks.size( ) )
#define COUNT_HOOKS( O ) ( CHK_PTR_DBL( O ) O->hooks.size( ) )

#define DOWN_LAT ( _p_->lat_down( ) )
#define DOWN_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_down( ) )
#define LEFT_LAT ( _p_->lat_left( ) )
#define LEFT_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_left( ) )
#define RIGHT_LAT ( _p_->lat_right( ) )
#define RIGHT_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_right( ) )
#define UP_LAT ( _p_->lat_up( ) )
#define UP_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_up( ) )

#define INIT_LAT( ... ) _init_lattice_( __VA_ARGS__ )
#define SAVE_LAT( ... ) _save_lattice_( __VA_ARGS__ )
#define DELETE_LAT _close_lattice_( true )

#define V_LAT( X, Y ) _read_lattice_( X, Y )
#define WRITE_LAT( X, ... ) _update_lattice_( X, __VA_ARGS__ )

#define V_NODEID ( CHK_NODE_DBL( _p_ ) _p_->node->id )
#define V_NODEIDS( O ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->id )
#define V_NODENAME ( CHK_NODE_CHR( _p_ ) _p_->node->name )
#define V_NODENAMES( O ) ( CHK_PTR_CHR( O ) CHK_NODE_CHR( O ) O->node->name )
#define V_LINK( L ) ( CHK_LNK_DBL( L ) L->weight )

#define STAT_NET( X ) ( _p_->stats_net( X, v ) )
#define STAT_NETS( O, X ) ( CHK_PTR_DBL( O ) O->stats_net( X, v ) )
#define STAT_NODE ( CHK_NODE_DBL( _p_ ) _p_->node->nlinks )
#define STAT_NODES( O ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->nlinks )

#define SEARCH_NODE( X, Y ) ( _p_->search_node_net( X, Y ) )
#define SEARCH_NODES( O, X, Y ) ( CHK_PTR_OBJ( O ) O->search_node_net( X, Y ) )
#define SEARCH_LINK( X ) ( _p_->search_link_net( X ) )
#define SEARCH_LINKS( O, X ) ( CHK_PTR_LNK( O ) O->search_link_net( X ) )

#define RNDDRAW_NODE( X ) ( _p_->draw_node_net( X ) )
#define RNDDRAW_NODES( O, X ) ( CHK_PTR_OBJ( O ) O->draw_node_net( X ) )
#define RNDDRAW_LINK ( _p_->draw_link_net( ) )
#define RNDDRAW_LINKS( O ) ( CHK_PTR_LNK( O ) O->draw_link_net( ) )

#define DRAWPROB_NODE( X ) ( CHK_NODE_DBL( _p_ ) _p_->node->prob = X )
#define DRAWPROB_NODES( O, X ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->prob = X )
#define DRAWPROB_LINK( L, X ) ( CHK_LNK_DBL( L ) L->probTo = X )

#define LINKTO( L ) ( CHK_LNK_OBJ( L ) L->to )
#define LINKFROM( L ) ( CHK_LNK_OBJ( L ) L->from )

#define WRITE_NODEID( X ) ( CHK_NODE_DBL( _p_ ) _p_->node->id = X )
#define WRITE_NODEIDS( O, X ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->id = X )
#define WRITE_NODENAME( X ) ( _p_->name_node_net( X ) )
#define WRITE_NODENAMES( O, X ) ( CHK_PTR_VOID( O ) O->name_node_net( X ) )
#define WRITE_LINK( L, X ) ( CHK_LNK_DBL( L ) L->weight = X )

#define INIT_NET( ... ) ( _p_->init_stub_net( __VA_ARGS__ ) )
#define INIT_NETS( O, ... ) ( CHK_PTR_DBL( O ) O->init_stub_net( __VA_ARGS__ ) )

#define LOAD_NET( X, Y ) ( _p_->read_file_net( X, "", Y, RND_SEED, "net" ) )
#define LOAD_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->read_file_net( X, "", Y, RND_SEED, "net" ) )
#define SAVE_NET( X, Y ) ( _p_->write_file_net( X, "", Y, RND_SEED, false ) )
#define SAVE_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->write_file_net( X, "", Y , RND_SEED, false ) )
#define SNAP_NET( X, Y ) ( _p_->write_file_net( X, "", Y, RND_SEED, true ) )
#define SNAP_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->write_file_net( X, "", Y, RND_SEED, true ) )

#define ADDNODE( X, Y ) ( _p_->add_node_net( X, Y, false ) )
#define ADDNODES( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_node_net( X, Y, false ) )

#define ADDLINK( X ) ( _p_->add_link_net( X, 0 , 1 ) )
#define ADDLINKS( O, X ) ( CHK_PTR_LNK( O ) O->add_link_net( X, 0 , 1 ) )
#define ADDLINKW( X, Y ) ( _p_->add_link_net( X, Y, 1 ) )
#define ADDLINKWS( O, X, Y ) ( CHK_PTR_LNK( O ) O->add_link_net( X, Y, 1 ) )

#define DELETE_NET( X ) ( _p_->delete_net( X ) )
#define DELETE_NETS( O, X ) ( CHK_PTR_VOID( O ) O->delete_net( X ) )
#define DELETE_NODE ( _p_->delete_node_net( ) )
#define DELETE_NODES( O ) ( CHK_PTR_VOID( O ) O->delete_node_net( ) )
#define DELETE_LINK( L ) ( CHK_LNK_VOID( L ) L->from->delete_link_net( L ) )

#define SHUFFLE_NET( X ) ( _p_->shuffle_nodes_net( X ) )
#define SHUFFLE_NETS( O, X ) ( CHK_PTR_OBJ( O ) O->shuffle_nodes_net( X ) )

#define INIT_TSEARCH( X ) ( _p_->initturbo( X ) )
#define INIT_TSEARCHS( O, X ) ( CHK_PTR_DBL( O ) O->initturbo( X ) )
#define TSEARCH_SET( X ) ( _p_->turboset( X ) )
#define TSEARCH_SETS( O, X ) ( CHK_PTR_DBL( O ) O->turboset( X ) )
#define TSEARCH( X, Y ) ( _p_->turbosearch( X, Y ) )
#define TSEARCHS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->turbosearch( X, Y ) )

#define INIT_TSEARCH_CND( X ) ( _p_->initturbo_cond( X ) )
#define INIT_TSEARCH_CNDS( O, X ) ( CHK_PTR_DBL( O ) O->initturbo_cond( X ) )
#define TSEARCH_CND_SET( X ) ( _p_->turboset_cond( X ) )
#define TSEARCH_CND_SETS( O, X ) ( CHK_PTR_DBL( O ) O->turboset_cond( X ) )
#define TSEARCH_CND( X, Y ) ( _p_->turbosearch_cond( X, Y ) )
#define TSEARCH_CNDS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->turbosearch_cond( X, Y ) )

#define V_CHEAT( X, Y ) ( _p_->cal( Y, X, 0 ) )
#define V_CHEATL( X, L, Y ) ( _p_->cal( Y, X, L ) )
#define V_CHEATS( O, X, Y ) ( CHK_PTR_DBL( O ) O->cal( Y, X, 0 ) )
#define V_CHEATLS( O, X, L, Y ) ( CHK_PTR_DBL( O ) O->cal( Y, X, L ) )

#define ADDEXT( C ) { if ( _p_->cext != NULL ) DELETE_EXT( C ); _p_->cext = reinterpret_cast < void * > ( new C ); }
#define ADDEXTS( O, C ) { CHK_PTR_NOP( O ); if ( O->cext != NULL ) DELETE_EXTS( O, C ); O->cext = reinterpret_cast < void * > ( new C ); }
#define ADDEXT_INIT( C, ... ) { if ( _p_->cext != NULL ) DELETE_EXT( C ); _p_->cext = reinterpret_cast < void * > ( new C( __VA_ARGS__ ) ); }
#define ADDEXT_INITS( O, C, ... ) { CHK_PTR_NOP( O ); if ( O->cext != NULL ) DELETE_EXTS( O, C ); O->cext = reinterpret_cast < void * > ( new C( __VA_ARGS__ ) ); }

#define DELETE_EXT( C ) { delete P_EXT( C ); _p_->cext = NULL; }
#define DELETE_EXTS( O, C ) { CHK_PTR_NOP( O ); delete P_EXTS( O, C ); O->cext = NULL; }

#define V_EXT( C, X ) ( P_EXT( C ) -> X )
#define V_EXTS( O, C, X ) ( P_EXTS( O, C ) -> X )
#define DO_EXT( C, X, ... ) ( P_EXT( C ) -> X( __VA_ARGS__ ) )
#define DO_EXTS( O, C, X, ... ) ( P_EXTS( O, C ) -> X( __VA_ARGS__ ) )
#define EXEC_EXT( C, X, Y, ... ) ( P_EXT( C ) -> X.Y( __VA_ARGS__ ) )
#define EXEC_EXTS( O, C, X, Y, ... ) ( P_EXTS( O, C ) -> X.Y( __VA_ARGS__ ) )

#define EXT( C ) ( * P_EXT( C ) )
#define EXTS( O, C ) ( * P_EXTS( O, C ) )
#define P_EXT( C ) ( reinterpret_cast < C * > ( _p_->cext ) )
#define P_EXTS( O, C ) ( reinterpret_cast < C * > ( O->cext ) )

#define WRITE_EXT( C, X, Y ) ( P_EXT( C ) -> X = Y )
#define WRITE_EXTS( O, C, X, Y ) ( P_EXTS( O, C ) -> X = Y )
#define WRITE_ARG_EXT( C, X, Y, ... ) ( P_EXT( C ) -> X( __VA_ARGS__ ) = Y )
#define WRITE_ARG_EXTS( O, C, X, Y, ... ) ( P_EXTS( O, C ) -> X( __VA_ARGS__ ) = Y )

#define CYCLE( X, Y ) for ( X = _cycle_obj_( _p_, Y, "CYCLE" ); X != NULL; X = BROTHER( X ) )
#define CYCLE_SAFE( X, Y ) for ( X = _cycle_obj_( _p_, Y, "CYCLE_SAFE" ), \
								 cyccur = BROTHER( X ); X != NULL; X = cyccur, \
								 cyccur != NULL ? cyccur = BROTHER( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFE( X, Y ) for ( X = _cycle_obj_( _p_, Y, "CYCLE_SAFE" ), \
								  cyccur2 = BROTHER( X ); X != NULL; X = cyccur2, \
								  cyccur2 != NULL ? cyccur2 = BROTHER( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFE( X, Y ) for ( X = _cycle_obj_( _p_, Y, "CYCLE_SAFE" ), \
								  cyccur3 = BROTHER( X ); X != NULL; X = cyccur3, \
								  cyccur3 != NULL ? cyccur3 = BROTHER( cyccur3 ) : cyccur3 = cyccur3 )

#define CYCLES( O, X, Y ) for ( X = _cycle_obj_( O, Y, "CYCLES" ); X != NULL; X = BROTHER( X ) )
#define CYCLE_SAFES( O, X, Y ) for ( X = _cycle_obj_( O, Y, "CYCLE_SAFES" ), \
									 cyccur = BROTHER( X ); X != NULL; X = cyccur, \
									 cyccur != NULL ? cyccur = BROTHER( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFES( O, X, Y ) for ( X = _cycle_obj_( O, Y, "CYCLE_SAFES" ), \
									  cyccur2 = BROTHER( X ); X != NULL; X = cyccur2, \
									  cyccur2 != NULL ? cyccur2 = BROTHER( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFES( O, X, Y ) for ( X = _cycle_obj_( O, Y, "CYCLE_SAFES" ), \
									  cyccur3 = BROTHER( X ); X != NULL; X = cyccur3, \
									  cyccur3 != NULL ? cyccur3 = BROTHER( cyccur3 ) : cyccur3 = cyccur3 )

#define CYCLE_EXT( X, Y, Z ) for ( X = EXEC_EXT( Y, Z, begin ); X != EXEC_EXT( Y, Z, end ); ++X )
#define CYCLE_EXTS( O, X, Y, Z ) for ( X = EXEC_EXTS( O, Y, Z, begin ); X != EXEC_EXTS( O, Y, Z, end ); ++X )

#ifdef NO_POINTER_INIT
	#define CYCLE_LINK( O ) for ( O = _p_->node->first; O != NULL; O = O->next )
	#define CYCLE_LINKS( C, O ) for ( O = C->node->first; O != NULL; O = O->next )
#else
	#define CYCLE_LINK( X ) if ( _p_->node == NULL ) \
								_no_node_dbl_( _p_->attr->label, __FILE__, __LINE__ ); \
							else \
								for ( X = _p_->node->first; X != NULL; X = X->next )
	#define CYCLE_LINKS( O, X ) if ( O == NULL ) \
									_bad_ptr_dbl_( O, __FILE__, __LINE__ ); \
								else \
									if ( O->node == NULL ) \
										_no_node_dbl_( O->attr->label, __FILE__, __LINE__ ); \
									else \
										for ( X = O->node->first; X != NULL; X = X->next )
#endif

// DEPRECATED MACRO COMPATIBILITY DEFINITIONS
// enabled only when directly including fun_head.h (and not lsd_head.h)
#ifdef SEMI_LEGACY_CODE
	namespace lsd
	{
		void equation::_close_sim_( void ) { }
	}

	#define c CALLER
	#define p THIS
	#define t T
	#define root ROOT
	#define seed RND_SEED
	#define plog PLOG
	#define quit _sim_->quit
	#define object lsd::object
#endif

#ifdef LEGACY_CODE
	namespace lsd
	{
		extern simp_vecT sims;				// vector holding existing simulations
		char msg[ MAX_BUFF_SIZE ];			// legacy auxiliary buffer
		const char *get_str( const char *tcl_var );
		void equation::_close_sim_( void ) { }
	}

	inline int deb( lsd::object *r, lsd::object *c, const char *lab, double *res, bool interact = false, const char *hl_var = "" ) { if ( lsd::sims[ 0 ]->liblnk != NULL ) return ( r->*lsd::sims[ 0 ]->liblnk->dlliblinkage::debugger ) ( c, lab, res, interact, hl_var ); else return -1; }
	inline void cmd( const char *cm, ... ) { if ( lsd::sims[ 0 ]->liblnk != NULL ) { va_list argptr; va_start( argptr, cm ); lsd::sims[ 0 ]->liblnk->cmd_backend( cm, argptr ); va_end( argptr ); } }
	inline void plog( const char *cm, ... ) { if ( lsd::sims[ 0 ]->liblnk != NULL ) { va_list argptr; va_start( argptr, cm ); lsd::sims[ 0 ]->liblnk->plog_backend( cm, "", argptr ); va_end( argptr ); } }

	#ifdef Tcl_GetVar
		#undef Tcl_GetVar
	#endif

	#define Tcl_GetVar( X, Y, Z ) lsd::get_str( Y )
	#define inter NULL

	#define SIM ( lsd::sims[ 0 ] )			// pointer to first simulation
	#define var _v_
	#define c CALLER
	#define caller CALLER
	#define p THIS
	#define t T
	#define root ROOT
	#define seed RND_SEED
	#define quit _sim_->quit
	#define object lsd::object
	#define path ( SIM->conf_path )
	#define poidev( ... ) ( SIM->poisson( __VA_ARGS__ ) )
	#define go_brother( O ) BROTHER( O )
	#define FUNCTION( X ) EQUATION( X )
	#define UNIFORM( X, Y ) uniform( X, Y )
	#define rnd_integer( X, Y ) uniform_int( X, Y )
	#define init_lattice( ... ) INIT_LAT( __VA_ARGS__ )
	#define update_lattice( X, Y, Z ) WRITE_LAT( X, Y, Z )
	#define VL_CHEAT( X, Y, C ) V_CHEATL( X, Y, C )
	#define VS_CHEAT( X, Y, C ) V_CHEATS( X, Y, C )
	#define VLS_CHEAT( X, Y, Z, C ) V_CHEATLS( X, Y, Z, C )
	#define ADDOBJL_EX( X, Y, Z ) ADDOBJ_EXL( X, Y, Z )
	#define ADDOBJS_EX( O, X, Y ) ADDOBJ_EXS( O, X, Y )
	#define ADDOBJLS_EX( O, X, Y, Z ) ADDOBJ_EXLS( O, X, Y, Z )
	#define ADDNOBJL_EX( X, Y, Z, W ) ADDNOBJ_EXL( X, Y, Z, W )
	#define ADDNOBJS_EX( O, X, Y, Z ) ADDNOBJ_EXS( O, X, Y, Z )
	#define ADDNOBJLS_EX( O, X, Y, Z, W ) ADDNOBJ_EXLS( O, X, Y, Z, W )
	#define INIT_TSEARCHT( X, Y ) INIT_TSEARCH( X )
	#define INIT_TSEARCHTS( O, X, Y ) INIT_TSEARCHS( O, X )
	#define TSEARCH_INI( X ) INIT_TSEARCH( X )
	#define TSEARCHS_INI( O, X ) INIT_TSEARCHS( O, X )
	#define TSEARCHT_INI( X, Y ) INIT_TSEARCH( X )
	#define TSEARCHTS_INI( O, X, Y ) INIT_TSEARCHS( O, X )
	#define TSEARCHT( X, Y, Z ) TSEARCH( X, Z )
	#define TSEARCHTS( O, X, Y, Z ) TSEARCHS( O, X, Z )
	#define SORTS2( O, X, Y, L, Z ) SORT2S( O, X, Y, L, Z )
	#define RNDDRAWFAIR( X ) RNDDRAW_FAIR( X )
	#define RNDDRAWFAIRS( Z, X ) RNDDRAW_FAIRS( Z, X )
	#define RNDDRAWTOT( X, Y,T ) RNDDRAW_TOT( X, Y,T )
	#define RNDDRAWTOTL( X, Y, Z, T ) RNDDRAW_TOTL( X, Y, Z, T )
	#define RNDDRAWTOTS( Z, X, Y, T ) RNDDRAW_TOTS( Z, X, Y, T )
	#define RNDDRAWTOTLS( O, X, Y, Z, T ) RNDDRAW_TOTLS( O, X, Y, Z, T )
	#define NETWORK_INI( X, Y, Z, ... ) INIT_NET( X, Y, Z, __VA_ARGS__ )
	#define NETWORKS_INI( O, X, Y, Z, ... ) INIT_NETS( O, X, Y, Z, __VA_ARGS__ )
	#define NETWORK_LOAD( X, Y, Z ) ( _p_->read_file_net( X, Y, Z, RND_SEED, "net" ) )
	#define NETWORKS_LOAD( O, X, Y, Z ) ( O == NULL ? 0. : O->read_file_net( X, Y, Z, RND_SEED, "net" ) )
	#define NETWORK_SAVE( X, Y, Z ) ( _p_->write_file_net( X, Y, Z, RND_SEED, false ) )
	#define NETWORKS_SAVE( O, X, Y, Z ) ( O == NULL ? 0. : O->write_file_net( X, Y, Z , RND_SEED, false ) )
	#define STATS_NET( O, X ) STAT_NETS( O, X )
	#define SHUFFLE( X ) SHUFFLE_NET( X )
	#define SHUFFLES( O, X ) SHUFFLE_NETS( O, X )
	#define RNDDRAW_NET( X ) RNDDRAW_NODE( X )
	#define RNDDRAWS_NET( O, X ) RNDDRAW_NODES( O, X )
	#define SEARCH_NET( X, Y ) SEARCH_NODE( X, Y )
	#define SEARCHS_NET( O, X, Y ) SEARCH_NODES( O, X, Y )
	#define VS_NODEID( O ) V_NODEIDS( O )
	#define VS_NODENAME( O ) V_NODENAMES( O )
	#define WRITES_NODEID( O, X ) WRITE_NODEIDS( O, X )
	#define WRITES_NODENAME( O, X ) WRITE_NODENAMES( O, X )
	#define STATS_NODE( O ) STAT_NODES( O )
	#define DELETELINK( O ) DELETE_LINK( O )
	#define SEARCHS_LINK( O, X ) SEARCH_LINKS( O, X )
	#define VS_WEIGHT( O ) V_LINK( O )
	#define WRITES_WEIGHT( O, X ) WRITE_LINK( O, X )
	#define ADD_EXT( CLASS ) ADDEXT( CLASS )
	#define ADDS_EXT( O, CLASS ) ADDEXTS( O, CLASS )
	#define DELETES_EXT( O, CLASS ) DELETE_EXTS( O, CLASS )
	#define PS_EXT( O, CLASS ) P_EXTS( O, CLASS )
	#define VS_EXT( O, CLASS, OBJ ) V_EXTS( O, CLASS, OBJ )
	#define WRITES_EXT( O, CLASS, OBJ, VAL ) WRITE_EXTS( O, CLASS, OBJ, VAL )
	#define EXECS_EXT( O, CLASS, OBJ, METHOD, ... ) EXEC_EXTS( O, CLASS, OBJ, METHOD, __VA_ARGS__ )
	#define DEBUG \
		f = fopen( "log.txt", "a" ); \
		fprintf( f, "t=%g\t%s\t(cur=%g)\n", T, _v_->attr->label, _v_->val[0] ); \
		fclose( f );
	#define DEBUG_AT( X ) \
		if ( T >= X ) \
		{ \
			DEBUG \
		};
	#define CYCLES_EXT( O, ITER, CLASS, OBJ ) CYCLE_EXTS( O, ITER, CLASS, OBJ )
	#define CYCLES_LINK( C, O ) CYCLE_LINKS( C, O )
#endif
