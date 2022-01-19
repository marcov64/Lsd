/******************************************************************************

	LABOR MARKET OBJECT EQUATIONS
	-----------------------------

	Equations that are specific to the Labor Market objects in the K+S LSD
	model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "Gtrain" )
/*
Public expenditure on unemployed worker training
*/
RESULT( V( "Ltrain" ) * V( "GammaCost" ) * V( "wAvg" ) )


EQUATION( "Ls" )
/*
Effective work force (labor) size
Increase workforce if full employment and flagAddWorkers is set to 1
Result is scaled according to the defined scale
*/

v[1] = V( "Lscale" );							// labor scaling
i = COUNT( "Worker" );							// count the worker objects
h = i * v[1];									// adjust for scale

v[0] = V( "delta" );							// population growth rate

// growing workforce and demand is higher than labor supply?
if ( VS( PARENT, "flagAddWorkers" ) == 1 &&
	 VS( CAPSECL1, "L1d" ) + VS( CONSECL1, "L2d" ) > h )
	v[0] += 0.01;								// lump grow in labor supply

j = h * ( 1 + v[0] );							// grow population

v[2] = V( "wU" );								// unemployed wage (benefit)
v[3] = V( "Tc" );								// contract term
v[4] = V( "sigma" );							// public vintage skills
v[5] = VS( PARENT, "flagWorkerLBU" );			// learning mode

if ( v[5] == 1 || v[5] == 3 )					// vintage skills in use?
	v[6] = v[4];								// get public skills
else
	v[6] = INISKILL;

for ( ; h < j ; h += v[1] )						// add missing workers
{
	cur = ADDOBJL( "Worker", T - 1 );			// insert object to be updated
	ADDHOOKS( cur, WORKERHK );					// add object hooks

	WRITES( cur, "_ID", ++i );					// new ID
	WRITES( cur, "_Tc", v[3] );					// set work contract term,
	WRITES( cur, "_wRes", v[2] );				// reservation wage
	WRITES( cur, "_employed", 0 );

	for ( i = 1; i <= 8; ++i )					// lagged wage memory
		WRITELLS( cur, "_w", v[2], T - 1, i );

	WRITELLS( cur, "_sV", v[6], T - 1, 1 );
	WRITELLS( cur, "_sT", INISKILL, T - 1, 1 );
}

RESULT( h )


EQUATION( "searchProb" )
/*
Probability of searching for job in period (global)
*/

if ( VS( PARENT, "flagSearchDisc" ) == 1 )		// global discouragement mode?
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

if ( VS( PARENT, "flagHeterWage" ) == 0 )
{
	v[1] = V( "psi1" );							// inflation adjust. parameter
	v[2] = V( "psi2" );							// general prod. adjust. param.
	v[3] = V( "psi3" );							// unemploym. adjust. parameter
	v[4] = VS( FINSECL1, "piT" );				// target inflation
	v[5] = VLS( CONSECL1, "dCPIb", 1 );			// current inflation
	v[6] = VLS( PARENT, "dAb", 1 );				// general productivity variat.
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
v[4] = VS( PARENT, "flagIndexMinWage" );		// flag and weight for indexat.
v[8] = V( "w0min" );							// absolute minimum

if ( v[4] != 0 )								// if minimum wage is indexed
{												// and it is time
	v[1] = V( "psi1" );							// inflation adjust. parameter
	v[2] = V( "psi2" );							// general prod. adjust. param.
	v[3] = V( "psi3" );							// unemploym. adjust. parameter
	v[5] = VLS( CONSECL1, "dCPIb", 1 );			// inflation variation
	v[6] = VLS( PARENT, "dAb", 1 );				// general productivity variat.
	v[7] = VL( "dUeB", 1 );						// unemployment variation

	v[0] *= 1 + ( v[1] * v[5] + v[2] * v[6] + v[3] * v[7] ) * v[4];
}

RESULT( v[0] < v[8] ? v[8] : v[0] )


EQUATION( "wU" )
/*
Unemployment benefit ("wage") paid by government
*/
RESULT( V( "phi" ) * VL( "wAvg", 1 ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "Bon" )
/*
Total (nominal) bonuses
*/
RESULT( SUM( "_Bon" ) * V( "Lscale" ) )


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


EQUATION( "TaxW" )
/*
Total taxes paid by workers on wages
*/
RESULT( SUM( "_TaxW" ) )


EQUATION( "Ue" )
/*
Effective unemployment rate excluding discouraged workers
Also updates:
	U: Unemployment rate including discouraged workers
	Us: Short term unemployment rate (workers unemployed for less than 1 period)
	TeAvg: Average number of periods in the current job
*/

v[1] = V( "Ls" );								// available workers
v[2] = V( "Lscale" );							// labor scaling

h = i = j = k = 0;								// count unemployed by type
CYCLE( cur, "Worker" )
	if ( ! VS( cur, "_employed" ) )
	{
		++h;									// not employed

		if ( VS( cur, "_discouraged" ) )
			++i;								// and not searching for work

		if ( VS( cur, "_Tu" ) <= 1 )
			++j;								// and briefly unemployed
	}
	else
		k += VS( cur, "_Te" );					// add time in job

v[3] = v[1] - ( h - j ) * v[2];					// long-term unemployed workers
v[4] = v[1] - h * v[2];							// employed workers
v[5] = v[1] - i * v[2];							// non-discouraged total workers

WRITE( "U", h * v[2] / v[1] );
WRITE( "Us", v[3] > 0 ? j * v[2] / v[3] : 1 );
WRITE( "TeAvg", v[4] > 0 ? k * v[2] / v[4] : 0 );

RESULT( v[5] > 0 ? ( h - i ) * v[2] / v[5] : 1 )


EQUATION( "W" )
/*
Total (nominal) wages
*/
RESULT( SUM_CND( "_w", "_employed", ">", 0 ) * V( "Lscale" ) )


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
RESULT( mov_avg_bound( THIS, "Ue", VS( PARENT, "mLim" ), VS( PARENT, "mPer" ) ) )


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

h = VS( PARENT, "flagWorkerLBU" );				// learning-by-use mode
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
	v[0] = INISKILL;
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
	v[0] = INISKILL;
	WRITE( "sVavg", INISKILL );
	WRITE( "sVsd", 0 );
}

RESULT( v[0] )


EQUATION( "wAvg" )
/*
Average wage received by workers (excluding bonus & unemployment benefits)
*/
v[0] = AVE_CND( "_w", "_employed", ">", 0 );
RESULT( ! isnan( v[0] ) ? v[0] : CURRENT )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/


/*============================= DUMMY EQUATIONS ==============================*/

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
