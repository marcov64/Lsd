/******************************************************************************

	FIRM2 OBJECT EQUATIONS
	----------------------

	Equations that are specific to the Firm2 objects in the K+S LSD model 
	are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_Deb2max" )
/*
Prudential maximum bank debt of firm in consumer-good sector
*/

// maximum debt allowed to firm, considering net worth and operating margin
v[5] = VS( FINSECL2, "Lambda" ) * max( VL( "_NW2", 1 ), 
									   VL( "_S2", 1 ) - VL( "_W2", 1 ) );
		   
// apply an absolute floor to maximum debt prudential limit
v[0] = max( v[5], VS( FINSECL2, "Lambda0" ) * VLS( CAPSECL2, "PPI", 1 ) / 
				  VS( CAPSECL2, "PPI0" ) );

WRITE( "_cred2c", 0 );							// reset constraint for period

RESULT( v[0] )
		

EQUATION( "_D2e" )
/*
Adaptive demand expectation of firm in consumer-good sector
*/

if ( V( "_life2cycle" ) < 3 )					// entrant?
	// myopic-optimistic expectations
	END_EQUATION( max( VL( "_D2d", 1 ), CURRENT ) );

v[10] = VS( PARENT, "e0" );						// animal spirits parameter

// compute the 4-period demand moving average, balanced between then
// effective fulfilled demand and the consumer potential demand (orders)
for ( i = 1; i <= 4; ++i )
	v[ i ] = ( 1 - v[10] ) * VL( "_D2", i ) + v[10] * VL( "_D2d", i );

switch ( ( int ) VS( GRANDPARENT, "flagExpect" ) )
{
	//  myopic expectations with 1-period memory
	case 0:
	default:
		v[0] = v[1];
		break;

	// myopic expectations with up to 4-period memory
	case 1:		
		v[11] = VS( PARENT, "e1" );				// weight of t-1 demand
		v[12] = VS( PARENT, "e2" );				// weight of t-2 demand
		v[13] = VS( PARENT, "e3" );				// weight of t-3 demand
		v[14] = VS( PARENT, "e4" );				// weight of t-4 demand
		
		for ( v[5] = v[6] = 0, i = 1; i <= 4; ++i )
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
				 VS( PARENT, "e8" ) * VLS( GRANDPARENT, "dGDP", 1 ) ) * v[1];
		break;
}

RESULT( v[0] )


EQUATION( "_E" )
/*
Effective competitiveness of a firm in sector 2, considering the price, 
unfilled demand and the quality of the product for the consumer
*/

if ( V( "_life2cycle" ) == 0 )
	END_EQUATION( VLS( PARENT, "Eavg", 1 ) );	// non-producing entrant

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

v[0] = ( v[7] * ( 1 - v[10] ) + v[8] * ( 1 - v[11] ) + v[9] * v[12] ) / 
	   ( v[7] + v[8] + v[9] );

RESULT( v[0] )


EQUATION( "_EI" )
/*
Effective expansion investment of firm in consumption-good sector
*/

V( "_Q2" );										// make sure production decided
V( "_supplier" );								// ensure supplier is selected
cur = SHOOKS( HOOK( SUPPL ) )->up;				// pointer to new supplier

v[1] = V( "_EId" );								// desired expansion investment

if ( v[1] == 0 )
	END_EQUATION( 0 );							// nothing to do

v[2] = V( "_cred2" );							// available credit
v[3] = V( "_NW2" );								// net worth (cash available)
v[4] = VS( PARENT, "m2" );						// machine output per period
v[5] = VS( cur, "_p1" );						// new machine price

v[6] = v[5] * v[1] / v[4];						// expansion investment cost

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
		v[0] = max( floor( ( v[3] - 1 + v[2] ) / v[5] ) * v[4], 0 );
		
		if ( v[0] == 0 )
			END_EQUATION( 0 );					// nothing to do

		v[7] = v[2];							// take all credit available
		v[8] = v[6] - v[3] + 1;					// desired credit
		v[3] = 1;								// keep minimum cash
	}
	
	update_debt2( p, v[8], v[7] );				// update firm debt
}

WRITE( "_NW2", v[3] );							// update the firm net worth

send_order( p, floor( v[0] / v[4] ) );			// send order to machine supplier

RESULT( v[0] )


EQUATION( "_EId" )
/*
Desired expansion investment of firm in consumption-good sector
*/

v[1] = V( "_Kd" );								// desired capital
v[2] = VL( "_K", 1 );							// available capital stock
v[3] = VS( PARENT, "m2" );						// machine output per period

if ( v[2] < v[3] )								// no capital yet?
	END_EQUATION( v[1] );						// no growth threshold

// min rounded capital
v[4] = ceil( ( 1 + VS( PARENT, "kappaMin" ) ) * v[2] / v[3] ) * v[3];

