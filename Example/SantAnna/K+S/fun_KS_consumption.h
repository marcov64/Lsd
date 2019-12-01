/******************************************************************************

	CONSUMER-GOODS MARKET OBJECT EQUATIONS
	--------------------------------------

	Equations that are specific to the consumer-goods market objects in the 
	K+S LSD model are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "D2" )
/*
Demand fulfilled by firms in consumption-good sector
Update '_D2'
*/

k = V( "F2" );									// number of firms
v[1] = V( "D2d" );								// real desired demand

// create and fill temporary share and supply vectors & initialize firm demand
dblVecT f2( k ), sup2( k ), dem2( k );

j = 0;
CYCLE( cur, "Firm2" )
{
	f2[ j ] = VS( cur, "_f2" );					// firm market share
	sup2[ j ] = VS( cur, "_Q2e" ) + VLS( cur, "_N", 1 );// firm available supply
	WRITES( cur, "_D2", 0 );					// demand fulfilled accumulator
	++j;
}

// cycle through firms until all demand is allocated or no more product to sell
v[0] = 0;										// fulfilled demand accumulator
while ( v[1] > 1 )
{
	v[2] = j = 0;								// shares yet unallocated
	CYCLE( cur, "Firm2" )
	{
		if ( f2[ j ] > 0 )						// firm has demand to supply
		{	
			if ( sup2[ j ] > 0 )				// product to supply?
			{
				dem2[ j ] = v[1] * f2[ j ];		// firm demand allocation
			
				if ( dem2[ j ] <= sup2[ j ] )	// can supply all demanded?
				{
					INCRS( cur, "_D2", dem2[ j ] );// supply all demanded
					v[0] += dem2[ j ];			// accumulate to total demand
					v[1] -= dem2[ j ];			// discount from desired demand
					v[2] += f2[ j ];			// save share yet to allocate
					sup2[ j ] -= dem2[ j ];		// make supplied unavailable
				}
				else
				{
					INCRS( cur, "_D2", sup2[ j ] );// supply all available
					v[0] += sup2[ j ];			// accumulate to total demand
					v[1] -= sup2[ j ];			// discount from desired demand
					f2[ j ] = sup2[ j ] = 0;	// nothing else to supply
				}
			}
			else
				f2[ j ] = 0;					// nothing else to supply
		}

		++j;
	}
	
	if ( v[2] > 0 )								// unallocated shares remaining?
		for ( j = 0; j < k; ++j )
			f2[ j ] /= v[2];					// rescale share remaining firms
	else
		break;									// nothing else to supply
}

RESULT( v[0] )


EQUATION( "MC2" )
/*
Market conditions index for entry in consumer-good sector
*/
RESULT( log( max( VL( "NW2", 1 ), 0 ) + 1 ) - log( VL( "Deb2", 1 ) + 1 ) )


EQUATION( "entry2exit" )
/*
Perform entry and exit of firms in the consumer-good sector
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
*/

VS( PARENT, "GDP" );							// ensure markets are closed
SUM( "_D2d" );									// desired demand before chg
SUMS( LABSUPL1, "_B" );							// register bonuses

V( "A2" ); V( "A2p" ); V( "B2" ); V( "CI" ); 	// ensure aggregates in industry
V( "CPI" ); V( "D2" ); V( "D2d" ); V( "D2e" );  // computed before entry/exit
V( "Deb2" ); V( "Div2" ); V( "Eavg" ); V( "EI" ); V( "I" ); V( "Inom" ); 
V( "JO2" ); V( "K" ); V( "Kd" ); V( "L2" ); V( "L2d" ); V( "N" ); V( "NW2" ); 
V( "Pi2" ); V( "Pi2rateAvg" ); V( "Q2" ); V( "Q2d" ); V( "Q2e" ); V( "Q2p" ); 
V( "Q2u" ); V( "S2" ); V( "SI" ); V( "Tax2" ); V( "W2" ); V( "c2" ); V( "c2e" ); 
V( "dCPI" ); V( "dCPIb" ); V( "dNnom" ); V( "f2critChg" ); V( "f2posChg" ); 
V( "fires2" ); V( "hires2" ); V( "l2avg" ); V( "l2max" ); V( "l2min" ); 
V( "oldVint" ); V( "p2avg" ); V( "p2max" ); V( "p2min" ); V( "q2avg" ); 
V( "q2max" ); V( "q2min" ); V( "quits2" ); V( "retires2" ); V( "sV2avg" ); 
V( "w2avg" ); V( "w2oAvg" ); V( "w2oMax" ); V( "w2realAvg" );

