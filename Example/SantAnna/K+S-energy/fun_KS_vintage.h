/******************************************************************************

	VINTAGE OBJECT EQUATIONS
	------------------------

	Equations that are specific to the Vint objects in the K+S LSD model
	are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "__LdVint" )
/*
Labor required for desired utilization of vintage
*/
RESULT( V( "__toUseVint" ) * VS( GRANDPARENT, "m2" ) / V( "__AlpVint" ) )


EQUATION( "__RSvint" )
/*
Number of machines to scrap in vintage of firm in consumption-good sector
Positive values indicate non-economical machines but still in technical life
Negative values represent machines out of technical life to be scrapped ASAP
*/

if ( V( "__tVint" ) < T - VS( GRANDPARENT, "eta" ) )// out of technical life?
	END_EQUATION( - V( "__nVint" ) );			// scrap if not in use

VS( PARENT, "_supplier" );						// ensure supplier is selected
cur = PARENTS( SHOOKS( HOOKS( PARENT, SUPPL ) ) );// pointer to supplier

v[1] = V( "__cVint" ) - VS( cur, "_cTau" );		// cost advantage of new machine

// if new machine cost is not better in absolute terms or
// payback period of replacing current vintage is over b
if ( v[1] <= 0 ||
	 VS( cur, "_p1" ) / VS( GRANDPARENT, "m2" ) / v[1] > VS( GRANDPARENT, "b" ) )
	END_EQUATION( 0 );							// nothing to scrap

RESULT( V( "__nVint" ) )						// scrap if can be replaced


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "__EmVint" )
/*
CO2 (carbon) emissions produced by vintage
*/
RESULT( V( "__Qvint" ) * V( "__AefVint" ) / V( "__AeeVint" ) )


EQUATION( "__EnVint" )
/*
Energy consumed by vintage
*/
RESULT( V( "__Qvint" ) / V( "__AeeVint" ) )


EQUATION( "__Qvint" )
/*
Vintage production with available workers
It is capped by the machines physical capacity
*/
VS( PARENT, "_alloc2" );						// ensure allocation is done
RESULT( V( "__AlpVint" ) * V( "__Lvint" ) )


EQUATION( "__cVint" )
/*
Planned (expected) unit production cost of vintage
*/
RESULT( VS( LABSUPL3, "w" ) / V( "__AlpVint" ) +
		( VS( ENESECL3, "pE" ) + VS( PARENTS( GRANDPARENT ), "trCO2" ) *
		V( "__AefVint" ) ) / V( "__AeeVint" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "__Lvint", "" )
/*
Labor available for operation of vintage
Updated in '_alloc2'
*/
