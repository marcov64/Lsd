/******************************************************************************

	CLASS AND MACRO DEFINITIONS
	---------------------------

	C++ class and preprocessor macro definitions used in the K+S LSD model 
	are coded below.
 
 ******************************************************************************/

/*============================= GENERAL CLASSES ==============================*/

struct firmRank									// element of pecking order rank
{
	double NWtoS;								// net-wealth-to-sales ratio
	object *firm;								// pointer to firm
};

typedef pair < int, object * > firmPairT;		// firm-to-object pair template
typedef map < int, object * > firmMapT;			// firm-to-object map template
typedef set < object * > firmSeT;				// firm-set template
typedef list < firmRank > firmLisT;				// ranked-firms list template
typedef vector < double > dblVecT;				// vector of doubles template
typedef vector < object * > objVecT;			// vector of objects template


/*======================== COUNTRY EXTENSION CLASS ===========================*/

struct country
{
	// static global pointers to speed-up the access to individual containers
	object *finSec, *capSec, *conSec, *labSup, *macSta, *secSta, *labSta;

	// country speed-up vectors & maps
	dblVecT bankWgtd;							// m. s. cum. weights in banking
	objVecT bankPtr;							// pointers to banks
	objVecT firm2ptr;							// pointers to firms in sector 2
	firmMapT firm2map;							// ID to pointer map for sector 2	
};


/*========================= HOOK-RELATED DEFINITIONS =========================*/

// number of dynamic hooks per object type
#define FIRM1HK 2			// Firm1
#define FIRM2HK 4			// Firm2

// dynamic hook name to number
#define BANK 0				// from Firm1/Firm2 to Bank
#define BCLIENT 1			// from Firm1/Firm2 to Cli1/Cli2 (in Bank)
#define SUPPL 2				// from Firm2 to Broch (in Firm2)
#define TOPVINT 3			// from Firm2 to Vint (in Firm2)


/*======================= OBJECT-LOCATION DEFINITIONS ========================*/

// pointers to speed-up the access to individual market containers by caller 
// equation levels
#define CAPSECL0 V_EXT( country, capSec )
#define CAPSECL1 V_EXTS( PARENT, country, capSec )
#define CAPSECL2 V_EXTS( GRANDPARENT, country, capSec )
#define CONSECL0 V_EXT( country, conSec )
#define CONSECL1 V_EXTS( PARENT, country, conSec )
#define CONSECL2 V_EXTS( GRANDPARENT, country, conSec )
#define LABSUPL0 V_EXT( country, labSup )
#define LABSUPL1 V_EXTS( PARENT, country, labSup )
#define LABSUPL2 V_EXTS( GRANDPARENT, country, labSup )
#define LABSUPL3 V_EXTS( GRANDPARENT->up, country, labSup )
#define FINSECL0 V_EXT( country, finSec )
#define FINSECL1 V_EXTS( PARENT, country, finSec )
#define FINSECL2 V_EXTS( GRANDPARENT, country, finSec )
#define MACSTAL0 V_EXT( country, macSta )
#define MACSTAL1 V_EXTS( PARENT, country, macSta )
#define MACSTAL2 V_EXTS( GRANDPARENT, country, macSta )
#define SECSTAL0 V_EXT( country, secSta )
#define SECSTAL1 V_EXTS( PARENT, country, secSta )
#define SECSTAL2 V_EXTS( GRANDPARENT, country, secSta )
#define LABSTAL0 V_EXT( country, labSta )
#define LABSTAL1 V_EXTS( PARENT, country, labSta )
#define LABSTAL2 V_EXTS( GRANDPARENT, country, labSta )


/*============================== SUPPORT MACROS ==============================*/

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
