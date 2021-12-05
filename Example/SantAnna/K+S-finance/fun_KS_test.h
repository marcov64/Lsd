/******************************************************************************

	TEST EQUATIONS
	--------------

	Equations that are not required for the model to run but can be used for
	testing, in-depth consistency analysis, and debugging.

 ******************************************************************************/

#define TEST1FILE "firms1.csv"					// names of test files on disk
#define TEST2FILE "firms2.csv"

#define SFCTHRD 1e-4							// threshold for SFC detection
#define TOL	0.1									// general error tolerance

/*========================= COUNTRY-LEVEL TESTS ==============================*/

EQUATION( "testSFC" )
/*
Test the balances of the balance-sheet and the transaction-flow matrices
Based on Nikiforos & Zezza 2017
*/

VS( PARENT, "entryExit" );						// ensure period is settled

// matrices data - capital-good sector
double Deb1 = VS( CAPSECL1, "Deb1" );			// bank debt/loans
double Div1_1 = VLS( CAPSECL1, "Div1", 1 );		// dividends
double Eq1 = VS( CAPSECL1, "Eq1" );				// equity
double NW1 = VS( CAPSECL1, "NW1" );				// bank deposits
double S1 = VS( CAPSECL1, "S1" );				// sales
double Tax1 = VS( CAPSECL1, "Tax1" );			// taxes
double W1 = VS( CAPSECL1, "W1" );				// wages
double cEntry1_1 = VLS( CAPSECL1, "cEntry1", 1 );// new equity
double cExit1_1 = VLS( CAPSECL1, "cExit1", 1 );	// liquidation payouts
double dDeb1 = Deb1 - VLS( CAPSECL1, "Deb1", 1 );// change in debt/loans
double dNW1 = NW1 - VLS( CAPSECL1, "NW1", 1 );	// change in deposits
double i1 = VS( CAPSECL1, "i1" );				// interest paid on debt
double iD1 = VS( CAPSECL1, "iD1" );				// interest got on deposits
double netPi1 = VS( CAPSECL1, "Pi1" ) - Tax1;	// net profits

// matrices data - consumption-good sector
double Deb2 = VS( CONSECL1, "Deb2" );			// bank debt/loans
double Div2_1 = VLS( CONSECL1, "Div2", 1 );		// dividends
double Eq2 = VS( CONSECL1, "Eq2" );				// equity
double Inom = VS( CONSECL1, "Inom" );			// investment (nominal terms)
double Knom = VS( CONSECL1, "Knom" );			// capital (nominal terms)
double NW2 = VS( CONSECL1, "NW2" );				// bank deposits
double S2 = VS( CONSECL1, "S2" );				// sales
double Tax2 = VS( CONSECL1, "Tax2" );			// taxes
double W2 = VS( CONSECL1, "W2" );				// wages
double cEntry2_1 = VLS( CONSECL1, "cEntry2", 1 );// new equity
double cExit2_1 = VLS( CONSECL1, "cExit2", 1 );	// liquidation payouts
double dDeb2 = Deb2 - VLS( CONSECL1, "Deb2", 1 );// change in debt/loans
double dNW2 = NW2 - VLS( CONSECL1, "NW2", 1 );	// change in deposits
double i2 = VS( CONSECL1, "i2" );				// interest paid on debt
double iD2 = VS( CONSECL1, "iD2" );				// interest got on deposits
double netPi2 = VS( CONSECL1, "Pi2" ) - Tax2;	// net profits

// matrices data - financial sector
double BadDeb_1 = VLS( FINSECL1, "BadDeb", 1 );	// bank bad debt
double BadDeb1_1 = VLS( FINSECL1, "BadDeb1", 1 );// bank bad debt in sector 1
double BadDeb2_1 = VLS( FINSECL1, "BadDeb2", 1 );// bank bad debt in sector 2
double BondsB = VS( FINSECL1, "BondsB" );		// bank gov. bond stock
double BondsB_1 = VLS( FINSECL1, "BondsB", 1 );
double BondsCB = VS( FINSECL1, "BondsCB" );		// central bank gov. bond stock
double BondsCB_1 = VLS( FINSECL1, "BondsCB", 1 );
double Depo = VS( FINSECL1, "Depo" );			// bank deposits
double Depo_1 = VLS( FINSECL1, "Depo", 1 );
double DepoG = VS( FINSECL1, "DepoG" );			// government deposits at c.b.
double DepoG_1 = VLS( FINSECL1, "DepoG", 1 );
double DivB_1 = VLS( FINSECL1, "DivB", 1 );		// dividends
double ExRes = VS( FINSECL1, "ExRes" );			// bank excess reserves
double ExRes_1 = VLS( FINSECL1, "ExRes", 1 );
double Gbail = VS( FINSECL1, "Gbail" );			// government bail-out funds
double Gbail_1 = VLS( FINSECL1, "Gbail", 1 );
double Loans = VS( FINSECL1, "Loans" );			// bank loans
double LoansCB = VS( FINSECL1, "LoansCB" );		// central liquidity loans
double LoansCB_1 = VLS( FINSECL1, "LoansCB", 1 );
double MB = Loans - Depo;						// credit-money supply
double PiCB = VS( FINSECL1, "PiCB" );			// central bank oper. result
double Res = VS( FINSECL1, "Res" );				// bank reserves at central bank
double Res_1 = VLS( FINSECL1, "Res", 1 );
double TaxB = VS( FINSECL1, "TaxB" );			// taxes
double dBondsB = BondsB - BondsB_1;				// change in bond stock (banks)
double dBondsCB = BondsCB - BondsCB_1;			// change in bonds (c.b.)
double dDepo = Depo - Depo_1;					// change in deposits
double dDepoG = DepoG - DepoG_1;				// change in gov. deposits c.b.
double dExRes = ExRes - ExRes_1;				// change in excess reserves
double dLoans = Loans - VLS( FINSECL1, "Loans", 1 );// change in loans
double dLoansCB = LoansCB - LoansCB_1;			// change in c.b. loans
double dMB = dLoans - dDepo;					// change in monetary base
double dRes = Res - Res_1;						// change in reserves at c.b.
double iB = VS( FINSECL1, "iB" );				// interest received on loans
double iDb = VS( FINSECL1, "iDb" );				// interest paid on deposits
double netPiB = VS( FINSECL1, "PiB" ) - TaxB;	// net profits
double r_1 = VLS( FINSECL1, "r", 1 );			// prime interest rate
double rBonds_1 = VLS( FINSECL1, "rBonds", 1 );	// interest rate on gov. bonds
double rD_1 = VLS( FINSECL1, "rD", 1 );			// interest rate on deposits
double rRes_1 = VLS( FINSECL1, "rRes", 1 );		// interest rate on c.b. reserves

// matrices data - workers/households
double Div_1 = VLS( PARENT, "Div", 1 );			// total dividends
double Eq = VS( PARENT, "Eq" );					// total equity
double SavAcc = VS( PARENT, "SavAcc" );			// bank deposits (= acc. savings)
double SavAcc_1 = VLS( PARENT, "SavAcc", 1 );
double TaxDiv = VS( PARENT, "TaxDiv" );			// taxes on dividends
double TaxW = VS( LABSUPL1, "TaxW" );			// taxes on wages
double W = VS( LABSUPL1, "W" );					// total wages
double cEntry_1 = VLS( PARENT, "cEntry", 1 );	// total new equity
double cExit_1 = VLS( PARENT, "cExit", 1 );		// total liquidation payouts
double dSavAcc = SavAcc - SavAcc_1;				// change in deposits

// matrices data matrices - country/government
double C = VS( PARENT, "C" );					// consumption
double Deb = VS( PARENT, "Deb" );				// government debt (total bonds)
double Deb_1 = VLS( PARENT, "Deb", 1 );
double G = VS( PARENT, "G" );					// government expenditure
double Tax = VS( PARENT, "Tax" );				// government income (taxes)
double dDeb = Deb - Deb_1;						// change in debt (bond) stock

// balance-sheet row sums (accounting identities)
double Equities = + Eq - Eq1 - Eq2;
double Deposits = + SavAcc + NW1 + NW2 - Depo;
double BankLoans = - Deb1 - Deb2 + Loans;
double CreditMoney = + MB - MB;
double Reserves = + Res - Res;
double ExReserves = + ExRes - ExRes;
double LiqFacilities = - LoansCB + LoansCB;
double GovBonds = + BondsB + BondsCB - Deb;
double GovDepo = - DepoG + DepoG;

v[1] = abs( Equities ) + abs( Deposits ) + abs( BankLoans ) +
	   abs( CreditMoney ) + abs( Reserves ) + abs( ExReserves ) +
	   abs( LiqFacilities ) + abs( GovBonds ) + abs( GovDepo );

// balance-sheet column sums (balance/net worth)
double Bal = + SavAcc + Eq;
double Bal1 = + NW1 - Deb1 - Eq1;
double Bal2 = + Knom + NW2 - Deb2 - Eq2;
double BalB = - Depo + Loans + MB + Res + ExRes - LoansCB + BondsB;
double BalCB = - MB - Res - ExRes + LoansCB + BondsCB - DepoG;
double BalG = - Deb + DepoG;

v[2] = - Bal - Bal1 - Bal2 - BalB - BalCB - BalG + Knom;

// transaction-flow matrix row sums (accounting identities)
double Consumption = - C + S2;
double Investment = + S1 - Inom;
double GovExpend = + G - G;
double Wages = + W - W1 - W2;
double Taxes = - TaxW - TaxDiv - Tax1 - Tax2 - TaxB + Tax;
double Profits = - netPi1 + netPi1 - netPi2 + netPi2 - netPiB + netPiB;
double Dividends = + Div_1 - Div1_1 - Div2_1 - DivB_1;
double NewEquity = - cEntry_1 + cEntry1_1 + cEntry2_1;
double LiqEquity = + cExit_1 - cExit1_1 - cExit2_1;
double BadDebt = + BadDeb1_1 + BadDeb2_1 - BadDeb_1;
double Bailout = + Gbail - Gbail + Gbail_1 - Gbail_1;
double CBprofit = - PiCB + PiCB;

