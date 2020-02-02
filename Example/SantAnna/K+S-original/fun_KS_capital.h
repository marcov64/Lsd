/******************************************************************************

	CAPITAL-GOODS MARKET OBJECT EQUATIONS
	-------------------------------------

	Equations that are specific to the capital-goods market objects in the 
	K+S LSD model are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "L1" )
/*
Work force (labor) size employed by capital-good sector
Updates 'L1rd'
*/

v[1] = VS( LABSUPL1, "Ls" );					// available labor force
v[2] = V( "L1rd" );								// R&D labor in sector 1
v[3] = V( "L1d" );								// desired workers in sector 1
v[4] = VS( CONSECL1, "L2d" );					// desired workers in sector 2
v[5] = V( "L1shortMax" );						// max shortage allowed in sector

v[2] = min( v[2], v[1] );						// ignore demand over total labor
v[3] = min( v[3], v[1] );
v[4] = min( v[4], v[1] );

if ( v[1] - v[2] < v[3] + v[4] )				// labor shortage?
{
	v[6] = ( v[1] - v[2] ) / ( v[3] + v[4] );	// shortage factor
	
	if ( v[6] < 1 - v[5] )						// over cap?
		v[6] = 1 - v[5];						// shortage on cap
}
else
	v[6] = 1;									// no shortage

RESULT( v[2] + ( v[3] - v[2] ) * v[6] )


EQUATION( "entry1exit" )
/*
Rate of entry-exit of firms in the capital-good sector
Perform entry and exit of firms 
All relevant aggregate variables in sector must be computed before existing
firms are deleted, so all active firms in period are considered
*/

VS( CONSECL1, "K" );							// ensure canceled orders acct'd
UPDATE;											// ensure aggregates are computed

double NW10u = V( "NW10" ) * V( "PPI" ) / V( "PPI0" );// minimum wealth in s. 1
double n1 = V( "n1" );							// market participation period
int F1 = V( "F1" );								// number of firms

vector < bool > quit( F1, false );				// vector of firms' quit status

// mark bankrupt and market-share-irrelevant firms to quit the market
h = F1;											// initial number of firms
v[1] = v[2] = v[3] = i = k = 0;					// accum., counters, registers
CYCLE( cur, "Firm1" )
{
	v[4] = VS( cur, "_NW1" );					// current net wealth
	
	if ( v[4] < 0 || T >= VS( cur, "_t1ent" ) + n1 )// bankrupt or incumbent?
	{
		for ( v[5] = j = 0; j < n1; ++j )
			v[5] += VLS( cur, "_BC", j );		// n1 periods customer number
		
		if ( v[4] < 0 || v[5] <= 0 )
		{
			quit[ i ] = true;					// mark for likely exit
			--h;								// one less firm
			
			if ( v[5] > v[3] )					// best firm so far?
			{
				k = i;							// save firm index
				v[3] = v[5];					// and customer number
			}
		}
	}
	
	++i;
}	

// quit candidate firms exit, except the best one if all going to quit
i = j = 0;										// firm counters
CYCLE_SAFE( cur, "Firm1" )
{
	if ( quit[ i ] )
	{
		if ( h > 0 || i != k )					// firm must exit?
		{
			// account liquidation credit due to public, if any
			v[2] += exit_firm1( cur );			// delete object and liq. val.
			++j;								// count exits
		}
		else
			if ( h == 0 && i == k )				// best firm must get new equity
			{
				// new equity required
				v[6] = NW10u + VS( cur, "_Deb1" ) - VS( cur, "_NW1" );
				v[1] += v[6];					// accumulate "entry" equity cost
				
				WRITES( cur, "_Deb1", 0 );		// reset debt
				INCRS( cur, "_NW1", v[6] );		// add new equity
			}
	}

	++i;
}

V( "f1rescale" );								// redistribute exiting m.s.

// replace exiting firms by entrants
v[1] += entry_firm1( p, j, false );				// add entrant-firm objects

INCRS( PARENT, "cEntry", v[1] );				// account equity cost of entry
INCRS( PARENT, "cExit", v[2] );					// account exit credits

V( "f1rescale" );								// redistribute entrant m.s.
INIT_TSEARCHT( "Firm1", i );					// prepare turbo search indexing

