/******************************************************************************

	DIRTY OBJECT EQUATIONS
	----------------------

	Equations that are specific to the dirty energy plant objects in the 
	K+S LSD model are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_RSde" )
/*
Capacity to scrap of dirty power plant
*/

if ( T - V( "_tDE" ) > VS( PARENT, "etaE" ) )	// over technical life
{
	v[0] = V( "_Kde" );							// scrap entire plant
	DELETE( THIS );								// delete plant object (suicide)
}
else
	v[0] = 0;
	
RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_cDE" )
/*
Production unit cost of dirty power plant
*/
RESULT( VS( PARENT, "pF" ) / V( "_Ade" ) )


EQUATION( "_EmDE" )
/*
CO2 emissions of dirty power plant
*/
RESULT( V( "_Qde" ) * V( "_emDE" ) / V( "_Ade" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_Qde", "" )
/*
Effective generation of dirty power plant
Updated in 'Ce'
*/
