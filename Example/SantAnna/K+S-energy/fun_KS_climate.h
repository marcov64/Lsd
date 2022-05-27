/******************************************************************************

	CLIMATE OBJECT EQUATIONS
	------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the climate CO2 cycle object in the
	K+S LSD model are coded below.

 ******************************************************************************/

#define GTCGTCO2	3.664						// conversion from GtC to GtCO2
#define PPMCO2GTC	2.13						// conversion from PPM CO2 to GtC
#define SEAFAREA	0.708						// sea fraction of global surface
#define LANDEQDEP	8.4							// equivalent land area thickness
#define SEAMDEP		100.0						// sea mixed layer depth
#define SEAD1DEP	300.0						// sea deep layer 1 depth
#define SEAD2DEP	300.0						// sea deep layer 2 depth
#define SEAD3DEP	1300.0						// sea deep layer 3 depth
#define SEAD4DEP	1800.0						// sea deep layer 4 depth

// thermal capacity (heat storage) of land and sea per m2 of surface per year
#define H2OHCAP		4186.0 * 1000 / ( 365 * 24 * 60 * 60 ) // 1m3 heat capacity
#define SURFHCAP	( H2OHCAP * ( SEAFAREA * SEAMDEP + \
								  ( 1 - SEAFAREA ) * LANDEQDEP ) )
#define SEAD1HCAP	( H2OHCAP * SEAFAREA * SEAD1DEP )
#define SEAD2HCAP	( H2OHCAP * SEAFAREA * SEAD2DEP )
#define SEAD3HCAP	( H2OHCAP * SEAFAREA * SEAD3DEP )
#define SEAD4HCAP	( H2OHCAP * SEAFAREA * SEAD4DEP )

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "CmRef" )
/*
Reference concentration of carbon in mixed sea layer
*/
RESULT( V( "Cm0pre" ) * SEAMDEP * ( 1 - V( "betaT2" ) * VL( "Tm", 1 ) ) )


