/******************************************************************************

	WORLD (ROOT) OBJECT EQUATIONS
	-----------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the World (Root) object in
	the K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "XcW" )
/*
Total exports (in int'l currency) of consumption goods worldwide
Also updates 'McD', 'XcD'
*/

V( "fXCrescale" );								// ensure export shares add to 1

v[2] = VL( "trMCavgW", 1 );						// expected average import duty

// create and fill temporary export supply and import demand vectors
k = COUNT( "Country" );							// number of countries
dblVecT McD( k ), XcD( k ), e( k ), fCMw( k ), fXC( k );

v[1] = j = 0;
CYCLE( cur, "Country" )
{
	if ( VS( cur, "flagTradeC" ) > 0 )			// consumption-good trade?
	{
		e[ j ] = VS( cur, "e" );				// country exchange rate
		McD[ j ] = VS( cur, "Cd" ) / e[ j ];	// country desired $ consumption
		XcD[ j ] = SUMS( cur, "S2d" ) / e[ j ];	// country desired gross $ supply
		fXC[ j ] = VS( cur, "fXC" );			// country exports market share

		v[3] = ( 1 - VS( cur, "fMC" ) ) * McD[ j ];// desired domestic demand

		if ( v[3] < XcD[ j ] )					// can fulfill domestic demand?
		{
			McD[ j ] -= v[3];					// supply the desired demand
			XcD[ j ] -= v[3];					// available supply to export
		}
		else
		{
			McD[ j ] -= XcD[ j ];				// supply all available
			fXC[ j ] = XcD[ j ] = 0;			// nothing left to export
		}

		XcD[ j ] /= ( 1 - v[2] ) * ( 1 - AVES( cur, "trX2" ) );// add exp. duty
		v[1] += McD[ j ];						// imports eff. global demand
	}
	else
		McD[ j ] = fXC[ j ] = 0;

	WRITES( cur, "McD", 0 );					// country imports accumulator
	WRITES( cur, "XcD", 0 );					// country exports accumulator
	++j;
}

for ( j = 0; j < k; ++j )						// compute import share vector
	fCMw[ j ] = v[1] > 0 ? McD[ j ] / v[1] : 0;

// cycle through countries until all exports are done or no more imports needed
v[0] = 0;										// fulfilled exports accumulator
for ( v[4] = v[1]; v[4] > 0.01; v[4] = v[5] )
{
	v[5] = v[4];								// remaining unallocated demand
	v[6] = j = h = 0;							// export shares yet unallocated
	CYCLE( cur, "Country" )						// each country as exporter
	{
		if ( fXC[ j ] > 0 )						// country has share to export?
		{
			if ( XcD[ j ] > 0 )					// product to export?
			{
				v[7] = v[4] * fXC[ j ];			// country exports allocation

				i = 0;							// import shares yet unallocated
				CYCLE( cur1, "Country" )		// try each country as importer
				{
					if ( cur1 != cur && McD[ i ] > 0 )// not itself and imports?
					{
						// export des. share bounded to imp. demand & exp. supply
						v[8] = min( min( v[7] * fCMw[ i ], McD[ i ] ), XcD[ j ] );

						v[0] += v[8];			// accumulate to total exports
						v[5] -= v[8];			// discount from desired imports
						McD[ i ] -= v[8];		// reduce pending imports
						XcD[ j ] -= v[8];		// make exports unavailable

						INCRS( cur1, "McD", v[8] * e[ i ] );// update c. imports
						INCRS( cur, "XcD", v[8] * e[ j ] );// update c. export

						++h;					// count transactions in cycle
					}

					++i;
				}
			}

			if ( XcD[ j ] > 0 )					// more exports possible?
				v[6] += fXC[ j ];				// save export share to allocate
			else
				fXC[ j ] = 0;					// no export share anymore
		}

		++j;
	}

	if ( v[6] > 0 && h > 0 )					// exports possible & transactions?
		for ( j = 0; j < k; ++j )
			fXC[ j ] /= v[6];					// rescale remaining exporters
	else
		break;									// nothing else to export
}

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "EcAvgW" )
/*
Average competitiveness of countries in exporting consumption goods worldwide
*/

