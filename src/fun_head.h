/*************************************************************

	LSD 7.1 - December 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

    This file also: Frederik Schaff, Ruhr-Universität Bochum

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

 *************************************************************/

/*************************************************************
    FUN_HEAD.H
    This file contains all the macros required by the
    model's equation file.
*************************************************************/

#define FUN												// comment this line to access internal LSD functions


#if defined( EIGENLIB ) && __cplusplus >= 201103L		// required C++11
#include <Eigen/Eigen>									// Eigen linear algebra library
using namespace Eigen;
#endif

#include "decl.h"										// LSD classes
#include "check.h"										// LSD macro check support code

// create and set fast lookup flag
#if ! defined FAST_LOOKUP || ! defined CPP11
bool fast_lookup = false;
void init_map( ) { };
#else
bool fast_lookup = true;
#endif

// set pointers to NULL to protect users (small overhead) if not disabled
#if defined FAST_LOOKUP && ! defined NO_POINTER_INIT
bool no_ptr_chk = false;
#define INIT_POINTERS \
	h = i = j = k = 0; \
	cur = cur1 = cur2 = cur3 = cur4 = cur5 = cur6 = cur7 = cur8 = cur9 = cyccur = cyccur2 = cyccur3 = NULL; \
	curl = curl1 = curl2 = curl3 = curl4 = curl5 = curl6 = curl7 = curl8 = curl9 = NULL; \
	f = NULL;
#define CHK_PTR_NOP( O ) if ( chk_ptr( O ) ) bad_ptr_void( O, __FILE__, __LINE__ );
#define CHK_PTR_CHR( O ) chk_ptr( O ) ? bad_ptr_chr( O, __FILE__, __LINE__ ) :
#define CHK_PTR_DBL( O ) chk_ptr( O ) ? bad_ptr_dbl( O, __FILE__, __LINE__ ) :
#define CHK_PTR_LNK( O ) chk_ptr( O ) ? bad_ptr_lnk( O, __FILE__, __LINE__ ) :
#define CHK_PTR_OBJ( O ) chk_ptr( O ) ? bad_ptr_obj( O, __FILE__, __LINE__ ) :
#define CHK_PTR_VOID( O ) chk_ptr( O ) ? bad_ptr_void( O, __FILE__, __LINE__ ) :
#define CHK_OBJ_OBJ( O ) chk_obj( O ) ? bad_ptr_obj( O, __FILE__, __LINE__ ) :
#define CHK_HK_OBJ( O, X ) chk_hook( O, X ) ? no_hook_obj( O, X, __FILE__, __LINE__ ) :
#define CHK_LNK_DBL( O ) O == NULL ? nul_lnk_dbl( __FILE__, __LINE__ ) :
#define CHK_LNK_OBJ( O ) O == NULL ? nul_lnk_obj( __FILE__, __LINE__ ) :
#define CHK_LNK_VOID( O ) O == NULL ? nul_lnk_void( __FILE__, __LINE__ ) :
#define CHK_NODE_CHR( O ) O->node == NULL ? no_node_chr( O->label, __FILE__, __LINE__ ) :
#define CHK_NODE_DBL( O ) O->node == NULL ? no_node_dbl( O->label, __FILE__, __LINE__ ) :
#else
bool no_ptr_chk = true;
#define INIT_POINTERS
#define CHK_PTR_NOP( O )
#define CHK_PTR_CHR( O )
#define CHK_PTR_DBL( O )
#define CHK_PTR_LNK( O )
#define CHK_PTR_OBJ( O )
#define CHK_PTR_VOID( O )
#define CHK_OBJ_OBJ( O )
#define CHK_HK_OBJ( O, X )
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

#define EQ_BEGIN \
	double res = def_res; \
	object *p = var->up, *c = caller, app; \
	int h, i, j, k; \
	double v[ USER_D_VARS ]; \
	object *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cyccur, *cyccur2, *cyccur3; \
	netLink *curl, *curl1, *curl2, *curl3, *curl4, *curl5, *curl6, *curl7, *curl8, *curl9; \
	FILE *f; \
	INIT_POINTERS \
	EQ_USER_VARS

#define EQ_NOT_FOUND \
	char msg[ TCL_BUFF_STR ]; \
	sprintf( msg, "equation not found for variable '%s'", label ); \
	error_hard( msg, "equation not found", "check your configuration (variable name) or\ncode (equation name) to prevent this situation\nPossible problems:\n- There is no equation for this variable\n- The equation name is different from the variable name (case matters!)" ); \
	return res;

#define EQ_TEST_RESULT \
	if ( quit == 0 && ( ( ! use_nan && is_nan( res ) ) || is_inf( res ) ) ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, "equation for '%s' produces the invalid value '%lf' at time %d", label, res, t ); \
		error_hard( msg, "invalid equation result", "check your equation code to prevent invalid math operations\nPossible problems:\n- Illegal math operation (division by zero, log of negative number etc.)\n- Use of too-large/small value in calculation\n- Use of non-initialized temporary variable in calculation", true ); \
		debug_flag = true; \
		debug = 'd'; \
	}

#ifndef NO_WINDOW
#define DEBUG_CODE \
	if ( debug_flag ) \
	{ \
		for ( int n = 0; n < USER_D_VARS; ++n ) \
			d_values[ n ] = v[ n ]; \
		i_values[ 0 ] = i; \
		i_values[ 1 ] = j; \
		i_values[ 2 ] = h; \
		i_values[ 3 ] = k; \
		o_values[ 0 ] = cur; \
		o_values[ 1 ] = cur1; \
		o_values[ 2 ] = cur2; \
		o_values[ 3 ] = cur3; \
		o_values[ 4 ] = cur4; \
		o_values[ 5 ] = cur5; \
		o_values[ 6 ] = cur6; \
		o_values[ 7 ] = cur7; \
		o_values[ 8 ] = cur8; \
		o_values[ 9 ] = cur9; \
		n_values[ 0 ] = curl; \
		n_values[ 1 ] = curl1; \
		n_values[ 2 ] = curl2; \
		n_values[ 3 ] = curl3; \
		n_values[ 4 ] = curl4; \
		n_values[ 5 ] = curl5; \
		n_values[ 6 ] = curl6; \
		n_values[ 7 ] = curl7; \
		n_values[ 8 ] = curl8; \
		n_values[ 9 ] = curl9; \
	};
#else
#define DEBUG_CODE
#endif

// handle fast equation look-up if enabled and C++11 is available
#if ! defined FAST_LOOKUP || ! defined CPP11
// use standard chain method for look-up
#define MODELBEGIN \
	double variable::fun( object *caller ) \
	{ \
		if ( quit == 2 ) \
			return def_res; \
		variable *var = this; \
		EQ_BEGIN

#define MODELEND \
		EQ_NOT_FOUND \
		end: \
		EQ_TEST_RESULT \
		DEBUG_CODE \
		return res; \
	}

#define EQUATION( X ) \
	if ( ! strcmp( label, X ) ) {

#define RESULT( X ) \
		res = X; \
		goto end; \
	}

#define END_EQUATION( X ) \
	{ \
		res = X; \
		goto end; \
	}

#define EQUATION_DUMMY( X, Y ) \
	if ( ! strcmp( label, X ) ) { \
		if ( strlen( Y ) > 0 ) \
		{ \
			var->dummy = true; \
			p->cal( p, ( char * ) Y, 0 ); \
		} \
		res = var->val[ 0 ]; \
		goto end; \
	}

#else
// use fast map method for equation look-up
#define MODELBEGIN \
	double variable::fun( object *caller ) \
	{ \
		double res = def_res; \
		if ( quit == 2 ) \
			return res; \
		if ( eq_func == NULL ) \
		{ \
			auto eq_it = eq_map.find( label ); \
			if ( eq_it != eq_map.end( ) ) \
				eq_func = eq_it->second; \
			else \
			{ \
				EQ_NOT_FOUND \
			} \
		} \
		res = ( eq_func )( caller, this ); \
		EQ_TEST_RESULT \
		return res; \
	} \
	void init_map( ) \
	{ \
		eq_map = \
		{

#define MODELEND \
		}; \
	}

#define EQUATION( X ) \
	{ string( X ), [ ]( object *caller, variable *var ) \
		{ \
			EQ_BEGIN

#define RESULT( X ) \
			; \
			res = X; \
			DEBUG_CODE \
			return res; \
		} \
	},

#define END_EQUATION( X ) \
	{ \
		res = X; \
		DEBUG_CODE \
		return res; \
	}

#define EQUATION_DUMMY( X, Y ) \
	{ string( X ), [ ]( object *caller, variable *var ) \
		{ \
			if ( strlen( Y ) > 0 ) \
			{ \
				var->dummy = true; \
				var->up->cal( var->up, ( char * ) Y, 0 ); \
			} \
			return var->val[ 0 ]; \
		} \
	},

#endif

// redefine as macro to avoid conflicts with C++ version in <cmath.h>
#define abs( X ) _abs( X )
#define pi M_PI

#define ABORT { quit = 1; }
#define DEBUG_START deb_log( true )
#define DEBUG_START_AT( X ) deb_log( true, X )
#define DEBUG_STOP deb_log( false )
#define DEBUG_STOP_AT( X ) deb_log( false, X )
#define DEFAULT_RESULT( X ) { def_res = X; }
#define FAST set_fast( 1 )
#define FAST_FULL set_fast( 2 )
#define OBSERVE set_fast( 0 )
#define NO_NAN { use_nan = false; }
#define USE_NAN { use_nan = true; }
#define NO_POINTER_CHECK build_obj_list( false )
#define USE_POINTER_CHECK build_obj_list( true )
#define NO_SEARCH { no_search = true; }
#define USE_SEARCH { no_search = false; }
#define NO_ZERO_INSTANCE { no_zero_instance = true; }
#define USE_ZERO_INSTANCE { no_zero_instance = false; }
#define PARAMETER { var->param = 1; }
#define RND_GENERATOR( X ) { ran_gen = X; }
#define RND_SETSEED( X ) { seed = ( unsigned ) X; init_random( seed ); }
#define SLEEP( X ) msleep( ( unsigned ) X )

