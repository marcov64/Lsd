//#define EIGENLIB			// uncomment to use Eigen linear algebra library

#include "fun_head_fast.h"

// do not add Equations in this area

#define latt2d true //to switch between 1d (false) and 2d (true) lat.

#include "PajekFromCpp/PajekFromCpp_head.h"
#define PAJEK

MODELBEGIN

// insert your equations here, between the MODELBEGIN and MODELEND words

/*****************
  Global Equations ( Scheduler , Initialise )
****************/
EQUATION("Scheduler")
/*
The Scheduler controls the updating scheme (that part which is not endogeneously defined 
*/
  PLOG("\nt %i - Start Scheduler: draw val %g",t,RND);
  V("Initialise");
  if (V("Randomise")==1.0){
  	SORT_RND("Agent");
  }
  double steps = CURRENT;
  char filename[300];

    //if parallel updating mode, check contentness in current situation for all agents
  if (V("SequentialUpdating") == 0.0){
    CYCLE(cur,"Agent"){
      VS(cur,"Content");
    }
  }

  #ifdef PAJEK
  CYCLE(cur,"Patch"){
    //(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR)
    PAJ_ADD_V_C(t,UIDS(cur),"Patch",1.0,POSITION_XS(cur),POSITION_YS(cur),"box",1.0,1.0,(COUNT_POSITIONS(cur,"Agent")==0?"White":"Black"));
  }
  #endif

  CYCLE(cur,"Agent"){
    cur1 = SEARCH_POSITIONS(cur,"Patch");
    bool has_moved = ( VS(cur,"Move") > 0.0 );
    #ifdef PAJEK
      PAJ_ADD_V_C(t,UIDS(cur),"Agent",1.0,POSITION_XS(cur),POSITION_YS(cur),(VS(cur,"Content")==1.0?"ellipse":"triangle"),1.0,1.0,(VS(cur,"Colour")==5?"Blue":"Red"));
    #endif
    if (has_moved) {
      steps++;
      #ifndef NO_WINDOW
      if (V("lattice")==2){
        if (COUNTS(p->up,"Model")==1){
          sprintf(filename,"Schelling_%g_t%i_step%g",RND_SEED,t,steps);
          SAVE_LAT(filename);
        }
      }
      #endif
            /* Save as network */
      #ifdef PAJEK
      PAJ_ADD_A_C(t,UIDS(cur1),"Patch",UIDS(cur),"Agent",1.0,"Move",1.0,"Green");
      #endif
    } //move action end
  }

      #ifndef NO_WINDOW
      if (V("lattice")==2){
        if (COUNTS(p->up,"Model")==1){
          sprintf(filename,"Schelling_%g_t%i_step%g",RND_SEED,t,steps);
          SAVE_LAT(filename);
        }
      }
      #endif
  PLOG("\tEnd Scheduler: %g",RND);
RESULT( steps )

