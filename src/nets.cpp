/*************************************************************

	LSD 8.0 - May 2022
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
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

There are also some alternative network generator algorithms available using:

parent->init_discon_net( lab, numNodes )

parent->init_random_dir_net( lab, numNodes, numLinks )

parent->init_random_undir_net( lab, numNodes, numLinks )

parent->init_uniform_net( lab, numNodes, outDeg )

parent->init_star_net( lab, numNodes )

parent->init_circle_net( lab, numNodes, outDeg )

parent->init_renyi_erdos_net( lab, numNodes, linkProb )

parent->init_small_world_net( lab, numNodes, outDeg, rho )

parent->init_scale_free_net( lab, numNodes, outDeg, expLink )

parent->init_lattice_net( nRow, nCol, lab, eightNeigbr )

The additional parameters for those generators are:

numNodes : number of nodes in network
numLinks : number of arcs (directed links) in network
(avg)outDeg : (average of) arcs (directed links) per node (out degree)
linkProb : probability of link between two nodes
expLink : power degree (power-law networks only)
rho : rewiring link probability (small-world networks only)
nRow : number of rows in the lattice
nCol : number of columns in the lattice
eightNeigbr : eight (true) or four (false) neighbors

Another, more general method for creating networks is (network type is a parameter):

parent->init_stub_net( lab, gen, numNodes, par1, par2 )

gen : "DISCONNECTED" , "RANDOM-DIR" (par1: numLinks), "RANDOM-UNDIR" (par1: numLinks),
	  "UNIFORM" (par1: outDeg), "STAR", "CIRCLE" (par1: outDeg), "RENYI-ERDOS" (par1: outDeg),
	  "SMALL-WORLD" (par1: outDeg, par2: rho), "SCALE-FREE" (par1: outDeg, par2: expLink),
	  "LATTICE" (par1: nCol, par2: eightNeigbr)

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

object->draw_link_net( )
*************************************************************/

#include "decl.h"

/****************************************************
NETLINK
	Initialize new link, at the end of linked list.
****************************************************/
netLink::netLink( object *origNode, object *destNode, double linkWeight, double destProb )
{
	time = t;							// save creation time

	if ( origNode->node == NULL )		// origin is not yet a node?
		origNode->node = new netNode( );// create node data structure

	if ( destNode->node == NULL )		// destination is not yet a node?
		destNode->node = new netNode( );// create node data structure

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


/****************************************************
~NETLINK
	Destroy link, preserving linked list integrity.
****************************************************/
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


/****************************************************
ADD_LINK_NET (*)
	Add new link from LSD object. Does NOT check if
	the link already exists. So, if multiple links
	are to be prevented, caller has to check before calling.
****************************************************/
netLink *object::add_link_net( object *destPtr, double weight, double probTo )
{
	netLink *cur;
	if ( this->up != destPtr->up || strcmp( this->label, destPtr->label ) )
		return NULL;					// different parent or object type?
	cur = new netLink( this, destPtr, weight, probTo );

	return cur;
}


/****************************************************
DELETE_LINK_NET (*)
	Remove link from LSD object.
****************************************************/
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


/****************************************************
SEARCH_LINK_NET (*)
	Search for existing link from LSD object.
	Return pointer to the first link found or NULL
	if link to destination does not exist.
****************************************************/
netLink *object::search_link_net( long destId )
{
	netLink *cur;
	if ( node == NULL )					// no network structure?
		return NULL;
	for ( cur = node->first; 			// scan all links from node
		  cur != NULL && cur->ptrTo->node->id != destId;
		  cur = cur->next);
	if ( cur != NULL && cur->ptrTo->node == NULL )
		return NULL;
	else
		return cur;
}


/****************************************************
DRAW_LINK_NET (*)
	Draw one of the outgoing links of a node randomly,
	with probability equal to probTo.
	Returns NULL if no link exists.
****************************************************/
netLink *object::draw_link_net( void )
{
	double sum, drawPoint, accProb;
	netLink *cur, *cur1;

	if ( node == NULL || node->first == NULL )	// no network structure?
		return NULL;

	for ( sum = 0, cur = node->first; cur != NULL; cur = cur->next )
		if ( cur->ptrTo->node != NULL )			// node still exists?
			sum += cur->probTo;					// add-up probabilities

	if ( ! is_finite( sum ) || sum <= 0 )		// check valid probabilities
	{
		error_hard( "invalid network operation",
					"check your configuration (parameter value) or\ncode (equation constant) to prevent this situation",
					false,
					"probabilities are invalid for link drawing" );
		return node->first;
	}

	do
		drawPoint = ran1( ) * sum;
	while ( drawPoint == sum );					// avoid ran1 == 1

	for ( accProb = 0, cur = cur1 = node->first;// accumulate probabilities
		  accProb <= drawPoint && cur != NULL; cur = cur->next )
	{											// until reaching the right object
		if ( cur->ptrTo->node != NULL )			// node still exists?
			accProb += cur->probTo;
		cur1 = cur;								// save previous object
	}

	return cur1;
}


/****************************************************
NETNODE
	Initialize netNode struct (no links).
****************************************************/
netNode::netNode( long nodeId, const char *nodeName, double nodeProb )
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
		strcpy( name, nodeName );
	}
	else
		name = NULL;

	prob = nodeProb;
}


