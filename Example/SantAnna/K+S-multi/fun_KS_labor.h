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

EQUATION( "Gtrain" )
/*
Public expenditure on unemployed worker training
*/
RESULT( V( "Ltrain" ) * V( "GammaCost" ) * V( "wAvg" ) )


EQUATION( "Ls" )
/*
Effective work force (labor) size
Workforce grows if full employment and 'flagAddWorkers' is set to 1
Updates 'cEntry'
*/

v[1] = V( "delta" );							// population growth rate

// growing workforce and effective unemployment is lower than 5%?
if ( VS( PARENT, "flagAddWorkers" ) == 1 && VL( "Ue", 1 ) < 0.01 && T > 1 )
	v[1] += 0.01;								// lump grow in labor supply

v[2] = ceil( CURRENT * v[1] );					// population growth

if ( v[2] > 0 )
	v[3] = CFUN( entry_worker, v[2], false );	// add worker objects
else
	v[3] = 0;

WRITES( PARENT, "cEntry", v[3] );

RESULT( CURRENT + v[2] )


EQUATION( "hires" )
/*
Number of workers hired by firms in all sectors
Process required hiring using the appropriate rule
*/

v[1] = SUMS( PARENT, "hires1" );				// sector 1 hiring first
v[2] = V( "Lscale" );							// labor scaling

k = VS( PARENT, "flagHeterWage" );				// heterogeneous wages flag
h = ( k == 0 ) ? 0 : VS( PARENT, "flagHireSeq" );// firm hiring order

// create pointer and sort wage offers list
woLisT *offers = & V_EXTS( PARENT, countryE, firm2wo );
CFUN( order_offers, h, offers );

// firms hire employees according to the selected hiring order
for ( i = 0, itw = offers->begin( ); itw != offers->end( ); ++itw )
{
	bool _post2chg = VS( itw->firm, "_post2chg" );
	v[3] = _post2chg ? VS( PARENT, "flagWageOfferChg" ) :
					   VS( PARENT, "flagWageOffer" );

	if ( k == 0 || v[3] == 0 )					// avoid re-sorting the applics.
	{
		// sort firm's candidate list according to the defined strategy
		int hireOrder = _post2chg ? VS( PARENT, "flagHireOrder2Chg" ) :
								 VS( PARENT, "flagHireOrder2" );
		CFUN( order_applications, hireOrder, & V_EXTS( itw->firm, firm2E, appl ) );
	}

	// hire the ordered applications until queue is exhausted
	j = ceil( VS( itw->firm, "_JO2" ) / v[2] );	// firm's jobs open (scaled)
	h = 0;										// firm hiring counter
	v[4] = DBL_MAX;								// minimum wage requested
	cur = NULL;									// pointer to worker asking it

	while ( j > 0 && EXEC_EXTS( itw->firm, firm2E, appl, size ) > 0 )
	{
		// get the candidate worker object element and a pointer to it
		const application candidate = EXEC_EXTS( itw->firm, firm2E, appl, front );

		// candidate not yet hired in this period and offered wage ok?
		v[5] = VS( candidate.wrk, "_employed" );
		if ( ! ( v[5] && VS( candidate.wrk, "_Te" ) == 0 ) )
		{
			if ( ROUND( candidate.w, itw->offer, 0.01 ) <= itw->offer )
			{
				// already employed? First quit current job
				if ( v[5] > 0 )
					CFUNS( candidate.wrk, quit_worker );

				// flag hiring and set wage, employer & vintage to be used by worker
				CFUNS( candidate.wrk, hire_worker, 2, itw->firm, itw->offer );
				++i;							// scaled count hire (total)
				++h;							// scaled count hire (firm)
				--j;							// adjust scaled labor demand
			}
			else
				if ( candidate.w < v[4] )
				{
					v[4] = candidate.w;
					cur = candidate.wrk;
				}
		}

		// remove worker from candidate queue
		EXEC_EXTS( itw->firm, firm2E, appl, pop_front );
	}

	// try to hire at least one worker, at any wage
	if ( j > 0 && h == 0 && cur != NULL )		// none hired but someone avail?
	{
		if ( VS( cur, "_employed" ) )			// quit job if needed
			CFUNS( cur, quit_worker );

		CFUNS( cur, hire_worker, 2, itw->firm, v[4] );// pay requested wage
		++i;
		++h;
	}

	WRITES( itw->firm, "_hires2", h * v[2] );	// update hires count for firm
	RECALCS( PARENTS( itw->firm ), "quits2" );	// force update of industry quits
	EXEC_EXTS( itw->firm, firm2E, appl, clear );// clear application queue
}

