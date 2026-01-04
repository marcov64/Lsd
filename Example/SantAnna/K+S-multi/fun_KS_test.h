/******************************************************************************

	TEST EQUATIONS
	--------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

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

if ( T == 1 )
	PLOG( "\n Optional statistics being computed in object 'Stats'" );

v[1] = V( "testCtIni" );
v[2] = V( "testCtEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

v[3] = 1 + v[2] - v[1];							// number of periods

static double iniGDP, iniA, iniDeb, iniSavBas, iniSavLux, lastID2 = 0, numInd = 0;

if ( T == v[1] )
	LOG( "\n @@@ TESTING OF COUNTRY MACRO STARTED" );

int errors = 0;									// error counter
v[4] = v[5] = v[6] = v[7] = v[8] = v[9] = 0;	// accumulators
v[10] = v[11] = v[12] = v[13] = v[14] = v[15] = v[16] = v[17] = 0;
CYCLES( PARENT, cur, "Consumption" )
{
	v[4] = max( VS( cur, "ID2" ), v[4] );
	v[5] += VS( cur, "f2" );
	v[6] += VS( cur, "L2" );
	v[7] += VS( cur, "D2" );
	v[8] += VS( cur, "D2a" );
	v[9] += VS( cur, "Q2e" );
	v[10] += VS( cur, "EI2" );
	v[11] += VS( cur, "SI2" );
	v[12] += VS( cur, "S2" );

	if ( VS( cur, "type2" ) == 0 )
	{
		v[13] += VS( cur, "S2" );
		v[15] += VS( cur, "D2a" );
	}
	else
	{
		v[14] += VS( cur, "S2" );
		v[16] += VS( cur, "D2a" );
	}

	++v[17];
}

double A = VS( PARENT, "A" );
double C = VS( PARENT, "C" );
double CdBas = VS( PARENT, "CdBas" );
double CdLux = VS( PARENT, "CdLux" );
double Creal = VS( PARENT, "Creal" );
double CPI = VS( PARENT, "CPI" );
double DcBas = VS( PARENT, "DcBas" );
double DcLux = VS( PARENT, "DcLux" );
double Deb = VS( PARENT, "Deb" );
double Def = VS( PARENT, "Def" );
double Gcons = VS( PARENT, "Gcons" );
double Gtrf = VS( PARENT, "Gtrf" );
double Gd = VS( PARENT, "Gd" );
double GDPnom = VS( PARENT, "GDPnom" );
double GDPreal = VS( PARENT, "GDPreal" );
double I = VS( PARENT, "I" );
double Inom = VS( PARENT, "Inom" );
double Ireal = VS( PARENT, "Ireal" );
double PPI = VS( PARENT, "PPI" );
double SavBas = VS( PARENT, "SavBas" );
double SavForc = VS( PARENT, "SavForc" );
double SavLux = VS( PARENT, "SavLux" );
double Sc = VS( PARENT, "Sc" );
double ScBas = VS( PARENT, "ScBas" );
double ScLux = VS( PARENT, "ScLux" );
double Tax = VS( PARENT, "Tax" );
double cEntry = VS( PARENT, "cEntry" );
double cExit = VS( PARENT, "cExit" );
double dCPIb = VS( PARENT, "dCPIb" );
double dGDPreal = VS( PARENT, "dGDPreal" );
double dNnom = VS( PARENT, "dNnom" );
double entryExit = VS( PARENT, "entryExit" );
double pC0 = VS( PARENT, "pC0" );
double pK0 = VS( PARENT, "pK0" );

double CPI_1 = VLS( PARENT, "CPI", 1 );
double SavBas_1 = VLS( PARENT, "SavBas", 1 );
double SavLux_1 = VLS( PARENT, "SavLux", 1 );

double AkL = WHTAVES( PARENT, "B1", "L1" ) / SUMS( PARENT, "L1" );
double AcL = WHTAVES( PARENT, "A2", "L2" ) / SUMS( PARENT, "L2" );
double Lk = SUMS( PARENT, "L1" );
double Lc = SUMS( PARENT, "L2" );
double QeK = SUMS( PARENT, "Q1e" );

double Gbail = VS( FINSECL1, "Gbail" );
double rD = VS( FINSECL1, "rD" );
double Gtrain = VS( LABSUPL1, "Gtrain" );
double L = VS( LABSUPL1, "L" );
double Cd = VS( MACSTAL1, "Cd" );
double GDI = VS( MACSTAL1, "GDI" );
double In = VS( MACSTAL1, "In" );
double Nnom = VS( MACSTAL1, "Nnom" );
double dA = VS( MACSTAL1, "dA" );
double dCPI = VS( MACSTAL1, "dCPI" );
double DcD = VS( SECSTAL1, "DcD" );
double DcE = VS( SECSTAL1, "DcE" );
double QcE = VS( SECSTAL1, "QcE" );
double pCavg = VS( SECSTAL1, "pCavg" );

// detect industry exit to ignore tests based on totals computed here
bool exitInd = ! ( ( v[4] == lastID2 && numInd == v[17] ) ||
				   ( v[4] == lastID2 + 1 && numInd == v[17] + 1 ) );

double nonNeg[ ] = { CdLux, DcLux, Gbail, Gcons, Gd, Gtrain, Gtrf, I, Inom, Ireal,
					 SavBas, SavForc, SavLux, ScLux, Tax, cEntry, cExit, rD };
double posit[ ] = { A, AkL, AcL, C, Cd, CdBas, Creal, CPI, DcBas, DcD, DcE, GDI,
					GDPnom, GDPreal, L, Lc, Lk, PPI, QcE, QeK, Sc, ScBas, pCavg,
					pC0, pK0 };
double finite[ ] = { dA, dCPI, dCPIb, dGDPreal, dNnom };

dblVecT all ( nonNeg, END_ARR( nonNeg ) );
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// first period actions
if ( T == v[1] )
{
	iniGDP = GDPreal;
	iniA = A;
	iniDeb = Deb;
	iniSavBas = SavBas;
	iniSavLux = SavLux;
}

// national accounting
LOG( "\n  @@ (t=%g) dA=%.2g dGDP=%.2g C%%%%=%.2g I%%%%=%.2g G%%%%=%.2g dN%%%%=%.2g Tax%%%%=%.2g",
	 T, dA, dGDPreal, C / GDPnom, Inom / GDPnom, ( Gcons + Gtrf + Gbail ) / GDPnom,
	 dNnom / GDPnom, Tax / GDPnom );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	CFUN( check_error, nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

for ( i = 0; i < LEN_ARR( posit ); ++i )
	CFUN( check_error, posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( itd = all.begin( ); itd != all.end( ); ++itd )
	CFUN( check_error, ! isfinite( *itd ),
		  "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

CFUN( check_error, dA < - TOL, "INVALID-PRODUCTIVITY", 0, & errors );

CFUN( check_error, ! exitInd && ( A < min( AkL, AcL ) || A > max( AkL, AcL ) ),
	  "INCONSISTENT-PRODUCTIVITY", 0, & errors );

CFUN( check_error, GDPnom > ( 1 + TOL ) * ( C + Gcons + dNnom + Inom ) ||
	  GDPnom < ( 1 - TOL ) * ( C + Gcons + dNnom + Inom ),
	  "GDPnom-GAP", 0, & errors );

CFUN( check_error, C > Cd || Gcons > Gd || Gcons > DcBas || DcBas + DcLux > ceil( DcD ) ||
	  floor( DcBas ) > CdBas  || floor( DcLux ) > CdLux ||
	  floor( Cd ) != floor( CdBas + CdLux ) ||
	  ( ! exitInd && ( floor( v[8] ) > DcD || floor( v[7] ) > v[8] ||
	  DcBas > ( 1 + TOL ) * v[13] || DcBas < ( 1 - TOL ) * v[13] ||
	  ( DcLux > 1000 && v[14] > 1000 && ( DcLux > ( 1 + TOL ) * v[14] ||
	  DcLux < ( 1 - TOL ) * v[14] ) ) ) ),
	  "INCONSISTENT-DEMAND", 0, & errors );

CFUN( check_error, ScBas > ( 1 + TOL ) * DcBas || ScBas < ( 1 - TOL ) * DcBas ||
	  ( ScLux > 1000 && DcLux > 1000 && ( ScLux > ( 1 + TOL ) * DcLux ||
	  ScLux < ( 1 - TOL ) * DcLux ) ) ||
	  ScBas + ScLux > ( 1 + TOL ) * ( C + Gcons ) ||
	  ScBas + ScLux < ( 1 - TOL ) * ( C + Gcons ) ||
	  ( ! exitInd && ( floor( Sc ) != floor( v[12] ) ||
	  floor( v[7] ) != floor( v[12] ) || v[13] > v[15] || v[14] > v[16] ) ),
 	  "INCONSISTENT-SALES", 0, & errors );

CFUN( check_error, ( C + Gcons > Cd + Gd && C + Gcons > QcE * CPI_1 ) ||
	  ( ! exitInd && floor( v[9] ) != floor( QcE ) ),
	  "INCONSISTENT-PRODUCTION", 0, & errors );

CFUN( check_error, ( ! exitInd && floor( v[10] + v[11] ) != floor( I ) ) ||
	  Inom > ( 1 + TOL ) * I * pK0 * PPI ||
	  Inom < ( 1 - TOL ) * I * pK0 * PPI,
	  "INCONSISTENT-INVESTMENT", 0, & errors );

// public deficit/debt, firms equity and dynamics
LOG( "\n   @ Def%%%%=%.2g Deb%%%%=%.2g cEntry%%%%=%.2g cExit%%%%=%.2g entryExit=%g ",
	 Def / GDPnom, Deb / GDPnom, cEntry / GDPnom, cExit / GDPnom, entryExit );

CFUN( check_error, cEntry / GDPnom > TOL, "HIGH-EQUITY", 0, & errors );

CFUN( check_error, Deb / GDPnom > 100 * TOL, "EXPLOSIVE-DEBT", 0, & errors );

CFUN( check_error, ROUND( v[5], 1, 0.001 ) != 1,
	  "INCONSISTENT-WALLET-SHARES", 0, & errors );

CFUN( check_error, ! exitInd && ( v[6] != Lc || v[6] + Lk != L ),
	  "INCONSISTENT-WORKERS", 0, & errors );

// income, desired consumption, inventories, forced savings
LOG( "\n   @ GDI%%%%=%.2g Cd+Gd%%%%=%.2g Nnom%%%%=%.2g SavBas%%%%=%.2g SavLux%%%%=%.2g",
	 GDI / GDPnom, ( Cd + Gd ) / GDPnom, Nnom / GDPnom, SavBas / GDPnom,
	 SavLux / GDPnom );

CFUN( check_error, floor( SavForc ) != floor( CdBas + Gd - ScBas ) ||
	  SavBas > SavBas_1 * ( 1 + rD ) + SavForc ||
	  SavLux < SavLux_1 * ( 1 + rD ) - ScLux,
	  "INCONSISTENT-SAVINGS", 0, & errors );

CFUN( check_error, GDPnom > ( 1 + TOL ) * GDI ||
	  GDPnom < ( 1 - TOL ) * GDI,
	  "GDI-GAP", 0, & errors );

CFUN( check_error, abs( dCPIb ) > TOL, "HIGH-INFLATION", 0, & errors );

CFUN( check_error, Nnom / pCavg > TOL * QcE && SavForc > TOL * Nnom,
	  "HIGH-INVENTORIES", 0, & errors );

CFUN( check_error, SavForc / GDPnom > TOL || ( SavLux - SavLux_1 ) / GDPnom > TOL,
	  "HIGH-SAVINGS", 0, & errors );

// save industries situation for next period
lastID2 = v[4];
numInd = v[17];

// last period actions
if ( T == v[2] )
{
	v[4] = ( log( GDPreal + 1 ) - log( iniGDP + 1 ) ) / v[3];
	v[5] = ( log( A ) - log( iniA ) ) / v[3];
	v[6] = ( log( Deb + 1 ) - log( iniDeb + 1 ) ) / v[3];
	v[7] = ( log( SavBas + 1 ) - log( iniSavBas + 1 ) ) / v[3];
	v[8] = ( log( SavLux + 1 ) - log( iniSavLux + 1 ) ) / v[3];

	LOG( "\n   @ GDPgwth=%.3g Agwth=%.3g DebGwth=%.3g SavGwth=%.3g",
		 v[4], v[5], v[6], v[7] );

	CFUN( check_error, v[4] < TOL / 20 || v[5] < TOL / 20,
		  "LOW-GROWTH", 0, & errors );

	CFUN( check_error, v[6] > ( 1 + TOL ) * v[4],
		  "FAST-DEBT-GROWTH", 0, & errors );

	CFUN( check_error, v[7] > ( 1 + TOL ) * v[4] || v[8] > ( 1 + TOL ) * v[4],
		  "FAST-SAVINGS-GROWTH", 0, & errors );

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

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

static double lastID2 = 0, numInd = 0;

if ( T == v[1] )
	LOG( "\n $$$$ TESTING OF FINANCIAL SECTOR STARTED" );

// scan banks for severe problems
objVecT TCerr;									// vector to save TC error banks
int errors = 0;									// error counter
v[4] = v[5] = v[6] = v[7] = v[8] = v[9] = k = 0;// accumulators
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

CYCLES( PARENT, cur, "Consumption" )
{
	v[8] = max( VS( cur, "ID2" ), v[4] );
	++v[9];
}

double BadDeb = VS( FINSECL1, "BadDeb" );
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
double HHb = VS( SECSTAL1, "HHb" );
double HPb = VS( SECSTAL1, "HPb" );

double SavBas = VS( PARENT, "SavBas" );
double SavLux = VS( PARENT, "SavLux" );
double cExit = VS( PARENT, "cExit" );

double F1sum = SUMS( PARENT, "F1" );
double F2sum = SUMS( PARENT, "F2" );
double NWk = SUMS( PARENT, "NW1" );
double NWc = SUMS( PARENT, "NW2" );
double entryK = SUMS( PARENT, "entry1" );
double entryC = SUMS( PARENT, "entry2" );
double exitK = SUMS( PARENT, "exit1" );
double exitC = SUMS( PARENT, "exit2" );

// detect industry exit to ignore tests based on totals computed here
bool exitInd = ! ( ( v[8] == lastID2 && numInd == v[9] ) ||
				   ( v[8] == lastID2 + 1 && numInd == v[9] + 1 ) );

double nonNeg[ ] = { Depo, DivB, Gbail, Loans, TaxB, phi, rRes, Bda, Bfail,
					 BadDeb, cExit, HHb, HPb, SavBas, SavLux, NWk, NWc };
double posit[ ] = { Cl, r, rDeb, F1sum, F2sum };
double finite[ ] = { TC, PiB, NWk, NWc, entryK, exitK, entryC, exitC };

dblVecT all ( nonNeg, END_ARR( nonNeg ) );
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// bank customers and crisis/bail-outs
LOG( "\n  $$$ (t=%g) #Bank=%d #Client1=%g #Client2=%g Bfail=%g Gbail=%.3g phi=%.2g",
	 T, k, v[4], v[5], Bfail, Gbail, phi );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	CFUN( check_error, nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

for ( i = 0; i < LEN_ARR( posit ); ++i )
	CFUN( check_error, posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( itd = all.begin( ); itd != all.end( ); ++itd )
	CFUN( check_error, ! isfinite( *itd ),
		  "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

CFUN( check_error, F1sum != v[4], "INCONSISTENT-CLIENT1", 0, & errors );

CFUN( check_error, F2sum != v[5], "INCONSISTENT-CLIENT2", 0, & errors );

CFUN( check_error, Cl != v[6], "INCONSISTENT-CLIENT", 0, & errors );

CFUN( check_error, v[7] < 1 - TOL / 10 || v[7] > 1 + TOL / 10,
	  "INCONSISTENT-SHARES", 0, & errors );

CFUN( check_error, phi >= 1, "INCONSISTENT-PHI", 0, & errors );

// interest rat structure and bank assets and liabilities
LOG( "\n   $$ r=%.2g rDeb=%.2g rRes=%.2g Depo=%.3g Loans=%.3g",
	 r, rDeb, rRes, Depo, Loans );

CFUN( check_error, rD > r || rRes > r || rDeb < r || rD > rRes || rRes > rDeb,
	  "INCONSISTENT-INTEREST-STRUCTURE", 0, & errors );

CFUN( check_error, ! exitInd && floor( Depo - cExit ) > NWk + NWc + SavBas + SavLux,
	  "INCONSISTENT-DEPOSITS", 0, & errors );

// banks cash-flow
LOG( "\n   $$ TC=%.3g BadDeb=%.3g TaxB=%.3g PiB=%.3g NWb=%.3g",
	 TC, BadDeb, TaxB, PiB, NWb );

CFUN( check_error, TC < -1, "NEGATIVE-TOTAL-CREDIT", 0, & errors );

CFUN( check_error, TCerr.size( ) > 0,
	  "INCONSISTENT-TC-FREE", TCerr.size( ), & errors );

// save industries situation for next period
lastID2 = v[8];
numInd = v[9];

if ( T == v[2] )
	LOG( "\n $$$$ TESTING OF FINANCIAL SECTOR FINISHED" );

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

if ( T == v[1] )
	LOG( "\n +++ TESTING OF LABOR SUPPLY STARTED" );

// scan workers for severe problems
int Tr = VS( LABSUPL1, "Tr" );					// time for retirement
int govExp = VS( PARENT, "flagGovExp" );		// type of minimum income
double Lscale = VS( LABSUPL1, "Lscale" );		// labor scaling factor
double tauT = VS( LABSUPL1, "tauT" );			// tenure skills acc. factor
double wCap = VS( LABSUPL1, "wCap" );			// wage cap
double wMinPol = VS( LABSUPL1, "wMinPol" );		// minimum wage
double wU = VS( LABSUPL1, "wU" );				// unemployment benefit
double w0min = VS( LABSUPL1, "w0min" );			// subsistence income
object *Capital = SEARCHS( PARENT, "Capital" );	// pointer to sector1

objVecT InZerr, Qerr, ageErr, hirErr, srchErr, 	// vectors to save error workers
		wlErr, wChgErr, wRerr, sTerr, VintErr;

int errors = 0;									// error counter
v[4] = v[5] = v[6] = v[7] = v[8] = 0;			// accumulators
v[9] = v[10] = v[11] = v[12] = k = 0;
CYCLES( LABSUPL1, cur, "Worker" )
{
	v[5] += VS( cur, "_Q" ) * Lscale;
	v[9] += VS( cur, "_Bon" ) * Lscale;

	if ( VS( cur, "_employed" ) > 0 )
	{
		v[6] += VS( cur, "_w" ) * Lscale;

		if ( PARENTS( HOOKS( cur, FWRK ) ) == Capital )
			++v[7];
		else
		{
			++v[8];

			if ( HOOKS( cur, VWRK ) != NULL &&
				 PARENTS( HOOKS( cur, FWRK ) ) != GRANDPARENTS( HOOKS( cur, VWRK ) ) )
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

	if ( VS( cur, "_In" ) <= 0 || VS( cur, "_w" ) < 0 ||
		 VS( cur, "_wR" ) <= 0 || VS( cur, "_wReal" ) < 0 ||
		 VS( cur, "_wRes" ) <= 0 || VS( cur, "_wS" ) <= 0 )
		InZerr.push_back( cur );

	if ( ( VS( cur, "_employed" ) && VS( cur, "_w" ) < wMinPol ) ||
		 ( govExp >= 2 && VS( cur, "_In" ) < wU ) ||
		 ( govExp < 2 && VS( cur, "_In" ) < w0min ) ||
		 VS( cur, "_w" ) > VS( cur, "_In" ) )
		wlErr.push_back( cur );

	if ( T > 1 && ( VS( cur, "_In" ) >= VLS( cur, "_In", 1 ) * wCap ||
					VS( cur, "_In" ) <= VLS( cur, "_In", 1 ) / wCap ) )
		 wChgErr.push_back( cur );

	if ( VS( cur, "_wR" ) == VLS( cur, "_wR", 1 ) * wCap ||
		 VS( cur, "_wR" ) == VLS( cur, "_wR", 1 ) / wCap )
		 wRerr.push_back( cur );

	++k;
}

// count worker bridge objects in both sectors
CYCLES( PARENT, cur, "Capital" )
	v[10] += COUNTS( cur, "Wrk1" );

CYCLES( PARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )
	{
		v[11] += COUNTS( cur1, "Wrk2" );
		v[12] += VS( cur1, "_L2" );
	}

double Bon = VS( LABSUPL1, "Bon" );
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
double wCent = VS( LABSUPL1, "wCent" );
double wLogSD = VS( LABSUPL1, "wLogSD" );

double QcE = VS( SECSTAL1, "QcE" );

double InAvg = VS( LABSTAL1, "InAvg" );
double Lpart = VS( LABSTAL1, "Lpart" );
double TuAvg = VS( LABSTAL1, "TuAvg" );
double Vac = VS( LABSTAL1, "V" );
double wAvgReal = VS( LABSTAL1, "wAvgReal" );
double wGini = VS( LABSTAL1, "wGini" );
double InLogSD = VS( LABSTAL1, "InLogSD" );
double wMax = VS( LABSTAL1, "wMax" );
double wMin = VS( LABSTAL1, "wMin" );
double wrAvg = VS( LABSTAL1, "wrAvg" );
double wrLogSD = VS( LABSTAL1, "wrLogSD" );
double wsAvg = VS( LABSTAL1, "wsAvg" );
double wsLogSD = VS( LABSTAL1, "wsLogSD" );

double Lk = SUMS( PARENT, "L1" );
double Lc = SUMS( PARENT, "L2" );
double Wk = SUMS( PARENT, "W1" );
double Wc = SUMS( PARENT, "W2" );
double firesK = SUMS( PARENT, "fires1" );
double firesC = SUMS( PARENT, "fires2" );
double hiresK = SUMS( PARENT, "hires1" );
double hiresC = SUMS( PARENT, "hires2" );
double quitsK = SUMS( PARENT, "quits1" );
double quitsC = SUMS( PARENT, "quits2" );
double retiresK = SUMS( PARENT, "retires1" );
double retiresC = SUMS( PARENT, "retires2" );

double nonNeg[ ] = { Gtrain, Ltrain, TeAvg, U, Ue, Us, sVavg, sTsd, sVsd,
					 wLogSD, wMinPol, wU, Vac, InLogSD, wMax, wMin, wrLogSD,
					 wsLogSD, Lk, Wk, firesK, hiresK, quitsK, retiresK, Bon, Lc,
					 QcE, Wc, hiresC, firesC, quitsC, retiresC, wGini, TuAvg };
double posit[ ] = { L, Ls, Lpart, appl, sAvg, sTavg, sTmax, sTmin, wAvg,
					searchProb, InAvg, wCent, wAvgReal, wrAvg, wsAvg };
double finite[ ] = { dUeB };

dblVecT all ( nonNeg, END_ARR( nonNeg ) );
all.insert( all.end( ), posit, END_ARR( posit ) );
all.insert( all.end( ), finite, END_ARR( finite ) );

// employment summary
LOG( "\n  ++ (t=%g) Ls=%g L=%g Ltrain=%g V=%.2g U=%.2g Us=%.2g Ue=%.2g",
	 T, Ls, L, Ltrain, Vac, U, Us, Ue );

for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
	CFUN( check_error, nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

for ( i = 0; i < LEN_ARR( posit ); ++i )
	CFUN( check_error, posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

for ( itd = all.begin( ); itd != all.end( ); ++itd )
	CFUN( check_error, ! isfinite( *itd ),
		  "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

CFUN( check_error, U > 1 || Vac > 1, "INCONSISTENT-LABOR-STATS", 0, & errors );

CFUN( check_error, Lk + Lc != L || Ls < L || Ls < Ltrain,
	  "INCONSISTENT-LABOR", 0, & errors );

CFUN( check_error, Lk != v[7] * Lscale || Lk != v[10] * Lscale,
	  "INCONSISTENT-LABOR-S1", 0, & errors );

CFUN( check_error, Lc < v[8] * Lscale || v[8] != v[11] || v[12] != v[11] * Lscale,
	  "INCONSISTENT-LABOR-S2", 0, & errors );

CFUN( check_error, Us > Ue || Ue > U, "INCONSISTENT-UNEMPLOYMENT", 0, & errors );

CFUN( check_error, InZerr.size( ) > 0,
	  "ZERO-WAGE-WORKERS", InZerr.size( ), & errors );

CFUN( check_error, wlErr.size( ) > 0,
	  "LOW-WAGE-WORKERS", wlErr.size( ), & errors );

// labor market
LOG( "\n   + TeAvg=%g TuAvg=%g appl=%.2g quits=%g retires=%g",
	 round( TeAvg ), round( TuAvg ), appl, round( quitsK + quitsC ),
	 round( retiresK + retiresC ) );

CFUN( check_error, ageErr.size( ) > 0,
	  "UNRETIRED-WORKERS", ageErr.size( ), & errors );

CFUN( check_error, hirErr.size( ) > 0,
	  "FAILED-HIRE-WORKERS", hirErr.size( ), & errors );

CFUN( check_error, srchErr.size( ) > 0,
	  "NOT-SEARCH-WORKERS", srchErr.size( ), & errors );

// productivity and production
LOG( "\n   + sAvg=%.2g sVavg=%.2g sVsd=%.2g sTavg=%.2g sTsd=%.2g Q=%.3g",
	 sAvg, sVavg, sVsd, sTavg, sTsd, v[5] );

CFUN( check_error, Qerr.size( ) > 0,
	  "ZERO-PROD-WORKERS", Qerr.size( ), & errors );

CFUN( check_error, sTerr.size( ) > 0,
	  "EXCESS-SKILL-WORKERS", sTerr.size( ), & errors );

CFUN( check_error, VintErr.size( ) > 0,
	  "EXCESS-SKILL-WORKERS", VintErr.size( ), & errors );

// wages
LOG( "\n   + InAvg=%.2g wAvg=%.2g wMin=%.2g wMax=%.2g wrAvg=%.2g wsAvg=%.2g",
	 InAvg, wAvg, wMin, wMax, wrAvg, wsAvg );

CFUN( check_error, Wk + Wc + Bon < floor( v[6] + v[9] ),
	  "INCONSISTENT-PAYROLL", 0, & errors );

CFUN( check_error, wrAvg < wsAvg, "INCONSISTENT-WAGES", 0, & errors );

CFUN( check_error, wChgErr.size( ) > 0,
	  "LARGE-WAGE-CHANGE", wChgErr.size( ), & errors );

CFUN( check_error, wRerr.size( ) > 0,
	  "LARGE-REQ-WAGE-CHANGE", wRerr.size( ), & errors );

if ( T == v[2] )
	LOG( "\n +++ TESTING OF LABOR SUPPLY FINISHED" );

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

if ( T == v[1] )
	LOG( "\n ^^^ TESTING OF CAPITAL-GOOD SECTOR STARTED" );

// scan firms for severe problems
double Ls = VS( LABSUPL1, "Ls" );				// labor force size
double Lscale = VS( LABSUPL1, "Lscale" );		// labor scaling factor

objVecT Aerr, RDerr, c1err, CliErr;				// vectors to save error firms

int errors = 0;									// error counter
v[5] = 0;
CYCLES( PARENT, cur, "Capital" )
{
	v[10] = v[11] = v[12] = j = 0;					// accumulators
	CYCLES( cur, cur1, "Firm1" )
	{
		v[10] += VS( cur1, "_f1" );
		v[11] += ( VS( cur1, "_L1" ) <= 0 ) ? 1 : 0;

		if ( VS( cur1, "_A1" ) <= TOL || VS( cur1, "_B1" ) <= TOL )
			Aerr.push_back( cur1 );

		if ( VS( cur1, "_RD1" ) < 0 )
			RDerr.push_back( cur1 );

		if ( VS( cur1, "_c1" ) <= 0 || VS( cur1, "_p1" ) <= 0 )
			c1err.push_back( cur1 );

		if ( VS( cur1, "_t1ent" ) > T &&
			 VS( cur1, "_HC1" ) + VS( cur1, "_NC1" ) <= 0 )
			CliErr.push_back( cur1 );

		++j;
	}

	CYCLES( cur, cur1, "Wrk1" )
		v[12] += VS( SHOOKS( cur1 ), "_w" ) * Lscale;

	double B1 = VS( cur, "B1" );
	double D1 = VS( cur, "D1" );
	double Deb1 = VS( cur, "Deb1" );
	double Div1 = VS( cur, "Div1" );
	double F1 = VS( cur, "F1" );
	double HH1 = VS( cur, "HH1" );
	double HP1 = VS( cur, "HP1" );
	double JO1 = VS( cur, "JO1" );
	double L1 = VS( cur, "L1" );
	double L1d = VS( cur, "L1d" );
	double L1dRD = VS( cur, "L1dRD" );
	double L1rd = VS( cur, "L1rd" );
	double MC1 = VS( cur, "MC1" );
	double NW1 = VS( cur, "NW1" );
	double Pi1 = VS( cur, "Pi1" );
	double Q1 = VS( cur, "Q1" );
	double Q1e = VS( cur, "Q1e" );
	double RD1 = VS( cur, "RD1" );
	double S1 = VS( cur, "S1" );
	double Tax1 = VS( cur, "Tax1" );
	double W1 = VS( cur, "W1" );
	double age1avg = VS( cur, "age1avg" );
	double cred1c = VS( cur, "cred1c" );
	double entry1exit = VS( cur, "entry1exit" );
	double fires1 = VS( cur, "fires1" );
	double hires1 = VS( cur, "hires1" );
	double imi1 = VS( cur, "imi1" );
	double inn1i = VS( cur, "inn1i" );
	double inn1r = VS( cur, "inn1r" );
	double noWrk1 = VS( cur, "noWrk1" );
	double p1 = VS( cur, "p1" );
	double quits1 = VS( cur, "quits1" );
	double retires1 = VS( cur, "retires1" );
	double w1avg = VS( cur, "w1avg" );

	double F1_1 = VLS( cur, "F1", 1 );

	double nonNeg[ ] = { D1, Deb1, Div1, HH1, HP1, JO1, L1, L1, L1d, L1dRD, L1rd,
						 Q1, Q1e, RD1, S1, Tax1, W1, age1avg, cred1c, fires1,
						 hires1, imi1, inn1i, inn1r, noWrk1, quits1, retires1 };
	double posit[ ] = { B1, F1, p1, w1avg };
	double finite[ ] = { NW1, entry1exit, Pi1 };

	dblVecT all ( nonNeg, END_ARR( nonNeg ) );
	all.insert( all.end( ), posit, END_ARR( posit ) );
	all.insert( all.end( ), finite, END_ARR( finite ) );

	// innovation
	LOG( "\n  ^^ (t=%g) F1=%g inn1r=%g inn1i=%g imi1=%g B1=%.3g",
		 T, F1, inn1r, inn1i, imi1, B1 );

	for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
		CFUN( check_error, nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

	for ( i = 0; i < LEN_ARR( posit ); ++i )
		CFUN( check_error, posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

	for ( itd = all.begin( ); itd != all.end( ); ++itd )
		CFUN( check_error, ! isfinite( *itd ), "NON-FINITE-VALUE",
			  itd - all.begin( ) + 1, & errors );

	CFUN( check_error, imi1 + inn1i + inn1r > F1_1,
		  "INCONSISTENT-INNOVATION", 0, & errors );

	CFUN( check_error, imi1 > 1 - 2 * TOL || inn1i > 1 - 2 * TOL || inn1r > 1 - 2 * TOL,
		  "HIGH-INNOVATION", 0, & errors );

	CFUN( check_error, Aerr.size( ) > 0, "ZERO-PROD-FIRMS", Aerr.size( ), & errors );

	CFUN( check_error, RDerr.size( ) > 0, "ZERO-RD-FIRMS", RDerr.size( ), & errors );

	// production
	LOG( "\n   ^ D1=%.3g Q1=%.3g Q1e=%.3g",
		 D1, Q1, Q1e );

	CFUN( check_error, Q1e > Q1 || Q1 * p1 > ( 1 + TOL ) * D1,
		  "INCONSISTENT-PRODUCTION", 0, & errors );

	// labor
	LOG( "\n   ^ JO1=%g L1d=%g L1=%g L1rd=%g ret1=%g quit1=%g fire1=%g hire1=%g",
		 JO1, L1d, L1, L1rd, retires1, quits1, fires1, hires1 );

	CFUN( check_error, L1dRD > L1rd || L1rd > L1 || L1d < JO1 || L1 > Ls ||
		  hires1 > L1d + quits1 + retires1 + Lscale,
		  "INCONSISTENT-LABOR", 0, & errors );

	// cash flow
	LOG( "\n   ^ S1=%.3g W1=%.3g Tax1=%.3g Pi1=%.3g NW1=%.3g",
		 S1, W1, Tax1, Pi1, NW1 );

	CFUN( check_error, floor( W1 ) != floor( v[12] ),
		  "INCONSISTENT-PAYROLL", 0, & errors );

	CFUN( check_error, S1 + RD1 < ( 1 - TOL ) * W1,
		  "INCONSISTENT-WAGES", 0, & errors );

	CFUN( check_error, c1err.size( ) > 0,
		  "ZERO-COST-FIRM", c1err.size( ), & errors );

	// finance
	LOG( "\n   ^ Deb1=%.3g cred1c=%.3g p1=%.2g noWrk1=%.2g",
		 Deb1, cred1c, p1, noWrk1 );

	CFUN( check_error, v[11] / F1 > TOL || noWrk1 > TOL,
		  "MANY-NO-WORKER", 0, & errors );

	// competition
	LOG( "\n   ^ age1avg=%.3g MC1=%.2g entry1exit=%g HH1=%.2g HP1=%.2g",
		 age1avg, MC1, entry1exit, HH1, HP1 );

	CFUN( check_error, v[10] < 1 - TOL / 10 || v[10] >  1 + TOL / 10,
		  "INCONSISTENT-SHARES", 0, & errors );

	CFUN( check_error, CliErr.size( ) > 0,
		  "NO-CLIENT-FIRMS", CliErr.size( ), & errors );

	CFUN( check_error, HH1 > 1 || HP1 > 2,
		  "INCONSISTENT-STATS", 0, & errors );

	v[5] += L1;
}

CFUN( check_error, v[5] > Ls, "INCONSISTENT-CAPITAL-LABOR", 0, & errors );

if ( T == v[2] )
	LOG( "\n ^^^ TESTING OF CAPITAL-GOOD SECTOR FINISHED" );

RESULT( errors )


EQUATION( "test2sec" )
/*
Print detailed statistics of consumption-good sector (!=0 if error is found)
Set the industry range by industry ID2 in 'test2SidIni' and 'test2SidEnd'
Set the time range in 'test2StIni' and 'test2StEnd'
If 'test2SidIni' is zero, list up to 'test2SidEnd' industries entered from
'test2StIni'
*/

