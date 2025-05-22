/*************************************************************

	LSD 9.0 - January 2024
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 NETS.CPP
 Network tools: functions to load, generate and save networks
 in LSD

 v1: initial compilation by Marcelo Pereira
 v2: full integration with LSD

 All functions work on specially defined LSD object's data
 structures (named here as "node"), with the following
 organization:

 object --+-- node --+- nodeID (long) : node unique ID number (re-orderable)
 					+- serial (long) : node sequential serial number (reset on save)
 					+- nlinks (long) : number of arcs FROM node
 					+- first (ptr) : pointer to the first outgoing link
 					+- last(ptr) : pointer to the last outgoing link
 					+- prob (double) : assigned node probability (power-law)
 					|
 					+-- link --+- from (ptr) : pointer to origin node
 					           +- to (ptr) : pointer to destination
 							   +- prev (ptr) : pointer to previous link or NULL
 							   +- next (ptr) : pointer to next link or NULL
 							   +- weight (double) : link weight
 							   +- probTo (double) : dest. node prob. (power-law)

 Objects with "node" equal to NULL are not elements of a
 network. Currently, each object can be part of only one
 network.

 Networks can be loaded and saved from/to Pajek-formatted
 files. Network objects are formatted as directed graphs
 (undirected links represented by two directed arcs in
 opposite directions). The available methods are:

 - parent->read_file_net( lab, dir, base_name, serial, ext )
 - parent->write_file_net( lab, dir, base_name, serial, ext )

 Where "parent" is the object where to search for "lab",
 that is the name of the object that will be used as the
 container for the nodes in the network. If the existing
 number of objects "lab" is less than the required number,
 the missing objects are automatically created. "dir" is
 the folder where the Pajek network file is located. The
 format for the network file name is
 "<base_name>_<serial>.<ext>".
 If multiple simulation runs are used, <serial> is
 incremented sequentially.

 There are also some alternative network generator
 algorithms available using:

 - parent->init_discon_net( lab, numNodes )
 - parent->init_random_dir_net( lab, numNodes, numLinks )
 - parent->init_random_undir_net( lab, numNodes, numLinks )
 - parent->init_uniform_net( lab, numNodes, outDeg )
 - parent->init_star_net( lab, numNodes )
 - parent->init_circle_net( lab, numNodes, outDeg )
 - parent->init_renyi_erdos_net( lab, numNodes, linkProb )
 - parent->init_small_world_net( lab, numNodes, outDeg, rho )
 - parent->init_scale_free_net( lab, numNodes, outDeg, expLink )
 - parent->init_lattice_net( nRow, nCol, lab, eightNeigbr )

 The additional parameters for those generators are:

 numNodes : number of nodes in network
 numLinks : number of arcs (directed links) in network
 (avg)outDeg : (average of) arcs (directed links) per node
               (out degree)
 linkProb : probability of link between two nodes
 expLink : power degree (power-law networks only)
 rho : rewiring link probability (small-world networks only)
 nRow : number of rows in the lattice
 nCol : number of columns in the lattice
 eightNeigbr : eight (true) or four (false) neighbors

 Another, more general method for creating networks is
 (network type is a parameter):

 - parent->init_stub_net( lab, gen, numNodes, par1, par2 )

 gen : "DISCONNECTED",
       "RANDOM-DIR" (par1: numLinks),
       "RANDOM-UNDIR" (par1: numLinks),
	   "UNIFORM" (par1: outDeg),
	   "STAR", "CIRCLE" (par1: outDeg),
	   "RENYI-ERDOS" (par1: outDeg),
 	   "SMALL-WORLD" (par1: outDeg, par2: rho),
	   "SCALE-FREE" (par1: outDeg, par2: expLink),
 	   "LATTICE" (par1: nCol, par2: eightNeigbr)

 All generators return the effective number of directed
 links (arcs) of the generated network. According to the
 generator used, the network may have to be reshuffled
 before use, applying the method:

 - parent->shuffle_net( lab )

 Reshuffling reassigns nodeIDs and the network object
 linked list order but does not change original node's
 serial numbers or the network structure.

 Other methods to directly manipulate nodes data and
 links:

 - object->add_node_net( id, name )
 - object->delete_node_net( void )
 - parent->search_node_net( lab, destId )
 - parent->draw_node_net( lab )
 - object->add_link_net( destPtr )
 - object->delete_link_net( destPtr )
 - object->search_link_net( destId )
 - object->draw_link_net( )
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 NETLINK
 Initialize new link, at the end of linked list.
 *************************************************************/
lsd::netlink::netlink( object *origNode, object *destNode, double linkWeight, double destProb )
{
	time = origNode->attr->cont->sim->t;			// save creation time

	if ( origNode->node == NULL )					// origin is not yet a node?
		origNode->node = new netnode( origNode );	// create one

	if ( destNode->node == NULL )					// destination is not yet a node?
		destNode->node = new netnode( destNode );	// create one

	to = destNode;
	from = origNode;
	prev = from->node->last;
	weight = linkWeight;
	probTo = destProb;

	if ( from->node->first == NULL )				// first link?
		from->node->first = this;
	else											// insert after last
		from->node->last->next = this;
	from->node->last = this;
	from->node->nlinks++;
}


/*************************************************************
 ~NETLINK
 Destroy link, preserving linked list integrity.
 *************************************************************/
lsd::netlink::~netlink( void )
{
	if ( from->node->first != this && from->node->last != this )
	{												// not first nor last link?
		prev->next = next;
		next->prev = prev;
	}
	else
		if ( from->node->first == this && from->node->last == this )
													// last link?
			from->node->first = from->node->last = NULL;
		else
			if ( from->node->first == this )		// first link?
			{
				from->node->first = next;
				next->prev = NULL;
			}
			else									// last link?
			{
				from->node->last = prev;
				prev->next = NULL;
			}

	from->node->nlinks--;
}


/*************************************************************
 ADD_LINK_NET (*)
 Add new link from LSD object. Does NOT check if
 the link already exists. So, if multiple links
 are to be prevented, caller has to check before calling.
 *************************************************************/
lsd::netlink *lsd::object::add_link_net( object *destPtr, double weight, double probTo )
{
	netlink *cur;
	if ( up != destPtr->up || attr != destPtr->attr )
		return NULL;								// different parent or object type?
	cur = new netlink( this, destPtr, weight, probTo );

	return cur;
}


/*************************************************************
 ADD_LINK_NET
 Add new link between two nodes by IDs.
 Does NOT check if the link already exists. So,
 if multiple links are to be prevented, caller
 has to check before calling.
 Mode numbers must start from 1 and contiguous,
 numbers refer to the object position in the
 nodes' brotherhood.
 *************************************************************/
