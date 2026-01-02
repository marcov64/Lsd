/******************************************************************************

	COUNTRY OBJECT EQUATIONS
	------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the Country objects in the K+S LSD model
	are coded below. Also the general country-level initialization is
	defined below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "Ec" )
/*
Competitiveness of country in exporting consumption goods
*/

v[1] = log( VLS( WORLDL0, "pCmaxW", 1 ) + 1 );	// market parameters
v[2] = log( VLS( WORLDL0, "pCminW", 1 ) + 1 );
v[3] = log( VLS( WORLDL0, "qCmaxW", 1 ) + 1 );
v[4] = log( VLS( WORLDL0, "qCminW", 1 ) + 1 );
v[5] = VS( WORLDL0, "delta1" );					// competitiveness weights
v[6] = VS( WORLDL0, "delta2" );

// compute country weighted averages in int'l currency
v[7] = SUML( "Q2e", 1 );					// effective goods productions
v[8] = v[7] > 0 ? log( V( "e" ) * ( 1 + AVE( "trX2" ) ) *
					   WHTAVEL( "p2avg", "Q2e", 1 ) / v[7] + 1 ) : v[1];
v[9] = v[7] > 0 ? log( WHTAVEL( "q2avg", "Q2e", 1 ) / v[7] + 1 ) : v[4];

// normalize price and quality to [0.1, 0.9]
// zero competitiveness is avoided to prevent weirdness in replicator
v[10] = v[1] > v[2] ? min( max( ( v[8] - v[2] ) / ( v[1] - v[2] ), 0 ), 1 ) : 0.5;
v[11] = v[3] > v[4] ? min( max( ( v[9] - v[4] ) / ( v[3] - v[4] ), 0 ), 1 ) : 0.5;
v[12] = 0.1 + 0.8 * v[10];
v[13] = 0.1 + 0.8 * v[11];

RESULT( ( v[5] * ( 1 - v[12] ) + v[6] * v[13] ) / ( v[5] + v[6] ) )


EQUATION( "Gcd" )
/*
Government desired public consumption (in money terms) expenditure
*/

i = V( "flagGovExp" );							// government expenditure mode

if ( i == 0 )									// no government expending?
	END_EQUATION( 0 );

v[1] = VLS( FINSECL0, "DepoG", 1 );				// accumulated surplus

v[0] = V( "g0" ) * V( "Ymt" ) + V( "sG" );		// desired consumption

// adjustments to desired consumption
if ( v[1] >= 0 )								// accumulated past surplus?
{
	if ( i == 3 )								// spend accumulated surplus
		v[0] += min( v[1], max( 0, - VL( "Def", 1 ) ) );// limit to cur. deficit

	if ( i == 4 )								// try to cover GDP gap + surplus
	{
		v[0] += v[2] = min( v[1], max( 0, VL( "Ymt", 1 ) - VL( "Y", 1 ) ) );
		v[0] += min( v[1] - v[2], max( 0, - VL( "Def", 1 ) ) );
	}
}
else											// there is public debt
{
	j = V( "flagFiscalRule" );					// fiscal rule to apply

	// apply fiscal rule if soft balanced budget rule not binding, after warm-up
	if ( ( j == 1 || j == 3 || ( j > 0 && VL( "dY", 1 ) > 0 ) ) &&
		 T >= VS( FINSECL0, "Trule" ) )
	{
		v[10] = VS( FINSECL0, "DebRule" );		// primary deficit rule limit
		v[11] = VS( FINSECL0, "DefPrule" );		// primary deficit rule limit
		v[12] = VL( "Tax", 1 );					// tax income
		v[13] = VL( "Deb", 1 );					// public debt
		v[14] = VL( "Y", 1 );					// current GDP in nominal terms

		if ( j > 2 && v[13] / v[14] > v[10] )	// debt rule applies?
		{
			// desired surplus
			v[15] = - VS( FINSECL0, "deltaDeb" ) * ( v[13] / v[14] - v[10] ) * v[13];

			if ( v[0] - v[12] > v[15] )			// expected surplus not enough?
				v[0] = v[12] + v[15];			// adjust to the desired surplus
		}
		else
			if ( v[0] - v[12] > v[11] * v[14] )	// deficit rule applies?
				v[0] = v[12] + v[11] * v[14];	// apply limit
	}
}

RESULT( max( v[0], 0 ) )


EQUATION( "Ymt" )
/*
Medium-term Gross domestic product trend (nominal/currency terms)
*/

k = V( "Tmt" );									// medium-term time horizon