if ( v[1] > v[4] )								// minimum capital reached?
{
	// max rounded capital
	v[6] = ceil( ( 1 + VS( PARENT, "kappaMax" ) ) * v[2] / v[3] ) * v[3];
	
	v[7] = min( v[1], v[6] );					// cap desired capital, if needed
	v[0] = max( ceil( ( v[7] - v[2] ) / v[3] ) * v[3], 0 );// rounded investment 
}
else
	v[0] = 0;									// no expansion investment

RESULT( v[0] )


EQUATION( "_Kd" )
/*
Desired capital stock of firm in consumption-good sector
*/

if ( V( "_life2cycle" ) == 0 )					// if fresh entrant
	END_EQUATION( CURRENT );					// keep initially desired capital

v[1] = VS( PARENT, "m2" );						// machine output per period

// desired capacity with slack and utilization, based on expectations/inventories
v[2] = max( ( 1 + VS( PARENT, "iota" ) ) * V( "_D2e" ) - VL( "_N", 1 ), 0 ) / 
	   VS( PARENT, "u" );

// desired capital with slack, round up to m2 units
RESULT( ceil( v[2] / v[1] ) * v[1] )


EQUATION( "_Q2" )
/*
Planned production for a firm in consumption-good sector
*/

v[1] = V( "_Q2d" );								// desired production

if ( v[1] == 0 )
	END_EQUATION( 0 );							// nothing to do

v[2] = V( "_cred2" );							// available credit
v[3] = VL( "_NW2", 1 );							// net worth (cash available)
v[4] = V( "_c2" );								// expected unit cost
v[5] = VS( PARENT, "m2" );						// machine output per period

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
		v[0] = floor( v[0] / v[5] ) * v[5];		// round to the # of machines
		
		if ( v[0] == 0 )
			END_EQUATION( 0 );					// nothing to do

		v[7] = v[2];							// take all credit available
		v[8] = v[6] - v[3] + 1;					// desired credit
		v[3] = 1;								// keep minimum cash
	}
	
	update_debt2( p, v[8], v[7] );				// update firm debt
}

// provision for wage expenses
WRITE( "_NW2", v[3] );							// update the firm net worth

RESULT( v[0] )


EQUATION( "_Q2d" )
/*
Desired output of firm in consumption-good sector
*/

if ( V( "_life2cycle" ) == 0 )					// if fresh entrant
	END_EQUATION( 0 );							// not yet producing

// desired production with slack, based on expectations, considering inventories
v[1] = max( ( 1 + VS( PARENT, "iota" ) ) * V( "_D2e" ) - VL( "_N", 1 ), 0 );

// limited to the available capital stock
RESULT( floor( min( v[1], VL( "_K", 1 ) ) ) )


EQUATION( "_SI" )
/*
Effective substitution investment of firm in consumption-good sector
*/

V( "_EI" );										// make sure expansion done

v[1] = V( "_SId" );								// desired substitution invest.

if ( v[1] == 0 )
	END_EQUATION( 0 );							// nothing to do

v[2] = V( "_cred2" );							// available credit
v[3] = V( "_NW2" );								// net worth (cash available)
v[4] = VS( PARENT, "m2" );						// machine output per period
v[5] = VS( SHOOKS( HOOK( SUPPL ) )->up, "_p1" );// new machine price

v[6] = v[5] * v[1] / v[4];						// substitution investment cost

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
		// invest as much as the available finance allows, rounded to # machines
		v[0] = max( floor( ( v[3] - 1 + v[2] ) / v[5] ) * v[4], 0 );
		
		if ( v[0] == 0 )
			END_EQUATION( 0 );					// nothing to do

		v[7] = v[2];							// take all credit available
		v[8] = v[6] - v[3] + 1;					// desired credit
		v[3] = 1;								// keep minimum cash
	}
	
	update_debt2( p, v[8], v[7] );				// update firm debt
}

WRITE( "_NW2", v[3] );							// update the firm net worth

send_order( p, floor( v[0] / v[4] ) );			// send order to machine supplier

RESULT( v[0] )


EQUATION( "_Tax2" )
/*
Tax paid by firm in consumption-good sector
Also updates final net wealth on period
*/

v[1] = V( "_Pi2" );								// firm profit in period
v[2] = VS( GRANDPARENT, "tr" );					// tax rate

if ( v[1] > 0 )									// profits?
{
	v[5] = VL( "_K", 1 );						// available capital in period
	
	v[0] = v[1] * v[2];							// tax to government
	
	// pay bonus only if firm has above-average profit rate and workers employed
	if ( v[5] > 0 && v[1] / v[5] > VLS( PARENT, "Pi2rateAvg", 1 ) &&
		 V( "_L2" ) > 0 ) 
		v[3] = VS( LABSUPL2, "psi6" ) * ( v[1] - v[0] );
	else
		v[3] = 0;
	
	v[4] = VS( PARENT, "d2" ) * ( v[1] - v[0] - v[3] );// shareholders dividend 
}
else
	v[0] = v[3] = v[4] = 0;						// no tax/bonus/divid. on losses

