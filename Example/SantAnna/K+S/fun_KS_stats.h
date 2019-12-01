/******************************************************************************

	STATISTICS EQUATIONS
	--------------------

	Equations that are not required for the model to run but may produce
	useful country- or sector-level statistics for analysis.
 
 ******************************************************************************/

/*========================= COUNTRY-LEVEL STATS ==============================*/

EQUATION( "DebGDP" )
/*
Government debt on GDP ratio
*/
v[1] = VS( GRANDPARENT, "GDP" );
RESULT( v[1] > 0 ? VS( GRANDPARENT, "Deb" ) / v[1] : CURRENT )


EQUATION( "DefGDP" )
/*
Government deficit on GDP ratio
*/
v[1] = VS( GRANDPARENT, "GDP" );
RESULT( v[1] > 0 ? VS( GRANDPARENT, "Def" ) / v[1] : CURRENT )


EQUATION( "GDI" )
/*
Gross domestic income (real terms)
*/
RESULT( ( VS( CAPSECL2, "W1" ) + VS( CONSECL2, "W2" ) +
		  VS( CAPSECL2, "Pi1" ) + VS( CONSECL2, "Pi2" ) + 
		  VS( FINSECL2, "PiB" ) + VS( GRANDPARENT, "G" ) - 
		  VS( GRANDPARENT, "Tax" ) + 
		  VS( CAPSECL2, "PPI" ) * VS( CONSECL2, "SI" ) / VS( CONSECL2, "m2" ) ) / 
		  VS( CONSECL2, "CPI" ) )


EQUATION( "dA" )
/*
Overall productivity (log) growth rate
*/
RESULT( log( VS( GRANDPARENT, "A" ) + 1 ) - log( VLS( GRANDPARENT, "A", 1 ) + 1 ) )


/*========================= FINANCIAL SECTOR STATS ===========================*/

EQUATION( "BadDeb" )
/*
Total banking sector bad debt (defaults) on the period
*/
RESULT( SUMS( FINSECL2, "_BadDeb" ) )


EQUATION( "BadDebAcc" )
/*
Bank accumulated losses from bad debt
*/
RESULT( CURRENT + VS( SECSTAL2, "BadDeb" ) )


EQUATION( "Bda" )
/*
Firms financial fragility defined as the ratio between accumulated bad debt (loans
in default) and total bank assets (stock of loans)
*/
RESULT( SUMS( FINSECL2, "_Bda" ) )


EQUATION( "Bfail" )
/*
Rate of failing banks
*/

VS( FINSECL2, "NWb" );							// make sure it is updated

// add-up failed banks
i = j = 0;										// bank/fail counters
CYCLES( FINSECL2, cur, "Bank" )
{
	if ( VS( cur, "_Gbail" ) > 0 )
		++j;
	
	++i;
}

RESULT( ( double ) j / i )


EQUATION( "HHb" )
/*
Normalized Herfindahl-Hirschman index for banking sector
*/

v[1] = i = 0;									// index accumulator & counter
CYCLES( FINSECL2, cur, "Bank" )
{
	v[1] += pow( VS( cur, "_fB" ), 2 );
	++i;
}

if ( i > 1 )
	v[0] = max( 0, ( v[1] - 1.0 / i ) / ( 1 - 1.0 / i ) );// normalize HHI
else
	v[0] = 1;
	
RESULT( v[0] )


EQUATION( "HPb" )
/*
Hymer-Pashigian index for banking sector
*/

v[0] = 0;										// index accumulator
CYCLES( FINSECL2, cur, "Bank" )
	v[0] += fabs( VLS( cur, "_fB", 1 ) - VS( cur, "_fB" ) );// sum share changes

RESULT( v[0] )	


EQUATION( "TC" )
/*
Total credit supply provided by banks to firms.
Negative value (-1) means unlimited credit.
*/

if ( VS( GRANDPARENT, "flagCreditRule" ) != 1 )
	END_EQUATION( -1 );

