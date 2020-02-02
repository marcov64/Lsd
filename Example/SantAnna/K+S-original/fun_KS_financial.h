/******************************************************************************

	FINANCIAL MARKET OBJECT EQUATIONS
	---------------------------------

	Equations that are specific to the financial market object in the K+S  
	LSD model are coded below.
 
 ******************************************************************************/

/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "BadDeb" )
/*
Bank bad debt (defaults) on the period
Just reset once per period, updated in 'entry1exit', 'entry2exit'
*/
RESULT( 0 )


EQUATION( "Depo" )
/*
Bank reserves (deposits)
*/

VS( PARENT, "Sav" );							// ensure savings are calculated

v[0] = VS( PARENT, "SavAcc" );					// workers deposits

CYCLES( CAPSECL1, cur, "Firm1" )				// sector 1 deposits
	v[0] += VS( cur, "_NW1" );

CYCLES( CONSECL1, cur, "Firm2" )
	v[0] += VS( cur, "_NW2" );					// sector 2 deposits
	
RESULT( v[0] )


EQUATION( "Loans" )
/*
Bank loans (non-defaulted)
*/

v[0] = 0;										// accumulator

CYCLES( CAPSECL1, cur, "Firm1" )
	v[0] += VS( cur, "_Deb1" );					// sector 1 debt

CYCLES( CONSECL1, cur, "Firm2" )
	v[0] += VS( cur, "_Deb2" );					// sector 2 debt
	
RESULT( v[0] )
