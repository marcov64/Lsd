/******************************************************************************

	FINANCIAL MARKET OBJECT EQUATIONS
	---------------------------------

	Equations that are specific to the financial market object in the K+S  
	LSD model are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "BS" )
/*
Sovereign bond supply from government
Also update 'DepoG'
*/

v[0] = CURRENT - VL( "BD", 1 );					// current outstanding bonds
v[1] = VLS( PARENT, "Def", 1 );					// new bonds to issue
v[2] = VL( "DepoG", 1 );						// government deposits

if ( v[0] + v[1] - v[2] < 0 )					// no bonds to supply?
{
	v[2] -= v[0] + v[1];						// keep surplus at central bank
	v[0] = 0;
}
else
{
	v[0] = v[0] + v[1] - v[2];					// supply just what is needed
	v[2] = 0;
}

WRITE( "DepoG", v[2] );

RESULT( v[0] )


EQUATION( "r" )
/*
Interest rate set by the central bank (prime rate)
*/

v[0] = CURRENT;									// last period rate
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

RESULT( max( v[0], 0 ) )


EQUATION( "rBonds" )
/*
Interest rate paid by government bonds to finance public debt
*/

v[0] = CURRENT;									// last period rate
v[1] = V( "rAdj" );								// rate adjustment step
v[2] = VLS( PARENT, "DebGDP", 1 );				// public debt over GDP

// positive feedback on excessive public debt
if ( v[2] > 0 )
{
	v[3] = ( 1 - V( "muBonds" ) ) * V( "r" ) * ( 1 + V( "rhoBonds" ) * v[2] );
	
	// smooth rate adjustment
	if ( abs( v[3] - v[0] ) > 2 * v[1] )
		v[0] += ( v[3] > v[0] ) ? 2 * v[1] : - 2 * v[1];// big adjustment
	else
		if ( abs( v[3] - v[0] ) > v[1] )
			v[0] += ( v[3] > v[0] ) ? v[1] : - v[1];// small adjustment
}

RESULT( max( v[0], 0 ) )


EQUATION( "rD" )
/*
Interest rate paid by banks on deposits
*/
RESULT( ( 1 - V( "muD" ) ) *  V( "r" ) )


EQUATION( "rDeb" )
/*
Interest rate charged by banks on debt
*/
RESULT( ( 1 + V( "muDeb" ) ) * V( "r" ) )


EQUATION( "rRes" )
/*
Interest rate paid by central bank to banks' reserves
*/
RESULT( ( 1 - V( "muRes" ) ) * V( "r" ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "BD" )
/*
Sovereign bond demand from banks
*/
V( "NWb" );										// ensure bank demand is done
RESULT( SUM( "_BD" ) )


EQUATION( "BadDeb" )
/*
Total bad debt (defaults) in financial sector
*/
RESULT( SUM( "_BadDeb" ) )


EQUATION( "Bonds" )
/*
Total sovereign bonds stock hold by financial sector
*/
RESULT( SUM( "_Bonds" ) )


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


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "banksMaps" )
/*
Updates the table of market share cumulative weights, used by firms when 
choosing a bank
*/

// clear vectors
EXEC_EXTS( PARENT, country, bankPtr, clear );
EXEC_EXTS( PARENT, country, bankWgtd, clear );

// add-up market share
i = 0;											// bank index in vector
v[0] = v[1] = 0;								// cumulative market share
CYCLE( cur, "Bank" )
{
	EXEC_EXTS( PARENT, country, bankPtr, push_back, cur );// pointer to bank
	
	v[1] += v[2] = max( VS( cur, "_fD" ), 0 );
	EXEC_EXTS( PARENT, country, bankWgtd, push_back, v[2] );
	
	++i;
}

// rescale the shares to 1 (just in case) and accumulate them
for ( j = 0; j < i; ++j )
{
	v[0] += V_EXTS( PARENT, country, bankWgtd[ j ] ) / v[1];
	WRITE_EXTS( PARENT, country, bankWgtd[ j ], min( v[0], 1 ) );
}

RESULT( i )


EQUATION( "cScores" )
/*
Define the credit class for both sectors' firms and adjust the 
pecking order of bank clients according to the credit scores
*/

firmRank firmData;
firmLisT rankE, rank1, rank2;

CYCLES( ENESECL1, cur, "FirmE" )				// rank entire energy sector
{
	v[1] = VLS( cur, "_NWe", 1 );				// net worth
	v[2] = VLS( cur, "_Se", 1 );				// sales
	firmData.NWtoS = ( v[1] > 0 && v[2] > 0 ) ? v[1] / v[2] : 0;
	firmData.firm = cur;
	rankE.push_back( firmData );
}

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
rankE.sort( rank_desc_NWtoS );
rank1.sort( rank_desc_NWtoS );
rank2.sort( rank_desc_NWtoS );

// attribute the pecking order in all sectors
h = 0;
for ( auto itr = rankE.begin( ); itr != rankE.end( ); ++h, ++itr )
	WRITES( itr->firm, "_pOrdE", h + 1 );

for ( auto itr = rank1.begin( ); itr != rank1.end( ); ++h, ++itr )
	WRITES( itr->firm, "_pOrd1", h + 1 );		// overall sector pecking order

h = 0;
for ( auto itr = rank2.begin( ); itr != rank2.end( ); ++h, ++itr )
	WRITES( itr->firm, "_pOrd2", h + 1 );
	
// sort the firm objects so credit is requested first by top ranked firms
SORTS( ENESECL1, "FirmE", "_pOrdE", "UP" );
SORTS( CAPSECL1, "Firm1", "_pOrd1", "UP" );
SORTS( CONSECL1, "Firm2", "_pOrd2", "UP" );

RESULT( SUM( "_cScores" ) )						// update banks' scores


EQUATION( "pickBank" )
/*
Pick a bank randomly with probability proportional to the desired market shares
of banks
*/

//V( "banksMaps" );								// ensure vector updated

dblVecT *weight = & V_EXTS( PARENT, country, bankWgtd );// bank weights

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
*/