RESULT( SUMS( FINSECL2, "_TC" ) )


/*======================= CAPITAL-GOOD SECTOR STATS ==========================*/

EQUATION( "HH1" )
/*
Normalized Herfindahl-Hirschman index for capital-good sector
*/

v[1] = i = 0;									// index accumulator & counter
CYCLES( CAPSECL2, cur, "Firm1" )
{
	v[1] += pow( VS( cur, "_f1" ), 2 );			// add the squared market shares
	++i;
}

if ( i > 1 )
	v[0] = ( v[1] - 1.0 / i ) / ( 1 - 1.0 / i );// normalize HHI
else
	v[0] = 1;
	
RESULT( v[0] )


EQUATION( "HP1" )
/*
Hymer-Pashigian index for capital-good sector
*/

v[0] = 0;										// index accumulator
CYCLES( CAPSECL2, cur, "Firm1" )
	v[0] += fabs( VLS( cur, "_f1", 1 ) - VS( cur, "_f1" ) );// sum share changes

RESULT( v[0] )	


EQUATION( "L1ent" )
/*
Entry rate of labor (hires over labor employed) in capital-good sector
*/
v[1] = VS( CAPSECL2, "L1" );
RESULT( v[1] > 0 ? VS( CAPSECL2, "hires1" ) / v[1] : 0 )


EQUATION( "L1exit" )
/*
Exit rate of labor (fires+quits over labor employed) in capital-good sector
*/

v[1] = VS( CAPSECL2, "L1" );

if ( v[1] > 0 )
	v[0] = ( VS( CAPSECL2, "fires1" ) + VS( CAPSECL2, "quits1" ) + 
			 VS( CAPSECL2, "retires1" ) ) / v[1];
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "L1v" )
/*
Vacancy rate of labor (unfilled positions over labor employed) in 
capital-good sector
*/

v[1] = VS( CAPSECL2, "L1" );					// current number of workers
v[2] = VS( CAPSECL2, "JO1" );					// current open positions

if( v[1] == 0 )									// firm has no worker?
	v[0] = ( v[2] == 0 ) ? 0 : 1;				// handle limit case
else
	v[0] = min( v[2] / v[1], 1 );				// or calculate the regular way

RESULT( v[0] )


EQUATION( "RD" )
/*
R&D expenditure of capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_RD" ) )


EQUATION( "age1avg" )
/*
Average age of firms in capital-good sector
*/

v[0] = 0;										// firm age accumulator
CYCLES( CAPSECL2, cur, "Firm1" )
	v[0] += t - VS( cur, "_t1ent" ) + 1;
	
RESULT( v[0] / VS( CAPSECL2, "F1" ) )


EQUATION( "cred1c" )
/*
Total credit constraint of firms in capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_cred1c" ) )


EQUATION( "s1avg" )
/*
Average workers compound skills in capital-good sector
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) == 0 )	// no worker-level learning?
	END_EQUATION( 1 );							// skills = 1
	
v[0] = i = 0;									// accumulators
CYCLES( CAPSECL2, cur, "Wrk1" )
{
	v[0] += VS( SHOOKS( cur ), "_s" );			// worker current compound skills
	++i;				
}

RESULT( i > 0 ? v[0] / i : 0 )


/*======================= CONSUMER-GOOD SECTOR STATS =========================*/

EQUATION( "A2posChg" )
/*
Average productivity of post-change firms in consumption-good sector
*/

v[0] = v[1] = 0;								// productivity/output accum.
CYCLES( CONSECL2, cur, "Firm2" )
	if ( VS( cur, "_postChg" ) )				// pre-change?
	{
		v[2] = VS( cur, "_A2" );
		v[3] = VS( cur, "_Q2" );
		if ( is_finite( v[2] ) && is_finite( v[3] ) )
		{
			v[0] += v[2] * v[3];
			v[1] += v[3];
		}
	}

