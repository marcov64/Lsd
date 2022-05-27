/******************************************************************************

	FINANCIAL MARKET OBJECT EQUATIONS
	---------------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the financial market object in the K+S
	LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "BS" )
/*
Sovereign bond supply (new issues) from government
Also updates 'DepoG'
*/

v[1] = VS( PARENT, "Def" );						// public deficit
v[2] = VL( "DepoG", 1 );						// government deposits at c.b.
v[3] = ( VL( "BondsB", 1 ) + VL( "BondsCB", 1 ) ) / V( "thetaBonds" );
												// bonds maturing in period

if ( v[1] + v[3] < v[2] )						// no new bonds to supply?
{
	v[0] = 0;									// no bond issue
	v[2] -= v[1] + v[3];						// keep surplus at central bank
}
else
{
	v[0] = v[1] + v[3] - v[2];					// issue just what is needed
	v[2] = 0;									// zero deposits
}

WRITE( "DepoG", v[2] );

RESULT( v[0] )


EQUATION( "r" )
/*
Interest rate set by the central bank (prime rate)
*/

v[0] = CURRENT - VL( "rShock", 1 );				// last period rate without shock
v[1] = V( "rAdj" );								// rate adjustment step

// Taylor rule
v[2] = V( "rT" ) + V( "gammaPi" ) * ( VLS( CONSECL1, "dCPIb", 1 ) - V( "piT" ) ) +
	   V( "gammaU" ) * ( V( "Ut" ) - VLS( LABSUPL1, "U", 1 ) );

// smooth rate adjustment
if ( abs( v[2] - v[0] ) > 2 * v[1] )
	v[0] += ( v[2] > v[0] ) ? 2 * v[1] : - 2 * v[1];// big adjustment
else
	if ( abs( v[2] - v[0] ) > v[1] )
		v[0] += ( v[2] > v[0] ) ? v[1] : - v[1];// small adjustment

v[0] = max( v[0], 0 );							// no negative nominal interest
v[0] += V( "rShock" );							// apply current shock level

RESULT( v[0] )


EQUATION( "rBonds" )
/*
Interest rate paid by government bonds to finance public debt
*/

v[0] = CURRENT;									// last period rate
v[1] = V( "rAdj" );								// rate adjustment step
v[2] = VLS( PARENT, "DebGDP", 1 );				// public debt over GDP

v[3] = ( 1 - V( "muBonds" ) ) * V( "r" );		// bonds base rate

// positive feedback on excessive public debt
if ( v[2] > 0 )
	v[3] *= 1 + V( "rhoBonds" ) * v[2];

// smooth rate adjustment
if ( abs( v[3] - v[0] ) > 2 * v[1] )
	v[0] += ( v[3] > v[0] ) ? 2 * v[1] : - 2 * v[1];// big adjustment
else
	if ( abs( v[3] - v[0] ) > v[1] )
		v[0] += ( v[3] > v[0] ) ? v[1] : - v[1];// small adjustment

RESULT( max( v[0], 0 ) )


EQUATION( "rD" )
/*
Interest rate paid by banks on deposits
*/
RESULT( ( 1 - V( "muD" ) ) *  V( "r" ) )


EQUATION( "rDeb" )
/*
Interest rate charged by banks on debt
Lower-bounded by the expected inflation rate
*/
RESULT( max( ( 1 + V( "muDeb" ) ) * V( "r" ), V( "piT" ) ) )


EQUATION( "rRes" )
/*
Interest rate paid by central bank to banks' reserves
*/
RESULT( ( 1 - V( "muRes" ) ) * V( "r" ) )


EQUATION( "rShock" )
/*
Shock to prime interest rate for impulse response function computation
*/
RESULT( VS( PARENT, "flagShock" ) && T == V( "Tshock" ) ?
		V( "r0shock" ) : ( 1 - V( "rhoShock" ) ) * CURRENT )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "BD" )
/*
Sovereign bond demand from banks
*/
V( "NWb" );										// ensure bank demand is done
RESULT( SUM( "_BD" ) )


EQUATION( "BadDeb" )
/*
Total losses from bad debt in financial sector
This variable must be explicitly recalculated after entry/exit
*/
RESULT( V( "BadDeb1" ) + V( "BadDeb2" ) )


EQUATION( "BadDeb1" )
/*
Total bad debt (defaults) in financial sector from capital-good sector
This variable must be explicitly recalculated after entry/exit
*/
RESULT( SUM( "_BadDeb1" ) )


EQUATION( "BadDeb2" )
/*
Total bad debt (defaults) in financial sector from consumption-good sector
This variable must be explicitly recalculated after entry/exit
*/
RESULT( SUM( "_BadDeb2" ) )


EQUATION( "BondsB" )
/*
Total sovereign bonds stock hold by financial sector
*/
V( "NWb" );										// ensure bank demand is done
RESULT( SUM( "_BondsB" ) )


EQUATION( "BondsCB" )
/*
Total sovereign bonds (residual) stock hold by central bank
Central bank absorbs all outstanding bonds issued
*/
V( "NWb" );										// ensure bank demand is done
RESULT( ROUND( CURRENT * ( 1 - 1 / V( "thetaBonds" ) ) + V( "BS" ) - V( "BD" ),
			   0, 0.001 ) )


EQUATION( "Cl" )
/*
Total number of firm clients (customers) of the banking sector
*/
RESULT( SUM( "_Cl" ) )


EQUATION( "Depo" )
/*
Total banking sector deposits
Net deposits from exiting and entering firms in period not considered
*/
RESULT( SUM( "_Depo" ) )						// sum-up banks deposits


