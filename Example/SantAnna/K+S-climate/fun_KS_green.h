/******************************************************************************

	GREEN OBJECT EQUATIONS
	----------------------

	Equations that are specific to the green energy plant objects in the 
	K+S LSD model are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_RSge" )
/*
Capacity to scrap of green power plant
*/

if ( T - V( "_tGE" ) > VS( PARENT, "etaE" ) )	// over technical life
{
	v[0] = V( "_Kge" );							// scrap entire plant
	DELETE( THIS );								// delete plant object (suicide)
}
else
	v[0] = 0;
	
RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_Qge" )
/*
Effective generation of green power plant
*/
v[1] = VS( PARENT, "Kge" );						// total green capacity
v[2] = max( v[1] - VS( PARENT, "De" ), 0 ) / v[1];// excess factor, if any
RESULT( V( "_Kge" ) * ( 1 - v[2] ) )			// adjust, if needed


/*============================= DUMMY EQUATIONS ==============================*/
