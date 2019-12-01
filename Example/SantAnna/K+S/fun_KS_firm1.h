/******************************************************************************

	FIRM1 OBJECT EQUATIONS
	----------------------

	Equations that are specific to the Firm1 objects in the K+S LSD model 
	are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_Atau" )
/*
Productivity of the current vintage of machines when employed for production.
Also updates '_Btau'.
*/

double Btau = VL( "_Btau", 1 );					// previous period productivity
double xi = VS( PARENT, "xi" );					// share of R&D for innovation

// normalized share of workers on R&D of the firm
double L1rdShr = V( "_L1rd" ) * VS( LABSUPL2, "Ls0" ) / VLS( LABSUPL2, "Ls", 1 );

// innovation process (success probability)
v[1] = 1 - exp( - VS( PARENT, "zeta1" ) * xi * L1rdShr );

if ( bernoulli( v[1] ) )						// innovation succeeded?
{
	double x1inf = VS( PARENT, "x1inf" );		// lower beta inno. draw support 
	double x1sup = VS( PARENT, "x1sup" );		// upper beta inno. draw support 
	double alpha1 = VS( PARENT, "alpha1" );		// beta distrib. alpha parameter
	double beta1 = VS( PARENT, "beta1" );		// beta distrib. beta parameter
	
	// new final productivity (A) from innovation
	v[2] = CURRENT * ( 1 + x1inf + beta( alpha1, beta1 ) * ( x1sup - x1inf ) );
	
	// new production productivity (B) from innovation
	v[3] = Btau * ( 1 + x1inf + beta( alpha1, beta1 ) * ( x1sup - x1inf ) );
}
else
	v[2] = v[3] = 0;									// innovation failure

// imitation process (success probability)
v[4] = 1 - exp( - VS( PARENT, "zeta2" ) * ( 1 - xi ) * L1rdShr ); 

if ( bernoulli( v[4] ) )						// imitation succeeded?
{
	k = VS( PARENT, "F1" );						// number of firms in sector 1
	dblVecT imiProb( k );						// vector for tech distance
	
	v[5] = i = 0;								// inverse distance/firm accum.
	CYCLES( PARENT, cur, "Firm1" )				// 1st run: abs. inv. distance
		if ( cur == p )
			imiProb[ i++ ] = 0;					// can't self-imitate
		else
		{
			v[6] = sqrt( max( pow( VLS( cur, "_Btau", 1 ) - Btau, 2 ) +
							   pow( VLS( cur, "_Atau", 1 ) - CURRENT, 2 ), 0 ) );
			v[5] += imiProb[ i++ ] = ( v[6] > 0 ) ? 1 / v[6] : 0;
		}

	if ( v[5] > 0 )
	{
		v[7] = i = 0;							// probabilities/firm accum.
		CYCLES( PARENT, cur, "Firm1" )			// 2nd run: cumulative imi. prob.
		{
			v[7] += imiProb[ i ] / v[5];		// normalize to add up to 1
			imiProb[ i++ ] = v[7];
		}
			
		// draw a firm to imitate according to the distance probabilities
		j = upper_bound( imiProb.begin( ), imiProb.end( ), RND ) - imiProb.begin( );
		
		if ( j < k )							// at least one firm reachable?
		{
			cur = TSEARCHS( PARENT, "Firm1", j + 1 );// get pointer to firm
			
			v[8] = VLS( cur, "_Atau", 1 );		// get imitated firm productivities
			v[9] = VLS( cur, "_Btau", 1 );
		}
		else
			v[8] = v[9] = 0;					// imitation failure
	}
	else
		v[8] = v[9] = 0;						// imitation failure
}
else
	v[8] = v[9] = 0;							// imitation failure

// select best option between the three options (current/innovation/imitation)
v[0] = CURRENT;									// current technology
v[10] = Btau;
v[11] = v[12] = 0;

