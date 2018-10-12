//#define EIGENLIB			// uncomment to use Eigen linear algebra library
#include "multidim.hpp" //to allow easy excess of multi dim containers used for debugging only
#include<random> //before fun_head**
#include "fun_head_fast.h"
//#include "gis.cpp"
#include <set>
// see https://github.com/AlbertoMarnetto/multidim for details

// do not add Equations in this area

#define TRACK_SEQUENCE_MAX_T 1
#include "debug.h"

MODELBEGIN

// insert your equations here, between the MODELBEGIN and MODELEND words

EQUATION("Get_ID")
/* Produce a unique ID */
double id = CURRENT + 1;
RESULT(id)

EQUATION("Scheduler")
V("Init_GIS");

if (t<20){
  object* patch = SEARCH_POSITION_XYS(SEARCH("Patch"),"Patch",2,2);
  i = 0;
PLOG("\nLooking for 'Patch' in distance 1 to 'Patch' with pos %g,%g",POSITION_XS(patch),POSITION_YS(patch));
  CYCLE_NEIGHBOURS(patch,cur,"Patch",1){
    PLOG("\n%i %s at (%g,%g) : distance %g position (%g,%g)", ++i,patch->label,
    POSITION_XS(patch),POSITION_YS(patch), DISTANCES(patch,cur),
    POSITION_XS(cur),POSITION_YS(cur)  );
  }
  
  cur=NEAREST_IN_DISTANCES(patch,"Patch",2);
  PLOG("\n%s %g,%g looks for nearest 'Patch' in distance. Result is %s at %g,%g",patch->label,POSITION_XS(patch),POSITION_YS(patch),cur->label,POSITION_XS(cur),POSITION_YS(cur));


  i = 0;
  PLOG("\n\n----\nLooking for ALL Agents with Colour > 10 within distance of 20 to %s at position (%g,%g)",patch->label,POSITION_XS(patch),POSITION_YS(patch));
  CYCLE_NEIGHBOUR_COND_CHEATLS(patch, cur, "Agent", 20, "Colour", ">", 10.0, 0, patch  )
  {
    PLOG("\n%i %s at (%g,%g) : distance to %s %g position (%g,%g) with colour %g", ++i,patch->label,
    POSITION_XS(patch),POSITION_YS(patch), cur->label, DISTANCES(patch,cur),
    POSITION_XS(cur),POSITION_YS(cur), VS(cur,"Colour")  );
  }

  PLOG("\nLooking for CLOSEST Agent with Colour > 10 within distance of 20 to %s at position (%g,%g)",patch->label,POSITION_XS(patch),POSITION_YS(patch));
  cur = NEAREST_IN_DISTANCE_COND_CHEATLS(patch, "Agent", 20, "Colour", ">", 10.0, 0, patch  );
   if (cur == NULL){
    PLOG("\n\tNothing found");
  } else {
    PLOG("\n\tFound an %s at Position (%g,%g) with Colour %g. Distance is %g.",cur->label,POSITION_XS(cur),POSITION_YS(cur),VS(cur,"Colour"),DISTANCES(patch,cur));
  }
  PLOG("\n----\n")
}
//We need to check if an agent dies before and after a potential combat.
//Also, the order in which agents act are very important.

std::vector<object*> agent_set;
CYCLE(cur,"Agent"){
  agent_set.push_back(cur); //Add pointer to set
}

SORT_RND("Agent");