/****************************************************
~NETNODE
	Destroy netNode struct.
****************************************************/
netNode::~netNode( void )
{
	if ( name != NULL )			// name assigned?
		delete name;

	while ( last != NULL )		// remove all links
		delete last;
}


/****************************************************
ADD_NODE_NET (*)
	Add netNode data structure to LSD object
****************************************************/
object *object::add_node_net( long id, const char nodeName[ ],
							  bool silent )
{
	long serNumOld = -1;

	if ( node != NULL )
	{
		if ( ! silent )
			plog( "\nWarning: existing network data discarded from object." );

		serNumOld = node->serNum;	// save serial number
		delete node;
	}

	node = new netNode( id, nodeName );

	// prevent replacing the serial number
	if ( serNumOld > 0 )
	{
		node->serNum = serNumOld;
		nodesSerial--;
	}

	return this;
}


/****************************************************
DELETE_NODE_NET (*)
	Remove netNode data structure from LSD object.
****************************************************/
void object::delete_node_net( void )
{
	delete node;
	node = NULL;
}


/****************************************************
NAME_NODE_NET (*)
	Set or reset the name of a node.
****************************************************/
void object::name_node_net( const char *nodeName )
{
	if ( node == 0 )				// invalid node?
		return;

	if ( node->name != NULL )		// name already set?
		delete node->name;

	if ( strcmp( nodeName, "" ) )	// name assigned?
	{
		node->name = new char[ strlen( nodeName ) + 1 ];
		strcpy( node->name, nodeName );
	}
	else
		node->name = NULL;
}


/****************************************************
SEARCH_NODE_NET (*)
	Search for existing node. Return pointer to the
	object containing it or NULL if node does not exist.
	Slow for large networks, turbosearch is better in this case.
****************************************************/
object *object::search_node_net( const char *lab, long destId )
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


/****************************************************
STATS_NET (*)
	Returns some basic statistics about the directed network.
	r[ 0 ]: number of nodes
	r[ 1 ]: number of links (arcs)
	r[ 2 ]: average out-degree
	r[ 3 ]: minimum out-degree
	r[ 4 ]: maximum out-degree
	r[ 5 ]: density (including loops)
****************************************************/
double object::stats_net( const char *lab, double *r )
{
	r[ 0 ] = r[ 1 ] = r[ 2 ] = r[ 3 ] = r[ 4 ] = r[ 5 ] = 0;

	object *cur = search( lab );

	if ( cur == NULL || cur->node == NULL )			// invalid network node?
		return NAN;

	for ( ; cur != NULL; cur = go_brother( cur ) )	// scan all nodes
		if ( cur->node != NULL )					// valid node?
		{
			double nLinks = (double) cur->node->nLinks;
			if ( r[ 0 ] == 0. )						// first node?
				r[ 3 ] = nLinks;						// update minimum
			else
				r[ 3 ] = r[ 3 ] < nLinks ? r[ 3 ] : nLinks;

			r[ 0 ]++;
			r[ 1 ] += nLinks;
			r[ 4 ] = r[ 4 ] > nLinks ? r[ 4 ] : nLinks;
		}

	if ( r[ 0 ] > 0. )
	{
		r[ 2 ] = r[ 1 ] / r[ 0 ];
		r[ 5 ] = r[ 1 ] / ( r[ 0 ] * ( r[ 0 ] - 1 ) );
	}

	return r[ 0 ];
}


