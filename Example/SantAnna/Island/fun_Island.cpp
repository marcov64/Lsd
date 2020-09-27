#define NO_POINTER_INIT							// disable pointer checking

#include "fun_head_fast.h"

// colors of lattice markers
#define UNKNOWN 3								// unknown island - yellow
#define KNOWN 2									// known island - green
#define COLONIZED 1								// colonized island - red
#define SEA 5									// sea (not an island) - blue
#define EXPLORER 1000							// explorer ship - white
#define IMITATOR 0								// imitator ship - black

// support C++ functions (code at the end of the file)
object *add_island( object *p, int x, int y, double *count, bool show, int size );
void neighborhood( object *knownIsland, double rho, double minSgnPrb );
void set_marker( bool show, int x, int y, int color, int size );

// insert your equations between MODELBEGIN and MODELEND

MODELBEGIN

//////////////////////////////// SEA object equations ////////////////////////////////

EQUATION( "Init" )
/*
Technical variable to create the sea lattice and initialize 
it with the islands and agents.
It is computed only once in the beginning of the simulation.
Must be the first variable in the list.
*/

FAST;											// comment to show log/debug messages
USE_ZERO_INSTANCE;								// enable zero-instance objects

v[0] = v[1] = 0;								// island/known island counters
v[2] = V( "pi" );								// island probability
v[3] = VL( "l", 1 );							// initial number of known islands
v[4] = V( "l0radius" );							// radius for search for initial islands
v[5] = V( "rho" );								// degree of locality of interactions
v[6] = V( "minSgnPrb" );						// minimum signal probability to consider
v[7] = V( "sizeLattice" );						// size of the lattice window

// check if there are enough islands on the defined radius for setting known islands
if ( v[3] > v[4] * v[4] && ! ( v[3] == 1 && v[4] == 0 ) )
{
	PLOG( "\n\nError: invalid l0=%.0lf for l0radius=%.0lf\n", v[3], v[4] );
	ABORT;
	END_EQUATION( 0 );
}

// create the visual lattice (blue ocean)
if ( v[7] > 0 && ! V( "latticeOpen" ) )
{
	i = min( v[7], 2 * LAST_T ) + 1;			// effective size of the lattice window
	INIT_LAT( SEA, i, i );						// create lattice window using SEA color
	WRITES( PARENT, "latticeOpen", 1 );			// avoid more than one lattice instance
	WRITE( "seaShown", 1 );						// signal this Sea instance has the lattice
	k = 1;										// lattice open
}
else
	k = 0;										// lattice not open

// set the KnownIsland object instances to be nodes of a network
INIT_NET( "KnownIsland", "DISCONNECTED", 1 );

// handle bounded economies (pi=0)
if ( v[2] == 0 && v[3] == 2 )
{
	add_island( THIS, 1, 1, &v[0], k, v[7] );	// island at (1, 1)
	add_island( THIS, 1, v[4], &v[0], k, v[7] );// island at (1, l0radius)
}
else
{
	// make sure there is an island at (0, 0)
	add_island( THIS, 0, 0, &v[0], k, v[7] );

	// create random islands to fill the initial radius plus one
	for ( i = - v[4] - 1; i <= v[4] + 1; ++i )
		for ( j = - v[4] - 1; j <= v[4] + 1; ++j )
			// draw the existence of an island (except in (0, 0))
			if ( RND < v[2] && ! ( i == 0 && j == 0 ) )
				// create island and add to the graphical lattice if required
				add_island( THIS, i, j, &v[0], k, v[7] );
}

