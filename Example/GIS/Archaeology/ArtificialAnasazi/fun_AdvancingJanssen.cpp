#include "fun_head_fast.h"

/* Switches for debugging/visually analysing the specific model */
//#define VALIDATION //checked - everything is as it should!
//Switches for validation module.
#ifndef VALIDATION
#define SWITCH_TEST_OFF     //(un)comment to switch on(off)
#define SWITCH_VERBOSE_OFF  //(un)comment to switch on(off)
#endif
#define TRACK_SEQUENCE_MAX_T 0 //number of steps for tracking
#define DISABLE_LOCAL_CLOCKS

#include "external/lsd_debug-tools/validate.h" //some tools for validation

//#define MODULE_PAJEK //uncomment to create Pajek Network Files
#ifdef MODULE_PAJEK
#include "ext_pajek.cpp" //pajek network visualisation, also loads external/Pajek
#endif

#define NOLATTICE   //uncomment to allow lattice - not fully working atm
#ifndef NOLATTICE
#include "ext_lattice.cpp" //lattice, also loads external/ColourGradient
#endif




/* Some hard-coded parameters */

int const YEAR_BEGIN = 800;
int const YEAR_END = 1350;
int const TOTALSTEPS = YEAR_END - YEAR_BEGIN + 1;
int const x_cols = 80;
int const y_rows = 120;


/* Include some model specific c++ backend */
#include "ext_data.cpp" //utilities to load and check the data, etc.

/* Data containers */
std::vector < double > apdsi;
std::vector < std:: vector <double > > hydro; //environment-data in Janssen 2009
std::vector < object*>  potential_farms_allTime; //List of pointers to sustainable farm-land ever.
std::vector < std::pair<double, object*> > sustainable_farms; //List of pointers to sustainable farm-land.
std::vector < std::pair<bool, object*> > potential_farms; //List of pointers to sustainable farm-land that are currently unsettled and unfarmed.

/*------------------*/



/************************************************

            LSD MACRO EQUATIONS
 *          *************************************************/

MODELBEGIN

/****
     Externals to be included after MODELBEGIN
 *   ****/

#include "fun_behaviour.cpp" //behaviour files
#include "fun_lattice.cpp" //lattice stuff
#include "fun_pajek.cpp" //pajek stuff

/*------------------*/

EQUATION("Farm_distWater")
/*Distance to next watersource*/
cur = NEAREST_IN_DISTANCE_CND("Waterpoint", -1, "Update_waterSource", ">", 0);
double distance = -1;
if (cur != NULL)
{
    distance = DISTANCE(cur);
}

RESULT( distance )

EQUATION("Scheduler")
TRACK_SEQUENCE
// TRACK_SEQUENCE_ALWAYS

//Initialise the model
object* Settings = SEARCH("Settings");
if(t == 1)
{

    mt32.discard(800000); //discard first 800k to get rid of warm-up.
    WRITE("Year", YEAR_BEGIN);
    //Initialise land and also the select_patch map.

    //Add the external object for the allocation mechanism

    //First, data
    MAKE_UNIQUE("Household");
    MAKE_UNIQUE("Farm");
    V("Init_geography");  //in here init land allocator
    V("Init_apdsi_hydro");
    V("Init_Waterpoints");


    //next agents
    V("Init_population"); //Initialise population


    if (max_step > TOTALSTEPS) {
        max_step = TOTALSTEPS;
    }
    else if (max_step < TOTALSTEPS) {
        PLOG("\nYou chose insufficient steps. Make it at least %i", TOTALSTEPS);
        quit = 1;
    }
}
else
{
    INCR("Year", 1.0); //increase the year by one

    //If you like to see individual firing rates and not cumulatives, uncomment:
    /*
        //Reset assumption trackers
        if (VS(Settings, "Assumption_resettleDie") > -1) {
        WRITES(Settings, "Assumption_resettleDie", 0);
        }
        if (VS(Settings, "Assumption_SettleFloodplain") > -1) {
        WRITES(Settings, "Assumption_SettleFloodplain", 0);
        }
        if (VS(Settings, "Assumption_ObserveHydro") > -1) {
        WRITES(Settings, "Assumption_ObserveHydro", 0);
        }
        if (VS(Settings, "Assumption_ObserveWater") > -1) {
        WRITES(Settings, "Assumption_ObserveWater", 0);
        }
        if (VS(Settings, "Assumption_waterNotNecessarySettle") > -1) {
        WRITES(Settings, "Assumption_waterNotNecessarySettle", 0);
        }
        if (VS(Settings, "Assumption_waterNotNecessaryFarm") > -1) {
        WRITES(Settings, "Assumption_waterNotNecessaryFarm", 0);
        }
        if (VS(Settings, "Assumption_negativeHarvest") > -1) {
        WRITES(Settings, "Assumption_negativeHarvest", 0);
        }
        if (VS(Settings, "Assumption_bugUplands") > -1) {
        WRITES(Settings, "Assumption_bugUplands", 0);
        }
        if (VS(Settings, "Assumption_child") > -1) {
        WRITES(Settings, "Assumption_child", 0);
        }
    */

}

//the "go" cycle in lhv.netlogo

/* Update Environment part 1 */

//Update apdsi info
V("Update_apdsi_hydro");

//Update Watersource info
if (V("Assumption_waterLast") != 1)
{
    double n_watersources = 0.0;
    CYCLE(cur, "Waterpoint") {
        n_watersources += VS(cur, "Update_waterSource");
    }
    VERBOSE_MODE(false) {
        PLOG("\nt is %i: There are %g watersources this time.", t, n_watersources);
    }
    if(n_watersources == 0.0) {
        TEST_MODE(true) {
            PLOG("\nt is %i: There is no watersource this time.", t);
        }
    }
}

//Update Yield & Information
V("Capacity"); //update yield for each patch

/* Update household consumption */

//Update HH decisions wrt harvesting - parallel
// PLOG("\n +++++ Eating ++++ ");
RCYCLE_GISS(root, cur, "Household") //Note: A non-random cycle breaks everything!
{
    VS(cur, "hh_NutritionNeedRemaining");
}

// PLOG("\n +++++ Demography ++++ ");
V("Update_Population"); //Old agents die, as do those without enough food.