VS( SECSTAL1, "A2posChg" ); VS( SECSTAL1, "A2preChg" ); VS( SECSTAL1, "A2sd" ); 
VS( SECSTAL1, "A2sdPosChg" ); VS( SECSTAL1, "A2sdPreChg" ); 
VS( SECSTAL1, "B2payers" ); VS( SECSTAL1, "HH2" ); VS( SECSTAL1, "HP2" ); 
VS( SECSTAL1, "age2avg" ); VS( SECSTAL1, "cred2c" ); VS( SECSTAL1, "dN" ); 
VS( SECSTAL1, "mu2avg" ); VS( SECSTAL1, "noWrk2" ); VS( SECSTAL1, "q2posChg" ); 
VS( SECSTAL1, "q2preChg" ); VS( SECSTAL1, "s2avg" ); 
VS( LABSTAL1, "L2ent" ); VS( LABSTAL1, "L2exit" ); VS( LABSTAL1, "L2larg" ); 
VS( LABSTAL1, "L2v" ); VS( LABSTAL1, "w2avgLarg" ); VS( LABSTAL1, "w2oMin" ); 
VS( LABSTAL1, "w2realPosChg" ); VS( LABSTAL1, "w2realPreChg" ); 
VS( LABSTAL1, "wAvgReal" ); 

double MC2 = V( "MC2" );						// market conditions in sector 2
double MC2_1 = VL( "MC2", 1 );					// market conditions in sector 2
double NW20u = V( "NW20" ) * VS( CAPSECL1, "PPI" ) / VS( CAPSECL1, "PPI0" );
												// minimum wealth in sector 2
double f2min = V( "f2min" );					// min market share in sector 2
double n2 = V( "n2" );							// market participation period
double omicron = VS( PARENT, "omicron" );		// entry sensitivity to mkt cond
double stick = VS( PARENT, "stick" );			// stickiness in number of firms
double x2inf = VS( PARENT, "x2inf" );			// entry lower distrib. support
double x2sup = VS( PARENT, "x2sup" );			// entry upper distrib. support
int F2 = V( "F2" );								// current number of firms
int F20 = V( "F20" );							// initial number of firms
int F2max = V( "F2max" );						// max firms in sector 2
int F2min = V( "F2min" );						// min firms in sector 2

h = F2;											// initial number of firms
vector < bool > quit( h, false );				// vector of firms' quit status

// mark bankrupt and market-share-irrelevant incumbent firms to quit the market
v[1] = v[2] = v[3] = 0;							// accumulators
v[4] = k = 0;									// best firm registers
i = 0;											// firm counter
CYCLE( cur, "Firm2" )
{
	if ( VS( cur, "_life2cycle" ) > 1 )
	{
		for ( v[5] = j = 0; j < n2; ++j )
			v[5] += VLS( cur, "_f2", j );		// n2 periods market share
		
		if ( VS( cur, "_NW2" ) < 0 || v[5] < f2min )
		{
			quit[ i ] = true;					// mark for likely exit
			--h;								// one less firm
			
			if ( v[5] > v[4] )					// best firm so far?
			{
				k = i;							// save firm index
				v[4] = v[5];					// and market share
			}
		}
	}
	
	++i;
}	

// quit candidate firms exit, except the best one if all going to quit
i = j = 0;										// firm counters
CYCLE_SAFE( cur, "Firm2" )
{
	if ( quit[ i ] )
	{
		if ( h > 0 || i != k )					// firm must exit?
		{
			// account liquidation credit due to public, if any
			v[3] += exit_firm2( cur, & v[1] );	// delete object and liq. val.
			
			++j;								// count exits
		}
		else
			if ( h == 0 && i == k )				// best firm must get new equity
			{
				// new equity required
				v[6] = NW20u + VS( cur, "_Deb2" ) - VS( cur, "_NW2" );
				v[2] += v[6];					// accumulate "entry" equity cost
				
				WRITES( cur, "_Deb2", 0 );		// reset debt
				INCRS( cur, "_NW2", v[6] );		// add new equity
			}
	}

	++i;
}

