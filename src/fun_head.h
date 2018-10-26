/*************************************************************

	LSD 7.1 - May 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/***************************************************
FUN_HEAD.CPP

This file contains all the declarations and macros 
available in a model's equation file.
****************************************************/

#define FUN												// comment this line to access internal LSD functions

#if defined( EIGENLIB ) && __cplusplus >= 201103L		// required C++11
#include <Eigen/Eigen>									// Eigen linear algebra library
using namespace Eigen;
#endif

#include "decl.h"										// LSD classes
#ifndef NO_WINDOW
#define DEBUG_CODE \
	if ( debug_flag ) \
	{ \
		for ( i = 0; i < 1000; i++ ) \
			i_values[ i ] = v[ i ]; \
	};
#else
#define DEBUG_CODE
#endif

// user defined variables for all equations (to be defined in equation file)
#if ! defined EQ_USER_VARS
#define EQ_USER_VARS
#endif

#define EQ_BEGIN \
	double res = def_res; \
	object *p = var->up, *c = caller, app; \
	int i, j, h, k; \
	double v[1000]; \
	object *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cur10, *cyccur, *cyccur2, *cyccur3; \
	cur = cur1 = cur2 = cur3 = cur4 = cur5 = cur6 = cur7 = cur8 = cur9 = cur10 = cyccur = cyccur2 = cyccur3 = NULL; \
	netLink *curl, *curl1, *curl2, *curl3, *curl4, *curl5; \
	curl = curl1 = curl2 = curl3 = curl4 = curl5 = NULL; \
	FILE *f = NULL; \
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

// create and set fast lookup flag
#if ! defined FAST_LOOKUP || ! defined CPP11
	bool fast_lookup = false;
#else
	bool fast_lookup = true;
#endif

// handle fast equation look-up if enabled and C++11 is available
#if ! defined FAST_LOOKUP || ! defined CPP11
	void init_map( ) { };
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

#endif

// redefine as macro to avoid conflicts with C++ version in <cmath.h>
#define abs( a ) _abs( a )
#define pi M_PI

#define ABORT quit = 1;
#define CURRENT ( var->val[ 0 ] )
#define PARAMETER var->param = 1;

// some macros to define unique ids for some objects
#ifdef CPP11
  #define MAKE_UNIQUE( LAB ) p->declare_as_unique( ( char * ) LAB )
  #define UNIQUE_ID  p->unique_id( )
  #define UNIQUE_IDS( PTR ) PTR->unique_id( )
  #define SEARCH_UID( ID ) obj_by_unique_id( ID )
#endif //#ifdef CPP11

#define OBSERVE set_fast( 0 );
#define FAST set_fast( 1 );
#define FAST_FULL set_fast( 2 );
#define USE_NAN use_nan = true;
#define NO_NAN use_nan = false;
#define USE_SEARCH no_search = false;
#define NO_SEARCH no_search = true;
#define DEFAULT_RESULT( X ) def_res = X;
#define RND_GENERATOR( X ) ran_gen = ( int ) X;
#define RND_SETSEED( X ) seed = ( int ) X; init_random( seed );
#define SLEEP( X ) msleep( ( unsigned ) X );
#define DEBUG_START deb_log( true );
#define DEBUG_START_AT( X ) deb_log( true, ( int ) X );
#define DEBUG_STOP deb_log( false );
#define DEBUG_STOP_AT( X ) deb_log( false, ( int ) X );

#define RND_SEED ( ( double ) seed - 1 )
#define PATH ( ( const char * ) path )
#define CONFIG ( ( const char * ) simul_name )
#define T ( ( double ) t )
#define LAST_T ( ( double ) max_step )

#define INTERACT( X, Y )  p->interact( ( char * ) X, Y, v)
#define INTERACTS( Z, X, Y ) Z->interact( ( char * ) X, Y, v)

// regular logging (disabled in any fast mode)
#define LOG( ... ) \
	if ( ! fast ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, __VA_ARGS__ ); \
		plog( msg ); \
	}
// priority logging (show also in fast mode 1)
#define PLOG( ... ) \
	if ( fast_mode < 2 ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, __VA_ARGS__ ); \
		plog( msg ); \
	}

#define V( X ) p->cal( p, ( char * ) X, 0 )
#define VL( X, Y ) p->cal( p, ( char * ) X, Y )
#define VS( X, Y ) X->cal( X, ( char * ) Y, 0 )
#define VLS( X, Y, Z ) X->cal( X, ( char * ) Y, Z )

#define V_CHEAT( X, C ) p->cal( C, ( char * ) X, 0 )
#define V_CHEATL( X, Y, C ) p->cal( C, ( char * ) X, Y )
#define V_CHEATS( X, Y, C ) X->cal( C, ( char * ) Y, 0 )
#define V_CHEATLS( X, Y, Z, C ) X->cal( C, ( char * ) Y, Z )