// compute log-linear trend
for ( v[1] = v[2] = v[3] = 0, h = 1; h <= k; ++h )
{
	v[1] += v[4] = log( VL( "Y", k - h + 1 ) );	// accumulate nominal GDP
	v[2] += h * v[4];
	v[3] += pow( h, 2 );
}

v[5] = v[1] / k;								// average log GDP
v[6] = ( k * ( k + 1 ) * v[5] - v[2] ) / ( k * pow( k + 1, 2 ) - v[3] );// b
v[7] = v[5] - v[6] * ( k + 1 );					// a

RESULT( exp( v[7] + v[6] * ( k + 1 ) ) )


EQUATION( "e" )
/*
Exchange rate (in domestic currency) to the international currency
*/
RESULT( CURRENT * ( 1 - V( "gammaE" ) * ( VL( "X", 1 ) - VL( "M", 1 ) ) /
										( CURRENT * VLS( WORLDL0, "Yw", 1 ) ) ) )


EQUATION( "fMC" )
/*
Desired market share of consumption-good imports
*/
RESULT( min( max( CURRENT * ( 1 + V( "chiM" ) * ( VS( WORLDL0, "EcAvgW" ) /
								  ( 1 + V( "trMC" ) ) / V( "Ec" ) - 1 ) ),
				  VS( WORLDL0, "f0" ) ),
			 V( "fMax" ) ) )


EQUATION( "fXC" )
/*
Desired market share of worldwide consumption-good exports
*/
RESULT( max( CURRENT * ( 1 + VS( WORLDL0, "chiX" ) *
							 ( V( "Ec" ) / VS( WORLDL0, "EcAvgW" ) - 1 ) ),
			 VS( WORLDL0, "f0" ) ) )


EQUATION( "sG" )
/*
Fiscal policy shock (in money terms) on public consumption
*/

h = V( "flagFiscalShock" );						// type(s) of fiscal shock
j = V( "TregChg");								// minimum period for shock
k = V( "Tsh");									// shock duration

v[0] = CURRENT;									// default is keep current state

if ( CURRENT == 0 )								// shock not running?
{
	if ( h & 0x1 && T >= j && V( "sGt" ) == 0 )	// single shock pending?
	{
		v[1] = 1 / ( LAST_T - k - j + 1 );		// shock probability

		if ( v[1] > RND )						// draw if shock happens now
		{
			v[0] = V( "ySh" ) * VL( "Y", 1 );	// shock size
			WRITE( "sGt", T );					// save shock time
		}
	}
}
else
	if ( h & 0x1 && V( "sGt" ) < T - k + 1 )	// single shock to clear?
		v[0] = 0;								// clear shock

RESULT( v[0] )


EQUATION( "sIn" )
/*
Fiscal policy shock (in percentage points) on income tax rate
*/

h = V( "flagFiscalShock" );						// type(s) of fiscal shock
j = V( "TregChg");								// minimum period for shock
k = V( "Tsh");									// shock duration

v[0] = CURRENT;									// default is keep current state

if ( CURRENT == 0 )								// shock not running?
{
	if ( h & 0x2 && T >= j && V( "sInT" ) == 0 )// single shock pending?
	{
		v[1] = 1 / ( LAST_T - k - j + 1 );		// shock probability

		if ( v[1] > RND )						// draw if shock happens now
		{
			v[0] = V( "trIn" ) * V( "ySh" ) * VL( "Y", 1 ) /
				   VLS( LABSUPL0, "TaxW", 1 );
												// shock size
			WRITE( "sInT", T );					// save shock time
		}
	}
}
else
	if ( h & 0x2 && V( "sInT" ) < T - k + 1 )	// single shock to clear?
		v[0] = 0;								// clear shock

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A" )
/*
Overall labor productivity
*/
v[1] = VS( LABSUPL0, "L" );
RESULT( v[1] > 0 ? V( "Yreal" ) / v[1] : CURRENT )


EQUATION( "C" )
/*
Nominal (monetary terms) aggregated consumption
*/
VS( CONSECL0, "D2" );							// ensure shortages allocated
RESULT( VS( LABSUPL0, "Cw" ) + V( "Gc" ) )


EQUATION( "Cd" )
/*
Nominal (monetary terms) desired aggregated consumption
*/
RESULT( VS( LABSUPL0, "CdW" ) + V( "Gcd" ) )


EQUATION( "Deb" )
/*
Accumulated (gross) government debt
*/
RESULT( VS( FINSECL0, "BondsB" ) + VS( FINSECL0, "BondsCB" ) )


EQUATION( "DebY" )
/*
Government debt on GDP ratio
*/
RESULT( V( "Deb" ) / V( "Y" ) )


