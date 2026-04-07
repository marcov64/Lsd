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
 FUN_TEST.CPP
 Macro compilation test file.
 *************************************************************/

#include <lsd_init.h>

#define EQ_USER_VARS int a; double b;
#define EQ_USER_CFUNS void x( void ) { };

//#define LEGACY_CODE
#include <lsd_head.h>

struct e
{
	int n;
	std::vector < int > p;
	e( int m = 0 ) : n( m ), p( 1, 0 ) { }
	int r( int m ) { n = m; return n; }
};


MODELBEGIN


EQUATION( "X" )
// Test variable

e g, *l;
std::string s;
std::vector < double > u;

pi;

b = abs( 0. );
b = exp( 0. );
b = fact( 1. );
b = log( 1. );
b = log10( 1. );
b = min( 0., 1. );
b = max( 0., 1. );
b = pow( 1., 1. );
b = ipow( 1., 1. );
b = round( 0. );
b = round_digits( 0., 0. );
b = sin( 0. );
b = cos( 0. );
b = tan( 0. );
b = asin( 0. );
b = acos( 0. );
b = atan( 0. );
b = sqrt( 0. );
b = cbrt( 0. );
b = tgamma( 1. );
b = lgamma( 1. );
b = mean( u );
b = med( u );
b = sd( u );
b = mad( u );
b = cov( u, u );
b = com( u, u );
b = t_star( 1., .95 );
b = z_star( .95 );
b = is_finite( 0. );
b = is_inf( 0. );
b = is_nan( 0. );
b = alapl( 0., 1., 1. );
b = bernoulli( .5 );
b = beta( 1., 1. );
b = binomial( .5, 1. );
b = bpareto( 1., 0., 1. );
b = cauchy( 1., 1. );
b = chi_squared( 1. );
b = exponential( 1. );
b = fisher( 1., 2. );
b = gamma( 1., 1. );
b = gamma( 1. );
b = geometric( 1. );
b = lnorm( 1., 1. );
b = norm( 0., 1. );
b = pareto( 1., 1. );
b = poisson( 1. );
b = student( 1. );
b = uniform( 0., 1. );
b = uniform_int( 0., 1. );
b = weibull( 1., 1. );
b = alaplS( cur, 0., 1., 1. );
b = bernoulliS( cur, .5 );
b = betaS( cur, 1., 1. );
b = binomialS( cur, .5, 1. );
b = bparetoS( cur, 1., 0., 1. );
b = cauchyS( cur, 1., 1. );
b = chi_squaredS( cur, 1. );
b = exponentialS( cur, 1. );
b = fisherS( cur, 1., 2. );
b = gammaS( cur, 1., 1. );
b = gammaS( cur, 1. );
b = geometricS( cur, 1. );
b = lnormS( cur, 1., 1. );
b = normS( cur, 0., 1. );
b = paretoS( cur, 1., 1. );
b = poissonS( cur, 1. );
b = studentS( cur, 1. );
b = uniformS( cur, 0., 1. );
b = uniform_intS( cur, 0., 1. );
b = weibullS( cur, 1., 1. );
b = alaplcdf( 0., 1., 1., 1. );
b = betacdf( 1., 1., 1. );
b = bparetocdf( 1., 0., 1., 1. );
b = lnormcdf( 0., 1., 1. );
b = normcdf( 0., 1., 1. );
b = paretocdf( 1., 1., 1. );
b = poissoncdf( 0., 1. );
b = unifcdf( 0., 1., 1. );

s = UP;
s = DOWN;

ABORT;
FAST;
FAST_FULL;
OBSERVE;
PARAMETER;

NO_NAN;
USE_NAN;
NO_POINTER_CHECK;
USE_POINTER_CHECK;
NO_SAVED;
USE_SAVED;
NO_SEARCH;
USE_SEARCH;
NO_SEARCH_UP;
USE_SEARCH_UP;
NO_ZERO_INSTANCE;
USE_ZERO_INSTANCE;

SLEEP( 1 );
DEBUG_START;
DEBUG_START_AT( 1 );
DEBUG_STOP;
DEBUG_STOP_AT( 1 );