#define SUM( X ) p->sum( ( char * ) X, 0 )
#define SUML( X, Y ) p->sum( ( char * ) X, Y )
#define SUMS( X, Y ) X->sum( ( char * ) Y, 0 )
#define SUMLS( X, Y, Z ) X->sum( ( char * ) Y, Z )

#define COUNT( X ) p->count( ( char * ) X )
#define COUNTS( O, X ) O->count( ( char * ) X )

#define COUNT_ALL( X ) p->count_all( ( char * ) X )
#define COUNT_ALLS( O, X ) O->count_all( ( char * ) X )

#define STAT( X ) p->stat( ( char * ) X, v )
#define STATS( O, X ) O->stat( ( char * ) X, v )

#define MAX( X ) p->overall_max( ( char * ) X, 0 )
#define MAXL( X, Y ) p->overall_max( ( char * ) X, Y )
#define MAXS( X, Y ) X->overall_max( ( char * ) X, 0 )
#define MAXLS( O, X, Y ) O->overall_max( ( char * ) X, Y )

#define MIN( X ) p->overall_min( ( char * ) X, 0 )
#define MINL( X, Y ) p->overall_min( ( char * ) X, Y )
#define MINS( X, Y ) X->overall_min( ( char * ) X, 0 )
#define MINLS( O, X, Y ) O->overall_min( ( char * ) X, Y )

#define AVE( X ) p->av( ( char * ) X, 0 )
#define AVEL( X, Y ) p->av( ( char * ) X, Y )
#define AVES( X, Y ) X->av( ( char * ) Y, 0 )
#define AVELS( X, Y, Z ) X->av( ( char * ) Y, Z )

#define WHTAVE( X, W ) p->whg_av( ( char * ) W, ( char * ) X, 0 )
#define WHTAVEL( X, W, Y ) p->whg_av( ( char * ) W, ( char * ) X, Y )
#define WHTAVES( X, W, Y ) X->whg_av( ( char * ) W, ( char * ) Y, 0 )
#define WHTAVELS( X, W, Y, Z ) X->whg_av( ( char * ) W, ( char * ) Y, Z )

#define SD( X ) p->sd( ( char * ) X, 0 )
#define SDL( X, Y ) p->sd( ( char * ) X, Y )
#define SDS( X, Y ) X->sd( ( char * ) Y, 0 )
#define SDLS( X, Y, Z ) X->sd( ( char * ) Y, Z )

#define INCRS(Q, X, Y ) Q->increment( ( char * ) X, Y )
#define INCR( X, Y ) p->increment( ( char * ) X, Y )

#define MULT( X, Y ) p->multiply( ( char * ) X, Y )
#define MULTS(Q, X, Y ) Q->multiply( ( char * ) X, Y )

#define CYCLE( O, L ) for ( O = get_cycle_obj( p, ( char * ) L, "CYCLE" ); O != NULL; O = go_brother( O ) )
#define CYCLES( C, O, L ) for ( O = get_cycle_obj( C, ( char * ) L, "CYCLES" ); O != NULL; O = go_brother( O ) )

#define CYCLE_SAFE( O, L ) for ( O = get_cycle_obj( p, ( char * ) L, "CYCLE_SAFE" ), \
							  cyccur = go_brother( O ); O != NULL; O = cyccur, \
							  cyccur != NULL ? cyccur = go_brother( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFE( O, L ) for ( O = get_cycle_obj( p, ( char * ) L, "CYCLE_SAFE" ), \
							  cyccur2 = go_brother( O ); O != NULL; O = cyccur2, \
							  cyccur2 != NULL ? cyccur2 = go_brother( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFE( O, L ) for ( O = get_cycle_obj( p, ( char * ) L, "CYCLE_SAFE" ), \
							  cyccur3 = go_brother( O ); O != NULL; O = cyccur3, \
							  cyccur3 != NULL ? cyccur3 = go_brother( cyccur3 ) : cyccur3 = cyccur3 )
#define CYCLE_SAFES( C, O, L ) for ( O = get_cycle_obj( C, ( char * ) L, "CYCLE_SAFES" ), \
								 cyccur = go_brother( O ); O != NULL; O = cyccur, \
								 cyccur != NULL ? cyccur = go_brother( cyccur ) : cyccur = cyccur )
#define CYCLE2_SAFES( C, O, L ) for ( O = get_cycle_obj( C, ( char * ) L, "CYCLE_SAFES" ), \
								 cyccur2 = go_brother( O ); O != NULL; O = cyccur2, \
								 cyccur2 != NULL ? cyccur2 = go_brother( cyccur2 ) : cyccur2 = cyccur2 )
#define CYCLE3_SAFES( C, O, L ) for ( O = get_cycle_obj( C, ( char * ) L, "CYCLE_SAFES" ), \
								 cyccur3 = go_brother( O ); O != NULL; O = cyccur3, \
								 cyccur3 != NULL ? cyccur3 = go_brother( cyccur3 ) : cyccur3 = cyccur3 )