EQUATION( "Def" )
/*
Government total deficit (negative if surplus)
*/
RESULT( V( "DefP" ) + VL( "EqEntryG", 1 ) - VL( "NWexitG", 1 ) -
		VS( FINSECL0, "PiCB" ) + VLS( FINSECL0, "Gbail", 1 ) +
		VLS( FINSECL0, "rBonds", 1 ) * ( VLS( FINSECL0, "BondsB", 1 ) +
									 VLS( FINSECL0, "BondsCB", 1 ) ) -
		VLS( FINSECL0, "rRes", 1 ) * VLS( FINSECL0, "DepoG", 1 ) )


EQUATION( "DefP" )
/*
Government primary deficit (negative if surplus)
*/
RESULT( V( "G" ) - V( "Tax" ) - VL( "DivG", 1 ) )


EQUATION( "DefPy" )
/*
Government primary deficit on GDP ratio
*/
RESULT( V( "DefP" ) / V( "Y" ) )


EQUATION( "DivG" )
/*
Total dividends received by government
*/
RESULT( VS( CAPSECL0, "Div1g" ) + VS( CONSECL0, "Div2g" ) )


EQUATION( "EqG" )
/*
Equity from firms hold by government
This variable must be explicitly recalculated after entry/exit
*/
RESULT( VL( "EqG", 1 ) + V( "EqEntryG" ) -
		VS( CAPSECL0, "Eq1exitG" ) - VS( CONSECL0, "Eq2exitG" ) )


EQUATION( "EqEntryG" )
/*
Cost of new equity from firm rescues to government
This variable must be explicitly recalculated after entry/exit
*/
RESULT( VS( CAPSECL0, "Eq1entryG" ) + VS( CONSECL0, "Eq2entryG" ) )


EQUATION( "G" )
/*
Government expenditure (exogenous demand)
*/
VS( CONSECL0, "D2" );							// ensure shortages allocated
RESULT( VS( LABSUPL0, "Gtrf" ) + V( "Gc" ) )


EQUATION( "M" )
/*
Total imports (in domestic currency) including domestic and foreign duties
*/
VS( CONSECL0, "D2" );							// ensure shortages allocated
RESULT( V( "Mk" ) + V( "Mc" ) )


EQUATION( "Mc" )
/*
Imports (in domestic currency) of consumption goods
*/
RESULT( V( "McD" ) )


EQUATION( "Mk" )
/*
Imports (in domestic currency) of capital goods
*/
RESULT( SUM( "M2" ) )


EQUATION( "NWexitG" )
/*
Residual net worth from equity hold on exiting government-owned firms
This variable must be explicitly recalculated after entry/exit
*/
RESULT( VS( CAPSECL0, "NW1exitG" ) + VS( CONSECL0, "NW2exitG" ) )


EQUATION( "Tax" )
/*
Government tax income
*/
RESULT( VS( CAPSECL0, "Tax1" ) + VS( CONSECL0, "Tax2" ) + VS( FINSECL0, "TaxB" ) +
		VS( LABSUPL0, "TaxW" ) + V( "TaxM" ) + V( "TaxX" ) )


EQUATION( "TaxM" )
/*
Duties collected on imports by local government
*/
RESULT( SUM( "TaxM2" ) +
		V( "trMC" ) * V( "Mc" ) * ( 1 - VS( WORLDL0, "trXCavgW" ) ) )


EQUATION( "TaxMf" )
/*
Duties collected on imports by foreign governments
*/
RESULT( SUM( "TaxM1f" ) + SUM( "TaxM2f" ) )


EQUATION( "TaxX" )
/*
Duties collected on exports
*/
RESULT( SUM( "TaxX1" ) + SUM( "TaxX2" ) )


EQUATION( "X" )
/*
Total exports (in domestic currency including duties)
*/
RESULT( V( "Xk" ) + V( "Xc" ) )


EQUATION( "Xc" )
/*
Exports (in domestic currency) of consumption goods
*/
RESULT( SUM( "X2" ) )


EQUATION( "Xk" )
/*
Exports (in domestic currency) of capital goods
*/
RESULT( SUM( "X1" ) )


EQUATION( "Y" )
/*
Gross domestic product (nominal/currency terms)
*/
RESULT( max( V( "C" ) + VS( CONSECL0, "Inom" ) + VS( CONSECL0, "dNnom" ) +
			 V( "X" ) - V( "M" ), 1 ) )


EQUATION( "Yreal" )
/*
Real (in initial prices terms) gross domestic product
*/
RESULT( max( VS( CAPSECL0, "Q1e" ) * VS( CAPSECL0, "pK0" ) +
			 VS( CONSECL0, "Q2e" ) * VS( CONSECL0, "pC0" ), 1 ) )