WRITE( "_B2", v[3] );							// save period bonus
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
			update_debt2( p, v[9], v[9] );		// finance all
			WRITE( "_NW2", 1 );					// minimum net wealth
		}
		else
		{
			update_debt2( p, v[8], v[8] );		// take what is possible
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
			update_debt2( p, 0, - v[10] );		// zero debt
			INCR( "_NW2", v[6] - v[10] );		// save the rest
		}
		else
			update_debt2( p, 0, - v[6] );		// repay part of debt
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
		
		move_worker( SHOOKS( cur1 ), cur, vint_learn );// move worker to vintage
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
		move_worker( SHOOKS( cur2 ), cur, vint_learn );// move worker to vintage
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
Planned average unit cost of firm in consumption-good sector
Also updates '_A2', '_A2p'
*/

v[1] = V( "_Q2d" );								// desired production
v[2] = VL( "_K", 1 );							// available capital stock
v[3] = VS( PARENT, "m2" );						// machine output per period
v[4] = VL( "_w2avg", 1 );						// average firm wage

if ( v[1] == 0 )								// nothing to produce?
	v[1] = v[2];								// just compute for all vintages

v[5] = max( floor( v[2] / v[3] ) - ceil( v[1] / v[3] ), 0 );// unused machines

v[0] = v[6] = v[7] = v[8] = 0;					// accumulators
CYCLE( cur, "Vint" )							// choose vintages to use
{
	v[9] = VS( cur, "_nVint" );					// number of machines in vintage
	
	if ( v[5] >= v[9] )							// none to be used in the vint.?
	{
		v[5] -= v[9];							// less machines not to use
		WRITES( cur, "_toUseVint", 0 );			// no machine to use for vint.
		continue;								// next vintage
	}
	
	v[6] += v[10] = v[9] - v[5];				// add to be used machines
	v[11] = VS( cur, "_Avint" );				// vintage notional productivity
	v[12] = VLS( cur, "_AeVint", 1 );			// vintage effective product.
	v[7] += v[10] * v[11];						// add notional productivities
	v[8] += v[10] * v[12];						// add effective productivities
	v[0] += v[10] * v[4] / v[12];				// add used machines cost

	v[5] = 0;									// no more machine not to use
	WRITES( cur, "_toUseVint", v[10] );			// number mach. to try to use
}

if ( v[6] == 0 )								// no machine?
{
	V( "_supplier" );							// ensure supplier is selected
	cur1 = SHOOKS( HOOK( SUPPL ) )->up;			// pointer to supplier
	
	v[6] = 1;									// 1 notional machine
	v[7] = v[8] = VS( cur1, "_Atau" );			// new machines productivity
	v[0] = v[4] / v[7];							// machine unit cost
}

WRITE( "_A2p", v[7] / v[6] );					// potential notional product.
WRITE( "_A2", v[8] / v[6] );					// expected productivity

RESULT( v[0] / v[6] )


EQUATION( "_f2" )
/*
Market share of firm in consumption-good sector
It is computed using a replicator equation over the relative competitiveness 
of the firm
Because of entrants, market shares may add-up to more than one, so 'f2rescale'
must be used before '_f2' is used
*/

v[1] = VS( PARENT, "f2min" );						// minimum share to stay

switch( ( int ) V( "_life2cycle" ) )				// entrant firm state
{
	case 0:											// non-producing entrant
	default:										// exiting incumbent
		END_EQUATION( 0 );

	case 1:											// first-period entrant
		// fair share lower-bounded slightly above exit threshold
		END_EQUATION( max( 1 / VLS( PARENT, "F2", 1 ), 1.01 * v[1] ) )
		
	case 2:											// 2nd-4th-period entrant
		// replicator equation					
		v[0] = VL( "_f2", 1 ) * ( 1 + VS( PARENT, "chi" ) * 
								  ( V( "_E" ) / VS( PARENT, "Eavg" ) - 1 ) );
								  
		// ensure entrant stay in the market for 4 periods
		if ( v[0] < v[1] )
			v[0] = 1.01 * v[1];
		break;
		
	case 3:											// incumbent
		// replicator equation					
		v[0] = VL( "_f2", 1 ) * ( 1 + VS( PARENT, "chi" ) * 
								  ( V( "_E" ) / VS( PARENT, "Eavg" ) - 1 ) );
}

// lower-bounded slightly below (multi-period) exit threshold
// to ensure firm sells production before leaving the market
RESULT( max( v[0], 0.99 * v[1] / VS( PARENT, "n2" ) ) )