#define CONFIG ( ( const char * ) simul_name )
#define PATH ( ( const char * ) path )
#define CURRENT ( var->val[ 0 ] )
#define PARENT ( p->up )
#define GRANDPARENT ( CHK_PTR_OBJ( p->up ) p->up->up )
// some macros to define unique ids for some objects
#ifdef CPP11
#define MAKE_UNIQUE( LAB ) p->declare_as_unique( ( char * ) LAB )
#define UID  ( double (p->unique_id( ) ) )
#define UIDS( PTR ) (double ( PTR->unique_id( ) ) )
#define SEARCH_UID( ID ) ( p->obj_by_unique_id( int ( ID ) ) )
#endif //#ifdef CPP11
#define RND_SEED ( ( double ) seed - 1 )
#define T ( ( double ) t )
#define LAST_T ( ( double ) max_step )

//modified from https://pmihaylov.com/macros-in-c/
#define PLOG_INFO(M, ...) \
do {fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
char buffer[300]; snprintf(buffer,sizeof(char)*300,"(INFO) (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
plog(buffer);} while(false)

#define LOG( ... ) \
{ \
	if ( ! fast ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, __VA_ARGS__ ); \
		plog( msg ); \
	} \
}
#define PLOG( ... ) \
{ \
	if ( fast_mode < 2 ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, __VA_ARGS__ ); \
		plog( msg ); \
	} \
}

#define V( X ) ( p->cal( p, ( char * ) X, 0 ) )
#define VL( X, Y ) ( p->cal( p, ( char * ) X, Y ) )
#define VS( O, X ) ( CHK_PTR_DBL( O ) O->cal( O, ( char * ) X, 0 ) )
#define VLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->cal( O, ( char * ) X, Y ) )
#define SUM( X ) ( p->sum( ( char * ) X, 0 ) )
#define SUML( X, Y ) ( p->sum( ( char * ) X, Y ) )
#define SUMS( O, X ) ( CHK_PTR_DBL( O ) O->sum( ( char * ) X, 0 ) )
#define SUMLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->sum( ( char * ) X, Y ) )
#define MAX( X ) ( p->overall_max( ( char * ) X, 0 ) )
#define MAXL( X, Y ) ( p->overall_max( ( char * ) X, Y ) )
#define MAXS( O, X ) ( CHK_PTR_DBL( O ) O->overall_max( ( char * ) X, 0 ) )
#define MAXLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->overall_max( ( char * ) X, Y ) )
#define MIN( X ) ( p->overall_min( ( char * ) X, 0 ) )
#define MINL( X, Y ) ( p->overall_min( ( char * ) X, Y ) )
#define MINS( O, X ) ( CHK_PTR_DBL( O ) O->overall_min( ( char * ) X, 0 ) )
#define MINLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->overall_min( ( char * ) X, Y ) )
#define AVE( X ) ( p->av( ( char * ) X, 0 ) )
#define AVEL( X, Y ) ( p->av( ( char * ) X, Y ) )
#define AVES( O, X ) ( CHK_PTR_DBL( O ) O->av( ( char * ) X, 0 ) )
#define AVELS( O, X, Y ) ( CHK_PTR_DBL( O ) O->av( ( char * ) X, Y ) )
#define WHTAVE( X, Y ) ( p->whg_av( ( char * ) Y, ( char * ) X, 0 ) )
#define WHTAVEL( X, Y, Z ) ( p->whg_av( ( char * ) Y, ( char * ) X, Z ) )
#define WHTAVES( O, X, Y ) ( CHK_PTR_DBL( O ) O->whg_av( ( char * ) Y, ( char * ) X, 0 ) )
#define WHTAVELS( O, X, Y, Z ) ( CHK_PTR_DBL( O ) O->whg_av( ( char * ) Y, ( char * ) X, Z ) )
#define SD( X ) ( p->sd( ( char * ) X, 0 ) )
#define SDL( X, Y ) ( p->sd( ( char * ) X, Y ) )
#define SDS( O, X ) ( CHK_PTR_DBL( O ) O->sd( ( char * ) X, 0 ) )
#define SDLS( O, X, Y ) ( CHK_PTR_DBL( O ) O->sd( ( char * ) X, Y ) )
#define COUNT( X ) ( p->count( ( char * ) X ) )
#define COUNTS( O, X ) ( CHK_PTR_DBL( O ) O->count( ( char * ) X ) )
#define COUNT_ALL( X ) ( p->count_all( ( char * ) X ) )
#define COUNT_ALLS( O, X ) ( CHK_PTR_DBL( O ) O->count_all( ( char * ) X ) )
#define STAT( X ) ( p->stat( ( char * ) X, v ) )
#define STATS( O, X ) ( CHK_PTR_DBL( O ) O->stat( ( char * ) X, v ) )

#define STAT_CND( LAB, condVarLab, condition, condVal ) ( p->stat( ( char * ) LAB, v, ( char * ) condVarLab, (char * ) condition, condVal  ) );
#define STAT_CNDS( O, LAB, condVarLab, condition, condVal ) ( O->stat( ( char * ) LAB, v, ( char * ) condVarLab, (char * ) condition, condVal  ) );
#define STAT_CNDL( LAB, condVarLab, condition, condVal, LAG ) ( p->stat( ( char * ) LAB, v, ( char * ) condVarLab, (char * ) condition, condVal, NULL, LAG  ) );
#define STAT_CNDLS( O, LAB, condVarLab, condition, condVal, LAG ) ( O->stat( ( char * ) LAB, v, ( char * ) condVarLab, (char * ) condition, condVal, NULL, LAG  ) );
#define STAT_CND_CHEAT( LAB, condVarLab, condition, condVal, CHEAT_CALLER ) ( p->stat( ( char * ) LAB, v, ( char * ) condVarLab, (char * ) condition, condVal, CHEAT_CALLER ) );
#define STAT_CND_CHEATS( O, LAB, condVarLab, condition, condVal, CHEAT_CALLER ) ( O->stat( ( char * ) LAB, v, ( char * ) condVarLab, (char * ) condition, condVal, , CHEAT_CALLER ) );

#define X_STAT_ALL( LAB ) ( p->xStats_all(( char * ) LAB, v ))
#define X_STAT_ALLL( LAB ) ( p->xStats_all(( char * ) LAB, v,LAG ))
#define X_STAT_ALLS( O,LAB ) ( CHK_PTR_DBL( O ) O->xStats_all(( char * ) LAB, v, LAG ))

#define X_STAT_ALL_CND( LAB,condVarLab, condition, condVal ) ( p->xStats_all_cnd(( char * ) LAB, v , ( char * ) condVarLab, (char * ) condition, condVal));
#define X_STAT_ALL_CNDS( O, LAB,condVarLab, condition, condVal ) ( CHK_PTR_DBL( O ) O->xStats_all_cnd(( char * ) LAB, v , ( char * ) condVarLab, (char * ) condition, condVal));
#define X_STAT_ALL_CNDL( LAB,condVarLab, condition, condVal, LAG) ( p->xStats_all_cnd(( char * ) LAB, v , ( char * ) condVarLab, (char * ) condition, condVal, NULL, LAG));
#define X_STAT_ALL_CNDLS( O, LAB,condVarLab, condition, condVal, LAG ) ( CHK_PTR_DBL( O ) O->xStats_all_cnd(( char * ) LAB, v , ( char * ) condVarLab, (char * ) condition, condVal, NULL, LAG));
#define X_STAT_ALL_CND_CHEAT( LAB,condVarLab, condition, condVal, CHEAT_CALLER ) ( p->xStats_all_cnd(( char * ) LAB, v , ( char * ) condVarLab, (char * ) condition, condVal,CHEAT_CALLER));
#define X_STAT_ALL_CND_CHEATS( O, LAB,condVarLab, condition, condVal, CHEAT_CALLER ) ( CHK_PTR_DBL( O ) O->xStats_all_cnd(( char * ) LAB, v , ( char * ) condVarLab, (char * ) condition, condVal,CHEAT_CALLER));

#define T_STAT(LAB)  ( p->tStats(( char * ) LAB, v ));
#define T_STATS(O,LAB)  ( CHK_PTR_DBL( O ) O->tStats(( char * ) LAB, v ));
#define T_STATL(LAB,LAG)  ( p->tStats(( char * ) LAB, v,LAG ));
#define T_STATLS(O,LAB,LAG)  ( CHK_PTR_DBL( O ) O->tStats(( char * ) LAB, v,LAG ));
#define T_STAT_INTVL(LAB,LAG)  ( p->tStats(( char * ) LAB, v,LAG ));
#define T_STAT_INTVLS(O,LAB,LAG)  ( CHK_PTR_DBL( O ) O->tStats(( char * ) LAB, v,LAG ));

#define INTERACT( X, Y ) ( p->interact( ( char * ) X, Y, v, i, j, h, k, \
	cur, cur1, cur2, cur3, cur4, cur5, cur6, cur7, cur8, cur9, \
	curl, curl1, curl2, curl3, curl4, curl5, curl6, curl7, curl8, curl9 ) )
#define INTERACTS( O, X, Y ) ( CHK_PTR_DBL( O ) O->interact( ( char * ) X, Y, v, i, j, h, k, \
	cur, cur1, cur2, cur3, cur4, cur5, cur6, cur7, cur8, cur9, \
	curl, curl1, curl2, curl3, curl4, curl5, curl6, curl7, curl8, curl9 ) )