// compute the potential number of entrants
v[7] = ( MC2_1 == 0 ) ? 0 : MC2 / MC2_1 - 1;// change in market conditions

k = max( 0, ceil( F2 * ( ( 1 - omicron ) * uniform( x2inf, x2sup ) + 
						 omicron * min( max( v[7], x2inf ), x2sup ) ) ) );
				 
// apply return-to-the-average stickiness random shock to the number of entrants
k -= min( RND * stick * ( ( double ) ( F2 - j ) / F20 - 1 ) * F20, k );

// ensure limits are enforced to the number of entrants
if ( F2 - j + k < F2min )
	k = F2min - F2 + j;

if ( F2 + k > F2max )
	k = F2max - F2 + j;

v[0] = k - j;									// net number of entrants
v[2] += entry_firm2( p, k, false );				// add entrant-firm objects

INCR( "F2", v[0] );								// update the number of firms
INCR( "fires2", v[1] );							// update fires
INCRS( PARENT, "cEntry", v[2] );				// account equity cost of entry
INCRS( PARENT, "cExit", v[3] );					// account exit credits
WRITE( "exit2", ( double ) j / F2 );
WRITE( "entry2", ( double ) k / F2 );

V( "f2rescale" );								// redistribute exiting m.s.
V( "firm2maps" );								// update firm mapping vectors

RESULT( v[0] )


EQUATION( "hires2" )
/*
Number of workers hired by firms in consumption-good sector
Process required hiring using the appropriate rule
*/

VS( CAPSECL1, "hires1" );						// ensure sector 1 is done

v[1] = VS( PARENT, "flagHeterWage" );			// heterogeneous wages flag
h = ( v[1] == 0 ) ? 0 : VS( PARENT, "flagHireSeq" );// firm hiring order
v[2] = VS( LABSUPL1, "Lscale" );				// labor scaling

// create pointer and sort wage offers list
woLisT *offers = & V_EXTS( PARENT, country, firm2wo );
order_offers( h, offers );