EQUATION( "dAb" )
/*
Notional overall productivity (bounded) rate of change
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "A", V( "mLim" ), V( "mPer" ) ) )


EQUATION( "dY" )
/*
Gross domestic product (log) growth rate
*/
RESULT( T > 1 ? log( V( "Yreal" ) ) - log( VL( "Yreal", 1 ) ) : 0 )


EQUATION( "entryExit" )
/*
Perform the entry and exit process in all sectors
*/

// ensure aggregates depending on firm objects are computed in all countries
CYCLES( WORLDL0, cur, "Country" )
{
	UPDATES( cur );								// country variables
	UPDATES( V_EXTS( cur, countryE, finSec ) );	// financial sector variables
	UPDATES( V_EXTS( cur, countryE, labSup ) );	// labor-supply variables
	UPDATES( V_EXTS( cur, countryE, macSta ) );	// statistics-only variables
	UPDATES( V_EXTS( cur, countryE, secSta ) );
	UPDATES( V_EXTS( cur, countryE, labSta ) );
}

v[0] = VS( CAPSECL0, "entry1exit" ) + VS( CONSECL0, "entry2exit" );

RECALCS( FINSECL0, "BadDeb" );					// bad debt update after exits
RECALC( "EqEntryG" );
RECALC( "NWexitG" );
RECALC( "EqG" );

VS( LABSUPL0, "EqAlloc" );						// reallocate equity to workers
VS( FINSECL0, "cScores" );						// set the credit pecking order

RESULT( v[0] )


EQUATION( "regChg" )
/*
Produces a labor market regime change at the time step defined in TregChg
If TregChg is zero, there is no regime change
Changed parameters:
	Global:
		flagFiscalShock		-> flagFiscalShockChg
		flagFireOrder1		-> flagFireOrder1Chg
		flagHireOrder1		-> flagHireOrder1Chg
		flagHireSeq			-> flagHireSeqChg
		flagIndexMinWage	-> flagIndexMinWageChg
		flagSearchMode		-> flagSearchModeChg
		flagTradeC			-> flagTradeCchg
		flagTradeK			-> flagTradeKchg
		tr		-> trChg
		trIn	-> trInChg
		Lambda	-> LambdaChg
		muRes	-> muResChg
		rT		-> rTchg
		tauB	-> tauBchg
		Ts		-> TsChg
		delta	-> deltaChg (only if delta > -1)
		omega	-> omegaPreChg/omegaPosChg
		phi		-> phiChg
		e0		-> e0Chg
		mu20	-> mu20Chg
	Firm-specific:
		flagFirmOwner		-> flagFirmOwnerChg
		flagFireOrder2		-> flagFireOrder2Chg
		flagFireRule		-> flagFireRuleChg
		flagHireOrder2		-> flagHireOrder2Chg
		flagIndexWage		-> flagIndexWageChg
		flagWageOffer		-> flagWageOfferChg
		b					-> bChg
*/

if ( T == ( int ) V( "TregChg" ) )				// in time, replace parameters
{
	WRITE( "flagFiscalShock", V( "flagFiscalShockChg" ) );
	WRITE( "flagFireOrder1", V( "flagFireOrder1Chg" ) );
	WRITE( "flagHireOrder1", V( "flagHireOrder1Chg" ) );
	WRITE( "flagHireSeq", V( "flagHireSeqChg" ) );
	WRITE( "flagIndexMinWage", V( "flagIndexMinWageChg" ) );
	WRITE( "flagSearchMode", V( "flagSearchModeChg" ) );
	WRITE( "flagTradeC", V( "flagTradeCchg" ) );
	WRITE( "flagTradeK", V( "flagTradeKchg" ) );
	WRITE( "tr", V( "trChg" ) );
	WRITE( "trIn", V( "trInChg" ) );
	WRITES( FINSECL0, "Lambda", VS( FINSECL0, "LambdaChg" ) );
	WRITES( FINSECL0, "muRes", VS( FINSECL0, "muResChg" ) );
	WRITES( FINSECL0, "rT", VS( FINSECL0, "rTchg" ) );
	WRITES( FINSECL0, "tauB", VS( FINSECL0, "tauBchg" ) );
	WRITES( LABSUPL0, "Ts", VS( LABSUPL0, "TsChg" ) );
	WRITES( LABSUPL0, "omega", VS( LABSUPL0, "omegaPosChg" ) );
	WRITES( LABSUPL0, "phi", VS( LABSUPL0, "phiChg" ) );
	WRITES( CONSECL0, "e0", VS( CONSECL0, "e0Chg" ) );
	WRITES( CONSECL0, "mu20", VS( CONSECL0, "mu20Chg" ) );

	if ( VS( LABSUPL0, "delta" ) > -1 )
		WRITES( LABSUPL0, "delta", VS( LABSUPL0, "deltaChg" ) );

	if ( ( int ) V( "flagAllFirmsChg" ) == 1 )
	{
		WRITE( "flagFirmOwner", V( "flagFirmOwnerChg" ) );
		WRITE( "flagFireOrder2", V( "flagFireOrder2Chg" ) );
		WRITE( "flagFireRule", V( "flagFireRuleChg" ) );
		WRITE( "flagHireOrder2", V( "flagHireOrder2Chg" ) );
		WRITE( "flagIndexWage", V( "flagIndexWageChg" ) );
		WRITE( "flagWageOffer", V( "flagWageOfferChg" ) );
		WRITES( CONSECL0, "b", VS( CONSECL0, "bChg" ) );

		CYCLES( CONSECL0, cur, "Firm2" )
			WRITES( cur, "_postChg", 1 );		// set all firms as post-change
	}

	LOG( "\n Regime changed (t=%g) at country %g", T, V( "IDcnt" ) );
	PARAMETER;									// no more evaluate this eq.
	v[0] = 1;
}
else
	v[0] = 0;									// no change

