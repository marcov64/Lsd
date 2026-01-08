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
Total demand (in product units) fulfilled by firms in consumption-good sector
Also allocates demand to firms
Update 'Gc', '_C', '_D2l', '_D2x', '_l2'
*/

v[1] = V( "D2xD" ) * V( "CPI" );				// committed $ exports
v[2] = VS( COUNTRL1, "Cd" ) - VS( COUNTRL1, "McD" );// local desired $ demand

// create and fill temporary share and supply vectors & initialize firm demand
k = V( "F2" );									// number of firms
dblVecT f2( k ), p2( k ), sup2( k );

j = 0;
CYCLE( cur, "Firm2" )
{
	sup2[ j ] = VS( cur, "_Q2e" ) + VLS( cur, "_N", 1 );// firm available supply
	f2[ j ] = VS( cur, "_f2" );					// firm market share
	p2[ j ] = VS( cur, "_p2" );					// firm price
	WRITES( cur, "_D2l", 0 );					// local demand fulf. accum.
	WRITES( cur, "_D2x", 0 );					// exports fulfilled accumulator
	WRITES( cur, "_l2", 0 );					// assume no unsatisfied demand
	++j;
}

// cycle through firms until all demand is allocated or no more product to sell
v[0] = i = 0;									// fulfilled demand accumulator
while ( v[1] > 0.01 || v[2] > 0.01 )			// while unfilled demand exists
{
	v[3] = v[1];								// remaining unallocated $ export
	v[4] = v[2];								// remaining unallocated $ demand

	v[5] = j = 0;								// shares yet unallocated
	CYCLE( cur, "Firm2" )
	{
		if ( f2[ j ] > 0 )						// firm has share to supply?
		{
			if ( sup2[ j ] > 0 )				// product to supply?
			{
				if ( v[3] > 0.01 )				// committed foreign demand?
				{
					v[6] = v[1] * f2[ j ];		// firm $ exports allocation
					v[7] = v[6] / p2[ j ];		// firm # exports allocation

					if ( v[7] <= sup2[ j ] )	// can supply all exports?
					{
						INCRS( cur, "_D2x", v[7] );// supply all exports
						v[0] += v[7];			// accumulate to total # demand
						v[3] -= v[6];			// discount from $ exports
						v[5] += f2[ j ];		// save share yet to allocate
						sup2[ j ] -= v[7];		// make supplied # unavailable
					}
					else
					{
						INCRS( cur, "_D2x", sup2[ j ] );// supply # available
						v[0] += sup2[ j ];		// accumulate to total # demand
						v[3] -= sup2[ j ] * p2[ j ];// discount from $ exports
						f2[ j ] = sup2[ j ] = 0;// nothing else to supply
					}
				}
				else
				{
					v[6] = v[2] * f2[ j ];		// firm $ demand allocation
					v[7] = v[6] / p2[ j ];		// firm # demand allocation

					if ( v[7] <= sup2[ j ] )	// can supply all demanded?
					{
						INCRS( cur, "_D2l", v[7] );// supply all demanded
						v[0] += v[7];			// accumulate to total # demand
						v[4] -= v[6];			// discount from desired $ demand
						v[5] += f2[ j ];		// save share yet to allocate
						sup2[ j ] -= v[7];		// make supplied # unavailable
					}
					else
					{
						if ( i == 0 )			// unsatisfied demand metric
							WRITES( cur, "_l2", v[7] - sup2[ j ] );

						INCRS( cur, "_D2l", sup2[ j ] );// supply all # available
						v[0] += sup2[ j ];		// accumulate to total # demand
						v[4] -= sup2[ j ] * p2[ j ];// discount from $ demand
						f2[ j ] = sup2[ j ] = 0;// nothing else to supply
					}
				}
			}
			else
				f2[ j ] = 0;					// nothing else to supply
		}

		++j;
	}

	if ( v[5] > 0 )								// unallocated shares remaining?
		for ( j = 0; j < k; ++j )
			f2[ j ] /= v[5];					// rescale remaining firms
	else
		break;									// nothing else to supply

	v[1] = v[3];								// update unallocated
	v[2] = v[4];
	++i;
}

