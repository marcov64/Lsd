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


EQUATION( "_TC" )
/*
Total credit supply provided by bank to firms.
Negative value (-1) means unlimited credit.
*/

if ( VS( GRANDPARENT, "flagCreditRule" ) == 1 )
	v[0] = V( "_NWb" ) / ( VS( PARENT, "tauB" ) * // Basel-like credit rule
						   ( 1 + VS( PARENT, "betaB" ) * V( "_Bda" ) ) );
else
	v[0] = -1;									// no-limit rule	

RESULT( v[0] )	


EQUATION( "_TC1free" )
/*
Minimum bank total credit supply to firms in sector 1
Updated in 'update_debt1' support function
*/

if ( VS( GRANDPARENT, "flagCreditRule" ) == 1 )
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
Maximum bank total credit supply to firms in sector 2
*/

if ( VS( GRANDPARENT, "flagCreditRule" ) == 1 )
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
Bank reserves (deposits on Central Bank)
*/

VS( GRANDPARENT, "Sav" );						// ensure savings are calculated

v[0] = V( "_fd" ) * VS( GRANDPARENT, "SavAcc" );// workers deposits

CYCLE( cur, "Cli1" )							// sector 1 deposits
	v[0] += VS( SHOOKS( cur ), "_NW1" );

CYCLE( cur, "Cli2" )
	v[0] += VS( SHOOKS( cur ), "_NW2" );		// sector 2 deposits
	
RESULT( v[0] )


EQUATION( "_Loans" )
/*
Bank loans (non-defaulted)
*/

v[0] = 0;										// accumulator

CYCLE( cur, "Cli1" )							// sector 1 debt
	v[0] += VS( SHOOKS( cur ), "_Deb1" );

CYCLE( cur, "Cli2" )							// sector 2 debt
	v[0] += VS( SHOOKS( cur ), "_Deb2" );
	
RESULT( v[0] )


EQUATION( "_NWb" )
/*
Bank net worth (liquid assets)
*/

v[0] = CURRENT + V( "_PiB" );					// update net worth

// government rescue bank when equity is below minimum
// use wages inflation to adjust initial capital
if ( v[0] < 0 )
{
	v[1] = V( "_fd" ) * VS( PARENT, "NWb0" ) * 	// capital requirements
		   VLS( LABSUPL2, "wAvgEmp", 1 ) / 1.0 +// initial wage is 1
		   VS( PARENT, "tauB" ) * V( "_Loans" );
	v[2] = - v[0] + v[1];						// government bailout
	v[0] = v[1];								// assets after bailout
	
	WRITE( "_Gbail", v[2] );					// register bailout
}
else
	WRITE( "_Gbail", 0 );						// no bailout	

RESULT( v[0] )	


EQUATION( "_PiB" )
/*
Bank profits (losses)
*/

v[1] = VLS( PARENT, "rDeb", 1 );				// interest on debt
v[2] = VS( PARENT, "kConst" );					// interest scaling
v[3] = VL( "_Depo", 1 );						// deposits

// compute the firm-specific interest expense
v[5] = 0;										// interest accumulator
CYCLE( cur, "Cli1" )							// sector 1
{
	j = VLS( SHOOKS( cur ), "_qc1", 1 );		// firm credit class
	v[4] = VLS( SHOOKS( cur ), "_Deb1", 1 );	// firm debt
	v[5] += v[4] * v[1] * ( 1 + ( j - 1 ) * v[2] );// interest received
}

CYCLE( cur, "Cli2" )							// sector 2
{
	j = VLS( SHOOKS( cur ), "_qc2", 1 );		// firm credit class
	v[4] = VLS( SHOOKS( cur ), "_Deb2", 1 );	// firm debt
	v[5] += v[4] * v[1] * ( 1 + ( j - 1 ) * v[2] );// interest received
}

v[6] = ( VLS( PARENT, "rRes", 1 ) - VS( PARENT, "rD" ) ) * v[3] + v[5] - 
	   VL( "_BadDeb", 1 );						// gross profit (loss)

if ( v[6] > 0 )									// profits?
{
	v[7] = VS( GRANDPARENT, "tr" );				// tax rate	
	
	v[8] = VS( PARENT, "dB" ) * v[6] * ( 1 - v[7] );// paid dividends
	v[9] = v[7] * v[6];							// paid taxes
}
else
	v[8] = v[9] = 0;							// no dividends/taxes

WRITE( "_DivB", v[8] );							// update dividends
WRITE( "_TaxB", v[9] );							// update taxes

RESULT( v[6] - v[8] - v[9] )					// net profit (loss)


EQUATION( "_fB" )
/*
Bank effective market share (in number of customers)
*/
RESULT( V( "_Cl" ) / VS( PARENT, "Cl" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_DivB", "_PiB" )
/*
Bank distributed dividends
Updated in '_PiB'
*/

EQUATION_DUMMY( "_Gbail", "_NWb" )
/*
Government bailout funds to bank
When there is a bailout, it is updated in '_NWb'
*/

EQUATION_DUMMY( "_TaxB", "_PiB" )
/*
Bank tax paid.
Updated in '_PiB'.
*/
