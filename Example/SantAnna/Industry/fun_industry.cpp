/******************************************************************************

	INDUSTRY LSD MODEL
	------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	VERSION: 2.1 - supporting advanced sensitivity analysis
	             - code streamlining

	This is the single code file for the model in LSD.

 ******************************************************************************/

#include "fun_head_fast.h"

#define PROD_INI 1							// initial notional productivity
#define PROD_SD 0.1							// initial productivity std. dev.
#define	STD_HHI false						// standardized HHI calculation


MODELBEGIN


// ######################### MODEL INITIALIZATION ###########################

//////////////////////////////////////////
EQUATION( "init" )
// Model initial setup variable

PARAMETER;									// turn into parameter (run once)
USE_NAN;									// allow "not a number" assignments

CYCLE( cur, "Market" )						// initialize each market
	VS( cur, "initMarket" );

RESULT( 1 )


//////////////////////////////////////////
EQUATION( "initMarket" )
// Market initial setup function

v[0] = COUNT( "Firm" );						// firm counter

if ( v[0] < 2 )								// just model firm exists?
{
	v[0] = max( 2, VL( "N", 1 ) );			// how many to create no less than 2
	ADDNOBJ( "Firm", v[0] - 1 );			// create objects
	WRITE( "InitShare", 1 / v[0] );			// entrant's initial share
}

WRITEL( "N", v[0], 0 );						// all incumbents (by definition)
WRITEL( "aMax", PROD_INI, 0 );				// initial max productivity is fixed

if ( V( "EntrReg" ) > 1 )					// entry regime is fixed # of firms?
	WRITE( "EntrReg", v[0] );				// set number of firms

CYCLE( cur, "Firm" )						// apply to all firms
	VS( cur, "_initFirm" );

RESULT( 1 )


//////////////////////////////////////////
EQUATION( "_initFirm" )
// Firm initial setup function

// if Mk I and no free entry, add noise in initial prod. to avoid equilibrium
if ( V( "MktReg" ) == 1 && V( "EntrReg" ) != 1 )
	WRITELL( "_a", norm( PROD_INI, PROD_SD ), T - 1, 1 );// noisy initial prod.
else
	WRITELL( "_a", PROD_INI, T - 1, 1 );	// fixed initial productivity

WRITELL( "_incumbent", T > 1 ? 0 : 1, T - 1, 1 );// all incumbents on start
WRITELL( "_s", V( "InitShare" ), T - 1, 1 );// initial market share (equal)
WRITELL( "_aEnd", PROD_INI, T - 1, 1 );		// define initial value in t=0
RECALC( "_aEnd" );							// and force recalculation
RECALC( "_aNorm" );							// in current t step when T > 1
RECALC( "_growth" );
RECALC( "_aGrowth" );

RESULT( 1 )


//////////////////////////////////////////
EQUATION( "_exitFirm" )
// Firm exit function

WRITE( "_incumbent", NAN );
WRITE( "_age", NAN );						// not consider in statistics
WRITE( "_s", NAN );
WRITE( "_a", NAN );
WRITE( "_aEnd", NAN );
WRITE( "_aNorm", NAN );
WRITE( "_growth", NAN );
WRITE( "_aGrowth", NAN );

DELETE( THIS );								// suicide

RESULT( 1 )


// ######################### COMPETITION DYNAMICS ###########################

//////////////////////////////////////////
EQUATION( "_s" )
// Replicator equation: firm current market share

// eq. 2
v[0] = VL( "_s", 1 ) * ( 1 + V( "A" ) * ( V( "_a" ) / V( "aAvg1" ) - 1 ) );

if ( v[0] <= 0 )							// only positive market shares
	v[0] = V( "sMin" );						// this code is not normally used

RESULT( v[0] )


// ########################## LEARNING DYNAMICS ############################

//////////////////////////////////////////
EQUATION( "_a" )
// Learning equation: firm current productivity
RESULT( VL( "_a", 1 ) * ( 1 + V( "_theta" ) ) )// eq. 1 and eq. 8


