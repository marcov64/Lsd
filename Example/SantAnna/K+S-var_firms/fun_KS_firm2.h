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
Also updates '_CD2', '_CD2c', '_CS2'
*/

// maximum debt allowed to firm, considering net worth and operating margin
v[1] = VS( FINSECL2, "Lambda" ) * max( VL( "_NW2", 1 ), 
									   VL( "_S2", 1 ) - VL( "_W2", 1 ) );
		   
// apply an absolute floor to maximum debt prudential limit
v[0] = max( v[1], VS( FINSECL2, "Lambda0" ) * VLS( CAPSECL2, "PPI", 1 ) / 
				  VS( CAPSECL2, "PPI0" ) );

WRITE( "_CD2", 0 );								// reset total credit demand
WRITE( "_CD2c", 0 );							// reset constraint for period
WRITE( "_CS2", 0 );								// reset total credit supplied

RESULT( v[0] )
		

EQUATION( "_D2e" )
/*
Adaptive demand expectation of firm in consumer-good sector
*/

h = V( "_t2ent" );								// firm entry period
k = VS( GRANDPARENT, "flagExpect" );			// expectation form
j = ( k == 0 || k > 4 ) ? 1 : ( k == 1 ) ? 4 : 2;// req. number of data periods

if ( h > 0 && h >= T - 1 - j )					// entrant or too few data?
	END_EQUATION( CURRENT );

// compute the mix between fulfilled and potential demand (orders)
v[9] = VS( PARENT, "e0" );						// animal spirits parameter
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

v[1] = VLS( PARENT, "l2avg", 1 ) + 1;

RESULT( - VS( PARENT, "omega1" ) * V( "_p2" ) / VS( PARENT, "p2avg" ) - 
		VS( PARENT, "omega2" ) * ( v[1] > 1 ? ( VL( "_l2", 1 ) + 1 ) / v[1] : 1 ) )


EQUATION( "_EI" )
/*
Effective expansion investment of firm in consumption-good sector
Also updates '_NW2', '_CD2', 'CD2c', 'CS2'
*/
V( "_Q2" );										// make sure production decided
V( "_supplier" );								// ensure supplier is selected
RESULT( invest( THIS, V( "_EId" ) ) )


EQUATION( "_EId" )
/*
Desired expansion investment of firm in consumption-good sector
*/

v[1] = V( "_Kd" );								// desired capital
v[2] = VL( "_K", 1 );							// available capital stock
v[3] = VS( PARENT, "m2" );						// machine output per period

if ( v[2] < v[3] )								// no capital yet?
	END_EQUATION( v[1] );						// no growth threshold
	
v[4] = VS( PARENT, "kappaMin" );				// investment floor multiple

// min rounded capital
v[5] = round( ( 1 + v[4] ) * v[2] / v[3] ) * v[3];

if ( v[1] > v[5] )								// minimum capital reached?
{
	if ( v[4] > 0 )
		v[0] = v[5] - v[2];
	else
	{
		v[6] = VS( PARENT, "kappaMax" );		// investment cap multiple
		v[7] = round( ( 1 + v[6] ) * v[2] / v[3] ) * v[3];
		
		if ( v[6] > 0 && v[1] > v[7] )
			v[0] = v[7] - v[2];					// max rounded capital
		else
			v[0] = floor( ( v[1] - v[2] ) / v[3] ) * v[3];
	}
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

// desired capacity with slack and utilization, based on expectations/inventories
RESULT( max( ( 1 + VS( PARENT, "iota" ) ) * V( "_D2e" ) - VL( "_N", 1 ), 0 ) / 
		VS( PARENT, "u" ) )


EQUATION( "_Q2" )
/*
Planned production for a firm in consumption-good sector
Also updates '_NW2', '_CD2', 'CD2c', 'CS2'
*/

v[1] = V( "_Q2d" );								// desired production

if ( v[1] <= 0 )
	END_EQUATION( 0 );							// nothing to do

v[2] = V( "_CS2a" );							// available credit supply
v[3] = VL( "_NW2", 1 );							// net worth (cash available)
v[4] = V( "_c2" );								// expected unit cost
v[5] = VS( PARENT, "m2" );						// machine output per period

v[6] = v[1] * v[4];								// cost of desired production

if ( v[6] <= v[3] - 1 )							// firm can self-finance?
{
	v[0] = v[1];								// plan the desired output
	v[3] -= v[6];								// remove wage cost from cash
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
		// produce as much as the available finance allows, rounded # machines
		v[0] = floor( max( ( v[3] - 1 + v[2] ) / v[4], 0 ) / v[5] ) * v[5];
		v[8] = v[6] - v[3] + 1;					// desired credit

		if ( v[0] == 0 )
			v[7] = 0;							// no finance
		else
		{
			v[6] = v[0] * v[4];					// reduced production cost
			v[7] = v[6] - v[3] + 1;				// finance the difference
			v[3] = 1;							// keep minimum cash
		}
	}
	
	update_debt2( THIS, v[8], v[7] );			// update debt (desired/granted)
}

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
RESULT( min( v[1], VL( "_K", 1 ) ) )


