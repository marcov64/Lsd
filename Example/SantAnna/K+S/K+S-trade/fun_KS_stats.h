/******************************************************************************

	STATISTICS EQUATIONS
	--------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

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
Real (in initial prices terms) aggregated consumption
*/
RESULT( SUMS( CONSECL2, "_D2l" ) * VS( CONSECL2, "pC0" ) )


EQUATION( "DefY" )
/*
Government deficit on GDP ratio
*/
RESULT( VS( COUNTRL2, "Def" ) / VS( COUNTRL2, "Y" ) )


EQUATION( "XnetY" )
/*
Net exports on GDP ratio
*/
RESULT( ( VS( COUNTRL2, "X" ) - VS( COUNTRL2, "M" ) ) / VS( COUNTRL2, "Y" ) )


EQUATION( "Yinc" )
/*
Gross domestic income (nominal terms)
*/
RESULT( VS( LABSUPL2, "In" ) - VS( LABSUPL2, "Gtrf" ) + VS( CAPSECL2, "Pi1" ) +
		VS( CAPSECL2, "i1" ) + VS( CONSECL2, "Pi2" ) + VS( CONSECL2, "i2" ) +
		VS( FINSECL2, "PiB" ) + VS( COUNTRL2, "TaxM" ) + VS( COUNTRL2, "TaxX" ) +
		VS( CAPSECL2, "PPI" ) * VS( CONSECL2, "SI" ) / VS( CONSECL2, "m2" ) )


EQUATION( "Ydefl" )
/*
GDP price deflator
*/
RESULT( VS( COUNTRL2, "Y" ) / VS( COUNTRL2, "Yreal" ) )


EQUATION( "dA" )
/*
Overall labor productivity growth rate
*/
v[1] = VLS( COUNTRL2, "A", 1 );
RESULT( v[1] > 0 ? VS( COUNTRL2, "A" ) / v[1] - 1 : 0 )


EQUATION( "fMK" )
/*
Market share of capital-good imports
*/
v[1] = VS( CONSECL2, "Inom" );
RESULT( v[1] > 0 ? VS( COUNTRL2, "Mk" ) / v[1] : 0 )


EQUATION( "fXK" )
/*
Market share of worldwide capital-good exports
*/
v[1] = SUMS( WORLDL2, "Xk" );
RESULT( v[1] > 0 ? VS( COUNTRL2, "Xk" ) / v[1] : 0 )


/*========================= FINANCIAL SECTOR STATS ===========================*/

EQUATION( "BadDebAcc" )
/*
Accumulated losses from bad debt in financial sector
*/
RESULT( CURRENT + VS( FINSECL2, "BadDeb" ) )


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

if ( VS( COUNTRL2, "flagCreditRule" ) < 1 )
	END_EQUATION( -1 );

RESULT( SUMS( FINSECL2, "_TC" ) )


/*======================= CAPITAL-GOOD SECTOR STATS ==========================*/

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


EQUATION( "D1l" )
/*
Local demand (in product units) of capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_D1l" ) )


EQUATION( "D1x" )
/*
Export demand (in product units) of capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_D1x" ) )


EQUATION( "Deb1max" )
/*
Total maximum prudential credit supplied to firms in capital-good sector
*/
RESULT( SUMS( CAPSECL2, "_Deb1max" ) )


EQUATION( "HCavg" )
/*
Average number of historical clients of capital-good firms
*/
RESULT( AVES( CAPSECL2, "_HC" ) )


EQUATION( "HCwAvg" )
/*
Average number of international historical clients of capital-good firms
*/
RESULT( AVES( CAPSECL2, "_HCw" ) )


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


EQUATION( "f1g" )
/*
Share of government-owned firms in capital-good sector
*/
RESULT( COUNT_CNDS( CAPSECL2, "Firm1", "_own1", "==", 1 ) /
		VS( CAPSECL2, "F1" ) )


EQUATION( "s1avg" )
/*
Average workers compound skills in capital-good sector
*/

if ( VS( COUNTRL2, "flagWorkerLBU" ) == 0 )	// no worker-level learning?
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
RESULT( COUNT_CNDS( CONSECL2, "Firm2", "_Bon2", ">", 0 ) )


