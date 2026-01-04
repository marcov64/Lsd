/******************************************************************************

	COUNTRY OBJECT EQUATIONS
	------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the Country objects in the K+S LSD model
	are coded below. Also the general country-level initialization is
	defined below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "CdBas" )
/*
Total desired (in money terms) consumption of basic consumption-goods
Doesn't include desired government demand 'Gd'
Also updates 'SavBas'
*/

// workers' desired consumption of basic goods
v[0] = SUMS( LABSUPL0, "_CdBas" ) * VS( LABSUPL0, "Lscale" );

// capitalists' income (past period dividends)
v[0] += ( V( "flagTax" ) == 2 ? 1 - V( "tr" ) : 1 ) * VL( "Div", 1 );

// handle accumulated forced savings from the past
switch ( ( int ) V( "flagCons" ) )
{
	case 0:										// ignore unfilled past demand
		break;

	case 1:
		// spend all forced savings, deducted from entry/exit public cost/credit
		v[0] += V( "SavBas" );
		WRITE( "SavBas", 0 );
		break;

	case 2:
	default:
		// slow spend of unfulfilled past consumption is necessary to avoid
		// economy overheating, so recover up to a limit of current consumption
		v[1] = V( "SavBas" );					// accumulated forced savings
		v[2] = v[0] * V( "Crec" );				// max recover limit

		if ( v[1] <= v[2] )						// fit in limit?
		{
			v[0] += v[1];						// yes: use all savings
			WRITE( "SavBas", 0 );
		}
		else
		{
			v[0] += v[2];						// no: spend the limit
			INCR( "SavBas", - v[2] );			// update forced savings
		}
}

RESULT( v[0] )


EQUATION( "CdLux" )
/*
Total desired (in money terms) consumption of luxury consumption-goods
*/
RESULT( SUMS( LABSUPL0, "_CdLux" ) * VS( LABSUPL0, "Lscale" ) )


EQUATION( "DcBas" )
/*
Demand (in money terms) which can be fulfilled by basic-good industries
Effective supply fulfillment is slightly different because of firm-allocation
indivisibilities, industry price averaging etc.
Also updates 'D2a', 'D2d'
*/

V( "fCrescale" );								// ensure w.s. are updated

v[1] = V( "CdBas" ) + V( "Gd" );				// desired cons. of basic goods

// create/fill temporary share and supply vectors & initialize industry demand
k = V( "Fc" );									// number of industries
dblVecT f2( k ), f2ini( k ), sup2( k );

v[2] = j = 0;
CYCLE( cur, "Consumption" )
{
	if ( VS( cur, "type2" ) == 0 )				// only consider basic industries
	{
		// industry available supply
		sup2[ j ] = 0;
		CYCLES( cur, cur1, "Firm2" )
			sup2[ j ] += ( VS( cur1, "_Q2e" ) + VLS( cur1, "_N2", 1 ) ) *
						 VS( cur1, "_p2" );

		v[2] += f2[ j ] = VS( cur, "f2" );		// industry expected wallet share
		WRITES( cur, "D2a", 0 );				// demand allocated accumulator
	}
	else
		f2[ j ] = 0;							// ignore luxury industries here

	++j;
}

// rescale shares of basic industries to 1 and register original desired demand
j = 0;
CYCLE( cur, "Consumption" )
{
	f2ini[ j ] = f2[ j ] /= v[2];				// rescale share remaining firms
	WRITES( cur, "D2d", v[1] * f2[ j ] );		// demand initially desired
	++j;
}

// cycle through industries until all demand allocated or no production to sell
v[0] = 0;										// fulfilled demand accumulator
while ( v[1] > 0.01 )
{
	v[2] = v[1];								// remaining unallocated demand
	v[3] = j = 0;								// shares yet unallocated
	CYCLE( cur, "Consumption" )
	{
		if ( f2[ j ] > 0 )						// industry has demand to supply
		{
			if ( sup2[ j ] > 0 )				// product to supply?
			{
				v[4] = v[1] * f2[ j ];			// industry demand allocation

				if ( v[4] <= sup2[ j ] )		// can supply all demanded?
				{
					// supply all demanded
					INCRS( cur, "D2a", v[4] );
					v[0] += v[4];				// accumulate to total demand
					v[2] -= v[4];				// discount from desired demand
					v[3] += f2[ j ];			// save share yet to allocate
					sup2[ j ] -= v[4];			// make supplied unavailable
				}
				else
				{
					// supply all available
					INCRS( cur, "D2a", sup2[ j ] );
					v[0] += sup2[ j ];			// accumulate to total demand
					v[2] -= sup2[ j ];			// discount from desired demand
					f2[ j ] = sup2[ j ] = 0;	// nothing else to supply
				}
			}
			else
				f2[ j ] = 0;					// nothing else to supply
		}

		++j;
	}

	if ( v[3] > 0 )								// unallocated shares remaining?
		for ( j = 0; j < k; ++j )
			f2[ j ] /= v[3];					// rescale remaining
	else
		break;									// nothing else to supply

	v[1] = v[2];								// update unallocated
}

// allocate excess global demand into industries' desired demands
if ( v[2] > 0.01 )
{
	j = 0;
	CYCLE( cur, "Consumption" )
	{
		if ( VS( cur, "type2" ) == 0 )			// only consider basic industries
			INCRS( cur, "D2a", v[2] * f2ini[ j ] );

		++j;
	}
}

RESULT( v[0] )