#define WRITE( X, Y ) p->write( ( char * ) X, Y, t )
#define WRITEL( X, Y, Z ) p->write( ( char * ) X, Y, Z )
#define WRITELL( X, Y, Z, L) p->write( ( char * ) X, Y, Z, L)
#define WRITES( O, X, Y ) O->write( ( char * ) X, Y, t )
#define WRITELS( O, X, Y, Z ) O->write( ( char * ) X, Y, Z )
#define WRITELLS( O, X, Y, Z, L) O->write( ( char * ) X, Y, Z, L)

#define RECALC( X ) p->recal( ( char * ) X )
#define RECALCS( O, X ) O->recal( ( char * ) X )

#define SEARCH_CND( X, Y ) p->search_var_cond( ( char * ) X, Y, 0 )
#define SEARCH_CNDL( X, Y, Z ) p->search_var_cond( ( char * ) X, Y, Z )
#define SEARCH_CNDS( O, X, Y ) O->search_var_cond( ( char * ) X, Y, 0 )
#define SEARCH_CNDLS( O, X, Y, Z ) O->search_var_cond( ( char * ) X, Y, Z )

#define SEARCH( X ) p->search( ( char * ) X )
#define SEARCHS( Y, X ) Y->search( ( char * ) X )

// Seeds turbo search: O=pointer to container object where searched objects are
//                     X=name of object contained inside the searched objects
//					   Y=total number of objects
#define INIT_TSEARCH( X ) p->initturbo( ( char * ) X, 0 )
#define INIT_TSEARCHS( O, X ) O->initturbo( ( char * ) X, 0 )
#define INIT_TSEARCHT( X, Y ) p->initturbo( ( char * ) X, Y )
#define INIT_TSEARCHTS( O, X, Y ) O->initturbo( ( char * ) X, Y )

// Performs turbo search: O, X as in TSEARCHS_INI
//                        Z=value of variable X to be searched for
#define TSEARCH( X, Z ) p->turbosearch( ( char * ) X, 0 , Z )
#define TSEARCHS( O, X, Z ) O->turbosearch( ( char * ) X, 0 , Z )

#define SORT( X, Y, Z ) p->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z )
#define SORTS( O, X, Y, Z ) O->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) Z )
#define SORT2( X, Y, L, Z ) p->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) L, ( char * ) Z )
#define SORT2S( O, X, Y, L, Z ) O->lsdqsort( ( char * ) X, ( char * ) Y, ( char * ) L, ( char * ) Z )

// Added RND option for sorting in CPP11
#ifdef CPP11
  #define SORT_RND( X ) p->lsdrndsort( ( char * ) X )
  #define SORT_RNDS( O, X ) O->lsdrndsort( ( char * ) X )
#endif //#ifdef CPP11

#define ADDOBJ( X ) p->add_n_objects2( ( char * ) X, 1 )
#define ADDOBJL( X, Y ) p->add_n_objects2( ( char * ) X, 1 , ( int ) Y )
#define ADDOBJS( O, X ) O->add_n_objects2( ( char * ) X, 1 )
#define ADDOBJLS( O, X, Y ) O->add_n_objects2( ( char * ) X, 1 , ( int ) Y )

#define ADDOBJ_EX( X, Y ) p->add_n_objects2( ( char * ) X, 1 , Y )
#define ADDOBJ_EXL( X, Y, Z ) p->add_n_objects2( ( char * ) X, 1 , Y, ( int ) Z )
#define ADDOBJ_EXS( O, X, Y ) O->add_n_objects2( ( char * ) X, 1 , Y )
#define ADDOBJ_EXLS( O, X, Y, Z ) O->add_n_objects2( ( char * ) X, 1 , Y, ( int ) Z )

#define ADDNOBJ( X, Y ) p->add_n_objects2( ( char * ) X, ( int ) Y )
#define ADDNOBJL( X, Y, Z ) p->add_n_objects2( ( char * ) X, ( int ) Y, ( int ) Z )
#define ADDNOBJS( O, X, Y ) O->add_n_objects2( ( char * ) X, ( int ) Y )
#define ADDNOBJLS( O, X, Y, Z ) O->add_n_objects2( ( char * ) X, ( int ) Y, ( int ) Z )

#define ADDNOBJ_EX( X, Y, Z ) p->add_n_objects2( ( char * ) X, ( int ) Y, Z )
#define ADDNOBJ_EXL( X, Y, Z, W ) p->add_n_objects2( ( char * ) X, ( int ) Y, Z, ( int ) W )
#define ADDNOBJ_EXS( O, X, Y, Z ) O->add_n_objects2( ( char * ) X, ( int ) Y, Z )
#define ADDNOBJ_EXLS( O, X, Y, Z, W ) O->add_n_objects2( ( char * ) X, ( int ) Y, Z, ( int ) W )

#define DELETE( X ) ( X == NULL ? nop( ) : X->delete_obj( ) )

