/******************************************************************************

	LABOR MARKET OBJECT EQUATIONS
	-----------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the Labor Market objects in the K+S LSD
	model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "CPIexp" )
/*
Consumer price index expectation of workers
*/

k = V( "Texp" );								// time horizon of expectation

for ( v[1] = 0, h = 1; h <= k; ++h )
	v[1] += VLS( CONSECL1, "CPI", h );			// sum CPI on time horizon

RESULT( v[1] / k )


EQUATION( "Ged" )
/*
Public expenditure on education
*/
RESULT( VS( COUNTRL1, "flagEduc" ) == 0 ? 0 :
		V( "epsilonEd" ) * VLS( COUNTRL1, "Y", 1 ) )


EQUATION( "Gtrain" )
/*
Public expenditure on unemployed worker training
*/
RESULT( V( "GammaCost" ) * V( "wAvg" ) * V( "Ltrain" ) )


EQUATION( "Ls" )
/*
Effective work force (labor) size
Increase workforce if full employment and flagAddWorkers is set to 1
Result is scaled according to the defined scale
*/

v[1] = V( "Lscale" );							// labor scaling
v[2] = V( "delta" );							// population growth rate

if ( v[2] <= -1 )								// just adjust to other countries?
{
	k = v[3] = 0;								// other countries accumulators
	CYCLES( GRANDPARENT, cur, "Country" )		// check what other countries do
	{
		cur1 = V_EXTS( cur, countryE, labSup );	// country's labor supply object

		if ( VS( cur1, "delta" ) <= -1 )		// dependent-growth country?
			++k;								// countries to split change
		else
			v[3] += VLS( cur1, "Ls", 1 ) - VS( cur1, "Ls" );// independent change
	}

	v[0] = CURRENT + v[3] / k;					// split the adjustment
}
else
	v[0] = CURRENT * ( 1 + v[2] );				// just update population

i = COUNT( "Worker" );							// count the worker objects
h = i * v[1];									// adjust for scale

// growing workforce and demand is higher than labor supply?
if ( VS( CAPSECL1, "L1d" ) + VS( CONSECL1, "L2d" ) > h )
	v[0] *= 1 + V( "deltaF" );					// lump grow in labor supply

j = max( ceil( v[0] / v[1] ), 1 ) * v[1];		// rounded-up effective population

if ( j > h )									// growing?
{
	v[3] = V( "wU" );							// unemployed wage (benefit)
	v[4] = V( "Tc" );							// contract term
	v[5] = max( V( "Ts" ), V( "Texp" ) );		// wage or expectation memory
	v[6] = VS( COUNTRL1, "flagWorkerLBU" );		// learning mode

	if ( v[6] == 1 || v[6] == 3 )				// vintage skills in use?
		v[7] = V( "sigma" );					// public vintage skills
	else
		v[7] = INISKILL;

	for ( ; j > h ; h += v[1] )					// add missing workers
	{
		cur = ADDOBJL( "Worker", T - 1 );		// insert object to be updated
		ADDHOOKS( cur, WORKERHK );				// add object hooks
		CFUNS( cur, set_education );			// draw education

		WRITES( cur, "_ID", ++i );				// new ID
		WRITES( cur, "_Tc", v[4] );				// set work contract term,
		WRITES( cur, "_emig", 0 );
		WRITES( cur, "_employed", 0 );

		for ( i = 1; i <= v[5]; ++i )			// fill multi-lag variables
		{
			WRITELLS( cur, "_CdReal", v[3], T - 1, i );// desired cons. memory
			WRITELLS( cur, "_w", v[3], T - 1, i );// wage memory
		}

		WRITELLS( cur, "_sV", v[7], T - 1, 1 );
		WRITELLS( cur, "_sT", INISKILL, T - 1, 1 );
	}
}
else
	for ( k = 0; j < h && k < i; ++k )			// shrink labor force if needed
	{
		cur = RNDDRAW_FAIR( "Worker" );			// pick worker at chance
		if ( ! VS( cur, "_emig" ) )				// not already emigrating?
		{
			WRITES( cur, "_emig", 1 );			// mark for emigration
			h -= v[1];							// account scaled emigrants
		}
	}

