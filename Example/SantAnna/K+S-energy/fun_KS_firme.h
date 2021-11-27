/******************************************************************************

	FIRME OBJECT EQUATIONS
	----------------------

	Equations that are specific to the FirmE objects in the K+S LSD model
	are coded below.

 ******************************************************************************/

#define TECLIMIT		0.7						// thermal efficiency Carnot limit

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_AtauDE" )
/*
Thermal efficiency of a new dirty power plant of energy producer
Also updates '_emTauDE', '_innDE'
*/

v[0] = CURRENT;									// current efficiency
v[1] = V( "_emTauDE" );							// current emissions

// share of R&D expenditure among dirty and green innovation
if ( VS( GRANDPARENT, "flagEnRDshare" ) == 0 )	// exogenous dirty/green share?
	v[2] = VS( PARENT, "xiDE" );				// fixed value
else
{
	v[3] = VS( PARENT, "fGE0" );				// R&D share floor
	v[2] = max( min( 1 - VL( "_fKge", 1 ), 1 - v[3] ), v[3] );// bounded R&D share
}

// normalized worker-equivalent on R&D (for ever-growing energy prices)
v[3] = ( VL( "_RDe", 1 ) / VLS( LABSUPL2, "wReal", 1 ) ) *
	   VS( LABSUPL2, "Ls0" ) / VLS( LABSUPL2, "Ls", 1 );

// dirty energy innovation process (success probability)
v[4] = 1 - exp( - VS( PARENT, "zetaDE" ) * v[2] * v[3] );

if ( bernoulli( v[4] ) )						// dirty innovation succeeded?
{
	v[5] = VS( PARENT, "xEinfTE" );				// lower support th. eff. innov.
	v[6] = VS( PARENT, "xEsupTE" );				// upper support th. eff. innov.
	v[7] = VS( PARENT, "xEinfEm" );				// lower support emissions innov.
	v[8] = VS( PARENT, "xEsupEm" );				// upper support emissions innov.
	v[9] = VS( PARENT, "alphaE" );				// beta distr. alpha parameter
	v[10] = VS( PARENT, "betaE" );				// beta distr. beta parameter

	// draw new innovation values for thermal efficiency and emissions
	v[11] = min( v[0] * ( 1 + v[5] + beta( v[9], v[10] ) * ( v[6] - v[5] ) ),
				 TECLIMIT );					// consider Carnot limit eff.
	v[12] = v[1] * ( 1 - v[7] - beta( v[9], v[10] ) * ( v[8] - v[7] ) );

	// select best between the two options (current/innovation) unit costs
	v[13] = VS( PARENT, "pF" );					// fossil fuel price
	v[14] = VS( PARENT, "trCO2e" );				// carbon tax on energy generat.
	if ( v[13] / v[11] + v[14] * v[12] < v[13] / v[0] + v[14] * v[1] ||
		 ( v[11] == v[0] && v[12] < v[1] ) )	// if eff. capped do emissions
	{
		v[0] = v[11];							// adopt technology
		v[1] = v[12];
		v[15] = 1;								// signal innovation
	}
	else
		v[15] = 0;								// no innovation
}
else
	v[15] = 0;									// no innovation

WRITE( "_emTauDE", v[1] );
WRITE( "_innDE", v[15] );

RESULT( v[0] )


EQUATION( "_DebEmax" )
/*
Prudential maximum bank debt of energy producer
Also updates '_CDe', '_CDeC', '_CSe'
*/

// maximum debt allowed to firm, considering net worth and operating margin
v[1] = VS( FINSECL2, "Lambda" ) * max( VL( "_NWe", 1 ),
									   VL( "_Se", 1 ) - VL( "_Ce", 1 ) );

// apply an absolute floor to maximum debt prudential limit
v[0] = max( v[1], VS( FINSECL2, "Lambda0" ) * VLS( CAPSECL2, "PPI", 1 ) /
				  VS( CAPSECL2, "pK0" ) );

WRITE( "_CDe", 0 );								// reset total credit demand
WRITE( "_CDeC", 0 );							// reset constraint for period
WRITE( "_CSe", 0 );								// reset total credit supplied

RESULT( v[0] )


EQUATION( "_DivE" )
/*
Dividends to pay by firm in energy sector
*/
RESULT( max( VS( PARENT, "dE" ) * ( V( "_PiE" ) - V( "_TaxE" ) ), 0 ) )


