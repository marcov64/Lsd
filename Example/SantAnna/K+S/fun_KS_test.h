/******************************************************************************

	TEST EQUATIONS
	--------------

	Equations that are not required for the model to run but can be used for
	testing, in-depth consistency analysis, and debugging.
 
 ******************************************************************************/

#define TEST1FILE "firms1.csv"					// names of test files on disk
#define TEST2FILE "firms2.csv"

#define TOL	0.1

/*========================= COUNTRY-LEVEL TESTS ==============================*/

EQUATION( "testCountry" )
/*
Print detailed statistics of country macro (!=0 if error is found)
Set the time range in 'testCtIni' and 'testCtEnd'
*/

if ( t == 1 )
	PLOG( "\n Optional statistics being computed in object 'Stats'" );

v[1] = V( "testCtIni" );
v[2] = V( "testCtEnd" );

if ( t >= v[2] )
	PARAMETER;									// compute for the last time

if ( t < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

static double iniGDP, iniA, iniDeb, iniSavAcc;

if ( t == v[1] )
	LOG( "\n @@@ TESTING OF COUNTRY MACRO STARTED" );

int errors = 0;									// error counter

double A = VS( PARENT, "A" );
double C = VS( PARENT, "C" );
double Deb = VS( PARENT, "Deb" );
double Def = VS( PARENT, "Def" );
double G = VS( PARENT, "G" );
double GDP = VS( PARENT, "GDP" );
double GDPnom = VS( PARENT, "GDPnom" );
double Sav = VS( PARENT, "Sav" );
double SavAcc = VS( PARENT, "SavAcc" );
double Tax = VS( PARENT, "Tax" );
double cEntry = VS( PARENT, "cEntry" );
double cExit = VS( PARENT, "cExit" );
double dGDP = VS( PARENT, "dGDP" );
double entryExit = VS( PARENT, "entryExit" );

double PPI = VS( CAPSECL1, "PPI" );
double A1 = VS( CAPSECL1, "A1" );
double Q1e = VS( CAPSECL1, "Q1e" );

double A2 = VS( CONSECL1, "A2" );
double CPI = VS( CONSECL1, "CPI" );
double D2d = VS( CONSECL1, "D2d" );
double I = VS( CONSECL1, "I" );
double Inom = VS( CONSECL1, "Inom" );
double N = VS( CONSECL1, "N" );
double Q2e = VS( CONSECL1, "Q2e" );
double dNnom = VS( CONSECL1, "dNnom" );

double GDI = VS( MACSTAL1, "GDI" );
double dA = VS( MACSTAL1, "dA" );

double dN = VS( SECSTAL1, "dN" );

double nonNeg[ ] = { G, I, Inom, N, Sav, SavAcc, Tax, cEntry, cExit };
double posit[ ] = { A, C, D2d, GDP, GDPnom, PPI, A1, Q1e, CPI, A2, Q2e, GDI };
double finite[ ] = { dA, dGDP, dN, dNnom };

dblVecT all ( nonNeg, END_ARR( nonNeg ) ); 
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// first period actions
if ( t == v[1] )
{
	iniGDP = GDP;
	iniA = A;
	iniDeb = Deb; 
	iniSavAcc = SavAcc;
}
	
// national accounting
LOG( "\n  @@ (t=%d) dA=%.2g dGDP=%.2g C%%%%=%.2g I%%%%=%.2g G%%%%=%.2g dN%%%%=%.2g Sav%%%%=%.2g", t, 
	  dA, dGDP, C / GDPnom, Inom / GDPnom, G / GDPnom, dNnom / GDPnom, Sav / GDPnom );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );
	
for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );
	
check_error( A <= 0 || dA < - TOL, "INVALID-PRODUCTIVITY", 0, & errors );

check_error( A < min( A1, A2 ) || A > max( A1, A2 ), 
			 "INCONSISTENT-PRODUCTIVITY", 0, & errors );
			 
check_error( GDPnom > ( 1 + TOL ) * ( Q1e * PPI + Q2e * CPI ) ||
			 GDPnom < ( 1 - TOL ) * ( Q1e * PPI + Q2e * CPI ), 
			 "INCONSISTENT-GDP", 0, & errors );
			 
check_error( GDP > ( 1 + TOL ) * ( C - Sav + Inom + G + dNnom ) / CPI ||
			 GDP < ( 1 - TOL ) * ( C - Sav + Inom + G + dNnom ) / CPI, 
			 "GDPnom-GAP", 0, & errors );
			 
check_error( GDP > ( 1 + 2 * TOL ) * GDI ||
			 GDP < ( 1 - 2 * TOL ) * GDI, 
			 "GDI-GAP", 0, & errors );

check_error( Sav / GDPnom > 5 * TOL, "HIGH-SAVINGS", 0, & errors );

// forced savings, public debt, firms equity and dynamics
LOG( "\n   @ Tax%%%%=%.2g Def%%%%=%.2g Deb%%%%=%.2g cEntry%%%%=%.2g cExit%%%%=%.2g entryExit=%g", 
	  Tax / GDPnom, Def / GDPnom, Deb / GDPnom, cEntry / GDPnom, cExit / GDPnom, 
	  entryExit );

check_error( cEntry / GDPnom > TOL, "HIGH-EQUITY", 0, & errors );

check_error( Deb / GDPnom > 100 * TOL, "EXPLOSIVE-DEBT", 0, & errors );

