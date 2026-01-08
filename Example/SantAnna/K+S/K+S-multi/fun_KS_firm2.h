/******************************************************************************

	FIRM2 OBJECT EQUATIONS
	----------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the Firm2 objects in the K+S LSD model
	are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_Deb2max" )
/*
Prudential maximum bank debt of firm in consumer-good industry
*/

// maximum debt allowed to firm, considering net worth and operating margin
v[5] = VS( FINSECL2, "Lambda" ) * max( VL( "_NW2", 1 ),
									   VL( "_S2", 1 ) - VL( "_W2", 1 ) );

// apply an absolute floor to maximum debt prudential limit
v[0] = max( v[5], VS( FINSECL2, "Lambda0" ) * VLS( GRANDPARENT, "PPI", 1 ) );

WRITE( "_cred2c", 0 );							// reset constraint for period

RESULT( v[0] )


EQUATION( "_D2e" )
/*
Adaptive demand (in money terms) expectation of firm in consumer-good industry
*/

if ( V( "_life2cycle" ) < 3 )					// entrant?
	// myopic-optimistic expectations
	END_EQUATION( max( VL( "_D2d", 1 ), CURRENT ) );

v[9] = VS( PARENT, "e0" );						// animal spirits parameter
k = VS( GRANDPARENT, "flagExpect" );			// expectation form
j = ( k == 0 || k > 4 ) ? 1 : ( k == 1 ) ? 4 : 2;// req. number of data periods

// compute the mix between fulfilled and potential demand (orders)
for ( i = 1; i <= j; ++i )
{
	v[10] = VL( "_D2", i );
	v[ i ] = max( ( 1 - v[9] ) * v[10] + v[9] * VL( "_D2d", i ), v[10] );
}

switch ( k )
{
	// myopic expectations with 1-period memory
	case 0:
	default:
		v[0] = v[1];
		break;

	// myopic expectations with up to j-period memory
	case 1:
		v[11] = VS( PARENT, "e1" );				// weight of t-1 demand
		v[12] = VS( PARENT, "e2" );				// weight of t-2 demand
		v[13] = VS( PARENT, "e3" );				// weight of t-3 demand
		v[14] = VS( PARENT, "e4" );				// weight of t-4 demand

		for ( v[5] = v[6] = 0, i = 1; i <= j; ++i )
			if ( v[i] > 0 )						// consider only periods with demand
			{
				v[5] += v[ 10 + i ] * v[ i ];
				v[6] += v[ 10 + i ];
			}

		v[0] = v[6] > 0 ? v[5] / v[6] : 0;		// rescale
		break;

	// accelerating GD expectations
	case 2:
		v[2] = max( v[2], 1 );					// floor to positive only

		v[0] = ( 1 + VS( PARENT, "e5" ) * ( v[1] - v[2] ) / v[2] ) * v[1];
		break;

	// 1st order adaptive expectations
	case 3:
		v[0] = CURRENT + VS( PARENT, "e6" ) * ( v[1] - v[2] );
		break;

	// extrapolative-accelerating expectations
	case 4:
		v[2] = max( v[2], 1 );					// floor to positive only

		v[0] = ( 1 + VS( PARENT, "e7" ) * ( v[1] - v[2] ) / v[2] +
				 VS( PARENT, "e8" ) * VLS( GRANDPARENT, "dGDPreal", 1 ) ) * v[1];
		break;
}

RESULT( v[0] )


EQUATION( "_E2" )
/*
Effective competitiveness of a firm in consumption-good industry, considering
the price, unfilled demand and the quality of the product for the consumer
*/

if ( V( "_life2cycle" ) == 0 )
	END_EQUATION( VLS( PARENT, "E2avg", 1 ) );	// non-producing entrant

v[1] = VS( PARENT, "p2min" );					// market parameters
v[2] = VS( PARENT, "p2max" );
v[3] = VLS( PARENT, "l2min", 1 );
v[4] = VLS( PARENT, "l2max", 1 );
v[5] = VS( PARENT, "q2min" );
v[6] = VS( PARENT, "q2max" );
v[7] = VS( PARENT, "omega1" );					// competitiveness weights
v[8] = VS( PARENT, "omega2" );
v[9] = VS( PARENT, "omega3" );

// normalize price, unfilled demand, and quality to [0.1, 0.9]
// zero competitiveness is avoided to prevent direct exit of worst in replicator
v[10] = v[2] > v[1] ? 0.1 + 0.8 * ( V( "_p2" ) - v[1] ) / ( v[2] - v[1] ) : 0.5;
v[11] = v[4] > v[3] ? 0.1 + 0.8 * ( VL( "_l2", 1 ) - v[3] ) / ( v[4] - v[3] ) : 0.5;
v[12] = v[6] > v[5] ? 0.1 + 0.8 * ( V( "_q2" ) - v[5] ) / ( v[6] - v[5] ) : 0.5;

RESULT( v[7] * ( 1 - v[10] ) + v[8] * ( 1 - v[11] ) + v[9] * v[12] )


EQUATION( "_EI2" )
/*
Effective expansion investment (in machine-number terms) of firm in
consumption-good industry
*/

V( "_Q2" );										// make sure production decided
V( "_suppl2" );									// ensure supplier is selected
cur = PARENTS( SHOOKS( HOOK( SUPPL ) ) );		// pointer to new supplier

v[1] = V( "_EI2d" );							// desired expansion investment

if ( v[1] <= 0 )
	END_EQUATION( 0 );							// nothing to do

v[2] = V( "_cred2" );							// available credit
v[3] = V( "_NW2" );								// net worth (cash available)
v[4] = VS( cur, "_p1" );						// new machine price

v[6] = v[4] * v[1];								// expansion investment cost