// PLOG("\n +++++ Resettle ++++ ");
RCYCLE_GISS(root, cur, "Household")
{
    bool die = -1.0 == VS(cur, "resettle"); //shall the hh resettle? 1: change pos, 0: keep pos, -1: die if assumption..
    if (V("Assumption_resettleDie") > -1 && die ) {
        VERBOSE_MODE(false) {
            PLOG("\nHousehold with ID %g died with age %g because it found no new farm.", UIDS(cur), VS(cur, "age"));
        }
        INCRS(Settings, "Assumption_resettleDie", 1.0); //track
        INCRS(SEARCH("Population"), "nHouseholds", -1);
        DELETE(cur); //Delete
    }
}

// PLOG("\n +++++ Birth ++++ ");
RCYCLE_GISS(root, cur, "Household")
{
    VS(cur, "fission"); //get children
}

//Update Watersource info
if (V("Assumption_waterLast") == 1)
{
    double n_watersources = 0;
    CYCLE(cur, "Waterpoint") {
        n_watersources += VS(cur, "Update_waterSource");
    }
    VERBOSE_MODE(false) {
        PLOG("\nt is %i: There are %g watersources this time.", t, n_watersources);
    }
    if(n_watersources == 0.0) {
        TEST_MODE(true) {
            PLOG("\nt is %i: There is no watersource this time.", t);
        }
    }
}

#ifdef VALIDATION
int t_count = 0;
int val_errs = 0;
auto env = P_EXTS(SEARCHS(p->up, "Environment"), ext_environment);
CYCLE( cur, "Land_Patch" )
{
    t_count++;
    double hydro = VS(cur, "hydro");
    double water = VS(cur, "watersource");
    double apdsi = VS(cur, "apdsi");
    double yield = VS(cur, "yield");
    int err_val = env->validate(t - 1, POSITION_XS(cur), POSITION_YS(cur), hydro, water, apdsi, yield);
    if (err_val < 0 ) {
        ABORT;
    }
    else {
        val_errs += err_val;
    }
}
if (val_errs > 0)
{
    PLOG("\ny=%i,t=%i  - Error: Validation not O.K. in %i of %i cases (%i yield (sum diffs: %g), %i hydro (sum diffs: %g), %i apdsi (sum diffs: %g), %i water (sum diffs: %g)).",
         int(V("Year")), t, val_errs, t_count * 4,
         env->errs_yield, env->sum_diffs_yield,
         env->errs_hydro, env->sum_diffs_hydro,
         env->errs_apdsi, env->sum_diffs_apdsi,
         env->errs_water, env->sum_diffs_water );
}
else
{
    VERBOSE_MODE(true) {
        PLOG("\nt=%i  - Validation fine, no errors in %i cases", t, t_count * 4);
    }
}
if (t_count < x_cols* y_rows)
{
    PLOG("\nError! There should be %i patches but there are only %i.", x_cols * y_rows, t_count);
    ABORT;
}
#endif

//together with the TRACK_SEQUENCE macro, this allows to see exactly what is
//happening that is not yet controlled by the scheduler.
VERBOSE_MODE(t <= TRACK_SEQUENCE_MAX_T)
{
    PLOG("\n -- -- -- S C H E D U L E R   is due. -- -- --\n");
}


if (t == TOTALSTEPS)
{
    ABORT
}

V("Update_Lattice");
V("Pajek");

RESULT(double (t))


EQUATION("Init_apdsi_hydro")
TRACK_SEQUENCE
/*  load APDSI data to c++ vectors. The data is later used to update
    the information.
*/
/************/

apdsi.clear();
std::ifstream  apdsi_data("data/adjustedPDSI.txt");
if (!apdsi_data)
{
    PLOG("\nError: No map data at path: 'data/adjustedPDSI.txt'");
    ABORT;
}
string s;
int count = 0;
while (apdsi_data >> s)  //read entry by entry
{
    apdsi.push_back(std::stod(s)); //save it to the vector.
    count++;
}
TEST_MODE(true)
{
    PLOG("\nElements in apdsi are: %i", count)
}
apdsi_data.close();

/* Finally, load the environment txt "hydro" data. What ever that is... */
std::ifstream  env_data("data/environment.txt");
if (!env_data)
{
    PLOG("\nError: No map data at path: 'data/environment.txt'");
    ABORT;
}

hydro.clear();
count = 0;
while (env_data >> s)  //read entry by entry
{
    count++;
    std::vector <double> entry;
    entry.push_back(std::stod(s));
    int i = 1;
    while (env_data >> s) {
        i++;
        count++;
        entry.push_back(std::stod(s));
        if (i == 15) {
            break;
        }
    }
    hydro.push_back(entry);
}
TEST_MODE(true)  //checked
{
    PLOG("\nElements in hydro are: %i  and container size is: %i x %i", count, hydro.size(), hydro.at(0).size());
}
env_data.close();

//Force update apdsi info
V("Update_apdsi_hydro");
PARAMETER
RESULT(0.0)

EQUATION("age")
TRACK_SEQUENCE
RESULT(CURRENT + 1)


EQUATION("Update_Population")
TRACK_SEQUENCE

double living_ps = 0;
double died_ps = 0;
double died_ps_old = 0;
WRITE("newHouseholds", 0); //reset
double current_hh = V("nHouseholds");

CYCLE_SAFES(p->up, cur, "Household")
{
    if (VS(cur, "age") >= V("deathAge") || VS(cur, "hh_NutritionNeedRemaining") > 0) {
        died_ps++;

        if (VS(cur, "age") >= V("deathAge")) {
            died_ps_old++;
        }
        if (died_ps < current_hh) {
            VERBOSE_MODE(false) {
                PLOG("\nHousehold with ID %g died with age %g remaining nutrition need %g.", VS(cur, "Household_ID"), VS(cur, "age"), VS(cur, "hh_NutritionNeedRemaining"));
            }
            DELETE(cur);
        }
        else {
            quit = 1;
            PLOG("\nLast household would have died. Simulation ends prematurely.");
        }

    }
    else {
        living_ps++;
    }
}

WRITE("Died_starving", died_ps - died_ps_old);
WRITE("Died_ofAge", died_ps_old);

/* Some stats for testing */
USE_NAN
STAT("age");
WRITES(p->up, "age_min", v[4]);
WRITES(p->up, "age_max", v[3]);
WRITES(p->up, "age_mean", v[1]);
WRITES(p->up, "age_sd", v[2] > 0 ? sqrt(v[2]) : NAN);

