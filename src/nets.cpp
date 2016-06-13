/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente (& Marcelo Pereira in this)
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/


/****************************************************************************************
NETS.CPP
	Network tools: functions to load, generate and save networks in LSD
	
	v1: initial compilation by Marcelo Pereira
	v2: full integration with LSD
	
	All functions work on specially defined LSD object's data structures (named here as 
    "node"), with the following organization:
	
    object --+-- node --+- nodeID (long) : node unique ID number (re-orderable)
                        +- serNum (long) : node serial number (initial order, fixed)
                        +- nLinks (long) : number of arcs FROM node
                        +- first (ptr) : pointer to the first outgoing link
                        +- last(ptr) : pointer to the last outgoing link
                        +- prob (double) : assigned node probability (power-law)
                        |
                        +-- link --+- serTo (long) : destination node serial
                                   +- ptrTo (ptr) : pointer to neighbor
                                   +- node (ptr) : node containing link
                                   +- prev (ptr) : pointer to previous link or NULL
                                   +- next (ptr) : pointer to next link or NULL
                                   +- weight (double) : link weight
                                   +- probTo (double) : dest. node prob. (power-law)

	Objects with "node" equal to NULL are not elements of a network. Currently, each
	object can be part of only one network.

	Networks can be loaded and saved from/to Pajek-formatted files. Network objects are
	formatted as directed graphs (undirected links represented by two directed arcs in 
	opposite directions). The available methods are:
	
	parent->read_file_net( lab, dir, base_name, serial, ext )
				
	parent->write_file_net( lab, dir, base_name, serial, ext )
	
	Where "parent" is the object where to search for "lab", that is the name of the 
	object that will be used as the container for the nodes in the network. If the 
	existing number of objects "lab" is less than the required number, the missing 
	objects are automatically created. "dir" is the folder where the Pajek network 
	file is located. The format for the network file name is "<base_name>_<serial>.<ext>".
	If multiple simulation runs are used, <serial> is incremented sequentially.
	
	There are also some alternative pseudo-random network generator algorithms available:

	parent->uniform_net( lab, numNodes, outDeg )
	
	parent->circle_net( lab, numNodes, outDeg )
	
	parent->renyi_erdos_net( lab, numNodes, linkProb )
	
	parent->small_world_net( lab, numNodes, outDeg, rho )

	parent->power_law_net( lab, numNodes, outDeg, expLink )
		
	The additional parameters for those generators are:
	
	numNodes : number of nodes in network
    numLinks : number of arcs (directed links) in network
    (avg)outDeg : (average of) arcs (directed links) per node (out degree)
	linkProb : probability of link between two nodes
    expLink : power degree (power-law networks only)
    rho : rewiring link probability (small-world networks only)
	
	All generators return the effective number of directed links (arcs) of the generated
	network. According to the generator used, the network may have to be reshuffled before
	use, applying the method:
	
	parent->shuffle_net( lab )

	Reshuffling reassigns nodeIDs and the network object linked list order but does not 
	change original node's serial numbers or the network structure.
	
	Other methods to directly manipulate nodes data and links:
	
	object->add_node_net( id, name )
	
	object->delete_node_net( void )

	parent->search_node_net( lab, destId )

	parent->draw_node_net( lab )

	object->add_link_net( destPtr )

	object->delete_link_net( destPtr )

	object->search_link_net( destId )
	
****************************************************************************************/

#include "decl.h"
#include <math.h>

#define GCCLIBS true			// enable linux code

#ifdef GCCLIBS
#include <ctype.h>
char *strupr( char *s )
{ char *p = s; for ( ; *p; ++p ) *p = toupper( *p ); return s; }
#endif

#define RND (double)ran1(&idum)

object *go_brother(object *c);
double ran1(long *idum);
double rnd_integer(double m, double x);
void error_hard(void);
void plog(char const *msg);

extern char msg[];
extern int t;
extern int max_step;
extern int seed;
extern long idum;
extern lsdstack *stacklog;

long nodesSerial = 0;					// node's serial number global counter


/*
	Initialize new link, at the end of linked list.
*/

netLink::netLink( object *origNode, object *destNode, double linkWeight, double destProb )
{
	time = t;							// save creation time

	if ( origNode->node == NULL )		// origin is not yet a node?
	{
		origNode->node = new netNode( );// create node data structure
		if( origNode->node == NULL )
		{
			plog( "\nError: out of memory." );
			error_hard( );
		}
	}
	if ( destNode->node == NULL )		// destination is not yet a node?
	{
		destNode->node = new netNode( );// create node data structure
		if( destNode->node == NULL )
		{
			plog( "\nError: out of memory." );
			error_hard( );
		}
	}
	
	ptrTo = destNode;
	ptrFrom = origNode;
	serTo = ptrTo->node->serNum;
	prev = ptrFrom->node->last;
	next = NULL;
	weight = linkWeight;
	probTo = destProb;
	
	if ( ptrFrom->node->first == NULL )	// first link?
		ptrFrom->node->first = this;
	else								// insert after last
		ptrFrom->node->last->next = this;
	ptrFrom->node->last = this;
	ptrFrom->node->nLinks++;
}


