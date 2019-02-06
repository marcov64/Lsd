//a simple test program
#include "PajekFromCpp.cpp"
#include "PajekFromCpp_macro.h"
#include <random>
//for the layout inside the patch. pos: 1=low left, 2=lr, 3=ur, 4=ul, 5=center
  double const svg_margin = 0.15;
  double const base_svg_size = 0.2;
  double const base_svg_lw = 2.0;
inline double pos_x(int pos, double x_low)
{
  switch (pos) {
    case 1: return x_low + svg_margin;
    case 2: return x_low + 1.0 - svg_margin;
    case 3: return x_low + svg_margin;
    case 4: return x_low + 1.0 - svg_margin;
    case 5: return x_low + 0.5;
    default: return x_low;
  }
}
inline double pos_y(int pos, double y_low)
{
  switch (pos) {
    case 1:
    case 2: return y_low + svg_margin;
    case 3:
    case 4: return y_low + 1.0 - svg_margin;
    case 5: return y_low + 0.5;
    default: return y_low;
  }
}
int main(){
  std::random_device rd; // to seed mt
  std::mt19937 prng(rd()); //the mt prng
  std::uniform_int_distribution<int> distr(0,2);
  std::uniform_real_distribution<double> uniform(0,1);
  std::uniform_int_distribution<int> distr_x(0,7);
  std::uniform_int_distribution<int> distr_y(0,5);
  std::uniform_int_distribution<int> distr_patch(0,48);

    //simple test that should work
  PAJ_MAKE_AVAILABLE
  PAJ_INIT_ANIM("Network","TestSet",1,"Test Set")
  for (int t = 1; t<5; t++){
    PAJ_STATIC("Network","TestSet",1,"Test Set") //create static info allongside
    int pid = 0;
    for (double x=0; x<8; x++) {
      for (double y=0; y<6; y++) {
        pid++;
        PAJ_ADD_V_C(t,pid,"Patch",1,x,y,"box",10,10,distr(prng)<1?"Black":(distr(prng)<2?"Red":"Blue"))

        PAJ_S_ADD_V_C(t,pid,"Patch",1,x,y,"box",10,10,distr(prng)<1?"Black":(distr(prng)<2?"Red":"Blue"))
      }
    }
    PAJ_ADD_V_C(t,1,"Dog",1,distr_x(prng),distr_y(prng),"triangle",1,1,"Green")
    PAJ_ADD_V_C(t,1,"Person",1,distr_x(prng),distr_y(prng),"ellipse",2,2,"Gray")
    PAJ_ADD_A(t,1,"Dog",distr_patch(prng),"Patch",1,"Arc")
    PAJ_ADD_A(t,1,"Person",1,"Dog",1,"Edge")

    PAJ_S_ADD_V_C(t,1,"Dog",1,distr_x(prng),distr_y(prng),"triangle",1,1,"Green")
    PAJ_S_ADD_V_C(t,1,"Person",1,distr_x(prng),distr_y(prng),"ellipse",2,2,"Gray")
    PAJ_S_ADD_A(t,1,"Dog",distr_patch(prng),"Patch",1,"Arc")
    PAJ_S_ADD_A(t,1,"Person",1,"Dog",1,"Edge")
    PAJ_S_SAVE
  }

  PAJ_SAVE

  {
          PAJ_STATIC("Networks","AA_Legend",1,"Artificial Anasazi - Replication of Janssen 2009 - Legend")
          PAJ_S_ADD_V_CL(0,0,"MaizeZone",1,0,0,"diamond",0.5,1.0,"Dandelion","Maize zone (suitable)")
          PAJ_S_ADD_V_CL(0,1,"MaizeZone",1,0,0,"diamond",0.5,0.5,"RedOrange","Maize zone (not sustainable)")
          PAJ_S_ADD_V_CL(0,2,"Floodplain",1,pos_x(5,1),pos_x(5,2),"box",base_svg_size*3,base_svg_size*3,"MidnightBlue","Floodplain area (hydro>0)")
          PAJ_S_ADD_V_CL(0,3,"Waterpoint",1,pos_x(2,2),pos_y(2,3),"triangle",base_svg_size,base_svg_size,"Blue","Waterpoint (active)")
          PAJ_S_ADD_V_CL(0,4,"Waterpoint",1,pos_x(2,2),pos_y(2,4),"triangle",base_svg_size,base_svg_size,"Gray45","Waterpoint (not active)")
          PAJ_S_ADD_V_CL(0,5,"Settlement",1,pos_x(5,3),pos_y(5,5),"house",base_svg_size*2,base_svg_size*2,"Red","Settlement (1 hh)")
          PAJ_S_ADD_V_CL(0,6,"Settlement",10,pos_x(5,3),pos_y(5,6),"house",base_svg_size*(1+sqrt(10)),base_svg_size*(1+sqrt(10)),"Red","Settlement (10 hh)")
          PAJ_S_ADD_V_CL(0,7,"Settlement",10,pos_x(5,3),pos_y(5,7),"house",base_svg_size*(1+sqrt(50)),base_svg_size*(1+sqrt(50)),"Red","Settlement (50 hh)")
          PAJ_S_ADD_V_CL(0,8,"Farm",1,pos_x(5,8),pos_y(5,4),"house",base_svg_size*3,base_svg_size*3,"PineGreen", "close Farm")
          PAJ_S_ADD_V_CL(0,9,"Farm",1,pos_x(5,9),pos_y(5,4),"house",base_svg_size*3,base_svg_size*3,"PineGreen", "distant Farm")
          PAJ_S_ADD_A_C(0,5,"Settlement",3,"Waterpoint",-17.0,"s-w out of distance",base_svg_lw,"Blue")
          PAJ_S_ADD_A_C(0,5,"Settlement",8,"Farm",5.0,"s-f in distance",base_svg_lw,"PineGreen")
          PAJ_S_ADD_A_C(0,8,"Farm",3,"Waterpoint",16.0,"f-w just in distance",base_svg_lw,"RedOrange")
          PAJ_S_ADD_A_C(0,7,"Settlement",4,"Waterpoint",8.0,"s-w in distance",base_svg_lw,"Blue")
          PAJ_S_ADD_A_C(0,7,"Settlement",9,"Farm",-22.0,"s-f out of distance",base_svg_lw,"PineGreen")
          PAJ_S_ADD_A_C(0,9,"Farm",4,"Waterpoint",8.0,"f-w in distance",base_svg_lw,"RedOrange")
          PAJ_S_SAVE
  }
}