EQUATION( "DcLux" )
/*
Demand (in money terms) effectively fulfilled by luxury-good industries
Effective supply fulfillment is slightly different because of firm-allocation
indivisibilities, industry price averaging etc.
Also updates 'D2a', 'D2d'
*/

V( "CdLux" );									// ensure desired demand is set
k = VS( LABSUPL0, "Tlux");						// period between luxury buys
v[1] = VS( LABSUPL0, "Lscale" );				// labor scaling

v[0] = 0;										// fulfilled demand accumulators
CYCLE( cur, "Consumption" )
	if ( VS( cur, "type2" ) > 0 )				// only consider luxury industries
	{
		// industry average price and available supply
		v[2] = v[3] = 0;
		CYCLES( cur, cur1, "Firm2" )
		{
			v[2] += v[4] = VS( cur1, "_Q2e" ) + VLS( cur1, "_N2", 1 );
			v[3] += v[4] * VS( cur1, "_p2" );
		}

		v[2] = v[3] / v[2];						// weighted average price
		h = VS( cur, "ID2" );					// industry/product ID

		buyLisT *orders = & V_EXTS( cur, ind2E, buyOrd );
		CFUN( shuffle_orders, orders );			// do different order each time

		v[4] = v[5] = 0;
		for ( buyLisT::iterator it = orders->begin( ); it != orders->end( ); ++it )
		{
			v[6] = v[7] = it->bdgt * v[1];		// customer budget (scaled)

			if ( v[3] > 0 )						// product to supply?
			{
				if ( v[3] >= v[6] )				// enough for order?
				{
					v[4] += v[6];				// simply fulfill it
					v[3] -= v[6];
					continue;
				}
				else
					if ( v[3] >= v[2] * v[1] )	// at least one unit?
					{
						v[4] += v[3];			// fulfill what is possible
						v[6] -= v[3];
						v[3] = 0;
					}
			}

			v[5] += v[6];						// unfilled demand

			// remove from bought list if no unit was supplied
			if ( v[6] == v[7] )
			{
				EXEC_EXTS( it->wrk, wrkE, buyLux, erase, h );
				WRITES( it->wrk, "_tLux", T - k );// try to buy next period
			}

			// restore order value to the consumer savings
			INCRS( it->wrk, "_SavLux", v[6] / v[1] );
		}

		v[0] += v[4];							// global accumulated demand
		orders->clear( );						// clear industry order set

		WRITES( cur, "D2a", v[4] );				// industry allocated demand
		WRITES( cur, "D2d", v[4] + v[5] );		// industry desired demand
	}

RESULT( v[0] )


EQUATION( "Gd" )
/*
Government desired expenditure (exogenous demand in nominal terms)
Excludes unemployment/minimum income benefits (transfers)
*/

v[0] = VS( LABSUPL0, "Gtrain" );				// worker training cost

i = V( "flagGovExp" );							// type of govt. exped.
if ( i >= 1 )									// do public spending?
{
	v[0] += V( "gG" ) * VL( "GDPnom", 1 );		// fixed public spending

	if ( i >= 3 )								// spend accumulated surplus?
	{
		v[2] = VL( "Deb", 1 );
		if ( v[2] < 0 )
		{
			v[3] = max( 0, - VL( "Def", 1 ) );	// limit to cur. superavit
			if ( - v[2] > v[3] )
			{
				v[0] += v[3];					// cap to current sup.
				INCR( "Deb", v[3] );			// discount from surplus
			}
			else
			{
				v[0] += - v[2];					// spend all surplus
				WRITE( "Deb", 0 );				// zero debt
			}
		}
	}
}

RESULT( v[0] )


EQUATION( "SavBas" )
/*
Savings accumulated for basic-goods consumption (forced savings)
Also updated in 'CdBas', 'SavForc'
*/

// apply interest to the savings balance
v[0] = CURRENT * ( 1 + VS( FINSECL0, "rD" ) );	// update savings

// add entry (equity) / exit (net cash) cost/credit incurred by households
v[0] += - VL( "cEntry", 1 ) + VL( "cExit", 1 );

RESULT( ROUND( v[0], 0, 0.001 ) )				// avoid rounding errors on zero


EQUATION( "SavForc" )
/*
Forced savings in period because of basic consumption-goods shortage
Also updates 'SavBas'
*/

// unfilled-demand forced savings of basic goods (avoid rounding errors on zero)
v[0] = ROUND( V( "CdBas" ) + V( "Gd" ) - V( "ScBas" ), 0, 0.001 );

V( "SavBas" );									// ensure up-to-date before
INCR( "SavBas", v[0] );							// updating accumulated

RESULT( v[0] )


EQUATION( "entryExit" )
/*
Perform the entry and exit process in all sectors
All relevant aggregate variables in country must be computed before existing
industries are deleted, so all active industries in period are considered
Updates 'DeltaG', 'cEntry', 'cExit', 'cExitAcc'
*/

// ensure aggregates depending on industry/firm objects are computed
UPDATE;											// country variables
UPDATES( FINSECL0 );							// financial sector variables
UPDATES( LABSUPL0 );							// labor-supply variables
UPDATES( MACSTAL0 );							// statistics-only variables
UPDATES( SECSTAL0 );
UPDATES( LABSTAL0 );

// reset entry/exit cost/credit
WRITE( "cExit", 0 );
v[1] = v[2] = 0;								// industry entry-exit cost acc.

// perform firms entry and exit in existing industries
v[0] = SUM( "entry1exit" ) + SUM( "entry2exit" );// net entry number of firms

