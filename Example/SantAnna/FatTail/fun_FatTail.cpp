#include "fun_head_fast.h"

bool	stdHHI = false;						// standardized HHI calculation flag
bool	reportNegMS = true;					// negative market share report flag
bool	reportAltMS = true;					// alternative market share report flag


MODELBEGIN


// ########################## INITIALIZATION ############################ //

EQUATION("init")
/*
Model initial setup
*/

CYCLE( cur, "Market" )							// initialize all markets
{
	v[0] = 0;									// firm counter
	CYCLES( cur, cur1, "Firm" )					// cycle all firms
		v[0]++;									// counting them

	if ( v[0] == 1 )							// just model firm exists
	{
		v[0] = floor ( VLS( cur, "N", 1 ) );	// check how many firms to create
		v[0] = fmax( 2, v[0] );					// no less than 2 firms
		ADDNOBJS( cur, "Firm", v[0] - 1 );		// create objects
		WRITES( cur, "InitShare", 1 / v[0] );	// set entrants initial share
	}

	WRITELS( cur, "N", v[0], 0 );				// all incumbents (by definition)
	WRITELS( cur, "totalFirms", v[0], 0 );		// set initial firm counts
	if( VS( cur, "EntrReg" ) > 1 )				// Entry regime is fixed number of firms?
		WRITES( cur, "EntrReg", v[0] );			// set number of firms

	WRITELS( cur, "aMax", 1, 0 );				// initial productivity is always 1

	CYCLES( cur, cur1, "Firm" )					// apply to all firms
	{	// if Mk I and no free entry, add noise in initial prod. to avoid equilibrium
		if( VS( cur, "MktReg" ) == 1 && VS( cur, "EntrReg" ) != 1 )
			WRITELS( cur1, "_a", norm( 1, 0.1 ), 0 );	// noisy productivity
		else
			WRITELS( cur1, "_a", 1, 0 );

		WRITELS( cur1, "_s", 1 / v[0], 0 );		// initial market share (equal)
		WRITELS( cur1, "_aEnd", 1, 0 );			// define initial value in t=0
		WRITELS( cur1, "_aNorm", 1, 0 );		// to force update in t=1
		WRITELS( cur1, "_incumbent", 1, 0 );
		WRITELS( cur1, "_age", 0, 0 );
		WRITELS( cur1, "_growth", 0, 0 );
		WRITELS( cur1, "_aGrowth", 0, 0 );
	}
}

use_nan = true;								// allow "not a number" assignments
fast = true;								// start in fast mode (less output)

PARAMETER;									// turn variable in parameter (run once)

RESULT( 1 )


// ################# PRODUCTIVITY & REPLICATOR DYNAMIC ################### //

EQUATION( "_a" )
/*
This equation specifies the dynamic of the productivity in the three regimes
*/

v[0] = VL( "_a", 1 );						// last period productivity
v[1] = V( "_theta");						// productivity shock

v[2] = v[0] * ( 1 + v[1] );					// inovation equation (eq. 1 and eq. 8)
  
RESULT( v[2] )


EQUATION( "_s" )
/*
Level of shares, computed as the past shares plus a share of the
difference of productivity in respect of the average productivity
*/

v[0] = VL( "_s", 1 );						// previous market share
v[1] = V( "A" );							// replicator selectivity
v[2] = V( "_a" );							// current productivity
v[3] = V( "aAvg1" );						// average productivity ( a(t), s(t-1) )
v[4] = V( "sMin" );							// minimum market share alowed

v[5] = v[0] * ( 1 + v[1] * ( v[2] - v[3] ) / v[3] );	// replicator equation (eq. 2)
if ( v[5] <= 0 )							// only positive market shares
{
	v[5] = v[4];							// keep the minimum market share

	if( reportNegMS && v[5] < 0 )			// do it only once, if needed
	{
		plog( "\nWarning: firm reached negative market share." );
		reportNegMS = false;
	}
}

RESULT( v[5] )


// ##################### PRODUCTIVITY SHOCKS (THETA) ##################### //

EQUATION( "_theta" )
/*
Calculates the applicable productivity shock
*/

v[0] = V( "_incumbent" );					// is the firm an incumbent?
v[1] = V( "MktReg" );						// market regime
v[2] = V( "gamma" );						// Matthew effect intensity (0=no effect)

if ( ! v[0] ) 								// get the right theta to update prod
	v[3] = 0;								// not an incumbent: keep learning
