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

EQUATION( "Cd" )
/*
Nominal (monetary terms) desired aggregated consumption
*/

// workers' net income after taxes
// wages and unemployment bonuses this period and past period dividends
v[0] = VS( LABSUPL0, "W" ) + V( "G" ) - VS( LABSUPL0, "TaxW" ) +
	   VL( "Div", 1 ) - V( "TaxDiv" );

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
		v[1] = V( "SavAcc" );					// accumulated forced savings
		v[2] = v[0] * V( "Crec" );				// max recover limit

		if ( v[1] <= v[2] )						// fit in limit?
		{
			v[0] += v[1];						// yes: use all savings
			WRITE( "SavAcc", 0 );
		}
		else
		{
			v[0] += v[2];						// no: spend the limit
			INCR( "SavAcc", - v[2] );			// update forced savings
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
v[3] = VLS( FINSECL0, "DepoG", 1 );				// accumulated surplus

if ( i < 2 )									// work-or-die + min
	v[0] = v[2] * VS( LABSUPL0, "w0min" );		// minimum income
else
	v[0] = v[2] * VS( LABSUPL0, "wU" );			// pay unemployment benefit

if ( i == 1 )
	v[0] += ( 1 + V( "gG" ) ) * CURRENT;

if ( v[3] > 0 )
{
	if ( i == 3 )								// spend accumulated surplus
		v[0] += min( v[3], max( 0, - VL( "Def", 1 ) ) );// limit to cur. deficit
}
else
{	// apply fiscal rule if soft balanced budget rule not binding, after warm-up
	if ( ( j == 1 || j == 3 || ( j > 0 && VL( "dGDP", 1 ) > 0 ) ) &&
		 T >= VS( FINSECL0, "Trule" ) )
	{
		v[5] = VS( FINSECL0, "DebRule" );		// primary deficit rule limit
		v[6] = VS( FINSECL0, "DefPrule" );		// primary deficit rule limit
		v[7] = VL( "Tax", 1 );					// tax income
		v[8] = VL( "Deb", 1 );					// public debt
		v[9] = VL( "GDPnom", 1 );				// current GDP in nominal terms

		if ( j > 2 && v[8] / v[9] > v[5] )		// debt rule applies?
		{
			// desired surplus
			v[10] = - VS( FINSECL0, "deltaDeb" ) * ( v[8] / v[9] - v[5] ) * v[8];

			if ( v[0] - v[7] > v[10] )			// expected surplus not enough?
				v[0] = v[7] + v[10];			// adjust to the desired surplus
		}
		else
			if ( v[0] - v[7] > v[6] * v[9] )	// deficit rule applies?
				v[0] = v[7] + v[6] * v[9];		// apply limit
	}
}

RESULT( max( v[0], 0 ) )


EQUATION( "SavAcc" )
/*
Accumulated nominal consumption from previous periods (forced savings)
Also updated in 'Cd', 'Sav'
*/

// apply interest to the savings balance
v[0] = CURRENT * ( 1 + VLS( FINSECL0, "rD", 1 ) );// update savings

// add entry (equity) / exit (net cash) cost/credit incurred by households
v[0] += - VL( "cEntry", 1 ) + VL( "cExit", 1 );

RESULT( ROUND( v[0], 0, 0.001 ) )				// avoid rounding errors on zero


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A" )
/*
Overall labor productivity
*/
RESULT( V( "GDPreal" ) / VS( LABSUPL0, "L" ) )


EQUATION( "C" )
/*
Nominal (monetary terms) aggregated consumption
*/
RESULT( VS( CONSECL0, "S2" ) )


EQUATION( "Creal" )
/*
Real (in initial prices terms) aggregated consumption
*/
RESULT( VS( CONSECL0, "Q2e" ) * VS( CONSECL0, "pC0" ) )


EQUATION( "Deb" )
/*
Accumulated (gross) government debt
*/
RESULT( VS( FINSECL0, "BondsB" ) + VS( FINSECL0, "BondsCB" ) )


EQUATION( "DebGDP" )
/*
Government debt on GDP ratio
*/
RESULT( V( "Deb" ) / V( "GDPnom" ) )


EQUATION( "Def" )
/*
Government total deficit (negative if surplus)
*/
RESULT( V( "DefP" ) - VS( FINSECL0, "PiCB" ) + VLS( FINSECL0, "Gbail", 1 ) +
		VLS( FINSECL0, "rBonds", 1 ) * ( VLS( FINSECL0, "BondsB", 1 ) +
									 VLS( FINSECL0, "BondsCB", 1 ) ) -
		VLS( FINSECL0, "rRes", 1 ) * VLS( FINSECL0, "DepoG", 1 ) +
		VS( ENESECL0, "CeEq" ) - VS( ENESECL0, "pF" ) * VS( ENESECL0, "Df" ) )


EQUATION( "DefP" )
/*
Government primary deficit (negative if surplus)
*/
RESULT( V( "G" ) - V( "Tax" ) )


EQUATION( "DefPgdp" )
/*
Government primary deficit on GDP ratio
*/
RESULT( V( "DefP" ) / V( "GDPnom" ) )


EQUATION( "Div" )
/*
Total dividends paid by firms and banks
*/
RESULT( VS( ENESECL0, "DivE" ) + VS( CAPSECL0, "Div1" ) +
		VS( CONSECL0, "Div2" ) + VS( FINSECL0, "DivB" ) )


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


EQUATION( "Eq" )
/*
Total firm equity hold by workers/households
*/
RESULT( VS( ENESECL0, "EqE" ) + VS( CAPSECL0, "Eq1" ) + VS( CONSECL0, "Eq2" ) )


EQUATION( "GDPreal" )
/*
Real (in initial prices terms) gross domestic product
*/
RESULT( max( VS( CONSECL0, "Ireal" ) + V( "Creal" ), 1 ) )


EQUATION( "GDPnom" )
/*
Gross domestic product (nominal/currency terms)
*/
RESULT( max( V( "C" ) + VS( CONSECL0, "Inom" ) + VS( CONSECL0, "dNnom" ), 1 ) )


EQUATION( "Sav" )
/*
Residual nominal consumption in period (forced savings in currency terms)
*/

// unfilled demand=forced savings
v[0] = V( "Cd" ) - V( "C" );
v[0] = ROUND( v[0], 0, 0.001 );					// avoid rounding errors on zero

V( "SavAcc" );									// ensure up-to-date before
INCR( "SavAcc", v[0] );							// updating accumulated

RESULT( v[0] )


EQUATION( "Tax" )
/*
Government tax income
*/
RESULT( VS( ENESECL0, "TaxE" ) + VS( CAPSECL0, "Tax1" ) + VS( CONSECL0, "Tax2" ) +
		VS( FINSECL0, "TaxB" ) + VS( LABSUPL0, "TaxW" ) + V( "TaxDiv" ) )


EQUATION( "TaxDiv" )
/*
Total taxes paid on dividends
*/
RESULT( V( "flagTax" ) >= 2 ? VL( "Div", 1 ) * V( "tr" ) : 0 )


EQUATION( "cEntry" )
/*
Cost (new equity) of firm entries in both industrial sectors
This variable must be explicitly recalculated after entry/exit
*/
RESULT( VS( ENESECL0, "cEntryE" ) + VS( CAPSECL0, "cEntry1" ) +
		VS( CONSECL0, "cEntry2" ) )


EQUATION( "cExit" )
/*
Credits (returned equity) from firm exits in both industrial sectors
This variable must be explicitly recalculated after entry/exit
*/
RESULT( VS( ENESECL0, "cExitE" ) + VS( CAPSECL0, "cExit1" ) +
		VS( CONSECL0, "cExit2" ) )


EQUATION( "dAb" )
/*
Notional overall productivity (bounded) rate of change
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "A", V( "mLim" ), V( "mPer" ) ) )


EQUATION( "dGDP" )
/*
Gross domestic product (log) growth rate
*/
RESULT( T > 1 ? log( V( "GDPreal" ) ) - log( VL( "GDPreal", 1 ) ) : 0 )


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
UPDATES( ENESTAL0 );
UPDATES( SECSTAL0 );
UPDATES( LABSTAL0 );

v[0] = VS( CAPSECL0, "entry1exit" ) + VS( CONSECL0, "entry2exit" ) +
	   VS( ENESECL0, "entryEexit" );

// recompute variables affected by entry/exit
RECALC( "Eq" );									// total equity after exit/entry
RECALC( "cEntry" );								// new equity after exit/entry
RECALC( "cExit" );								// returned equity from exits
RECALCS( FINSECL0, "BadDeb" );					// bad debt update after exits

VS( FINSECL0, "cScores" );						// set the credit pecking order

RESULT( v[0] )


/*========================= INITIALIZATION EQUATION ==========================*/

EQUATION( "initCountry" )
/*
Initialize the K+S country object. It is run only once per country.
*/

PARAMETER;										// execute only once per country

// create the new country as an extension to current 'Country' object
ADDEXT_INIT( countryE );

// country-level pointers to speed-up the access to individual containers
WRITE_EXT( countryE, capSec, SEARCH( "Capital" ) );
WRITE_EXT( countryE, conSec, SEARCH( "Consumption" ) );
WRITE_EXT( countryE, finSec, SEARCH( "Financial" ) );
WRITE_EXT( countryE, labSup, SEARCH( "Labor" ) );
WRITE_EXT( countryE, eneSec, SEARCH( "Energy" ) );
WRITE_EXT( countryE, climat, SEARCH( "Climate" ) );
WRITE_EXT( countryE, macSta, SEARCH( "Mac" ) );
WRITE_EXT( countryE, eneSta, SEARCH( "Ene" ) );
WRITE_EXT( countryE, secSta, SEARCH( "Sec" ) );
WRITE_EXT( countryE, labSta, SEARCH( "Lab" ) );

// pointer shortcuts the access to individual market containers
cur1 = CAPSECL0;
cur2 = CONSECL0;
cur3 = FINSECL0;
cur4 = LABSUPL0;
cur5 = ENESECL0;
cur6 = CLIMATL0;

// reset initial conditions, if restarting
CFUN( init_cond, NULL );

// check unwanted extra instances (only one of each is required)
if ( COUNT( "Capital" ) > 1 || COUNT( "Consumption" ) > 1 ||
	 COUNT( "Financial" ) > 1 || COUNT( "Energy" ) > 1 ||
	 COUNT( "Climate" ) > 1 || COUNT( "Labor" ) > 1 ||
	 COUNT( "Stats" ) > 1 ||
	 COUNTS( cur1, "Firm1" ) > 1 || COUNTS( cur2, "Firm2" ) > 1 ||
	 COUNTS( cur3, "Bank" ) > 1 || COUNTS( cur5, "FirmE" ) > 1 ||
	 COUNTS( SEARCHS( cur1, "Firm1" ), "Cli" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Broch" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Vint" ) > 1 ||
	 COUNTS( SEARCHS( cur3, "Bank" ), "Cli1" ) > 1 ||
	 COUNTS( SEARCHS( cur3, "Bank" ), "Cli2" ) > 1 ||
	 COUNTS( SEARCHS( cur5, "FirmE" ), "Dirty" ) > 1 ||
	 COUNTS( SEARCHS( cur5, "FirmE" ), "Green" ) > 1 )
{
	PLOG( "\n Error: multiple-instance objects not allowed, aborting!" );
	ABORT;
}

// ensure initial number of firms and banks is consistent
if ( VS( cur1, "F1min" ) < 1 || VS( cur2, "F2min" ) < 1 ||
	 VS( cur3, "B" ) < 1 || VS( cur5, "FeMin" ) < 1 ||
	 VS( cur1, "F1max" ) < VS( cur1, "F1min" ) ||
	 VS( cur2, "F2max" ) < VS( cur2, "F2min" ) ||
	 VS( cur5, "FeMax" ) < VS( cur5, "FeMin" ) )
{
	PLOG( "\n Error: invalid number of agents, aborting!" );
	ABORT;
}

WRITES( cur1, "F10", min( max( VS( cur1, "F10" ), VS( cur1, "F1min" ) ),
						  VS( cur1, "F1max" ) ) );
WRITES( cur2, "F20", min( max( VS( cur2, "F20" ), VS( cur2, "F2min" ) ),
						  VS( cur2, "F2max" ) ) );
WRITES( cur5, "Fe0", min( max( VS( cur5, "Fe0" ), VS( cur5, "FeMin" ) ),
						  VS( cur5, "FeMax" ) ) );

// adjust parameters required to be integer and positive
WRITES( cur2, "m2", max( 1, ceil( VS( cur2, "m2" ) ) ) );

// prepare data required to set initial conditions
double EqB0 = VS( cur3, "EqB0" );				// initial bank equity multiple
double NWe0 = VS( cur5, "NWe0" );				// initial net worth in en. sec.
double NW10 = VS( cur1, "NW10" );				// initial net worth in sector 1
double NW20 = VS( cur2, "NW20" );				// initial net worth in sector 2
double alphaB = VS( cur3, "alphaB" );			// bank size distrib. parameter
double tauB = VS( cur3, "tauB" );				// minimum capital adequacy rate
double p10 = CFUN( init_cond, "p10" );			// initial price sector 1
double p20 = CFUN( init_cond, "p20" );			// initial price sector 2
int B = VS( cur3, "B" );						// number of banks
int Fe0 = VS( cur5, "Fe0" );					// initial energy producers
int FeMax = VS( cur5, "FeMax" );				// max energy producers
int F10 = VS( cur1, "F10" );					// initial firms in sector 1
int F20 = VS( cur2, "F20" );					// initial firms in sector 2
int F2max = VS( cur2, "F2max" );				// max firms in sector 2

// reserve space for country-level non-initialized vectors
EXEC_EXT( countryE, firmEptr, reserve, FeMax );	// energy sector firm objects
EXEC_EXT( countryE, firm2ptr, reserve, F2max );	// sector 2 firm objects
EXEC_EXT( countryE, bankPtr, reserve, B );		// bank objects
EXEC_EXT( countryE, bankWgtd, reserve, B );

// reset serial ID counters for dynamic objects
WRITES( cur1, "lastID1", 0 );
WRITES( cur2, "lastID2", 0 );

// initialize lagged variables depending on parameters
WRITEL( "A", CFUN( init_cond, "A0" ), -1 );
WRITEL( "Def", CFUN( init_cond, "Def0" ), -1 );
WRITEL( "Em", CFUN( init_cond, "Em0" ), -1 );
WRITEL( "En", CFUN( init_cond, "En0" ), -1 );
WRITEL( "G", CFUN( init_cond, "G0" ), -1 );
WRITEL( "GDPnom", CFUN( init_cond, "GDPnom0" ), -1 );
WRITEL( "GDPreal", CFUN( init_cond, "GDPnom0" ), -1 );
WRITEL( "Tax", CFUN( init_cond, "Tax0" ), -1 );
WRITES( cur1, "pK0", p10 );
WRITES( cur2, "pC0", p20 );
WRITES( cur5, "ICge0", CFUN( init_cond, "ICge0" ) );
WRITELS( cur1, "A1", CFUN( init_cond, "BtauLP0" ), -1 );
WRITELS( cur1, "F1", F10, -1 );
WRITELS( cur1, "L1", CFUN( init_cond, "L10" ), -1 );
WRITELS( cur1, "PPI", p10, -1 );
WRITELS( cur1, "p1avg", p10, -1 );
WRITELS( cur2, "CPI", p20, -1 );
WRITELS( cur2, "Eavg", CFUN( init_cond, "E0" ), -1 );
WRITELS( cur2, "F2", F20, -1 );
WRITELS( cur2, "c2", CFUN( init_cond, "c20" ), -1 );
WRITELS( cur3, "r", CFUN( init_cond, "r0" ), -1 );
WRITELS( cur3, "rBonds", CFUN( init_cond, "rBonds0" ), -1 );
WRITELS( cur3, "rD", CFUN( init_cond, "rD0" ), -1 );
WRITELS( cur3, "rDeb", CFUN( init_cond, "rDeb0" ), -1 );
WRITELS( cur3, "rRes", CFUN( init_cond, "rRes0" ), -1 );
WRITELS( cur4, "Ls", VS( cur4, "Ls0" ), -1 );
WRITELS( cur4, "U", CFUN( init_cond, "U0" ), -1 );
WRITELS( cur4, "w", CFUN( init_cond, "w0" ), -1 );
WRITELS( cur4, "wReal", CFUN( init_cond, "wReal0" ), -1 );
WRITELS( cur5, "Ae", CFUN( init_cond, "Ae0" ), -1 );
WRITELS( cur5, "AeMavg", CFUN( init_cond, "Ae0" ), -1 );
WRITELS( cur5, "De", CFUN( init_cond, "De0" ), -1 );
WRITELS( cur5, "Df", CFUN( init_cond, "Df0" ), -1 );
WRITELS( cur5, "EmE", CFUN( init_cond, "EmE0" ), -1 );
WRITELS( cur5, "pE", CFUN( init_cond, "pE0" ), -1 );
WRITELS( cur5, "pF", VS( cur5, "pF0" ), -1 );

// variables not to recalculate in t=1
WRITES( cur5, "pE", CFUN( init_cond, "pE0" ) );

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

	DELETE( SEARCHS( cur, "Cli1" ) );			// remove empty client instances
	DELETE( SEARCHS( cur, "Cli2" ) );
	DELETE( SEARCHS( cur, "CliE" ) );

	++k;
}