EQUATION( "_DeE" )
/*
Adaptive demand expectation of of energy producer
*/
RESULT( V( "_tEent" ) < T - 1 ? VL( "_De", 1 ) : CURRENT )


EQUATION( "_EIe" )
/*
Expansion investment (in capacity terms) of energy producer
Chooses between technologies and deploys new plants
Manages finance for investment if required
Also updates '_NWe', '_DebE', '_CDe', 'CDeC', 'CSe'
*/

v[1] = V( "_EIeD" );							// desired expansion investment
v[2] = v[1] + V( "_SIe" );						// total investment in capacity

if ( v[2] < 1 )									// ignore too small expansions
	END_EQUATION( 0 );							// nothing to invest

v[3] = V( "_AtauDE" );							// dirty unit efficiency
v[4] = V( "_ICtauGE" );							// green unit install cost
v[5] = VS( PARENT, "kappaE" );					// max capacity growth rate
v[6] = VL( "_Kge", 1 );							// current green capacity
v[7] = VL( "_Kde", 1 );							// current dirty capacity
v[8] = V( "_CSeA" );							// available credit supply
v[9] = VL( "_NWe", 1 );							// net worth (cash available)

// try to keep fix share until atmospheric CO2 reference time
if ( T <= VS( CLIMATL2, "tA0" ) )
{
	v[0] = max( ( 1 + VS( PARENT, "iotaE" ) ) * V( "_DeE" ) *
				VS( PARENT, "fGE0" ) - v[6] + V( "_SIgeD" ), 0 );
												// green new capacity
	v[10] = v[2] - v[0];						// dirty new capacity
}
else
	// if green plants are cheaper
	if ( v[4] <= VS( PARENT, "bE") * VS( PARENT, "pF" ) / v[3] )
		// if cap is disabled or new green plants capacity is under cap
		if ( v[5] == 0 || v[6] + v[7] == 0 || v[2] <= v[5] * v[6] )
		{
			v[0] = v[2];						// only green new plants
			v[10] = 0;							// no dirty new plants
		}
		else									// cap limits new green plants
		{
			if ( v[2] - v[5] * v[6] <= v[5] * v[7] )// dirty growth cap ok?
				v[0] = v[5] * v[6];				// invest in green to the cap
			else								// both caps can't be enforced
				v[0] = v[2] * v[6] / ( v[6] + v[7] );// keep green share

			v[10] = v[2] - v[0];				// dirty complements investment
		}
	else										// dirty plants are cheaper
		// if cap is disabled or new dirty plants capacity is under cap
		if ( v[5] == 0 || v[6] + v[7] == 0 || v[2] <= v[5] * v[7] )
		{
			v[10] = v[2];						// only dirty new plants
			v[0] = 0;							// no green new plants
		}
		else									// cap limits new dirty plants
		{
			if ( v[2] - v[5] * v[7] <= v[5] * v[6] )// green growth cap ok?
				v[10] = v[5] * v[7];			// invest in dirty to the cap
			else								// both caps can't be enforced
				v[10] = v[2] * v[7] / ( v[6] + v[7] );// keep dirty share

			v[0] = v[2] - v[10];				// green complements investment
		}