else
	switch ( (int) v[1] )					// incumbent: learning according regime
	{
		case 1:								// Schumpeter Mark I regime
		  	v[3] = 0;						// no learning for incumbent (eq. 5)
 			break;
  
		case 2:								// intermediate regime
			v[3] = V( "InnoShock" );		// learning from asym. Laplace (eq. 7)
			break;
  
		case 3:
			v[4] = V( "InnoShock" );		// learning from beta distribution
			v[5] = VL( "_a", 1 );			// previous productivity
			v[6] = V( "aAvg2" );			// average productivity (a(t-1), s(t-1))
			v[3] = v[4] * pow( ( v[5] / v[6] ), v[2] );	// Matthew effect scaling (eq. 9)
		break;
	}

v[3] = max( v[3], 0);						// discard negative theta (behavioral)

RESULT( v[3] )


// ######################### MARKET EXIT DYNAMICS ######################## //

EQUATION( "killIncumbents" )
/*
Kill incumbents with too small market share
*/

V( "growthAvg" );							// make growth calculated before exits

v[1] = V( "sMin" );							// market share threshold to leave market
v[2] = V( "aThresh" );						// min productivity multiplier threshold
v[3] = V( "aAvg1" );						// current productivity ( a(t), s(t-1) )
v[4] = V( "OverlapExit" );					// overlap exiting incumbent data flag

v[5] = v[2] * v[3];							// minimum productivity to stay in market

v[0] = 0;
CYCLE( cur, "Firm" )						// count current incumbents
	if ( VS( cur, "_incumbent" ) )
		v[0]++;

v[6] = 0;									// count killed firms

CYCLE_SAFE( cur, "Firm" )					// scan all firms
{
	v[7] = VS( cur, "_s" );					// get firm's current share
	v[8] = VS( cur, "_a" );					// and current productivity
	v[9] = VS( cur, "_incumbent" );			// and if it's an incumbent

	if ( v[9] )								// only if it's an incumbent
  		if ( v[7] <= v[1] || v[8] < v[5] )	// market exit equation (eq. 3)
  			if ( v[6] < ( v[0] - 1 ) )		// avoid deleting the last firm
  			{
  				if ( ! v[4] )				// if no overlap
  				{							// mark object data as NaN for t
					WRITES( cur, "_incumbent", NAN );
					WRITES( cur, "_age", NAN );
					WRITES( cur, "_theta", NAN );
					WRITES( cur, "_s", NAN );
					WRITES( cur, "_a", NAN );
					WRITES( cur, "_aEnd", NAN );
					WRITES( cur, "_aNorm", NAN );
					WRITES( cur, "_growth", NAN );
					WRITES( cur, "_aGrowth", NAN );
				}
				else						// update variables before killing
				{
					VS( cur, "_incumbent" );
					VS( cur, "_age" );
					VS( cur, "_theta" );
					VS( cur, "_s" );
					VS( cur, "_a" );
					VS( cur, "_aEnd" );
					VS( cur, "_aNorm" );
					VS( cur, "_growth" );
					VS( cur, "_aGrowth" );
				}

	  			DELETE( cur );				// delete firm's object
  				v[6]++;						// one more killed
			}
}

RESULT( v[6] )


// ######################## MARKET ENTRY DYNAMICS ######################## //

EQUATION( "E" )
/*
Draws the number of entrants in the period.
Entry Process occurs in one of three scenarios: 
. no entry
. according to a proportion on the number of incumbent in the previous period
. fixed number of firms (may be one less for a while)
*/

v[0] = V( "uniformDrawEntr" );				// uniform draw function (for entry)
v[1] = V( "N" );							// current number of incumbents
v[2] = V( "EntrReg" );						// entry parameter (0=no entry,
											// 1=free entry, >1=constant fims)
v[3] = VL( "totalFirms", 1 );				// number of firms in previous period
v[4] = V( "killIncumbents" );				// process exits this period

switch ( (int) v[2] )
{
	case 0:
		v[5] = 0;							// no entrant
		break;

	case 1:
		v[5] = round( v[0] * v[1] );		// (integer) entry equation (eq. 4)
		break;

	default:
		v[5] = max( v[2] - ( v[3] - v[4] ), 0 );	// (positive) missing number of firms
		break;
}

RESULT( v[5] )


EQUATION( "createEntrants" )
/*
Create as many new entrant firm objects as indicated by "Entrants"
*/