lsd::netlink *lsd::object::add_link_net( const char *nodeName, long startNode, long endNode, double weight, double probTo, bool edge )
{
	netlink *curl = NULL;
	object *cur, *cur1;

	if ( ! turboset( nodeName ) )					// initialize if needed
		initturbo( nodeName );

	cur = turbosearch( nodeName, ( double ) startNode );// searches first node object

	if ( cur->node == NULL || cur->node->id != startNode )
		attr->cont->sim->plog( "\nWarning: invalid %s origin (%ld to %ld), ignored", edge ? "edge" : "arc", startNode, endNode );
	else
	{
		cur1 = turbosearch( nodeName, ( double ) endNode );// searches second node object

		if ( cur1->node == NULL || cur1->node->id != endNode )
			attr->cont->sim->plog( "\nWarning: invalid %s destination (%ld to %ld), ignored", edge ? "edge" : "arc", startNode, endNode );
		else
		{
			curl = cur->add_link_net( cur1, weight, probTo );// add link(s) to network

			if ( edge )
				cur1->add_link_net( cur, weight, probTo );
		}
	}

	return curl;
}


/*************************************************************
 DELETE_LINK_NET (*)
 Remove link from LSD object.
 *************************************************************/
void lsd::object::delete_link_net( netlink *ptr )
{
	netlink *cur;
	if ( node == NULL || ptr == NULL )		// no network structure or invalid ptr?
		return;
	for ( cur = node->first; 				// scan all links from node
		  cur != NULL && cur != ptr; 		// to make sure pointer belongs to node
		  cur = cur->next);
	if ( cur != NULL )
		delete cur;
}


/*************************************************************
 SEARCH_LINK_NET (*)
 Search for existing link from LSD object.
 Return pointer to the first link found or NULL
 if link to destination does not exist.
 *************************************************************/
lsd::netlink *lsd::object::search_link_net( long destId )
{
	netlink *cur;
	if ( node == NULL )								// no network structure?
		return NULL;
	for ( cur = node->first; 						// scan all links from node
		  cur != NULL && cur->to->node->id != destId;
		  cur = cur->next);
	if ( cur != NULL && cur->to->node == NULL )
		return NULL;
	else
		return cur;
}


/*************************************************************
 DRAW_LINK_NET (*)
 Draw one of the outgoing links of a node randomly,
 with probability equal to probTo.
 Returns NULL if no link exists.
 *************************************************************/
lsd::netlink *lsd::object::draw_link_net( void )
{
	double sum, drawPoint, accProb;
	netlink *cur, *cur1;

	if ( node == NULL || node->first == NULL )		// no network structure?
		return NULL;

	for ( sum = 0, cur = node->first; cur != NULL; cur = cur->next )
		if ( cur->to->node != NULL )				// node still exists?
			sum += cur->probTo;						// add-up probabilities

	if ( ! std::isfinite( sum ) || sum <= 0 )		// check valid probabilities
	{
		attr->cont->sim->error_hard( "invalid network operation",
									 "check your configuration (parameter value) or\ncode (equation constant) to prevent this situation",
									 false,
									 "probabilities are invalid for link drawing" );
		return node->first;
	}

	do
		drawPoint = attr->cont->sim->_ran1_( ) * sum;
	while ( drawPoint == sum );						// avoid ran1 == 1

	for ( accProb = 0, cur = cur1 = node->first;	// accumulate probabilities
		  accProb <= drawPoint && cur != NULL; cur = cur->next )
	{												// until reaching the right object
		if ( cur->to->node != NULL )				// node still exists?
			accProb += cur->probTo;
		cur1 = cur;									// save previous object
	}

	return cur1;
}


/*************************************************************
 NETNODE
 Initialize netnode struct (no links).
 *************************************************************/
lsd::netnode::netnode( object *_up, long nodeId, const char *nodeName, double nodeProb )
{
	up = _up;
	id = nodeId;
	time = up->attr->cont->sim->t;					// save creation time
	serial = up->attr->cont->sim->node_serial++;
	prob = nodeProb;

	if ( id < 0 )									// ID assigned?
		id = serial;

	if ( strcmp( nodeName, "" ) && valid_xml_string( nodeName ) )// valid name assigned?
	{
		name = new char[ strlen( nodeName ) + 1 ];
		strcpy( name, nodeName );
	}
	else
		if ( strcmp( nodeName, "" ) )
			up->attr->cont->sim->plog( "\nWarning: network node name '%s' is invalid, ignored.", nodeName );
}


/*************************************************************
 ~NETNODE
 Destroy netnode struct.
 *************************************************************/
lsd::netnode::~netnode( void )
{
	if ( name != NULL )								// name assigned?
		delete name;

	while ( last != NULL )							// remove all links
		delete last;
}


/*************************************************************
 ADD_NODE_NET (*)
 Add netnode data structure to LSD object
 *************************************************************/
lsd::object *lsd::object::add_node_net( long id, const char nodeName[ ],
							  bool silent )
{
	long serialOld = -1;

	if ( node != NULL )
	{
		if ( ! silent )
			attr->cont->sim->plog( "\nWarning: existing network data discarded from object." );

		serialOld = node->serial;					// save serial number
		delete node;
	}

	node = new netnode( this, id, nodeName );

	// prevent replacing the serial number
	if ( serialOld > 0 )
	{
		node->serial = serialOld;
		attr->cont->sim->node_serial--;
	}

	return this;
}


/*************************************************************
 DELETE_NODE_NET (*)
 Remove netnode data structure from LSD object.
 *************************************************************/
void lsd::object::delete_node_net( void )
{
	delete node;
	node = NULL;
}


/*************************************************************
 NAME_NODE_NET (*)
 Set or reset the name of a node.
 *************************************************************/
void lsd::object::name_node_net( const char *nodeName )
{
	if ( node == 0 )								// invalid node?
		return;

	if ( node->name != NULL )						// name already set?
		delete node->name;

	if ( strcmp( nodeName, "" ) )					// name assigned?
	{
		node->name = new char[ strlen( nodeName ) + 1 ];
		strcpy( node->name, nodeName );
	}
	else
		node->name = NULL;
}


/*************************************************************
 SEARCH_NODE_NET (*)
 Search for existing node. Return pointer to the
 object containing it or NULL if node does not exist.
 Slow for large networks, turbosearch is better in
 this case.
 *************************************************************/
lsd::object *lsd::object::search_node_net( const char *lab, long destId )
{
	object *cur;

	for ( cur = search_err( lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, "searching net node" );
		  cur != NULL && cur->node != NULL && cur->node->id != destId;
		  cur = BROTHER( cur ) );
	if ( cur == NULL || cur->node == NULL )			// no network structure?
		return NULL;
	else
		return cur;
}


