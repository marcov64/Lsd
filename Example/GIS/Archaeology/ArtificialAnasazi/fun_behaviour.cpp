/*  ==========================================================================

    fun_behaviour.cpp

    Contains behavioural routines of the agents

    ========================================================================== */


/****************************

          Household

 *        *******************/

EQUATION("lastharvest")
/*  This is the harvest at any point in time.
    NetLogo Code: set lastHarvest [BaseYield] of patch Farm_x Farm_y * (1 + ((random-normal 0 1) * HarvestVariance))
    We also change it to be a variable in LSD. And we do not allow negative harvest.
*/
TRACK_SEQUENCE

double harvest = 0.0;
object* farm = SEARCH("Farm");

//checks
if (farm->up != p)
{
    error_hard("\nERROR!", "check lastharvest", "Farm is not contained in Household");
}
if (!ANY_GISS(farm))
{
    error_hard("\nERROR!", "check lastharvest", "Farm is not in GIS");
}

if (!SAME_GIS(farm))
{
    error_hard("\nERROR!", "check lastharvest", "Farm is not in same GIS as household");
}

object* patch = SEARCH_POSITION_GRIDS(farm, "Land_Patch");

if (!SAME_GISS(farm, patch))
{
    error_hard("\nERROR!", "check lastharvest", "Farm is not in same GIS as the land");
}


harvest = VS(patch, "BaseYield") * (1 + norm(0.0, 1.0) * V("harvestVariance") );

if (harvest < 0 && V("Assumption_negativeHarvest") > -1.0) //
{
    cur = SEARCHS(root, "Settings");
    INCRS(cur, "Assumption_negativeHarvest", 1.0); //save info of assumption fired
    harvest = 0.0;
}

VERBOSE_MODE(false & harvest < 0)
{
    PLOG("\nNegative harvest for person %i", int(V("ID_Person")));
}


//Add current harvest to stock
cur = ADDOBJ("agedCornStock");
WRITES(cur, "cornYear", V("Year"));
WRITES(cur, "cornStock", harvest);

VERBOSE_MODE(false)
{
    PLOG("\n\tAdding corn stock for agent %g with year %g and amount %g", UID, VS(cur, "cornYear"), VS(cur, "cornStock"));
}

RESULT(harvest)

EQUATION("hh_NutritionNeed")
/*  This is the nutrition Needed for a single household, potentially heterogeneous:
    nutritionNeed = Uniform(minNutritionNeed,maxNutritionNeed).
    However, in the Janssen model both are equally fixed to:
    min/maxNutritionNeed = baseNutritionNeed * typicalHouseholdSize := 160 * 5 = 800.
    We will fix it in the simple version here, too.
*/
TRACK_SEQUENCE
double nneed = V("baseNutritionNeed") * V("typicalHouseholdSize");
PARAMETER
RESULT(nneed)

/**************/

EQUATION("hh_NutritionNeedRemaining")
/*  Nutrition need that could not be supplied, if any.
    Calling this variable also updates the stocks.
*/
TRACK_SEQUENCE

bool verbose_here = false;    //checked and ok.

double hh_NutritionNeedRemaining = V("hh_NutritionNeed");
VERBOSE_MODE(verbose_here)
{
    PLOG("\n\nt=%g -- Calling agent %g in year %g to satisfy Nutritionneed: %g", T, UID, VS(root, "Year"), hh_NutritionNeedRemaining);
}

V("lastharvest"); //first, harvest new corn!
double lastGoodYear = VS(root, "Year") - V("yearsOfStock"); //current year is e.g. 800, yOs 2 -> 799 and 798 are still fine.
//The chain of corn is always from old to new. Delete corn too old and use up other corn lifo
CYCLE_SAFE(cur, "agedCornStock")
{
    VERBOSE_MODE(verbose_here) {
        PLOG("\n\tCorn of year %g with amount %g", VS(cur, "cornYear"), VS(cur, "cornStock"));
    }
    if (VS(cur, "cornYear") < lastGoodYear ) {
        VERBOSE_MODE(verbose_here) {
            PLOG(" -- deleting because too old");
        }
        DELETE(cur);
    }
    else {
        double cornStock = VS(cur, "cornStock");
        VERBOSE_MODE(verbose_here) {
            PLOG(" -- eating from it");
        }
        if (cornStock <= hh_NutritionNeedRemaining) {
            VERBOSE_MODE(verbose_here) {
                PLOG(" -- emptying it completely");
            }
            hh_NutritionNeedRemaining -= cornStock;
            if (cur->next != NULL) {      //Check that last element is never deleted.
                DELETE(cur);
            }
            else {
                WRITES(cur, "cornStock", 0.0);
            }
        }
        else {
            cornStock -= hh_NutritionNeedRemaining;
            hh_NutritionNeedRemaining = 0.0;
            WRITES(cur, "cornStock", cornStock);
            VERBOSE_MODE(verbose_here) {
                PLOG(" -- remaining corn: %g. Satisfied.\n", cornStock);
            }
            break; //done - all need satisfied
        }
    }
}

