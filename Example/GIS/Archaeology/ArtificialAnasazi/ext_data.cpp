/* ========================================================================== */
/*                                                                            */
/*   ext_data.cpp                                                            */
/*   load the data from the files.                                            */
/* ========================================================================== */

#include <vector>
#include <algorithm>

class ext_environment;
class ext_population;

class ext_environment
{
public:
  object* up=NULL;
  int patches = 0;
  int years = 0;
  int toggle_print = 30;
  int max_x;
  int max_y;
  #ifdef VALIDATION
  std::vector <std::vector <double>> validate_hydro, validate_apdsi, validate_water, validate_yield;
  int errs_hydro, errs_apdsi, errs_water, errs_yield;
  double sum_diffs_hydro,sum_diffs_apdsi,sum_diffs_water,sum_diffs_yield;
  int val_time;

  int validate(int time, int x, int y, double hydro, double water, double apdsi, double yield)
  {
    if (time > validate_yield.size() ){
      PLOG("\n Error: The simulation time horizon is longer than the validation data.");
      return -1; //no check possible.
    }
    int error = 0;
    if (val_time != time) {
      val_time = time;
      errs_yield=errs_water=errs_apdsi=errs_hydro=0; //reset
      sum_diffs_hydro=sum_diffs_apdsi=sum_diffs_water=sum_diffs_yield=0;
    }
    int year = VS(root,"Year");
    object *cur_obj = SEARCH_POSITION_XYS(SEARCHS(root,"Land_Patch"),"Land_Patch",x,y);
    int zone_ = VS(cur_obj,"land_type");
    std::string zone_name  = "";
    switch (zone_){
          case  0:  /*General, Yield_2, black*/
                      zone_name = "General";
                      break;

            case  10: /*North, Yield_1, red*/
                      zone_name = "North Valley";
                      break;

            case  15:  /*North Dunes, Sand_dune, white*/
                      zone_name = "North Dunes";
                      break;

            case  20:  /*Mid, x<=74?Yield_1:Yield_2, gray*/
                      zone_name = "Mid Valley";
                      break;

            case  25:  /*Mid Dunes, Sand_dune, white*/
                      zone_name = "Mid Dunes";
                      break;

            case  30:  /*Natural, No_Yield, yellow*/
                      zone_name = "Natural";
                      break;

            case  40:  /*Uplands, Yield_3, blue*/
                      zone_name = "Uplands";
                      break;

            case  50:  /*Kinbiko, Yield_1, pink*/
                      zone_name = "Kinbiko";
                      break;

            case  60:  /*Empty, Empty, white*/
                      zone_name = "Empty";
                      break;
    }

    int xy = (x*(max_y+1))+(max_y-y); //position in row-data-file x runs from 0 to 79.

    if (false) { //Tested and OK!
      int test_xy = -1;
      int t_x=0;
      int t_y=max_y;
      bool found=false;
      while(!found && t_x<max_x+1){
        while(t_y>=0){
          test_xy++;
          if (t_x == x && t_y == y){
            found=true;
            break;
          }
          t_y--;
        }
        t_y = max_y;
        t_x++;
      }
      if (test_xy != xy){
        PLOG("\n Error: Check counting in ext_environment::validate! xy=%i,test_xy=%i for x=%i, y=%i",xy,test_xy,x,y);
        return -1;
      }
    }
    if (xy > validate_yield.at(time).size()){
      PLOG("\nError: requested patch %i,%i at position %i, which is outside the size %i",x,y,xy,validate_yield.at(time).size());
      return -1;
    }

    if ( yield != validate_yield.at(time).at(xy) ){
      sum_diffs_yield+=(yield-validate_yield.at(time).at(xy));
      error++;
      errs_yield++;
      if (errs_yield < toggle_print){
        PLOG("\nError (validation): At time %i (year %i)  the yield at patch %i,%i (zone %s) should be %g but is %g",time+1,year,x,y,zone_name.c_str(),validate_yield.at(time).at(xy),yield);
      }
    }
    if ( hydro != validate_hydro.at(time).at(xy) ){
      sum_diffs_hydro+=(hydro-validate_hydro.at(time).at(xy));
      error++;
      errs_hydro++;
      if (errs_hydro < toggle_print){
        PLOG("\nError (validation): At time %i (year %i)  the hydro at patch %i,%i (zone %s) should be %g but is %g",time+1,year,x,y,zone_name.c_str(),validate_hydro.at(time).at(xy),hydro);
      }
    }
    if ( water != validate_water.at(time).at(xy) ){
      sum_diffs_water+=(water-validate_water.at(time).at(xy));
      error++;
      errs_water++;
      if ( errs_water < toggle_print){
        PLOG("\nError (validation): At time %i (year %i)  the water at patch %i,%i (zone %s) should be %g but is %g",time+1,year,x,y,zone_name.c_str(),validate_water.at(time).at(xy),water);
      }
    }
    if ( apdsi != validate_apdsi.at(time).at(xy) ){
      sum_diffs_apdsi+=(apdsi-validate_apdsi.at(time).at(xy));
      error++;
      errs_apdsi++;
      if (errs_apdsi < toggle_print){
        PLOG("\nError (validation): At time %i (year %i)  the apdsi at patch %i,%i (zone %s) should be %g but is %g",time+1,year,x,y,zone_name.c_str(),validate_apdsi.at(time).at(xy),apdsi);
      }
    }

    return error;
  }