//////////////////////////////////////////
EQUATION( "_theta" )
// Firm productivity shock

if ( ! V( "_incumbent" ) ) 					// get right theta to update prod.
	v[0] = 0;								// entrant already learned in t
else
	switch ( ( int ) V( "MktReg" ) )		// incumbent learn according regime
	{
		case 1:								// Schumpeter Mark I regime
		  	v[0] = 0;						// no learning for incumbent (eq. 5)
 			break;

		case 2:								// intermediate regime (eq. 7)
			v[0] = V( "InnoShock" );		// learning just from shock
			break;

		case 3:
			// learning from shock with Matthew effect scaling (eq. 9)
			v[0] = V( "InnoShock" ) *
				   pow( ( VL( "_a", 1 ) / V( "aAvg2" ) ), V( "gamma" ) );
	}

RESULT( max( v[0], 0 ) )					// discard negative (behavioral)


//////////////////////////////////////////
EQUATION( "InnoShock" )
// Innovation shock function drawing from the appropriated distribution function

switch ( ( int ) V( "InnoProf" ) )			// pick the right distribution
{
	case 1:									// Uniform
		v[0] = V( "uniformInnoDraw" );
		break;

	case 2:									// Poisson
		v[0] = V( "poissonInnoDraw" );
		break;

	case 3:									// Gaussian (normal)
		v[0] = V( "normalInnoDraw" );
		break;

	case 4:									// Lognormal
		v[0] = V( "lognormInnoDraw" );
		break;

	case 5:									// (asymmetric) Laplace
		v[0] = V( "laplaceInnoDraw" );
		break;

	case 6:									// Beta
		v[0] = V( "betaInnoDraw" );
}

RESULT( v[0] )


// ################ ALTERNATIVE SHOCK DISTRIBUTION FUNCTIONS ################

//////////////////////////////////////////
EQUATION( "uniformInnoDraw" )
// Draw random number from (truncated) uniform distribution

v[0] = V( "UniformMin" );
v[1] = V( "UniformMax" );
v[2] = V( "MuMax" );						// maximum draw value (if >0)

v[3] = uniform( v[0], v[1] );				// uniform draw

if ( v[2] > 0 && v[2] > v[0] && v[2] < v[1] )// if upper truncation is necessary
{
	v[4] = unifcdf( v[0], v[1], v[2] ); 	// cdf up to upper bound

	v[3] = min( v[3], v[2] );				// truncate to the upper bound
	v[3] /= v[4];							// rescale draw
}

RESULT( v[3] )


//////////////////////////////////////////
EQUATION( "poissonInnoDraw" )
// Draw random number from (truncated) Poisson distribution (mean Mu)

v[0] = V( "Mu" );							// innovation expected value
v[1] = V( "MuMax" );						// maximum draw value (if >0)

v[2] = poisson( v[0] );						// poisson draw

if ( v[1] > 0 && v[1] > v[0] )			 	// if upper truncation is necessary
{
	v[3] = poissoncdf( v[0], v[1] );		// cdf up to upper bound

	v[2] = min( v[2], v[1] );				// truncate to the upper bound
	v[2] /= v[3];					 		// rescale draw
}

RESULT( v[2] )


//////////////////////////////////////////
EQUATION( "normalInnoDraw" )
// Draw random number from (truncated) Gaussian distribution (mean Mu)

v[0] = V( "Mu" );							// innovation expected value
v[1] = V( "NormalSD" );						// standard deviation
v[2] = V( "MuMax" );						// maximum draw value (if >0)

v[3] = norm( v[0], v[1] * v[0] );			// normal draw

if ( v[2] > 0 && v[2] > v[0] )			 	// if upper truncation is necessary
{
	v[4] = normcdf( v[0], v[1], v[2] );		// cdf up to upper bound

	v[3] = min( v[3], v[2] );				// truncate to the upper bound
	v[3] /= v[4];					 		// rescale draw
}

RESULT( v[3] )


//////////////////////////////////////////
EQUATION( "lognormInnoDraw" )
// Draw random number from (truncated) lognormal distribution

