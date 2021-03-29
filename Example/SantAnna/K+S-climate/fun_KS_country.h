/******************************************************************************

	COUNTRY OBJECT EQUATIONS
	------------------------

	Equations that are specific to the Country objects in the K+S LSD model 
	are coded below. Also the general country-level initialization is 
	defined below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "C" )
/*
Nominal (monetary terms) aggregated consumption
*/

i = V( "flagTax" );								// taxation rule
v[1] = V( "tr" );								// tax rate

// workers' wages
v[0] = VS( CAPSECL0, "W1" ) + VS( CONSECL0, "W2" );

if ( i == 1 )
	v[0] *= 1 - v[1];							// apply tax to workers

// capitalists' income (past period dividends)
v[0] += VLS( CAPSECL0, "Div1", 1 ) + VLS( CONSECL0, "Div2", 1 ) + 
		VLS( FINSECL0, "DivB", 1 );

if ( i == 2 )
	v[0] *= 1 - v[1];							// apply tax to all

// handle accumulated forced savings from the past
switch ( ( int ) V( "flagCons" ) )
{
	case 0:										// ignore unfilled past demand
		break;
		
	case 1:												
		// spend all savings, deducted from entry/exit public cost/credit
		v[0] += V( "SavAcc" );
		WRITE( "SavAcc", 0 );
		break;
		
	case 2:
	default:
		// slow spend of unfulfilled past consumption is necessary to avoid
		// economy overheating, so recover up to a limit of current consumption
		v[2] = V( "SavAcc" );					// accumulated forced savings
		v[3] = v[0] * V( "Crec" );				// max recover limit
		
		if ( v[2] <= v[3] )						// fit in limit?
		{
			v[0] += v[2];						// yes: use all savings
			WRITE( "SavAcc", 0 );
		}
		else
		{
			v[0] += v[3];						// no: spend the limit
			INCR( "SavAcc", - v[3] );			// update forced savings
		}
}

RESULT( v[0] )


EQUATION( "G" )
/*
Government expenditure (exogenous demand)
*/

i = V( "flagGovExp" );							// type of govt. exped.
j = V( "flagFiscalRule" );						// fiscal rule to apply

v[2] = VS( LABSUPL0, "Ls" ) - VS( LABSUPL0, "L" );// unemployed workers

if ( i < 2 )									// work-or-die + min
	v[0] = v[2] * VS( LABSUPL0, "w0min" );		// minimum income
else
	v[0] = v[2] * VS( LABSUPL0, "wU" );			// pay unemployment benefit
	
if ( i == 1 )
	v[0] += ( 1 + V( "gG" ) ) * CURRENT;

if ( i == 3 )									// if government has accumulated 
{												// surplus, it may spend it
	v[3] = VL( "Deb", 1 );
	if ( v[3] < 0 )
	{
		v[4] = max( 0, - VL( "Def", 1 ) );		// limit to current surplus
		if ( - v[3] > v[4] )
		{
			v[0] += v[4];
			INCR( "Deb", v[4] );				// discount from debt
		}
		else
		{
			v[0] += - v[3];						// spend all surplus
			WRITE( "Deb", 0 );					// zero debt
		}
		
		END_EQUATION( v[0] );					// don't apply fiscal rules
	}
}

// apply fiscal rule if soft balanced budget rule not binding, after warm-up
if ( ( j == 1 || j == 3 || ( j > 0 && VL( "dGDP", 1 ) > 0 ) ) && 
	 T >= VS( FINSECL0, "Trule" ) )
{
	v[5] = VS( FINSECL0, "DebRule" );			// primary deficit rule limit
	v[6] = VS( FINSECL0, "DefPrule" );			// primary deficit rule limit
	v[7] = VL( "Tax", 1 );						// tax income
	v[8] = VL( "Deb", 1 );						// public debt
	v[9] = VL( "GDPnom", 1 );					// current GDP in nominal terms

	if ( j > 2 && v[8] / v[9] > v[5] )			// debt rule applies?
	{
		// desired surplus
		v[10] = - VS( FINSECL0, "deltaDeb" ) * ( v[8] / v[9] - v[5] ) * v[8];
		
		if ( v[0] - v[7] > v[10] )				// expected surplus not enough?
			v[0] = v[7] + v[10];				// adjust to the desired surplus	
	}
	else
		if ( v[0] - v[7] > v[6] * v[9] )		// deficit rule applies?
			v[0] = v[7] + v[6] * v[9];			// apply limit
}

