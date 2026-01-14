/******************************************************************************

	FIRM1 OBJECT EQUATIONS
	----------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the Firm1 objects in the K+S LSD model
	are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_Atau" )
/*
Labor productivity of new vintage of machine when employed for production
Also updates '_Btau'
*/

double Ainn, Binn, Aimi, Bimi, cImi, cInn, c2avg, g, pImi, pInn, p1avg;

double Atau = CURRENT;							// previous period productivity
double Btau = VL( "_Btau", 1 );
double b = VS( CONSECL2, "b" );					// payback period
double e = VLS( COUNTRL2, "e", 1 );				// exchange rate
double m1 = VS( PARENT, "m1" );					// modularity in sector 1
double m2 = VS( CONSECL2, "m2" );				// machine modularity in sector 2
double mu1 = VS( PARENT, "mu1" );				// mark-up in sector 1
double pTau = VL( "_p1", 1 );					// current price w/ exist. tech.
double w1avg = VLS( PARENT, "w1avg", 1 );		// current domestic wage in s. 1
double xi = VS( PARENT, "xi" );					// share of R&D for innovation
int flagEduc = VS( COUNTRL2, "flagEduc" );		// worker education setting
int flagTradeK = VS( COUNTRL2, "flagTradeK" );	// international trade setting

bool intlImi = ( flagTradeK <= 1 || flagTradeK == 3 ) ? false : true;
double w2avg = flagTradeK > 0 ? VLS( WORLDL2, "wCavgW", 1 ) * e :
								VLS( CONSECL2, "w2avg", 1 );// client wage ref.
double cTau = w2avg / Atau;						// cur. op. cost. w/ exist. tech.
cImi = cInn = pImi = pInn = DBL_MAX;			// assume innov./imit. failure

// normalized workers on R&D of the firm
double L1rdN = VL( "_L1rd", 1 ) * VS( LABSUPL2, "Ls0" ) / VLS( LABSUPL2, "Ls", 1 );

if ( flagEduc == 0 )
	g = 1;
else
	g = pow( VS( LABSUPL2, "epsilonEd" ) / VS( LABSUPL2, "epsilonAd" ),
			 VS( LABSUPL2, "varthetaEd" ) );	// gov. educ. expenditure effect

// innovation process (success probability)
v[1] = 1 - exp( - VS( PARENT, "zeta1" ) * g * xi * L1rdN );

if ( bernoulli( v[1] ) )						// innovation succeeded?
{
	double x1inf = VS( PARENT, "x1inf" );		// lower beta inno. draw support
	double x1sup = VS( PARENT, "x1sup" );		// upper beta inno. draw support
	double alpha1 = VS( PARENT, "alpha1" );		// beta distrib. alpha parameter
	double beta1 = VS( PARENT, "beta1" );		// beta distrib. beta parameter

	// new final productivity (A) from innovation
	Ainn = Atau * ( 1 + x1inf + beta( alpha1, beta1 ) * ( x1sup - x1inf ) );

	// new production productivity (B) from innovation
	Binn = Btau * ( 1 + x1inf + beta( alpha1, beta1 ) * ( x1sup - x1inf ) );

	// price and operating cost of new machine
	pInn = ( 1 + mu1 ) * w1avg / Binn / m1;
	cInn = w2avg / Ainn;
}

// imitation process (success probability)
v[2] = 1 - exp( - VS( PARENT, "zeta2" ) * g * ( 1 - xi ) * L1rdN );