// draw the required number of known islands ((0, 0) is always known)
while( v[1] < v[3] )
{
	if ( v[1] == 0 )
		cur = SEARCH( "Island" );				// first is island at (0, 0) or (1, 1)
	else
		cur = RNDDRAW_FAIR( "Island" );			// drawn existing island
		
	int x = VS( cur, "_xIsland" );				// island (0, 0)-centered coordinates
	int y = VS( cur, "_yIsland" );
	
	// avoid the islands just over the radius or already set as known
	if ( VS( cur, "_known" ) || ( x < - v[4] || x > v[4] || y < - v[4] || y > v[4] ) )
		continue;

	// translate from (0, 0)-centered to absolute coordinates
	i = x + LAST_T;
	j = y + LAST_T;

	if ( v[1] == 0 )							// first known island?
		cur1 = SEARCH( "KnownIsland" );			// pick existing object
	else
		cur1 = ADDOBJ( "KnownIsland" );			// add new object instance
						
	WRITE_SHOOKS( cur, cur1 );					// save pointer to KnownIsland object
	WRITE_SHOOKS( cur1, cur );					// save pointer to Island object
				
	++v[1];										// count the known islands
	WRITES( cur, "_known", 1 );					// flag island as known
	WRITES( cur1, "_s", abs( x ) + abs( y ) );	// island prod. coeff.
	WRITES( cur1, "_idKnown", v[1] );			// save known island id
				
	neighborhood( cur1, v[5], v[6] );			// create neighborhood network			
	set_marker( k, x, y, KNOWN, v[7] ); 		// change island marker
					
	LOG( "\nKnownIsland=%.0lf at x=%d y=%d", v[1], x, y );				
}

WRITE( "J", v[0] );								// number of existing islands in t=1
WRITE( "northFrontier", v[4] + 1 );				// register the initial sea borders
WRITE( "southFrontier", - v[4] - 1 );
WRITE( "eastFrontier", v[4] + 1 );
WRITE( "westFrontier", - v[4] - 1 );
		
// create agents objects	
j = V( "N" );									// the total number of agents
INIT_TSEARCHT( "KnownIsland", v[1] );			// prepare turbo search

for ( i = 0; i < j; ++i )
{
	k = uniform_int( 1, v[1] );					// island where the agent starts
	cur = TSEARCH( "KnownIsland", k );			// pointer to KnownIsland object
	
	if ( i == 0 )								// first agent?
		cur1 = SEARCH( "Agent" );				// pick existing agent object
	else
		cur1 = ADDOBJ( "Agent" );				// add new agent object instance
		
	cur2 = SEARCHS( cur, "Miner" );				// pick existing miner object in island
	if ( ! ( SHOOKS( cur2 ) == NULL ) )			// existing object instance already used?
		cur2 = ADDOBJS( cur, "Miner" );			// add new miner object instance
		
	WRITE_SHOOKS( cur1, cur2 );					// save pointer to agent as miner
	WRITE_SHOOKS( cur2, cur1 );					// save pointer to Agent object
	cur3 = SHOOKS( cur );						// pointer to island
	
	WRITES( cur1, "_idAgent", i + 1 );			// save agent id number
	WRITES( cur1, "_xAgent", VS( cur3, "_xIsland" ) );	// agent x coordinate
	WRITES( cur1, "_yAgent", VS( cur3, "_yIsland" ) );	// agent y coordinate
	WRITELS( cur1, "_a", 1, -1 );				// agent initial state (miner)
	WRITES( cur2, "_active", 1 );				// flag active Miner
}

PARAMETER;										// turn variable into parameter

RESULT( 1 )


EQUATION( "Step" )
/*
Technical variable to force the calculation of any Variable 
that has to be calculated early in the time step.
Must be the second variable in the list, after 'Init'.
*/

v[1] = V( "simSpeed" );

if ( v[1] > 0 )									// avoid calling with 0 argument (win32)
	SLEEP( v[1] );								// slow down the simulation, if needed

CYCLE( cur, "Agent" )							// make sure the agents decisions are
	VS( cur, "_a" );							// taken first during the time step

RESULT( 1 )


EQUATION( "g" )
/*
Total output (GDP) growth rate.
*/

v[1] = VL( "Q", 1 );							// past period GDP
v[2] = V( "Q" );								// current GDP

if ( v[1] > 0 && v[2] > 0 )						// don't compute if Q is zero
	v[0] = log( v[2] ) - log( v[1] );
else
	v[0] = 0;
	
RESULT( v[0] )


