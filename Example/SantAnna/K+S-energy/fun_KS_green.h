/******************************************************************************

	GREEN OBJECT EQUATIONS
	----------------------

	Equations that are specific to the green energy plant objects in the 
	K+S LSD model are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "__LgeD" )
/*
Labor required for operation of green power plant
*/
RESULT( V( "__Qge" ) * VS( GRANDPARENT, "mGE" ) )


EQUATION( "__RSge" )
/*
Capacity to scrap of green power plant
*/

if ( T - V( "__tGE" ) > VS( GRANDPARENT, "etaE" ) )// over technical life
{
	v[0] = V( "__Kge" );						// scrap entire plant
	DELETE( THIS );								// delete plant object (suicide)
}
else
	v[0] = 0;
	
RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "__cGE" )
/*
Production unit cost of green power plant
*/
RESULT( VS( GRANDPARENT, "mGE" ) * VS( LABSUPL3, "w" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "__Qge", "" )
/*
Effective generation of green power plant
Updated in '_allocE'
*/
