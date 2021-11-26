/******************************************************************************

	DIRTY OBJECT EQUATIONS
	----------------------

	Equations that are specific to the dirty energy plant objects in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "__RSde" )
/*
Capacity to scrap of dirty power plant
*/

if ( T - V( "__tDE" ) > VS( PARENT, "etaE" ) )	// over technical life
{
	v[0] = V( "__Kde" );						// scrap entire plant
	DELETE( THIS );								// delete plant object (suicide)
}
else
	v[0] = 0;

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "__Df" )
/*
Demand (physical units) of fuel by dirty power plant
*/
VS( PARENT, "Ce" );								// ensure generation allocated
RESULT( V( "__Qde" ) / V( "__Ade" ) )


EQUATION( "__EmDE" )
/*
CO2 emissions of dirty power plant
*/
RESULT( V( "__Df" ) * V( "__emDE" ) )


EQUATION( "__cDE" )
/*
Production unit cost of dirty power plant
*/
RESULT( VS( PARENT, "pF" ) / V( "__Ade" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "__Qde", "" )
/*
Effective generation of dirty power plant
Updated in 'Ce'
*/
