/* ==========================================================================

  fun_lattice.cpp

  Contains behavioural routines of the agents

 ========================================================================== */

EQUATION("Update_Lattice")
/* Updates the lattice according to the current view. */
V("Init_Lattice");

  #ifndef NOLATTICE
 
  int Update_Interval = V("Update_Interval");

  //If the updating is deselected, no lattice is printed.
  if (Update_Interval == 0){
    PARAMETER;
    END_EQUATION(0);
  }

  int Mapview = V("Mapview");
  int views =  P_EXT(ext_lattice)->views; //number of pot views, 6
  /* views:
  0 = Zones (special)
  1 = Watersources (blue/white)
  2 = Hydro (green/white)
  3 = Occupation (blue--green--red)
  4 = Yield (black;white--yellow)
  5 = apdsi (red-white-blue)
  */
  if (Mapview < -1 || Mapview > views-1){
    PLOG("\nERROR: You selected an invalid view. It must be in (%i,%i)",-1,views-1);
    ABORT;
    END_EQUATION(0.0);
  }

  //Check if an update is warranted
  if( t==1 || (t+1)%Update_Interval==0 || t == TOTALSTEPS){ //create each 40 timesteps a view.

    /*
        Update the data where necessary
    */
    object *model_obj = SEARCHS(root,"Model");
    object *c_patch;
    int x,y;
    // 0 = Zones (special) & zones never change
    if (t==1 && (Mapview == 0 || Mapview == -1) ){
      CYCLES(model_obj,c_patch,"Land_Patch"){
        x = int(POSITION_XS(c_patch));
        y = int(POSITION_YS(c_patch));
        P_EXT(ext_lattice)->add_mapview_colour(0 /*zones view*/,x,y,VS(c_patch,"map_colour"));
      }
    }

    // 1 = Watersources (blue/white)
    if (Mapview == 1 || Mapview == -1){
      CYCLES(model_obj,c_patch,"Land_Patch"){
        x = int(POSITION_XS(c_patch));
        y = int(POSITION_YS(c_patch));
        P_EXT(ext_lattice)->add_mapview_colour(1 /*watersources view*/,x,y, VS(c_patch,"watersource") == 0 ? 0 : 6 /* dark blue */);
      }
    }

    // 2 = Hydro (green/white)
    if (Mapview == 2 || Mapview == -1){
      CYCLES(model_obj,c_patch,"Land_Patch"){
        x = int(POSITION_XS(c_patch));
        y = int(POSITION_YS(c_patch));
          double hydro = VS(c_patch,"hydro");
          hydro = hydro < -5 ? -5 : (hydro > 5 ? 5 : hydro); //bound in -5,5
          hydro = ColorGradient::normalise_value(-5,5,hydro);
          int rgb = ColorGradient::LSD_rgb(hydro,2 );  /*red-white-blue*/
          P_EXT(ext_lattice)->add_mapview_colour(2 /*Hydro*/,x,y,rgb);
      }
    }

    // 3 = Occupation (yellow; red; pink)
    if (Mapview == 3 || Mapview == -1){
      CYCLES(model_obj,c_patch,"Land_Patch"){
        x = int(POSITION_XS(c_patch));
        y = int(POSITION_YS(c_patch));
          int ochh = VS(c_patch,"ochousehold");
          int ocfarm = VS(c_patch,"ocfarm");
          int rgb = 0; //black, default
          if (ocfarm == 1){
            rgb = 3; //yellow
          } else {
            if (ochh == 1) {
              rgb = 1; //red
            } else if (ochh > 1) {
              rgb = 4; //pink
            }
          }
          P_EXT(ext_lattice)->add_mapview_colour(3 /*occupation view*/,x,y, rgb /* dark blue */);
      }
    }

    // 4 = Yield (black;white--yellow)
    if (Mapview == 4 || Mapview == -1){
      CYCLES(model_obj,c_patch,"Land_Patch"){
        x = int(POSITION_XS(c_patch));
        y = int(POSITION_YS(c_patch));
          double yield = VS(c_patch,"yield");
          yield = ColorGradient::normalise_value(0,1201,yield);
          int rgb = ColorGradient::LSD_rgb(yield,4 /*black; white-yellow*/ );
          P_EXT(ext_lattice)->add_mapview_colour(4 /*Yield*/,x,y,rgb);     // black where never anything grows.
      }
    }

    // 5 = apdsi (red-white-blue)
    if (Mapview == 5 || Mapview == -1){
      CYCLES(model_obj,c_patch,"Land_Patch"){
        x = int(POSITION_XS(c_patch));
        y = int(POSITION_YS(c_patch));
          double apdsi = VS(c_patch,"apdsi");
          apdsi = apdsi < -4 ? -4 : (apdsi > 4 ? 4 : apdsi); //bound in -4,4
          apdsi = ColorGradient::normalise_value(-4,4,apdsi);
          int rgb = ColorGradient::LSD_rgb(apdsi,2 );
          P_EXT(ext_lattice)->add_mapview_colour(5 /*apdsi*/,x,y,rgb);
      }
    }


    /*
          Update the mapviews and save a lattice to disk
    */



    int start_view = Mapview==-1 ? 0 : Mapview;
    int end_view = Mapview==-1? views: Mapview+1;

    //create a picture for each view.
    for (int i=start_view;i<end_view;i++){
      if (i!=0 || (i==0 && t==1) /*zones do not change*/  ) {
        P_EXT(ext_lattice)->change_mapview(i); //as provided in the config
        makePath("mapview/");
        char buff_prec_zeros[6];
        snprintf(buff_prec_zeros,sizeof(char)*6,"%0i",int(V("Year")));
        std::string buffer_n = "mapview/map_view_"+ P_EXT(ext_lattice)->view_names.at(i) + "_at_y_" +std::string(buff_prec_zeros);
        SAVE_LAT(buffer_n.c_str());
      }
    }

  }

  #endif

RESULT(0)

EQUATION("Init_Lattice")
#ifndef NOLATTICE
if (V("Update_Interval") == 0){
  PARAMETER;
  END_EQUATION(0);
}
  INIT_LAT( 0, y_rows,x_cols);
  ADDEXT(ext_lattice);
  P_EXT(ext_lattice)->init(x_cols,y_rows);


#endif

PARAMETER
RESULT(0)