if ( T > 1 && bernoulli( v[2] ) )				// imitation succeeded?
{
	if ( flagTradeK > 0 )						// international imitation?
	{
		c2avg = VLS( WORLDL2, "cCw", 1 ) * e;	// world machine operating cost
		p1avg = VLS( WORLDL2, "pKavgW", 1 ) * e;// average world machine price
		k = VS( WORLDL2, "FkW" );				// number of s.1 firms worldwide
	}
	else
	{
		c2avg = VLS( CONSECL2, "c2", 1 );		// nat'l machine operating cost
		p1avg = VLS( PARENT, "p1avg", 1 );		// average nat'l machine price
		k = VS( PARENT, "F1" );					// number of s.1 firms in country
	}

	dblVecT imiProb( k );						// vector for tech distance
	intVecT firmEnd( 1, 0 );					// vector country end firm

	// look for firms in the capital sector of all countries
	v[3] = i = 0;								// inverse distance/firm accum.
	CYCLES( WORLDL2, cur1, "Country" )			// 1st run: abs. inv. dist.
		if ( intlImi || cur1 == COUNTRL2 )		// consider country?
			CYCLES( V_EXTS( cur1, countryE, capSec ), cur, "Firm1" )
			{
				if ( cur == THIS )
					imiProb[ i++ ] = 0;			// can't self-imitate
				else
				{
					// price and operating cost of firm candidate for imitation
					double p = ( 1 + mu1 ) * w1avg / VLS( cur, "_Btau", 1 ) / m1;
					double c = w2avg / VLS( cur, "_Atau", 1 );

					// euclidian distance in 2-dimensional mean-standardized space
					v[4] = sqrt( pow( ( p - pTau ) / p1avg, 2 ) +
								 pow( ( c - cTau ) / c2avg, 2 ) );

					v[3] += imiProb[ i++ ] = ( v[4] > 0 ) ? 1 / v[4] : 0;
				}
			}

	if ( v[3] > 0 )
	{
		v[5] = i = 0;							// probabilities/firm accum.
		CYCLES( WORLDL2, cur1, "Country" )		// 2nd run: cumulative imi. prob.
			if ( intlImi || cur1 == COUNTRL2 )	// consider country?
			{
				CYCLES( V_EXTS( cur1, countryE, capSec ), cur, "Firm1" )
				{
					v[5] += imiProb[ i ] / v[3];// normalize to add up to 1
					imiProb[ i++ ] = v[5];
				}

				firmEnd.push_back( i );			// first after last country firm
			}

		// draw a firm to imitate according to the distance probabilities
		j = upper_bound( imiProb.begin( ), imiProb.end( ), RND ) - imiProb.begin( );

		if ( j < k )							// at least one firm reachable?
		{
			h = 0;
			CYCLES( WORLDL2, cur1, "Country" )	// 3rd run: find drawn country
				if ( intlImi || cur1 == COUNTRL2 )// consider country?
					if ( j < firmEnd[ ++h ] )	// firm in this country?
					{
						cur = TSEARCHS( V_EXTS( cur1, countryE, capSec ), "Firm1",
										j - firmEnd[ h - 1 ] + 1 );// firm pointer
						break;
					}

			if ( flagEduc == 0 )
				v[5] = 1;
			else
				v[5] = pow( VS( LABSUPL2, "epsilonEd" ) /
							VS( SEARCHS( cur1, "Labor" ), "epsilonEd" ),
							VS( LABSUPL2, "varthetaEd" ) );

			Aimi = v[5] * VLS( cur, "_Atau", 1 );// get imitated firm productivities
			Bimi = v[5] * VLS( cur, "_Btau", 1 );

			// price and operating cost of new machine
			pImi = ( 1 + mu1 ) * w1avg / Bimi / m1;
			cImi = w2avg / Aimi;
		}
	}
}

// select best option between the three options (current/innovation/imitation)
v[6] = v[7] = 0;								// innovation/imitation flags

// is innovation ownership unit cost lower than current tech for client?
if ( pInn / m2 + cInn * b < pTau / m2 + cTau * b )
{
	pTau = pInn;								// use it
	cTau = cInn;
	Atau = Ainn;
	Btau = Binn;
	v[6] = 1;									// innovation succeeded
}

// is imitation ownership unit cost even lower?
if ( pImi / m2 + cImi * b < pTau / m2 + cTau * b )
{
	Atau = Aimi;
	Btau = Bimi;
	v[6] = 0;									// no innovation
	v[7] = 1;									// imitation succeeded
}

WRITE( "_Btau", Btau );
WRITE( "_inn", v[6] );
WRITE( "_imi", v[7] );

// add entry to the map of vintage productivity and skill
i = V( "_ID1" );								// firm ID
j = VNT( T, i );								// vintage ID
h = VS( COUNTRL2, "flagWorkerLBU" );			// learning mode
if ( h == 0 || h == 2 )							// no learning by using
	v[8] = 1;									// maximum skills
else
	v[8] = VS( PARENT, "sigma" );

// block access to vintProd from other parallel threads
mtxLckT lock( V_EXTS( COUNTRL2, countryE, vintProdMtx ) );

WRITE_EXTS( COUNTRL2, countryE, vintProd[ j ].sVp, v[8] );
WRITE_EXTS( COUNTRL2, countryE, vintProd[ j ].sVavg, v[8] );

RESULT( Atau )


EQUATION( "_Deb1max" )
/*
Prudential maximum bank debt of firm in capital-good sector
Also updates '_CD1', '_CD1c', '_CS1'
*/