#define SEARCH( X ) ( p->search( ( char * ) X ) )
#define SEARCHS( O, X ) ( CHK_PTR_OBJ( O ) O->search( ( char * ) X ) )
#define SEARCH_CND( X, Y ) ( p->search_var_cond( ( char * ) X, Y, 0 ) )
#define SEARCH_CNDL( X, Y, Z ) ( p->search_var_cond( ( char * ) X, Y, Z ) )
#define SEARCH_CNDS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->search_var_cond( ( char * ) X, Y, 0 ) )
#define SEARCH_CNDLS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->search_var_cond( ( char * ) X, Y, Z ) )
#define SEARCH_INST( X ) ( p->search_inst( X ) )
#define SEARCH_INSTS( O, X ) ( CHK_PTR_DBL( O ) O->search_inst( X ) )
#define RNDDRAW( X, Y ) ( p->draw_rnd( ( char * ) X, ( char * ) Y, 0 ) )
#define RNDDRAWL( X, Y, Z ) ( p->draw_rnd( ( char * ) X, ( char * ) Y, Z ) )
#define RNDDRAWS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X, ( char * ) Y, 0 ) )
#define RNDDRAWLS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X, ( char * ) Y, Z ) )
#define RNDDRAW_FAIR( X ) ( p->draw_rnd( ( char * ) X ) )
#define RNDDRAW_FAIRS( O, X ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X ) )
#define RNDDRAW_TOT( X, Y, Z ) ( p->draw_rnd( ( char * ) X, ( char * ) Y, 0, Z ) )
#define RNDDRAW_TOTL( X, Y, Z, W ) ( p->draw_rnd( ( char * ) X, ( char * ) Y, Z, W ) )
#define RNDDRAW_TOTS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X, ( char * ) Y, 0, Z ) )
#define RNDDRAW_TOTLS( O, X, Y, Z, W ) ( CHK_PTR_OBJ( O ) O->draw_rnd( ( char * ) X, ( char * ) Y, Z, W ) )
#define WRITE( X, Y ) ( p->write( ( char * ) X, Y, t ) )
#define WRITEL( X, Y, Z ) ( p->write( ( char * ) X, Y, Z ) )
#define WRITELL( X, Y, Z, W ) ( p->write( ( char * ) X, Y, Z, W ) )
#define WRITES( O, X, Y ) ( CHK_PTR_DBL( O ) O->write( ( char * ) X, Y, t ) )
#define WRITELS( O, X, Y, Z ) ( CHK_PTR_DBL( O ) O->write( ( char * ) X, Y, Z ) )
#define WRITELLS( O, X, Y, Z, W ) ( CHK_PTR_DBL( O ) O->write( ( char * ) X, Y, Z, W ) )
#define INCR( X, Y ) ( p->increment( ( char * ) X, Y ) )
#define INCRS( O, X, Y ) ( CHK_PTR_DBL( O ) O->increment( ( char * ) X, Y ) )
#define MULT( X, Y ) ( p->multiply( ( char * ) X, Y ) )
#define MULTS( O, X, Y ) ( CHK_PTR_DBL( O ) O->multiply( ( char * ) X, Y ) )
#define ADDOBJ( X ) ( p->add_n_objects2( ( char * ) X, 1 ) )
#define ADDOBJL( X, Y ) ( p->add_n_objects2( ( char * ) X, 1, Y ) )
#define ADDOBJS( O, X ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, 1 ) )
#define ADDOBJLS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, 1, Y ) )
#define ADDNOBJ( X, Y ) ( p->add_n_objects2( ( char * ) X, Y ) )
#define ADDNOBJL( X, Y, Z ) ( p->add_n_objects2( ( char * ) X, Y, Z ) )
#define ADDNOBJS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, Y ) )
#define ADDNOBJLS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, Y, Z ) )
#define ADDOBJ_EX( X, Y ) ( p->add_n_objects2( ( char * ) X, 1, Y ) )
#define ADDOBJ_EXL( X, Y, Z ) ( p->add_n_objects2( ( char * ) X, 1, Y, Z ) )
#define ADDOBJ_EXS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, 1, Y ) )
#define ADDOBJ_EXLS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, 1, Y, Z ) )
#define ADDNOBJ_EX( X, Y, Z ) ( p->add_n_objects2( ( char * ) X, Y, Z ) )
#define ADDNOBJ_EXL( X, Y, Z, W ) ( p->add_n_objects2( ( char * ) X, Y, Z, W ) )
#define ADDNOBJ_EXS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, Y, Z ) )
#define ADDNOBJ_EXLS( O, X, Y, Z, W ) ( CHK_PTR_OBJ( O ) O->add_n_objects2( ( char * ) X, Y, Z, W ) )
#define DELETE( O ) ( CHK_PTR_VOID( O ) O->delete_obj( ) )
#define SORT( X, Y, Z ) ( p->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z ) )
#define SORTS( O, X, Y, Z ) ( CHK_PTR_OBJ( O ) O->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z ) )
#define SORT2( X, Y, Z, W ) ( p->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z, ( char * ) W ) )
#define SORT2S( O, X, Y, Z, W ) ( CHK_PTR_OBJ( O ) O->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z, ( char * ) W ) )
// Added RND option for sorting in CPP11
#define SORT_RND( X ) p->lsdqsort( ( char * ) X , NULL , ( char * ) "RANDOM" )
#define SORT_RNDS( O, X ) O->lsdqsort( ( char * ) X , NULL , ( char * ) "RANDOM" )
#define HOOK( X ) ( CHK_HK_OBJ( p, X ) p->hooks[ X ] )
#define HOOKS( O, X ) ( CHK_PTR_OBJ( O ) CHK_HK_OBJ( O, X ) O->hooks[ X ] )
#define SHOOK ( p->hook )
#define SHOOKS( O ) ( CHK_PTR_OBJ( O ) O->hook )
#define WRITE_HOOK( X, Y ) ( CHK_HK_OBJ( p, X ) CHK_OBJ_OBJ( Y ) p->hooks[ X ] = Y )
#define WRITE_HOOKS( O, X, Y ) ( CHK_PTR_OBJ( O ) CHK_HK_OBJ( O, X ) CHK_OBJ_OBJ( Y ) O->hooks[ X ] = Y )
#define WRITE_SHOOK( X ) ( CHK_OBJ_OBJ( X ) p->hook = X )
#define WRITE_SHOOKS( O, X ) ( CHK_PTR_OBJ( O ) CHK_OBJ_OBJ( X ) O->hook = X )
#define ADDHOOK( X ) ( p->hooks.resize( ( unsigned ) X ), NULL )
#define ADDHOOKS( O, X ) ( CHK_PTR_VOID( O ) O->hooks.resize( ( unsigned ) X, NULL ) )
#define COUNT_HOOK ( p->hooks.size( ) )
#define COUNT_HOOKS( O ) ( CHK_PTR_DBL( O ) O->hooks.size( ) )
#define DOWN_LAT ( p->lat_down( ) )
#define DOWN_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_down( ) )
#define LEFT_LAT ( p->lat_left( ) )
#define LEFT_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_left( ) )
#define RIGHT_LAT ( p->lat_right( ) )
#define RIGHT_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_right( ) )
#define UP_LAT ( p->lat_up( ) )
#define UP_LATS( O ) ( CHK_PTR_OBJ( O ) O->lat_up( ) )
#define INIT_LAT( ... ) init_lattice( __VA_ARGS__ )
#define DELETE_LAT close_lattice( )
#define V_LAT( X, Y ) read_lattice( X, Y )
#define WRITE_LAT( X, ... ) update_lattice( X, __VA_ARGS__ )
#define SAVE_LAT( ... ) save_lattice( __VA_ARGS__ )
#define COLOR_LAT( NAME ) named_color( ( NAME ) )
#define V_NODEID ( CHK_NODE_DBL( p ) p->node->id )
#define V_NODEIDS( O ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->id )
#define V_NODENAME ( CHK_NODE_CHR( p ) p->node->name )
#define V_NODENAMES( O ) ( CHK_PTR_CHR( O ) CHK_NODE_CHR( O ) O->node->name )
#define V_LINK( L ) ( CHK_LNK_DBL( L ) L->weight )
#define STAT_NET( X ) ( p->stats_net( ( char * ) X, v ) )
#define STAT_NETS( O, X ) ( CHK_PTR_DBL( O ) O->stats_net( ( char * ) X, v ) )
#define STAT_NODE ( CHK_NODE_DBL( p ) p->node->nLinks )
#define STAT_NODES( O ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->nLinks )
#define SEARCH_NODE( X, Y ) ( p->search_node_net( ( char * ) X, Y ) )
#define SEARCH_NODES( O, X, Y ) ( CHK_PTR_OBJ( O ) O->search_node_net( ( char * ) X, Y ) )
#define SEARCH_LINK( X ) ( p->search_link_net( X ) )
#define SEARCH_LINKS( O, X ) ( CHK_PTR_LNK( O ) O->search_link_net( X ) )
#define RNDDRAW_NODE( X ) ( p->draw_node_net( ( char * ) X ) )
#define RNDDRAW_NODES( O, X ) ( CHK_PTR_OBJ( O ) O->draw_node_net( ( char * ) X ) )
#define RNDDRAW_LINK ( p->draw_link_net( ) )
#define RNDDRAW_LINKS( O ) ( CHK_PTR_LNK( O ) O->draw_link_net( ) )
#define DRAWPROB_NODE( X ) ( CHK_NODE_DBL( p ) p->node->prob = X )
#define DRAWPROB_NODES( O, X ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->prob = X )
#define DRAWPROB_LINK( L, X ) ( CHK_LNK_DBL( L ) L->probTo = X )
#define LINKTO( L ) ( CHK_LNK_OBJ( L ) L->ptrTo )
#define LINKFROM( L ) ( CHK_LNK_OBJ( L ) L->ptrFrom )
#define WRITE_NODEID( X ) ( CHK_NODE_DBL( p ) p->node->id = X )
#define WRITE_NODEIDS( O, X ) ( CHK_PTR_DBL( O ) CHK_NODE_DBL( O ) O->node->id = X )
#define WRITE_NODENAME( X ) ( p->name_node_net( ( char * ) X ) )
#define WRITE_NODENAMES( O, X ) ( CHK_PTR_VOID( O ) O->name_node_net( ( char * ) X ) )
#define WRITE_LINK( L, X ) ( CHK_LNK_DBL( L ) L->weight = X )
#define INIT_NET( X, ... ) ( p->init_stub_net( ( char * ) X, __VA_ARGS__ ) )
#define INIT_NETS( O, X, ... ) ( CHK_PTR_DBL( O ) O->init_stub_net( ( char * ) X, __VA_ARGS__ ) )
#define LOAD_NET( X, Y ) ( p->read_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, "net" ) )
#define LOAD_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->read_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, "net" ) )
#define SAVE_NET( X, Y ) ( p->write_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, false ) )
#define SAVE_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->write_file_net( ( char * ) X, "", ( char * ) Y , seed - 1, false ) )
#define SNAP_NET( X, Y ) ( p->write_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, true ) )
#define SNAP_NETS( O, X, Y ) ( CHK_PTR_DBL( O ) O->write_file_net( ( char * ) X, "", ( char * ) Y, seed - 1, true ) )
#define ADDNODE( X, Y ) ( p->add_node_net( X, Y, false ) )
#define ADDNODES( O, X, Y ) ( CHK_PTR_OBJ( O ) O->add_node_net( X, Y, false ) )
#define ADDLINK( X ) ( p->add_link_net( X, 0 , 1 ) )
#define ADDLINKW( X, Y ) ( p->add_link_net( X, Y, 1 ) )
#define ADDLINKS( O, X ) ( CHK_PTR_LNK( O ) O->add_link_net( X, 0 , 1 ) )
#define ADDLINKWS( O, X, Y ) ( CHK_PTR_LNK( O ) O->add_link_net( X, Y, 1 ) )
#define DELETE_NET( X ) ( p->delete_net( ( char * ) X ) )
#define DELETE_NETS( O, X ) ( CHK_PTR_VOID( O ) O->delete_net( ( char * ) X ) )
#define DELETE_NODE ( p->delete_node_net( ) )
#define DELETE_NODES( O ) ( CHK_PTR_VOID( O ) O->delete_node_net( ) )
#define DELETE_LINK( L ) ( CHK_LNK_VOID( L ) L->ptrFrom->delete_link_net( L ) )
#define SHUFFLE_NET( X ) ( p->shuffle_nodes_net( ( char * ) X ) )
#define SHUFFLE_NETS( O, X ) ( CHK_PTR_OBJ( O ) O->shuffle_nodes_net( ( char * ) X ) )
#define RECALC( X ) ( p->recal( ( char * ) X ) )
#define RECALCS( O, X ) ( CHK_PTR_DBL( O ) O->recal( ( char * ) X ) )
#define INIT_TSEARCH( X ) ( p->initturbo( ( char * ) X, 0 ) )
#define INIT_TSEARCHT( X, Y ) ( p->initturbo( ( char * ) X, Y ) )
#define INIT_TSEARCHS( O, X ) ( CHK_PTR_DBL( O ) O->initturbo( ( char * ) X, 0 ) )
#define INIT_TSEARCHTS( O, X, Y ) ( CHK_PTR_DBL( O ) O->initturbo( ( char * ) X, Y ) )
#define TSEARCH( X, Y ) ( p->turbosearch( ( char * ) X, 0, Y ) )
#define TSEARCHS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->turbosearch( ( char * ) X, 0, Y ) )
#define INIT_TSEARCH_CND( X ) ( p->initturbo_cond( ( char * ) X ) )
#define INIT_TSEARCH_CNDS( O, X ) ( CHK_PTR_DBL( O ) O->initturbo_cond( ( char * ) X ) )
#define TSEARCH_CND( X, Y ) ( p->turbosearch_cond( ( char * ) X, Y ) )
#define TSEARCH_CNDS( O, X, Y ) ( CHK_PTR_OBJ( O ) O->turbosearch_cond( ( char * ) X, Y ) )
#define V_CHEAT( X, Y ) ( p->cal( Y, ( char * ) X, 0 ) )
#define V_CHEATL( X, Y, Z ) ( p->cal( Z, ( char * ) X, Y ) )
#define V_CHEATS( O, X, Y ) ( CHK_PTR_DBL( O ) O->cal( Y, ( char * ) X, 0 ) )
#define V_CHEATLS( O, X, Y, Z ) ( CHK_PTR_DBL( O ) O->cal( Z, ( char * ) X, Y ) )
#define ADDEXT( X ) { if ( p->cext != NULL ) DELETE_EXT( X ); p->cext = reinterpret_cast< void * >( new X ); }
#define ADDEXTS( O, X ) { CHK_PTR_NOP( O ); if ( O->cext != NULL ) DELETE_EXTS( O, X ); O->cext = reinterpret_cast< void * >( new X ); }
#define ADDEXT_INIT( X, ... ) { if ( p->cext != NULL ) DELETE_EXT( X ); p->cext = reinterpret_cast< void * >( new X( __VA_ARGS__ ) ); }
#define ADDEXT_INITS( O, X, ... ) { CHK_PTR_NOP( O ); if ( O->cext != NULL ) DELETE_EXTS( O, X ); O->cext = reinterpret_cast< void * >( new X( __VA_ARGS__ ) ); }
#define DELETE_EXT( X ) { delete P_EXT( X ); p->cext = NULL; }
#define DELETE_EXTS( O, X ) { CHK_PTR_NOP( O ); delete P_EXTS( O, X ); O->cext = NULL; }
#define DO_EXT( X, Y, ... ) ( P_EXT( X ) -> Y( __VA_ARGS__ ) )
#define DO_EXTS( O, X, Y, ... ) ( P_EXTS( O, X ) -> Y( __VA_ARGS__ ) )
#define EXEC_EXT( X, Y, Z, ... ) ( P_EXT( X ) -> Y.Z( __VA_ARGS__ ) )
#define EXEC_EXTS( O, X, Y, Z, ... ) ( P_EXTS( O, X ) -> Y.Z( __VA_ARGS__ ) )
#define EXT( X ) ( * P_EXT( X ) )
#define EXTS( O, X ) ( * P_EXTS( O, X ) )
#define P_EXT( X ) ( reinterpret_cast< X * >( p->cext ) )
#define P_EXTS( O, X ) ( reinterpret_cast< X * >( O->cext ) )
#define V_EXT( X, Y ) ( P_EXT( X ) -> Y )
#define V_EXTS( O, X, Y ) ( P_EXTS( O, X ) -> Y )
#define WRITE_EXT( X, Y, Z ) ( P_EXT( X ) -> Y = Z )
#define WRITE_EXTS( O, X, Y, Z ) ( P_EXTS( O, X ) -> Y = Z )
#define WRITE_ARG_EXT( X, Y, Z, ... ) ( P_EXT( X ) -> Y( __VA_ARGS__ ) = Z )
#define WRITE_ARG_EXTS( O, X, Y, Z, ... ) ( P_EXTS( O, X ) -> Y( __VA_ARGS__ ) = Z )