EQUATION("Initialise")
/*
Initialise the model
*/
    PLOG("\n\nInitialise Start: %g",RND);
   MAKE_UNIQUE("Agent"); //provide agents with unique ids
   MAKE_UNIQUE("Patch"); //provide patches with unique ids
  #ifdef PAJEK

    PAJ_INIT_ANIM("Pajek","Schelling1d",RND_SEED,"Schelling1d");
  #endif

  //Initialise the space
  #if latt2d
    INIT_SPACE_GRID_WRAP("Patch",V("xn"),V("yn"),V("wrapping") );
  #else
    INIT_SPACE_GRID_WRAP("Patch",V("xn"),1,V("wrapping") );
  #endif

  //Initialise the lattice, but only for first Model and only if just one Model
  #ifndef NO_WINDOW
  if (V("lattice")>0){
    if (COUNTS(p->up,"Model")==1){
      cur = SEARCH("Patch");
      INIT_LAT_GISS(cur);
    }
  }
  #endif
  //Initialse the agents
    #if latt2d
      double nAgents =  floor(V("fracAgents")*V("xn")*V("yn"));
    #else
      double nAgents =  floor(V("fracAgents")*V("xn")*1);
    #endif
  	ADDNOBJ("Agent", nAgents -1); //add missing number of agents to the SPACE - same place as other agents.
  //Add agents to random position


    bool check = false;
    object *cAgent = SEARCH("Agent");
    CYCLE_GIS_RNDS(SEARCH("Patch"),cur,"Patch"){
      ADD_TO_SPACE_SHARES(cAgent,cur); //Register cAgent at pos of cur Patch
      //prepare next
      cAgent = go_brother(cAgent);
      if (cAgent == NULL){
        check=true;
        break;
      }
      if (check)
        PLOG("\n!!!ERROR!!!");
  	}

  	//colour the agents in random order
  	double nAgentsBlue = nAgents * V("fracBlue");
  	i = 0;
    SORT_RND("Agent");
    PLOG("\nt randomised: %g",RND);
  	CYCLE(cur,"Agent"){
  		i++; //increase by 1
  		if (i <= nAgentsBlue) {
  			WRITES(cur,"Colour",5); //blue
  		} else {
  			WRITES(cur,"Colour",1); //red
  		}
      #ifndef NO_WINDOW
      if (V("lattice")>0){
        if (COUNTS(p->up,"Model")==1){
          WRITE_LAT_GISS(cur,VS(cur,"Colour"));
        }
      }
      #endif
  	}
    #ifndef NO_WINDOW
    if (V("lattice")>0){
      if (COUNTS(p->up,"Model")==1){
        char filename[300];
        sprintf(filename,"Schelling_%g_t%i_step0",RND_SEED,t);
        SAVE_LAT(filename);
      }
    }
    #endif

  #ifdef PAJEK
  CYCLE(cur,"Patch"){
    //(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR)
    PAJ_ADD_V_C(0,UIDS(cur),"Patch",1.0,POSITION_XS(cur),POSITION_YS(cur),"box",1.0,1.0,(COUNT_POSITIONS(cur,"Agent")==0?"White":"Black"));
  }
  CYCLE(cur,"Agent"){
    //(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR)
    PAJ_ADD_V_C(0,UIDS(cur),"Agent",1.0,POSITION_XS(cur),POSITION_YS(cur),(VS(cur,"Content")==1.0?"ellipse":"triangle"),1.0,1.0,(VS(cur,"Colour")==5?"Blue":"Red"));
  }
  #endif

  PARAMETER //Set to parameter, i.e. compute only once
  PLOG("\tInitialise End: %g\n\n",RND);
RESULT( 0.0 )

/*****************
  Patch Equations ( isOption )
****************/