VERBOSE_MODE(verbose_here)
{
    PLOG("\n\tRemaining total need: %g", hh_NutritionNeedRemaining);
}

RESULT(hh_NutritionNeedRemaining)

/**************/

EQUATION("estimate")
/*
    This is the estimated harvest. NetLogo line 447-457

    Simply: Sum up the corn that will still be good next year and assume to have
    the same harvest again.
*/
TRACK_SEQUENCE
const bool verbose_here = false;
//make sure that corn is eaten first
V("hh_NutritionNeedRemaining");
//Sum up the storred corn that is still good next year.
double lastGoodYear_ny = V("Year") - V("yearsOfStock") + 1; //current year is e.g. 800, yOs 2 -> 800 & 799.
double estimate = V("lastharvest");
VERBOSE_MODE(verbose_here)
{
    PLOG("\n\nt=%g -- Agent %g estimates corn next year: Naive forecast %g", T, UID, estimate);
}
CYCLE(cur, "agedCornStock")
{
    if (VS(cur, "cornYear") >= lastGoodYear_ny ) {
        estimate += VS(cur, "cornStock");
        VERBOSE_MODE(verbose_here) {
            PLOG(" + %g for year %g", VS(cur, "cornStock"), VS(cur, "cornYear"));
        }
    }
}
VERBOSE_MODE(verbose_here)
{
    PLOG(" = final estimate is %g", estimate);
}
RESULT(estimate)

/**************/

EQUATION("fission")
/*  Check if a household gets a new child and if yes, create new household
    with farm and initialise all.*/
double fission_ret = 0.0;
{
    if (V("age") >= V("fertilityStartAge") && V("age") <= V("fertilityEndAge")) {
        TRACK_SEQUENCE
        if (uniform(0.0, 1.0) < V("fertility")) {
            V_CHEAT("NewPerson", p); //Note: There is another assumption that must be met in NewPerson.
            fission_ret = 1.0;
        }
    }
}
RESULT(fission_ret)

/**************/

EQUATION("resettle")
/* The resettle decision. Out: 0-no change, 1-resettled, (-1) die (no place) */
TRACK_SEQUENCE
// TRACK_SEQUENCE_ALWAYS

//   bool follow_in_detail = false;//p==SEARCHS(p->up,p->label); //V("Household_ID")==1; //only follow the first household in detail.
const bool verbose_mode = false;
double changed_pos = 0; //0: stay, 1: new good pos, -1: die if Assumption_resettleDie, else take best option (which is insufficient)

//Check if the household needs to resettle

//in the original implementation this is decided by the estimate only.
object* Settings = SEARCHS(root, "Settings");
bool resettle = false;
if (c == root)
{
    resettle = true; //force search
}
else if (V("estimate") < V("hh_NutritionNeed"))    //regular call
{
    resettle = true;
}

// In the original model it is not checked if the current place is still O.K.
// according to some criteria, which we will check here:

//However, the household should also resettle if the settlement is flooded
if (resettle == false )
{
    object* myPatch = SEARCH_POSITION_GRID("Land_Patch");
    if(VS(myPatch, "hydro") > 0) {
        if (VS(Settings, "Assumption_ObserveHydro") < 0) {
            resettle = true;//search new position nearby that is suitable to live in.
        }
        else {
            //increase information of conflict
            INCRS(Settings, "Assumption_ObserveHydro", 1.0);
        }
    }
}