/*
	Destroy link, preserving linked list integrity.
*/

netLink::~netLink( void )
{
	if ( ptrFrom->node->first != this && ptrFrom->node->last != this )
	{											// not first nor last link?
		prev->next = next;
		next->prev = prev;
	}
	else
		if ( ptrFrom->node->first == this && ptrFrom->node->last == this )	
												// last link?
			ptrFrom->node->first = ptrFrom->node->last = NULL;
		else
			if ( ptrFrom->node->first == this )	// first link?
			{
				ptrFrom->node->first = next;
				next->prev = NULL;
			}
			else								// last link?
			{
				ptrFrom->node->last = prev;
				prev->next = NULL;
			}

	ptrFrom->node->nLinks--;
}


/*
	Add new link from Lsd object. Does NOT check if the link already exists.
	So, if multiple links are to be prevented, caller has to check before calling.
*/

netLink *object::add_link_net( object *destPtr, double weight = 0, double probTo = 1 )
{
	netLink *cur;
	if ( this->up != destPtr->up || strcmp( this->label, destPtr->label ) )
		return NULL;					// different parent or object type?
	cur = new netLink( this, destPtr, weight, probTo );
	if( cur == NULL )
	{
		plog( "\nError: out of memory." );
		error_hard( );
		return NULL;
	}

	return cur;
}


/*
	Remove link from Lsd object.
*/

void object::delete_link_net( netLink *ptr )
{
	netLink *cur;
	if ( node == NULL || ptr == NULL )	// no network structure or invalid ptr?
		return;
	for ( cur = node->first; 			// scan all links from node
		  cur != NULL && cur != ptr; 	// to make sure pointer belongs to node
		  cur = cur->next);
	if ( cur != NULL )
		delete cur;
}


/*
	Search for existing link from Lsd object. Return pointer to the first 
	link found or NULL if link to destination does not exist.
*/

netLink *object::search_link_net( long destId )
{
	netLink *cur;
	if ( node == NULL )					// no network structure?
		return NULL;
	for ( cur = node->first; 			// scan all links from node
		  cur != NULL && cur->ptrTo->node != NULL && 
		  cur->ptrTo->node->id != destId; 
		  cur = cur->next);
	if ( cur != NULL && cur->ptrTo->node == NULL )
		return NULL;
	else
		return cur;
}


/*
	Initialize netNode struct (no links).
*/

netNode::netNode( long nodeId, char const *nodeName, double nodeProb )
{
	id = nodeId;
	time = t;						// save creation time
	serNum = ++nodesSerial;
	nLinks = 0;
	first = last = NULL;
	
	if ( id < 0 )					// ID assigned?
		id = serNum;

	if ( strcmp( nodeName, "" ) )	// name assigned?
	{
		name = new char[ strlen( nodeName ) + 1 ];
		if( name == NULL )
		{
			plog( "\nError: out of memory." );
			error_hard( );
		}
		strcpy( name, nodeName );
	}
	else
		name = NULL;
	
	prob = nodeProb;
}


/*
	Destroy netNode struct.
*/

netNode::~netNode( void )
{
	if ( name != NULL )			// name assigned?
		delete name;
		
	while ( last != NULL )		// remove all links
		delete last;
}


/*
	Add netNode data structure to Lsd object
*/

netNode *object::add_node_net( long id = -1, char const nodeName[] = "", 
							   bool silent = false )
{
	if ( node != NULL )
	{
		if ( ! silent )
			plog( "\nWarning: existing network data discarded from object." );
		delete node;
	}
	
	node = new netNode( id, nodeName );
	if( node == NULL )
	{
		plog( "\nError: out of memory." );
		error_hard( );
		return NULL;
	}
	
	return node;
}


/*
	Remove netNode data structure from Lsd object.
*/

void object::delete_node_net( void )
{
	delete node;
	node = NULL;
}


/*
	Set or reset the name of a node.
*/

void object::name_node_net( char const *nodeName )
{
	if ( node == 0 )				// invalid node?
		return;
		
	if ( node->name != NULL )		// name already set?
		delete node->name;
		
	if ( strcmp( nodeName, "" ) )	// name assigned?
	{
		node->name = new char[ strlen( nodeName ) + 1 ];
		if( node->name == NULL )
		{
			plog( "\nError: out of memory." );
			error_hard( );
		}
		strcpy( node->name, nodeName );
	}
	else
		node->name = NULL;
}


