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
v[0] += VLS( CAPSECL0, "Div1", 1 ) + VLS( CONSECL0, "Div2", 1 );

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
v[2] = VS( LABSUPL0, "Ls" ) - VS( LABSUPL0, "L" );// unemployed workers

v[0] = 0;										// wages accumulator

if ( i == 0 )									// work-or-die + min
	// minimum income
	v[0] += v[2] * VS( LABSUPL0, "w0min" );	
else
{		
	if ( i == 1 )								// growing g. expenditure
		v[0] += ( 1 + V( "gG" ) ) * CURRENT;	// do fixed public spending
	else										// unemployment benefit
	{
		// pay unemployment benefit
		v[0] += v[2] * VS( LABSUPL0, "wU" );
		
		// if government has an accumulated surplus, it may spend it 
		v[3] = VL( "Deb", 1 );
		if ( i == 3 && v[3] < 0 )
		{
			v[4] = max( 0, - VL( "Def", 1 ) );	// limit to cur. superavit
			if ( - v[3] > v[4] )
			{
				v[0] += v[4];					// cap to current sup.
				INCR( "Deb", v[4] );			// discount from surplus
			}
			else
			{
				v[0] += - v[3];					// spend all surplus
				WRITE( "Deb", 0 );				// zero surplus
			}
		}
	}
}

RESULT( v[0] )


EQUATION( "SavAcc" )
/*
Accumulated nominal consumption from previous periods (forced savings)
Also updated in 'C', 'Sav'
*/

// apply interest to the savings balance
v[0] = CURRENT * ( 1 + VS( FINSECL0, "r" ) );	// update savings

// add entry (equity) / exit (net cash) cost/credit incurred by households
v[0] += - VL( "cEntry", 1 ) + VL( "cExit", 1 );

RESULT( ROUND( v[0], 0, 0.001 ) )				// avoid rounding errors on zero


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A" )
/*
Overall productivity
*/
RESULT( ( VS( CAPSECL0, "A1" ) * VS( CAPSECL0, "L1" ) + 
		  VS( CONSECL0, "A2" ) * VS( CONSECL0, "L2" ) ) / VS( LABSUPL0, "L" ) )


EQUATION( "Deb" )
/*
Accumulated government debt
*/
RESULT( CURRENT + V( "Def" ) )


EQUATION( "Def" )
/*
Government current deficit (negative if superavit)
*/
RECALC( "Deb" );								// force update if updated in 'C'
RESULT( V( "G" ) + VLS( FINSECL0, "r", 1 ) * VL( "Deb", 1 ) - VL( "Tax", 1 ) )


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
	v[1] += VS( CAPSECL0, "Div1" ) + VS( CONSECL0, "Div2" );

// compute household's taxes plus firms' taxes
v[0] = v[1] * V( "tr" ) + VS( CAPSECL0, "Tax1" ) + VS( CONSECL0, "Tax2" );

RESULT( v[0] )


EQUATION( "dAb" )
/*
Notional overall productivity (bounded) rate of change
Used for wages adjustment only
*/
RESULT( mov_avg_bound( p, "A", V( "mLim" ) ) )


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
UPDATES( LABSUPL0 );							// labor-supply variables
UPDATES( MACSTAL0 );							// statistics-only variables
UPDATES( SECSTAL0 );
UPDATES( LABSTAL0 );

// reset entry/exit cost/credit
WRITE( "cEntry", 0 );
WRITE( "cExit", 0 );

v[0] = VS( CAPSECL0, "entry1exit" ) + VS( CONSECL0, "entry2exit" );

RECALCS( FINSECL0, "BadDeb" );					// update bad debt after exits

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
WRITE_EXT( country, macSta, SEARCH( "Mac" ) );
WRITE_EXT( country, secSta, SEARCH( "Sec" ) );
WRITE_EXT( country, labSta, SEARCH( "Lab" ) );

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
double m1 = VS( cur1, "m1" );					// labor output factor
double mu1 = VS( cur1, "mu1" );					// mark-up in sector 1
double mu20 = VS( cur2, "mu20" );				// initial mark-up in sector 2
double w0min = VS( cur4, "w0min" );				// absolute/initial minimum wage
int F1 = VS( cur1, "F1" );						// number of firms in sector 1
int F2 = VS( cur2, "F2" );						// number of firms in sector 2
int Ls0 = VS( cur4, "Ls0" );					// initial labor supply

double p10 = ( 1 + mu1 ) / m1;					// initial price sector 1
double p20 = 1 + mu20;							// initial price sector 2
double G0 = V( "gG" ) * Ls0;					// initial public spending

// reserve space for country-level non-initialized vectors
EXEC_EXT( country, firm2ptr, reserve, F2 );		// sector 2 firm objects

// reset serial ID counters for dynamic objects
WRITES( cur1, "lastID1", 0 );
WRITES( cur2, "lastID2", 0 );

// initialize lagged variables depending on parameters
WRITEL( "G", G0, -1 );
WRITELS( cur1, "PPI", p10, -1 );
WRITELS( cur1, "PPI0", p10, -1 );
WRITELS( cur2, "CPI", p20, -1 );
WRITELS( cur4, "Ls", Ls0, -1 );

// create firms' objects and set initial values
cur = SEARCHS( cur1, "Firm1" );					// remove empty firm instance
DELETE( cur );
cur = SEARCHS( cur2, "Firm2" );					// remove empty firm instance
DELETE( cur );

v[1] = entry_firm1( cur1, F1, true );			// add capital-good firms
v[1] += entry_firm2( cur2, F2, true );			// add consumer-good firms

WRITE( "cEntry", v[1] );						// save equity cost of entry

INIT_TSEARCHTS( cur1, "Firm1", k - 1 );			// prepare turbo search indexing
VS( cur2, "firm2maps" );						// update the mapping vectors

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

WRITELS( cur3, "Loans",  v[2], -1 );			// bank loans
WRITELS( cur3, "Depo",  v[3], -1 );				// bank deposits
WRITELS( cur3, "BadDeb",  0, -1 );				// no bad debt

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
