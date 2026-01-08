/******************************************************************************

	CAPITAL-GOODS MARKET OBJECT EQUATIONS
	-------------------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the capital-goods market objects in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "JO1" )
/*
Open job positions in capital-good sector
Also updates 'JO11', 'JO12', 'JO13'
*/

v[1] = VS( LABSUPL1, "Ls" );					// available labor force
v[2] = V( "L1dRD" );							// R&D labor demand in sector 1
v[3] = V( "L1d" );								// production labor in sector 1
v[4] = VS( CONSECL1, "L2d" );					// production labor in sector 2
v[5] = COUNT( "Wrk1" ) * VS( LABSUPL1, "Lscale" );// current workers

v[2] = min( v[2], v[1] );						// ignore demand over total labor
v[3] = min( v[3], v[1] );

v[3] -= v[2];									// labor used in production

// split possible labor shortage proportionally up to a limit (to avoid crashes)
if ( ( v[3] + v[4] ) > ( v[1] - v[2] ) )
	v[3] *= max( ( v[1] - v[2] ) / ( v[3] + v[4] ), 1 - V( "L1shortMax" ) );

v[0] = max( ceil( v[2] + v[3] - v[5] ), 0 );	// hires scaled and rounded up

if ( VS( COUNTRL1, "flagEduc" ) == 0 )			// no education?
{
	v[7] = v[0];								// category 1 jobs only
	v[8] = v[9] = 0;
}
else
{
	v[6] = min( v[2] - VL( "L1rd", 1 ), v[0] );	// R&D jobs open (approx.)
	v[8] = max( ceil( VS( LABSUPL1, "theta2" ) * ( v[0] - v[6] ) ), 0 );
	v[9] = max( ceil( v[6] + VS( LABSUPL1, "theta3" ) * ( v[0] - v[6] ) ), 0 );
	v[7] = max( v[0] - v[8] - v[9], 0 );		// cat. 1 jobs open is residual
	v[0] = min( v[7] + v[8] + v[9], v[3] );		// prevent rounding errors
}

WRITE( "JO11", v[7] );
WRITE( "JO12", v[8] );
WRITE( "JO13", v[9] );

RESULT( v[0] )


EQUATION( "MC1" )
/*
Market entry conditions index in capital-good sector
*/
RESULT( log( max( VL( "NW1", 1 ), 0 ) + 1 ) - log( VL( "Deb1", 1 ) + 1 ) )


EQUATION( "entry1exit" )
/*
Net (number of) entrant firms in capital-good sector
Perform entry and exit of firms in the capital-good sector
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
Also updates 'F1', 'Eq1entryG', 'Eq1exitG', 'NW1exitG', 'Eq1entryW', 'Eq1exitW',
'NW1exitW', 'exit1', 'entry1', 'exit1fail'
*/

CYCLES( WORLDL1, cur, "Country" )				// in all countries
	VS( V_EXTS( cur, countryE, conSec ), "K" );	// ensure canceled orders acct'd

UPDATE;											// ensure aggregates are computed

double MC1 = V( "MC1" );						// market conditions in sector 1
double MC1_1 = VL( "MC1", 1 );					// market conditions in sector 1
double n1 = V( "n1" );							// market participation period
double omicron = VS( COUNTRL1, "omicron" );		// entry sensitivity to mkt cond
double stick = VS( COUNTRL1, "stick" );			// stickiness in number of firms
double x2inf = VS( COUNTRL1, "x2inf" );			// entry lower distrib. support
double x2sup = VS( COUNTRL1, "x2sup" );			// entry upper distrib. support
int F1 = V( "F1" );								// current number of firms
int F10 = V( "F10" );							// initial number of firms
int F1max = V( "F1max" );						// max firms in sector 1
int F1min = V( "F1min" );						// min firms in sector 1
int flagFirmOwner = VS( COUNTRL1, "flagFirmOwner" );// government firms?
int l, n;

double statThrs = V( "gamma1g" ) * V( "AtauAvg" );// statization threshold
intVecT quit( F1, 0 );							// firms' quit options

WRITE( "Eq1entryW", 0 );						// private shareholders accum.
WRITE( "Eq1exitW", 0 );
WRITE( "NW1exitW", 0 );
WRITE( "Eq1entryG", 0 );						// government accumulators
WRITE( "Eq1exitG", 0 );
WRITE( "NW1exitG", 0 );