EQUATION( "DivB" )
/*
Total banking sector distributed dividends
*/
V( "PiB" );										// make sure it is updated
RESULT( SUM( "_DivB" ) )						// sum-up banks dividends


EQUATION( "ExRes" )
/*
Excess reserves (free cash) hold by financial sector
*/
RESULT( SUM( "_ExRes" ) )


EQUATION( "Gbail" )
/*
Total government bailout funds to banking sector
*/
V( "NWb" );										// make sure it is updated
RESULT( SUM( "_Gbail" ) )						// sum-up banks bailouts


EQUATION( "Loans" )
/*
Total banking sector loans (non-defaulted)
Net loans to exiting and entering firms in period not considered
*/
RESULT( SUM( "_Loans" ) )


EQUATION( "LoansCB" )
/*
Total liquidity loans from central bank to banking sector
*/
RESULT( SUM( "_LoansCB" ) )


EQUATION( "NWb" )
/*
Total banking sector net worth (liquid assets)
*/
RESULT( SUM( "_NWb" ) )							// sum-up banks net worth


EQUATION( "PiB" )
/*
Total banking sector profits (losses)
*/
RESULT( SUM( "_PiB" ) )


EQUATION( "PiCB" )
/*
Central bank operational surplus (deficit)
*/
RESULT( VL( "r", 1 ) * VL( "LoansCB", 1 ) +
		VL( "rBonds", 1 ) * VL( "BondsCB", 1 ) -
		VL( "rRes", 1 ) * ( VL( "Res", 1 ) + VL( "DepoG", 1 ) ) )


EQUATION( "Res" )
/*
Total reserves of financial sector at the Central Bank
*/
RESULT( SUM( "_Res" ) )


EQUATION( "TaxB" )
/*
Total taxes paid by banks in financial sector
*/
V( "PiB" );										// make sure it is updated
RESULT( SUM( "_TaxB" ) )


EQUATION( "iB" )
/*
Interest on loans received by financial sector
*/
RESULT( SUM( "_iB" ) )


EQUATION( "iDb" )
/*
Interest on deposits paid by financial sector
*/
RESULT( SUM( "_iDb" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "banksMaps" )
/*
Updates the table of market share cumulative weights, used by firms when
choosing a bank
*/

// clear vectors
EXEC_EXTS( PARENT, countryE, bankPtr, clear );
EXEC_EXTS( PARENT, countryE, bankWgtd, clear );

// add-up market share
i = 0;											// bank index in vector
v[0] = v[1] = 0;								// cumulative market share
CYCLE( cur, "Bank" )
{
	EXEC_EXTS( PARENT, countryE, bankPtr, push_back, cur );// pointer to bank

	v[1] += v[2] = max( VS( cur, "_fD" ), 0 );
	EXEC_EXTS( PARENT, countryE, bankWgtd, push_back, v[2] );

	++i;
}

// rescale the shares to 1 (just in case) and accumulate them
for ( j = 0; j < i; ++j )
{
	v[0] += V_EXTS( PARENT, countryE, bankWgtd[ j ] ) / v[1];
	WRITE_EXTS( PARENT, countryE, bankWgtd[ j ], min( v[0], 1 ) );
}

RESULT( i )


EQUATION( "cScores" )
/*
Define the credit class for both sectors' firms and adjust the
pecking order of bank clients according to the credit scores
*/

firmRank firmData;
firmLisT rank1, rank2;

CYCLES( CAPSECL1, cur, "Firm1" )				// rank entire sector 1
{
	v[1] = VLS( cur, "_NW1", 1 );				// net worth
	v[2] = VLS( cur, "_S1", 1 );				// sales
	firmData.NWtoS = ( v[1] > 0 && v[2] > 0 ) ? v[1] / v[2] : 0;
	firmData.firm = cur;
	rank1.push_back( firmData );
}

CYCLES( CONSECL1, cur, "Firm2" )				// rank entire sector 2
{
	v[1] = VLS( cur, "_NW2", 1 );				// net worth
	v[2] = VLS( cur, "_S2", 1 );				// sales
	firmData.NWtoS = ( v[1] > 0 && v[2] > 0 ) ? v[1] / v[2] : 0;
	firmData.firm = cur;
	rank2.push_back( firmData );
}

// order global rankings in descending order
rank1.sort( rank_desc_NWtoS );
rank2.sort( rank_desc_NWtoS );

// attribute the pecking order in both sectors
h = 0;
for ( auto itr = rank1.begin( ); itr != rank1.end( ); ++h, ++itr )
	WRITES( itr->firm, "_pOrd1", h + 1 );		// overall sector pecking order

h = 0;
for ( auto itr = rank2.begin( ); itr != rank2.end( ); ++h, ++itr )
	WRITES( itr->firm, "_pOrd2", h + 1 );

// sort the firm objects so credit is requested first by top ranked firms
SORTS( CAPSECL1, "Firm1", "_pOrd1", "UP" );
SORTS( CONSECL1, "Firm2", "_pOrd2", "UP" );

RESULT( SUM( "_cScores" ) )						// update banks' scores


EQUATION( "pickBank" )
/*
Pick a bank randomly with probability proportional to the desired market shares
of banks
*/

//V( "banksMaps" );								// ensure vector updated

dblVecT *weight = & V_EXTS( PARENT, countryE, bankWgtd );// bank weights

// see which bank is in the RND position for accumulated market share
// in practice, it draws banks with probability proportional to m.s.
auto bank = upper_bound( weight->begin( ), weight->end( ), RND );

// calculate position (index) in the vector (same as bank ID - 1)
i = bank - weight->begin( ) + 1;

RESULT( i )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "DepoG", "BS" )
/*
Government deposits at central bank (accumulated surpluses)
Updated in 'BS'
*/
