/******************************************************************************

	CONSUMER-GOODS MARKET OBJECT EQUATIONS
	--------------------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the consumer-goods market objects in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "D2" )
/*
Demand fulfilled (in money terms) by firms in consumption-good industry
Also updates '_D2', '_l2'
*/

VS( PARENT, "DcBas" );							// ensure industry demand alloc.
VS( PARENT, "DcLux" );
V( "p2" );										// ensure m.s. are updated

k = V( "F2" );									// number of firms
v[1] = V( "D2a" );								// total desired nominal

// create/fill temporary share and supply vectors & initialize firm demand
dblVecT _f2( k ), _sup2( k );

j = 0;
CYCLE( cur, "Firm2" )
{
	// firm available supply
	_sup2[ j ] = ( VS( cur, "_Q2e" ) + VLS( cur, "_N2", 1 ) ) * VS( cur, "_p2" );
	_f2[ j ] = VS( cur, "_f2" );				// firm market share
	WRITES( cur, "_D2", 0 );					// demand fulfilled accumulator
	WRITES( cur, "_l2", 0 );					// assume no unsatisfied demand
	++j;
}

// cycle through firms until all demand is allocated or no more product to sell
v[0] = i = 0;									// fulfilled demand accumulator
while ( v[1] > 0.01 )
{
	v[2] = v[1];								// remaining unallocated demand
	v[3] = j = 0;								// shares yet unallocated
	CYCLE( cur, "Firm2" )
	{
		if ( _f2[ j ] > 0 )						// firm has demand to supply
		{
			if ( _sup2[ j ] > 0 )				// product to supply?
			{
				v[4] = v[1] * _f2[ j ];			// firm demand allocation

				if ( v[4] <= _sup2[ j ] )		// can supply all demanded?
				{
					// supply all demanded
					INCRS( cur, "_D2", v[4] );
					v[0] += v[4];				// accumulate to total demand
					v[2] -= v[4];				// discount from desired demand
					v[3] += _f2[ j ];			// save share yet to allocate
					_sup2[ j ] -= v[4];			// make supplied unavailable
				}
				else
				{
					if ( i == 0 )				// unsatisfied demand metric
						WRITES( cur, "_l2", ( v[4] - _sup2[ j ] ) /
											VS( cur, "_p2" ) );

					// supply all available
					INCRS( cur, "_D2", _sup2[ j ] );
					v[0] += _sup2[ j ];			// accumulate to total demand
					v[2] -= _sup2[ j ];			// discount from desired demand
					_f2[ j ] = _sup2[ j ] = 0;	// nothing else to supply
				}
			}
			else
				_f2[ j ] = 0;					// nothing else to supply
		}

		++j;
	}

	if ( v[3] > 0 )								// unallocated shares remaining?
		for ( j = 0; j < k; ++j )
			_f2[ j ] /= v[3];					// rescale remaining
	else
		break;									// nothing else to supply

	v[1] = v[2];								// update unallocated
	++i;
}

RESULT( v[0] )


EQUATION( "E2" )
/*
Effective competitiveness of industry in consumption-goods sector, considering
the price, quality, newness and complexity of the product for the consumer
*/

double delta1 = VS( PARENT, "delta1" );
double delta2 = VS( PARENT, "delta2" );
double delta3 = VS( PARENT, "delta3" );
double delta4 = VS( PARENT, "delta4" );
double pCmax = log( VLS( PARENT, "pCmax", 1 ) + 1 );
double pCmin = log( VLS( PARENT, "pCmin", 1 ) + 1 );
double qCmax = log( VLS( PARENT, "qCmax", 1 ) + 1 );
double qCmin = log( VLS( PARENT, "qCmin", 1 ) + 1 );
double nCmax = log( VS( PARENT, "nCmax" ) + 1 );
double nCmin = log( VS( PARENT, "nCmin" ) + 1 );
double kCmax = log( VS( PARENT, "kCmax" ) + 1 );
double kCmin = log( VS( PARENT, "kCmin" ) + 1 );
double p2 = log( VL( "p2", 1 ) + 1 );
double q2 = log( VL( "q2", 1 ) + 1 );
double n2 = log( T - V( "t2ent" ) + 1 );
double k2 = log( V( "k2" ) + 1 );

// consider new industry values in the extremes
if ( V( "t2ent" ) >= T - 1 )
{
	pCmax = max( pCmax, p2 );
	pCmin = min( pCmin, p2 );
	qCmax = max( qCmax, q2 );
	qCmin = min( qCmin, q2 );
	nCmax = max( nCmax, n2 );
	nCmin = min( nCmin, n2 );
	kCmax = max( kCmax, k2 );
	kCmin = min( kCmin, k2 );
}