EQUATION( "l" )
/*
Number of known islands (technologies).
*/

RESULT( COUNT( "KnownIsland" ) )


EQUATION( "m" )
/*
Number of agents currently mining on all islands.
*/

RESULT( SUM( "_m" ) )


EQUATION( "Q" )
/*
Total output (GDP).
*/

RESULT( SUM( "_Qisland" ) )


EQUATION( "J" )
/*
The number of existing islands.
Expands the size of the sea as agents get closer to the borders.
*/

v[0] = CURRENT;									// number of islands
v[1] = V( "pi" );								// island probability
v[2] = V( "seaShown" );							// graphical lattice on flag
v[3] = V( "sizeLattice" );						// size of the lattice window
v[4] = V( "westFrontier" );						// existing sea frontiers
v[5] = V( "eastFrontier" );
v[6] = V( "southFrontier" );
v[7] = V( "northFrontier" );

// check agents closer to the existing sea frontier islands
v[8] = MIN( "_xAgent" );
v[9] = MAX( "_xAgent" );
v[10] = MIN( "_yAgent" );
v[11] = MAX( "_yAgent" );

// expand to the west if required
if ( v[8] <= v[4] )
{
	WRITE( "westFrontier", --v[4] );			// update the frontier
	for ( i = v[4], j = v[6]; j <= v[7]; ++j )	// move south -> north
		if ( RND < v[1] )						// is it an island?
			add_island( THIS, i, j, & v[0], v[2], v[3] );
}

// expand to the east if required
if ( v[9] >= v[5] )
{
	WRITE( "eastFrontier", ++v[5] );			// update the frontier
	for ( i = v[9], j = v[6]; j <= v[7]; ++j )	// move south -> north
		if ( RND < v[1] )						// is it an island?
			add_island( THIS, i, j, & v[0], v[2], v[3] );
}

// expand to the south if required
if ( v[10] <= v[6] )
{
	WRITE( "southFrontier", --v[6] );			// update the frontier
	for ( j = v[6], i = v[4]; i <= v[5]; ++i )	// move west -> east
		if ( RND < v[1] )						// is it an island?
			add_island( THIS, i, j, & v[0], v[2], v[3] );
}

// expand to the north if required
if ( v[11] >= v[7] )
{
	WRITE( "northFrontier", ++v[7] );			// update the frontier
	for ( j = v[7], i = v[4]; i <= v[5]; ++i )	// move west -> east
		if ( RND < v[1] )						// is it an island?
			add_island( THIS, i, j, & v[0], v[2], v[3] );
}

RESULT( v[0] )


//////////////////////////// KNOWNISLAND object equations ////////////////////////////

EQUATION( "_m" )
/*
Number of agents currently mining on island.
*/

v[0] = SUM( "_active" );						// count active Miner objects
v[1] = V( "sizeLattice" );						// size of the lattice window

// update island marker
if ( v[0] == 0 )
	set_marker( V( "seaShown" ),  VS( SHOOK, "_xIsland" ), 
				VS( SHOOK, "_yIsland" ), KNOWN, v[1] );	
else
	set_marker( V( "seaShown" ),  VS( SHOOK, "_xIsland" ), 
				VS( SHOOK, "_yIsland" ), COLONIZED, v[1] );	

RESULT( v[0] )


EQUATION( "_Qisland" )
/*
Total output of island.
*/

RESULT( SUM( "_Qminer" ) )


EQUATION( "_c" )
/*
The productivity of the island.
*/

v[1] = V( "_m" );

if ( v[1] != 0 )								// avoid division by zero
	v[0] = V( "_Qisland" ) / v[1];
else
	v[0] = 0;
	
RESULT( v[0] )


/////////////////////////////// MINER object equations ///////////////////////////////

EQUATION( "_Qminer" )
/*
Output of miner.
*/

RESULT( V( "_s" ) * pow( V( "_m" ), V( "alpha" ) - 1 ) * V( "_active" ) )


EQUATION( "_cBest" )
/*
The productivity of the best known island with a signal received.
*/