LOG( "X" );
LOG( "%g", 1. );
PLOG( "X" );
PLOG( "%g", 1. );

s = NAME;
s = NAMES( cur );
s = CONFIG;
s = PATH;

b = CURRENT;
a = T;
a = LAST_T;
a = RUN;
a = LAST_RUN;
a = LAST_CALC( "X" );
a = LAST_CALCS( cur, "X" );

cur = ROOT;
cur = THIS;
cur = CALLER;
cur = NEXT;
cur = NEXTS( cur );
cur = PARENT;
cur = PARENTS( cur );
cur = GRANDPARENT;
cur = GRANDPARENTS( cur );

b = RECALC( "X" );
b = RECALCS( cur, "X" );
UPDATE;
UPDATES( cur );
UPDATE_REC;
UPDATE_RECS( cur );

b = RND;
b = RNDS( cur );
a = RND_SEED;
a = RND_SEEDS( cur );
RND_SETSEED( 1 );
RND_SETSEEDS( cur, 1 );
RND_GENERATOR( 0 );
RND_GENERATORS( cur, 0 );

b = V( "X" );
b = VL( "X", 1 );
b = VS( cur, "X" );
b = VLS( cur, "X", 1 );

b = MAVE( "X", 1 );
b = MAVEL( "X", 1, 1 );
b = MAVES( cur, "X", 1 );
b = MAVELS( cur, "X", 1, 1 );

b = WHTMAVE( "X", 1, v );
b = WHTMAVEL( "X", 1, v, 1 );
b = WHTMAVES( cur, "X", 1, v );
b = WHTMAVELS( cur, "X", 1, v, 1 );

b = SUM( "X" );
b = SUML( "X", 1 );
b = SUMS( cur, "X" );
b = SUMLS( cur, "X", 1 );
b = SUM_CND( "X", "Y", "==", 0 );
b = SUM_CNDL( "X", "Y", "==", 0, 1 );
b = SUM_CNDS( cur, "X", "Y", "==", 0 );
b = SUM_CNDLS( cur, "X", "Y", "==", 0, 1 );

b = MAX( "X" );
b = MAXL( "X", 1 );
b = MAXS( cur, "X" );
b = MAXLS( cur, "X", 1 );
b = MAX_CND( "X", "Y", "==", 0 );
b = MAX_CNDL( "X", "Y", "==", 0, 1 );
b = MAX_CNDS( cur, "X", "Y", "==", 0 );
b = MAX_CNDLS( cur, "X", "Y", "==", 0, 1 );

b = MIN( "X" );
b = MINL( "X", 1 );
b = MINS( cur, "X" );
b = MINLS( cur, "X", 1 );
b = MIN_CND( "X", "Y", "==", 0 );
b = MIN_CNDL( "X", "Y", "==", 0, 1 );
b = MIN_CNDS( cur, "X", "Y", "==", 0 );
b = MIN_CNDLS( cur, "X", "Y", "==", 0, 1 );

b = AVE( "X" );
b = AVEL( "X", 1 );
b = AVES( cur, "X" );
b = AVELS( cur, "X", 1 );
b = AVE_CND( "X", "Y", "==", 0 );
b = AVE_CNDL( "X", "Y", "==", 0, 1 );
b = AVE_CNDS( cur, "X", "Y", "==", 0 );
b = AVE_CNDLS( cur, "X", "Y", "==", 0, 1 );

b = WHTAVE( "X", "Y" );
b = WHTAVEL( "X", "Y", 1 );
b = WHTAVES( cur, "X", "Y" );
b = WHTAVELS( cur, "X", "Y", 1 );
b = WHTAVE_CND( "X", "Y", "Z", "==", 0 );
b = WHTAVE_CNDL( "X", "Y", "Z", "==", 0, 1 );
b = WHTAVE_CNDS( cur, "X", "Y", "Z", "==", 0 );
b = WHTAVE_CNDLS( cur, "X", "Y", "Z", "==", 0, 1 );

