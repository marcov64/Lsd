/******************************************************************************

	SUPPORT FUNCTIONS IN C++
	------------------------

 ******************************************************************************/
#include <algorithm>
#include <list>
#include <map>
#include <set>

using namespace std;
typedef unsigned long ulong;

#define _VS(X,Y) X->cal(NULL,(char*)Y,0)			// for simpler handling
#define _VLS(X,Y,Z) X->cal(NULL,(char*)Y,Z)			// for simpler handling


/********* USER NEIGHBORHOOD SEARCH AND PROVIDER RANKING *********/

/*---------------------------------------------------------------------------
	Object class to hold a single provider rank data in a rank.
  ---------------------------------------------------------------------------*/
struct rank_item									// item template of prov_rank
{                                           
	ulong ID,										// ID of the provider
		  freq;										// frequency of the provider
	double sk,										// neighborhood share of provider
		   P,										// price for provider
		   M;										// quality of provider
	object *p;										// pointer to the provider object
	
	rank_item( void );								// default constructor
	rank_item( long, double, double, object * );	// constructor
};

/*
	Initialize the item.
*/
rank_item::rank_item( void )
{
	ID = 0;											// updates object data
	freq = 1;
	sk = P = M = 0.;
	p = NULL;
}

// comparison criteria for price (low first), quality (high first), ratio (low first)
bool compare_P( rank_item first, rank_item second ) { return first.P < second.P; }
bool compare_M( rank_item first, rank_item second ) { return first.M > second.M; }
bool compare_PM( rank_item first, rank_item second ) 
	{ return first.P / first.M < second.P / second.M; }
bool compare_ID( rank_item first, rank_item second ) { return first.ID < second.ID; }
bool duplicate_ID( rank_item first, rank_item second ) { return first.ID == second.ID; }


/*---------------------------------------------------------------------------
	Object class to create and hold the global provider rank data.
  ---------------------------------------------------------------------------*/
class rank_prov_global
{
	list< rank_item > rankP, rankM, rankPM;			// lists to store the sub-ranks

	public:
		list< rank_item > rank;						// list to store the main rank
		
		ulong size( void ) { return ( ulong ) rank.size( ); }
		void update( object *, unsigned int );
		void remove( ulong );
};

/*
	Rebuild the global rank of providers.
	firstProv: first provider object pointer
	rankWindow: sub-rankings window size
*/
void rank_prov_global::update( object *firstProv, unsigned int window )
{
	object *cur;
	rank_item prov;
	list< rank_item >::iterator it;						// iterator to rank

	rank.clear( );										// clear previous items
	
	for ( cur = firstProv; cur != NULL; cur = go_brother( cur ) )
	{													// scan all providers
		prov.p = cur;									// get pointer to provider
		prov.ID = ( ulong ) _VLS( cur, "provID", 1 );	// get provider ID
		prov.P = _VLS( cur, "Pprov", 1 );				// get provider current price
		prov.M = _VLS( cur, "M", 1 );					// and quality
		prov.freq = 0;									// not from the neighborhood
		
		rankP.push_back( prov );						// inserts items in lists
		rankM.push_back( prov );
		rankPM.push_back( prov );
	}
	
	rankP.sort( compare_P );							// ranks the lists
	rankM.sort( compare_M );
	rankPM.sort( compare_PM );

	if ( rankP.size( ) > window )						// check if removal is needed
	{
		advance( it = rankP.begin( ), window );			// remove providers over window
		rankP.erase( it, rankP.end( ) );
		advance( it = rankM.begin( ), window );
		rankP.erase( it, rankM.end( ) );
		advance( it = rankPM.begin( ), window );
		rankP.erase( it, rankPM.end( ) );
	}
	
	rank.splice( rank.end( ), rankP );					// merges all sub-ranks
	rank.splice( rank.end( ), rankM );
	rank.splice( rank.end( ), rankPM );
	
	rank.sort( compare_ID );							// sort by ID
	rank.unique( duplicate_ID );						// remove duplicates
}