EQUATION( "_SI" )
/*
Effective substitution investment of firm in consumption-good sector
Also updates '_NW2', '_CD2', 'CD2c', 'CS2'
*/
V( "_EI" );										// make sure expansion done
RESULT( invest( THIS, V( "_SId" ) ) )


EQUATION( "_Tax2" )
/*
Tax paid by firm in consumption-good sector
Also updates '_Div2', '_NW2', '_CD2', 'CD2c', 'CS2'
*/

v[1] = V( "_Pi2" );								// firm profit in period
v[2] = VS( GRANDPARENT, "tr" );					// tax rate

if ( v[1] > 0 )									// profits?
{
	v[0] = v[1] * v[2];							// tax to government
	v[4] = VS( PARENT, "d2" ) * ( v[1] - v[0] );// shareholders dividend 
}
else
	v[0] = v[4] = 0;							// no tax/divid. on losses

WRITE( "_Div2", v[4] );							// save period dividends

v[6] = v[1] - v[0] - v[4];						// free cash flow

// net worth after reversing provisioned expected production cost in '_Q2'
v[7] = INCR( "_NW2", V( "_Q2" ) * V( "_c2" ) );

if ( v[6] < 0 )									// must finance losses?
{
	if ( v[7] >= - v[6] + 1 )					// can cover losses with reserves?
		INCR( "_NW2", v[6] );					// draw from net worth
	else
	{
		v[8] = V( "_CS2a" );					// available credit supply
		v[9] = - v[6] - v[7] + 1;				// desired finance
		
		if ( v[8] >= v[9] )						// can finance losses?
		{
			update_debt2( THIS, v[9], v[9] );	// finance all
			WRITE( "_NW2", 1 );					// minimum net worth
		}
		else
		{
			update_debt2( THIS, v[9], v[8] );	// take what is possible
			INCR( "_NW2", v[6] - v[8] );		// let negative NW (bankruptcy)
		}					
	}
}
else											// pay debt with available cash
{
	v[10] = V( "_Deb2" ) * VS( FINSECL2, "deltaB" );// desired debt repayment
	
	if ( v[10] > 0 )							// something to repay?
	{
		if ( v[6] > v[10] )						// can repay desired and more
		{
			update_debt2( THIS, 0, - v[10] );	// repay desired
			INCR( "_NW2", v[6] - v[10] );		// save the rest
		}
		else
			update_debt2( THIS, 0, - v[6] );	// repay what is possible
	}
	else
		INCR( "_NW2", v[6] );					// save all
}

RESULT( v[0] )


EQUATION( "_alloc2" )
/*
Allocate workers to vintages, prioritizing newer vintages
Also updates '_Lvint'
*/

cur = HOOK( TOPVINT );							// start with top vintage
if ( cur == NULL )
	END_EQUATION( 0 );							// no vintage, nothing to do

V( "_c2" );										// ensure machines are set
v[1] = V( "_L2" );								// workers to allocate
v[2] = VS( cur, "_LdVint" );					// labor demand of vintage