// compute final effective consumption, allocating possible shortages
v[8] = VS( COUNTRL1, "Gcd" );					// government desired consumption
v[9] = 1 - v[4] / ( VS( LABSUPL1, "CdW" ) + v[8] );// rationing factor

WRITES( COUNTRL1, "Gc", v[8] * v[9] );			// effective government cons.

CYCLES( LABSUPL1, cur, "Worker" )
	WRITES( cur, "_C", VS( cur, "_Cd" ) * v[9] );// effective worker consumption

RESULT( v[0] )


EQUATION( "MC2" )
/*
Market entry conditions index in consumer-good sector
*/
RESULT( log( max( VL( "NW2", 1 ), 0 ) + 1 ) - log( VL( "Deb2", 1 ) + 1 ) )


EQUATION( "entry2exit" )
/*
Net (number of) entrant firms in consumer-good sector
Perform entry and exit of firms in the consumer-good sector
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
Also updates 'F2', 'Eq2entryG', 'Eq2exitG', 'NW2exitG', 'Eq2entryW', 'Eq2exitW',
'NW2exitW', 'exit2', 'entry2', 'exit2fail'
*/

SUM( "_D2d" );									// desired demand before chg
SUMS( LABSUPL1, "_Bon" );						// register bonuses
UPDATE;											// ensure aggregates are computed

double MC2 = V( "MC2" );						// market conditions in sector 2
double MC2_1 = VL( "MC2", 1 );					// market conditions in sector 2
double f2min = V( "f2min" );					// min market share in sector 2
double n2 = V( "n2" );							// market participation period
double omicron = VS( COUNTRL1, "omicron" );		// entry sensitivity to mkt cond
double stick = VS( COUNTRL1, "stick" );			// stickiness in number of firms
double x2inf = VS( COUNTRL1, "x2inf" );			// entry lower distrib. support
double x2sup = VS( COUNTRL1, "x2sup" );			// entry upper distrib. support
int F2 = V( "F2" );								// current number of firms
int F20 = V( "F20" );							// initial number of firms
int F2max = V( "F2max" );						// max firms in sector 2
int F2min = V( "F2min" );						// min firms in sector 2
int flagFirmOwner = VS( COUNTRL1, "flagFirmOwner" );// government firms?
int l, n;

double statThrs = V( "gamma2g" ) * V( "L2" ) / F2;// statization threshold
intVecT quit( F2, 0 );							// firms' quit options

WRITE( "Eq2entryW", 0 );						// reset exit/entry accumulators
WRITE( "Eq2exitW", 0 );
WRITE( "NW2exitW", 0 );
WRITE( "Eq2entryG", 0 );						// private shareholders accum.
WRITE( "Eq2exitG", 0 );
WRITE( "NW2exitG", 0 );

// mark bankrupt and market-share-irrelevant incumbent firms to quit the market
h = F2;											// initial number of firms
v[1] = v[2] = i = k = l = 0;					// accum., counters, registers
CYCLE( cur, "Firm2" )
{
	v[3] = VS( cur, "_NW2" );					// current net wealth

	if ( v[3] < 0 || VS( cur, "_life2cycle" ) > 1 )// bankrupt or incumbent?
	{
		for ( v[4] = j = 0; j < n2; ++j )
			v[4] += VLS( cur, "_f2", j ) / n2;	// n2 periods avg. market share

		if ( v[3] < 0 || v[4] < f2min )
		{
			if ( v[3] < 0 && flagFirmOwner > 1 && VS( cur, "_own2" ) == 0 &&
				 VS( cur, "_L2" ) > statThrs )	// is statization possible?
			{
				quit[ i ] = 2;					// mark for possible statization

				v[5] = VS( cur, "_L2" );		// firm number of workers
				if ( v[5] > v[2] )				// best candidate so far?
				{
					l = i;						// save firm index
					v[2] = v[5];				// and number of workers
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
				v[1] = v[4];					// and market share
			}
		}
	}

	++i;
}

// quit candidate firms exit, except the best one if all going to quit
// and good bankrupt firms that are rescued by the government (statization)
v[6] = i = j = n = 0;							// firm counters
CYCLE_SAFE( cur, "Firm2" )
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
			if ( VS( cur, "_NW2" ) < 0 )		// count bankruptcies
				++n;

			CFUNS( cur, exit_firm, & v[6] );	// delete object properly
		}
		else
			if ( h == 0 && i == k )				// best firm must be rescued?
				CFUNS( cur, rescue_firm, false );// shareholder rescue
	}

	++i;
}