STAT("lastharvest");
WRITES(p->up, "lastharvest_min", v[4]);
WRITES(p->up, "lastharvest_max", v[3]);
WRITES(p->up, "lastharvest_mean", v[1]);
WRITES(p->up, "lastharvest_sd", v[2] > 0 ? sqrt(v[2]) : NAN);

VERBOSE_MODE(false)
{
    PLOG("\nt=%g -- At this time a total of %g died starving and %g died of old age. Remaing %g with min age %g mean %g max %g std dev %g", T, died_ps - died_ps_old, died_ps_old, living_ps, v[4], v[1], v[3], v[2]);

}

TEST_MODE(living_ps != current_hh - died_ps)
{
    PLOG("\nError in update population. Number of households wrong.");
}
WRITE("nHouseholds", living_ps);

VERBOSE_MODE(false)  //OK
{
    LOG("\n At time %g a number of %g households died of which %g died of old age. A number of %i survived.", t, died_ps, died_ps_old, living_ps);
}

WRITE("histothouseholds", P_EXT(ext_population)->hist_pop.at(t - 1));

RESULT(double(died_ps)) //* typical household size

EQUATION("ochousehold")
/* Is a function */
TRACK_SEQUENCE_FIRST_OR_LAST
double old = CURRENT;
double now = 0;
DCYCLE_NEIGHBOUR(cur, "Household", 0.0)
{
    now++;
}


RESULT( now  )

EQUATION("ocfarm")
/* Is a function */
TRACK_SEQUENCE_FIRST_OR_LAST
double now = 0;
DCYCLE_NEIGHBOUR(cur, "Farm", 0.0)
{
    now++;
}

RESULT( now )

EQUATION("Update_apdsi_hydro")
TRACK_SEQUENCE
/* Update APDSI data and "hydro" - whatever that last is.. */
int year = V("Year");
//NetLogo Code below

// ; calculate the yield and whether water is available for each patch based on the PDSI and watere availability data.
//   let generalapdsi item (year - 200) apdsi-data
//   let northapdsi item (1100 + year) apdsi-data
//   let midapdsi item (2400 + year) apdsi-data
//   let naturalapdsi item (3700 + year) apdsi-data
//   let uplandapdsi item (3700 + year) apdsi-data
//   let kinbikoapdsi item (1100 + year) apdsi-data


double generalapdsi = apdsi.at(year - 200);
double northapdsi = apdsi.at(1100 + year);
double midapdsi = apdsi.at(2400 + year);
double naturalapdsi = apdsi.at(3700 + year);
double uplandapdsi = apdsi.at(3700 + year);
double kinbikoapdsi = apdsi.at(1100 + year);

//NetLogo Code below
//   let generalhydro item 1 (item (year - 382) environment-data)
//   let northhydro item 4 (item (year - 382) environment-data)
//   let midhydro item 7 (item (year - 382) environment-data)
//   let naturalhydro item 10 (item (year - 382) environment-data)
//   let uplandhydro item 10 (item (year - 382) environment-data)
//   let kinbikohydro item 13 (item (year - 382) environment-data)

double generalhydro = hydro.at(year - 382).at(1);
double northhydro = hydro.at(year - 382).at(4);
double midhydro = hydro.at(year - 382).at(7);
double naturalhydro = hydro.at(year - 382).at(10);
double uplandhydro = hydro.at(year - 382).at(10);
double kinbikohydro = hydro.at(year - 382).at(13);

bool withBug = V("Assumption_bugUplands") == 0;

CYCLES(p->up, cur, "Land_Patch")
{
    int zone = VS(cur, "land_type");

    switch (zone) {

        case 0:
            WRITES(cur, "hydro", generalhydro); /*general*/
            WRITES(cur, "apdsi", generalapdsi);
            break;
        case 10:
            WRITES(cur, "hydro", northhydro); /*north valley*/
            WRITES(cur, "apdsi", northapdsi);
            break;
        case 15: /*? No change. Is the default "0"? */ /*dunes and empty*/
            break;
        case 25: /*? No change. Is the default "0"? */ /*dunes and empty*/
            break;
        case 20:
            WRITES(cur, "hydro", midhydro); /*mid valley*/
            WRITES(cur, "apdsi", midapdsi);
            break;
        case 30:
            WRITES(cur, "hydro", naturalhydro); /*nonarable uplands / NATURAL */
            WRITES(cur, "apdsi", naturalapdsi);
            break;
        case 40:
            if (!withBug) {
                WRITES(cur, "hydro", uplandhydro); /*arable uplands*/
                WRITES(cur, "apdsi", uplandapdsi);
            }
            else {      //In Janssen there was a bug, search for Upland(s) in lhv.netlog.
                WRITES(cur, "hydro", 0); /*arable uplands*/
                WRITES(cur, "apdsi", 0);
            }
            break;
        case 50:
            WRITES(cur, "hydro", kinbikohydro); /*kinbiko*/
            WRITES(cur, "apdsi", kinbikoapdsi);
            break;
        case 60: /*? No change. Is the default "0"? */ /*dunes and empty*/
            break;
        default:
            TEST_MODE(true) {
                PLOG("The default option should be irrelevant. See line 587!");   /* nothing */
            }
            break;
    }

}



RESULT(double(t))

EQUATION("FindFarmAndSettlement")
/*  Function that updates the best farm and settlement available for the callee.
    Reports -1 of no option found, +1 otherwise.

    The algorithm may be a bit of confusing. This is because it follows the
    original one and we decided to not simplify it too much.
*/
TRACK_SEQUENCE
const bool verbose_mode = false;

if ( V("Potfarms") < 1.0)  //get list of potential farms other than current owned one also updates the list
{
    VERBOSE_MODE(verbose_mode) {
        PLOG("\nNo potential farms.");
    }
    // A: Die, no option
    INCR("Settle_A", 1.0);
    END_EQUATION(-1.0)
}

object* farm = SEARCHS(c, "Farm"); //get farm




