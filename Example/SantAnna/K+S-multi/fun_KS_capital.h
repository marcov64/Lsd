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
*/

v[1] = VS( LABSUPL1, "Ls" );					// available labor force
v[2] = V( "L1dRD" );							// R&D labor demand in sector 1
v[3] = V( "L1d" );								// production labor in sector 1
v[4] = SUMS( PARENT, "L2d" );					// production labor in sector 2
v[5] = COUNT( "Wrk1" ) * VS( LABSUPL1, "Lscale" );// current workers

v[2] = min( v[2], v[1] );						// ignore demand over total labor
v[3] = min( v[3], v[1] );

v[3] -= v[2];									// labor used in production

// split possible labor shortage proportionally up to a limit
if ( ( v[3] + v[4] ) > ( v[1] - v[2] ) )
	v[3] *= max( ( v[1] - v[2] ) / ( v[3] + v[4] ), 1 - V( "L1shortMax" ) );

RESULT( max( ceil( v[2] + v[3] - v[5] ), 0 ) )	// hires scaled and rounded up


EQUATION( "MC1" )
/*
Market conditions index for entry in capital-good sector
*/
RESULT( log( max( VL( "NW1", 1 ), 0 ) + 1 ) - log( VL( "Deb1", 1 ) + 1 ) )


EQUATION( "entry1exit" )
/*
Net (number of) entrant firms in capital-good sector
Perform entry and exit of firms in the capital-good sector
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
Updates 'F1', 'cEntry', 'cExit', 'entry1', 'exit1'
*/

UPDATE;											// ensure aggregates are computed

double MC1 = V( "MC1" );						// market conditions in sector 1
double MC1_1 = VL( "MC1", 1 );					// market conditions in sector 1
double omicron = VS( PARENT, "omicron" );		// entry sensitivity to mkt cond
double stick = VS( PARENT, "stick" );			// stickiness in number of firms
double x2inf = VS( PARENT, "x2inf" );			// entry lower distrib. support
double x2sup = VS( PARENT, "x2sup" );			// entry upper distrib. support
int F1 = V( "F1" );								// current number of firms
int F10 = V( "F10" );							// initial number of firms
int F1max = V( "F1max" );						// max firms in sector 1
int F1min = V( "F1min" );						// min firms in sector 1

v[1] = v[2] = v[3] = 0;							// firm entry-exit cost accum.

// firms exit, except the best one if all going to quit
j = CFUN( exit_firm1, & v[1], & v[2], & v[3], false );

V( "f1rescale" );								// redistribute exiting m.s.

// compute the potential number of entrants
v[4] = ( MC1_1 == 0 ) ? 0 : MC1 / MC1_1 - 1;	// change in market conditions

k = max( 0, ceil( F1 * ( ( 1 - omicron ) * uniform( x2inf, x2sup ) +
						 omicron * min( max( v[4], x2inf ), x2sup ) ) ) );

// apply return-to-the-average stickiness random shock to the number of entrants
if ( F1 - j + k > F10 )
	k -= min( stick * RND * ( ( double ) ( F1 - j ) / F10 - 1 ) * F10, k );

// ensure limits are enforced to the number of entrants
if ( F1 - j + k < F1min )
	k = F1min - F1 + j;

if ( F1 - j + k > F1max )
	k = F1max - F1 + j;

v[0] = k - j;									// net number of entrants

if ( k > 0 )
	v[1] += CFUN( entry_firm1, k, false );		// add entrant-firm objects

i = INCR( "F1", v[0] );							// update the number of firms
INCRS( PARENT, "cEntry", v[1] );				// account equity cost of entry
INCRS( PARENT, "cExit", v[2] );					// account exit credits
WRITE( "exit1", ( double ) j / F1 );
WRITE( "entry1", ( double ) k / F1 );
WRITE( "exit1fail", v[3] / F1 );

V( "f1rescale" );								// redistribute entrant m.s.
INIT_TSEARCH( "Firm1" );						// update turbo search indexing

RESULT( v[0] )


EQUATION( "fires1" )
/*
Number of workers fired in capital-good sector
Process required firing
*/

VS( LABSUPL1, "retires" );						// process retirements
VS( LABSUPL1, "quits" );						// process spontaneous quits
RECALCS( LABSUPL1, "quits" );					// recompute to add addt'l quits