// last period actions
if ( t == v[2] )
{
	v[4] = ( log( GDP + 1 ) - log( iniGDP + 1 ) ) / v[3]; 
	v[5] = ( log( A ) - log( iniA ) ) / v[3];
	v[6] = ( log( Deb + 1 ) - log( iniDeb + 1 ) ) / v[3];
	v[7] = ( log( SavAcc + 1 ) - log( iniSavAcc + 1 ) ) / v[3];
	
	LOG( "\n   @ GDPgwth=%.3g Agwth=%.3g DebGwth=%.3g SavGwth=%.3g",
		 v[4], v[5], v[6], v[7] );
		 
	check_error( v[4] < TOL / 20 || v[5] < TOL / 20, "LOW-GROWTH", 0, & errors );
		 
	check_error( v[6] > ( 1 + TOL ) * v[4], "EXPLOSIVE-DEBT-GROWTH", 0, & errors );
		 
	check_error( v[7] > ( 1 + TOL ) * v[4], "EXPLOSIVE-SAVINGS-GROWTH", 0, & errors );
		 
	LOG( "\n @@@ TESTING OF COUNTRY MACRO FINISHED" );
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

if ( t >= v[2] )
	PARAMETER;									// compute for the last time

if ( t < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

if ( t == v[1] )
	LOG( "\n $$$$ TESTING OF FINANCIAL SECTOR STARTED" );

// scan banks for severe problems
objVecT TCerr;									// vector to save TC error banks
int errors = 0;									// error counter
k = v[4] = v[5] = v[6] = v[7] = 0;				// accumulators
CYCLES( FINSECL1, cur, "Bank" )
{
	v[4] += COUNTS( cur, "Cli1" );
	v[5] += COUNTS( cur, "Cli2" );
	v[6] += VS( cur, "_Cl" );
	v[7] += VS( cur, "_fB" );
	
	if ( VS( cur, "_TC" ) < VS( cur, "_TC1free" ) + VS( cur, "_TC2free" ) )
		TCerr.push_back( cur );
	
	++k;
}

double Cl = VS( FINSECL1, "Cl" );
double Depo = VS( FINSECL1, "Depo" );
double DivB = VS( FINSECL1, "DivB" );
double Gbail = VS( FINSECL1, "Gbail" );
double Loans = VS( FINSECL1, "Loans" );
double NWb = VS( FINSECL1, "NWb" );
double PiB = VS( FINSECL1, "PiB" );
double TaxB = VS( FINSECL1, "TaxB" );
double phi = VS( FINSECL1, "phi" );
double r = VS( FINSECL1, "r" );
double rDeb = VS( FINSECL1, "rDeb" );
double rRes = VS( FINSECL1, "rRes" );
double rD = VS( FINSECL1, "rD" );

double TC = VS( MACSTAL1, "TC" );
double Bda = VS( SECSTAL1, "Bda" );
double Bfail = VS( SECSTAL1, "Bfail" );
double BadDeb = VS( SECSTAL1, "BadDeb" );
double HHb = VS( SECSTAL1, "HHb" );
double HPb = VS( SECSTAL1, "HPb" );

double SavAcc = VS( PARENT, "SavAcc" );

double F1 = VS( CAPSECL1, "F1" );
double NW1 = VS( CAPSECL1, "NW1" );
double entry1 = VS( CAPSECL1, "entry1" );
double exit1 = VS( CAPSECL1, "exit1" );

double F2 = VS( CONSECL1, "F2" );
double NW2 = VS( CONSECL1, "NW2" );
double entry2 = VS( CONSECL1, "entry2" );
double exit2 = VS( CONSECL1, "exit2" );

double nonNeg[ ] = { Depo, DivB, Gbail, Loans, TaxB, phi, rRes, Bda, Bfail, 
					 BadDeb, HHb, HPb, SavAcc, NW1, NW2 };
double posit[ ] = { Cl, r, rDeb, F1, F2 };
double finite[ ] = { TC, PiB, NW1, NW2, entry1, exit1, entry2, exit2 };

dblVecT all ( nonNeg, END_ARR( nonNeg ) ); 
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// bank customers and crisis/bail-outs
LOG( "\n  $$$ (t=%d) #Bank=%d #Client1=%g #Client2=%g Bfail=%g Gbail=%.3g phi=%.2g", 
	 t, k, v[4], v[5], Bfail, Gbail, phi );
	 
for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );
	
for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

check_error( F1 != v[4], "INCONSISTENT-CLIENT1", 0, & errors );

check_error( F2 != v[5], "INCONSISTENT-CLIENT2", 0, & errors );

check_error( round( F1 * ( 1 - entry1 + exit1 ) + F2 * ( 1 - entry2 + exit2 ) ) != Cl || v[6] != Cl, 
			 "INCONSISTENT-CLIENT", 0, & errors );

check_error( v[7] < 1 - TOL / 10 || v[7] > 1 + TOL / 10, 
			 "INCONSISTENT-SHARES", 0, & errors );

check_error( phi >= 1, "INCONSISTENT-PHI", 0, & errors );

// interest rat structure and bank assets and liabilities
LOG( "\n   $$ r=%.2g rDeb=%.2g rRes=%.2g Depo=%.3g Loans=%.3g", 
	 r, rDeb, rRes, Depo, Loans );

check_error( rD > r || rRes > r || rDeb < r || rD > rRes || rRes > rDeb, 
			 "INCONSISTENT-INTEREST-STRUCTURE", 0, & errors );

check_error( round( Depo ) > round( NW1 + NW2 + SavAcc ), 
			 "INCONSISTENT-DEPOSITS", 0, & errors );

// banks cash-flow
LOG( "\n   $$ TC=%.3g BadDeb=%.3g TaxB=%.3g PiB=%.3g NWb=%.3g", 
	 TC, BadDeb, TaxB, PiB, NWb );
	
check_error( TC < -1, "NEGATIVE-TOTAL-CREDIT", 0, & errors );

check_error( TCerr.size( ) > 0, "INCONSISTENT-TC-FREE", TCerr.size( ), & errors ); 

if ( t == v[2] )
	LOG( "\n $$$$ TESTING OF FINANCIAL SECTOR FINISHED" );

RESULT( errors )


EQUATION( "testLabor" )
/*
Print detailed statistics of the labor supply (!=0 if error is found)
Set the time range in 'testLtIni' and 'testLtEnd'
*/

v[1] = V( "testLtIni" );
v[2] = V( "testLtEnd" );

if ( t >= v[2] )
	PARAMETER;									// compute for the last time

if ( t < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

if ( t == v[1] )
	LOG( "\n +++ TESTING OF LABOR SUPPLY STARTED" );

// scan workers for severe problems
int Tr = VS( LABSUPL1, "Tr" );					// time for retirement
double Lscale = VS( LABSUPL1, "Lscale" );		// labor scaling factor
double tauT = VS( LABSUPL1, "tauT" );			// tenure skills acc. factor
double wCap = VS( LABSUPL1, "wCap" );			// wage cap
double wMinPol = VS( LABSUPL1, "wMinPol" );		// minimum wage
double wU = VS( LABSUPL1, "wU" );				// unemployment benefit
object *Capital = SEARCHS( PARENT, "Capital" );	// pointer to sector1

objVecT Qerr, ageErr, hirErr, srchErr, wzErr, 	// vectors to save error workers
		wlErr, wChgErr, wRerr, sTerr, VintErr;

int errors = 0;									// error counter
k = v[4] = v[5] = v[6] = v[7] = v[8] = v[9] = v[11] = 0;// accumulators
CYCLES( LABSUPL1, cur, "Worker" )
{
	v[5] += VS( cur, "_Q" ) * Lscale;
	v[9] += VS( cur, "_B" ) * Lscale;
	
	if ( VS( cur, "_employed" ) > 0 )
	{
		v[6] += VS( cur, "_w" ) * Lscale;

		if ( HOOKS( cur, FWRK )->up == Capital )
			++v[7];
		else
		{
			++v[8];
			
			if ( HOOKS( cur, VWRK ) != NULL && 
				 HOOKS( cur, FWRK )->up != HOOKS( cur, VWRK )->up->up )
				VintErr.push_back( cur );
		}
	}
	
	if ( HOOKS( cur, VWRK ) != NULL && VS( cur, "_Q" ) <= 0 )
		Qerr.push_back( cur );
	
	if ( Tr > 0 && VS( cur, "_age" ) > Tr )
		ageErr.push_back( cur );
	
	if ( VS( cur, "_sT" ) > pow( 1 + tauT, VS( cur, "_age" ) + 1 ) )
		sTerr.push_back( cur );
	
	if ( VS( cur, "_Te" ) > 0 && VS( cur, "_employed" ) == 0 )
		hirErr.push_back( cur );
	
	if ( VS( cur, "_searchProb" ) <= 0 )
		srchErr.push_back( cur );
	
	if ( VS( cur, "_w" ) <= 0 || VS( cur, "_wR" ) <= 0 || 
		 VS( cur, "_wReal" ) <= 0 || VS( cur, "_wRes" ) <= 0 || 
		 VS( cur, "_wS" ) <= 0 )
		wzErr.push_back( cur );
	
	if ( ( VS( cur, "_employed" ) && VS( cur, "_w" ) < wMinPol ) || 
		 VS( cur, "_w" ) < wU )
		wlErr.push_back( cur );
	
	if ( VS( cur, "_w" ) == VLS( cur, "_w", 1 ) * wCap ||
		 VS( cur, "_w" ) == VLS( cur, "_w", 1 ) / wCap )
		 wChgErr.push_back( cur );
	
	if ( VS( cur, "_wR" ) == VLS( cur, "_wR", 1 ) * wCap ||
		 VS( cur, "_wR" ) == VLS( cur, "_wR", 1 ) / wCap )
		 wRerr.push_back( cur );
	
	++k;
}

// count worker bridge objects in both sectors
v[10] = COUNTS( CAPSECL1, "Wrk1" );
CYCLES( CONSECL1, cur, "Firm2" )
	v[11] += COUNTS( cur, "Wrk2" );
		
double Gtrain = VS( LABSUPL1, "Gtrain" );
double L = VS( LABSUPL1, "L" );
double Ls = VS( LABSUPL1, "Ls" );
double Ltrain = VS( LABSUPL1, "Ltrain" );
double Ue = VS( LABSUPL1, "Ue" );
double Us = VS( LABSUPL1, "Us" );
double U = VS( LABSUPL1, "U" );
double TeAvg = VS( LABSUPL1, "TeAvg" );
double appl = VS( LABSUPL1, "appl" );
double dUeB = VS( LABSUPL1, "dUeB" );
double sAvg = VS( LABSUPL1, "sAvg" );
double sTavg = VS( LABSUPL1, "sTavg" );
double sTmax = VS( LABSUPL1, "sTmax" );
double sTmin = VS( LABSUPL1, "sTmin" );
double sTsd = VS( LABSUPL1, "sTsd" );
double sVavg = VS( LABSUPL1, "sVavg" );
double sVsd = VS( LABSUPL1, "sVsd" );
double searchProb = VS( LABSUPL1, "searchProb" );
double wAvg = VS( LABSUPL1, "wAvg" );
double wAvgEmp = VS( LABSUPL1, "wAvgEmp" );
double wCent = VS( LABSUPL1, "wCent" );
double wLogSDemp = VS( LABSUPL1, "wLogSDemp" );

double TuAvg = VS( LABSTAL1, "TuAvg" );
double Vac = VS( LABSTAL1, "V" );
double part = VS( LABSTAL1, "part" );
double w2oMin = VS( LABSTAL1, "w2oMin" );
double wAvgReal = VS( LABSTAL1, "wAvgReal" );
double wGini = VS( LABSTAL1, "wGini" );
double wLogSD = VS( LABSTAL1, "wLogSD" );
double wMax = VS( LABSTAL1, "wMax" );
double wMin = VS( LABSTAL1, "wMin" );
double wrAvg = VS( LABSTAL1, "wrAvg" );
double wrLogSD = VS( LABSTAL1, "wrLogSD" );
double wsAvg = VS( LABSTAL1, "wsAvg" );
double wsLogSD = VS( LABSTAL1, "wsLogSD" );

double L1 = VS( CAPSECL1, "L1" );
double W1 = VS( CAPSECL1, "W1" );
double quits1 = VS( CAPSECL1, "quits1" );
double retires1 = VS( CAPSECL1, "retires1" );

double B2 = VS( CONSECL1, "B2" );
double L2 = VS( CONSECL1, "L2" );		// total workers before firm exits
double Q2e = VS( CONSECL1, "Q2e" );
double W2 = VS( CONSECL1, "W2" );
double quits2 = VS( CONSECL1, "quits2" );
double retires2 = VS( CONSECL1, "retires2" );
double w2oMax = VS( CONSECL1, "w2oMax" );

v[12] = SUMS( CONSECL1, "_L2" );		// total workers after firm exits

double nonNeg[ ] = { Gtrain, Ltrain, TeAvg, U, Ue, Us, sTsd, sVsd, 
					 wAvg, wLogSDemp, wMinPol, wU, Vac, wLogSD, wMax, wMin, 
					 wrLogSD, wsLogSD, L1, W1, quits1, retires1, B2, L2, Q2e, 
					 W2, quits2, retires2, TuAvg };
double posit[ ] = { L, Ls, appl, sAvg, sTavg, sTmax, sTmin, sVavg, 
					searchProb, wAvgEmp, wCent, part, w2oMax, w2oMin, wAvgReal, 
					wGini, wrAvg, wsAvg };
double finite[ ] = { dUeB };

dblVecT all ( nonNeg, END_ARR( nonNeg ) ); 
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// employment summary
LOG( "\n  ++ (t=%d) Ls=%g L=%g Ltrain=%g V=%.2g U=%.2g Us=%.2g Ue=%.2g", 
	 t, Ls, L, Ltrain, Vac, U, Us, Ue );
	 
for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );
	
for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

check_error( Ls < L || Ls < Ltrain, "INCONSISTENT-LABOR", 0, & errors );

check_error( L1 != v[7] * Lscale || L1 != v[10] * Lscale, 
			 "INCONSISTENT-LABOR-S1", 0, & errors );

check_error( L2 < v[8] * Lscale || v[8] != v[11] || v[12] != v[11] * Lscale, 
			 "INCONSISTENT-LABOR-S2", 0, & errors );

check_error( Us > Ue || Ue > U, "INCONSISTENT-UNEMPLOYMENT", 0, & errors );

check_error( wzErr.size( ) > 0, "ZERO-WAGE-WORKERS", wzErr.size( ), & errors ); 

check_error( wlErr.size( ) > 0, "LOW-WAGE-WORKERS", wlErr.size( ), & errors ); 

// labor market
LOG( "\n   + TeAvg=%g TuAvg=%g appl=%.2g quits=%g retires=%g", 
	 round( TeAvg ), round( TuAvg ), appl, round( quits1 + quits2 ), 
	 round( retires1 + retires2 ) );

check_error( ageErr.size( ) > 0, "UNRETIRED-WORKERS", ageErr.size( ), & errors ); 

check_error( hirErr.size( ) > 0, "FAILED-HIRE-WORKERS", hirErr.size( ), & errors ); 

check_error( srchErr.size( ) > 0, "NOT-SEARCH-WORKERS", srchErr.size( ), & errors ); 

// productivity and production
LOG( "\n   + sAvg=%.2g sVavg=%.2g sVsd=%.2g sTavg=%.2g sTsd=%.2g Q=%.3g", 
	 sAvg, sVavg, sVsd, sTavg, sTsd, v[5] );
	
check_error( Qerr.size( ) > 0, "ZERO-PROD-WORKERS", Qerr.size( ), & errors ); 

check_error( sTerr.size( ) > 0, "EXCESS-SKILL-WORKERS", sTerr.size( ), & errors ); 

check_error( VintErr.size( ) > 0, "EXCESS-SKILL-WORKERS", VintErr.size( ), & errors ); 

// wages
LOG( "\n   + wAvg=%.2g wAvgEmp=%.2g wMin=%.2g wMax=%.2g wrAvg=%.2g wsAvg=%.2g", 
	 wAvg, wAvgEmp, wMin, wMax, wrAvg, wsAvg );
	
check_error( round( W1 + W2 + B2 ) < round( v[6] + v[9] ), 
			 "INCONSISTENT-PAYROLL", 0, & errors );

check_error( wAvg > wAvgEmp || wrAvg < wsAvg, "INCONSISTENT-WAGES", 0, & errors );

check_error( wChgErr.size( ) > 0, "LARGE-WAGE-CHANGE", wChgErr.size( ), & errors ); 

check_error( wRerr.size( ) > 0, "LARGE-REQ-WAGE-CHANGE", wRerr.size( ), & errors ); 

if ( t == v[2] )
	LOG( "\n +++ TESTING OF LABOR SUPPLY FINISHED" );

RESULT( errors )


EQUATION( "test1sec" )
/*
Print detailed statistics of capital-good sector (!=0 if error is found)
Set the time range in 'test1StIni' and 'test1StEnd'
*/

v[1] = V( "test1StIni" );
v[2] = V( "test1StEnd" );

if ( t >= v[2] )
	PARAMETER;									// compute for the last time

if ( t < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

if ( t == v[1] )
	LOG( "\n ^^^ TESTING OF CAPITAL-GOOD SECTOR STARTED" );

// scan firms for severe problems
double Lscale = VS( LABSUPL1, "Lscale" );		// labor scaling factor

objVecT Aerr, RDerr, c1err, CliErr;				// vectors to save error firms
int errors = 0;									// error counter
k = v[4] = v[5] = 0;							// accumulators
CYCLES( CAPSECL1, cur, "Firm1" )
{
	v[4] += VS( cur, "_f1" );
	
	if ( VS( cur, "_Atau" ) <= TOL || VS( cur, "_Btau" ) <= TOL )
		Aerr.push_back( cur );
	
	if ( VS( cur, "_RD" ) < 0 )
		RDerr.push_back( cur );
	
	if ( VS( cur, "_c1" ) <= 0 || VS( cur, "_p1" ) <= 0 )
		c1err.push_back( cur );
	
	if ( VS( cur, "_t1ent" ) > t && VS( cur, "_HC" ) + VS( cur, "_NC" ) <= 0 )
		CliErr.push_back( cur );
	
	++k;
}

CYCLES( CAPSECL1, cur1, "Wrk1" )
	v[5] += VS( SHOOKS( cur1 ), "_w" ) * Lscale;

double A1 = VS( CAPSECL1, "A1" );
double D1 = VS( CAPSECL1, "D1" );
double Deb1 = VS( CAPSECL1, "Deb1" );
double Div1 = VS( CAPSECL1, "Div1" );
double F1 = VS( CAPSECL1, "F1" );
double JO1 = VS( CAPSECL1, "JO1" );
double L1 = VS( CAPSECL1, "L1" );
double L1d = VS( CAPSECL1, "L1d" );
double L1rd = VS( CAPSECL1, "L1rd" );
double MC1 = VS( CAPSECL1, "MC1" );
double NW1 = VS( CAPSECL1, "NW1" );
double PPI = VS( CAPSECL1, "PPI" );
double Pi1 = VS( CAPSECL1, "Pi1" );
double Q1 = VS( CAPSECL1, "Q1" );
double Q1e = VS( CAPSECL1, "Q1e" );
double S1 = VS( CAPSECL1, "S1" );
double Tax1 = VS( CAPSECL1, "Tax1" );
double W1 = VS( CAPSECL1, "W1" );
double entry1exit = VS( CAPSECL1, "entry1exit" );
double fires1 = VS( CAPSECL1, "fires1" );
double hires1 = VS( CAPSECL1, "hires1" );
double imi = VS( CAPSECL1, "imi" );
double inn = VS( CAPSECL1, "inn" );
double quits1 = VS( CAPSECL1, "quits1" );
double retires1 = VS( CAPSECL1, "retires1" );
double sT1min = VS( CAPSECL1, "sT1min" );
double w1avg = VS( CAPSECL1, "w1avg" );

double HH1 = VS( SECSTAL1, "HH1" );
double HP1 = VS( SECSTAL1, "HP1" );
double RD = VS( SECSTAL1, "RD" );
double age1avg = VS( SECSTAL1, "age1avg" );
double cred1c = VS( SECSTAL1, "cred1c" );

double nonNeg[ ] = { D1, Deb1, Div1, JO1, L1, L1d, L1rd, Q1, Q1e, S1, Tax1, W1, 
					 fires1, hires1, imi, inn, quits1, retires1, HH1, HP1, RD,
					 age1avg, cred1c };
double posit[ ] = { A1, F1, PPI, sT1min, w1avg };
double finite[ ] = { NW1, entry1exit, Pi1 };

dblVecT all ( nonNeg, END_ARR( nonNeg ) ); 
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// innovation, productivity
LOG( "\n  ^^ (t=%d) F1=%g inn=%g Imi=%g A1=%.3g D1=%.3g Q1=%.3g Q1e=%.3g", 
	 t, F1, inn, imi, A1, D1, Q1, Q1e );
	 
for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );
	
for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", 
				 itd - all.begin( ) + 1, & errors );

