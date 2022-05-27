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
v[2] = VS( LABSUPL0, "Ls" ) - VS( LABSUPL0, "L" );// unemployed workers
v[3] = VLS( FINSECL0, "DepoG", 1 );				// accumulated surplus

if ( i < 2 )									// work-or-die + min
	v[0] = v[2] * VS( LABSUPL0, "w0min" );		// minimum income
else
	v[0] = v[2] * VS( LABSUPL0, "wU" );			// pay unemployment benefit

if ( i == 1 )
	v[0] += ( 1 + V( "gG" ) ) * CURRENT;

if ( v[3] > 0 && i == 3 )						// spend accumulated surplus
	v[0] += min( v[3], max( 0, - VL( "Def", 1 ) ) );// limit to cur. deficit

RESULT( v[0] )


EQUATION( "SavAcc" )
/*
Accumulated nominal consumption from previous periods (forced savings)
Also updated in 'Cd', 'Sav'
*/

// apply interest to the savings balance
v[0] = CURRENT * ( 1 + VS( FINSECL0, "r" ) );	// update savings

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
Accumulated government debt
*/
RESULT( max( CURRENT + V( "Def" ) - VLS( FINSECL0, "DepoG", 1 ), 0 ) )


EQUATION( "DebGDP" )
/*
Government debt on GDP ratio
*/
RESULT( V( "Deb" ) / V( "GDPnom" ) )


EQUATION( "Def" )
/*
Government total deficit (negative if surplus)
*/
RESULT( V( "DefP" ) - VS( FINSECL0, "PiB" ) )


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
RESULT( VS( CAPSECL0, "Div1" ) + VS( CONSECL0, "Div2" ) )


EQUATION( "Eq" )
/*
Total firm equity hold by workers/households
*/
RESULT( VS( CAPSECL0, "Eq1" ) + VS( CONSECL0, "Eq2" ) )


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
RESULT( VS( CAPSECL0, "Tax1" ) + VS( CONSECL0, "Tax2" ) +
		VS( LABSUPL0, "TaxW" ) + V( "TaxDiv" ) )


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
RESULT( VS( CAPSECL0, "cEntry1" ) + VS( CONSECL0, "cEntry2" ) )


EQUATION( "cExit" )
/*
Credits (returned equity) from firm exits in both industrial sectors
This variable must be explicitly recalculated after entry/exit
*/
RESULT( VS( CAPSECL0, "cExit1" ) + VS( CONSECL0, "cExit2" ) )


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
RESULT( T > 1 ? log( V( "GDPreal" ) ) - log( VL( "GDPreal", 1 ) ) : 0 )


EQUATION( "entryExit" )
/*
Perform the entry and exit process in all sectors
*/

// ensure aggregates depending on firm objects are computed
UPDATE;											// country variables
UPDATES( FINSECL0 );							// financial sector variables
UPDATES( LABSUPL0 );							// labor-supply variables
UPDATES( MACSTAL0 );							// statistics-only variables
UPDATES( SECSTAL0 );
UPDATES( LABSTAL0 );

v[0] = VS( CAPSECL0, "entry1exit" ) + VS( CONSECL0, "entry2exit" );

// recompute variables affected by entry/exit
RECALC( "Eq" );									// total equity after exit/entry
RECALC( "cEntry" );								// new equity after exit/entry
RECALC( "cExit" );								// returned equity from exits
RECALCS( FINSECL0, "BadDeb" );					// bad debt update after exits

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
WRITE_EXT( countryE, macSta, SEARCH( "Mac" ) );
WRITE_EXT( countryE, secSta, SEARCH( "Sec" ) );
WRITE_EXT( countryE, labSta, SEARCH( "Lab" ) );

// pointer shortcuts the access to individual market containers
cur1 = CAPSECL0;
cur2 = CONSECL0;
cur3 = FINSECL0;
cur4 = LABSUPL0;

// check unwanted extra instances (only one of each is required)
if ( COUNT( "Capital" ) > 1 || COUNT( "Consumption" ) > 1 ||
	 COUNT( "Financial" ) > 1 || COUNT( "Labor" ) > 1 ||
	 COUNT( "Stats" ) > 1 || COUNTS( cur1, "Firm1" ) > 1 ||
	 COUNTS( cur2, "Firm2" ) > 1 ||
	 COUNTS( SEARCHS( cur1, "Firm1" ), "Cli" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Broch" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Vint" ) > 1 )
{
	PLOG( "\n Error: multiple-instance objects not allowed, aborting!" );
	ABORT;
}

// ensure initial number of firms and banks is consistent
if ( VS( cur1, "F1" ) < 1 || VS( cur2, "F2" ) < 1 )
{
	PLOG( "\n Error: invalid number of agents, aborting!" );
	ABORT;
}