RESULT( j )


EQUATION( "emig" )
/*
Number of leaving emigrants
Also updates '_emigTrf'
*/

VS( CAPSECL1, "fires1" );						// make sure firing is done
VS( CONSECL1, "fires2" );						// in both sectors

v[1] = V( "Lscale" );							// labor scaling
v[2] = V( "Ls" );								// staying workers

v[0] = v[3] = 0;								// emigrant/savings counters
CYCLE_SAFE( cur, "Worker" )
	if ( VS( cur, "_emig" ) )					// emigrating?
	{
		v[3] += VLS( cur, "_SavAcc", 1 );		// worker savings
		DELETE( cur );							// remove from country
		v[0] += v[1];
	}

v[3] *= v[1] / v[2];							// share of savings to transfer
CYCLE( cur, "Worker" )
	WRITES( cur, "_emigTrf", v[3] );			// transfer $ to staying workers

RESULT( v[0] )


EQUATION( "searchProb" )
/*
Probability of searching for job in period (global)
*/

if ( VS( COUNTRL1, "flagSearchDisc" ) == 1 )	// global discouragement mode?
{
	v[1] = V( "kappa" );
	v[2] = VL( "Ue", 1 );						// unemployment rate
	v[0] = v[1] * exp( - v[1] * v[2] );			// compute global prob.
}
else
	v[0] = 1;									// no: always search

RESULT( v[0] )


EQUATION( "wCent" )
/*
Centralized wage imposed to all workers
Only used if 'flagHeterWage' is set to zero
*/

if ( VS( COUNTRL1, "flagHeterWage" ) == 0 )
{
	v[1] = V( "psi1" );							// inflation adjust. parameter
	v[2] = V( "psi2" );							// general prod. adjust. param.
	v[3] = V( "psi3" );							// unemploym. adjust. parameter
	v[4] = VS( FINSECL1, "piT" );				// target inflation
	v[5] = VLS( CONSECL1, "dCPIb", 1 );			// current inflation
	v[6] = VLS( COUNTRL1, "dAb", 1 );			// general productivity variat.
	v[7] = VL( "dUeB", 1 );						// unemployment variation

	v[0] = CURRENT * ( 1 + v[4] + v[1] * ( v[5] - v[4] ) + v[2] * v[6] +
					   v[3] * v[7] );
	v[0] = max( v[0], V( "wMinPol" ) );			// adjust to minimum if needed
}
else
	v[0] = VL( "wAvg", 1 );						// simply equal to mkt avg

RESULT( v[0] )


EQUATION( "wMinPol" )
/*
Minimum wage set by government policy
*/

v[0] = CURRENT;									// previous period min wage
v[4] = VS( COUNTRL1, "flagIndexMinWage" );		// flag and weight for indexat.
v[8] = V( "w0min" );							// absolute minimum

if ( v[4] != 0 )								// if minimum wage is indexed
{												// and it is time
	v[1] = V( "psi1" );							// inflation adjust. parameter
	v[2] = V( "psi2" );							// general prod. adjust. param.
	v[3] = V( "psi3" );							// unemploym. adjust. parameter
	v[5] = VLS( CONSECL1, "dCPIb", 1 );			// inflation variation
	v[6] = VLS( COUNTRL1, "dAb", 1 );			// general productivity variat.
	v[7] = VL( "dUeB", 1 );						// unemployment variation

	v[0] *= 1 + ( v[1] * v[5] + v[2] * v[6] + v[3] * v[7] ) * v[4];
}

RESULT( v[0] < v[8] ? v[8] : v[0] )


