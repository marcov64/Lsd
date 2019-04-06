/* ========================================================================== */
/*                                                                            */
/*   LATTICE utilities - produce the usual AA figures as in NetLogo           */
/*                                                                            */
/* ========================================================================== */

#include <vector>
#include <string>
#include "external/CreateDir/CreateDir.h" //allow creating directories
#include "external/ColourGradient/ColourGradient.cpp" //Colour Gradient
class ext_lattice{
  public:
  int mapview_view = -1;
  std::vector <std::vector <std::vector < int > > > mapview_colours;
  int const views = 6;
  std::vector <std::string> view_names = {"Zones","Watersources","Hydro","Occupation","Yield","apdsi"};
  /* vecs:
  0 = Zones (special)
  1 = Watersources (blue/white)
  2 = Hydro (green/white)
  3 = Occupation (blue--green--red)
  4 = Yield (black;white--yellow)
  5 = apdsi (red-white-blue)
  */

  int x_cols = -1;
  int y_rows = -1;

  void init(int x_cols, int y_rows);
  void add_mapview_colour(int view, int x, int y, int rgb);
  void change_mapview(int view);
  void update_mapview_colour(int view, int x, int y, int rgb);

};


  void ext_lattice::init(int _x_cols, int _y_rows){
    /* Initialise the structure */
    mapview_colours.clear();
    y_rows=_y_rows;
    x_cols=_x_cols;
    ColorGradient::init_colorgradient(); //for views 3-5
    std::vector<int> y_colours(y_rows);
    std::vector <std::vector < int > > x_colours(x_cols,y_colours);
    for (int i=0; i<views; i++){
      mapview_colours.emplace_back(x_colours);
    }
  }
  void ext_lattice::add_mapview_colour(int view, int x, int y, int rgb){
    //only save to view
    mapview_colours.at(view).at(x).at(y)=rgb;
  }

  void ext_lattice::change_mapview(int view){
    //switch to other view, completely updating the map
    if (mapview_view != view){
      mapview_view = view;
      for (int x=0;x<x_cols;x++){
        for (int y=0;y<y_rows;y++){
          WRITE_LAT(y_rows-y,x+1,mapview_colours.at(view).at(x).at(y));   //top-down left-right parsing mapped to bottom-up left-right parsing
        }
      }
    }
  }
  void ext_lattice::update_mapview_colour(int view, int x, int y, int rgb){
    //Save to view and update lattice
    add_mapview_colour(view,x,y,rgb);
    if (mapview_view != view) {
      change_mapview(view);
    } else {
      WRITE_LAT(y_rows-y,x+1,rgb);   //top-down left-right parsing
    }
  }

// };

#ifndef NOLATTICE //make sure that in no_window mode no lattice data is calculated, etc.
  #ifdef NO_WINDOW
    #define NOLATTICE
  #endif
#endif


