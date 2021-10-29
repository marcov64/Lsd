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
RESULT( V( "__Qde" ) * VS( GRANDPARENT, "mDE" ) )


EQUATION( "__RSde" )
/*
Capacity to scrap of dirty power plant
*/

if ( T - V( "__tDE" ) > VS( GRANDPARENT, "etaE" ) )// over technical life
{
	v[0] = V( "__Kde" );						// scrap entire plant
	DELETE( THIS );								// delete plant object (suicide)
}
else
	v[0] = 0;
	
RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "__Cf" )
/*
Cost of fuel employed in operating dirty power plant
*/
RESULT( V( "__Qde" ) * VS( GRANDPARENT, "pF" ) / V( "__Ade" ) )


EQUATION( "__EmDE" )
/*
CO2 emissions of dirty power plant
*/
RESULT( V( "__Qde" ) * V( "__emDE" ) / V( "__Ade" ) )


EQUATION( "__cDE" )
/*
Production unit cost of dirty power plant
*/
RESULT( VS( GRANDPARENT, "pF" ) / V( "__Ade" ) + 
		VS( GRANDPARENT, "mDE" ) * VS( LABSUPL3, "w" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "__Qde", "" )
/*
Effective generation of dirty power plant
Updated in '_allocE'
*/
