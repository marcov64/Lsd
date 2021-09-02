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

// workers' income (wages this period and past period bonuses)
v[0] = VS( CAPSECL0, "W1" ) + VS( CONSECL0, "W2" ) + VLS( CONSECL0, "B2", 1 );

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
	
v[0] += VS( LABSUPL0, "Gtrain" );				// worker training cost

if ( i == 1 )
	v[0] += ( 1 + V( "gG" ) ) * ( CURRENT - VLS( LABSUPL0, "Gtrain", 1 ) );
	
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
	v[1] += VS( CAPSECL0, "W1" ) + VS( CONSECL0, "W2" ) + 
			VLS( CONSECL0, "B2", 1 );

if ( i >= 2 )									// tax capitalists' income?
	v[1] += VLS( CAPSECL0, "Div1", 1 ) + VLS( CONSECL0, "Div2", 1 ) + 
			VLS( FINSECL0, "DivB", 1 );

// compute household's taxes plus firms' taxes
v[0] = v[1] * V( "tr" ) + VS( CAPSECL0, "Tax1" ) + VS( CONSECL0, "Tax2" ) + 
	   VS( FINSECL0, "TaxB" );

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
UPDATES( LABSUPL0 );							// labor-supply variables
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


EQUATION( "regChg" )
/*
Produces a labor market regime change at the time step defined in TregChg
If TregChg is zero, there is no regime change
Changed parameters:
	Global:
		flagSearchMode -> flagSearchModeChg
		flagIndexMinWage -> flagIndexMinWageChg
		flagHireSeq -> flagHireSeqChg
		flagHireOrder1 -> flagHireOrder1Chg
		flagFireOrder1 -> flagFireOrder1Chg
		tr -> trChg
		Lambda -> LambdaChg
		muRes -> muResChg
		tauB -> tauBchg
		rT -> rTchg
		Ts -> TsChg
		phi -> phiChg
		e0 -> e0Chg
		mu20 -> mu20Chg
		omega -> omegaPosChg
	Firm-specific:
		flagHireOrder2 -> flagHireOrder2Chg
		flagFireOrder2 -> flagFireOrder2Chg
		flagFireRule -> flagFireRuleChg
		flagWageOffer -> flagWageOfferChg
		flagIndexWage -> flagIndexWageChg
		b -> bChg
*/