EQUATION( "CD2" )
/*
Total credit demand of firms in consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_CD2" ) )


EQUATION( "CD2c" )
/*
Total credit demand constraint of firms in consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_CD2c" ) )


EQUATION( "CS2" )
/*
Total credit supplied to firms in consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_CS2" ) )


EQUATION( "D2l" )
/*
Local demand (in product units) of consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_D2l" ) )


EQUATION( "D2x" )
/*
Export demand (in product units) of consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_D2x" ) )


EQUATION( "Deb2max" )
/*
Total maximum prudential credit supplied to firms in consumption-good sector
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


EQUATION( "L21" )
/*
Work force of category 1 in consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_L21" ) )


EQUATION( "L22" )
/*
Work force category 2 in consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_L22" ) )


EQUATION( "L23" )
/*
Work force category 3 in consumption-good sector
*/
RESULT( SUMS( CONSECL2, "_L23" ) )


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


EQUATION( "ed2avg" )
/*
Average education level in consumption-good sector
*/
v[0] = AVE_CNDS( LABSUPL2, "_ed", "_employed", "==", 2 );
RESULT( is_nan( v[0] ) ? 0 : v[0] )


EQUATION( "f2g" )
/*
Share of government-owned firms in capital-good sector
*/
RESULT( COUNT_CNDS( CONSECL2, "Firm2", "_own2", "==", 1 ) /
		VS( CONSECL2, "F2" ) )


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

if ( VS( COUNTRL2, "flagWorkerLBU" ) == 0 )	// no worker-level learning?
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
	v[3] =	VS( cur, "_L2" );

	if ( v[3] > v[1] )							// largest firm yet?
	{
		v[1] = v[3];
		v[0] = v[2];							// average wage of largest
	}
}

RESULT( v[0] )


EQUATION( "w2o1min" )
/*
Lowest wage offered to category 1 workers in consumption-good sector
*/
RESULT( MINS( CONSECL2, "_w2o1" ) )


EQUATION( "w2o2min" )
/*
Lowest wage offered to category 2 workers in consumption-good sector
*/
RESULT( MINS( CONSECL2, "_w2o2" ) )


EQUATION( "w2o3min" )
/*
Lowest wage offered to category 3 workers in consumption-good sector
*/
RESULT( MINS( CONSECL2, "_w2o3" ) )


EQUATION( "w2realPosChg" )
/*
Weighted average real wage paid by post-change firms in consumption-good sector
*/
v[1] = SUM_CNDS( CONSECL2, "_L2", "_postChg", "!=", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( CONSECL2, "_w2avg", "_L2", "_postChg", "!=", 0 ) /
				   v[1] / VS( MACSTAL2, "Ydefl" ) : 0 )


EQUATION( "w2realPreChg" )
/*
Weighted average real wage paid by pre-change firms in consumption-good sector
*/
v[1] = SUM_CNDS( CONSECL2, "_L2", "_postChg", "==", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( CONSECL2, "_w2avg", "_L2", "_postChg", "==", 0 ) /
				   v[1] / VS( MACSTAL2, "Ydefl" ) : 0 )


/*============================= LABOR STATS ==================================*/

EQUATION( "BonPayers" )
/*
Share of bonus paying firms in consumption-good sector
*/

i = j = 0;										// firm counter
CYCLES( CONSECL2, cur1, "Firm2" )
{
	if ( VS( cur1, "_Bon2" ) > 0 )
		++i;
	++j;
}

RESULT( ( double ) i / j )


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


EQUATION( "Lpart" )
/*
Participation rate of the work force (employed + looking for jobs)
*/

i = 0;
CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
	// but account only employed or searching workers
	if ( VS( cur, "_employed" ) || VS( cur, "_appl" ) > 0 )
		++i;

RESULT( i * VS( LABSUPL2, "Lscale" ) / VS( LABSUPL2, "Ls" ) )


EQUATION( "Ls1" )
/*
Category 1 labor force size
*/
RESULT( COUNT_CNDS( LABSUPL2, "Worker", "_cat", "==", 1 ) *
		VS( LABSUPL2, "Lscale" ) )


