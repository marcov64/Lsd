/******************************************************************************

	VINTAGE OBJECT EQUATIONS
	------------------------

	Equations that are specific to the Vint objects in the K+S LSD model 
	are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_LdVint" )
/*
Labor required for desired utilization of vintage
*/
RESULT( V( "_toUseVint" ) * VS( GRANDPARENT, "m2" ) / V( "_AlpVint" ) )


EQUATION( "_RSvint" )
/*
Number of machines to scrap in vintage of firm in consumption-good sector
Positive values indicate non-economical machines but still in technical life
Negative values represent machines out of technical life to be scrapped ASAP
*/

if ( V( "_tVint" ) < T - VS( GRANDPARENT, "eta" ) )// out of technical life?
	END_EQUATION( - V( "_nVint" ) );			// scrap if not in use

VS( PARENT, "_supplier" );						// ensure supplier is selected
cur = PARENTS( SHOOKS( HOOKS( PARENT, SUPPL ) ) );// pointer to supplier

v[1] = V( "_cVint" ) - VS( cur, "_cTau" );		// cost advantage of new machine

// if new machine cost is not better in absolute terms or
// payback period of replacing current vintage is over b
if ( v[1] <= 0 || 
	 VS( cur, "_p1" ) / VS( GRANDPARENT, "m2" ) / v[1] > VS( GRANDPARENT, "b" ) )
	END_EQUATION( 0 );							// nothing to scrap	

RESULT( V( "_nVint" ) )							// scrap if can be replaced


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_EmVint" )
/*
CO2 (carbon) emissions produced by vintage
*/
RESULT( V( "_Qvint" ) * V( "_AefVint" ) / V( "_AeeVint" ) )


EQUATION( "_EnVint" )
/*
Energy consumed by vintage
*/
RESULT( V( "_Qvint" ) / V( "_AeeVint" ) )


EQUATION( "_Qvint" )
/*
Vintage production with available workers
It is capped by the machines physical capacity
*/
VS( PARENT, "_alloc2" );						// ensure allocation is done
RESULT( V( "_AlpVint" ) * V( "_Lvint" ) )


EQUATION( "_cVint" )
/*
Planned (expected) unit production cost of vintage
*/
RESULT( VS( LABSUPL3, "w" ) / V( "_AlpVint" ) + 
	    ( VLS( ENESECL3, "pE", 1 ) + VS( PARENTS( GRANDPARENT ), "trCO2" ) * 
		V( "_AefVint" ) ) / V( "_AeeVint" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_Lvint", "" )
/*
Labor available for operation of vintage
Updated in '_alloc2'
*/