EQUATION( "DeltaCam" )
/*
Flux of carbon from atmosphere to mixed sea layer
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( V( "CmRef" ) * pow( VL( "Ca", 1 ) / V( "Ca0pre" ), 1 / V( "xiA" ) ) -
		  VL( "Cm", 1 ) ) / ( V( "tMmix" ) * V( "tScale" ) ) )// chemical balance


EQUATION( "DeltaCba" )
/*
Flux of carbon from biomass to atmosphere
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( 1 - V( "phiA" ) ) * VL( "Cb", 1 ) / ( V( "tauBa" ) * V( "tScale" ) ) )


EQUATION( "DeltaCbs" )
/*
Flux of carbon from biomass to soils
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( V( "phiA" ) * VL( "Cb", 1 ) / ( V( "tauBa" ) * V( "tScale" ) ) )


EQUATION( "DeltaCd12" )
/*
Flux of carbon from deep sea layer 1 to layer 2
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( V( "kEddy" ) / V( "tScale" ) ) * ( ( VL( "Cd1", 1 ) / SEAD1DEP ) -
											 ( VL( "Cd2", 1 ) / SEAD2DEP ) ) /
		( ( SEAD1DEP + SEAD2DEP ) / 2 ) )


EQUATION( "DeltaCd23" )
/*
Flux of carbon from deep sea layer 2 to layer 3
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( V( "kEddy" ) / V( "tScale" ) ) * ( ( VL( "Cd2", 1 ) / SEAD2DEP ) -
											 ( VL( "Cd3", 1 ) / SEAD3DEP ) ) /
		( ( SEAD2DEP + SEAD3DEP ) / 2 ) )


EQUATION( "DeltaCd34" )
/*
Flux of carbon from deep sea layer 3 to layer 4
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( V( "kEddy" ) / V( "tScale" ) ) * ( ( VL( "Cd3", 1 ) / SEAD3DEP ) -
											 ( VL( "Cd4", 1 ) / SEAD4DEP ) ) /
		( ( SEAD3DEP + SEAD4DEP ) / 2 ) )


EQUATION( "DeltaCmd" )
/*
Flux of carbon from mixed to deep sea layer 1
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( V( "kEddy" ) / V( "tScale" ) ) * ( ( VL( "Cm", 1 ) / SEAMDEP ) -
											 ( VL( "Cd1", 1 ) / SEAD1DEP ) ) /
		( ( SEAMDEP + SEAD1DEP ) / 2 ) )


EQUATION( "DeltaCsa" )
/*
Flux of carbon from soils to atmosphere
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( VL( "Cs", 1 ) / ( V( "tauSa" ) * V( "tScale" ) ) )


EQUATION( "DeltaHd12" )
/*
Heat transfer from deep sea layer 1 to layer 2
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( VL( "Hd1", 1 ) / ( SEAD1HCAP * V( "tScale" ) ) -
		  VL( "Hd2", 1 ) / ( SEAD2HCAP * V( "tScale" ) ) ) * V( "hTr" ) )


EQUATION( "DeltaHd23" )
/*
Heat transfer from deep sea layer 2 to layer 3
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( VL( "Hd2", 1 ) / ( SEAD2HCAP * V( "tScale" ) ) -
		  VL( "Hd3", 1 ) / ( SEAD3HCAP * V( "tScale" ) ) ) * V( "hTr" ) )


EQUATION( "DeltaHd34" )
/*
Heat transfer from deep sea layer 3 to layer 4
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( VL( "Hd3", 1 ) / ( SEAD3HCAP * V( "tScale" ) ) -
		  VL( "Hd4", 1 ) / ( SEAD4HCAP * V( "tScale" ) ) ) * V( "hTr" ) )


EQUATION( "DeltaHmd" )
/*
Heat transfer from mixed to deep sea layer 1
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( ( VL( "Tm", 1 ) -
		  VL( "Hd1", 1 ) / ( SEAD1HCAP * V( "tScale" ) ) ) * V( "hTr" ) )


EQUATION( "EmCa" )
/*
Carbon emissions calibrated to reference period
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

v[1] = V( "EmCa0" ) / V( "tScale" );			// initial calibration value
v[2] = V( "EmC" );								// effective emissions
v[3] = V( "EmC0" );

if ( v[1] > 0 && v[3] > 0 )						// only scale if calibration on
	v[0] = v[2] * v[1] / v[3];
else
	v[0] = v[2];

if ( v[1] < 0 )									// use only calibrated growth
	// trend based on C-ROADS 2000-2100 baseline forecast line fit (R2=0.9941)
	v[0] = ( 7.4995 + 0.1243 / V( "tScale" ) * ( ( T - V( "tA0" ) ) ) ) /
		   V( "tScale" );

RESULT( v[0] )


EQUATION( "FC" )
/*
Feedback cooling of mixed sea layer and atmosphere (blackbody radiation)
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( VL( "Tm", 1 ) * V( "gammaA" ) * log( 2 ) / V( "alphaA" ) )


EQUATION( "Fco2" )
/*
Radiative forcing in the atmosphere from CO2
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

RESULT( V( "gammaA" ) * log( VL( "Ca", 1 ) / V( "Ca0pre" ) ) )


EQUATION( "NPP" )
/*
Net primary production (flux) of carbon from atmosphere to biomass
Considers the fertilization, diminishing returns and warming effects
*/

if ( T < V( "tA0" ) )							// compute after climate starts
	END_EQUATION( 0 );

v[1] = VL( "Ca", 1 ) / V( "Ca0pre" );			// atmo. carbon growth

RESULT( ( V( "NPP0pre" ) / V( "tScale" ) ) *
		( 1 + V( "betaC1" ) * log( v[1] ) ) *	// fertilization
		( 1 - V( "betaC2" ) * max( 0., v[1] / V( "betaC3" ) - 1 ) ) *// dim. ret.
		( 1 - V( "betaT1" ) * VL( "Tm", 1 ) ) )	// warming


EQUATION( "xiA" )
/*
Revelle (oceans buffer) factor
*/
RESULT( V( "xiA0" ) + V( "deltaA" ) * log( VL( "Ca", 1 ) / V( "Ca0pre" ) ) )


/*============================ KEY LSD FUNCTIONS =============================*/

EQUATION( "shockA" )
/*
Disaster generating function
*/

if ( VS( PARENT, "flagEnClim" ) == 0 || T < V( "tA0" ) )
	END_EQUATION( 0 );

v[1] = V( "a0" ) * ( 1 + log( VL( "Tm", 1 ) / V( "Tm0" ) ) );// a parameter
v[2] = V( "b0" ) * V( "sigma10y0" ) / VL( "sigma10y", 1 );// b parameter

// the beta parameters are for yearly shocks, so adjust draw probability
if ( v[1] <= 0 || ! isfinite( v[2] ) || RND > 1 / V( "tScale" ) )
	v[0] = 0;
else
	v[0] = beta( v[1], v[2] );					// draw again the size of shock

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "Ca" )
/*
Concentration of carbon in atmosphere
*/
RESULT( CURRENT + V( "EmCa" ) - V( "NPP" ) +	// man (+), biomass (-/+)
		V( "DeltaCba" ) + V( "DeltaCsa" ) -		// and soils (+) contributions
		V( "DeltaCam" )	)						// net sea contribution