RESULT( v[0] )


/*========================= INITIALIZATION EQUATION ==========================*/

EQUATION( "initCountry" )
/*
Initialize the K+S country object. It is run only once per country.
*/

PARAMETER;										// execute only once per country

WRITE( "IDcnt", INCRS( WORLDL0, "last_IDcnt", 1 ) );// new country ID

// create the new country as an extension to current 'Country' object
ADDEXT_INIT( countryE );

// country-level pointers to speed-up the access to individual containers
WRITE_EXT( countryE, capSec, SEARCH( "Capital" ) );
WRITE_EXT( countryE, conSec, SEARCH( "Consumption" ) );
WRITE_EXT( countryE, finSec, SEARCH( "Financial" ) );
WRITE_EXT( countryE, labSup, SEARCH( "Labor" ) );
WRITE_EXT( countryE, macSta, SEARCH( "Mac" ) );
WRITE_EXT( countryE, secSta, SEARCH( "Sec" ) );
WRITE_EXT( countryE, labSta, SEARCH( "Lab" ) );

// pointer shortcuts the access to individual market containers
cur1 = CAPSECL0;
cur2 = CONSECL0;
cur3 = FINSECL0;
cur4 = LABSUPL0;

V( "regChg" );									// enforce change even at t=1

// check unwanted extra instances (only one of each is required)
if ( COUNT( "Capital" ) > 1 || COUNT( "Consumption" ) > 1 ||
	 COUNT( "Financial" ) > 1 || COUNT( "Labor" ) > 1 ||
	 COUNT( "Stats" ) > 1 || COUNTS( cur1, "Firm1" ) > 1 ||
	 COUNTS( cur1, "Wrk1" ) > 1 || COUNTS( cur2, "Firm2" ) > 1 ||
	 COUNTS( cur3, "Bank" ) > 1 || COUNTS( cur4, "Worker" ) > 1 ||
	 COUNTS( SEARCHS( cur1, "Firm1" ), "Cli" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Broch" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Vint" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Wrk2" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Vint" ), "WrkV" ) > 1 ||
	 COUNTS( SEARCHS( cur3, "Bank" ), "Cli1" ) > 1 ||
	 COUNTS( SEARCHS( cur3, "Bank" ), "Cli2" ) > 1 )
{
	PLOG( "\n Error: multiple-instance objects not allowed, aborting!" );
	ABORT;
}

// ensure initial number of firms, banks and workers is consistent
if ( VS( cur1, "F1min" ) < 1 || VS( cur2, "F2min" ) < 1 || VS( cur3, "B" ) < 1 ||
	 VS( cur1, "F1max" ) < VS( cur1, "F1min" ) ||
	 VS( cur2, "F2max" ) < VS( cur2, "F2min" ) ||
	 VS( cur4, "Lscale" ) < 1 || VS( cur4, "Lscale" ) > VS( cur4, "Ls0" ) )
{
	PLOG( "\n Error: invalid number of agents, aborting!" );
	ABORT;
}

WRITES( cur1, "F10", min( max( VS( cur1, "F10" ), VS( cur1, "F1min" ) ),
						  VS( cur1, "F1max" ) ) );
WRITES( cur2, "F20", min( max( VS( cur2, "F20" ), VS( cur2, "F2min" ) ),
						  VS( cur2, "F2max" ) ) );

if ( VS( cur4, "Ls0" ) / VS( cur4, "Lscale" ) <
	 10 * ( VS( cur1, "F10" ) + VS( cur2, "F20" ) ) )
	PLOG( "\n Warning: small number of workers" );