offers->clear( );								// clear offer set

RESULT( v[1] + i * v[2] )


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
	v[5] = VLS( PARENT, "dCPIb", 1 );			// current inflation
	v[6] = VLS( PARENT, "dAb", 1 );				// general productivity variat.
	v[7] = VL( "dUeB", 1 );						// unemployment variation

	v[0] = CURRENT * ( 1 + v[4] + v[1] * ( v[5] - v[4] ) + v[2] * v[6] + v[3] * v[7] );
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
	v[5] = VLS( PARENT, "dCPIb", 1 );			// inflation variation
	v[6] = VLS( PARENT, "dAb", 1 );				// general productivity variat.
	v[7] = VL( "dUeB", 1 );						// unemployment variation

	v[0] *= 1 + ( v[1] * v[5] + v[2] * v[6] + v[3] * v[7] ) * v[4];
}

RESULT( v[0] < v[8] ? v[8] : v[0] )


EQUATION( "wU" )
/*
Unemployment benefit ("wage") paid by government
*/
RESULT( VS( FINSECL1, "phi" ) * VL( "wAvg", 1 ) )// fraction of last avg wage


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "Bon" )
/*
Total bonuses paid by firms to workers
*/
RESULT( SUMS( PARENT, "Bon2" ) )


EQUATION( "InLux" )
/*
Income received by workers buying luxury goods
(including bonus and unemployment benefit)
*/
RESULT( PERC( "_In", V( "phiLux" ) ) )


EQUATION( "L" )
/*
Labor aggregated demand
*/
RESULT( SUMS( PARENT, "L1" ) + SUMS( PARENT, "L2" ) )


EQUATION( "Ld" )
/*
Labor aggregated desired demand
*/
RESULT( SUMS( PARENT, "L1d" ) + SUMS( PARENT, "L2d" ) )


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


EQUATION( "W" )
/*
Total wages paid by all firms to workers
*/
RESULT( SUMS( PARENT, "W1" ) + SUMS( PARENT, "W2" ) )


EQUATION( "appl" )
/*
Number of worker applications for jobs
*/
RESULT( SUM( "_appl" ) * V( "Lscale" ) )


EQUATION( "buyListCleanup" )
/*
Periodically remove retired products/industries from workers past-buy lists
To be scheduled to run periodically, at least each 50 periods
Only for speeding-up simulation
*/

goodMapT allGoods, *bought;						// map to all current product IDs
goodMapT::iterator it, it1;

CYCLES( PARENT, cur, "Consumption" )
	if ( VS( cur, "type2" ) > 0 )				// just luxury goods
		allGoods.insert( goodPairT( ( int ) VS( cur, "ID2" ), 0 ) );

CYCLE( cur, "Worker")
{
	bought = & V_EXTS( cur, wrkE, buyLux );		// current worker past-buys
	if ( bought->empty( ) )
		continue;

	for ( it = bought->begin( ); it != bought->end( ); )
	{
		it1 = it;								// save next before deleting
		++it;

		if ( allGoods.find( it1->first ) == allGoods.end( ) )
			bought->erase( it1 );				// remove from list
	}
}

RESULT( T )


EQUATION( "dUeB" )
/*
Notional unemployment (bounded) rate of change
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "Ue", VS( PARENT, "mLim" ) ) )


EQUATION( "fires" )
/*
Number of workers fired (not quitting or retiring)
*/
RESULT( SUMS( PARENT, "fires1" ) + SUMS( PARENT, "fires2" ) )