#define RNDDRAW( X, Y ) p->draw_rnd( ( char * ) X, ( char * ) Y, 0 )
#define RNDDRAWL( X, Y, Z ) p->draw_rnd( ( char * ) X, ( char * ) Y, Z )
#define RNDDRAWS( Z, X, Y ) Z->draw_rnd( ( char * ) X, ( char * ) Y, 0 )
#define RNDDRAWLS( O, X, Y, Z ) O->draw_rnd( ( char * ) X, ( char * ) Y, Z )

#define RNDDRAW_FAIR( X ) p->draw_rnd( ( char * ) X )
#define RNDDRAW_FAIRS( Z, X ) Z->draw_rnd( ( char * ) X )

#define RNDDRAW_TOT( X, Y, T ) p->draw_rnd( ( char * ) X, ( char * ) Y, 0 ,T )
#define RNDDRAW_TOTL( X, Y, Z, T ) p->draw_rnd( ( char * ) X, ( char * ) Y, Z, T )
#define RNDDRAW_TOTS( Z, X, Y,T ) Z->draw_rnd( ( char * ) X, ( char * ) Y, 0 ,T )
#define RNDDRAW_TOTLS( O, X, Y, Z, T ) O->draw_rnd( ( char * ) X, ( char * ) Y, Z, T )

// LATTICE MACROS
#define INIT_LAT( ... ) init_lattice( __VA_ARGS__ )
#define DELETE_LAT close_lattice( )
#define SAVE_LAT( ... ) save_lattice( __VA_ARGS__ )

#define V_LAT( Y, X ) read_lattice( Y, X )
#define WRITE_LAT( Y, X, ... ) update_lattice( Y, X, __VA_ARGS__ )

#define UP_LAT lat_up( )
#define UP_LATS( O ) O->lat_up( )
#define DOWN_LAT lat_down( )
#define DOWN_LATS( O ) O->lat_down( )
#define RIGHT_LAT lat_right( )
#define RIGHT_LATS( O ) O->lat_right( )
#define LEFT_LAT lat_left( )
#define LEFT_LATS( O ) O->lat_left( )

// NETWORK MACROS
// create a network using as nodes object label X, located inside object O,
// applying generator Y, number of nodes Z, out-degree W and 
// parameter V
#define INIT_NET( X, Y, ... ) ( p->init_stub_net( ( char * ) X, ( char * ) Y, __VA_ARGS__ ) )
#define INIT_NETS( O, X, Y, ... ) ( O == NULL ? 0. : O->init_stub_net( ( char * ) X, ( char * ) Y, __VA_ARGS__ ) )

// read a network in Pajek format from file named Z_xx.net (xx is the current seed) 
// using as nodes object with label X located inside object O
#define LOAD_NET( X, Z ) ( p->read_file_net( ( char * ) X, "", ( char * ) Z, seed-1, "net" ) )
#define LOAD_NETS( O, X, Z ) ( O == NULL ? 0. : O->read_file_net( ( char * ) X, "", ( char * ) Z, seed-1, "net" ) )

// save a network in Pajek format to file from the network formed by nodes
// with label X located inside object O with filename Z (file name is Z_xx.net)
#define SAVE_NET( X, Z ) ( p->write_file_net( ( char * ) X, "", ( char * ) Z, seed-1, false ) )
#define SAVE_NETS( O, X, Z ) ( O == NULL ? 0. : O->write_file_net( ( char * ) X, "", ( char * ) Z , seed-1, false ) )

// add a network snapshot in Pajek format to file from the network formed by nodes
// with label X located inside object O with filename Z (file name is Z_xx.paj)
#define SNAP_NET( X, Z ) ( p->write_file_net( ( char * ) X, "", ( char * ) Z, seed-1, true ) )
#define SNAP_NETS( O, X, Z ) ( O == NULL ? 0. : O->write_file_net( ( char * ) X, "", ( char * ) Z, seed-1, true ) )

// delete a network using as nodes object label X, located inside object O
#define DELETE_NET( X ) ( p->delete_net( ( char * ) X )
#define DELETE_NETS( O, X ) if ( O != NULL ) O->delete_net( ( char * ) X );

// get statistics of network based on object X, contained in O
#define STAT_NET( X ) p->stats_net( ( char * ) X, v );
#define STAT_NETS( O, X ) if ( O != NULL )O->stats_net( ( char * ) X, v );

// add a network node to object O, defininig id=X and name=Y
#define ADDNODE( X, Y ) ( p->add_node_net( X, Y, false ) )
#define ADDNODES( O, X, Y ) ( O == NULL ? NULL : O->add_node_net( X, Y, false ) )

// shuffle the nodes of a network composed by objects with label X, contained in O
#define SHUFFLE_NET( X ) p->shuffle_nodes_net( ( char * ) X );
#define SHUFFLE_NETS( O, X ) if ( O != NULL ) O->shuffle_nodes_net( ( char * ) X );