#define CYCLE( X, Y ) for ( X = cycle_obj( p, ( char * ) Y, "CYCLE" ); X != NULL; X = brother( X ) )
#define CYCLE_SAFE( X, Y ) for ( X = cycle_obj( p, ( char * ) Y, "CYCLE_SAFE" ), \
							  cyccur = brother( X ); X != NULL; X = cyccur, \
							  cyccur != NULL ? cyccur = brother( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFE( X, Y ) for ( X = cycle_obj( p, ( char * ) Y, "CYCLE_SAFE" ), \
							  cyccur2 = brother( X ); X != NULL; X = cyccur2, \
							  cyccur2 != NULL ? cyccur2 = brother( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFE( X, Y ) for ( X = cycle_obj( p, ( char * ) Y, "CYCLE_SAFE" ), \
							  cyccur3 = brother( X ); X != NULL; X = cyccur3, \
							  cyccur3 != NULL ? cyccur3 = brother( cyccur3 ) : cyccur3 = cyccur3 )
#define CYCLES( O, X, Y ) for ( X = cycle_obj( O, ( char * ) Y, "CYCLES" ); X != NULL; X = brother( X ) )
#define CYCLE_SAFES( O, X, Y ) for ( X = cycle_obj( O, ( char * ) Y, "CYCLE_SAFES" ), \
								 cyccur = brother( X ); X != NULL; X = cyccur, \
								 cyccur != NULL ? cyccur = brother( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFES( O, X, Y ) for ( X = cycle_obj( O, ( char * ) Y, "CYCLE_SAFES" ), \
								 cyccur2 = brother( X ); X != NULL; X = cyccur2, \
								 cyccur2 != NULL ? cyccur2 = brother( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFES( O, X, Y ) for ( X = cycle_obj( O, ( char * ) Y, "CYCLE_SAFES" ), \
								 cyccur3 = brother( X ); X != NULL; X = cyccur3, \
								 cyccur3 != NULL ? cyccur3 = brother( cyccur3 ) : cyccur3 = cyccur3 )

#ifdef NO_POINTER_INIT
#define CYCLE_LINK( O ) for ( O = p->node->first; O != NULL; O = O->next )
#define CYCLE_LINKS( C, O ) for ( O = C->node->first; O != NULL; O = O->next )
#else
#define CYCLE_LINK( X ) if ( p->node == NULL ) \
		no_node_dbl( p->label, __FILE__, __LINE__ ); \
	else \
		for ( X = p->node->first; X != NULL; X = X->next )
#define CYCLE_LINKS( O, X ) if ( O == NULL ) \
		bad_ptr_dbl( O, __FILE__, __LINE__ ); \
	else if ( O->node == NULL ) \
		no_node_dbl( O->label, __FILE__, __LINE__ ); \
	else \
		for ( X = O->node->first; X != NULL; X = X->next )
#endif

#define CYCLE_EXT( X, Y, Z ) for ( X = EXEC_EXT( Y, Z, begin ); X != EXEC_EXT( Y, Z, end ); ++X )
#define CYCLE_EXTS( O, X, Y, Z ) for ( X = EXEC_EXTS( O, Y, Z, begin ); X != EXEC_EXTS( O, Y, Z, end ); ++X )

#ifdef CPP11

// GIS MACROS
// The GIS is implemented as a 2d map (continous euclidean space) with a
//   raster-filter (the integer positions). All different kind of LSD objects
//   can be registered in a map. They can share the same map, but it is also
//   possible to have multiple maps.
//
// As with all LSD macros, the macro uses the current object (p) as starting
//   point. Alternative "S" versions of the macros exist as well as all the
//   other standard LSD macro-types, when appropriate (CHEAT, L, S)
//   CHEAT: Please note that passing NULL is equivalent of passing the candidate
//          itself (i.e. using non-cheat version)
//
// A new, GIS-specific post-fix is SHARE which means that the target object
//   (TARGET) will use GIS information of the calling object.
//
// Many macros require the reference to an existing gis-object to defer
//  from this object the map. In this case, GISOBJ relates to such an object.
//  If the user selected one of the "root" options (see below) the user
//  may safely use root each time GISOBJ is demanded.
//
//
//
// There are NN classes of macros:
// a) Initialisation of the map
// b) Adding and Removing objects from the map
//    (note: if an object is deleted, it is removed from the map automatically)
// c) Moving objects inside the map (MOVE, TELEPORT)
// d) General utilities (POSITION, DISTANCE)
// e) Search utilities (search at (grid-)position, CYCLE_NEIGHBOUR, get nearest neighbour with conditions

// Initialisation macros
//
// WRAP Versions allow to define world wrapping
// Non-WRAP Versions assume there is no world wrapping
// there are 2^4 options. We use a bit-code (0=off):
//   0-bit: left     : 0=0 1=1
//   1-bit: right    : 0=0 1=2
//   2-bit: top      : 0=0 1=4
//   3-bit: bottom   : 0=0 1=8
//   sum the values to generate desired wrapping (e.g. 15 - torus world)

// INIT_SPACE_ROOT
// If there is only one GIS or a single GIS is used heavily, it makes sense to
// host in in the root object for easy accessing later on.
#define INIT_SPACE_ROOT(XN,YN)  { root->init_gis_singleObj(0, 0, XN, YN); }
#define INIT_SPACE_ROOT_WRAP(XN, YN, WRAP)  { root->init_gis_singleObj(0, 0, XN, YN, WRAP); }
#define ADD_ROOT_TO_SPACE(GISOBJ) { ( root==GISOBJ ? false : root->register_at_map(GISOBJ->ptr_map(), 0, 0) ); }

// INIT_SPACE_SINGLE
// Initialise the space with a single object
#define INIT_SPACE_SINGLE( X, Y, XN, YN)              { p->init_gis_singleObj(X, Y, XN, YN); }
#define INIT_SPACE_SINGLES( GISOBJ, X, Y, XN, YN)              { GISOBJ->init_gis_singleObj(X, Y, XN, YN); }
#define INIT_SPACE_SINGLE_WRAP( X, Y, XN, YN, WRAP )  { p->init_gis_singleObj( X, Y, XN, YN, WRAP ); }
#define INIT_SPACE_SINGLE_WRAPS( GISOBJ, X, Y, XN, YN, WRAP )  { GISOBJ->init_gis_singleObj( X, Y, XN, YN, WRAP ); }

// INIT_SPACE_GRID
//  bool object::init_gis_regularGrid(char const lab[], int xn, int yn, int _wrap, int _lag, int n){
// Initialise the regular space and use the object LAB contained in p as "Patches"
// Using Column Major (change?) the objects are added to a 2d grid and get xy coords respectively
#define INIT_SPACE_GRID( LAB, XN, YN )             { p->init_gis_regularGrid( LAB, XN, YN, 0, 0 ); }
#define INIT_SPACE_GRIDS(PTR, LAB, XN, YN )             { PTR->init_gis_regularGrid( LAB, XN, YN, 0, 0 ); }

#define INIT_SPACE_GRIDL( LAB, XN, YN, TIME )             { p->init_gis_regularGrid( LAB, XN, YN, 0, TIME ); }
#define INIT_SPACE_GRIDLS(PTR, LAB, XN, YN, TIME )             { PTR->init_gis_regularGrid( LAB, XN, YN, 0, TIME ); }

#define INIT_SPACE_GRID_WRAP( LAB, XN, YN, WRAP )  { p->init_gis_regularGrid( LAB, XN, YN, WRAP, 0 ); }
#define INIT_SPACE_GRID_WRAPS( PTR, LAB, XN, YN, WRAP )  { PTR->init_gis_regularGrid( LAB, XN, YN, WRAP, 0 ); }

#define INIT_SPACE_GRID_WRAPL( LAB, XN, YN, WRAP, TIME )  { p->init_gis_regularGrid( LAB, XN, YN, WRAP, TIME ); }
#define INIT_SPACE_GRID_WRAPLS( PTR, LAB, XN, YN, WRAP, TIME )  { PTR->init_gis_regularGrid( LAB, XN, YN, WRAP, TIME ); }

//n versions with sparce space
#define INIT_SPACE_GRIDN( LAB, XN, YN, N )             { p->init_gis_regularGrid( LAB, XN, YN, 0, 0, N ); }
#define INIT_SPACE_GRIDNS(PTR, LAB, XN, YN, N )             { PTR->init_gis_regularGrid( LAB, XN, YN, 0, 0, N ); }

#define INIT_SPACE_GRIDNL( LAB, XN, YN, N, TIME )             { p->init_gis_regularGrid( LAB, XN, YN, 0, TIME, N ); }
#define INIT_SPACE_GRIDNLS(PTR, LAB, XN, YN, N, TIME )             { PTR->init_gis_regularGrid( LAB, XN, YN, 0, TIME, N ); }

#define INIT_SPACE_GRID_WRAPN( LAB, XN, YN, WRAP, N )  { p->init_gis_regularGrid( LAB, XN, YN, WRAP, 0, N ); }
#define INIT_SPACE_GRID_WRAPNS( PTR, LAB, XN, YN, WRAP, N )  { PTR->init_gis_regularGrid( LAB, XN, YN, WRAP, 0, N ); }

#define INIT_SPACE_GRID_WRAPNL( LAB, XN, YN, WRAP, N , TIME )  { p->init_gis_regularGrid( LAB, XN, YN, WRAP, TIME, N ); }
#define INIT_SPACE_GRID_WRAPNLS( PTR, LAB, XN, YN, WRAP, N, TIME )  { PTR->init_gis_regularGrid( LAB, XN, YN, WRAP, TIME, N ); }

#define SET_GIS_DISTANCE_TYPE( TYPE ) { p->set_distance_type( TYPE ) ; }
#define SET_GIS_DISTANCE_TYPES( PTR, TYPE ) { PTR->set_distance_type( TYPE ) ; }

// DELETE_SPACE / DELETE_FROM_SPACE
// Delete the map and unregister all object-registrations in the map. Do not delte the LSD objects.
#define DELETE_SPACE( OBJ ) { OBJ->delete_map(); }
#define DELETE_FROM_SPACE { p->unregister_from_gis(); }
#define DELETE_FROM_SPACES( PTR ) { PTR->unregister_from_gis(); }

// ADD_TO_SPACE
// Register object in space, providing explicit x,y position or sharing object
// If already registered, move instead and print info.
#define ADD_TO_SPACE_XY( GISOBJ, X, Y)  { ( p==GISOBJ ? false : p->register_at_map(GISOBJ->ptr_map(), X, Y) ); }
#define ADD_TO_SPACE_XYS( PTR, GISOBJ, X, Y)  { ( PTR==GISOBJ ? false : PTR->register_at_map(GISOBJ->ptr_map(), X, Y) ); }
#define ADD_TO_SPACE_SHARE(TARGET) { p->register_at_map(TARGET); }
#define ADD_TO_SPACE_SHARES(PTR, TARGET) { PTR->register_at_map(TARGET); }

#define ADD_TO_SPACE_CENTER_XY( GISOBJ, X, Y, X2, Y2)  { ( p==GISOBJ ? false : p->register_at_map_between(GISOBJ->ptr_map(), X, Y, X2, Y2) ); }
#define ADD_TO_SPACE_CENTER_XYS( PTR, GISOBJ, X, Y, X2, Y2)  { ( PTR==GISOBJ ? false : PTR->register_at_map_between(GISOBJ->ptr_map(), X, Y, X2, Y2) ); }
#define ADD_TO_SPACE_CENTER_SHARE(TARGET, TARGET2) { p->register_at_map_between(TARGET, TARGET2); }
#define ADD_TO_SPACE_CENTER_SHARES(PTR, TARGET, TARGET2) { PTR->register_at_map_between(TARGET, TARGET2); }

#define ADD_TO_SPACE_RND(TARGET) { p->register_at_map_rnd(TARGET); }
#define ADD_TO_SPACE_RNDS(PTR, TARGET) { PTR->register_at_map_rnd(TARGET); }
#define ADD_TO_SPACE_RND_GRID(TARGET) { p->register_at_map_rnd(TARGET,true); }
#define ADD_TO_SPACE_RND_GRIDS(PTR, TARGET) { PTR->register_at_map_rnd(TARGET,true); }

#define ADD_ALL_TO_SPACE( obj ) { p->register_allOfKind_at_grid_rnd( obj); }
#define ADD_ALL_TO_SPACES( GISOBJ, obj ) { GISOBJ->register_allOfKind_at_grid_rnd( obj); }

#define ADD_ALL_TO_SPACE_CND( obj, condVarLab, condition, condVal ) { p->register_allOfKind_at_grid_rnd_cnd( obj, condVarLab, condition, condVal); }
#define ADD_ALL_TO_SPACE_CNDS( GISOBJ, obj, condVarLab, condition, condVal ) { GISOBJ->register_allOfKind_at_grid_rnd_cnd( obj, condVarLab, condition, condVal); }

// POSITION
// Macros to get x or y position or produce random position
#define POSITION_X ( p->get_pos('x') )
#define POSITION_Y ( p->get_pos('y') )
#define POSITION_Z ( p->get_pos('z') )
#define POSITION_XS(PTR) ( PTR->get_pos('x') )
#define POSITION_YS(PTR) ( PTR->get_pos('y') )
#define POSITION_ZS(PTR) ( PTR->get_pos('z') )
#define RANDOM_POSITION_X ( p->random_pos('x') )
#define RANDOM_POSITION_Y ( p->random_pos('y') )
#define RANDOM_POSITION_XS(GISOBJ) ( GISOBJ->random_pos('x') )
#define RANDOM_POSITION_YS(GISOBJ) ( GISOBJ->random_pos('y') )

#define POSITION_INTERCEPT(OBJ1, OBJ2, REL_POS) { p->position_between(v[0], v[1], OBJ1, OBJ2, REL_POS); }
#define POSITION_INTERCEPT_XY(x1, y1, x2, y2, REL_POS) { p->position_between(v[0], v[1], x1, y1, x2, y2, REL_POS); }
#define POSITION_INTERCEPT_XYS(PTR,x1, y1, x2, y2, REL_POS) { PTR->position_between(v[0], v[1], x1, y1, x2, y2, REL_POS); }

// MOVE
// move a single step in one of eight directions
// 0: stay put. 1: north, 2: north-east, 3: east, 4: south-east,
// 5: south, 6: sout-west, 7: west and 8: north-west
// Note: There is no "orientation" currently.
// return value: succes, bool (true == 1/false == 0)
#define MOVE(DIRECTION) ( p->move(DIRECTION) )
#define MOVES(PTR, DIRECTION) ( PTR->move(DIRECTION) )
//to add: Move sequence, use ints.

// TELEPORT
// Move object to target xy or position of target
// return value: succes, bool (true == 1/false == 0)
// the ADJUST version allows to adjust positions if wrapping is allowed.
// the direction is from the starting position in direction of the new one
#define TELEPORT_XY(X,Y) { p->change_position(X,Y,true); }
#define TELEPORT_XYS(PTR,X,Y) { PTR->change_position(X,Y,true); }
#define TELEPORT_ADJUST_XY(X,Y) { p->change_position(X,Y); }
#define TELEPORT_ADJUST_XYS(PTR,X,Y) { PTR->change_position(X,Y); }
#define TELEPORT_SHARE(TARGET) { p->change_position(TARGET); }
#define TELEPORT_SHARES(PTR, TARGET) { PTR->change_position(TARGET); }

//Cycle through all the objects LAB anywhere in random order (RCYCLE) or unsorted fast (FCYCLE)
#define RCYCLE_GIS( O, LAB ) for ( O = p->first_neighbour_full(LAB, true); O != NULL; O = p->next_neighbour() )
#define RCYCLE_GISS( C, O, LAB ) for ( O = C->first_neighbour_full(LAB, true); O != NULL; O = C->next_neighbour() )
#define FCYCLE_GIS( O, LAB ) for ( O = p->first_neighbour_full(LAB, false); O != NULL; O = p->next_neighbour() )
#define FCYCLE_GISS( C, O, LAB ) for ( O = C->first_neighbour_full(LAB, false); O != NULL; O = C->next_neighbour() )

// CYCLE_NEIGHBOUR
// Cycle through all the objects LAB within radius RAD by increasing radius
// _CND: Special version that checks conditions
// For each candidate it is checked if the Variable VAR with lag LAG called by
// either the candidate or CHEAT_C is  COND (<,>,==,!=) CONDVAL
// Note that CHEAT does not work with NULL.

//The RCYCLE options randomise the order.
#define RCYCLE_NEIGHBOUR( O, LAB, RAD ) for ( O = p->first_neighbour(LAB, RAD, 'r'); O != NULL; O = p->next_neighbour() )
#define RCYCLE_NEIGHBOURS( C, O, LAB, RAD ) for ( O = C->first_neighbour(LAB, RAD, 'r'); O != NULL; O = C->next_neighbour() )

#define RCYCLE_NEIGHBOUR_CND(O, LAB, RAD, VAR, COND, CONDVAL ) for ( O = p->first_neighbour(LAB, RAD, 'r', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define RCYCLE_NEIGHBOUR_CNDS(C, O, LAB, RAD, VAR, COND, CONDVAL ) for ( O = C->first_neighbour(LAB, RAD, 'r', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define RCYCLE_NEIGHBOUR_CNDL(O, LAB, RAD, VAR, COND, CONDVAL, LAG ) for ( O = p->first_neighbour(LAB, RAD, 'r', NULL,LAG,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define RCYCLE_NEIGHBOUR_CNDLS(C, O, LAB, RAD, VAR, COND, CONDVAL, LAG ) for ( O = C->first_neighbour(LAB, RAD, 'r', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define RCYCLE_NEIGHBOUR_CND_CHEAT(O, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = p->first_neighbour(LAB, RAD, 'r',CHEAT_C,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define RCYCLE_NEIGHBOUR_CND_CHEATS(C, O, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = C->first_neighbour(LAB, RAD, 'r',CHEAT_C,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )
//The DCYCLE options cycle in increasing distance, randomising agents with same distances.
#define DCYCLE_NEIGHBOUR( O, LAB, RAD ) for ( O = p->first_neighbour(LAB, RAD, 'd'); O != NULL; O = p->next_neighbour() )
#define DCYCLE_NEIGHBOURS( C, O, LAB, RAD ) for ( O = C->first_neighbour(LAB, RAD, 'd'); O != NULL; O = C->next_neighbour() )

#define DCYCLE_NEIGHBOUR_CND(O, LAB, RAD, VAR, COND, CONDVAL ) for ( O = p->first_neighbour(LAB, RAD, 'd', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define DCYCLE_NEIGHBOUR_CNDS(C, O, LAB, RAD, VAR, COND, CONDVAL ) for ( O = C->first_neighbour(LAB, RAD, 'd', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define DCYCLE_NEIGHBOUR_CNDL(O, LAB, RAD, VAR, COND, CONDVAL, LAG ) for ( O = p->first_neighbour(LAB, RAD, 'd', NULL,LAG,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define DCYCLE_NEIGHBOUR_CNDLS(C, O, LAB, RAD, VAR, COND, CONDVAL, LAG ) for ( O = C->first_neighbour(LAB, RAD, 'd', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define DCYCLE_NEIGHBOUR_CND_CHEAT(O, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = p->first_neighbour(LAB, RAD, 'd',CHEAT_C,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define DCYCLE_NEIGHBOUR_CND_CHEATS(C, O, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = C->first_neighbour(LAB, RAD, 'd',CHEAT_C,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )
//The FCYCLE options cycles through the elements without controlling distance or randomisation.
#define FCYCLE_NEIGHBOUR( O, LAB, RAD ) for ( O = p->first_neighbour(LAB, RAD, 'f'); O != NULL; O = p->next_neighbour() )
#define FCYCLE_NEIGHBOURS( C, O, LAB, RAD ) for ( O = C->first_neighbour(LAB, RAD, 'f'); O != NULL; O = C->next_neighbour() )

#define FCYCLE_NEIGHBOUR_CND(O, LAB, RAD, VAR, COND, CONDVAL ) for ( O = p->first_neighbour(LAB, RAD, 'f', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define FCYCLE_NEIGHBOUR_CNDS(C, O, LAB, RAD, VAR, COND, CONDVAL ) for ( O = C->first_neighbour(LAB, RAD, 'f', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define FCYCLE_NEIGHBOUR_CNDL(O, LAB, RAD, VAR, COND, CONDVAL, LAG ) for ( O = p->first_neighbour(LAB, RAD, 'f', NULL,LAG,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define FCYCLE_NEIGHBOUR_CNDLS(C, O, LAB, RAD, VAR, COND, CONDVAL, LAG ) for ( O = C->first_neighbour(LAB, RAD, 'f', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define FCYCLE_NEIGHBOUR_CND_CHEAT(O, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = p->first_neighbour(LAB, RAD, 'f',CHEAT_C,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define FCYCLE_NEIGHBOUR_CND_CHEATS(C, O, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = C->first_neighbour(LAB, RAD, 'f',CHEAT_C,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

// The "N" Options allow to specify the number of elements and the radius searched. They are efficient when it is uncertain how large the search needs to be, but only a small part of the total
// Elements shall be searched.

//The NRCYCLE options randomise the order.
#define NRCYCLE_NEIGHBOUR( O, LAB, N, RAD ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'r'); O != NULL; O = p->next_neighbour() )
#define NRCYCLE_NEIGHBOURS( C, O, LAB, N, RAD ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'r'); O != NULL; O = C->next_neighbour() )

#define NRCYCLE_NEIGHBOUR_CND(O, LAB, N, RAD, VAR, COND, CONDVAL ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'r', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NRCYCLE_NEIGHBOUR_CNDS(C, O, LAB, N, RAD, VAR, COND, CONDVAL ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'r', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define NRCYCLE_NEIGHBOUR_CNDL(O, LAB, N, RAD, VAR, COND, CONDVAL, LAG ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'r', NULL,LAG,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NRCYCLE_NEIGHBOUR_CNDLS(C, O, LAB, N, RAD, VAR, COND, CONDVAL, LAG ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'r', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define NRCYCLE_NEIGHBOUR_CND_CHEAT(O, LAB, N, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'r',CHEAT_C,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NRCYCLE_NEIGHBOUR_CND_CHEATS(C, O, LAB, N, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'r',CHEAT_C,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )
//The NDCYCLE options cycle in increasing distance, randomising agents with same distances.
#define NDCYCLE_NEIGHBOUR( O, LAB, N, RAD ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'd'); O != NULL; O = p->next_neighbour() )
#define NDCYCLE_NEIGHBOURS( C, O, LAB, N, RAD ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'd'); O != NULL; O = C->next_neighbour() )

#define NDCYCLE_NEIGHBOUR_CND(O, LAB, N, RAD, VAR, COND, CONDVAL ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'd', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NDCYCLE_NEIGHBOUR_CNDS(C, O, LAB, N, RAD, VAR, COND, CONDVAL ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'd', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define NDCYCLE_NEIGHBOUR_CNDL(O, LAB, N, RAD, VAR, COND, CONDVAL, LAG ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'd', NULL,LAG,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NDCYCLE_NEIGHBOUR_CNDLS(C, O, LAB, N, RAD, VAR, COND, CONDVAL, LAG ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'd', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define NDCYCLE_NEIGHBOUR_CND_CHEAT(O, LAB, N, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'd',CHEAT_C,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NDCYCLE_NEIGHBOUR_CND_CHEATS(C, O, LAB, N, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'd',CHEAT_C,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )
//The NFCYCLE options cycles through the elements without controlling distance or randomisation.
#define NFCYCLE_NEIGHBOUR( O, LAB, N, RAD ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'f'); O != NULL; O = p->next_neighbour() )
#define NFCYCLE_NEIGHBOURS( C, O, LAB, N, RAD ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'f'); O != NULL; O = C->next_neighbour() )

#define NFCYCLE_NEIGHBOUR_CND(O, LAB, N, RAD, VAR, COND, CONDVAL ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'f', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NFCYCLE_NEIGHBOUR_CNDS(C, O, LAB, N, RAD, VAR, COND, CONDVAL ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'f', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define NFCYCLE_NEIGHBOUR_CNDL(O, LAB, N, RAD, VAR, COND, CONDVAL, LAG ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'f', NULL,LAG,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NFCYCLE_NEIGHBOUR_CNDLS(C, O, LAB, N, RAD, VAR, COND, CONDVAL, LAG ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'f', NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define NFCYCLE_NEIGHBOUR_CND_CHEAT(O, LAB, N, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = p->first_neighbour_n(LAB, N, RAD, 'f',CHEAT_C,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define NFCYCLE_NEIGHBOUR_CND_CHEATS(C, O, LAB, N, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = C->first_neighbour_n(LAB, N, RAD, 'f',CHEAT_C,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )



// NEAREST_IN_DISTANCE
// Provide the closest item in distance RAD with label LAB or NULL if none.
// A radius <0 searches everything
// If several items with the same distance exist, draw randomly with equal probability
// _CND: Special version that checks conditions
// For each candidate it is checked if the Variable VAR with lag LAG called by
// either the candidate or CHEAT_C is  COND (<,>,==,!=) CONDVAL
// Note that CHEAT does not work with NULL.
#define NEAREST_IN_DISTANCE(LAB, RAD) ( p->closest_in_distance(LAB, RAD, true) )
#define NEAREST_IN_DISTANCES(PTR, LAB, RAD) ( PTR->closest_in_distance(LAB, RAD, true) )

#define NEAREST_IN_DISTANCE_CND(LAB, RAD, VAR, COND, CONDVAL ) ( p->closest_in_distance(LAB, RAD, true, NULL, 0, VAR, COND, CONDVAL) )
#define NEAREST_IN_DISTANCE_CNDS(PTR, LAB, RAD, VAR, COND, CONDVAL ) ( PTR->closest_in_distance(LAB, RAD, true, NULL, 0, VAR, COND, CONDVAL) )

#define NEAREST_IN_DISTANCE_CNDL(LAB, RAD, VAR, COND, CONDVAL, LAG ) ( p->closest_in_distance(LAB, RAD, true, NULL, LAG, VAR, COND, CONDVAL) )
#define NEAREST_IN_DISTANCE_CNDLS(PTR, LAB, RAD, VAR, COND, CONDVAL, LAG ) ( PTR->closest_in_distance(LAB, RAD, true, NULL, LAG, VAR, COND, CONDVAL) )

#define NEAREST_IN_DISTANCE_CND_CHEAT(LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) ( p->closest_in_distance(LAB, RAD, true, CHEAT_C, 0, VAR, COND, CONDVAL) )
#define NEAREST_IN_DISTANCE_CND_CHEATS(PTR, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) ( PTR->closest_in_distance(LAB, RAD, true, CHEAT_C, 0, VAR, COND, CONDVAL) )

// DISTANCE
// measures the distance from self to a TARGET or a position
#define DISTANCE(TARGET) ( p -> distance (TARGET) )
#define DISTANCE_XY(X, Y) ( p-> distance (X,Y) )
#define DISTANCE_XYS(PTR, X, Y) ( PTR-> distance (X,Y) )

//DISTANCE2 - Distance between different items / points in space
#define DISTANCE2(TARGET1, TARGET2) ( TARGET1 -> distance (TARGET2) )
#define DISTANCE2_XY(X1, Y1, X2, Y2) ( p-> distance (X1, Y1, X2, Y2) )
#define DISTANCE2_XYS(PTR, X1, Y1, X2, Y2) ( PTR-> distance (X1, Y1, X2, Y2) )

#define RELATIVE_DISTANCE( dist ) ( p->relative_distance(dist) )
#define RELATIVE_DISTANCES( PTR, dist ) ( PTR->relative_distance(dist) )

#define ABSOLUTE_DISTANCE( dist ) ( p->absolute_distance(dist) )
#define ABSOLUTE_DISTANCES( PTR, dist ) ( PTR->absolute_distance(dist) )

#define CENTER_POSITIONX ( p->center_position('x') )
#define CENTER_POSITIONY ( p->center_position('y') )
#define CENTER_POSITIONXS( PTR ) ( PTR->center_position('x') )
#define CENTER_POSITIONYS( PTR ) ( PTR->center_position('y') )

//  SEARCH_POSITION, RSEARCH_POSITION
//  Searches at an exact position for an object with label LAB
//  If it exists it is reported. The RND version works if there can be more
//  than one object at the same place (returning one randomly)
//  The standard version will yield an error if more than one option exist.
//  Change style

#define SEARCH_POSITION_XY(LAB, X, Y)  ( p->search_at_position(LAB, X, Y, true) )
#define SEARCH_POSITION_XYS(PTR, LAB, X, Y)  ( PTR->search_at_position(LAB, X, Y, true) )
#define SEARCH_POSITION(LAB)  ( p->search_at_position(LAB, true) )
#define SEARCH_POSITIONS(PTR, LAB)  ( PTR->search_at_position(LAB, true) )
#define SEARCH_POSITION_NEIGHBOUR(LAB, direction)  ( p->search_at_neighbour_position(LAB, direction, true) )
#define SEARCH_POSITION_NEIGHBOURS(PTR, LAB, direction)  ( PTR->search_at_neighbour_position(LAB, direction, true) )




#define RSEARCH_POSITION_XY(LAB, X, Y)  ( p->search_at_position(LAB, X, Y, false) )
#define RSEARCH_POSITION_XYS(PTR, LAB, X, Y)  ( PTR->search_at_position(LAB, X, Y, false) )
#define RSEARCH_POSITION(LAB)  ( p->search_at_position(LAB, false) )
#define RSEARCH_POSITIONS(PTR, LAB)  ( PTR->search_at_position(LAB, false) )
#define RSEARCH_POSITION_NEIGHBOUR(LAB, direction)  ( p->search_at_neighbour_position(LAB, direction, false) )
#define RSEARCH_POSITION_NEIGHBOURS(PTR, LAB, direction)  ( PTR->search_at_neighbour_position(LAB, direction, false) )

//  SEARCH_POSITION_GRID and SEARCH_POSITION_RND_GRID
//  Similar to above, but it searches at the truncated position.
//  This macro is useful if one set of agents is distributed in continuous space
//  And another one, like "land patches", in discrete space. Then, the modeller
//  Can safely get information on the associated "land patch" of an "agent" with
//  this command.

#define SEARCH_POSITION_GRID_XY(LAB, X, Y)  ( p->search_at_position(LAB, X, Y, true) )
#define SEARCH_POSITION_GRID_XYS(PTR, LAB, X, Y)  ( PTR->search_at_position(LAB, trunc(X), trunc(Y), true) )
#define SEARCH_POSITION_GRID(LAB)  ( p->search_at_position(LAB, true, true) )
#define SEARCH_POSITION_GRIDS(PTR, LAB)  ( PTR->search_at_position(LAB, true, true) )

#define RSEARCH_POSITION_GRID_XY(LAB, X, Y)  ( p->search_at_position(LAB, X, Y, false) )
#define RSEARCH_POSITION_GRID_XYS(PTR, LAB, X, Y)  ( PTR->search_at_position(LAB, trunc(X), trunc(Y), false) )
#define RSEARCH_POSITION_GRID(LAB)  ( p->search_at_position(LAB, false, true) )
#define RSEARCH_POSITION_GRIDS(PTR, LAB)  ( PTR->search_at_position(LAB, false, true) )

//  COUNT_POSITION(S)(_GRID(S))
//  Similar to the search, you can use these macros to count the number of
//  elements at the given position.

#define COUNT_POSITION(LAB)  ( p->elements_at_position( LAB, false ) )
#define COUNT_POSITIONS(PTR, LAB) ( PTR->elements_at_position( LAB, false ) )

#define COUNT_POSITION_XY(LAB, X, Y)  ( p->elements_at_position( LAB, X, Y ) )
#define COUNT_POSITION_XYS(PTR, LAB, X, Y) ( PTR->elements_at_position( LAB, X, Y ) )

#define COUNT_POSITION_GRID(LAB)  ( p->elements_at_position( LAB, true ) )
#define COUNT_POSITION_GRIDS(PTR, LAB) ( PTR->elements_at_position( LAB, true ) )

// Additional Utilities
// ANY_GIS just checks if there is a map associated to the object
// SAME_GIS checks if the two objects share the same map
#define ANY_GIS ( p->position != NULL ? true : false  )
#define ANY_GISS(PTR) ( PTR->position != NULL ? true : false )

#define SAME_GIS(TARGET) ( p->ptr_map() == TARGET->ptr_map() )
#define SAME_GISS(PTR,TARGET) ( PTR->ptr_map() == TARGET->ptr_map() )

#define GIS_INFOS( obj ) ( (obj->gis_info().c_str()) )
#define GIS_INFO ( (p->gis_info().c_str()) )

// GIS Lattice utilities


#ifndef NO_WINDOW

#define INIT_LAT_GIS( ... )         ( p->init_lattice_gis( __VA_ARGS__ ) )
#define INIT_LAT_GISS( PTR, ... )   ( PTR->init_lattice_gis( __VA_ARGS__ ) )
#define DELETE_LAT_GIS              ( close_lattice_gis( ) ) // convenient
#define SAVE_LAT_GIS( ... )         ( save_lattice( __VA_ARGS__ ) ) //convenient

#define V_LAT_GIS                   ( p->read_lattice_gis( ) )
#define V_LAT_GISS( PTR )           ( PTR->read_lattice_gis( ) )
#define WRITE_LAT_GIS( VAL )        ( p->write_lattice_gis( VAL ) )
#define WRITE_LAT_GISS( PTR, VAL )  ( PTR->write_lattice_gis( VAL ) )

#define V_LAT_GIS_XY( X, Y )                    ( p->read_lattice_gis( X, Y, true ) )
#define V_LAT_GIS_XYS( PTR, X, Y )              ( PTR->read_lattice_gis( X, Y, true ) )
#define WRITE_LAT_GIS_XY( X, Y, VAL )           ( p->write_lattice_gis( X, Y, VAL, true ) )
#define WRITE_LAT_GIS_XYS( PTR, X, Y, VAL )     ( PTR->write_lattice_gis( X, Y, VAL, true ) )

#define V_LAT_GIS_ADJUST_XY( X, Y )                 ( p->read_lattice_gis( X, Y, false ) )
#define V_LAT_GIS_ADJUST_XYS( PTR, X, Y )           ( PTR->read_lattice_gis( X, Y, false ) )
#define WRITE_LAT_GIS_ADJUST_XY( X, Y, VAL )        ( p->write_lattice_gis( X, Y, VAL, false ) )
#define WRITE_LAT_GIS_ADJUST_XYS( PTR, X, Y, VAL )  ( PTR->write_lattice_gis( X, Y, VAL, false ) )

#define SET_LAT_PRIORITY( VAL ) ( p->set_lattice_priority( ( VAL ) ) )
#define SET_LAT_PRIORITYS( PTR, VAL ) ( PTR->set_lattice_priority( ( VAL ) ) )
#define SET_LAT_COLOR( VAL ) ( p->set_lattice_color( ( VAL ) ) )
#define SET_LAT_COLORS( PTR, VAL ) ( PTR->set_lattice_color( ( VAL ) ) )

#define RETRIVE_LAT_COLOR           ( p->read_lattice_color( ) )
#define RETRIVE_LAT_COLORS( PTR )   ( PTR->read_lattice_color( ) )

#else

#define INIT_LAT_GIS( ... )         ( void( ) )
#define INIT_LAT_GISS( PTR, ... )   ( void( ) )
#define DELETE_LAT_GIS              ( void( ) ) // convenient
#define SAVE_LAT_GIS( ... )         ( void( ) ) //convenient

#define V_LAT_GIS                   ( void( ) )
#define V_LAT_GISS( PTR )           ( void( ) )
#define WRITE_LAT_GIS( VAL )        ( void( ) )
#define WRITE_LAT_GISS( PTR, VAL )  ( void( ) )

#define V_LAT_GIS_XY( X, Y )                    ( void( ) )
#define V_LAT_GIS_XYS( PTR, X, Y )              ( void( ) )
#define WRITE_LAT_GIS_XY( X, Y, VAL )           ( void( ) )
#define WRITE_LAT_GIS_XYS( PTR, X, Y, VAL )     ( void( ) )

#define V_LAT_GIS_ADJUST_XY( X, Y )                 ( void( ) )
#define V_LAT_GIS_ADJUST_XYS( PTR, X, Y )           ( void( ) )
#define WRITE_LAT_GIS_ADJUST_XY( X, Y, VAL )        ( void( ) )
#define WRITE_LAT_GIS_ADJUST_XYS( PTR, X, Y, VAL )  ( void( ) )

#define SET_LAT_PRIORITY( VAL )         ( void( ) )
#define SET_LAT_PRIORITYS( PTR, VAL )   ( void( ) )
#define SET_LAT_COLOR( VAL )            ( void( ) )
#define SET_LAT_COLORS( PTR, VAL )      ( void( ) )
#endif

//And some new macros to load data from txt files.
//The txt file is in the format x \t y \t value \n
#define LOAD_DATA_GIS(inputfile, obj_lab, var_lab ) p->load_data_gis( (const char *) inputfile, (const char *) obj_lab, (const char *) var_lab, t )
#define LOAD_DATA_GISL(inputfile, obj_lab, var_lab, t_update ) p->load_data_gis( (const char *) inputfile, (const char *) obj_lab, (const char *) var_lab, t_update )
#define LOAD_DATA_GISS(PTR, inputfile, obj_lab, var_lab ) PTR->load_data_gis( (const char *) inputfile, (const char *) obj_lab, (const char *) var_lab, t )
#define LOAD_DATA_GISSL(PTR, inputfile, obj_lab, var_lab, t_update ) PTR->load_data_gis( (const char *) inputfile, (const char *) obj_lab, (const char *) var_lab, t_update )

//The txt file is in gridded format with row and colum headers. In the grid single values are stored. (0,0) is bottom left.
#define LOAD_DATA_GIS_MAT(inputfile, obj_lab, var_lab ) p->load_data_gis_mat( (const char *) inputfile, (const char *) obj_lab, (const char *) var_lab, t )
#define LOAD_DATA_GIS_MATL(inputfile, obj_lab, var_lab, t_update ) p->load_data_gis_mat( (const char *) inputfile, (const char *) obj_lab, (const char *) var_lab, t_update )
#define LOAD_DATA_GIS_MATS(PTR, inputfile, obj_lab, var_lab ) PTR->load_data_gis_mat( (const char *) inputfile, (const char *) obj_lab, (const char *) var_lab, t )
#define LOAD_DATA_GIS_MATSL(PTR, inputfile, obj_lab, var_lab, t_update ) PTR->load_data_gis_mat( (const char *) inputfile, (const char *) obj_lab, (const char *) var_lab, t_update )


// Some simple functions to load data to rapidscv::Document type
// #define LOAD_DATA_CSV(doc_name, inputfile) rapidcsv::Document doc_name(inputfile, rapidcsv::LabelParams( -1, -1), ',' );
// #define LOAD_DATA_S(doc_name, inputfile, separator) rapidcsv::Document doc_name(inputfile, rapidcsv::LabelParams( -1, -1), rapidcsv::SeparatorParams(separator) );
// #define LOAD_DATA_SC(doc_name, inputfile, separator) rapidcsv::Document doc_name(inputfile, rapidcsv::LabelParams( 0, -1), rapidcsv::SeparatorParams(separator) );
// #define LOAD_DATA_SR(doc_name, inputfile, separator) rapidcsv::Document doc_name(inputfile, rapidcsv::LabelParams( -1, 0), rapidcsv::SeparatorParams(separator) );
// #define LOAD_DATA_SCR(doc_name, inputfile, separator) rapidcsv::Document doc_name(inputfile, rapidcsv::LabelParams( 0, 0), rapidcsv::SeparatorParams(separator) );

//Simple function to cycle through x,y from csv loadet before.
// #define CYCLE_DATA_ROWS(doc_name,)
// for (int row = 0; row< f_in.GetRowCount(); ++row) {
//   for (int col = 0; col< f_in.GetColumnCount(); ++col) {
//
//   }
// }


#define ABMAT_ADD_VARIABLE( type, name ) add_abmat_object( type, name );


#endif //#ifdef CPP11

// DEPRECATED MACRO COMPATIBILITY DEFINITIONS
// enabled only when directly including fun_head.h (and not fun_head_fast.h)
#ifndef FAST_LOOKUP

double init_lattice( double pixW = 0, double pixH = 0, double nrow = 100, double ncol = 100,
                     char const lrow[ ] = "y", char const lcol[ ] = "x", char const lvar[ ] = "",
                     object* p = NULL, int init_color = -0xffffff );
double poidev( double xm, long* idum_loc = NULL );
int deb( object* r, object* c, char const* lab, double* res, bool interact = false );
object* go_brother( object* c );
void cmd( const char* cm, ... );
#define FUNCTION( X ) \
	if ( ! strcmp( label, X ) ) { \
		last_update--; \
		if ( c == NULL ) { \
			res = val[ 0 ]; \
			goto end; \
		}

#define UNIFORM( X, Y ) uniform( X, Y )
#define rnd_integer( X, Y ) uniform_int( X, Y )
#define VL_CHEAT( X, Y, C ) V_CHEATL( X, Y, C )
#define VS_CHEAT( X, Y, C ) V_CHEATS( X, Y, C )
#define VLS_CHEAT( X, Y, Z, C ) V_CHEATLS( X, Y, Z, C )
#define ADDOBJL_EX( X, Y, Z ) ADDOBJ_EXL( X, Y, Z )
#define ADDOBJS_EX( O, X, Y ) ADDOBJ_EXS( O, X, Y )
#define ADDOBJLS_EX( O, X, Y, Z ) ADDOBJ_EXLS( O, X, Y, Z )
#define ADDNOBJL_EX( X, Y, Z, W ) ADDNOBJ_EXL( X, Y, Z, W )
#define ADDNOBJS_EX( O, X, Y, Z ) ADDNOBJ_EXS( O, X, Y, Z )
#define ADDNOBJLS_EX( O, X, Y, Z, W ) ADDNOBJ_EXLS( O, X, Y, Z, W )
#define TSEARCH_INI( X ) INIT_TSEARCH( X )
#define TSEARCHS_INI( O, X ) INIT_TSEARCHS( O, X )
#define TSEARCHT_INI( X, Y ) INIT_TSEARCHT( X, Y)	// the number of objects no longer required
#define TSEARCHT( X, Y, Z ) TSEARCH( X, Z )			// when calling turbo search, as it is already
#define TSEARCHTS( O, X, Y, Z ) TSEARCHS( O, X, Z )	// stored in the bridge in faster log form
#define SORTS2( O, X, Y, L, Z ) SORT2S( O, X, Y, L, Z )
#define RNDDRAWFAIR( X ) RNDDRAW_FAIR( X )
#define RNDDRAWFAIRS(Z, X ) RNDDRAW_FAIRS(Z, X )
#define RNDDRAWTOT( X, Y,T ) RNDDRAW_TOT( X, Y,T )
#define RNDDRAWTOTL( X, Y, Z, T ) RNDDRAW_TOTL( X, Y, Z, T )
#define RNDDRAWTOTS(Z, X, Y,T ) RNDDRAW_TOTS(Z, X, Y,T )
#define RNDDRAWTOTLS( O, X, Y, Z, T ) RNDDRAW_TOTLS( O, X, Y, Z, T )
#define NETWORK_INI( X, Y, Z, ... ) INIT_NET( X, Y, Z, __VA_ARGS__ )
#define NETWORKS_INI( O, X, Y, Z, ... ) INIT_NETS( O, X, Y, Z, __VA_ARGS__ )
#define NETWORK_LOAD( X, Y, Z ) ( p->read_file_net( ( char * ) X, ( char * ) Y, ( char * ) Z, seed-1, "net" ) )
#define NETWORKS_LOAD( O, X, Y, Z ) ( O == NULL ? 0. : O->read_file_net( ( char * ) X, ( char * ) Y, ( char * ) Z, seed-1, "net" ) )
#define NETWORK_SAVE( X, Y, Z ) ( p->write_file_net( ( char * ) X, ( char * ) Y, ( char * ) Z, seed-1, false ) )
#define NETWORKS_SAVE( O, X, Y, Z ) ( O == NULL ? 0. : O->write_file_net( ( char * ) X, ( char * ) Y, ( char * ) Z , seed-1, false ) )
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
#define CYCLES_LINK( C, O ) CYCLE_LINKS( C, O )
#define ADD_EXT( CLASS ) ADDEXT( CLASS )
#define ADDS_EXT( O, CLASS ) ADDEXTS( O, CLASS )
#define DELETES_EXT( O, CLASS ) DELETE_EXTS( O, CLASS )
#define PS_EXT( O, CLASS ) P_EXTS( O, CLASS )
#define VS_EXT( O, CLASS, OBJ ) V_EXTS( O, CLASS, OBJ )
#define WRITES_EXT( O, CLASS, OBJ, VAL ) WRITE_EXTS( O, CLASS, OBJ, VAL )
#define EXECS_EXT( O, CLASS, OBJ, METHOD, ... ) EXEC_EXTS( O, CLASS, OBJ, METHOD, __VA_ARGS__ )
#define CYCLES_EXT( O, ITER, CLASS, OBJ ) CYCLE_EXTS( O, ITER, CLASS, OBJ )

#define DEBUG \
	f = fopen( "log.txt", "a" ); \
	fprintf( f, "t=%d\t%s\t(cur=%g)\n", t, var->label, var->val[0] ); \
	fclose( f );

#define DEBUG_AT( X ) \
	if ( t >= X ) \
	{ \
		DEBUG \
	};

#endif

#ifndef NO_WINDOW //these macros only exist in windowed mode.
//A helper macro that can be used together with others to pass a file-name
#define SELECT_FILE( _message ) p->grab_filename_interactive( _message )
#endif