b = MED( "X" );
b = MEDL( "X", 1 );
b = MEDS( cur, "X" );
b = MEDLS( cur, "X", 1 );
b = MED_CND( "X", "Y", "==", 0 );
b = MED_CNDL( "X", "Y", "==", 0, 1 );
b = MED_CNDS( cur, "X", "Y", "==", 0 );
b = MED_CNDLS( cur, "X", "Y", "==", 0, 1 );

b = PERC( "X", 1. );
b = PERCL( "X", 1., 1 );
b = PERCS( cur, "X", 1. );
b = PERCLS( cur, "X", 1., 1 );
b = PERC_CND( "X", 1., "Y", "==", 0 );
b = PERC_CNDL( "X", 1., "Y", "==", 0, 1 );
b = PERC_CNDS( cur, "X", 1., "Y", "==", 0 );
b = PERC_CNDLS( cur, "X", 1., "Y", "==", 0, 1 );

b = SD( "X" );
b = SDL( "X", 1 );
b = SDS( cur, "X" );
b = SDLS( cur, "X", 1 );
b = SD_CND( "X", "Y", "==", 0 );
b = SD_CNDL( "X", "Y", "==", 0, 1 );
b = SD_CNDS( cur, "X", "Y", "==", 0 );
b = SD_CNDLS( cur, "X", "Y", "==", 0, 1 );

a = COUNT( "X" );
a = COUNTS( cur, "X" );
a = COUNT_CND( "X", "Y", "==", 0 );
a = COUNT_CNDL( "X", "Y", "==", 0, 1 );
a = COUNT_CNDS( cur, "X", "Y", "==", 0 );
a = COUNT_CNDLS( cur, "X", "Y", "==", 0, 1 );

a = COUNT_ALL( "X" );
a = COUNT_ALLS( cur, "X" );
a = COUNT_ALL_CND( "X", "Y", "==", 0 );
a = COUNT_ALL_CNDL( "X", "Y", "==", 0, 1 );
a = COUNT_ALL_CNDS( cur, "X", "Y", "==", 0 );
a = COUNT_ALL_CNDLS( cur, "X", "Y", "==", 0, 1 );

b = STAT( "X" );
b = STATL( "X", 1 );
b = STATS( cur, "X" );
b = STATLS( cur, "X", 1 );
b = STAT_CND( "X", "Y", "==", 0 );
b = STAT_CNDL( "X", "Y", "==", 0, 1 );
b = STAT_CNDS( cur, "X", "Y", "==", 0 );
b = STAT_CNDLS( cur, "X", "Y", "==", 0, 1 );

b = INTERACT( "X", 0. );
b = INTERACTS( cur, "X", 0. );

cur = SEARCH( "X" );
cur = SEARCHS( cur, "X" );
cur = SEARCH_CND( "X", 0. );
cur = SEARCH_CNDL( "X", 0., 1 );
cur = SEARCH_CNDS( cur, "X", 0. );
cur = SEARCH_CNDLS( cur, "X", 0., 1 );

a = SEARCH_INST( cur1 );
a = SEARCH_INSTS( cur, cur1 );

cur = RNDDRAW( "X", "Y" );
cur = RNDDRAWL( "X", "Y", 1 );
cur = RNDDRAWS( cur, "X", "Y" );
cur = RNDDRAWLS( cur, "X", "Y", 1 );

cur = RNDDRAW_FAIR( "X" );
cur = RNDDRAW_FAIRS( cur, "X" );
cur = RNDDRAW_TOT( "X", "Y", 1. );
cur = RNDDRAW_TOTL( "X", "Y", 1, 1. );
cur = RNDDRAW_TOTS( cur, "X", "Y", 1. );
cur = RNDDRAW_TOTLS( cur, "X", "Y", 1, 1. );

b = WRITE( "X", 0. );
b = WRITEL( "X", 0., 1 );
b = WRITELL( "X", 0., 1, 1 );
b = WRITES( cur, "X", 0. );
b = WRITELS( cur, "X", 0., 1 );
b = WRITELLS( cur, "X", 0., 1, 1 );

b = INCR( "X", 0. );
b = INCRS( cur, "X", 0. );

b = MULT( "X", 0. );
b = MULTS( cur, "X", 0. );