EQUATION( "quits" )
/*
Number of workers quitting jobs (not fired)
*/
RESULT( SUMS( PARENT, "quits1" ) + SUMS( PARENT, "quits2" ) )


EQUATION( "retires" )
/*
Number of workers retiring (not fired)
*/
RESULT( SUMS( PARENT, "retires1" ) + SUMS( PARENT, "retires2" ) )


EQUATION( "sAvg" )
/*
Average workers compound skills in all sectors
Also updates:
	sSD: compound skills standard deviation in all sectors
	sMin: minimum compound skills in all sectors
	sMax: maximum compound skills in all sectors
*/

if ( VS( PARENT, "flagWorkerLBU" ) == 0 )		// no worker-level learning?
{
	WRITE( "sSD", 0 );
	WRITE( "sMax", 1 );
	WRITE( "sMin", 1 );
	END_EQUATION( 1 );
}

v[0] = v[1] = i = 0;							// accumulators
v[2] = 0;										// current maximums
v[3] = DBL_MAX;									// current minimums
CYCLE( cur, "Worker" )
{
	v[0] += v[4] = VS( cur, "_s" );				// sum of compound skills
	v[1] += pow( v[4], 2 );						// sum of square comp. skills

	if ( v[4] > v[2] )
		v[2] = v[4];							// new maximum

	if ( v[4] < v[3] )
		v[3] = v[4];							// new minimum

	++i;
}

WRITE( "sSD", sqrt( max( ( v[1] / i ) - pow( v[0] / i, 2 ), 0 ) ) );
WRITE( "sMax", v[2] );
WRITE( "sMin", v[3] );

RESULT( v[0] / i )


EQUATION( "sTavg" )
/*
Average workers tenure skills in all sectors
Also updates:
	sTsd: tenure skills standard deviation in all sectors
	sTmax: maximum workers tenure skills in all sectors
	sTmin: minimum workers tenure skills in all sectors
*/

if ( VS( PARENT, "flagWorkerLBU" ) < 2 )		// no learning-by-use?
{
	WRITE( "sTsd", 0 );
	WRITE( "sTmax", 1 );
	WRITE( "sTmin", 1 );
	END_EQUATION( 1 );
}

v[0] = v[1] = i = 0;							// accumulators
v[2] = 0;										// current maximums
v[3] = DBL_MAX;									// current minimums
CYCLE( cur, "Worker" )
{
	v[0] += v[4] = VS( cur, "_sT" );			// sum of tenure skills
	v[1] += pow( v[4], 2 );						// sum of square ten. skills

	if ( v[4] > v[2] )
		v[2] = v[4];							// new maximum

	if ( v[4] < v[3] )
		v[3] = v[4];							// new minimum

	++i;
}

WRITE( "sTsd", sqrt( max( ( v[1] / i ) - pow( v[0] / i, 2 ), 0 ) ) );
WRITE( "sTmax", v[2] );
WRITE( "sTmin", v[3] );

RESULT( v[0] / i )


EQUATION( "sVavg" )
/*
Average vintage skills of workers for all technologies/vintages
Also updates 'sVsd'
Also updates the map of vintage productivity and skill
*/

v[1] = V( "Lscale" );							// workers to objects ratio

vintMapT *vint = & EXTS( PARENT, countryE ).vintProd;// vintage map
vintMapT::iterator itm;

h = T;
CYCLES( PARENT, cur, "Consumption" )			// search time of oldest vintage
	if ( ( i = T0( VLS( cur, "old2vint", 1 ) ) ) < h )
		h = i;

// remove unused vintages from the map
for( itm = vint->begin( ); T0( itm->first ) < h && itm != vint->end( ); ++itm );

vint->erase( vint->begin( ), itm );

// update the rest of the map lagged (past period) vintage skills
for( ; itm != vint->end( ); ++itm )
{
	itm->second.sVavgLag = itm->second.sVavg;
	itm->second.sVavg = 0;
	itm->second.workers = 0;
}