double DepoIntrst = + rD_1 * SavAcc_1 + iD1 + iD2 - iDb;
double LoanIntrst = - i1 - i2 + iB;
double ResIntrst = + rRes_1 * Res_1 - rRes_1 * Res_1;
double LiqFacIntrst = - r_1 * LoansCB_1 + r_1 * LoansCB_1;
double BondIntrst = + rBonds_1 * BondsB_1 + rBonds_1 * BondsCB_1
					- rBonds_1 * Deb_1;
double GovDepoIntrst = - rRes_1 * DepoG_1 + rRes_1 * DepoG_1;

double DepoChg = - dSavAcc - dNW1 - dNW2 + dDepo;
double LoanChg = + dDeb1 + dDeb2 - dLoans;
double MBchg = + dMB - dMB;
double ResChg = - dRes + dRes;
double ExResChg = - dExRes + dExRes;
double LiqFacChg = + dLoansCB - dLoansCB;
double BondChg = - dBondsB - dBondsCB + dDeb;
double GovDepoChg = + dDepoG - dDepoG;

v[3] = abs( Consumption ) + abs( Investment ) + abs( GovExpend ) + abs( Wages ) +
	   abs( Taxes ) + abs( Profits ) + abs( Dividends ) + abs( NewEquity ) +
	   abs( LiqEquity ) + abs( BadDebt ) + abs( Bailout ) + abs( CBprofit ) +
	   abs( DepoIntrst ) + abs( LoanIntrst ) + abs( ResIntrst ) +
	   abs( LiqFacIntrst ) + abs( BondIntrst ) + abs( GovDepoIntrst ) +
	   abs( DepoChg ) + abs( LoanChg ) + abs( MBchg ) + abs( ResChg ) +
	   abs( ExResChg ) + abs( LiqFacChg ) + abs( BondChg ) + abs( GovDepoChg );

// capital transaction sub-matrix column sums (net lending)
double workersNL = - C + G + W - TaxW - TaxDiv + Div_1 - cEntry_1
				   + cExit_1 + rD_1 * SavAcc_1;
double firms1nl = + netPi1 - Div1_1 + cEntry1_1 - cExit1_1 + BadDeb1_1;
double firms2nl = - Inom + netPi2 - Div2_1 + cEntry2_1 - cExit2_1
				  + BadDeb2_1;
double banksNL = + netPiB - DivB_1 + Gbail;
double cBankNL = - PiCB - Gbail + Gbail_1 - rRes_1 * Res_1 + r_1 * LoansCB_1
				 + rBonds_1 * BondsCB_1 - rRes_1 * DepoG_1;
double govtNL = - G + Tax + PiCB - Gbail_1 - rBonds_1 * Deb_1
				+ rRes_1 * DepoG_1;

v[4] = workersNL + firms1nl + firms2nl + banksNL + cBankNL + govtNL;

// transaction-flow matrix column sums (net flows/changes in stocks)
double workers = + workersNL - dSavAcc;
double firms1c = + S1 - W1 - Tax1 - netPi1 + iD1 - i1;
double firms1k = + firms1nl - dNW1 + dDeb1;
double firms2c = + S2 - W2 - Tax2 - netPi2 + iD2 - i2;
double firms2k = + firms2nl - dNW2 + dDeb2;
double banksC = - TaxB - netPiB - BadDeb_1 - iDb + iB + rRes_1 * Res_1
				- r_1 * LoansCB_1 + rBonds_1 * BondsB_1;
double banksK = + banksNL + dDepo - dLoans + dMB - dRes - dExRes + dLoansCB
				- dBondsB;
double cBank = + cBankNL - dMB + dRes + dExRes - dLoansCB - dBondsCB + dDepoG;
double govt = + govtNL + dDeb - dDepoG;

v[5] = abs( workers ) + abs( firms1c ) + abs( firms1k ) + abs( firms2c ) +
	   abs( firms2k ) + abs( banksC ) + abs( banksK ) + abs( cBank ) + abs( govt );

v[6] = VS( PARENT, "GDPnom" );					// reference for % of GDP results

v[1] = ROUND( v[1] / v[6], 0, SFCTHRD );		// round close to zero values
v[2] = ROUND( v[2] / v[6], 0, SFCTHRD );
v[3] = ROUND( v[3] / v[6], 0, SFCTHRD );
v[4] = ROUND( v[4] / v[6], 0, SFCTHRD );
v[5] = ROUND( v[5] / v[6], 0, SFCTHRD );

// don't show messages if not called from 'testXXXX' equations
if ( CALLER != NULL && strcmp( NAMES( CALLER ), "Stats" ) == 0 )
{
	LOG( "\n   @ SFCbRows=%.4g SFCbCols=%.4g SFCtRows=%.4g SFCtCols=%.4g SFCnl=%.4g",
		 v[1], v[2], v[3], v[5], v[4] );

	if ( v[1] != 0 )
		 LOG( " SFC-BAL-ROW-NOT-ZERO" );

	if ( v[2] != 0 )
		 LOG( " SFC-BAL-COL-NOT-ZERO" );

	if ( v[3] != 0 )
		 LOG( " SFC-TRANS-ROW-NOT-ZERO" );

	if ( v[4] != 0 )
		 LOG( " SFC-NET-LEND-NOT-ZERO" );

	if ( v[5] != 0 )
		 LOG( " SFC-TRANS-COL-NOT-ZERO" );
}

RESULT( v[5] / 2 )


EQUATION( "testCountry" )
/*
Print detailed statistics of country macro (!=0 if error is found)
Set the time range in 'testCtIni' and 'testCtEnd'
*/

if ( T == 1 )
	PLOG( "\n Optional statistics being computed in object 'Stats'" );

