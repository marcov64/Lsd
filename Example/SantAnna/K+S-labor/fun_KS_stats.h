/******************************************************************************

	STATISTICS EQUATIONS
	--------------------

	Equations that are not required for the model to run but may produce
	useful country- or sector-level statistics for analysis.
 
 ******************************************************************************/

/*========================= COUNTRY-LEVEL STATS ==============================*/

EQUATION( "CD" )
/*
Total credit demand
*/
RESULT( VS( SECSTAL2, "CD1" ) + VS( SECSTAL2, "CD2" ) )


EQUATION( "CDc" )
/*
Total credit demand constraint
*/
RESULT( VS( SECSTAL2, "CD1c" ) + VS( SECSTAL2, "CD2c" ) )


EQUATION( "CS" )
/*
Total credit supplied
*/
RESULT( VS( SECSTAL2, "CS1" ) + VS( SECSTAL2, "CS2" ) )


EQUATION( "Creal" )
/*
Real aggregated consumption
*/
RESULT( VS( GRANDPARENT, "C" ) / VS( CONSECL2, "CPI" ) )


EQUATION( "DefGDP" )
/*
Government deficit on GDP ratio
*/
v[1] = VS( GRANDPARENT, "GDPnom" );
RESULT( v[1] > 0 ? VS( GRANDPARENT, "Def" ) / v[1] : CURRENT )


EQUATION( "GDI" )
/*
Gross domestic income (nominal terms)
*/
RESULT( VS( CAPSECL2, "W1" ) + VS( CONSECL2, "W2" ) +
		VS( CAPSECL2, "Pi1" ) + VS( CONSECL2, "Pi2" ) + 
		VS( FINSECL2, "PiB" ) + VS( GRANDPARENT, "G" ) - 
		VS( GRANDPARENT, "Tax" ) + 
		VS( CAPSECL2, "PPI" ) * VS( CONSECL2, "SI" ) / VS( CONSECL2, "m2" ) )


EQUATION( "dA" )
/*
Overall labor productivity growth rate
*/
v[1] = VLS( GRANDPARENT, "A", 1 );
RESULT( v[1] > 0 ? VS( GRANDPARENT, "A" ) / v[1] - 1 : 0 )


/*========================= FINANCIAL SECTOR STATS ===========================*/

EQUATION( "BadDeb" )
/*
Total bad debt (defaults) in financial sector
*/
RESULT( SUMS( FINSECL2, "_BadDeb" ) )


EQUATION( "BadDebAcc" )
/*
Accumulated losses from bad debt in financial sector
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
RESULT( COUNT_CNDS( FINSECL2, "Bank", "_Gbail", ">", 0 ) / 
		COUNTS( FINSECL2, "Bank" ) )


EQUATION( "ExRes" )
/*
Excess reserves (free cash) hold by financial sector
*/
RESULT( SUMS( FINSECL2, "_ExRes" ) )


EQUATION( "HHb" )
/*
Normalized Herfindahl-Hirschman index for financial sector
*/
i = COUNTS( FINSECL2, "Bank" );
RESULT( i > 1 ? max( 0, ( WHTAVES( FINSECL2, "_fB", "_fB" ) - 1.0 / i ) / 
						( 1 - 1.0 / i ) ) : 1 )


EQUATION( "HPb" )
/*
Hymer-Pashigian index for financial sector
*/

v[0] = 0;										// index accumulator
CYCLES( FINSECL2, cur, "Bank" )
	v[0] += fabs( VLS( cur, "_fB", 1 ) - VS( cur, "_fB" ) );// sum share changes

RESULT( v[0] )	


EQUATION( "TC" )
/*
Total credit supply provided by financial sector
Negative value (-1) means unlimited credit
*/

if ( VS( GRANDPARENT, "flagCreditRule" ) < 1 )
	END_EQUATION( -1 );

RESULT( SUMS( FINSECL2, "_TC" ) )


/*======================= CAPITAL-GOOD SECTOR STATS ==========================*/

EQUATION( "AtauAvg" )
/*
Average labor productivity of machines supplied by capital-good sector
*/
RESULT( AVES( CAPSECL2, "_Atau" ) )


EQUATION( "BtauAvg" )
/*
Average labor productivity of machines produced by capital-good sector
*/
RESULT( AVES( CAPSECL2, "_Btau" ) )


EQUATION( "CD1" )
/*
Total credit demand of firms in capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_CD1" ) )


EQUATION( "CD1c" )
/*
Total credit demand constraint of firms in capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_CD1c" ) )


EQUATION( "CS1" )
/*
Total credit supplied to firms in capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_CS1" ) )


EQUATION( "Deb1max" )
/*
Total maximum prudential credit supplied to firms in capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_Deb1max" ) )


EQUATION( "HCavg" )
/*
Number of historical clients of capital-good firms
*/
RESULT( AVES( CAPSECL2, "_HC" ) )