// take under-performing industries out of consumption-goods sector (sector 2)
v[2] = CFUN( exit_consumption, & v[0] );

// likelihood of new basic industry, related to initial opportunities
i = V( "FcBas" );								// current number of industries
j = V( "FcBasMin" );							// min number of industries
k = V( "FcBasMax" );							// max number of industries
h = V( "Fc0bas" );								// initial number of industries
v[3] = max( 1 - V( "stick" ) * ( i - h ) / i, 0 );

// draw the emergence of new basic industry in consumption-goods sector
if ( T > 1 && ( ( V( "toExitCbas" ) && k > 1 ) ||
	 ( i < k && bernoulli( 1 - exp( - V( "zetaBas" ) * v[3] ) ) ) ) )
	v[1] += CFUN( entry_consumption, max( 1, j - i ), true, 0, & v[0] );

// likelihood of new luxury industry, related to last technology jump
i = V( "FcLux" );								// current number of industries
j = V( "FcLuxMin" );							// min number of industries
k = V( "FcLuxMax" );							// max number of industries
v[4] = V( "DeltaG" );

// draw the emergence of new luxury industry in consumption-goods sector
if ( T > 1 && ( ( V( "toExitClux" ) && k > 1 ) ||
	 ( i < k && bernoulli( 1 - exp( - V( "zetaLux" ) * v[4] ) ) ) ) )
	v[1] += CFUN( entry_consumption, max( 1, j - i ), false, 0, & v[0] );

// use part or all of the exit equity credit to pay all or part of entry costs
// unused exit equity is saved on fund to pay future entries
v[1] += V( "cEntry" );							// total accumulated entry cost
v[2] += V( "cExit" );							// total accumulated exit credit

switch ( ( int ) V( "flagExitCredit" ) )
{
	case 0:										// exit credits pay entry costs
	default:
		v[5] = max( v[1] - v[2], 0 );			// net to be paid in period
		v[6] = VL( "cExitAcc", 1 ) + max( v[2] - v[1], 0 );// accumulated credit
		v[7] = max( v[5] - v[6], 0 );			// pay part or all entry cost
		v[8] = 0;								// don't distribute exit credit
		v[9] = max( v[6] - v[5], 0 );			// keep the rest, if any
		break;

	case 1:										// credit returned to workers
		v[7] = v[1];
		v[8] = v[2];
		v[9] = 0;
		break;

	case 2:										// net credit reduce public debt
		v[7] = max( v[1] - v[2], 0 );			// net entry cost to be paid
		v[8] = v[9] = 0;						// no credit return/saving
		V( "Deb" );								// ensure public debt is updated
		INCR( "Deb", - max( v[2] - v[1], 0 ) );	// reduce public debt
}

WRITE( "cEntry", v[7] );
WRITE( "cExit", v[8] );
WRITE( "cExitAcc", v[9] );

V( "fCrescale" );								// redistribute wallet shares
V( "firmMaps" );								// update firm mapping vectors
V( "indMaps" );									// and industry mpping vectors
VS( FINSECL0, "cScores" );						// set new credit pecking order
RECALCS( FINSECL0, "BadDeb" );					// update bad debt after exits

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A" )
/*
Overall (single stage) labor productivity
*/
RESULT( ( WHTAVE( "B1", "L1" ) + WHTAVE( "A2", "L2" ) ) / VS( LABSUPL0, "L" ) )


EQUATION( "AcBas" )
/*
Weighted-average (single-stage) labor productivity of basic consumption-goods
subsector
*/
v[1] = SUM_CND( "L2", "type2", "==", 0 );
RESULT( v[1] > 0 ? WHTAVE_CND( "A2", "L2", "type2", "==", 0 ) / v[1] : CURRENT )


EQUATION( "AcLux" )
/*
Weighted-average (single-stage) labor productivity of luxury consumption-goods
subsector
*/
v[1] = SUM_CND( "L2", "type2", ">", 0 );
RESULT( v[1] > 0 ? WHTAVE_CND( "A2", "L2", "type2", ">", 0 ) / v[1] : CURRENT )


EQUATION( "C" )
/*
Nominal (in money terms) effective aggregated consumption
Government has priority on goods acquisition
*/
v[1] = V( "ScBas" );							// fulfilled basic-goods demand
v[2] = V( "Gd" );								// government demand
RESULT( ( v[1] > v[2] ? min( V( "CdBas" ), v[1] - v[2] ) : 0 ) + V( "ScLux" ) )


EQUATION( "Creal" )
/*
Real (in complexity-adjusted initial prices terms) aggregated consumption
*/

// number of single-production-stage units consumed
v[1] = 0;
CYCLE( cur, "Consumption" )
{
	v[2] = VS( cur, "k2" ) / INICPLX;
	CYCLES( cur, cur1, "Firm2" )
		v[1] += v[2] * VS( cur1, "_D2" ) / VS( cur1, "_p2" );
}

RESULT( v[1] * V( "pC0" ) )


EQUATION( "CPI" )
/*
Consumer price index
*/
RESULT( WHTAVE( "p2", "f2e" ) / V( "pC0" ) )


EQUATION( "Deb" )
/*
Accumulated government debt
Also updated in 'Gd', 'entryExit'
*/
RESULT( CURRENT + V( "Def" ) )


EQUATION( "Def" )
/*
Government current deficit (negative if superavit)
*/

// interest paid on public debt
v[1] = VLS( FINSECL0, "r", 1 ) * VL( "Deb", 1 );

