//#define EIGENLIB			// uncomment to use Eigen linear algebra library

#include "fun_head_fast.h"

// do not add Equations in this area


MODELBEGIN

// insert your equations here, between the MODELBEGIN and MODELEND words

/*****************
  Global Equations ( Scheduler , Initialise )
****************/
EQUATION("Scheduler")
/*
The Scheduler controls the updating scheme (that part which is not endogeneously defined 
*/
  V("Initialise");
  if (V("Randomise")==1.0){
  	SORT_RND("Agent");
  }
  double steps = CURRENT;
  char filename[300];
  CYCLE(cur,"Agent"){
    if (VS(cur,"Move") > 0.0) {
      steps++;
      #ifndef NO_WINDOW
      if (COUNTS(p->up,"Model")==1){
        sprintf(filename,"Schelling_%g_t%i_step%g",RND_SEED,t,steps);
        SAVE_LAT(filename);
        SLEEP(10);
      }
    #endif
    }
  }
RESULT( steps )

EQUATION("Initialise")
/*
Initialise the model
*/

  //Initialise the space
  INIT_SPACE_GRID_WRAP("Patch",V("xn"),1,V("wrapping") );

  //Initialise the lattice, but only for first Model and only if just one Model
  #ifndef NO_WINDOW
  if (COUNTS(p->up,"Model")==1){
  	INIT_LAT(1000,1,V("xn") );
  }
  #endif
  //Initialse the agents
  	double nAgents =  floor(V("fracAgents")*V("xn"));
  	ADDNOBJ("Agent", nAgents -1); //add missing number of agents to the SPACE - same place as other agents.
  //Add agents to random position

    object *cAgent = SEARCH("Agent");
    CYCLE_GIS_RNDS(SEARCH("Patch"),cur,"Patch"){
      ADD_TO_SPACE_SHARES(cAgent,cur); //Register cAgent at pos of cur Patch
      //prepare next
      cAgent = go_brother(cAgent);
      if (cAgent == NULL){
        break;
      }
  	}

  	//colour the agents in random order
  	double nAgentsBlue = nAgents * V("fracBlue");
  	i = 0;
  	CYCLE_GIS_RNDS(SEARCH("Agent"),cur,"Agent"){
  		i++; //increase by 1
  		if (i <= nAgentsBlue) {
  			WRITES(cur,"Colour",5); //blue
  		} else {
  			WRITES(cur,"Colour",1); //red
  		}
      #ifndef NO_WINDOW
      if (COUNTS(p->up,"Model")==1){
        WRITE_LAT(POSITION_YS(cur)+1,POSITION_XS(cur)+1,VS(cur,"Colour"));
      }
      #endif
  	}
    #ifndef NO_WINDOW
    if (COUNTS(p->up,"Model")==1){
      char filename[300];
      sprintf(filename,"Schelling_%g_t%i_step0",RND_SEED,t);
      SAVE_LAT(filename);
    }
    #endif
  PARAMETER //Set to parameter, i.e. compute only once
RESULT( 0.0 )

/*****************
  Patch Equations ( isOption )
****************/

EQUATION("isOption")
/* Check if the place would be an option for the calling agent
   check 1: is empty?
   if empty, then check 2 else it is not an option
   check 2: is locFracOther <= tolerance?
   if true, then it is an option else not
   Take into account that self is no more at old position!
*/
  double isOption = 0.0; //default, no option
  if (COUNT_POSITION("Agent")==0.0) {
    double callerColour = VS(c,"Colour");
    double fracOther = 0.0;
    double nNeighbours = 0.0; //A function for neighbourhood statistics would be good
    i = 0;
    CYCLE_NEIGHBOUR(cur,"Agent",VS(p->up,"distance")){
      if (cur == c){
        continue; //skip self
      }
    //     PLOG("\nt=%i, Checking neighbour %i",t,++i);
      	if (VS(cur,"Colour")!=callerColour){
      		fracOther++;
      	}
    	nNeighbours++;
    }

    if (nNeighbours > 0.0) {
      fracOther /= nNeighbours;
    }

    if (fracOther <= VS(p->up,"tolerance")){
      isOption = 1.0; //it is an option.
    }
  }
RESULT(isOption)

/*****************
  Agent Equations ( Move ,Content, locSegregation )
****************/