if ( v[0] >= 1 )								// invest in new green plants?
{
	V( "_supplierE" );							// ensure supplier is selected

	v[11] = VS( PARENTS( SHOOKS( HOOK( SUPPL ) ) ), "_p1" );
	v[12] = ceil( v[4] * v[0] / v[11] );		// rounded-up machines
	v[13] = v[0] / v[12];						// unit (machine) power capacity

	v[14] = v[11] * v[12];						// desired investment cost

	if ( v[14] <= v[9] )						// can invest with own funds?
		v[9] -= v[14];							// remove plant cost from cash
	else
	{
		if ( v[14] <= v[9] + v[8] )				// possible to finance all?
		{
			v[15] = v[16] = v[14] - v[9];		// finance the difference
			v[9] = 0;							// no cash
		}
		else									// credit constrained firm
		{
			// invest as much as the available finance allows, rounded # machines
			v[17] = max( floor( ( v[9] + v[8] ) / v[11] ) * v[13], 0 );
			v[16] = v[14] - v[9];				// desired credit

			if ( v[17] == 0 )
				v[15] = 0;						// no finance
			else
			{
				v[12] = v[17] / v[13];			// reduced machine number
				v[14] = v[11] * v[12];			// reduced investment cost
				if ( v[14] <= v[9] )			// just own funds?
				{
					v[15] = 0;
					v[9] -= v[14];				// remove machines cost from cash
				}
				else
				{
					v[15] = v[14] - v[9];		// finance the difference
					v[9] = 0;					// no cash
				}
			}

			v[10] += v[0] - v[17];				// increase dirty generation
			v[0] = v[17];						// reduce green generation

			if ( v[2] - v[1] > v[0] )			// cannot cover substitution?
				WRITE( "_SIe", v[0] );			// adjust substitution
		}

		update_debt( THIS, v[16], v[15] );		// update debt (desired/granted)
	}

	if ( v[0] > 0 )								// investment to do?
	{
		update_depo( THIS, v[9], false );		// update the firm net worth
		send_order( THIS, v[12] );				// send order to sector 1
		add_green_plant( THIS, v[0], v[12], false );// create green plant object
	}
}
else
{
	v[10] += v[0];								// consolidate too small plants
	v[0] = 0;
}

if ( v[10] > 0 )								// new dirty plant?
	add_dirty_plant( THIS, v[10], false );		// create dirty plant object

RESULT( v[0] + v[10] - V( "_SIe" ) )


EQUATION( "_EIeD" )
/*
Desired expansion investment (in capacity terms) of energy producer
*/
RESULT( max( ( 1 + VS( PARENT, "iotaE" ) ) * V( "_DeE" ) - VL( "_Ke", 1 ), 0 ) )


EQUATION( "_ICtauGE" )
/*
Installation cost per unit of capacity of green power plant of energy producer
Also updates '_innGE'
*/

v[0] = CURRENT;									// current installation cost

// share of R&D expenditure among dirty and green innovation
// even if R&D share is endogenous wait until climate warm-up period finishes
if ( VS( GRANDPARENT, "flagEnRDshare" ) == 0 || VS( CLIMATL2, "tA0" ) < T )
	v[2] = 1 - VS( PARENT, "xiDE" );			// fixed value
else
{
	v[1] = VS( PARENT, "fGE0" );				// R&D share floor
	v[2] = max( min( VL( "_fKge", 1 ), 1 - v[1] ), v[1] );// bounded R&D share
}

// normalized worker-equivalent on R&D (for ever-growing energy prices)
v[3] = ( VL( "_RDe", 1 ) / VLS( LABSUPL2, "wReal", 1 ) ) *
	   VS( LABSUPL2, "Ls0" ) / VLS( LABSUPL2, "Ls", 1 );

// green energy innovation process (success probability)
v[4] = 1 - exp( - VS( PARENT, "zetaGE" ) * v[2] * v[3] );

if ( bernoulli( v[4] ) )						// green innovation succeeded?
{
	v[5] = VS( PARENT, "xEinfIC" );				// lower support inst. cost inn.
	v[6] = VS( PARENT, "xEsupIC" );				// upper support inst. cost inn.
	v[7] = VS( PARENT, "alphaE" );				// beta distr. alpha parameter
	v[8] = VS( PARENT, "betaE" );				// beta distr. beta parameter

	// draw new innovation values for installation cost
	v[9] = v[0] * ( 1 - v[5] - beta( v[7], v[8] ) * ( v[6] - v[5] ) );

	// select best between the two options (current/innovation)
	if ( v[9] < v[0] )							// compare costs
	{
		v[0] = v[9];							// adopt technology
		v[10] = 1;								// signal innovation
	}
	else
		v[10] = 0;								// no innovation
}
else
	v[10] = 0;									// no innovation

WRITE( "_innGE", v[10] );

RESULT( v[0] )


EQUATION( "_QeO" )
/*
Generation offered (bid) by energy producer
*/
RESULT( VL( "_Ke", 1 ) )						// all operating capacity


EQUATION( "_RDe" )
/*
R&D expenditure of energy producer
*/
// R&D floor is 1 worker
RESULT( VS( GRANDPARENT, "flagEnClim" ) > 0 ?
		max( VS( PARENT, "nuE" ) * VL( "_Se", 1 ), VS( LABSUPL2, "w" ) ) : 0 )