// interest paid by central bank on bank reserves
v[2] = VLS( FINSECL0, "rRes", 1 ) * VLS( FINSECL0, "Depo", 1 );

RECALC( "Deb" );								// force new debt update

RESULT( V( "Gcons" ) + V( "Gtrf" ) + VLS( FINSECL0, "Gbail", 1 ) + v[1] + v[2] -
		V( "Tax" ) )


EQUATION( "Div" )
/*
Total dividends paid by non-financial firms
*/
RESULT( SUM( "Div1" ) + SUM( "Div2" ) + VS( FINSECL0, "DivB" ) )


EQUATION( "EI" )
/*
Aggregated expansion investment (in machine-number terms)
*/
RESULT( SUM( "EI2" ) )


EQUATION( "EcAvg" )
/*
Weighted average expected competitiveness of consumption-good industries
*/
RESULT( WHTAVEL( "E2", "f2", 1 ) )


EQUATION( "Fc" )
/*
Number of industries in consumption-good sector
Also updated in 'entryExit'
*/
RESULT( COUNT( "Consumption" ) )


EQUATION( "FcBas" )
/*
Number of basic industries in consumption-good sector
Also updated in 'entryExit'
*/
RESULT( COUNT_CND( "Consumption", "type2", "==", 0 ) )


EQUATION( "FcLux" )
/*
Number of luxury industries in consumption-good sector
Also updated in 'entryExit'
*/
RESULT( V( "Fc" ) - V( "FcBas" ) )


EQUATION( "Gcons" )
/*
Government effective expenditure in consumption-goods
Government has priority on basic goods acquisition
Includes worker training expenses (Gtrain)
*/
RESULT( min( V( "Gd" ), V( "ScBas" ) ) )


EQUATION( "Gtrf" )
/*
Government transfers (minimum subsistency income or unemployment benefit)
*/

v[1] = VS( LABSUPL0, "Ls" ) - VS( LABSUPL0, "L" );// unemployed workers

if ( V( "flagGovExp" ) < 2 )					// type of govt. exped.
	v[0] = v[1] * VS( LABSUPL0, "w0min" );		// minimum income
else
	v[0] = v[1] * VS( LABSUPL0, "wU" );			// pay unemployment benefit

RESULT( v[0] )


EQUATION( "GDPreal" )
/*
Real (in complexity-adjusted initial prices terms) gross domestic product
*/
//RESULT( max( V( "Creal" ) + V( "Ireal" ) + V( "dNreal" ), 1 ) )
RESULT( max( V( "Creal" ) + V( "Ireal" ), 1 ) )


EQUATION( "GDPnom" )
/*
Nominal (in money terms) gross domestic product
*/
RESULT( max( V( "C" ) + V( "Inom" ) + V( "Gcons" ) + V( "dNnom" ), 1 ) )


EQUATION( "I" )
/*
Aggregated investment (in machine-number terms)
*/
RESULT( V( "SI" ) + V( "EI" ) )


EQUATION( "Id" )
/*
Aggregated desired investment (in machine-number terms)
Don't recompute 'SI'/'EI' at this stage, to wait for order cancellations
*/

v[0] = 0;
CYCLE( cur, "Consumption" )
	v[0] += SUMS( cur, "_SI2" ) + SUMS( cur, "_EI2" );

RESULT( v[0] )


EQUATION( "Inom" )
/*
Aggregated investment (in money terms)
*/
RESULT( SUM( "S1" ) )


EQUATION( "Ireal" )
/*
Aggregated real investment (in initial prices terms)
*/
RESULT( V( "I" ) * V( "pK0" ) )


EQUATION( "K" )
/*
Aggregated capital stock (in machine-number terms)
*/
RESULT( SUM( "K2" ) )


EQUATION( "PPI" )
/*
Producer price index
*/
RESULT( AVE( "p1" ) / V( "pK0" ) )


EQUATION( "SI" )
/*
Aggregated substitution investment (in machine-number terms)
*/
RESULT( SUM( "SI2" ) )


EQUATION( "SavLux" )
/*
Savings accumulated for luxury goods consumption
*/
V( "CdLux" );									// ensure savings are updated
RESULT( SUMS( LABSUPL0, "_SavLux" ) * VS( LABSUPL0, "Lscale" ) )


EQUATION( "Sc" )
/*
Total sales (in money terms) of consumption-good industries
*/
RESULT( SUM( "S2" ) )


EQUATION( "ScBas" )
/*
Total sales (in money terms) of basic consumption-good industries
*/
RESULT( SUM_CND( "S2", "type2", "==", 0 ) )


EQUATION( "ScLux" )
/*
Total sales (in money terms) of luxury consumption-good industries
*/
RESULT( V( "Sc" ) - V( "ScBas" ) )


EQUATION( "Tax" )
/*
Government tax income
*/

i = V( "flagTax" );								// taxation rule

v[1] = 0;										// taxable income acc.

if ( i >= 1 )									// tax workers' income?
	v[1] += VS( LABSUPL0, "W" ) + VLS( LABSUPL0, "Bon", 1 );

if ( i >= 2 )									// tax capitalists' income?
	v[1] += VL( "Div", 1 );

// compute household's taxes plus firms' taxes
v[0] = v[1] * V( "tr" ) + SUM( "Tax1" ) + SUM( "Tax2" ) + VS( FINSECL0, "TaxB" );

RESULT( v[0] )


EQUATION( "dAb" )
/*
Notional overall productivity (bounded) rate of change
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "A", V( "mLim" ) ) )


EQUATION( "dCPIb" )
/*
Consumer price index inflation, bounded moving-average rate
*/
RESULT( CFUN( mov_avg_bound, "CPI", V( "mLim" ) ) )


