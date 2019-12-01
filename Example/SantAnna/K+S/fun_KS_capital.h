/******************************************************************************

	CAPITAL-GOODS MARKET OBJECT EQUATIONS
	-------------------------------------

	Equations that are specific to the capital-goods market objects in the 
	K+S LSD model are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "JO1" )
/*
Open job positions in capital-good sector
*/

v[1] = VS( LABSUPL1, "Ls" );					// available labor force
v[2] = V( "L1rd" );								// R&D labor in sector 1
v[3] = V( "L1d" ) - v[2];						// production labor in sector 1
v[4] = VS( CONSECL1, "L2d" );					// production labor in sector 2
v[5] = COUNT( "Wrk1" ) * VS( LABSUPL1, "Lscale" );// current workers

// split possible labor shortage proportionally up to a limit (to avoid crashes)
if ( ( v[3] + v[4] ) > ( v[1] - v[2] ) )
	v[3] *= max( ( v[1] - v[2] ) / ( v[3] + v[4] ), 1 - V( "short1max" ) );

RESULT( max( 0, ceil( v[2] + v[3] - v[5] ) ) )	// hires scaled and rounded up


EQUATION( "MC1" )
/*
Market conditions index for entry in capital-good sector
*/
RESULT( log( max( VL( "NW1", 1 ), 0 ) + 1 ) - log( VL( "Deb1", 1 ) + 1 ) )


EQUATION( "entry1exit" )
/*
Perform entry and exit of firms in the capital-good sector
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
*/

VS( CONSECL1, "K" );							// ensure canceled orders acct'd
VS( PARENT, "GDP" );							// ensure markets are closed

V( "A1" ); V( "D1" ); V( "Deb1" ); V( "Div1" );	// ensure aggregates in sector
V( "JO1" ); V( "L1" ); V( "L1d" ); V( "L1rd" );	// computed before entry/exit
V( "NW1" ); V( "Pi1" ); V( "Q1" ); V( "Q1e" ); V( "S1" ); V( "Tax1" ); 
V( "W1" ); V( "dA1b" ); V( "fires1" ); V( "hires1" ); V( "imi" ); V( "inn" ); 
V( "quits1" ); V( "retires1" ); V( "sT1min" ); V( "w1avg" );

VS( SECSTAL1, "HH1" ); VS( SECSTAL1, "HP1" ); VS( SECSTAL1, "RD" ); 
VS( SECSTAL1, "age1avg" ); VS( SECSTAL1, "cred1c" ); VS( SECSTAL1, "s1avg" ); 
VS( LABSTAL1, "L1ent" ); VS( LABSTAL1, "L1exit" ); VS( LABSTAL1, "L1v" ); 

double MC1 = V( "MC1" );						// market conditions in sector 1
double MC1_1 = VL( "MC1", 1 );					// market conditions in sector 1
double NW10u = V( "NW10" ) * V( "PPI" ) / V( "PPI0" );// minimum wealth in s. 1
double n1 = V( "n1" );							// market participation period
double omicron = VS( PARENT, "omicron" );		// entry sensitivity to mkt cond
double stick = VS( PARENT, "stick" );			// stickiness in number of firms
double x2inf = VS( PARENT, "x2inf" );			// entry lower distrib. support
double x2sup = VS( PARENT, "x2sup" );			// entry upper distrib. support
int F1 = V( "F1" );								// current number of firms
int F10 = V( "F10" );							// initial number of firms
int F1max = V( "F1max" );						// max firms in sector 1
int F1min = V( "F1min" );						// min firms in sector 1

h = F1;											// initial number of firms
vector < bool > quit( h, false );				// vector of firms' quit status

// mark bankrupt and market-share-irrelevant firms to quit the market
j = k = 0;										// best firm registers
i = 0;											// firm counter
v[1] = v[2] = 0;								// accumulators
CYCLE( cur, "Firm1" )
{
	if ( t >= VS( cur, "_t1ent" ) + n1 )
	{
		for ( v[3] = j = 0; j < n1; ++j )
			v[3] += VLS( cur, "_BC", j );		// n1 periods customer number
		
		if ( VS( cur, "_NW1" ) < 0 || v[3] == 0 )
		{
			quit[ i ] = true;					// mark for likely exit
			--h;								// one less firm
			
			if ( v[3] > j )						// best firm so far?
			{
				k = i;							// save firm index
				j = v[3];						// and customer number
			}
		}
	}
	
	++i;
}	

// quit candidate firms exit, except the best one if all going to quit
i = j = 0;										// firm counters
CYCLE_SAFE( cur, "Firm1" )
{
	if ( quit[ i ] )
	{
		if ( h > 0 || i != k )					// firm must exit?
		{
			// account liquidation credit due to public, if any
			v[2] += exit_firm1( cur );			// delete object and liq. val.
			
			++j;								// count exits
		}
		else
			if ( h == 0 && i == k )				// best firm must get new equity
			{
				// new equity required
				v[4] = NW10u + VS( cur, "_Deb1" ) - VS( cur, "_NW1" );
				v[1] += v[4];					// accumulate "entry" equity cost
				
				WRITES( cur, "_Deb1", 0 );		// reset debt
				INCRS( cur, "_NW1", v[4] );		// add new equity
			}
	}

	++i;
}

