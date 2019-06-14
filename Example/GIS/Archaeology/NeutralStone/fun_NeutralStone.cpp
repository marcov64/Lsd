//#define EIGENLIB			// uncomment to use Eigen linear algebra library
//#define NO_POINTER_INIT	// uncomment to disable pointer checking

#include "fun_head_fast.h"
#include "modules/PajekFromCpp/PajekFromCpp_head.h"
bool USE_PAJEK = false;

// do not add EQUATIONs in this area

MODELBEGIN

// insert your EQUATIONs here, between the MODELBEGIN and MODELEND words

EQUATION("Scheduler")
/* Guarantee the correct order of processing */
V("Init");
CYCLE(cur, "Forager")
{
    V("Action");
}
RESULT(T)


EQUATION("Init")
/* Initialise the model */
double init_ret = 0.0;
{
    //Create the geography - no wrapping
    double xn = V("xn");
    double yn = V("yn");
    double wrap = V("wrap");
    INIT_SPACE_ROOT_WRAP(xn, yn, wrap);

    if ( 0 < V("UsePajek") ) {
        USE_PAJEK = true;
        PAJ_INIT_ANIM("Pajek", "Test", RND_SEED, "Neutral-Model");
    }


    if (1.0 == V("GraphLat") ) {
        INIT_LAT_GIS(0); //Create a visualisation - all black
    }

    MAKE_UNIQUE("Resource");//Provide Resources with unique IDs


    double n_resources = V("n_resources"); //Create the missing resources
    
    if (n_resources < 0) {
    		n_resources = ceil(-n_resources * xn * yn);
				PLOG("\nChanged relative densitiy of resources (%g) to absolute number of %g", -V("n_resources"),n_resources);
    }
    
    ADDNOBJ("Resource", n_resources - 1); //one exists already


    ADD_ALL_TO_SPACE(SEARCH("Resource")); //add them to space at random grid position.

    CYCLE(cur, "Resource") { //Loop over all resources
        if (1.0 == V("GraphLat") ) {
            SET_LAT_PRIORITYS(cur, 0); //lowest positive priority.
            SET_LAT_COLORS(cur, 1000); //white colour
        }

        object* nearestNeighbour = NEAREST_IN_DISTANCES( cur, "Resource", -1 );
        double dist = DISTANCE_BETWEEN( cur, nearestNeighbour );
        WRITES(cur, "DistNearNeigh", dist); //Save the minimum nearest neighbour distance.
    }

    MAKE_UNIQUE("Forager"); //Provide Forager with unique ID. This is necessary for the visualisation with pajek
    double n_Forangers = V("n_Forangers"); //Create the missing foragers
    ADDNOBJ("Forager", n_Forangers - 1); //one exists already

    CYCLE(cur, "Forager") { //In differene to the resources, we do not controll if the foranger just happen to use the same space.
        ADD_TO_SPACE_RND_GRIDS_WHERE(cur, root);
        WRITES(cur, "x_init", POSITION_XS(cur) );
        WRITES(cur, "y_init", POSITION_YS(cur) );

        if (1.0 == V("GraphLat") ) {
            SET_LAT_PRIORITYS(cur, 1); //higher priority.
            SET_LAT_COLORS(cur, 1); //mark position.
        }
    }


    USE_ZERO_INSTANCE //Allow there to be zero instances.
    USE_NAN //Allow not a number numbers.
    PARAMETER //initialise only once
}
RESULT(init_ret)


EQUATION("Action")
/* The Forager moves randomly. The movement consumes one item of the toolkit, if available. If it reaches a resource, it refils the toolkit. */

//First, consume item
cur = RNDDRAW_FAIR("Tool");
if (cur != NULL)
{
    DELETE(cur);
}

//Next, move

if (1.0 == V("GraphLat") )
{
    WRITE_LAT_GIS(2);//mark old position visited
}

//Move and check if the forager dies because he hits the boarder, as in the paper
bool didn_hit_border = MOVE(uniform_int(0, 8));

if ( V("NoBorderCrossing") == 1.0 && false == didn_hit_border )
{
    V("Kill_Self");
    END_EQUATION(0.0); //abort this equation
}

// if (1.0 == V("GraphLat") )
// {
// WRITE_LAT_GIS(1);//mark new position
// }


//Finally, check if you can fill up the toolkit
object* resource = SEARCH_POSITION("Resource");

if (resource != NULL)
{
    //Fill up toolkit
    int n_items = V("n_toolkit") - COUNT("Tool"); //how many items shall be picked up?
    double id = UIDS(resource);
    for (int i = 0; i < n_items; ++i) { //add a new object for each one
        cur = ADDOBJ("Tool");
        ADD_TO_SPACE_SHARES_WHERE(cur, resource); //Add to same position in space - this allows us to analys the distance.
        WRITES(cur, "ID", id); //Information of the kind of the toolkit
    }
    if (VS(resource, "Extracted") == 0) { //add global info of visited resources
        INCRS(root, "UniqueResourcesFound", 1.0);
    }
    INCRS(resource, "Extracted", n_items);
}


//Make sure that all the agent actions are in the correct order.
V("MaxDistance");
V("Statistics");


RESULT(0.0)

EQUATION("MaxDistance")
/* Keep track of the maximum foraging radius. */
double distance = DISTANCE_TO_XY(V("x_init"), V("y_init")); //root is at start
distance = max(distance, CURRENT);
RESULT(distance)

EQUATION("ColourResource")
/*  Colour the resource according to its extraction status
    white: not yet visited
    blue: has been visited
*/
double extracted = V("Extracted");
int colour = 1000; //default -
std::string paj_colour;