if ( v[6] <= v[3] - 1 )							// can invest with own funds?
{
	v[0] = v[1];								// expand as planned
	v[3] -= v[6];								// remove machines cost from cash
}
else
{
	if ( v[6] <= v[3] - 1 + v[2] )				// possible to finance all?
	{
		v[0] = v[1];							// expand as planned
		v[7] = v[8] = v[6] - v[3] + 1;			// finance the difference
		v[3] = 1;								// keep minimum cash
	}
	else										// credit constrained firm
	{
		// invest as much as the available finance allows, rounded to # machines
		v[0] = max( ( v[3] - 1 + v[2] ) / v[4], 0 );

		if ( v[0] == 0 )
			END_EQUATION( 0 );					// nothing to do

		v[7] = v[2];							// take all credit available
		v[8] = v[6] - v[3] + 1;					// desired credit
		v[3] = 1;								// keep minimum cash
	}

	CFUN( update_debt2, v[8], v[7] );			// update firm debt
}

WRITE( "_NW2", v[3] );							// update the firm net worth

CFUN( send_order, v[0] );						// send order to machine supplier

RESULT( v[0] )


EQUATION( "_EI2d" )
/*
Desired expansion investment (in machine-number terms) of firm in
consumption-good industry
*/

v[1] = V( "_K2d" );								// desired capital
v[2] = VL( "_K2", 1 );							// available capital stock

if ( v[2] <= 0 )								// no capital yet?
	END_EQUATION( v[1] );						// no growth threshold

v[4] = VS( PARENT, "kappaMin" );				// investment floor multiple

v[5] = ( 1 + v[4] ) * v[2];						// min capital

if ( v[1] > v[5] )								// minimum capital reached?
{
	if ( v[4] > 0 )
		v[0] = v[5] - v[2];
	else
	{
		v[6] = VS( PARENT, "kappaMax" );		// investment cap multiple
		v[7] = ( 1 + v[6] ) * v[2];				// max capital

		if ( v[6] > 0 && v[1] > v[7] )
			v[0] = v[7] - v[2];					// apply cap
		else
			v[0] = v[1] - v[2];
	}
}
else
	v[0] = 0;									// no expansion investment

RESULT( v[0] )


EQUATION( "_K2d" )
/*
Desired capital stock (in machine-number terms) of firm in
consumption-good industry
*/

if ( V( "_life2cycle" ) == 0 )					// if fresh entrant
	END_EQUATION( CURRENT );					// keep initially desired capital

// desired capacity with slack and utilization based on expectations/inventories
v[1] = max( ( 1 + VS( PARENT, "iota" ) ) * V( "_D2e" ) / VL( "_p2", 1 ) -
			 VL( "_N2", 1 ), 0 ) / VS( PARENT, "u" );

// desired capital (number of machines) considering final machine productivity
RESULT( v[1] / ( VS( PARENT, "m2" ) / VS( PARENT, "k2" ) ) )


EQUATION( "_Q2" )
/*
Planned production for a firm in consumption-good industry
*/

v[1] = V( "_Q2d" );								// desired production

if ( v[1] == 0 )
	END_EQUATION( 0 );							// nothing to do

v[2] = V( "_cred2" );							// available credit
v[3] = VL( "_NW2", 1 );							// net worth (cash available)
v[4] = V( "_c2" );								// expected unit cost

v[6] = v[1] * v[4];								// cost of desired production

if ( v[6] <= v[3] - 1 )							// firm can self-finance?
{
	v[0] = v[1];								// plan the desired output
	v[3] -= v[6];								// remove cost (wages) from cash
}
else
{
	if ( v[6] <= v[3] - 1 + v[2] )				// possible to finance all?
	{
		v[0] = v[1];							// plan the desired output
		v[7] = v[8] = v[6] - v[3] + 1;			// finance the difference
		v[3] = 1;								// keep minimum cash
	}
	else										// credit constrained firm
	{
		// produce as much as the available finance allows
		v[0] = max( ( v[3] - 1 + v[2] ) / v[4], 0 ); // positive production only

		if ( v[0] == 0 )
			END_EQUATION( 0 );					// nothing to do

		v[7] = v[2];							// take all credit available
		v[8] = v[6] - v[3] + 1;					// desired credit
		v[3] = 1;								// keep minimum cash
	}

	CFUN( update_debt2, v[8], v[7] );			// update firm debt
}

// provision for wage expenses
WRITE( "_NW2", v[3] );							// update the firm net worth

RESULT( v[0] )


EQUATION( "_Q2d" )
/*
Desired output of firm in consumption-good industry
*/

if ( V( "_life2cycle" ) == 0 )					// if fresh entrant
	END_EQUATION( 0 );							// not yet producing

// available production capacity considering final machine productivity
v[1] = VL( "_K2", 1 ) * VS( PARENT, "m2" ) / VS( PARENT, "k2" );

// desired production with slack, based on expectations, considering inventories
v[2] = max( ( 1 + VS( PARENT, "iota" ) ) * V( "_D2e" ) / VL( "_p2", 1 ) -
			VL( "_N2", 1 ), 0 );

RESULT( min( v[1], v[2] ) )						// limit to available capital


EQUATION( "_SI2" )
/*
Effective substitution investment (in machine-number terms) of firm in
consumption-good industry
*/

V( "_EI2" );									// make sure expansion done

v[1] = V( "_SI2d" );							// desired substitution invest.

if ( v[1] <= 0 )
	END_EQUATION( 0 );							// nothing to do

v[2] = V( "_cred2" );							// available credit
v[3] = V( "_NW2" );								// net worth (cash available)
v[4] = VS( PARENTS( SHOOKS( HOOK( SUPPL ) ) ), "_p1" );// new machine price

v[6] = v[4] * v[1];								// substitution investment cost

if ( v[6] <= v[3] - 1 )							// can invest with own funds?
{
	v[0] = v[1];								// substitute as planned
	v[3] -= v[6];								// remove machines cost from cash
}
else
{
	if ( v[6] <= v[3] - 1 + v[2] )				// possible to finance all?
	{
		v[0] = v[1];							// substitute as planned
		v[7] = v[8] = v[6] - v[3] + 1;			// finance the difference
		v[3] = 1;								// keep minimum cash
	}
	else										// credit constrained firm
	{
		// invest as much as the available finance allows
		v[0] = max( ( v[3] - 1 + v[2] ) / v[4], 0 );

		if ( v[0] == 0 )
			END_EQUATION( 0 );					// nothing to do

		v[7] = v[2];							// take all credit available
		v[8] = v[6] - v[3] + 1;					// desired credit
		v[3] = 1;								// keep minimum cash
	}

	CFUN( update_debt2, v[8], v[7] );			// update firm debt
}