if ( v[2] * v[3] > v[0] * v[10] )				// is innovation better?
{
	v[0] = v[2];								// new Atau
	v[10] = v[3];								// new Btau
	v[11] = 1;									// innovation succeeded
	v[12] = 0;									// no imitation
}

if ( v[8] * v[9] > v[0] * v[10] )				// is imitation better (yet)?
{
	v[0] = v[8];
	v[10] = v[9];
	v[11] = 0;									// no innovation
	v[12] = 1;									// imitation succeeded
}

WRITE( "_Btau", v[10] );
WRITE( "_inn", v[11] );
WRITE( "_imi", v[12] );

// add entry to the map of vintage productivity and skill
i = V( "_ID1" );								// firm ID
j = VNT( t, i );								// vintage ID
h = VS( GRANDPARENT, "flagWorkerLBU" );			// learning mode
if ( h == 0 || h == 2 )							// no learning by using
	v[13] = 1;									// maximum skills
else
	v[13] = VS( PARENT, "sigma" );

WRITE_EXTS( GRANDPARENT, country, vintProd[ j ].sVp, v[13] );
WRITE_EXTS( GRANDPARENT, country, vintProd[ j ].sVavg, v[13] );

RESULT( v[0] )


EQUATION( "_Deb1max" )
/*
Prudential maximum bank debt of firm in capital-good sector
*/

// maximum debt allowed to firm, considering net worth and operating margin
v[5] = VS( FINSECL2, "Lambda" ) * max( VL( "_NW1", 1 ), 
									   VL( "_S1", 1 ) - VL( "_W1", 1 ) );
		   
// apply an absolute floor to maximum debt prudential limit
v[0] = max( v[5], VS( FINSECL2, "Lambda0" ) * VLS( PARENT, "PPI", 1 ) / 
				  VS( PARENT, "PPI0" ) );
			
WRITE( "_cred1c", 0 );							// reset constraint for period

RESULT( v[0] )
		

EQUATION( "_NC" )
/*
Number of new client firms in the period.
Also creates the client-supplier connecting objects.
*/

firmMapT firms = V_EXTS( GRANDPARENT, country, firm2map );// list with all firms
h = firms.size( );								// number of firms in sector 2
k = V( "_HC" );									// number of historical clients
j = V( "_ID1" );								// current firm ID
v[1] = VS( PARENT, "gamma" );					// new customer share
v[2] = VS( PARENT, "F1" );						// firms in sector 1

CYCLE( cur, "Cli" )								// remove historical clients
	firms.erase( ( int ) VS( cur, "_IDc" ) );
	
i = ceil( v[1] * k );							// new clients in period
i = v[0] = min( max( i, 1 ), min( h - k, firms.size( ) ) );// between [1, F2 - HC]

v[3] = max( 1, ceil( h / v[2] ) );				// firm fair share

if ( k + i < v[3] )								// ensure at least fair share
	i = v[0] = v[3] - k;

// build vector of all target firms (not yet clients)
vector < firmPairT > targets( firms.begin( ), firms.end( ) );

// draw new clients from target list, updating the list after each draw
for ( ; i > 0; --i )
{
	h = uniform_int( 0, targets.size( ) - 1 );	// draw a index in the list
	firmPairT client = targets[ h ];			// retrieve drawn map pair
	targets.erase( targets.begin( ) + h );		// remove drawn firm from list

	cur1 = ADDOBJ( "Cli" );						// add object to new client
	WRITES( cur1, "_IDc", client.first );		// update client ID
	WRITES( cur1, "_tSel", t );					// update selection time

	cur2 = ADDOBJS( client.second, "Broch" );	// add brochure to client
	WRITES( cur2, "_IDs", j );					// update supplier ID
	WRITE_SHOOKS( cur2, cur1 );					// pointer to supplier client list
	WRITE_SHOOKS( cur1, cur2 );					// pointer to client brochure list
}

RESULT( v[0] )


EQUATION( "_Q1" )
/*
Planed production for a firm in capital-good sector
*/