// mark bankrupt and market-share-irrelevant incumbent firms to quit the market
h = F1;											// initial number of firms
v[1] = v[2] = i = k = l = 0;					// accum., counters, registers
CYCLE( cur, "Firm1" )
{
	v[3] = VS( cur, "_NW1" );					// current net wealth

	if ( v[3] < 0 || T >= VS( cur, "_t1ent" ) + n1 )// bankrupt or incumbent?
	{
		for ( v[4] = j = 0; j < n1; ++j )
			v[4] += VLS( cur, "_BC", j );		// n1 periods customer number

		if ( v[3] < 0 || v[4] <= 0 )
		{
			if ( v[3] < 0 && flagFirmOwner % 2 == 1 && VS( cur, "_own1" ) == 0 &&
				 VS( cur, "_Atau" ) > statThrs )// is statization possible?
			{
				quit[ i ] = 2;					// mark for possible statization

				v[5] = VS( cur, "_Atau" );		// firm machine productivity
				if ( v[5] > v[2] )				// best candidate so far?
				{
					l = i;						// save firm index
					v[2] = v[5];				// and machine productivity
				}
			}
			else
			{
				quit[ i ] = 1;					// mark for likely exit
				--h;							// one less firm
			}

			if ( v[4] > v[1] )					// larger firm so far?
			{
				k = i;							// save firm index
				v[1] = v[4];					// and customer number
			}
		}
	}

	++i;
}

// quit candidate firms exit, except the best one if all going to quit
// and good bankrupt firms that are rescued by the government (statization)
i = j = n = 0;									// firm counters
CYCLE_SAFE( cur, "Firm1" )
{
	if ( quit[ i ] == 2 )						// firm statization possible?
	{
		if ( i == l )							// best firm for statization?
			CFUNS( cur, rescue_firm, true );	// state rescue, transfer firm
		else
		{
			quit[ i ] = 1;						// mark for likely exit
			--h;								// one less firm
		}
	}

	if ( quit[ i ] == 1 )
	{
		if ( h > 0 || i != k )					// firm must exit?
		{
			++j;								// count exits
			if ( VS( cur, "_NW1" ) < 0 )		// count bankruptcies
				++n;

			CFUNS( cur, exit_firm, NULL );		// delete object properly
		}
		else
			if ( h == 0 && i == k )				// best firm must be rescued?
				CFUNS( cur, rescue_firm, false );// shareholder rescue
	}

	++i;
}

V( "f1rescale" );								// redistribute exiting m.s.

// compute the potential number of entrants
v[6] = ( MC1_1 == 0 ) ? 0 : MC1 / MC1_1 - 1;	// change in market conditions

k = max( 0, round( F1 * ( ( 1 - omicron ) * uniform( x2inf, x2sup ) +
						  omicron * min( max( v[6], x2inf ), x2sup ) ) ) );

// apply return-to-the-average stickiness random shock to the number of entrants
k -= min( RND * stick * ( ( double ) ( F1 - j ) / F10 - 1 ) * F10, k );

// ensure limits are enforced to the number of entrants
if ( F1 - j + k < F1min )
	k = F1min - F1 + j;

if ( F1 + k > F1max )
	k = F1max - F1 + j;

CFUN( entry_firm1, k, false );					// add entrant-firm objects

v[0] = k - j;									// net number of entrants
i = INCR( "F1", v[0] );							// update the number of firms
WRITE( "exit1", ( double ) j / F1 );
WRITE( "entry1", ( double ) k / F1 );
WRITES( SECSTAL1, "exit1fail", ( double ) n / F1 );
RECALCS( FINSECL1, "BadDeb1" );					// update bad debt after exits
RECALCS( WORLDL1, "FkW" );						// world firms after entry/exits
UPDATES( WORLDL1 );								// force saving recalculated

V( "f1rescale" );								// redistribute entrant m.s.
INIT_TSEARCH( "Firm1" );						// prepare turbo search indexing

RESULT( v[0] )


EQUATION( "fires1" )
/*
Number of workers fired in capital-good sector
Process required firing
*/

VS( COUNTRL1, "regChg" );						// ensure reg. regime has changed