// adjust parameters required to be integer and positive
WRITES( cur2, "m2", max( 1, ceil( VS( cur2, "m2" ) ) ) );

// prepare data required to set initial conditions
double eta = VS( cur2, "eta" );					// technical machine life time
double m1 = VS( cur1, "m1" );					// output factor in sector 1
double m2 = VS( cur2, "m2" );					// output factor in sector 2
double mu1 = VS( cur1, "mu1" );					// mark-up in sector 1
double mu20 = VS( cur2, "mu20" );				// initial mark-up in sector 2
double phi = VS( cur4, "phi" );					// unemployment benefit rate
int F1 = VS( cur1, "F1" );						// number of firms in sector 1
int F2 = VS( cur2, "F2" );						// number of firms in sector 2
int Ls0 = VS( cur4, "Ls0" );					// initial labor supply

double Btau0 = ( 1 + mu1 ) * INIPROD /			// initial productivity in sec. 1
			   ( m1 * m2 * VS( cur2, "b" ) );
double c10 = INIWAGE / ( Btau0 * m1 );			// initial cost in sector 1
double c20 = INIWAGE / INIPROD;					// initial cost in sector 2
double p10 = ( 1 + mu1 ) * c10;					// initial price sector 1
double p20 = ( 1 + mu20 ) * c20;				// initial price sector 2
double trW = V( "flagTax" ) > 0 ? V( "tr" ) : 0;// tax rate on wages
double K0 = Ls0 * INIWAGE / p20;				// full employment K required
double D10 = K0 / ( m2 * eta );					// initial demand for sector 1
double RD0 = VS( cur1, "nu" ) * D10 * p10;		// initial R&D expense
double D20 = ( ( D10 * c10 + RD0 ) * ( 1 - phi - trW ) + Ls0 * INIWAGE * phi ) /
			 ( mu20 + phi + trW ) * c20;		// initial demand for sector 2
double Ld10 = RD0 / INIWAGE + D10 / ( Btau0 * m1 );// initial labor demand sec. 1
double Ld20 = D20 / INIPROD;					// initial labor demand sector 2
double A0 = ( D10 * p10 + D20 * p20 ) / ( Ld10 + Ld20 );// initial prod.
double Eavg0 = ( VS( cur2, "omega1" ) + VS( cur2, "omega2" ) ) / 2;
												// initial competitiveness
double G0 = V( "gG" ) * Ls0;					// initial public spending

// reserve space for country-level non-initialized vectors
EXEC_EXT( countryE, firm2ptr, reserve, F2 );	// sector 2 firm objects

// reset serial ID counters for dynamic objects
WRITES( cur1, "lastID1", 0 );
WRITES( cur2, "lastID2", 0 );

// initialize lagged variables depending on parameters
WRITEL( "A", A0, -1 );
WRITEL( "G", G0, -1 );
WRITES( cur1, "pK0", p10 );
WRITES( cur2, "pC0", p20 );
WRITELS( cur1, "A1", Btau0, -1 );
WRITELS( cur1, "PPI", p10, -1 );
WRITELS( cur1, "p1avg", p10, -1 );
WRITELS( cur2, "CPI", p20, -1 );
WRITELS( cur2, "Eavg", Eavg0, -1 );
WRITELS( cur2, "c2", c20, -1 );
WRITELS( cur4, "Ls", Ls0, -1 );
WRITELS( cur4, "w", INIWAGE, -1 );

// create firms' objects and set initial values
DELETE( SEARCHS( cur1, "Firm1" ) );				// remove empty firm instances
DELETE( SEARCHS( cur2, "Firm2" ) );

v[1] = entry_firm1( var, cur1, F1, true );		// add capital-good firms
INIT_TSEARCHTS( cur1, "Firm1", F1 );			// prepare turbo search indexing

v[1] += entry_firm2( var, cur2, F2, true );		// add consumer-good firms
VS( cur2, "firm2maps" );						// update the mapping vectors

WRITEL( "Eq", v[1], -1 );						// save existing equity

// set bank initial assets according to existing loans to firms
v[2] = v[3] = 0;								// loans and deposits accumulator

CYCLES( cur1, cur5, "Firm1" )
{
	v[2] += VLS( cur5, "_Deb1", 1 );			// firm debt
	v[3] += VLS( cur5, "_NW1", 1 );				// firm net wealth
}

CYCLES( cur2, cur6, "Firm2" )
{
	v[2] += VLS( cur6, "_Deb2", 1 );			// firm debt
	v[3] += VLS( cur6, "_NW2", 1 );				// firm net wealth
}

WRITELS( cur3, "Loans",	 v[2], -1 );			// bank loans
WRITELS( cur3, "Depo",	v[3], -1 );				// bank deposits

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