v[1] = V( "_D1" );								// potential production (orders)
v[2] = V( "_cred1" );							// available credit
v[3] = VL( "_NW1", 1 );							// net worth (cash available)
v[4] = V( "_c1" );								// unit cost
v[5] = V( "_p1" );								// machine price
v[6] = V( "_RD" );								// R&D costs still to pay

v[7] = v[1] * ( v[4] - v[5] ) + v[6];			// cash flow to fulfill orders

if ( v[7] < 0 || v[7] <= v[3] - 1 )				// firm can self-finance?
{
	v[0] = v[1];								// plan the desired output
	v[3] -= v[7];								// remove flow from cash
}
else
{
	if ( v[7] <= v[3] - 1 + v[2] )				// possible to finance all?
	{
		v[0] = v[1];							// plan the desired output
		v[8] = v[9] = v[7] - v[3] + 1;			// finance the difference
		v[3] = 1;								// keep minimum cash
	}
	else										// credit constrained firm
	{
		// produce as much as the available finance allows
		v[0] = floor( ( v[3] - 1 - v[7] + v[2] ) / v[4] );// max possible
		v[0] = min( max( v[0], 0 ), v[1] );		// positive but up to D1
		
		v[8] = v[2];							// finance what is possible
		v[9] = v[6] - v[3] + 1;					// desired credit
	
		if ( v[0] == 0 )
		{
			v[10] = 1;							// all orders canceled
			v[3] -= v[6] - v[2];				// let negative NW (bankruptcy)
		}
		else
		{
			v[10] = 1 - v[0] / v[1];			// machine shortage factor
			v[3] = 1;							// keep minimum cash
		}
		
		// shrink or cancel all exceeding orders
		CYCLE( cur, "Cli" )
			if ( VS( cur, "_tOrd" ) == t )		// order in this period?
			{
				if ( v[10] == 1 )				// bankruptcy?
					INCRS( cur, "_nCan", VS( cur, "_nOrd" ) );
				else
					INCRS( cur, "_nCan", floor( VS( cur, "_nOrd" ) * v[10] ) );
			}
	}
	
	update_debt1( p, v[9], v[8] );				// update firm debt
}

// provision for revenues and expenses
WRITE( "_NW1", v[3] );							// update the firm net worth

RESULT( v[0] )


EQUATION( "_RD" )
/*
R&D expenditure of firm in capital-good sector
*/

v[1] = VL( "_S1", 1 );							// sales in previous period
v[2] = VS( PARENT, "nu" );						// R&D share of sales

if ( v[1] > 0 )
	v[0] = v[2] * v[1];
else											// no sales
	// keep current expenditure or a share of available cash
	v[0] = max( CURRENT, v[2] * VL( "_NW1", 1 ) );
	
// always hire at least one worker
RESULT( max( v[0], VLS( PARENT, "w1avg", 1 ) ) )


EQUATION( "_Tax1" )
/*
Total tax paid by firm in capital-good sector
Also updates final net wealth on period
*/

v[1] = V( "_Pi1" );								// firm profit in period
v[2] = VS( GRANDPARENT, "tr" );					// tax rate

if ( v[1] > 0 )									// profits?
{
	v[0] = v[1] * v[2];							// tax to government
	v[4] = VS( PARENT, "d1" ) * ( v[1] - v[0] );// dividend to shareholders
}
else
	v[0] = v[4] = 0;							// no tax/dividend on losses

WRITE( "_Div1", v[4] );							// save period dividends

// compute free cash flow
v[6] = v[1] - v[0] - v[4];

// remove from net wealth the provision for revenues and expenses
v[7] = INCR( "_NW1", V( "_Q1" ) * ( V( "_c1" ) - V( "_p1" ) ) + V( "_RD" ) );