EQUATION("isOption")
/* Check if the place would be an option for the calling agent
   check 1: is empty? else it is not an option
   if empty, then, without RationalExpectations take true, else check 2.
   check 2: is locFracOther <= tolerance?
   if true, then it is an option else not
   Take into account that self is no more at old position!
*/
  double isOption = 0.0; //default, no option
  if (COUNT_POSITION("Agent")==0.0) {
    if (V("RationalExpectations") == 0.0){
      isOption = 1.0; //is free and hence an option
    } else {

      //check if it makes the agent better off.

      double callerColour = VS(c,"Colour");
      double fracOther = 0.0;
      double nNeighbours = 0.0; //A function for neighbourhood statistics would be good
      CYCLE_NEIGHBOUR(cur,"Agent",VS(p->up,"distance")){
        if (cur == c){
          continue; //skip self
        }
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
  }
RESULT(isOption)

/*****************
  Agent Equations ( Move ,Content, locSegregation )
****************/

EQUATION("Move")
/*
The agent moves if it is not content with its situation.
*/
  double move = 0; //no move
  double Content = 1.0;

  //In case of sequential updating, we recalculate the contentness.
  if ( V("SequentialUpdating") == 1.0 ) {
    Content = V("Content");
  } else {
    //Otherwise we recall the state updated at the beginning of the current step
    Content = V_CHEAT("Content",NULL);   //V_CHEAT passing NULL: no updating for functions
  }


  if (Content == 0.0){
    //move

      //Depending on RationalExpectations on/off, only free patches that make
      //the agent content are options (on) or all free patches are options (off)
    object* freePatch = NEAREST_IN_DISTANCE_CND("Patch",-1,"isOption","=",1.0);


    if (freePatch != NULL){
      move = 1; 	//move
    	/* For now: Manual approach */
      #ifndef NO_WINDOW
      if (V("lattice")>0){
        if (COUNTS(p->up,"Model")==1){
          WRITE_LAT_GIS(1000); //free white
        }
      }
      #endif
      TELEPORT_SHARE(freePatch);
      #ifndef NO_WINDOW
      if (V("lattice")>0){
        if (COUNTS(p->up,"Model")==1){
          WRITE_LAT_GIS(V("Colour"));
        }
      }
      #endif
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
    //not first step, no movement now, no movement the time before.
  if ( t == max_step || (t>1 && v[3] == 0.0 && CURRENT == 0.0) ) {
    PLOG("\nSimulation of %g models at end after %g steps with dissimilarity %g.",v[0],T,V("dissimilarity"));
    ABORT; //finish simulation
    END_EQUATION( T ) //save current time.
  }
RESULT( v[3] ) //save maximum fraction that moved for automatic stop condition

EQUATION("fracContent")
/*
Monitor the fraction of agents that are content.
*/
  V("Scheduler"); //make sure that scheduler is in charge.
  STAT("Content");
RESULT( v[1] /* Average */ )

EQUATION("dissimilarity")
/*
Monitor the aggregate level of segregation with the dissimilarity index for the
moore neighbourhood.

This is an adjusted concept of the standard index:
D = 1/2 * sum_xn ( |b_i/B_t - r_i/R_t| )
where b_i is the number of blue in the 9-field neighbourhood (moore) and r_i the
number of red respectively. B_t and R_t are the total numbers of Blue and Red.

In our case, we measure overlapping neighbourhoods. To define the neighbourhood
accordingly in both, 2d and 1d setting and also the continuous case, we define
it as the patch and its neirest 8 neighbours.Therefore, we aggegragte
for each agent, the deviation of the local segregation from the global
segregation and normalised in 0..1:

D = (2 * sum_n ( |b_i/n_i - B/n| ) ) / n

Note: The distance for the measurement is different from that of the agents
      perception of "neighbourhood".

*/

  double dissimilarity = 0.0;
  double loc_diss;
  #if latt2d
    double const neighbDist = 1.5; //sufficiently big geometric distance to have the moore (8) neighbourhood
  #else
    double const neighbDist = 4; //sufficiently big geometric distance to have the moore (8) neighbourhood
  #endif
  double loc_b, loc_n;
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
  double tot_n = tot_b + tot_r;


    //calculate the index
  CYCLE(cur,"Agent"){
    loc_n = 1.0; //number of agents in neighbourhood
    loc_b = 0.0;
    if (VS(cur,"Colour") == 5 /*blue*/){
      loc_b++;
    }
    loc_diss = 0.0;
    //for each agent, calculate the local dissimilarity index
    CYCLE_NEIGHBOURS(cur, cur1, "Agent", neighbDist ) {
      loc_n++;
      if ( VS(cur1,"Colour") == 5.0 ){
        loc_b++;
      }
    }

    loc_diss = abs( loc_b/loc_n - tot_b/tot_n );
    dissimilarity += loc_diss;

  }
  dissimilarity /= tot_n/2;

RESULT( dissimilarity )


MODELEND


// do not add Equations in this area


void close_sim( void )
{
#ifdef PAJEK
  plog("saving pajek");
  PAJ_SAVE	// close simulation special commands go here
#endif
}