EQUATION( "dGDPreal" )
/*
Real gross domestic product (log) growth rate
*/
RESULT( T > 1 ? log( V( "GDPreal" ) ) - log( VL( "GDPreal", 1 ) ) : 0 )


EQUATION( "dNnom" )
/*
Change in total nominal inventories (in money terms)
*/
RESULT( SUM( "dN2nom" ) )


EQUATION( "innI" )
/*
Incremental innovation rate of capital goods
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
RESULT( WHTAVE( "inn1i", "F1" ) )


EQUATION( "kCavgBas" )
/*
Weighted-average expected complexity of basic consumption-goods
*/
v[1] = SUM_CND( "f2", "type2", "==", 0 );
RESULT( v[1] > 0 ? WHTAVE_CND( "k2", "f2", "type2", "==", 0 ) / v[1] : CURRENT )


EQUATION( "kCavgLux" )
/*
Weighted-average expected complexity of luxury consumption-goods
*/
v[1] = SUM_CND( "f2", "type2", ">", 0 );
RESULT( v[1] > 0 ? WHTAVE_CND( "k2", "f2", "type2", ">", 0 ) / v[1] : CURRENT )


EQUATION( "kCmax" )
/*
Maximum complexity of consumption-good sector
*/
RESULT( MAX( "k2" ) )


EQUATION( "kCmin" )
/*
Minimum complexity of consumption-good sector
*/
RESULT( MIN( "k2" ) )


EQUATION( "nCmax" )
/*
Maximum product newness of consumption-good sector
*/
RESULT( T - MIN( "t2ent" ) )


EQUATION( "nCmin" )
/*
Minimum product newness of consumption-good sector
*/
RESULT( T - MAX( "t2ent" ) )


EQUATION( "pCmax" )
/*
Maximum price of consumption-good sector
*/
RESULT( MAX( "p2" ) )


EQUATION( "pCmin" )
/*
Minimum price of consumption-good sector
*/
RESULT( MIN( "p2" ) )


EQUATION( "qCmax" )
/*
Maximum product quality of consumption-good sector
*/
RESULT( MAX( "q2" ) )


EQUATION( "qCmin" )
/*
Minimum product quality of consumption-good sector
*/
RESULT( MIN( "q2" ) )


EQUATION( "regChg" )
/*
Produces a labor market regime change at the time step defined in TregChg
If TregChg is zero, there is no regime change
Changed parameters:
	Global:
		flagSearchMode -> flagSearchModeChg
		flagIndexMinWage -> flagIndexMinWageChg
		flagHireSeq -> flagHireSeqChg
		flagHireOrder1 -> flagHireOrder1Chg
		flagFireOrder1 -> flagFireOrder1Chg
		tr -> trChg
		phiT -> phiTchg
		Lambda -> LambdaChg
		muRes -> muResChg
		tauB -> tauBchg
		rT -> rTchg
		Ts -> TsChg
		omega -> omegaPosChg
		e0 -> e0Chg
		mu0bas -> mu0basChg
		mu0lux -> mu0luxChg
	Firm-specific:
		flagHireOrder2 -> flagHireOrder2Chg
		flagFireOrder2 -> flagFireOrder2Chg
		flagFireRule -> flagFireRuleChg
		flagWageOffer -> flagWageOfferChg
		flagIndexWage -> flagIndexWageChg
*/

if ( T == V( "TregChg" ) )						// in time, replace parameters
{
	WRITE( "flagSearchMode", V( "flagSearchModeChg" ) );
	WRITE( "flagIndexMinWage", V( "flagIndexMinWageChg" ) );
	WRITE( "flagHireSeq", V( "flagHireSeqChg" ) );
	WRITE( "flagHireOrder1", V( "flagHireOrder1Chg" ) );
	WRITE( "flagFireOrder1", V( "flagFireOrder1Chg" ) );
	WRITE( "tr", V( "trChg" ) );
	WRITE( "mu0bas", V( "mu0basChg" ) );
	WRITE( "mu0lux", V( "mu0luxChg" ) );
	WRITES( FINSECL0, "phiT", VS( FINSECL0, "phiTchg" ) );
	WRITES( FINSECL0, "Lambda", VS( FINSECL0, "LambdaChg" ) );
	WRITES( FINSECL0, "muRes", VS( FINSECL0, "muResChg" ) );
	WRITES( FINSECL0, "tauB", VS( FINSECL0, "tauBchg" ) );
	WRITES( FINSECL0, "rT", VS( FINSECL0, "rTchg" ) );
	WRITES( LABSUPL0, "Ts", VS( LABSUPL0, "TsChg" ) );
	WRITES( LABSUPL0, "omega", VS( LABSUPL0, "omegaPosChg" ) );

	h = V( "flagAllFirmsChg" );
	if ( h == 1 )
	{
		WRITE( "flagHireOrder2", V( "flagHireOrder2Chg" ) );
		WRITE( "flagFireOrder2", V( "flagFireOrder2Chg" ) );
		WRITE( "flagFireRule", V( "flagFireRuleChg" ) );
		WRITE( "flagWageOffer", V( "flagWageOfferChg" ) );
		WRITE( "flagIndexWage", V( "flagIndexWageChg" ) );
	}

	CYCLE( cur, "Consumption" )					// set each industry
	{
		WRITES( cur, "e0", VS( cur, "e0Chg" ) );

		if ( h == 1 )
		{
			if ( VS( cur, "type2" ) == 0 )
				WRITES( cur, "mu20", V( "mu0basChg" ) );
			else
				WRITES( cur, "mu20", V( "mu0luxChg" ) );

			CYCLES( cur, cur1, "Firm2" )
				WRITES( cur1, "_post2chg", 1 );	// set all firms as post-change
		}
	}

	LOG( "\n Regime changed (t=%g)", T );
	PARAMETER;									// no more evaluate this eq.
	v[0] = 1;
}
else
	v[0] = 0;									// no change