//select best farm
object* bestFarmPlace = NULL;
for (auto& farmLand : potential_farms)
{
    if (farmLand.first == true) {
        bestFarmPlace = farmLand.second;
        VERBOSE_MODE(verbose_mode) {
            PLOG("\nFound a new farm with ID %g Position %g,%g and Baseyield %g", VS(bestFarmPlace, "Land_Patch_ID"), POSITION_XS(bestFarmPlace), POSITION_YS(bestFarmPlace), VS(bestFarmPlace, "BaseYield"));
        }
        break;
    }
}

//Set the farm to the new best place
TELEPORT_SHARES(farm, bestFarmPlace);

double bestYield = VS(bestFarmPlace, "yield"); //Yield is associated to zone.

//Try to find a place that is
//  within watersource distance
//  not farmed
//  has yield lower than that of the farm.
//  select closest to farm

object* bestSettlePlace = NULL;
DCYCLE_NEIGHBOURS( bestFarmPlace, cur, "Land_Patch", V("waterSourceDistance") )
{
    if (VS(cur, "ocfarm") == 0 && VS(cur, "yield") < bestYield && VS(cur, "watersource") == 1.0) {
        bestSettlePlace = cur;
        // Is it outside a floodplain?

        if ( VS(bestSettlePlace, "hydro") <= 0 ) {
            INCR("Settle_B", 1.0);
            //B: Settle optimal
        }
        else {
            //C: Settle nearby watersource habitable
            INCR("Settle_C", 1.0);
            bestSettlePlace =  NEAREST_IN_DISTANCE_CNDS(bestSettlePlace, "Land_Patch", -1.0, "habitable", ">", 0.0);
        }
        break;
    }
}



if (bestSettlePlace == NULL) //neither A, B or C.
{
    //try to find any land within watersource distance and not farmed.
    bestSettlePlace =  NEAREST_IN_DISTANCE_CNDS(bestFarmPlace, "Land_Patch", V("waterSourceDistance"), "ocfarm", "=", 0.0);

    if ( NULL != bestSettlePlace ) {
        //Check if it is outside a floodplain
        if (VS(bestSettlePlace, "hydro") <= 0.0) {
            INCR("Settle_D", 1.0);
            //D: Settle
        }
        else {
            // Search nearest place to this that is outside a floodplain
            bestSettlePlace =  NEAREST_IN_DISTANCE_CNDS(bestSettlePlace, "Land_Patch", -1.0, "habitable", ">", 0.0);
            if ( bestSettlePlace != NULL ) {
                //E: Settle
                INCR("Settle_E", 1.0);
            }
            else {
                //F: Die
                INCR("Settle_F", 1.0);
                END_EQUATION(-1.0);

            }
        }
    }
    else {
        //search for a place as close as possible to the bestfarm that is outside a floodplain and not farmed.
        bestSettlePlace =  NEAREST_IN_DISTANCE_CNDS(bestFarmPlace, "Land_Patch", -1.0, "habitable", ">", 0.0);
        if (bestSettlePlace == NULL) {
            //Simply find the one closest and not farmed, even if it is in floodplain
            bestSettlePlace =  NEAREST_IN_DISTANCE_CNDS(bestSettlePlace, "Land_Patch", -1, "ocfarm", "=", 0.0);
            if ( bestSettlePlace == NULL ) {
                //F: Die
                INCR("Settle_F", 1.0);
                END_EQUATION(-1.0)
            }
            else {
                //G: Settle
                INCR("Settle_G", 1.0);
            }
        }
        else {
            //H: Settle
            INCR("Settle_H", 1.0);
        }
    }
}


if (bestSettlePlace != NULL)
{
    //if such a habitable place exists, we move there
    TELEPORT_SHARES(c, bestSettlePlace);
    object* nearestWaterSource = NEAREST_IN_DISTANCE_CNDS(bestSettlePlace, "Land_Patch", -1, "watersource", "=", 1.0);
    nearestWaterSource = SEARCH_POSITION_GRIDS(nearestWaterSource, "Waterpoint");
    WRITES(c, "Waterpoint_by_ID", VS(nearestWaterSource, "Waterpoint_ID"));
    END_EQUATION(1.0) // B: Optimal option found
}
else
{
    PLOG("\nERRORRRRRRRRRRRRRRRRRRRRRRRRRRRR!");
    ABORT;
}

PLOG("\ERROR/INFO: nWe should not get here!");
RESULT(-1.0)

EQUATION("habitable")
/*  Checks if the land patch is not occupied nor in a floodplain
    Needs to be called by fake_caller!
*/
TRACK_SEQUENCE
double is_good = -1.0; //not good
if (V("ocfarm") == 0.0 && V("hydro") <= 0)
{
    is_good = 1.0;
}
RESULT(is_good)

EQUATION("NewPerson")
TRACK_SEQUENCE
/*  This function is called via fake-caller from a parent to create a new spawn.
    It can also be called from root, indicating that we are in initialisation mode.
    Its value represents the number of times it has created households.
*/
//add new person and add it to space.

//can the land support another household?

// Note, all these objects are created "as if" they processed the day
//   when the function was called by root, i.e. to initialise the lot. Else,
//   they will be added as if the did not yet do anything (beginning of the
//   day).
//  RECALS can be used to change behaviour for single variables (like resetle).

int last_update = t; //now
if (c == root)
{
    last_update = 0;   //day bevor -- update all variables!
}
object* hh = ADDOBJLS(p->up, "Household", last_update); //Create new Household
//Assign unique IDs
WRITES(hh, "Household_ID", UIDS(hh));
INCRS(p->up, "total_Households", 1);
object* farm = SEARCHS(hh, "Farm");
WRITES(farm, "Farm_ID", UIDS(farm));
INCRS(p->up, "total_Farms", 1);


ADD_TO_SPACE_XYS(hh, root, RANDOM_POSITION_XS(root), RANDOM_POSITION_YS(root));
ADD_TO_SPACE_XYS(farm, root, RANDOM_POSITION_XS(root), RANDOM_POSITION_YS(root));

//next: find a settlement position
object* Settings = SEARCHS(root, "Settings");
RECALCS(hh, "resettle");
double allowed =  V_CHEATS(hh, "resettle", root); //force resettle, no update of estimate etc.