check_error( Aerr.size( ) > 0, "ZERO-PROD-FIRMS", Aerr.size( ), & errors ); 

check_error( RDerr.size( ) > 0, "ZERO-RD-FIRMS", RDerr.size( ), & errors ); 

check_error( Q1e > Q1 || Q1 > D1, "INCONSISTENT-PRODUCTION", 0, & errors );

// labor
LOG( "\n   ^ JO1=%g L1d=%g L1=%g L1rd=%g ret1=%g quit1=%g fire1=%g hire1=%g", 
	 JO1, L1d, L1, L1rd, retires1, quits1, fires1, hires1 );

check_error( L1rd > L1 || L1d < JO1 || hires1 > L1d + quits1 + retires1, 
			 "INCONSISTENT-LABOR", 0, & errors );

// cash flow
LOG( "\n   ^ S1=%.3g W1=%.3g Tax1=%.3g Pi1=%.3g NW1=%.3g", 
	 S1, W1, Tax1, Pi1, NW1 );

check_error( round( W1 ) != round( v[5] ), "INCONSISTENT-PAYROLL", 0, & errors ); 

check_error( S1 + RD < W1, "INCONSISTENT-WAGES", 0, & errors );

check_error( c1err.size( ) > 0, "ZERO-COST-FIRM", c1err.size( ), & errors ); 