//CYCLE_SAFE(cur,"Agent"){    //randomisation missing
for (i=0;i<agent_set.size();i++){
  cur=agent_set.at(i);
  if (VS(cur,"Strength") < 1){
    DELETE(cur);
    continue;   //if deleted, we cannot use it.
  }
  VS(cur,"Move");
  if (VS(cur,"Strength") < 1){
    double cur_colour = VS(cur,"Colour");
    DELETE(cur);
    // Check if others of the same colour exist. If not, colour all patches white.
    bool others_of_colour = false;
    CYCLE(cur2,"Agent"){
      if (VS(cur2,"Colour")==cur_colour){
        others_of_colour = true;
        break;
      }
    }
    if (others_of_colour == false){
      CYCLE(cur2,"Patch"){
        if (VS(cur2,"Owned") == cur_colour){
          WRITES(cur2,"Owned",1000);
          if (V("y_dim") * V("x_dim") < 2501) {
            WRITE_LAT( int(POSITION_YS(cur2)+1), int(POSITION_XS(cur2)+1), 1000 ); //Offset: Lattice ranges from 1 to xn, 1 to yn AND row-major
          }
        }
      }
    }
  }
}
cur=SEARCH("Agent");
double count=0;
std::set<double> colors;
double avg_strength=VS(cur,"Strength");
double min_strength=avg_strength;
double max_strength=avg_strength;
CYCLE(cur,"Agent"){
count++;
  colors.insert(VS(cur,"Colour"));
  avg_strength+=VS(cur,"Strength");
  min_strength=min(min_strength,VS(cur,"Strength"));
  max_strength=max(max_strength,VS(cur,"Strength"));
}
avg_strength/=count;
//Delete colours
CYCLE(cur,"Patch"){	
	const bool is_in = std::find(colors.begin(),colors.end(),VS(cur,"Owned")) != colors.end();
	if (!is_in){
		WRITES(cur,"Owned",1000);
		WRITE_LAT( int(POSITION_YS(cur)+1), int(POSITION_XS(cur)+1), 1000 ); //Offset: Lattice ranges from 1 to xn, 1 to yn AND row-major	
	}
}

WRITE("nColours",colors.size());
WRITE("nAgents",count);
WRITE("Avg_Strength",avg_strength);
WRITE("Min_Strength",min_strength);
WRITE("Max_Strength",max_strength);

if (colors.size() ==1){
  WRITE("Time_Sim_Ends",T);
  quit=1; //end the run. The winner is set.
}

RESULT(0.0)

EQUATION("Init_GIS")
if (V("Manual_Seed")>0){
  RND_SETSEED(V("Manual_Seed"));
}
cur = SEARCH("Patch"); //Get first object of "Patch" type. This will be our fixed landscape.

/***** Some operations just to check if they work **/
INIT_SPACE_GRID("Patch",5,5); //Initialise Grid

// Print info on space.
if(false){
  auto flatView = multidim::makeFlatView(cur->position->map->elements);
  for (object* obj : flatView){
//     PLOG("\n1: (%g,%g)",obj->position->x,obj->position->y);
  }
}


DELETE_SPACE(cur)//Delete it
//INIT_SPACE_SINGLE_WRAP(cur, 0,0, 10,20, 16)//Initialise grid but link to single item
INIT_SPACE_SINGLE_WRAPS(cur, 0,0, 5,5,0)//Initialise grid but link to single item

int x=0;
int y=0;
CYCLE(cur1,"Patch"){ //link to all remaining items manually. Items linked already to the same space are only moved.
  ADD_TO_SPACE_XYS(cur1,cur,x,y)
  x++;
  if (x == 5) {
    x = 0;
    y++;
  }

  //No one owns it
  WRITES(cur1,"Owned",1000); //White colour for lattice
}
if(false){
  // Print info on space.
  auto flatView = multidim::makeFlatView(cur->position->map->elements);
  for (object* obj : flatView){
//     PLOG("\n2: (%g,%g)",obj->position->x,obj->position->y);
  }
}
DELETE_SPACE(cur)//Delete it, again

/*********** END OF TEST **********/



INIT_SPACE_GRID_WRAP("Patch", V("x_dim"),V("y_dim"),V("Wrap"))//Initialise relative large space
CYCLE(cur1,"Patch"){
  WRITES(cur1,"Owned",1000); //White colour for lattice
}

if (V("y_dim") * V("x_dim") < 2501){
  INIT_LAT( 1000, V("y_dim"), V("x_dim"), 400, 400 );      //Lattice is row-major
} else {
  PLOG("\nThe number of cells is to high for a graphical lattice.");
}

//Add other objects
double color = 0;
ADDNOBJ("Agent",V("n_agents")-1);
CYCLE(cur1,"Agent"){
  ADD_TO_SPACE_XYS(cur1,cur,RANDOM_POSITION_XS(cur),RANDOM_POSITION_YS(cur));
  WRITES(cur1,"Colour",uniform_int(0,20));
  WRITES(cur1,"Strength",1);
  WRITES(cur1,"NextSpawn",2);
  WRITES(cur1,"Agent_ID",V("Get_ID"));
}