RESULT( max( v[0], 0 ) )


EQUATION( "SavAcc" )
/*
Accumulated nominal consumption from previous periods (forced savings)
Also updated in 'C', 'Sav'
*/

// apply interest to the savings balance
v[0] = CURRENT * ( 1 + VS( FINSECL0, "rD" ) );	// update savings

// add entry (equity) / exit (net cash) cost/credit incurred by households
v[0] += - VL( "cEntry", 1 ) + VL( "cExit", 1 );

RESULT( ROUND( v[0], 0, 0.001 ) )				// avoid rounding errors on zero


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A" )
/*
Overall labor productivity
*/
RESULT( ( VS( CAPSECL0, "A1" ) * VS( CAPSECL0, "L1" ) + 
		  VS( CONSECL0, "A2" ) * VS( CONSECL0, "L2" ) ) / VS( LABSUPL0, "L" ) )


EQUATION( "Deb" )
/*
Accumulated government debt
Includes the operational result of central bank (interest paid on reserves
less the interest accrued on liquidity lines)
*/
RESULT( CURRENT + V( "Def" ) + 
		VS( FINSECL0, "rRes" ) * VLS( FINSECL0, "Res", 1 ) -
		VS( FINSECL0, "r" ) * VLS( FINSECL0, "LoansCB", 1 ) )


EQUATION( "DebGDP" )
/*
Government debt on GDP ratio
*/
v[1] = V( "GDPnom" );
RESULT( v[1] > 0 ? V( "Deb" ) / v[1] : CURRENT )


EQUATION( "Def" )
/*
Government total deficit (negative if surplus)
*/
RESULT( V( "DefP" ) + VLS( FINSECL0, "Gbail", 1 ) + 
		VS( FINSECL0, "rBonds" ) * VLS( FINSECL0, "Bonds", 1 ) )


EQUATION( "DefP" )
/*
Government primary deficit (negative if surplus)
*/
RECALC( "Deb" );								// ensure debt update next
RESULT( V( "G" ) - VL( "Tax", 1 ) )


EQUATION( "DefPgdp" )
/*
Government primary deficit on GDP ratio
*/
v[1] = V( "GDPnom" );
RESULT( v[1] > 0 ? V( "DefP" ) / v[1] : CURRENT )


EQUATION( "Em" )
/*
Total CO2 (carbon) emissions produced
*/
RESULT( VS( CAPSECL0, "Em1" ) + VS( CONSECL0, "Em2" ) + VS( ENESECL0, "EmE" ) )


EQUATION( "En" )
/*
Total energy consumed
*/
RESULT( VS( CAPSECL0, "En1" ) + VS( CONSECL0, "En2" ) )


EQUATION( "GDP" )
/*
Gross domestic product (real terms)
*/
RESULT( VS( CAPSECL0, "Q1e" ) + VS( CONSECL0, "Q2e" ) )


EQUATION( "GDPnom" )
/*
Gross domestic product (nominal/currency terms)
*/
RESULT( V( "C" ) - V( "Sav" ) + VS( CONSECL0, "Inom" ) + V( "G" ) + 
		VS( CONSECL0, "dNnom" ) )


EQUATION( "Sav" )
/*
Residual nominal consumption in period (forced savings in currency terms)
*/

// unfilled demand=forced savings
v[0] = V( "C" ) + V( "G" ) - VS( CONSECL0, "D2" ) * VS( CONSECL0, "CPI" );
v[0] = ROUND( v[0], 0, 0.001 );					// avoid rounding errors on zero

V( "SavAcc" );									// ensure up-to-date before
INCR( "SavAcc", v[0] );							// updating accumulated

RESULT( v[0] )


EQUATION( "Tax" )
/*
Government tax income
*/

i = V( "flagTax" );								// taxation rule

v[1] = 0;										// taxable income acc.

if ( i >= 1 )									// tax workers' income?
	v[1] += VS( CAPSECL0, "W1" ) + VS( CONSECL0, "W2" );

if ( i >= 2 )									// tax capitalists' income?
	v[1] += VLS( CAPSECL0, "Div1", 1 ) + VLS( CONSECL0, "Div2", 1 ) + 
			VLS( FINSECL0, "DivB", 1 );

// compute household's taxes plus firms' taxes
v[0] = v[1] * V( "tr" ) + VS( CAPSECL0, "Tax1" ) + VS( CONSECL0, "Tax2" ) + 
	   VS( ENESECL0, "TaxE" ) + VS( FINSECL0, "TaxB" );