v[0] = V( "E" );							// how many entrant objects to create
v[1] = V( "aAvg2" );						// avg  productivity ( a(t-1), s(t-1) )
v[2] = VL( "aMax", 1 );						// technological frontier (max prod.)
v[3] = V( "InitShare" );					// initial entrant's market share
v[4] = V( "ArrivReg" );						// arrival regime flag (0=stoch., 1=K+S)
v[5] = V( "EntrLead" ); 					// average entrant lead over frontier

for ( v[6]=1; v[6] <= v[0] ; v[6]++ )		// do one at a time
{
	switch ( (int) v[4] )
	{
		case 0:
			v[7] = 1 + V( "InnoShock" );	// draw a theta for entrant (eq. 7) 
			v[8] = v[1];					// average productivity as reference
			break;

		case 1:
			v[7] = 1 + ( 1 + v[5] ) * V( "betaInnoDrawEntr" );
 											// draw a (theta) based on K+S
			v[8] = v[2];					// uses top productivity (frontier)
			break;

		case 2:
			v[7] = 1 + V( "betaInnoDrawEntr" );	// draw a theta for entrant (eq. 7) 
			v[8] = v[2];					// uses top productivity (frontier)
			break;
	}

	cur = ADDOBJ( "Firm" );					// create a new "Firm" in current market
  
	WRITES( cur, "_age", 0 );				// set age to 0
	WRITES( cur, "_incumbent", 0 );			// set as not an incumbent
	WRITES( cur, "_theta", v[7] );			// initial shock
	WRITES( cur, "_a", v[7] * v[8] );		// set initial productivity (eq. 6)
	WRITES( cur, "_s", v[3] );				// set initial market share

	WRITELLS( cur, "_aEnd", 0, t, 1 );		// zero previous counters
	WRITES( cur, "_aNorm", 0 );
	WRITES( cur, "_growth", 0 );
	WRITES( cur, "_aGrowth", 0 );
	RECALCS( cur, "_aEnd" );				// and force recalculaton
	RECALCS( cur, "_aNorm" );				// in current t step
	RECALCS( cur, "_growth" );
	RECALCS( cur, "_aGrowth" );
}

RESULT( v[6] - 1 )


EQUATION( "adjustShares")
/*
Adjust incumbents shares proportionally after entries and exits (if possible).
If incumbents share is not large enough, adjust all shares proportionally.
*/

V( "killIncumbents" );						// ensure entry/exit already happened

v[0] = v[1] = 0;							// entrant & incumbent share accumulators

CYCLE( cur, "Firm" )						// scans incumbents and entrants in t
{
	v[2] = VS( cur, "_incumbent" );			// gets firm's incumbent (or not) status
	v[3] = VS( cur, "_s" );					// gets firm's share
  
	if ( v[2] )								// if it's an incumbent
		v[1] += v[3];						// add it to the right accumulator
	else
		v[0] += v[3];
}

if ( ( v[0] + v[1] - 1 ) > v[1] ) 			// if not possible incumbents compensate
{
	v[4] = 1 / ( v[0] + v[1] );				// calculates all firms adjustment factor

	CYCLE( cur, "Firm" )					// scan again to adjust
	{
		v[3] = VS( cur, "_s" );				// gets firm's share
		WRITES( cur, "_s", v[3] * v[4] );	// adjusts share
	}

	if( reportAltMS )
	{
		plog("\nWarning: alternative m. s. ajustment employed.");
		reportAltMS = false;
	}
}
else
{
	v[4] = 1 - ( v[0] + v[1] - 1 ) / v[1];	// calculates incumbents adjust factor

	CYCLE( cur, "Firm" )					// scan incumbents again to adjust
	{
		v[2] = VS( cur, "_incumbent" );		// gets firm's incumbent (or not) status
		v[3] = VS( cur, "_s" );				// gets firm's share
  
		if ( v[2] )							// only do for incumbents
			WRITES( cur, "_s", v[3] * v[4] );	// adjusts share
	}
}

RESULT( v[4] )


// ##################### FIRM ATTRIBUTES MAINTENANCE ##################### //

EQUATION( "_incumbent" )
/*
This equation determines who are the incumbents
*/

v[0] = VL( "_incumbent", 1 );
v[1] = V( "_age" );
v[2] = V( "AgeEntrant" );					// maximum age to be an entrant

if ( v[1] > v[2] )
	v[3] = true;							// flag incumbent if older than limit
else
	v[3] = v[0];							// else keep as it is
  
RESULT( v[3] )  


EQUATION( "_age" )	
/*
This equation updates the age of the firm
*/

v[0] = VL( "_age", 1 );
v[0] ++;

RESULT( v[0] )