/****************************************************
DRAW_NODE_NET (*)
	Draw a node randomly, with probability equal to prob.
****************************************************/
object *object::draw_node_net( const char *lab )
{
	double sum, drawPoint, accProb;
	object *cur, *cur1, *cur2;

	// make sure this is being called from the parent (container) object
	cur1 = cur = search( lab );
	if ( cur == NULL )
		return NULL;

	for ( sum = 0; cur != NULL && cur->node != NULL; cur = cur->next )
													// add-up probabilities
		sum += cur->node->prob;

	if ( ! is_finite( sum ) || sum <= 0 )			// check valid probabilities
	{
		error_hard( "invalid network operation",
					"check your configuration (parameter value) or\ncode (equation constant) to prevent this situation",
					false,
					"probabilities are invalid for node drawing" );
		return cur1;
	}

	do
		drawPoint = ran1( ) * sum;
	while ( drawPoint == sum );						// avoid ran1 == 1

	for ( accProb = 0, cur = cur2 = cur1;			// accumulate probabilities
		  accProb <= drawPoint && cur != NULL; 		// until reaching the right object
		  accProb += cur->node->prob, cur = cur->next )
		cur2 = cur;									// save previous object

	return cur2;
}


/****************************************************
SHUFFLE_NODES_NET (*)
	Shuffle nodes order in the linked list of node objects.
	Use Fischer-Yates shuffling algorithm.
****************************************************/
object *object::shuffle_nodes_net( const char *lab )
{
	long i, j, iId, jId, numNodes;
	object *cur, *cur1;

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return NULL;

	for ( numNodes = 0; cur != NULL;
		  numNodes++, cur = go_brother( cur ) );	// count number of nodes

	initturbo( lab, numNodes );						// seed the turbosearch linked list

	for ( i = numNodes; i > 1; i-- )				// run the shuffling
	{
		j = (long) uniform_int( 1, i );
		cur = turbosearch( lab, 0, (double) i );
		cur1 = turbosearch( lab, 0, (double) j );

		if ( cur->node == NULL || cur1->node == NULL )
		{
			error_hard( "invalid network object",
						"check your equation code to add\nthe network structure before using this macro",
						true,
						"object '%s' has no network data structure", lab  );
			return NULL;
		}

		iId = cur->node->id;
		jId = cur1->node->id;
		cur->node->id = jId;
		cur1->node->id = iId;
	}

	lsdqsort( lab, NULL, "UP", 0 );					// sort according to shuffled IDs

	return search( lab );
}


/****************************************************
NODES2CREATE
	Calculate the missing number of object copies.
	Prints a warning if there are more
	existing copies than needed and returns 0.
****************************************************/
long nodes2create( object *parent, const char *lab, long numNodes )
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


/****************************************************
INIT_STUB_NET (*)
	Stub function to call the appropriate network generator.
****************************************************/
double object::init_stub_net( const char *lab, const char* gen, long numNodes, long par1, double par2 )
{
	char option[ 32 ];

	strcpyn( option, gen, 32 );
	strupr( option );

	// auto set all instances as nodes if necessary
	if ( numNodes <= 0 )
		numNodes = count( lab );

	// must have a label, and two nodes except is a disconnected network (1 node minimum)
	if ( ( numNodes < 2 && strcmp( option, "DISCONNECTED" ) ) || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your equation code to prevent this situation",
					true,
					"invalid parameter values for a %s network in object '%s'", option, lab );
		return 0;
	}

	if ( ! strcmp( option, "DISCONNECTED" ) )
			return init_discon_net( lab, numNodes );

	if ( ! strcmp( option, "CONNECTED" ) )
			return init_connect_net( lab, numNodes );

	if ( ! strcmp( option, "RANDOM-DIR" ) )
		if ( par1 > 0 )
			return init_random_dir_net( lab, numNodes, par1 );

	if ( ! strcmp( option, "RANDOM-UNDIR" ) )
		if ( par1 > 0 )
			return init_random_undir_net( lab, numNodes, par1 );

	if ( ! strcmp( option, "UNIFORM" ) )
		if ( par1 > 0 )
			return init_uniform_net( lab, numNodes, par1 );

	if ( ! strcmp( option, "STAR" ) )
			return init_star_net( lab, numNodes );

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

	if ( ! strcmp( option, "LATTICE" ) )
		if ( numNodes % par1 == 0 && par1 > 0 )
			return init_lattice_net( numNodes / par1, par1, lab, ( bool ) par2 );

	error_hard( "cannot create network",
				"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
				true,
				"invalid parameter values for a %s network in object '%s'", option, lab );
	return 0;
}