/*************************************************************
 STATS_NET (*)
 Returns some basic statistics about the directed
 network.
 r[ 0 ]: number of nodes
 r[ 1 ]: number of links (arcs)
 r[ 2 ]: average out-degree
 r[ 3 ]: minimum out-degree
 r[ 4 ]: maximum out-degree
 r[ 5 ]: density (including loops)
 *************************************************************/
double lsd::object::stats_net( const char *lab, double *r )
{
	r[ 0 ] = r[ 1 ] = r[ 2 ] = r[ 3 ] = r[ 4 ] = r[ 5 ] = 0;

	object *cur = search_err( lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, "stating net" );

	if ( cur == NULL || cur->node == NULL )			// invalid network node?
		return NAN;

	for ( ; cur != NULL; cur = BROTHER( cur ) )	// scan all nodes
		if ( cur->node != NULL )					// valid node?
		{
			double nlinks = ( double ) cur->node->nlinks;
			if ( r[ 0 ] == 0. )						// first node?
				r[ 3 ] = nlinks;					// update minimum
			else
				r[ 3 ] = r[ 3 ] < nlinks ? r[ 3 ] : nlinks;

			r[ 0 ]++;
			r[ 1 ] += nlinks;
			r[ 4 ] = r[ 4 ] > nlinks ? r[ 4 ] : nlinks;
		}

	if ( r[ 0 ] > 0. )
	{
		r[ 2 ] = r[ 1 ] / r[ 0 ];
		r[ 5 ] = r[ 1 ] / ( r[ 0 ] * ( r[ 0 ] - 1 ) );
	}

	return r[ 0 ];
}


/*************************************************************
 DRAW_NODE_NET (*)
 Draw a node randomly, with probability equal to prob.
 *************************************************************/
lsd::object *lsd::object::draw_node_net( const char *lab )
{
	double sum, drawPoint, accProb;
	object *cur, *cur1, *cur2;

	// make sure this is being called from the parent (container) object
	cur1 = cur = search_err( lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, "drawing net node" );
	if ( cur == NULL )
		return NULL;

	for ( sum = 0; cur != NULL && cur->node != NULL; cur = cur->next )
													// add-up probabilities
		sum += cur->node->prob;

	if ( ! std::isfinite( sum ) || sum <= 0 )		// check valid probabilities
	{
		attr->cont->sim->error_hard( "invalid network operation",
									 "check your configuration (parameter value) or\ncode (equation constant) to prevent this situation",
									 false,
									 "probabilities are invalid for node drawing" );
		return cur1;
	}

	do
		drawPoint = attr->cont->sim->_ran1_( ) * sum;
	while ( drawPoint == sum );						// avoid ran1 == 1

	for ( accProb = 0, cur = cur2 = cur1;			// accumulate probabilities
		  accProb <= drawPoint && cur != NULL; 		// until reaching the right object
		  accProb += cur->node->prob, cur = cur->next )
		cur2 = cur;									// save previous object

	return cur2;
}


/*************************************************************
 SHUFFLE_NODES_NET (*)
 Shuffle nodes order in the linked list of node objects.
 Use Fischer-Yates shuffling algorithm.
 *************************************************************/