v[0] = V( "LognormLoc" );					// location parameter
v[1] = V( "LognormScale" );					// scale parameter
v[2] = V( "MuMax" );						// maximum draw value (if >0)

v[3] = lnorm( v[0], v[1] );					// lognormal draw

if ( v[2] > 0 && v[2] > v[0] )			 	// if upper truncation is necessary
{
	v[4] = lnormcdf( v[0], v[1], v[2] );	// cdf up to upper bound

	v[3] = min( v[3], v[2] );				// truncate to the upper bound
	v[3] /= v[4];							// rescale draw
}

RESULT( v[3] )


//////////////////////////////////////////
EQUATION( "laplaceInnoDraw" )
// Draw random from (truncated) asymmetric Laplace distribution (median Mu)

v[0] = V( "Mu" );							// innovation expected value
v[1] = V( "LaplaceAlpha1" );
v[2] = V( "LaplaceAlpha2" );
v[3] = V( "MuMax" );						// maximum draw value (if >0)

v[4] = alapl( v[0], v[1], v[2] ); 			// asymmetric laplace draw

if ( v[3] > 0 && v[3] > v[0] )				// if upper truncation is necessary
{
	v[5] = alaplcdf( v[0], v[1], v[2], v[3] );	// cdf up to upper bound

	v[4] = min( v[4], v[3] );				// truncate to the upper bound
	v[4] /= v[5];					 		// rescale draw
}

RESULT ( v[4] )


//////////////////////////////////////////
EQUATION( "betaInnoDraw" )
// Draw random number from (truncated) beta distribution

v[0] = V( "BetaAlpha" );					// get appropriate alfa and beta
v[1] = V( "BetaBeta" );
v[2] = V( "BetaMin" );						// inferior support to beta distr.
v[3] = V( "BetaMax" );						// superior support to beta distr.
v[4] = V( "MuMax" );						// maximum draw value (if >0)

v[5] = beta( v[0], v[1] );					// draw from Beta(alpha,beta)
v[5] = v[2] + v[5] * ( v[3] - v[2] );		// rescale support (betaMin,betaMax)

if ( v[4] > 0 && v[4] > v[2] && v[4] < v[3] )// if upper truncation is necessary
{
	v[6] = ( v[4] - v[2] ) / ( v[3] - v[2] ); // descale upper bound
	v[7] = betacdf( v[0], v[1], v[6] );		// cdf up to descaled upper bound

	v[5] = min( v[5], v[4] );				// truncate to the upper bound
	v[5] /= v[7];							// rescale draw
}

RESULT( v[5] )


// ####################### MARKET EXIT-ENTRY DYNAMICS ######################

//////////////////////////////////////////
EQUATION( "N" )
// Total number of incumbents, exits incumbents with too small market share

V( "growthAvg" );							// make calculation before exits

v[0] = 0;									// count staying firms
CYCLE_SAFE( cur, "Firm" )					// scan all firms
	if ( VS( cur, "_incumbent" ) )			// only if it's an incumbent
	{
  		if ( VS( cur, "_s" ) <= V( "sMin" ) && // eq. 3
			 COUNT_CND( "Firm", "_incumbent", "==", 1 ) > 1 ) // keep last firm
			VS( cur, "_exitFirm" );			// properly destroy firm object
		else
			v[0]++;							// one more staying incumbent
	}

RESULT( v[0] )


//////////////////////////////////////////
EQUATION( "E" )
// Draws the number of entrants in the period and creates them

v[1] = V( "N" );							// process exits this period
i = V( "EntrReg" );							// entry regime

switch ( i )
{
	case 0:									// no entry
		j = 0;
		break;

	case 1:									// prop. on # of incumbents (eq. 4)
		j = uniform( V( "UniformMinEntr" ), V( "UniformMaxEntr" ) ) * v[1];
		break;

	default:								// fixed number of firms
		j = max( i - v[1], 0 );
}

