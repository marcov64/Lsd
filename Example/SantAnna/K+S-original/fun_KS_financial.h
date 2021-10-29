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
Total banking sector deposits
Net deposits from exiting and entering firms in period not considered
*/
VS( PARENT, "Sav" );							// ensure savings are calculated
RESULT( VS( PARENT, "SavAcc" ) + SUMS( CAPSECL1, "_NW1" ) + 
		SUMS( CONSECL1, "_NW2" ) )


EQUATION( "Loans" )
/*
Total banking sector loans (non-defaulted)
Net loans to exiting and entering firms in period not considered
*/
RESULT( SUMS( CAPSECL1, "_Deb1" ) + SUMS( CONSECL1, "_Deb2" ) )