// adjust parameters required to be integer and positive
WRITE( "TregChg", max( 0, ceil( V( "TregChg" ) ) ) );
WRITES( cur2, "m2", max( 1, ceil( VS( cur2, "m2" ) ) ) );
WRITES( cur4, "Tc", max( 1, ceil( VS( cur4, "Tc" ) ) ) );
WRITES( cur4, "Tr", max( 0, ceil( VS( cur4, "Tr" ) ) ) );

// prepare data required to set initial conditions
double EqB0 = VS( cur3, "EqB0" );				// initial bank equity multiple
double Lscale = VS( cur4, "Lscale" );			// labor scaling factor
double NW10 = VS( cur1, "NW10" );				// initial net worth in sector 1
double NW20 = VS( cur2, "NW20" );				// initial net worth in sector 2
double alphaB = VS( cur3, "alphaB" );			// bank size distrib. parameter
double eta = VS( cur2, "eta" );					// technical machine life time
double f2trdChg = VS( cur2, "f2trdChg" );		// threshold cor post-change firms
double m1 = VS( cur1, "m1" );					// output factor in sector 1
double m2 = VS( cur2, "m2" );					// output factor in sector 2
double mu1 = VS( cur1, "mu1" );					// mark-up in sector 1
double mu20 = VS( cur2, "mu20" );				// initial mark-up in sector 2
double phi = VS( cur4, "phi" );					// unemployment benefit rate
double phi2 = VS( cur4, "phi2" );				// secondary educ. wage premium
double phi3 = VS( cur4, "phi3" );				// tertiary educ. wage premium
double rT = VS( cur3, "rT" );					// prime rate target
double tauB = VS( cur3, "tauB" );				// minimum capital adequacy rate
double theta2 = VS( cur4, "theta2" );			// secondary educ. labor share
double theta3 = VS( cur4, "theta3" );			// tertiary educ. labor share
double trIn = V( "trIn" );						// tax rate on worker income
double w0min = VS( cur4, "w0min" );				// absolute/initial minimum wage
int B = VS( cur3, "B" );						// number of banks
int F10 = VS( cur1, "F10" );					// initial firms in sector 1
int F20 = VS( cur2, "F20" );					// initial firms in sector 2
int F2max = VS( cur2, "F2max" );				// max firms in sector 2
int Ls0 = VS( cur4, "Ls0" );					// initial labor supply
int Tc = VS( cur4, "Tc" );						// work-contract term
int Texp = VS( cur4, "Texp" );					// expectation formation period
int Tmt = V( "Tmt" );							// medium-term GDP horizon
int Tr = VS( cur4, "Tr" );						// work-life duration
int Ts = VS( cur4, "Ts" );						// wage memory
int flagWorkerLBU = V( "flagWorkerLBU" );		// worker learning-by-use mode
int flagEduc = V( "flagEduc" );					// education mode

double w2o2avg = flagEduc ? ( 1 + phi2 ) * INIWAGE : 0;// initial offered wages
double w2o3avg = flagEduc ? ( 1 + phi3 ) * w2o2avg : 0;
double w0 = flagEduc == 0 ? INIWAGE : ( 1 - theta2 - theta3 ) * INIWAGE +
			theta2 * w2o2avg + theta3 * w2o3avg;// initial average wage
double Btau0 = ( 1 + mu1 ) * INIPROD / ( m1 * m2 * VS( cur2, "b" ) );
												// initial productivity in sec. 1
double c10 = w0 / ( Btau0 * m1 );				// initial cost in sector 1
double c20 = w0 / INIPROD;						// initial cost in sector 2
double p10 = ( 1 + mu1 ) * c10;					// initial price sector 1
double p20 = ( 1 + mu20 ) * c20;				// initial price sector 2
double K0 = Ls0 * w0 / p20;						// full employment K required
double D10 = K0 / ( m2 * eta );					// initial demand for sector 1
double RD0 = VS( cur1, "nu" ) * D10 * p10;		// initial R&D expense
double D20 = ( ( D10 * c10 + RD0 ) * ( 1 - phi - trIn ) + Ls0 * w0 * phi ) /
			 ( mu20 + phi + trIn ) * c20;		// initial demand for sector 2
double Ld10 = RD0 / w0 + D10 / ( Btau0 * m1 );	// initial labor demand sec. 1
double Ld20 = D20 / INIPROD;					// initial labor demand sector 2
double Y0 = D10 * p10 + D20 * p20;				// initial GDP
double A0 = Y0 / ( Ld10 + Ld20 );				// initial productivity
double CdReal0 = ( VS( cur4, "alphaC" ) / ( 1 - VS( cur4, "betaC" ) ) ) *
				 w0 * ( 1 - trIn ) / p20;		// initial desired real cons.