// finance
LOG( "\n   ^ Deb1=%.3g cred1c=%.3g PPI=%.2g", 
	 Deb1, cred1c, PPI );
	
// competition
LOG( "\n   ^ age1avg=%.3g sT1min=%.2g MC1=%.2g entry1exit=%g HH1=%.2g HP1=%.2g", 
	 age1avg, sT1min, MC1, entry1exit, HH1, HP1 );

check_error( v[4] < 1 - TOL / 10 || v[4] >  1 + TOL / 10, 
			 "INCONSISTENT-SHARES", 0, & errors );

check_error( CliErr.size( ) > 0, "NO-CLIENT-FIRMS", CliErr.size( ), & errors ); 

check_error( HH1 > 1 || HP1 > 2, "INCONSISTENT-STATS", 0, & errors );

if ( t == v[2] )
	LOG( "\n ^^^ TESTING OF CAPITAL-GOOD SECTOR FINISHED" );

RESULT( errors )


EQUATION( "test2sec" )
/*
Print detailed statistics of consumption-good sector (!=0 if error is found)
Set the time range in 'test2StIni' and 'test2StEnd'
*/

v[1] = V( "test2StIni" );
v[2] = V( "test2StEnd" );

if ( t >= v[2] )
	PARAMETER;									// compute for the last time