// random draw one node from a network composed by objects with label X, contained in O
#define RNDDRAW_NODE( X ) ( p->draw_node_net( ( char * ) X ) )
#define RNDDRAW_NODES( O, X ) ( O == NULL ? NULL : O->draw_node_net( ( char * ) X ) )

// set random draw probability to X for one node in a network
#define DRAWPROB_NODE( X ) { if ( p->node != NULL ) p->node->prob = X; }
#define DRAWPROB_NODES( O, X ) { if ( O != NULL && O->node != NULL ) O->node->prob = X; }

// search node objects X, contained in O, for first occurrence of id=Y
#define SEARCH_NODE( X, Y ) ( p->search_node_net( ( char * ) X, ( long ) Y ) )
#define SEARCH_NODES( O, X, Y ) ( O == NULL? NULL : O->search_node_net( ( char * ) X, ( long ) Y ) )

// delete the node pointed by O
#define DELETE_NODE p->delete_node_net( );
#define DELETE_NODES( O ) if ( O != NULL ) O->delete_node_net( );

// get the id of the node object O
#define V_NODEID ( p->node==NULL?0.:(double)p->node->id)
#define V_NODEIDS( O ) ( O == NULL ? 0. : O->node==NULL?0.:(double)O->node->id)

// get the name of the node object O
#define V_NODENAME ( p->node == NULL ? "" : p->node->name == NULL ? "" : p->node->name )
#define V_NODENAMES( O ) ( O == NULL ? "" : O->node == NULL ? "" : O->node->name==NULL ? "" : O->node->name )

// set the id of the node object O
#define WRITE_NODEID( X ) if ( p->node != NULL )p->node->id=(double)X;
#define WRITE_NODEIDS( O, X ) if ( O != NULL ) if ( O->node != NULL ) O->node->id = ( double ) X;

// set the name of the node object O to X
#define WRITE_NODENAME( X ) p->name_node_net( ( char * ) X );
#define WRITE_NODENAMES( O, X ) if ( O != NULL )O->name_node_net( ( char * ) X );

// get the number of outgoing links from object O
#define STAT_NODE p->node == NULL ? v[0] = 0. : v[0] = ( double ) p->node->nLinks;
#define STAT_NODES( O ) O == NULL ? v[0] = 0. : O->node == NULL ? v[0] = 0. : v[0] = ( double ) O->node->nLinks;

// add a link from object O to object X, both located inside same parent, same label
// and optional weight Y
#define ADDLINK( X ) ( p->add_link_net( X, 0 , 1 ) )
#define ADDLINKS( O, X ) ( O == NULL ? NULL : O->add_link_net( X, 0 , 1 ) )
#define ADDLINKW( X, Y ) ( p->add_link_net( X, Y, 1 ) )
#define ADDLINKWS( O, X, Y ) ( O == NULL ? NULL : O->add_link_net( X, Y, 1 ) )

// delete the link pointed by O
#define DELETE_LINK( O ) if ( O != NULL ) O->ptrFrom->delete_link_net( O );

// search outgoing links from object O for first occurrence of id=X
#define SEARCH_LINK( X ) ( p->search_link_net( ( long ) X ) )
#define SEARCH_LINKS( O, X ) ( O == NULL ? NULL : O->search_link_net( ( long ) X ) )

// random draw one link from a node
#define RNDDRAW_LINK ( p->draw_link_net() )
#define RNDDRAW_LINKS( O ) ( O == NULL ? NULL : O->draw_link_net( ) )

// set random draw probability to X for one link in a network
#define DRAWPROB_LINK( O, X ) { if ( O != NULL ) O->probTo = X; }

// get the destination object of link pointed by O
#define LINKTO( O ) ( O == NULL ? NULL : O->ptrTo )

// get the destination object of link pointed by O
#define LINKFROM( O ) ( O == NULL ? NULL : O->ptrFrom )

// get the weight of link pointed by O
#define V_LINK( O ) ( O == NULL ? 0. : O->weight )

// set the weight of link pointed by O to X
#define WRITE_LINK( O, X ) if ( O != NULL ) O->weight = X;

// cycle through set of links of object C, using link pointer O
#define CYCLE_LINK( O ) if ( p->node == NULL ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, "object '%s' has no network data structure", p->label ); \
		error_hard( msg, "invalid network object", "check your equation code to add\nthe network structure before using this macro", true ); \
	}\
	else \
		for ( O = p->node->first; O != NULL; O = O->next )

#define CYCLE_LINKS( C, O ) if ( C == NULL || C->node == NULL ) \
	{ \
		char msg[ TCL_BUFF_STR ]; \
		sprintf( msg, "object '%s' has no network data structure", C->label ); \
		error_hard( msg, "invalid network object", "check your equation code to add\nthe network structure before using this macro", true ); \
	} \
	else \
		for ( O = C->node->first; O != NULL; O = O->next )