/*
	Remove a firm from the ranking.
	If the firm doesn't belong to the ranking, simply ignore.
*/
void rank_prov_global::remove( ulong provID )
{
	list< rank_item >::iterator it;					// iterator to rank
	
	for ( it = rank.begin( ); it != rank.end( ); it++ )
		if ( it->ID == provID )						// scan the list
		{
			rank.erase( it );						// and remove firm if found
			break;
		}
}


/*---------------------------------------------------------------------------
	Object class to create and hold the provider rank data of a user.
  ---------------------------------------------------------------------------*/
class rank_prov_user
{
	list< rank_item > neighList,					// the list of the neighbors providers
					  rank;							// rank of candidate providers for user
	list< rank_item >::iterator itpos;				// iterator to a position in rank
	set< ulong > neighSet;							// set of neighbors' providers
	object *User;									// user node pointer
	
	public:
		ulong neighProv,							// number of neighbors with provider
			  neighTot;								// total number of neighbors
		double sNeighProv;							// share of neighbors with provider
		                                            
		rank_prov_user( object * );					// constructor
		ulong size( void ) { return ( ulong ) rank.size( ); };
		void pos( ulong n ) { advance( itpos = rank.begin( ), n ); };
		void next( void ) { itpos++; };
		ulong ID( void ) { return itpos->ID; };
		double P( void ) { return itpos->P; };
		double M( void ) { return itpos->M; };
		double sk( void ) { return itpos->sk; };
		double find_sk( ulong );
		object *p( void ) { return itpos->p; };
	
	private:
		void upd_freq( ulong );
};

// comparison criteria for neighborhood share (high first)
bool compare_sk( rank_item first, rank_item second ) { return first.sk > second.sk; }

/*
	Build a rank of providers of the neighbors of a user (node), 
	including the census of providers.
*/
rank_prov_user::rank_prov_user( object *User ) : User( User )
{
	netLink *curl;
	object *nProvPtr;
	rank_item prov;
	list< rank_item >::iterator it;					// iterator to rank
	extern rank_prov_global *rp;					// global provider rank
	
	neighProv = neighTot = 0;						// initialize rank object
	
	CYCLE_LINKS( User, curl )
	{												// scan all neighbors
		nProvPtr = (LINKTO( curl ))->hook;			// pointer to neighbor provider
		if ( nProvPtr != NULL )						// if neigh. has a valid provider
		{
			prov.ID = ( ulong ) _VLS( nProvPtr, "provID", 1 );	// get provider ID
			if ( neighSet.find( prov.ID ) == neighSet.end( ) )
			{										// is provider not in the set?
				prov.p = nProvPtr;					// save pointer to provider
				prov.P = _VLS( prov.p, "Pprov", 1 );// get data
				prov.M = _VLS( prov.p, "M", 1 );
				neighSet.insert( prov.ID );			// insert in the provider set
				neighList.push_back( prov );		// insert to end of the prov. list
			}
			else
				upd_freq( prov.ID );				// update the provider frequency
			
			++ neighProv;							// another neighbor w/ provider
		}
		++ neighTot;								// another neighbor
	}
	
	if ( neighProv > 0 )							// at least 1 provider
		for ( it = neighList.begin( ); it != neighList.end( ); it++ ) 
													// for all neighbors' providers
			it->sk = ( double ) it->freq / neighProv;	// compute neighborhood share

	sNeighProv = ( double ) neighProv / neighTot ;	// share of neighbors with provider
	
	rank = neighList;								// build rank, start with neighbors'
	rank.insert( rank.end( ), rp->rank.begin( ), rp->rank.end( ) );
													// candidates then add public ones
	rank.sort( compare_ID );						// sort by ID
	rank.unique( duplicate_ID );					// remove duplicates
}