if ( t < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

h = v[3] = 1 + v[2] - v[1];						// number of periods

if ( t == v[1] )
	LOG( "\n &&& TESTING OF CONSUMPTION-GOOD SECTOR STARTED" );

// scan firms for severe problems
double Lscale = VS( LABSUPL1, "Lscale" );		// labor scaling factor
double mu20 = VS( CONSECL1, "mu20" );			// initial mark-up
double wCap = VS( LABSUPL1, "wCap" );			// wage cap

objVecT B2err, D2err, Kerr, L2err, Q2err, W2err,// vectors to save error firms
		c2err, mu2err, w2oErr;	
		
int errors = 0;									// error counter
k = v[4] = v[5] = 0;							// accumulators
CYCLES( CONSECL1, cur, "Firm2" )
{
	v[4] += VS( cur, "_f2" );
	v[5] += ( VS( cur, "_life2cycle" ) > 0 && VS( cur, "_L2" ) <= 0 ) ? 1 : 0;
	
	v[6] = v[7] = 0;
	CYCLES( cur, cur1, "Wrk2" )
	{
		v[6] += VS( SHOOKS( cur1 ), "_w" ) * Lscale;
		v[7] += VS( SHOOKS( cur1 ), "_B" ) * Lscale;
	}
		
	if ( round( VS( cur, "_W2" ) ) != round( v[6] ) )
		W2err.push_back( cur );
	
	if ( round( VS( cur, "_B2" ) ) != round( v[7] ) )
		B2err.push_back( cur );
	
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
	
	if ( VS( cur, "_w2o" ) == VLS( cur, "_w2o", 1 ) * wCap ||
		 VS( cur, "_w2o" ) == VLS( cur, "_w2o", 1 ) / wCap )
		 w2oErr.push_back( cur );
	
	++k;
}

double A2 = VS( CONSECL1, "A2" );
double A2p = VS( CONSECL1, "A2p" );
double B2 = VS( CONSECL1, "B2" );
double CI = VS( CONSECL1, "CI" );
double CPI = VS( CONSECL1, "CPI" );
double D2 = VS( CONSECL1, "D2" );
double D2d = VS( CONSECL1, "D2d" );
double D2e = VS( CONSECL1, "D2e" );
double Deb2 = VS( CONSECL1, "Deb2" );
double Div2 = VS( CONSECL1, "Div2" );
double EI = VS( CONSECL1, "EI" );
double F2 = VS( CONSECL1, "F2" );
double JO2 = VS( CONSECL1, "JO2" );
double K = VS( CONSECL1, "K" );
double Kd = VS( CONSECL1, "Kd" );
double L2 = VS( CONSECL1, "L2" );
double L2d = VS( CONSECL1, "L2d" );
double MC2 = VS( CONSECL1, "MC2" );
double NW2 = VS( CONSECL1, "NW2" );
double Pi2 = VS( CONSECL1, "Pi2" );
double Pi2rateAvg = VS( CONSECL1, "Pi2rateAvg" );
double Q2 = VS( CONSECL1, "Q2" );
double Q2d = VS( CONSECL1, "Q2d" );
double Q2e = VS( CONSECL1, "Q2e" );
double Q2u = VS( CONSECL1, "Q2u" );
double S2 = VS( CONSECL1, "S2" );
double SI = VS( CONSECL1, "SI" );
double Tax2 = VS( CONSECL1, "Tax2" );
double W2 = VS( CONSECL1, "W2" );
double c2 = VS( CONSECL1, "c2" );
double c2e = VS( CONSECL1, "c2e" );
double dCPI = VS( CONSECL1, "dCPI" );
double entry2exit = VS( CONSECL1, "entry2exit" );
double fires2 = VS( CONSECL1, "fires2" );
double hires2 = VS( CONSECL1, "hires2" );
double l2max = VS( CONSECL1, "l2max" );
double l2min = VS( CONSECL1, "l2min" );
double oldVint = VS( CONSECL1, "oldVint" );
double p2max = VS( CONSECL1, "p2max" );
double p2min = VS( CONSECL1, "p2min" );
double q2max = VS( CONSECL1, "q2max" );
double q2min = VS( CONSECL1, "q2min" );
double quits2 = VS( CONSECL1, "quits2" );
double retires2 = VS( CONSECL1, "retires2" );
double sV2avg = VS( CONSECL1, "sV2avg" );
double w2avg = VS( CONSECL1, "w2avg" );
double w2oAvg = VS( CONSECL1, "w2oAvg" );

double Kavb = VLS( CONSECL1, "K", 1 );

double HH2 = VS( SECSTAL1, "HH2" );
double HP2 = VS( SECSTAL1, "HP2" );
double age2avg = VS( SECSTAL1, "age2avg" );
double cred2c = VS( SECSTAL1, "cred2c" );
double mu2avg = VS( SECSTAL1, "mu2avg" );
double noWrk2 = VS( SECSTAL1, "noWrk2" );//
double s2avg = VS( SECSTAL1, "s2avg" );

double nonNeg[ ] = { B2, CPI, D2, D2d, D2e, Deb2, Div2, JO2, Kavb, L2, L2d, Q2, 
					 Q2e, SI, Tax2, W2, fires2, hires2, quits2, retires2, l2max, 
					 l2min, CI, EI, HH2, HP2, K, Kd, Q2d, Q2u, S2, cred2c, 
					 noWrk2 };
double posit[ ] = { A2, A2p, F2, Pi2rateAvg, oldVint, p2max, p2min, q2max, 
					q2min, s2avg, sV2avg, w2avg, w2oAvg, age2avg, mu2avg };
double finite[ ] = { NW2, dCPI, entry2exit, Pi2 };

dblVecT all ( nonNeg, END_ARR( nonNeg ) ); 
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// capital and investment
LOG( "\n  && (t=%d) F2=%g Kd=%.3g Kavb=%.3g K=%.3g", 
	 t, F2, Kd, Kavb, K );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );
	
for ( i = 0; i < LEN_ARR( posit ); ++i )
	check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( itd = all.begin( ); itd != all.end( ); ++itd )
	check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

check_error( Kerr.size( ) > 0, "NO-CAPITAL-FIRMS", Kerr.size( ), & errors ); 

LOG( "\n   & EI=%.3g SI=%.3g CI=%.3g", 
	 EI, SI, CI );

check_error( SI > Kavb || K > Kavb + EI + SI, 
			 "INCONSISTENT-CAPITAL", 0, & errors );
		 
// productivity
LOG( "\n   & A2p=%.3g A2=%.3g c2=%.3g c2e=%.3g sV2avg=%.2g", 
	 A2p, A2, c2, c2e, sV2avg );
	 
check_error( A2 > ( 1 + TOL ) * A2p, "INCONSISTENT-PRODUCTIVITY", 0, & errors );		 

check_error( c2e < ( 1 - TOL ) * c2, "INCONSISTENT-UNIT-COST", 0, & errors );		 

// production
LOG( "\n   & Q2d=%.3g Q2=%.3g Q2e=%.3g s2avg=%.2g", 
	 Q2d, Q2, Q2e, s2avg );
	 
check_error( Q2e > Q2 || Q2 > Q2d || Q2e > K * s2avg, 
			 "INCONSISTENT-PRODUCTION", 0, & errors );

check_error( c2err.size( ) > 0, "ZERO-COST-FIRMS", c2err.size( ), & errors ); 

