/******************************************************************************

	ENERGY OBJECT EQUATIONS
	-----------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the Energy sector object in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "CeEq" )
/*
Cost (revenue) of energy price equalization supported by government
*/
RESULT( V( "Se" ) - V( "De" ) * V( "pE" ) )


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
		if ( v[2] > 0 )
			v[3] = VS( cur, "_QeO" ) / v[2];	// producer capacity share
		else
			v[3] = 1 / V( "Fe" );				// fair share

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


EQUATION( "GrdE" )
/*
Government R&D subsidy to energy firms
*/

std::multimap < double, object * > candidates;	// ordered set of candidate firms
std::multimap < double, object * >::reverse_iterator rit;

i = V( "flagIndPolicy" );						// industrial policies in place
j = V( "flagEnClim" );							// energy sector active?

if ( SUBS_CAP_RD( i ) && SUBS_ENE_RD( i ) && j == 1 )// split budget?
{
	v[1] = V( "RDe" );
	v[2] = v[1] / ( VS( CAPSECL1, "RD1" ) + v[1] );// share to energy firms
}
else
	if ( SUBS_ENE_RD( i ) && j == 1 )
		v[2] = 1;								// all to energy firms
	else
		END_EQUATION( 0 );						// policy inactive

v[3] = v[4] = V( "gammaRD" ) * VL( "GDPnom", 1 ) * v[2];// planned policy budget
v[5] = V( "phiRD" );							// share of R&D to subsidize

if ( v[3] > 0 )									// allocate subsidies energy
{
	CYCLE( cur, "FirmE" )						// create ordered set
		candidates.emplace( std::make_pair( 1 / VS( cur, "_ICtauGE" ), cur ) );

	for ( rit = candidates.rbegin( ); rit != candidates.rend( ) && v[4] > 0;
		  ++rit )
	{
		v[6] = min( v[5] * VS( rit->second, "_RDe" ), v[4] );// possible subsidy
		v[4] -= v[6];
		WRITES( rit->second, "_GrdE", v[6] );	// account subsidy
	}
}

RESULT( v[3] - v[4] )							// budget less unallocated


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
RESULT( log( max( VL( "NWe", 1 ), 0 ) + 1 ) -
		log( VL( "DebE", 1 ) + VL( "DebGE", 1 ) + 1 ) )


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
int Tcon = V( "Tcon" );							// plant construction time

boolVecT quit( Fe, false );						// vector of firms' quit status

WRITE( "cEntryE", 0 );							// reset exit/entry accumulators
WRITE( "cExitE", 0 );