EQUATION( "HH1" )
/*
Normalized Herfindahl-Hirschman index for capital-good sector
*/
i = COUNTS( CAPSECL2, "Firm1" );
RESULT( i > 1 ? max( 0, ( WHTAVES( CAPSECL2, "_f1", "_f1" ) - 1.0 / i ) / 
						( 1 - 1.0 / i ) ) : 1 )


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


EQUATION( "NCavg" )
/*
Number of new clients of capital-good firms
*/
RESULT( AVES( CAPSECL2, "_NC" ) )


EQUATION( "RD" )
/*
R&D expenditure of capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_RD" ) )


EQUATION( "age1avg" )
/*
Average age of firms in capital-good sector
*/
RESULT( T - AVES( CAPSECL2, "_t1ent" ) )


EQUATION( "s1avg" )
/*
Average workers compound skills in capital-good sector
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) == 0 )	// no worker-level learning?
	END_EQUATION( INISKILL );
	
v[0] = i = 0;									// accumulators
CYCLES( CAPSECL2, cur, "Wrk1" )
{
	v[0] += VS( SHOOKS( cur ), "_s" );			// worker current compound skills
	++i;				
}

RESULT( i > 0 ? v[0] / i : CURRENT )


/*======================= CONSUMER-GOOD SECTOR STATS =========================*/

EQUATION( "A2posChg" )
/*
Machine-level weighted-average labor productivity of post-change firms in 
consumption-good sector
*/
v[1] = SUM_CNDS( CONSECL2, "_Q2", "_postChg", "!=", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( CONSECL2, "_A2", "_Q2", "_postChg", "!=", 0 ) / 
				   v[1] : 0 )


EQUATION( "A2preChg" )
/*
Machine-level weighted-average labor productivity of pre-change firms in 
consumption-good sector
*/
v[1] = SUM_CNDS( CONSECL2, "_Q2", "_postChg", "==", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( CONSECL2, "_A2", "_Q2", "_postChg", "==", 0 ) / 
				   v[1] : 0 )


EQUATION( "A2sd" )
/*
Standard deviation of machine-level log labor productivity of firms in 
consumption-good sector
Also updates 'A2sdPreChg', 'A2sdPosChg'
*/

v[1] = VS( CONSECL2, "A2" );					// average productivities
v[2] = VS( SECSTAL2, "A2preChg" );
v[3] = VS( SECSTAL2, "A2posChg" );

v[1] = ( v[1] >= 0 ) ? log( v[1] + 1 ) : 0;		// average log productivities
v[2] = ( v[2] >= 0 ) ? log( v[2] + 1 ) : 0;
v[3] = ( v[3] >= 0 ) ? log( v[3] + 1 ) : 0;

v[0] = v[4] = v[5] = i = j = k = 0;
CYCLES( CONSECL2, cur, "Firm2" )
{
	v[6] = VS( cur, "_A2" );
	if ( v[6] < 0 )
		continue;

	v[0] += pow( log( v[6] + 1 ) - v[1], 2 );
	++i;
	
	if ( ! VS( cur, "_postChg" ) )				// pre-change?
	{
		v[4] += pow( log( v[6] + 1 ) - v[2], 2 );
		++j;
	}
	else
	{
		v[5] += pow( log( v[6] + 1 ) - v[3], 2 );
		++k;
	}
}

WRITE( "A2sdPreChg", j > 0 ? sqrt( v[4] / j ) : 0 );
WRITE( "A2sdPosChg", k > 0 ? sqrt( v[5] / k ) : 0 );

RESULT( i > 0 ? sqrt( v[0] / i ) : 0 )


EQUATION( "B2payers" )
/*
Number of bonus paying firms in consumption-good sector
*/
RESULT( COUNT_CNDS( CONSECL2, "Firm2", "_B2", ">", 0 ) )


EQUATION( "CD2" )
/*
Total credit demand of firms in consumer-good sector
*/
RESULT( SUMS( CONSECL2, "_CD2" ) )


EQUATION( "CD2c" )
/*
Total credit demand constraint of firms in consumer-good sector
*/
RESULT( SUMS( CONSECL2, "_CD2c" ) )


EQUATION( "CS2" )
/*
Total credit supplied to firms in consumer-good sector
*/
RESULT( SUMS( CONSECL2, "_CS2" ) )


EQUATION( "Deb2max" )
/*
Total maximum prudential credit supplied to firms in consumer-good sector
*/
RESULT( SUMS( CONSECL2, "_Deb2max" ) )


EQUATION( "EId" )
/*
Total desired expansion investment in consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_EId" ) )


EQUATION( "HH2" )
/*
Normalized Herfindahl-Hirschman index for consumption-good sector
*/
i = COUNTS( CONSECL2, "Firm2" );
RESULT( i > 1 ? max( 0, ( WHTAVES( CONSECL2, "_f2", "_f2" ) - 1.0 / i ) / 
						( 1 - 1.0 / i ) ) : 1 )


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