if (allowed < 0)
{
    //we kill the household again.
    VERBOSE_MODE(false) {
        PLOG("\nAdded new household with ID %g and killed it again for there is not enough habitable land", UIDS(hh));
    }
    DELETE(hh);
    END_EQUATION(CURRENT); //no spawn
}
else
{
    INCR("nHouseholds", 1);
    INCR("newHouseholds", 1);

    VERBOSE_MODE(false) {
        PLOG("\nAdded new household with ID %g", UIDS(hh));
    }

    //Initialisation at t==0, i.e. without parents
    if (c == root) {

        object* cornStock = SEARCHS(hh, "agedCornStock"); //we will later delete the old place holder
        WRITES(cornStock, "cornStock", V("householdMinInitialCorn") + uniform(0.0, 1.0) * (V("householdMaxInitialCorn") - V("householdMinInitialCorn")) );
        WRITES(cornStock, "cornYear", V("Year"));
        WRITES(hh, "age", round(V("householdMinInitialAge") + (V("householdMaxInitialAge") - V("householdMinInitialAge")) * uniform(0.0, 1.0)) ); //Initial age

        //If there are parents, we inherit their attributes from the caller
    }
    else {

        WRITES(hh, "age", V("newHouseholdAge")); //new Agents start with age 0


        /* Get corn from parent */
        object* to_delete = SEARCHS(hh, "agedCornStock"); //we will later delete the old place holder

        //We distribute the stock of corn passed by evenly among different vintages. How does Janssen do it?

        CYCLES(c, cur, "agedCornStock") {
            double gift = VS(cur, "cornStock") * V("maizeGiftToChild");
            INCRS(cur, "cornStock", -gift);
            cur1 = ADDOBJS(hh, "agedCornStock");
            WRITES(cur1, "cornStock", gift);
            WRITES(cur1, "cornYear", VS(cur, "cornYear"));
        }

        DELETE(to_delete);
    }

    END_EQUATION(CURRENT++); //successful spawn.
}

RESULT(CURRENT)

EQUATION("Init_population")
TRACK_SEQUENCE
/* Initialise the population */

/* Load reference data from Janssen */
ADDEXT(ext_population);
auto ext_pop =  P_EXT(ext_population); //reference to the ext_obj
if ( !ext_pop->init_hist_pop(TOTALSTEPS) )
{
    PLOG("\nError: Could not initialise historical population levels.");
    ABORT;
}



//save initial object to delete
object* del_hh = SEARCHS(p->up, "Household");

i = 0;
while(i < V("nHouseholdsInitial"))
{
    V_CHEAT("NewPerson", root);
    i++;
}


//Check that the corresponding element is not the last one. Use info that it must be the first one.
if (del_hh->next != NULL)
{
    DELETE(del_hh);
}
else
{
    PLOG("\nError in initialisation. All persons are already dead?!");
    //   INTERACT("kill",0.0);
    ABORT
    END_EQUATION(0.0);
}

VERBOSE_MODE(true)   //checked
{
    PLOG("\nInit_Population: Initialised and connected a total of %i persons/households/farms", i);
}
PARAMETER
RESULT(double(i))

EQUATION("Init_geography")
/* This function initialises the geography */
TRACK_SEQUENCE
potential_farms_allTime.clear();
//Initialise the GIS
int wrapping = (V("Assumption_wrapping") < 0 ? 0 : 15); //turn wrapping on if assumption is set (>=0)
INIT_SPACE_GRID_WRAPLS(p->up, "Land_Patch", x_cols, y_rows, wrapping, 0);
double id = 0;
CYCLES(p->up, cur, "Land_Patch")
{
    WRITES(cur, "Land_Patch_ID", ++id);
    if(!ANY_GISS(cur) ) {
        PLOG("\nError! There is no GIS connected to the Patch %g.", VS(cur, "Land_Patch_ID"));
    }
    else {
        //PLOG("\nAdded Patch with ID %g at Position %g,%g.",VS(cur,"Land_Patch_ID"),POSITION_XS(cur),POSITION_YS(cur));
    }
}
cur = SEARCHS(root, "Land_Patch");
ADD_ROOT_TO_SPACE(cur);

//Initialise the environmental data broker
ADDEXT(ext_environment);
auto ext_env =  P_EXT(ext_environment); //reference to the ext_obj
#ifdef VALIDATION
if ( !ext_env->init_validate(TOTALSTEPS, x_cols, y_rows) )
{
    PLOG("\nError: Could not initialise validation data.");
    ABORT;
}
#endif

std::vector <double> mapview_zones; //more efficient: outside of LSD model, so only load once in a row of sims. Relevant for all data.

std::ifstream  map_data("data/map.txt");
if (!map_data)
{
    PLOG("\nError: No map data at path: 'data/map.txt'");
    ABORT;
}
//       cur = SEARCHS(p->up,"Land_Patch"); //Get first patch of land
int x = 0, y = y_rows - 1;
string s;
while (map_data >> s)  //read entry by entry
{
    cur = SEARCH_POSITION_XYS(SEARCHS(p->up, "Land_Patch"), "Land_Patch", x, y);
    int map_info = std::stoi(s); //convert string to int.
    switch (map_info) {
        /*Land_Type, maizeZone, color for map*/

        case  0:  /*General, Yield_2, black*/
            WRITES(cur, "land_type", 0);
            WRITES(cur, "maizeZone", 2);
            WRITES(cur, "map_colour", 0); //colours from LSD 7 lattice
            potential_farms_allTime.push_back(cur);
            break;

        case  10: /*North, Yield_1, red*/
            WRITES(cur, "land_type", 10);
            WRITES(cur, "maizeZone", 1);
            WRITES(cur, "map_colour", 1);
            potential_farms_allTime.push_back(cur);
            break;

        case  15:  /*North Dunes, Sand_dune, white*/
            WRITES(cur, "land_type", 15);
            WRITES(cur, "maizeZone", 4);
            WRITES(cur, "map_colour", 1000);
            potential_farms_allTime.push_back(cur);
            break;

        case  20:  /*Mid, x<=74?Yield_1:Yield_2, gray*/
            WRITES(cur, "land_type", 20);
            WRITES(cur, "maizeZone", x <= 74 ? 1 : 2);
            WRITES(cur, "map_colour", 7);
            potential_farms_allTime.push_back(cur);
            break;

        case  25:  /*Mid Dunes, Sand_dune, white*/
            WRITES(cur, "land_type", 25);
            WRITES(cur, "maizeZone", 4);
            WRITES(cur, "map_colour", 1000);
            potential_farms_allTime.push_back(cur);
            break;

        case  30:  /*Natural, No_Yield, yellow*/
            WRITES(cur, "land_type", 30);
            WRITES(cur, "maizeZone", 0);
            WRITES(cur, "map_colour", 3);
            //not ever grows maize
            break;

        case  40:  /*Uplands, Yield_3, blue*/
            WRITES(cur, "land_type", 40);
            WRITES(cur, "maizeZone", 3);
            WRITES(cur, "map_colour", 5);
            potential_farms_allTime.push_back(cur);
            break;

        case  50:  /*Kinbiko, Yield_1, pink*/
            WRITES(cur, "land_type", 50);
            WRITES(cur, "maizeZone", 1);
            WRITES(cur, "map_colour", 4);
            potential_farms_allTime.push_back(cur);
            break;

        case  60:  /*Empty, Empty, white*/
            WRITES(cur, "land_type", 60);
            WRITES(cur, "maizeZone", 0);
            WRITES(cur, "map_colour", 1000);
            //not ever grows maize
            break;

        default:
            PLOG("\n Error: map.txt is not correctly codified!");
            ABORT;
    }

    if (y == y_rows - 1) {
        //           select_patch.push_back(std::vector<object*>{}); //add new column
    }
    WRITES(cur, "Land_Patch_x", double(x)); //TODEL
    WRITES(cur, "Land_Patch_y", double(y)); //TODEL
    //         select_patch.at(x).push_back(cur); //currently in reversed order.


    if (y > 0) {
        y--;
    }
    else {
        /*  We are now at the end of the first x column. However, the columns
            where filled "reversed", for this is the data-file format.
            We change this now for each column processed.
        */
        //           std::reverse(select_patch.at(x).begin(),select_patch.at(x).end());    //fine!
        //and move to the next column
        x++;
        y = y_rows - 1;
    }
    //         cur = cur->next; //move to next patch
}
map_data.close();


