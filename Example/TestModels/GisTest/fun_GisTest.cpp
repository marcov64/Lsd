//#define EIGENLIB			// uncomment to use Eigen linear algebra library
#include "multidim.hpp" //to allow easy excess of multi dim containers used for debugging only
#include<random> //before fun_head**
#include "fun_head_fast.h"

#include <set>
// see https://github.com/AlbertoMarnetto/multidim for details

// do not add Equations in this area

#define TRACK_SEQUENCE_MAX_T 1


MODELBEGIN

// insert your equations here, between the MODELBEGIN and MODELEND words

EQUATION("Get_ID")
/* Produce a unique ID */
double id = CURRENT + 1;
RESULT(id)


EQUATION("Init_GIS")
if (V("Manual_Seed")>0){
  RND_SETSEED(V("Manual_Seed"));
}
cur = SEARCH("Patch"); //Get first object of "Patch" type. This will be our fixed landscape.

/***** Some operations just to check if they work **/
int xn=5;int yn=5;
INIT_SPACE_GRID("Patch",xn,yn); //Initialise Grid
PLOG("\n")
// Print info on space.

  auto flatView = multidim::makeFlatView(cur->position->map->elements);

  
  //for (int y = 0; y < cur->position->map->yn; y++) {
    //    for (int x = 0; x <cur->position->map->xn; x++) {          PLOG("(%g,%g)",flatView[x+y*xn]->position->x,flatView[x+y*xn]->position->y);
     //      if (x< cur->position->map->xn-1)  PLOG("--");
     //   }        
    //    PLOG("\n");
   // }

  for (int y =cur->position->map->yn; y >0 ; y--) {
        for (int x = cur->position->map->xn; x >0; x--) {          PLOG("(%g,%g)",flatView[y*xn-x]->position->y,flatView[y*xn-x]->position->x);
           if (x> 1)  PLOG("--");
        }        
        PLOG("\n");
    }
   
    int x=2;int y=2;
    
object* patch = SEARCH_POSITION_XYS(SEARCH("Patch"),"Patch",x,y);