V( "f2rescale" );								// redistribute exiting m.s.

// compute the potential number of entrants
v[7] = ( MC2_1 == 0 ) ? 0 : MC2 / MC2_1 - 1;	// change in market conditions

k = max( 0, round( F2 * ( ( 1 - omicron ) * uniform( x2inf, x2sup ) +
						  omicron * min( max( v[7], x2inf ), x2sup ) ) ) );

// apply return-to-the-average stickiness random shock to the number of entrants
k -= min( RND * stick * ( ( double ) ( F2 - j ) / F20 - 1 ) * F20, k );

// ensure limits are enforced to the number of entrants
if ( F2 - j + k < F2min )
	k = F2min - F2 + j;

if ( F2 + k > F2max )
	k = F2max - F2 + j;

CFUN( entry_firm2, k, false );					// add entrant-firm objects

v[0] = k - j;									// net number of entrants
INCR( "F2", v[0] );								// update the number of firms
INCR( "fires2", v[6] );							// update fires
WRITE( "exit2", ( double ) j / F2 );
WRITE( "entry2", ( double ) k / F2 );
WRITES( SECSTAL1, "exit2fail", ( double ) n / F2 );
RECALCS( FINSECL1, "BadDeb2" );					// update bad debt after exits

V( "f2rescale" );								// redistribute entrant m.s.
V( "firm2maps" );								// update firm mapping vectors

RESULT( v[0] )


EQUATION( "hires2" )
/*
Number of workers hired by firms in consumption-good sector
Process required hiring using the appropriate rule
Updates '_hires2'
*/

VS( CAPSECL1, "hires1" );						// ensure sector 1 is done

i = VS( COUNTRL1, "flagHeterWage" );			// heterogeneous wages flag

// organize applications to the selected hiring order
CYCLE( cur, "Firm2" )
{
	j = VS( cur, "_postChg" ) ? VS( COUNTRL1, "flagWageOfferChg" ) :
								VS( COUNTRL1, "flagWageOffer" );

	if ( i == 0 || j == 0 )						// avoid re-sorting the applics.
	{
		// sort firm's candidate list according to the defined strategy
		k = VS( cur, "_own2" ) == 1 ? VS( COUNTRL1, "flagHireOrder2g" ) :
			VS( cur, "_postChg" ) ? VS( COUNTRL1, "flagHireOrder2Chg" ) :
									VS( COUNTRL1, "flagHireOrder2" );
		CFUN( order_applications, k, & V_EXTS( cur, firm2E, appl ) );
	}

	WRITES( cur, "_hires2", 0 );				// zero hires count for firm
}

v[0] = 0;
if ( VS( COUNTRL1, "flagEduc" ) > 0 )			// education enabled?
{
	v[0] += CFUN( hire_workers, 3 );		// hire upper categories workers
	v[0] += CFUN( hire_workers, 2 );
}

v[0] += CFUN( hire_workers, 1 );			// hire category 1 workers

CYCLE( cur, "Firm2" )
	EXEC_EXTS( cur, firm2E, appl, clear );		// clear application queues

RECALC( "quits2" );								// force update of industry quits

RESULT( v[0] )


EQUATION( "sV2avg" )
/*
Average vintage skills for all technologies/vintages in consumption-good sector
Also updates the map of vintage productivity and skill
*/