EQUATION( "_growth" )	
/*
Calculates the growth rate of the market share of the firm (log)
*/

v[0] = VL( "_s", 1 );
v[1] = V( "_s" );
v[2] = V( "_age" );
v[3] = V( "_incumbent" );

if( v[2] == 0 && ! v[3] )					// just entered market (non-incumbent)
	v[4] = NAN;								// growth mark not a number
else
	v[4] = log( v[1] ) - log( v[0] );

RESULT( v[4] )


EQUATION( "_aEnd" )
/*
Productivity after entry/exit (should not be used out of here).
It's needed because regular "_a" is recalculated in the beginning of
the time step, before exit and entry happen, so the saved values of
"_a" are inconsistent for cross-section analysis.
*/

v[0] = V( "_a" );

RESULT( v[0] )


EQUATION( "_aNorm" )
/*
Normalized productivity after entry/exit (should not be used out of here).
Normalized using current period weighted average productivity.
*/

v[0] = V( "_aEnd" );
v[1] = V( "aAvg" );

RESULT( v[0] / v[1] )


EQUATION( "_aGrowth" )
/*
Calculates the growth rate of the (stable) productivity of the firm
*/

v[0] = VL( "_aEnd", 1 );
v[1] = V( "_aEnd" );
v[2] = V( "_age" );
v[3] = V( "_incumbent" );

if( v[2] == 0 && ! v[3] )					// just entered market (non-incumbent)
	v[4] = NAN;								// growth not a number
else
	v[4] = v[1] / v[0] - 1;

RESULT( v[4] )


// ##################### MARKET AGGREGATES CALCULATION ################### //

EQUATION( "aAvg1" )
/*
Average Productivity weighted with the value of shares.
Calculated with the productivity values before entry/exit ("_a").
*/

v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VS( cur, "_a" );
	v[2] = VLS( cur, "_s", 1 ); 
  
	if( ! isnan( v[1] ) && ! isnan( v[2] ) )	// do not consider NaNs
		v[0] += v[1] * v[2];
	else
		v[0] = NAN;							// not a number
}

RESULT( v[0] )


EQUATION( "aAvg2" )
/*
Average Productivity weighted with the value of past shares and productivities. 
Only for MarkII.
Calculated with the productivity values before entry/exit ("_a").
*/

v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VLS( cur, "_a", 1 );
	v[2] = VLS( cur, "_s", 1 );

	if( ! isnan( v[1] ) && ! isnan( v[2] ) )	// do not consider NaNs
		v[0] += v[1] * v[2];
	else
		v[0] = NAN;							// not a number
}

RESULT( v[0] )


EQUATION( "aAvg" )
/*
Average Productivity weighted with the current value of shares and  productivity.
Only for statistics (do not use in equations).
Calculated with the productivity values after entry/exit ("_a").
*/

v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VS( cur, "_aEnd" );
	v[2] = VS( cur, "_s" );

	if( ! isnan( v[1] ) )					// do not consider NaNs
		v[0] += v[1] * v[2];
}

RESULT( v[0] )


EQUATION( "aMax" )
/*
Technological frontier (maximum productivity)
Calculated with the productivity values after entry/exit ("_aEnd")
*/

v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VS( cur, "_aEnd" );

	if( ! isnan( v[1] ) && v[1] > v[0] )
		v[0] = v[1];						// do not consider NaNs
}

RESULT( v[0] )								// maximum productivity


EQUATION( "ageAvg" )
/*
Firms' average age
*/

v[0] = v[1] = 0;

CYCLE( cur, "Firm" )
{
	v[2] = VS( cur, "_age" );

	if( ! isnan( v[2] ) )					// do not consider NaNs
	{
		v[0] += v[2];
		v[1] ++;
	}
}

RESULT( v[0] / v[1] )						// average age


EQUATION( "ageMax" )
/*
Firms' maximum age
*/

v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VS( cur, "_age" );

	if( ! isnan( v[1] ) && v[1] > v[0] )
		v[0] = v[1];						// do not consider NaNs
}

RESULT( v[0] )								// maximum age


EQUATION( "growthAvg" )
/*
Firms' average market share growth
*/

v[0] = v[1] = 0;

CYCLE( cur, "Firm" )
{
	v[2] = VS( cur, "_growth" );

	if( ! isnan( v[2] ) )					// do not consider NaNs
	{
		v[0] += v[2];
		v[1] ++;
	}
}

RESULT( v[0] / v[1] )						// average 


EQUATION( "growthSD" )
/*
Firms' market share growth standard deviation
*/