check_error( Q2err.size( ) > 0, "NO-PROD-FIRMS", Q2err.size( ), & errors ); 

// labor
LOG( "\n   & JO2=%g L2d=%g L2=%g ret2=%g quit2=%g fire2=%g hire2=%g", 
	 JO2, L2d, L2, retires2, quits2, fires2, hires2 );

check_error( L2d < JO2 || hires2 > L2d + quits2 + retires2, 
			 "INCONSISTENT-LABOR", 0, & errors );

check_error( L2err.size( ) > 0, "NO-LABOR-PRODUCING", L2err.size( ), & errors ); 

check_error( w2oErr.size( ) > 0, "LARGE-WAGE-OFFER-CHANGE", w2oErr.size( ), & errors ); 

// cash flow
LOG( "\n   & W2=%.3g B2=%.3g Tax2=%.3g Pi2=%.3g Pi2rAvg=%.2g", 
	 W2, B2, Tax2, Pi2, Pi2rateAvg );

check_error( W2err.size( ) > 0, "WAGES-GAP", W2err.size( ), & errors ); 

check_error( B2err.size( ) > 0, "BONUSES-GAP", B2err.size( ), & errors ); 

check_error( Pi2rateAvg > 1, "INCONSISTENT-PROFITS", 0, & errors );

check_error( S2 < W2 + B2, "INCONSISTENT-WAGES", 0, & errors );

// finance
LOG( "\n   & NW2=%.3g Deb2=%.3g cred2c=%.3g CPI=%.2g noWrk2=%.2g", 
	 NW2, Deb2, cred2c, CPI, noWrk2 );
	 
check_error( v[5] / F2 > TOL || noWrk2 > TOL, "MANY-NO-WORKER", 0, & errors );

// market
LOG( "\n   & mu2avg=%.2g p2min=%.2g p2max=%.2g D2=%.3g D2e=%.3g S2=%.3g", 
	 mu2avg, p2min, p2max, D2, D2e, S2 );

check_error( D2 > D2d, "INCONSISTENT-DEMAND", 0, & errors );
	 
check_error( D2err.size( ) > 0, "NO-DEMAND-FIRMS", D2err.size( ), & errors ); 

check_error( mu2err.size( ) > 0, "BAD-MARKUP-FIRMS", mu2err.size( ), & errors ); 

// competition
LOG( "\n   & age2avg=%g MC2=%.2g entry2exit=%g HH2=%.2g HP2=%.2g", 
	 age2avg, MC2, entry2exit, HH2, HP2 );
LOG( "\n   & q2min=%.2g q2max=%.2g l2min=%.2g l2max=%.2g", 
	 q2min, q2max, l2min, l2max );

check_error( v[4] < 1 - TOL / 10 || v[4] > 1 + TOL / 10, 
			 "INCONSISTENT-SHARES", 0, & errors );

check_error( HH2 > 1 || HP2 > 2, "INCONSISTENT-STATS", 0, & errors );

if ( t == v[2] )
	LOG( "\n &&& TESTING OF CONSUMPTION-GOOD SECTOR FINISHED" );

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

if ( t >= v[2] )
	PARAMETER;									// compute for the last time