h = T0( VL( "oldVint", 1 ) );					// time of oldest vintage
v[1] = VS( LABSUPL1, "Lscale" );				// workers to objects ratio

vintMapT *vint = & EXTS( COUNTRL1, countryE ).vintProd;// vintage map
vintMapT::iterator itm;

// remove unused vintages from the map
for( itm = vint->begin( ); T0( itm->first ) < ( ullongT ) h &&
						   itm != vint->end( ); ++itm );

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
		i = VS( PARENTS( cur1 ), "__IDvint" );	// vintage ID
		v[2] = VLS( cur, "_sV", 1 ) * v[1];		// last skills (weighted)
		EXTS( COUNTRL1, countryE ).vintProd[ i ].sVavg += v[2];
		EXTS( COUNTRL1, countryE ).vintProd[ i ].workers += v[1];
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

RESULT( v[2] > 0 ? v[0] / v[2] : INISKILL )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A2" )
/*
Machine-level weighted-average labor productivity of consumption-good sector
*/
V( "CPI" );										// ensure m.s. are updated
RESULT( WHTAVE( "_A2", "_f2" ) )


EQUATION( "A2p" )
/*
Machine-level weighted-average labor potential productivity of consumption-good
sector
*/
V( "CPI" );										// ensure m.s. are updated
RESULT( WHTAVE( "_A2p", "_f2" ) )


EQUATION( "Bon2" )
/*
Total bonuses paid by firms in consumption-good sector
*/
V( "Tax2" );									// ensure bonuses are computed
RESULT( SUM( "_Bon2" ) )


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


EQUATION( "D2e" )
/*
Total demand expectation (in product units) of firms in consumer-good sector
*/
RESULT( SUM( "_D2e" ) )


EQUATION( "D2lD" )
/*
Desired local demand (in product units) for firms in consumption-good sector
*/
RESULT( VS( COUNTRL1, "Cd" ) / V( "CPI" ) )


EQUATION( "D2xD" )
/*
Desired export demand (in product units) for firms in in consumption-good sector
*/
VS( WORLDL1, "XcW" );							// ensure import/export done
RESULT( VS( COUNTRL1, "XcD" ) * ( 1 - VS( WORLDL1, "trMCavgW" ) ) *
								( 1 - V( "trX2" ) ) / V( "CPI" ) )


EQUATION( "Deb2" )
/*
Total debt of consumption-good sector
*/
RESULT( SUM( "_Deb2" ) )


EQUATION( "Div2" )
/*
Total dividends paid by firms in consumption-good sector
*/
RESULT( V( "Div2g" ) + V( "Div2w" ) )


EQUATION( "Div2g" )
/*
Total dividends paid by government-owned firms in consumption-good sector
*/
V( "Tax2" );									// ensure dividends are computed
RESULT( SUM_CND( "_Div2", "_own2", "==", 1 ) )


EQUATION( "Div2w" )
/*
Total dividends paid by private firms in consumption-good sector
*/
V( "Tax2" );									// ensure dividends are computed
RESULT( SUM_CND( "_Div2", "_own2", "==", 0 ) )


EQUATION( "EI" )
/*
Total expansion investment in consumption-good sector
*/
V( "CI" );										// ensure cancellations acct'd
RESULT( SUM( "_EI" ) )


EQUATION( "E2avg" )
/*
Weighted average competitiveness of firms in consumption-good sector
*/

v[0] = 0;
CYCLE( cur, "Firm2" )
	if ( VS( cur, "_life2cycle" ) > 0 )			// consider only non-entrants
		v[0] += VS( cur, "_E2" ) * VLS( cur, "_f2", 1 );
												// compute weighted average
RESULT( v[0] )


EQUATION( "Eq2" )
/*
Equity hold by workers/households from firms in consumption-good sector
*/
RESULT( SUM( "_Eq2" ) )


EQUATION( "F2" )
/*
Number of firms in consumption-good sector
*/
RESULT( COUNT( "Firm2" ) )


EQUATION( "Id" )
/*
Total desired investment in terms of output capacity (real terms)
Don't recompute 'SI'/'EI' at this stage, to wait for order cancellations
*/
RESULT( SUM( "_SI" ) + SUM( "_EI" ) )