EQUATION( "_SIe" )
/*
Substitution investment (in capacity terms) of energy producer
*/
RESULT( max( V( "_SIeD" ) - max( VL( "_Ke", 1 ) -
			 ( 1 + VS( PARENT, "iotaE" ) ) * V( "_DeE" ), 0 ), 0 ) )


EQUATION( "_TaxE" )
/*
Taxes paid by energy producer
Also updates '_NWe', '_DebE', '_CDe', '_CDeC', '_CSe'
*/

v[1] = V( "_PiE" );								// firm profit in period

if ( v[1] > 0 )									// profits?
	v[0] = v[1] * VS( GRANDPARENT, "tr" );		// tax to government
else
	v[0] = 0;									// no tax/dividend on losses

cash_flow( THIS, v[1], v[0] );					// manage the period cash flow

v[0] += V( "_EmE" ) * VS( PARENT, "trCO2e" );	// tax on emissions already paid

RESULT( v[0] )


EQUATION( "_allocE" )
/*
Allocate generation among power plants of energy producer
Also updates '__Qde', '__Qge'
*/

VS( PARENT, "De" );								// ensure demand is allocated
V( "_SIeD" );									// ensure scrapping is done

v[1] = V( "_De" );								// energy demand for producer
v[2] = SUM_CND( "__Kge", "__tGE", "<", T );		// available green capacity
v[3] = SUM_CND( "__Kde", "__tDE", "<", T );		// available dirty capacity
v[4] = min( v[1], v[2] );						// green generation demand
v[5] = max( v[1] - v[4], 0 );					// dirty generation demand
v[6] = max( v[2] - v[4], 0 );					// green capacity not to use
v[7] = max( v[3] - v[5], 0 );					// dirty capacity not to use

v[8] = v[9] = 1;								// assume no plant overuse

if ( v[1] > v[2] + v[3] && v[2] + v[3] > 0 )	// excess demand over capacity?
{
	if ( v[3] > 0 )								// dirty plants available?
		v[9] = v[5] / v[3];						// dirty plants overuse factor
	else
		v[8] = v[1] / v[2];						// green plants overuse factor
}

// allocate generation among green plants, favoring larger ones
i = j = 0;										// plants installed/used
SORT( "Green", "__Kge", "UP" );					// sort smaller plants first
CYCLE( cur, "Green" )							// turn on required green plants
{
	if ( VS( cur, "__tGE" ) == T )				// in installation?
		continue;

	v[10] = VS( cur, "__Kge" );					// plant notional capacity

	if( v[6] >= v[10] )							// no use for this plant?
	{
		v[11] = 0;								// no generation for plant
		v[6] -= v[10];							// less capacity not to use
	}
	else										// use plant (full or partial)
	{
		v[11] = v[10] * v[8] - v[6];			// generate what is needed
		v[6] = 0;								// no more plants not to use
		++j;									// one more plant used
	}

	WRITES( cur, "__Qge", v[11] );				// generation for plant
	++i;
}

// allocate remaining generation among dirty plants, favoring cheaper (newer)
CYCLE( cur, "Dirty" )							// turn on required dirty plants
{
	if ( VS( cur, "__tDE" ) == T )				// in installation?
		continue;

	v[12] = VS( cur, "__Kde" );					// plant notional capacity

	if( v[7] >= v[12] )							// no use for this plant?
	{
		v[13] = 0;								// no generation for plant
		v[7] -= v[12];							// less capacity not to use
	}
	else										// use plant (full or partial)
	{
		v[13] = v[12] * v[9] - v[7];			// generate what is needed
		v[7] = 0;								// no more plants not to use
		++j;									// one more plant used
	}

	WRITES( cur, "__Qde", v[13] );				// generation for plant
	++i;
}

RESULT( i > 0 ? ( double ) j / i : 0 )


EQUATION( "_muE" )
/*
Mark-up (in absolute money terms) of energy producer
*/

v[1] = VS( PARENT, "muE0" ) * VLS( LABSUPL2, "wReal", 1 );// updated mark-up floor

if ( VS( PARENT, "FeMax" ) == 1 )				// imposed monopoly?
	END_EQUATION( v[1] );

