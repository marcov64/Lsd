/******************************************************************************

	ENERGY OBJECT EQUATIONS
	-----------------------

	Equations that are specific to the Energy sector object in the 
	K+S LSD model are coded below.
 
 ******************************************************************************/
 
#define TECLIMIT		0.7						// thermal efficiency Carnot limit

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "AtauDE" )
/*
Thermal efficiency of a new dirty energy power plant
Also updates 'emTauDE', 'innDE'
*/

v[0] = CURRENT;									// current efficiency
v[1] = V( "emTauDE" );							// current emissions

// share of R&D expenditure among dirty and green innovation
if ( VS( PARENT, "flagEnRDshare" ) == 0 )		// exogenous dirty/green share?
	v[2] = V( "xiDE" );							// fixed value
else
{
	v[3] = V( "fGE0" );							// R&D share floor
	v[2] = max( min( 1 - VL( "fKge", 1 ), 1 - v[3] ), v[3] );// bounded R&D share
}

// normalized worker-equivalent on R&D (for ever-growing energy prices)
v[3] = ( VL( "RDe", 1 ) / VLS( LABSUPL1, "wReal", 1 ) ) * VS( LABSUPL1, "Ls0" ) / 
	   VLS( LABSUPL1, "Ls", 1 );

// dirty energy innovation process (success probability)
v[4] = 1 - exp( - V( "zetaDE" ) * v[2] * v[3] );