if ( t < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

v[3] = V( "test1idIni" );
v[4] = V( "test1idEnd" );

h = v[5] = 1 + v[2] - v[1];						// number of periods
k = v[6] = v[3] > 0 ? 1 + v[4] - v[3] : v[4];	// number of firms

static double iniAtau, iniBtau;
static firmMapT entr;

if ( t == v[1] )
{
	LOG( "\n *** TESTING OF CAPITAL-GOOD FIRMS STARTED" );

	if ( v[3] > 0 )
		LOG( " (t=%g-%g, ID1=%g-%g, file=%s)", 
			 v[1], v[2], v[3], v[4], TEST1FILE )
	else
		LOG( " (t=%g-%g, up to %g entrants, file=%s)", 
			 v[1], v[2], v[4], TEST1FILE )
	
	if ( firms1 == NULL )						// don't reopen if already open
	{
		firms1 = fopen( TEST1FILE, "w" );		// (re)create the file
		fprintf( firms1, "%s,%s,%s,%s\n",		// file header
				 "t,ID1,t1ent,Client",
				 "RD,Atau,Btau,c1,D1,Q1,Q1e,L1d,L1",
				 "Pi1,NW1,Deb1,Deb1max,cred1,cred1c",
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
	double _D1 = VS( cur, "_D1" );
	double _Deb1 = VS( cur, "_Deb1" );
	double _Deb1max = VS( cur, "_Deb1max" );
	double _HC = VS( cur, "_HC" );
	double _L1d = VS( cur, "_L1d" );
	double _L1 = VS( cur, "_L1" );
	double _NC = VS( cur, "_NC" );
	double _NW1 = VS( cur, "_NW1" );
	double _Pi1 = VS( cur, "_Pi1" );
	double _Q1 = VS( cur, "_Q1" );
	double _Q1e = VS( cur, "_Q1e" );
	double _RD = VS( cur, "_RD" );
	double _S1 = VS( cur, "_S1" );
	double _c1 = VS( cur, "_c1" );
	double _cred1 = VS( cur, "_cred1" );
	double _cred1c = VS( cur, "_cred1c" );
	double _f1 = VS( cur, "_f1" );
	double _p1 = VS( cur, "_p1" );
			  
	double nonNeg[ ] = { _HC, _NC, _RD, _D1, _Q1, _Q1e, _BC, _L1d, _Deb1, _Deb1max, 
						 _cred1, _cred1c, _S1, _f1 };
	double posit[ ] = { _Atau, _Btau, _c1, _p1 };
	double finite[ ] = { _NW1, _Pi1 };

	dblVecT all ( nonNeg, END_ARR( nonNeg ) ); 
	all.insert( all.end( ), posit, END_ARR( posit ) );
	all.insert( all.end( ), finite, END_ARR( finite ) );

	// first period actions (single-firm analysis only)
	if ( k == 1 && t == v[1] )
	{
		iniAtau = _Atau;
		iniBtau = _Btau;
	}
	
	LOG( "\n  ** (t=%d) ID1=%d t1ent=%d #Client=%d:", t, i, h, j );
	fprintf( firms1, "%d,%d,%d,%d", t, i, h, j );
	
	for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
		check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );
		
	for ( i = 0; i < LEN_ARR( posit ); ++i )
		check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

	for ( itd = all.begin( ); itd != all.end( ); ++itd )
		check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

	// innovation, productivity
	LOG( "\n   * RD=%g Atau=%.3g Btau=%.3g c1=%.3g D1=%g Q1=%g Q1e=%g L1d=%g L1=%g", 
		 round( _RD ), _Atau, _Btau, _c1, _D1, _Q1, _Q1e, _L1d, _L1 ); 
	fprintf( firms1, ",%g,%g,%g,%g,%g,%g,%g,%g,%g",
			 _RD, _Atau, _Btau, _c1, _D1, _Q1, _Q1e, _L1d, _L1 );
		 
	check_error( _RD <= 0, "NO-R&D", 0, & errors );

	check_error( _Atau < TOL || _Btau < TOL, "INCONSISTENT-PRODUCTIVITY", 0, & errors );

	// finance
	LOG( "\n   * Pi1=%g NW1=%g Deb1=%g Deb1max=%g cred1=%g cred1c=%g", 
		 round( _Pi1 ), round( _NW1 ), round( _Deb1 ), round( _Deb1max ), 
		 round( _cred1 ), round( _cred1c ) );
	fprintf( firms1, ",%g,%g,%g,%g,%g,%g", 
			 _Pi1, _NW1, _Deb1, _Deb1max, _cred1, _cred1c );
	
	// market and client dynamics
	LOG( "\n   * HC=%g NC=%g BC=%g p1=%.2g S1=%g f1=%.2g", 
		 _HC, _NC, _BC, _p1, round( _S1 ), _f1 );
	fprintf( firms1, ",%g,%g,%g,%g,%g,%g", 
			 _HC, _NC, _BC, _p1, _S1, _f1 );

	check_error( j > _HC + _NC, "INCONSISTENT-CLIENTS", 0, & errors );

	check_error( j < _BC, "INCONSISTENT-BUYERS", 0, & errors );

	// last period actions (single-firm analysis only)
	if ( k == 1 && t == v[2] )
	{
		LOG( "\n   * AtauGwth=%.3g BtauGwth=%.3g", 
			 ( log( _Atau + 1 ) - log( iniAtau + 1 ) ) / v[5], 
			 ( log( _Btau + 1 ) - log( iniBtau + 1 ) ) / v[5] );
	}
	
	fputs( "\n", firms1 );
}

if ( t == v[2] )
{
	LOG( "\n *** TESTING OF CAPITAL-GOOD FIRMS FINISHED" );
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

if ( t >= v[2] )
	PARAMETER;									// compute for the last time

if ( t < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

v[3] = V( "test2idIni" );
v[4] = V( "test2idEnd" );

h = v[5] = 1 + v[2] - v[1];						// number of periods
k = v[6] = v[3] > 0 ? 1 + v[4] - v[3] : v[4];	// number of firms

static double iniK, iniNW2, iniDeb2; 
static firmMapT entr;

if ( t == v[1] )
{
	LOG( "\n ### TESTING OF CONSUMER-GOOD FIRMS STARTED" );
	
	if ( v[3] > 0 )
		LOG( " (t=%g-%g, ID2=%g-%g, file=%s)", 
			 v[1], v[2], v[3], v[4], TEST2FILE )
	else
		LOG( " (t=%g-%g, up to %g entrants, file=%s)", 
			 v[1], v[2], v[4], TEST2FILE )
	
	if ( firms2 == NULL )						// don't reopen if already open
	{
		firms2 = fopen( TEST2FILE, "w" );		// (re)create the file
		fprintf( firms2, "%s,%s,%s,%s\n",		// file header
				 "t,ID2,t2ent,life2cycle,Broch,Vint,Wrk",
				 "Kd,Kavb,EId,SId,EI,SI,CI,K,Q2d,Q2,Q2e,L2d,L2,c2,s2avg",
				 "Pi2,NW2,Deb2,Deb2max,cred2,cred2c",
				 "mu2,p2,D2,S2,W2+B2,f2,N" );
	}
}

double Lscale = VS( LABSUPL1, "Lscale" );		// labor scale
double m2 = VS( CONSECL1, "m2" );				// machine scale
double mu20 = VS( CONSECL1, "mu20" );			// initial mark-up
double wCap = VS( LABSUPL1, "wCap" );			// wage cap

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

	objVecT IDerr, tVintErr, LvintErr, QvintErr,// vectors to save error firms
			dLvintErr, noWrkVintErr;			
	
	v[7] = v[8] = v[9] = v[10] = v[11] = 0;		// accumulators
	v[12] = v[13] = v[14] = v[15] = v[16] = v[17] = 0;
	cur2 = NULL;								// last vintage
	CYCLES( cur, cur1, "Vint" )
	{
		i = COUNTS( cur1, "WrkV" ) * Lscale;
		
		v[7] += VS( cur1, "_nVint" );
		v[8] += VS( cur1, "_Qvint" );
		v[9] += i; 
		
		v[99] = 0;
		CYCLES( cur1, cur2, "WrkV" )
		{
			v[99] += VS( SHOOKS( cur2 ), "_Q" ) * Lscale;
			v[10] += VS( SHOOKS( cur2 ), "_Q" ) * Lscale;
			v[11] += VS( SHOOKS( cur2 ), "_s" ) * Lscale;
			v[12] += VS( SHOOKS( cur2 ), "_w" ) * Lscale;
			v[13] += VS( SHOOKS( cur2 ), "_B" ) * Lscale;
		}
		
		if ( VS( cur1, "_IDvint" ) < h )
			IDerr.push_back( cur1 );
		
		if ( VS( cur1, "_tVint" ) < h || VS( cur1, "_tVint" ) > t )
			tVintErr.push_back( cur1 );
		
		if ( i < 0 || i > _L2 )
			LvintErr.push_back( cur1 );
		
		if ( VS( cur1, "_Qvint" ) < 0 || 
			 ( i > 0 && VS( cur1, "_Qvint" ) <= 0 ) ||
			 round( VS( cur1, "_Qvint" ) ) != round( v[99] ) )
			QvintErr.push_back( cur1 );
			
		if ( VS( cur1, "_toUseVint" ) == 0 && VS( cur1, "_dLdVint" ) > 0 )
			dLvintErr.push_back( cur1 );
		
		if ( cur2 != NULL && 
			 VS( cur1, "_Qvint" ) > 0 && VS( cur2, "_Qvint" ) == 0 )
			noWrkVintErr.push_back( cur1 );
		
		cur2 = cur1;
	}
	
	CYCLES( cur, cur2, "Wrk2" )
	{
		v[14] += VS( SHOOKS( cur2 ), "_Q" ) * Lscale;
		v[15] += VS( SHOOKS( cur2 ), "_s" ) * Lscale;
		v[16] += VS( SHOOKS( cur2 ), "_w" ) * Lscale;
		v[17] += VS( SHOOKS( cur2 ), "_B" ) * Lscale;
	}
	
	v[18] = COUNTS( cur, "Broch" );
	v[19] = COUNTS( cur, "Vint" );
	v[20] = COUNTS( cur, "Wrk2" ) * Lscale;
	
	double _B2 = VS( cur, "_B2" );
	double _CI = VS( cur, "_CI" );
	double _D2 = VS( cur, "_D2" );
	double _Deb2 = VS( cur, "_Deb2" );
	double _Deb2max = VS( cur, "_Deb2max" );
	double _EI = VS( cur, "_EI" );
	double _EId = VS( cur, "_EId" );
	double _K = VS( cur, "_K" );
	double _Kd = VS( cur, "_Kd" );
	double _L2d = VS( cur, "_L2d" );
	double _N = VS( cur, "_N" );
	double _NW2 = VS( cur, "_NW2" );
	double _Pi2 = VS( cur, "_Pi2" );
	double _Q2d = VS( cur, "_Q2d" );
	double _Q2 = VS( cur, "_Q2" );
	double _S2 = VS( cur, "_S2" );
	double _SI = VS( cur, "_SI" );
	double _SId = VS( cur, "_SId" );
	double _W2 = VS( cur, "_W2" );
	double _c2 = VS( cur, "_c2" );
	double _c2e = VS( cur, "_c2e" );
	double _cred2 = VS( cur, "_cred2" );
	double _cred2c = VS( cur, "_cred2c" );
	double _f2 = VS( cur, "_f2" );
	double _life2cycle = VS( cur, "_life2cycle" );
	double _mu2 = VS( cur, "_mu2" );
	double _p2 = VS( cur, "_p2" );
	double _s2avg = VS( cur, "_s2avg" );
	double _t2ent = VS( cur, "_t2ent" );
	double _w2o = VS( cur, "_w2o" );

	double _Kavb = VLS( cur, "_K", 1 );
	double _w2oPast = VLS( cur, "_w2o", 1 );
	
	double nonNeg[ ] = { _B2, _CI, _D2, _Deb2, _Deb2max, _EI, _EId, _K, _Kavb, 
						 _Kd, _L2, _L2d, _N, _Q2d, _Q2, _Q2e, _S2, _SI, _SId, 
						 _W2, _cred2, _cred2c, _c2, _c2e, _f2, _life2cycle, 
						 _s2avg };
	double posit[ ] = { _mu2, _p2, _t2ent, _w2o };
	double finite[ ] = { _NW2, _Pi2 };

	dblVecT all ( nonNeg, END_ARR( nonNeg ) ); 
	all.insert( all.end( ), posit, END_ARR( posit ) );
	all.insert( all.end( ), finite, END_ARR( finite ) );

	// first period actions (single-firm analysis only)
	if ( k == 1 && t == v[1] )
	{
		iniK = _K;
		iniNW2 = _NW2;
		iniDeb2 = _Deb2;
	}
	
	LOG( "\n  ## (t=%d) ID2=%d t2ent=%d life2cycle=%g #Broch=%g #Vint=%g #Wrk=%g", 
		 t, j, h, _life2cycle, v[18], v[19], v[20] );
	fprintf( firms2, "%d,%d,%d,%g,%g,%g,%g", t, j, h, _life2cycle, v[18], v[19], v[20] );
	
	for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
		check_error( nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );
		
	for ( i = 0; i < LEN_ARR( posit ); ++i )
		check_error( posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

	for ( itd = all.begin( ); itd != all.end( ); ++itd )
		check_error( ! isfinite( *itd ), "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

	check_error( v[18] == 0, "NO-BROCHURE", 0, & errors ); 
	
	// capital and investment
	LOG( "\n   # Kd=%g Kavb=%g EId=%g SId=%g EI=%g SI=%g CI=%g K=%g", 
		 _Kd, _Kavb, _EId, _SId, _EI, _SI, _CI, _K );
		 
	check_error( _life2cycle > 0 && v[19] == 0, "NO-VINTAGE", 0, & errors ); 
	
	check_error( IDerr.size( ) > 0, "INVALID-ID-VINT", IDerr.size( ), & errors ); 
	
	check_error( tVintErr.size( ) > 0, "INVALID-T-VINT", tVintErr.size( ), & errors ); 
	
	check_error( noWrkVintErr.size( ) > 0, "INCONSISTENT-VINT-PROD", 
				 noWrkVintErr.size( ), & errors ); 
	
	check_error( _SId > _Kavb || _K > _Kavb + _EI + _SI || v[7] * m2 > _K ||
				 ( _life2cycle > 0 && _K == 0 ), 
				 "INCONSISTENT-CAPITAL", 0, & errors );
		 
	LOG( "\n   # Q2d=%g Q2=%g Q2e=%g L2d=%g L2=%g c2=%.2g s2avg=%.2g", 
		 _Q2d, _Q2, round( _Q2e ), _L2d, _L2, _c2, _s2avg );
	fprintf( firms2, ",%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
		 _Kd, _Kavb, _EId, _SId, _EI, _SI, _CI, _K, _Q2d, _Q2, 
		 _Q2e, _L2d, _L2, _c2, _s2avg );
		 
	check_error( _Q2 > _Q2d || _Q2e > _K * _s2avg || _Q2e > _Q2, 
				 "INCONSISTENT-PRODUCTION", 0, & errors );

	check_error( QvintErr.size( ) > 0, "INVALID-PROD-VINT", QvintErr.size( ), & errors ); 

	check_error( _L2d > 0 && _L2 == 0, "NO-WORKER", 0, & errors );

	check_error( v[9] > _L2 || v[9] > v[20], "INCONSISTENT-WORKER", 0, & errors );

	check_error( LvintErr.size( ) > 0, "INVALID-LABOR-VINT", LvintErr.size( ), & errors ); 
	
	check_error( dLvintErr.size( ) > 0, "INVALID-VINT-DEMAND", 
				 dLvintErr.size( ), & errors ); 
	
	check_error( _life2cycle > 0 && ( _c2 == 0 || _c2e == 0 ), 
				 "INCONSISTENT-COST", 0, & errors );

	check_error( v[9] > 0 && ( v[11] / v[9] < _s2avg * ( 1 - TOL ) || 
							   v[11] / v[9] > _s2avg * ( 1 + TOL ) ), 
				 "INCONSISTENT-SKILLS", 0, & errors );

	check_error( _w2o == _w2oPast * wCap || _w2o == _w2oPast / wCap, 
				 "LARGE-WAGE-OFFER-CHANGE", 0, & errors );

	check_error( round( v[10] ) != round( v[14] ), 
				 "INCONSISTENT-WORKER-PROD", 0, & errors );
	
	// finance
	LOG( "\n   # Pi2=%g NW2=%g Deb2=%g Deb2max=%g cred2=%g cred2c=%g", 
		 round( _Pi2 ), round( _NW2 ), round( _Deb2 ), round( _Deb2max ), 
		 round( _cred2 ), round( _cred2c ) );
	fprintf( firms2, ",%g,%g,%g,%g,%g,%g", 
			 _Pi2, _NW2, _Deb2, _Deb2max, _cred2, _cred2c );
	
	// market
	LOG( "\n   # mu2=%.2g p2=%.2g D2=%g S2=%g W2+B2=%g f2e=%.2g N=%g", _mu2, _p2, 
		 round( _D2 ), round( _S2 ), round( _W2 + _B2 ), _f2, round( _N ) );
	fprintf( firms2, ",%g,%g,%g,%g,%g,%g,%g", _mu2, _p2, _D2, _S2, _W2 + _B2, _f2, _N );
		  
	check_error( _mu2 > 5 * mu20, "EXCESSIVE-MARKUP", 0, & errors );

	check_error( _mu2 < mu20 / 5, "INSUFFICIENT-MARKUP", 0, & errors );
		  
	check_error( round( v[12] + v[13] ) > round( v[16] + v[17] ) ||
				 round( _W2 + _B2 ) != round( v[16] + v[17] ), 
				 "INCONSISTENT-PAYROLL", 0, & errors ); 

	// last period actions (single-firm analysis only)
	if ( k == 1 && t == v[2] )
	{
		LOG( "\n   # Kgwth=%.3g NW2gwth=%.3g Deb2gwth=%.3g",
			 ( log( _K + 1 ) - log( iniK + 1 ) ) / v[5], 
			 ( log( _NW2 + 1 ) - log( iniNW2 + 1 ) ) / v[5], 
			 ( log( _Deb2 + 1 ) - log( iniDeb2 + 1 ) ) / v[5] );
	}
		  
	fputs( "\n", firms2 );
}

if ( t == v[2] )
{
	LOG( "\n ### TESTING OF CONSUMER-GOOD FIRMS FINISHED" );
	fclose( firms2 );
	firms2 = NULL;
}

RESULT( errors )