RESULT( v[1] > 0 ? v[0] / v[1] : 0 )			// sum (prod*output)/output


EQUATION( "A2preChg" )
/*
Average productivity of pre-change firms in consumption-good sector
*/

v[0] = v[1] = 0;								// productivity/output accum.
CYCLES( CONSECL2, cur, "Firm2" )
	if ( ! VS( cur, "_postChg" ) )				// pre-change?
	{
		v[2] = VS( cur, "_A2" );
		v[3] = VS( cur, "_Q2" );
		if ( is_finite( v[2] ) && is_finite( v[3] ) )
		{
			v[0] += v[2] * v[3];
			v[1] += v[3];
		}
	}

RESULT( v[1] > 0 ? v[0] / v[1] : 0 )			// sum (prod*output)/output


EQUATION( "A2sd" )
/*
Standard deviation of log productivity of firms in consumption-good sector
*/

v[1] = VS( CONSECL2, "A2" );					// average productivity
if ( v[1] <= 0 )
	END_EQUATION( 0 );							// probably no production yet

v[1] = log( v[1] + 1 );							// average log productivity

i = 0;											// valid cases count
v[0] = 0;										// square difference accumulator
CYCLES( CONSECL2, cur, "Firm2" )
{
	v[2] = VS( cur, "_A2" );
	if ( is_finite( v[2] ) && v[2] > 0 )
	{
		v[0] += pow( log( v[2] + 1 ) - v[1], 2 );
		++i;
	}
}

RESULT( i > 0 ? sqrt( v[0] / i ) : 0 )


EQUATION( "A2sdPosChg" )
/*
Standard deviation of log productivity of post-change firms in consumption-good 
sector
*/

v[1] = VS( SECSTAL2, "A2posChg" );				// average productivity
if ( v[1] <= 0 )
	END_EQUATION( 0 );							// probably no firm left

v[1] = log( v[1] + 1 );

i = 0;											// valid cases count
v[0] = 0;										// square difference accumulator
CYCLES( CONSECL2, cur, "Firm2" )
	if ( VS( cur, "_postChg" ) )				// post-change?
	{
		v[2] = VS( cur, "_A2" );
		if ( is_finite( v[2] ) && v[2] > 0 )
		{
			v[0] += pow( log( v[2] + 1 ) - v[1], 2 );
			++i;
		}
	}

RESULT( i > 0 ? sqrt( v[0] / i ) : 0 )


EQUATION( "A2sdPreChg" )
/*
Standard deviation of log productivity of pre-change firms in consumption-good 
sector
*/

v[1] = VS( SECSTAL2, "A2preChg" );				// average productivity
if ( v[1] <= 0 )
	END_EQUATION( 0 );							// probably no firm left

v[1] = log( v[1] + 1 );

i = 0;											// valid cases count
v[0] = 0;										// square difference accumulator
CYCLES( CONSECL2, cur, "Firm2" )
	if ( ! VS( cur, "_postChg" ) )				// pre-change?
	{
		v[2] = VS( cur, "_A2" );
		if ( is_finite( v[2] ) && v[2] > 0 )
		{
			v[0] += pow( log( v[2] + 1 ) - v[1], 2 );
			++i;
		}
	}

RESULT( i > 0 ? sqrt( v[0] / i ) : 0 )


EQUATION( "B2payers" )
/*
Number of bonus paying firms in consumption-good sector
*/

i = 0;											// firm counter
CYCLES( CONSECL2, cur, "Firm2" )
	if ( VS( cur, "_B2" ) > 0 )
		++i;

RESULT( i )


EQUATION( "HH2" )
/*
Normalized Herfindahl-Hirschman index for consumption-good sector
*/

v[1] = j = 0;									// index accumulator & counter
CYCLES( CONSECL2, cur, "Firm2" )
{
	v[1] += pow( VS( cur, "_f2" ), 2 );			// add the squared market shares
	++j;
}

