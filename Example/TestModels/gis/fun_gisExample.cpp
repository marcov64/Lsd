//#define EIGENLIB			// uncomment to use Eigen linear algebra library
#include "multidim.hpp" //to allow easy excess of multi dim containers used for debugging only
#include<random> //before fun_head**
#include "fun_head_fast.h"
#include "gis.cpp"

// see https://github.com/AlbertoMarnetto/multidim for details

// do not add Equations in this area


#include<set>

std::mt19937 my_PRNG;
MODELBEGIN

// insert your equations here, between the MODELBEGIN and MODELEND words

EQUATION("Scheduler")
V("Init_GIS");

//We need to check if an agent dies before and after a potential combat.
//Also, the order in which agents act are very important.

std::vector<object*> agent_set;
CYCLE(cur,"Agent"){
  agent_set.push_back(cur); //Add pointer to set
}


if (V("RandomiseOrder")==1){
  std::shuffle(agent_set.begin(),agent_set.end(),my_PRNG); //do not know how to link to LSD prng, a macro/bind would be nice!
}

//CYCLE_SAFE(cur,"Agent"){    //randomisation missing
for (i=0;i<agent_set.size();i++){
  cur=agent_set.at(i);
  if (VS(cur,"Strength") < 1){
    DELETE(cur);
    continue;   //if deleted, we cannot use it.
  }
  VS(cur,"Move");
  if (VS(cur,"Strength") < 1){
    DELETE(cur);
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
my_PRNG.seed(RND_SEED);
cur = SEARCH("Patch"); //Get first object of "Patch" type. This will be our fixed landscape.

INIT_SPACE("Patch",10,20); //Initialise Grid

// Print info on space.
if(false){
  auto flatView = multidim::makeFlatView(cur->position->map->elements);
  for (object* obj : flatView){
//     PLOG("\n1: (%g,%g)",obj->position->x,obj->position->y);
  }
}


DELETE_SPACE(cur)//Delete it
//INIT_SPACE_SINGLE_WRAP(cur, 0,0, 10,20, 16)//Initialise grid but link to single item
INIT_SPACE_SINGLE_WRAP(cur, 0,0, 10,20,0)//Initialise grid but link to single item

int x=0;
int y=0;
CYCLE(cur1,"Patch"){ //link to all remaining items manually. Existing ones are skipped.
  ADD_TO_SPACES(cur1,cur,x,y)
  x++;
  if (x == 10) {
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
INIT_SPACE_WRAP("Patch", V("x_dim"),V("y_dim"),V("Wrap"))//Initialise relative large space
CYCLE(cur1,"Patch"){
  WRITES(cur1,"Owned",1000); //White colour for lattice
}

INIT_LAT( 1000, V("y_dim"), V("x_dim"), 400, 400 );      //Lattice is row-major

//Add other objects
double color = 0;
ADDNOBJ("Agent",19);
CYCLE(cur1,"Agent"){
  ADD_TO_SPACES(cur1,cur,uniform(0,V("x_dim")),uniform(0,V("y_dim")))
  WRITES(cur1,"Colour",color++);
  WRITES(cur1,"Strength",1);
  WRITES(cur1,"NextSpawn",2);
}

//Testing command
cur1 = SEARCH_POSITIONS(cur,"Patch",1.3,2);
if (cur1!=NULL)
  PLOG("\nSearched for patch at position 1.3,2 and found %s with (%g,%g)",cur1->label,POSITION_XS(cur1),POSITION_YS(cur1));

PARAMETER
RESULT(0.0)

EQUATION("Move")
/* Simplistic Toy model: Agents move one field per strength they have.
They colour the patch equal to
their own color. When they encounter another agent with different colour,
they fight. If they have equal strength, a random coin toss decides who wins.
 If they have different strength, the stronger one wins.
 The winner gains 1 more strength, the looser's strength is reduced by an equal
 amount. If strength drops to zero, the agent dies. If strength doubles, the
 agent spawns a second copy of itself.
*/



  //Move randomly to a neaby patch
  for (i=0; i<V("Strength"); i++){
    MOVE(uniform_int(0,8));
    object *winner = p; //assume self wins
    //Check if there are agents to fight with at the patch. Fight first one once.
    CYCLE_NEIGHBOUR(cur,"Agent",1.0){
      if (VS(cur,"Colour")==V("Colour")){
        continue; //skip
      }
      object *sucker = cur;
      if ( V("Strength") <  VS(cur,"Strength") //other stronger
        || ( V("Strength") ==  VS(cur,"Strength") && uniform(0,1) < .5 ) ) //other lucky
      {
        winner = cur;
        sucker = p;
      }
      INCRS(winner,"Strength",1.0);
      if (VS(winner,"Strength")>=VS(winner,"NextSpawn")){
        //Spawn new one!
//         PLOG("\nGrowing in Numbers!");
        object* newOne = ADDOBJ_EXS(p->up,"Agent",winner);
        WRITES(newOne, "Strength", 1.0); //reset
        WRITES(newOne, "NextSpawn", 2.0); //reset
        MULTS(winner,"NextSpawn",2.0);
      }
      INCRS(sucker,"Strength",-1.0);
    }
    if (winner==p){
      WRITE_LAT( int(POSITION_Y+1), int(POSITION_X+1), V("Colour") ); //Offset: Lattice ranges from 1 to xn, 1 to yn AND row-major
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
