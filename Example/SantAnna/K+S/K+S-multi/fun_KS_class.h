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

struct application								// element of application list
{
	double w, s, ws, Te;						// ordering attributes
	object *wrk;								// pointer to worker
};

struct buyOrder									// element of buy-order list
{
	double bdgt;								// budget to buy
	object *wrk;								// pointer to worker/consumer
};

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

typedef const variable cVarT;					// LSD read-only variable type
typedef list < application > appLisT;			// job-application list
typedef list < buyOrder > buyLisT;				// buy-order list template
typedef list < firmRank > firmLisT;				// ranked-firms list template
typedef list < wageOffer > woLisT;				// ranked-wage-offer list
typedef map < int, object * > firmMapT;			// firm-to-object map template
typedef map < int, int > goodMapT;				// consumer good map template
typedef map < int, vintage > vintMapT;			// ID-to-vintage map template
typedef pair < int, int > goodPairT;			// good-to-time pair template
typedef pair < int, object * > firmPairT;		// firm-to-object pair template
typedef set < object * > firmSeT;				// firm-set template
typedef vector < double > dblVecT;				// vector of doubles template
typedef vector < object * > objVecT;			// vector of objects template


/*======================== COUNTRY EXTENSION CLASS ===========================*/

struct countryE
{
	// static global pointers to speed-up the access to individual containers
	object *finSec, *labSup, *macSta, *secSta, *labSta;

	// country speed-up vectors
	dblVecT bankWgtd;							// m. s. cum. weights in banking
	dblVecT firm2wgtd;							// m. s. cum. weights in sector 2
	dblVecT ind2wgtd;							// compet. cum. weights in s. 2
	objVecT bankPtr;							// pointers to banks
	objVecT g1ptr;								// pointers to tech. gen. in s.1
	objVecT firm2ptr;							// pointers to firms in sector 2
	objVecT ind2ptr;							// pointers to industries in s.2

	// country speed-up maps
	firmMapT firm2map;							// ID to pointer map for sector 2
	vintMapT vintProd;							// vintage productivities & skills

	// country lists
	woLisT firm2wo;								// list of wage offers
	appLisT firm1appl;							// sector 1 job applications
};


/*========================= INDUSTRY EXTENSION CLASS =========================*/

struct ind2E									// extensions to Consumption obj.
{
	buyLisT buyOrd;								// luxury-good buy orders
};


/*=========================== FIRM EXTENSION CLASS ===========================*/

struct firm2E									// extensions to Firm2 object
{
	appLisT appl;								// firm job applications
};


/*========================== WORKER EXTENSION CLASS ==========================*/

struct wrkE										// extensions to Worker object
{
	goodMapT buyLux;							// map to luxury goods bought
};


/*======================= INITIAL NOTIONAL DEFINITIONS =======================*/

#define INICPLX		1			// initial notional basic product complexity
#define INIPROD		1			// initial notional machine productivity
#define INIWAGE		1			// initial notional wage
#define INISKILL	1			// initial notional worker skills


/*========================= HOOK-RELATED DEFINITIONS =========================*/

// number of dynamic hooks per object type
#define FIRM1HK		2			// Firm1 (in Capital)
#define FIRM2HK		4			// Firm2 (in Consumption)
#define WORKERHK	2			// Worker (in Labor)
#define VINTHK		1			// Vint (in Consumption/Firm2)

// dynamic hook name to number
#define BANK		0			// from Firm1/Firm2 to Bank
#define BCLIENT		1			// from Firm1/Firm2 to Cli1/Cli2 (in Bank)
#define SUPPL		2			// from Firm2 to Broch (in Firm2)
#define TOPVINT		3			// from Firm2 to Vint (in Firm2)
#define FWRK		0			// from Worker to Wkr1/Wrkr2 (in Capital/Firm2)
#define VWRK		1			// from Worker to WrkV (in Firm2/Vint)
#define TGEN		0			// from Vint to T1 (in Capital/T1)