/*
	Search for existing node. Return pointer to the object containing it
	or NULL if node does not exist.
	Slow for large networks, turbosearch is better in this case.
*/

object *object::search_node_net( char const *lab, long destId )
{
	object *cur;
	for ( cur = search( lab ); 
		  cur != NULL && cur->node != NULL && cur->node->id != destId; 
		  cur = go_brother( cur ) );
	if ( cur->node == NULL )						// no network structure?
		return NULL;
	else
		return cur;
}


/*
	Returns some basic statistics about the directed network.
	r[0]: number of nodes
	r[1]: number of links (arcs)
	r[2]: average out-degree
	r[3]: minimum out-degree
	r[4]: maximum out-degree
	r[5]: density (including loops)
*/

void object::stats_net( char const *lab, double *r )
{
	r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = 0;
	object *cur = search( lab );
	
	if ( cur == NULL || cur->node == NULL )			// invalid network node?
		return;
		
	for ( ; cur != NULL; cur = go_brother( cur ) )	// scan all nodes
		if ( cur->node != NULL )					// valid node?
		{
			double nLinks = (double) cur->node->nLinks;
			if ( r[0] == 0. )						// first node? 
				r[3] = nLinks;						// update minimum
			else
				r[3] = r[3] < nLinks ? r[3] : nLinks;
			
			r[0]++;
			r[1] += nLinks;
			r[4] = r[4] > nLinks ? r[4] : nLinks;
		}
		
	if ( r[0] > 0. )
	{
		r[2] = r[1] / r[0];
		r[5] = r[1] / ( r[0] * ( r[0] - 1 ) );
	}
}


/*
	Draw a node randomly, with probability equal to prob.
*/

object *object::draw_node_net( char const *lab )
{
	double sum, drawPoint, accProb;
	object *cur, *cur1, *cur2;

	cur1 = cur = search( lab );						// point to first object

	for ( sum = 0; cur != NULL && cur->node != NULL; cur = cur->next )
													// add-up probabilities
		sum += cur->node->prob;
		
	if ( isinf( sum ) || sum == 0 )					// check valid probabilities
	{
		sprintf( msg, "\nError (network node draw for '%s'): draw probabilities are invalid.", stacklog->vs->label );
		plog( msg );
		sprintf( msg, "\nFirst object (%s) returned.\n", lab );
		plog( msg );
		return cur1;
	}

	do
		drawPoint = RND * sum;
	while ( drawPoint == sum );						// avoid RND == 1
	
	for ( accProb = 0, cur = cur2 = cur1;			// accumulate probabilities
		  accProb <= drawPoint && cur != NULL; 		// until reaching the right object
		  accProb += cur->node->prob, cur = cur->next)
		cur2 = cur;									// save previous object

	return cur2;
}


/*
	Shuffle nodes order in the linked list of node objects.
	Use Fischer-Yates shuffling algorithm.
*/

object *object::shuffle_nodes_net( char const *lab )
{
	long i, j, iId, jId, numNodes;
	object *cur, *cur1;

	for ( numNodes = 0, cur = search( lab ); cur != NULL; 
		  numNodes++, cur = go_brother( cur ) );				// count number of nodes

	initturbo( lab, numNodes );									// seed the turbosearch linked list

	for ( i = numNodes; i > 1; i-- )							// run the shuffling
	{
		j = (long) rnd_integer( 1, i );
		cur = turbosearch( lab, 0, (double) i );
		cur1 = turbosearch( lab, 0, (double) j );
		
		if ( cur->node == NULL || cur1->node == NULL )
		{
			plog( "\nError: object " );
			plog( lab );
			plog( " does not contain the correct network data structure." );
			return NULL;
		}
		
		iId = cur->node->id;
		jId = cur1->node->id;
		cur->node->id = jId;
		cur1->node->id = iId;
	}

	lsdqsort( lab, NULL, "UP" );							// sort according to shuffled IDs
	
	return search( lab );
}


/*
	Calculate the missing number of object copies. Prints a warning if there are more
	existing copies than needed and returns 0.
*/

long nodes2create( object *parent, char const *lab, long numNodes )
{
	long count;
	object *cur;
	for ( count = 0, cur = parent->search( lab ); 
		  cur != NULL; count++, cur = go_brother( cur ) );
	if ( numNodes >= count )
		return numNodes - count;
	plog( "\nWarning: number of existing nodes is more than the required." );
	return 0;
}


