#define NO_POINTER_INIT							// disable pointer checking

#include "fun_head.h"

MODELBEGIN

const char *chstr;

EQUATION("Init")
/* Initialise the model. Generate the cells and mark alive ones randomly. */

INIT_SPACE_PATCH_WRAP("Cell", V("ncol"), V("nrow"), V("wrapping") ); //Initialise the space
ADD_TO_SPACE_XY_WHERE(0,0,SEARCH("Cell")); //Ease access, adding Lattice object to space.

SET_GIS_DISTANCE_TYPE('c'); //Chebyshev metric aka moore neighbourhood

// The window is sized at most pixel * pixel
int pcol,prow;
pcol=prow=(int)V("WPixel");
if (V("ncol")>V("nrow")) {
  prow = V("nrow")/V("ncol") * prow; //scale down
} if (V("ncol")<V("nrow")) {
  pcol = V("ncol")/V("nrow") * pcol; //scale down
}

INIT_LAT_GISS ( SEARCH("Cell"), 0, pcol, prow ); //Initialise the graphical lattice (black)

int active_cells = V("PercActive") *  V("ncol") * V("nrow") ;

if (active_cells > 0) {
  //Cycle randomly through cells and active the first active_cells cells.
  RCYCLE_GIS(cur,"Cell"){
    WRITELS(cur,"State",1, t-1);
    active_cells--;
    if (active_cells == 0){
      break; //stop when the specified number of cells have been made alive.
    }
  }

  //otherwise load data from file
} else {
//   cmd("set fname [tk_getOpenFile -title \"Select file with initial active cells and values\"]");
//   chstr= (char *) Tcl_GetVar(inter, "fname",0);
//   LOAD_DATA_GIS(chstr, "Cell", "State"); //Create if necessary  (here not) and activate them

LOAD_DATA_GIS(SELECT_FILE("Select file with initial active cells and values"),"Cell","State");

  //in this case we need to manually update the lattice.
  CYCLE(cur,"Cell"){
    if (VLS(cur,"State",1)==1){
      WRITE_LAT_GISS(cur,1);
    }
  }

}
CYCLE(cur,"Cell"){
    SET_LAT_PRIORITYS(cur,0); //Make them visible
    SET_LAT_COLORS(cur,VLS(cur,"State",1));
}
PARAMETER
RESULT(0.0)


EQUATION("State")
/*
Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
Any live cell with two or three live neighbours lives on to the next generation.
Any live cell with more than three live neighbours dies, as if by overpopulation.
Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.

The neighbourhood is the moore neighbourhood (8 cells)

In this version the updating is "simultaneous"
*/

int neighbours_alive = 0;
FCYCLE_NEIGHBOUR( cur, "Cell", 1.0 ){
  neighbours_alive += VLS(cur,"State",1); //0 or 1 dead or alive
}

double state = CURRENT; //check if cell is just alive

if (1.0 == state) {
  if ( neighbours_alive < 2 || neighbours_alive > 3 ){
    state = 0.0; //die
  }
} else {
  if (neighbours_alive == 3.0 ){
    state = 1.0; //live
  }
}

if (CURRENT != state){
  SET_LAT_COLOR(state); //update lattice
}

RESULT( state )

EQUATION("SlowDown")
/*
Equation wasting time to slow down the graph, if not in fast mode
*/
if (false == fast)
  SLEEP((int) V("TimeSleep") );

RESULT(1)



MODELEND




void close_sim(void)
{

}