lsd::object *lsd::object::shuffle_nodes_net( const char *lab )
{
	long i, j, iId, jId, numNodes;
	object *cur, *cur1;

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab );
	if ( cur == NULL )
		return NULL;

	for ( numNodes = 0; cur != NULL;
		  numNodes++, cur = BROTHER( cur ) );		// count number of nodes

	initturbo( lab );								// seed the turbosearch linked list

	for ( i = numNodes; i > 1; i-- )				// run the shuffling
	{
		j = ( long ) attr->cont->sim->rnd_int( 1, i );
		cur = turbosearch( lab, ( double ) i );
		cur1 = turbosearch( lab, ( double ) j );

		if ( cur->node == NULL || cur1->node == NULL )
		{
			attr->cont->sim->error_hard( "invalid network object",
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


/*************************************************************
 NODES2CREATE
 Calculate the missing number of object copies.
 Prints a warning if there are more
 existing copies than needed and returns 0.
 *************************************************************/
long lsd::object::nodes2create( const char *lab, long numNodes )
{
	long count;
	object *cur;

	for ( count = 0, cur = search_err( lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, "adding net node" );
		  cur != NULL; count++, cur = BROTHER( cur ) );

	if ( numNodes >= count )
		return numNodes - count;

	attr->cont->sim->plog( "\nWarning: number of existing nodes is more than the required." );

	return 0;
}


/*************************************************************
 INIT_STUB_NET (*)
 Stub function to call the appropriate network
 generator.
 *************************************************************/
double lsd::object::init_stub_net( const char *lab, const char gen[ ], long numNodes,
							  long par1, double par2 )
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
		attr->cont->sim->error_hard( "cannot create network",
									 "check your equation code to prevent this situation",
									 true,
									 "invalid parameter values for a %s network in object '%s'",
									 option, lab );
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
			par2 = ( double ) par1 / numNodes;		// compute parameter
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

	attr->cont->sim->error_hard( "cannot create network",
								 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
								 true,
								 "invalid parameter values for a %s network in object '%s'",
								 option, lab );
	return 0;
}


/*************************************************************
 INIT_DISCON_NET
 Create a disconnected network, just with nodes
 and no links.
 Links can be added node by node by the user.
 *************************************************************/
long lsd::object::init_discon_net( const char *lab, long numNodes )
{
	long idNode;
	object *cur;

	if ( numNodes < 1 || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for disconnected network in object '%s'",
									 lab );
		return -1;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab );
	if ( cur == NULL )
		return -1;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( idNode = 1; cur != NULL; cur = BROTHER( cur ), ++idNode )
		cur->add_node_net( idNode );				// scan all nodes aplying ID numbers

	return 0;
}


/*************************************************************
 INIT_CONNECT_NET
 Create a fully connected undirected network.
 All links/arcs are reciprocal.
 *************************************************************/
long lsd::object::init_connect_net( const char *lab, long numNodes )
{
	long idNode, links = 0;
	object *cur, *cur1, *cur2;

	if ( numNodes < 2 || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for fully connected network in object '%s'",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( idNode = 1; cur != NULL; cur = BROTHER( cur ) )
		cur->add_node_net( idNode++ );				// scan all nodes applying ID numbers

	for ( cur1 = search( lab ), links = 0; cur1 != NULL; cur1 = BROTHER( cur1 ) )
		for ( cur2 = BROTHER( cur1 ); cur2 != NULL; cur2 = BROTHER( cur2 ) )
		{
			cur1->add_link_net( cur2 );				// arc from hub to spoke
			cur2->add_link_net( cur1 );				// arc from spoke to hub

			links += 2;
		}

	return links;
}


/*************************************************************
 INIT_STAR_NET
 Create a star network, first object in the chain
 is the hub.
 All other objects are spokes with bi-directional
 links to hub.
 *************************************************************/
long lsd::object::init_star_net( const char *lab, long numNodes )
{
	long links;
	object *cur1, *cur2;

	// first build a disconnected network
	if ( init_discon_net( lab, numNodes ) != 0 )
		return 0;

	cur1 = search_err( lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, "initing net" );
	if ( cur1 == NULL )
		return 0;

	for ( cur2 = BROTHER( cur1 ), links = 0; cur2 != NULL;
		 cur2 = BROTHER( cur2 ) )					// create the strokes
	{
		cur1->add_link_net( cur2 );					// arc from hub to spoke
		cur2->add_link_net( cur1 );					// arc from spoke to hub

		links += 2;
	}

	return links;
}


/*************************************************************
 INIT_RANDOM_DIR_NET
 Create a completely random network with a fixed
 number of directed links.
 Links/arcs are directed and not reciprocal.
 *************************************************************/
long lsd::object::init_random_dir_net( const char *lab, long numNodes, long numLinks )
{
	long idNode, links = 0;
	object *cur, *cur1;

	if ( numNodes < 2 || numLinks < 0 || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for random directed network in object '%s'",
									 lab );
		return 0;
	}

	if ( numLinks > ( numNodes * ( numNodes - 1 ) ) )// test if net is achievable
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your configuration (parameter value) or\ncode (equation constant) to prevent this situation",
									 false,
									 "object '%s' has numLinks > ( numNodes * ( numNodes - 1 ) )",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( idNode = 1; cur != NULL; cur = BROTHER( cur ) )
		cur->add_node_net( idNode++ );				// scan all nodes applying ID numbers

	while ( links < numLinks )						// create all links
	{
		cur = draw_node_net( lab );					// draw origin node
		cur1 = draw_node_net( lab );				// draw destination node

		if ( cur != cur1 )							// different origin-destination?
			if ( cur->search_link_net( cur1->node->id ) == NULL )// link doesn't exist yet
			{
				cur->add_link_net( cur1 );			// set link to found new link node ID
				links++;
			}
	}

	return links;
}


/*************************************************************
 INIT_RANDOM_UNDIR_NET
 Create a completely random network with a fixed
 number of directed links. Links/arcs are reciprocal
 to form an undirected network.
 *************************************************************/
long lsd::object::init_random_undir_net( const char *lab, long numNodes, long numLinks )
{
	long idNode, links = 0;
	object *cur, *cur1;

	if ( numNodes < 2 || numLinks < 0 || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for random undirected network in object '%s'",
									 lab );
		return 0;
	}

	if ( numLinks > ( numNodes * ( numNodes - 1 ) ) / 2 )// test if net is achievable
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your configuration (parameter value) or\ncode (equation constant) to prevent this situation",
									 false,
									 "object '%s' has numLinks > ( numNodes * ( numNodes - 1 ) ) / 2",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( idNode = 1; cur != NULL; cur = BROTHER( cur ) )
		cur->add_node_net( idNode++ );				// scan all nodes applying ID numbers

	while ( links < numLinks )						// create all links
	{
		cur = draw_node_net( lab );					// draw origin node
		cur1 = draw_node_net( lab );				// draw destination node

		if ( cur != cur1 )							// different origin-destination?
			if ( cur->search_link_net( cur1->node->id ) == NULL )	// link doesn't exist yet
			{
				cur->add_link_net( cur1 );			// set link (origin->destination)
				cur1->add_link_net( cur );			// set link (destination->origin)
				links += 2;
			}
	}

	return links;
}


/*************************************************************
 INIT_UNIFORM_NET
 Create a uniform random network with a fixed number
 of directed links per node. The objects representing
 the nodes must be located inside the current object.
 *************************************************************/
