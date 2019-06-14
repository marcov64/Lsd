/* ==========================================================================

  fun_pajek.cpp

  Contains pajek lsd equation

 ========================================================================== */

EQUATION("Pajek")
/* Gather descriptive network information and pass it to pajek*/
#ifdef MODULE_PAJEK
if (t == 1)
{
    PAJ_INIT("Networks", "ArtificialAnasazi", RND_SEED, "Artificial Anasazi - Replication of Janssen 2009") //Initialise pajek dynamic mode

    /* Save static information */
    const bool save_static_info = false;
    if (save_static_info) {

        {
            //Save the zones as SVG
            PAJ_STATIC("Networks", "AA_ZonesMap", 0, "Artificial Anasazi - Replication of Janssen 2009 - Zones Map")
            CYCLE(cur, "Land_Patch") {
                PAJ_S_ADD_V_C(0, VS(cur, "Land_Patch_ID"), "Land_Patch", VS(cur, "land_type"), POSITION_XS(cur), POSITION_YS(cur), "box", 1.0, 1.0, MapColor(VS(cur, "map_colour")))
            }
            PAJ_S_SAVE //save
        } //scope end

        {
            //Create a legend SVG (basis - move vertices in pajek)
            PAJ_STATIC("Networks", "AA_Legend", 1, "Artificial Anasazi - Replication of Janssen 2009 - Legend")
            PAJ_S_ADD_V_CL(0, 0, "MaizeZone", 1, 0, 0, "diamond", 0.5, 1.0, "Dandelion", "Maize zone (suitable)")
            PAJ_S_ADD_V_CL(0, 1, "MaizeZone", 1, 0, 0, "diamond", 0.5, 0.5, "RedOrange", "Maize zone (not sustainable)")
            PAJ_S_ADD_V_CL(0, 2, "Floodplain", 1, pos_x(5, 1), pos_x(5, 2), "box", base_svg_size * 3, base_svg_size * 3, "MidnightBlue", "Floodplain area (hydro>0)")
            PAJ_S_ADD_V_CL(0, 3, "Waterpoint", 1, pos_x(2, 2), pos_y(2, 3), "triangle", base_svg_size, base_svg_size, "Blue", "Waterpoint (active)")
            PAJ_S_ADD_V_CL(0, 4, "Waterpoint", 1, pos_x(2, 2), pos_y(2, 4), "triangle", base_svg_size, base_svg_size, "Gray45", "Waterpoint (not active)")
            PAJ_S_ADD_V_CL(0, 5, "Settlement", 1, pos_x(5, 3), pos_y(5, 5), "house", base_svg_size * 2, base_svg_size * 2, "Red", "Settlement (1 hh)")
            PAJ_S_ADD_V_CL(0, 6, "Settlement", 10, pos_x(5, 3), pos_y(5, 6), "house", base_svg_size * 1 * (2 + sqrt(10)), base_svg_size * 1 * (2 + sqrt(10)), "Red", "Settlement (10 hh)")
            PAJ_S_ADD_V_CL(0, 7, "Settlement", 10, pos_x(5, 3), pos_y(5, 7), "house", base_svg_size * 1 * (2 + sqrt(50)), base_svg_size * (2 + sqrt(50)), "Red", "Settlement (50 hh)")
            PAJ_S_ADD_V_CL(0, 8, "Farm", 1, pos_x(5, 8), pos_y(5, 4), "cross", 1.0, 1.0, "PineGreen", "close Farm, sustainable")
            PAJ_S_ADD_V_CL(0, 9, "Farm", 1, pos_x(5, 9), pos_y(5, 4), "cross", 1.0, 1.0, "Orchid", "distant Farm, not sustainable")
            PAJ_S_ADD_A_C(0, 5, "Settlement", 3, "Waterpoint", -17.0, "s-w out of distance", base_svg_lw, "Blue")
            PAJ_S_ADD_A_C(0, 5, "Settlement", 8, "Farm", 5.0, "s-f in distance", base_svg_lw, "PineGreen")
            PAJ_S_ADD_A_C(0, 8, "Farm", 3, "Waterpoint", 16.0, "f-w just in distance", base_svg_lw, "RedOrange")
            PAJ_S_ADD_A_C(0, 7, "Settlement", 4, "Waterpoint", 8.0, "s-w in distance", base_svg_lw, "Blue")
            PAJ_S_ADD_A_C(0, 7, "Settlement", 9, "Farm", -22.0, "s-f out of distance", base_svg_lw, "PineGreen")
            PAJ_S_ADD_A_C(0, 9, "Farm", 4, "Waterpoint", 8.0, "f-w in distance", base_svg_lw, "RedOrange")
            PAJ_S_SAVE
        }

    }
}