v[1] = V( "testCtIni" );
v[2] = V( "testCtEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

static double iniGDP, iniA, iniDeb, iniSavAcc;
static int errorsTot = 0;						// all runs error accumulator

if ( T == v[1] )
	LOG( "\n @@@ TESTING OF COUNTRY MACRO STARTED" );

int errors = 0;									// error counter

double A = VS( PARENT, "A" );
double C = VS( PARENT, "C" );
double Creal = VS( PARENT, "Creal" );
double Cd = VS( PARENT, "Cd" );
double Deb = VS( PARENT, "Deb" );
double Def = VS( PARENT, "Def" );
double Div = VS( PARENT, "Div" );
double Eq = VS( PARENT, "Eq" );
double G = VS( PARENT, "G" );
double GDPreal = VS( PARENT, "GDPreal" );
double GDPnom = VS( PARENT, "GDPnom" );
double Sav = VS( PARENT, "Sav" );
double SavAcc = VS( PARENT, "SavAcc" );
double Tax = VS( PARENT, "Tax" );
double TaxDiv = VS( PARENT, "TaxDiv" );
double cEntry = VS( PARENT, "cEntry" );
double cExit = VS( PARENT, "cExit" );
double dGDP = VS( PARENT, "dGDP" );
double dAb = VS( PARENT, "dAb" );
double entryExit = VS( PARENT, "entryExit" );

double L1 = VS( CAPSECL1, "L1" );
double PPI = VS( CAPSECL1, "PPI" );
double Q1e = VS( CAPSECL1, "Q1e" );
double pK0 = VS( CAPSECL1, "pK0" );

double CPI = VS( CONSECL1, "CPI" );
double D2d = VS( CONSECL1, "D2d" );
double Ireal = VS( CONSECL1, "Ireal" );
double Inom = VS( CONSECL1, "Inom" );
double L2 = VS( CONSECL1, "L2" );
double N = VS( CONSECL1, "N" );
double Q2e = VS( CONSECL1, "Q2e" );
double dNnom = VS( CONSECL1, "dNnom" );
double pC0 = VS( CONSECL1, "pC0" );

double GDI = VS( MACSTAL1, "GDI" );
double dA = VS( MACSTAL1, "dA" );

double dN = VS( SECSTAL1, "dN" );

double nonNeg[ ] = { Div, Eq, G, Ireal, Inom, N, Sav, SavAcc, Tax, TaxDiv,
					 cEntry, cExit };
double posit[ ] = { A, C, Creal, Cd, D2d, GDPreal, GDPnom, GDI };
double finite[ ] = { dA, dAb, dGDP, dN, dNnom };

dblVecT all ( nonNeg, END_ARR( nonNeg ) );
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// first period actions
if ( T == v[1] )
{
	iniGDP = GDPreal;
	iniA = A;
	iniDeb = Deb;
	iniSavAcc = SavAcc;
}

// national accounting
LOG( "\n  @@ (t=%g) dA=%.2g dGDP=%.2g C%%=%.2g I%%=%.2g G%%=%.2g dN%%=%.2g Sav%%=%.2g", T,
	  dA, dGDP, C / GDPnom, Inom / GDPnom, G / GDPnom, dNnom / GDPnom, Sav / GDPnom );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( auto itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

check_error( dA < - 2 * TOL, "HIGH-PRODUCTIVITY-DROP", 0, & errors );

check_error( A < min( Q1e * pK0 / L1, Q2e * pC0 / L2 ) ||
			 A > max( Q1e * pK0 / L1, Q2e * pC0 / L2 ),
			 "INCONSISTENT-PRODUCTIVITY", 0, & errors );

check_error( ceil( Cd ) < floor( C ), "INCONSISTENT-CONSUMPTION", 0, & errors );

check_error( GDPnom > ( 1 + 2 * TOL ) * ( Q1e * PPI + Q2e * CPI ) ||
			 GDPnom < ( 1 - 2 * TOL ) * ( Q1e * PPI + Q2e * CPI ) ||
			 GDPnom > ( 1 + TOL ) * ( C + Inom + dNnom ) ||
			 GDPnom < ( 1 - TOL ) * ( C + Inom + dNnom ),
			 "INCONSISTENT-GDP", 0, & errors );

check_error( GDPnom > ( 1 + 2 * TOL ) * GDI ||
			 GDPnom < ( 1 - 2 * TOL ) * GDI,
			 "GDI-GAP", 0, & errors );

check_error( Sav / GDPnom > 5 * TOL, "HIGH-SAVINGS", 0, & errors );

// forced savings, public debt, firms equity and dynamics
LOG( "\n   @ Tax%%=%.2g Def%%=%.2g Deb%%=%.2g cEntry%%=%.2g cExit%%=%.2g entryExit=%g",
	  Tax / GDPnom, Def / GDPnom, Deb / GDPnom, cEntry / GDPnom, cExit / GDPnom,
	  entryExit );

check_error( cEntry / GDPnom > 2 * TOL, "HIGH-EQUITY", 0, & errors );

check_error( Deb / GDPnom > 100 * TOL, "EXPLOSIVE-DEBT", 0, & errors );

// SFC check
RECALC( "testSFC" );
if ( V( "testSFC" ) > TOL )
	++errors;

errorsTot += errors;

// last period actions
if ( T == v[2] )
{
	v[4] = ( log( GDPreal + 1 ) - log( iniGDP + 1 ) ) / v[3];
	v[5] = ( log( A ) - log( iniA ) ) / v[3];
	v[6] = ( log( Deb + 1 ) - log( iniDeb + 1 ) ) / v[3];
	v[7] = ( log( SavAcc + 1 ) - log( iniSavAcc + 1 ) ) / v[3];

	LOG( "\n   @ GDPgwth=%.3g Agwth=%.3g DebGwth=%.3g SavGwth=%.3g",
		 v[4], v[5], v[6], v[7] );

	check_error( v[4] < TOL / 20 || v[5] < TOL / 20,
				 "LOW-GROWTH", 0, & errorsTot );

	check_error( v[6] > ( 1 + TOL ) * v[4],
				 "EXPLOSIVE-DEBT-GROWTH", 0, & errorsTot );

	check_error( v[7] > ( 1 + TOL ) * v[4],
				 "EXPLOSIVE-SAVINGS-GROWTH", 0, & errorsTot );

	LOG( "\n @@@ TESTING OF COUNTRY MACRO FINISHED (%d)", errorsTot );
}

RESULT( errors )


/*========================== SECTOR-LEVEL TESTS ==============================*/

EQUATION( "testFin" )
/*
Print detailed statistics of financial sector (!=0 if error is found)
Set the time range in 'testFtIni' and 'testFtEnd'
*/

v[1] = V( "testFtIni" );
v[2] = V( "testFtEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

static int errorsTot = 0;						// all runs error accumulator

if ( T == v[1] )
	LOG( "\n $$$$ TESTING OF FINANCIAL SECTOR STARTED" );

// scan banks for severe problems				// interest rates
double F1 = VS( CAPSECL1, "F1" );
double F2 = VS( CONSECL1, "F2" );
double r_1 = VLS( FINSECL1, "r", 1 );
double rBonds_1 = VLS( FINSECL1, "rBonds", 1 );
double rRes_1 = VLS( FINSECL1, "rRes", 1 );

objVecT TCerr, sfcCerr, sfcKerr;				// vector to save TC error banks
int errors = 0;									// error counter
k = v[4] = v[5] = v[6] = v[7] = v[8] = v[9] = 0;// accumulators
v[10] = v[11] = v[12] = v[13] = 0;

CYCLES( FINSECL1, cur, "Bank" )
{
	v[4] += COUNTS( cur, "Cli1" );
	v[5] += COUNTS( cur, "Cli2" );
	v[6] += VS( cur, "_Cl" );
	v[7] += VS( cur, "_fB" );
	v[8] += VS( cur, "_BondsB" );
	v[9] += VS( cur, "_Depo" );
	v[10] += VS( cur, "_Loans" );
	v[11] += VS( cur, "_LoansCB" );
	v[12] += VS( cur, "_Res" );
	v[13] += VS( cur, "_ExRes" );

	if ( VS( cur, "_TC1free" ) + VS( cur, "_TC2free" ) >
		 ( 1 + TOL / 10 ) * VS( cur, "_TC" ) )
		TCerr.push_back( cur );

	if ( abs( - VS( cur, "_TaxB" ) - ( VS( cur, "_PiB" ) - VS( cur, "_TaxB" ) )
			  - ( VLS( cur, "_BadDeb1", 1 ) + VLS( cur, "_BadDeb2", 1 ) )
			  - VS( cur, "_iDb" ) + VS( cur, "_iB" )
			  + rRes_1 * VLS( cur, "_Res", 1 ) - r_1 * VLS( cur, "_LoansCB", 1 )
			  + rBonds_1 * VLS( cur, "_BondsB", 1 ) ) > TOL )
		 sfcCerr.push_back( cur );

	if ( abs( + VS( cur, "_Gbail" ) + ( VS( cur, "_PiB" ) - VS( cur, "_TaxB" ) )
			  - VLS( cur, "_DivB", 1 )
			  + ( VS( cur, "_Depo" ) - VLS( cur, "_Depo", 1 ) )
			  - ( VS( cur, "_Loans" ) - VLS( cur, "_Loans", 1 ) )
			  + ( ( VS( cur, "_Loans" ) - VLS( cur, "_Loans", 1 ) )
			  - ( VS( cur, "_Depo" ) - VLS( cur, "_Depo", 1 ) ) )
			  - ( VS( cur, "_Res" ) - VLS( cur, "_Res", 1 ) )
			  - ( VS( cur, "_ExRes" ) - VLS( cur, "_ExRes", 1 ) )
			  + ( VS( cur, "_LoansCB" ) - VLS( cur, "_LoansCB", 1 ) )
			  - ( VS( cur, "_BondsB" ) - VLS( cur, "_BondsB", 1 ) ) ) > TOL )
		 sfcKerr.push_back( cur );

	++k;
}

double BD = VS( FINSECL1, "BD" );
double BS = VS( FINSECL1, "BS" );
double BadDeb = VS( FINSECL1, "BadDeb" );
double BadDeb1 = VS( FINSECL1, "BadDeb1" );
double BadDeb2 = VS( FINSECL1, "BadDeb2" );
double BondsB = VS( FINSECL1, "BondsB" );
double BondsCB = VS( FINSECL1, "BondsCB" );
double Cl = VS( FINSECL1, "Cl" );
double Depo = VS( FINSECL1, "Depo" );
double DepoG = VS( FINSECL1, "DepoG" );
double DivB = VS( FINSECL1, "DivB" );
double ExRes = VS( FINSECL1, "ExRes" );
double Gbail = VS( FINSECL1, "Gbail" );
double Loans = VS( FINSECL1, "Loans" );
double LoansCB = VS( FINSECL1, "LoansCB" );
double NWb = VS( FINSECL1, "NWb" );
double PiB = VS( FINSECL1, "PiB" );
double PiCB = VS( FINSECL1, "PiCB" );
double Res = VS( FINSECL1, "Res" );
double TaxB = VS( FINSECL1, "TaxB" );
double iB = VS( FINSECL1, "iB" );
double iDb = VS( FINSECL1, "iDb" );
double r = VS( FINSECL1, "r" );
double rBonds = VS( FINSECL1, "rBonds" );
double rD = VS( FINSECL1, "rD" );
double rDeb = VS( FINSECL1, "rDeb" );
double rRes = VS( FINSECL1, "rRes" );

double thetaBonds = VS( FINSECL1, "thetaBonds" );

double CD = VS( MACSTAL1, "CD" );
double CDc = VS( MACSTAL1, "CDc" );
double CS = VS( MACSTAL1, "CS" );
double TC = VS( MACSTAL1, "TC" );
double Bda = VS( SECSTAL1, "Bda" );
double Bfail = VS( SECSTAL1, "Bfail" );
double HHb = VS( SECSTAL1, "HHb" );
double HPb = VS( SECSTAL1, "HPb" );

double Deb = VS( PARENT, "Deb" );
double Def = VS( PARENT, "Def" );
double Def_1 = VLS( PARENT, "Def", 1 );
double SavAcc = VS( PARENT, "SavAcc" );

double Deb1 = VS( CAPSECL1, "Deb1" );
double NW1 = VS( CAPSECL1, "NW1" );

double Deb2 = VS( CONSECL1, "Deb2" );
double NW2 = VS( CONSECL1, "NW2" );

double BS_1 = VLS( FINSECL1, "BS", 1 );
double BD_1 = VLS( FINSECL1, "BD", 1 );
double BadDeb_1 = VLS( FINSECL1, "BadDeb", 1 );
double BondsB_1 = VLS( FINSECL1, "BondsB", 1 );
double Depo_1 = VLS( FINSECL1, "Depo", 1 );
double DepoG_1 = VLS( FINSECL1, "DepoG", 1 );
double DivB_1 = VLS( FINSECL1, "DivB", 1 );
double ExRes_1 = VLS( FINSECL1, "ExRes", 1 );
double Loans_1 = VLS( FINSECL1, "Loans", 1 );
double LoansCB_1 = VLS( FINSECL1, "LoansCB", 1 );
double Res_1 = VLS( FINSECL1, "Res", 1 );

double nonNeg[ ] = { BS, BondsB, BondsCB, CD, CDc, CS, Deb1, Deb2, Depo, DepoG,
					 DivB, ExRes, Gbail, Loans, LoansCB, Res, TaxB, Bda, Bfail,
					 BadDeb, BadDeb1, BadDeb2, HHb, HPb, SavAcc, NW1, NW2, iB,
					 iDb, r, rD, rRes };
double posit[ ] = { Cl, rBonds, rDeb };
double finite[ ] = { BD, Deb, TC, PiB, PiCB, NW1, NW2 };

dblVecT all ( nonNeg, END_ARR( nonNeg ) );
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// interest rate structure
LOG( "\n  $$$ (t=%g) rD=%.2g rRes=%.2g rBonds=%.2g r=%.2g rDeb=%.2g",
	 T, rD, rRes, rBonds, r, rDeb );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( auto itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

check_error( rD > rRes || rD > rBonds || rRes > r || r > rDeb ||
			 rBonds - r > VS( FINSECL1, "rAdj" ),
			 "INCONSISTENT-INTEREST-STRUCTURE", 0, & errors );

// central bank
LOG( "\n   $$ Res=%.3g ExRes=%.3g LoansCB=%.3g BondsCB=%.3g DepoG=%.3g Gbail=%.3g",
	 Res, ExRes, LoansCB, BondsCB, DepoG, Gbail );

check_error( Res > Depo || round( Res ) != round( v[12] ) ||
			 round( ExRes ) != round( v[13] ),
			 "INCONSISTENT-RESERVES", 0, & errors );

check_error( round( LoansCB ) != round ( v[11] ),
			 "INCONSISTENT-CB-LOANS", 0, & errors );

check_error( round( BondsCB ) < round( BS - BD ),
			 "INCONSISTENT-CB-BONDS", 0, & errors );

// government bonds and debt
LOG( "\n   $$ BS=%.3g BD=%.3g BSnew=%.3g Def_1=%.3g Deb=%.3g",
	 BS, BD, BS - BS_1 + BD_1, Def_1, Deb );

check_error( BS - BS_1 > max( Def + ( BondsB + BondsCB ) / thetaBonds, 0 ) ||
			 floor( BD ) > BS + BondsCB || floor( BD ) > BondsB ||
			 round( BondsB ) != round( v[8] ) ||
			 ( BS - BS_1 > TOL && DepoG - DepoG_1 > TOL ),
			 "INCONSISTENT-BONDS", 0, & errors );

// bank customers and crisis/bail-outs
LOG( "\n   $$ #Bank=%d #Client1=%g #Client2=%g Bfail=%g",
	 k, v[4], v[5], Bfail );

check_error( F1 != v[4], "INCONSISTENT-CLIENT1", 0, & errors );

check_error( F2 != v[5], "INCONSISTENT-CLIENT2", 0, & errors );

check_error( v[7] < 1 - TOL / 10 || v[7] > 1 + TOL / 10,
			 "INCONSISTENT-SHARES", 0, & errors );

// bank assets and liabilities, credit dynamic
LOG( "\n   $$ Depo=%.3g Loans=%.3g CD=%.3g CS=%.3g CDc=%.3g",
	 Depo, Loans, CD, CS, CDc );

check_error( round( Depo ) != round( v[9] ) || round( Loans ) != round( v[10] ),
			 "INCONSISTENT-BANK-ACCOUNTS", 0, & errors );

// try to account for deposits from loans of entrant firms (very crude)
check_error( abs( NW1 + NW2 + SavAcc - Depo - Deb1 - Deb2 + Loans ) / Depo > TOL,
			 "LARGE-DEPO-LOANS-GAP", 0, & errors );

check_error( CS > CD || CDc > CD, "INCONSISTENT-FINANCE", 0, & errors );

check_error( abs( + Gbail + ( PiB - TaxB ) - DivB_1 + ( Depo - Depo_1 )
				  - ( Loans - Loans_1 )
				  + ( ( Loans - Loans_1 ) - ( Depo - Depo_1 ) )
				  - ( Res - Res_1 ) - ( ExRes - ExRes_1 )
				  + ( LoansCB - LoansCB_1 ) - ( BondsB - BondsB_1 ) ) > TOL,
				 "INCONSISTENT-SFC-CAPITAL", 0, & errors );

check_error( sfcKerr.size( ) > 0, "SFC-CAP-ERR-BANK", sfcKerr.size( ), & errors );

// banks cash-flow
LOG( "\n   $$ TC=%.2g BadDeb=%.2g iB=%.2g TaxB=%.2g PiB=%.2g NWb=%.2g",
	 TC, BadDeb1 + BadDeb2, iB, TaxB, PiB, NWb );

check_error( TC < -1, "NEGATIVE-TOTAL-CREDIT", 0, & errors );

check_error( PiB - TaxB > iB + rRes_1 * Res_1 + rBonds_1 * BondsB_1,
			 "INCONSISTENT-BANK-PROFIT", 0, & errors );

check_error( TCerr.size( ) > 0, "INCONSISTENT-TC-FREE", TCerr.size( ), & errors );

check_error( abs( - TaxB - ( PiB - TaxB ) - BadDeb_1 - iDb + iB + rRes_1 * Res_1
				  - r_1 * LoansCB_1 + rBonds_1 * BondsB_1 ) > TOL,
				 "INCONSISTENT-SFC-FLOW", 0, & errors );

check_error( sfcCerr.size( ) > 0, "SFC-FLOW-ERR-BANK", sfcCerr.size( ), & errors );

errorsTot += errors;

if ( T == v[2] )
	LOG( "\n $$$$ TESTING OF FINANCIAL SECTOR FINISHED (%d)", errorsTot );

RESULT( errors )


EQUATION( "testLabor" )
/*
Print detailed statistics of the labor supply (!=0 if error is found)
Set the time range in 'testLtIni' and 'testLtEnd'
*/

v[1] = V( "testLtIni" );
v[2] = V( "testLtEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

static int errorsTot = 0;						// all runs error accumulator

if ( T == v[1] )
	LOG( "\n +++ TESTING OF LABOR SUPPLY STARTED" );

int errors = 0;									// error counter

double L = VS( LABSUPL1, "L" );
double Ls = VS( LABSUPL1, "Ls" );
double TaxW = VS( LABSUPL1, "TaxW" );
double U = VS( LABSUPL1, "U" );
double W = VS( LABSUPL1, "W" );
double dUb = VS( LABSUPL1, "dUb" );
double w = VS( LABSUPL1, "w" );

double Vac = VS( LABSTAL1, "V" );

double C = VS( PARENT, "C" );
double G = VS( PARENT, "G" );
double Eq = VS( PARENT, "Eq" );
double SavAcc = VS( PARENT, "SavAcc" );

double Div_1 = VLS( PARENT, "Div", 1 );
double SavAcc_1 = VLS( PARENT, "SavAcc", 1 );
double cEntry_1 = VLS( PARENT, "cEntry", 1 );
double cExit_1 = VLS( PARENT, "cExit", 1 );
double rD_1 = VLS( FINSECL1, "rD", 1 );

double L1 = VS( CAPSECL1, "L1" );
double W1 = VS( CAPSECL1, "W1" );
double L2 = VS( CONSECL1, "L2" );
double W2 = VS( CONSECL1, "W2" );

double nonNeg[ ] = { TaxW, U, Vac, W, L1, W1, L2, W2 };
double posit[ ] = { L, Ls, w };
double finite[ ] = { dUb };

dblVecT all ( nonNeg, END_ARR( nonNeg ) );
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// summary
LOG( "\n  ++ (t=%g) Ls=%g L=%g V=%.2g U=%.2g w=%.2g",
	 T, Ls, L, Vac, U, w );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( auto itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

check_error( U > 1 || Vac > 1, "INCONSISTENT-LABOR-STATS", 0, & errors );

check_error( L1 + L2 != L || Ls < L, "INCONSISTENT-LABOR", 0, & errors );

check_error( W1 + W2 < ( 1 - TOL / 10 ) * L * w ||
			 W1 + W2 > ( 1 + TOL / 10 ) * L * w,
			 "INCONSISTENT-WAGES", 0, & errors );

// finance
LOG( "\n   + W=%.2g Div=%.2g cEntry=%.2g cExit=%.2g SavAcc=%.2g",
	 W, Div_1, cEntry_1, cExit_1, SavAcc );

check_error( abs( - C + G + W - TaxW + Div_1 - cEntry_1 + cExit_1
				  + rD_1 * SavAcc_1 - ( SavAcc - SavAcc_1 ) ) > TOL,
				 "INCONSISTENT-SFC-FLOW", 0, & errors );

errorsTot += errors;

if ( T == v[2] )
	LOG( "\n +++ TESTING OF LABOR SUPPLY FINISHED (%d)", errorsTot );

RESULT( errors )


EQUATION( "test1sec" )
/*
Print detailed statistics of capital-good sector (!=0 if error is found)
Set the time range in 'test1StIni' and 'test1StEnd'
*/

v[1] = V( "test1StIni" );
v[2] = V( "test1StEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

static int errorsTot = 0;						// all runs error accumulator

if ( T == v[1] )
	LOG( "\n ^^^ TESTING OF CAPITAL-GOOD SECTOR STARTED" );

// scan firms for severe problems
double F1 = VS( CAPSECL1, "F1" );

objVecT Aerr, RDerr, c1err, CliErr, sfcCerr,	// vectors to save error firms
		sfcKerr;
int errors = 0;									// error counter
k = v[4] = v[5] = v[6] = 0;						// accumulators
CYCLES( CAPSECL1, cur, "Firm1" )
{
	v[4] += VS( cur, "_f1" );
	v[5] += VS( cur, "_L1" );

	if ( VS( cur, "_t1ent" ) < T )				// ignore just entered firm
		v[6] += VS( cur, "_L1rd" );

	if ( VS( cur, "_Atau" ) <= TOL || VS( cur, "_Btau" ) <= TOL / 10 )
		Aerr.push_back( cur );

	if ( VS( cur, "_RD" ) < 0 )
		RDerr.push_back( cur );

	if ( VS( cur, "_c1" ) <= 0 || VS( cur, "_p1" ) <= 0 )
		c1err.push_back( cur );

	if ( VS( cur, "_t1ent" ) > T && VS( cur, "_HC" ) + VS( cur, "_NC" ) <= 0 )
		CliErr.push_back( cur );

	if ( abs( + VS( cur, "_S1" ) - VS( cur, "_W1" ) - VS( cur, "_Tax1" )
			  - ( VS( cur, "_Pi1" ) - VS( cur, "_Tax1" ) )
			  + VS( cur, "_iD1" ) - VS( cur, "_i1" ) ) > TOL )
		sfcCerr.push_back( cur );

	if ( abs( + ( VS( cur, "_Pi1" ) - VS( cur, "_Tax1" ) )
			  - VLS( cur, "_Div1", 1 )
			  - ( VS( cur, "_NW1" ) - VLS( cur, "_NW1", 1 ) )
			  + ( VS( cur, "_Deb1" ) - VLS( cur, "_Deb1", 1 ) )
			  + ( VS( cur, "_t1ent" ) == T ? VS( cur, "_Eq1" ) : 0 ) ) > TOL )
		sfcKerr.push_back( cur );

	++k;
}

double A1 = VS( CAPSECL1, "A1" );
double D1 = VS( CAPSECL1, "D1" );
double Deb1 = VS( CAPSECL1, "Deb1" );
double Div1 = VS( CAPSECL1, "Div1" );
double Eq1 = VS( CAPSECL1, "Eq1" );
double JO1 = VS( CAPSECL1, "JO1" );
double L1 = VS( CAPSECL1, "L1" );
double L1d = VS( CAPSECL1, "L1d" );
double L1dRD = VS( CAPSECL1, "L1dRD" );
double L1rd = VS( CAPSECL1, "L1rd" );
double NW1 = VS( CAPSECL1, "NW1" );
double PPI = VS( CAPSECL1, "PPI" );
double Pi1 = VS( CAPSECL1, "Pi1" );
double Q1 = VS( CAPSECL1, "Q1" );
double Q1e = VS( CAPSECL1, "Q1e" );
double S1 = VS( CAPSECL1, "S1" );
double Tax1 = VS( CAPSECL1, "Tax1" );
double W1 = VS( CAPSECL1, "W1" );
double cEntry1 = VS( CAPSECL1, "cEntry1" );
double cExit1 = VS( CAPSECL1, "cExit1" );
double entry1exit = VS( CAPSECL1, "entry1exit" );
double i1 = VS( CAPSECL1, "i1" );
double iD1 = VS( CAPSECL1, "iD1" );
double imi = VS( CAPSECL1, "imi" );
double inn = VS( CAPSECL1, "inn" );

double Deb1_1 = VLS( CAPSECL1, "Deb1", 1 );
double Div1_1 = VLS( CAPSECL1, "Div1", 1 );
double NW1_1 = VLS( CAPSECL1, "NW1", 1 );
double cEntry1_1 = VLS( CAPSECL1, "cEntry1", 1 );
double cExit1_1 = VLS( CAPSECL1, "cExit1", 1 );

double BadDeb1_1 = VLS( FINSECL1, "BadDeb1", 1 );
double CD1 = VS( SECSTAL1, "CD1" );
double CD1c = VS( SECSTAL1, "CD1c" );
double CS1 = VS( SECSTAL1, "CS1" );
double Ls = VS( LABSUPL1, "Ls" );
double HH1 = VS( SECSTAL1, "HH1" );
double HP1 = VS( SECSTAL1, "HP1" );
double RD = VS( SECSTAL1, "RD" );
double age1avg = VS( SECSTAL1, "age1avg" );

double nonNeg[ ] = { CD1, CD1c, CS1, D1, Deb1, Div1, Eq1, JO1, L1, L1d, L1dRD,
					 L1rd, Q1, Q1e, S1, Tax1, W1, cEntry1, cExit1, i1, iD1, imi,
					 inn, HH1, HP1, RD, age1avg };
double posit[ ] = { A1, PPI };
double finite[ ] = { NW1, entry1exit, Pi1 };

dblVecT all ( nonNeg, END_ARR( nonNeg ) );
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// innovation, productivity
LOG( "\n  ^^ (t=%g) inn=%.3g imi=%.3g A1=%.3g D1=%.3g Q1=%.3g Q1e=%.3g",
	 T, inn, imi, A1, D1, Q1, Q1e );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( auto itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE",
				 itd - all.begin( ) + 1, & errors );

check_error( Aerr.size( ) > TOL * F1, "ZERO-PROD-FIRMS", Aerr.size( ), & errors );

check_error( RDerr.size( ) > TOL * F1, "ZERO-RD-FIRMS", RDerr.size( ), & errors );

check_error( floor( Q1e ) > Q1 || floor( Q1 ) > D1,
			 "INCONSISTENT-PRODUCTION", 0, & errors );

// labor
LOG( "\n   ^ JO1=%g L1d=%g L1dRD=%g L1=%g L1rd=%g",
	 JO1, L1d, L1dRD, L1, L1rd );

check_error( ceil( L1dRD ) < L1rd || floor( L1rd ) > L1 || ceil( L1d ) < JO1 ||
			 floor( L1 ) > Ls || ceil( L1 ) < v[5] || ceil( L1rd ) < v[6],
			 "INCONSISTENT-LABOR", 0, & errors );

// cash flow
LOG( "\n   ^ S1=%.3g W1=%.3g Tax1=%.3g Pi1=%.3g NW1=%.3g",
	 S1, W1, Tax1, Pi1, NW1 );

check_error( S1 + RD < ( 1 - TOL ) * W1, "HIGH-WAGES", 0, & errors );

check_error( c1err.size( ) > 0, "ZERO-COST-FIRM", c1err.size( ), & errors );

check_error( abs( S1 - W1 - Tax1 - ( Pi1 - Tax1 ) + iD1 - i1 ) > TOL,
			 "INCONSISTENT-SFC-FLOW", 0, & errors );

check_error( sfcCerr.size( ) > 0, "SFC-FLOW-ERR-FIRM", sfcCerr.size( ), & errors );

// finance
LOG( "\n   ^ Deb1=%.3g CD1=%.3g CS1=%.3g CD1c=%.3g PPI=%.2g",
	 Deb1, CD1, CS1, CD1c, PPI );

check_error( CS1 > CD1 || CD1c > CD1, "INCONSISTENT-FINANCE", 0, & errors );

check_error( abs( ( Pi1 - Tax1 ) - Div1_1 + cEntry1_1 - cExit1_1 + BadDeb1_1
				  - ( NW1 - NW1_1 ) + ( Deb1 - Deb1_1 ) ) > TOL,
				 "INCONSISTENT-SFC-CAPITAL", 0, & errors );

check_error( sfcKerr.size( ) > 0, "SFC-CAP-ERR-FIRM", sfcKerr.size( ), & errors );

// competition
LOG( "\n   ^ age1avg=%.3g entry1exit=%.2g HH1=%.2g HP1=%.2g",
	 age1avg, entry1exit, HH1, HP1 );

check_error( v[4] < 1 - TOL / 10 || v[4] >	1 + TOL / 10,
			 "INCONSISTENT-SHARES", 0, & errors );

check_error( CliErr.size( ) > 0, "NO-CLIENT-FIRMS", CliErr.size( ), & errors );

check_error( HH1 > 1 || HP1 > 2, "INCONSISTENT-STATS", 0, & errors );

errorsTot += errors;

if ( T == v[2] )
	LOG( "\n ^^^ TESTING OF CAPITAL-GOOD SECTOR FINISHED (%d)", errorsTot );

RESULT( errors )


EQUATION( "test2sec" )
/*
Print detailed statistics of consumption-good sector (!=0 if error is found)
Set the time range in 'test2StIni' and 'test2StEnd'
*/

v[1] = V( "test2StIni" );
v[2] = V( "test2StEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

static int errorsTot = 0;						// all runs error accumulator

if ( T == v[1] )
	LOG( "\n &&& TESTING OF CONSUMPTION-GOOD SECTOR STARTED" );

// scan firms for severe problems
double F2 = VS( CONSECL1, "F2" );				// number of firms
double mu20 = VS( CONSECL1, "mu20" );			// initial mark-up

objVecT D2err, Kerr, L2err, Q2err, W2err,		// vectors to save error firms
		c2err, mu2err, sfcCerr, sfcKerr;

int errors = 0;									// error counter
k = v[4] = v[5] = 0;							// accumulators
CYCLES( CONSECL1, cur, "Firm2" )
{
	v[4] += VS( cur, "_f2" );
	v[5] += ( VS( cur, "_life2cycle" ) > 0 && VS( cur, "_L2" ) <= 0 ) ? 1 : 0;

	if ( VS( cur, "_life2cycle" ) > 0 && VLS( cur, "_K", 1 ) <= 0 )
		Kerr.push_back( cur );

	if ( VS( cur, "_life2cycle" ) > 0 &&
		 VS( cur, "_Q2d" ) + VLS( cur, "_N", 1 ) <= 0 )
		D2err.push_back( cur );

	if ( VS( cur, "_Q2d" ) > 0 && VS( cur, "_L2" ) > 0 &&
		 VS( cur, "_Q2e" ) <= 0 )
		Q2err.push_back( cur );

	if ( VS( cur, "_Q2e" ) > 0 && VS( cur, "_L2" ) <= 0 )
		L2err.push_back( cur );

	if ( VS( cur, "_c2" ) <= 0 || VS( cur, "_p2" ) <= 0 )
		c2err.push_back( cur );

	if ( VS( cur, "_mu2" ) < mu20 / 5 || VS( cur, "_mu2" ) > mu20 * 5 )
		mu2err.push_back( cur );

	if ( abs( + VS( cur, "_S2" ) - VS( cur, "_W2" ) - VS( cur, "_Tax2" )
			  - ( VS( cur, "_Pi2" ) - VS( cur, "_Tax2" ) )
			  + VS( cur, "_iD2" ) - VS( cur, "_i2" ) ) > TOL )
		 sfcCerr.push_back( cur );

	if ( abs( - VS( cur, "_Inom" ) + ( VS( cur, "_Pi2" ) - VS( cur, "_Tax2" ) )
			  - VLS( cur, "_Div2", 1 )
			  - ( VS( cur, "_NW2" ) - VLS( cur, "_NW2", 1 ) )
			  + ( VS( cur, "_Deb2" ) - VLS( cur, "_Deb2", 1 ) )
			  + ( VS( cur, "_t2ent" ) == T ? VS( cur, "_Eq2" ) : 0 ) ) > TOL )
		 sfcKerr.push_back( cur );

	++k;
}

double A2 = VS( CONSECL1, "A2" );
double CI = VS( CONSECL1, "CI" );
double CPI = VS( CONSECL1, "CPI" );
double Deb2 = VS( CONSECL1, "Deb2" );
double Div2 = VS( CONSECL1, "Div2" );
double D2 = VS( CONSECL1, "D2" );
double D2d = VS( CONSECL1, "D2d" );
double D2e = VS( CONSECL1, "D2e" );
double Eavg = VS( CONSECL1, "Eavg" );
double EI = VS( CONSECL1, "EI" );
double Eq2 = VS( CONSECL1, "Eq2" );
double Id = VS( CONSECL1, "Id" );
double Inom = VS( CONSECL1, "Inom" );
double Ireal = VS( CONSECL1, "Ireal" );
double JO2 = VS( CONSECL1, "JO2" );
double K = VS( CONSECL1, "K" );
double Kd = VS( CONSECL1, "Kd" );
double Knom = VS( CONSECL1, "Knom" );
double L2 = VS( CONSECL1, "L2" );
double L2d = VS( CONSECL1, "L2d" );
double N = VS( CONSECL1, "N" );
double NW2 = VS( CONSECL1, "NW2" );
double Pi2 = VS( CONSECL1, "Pi2" );
double Q2 = VS( CONSECL1, "Q2" );
double Q2d = VS( CONSECL1, "Q2d" );
double Q2e = VS( CONSECL1, "Q2e" );
double Q2p = VS( CONSECL1, "Q2p" );
double Q2u = VS( CONSECL1, "Q2u" );
double S2 = VS( CONSECL1, "S2" );
double SI = VS( CONSECL1, "SI" );
double Tax2 = VS( CONSECL1, "Tax2" );
double W2 = VS( CONSECL1, "W2" );
double c2 = VS( CONSECL1, "c2" );
double c2e = VS( CONSECL1, "c2e" );
double dCPI = VS( CONSECL1, "dCPI" );
double cEntry2 = VS( CONSECL1, "cEntry2" );
double cExit2 = VS( CONSECL1, "cExit2" );
double entry2exit = VS( CONSECL1, "entry2exit" );
double i2 = VS( CONSECL1, "i2" );
double iD2 = VS( CONSECL1, "iD2" );
double l2avg = VS( CONSECL1, "l2avg" );
double oldVint = VS( CONSECL1, "oldVint" );
double p2avg = VS( CONSECL1, "p2avg" );

double Deb2_1 = VLS( CONSECL1, "Deb2", 1 );
double Div2_1 = VLS( CONSECL1, "Div2", 1 );
double Kavb = VLS( CONSECL1, "K", 1 );
double NW2_1 = VLS( CONSECL1, "NW2", 1 );
double cEntry2_1 = VLS( CONSECL1, "cEntry2", 1 );
double cExit2_1 = VLS( CONSECL1, "cExit2", 1 );

double BadDeb2_1 = VLS( FINSECL1, "BadDeb2", 1 );
double CD2 = VS( SECSTAL1, "CD2" );
double CD2c = VS( SECSTAL1, "CD2c" );
double CS2 = VS( SECSTAL1, "CS2" );
double Ls = VS( LABSUPL1, "Ls" );
double HH2 = VS( SECSTAL1, "HH2" );
double HP2 = VS( SECSTAL1, "HP2" );
double age2avg = VS( SECSTAL1, "age2avg" );
double mu2avg = VS( SECSTAL1, "mu2avg" );

double nonNeg[ ] = { CD2, CD2c, CS2, CPI, CI, D2, D2d, D2e, Deb2, Div2,
					 EI, Eq2, Id, Inom, Ireal, JO2, K, Kavb, Kd, Knom, L2,
					 L2d, N, Q2, Q2d, Q2e, Q2p, Q2u, SI, S2, Tax2, W2, cEntry2,
					 cExit2, entry2exit, i2, iD2, l2avg, HH2, HP2 };
double posit[ ] = { A2, - Eavg, oldVint, p2avg, age2avg, mu2avg };
double finite[ ] = { NW2, dCPI, Pi2 };

dblVecT all ( nonNeg, END_ARR( nonNeg ) );
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// capital and investment
LOG( "\n  && (t=%g) Kd=%.3g Kavb=%.3g K=%.3g",
	 T, Kd, Kavb, K );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( auto itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

check_error( Kerr.size( ) > 0, "NO-CAPITAL-FIRMS", Kerr.size( ), & errors );

LOG( "\n   & Inom=%.3g Ireal=%.3g EI=%.3g SI=%.3g CI=%.3g",
	 Inom, Ireal, EI, SI, CI );

check_error( SI + EI > Id, "INCONSISTENT-INVESTMENT", 0, & errors );

check_error( SI > Kavb || K > Kavb + EI + SI,
			 "INCONSISTENT-CAPITAL", 0, & errors );

// productivity
LOG( "\n   & A2=%.3g c2=%.3g c2e=%.3g",
	 A2, c2, c2e );

check_error( c2e < ( 1 - 3 * TOL * Q2 / Q2e ) * c2,
			 "LOW-UNIT-COST", 0, & errors );

// production
LOG( "\n   & Q2d=%.3g Q2=%.3g Q2e=%.3g",
	 Q2d, Q2, Q2e );

check_error( Q2e > Q2 || Q2 > Q2d || Q2e > K,
			 "INCONSISTENT-PRODUCTION", 0, & errors );

check_error( c2err.size( ) > 0, "ZERO-COST-FIRMS", c2err.size( ), & errors );

check_error( Q2err.size( ) > 0, "NO-PROD-FIRMS", Q2err.size( ), & errors );

// labor
LOG( "\n   & JO2=%g L2d=%g L2=%g",
	 JO2, L2d, L2 );

check_error( ceil( L2d ) < JO2 || floor( L2 ) > Ls,
			 "INCONSISTENT-LABOR", 0, & errors );

check_error( L2err.size( ) > 0, "NO-LABOR-PRODUCING", L2err.size( ), & errors );

// cash flow
LOG( "\n   & W2=%.3g Tax2=%.3g Pi2=%.3g",
	 W2, Tax2, Pi2 );

check_error( W2err.size( ) > 0, "WAGES-GAP", W2err.size( ), & errors );

check_error( S2 < ( 1 - TOL ) * W2, "HIGH-WAGES", 0, & errors );

check_error( v[5] / F2 > TOL, "MANY-NO-WORKER", 0, & errors );

check_error( abs( S2 - W2 - Tax2 - ( Pi2 - Tax2 ) + iD2 - i2 ) > TOL,
			 "INCONSISTENT-SFC-FLOW", 0, & errors );

check_error( sfcCerr.size( ) > 0, "SFC-FLOW-ERR-FIRM", sfcCerr.size( ), & errors );

// finance
LOG( "\n   & NW2=%.3g Deb2=%.3g CD2=%.3g CD2c=%.3g CPI=%.2g",
	 NW2, Deb2, CD2, CD2c, CPI );

check_error( CS2 > CD2 || CD2c > CD2, "INCONSISTENT-FINANCE", 0, & errors );

check_error( abs( - Inom + ( Pi2 - Tax2 ) - Div2_1 + cEntry2_1
				  - cExit2_1 + BadDeb2_1 - ( NW2 - NW2_1 )
				  + ( Deb2 - Deb2_1 ) ) > TOL,
				 "INCONSISTENT-SFC-CAPITAL", 0, & errors );

check_error( sfcKerr.size( ) > 0, "SFC-CAP-ERR-FIRM", sfcKerr.size( ), & errors );

// market
LOG( "\n   & mu2avg=%.2g p2avg=%.2g D2=%.3g D2e=%.3g S2=%.3g",
	 mu2avg, p2avg, D2, D2e, S2 );

check_error( D2 > ( 1 + TOL ) * D2d, "INCONSISTENT-DEMAND", 0, & errors );

check_error( D2err.size( ) > 0, "NO-DEMAND-FIRMS", D2err.size( ), & errors );

check_error( mu2err.size( ) > 0, "BAD-MARKUP-FIRMS", mu2err.size( ), & errors );

// competition
LOG( "\n   & age2avg=%g entry2exit=%.2g HH2=%.2g HP2=%.2g",
	 age2avg, entry2exit, HH2, HP2 );
LOG( "\n   & l2avg=%.2g", l2avg );

check_error( v[4] < 1 - TOL / 10 || v[4] > 1 + TOL / 10,
			 "INCONSISTENT-SHARES", 0, & errors );

check_error( HH2 > 1 || HP2 > 2, "INCONSISTENT-STATS", 0, & errors );

errorsTot += errors;

if ( T == v[2] )
	LOG( "\n &&& TESTING OF CONSUMPTION-GOOD SECTOR FINISHED (%d)", errorsTot );

RESULT( errors )


/*=========================== FIRM-LEVEL TESTS ===============================*/

EQUATION( "test1firm" )
/*
Print detailed statistics of firms in capital-good sector (!=0 if error)
Set the firm range by firm _ID1 in 'test1idIni' and 'test1idEnd'
Set the time range in 'test1tIni' and 'test1tEnd'
If 'test1idIni' is zero, list up to 'test1idEnd' firms entered from 'test1tIni'
*/

static FILE *firms1 = NULL;						// output file pointer

v[1] = V( "test1tIni" );
v[2] = V( "test1tEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

v[3] = V( "test1idIni" );
v[4] = V( "test1idEnd" );

h = v[5] = 1 + v[2] - v[1];						// number of periods
k = v[6] = v[3] > 0 ? 1 + v[4] - v[3] : v[4];	// number of firms

static double iniAtau, iniBtau;
static int errorsTot = 0;						// all runs error accumulator
static firmMapT entr;

if ( T == v[1] )
{
	LOG( "\n *** TESTING OF CAPITAL-GOOD FIRMS STARTED" );

	if ( v[3] > 0 )
		LOG( " (t=%g-%g, ID1=%g-%g, file=%s)",
			 v[1], v[2], v[3], v[4], TEST1FILE );
	else
		LOG( " (t=%g-%g, up to %g entrants, file=%s)",
			 v[1], v[2], v[4], TEST1FILE );

	if ( firms1 == NULL )						// don't reopen if already open
	{
		firms1 = fopen( TEST1FILE, "w" );		// (re)create the file
		fprintf( firms1, "%s,%s,%s,%s\n",		// file header
				 "t,ID1,t1ent,Client",
				 "RD,Atau,Btau,c1,D1,Q1,Q1e,L1d,L1",
				 "Pi1,NW1,Deb1,Deb1max,CS1,CD1c",
				 "HC,NC,BC,p1,S1,f1" );
	}
}

int errors = 0;									// error counter
CYCLES( CAPSECL1, cur, "Firm1" )
{
	i = VS( cur, "_ID1" );
	h = VS( cur, "_t1ent" );

	// print ID range
	if ( v[3] > 0 && ( i < v[3] || i > v[4] ) )
		continue;
	else
		// print entrants in certain period
		if ( v[3] <= 0 )
		{
			if ( entr.find( i ) == entr.end( ) )// not in list?
			{
				// too old or list is full?
				if ( h < v[1] || entr.size( ) >= ( unsigned ) v[4] )
					continue;
				else
					entr[ i ] = cur;			// add entrant in to show list
			}
		}

	j = COUNTS( cur, "Cli" );

	double _Atau = VS( cur, "_Atau" );
	double _Btau = VS( cur, "_Btau" );
	double _BC = VS( cur, "_BC" );
	double _CD1 = VS( cur, "_CD1" );
	double _CD1c = VS( cur, "_CD1c" );
	double _CS1 = VS( cur, "_CS1" );
	double _D1 = VS( cur, "_D1" );
	double _Deb1 = VS( cur, "_Deb1" );
	double _Deb1max = VS( cur, "_Deb1max" );
	double _Div1 = VS( cur, "_Div1" );
	double _Eq1 = VS( cur, "_Eq1" );
	double _HC = VS( cur, "_HC" );
	double _L1 = VS( cur, "_L1" );
	double _L1d = VS( cur, "_L1d" );
	double _L1dRD = VS( cur, "_L1dRD" );
	double _L1rd = VS( cur, "_L1rd" );
	double _NC = VS( cur, "_NC" );
	double _NW1 = VS( cur, "_NW1" );
	double _Pi1 = VS( cur, "_Pi1" );
	double _Q1 = VS( cur, "_Q1" );
	double _Q1e = VS( cur, "_Q1e" );
	double _RD = VS( cur, "_RD" );
	double _S1 = VS( cur, "_S1" );
	double _Tax1 = VS( cur, "_Tax1" );
	double _W1 = VS( cur, "_W1" );
	double _c1 = VS( cur, "_c1" );
	double _f1 = VS( cur, "_f1" );
	double _i1 = VS( cur, "_i1" );
	double _iD1 = VS( cur, "_iD1" );
	double _imi = VS( cur, "_imi" );
	double _inn = VS( cur, "_inn" );
	double _p1 = VS( cur, "_p1" );
	double _qc1 = VS( cur, "_qc1" );

	double _Deb1_1 = VLS( cur, "_Deb1", 1 );
	double _Div1_1 = VLS( cur, "_Div1", 1 );
	double _NW1_1 = VLS( cur, "_NW1", 1 );

	double nonNeg[ ] = { _CS1, _CD1, _CD1c, _Div1, _Eq1, _HC, _NC, _RD, _D1, _Q1,
						 _Q1e, _BC, _L1, _L1d, _L1dRD, _L1rd, _Deb1, _Deb1max,
						 _S1, _Tax1, _W1, _f1, _i1, _iD1, _imi, _inn, _qc1 };
	double posit[ ] = { _Atau, _Btau, _c1, _p1 };
	double finite[ ] = { _NW1, _Pi1 };

	dblVecT all ( nonNeg, END_ARR( nonNeg ) );
	all.insert( all.end( ), posit, END_ARR( posit ) );
	all.insert( all.end( ), finite, END_ARR( finite ) );

	// first period actions (single-firm analysis only)
	if ( k == 1 && T == v[1] )
	{
		iniAtau = _Atau;
		iniBtau = _Btau;
	}

	LOG( "\n  ** (t=%g) ID1=%d t1ent=%d #Client=%d:", T, i, h, j );
	fprintf( firms1, "%g,%d,%d,%d", T, i, h, j );

	for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
		check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

	for ( i = 0; i < LEN_ARR( posit ); ++i )
		check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

	for ( auto itd = all.begin( ); itd != all.end( ); ++itd )
		check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

	// innovation, productivity
	LOG( "\n   * RD=%g Atau=%.3g Btau=%.3g c1=%.3g D1=%g Q1=%g Q1e=%g L1d=%g L1=%g",
		 round( _RD ), _Atau, _Btau, _c1, _D1, _Q1, _Q1e, _L1d, _L1 );
	fprintf( firms1, ",%g,%g,%g,%g,%g,%g,%g,%g,%g",
			 _RD, _Atau, _Btau, _c1, _D1, _Q1, _Q1e, _L1d, _L1 );

	check_error( _RD <= 0, "NO-R&D", 0, & errors );

	check_error( _Btau > _Atau || _Atau < TOL || _Btau < TOL / 10,
				 "INCONSISTENT-PRODUCTIVITY", 0, & errors );

	// finance
	LOG( "\n   * Pi1=%g NW1=%g Deb1=%g Deb1max=%g CS1=%g CD1c=%g",
		 round( _Pi1 ), round( _NW1 ), round( _Deb1 ), round( _Deb1max ),
		 round( _CS1 ), round( _CD1c ) );
	fprintf( firms1, ",%g,%g,%g,%g,%g,%g",
			 _Pi1, _NW1, _Deb1, _Deb1max, _CS1, _CD1c );

	check_error( _CS1 > _CD1 || _CD1c > _CD1, "INCONSISTENT-FINANCE", 0, & errors );

	check_error( abs( _S1 - _W1 - _Tax1 - ( _Pi1 - _Tax1 ) + _iD1 - _i1 ) > TOL,
				 "INCONSISTENT-SFC-FLOW", 0, & errors );

	check_error( abs( ( _Pi1 - _Tax1 ) - _Div1_1 - ( _NW1 - _NW1_1 ) +
					  ( _Deb1 - _Deb1_1 ) + ( h == T ? + _Eq1 : 0 ) ) > TOL,
				 "INCONSISTENT-SFC-CAPITAL", 0, & errors );

	// market and client dynamics
	LOG( "\n   * HC=%g NC=%g BC=%g p1=%.2g S1=%g f1=%.2g",
		 _HC, _NC, _BC, _p1, round( _S1 ), _f1 );
	fprintf( firms1, ",%g,%g,%g,%g,%g,%g",
			 _HC, _NC, _BC, _p1, _S1, _f1 );

	check_error( j > _HC + _NC, "INCONSISTENT-CLIENTS", 0, & errors );

	check_error( j < _BC, "INCONSISTENT-BUYERS", 0, & errors );

	// last period actions (single-firm analysis only)
	if ( k == 1 && T == v[2] )
	{
		LOG( "\n   * AtauGwth=%.3g BtauGwth=%.3g",
			 ( log( _Atau + 1 ) - log( iniAtau + 1 ) ) / v[5],
			 ( log( _Btau + 1 ) - log( iniBtau + 1 ) ) / v[5] );
	}

	fputs( "\n", firms1 );
}

errorsTot += errors;

if ( T == v[2] )
{
	LOG( "\n *** TESTING OF CAPITAL-GOOD FIRMS FINISHED (%d)", errorsTot );
	fclose( firms1 );
	firms1 = NULL;
}

RESULT( errors )


EQUATION( "test2firm" )
/*
Print detailed statistics of firms in consumption-good sector (!=0 if error)
Set the firm range by firm _ID2 in 'test2idIni' and 'test2idEnd'
Set the time range in 'test2tIni' and 'test2tEnd'
If 'test2idIni' is zero, list up to 'test2idEnd' firms entered from 'test2tIni'
*/

static FILE *firms2 = NULL;						// output file pointer

v[1] = V( "test2tIni" );
v[2] = V( "test2tEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

v[3] = V( "test2idIni" );
v[4] = V( "test2idEnd" );

h = v[5] = 1 + v[2] - v[1];						// number of periods
k = v[6] = v[3] > 0 ? 1 + v[4] - v[3] : v[4];	// number of firms

static double iniK, iniNW2, iniDeb2;
static int errorsTot = 0;						// all runs error accumulator
static firmMapT entr;

if ( T == v[1] )
{
	LOG( "\n ### TESTING OF CONSUMER-GOOD FIRMS STARTED" );

	if ( v[3] > 0 )
		LOG( " (t=%g-%g, ID2=%g-%g, file=%s)",
			 v[1], v[2], v[3], v[4], TEST2FILE );
	else
		LOG( " (t=%g-%g, up to %g entrants, file=%s)",
			 v[1], v[2], v[4], TEST2FILE );

	if ( firms2 == NULL )						// don't reopen if already open
	{
		firms2 = fopen( TEST2FILE, "w" );		// (re)create the file
		fprintf( firms2, "%s,%s,%s,%s,%s\n",	// file header
				 "t,ID2,t2ent,life2cycle,Broch,Vint,pVint",
				 "Kd,Kavb,EId,SId,EI,SI,CI,K", "D2e,Q2d,Q2,Q2e,L2d,L2,c2",
				 "Pi2,NW2,Deb2,Deb2max,CS2,CD2c",
				 "mu2,p2,D2,S2,W2,f2,N" );
	}
}

double eta = VS( CONSECL1, "eta" );				// machine technical life
double iota = VS( CONSECL1, "iota" );			// production slack
double m2 = VS( CONSECL1, "m2" );				// machine scale
double mu20 = VS( CONSECL1, "mu20" );			// initial mark-up

int errors = 0;									// error counter
CYCLES( CONSECL1, cur, "Firm2" )
{
	j = VS( cur, "_ID2" );
	h = VS( cur, "_t2ent" );

	// print ID range
	if ( v[3] > 0 && ( j < v[3] || j > v[4] ) )
		continue;
	else
		// print entrants in certain period
		if ( v[3] <= 0 )
		{
			if ( entr.find( j ) == entr.end( ) )// not in list?
			{
				// too old or list is full?
				if ( h < v[1] || entr.size( ) >= ( unsigned ) v[4] )
					continue;
				else
					entr[ j ] = cur;			// add entrant in to show list
			}
		}

	// scan vintages for severe problems
	double _L2 = VS( cur, "_L2" );
	double _Q2e = VS( cur, "_Q2e" );

	objVecT IDerr, tVintErr, QvintErr,			// vectors to save error firms
			LvintErr;

	v[7] = v[8] = 0;							// accumulators
	cur2 = NULL;								// last vintage
	CYCLES( cur, cur1, "Vint" )
	{
		v[7] += VS( cur1, "__nVint" );
		v[8] += VS( cur1, "__Qvint" );

		if ( VS( cur1, "__IDvint" ) < h )
			IDerr.push_back( cur1 );

		if ( ( h == 1 && VS( cur1, "__tVint" ) < 1 - eta ) ||
			 ( h > 1 && VS( cur1, "__tVint" ) < h ) || VS( cur1, "__tVint" ) > T )
			tVintErr.push_back( cur1 );

		if ( VS( cur1, "__Qvint" ) < 0 )
			QvintErr.push_back( cur1 );

		if ( VS( cur1, "__toUseVint" ) == 0 && ( VS( cur1, "__LdVint" ) > 0 ||
			 VS( cur1, "__Lvint" ) > 0 ) )
			LvintErr.push_back( cur1 );

		cur2 = cur1;
	}

	v[18] = COUNTS( cur, "Broch" );
	v[19] = COUNTS( cur, "Vint" );
	cur1 = HOOKS( cur, TOPVINT );

	double _A2 = VS( cur, "_A2" );
	double _CD2 = VS( cur, "_CD2" );
	double _CD2c = VS( cur, "_CD2c" );
	double _CI = VS( cur, "_CI" );
	double _CS2 = VS( cur, "_CS2" );
	double _D2 = VS( cur, "_D2" );
	double _D2d = VS( cur, "_D2d" );
	double _D2e = VS( cur, "_D2e" );
	double _Deb2 = VS( cur, "_Deb2" );
	double _Deb2max = VS( cur, "_Deb2max" );
	double _Div2 = VS( cur, "_Div2" );
	double _E = VS( cur, "_E" );
	double _EI = VS( cur, "_EI" );
	double _EId = VS( cur, "_EId" );
	double _Eq2 = VS( cur, "_Eq2" );
	double _Inom = VS( cur, "_Inom" );
	double _JO2 = VS( cur, "_JO2" );
	double _K = VS( cur, "_K" );
	double _Kd = VS( cur, "_Kd" );
	double _Knom = VS( cur, "_Knom" );
	double _L2d = VS( cur, "_L2d" );
	double _N = VS( cur, "_N" );
	double _NW2 = VS( cur, "_NW2" );
	double _Pi2 = VS( cur, "_Pi2" );
	double _Q2 = VS( cur, "_Q2" );
	double _Q2d = VS( cur, "_Q2d" );
	double _Q2p = VS( cur, "_Q2p" );
	double _Q2pe = VS( cur, "_Q2pe" );
	double _Q2u = VS( cur, "_Q2u" );
	double _RS2 = VS( cur, "_RS2" );
	double _S2 = VS( cur, "_S2" );
	double _SI = VS( cur, "_SI" );
	double _SId = VS( cur, "_SId" );
	double _Tax2 = VS( cur, "_Tax2" );
	double _W2 = VS( cur, "_W2" );
	double _c2 = VS( cur, "_c2" );
	double _c2e = VS( cur, "_c2e" );
	double _f2 = VS( cur, "_f2" );
	double _i2 = VS( cur, "_i2" );
	double _iD2 = VS( cur, "_iD2" );
	double _l2 = VS( cur, "_l2" );
	double _life2cycle = VS( cur, "_life2cycle" );
	double _mu2 = VS( cur, "_mu2" );
	double _p2 = VS( cur, "_p2" );
	double _qc2 = VS( cur, "_qc2" );
	double _t2ent = VS( cur, "_t2ent" );

	double _Deb2_1 = VLS( cur, "_Deb2", 1 );
	double _Div2_1 = VLS( cur, "_Div2", 1 );
	double _Kavb = VLS( cur, "_K", 1 );
	double _N_1 = VLS( cur, "_N", 1 );
	double _NW2_1 = VLS( cur, "_NW2", 1 );
	double __pVint = ( cur1 != NULL && VS( cur1, "__tVint" ) == T ) ?
					VS( cur1, "__pVint" ) : 0;

	double nonNeg[ ] = { _A2, _CD2, _CD2c, _CI, _CS2, _D2, _D2d, _D2e, _Deb2,
						 _Deb2max, _Div2, _EI, _EId, _Eq2, _JO2, _K, _Kavb,
						 _Kd, _L2, _L2d, _N, _Q2, _Q2d, _Q2e, _Q2p, _Q2pe,
						 _Q2u, _RS2, _S2, _SI, _SId, _Tax2, _W2, _c2, _c2e,
						 _f2, _i2, _iD2, _l2, _life2cycle, __pVint, _qc2,
						 _t2ent };
	double posit[ ] = { - _E, _mu2, _p2 };
	double finite[ ] = { _NW2, _Pi2 };

	dblVecT all ( nonNeg, END_ARR( nonNeg ) );
	all.insert( all.end( ), posit, END_ARR( posit ) );
	all.insert( all.end( ), finite, END_ARR( finite ) );

	// first period actions (single-firm analysis only)
	if ( k == 1 && T == v[1] )
	{
		iniK = _K;
		iniNW2 = _NW2;
		iniDeb2 = _Deb2;
	}

	LOG( "\n  ## (t=%g) ID2=%d t2ent=%d life2cycle=%g #Broch=%g #Vint=%g pVint=%g",
		 T, j, h, _life2cycle, v[18], v[19], round( __pVint ) );
	fprintf( firms2, "%g,%d,%d,%g,%g,%g,%g", T, j, h, _life2cycle, v[18],
			 v[19], __pVint );

	for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
		check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

	for ( i = 0; i < LEN_ARR( posit ); ++i )
		check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

	for ( auto itd = all.begin( ); itd != all.end( ); ++itd )
		check_error( ! isfinite( *itd ),
					 "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

	check_error( v[18] == 0, "NO-BROCHURE", 0, & errors );

	// capital and investment
	LOG( "\n   # Kd=%g Kavb=%g EId=%g SId=%g EI=%g SI=%g CI=%g K=%g",
		 _Kd, _Kavb, _EId, _SId, _EI, _SI, _CI, _K );
	fprintf( firms2, ",%g,%g,%g,%g,%g,%g,%g,%g",
		 _Kd, _Kavb, _EId, _SId, _EI, _SI, _CI, _K );

	check_error( _life2cycle > 0 && v[19] == 0, "NO-VINTAGE", 0, & errors );

	check_error( IDerr.size( ) > 0, "INVALID-ID-VINT", IDerr.size( ), & errors );

	check_error( tVintErr.size( ) > 0,
				 "INVALID-T-VINT", tVintErr.size( ), & errors );

	check_error( _SId > _Kavb || _K > _Kavb + _EI + _SI || v[7] * m2 > _K ||
				 ( _life2cycle > 0 && _K == 0 ),
				 "INCONSISTENT-CAPITAL", 0, & errors );

	LOG( "\n   # D2e=%g Q2d=%g Q2=%g Q2e=%g L2d=%g L2=%g c2=%.2g",
		 round( _D2e ), _Q2d, _Q2, round( _Q2e ), _L2d, _L2, _c2 );
	fprintf( firms2, ",%g,%g,%g,%g,%g,%g,%g",
		 _D2e, _Q2d, _Q2, _Q2e, _L2d, _L2, _c2 );

	check_error( ( _Q2d > 0 &&
				 floor( _Q2d ) > ceil( ( 1 + iota ) * _D2e - _N_1 ) ) ||
				 _Q2 > _Q2d || _Q2e > _K || _Q2e > _Q2,
				 "INCONSISTENT-PRODUCTION", 0, & errors );

	check_error( QvintErr.size( ) > 0,
				 "INVALID-PROD-VINT", QvintErr.size( ), & errors );

	check_error( _L2d > 0 && _L2 == 0, "NO-WORKER", 0, & errors );

	check_error( LvintErr.size( ) > 0,
				 "INVALID-LABOR-VINT", LvintErr.size( ), & errors );

	check_error( _life2cycle > 0 && ( _c2 == 0 || _c2e == 0 ),
				 "INCONSISTENT-COST", 0, & errors );

	// finance
	LOG( "\n   # Pi2=%g NW2=%g Deb2=%g Deb2max=%g CS2=%g CD2c=%g",
		 round( _Pi2 ), round( _NW2 ), round( _Deb2 ), round( _Deb2max ),
		 round( _CS2 ), round( _CD2c ) );
	fprintf( firms2, ",%g,%g,%g,%g,%g,%g",
			 _Pi2, _NW2, _Deb2, _Deb2max, _CS2, _CD2c );

	check_error( _CS2 > _CD2 || _CD2c > _CD2, "INCONSISTENT-FINANCE", 0, & errors );

	check_error( abs( _S2 - _W2 - _Tax2 - ( _Pi2 - _Tax2 ) + _iD2 - _i2 ) > TOL,
				 "INCONSISTENT-SFC-FLOW", 0, & errors );

	check_error( abs( - _Inom + ( _Pi2 - _Tax2 ) - _Div2_1
					  - ( _NW2 - _NW2_1 ) + ( _Deb2 - _Deb2_1 )
					  + ( h == T ? + _Eq2 : 0 ) ) > TOL,
				 "INCONSISTENT-SFC-CAPITAL", 0, & errors );

	// market
	LOG( "\n   # mu2=%.2g p2=%.2g D2=%g S2=%g W2=%g f2=%.2g N=%g", _mu2, _p2,
		 round( _D2 ), round( _S2 ), round( _W2 ), _f2, round( _N ) );
	fprintf( firms2, ",%g,%g,%g,%g,%g,%g,%g", _mu2, _p2, _D2, _S2, _W2, _f2, _N );

	check_error( _mu2 > 5 * mu20, "EXCESSIVE-MARKUP", 0, & errors );

	check_error( _mu2 < mu20 / 5, "INSUFFICIENT-MARKUP", 0, & errors );

	// last period actions (single-firm analysis only)
	if ( k == 1 && T == v[2] )
	{
		LOG( "\n   # Kgwth=%.3g NW2gwth=%.3g Deb2gwth=%.3g",
			 ( log( _K + 1 ) - log( iniK + 1 ) ) / v[5],
			 ( log( _NW2 + 1 ) - log( iniNW2 + 1 ) ) / v[5],
			 ( log( _Deb2 + 1 ) - log( iniDeb2 + 1 ) ) / v[5] );
	}

	fputs( "\n", firms2 );
}

errorsTot += errors;

if ( T == v[2] )
{
	LOG( "\n ### TESTING OF CONSUMER-GOOD FIRMS FINISHED (%d)", errorsTot );
	fclose( firms2 );
	firms2 = NULL;
}

RESULT( errors )
