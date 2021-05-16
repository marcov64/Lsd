/******************************************************************************

	ENERGY OBJECT EQUATIONS
	-----------------------

	Equations that are specific to the Energy sector object in the 
	K+S LSD model are coded below.
 
 ******************************************************************************/
 
/*============================== KEY EQUATIONS ===============================*/

EQUATION( "De" )
/*
Demand of energy to energy sector
Allocates demand by a short-term first-price sealed-bid auction,
based on prices offered by producers and the offered generation
If demand is larger than total offered generation, excess demand is allocated
proportionally to offered capacity among producers
Updates '_De'
*/

if ( VS( PARENT, "flagEnClim" ) == 0 )
	END_EQUATION( 0 );

v[1] = VS( PARENT, "En" );						// total energy demand
v[2] = V( "QeO" );								// total offered generation

v[0] = 0;										// fulfilled demand accumulator

if ( v[1] >= v[2] )								// demand requires all capacity?
	CYCLE( cur, "FirmE" )						// no auction necessary
	{
		v[3] = VS( cur, "_QeO" ) / v[2];		// producer capacity share
		v[0] += v[4] = v[1] * v[3];				// producer supply allocation
		WRITES( cur, "_De", v[4] );
	}
else											// excess supply, auction req'd
{
	SORTL( "FirmE", "_pE", "UP", 1 );			// sort producers by incr. price

	CYCLE( cur, "FirmE" )						// allocate cheaper suppliers 1st
	{
		v[3] = VS( cur, "_QeO" );				// available (offered) generation
		
		if ( v[1] <= v[3] )						// can supply all demanded?
		{
			v[0] += v[4] = v[1];				// supply all demanded
			v[1] = 0;							// nothing else to be supplied
		}
		else
		{
			v[0] += v[4] = v[3];				// supply all available
			v[1] -= v[3];						// demand yet to be fulfilled
		}
		
		WRITES( cur, "_De", v[4] );
	}
}

RESULT( v[0] )


EQUATION( "Le" )
/*
Total labor employed in sector energy sector
*/

v[1] = VS( LABSUPL1, "Ls" );					// available labor force
v[2] = V( "LeRD" );								// R&D labor in energy sector
v[3] = v[2] + VS( CAPSECL1, "L1rd" );			// R&D labor in all sectors
v[4] = V( "LeD" );								// desired workers in en. sector
v[5] = VS( CAPSECL1, "L1d" ) + VS( CONSECL1, "L2d" );// desired workers other sec.
v[6] = V( "LeShortMax" );						// max shortage allowed in sector

v[4] = min( v[4], v[1] );						// ignore demand over total labor
v[5] = min( v[5], v[1] );

if ( v[1] - v[3] < v[4] + v[5] )				// labor shortage?
{
	v[7] = ( v[1] - v[3] ) / ( v[4] + v[5] );	// shortage factor
	
	if ( v[7] < 1 - v[6] )						// over cap?
		v[7] = 1 - v[6];						// shortage on cap
}
else
	v[7] = 1;									// no shortage

RESULT( v[2] + ( v[4] - v[2] ) * v[7] )


EQUATION( "MCe" )
/*
Market entry conditions index in energy sector
*/
RESULT( log( max( VL( "NWe", 1 ), 0 ) + 1 ) - log( VL( "DebE", 1 ) + 1 ) )


EQUATION( "entryEexit" )
/*
Net (number of) entrant firms in energy sector
Perform entry and exit of firms in the energy sector
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
Also updates 'Fe', 'cEntry', 'cExit', 'exitE', 'entryE', 'exitEfail'
*/

UPDATE;											// ensure aggregates are computed

double MCe = V( "MCe" );						// market conditions in en. sec.
double MCe_1 = VL( "MCe", 1 );					// market conditions in en. sec.
double NWe0u = V( "NWe0" );						// minimum wealth in energy sec.
double fEmin = V( "fEmin" );					// min market share in en. sec.
double nE = V( "nE" );							// market participation period
double omicron = VS( PARENT, "omicron" );		// entry sensitivity to mkt cond
double stick = VS( PARENT, "stick" );			// stickiness in number of firms
double x2inf = VS( PARENT, "x2inf" );			// entry lower distrib. support
double x2sup = VS( PARENT, "x2sup" );			// entry upper distrib. support
int Fe = V( "Fe" );								// current number of firms
int Fe0 = V( "Fe0" );							// initial number of firms
int FeMax = V( "FeMax" );						// max firms in energy sector
int FeMin = V( "FeMin" );						// min firms in energy sector