v[1] = VL( "m", 1 );							// total number of miners

if ( ! V( "_active" ) || v[1] == 0 )			// only for active miners and
	END_EQUATION( 0 );							// avoid division by zero

v[2] = 0;										// best productivity so far
i = j = 0;										// best island coordinates

// check all network connections of current island for signals
CYCLE_LINKS( PARENT, curl )
{
	cur = LINKTO( curl );						// object connected
	
	// the probability of message being received from this connection
	v[3] = ( VLS( cur, "_m", 1 ) / v[1] ) * V_LINK( curl );
	
	if ( RND < v[3] )							// signal received?
	{
		v[4] = VLS( cur, "_c", 1 );				// connection productivity
		
		if ( v[4] > v[2] )						// is it the best so far?
		{
			v[2] = v[4];						// save best productivity
			i = VS( SHOOKS( cur ), "_xIsland" );// and the island coordinates
			j = VS( SHOOKS( cur ), "_yIsland" );
		}
	}
}

WRITE( "_xBest", i );							// save best island coordinates
WRITE( "_yBest", j );

RESULT( v[2] )


/////////////////////////////// AGENT object equations ///////////////////////////////

EQUATION( "_a" )
/*
The agent state. Can be:
1 - miner
2 - explorer
3 - imitator
*/

v[1] = V( "sizeLattice" );						// size of the lattice window
v[2] = V( "_idAgent" );							// id of current agent
i = V( "_xAgent" );								// agent coordinates in t-1
j = V( "_yAgent" );
	