// normalize log price, quality, newness and complexity to [0.1, 0.9]
// zero competitiveness is avoided to prevent direct exit of worst in replicator
v[1] = ( pCmax > pCmin ) ? 0.1 + 0.8 * ( p2 - pCmin ) / ( pCmax - pCmin ) : 0.5;
v[2] = ( qCmax > qCmin ) ? 0.1 + 0.8 * ( q2 - qCmin ) / ( qCmax - qCmin ) : 0.5;
v[3] = ( nCmax > nCmin ) ? 0.1 + 0.8 * ( n2 - nCmin ) / ( nCmax - nCmin ) : 0.5;
v[4] = ( kCmax > kCmin ) ? 0.1 + 0.8 * ( k2 - kCmin ) / ( kCmax - kCmin ) : 0.5;

RESULT( ( delta1 * ( 1 - v[1] ) + delta2 * v[2] +
		  delta3 * ( 1 - v[3] ) + delta4 * v[4] ) )


EQUATION( "MC2" )
/*
Market conditions index for entry in consumer-good industry
*/
RESULT( log( max( VL( "NW2", 1 ), 0 ) + 1 ) - log( VL( "Deb2", 1 ) + 1 ) )


EQUATION( "entry2exit" )
/*
Perform entry and exit of firms in the consumer-good industry
All relevant aggregate variables in industry must be computed before existing
firms are deleted, so all active firms in period are considered
Updates 'F2', 'cEntry', 'cExit', 'entry2', 'exit2'
*/

UPDATE;											// ensure aggregates are computed

SUM( "_D2d" );									// save desired demand before chg

double MC2 = V( "MC2" );						// market conditions in industry
double MC2_1 = VL( "MC2", 1 );					// market conditions in industry
double omicron = VS( PARENT, "omicron" );		// entry sensitivity to mkt cond
double stick = VS( PARENT, "stick" );			// stickiness in number of firms
double x2inf = VS( PARENT, "x2inf" );			// entry lower distrib. support
double x2sup = VS( PARENT, "x2sup" );			// entry upper distrib. support
int F2 = V( "F2" );								// current number of firms
int F20 = V( "F20" );							// initial number of firms
int F2max = V( "F2max" );						// max firms in industry
int F2min = V( "F2min" );						// min firms in industry

v[1] = v[2] = v[3] = 0;							// firm entry-exit cost accum.

// firms exit, except the best one if all going to quit
j = CFUN( exit_firm2, & v[1], & v[2], & v[3], false );

V( "f2rescale" );								// redistribute exiting m.s.

// compute the potential number of entrants
v[4] = ( MC2_1 == 0 ) ? 0 : MC2 / MC2_1 - 1;	// change in market conditions

k = max( 0, ceil( F2 * ( ( 1 - omicron ) * uniform( x2inf, x2sup ) +
						 omicron * min( max( v[4], x2inf ), x2sup ) ) ) );

// apply return-to-the-average stickiness random shock to the number of entrants
if ( F2 - j + k > F20 )
	k -= min( stick * RND * ( ( double ) ( F2 - j ) / F20 - 1 ) * F20, k );

// ensure limits are enforced to the number of entrants
if ( F2 - j + k < F2min )
	k = F2min - F2 + j;

if ( F2 - j + k > F2max )
	k = F2max - F2 + j;

v[0] = k - j;									// net number of entrants

if ( k > 0 )
	v[1] += CFUN( entry_firm2, k, false );		// add entrant-firm objects

INCR( "F2", v[0] );								// update the number of firms
INCRS( PARENT, "cEntry", v[1] );				// account equity cost of entry
INCRS( PARENT, "cExit", v[2] );					// account exit credits
WRITE( "exit2", ( double ) j / F2 );
WRITE( "entry2", ( double ) k / F2 );
WRITE( "exit2fail", v[3] / F2 );

V( "f2rescale" );								// redistribute entrant m.s.

RESULT( v[0] )


EQUATION( "f2" )
/*
Expected wallet share of consumption-good industry
Computed using a replicator equation over the relative industry competitiveness
Because of entry/exit, wallet shares may add to more/less than one,
so 'fCrescale' must be used before 'f2' is used
Effective wallet share (f2e) may be significantly different because off
lack of demand for luxury goods (which are bought only once)
*/

v[1] = VS( PARENT, "fCmin" );					// minimum share to stay

// replicator equation multiplier
v[2] = 1 + VS( PARENT, "chiC" ) * ( V( "E2" ) / VS( PARENT, "EcAvg" ) - 1 );
v[0] = CURRENT * v[2];