double E2avg0 = ( VS( cur2, "omega1" ) + VS( cur2, "omega2" ) +
				  VS( cur2, "omega3" ) ) / 3;	// initial competitiveness
double rBonds = rT * ( 1 - VS( cur3, "muBonds" ) );// initial interest on bonds
double rD = rT * ( 1 - VS( cur3, "muD" ) );		// initial interest on deposits
double rDeb = rT * ( 1 + VS( cur3, "muDeb" ) );	// initial interest on debt
double rRes = rT * ( 1 - VS( cur3, "muRes" ) );	// initial interest on reserves
double sV0 = ( flagWorkerLBU == 0 || flagWorkerLBU == 2 ) ?
			 INISKILL : VS( cur4, "sigma" );	// initial vintage skills
double wU = V( "flagGovExp" ) >= 2 ? phi * w0 : w0min;// initial unempl. benefit

// reserve space for country-level non-initialized vectors
EXEC_EXT( countryE, firm2ptr, reserve, F2max );	// sector 2 firm objects
EXEC_EXT( countryE, firm2wgtd, reserve, F2max );
EXEC_EXT( countryE, bankPtr, reserve, B );		// bank objects
EXEC_EXT( countryE, bankWgtd, reserve, B );

// reset serial ID counters for dynamic objects
WRITES( cur1, "lastID1", 0 );
WRITES( cur2, "lastID2", 0 );

// initialize lagged variables depending on parameters
WRITEL( "A", A0, -1 );
WRITEL( "e", INIEXCH, -1 );
WRITES( cur1, "pK0", p10 );
WRITES( cur2, "pC0", p20 );
WRITELS( cur1, "A1", Btau0, -1 );
WRITELS( cur1, "AtauAvg", INIPROD, -1 );
WRITELS( cur1, "F1", F10, -1 );
WRITELS( cur1, "PPI", p10, -1 );
WRITELS( cur1, "p1avg", p10, -1 );
WRITELS( cur1, "sT1min", INISKILL, -1 );
WRITELS( cur1, "w1avg", w0, -1 );
WRITELS( cur2, "E2avg", E2avg0, -1 );
WRITELS( cur2, "F2", F20, -1 );
WRITELS( cur2, "c2", c20, -1 );
WRITELS( cur2, "oldVint", 1, -1 );
WRITELS( cur2, "w2avg", w0, -1 );
WRITELS( cur2, "w2o1avg", INIWAGE, -1 );
WRITELS( cur2, "w2o2avg", w2o2avg, -1 );
WRITELS( cur2, "w2o3avg", w2o3avg, -1 );
WRITELS( cur3, "r", rT, -1 );
WRITELS( cur3, "rBonds", rBonds, -1 );
WRITELS( cur3, "rD", rD, -1 );
WRITELS( cur3, "rDeb", rDeb, -1 );
WRITELS( cur3, "rRes", rRes, -1 );
WRITELS( cur4, "Ls", Ls0, -1 );
WRITELS( cur4, "sAvg", sV0, -1 );
WRITELS( cur4, "sTavg", INISKILL, -1 );
WRITELS( cur4, "sTmin", INISKILL, -1 );
WRITELS( cur4, "wAvg", w0, -1 );
WRITELS( cur4, "wCent", w0, -1 );
WRITELS( cur4, "wMinPol", w0min, -1 );
WRITELS( cur4, "wU", wU, -1 );

for ( i = 1; i <= Tmt; ++i )
	WRITEL( "Y", Y0, - i );						// GDP medium-term memory

for ( i = 1; i <= Texp; ++i )
	WRITELS( cur2, "CPI", p20, - i );			// expected CPI memory

// create banks' objects and set initial values
k = 1;											// initial bank ID
i = 0;											// bank clients accumulator
ADDNOBJLS( cur3, "Bank", B - 1, 0 );			// new objects recalculate in t
CYCLES( cur3, cur, "Bank" )
{
	// draw the number of clients from a bounded Pareto distr.
	i += j = floor( bpareto( alphaB, 2, ( F10 + F20 ) / 2 ) );

	WRITES( cur, "_IDb", k );
	WRITES( cur, "_fD", j );

	DELETE( SEARCHS( cur, "Cli1" ) );			// remove empty client instances
	DELETE( SEARCHS( cur, "Cli2" ) );

	++k;
}

CYCLES( cur3, cur, "Bank" )
	WRITES( cur, "_fD", VS( cur, "_fD" ) / i );	// adjust desired market shares