v[0] = 0;										// accumulator
CYCLE( cur, "Country" )
	v[0] += VLS( cur, "fXC", 1 ) * VS( cur, "Ec" );

RESULT( v[0] > 0 ? v[0] : 0.5 )


EQUATION( "FkW" )
/*
Number of firms in capital-good sectors worldwide
*/

v[0] = 0;										// firm number accumulator
CYCLE( cur, "Country" )							// in all countries
	v[0] += SUMS( cur, "F1" );					// count firms

RESULT( v[0] )


EQUATION( "XnetW" )
/*
Net world balance of trade (in int'l currency) consistency (must be zero)
*/

v[0] = 0;
CYCLE( cur, "Country" )
	v[0] += ( VS( cur, "X" ) - VS( cur, "M" ) ) / VS( cur, "e" );

RESULT( ROUND( v[0], 0, 1 ) )					// remove rounding errors


EQUATION( "Yw" )
/*
Total gross world product (in int'l currency)
*/

v[0] = 0;										// accumulator
CYCLE( cur, "Country" )
	v[0] += VS( cur, "Y" ) / VS( cur, "e" );

RESULT( v[0] )


EQUATION( "cCw" )
/*
Planned average unit cost (in int'l currency) of consumption goods worldwide
*/

v[1] = v[2] = 0;								// accumulators
CYCLE( cur, "Country" )							// in all countries
{
	v[1] += SUMS( cur, "Q2" );					// planned goods productions
	v[2] += WHTAVES( cur, "c2", "Q2" ) / VS( cur, "e" );// w. planned unit cost
}

RESULT( v[1] > 0 ? v[2] / v[1] : CURRENT )


EQUATION( "pCmaxW" )
/*
Maximum average price (in int'l currency) of consumption goods worldwide
*/

v[0] = - DBL_MAX;
CYCLE( cur, "Country" )							// in all countries
	v[0] = max( v[0], MAXS( cur, "p2avg" ) / VS( cur, "e" ) );// max price so far

RESULT( v[0] )


EQUATION( "pCminW" )
/*
Minimum average price (in int'l currency) of consumption goods worldwide
*/

v[0] = DBL_MAX;
CYCLE( cur, "Country" )							// in all countries
	v[0] = min( v[0], MINS( cur, "p2avg" ) / VS( cur, "e" ) );// max price so far

RESULT( v[0] )


EQUATION( "pCavgW" )
/*
Weighted average price (in int'l currency) of consumption goods worldwide
*/

v[1] = v[2] = 0;								// accumulators
CYCLE( cur, "Country" )							// in all countries
{
	v[1] += SUMS( cur, "Q2e" );					// effective good productions
	v[2] += WHTAVES( cur, "p2avg", "Q2e" ) / VS( cur, "e" );// weighted price
}

RESULT( v[1] > 0 ? v[2] / v[1] : CURRENT )


EQUATION( "pKavgW" )
/*
Weighted average price (in int'l currency) of capital goods worldwide
*/

v[1] = v[2] = 0;								// accumulators
CYCLE( cur, "Country" )							// in all countries
{
	v[1] += SUMS( cur, "Q1e" );					// effective machine productions
	v[2] += WHTAVES( cur, "p1avg", "Q1e" ) / VS( cur, "e" );// weighted price
}

RESULT( v[1] > 0 ? v[2] / v[1] : CURRENT )


EQUATION( "qCmaxW" )
/*
Maximum average product quality of consumption goods worldwide
*/

v[0] = - DBL_MAX;
CYCLE( cur, "Country" )							// in all countries
	v[0] = max( v[0], MAXS( cur, "q2avg" ) );	// max quality so far

RESULT( v[0] )


EQUATION( "qCminW" )
/*
Minimum average product quality of consumption goods worldwide
*/

v[0] = DBL_MAX;
CYCLE( cur, "Country" )							// in all countries
	v[0] = min( v[0], MINS( cur, "q2avg" ) );	// max quality so far

RESULT( v[0] )


EQUATION( "trMCavgW" )
/*
Weighted average import duties for consumption goods worldwide
*/

