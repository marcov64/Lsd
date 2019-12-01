/******************************************************************************

	FINANCIAL MARKET OBJECT EQUATIONS
	---------------------------------

	Equations that are specific to the financial market object in the K+S  
	LSD model are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "phi" )
/*
Unemployment benefit rate on average wages applied by government 
*/

v[1] = V( "phiT" );										// target unempl. benefit
v[2] = VLS( PARENT, "GDPnom", 1 );						// monetary GDP in t-1

switch ( ( int ) VS( PARENT, "flagFiscalRule" ) )		// fiscal rule to use
{
	case 0:												// fixed benefit
	default:
		v[0] = v[1];
		break;

	case 2:												// soft max deficit rule
		if ( VLS( PARENT, "dGDP", 1 ) < 0 )				// negative GDP growth?
		{
			v[0] = CURRENT;								// keep benefit
			break;
		}
			
	case 1:												// max deficit rule
		v[3] = ( v[2] > 0 ) ? VLS( PARENT, "Def", 1 ) / v[2] : 0;// def. on GDP
		if ( v[3] > V( "defRule" ) )
			v[0] = CURRENT - V( "rAdj" );				// reduce benefit
		else
			if ( CURRENT < v[1] )
				v[0] = min( CURRENT + V( "rAdj" ), v[1] );// increase benefit
			else
				v[0] = CURRENT;
		break;
		
	case 4:												// soft debt-reduction rule
		if ( VLS( PARENT, "dGDP", 1 ) < 0 )				// negative GDP growth?
		{
			v[0] = CURRENT;								// keep benefit
			break;
		}
			
	case 3:												// debt-reduction rule
		v[3] = ( v[2] > 0 ) ? VLS( PARENT, "Def", 1 ) / v[2] : 0;// def. on GDP
		v[4] = ( v[2] > 0 ) ? VLS( PARENT, "Deb", 1 ) / v[2] : 0;// debt on GDP
		
		if ( v[3] > V( "defRule" ) || v[4] > V( "debRule" ) )
			v[0] = CURRENT - V( "rAdj" );				// reduce benefit
		else
			if ( CURRENT < v[1] )
				v[0] = min( CURRENT + V( "rAdj" ), v[1] );// increase benefit
			else
				v[0] = CURRENT;
		break;
}

RESULT( max( v[0], 0 ) )								// no negative rate


EQUATION( "r" )
/*
Interest rate set by the central bank (prime rate)
*/

v[0] = CURRENT;											// last period rate
v[1] = V( "rAdj" );										// rate adjustment step

// Taylor rule
v[2] = V( "rT" ) + V( "gammaPi" ) * ( VLS( CONSECL1, "dCPI", 1 ) - V( "piT" ) ) + 
	   V( "gammaU" ) * ( V( "Ut" ) - VLS( LABSUPL1, "Ue", 1 ) );

if ( abs( v[2] - v[0] ) > v[1] )						// smooth rate adjustment
	v[0] += ( v[2] > v[0] ) ? v[1] : - v[1];
else
	v[0] = v[2];

RESULT( max( v[0], 0 ) )								// no negative interest


EQUATION( "rDeb" )
/*
Interest rate charged by banks on debt
*/
RESULT( V( "r" ) * ( 1 + V( "muDeb" ) ) )