if ( v[6] < 0 )									// must finance losses?
{
	if ( v[7] >= - v[6] + 1 )					// can cover losses with reserves?
		INCR( "_NW1", v[6] );					// draw from net wealth
	else
	{
		v[8] = V( "_cred1" );					// available credit
		v[9] = - v[6] - v[7] + 1;				// desired finance
		
		if ( v[8] >= v[9] )						// can finance losses?
		{
			update_debt1( p, v[9], v[9] );		// finance all
			WRITE( "_NW1", 1 );					// minimum net wealth
		}
		else
		{
			update_debt1( p, v[8], v[8] );		// take what is possible
			INCR( "_NW1", v[6] - v[8] );		// let negative NW (bankruptcy exit)
		}					
	}
}
else											// pay debt with available cash
{
	v[10] = V( "_Deb1" );						// current debt
	
	if ( v[10] > 0 )							// has debt?
	{
		if ( v[6] > v[10] )						// can repay all debt and more
		{
			update_debt1( p, 0, - v[10] );		// zero debt
			INCR( "_NW1", v[6] - v[10] );		// save the rest
		}
		else
			update_debt1( p, 0, - v[6] );		// repay part of debt
	}
	else
		INCR( "_NW1", v[6] );					// save all
}
		
RESULT( v[0] )