CYCLES( cur3, cur, "Bank" )
	WRITES( cur, "_fD", VS( cur, "_fD" ) / i );	// adjust desired market shares

VS( cur3, "banksMaps" );						// update the mapping vectors

// create firms' objects and set initial values
DELETE( SEARCHS( cur1, "Firm1" ) );				// remove empty firm instances
DELETE( SEARCHS( cur2, "Firm2" ) );
DELETE( SEARCHS( cur5, "FirmE" ) );

v[1] = CFUNS( cur1, entry_firm1, F10, true );	// add capital-good firms
INIT_TSEARCHS( cur1, "Firm1" );					// prepare turbo search indexing

v[1] += CFUNS( cur2, entry_firm2, F20, true );	// add consumer-good firms
VS( cur2, "firm2maps" );						// update the mapping vectors

v[1] += CFUNS( cur5, entry_firmE, Fe0, true );	// add energy producers
VS( cur5, "firmEmaps" );						// update the mapping vectors

WRITEL( "Eq", v[1], -1 );						// save existing equity

// set banks initial assets according to existing loans to firms
v[2] = v[3] = v[4] = v[5] = v[6] = 0;			// accumulators
CYCLES( cur3, cur, "Bank" )
{
	v[7] = v[8] = 0;							// loans and deposits accumulator

	CYCLES( cur, cur7, "Cli1" )
	{
		v[7] += VLS( SHOOKS( cur7 ), "_Deb1", 1 );// firm debt
		v[8] += VLS( SHOOKS( cur7 ), "_NW1", 1 );// firm net wealth
	}

	CYCLES( cur, cur8, "Cli2" )
	{
		v[7] += VLS( SHOOKS( cur8 ), "_Deb2", 1 );// firm debt
		v[8] += VLS( SHOOKS( cur8 ), "_NW2", 1 );// firm net wealth
	}

	CYCLES( cur, cur9, "CliE" )
	{
		v[7] += VLS( SHOOKS( cur9 ), "_DebE", 1 );// firm debt
		v[8] += VLS( SHOOKS( cur9 ), "_NWe", 1 );// firm net wealth
	}

	v[2] += v[7];
	v[3] += v[8];

	if ( v[8] > 0 )								// bank has clients?
		v[9] = EqB0 * v[8];						// bank initial tier 1 capital
	else
		v[9] = EqB0 * ( F10 * NW10 + F20 * NW20 + Fe0 * NWe0 ) *
			   VS( cur, "_fD" );// use proxy

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