EQUATION( "Ls2" )
/*
Category 2 labor force size
*/
RESULT( COUNT_CNDS( LABSUPL2, "Worker", "_cat", "==", 2 ) *
		VS( LABSUPL2, "Lscale" ) )


EQUATION( "Ls3" )
/*
Category 3 labor force size
*/
RESULT( COUNT_CNDS( LABSUPL2, "Worker", "_cat", "==", 3 ) *
		VS( LABSUPL2, "Lscale" ) )


EQUATION( "SavAccY" )
/*
Accumulated savings of workers over GDP
*/
RESULT( VS( LABSUPL2, "SavAcc" ) / VS( COUNTRL2, "Y" ) )


EQUATION( "TuAvg" )
/*
Average number of periods of unemployment for unemployed workers
*/
v[0] = AVE_CNDS( LABSUPL2, "_Tu", "_employed", "==", 0 );
RESULT( ! is_nan( v[0] ) ? v[0] : 0 )


EQUATION( "V" )
/*
Effective vacancy rate (unfilled positions over total labor supply)
*/
RESULT( T > 1 ? min( ( VS( CAPSECL2, "JO1" ) + VS( CONSECL2, "JO2" ) ) /
					 VS( LABSUPL2, "Ls" ), 1 ) : 0 )


EQUATION( "V1" )
/*
Category 1 worker vacancy rate
*/
RESULT( T > 1 ? min( ( VS( CAPSECL2, "JO11" ) + VS( CONSECL2, "JO21" ) ) /
					 V( "Ls1" ), 1 ) : 0 )


EQUATION( "V2" )
/*
Category 2 worker vacancy rate
*/
RESULT( T > 1 ? min( ( VS( CAPSECL2, "JO12" ) + VS( CONSECL2, "JO22" ) ) /
					 V( "Ls2" ), 1 ) : 0 )


EQUATION( "V3" )
/*
Category 3 worker vacancy rate
*/
RESULT( T > 1 ? min( ( VS( CAPSECL2, "JO13" ) + VS( CONSECL2, "JO23" ) ) /
					 V( "Ls3" ), 1 ) : 0 )


EQUATION( "dw" )
/*
Nominal average wage growth rate
*/
v[1] = VLS( LABSUPL2, "wAvg", 1 );
v[2] = VS( LABSUPL2, "wAvg" );
RESULT( v[1] > 0 && v[2] > 0 ? log( v[2] ) - log( v[1] ) : 0 )


EQUATION( "edAvg" )
/*
Average education level
*/
RESULT( AVES( LABSUPL2, "_ed" ) )


EQUATION( "wAvgReal" )
/*
Average real wage received by workers (excluding unemployment benefits)
Also updates several worker wage/income statistics at once for efficiency:
	Gini: Gini index including all workers income and firm owners net cash flows
	InAvg: average income of workers (including bonus & unemployment benefit)
	InAvgReal: average real income of workers (including bonus & unemp. benefit)
	InLogSD: income dispersion measured by the standard deviation of income log
	InMax: highest income
	InMin: lowest income
	wGini: wage dispersion measured by the Gini index (Jasso-Deaton formula)
	wLogSD: wage dispersion measured by the standard deviation of wage log
	wMax: highest wage
	wMin: lowest wage
	wAvg1: average wage of category 1 workers
	wAvg1real: average real wage of category 1 workers
	wLog1sd: category 1 wage dispersion as standard deviation of wage log
	wMax1: highest category 1 wage
	wMin1: lowest category 1 wage
	wAvg2: average wage of category 2 workers
	wAvg2real: average real wage of category 2 workers
	wLog2sd: category 2 wage dispersion as standard deviation of wage log
	wMax2: highest category 2 wage
	wMin2: lowest category 2 wage
	wAvg3: average wage of category 3 workers
	wAvg3real: average real wage of category 3 workers
	wLog3sd: category 3 wage dispersion as standard deviation of wage log
	wMax3: highest category 3 wage
	wMin3: lowest category 3 wage
	wrAvg: average wage requested by workers
	wrLogSD: log wage requested standard deviation
	wrMax: highest wage requested by workers
	wrMin: lowest wage requested by workers
	wsAvg: average satisficing wage of workers
	wsLogSD: log satisficing wage standard deviation
	wsMax: highest satisficing wage of workers
	wsMin: lowest satisficing wage of workers
Wages do not include unemployment benefits (if available) or bonuses.
Income includes wages and bonuses.
*/