EQUATION( "_fires2" )
/*
Number of workers fired by firm in consumption-good sector
Process required firing using the appropriate rule
*/

if ( V( "_life2cycle" ) == 0 )					// entrant firm?
	END_EQUATION( 0 );
	
v[1] = max( V( "_Q2pe" ) - V( "_Q2d" ), 0 );	// expected extra capacity

// check if would fire too many workers because of scaling (# of "modules")
v[2] = VS( LABSUPL2, "Lscale" ) * V( "_A2" );	// prod.-adjusted module size
v[1] = floor( v[1] / v[2] ) * v[2];				// rounded down capacity shrink

// pick the appropriate firing rule
int fRule = V( "_postChg" ) ? VS( GRANDPARENT, "flagFireRuleChg" ) : 
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
			v[0] = fire_workers( p, MODE_ADJ, v[1], &v[2] );
		else
			v[0] = 0;
		break;
		
	case 3:										// only fire if firm at losses
		// production being reduced and extra capacity is expected?
		if ( VL( "_Pi2", 1 ) < 0 && v[1] > 0 )	// workers have to be fired?
			v[0] = fire_workers( p, MODE_ADJ, v[1], &v[2] );
		else
			v[0] = 0;
		break;
		
	case 4:										// fire if payback is achieved
		// fire insufficient payback workers
		v[0] = fire_workers( p, MODE_PBACK, v[1], &v[2] );
		break;
		
	case 5:										// fire when contract ends
		// fire all workers with finished contracts
		v[0] = fire_workers( p, MODE_ALL, v[1], &v[2] );
		break;
		
	case 6:										// reg. 5 until t=T, then reg. Y
		// fire non needed, non stable workers
		v[0] = fire_workers( p, MODE_IPROT, v[1], &v[2] );
}

RESULT( v[0] )


EQUATION( "_mu2" )
/*
Mark-up of firm in consumption-good sector
*/

v[1] = VL( "_f2", 1 );							// past periods market shares
v[2] = VL( "_f2", 2 );
v[3] = VS( PARENT, "f2min" );					// market exit share threshold
v[4] = VL( "_Q2e", 1 );							// past periods production
v[5] = VL( "_Q2e", 2 );
v[6] = VS( GRANDPARENT, "mLim" );				// absolute change limit

// update mark-up only if not exiting and only under normal competition and 
// operating conditions
if ( v[1] > v[3] && v[2] > v[3] && v[4] > 0 && v[5] > 0 && 
	 ( ( v[1] > v[2] && v[4] > v[5] ) || ( v[1] < v[2] && v[4] < v[5] ) ) )
{
	v[7] = VS( PARENT, "upsilon" ) * ( ( v[1] - v[2] ) / v[2] );
	
	if ( v[6] > 0 )
		v[7] = max( min( v[7], v[6] ), - v[6] );// apply bounds to change
}
else
	v[7] = 0;

RESULT( CURRENT * ( 1 + v[7] ) )


EQUATION( "_q2" )
/*
Product quality of a firm in consumption-good sector
Equal to the average of the log tenure skills of workers
*/

if ( V( "_life2cycle" ) == 0 )
	END_EQUATION( VLS( PARENT, "q2avg", 1 ) );

v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_sT" );
	++i;
}

RESULT( i > 0 ? v[0] / i : 1 )


EQUATION( "_supplier" )
/*
Selected machine supplier by firm in consumption-good sector
Also set firm 'hook' pointers to supplier firm object
*/

VS( CAPSECL2, "inn" );							// ensure innovation is done and
												// brochures distributed