//Testing command
cur1 = SEARCH_POSITION_XYS(cur,"Patch",1.3,2);
if (cur1!=NULL)
  PLOG("\nSearched for patch at position 1.3,2 and found %s with (%g,%g)",cur1->label,POSITION_XS(cur1),POSITION_YS(cur1));

PARAMETER
RESULT(0.0)

EQUATION("Am_I_Target")
/* This function is called via a fake-caller. It determines if THIS object is
  considered a valid target from the fake_caller.
*/
TRACK_SEQUENCE
double iAmTarget = 0.0; //nope
double myStrength = V("Strength");
if (myStrength > 0 && VS(c,"Strength")>=myStrength && VS(c,"Colour") != V("Colour") ){
  iAmTarget = 1.0;
}

RESULT(iAmTarget)

EQUATION("Move")
/* Simplistic Toy model: Agents move up to one field per strength they have.
They colour the patch equal to
their own color. When they encounter another agent with different colour,
they fight. If they have equal strength, a random coin toss decides who wins.
 If they have different strength, the stronger one wins.
 The winner gains 1 more strength, the looser's strength is reduced by an equal
 amount. If strength drops to zero, the agent dies. If strength doubles, the
 agent spawns a second copy of itself.

 Agents search in a radius equal to their strength for other agents. If another
 agent with different colour, at most equal strength and "living" exists, they fight.

 If no such agent exists, the move randomly.

*/
  TRACK_SEQUENCE
  for (i=V("Strength"); i>0; ){
    SET_LOCAL_CLOCK_RF;
    double radius = V("Strength")-i;
    if (radius < 0){
      radius = 0;
      PLOG("\nIt happened");          // NEAREST_IN_DISTANCE_COND_CHEAT(LAB, RAD, VAR, COND, CONDVAL, CHEAT_C  )
    }
    object* target = NEAREST_IN_DISTANCE_COND_CHEAT("Agent", radius, "Am_I_Target", "=", 1.0,  p );
    REPORT_LOCAL_CLOCK_CND(1);
    object *winner = p; //assume self wins
    if (target == NULL){
      MOVE(uniform_int(0,8));//Move randomly to a neaby patch at max distance
      i--; //costs one strength.
    } else {
      i-= max(1.0, ceil(DISTANCE(target) ) ); //here they may move shortest path //max because of feeding on succers
      TELEPORT_SHARE(target);
      object *sucker = target;
      if ( V("Strength") <  VS(target,"Strength") //other stronger
        || ( V("Strength") ==  VS(target,"Strength") && uniform(0,1) < .5 ) ) //other lucky
      {
        winner = target;
        sucker = p;
      }
        INCRS(winner,"Strength",1.0);
      if (VS(winner,"Strength")>=VS(winner,"NextSpawn")){
        //Spawn new one!
//         PLOG("\nGrowing in Numbers!");
        object* newOne = ADDOBJ_EXS(p->up,"Agent",winner);
        WRITES(newOne, "Strength", 1.0); //reset
        WRITES(newOne, "NextSpawn", 2.0); //reset
        WRITES(newOne,"Agent_ID",V("Get_ID"));
        MULTS(winner,"NextSpawn",2.0);
      }
      INCRS(sucker,"Strength",-1.0);
      //eventually delete sucker. Important, else infinit loop with fight against sucker at no cost.
      if (sucker != p && VS(sucker,"Strength")<1){
        //DELETE(sucker); Not possible. So currently one can feed on the canvas of suckers.
      }
    }
    if (winner==p){
      if (V("y_dim") * V("x_dim") < 2501) {
        WRITE_LAT( int(POSITION_Y+1), int(POSITION_X+1), V("Colour") ); //Offset: Lattice ranges from 1 to xn, 1 to yn AND row-major
      }
      object* patch = SEARCH_POSITION_GRID("Patch");
      WRITES(patch,"Owned",V("Colour"));
    } else {
      break; //if succer, no more move.
    }

  }


RESULT(0.0)









MODELEND


// do not add Equations in this area


void close_sim( void )
{
	// close simulation special commands go here
}