v[0] = 0;										// allocated worker counter
while ( v[1] > 0 )								// allocate all available workers
{
	while ( v[2] == 0 )							// find vintage with posit. dem.
	{
		cur = SHOOKS( cur );					// so go to the previous one
		if ( cur == NULL || VS( cur, "_toUseVint" ) == 0 )// oldest or done?
			goto done_alloc2;					// not possible to allocate more
			
		v[2] = VS( cur, "_LdVint" );			// labor demand of vintage
	}
	
	if ( v[1] >= v[2] )							// can fully fulfill demand?
	{
		WRITES( cur, "_Lvint", v[2] );			// update available worker counter
		v[1] -= v[2];
		v[0] += v[2];
		v[2] = 0;
	}
	else
	{
		WRITES( cur, "_Lvint", v[1] );			// supply what is available
		v[0] += v[1];
		v[1] = 0;
	}
}

done_alloc2:

RESULT( v[0] )


EQUATION( "_c2" )
/*
Planned average unit cost of firm in consumption-good sector
Considers even unused machines, as in original K+S
Also updates '_A2'
*/

v[1] = V( "_Q2d" );								// desired production
v[2] = VL( "_K", 1 );							// available capital stock
v[3] = VS( PARENT, "m2" );						// machine output per period
v[4] = VS( LABSUPL2, "w" );						// firm wage

v[5] = max( floor( v[2] / v[3] ) - ceil( v[1] / v[3] ), 0 );// unused machines

// scan all vintages, from oldest to newest, preferring to use newer ones
v[0] = v[6] = v[7] = 0;							// accumulators
CYCLE( cur, "Vint" )							// choose vintages to use
{
	v[8] = VS( cur, "_nVint" );					// number of machines in vintage
	v[9] = VS( cur, "_Avint" );					// vintage productivity
	
	v[0] += v[8] * v[4] / v[9];					// add machines cost
	v[6] += v[8];								// add machines
	v[7] += v[8] * v[9];						// add productivities

	if ( v[5] >= v[8] )							// none to be used in the vint.?
	{
		v[5] -= v[8];							// less machines not to use
		v[10] = 0;								// no machines to use in vintage
	}
	else
	{
		v[10] = v[8] - v[5];					// machines to use in vintage
		v[5] = 0;								// no more machine not to use
	}
	
	WRITES( cur, "_toUseVint", v[10] );			// number mach. to try to use
	WRITES( cur, "_Lvint", 0 );					// no worker allocated yet
}

if ( v[6] == 0 )								// no machine?
{
	V( "_supplier" );							// ensure supplier is selected
	cur1 = PARENTS( SHOOKS( HOOK( SUPPL ) ) );	// pointer to supplier
	
	v[6] = 1;									// 1 notional machine
	v[7] = VS( cur1, "_Atau" );					// new machines productivity
	v[0] = v[4] / v[7];							// machine unit cost
}

WRITE( "_A2", v[7] / v[6] );					// potential productivity

RESULT( v[0] / v[6] )


EQUATION( "_f2" )
/*
Market share of firm in consumption-good sector
It is computed using a replicator equation over the relative competitiveness 
of the firm
Because of entrants, market shares may add-up to more than one, so 'f2rescale'
must be used before '_f2' is used, by calling 'CPI'
*/

if( V( "_life2cycle" ) == 0 )					// entrant firm state
	END_EQUATION( 0 );

v[1] = VS( PARENT, "f2min" );					// minimum share to stay

if ( CURRENT == 0 && V( "_t2ent" ) == T - 2 )	// first-period entrant	?
{
	v[0] = VL( "_K", 1 ) / VLS( PARENT, "K", 1 );// same as capital share
	END_EQUATION( max( v[0], v[1] ) );			// but over minimum
}
		
// replicator equation					
v[0] = VL( "_f2", 1 ) * ( 1 - VS( PARENT, "chi" ) * 
						  ( V( "_E" ) / VS( PARENT, "Eavg" ) - 1 ) );

// lower-bounded slightly below exit threshold
// to ensure firm sells production before leaving the market
RESULT( max( v[0], 0.99 * v[1] ) )


EQUATION( "_mu2" )
/*
Mark-up of firm in consumption-good sector
*/

v[1] = VL( "_f2", 1 );							// past periods market shares
v[2] = VL( "_f2", 2 );
v[3] = VS( PARENT, "f2min" );					// market exit share threshold

if ( v[1] < v[3] || v[2] < v[3] )				// just entered firms keep it
	END_EQUATION( CURRENT );

RESULT( CURRENT * ( 1 + VS( PARENT, "upsilon" ) * ( v[1] / v[2] - 1 ) ) )