v[1] = VS( LABSUPL1, "Lscale" );				// labor scaling
v[2] = VS( LABSUPL1, "Ls" );					// available labor force
v[3] = V( "L1dRD" );							// R&D labor demand in sector 1
v[4] = V( "L1d" ) - v[3];						// production labor in sector 1
v[5] = SUMS( PARENT, "L2d" );					// production labor in sector 2
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
CFUN( order_workers, ( int ) VS( PARENT, "flagFireOrder1" ), OBJ_WRK1 );

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

VS( LABSUPL1, "fires" );						// make sure firing is done
VS( LABSUPL1, "appl" );							// and applications are done

v[1] = VS( LABSUPL1, "Lscale" );				// labor scaling

j = ceil( V( "JO1" ) / v[1] );					// scaled open jobs in sector 1
appLisT *appl = & V_EXTS( PARENT, countryE, firm1appl );
												// pointer to applications pool
// get current top wage offered
v[2] = MAXS( PARENT, "w2oMax" );

// sort sector 1 candidate list according to the defined mode
CFUN( order_applications, ( int ) VS( PARENT, "flagHireOrder1" ), appl );

// hire the ordered applications until queue is exhausted
i = 0;											// counter to hired workers
v[3] = DBL_MAX;									// minimum wage requested
cur = NULL;										// pointer to worker asking it