RESULT( v[0] )


/*========================= INITIALIZATION EQUATION ==========================*/

EQUATION( "initCountry" )
/*
Initialize the K+S country object, saving the total initial cost of entry equity
It is run only once per country
*/

PARAMETER;										// execute only once per country

// create the new country as an extension to current 'Country' object
ADDEXT_INIT( countryE );

// country-level pointers to speed-up the access to individual containers
WRITE_EXT( countryE, finSec, SEARCH( "Financial" ) );
WRITE_EXT( countryE, labSup, SEARCH( "Labor" ) );
WRITE_EXT( countryE, macSta, SEARCH( "Mac" ) );
WRITE_EXT( countryE, secSta, SEARCH( "Sec" ) );
WRITE_EXT( countryE, labSta, SEARCH( "Lab" ) );

// pointer shortcuts the access to individual market containers
cur1 = SEARCH( "Capital" );
cur2 = SEARCH( "Consumption" );
cur3 = FINSECL0;
cur4 = LABSUPL0;

V( "regChg" );									// enforce change even at t=1

// check unwanted extra instances (only one of each is required)
if ( COUNT( "Capital" ) > 1 || COUNT( "Consumption" ) > 1 ||
	 COUNT( "Financial" ) > 1 || COUNT( "Labor" ) > 1 ||
	 COUNT( "Stats" ) > 1 || COUNTS( cur1, "Firm1" ) > 1 ||
	 COUNTS( cur1, "Wrk1" ) > 1 || COUNTS( cur2, "Firm2" ) > 1 ||
	 COUNTS( cur3, "Bank" ) > 1 || COUNTS( cur4, "Worker" ) > 1 ||
	 COUNTS( SEARCHS( cur1, "Firm1" ), "Cli" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Broch" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Vint" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Firm2" ), "Wrk2" ) > 1 ||
	 COUNTS( SEARCHS( cur2, "Vint" ), "WrkV" ) > 1 ||
	 COUNTS( SEARCHS( cur3, "Bank" ), "Cli1" ) > 1 ||
	 COUNTS( SEARCHS( cur3, "Bank" ), "Cli2" ) > 1 )
{
	PLOG( "\n Error: multiple-instance objects not allowed, aborting!" );
	ABORT;
}

// remove unwanted object instances
cur = SEARCHS( cur1, "Firm1" );
DELETE( cur );
cur = SEARCHS( cur1, "Wrk1" );
DELETE( cur );

// ensure initial number of industries, firms, banks or workers is consistent
if ( V( "FcBasMin" ) < 1 || V( "FcLuxMin" ) < 0 ||
	 VS( cur1, "F1min" ) < 1 || VS( cur2, "F2min" ) < 1 || VS( cur3, "B" ) < 1 ||
	 V( "FcBasMax" ) < V( "FcBasMin" ) || V( "FcLuxMax" ) < V( "FcLuxMin" ) ||
	 VS( cur1, "F1max" ) < VS( cur1, "F1min" ) ||
	 VS( cur2, "F2max" ) < VS( cur2, "F2min" ) ||
	 VS( cur4, "Lscale" ) < 1 || VS( cur4, "Lscale" ) > VS( cur4, "Ls0" ) )
{
	PLOG( "\n Error: inconsistent number of agents, aborting!" );
	ABORT;
}

WRITE( "Fc0bas", min( max( V( "Fc0bas" ), V( "FcBasMin" ) ), V( "FcBasMax" ) ) );
WRITE( "Fc0lux", min( max( V( "Fc0lux" ), 0 ), V( "FcLuxMax" ) ) );
WRITES( cur1, "F10", min( max( VS( cur1, "F10" ), VS( cur1, "F1min" ) ),
						  VS( cur1, "F1max" ) ) );
WRITES( cur2, "F20", min( max( VS( cur2, "F20" ), VS( cur2, "F2min" ) ),
						  VS( cur2, "F2max" ) ) );

if ( VS( cur4, "Ls0" ) / VS( cur4, "Lscale" ) < 2 * ( VS( cur1, "F10" ) +
		 ( V( "Fc0bas" ) + V( "Fc0lux" ) ) * VS( cur2, "F20" ) ) )
	PLOG( "\n Warning: small number of workers" );

// adjust parameters required to be integer and positive
WRITE( "TregChg", max( 0, ceil( V( "TregChg" ) ) ) );
WRITES( cur4, "Tc", max( 1, ceil( VS( cur4, "Tc" ) ) ) );
WRITES( cur4, "Tr", max( 0, ceil( VS( cur4, "Tr" ) ) ) );

// prepare data required to set initial conditions
double m1 = VS( cur1, "m1" );					// capital prod. in sector 1
double mu1 = VS( cur1, "mu1" );					// mark-up in sector 1
double mu20 = V( "mu0bas" );					// initial mark-up in sector 2
double m2 = VS( cur2, "m2" );					// capital prod. in sector 2
double phiT = VS( cur3, "phiT" );				// unemployed benefit rate target
double rT = VS( cur3, "rT" );					// prime rate target
double Sav0bas = VS( cur4, "Sav0bas" );			// savings mult. for basic goods
double Sav0lux = VS( cur4, "Sav0lux" );			// savings mult. for luxury goods
double w0min = VS( cur4, "w0min" );				// absolute/initial minimum wage
int B = VS( cur3, "B" );						// number of banks
int F10 = VS( cur1, "F10" );					// initial firms in sector 1
int F2max = VS( cur2, "F2max" );				// max firms in sector 2
int Fc0bas = V( "Fc0bas" );						// initial basic industries
int Fc0lux = V( "Fc0lux" );						// initial luxury industries
int Ls0 = VS( cur4, "Ls0" );					// initial labor supply

double A0 = INIPROD;							// initial productivities
double k20 = INICPLX;							// initial complexity
double w0 = INIWAGE;							// initial wages

double c10 = w0 / ( m1 * A0 );					// initial cost in sector 1
double c20 = k20 * w0 / ( m2 * A0 );			// initial cost in sector 2
double f20 = 1.0 / ( Fc0bas + Fc0lux );			// initial industry wallet share
double p10 = ( 1 + mu1 ) * c10;					// initial price sector 1
double p20 = ( 1 + mu20 ) * c20;				// initial price sector 2
double sV0 = ( V( "flagWorkerLBU" ) == 0 || V( "flagWorkerLBU" ) == 2 ) ?
			 1 : VS( cur4, "sigma" );			// initial vintage skills
double EcAvg = ( V( "delta1" ) + V( "delta2" ) +
				 V( "delta3" ) + V( "delta4" ) ) / 2;// initial ind. competitiv.
double SavBas = Sav0bas * w0 * Ls0;				// initial basic forced savings
double SavLux = Sav0lux * w0 * Ls0;				// initial luxury savings

// reserve space for country-level uninitialized vectors
EXEC_EXT( countryE, g1ptr, reserve, 200 );
EXEC_EXT( countryE, firm2ptr, reserve, F2max * ( Fc0bas + Fc0lux ) );
EXEC_EXT( countryE, firm2wgtd, reserve, F2max * ( Fc0bas + Fc0lux ) );
EXEC_EXT( countryE, ind2ptr, reserve, Fc0bas + Fc0lux );
EXEC_EXT( countryE, ind2wgtd, reserve, Fc0bas + Fc0lux );
EXEC_EXT( countryE, bankPtr, reserve, B );
EXEC_EXT( countryE, bankWgtd, reserve, B );

// initialize elements depending on parameters
WRITE( "pK0", p10 );
WRITE( "pC0", p20 );
WRITELL( "AcBas", A0, 0, 1 );
WRITELL( "AcLux", A0, 0, 1 );
WRITELL( "EcAvg", EcAvg, 0, 1 );
WRITELL( "PPI", 1, 0, 1 );
WRITELL( "SavBas", SavBas, 0, 1 );
WRITELL( "SavLux", SavLux, 0, 1 );
WRITELL( "kCavgBas", k20, 0, 1 );
WRITELL( "kCavgLux", k20, 0, 1 );
WRITELL( "pCmax", p20, 0, 1 );
WRITELL( "pCmin", p20, 0, 1 );
WRITELL( "qCmax", 1, 0, 1 );
WRITELL( "qCmin", 1, 0, 1 );
WRITELLS( cur1, "w1avg", w0, 0, 1 );
WRITELLS( cur3, "phi", phiT, 0, 1 );
WRITELLS( cur3, "r", rT, 0, 1 );
WRITELLS( cur3, "rDeb", rT, 0, 1 );				// interest rate structure
WRITELLS( cur3, "rRes", rT, 0, 1 );				// to be defined later
WRITELLS( cur4, "InLux", w0, 0, 1 );
WRITELLS( cur4, "Ls", Ls0, 0, 1 );
WRITELLS( cur4, "sAvg", 1, 0, 1 );
WRITELLS( cur4, "sTavg", 1, 0, 1 );
WRITELLS( cur4, "sTmin", 1, 0, 1 );
WRITELLS( cur4, "sVavg", sV0, 0, 1 );
WRITELLS( cur4, "wAvg", w0, 0, 1 );
WRITELLS( cur4, "wCent", w0, 0, 1 );
WRITELLS( cur4, "wMinPol", w0min, 0, 1 );
WRITELLS( cur4, "woAvg", w0, 0, 1 );

for ( i = 1; i <= 4; ++i )
{
	WRITELL( "A", A0, 0, i );
	WRITELL( "CPI", 1, 0, i );
	WRITELLS( cur1, "B1", A0, 0, i );
}

// create banks' objects and set initial values
v[0] = CFUNS( cur3, entry_bank, B, true );
VS( cur3, "bankMaps" );							// update bank mapping vectors

// create capital-goods firms' objects and set initial values
CFUNS( cur1, add_generation );					// set initial tech. generation
v[0] += CFUNS( cur1, entry_firm1, F10, true );
INIT_TSEARCHS( cur1, "Firm1" );					// prepare turbo search indexing

// create consumer-goods industries and set initial values
v[1] = 0;
v[0] += CFUN( entry_consumption, Fc0bas, true, f20, & v[1] );// basic inds
v[0] += CFUN( entry_consumption, Fc0lux, false, f20, & v[1] );// luxury inds
V( "firmMaps" );								// update firm mapping vectors
V( "indMaps" );									// update industry map vectors

// create workers' objects and set initial values
v[0] += CFUNS( cur4, entry_worker, Ls0, true );

// set banks initial assets according to existing loans to firms
v[0] += CFUNS( cur3, init_bank, true );

RESULT( v[0] )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "fCrescale" )
/*
Rescale wallet shares in consumption-goods sector to ensure adding to 1
To be called after market shares are changed in 'f2' and 'entryExit'
*/