if ( j > 1 )
	v[0] = ( v[1] - 1.0 / j ) / ( 1 - 1.0 / j );// normalize HHI
else
	v[0] = 1;
	
RESULT( v[0] )


EQUATION( "HP2" )
/*
Hymer-Pashigian index for consumption-good sector
*/

v[0] = 0;										// index accumulator
CYCLES( CONSECL2, cur, "Firm2" )
	v[0] += fabs( VLS( cur, "_f2", 1 ) - VS( cur, "_f2" ) );// sum share changes

RESULT( v[0] )	


EQUATION( "L2ent" )
/*
Entry rate of labor (hires over labor employed) in consumption-good sector
*/
v[1] = VS( CONSECL2, "L2" );
RESULT( v[1] > 0 ? VS( CONSECL2, "hires2" ) / v[1] : 0 )


EQUATION( "L2exit" )
/*
Exit rate of labor (fires+quits over labor employed) in consumption-good sector
*/

v[1] = VS( CONSECL2, "L2" );

if ( v[1] > 0 )
	v[0] = ( VS( CONSECL2, "fires2" ) + VS( CONSECL2, "quits2" ) + 
			 VS( CONSECL2, "retires2" ) ) / v[1];
else
	v[0] = 0;

RESULT( v[0] ) 


EQUATION( "L2larg" )
/*
Number of workers of largest firm in consumption-good sector
*/
RESULT( MAXS( CONSECL2, "_L2" ) )


EQUATION( "L2v" )
/*
Vacancy rate of labor (unfilled positions over labor employed) in 
consumption-good sector
*/

v[1] = VS( CONSECL2, "L2" );					// current number of workers
v[2] = VS( CONSECL2, "JO2" );					// current open positions

if( v[1] == 0 )									// firm has no worker?
	v[0] = ( v[2] == 0 ) ? 0 : 1;				// handle limit case
else
	v[0] = min( v[2] / v[1], 1 );				// or calculate the regular way

RESULT( v[0] )


EQUATION( "age2avg" )
/*
Average age of firms in consumption-good sector
*/

v[0] = 0;										// firm age accumulator
CYCLES( CONSECL2, cur, "Firm2" )
	v[0] += t - VS( cur, "_t2ent" ) + 1;
	
RESULT( v[0] / VS( CONSECL2, "F2" ) )


EQUATION( "cred2c" )
/*
Total credit constraint of firms in consumer-good sector
*/
RESULT( SUMS( CONSECL2, "_cred2c" ) )


EQUATION( "dN" )
/*
Change in total inventories (real terms)
*/
RESULT( VS( CONSECL2, "N" ) - VLS( CONSECL2, "N", 1 ) )


EQUATION( "mu2avg" )
/*
Weighted average mark-up of consumption-good sector
*/
RESULT( WHTAVES( CONSECL2, "_mu2", "_f2" ) )


EQUATION( "noWrk2" )
/*
Share of operating firms with no worker hired in consumption-good sector
*/

i = 0;											// no worker firm accumulator
CYCLES( CONSECL2, cur, "Firm2" )
	if ( VS( cur, "_L2" ) == 0 && VS( cur, "_life2cycle" ) > 0 )
		++i;

RESULT( i / VS( CONSECL2, "F2" ) )


EQUATION( "q2posChg" )
/*
Average product quality in consumer-good sector (weighted by output) 
of post-change firms
*/

v[0] = v[1] = 0;								// quality/production accum.
CYCLES( CONSECL2, cur, "Firm2" )				// scan all firms
	if ( VS( cur, "_postChg" ) )				// post-change?
	{
		v[1] += v[2] = VS( cur, "_Q2e" );
		v[0] += VS( cur, "_q2" ) * v[2];
	}

RESULT( v[1] > 0 ? v[0] / v[1] : 0 )			


EQUATION( "q2preChg" )
/*
Average product quality in consumer-good sector (weighted by output) 
of pre-change firms
*/

