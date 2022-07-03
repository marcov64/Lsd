/******************************************************************************

	GREEN OBJECT EQUATIONS
	----------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the green energy plant objects in the
	K+S LSD model are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "__LgeD" )
/*
Labor required for operation of green power plant
*/
RESULT( max( VL( "__QgeU", 1 ), VS( GRANDPARENT, "mEmin" ) ) *
		V( "__Kge" ) * VS( GRANDPARENT, "mGE" ) )


EQUATION( "__RSge" )
/*
Capacity to scrap of green power plant
*/

v[0] = 0;										// assume no or done scrapping

if ( CURRENT > 0 )								// scrapped last period?
{
	if ( HOOKS( PARENT, TOPVINT ) == THIS )		// last green plant?
		WRITE_HOOKS( PARENT, TOPVINT, NULL );	// remove parent top pointer

	DELETE( THIS );								// delete plant object (suicide)
}
else
	if ( T - V( "__tGE" ) > VS( GRANDPARENT, "etaE" ) )// over technical life
		v[0] = V( "__Kge" );					// scrap entire plant

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "__QgeU" )
/*
Utilization of green power plant
*/
VS( PARENT, "_allocE" );						// ensure generation allocated
RESULT( V( "__Qge" ) / V( "__Kge" ) )


EQUATION( "__cGE" )
/*
Planned production unit cost of green power plant
*/
RESULT( VS( GRANDPARENT, "mGE" ) * VS( LABSUPL3, "w" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "__Qge", "" )
/*
Effective generation of green power plant
Updated in '_allocE'
*/