VS( cur3, "banksMaps" );						// update the mapping vectors

// create firms' objects and set initial values
DELETE( SEARCHS( cur1, "Firm1" ) );				// remove empty instances
DELETE( SEARCHS( cur2, "Firm2" ) );
DELETE( SEARCHS( cur1, "Wrk1" ) );

v[1] = CFUNS( cur1, entry_firm1, F10, true );	// add capital-good firms
INIT_TSEARCHS( cur1, "Firm1" );					// prepare turbo search indexing

v[1] += CFUNS( cur2, entry_firm2, F20, true );	// add consumer-good firms
VS( cur2, "firm2maps" );						// update the mapping vectors

// set banks initial assets according to existing loans to firms
v[2] = v[3] = v[4] = v[5] = v[6] = 0;			// accumulators
CYCLES( cur3, cur, "Bank" )
{
	v[7] = v[8] = 0;							// loans and deposits accumulator

	CYCLES( cur, cur5, "Cli1" )
	{
		v[7] += VLS( SHOOKS( cur5 ), "_Deb1", 1 );// firm debt
		v[8] += VLS( SHOOKS( cur5 ), "_NW1", 1 );// firm net wealth
	}

	CYCLES( cur, cur6, "Cli2" )
	{
		v[7] += VLS( SHOOKS( cur6 ), "_Deb2", 1 );// firm debt
		v[8] += VLS( SHOOKS( cur6 ), "_NW2", 1 );// firm net wealth
	}

	v[2] += v[7];
	v[3] += v[8];

	if ( v[8] > 0 )								// bank has clients?
		v[9] = EqB0 * v[8];						// bank initial tier 1 capital
	else
		v[9] = EqB0 * ( F10 * NW10 + F20 * NW20 ) * VS( cur, "_fD" );// use proxy

	v[4] += v[10] = tauB * v[8];				// part on required reserves
	v[5] += v[11] = v[9] - v[10];				// shareholder equity
	v[6] += v[12] = v[7] + v[9] - v[8];			// and net worth

	WRITELS( cur, "_Loans",	 v[7], -1 );		// bank loans
	WRITELS( cur, "_Res", v[10], -1 );			// bank reserves at central bank
	WRITELS( cur, "_ExRes", v[11], -1 );		// bank shareholder equity
	WRITELS( cur, "_Depo",	v[8], -1 );			// bank deposits/reserves
	WRITELS( cur, "_NWb", v[12], -1 );			// bank net worth
}

WRITELS( cur3, "Loans", v[2], -1 );				// initial industry totals
WRITELS( cur3, "Depo", v[3], -1 );
WRITELS( cur3, "Res", v[4], -1 );
WRITELS( cur3, "ExRes", v[5], -1 );
WRITELS( cur3, "NWb", v[6], -1 );

// create workers' objects and set initial values
k = 1;											// initial worker ID
ADDNOBJLS( cur4, "Worker", ceil( Ls0 / Lscale ) - 1, 0 );// recalculate in t
CYCLES( cur4, cur, "Worker" )
{
	ADDHOOKS( cur, WORKERHK );					// add worker hooks
	CFUNS( cur, set_education );				// draw education

	WRITES( cur, "_ID", k );
	WRITES( cur, "_Tc", Tc );
	WRITES( cur, "_age", uniform_int( 1, Tr ) );
	WRITES( cur, "_emig", 0 );
	WRITES( cur, "_employed", 0 );
	WRITELS( cur, "_Eq", v[1] / Ls0, -1 );
	WRITELS( cur, "_sT", INISKILL, -1 );
	WRITELS( cur, "_sV", sV0, -1 );

	for ( i = 1; i <= max( Ts, Texp ); ++i )	// fill multi-lag variables
	{
		WRITELS( cur, "_CdReal", CdReal0, - i );
		WRITELS( cur, "_w", w0, - i );
	}
	++k;
}

WRITELS( cur4, "EqW", v[1], -1 );				// initial shareholder equity

RESULT( 1 )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "Gc", "" )
/*
Government desired public consumption expenditure
Updated in 'D2'
*/

EQUATION_DUMMY( "McD", "" )
/*
Desired imports (in domestic currency) of consumption goods
Updated in 'XcW'
*/

EQUATION_DUMMY( "XcD", "" )
/*
Exports (in domestic currency) of consumption goods
Updated in 'XcW'
*/

EQUATION_DUMMY( "sGt", "sG" )
/*
Time (last) public consumption shock started
Updated in 'sG'
*/

EQUATION_DUMMY( "sInT", "sIn" )
/*
Time (last) income tax rate shock started
Updated in 'sIn'
*/