// if explorer or imitator, move in the lattice sea
if ( CURRENT > 1 )
{
	// clear current marker from lattice, if required
	set_marker( V( "seaShown" ), i, j, SEA, v[1] );	

	// if it is an explorer, move randomly across the sea
	if ( CURRENT == 2 )
	{
		if ( V( "pi" ) == 0 )					// no exploration
		{
			v[3] = V( "l0radius" );				// radius to constrain search
			v[4] = 2;							// north-south move only
		}
		else
		{
			v[3] = LAST_T;						// can move anywhere
			v[4] = 4;							// move in four directions
		}
		
		h = uniform_int( 1, v[4] );				// decide direction to move
		switch( h )		
		{
			case 1:								// north
				j = ( j < v[3] ) ? j + 1 : j;
				break;
			case 2:								// south
				v[5] = ( v[4] == 2 ) ? 1 : - v[3]; // limit if no exploration
				j = ( j > v[5] ) ? j - 1 : j;	
				break;
			case 3:								// east
				i = ( i < v[3] ) ? i + 1 : i;
				break;
			case 4:								// west
				i = ( i > - v[3] ) ? i - 1 : i;
				break;
		}
		
		LOG( "\n Agent=%.0lf explorer at x=%d y=%d to %s", v[2], i, j, 
			 h == 1 ? "north" : h == 2 ? "south" : h == 3 ? "east" : "west" );	
	
		// check if island exists
		cur = SEARCH_CNDS( PARENT, "_idIsland", ( i + LAST_T ) * 1E6 + ( j + LAST_T ) );
	}
		
	// if it is an imitator, move straight to the new island
	if ( CURRENT == 3 )							// it is an imitator?
	{
		h = V( "_xTarget" );					// target imitated island
		k = V( "_yTarget" );
		
		if ( abs( h - i ) > abs( k - j ) )		// distance on x larger?
			i += copysign( 1, h - i );			// get closer by the x direction
		else
			j += copysign( 1, k - j );			// get closer by the y direction
			
		if ( i == h && j == k ) 
			cur = SEARCH_CNDS( PARENT, "_idIsland", ( i + LAST_T ) * 1E6 + ( j + LAST_T ) );
		else
			cur = NULL;
			
		LOG( "\n Agent=%.0lf imitator at x=%d y=%d to x=%d y=%d", v[2], i, j, h, k );
	}
	
	WRITE( "_xAgent", i );						// update agent position
	WRITE( "_yAgent", j );
		
	if ( cur != NULL )							// found an island?
	{
		if ( ! VS( cur, "_known" ) )			// discovered a new island?
		{
			// compute the productivity coefficient of the discovered island
			// uniform( -sqrt( 3 ), sqrt( 3 ) ) is a r.v. with mean 0 and variance 1
			v[1] = ( 1 + poisson( V( "lambda" ) ) ) * 
				   ( abs( i ) + abs( j ) + V( "phi" ) * V( "_Qlast" ) + 
				   	 uniform( -sqrt( 3 ), sqrt( 3 ) ) );
		
			k = COUNTS( PARENT, "KnownIsland" );// last island number

			cur1 = ADDOBJS( PARENT, "KnownIsland" );	// add new KnownIsland instance
			cur2 = SEARCHS( cur1, "Miner" );	// pointer to the first existing Miner
			
			WRITE_SHOOKS( cur, cur1 );			// save pointer to KnownIsland object
			WRITE_SHOOKS( cur1, cur );			// save pointer to Island object
			
			WRITES( cur, "_known", 1 );			// flag island as known
			WRITES( cur1, "_s", v[1] );			// island prod. coeff.
			WRITES( cur1, "_idKnown", k + 1 );	// save known island id
						
			neighborhood( cur1, V( "rho" ),V( "minSgnPrb") );// create neighborhood network
		}
		else
		{
			cur1 = SHOOKS( cur );				// known island, just pick pointer	
			cur2 = SEARCHS( cur1, "Miner" );	// check if the first Miner object is unused
			if ( VS( cur2, "_active" ) )		// an used object points to an existing Agent
				cur2 = ADDOBJS( cur1, "Miner" );// add new miner object instance
		}
		
		WRITE_SHOOK( cur2 );					// save pointer to agent as miner
		WRITE_SHOOKS( cur2, THIS );				// save pointer to Agent object
		
		WRITES( cur2, "_active", 1 );			// flag active Miner
		WRITES( cur2, "_agentId", V( "_idAgent" ) );// keep pairing numbers between Agent
		WRITE( "_knownId", VS( cur1, "_idKnown" ) );// and KnownIsland while mining (debug)
		WRITE( "_xTarget", 0 );					// clear target coordinates
		WRITE( "_yTarget", 0 );

		LOG( "\n Agent=%.0lf mining at x=%d y=%d Known=%.0lf", 
			 v[2], i, j, VS( cur1, "_idKnown" ) );

		END_EQUATION( 1 );						// becomes a miner again
	}
	else										// keep on the navigation
	{
		if ( CURRENT == 2 )
			set_marker( V( "seaShown" ), V( "_xAgent" ), V( "_yAgent" ), 
						EXPLORER, v[1] );	
		else
			set_marker( V( "seaShown" ), V( "_xAgent" ), V( "_yAgent" ), 
						IMITATOR, v[1] );	
			 	
		END_EQUATION( CURRENT );
	}
}

// a miner decides if become explorer
if ( RND < V( "epsilon" ) )
{
	LOG( "\n Agent=%.0lf exploring from x=%d y=%d", v[2], i, j );	
	
	WRITE( "_Qlast", VLS( SHOOK, "_Qminer", 1 ) );// save last output 
		
	if ( COUNTS( PARENTS( SHOOK ), "Miner" ) > 1 )// don't delete last object instance
		DELETE( SHOOK );						// or delete associated Miner object
	else
	{
		WRITES( SHOOK, "_active", 0 );			// or flag inactive Miner
		WRITES( SHOOK, "_agentId", 0 );			// disconnect pairing Miner->Agent
		WRITE_SHOOKS( SHOOK, NULL );			// disconnect Miner from Agent object
	}
			
	WRITE( "_knownId", 0 );						// disconnect pairing Agent->KnownIsland
	WRITE_SHOOK( NULL );						// disconnect Agent from Miner object
	
	END_EQUATION( 2 );							// become explorer
}