EQUATION( "RS2" )
/*
Machine (planned) scrapping rate in consumption-good sector
*/
v[1] = VLS( CONSECL2, "K", 1 );
RESULT( T > 1 && v[1] > 0 ? SUMS( CONSECL2, "_RS2" ) / 
		( v[1] / VS( CONSECL2, "m2" ) ) : 0 )


EQUATION( "SId" )
/*
Total desired substitution investment in consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_SId" ) )


EQUATION( "age2avg" )
/*
Average age of firms in consumption-good sector
*/
RESULT( T - AVES( CONSECL2, "_t2ent" ) )


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


EQUATION( "nBrochAvg" )
/*
Average number of machine brochures available to firms in consumer-good sector
*/

v[0] = 0;
CYCLES( CONSECL2, cur, "Firm2" )
	v[0] += COUNTS( cur, "Broch");

RESULT( v[0] / VS( CONSECL2, "F2" ) )


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
v[1] = SUM_CNDS( CONSECL2, "_Q2e", "_postChg", "!=", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( CONSECL2, "_q2", "_Q2e", "_postChg", "!=", 0 ) / v[1] : 0 )


EQUATION( "q2preChg" )
/*
Average product quality in consumer-good sector (weighted by output) 
of pre-change firms
*/
v[1] = SUM_CNDS( CONSECL2, "_Q2e", "_postChg", "==", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( CONSECL2, "_q2", "_Q2e", "_postChg", "==", 0 ) / v[1] : 0 )


EQUATION( "s2avg" )
/*
Average weighted firms' workers compound skills in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) == 0 )	// no worker-level learning?
	END_EQUATION( INISKILL );
	
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
v[1] = SUM_CNDS( CONSECL2, "_L2", "_postChg", "!=", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( CONSECL2, "_w2avg", "_L2", "_postChg", "!=", 0  ) / 
				   v[1] / VS( CONSECL2, "CPI" ) : 0 )


EQUATION( "w2realPreChg" )
/*
Weighted average real wage paid by pre-change firms in consumption-good sector
*/
v[1] = SUM_CNDS( CONSECL2, "_L2", "_postChg", "==", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( CONSECL2, "_w2avg", "_L2", "_postChg", "==", 0  ) / 
				   v[1] / VS( CONSECL2, "CPI" ) : 0 )


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
v[0] = AVE_CNDS( LABSUPL2, "_Tu", "_employed", "==", 0 );
RESULT( ! isnan( v[0] ) ? v[0] : 0 )


EQUATION( "V" )
/*
Effective vacancy rate (unfilled positions over total labor supply)
*/
RESULT( T > 1 ? min( ( VS( CAPSECL2, "JO1" ) + VS( CONSECL2, "JO2" ) ) / 
					   VS( LABSUPL2, "Ls" ), 1 ) : 0 )


EQUATION( "dw" )
/*
Nominal average wage growth rate
*/
v[1] = VLS( LABSUPL2, "wAvg", 1 );
v[2] = VS( LABSUPL2, "wAvg" );
RESULT( v[1] > 0 && v[2] > 0 ? log( v[2] ) - log( v[1] ) : 0 )


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
Machine-level effective productivity of firm in consumption-good sector
*/
i = V( "_L2" );
RESULT( i > 0 ? V( "_Q2e" ) / i : CURRENT )


EQUATION( "_RS2" )
/*
Number of machines to scrap of firm in consumption-good sector
*/

v[0] = 0;
CYCLE( cur, "Vint" )
	v[0] += abs( VS( cur, "_RSvint" ) );
	
RESULT( v[0] )


EQUATION( "_sT2avg" )
/*
Weighted average workers tenure skills of a firm in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) <= 1 )	// no learning by tenure mode?
	END_EQUATION( INISKILL );

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
	END_EQUATION( INISKILL );
	
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
	v[0] = V_EXTS( GRANDPARENT, countryE, vintProd[ j ].sVp );
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

EQUATION_DUMMY( "A2sdPosChg", "A2sd" )
/*
Standard deviation of machine-level log labor productivity of post-change firms 
in consumption-good sector
Updated in 'A2sd'
*/

EQUATION_DUMMY( "A2sdPreChg", "A2sd" )
/*
Standard deviation of machine-level log labor productivity of pre-change firms 
in consumption-good sector
Updated in 'A2sd'
*/

EQUATION_DUMMY( "Gini", "" )
/*
Gini index including workers all income and firm owners net cash flows
Updated in 'wAvg'
*/

EQUATION_DUMMY( "exit1fail", "" )
/*
Rate of exiting bankrupt firms in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "exit2fail", "" )
/*
Rate of exiting bankrupt firms in consumption-good sector
Updated in 'entry2exit'
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
