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
RESULT( V( "_toUseVint" ) * VS( GRANDPARENT, "m2" ) / V( "_Avint" ) )


EQUATION( "_RS" )
/*
Number of machines to scrap in vintage of firm in consumption-good sector
Positive values indicate non-economical machines but still in technical life
Negative values represent machines out of technical life to be scrapped ASAP
*/

if ( V( "_tVint" ) < T - VS( GRANDPARENT, "eta" ) )// out of technical life?
	END_EQUATION( - V( "_nVint" ) );			// scrap if not in use

VS( PARENT, "_supplier" );						// ensure supplier is selected
cur = SHOOKS( HOOKS( PARENT, SUPPL ) )->up;		// pointer to supplier
v[1] = VS( LABSUPL3, "w" );						// firm wage

// unit cost advantage of new machines
v[2] = v[1] / V( "_Avint" ) - v[1] / VS( cur, "_Atau" );

// if new machine cost is not better in absolute terms or
// payback period of replacing current vintage is over b
if ( v[2] <= 0 || 
	 VS( cur, "_p1" ) / VS( GRANDPARENT, "m2" ) / v[2] > VS( GRANDPARENT, "b" ) )
	END_EQUATION( 0 );							// nothing to scrap	

RESULT( V( "_nVint" ) )							// scrap if can be replaced


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_Qvint" )
/*
Vintage production with available workers
It is capped by the machines physical capacity
*/
VS( PARENT, "_alloc2" );						// ensure allocation is done
RESULT( V( "_Avint" ) * V( "_Lvint" ) )


/*============================= DUMMY EQUATIONS ==============================*/