while ( j > 0 && appl->size( ) > 0 )
{
	// get the candidate worker object element and a pointer to it
	const application candidate = appl->front( );

	// offered wage ok?
	if ( ROUND( candidate.w, v[2], 0.01 ) <= v[2] )
	{
		// already employed? First quit current job
		if ( VS( candidate.wrk, "_employed" ) )
			CFUNS( candidate.wrk, quit_worker );

		// flag hiring and set wage, employer and vintage to be used by worker
		CFUNS( candidate.wrk, hire_worker, 1, THIS, v[2] );// set firm, vintage & wage
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
		CFUNS( cur, quit_worker );

	CFUNS( cur, hire_worker, 1, THIS, v[3] );	// pay requested wage
	++i;
}

appl->clear( );									// clear job application queue

RESULT( i * v[1] )


EQUATION( "g1front" )
/*
ID of technological frontier for machine generation (paradigm)
Also updates 'Capital' object static pointer to the top tech. object 'T1'
*/

if ( bernoulli( V( "zetaG" ) ) )				// new generation accessed?
	v[0] = CFUN( add_generation );				// add new generation object
else
	v[0] = CURRENT;

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A1max" )
/*
Maximum (single-stage) labor productivity of machines produced in
capital-good sector
*/
RESULT( MAX( "_A1" ) )


EQUATION( "B1" )
/*
Labor productivity of capital-good sector
*/
V( "p1" );										// ensure m.s. are updated
RESULT( WHTAVE( "_B1", "_f1" ) )


EQUATION( "D1" )
/*
Potential demand (in money terms) received by firms in
capital-good sector
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


EQUATION( "HH1" )
/*
Normalized Herfindahl-Hirschman index for capital-good sector
*/

v[1] = i = 0;									// index accumulator & counter
CYCLE( cur, "Firm1" )
{
	v[1] += pow( VS( cur, "_f1" ), 2 );
	++i;
}

RESULT( i > 1 ? max( 0, ( v[1] - 1.0 / i ) / ( 1 - 1.0 / i ) ) : 1 )


EQUATION( "HP1" )
/*
Hymer-Pashigian index for capital-good sector
*/

v[0] = 0;										// index accumulator
CYCLE( cur, "Firm1" )
	v[0] += fabs( VLS( cur, "_f1", 1 ) - VS( cur, "_f1" ) );// sum share changes

RESULT( v[0] )


EQUATION( "L1" )
/*
Work force (labor) size employed by capital-good sector
Result is scaled according to the defined scale
*/
VS( LABSUPL1, "hires" );						// ensure hires are done
RESULT( COUNT( "Wrk1" ) * VS( LABSUPL1, "Lscale" ) )


EQUATION( "L1d" )
/*
Total labor demand from firms in capital-good sector
Includes R&D labor
*/
RESULT( SUM( "_L1d" ) )


EQUATION( "L1dRD" )
/*
Total R&D labor demand from firms in capital-good sector
*/
RESULT( SUM( "_L1dRD" ) )


EQUATION( "L1rd" )
/*
Total R&D labor demand from firms in capital-good sector
*/
RESULT( min( V( "L1dRD" ),
		min( V( "L1" ), round( VS( LABSUPL1, "Ls" ) * V( "L1rdMax" ) ) ) ) )


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


EQUATION( "RD1" )
/*
R&D expenditure of capital-good sector
*/
RESULT( SUM( "_RD1" ) )


EQUATION( "S1" )
/*
Total sales (in money terms) of capital-good sector
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


EQUATION( "age1avg" )
/*
Average age of firms in capital-good sector
*/
RESULT( T - AVE( "_t1ent" ) )


EQUATION( "cred1c" )
/*
Total credit constraint of firms in capital-good sector
*/
RESULT( SUM( "_cred1c" ) )


EQUATION( "dB1b" )
/*
Notional productivity (bounded) rate of change in capital-good sector
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "B1", VS( PARENT, "mLim" ) ) )


EQUATION( "g1max" )
/*
ID of most advanced machine generation (paradigm) in production
*/
RESULT( MAX( "_g1" ) )


EQUATION( "imi1" )
/*
Imitation success rate in capital-good sector
Also ensures all brochures are distributed and
learning-by-doing skills are updated
*/
SUM( "_A1" );									// ensure innovation is done
SUM( "_NC1" );									// ensure brochures distributed
VS( LABSUPL1, "sVavg" );						// ensure vintage skills updt'd
RESULT( SUM( "_imi1" ) / V( "F1" ) )


EQUATION( "inn1i" )
/*
Incremental innovation success rate in capital-good sector
*/
V( "imi1" );									// ensure imitation is done
RESULT( SUM( "_inn1i" ) / V( "F1" ) )


EQUATION( "inn1r" )
/*
Radical innovation success rate in capital-good sector
*/
V( "imi1" );									// ensure imitation is done
RESULT( SUM( "_inn1r" ) / V( "F1" ) )


EQUATION( "noWrk1" )
/*
Share of operating firms with no worker hired in capital-good sector
*/
RESULT( COUNT_CND( "Firm1", "_L1", "==", 0 ) / COUNT( "Firm1" ) )


EQUATION( "p1" )
/*
Weighted (by market share) average price charged by capital-good industry
Ensures market shares are properly computed/rescaled after production is done
Also prevents multiple runs of the rescaling algorithm
*/
V( "f1rescale" );								// ensure m.s. computed/rescaled
RESULT( WHTAVE( "_p1", "_f1" ) )


EQUATION( "quits1" )
/*
Number of workers quitting (not fired) from firms in capital-good sector
Updated in 'hires1' and 'hires'
*/

if ( VS( PARENT, "flagGovExp" ) < 2 )			// unemployment benefit exists?
	END_EQUATION( 0 );

v[1] = VS( LABSUPL1, "wU" );					// unemployment benefit in t

i = 0;
CYCLE_SAFE( cur, "Wrk1" )
	if ( VS( SHOOKS( cur ), "_w" ) <= v[1] )	// wage under unemp. benefit?
	{
		CFUNS( SHOOKS( cur ), fire_worker );	// register quit
		++i;									// scaled equivalent fires
	}

RESULT( i * VS( LABSUPL1, "Lscale" ) )


EQUATION( "retires1" )
/*
Number of workers retiring (not fired) from firms in capital-good sector
*/

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


EQUATION( "w1avg" )
/*
Average wage paid by firms in capital-good sector
*/
v[1] = V( "L1" );
RESULT( v[1] > 0 ? V( "W1" ) / v[1] : CURRENT )

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
		WRITES( cur, "_f1", v[2] );				// save updated share
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

EQUATION_DUMMY( "entry1", "" )
/*
Rate of entering firms in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "exit1", "" )
/*
Rate of exiting firms in capital-good sector
Updated in 'entry1exit'
*/

EQUATION_DUMMY( "exit1fail", "" )
/*
Rate of bankrupt firms in capital-good sector
Updated in 'entry1exit'
*/