double _In, _w, _wR, _wS,
	   Gini, InSum, InLogSD, InLogSum, InSqLogSum, InMax, InMin,
	   rank1wSum, wRank1Sum, rank2InSum, InRank2Sum, wAvgReal, wLogSD,
	   wAvg1, wAvg2, wAvg3, wAvg1real, wAvg2real, wAvg3real, wGini,
	   wLog1sd, wLog2sd, wLog3sd,
	   wSum, wLogSum, wSqLogSum, wMax, wMin,
	   wsSum, wsLogSum, wsSqLogSum, wsMax, wsMin, wsLogSD,
	   wrSum, wrLogSum, wrSqLogSum, wrMax, wrMin, wrLogSD,
	   w1sum, wLog1sum, wSqLog1sum, wMax1, wMin1,
	   w2sum, wLog2sum, wSqLog2sum, wMax2, wMin2,
	   w3sum, wLog3sum, wSqLog3sum, wMax3, wMin3;

int agtN, empN, emp1n, emp2n, emp3n, wrkN;
double Ydefl = VS( MACSTAL2, "Ydefl" );			// GDP deflator
double Lscale = VS( LABSUPL2, "Lscale" );		// workers per object

wrkN = COUNTS( LABSUPL2, "Worker" );			// count scaled workers
agtN = wrkN * Lscale + COUNTS( CAPSECL2, "Firm1" ) + COUNTS( CONSECL2, "Firm2" );
												// number of worker+firm agents
double *rank1 = new double[ wrkN ];				// allocate space for worker rank
double *rank2 = new double[ agtN ];				// allocate space for indiv. rank

agtN = empN = emp1n = emp2n = emp3n = wrkN = 0;
InSum = InLogSum = InSqLogSum = wSum = wLogSum = wSqLogSum = 0;
w1sum = wLog1sum = wSqLog1sum = w2sum = wLog2sum = wSqLog2sum = 0;
w3sum = wLog3sum = wSqLog3sum = 0;
wsSum = wsLogSum = wsSqLogSum = wrSum = wrLogSum = wrSqLogSum = 0;
InMax = wMax = wMax1 = wMax2 = wMax3 = wrMax = wsMax = 0;
InMin = wMin = wMin1 = wMin2 = wMin3 = wsMin = wrMin = DBL_MAX;

CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
{
	_w = VS( cur, "_w" );						// current wage
	_In = VS( cur, "_In" );						// total worker income
	InSum += _In;								// sum of income
	InLogSum += log( _In + 1 );					// sum of log income
	InSqLogSum += pow( log( _In + 1 ), 2 );		// sum of sq. log income
	InMax = max( _In, InMax );					// max income
	InMin = min( _In, InMin );					// min income

	for ( int worker = 0; worker < Lscale; ++worker )
		rank2[ agtN++ ] = _In;					// insert income in rank array

	if ( VS( cur, "_employed" ) )
	{
		wSum += _w;								// sum of wages
		wLogSum += log( _w + 1 );				// sum of log wages
		wSqLogSum += pow( log( _w + 1 ), 2 );	// sum of squared log wages
		wMax = max( _w, wMax );					// max wage
		wMin = min( _w, wMin );					// min wage
		rank1[ empN++ ] = _w;					// insert wage in rank array

		switch( ( int ) VS( cur, "_cat" ) )
		{
			case 1:								// primary education workers
			default:
				w1sum += _w;
				wLog1sum += log( _w + 1 );
				wSqLog1sum += pow( log( _w + 1 ), 2 );
				wMax1 = max( _w, wMax1 );
				wMin1 = min( _w, wMin1 );
				++emp1n;
				break;

			case 2:								// secondary education workers
				w2sum += _w;
				wLog2sum += log( _w + 1 );
				wSqLog2sum += pow( log( _w + 1 ), 2 );
				wMax2 = max( _w, wMax2 );
				wMin2 = min( _w, wMin2 );
				++emp2n;
				break;

			case 3:								// tertiary education workers
				w3sum += _w;
				wLog3sum += log( _w + 1 );
				wSqLog3sum += pow( log( _w + 1 ), 2 );
				wMax3 = max( _w, wMax3 );
				wMin3 = min( _w, wMin3 );
				++emp3n;
		}
	}

	_wR = VS( cur, "_wR" );						// current requested wage
	wrSum += _wR;								// sum of requested wages
	wrLogSum += log( _wR + 1 );					// sum of log requested wages
	wrSqLogSum += pow( log( _wR + 1 ), 2 );		// sum of sq. log req. wages
	wrMax = max( _wR, wrMax );					// max requested wage
	wrMin = min( _wR, wrMin );					// min requested wage

	_wS = VS( cur, "_wS" );						// current satisfacing wage
	wsSum += _wS;								// sum of satisfacing wages
	wsLogSum += log( _wS + 1 );					// sum of log satisfacing wages
	wsSqLogSum += pow( log( _wS + 1 ), 2 );		// sum of sq. log satisf. wages
	wsMax = max( _wS, wsMax );					// max satisfacing wage
	wsMin = min( _wS, wsMin );					// min satisfacing wage

	++wrkN;
}