/*======================= OBJECT-LOCATION DEFINITIONS ========================*/

// pointers to speed-up the access to individual containers by caller
// calling equation levels (L0=country)
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

// macro to round values too close to a reference
#define ROUND( V, Ref, Tol ) ( abs( V - Ref ) > Tol ? V : Ref )

// macros to pack/unpack firm ID in multiple-industry sector
#define ID( IDind, numFirm ) \
	( 10000 * ( int ) trunc( IDind ) + ( int ) trunc( numFirm ) )
#define ID_IND( ID ) \
	( ( int ) round( trunc( ID ) / 10000 ) )
#define FIRM_NUM( ID ) \
	( ( int ) ( trunc( ID ) - 10000 * ID_IND( ID ) ) )

// macros to pack/unpack vintage (machine technological generation) data
#define VNT( T0, IDsuppl ) \
	( 10000 * ( int ) trunc( T0 ) + ( int ) trunc( IDsuppl ) )
#define T0( IDvint ) \
	( ( int ) round( trunc( IDvint ) / 10000 ) )
#define SUP( IDvint ) \
	( ( int ) ( trunc( IDvint ) - 10000 * T0( IDvint ) ) )

// macros to work with standard C arrays
#define LEN_ARR( A ) ( ( int ) ( sizeof A / sizeof A[0] ) )
#define END_ARR( A ) ( A + LEN_ARR( A ) )


/*================== FORWARD DECLARATION OF C++ EXTENSIONS ===================*/

#define EQ_USER_VARS dblVecT::iterator itd, itd1; \
					 objVecT::iterator ito, ito1; \
					 appLisT::iterator its, its1; \
					 woLisT::iterator itw, itw1;// general purpose iterators

#define EQ_USER_CFUNS \
	CFUN_DBL( add_generation ); \
	CFUN_DBL( entry_bank, int n, bool init ); \
	CFUN_DBL( entry_consumption, int n, bool basic, double f20, double *F2acc ); \
	CFUN_DBL( entry_firm1, int n, bool newInd ); \
	CFUN_DBL( entry_firm2, int n, bool newInd ); \
	CFUN_DBL( entry_worker, int n, bool init ); \
	CFUN_DBL( exit_consumption, double *firmExits ); \
	CFUN_DBL( exit_firm1, double *cEntry, double *cExit, double *nFail, bool all ); \
	CFUN_DBL( exit_firm2, double *cEntry, double *cExit, double *nFail, bool all ); \
	CFUN_DBL( fire_workers, int mode, double xsCap, double *redCap ); \
	CFUN_DBL( init_bank, bool init ); \
	CFUN_DBL( mov_avg_bound, const char *var, double lim, double per, int lag ); \
	CFUN_DBL( scrap_vintage ); \
	CFUN_OBJ( send_brochure, object *client ); \
	CFUN_OBJ( set_bank ); \
	CFUN_VOID( add_vintage, double nMach, bool newInd ); \
	CFUN_VOID( check_error, bool cond, const char* errMsg, int errCount, \
			   int *errCounter ); \
	CFUN_VOID( fire_worker ); \
	CFUN_VOID( hire_worker, int sec, object *firm, double wage ); \
	CFUN_VOID( move_worker, object *vint, bool vint_learn ); \
	CFUN_VOID( order_applications, int order, appLisT *appl ); \
	CFUN_VOID( order_offers, int order, woLisT *offers ); \
	CFUN_VOID( order_workers, int order, int obj ); \
	CFUN_VOID( quit_worker ); \
	CFUN_VOID( send_order, double nMach ); \
	CFUN_VOID( shuffle_offers, woLisT *offers ); \
	CFUN_VOID( shuffle_orders, buyLisT *orders ); \
	CFUN_VOID( update_debt1, double desired, double loan ); \
	CFUN_VOID( update_debt2, double desired, double loan );