EQUATION( "wU" )
/*
Unemployment benefit ("wage") paid by government (or minimum income otherwise)
*/
RESULT( VS( COUNTRL1, "flagGovExp" ) >= 2 ? V( "phi" ) * VL( "wAvg", 1 ) :
											V( "w0min" ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "Bon" )
/*
Total (nominal) bonuses
*/
VS( CONSECL1, "hires2" );						// ensure hires are done
RESULT( SUM( "_Bon" ) * V( "Lscale" ) )


EQUATION( "CdW" )
/*
Desired consumption (in money terms) of workers
*/
VS( CONSECL1, "hires2" );						// ensure hires are done
RESULT( SUM( "_Cd" ) * V( "Lscale" ) )


EQUATION( "Cw" )
/*
Effective nominal consumption (in money terms) of workers
*/
VS( CONSECL1, "D2" );							// ensure shortages allocated
RESULT( SUM( "_C" ) * V( "Lscale" ) )


EQUATION( "DivW" )
/*
Total dividends received by workers
*/
VS( CONSECL1, "hires2" );						// ensure hires are done
RESULT( SUM( "_Div" ) * V( "Lscale" ) )


EQUATION( "EqW" )
/*
Equity from firms hold by workers
This variable must be explicitly recalculated after entry/exit
*/
RESULT( SUM( "_Eq" ) * V( "Lscale" ) )


EQUATION( "EqEntryW" )
/*
Cost of new equity from firm entries to workers
This variable must be explicitly recalculated after entry/exit
*/
RESULT( SUM( "_EqEntry" ) * V( "Lscale" ) )


EQUATION( "Gtrf" )
/*
Government transfers (in money terms) to workers
*/
VS( CONSECL1, "hires2" );						// ensure hires are done
RESULT( SUM( "_Gtrf" ) * V( "Lscale" ) )


EQUATION( "In" )
/*
Income received by workers (including bonus and unemployment benefits)
*/
VS( CONSECL1, "hires2" );						// ensure hires are done
RESULT( SUM( "_In" ) * V( "Lscale" ) )


EQUATION( "L" )
/*
Labor aggregated demand
*/
RESULT( VS( CAPSECL1, "L1" ) + VS( CONSECL1, "L2" ) )


EQUATION( "Ltrain" )
/*
Number of workers under government-supplied training
*/
RESULT( ( V( "Ls" ) - V( "L" ) ) * V( "Gamma" ) )// workers under training


EQUATION( "NWexitW" )
/*
Residual net worth from equity hold on exiting private firms
This variable must be explicitly recalculated after entry/exit
*/
RESULT( SUM( "_NWexit" ) * V( "Lscale" ) )


EQUATION( "Sav" )
/*
Savings of workers (in money terms) in period
*/
V( "Cw" );										// ensure consumption done
RESULT( SUM( "_Sav" ) * V( "Lscale" ) )


EQUATION( "SavAcc" )
/*
Accumulated savings (in money terms) of workers at the end of the period
*/
V( "Sav" );										// ensure saving done
RESULT( ROUND( SUM( "_SavAcc" ) * V( "Lscale" ), 0, 0.01 ) )


EQUATION( "TaxW" )
/*
Total taxes paid by workers on income
*/
V( "Cw" );										// ensure consumption done
RESULT( SUM( "_Tax" ) * V( "Lscale" ) )


EQUATION( "Ue" )
/*
Effective unemployment rate excluding discouraged workers
Also updates:
	U: Unemployment rate including discouraged workers
	U1, U2, U3: Category-level unemployment rate including discouraged
	Us: Short term unemployment rate (workers unemployed for less than 1 period)
	TeAvg: Average number of periods in the current job
*/

VS( CONSECL1, "hires2" );						// ensure hiring done

v[7] = V( "Ls" );								// available workers
v[8] = V( "Lscale" );							// labor scaling

h = i = j = k = v[1] = v[2] = v[3] = 0;			// count unemployed by type
v[4] = v[5] = v[6] = 0;							// available category workers
CYCLE( cur, "Worker" )
{
	v[9] = VS( cur, "_cat" );					// worker category

	if ( ! VS( cur, "_employed" ) )
	{
		++h;									// not employed

		if ( VS( cur, "_discouraged" ) )
			++i;								// and not searching for work

		if ( VS( cur, "_Tu" ) <= 1 )
			++j;								// and briefly unemployed

		++ v[ ( int ) v[9] ];					// by-category unemployed
	}
	else
		k += VS( cur, "_Te" );					// add time in job

	++ v[ ( int ) v[9] + 3 ];					// by category workers
}

v[10] = v[7] - ( h - j ) * v[8];				// long-term unemployed workers
v[11] = v[7] - h * v[8];						// employed workers
v[12] = v[7] - i * v[8];						// non-discouraged total workers

WRITE( "U", h * v[8] / v[7] );
WRITE( "Us", v[10] > 0 ? j * v[8] / v[10] : 1 );
WRITE( "TeAvg", v[11] > 0 ? k * v[8] / v[11] : 0 );
WRITES( LABSTAL1, "U1", v[4] > 0 ? v[1] / v[4] : 0 );
WRITES( LABSTAL1, "U2", v[5] > 0 ? v[2] / v[5] : 0 );
WRITES( LABSTAL1, "U3", v[6] > 0 ? v[3] / v[6] : 0 );

RESULT( v[12] > 0 ? ( h - i ) * v[8] / v[12] : 1 )


EQUATION( "W" )
/*
Total (nominal) wages (excluding bonus & unemployment benefits)
*/
VS( CONSECL1, "hires2" );						// ensure hiring done
RESULT( SUM( "_w" ) * V( "Lscale" ) )


EQUATION( "appl" )
/*
Number of worker applications for jobs
*/
RESULT( SUM( "_appl" ) * V( "Lscale" ) )


EQUATION( "dUeB" )
/*
Notional unemployment (bounded) rate of change
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "Ue", VS( COUNTRL1, "mLim" ), VS( COUNTRL1, "mPer" ) ) )


EQUATION( "iDw" )
/*
Interest received from savings by workers
*/
V( "Sav" );										// ensure saving done
RESULT( SUM( "_iD" ) * V( "Lscale" ) )


EQUATION( "sAvg" )
/*
Average workers compound skills in all sectors
Also updates:
	sTavg: average workers tenure skills in all sectors
	sTsd: workers tenure skills standard deviation in all sectors
	sTmax: maximum workers tenure skills in all sectors
	sTmin: minimum workers tenure skills in all sectors
	sVavg: average workers vintage skills in all sectors
	sVsd: workers vintage skills standard deviation in all sectors
*/

VS( CONSECL1, "hires2" );						// ensure hiring done

h = VS( COUNTRL1, "flagWorkerLBU" );			// learning-by-use mode
if ( h == 0 )									// no worker-level learning?
	END_EQUATION( INISKILL );

v[0] = v[1] = v[2] = v[3] = v[4] = i = 0;		// accumulators
v[5] = 0;										// current maximum
v[6] = DBL_MAX;									// current minimum
CYCLE( cur, "Worker" )
{
	v[0] += VS( cur, "_s" );
	++i;

	if ( h >= 2 )
	{
		v[8] = VS( cur, "_sT" );
		v[1] += v[8];							// sum of tenure skills
		v[2] += pow( v[8], 2 );					// sum of square skills

		if ( v[8] > v[5] )
			v[5] = v[8];						// new maximum

		if ( v[8] < v[6] )
			v[6] = v[8];						// new minimum
	}

	if ( h == 1 || h == 3 )
	{
		v[9] = VS( cur, "_sV" );

		v[3] += v[9];							// sum of vintage skills
		v[4] += pow( v[9], 2 );					// sum of square skills
	}
}

v[0] /= i;

if ( h >= 2 )
{
	WRITE( "sTavg", v[1] / i );
	WRITE( "sTsd", sqrt( max( ( v[2] / i ) - pow( v[1] / i, 2 ), 0 ) ) );
	WRITE( "sTmax", v[5] );
	WRITE( "sTmin", v[6] );
}
else
{
	WRITE( "sTavg", INISKILL );
	WRITE( "sTsd", 0 );
	WRITE( "sTmax", INISKILL );
	WRITE( "sTmin", INISKILL );
}

if ( h == 1 || h == 3 )
{
	WRITE( "sVavg", v[3] / i );
	WRITE( "sVsd", sqrt( max( ( v[4] / i ) - pow( v[3] / i, 2 ), 0 ) ) );
}
else
{
	WRITE( "sVavg", INISKILL );
	WRITE( "sVsd", 0 );
}

RESULT( v[0] )


EQUATION( "wAvg" )
/*
Average wage received by workers (excluding bonus & unemployment benefits)
*/
VS( CONSECL1, "hires2" );						// ensure hiring done
v[0] = AVE( "_w" );
RESULT( ! isnan( v[0] ) ? v[0] : CURRENT )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "EqAlloc" )
/*
Allocate destroyed/created equity & liq. net worth from exit/entry to workers
*/

