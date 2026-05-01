/******************************************************************************

	VINTAGE OBJECT EQUATIONS
	------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

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
v[0] = V( "__nVint" );							// number of machines in vintage
v[1] = VS( cur, "_p1" );						// machine price
v[2] = VS( PARENT, "_Gsi" );					// available subsidy pool
v[3] = V( "__cVint" ) - VS( cur, "_cTau" );		// cost advantage of new machine

// if new machine cost is not better in absolute terms or
// payback period of replacing current vintage is over b
if ( v[3] <= 0 || ( v[1] - v[2] / v[0] ) / VS( GRANDPARENT, "m2" ) / v[3] >
	 VS( GRANDPARENT, "b" ) )
	END_EQUATION( 0 );							// nothing to scrap

v[4] = min( v[0] * v[1], v[2] );				// subsidy effectively used
WRITE( "__Gsi", v[4] );							// register subsidy use
INCRS( PARENT, "_Gsi", - v[4] );				// reduce subsidy pool

RESULT( v[0] )									// scrap if can be replaced


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


EQUATION( "__Gsi" )
/*
Employed machine-replacement subsidy received from government
Updated in '__RSvint'
*/
RESULT( 0 )										// subsidy not used so far


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
