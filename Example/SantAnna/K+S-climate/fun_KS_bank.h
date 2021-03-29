/******************************************************************************

	BANK OBJECTS EQUATIONS
	----------------------

	Equations that are specific to the bank objects in the K+S LSD model
	are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_Bda" )
/*
Customer firms financial fragility defined as the ratio between accumulated 
bad debt (loans in default) and total bank assets (stock of loans)
*/

v[1] = max( VL( "_BadDeb", 1 ), 0 );			// losses with bad debt, discarding
												// negative losses (proceedings)
v[2] = VL( "_Loans", 1 );						// stock of non-defaulted loans

RESULT( ( v[1] + v[2] ) > 0 ? v[1] / ( v[1] + v[2] ) : 0 )


EQUATION( "_ExRes" )
/*
Bank excess reserves (free cash)
Bank try to minimize excess reserves (not paying interest) trading bonds
Updates '_Bonds', '_BD'
*/

// compute initial free cash in period, before adjustments
v[0] = CURRENT + ( V( "_PiB" ) - V( "_TaxB" ) - V( "_DivB" ) ) - // + net profit 
	   ( V( "_Res" ) - VL( "_Res", 1 ) );		// - increased reserves

// account existing sovereign bonds maturing
v[1] = VL( "_Bonds" , 1 );						// bonds hold by bank
v[2] = v[1] / VS( PARENT, "thetaBonds" );		// bonds maturing in period
v[1] -= v[2];									// remaining bonds
v[0] += v[2];									// add matured bonds income

// repay exiting loans from central bank (liquidity lines)
v[3] = VL( "_LoansCB", 1 );						// liquidity loans from c. bank
if ( v[0] > 0 && v[3] > 0 )						// repay loans if possible
{
	INCR( "_LoansCB", v[0] > v[3] ? - v[3] : - v[0] );// repay what is possible
	v[0] = max( v[0] - v[3], 0 );				// remaining free cash
}

// trade bonds: sell if free cash negative or try to buy otherwise
if ( v[0] <= 0 )								// need to top up total reserves?
{
	if ( - v[0] > v[1] )						// need larger than stock?
		v[4] = - v[1];							// sell all bonds
	else
		v[4] = v[0];							// sell just required amount
}
else											// try to buy bonds
{
	v[5] = V( "_fB" ) * VS( PARENT, "BS" );		// available bond supply
	
	if ( v[0] > v[5] )							// bonds are rationed?
		v[4] = v[5];							// buy what is available
	else
		v[4] = v[0];							// buy using all excess reserves
}

WRITE( "_BD", v[4] );
INCR( "_Bonds", v[4] - v[2] );
v[0] -= v[4];									// update free cash after trade

// request loan from central bank if still illiquid (liquidity line)
if ( v[0] < 0 )									// negative free cash
{
	INCR( "_LoansCB", - v[0] );					// get loan from central bank
	v[0] = 0;									// no excess reserves
}

RESULT( v[0] )


EQUATION( "_NWb" )
/*
Bank net worth (book value)
Also updates '_Gbail', '_ExRes', '_LoansCB'
*/

// net worth as assets minus liabilities (deposits)
v[0] = V( "_Loans" ) + V( "_Res" ) + V( "_ExRes" ) + V( "_Bonds" ) - 
	   V( "_Depo" ) - V( "_LoansCB" );

// government rescue bank when net worth is negative (Basel-like rule)
if ( v[0] < 0 && VS( GRANDPARENT, "flagCreditRule" ) == 2 )
{
	// use fraction of weighted average bank market to adjust net worth
	v[1] = max( VL( "_fB", 1 ), V( "_fD" ) );	// lower bounded market share
	
	// remove effect of this bank net worth in the bank total average
	if ( v[1] < 1 )
	v[2] = ( VLS( PARENT, "NWb", 1 ) - CURRENT ) / ( 1 - v[1] );
	else
		v[2] = 0;								// monopoly, use capital rule
	
	// desired bank new capital after bailout
	v[3] = VS( PARENT, "PhiB" ) * v[1] * v[2];

	// ensure respecting the capital adequacy rate
	v[4] = max( v[3], VS( PARENT, "tauB" ) * V( "_Loans" ) );

	v[5] = - v[0] + v[4];						// government bailout
	v[0] = v[4];								// assets after bailout
	
	INCR( "_ExRes", v[5] );						// bailout enter as free cash
	WRITE( "_LoansCB", 0 );						
	WRITE( "_Gbail", v[5] );					// register bailout
}
else
	WRITE( "_Gbail", 0 );						// no bailout	

RESULT( v[0] )	


EQUATION( "_TC" )
/*
Total credit supply provided by bank to firms.
Negative value (-1) means unlimited credit.
*/

k = VS( GRANDPARENT, "flagCreditRule" );		// credit limit & bail-out rule

if ( k == 1 )									// deposits multiplier rule?
	v[0] = VL( "_Depo", 1 ) / VS( PARENT, "tauB" );// 1/tauB = bank multiplier
else
	if ( k == 2 )								// Basel-like credit rule?
		v[0] = VL( "_NWb", 1 ) / ( VS( PARENT, "tauB" ) *
						   ( 1 + VS( PARENT, "betaB" ) * V( "_Bda" ) ) );
	else
	v[0] = -1;									// no-limit rule	

RESULT( v[0] )	


EQUATION( "_TC1free" )
/*
Minimum bank total credit supply to firms in capital-good sector
Updated in 'update_debt1' support function
*/