if ( T == ( int ) V( "TregChg" ) )				// in time, replace parameters
{
	WRITE( "flagSearchMode", V( "flagSearchModeChg" ) );
	WRITE( "flagIndexMinWage", V( "flagIndexMinWageChg" ) );
	WRITE( "flagHireSeq", V( "flagHireSeqChg" ) );
	WRITE( "flagHireOrder1", V( "flagHireOrder1Chg" ) );
	WRITE( "flagFireOrder1", V( "flagFireOrder1Chg" ) );
	WRITE( "tr", V( "trChg" ) );
	WRITES( FINSECL0, "Lambda", VS( FINSECL0, "LambdaChg" ) );
	WRITES( FINSECL0, "muRes", VS( FINSECL0, "muResChg" ) );
	WRITES( FINSECL0, "tauB", VS( FINSECL0, "tauBchg" ) );
	WRITES( FINSECL0, "rT", VS( FINSECL0, "rTchg" ) );
	WRITES( LABSUPL0, "Ts", VS( LABSUPL0, "TsChg" ) );
	WRITES( LABSUPL0, "phi", VS( LABSUPL0, "phiChg" ) );
	WRITES( LABSUPL0, "omega", VS( LABSUPL0, "omegaPosChg" ) );
	WRITES( CONSECL0, "e0", VS( CONSECL0, "e0Chg" ) );
	WRITES( CONSECL0, "mu20", VS( CONSECL0, "mu20Chg" ) );
	
	if ( ( int ) V( "flagAllFirmsChg" ) == 1 )
	{
		WRITE( "flagHireOrder2", V( "flagHireOrder2Chg" ) );
		WRITE( "flagFireOrder2", V( "flagFireOrder2Chg" ) );
		WRITE( "flagFireRule", V( "flagFireRuleChg" ) );
		WRITE( "flagWageOffer", V( "flagWageOfferChg" ) );
		WRITE( "flagIndexWage", V( "flagIndexWageChg" ) );
		WRITES( CONSECL0, "b", VS( CONSECL0, "bChg" ) );
		
		CYCLES( CONSECL0, cur, "Firm2" )
			WRITES( cur, "_postChg", 1 );		// set all firms as post-change
	}
	
	LOG( "\n Regime changed (t=%g)", T );
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
if ( VS( cur1, "F1min" ) < 1 || VS( cur2, "F2min" ) < 1 ||  VS( cur3, "B" ) < 1 ||
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
double f2trdChg = VS( cur2, "f2trdChg" );		// threshold cor post-change firms
double m1 = VS( cur1, "m1" );					// labor output factor
double mu1 = VS( cur1, "mu1" );					// mark-up in sector 1
double rT = VS( cur3, "rT" );					// prime rate target
double tauB = VS( cur3, "tauB" );				// minimum capital adequacy rate
double w0min = VS( cur4, "w0min" );				// absolute/initial minimum wage
int B = VS( cur3, "B" );						// number of banks
int F10 = VS( cur1, "F10" );					// initial firms in sector 1
int F20 = VS( cur2, "F20" );					// initial firms in sector 2
int F2max = VS( cur2, "F2max" );				// max firms in sector 2
int Ls0 = VS( cur4, "Ls0" );					// initial labor supply
int Tc = VS( cur4, "Tc" );						// work-contract term
int Tr = VS( cur4, "Tr" );						// work-life duration

double Btau0 = ( 1 + mu1 ) * INIPROD / 			// initial productivity in sec. 1
			   ( m1 * VS( cur2, "m2" ) * VS( cur2, "b" ) );
double c10 = INIWAGE / ( Btau0 * m1 );			// initial cost in sector 1
double c20 = INIWAGE / INIPROD;					// initial cost in sector 2
double p10 = ( 1 + mu1 ) * c10;					// initial price sector 1
double p20 = ( 1 + VS( cur2, "mu20" ) ) * c20;	// initial price sector 2
double Eavg0 = ( VS( cur2, "omega1" ) + VS( cur2, "omega2" ) + 
				VS( cur2, "omega3" ) ) / 3;		// initial competitiveness
double rBonds = rT * ( 1 - VS( cur3, "muBonds" ) );// initial interest on bonds
double G0 = V( "gG" ) * Ls0;					// initial public spending
double sV0 = ( V( "flagWorkerLBU" ) == 0 || V( "flagWorkerLBU" ) == 2 ) ?
			 INISKILL : VS( cur4, "sigma" );	// initial vintage skills
double wRes = VS( cur4, "phi" ) * INIWAGE;		// initial reservation wage

// reserve space for country-level non-initialized vectors
EXEC_EXT( countryE, firm2ptr, reserve, F2max );	// sector 2 firm objects
EXEC_EXT( countryE, firm2wgtd, reserve, F2max );
EXEC_EXT( countryE, bankPtr, reserve, B );		// bank objects
EXEC_EXT( countryE, bankWgtd, reserve, B );

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
WRITELS( cur1, "sT1min", INISKILL, -1 );
WRITELS( cur1, "w1avg", INIWAGE, -1 );
WRITELS( cur2, "CPI", p20, -1 );
WRITELS( cur2, "Eavg", Eavg0, -1 );
WRITELS( cur2, "F2", F20, -1 );
WRITELS( cur2, "c2", c20, -1 );
WRITELS( cur2, "oldVint", 1, -1 );
WRITELS( cur2, "w2avg", INIWAGE, -1 );
WRITELS( cur2, "w2oAvg", INIWAGE, -1 );
WRITELS( cur3, "r", rT, -1 ); 
WRITELS( cur3, "rBonds", rBonds, -1 );
WRITELS( cur4, "Ls", Ls0, -1 );
WRITELS( cur4, "sAvg", sV0, -1 );
WRITELS( cur4, "sTavg", INISKILL, -1 );
WRITELS( cur4, "sTmin", INISKILL, -1 );
WRITELS( cur4, "wAvg", INIWAGE, -1 );
WRITELS( cur4, "wAvgEmp", INIWAGE, -1 );
WRITELS( cur4, "wCent", INIWAGE, -1 );
WRITELS( cur4, "wMinPol", w0min, -1 );
WRITES( cur2, "f2critChg", 0 );					// critical threshold not met

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
	
	cur5 = SEARCHS( cur, "Cli1" );				// remove empty client instances
	cur6 = SEARCHS( cur, "Cli2" );
	DELETE( cur5 );
	DELETE( cur6 );
	
	++k;
}

CYCLES( cur3, cur, "Bank" )
	WRITES( cur, "_fD", VS( cur, "_fD" ) / i );	// adjust desired market shares

VS( cur3, "banksMaps" );						// update the mapping vectors

// create workers' objects and set initial values
k = 1;											// initial worker ID
ADDNOBJLS( cur4, "Worker", ceil( Ls0 / Lscale ) - 1, 0 );// recalculate in t
CYCLES( cur4, cur, "Worker" )
{
	v[8] = uniform_int( 1, Tr );				// draw worker age in [1, Tr]
	
	ADDHOOKS( cur, WORKERHK );					// add worker hooks
	
	WRITES( cur, "_ID", k );
	WRITES( cur, "_Tc", Tc );
	WRITES( cur, "_age", v[8] );
	WRITES( cur, "_wRes", wRes );
	WRITES( cur, "_employed", 0 );
	WRITELS( cur, "_sT", INISKILL, -1 );
	WRITELS( cur, "_sV", sV0, -1 );
	
	++k;
}

// create firms' objects and set initial values
cur = SEARCHS( cur1, "Firm1" );					// remove empty firm instance
DELETE( cur );
cur = SEARCHS( cur1, "Wrk1" );					// remove empty worker instance
DELETE( cur );
cur = SEARCHS( cur2, "Firm2" );					// remove empty firm instance
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
	
	CYCLES( cur, cur5, "Cli1" )
	{
		v[3] += VLS( SHOOKS( cur5 ), "_Deb1", 1 );// firm debt
		v[4] += VLS( SHOOKS( cur5 ), "_NW1", 1 );// firm net wealth
	}

	CYCLES( cur, cur6, "Cli2" )
	{
		v[3] += VLS( SHOOKS( cur6 ), "_Deb2", 1 );// firm debt
		v[4] += VLS( SHOOKS( cur6 ), "_NW2", 1 );// firm net wealth
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