// compute the potential number of entrants
v[5] = ( MC1_1 == 0 ) ? 0 : MC1 / MC1_1 - 1;// change in market conditions

k = max( 0, ceil( F1 * ( ( 1 - omicron ) * uniform( x2inf, x2sup ) + 
						 omicron * min( max( v[5], x2inf ), x2sup ) ) ) );
				 
// apply return-to-the-average stickiness random shock to the number of entrants
k -= min( RND * stick * ( ( double ) ( F1 - j ) / F10 - 1 ) * F10, k );

// ensure limits are enforced to the number of entrants
if ( F1 - j + k < F1min )
	k = F1min - F1 + j;

if ( F1 + k > F1max )
	k = F1max - F1 + j;

v[0] = k - j;									// net number of entrants
v[1] += entry_firm1( p, k, false );				// add entrant-firm objects

i = INCR( "F1", v[0] );							// update the number of firms
INCRS( PARENT, "cEntry", v[1] );				// account equity cost of entry
INCRS( PARENT, "cExit", v[2] );					// account exit credits
WRITE( "exit1", ( double ) j / F1 );
WRITE( "entry1", ( double ) k / F1 );

V( "f1rescale" );								// redistribute exiting m.s.
INIT_TSEARCHT( "Firm1", i );					// prepare turbo search indexing

RESULT( v[0] )


EQUATION( "fires1" )
/*
Number of workers fired in capital-good sector
Process required firing
*/

V( "retires1" );								// process retirements
VS( CONSECL1, "retires2" );
V( "quits1" );									// process quits
VS( CONSECL1, "quits2" );

v[1] = VS( LABSUPL1, "Lscale" );				// labor scaling
v[2] = VS( LABSUPL1, "Ls" );					// available labor force
v[3] = V( "L1rd" );								// R&D labor in sector 1
v[4] = V( "L1d" ) - v[3];						// production labor in sector 1
v[5] = VS( CONSECL1, "L2d" );					// production labor in sector 2
v[6] = COUNT( "Wrk1" );							// current workers (scaled)

// split possible labor shortage proportionally (to avoid crashes)
if ( ( v[4] + v[5] ) > ( v[2] - v[3] ) )
	v[4] *= ( v[2] - v[3] ) / ( v[4] + v[5] );

// fires in sector 1, scaled-down and rounded-up
j = v[6] - ceil( ( v[4] + v[3] ) / v[1] );

if ( j <= 0 )									// nobody to fire?
	END_EQUATION( 0 );

if ( j >= v[6] )
	j = v[6] - 1;								// keep at least 1 scaled worker

// order firing list
order_workers( ( int ) VS( PARENT, "flagFireOrder1" ), OBJ_WRK1, p );

