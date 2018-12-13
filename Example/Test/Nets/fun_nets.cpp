#include "fun_head_fast.h"

/****************************************************************************
	
	Network generation algorithms
	=============================

	Networks are created as sets of two type of objects: nodes and links. 
	Links are special properties of object nodes, so	each undirected link is 
	represented by two objects (two directed links), each one contained inside 
	its own node.

 ****************************************************************************/

object *network;							// global pointer to the network
											// used to save network after simul.

MODELBEGIN


EQUATION( "InitNet" )
/*
Initialize the net
*/

char message[256];

cur = network = SEARCH( "Network" );		// network nodes container (parent object)
v[0] = V( "typeNet" );						// type of network to build
v[1] = V( "numNodes" );						// number of nodes in network
v[2] = V( "numLinks" );						// number of links in network
v[3] = V( "avgOutDeg" );					// average outgoing degree
v[4] = V( "linkProb" );						// probability of two nodes linking
v[5] = V( "rho" );							// small-world rewiring parameter
v[6] = V( "expLink" );						// power-law exponential parameter
v[7] = V( "nCol" );							// number of columns in lattice
v[8] = V( "eightNeigbr" );					// flag to use 8 neighbors instead of 4

switch ( (int) v[0] )						// executes initialization for each type
{
	case 0:									// read external network file
		v[2] = LOAD_NETS( cur, "Node", "net" );
		sprintf( message, "\n Network read from file: net_%d.net", seed - 1); 
	break;

	case 1:									// (fully) Directed random network
		v[2] = INIT_NETS( cur, "Node", "RANDOM-DIR", v[1], v[2], 0 );
		strcpy( message, "\n Fully random network (directed) created" );
	break;

	case 2:									// (fully) Undirected random network
		v[2] = INIT_NETS( cur, "Node", "RANDOM-UNDIR", v[1], v[2], 0 );
		strcpy( message, "\n Fully random network (undirected) created" );
	break;

	case 3:									// Uniform random network
		v[2] = INIT_NETS( cur, "Node", "UNIFORM", v[1], v[3], 0 );
		strcpy( message, "\n Uniform random network created" );
	break;

	case 4:									// Renyi-Erdos random network
		v[2] = INIT_NETS( cur, "Node", "RENYI-ERDOS", v[1], 0, v[4] );
		strcpy( message, "\n Renyi-Erdos random network created" );
	break;

	case 5:									// Circle (non-random) network
		v[2] = INIT_NETS( cur, "Node", "CIRCLE", v[1], v[3], 0 );
		strcpy( message, "\n Circle regular network created" );
	break;

	case 6:									// Small-World random network
		v[2] = INIT_NETS( cur, "Node", "SMALL-WORLD", v[1], v[3], v[5] );
		strcpy( message, "\n Small-world random network created" );
	break;  
		
	case 7:									// Scale-free (power law) random network
		v[2] = INIT_NETS( cur, "Node", "SCALE-FREE", v[1], v[3], v[6] );
		strcpy( message, "\n Scale-free random network created" );
	break;

	case 8:									// Star non-random network
		v[2] = INIT_NETS( cur, "Node", "STAR", v[1], 0, 0 );
		strcpy( message, "\n Star network created" );
	break;

	case 9:									// Lattice network
		v[2] = INIT_NETS( cur, "Node", "LATTICE", v[1], v[7], v[8] );
		strcpy( message, "\n Lattice network created" );
	break;
		
	default:
		strcpy( message, "\nError: Invalid type of network selected!\n" );
}

STAT_NETS( cur, "Node" );					// get network statistics

WRITES( cur, "numNodes", v[0] );			// updates parameters
WRITES( cur, "numLinks", v[1] );
WRITES( cur, "avgOutDeg", v[2] );

LOG( message );
LOG( "\n Num. nodes: %.0f, Num. arcs: %.0f, Avg. out-degree: %.4f", v[0], v[1], v[2] );
LOG( "\n Min. out-degree: %.0f, Max. out-degree: %.0f, Density: %.4f", v[3], v[4], v[5] );
																
SHUFFLE_NETS( cur, "Node" );				// shuffles node sequence 
											// (IDs and linked list order)

PARAMETER;									// turn variable into parameter
											// run variable just once
RESULT( 1 )									// return 1 as ok


EQUATION( "NewLink" )
/*
Add one new bidirectional link randomly every time step,
with weight equal to the time
*/

cur1 = RNDDRAW_NODE( "Node" );				// draw first node
cur2 = RNDDRAW_NODE( "Node" );				// draw second node

ADDLINKWS( cur1, cur2, t );					// link from 1st to 2nd
ADDLINKWS( cur2, cur1, t );					// link from 2nd to 1st

SNAP_NET( "Node", CONFIG );	// create a network snapshot

RESULT( 1 )


EQUATION( "TotalLinks" )
/*
Register the total number of links in network
*/

STAT_NET( "Node" );							// get network statistics

RESULT( v[1] )


EQUATION( "NodeLinks" )
/*
Register the total number of links in the node
*/

STAT_NODE;									// get node statistics

RESULT( v[0] )


EQUATION( "TopLink" )
/*
Register the id of the highest weight link destination for the node
and reduce the weight by one
*/

v[0] = 0;									// max weight
v[1] = 0;									// max weight link dest. id
cur = NULL;									// pointer to destination
curl1 = NULL;

CYCLE_LINK( curl )
	if ( V_LINK( curl ) > v[0] )			// is this link bigger?
	{
		v[0] = V_LINK( curl );				// save the new top weight
		cur = LINKTO( curl );				// to where it points
		curl1 = curl;
		v[1] = V_NODEIDS( cur );			// and its id
	}

if ( curl1 != NULL )
	WRITE_LINK( curl1, v[0] - 0.1 ); 		// reduces weight of top link

if ( v[1] != 0 )							// search top pointed node
{
	cur2 = SEARCH_NODES( p->up, "Node", v[1] );

	if ( cur != NULL && cur != cur2 )		// check pointer & id match
		plog( "\nWarning: pointer mismatch" );
	else
	{
		v[2] = V_NODEIDS( cur );
		v[3] = V( "numNodes" );
		WRITE_NODEIDS( cur, v[2] + v[3] );	// change top node id
	}
}

RESULT( v[1] )


MODELEND


void close_sim( void )
{
	// save final network as Pajek file
	LOG( "\n Saving final network: %s%s%s_%d.net", 
			 path, strlen( path ) == 0 ? "" : "/", simul_name, seed - 1 );
	SAVE_NETS( network, "Node", CONFIG );
}
