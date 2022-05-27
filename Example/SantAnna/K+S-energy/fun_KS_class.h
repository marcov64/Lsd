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
	object *finSec, *capSec, *conSec, *labSup, *climat, *eneSec,
		   *macSta, *secSta, *labSta, *eneSta;

	// country speed-up vectors & maps
	dblVecT bankWgtd;							// m. s. cum. weights in banking
	objVecT bankPtr;							// pointers to banks
	objVecT firm2ptr;							// pointers to firms
	objVecT firmEptr;
	firmMapT firm2map;							// firm ID to pointer map
	firmMapT firmEmap;
};


/*======================= INITIAL NOTIONAL DEFINITIONS =======================*/

#define INIPROD		1					// initial notional machine productivity
#define INIEEFF		10					// initial notional energy efficiency
#define INIEFRI		40					// initial notional envir. friendliness
#define INIWAGE		1					// initial notional wage


/*========================= HOOK-RELATED DEFINITIONS =========================*/

// number of dynamic hooks per object type
#define FIRM1HK		2				// Firm1
#define FIRM2HK		4				// Firm2
#define FIRMEHK		4				// FirmE

// dynamic hook name to number
#define BANK		0				// from Firm1/Firm2/FirmE to Bank
#define BCLIENT		1				// from Firm1/Firm2/FirmE to CliX (in Bank)
#define SUPPL		2				// from Firm2/FirmE to Broch/BrE (in FirmX)
#define TOPVINT		3				// from Firm2/FirmE to Vint/Green (in FirmX)


/*======================= OBJECT-LOCATION DEFINITIONS ========================*/

// pointers to speed-up the access to individual market containers by caller
// equation levels
#define CAPSECL0 V_EXT( countryE, capSec )
#define CAPSECL1 V_EXTS( PARENT, countryE, capSec )
#define CAPSECL2 V_EXTS( GRANDPARENT, countryE, capSec )
#define CONSECL0 V_EXT( countryE, conSec )
#define CONSECL1 V_EXTS( PARENT, countryE, conSec )
#define CONSECL2 V_EXTS( GRANDPARENT, countryE, conSec )
#define ENESECL0 V_EXT( countryE, eneSec )
#define ENESECL1 V_EXTS( PARENT, countryE, eneSec )
#define ENESECL2 V_EXTS( GRANDPARENT, countryE, eneSec )
#define ENESECL3 V_EXTS( PARENTS( GRANDPARENT ), countryE, eneSec )
#define LABSUPL0 V_EXT( countryE, labSup )
#define LABSUPL1 V_EXTS( PARENT, countryE, labSup )
#define LABSUPL2 V_EXTS( GRANDPARENT, countryE, labSup )
#define LABSUPL3 V_EXTS( PARENTS( GRANDPARENT ), countryE, labSup )
#define FINSECL0 V_EXT( countryE, finSec )
#define FINSECL1 V_EXTS( PARENT, countryE, finSec )
#define FINSECL2 V_EXTS( GRANDPARENT, countryE, finSec )
#define CLIMATL0 V_EXT( countryE, climat )
#define CLIMATL1 V_EXTS( PARENT, countryE, climat )
#define CLIMATL2 V_EXTS( GRANDPARENT, countryE, climat )
#define CLIMATL3 V_EXTS( PARENTS( GRANDPARENT ), countryE, climat )
#define MACSTAL0 V_EXT( countryE, macSta )
#define MACSTAL1 V_EXTS( PARENT, countryE, macSta )
#define MACSTAL2 V_EXTS( GRANDPARENT, countryE, macSta )
#define SECSTAL0 V_EXT( countryE, secSta )
#define SECSTAL1 V_EXTS( PARENT, countryE, secSta )
#define SECSTAL2 V_EXTS( GRANDPARENT, countryE, secSta )
#define ENESTAL0 V_EXT( countryE, eneSta )
#define ENESTAL1 V_EXTS( PARENT, countryE, eneSta )
#define ENESTAL2 V_EXTS( GRANDPARENT, countryE, eneSta )
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

// macros to pack/unpack firm ID in multiple-industries
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