v[0] = v[1] = v[2] = 0;

CYCLE( cur, "Firm" )
{
	v[3] = VS( cur, "_growth" );

	if( ! isnan( v[3] ) )					// do not consider NaNs
	{
		v[0] += v[3];
		v[1] += pow( v[3], 2 );
		v[2] ++;
	}
}

v[4] = v[1] / v[2] - pow( v[0] / v[2], 2 );	// variance

if ( v[4] < 0 )								// prevent small rounding errors
	v[5] = 0;								// when SD is too small
else
	v[5] = sqrt( v[4] ) ;

RESULT( v[5] )


EQUATION( "growthMax" )
/*
Firms' maximum market share growth
*/

v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VS( cur, "_growth" );

	if( ! isnan( v[1] ) && v[1] > v[0] )	// do not consider NaNs
		v[0] = v[1];
}

RESULT( v[0] )								// maximum 


EQUATION( "aGrowthAvg" )
/*
Firms' average productivity growth
*/

v[0] = v[1] = 0;

CYCLE( cur, "Firm" )
{
	v[2] = VS( cur, "_aGrowth" );

	if( ! isnan( v[2] ) )					// do not consider NaNs
	{
		v[0] += v[2];
		v[1] ++;
	}
}

RESULT( v[0] / v[1] )						// average 


EQUATION( "aGrowthSD" )
/*
Firms' productivity growth standard deviation
*/

v[0] = v[1] = v[2] = 0;

CYCLE( cur, "Firm" )
{
	v[3] = VS( cur, "_aGrowth" );

	if( ! isnan( v[3] ) )					// do not consider NaNs
	{
		v[0] += v[3];
		v[1] += pow( v[3], 2 );
		v[2] ++;
	}
}

v[4] = v[1] / v[2] - pow( v[0] / v[2], 2 );	// variance

if ( v[4] < 0 )								// prevent small rounding errors
	v[5] = 0;								// when SD is too small
else
	v[5] = sqrt( v[4] ) ;

RESULT( v[5] )


EQUATION( "aGrowthMax" )
/*
Firms' maximum productivity growth
*/

v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VS( cur, "_aGrowth" );

	if( ! isnan( v[1] ) && v[1] > v[0] )	// do not consider NaNs
		v[0] = v[1];
}

RESULT( v[0] )								// maximum 


// ######################## STATISTICS GENERATION ######################## //

EQUATION( "totalShares" )
/*
Sum all the shares
*/

V( "adjustShares" );						// ensure shares are already adjusted
v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VS( cur, "_s" );

	if( ! isnan( v[1] ) )					// do not consider NaNs
		v[0] += v[1];
}

RESULT( v[0] )								// maximum 


EQUATION( "totalFirms" )
/*
Total number of firms
*/

v[0] = 0;

CYCLE( cur, "Firm" )
{
	v[1] = VS( cur, "_s" );

	if( ! isnan( v[1] ) )					// do not consider NaNs
		v[0]++;
}

RESULT( v[0] )


EQUATION( "N" )
/*
Total number of incumbents
*/

v[0] = 0;									// incumbent counter

CYCLE( cur, "Firm" )						// get number of firms in v[0]
{
	v[1] = VS( cur, "_incumbent" );			// incumbent flag

	if ( ! isnan( v[1] ) && v[1] )			// if it's a incumbent (and not a NaN)
		v[0]++;								// do not consider outliers
}

RESULT( v[0] )


EQUATION( "HHI" )
/*
Herfindahl-Hirschman index 
According to the stdHHI flag, the normalized/standardized HHI (corrected for 
the number of firms) is calculated
*/

v[0] = v[1] = 0;							// squared share / firm accumulators

CYCLE( cur, "Firm" )						// all firms in the market
{
	v[2] = VS( cur, "_s" );					// market share

	if( ! isnan( v[2] ) )					// do not consider NaNs
	{
		v[0] += pow( v[2], 2 );				// compute sum of squares
		++v[1];								// count firm
	}
}

if ( stdHHI )
{
	if ( v[1] > 1 )
		v[0] = ( v[0] - 1 / v[1] ) / ( 1 - 1 / v[1] );// normalization
	else
		if ( v[1] == 1 )
			v[0] = 1;
		else
			v[0] = 0;
}
	
v[0] = fmax( 0, fmin( v[0], 1 ) );			// handle abnormal cases	

RESULT( v[0] )


EQUATION( "dS" )
/*
Sum of the delta marke shares index
*/