v[1] = SUM( "f2" );								// add-up wallet shares

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator

if ( v[1] > 0 )									// production ok?
	CYCLE( cur, "Consumption" )					// rescale to add-up to 1
	{
		v[0] += v[2] = VS( cur, "f2" ) / v[1];	// rescaled wallet share
		WRITES( cur, "f2", v[2] );				// save updated share
	}
else
{
	v[2] = 1 / COUNT( "Consumption" );			// industry fair share

	CYCLE( cur, "Consumption" )					// rescale to add-up to 1
	{
		v[0] += v[2];
		WRITES( cur, "f2", v[2] );
	}
}

RESULT( v[0] )


EQUATION( "firmMaps" )
/*
Updates the static maps for firms in consumption-good industry
Updates the table of log transformed market share cumulative weights, used
by workers when choosing where to queue for jobs, bigger firms get more applicants
Market shares are rescaled based on the minimum market share and log transformed
Only to be called after firm/industry objects are created or destroyed
*/

// clear vectors
EXEC_EXT( countryE, firm2map, clear );
EXEC_EXT( countryE, firm2ptr, clear );
EXEC_EXT( countryE, firm2wgtd, clear );

i = 0;											// firm index in vector
v[3] = 0;										// cumulative market share
CYCLE( cur, "Consumption" )						// for each industry
{
	v[1] = VS( cur, "f2min" );					// market exit threshold
	v[2] = max( 1 / VLS( cur, "F2", 1 ), 2 * v[1] );// entrant fair share

	CYCLES( cur, cur1, "Firm2" )				// do for all firms in industry
	{
		switch ( ( int ) VS( cur1, "_life2cycle" ) )
		{
			case 0:								// just entered, no share
				v[4] = 0;
				break;

			case 1:								// entrant starting operation
				// fair share lower-bounded (twice) above exit threshold
				v[4] = v[2];
				break;

			default:							// operating entrant/incumbent
				v[4] = max( VLS( cur1, "_f2", 1 ), v[2] );// too small floor
		}

		// log transform market share
		v[3] += v[5] = max( log( v[4] * VLS( cur, "f2", 1 ) / v[1] + 1 ), 0 );

		EXEC_EXT( countryE, firm2wgtd, push_back, v[5] );// firm weight
		EXEC_EXT( countryE, firm2ptr, push_back, cur1 );// firm pointer
		EXEC_EXT( countryE, firm2map, insert, 			// firm map
				  firmPairT( ( int ) VS( cur1, "_ID2" ), cur1 ) );

		++i;
	}
}