for ( k = 1; k <= j ; k++ )					// do one entrant at a time
{
	switch ( ( int ) V( "ArrivReg" ) )
	{
		case 0:
			v[1] = 1 + V( "InnoShock" );	// draw a theta for entrant (eq. 7)
			v[2] = V( "aAvg2" );			// average productivity as reference
			break;

		case 1:
			v[1] = 1 + ( 1 + V( "EntrLead" ) ) * ( V( "BetaMinEntr" ) +
				   beta( V( "BetaAlphaEntr" ), V( "BetaBetaEntr" ) ) *
				   ( V( "BetaMaxEntr" ) - V( "BetaMinEntr" ) ) );
			v[2] = VL( "aMax", 1 );			// uses top productivity (frontier)
			break;

		case 2:
			v[1] = 1 + V( "BetaMinEntr" ) +
				   beta( V( "BetaAlphaEntr" ), V( "BetaBetaEntr" ) ) *
				   ( V( "BetaMaxEntr" ) - V( "BetaMinEntr" ) );// eq. 7
			v[2] = VL( "aMax", 1 );			// uses top productivity (frontier)
	}

	cur = ADDOBJ( "Firm" );					// create new "Firm" in market
	VS( cur, "_initFirm" );					// initialize entrant

	WRITES( cur, "_theta", v[1] );			// initial shock
	WRITES( cur, "_a", v[1] * v[2] );		// set initial productivity (eq. 6)

	WRITELLS( cur, "_aEnd", 0, T, 1 );		// zero previous counters
}

V( "adjustShares" );						// adjust shares to add up to 1

RESULT( j )


//////////////////////////////////////////
EQUATION( "adjustShares")
// Adjust incumbents shares proportionally after entries and exits

v[0] = v[1] = 0;							// entrant/incumbent share accums.
CYCLE( cur, "Firm" )						// scans incumbents/entrants in t
	if ( VS( cur, "_incumbent" ) )			// if it's an incumbent
		v[1] += VS( cur, "_s" );			// add it to the right accumulator
	else
		v[0] += VS( cur, "_s" );

if ( v[0] + v[1] - 1 > v[1] ) 				// if impossible incumbent compensate
{
	v[2] = 1 / ( v[0] + v[1] );				// all firms adjustment factor

	CYCLE( cur, "Firm" )					// scan again to adjust
		MULTS( cur, "_s", v[2] );			// adjusts share
}
else
{
	v[2] = 1 - ( v[0] + v[1] - 1 ) / v[1];	// incumbent adjust factor

	CYCLE( cur, "Firm" )					// scan incumbents again to adjust
		if ( VS( cur, "_incumbent" ) )		// only do for incumbents
			MULTS( cur, "_s", v[2] );		// adjusts share
}

RESULT( v[2] )


// ####################### FIRM ATTRIBUTE CALCULATION #######################

//////////////////////////////////////////
EQUATION( "_incumbent" )
// Incumbent identification
RESULT( 1 )


//////////////////////////////////////////
EQUATION( "_age" )
// Updates the age of the firm
RESULT( CURRENT + 1 )


//////////////////////////////////////////
EQUATION( "_growth" )
// Growth rate of the market share of the firm (log)

v[0] = log( V( "_s" ) ) - log( VL( "_s", 1 ) );

if ( isinf( v[0] ) )						// just entered market
	v[0] = NAN;								// growth is not a number

RESULT( v[0] )


//////////////////////////////////////////
EQUATION( "_aEnd" )
// Productivity after entry/exit (should not be used out of here)
/*
This is needed because regular "_a" is recalculated in the beginning of
the time step, before exit and entry happen, so the saved values of
"_a" are inconsistent for cross-section analysis.
*/
V( "E" );									// ensure entry/exit already happened
RESULT( V( "_a" ) )


//////////////////////////////////////////
EQUATION( "_aNorm" )
// Normalized productivity after entry/exit (should not be used out of here)
RESULT( V( "_aEnd" ) / V( "aAvg" ) )


//////////////////////////////////////////
EQUATION( "_aGrowth" )
// Growth rate of the (stable) productivity of the firm