EQUATION( "_supplier" )
/*
Selected machine supplier by firm in consumption-good sector
Also set firm 'hook' pointers to supplier firm object
*/

VS( CAPSECL2, "inn" );							// ensure innovation is done and
												// brochures distributed
v[1] = VS( PARENT, "m2" );						// machine modularity
v[2] = VS( PARENT, "b" );						// required payback period
												
v[3] = DBL_MAX;									// supplier price/cost ratio
i = 0;
cur2 = cur3 = NULL;
CYCLE( cur, "Broch" )							// use brochures to find supplier
{
	cur1 = PARENTS( SHOOKS( cur ) );			// pointer to supplier object	
	
	// compare total machine unit cost (acquisition + operation for payback period)
	v[4] = VS( cur1, "_p1" ) / v[1] + VS( cur1, "_cTau" ) * v[2];
	if ( v[4] < v[3] )							// best so far?
	{
		v[3] = v[4];							// save current best supplier
		i = VS( cur1, "_ID1" );					// supplier ID
		cur2 = SHOOKS( cur );					// own entry on supplier list
		cur3 = cur;								// best supplier brochure
	}
}

// if supplier is found, simply update it, if not, draw a random one
if ( cur2 != NULL && cur3 != NULL )
	WRITES( cur2, "_tSel", T );					// update selection time
else											// no brochure received
{
	cur1 = RNDDRAWS( CAPSECL2, "Firm1", "_Atau" );// try draw new good supplier
	i = VS( cur1, "_ID1" );
	
	// create the brochure/client interconnected objects
	cur3 = send_brochure( i, cur1, V( "_ID2" ), THIS );
}

WRITE_HOOK( SUPPL, cur3 );						// pointer to current brochure

RESULT( i )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_CI" )
/*
Canceled investment of firm in consumption-good sector
*/

cur = HOOK( SUPPL );							// pointer to current supplier
VS( PARENTS( SHOOKS( cur ) ), "_Q1e" );			// make sure supplier produced

v[1] = VS( SHOOKS( cur ), "_nCan" );			// canceled machine number
k = VS( SHOOKS( cur ), "_tOrd" );				// time of canceled order

if ( k == T && v[1] > 0 )
{	
	v[2] = VS( PARENT, "m2" );					// machine output per period
	v[3] = V( "_SI" ) / v[2];					// machines to substitute
	v[4] = V( "_EI" ) / v[2];					// machines to expand
	v[5] = VS( PARENTS( SHOOKS( cur ) ), "_p1" );// machine price
	
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
VS( PARENT, "CPI" );							// ensure m.s. updated
RESULT( V( "_f2" ) * VS( PARENT, "D2d" ) )


EQUATION( "_JO2" )
/*
Open job positions for a firm in consumption-good sector
*/
RESULT( max( V( "_L2d" ) - VL( "_L2", 1 ), 0 ) )


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
		add_vintage( THIS, v[4], false );		// create vintage
}

v[5] = max( VL( "_K", 1 ) + v[3] - V( "_Kd" ), 0 );// desired capital shrinkage
v[6] = floor( v[5] / v[1] );					// machines to remove from K

v[7] = floor( v[2] / v[1] );					// machines to substitute in K