v[0] = 0;									// d share accumulator

CYCLE( cur, "Firm" )						// all firms in the market
{
	v[1] = VS( cur, "_s" );					// market share in t
	v[2] = VLS( cur, "_s", 1 );				// market share in t-1

	if( ! isnan( v[1] ) && ! isnan( v[2] ) )	// do not consider NaNs
		v[0] += abs( v[1] - v[2] );			// compute sum
}

v[0] = fmax( 0, fmin( v[0], 2 ) );			// handle abnormal cases	

RESULT( v[0] )


// ######################## RANDOM DRAW FUNCTIONS ######################## //

EQUATION( "InnoShock" )
/*
Represent an innovation shock drawing from the appropriated distribution,
according to the selected profile
*/

v[0] = floor( V( "InnoProf" ) );			// get innovation profile
v[0] = fmax( 1, fmin( v[0], 6 ) );			// limit to valid range

switch ( (int) v[0] )						// pick the right distribution
{
	case 1:									// Uniform
		v[1] = V( "uniformInnoDraw" );
		break;

	case 2:									// Poisson
		v[1] = V( "poissonInnoDraw" );
		break;

	case 3:									// Gaussian (normal)
		v[1] = V( "normalInnoDraw" );
		break;

	case 4:									// Lognormal
		v[1] = V( "lognormInnoDraw" );
		break;

	case 5:									// (asymmetric) Laplace
		v[1] = V( "laplaceInnoDraw" );
		break;

	case 6:									// Beta
		v[1] = V( "betaInnoDraw" );
		break;
}

RESULT( v[1] )


EQUATION( "uniformInnoDraw" )
/*
Draw a random number from a uniform distribution
*/

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


EQUATION( "poissonInnoDraw" )
/*
Draw a random number from a Poisson distribution.
The draw result has expected value defined by Mu.
*/

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


EQUATION( "normalInnoDraw" )
/*
Draw a random number from a Gaussian (normal) distribution.
The draw result has expected value defined by Mu.
*/

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


EQUATION( "lognormInnoDraw" )
/*
Draw a random number from a lognormal distribution
*/

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


EQUATION( "laplaceInnoDraw" )
/*
Draw a random number from a asymmetric Laplace distribution.
The draw result has expected value defined by Mu.
*/

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


EQUATION( "betaInnoDraw" )
/*
Draw a random number from a beta distribution using two fixed parameters.
The draw result is rescaled to the defined support.
*/

v[0] = V( "BetaAlpha" );					// get the appropriate values for alfa and beta
v[1] = V( "BetaBeta" );
v[2] = V( "BetaMin" );						// inferior support to beta distribution
v[3] = V( "BetaMax" );						// superior support to beta distribution
v[4] = V( "MuMax" );						// maximum draw value (if >0)

v[5] = beta( v[0], v[1] );					// draw from Beta(alpha,beta)
v[5] = v[2] + v[5] * ( v[3] - v[2] );		// rescale on support (betaMin,betaMax)

if ( v[4] > 0 && v[4] > v[2] && v[4] < v[3] )// if upper truncation is necessary
{
	v[6] = ( v[4] - v[2] ) / ( v[3] - v[2] ); // descale upper bound
	v[7] = betacdf( v[0], v[1], v[6] );		// cdf up to descaled upper bound

	v[5] = min( v[5], v[4] );				// truncate to the upper bound
	v[5] /= v[7];							// rescale draw
}

RESULT( v[5] )


EQUATION( "uniformDrawEntr" )
/*
Draw a random number from a uniform distribution (for entry only)
*/

v[0] = V( "UniformMinEntr" );
v[1] = V( "UniformMaxEntr" );

RESULT( uniform( v[0], v[1] ) )


EQUATION( "betaInnoDrawEntr" )
/*
Draw a random number from a beta distribution using two fixed parameters.
Used for initial theta (entrants) only.
Expected value is not controlled (given by the parameters only).
*/

v[0] = V( "BetaAlphaEntr" );				// get the appropriate values for alfa and beta
v[1] = V( "BetaBetaEntr" );
v[2] = V( "BetaMinEntr" );					// inferior support to beta distribution
v[3] = V( "BetaMaxEntr" );					// superior support to beta distribution

v[4] = beta( v[0], v[1] );					// Estraggo da Beta(alpha,beta)
v[4] = v[2] + v[4] * ( v[3] - v[2] );		// Riscalo su supporto (betaMin,betaMax)

RESULT( v[4] )


MODELEND


void close_sim(void)
{

}