v[1] = VS( CAPSECL1, "NW1exitW" ) + VS( CONSECL1, "NW2exitW" );// liq. net worth
v[2] = VS( CAPSECL1, "Eq1exitW" ) + VS( CONSECL1, "Eq2exitW" );// eq. destroyed
v[3] = VS( CAPSECL1, "Eq1entryW" ) + VS( CONSECL1, "Eq2entryW" );// eq. created
v[4] = VL( "EqW", 1 );							// current shareholder equity

v[5] = SUM_CND( "_SavAcc", "_SavAcc", ">", 0 ) * V( "Lscale" );
												// total positive savings
if ( v[4] == 0 )								// handle no equity case
{
	v[6] = 1 / V( "Ls" );						// split exit NW equally
	v[7] = 0;									// zero total equity
}
else
	v[8] = 1 - v[2] / v[4];						// equity shrink factor

if ( v[5] == 0 )								// handle no savings case
	v[9] = 1 / V( "Ls" );						// split new equity equally

CYCLE( cur, "Worker" )							// update each worker
{
	if ( v[4] > 0 )								// is equity being used?
	{
		v[10] = VLS( cur, "_Eq", 1 );			// current equity
		v[6] = v[10] / v[4];					// current equity share
		v[7] = v[8] * v[10];					// equity after exits
	}

	if ( v[5] > 0 )								// are savings available?
		v[9] = max( VS( cur, "_SavAcc" ), 0 ) / v[5];

	v[11] = v[9] * v[3];						// new equity because entry

	WRITES( cur, "_NWexit", v[6] * v[1] );		// paid liquidation net worth
	WRITES( cur, "_EqEntry", v[11] );
	WRITES( cur, "_Eq", v[7] + v[11] );			// equity after exits & entries
}