v[1] = v[2] = 0;								// accumulators for Mc/trMC
CYCLE( cur, "Country" )							// in all countries
{
	v[1] += v[3] = VS( cur, "Mc" ) / VS( cur, "e" );// imports in int'l terms
	v[2] += VS( cur, "trMC" ) * v[3];
}

RESULT( v[1] > 0 ? v[2] / v[1] : AVE( "trMC" ) )


EQUATION( "trMKavgW" )
/*
Weighted average import duties for capital goods worldwide
*/

v[1] = v[2] = 0;								// accumulators for Mk/trMK
CYCLE( cur, "Country" )							// in all countries
{
	v[1] += v[3] = VS( cur, "Mk" ) / VS( cur, "e" );// imports in int'l terms
	v[2] += VS( cur, "trMK" ) * v[3];
}

RESULT( v[1] > 0 ? v[2] / v[1] : AVE( "trMK" ) )


EQUATION( "trXCavgW" )
/*
Weighted average export duties for consumption goods worldwide
*/

v[1] = v[2] = v[3] = i = 0;						// accumulators for Xc/trX2
CYCLE( cur, "Country" )							// in all countries
{
	v[1] += v[4] = VS( cur, "Xc" ) / VS( cur, "e" );// exports in int'l terms
	v[2] += v[5] = AVES( cur, "trX2" );
	v[3] += v[4] * v[5];
	++i;
}

RESULT( v[1] > 0 ? v[3] / v[1] : v[2] / i )


EQUATION( "trXKavgW" )
/*
Weighted average export duties for capital goods worldwide
*/

v[1] = v[2] = v[3] = i = 0;						// accumulators for Xk/trX1
CYCLE( cur, "Country" )							// in all countries
{
	v[1] += v[4] = VS( cur, "Xk" ) / VS( cur, "e" );// exports in int'l terms
	v[2] += v[5] = AVES( cur, "trX1" );
	v[3] += v[4] * v[5];
	++i;
}

RESULT( v[1] > 0 ? v[3] / v[1] : v[2] / i )


EQUATION( "wCavgW" )
/*
Weighted average wage  (in int'l currency) in consumption-good sectors worldwide
*/

v[1] = v[2] = 0;								// accumulators
CYCLE( cur, "Country" )							// in all countries
{
	v[1] += SUMS( cur, "L2" );					// labor in consumption sector
	v[2] += WHTAVES( cur, "w2avg", "L2" ) / VS( cur, "e" );// w. average wage
}

RESULT( v[1] > 0 ? v[2] / v[1] : CURRENT )


/*========================= INITIALIZATION EQUATION ==========================*/

EQUATION( "initWorld" )
/*
Initialize the K+S world (root) object
*/

PARAMETER;										// execute only once

// initialize lagged variables
WRITEL( "Yw", DBL_MAX, -1 );

// initialize each country, accumulating
i = v[1] = v[2] = v[3] = 0;
CYCLE( cur, "Country" )
{
	VS( cur, "initCountry" );

	v[1] += AVELS( cur, "c2", 1 ) / INIEXCH;
	v[2] += AVELS( cur, "p1avg", 1 ) / INIEXCH;
	v[3] += AVELS( cur, "w2avg", 1 ) / INIEXCH;
	++i;
}

// initialize lagged variables depending on countries' initialization
WRITEL( "cCw", v[1] / i, -1 );
WRITEL( "pKavgW", v[2] / i, -1 );
WRITEL( "wCavgW", v[3] / i, -1 );

RESULT( 1 )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "fXCrescale" )
/*
Rescale desired market shares in consumption-good exports to ensure adding to 1
To be called after market shares are changed in '_fCX'
*/

v[1] = SUM( "fXC" );							// add-up market shares

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator
CYCLE( cur, "Country" )							// rescale to add-up to 1
{
	if ( v[1] > 0 )
		v[2] = VS( cur, "fXC" ) / v[1];			// rescaled market share
	else
		v[2] = 1 / COUNT( "Country" );			// fair share

	v[0] += v[2];
	WRITES( cur, "fXC", v[2] );					// save updated m.s.
}

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/