double nutruitionneed = V("baseNutritionNeed") * V("typicalHouseholdSize");
CYCLE(cur, "Land_Patch")
{
    double co_x = POSITION_XS(cur);
    double co_y = POSITION_YS(cur);

    //     PAJ_ADD_V_C(t,VS(cur,"Land_Patch_ID"),"Land_Patch",VS(cur,"land_type"),co_x,co_y,"box",1.0,1.0,MapColor(VS(cur,"map_colour")))

    if (VS(cur, "maizeZone") > 0) {
        double yield = VS(cur, "BaseYield");
        PAJ_ADD_V_C(t, VS(cur, "Land_Patch_ID"), "MaizeZone", yield, co_x, co_y, "diamond", MaizeSize(nutruitionneed, yield), MaizeSize(nutruitionneed, yield), nutruitionneed > yield ? "RedOrange" : "Dandelion")
    }

    if (VS(cur, "hydro") > 0) { //not suitable
        PAJ_ADD_V_C(t, VS(cur, "Land_Patch_ID"), "Floodplain", VS(cur, "hydro"), pos_x(5, co_x), pos_x(5, co_y), "box", base_svg_size * 2, base_svg_size * 2, "MidnightBlue")
    }
}

double waterSourceDistance = V("waterSourceDistance");
CYCLE(cur, "Waterpoint")
{
    PAJ_ADD_V_C(t, VS(cur, "Waterpoint_ID"), "Waterpoint", VS(cur, "Update_waterSource"), pos_x(3, POSITION_XS(cur) ), pos_y(3, POSITION_YS(cur) ), "triangle", base_svg_size, base_svg_size, VS(cur, "Update_waterSource") > 0 ? "Blue" : "Gray30")
}

CYCLE(cur, "Household")
{
    double n_settlement = 1;
    DCYCLE_NEIGHBOURS(cur, cur1, "Household", 0.0) {
        n_settlement++;
    }
    //object* patch = P_EXTS(cur,ext_household)->patch;
    //to do: circle around
    PAJ_ADD_V_C(t, VS(cur, "Household_ID"), "Household", n_settlement /* change to #members */, pos_x(5, POSITION_XS(cur) ), pos_y(5, POSITION_YS(cur) ), "house", base_svg_size * 1 * (2 + sqrt(n_settlement)), base_svg_size * 1 * (2 + sqrt(n_settlement)), "Red")
    PAJ_ADD_A_C(t, VS(cur, "Household_ID"), "Household", VS(cur, "Waterpoint_by_ID"), "Waterpoint", VS(cur, "Household_distWater") > waterSourceDistance ? -1.0 : 1.0, "drinking", base_svg_lw, "Blue")

    //and relations
    object* farm = NULL;
    CYCLES(cur, farm, "Farm") {
        PAJ_ADD_V_C(t, VS(farm, "Farm_ID"), "Farm", 1.0 /* change to lastharvest */, pos_x(5, POSITION_XS(farm)), pos_y(5, POSITION_YS(farm)), "cross", base_svg_size * 3, base_svg_size * 3, VS(cur, "lastharvest") > nutruitionneed ? "PineGreen" : "Orchid")
        PAJ_ADD_A_C(t, VS(cur, "Household_ID"), "Household", VS(farm, "Farm_ID"), "Farm", VS(cur, "Household_distFarm") > waterSourceDistance ? -1.0 : 1.0, "farming", base_svg_lw, "PineGreen")
        PAJ_ADD_A_C(t, VS(farm, "Farm_ID"), "Farm", VS(cur, "Waterpoint_ID"), "Waterpoint", VS(farm, "Farm_distWater") > waterSourceDistance ? -1.0 : 1.0, "farmWater", base_svg_lw, "RedOrange")
    }
}
if (quit != 0){
    PAJ_SAVE;
}
#endif
RESULT(0.0)