/******************************************************************************

	CLASS AND MACRO DEFINITIONS
	---------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	C++ class and preprocessor macro definitions used in the K+S LSD model
	are coded below.

 ******************************************************************************/

// set default name spaces (C++ STL, LSD)
using namespace std;
using namespace lsd;

// K+S random engine (!= LSD)
mt19937_64 random_engine;


/*============================= GENERAL CLASSES ==============================*/

struct firmRank									// element of pecking order rank
{
	double NWtoS;								// net-wealth-to-sales ratio
	object *firm;								// pointer to firm
};

struct vintage									// element of map of vintages
{
	double sVp;									// public skills for vintage
	double sVavg, sVavgLag;						// average skills for vintage
	int workers;								// current workers using
};

struct wageOffer								// element of wage offer list
{
	double offer;								// wage offer value
	int workers;								// workers in firm
	object *firm;								// pointer to firm
};

struct application								// element of application list
{
	double w, edS, wEdS;						// ordering attributes
	int Te, cat;
	object *wrk;								// pointer to worker
};

typedef pair < int, object * > firmPairT;		// firm-to-object pair template
typedef map < int, object * > firmMapT;			// firm-to-object map template
typedef set < object * > firmSeT;				// firm-set template
typedef map < int, vintage > vintMapT;			// ID-to-vintage map template
typedef list < firmRank > firmLisT;				// ranked-firms list template
typedef list < wageOffer > woLisT;				// ranked-wage-offer list
typedef list < application > appLisT;			// job-application list
typedef vector < double > dblVecT;				// vector of doubles template
typedef vector < int > intVecT;					// vector of integers template
typedef vector < object * > objVecT;			// vector of objects template
typedef unsigned long long ullongT;				// very wide unsigned integer


/*======================== COUNTRY EXTENSION CLASS ===========================*/

struct countryE
{
	// static global pointers to speed-up the access to individual containers
	object *finSec, *capSec, *conSec, *labSup, *macSta, *secSta, *labSta;

	// country speed-up vectors & maps
	dblVecT bankWgtd;							// m. s. cum. weights in banking
	dblVecT firm2wgtd;							// m. s. cum. weights in sector 2
	objVecT bankPtr;							// pointers to banks
	objVecT firm2ptr;							// pointers to firms in sector 2
	firmMapT firm2map;							// ID to pointer map for sector 2
	vintMapT vintProd;							// vintage productivities & skills

	// country lists
	woLisT firm2wo1, firm2wo2, firm2wo3;		// lists of category wage offers
	appLisT firm1appl;							// sector 1 job applications

	// mutexes for synchronizing critical parallel operations on the above items
	mutex vintProdMtx, firm2woMtx, firmApplMtx;
};


/*=========================== FIRM EXTENSION CLASS ===========================*/

struct firm2E									// extensions to Firm2 object
{
	appLisT appl;								// firm job applications
};


/*======================= INITIAL NOTIONAL DEFINITIONS =======================*/

#define INIPROD		1					// initial notional machine productivity
#define INIWAGE		1					// initial notional wage (category 1)
#define INISKILL	1					// initial notional worker skills
#define INIEXCH		1					// initial notional exchange rate


/*========================= HOOK-RELATED DEFINITIONS =========================*/

// number of dynamic hooks per object type
#define FIRM1HK		2				// Firm1
#define FIRM2HK		4				// Firm2
#define WORKERHK	2				// Worker

// dynamic hook name to number
#define BANK		0				// from Firm1/Firm2 to Bank
#define BCLIENT		1				// from Firm1/Firm2 to Cli1/Cli2 (in Bank)
#define SUPPL		2				// from Firm2 to Broch (in Firm2)
#define TOPVINT		3				// from Firm2 to Vint (in Firm2)
#define FWRK		0				// from Worker to Wkr1/Wrkr2 (in Capital/Firm2)
#define VWRK		1				// from Worker to WrkV (in Vint in Firm2)


/*======================= OBJECT-LOCATION DEFINITIONS ========================*/

// pointers to speed-up the access to individual market containers by caller
// equation levels
#define WORLDL0 PARENT
#define WORLDL1 GRANDPARENT
#define WORLDL2 PARENTS( GRANDPARENT )
#define COUNTRL0 THIS
#define COUNTRL1 PARENT
#define COUNTRL2 GRANDPARENT
#define COUNTRL3 PARENTS( GRANDPARENT )
#define CAPSECL0 V_EXT( countryE, capSec )
#define CAPSECL1 V_EXTS( PARENT, countryE, capSec )
#define CAPSECL2 V_EXTS( GRANDPARENT, countryE, capSec )
#define CONSECL0 V_EXT( countryE, conSec )
#define CONSECL1 V_EXTS( PARENT, countryE, conSec )
#define CONSECL2 V_EXTS( GRANDPARENT, countryE, conSec )
#define LABSUPL0 V_EXT( countryE, labSup )
#define LABSUPL1 V_EXTS( PARENT, countryE, labSup )
#define LABSUPL2 V_EXTS( GRANDPARENT, countryE, labSup )
#define LABSUPL3 V_EXTS( PARENTS( GRANDPARENT ), countryE, labSup )
#define FINSECL0 V_EXT( countryE, finSec )
#define FINSECL1 V_EXTS( PARENT, countryE, finSec )
#define FINSECL2 V_EXTS( GRANDPARENT, countryE, finSec )
#define MACSTAL0 V_EXT( countryE, macSta )
#define MACSTAL1 V_EXTS( PARENT, countryE, macSta )
#define MACSTAL2 V_EXTS( GRANDPARENT, countryE, macSta )
#define SECSTAL0 V_EXT( countryE, secSta )
#define SECSTAL1 V_EXTS( PARENT, countryE, secSta )
#define SECSTAL2 V_EXTS( GRANDPARENT, countryE, secSta )
#define LABSTAL0 V_EXT( countryE, labSta )
#define LABSTAL1 V_EXTS( PARENT, countryE, labSta )
#define LABSTAL2 V_EXTS( GRANDPARENT, countryE, labSta )