v[2] = VL( "_fE", 1 );							// past periods market shares
v[3] = VL( "_fE", 2 );
v[4] = VS( PARENT, "fEmin" );					// market exit share threshold

if ( v[2] < v[4] || v[3] < v[4] )				// just entered firms keep floor
	END_EQUATION( v[1] );

RESULT( max( CURRENT * ( 1 + VS( PARENT, "upsilonE" ) * ( v[2] / v[3] - 1 ) ),
			 v[1] ) )


EQUATION( "_pE" )
/*
Price of energy producer
This is the price producer is bidding in the energy auction, to be applied
on the allocated demand in next period
*/

V( "_EIe" );									// ensure capital is deployed

if ( V( "_De" ) <= V( "_Kge" ) + 0.01 )			// just green energy to produce?
	v[1] = MAX_CND( "__cGE", "__Qge", ">", 0 );	// hi price among used g. plants
else
	v[1] = MAX_CND( "__cDE", "__Qde", ">", 0 );	// hi price among used d. plants

RESULT( V( "_muE" ) + ( isfinite( v[1] ) ? v[1] : 0 ) )


EQUATION( "_supplierE" )
/*
Selected capital supplier by firm in energy sector
Also set firm 'hook' pointers to supplier firm object
*/

VS( CAPSECL2, "inn" );							// ensure brochures distributed

v[2] = VS( PARENT, "bE" );						// required payback period

v[4] = DBL_MAX;									// supplier price/cost ratio
i = 0;
cur2 = cur3 = NULL;
CYCLE( cur, "BrE" )								// use brochures to find supplier
{
	cur1 = PARENTS( SHOOKS( cur ) );			// pointer to supplier object

	// compare total machine unit cost (acquisition + operation for payback period)
	v[5] = VS( cur1, "_p1" ) + VS( cur1, "_cTau" ) * v[2];
	if ( v[5] < v[4] )							// best so far?
	{
		v[4] = v[5];							// save current best supplier
		i = VS( cur1, "_ID1" );					// supplier ID
		cur2 = SHOOKS( cur );					// own entry on supplier list
		cur3 = cur;								// best supplier brochure
	}
}

// if supplier is found, simply update it, if not, draw a random one
if ( cur2 != NULL && cur3 != NULL )
	WRITES( cur2, "__tSelE", T );				// update selection time
else											// no brochure received
{
	cur1 = RNDDRAWS( CAPSECL2, "Firm1", "_AtauLP" );// try draw new good supplier
	i = VS( cur1, "_ID1" );

	// create the brochure/client interconnected objects
	cur3 = send_brochure( cur1, THIS );
}

WRITE_HOOK( SUPPL, cur3 );						// pointer to current brochure

RESULT( i )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_CIe" )
/*
Canceled investment of firm in energy sector
Updates '_EIe', '_SIe', '_NWe', '__ICge', '__Kge'
*/

V( "_supplierE" );								// ensure supplier is selected
cur = HOOK( SUPPL );							// pointer to current supplier
VS( PARENTS( SHOOKS( cur ) ), "_Q1e" );			// make sure supplier produced

v[1] = VS( SHOOKS( cur ), "__nCanE" );			// canceled machine number
k = VS( SHOOKS( cur ), "__tOrdE" );				// time of canceled order

if ( k == T && v[1] > 0 )
{
	v[0] = v[1] * VS( HOOK( TOPVINT ), "__mGE" );// canceled investment/capacity
	v[2] = V( "_EIe" );							// initial capacity to expand

	if ( v[2] < v[0] )							// no space for expansion?
	{
		WRITE( "_EIe", 0 );						// reset expansion investment
		INCR( "_SIe", - v[0] + v[2] );			// remaining to substitution
	}
	else
		INCR( "_EIe", - v[0] );					// shrink expansion investment

	v[7] = v[1] * VS( PARENTS( SHOOKS( cur ) ), "_p1" );// paid machine value

	if ( INCRS( HOOK( TOPVINT ), "__Kge", - v[0] ) < 1 )// full cancel?
	{
		DELETE( HOOK( TOPVINT ) );				// remove green plant
		WRITE_HOOK( TOPVINT, NULL );
	}
	else
		INCRS( HOOK( TOPVINT ), "__ICge", - v[7] );// update plant investment

	update_depo( THIS, v[7], true );			// recover paid machines value
}
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "_Ce" )
/*
Operational costs of energy producer
*/
RESULT( V( "_Df" ) * VS( PARENT, "pF" ) + V( "_We" ) +
		V( "_EmE" ) * VS( PARENT, "trCO2e" ) )