// mark bankrupt and market-share-irrelevant firms to quit the market
h = Fe;											// initial number of firms
v[1] = v[3] = i = k = 0;						// accum., counters, registers
CYCLE( cur, "FirmE" )
{
	j = VS( cur, "_tEent" );					// time of entry
	v[4] = VS( cur, "_NWe" );					// current net wealth

	if ( ( T > j + Tcon && v[4] < 0 ) || T >= j + Tcon + nE )// bankrupt/incumb.?
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
CYCLE( cur, "FirmE" )
{
	if ( quit[ i ] )
	{
		if ( h > 0 || i != k )					// firm must exit?
		{
			++j;								// count exits
			if ( VS( cur, "_NWe" ) < 0 )		// count bankruptcies
				++v[6];

			CFUNS( cur, exit_firm );			// del obj & collect liq. value
		}
		else
			if ( h == 0 && i == k )				// best firm must get new equity
			{
				// new equity required
				v[1] += v[7] = NWe0u + VS( cur, "_DebE" ) + VS( cur, "_DebGE" ) -
							   VS( cur, "_NWe" );

				WRITES( cur, "_DebE", 0 );		// reset debt
				CYCLES( cur, cur1, "Green" )	// reset project finance debt
				{
					WRITES( cur1, "__DebGE", 0 );
					WRITES( cur1, "__amtGE", 0 );
				}
				INCRS( cur, "_EqE", v[7] );		// add new equity
				INCRS( cur, "_NWe", v[7] );
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

CFUN( entry_firmE, k, false );					// add entrant-firm objects

v[0] = k - j;									// net number of entrants
INCR( "Fe", v[0] );								// update the number of firms
INCR( "cEntryE", v[1] );						// add cost of additional equity
WRITE( "exitE", ( double ) j / Fe );
WRITE( "entryE", ( double ) k / Fe );
WRITES( ENESTAL1, "exitEfail", v[6] / Fe );
RECALCS( FINSECL1, "BadDebE" );					// update bad debt after exits
RECALCS( FINSECL1, "BadDebGE" );

V( "fErescale" );								// redistribute entrant m.s.
V( "firmEmaps" );								// update firm mapping vectors

RESULT( v[0] )


EQUATION( "pE" )
/*
Price of energy
*/
RESULT( VS( PARENT, "flagEnClim" ) > 0 ? WHTAVEL( "_pE", "_fE", 1 ) : 0 )


EQUATION( "pF" )
/*
Price of fossil fuel
*/
v[1] = VS( PARENT, "mLim" );					// growth rate limit
v[2] = V( "Tplan" );							// moving average period
RESULT( CURRENT * ( 1 + V( "upsilonF" ) *
					( CFUN( mov_avg_bound, "pE", v[1], v[2], 1 ) +
					  CFUN( mov_avg_bound, "Df", v[1], v[2], 1 ) ) ) )


EQUATION( "trCO2e" )
/*
Tax fee on CO2 (carbon) emissions of non-energy firms
*/
RESULT( ( 1 + VS( PARENT, "deltaTC" ) ) * CURRENT )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "Ae" )
/*
Labor productivity of energy sector
*/
v[1] = V( "Le" );
RESULT( v[1] > 0 ? V( "Qe" ) / v[1] : VL( "AeMavg", 1 ) )


EQUATION( "AeMavg" )
/*
Moving average of labor productivity in energy sector
*/
RESULT( MAVE( "Ae", V( "Tplan" ) ) )


EQUATION( "Ce" )
/*
Total generation costs of energy sector
*/
RESULT( SUM( "_Ce" )  )


EQUATION( "CIe" )
/*
Total canceled investment in energy sector
*/
RESULT( SUM( "_CIe" ) )


EQUATION( "DeE" )
/*
Demand expectation of energy sector
*/
RESULT( SUM( "_DeE" ) )


EQUATION( "DebE" )
/*
Total regular debt of energy sector
*/
V( "TaxE" );									// ensure debt is updated
RESULT( SUM( "_DebE" ) )


EQUATION( "DebGE" )
/*
Total debt in green energy project finance of energy sector
*/
RESULT( SUM( "_DebGE" ) )


EQUATION( "Df" )
/*
Demand (physical units) of fossil fuel by energy sector
*/
RESULT( SUM( "_Df" ) )


EQUATION( "DivE" )
/*
Total dividends paid by energy sector
*/
RESULT( SUM( "_DivE" ) )


EQUATION( "EIe" )
/*
Total expansion investment (in capacity terms) of energy sector
*/
RESULT( SUM( "_EIe" )  )


EQUATION( "EIeD" )
/*
Total desired expansion investment (in capacity terms) of energy sector
*/
RESULT( SUM( "_EIeD" )  )


EQUATION( "EmE" )
/*
Total CO2 (carbon) emissions of energy sector
*/
RESULT( SUM( "_EmE" ) )


EQUATION( "EqE" )
/*
Equity hold by workers/households from firms in energy sector
*/
RESULT( SUM( "_EqE" ) )


EQUATION( "Fe" )
/*
Number of firms in energy sector
*/
RESULT( COUNT( "FirmE" ) )


EQUATION( "IeNom" )
/*
Total investment (nominal/currency terms) of energy sector
*/
RESULT( SUM( "_IeNom" ) )


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


EQUATION( "KeNom" )
/*
Total capital (nominal/money terms) in energy sector
*/
RESULT( SUM( "_KeNom" ) )


EQUATION( "Kde" )
/*
Total generation capacity of dirty power plants in energy sector
*/
RESULT( SUM( "_Kde" ) )


EQUATION( "Kge" )
/*
Total generation capacity of green power plants in energy sector
*/
RESULT( SUM( "_Kge" ) )


EQUATION( "LeD" )
/*
Total desired labor in energy sector
*/
RESULT( SUM( "_LeD" ) )


EQUATION( "LeDrd" )
/*
Total desired R&D labor in energy sector
*/
RESULT( SUM( "_LeDrd" ) )


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


EQUATION( "Qge" )
/*
Total green energy generation of energy sector
*/
V( "De" );										// ensure demand is allocated
RESULT( SUM( "_Qge" ) )


EQUATION( "QeO" )
/*
Total generation offered by energy sector
*/
RESULT( SUM( "_QeO" ) )


EQUATION( "RDe" )
/*
R&D expenditure of energy sector
*/
RESULT( SUM( "_RDe" )  )


EQUATION( "SIe" )
/*
Total substitution investment (in capacity terms) of energy sector
*/
RESULT( SUM( "_SIe" )  )


EQUATION( "SIeD" )
/*
Total desired substitution investment (in capacity terms) of energy sector
*/
RESULT( SUM( "_SIeD" )	)


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


EQUATION( "dDeMavg" )
/*
Moving average of growth rate of energy demand
*/
RESULT( CFUN( mov_avg_bound, "De", VS( PARENT, "mLim" ), V( "Tplan" ) ) )


EQUATION( "iDe" )
/*
Interest received from deposits by energy sector
*/
RESULT( SUM( "_iDe" ) )


EQUATION( "iE" )
/*
Interest on regular debt paid by energy sector
*/
RESULT( SUM( "_iE" ) )


EQUATION( "iGE" )
/*
Interest on green energy project finance debt paid by energy sector
*/
RESULT( SUM( "_iGE" ) )


EQUATION( "pEmavg" )
/*
Moving average of energy price
*/
RESULT( MAVE( "pE", V( "Tplan" ) ) )


EQUATION( "uE" )
/*
Green power plant utilization in energy sector
*/
v[1] = V( "Ke" );								// total green power capacity
RESULT( v[1] > 0 ? V( "Qe" ) / v[1] : 0 )


EQUATION( "uEmavg" )
/*
Moving average of green power plant utilization in energy sector
*/
RESULT( MAVE( "uE", V( "Tplan" ) ) )


EQUATION( "wEmavg" )
/*
Moving average of wage in energy sector
*/
RESULT( MAVES( LABSUPL1, "w", V( "Tplan" ) ) )


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


EQUATION( "firmEmaps" )
/*
Updates the static maps of firms in energy sector
Only to be called if firm objects in energy sector are created or destroyed
*/

// clear vectors
EXEC_EXTS( PARENT, countryE, firmEmap, clear );
EXEC_EXTS( PARENT, countryE, firmEptr, clear );

i = 0;											// firm index in vector
CYCLE( cur, "FirmE" )							// do for all firms in sector 2
{
	EXEC_EXTS( PARENT, countryE, firmEptr, push_back, cur );// pointer to firm
	EXEC_EXTS( PARENT, countryE, firmEmap, insert, // save in firm's map
			   firmPairT( ( int ) VS( cur, "_IDe" ), cur ) );

	++i;
}

RESULT( i )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "cEntryE", "" )
/*
Cost (new equity) of firm entries in energy sector
Updated in 'entryEexit'
*/

EQUATION_DUMMY( "cExitE", "" )
/*
Credits (returned equity) from firm exits in energy sector
Updated in 'entryEexit'
*/

EQUATION_DUMMY( "entryE", "entryEexit" )
/*
Rate of entering firms in energy sector
Updated in 'entryEexit'
*/

EQUATION_DUMMY( "exitE", "entryEexit" )
/*
Rate of exiting firms in energy sector
Updated in 'entryEexit'
*/