v[0] = v[1] = 0;								// quality/production accum.
CYCLES( CONSECL2, cur, "Firm2" )				// scan all firms
	if ( ! VS( cur, "_postChg" ) )				// pre-change?
	{
		v[1] += v[2] = VS( cur, "_Q2e" );
		v[0] += VS( cur, "_q2" ) * v[2];
	}

RESULT( v[1] > 0 ? v[0] / v[1] : 0 )			


EQUATION( "s2avg" )
/*
Average weighted firms' workers compound skills in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) == 0 )	// no worker-level learning?
	END_EQUATION( 1 );							// skills = 1
	
RESULT( WHTAVES( CONSECL2, "_s2avg", "_f2" ) )


EQUATION( "w2avgLarg" )
/*
Average wage paid by largest firm in consumption-good sector
*/

v[0] = v[1] = 0;								// max accumulator
CYCLES( CONSECL2, cur, "Firm2" )				// scan all firms
{
	v[2] = VS( cur, "_w2avg" );
	v[3] =  VS( cur, "_L2" );
	
	if ( v[3] > v[1] )							// largest firm yet?
	{
		v[1] = v[3];
		v[0] = v[2];							// average wage of largest
	}
}

RESULT( v[0] )


EQUATION( "w2oMin" )
/*
Lowest wage offered by a firm in consumption-good sector
Updated in 'w2oMax'
*/
RESULT( MINS( CONSECL2, "_w2o" ) )


EQUATION( "w2realPosChg" )
/*
Weighted average real wage paid by post-change firms in consumption-good sector
*/

i = v[0] = 0;									// worker counter/wage average
CYCLES( CONSECL2, cur, "Firm2" )				// scan all firms
	if ( VS( cur, "_postChg" ) )				// post-change?
	{
		i += j = VS( cur, "_L2" );
		v[0] +=  VS( cur, "_w2avg" ) * j;
	}

RESULT( i > 0 ? v[0] / i / VS( CONSECL2, "CPI" ) : 0 )			


EQUATION( "w2realPreChg" )
/*
Weighted average real wage paid by pre-change firms in consumption-good sector
*/

i = v[0] = 0;									// worker counter/wage average
CYCLES( CONSECL2, cur, "Firm2" )				// scan all firms
	if ( ! VS( cur, "_postChg" ) )				// pre-change?
	{
		i += j = VS( cur, "_L2" );
		v[0] +=  VS( cur, "_w2avg" ) * j;
	}

RESULT( i > 0 ? v[0] / i / VS( CONSECL2, "CPI" ) : 0 )			


/*============================= LABOR STATS ==================================*/

EQUATION( "Lent" )
/*
Entry rate of labor (hires over total labor)
*/
RESULT( ( VS( CAPSECL2, "hires1" ) + VS( CONSECL2, "hires2" ) ) / 
		VS( LABSUPL2, "Ls" ) )


EQUATION( "Lexit" )
/*
Exit rate of labor (fires+quits+retires over labor employed)
*/
v[1] = VS( CAPSECL2, "fires1" ) + VS( CAPSECL2, "quits1" ) + 
	   VS( CAPSECL2, "retires1" ) + VS( CONSECL2, "fires2" ) +
	   VS( CONSECL2, "quits2" ) + VS( CONSECL2, "retires2" );
RESULT( v[1] / VS( LABSUPL2, "Ls" ) )


EQUATION( "TuAvg" )
/*
Average number of periods of unemployment for unemployed workers
*/

i = j = 0;										// counters
CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
	if ( ! VS( cur, "_employed" ) )				// but account only unemployed
	{
		++i;									// add unemployed
		j += VS( cur, "_Tu" );					// add unemployment periods
	}

RESULT( i > 0 ? j / ( double ) i : 0 )