/****************************************************
INIT_DISCON_NET
	Create a disconnected network, just with nodes and no links.
	Links can be added node by node by the user.
****************************************************/
long object::init_discon_net( const char *lab, long numNodes )
{
	long idNode;
	object *cur;

	if ( numNodes < 1 || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for disconnected network in object '%s'", lab );
		return -1;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return -1;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one
	for ( idNode = 1; cur != NULL; cur = go_brother( cur ), ++idNode )
		cur->add_node_net( idNode );								// scan all nodes aplying ID numbers

	return 0;
}


/****************************************************
INIT_CONNECT_NET
	Create a fully connected undirected network.
	All links/arcs are reciprocal.
****************************************************/
long object::init_connect_net( const char *lab, long numNodes )
{
	long idNode, links = 0;
	object *cur, *cur1, *cur2;

	if ( numNodes < 2 || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for fully connected network in object '%s'", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	add_n_objects2( lab, nodes2create( this, lab, numNodes ) );		// creates the missing node objects,
																	// cloning the first one

	for ( idNode = 1; cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes applying ID numbers

	for ( cur1 = search( lab ), links = 0; cur1 != NULL; cur1 = go_brother( cur1 ) )
		for ( cur2 = go_brother( cur1 ); cur2 != NULL; cur2 = go_brother( cur2 ) )
		{
			cur1->add_link_net( cur2 );		// arc from hub to spoke
			cur2->add_link_net( cur1 );		// arc from spoke to hub

			links += 2;
		}

	return links;
}


/****************************************************
INIT_STAR_NET
	Create a star network, first object in the chain is the hub.
	All other objects are spokes with bi-directional links to hub.
****************************************************/
long object::init_star_net( const char *lab, long numNodes )
{
	long links;
	object *cur1, *cur2;

	// first build a disconnected network
	if ( init_discon_net( lab, numNodes ) != 0 )
		return 0;

	cur1 = search( lab );				// save hub

	for ( cur2 = go_brother( cur1 ), links = 0; cur2 != NULL;
		 cur2 = go_brother( cur2 ) )	// create the strokes
	{
		cur1->add_link_net( cur2 );		// arc from hub to spoke
		cur2->add_link_net( cur1 );		// arc from spoke to hub

		links += 2;
	}

	return links;
}


/****************************************************
INIT_RANDOM_DIR_NET
	Create a completely random network with a fixed
	number of directed links.
	Links/arcs are directed and not reciprocal.
****************************************************/
long object::init_random_dir_net( const char *lab, long numNodes, long numLinks )
{
	long idNode, links = 0;
	object *cur, *cur1;

	if ( numNodes < 2 || numLinks < 0 || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for random directed network in object '%s'", lab );
		return 0;
	}

	if ( numLinks > ( numNodes * ( numNodes - 1 ) ) )				// test if net is achievable
	{
		error_hard( "cannot create network",
					"check your configuration (parameter value) or\ncode (equation constant) to prevent this situation",
					false,
					"object '%s' has numLinks > ( numNodes * ( numNodes - 1 ) )", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	add_n_objects2( lab, nodes2create( this, lab, numNodes ) );		// creates the missing node objects,
																	// cloning the first one

	for ( idNode = 1; cur != NULL; cur = go_brother( cur ) )
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


/****************************************************
INIT_RANDOM_UNDIR_NET
	Create a completely random network with a fixed
	number of directed links. Links/arcs are reciprocal
	to form an undirected network.
****************************************************/
long object::init_random_undir_net( const char *lab, long numNodes, long numLinks )
{
	long idNode, links = 0;
	object *cur, *cur1;

	if ( numNodes < 2 || numLinks < 0 || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for random undirected network in object '%s'", lab );
		return 0;
	}

	if ( numLinks > ( numNodes * ( numNodes - 1 ) ) / 2 )			// test if net is achievable
	{
		error_hard( "cannot create network",
					"check your configuration (parameter value) or\ncode (equation constant) to prevent this situation",
					false,
					"object '%s' has numLinks > ( numNodes * ( numNodes - 1 ) ) / 2", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one

	for ( idNode = 1; cur != NULL; cur = go_brother( cur ) )
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


/****************************************************
INIT_UNIFORM_NET
	Create a uniform random network with a fixed number
	of directed links per node. The objects representing
	the nodes must be located inside the current object.
****************************************************/
long object::init_uniform_net( const char *lab, long numNodes, long outDeg )
{
	bool newNode;
	long link, idNode, numLinks, tryNode;
	object *firstNode, *cur, *cur1;

	if ( numNodes < 2 || outDeg < 0 || outDeg >= numNodes || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for uniform random network in object '%s'", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	firstNode = cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one
	for ( idNode = 1; cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes aplying ID numbers

	numNodes = idNode - 1;											// effective number of nodes
	initturbo( lab, numNodes );										// seed the turbosearch linked list

	for ( numLinks = 0, cur = firstNode; cur != NULL; cur = go_brother( cur ) )
	{
		idNode = cur->node->id;										// current node id
		for ( link = 1; link <= outDeg; link++ )
		{															// run through all node's links
			newNode = false;
			tryNode = idNode;
			while ( ! newNode || tryNode == idNode )				// while no new link found
			{
				tryNode = (long) uniform_int( 1, numNodes );		// draw link (other node ID)
				if ( cur->search_link_net( tryNode ) )				// link already exists?
					newNode = false;								// yes
				else
					newNode = true;									// no, flag new link
			}
			cur1 = turbosearch( lab, 0, (double) tryNode );			// get target node object
			cur->add_link_net( cur1 );								// set link to found new link node ID
			numLinks++;												// one more link
		}
	}
	return numLinks;
}


/****************************************************
INIT_RENYI_ERDOS_NET
	Create a undirected network with random links. The probability of any two
	nodes being linked is: linkProb. This is the classic Renyi-Erdos network.
	The objects representing the nodes must be located inside the current object.
****************************************************/
long object::init_renyi_erdos_net( const char *lab, long numNodes, double linkProb )
{
	long idNode, numLinks, startNode, endNode;
	object *cur, *cur1;

	if ( numNodes < 2 || linkProb < 0 || linkProb > 1 || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for Renyi-Erdos network in object '%s'", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one
	for ( idNode = 1; cur != NULL; cur = go_brother( cur ) )
		cur->add_node_net( idNode++ );								// scan all nodes aplying ID numbers

	numNodes = idNode - 1;											// effective number of nodes
	initturbo( lab, numNodes );										// seed the turbosearch linked list

	for ( numLinks = 0, startNode = 1; startNode < numNodes; startNode++ )
	{																// for all nodes except last
		for ( endNode = startNode + 1; endNode <= numNodes; endNode++ )
		{															// and for all higher numbered nodes
			if ( ran1( ) < linkProb )								// draws the existence of a link between both
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


/****************************************************
INIT_CIRCLE_NET
	Create a network placing agents on a circle with avgOutDeg/2
	neighbours on each side (efficient algorithm). If avgOutDeg
	is odd, rounds neighbours # down.
****************************************************/
long object::init_circle_net( const char *lab, long numNodes, long outDeg )
{
	long link, idNode, numLinks, lowNeigh;
	object *firstNode, *cur, *cur1;

	if ( numNodes < 2 || outDeg < 0 || outDeg >= numNodes || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for circle network in object '%s'", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	firstNode = cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );
																	// creates the missing node objects,
																	// cloning the first one
	for ( idNode = 1; cur != NULL; cur = go_brother( cur ) )
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
				if ( lowNeigh < 1 )									// too low target node ID
					lowNeigh += numNodes;							// big jump up
				if ( lowNeigh > numNodes )							// too high target node ID
					lowNeigh -= numNodes;							// big jump down
			}

			cur1 = turbosearch( lab, 0, (double) lowNeigh );		// get target node object

			if ( cur->search_link_net( cur1->node->id ) == NULL )	// link doesn't exist yet
			{
				cur->add_link_net( cur1 );							// set link to current target node ID
				numLinks++;											// one more link
			}

			lowNeigh++;												// next target ID
		}
	}
	return numLinks;
}


/****************************************************
INIT_SMALL_WORLD_NET
	Implement the Small-World rewiring according to
	the Watts&Strogatz Nature '98 paper.
	rho is the rewiring parameter.
****************************************************/
long object::init_small_world_net( const char *lab, long numNodes, long outDeg, double rho )
{
	long link, idNode, numLinks, numNeigh, tryNode, newNode;
	object *cur, *cur1;
	netLink *curl;

	if ( numNodes < 2 || outDeg < 0 || outDeg >= numNodes || rho < 0 || rho > 1 || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for Small-World network in object '%s'", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	numLinks = init_circle_net( lab, numNodes, outDeg );			// first generate a circle regular network

	numNeigh = outDeg / 2;											// number of neighbors (each side)

	for ( ; cur != NULL; cur = go_brother( cur ) )
																	// scan all nodes
		for ( link = 1; link <= numNeigh; link++ )					// all possible neighbors' node IDs
			if ( ran1( ) < rho ) 									// draw rewiring probability
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
				while ( newNode == idNode )
					newNode = (long) uniform_int( 1, numNodes );	// draw a random int different from this agent
				cur1 = turbosearch( lab, 0, newNode );				// and get new linking node object

				cur->add_link_net( cur1 );							// create a new link to the new neighbor
				cur1->add_link_net( cur );							// and vice-versa
			}
	return numLinks;
}


/****************************************************
INIT_SCALE_FREE_NET
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
****************************************************/
long object::init_scale_free_net( const char *lab, long numNodes, long outDeg, double expLink )
{
	long idNode, numLinks, nLinks, i;
	double curProb;
	bool node1;
	object *firstNode, *cur, *cur1;
	netLink *cur2;

	if ( numNodes < 2 || outDeg < 0 || outDeg >= numNodes || expLink <= 0 || lab == NULL )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for scale-free network in object '%s'", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	firstNode = cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
																	// cloning the first one
	for ( idNode = 1; cur != NULL; cur = go_brother( cur ) )
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
		  cur = cur1, cur1 != NULL ? cur1 = go_brother( cur1 ) : cur = cur1 )
																	// then safely remove isolated nodes
		if ( cur->node->nLinks == 0 )								// no links?
			cur->delete_obj( );										// remove node

	for ( idNode = 1, cur = firstNode; cur != NULL; idNode++, cur = go_brother( cur ) )
		cur->node->id = idNode;										// make node ID sequential/continuous

	return numLinks;
}


/****************************************************
INIT_LATTICE_NET
	Generates a lattice, a regular square network where each cell in row i and
	column j is connected to its 4 or 8 neighbours, depending on an optional parameter
	The links are generated clockwise starting from "North", that is cell (i-1, j),
	then either East or Northeast, depending on the final option, cells (i,j+1) or
	(i-1,j+1), respectively.
	The lattice is a torus, i.e. cells at the borders are connected to the opposite
	border.
****************************************************/
long object::init_lattice_net( int nRow, int nCol, const char *lab, int eightNeigbr )
{
	long idNode, i, j, h, numNodes = nRow * nCol, numLinks = 0;
	object *cur, *cur1;

	eightNeigbr = ( eightNeigbr == 4 ) ? 0 : ( eightNeigbr == 8 ) ? 1 : eightNeigbr;

	if ( nRow <= 0 || nCol <= 0 || lab == NULL || ( eightNeigbr != 0 && eightNeigbr != 1 ) )
	{
		error_hard( "cannot create network",
					"check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
					true,
					"invalid parameter values for lattice network in object '%s'", lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab );
	if ( cur == NULL )
		return 0;

	add_n_objects2( lab , nodes2create( this, lab, numNodes ) );
													// creates the missing node objects,
													// cloning the first one
	for ( i = j = 0, cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
	{												// scan all nodes aplying ID numbers
		idNode = nCol * i + j + 1;

		cur->add_node_net( idNode );
		if ( ++j >= nCol )
		{
			i++;
			j = 0;
		}
	}

	initturbo( lab, numNodes );						// seed the turbosearch linked list

	for ( i = j = 0, cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
	{
		h = nCol * ( i == 0 ? nRow - 1 : i - 1 ) + j + 1;	// north
		cur1 = turbosearch( lab, numNodes, h );
		cur->add_link_net( cur1, 0, 1 );

		if ( eightNeigbr )
		{
			h = nCol * ( i == 0 ? nRow - 1 : i - 1 ) + ( j == nCol - 1 ? 0 : j + 1 ) + 1;
			cur1 = turbosearch( lab, numNodes, h );			// northeast
			cur->add_link_net( cur1, 0, 1 );
		}

		h = nCol * i + ( j == nCol - 1 ? 0 : j + 1 ) + 1;	// east
		cur1 = turbosearch( lab, numNodes, h );
		cur->add_link_net( cur1, 0, 1 );

		if ( eightNeigbr )
		{
			h = nCol * ( i == nRow - 1 ? 0 : i + 1 ) + ( j == nCol - 1 ? 0 : j + 1 ) + 1;
			cur1 = turbosearch( lab, numNodes, h );			// southeast
			cur->add_link_net( cur1, 0, 1 );
		}

		h = nCol * ( i == nRow - 1 ? 0 : i + 1 ) + j + 1;	// south
		cur1 = turbosearch( lab, numNodes, h );
		cur->add_link_net( cur1, 0, 1 );

		if ( eightNeigbr )
		{
			h = nCol * ( i == nRow - 1 ? 0 : i + 1 ) + ( j == 0 ? nCol - 1 : j - 1 ) + 1;
			cur1 = turbosearch( lab, numNodes, h );			// southwest
			cur->add_link_net( cur1, 0, 1 );
		}

		h = nCol * i + ( j == 0 ? nCol - 1 : j - 1 ) + 1;	// west
		cur1 = turbosearch( lab, numNodes, h );
		cur->add_link_net( cur1, 0, 1 );

		if ( eightNeigbr )
		{
			h = nCol * ( i == 0 ? nRow - 1 : i - 1 ) + ( j == 0 ? nCol - 1 : j - 1 )  + 1;
			cur1 = turbosearch( lab, numNodes, h );			// northwest
			cur->add_link_net( cur1, 0, 1 );

			numLinks += 8;
		}
		else
			numLinks += 4;

		if ( ++j >= nCol )
		{
			++i;
			j = 0;
		}
	}

	return numLinks;
}


/****************************************************
READ_FILE_NET (*)
	Read directed or undirected network text file in Pajek format.
****************************************************/
void get_line( char *lBuffer, FILE *fPtr )
{
	char firstChar;
	do
	{
		fgets( lBuffer, MAX_LINE_SIZE, fPtr );				// gets next text line
		firstChar = '\0';
		sscanf( lBuffer, " %c", &firstChar );
	}
	while ( firstChar == '%' );								// skipping comments

	if ( firstChar == '*' )									// check new section start
		strupr( lBuffer );									// to uppercase
}

double object::read_file_net( const char *lab, const char dir[ ], const char base_name[ ],
							int serial, const char ext[ ] )
{
	long idNode, numNodes, countNodes, numLinks, startNode, endNode;
	int rd;
	double weight;
	char fileName[ MAX_PATH_LENGTH ], textLine[ MAX_LINE_SIZE ], nameNode[ MAX_LINE_SIZE ];
	bool inSection;
	object *cur, *cur1;
	netLink *cur2, *cur3;
	FILE *pajekFile;

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( this, lab, serial < 0 );
	if ( cur == NULL )
		return 0;

	if ( serial >= 0 )
		snprintf( fileName, MAX_PATH_LENGTH, "%s%s%s_%i%s%s", dir, foldersep( dir ), base_name, serial, strlen( ext ) == 0 ? "" : ".", ext );	// fully formed file name
	else
		snprintf( fileName, MAX_PATH_LENGTH, "%s%s%s%s%s", dir, foldersep( dir ), base_name,  strlen( ext ) == 0 ? "" : ".", ext );

	if ( ! ( pajekFile = fopen( fileName, "r" ) ) )			// open file for reading
	{
		if ( serial >= 0 )									// interactive mode - handle in interf.cpp
			error_hard( "network file error",
						"check if the file requested in equation code exists",
						true,
						"cannot open network file '%s'", fileName );
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
		fclose( pajekFile );
		if ( serial >= 0 )									// interactive mode - handle in interf.cpp
			error_hard( "network file error",
						"check the requested file content",
						false,
						"empty or invalid network file '%s'", fileName );
		return 0;
	}

	for ( countNodes = 1, inSection = true; countNodes <= numNodes;
		  ++countNodes, cur = go_brother( cur ) )
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

		if ( cur == NULL )									// node does not exist?
			cur = add_n_objects2( lab, 1 );					// create new node object

		cur->add_node_net( idNode, nameNode, true );		// add (or reset) net data
	}

	numNodes = countNodes - 1;								// effective number of nodes

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


/****************************************************
WRITE_FILE_NET (*)
	Write directed network in Pajek text file format.
****************************************************/
double object::write_file_net( const char *lab, const char dir[ ], const char base_name[ ],
							 int serial, bool append )
{
	int tCur = ( t > max_step ) ? max_step : t;				// effective current time
	long numNodes, numLinks = 0;
	double weight;
	char *c, mode[ 2 ], fileName[ MAX_PATH_LENGTH ], name[ MAX_PATH_LENGTH ];
	object *firstNode, *cur;
	netLink *cur1;
	FILE *pajekFile;

	// make sure this is being called from the parent (container) object
	firstNode = cur = check_net_struct( this, lab, serial < 0 );
	if ( cur == NULL )
		return 0;

	if ( serial >= 0 )
		snprintf( fileName, MAX_PATH_LENGTH, "%s%s%s_%i.%s", dir, foldersep( dir ), base_name, serial, append ? "paj" : "net" );				// fully formed file name
	else
		snprintf( fileName, MAX_PATH_LENGTH, "%s%s%s.%s", dir, foldersep( dir ), base_name, append ? "paj" : "net" );

	if ( append && tCur > 1 )								// select write mode
		strcpy( mode, "a" );
	else
		strcpy( mode, "w" );

	if ( ! ( pajekFile = fopen( fileName, mode ) ) )		// create new file
	{
		if ( serial >= 0 )									// interactive mode - handle in interf.cpp
			error_hard( "network file error",
						"check disk space and permissions",
						false,
						"cannot create network file '%s'", fileName );
		return 0;
	}

	if ( append )
	{
		strcpyn( name, base_name, MAX_PATH_LENGTH );
		while ( ( c = strchr( name, ' ' ) ) != NULL )
			c[ 0 ] = '_';										// replace space by underscore

		fprintf( pajekFile, "\n*Network %s_%d_%d\n", base_name, serial, tCur );	// name network
	}
	else
		fprintf( pajekFile, "%% %s objects from LSD '%s' configuration\n", lab, strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME );

	for ( numNodes = 0; cur != NULL;
		  numNodes++, cur = go_brother( cur ) );			// count number of nodes

	fprintf( pajekFile, "*Vertices %lu\n", numNodes);		// start vertices section

	for ( cur = firstNode; cur != NULL; cur = go_brother( cur ) )// scan all nodes
	{
		if ( cur->node == NULL )							// not node of a network?
		{
			fclose( pajekFile );
			if ( serial >= 0 )								// interactive mode - handle in interf.cpp
				error_hard( "invalid network object",
							"check your equation code to add\nthe network structure before using this macro",
							true,
							"object '%s' has no network data structure, file '%s' not saved", lab, fileName );
			return 0;
		}

		if ( cur->node->name == NULL )						// no name assigned?
			fprintf( pajekFile, "%ld \"%ld\" [%d-%d]\n", cur->node->serNum,
					 cur->node->id, cur->node->time, tCur );	// output id as name
		else
			fprintf( pajekFile, "%ld \"%s\" [%d-%d]\n", cur->node->serNum,
					 cur->node->name, cur->node->time, tCur );	// output text name
	}

	fprintf( pajekFile, "*Arcs\n" );						// start arcs section

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


/****************************************************
DELETE_NET (*)
	Delete a network, removing nodes and links.
****************************************************/
void object::delete_net( const char *lab )
{
	object *cur;

	for ( cur = search( lab ); cur != NULL; cur = go_brother( cur ) )
		cur->delete_node_net( );								// scan all nodes
}


/****************************************************
CHECK_NET_STRUCT
	Check the contextual objects structure.
	The calling object has to be a immediate parent of the existing object named 'lab'.
	Root cannot be the calling object (not a valid network container).
****************************************************/
object *check_net_struct( object *caller, const char *nodeLab, bool noErr )
{
	object *cur = caller->search( nodeLab );

	if ( cur == NULL )
	{
		if ( ! noErr )									// interactive mode - handle in interf.cpp
			error_hard( "object not found",
						"create object in model structure",
						false,
						"object '%s' is missing", nodeLab );
		return NULL;
	}

	if ( cur->up == NULL )
	{
		if ( ! noErr )									// interactive mode - handle in interf.cpp
			error_hard( "invalid network data structure",
						"check your model structure to prevent this situation",
						false,
						"cannot create network at the Root level" );
		return NULL;
	}

	if ( strcmp( cur->up->label, caller->label ) )
	{
		if ( ! noErr )									// interactive mode - handle in interf.cpp
			error_hard( "invalid network data structure",
						"check your model structure to prevent this situation",
						false,
						"no descending object '%s' in container object '%s'", nodeLab, caller->label );
		return NULL;
	}

	return cur;
}