// a miner evaluates becoming an imitator
if ( VS( SHOOK, "_cBest" ) > VLS( PARENTS( SHOOK ), "_c", 1 ) )
{
	LOG( "\n Agent=%.0lf imitating from x=%d y=%d to x=%.0lf y=%.0lf", 
		 v[2], i, j, VS( SHOOK, "_xBest" ), VS( SHOOK, "_yBest" ) );	

	WRITE( "_xTarget", VS( SHOOK, "_xBest" ) );	// coordinates of new target island
	WRITE( "_yTarget", VS( SHOOK, "_yBest" ) );
	
	if ( COUNTS( PARENTS( SHOOK ), "Miner" ) > 1 )// don't delete last object instance
		DELETE( SHOOK );						// or delete associated Miner object
	else
	{
		WRITES( SHOOK, "_active", 0 );			// or flag inactive Miner
		WRITES( SHOOK, "_agentId", 0 );			// disconnect pairing Miner->Agent
		WRITE_SHOOKS( SHOOK, NULL );			// disconnect Miner from Agent object
	}
			
	WRITE( "_knownId", 0 );						// disconnect pairing Agent->KnownIsland
	WRITE_SHOOK( NULL );						// disconnect Agent from Miner object
	
	END_EQUATION( 3 );							// become imitator
}

RESULT( 1 )										// keep mining


MODELEND

// do not add Equations in this area - support C++ functions below

// add one (unknown) island to the model
object *add_island( object *p, int x, int y, double *count, bool show, int size )
{
	int i, j;
	object *cur;
	
	i = x + LAST_T;								// calculated absolute coordinates
	j = y + LAST_T;

	if ( *count == 0 )							// first island?
		cur = SEARCHS( THIS, "Island" );		// pick existing object
	else	
		cur = ADDOBJS( THIS, "Island" );		// add new object instance
	
	( *count )++;								// update the islands counter
	WRITES( cur, "_idIsland", i * 1E6 + j );	// save island id number (coord)
	WRITES( cur, "_xIsland", x );				// save island x coordinate
	WRITES( cur, "_yIsland", y );				// save island y coordinate
	
	set_marker( show, x, y, UNKNOWN, size );	// create island marker
	
	LOG( "\nIsland=%.0lf at x=%d y=%d", *count, x, y );
	
	return cur;
}

// build the diffusion (star) network among known islands
void neighborhood( object *knownIsland, double rho, double minSgnPrb )
{
	double x, xj, y, yj, maxSgnPrb;
	object *cur;
	
	// get the coordinates of the network hub (new known island) 
	x = VS( SHOOKS( knownIsland ), "_xIsland" );
	y = VS( SHOOKS( knownIsland ), "_yIsland" );
	
	// run over all known islands to create network links
	CYCLES( PARENTS( knownIsland ), cur, "KnownIsland" )
		// check if the link already exists (no link to self)
		if ( cur != knownIsland && SEARCH_LINKS( knownIsland, V_NODEIDS( cur ) ) == NULL )
		{
			// coordinates of the current network spoke (existing known island) 
			xj = VS( SHOOKS( cur ), "_xIsland" );
			yj = VS( SHOOKS( cur ), "_yIsland" );
	
			// calculate the maximum signal probability (intensity)
			maxSgnPrb = exp( - rho * ( abs( x - xj ) + abs( y - yj ) ) );
			
			// only create link if probability is above minimum threshold
			if ( maxSgnPrb < minSgnPrb )
				continue;
				
			// add bidirectional link weighted by the maximum signal probability
			ADDLINKWS( knownIsland, cur, maxSgnPrb );
			ADDLINKWS( cur, knownIsland, maxSgnPrb );
		}
}


// function to set a marker in the lattice
void set_marker( bool show, int x, int y, int color, int size )
{
	// transform from (0, 0)-centered to lattice window absolute coordinates
	int i = x + size / 2 + 1;
	int j = size - ( y + size / 2 ) + 1;
	
	// check lattice shown and ignore markers outside canvas area
	if ( show && ( i >= 1 && i <= size + 1 && j >= 1 && j <= size + 1 ) )
		WRITE_LAT( j, i, color );
}


// close simulation special commands
void close_sim( void )
{

}