EQUATION( "V" )
/*
Effective vacancy rate (unfilled positions over total labor supply)
*/
RESULT( t > 1 ? min( ( VS( CAPSECL2, "JO1" ) + VS( CONSECL2, "JO2" ) ) / 
					   VS( LABSUPL2, "Ls" ), 1 ) : 0 )


EQUATION( "dw" )
/*
Nominal average wage growth rate
*/
v[1] = VLS( LABSUPL2, "wAvg", 1 );
RESULT( v[1] > 0 ? VS( LABSUPL2, "wAvg" ) / v[1] : 0 )


EQUATION( "part" )
/*
Participation rate of the work force (employed + looking for jobs)
*/

i = 0;
CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
	// but account only employed or searching workers
	if ( VS( cur, "_employed" ) || VS( cur, "_appl" ) > 0 )
		++i;

RESULT( i * VS( LABSUPL2, "Lscale" ) / VS( LABSUPL2, "Ls" ) )


EQUATION( "wAvgReal" )
/*
Average real wages received by workers (including unemployment benefits)
Also updates several worker level wage/income statistics at once:
	wMax: highest wage
	wMin: lowest wage
	wrAvg: average wage requested by workers
	wrMax: highest wage requested by workers
	wrMin: lowest wage requested by workers
	wsAvg: average satisficing wage of workers
	wsMax: highest satisficing wage of workers
	wsMin: lowest satisficing wage of workers
	wLogSD: wage dispersion measured by the standard deviation of wage log
	wrLogSD: log wage requested standard deviation
	wsLogSD: log satisficing wage standard deviation
	wGini: wage dispersion measured by the Gini index (Jasso-Deaton formula)
	Gini: Gini index including workers all income and firm owners net cash flows
Wages include unemployment benefits (if available) but not bonuses.
Income includes wages and bonuses.
*/

v[30] = VS( LABSUPL2, "Lscale" );				// workers per object

j = COUNTS( LABSUPL2, "Worker" );				// count scaled workers
h = COUNTS( CAPSECL2, "Firm1" );				// count firms (after entry-exit)
h += COUNTS( CONSECL2, "Firm2" );
h += j * v[30];									// number of worker+firm objects
	
double *rank1 = new double[ j ];				// allocate space for rank
double *rank2 = new double[ h ];				// allocate space for indiv. rank

i = k = 0;
v[1] = v[2] = v[3] = v[4] = 0;
v[11] = v[12] = v[13] = v[14] = 0;
v[21] = v[22] = v[23] = v[24] = 0;
v[5] = v[15] = v[25] = DBL_MAX;					// minimum registers
CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
{
	v[0] = VS( cur, "_w" ) + VS( cur, "_B" );	// current wage + bonus
	v[1] += v[0];								// sum of wages + bonus
	v[2] += log( v[0] + 1 );					// sum of log wages
	v[3] += pow( log( v[0] + 1 ), 2 );			// sum of squared log wages
	v[4] = max( v[0], v[4] );					// max wage
	v[5] = min( v[0], v[5] );					// min wage
	
	rank1[ i++ ] = v[0];						// insert wage in rank array
	
	for ( int worker = 0; worker < v[30]; ++worker )
		rank2[ k++ ] = v[0];					// same for income (wage+bonus)
	
	v[0] = VS( cur, "_wS" );					// current satisfacing wage
	v[11] += v[0];								// sum of satisfacing wages
	v[12] += log( v[0] + 1 );					// sum of log satisfacing wages
	v[13] += pow( log( v[0] + 1 ), 2 );			// sum of sq. log satisf. wages
	v[14] = max( v[0], v[14] );					// max satisfacing wage
	v[15] = min( v[0], v[15] );					// min satisfacing wage
	
	v[0] = VS( cur, "_wR" );					// current requested wage
	v[21] += v[0];								// sum of requested wages
	v[22] += log( v[0] + 1 );					// sum of log requested wages
	v[23] += pow( log( v[0] + 1 ), 2 );			// sum of sq. log req.  wages
	v[24] = max( v[0], v[24] );					// max requested wage
	v[25] = min( v[0], v[25] );					// min requested wage
}