cur = ADDOBJ( "X" );
cur = ADDOBJL( "X", 1 );
cur = ADDOBJS( cur, "X" );
cur = ADDOBJLS( cur, "X", 1 );
cur = ADDNOBJ( "X", 1 );
cur = ADDNOBJL( "X", 1, 1 );
cur = ADDNOBJS( cur, "X", 1 );
cur = ADDNOBJLS( cur, "X", 1, 1 );

cur = ADDOBJ_EX( "X", cur1 );
cur = ADDOBJ_EXL( "X", cur1, 1 );
cur = ADDOBJ_EXS( cur, "X", cur1 );
cur = ADDOBJ_EXLS( cur, "X", cur1, 1 );
cur = ADDNOBJ_EX( "X", 1, cur1 );
cur = ADDNOBJ_EXL( "X", 1, cur1, 1 );
cur = ADDNOBJ_EXS( cur, "X", 1, cur1 );
cur = ADDNOBJ_EXLS( cur, "X", 1, cur1, 1 );

DELETE( cur );
a = DELETING;
a = DELETINGS( cur );

cur = SORT( "X", "Y", UP );
cur = SORTL( "X", "Y", UP, 1 );
cur = SORTS( cur, "X", "Y", UP );
cur = SORTLS( cur, "X", "Y", UP, 1 );

cur = SORT2( "X", "Y", "Z", DOWN );
cur = SORT2L( "X", "Y", "Z", DOWN, 1 );
cur = SORT2S( cur, "X", "Y", "Z", DOWN );
cur = SORT2LS( cur, "X", "Y", "Z", DOWN, 1 );

cur = HOOK( 0 );
cur = HOOKS( cur, 0 );
cur = SHOOK;
cur = SHOOKS( cur );

cur = WRITE_HOOK( 0, cur1 );
cur = WRITE_HOOKS( cur, 0, cur1 );
cur = WRITE_SHOOK( cur );
cur = WRITE_SHOOKS( cur, cur1 );

ADDHOOK( 1 );
ADDHOOKS( cur, 1 );
a = COUNT_HOOK;
a = COUNT_HOOKS( cur );

cur = DOWN_LAT;
cur = DOWN_LATS( cur );
cur = LEFT_LAT;
cur = LEFT_LATS( cur );
cur = RIGHT_LAT;
cur = RIGHT_LATS( cur );
cur = UP_LAT;
cur = UP_LATS( cur );

a = INIT_LAT( 0 );
a = INIT_LAT( 0, 1, 1 );
a = INIT_LAT( 0, 1, 1, 1, 1 );
a = SAVE_LAT( );
a = SAVE_LAT( "X" );
DELETE_LAT;

b = V_LAT( 1, 1 );
b = WRITE_LAT( 1, 1 );
b = WRITE_LAT( 1, 1, 0 );

a = V_NODEID;
a = V_NODEIDS( cur );
s = V_NODENAME;
s = V_NODENAMES( cur );
b = V_LINK( curl );

b = STAT_NET( "X" );
b = STAT_NETS( cur, "X" );
b = STAT_NODE;
b = STAT_NODES( cur );

cur = SEARCH_NODE( "X", 0 );
cur = SEARCH_NODES( cur, "X", 0 );
curl = SEARCH_LINK( 0 );
curl = SEARCH_LINKS( cur, 0 );

cur = RNDDRAW_NODE( "X" );
cur = RNDDRAW_NODES( cur, "X" );
curl = RNDDRAW_LINK;
curl = RNDDRAW_LINKS( cur );

b = DRAWPROB_NODE( 0. );
b = DRAWPROB_NODES( cur, 0. );
b = DRAWPROB_LINK( curl, 0. );

cur = LINKTO( curl );
cur = LINKFROM( curl );

a = WRITE_NODEID( 0 );
a = WRITE_NODEIDS( cur, 0 );
WRITE_NODENAME( "X" );
WRITE_NODENAMES( cur, "X" );
b = WRITE_LINK( curl, 0. );