EQUATION( "Inom" )
/*
Aggregated investment (nominal/currency terms)
*/
RESULT( SUM( "_Inom" ) )


EQUATION( "Ireal" )
/*
Aggregated real investment (in initial prices terms)
*/
RESULT( ( V( "SI" ) + V( "EI" ) ) / V( "m2" ) * VS( CAPSECL1, "pK0" ) )


EQUATION( "JO2" )
/*
Open job positions in consumption-good sector
*/
RESULT( V( "JO21" ) + V( "JO22" ) + V( "JO23" ) )


EQUATION( "JO21" )
/*
Open job positions of category 1 in consumption-good sector
*/
RESULT( SUM( "_JO21" ) )


EQUATION( "JO22" )
/*
Open job positions of category 2 in consumption-good sector
*/
RESULT( SUM( "_JO22" ) )


EQUATION( "JO23" )
/*
Open job positions of category 3 in consumption-good sector
*/
RESULT( SUM( "_JO23" ) )


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


EQUATION( "Knom" )
/*
Total capital (nominal/money terms) in consumption-good sector
*/
RESULT( SUM( "_Knom" ) )


EQUATION( "L2" )
/*
Work force (labor) size in consumption-good sector
*/
RESULT( SUM( "_L2" ) )


EQUATION( "L2d" )
/*
Total labor demand from firms in consumption-good sector
*/
RESULT( V( "L2d1" ) + V( "L2d2" ) + V( "L2d3" ) )


EQUATION( "L2d1" )
/*
Total category 1 labor demand from firms in consumption-good sector
*/
RESULT( SUM( "_L2d1" ) )


EQUATION( "L2d2" )
/*
Total category 2 labor demand from firms in consumption-good sector
*/
RESULT( SUM( "_L2d2" ) )


EQUATION( "L2d3" )
/*
Total category 3 labor demand from firms in consumption-good sector
*/
RESULT( SUM( "_L2d3" ) )


EQUATION( "M2" )
/*
Machine imports (in domestic currency including duties) by consumption-good sector
*/
RESULT( SUM( "_M2" ) )


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
V( "CPI" );										// ensure m.s. are updated
RESULT( WHTAVE( "_Pi2rate", "_f2"  ) )


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
RESULT( V( "S2l" ) + V( "S2x" ) )


EQUATION( "S2d" )
/*
Total desired sales of consumption-good sector
*/
RESULT( SUM( "_S2d" ) )


EQUATION( "S2l" )
/*
Local sales of consumption-good sector
*/
RESULT( SUM( "_S2l" ) )


EQUATION( "S2x" )
/*
Export sales of consumption-good sector
*/
RESULT( SUM( "_S2x" ) )


EQUATION( "SI" )
/*
Total substitution investment in consumption-good sector
*/
V( "CI" );										// ensure cancellations acct'd
RESULT( SUM( "_SI" ) )


EQUATION( "Tax2" )
/*
Taxes on profits paid by firms in consumption-good sector
*/
RESULT( SUM( "_Tax2" ) )


EQUATION( "TaxM2" )
/*
Local import duties (in domestic currency) paid by consumption-good sector
*/
RESULT( SUM( "_TaxM2" ) )


EQUATION( "TaxM2f" )
/*
Foreign import duties (in domestic currency) paid by consumption-good sector
*/
RESULT( SUM( "_TaxM2f" ) )


EQUATION( "TaxX2" )
/*
Local export duties (in domestic currency) paid by consumption-good sector
*/
RESULT( SUM( "_TaxX2" ) )


EQUATION( "W2" )
/*
Total wages paid by all firms in sector 2
*/
RESULT( SUM( "_W2" ) )


EQUATION( "X2" )
/*
Exports (in domestic currency including duties) by consumption-good sector
*/
RESULT( SUM( "_X2" ) )


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
RESULT( CFUN( mov_avg_bound, "CPI", VS( COUNTRL1, "mLim" ), VS( COUNTRL1, "mPer" ) ) )