// update again variables affected by entry/exit
RECALC( "EqEntryW" );
RECALC( "NWexitW" );
RECALC( "EqW" );

RESULT( v[1] - v[2] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "TeAvg", "" )
/*
Average number of periods in the current job
Updated in 'Ue'
*/

EQUATION_DUMMY( "U", "" )
/*
Unemployment rate including discouraged workers
Updated in 'Ue'
*/

EQUATION_DUMMY( "Us", "" )
/*
Short term unemployment rate (workers unemployed for less than 1 period)
Updated in 'Ue'
*/

EQUATION_DUMMY( "sTavg", "sAvg" )
/*
Average workers tenure skills in all sectors
Updated in 'sAvg'
*/

EQUATION_DUMMY( "sTmax", "sAvg" )
/*
Maximum workers tenure skills in all sectors
Updated in 'sAvg'
*/

EQUATION_DUMMY( "sTmin", "sAvg" )
/*
Minimum workers tenure skills in all sectors
Updated in 'sAvg'
*/

EQUATION_DUMMY( "sTsd", "sAvg" )
/*
Workers tenure skills standard deviation in all sectors
Updated in 'sAvg'
*/

EQUATION_DUMMY( "sVavg", "sAvg" )
/*
Average workers vintage skills in all sectors
Updated in 'sAvg'
*/

EQUATION_DUMMY( "sVsd", "sAvg" )
/*
Workers vintage skills standard deviation in all sectors
Updated in 'sAvg'
*/