// rescale the transformed shares to 1 and accumulate them
for ( v[6] = 0, j = 0; j < i; ++j )
{
	v[6] += V_EXT( countryE, firm2wgtd [ j ] ) / v[3];
	WRITE_EXT( countryE, firm2wgtd[ j ], min( v[6], 1 ) );
}

RESULT( i )


EQUATION( "indMaps" )
/*
Updates the static maps for consumption-good industries
Updates table of ordered competitiveness of goods from sector 2 industries
Only to be called after firm/industry objects are created or destroyed
*/

// clear vectors
EXEC_EXT( countryE, ind2ptr, clear );
EXEC_EXT( countryE, ind2wgtd, clear );

i = 0;											// industry index in vector
v[1] = 0;										// cumulative competitiveness
CYCLE( cur, "Consumption" )
	if ( VS( cur, "type2" ) > 0 )				// luxury industry?
	{
		v[1] += v[2] = VS( cur, "E2" );			// competitiveness

		EXEC_EXT( countryE, ind2wgtd, push_back, v[2] );// industry weight
		EXEC_EXT( countryE, ind2ptr, push_back, cur );// industry pointer

		++i;
	}

// rescale the competitiveness to 1 and accumulate them
for ( v[3] = 0, j = 0; j < i; ++j )
{
	v[3] += V_EXT( countryE, ind2wgtd [ j ] ) / v[1];
	WRITE_EXT( countryE, ind2wgtd[ j ], min( v[3], 1 ) );
}

RESULT( i )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "DeltaG", "entryExit" )
/*
Improvement of latest generation in comparison to the last
exploited one in basic consumption-good industries
Updated in 'entryExit'
*/

EQUATION_DUMMY( "cEntry", "" )
/*
Entry equity costs in all sectors
Updated in 'initCountry', 'entryExit', 'entry1exit', 'entry2exit', 'Ls'
*/

EQUATION_DUMMY( "cExit", "" )
/*
Paid exit equity credits in all sectors
Updated in 'entryExit', 'entry1exit', 'entry2exit'
*/

EQUATION_DUMMY( "cExitAcc", "entryExit" )
/*
Accumulated exit equity credits in all sectors
Updated in 'entryExit'
*/

EQUATION_DUMMY( "toExitCbas", "entryExit" )
/*
Flag indicating last required basic consumption-good industry about to exit
Updated in 'entryExit'
*/

EQUATION_DUMMY( "toExitClux", "entryExit" )
/*
Flag indicating last required luxury consumption-good industry about to exit
Updated in 'entryExit'
*/