/*
	Stub function to call the appropriate network generator.
*/

long object::init_stub_net( char const *lab, const char* gen, long numNodes, long par1, double par2 = 0 )
{
	char option[32];
	strncpy( option, gen, 31 );
	option[31] = '\0';
	strupr( option );
	
	if ( numNodes < 2 )								// less than 2 nodes?
		return 0;
		
	if ( ! strcmp( option, "DISCONNECTED" ) )
			return init_discon_net( lab, numNodes );
	
	if ( ! strcmp( option, "RANDOM-DIR" ) )
		if ( par1 > 0 )
			return init_random_dir_net( lab, numNodes, par1 );

	if ( ! strcmp( option, "RANDOM-UNDIR" ) )
		if ( par1 > 0 )
			return init_random_undir_net( lab, numNodes, par1 );

	if ( ! strcmp( option, "UNIFORM" ) )
		if ( par1 > 0 )
			return init_uniform_net( lab, numNodes, par1 );
	
	if ( ! strcmp( option, "CIRCLE" ) )
		if ( par1 > 0 )
			return init_circle_net( lab, numNodes, par1 );
	
	if ( ! strcmp( option, "RENYI-ERDOS" ) )
	{
		if ( par2 == 0 && numNodes != 0 && par1 != 0 )
			par2 = (double) par1 / numNodes;		// compute parameter
		if ( par2 > 0 )
			return init_renyi_erdos_net( lab, numNodes, par2 );
	}
	
	if ( ! strcmp( option, "SMALL-WORLD" ) )
		if ( par1 > 0 && par2 > 0 )
			return init_small_world_net( lab, numNodes, par1, par2 );
	
	if ( ! strcmp( option, "SCALE-FREE" ) )
		if ( par1 > 0 && par2 > 0 )
			return init_scale_free_net( lab, numNodes, par1, par2 );
	
	return 0;
}


/*
	Create a disconnected network, just with nodes and no links.
	Links can be added node by node by the user.
*/