static firmMapT entr;

v[1] = V( "test2StIni" );
v[2] = V( "test2StEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

v[3] = V( "test2SidIni" );
v[4] = V( "test2SidEnd" );

k = v[3] > 0 ? 1 + v[4] - v[3] : v[4];			// number of firms

if ( T == v[1] )
	LOG( "\n &&& TESTING OF CONSUMPTION-GOOD SECTOR STARTED" );

// scan firms for severe problems
double Ls = VS( LABSUPL1, "Ls" );				// labor force size
double Lscale = VS( LABSUPL1, "Lscale" );		// labor scaling factor
double wCap = VS( LABSUPL1, "wCap" );			// wage cap

objVecT Bon2err, D2err, Kerr, L2err, Q2err,		// vectors to save error firms
		W2err, c2err, mu2err, w2oErr;

int errors = 0;									// error counter
v[5] = 0;
CYCLES( PARENT, cur, "Consumption" )
{
	j = VS( cur, "ID2" );
	h = VS( cur, "t2ent" );

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

	double mu20 = VS( cur, "mu20" );			// initial markup

	v[10] = v[11] = i = 0;						// accumulators
	CYCLES( cur, cur1, "Firm2" )
	{
		v[10] += VS( cur1, "_f2" );
		v[11] += ( VS( cur1, "_life2cycle" ) > 0 &&
				  VS( cur1, "_L2" ) <= 0 ) ? 1 : 0;

		v[20] = v[21] = v[8] = 0;
		CYCLES( cur1, cur2, "Wrk2" )
		{
			v[20] += VS( SHOOKS( cur2 ), "_w" ) * Lscale;
			v[21] += VS( SHOOKS( cur2 ), "_Bon" ) * Lscale;
		}

		if ( floor( VS( cur1, "_W2" ) ) != floor( v[20] ) )
			W2err.push_back( cur1 );

		if ( floor( VS( cur1, "_Bon2" ) ) != floor( v[21] ) )
			Bon2err.push_back( cur1 );

		if ( VS( cur1, "_life2cycle" ) > 0 && VLS( cur1, "_K2", 1 ) <= 0 )
			Kerr.push_back( cur1 );

		if ( VS( cur1, "_life2cycle" ) > 0 &&
			 VS( cur1, "_Q2d" ) + VLS( cur1, "_N2", 1 ) <= 0 )
			D2err.push_back( cur1 );

		if ( VS( cur1, "_Q2d" ) > 0 && VS( cur1, "_L2" ) > 0 &&
			 VS( cur1, "_Q2e" ) <= 0 )
			Q2err.push_back( cur1 );

		if ( VS( cur1, "_Q2e" ) > 0 && VS( cur1, "_L2" ) <= 0 )
			L2err.push_back( cur1 );

		if ( VS( cur1, "_c2" ) <= 0 || VS( cur1, "_p2" ) <= 0 )
			c2err.push_back( cur1 );

		if ( VS( cur1, "_mu2" ) < mu20 / 5 || VS( cur1, "_mu2" ) > mu20 * 5 )
			mu2err.push_back( cur1 );

		if ( VS( cur1, "_w2o" ) == VLS( cur1, "_w2o", 1 ) * wCap ||
			 VS( cur1, "_w2o" ) == VLS( cur1, "_w2o", 1 ) / wCap )
			 w2oErr.push_back( cur1 );

		++i;
	}

	double A2 = VS( cur, "A2" );
	double A2p = VS( cur, "A2p" );
	double Bon2 = VS( cur, "Bon2" );
	double CI2 = VS( cur, "CI2" );
	double D2 = VS( cur, "D2" );
	double D2a = VS( cur, "D2a" );
	double D2d = VS( cur, "D2d" );
	double D2e = VS( cur, "D2e" );
	double Deb2 = VS( cur, "Deb2" );
	double Div2 = VS( cur, "Div2" );
	double E2 = VS( cur, "E2" );
	double EI2 = VS( cur, "EI2" );
	double F2 = VS( cur, "F2" );
	double HH2 = VS( cur, "HH2" );
	double HP2 = VS( cur, "HP2" );
	double JO2 = VS( cur, "JO2" );
	double K2 = VS( cur, "K2" );
	double K2d = VS( cur, "K2d" );
	double L2 = VS( cur, "L2" );
	double L2d = VS( cur, "L2d" );
	double MC2 = VS( cur, "MC2" );
	double N2 = VS( cur, "N2" );
	double NW2 = VS( cur, "NW2" );
	double Pi2 = VS( cur, "Pi2" );
	double Pi2rateAvg = VS( cur, "Pi2rateAvg" );
	double Q2 = VS( cur, "Q2" );
	double Q2d = VS( cur, "Q2d" );
	double Q2e = VS( cur, "Q2e" );
	double Q2u = VS( cur, "Q2u" );
	double S2 = VS( cur, "S2" );
	double SI2 = VS( cur, "SI2" );
	double Tax2 = VS( cur, "Tax2" );
	double W2 = VS( cur, "W2" );
	double age2avg = VS( cur, "age2avg" );
	double c2avg = VS( cur, "c2avg" );
	double c2eAvg = VS( cur, "c2eAvg" );
	double cred2c = VS( cur, "cred2c" );
	double entry2exit = VS( cur, "entry2exit" );
	double hires2 = VS( cur, "hires2" );
	double fires2 = VS( cur, "fires2" );
	double k2 = VS( cur, "k2" );
	double l2max = VS( cur, "l2max" );
	double l2min = VS( cur, "l2min" );
	double m2 = VS( cur, "m2" );
	double mu2avg = VS( cur, "mu2avg" );
	double noWrk2 = VS( cur, "noWrk2" );
	double old2vint = VS( cur, "old2vint" );
	double p2 = VS( cur, "p2" );
	double p2max = VS( cur, "p2max" );
	double p2min = VS( cur, "p2min" );
	double q2max = VS( cur, "q2max" );
	double q2min = VS( cur, "q2min" );
	double quits2 = VS( cur, "quits2" );
	double retires2 = VS( cur, "retires2" );
	double w2avg = VS( cur, "w2avg" );
	double w2oAvg = VS( cur, "w2oAvg" );

	double K2avb = VLS( cur, "K2", 1 );
	double N2_1 = VLS( cur, "N2", 1 );

	double sAvg = VS( LABSUPL1, "sAvg" );

	double nonNeg[ ] = { Bon2, CI2, D2, D2a, D2d, D2e, Deb2, Div2, E2, EI2, JO2,
						 K2, K2d, K2avb, L2, L2d, N2, Q2, Q2e, SI2, Tax2, W2,
						 fires2, hires2, p2, quits2, retires2, l2max, l2min,
						 HH2, HP2, Q2d, Q2u, S2, cred2c, noWrk2 };
	double posit[ ] = { A2, A2p, F2, c2avg, c2eAvg, k2, old2vint, p2max, p2min,
						q2max, q2min, sAvg, w2avg, w2oAvg, age2avg,
						mu2avg };
	double finite[ ] = { NW2, entry2exit, Pi2, Pi2rateAvg };

	dblVecT all ( nonNeg, END_ARR( nonNeg ) );
	all.insert( all.end( ), posit, END_ARR( posit ) );
	all.insert( all.end( ), finite, END_ARR( finite ) );

	// capital and investment
	LOG( "\n  && (t=%g) ID2=%d t2ent=%d F2=%g K2d=%.3g K2avb=%.3g",
		 T, j, h, F2, K2d, K2avb );

	CFUN( check_error, i != F2, "INCONSISTENT-NUM-FIRMS", 0, & errors );

	for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
		CFUN( check_error, nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

	for ( i = 0; i < LEN_ARR( posit ); ++i )
		CFUN( check_error, posit[ i ] <= 0,
			  "NON-POSITIVE-VALUE", i + 1, & errors );

	for ( itd = all.begin( ); itd != all.end( ); ++itd )
		CFUN( check_error, ! isfinite( *itd ),
			  "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

	CFUN( check_error, Kerr.size( ) > 0,
		  "NO-CAPITAL-FIRMS", Kerr.size( ), & errors );

	LOG( "\n   & EI2=%.3g SI2=%.3g CI2=%.3g K2=%.3g", EI2, SI2, CI2, K2 );

	CFUN( check_error, floor( SI2 ) > K2avb || floor( K2 ) > K2avb + EI2 + SI2,
		  "INCONSISTENT-CAPITAL", 0, & errors );

	// productivity
	LOG( "\n   & A2p=%.3g A2=%.3g c2avg=%.3g c2eAvg=%.3g",
		 A2p, A2, c2avg, c2eAvg );

	CFUN( check_error, Q2e > ( 1 - 2 * TOL ) * Q2 && A2 > ( 1 + 2 * TOL ) * A2p,
		  "INCONSISTENT-PRODUCTIVITY", 0, & errors );

	CFUN( check_error, Q2e > ( 1 - 2 * TOL ) * Q2 && c2eAvg > ( 1 + 2 * TOL ) * c2avg,
		  "INCONSISTENT-UNIT-COST", 0, & errors );

	// production
	LOG( "\n   & Q2d=%.3g Q2=%.3g Q2e=%.3g N2=%.3g",
		 Q2d, Q2, Q2e, N2 );

	CFUN( check_error, Q2e > Q2 || Q2 > Q2d || floor( Q2e ) > sAvg * K2 * m2 / k2,
		  "INCONSISTENT-PRODUCTION", 0, & errors );

	CFUN( check_error, floor( Q2e ) > S2 / p2 && floor( S2 / p2 - N2 + N2_1 ) > Q2e,
		  "INCONSISTENT-INVENTORIES", 0, & errors );

	CFUN( check_error, c2err.size( ) > 0,
		  "ZERO-COST-FIRMS", c2err.size( ), & errors );

	CFUN( check_error, Q2err.size( ) > 0, "NO-PROD-FIRMS", Q2err.size( ), & errors );

	// labor
	LOG( "\n   & JO2=%g L2d=%g L2=%g ret2=%g quit2=%g fire2=%g hire2=%g",
		 JO2, L2d, L2, retires2, quits2, fires2, hires2 );

	CFUN( check_error, L2d < JO2 || L2 > Ls || hires2 > L2d + quits2 + retires2,
		  "INCONSISTENT-LABOR", 0, & errors );

	CFUN( check_error, L2err.size( ) > 0,
		  "NO-LABOR-PRODUCING", L2err.size( ), & errors );

	CFUN( check_error, w2oErr.size( ) > 0,
		  "LARGE-WAGE-OFFER-CHANGE", w2oErr.size( ), & errors );

	// cash flow
	LOG( "\n   & W2=%.3g Bon2=%.3g Tax2=%.3g Pi2=%.3g Pi2rAvg=%.2g",
		 W2, Bon2, Tax2, Pi2, Pi2rateAvg );

	CFUN( check_error, W2err.size( ) > 0,
		  "WAGES-GAP", W2err.size( ), & errors );

	CFUN( check_error, Bon2err.size( ) > 0,
		  "BONUSES-GAP", Bon2err.size( ), & errors );

	CFUN( check_error, Pi2rateAvg > 1,
		  "INCONSISTENT-PROFITS", 0, & errors );

	CFUN( check_error, S2 + N2 * p2 < ( 1 - TOL ) * W2,
		  "EXCESS-WORKERS", 0, & errors );

	// finance
	LOG( "\n   & NW2=%.3g Deb2=%.3g cred2c=%.3g p2=%.2g noWrk2=%.2g",
		 NW2, Deb2, cred2c, p2, noWrk2 );

	CFUN( check_error, v[11] / F2 > 2 * TOL || noWrk2 > 2 * TOL,
		  "MANY-NO-WORKER", 0, & errors );

	// market
	LOG( "\n   & mu2avg=%.2g p2min=%.2g p2max=%.2g D2=%.3g D2e=%.3g S2=%.3g",
		 mu2avg, p2min, p2max, D2, D2e, S2 );

	CFUN( check_error, floor( D2 ) > D2a,
		  "INCONSISTENT-DEMAND", 0, & errors );

	CFUN( check_error, D2err.size( ) > 0,
		  "NO-DEMAND-FIRMS", D2err.size( ), & errors );

	CFUN( check_error, mu2err.size( ) > 0,
		  "BAD-MARKUP-FIRMS", mu2err.size( ), & errors );

	// competition
	LOG( "\n   & age2avg=%g MC2=%.2g entry2exit=%g HH2=%.2g HP2=%.2g",
		 age2avg, MC2, entry2exit, HH2, HP2 );
	LOG( "\n   & q2min=%.2g q2max=%.2g l2min=%.2g l2max=%.2g",
		 q2min, q2max, l2min, l2max );

	CFUN( check_error, v[10] < 1 - TOL / 10 || v[10] > 1 + TOL / 10,
		  "INCONSISTENT-SHARES", 0, & errors );

	CFUN( check_error, HH2 > 1 || HP2 > 2,
		  "INCONSISTENT-STATS", 0, & errors );

	v[5] += L2;
}

CFUN( check_error, v[5] > Ls,
	  "INCONSISTENT-CONSUMER-LABOR", 0, & errors );

if ( T == v[2] )
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
static double iniA1, iniB1;
static firmMapT entr;

v[1] = V( "test1tIni" );
v[2] = V( "test1tEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

v[3] = V( "test1idIni" );
v[4] = V( "test1idEnd" );

v[5] = 1 + v[2] - v[1];							// number of periods
k = v[3] > 0 ? 1 + v[4] - v[3] : v[4];			// number of firms

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
				 "t,ID1,t1ent,Client,L1d,L1,L1rd",
				 "RD1,g1,A1,B1,c1,D1,Q1,Q1e",
				 "Pi1,NW1,Deb1,Deb1max,cred1,cred1c",
				 "HC1,NC1,BC1,p1,S1,f1" );
	}
}

int errors = 0;									// error counter
CYCLES( PARENT, cur, "Capital" )
	CYCLES( cur, cur1, "Firm1" )
	{
		i = VS( cur1, "_ID1" );
		h = VS( cur1, "_t1ent" );

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
						entr[ i ] = cur1;		// add entrant in to show list
				}
			}

		j = COUNTS( cur1, "Cli" );

		double _A1 = VS( cur1, "_A1" );
		double _B1 = VS( cur1, "_B1" );
		double _BC1 = VS( cur1, "_BC1" );
		double _D1 = VS( cur1, "_D1" );
		double _Deb1 = VS( cur1, "_Deb1" );
		double _Deb1max = VS( cur1, "_Deb1max" );
		double _HC1 = VS( cur1, "_HC1" );
		double _L1d = VS( cur1, "_L1d" );
		double _L1 = VS( cur1, "_L1" );
		double _L1rd = VS( cur1, "_L1rd" );
		double _NC1 = VS( cur1, "_NC1" );
		double _NW1 = VS( cur1, "_NW1" );
		double _Pi1 = VS( cur1, "_Pi1" );
		double _Q1 = VS( cur1, "_Q1" );
		double _Q1e = VS( cur1, "_Q1e" );
		double _RD1 = VS( cur1, "_RD1" );
		double _S1 = VS( cur1, "_S1" );
		double _c1 = VS( cur1, "_c1" );
		double _cred1 = VS( cur1, "_cred1" );
		double _cred1c = VS( cur1, "_cred1c" );
		double _f1 = VS( cur1, "_f1" );
		double _g1 = VS( cur1, "_g1" );
		double _imi1 = VS( cur1, "_imi1" );
		double _inn1i = VS( cur1, "_inn1i" );
		double _inn1r = VS( cur1, "_inn1r" );
		double _p1 = VS( cur1, "_p1" );

		double _A1_1 = VLS( cur1, "_A1", 1 );
		double _B1_1 = VLS( cur1, "_B1", 1 );

		double nonNeg[ ] = { _HC1, _NC1, _RD1, _D1, _Q1, _Q1e, _BC1, _L1, _L1d,
							 _L1rd, _Deb1, _Deb1max, _cred1, _cred1c, _S1, _f1,
							 _g1, _imi1, _inn1i, _inn1r };
		double posit[ ] = { _A1, _B1, _c1, _p1 };
		double finite[ ] = { _NW1, _Pi1 };

		dblVecT all ( nonNeg, END_ARR( nonNeg ) );
		all.insert( all.end( ), posit, END_ARR( posit ) );
		all.insert( all.end( ), finite, END_ARR( finite ) );

		// first period actions (single-firm analysis only)
		if ( k == 1 && T == v[1] )
		{
			iniA1 = _A1;
			iniB1 = _B1;
		}

		LOG( "\n  ** (t=%g) ID1=%d t1ent=%d #Client=%d L1d=%g L1=%g L1rd=%g",
			 T, i, h, j, _L1d, _L1, _L1rd );
		fprintf( firms1, "%g,%d,%d,%d,%g,%g,%g", T, i, h, j, _L1d, _L1, _L1rd );

		for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
			CFUN( check_error, nonNeg[ i ] < 0,
				  "NEGATIVE-VALUE", i + 1, & errors );

		for ( i = 0; i < LEN_ARR( posit ); ++i )
			CFUN( check_error, posit[ i ] <= 0,
				  "NON-POSITIVE-VALUE", i + 1, & errors );

		for ( itd = all.begin( ); itd != all.end( ); ++itd )
			CFUN( check_error, ! isfinite( *itd ),
				  "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

		CFUN( check_error, _L1 > _L1d || _L1 < _L1rd || _L1rd > _L1d,
			  "INCONSISTENT-LABOR", 0, & errors );

		// innovation, productivity
		LOG( "\n   * RD1=%g g1=%g A1=%.3g B1=%.3g c1=%.3g D1=%g Q1=%g Q1e=%g",
			 round( _RD1 ), _g1, _A1, _B1, _c1, _D1, _Q1, _Q1e );
		fprintf( firms1, ",%g,%g,%g,%g,%g,%g,%g,%g",
				 _RD1, _g1, _A1, _B1, _c1, _D1, _Q1, _Q1e );

		CFUN( check_error, _RD1 <= 0, "NO-R&D", 0, & errors );

		CFUN( check_error, _A1 < TOL || _B1 < TOL,
			  "INCONSISTENT-PRODUCTIVITY", 0, & errors );

		CFUN( check_error, _imi1 + _inn1i + _inn1r > 1 || _A1 * _B1 < _A1_1 * _B1_1,
			  "INCONSISTENT-INNOVATION", 0, & errors );

		// finance
		LOG( "\n   * Pi1=%g NW1=%g Deb1=%g Deb1max=%g cred1=%g cred1c=%g",
			 round( _Pi1 ), round( _NW1 ), round( _Deb1 ), round( _Deb1max ),
			 round( _cred1 ), round( _cred1c ) );
		fprintf( firms1, ",%g,%g,%g,%g,%g,%g",
				 _Pi1, _NW1, _Deb1, _Deb1max, _cred1, _cred1c );

		// market and client dynamics
		LOG( "\n   * HC1=%g NC1=%g BC1=%g p1=%.2g S1=%g f1=%.2g",
			 _HC1, _NC1, _BC1, _p1, round( _S1 ), _f1 );
		fprintf( firms1, ",%g,%g,%g,%g,%g,%g",
				 _HC1, _NC1, _BC1, _p1, _S1, _f1 );

		CFUN( check_error, j > _HC1 + _NC1,
			  "INCONSISTENT-CLIENTS", 0, & errors );

		CFUN( check_error, j < _BC1, "INCONSISTENT-BUYERS", 0, & errors );

		// last period actions (single-firm analysis only)
		if ( k == 1 && T == v[2] )
		{
			LOG( "\n   * AtauGwth=%.3g BtauGwth=%.3g",
				 ( log( _A1 + 1 ) - log( iniA1 + 1 ) ) / v[5],
				 ( log( _B1 + 1 ) - log( iniB1 + 1 ) ) / v[5] );
		}

		fputs( "\n", firms1 );
	}