// prudential credit multiple, differentiated for government-owned firms
v[1] = V( "_own1" ) == 1 ? VS( FINSECL2, "LambdaG" ) : VS( FINSECL2, "Lambda" );

// maximum debt allowed to firm, considering net worth and operating margin
v[2] = v[1] * max( VL( "_NW1", 1 ), VL( "_S1l", 1 ) +
									VL( "_S1x", 1 ) - VL( "_W1", 1 ) );

// apply an absolute floor to maximum debt prudential limit
v[0] = max( v[2], VS( FINSECL2, "Lambda0" ) * VLS( PARENT, "PPI", 1 ) /
				  VS( PARENT, "pK0" ) );

WRITE( "_CD1", 0 );								// reset total credit demand
WRITE( "_CD1c", 0 );							// reset constraint for period
WRITE( "_CS1", 0 );								// reset total credit supplied

RESULT( v[0] )


EQUATION( "_Div1" )
/*
Dividends to pay by firm in capital-good sector
*/
RESULT( max( VS( PARENT, "d1" ) * ( V( "_Pi1" ) - V( "_Tax1" ) ), 0 ) )


EQUATION( "_NC" )
/*
Number of new client firms in the period.
Also creates the client-supplier connecting objects.
*/

firmMapT firms, *firmsC;						// firm lists (world/country)

i = VS( COUNTRL2, "flagTradeK" );				// international trade mode
v[1] = 0;										// world number sector 1 firms

CYCLES( WORLDL2, cur, "Country" )				// search all countries
	if ( i > 0 || cur == COUNTRL2 )				// international trade?
	{
		v[1] += VS( V_EXTS( cur, countryE, capSec ), "F1" );// country s.1 firm #
		firmsC = & V_EXTS( cur, countryE, firm2map );// country's s.2 firms
		firms.insert( firmsC->begin( ), firmsC->end( ) );// add to world list
	}

k = firms.size( );								// number firms in all sectors 2
h = V( "_HC" );									// number of historical clients

CYCLE( cur, "Cli" )								// remove historical clients
	firms.erase( ( int ) VS( cur, "__IDc" ) );

i = ceil( VS( PARENT, "gamma" ) * h );			// new clients in period
i = min( max( i, 1 ), min( k - h, firms.size( ) ) );// in [1, F2 - HC]

j = max( 1, ceil( k / v[1] ) );					// firm world fair share

if ( h + i < j )								// ensure at least fair share
	i = j - h;

// build vector of all target firms (not yet clients)
firmPairVecT targets( firms.begin( ), firms.end( ) );

// draw new clients from target list, updating the list after each draw
for ( k = 0; i > 0 && targets.size( ) > 0; --i, ++k )
{
	h = uniform_int( 0, targets.size( ) - 1 );	// draw a index in the list
	firmPairT client = targets[ h ];			// retrieve drawn map pair
	targets.erase( targets.begin( ) + h );		// remove drawn firm from list

	// create the brochure/client interconnected objects
	CFUN( send_brochure, client.second );
}

RESULT( k )


EQUATION( "_Q1" )
/*
Planed production for a firm in capital-good sector
Also updates '_NW1', '_NW1p', '_Deb1', '_CD1', '_CD1c', '_CS1'
*/

v[1] = V( "_D1l" ) + V( "_D1x" );				// potential production (orders)
v[2] = V( "_CS1a" );							// available credit supply
v[3] = VL( "_NW1", 1 );							// net worth (cash available)
v[4] = V( "_c1" );								// unit cost
v[5] = V( "_p1" );								// machine price
v[6] = V( "_RD" );								// R&D costs still to pay

v[7] = v[1] * ( v[4] - v[5] ) + v[6];			// cash to fulfill orders & R&D