WRITE( "_NW2", v[3] );							// update the firm net worth

CFUN( send_order, v[0] );						// send order to machine supplier

RESULT( v[0] )


EQUATION( "_Tax2" )
/*
Tax paid by firm in consumption-good industry
Also updates final net wealth on period
*/

v[1] = V( "_Pi2" );								// firm profit in period
v[2] = VS( GRANDPARENT, "tr" );					// tax rate

if ( v[1] > 0 )									// profits?
{
	v[0] = v[1] * v[2];							// tax to government

	// pay bonus only if firm has above-average profit rate and workers employed
	if ( V( "_Pi2rate" ) > VLS( PARENT, "Pi2rateAvg", 1 ) && V( "_L2" ) > 0 )
		v[3] = VS( LABSUPL2, "psi6" ) * ( v[1] - v[0] );
	else
		v[3] = 0;

	v[4] = VS( PARENT, "d2" ) * ( v[1] - v[0] - v[3] );// shareholders dividend
}
else
	v[0] = v[3] = v[4] = 0;						// no tax/bonus/divid. on losses

WRITE( "_Bon2", v[3] );							// save period bonus
WRITE( "_Div2", v[4] );							// save period dividends

// compute free cash flow
v[6] = v[1] - v[0] - v[3] - v[4];

// remove from net wealth the provision for wages
v[7] = INCR( "_NW2", V( "_Q2" ) * V( "_c2" ) );

if ( v[6] < 0 )									// must finance losses?
{
	if ( v[7] >= - v[6] + 1 )					// can cover losses with reserves?
		INCR( "_NW2", v[6] );					// draw from net wealth
	else
	{
		v[8] = V( "_cred2" );					// available credit
		v[9] = - v[6] - v[7] + 1;				// desired finance

		if ( v[8] >= v[9] )						// can finance losses?
		{
			CFUN( update_debt2, v[9], v[9] );	// finance all
			WRITE( "_NW2", 1 );					// minimum net wealth
		}
		else
		{
			CFUN( update_debt2, v[8], v[8] );	// take what is possible
			INCR( "_NW2", v[6] - v[8] );		// let negative NW (bankruptcy)
		}
	}
}
else											// pay debt with available cash
{
	v[10] = V( "_Deb2" );						// current debt

	if ( v[10] > 0 )							// has debt?
	{
		if ( v[6] > v[10] )						// can repay all debt and more
		{
			CFUN( update_debt2, 0, - v[10] );	// zero debt
			INCR( "_NW2", v[6] - v[10] );		// save the rest
		}
		else
			CFUN( update_debt2, 0, - v[6] );	// repay part of debt
	}
	else
		INCR( "_NW2", v[6] );					// save all
}

RESULT( v[0] )


EQUATION( "_alloc2" )
/*
Allocate unallocated workers to vintages, prioritizing newer vintages
*/

cur = HOOK( TOPVINT );							// start with top vintage
if ( cur == NULL )
	END_EQUATION( 0 );							// no vintage, nothing to do

V( "_L2" );										// ensure hiring is done
V( "_c2" );										// ensure machines are allocated
SUM( "_dLdVint" );								// unallocate unneeded workers

h = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
bool vint_learn = ( h != 0 && h != 2 );			// learning-by-vintage in use?

// first, allocate unallocated workers to top vintages
// going back from newest vintage to oldest, if needed
i = 0;											// allocations counter
k = VS( cur, "_dLdVint" );						// addt'l labor demand of vint.
CYCLE( cur1, "Wrk2" )							// search for unallocated worker
	if ( HOOKS( SHOOKS( cur1 ), VWRK ) == NULL )
	{
		while ( k == 0 )						// find vintage with open posit.
		{
			cur = SHOOKS( cur );				// so go to the previous one
			if ( cur == NULL || VS( cur, "_toUseVint" ) == 0 )// oldest or done?
				goto done_alloc2;				// not possible to allocate more

			k = VS( cur, "_dLdVint" );			// addt'l labor demand of vint.
		}

		CFUNS( SHOOKS( cur1 ), move_worker, cur, vint_learn );// move to vintage
		++i;
		--k;									// update vintage worker demand
	}

// then, if needed, try to move workers from older to newer vintages
// keep going back while not get to oldest or the last vintage in use
CYCLE( cur1, "Vint" )							// search for unallocated worker
{
	j = COUNTS( cur1, "WrkV" );
	while ( j > 0 )
	{
		while ( k == 0 )						// find vintage with open posit.
		{
			cur = SHOOKS( cur );				// so go to the previous one
			if ( cur == NULL || cur == cur1 )	// oldest or same vintage?
				goto done_alloc2;				// not possible to allocate more

			k = VS( cur, "_dLdVint" );			// addt'l labor demand of vint.
		}

		cur2 = SEARCHS( cur1, "WrkV" );			// pick old vint. first worker
		CFUNS( SHOOKS( cur2 ), move_worker, cur, vint_learn );// move to vintage
		DELETE( cur2 );							// remove old bridge-object
		++i;
		--k;									// update new vintage demand
		--j;									// one worker less in old vint.
	}
}

done_alloc2:

RESULT( i * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_c2" )
/*
Planned average output unit cost of firm in consumption-good industry
Unit costs depend on the number of production stages (complexity)
Machine-level productivities (_A2, _A2p, m2) are notional (single production
stage) and DO NOT directly represent firm final labor/capital productivities
Also updates '_A2', '_A2p'
*/

v[1] = V( "_Q2d" );								// desired production
v[2] = VL( "_K2", 1 );							// available capital stock
v[3] = VS( PARENT, "m2" ) / VS( PARENT, "k2" );	// final capital productivity
v[4] = VL( "_w2avg", 1 );						// average firm wage

// number of unused machines, max should be total-1 to compute cost/productivity
v[5] = max( v[2] - max( v[1] / v[3], 1 ), 0 );

// scan all vintages, from oldest to newest, preferring to use newer ones
v[0] = v[6] = v[7] = v[8] = 0;					// accumulators
CYCLE( cur, "Vint" )							// choose vintages to use
{
	v[9] = VS( cur, "_nVint" );					// number of machines in vintage

	if ( v[5] >= v[9] )							// none to be used in the vint.?
	{
		v[5] -= v[9];							// less machines not to use
		v[12] = 0;								// no machine to use in vintage
	}
	else
	{
		v[10] = VS( cur, "_Avint" );			// vintage notional productivity
		v[11] = VLS( cur, "_AeVint", 1 );		// vintage effective product.
		v[6] += v[12] = v[9] - v[5];			// add to be used machines
		v[7] += v[12] * v[10];					// add notional productivities
		v[8] += v[12] * v[11];					// add effective productivities
		v[0] += v[12] * v[4] / ( v[3] * v[11] );// add used machines oper. cost
		v[5] = 0;								// no more machine not to use

		if ( v[1] == 0 )
			v[12] = 0;							// no worker if no production
	}

	WRITES( cur, "_toUseVint", v[12] );			// number mach. to try to use
}

if ( v[6] == 0 )								// no machine?
{
	V( "_suppl2" );								// ensure supplier is selected
	cur1 = PARENTS( SHOOKS( HOOK( SUPPL ) ) );	// pointer to supplier

	v[6] = 1;									// 1 notional machine
	v[7] = v[8] = VS( cur1, "_A1" );			// new machines prod.
	v[0] = v[4] / v[7];							// machine unit cost
}

WRITE( "_A2p", v[7] / v[6] );					// potential notional product.
WRITE( "_A2", v[8] / v[6] );					// expected notional product.

RESULT( v[0] / v[6] )


EQUATION( "_f2" )
/*
Market share of firm in consumption-good industry
Computed using a replicator equation over the relative firm competitiveness
Because of entry/exit, market shares may add to more/less than one,
so 'f2rescale' must be used before '_f2' is used
*/

v[1] = VS( PARENT, "f2min" );						// minimum share to stay

switch( ( int ) V( "_life2cycle" ) )				// entrant firm state
{
	case 0:											// non-producing entrant
	default:										// exiting incumbent
		END_EQUATION( 0 );

	case 1:											// first-period entrant
		v[0] = VL( "_K2", 1 ) / VLS( PARENT, "K2", 1 );// use capital share
		END_EQUATION( max( v[0], v[1] ) );			// but over minimum

	case 2:											// 2nd-4th-period entrant
		// inter-firm, intra-industry replicator equation
		v[0] = CURRENT * ( 1 + VS( PARENT, "chi2" ) *
						   ( V( "_E2" ) / VS( PARENT, "E2avg" ) - 1 ) );
		v[0] = max( v[0], v[1] );					// but over minimum
		break;

	case 3:											// incumbent
		// inter-firm, intra-industry replicator equation
		v[0] = CURRENT * ( 1 + VS( PARENT, "chi2" ) *
						   ( V( "_E2" ) / VS( PARENT, "E2avg" ) - 1 ) );
}

// lower-bounded slightly below (multi-period) exit threshold
// to ensure firm sells production before leaving the market
RESULT( max( v[0], 0.99 * v[1] / VS( PARENT, "n2" ) ) )


EQUATION( "_fires2" )
/*
Number of workers fired by firm in consumption-good industry
Process required firing using the appropriate rule
*/

if ( V( "_life2cycle" ) == 0 )					// entrant firm?
	END_EQUATION( 0 );

v[1] = max( V( "_Q2pe" ) - V( "_Q2d" ), 0 );	// expected extra capacity

// check if would fire too many workers because of scaling (# of "modules")
v[2] = ( VS( PARENT, "m2" ) * V( "_A2" ) / VS( PARENT, "k2" ) ) *
	   VS( LABSUPL2, "Lscale" );				// prod.-adjusted module size
v[1] = floor( v[1] / v[2] ) * v[2];				// rounded down headcount shrink

// pick the appropriate firing rule
int fRule = V( "_post2chg" ) ? VS( GRANDPARENT, "flagFireRuleChg" ) :
							   VS( GRANDPARENT, "flagFireRule" );
switch ( fRule )
{
	case 0:										// never fire (exc. retirement)
	case 1:										// never fire with sharing
	default:
		v[0] = 0;
		break;

	case 2:										// only fire if firm downsizing
		// production being reduced and extra capacity is expected?
		if ( V( "_dQ2d" ) < 0 && v[1] > 0 )		// workers have to be fired?
			v[0] = CFUN( fire_workers, MODE_ADJ, v[1], & v[2] );
		else
			v[0] = 0;
		break;

	case 3:										// only fire if firm at losses
		// production being reduced and extra capacity is expected?
		if ( VL( "_Pi2", 1 ) < 0 && v[1] > 0 )	// workers have to be fired?
			v[0] = CFUN( fire_workers, MODE_ADJ, v[1], & v[2] );
		else
			v[0] = 0;
		break;

	case 4:										// fire if payback is achieved
		// fire insufficient payback workers
		v[0] = CFUN( fire_workers, MODE_PBACK, v[1], & v[2] );
		break;

	case 5:										// fire when contract ends
		// fire all workers with finished contracts
		v[0] = CFUN( fire_workers, MODE_ALL, v[1], & v[2] );
		break;

	case 6:										// reg. 5 until t=T, then reg. Y
		// fire non needed, non stable workers
		v[0] = CFUN( fire_workers, MODE_IPROT, v[1], & v[2] );
}

RESULT( v[0] )


EQUATION( "_mu2" )
/*
Mark-up of firm in consumption-good industry
*/

v[1] = VL( "_f2", 1 );							// past periods market shares
v[2] = VL( "_f2", 2 );
v[3] = VS( PARENT, "f2min" );					// market exit share threshold

if ( v[1] < v[3] || v[2] < v[3] )				// just entered firms keep it
	END_EQUATION( CURRENT );

v[0] = CURRENT * ( 1 + VS( PARENT, "upsilon" ) * ( v[1] / v[2] - 1 ) );

// check for reduction when out of credit
if ( v[0] < CURRENT && V( "_cred2" ) <= 0 )
	v[0] = CURRENT;

// check for and bound abnormal change
v[4] = VS( GRANDPARENT, "mLim" );				// growth cap if != 0
if ( v[4] > 0 )
{
	v[5] = v[0] / CURRENT;						// calculate multiple
	v[6] = v[5] < 1 ? 1 / v[5] : v[5];			// handle decreases/increases
	if ( v[6] > 1 + v[4] )						// explosive change?
		v[0] = v[5] < 1 ? CURRENT / ( 1 + v[4] ) : CURRENT * ( 1 + v[4] );
}

RESULT( v[0] )


EQUATION( "_p2" )
/*
Price of good of firm in consumption-good industry
Entrants notional price are the market average
*/
RESULT( V( "_life2cycle" ) == 0 ? VLS( PARENT, "p2", 1 ) :
								  ( 1 + V( "_mu2" ) ) * V( "_c2" ) )


EQUATION( "_q2" )
/*
Product quality of a firm in consumption-good industry
Equal to the average of the log tenure skills of workers
*/

v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_sT" );
	++i;
}