v[1] = DBL_MAX;									// supplier price/cost ratio
i = 0;
cur2 = cur3 = NULL;
CYCLE( cur, "Broch" )							// use brochures to find supplier
{
	cur1 = SHOOKS( cur )->up;					// pointer to supplier object	
	
	// compare price to productivity ratios
	v[2] = VS( cur1, "_p1" ) / VS( cur1, "_Atau" );
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
	WRITES( cur2, "_tSel", t );					// update selection time
else											// no brochure received
{
	cur1 = RNDDRAW_FAIRS( CAPSECL2, "Firm1" );	// draw new supplier
	i = VS( cur1, "_ID1" );
	
	cur2 = ADDOBJS( cur1, "Cli" );				// add object to new supplier
	WRITES( cur2, "_IDc", V( "_ID2" ) );		// update client ID
	WRITES( cur2, "_tSel", t );					// update selection time
	
	cur3 = ADDOBJ( "Broch" );					// add brochure
	WRITES( cur3, "_IDs", i );					// update supplier ID
	WRITE_SHOOKS( cur3, cur2 );					// pointer to supplier client list
	WRITE_SHOOKS( cur2, cur3 );					// pointer to client brochure list
}

WRITE_HOOK( SUPPL, cur3 );						// pointer to current brochure

RESULT( i )


EQUATION( "_w2o" )
/*
Wage offer to workers in queue of firm in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagHeterWage" ) == 0 )	// centralized wage setting?
{
	v[0] = VS( LABSUPL2, "wCent" );				// single wage centrally defined
	goto end_offer;
}

h = V( "_life2cycle" );							// firm status
k = V( "_postChg" ) ? VS( GRANDPARENT, "flagWageOfferChg" ) : 
					  VS( GRANDPARENT, "flagWageOffer" );
					  
if ( k == 0 )									// wage premium mode?
{
	if ( h == 0 )								// if entrant
		v[0] = VLS( PARENT, "w2oAvg", 1 );		// use market average as base
	else
		v[0] = CURRENT;
	
	switch ( ( int ) VS( GRANDPARENT, "flagWagePremium" ) )
	{											// define wage premium type
		case 0:									// no premium
		default:
			break;
		
		case 1:									// indexed premium (WP1)
			v[1] = VS( LABSUPL2, "psi1" );		// inflation adjust. parameter
			v[2] = VS( LABSUPL2, "psi2" );		// general prod. adjust. param.
			v[3] = VS( LABSUPL2, "psi3" );		// unemploym. adjust. parameter
			v[4] = VS( LABSUPL2, "psi4" );		// firm prod. adjust. parameter
			v[11] = VS( LABSUPL2, "psi5" );		// firm vacancy booster param.
			v[5] = VLS( PARENT, "dCPIb", 1 );	// inflation variation
			v[6] = VLS( GRANDPARENT, "dAb", 1 );// general productivity var.
			v[7] = VLS( LABSUPL2, "dUeB", 1 );	// unemployment variation
			
			// notional productivity variation (firm), consider entrants
			v[8] = ( h == 0 ) ? 0 : VL( "_dA2b", 1 );
			
			// negative product. floor?
			if ( VS( GRANDPARENT, "flagNegProdWage" ) == 1 )
				v[8] = max( 0, v[8] );			// apply limit if enabled
			v[12] = VL( "_L2vac", 1 );			// previous vacancy rate
			
			// make sure total productivity effect is bounded to 1
			if ( ( v[2] + v[4] ) > 1 )
				v[2] = max( 1 - v[4], 0 );		// adjust general prod. effect

			v[0] *= 1 + v[1] * v[5] + v[2] * v[6] + v[3] * v[7] + 
					v[4] * v[8] + v[11] * v[12];
			break;

		case 2:									// endogenous mechanism (WP2)
			v[9] = VLS( PARENT, "w2oAvg", 1 );
			if ( v[0] != 0 )					// valid  offer last period?
				v[0] *= 1 + max( v[9] / v[0] - 1, 0 );
			else								// no: use market average
				v[0] = VLS( PARENT, "w2oAvg", 1 );
	}
}
else		
{												// lowest wage mode
	VS( LABSUPL2, "appl" );						// ensure applications are done
	j = ceil( ( V( "_L2d" ) - VL( "_L2", 1 ) ) * // number of workers (scaled)
			  ( 1 + VS( LABSUPL2, "theta" ) ) / VS( LABSUPL2, "Lscale" ) );
	j = max( j, 1 );							// minimum one worker for calc.
												
	// sort firm's candidate list according to the defined strategy
	int hOrder = V( "_postChg" ) ? VS( GRANDPARENT, "flagHireOrder2Chg" ) : 
								   VS( GRANDPARENT, "flagHireOrder2" );
	order_applications( hOrder, & V_EXT( firm2, appl ) );

	// search applications set (increasing wage requests) for enough workers
	i = 0;									// workers counter
	v[0] = 0;								// highest wage found
	CYCLE_EXT( its, firm2, appl )			// run over enough applications
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
	v[10] = VL( "_p2", 1 ) * VL( "_A2", 1 );	
												// max wage for minimum markup
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
woData.firm = p;
EXEC_EXTS( GRANDPARENT, country, firm2wo, push_back, woData );

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_CI" )
/*
Canceled investment of firm in consumption-good sector
*/

cur = HOOK( SUPPL );							// pointer to current supplier
VS( SHOOKS( cur )->up, "_Q1e" );				// make sure supplier produced

v[1] = VS( SHOOKS( cur ), "_nCan" );			// canceled machine number
k = VS( SHOOKS( cur ), "_tOrd" );				// time of canceled order

if ( k == t && v[1] > 0 )
{	
	v[2] = VS( PARENT, "m2" );					// machine output per period
	v[3] = V( "_SI" ) / v[2];					// machines to substitute
	v[4] = V( "_EI" ) / v[2];					// machines to expand
	v[5] = VS( SHOOKS( cur )->up, "_p1" );		// machine price
	
	if ( v[1] > v[3] )							// no space for substitution?
	{
		WRITE( "_SI", 0 );						// reset substitution investment
		WRITE( "_EI", ( v[4] - v[1] + v[3] ) * v[2] );// remaining expansion inv.
	}
	else
		WRITE( "_SI", ( v[3] - v[1] ) * v[2] );	// shrink substitution investm.

	INCR( "_NW2", v[5] * v[1] );				// recover paid machines value
	
	v[0] = v[1] * v[2];							// canceled investment
}
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "_D2d" )
/*
Desired (potential) demand for firm in consumption-good sector
*/
RESULT( VS( PARENT, "D2d" ) * V( "_f2" ) )


