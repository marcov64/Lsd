/******************************************************************************

	FINANCIAL MARKET OBJECT EQUATIONS
	---------------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the financial market object in the K+S
	LSD model are coded below.

 ******************************************************************************/

/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "BadDeb" )
/*
Total bank losses from bad debt
This variable must be explicitly recalculated after entry/exit
*/
RESULT( V( "BadDeb1" ) + V( "BadDeb2" ) )


EQUATION( "BadDeb1" )
/*
Bank bad debt (defaults) from capital-good sector
Just reset once per period, updated in 'entry1exit'
*/
RESULT( 0 )


EQUATION( "BadDeb2" )
/*
Bank bad debt (defaults) from consumption-good sector
Just reset once per period, updated in 'entry2exit'
*/
RESULT( 0 )


EQUATION( "Depo" )
/*
Bank deposits
Net deposits from exiting and entering firms in period not considered
*/
VS( PARENT, "Sav" );							// ensure savings are calculated
V( "Loans" );									// ensure all transactions done
RESULT( VS( PARENT, "SavAcc" ) + VS( CAPSECL1, "NW1" ) + VS( CONSECL1, "NW2" ) )


EQUATION( "DepoG" )
/*
Government deposits
*/
RESULT( max( CURRENT - VS( PARENT, "Def" ) - VLS( PARENT, "Deb", 1 ), 0 ) )


EQUATION( "Loans" )
/*
Bank loans (non-defaulted)
Net loans to exiting and entering firms in period not considered
*/
VS( CAPSECL1, "Tax1" );							// ensure transactions are done
VS( CONSECL1, "Tax2" );
RESULT( VS( CAPSECL1, "Deb1" ) + VS( CONSECL1, "Deb2" ) )


EQUATION( "PiB" )
/*
Bank profits (losses)
*/
RESULT( V( "iB" ) - V( "iDb" ) - VL( "BadDeb", 1 ) )


EQUATION( "iB" )
/*
Bank interest income from loans
*/
RESULT( V( "r" ) * ( SUMLS( CAPSECL1, "_Deb1", 1 ) +
					 SUMLS( CONSECL1, "_Deb2", 1 ) ) )


EQUATION( "iDb" )
/*
Bank interest payments from deposits
*/
RESULT( V( "r" ) * ( VLS( PARENT, "SavAcc", 1 ) +
					 SUMLS( CAPSECL1, "_NW1", 1 ) +
					 SUMLS( CONSECL1, "_NW2", 1 ) ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "DepoG", "Deb" )
/*
Government deposits at central bank (accumulated surpluses)
Updated in 'Deb'
*/