TEST_MODE(true)  //checked!
{
    int errs = 0;
    int siz = 0;
    for (int xx = 0; xx < x_cols; xx++) {
        for (int yy = 0; yy < y_rows; yy++) {
            siz++;
            TEST_MODE(xx != VS( SEARCH_POSITION_XYS(SEARCHS(p->up, "Land_Patch"), "Land_Patch", xx, yy), "Land_Patch_x") || yy != VS( SEARCH_POSITION_XYS(SEARCHS(p->up, "Land_Patch"), "Land_Patch", xx, yy), "Land_Patch_y")) {
                errs++;
            }
        }
    }
    PLOG("\nTested the mapping of land_patch via x,y coords used in POSITION(..,x,y). %i errors in %i patches.", errs, siz);
}

PARAMETER
RESULT(0.0)

EQUATION("Init_Waterpoints")
TRACK_SEQUENCE
/*  Next, we add the waterpoints that update Land_Patch->watersource

    We use the data in water.txt and also the hardcoded data in Janssen.2009
*/
object* exmplGisObj = SEARCHS(root, "Land_Patch");
std::ifstream  water_data("data/water.txt");
string s;
if (!water_data)
{
    PLOG("\nError: No water data at path: 'data/water.txt'");
    ABORT;
}
int items = 0;
int lists = 0;
//string s; //allready there
//start by readying all in, as in Janssen.

//In the file there is a last entry all zeros. netlogo does not read it
//in. I skip it manually.
bool include_last_item = false;

std::vector< std::vector < int > > water_input; //y , x, typewater, start, end
while (water_data.peek() != EOF && (lists != 108 || include_last_item) )
{
    lists++;
    std::vector < int > entry; //there should be 108 entries
    //PLOG("\nentry %i: ",lists);
    for (int i = 0; i < 6; i++) {
        items++;
        water_data >> s; //read item
        entry.push_back(std::stod(s)); //add to list
        //PLOG("(%i,%s) ",items,s.c_str());
    }
    water_input.push_back(entry); //add to list of lists
}



TEST_MODE(lists != 108)
{
    PLOG("\nError! There should be 108 lists of 6 each. There are: %i", lists);
}
TEST_MODE(items != 108 * 6)
{
    PLOG("\nError! There should be %i total items. There are: %i", 108 * 6, items);
}

//Next, transform coordinates from east / north to x,y in grid and add
//waterpoints as temporary structs

struct waterpoint {
    double x = -1;
    double y = -1;
    int typewater = -1;
    int startdate = -2;
    int enddate = -2;
};

std::vector <waterpoint> waterpoints;
for (auto& wi : water_input)
{
    waterpoint temp;
    temp.x = int(ceil(24.5 + int ((wi.at(2) - 2392) / 93.5) )); //same truncation etc as in janssen
    temp.y = int(ceil(45 + int (37.6 + ((wi.at(1) - 7954) / 93.5) ) ));
    temp.typewater =  wi.at(3);
    temp.startdate = wi.at(4);
    temp.enddate = wi.at(5);
    waterpoints.push_back(temp);
}
TEST_MODE(waterpoints.size() != 108)
{
    PLOG("\nError: The number of waterpoints is %i and should be %i", waterpoints.size(), 108);
}

/*  the "typewater" variable can be 1,2 or 3. 1 is ignored, 2 implies
    permanent water supply and 3 only temporary water supply.

    In this implementation, a special agent "Waterpoint" will be added for
    any watersource. (type 2 or 3). This agent contains objects water-period
    that tell when there will be a watersource. In case of the type 2 (per-
    manent) watersource, the start end enddate are set to -1.

    In addition to the sources from the water.txt, there are some hard-coded
    sources of type 2. I will add them now.
*/

//by position
std::vector < std::vector < int > > hard_coded_sources = {
    {72, 114},
    {70, 113},
    {69, 112},
    {68, 111},
    {67, 110},
    {66, 109},
    {65, 108},
    {65, 107}
};
for (auto& item : hard_coded_sources)
{
    waterpoint temp;
    temp.x =  item.at(0);
    temp.y =  item.at(1);
    temp.typewater =  2;
    temp.startdate = 0;
    temp.enddate = 0;
    waterpoints.push_back(temp);
}

//And add waterpoint objects "by zone" for times where alluvium or
//streamsexists
std::vector < std::vector < int > > streamexist = {
    {280, 360   - 1   },
    {800, 930   - 1   },
    {1300, 1450  - 1  }
};