//Also, if the watersource is not available any more, we need to find a new
//one or resettle.
if (V("Waterpoint_by_ID") == 0)
{
    resettle = true; //no WP yet
}
if (resettle == false )
{
    object* myWP = SEARCH_CNDS(root, "Waterpoint_ID", V("Waterpoint_by_ID"));
    if (VS(myWP, "Update_waterSource") == 0) {
        if (VS(Settings, "Assumption_ObserveWater") < 0) {
            object* newWP = NULL;
            RCYCLE_NEIGHBOUR(newWP, "Waterpoint", VS(Settings, "waterSourceDistance")) {
                if (VS(newWP, "Update_waterSource") > 0) {
                    break;
                }
                newWP = NULL; //reset if not valid
            }
            if (newWP == NULL ) {
                resettle = true;
                WRITE("Waterpoint_by_ID", 0.0); //delete info of waterpoint
            }
            else {
                WRITE("Waterpoint_by_ID", VS(newWP, "Waterpoint_ID")); //Change waterpoint
            }
        }
        else {
            INCRS(Settings, "Assumption_ObserveWater", 1.0);
        }
    }
}

////



if (resettle == false)
{
    changed_pos = 0.0; //no change of positon
    END_EQUATION(0.0);
}
else
{
    VERBOSE_MODE(verbose_mode) {
        PLOG("\n\nt=%g -- Person %g looks for a suitable place ", T, UID);
    }

    // if we do resettle, start search.
    VERBOSE_MODE(false && verbose_mode) {
        cur2 = SEARCH_POSITION_GRID("Land_Patch");
        PLOG("\nHousehold looks for a farm.\n\t Old settlement P-ID %g", VS(cur2, "Land_Patch_ID"));
        cur3 = SEARCH_POSITION_GRIDS(SEARCH("Farm"), "Land_Patch");
        PLOG("\n\t, old Farm P-ID %g", VS(cur3, "Land_Patch_ID"));
        PLOG("\n\t, old Waterpoint ID %g", V("Waterpoint_by_ID"));
    }

    //we need to make sure that the household object is passed as caller
    changed_pos =  V_CHEATS(root, "FindFarmAndSettlement", p) ; //-1 no good place, die; 1 success
    if (changed_pos < 0 && c == root && V("Assumption_child") > -1) {
        INCRS(Settings, "Assumption_child", 1.0); //keep track of prohibited fissioning
    }
}

if ( changed_pos > 0)
{
    object* myPatch = SEARCH_POSITION_GRID("Land_Patch");
    object* myWP = SEARCH_CNDS(root, "Waterpoint_ID", V("Waterpoint_by_ID"));
    object* myFarm = SEARCH("Farm");
    VERBOSE_MODE(false && verbose_mode) {
        PLOG("\nHousehold found a new farm.\n\t New settlement P-ID %g", VS(myPatch, "Land_Patch_ID"));
        PLOG("\n\t, new Farm P-ID %g", myWP);
        PLOG("\n\t, new Waterpoint ID %g", V("Waterpoint_by_ID"));
    }
    WRITES(myFarm, "Farm_distWater", DISTANCE_BETWEEN(myFarm, myWP) );
    WRITE("Household_distWater", DISTANCE_TO(myWP) );
    WRITE("Household_distFarm", DISTANCE_TO(myFarm) );


    //in a floodplain
    if (VS(myPatch, "hydro") > 0) {
        if (VS(Settings, "Assumption_SettleFloodplain") > -1) {
            INCRS(Settings, "Assumption_SettleFloodplain", 1.0);
        }
        else {
            changed_pos = -1; //die!
        }
    }
    //settlement is to far from water to life.
    if ( V("Household_distWater") > V("waterSourceDistance") ) {
        if (VS(Settings, "Assumption_waterNotNecessarySettle") > -1) {
            INCRS(Settings, "Assumption_waterNotNecessarySettle", 1.0);
        }
        else {
            changed_pos = -1; //die!
        }
    }
    //farm is to far from water
    if ( VS(myFarm, "Farm_distWater") > V("waterSourceDistance") ) {
        if (VS(Settings, "Assumption_waterNotNecessaryFarm") > -1) {
            INCRS(Settings, "Assumption_waterNotNecessaryFarm", 1.0);
        }
        else {
            changed_pos = -1; //die!
        }
    }



    VERBOSE_MODE(verbose_mode) {
        cur = SEARCH_POSITION_GRIDS(myFarm, "Land_Patch");
        PLOG(" ... and found one with base yield %g and yield %g ", VS(cur, "BaseYield"), VS(cur, "yield") );
    }

}
else if ( changed_pos < 0)
{
    VERBOSE_MODE(verbose_mode) {
        PLOG(" ... but cannot find a suitable place and will die.");
    }
}

RESULT(changed_pos) //end of "resettle"