a = INIT_NET( "X" );
a = INIT_NET( "X", "DISCONNECTED" );
a = INIT_NET( "X", "DISCONNECTED", 0 );
a = INIT_NET( "X", "DISCONNECTED", 0, 0 );
a = INIT_NET( "X", "DISCONNECTED", 0, 0, 0. );
a = INIT_NETS( cur, "X" );
a = INIT_NETS( cur, "X", "DISCONNECTED" );
a = INIT_NETS( cur, "X", "DISCONNECTED", 0 );
a = INIT_NETS( cur, "X", "DISCONNECTED", 0, 0 );
a = INIT_NETS( cur, "X", "DISCONNECTED", 0, 0, 0. );

a = LOAD_NET( "X", "Y" );
a = LOAD_NETS( cur, "X", "Y" );
a = SAVE_NET( "X", "Y" );
a = SAVE_NETS( cur, "X", "Y" );
a = SNAP_NET( "X", "Y" );
a = SNAP_NETS( cur, "X", "Y" );

cur = ADDNODE( 0, "Y" );
cur = ADDNODES( cur, 0, "Y" );

curl = ADDLINK( cur );
curl = ADDLINKS( cur, cur1 );
curl = ADDLINKW( cur, 0. );
curl = ADDLINKWS( cur, cur1, 0. );

DELETE_NET( "X" );
DELETE_NETS( cur, "X" );
DELETE_NODE;
DELETE_NODES( cur );
DELETE_LINK( curl );

cur = SHUFFLE_NET( "X" );
cur = SHUFFLE_NETS( cur, "X" );

a = INIT_TSEARCH( "X" );
a = INIT_TSEARCHS( cur, "X" );
a = TSEARCH_SET( "X" );
a = TSEARCH_SETS( cur, "X" );
cur = TSEARCH( "X", 1 );
cur = TSEARCHS( cur, "X", 1 );

a = INIT_TSEARCH_CND( "X" );
a = INIT_TSEARCH_CNDS( cur, "X" );
a = TSEARCH_CND_SET( "X" );
a = TSEARCH_CND_SETS( cur, "X" );
cur = TSEARCH_CND( "X", 0. );
cur = TSEARCH_CNDS( cur, "X", 0. );

b = V_CHEAT( "X", cur1 );
b = V_CHEATL( "X", 1, cur1 );
b = V_CHEATS( cur, "X", cur1 );
b = V_CHEATLS( cur, "X", 1, cur1 );

ADDEXT( e )
ADDEXTS( cur, e )
ADDEXT_INIT( e, 0 )
ADDEXT_INITS( cur, e, 0 )

DELETE_EXT( e )
DELETE_EXTS( cur, e )

a = V_EXT( e, n );
a = V_EXTS( cur, e, n );
b = DO_EXT( e, r, 0 );
b = DO_EXTS( cur, e, r, 0 );
a = EXEC_EXT( e, p, size );
a = EXEC_EXTS( cur, e, p, size );

g = EXT( e );
g = EXTS( cur, e );
l = P_EXT( e );
l = P_EXTS( cur, e );

a = WRITE_EXT( e, n, 0 );
a = WRITE_EXTS( cur, e, n, 0 );
a = WRITE_ARG_EXT( e, p.at, 0, 0 );
a = WRITE_ARG_EXTS( cur, e, p.at, 0, 0 );

CYCLE( cur, "Y" ) { }
CYCLE_SAFE( cur, "Y" ) { }
CYCLE2_SAFE( cur, "Y" ) { }
CYCLE3_SAFE( cur, "Y" ) { }

#ifndef LEGACY_CODE
auto p = _p_;
#endif

cur = p;
CYCLES( cur, cur1, "Y" ) { }
CYCLE_SAFES( cur, cur1, "Y" ) { }
CYCLE2_SAFES( cur, cur1, "Y" ) { }
CYCLE3_SAFES( cur, cur1, "Y" ) { }

CYCLE_LINK( curl ) { }
CYCLE_LINKS( cur, curl ) { }

std::vector < int >::iterator q;
CYCLE_EXT( q, e, p ) { }
CYCLE_EXTS( cur, q, e, p ) { }