RESULT( v[0] )


EQUATION( "dAb" )
/*
Notional overall productivity (bounded) rate of change
Used for wages adjustment only
*/
RESULT( mov_avg_bound( THIS, "A", V( "mLim" ), V( "mPer" ) ) )


EQUATION( "dGDP" )
/*
Gross domestic product (log) growth rate
*/
RESULT( T > 1 ? log( V( "GDP" ) + 1 ) - log( VL( "GDP", 1 ) + 1 ) : 0 )


EQUATION( "entryExit" )
/*
Perform the entry and exit process in all sectors
*/

// ensure aggregates depending on firm objects are computed
UPDATE;											// country variables
UPDATES( FINSECL0 );							// financial sector variables
UPDATES( ENESECL0 );							// energy sector variables
UPDATES( LABSUPL0 );							// labor-supply variables
UPDATES( CLIMATL0 );							// carbon cycle variables
UPDATES( MACSTAL0 );							// statistics-only variables
UPDATES( SECSTAL0 );
UPDATES( LABSTAL0 );

// reset entry/exit cost/credit
WRITE( "cEntry", 0 );
WRITE( "cExit", 0 );

v[0] = VS( CAPSECL0, "entry1exit" ) + VS( CONSECL0, "entry2exit" );

VS( FINSECL0, "cScores" );						// set the credit pecking order
RECALCS( SECSTAL0, "BadDeb" );					// update bad debt after exits

RESULT( v[0] )


/*========================= INITIALIZATION EQUATION ==========================*/

EQUATION( "initCountry" )
/*
Initialize the K+S country object. It is run only once per country.
*/

PARAMETER;										// execute only once per country

// create the new country as an extension to current 'Country' object
ADDEXT_INIT( country );

// country-level pointers to speed-up the access to individual containers
WRITE_EXT( country, capSec, SEARCH( "Capital" ) );
WRITE_EXT( country, conSec, SEARCH( "Consumption" ) );
WRITE_EXT( country, finSec, SEARCH( "Financial" ) );
WRITE_EXT( country, labSup, SEARCH( "Labor" ) );
WRITE_EXT( country, eneSec, SEARCH( "Energy" ) );
WRITE_EXT( country, climat, SEARCH( "Climate" ) );
WRITE_EXT( country, macSta, SEARCH( "Mac" ) );
WRITE_EXT( country, secSta, SEARCH( "Sec" ) );
WRITE_EXT( country, labSta, SEARCH( "Lab" ) );

// pointer shortcuts the access to individual market containers
cur1 = CAPSECL0;
cur2 = CONSECL0;
cur3 = FINSECL0;
cur4 = LABSUPL0;
cur5 = ENESECL0;
cur6 = CLIMATL0;

// check unwanted extra instances (only one of each is required)
if ( COUNT( "Capital" ) > 1 || COUNT( "Consumption" ) > 1 || 
	 COUNT( "Financial" ) > 1 || COUNT( "Energy" ) > 1 || 
	 COUNT( "Climate" ) > 1 || COUNT( "Labor" ) > 1 || 
	 COUNT( "Stats" ) > 1 || COUNTS( cur3, "Bank" ) > 1 || 
	 COUNTS( cur1, "Firm1" ) > 1 || COUNTS( cur2, "Firm2" ) > 1 || 
	 COUNTS( cur5, "Dirty" ) > 1 || COUNTS( cur5, "Green" ) > 1 ||
	 COUNTS( SEARCHS( cur1, "Firm1" ), "Cli" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Broch" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Vint" ) > 1 ||
	 COUNTS( SEARCHS( cur3, "Bank" ), "Cli1" ) > 1 ||
	 COUNTS( SEARCHS( cur3, "Bank" ), "Cli2" ) > 1 )
{
	PLOG( "\n Error: multiple-instance objects not allowed, aborting!" );
	ABORT;
}

// ensure initial number of firms and banks is consistent
if ( VS( cur1, "F1min" ) < 1 || VS( cur2, "F2min" ) < 1 ||  VS( cur3, "B" ) < 1 ||
	 VS( cur1, "F1max" ) < VS( cur1, "F1min" ) || 
	 VS( cur2, "F2max" ) < VS( cur2, "F2min" ) )
{
	PLOG( "\n Error: invalid number of agents, aborting!" );
	ABORT;
}