vector < bool > quit( Fe, false );				// vector of firms' quit status

// mark bankrupt and market-share-irrelevant firms to quit the market
h = Fe;											// initial number of firms
v[1] = v[2] = v[3] = i = k = 0;					// accum., counters, registers
CYCLE( cur, "FirmE" )
{
	v[4] = VS( cur, "_NWe" );					// current net wealth
	
	if ( v[4] < 0 || T >= VS( cur, "_tEent" ) + nE )// bankrupt or incumbent?
	{
		for ( v[5] = j = 0; j < nE; ++j )
			v[5] += VLS( cur, "_fE", j ) / nE;	// nE periods market share
		
		if ( v[4] < 0 || v[5] <= fEmin )
		{
			quit[ i ] = true;					// mark for likely exit
			--h;								// one less firm
			
			if ( v[5] > v[3] )					// best firm so far?
			{
				k = i;							// save firm index
				v[3] = v[5];					// and market share
			}
		}
	}
	
	++i;
}	

// quit candidate firms exit, except the best one if all going to quit
v[6] = i = j = 0;								// firm counters
CYCLE_SAFE( cur, "FirmE" )
{
	if ( quit[ i ] )
	{
		if ( h > 0 || i != k )					// firm must exit?
		{
			++j;								// count exits
			if ( VS( cur, "_NWe" ) < 0 )		// count bankruptcies
				++v[6];
			
			// account liquidation credit due to public, if any
			v[2] += exit_firmE( var, cur );		// del obj & collect liq. value
		}
		else
			if ( h == 0 && i == k )				// best firm must get new equity
			{
				// new equity required
				v[7] = NWe0u + VS( cur, "_DebE" ) - VS( cur, "_NWe" );
				v[1] += v[7];					// accumulate "entry" equity cost
				
				WRITES( cur, "_DebE", 0 );		// reset debt
				INCRS( cur, "_NWe", v[7] );		// add new equity
			}
	}

	++i;
}

V( "fErescale" );								// redistribute exiting m.s.

// compute the potential number of entrants
v[8] = ( MCe_1 == 0 ) ? 0 : MCe / MCe_1 - 1;	// change in market conditions

k = max( 0, round( Fe * ( ( 1 - omicron ) * uniform( x2inf, x2sup ) + 
						  omicron * min( max( v[8], x2inf ), x2sup ) ) ) );
				 
// apply return-to-the-average stickiness random shock to the number of entrants
k -= min( RND * stick * ( ( double ) ( Fe - j ) / Fe0 - 1 ) * Fe0, k );

// ensure limits are enforced to the number of entrants
if ( Fe - j + k < FeMin )
	k = FeMin - Fe + j;

if ( Fe + k > FeMax )
	k = FeMax - Fe + j;

v[0] = k - j;									// net number of entrants
v[1] += entry_firmE( var, THIS, k, false );		// add entrant-firm objects

i = INCR( "Fe", v[0] );							// update the number of firms
INCRS( PARENT, "cEntry", v[1] );				// account equity cost of entry
INCRS( PARENT, "cExit", v[2] );					// account exit credits
WRITES( ENESTAL1, "exitE", ( double ) j / Fe );
WRITES( ENESTAL1, "entryE", ( double ) k / Fe );
WRITES( ENESTAL1, "exitEfail", v[6] / Fe );

V( "fErescale" );								// redistribute entrant m.s.

RESULT( v[0] )


EQUATION( "pE" )
/*
Price of energy
*/
RESULT( VS( PARENT, "flagEnClim" ) != 0 ? WHTAVEL( "_pE", "_fE", 1 ) : 0 )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "Ce" )
/*
Total generation costs of energy sector
*/
V( "Cf" );										// ensure costs are updated
RESULT( SUM( "_Ce" )  )