EQUATION( "rRes" )
/*
Interest rate paid by central bank to banks' reserves
*/
RESULT( V( "r" ) * ( 1 + V( "muRes" ) ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "Cl" )
/*
Total number of firm clients (customers) of the banking sector
*/
RESULT( SUM( "_Cl" ) )


EQUATION( "Depo" )
/*
Total banking sector reserves (deposits on Central Bank)
*/
RESULT( SUM( "_Depo" ) )								// sum-up banks deposits


EQUATION( "DivB" )
/*
Total banking sector distributed dividends
*/
V( "PiB" );												// make sure it is updated
RESULT( SUM( "_DivB" ) )								// sum-up banks dividends	


EQUATION( "Gbail" )
/*
Total government bailout funds to banking sector
*/
V( "NWb" );												// make sure it is updated
RESULT( SUM( "_Gbail" ) )								// sum-up banks bailouts


EQUATION( "Loans" )
/*
Total banking sector loans (non-defaulted)
*/
RESULT( SUM( "_Loans" ) )


EQUATION( "NWb" )
/*
Total banking sector net worth (liquid assets)
*/
RESULT( SUM( "_NWb" ) )									// sum-up banks net worth


EQUATION( "PiB" )
/*
Total banking sector profits (losses)
*/
RESULT( SUM( "_PiB" ) )


EQUATION( "TaxB" )
/*
Total taxes paid by banks in financial sector
*/
V( "PiB" );												// make sure it is updated
RESULT( SUM( "_TaxB" ) )


EQUATION( "cScores" )
/*
Adjust the pecking order of firms according to the bank credit scores
Also define the credit class for the firms
*/

VS( CAPSECL1, "entry1exit" );							// ensure entry-exit done
VS( CONSECL1, "entry2exit" );

firmRank firmData;
firmLisT rank1, rank2;

CYCLES( CAPSECL1, cur, "Firm1" )						// build ranking for sector 1
{
	v[1] = VS( cur, "_NW1" );							// net worth
	v[2] = VS( cur, "_S1" );							// sales
	firmData.NWtoS = ( v[2] > 0 ) ? v[1] / v[2] : v[1];	// net-worth to sales ratio
	firmData.firm = cur;
	rank1.push_back( firmData );
}

CYCLES( CONSECL1, cur, "Firm2" )						// build ranking for sector 2
{
	v[1] = VS( cur, "_NW2" );							// net worth
	v[2] = VS( cur, "_S2" );							// sales
	firmData.NWtoS = ( v[2] > 0 ) ? v[1] / v[2] : v[1];	// net-worth to sales ratio
	firmData.firm = cur;
	rank2.push_back( firmData );
}

// order rankings in descending order
rank1.sort( rank_desc_NWtoS );
rank2.sort( rank_desc_NWtoS );

h = i = rank1.size( );									// # of firms in sector 1
j = 0; 													// firm counter
for ( firmLisT::iterator itr = rank1.begin( ); itr != rank1.end( ); ++j, ++itr )
{
	// find the credit class of firm
	k = ( j < i * 0.25 ) ? 1 : ( j < i * 0.5 ) ? 2 : ( j < i * 0.75 ) ? 3 : 4; 
	WRITES( itr->firm, "_qc1", k );						// save credit class
	WRITES( itr->firm, "_pOrd1", j + 1 );				// save pecking order
}

h += i = rank2.size( );									// # of firms in sector 2
j = 0; 													// firm counter
for ( firmLisT::iterator itr = rank2.begin( ); itr != rank2.end( ); ++j, ++itr )
{
    // find the credit class of firm
	k = ( j < i * 0.25 ) ? 1 : ( j < i * 0.5 ) ? 2 : ( j < i * 0.75 ) ? 3 : 4; 
	WRITES( itr->firm, "_qc2", k );						// save credit class
	WRITES( itr->firm, "_pOrd2", j + 1 );				// save pecking order
}

// sort the firm objects so credit is requested first by top ranking firms
SORTS( CAPSECL1, "Firm1", "_pOrd1", "UP" );
SORTS( CONSECL1, "Firm2", "_pOrd2", "UP" );

RESULT( h )


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
i = 0;													// bank index in vector
v[0] = v[1] = 0;										// cumulative market share
CYCLE( cur, "Bank" )
{
	EXEC_EXTS( PARENT, country, bankPtr, push_back, cur );// pointer to bank
	
	v[1] += v[2] = max( VS( cur, "_fd" ), 0 );
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


EQUATION( "pickBank" )
/*
Pick a bank randomly with probability proportional to the desired market shares
of banks
*/

//V( "banksMaps" );										// ensure vector updated

dblVecT *weight = & V_EXTS( PARENT, country, bankWgtd );// bank weights

// see which bank is in the RND position for accumulated market share
// in practice, it draws banks with probability proportional to m.s.
dblVecT::iterator bank = upper_bound( weight->begin( ), weight->end( ), RND );

// calculate position (index) in the vector (same as bank ID - 1)
i = bank - weight->begin( ) + 1;

RESULT( i )


/*============================= DUMMY EQUATIONS ==============================*/