EQUATION( "_Df" )
/*
Demand (physical units) of fossil fuel by energy producer
*/
V( "_allocE" );									// ensure generation is assigned
RESULT( SUM( "__Df" ) )


EQUATION( "_EmE" )
/*
CO2 (carbon) emissions of energy producer
*/
V( "_allocE" );									// ensure generation is assigned
RESULT( SUM( "__EmDE" ) )


EQUATION( "_IeNom" )
/*
Investment (nominal/currency terms) of firm in energy sector
*/

V( "_CIe" );									// ensure capital is deployed

cur = HOOK( TOPVINT );							// last capital vintage

if ( cur != NULL && VS( cur, "__tGE" ) == T )	// capital deployed in period?
	v[0] = VS( cur, "__ICge" );
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "_JOe" )
/*
Open job positions of energy producer
*/
RESULT( max( V( "_LeD" ) - VL( "_Le", 1 ), 0 ) )


EQUATION( "_Ke" )
/*
Total generation capacity of power plants of energy producer
*/
RESULT( V( "_Kde" ) + V( "_Kge" ) )


EQUATION( "_KeNom" )
/*
Capital stock (in historic money terms) of energy producer
*/
V( "_EIe" );									// ensure capital is deployed
RESULT( SUM( "__ICge" ) )


EQUATION( "_Kde" )
/*
Total generation capacity of dirty power plants of energy producer
*/
V( "_EIe" );									// ensure capital is deployed
RESULT( SUM_CND( "__Kde", "__RSde", "==", 0 ) )	// sum non-deprecated capacity


EQUATION( "_Kge" )
/*
Total generation capacity of green power plants of energy producer
*/
V( "_EIe" );									// ensure capital is deployed
RESULT( SUM_CND( "__Kge", "__RSge", "==", 0 ) )	// sum non-deprecated capacity


EQUATION( "_Le" )
/*
Labor employed by energy producer
Includes R&D labor
*/

v[1] = VS( PARENT, "Le" );						// total labor in energy sector
v[2] = VS( PARENT, "LeD" );						// total labor demand in en. sec.
v[3] = VS( PARENT, "LeRD" );					// R&D labor in energy sector
v[4] = VS( PARENT, "LeDrd" );					// R&D labor demand in en. sec.
v[5] = V( "_LeD" );								// firm total labor demand
v[6] = V( "_LeRD" );							// firm R&D labor
v[7] = V( "_LeDrd" );							// firm R&D labor demand

// total workers in firm after possible shortage of workers
RESULT( min( v[5], v[6] + ( v[2] > v[4] ? ( v[5] - v[7] ) * ( v[1] - v[3] ) /
										  ( v[2] - v[4] ) : 0 ) ) )


EQUATION( "_LeD" )
/*
Labor demand of energy producer
Includes R&D labor
*/
RESULT( V( "_LeDrd" ) + SUM( "__LdeD" ) + SUM( "__LgeD" ) )


EQUATION( "_LeDrd" )
/*
R&D labor demand of energy producer
*/
RESULT( V( "_RDe" ) / VS( LABSUPL2, "w" ) )


EQUATION( "_LeRD" )
/*
R&D labor employed by energy producer
*/
v[1] = VS( PARENT, "LeDrd" );
RESULT( v[1] > 0 ? V( "_LeDrd" ) * VS( PARENT, "LeRD" ) / v[1] : 0 )


EQUATION( "_Qe" )
/*
Total generation of energy producer
*/
RESULT( V( "_Qde" ) + V( "_Qge" ) )


EQUATION( "_Qde" )
/*
Total generation of dirty plants of energy producer
*/
V( "_allocE" );									// ensure dirty usage is comput.
RESULT( SUM( "__Qde" ) )


EQUATION( "_Qge" )
/*
Total generation of green plants of energy producer
*/
V( "_allocE" );									// ensure green usage is comput.
RESULT( SUM( "__Qge" ) )


EQUATION( "_PiE" )
/*
Profit (before taxes) of energy producer
*/