EQUATION( "Cb" )
/*
Concentration of carbon in biomass
*/
RESULT( CURRENT + V( "NPP" ) - V( "DeltaCba" ) - V( "DeltaCbs" ) )


EQUATION( "Cd1" )
/*
Concentration of carbon in deep sea layer 1
*/
RESULT( CURRENT + V( "DeltaCmd" ) - V( "DeltaCd12" ) )


EQUATION( "Cd2" )
/*
Concentration of carbon in deep sea layer 2
*/
RESULT( CURRENT + V( "DeltaCd12" ) - V( "DeltaCd23" ) )


EQUATION( "Cd3" )
/*
Concentration of carbon in deep sea layer 3
*/
RESULT( CURRENT + V( "DeltaCd23" ) - V( "DeltaCd34" ) )


EQUATION( "Cd4" )
/*
Concentration of carbon in deep sea layer 4
*/
RESULT( CURRENT + V( "DeltaCd34" ) )


EQUATION( "Cm" )
/*
Effective concentration of carbon in mixed sea layer
*/
RESULT( CURRENT + V( "DeltaCam" ) - V( "DeltaCmd" )	)


EQUATION( "Cs" )
/*
Concentration of carbon in soils
*/
RESULT( CURRENT + V( "DeltaCbs" ) - V( "DeltaCsa" ) )


EQUATION( "EmC" )
/*
Emissions in GtC (uncalibrated)
Also updates once 'EmC0'
*/

v[0] = VS( PARENT, "Em" ) / ( GTCGTCO2 * 1e6 );

if ( T == V( "tA0" ) )							// starting climate variables?
	WRITE( "EmC0", v[0] );						// save reference period emissions

RESULT( v[0] )


EQUATION( "Hd1" )
/*
Heat in in deep sea layer 1
*/
RESULT( CURRENT + V( "DeltaHmd" ) - V( "DeltaHd12" ) )


EQUATION( "Hd2" )
/*
Heat in in deep sea layer 2
*/
RESULT( CURRENT + V( "DeltaHd12" ) - V( "DeltaHd23" ) )


EQUATION( "Hd3" )
/*
Heat in in deep sea layer 3
*/
RESULT( CURRENT + V( "DeltaHd23" ) - V( "DeltaHd34" ) )


EQUATION( "Hd4" )
/*
Heat in in deep sea layer 4
*/
RESULT( CURRENT + V( "DeltaHd34" ) )


EQUATION( "Hm" )
/*
Heat in mixed sea layer and atmosphere
*/
RESULT( CURRENT + V( "Fco2" ) -					// greenhouse effect less
		V( "FC" ) - V( "DeltaHmd" )	)			// feedback/lower layer cooling


EQUATION( "Td1" )
/*
Temperature change (from pre-industrial) of atmosphere at deep sea layer 1
*/
RESULT( V( "Hd1" ) / ( SEAD1HCAP * V( "tScale" ) ) )


EQUATION( "Td2" )
/*
Temperature change (from pre-industrial) of atmosphere at deep sea layer 2
*/
RESULT( V( "Hd2" ) / ( SEAD2HCAP * V( "tScale" ) ) )


EQUATION( "Td3" )
/*
Temperature change (from pre-industrial) of atmosphere at deep sea layer 3
*/
RESULT( V( "Hd3" ) / ( SEAD3HCAP * V( "tScale" ) ) )


EQUATION( "Td4" )
/*
Temperature change (from pre-industrial) of atmosphere at deep sea layer 4
*/
RESULT( V( "Hd4" ) / ( SEAD4HCAP * V( "tScale" ) ) )


EQUATION( "Tm" )
/*
Temperature change (from pre-industrial) at mixed sea layer
*/
RESULT( V( "Hm" ) / ( SURFHCAP * V( "tScale" ) ) )


EQUATION( "sigma10y" )
/*
Standard deviation of temperature anomaly in past 10 years
*/

v[1] = V( "tA0" );								// climate machine start period
v[2] = V( "tScale" );							// climate time scale

if ( T < v[1] + 2 )								// not enough data to compute?
	END_EQUATION( 0 );

h = min( T - v[1], 10 * V( "tScale" ) );		// usable years of data up to 10

for ( v[3] = v[4] = i = 0; i < h; ++i )
{
	v[3] += v[5] = VL( "Tm", i );				// sum temperature anomalies
	v[4] += pow( v[5], 2 );						// and the squares
}

RESULT( sqrt( ( h * v[4] - pow( v[3], 2 ) ) / ( h * ( h - 1 ) ) ) )// sample SD