if ( v[7] < 0 || v[7] <= v[3] )					// firm can self-finance?
{
	v[0] = v[1];								// plan the desired output
	v[8] = v[3] - v[7];							// remove wage cost from cash
	v[9] = 0;									// no finance
}
else
{
	if ( v[7] <= v[3] + v[2] )					// possible to finance all?
	{
		v[0] = v[1];							// plan the desired output
		v[8] = 0;								// no cash
		v[9] = v[10] = v[7] - v[3];				// finance the difference
	}
	else										// credit constrained firm
	{
		// produce as much as the available finance allows, positive up to demand
		v[0] = min( max( floor( ( v[3] + v[2] - v[6] ) / v[4] ), 0 ), v[1] );
		v[10] = v[7] - v[3];					// desired credit

		if ( v[0] == 0 )
		{
			if ( v[6] <= v[3] )					// can pay R&D?
			{
				v[8] = v[3] - v[6];				// remove R&D wage cost from cash
				v[9] = v[10] = 0;				// no finance
			}
			else								// always finance at least R&D
			{
				v[8] = 0;						// no cash
				v[9] = v[6] - v[3];				// always finance at least R&D
			}

			v[11] = 1;							// all orders canceled
		}
		else
		{
			v[7] = v[0] * ( v[4] - v[5] ) + v[6];// reduced operation cost
			if ( v[7] <= v[3] )
			{
				v[8] = v[3] - v[7];				// pay with available cash
				v[9] = 0;						// no finance
			}
			else
			{
				v[8] = 0;						// no cash
				v[9] = v[7] - v[3];				// finance the difference
			}

			v[11] = 1 - v[0] / v[1];			// machine shortage factor
		}

		// shrink or cancel all exceeding orders
		CYCLE( cur, "Cli" )
			if ( VS( cur, "__tOrd" ) == T )		// order in this period?
				INCRS( cur, "__nCan", floor( VS( cur, "__nOrd" ) * v[11] ) );
	}

	CFUN( update_debt, v[10], v[9] );			// update debt (desired/granted)
}

CFUN( update_depo, v[8], false );				// update the firm net worth
WRITE( "_NW1p", v[3] - v[8] + v[9] );			// provision for production

RESULT( v[0] )


EQUATION( "_RD" )
/*
R&D expenditure of firm in capital-good sector
*/

v[1] = VL( "_S1l", 1 ) + VL( "_S1x", 1 );		// sales in previous period
v[2] = V( "_own1" ) == 1 ? VS( PARENT, "nuG" ) : VS( PARENT, "nu" );
												// R&D share of sales
if ( v[1] > 0 )
	v[0] = v[2] * v[1];
else											// no sales
	// keep current expenditure or a share of available cash
	v[0] = min( CURRENT, v[2] * VL( "_NW1", 1 ) );

// always hire at least one worker
RESULT( max( v[0], VLS( PARENT, "w1avg", 1 ) ) )


EQUATION( "_Tax1" )
/*
Tax on profits paid by firm in capital-good sector
Also updates '_NW1', '_Deb1', '_CD1', '_CD1c', '_CS1'
*/

v[1] = V( "_Pi1" );								// firm profit in period

if ( v[1] > 0 )									// profits?
	v[0] = v[1] * VS( COUNTRL2, "tr" );			// tax to government
else
	v[0] = 0;									// no tax on losses

CFUN( cash_flow, v[1], v[0] );					// manage the period cash flow

RESULT( v[0] )