EQUATION( "dN" )
/*
Change in total inventories (real terms)
*/
RESULT( V( "N" ) - VL( "N", 1 ) )


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

if ( T >= VS( COUNTRL1, "TregChg" ) + V( "ent2HldPer" ) &&
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
v[0] = SUM_CND( "_f2", "_postChg", "!=", 0 )
RESULT( ! isnan( v[0] ) ? v[0] : 0 )


EQUATION( "fires2" )
/*
Workers fired in consumption-good sector
*/
RESULT( SUM( "_fires2" ) )


EQUATION( "i2" )
/*
Interest paid by consumption-good sector
*/
RESULT( SUM( "_i2" ) )


EQUATION( "iD2" )
/*
Interest received from deposits by consumption-good sector
*/
RESULT( SUM( "_iD2" ) )


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
RESULT( v[1] > 0 ? WHTAVE( "_q2", "_Q2e" ) / v[1] : CURRENT	 )


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


EQUATION( "w2o1avg" )
/*
Weighted average wage offered to category 1 workers in consumption-good sector
*/
v[1] = WHTAVE( "_w2o1", "_L2d1" );
v[2] = V( "L2d1" );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : CURRENT )


EQUATION( "w2o2avg" )
/*
Weighted average wage offered to category 2 workers in consumption-good sector
*/
v[1] = WHTAVE( "_w2o2", "_L2d2" );
v[2] = V( "L2d2" );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : CURRENT )


EQUATION( "w2o3avg" )
/*
Weighted average wage offered to category 3 workers in consumption-good sector
*/
v[1] = WHTAVE( "_w2o3", "_L2d3" );
v[2] = V( "L2d3" );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : CURRENT )


EQUATION( "w2o1max" )
/*
Highest wage offered to category 1 workers in consumption-good sector
*/
RESULT( MAX( "_w2o1" ) )


EQUATION( "w2o2max" )
/*
Highest wage offered to category 2 workers in consumption-good sector
*/
RESULT( MAX( "_w2o2" ) )


EQUATION( "w2o3max" )
/*
Highest wage offered to category 3 workers in consumption-good sector
*/
RESULT( MAX( "_w2o3" ) )


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

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
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
EXEC_EXTS( COUNTRL1, countryE, firm2map, clear );
EXEC_EXTS( COUNTRL1, countryE, firm2ptr, clear );
EXEC_EXTS( COUNTRL1, countryE, firm2wgtd, clear );

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

	EXEC_EXTS( COUNTRL1, countryE, firm2wgtd, push_back, v[5] );
	EXEC_EXTS( COUNTRL1, countryE, firm2ptr, push_back, cur );// pointer to firm
	EXEC_EXTS( COUNTRL1, countryE, firm2map, insert,// save in firm's map
			   firmPairT( ( int ) VS( cur, "_ID2" ), cur ) );

	++i;
}

// rescale the transformed shares to 1 and accumulate them
for ( v[6] = 0, j = 0; j < i; ++j )
{
	v[6] += V_EXTS( COUNTRL1, countryE, firm2wgtd [ j ] ) / v[3];
	WRITE_EXTS( COUNTRL1, countryE, firm2wgtd[ j ], min( v[6], 1 ) );
}

RESULT( i )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "Eq2entryG", "" )
/*
Cost of new equity from firms rescued by government in consumption-good sector
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "Eq2entryW", "" )
/*
Cost of new equity from private-firm entries in consumption-good sector
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "Eq2exitG", "" )
/*
Destroyed equity from government-owned firm exits in consumption-good sector
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "Eq2exitW", "" )
/*
Destroyed equity from private-firm exits in consumption-good sector
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "NW2exitG", "" )
/*
Liquidation net worth returned from government firm exits in consumption sector
Updated in 'entry2exit'
*/

EQUATION_DUMMY( "NW2exitW", "" )
/*
Liquidation net worth returned from private-firm exits in consumption sector
Updated in 'entry2exit'
*/

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