// EXTENDED DATA/ATTRIBUTES MANAGEMENT MACROS
// macros for handling extended attributes (usually lists) attached to objects' cext pointer

// add/delete extension c++ data structures of type CLASS to a void pointer by current/PTR
#define ADDEXT( CLASS ) { if ( p->cext != NULL ) DELETE_EXT( CLASS ); p->cext = reinterpret_cast< void * >( new CLASS ); }
#define ADDEXTS( PTR, CLASS ) { if ( PTR->cext != NULL ) DELETE_EXTS( PTR, CLASS ); PTR->cext = reinterpret_cast< void * >( new CLASS ); }
#define DELETE_EXT( CLASS ) { delete reinterpret_cast< CLASS * >( p->cext ); p->cext = NULL; }
#define DELETE_EXTS( PTR, CLASS ) { delete reinterpret_cast< CLASS * >( PTR->cext ); PTR->cext = NULL; }

// convert current (or a pointer PTR from) void type in the user defined CLASS type
#define P_EXT( CLASS ) ( reinterpret_cast< CLASS * >( p->cext ) )
#define P_EXTS( PTR, CLASS ) ( reinterpret_cast< CLASS * >( PTR->cext ) )

// access the extension object directly
#define EXT( CLASS ) ( * P_EXT( CLASS ) )
#define EXTS( PTR, CLASS ) ( * P_EXTS( PTR, CLASS ) )

// read/write from object OBJ pointed by pointer current/PTR of type CLASS
#define V_EXT( CLASS, OBJ ) ( P_EXT( CLASS ) != NULL ? P_EXT( CLASS ) -> OBJ : 0 )
#define V_EXTS( PTR, CLASS, OBJ ) ( P_EXTS( PTR, CLASS ) != NULL ? P_EXTS( PTR, CLASS ) -> OBJ : 0 )
#define WRITE_EXT( CLASS, OBJ, VAL ) { if ( P_EXT( CLASS ) != NULL ) P_EXT( CLASS ) -> OBJ = VAL; }
#define WRITE_EXTS( PTR, CLASS, OBJ, VAL ) { if ( P_EXTS( PTR, CLASS ) != NULL ) P_EXTS( PTR, CLASS ) -> OBJ = VAL; }

// execute top CLASS level METHOD with the parameters ...
#define DO_EXT( CLASS, METHOD, ... ) ( P_EXT( CLASS ) -> METHOD( __VA_ARGS__ ) )
#define DO_EXTS( PTR, CLASS, METHOD, ... ) ( P_EXTS( PTR, CLASS ) -> METHOD( __VA_ARGS__ ) )

// execute METHOD contained in OBJ pointed by pointer current/PTR of type CLASS with the parameters ...
#define EXEC_EXT( CLASS, OBJ, METHOD, ... ) ( P_EXT( CLASS ) -> OBJ.METHOD( __VA_ARGS__ ) )
#define EXEC_EXTS( PTR, CLASS, OBJ, METHOD, ... ) ( P_EXTS( PTR, CLASS ) -> OBJ.METHOD( __VA_ARGS__ ) )

// cycle over elements of OBJ pointed by pointer current/PTR of type CLASS using iterator ITER
#define CYCLE_EXT( ITER, CLASS, OBJ ) for ( ITER = EXEC_EXT( CLASS, OBJ, begin ); ITER != EXEC_EXT( CLASS, OBJ, end ); ++ITER )
#define CYCLE_EXTS( PTR, ITER, CLASS, OBJ ) for ( ITER = EXEC_EXTS( PTR, CLASS, OBJ, begin ); ITER != EXEC_EXTS( PTR, CLASS, OBJ, end ); ++ITER )

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
// Initialise the regular space and use the object LAB contained in p as "Patches"
// Using Column Major (change?) the objects are added to a 2d grid and get xy coords respectively
#define INIT_SPACE_GRID( LAB, XN, YN )             { p->init_gis_regularGrid( LAB, XN, YN ); }
#define INIT_SPACE_GRIDS(PTR, LAB, XN, YN )             { PTR->init_gis_regularGrid( LAB, XN, YN ); }

#define INIT_SPACE_GRIDL( LAB, XN, YN, LAG )             { p->init_gis_regularGrid( LAB, XN, YN, 0, LAG ); }
#define INIT_SPACE_GRIDLS(PTR, LAB, XN, YN, LAG )             { PTR->init_gis_regularGrid( LAB, XN, YN, 0, LAG ); }

#define INIT_SPACE_GRID_WRAP( LAB, XN, YN, WRAP )  { p->init_gis_regularGrid( LAB, XN, YN, WRAP ); }
#define INIT_SPACE_GRID_WRAPS( PTR, LAB, XN, YN, WRAP )  { PTR->init_gis_regularGrid( LAB, XN, YN, WRAP ); }