RESULT( i > 0 ? v[0] / i : VLS( PARENT, "q2", 1 ) )


EQUATION( "_suppl2" )
/*
Selected machine supplier by firm in consumption-good industry
Also set firm 'hook' pointers to supplier firm object
*/

VS( GRANDPARENT, "innI" );						// ensure innovation is done and
												// brochures distributed
h = VS( PARENT, "g2base" );						// base technology generation

v[1] = DBL_MAX;									// supplier price/cost ratio
i = 0;
cur2 = cur3 = NULL;
CYCLE( cur, "Broch" )							// use brochures to find supplier
{
	cur1 = PARENTS( SHOOKS( cur ) );			// pointer to supplier object

	// compare price to productivity ratios
	v[2] = VS( cur1, "_p1" ) / VS( cur1, "_A1" );

	if ( h > 0 && VS( cur1, "_g1" ) >= h )		// adequate tech. generation?
		v[2] /= 1000;							// give preference to base gen.

	if ( v[2] < v[1] )							// best so far?
	{
		v[1] = v[2];							// save current best supplier
		i = VS( cur1, "_ID1" );					// supplier ID
		cur2 = SHOOKS( cur );					// own entry on supplier list
		cur3 = cur;								// best supplier brochure
	}
}

// if supplier is found, simply update it, if not, draw a random one
if ( cur2 != NULL && cur3 != NULL )
	WRITES( cur2, "_tSel", T );					// update selection time
else											// no brochure received
{	// draw new supplier
	cur1 = RNDDRAW_FAIRS( SEARCHS( GRANDPARENT, "Capital" ), "Firm1" );
	i = VS( cur1, "_ID1" );

	// create the brochure/client interconnected objects
	cur3 = CFUNS( cur1, send_brochure, THIS );
}

WRITE_HOOK( SUPPL, cur3 );						// pointer to current brochure

RESULT( i )


EQUATION( "_w2o" )
/*
Wage offer to workers in queue of firm in consumption-good industry
*/

if ( VS( GRANDPARENT, "flagHeterWage" ) == 0 )	// centralized wage setting?
{
	v[0] = VS( LABSUPL2, "wCent" );				// single wage centrally defined
	goto end_offer;
}

h = V( "_life2cycle" );							// firm status
k = V( "_post2chg" ) ? VS( GRANDPARENT, "flagWageOfferChg" ) :
					   VS( GRANDPARENT, "flagWageOffer" );

if ( k == 0 )									// wage premium mode?
{
	switch ( ( int ) VS( GRANDPARENT, "flagWagePremium" ) )
	{											// define wage premium type
		case 0:									// no premium
		default:
			v[0] = CURRENT;
			break;

		case 1:									// indexed premium (WP1)
			v[1] = VS( LABSUPL2, "psi1" );		// inflation adjust. parameter
			v[2] = VS( LABSUPL2, "psi2" );		// general prod. adjust. param.
			v[3] = VS( LABSUPL2, "psi3" );		// unemploym. adjust. parameter
			v[4] = VS( LABSUPL2, "psi4" );		// firm prod. adjust. parameter
			v[11] = VS( LABSUPL2, "psi5" );		// firm vacancy booster param.
			v[5] = VLS( GRANDPARENT, "dCPIb", 1 );// inflation variation
			v[6] = VLS( GRANDPARENT, "dAb", 1 );// general productivity var.
			v[7] = VLS( LABSUPL2, "dUeB", 1 );	// unemployment variation
			v[8] = ( h > 2 ) ? VL( "_dA2b", 1 ) : v[6];// firm productivity var.
			v[12] = VL( "_L2v", 1 );			// previous vacancy rate

			// make sure total productivity effect is bounded to 1
			if ( ( v[2] + v[4] ) > 1 )
				v[2] = max( 1 - v[4], 0 );		// adjust general prod. effect

			v[0] = CURRENT * ( 1 + v[1] * v[5] + v[2] * v[6] + v[3] * v[7] +
							   v[4] * v[8] + v[11] * v[12] );
			break;

		case 2:									// endogenous mechanism (WP2)
			v[9] = VLS( PARENT, "w2oAvg", 1 );
			v[9] = ( v[9] > 0 ) ? v[9] : VLS( LABSUPL2, "woAvg", 1 );
			v[0] = CURRENT * ( 1 + max( v[9] / CURRENT - 1, 0 ) );
	}
}
else
{												// lowest wage mode
	VS( LABSUPL2, "appl" );						// ensure applications are done
	j = ceil( ( V( "_L2d" ) - VL( "_L2", 1 ) ) * // number of workers (scaled)
			  ( 1 + VS( LABSUPL2, "theta" ) ) / VS( LABSUPL2, "Lscale" ) );
	j = max( j, 1 );							// minimum one worker for calc.

	// sort firm's candidate list according to the defined strategy
	int hOrder = V( "_post2chg" ) ? VS( GRANDPARENT, "flagHireOrder2Chg" ) :
									VS( GRANDPARENT, "flagHireOrder2" );
	CFUN( order_applications, hOrder, & V_EXT( firm2E, appl ) );

	// search applications set (increasing wage requests) for enough workers
	i = 0;									// workers counter
	v[0] = 0;								// highest wage found
	CYCLE_EXT( its, firm2E, appl )			// run over enough applications
	{
		if ( its->w > v[0] )				// new high wage request?
			v[0] = its->w;					// i-th worker wage

		if ( ++i >= j )
			break;							// stop when enough workers
	}

	if ( v[0] == 0 )						// no worker in queue
		v[0] = CURRENT;						// keep current offer
}