if ( T == v[2] )
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
If 'test2idIni' is negative, list up to 'test2idEnd' firms entered from
'test2tIni' in industry ID2, being test2idIni=-ID2
*/

static FILE *firms2 = NULL;						// output file pointer
static double iniK2, iniNW2, iniDeb2;
static firmMapT entr;

v[1] = V( "test2tIni" );
v[2] = V( "test2tEnd" );

if ( T >= v[2] )
	PARAMETER;									// compute for the last time

if ( T < v[1] || v[2] == 0 )
	END_EQUATION( 0 )

v[3] = V( "test2idIni" );
v[4] = V( "test2idEnd" );

v[5] = 1 + v[2] - v[1];							// number of periods
k = v[3] > 0 ? 1 + v[4] - v[3] : v[4];			// number of firms

if ( T == v[1] )
{
	LOG( "\n ### TESTING OF CONSUMER-GOOD FIRMS STARTED" );

	if ( v[3] > 0 )
		LOG( " (t=%g-%g, ind=%d, firms=%d-%d, file=%s)",
			 v[1], v[2], ID_IND( v[3] ), FIRM_NUM( v[3] ), FIRM_NUM( v[4] ),
			 TEST2FILE );
	else
		if ( v[3] < 0 )
			LOG( " (t=%g-%g, ind=%g, up to %g entrants, file=%s)",
				 v[1], v[2], - v[3], v[4], TEST2FILE );
		else
			LOG( " (t=%g-%g, up to %g entrants, file=%s)",
				 v[1], v[2], v[4], TEST2FILE );

	if ( firms2 == NULL )						// don't reopen if already open
	{
		firms2 = fopen( TEST2FILE, "w" );		// (re)create the file
		fprintf( firms2, "%s,%s,%s,%s,%s\n",	// file header
				 "t,ID2,t2ent,life2cycle,Broch,Vint,Wrk,pVint",
				 "K2d,K2avb,EI2d,SI2d,EI2,SI2,CI2,K2",
				 "D2e,Q2d,Q2,Q2e,L2d,L2,c2,s2avg",
				 "Pi2,NW2,Deb2,Deb2max,cred2,cred2c",
				 "mu2,p2,D2,S2,W2+Bon2,w2o,f2,N2" );
	}
}

