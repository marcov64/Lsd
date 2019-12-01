/******************************************************************************

	VINTAGE OBJECT EQUATIONS
	------------------------

	Equations that are specific to the Vint objects in the K+S LSD model 
	are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_RS" )
/*
Number of machines to scrap in vintage of firm in consumption-good sector
Positive values indicate non-economical machines but still in technical life
Negative values represent machines out of technical life to be scrapped ASAP
*/

VS( PARENT, "_supplier" );						// ensure supplier is selected
cur = SHOOKS( HOOKS( PARENT, SUPPL ) )->up;		// pointer to new supplier
v[1] = VLS( PARENT, "_w2avg", 1 );				// average firm wage

if ( V( "_tVint" ) > t - VS( GRANDPARENT, "eta" ) )	// still in technical life?
{
	// unit cost advantage of new machines
	v[2] = v[1] / V( "_Avint" ) - v[1] / VS( cur, "_Atau" );
	
	// if new machine cost is not better in absolute terms or
	// payback period of replacing current vintage is over b
	if ( v[2] <= 0 || 
		 VS( cur, "_p1" ) / VS( GRANDPARENT, "m2" ) / v[2] > VS( GRANDPARENT, "b" ) )
		END_EQUATION( 0 );						// nothing to scrap	

	END_EQUATION( V( "_nVint" ) );				// scrap if can be replaced
}

RESULT( - V( "_nVint" ) )						// scrap if not in use


EQUATION( "_dLdVint" )
/*
Additional labor required for required utilization of vintage
*/

h = VS( GRANDPARENT->up, "flagWorkerLBU" );		// worker-level learning mode
v[1] = V( "_Avint" ) * VS( LABSUPL3, "Lscale" );// vintage notional productivity
v[2] = V( "_toUseVint" ) * VS( GRANDPARENT, "m2" );	// machine required capacity

v[3] = v[4] = 0;								// accumulators
cur1 = NULL;
CYCLE( cur, "WrkV" )
{
	v[3] += v[4] = VS( SHOOKS( cur ), "_s" );	// add current workers skills
	cur1 = cur;									// save last (newest) worker
}

if ( v[2] == 0 && cur1 == NULL )				// nothing to do
	END_EQUATION( 0 );
	
v[5] = v[4] * v[1];								// last hired worker capacity
v[6] = v[3] * v[1]; 							// all workers capacity

if ( v[6] - v[5] > v[2] )						// is last worker in excess?
{
	WRITE_HOOKS( SHOOKS( cur1 ), VWRK, NULL );	// worker not in any vintage
	DELETE( cur1 );								// remove vintage bridge-object

	v[0] = 0;									// nobody to hire
}
else
	if ( v[6] < v[2] )							// makes sense to hire workers?
	{
		if ( h != 0 && h != 2 )					// learning by vintage mode?
		{										// worker has public skills
			i = V( "_IDvint" );
			v[7] = V_EXTS( GRANDPARENT, country, vintProd[ i ].sVp );
		}
		else
			v[7] = 1;							// no skill factor

		v[8] = v[7] * v[1];						// new hired worker capacity

		v[0] = ceil( ( v[2] - v[6] ) / v[8] );	// desired new workers to hire
	}
	else
		v[0] = 0;

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_Qvint" )
/*
Vintage production with available workers
It is capped by the machines physical capacity
*/

VS( PARENT, "_alloc2" );						// ensure allocation is done

v[1] =  V( "_Avint" );							// vintage notional productivity

i = v[2] = 0;									// accumulators
CYCLE( cur, "WrkV" )
{
	v[2] += VS( SHOOKS( cur ), "_s" );			// add current workers skills
	++i;
}

WRITE( "_AeVint", i > 0 ? v[2] * v[1] / i : v[1] );

RESULT( v[2] * v[1] * VS( LABSUPL3, "Lscale" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_AeVint", "_Qvint" )
/*
Effective productivity of vintage
Use potential productivity if no worker available
*/