EQUATION( "Cf" )
/*
Total fuel costs of energy sector
*/
V( "TaxE" );									// ensure accounting updated
RESULT( SUM( "_Cf" )  )


EQUATION( "SfAcc" )
/*
Accumulated fuel sales to energy sector
*/
RESULT( CURRENT + V( "Cf" )  )


EQUATION( "DebE" )
/*
Total debt of energy sector
*/
V( "TaxE" );									// ensure debt is updated
RESULT( SUM( "_DebE" ) )


EQUATION( "DivE" )
/*
Total dividends paid by energy sector
*/
V( "TaxE" );									// ensure net worth is updated
RESULT( SUM( "_DivE" ) )


EQUATION( "EIe" )
/*
Total expansion investment (in capacity terms) of energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_EIe" )  )


EQUATION( "EmE" )
/*
Total CO2 (carbon) emissions of energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_EmE" ) )


EQUATION( "Fe" )
/*
Number of firms in energy sector
*/
RESULT( COUNT( "FirmE" ) )


EQUATION( "Ie" )
/*
Total investment (in capacity terms) of energy sector
*/
RESULT( V( "EIe" ) + V( "SIe" ) )


EQUATION( "JOe" )
/*
Total open job positions in energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_JOe" ) )


EQUATION( "Ke" )
/*
Total generation capacity of power plants in energy sector
*/
RESULT( V( "Kde" ) + V( "Kge" ) )


EQUATION( "Kde" )
/*
Total generation capacity of dirty power plants in energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_Kde" ) )


EQUATION( "Kge" )
/*
Total generation capacity of green power plants in energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_Kge" ) )


EQUATION( "LeD" )
/*
Total desired labor in energy sector
*/
RESULT( SUML( "_LeD", 1 ) )


EQUATION( "LeDrd" )
/*
Total desired R&D labor in energy sector
*/
RESULT( SUML( "_LeDrd", 1 ) )


EQUATION( "LeRD" )
/*
Total R&D labor employed in energy sector
*/
RESULT( min( V( "LeDrd" ), VS( LABSUPL1, "Ls" ) * V( "LeRDmax" ) ) )


EQUATION( "NWe" )
/*
Total net worth of energy sector
*/
V( "TaxE" );									// ensure net worth is updated
RESULT( SUM( "_NWe" ) )


EQUATION( "PiE" )
/*
Total profits of energy sector
*/
V( "TaxE" );									// ensure profits are updated
RESULT( SUM( "_PiE" ) )


EQUATION( "Qe" )
/*
Total generation of energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_Qe" ) )


EQUATION( "QeO" )
/*
Total generation offered by energy sector
*/
RESULT( SUM( "_QeO" ) )


EQUATION( "SIe" )
/*
Total substitution investment (in capacity terms) of energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_SIe" )  )


EQUATION( "SIeD" )
/*
Total desired substitution investment (in capacity terms) of energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_SIeD" )  )


EQUATION( "Se" )
/*
Total sales of energy sector
*/
V( "TaxE" );									// ensure accounting is updated
RESULT( SUM( "_Se" ) )	


EQUATION( "TaxE" )
/*
Total taxes paid by energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_TaxE" ) )


EQUATION( "We" )
/*
Total wages paid by energy sector
*/
V( "TaxE" );									// ensure accounting is updated
RESULT( SUM( "_We" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "fErescale" )
/*
Rescale market shares in energy sector to ensure adding to 1
To be called after market shares are changed in '_fE' and 'entryEexit'
*/

v[1] = SUM( "_fE" );							// add-up market shares

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator

if ( v[1] > 0 )									// production ok?
	CYCLE( cur, "FirmE" )						// rescale to add-up to 1
	{
		v[0] += v[2] = VS( cur, "_fE" ) / v[1];	// rescaled market share
		WRITES( cur, "_fE", v[2] );				// save updated m.s.
	}
else
{
	v[2] = 1 / COUNT( "FirmE" );				// firm fair share
	
	CYCLE( cur, "FirmE" )						// rescale to add-up to 1
	{
		v[0] += v[2];
		WRITES( cur, "_fE", v[2] );
	}
}

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/