/*
	Updates the frequency of a provider in a rank.
	If provider doesn't exist yet, do nothing.
*/
void rank_prov_user::upd_freq( ulong provID )
{
	list< rank_item >::iterator it;					// iterator to rank
	
	for ( it = neighList.begin( ); it != neighList.end( ); it++ )
		if ( it->ID == provID )						// scan the list
		{
			it->freq++;								// and update counter if found
			break;
		}
}

/*
	Return the neighborhood share of a given provider.
	If provider doesn't exist, returns 0.
*/
double rank_prov_user::find_sk( ulong provID )
{
	list< rank_item >::iterator it;					// iterator to rank
	
	for ( it = neighList.begin( ); it != neighList.end( ); it++ )
		if ( it->ID == provID )						// scan the list
			return it->sk;							// return share if found
	return 0;
}

/*
	Choose how to initialize the net
*/

// Choose separator
#define foldersep( dir ) ( dir[0] == '\0' ? "" : "/" )

long init_net( object *container, char const *lab, char const *dir, char const *base_name )
{
	int typeNet;
	long numNodes, numLinks, avgOutDeg;
	double linkProb, rho, expLink, v[6];
	char netName[256], netInfo[256];
	
	// reads network parameters
	typeNet = max( 0, min( 6, floor( _VS( container, "typeNet" ) ) ) );
	numNodes = floor( _VS( container, "numNodes" ) );		// number of nodes in network
	avgOutDeg = round( _VS( container, "avgOutDeg" ) );		// average outgoing degree
	linkProb = numNodes > 0 ? avgOutDeg / numNodes : 0;
	rho = _VS( container, "rho" );							// small-world rewiring parameter
	expLink = _VS( container, "expLink" );					// power-law exponential parameter

	switch ( typeNet )										// execute the appropriate functions
	{
		case 0:												// read external network file
			numLinks = container->read_file_net( lab, dir, base_name, ( seed - 2 ) % NUM_NETS + 1, "net" );
			sprintf( netName, "\n Network read from file: %s%s%s_%d.net", dir, 
					 foldersep( dir ), base_name, ( seed - 2 ) % NUM_NETS + 1 ); 
		break;

		case 1:												// Uniform network
			numLinks = INIT_NETS( container, lab, "UNIFORM", numNodes, avgOutDeg, 0 );
			strcpy( netName, "\n Uniform random network created" );
		break;

		case 2:												// Renyi-Erdos random network
			numLinks = INIT_NETS( container, lab, "RENYI-ERDOS", numNodes, 0, linkProb );
			strcpy( netName, "\n Renyi-Erdos random network created" );
		break;

		case 3:												// Circle network
			numLinks = INIT_NETS( container, lab, "CIRCLE", numNodes, avgOutDeg, 0 );
			strcpy( netName, "\n Circle regular network created" );
		break;

		case 4:												// Small-World network
			numLinks = INIT_NETS( container, lab, "SMALL-WORLD", numNodes, avgOutDeg, rho );
			strcpy( netName, "\n Small-world random network created" );
		break;  
		
		case 5:												// Scale-free network
			numLinks = INIT_NETS( container, lab, "SCALE-FREE", numNodes, avgOutDeg, rho );
			strcpy( netName, "\n Scale-free random network created" );
		break;
		
		case 6:												// Fully-random network
			numLinks = INIT_NETS( container, lab, "RANDOM-UNDIR", numNodes, numNodes * avgOutDeg, 0 );
			strcpy( netName, "\n Fully random network created" );
		break;
		
		default:
			strcpy( netName, "\nError: Invalid type of network selected!\n" );
			return 0;
	}
	plog( netName );
	
	STAT_NETS( container, lab );							// get network statistics
	sprintf( netInfo, "\n Num. nodes: %.0f, Num. arcs: %.0f, Avg. out-degree: %.4f", 
			 v[0], v[1], v[2] );
	plog( netInfo );
	sprintf( netInfo, "\n Min. out-degree: %.0f, Max. out-degree: %.0f, Density: %.4f", 
			 v[3], v[4], v[5] );
	plog( netInfo );

	return numLinks;										// return the num. of links
}