if ( VS( GRANDPARENT, "flagCreditRule" ) > 0 )
{
	v[1] = VS( CAPSECL2, "F1" );				// # firms in sector 1

	v[2] = v[1] / ( v[1] + VS( CONSECL2, "F2" ) );// credit allocation for s. 1
	v[0] = v[2] * max( 0, V( "_TC" ) - VL( "_Loans", 1 ) );// free cash to lend
}
else
	v[0] = -1;									// no limit

RESULT( v[0] )


EQUATION( "_TC2free" )
/*
Maximum bank total credit supply to firms in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagCreditRule" ) > 0 )
{
	v[1] = VS( CONSECL2, "F2" );				// # firms in sector 2

	v[2] = v[1] / ( VS( CAPSECL2, "F1" ) + v[1] );// credit allocation for s. 2
	v[0] = v[2] * max( 0, V( "_TC" ) - VL( "_Loans", 1 ) );// free cash to lend
}
else
	v[0] = -1;									// no limit
	
RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_BadDeb" )
/*
Bank bad debt (defaults) on the period
Just reset once per period, updated in 'entry1exit', 'entry2exit'
*/
RESULT( 0 )


EQUATION( "_Cl" )
/*
Number of bank clients
*/
RESULT( COUNT( "Cli1" ) + COUNT( "Cli2" ) )


EQUATION( "_Depo" )
/*
Bank deposits
Net deposits from exiting and entering firms in period not considered
*/

VS( GRANDPARENT, "Sav" );						// ensure savings are calculated
V( "_Loans" );									// ensure all transactions done

v[0] = V( "_fD" ) * VS( GRANDPARENT, "SavAcc" );// workers deposits

CYCLE( cur, "Cli1" )							// sector 1 deposits
	v[0] += max( VS( SHOOKS( cur ), "_NW1" ), 0 );

CYCLE( cur, "Cli2" )							// sector 2 deposits
	v[0] += max( VS( SHOOKS( cur ), "_NW2" ), 0 );
	
RESULT( max( v[0], 0 ) )


EQUATION( "_Loans" )
/*
Bank loans (non-defaulted)
Net loans to exiting and entering firms in period not considered
*/

VS( CAPSECL2, "Tax1" );							// ensure transactions are done
VS( CONSECL2, "Tax2" );

v[0] = 0;										// accumulator

CYCLE( cur, "Cli1" )							// sector 1 debt
	v[0] += VS( SHOOKS( cur ), "_Deb1" );

CYCLE( cur, "Cli2" )							// sector 2 debt
	v[0] += VS( SHOOKS( cur ), "_Deb2" );
	
RESULT( v[0] )


EQUATION( "_PiB" )
/*
Bank gross profits (losses) before dividends/taxes
*/

v[1] = VS( PARENT, "rDeb" );					// interest on debt
v[2] = VS( PARENT, "kConst" );					// interest scaling

// compute the firm-specific interest expense
v[3] = 0;										// interest accumulator
CYCLE( cur, "Cli1" )							// sector 1
{
	j = VLS( SHOOKS( cur ), "_qc1", 1 );		// firm credit class
	v[4] = VLS( SHOOKS( cur ), "_Deb1", 1 );	// firm debt
	v[3] += v[4] * v[1] * ( 1 + ( j - 1 ) * v[2] );// interest received
}

CYCLE( cur, "Cli2" )							// sector 2
{
	j = VLS( SHOOKS( cur ), "_qc2", 1 );		// firm credit class
	v[4] = VLS( SHOOKS( cur ), "_Deb2", 1 );	// firm debt
	v[3] += v[4] * v[1] * ( 1 + ( j - 1 ) * v[2] );// interest received
}

RESULT( v[3] + VS( PARENT, "rRes" ) * VL( "_Res", 1 ) + 
		VS( PARENT, "rBonds" ) * VL( "_Bonds", 1 ) - 
		VS( PARENT, "r" ) * VL( "_LoansCB", 1 ) -
		VS( PARENT, "rD" ) * VL( "_Depo", 1 ) - VL( "_BadDeb", 1 ) )


EQUATION( "_Res" )
/*
Bank required reserves hold at the central bank
*/
RESULT( VS( PARENT, "tauB" ) * V( "_Depo" ) )


EQUATION( "_TaxB" )
/*
Tax paid by bank in consumption-good sector
Also updates '_DivB'
*/

v[1] = V( "_PiB" );								// gross profits

if ( v[1] > 0 )									// profits?
{
	v[2] = VS( GRANDPARENT, "tr" );				// tax rate	
	
	v[0] = v[2] * v[1];							// paid taxes
	v[3] = VS( PARENT, "dB" ) * v[1] * ( 1 - v[2] );// paid dividends
}
else
	v[3] = v[0] = 0;							// no dividends/taxes

WRITE( "_DivB", v[3] );							// update dividends

RESULT( v[0] )


EQUATION( "_fB" )
/*
Bank effective market share (in number of customers)
*/
RESULT( V( "_Cl" ) / VS( PARENT, "Cl" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_BD", "_ExRes" )
/*
Sovereign bonds demand by bank
Updated in '_ExRes'
*/

EQUATION_DUMMY( "_Bonds", "_ExRes" )
/*
Sovereign bonds stock hold by bank
Updated in '_ExRes'
*/

EQUATION_DUMMY( "_DivB", "_TaxB" )
/*
Bank distributed dividends
Updated in '_PiB'
*/

EQUATION_DUMMY( "_Gbail", "_NWb" )
/*
Government bailout funds to bank
Updated in '_NWb'
*/

EQUATION_DUMMY( "_LoansCB", "_ExRes" )
/*
Liquidity loans from central bank to bank
Updated in '_ExRes', '_NWb'
*/