// check for abnormal change
v[13] = VS( LABSUPL2, "wCap" );					// wage cap multiplier
if ( h > 0 && v[13] > 0 )
{
	v[14] = v[0] / CURRENT;						// calculate multiple
	v[15] = v[14] < 1 ? 1 / v[14] : v[14];
	if ( v[15] > v[13] )						// explosive change?
	{
		v[16] = v[0];
		v[0] = v[14] < 1 ? CURRENT / v[13] : CURRENT * v[13];
	}
}

// check if non-entrant firm is able to pay wage
if ( h > 0 )
{
	v[10] = VL( "_p2", 1 ) * VS( PARENT, "m2" ) * VL( "_A2", 1 ) /
			VS( PARENT, "k2" );					// max wage for zero markup

	if ( v[10] > 0 && v[0] > v[10] )			// over max?
		v[0] = v[10];
	else
		if ( v[10] <= 0 )						// max can't be calculated?
			v[0] = min( v[0], CURRENT );		// limit to current
}

// under unemployment benefit or minimum wage? Adjust if necessary
v[0] = max( v[0], max( VS( LABSUPL2, "wU" ), VS( LABSUPL2, "wMinPol" ) ) );

end_offer:

// save offer in global offers set
wageOffer woData;
woData.offer = v[0];
woData.workers = VL( "_L2", 1 );
woData.firm = THIS;
EXEC_EXTS( GRANDPARENT, countryE, firm2wo, push_back, woData );

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_CI2" )
/*
Canceled investment (in machine-number terms) of firm in
consumption-good industry
*/

cur = HOOK( SUPPL );							// pointer to current supplier
VS( PARENTS( SHOOKS( cur ) ), "_Q1e" );			// make sure supplier produced

v[0] = VS( SHOOKS( cur ), "_nCan" );			// canceled machine number
k = VS( SHOOKS( cur ), "_tOrd" );				// time of canceled order

if ( k == T && v[0] > 0 )
{
	v[3] = V( "_SI2" );							// machines to substitute
	v[4] = V( "_EI2" );							// machines to expand
	v[5] = VS( PARENTS( SHOOKS( cur ) ), "_p1" );// machine price

	if ( v[0] > v[3] )							// no space for substitution?
	{
		WRITE( "_SI2", 0 );						// reset substitution investment
		WRITE( "_EI2", v[4] - v[0] + v[3] );	// remaining expansion inv.
	}
	else
		WRITE( "_SI2", v[3] - v[0] );			// shrink substitution investm.

	INCR( "_NW2", v[5] * v[0] );				// recover paid machines value
}
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "_D2d" )
/*
Desired (potential) demand (in money terms) for firm in consumption-good industry
*/
RESULT( VS( PARENT, "D2d" ) * V( "_f2" ) )


EQUATION( "_JO2" )
/*
Open job positions for a firm in consumption-good industry
*/

V( "_fires2" );									// ensure fires are done

v[1] = V( "_L2d" ) - COUNT( "Wrk2" ) * VS( LABSUPL2, "Lscale" );

RESULT( max( ceil( v[1] * ( 1 + VS( LABSUPL2, "theta" ) ) ), 0 ) )


EQUATION( "_K2" )
/*
Capital (in machine-number terms) employed by firm in consumption-good industry
Updates '_old2vint'
*/

V( "_CI2" );									// ensure cancel investment done

v[2] = V( "_SI2" );								// substitution investment
v[3] = V( "_EI2" );								// expansion investment

if ( v[2] + v[3] > 0 )							// new machines delivered?
	CFUN( add_vintage, v[2] + v[3], false );	// create new vintage

v[4] = max( VL( "_K2", 1 ) + v[3] - V( "_K2d" ), 0 );// machines to remove from K

h = VNT( T, 0 );								// default vintage ID
j = T + 1;										// oldest vintage so far
CYCLE_SAFE( cur, "Vint" )						// search from older vintages
{
	v[5] = VS( cur, "_RSvint" );				// number of machines to scrap

	if ( v[5] < 0 )								// end-of-life vintage to scrap?
	{
		v[5] = - v[5];							// absolute vintage size

		if ( v[4] > 0 )							// yet capital to shrink?
		{
			if ( v[5] > v[4] )					// more than needed?
			{
				v[5] -= v[4];					// just reduce vintage
				v[4] = 0;						// shrinkage done
				WRITES( cur, "_nVint", v[5] );
			}
			else								// scrap entire vintage
			{
				if ( CFUNS( cur, scrap_vintage ) >= 0 )// not last vintage?
				{
					v[4] -= v[5];
					continue;					// don't consider for old vint.
				}
				else
				{
					v[5] = 0;					// last: nothing else to scrap
					v[4] -= v[5] - 1;
				}
			}
		}
	}

	// something yet to scrap and substitution to be done?
	if ( v[5] > 0 && v[2] > 0 )
	{
		if ( v[5] > v[2] )						// more than needed?
		{
			v[5] -= v[2];						// just reduce vintage
			v[2] = 0;							// substitution done
			WRITES( cur, "_nVint", v[5] );
		}
		else									// scrap entire vintage
		{
			if ( CFUNS( cur, scrap_vintage ) >= 0 )// not last vintage?
			{
				v[2] -= v[5];
				continue;						// don't consider for old vint.
			}
		}
	}

	i = VS( cur, "_tVint" );					// time of vintage install
	if ( i < j )								// oldest so far?
	{
		j = i;
		h = VS( cur, "_IDvint" );				// save oldest
	}
}