EQUATION( "_p1" )
/*
Price of good of firm in capital-good sector
*/
RESULT( ( 1 + VS( PARENT, "mu1" ) ) * V( "_c1" ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_D1" )
/*
Potential demand (orders) received by a firm in capital-good sector
*/

VS( CONSECL2, "Id" );							// make sure all orders are sent

j = v[0] = 0;									// machine/active customer count
CYCLE( cur, "Cli" )
{
	if ( VS( cur, "_tOrd" ) == t )				// order in this period?
	{
		v[0] += VS( cur, "_nOrd" );
		++j;
	}
}

WRITE( "_BC", j );

RESULT( v[0] )


EQUATION( "_HC" )
/*
Number of historical client firms from consumer-good sector.
Also removes old, non-buying clients.
*/

j = V( "_ID1" );								// current firm ID

i = 0;											// client counter
CYCLE_SAFE( cur, "Cli" )						// remove old clients
{
	if ( VS( cur, "_tSel" ) < t - 1 )			// last selection is old?
	{
		DELETE( SHOOKS( cur ) );				// remove supplier brochure entry
		DELETE( cur );							// remove client entry
	}
	else
		++i;
}

RESULT( i )


EQUATION( "_L1" )
/*
Labor employed by firm in capital-good sector
Includes R&D labor
*/

// total workers in firm after possible shortage
v[0] = round( V( "_L1d" ) * VS( PARENT, "L1" ) / VS( PARENT, "L1d" ) );
			  
// adjust R&D count if supply is very limiter (< L1rd)
if ( v[0] < V( "_L1rd" ) )
	WRITE( "_L1rd", v[0] );

RESULT( v[0] )


EQUATION( "_L1d" )
/*
Labor demand of firm in capital-good sector
Includes R&D labor
*/

v[1] = V( "_L1rd" );							// R&D demand
v[2] = ceil( V( "_Q1" ) / ( V( "_Btau" ) * VS( PARENT, "m1" ) ) );
												// machine production demand
RESULT( v[1] + v[2] )


EQUATION( "_L1rd" )
/*
R&D labor demand of firm in capital-good sector
*/
RESULT( ceil( V( "_RD" ) / VLS( PARENT, "w1avg", 1 ) ) )


EQUATION( "_Pi1" )
/*
Profit of firm in capital-good sector
*/

v[1] = V( "_S1" ) - V( "_W1" );					// gross operating margin
v[2] = VS( FINSECL2, "rD" ) * VL( "_NW1", 1 );	// financial income

// firm effective interest rate on debt
v[3] = VLS( FINSECL2, "rDeb", 1 ) * ( 1 + ( VL( "_qc1", 1 ) - 1 ) * 
	   VS( FINSECL2, "kConst" ) ); 

v[4] = v[3] * VL( "_Deb1", 1 );					// interest to pay

RESULT( v[1] + v[2] - v[4] )					// firm profits before taxes


EQUATION( "_Q1e" )
/*
Effective output of firm in capital-good sector
*/

v[0] = V( "_Q1" );								// planned production
j = V( "_L1" );									// effective labor available
k = V( "_L1d" );								// desired total workers

if ( j >= k || 									// required labor available?
	 VS( GRANDPARENT, "flagMachDeliv" ) == 0 )
	END_EQUATION( v[0] );						// produce as planned
	
h = V( "_L1rd" );								// workers not in production

v[1] = 1 - ( double ) max( j - h, 0 ) / ( k - h );// adjustment factor

// adjust all pending orders, supplying at least one machine
CYCLE( cur, "Cli" )
	if ( VS( cur, "_tOrd" ) == t )				// order in this period?
	{
		v[2] = VS( cur, "_nOrd" ) - VS( cur, "_nCan" );// existing net orders
		v[0] -= v[3] = min( floor( v[2] * v[1] ), v[2] - 1 );
		INCRS( cur, "_nCan", v[3] );
	}
	
RESULT( max( v[0], 0 ) )						// avoid negative in part. cases


EQUATION( "_S1" )
/*
Sales of firm in capital-good sector
*/
RESULT( V( "_p1" ) * V( "_Q1e" ) )


EQUATION( "_W1" )
/*
Total wages paid by firm in capital-good sector
*/
RESULT( V( "_L1" ) * VS( PARENT, "w1avg" ) )


EQUATION( "_c1" )
/*
Unit cost of production (1 machine) of firm in capital-good sector.
Use sectoral average (pool of sector 1 workers).
*/
V( "_Atau" );									// ensure innovation process ok
RESULT( VLS( PARENT, "w1avg", 1 ) / ( V( "_Btau" ) * VS( PARENT, "m1" ) ) )


EQUATION( "_f1" )
/*
Market share of firm in capital-good sector
Keep market share if sector didn't produce
*/
v[1] = VS( PARENT, "Q1e" );
RESULT( v[1] > 0 ? V( "_Q1e" ) / v[1] : CURRENT )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "_cred1" )
/*
Bank credit available (new debt) to firm in capital-good sector
Function called multiple times in single time step
*/

v[1] = V( "_Deb1" );							// current firm debt
v[2] = V( "_Deb1max" );							// maximum prudential credit

if ( v[2] > v[1] )								// more credit possible?
{
	v[0] = v[2] - v[1];							// potential free credit
	
	cur = HOOK( BANK );							// firm's bank
	v[3] = VS( cur, "_TC1free" );				// bank's available credit
	
	if ( v[3] > -0.1 )							// credit limit active
		v[0] = min( v[0], v[3] );				// take just what is possible
}
else
	v[0] = 0;									// no credit available

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_BC", "" )
/*
Number of buying clients for firm in capital-good sector
Updated in '_D1'
*/

EQUATION_DUMMY( "_Btau", "" )
/*
Productivity of labor in producing the current vintage of machines
Updated in '_Atau'
*/

EQUATION_DUMMY( "_Deb1", "" )
/*
Stock of bank debt of firm in capital-good sector
Updated in '_Q1', '_Tax1'
*/

EQUATION_DUMMY( "_Div1", "" )
/*
Dividends paid by firm in capital-good sector
Updated in '_Tax1'
*/

EQUATION_DUMMY( "_NW1", "" )
/*
Net wealth (free cash) of firm in capital-good sector
Updated in '_Q1' and '_Tax1'
*/

EQUATION_DUMMY( "_cred1c", "" )
/*
Credit constraint for firm in capital-good sector
Updated in '_Deb1max', '_Q1'
*/

EQUATION_DUMMY( "_imi", "" )
/*
Imitation success (1) or failure (0) for firm in capital-good sector
Updated in '_Atau'
*/

EQUATION_DUMMY( "_inn", "" )
/*
Innovation success (1) or failure (0) for firm in capital-good sector
Updated in '_Atau'
*/

EQUATION_DUMMY( "_qc1", "cScores" )
/*
Credit class of firm in sector 1 (1,2,3,4)
Updated in 'cScores'
*/