  bool init_validate(int _years, int x_cols, int y_rows)
  {
    years = _years;
    val_time = -1;
    patches = x_cols*y_rows;
    max_x = x_cols-1;
    max_y = y_rows-1;
    validate_hydro.clear();
    validate_apdsi.clear();
    validate_water.clear();
    validate_yield.clear();

    std::ifstream  data_hydro("data/hydro_janssen.txt");
    if (!data_hydro){
      PLOG("\nError: One of the file 'data/hydro_janssen.txt' is missing'.");
      return false;
    } else {
      std::string s;
      std::vector <double> temp;
      int count_y = 0;
      int count_p = 0;
      while (data_hydro >> s){ //read by entry
        temp.push_back(std::stod(s));
        count_p++;
        if (count_p == patches){
          count_p = 0;
          count_y++;
          validate_hydro.push_back(temp);
          temp.clear();
        }
      }
      if (count_p != 0) {
        PLOG("\nError: After %i years of %i processed, the file 'data/hydro_janssen.txt' ends at patch %i of %i",count_y,years,count_p,patches);
        return false;
      }
      if (count_y != years) {
        PLOG("\nError: The validation data: hydro consists of %i entries vs. user supplied %i years",count_y,years);
        return false;
      }
    }
    data_hydro.close();

    std::ifstream  data_apdsi("data/apdsi_janssen.txt");
    if (!data_apdsi){
      PLOG("\nError: One of the file 'data/apdsi_janssen.txt' is missing'.");
      return false;
    } else {
      std::string s;
      std::vector <double> temp;
      int count_y = 0;
      int count_p = 0;
      while (data_apdsi >> s){ //read by entry
        temp.push_back(std::stod(s));
        count_p++;
        if (count_p == patches){
          count_p = 0;
          count_y++;
          validate_apdsi.push_back(temp);
          temp.clear();
        }
      }
      if (count_p != 0) {
        PLOG("\nError: After %i years of %i processed, the file 'data/apdsi_janssen.txt' ends at patch %i of %i",count_y,years,count_p,patches);
        return false;
      }
      if (count_y != years) {
        PLOG("\nError: The validation data: apdsi consists of %i entries vs. user supplied %i years",count_y,years);
        return false;
      }
    }
    data_apdsi.close();

    std::ifstream  data_water("data/water_janssen.txt");
    if (!data_water){
      PLOG("\nError: One of the file 'data/water_janssen.txt' is missing'.");
      return false;
    } else {
      std::string s;
      std::vector <double> temp;
      int count_y = 0;
      int count_p = 0;
      while (data_water >> s){ //read by entry
        temp.push_back(std::stod(s));
        count_p++;
        if (count_p == patches){
          count_p = 0;
          count_y++;
          validate_water.push_back(temp);
          temp.clear();
        }
      }
      if (count_p != 0) {
        PLOG("\nError: After %i years of %i processed, the file 'data/water_janssen.txt' ends at patch %i of %i",count_y,years,count_p,patches);
        return false;
      }
      if (count_y != years) {
        PLOG("\nError: The validation data: water consists of %i entries vs. user supplied %i years",count_y,years);
        return false;
      }
    }
    data_water.close();

    std::ifstream  data_yield("data/yield_janssen.txt");
    if (!data_yield){
      PLOG("\nError: One of the file 'data/yield_janssen.txt' is missing'.");
      return false;
    } else {
      std::string s;
      std::vector <double> temp;
      int count_y = 0;
      int count_p = 0;
      while (data_yield >> s){ //read by entry
        temp.push_back(std::stod(s));  //stoi also fine
        count_p++;
        if (count_p == patches){
          count_p = 0;
          count_y++;
          validate_yield.push_back(temp);
          temp.clear();
        }
      }
      if (count_p != 0) {
        PLOG("\nError: After %i years of %i processed, the file data/yield_janssen.txt' ends at patch %i of %i",count_y,years,count_p,patches);
        return false;
      }
      if (count_y != years) {
        PLOG("\nError: The validation data: yield consists of %i entries vs. user supplied %i years",count_y,years);
        return false;
      }
    }
    data_yield.close();

    return true;
  } //init_validate
  #endif

};

class ext_population
{
  public:
  object* up=NULL;
  int years = 0;
  std::vector <double> hist_pop;

  bool init_hist_pop(int _years) {
    years = _years;
    hist_pop.clear();
    std::ifstream  data_hist("data/histPop_janssen.txt");
    if (!data_hist){
      PLOG("\nError: One of the file 'data/histPop_janssen.txt' is missing'.");
      return false;
    } else {
      int count = 0;
      std::string s_h;
      while (data_hist >> s_h){ //read by entry
        hist_pop.push_back(std::stoi(s_h));
        count ++;
      }
      if (count != years) {
        PLOG("\nError: The historical population data consists of %i entries vs. user supplied %i years",count,years);
        return false;
      }
    }
    data_hist.close();
    return true;
  }

};