WRITES( cur1, "F10", min( max( VS( cur1, "F10" ), VS( cur1, "F1min" ) ), 
						  VS( cur1, "F1max" ) ) );
WRITES( cur2, "F20", min( max( VS( cur2, "F20" ), VS( cur2, "F2min" ) ), 
						  VS( cur2, "F2max" ) ) );
						  
// adjust parameters required to be integer and positive
WRITES( cur2, "m2", max( 1, ceil( VS( cur2, "m2" ) ) ) );

// prepare data required to set initial conditions
double Ade0 = VS( cur5, "Ade0" );				// initial efficiency dirty plant
double EqB0 = VS( cur3, "EqB0" );				// initial bank equity multiple
double NW10 = VS( cur1, "NW10" );				// initial net worth in sector 1
double NW20 = VS( cur2, "NW20" );				// initial net worth in sector 2
double NWe0 = VS( cur5, "NWe0" );				// initial net worth in energy
double alphaB = VS( cur3, "alphaB" );			// bank size distrib. parameter
double emDE0 = VS( cur5, "emDE0" );				// initial emissions dirty plant
double fGE0 = VS( cur5, "fGE0" );				// initial share of green energy
double m1 = VS( cur1, "m1" );					// labor output factor
double m2 = VS( cur2, "m2" );					// machine output factor
double mu1 = VS( cur1, "mu1" );					// mark-up in sector 1
double mu20 = VS( cur2, "mu20" );				// initial mark-up in sector 2
double muBonds = VS( cur3, "muBonds" );			// interest mark-down on g. bonds
double muE = VS( cur5, "muE" );					// initial mark-up of energy
double pF = VS( cur5, "pF" );					// price of fossil fuel
double rT = VS( cur3, "rT" );					// prime rate target
double tauB = VS( cur3, "tauB" );				// minimum capital adequacy rate
double trCO2 = V( "trCO2" );					// carbon tax rate
double w0min = VS( cur4, "w0min" );				// absolute/initial minimum wage
int B = VS( cur3, "B" );						// number of banks
int F10 = VS( cur1, "F10" );					// initial firms in sector 1
int F20 = VS( cur2, "F20" );					// initial firms in sector 2
int F2max = VS( cur2, "F2max" );				// max firms in sector 2
int Ls0 = VS( cur4, "Ls0" );					// initial labor supply

double Btau0 = ( 1 + mu1 ) * INIPROD / ( m1 * m2 );// initial prod. in sector 1
double ICge0 = VS( cur5, "bE" ) * pF / Ade0;	// initial green plants cost
double pE0 = INIWAGE * muE + ( fGE0 == 1 ? 0 : pF / Ade0 );// init. energy price
double c10 = ( INIWAGE / Btau0 + ( pE0 + trCO2 * INIEFRI ) / INIEEFF ) / m1;
												// initial unit cost in sector 1
double c20 = INIWAGE / INIPROD + ( pE0 + trCO2 * INIEFRI ) / INIEEFF;
												// initial unit cost in sector 2
double p10 = ( 1 + mu1 ) * c10;					// initial price sector 1
double p20 = ( 1 + mu20 ) * c20;				// initial price sector 2
double Eavg0 = ( VS( cur2, "omega1" ) + VS( cur2, "omega2" ) ) / 2;	
												// initial competitiveness
double rBonds = rT * ( 1 - muBonds );			// initial interest on g. bonds
double G0 = V( "gG" ) * Ls0;					// initial public spending

// reserve space for country-level non-initialized vectors
EXEC_EXT( country, firm2ptr, reserve, F2max );	// sector 2 firm objects
EXEC_EXT( country, bankPtr, reserve, B );		// bank objects
EXEC_EXT( country, bankWgtd, reserve, B );

// reset serial ID counters for dynamic objects
WRITES( cur1, "lastID1", 0 );
WRITES( cur2, "lastID2", 0 );