V( "retires1" );								// process retirements
VS( CONSECL1, "retires2" );
V( "quits1" );									// process quits
VS( CONSECL1, "quits2" );

v[1] = VS( LABSUPL1, "Lscale" );				// labor scaling
v[2] = VS( LABSUPL1, "Ls" );					// available labor force
v[3] = V( "L1dRD" );							// R&D labor demand in sector 1
v[4] = V( "L1d" ) - v[3];						// production labor in sector 1
v[5] = VS( CONSECL1, "L2d" );					// production labor in sector 2
v[6] = COUNT( "Wrk1" );							// current workers (scaled)

// split possible labor shortage proportionally up to a limit (to avoid crashes)
if ( ( v[4] + v[5] ) > ( v[2] - v[3] ) )
	v[4] *= max( ( v[2] - v[3] ) / ( v[4] + v[5] ), 1 - V( "L1shortMax" ) );

// fires in sector 1, scaled-down and rounded-up
j = v[6] - ceil( ( v[4] + v[3] ) / v[1] );

if ( j <= 0 )									// nobody to fire?
	END_EQUATION( 0 );

if ( j >= v[6] )
	j = v[6] - 1;								// keep at least 1 scaled worker

// order firing list
CFUN( order_workers, ( int ) VS( COUNTRL1, "flagFireOrder1" ), OBJ_WRK1 );

// then check firing worker by worker in sector 1 pool
i = 0;
CYCLE_SAFE( cur, "Wrk1" )
	if ( i < j )
	{
		cur1 = SHOOKS( cur );					// pointer to Worker object
		if ( VLS( cur1, "_Te", 1 ) + 1 < VS( cur1, "_Tc" ) )// contract not over?
			continue;							// go to next worker

		CFUNS( cur1, fire_worker );				// register fire
		++i;									// scaled equivalent fires
	}
	else
		break;

RESULT( i * v[1] )


EQUATION( "hires1" )
/*
Number of workers hired by firms in capital-good sector
Start the labor market hiring all workers needed in capital-good sector
*/

V( "fires1" );									// make sure firing is done
VS( CONSECL1, "fires2" );						// in both sectors
VS( LABSUPL1, "emig" );							// emigration is done
VS( LABSUPL1, "appl" );							// and applications are done

// split open positions between R&D and production (scaled figures)
v[4] = VS( LABSUPL1, "Lscale" );				// labor scaling
v[1] = ceil( V( "JO11" ) / v[4] );				// scaled cat. open 1 jobs
v[21] = VS( CONSECL1, "w2o1max" );				// cat. 1 top wage offered

if ( VS( COUNTRL1, "flagEduc" ) == 0 )
{
	k = 1;										// just category 1 to hire
	v[2] = v[3] = 0;							// no category 2 & 3 workers
}
else
{
	k = 3;										// 3 categories to hire
	v[2] = ceil( V( "JO12" ) / v[4] );			// scaled cat. 2 open jobs
	v[3] = ceil( V( "JO13" ) / v[4] );			// scaled cat. 3 open jobs
	v[22] = VS( CONSECL1, "w2o2max" );			// cat. 2/3 top wages offered
	v[23] = VS( CONSECL1, "w2o3max" );
}

// sort sector 1 candidate list according to the defined mode
appLisT *appl = & V_EXTS( COUNTRL1, countryE, firm1appl );
CFUN( order_applications, ( int ) VS( COUNTRL1, "flagHireOrder1" ), appl );

for ( h = 0, j = k; j >= 1; --j )				// all categories, reverse order
{
	// hire the ordered applications until queue is exhausted for category
	i = 0;										// hired workers in category
	auto ita = appl->begin( );					// first application

	while ( v[ j ] - i > 0 && ita != appl->end( ) )
		if ( VS( ita->wrk, "_cat" ) != j )
			++ita;								// ignore other worker categories
		else
		{
			// offered wage ok (ignore very small differences)?
			if ( ROUND( ita->w, v[ 20 + j ], 0.01 ) <= v[ 20 + j ] )
			{
				// flag hiring and set wage & employer of worker
				CFUNS( ita->wrk, hire_worker, 1, THIS, v[ 20 + j ] );
				++i;							// scaled count hire (category)
			}

			ita = appl->erase( ita );			// remove worker from list
		}

	// adjust lower categories demand to account for unfilled positions
	if ( v[ j ] - i > 0 && j > 1 )
		v[ j - 1 ] += v[ j ] - i;

	h += i;										// total hired in sector
}

