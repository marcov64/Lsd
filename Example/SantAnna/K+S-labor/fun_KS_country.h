/******************************************************************************

	COUNTRY OBJECT EQUATIONS
	------------------------

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
// wages and unemployment bonuses this period and past period bonuses+dividends
v[0] = VS( LABSUPL0, "W" ) + V( "G" ) + VLS( LABSUPL0, "Bon", 1 ) -
	   VS( LABSUPL0, "TaxW" ) + VL( "Div", 1 ) - V( "TaxDiv" );

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

v[0] += VS( LABSUPL0, "Gtrain" );				// worker training cost

if ( i == 1 )
	v[0] += ( 1 + V( "gG" ) ) * ( CURRENT - VLS( LABSUPL0, "Gtrain", 1 ) );

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
		VLS( FINSECL0, "rRes", 1 ) * VLS( FINSECL0, "DepoG", 1 ) )


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
RESULT( VS( CAPSECL0, "Div1" ) + VS( CONSECL0, "Div2" ) + VS( FINSECL0, "DivB" ) )


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
RESULT( VS( CAPSECL0, "Tax1" ) + VS( CONSECL0, "Tax2" ) + VS( FINSECL0, "TaxB" ) +
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

VS( FINSECL0, "cScores" );						// set the credit pecking order

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
int flagWorkerLBU = V( "flagWorkerLBU" );		// worker learning-by-use mode

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
double Eavg0 = ( VS( cur2, "omega1" ) + VS( cur2, "omega2" ) +
				 VS( cur2, "omega3" ) ) / 3;	// initial competitiveness
double rBonds = rT * ( 1 - VS( cur3, "muBonds" ) );// initial interest on bonds
double rD = rT * ( 1 - VS( cur3, "muD" ) );		// initial interest on deposits
double rDeb = rT * ( 1 + VS( cur3, "muDeb" ) );	// initial interest on debt
double rRes = rT * ( 1 - VS( cur3, "muRes" ) );	// initial interest on reserves
double G0 = V( "gG" ) * Ls0;					// initial public spending
double sV0 = ( flagWorkerLBU == 0 || flagWorkerLBU == 2 ) ?
			 INISKILL : VS( cur4, "sigma" );	// initial vintage skills
double wRes = phi * INIWAGE;					// initial reservation wage

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
WRITEL( "G", G0, -1 );
WRITES( cur1, "pK0", p10 );
WRITES( cur2, "pC0", p20 );
WRITELS( cur1, "A1", Btau0, -1 );
WRITELS( cur1, "F1", F10, -1 );
WRITELS( cur1, "PPI", p10, -1 );
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
WRITELS( cur3, "rD", rD, -1 );
WRITELS( cur3, "rDeb", rDeb, -1 );
WRITELS( cur3, "rRes", rRes, -1 );
WRITELS( cur4, "Ls", Ls0, -1 );
WRITELS( cur4, "sAvg", sV0, -1 );
WRITELS( cur4, "sTavg", INISKILL, -1 );
WRITELS( cur4, "sTmin", INISKILL, -1 );
WRITELS( cur4, "wAvg", INIWAGE, -1 );
WRITELS( cur4, "wCent", INIWAGE, -1 );
WRITELS( cur4, "wMinPol", w0min, -1 );

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

// create workers' objects and set initial values
k = 1;											// initial worker ID
ADDNOBJLS( cur4, "Worker", ceil( Ls0 / Lscale ) - 1, 0 );// recalculate in t
CYCLES( cur4, cur, "Worker" )
{
	v[1] = uniform_int( 1, Tr );				// draw worker age in [1, Tr]

	ADDHOOKS( cur, WORKERHK );					// add worker hooks

	WRITES( cur, "_ID", k );
	WRITES( cur, "_Tc", Tc );
	WRITES( cur, "_age", v[1] );
	WRITES( cur, "_wRes", wRes );
	WRITES( cur, "_employed", 0 );
	WRITELS( cur, "_sT", INISKILL, -1 );
	WRITELS( cur, "_sV", sV0, -1 );

	++k;
}

// create firms' objects and set initial values
DELETE( SEARCHS( cur1, "Firm1" ) );				// remove empty instances
DELETE( SEARCHS( cur2, "Firm2" ) );
DELETE( SEARCHS( cur1, "Wrk1" ) );

v[1] = entry_firm1( var, cur1, F10, true );		// add capital-good firms
INIT_TSEARCHTS( cur1, "Firm1", F10 );			// prepare turbo search indexing

v[1] += entry_firm2( var, cur2, F20, true );	// add consumer-good firms
VS( cur2, "firm2maps" );						// update the mapping vectors

WRITEL( "Eq", v[1], -1 );						// save existing equity

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