long object::init_discon_net( char const *lab, long numNodes )
{
	long idNode;
	object *cur;
	
	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one
	for ( idNode = 1, cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes aplying ID numbers
	
	return 0;
}


/*
	Create a completely random network with a fixed number of directed links.
	Links/arcs are directed and not reciprocal.
*/

long object::init_random_dir_net( char const *lab, long numNodes, long numLinks )
{
	long idNode, links = 0;
	object *cur, *cur1;
	
	if ( numLinks > ( numNodes * ( numNodes - 1 ) ) )				// test if net is achievable
	{
		plog( "\nError: numLinks > ( numNodes * ( numNodes - 1 ) )" );
		return 0;
	}
	
	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one

	for ( idNode = 1, cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes applying ID numbers
		
	while ( links < numLinks )										// create all links
	{
		cur = draw_node_net( lab );									// draw origin node
		cur1 = draw_node_net( lab );								// draw destination node
		
		if ( cur != cur1 )											// different origin-destination?
			if ( cur->search_link_net( cur1->node->id ) == NULL )	// link doesn't exist yet
			{
				cur->add_link_net( cur1 );							// set link to found new link node ID
				links++;
			}
	}
	
	return links;
}


/*
	Create a completely random network with a fixed number of directed links.
	Links/arcs are reciprocal and form an undirected network.
*/

long object::init_random_undir_net( char const *lab, long numNodes, long numLinks )
{
	long idNode, links = 0;
	object *cur, *cur1;
	
	if ( numLinks > ( numNodes * ( numNodes - 1 ) ) / 2 )			// test if net is achievable
	{
		plog( "\nError: numLinks > ( numNodes * ( numNodes - 1 ) ) / 2" );
		return 0;
	}
	
	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one

	for ( idNode = 1, cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes applying ID numbers
		
	while ( links < numLinks )										// create all links
	{
		cur = draw_node_net( lab );									// draw origin node
		cur1 = draw_node_net( lab );								// draw destination node
		
		if ( cur != cur1 )											// different origin-destination?
			if ( cur->search_link_net( cur1->node->id ) == NULL )	// link doesn't exist yet
			{
				cur->add_link_net( cur1 );							// set link (origin->destination)
				cur1->add_link_net( cur );							// set link (destination->origin)
				links += 2;
			}
	}
	
	return links;
}


/*
	Create a uniform random network with a fixed number of directed links per node.
	The objects representing the nodes must be located inside the current object.
*/

long object::init_uniform_net( char const *lab, long numNodes, long outDeg )
{
	long link, idNode, numLinks, newNode, tryNode;
	object *firstNode, *cur, *cur1;
	
	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one
	firstNode = search( lab );										// start from first node

	for ( idNode = 1, cur = firstNode; cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes aplying ID numbers
		
	numNodes = idNode - 1;											// effective number of nodes
	initturbo( lab, numNodes );										// seed the turbosearch linked list

	for ( numLinks = 0, cur = firstNode; cur != NULL; cur = go_brother( cur ) )
	{
		idNode = cur->node->id;										// current node id
		for ( link = 1; link <= outDeg; link++ )
		{															// run through all node's links
			newNode = 0;
			while( ! newNode || tryNode == idNode )					// while no new link found
			{
				tryNode = (long) rnd_integer( 1, numNodes );		// draw link (other node ID)
				if ( cur->search_link_net( tryNode ) )				// link already exists?
					newNode = 0;									// yes
				else
					newNode = 1;									// no, flag new link
			}
			cur1 = turbosearch( lab, 0, (double) tryNode);			// get target node object
			cur->add_link_net( cur1 );								// set link to found new link node ID
			numLinks++;												// one more link
		}
	}
	return numLinks;
}


/*
	Create a undirected network with random links. The probability of any two 
	nodes being linked is: linkProb. This is the classic Renyi-Erdos network.
	The objects representing the nodes must be located inside the current object.
*/

long object::init_renyi_erdos_net( char const *lab, long numNodes, double linkProb )
{
	long idNode, numLinks, startNode, endNode;
	object *cur, *cur1;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one
	for ( idNode = 1, cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes aplying ID numbers
		
	numNodes = idNode - 1;											// effective number of nodes
	initturbo( lab, numNodes );										// seed the turbosearch linked list

	for ( numLinks = 0, startNode = 1; startNode < numNodes; startNode++ )
	{																// for all nodes except last
		for ( endNode = startNode + 1; endNode <= numNodes; endNode++ )
		{															// and for all higher numbered nodes
			if ( RND < linkProb )									// draws the existence of a link between both
			{
				cur = turbosearch( lab, 0, (double) startNode );	// searches first node object
				cur1 = turbosearch( lab, 0, (double) endNode );		// searches second node object
				cur->add_link_net( cur1 );							// create link start->end
				cur1->add_link_net( cur );							// create link end->start

				numLinks += 2;										// two more links in network
			}
		}
	}
	return numLinks;
}


/*
Create a network placing agents on a circle with avgOutDeg/2 neighbours on each 
side (efficient algorithm). If avgOutDeg is odd, rounds neighbours # down.
*/

long object::init_circle_net( char const *lab, long numNodes, long outDeg )
{
	long link, idNode, numLinks, lowNeigh;
	object *firstNode, *cur, *cur1;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );
																	// creates the missing node objects,
																	// cloning the first one
	firstNode = search( lab );										// start from first node

	for ( idNode = 1, cur = firstNode; cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes aplying ID numbers
		
	numNodes = idNode - 1;											// effective number of nodes
	initturbo( lab, numNodes );										// seed the turbosearch linked list

	for ( numLinks = 0, cur = firstNode; cur != NULL; cur = go_brother( cur ) )
	{
		idNode = cur->node->serNum;									// gets ID of current node
		lowNeigh = idNode - outDeg / 2;								// calculates lower ID neighbour

		for ( link = 1; link <= outDeg; link++ )					// run through all node's links
		{
			while ( lowNeigh == idNode || lowNeigh < 1 || lowNeigh > numNodes )// fix invalid node IDs
			{
				if ( lowNeigh == idNode )							// same target as original node
					lowNeigh++;     								// go up
				if( lowNeigh < 1 )									// too low target node ID
					lowNeigh += numNodes;							// big jump up
				if( lowNeigh > numNodes )							// too high target node ID
					lowNeigh -= numNodes;							// big jump down
			}
		
			cur1 = turbosearch( lab, 0, (double) lowNeigh );		// get target node object
			cur->add_link_net( cur1 );								// set link to current target node ID

			numLinks++;												// one more link
			lowNeigh++;												// next target ID
		}
	}
	return numLinks;
}


/*
	Implement the Small-World rewiring according to the Watts&Strogatz Nature '98 paper. 
	rho is the rewiring parameter.
*/

long object::init_small_world_net( char const *lab, long numNodes, long outDeg, double rho )
{
	long link, idNode, numLinks, numNeigh, tryNode, newNode;
	object *cur, *cur1;
	netLink *curl;

	numLinks = init_circle_net( lab, numNodes, outDeg );			// first generate a circle regular network	

	numNeigh = outDeg / 2;											// number of neighbors (each side)

	for ( cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
																	// scan all nodes
		for ( link = 1; link <= numNeigh; link++ )					// all possible neighbors' node IDs
			if ( RND < rho ) 										// draw rewiring probability
			{														// if rewiring
				idNode = cur->node->serNum;							// get current node ID
				tryNode = idNode + link;							// next node to try
			
				if ( tryNode > numNodes )							// if above max node ID
					tryNode -= numNodes;							// take one round turn
					
				curl = cur->search_link_net( tryNode );				// get dest. link
				if ( curl == NULL || curl->ptrTo == NULL )			// invalid pointers?
					continue;
				else
					cur1 = curl->ptrTo;								// get dest. object pointer
				
				cur1->delete_link_net( cur1->search_link_net ( idNode ) );
																	// remove link to this object
				cur->delete_link_net( cur->search_link_net ( tryNode ) );
																	// and the link from this object
				newNode = idNode;									// look for a new node to create a link
				while( newNode == idNode )
					newNode = (long) rnd_integer( 1, numNodes );	// draw a random int different from this agent
				cur1 = turbosearch( lab, 0, newNode );				// and get new linking node object
		
				cur->add_link_net( cur1 );							// create a new link to the new neighbor
				cur1->add_link_net( cur );							// and vice-versa
			}		
	return numLinks;
}			


/*
	Create a scale-free network with preferential attachment generating a power law 
	distribution of number of links. The procedure can be read as a generalization 
	of Barabasi procedure with two constraints:
	- Fixed number of nodes
	- Arbitrary average number of links.
	
	The procedure consists in a first round scanning all the nodes, and assigning 
	links according to the PA procedure. At the end of this first round it assigns 
	to each node the probability of being chosen. These probabilities are used in 
	subsequent rounds in which all nodes choose new links according to the 
	probabilities fixed at the first round.
*/

long object::init_scale_free_net( char const *lab, long numNodes, long outDeg, double expLink )
{
	long idNode, numLinks, startNode, endNode, nLinks, i;
	double curProb;
	bool node1;
	object *firstNode, *cur, *cur1;
	netLink *cur2;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one
	firstNode = search( lab );										// start from first node
	
	for ( idNode = 1, cur = firstNode; cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes aplying ID numbers
		
	numNodes = idNode - 1;											// effective number of nodes
	initturbo( lab, numNodes );										// seed the turbosearch linked list

	for ( numLinks = 0, node1 = true, cur = firstNode; cur != NULL; cur = go_brother( cur ) )
	{																// run through all nodes (first scan)
		if ( node1 )												// if first node
		{
			node1 = false;											// no more first node
			cur->node->prob = 1;									// assign maximum probability to it
			cur1 = cur;												// save first object pointer
			cur = cur->next;										// point to next node
		}
		else
		{
			curProb = cur->node->prob;								// save current assigned probability					
			cur->node->prob = 0;									// temporarily remove all probability
			cur1 = draw_node_net( lab );							// draw a new destination node
			cur->node->prob = curProb;								// restore probability
		}
		
		nLinks = cur1->node->nLinks + 1;							// updated link counter of new destination
		cur1->node->prob = pow( nLinks, expLink );					// update link probability (new dest. node)
		cur1->add_link_net( cur );									// and add node ID to link in new node
		nLinks = cur->node->nLinks + 1;								// updated link counter of origin
		cur->node->prob = pow( nLinks, expLink );					// update origin node the same way	
		cur->add_link_net( cur1 );									// as the destination node
		
		numLinks += 2;												// two more links in the network
	}

	for ( cur1 = firstNode; cur1 != NULL; cur1 = go_brother( cur1 ) )
																	// run through all nodes (second scan)
		for ( i = 2; i < outDeg; i += 2 ) 							// for all desired number of links
		{
			for ( cur2 = cur1->node->first; cur2 != NULL; cur2 = cur2->next )
			{														// scan node's links
				cur2->probTo = cur2->ptrTo->node->prob;				// get link destination node probability,
																	// assign it to link
				cur2->ptrTo->node->prob = 0;						// and removes destination node from draws
			}
	
			curProb = cur1->node->prob;								// save current node probability
			cur1->node->prob = 0;									// and take it from draws
			cur = draw_rnd( lab );									// draw another node
			cur1->node->prob = curProb;								// and restore node probability
		
			for ( cur2 = cur1->node->first; cur2 != NULL; cur2 = cur2->next )
																	// scan node's links again
				cur2->ptrTo->node->prob = cur2->probTo;				// restore node probability
	
			cur1->add_link_net( cur );								// add link objects, same as before
			cur->add_link_net( cur1 );
			
			numLinks += 2;											// two more links in the network
		}

	for ( cur = firstNode, cur1 = go_brother( cur ); cur != NULL; 
		  cur = cur1, cur1 != NULL ? cur1 = go_brother( cur1 ) : cur1 = cur1 )
																	// then safely remove isolated nodes
		if( cur->node->nLinks == 0 )								// no links?
			cur->delete_obj( );										// remove node

	for ( idNode = 1, cur = firstNode; cur != NULL; idNode++, cur = go_brother( cur ) )
		cur->node->id = idNode;										// make node ID sequential/continuous
		
	return numLinks;
}


/*
	Read directed or undirected network text file in Pajek format.
*/

#define FILE_PATH_LEN 300
#define LINE_BUFFER 300

// Choose separator
#define foldersep( dir ) ( dir[0] == '\0' ? "" : "/" )

void get_line( char *lBuffer, FILE *fPtr )
{
	char firstChar;
	do
	{
		fgets( lBuffer, LINE_BUFFER - 1, fPtr );			// gets next text line
		firstChar = '\0';
		sscanf( lBuffer, " %c", &firstChar );
	}
	while ( firstChar == '%' );								// skipping comments

	if ( firstChar == '*' )									// check new section start
		strupr( lBuffer );									// to uppercase
}

long object::read_file_net( char const *lab, char const dir[] = "", char const base_name[] = "net", 
							int serial = 1, char const ext[] = "net" )
{
	long idNode, numNodes, exNodes, numLinks, startNode, endNode;
	int rd;
	double weight;
	char *p, fileName[FILE_PATH_LEN], textLine[LINE_BUFFER], nameNode[LINE_BUFFER];
	bool inSection;
	object *cur, *cur1;
	netLink *cur2, *cur3;
	FILE *pajekFile;
  
	sprintf( fileName, "%s%s%s_%i.%s", dir, foldersep( dir ), base_name, serial, ext);
															// fully formed file name
	if ( !( pajekFile = fopen( fileName, "r" ) ) )			// open file for reading
	{
		plog( "\nError opening network file: " ); plog( fileName );
		return 0;
	}
 
	for ( exNodes = 0, cur = search( lab ); cur != NULL; exNodes++, cur = go_brother( cur ) );
															// count existing nodes
	if ( exNodes == 0 )
	{
		plog( "\nError: no object exists with name '" ); plog( fileName ); plog( "'" );
		return 0;
	}
		
	numNodes = 0;											// no node read yet
	while ( !feof( pajekFile ) )							// try to read number of vertices
	{
		get_line( textLine, pajekFile );					// gets next text line
		if ( sscanf( textLine, " *VERTICES %ld ", &numNodes ) == 1 )
															// look for section header
			break;											// and get out when find
	}

	if ( numNodes == 0 )									// no nodes to create (or EOF)
	{
		plog( "\nError: empty or invalid network file: " );
		plog( fileName );
		fclose( pajekFile );
		return 0;
	}

	for ( idNode = 1, cur = search( lab ), inSection = true; idNode <= numNodes; idNode++ )
	{														// creates all nodes
		strcpy( nameNode, "" );

		if ( inSection )									// if still in *Vertices
		{
			get_line( textLine, pajekFile );				// gets next text line
			if ( strchr( textLine, '*' ) )					// check new section start
				inSection = false;							// no more *Vertices section
			else											// read attributes
				sscanf( textLine, " %ld \"%[^\"]", &idNode, nameNode );
		}

		if ( idNode > exNodes )								// node does not exist?
			cur = add_n_objects2( lab, 1 );					// create new node object
		
		cur->add_node_net( idNode, nameNode, true );		// add (or reset) net data
	}
  
	numNodes = idNode - 1;									// effective number of nodes

	if ( inSection )										// * was not already read
		get_line( textLine, pajekFile );					// gets next text line
	
	numLinks = 0;											// prepare to count links
	initturbo( lab, numNodes );								// seed the turbosearch linked list
	
	while ( !feof( pajekFile ) )							// while file is not over
	{
		inSection = true;									// assume still inside section

		if ( strstr( textLine, "*ARCS" ) )					// check *Arcs section start
			while ( inSection )								// scan *Arcs section
			{
				get_line( textLine, pajekFile );						// gets next text line
   
   				if ( strchr( textLine, '*' ) || feof( pajekFile ) )		// check new section start or file end
   					inSection = false;									// no more in *Arcs section
   				else																						
				if ( ( rd = sscanf( textLine, " %ld %ld %lf", &startNode, &endNode, &weight ) ) >= 2 )
   					{													// read new arc start/end
   						cur = turbosearch( lab, 0, (double) startNode );// searches first node object
   						cur1 = turbosearch( lab, 0, (double) endNode );	// searches second node object
						cur2 = cur->add_link_net( cur1 );				// add link to network

						if ( rd >=3 )									// is there a weight?
							cur2->weight = weight;

						numLinks++;										// one more link in network
   					}
   			}
		else
			if ( strstr( textLine, "*EDGES" ) )							// check *Edges section start
				while ( inSection )										// scan *Edges section
				{
					get_line( textLine, pajekFile );					// gets next text line
   
   					if ( strchr( textLine, '*' ) || feof( pajekFile ) )	// check new section start or file end
   						inSection = false;								// no more *Edges section
   					else																						
   						if ( ( rd = sscanf( textLine, " %ld %ld %lf", &startNode, &endNode, &weight ) ) >= 2 )
   						{												// read edge start/end
   							cur = turbosearch( lab, 0, (double) startNode );	
																		// searches first node object
   							cur1 = turbosearch( lab, 0, (double) endNode );	
																		// searches second node object
							cur2 = cur->add_link_net( cur1 );			// add links in both directions
							cur3 = cur1->add_link_net( cur );

							if ( rd >=3 )								// is there a weight?
								cur2->weight = cur3->weight = weight;

							numLinks += 2;								// two more links in network
   						}	
   				}
			else														// no more sections
				get_line( textLine, pajekFile );						// gets next text line
	}	
	fclose( pajekFile );
	
	return numLinks;
}


/*
	Write directed network in Pajek text file format.
*/

long object::write_file_net( char const *lab, char const dir[] = "", 
							 char const base_name[] = "net", 
							 int serial = 1, bool append = false )
{
	int tCur = ( t > max_step ) ? max_step : t;					// effective current time
	long numNodes, numLinks = 0;
	double weight;
	char *c, mode[2], fileName[FILE_PATH_LEN], actIntv[64];
	object *firstNode, *cur;
	netLink *cur1;
	FILE *pajekFile;

	sprintf( fileName, "%s%s%s_%i.%s", dir, foldersep( dir ), base_name, serial, 
			 append ? "paj" : "net" );							// fully formed file name
			 
	if ( append && tCur > 1 )									// select write mode
		strcpy( mode, "a" );
	else
		strcpy( mode, "w" );
		
	if ( !( pajekFile = fopen( fileName, mode ) ) )				// create new file
	{
		plog( "\nError creating network file: " ); plog( fileName );
		return  0;
	}
	
	if ( append )
	{
		char name[FILE_PATH_LEN];
		strcpy( name, base_name );
		while ( ( c = strchr( name, ' ' ) ) != NULL )
			c[0] = '_';											// replace space by underscore
		
		fprintf( pajekFile, "\n*Network %s_%d_%d\n", base_name, serial, tCur );	// name network
	}

	firstNode = search( lab );									// pointer to first node
  
	for ( numNodes = 0, cur = firstNode; cur != NULL; 
		  numNodes++, cur = go_brother( cur ) );				// count number of nodes

	fprintf( pajekFile, "*Vertices %lu\n", numNodes);			// start vertices section
  
	for ( cur = firstNode; cur != NULL; cur = go_brother( cur ) )// scan all nodes
	{
		if ( cur->node == NULL )								// not node of a network?
		{
			plog( "\nError: object " ); plog( lab );
			plog( " does not contain the correct network data structure." );
			plog( "\nFile "); plog( fileName ); plog ( " not saved.");
			fclose( pajekFile );
			return 0;
		}
		
		if ( cur->node->name == NULL )							// no name assigned?
			fprintf( pajekFile, "%ld \"%ld\" [%d-%d]\n", cur->node->serNum, 
					 cur->node->id, cur->node->time, tCur );	// output id as name
		else
			fprintf( pajekFile, "%ld \"%s\" [%d-%d]\n", cur->node->serNum, 
					 cur->node->name, cur->node->time, tCur );	// output text name
	}
  
	fprintf( pajekFile, "*Arcs\n" );							// start arcs section
  
	for ( cur = firstNode; cur != NULL; cur = go_brother(cur) )	// scan all nodes
		if ( cur->node->nLinks > 0 )							// if node has at least one link
			for ( cur1 = cur->node->first; cur1 != NULL; cur1 = cur1->next )
			{													// scan all links from node
				weight = ( cur1->weight == 0 ) ? 1 : cur1->weight;
				fprintf( pajekFile, "%ld %ld %g [%d-%d]\n", 
						 cur->node->serNum, cur1->serTo, weight, cur1->time, tCur );
				numLinks++;
			}
	fclose( pajekFile );				
	
	return numLinks;
}


/*
	Delete a network, removing nodes and links.
*/

void object::delete_net( char const *lab )
{
	object *cur;
	
	for ( cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
		cur->delete_node_net( );								// scan all nodes
}
