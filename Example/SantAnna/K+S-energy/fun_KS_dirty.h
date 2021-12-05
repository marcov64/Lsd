/******************************************************************************

	DIRTY OBJECT EQUATIONS
	----------------------

	Equations that are specific to the dirty energy plant objects in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "__LdeD" )
/*
Labor required for operation of dirty power plant
*/
RESULT( max( VL( "__QdeU", 1 ), VS( GRANDPARENT, "mEmin" ) ) *
		V( "__Kde" ) * VS( GRANDPARENT, "mDE" ) )


EQUATION( "__RSde" )
/*
Capacity to scrap of dirty power plant
*/

v[0] = 0;										// assume no or done scrapping

if ( CURRENT > 0 )								// scrapped last period?
	DELETE( THIS );								// delete plant object (suicide)
else
	if ( T - V( "__tDE" ) > VS( GRANDPARENT, "etaE" ) )// over technical life
		v[0] = V( "__Kde" );					// scrap entire plant

RESULT( v[0] )


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