EQUATION( "_JO2" )
/*
Open job positions for a firm in consumption-good sector
*/

VS( CAPSECL2, "hires1" );						// ensure sector 1 is done
V( "_fires2" );									// and fires are also done

v[1] = V( "_L2d" ) - COUNT( "Wrk2" ) * VS( LABSUPL2, "Lscale" );

RESULT( max( ceil( v[1] * ( 1 + VS( LABSUPL2, "theta" ) ) ), 0 ) )


EQUATION( "_K" )
/*
Capital employed by firm in consumption-good sector
*/

V( "_CI" );										// ensure cancel investment done

v[1] = VS( PARENT, "m2" );						// machine output per period
v[2] = V( "_SI" );								// substitution investment
v[3] = V( "_EI" );								// expansion investment

// new machines delivered only if supplier has not canceled the order
if ( v[2] + v[3] > 0 )
{
	v[4] = floor( ( v[2] + v[3] ) / v[1] );		// total number of new machines

	if ( v[4] > 0 )								// new machines to install?
		add_vintage( p, v[4] );					// create new vintage
}

v[5] = max( VL( "_K", 1 ) + v[3] - V( "_Kd" ), 0 );// desired capital shrinkage
v[6] = floor( v[5] / v[1] );					// machines to remove from K

v[7] = floor( v[2] / v[1] );					// machines to substitute in K