v[0] = V( "_aEnd" ) / VL( "_aEnd", 1 ) - 1;

if ( isinf( v[0] ) )						// just entered market
	v[0] = NAN;								// growth is not a number

RESULT( v[0] )


// ###################### MARKET AGGREGATES CALCULATION ####################

//////////////////////////////////////////
EQUATION( "aAvg1" )
// Average Productivity weighted with the value of shares
// Calculated with the productivity values before entry/exit ("_a")

v[0] = 0;
CYCLE( cur, "Firm" )
	v[0] += VS( cur, "_a" ) * VLS( cur, "_s", 1 );

RESULT( v[0] )


//////////////////////////////////////////
EQUATION( "aAvg2" )
// Average Productivity weighted by past shares and productivities
RESULT( WHTAVEL( "_a", "_s", 1 ) )


//////////////////////////////////////////
EQUATION( "aAvg" )
// Average Productivity weighted by current shares and productivities
// do not consider NaNs
RESULT( WHTAVE_CND( "_aEnd", "_s", "_aEnd", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "aMax" )
// Technological frontier (maximum productivity) (don't consider NaNs)
RESULT( MAX_CND( "_aEnd", "_aEnd", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "ageAvg" )
// Firms' average age (don't consider NaNs)
RESULT( AVE_CND( "_age", "_age", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "ageMax" )
// Firms' maximum age (don't consider NaNs)
RESULT( MAX_CND( "_age", "_age", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "growthAvg" )
// Firms' average market share growth (don't consider NaNs)
RESULT( AVE_CND( "_growth", "_growth", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "growthSD" )
// Firms' market share growth standard deviation (don't consider NaNs)
RESULT( SD_CND( "_growth", "_growth", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "growthMax" )
// Firms' maximum market share growth (don't consider NaNs)
RESULT( MAX_CND( "_growth", "_growth", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "aGrowthAvg" )
// Firms' average productivity growth (don't consider NaNs)
RESULT( AVE_CND( "_aGrowth", "_aGrowth", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "aGrowthSD" )
// Firms' productivity growth standard deviation (don't consider NaNs)
RESULT( SD_CND( "_aGrowth", "_aGrowth", "NNAN", 0 ) )


//////////////////////////////////////////
EQUATION( "aGrowthMax" )
// Firms' maximum productivity growth (don't consider NaNs)
RESULT( MAX_CND( "_aGrowth", "_aGrowth", "NNAN", 0 ) )


// ###################### OTHER STATISTICS GENERATION ######################

//////////////////////////////////////////
EQUATION( "HHI" )
// Herfindahl-Hirschman index

v[1] = V( "N" ) + V( "E" );					// ensure entry/exit already happened

v[0] = WHTAVE_CND( "_s", "_s", "_s", "NNAN", 0 );// squared share sum (no NaNs)

// Standardize HHI (correct for the number of firms), if appropriate
if ( STD_HHI )
{
	if ( v[1] > 1 )
		v[0] = ( v[0] - 1 / v[1] ) / ( 1 - 1 / v[1] );// normalization
	else
		if ( v[1] == 1 )
			v[0] = 1;
		else
			v[0] = 0;
}

RESULT( max( 0, min( v[0], 1 ) ) )			// handle abnormal cases


//////////////////////////////////////////
EQUATION( "dS" )
// Hymer-Pashigian index: sum of the delta market shares index

V( "E" );									// ensure entry/exit already happened

v[0] = 0;									// d share accumulator
CYCLE( cur, "Firm" )						// all firms in the market
{
	v[1] = VS( cur, "_s" );					// market share in t
	v[2] = VLS( cur, "_s", 1 );				// market share in t-1

	if ( ! isnan( v[1] ) && ! isnan( v[2] ) )// do not consider NaNs
		v[0] += abs( v[1] - v[2] );			// compute sum
}

RESULT( max( 0, min( v[0], 2 ) ) )			// handle abnormal cases


MODELEND


void close_sim(void)
{

}