// collect worker data
v[3] = v[4] = v[5] = 0;
CYCLE( cur, "Worker" )							// scan all workers
{
	cur1 = HOOKS( cur, VWRK );					// pointer to vintage bridge
	if ( cur1 != NULL )							// discard disalloc. unempl./s.1
	{
		v[3] += v[2] = VLS( cur, "_sV", 1 ) * v[1];// last skills
		v[4] += v[2] * v[2];
		v[5] += v[1];

		i = VS( PARENTS( cur1 ), "_IDvint" );	// vintage ID
		EXTS( PARENT, countryE ).vintProd[ i ].sVavg += v[2];
		EXTS( PARENT, countryE ).vintProd[ i ].workers += v[1];
	}
}

// update public skills map
for( itm = vint->begin( ); itm != vint->end( ); ++itm )
{
	itm->second.sVp += V( "sigma" ) * ( itm->second.sVavgLag - itm->second.sVp );

	if ( itm->second.workers != 0 )
		itm->second.sVavg /= itm->second.workers;
	else
		itm->second.sVavg = itm->second.sVp;
}

WRITE( "sVsd", v[5] > 0 ? sqrt( max( ( v[4] / v[5] ) -
								pow( v[3] / v[5], 2 ), 0 ) ) : CURRENT );

RESULT( v[5] > 0 ? v[3] / v[5] : CURRENT )


EQUATION( "wAvg" )
/*
Average wage received by workers (excluding bonus and unemployment benefits)
Also updates 'wLogSD'
*/

v[0] = v[1] = v[2] = i = 0;						// accumulators
CYCLE( cur, "Worker" )							// consider all workers
	if ( VS( cur, "_employed" ) )				// account only employed
	{
		v[3] = VS( cur, "_w" );
		v[0] += v[3];							// sum of wages
		v[1] += log( v[3] + 1 );				// sum of log wages
		v[2] += pow( log( v[3] + 1 ), 2 );		// sum of squared log wages

		++i;
	}

WRITE( "wLogSD", i > 0 ? sqrt( max( ( v[2] / i ) - pow( v[1] / i, 2 ), 0 ) ) : 0 );

RESULT( i > 0 ? v[0] / i : CURRENT )


EQUATION( "woAvg" )
/*
Average wage offered by firms
*/
v[1] = SUMS( PARENT, "L2" );
RESULT( v[1] > 0 ? WHTAVES( PARENT, "w2oAvg", "L2" ) / v[1] :
				   AVES( PARENT, "w2oAvg" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/


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

EQUATION_DUMMY( "Us", "Ue" )
/*
Short term unemployment rate (workers unemployed for less than 1 period)
Updated in 'Ue'
*/

EQUATION_DUMMY( "sMax", "sAvg" )
/*
Maximum workers compound skills in all sectors
Updated in 'sAvg'
*/

EQUATION_DUMMY( "sMin", "sAvg" )
/*
Minimum workers compound skills in all sectors
Updated in 'sAvg'
*/

EQUATION_DUMMY( "sSD", "sAvg" )
/*
Workers compound skills standard deviation in all sectors
Updated in 'sAvg'
*/

EQUATION_DUMMY( "sTmax", "sTavg" )
/*
Maximum workers tenure skills in all sectors
Updated in 'sTavg'
*/

EQUATION_DUMMY( "sTmin", "sTavg" )
/*
Minimum workers tenure skills in all sectors
Updated in 'sTavg'
*/

EQUATION_DUMMY( "sTsd", "sTavg" )
/*
Workers tenure skills standard deviation in all sectors
Updated in 'sTavg'
*/

EQUATION_DUMMY( "sVsd", "sVavg" )
/*
Workers vintage skills standard deviation in all sectors
Updated in 'sVavg'
*/

EQUATION_DUMMY( "wLogSD", "wAvg" )
/*
Employed workers wage standard deviation of wage log
Updated in 'wAvg'
*/