RESULT( ( double ) j / F1 )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "A1" )
/*
Productivity of capital-good sector
*/
V( "PPI" );										// ensure m.s. are updated
RESULT( V( "Q1e" ) > 0 ? WHTAVE( "_Btau", "_f1" ) : CURRENT )


EQUATION( "D1" )
/*
Potential demand (orders) received by firms in capital-good sector
*/
RESULT( SUM( "_D1" ) )


EQUATION( "Deb1" )
/*
Total debt of capital-good sector
*/
RESULT( SUM( "_Deb1" ) )


EQUATION( "Div1" )
/*
Total dividends paid by firms in capital-good sector
*/
V( "Tax1" );									// ensure dividends are computed
RESULT( SUM( "_Div1" ) )


EQUATION( "JO1" )
/*
Open job positions in capital-good sector
*/
RESULT( SUM( "_JO1" ) )


EQUATION( "L1d" )
/*
Total labor demand from firms in capital-good sector
Includes R&D labor
*/
RESULT( SUM( "_L1d" ) )


EQUATION( "L1dRD" )
/*
R&D labor demand from firms in capital-good sector
*/
RESULT( SUM( "_L1dRD" ) )


EQUATION( "L1rd" )
/*
Total R&D labor employed by firms in capital-good sector
*/
RESULT( min( V( "L1dRD" ), VS( LABSUPL1, "Ls" ) * V( "L1rdMax" ) ) )


EQUATION( "NW1" )
/*
Total net wealth (free cash) of firms in capital-good sector
*/
RESULT( SUM( "_NW1" ) )


EQUATION( "Pi1" )
/*
Total profits of capital-good sector
*/
RESULT( SUM( "_Pi1" ) )


EQUATION( "PPI" )
/*
Producer price index
*/
V( "f1rescale" );								// ensure m.s. computed/rescaled
RESULT( WHTAVE( "_p1", "_f1" ) )


EQUATION( "Q1" )
/*
Total planned output of firms in capital-good sector
*/
RESULT( SUM( "_Q1" ) )


EQUATION( "Q1e" )
/*
Total effective real output (orders) of capital-good sector
*/
RESULT( SUM( "_Q1e" ) )


EQUATION( "S1" )
/*
Total sales of capital-good sector
*/
RESULT( SUM( "_S1" ) )


EQUATION( "Tax1" )
/*
Total taxes paid by firms in capital-good sector
*/
RESULT( SUM( "_Tax1" ) )


EQUATION( "W1" )
/*
Total wages paid by firms in capital-good sector
*/
RESULT( SUM( "_W1" ) )


EQUATION( "imi" )
/*
Imitation success rate in capital-good sector
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
SUM( "_Atau" );									// ensure innovation is done
SUM( "_NC" );									// ensure brochures distributed
RESULT( SUM( "_imi" ) / V( "F1" ) )


EQUATION( "inn" )
/*
Innovation success rate in capital-good sector
Also ensures all innovation/imitation is done, brochures are distributed and
learning-by-doing skills are updated
*/
V( "imi" );										// ensure innovation is done
RESULT( SUM( "_inn" ) / V( "F1" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "f1rescale" )
/*
Rescale market shares in capital-good sector to ensure adding to 1
To be called after market shares are changed in '_f1' and 'entry1exit'
*/

v[1] = SUM( "_f1" );							// add-up market shares

if ( ROUND( v[1], 1, 0.001 ) == 1.0 )			// ignore rounding errors
	END_EQUATION( v[1] );

v[0] = 0;										// accumulator

if ( v[1] > 0 )									// production ok?
	CYCLE( cur, "Firm1" )						// rescale to add-up to 1
	{
		v[0] += v[2] = VS( cur, "_f1" ) / v[1];	// rescaled market share
		WRITES( cur, "_f1", v[2] );				// save updated m.s.
	}
else
{
	v[2] = 1 / COUNT( "Firm1" );				// firm fair share
	
	CYCLE( cur, "Firm1" )						// rescale to add-up to 1
	{
		v[0] += v[2];
		WRITES( cur, "_f1", v[2] );
	}
}

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "exit1fail", "entry1exit" )
/*
Rate of bankrupt firms in capital-good sector
Updated in 'entry1exit'
*/
