/******************************************************************************

	DIRTY OBJECT EQUATIONS
	----------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the dirty energy plant objects in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "__LdeD" )
/*
Labor required for operation of dirty power plant
*/
RESULT( V( "__lifeDEcycle" ) > 0 ?
			max( VL( "__QdeU", 1 ), VS( GRANDPARENT, "mEmin" ) ) *
			V( "__Kde" ) * VS( GRANDPARENT, "mDE" ) : 0 )


EQUATION( "__RSde" )
/*
Capacity to scrap of dirty power plant
*/

// request scrapping Tcon-1 periods before end of technical life
if ( T - V( "__tDE" ) == VS( GRANDPARENT, "etaE" ) -
						 ( VS( GRANDPARENT, "Tcon" ) - 1 ) )
	v[0] = V( "__Kde" );					// request scrap
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "__lifeDEcycle" )
/*
Stage in life cycle of dirty energy plant:
0 = in construction
1 = available for operation next period
2 = operating
-1 = scrapped
-2 = object removed
Also remove scrapped-plant object in next period
*/

h = CURRENT;									// current state

switch( h )
{
	case 0:										// non-operational
		if ( T == V( "__tDE" ) + VS( GRANDPARENT, "Tcon" ) - 1 )
			h = 1;
		break;

	case 1:										// available next period
		h = 2;
		break;

	case 2:										// operating
		if ( T - V( "__tDE" ) > VS( GRANDPARENT, "Tcon" ) +
								VS( GRANDPARENT, "etaE" ) )
			h = -1;								// scrap plant
		break;

	case -1:									// scrapped
		h = -2;
		DELETE( THIS );							// delete plant object (suicide)
}

RESULT( h )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "__Df" )
/*
Demand (physical units) of fuel by dirty power plant
*/
VS( PARENT, "_allocE" );						// ensure generation allocated
RESULT( V( "__Qde" ) / V( "__Ade" ) )


EQUATION( "__EmDE" )
/*
CO2 emissions of dirty power plant
*/
RESULT( V( "__Df" ) * V( "__emDE" ) )


EQUATION( "__QdeU" )
/*
Utilization of dirty power plant
*/
VS( PARENT, "_allocE" );						// ensure generation allocated
RESULT( V( "__Qde" ) / V( "__Kde" ) )


EQUATION( "__cDE" )
/*
Planned production unit cost of dirty power plant
*/
RESULT( VS( GRANDPARENT, "pF" ) / V( "__Ade" ) +
		VS( GRANDPARENT, "mDE" ) * VS( LABSUPL3, "w" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "__Qde", "" )
/*
Effective generation of dirty power plant
Updated in '_allocE'
*/