// initialize lagged variables depending on parameters
WRITEL( "A", INIPROD, -1 );
WRITEL( "G", G0, -1 );
WRITELS( cur1, "A1", Btau0, -1 );
WRITELS( cur1, "F1", F10, -1 );
WRITELS( cur1, "PPI", p10, -1 );
WRITELS( cur1, "PPI0", p10, -1 );
WRITELS( cur1, "p1avg", p10, -1 );
WRITELS( cur2, "CPI", p20, -1 );
WRITELS( cur2, "Eavg", Eavg0, -1 );
WRITELS( cur2, "F2", F20, -1 );
WRITELS( cur2, "c2", c20, -1 );
WRITELS( cur3, "r", rT, -1 );
WRITELS( cur3, "rBonds", rBonds, -1 );
WRITELS( cur4, "Ls", Ls0, -1 );
WRITELS( cur4, "w", INIWAGE, -1 );
WRITELS( cur4, "wReal", INIWAGE, -1 );
WRITELS( cur5, "AtauDE", Ade0, -1 );			// energy sector initial conds.
WRITELS( cur5, "ICtauGE", ICge0, -1 );
WRITELS( cur5, "NWe", NWe0, -1 );
WRITELS( cur5, "emTauDE", emDE0, -1 );
WRITELS( cur5, "fKge", fGE0, -1 );
WRITELS( cur5, "pE", pE0, -1 );

// initialize climate
VS( cur6, "initClimate" );

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
	
	cur7 = SEARCHS( cur, "Cli1" );				// remove empty client instances
	cur8 = SEARCHS( cur, "Cli2" );
	DELETE( cur7 );
	DELETE( cur8 );
	
	++k;
}

CYCLES( cur3, cur, "Bank" )
	WRITES( cur, "_fD", VS( cur, "_fD" ) / i );	// adjust desired market shares

VS( cur3, "banksMaps" );						// update the mapping vectors

// create firms' objects and set initial values
cur = SEARCHS( cur1, "Firm1" );					// remove empty firm instances
DELETE( cur );
cur = SEARCHS( cur2, "Firm2" );
DELETE( cur );

v[1] = entry_firm1( var, cur1, F10, true );		// add capital-good firms
INIT_TSEARCHTS( cur1, "Firm1", F10 );			// prepare turbo search indexing

v[1] += entry_firm2( var, cur2, F20, true );	// add consumer-good firms
VS( cur2, "firm2maps" );						// update the mapping vectors

WRITE( "cEntry", v[1] );						// save equity cost of entry

// set banks initial assets according to existing loans to firms
v[2] = 0;										// capital accumulator
CYCLES( cur3, cur, "Bank" )
{
	v[3] = v[4] = 0;							// loans and deposits accumulator
	
	CYCLES( cur, cur7, "Cli1" )
	{
		v[3] += VLS( SHOOKS( cur7 ), "_Deb1", 1 );// firm debt
		v[4] += VLS( SHOOKS( cur7 ), "_NW1", 1 );// firm net wealth
	}

	CYCLES( cur, cur8, "Cli2" )
	{
		v[3] += VLS( SHOOKS( cur8 ), "_Deb2", 1 );// firm debt
		v[4] += VLS( SHOOKS( cur8 ), "_NW2", 1 );// firm net wealth
	}

	if ( v[4] > 0 )								// bank has clients?
		v[5] = EqB0 * v[4];						// bank initial tier 1 capital
	else
		v[5] = EqB0 * ( F10 * NW10 + F20 * NW20 ) * VS( cur, "_fD" );// use proxy

	v[6] = tauB * v[4];							// part on required reserves
	v[2] += v[7] = v[3] + v[5] - v[4];			// and net worth
		
	WRITELS( cur, "_Loans",  v[3], -1 );		// bank loans
	WRITELS( cur, "_Res", v[6], -1 );			// bank reserves at central bank
	WRITELS( cur, "_ExRes", v[5] - v[6], -1 );	// bank shareholder equity
	WRITELS( cur, "_Depo",  v[4], -1 );			// bank deposits/reserves
	WRITELS( cur, "_NWb", v[7], -1 );			// bank net worth
	WRITELS( cur, "_BadDeb",  0, -1 );			// no bad debt
	WRITELS( cur, "_Bda",  0, -1 );				// no fragility
}

WRITELS( cur3, "NWb", v[2], -1 );

// remove empty (uninitialized) power plants
cur = SEARCHS( cur5, "Dirty" );
DELETE( cur );
cur = SEARCHS( cur5, "Green" );
DELETE( cur );

RESULT( 1 )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "cEntry", "" )
/*
Entry costs (from equity) in current period in both sectors
Updated in 'initCountry', 'entry1exit', 'entry2exit'
*/

EQUATION_DUMMY( "cExit", "" )
/*
Exit credits (from net cash) in current period in both sectors
Updated in 'entry1exit', 'entry2exit'
*/