EQUATION("Move")
/*
The agent decides if it should move.
*/
  double move = 0; //no move
  if (V("Content")==0.0){

    //move
  //   PLOG("\nLooking for a free patch!");

    //Search for the nearest free place in which the agent would be content, if any.
    object* freePatch = NEAREST_IN_DISTANCE_CND("Patch",-1,"isOption","=",1.0);

    if (freePatch != NULL){
      move = 1; 	//move
    	/* For now: Manual approach */
      #ifndef NO_WINDOW
      if (COUNTS(p->up,"Model")==1){
        WRITE_LAT(POSITION_Y+1,POSITION_X+1,1000); //free white
      }
      #endif
      TELEPORT_SHARE(freePatch);
      #ifndef NO_WINDOW
      if (COUNTS(p->up,"Model")==1){
        WRITE_LAT(POSITION_Y+1,POSITION_X+1,V("Colour"));
      }
      #endif
    //   PLOG("\nFound a free patch!");
    } else {
  //     PLOG("\nNo free patch that meets condition.")
    }

  }
RESULT( move )



EQUATION("locFracOther")
/*
Measures the fraction of people in the neighbourhood that differ from ones own color.
*/
  double ownColour = V("Colour");
  double fracOther = 0.0;
  double nNeighbours = 0.0; //A function for neighbourhood statistics would be good
  i = 0;
  CYCLE_NEIGHBOUR(cur,"Agent",VS(p->up,"distance")){
  	if (VS(cur,"Colour")!=ownColour){
  		fracOther++;
  	}
  	nNeighbours++;
  }

  if (nNeighbours > 0.0) {
    fracOther /= nNeighbours;
  }
RESULT( fracOther )

EQUATION("Content")
/*
Function that at the point in time measures if the agent is content with its situation.
*/
  double content = 1.0; //yes
  if (V("locFracOther") > VS(p->up,"tolerance") ){
    content = 0.0; //nay
  }
RESULT ( content )

/*****************
  Monitor Equations ( fracMove , fracContent , dissimilarity)
****************/

EQUATION("fracMove")
/*
Monitor the fraction of agents that decided to move.
*/
  V("Scheduler"); //make sure that scheduler is in charge.
  STAT("Move");
RESULT( v[1] /*Average*/ )

EQUATION("EndOfSim")
/* Stop the simulation if no model changes any more. */
  STAT("fracMove");
  if (v[3] == 0.0 ) {
    PLOG("\nSimulation of %g models at end after %g steps.",v[0],T);
    ABORT; //finish simulation
  }
RESULT( T )

EQUATION("fracContent")
/*
Monitor the fraction of agents that are content.
*/
  STAT("Content");
RESULT( v[1] /* Average */ )

EQUATION("dissimilarity")
/*
Monitor the aggregate level of segregation with the dissimilarity index for the
moore neighbourhood.
D = 1/xn * sum_xn ( |b_i/B_t - r_i/R_t| )

where b_i is the number of blue in the 9-field neighbourhood (moore) and r_i the
number of red respectively. B_t and R_t are the total numbers of Blue and Red.

To define the neighbourhood accordingly in both, 2d and 1d setting and also the
continuous case, we define it as the patch and its neirest 8 neighbours.
*/

  double dissimilarity = 0.0;
  double loc_diss;
  double const neighbDist = 4.0; //sufficiently big geometric distance to have the moore neighbourhood
  double loc_b, loc_r;
  double tot_b = 0.0;
  double tot_r = 0.0;
    //Calculate the total number of red and blue agents.
  CYCLE(cur,"Agent"){
    if ( VS(cur,"Colour") == 5.0 ){
      tot_b++;
    } else {
      tot_r++;
    }
  }

    //calculate the index
  double n_tot = 0.0; //can be different from number of agents.
  CYCLE(cur,"Patch"){

    loc_b = 0.0;
    loc_r = 0.0;
    loc_diss = 0.0;
    //for each patch (i.e. place), calculate the local dissimilarity index
    CYCLE_NEIGHBOURS(cur, cur1, "Agent", neighbDist ) {
      if ( VS(cur1,"Colour") == 5.0 ){
        loc_b++;
      } else {
        loc_r++;
      }
    }
      //empty neighbourhoods are not counted.
    if ( !(loc_b == 0.0 && loc_r == 0.0) ) {
      n_tot++;
      loc_diss = abs( loc_b/tot_b - loc_r/tot_r );
      dissimilarity += loc_diss;
    }

  }
  dissimilarity /= n_tot;

RESULT( dissimilarity )













MODELEND


// do not add Equations in this area


void close_sim( void )
{
	// close simulation special commands go here
}
