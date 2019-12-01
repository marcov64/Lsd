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
RESULT( V( "Ltrain" ) * V( "GammaCost" ) * V( "wAvgEmp" ) )


EQUATION( "Ls" )
/*
Effective work force (labor) size, increase workforce if full employment and
flagAddWorkers is set to 1
Result is scaled according to the defined scale
*/

v[1] = V( "Lscale" );							// labor scaling
i = COUNT( "Worker" );							// count the worker objects
h = i * v[1];									// adjust for scale
	
v[0] = V( "delta" );							// population growth rate

// growing workforce and demand is higher than labor supply?
if ( VS( PARENT, "flagAddWorkers" ) == 1 && 
	 VS( CAPSECL1, "L1d" ) + VS( CONSECL1, "L2d" ) > h )
	v[0] += 0.02;								// lump grow in labor supply
	
j = h * ( 1 + v[0] );							// grow population

v[2] = V( "wU" );								// unemployed wage (benefit)
v[3] = V( "Tc" );								// contract term
v[4] = V( "sigma" );							// public vintage skills
v[5] = VS( PARENT, "flagWorkerLBU" );			// learning mode
	
for ( ; h < j ; h += v[1] ) 					// add missing workers
{	
	cur = ADDOBJL( "Worker", t - 1 );			// insert object to be updated
	ADDHOOKS( cur, WORKERHK );					// add object hooks
	
	WRITES( cur, "_ID", ++i );					// new ID
	WRITES( cur, "_Tc", v[3] );					// set work contract term,
	WRITES( cur, "_wRes", v[2] );				// reservation wage
	WRITES( cur, "_employed", 0 );
	
	for ( i = 1; i <= 8; ++i )					// lagged wage memory
		WRITELLS( cur, "_w", v[2], t - 1, i );
	
	if ( v[5] == 1 || v[5] == 3 )				// vintage skills in use?
		v[6] = v[4];							// get public skills
	else
		v[6] = 1;
	
	WRITES( cur, "_sV", v[6] );
	WRITES( cur, "_s", v[6] );
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
	v[1] = VS( LABSUPL2, "psi1" );				// inflation adjust. parameter
	v[2] = VS( LABSUPL2, "psi2" );				// general prod. adjust. param.
	v[3] = VS( LABSUPL2, "psi3" );				// unemploym. adjust. parameter
	v[5] = VLS( CONSECL1, "dCPIb", 1 );			// inflation variation
	v[6] = VLS( PARENT, "dAb", 1 );				// general productivity variat.
	v[7] = VL( "dUeB", 1 );						// unemployment variation

	v[0] = CURRENT * ( 1 + v[1] * v[5] + v[2] * v[6] + v[3] * v[7] );
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
RESULT( VS( FINSECL1, "phi" ) * VL( "wAvgEmp", 1 ) )// fraction of last avg wage


/*============================ SUPPORT EQUATIONS =============================*/

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

WRITE( "U", h * v[2] / v[1] );
WRITE( "Us", j * v[2] / ( v[1] - ( h - j ) ) );
WRITE( "TeAvg", k / ( v[1] - h ) );
	
RESULT( ( h - i ) * v[2] / ( v[1] - i ) )


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
RESULT( mov_avg_bound( p, "Ue", VS( PARENT, "mLim" ) ) )


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
	END_EQUATION( 1 );
	
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

if ( h >= 2 )
{
	WRITE( "sTavg", v[1] / i );
	WRITE( "sTsd", sqrt( max( ( v[2] / i ) - pow( v[1] / i, 2 ), 0 ) ) );
	WRITE( "sTmax", v[5] );
	WRITE( "sTmin", v[6] );
}
else
{
	WRITE( "sTavg", 1 );
	WRITE( "sTsd", 0 );
	WRITE( "sTmax", 1 );
	WRITE( "sTmin", 1 );
}
	
if ( h == 1 || h == 3 )
{
	WRITE( "sVavg", v[3] / i );
	WRITE( "sVsd", sqrt( max( ( v[4] / i ) - pow( v[3] / i, 2 ), 0 ) ) );
}
else
{
	WRITE( "sVavg", 1 );
	WRITE( "sVsd", 0 );
}
	
RESULT( v[0] / i )


EQUATION( "wAvg" )
/*
Average wages received by workers (including unemployment benefits)
Also updates other worker level wage statistics at once:
	wAvgEmp: average wage of employed workers (excluding unemployment benefits)
	wLogSDemp: employed workers wage standard deviation of wage log
*/

i = j = 0;										// counters
v[1] = v[2] = v[3] = v[4] = 0;					// accumulators
CYCLE( cur, "Worker" )							// consider all workers
{
	v[0] = VS( cur, "_w" );						// current wage or un. benefit
	v[1] += v[0];								// sum of wages & benefits
	
	if ( VS( cur, "_employed" ) )				// account only employed
	{
		v[2] += v[0];							// sum of wages
		v[3] += log( v[0] + 1 );				// sum of log wages
		v[4] += pow( log( v[0] + 1 ), 2 );		// sum of squared log wages
		
		++j;
	}
	
	++i;
}

if ( j > 0 )
{
	WRITE( "wAvgEmp", v[2] / j );
	WRITE( "wLogSDemp", sqrt( max( ( v[4] / j ) - pow( v[3] / j, 2 ), 0 ) ) );
}
else
{
	WRITE( "wAvgEmp", CURRENT );
	WRITE( "wLogSDemp", 0 );
}

RESULT( i > 0 ? v[1] / i : CURRENT )


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

EQUATION_DUMMY( "wAvgEmp", "" )
/*
Average wage received by employed workers (excluding unemployment benefits)
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wLogSDemp", "" )
/*
Employed workers wage standard deviation of wage log
Updated in 'wAvg'
*/