WRITE( "_old2vint", h );

RESULT( SUM( "_nVint" ) )


EQUATION( "_K2nom" )
/*
Nominal capital book value (in money terms) of firm in consumption-good industry
*/
RESULT( WHTAVE( "_nVint", "_pVint" ) )


EQUATION( "_L2" )
/*
Effective (absolute) number of workers for firm in consumption-good industry
Result is scaled according to the defined scale
*/
VS( LABSUPL2, "hires" );						// make sure hiring done
RESULT( COUNT( "Wrk2" ) * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_L2d" )
/*
Labor demand of firm in consumption-good industry
*/
RESULT( V( "_life2cycle" ) > 0 ? ceil( V( "_Q2" ) / ( VS( PARENT, "m2" ) *
									   V( "_A2" ) / VS( PARENT, "k2" ) ) ) : 0 )


EQUATION( "_L2v" )
/*
Net vacancy rate of labor (unfilled positions over labor employed) for firm in
consumption-good industry
*/

v[1] = V( "_L2" );								// current number of workers
v[2] = V( "_JO2" );								// current open positions

if( v[1] == 0 )									// firm has no worker?
	v[0] = ( v[2] == 0 ) ? 0 : 1;				// handle limit case
else
	v[0] = min( v[2] / v[1], 1 );				// or calculate the regular way

RESULT( v[0] )


EQUATION( "_N2" )
/*
Inventories (unsold output units) of firm in consumption-good industry
*/

VS( PARENT, "D2" );								// ensure $ demand is allocated

v[0] = CURRENT + V( "_Q2e" ) - V( "_D2" ) / V( "_p2" );

RESULT( ROUND( v[0], 0, 0.001 ) )				// avoid rounding errors on zero


EQUATION( "_Pi2rate" )
/*
Profit rate (profits over capital) of firm in consumption-good industry
*/
// ignore no or minimum capital firms
RESULT( VL( "_K2", 1 ) > 1 ? V( "_Pi2" ) / VL( "_K2nom", 1 ) : 0 )


EQUATION( "_Pi2" )
/*
Profit of firm in consumption-good industry
*/
RESULT( V( "_S2" ) - V( "_W2" ) - V( "_i2net" ) )// firm profits before taxes


EQUATION( "_Q2e" )
/*
Effective output of firm in consumption-good industry
*/
RESULT( min( V( "_Q2" ), V( "_Q2p" ) ) )


EQUATION( "_Q2p" )
/*
Potential production with current machines and workers for a firm in
consumption-good industry
*/
RESULT( SUM( "_Qvint" ) )


EQUATION( "_Q2pe" )
/*
Expected potential production with remaining workers for a firm in
consumption-good industry
*/

v[0] = 0;										// accumulator
CYCLE( cur, "Wrk2" )
	v[0] += VLS( SHOOKS( cur ), "_Q", 1 );		// add previous production

RESULT( v[0] * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_Q2u" )
/*
Capacity utilization for a firm in consumption-good industry
*/
v[1] = VL( "_K2", 1 ) * VS( PARENT, "m2" ) / VS( PARENT, "k2" );
RESULT( v[1] > 0 ? V( "_Q2e" ) / v[1] : VLS( PARENT, "Q2u", 1 ) )


EQUATION( "_S2" )
/*
Sales (in money terms) of firm in consumption-good industry
*/
VS( PARENT, "D2" );								// ensure $ demand is allocated
RESULT( V( "_D2" ) )


EQUATION( "_SI2d" )
/*
Desired substitution investment (in machine-number terms) of firm in
consumption-good industry
*/

v[2] = 0;										// scrapped machine accumulator
CYCLE( cur1, "Vint" )							// search last vintage to scrap
{
	v[3] = VS( cur1, "_RSvint" );				// number of machines to scrap

	if ( v[3] == 0 )							// nothing else to do
		break;

	v[2] += abs( v[3] );						// accumulate vintage
}

// capital shrinkage desired, if any (do not substitute if unneeded)
RESULT( max( v[2] - max( VL( "_K2", 1 ) - V( "_K2d" ), 0 ), 0 ) )


EQUATION( "_W2" )
/*
Total wages paid by firm in consumption-good industry
*/

V( "_L2" );										// ensure hiring is done

v[0] = 0;										// wage accumulator
CYCLE( cur, "Wrk2" )
	v[0] += VS( SHOOKS( cur ), "_w" );			// adding wages

RESULT( v[0] * VS( LABSUPL2, "Lscale" ) )		// consider labor scaling


EQUATION( "_c2e" )
/*
Effective average output unit cost of firm in consumption-good industry
Use expected cost if firm is not producing
*/
v[1] = V( "_Q2e" );
RESULT( v[1] > 0 ? V( "_W2" ) / v[1] : V( "_c2" ) )


EQUATION( "_dA2b" )
/*
Notional productivity (bounded) rate of change of firm in
consumption-good industry
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "_A2", VS( GRANDPARENT, "mLim" ) ) )


EQUATION( "_dN2nom" )
/*
Change in firm's nominal (in money terms) inventories for a firm in
consumption-good industry
*/
RESULT( V( "_p2" ) * V( "_N2" ) - VL( "_p2", 1 ) * VL( "_N2", 1 ) )


EQUATION( "_dQ2d" )
/*
Desired production change (absolute) for a firm in consumption-good industry
*/
RESULT( V( "_Q2d" ) - VL( "_Q2e", 1 ) )


EQUATION( "_i2net" )
/*
Net interest expenses (income) by firm in consumption-good industry
*/

v[1] = VLS( FINSECL2, "rDeb", 1 ) * ( 1 + ( VL( "_qc2", 1 ) - 1 ) *
	   VS( FINSECL2, "kConst" ) ); 				// effective debt interest rate
v[0] = v[1] * VL( "_Deb2", 1 );					// interest to pay
v[0] -= VS( FINSECL2, "rD" ) * VL( "_NW2", 1 );	// less financial income

RESULT( v[0] )


EQUATION( "_life2cycle" )
/*
Stage in life cycle of firm in consumer-good industry:
0 = pre-operational entrant firm
1 = operating entrant firm (first period producing)
2.x = operating entrant firm (x=2nd-4th period producing)
3 = incumbent firm
4 = exiting firm
*/

switch( ( int ) CURRENT )
{
	case 0:										// pre-operational entrant
		if ( VL( "_K2", 1 ) > 0 )				// firm has capital available?
			v[0] = 1;							// turn into operational entrant
		else
			v[0] = 0;							// keep as pre-op. entrant
		break;

	case 1:										// operating entrant
		v[0] = 2.2;								// turn into running entrant
		break;

	case 2:										// running entrant
		v[0] = CURRENT + 0.1;
		if ( v[0] > 2.4001 )					// handle rounding error
			v[0] = 3;							// turn into incumbent
		break;

	default:									// incumbent/exiting
		v[0] = CURRENT;							// keep as it is
		PARAMETER;								// but no longer compute
}

RESULT( v[0] )


EQUATION( "_quits2" )
/*
Number of workers quitting jobs (not fired) in period for firm in
consumption-good industry
Updated in 'hires1' and 'hires'
*/

if ( VS( GRANDPARENT, "flagGovExp" ) < 2 )		// unemployment benefit exists?
	END_EQUATION( 0 );

v[1] = VS( LABSUPL2, "wU" );					// unemployment benefit in t

h = 0;
CYCLE_SAFE( cur, "Wrk2" )
	if ( VS( SHOOKS( cur ), "_w" ) <= v[1] )	// under unemp. benefit?
	{
		CFUNS( SHOOKS( cur ), fire_worker );
		++h;
	}

RESULT( h * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_retires2" )
/*
Number of workers retiring from jobs (not fired) in period for firm in
consumption-good industry
*/

if ( VS( LABSUPL2, "Tr" ) == 0 )				// retirement disabled?
	END_EQUATION( 0 )

h = 0;
CYCLE_SAFE( cur, "Wrk2" )
	if ( VS( SHOOKS( cur ), "_age" ) == 1 )		// is a "reborn"?
	{
		CFUNS( SHOOKS( cur ), fire_worker );
		++h;
	}

RESULT( h * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_sT2min" )
/*
Minimum workers tenure skills of a firm in consumption-good industry
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 1 )							// no learning by tenure mode?
	END_EQUATION( INISKILL );

v[0] = DBL_MAX;									// current minimum
CYCLE( cur, "Wrk2" )
{
	v[1] = VS( SHOOKS( cur ), "_sT" );

	if ( v[1] < v[0] )
		v[0] = v[1];							// keep minimum
}

RESULT( v[0] < DBL_MAX ? v[0] : VS( LABSUPL2, "sTmin" ) )// handle no worker case


EQUATION( "_s2avg" )
/*
Weighted average workers compound skills of a firm in consumption-good industry
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) == 0 )	// no learning ?
	END_EQUATION( INISKILL );

v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_s" );
	++i;
}

RESULT( i > 0 ? v[0] / i : VS( LABSUPL2, "sAvg" ) )// handle no worker case


EQUATION( "_w2avg" )
/*
Average wage paid by firm in consumption-good industry
*/

v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_w" );
	++i;
}

RESULT( i > 0 ? v[0] / i : VS( LABSUPL2, "wAvg" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "_cred2" )
/*
Bank credit available (new debt) to firm in consumer-good industry
Function called multiple times in single time step
*/

v[1] = V( "_Deb2" );							// current firm debt
v[2] = V( "_Deb2max" );							// maximum prudential credit

if ( v[2] > v[1] )								// more credit possible?
{
	v[0] = v[2] - v[1];							// potential free credit

	cur = HOOK( BANK );							// firm's bank
	v[3] = VS( cur, "_TC2free" );				// bank's available credit

	if ( v[3] > -0.1 )							// credit limit active
		v[0] = min( v[0], v[3] );				// take just what is possible
}
else
	v[0] = 0;									// no credit available

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_A2", "" )
/*
Average (single-stage) labor productivity of firm in consumption-good industry
Machine-level productivity is notional (single production stage) and
DO NOT directly represent firm final labor productivity
Updated in '_c2'
*/

EQUATION_DUMMY( "_A2p", "_c2" )
/*
Average potential (single-stage) labor productivity of firm in
consumption-good industry
Machine-level productivity is notional (single production stage) and
DO NOT directly represent firm final labor productivity
Updated in '_c2'
*/

EQUATION_DUMMY( "_Bon2", "_Tax2" )
/*
Bonus paid by firm in consumption-good industry
Updated in '_Tax2'
*/

EQUATION_DUMMY( "_D2", "D2" )
/*
Demand (in money terms) fulfilled by firm in consumption-good industry
Updated in 'D2'
*/

EQUATION_DUMMY( "_Deb2", "" )
/*
Stock of bank debt of firm in consumption-good industry
Updated in '_Q2', '_EI2', '_SI2', '_Pi2' and '_Tax2'
*/

EQUATION_DUMMY( "_Div2", "_Tax2" )
/*
Dividends paid by firm in consumption-good industry
Updated in '_Tax2'
*/

EQUATION_DUMMY( "_NW2", "" )
/*
Net wealth (free cash) of firm in consumption-good industry
Updated in '_Q2', '_EI2', '_SI2' and '_Tax2'
*/

EQUATION_DUMMY( "_cred2c", "" )
/*
Credit constraint for firm in consumption-good industry
Updated in '_Deb2max', '_Q2', '_EI2' and '_SI2'
*/

EQUATION_DUMMY( "_hires2", "hires" )
/*
Effective number of workers hired in period for firm in consumption-good industry
Updated in 'hires'
*/

EQUATION_DUMMY( "_l2", "D2" )
/*
Unfilled demand (in unit terms) of firm in consumption-good sector
Updated in 'D2'
*/

EQUATION_DUMMY( "_old2vint", "_K2" )
/*
Oldest vintage (ID) in use in period by firm in consumption-good industry
Updated in '_K2'
*/

EQUATION_DUMMY( "_qc2", "" )
/*
Credit class of firm in consumption-good industry (1,2,3,4)
Updated in 'cScores'
*/