j = T + 1;										// oldest vintage so far
h = 0;											// oldest vintage ID
CYCLE_SAFE( cur, "Vint" )						// search from older vintages
{
	v[8] = VS( cur, "_RSvint" );				// number of machines to scrap
	
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
				if ( scrap_vintage( var, cur ) >= 0 )// not last vintage?
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
			if ( scrap_vintage( var, cur ) >= 0 )// not last vintage?
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

RESULT( SUM( "_nVint" ) * v[1] )


EQUATION( "_L2" )
/*
Labor employed by firm in consumption-good sector
*/
v[1] = VS( PARENT, "L2d" );
RESULT( v[1] > 0 ? V( "_L2d" ) * VS( PARENT, "L2" ) / v[1] : 0 )


EQUATION( "_L2d" )
/*
Labor demand of firm in consumption-good sector
*/
RESULT( V( "_life2cycle" ) > 0 ? V( "_Q2" ) / V( "_A2" ) : 0 )


EQUATION( "_N" )
/*
Inventories (unsold output) of firm in consumption-good sector
*/

VS( PARENT, "D2" );								// ensure demand is allocated

v[0] = CURRENT + V( "_Q2e" ) - V( "_D2" );

RESULT( ROUND( v[0], 0, 0.001 ) )				// avoid rounding errors on zero


EQUATION( "_Pi2" )
/*
Profit of firm (before taxes) in consumption-good sector
*/

v[1] = V( "_S2" ) - V( "_W2" );					// gross operating margin
v[2] = VS( FINSECL2, "rD" ) * VL( "_NW2", 1 );	// financial income

// firm effective interest rate on debt
v[3] = VS( FINSECL2, "rDeb" ) * ( 1 + ( VL( "_qc2", 1 ) - 1 ) * 
	   VS( FINSECL2, "kConst" ) ); 

v[4] = v[3] * VL( "_Deb2", 1 );					// interest to pay

RESULT( v[1] + v[2] - v[4] )					// firm profits before taxes


EQUATION( "_Q2e" )
/*
Effective output of firm in consumption-good sector
*/
RESULT( min( V( "_Q2" ), SUM( "_Qvint" ) ) )


EQUATION( "_Q2p" )
/*
Potential production with current machines for a firm in 
consumption-good sector
*/
RESULT( SUM( "_nVint" ) * VS( PARENT, "m2" ) )


EQUATION( "_Q2pe" )
/*
Expected potential production with existing workers for a firm in 
consumption-good sector
*/
RESULT( VL( "_L2", 1 ) * V( "_A2" ) )


EQUATION( "_Q2u" )
/*
Capacity utilization for a firm in consumption-good sector
*/
v[1] = V( "_Q2p" );
RESULT( v[1] > 0 ? V( "_Q2e" ) / v[1] : VLS( PARENT, "Q2u", 1 ) )


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
	v[3] = VS( cur1, "_RSvint" );				// number of machines to scrap 
	
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
RESULT( V( "_L2" ) * VS( LABSUPL2, "w" ) )


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
RESULT( mov_avg_bound( THIS, "_A2", VS( GRANDPARENT, "mLim" ), 
					   VS( GRANDPARENT, "mPer" ) ) )


EQUATION( "_dNnom" )
/*
Change in firm's nominal (currency terms) inventories for a firm in 
consumption-good sector
*/
RESULT( V( "_p2" ) * V( "_N" ) - VL( "_p2", 1 ) * VL( "_N", 1 ) )


EQUATION( "_life2cycle" )
/*
Stage in life cycle of firm in consumer-good sector:
0 = pre-operational entrant firm
1 = operating entrant firm (first period producing)
*/
RESULT( CURRENT > 0 ? 1 : VL( "_K", 1 ) > 0 ? 1 : 0 )


EQUATION( "_p2" )
/*
Price of good of firm in consumption-good sector
Entrants notional price are the market average
*/
RESULT( ( 1 + V( "_mu2" ) ) * ( V( "_life2cycle" ) == 0 ? 
								VLS( PARENT, "c2", 1 ) : V( "_c2" ) ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "_CS2a" )
/*
Bank credit supply available (new debt) to firm in consumer-good sector
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
Machine-level average labor productivity of firm in consumption-good sector
Updated in '_c2'
*/

EQUATION_DUMMY( "_CD2", "" )
/*
Credit demand for firm in consumption-good sector
Updated in '_Deb2max', '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_CD2c", "" )
/*
Credit demand constraint for firm in consumption-good sector
Updated in '_Deb2max', '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_CS2", "" )
/*
Credit supplied to firm in consumption-good sector
Updated in '_Deb2max', '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_D2", "D2" )
/*
Demand fulfilled by firm in consumption-good sector
Updated in 'D2'
*/

EQUATION_DUMMY( "_Deb2", "" )
/*
Stock of bank debt of firm in consumption-good sector
Updated in '_Q2', '_EI', '_SI', '_Pi2', '_Tax2'
*/

EQUATION_DUMMY( "_Div2", "" )
/*
Dividends paid by firm in consumption-good sector
Updated in '_Tax2'
*/

EQUATION_DUMMY( "_NW2", "" )
/*
Net worth of firm in consumption-good sector
Updated in '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_l2", "D2" )
/*
Unfilled demand of firm in consumption-good sector
Updated in 'D2'
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