PLOG("\n Position and RandPosition Test");
PLOG("\nCurrent Poistion(%g,%g,%g)",POSITION_XS(cur),POSITION_YS(cur),POSITION_ZS(cur));
TELEPORT_XYS(cur,2,2);
PLOG("\nAfter Teleporting to (2,2) (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));

MOVES(cur,"n");
PLOG("\nAfter moving to North dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"e");
PLOG("\n After moving to East dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"s");
PLOG("\n After moving to South dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"w");
PLOG("\n After moving to West dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));

MOVES(cur,"ne");
PLOG("\n After moving to NorthEast dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"se");
PLOG("\n After moving to SouthEast dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"nw");
PLOG("\n After moving to NorthWest dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"sw");
PLOG("\n After moving to SouthWest dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));

int dist=1;//radius less than one is error//
patch = SEARCH_POSITION_XYS(cur,"Patch",1,1);
cur=NEAREST_IN_DISTANCES(patch,"Patch",dist);
 PLOG("\n%s (%g,%g) looks for nearest 'Patch' in distance of %i. Result is %s at (%g,%g)"
 ,patch->label,POSITION_XS(patch),POSITION_YS(patch),dist,cur->label,POSITION_XS(cur),POSITION_YS(cur));

 dist=2;

cur=NEAREST_IN_DISTANCES(patch,"Patch",dist);
 PLOG("\n%s (%g,%g) looks for nearest 'Patch' in distance of %i. Result is %s at (%g,%g)"
 ,patch->label,POSITION_XS(patch),POSITION_YS(patch),dist,cur->label,POSITION_XS(cur),POSITION_YS(cur));

 dist=3;

cur=NEAREST_IN_DISTANCES(patch,"Patch",dist);
 PLOG("\n%s (%g,%g) looks for nearest 'Patch' in distance of %i. Result is %s at (%g,%g)"
 ,patch->label,POSITION_XS(patch),POSITION_YS(patch),dist,cur->label,POSITION_XS(cur),POSITION_YS(cur));
 
 dist=4;

cur=NEAREST_IN_DISTANCES(patch,"Patch",dist);
 PLOG("\n%s (%g,%g) looks for nearest 'Patch' in distance of %i. Result is %s at (%g,%g)"
 ,patch->label,POSITION_XS(patch),POSITION_YS(patch),dist,cur->label,POSITION_XS(cur),POSITION_YS(cur));




PLOG("\n Generating Random Position within grid")
PLOG("\n Rand Post  is (%g,%g)  ",RANDOM_POSITION_XS(cur),RANDOM_POSITION_YS(cur))

PLOG("\n Creating 16 agents")
ADDNOBJ("Agent",V("n_agents")-1);
PLOG("\n Adding them to grid from (0,0) to (3,3) and giving random colors")

 x=0;
 y=0;

CYCLE(cur1,"Agent"){ //link to all remaining items manually. Items linked already to the same space are only moved.
 
  ADD_TO_SPACE_XYS(cur1,cur,x,y);
  WRITES(cur1,"Colour",uniform_int(0,20));  
   WRITES(cur1,"Agent_ID",V("Get_ID"));
  x++;
  if (x == xn-1) {
    x = 0;
    y++;
  }
 } 
 
cur1=SEARCH_POSITION_XYS(cur,"Agent",3,3) ;
PLOG("\nSearched for Agent at position 0,3 and found %s at (%g,%g) with color %g\n",cur1->label,POSITION_XS(cur1),POSITION_YS(cur1),VS(cur1,"Colour") );

cur = SEARCH("Agent"); 
for (int y =cur->position->map->yn; y >0 ; y--) {
        for (int x = cur->position->map->xn; x >0; x--) { PLOG("(%g,%g,%g)",flatView[y*xn-x]->position->y,flatView[y*xn-x]->position->x,VS(cur,"Colour") );
           if (x> 1)  PLOG("--");
        }        
        PLOG("\n");
    }
  double rad=1.5;   
 PLOG("\nLooking for 'Patch' in distance %g to 'Patch' with pos (%g,%g)",rad,POSITION_XS(patch),POSITION_YS(patch));   
 CYCLE_NEIGHBOURS(patch,cur,"Patch",rad){
    PLOG("\n%i %s at (%g,%g) : distance %g position (%g,%g)", ++i,patch->label,
    POSITION_XS(patch),POSITION_YS(patch), DISTANCES(patch,cur),
    POSITION_XS(cur),POSITION_YS(cur)  );
  }
  
  
  


PARAMETER

RESULT(0.0)

EQUATION("Init_GIS_W")
PLOG("\n WRAPING\n")
if (V("Manual_Seed")>0){
  RND_SETSEED(V("Manual_Seed"));
}

cur = SEARCH("Patch"); 
DELETE_SPACE(cur)
int xn=8;int yn=8;
INIT_SPACE_GRID_WRAP("Patch",xn,yn,V("Wrap")); //Initialise Grid

// Print info on space.

  auto flatView = multidim::makeFlatView(cur->position->map->elements);
 
  

  for (int y =cur->position->map->yn; y >0 ; y--) {
        for (int x = cur->position->map->xn; x >0; x--) {          PLOG("(%g,%g)",flatView[y*xn-x]->position->y,flatView[y*xn-x]->position->x);
           if (x> 1)  PLOG("--");
        }        
        PLOG("\n");
    }
   PLOG("\nTesting Wraping Left-Right");
    PLOG("\nOn Boundary");
 object* patch = SEARCH_POSITION_XYS(cur,"Patch",0,1);
 object* target = SEARCH_POSITION_XYS(cur,"Patch",7,1);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
  			PLOG("\nInside Grid");	
    patch = SEARCH_POSITION_XYS(cur,"Patch",7,1);
  target = SEARCH_POSITION_XYS(cur,"Patch",0,1);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
 ////////////////////////////// 
 PLOG("\nTesting Wraping Up-Bottom");
  PLOG("\nOn Boundary");
    patch = SEARCH_POSITION_XYS(cur,"Patch",1,7);
  target = SEARCH_POSITION_XYS(cur,"Patch",1,0);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
  				patch = SEARCH_POSITION_XYS(cur,"Patch",1,0);
  target = SEARCH_POSITION_XYS(cur,"Patch",1,7);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
 	PLOG("\nInside Grid");	
    patch = SEARCH_POSITION_XYS(cur,"Patch",1,6);
  target = SEARCH_POSITION_XYS(cur,"Patch",1,1);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
  				patch = SEARCH_POSITION_XYS(cur,"Patch",1,1);
  target = SEARCH_POSITION_XYS(cur,"Patch",1,6);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
  ////////////////////////////////////////
   PLOG("\nTesting Wraping Diagonal Corners");
    patch = SEARCH_POSITION_XYS(cur,"Patch",0,0);
  target = SEARCH_POSITION_XYS(cur,"Patch",7,7);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
   
    patch = SEARCH_POSITION_XYS(cur,"Patch",0,7);
  target = SEARCH_POSITION_XYS(cur,"Patch",7,0);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
  
   PLOG("\nTesting Wraping Diagonal Corners");
    patch = SEARCH_POSITION_XYS(cur,"Patch",0,0);
  target = SEARCH_POSITION_XYS(cur,"Patch",0,7);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
   
    patch = SEARCH_POSITION_XYS(cur,"Patch",0,0);
  target = SEARCH_POSITION_XYS(cur,"Patch",7,7);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
  				patch = SEARCH_POSITION_XYS(cur,"Patch",7,7);
  target = SEARCH_POSITION_XYS(cur,"Patch",7,0);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
  	patch = SEARCH_POSITION_XYS(cur,"Patch",0,7);
  target = SEARCH_POSITION_XYS(cur,"Patch",7,0);
  PLOG("\n  Distance between two patches at (%g,%g) and at (%g,%g)  respectively is %g "
  				,POSITION_XS(patch),POSITION_YS(patch),POSITION_XS(target),POSITION_YS(target)
  				,DISTANCES(patch,target))
 int dist=2;//radius less than one is error//
patch = SEARCH_POSITION_XYS(cur,"Patch",1,1);
cur=NEAREST_IN_DISTANCES(patch,"Patch",dist);
 PLOG("\n%s (%g,%g) looks for nearest 'Patch' in distance of %i. Result is %s at (%g,%g)"
 ,patch->label,POSITION_XS(patch),POSITION_YS(patch),dist,cur->label,POSITION_XS(cur),POSITION_YS(cur));
cur=NEAREST_IN_DISTANCES(patch,"Patch",dist);
 PLOG("\n%s (%g,%g) looks for nearest 'Patch' in distance of %i. Result is %s at (%g,%g)"
 ,patch->label,POSITION_XS(patch),POSITION_YS(patch),dist,cur->label,POSITION_XS(cur),POSITION_YS(cur));
 ///////////////////////////////////////////////////////////////////////////////////////////////////////
PLOG("\n Position and RandPosition Test in Wraping");
PLOG("\n Left Wraping"); 
PLOG("\nCurrent Poistion(%g,%g,%g)",POSITION_XS(cur),POSITION_YS(cur),POSITION_ZS(cur));
TELEPORT_XYS(cur,1,1);
PLOG("\nAfter Teleporting to (1,1) position is (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"w");
PLOG("\nAfter moving to West dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"w");
PLOG("\n After moving to West dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"w");
PLOG("\n After moving to West dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));

PLOG("\n Right Wraping"); 
PLOG("\nCurrent Poistion(%g,%g,%g)",POSITION_XS(cur),POSITION_YS(cur),POSITION_ZS(cur));
TELEPORT_XYS(cur,7,1);
PLOG("\nAfter Teleporting to (7,1) position is (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"e");
PLOG("\nAfter moving to East dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"e");
PLOG("\n After moving to East dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"e");
PLOG("\n After moving to East dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));

PLOG("\n Top Wraping"); 
PLOG("\nCurrent Poistion(%g,%g,%g)",POSITION_XS(cur),POSITION_YS(cur),POSITION_ZS(cur));
TELEPORT_XYS(cur,5,5);
PLOG("\nAfter Teleporting to (5,5) position is (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"n");
PLOG("\nAfter moving to North dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"n");
PLOG("\n After moving to North dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"n");
PLOG("\n After moving to North dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));

PLOG("\n Bottom Wraping"); 
PLOG("\nCurrent Poistion(%g,%g,%g)",POSITION_XS(cur),POSITION_YS(cur),POSITION_ZS(cur));
TELEPORT_XYS(cur,2,2);
PLOG("\nAfter Teleporting to (2,2) position is (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"s");
PLOG("\nAfter moving to South dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"s");
PLOG("\n After moving to South dir Agent is at (%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
MOVES(cur,"s");
PLOG("\n After moving to South dir Agent is at(%g,%g)",POSITION_XS(cur),POSITION_YS(cur));
////////////////////////////////////////////////////////////////////////////////

PLOG("\nCreating new Agent");
cur1 = ADDOBJ("Agent");
PLOG("\nNew agent is not in space: %s",ANY_GISS(cur1)==true?"false":"true");
PLOG("\nAdding Agent to random position");
ADD_TO_SPACE_RNDS(cur1,cur); //Add agent to the space of cur, random position.
PLOG("\nNew agent now shares the same GIS as the Target: %s",SAME_GISS(cur,cur1)==true?"true":"false");
PLOG("\nIt is positioned at: %g,%g",POSITION_XS(cur1),POSITION_YS(cur1));

PARAMETER

RESULT(0.0)

EQUATION("TEST_LATTICE")
/* Create a lattice and test it. */
 INIT_SPACE_GRID("lat_obj",20,20);
 cur = SEARCH("lat_obj");
 INIT_LAT_GISS(cur);
 char filename[300];
 int step = 0;
 sprintf(filename,"TestLattice_step_%i",step);
 SAVE_LAT_GIS(filename);
 PLOG("\nAdded the lattice. Bottom left corner should be black.");
 CYCLE_GIS_RNDS(cur,cur1,"lat_obj"){
  if (POSITION_XS(cur1) < 5)
    PLOG("\nChanging colour for position (%g,%g) from %g",POSITION_XS(cur1),POSITION_YS(cur1), V_LAT_GIS_XYS(cur,POSITION_XS(cur1),POSITION_YS(cur1)));
  WRITE_LAT_GISS(cur1,max(POSITION_XS(cur1),POSITION_YS(cur1)));
  if (POSITION_XS(cur1) < 5)
    PLOG(" to %g", V_LAT_GISS(cur1) );
  sprintf(filename,"TestLattice_step_%i",++step);
  SAVE_LAT_GIS(filename);
 }


PARAMETER
RESULT(0.0)




MODELEND


// do not add Equations in this area


void close_sim( void )
{
	// close simulation special commands go here
}