std::vector < std::vector < int > > alluviumexist = {
    {420, 560  - 1 },
    {630, 680  - 1 },
    {980, 1120 - 1 },
    {1180, 1230 - 1 }
};

CYCLES(p->up, cur, "Land_Patch")
{
    int zone = VS(cur, "land_type");
    if(zone == 0 /*"General"*/  ||
            zone == 10 /*"North"*/    ||
            zone == 20 /*"Mid"*/      ||
            zone == 50 /*"Kinbiko"*/) {
        for (auto& item : alluviumexist) {
            waterpoint temp;
            temp.x =  VS(cur, "Land_Patch_x");
            temp.y =  VS(cur, "Land_Patch_y");
            temp.typewater =  3;
            temp.startdate = item.at(0);
            temp.enddate = item.at(1);
            waterpoints.push_back(temp);
        }
    }
    if (zone == 50 /*"Kinbiko"*/) {
        for (auto& item : streamexist) {
            waterpoint temp;
            temp.x =  VS(cur, "Land_Patch_x");
            temp.y =  VS(cur, "Land_Patch_y");
            temp.typewater =  3;
            temp.startdate = item.at(0);
            temp.enddate = item.at(1);
            waterpoints.push_back(temp);
        }
    }
}

//sort by x then y
std::sort(std::begin(waterpoints), std::end(waterpoints), [](auto const& t1, auto const& t2)
{
    return (t1.x < t2.x ? true :  ( t1.x > t2.x ? false :   /*x equal*/
                                    ( t1.y < t2.y ? true : (t1.y > t2.y ? false : /*y equal*/
                                            ( t1.startdate < t2.startdate ? true : false) ) ) ) ); //sort ascending by x then y then startdate (weak)
});


//Now, create the relevant Waterpoint objects and set the relevant
//time-periods
auto wp_to_delete =  SEARCHS(p->up, "Waterpoint"); //the old blueprint will be deleted later

waterpoint prior;
int wper_count = 0;
double wps_count = 0;
for (auto& item : waterpoints)
{
    //process only valid items
    if (item.x < 0.0 || item.x >= x_cols || item.y < 0.0 || item.y >= y_rows ) {
        VERBOSE_MODE(false) {
            PLOG("\nIn setup of waterpoints, skipped entry with x=%i, y=%i.", int(item.x), int(item.y));
        }
        continue; //skip this item
    }

    if (prior.x == item.x && prior.y == item.y && prior.typewater == item.typewater && prior.startdate == item.startdate && prior.enddate == prior.enddate ) {
        VERBOSE_MODE(false) {
            PLOG("\nIn setup of waterpoints, skipped multiple entry with x=%i, y=%i, typewater=%i, t=%i,%i.", item.x, item.y, item.typewater, item.startdate, item.enddate);
        }
        continue;
    }

    wper_count++;

    if (item.typewater == 2 || item.typewater == 3) { //only create relevant agents
        if (! (item.x == prior.x && item.y == prior.y) ) {
            prior = item; //copy information
            cur = ADDOBJLS(p->up, "Waterpoint", 0); //Add new agent
            WRITES(cur, "Waterpoint_x", item.x);
            WRITES(cur, "Waterpoint_y", item.y);
            WRITES(cur, "Waterpoint_ID", wps_count++);
            //Add external object
            ADD_TO_SPACE_XYS(cur, exmplGisObj, item.x, item.y); //Add to GIS
            cur1 = SEARCHS(cur, "water_period"); //select existing blueprint
        }
        else {
            cur1 = ADDOBJS(cur, "water_period"); //add ne waterperiod to existing agent
        }

        WRITES(cur1, "water_start", item.typewater == 3 ? item.startdate : -1 /*else permanent*/);
        WRITES(cur1, "water_end", item.typewater == 3 ? item.enddate : -1 /*else permanent*/);
        WRITES(cur1, "typewater", item.typewater);

    }
}
DELETE(wp_to_delete);

/* Note: Some logical tests are made manually, checking the printed data below*/
VERBOSE_MODE(false)  //checked and fine
{
    PLOG("\nt is %i:Added a total of %i waterperiods, distributed over %g waterpoints.", t, wper_count, wps_count );
    VERBOSE_MODE(false) {
        CYCLES(p->up, cur, "Waterpoint") {
            CYCLES(cur, cur1, "water_period") {
                PLOG("\n xy: %i,%i\ttype: %i \ttime: %i,%i", int(VS(cur, "Waterpoint_x")), int(VS(cur, "Waterpoint_y")), int(VS(cur1, "typewater")), int(VS(cur1, "water_start")), int(VS(cur1, "water_end")) );
            }
        }
    }
}

//Update once at beginning -- conflict with NetLogo?
CYCLES(p->up, cur, "Waterpoint")
{
    VS(cur, "Update_waterSource");
}

PARAMETER
RESULT(wps_count)

EQUATION("Update_waterSource")
/* Update the watersource information of the associated patch of land. */
TRACK_SEQUENCE_FIRST_OR_LAST
// TRACK_SEQUENCE_FIRST_OR_LAST_ALWAYS

auto associated_patch = SEARCH_POSITION_GRID("Land_Patch");
double is_source = 0.0;
CYCLE(cur, "water_period")
{
    if (VS(cur, "typewater") == 2.0) { //there is always water
        is_source = 1.0;
        VERBOSE_MODE(false) {
            PLOG("\nChecking watersource: %i,%i, setting is_source 1 for ever", int(VS(cur, "Waterpoint_x")), int(VS(cur, "Waterpoint_y")));
        }
        break;
    }
    else if (VS(cur, "typewater") == 3.0 && V("Year") >= VS(cur, "water_start") && V("Year") <= VS(cur, "water_end") ) {
        is_source = 1.0;
        VERBOSE_MODE(false) {
            PLOG("\nChecking watersource: %i,%i, zone %i setting is_source 1", int(VS(cur, "Waterpoint_x")), int(VS(cur, "Waterpoint_y")), int(VS(associated_patch, "land_type")) );
        }
        break;
    }
}
WRITES( associated_patch, "watersource", is_source ); //default
RESULT(is_source)

/****************************

          Land_Patch

 *        *******************/