CYCLES( CAPSECL2, cur, "Firm1" )				// consider sector 1 firm owners
	rank2[ k++ ] = max( VS( cur, "_NW1" ) - VLS( cur, "_NW1", 1 ), 0 );
												// Positive net wealth change

CYCLES( CONSECL2, cur, "Firm2" )				// consider sector 2 firm owners
	rank2[ k++ ] = max( VS( cur, "_NW2" ) - VLS( cur, "_NW2", 1 ), 0 );
												// Positive net wealth change
// averages
v[6] = v[1] / i;								// average of wages
v[16] = v[11] / i;								// average of satisfacing wages
v[26] = v[21] / i;								// average of requested wages

// SDs
v[7] = sqrt( max( ( v[3] / i ) - pow( v[2] / i, 2 ), 0 ) );// SD of log wages
v[17] = sqrt( max( ( v[13] / i ) - pow( v[12] / i, 2 ), 0 ) );// SD log sat. wage
v[27] = sqrt( max( ( v[23] / i ) - pow( v[22] / i, 2 ), 0 ) );// SD log req. wage

// sort ranks in descending order
sort( rank1, rank1 + j, greater< double >() );
sort( rank2, rank2 + h, greater< double >() );

for ( v[8] = v[9] = 0, k = 0; k < j; ++k )
{
	v[8] += ( k + 1 ) * rank1[ k ];				// sum rank x wage
	v[9] += rank1[ k ];							// sum wage
}

for ( v[18] = v[19] = 0, k = 0; k < h; ++k )
{
	v[18] += ( k + 1 ) * rank2[ k ];			// sum rank x wage
	v[19] += rank2[ k ];						// sum wage
}

delete [] rank1;
delete [] rank2;

// apply the Jasso-Deaton formula
v[10] = ( double ) ( j + 1 ) / ( j - 1 ) - 2 * v[8] / ( ( j - 1 ) * v[9] );
v[20] = ( double ) ( h + 1 ) / ( h - 1 ) - 2 * v[18] / ( ( h - 1 ) * v[19] );

// write results into dummy vars
WRITES( LABSTAL2, "wMax", v[4] );				// max wage
WRITES( LABSTAL2, "wMin", v[5] );				// min wage
WRITES( LABSTAL2, "wLogSD", v[7] );				// log wage SD
WRITES( LABSTAL2, "wGini", v[10] );				// Gini index
WRITES( LABSTAL2, "wsAvg", v[16] );				// average satisficing wage
WRITES( LABSTAL2, "wsMax", v[14] );				// max satisficing wage
WRITES( LABSTAL2, "wsMin", v[15] );				// min satisficing wage
WRITES( LABSTAL2, "wsLogSD", v[17] );			// log satisficing wage SD
WRITES( LABSTAL2, "wrAvg", v[26] );				// average requested wage
WRITES( LABSTAL2, "wrMax", v[24] );				// max requested wage
WRITES( LABSTAL2, "wrMin", v[25] );				// min requested wage
WRITES( LABSTAL2, "wrLogSD", v[27] );			// log requested wage SD
WRITES( MACSTAL2, "Gini", v[20] );				// overall Gini index

RESULT( v[6] / VS( CONSECL2, "CPI" ) )


EQUATION( "wAvgRealEmp" )
/*
Average real wage received by employed workers (excluding unemployment benefits)
Also updates other worker level wage statistics at once:
	wMaxEmp: highest wage of employed workers
	wMinEmp: lowest wage of employed workers
*/

i = 0;
v[1] = v[2] = 0;
v[3] = DBL_MAX;									// minimum register
CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
	if ( VS( cur, "_employed" ) )				// account only employed
	{
		v[0] = VS( cur, "_w" ) + VS( cur, "_B" );// current wage + bonus
		v[1] += v[0];							// sum of wages
		v[2] = max( v[0], v[2] );				// max wage
		v[3] = min( v[0], v[3] );				// min wage
		
		++i;
	}