#ifdef LEGACY_CODE
s = msg;
a = deb( p, c, "X", & b );
a = deb( p, c, "X", & b, false );
a = deb( p, c, "X", & b, false, "" );
cmd( "X" );
cmd( "%g", 1. );
plog( "X" );
plog( "%g", 1. );
const simulation *z = SIM;
const variable *w = var;
cur = c;
cur = caller;
cur = p;
b = t;
cur = root;
a = seed;
quit = 0;
object *o = NULL;
s = path;
b = poidev( 1. );
cur = go_brother( cur1 );
b = UNIFORM( 0., 1. );
b = rnd_integer( 0, 1 );
b = VL_CHEAT( "X", 1, cur1 );
b = VS_CHEAT( cur, "X", cur1 );
b = VLS_CHEAT( cur, "X", 1, cur1 );
cur = ADDOBJL_EX( "X", cur1, 1 );
cur = ADDOBJS_EX( cur, "X", cur1 );
cur = ADDOBJLS_EX( cur, "X", cur1, 1 );
cur = ADDNOBJL_EX( "X", 1, cur1, 1 );
cur = ADDNOBJS_EX( cur, "X", 1, cur1 );
cur = ADDNOBJLS_EX( cur, "X", 1, cur1, 1 );
a = INIT_TSEARCHT( "X", 1 );
a = INIT_TSEARCHTS( cur, "X", 1 );
a = TSEARCH_INI( "X" );
a = TSEARCHS_INI( cur, "X" );
a = TSEARCHT_INI( "X", 1 );
a = TSEARCHTS_INI( cur, "X", 1 );
cur = TSEARCHT( "X", 1, 1 );
cur = TSEARCHTS( cur, "X", 1, 1 );
cur = SORTS2( cur, "X", "Y", "Z", DOWN );
cur = RNDDRAWFAIR( "X" );
cur = RNDDRAWFAIRS( cur, "X" );
cur = RNDDRAWTOT( "X", "Y", 1. );
cur = RNDDRAWTOTL( "X", "Y", 1, 1. );
cur = RNDDRAWTOTS( cur, "X", "Y", 1. );
cur = RNDDRAWTOTLS( cur, "X", "Y", 1, 1. );
a = NETWORK_INI( "X", "DISCONNECTED", 0, 0, 0. );
a = NETWORKS_INI( cur, "X", "DISCONNECTED", 0, 0, 0. );
a = NETWORK_LOAD( "X", "Y", "Z" );
a = NETWORKS_LOAD( cur, "X", "Y", "Z" );
a = NETWORK_SAVE( "X", "Y", "Z" );
a = NETWORKS_SAVE( cur, "X", "Y", "Z" );
b = STATS_NET( cur, "X" );
cur = SHUFFLE( "X" );
cur = SHUFFLES( cur, "X" );
cur = RNDDRAW_NET( "X" );
cur = RNDDRAWS_NET( cur, "X" );
cur = SEARCH_NET( "X", 0 );
cur = SEARCHS_NET( cur, "X", 0 );
a = VS_NODEID( cur );
s = VS_NODENAME( cur );
a = WRITES_NODEID( cur, 0 );
WRITES_NODENAME( cur, "X" );
b = STATS_NODE( cur );
DELETELINK( curl );
curl = SEARCHS_LINK( cur, 0 );
b = VS_WEIGHT( curl );
b = WRITES_WEIGHT( curl, 0. );
ADD_EXT( e );
ADDS_EXT( cur, e );
DELETES_EXT( cur, e );
l = PS_EXT( cur, e );
a = VS_EXT( cur, e, n );
a = WRITES_EXT( cur, e, n, 0 );
a = EXECS_EXT( cur, e, p, size );
DEBUG;
DEBUG_AT( 1 );
CYCLES_EXT( cur, q, e, p ) { }
CYCLES_LINK( cur, curl ) { }
#endif

END_EQUATION( 0 );

RESULT( 0 )


EQUATION_DUMMY( "Y", "X" )
// Test dummy equation


#ifdef LEGACY_CODE
FUNCTION( "Z" )
// Test function
RESULT( 0 )
#endif


MODELEND

#ifndef LEGACY_CODE
CLOSEBEGIN
CLOSEEND
#else
void close_sim( void ) { }
#endif