v[1] = V( "_Se" ) - V( "_Ce" );					// gross operating margin
v[2] = VS( FINSECL2, "rD" ) * VL( "_NWe", 1 );	// financial income
v[3] = V( "_iE" );								// financial expense

RESULT( v[1] + v[2] - v[3] )					// firm profits before taxes


EQUATION( "_SIeD" )
/*
Desired substitution investment (in capacity terms) of energy producer
*/
RESULT( V( "_SIgeD" ) + V( "_SIdeD" ) )


EQUATION( "_SIdeD" )
/*
Desired substitution investment (in capacity terms) in dirty energy
of energy producer
*/
RESULT( SUM( "__RSde" ) )


EQUATION( "_SIgeD" )
/*
Desired substitution investment (in capacity terms) in green energy
of energy producer
*/
RESULT( SUM( "__RSge" ) )


EQUATION( "_Se" )
/*
Sales of energy producer
*/
RESULT( VL( "_pE", 1 ) * V( "_Qe" ) )


EQUATION( "_We" )
/*
Total wages paid by energy producer
*/
RESULT( V( "_Le" ) * VS( LABSUPL2, "w" ) )


EQUATION( "_fE" )
/*
Market share of energy producer
*/
v[1] = VS( PARENT, "De" );
RESULT( v[1] > 0 ? V( "_De" ) / v[1] : CURRENT )


EQUATION( "_fKge" )
/*
Share of green energy power plants in installed generation capacity
of energy producer
*/
v[1] = V( "_Kge" );
RESULT( v[1] > 0 ? v[1] / ( v[1] + V( "_Kde" ) ) : 0 )


EQUATION( "_iE" )
/*
Interest paid by energy producer
*/
RESULT( VL( "_DebE", 1 ) * VS( FINSECL2, "rDeb" ) *
		( 1 + ( VL( "_qcE", 1 ) - 1 ) * VS( FINSECL2, "kConst" ) ) )


EQUATION( "_iDe" )
/*
Interest received from deposits by firm in energy sector
*/
RESULT( VL( "_NWe", 1 ) * VLS( FINSECL2, "rD", 1 ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "_CSeA" )
/*
Bank credit supply available (new debt) to energy producer
Function called multiple times in single time step
*/

v[1] = V( "_DebE" );							// current firm debt
v[2] = V( "_DebEmax" );							// maximum prudential credit

if ( v[2] > v[1] )								// more credit possible?
{
	v[0] = v[2] - v[1];							// potential free credit

	cur = HOOK( BANK );							// firm's bank
	v[3] = VS( cur, "_TCeFree" );				// bank's available credit

	if ( v[3] > -0.1 )							// credit limit active
		v[0] = min( v[0], v[3] );				// take just what is possible
}
else
	v[0] = 0;									// no credit available

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_CDe", "" )
/*
Credit demand for energy producer
Updated in '_DebEmax', '_EIe', '_TaxE'
*/

EQUATION_DUMMY( "_CDeC", "" )
/*
Credit demand constraint for energy producer
Updated in '_DebEmax', '_EIe', '_TaxE'
*/

EQUATION_DUMMY( "_CSe", "" )
/*
Credit supplied to energy producer
Updated in '_DebEmax', '_EIe', '_TaxE'
*/

EQUATION_DUMMY( "_De", "" )
/*
Demand of energy of energy producer
Updated in 'De'
*/

EQUATION_DUMMY( "_DebE", "" )
/*
Stock of bank debt of energy producer
Updated in '_EIe', '_TaxE'
*/

EQUATION_DUMMY( "_NWe", "" )
/*
Net worth of energy producer
Updated in '_EIe', '_TaxE'
*/

EQUATION_DUMMY( "_emTauDE", "" )
/*
Emissions of a new dirty energy power plant of energy producer
Updated in '_AtauDE'
*/

EQUATION_DUMMY( "_innDE", "_AtauDE" )
/*
Innovation success (1) or failure (0) for dirty energy innovation
of energy producer
Updated in '_AtauDE'
*/

EQUATION_DUMMY( "_innGE", "_ICtauGE" )
/*
Innovation success (1) or failure (0) for green energy innovation
of energy producer
Updated in '_ICtauGE'
*/

EQUATION_DUMMY( "_qcE", "cScores" )
/*
Credit class of energy producer (1,2,3,4)
Updated in 'cScores'
*/