v[4] = ( i > 0 ) ? v[1] / i : CURRENT;			// average of wages

// write results into parameters
WRITE( "wMaxEmp", v[2] );
WRITE( "wMinEmp", v[3] );

RESULT( v[4] / VS( CONSECL2, "CPI" ) )


/*============================ AGENT-LEVEL STATS =============================*/

EQUATION( "_A2e" )
/*
Effective productivity of firm in consumption-good sector
*/
i = V( "_L2" );
RESULT( i > 0 ? V( "_Q2e" ) / i : CURRENT )


EQUATION( "_sT2avg" )
/*
Weighted average workers tenure skills of a firm in consumption-good sector
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 1 )							// no learning by tenure mode?
	END_EQUATION( 1 );							// skills = 1

v[0] = v[1] = 0;
CYCLE( cur, "Wrk2" )
{
	cur1 = SHOOKS( cur );
	v[1] += v[2] = VS( cur1, "_Q" );			// worker current production
	v[0] += VS( cur1, "_sT" ) * v[2] ;			// add worker potential
}
	
RESULT( v[1] > 0 ? v[0] / v[1] : 0 )			// handle the no production case


EQUATION( "_sV2avg" )
/*
Weighted average workers vintage skills of a firm in consumption-good sector
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 2 )							// no learning by vintage mode?
	END_EQUATION( 1 );							// skills = 1
	
v[0] = v[1] = 0;
CYCLE( cur, "Wrk2" )
{
	cur1 = SHOOKS( cur );
	v[1] += v[2] = VS( cur1, "_Q" );			// worker current production
	v[0] += VS( cur1, "_sV" ) * v[2] ;			// add worker potential
}

if ( v[0] == 0 || v[1] == 0 )					// no worker hired or no prod.?
{
	j = VS( HOOK( TOPVINT ), "_IDvint" );		// current top vintage
	v[0] = V_EXTS( GRANDPARENT, country, vintProd[ j ].sVp );
}												// assume public skills
else
	v[0] /= v[1];								// weighthed average

RESULT( v[0] )


EQUATION( "_wReal" )
/*
Real wage received by worker
*/
RESULT( V( "_w" ) / VS( CONSECL2, "CPI" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "Gini", "" )
/*
Gini index including workers all income and firm owners net cash flows
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wAvgReal", "wAvg" )
/*
Real average wage (CPI adjusted)
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wGini", "wAvg" )
/*
Wage dispersion measured by the Gini index (Jasso-Deaton formula)
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wLogSD", "wAvg" )
/*
Wage dispersion measured by the standard deviation of wage log
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wMax", "wAvg" )
/*
Highest wage
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wMaxEmp", "wAvgRealEmp" )
/*
Highest wage of employed workers
Updated in 'wAvgRealEmp'
*/

EQUATION_DUMMY( "wMin", "wAvg" )
/*
Lowest wage
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wMinEmp", "wAvgRealEmp" )
/*
Lowest wage of employed workers
Updated in 'wAvgRealEmp'
*/

EQUATION_DUMMY( "wrAvg", "wAvg" )
/*
Average wage requested by workers
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wrLogSD", "wAvg" )
/*
Log wage requested standard deviation
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wrMax", "wAvg" )
/*
Highest wage requested by workers
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wrMin", "wAvg" )
/*
Lowest wage requested by workers
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wsAvg", "wAvg" )
/*
Average satisficing wage of workers
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wsLogSD", "wAvg" )
/*
Log satisficing wage standard deviation
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wsMax", "wAvg" )
/*
Highest satisficing wage of workers
Updated in 'wAvg'
*/

EQUATION_DUMMY( "wsMin", "wAvg" )
/*
Lowest satisficing wage of workers
Updated in 'wAvg'
*/