if ( 1.0 > extracted )
{
    colour = 1000; //white
    paj_colour = "Black";
}
else
{
    colour = 6; //blue
    paj_colour = "Blue";
    if ( 0 < V("UsePajek") ) {
        double spread_x = max(VS(p->up, "xh") - VS(p->up, "xl"), 1.0 );
        double rel_x = POSITION_X - VS(p->up, "xl") / spread_x ;
        double spread_y = max(VS(p->up, "yh") - VS(p->up, "yl"), 1.0 );
        double rel_y = POSITION_Y - VS(p->up, "yl") / spread_y ;
        PAJ_ADD_V_C(t, UID, "Resource", extracted, rel_x, rel_y, "box", 1.0, 1.0, paj_colour.c_str());
    }
}

if (1.0 == V("GraphLat") )
{
    SET_LAT_COLOR(colour);
}

RESULT(0.0)

EQUATION("DistanceToSource")
/*  Calculate the distance to the source.
    Use fact that the tool is registered at the position of its
    resource */
RESULT(DISTANCE_TO(p->up))

EQUATION("Statistics")
/*  Take the statistics
    at the forager level

	Distance Statistics
    //	Rank-Number Distribution not yet done
    //	Rank-Distance Distribution not yet done

*/

//USE_ZERO_INSTANCE //Allow there to be zero instances.
//USE_NAN

//Update bounds x y for visualisation

if ( 0 < V("UsePajek") )
{
    WRITES(p->up, "xl", min(VS(p->up, "xl"), POSITION_X));
    WRITES(p->up, "xh", max(VS(p->up, "xh"), POSITION_X));
    WRITES(p->up, "yl", min(VS(p->up, "yl"), POSITION_Y));
    WRITES(p->up, "yh", max(VS(p->up, "yh"), POSITION_Y));
    double spread_x = max(VS(p->up, "xh") - VS(p->up, "xl"), 1.0 );
    double rel_x = POSITION_X - VS(p->up, "xl") / spread_x ;
    double spread_y = max(VS(p->up, "yh") - VS(p->up, "yl"), 1.0 );
    double rel_y = POSITION_Y - VS(p->up, "yl") / spread_y ;
    PAJ_ADD_V_C(t, UID, "Forager", COUNT("Tool"), rel_x, rel_y, "diamond", 1.0, 1.0, "Red");
    rel_x = V("x_init") - VS(p->up, "xl") / spread_x ;
    rel_y = V("y_init") - VS(p->up, "yl") / spread_y ;
    PAJ_ADD_V_C(t, UID, "StartPos", 1.0, rel_x, rel_y, "house", 1.0, 1.0, "Red");
    PAJ_ADD_E_C(t, UID, "Forager", UID, "StartPos", 1.0, "Travelled", 1.0, "Red"); //just to allow pajekToSvgAnim to work
}
USE_ZERO_INSTANCE

STAT("DistanceToSource");
WRITE("toolkit_size", v[0]);
WRITE("Avg_Distance", v[1]);
WRITE("SD_Distance", v[2] > 0 ? sqrt(v[2]) : v[2]);
WRITE("Max_Distance", v[3]);
WRITE("Min_Distance", v[4]);

std::map<int, int> diffTypes;
CYCLE(cur, "Tool")
{
    int id = VS(cur, "ID");
    if (diffTypes.find(id) == diffTypes.end() ) {
        diffTypes[id] = 0;
    }
    diffTypes.at(id) = diffTypes.at(id) + 1;
}
double toolkitsize = V("n_toolkit");
WRITE("toolkit_richnes", (double)diffTypes.size());
for (auto const& item : diffTypes)
{

    VS(SEARCH_UID(item.first), "ColourResource"); //Update colouring of resource

    if ( 0 < V("UsePajek") ) {
        double spread_x = max(VS(p->up, "xh") - VS(p->up, "xl"), 1.0 );
        double rel_x = POSITION_X - VS(p->up, "xl") / spread_x ;
        double spread_y = max(VS(p->up, "yh") - VS(p->up, "yl"), 1.0 );
        double rel_y = POSITION_Y - VS(p->up, "yl") / spread_y ;
        PAJ_ADD_E_C(t, item.first, "Resource", UID, "Forager", double(item.second), "Toolkit", 1 + 4.0 * double(item.second) / toolkitsize, "Black"); // /toolkitsize
    }
}

RESULT(0.0)



EQUATION("Kill_Self")
/* FUNCTION: Kill the forager */
// if (1.0 == V("GraphLat") )
// WRITE_LAT_GIS(0); //black

PLOG("\nAnother bites the dust.");
if ( 0 < V("UsePajek") )
{
    double spread_x = max(VS(p->up, "xh") - VS(p->up, "xl"), 1.0 );
    double rel_x = POSITION_X - VS(p->up, "xl") / spread_x ;
    double spread_y = max(VS(p->up, "yh") - VS(p->up, "yl"), 1.0 );
    double rel_y = POSITION_Y - VS(p->up, "yl") / spread_y ;
    PAJ_ADD_V_C(t, UID, "Forager", COUNT("Tool"), rel_x, rel_y, "diamond", 1.0, 1.0, "Black");
}
DELETE(p);

RESULT(T)

EQUATION("End_Sim")
/* Check if the Simulation is at end. */
double endCondResourcesFound = V("EndCondResourcesFound");
bool endUniqueResFound = ( endCondResourcesFound > 0 && endCondResourcesFound <= V("UniqueResourcesFound") ) ? true : false;
if ( COUNT("Forager") == 0 ||  endUniqueResFound )
{
    ABORT;
}
RESULT(T)




MODELEND

// do not add EQUATIONs in this area

void close_sim( void )
{
    if (USE_PAJEK) {
        PAJ_SAVE	// close simulation special commands go here
    }
}