if ( bernoulli( v[4] ) )						// dirty innovation succeeded?
{
	v[5] = V( "xEinfTE" );						// lower support th. eff. innov.
	v[6] = V( "xEsupTE" );						// upper support th. eff. innov.
	v[7] = V( "xEinfEm" );						// lower support emissions innov.
	v[8] = V( "xEsupEm" );						// upper support emissions innov.
	v[9] = V( "alphaE" );						// beta distr. alpha parameter
	v[10] = V( "betaE" );						// beta distr. beta parameter
	
	// draw new innovation values for thermal efficiency and emissions
	v[11] = min( v[0] * ( 1 + v[5] + beta( v[9], v[10] ) * ( v[6] - v[5] ) ), 
				 TECLIMIT );					// consider Carnot limit eff.
	v[12] = v[1] * ( 1 - v[7] - beta( v[9], v[10] ) * ( v[8] - v[7] ) );
	
	// select best between the two options (current/innovation) unit costs
	v[13] = V( "pF" );							// fossil fuel price
	v[14] = V( "trCO2e" );						// carbon tax on energy generat.
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

WRITE( "emTauDE", v[1] );
WRITE( "innDE", v[15] );

RESULT( v[0] )


EQUATION( "Ce" )
/*
Total generation costs of energy producer
Also updates '_Qde'
*/

V( "IeNom" );									// ensure investment is done

v[1] = V( "De" ) - V( "Kge" );					// dirty generation demand

if ( v[1] <= 0 )
	END_EQUATION( 0 );							// no generation costs

v[2] = max( V( "Kde" ) - v[1], 0 );				// capacity not to use

// allocate generation among plants, favoring cheaper (newer) plants
v[0] = 0;										// cost accumulator
CYCLE( cur, "Dirty" )							// turn on required dirty plants
{
	v[3] = VS( cur, "_Kde" );					// plant notional capacity
	
	if( v[2] >= v[3] )							// no use for this plant?
	{
		v[4] = v[5] = 0;						// no generation/cost for plant
		v[2] -= v[3];							// less capacity not to use
	}
	else										// use plant (full or partial)
	{
		v[4] = v[3] - v[2];						// generate what is needed
		v[5] = v[4] * VS( cur, "_cDE" );		// generation cost
		v[2] = 0;								// no more plants not to use
	}
	
	WRITES( cur, "_Qde", v[4] );				// generation for plant
	v[0] += v[5];								// accumulated generation costs
}

RESULT( v[0] )


EQUATION( "EIe" )
/*
Expansion investment (in capacity terms) of energy producer
*/
RESULT( max( V( "De" ) - VL( "Ke", 1 ), 0 ) )


EQUATION( "EIeNom" )
/*
Expansion investment (in money terms) of energy producer
Chooses between technologies and deploys new plants
Also updates 'SIeNom'
*/

v[1] = V( "EIe" );								// investment in cap. expansion
v[2] = v[1] + V( "SIe" );						// total investment in capacity

if ( v[2] == 0 )
{
	WRITE( "SIeNom", 0 );
	END_EQUATION( 0 );							// nothing to invest
}

v[3] = V( "AtauDE" );							// dirty unit efficiency
v[4] = V( "ICtauGE" );							// green unit install cost
v[5] = V( "kappaE" );							// max capacity growth rate
v[6] = VL( "Kge", 1 );							// current green capacity
v[7] = VL( "Kde", 1 );							// current dirty capacity

// try to keep fix share until atmospheric CO2 reference time 
if ( T <= VS( CLIMATL1, "tA0" ) )
{
	v[8] = max( V( "fGE0" ) * V( "De" ) - v[6] + V( "SIgeD" ), 0 );// green new
	v[9] = v[2] - v[8];							// dirty new capacity
}
else
	if ( v[4] <= V( "bE") * V( "pF" ) / v[3] )	// are green plants cheaper?
		// if cap is disabled or new green plants capacity is under cap
		if ( v[5] == 0 || v[2] <= v[5] * v[6] )
		{
			v[8] = v[2];						// only green new plants
			v[9] = 0;							// no dirty new plants
		}
		else									// cap limits new green plants
		{
			if ( v[2] - v[5] * v[6] <= v[5] * v[7] )// dirty growth cap ok?
				v[8] = v[5] * v[6];				// invest in green to the cap
			else								// both caps can't be enforced
				v[8] = v[2] * v[6] / ( v[6] + v[7] );// keep green share
			
			v[9] = v[2] - v[8];					// dirty complements investment
		}
	else										// dirty plants are cheaper
		// if cap is disabled or new dirty plants capacity is under cap
		if ( v[5] == 0 || v[2] <= v[5] * v[7] )
		{
			v[9] = v[2];						// only dirty new plants
			v[8] = 0;							// no green new plants
		}
		else									// cap limits new dirty plants
		{
			if ( v[2] - v[5] * v[7] <= v[5] * v[6] )// green growth cap ok?
				v[9] = v[5] * v[7];				// invest in dirty to the cap
			else								// both caps can't be enforced
				v[9] = v[2] * v[7] / ( v[6] + v[7] );// keep dirty share
				
			v[8] = v[2] - v[9];					// green complements investment
		}

if ( v[8] > 0 )									// invest in new green plants?
{
	v[0] = v[4] * v[8] * v[1] / v[2];			// green expansion cost
	v[10] = v[4] * v[8] * ( 1 - v[1] / v[2] ) ;	// green substitution cost
	
	cur = ADDOBJL( "Green", T - 1 );			// create new green plant object
	WRITES( cur, "_tGE", T );					// installation time
	WRITES( cur, "_Kge", v[8] );				// plant generation capacity
	WRITES( cur, "_ICge", v[0] + v[10] );		// plant installation cost
}
else
	v[0] = v[10] = 0;							// no green plant investment

if ( v[9] > 0 )									// new dirty plants?
{
	cur = ADDOBJL( "Dirty", T - 1 );			// create new dirty plant object
	WRITES( cur, "_tDE", T );					// installation time
	WRITES( cur, "_Kde", v[9] );				// plant generation capacity
	WRITES( cur, "_Ade", v[3] );				// plant thermal efficiency
	WRITES( cur, "_emDE", V( "emTauDE" ) );		// plant emissions
}

WRITE( "SIeNom", v[10] );						// green substitution cost

RESULT( v[0] )


EQUATION( "ICtauGE" )
/*
Installation cost per unit of capacity of green energy power plant
Also updates 'innGE'
*/

v[0] = CURRENT;									// current installation cost

// share of R&D expenditure among dirty and green innovation
// even if R&D share is endogenous wait until climate warm-up period finishes
if ( VS( PARENT, "flagEnRDshare" ) == 0 || VS( CLIMATL1, "tA0" ) < T )
	v[2] = 1 - V( "xiDE" );						// fixed value
else
{
	v[1] = V( "fGE0" );							// R&D share floor
	v[2] = max( min( VL( "fKge", 1 ), 1 - v[1] ), v[1] );// bounded R&D share
}

// normalized worker-equivalent on R&D (for ever-growing energy prices)
v[3] = ( VL( "RDe", 1 ) / VLS( LABSUPL1, "wReal", 1 ) ) * VS( LABSUPL1, "Ls0" ) / 
	   VLS( LABSUPL1, "Ls", 1 );

// green energy innovation process (success probability)
v[4] = 1 - exp( - V( "zetaGE" ) * v[2] * v[3] );

if ( bernoulli( v[4] ) )						// green innovation succeeded?
{
	v[5] = V( "xEinfIC" );						// lower support inst. cost inn.
	v[6] = V( "xEsupIC" );						// upper support inst. cost inn.
	v[7] = V( "alphaE" );						// beta distr. alpha parameter
	v[8] = V( "betaE" );						// beta distr. beta parameter

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

WRITE( "innGE", v[10] );

RESULT( v[0] )


EQUATION( "RDe" )
/*
R&D expenditure of energy producer
*/
// cap expenditure to existing funds / min R&D (1 worker) if negative net wealth
RESULT( VS( PARENT, "flagEnClim" ) > 0 ?
		max( min( V( "nuE" ) * VL( "Se", 1 ), VL( "NWe", 1 ) ), 
		VS( LABSUPL1, "w" ) ) : 0 )


EQUATION( "SIe" )
/*
Substitution investment (in capacity terms) of energy producer
*/
RESULT( max( V( "SIeD" ) - max( VL( "Ke", 1 ) - V( "De" ), 0 ), 0 ) )


EQUATION( "muE" )
/*
Mark-up of energy producer
*/
RESULT( VLS( LABSUPL1, "wReal", 1 ) * V( "muE0" ) )


EQUATION( "pE" )
/*
Price of energy
*/

if ( VS( PARENT, "flagEnClim" ) == 0 )
	END_EQUATION( 0 );							// free energy

V( "Ce" );										// ensure plant allocation done

if ( V( "De" ) <= V( "Kge" ) + 0.01 )			// just green energy produced?
	v[1] = 0;									// no operating cost
else
	v[1] = MAX_CND( "_cDE", "_Qde", ">", 0 );	// higher cost among used plants

RESULT( isfinite( v[1] ) ? v[1] + V( "muE" ) : V( "muE" ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "De" )
/*
Demand of energy from firms in capital- and consumer-good sectors
*/
RESULT( VS( PARENT, "flagEnClim" ) == 0 ? 0 : VS( PARENT, "En" ) )


EQUATION( "EmE" )
/*
CO2 (carbon) emissions of energy producer
*/
V( "Ce" );										// ensure generation is assigned
RESULT( SUM( "_EmDE" ) )


EQUATION( "IeNom" )
/*
Total investment (nominal/currency terms) of energy producer
*/
RESULT( V( "EIeNom" ) + V( "SIeNom" ) )


EQUATION( "Ke" )
/*
Total generation capacity of power plants
*/
RESULT( V( "Kde" ) + V( "Kge" ) )


EQUATION( "Kde" )
/*
Total generation capacity of dirty power plants
*/
V( "IeNom" );									// assure new plants installed
RESULT( SUM( "_Kde" ) - SUM( "_RSde" ) )		// don't consider deferred scrap


EQUATION( "Kge" )
/*
Total generation capacity of green power plants
*/
V( "IeNom" );									// assure new plants installed
RESULT( SUM( "_Kge" ) - SUM( "_RSge" ) )		// don't consider deferred scrap


EQUATION( "NWe" )
/*
Net worth of energy producer
*/
RESULT( CURRENT + V( "PiE" ) - V( "IeNom" ) )


EQUATION( "Qe" )
/*
Total generation of energy producer
*/
V( "Ce" );										// ensure dirty usage is comput.
RESULT( SUM( "_Qde" ) + SUM( "_Qge" ) )


EQUATION( "PiE" )
/*
Profit of energy producer
*/
RESULT( V( "Se" ) - V( "Ce" ) - V( "TaxE" ) - V( "RDe" ) )


EQUATION( "SIeD" )
/*
Desired substitution investment (in capacity terms) of energy producer
*/
RESULT( V( "SIgeD" ) + V( "SIdeD" ) )


EQUATION( "SIdeD" )
/*
Desired substitution investment (in capacity terms) in dirty energy
*/
RESULT( SUM( "_RSde" ) )


EQUATION( "SIgeD" )
/*
Desired substitution investment (in capacity terms) in green energy
*/
RESULT( SUM( "_RSge" ) )


EQUATION( "Se" )
/*
Sales of energy producer
*/
RESULT( VL( "pE", 1 ) * V( "Qe" ) )


EQUATION( "TaxE" )
/*
Taxes paid by energy producer
*/
RESULT( V( "EmE" ) * V( "trCO2e" ) )


EQUATION( "fKge" )
/*
Share of green energy power plants in installed generation capacity
*/
v[1] = V( "Kge" );
RESULT( v[1] > 0 ? v[1] / ( v[1] + V( "Kde" ) ) : 0 )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "SIeNom", "EIeNom" )
/*
Substitution investment (in money terms) of energy producer
Updated in 'EIeNom'
*/

EQUATION_DUMMY( "emTauDE", "AtauDE" )
/*
Emissions of a new dirty energy power plant
Updated in 'AtauDE'
*/

EQUATION_DUMMY( "innDE", "AtauDE" )
/*
Innovation success (1) or failure (0) for dirty energy innovation
Updated in 'AtauDE'
*/

EQUATION_DUMMY( "innGE", "ICtauGE" )
/*
Innovation success (1) or failure (0) for green energy innovation
Updated in 'ICtauGE'
*/