/*========================= INITIALIZATION EQUATION ==========================*/

EQUATION( "initClimate" )
/*
Initialize the K+S climate object
It is run only once per country
*/

PARAMETER;										// execute only once

// prepare data required to set initial conditions
double Ca0 = V( "Ca0" );						// reference time atmos. carbon
double Ca0pre = V( "Ca0pre" );					// pre-industrial atmos. carbon
double Td0 = V( "Td0" );						// ref. time deep sea L4 temp.
double gammaA = V( "gammaA" );					// CO2 radiative forcing coeff.
double tScale = V( "tScale" );					// climate-year time steps
double tauBa = V( "tauBa" );					// carbon resid. time in biomass

// initial temperatures compatible with heat transfer equilibrium at ref. time
double Tm0 = gammaA * log( Ca0 / Ca0pre ) /		// mixed sea layer eq. temp.
			 ( 1 + gammaA * log( 2 ) / V( "alphaA" ) );
double Td10 = Tm0 - ( Tm0 - Td0 ) * ( 1 / SEAD1DEP ) /
			  ( 1 / SEAD1DEP + 1 / SEAD2DEP + 1 / SEAD3DEP + 1 / SEAD4DEP );
double Td20 = Td10 - ( Tm0 - Td0 ) * ( 1 / SEAD2DEP ) /
			  ( 1 / SEAD1DEP + 1 / SEAD2DEP + 1 / SEAD3DEP + 1 / SEAD4DEP );
double Td30 = Td20 - ( Tm0 - Td0 ) * ( 1 / SEAD3DEP ) /
			  ( 1 / SEAD1DEP + 1 / SEAD2DEP + 1 / SEAD3DEP + 1 / SEAD4DEP );
double Td40 = Td0;

// initial carbon stocks compatible with carbon flux equilibrium at ref. time
double Cm0 = V( "Cm0pre" ) * SEAMDEP * ( 1 - V( "betaT2" ) * Tm0 ) *
			 pow( Ca0 / Ca0pre,					// mixed sea layer
				  1 / ( V( "xiA0" ) + V( "deltaA" ) * log( Ca0 / Ca0pre ) ) );
double Cb0 = tauBa * V( "NPP0pre" ) *			// biomass
			 ( 1 + V( "betaC1" ) * log( Ca0 / Ca0pre ) ) *
			 ( 1 - V( "betaC2" ) * max( 0, Ca0 / Ca0pre / V( "betaC3" ) - 1 ) ) *
			 ( 1 - V( "betaT1" ) * Tm0 );
double Cs0 = V( "phiA" ) * V( "tauSa" ) * Cb0 / tauBa;// soils carbon
double Cd10 = Cm0 * SEAD1DEP / SEAMDEP;			// deep sea layer 1
double Cd20 = Cd10 * SEAD2DEP / SEAD1DEP;		// deep sea layer 2
double Cd30 = Cd20 * SEAD3DEP / SEAD2DEP;		// deep sea layer 3
double Cd40 = Cd30 * SEAD4DEP / SEAD3DEP;		// deep sea layer 4

// climate initial thermal capacities (heat stocks) at reference time (tA0)
double Hm0 = Tm0 * SURFHCAP * tScale;			// land and mixed sea layer
double Hd10 = Td10 * SEAD1HCAP * tScale;		// deep sea layer 1
double Hd20 = Td20 * SEAD2HCAP * tScale;		// deep sea layer 2
double Hd30 = Td30 * SEAD3HCAP * tScale;		// deep sea layer 4
double Hd40 = Td40 * SEAD4HCAP * tScale;		// deep sea layer 4

// initialize lagged variables depending on parameters
WRITEL( "Ca", Ca0, -1 );
WRITEL( "Cb", Cb0, -1 );
WRITEL( "Cd1", Cd10, -1 );
WRITEL( "Cd2", Cd20, -1 );
WRITEL( "Cd3", Cd30, -1 );
WRITEL( "Cd4", Cd40, -1 );
WRITEL( "Cm", Cm0, -1 );
WRITEL( "Cs", Cs0, -1 );
WRITEL( "Hd1", Hd10, -1 );
WRITEL( "Hd2", Hd20, -1 );
WRITEL( "Hd3", Hd30, -1 );
WRITEL( "Hd4", Hd40, -1 );
WRITEL( "Hm", Hm0, -1 );
WRITEL( "Tm", Tm0, -1 );
WRITEL( "Tm0", Tm0, -1 );

RESULT( 1 )


/*============================= DUMMY EQUATIONS ==============================*/