appl->clear( );									// clear job application queue

RESULT( h * v[4] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A1" )
/*
Labor productivity of capital-good sector
*/
V( "PPI" );										// ensure m.s. are updated
RESULT( WHTAVE( "_Btau", "_f1" ) )


EQUATION( "AtauAvg" )
/*
Average labor productivity of machines supplied by capital-good sector
*/
RESULT( AVE( "_Atau" ) )


EQUATION( "D1" )
/*
Total demand orders (in product units) received by capital-good sector
*/
RESULT( SUM( "_D1l" ) + SUM( "_D1x" ) )


EQUATION( "Deb1" )
/*
Total debt of capital-good sector
*/
RESULT( SUM( "_Deb1" ) )


EQUATION( "Div1" )
/*
Total dividends paid by firms in capital-good sector
*/
RESULT( V( "Div1g" ) + V( "Div1w" ) )


EQUATION( "Div1g" )
/*
Total dividends paid by government-owned firms in capital-good sector
*/
V( "Tax1" );									// ensure dividends are computed
RESULT( SUM_CND( "_Div1", "_own1", "==", 1 ) )


EQUATION( "Div1w" )
/*
Total dividends paid by private firms in capital-good sector
*/
V( "Tax1" );									// ensure dividends are computed
RESULT( SUM_CND( "_Div1", "_own1", "==", 0 ) )


EQUATION( "Eq1" )
/*
Equity hold by workers/households from firms in capital-good sector
*/
RESULT( SUM( "_Eq1" ) )


EQUATION( "F1" )
/*
Number of firms in capital-good sector
*/
RESULT( COUNT( "Firm1" ) )


EQUATION( "L1" )
/*
Work force (labor) size employed by capital-good sector
Result is scaled according to the defined scale
*/
VS( CONSECL1, "hires2" );						// ensure hires are done
RESULT( COUNT( "Wrk1" ) * VS( LABSUPL1, "Lscale" ) )


EQUATION( "L1d" )
/*
Total labor demand from firms in capital-good sector
Includes R&D labor
*/
RESULT( SUM( "_L1d" ) )


EQUATION( "L1dRD" )
/*
R&D labor demand from firms in capital-good sector
*/
RESULT( SUM( "_L1dRD" ) )


EQUATION( "L1rd" )
/*
Total R&D labor employed by firms in capital-good sector
Also updates '_L1', '_L1rd'
*/

v[1] = V( "L1" );								// total workers in sector 1
v[2] = V( "L1d" );								// total labor demand in sector 1
v[3] = V( "L1dRD" );							// R&D labor demand in sector 1

// R&D labor in sector 1
v[0] = min( v[3], min( v[1], round( VS( LABSUPL1, "Ls" ) * V( "L1rdMax" ) ) ) );

v[4] = v[5] = 0;
CYCLE( cur, "Firm1" )
{
	v[6] = VS( cur, "_L1d" );					// firm total labor demand
	v[7] = VS( cur, "_L1dRD" );					// firm R&D labor demand

	if ( v[3] > 0 )
		v[8] = ceil( v[7] * v[0] / v[3] );		// effective firm workers in R&D
	else
		v[8] = 0;

	// total production workers in firm after possible shortage of workers
	if ( v[2] > v[3] )
		v[9] = round( ( v[6] - v[7] ) * ( v[1] - v[0] ) / ( v[2] - v[3] ) );
	else
		v[9] = 0;

	v[10] = min( v[8] + v[9], v[6] );

	// prevent rounding errors from allocating exactly the total workers number
	if ( v[4] + v[10] > v[1] || ( NEXTS( cur ) == NULL && v[4] + v[10] < v[1] ) )
		v[10] = v[1] - v[4];

	v[4] += v[10];								// update allocated workers
	v[8] = min( v[8], v[10] );					// enforce firm total

	// prevent rounding errors again, for total R&D workers number
	if ( v[5] + v[8] > v[0] || ( NEXTS( cur ) == NULL && v[5] + v[8] < v[0] ) )
		v[8] = v[0] - v[5];

	v[5] += v[8];								// update allocated R&D workers

	WRITES( cur, "_L1", v[10] );
	WRITES( cur, "_L1rd", v[8] );
}

RESULT( v[0] )


EQUATION( "NW1" )
/*
Total net wealth (free cash) of firms in capital-good sector
*/
RESULT( SUM( "_NW1" ) )


EQUATION( "Pi1" )
/*
Total profits of capital-good sector
*/
RESULT( SUM( "_Pi1" ) )


EQUATION( "PPI" )
/*
Producer price index
*/
V( "f1rescale" );								// ensure m.s. computed/rescaled
RESULT( WHTAVE( "_p1", "_f1" ) )


EQUATION( "Q1" )
/*
Total planned output of firms in capital-good sector
*/
RESULT( SUM( "_Q1" ) )


EQUATION( "Q1e" )
/*
Total effective real output (orders) of capital-good sector
*/
RESULT( SUM( "_Q1e" ) )


EQUATION( "S1" )
/*
Total sales of capital-good sector
*/
RESULT( V( "S1l" ) + V( "S1x" ) )


EQUATION( "S1l" )
/*
Local sales of capital-good sector
*/
RESULT( SUM( "_S1l" ) )


EQUATION( "S1x" )
/*
Export sales of capital-good sector
*/
RESULT( SUM( "_S1x" ) )


EQUATION( "Tax1" )
/*
Taxes on profits paid by firms in capital-good sector
*/
RESULT( SUM( "_Tax1" ) )


EQUATION( "TaxM1f" )
/*
Foreign import duties (in domestic currency) paid by capital-good sector
*/
V( "S1x" );										// ensure foreign sales processed
RESULT( SUM( "_TaxM1f" ) )


EQUATION( "TaxX1" )
/*
Local export duties (in domestic currency) paid by capital-good sector
*/
V( "S1x" );										// ensure foreign sales processed
RESULT( SUM( "_TaxX1" ) )


EQUATION( "W1" )
/*
Total wages paid by firms in capital-good sector
*/

V( "L1" );										// ensure hiring is done

v[0] = 0;										// wage accumulator
CYCLE( cur, "Wrk1" )
	v[0] += VS( SHOOKS( cur ), "_w" );			// go through all workers

RESULT( v[0] * VS( LABSUPL1, "Lscale" ) )		// consider labor scaling


EQUATION( "X1" )
/*
Exports (in domestic currency including duties) by capital-good sector
*/
V( "S1x" );										// ensure foreign sales processed
RESULT( SUM( "_X1" ) )


EQUATION( "dA1b" )
/*
Notional productivity (bounded) rate of change in capital-good sector
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "A1", VS( COUNTRL1, "mLim" ), VS( COUNTRL1, "mPer" ) ) )


EQUATION( "ed1avg" )
/*
Average education level in capital-good sector
*/

v[1] = V( "L1" );								// ensure hiring is done

v[2] = 0;										// schooling-years accumulator
CYCLE( cur, "Wrk1" )
	v[2] += VS( SHOOKS( cur ), "_ed" );			// go through all workers

RESULT( v[1] > 0 ? v[2] * VS( LABSUPL1, "Lscale" ) / v[1] : CURRENT )


EQUATION( "i1" )
/*
Interest paid by capital-good sector
*/
RESULT( SUM( "_i1" ) )


EQUATION( "iD1" )
/*
Interest received from deposits by capital-good sector
*/
RESULT( SUM( "_iD1" ) )


EQUATION( "imi" )
/*
Imitation success rate in capital-good sector
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
SUM( "_Atau" );									// ensure innovation is done
SUM( "_NC" );									// ensure brochures distributed
VS( CONSECL1, "sV2avg" );						// ensure vintage skills updt'd
RESULT( SUM( "_imi" ) / V( "F1" ) )


EQUATION( "inn" )
/*
Innovation success rate in capital-good sector
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
V( "imi" );										// ensure innovation is done
RESULT( SUM( "_inn" ) / V( "F1" ) )


EQUATION( "p1avg" )
/*
Weighted average price charged in capital-good sector
*/
v[1] = V( "Q1e" );
RESULT( v[1] > 0 ? WHTAVE( "_p1", "_Q1e" ) / v[1] : AVE( "_p1" ) )


EQUATION( "quits1" )
/*
Number of workers quitting jobs (not fired) in period in capital-good sector
Updated in 'hires1' and 'hires2'
*/

V( "retires1" );								// ensure retiring is done

v[1] = VS( LABSUPL1, "wU" );					// unemployment benefit in t

i = 0;
CYCLE_SAFE( cur, "Wrk1" )
	if ( VS( SHOOKS( cur ), "_w" ) <= v[1] ||	// wage under unemp. benefit?
		 VS( SHOOKS( cur ), "_emig" ) )			// or emigrating?
	{
		CFUNS( SHOOKS( cur ), fire_worker );	// register quit
		++i;									// scaled equivalent fires
	}

RESULT( i * VS( LABSUPL1, "Lscale" ) )


EQUATION( "retires1" )
/*
Workers retiring (not fired) in capital-good sector
*/

VS( LABSUPL1, "Ls" );							// ensure emigration is done

if ( VS( LABSUPL1, "Tr" ) == 0 )				// retirement disabled?
	END_EQUATION( 0 )

i = 0;
CYCLE_SAFE( cur, "Wrk1" )
	if ( VS( SHOOKS( cur ), "_age" ) == 1 )		// is a "reborn"?
	{
		CFUNS( SHOOKS( cur ), fire_worker );	// register retirement
		++i;									// scaled equivalent fires
	}

RESULT( i * VS( LABSUPL1, "Lscale" ) )


EQUATION( "sT1min" )
/*
Minimum workers tenure skills in capital-good sector
*/

i = VS( COUNTRL1, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 1 )							// no tenure learning?
	END_EQUATION( 1 );							// skills = 1

V( "L1" );										// ensure hiring is done

v[0] = DBL_MAX;									// current minimum
CYCLE( cur, "Wrk1" )
{
	v[1] = VS( SHOOKS( cur ), "_sT" );			// worker current tenure skills
	if ( v[1] < v[0] )
		v[0] = v[1];							// save minimum
}

RESULT( v[0] < DBL_MAX ? v[0] : 1 )


EQUATION( "w1avg" )
/*
Average wage paid by firms in capital-good sector
*/

v[1] = V( "L1" );								// workers in sector 1

if ( v[1] == 0 )								// no worker?
	v[0] = VS( CONSECL1, "w2avg" );				// use sector 2 wage as proxy
else
	v[0] = V( "W1" ) / v[1];

RESULT( v[0] > 0 ? v[0] : CURRENT )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "f1rescale" )
/*
Rescale market shares in capital-good sector to ensure adding to 1
To be called after market shares are changed in '_f1' and 'entry1exit'
*/