// ensure emerging industry has minimum wallet share
if ( v[0] < v[1] && T < V( "t2ent" ) + VS( PARENT, "nC" ) )
	v[0] = max( v[1] * 2, v[1] * v[2] ) ;		// enforce not shrinking floor

RESULT( v[0] )


EQUATION( "f2e" )
/*
Effective wallet share of consumption-good industry
Considers lack of demand for luxury goods (which are bought only once)
*/
v[1] = VS( PARENT, "Sc" );
RESULT( v[1] > 0 ? V( "S2" ) / v[1] : V( "f2" ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A2" )
/*
Weighted-average (single-stage) labor productivity of consumption-good industry
Machine-level productivity is notional (single production stage) and
DO NOT directly represent firm final labor productivity
*/
V( "p2" );										// ensure m.s. are updated
RESULT( WHTAVE( "_A2", "_f2" ) )


EQUATION( "A2p" )
/*
Weighted-average (single-stage) potential labor productivity of
consumption-good industry
Machine-level productivity is notional (single production stage) and
DO NOT directly represent firm final labor productivity
*/
V( "p2" );										// ensure m.s. are updated
RESULT( WHTAVE( "_A2p", "_f2" ) )


EQUATION( "Bon2" )
/*
Total bonuses paid by firms in consumption-good industry
*/
V( "Tax2" );									// ensure bonuses are computed
RESULT( SUM( "_Bon2" ) )


EQUATION( "CI2" )
/*
Total canceled investment (in machine-number terms) in consumption-good industry
*/
RESULT( SUM( "_CI2" ) )


EQUATION( "D2e" )
/*
Demand expectation (in money terms) of firms in consumer-good industry
*/
RESULT( SUM( "_D2e" ) )


EQUATION( "Deb2" )
/*
Total debt of consumption-good industry
*/
RESULT( SUM( "_Deb2" ) )


EQUATION( "Div2" )
/*
Total dividends paid by firms in consumption-good industry
*/
V( "Tax2" );									// ensure dividends are computed
RESULT( SUM( "_Div2" ) )


EQUATION( "E2avg" )
/*
Weighted average competitiveness of firms in consumption-good industry
*/
RESULT( WHTAVEL( "_E2", "_f2", 1 ) )


EQUATION( "EI2" )
/*
Total expansion investment (in machine-number terms) in
consumption-good industry
*/
V( "CI2" );										// ensure cancellations acct'd
RESULT( SUM( "_EI2" ) )


EQUATION( "F2" )
/*
Number of firms in consumption-good industry
*/
RESULT( COUNT( "Firm2" ) )


EQUATION( "HH2" )
/*
Normalized Herfindahl-Hirschman index for consumption-good industry
*/
V( "p2" );										// ensure m.s. are updated
i = COUNT( "Firm2" );
RESULT( i > 1 ? max( 0, ( WHTAVE( "_f2", "_f2" ) - 1.0 / i ) / ( 1 - 1.0 / i ) ) : 1 )


EQUATION( "HP2" )
/*
Hymer-Pashigian index for consumption-good industry
*/

V( "p2" );										// ensure m.s. are updated

v[0] = 0;										// index accumulator
CYCLE( cur, "Firm2" )
	v[0] += fabs( VLS( cur, "_f2", 1 ) - VS( cur, "_f2" ) );// sum share changes

RESULT( v[0] )


EQUATION( "JO2" )
/*
Open job positions in consumption-good industry
*/
RESULT( SUM( "_JO2" ) )


EQUATION( "K2" )
/*
Total capital (in machine-number terms) accumulated by firms in
consumption-good industry
After new machine orders are delivered
*/
RESULT( SUM( "_K2" ) )


EQUATION( "K2d" )
/*
Total desired capital stock (in machine-number terms) of firms in
consumption-good industry
*/
RESULT( SUM( "_K2d" ) )


EQUATION( "L2" )
/*
Work force (labor) size employed by consumption-good industry
*/
RESULT( SUM( "_L2" ) )


EQUATION( "L2d" )
/*
Total labor demand from firms in consumption-good industry
*/
RESULT( SUM( "_L2d" ) )


EQUATION( "N2" )
/*
Inventories (unsold output units) of firms in consumption-good industry
*/
RESULT( SUM( "_N2" ) )


EQUATION( "NW2" )
/*
Total net wealth (free cash) of firms in consumption-good industry
*/
RESULT( SUM( "_NW2" ) )


EQUATION( "Pi2" )
/*
Total profits of consumer-good industry
*/
RESULT( SUM( "_Pi2" ) )


EQUATION( "Pi2rateAvg" )
/*
Weighted-average profit rate of firm in consumption-good industry
*/
V( "p2" );										// ensure m.s. are updated
RESULT( WHTAVE( "_Pi2rate", "_f2" ) )


EQUATION( "Q2" )
/*
Total planned output before labor/financial constraints in
consumption-good industry
*/
RESULT( SUM( "_Q2" ) )


EQUATION( "Q2d" )
/*
Total desired real output in consumption-good industry
*/
RESULT( SUM( "_Q2d" ) )


EQUATION( "Q2e" )
/*
Total effective output of firms in consumption-good industry
*/
RESULT( SUM( "_Q2e" ) )


EQUATION( "Q2p" )
/*
Potential production with current machines in consumption-good industry
*/
RESULT( SUM( "_Q2p" ) )


EQUATION( "Q2u" )
/*
Capacity utilization in consumption-good industry
*/
v[1] = VL( "K2", 1 ) * V( "m2" ) / V( "k2" );
RESULT( v[1] > 0 ? V( "Q2e" ) / v[1] : 0 )


EQUATION( "S2" )
/*
Total sales (in money terms) of consumption-good industry
*/
RESULT( SUM( "_S2" ) )


EQUATION( "SI2" )
/*
Total substitution investment (in machine-number terms) in
consumption-good industry
*/
V( "CI2" );										// ensure cancellations acct'd
RESULT( SUM( "_SI2" ) )


EQUATION( "Tax2" )
/*
Total taxes paid by firms in consumption-good industry
*/
RESULT( SUM( "_Tax2" ) )


EQUATION( "W2" )
/*
Total wages paid by all firms in consumption-good industry
*/
RESULT( SUM( "_W2" ) )


EQUATION( "age2avg" )
/*
Average age of firms in consumption-good industry
*/
RESULT( T - AVE( "_t2ent" ) )


EQUATION( "c2avg" )
/*
Planned average output unit cost in consumption-good industry
*/
v[1] = V( "Q2" );
RESULT( v[1] > 0 ? WHTAVE( "_c2", "_Q2" ) / v[1] : CURRENT )


EQUATION( "c2eAvg" )
/*
Effective average output unit cost in consumption-good industry
*/
v[1] = V( "Q2e" );
RESULT( v[1] > 0 ? WHTAVE( "_c2e", "_Q2e" ) / v[1] : CURRENT )


EQUATION( "cred2c" )
/*
Total credit constraint of firms in consumption-good industry
*/
RESULT( SUM( "_cred2c" ) )


EQUATION( "dN2nom" )
/*
Change in total inventories (in money terms) in consumption-good industry
*/
RESULT( SUM( "_dN2nom" ) )


EQUATION( "f2critChg" )
/*
Check if critical threshold for post-change firms in consumption-good industry
was met
Three criteria: 1. minimum entry holding period
			    2. entrants jointly acquire a certain market share
				3. entrant in a post-change industry
*/

v[1] = VS( PARENT, "TregChg" );					// time of regulatory change

if ( ( T >= v[1] + V( "ent2HldPer" ) && V( "f2posChg" ) >= V( "f2trdChg" ) ) ||
	 V( "t2ent" ) >= v[1] )
{
	v[0] = 1;									// threshold was met
	PARAMETER;									// turn into parameter
}
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "f2posChg" )
/*
Joint market share hold by all firms of post-change type in consumption-good
industry
*/
V( "p2" );										// ensure m.s. are updated
v[0] = SUM_CND( "_f2", "_post2chg", "!=", 0 )
RESULT( ! is_nan( v[0] ) ? v[0] : 0 )


EQUATION( "fires2" )
/*
Number of workers fired by firms in consumer-good industry
*/
RESULT( SUM( "_fires2" ) )


EQUATION( "hires2" )
/*
Number of workers hired by firms in consumer-good industry
*/
RESULT( SUM( "_hires2" ) )


EQUATION( "l2avg" )
/*
Weighted average unfilled demand in consumption-good industry
*/
v[1] = V( "Q2e" );
RESULT( v[1] > 0 ? WHTAVE( "_l2", "_Q2e" ) / v[1] : CURRENT )


EQUATION( "l2max" )
/*
Maximum unfilled demand of a firm in consumption-good industry
*/
RESULT( MAX( "_l2" ) )


EQUATION( "l2min" )
/*
Minimum unfilled demand of a firm in consumption-good industry
*/
RESULT( MIN( "_l2" ) )


EQUATION( "mu2avg" )
/*
Weighted-average mark-up of consumption-good industry
*/
V( "p2" );										// ensure m.s. are updated
RESULT( WHTAVE( "_mu2", "_f2" ) )


EQUATION( "noWrk2" )
/*
Share of operating firms with no worker hired in consumption-good industry
*/

i = j = 0;
CYCLE( cur, "Firm2" )
{
	if ( VS( cur, "_L2" ) == 0 && VS( cur, "_life2cycle" ) > 0 )
		++i;
	++j;
}

RESULT( j > 0 ? ( double ) i / j : 0 )


EQUATION( "old2vint" )
/*
Oldest vintage (ID) in use in period by any firm in consumption-good industry
*/
RESULT( MIN( "_old2vint" ) )


EQUATION( "p2" )
/*
Weighted-average price charged by consumption-good industry
Ensures market shares are properly rescaled after production is done
and replicator is applied to define current market shares
Also prevents multiple runs of the rescaling algorithm
*/
V( "Q2e" );										// ensure production is done
V( "f2rescale" );								// ensure m.s. are updated
RESULT( WHTAVE( "_p2", "_f2" ) )


EQUATION( "p2max" )
/*
Maximum price of a firm in consumption-good industry
*/
RESULT( MAX( "_p2" ) )


EQUATION( "p2min" )
/*
Minimum price of a firm in consumption-good industry
*/
RESULT( MIN( "_p2" ) )


EQUATION( "q2" )
/*
Weighted-average quality of consumption-good industry
*/
V( "p2" );										// ensure m.s. are updated
RESULT( WHTAVE( "_q2", "_f2" ) )


EQUATION( "q2max" )
/*
Maximum product quality of a firm in consumption-good industry
*/
RESULT( MAX( "_q2" ) )


EQUATION( "q2min" )
/*
Minimum product quality of a firm in consumption-good industry
*/
RESULT( MIN( "_q2" ) )


EQUATION( "quits2" )
/*
Number of workers quitting (not fired) from firms in consumer-good industry
Also updated in 'quits'
*/
RESULT( SUM( "_quits2" ) )


EQUATION( "retires2" )
/*
Number of workers retiring (not fired) from firms in consumer-good industry
*/
RESULT( SUM( "_retires2" ) )


EQUATION( "s2avg" )
/*
Weighted average worker skills of firms in consumption-good industry
*/
v[1] = WHTAVE( "_s2avg", "_L2" );
RESULT( v[1] > 0 ? v[1] / V( "L2" ) : CURRENT )


EQUATION( "w2avg" )
/*
Weighted average wage paid by firms in consumption-good industry
*/
v[1] = WHTAVE( "_w2avg", "_L2" );
RESULT( v[1] > 0 ? v[1] / V( "L2" ) : CURRENT )


EQUATION( "w2oAvg" )
/*
Weighted average wage offered by firms in consumption-good industry
*/
v[1] = WHTAVE( "_w2o", "_L2d" );
RESULT( v[1] > 0 ? v[1] / V( "L2d" ) : CURRENT )


EQUATION( "w2oMax" )
/*
Highest wage offered by a firm in consumption-good industry
*/
RESULT( MAX( "_w2o" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "f2rescale" )
/*
Rescale market shares in consumption-good industry to ensure adding to 1
To be called after market shares are changed in '_f2' and 'entry2exit'
*/

v[1] = SUM( "_f2" );							// add-up market shares

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator

if ( v[1] > 0 )									// production ok?
	CYCLE( cur, "Firm2" )						// rescale to add-up to 1
	{
		v[0] += v[2] = VS( cur, "_f2" ) / v[1];	// rescaled market share
		WRITES( cur, "_f2", v[2] );				// save updated share
	}
else
{
	v[2] = 1 / COUNT( "Firm2" );				// firm fair share

	CYCLE( cur, "Firm2" )						// rescale to add-up to 1
	{
		v[0] += v[2];
		WRITES( cur, "_f2", v[2] );
	}
}

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "D2a", "" )
/*
Demand (in money terms) effectively allocated to consumption-good industry
Updated in 'DcBas', 'DcLux'
*/

EQUATION_DUMMY( "D2d", "" )
/*
Demand (in money terms) desired from consumption-good industry
Updated in 'DcBas', 'DcLux'
*/

EQUATION_DUMMY( "entry2", "" )
/*
Rate of entering firms in consumption-good industry
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "exit2", "" )
/*
Rate of exiting firms in consumption-good industry
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "exit2fail", "" )
/*
Rate of bankrupt firms in consumption-good sector
Updated in 'entry2exit'
*/