CYCLES( CAPSECL2, cur, "Firm1" )				// consider sector 1 firm owners
	rank2[ agtN++ ] = max( VS( cur, "_NW1" ) - VLS( cur, "_NW1", 1 ), 0 );
												// Positive net wealth change

CYCLES( CONSECL2, cur, "Firm2" )				// consider sector 2 firm owners
	rank2[ agtN++ ] = max( VS( cur, "_NW2" ) - VLS( cur, "_NW2", 1 ), 0 );
												// Positive net wealth change

// apply the Jasso-Deaton formula
sort( rank1, rank1 + empN, std::greater < double > ( ) );// sort in descending order
sort( rank2, rank2 + agtN, std::greater < double > ( ) );

for ( rank1wSum = wRank1Sum = k = 0; k < empN; ++k )
{
	rank1wSum += ( k + 1 ) * rank1[ k ];
	wRank1Sum += rank1[ k ];
}

for ( rank2InSum = InRank2Sum = k = 0; k < agtN; ++k )
{
	rank2InSum += ( k + 1 ) * rank2[ k ];
	InRank2Sum += rank2[ k ];
}

if ( empN > 1 && wRank1Sum > 0 )
	wGini = ROUND( ( double ) ( empN + 1 ) / ( empN - 1 ) -
				   2 * rank1wSum / ( ( empN - 1 ) * wRank1Sum ), 0, 0.01 );
else
	wGini = 0;

if ( agtN > 1 && InRank2Sum > 0 )
	Gini = ROUND( ( double ) ( agtN + 1 ) / ( agtN - 1 ) -
				  2 * rank2InSum / ( ( agtN - 1 ) * InRank2Sum ), 0, 0.01 );
else
	Gini = 0;

if ( wrkN > 0 )
{
	InLogSD = sqrt( max( ( InSqLogSum / wrkN ) -
						 pow( InLogSum / wrkN, 2 ), 0 ) );
	wrLogSD = sqrt( max( ( wrSqLogSum / wrkN ) -
						 pow( wrLogSum / wrkN, 2 ), 0 ) );
	wsLogSD = sqrt( max( ( wsSqLogSum / wrkN ) -
						 pow( wsLogSum / wrkN, 2 ), 0 ) );
}
else
	InLogSD = wrLogSD = wsLogSD = 0;

if ( empN > 0 )
{
	wAvgReal = wSum / empN / Ydefl;
	wLogSD = sqrt( max( ( wSqLogSum / empN ) -
						pow( wLogSum / empN, 2 ), 0 ) );
}
else
	wAvgReal = wLogSD = wMin = 0;