long lsd::object::init_uniform_net( const char *lab, long numNodes, long outDeg )
{
	bool newNode;
	long link, idNode, numLinks, tryNode;
	object *firstNode, *cur, *cur1;

	if ( numNodes < 2 || outDeg < 0 || outDeg >= numNodes || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for uniform random network in object '%s'",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	firstNode = cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( idNode = 1; cur != NULL; cur = BROTHER( cur ) )
		cur->add_node_net( idNode++ );				// scan all nodes aplying ID numbers

	numNodes = idNode - 1;							// effective number of nodes
	initturbo( lab );								// seed the turbosearch linked list

	for ( numLinks = 0, cur = firstNode; cur != NULL; cur = BROTHER( cur ) )
	{
		idNode = cur->node->id;						// current node id
		for ( link = 1; link <= outDeg; link++ )
		{											// run through all node's links
			newNode = false;
			tryNode = idNode;
			while ( ! newNode || tryNode == idNode )// while no new link found
			{
				tryNode = ( long ) attr->cont->sim->rnd_int( 1, numNodes );// draw link (other node ID)
				if ( cur->search_link_net( tryNode ) )// link already exists?
					newNode = false;				// yes
				else
					newNode = true;					// no, flag new link
			}
			cur1 = turbosearch( lab, ( double ) tryNode );// get target node object
			cur->add_link_net( cur1 );				// set link to found new link node ID
			numLinks++;								// one more link
		}
	}
	return numLinks;
}


/*************************************************************
 INIT_RENYI_ERDOS_NET
 Create a undirected network with random links.
 The probability of any two nodes being linked is:
 linkProb. This is the classic Renyi-Erdos network.
 The objects representing the nodes must be
 located inside the current object.
 *************************************************************/
long lsd::object::init_renyi_erdos_net( const char *lab, long numNodes, double linkProb )
{
	long idNode, numLinks, startNode, endNode;
	object *cur, *cur1;

	if ( numNodes < 2 || linkProb < 0 || linkProb > 1 || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for Renyi-Erdos network in object '%s'",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( idNode = 1; cur != NULL; cur = BROTHER( cur ) )
		cur->add_node_net( idNode++ );				// scan all nodes aplying ID numbers

	numNodes = idNode - 1;							// effective number of nodes
	initturbo( lab );								// seed the turbosearch linked list

	for ( numLinks = 0, startNode = 1; startNode < numNodes; startNode++ )
	{												// for all nodes except last
		for ( endNode = startNode + 1; endNode <= numNodes; endNode++ )
		{											// and for all higher numbered nodes
			if ( attr->cont->sim->_ran1_( ) < linkProb )// draws the existence of a link between both
			{
				cur = turbosearch( lab, ( double ) startNode );// searches first node object
				cur1 = turbosearch( lab, ( double ) endNode );// searches second node object
				cur->add_link_net( cur1 );			// create link start->end
				cur1->add_link_net( cur );			// create link end->start

				numLinks += 2;						// two more links in network
			}
		}
	}
	return numLinks;
}


/*************************************************************
 INIT_CIRCLE_NET
 Create a network placing agents on a circle with
 avgOutDeg/2 neighbors on each side (efficient
 algorithm). If avgOutDeg is odd, rounds neighbors
 # down.
 *************************************************************/
long lsd::object::init_circle_net( const char *lab, long numNodes, long outDeg )
{
	long link, idNode, numLinks, lowNeigh;
	object *firstNode, *cur, *cur1;

	if ( numNodes < 2 || outDeg < 0 || outDeg >= numNodes || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for circle network in object '%s'",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	firstNode = cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( idNode = 1; cur != NULL; ++idNode, cur = BROTHER( cur ) )
		cur->add_node_net( idNode );				// scan all nodes aplying ID numbers

	numNodes = idNode - 1;							// effective number of nodes
	initturbo( lab );								// seed the turbosearch linked list

	for ( numLinks = 0, cur = firstNode; cur != NULL; cur = BROTHER( cur ) )
	{
		idNode = cur->node->id;						// gets ID of current node
		lowNeigh = idNode - outDeg / 2;				// calculates lower ID neighbour

		for ( link = 1; link <= outDeg; link++ )	// run through all node's links
		{
			while ( lowNeigh == idNode || lowNeigh < 1 || lowNeigh > numNodes )// fix invalid node IDs
			{
				if ( lowNeigh == idNode )			// same target as original node
					lowNeigh++;     				// go up
				if ( lowNeigh < 1 )					// too low target node ID
					lowNeigh += numNodes;			// big jump up
				if ( lowNeigh > numNodes )			// too high target node ID
					lowNeigh -= numNodes;			// big jump down
			}

			cur1 = turbosearch( lab, ( double ) lowNeigh );// get target node object

			if ( cur->search_link_net( cur1->node->id ) == NULL )// link doesn't exist yet
			{
				cur->add_link_net( cur1 );			// set link to current target node ID
				numLinks++;							// one more link
			}

			lowNeigh++;								// next target ID
		}
	}
	return numLinks;
}


/*************************************************************
 INIT_SMALL_WORLD_NET
 Implement the Small-World rewiring according to
 the Watts&Strogatz Nature '98 paper.
 rho is the rewiring parameter.
 *************************************************************/
long lsd::object::init_small_world_net( const char *lab, long numNodes, long outDeg, double rho )
{
	long link, idNode, numLinks, numNeigh, tryNode, newNode;
	object *cur, *cur1;
	netlink *curl;

	if ( numNodes < 2 || outDeg < 0 || outDeg >= numNodes || rho < 0 || rho > 1 || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for Small-World network in object '%s'",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	numLinks = init_circle_net( lab, numNodes, outDeg );// first generate a circle regular network

	numNeigh = outDeg / 2;							// number of neighbors (each side)

	for ( ; cur != NULL; cur = BROTHER( cur ) )
													// scan all nodes
		for ( link = 1; link <= numNeigh; link++ )	// all possible neighbors' node IDs
			if ( attr->cont->sim->_ran1_( ) < rho )	// draw rewiring probability
			{										// if rewiring
				idNode = cur->node->id;				// get current node ID
				tryNode = idNode + link;			// next node to try

				if ( tryNode > numNodes )			// if above max node ID
					tryNode -= numNodes;			// take one round turn

				curl = cur->search_link_net( tryNode );// get dest. link
				if ( curl == NULL || curl->to == NULL )// invalid pointers?
					continue;
				else
					cur1 = curl->to;				// get dest. object pointer

				cur1->delete_link_net( cur1->search_link_net ( idNode ) );
													// remove link to this object
				cur->delete_link_net( cur->search_link_net ( tryNode ) );
													// and the link from this object
				newNode = idNode;					// look for a new node to create a link
				while ( newNode == idNode )
					newNode = ( long ) attr->cont->sim->rnd_int( 1, numNodes );// draw a random int different from this agent
				cur1 = turbosearch( lab, newNode );	// and get new linking node object

				cur->add_link_net( cur1 );			// create a new link to the new neighbor
				cur1->add_link_net( cur );			// and vice-versa
			}
	return numLinks;
}


/*************************************************************
 INIT_SCALE_FREE_NET
 Create a scale-free network with preferential
 attachment generating a power law distribution
 of number of links. The procedure can be read as
 a generalization of Barabasi procedure with two
 constraints:
 - Fixed number of nodes
 - Arbitrary average number of links.

 The procedure consists in a first round scanning
 all the nodes, and assigning links according to
 the PA procedure. At the end of this first round
 it assigns to each node the probability of being
 chosen. These probabilities are used in subsequent
 rounds in which all nodes choose new links according
 to the probabilities fixed at the first round.
 *************************************************************/
long lsd::object::init_scale_free_net( const char *lab, long numNodes, long outDeg, double expLink )
{
	long idNode, numLinks, nlinks, i;
	double curProb;
	bool node1;
	object *firstNode, *cur, *cur1;
	netlink *cur2;

	if ( numNodes < 2 || outDeg < 0 || outDeg >= numNodes || expLink <= 0 || lab == NULL )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for scale-free network in object '%s'",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	firstNode = cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( idNode = 1; cur != NULL; cur = BROTHER( cur ) )
		cur->add_node_net( idNode++ );				// scan all nodes aplying ID numbers

	numNodes = idNode - 1;							// effective number of nodes
	initturbo( lab );								// seed the turbosearch linked list

	for ( numLinks = 0, node1 = true, cur = firstNode; cur != NULL; cur = BROTHER( cur ) )
	{												// run through all nodes (first scan)
		if ( node1 )								// if first node
		{
			node1 = false;							// no more first node
			cur->node->prob = 1;					// assign maximum probability to it
			cur1 = cur;								// save first object pointer
			cur = cur->next;						// point to next node
		}
		else
		{
			curProb = cur->node->prob;				// save current assigned probability
			cur->node->prob = 0;					// temporarily remove all probability
			cur1 = draw_node_net( lab );			// draw a new destination node
			cur->node->prob = curProb;				// restore probability
		}

		nlinks = cur1->node->nlinks + 1;			// updated link counter of new destination
		cur1->node->prob = pow( nlinks, expLink );	// update link probability (new dest. node)
		cur1->add_link_net( cur );					// and add node ID to link in new node
		nlinks = cur->node->nlinks + 1;				// updated link counter of origin
		cur->node->prob = pow( nlinks, expLink );	// update origin node the same way
		cur->add_link_net( cur1 );					// as the destination node

		numLinks += 2;								// two more links in the network
	}

	for ( cur1 = firstNode; cur1 != NULL; cur1 = BROTHER( cur1 ) )
													// run through all nodes (second scan)
		for ( i = 2; i < outDeg; i += 2 ) 			// for all desired number of links
		{
			for ( cur2 = cur1->node->first; cur2 != NULL; cur2 = cur2->next )
			{										// scan node's links
				cur2->probTo = cur2->to->node->prob;// get link destination node probability,
													// assign it to link
				cur2->to->node->prob = 0;			// and removes destination node from draws
			}

			curProb = cur1->node->prob;				// save current node probability
			cur1->node->prob = 0;					// and take it from draws
			cur = draw_rnd( lab );					// draw another node
			cur1->node->prob = curProb;				// and restore node probability

			for ( cur2 = cur1->node->first; cur2 != NULL; cur2 = cur2->next )
													// scan node's links again
				cur2->to->node->prob = cur2->probTo;// restore node probability

			cur1->add_link_net( cur );				// add link objects, same as before
			cur->add_link_net( cur1 );

			numLinks += 2;							// two more links in the network
		}

	for ( cur = firstNode, cur1 = BROTHER( cur ); cur != NULL;
		  cur = cur1, cur1 != NULL ? cur1 = BROTHER( cur1 ) : cur = cur1 )
													// then safely remove isolated nodes
		if ( cur->node->nlinks == 0 )				// no links?
			cur->delete_obj( );						// remove node

	for ( idNode = 1, cur = firstNode; cur != NULL; idNode++, cur = BROTHER( cur ) )
		cur->node->id = idNode;						// make node ID sequential/continuous

	return numLinks;
}


/*************************************************************
 INIT_LATTICE_NET
 Generates a lattice, a regular square network
 where each cell in row i and column j is connected
 to its 4 or 8 neighbors, depending on an optional
 parameter
 The links are generated clockwise starting from
 "North", that is cell (i-1, j), then either East
 or Northeast, depending on the final option, cells
 (i,j+1) or (i-1,j+1), respectively.
 The lattice is a torus, i.e. cells at the borders
 are connected to the opposite border.
 *************************************************************/
long lsd::object::init_lattice_net( int nRow, int nCol, const char *lab, int eightNeigbr )
{
	long idNode, i, j, h, numNodes = nRow * nCol, numLinks = 0;
	object *cur, *cur1;

	eightNeigbr = ( eightNeigbr == 4 ) ? 0 : ( eightNeigbr == 8 ) ? 1 : eightNeigbr;

	if ( nRow <= 0 || nCol <= 0 || lab == NULL || ( eightNeigbr != 0 && eightNeigbr != 1 ) )
	{
		attr->cont->sim->error_hard( "cannot create network",
									 "check your code (equation constants) or\nconfiguration (parameter values) to prevent this situation",
									 true,
									 "invalid parameter values for lattice network in object '%s'",
									 lab );
		return 0;
	}

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab );
	if ( cur == NULL )
		return 0;

	// creates the missing objects, cloning the first one
	add_n_objects2( lab, nodes2create( lab, numNodes ) );

	for ( i = j = 0, cur = search( lab ); cur != NULL; cur = BROTHER( cur ) )
	{												// scan all nodes aplying ID numbers
		idNode = nCol * i + j + 1;

		cur->add_node_net( idNode );
		if ( ++j >= nCol )
		{
			i++;
			j = 0;
		}
	}

	initturbo( lab );								// seed the turbosearch linked list

	for ( i = j = 0, cur = search( lab ); cur != NULL; cur = BROTHER( cur ) )
	{
		h = nCol * ( i == 0 ? nRow - 1 : i - 1 ) + j + 1;// north
		cur1 = turbosearch( lab, h );
		cur->add_link_net( cur1, 0, 1 );

		if ( eightNeigbr )
		{
			h = nCol * ( i == 0 ? nRow - 1 : i - 1 ) + ( j == nCol - 1 ? 0 : j + 1 ) + 1;
			cur1 = turbosearch( lab, h );			// northeast
			cur->add_link_net( cur1, 0, 1 );
		}

		h = nCol * i + ( j == nCol - 1 ? 0 : j + 1 ) + 1;// east
		cur1 = turbosearch( lab, h );
		cur->add_link_net( cur1, 0, 1 );

		if ( eightNeigbr )
		{
			h = nCol * ( i == nRow - 1 ? 0 : i + 1 ) + ( j == nCol - 1 ? 0 : j + 1 ) + 1;
			cur1 = turbosearch( lab, h );			// southeast
			cur->add_link_net( cur1, 0, 1 );
		}

		h = nCol * ( i == nRow - 1 ? 0 : i + 1 ) + j + 1;// south
		cur1 = turbosearch( lab, h );
		cur->add_link_net( cur1, 0, 1 );

		if ( eightNeigbr )
		{
			h = nCol * ( i == nRow - 1 ? 0 : i + 1 ) + ( j == 0 ? nCol - 1 : j - 1 ) + 1;
			cur1 = turbosearch( lab, h );			// southwest
			cur->add_link_net( cur1, 0, 1 );
		}

		h = nCol * i + ( j == 0 ? nCol - 1 : j - 1 ) + 1;// west
		cur1 = turbosearch( lab, h );
		cur->add_link_net( cur1, 0, 1 );

		if ( eightNeigbr )
		{
			h = nCol * ( i == 0 ? nRow - 1 : i - 1 ) + ( j == 0 ? nCol - 1 : j - 1 )  + 1;
			cur1 = turbosearch( lab, h );			// northwest
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


/*************************************************************
 GET_LINE
 Read line from network text file
 *************************************************************/
void lsd::object::get_line( char *lBuffer, FILE *fPtr )
{
	char firstChar;

	strcpy( lBuffer, "" );

	do
	{
		fgets( lBuffer, MAX_LINE_SIZE, fPtr );		// gets next text line
		firstChar = '\0';
		sscanf( lBuffer, " %c", &firstChar );
	}
	while ( firstChar == '%' );						// skipping comments

	if ( firstChar == '*' )							// check new section start
		strupr( lBuffer );							// to uppercase

	strtrimin( lBuffer, lBuffer, MAX_LINE_SIZE );	// remove extra spaces
}


/*************************************************************
 READ_FILE_NET (*)
 Read directed or undirected network text file in
 Pajek format.
 *************************************************************/
double lsd::object::read_file_net( const char *lab, const char dir[ ], const char base_name[ ], int serial, const char ext[ ] )
{
	int i;
	double weight;
	long idNode, numNodes, countNode, numLinks, startNode, endNode;
	char fileName[ MAX_PATH_LENGTH ], textLine[ MAX_LINE_SIZE ], nameNode[ MAX_LINE_SIZE ];
	bool inSection;
	str_vecT nodes;
	object *cur;
	FILE *pajekFile;

	// make sure this is being called from the parent (container) object
	cur = check_net_struct( lab, serial < 0 );
	if ( cur == NULL )
		return -1;

	if ( serial >= 0 )
		snprintf( fileName, MAX_PATH_LENGTH, "%s%s%s_%i%s%s", dir, foldersep( dir ),
				  base_name, serial, strlen( ext ) == 0 ? "" : ".", ext );	// fully formed file name
	else
		snprintf( fileName, MAX_PATH_LENGTH, "%s%s%s%s%s", dir, foldersep( dir ),
				  base_name,  strlen( ext ) == 0 ? "" : ".", ext );

	if ( ! ( pajekFile = fopen( fileName, "r" ) ) )	// open file for reading
	{
		if ( serial >= 0 )							// interactive mode - handle in interf.cpp
			attr->cont->sim->error_hard( "network file error",
										 "check if the file requested in equation code exists",
										 true,
										 "cannot open network file '%s'",
										 fileName );
		return -2;
	}

	numNodes = 0;									// no node read yet
	while ( !feof( pajekFile ) )					// try to read number of vertices
	{
		get_line( textLine, pajekFile );			// gets next text line
		if ( sscanf( textLine, " *VERTICES %ld ", &numNodes ) == 1 )
													// look for section header
			break;									// and get out when find
	}

	if ( numNodes == 0 )							// no nodes to create (or EOF)
	{
		fclose( pajekFile );
		if ( serial >= 0 )							// interactive mode - handle in interf.cpp
			attr->cont->sim->error_hard( "network file error",
										 "check the requested file content",
										 false,
										 "empty or invalid network file '%s'",
										 fileName );
		return -3;
	}

	for ( countNode = 1, inSection = true; countNode <= numNodes;
		  ++countNode, cur = BROTHER( cur ) )
	{												// creates all nodes
		idNode = -1;								// assume no explicit nodes
		strcpy( nameNode, "" );

		if ( inSection )							// if still in *Vertices
		{
			get_line( textLine, pajekFile );		// gets next text line
			if ( strchr( textLine, '*' ) )			// check new section start
				inSection = false;					// no more *Vertices section
			else									// read attributes
				sscanf( textLine, " %ld \"%[^\"]", &idNode, nameNode );
		}

		if ( cur == NULL )							// node does not exist?
			cur = add_n_objects2( lab, 1 );			// create new node object

		if ( idNode > 0 && idNode != countNode )
		{
			if ( serial >= 0 )
				attr->cont->sim->plog( "\nWarning: network node # %d is invalid, changing to %d", idNode, countNode );
			idNode = countNode;
		}

		if ( idNode < 0 )
			cur->add_node_net( countNode, nameNode, true );	// add (or reset) net data
		else
			cur->add_node_net( idNode, nameNode, true );

		if ( feof( pajekFile ) )					// check file end
			break;
	}

	if ( inSection )								// * was not already read
		get_line( textLine, pajekFile );			// gets next text line

	numLinks = 0;									// prepare to count links

	while ( ! feof( pajekFile ) )					// while file is not over
	{
		inSection = true;							// assume still inside section

		if ( strstr( textLine, "*ARCS" ) )			// check *Arcs section start
			while ( inSection )						// scan *Arcs section
			{
				weight = 0;							// default is no weight
				get_line( textLine, pajekFile );	// gets next text line

				if ( strchr( textLine, '*' ) )		// check new section start
					inSection = false;				// no more in *Arcs section
				else
					if ( sscanf( textLine, " %ld %ld %lf", &startNode, &endNode, &weight ) >= 2 )
					{
						add_link_net( lab, startNode, endNode, weight );
						++numLinks;					// one more link in network
					}
					else
						if ( serial >= 0 && strlen( textLine ) > 0 )
							attr->cont->sim->plog( "\nWarning: invalid arc (%s), ignored", textLine );

				if ( feof( pajekFile ) )			// check file end
					break;
			}
		else
			if ( strstr( textLine, "*EDGESLIST" ) )	// check *EdgesList
				while ( inSection )					// scan *EdgesList section
				{
					get_line( textLine, pajekFile );

					if ( strchr( textLine, '*' ) )
						inSection = false;			// no more *EdgesList section
					else
					{
						nodes = strtostrsplit( textLine, ' ' );
						if ( nodes.size( ) >= 2 )
						{
							startNode = strtol( nodes[ 0 ].c_str( ), NULL, 10, 0 );

							for ( i = 1; i < ( int ) nodes.size( ); ++i )
							{
								endNode = strtol( nodes[ i ].c_str( ), NULL, 10, 0 );
								add_link_net( lab, startNode, endNode, 0, 1, true );
								numLinks += 2;		// two more links in network
							}
						}
						else
							if ( serial >= 0 && strlen( textLine ) > 0 )
								attr->cont->sim->plog( "\nWarning: invalid edge list (%s), ignored", textLine );
					}

					if ( feof( pajekFile ) )
						break;
				}
			else
				if ( strstr( textLine, "*EDGES" ) )	// check *Edges section start
					while ( inSection )				// scan *Edges section
					{
						weight = 0;
						get_line( textLine, pajekFile );

						if ( strchr( textLine, '*' ) )
							inSection = false;		// no more *Edges section
						else
							if ( sscanf( textLine, " %ld %ld %lf", &startNode, &endNode, &weight ) >= 2 )
							{
								add_link_net( lab, startNode, endNode, weight, 1, true );
								numLinks += 2;		// two more links in network
							}
							else
								if ( serial >= 0 && strlen( textLine ) > 0 )
									attr->cont->sim->plog( "\nWarning: invalid edge (%s), ignored", textLine );

						if ( feof( pajekFile ) )
							break;
					}
				else								// no more sections
					get_line( textLine, pajekFile );// gets next text line
	}

	fclose( pajekFile );

	return numLinks;
}


/*************************************************************
 WRITE_FILE_NET (*)
 Write directed network in Pajek text file format.
 *************************************************************/
double lsd::object::write_file_net( const char *lab, const char dir[ ], const char base_name[ ],
							 int serial, bool append )
{
	bool iniSec, noName, noTime, noWeight;
	int tCur = ( attr->cont->sim->t > attr->cont->sim->last_t ) ? attr->cont->sim->last_t : attr->cont->sim->t;// effective current time
	long l, numNodes, numLinks = 0;
	char *c, mode[ 2 ], fileName[ MAX_PATH_LENGTH ], name[ MAX_PATH_LENGTH ];
	object *firstNode, *cur, *cur1;
	netlink *curl;
	FILE *pajekFile;

	// make sure this is being called from the parent (container) object
	firstNode = cur = check_net_struct( lab, serial < 0 );
	if ( cur == NULL )
		return -1;

	if ( serial >= 0 )
		snprintf( fileName, MAX_PATH_LENGTH, "%s%s%s_%i.%s", dir, foldersep( dir ),
				  base_name, serial, append ? "paj" : "net" );// fully formed file name
	else
		snprintf( fileName, MAX_PATH_LENGTH, "%s%s%s.%s", dir, foldersep( dir ),
				  base_name, append ? "paj" : "net" );

	if ( append && tCur > 1 )						// select write mode
		strcpy( mode, "a" );
	else
		strcpy( mode, "w" );

	if ( ! ( pajekFile = fopen( fileName, mode ) ) )// create new file
	{
		if ( serial >= 0 )							// interactive mode - handle in interf.cpp
			attr->cont->sim->error_hard( "network file error",
										 "check disk space and permissions",
										 false,
										 "cannot create network file '%s'",
										 fileName );
		return -2;
	}

	if ( append )
	{
		strcpyn( name, base_name, MAX_PATH_LENGTH );
		while ( ( c = strchr( name, ' ' ) ) != NULL )
			c[ 0 ] = '_';							// replace space by underscore

		fprintf( pajekFile, "\n*Network %s_%d_%d\n\n", base_name, serial, tCur );	// name network
	}
	else
		fprintf( pajekFile, "%% %s objects from LSD '%s' configuration\n\n",
				 lab, strlen( attr->cont->sim->conf_name ) > 0 ? attr->cont->sim->conf_name : NO_CONF_NAME );

	// get network information
	for ( numNodes = l = 0, noName = noTime = noWeight = true, cur1 = NULL,
		  cur = firstNode; cur != NULL; ++l, cur1 = cur, cur = BROTHER( cur ) )
		if ( cur->node != NULL )
		{
			++numNodes;

			if ( cur->node->name != NULL && strlen( cur->node->name ) > 0 )
				noName = false;

			if ( cur->node->time > 0 )
				noTime = false;

			for ( curl = cur->node->first; curl != NULL; curl = curl->next )
				if ( curl->weight != 0 )
				{
					noWeight = false;
					break;
				}
		}

	if ( serial >= 0 && l > numNodes )
		attr->cont->sim->plog( "\nWarning: instances of object '%s' have no data structure,\nthey must be at the end of the chain of siblings", cur1->attr->label );

	if ( serial >= 0 && cur1->hyper_next( ) != NULL )
		attr->cont->sim->plog( "\nWarning: multiple parents of object '%s', considering just first", cur1->attr->label );

	fprintf( pajekFile, "*Vertices %lu\n", numNodes);// start vertices section

	for ( l = 1, cur = firstNode; cur != NULL; cur = BROTHER( cur ) )// scan all nodes
	{
		if ( cur->node == NULL && l <= numNodes )	// non-node at the beginning?
		{
			fclose( pajekFile );
			if ( serial >= 0 )						// interactive mode - handle in interf.cpp
				attr->cont->sim->error_hard( "invalid network object",
											 "check your equation code to add\nthe network structure before using this macro",
											 true,
											 "object '%s' has incorrect network structure, file '%s' not saved",
											 lab, fileName );
			return -3;
		}

		if ( cur->node != NULL )					// valid node?
		{
			if ( ! noName || ! noTime || tCur > 0 )	// adding node lines?
			{
				if ( cur->node->name == NULL )		// no name assigned?
					fprintf( pajekFile, "%ld \"%ld\"", l, cur->node->id );// id as name
				else
					fprintf( pajekFile, "%ld \"%s\"", l, cur->node->name );

				if ( ! noTime || tCur > 0 )			// time information?
					fprintf( pajekFile, " [%d-%d]", cur->node->time, tCur );

				fprintf( pajekFile, "\n" );
			}

			cur->node->serial = l++;				// reset serials
		}
	}

	for ( iniSec = true, cur = firstNode; cur != NULL; cur = BROTHER( cur ) )
		if ( cur->node != NULL )
			for ( curl = cur->node->first; curl != NULL; curl = curl->next )
			{										// scan all links from node
				if ( iniSec )
				{
					fprintf( pajekFile, "\n*Arcs\n" );	// start arcs section
					iniSec = false;
				}

				if ( curl->to->node != NULL )
				{
					fprintf( pajekFile, "%ld %ld", cur->node->serial,
							 curl->to->node->serial );

					if ( ! noWeight )
						fprintf( pajekFile, " %g", curl->weight );

					if ( ! noTime || tCur > 0 )
						fprintf( pajekFile, " [%d-%d]", curl->time, tCur );

					fprintf( pajekFile, "\n" );
					numLinks++;
				}
			}

	fclose( pajekFile );

	return numLinks;
}


/*************************************************************
 DELETE_NET (*)
 Delete a network, removing nodes and links.
 *************************************************************/
void lsd::object::delete_net( const char *lab )
{
	object *cur;

	for ( cur = search_err( lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, "deleting net" );
		  cur != NULL; cur = BROTHER( cur ) )
		cur->delete_node_net( );					// scan all nodes
}


/*************************************************************
 CHECK_NET_STRUCT
 Check the contextual objects structure.
 The calling object has to be a immediate parent
 of the existing object named 'lab'.
 Root cannot be the calling object (not a valid
 network container).
 *************************************************************/
lsd::object *lsd::object::check_net_struct( const char *nodeLab, bool noErr )
{
	object *cur = search( nodeLab, attr->cont->sim->no_search, attr->cont->sim->no_search_up );

	if ( cur == NULL )
	{
		if ( ! noErr )								// interactive mode - handle in interf.cpp
			attr->cont->sim->error_hard( "object not found",
										 "create object in model structure",
										 false,
										 "object '%s' is missing",
										 nodeLab );
		return NULL;
	}

	if ( cur->up == NULL )
	{
		if ( ! noErr )								// interactive mode - handle in interf.cpp
			attr->cont->sim->error_hard( "invalid network data structure",
										 "check your model structure to prevent this situation",
										 false,
										 "cannot create network at the root level" );
		return NULL;
	}

	if ( cur->up->attr != attr )
	{
		if ( ! noErr )								// interactive mode - handle in interf.cpp
			attr->cont->sim->error_hard( "invalid network data structure",
										 "check your model structure to prevent this situation",
										 false,
										 "no descending object '%s' in container object '%s'",
										 nodeLab, attr->label );
		return NULL;
	}

	return cur;
}
