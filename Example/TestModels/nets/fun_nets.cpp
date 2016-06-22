#include "fun_head.h"

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

switch ( (int) v[0] )						// executes initialization for each type
{
	case 0:									// read external network file
		v[2] = NETWORKS_LOAD( cur, "", "Node", "net" );
		sprintf( message, "\n Network read from file: net_%d.net", seed - 1); 
	break;

	case 1:									// (fully) Directed random network
		v[2] = NETWORKS_INI( cur, "Node", "RANDOM-DIR", v[1], v[2], 0 );
		strcpy( message, "\n Fully random network (directed) created" );
	break;

	case 2:									// (fully) Undirected random network
		v[2] = NETWORKS_INI( cur, "Node", "RANDOM-UNDIR", v[1], v[2], 0 );
		strcpy( message, "\n Fully random network (undirected) created" );
	break;

	case 3:									// Uniform random network
		v[2] = NETWORKS_INI( cur, "Node", "UNIFORM", v[1], v[3], 0 );
		strcpy( message, "\n Uniform random network created" );
	break;

	case 4:									// Renyi-Erdos random network
		v[2] = NETWORKS_INI( cur, "Node", "RENYI-ERDOS", v[1], 0, v[4] );
		strcpy( message, "\n Renyi-Erdos random network created" );
	break;

	case 5:									// Circle (non-random) network
		v[2] = NETWORKS_INI( cur, "Node", "CIRCLE", v[1], v[3], 0 );
		strcpy( message, "\n Circle regular network created" );
	break;

	case 6:									// Small-World random network
		v[2] = NETWORKS_INI( cur, "Node", "SMALL-WORLD", v[1], v[3], v[5] );
		strcpy( message, "\n Small-world random network created" );
	break;  
		
	case 7:									// Scale-free (power law) random network
		v[2] = NETWORKS_INI( cur, "Node", "SCALE-FREE", v[1], v[3], v[6] );
		strcpy( message, "\n Scale-free random network created" );
	break;

	case 8:									// Star (non-random) network
		v[2] = NETWORKS_INI( cur, "Node", "DISCONNECTED", v[1], 0, 0 );
											// start just with nodes
		v[10] = 1;							// node counter from first
		CYCLES( cur, cur2, "Node" )			// create the strokes
		{
			if ( VS_NODEID( cur2 ) == 1 )	// first node?
			{
				cur1 = cur2;				// save pointer
				WRITES_NODENAME( cur1, "Center" );// and name it
			}
			else							// other nodes
			{
				ADDLINKS( cur1, cur2 );		// link from 1 to n
				ADDLINKS( cur2, cur1 );		// link from n to 1

				sprintf( message, "Spoke%4.4d", (int) v[10] - 1 );
				WRITES_NODENAME( cur2, message );	// name node
			}
			v[10] = v[10] + 1;				// count nodes
		}
		v[2] = 2 * ( v[10] - 1 );			// two links per node
											// except first
		strcpy( message, "\n Star network created" );
	break;
		
	default:
		strcpy( message, "\nError: Invalid type of network selected!\n" );
}

STATS_NET( cur, "Node" );					// get network statistics

WRITES( cur, "numNodes", v[0] );			// updates parameters
WRITES( cur, "numLinks", v[1] );
WRITES( cur, "avgOutDeg", v[2] );

plog( message );
sprintf( message, "\n Num. nodes: %.0f, Num. arcs: %.0f, Avg. out-degree: %.4f", v[0], v[1], v[2] );
plog( message );
sprintf( message, "\n Min. out-degree: %.0f, Max. out-degree: %.0f, Density: %.4f", v[3], v[4], v[5] );
plog( message );
																
SHUFFLES( cur, "Node" );					// shuffles node sequence 
											// (IDs and linked list order)

PARAMETER;									// turn variable into parameter
											// run variable just once
RESULT( 1 )									// return 1 as ok


EQUATION( "NewLink" )
/*
Add one new bidirectional link randomly every time step,
with weight equal to the time
*/

cur1 = RNDDRAW_NET( "Node" );				// draw first node
cur2 = RNDDRAW_NET( "Node" );				// draw second node

ADDLINKWS( cur1, cur2, t );					// link from 1st to 2nd
ADDLINKWS( cur2, cur1, t );					// link from 2nd to 1st

NETWORK_SNAP( "Node", path, simul_name );	// create a network snapshot

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
	if ( VS_WEIGHT( curl ) > v[0] )			// is this link bigger?
	{
		v[0] = VS_WEIGHT( curl );			// save the new top weight
		cur = LINKTO( curl );				// to where it points
		curl1 = curl;
		v[1] = VS_NODEID( cur );			// and its id
	}

if ( curl1 != NULL )
	WRITES_WEIGHT( curl1, v[0] - 0.1 ); 	// reduces weight of top link

if ( v[1] != 0 )							// search top pointed node
{
	cur2 = SEARCHS_NET( p->up, "Node", v[1] );

	if ( cur != NULL && cur != cur2 )		// check pointer & id match
		plog( "\nWarning: pointer mismatch" );
	else
	{
		v[2] = VS_NODEID( cur );
		v[3] = V( "numNodes" );
		WRITES_NODEID( cur, v[2] + v[3] );	// change top node id
	}
}

RESULT( v[1] )


MODELEND


void close_sim( void )
{
	// save final network as Pajek file
	char message[256];
	sprintf( message, "\n Saving final network: %s%s%s_%d.net", 
			 path, strlen( path ) == 0 ? "" : "/", simul_name, seed - 1 );
	plog( message );
	NETWORKS_SAVE( network, "Node", path, simul_name );
}