if ( emp1n > 0 )
{
	wAvg1 = w1sum / emp1n;
	wAvg1real = wAvg1 / Ydefl;
	wLog1sd = sqrt( max( ( wSqLog1sum / emp1n ) -
						 pow( wLog1sum / emp1n, 2 ), 0 ) );
}
else
	wAvg1 = wAvg1real = wLog1sd = wMin1 = 0;

if ( emp2n > 0 )
{
	wAvg2 = w2sum / emp2n;
	wAvg2real = wAvg2 / Ydefl;
	wLog2sd = sqrt( max( ( wSqLog2sum / emp2n ) -
						 pow( wLog2sum / emp2n, 2 ), 0 ) );
}
else
	wAvg2 = wAvg2real = wLog2sd = wMin2 = 0;

if ( emp3n > 0 )
{
	wAvg3 = w3sum / emp3n;
	wAvg3real = wAvg3 / Ydefl;
	wLog3sd = sqrt( max( ( wSqLog3sum / emp3n ) -
						 pow( wLog3sum / emp3n, 2 ), 0 ) );
}
else
	wAvg3 = wAvg3real = wLog3sd = wMin3 = 0;

delete [] rank1;
delete [] rank2;

WRITES( MACSTAL2, "Gini", Gini );				// overall Gini index
WRITES( LABSTAL2, "InAvg", InSum / wrkN );		// average income
WRITES( LABSTAL2, "InAvgReal", InSum / wrkN / Ydefl);// average income
WRITES( LABSTAL2, "InLogSD", InLogSD );			// log income SD
WRITES( LABSTAL2, "InMax", InMax );				// max income
WRITES( LABSTAL2, "InMin", InMin );				// min income
WRITES( LABSTAL2, "wGini", wGini );				// Gini index
WRITES( LABSTAL2, "wLogSD", wLogSD );			// log wage SD
WRITES( LABSTAL2, "wMax", wMax );				// max wage
WRITES( LABSTAL2, "wMin", wMin );				// min wage
WRITES( LABSTAL2, "wAvg1", wAvg1 );				// average category 1 wage
WRITES( LABSTAL2, "wAvg1real", wAvg1real );		// average category 1 real wage
WRITES( LABSTAL2, "wLog1sd", wLog1sd );			// log category 1 wage SD
WRITES( LABSTAL2, "wMax1", wMax1 );				// max category 1 wage
WRITES( LABSTAL2, "wMin1", wMin1 );				// min category 1 wage
WRITES( LABSTAL2, "wAvg2", wAvg2 );				// average category 2 wage
WRITES( LABSTAL2, "wAvg2real", wAvg2real );		// average category 2 real wage
WRITES( LABSTAL2, "wLog2sd", wLog2sd );			// log category 2 wage SD
WRITES( LABSTAL2, "wMax2", wMax2 );				// max category 2 wage
WRITES( LABSTAL2, "wMin2", wMin2 );				// min category 2 wage
WRITES( LABSTAL2, "wAvg3", wAvg3 );				// average category 3 wage
WRITES( LABSTAL2, "wAvg3real", wAvg3real );		// average category 3 real wage
WRITES( LABSTAL2, "wLog3sd", wLog3sd );			// log category 1 wage SD
WRITES( LABSTAL2, "wMax3", wMax3 );				// max category 1 wage
WRITES( LABSTAL2, "wMin3", wMin3 );				// min category 1 wage
WRITES( LABSTAL2, "wrAvg", wrSum / wrkN );		// average requested wage
WRITES( LABSTAL2, "wrLogSD", wrLogSD );			// log requested wage SD
WRITES( LABSTAL2, "wrMax", wrMax );				// max requested wage
WRITES( LABSTAL2, "wrMin", wrMin );				// min requested wage
WRITES( LABSTAL2, "wsAvg", wsSum / wrkN );		// average category wage
WRITES( LABSTAL2, "wsLogSD", wsLogSD );			// log satisficing wage SD
WRITES( LABSTAL2, "wsMax", wsMax );				// max satisficing wage
WRITES( LABSTAL2, "wsMin", wsMin );				// min satisficing wage

RESULT( wAvgReal )


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
	v[0] += abs( VS( cur, "__RSvint" ) );

RESULT( v[0] )


EQUATION( "_sT2avg" )
/*
Weighted average workers tenure skills of a firm in consumption-good sector
*/