v[1] = SUM( "_f1" );							// add-up market shares

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator

if ( v[1] > 0 )									// production ok?
	CYCLE( cur, "Firm1" )						// rescale to add-up to 1
	{
		v[0] += v[2] = VS( cur, "_f1" ) / v[1];	// rescaled market share
		WRITES( cur, "_f1", v[2] );				// save updated m.s.
	}
else
{
	v[2] = 1 / COUNT( "Firm1" );				// firm fair share

	CYCLE( cur, "Firm1" )						// rescale to add-up to 1
	{
		v[0] += v[2];
		WRITES( cur, "_f1", v[2] );
	}
}

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "Eq1entryG", "" )
/*
Cost of new equity from firms rescued by government in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "Eq1entryW", "" )
/*
Cost of new equity from private-firm entries in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "Eq1exitG", "" )
/*
Destroyed equity from government-owned firm exits in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "Eq1exitW", "" )
/*
Destroyed equity from private-firm exits in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "JO11", "JO1" )
/*
Open job positions of category 1 in capital-good sector
Updated in 'JO1'
*/

EQUATION_DUMMY( "JO12", "JO1" )
/*
Open job positions of category 2 in capital-good sector
Updated in 'JO1'
*/

EQUATION_DUMMY( "JO13", "JO1" )
/*
Open job positions of category 3 in capital-good sector
Updated in 'JO1'
*/

EQUATION_DUMMY( "NW1exitG", "" )
/*
Liquidation net worth returned from government firm exits in capital sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "NW1exitW", "" )
/*
Liquidation net worth returned from private-firm exits in capital sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "entry1", "entry1exit" )
/*
Rate of entering firms in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "exit1", "entry1exit" )
/*
Rate of exiting firms in capital-good sector
Updated in 'entry1exit'
*/
