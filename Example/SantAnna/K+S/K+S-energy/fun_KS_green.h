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
RESULT( V( "__lifeGEcycle" ) > 0 ?
			max( VL( "__QgeU", 1 ), VS( GRANDPARENT, "mEmin" ) ) *
			V( "__Kge" ) * VS( GRANDPARENT, "mGE" ) : 0 )


EQUATION( "__RSge" )
/*
Capacity to scrap of green power plant
*/

// request scrapping Tcon-1 periods before end of technical life
if ( T - V( "__tGE" ) == VS( GRANDPARENT, "etaE" ) -
						 ( VS( GRANDPARENT, "Tcon" ) - 1 ) )
	v[0] = V( "__Kge" );					// request scrap
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "__lifeGEcycle" )
/*
Stage in life cycle of green energy plant:
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
		if ( T == V( "__tGE" ) + VS( GRANDPARENT, "Tcon" ) - 1 )
			h = 1;
		break;

	case 1:										// available next period
		h = 2;
		break;

	case 2:										// operating
		if ( T - V( "__tGE" ) > VS( GRANDPARENT, "Tcon" ) +
								VS( GRANDPARENT, "etaE" ) )
			h = -1;								// scrap plant
		break;

	case -1:									// scrapped
		h = -2;
		if ( HOOKS( PARENT, TOPVINT ) == THIS )	// last green plant?
			WRITE_HOOKS( PARENT, TOPVINT, NULL );// remove parent top pointer
		DELETE( THIS );							// delete plant object (suicide)
}

RESULT( h )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "__DebGE" )
/*
Stock of project finance debt of green power plant
Also updated in '_EIe'
*/

v[0] = CURRENT;									// debt at end of last period

if ( v[0] <= 0 )								// no project finance debt?
{
	PARAMETER;									// stop computing
	END_EQUATION( 0 );
}

// check bank early exit option on first amortization period
if ( T == V( "__tGE" ) + VS( GRANDPARENT, "Tcon" ) )
{
	v[1] = npv( VLS( GRANDPARENT, "pEmavg", 1 ) * VLS( GRANDPARENT, "uEmavg", 1 ) -
				VLS( GRANDPARENT, "wEmavg", 1 ) / VLS( GRANDPARENT, "AeMavg", 1 ),
				V( "__rGEdeb" ), VS( GRANDPARENT, "etaE" ) ) - v[0];

	if ( v[1] < 0 )								// exorcize early exit option
	{
		INCRS( PARENT, "_DebE", v[0] );			// add to short term debt
		WRITE( "__pfinGE", 2 );					// indicate option exercised
		v[0] = 0;								// no outstanding project finance
	}
}

RESULT( ROUND( v[0] - V( "__amtGE" ), 0, 0.001 ) )// eliminate rounding errors


EQUATION( "__QgeU" )
/*
Utilization of green power plant
*/
VS( PARENT, "_allocE" );						// ensure generation allocated
RESULT( V( "__Qge" ) / V( "__Kge" ) )


EQUATION( "__amtGE" )
/*
Amortization of of project finance debt of green power plant
*/

v[1] = V( "__tGE" ) + VS( GRANDPARENT, "Tcon" );// first amortization period

if ( T < v[1] )									// grace period?
	END_EQUATION( 0 );							// no amortization

v[1] += V( "__TfinGE" );						// project finance end

if ( T >= v[1] || V( "__pfinGE" ) != 1 )		// nothing to amortize?
{
	PARAMETER;									// stop computing
	END_EQUATION( 0 );
}

RESULT( VL( "__DebGE", 1 ) / ( v[1] - T ) )


EQUATION( "__cGE" )
/*
Planned production unit cost of green power plant
*/
RESULT( VS( GRANDPARENT, "mGE" ) * VS( LABSUPL3, "w" ) )


EQUATION( "__iGE" )
/*
Interest paid on project finance of green power plant
*/
RESULT( VL( "__DebGE", 1 ) * V( "__rGEdeb" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "__Qge", "" )
/*
Effective generation of green power plant
Updated in '_allocE'
*/