if ( VS( COUNTRL2, "flagWorkerLBU" ) <= 1 )	// no learning by tenure mode?
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

i = VS( COUNTRL2, "flagWorkerLBU" );			// worker-level learning mode
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
	j = VS( HOOK( TOPVINT ), "__IDvint" );		// current top vintage
	v[0] = V_EXTS( COUNTRL2, countryE, vintProd[ j ].sVp );
}												// assume public skills
else
	v[0] /= v[1];								// weighthed average

RESULT( v[0] )


EQUATION( "_wReal" )
/*
Real wage received by worker
*/
RESULT( V( "_w" ) / VS( MACSTAL2, "Ydefl" ) )


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

EQUATION_DUMMY( "Gini", "wAvgReal" )
/*
Gini index including workers all income and firm owners net cash flows
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "InAvg", "wAvgReal" )
/*
Average income of workers (including bonus & unemployment benefit)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "InAvgReal", "wAvgReal" )
/*
Average real income of workers (including bonus & unemployment benefit)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "InLogSD", "wAvgReal" )
/*
Income dispersion measured by the standard deviation of income log
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "InMax", "wAvgReal" )
/*
Highest income
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "InMin", "wAvgReal" )
/*
Lowest income
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "U1", "Ue" )
/*
Effective category 1 unemployment rate including discouraged workers
*/

EQUATION_DUMMY( "U2", "Ue" )
/*
Effective category 2 unemployment rate including discouraged workers
*/

EQUATION_DUMMY( "U3", "Ue" )
/*
Effective category 3 unemployment rate including discouraged workers
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

EQUATION_DUMMY( "wGini", "wAvgReal" )
/*
Wage dispersion measured by the Gini index (Jasso-Deaton formula)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wLogSD", "wAvgReal" )
/*
Wage dispersion measured by the standard deviation of wage log
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMax", "wAvgReal" )
/*
Highest wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMin", "wAvgReal" )
/*
Lowest wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wAvg1", "wAvgReal" )
/*
Average wage of category 1 workers	(excluding bonus & unemployment benefits)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wAvg1real", "wAvgReal" )
/*
Average real wage of category 1 workers	 (excluding bonus & unemployment benefits)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wLog1sd", "wAvgReal" )
/*
Category 1 wage dispersion measured by the standard deviation of wage log
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMax1", "wAvgReal" )
/*
Highest category 1 wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMin1", "wAvgReal" )
/*
Lowest category 1 wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wAvg2", "wAvgReal" )
/*
Average wage of category 2 workers	(excluding bonus & unemployment benefits)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wAvg2real", "wAvgReal" )
/*
Average real wage of category 2 workers	 (excluding bonus & unemployment benefits)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wLog2sd", "wAvgReal" )
/*
Category 2 wage dispersion measured by the standard deviation of wage log
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMax2", "wAvgReal" )
/*
Highest category 2 wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMin2", "wAvgReal" )
/*
Lowest category 2 wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wAvg3", "wAvgReal" )
/*
Average wage of category 3 workers	(excluding bonus & unemployment benefits)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wAvg3real", "wAvgReal" )
/*
Average real wage of category 3 workers	 (excluding bonus & unemployment benefits)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wLog3sd", "wAvgReal" )
/*
Category 3 wage dispersion measured by the standard deviation of wage log
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMax3", "wAvgReal" )
/*
Highest category 3 wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMin3", "wAvgReal" )
/*
Lowest category 3 wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wrAvg", "wAvgReal" )
/*
Average wage requested by workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wrLogSD", "wAvgReal" )
/*
Log wage requested standard deviation
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wrMax", "wAvgReal" )
/*
Highest wage requested by workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wrMin", "wAvgReal" )
/*
Lowest wage requested by workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wsAvg", "wAvgReal" )
/*
Average satisficing wage of workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wsLogSD", "wAvgReal" )
/*
Log satisficing wage standard deviation
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wsMax", "wAvgReal" )
/*
Highest satisficing wage of workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wsMin", "wAvgReal" )
/*
Lowest satisficing wage of workers
Updated in 'wAvgReal'
*/