#define INIT_SPACE_GRID_WRAPL( LAB, XN, YN, WRAP, LAG )  { p->init_gis_regularGrid( LAB, XN, YN, WRAP, LAG ); }
#define INIT_SPACE_GRID_WRAPLS( PTR, LAB, XN, YN, WRAP, LAG )  { PTR->init_gis_regularGrid( LAB, XN, YN, WRAP, LAG ); }

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
#define ADD_TO_SPACE_RND(TARGET) { p->register_at_map_rnd(TARGET); }
#define ADD_TO_SPACE_RNDS(PTR, TARGET) { PTR->register_at_map_rnd(TARGET); }

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

//Cycle through all the objects LAB anywhere in random order
#define CYCLE_GIS_RND( O, LAB ) for ( O = p->first_neighbour_rnd_full(LAB); O != NULL; O = p->next_neighbour() )
#define CYCLE_GIS_RNDS( C, O, LAB ) for ( O = C->first_neighbour_rnd_full(LAB); O != NULL; O = C->next_neighbour() )

// CYCLE_NEIGHBOUR
// Cycle through all the objects LAB within radius RAD by increasing radius
// _CND: Special version that checks conditions
// For each candidate it is checked if the Variable VAR with lag LAG called by
// either the candidate or CHEAT_C is  COND (<,>,==,!=) CONDVAL
// Note that CHEAT does not work with NULL.

#define CYCLE_NEIGHBOUR( O, LAB, RAD ) for ( O = p->first_neighbour(LAB, RAD, true); O != NULL; O = p->next_neighbour() )
#define CYCLE_NEIGHBOURS( C, O, LAB, RAD ) for ( O = C->first_neighbour(LAB, RAD, true); O != NULL; O = C->next_neighbour() )

#define CYCLE_NEIGHBOUR_CND(O, LAB, RAD, VAR, COND, CONDVAL ) for ( O = p->first_neighbour(LAB, RAD, true, NULL,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define CYCLE_NEIGHBOUR_CNDS(C, O, LAB, RAD, VAR, COND, CONDVAL ) for ( O = C->first_neighbour(LAB, RAD, true, NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define CYCLE_NEIGHBOUR_CNDL(O, LAB, RAD, VAR, COND, CONDVAL, LAG ) for ( O = p->first_neighbour(LAB, RAD, true, NULL,LAG,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define CYCLE_NEIGHBOUR_CNDLS(C, O, LAB, RAD, VAR, COND, CONDVAL, LAG ) for ( O = C->first_neighbour(LAB, RAD, true, NULL,0,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )

#define CYCLE_NEIGHBOUR_CND_CHEAT(O, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = p->first_neighbour(LAB, RAD, true,CHEAT_C,0,VAR,COND,CONDVAL); O!=NULL; O = p->next_neighbour() )
#define CYCLE_NEIGHBOUR_CND_CHEATS(C, O, LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  ) for ( O = C->first_neighbour(LAB, RAD, true,CHEAT_C,VAR,COND,CONDVAL); O!=NULL; O = C->next_neighbour() )


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
// measures the distance to a TARGET or a position
#define DISTANCE(TARGET) ( p -> distance (TARGET) )
#define DISTANCES(PTR, TARGET) ( PTR -> distance (TARGET) )
#define DISTANCE_XY(X, Y) ( p-> distance (X,Y) )
#define DISTANCE_XYS(PTR, X, Y) ( PTR-> distance (X,Y) )

//  SEARCH_POSITION, SEARCH_POSITION_RND
//  Searches at an exact position for an object with label LAB
//  If it exists it is reported. The RND version works if there can be more
//  than one object at the same place (returning one randomly)
//  The standard version will yield an error if more than one option exist.

#define SEARCH_POSITION_XY(LAB, X, Y)  ( p->search_at_position(LAB, X, Y, true) )
#define SEARCH_POSITION_XYS(PTR, LAB, X, Y)  ( PTR->search_at_position(LAB, X, Y, true) )
#define SEARCH_POSITION(LAB)  ( p->search_at_position(LAB, true) )
#define SEARCH_POSITIONS(PTR, LAB)  ( PTR->search_at_position(LAB, true) )

#define SEARCH_POSITION_RND_XY(LAB, X, Y)  ( p->search_at_position(LAB, X, Y, false) )
#define SEARCH_POSITION_RND_XYS(PTR, LAB, X, Y)  ( PTR->search_at_position(LAB, X, Y, false) )
#define SEARCH_POSITION_RND(LAB)  ( p->search_at_position(LAB, false) )
#define SEARCH_POSITION_RNDS(PTR, LAB)  ( PTR->search_at_position(LAB, false) )