j = t + 1;										// oldest vintage so far
h = 0;											// oldest vintage ID
CYCLE_SAFE( cur, "Vint" )						// search from older vintages
{
	v[8] = VS( cur, "_RS" );					// number of machines to scrap
	
	if ( v[8] < 0 )								// end-of-life vintage to scrap?
	{
		v[8] = - v[8];							// absolute vintage size
		
		if ( v[6] > 0 )							// yet capital to shrink?
		{
			if ( v[8] > v[6] )					// more than needed?
			{
				v[8] -= v[6];					// just reduce vintage
				v[6] = 0;						// shrinkage done
				WRITES( cur, "_nVint", v[8] );		
			}
			else								// scrap entire vintage
			{
				if ( scrap_vintage( cur ) >= 0 )// not last vintage?
				{
					v[6] -= v[8];
					continue;					// don't consider for old vint.
				}
				else
				{
					v[8] = 0;					// last: nothing else to scrap
					v[6] -= v[8] - 1;
				}
			}
		}
	}
	
	// something yet to scrap and substitution to be done?
	if ( v[8] > 0 && v[7] > 0 )
	{
		if ( v[8] > v[7] )						// more than needed?
		{
			v[8] -= v[7];						// just reduce vintage
			v[7] = 0;							// substitution done
			WRITES( cur, "_nVint", v[8] );		
		}
		else									// scrap entire vintage
		{
			if ( scrap_vintage( cur ) >= 0 )	// not last vintage?
			{
				v[7] -= v[8];
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

WRITE( "_oldVint", h );

RESULT( h > 0 ? SUM( "_nVint" ) * v[1] : 0 )	// handle zero capital entrant


EQUATION( "_L2" )
/*
Effective (absolute) number of workers for firm in consumption-good sector
Result is scaled according to the defined scale
*/
VS( PARENT, "hires2" );							// make sure hiring done
RESULT( COUNT( "Wrk2" ) * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_L2d" )
/*
Labor demand of firm in consumption-good sector
*/
RESULT( V( "_life2cycle" ) > 0 ? ceil( V( "_Q2" ) / V( "_A2" ) ) : 0 )


EQUATION( "_L2vac" )
/*
Net vacancy rate of labor (unfilled positions over labor employed) for firm in
consumption-good sector
*/

v[1] = V( "_L2" );								// current number of workers
v[2] = V( "_JO2" );								// current open positions

if( v[1] == 0 )									// firm has no worker?
	v[0] = ( v[2] == 0 ) ? 0 : 1;				// handle limit case
else
	v[0] = min( v[2] / v[1], 1 );				// or calculate the regular way

RESULT( v[0] )


EQUATION( "_N" )
/*
Inventories (unsold output) of firm in consumption-good sector
*/

VS( PARENT, "D2" );								// ensure demand is allocated

v[0] = CURRENT + V( "_Q2e" ) - V( "_D2" );

RESULT( ROUND( v[0], 0 ) )						// avoid rounding errors on zero


EQUATION( "_Pi2rate" )
/*
Profit rate (profits over capital) of firm in consumption-good sector
*/
v[1] = VL( "_K", 1 );							// available capital
RESULT( v[1] > 0 ? V( "_Pi2" ) / v[1] : 0 )		// ignore no capital firms


EQUATION( "_Pi2" )
/*
Profit of firm in consumption-good sector
*/

v[1] = V( "_S2" ) - V( "_W2" );					// gross operating margin
v[2] = VS( FINSECL2, "rD" ) * VL( "_NW2", 1 );	// financial income

// firm effective interest rate on debt
v[3] = VLS( FINSECL2, "rDeb", 1 ) * ( 1 + ( VL( "_qc2", 1 ) - 1 ) * 
	   VS( FINSECL2, "kConst" ) ); 

v[4] = v[3] * VL( "_Deb2", 1 );					// interest to pay

RESULT( v[1] + v[2] - v[4] )					// firm profits before taxes


EQUATION( "_Q2e" )
/*
Effective output of firm in consumption-good sector
*/
RESULT( V( "_life2cycle" ) > 0 ? min( V( "_Q2" ), SUM( "_Qvint" ) ) : 0 )


EQUATION( "_Q2p" )
/*
Potential production with current machines for a firm in 
consumption-good sector
*/
RESULT( V( "_life2cycle" ) > 0 ? SUM( "_nVint" ) * VS( PARENT, "m2" ) : 0 )


EQUATION( "_Q2pe" )
/*
Expected potential production with remaining workers for a firm in 
consumption-good sector
*/

v[0] = 0;										// accumulator
CYCLE( cur, "Wrk2" )
	v[0] += VLS( SHOOKS( cur ), "_Q", 1 );		// add previous production

RESULT( v[0] * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_Q2u" )
/*
Capacity utilization for a firm in consumption-good sector
*/
RESULT( V( "_life2cycle" ) > 0 ? V( "_Q2e" ) / V( "_Q2p" ) :
								 VLS( PARENT, "Q2u", 1 ) )


EQUATION( "_S2" )
/*
Sales of firm in consumption-good sector
*/
VS( PARENT, "D2" );								// ensure demand is allocated
RESULT( V( "_p2" ) * V( "_D2" ) )


EQUATION( "_SId" )
/*
Desired substitution investment of firm in consumption-good sector
*/

v[1] = VS( PARENT, "m2" );						// machine output per period

v[2] = 0;										// scrapped machine accumulator
CYCLE( cur1, "Vint" )							// search last vintage to scrap
{
	v[3] = VS( cur1, "_RS" );					// number of machines to scrap 
	
	if ( v[3] == 0 )							// nothing else to do
		break;
		
	v[2] += abs( v[3] );						// accumulate vintage
}

v[4] = max( VL( "_K", 1 ) - V( "_Kd" ), 0 );	// capital shrinkage desired?
v[5] = floor( v[4] / v[1] );					// machines to remove from K

RESULT( max( v[2] - v[5], 0 ) * v[1] )


EQUATION( "_W2" )
/*
Total wages paid by firm in sector 2
*/

V( "_L2" );										// ensure hiring is done

v[0] = 0;										// wage accumulator
CYCLE( cur, "Wrk2" )
	v[0] += VS( SHOOKS( cur ), "_w" );			// adding wages

RESULT( v[0] * VS( LABSUPL2, "Lscale" ) )		// consider labor scaling


EQUATION( "_c2e" )
/*
Effective average unit cost of firm in consumption-good sector
Use expected cost if firm is not producing
*/
v[1] = V( "_Q2e" );
RESULT( v[1] > 0 ? V( "_W2" ) / v[1] : V( "_c2" ) )


EQUATION( "_dA2b" )
/*
Notional productivity (bounded) rate of change of firm in consumption-good sector
Used for wages adjustment only
*/
RESULT( mov_avg_bound( p, "_A2", VS( GRANDPARENT, "mLim" ) ) )


EQUATION( "_dNnom" )
/*
Change in firm's nominal (currency terms) inventories for a firm in 
consumption-good sector
*/
RESULT( V( "_p2" ) * V( "_N" ) - VL( "_p2", 1 ) * VL( "_N", 1 ) )


EQUATION( "_dQ2d" )
/*
Desired production change (absolute) for a firm in consumption-good sector
*/
RESULT( V( "_Q2d" ) - VL( "_Q2e", 1 ) )


EQUATION( "_l2" )
/*
Unfilled demand of firm in consumption-good sector
*/
RESULT( max( V( "_D2d" ) - V( "_D2" ), 0 ) )


EQUATION( "_life2cycle" )
/*
Stage in life cycle of firm in consumer-good sector:
0 = pre-operational entrant firm
1 = operating entrant firm (first period producing)
2.x = operating entrant firm (x=2nd-4th period producing)
3 = incumbent firm
4 = exiting firm
*/

switch( ( int ) CURRENT )
{
	case 0:										// pre-operational entrant
		if ( VL( "_K", 1 ) > 0 )				// firm has capital available?	
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


EQUATION( "_p2" )
/*
Price of good of firm in consumption-good sector
Entrants notional price are the market average
*/
RESULT( ( 1 + V( "_mu2" ) ) * ( V( "_life2cycle" ) == 0 ? 
								VLS( PARENT, "c2", 1 ) : V( "_c2" ) ) )


EQUATION( "_quits2" )
/*
Number of workers quitting jobs (not fired) in period for firm in sector 2
Updated in 'hires1' and 'hires2'
*/

v[1] = VS( LABSUPL2, "wU" );					// unemployment benefit in t

h = 0;
CYCLE_SAFE( cur, "Wrk2" )
	if ( VS( SHOOKS( cur ), "_w" ) <= v[1] )	// under unemp. benefit?
	{
		fire_worker( SHOOKS( cur ) );
		++h;
	}

RESULT( h * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_retires2" )
/*
Number of workers retiring from jobs (not fired) in period for firm in sector 2
*/

if ( VS( LABSUPL2, "Tr" ) == 0 )				// retirement disabled?
	END_EQUATION( 0 )

h = 0;
CYCLE_SAFE( cur, "Wrk2" )
	if ( VS( SHOOKS( cur ), "_age" ) == 1 )		// is a "reborn"?
	{
		fire_worker( SHOOKS( cur ) );
		++h;
	}

RESULT( h * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_sT2min" )
/*
Minimum workers tenure skills of a firm in consumption-good sector
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 1 )							// no learning by tenure mode?
	END_EQUATION( 1 );							// skills = 1
	
v[0] = DBL_MAX;									// current minimum
CYCLE( cur, "Wrk2" )
{
	v[1] = VS( SHOOKS( cur ), "_sT" );
	
	if ( v[1] < v[0] )
		v[0] = v[1];							// keep minimum					
}

RESULT( v[0] < DBL_MAX ? v[0] : CURRENT )		// handle the no worker case


EQUATION( "_s2avg" )
/*
Weighted average workers compound skills of a firm in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) == 0 )	// no learning ?
	END_EQUATION( 1 );							// skills = 1
	
v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_s" );
	++i;
}

RESULT( i > 0 ? v[0] / i : 1 )					// handle the no worker case


EQUATION( "_w2avg" )
/*
Average wage paid by firm in consumption-good sector
*/

v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_w" );
	++i;
}

RESULT( i > 0 ? v[0] / i : VLS( PARENT, "w2avg", 1 ) )


EQUATION( "_w2realAvg" )
/*
Average real wage paid by firm in consumption-good sector
*/
RESULT( V( "_w2avg" ) / VS( PARENT, "CPI" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "_cred2" )
/*
Bank credit available (new debt) to firm in consumer-good sector
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
Productivity expected for firm in consumption-good sector
Updated in '_c2'
*/

EQUATION_DUMMY( "_A2p", "_c2" )
/*
Productivity potential for firm in consumption-good sector
Updated in '_c2'
*/

EQUATION_DUMMY( "_B2", "" )
/*
Bonus paid by firm in consumption-good sector
Updated in '_Tax2'
*/

EQUATION_DUMMY( "_D2", "D2" )
/*
Demand fulfilled by firm in consumption-good sector
Updated in 'D2'
*/

EQUATION_DUMMY( "_Deb2", "" )
/*
Stock of bank debt of firm in consumption-good sector
Updated in '_Q2', '_EI', '_SI', '_Pi2' and '_Tax2'
*/

EQUATION_DUMMY( "_Div2", "" )
/*
Dividends paid by firm in consumption-good sector
Updated in '_Tax2'
*/

EQUATION_DUMMY( "_NW2", "" )
/*
Net wealth (free cash) of firm in consumption-good sector
Updated in '_Q2', '_EI', '_SI' and '_Tax2'
*/

EQUATION_DUMMY( "_cred2c", "" )
/*
Credit constraint for firm in consumption-good sector
Updated in '_Deb2max', '_Q2', '_EI' and '_SI'
*/

EQUATION_DUMMY( "_hires2", "hires2" )
/*
Effective number of workers hired in period for firm in sector 2
Updated in 'hires2'
*/

EQUATION_DUMMY( "_oldVint", "_K" )
/*
Oldest vintage (ID) in use in period by firm in consumption-good sector
Updated in '_K'
*/

EQUATION_DUMMY( "_qc2", "cScores" )
/*
Credit class of firm in sector 2 (1,2,3,4)
Updated in 'cScores'
*/