EQUATION("yield")
/*
    The potential yield at any given point in time.

    From the NetLogo code 414-443:

    if (maizeZone = "No_Yield" or maizeZone = "Empty") [set yield 0]
    if (maizeZone = "Yield_1") [
      if (apdsi >=  3.0) [set yield 1153]
      if (apdsi >=  1.0 and apdsi < 3.0) [set yield 988]
      if (apdsi >  -1.0 and apdsi < 1.0) [set yield 821]
      if (apdsi >  -3.0 and apdsi <= -1.0) [set yield 719]
      if (apdsi <= -3.0) [set yield 617]]

    if (maizeZone = "Yield_2") [
      if (apdsi >=  3.0) [set yield 961]
      if (apdsi >=  1.0 and apdsi < 3.0) [set yield 824]
      if (apdsi >  -1.0 and apdsi < 1.0) [set yield 684]
	    if (apdsi >  -3.0 and apdsi <= -1.0) [set yield 599]
	    if (apdsi <= -3.0) [set yield 514]]

    if (maizeZone = "Yield_3") [
      if (apdsi >=  3.0) [set yield 769]
	    if (apdsi >=  1.0 and apdsi < 3.0) [set yield 659]
	    if (apdsi >  -1.0 and apdsi < 1.0) [set yield 547]
	    if (apdsi > -3.0 and apdsi <= -1.0) [set yield 479]
	    if (apdsi <= -3.0) [set yield 411]]

    if (maizeZone = "Sand_dune") [
      if (apdsi >=  3.0) [set yield 1201]
	    if (apdsi >=  1.0 and apdsi < 3.0) [set yield 1030]
	    if (apdsi >  -1.0 and apdsi < 1.0) [set yield 855]
	    if (apdsi >  -3.0 and apdsi <= -1.0) [set yield 749]
	    if (apdsi <= -3.0) [set yield 642]]
*/
TRACK_SEQUENCE_FIRST_OR_LAST
double yield = 0.0;
int maizeZone = V("maizeZone");
double apdsi = V("apdsi");

switch (maizeZone)
{

    // "No_Yield" or "Empty"
case 0:
    break;

    // Yield_1
case 1:
    if (apdsi >=  3.0) {
        yield = 1153;
    }
    else if (apdsi >=  1.0) {
        yield = 988;
    }
    else if (apdsi >  -1.0) {
        yield = 821;
    }
    else if (apdsi >  -3.0) {
        yield = 719;
    }
    else {
        yield = 617;
    }
    break;

    // Yield_2
case 2:
    if (apdsi >=  3.0) {
        yield = 961;
    }
    else if (apdsi >=  1.0) {
        yield = 824;
    }
    else if (apdsi >  -1.0) {
        yield = 684;
    }
    else if (apdsi >  -3.0) {
        yield = 599;
    }
    else {
        yield = 514;
    }
    break;

    // Yield_3
case 3:
    if (apdsi >=  3.0) {
        yield = 769;
    }
    else if (apdsi >=  1.0) {
        yield = 659;
    }
    else if (apdsi >  -1.0) {
        yield = 547;
    }
    else if (apdsi >  -3.0) {
        yield = 479;
    }
    else {
        yield = 411;
    }
    break;

    // Sand_dune
case 4:
    if (apdsi >=  3.0) {
        yield = 1201;
    }
    else if (apdsi >=  1.0) {
        yield = 1030;
    }
    else if (apdsi >  -1.0) {
        yield = 855;
    }
    else if (apdsi >  -3.0) {
        yield = 749;
    }
    else {
        yield = 642;
    }
    break;

default:
    break;
}

RESULT(yield)

EQUATION("BaseYield")
/*  The adjusted yield at a specific point in time.
    Using the global parameter "harvestAdjustment":
    BaseYield = yield * quality * harvestAdjustment

    Deterministic.
*/
TRACK_SEQUENCE_FIRST_OR_LAST
RESULT(V("yield")*V("quality")*V("harvestAdjustment"))



EQUATION("quality")
TRACK_SEQUENCE_FIRST_OR_LAST
double quality = max(0, norm(0.0, 1.0) * V("harvestVariance") + 1);
PARAMETER
RESULT(quality)



EQUATION("Seed")
/*  Save the current seed as parameter
    Or, if a parameter, provide it.
*/
TRACK_SEQUENCE
PARAMETER
//PLOG("\nUsing LSD automatic seed: %i", seed - 1);
RESULT(RND_SEED)


EQUATION("Capacity")
/*  Theoratical capacity of the land
*/
TRACK_SEQUENCE

double BaseYield;
double Capacity = 0; //the capacity indicates how many households can be sustained. It is a crude measure!
double nutruitionneed = V("baseNutritionNeed") * V("typicalHouseholdSize");
sustainable_farms.clear(); //set free pointers
for (auto& pot_F : potential_farms_allTime)
{
    BaseYield = VS(pot_F, "BaseYield");
    if (BaseYield >= nutruitionneed) {
        Capacity++;
        sustainable_farms.emplace_back(BaseYield, pot_F);
    }
}
std::sort(sustainable_farms.rbegin(), sustainable_farms.rend()); //sort increasing by yield. Note the "r" for reverse!
potential_farms.clear();
i = 0;
//add all the sustainable farms to the list of potential farms
for (auto& farm : sustainable_farms)
{
    potential_farms.emplace_back(false, farm.second);
    i++;
    VERBOSE_MODE(false) {
        PLOG("\n%i: %s %g at %g,%g has baseyield %g.", i, farm.second->label, VS(farm.second, "Land_Patch_ID"), POSITION_XS(farm.second), POSITION_YS(farm.second), VS(farm.second, "BaseYield"));
    }
}
VERBOSE_MODE(false)
{
    PLOG("\n\nt=%i -- Total Capacity is %g == %i?", t, Capacity, i);
}

RESULT(Capacity)

EQUATION("Potfarms")
/* Is a functionDetermine number of farms available */
TRACK_SEQUENCE
V("Capacity"); //make sure it is up to date
double farmSitesAvailable = 0;

//check for each potential farm if it is available or not
for (auto& farm : potential_farms)
{
    if (VS(farm.second, "ocfarm") == 0 && VS(farm.second, "ochousehold") == 0) {
        farm.first = true;
        farmSitesAvailable++;
    }
    else {
        farm.first = false; //occupied
    }
}
RESULT(farmSitesAvailable)


MODELEND




void close_sim(void)
{

}