EQUATION( "_p1" )
/*
Price of good of firm in capital-good sector
*/
RESULT( ( 1 + VS( PARENT, "mu1" ) ) * V( "_c1" ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_BC" )
/*
Number of buying clients for firm in capital-good sector
*/
RESULT( COUNT_CND( "Cli", "__tOrd", "==", T ) )


EQUATION( "_D1l" )
/*
Local demand orders (in product units) received by firm in capital-good sector
*/

VS( CONSECL2, "Id" );							// ensure all orders are sent

v[0] = 0;
CYCLE( cur, "Cli" )
	if ( GRANDPARENTS( PARENTS( SHOOKS( cur ) ) ) == COUNTRL2 )// local client?
		if ( VS( cur, "__tOrd" ) == T )
			v[0] += VS( cur, "__nOrd" );		// received orders this period

RESULT( v[0] )


EQUATION( "_D1x" )
/*
Export demand orders (in product units) received by firm in capital-good sector
*/

CYCLES( WORLDL2, cur, "Country" )				// in all foreign countries
	if ( cur != COUNTRL2 )
		SUMS( cur, "Id" );						// ensure country orders sent

v[0] = 0;
CYCLE( cur, "Cli" )
	if ( GRANDPARENTS( PARENTS( SHOOKS( cur ) ) ) != COUNTRL2 )// foreign client?
		if ( VS( cur, "__tOrd" ) == T )
			v[0] += VS( cur, "__nOrd" );		// received orders this period

RESULT( v[0] )


EQUATION( "_HC" )
/*
Number of historical client firms from consumer-good sector
Removes old, non-buying clients
Also updates '_HCw'
*/

i = j = 0;										// client counter
CYCLE_SAFE( cur, "Cli" )						// remove old clients
{
	if ( VS( cur, "__tSel" ) < T - 1 )			// last selection is old?
	{
		DELETE( SHOOKS( cur ) );				// remove supplier brochure entry
		DELETE( cur );							// remove client entry
	}
	else
	{
		++i;

		if ( ID_CNT( VS( cur, "__IDc" ) ) != ID_CNT( V( "_ID1" ) ) )
			++j;								// foreign clients
	}
}

WRITE( "_HCw", j );

RESULT( i )


EQUATION( "_L1d" )
/*
Labor demand of firm in capital-good sector
Includes R&D labor
*/
RESULT( V( "_L1dRD" ) + ceil( V( "_Q1" ) / ( V( "_Btau" ) * VS( PARENT, "m1" ) ) ) )


EQUATION( "_L1dRD" )
/*
R&D labor demand of firm in capital-good sector
*/
RESULT( ceil( V( "_RD" ) / VLS( PARENT, "w1avg", 1 ) ) )


EQUATION( "_Pi1" )
/*
Profit (before taxes) of firm in capital-good sector
*/
RESULT( V( "_S1l" ) + V( "_S1x" ) + V( "_iD1" ) - V( "_W1" ) - V( "_i1" ) )


EQUATION( "_Q1e" )
/*
Effective output of firm in capital-good sector
*/

VS( PARENT, "L1rd" );							// ensure labor is allocated

v[0] = V( "_Q1" );								// planned production
v[1] = V( "_L1" );								// effective labor available
v[2] = V( "_L1d" );								// desired total workers

if ( v[1] >= v[2] )
	END_EQUATION( v[0] );						// produce as planned

v[3] = V( "_L1rd" );							// effective R&D workers
v[4] = V( "_L1dRD" );							// desired R&D workers

// adjustment factor
v[5] = v[2] > v[4] ? 1 - ( v[1] - v[3] ) / ( v[2] - v[4] ) : 1;

// adjust all pending orders, supplying at least one machine
CYCLE( cur, "Cli" )
	if ( VS( cur, "__tOrd" ) == T )				// order in this period?
	{
		v[6] = VS( cur, "__nOrd" ) - VS( cur, "__nCan" );// existing net orders
		v[0] -= v[7] = min( floor( v[6] * v[5] ), v[6] - 1 );
		INCRS( cur, "__nCan", v[7] );
	}

RESULT( max( v[0], 0 ) )						// avoid negative in part. cases


EQUATION( "_S1l" )
/*
Local sales of firm in capital-good sector
*/

V( "_Q1e" );									// ensure machines are delivered

v[0] = 0;
CYCLE( cur, "Cli" )
	if ( GRANDPARENTS( PARENTS( SHOOKS( cur ) ) ) == COUNTRL2 )// local client?
	{
		VS( PARENTS( SHOOKS( cur ) ), "_K" );	// ensure machines are accounted

		if ( VS( cur, "__tOrd" ) == T )
			v[0] += VS( cur, "__nOrd" ) - VS( cur, "__nCan" );// deliv'd orders
	}

RESULT( V( "_p1" ) * v[0] )


EQUATION( "_S1x" )
/*
Export sales of firm in capital-good sector
*/

V( "_Q1e" );									// ensure machines are delivered

v[1] = V( "_p1" );								// machine local price
v[2] = VS( PARENT, "trX1" );					// export duty rate

v[0] = v[3] = v[4] = v[5] = 0;					// accumulators
CYCLE( cur, "Cli" )
	if ( GRANDPARENTS( PARENTS( SHOOKS( cur ) ) ) != COUNTRL2 )// foreign client?
	{
		VS( PARENTS( SHOOKS( cur ) ), "_K" );	// ensure machines are accounted

		if ( VS( cur, "__tOrd" ) == T )
		{
			v[6] = VS( GRANDPARENTS( PARENTS( SHOOKS( cur ) ) ), "trMK" );
												// foreign import duty rate
			v[0] += v[7] = ( VS( cur, "__nOrd" ) - VS( cur, "__nCan" ) ) * v[1];
												// domestic value
			v[8] = v[7] / ( 1 - v[2] );			// value with local export duty
			v[3] += v[8] - v[7];				// local export duty
			v[5] += v[9] = v[8] / ( 1 - v[6] );	// value with all duties
			v[4] += v[9] - v[8];				// foreign import duty
		}
	}

WRITE( "_TaxX1", v[3] );
WRITE( "_TaxM1f", v[4] );
WRITE( "_X1", v[5] );

RESULT( v[0] )


EQUATION( "_W1" )
/*
Total wages paid by firm in capital-good sector
*/
VS( PARENT, "L1rd" );							// ensure labor is allocated
RESULT( V( "_L1" ) * VS( PARENT, "w1avg" ) )


EQUATION( "_c1" )
/*
Unit cost of production (1 machine) of firm in capital-good sector.
Use sectoral average (pool of sector 1 workers).
*/

V( "_Atau" );									// ensure innovation process ok

v[1] = VLS( PARENT, "ed1avg", 1 );				// average education in sector 1

if ( v[1] == 0 || VS( COUNTRL2, "flagEduc" ) == 0 )
	v[2] = 1;
else
{
	v[3] = VS( LABSUPL2, "alphaEd" );
	v[2] = pow( v[1] / ( 16 * v[3] / ( v[3] + VS( LABSUPL2, "betaEd" ) ) ),
				VS( LABSUPL2, "tauEd" ) );
}

RESULT( VLS( PARENT, "w1avg", 1 ) / ( v[2] * V( "_Btau" ) * VS( PARENT, "m1" ) ) )


EQUATION( "_f1" )
/*
Market share of firm in capital-good sector
Fair market share if sector didn't produce
*/
v[1] = VS( PARENT, "Q1e" );
RESULT( v[1] > 0 ? V( "_Q1e" ) / v[1] : 1 / VS( PARENT, "F1" ) )


EQUATION( "_i1" )
/*
Interest paid by firm in capital-good sector
*/
RESULT( VL( "_Deb1", 1 ) * VLS( FINSECL2, "rDeb", 1 ) *
		( 1 + ( VL( "_qc1", 1 ) - 1 ) * VS( FINSECL2, "kConst" ) ) )


EQUATION( "_iD1" )
/*
Interest received from deposits by firm in capital-good sector
*/
RESULT( max( VL( "_NW1", 1 ) * VLS( FINSECL2, "rD", 1 ), 0 ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "_CS1a" )
/*
Bank credit supply available (new debt) to firm in capital-good sector
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

EQUATION_DUMMY( "_Btau", "" )
/*
Labor productivity of new vintage of machine when employed for production
Updated in '_Atau'
*/

EQUATION_DUMMY( "_CD1", "" )
/*
Credit demand for firm in capital-good sector
Updated in '_Deb1max', '_Q1', '_Tax1'
*/

EQUATION_DUMMY( "_CD1c", "" )
/*
Credit demand constraint for firm in capital-good sector
Updated in '_Deb1max', '_Q1', '_Tax1'
*/

EQUATION_DUMMY( "_CS1", "" )
/*
Credit supplied to firm in capital-good sector
Updated in '_Deb1max', '_Q1', '_Tax1'
*/

EQUATION_DUMMY( "_Deb1", "" )
/*
Stock of bank debt of firm in capital-good sector
Updated in '_Q1', '_Tax1'
*/

EQUATION_DUMMY( "_HCw", "_HC" )
/*
Number of historical foreign client firms from consumer-good sector
Updated in '_HC'
*/

EQUATION_DUMMY( "_L1", "" )
/*
Labor employed by firm in capital-good sector
Includes R&D labor
Updated in 'L1rd'
*/

EQUATION_DUMMY( "_L1rd", "" )
/*
R&D labor employed by firm in capital-good sector
Updated in 'L1rd'
*/

EQUATION_DUMMY( "_NW1", "" )
/*
Net worth of firm in capital-good sector
Updated in '_Q1', '_Tax1'
*/

EQUATION_DUMMY( "_NW1p", "_Q1" )
/*
Provision for production of firm in capital-good sector
Updated in '_Q1'
*/

EQUATION_DUMMY( "_TaxM1f", "_S1x" )
/*
Foreign import duties (in domestic currency) paid by firm in capital-good sector
Updated in '_S1x'
*/

EQUATION_DUMMY( "_TaxX1", "_S1x" )
/*
Local export duties (in domestic currency) paid by firm in capital-good sector
Updated in '_S1x'
*/

EQUATION_DUMMY( "_X1", "_S1x" )
/*
Exports (in domestic currency including duties) of firm in capital-good sector
Updated in '_S1x'
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

EQUATION_DUMMY( "_qc1", "" )
/*
Credit class of firm in sector 1 (1,2,3,4)
Updated in 'cScores'
*/