// firms hire employees according to the selected hiring order
for ( i = 0, itw = offers->begin( ); itw != offers->end( ); ++itw )
{
	v[3] = VS( itw->firm, "_postChg" ) ? VS( PARENT, "flagWageOfferChg" ) : 
										 VS( PARENT, "flagWageOffer" );

	if ( v[1] == 0 || v[3] == 0 )				// avoid re-sorting the applics.
	{
		// sort firm's candidate list according to the defined strategy
		int hOrder = VS( itw->firm, "_postChg" ) ? VS( PARENT, "flagHireOrder2Chg" ) : 
												   VS( PARENT, "flagHireOrder2" );
		order_applications( hOrder, & V_EXTS( itw->firm, firm2, appl ) );
	}
	
	// hire the ordered applications until queue is exhausted
	j = ceil( VS( itw->firm, "_JO2" ) / v[2] );	// firm's jobs open (scaled)
	h = 0;										// firm hiring counter
	v[4] = DBL_MAX;								// minimum wage requested
	cur = NULL;									// pointer to worker asking it

	while ( j > 0 && EXEC_EXTS( itw->firm, firm2, appl, size ) > 0 )
	{
		// get the candidate worker object element and a pointer to it
		const application candidate = EXEC_EXTS( itw->firm, firm2, appl, front );

		// candidate not yet hired in this period and offered wage ok?
		v[5] = VS( candidate.wrk, "_employed" );
		if ( ! ( v[5] && VS( candidate.wrk, "_Te" ) == 0 ) )
		{
			if ( ROUND( candidate.w, itw->offer ) <= itw->offer )
			{
				// already employed? First quit current job
				if ( v[5] )
					quit_worker( candidate.wrk );

				// flag hiring and set wage, employer and vintage to be used by worker
				hire_worker( candidate.wrk, 2, itw->firm, itw->offer );
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
		EXEC_EXTS( itw->firm, firm2, appl, pop_front );
	}
	
	// try to hire at least one worker, at any wage
	if ( j > 0 && h == 0 && cur != NULL )		// none hired but someone avail?
	{
		if ( VS( cur, "_employed" ) )			// quit job if needed
			quit_worker( cur );
		
		hire_worker( cur, 2, itw->firm, v[4] );	// pay requested wage
		++h;
	}	
	
	WRITES( itw->firm, "_hires2", h * v[2] );	// update hires count for firm
	EXEC_EXTS( itw->firm, firm2, appl, clear );	// clear application queue
}

offers->clear( );								// clear offers set
RECALC( "quits2" );								// force update of industry quits

RESULT( i * v[2] )


EQUATION( "sV2avg" )
/*
Average vintage skills for all technologies/vintages in consumption-good sector
Also updates the map of vintage productivity and skill
*/

h = T0( VL( "oldVint", 1 ) );					// time of oldest vintage
v[1] = VS( LABSUPL1, "Lscale" );				// workers to objects ratio

vintMapT *vint = & EXTS( PARENT, country ).vintProd;// vintage map
vintMapT::iterator itm;

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
v[0] = v[2] = 0;
CYCLES( LABSUPL1, cur, "Worker" )				// scan all workers
{
	cur1 = HOOKS( cur, VWRK );					// pointer to vintage bridge
	if ( cur1 != NULL )							// discard disalloc. unempl. s.1
	{
		i = VS( cur1->up, "_IDvint" );			// vintage ID
		v[2] = VLS( cur, "_sV", 1 ) * v[1];		// last skills (weighted)
		EXTS( PARENT, country ).vintProd[ i ].sVavg += v[2];
		EXTS( PARENT, country ).vintProd[ i ].workers += v[1];
		v[0] += v[2];
		v[2] += v[1];
	}
}

// update public skills map
for( itm = vint->begin( ); itm != vint->end( ); ++itm )
{
	itm->second.sVp += VS( LABSUPL1, "sigma" ) * 
					   ( itm->second.sVavgLag - itm->second.sVp );
	if ( itm->second.workers != 0 )
		itm->second.sVavg /= itm->second.workers;
	else
		itm->second.sVavg = itm->second.sVp;
}

RESULT( v[2] > 0 ? v[0] / v[2] : 0 )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A2" )
/*
Productivity of consumption-good sector
*/
v[1] = V( "Q2e" );
RESULT( v[1] > 0 ? WHTAVE( "_A2", "_Q2e" ) / v[1] : AVE( "_A2" ) )


EQUATION( "A2p" )
/*
Potential notional productivity of consumption-good sector
*/
v[1] = V( "Q2" );
RESULT( v[1] > 0 ? WHTAVE( "_A2p", "_Q2" ) / v[1] : CURRENT )


EQUATION( "B2" )
/*
Total bonuses paid by firms in consumption-good sector
*/
V( "Tax2" );									// ensure bonuses are computed
RESULT( SUM( "_B2" ) )


EQUATION( "CI" )
/*
Total canceled investment in consumption-good sector
*/
RESULT( SUM( "_CI" ) )	


EQUATION( "CPI" )
/*
Consumer price index
*/
V( "f2rescale" );								// ensure m.s. computed/rescaled
RESULT( WHTAVE( "_p2", "_f2" ) )


EQUATION( "D2d" )
/*
Desired demand for consumption-good sector
*/
RESULT( ( VS( PARENT, "C" ) + VS( PARENT, "G" ) ) / V( "CPI" ) )


EQUATION( "D2e" )
/*
Demand expectation of firms in consumer-good sector
*/
RESULT( SUM( "_D2e" ) )


EQUATION( "Deb2" )
/*
Total debt of consumption-good sector
*/
RESULT( SUM( "_Deb2" ) )


EQUATION( "Div2" )
/*
Total dividends paid by firms in consumption-good sector
*/
V( "Tax2" );									// ensure dividends are computed
RESULT( SUM( "_Div2" ) )


EQUATION( "Eavg" )
/*
Weighted average competitiveness of firms in consumption-good sector
*/

v[0] = 0;
CYCLE( cur, "Firm2" )
	if ( VS( cur, "_life2cycle" ) > 0 )			// consider only non-entrants
		v[0] += VS( cur, "_E" ) * VLS( cur, "_f2", 1 );
												// compute weighted average
RESULT( v[0] )


EQUATION( "EI" )
/*
Total expansion investment in consumption-good sector
*/
V( "CI" );										// ensure cancellations acct'd 
RESULT( SUM( "_EI" ) )	


EQUATION( "F2" )
/*
Number of firms in consumption-good sector
*/
RESULT( COUNT( "Firm2" ) )


EQUATION( "I" )
/*
Total (real) investment in consumption-good sector
*/
RESULT( V( "SI" ) + V( "EI" ) )	


EQUATION( "Id" )
/*
Total desired investment in terms of output capacity (real terms)
Don't recompute 'SI'/'EI' at this stage, to wait for order cancellations
*/

v[0] = 0;
CYCLE( cur, "Firm2" )
	v[0] += VS( cur, "_SI" ) + VS( cur, "_EI" );
		
RESULT( v[0] )	


EQUATION( "Inom" )
/*
Aggregated investment (nominal/currency terms)
*/
RESULT( VS( CAPSECL1, "S1" ) )


EQUATION( "JO2" )
/*
Open job positions in consumption-good sector
*/
RESULT( SUM( "_JO2" ) )


EQUATION( "K" )
/*
Total capital accumulated by firms in consumption-good sector
After new machine orders are delivered
*/
RESULT( SUM( "_K" ) )


EQUATION( "Kd" )
/*
Total desired capital stock of firms in consumption-good sector
*/
RESULT( SUM( "_Kd" ) )


EQUATION( "L2" )
/*
Work force (labor) size employed by consumption-good sector
*/
RESULT( SUM( "_L2" ) )


EQUATION( "L2d" )
/*
Total labor demand from firms in consumption-good sector
Includes R&D labor
*/
RESULT( SUM( "_L2d" ) )


EQUATION( "N" )
/*
Total inventories (real terms)
*/
RESULT( SUM( "_N" ) )


EQUATION( "NW2" )
/*
Total net wealth (free cash) of firms in consumption-good sector
*/
RESULT( SUM( "_NW2" ) )


EQUATION( "Pi2" )
/*
Total profits of consumer-good sector
*/
RESULT( SUM( "_Pi2" ) )


EQUATION( "Pi2rateAvg" )
/*
Average (weighted by market share) profit rate of firm in consumption-good 
sector
*/

v[0] = 0;										// accumulator
CYCLE( cur, "Firm2" )
	if ( VS( cur, "_life2cycle" ) > 0 )			// consider only non-entrants
		v[0] += VS( cur, "_Pi2rate" ) * VS( cur, "_f2" );
												// compute weighted average
RESULT( v[0] )


EQUATION( "Q2" )
/*
Total planned output before labor/financial constraints in consumption-good sector
*/
RESULT( SUM( "_Q2" ) )


EQUATION( "Q2d" )
/*
Total desired real output of consumption-good sector
*/
RESULT( SUM( "_Q2d" ) )


EQUATION( "Q2e" )
/*
Total effective output of firms in consumption-good sector
*/
RESULT( SUM( "_Q2e" ) )


EQUATION( "Q2p" )
/*
Potential production with current machines in consumption-good sector
*/
RESULT( SUM( "_Q2p" ) )


EQUATION( "Q2u" )
/*
Capacity utilization of consumption-good sector
*/
v[1] = V( "Q2p" );
RESULT( v[1] > 0 ? V( "Q2e" ) / v[1] : 0 )


EQUATION( "S2" )
/*
Total sales of consumption-good sector
*/
RESULT( SUM( "_S2" ) )


EQUATION( "SI" )
/*
Total substitution investment in consumption-good sector
*/
V( "CI" );										// ensure cancellations acct'd 
RESULT( SUM( "_SI" ) )	


EQUATION( "Tax2" )
/*
Total taxes paid by firms in consumption-good sector
*/
RESULT( SUM( "_Tax2" ) )


EQUATION( "W2" )
/*
Total wages paid by all firms in sector 2
*/
RESULT( SUM( "_W2" ) )


EQUATION( "c2" )
/*
Planned average unit cost in consumption-good sector
*/
v[1] = V( "Q2" );
RESULT( v[1] > 0 ? WHTAVE( "_c2", "_Q2" ) / v[1] : CURRENT )


EQUATION( "c2e" )
/*
Planned average unit cost in consumption-good sector
*/
v[1] = V( "Q2e" );
RESULT( v[1] > 0 ? WHTAVE( "_c2e", "_Q2e" ) / v[1] : CURRENT )


EQUATION( "dCPI" )
/*
Consumer price index inflation (change) rate
*/
RESULT( V( "CPI" ) / VL( "CPI", 1 ) - 1 )


EQUATION( "dCPIb" )
/*
Consumer price index inflation (change) rate
*/
RESULT( mov_avg_bound( p, "CPI", VS( PARENT, "mLim" ) ) )


EQUATION( "dNnom" )
/*
Change in total nominal inventories (currency terms)
*/
RESULT( SUM( "_dNnom" ) )


EQUATION( "f2critChg" )
/*
Check if critical threshold for post-change firms in consumption-good sector 
was met
Two criteria: 1. minimum entry holding period
			  2. entrants jointly acquire a certain market share
*/

if ( t >= VS( PARENT, "TregChg" ) + V( "ent2HldPer" ) && 
	 V( "f2posChg" ) >= V( "f2trdChg" ) )
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
sector
*/

V( "CPI" );										// ensure m.s. are updated

v[0] = 0;										// market share accumulator
CYCLE( cur, "Firm2" )
	if ( VS( cur, "_postChg" ) )				// is a post-change firm?
		v[0] += VS( cur, "_f2" );

RESULT( v[0] )


EQUATION( "fires2" )
/*
Workers fired in consumption-good sector
*/
RESULT( SUM( "_fires2" ) )


EQUATION( "l2avg" )
/*
Weighted average unfilled demand in consumption-good sector
*/
v[1] = V( "Q2e" );
RESULT( v[1] > 0 ? WHTAVE( "_l2", "_Q2e" ) / v[1] : CURRENT )


EQUATION( "l2max" )
/*
Maximum unfilled demand of a firm in consumption-good sector
*/
RESULT( MAX( "_l2" ) )


EQUATION( "l2min" )
/*
Minimum unfilled demand of a firm in consumption-good sector
*/
RESULT( MIN( "_l2" ) )


EQUATION( "oldVint" )
/*
Oldest vintage (ID) in use in period by any firm in consumption-good sector
*/
RESULT( MIN( "_oldVint" ) )


EQUATION( "p2avg" )
/*
Weighted average price charged in consumption-good sector
*/
v[1] = V( "Q2e" );
RESULT( v[1] > 0 ? WHTAVE( "_p2", "_Q2e" ) / v[1] : CURRENT )


EQUATION( "p2max" )
/*
Maximum price of a firm in consumption-good sector
*/
RESULT( MAX( "_p2" ) )


EQUATION( "p2min" )
/*
Minimum price of a firm in consumption-good sector
*/
RESULT( MIN( "_p2" ) )


EQUATION( "q2avg" )
/*
Weighted average product quality in consumer-good sector
*/
v[1] = V( "Q2e" );
RESULT( v[1] > 0 ? WHTAVE( "_q2", "_Q2e" ) / v[1] : CURRENT  )


EQUATION( "q2max" )
/*
Maximum product quality of a firm in consumption-good sector
*/
RESULT( MAX( "_q2" ) )


EQUATION( "q2min" )
/*
Minimum product quality of a firm in consumption-good sector
*/
RESULT( MIN( "_q2" ) )


EQUATION( "quits2" )
/*
Workers quitting (not fired) in consumption-good sector
Also updated in 'quits'
*/
RESULT( SUM( "_quits2" ) )


EQUATION( "retires2" )
/*
Workers retiring (not fired) in consumption-good sector
*/
RESULT( SUM( "_retires2" ) )


EQUATION( "w2avg" )
/*
Weighted average wage paid by firms in consumption-good sector
*/
v[1] = WHTAVE( "_w2avg", "_L2" );
RESULT( v[1] > 0 ? v[1] / V( "L2" ) : CURRENT )


EQUATION( "w2oAvg" )
/*
Weighted average wage offered by firms in consumption-good sector
*/
v[1] = WHTAVE( "_w2o", "_L2d" );
RESULT( v[1] > 0 ? v[1] / V( "L2d" ) : CURRENT )


EQUATION( "w2oMax" )
/*
Highest wage offered by a firm in consumption-good sector
*/
RESULT( MAX( "_w2o" ) )


EQUATION( "w2realAvg" )
/*
Weighted real average wage paid by firms in consumption-good sector
*/
v[1] = WHTAVE( "_w2realAvg", "_L2" );
RESULT( v[1] > 0 ? v[1] / V( "L2" ) : CURRENT )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "f2rescale" )
/*
Rescale market shares in consumption-good sector to ensure adding to 1
To be called after market shares are changed in '_f2' and 'entry2exit'
*/

v[1] = SUM( "_f2" );							// add-up market shares

if ( ROUND( v[1], 1 ) == 1.0 )					// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator

if ( v[1] > 0 )									// production ok?
	CYCLE( cur, "Firm2" )						// rescale to add-up to 1
	{
		v[0] += v[2] = VS( cur, "_f2" ) / v[1];	// rescaled market share
		WRITES( cur, "_f2", v[2] );				// save updated m.s.
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


EQUATION( "firm2maps" )
/*
Updates the static maps of firms in consumption-good sector
Also updates the table of log transformed market share cumulative weights, used by 
workers when choosing where to queue for jobs (bigger firms get more applicants)
Market shares are rescaled based on the minimum market share and log transformed
Only to be called if firm objects in sector 2 are created or destroyed
*/

// clear vectors
EXEC_EXTS( PARENT, country, firm2map, clear );
EXEC_EXTS( PARENT, country, firm2ptr, clear );
EXEC_EXTS( PARENT, country, firm2wgtd, clear );

v[1] = V( "f2min" );							// market exit threshold
v[2] = max( 1 / VL( "F2", 1 ), 2 * v[1] );		// entrant bounded fair share

i = 0;											// firm index in vector
v[3] = 0;										// cumulative market share
CYCLE( cur, "Firm2" )							// do for all firms in sector 2
{
	switch ( ( int ) VS( cur, "_life2cycle" ) )
	{
		case 0:									// just entered, no share
			v[4] = 0;
			break;
			
		case 1:									// entrant starting operation
			// fair share lower-bounded (twice) above exit threshold
			v[4] = v[2];
			break;
			
		default:								// operating entrant/incumbent
			v[4] = max( VLS( cur, "_f2", 1 ), v[2] );// too small floor
	}
	
	// log transform market share
	v[3] += v[5] = max( log( v[4] / v[1] + 1 ), 0 );		
			   
	EXEC_EXTS( PARENT, country, firm2wgtd, push_back, v[5] );
	EXEC_EXTS( PARENT, country, firm2ptr, push_back, cur );// pointer to firm
	EXEC_EXTS( PARENT, country, firm2map, insert, // save in firm's map
			   firmPairT( ( int ) VS( cur, "_ID2" ), cur ) );
	
	++i;
}

// rescale the transformed shares to 1 and accumulate them
for ( v[6] = 0, j = 0; j < i; ++j )
{
	v[6] += V_EXTS( PARENT, country, firm2wgtd [ j ] ) / v[3];
	WRITE_EXTS( PARENT, country, firm2wgtd[ j ], min( v[6], 1 ) );
}

RESULT( i )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "entry2", "entry2exit" )
/*
Rate of entering firms in consumption-good sector
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "exit2", "entry2exit" )
/*
Rate of exiting firms in consumption-good sector
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "exit2fail", "entry2exit" )
/*
Rate of bankrupt firms in consumption-good sector
Updated in 'entry2exit'
*/