//  SEARCH_POSITION_GRID and SEARCH_POSITION_RND_GRID
//  Similar to above, but it searches at the truncated position.
//  This macro is useful if one set of agents is distributed in continuous space
//  And another one, like "land patches", in discrete space. Then, the modeller
//  Can safely get information on the associated "land patch" of an "agent" with
//  this command.
#define SEARCH_POSITION_GRID(LAB)  ( p->search_at_position(LAB, true, true) )
#define SEARCH_POSITION_GRIDS(PTR, LAB)  ( PTR->search_at_position(LAB, true, true) )
#define SEARCH_POSITION_RND_GRID(LAB)  ( p->search_at_position(LAB, false, true) )
#define SEARCH_POSITION_RND_GRIDS(PTR, LAB)  ( PTR->search_at_position(LAB, false, true) )

//  COUNT_POSITION(S)(_GRID(S))
//  Similar to the search, you can use these macros to count the number of
//  elements at the given position.

#define COUNT_POSITION(LAB)  ( p->elements_at_position( LAB, false ) )
#define COUNT_POSITIONS(PTR, LAB) ( PTR->elements_at_position( LAB, false ) )

#define COUNT_POSITION_GRID(LAB)  ( p->elements_at_position( LAB, true ) )
#define COUNT_POSITION_GRIDS(PTR, LAB) ( PTR->elements_at_position( LAB, true ) )

// Additional Utilities
// ANY_GIS just checks if there is a map associated to the object
// SAME_GIS checks if the two objects share the same map
#define ANY_GIS ( p->position != NULL ? true : false  )
#define ANY_GISS(PTR) ( PTR->position != NULL ? true : false )

#define SAME_GIS(TARGET) ( p->ptr_map() == TARGET->ptr_map() )
#define SAME_GISS(PTR,TARGET) ( PTR->ptr_map() == TARGET->ptr_map() )

// GIS Lattice utilities
#define INIT_LAT_GIS( ... ) p->init_lattice_gis( __VA_ARGS__ )
#define INIT_LAT_GISS( PTR, ... ) PTR->init_lattice_gis( __VA_ARGS__ )
#define DELETE_LAT_GIS close_lattice( ) // convenient
#define SAVE_LAT_GIS( ... ) save_lattice( __VA_ARGS__ ) //convenient

#define V_LAT_GIS p->read_lattice_gis( )
#define V_LAT_GISS( PTR ) PTR->read_lattice_gis( )
#define WRITE_LAT_GIS( VAL ) p->update_lattice_gis( VAL )
#define WRITE_LAT_GISS( PTR, VAL ) PTR->update_lattice_gis( VAL )

#define V_LAT_GIS_XY( X, Y ) p->read_lattice_gis( X, Y, true )
#define V_LAT_GIS_XYS( PTR, X, Y ) PTR->read_lattice_gis( X, Y, true )
#define WRITE_LAT_GIS_XY( X, Y, VAL ) p->update_lattice_gis( X, Y, VAL, true )
#define WRITE_LAT_GIS_XYS( PTR, X, Y, VAL ) PTR->update_lattice_gis( X, Y, VAL, true )

#define V_LAT_GIS_ADJUST_XY( X, Y ) p->read_lattice_gis( X, Y, false )
#define V_LAT_GIS_ADJUST_XYS( PTR, X, Y ) PTR->read_lattice_gis( X, Y, false )
#define WRITE_LAT_GIS_ADJUST_XY( X, Y, VAL ) p->update_lattice_gis( X, Y, VAL, false )
#define WRITE_LAT_GIS_ADJUST_XYS( PTR, X, Y, VAL ) PTR->update_lattice_gis( X, Y, VAL, false )

#endif //#ifdef CPP11
	
// DEPRECATED MACRO COMPATIBILITY DEFINITIONS
// enabled only when directly including fun_head.h (and not fun_head_fast.h)
#ifndef FAST_LOOKUP

double init_lattice( double pixW = 0, double pixH = 0, double nrow = 100, double ncol = 100, 
					 char const lrow[ ] = "y", char const lcol[ ] = "x", char const lvar[ ] = "", 
					 object *p = NULL, int init_color = -0xffffff );
double poidev( double xm, long *idum_loc = NULL );
int deb( object *r, object *c, char const *lab, double *res, bool interact = false );
void cmd( const char *cm, ... );
#define FUNCTION( X ) EQUATION( X )
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
#define ADDS_EXT( PTR, CLASS ) ADDEXTS( PTR, CLASS )
#define DELETES_EXT( PTR, CLASS ) DELETE_EXTS( PTR, CLASS )
#define PS_EXT( PTR, CLASS ) P_EXTS( PTR, CLASS )
#define VS_EXT( PTR, CLASS, OBJ ) V_EXTS( PTR, CLASS, OBJ )
#define WRITES_EXT( PTR, CLASS, OBJ, VAL ) WRITE_EXTS( PTR, CLASS, OBJ, VAL )
#define EXECS_EXT( PTR, CLASS, OBJ, METHOD, ... ) EXEC_EXTS( PTR, CLASS, OBJ, METHOD, __VA_ARGS__ )
#define CYCLES_EXT( PTR, ITER, CLASS, OBJ ) CYCLE_EXTS( PTR, ITER, CLASS, OBJ )

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