double Lscale = VS( LABSUPL1, "Lscale" );		// labor scale
double wCap = VS( LABSUPL1, "wCap" );			// wage cap

int errors = 0;									// error counter
CYCLES( PARENT, cur, "Consumption" )
{
	double eta = VS( cur, "eta" );				// machine technical life
	double iota = VS( cur, "iota" );			// production slack
	double k2 = VS( cur, "k2" );				// product complexity
	double m2 = VS( cur, "m2" );				// capital productivity
	double mu20 = VS( cur, "mu20" );			// initial markup

	CYCLES( cur, cur1, "Firm2" )
	{
		j = VS( cur1, "_ID2" );
		h = VS( cur1, "_t2ent" );

		// print ID range
		if ( ( v[3] > 0 && ( j < v[3] || j > v[4] ) ) ||
			 ( v[3] < 0 && ID_IND( j ) != - v[3] ) )
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
						entr[ j ] = cur1;		// add entrant in to show list
				}
			}

		// scan vintages for severe problems
		double _L2 = VS( cur1, "_L2" );
		double _Q2e = VS( cur1, "_Q2e" );

		objVecT IDerr, tVintErr, LvintErr, 		// vectors to save error firms
				QvintErr, dLvintErr, noWrkVintErr;

		v[7] = v[8] = v[9] = v[10] = v[11] = 0;	// accumulators
		v[12] = v[13] = v[14] = v[15] = v[16] = v[17] = 0;
		cur3 = NULL;							// last vintage
		CYCLES( cur1, cur2, "Vint" )
		{
			i = COUNTS( cur2, "WrkV" ) * Lscale;

			v[7] += VS( cur2, "_nVint" );
			v[8] += VS( cur2, "_Qvint" );
			v[9] += i;

			v[21] = 0;
			CYCLES( cur2, cur3, "WrkV" )
			{
				v[21] += VS( SHOOKS( cur3 ), "_Q" ) * Lscale;
				v[10] += VS( SHOOKS( cur3 ), "_Q" ) * Lscale;
				v[11] += VS( SHOOKS( cur3 ), "_s" ) * Lscale;
				v[12] += VS( SHOOKS( cur3 ), "_w" ) * Lscale;
				v[13] += VS( SHOOKS( cur3 ), "_Bon" ) * Lscale;
			}

			if ( VS( cur2, "_IDvint" ) < h )
				IDerr.push_back( cur2 );

			if ( VS( cur2, "_tVint" ) < h - eta - 1 || VS( cur2, "_tVint" ) > T )
				tVintErr.push_back( cur2 );

			if ( i < 0 || i > _L2 )
				LvintErr.push_back( cur2 );

			if ( VS( cur2, "_Qvint" ) < 0 ||
				 ( i > 0 && VS( cur2, "_Qvint" ) <= 0 ) ||
				 floor( VS( cur2, "_Qvint" ) ) != floor( v[21] ) )
				QvintErr.push_back( cur2 );

			if ( VS( cur2, "_toUseVint" ) == 0 && VS( cur2, "_dLdVint" ) > 0 )
				dLvintErr.push_back( cur2 );

			if ( cur3 != NULL &&
				 VS( cur2, "_Qvint" ) > 0 && VS( cur3, "_Qvint" ) == 0 )
				noWrkVintErr.push_back( cur2 );

			cur3 = cur2;
		}

		CYCLES( cur1, cur3, "Wrk2" )
		{
			v[14] += VS( SHOOKS( cur3 ), "_Q" ) * Lscale;
			v[15] += VS( SHOOKS( cur3 ), "_s" ) * Lscale;
			v[16] += VS( SHOOKS( cur3 ), "_w" ) * Lscale;
			v[17] += VS( SHOOKS( cur3 ), "_Bon" ) * Lscale;
		}

		v[18] = COUNTS( cur1, "Broch" );
		v[19] = COUNTS( cur1, "Vint" );
		v[20] = COUNTS( cur1, "Wrk2" ) * Lscale;
		cur2 = HOOKS( cur1, TOPVINT );

		double _Bon2 = VS( cur1, "_Bon2" );
		double _CI2 = VS( cur1, "_CI2" );
		double _D2 = VS( cur1, "_D2" );
		double _D2e = VS( cur1, "_D2e" );
		double _Deb2 = VS( cur1, "_Deb2" );
		double _Deb2max = VS( cur1, "_Deb2max" );
		double _EI2 = VS( cur1, "_EI2" );
		double _EI2d = VS( cur1, "_EI2d" );
		double _K2 = VS( cur1, "_K2" );
		double _K2d = VS( cur1, "_K2d" );
		double _L2d = VS( cur1, "_L2d" );
		double _N2 = VS( cur1, "_N2" );
		double _NW2 = VS( cur1, "_NW2" );
		double _Pi2 = VS( cur1, "_Pi2" );
		double _Q2d = VS( cur1, "_Q2d" );
		double _Q2 = VS( cur1, "_Q2" );
		double _S2 = VS( cur1, "_S2" );
		double _SI2 = VS( cur1, "_SI2" );
		double _SI2d = VS( cur1, "_SI2d" );
		double _W2 = VS( cur1, "_W2" );
		double _c2 = VS( cur1, "_c2" );
		double _c2e = VS( cur1, "_c2e" );
		double _cred2 = VS( cur1, "_cred2" );
		double _cred2c = VS( cur1, "_cred2c" );
		double _f2 = VS( cur1, "_f2" );
		double _life2cycle = VS( cur1, "_life2cycle" );
		double _mu2 = VS( cur1, "_mu2" );
		double _p2 = VS( cur1, "_p2" );
		double _s2avg = VS( cur1, "_s2avg" );
		double _t2ent = VS( cur1, "_t2ent" );
		double _w2o = VS( cur1, "_w2o" );

		double _K2avb = VLS( cur1, "_K2", 1 );
		double _N2_1 = VLS( cur1, "_N2", 1 );
		double _w2oPast = VLS( cur1, "_w2o", 1 );
		double _pVint = ( cur2 != NULL && VS( cur2, "_tVint" ) == T ) ?
						VS( cur2, "_pVint" ) : 0;

		double nonNeg[ ] = { _Bon2, _CI2, _D2, _D2e, _Deb2, _Deb2max, _EI2,
							 _EI2d, _K2, _K2avb, _K2d, _L2, _L2d, _N2, _Q2d,
							 _Q2, _Q2e, _S2, _SI2, _SI2d, _W2, _cred2, _cred2c,
							 _c2, _c2e, _f2, _life2cycle, _s2avg, _pVint };
		double posit[ ] = { _mu2, _p2, _t2ent, _w2o };
		double finite[ ] = { _NW2, _Pi2 };

		dblVecT all ( nonNeg, END_ARR( nonNeg ) );
		all.insert( all.end( ), posit, END_ARR( posit ) );
		all.insert( all.end( ), finite, END_ARR( finite ) );

		// first period actions (single-firm analysis only)
		if ( k == 1 && T == v[1] )
		{
			iniK2 = _K2;
			iniNW2 = _NW2;
			iniDeb2 = _Deb2;
		}

		LOG( "\n  ## (t=%g) ID2=%d t2ent=%d life2cycle=%g #Broch=%g #Vint=%g #Wrk=%g pVint=%g",
			 T, j, h, _life2cycle, v[18], v[19], v[20], round( _pVint ) );
		fprintf( firms2, "%g,%d,%d,%g,%g,%g,%g,%g", T, j, h, _life2cycle, v[18],
				 v[19], v[20], _pVint );

		for ( i = 0; i < LEN_ARR( nonNeg ); ++i )
			CFUN( check_error, nonNeg[ i ] < 0, "NEGATIVE-VALUE", i + 1, & errors );

		for ( i = 0; i < LEN_ARR( posit ); ++i )
			CFUN( check_error, posit[ i ] <= 0, "NON-POSITIVE-VALUE", i + 1, & errors );

		for ( itd = all.begin( ); itd != all.end( ); ++itd )
			CFUN( check_error, ! isfinite( *itd ),
				  "NON-FINITE-VALUE", itd - all.begin( ) + 1, & errors );

		CFUN( check_error, v[18] == 0, "NO-BROCHURE", 0, & errors );

		// capital and investment
		LOG( "\n   # K2d=%g K2avb=%g EI2d=%g SI2d=%g EI2=%g SI2=%g CI2=%g K2=%g",
			 _K2d, _K2avb, _EI2d, _SI2d, _EI2, _SI2, _CI2, _K2 );
		fprintf( firms2, ",%g,%g,%g,%g,%g,%g,%g,%g",
			 _K2d, _K2avb, _EI2d, _SI2d, _EI2, _SI2, _CI2, _K2 );

		CFUN( check_error, _life2cycle > 0 && v[19] == 0, "NO-VINTAGE", 0, & errors );

		CFUN( check_error, IDerr.size( ) > 0,
			  "INVALID-ID-VINT", IDerr.size( ), & errors );

		CFUN( check_error, tVintErr.size( ) > 0,
			  "INVALID-T-VINT", tVintErr.size( ), & errors );

		CFUN( check_error, noWrkVintErr.size( ) > 0, "INCONSISTENT-VINT-PROD",
			  noWrkVintErr.size( ), & errors );

		CFUN( check_error, floor( _SI2d ) > _K2avb ||
			  floor( _K2 ) > _K2avb + _EI2 + _SI2 ||
			  floor( v[7] ) > _K2 || ( _life2cycle > 0 && _K2 == 0 ),
			  "INCONSISTENT-CAPITAL", 0, & errors );

		LOG( "\n   # D2e=%g Q2d=%g Q2=%g Q2e=%g L2d=%g L2=%g c2=%.2g s2avg=%.2g",
			 round( _D2e ), _Q2d, _Q2, round( _Q2e ), _L2d, _L2, _c2, _s2avg );
		fprintf( firms2, ",%g,%g,%g,%g,%g,%g,%g,%g",
			 _D2e, _Q2d, _Q2, _Q2e, _L2d, _L2, _c2, _s2avg );

		CFUN( check_error, ( _Q2d > 0 &&
			  floor( _Q2d ) > ceil( ( 1 + iota ) * _D2e / _p2 - _N2_1 ) ) ||
			  _Q2 > _Q2d || _Q2e > _Q2 ||
			  floor( _Q2e ) > _s2avg * _K2 * m2 / k2,
			  "INCONSISTENT-PRODUCTION", 0, & errors );

		CFUN( check_error, QvintErr.size( ) > 0,
			  "INVALID-PROD-VINT", QvintErr.size( ), & errors );

		CFUN( check_error, _L2d > 0 && _L2 == 0, "NO-WORKER", 0, & errors );

		CFUN( check_error, v[9] > _L2 || v[9] > v[20],
			  "INCONSISTENT-WORKER", 0, & errors );

		CFUN( check_error, LvintErr.size( ) > 0,
			  "INVALID-LABOR-VINT", LvintErr.size( ), & errors );

		CFUN( check_error, dLvintErr.size( ) > 0, "INVALID-VINT-DEMAND",
			  dLvintErr.size( ), & errors );

		CFUN( check_error, _life2cycle > 0 && ( _c2 == 0 || _c2e == 0 ),
			  "INCONSISTENT-COST", 0, & errors );

		CFUN( check_error, v[9] > 0 && ( v[11] / v[9] < _s2avg * ( 1 - TOL ) ||
			  v[11] / v[9] > _s2avg * ( 1 + TOL ) ),
			  "INCONSISTENT-SKILLS", 0, & errors );

		CFUN( check_error, _w2o == _w2oPast * wCap || _w2o == _w2oPast / wCap,
			  "LARGE-WAGE-OFFER-CHANGE", 0, & errors );

		CFUN( check_error, floor( v[10] ) != floor( v[14] ),
			  "INCONSISTENT-WORKER-PROD", 0, & errors );

		// finance
		LOG( "\n   # Pi2=%g NW2=%g Deb2=%g Deb2max=%g cred2=%g cred2c=%g",
			 round( _Pi2 ), round( _NW2 ), round( _Deb2 ), round( _Deb2max ),
			 round( _cred2 ), round( _cred2c ) );
		fprintf( firms2, ",%g,%g,%g,%g,%g,%g",
				 _Pi2, _NW2, _Deb2, _Deb2max, _cred2, _cred2c );

		// market
		LOG( "\n   # mu2=%.2g p2=%.2g D2=%g S2=%g W2+Bon2=%g w2o=%.3g f2=%.2g N2=%g",
			 _mu2, _p2, round( _D2 ), round( _S2 ), round( _W2 + _Bon2 ), _w2o,
			 _f2, round( _N2 ) );
		fprintf( firms2, ",%g,%g,%g,%g,%g,%g,%g,%g", _mu2, _p2, _D2, _S2,
				 _W2 + _Bon2, _w2o, _f2, _N2 );

		CFUN( check_error, _mu2 > 5 * mu20, "EXCESSIVE-MARKUP", 0, & errors );

		CFUN( check_error, _mu2 < mu20 / 5, "INSUFFICIENT-MARKUP", 0, & errors );

		CFUN( check_error, floor( v[12] + v[13] ) > v[16] + v[17] ||
			  floor( _W2 + _Bon2 ) != floor( v[16] + v[17] ),
			  "INCONSISTENT-PAYROLL", 0, & errors );

		// last period actions (single-firm analysis only)
		if ( k == 1 && T == v[2] )
		{
			LOG( "\n   # Kgwth=%.3g NW2gwth=%.3g Deb2gwth=%.3g",
				 ( log( _K2 + 1 ) - log( iniK2 + 1 ) ) / v[5],
				 ( log( _NW2 + 1 ) - log( iniNW2 + 1 ) ) / v[5],
				 ( log( _Deb2 + 1 ) - log( iniDeb2 + 1 ) ) / v[5] );
		}

		fputs( "\n", firms2 );
	}
}

if ( T == v[2] )
{
	LOG( "\n ### TESTING OF CONSUMER-GOOD FIRMS FINISHED" );
	fclose( firms2 );
	firms2 = NULL;
}

RESULT( errors )