// then check firing worker by worker in sector 1 pool
i = 0;
CYCLE_SAFE( cur, "Wrk1" )
	if ( i < j )
	{
		cur1 = SHOOKS( cur );					// pointer to Worker object
		if ( VLS( cur1, "_Te", 1 ) + 1 < VS( cur1, "_Tc" ) )// contract not over?
			continue;							// go to next worker
		
		fire_worker( cur1 );					// register fire
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
VS( LABSUPL1, "appl" );							// and applications are done

v[1] = VS( LABSUPL1, "Lscale" );				// labor scaling

j = ceil( V( "JO1" ) / v[1] );					// scaled open jobs in sector 1
appLisT *appl = & V_EXTS( PARENT, country, firm1appl );
												// pointer to applications pool
// get current top wage offered
v[2] = VS( CONSECL1, "w2oMax" );

// sort sector 1 candidate list according to the defined mode
order_applications( ( int ) VS( PARENT, "flagHireOrder1" ), appl );

// hire the ordered applications until queue is exhausted
i = 0;											// counter to hired workers
v[3] = DBL_MAX;									// minimum wage requested
cur = NULL;										// pointer to worker asking it

while ( j > 0 && appl->size( ) > 0 )
{
	// get the candidate worker object element and a pointer to it
	const application candidate = appl->front( );

	// offered wage ok (ignore very small differences)?
	if ( ROUND( candidate.w, v[2] ) <= v[2] )
	{
		// already employed? First quit current job
		if ( VS( candidate.wrk, "_employed" ) )
			quit_worker( candidate.wrk );

		// flag hiring and set wage, employer and vintage to be used by worker
		hire_worker( candidate.wrk, 1, p, v[2] );// set firm, vintage & wage
		++i;									// scaled count hire
		--j;									// adjust scaled labor demand
	}
	else
		if ( candidate.w < v[3] )
		{
			v[3] = candidate.w;
			cur = candidate.wrk;
		}
	
	// remove worker from candidate queue
	appl->pop_front();
}

// try to hire at least one worker, at any wage
if ( j > 0 && i == 0 && cur != NULL )			// none hired but someone avail?
{
	if ( VS( cur, "_employed" ) )				// quit job if needed
		quit_worker( cur );
	
	hire_worker( cur, 1, p, v[3] );				// pay requested wage
	++i;
}	

appl->clear( );									// clear job application queue

RESULT( i * v[1] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A1" )
/*
Productivity of capital-good sector
*/
v[1] = V( "Q1e" );
RESULT( v[1] > 0 ? WHTAVE( "_Btau", "_Q1e" ) / v[1] : AVE( "_Btau" ) )


EQUATION( "D1" )
/*
Potential demand (orders) received by firms in capital-good sector
*/
RESULT( SUM( "_D1" ) )


EQUATION( "Deb1" )
/*
Total debt of capital-good sector
*/
RESULT( SUM( "_Deb1" ) )


EQUATION( "Div1" )
/*
Total dividends paid by firms in capital-good sector
*/
V( "Tax1" );									// ensure dividends are computed
RESULT( SUM( "_Div1" ) )


EQUATION( "F1" )
/*
Number of firms in capital-good sector
*/
RESULT( COUNT( "Firm1" ) )


EQUATION( "imi" )
/*
Imitation successes in capital-good sector
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
SUM( "_Atau" );									// ensure innovation is done
SUM( "_NC" );									// ensure brochures distributed
VS( CONSECL1, "sV2avg" );						// ensure vintage skills updt'd
RESULT( SUM( "_imi" ) )


EQUATION( "inn" )
/*
Innovation successes in capital-good sector
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
V( "imi" );										// ensure innovation is done
RESULT( SUM( "_inn" ) )


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


EQUATION( "L1rd" )
/*
Total R&D labor demand from firms in capital-good sector
*/
RESULT( SUM( "_L1rd" ) )


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
RESULT( SUM( "_S1" ) )


EQUATION( "Tax1" )
/*
Total taxes paid by firms in capital-good sector
*/
RESULT( SUM( "_Tax1" ) )


EQUATION( "W1" )
/*
Total wages paid by firms in capital-good sector
*/

v[0] = 0;										// wage accumulator
CYCLE( cur, "Wrk1" )
	v[0] += VS( SHOOKS( cur ), "_w" );			// go through all workers

RESULT( v[0] * VS( LABSUPL1, "Lscale" ) )		// consider labor scaling


EQUATION( "dA1b" )
/*
Notional productivity (bounded) rate of change in capital-good sector
Used for wages adjustment only
*/
RESULT( mov_avg_bound( p, "A1", VS( PARENT, "mLim" ) ) )


EQUATION( "quits1" )
/*
Number of workers quitting jobs (not fired) in period in capital-good sector
Updated in 'hires1' and 'hires2'
*/

v[1] = VS( LABSUPL1, "wU" );					// unemployment benefit in t

i = 0;
CYCLE_SAFE( cur, "Wrk1" )
	if ( VS( SHOOKS( cur ), "_w" ) <= v[1] )	// wage under unemp. benefit?
	{
		fire_worker( SHOOKS( cur ) );			// register quit
		++i;									// scaled equivalent fires
	}

RESULT( i * VS( LABSUPL1, "Lscale" ) )


EQUATION( "retires1" )
/*
Workers retiring (not fired) in capital-good sector
*/

if ( VS( LABSUPL1, "Tr" ) == 0 )				// retirement disabled?
	END_EQUATION( 0 )

i = 0;
CYCLE_SAFE( cur, "Wrk1" )
	if ( VS( SHOOKS( cur ), "_age" ) == 1 )		// is a "reborn"?
	{
		fire_worker( SHOOKS( cur ) );			// register retirement
		++i;									// scaled equivalent fires
	}

RESULT( i * VS( LABSUPL1, "Lscale" ) )


EQUATION( "sT1min" )
/*
Minimum workers tenure skills in capital-good sector
*/

i = VS( PARENT, "flagWorkerLBU" );				// worker-level learning mode
if ( i == 0 || i == 1 )							// no tenure learning?
	END_EQUATION( 1 );							// skills = 1
	
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

if ( v[1] == 0 )
	v[0] =  VS( CONSECL1, "w2avg" );			// use sector 2 wage as proxy
else
{
	v[0] = 0;									// wage accumulator
	CYCLE( cur, "Wrk1" )
		v[0] += VS( SHOOKS( cur ), "_w" );
	
	v[0] *= VS( LABSUPL1, "Lscale" ) / v[1];	// compute average
}

RESULT( v[0] > 0 ? v[0] : CURRENT )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "f1rescale" )
/*
Rescale market shares in capital-good sector to ensure adding to 1
To be called after market shares are changed in '_f1' and 'entry1exit'
*/

v[1] = SUM( "_f1" );							// add-up market shares

if ( ROUND( v[1], 1 ) == 1.0 )					// ignore rounding errors
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

EQUATION_DUMMY( "exit1fail", "entry1exit" )
/*
Rate of bankrupt firms in capital-good sector
Updated in 'entry1exit'
*/
