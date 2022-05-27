/******************************************************************************

	CLASS AND MACRO DEFINITIONS
	---------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

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

struct countryE
{
	// static global pointers to speed-up the access to individual containers
	object *finSec, *capSec, *conSec, *labSup, *macSta, *secSta, *labSta;

	// country speed-up vectors & maps
	objVecT firm2ptr;							// pointers to firms in sector 2
	firmMapT firm2map;							// ID to pointer map for sector 2
};


/*======================= INITIAL NOTIONAL DEFINITIONS =======================*/

#define INIPROD		1					// initial notional machine productivity
#define INIWAGE		1					// initial notional wage


/*========================= HOOK-RELATED DEFINITIONS =========================*/

// number of dynamic hooks per object type
#define FIRM2HK		2					// Firm2

// dynamic hook name to number
#define SUPPL		0					// from Firm2 to Broch (in Firm2)
#define TOPVINT		1					// from Firm2 to Vint (in Firm2)


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
