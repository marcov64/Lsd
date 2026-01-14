/******************************************************************************

	CLASS AND MACRO DEFINITIONS
	---------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	C++ class and preprocessor macro definitions used in the K+S LSD model
	are coded below.

 ******************************************************************************/

// K+S random engine (!= LSD)
std::mt19937_64 random_engine;


/*============================= GENERAL CLASSES ==============================*/

struct firmRank									// element of pecking order rank
{
	double NWtoS;								// net-wealth-to-sales ratio
	lsd::object *firm;							// pointer to firm
};

typedef std::pair < int, lsd::object * > firmPairT;// firm-to-object pair templ.
typedef std::map < int, lsd::object * > firmMapT;// firm-to-object map template
typedef std::map < std::string, double > strMapT;// string-to-double map templ.
typedef std::set < lsd::object * > firmSeT;		// firm-set template
typedef std::list < firmRank > firmLisT;		// ranked-firms list template
typedef std::vector < bool > boolVecT;			// vector of booleans template
typedef std::vector < double > dblVecT;			// vector of doubles template
typedef std::vector < firmPairT > firmPairVecT;	// vector of firm pairs
typedef std::vector < firmRank > firmVecT;		// vector of firm ranks template
typedef std::vector < int > intVecT;			// vector of integers template
typedef std::vector < lsd::object * > objVecT;	// vector of objects template


/*======================== COUNTRY EXTENSION CLASS ===========================*/

struct countryE
{
	// static global pointers to speed-up the access to individual containers
	lsd::object *finSec, *capSec, *conSec, *labSup, *macSta, *secSta, *labSta;

	// country speed-up vectors & maps
	dblVecT bankWgtd;							// m. s. cum. weights in banking
	objVecT bankPtr;							// pointers to banks
	objVecT firm2ptr;							// pointers to firms in sector 2
	firmMapT firm2map;							// ID to pointer map for sector 2
};


/*======================= INITIAL NOTIONAL DEFINITIONS =======================*/

#define INIPROD		1					// initial notional machine productivity
#define INIWAGE		1					// initial notional wage


/*========================= HOOK-RELATED DEFINITIONS =========================*/

// number of dynamic hooks per object type
#define FIRM1HK		2				// Firm1
#define FIRM2HK		4				// Firm2

// dynamic hook name to number
#define BANK		0				// from Firm1/Firm2 to Bank
#define BCLIENT		1				// from Firm1/Firm2 to Cli1/Cli2 (in Bank)
#define SUPPL		2				// from Firm2 to Broch (in Firm2)
#define TOPVINT		3				// from Firm2 to Vint (in Firm2)


/*======================= OBJECT-LOCATION DEFINITIONS ========================*/

// pointers to speed-up the access to individual market containers by caller
// equation levels
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

/*=================== FORWARD DECLARATION OF C++ FUNCTIONS ===================*/

#define EQ_USER_CFUNS \
	CFUN_DBL( cash_flow, double profit, double tax ); \
	CFUN_DBL( entry_firm1, int n, bool newInd ); \
	CFUN_DBL( entry_firm2, int n, bool newInd ); \
	CFUN_DBL( exit_firm ); \
	CFUN_DBL( invest, double desired ); \
	CFUN_DBL( mov_avg_bound, const char *var, double lim, double per, int lag ); \
	CFUN_DBL( scrap_vintage ); \
	CFUN_DBL( update_debt, double desired, double loan ); \
	CFUN_DBL( update_depo, double depo, bool incr ); \
	CFUN_OBJ( send_brochure, lsd::object *client ); \
	CFUN_OBJ( set_bank ); \
	CFUN_OBJ( set_supplier ); \
	CFUN_VOID( add_vintage, double nMach, bool newInd ); \
	CFUN_VOID( check_error, bool cond, const char* errMsg, int errCount, \
			   int *errCounter ); \
	CFUN_VOID( send_order, double nMach );