/*============================== SUPPORT MACROS ==============================*/

// macro for checking if variable was already computed (used in timeStep)
#define NEW_VS( VAL, OBJ, VAR ) \
	if ( LAST_CALCS( OBJ, VAR ) == T ) \
		LOG( "\n (t=%g) Variable '%s = %.4g' already computed", \
			 T, VAR, VAL = VS( OBJ, VAR ) ); \
	else \
		VAL = VS( OBJ, VAR );

// macro to round values too close to a reference
#define ROUND( V, Ref, Tol ) ( abs( V - Ref ) > Tol ? V : Ref )

// macros to pack/unpack firm ID in multiple countries and industries
#define ID( IDcountry, IDind, FIRM_NUM ) \
	( 1000000000 * ( ullongT ) trunc( IDcountry ) + \
	  100000	 * ( ullongT ) trunc( IDind ) + \
	  1			 * ( ullongT ) trunc( FIRM_NUM ) )
#define ID_CNT( ID ) \
	( ( ullongT ) round( ( ullongT ) trunc( ID ) / 1000000000 ) )
#define ID_IND( ID ) \
	( ( ullongT ) round( ( ( ullongT ) trunc( ID ) - \
						   1000000000 * ID_CNT( ID ) ) / 100000 ) )
#define FIRM_NUM( ID ) \
	( ( ullongT ) trunc( ID ) - \
	  100000 * ID_IND( ID ) - \
	  1000000000 * ID_CNT( ID ) )

// macros to pack/unpack vintage (machine technological generation) data
#define VNT( T0, IDsuppl ) \
	( 100000000 * ( ullongT ) trunc( T0 ) + \
	  100000	* ID_CNT( IDsuppl ) + \
	  1			* FIRM_NUM( IDsuppl ) )
#define T0( IDvint ) \
	( ( ullongT ) round( ( ullongT ) trunc( IDvint ) / 100000000 ) )
#define CNT( IDvint ) \
	( ( ullongT ) round( ( ( ullongT ) trunc( IDvint ) - \
						   100000000 * T0( IDvint ) ) / 100000 ) )
#define SUP( IDvint ) \
	ID( CNT( IDvint ), 1, ( ullongT ) trunc( IDvint ) - \
						  100000 * CNT( IDvint ) - \
						  100000000 * T0( IDvint ) )

// macros to work with standard C arrays
#define LEN_ARR( A ) ( ( int ) ( sizeof A / sizeof A[0] ) )
#define END_ARR( A ) ( A + LEN_ARR( A ) )


/*=================== FORWARD DECLARATION OF C++ FUNCTIONS ===================*/

#define EQ_USER_CFUNS \
	CFUN_DBL( cash_flow, double profit, double tax ); \
	CFUN_DBL( count_workers, int cat ); \
	CFUN_DBL( entry_firm1, int n, bool newInd ); \
	CFUN_DBL( entry_firm2, int n, bool newInd ); \
	CFUN_DBL( exit_firm, double *firesAcc ); \
	CFUN_DBL( fire_workers, int mode, double xsCap, double *redCap ); \
	CFUN_DBL( hire_workers, int cat ); \
	CFUN_DBL( invest, double desired ); \
	CFUN_DBL( mov_avg_bound, const char *var, double lim, double per, int lag ); \
	CFUN_DBL( open_positions, int cat ); \
	CFUN_DBL( rescue_firm, bool stat ); \
	CFUN_DBL( scrap_vintage ); \
	CFUN_DBL( update_debt, double desired, double loan ); \
	CFUN_DBL( update_depo, double depo, bool incr ); \
	CFUN_DBL( wage_offer, int cat ); \
	CFUN_OBJ( send_brochure, object *client ); \
	CFUN_OBJ( set_bank ); \
	CFUN_OBJ( set_supplier ); \
	CFUN_VOID( add_vintage, double nMach, bool newInd ); \
	CFUN_VOID( best_tech, double & AtauMax, double & BtauMax ); \
	CFUN_VOID( check_error, bool cond, const char* errMsg, int errCount, \
			   int *errCounter ); \
	CFUN_VOID( fire_worker ); \
	CFUN_VOID( hire_worker, int sec, object *firm, double wage ); \
	CFUN_VOID( move_worker, object *vint, bool vint_learn ); \
	CFUN_VOID( order_applications, int order, appLisT *appl ); \
	CFUN_VOID( order_offers, int order, woLisT *offers ); \
	CFUN_VOID( order_workers, int order, int obj ); \
	CFUN_VOID( send_order, double nMach ); \
	CFUN_VOID( set_education ); \
	CFUN_VOID( shuffle_offers, woLisT *offers );
