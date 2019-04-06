/* ========================================================================== */
/*                                                                            */
/*   ColourGradient.cpp                                                       */
/*								                                                            */
/*                                                                            */
/*                                                                            */
/* ========================================================================== */

#include <iostream>
#include <vector>



/*	Taken from:                                                              */
/*  http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients       */

namespace ColorGradient
{
  using namespace std;
  class ColorGradient
  {
  private:
    struct ColorPoint  // Internal class used to store colors at different points in the gradient.
    {
      float r,g,b;      // Red, green and blue values of our color.
      float val;        // Position of our color along the gradient (between 0 and 1).
      ColorPoint(float red, float green, float blue, float value)
        : r(red), g(green), b(blue), val(value) {}
    };
    vector<ColorPoint> color;      // An array of color points in ascending value.
    vector<ColorPoint> color_monochrome;   //White to black
    vector<ColorPoint> color_RedWhiteGreen; //Red to white to green
    vector<ColorPoint> color_RedWhiteBlue; //Red to white to blue
    vector<ColorPoint> color_RedWhiteRed; //Red to white to red
    vector<ColorPoint> color_BLACK_WhiteYellow; //White to yellow
    vector<ColorPoint> color_WhiteBlue; //White to blue
  
  public:
    //-- Default constructor:
    ColorGradient()  {  createDefaultHeatMapGradient(); createMonochromeGradient(); createRedWhiteGreenGradient(); createRedWhiteBlueGradient(); createRedWhiteRedGradient(); createBLACK_WhiteYellowGradient(); createWhiteBlueGradient();}
  
    //-- Inserts a new color point into its correct position:
    void addColorPoint(float red, float green, float blue, float value)
    {
      for(int i=0; i<color.size(); i++)  {
        if(value < color[i].val) {
          color.insert(color.begin()+i, ColorPoint(red,green,blue, value));
          return;  }}
      color.push_back(ColorPoint(red,green,blue, value));
    }
  
  
    //-- Inserts a new color point into its correct position:
    void clearGradient() { color.clear(); }
  
    //-- Places a 5 color heapmap gradient into the "color" vector:
    void createDefaultHeatMapGradient()
    {
      color.clear();
      color.push_back(ColorPoint(0, 0, 1,   0.0f));      // Blue.
      color.push_back(ColorPoint(0, 1, 1,   0.25f));     // Cyan.
      color.push_back(ColorPoint(0, 1, 0,   0.5f));      // Green.
      color.push_back(ColorPoint(1, 1, 0,   0.75f));     // Yellow.
      color.push_back(ColorPoint(1, 0, 0,   1.0f));      // Red.
    }
  
    //Monochrome heatmap:
    void createMonochromeGradient()
    {
      color_monochrome.clear();
      color_monochrome.push_back(ColorPoint(1, 1, 1,   0.0f));      // White.
      color_monochrome.push_back(ColorPoint(0, 0, 0,   1.0f));     // Black.
    }
  
    //Red White Green heatmap:
    void createRedWhiteGreenGradient()
    {
      color_RedWhiteGreen.clear();
      color_RedWhiteGreen.push_back(ColorPoint(1, 0, 0,   0.0f));      // Red.
      color_RedWhiteGreen.push_back(ColorPoint(1, 1, 1,   0.5f));      // White.
      color_RedWhiteGreen.push_back(ColorPoint(0, 1, 0,   1.0f));     // Green.
    }

    //Red White Blue heatmap:
    void createRedWhiteBlueGradient()
    {
      color_RedWhiteBlue.clear();
      color_RedWhiteBlue.push_back(ColorPoint(1, 0, 0,   0.0f));      // Red.
      color_RedWhiteBlue.push_back(ColorPoint(1, 1, 1,   0.5f));      // White.
      color_RedWhiteBlue.push_back(ColorPoint(0, 0, 1,   1.0f));     // Blue.
    }

  
    //Red White Red heatmap:
    void createRedWhiteRedGradient()
    {
      color_RedWhiteRed.clear();
      color_RedWhiteRed.push_back(ColorPoint(1, 0, 0,   0.0f));      // Red.
      color_RedWhiteRed.push_back(ColorPoint(1, 1, 1,   0.5f));      // White.
      color_RedWhiteRed.push_back(ColorPoint(1, 0, 0,   1.0f));     // Red.
    }

    //White Yellow heatmap:
    void createBLACK_WhiteYellowGradient()
    {
      color_BLACK_WhiteYellow.clear();
      color_BLACK_WhiteYellow.push_back(ColorPoint(0, 0, 0,   0.0f));      // Black.
      color_BLACK_WhiteYellow.push_back(ColorPoint(1, 1, 1,   0.01f));      // White.
//       color_BLACK_WhiteYellow.push_back(ColorPoint(1, .95, .7,   0.2f));     // Yellow
//       color_BLACK_WhiteYellow.push_back(ColorPoint(1, .89, .4,   0.5f));     // Yellow
//       color_BLACK_WhiteYellow.push_back(ColorPoint(1, .86, .2,   0.75f));     // Yellow.
      color_BLACK_WhiteYellow.push_back(ColorPoint(.965, .831, .192,   1.0f));     // Yellow.
    }

    //White Blue heatmap:
    void createWhiteBlueGradient()
    {
      color_WhiteBlue.clear();
      color_WhiteBlue.push_back(ColorPoint(1, 1, 1,   0.0f));      // White.
      color_WhiteBlue.push_back(ColorPoint(.25, .5, .9,   1.0f));     // Blue.
    }
  
    //-- Inputs a (value) between 0 and 1 and outputs the (red), (green) and (blue)
    //-- values representing that position in the gradient.
    void getColorAtValue(const float value, float &red, float &green, float &blue)
    {
      if(color.size()==0)
        return;
  
      for(int i=0; i<color.size(); i++)
      {
        ColorPoint &currC = color[i];
        if(value < currC.val)
        {
          ColorPoint &prevC  = color[ max(0,i-1) ];
          float valueDiff    = (prevC.val - currC.val);
          float fractBetween = (valueDiff==0) ? 0 : (value - currC.val) / valueDiff;
          red   = (prevC.r - currC.r)*fractBetween + currC.r;
          green = (prevC.g - currC.g)*fractBetween + currC.g;
          blue  = (prevC.b - currC.b)*fractBetween + currC.b;
          return;
        }
      }
      red   = color.back().r;
      green = color.back().g;
      blue  = color.back().b;
      return;
    }
  
    void getMonochromeColorAtValue(const float value, float &red, float &green, float &blue)
    {
      if(color_monochrome.size()==0)
        return;
  
      for(int i=0; i<color_monochrome.size(); i++)
      {
        ColorPoint &currC = color_monochrome[i];
        if(value < currC.val)
        {
          ColorPoint &prevC  = color_monochrome[ max(0,i-1) ];
          float valueDiff    = (prevC.val - currC.val);
          float fractBetween = (valueDiff==0) ? 0 : (value - currC.val) / valueDiff;
          red   = (prevC.r - currC.r)*fractBetween + currC.r;
          green = (prevC.g - currC.g)*fractBetween + currC.g;
          blue  = (prevC.b - currC.b)*fractBetween + currC.b;
          return;
        }
      }
      red   = color_monochrome.back().r;
      green = color_monochrome.back().g;
      blue  = color_monochrome.back().b;
      return;
    }
  
    void getRedWhiteGreenColorAtValue(const float value, float &red, float &green, float &blue)
    {
      if(color_RedWhiteGreen.size()==0)
        return;
  
      for(int i=0; i<color_RedWhiteGreen.size(); i++)
      {
        ColorPoint &currC = color_RedWhiteGreen[i];
        if(value < currC.val)
        {
          ColorPoint &prevC  = color_RedWhiteGreen[ max(0,i-1) ];
          float valueDiff    = (prevC.val - currC.val);
          float fractBetween = (valueDiff==0) ? 0 : (value - currC.val) / valueDiff;
          red   = (prevC.r - currC.r)*fractBetween + currC.r;
          green = (prevC.g - currC.g)*fractBetween + currC.g;
          blue  = (prevC.b - currC.b)*fractBetween + currC.b;
          return;
        }
      }
      red   = color_RedWhiteGreen.back().r;
      green = color_RedWhiteGreen.back().g;
      blue  = color_RedWhiteGreen.back().b;
      return;
    }

    void getRedWhiteBlueColorAtValue(const float value, float &red, float &green, float &blue)
    {
      if(color_RedWhiteBlue.size()==0)
        return;

      for(int i=0; i<color_RedWhiteBlue.size(); i++)
      {
        ColorPoint &currC = color_RedWhiteBlue[i];
        if(value < currC.val)
        {
          ColorPoint &prevC  = color_RedWhiteBlue[ max(0,i-1) ];
          float valueDiff    = (prevC.val - currC.val);
          float fractBetween = (valueDiff==0) ? 0 : (value - currC.val) / valueDiff;
          red   = (prevC.r - currC.r)*fractBetween + currC.r;
          green = (prevC.g - currC.g)*fractBetween + currC.g;
          blue  = (prevC.b - currC.b)*fractBetween + currC.b;
          return;
        }
      }
      red   = color_RedWhiteBlue.back().r;
      green = color_RedWhiteBlue.back().g;
      blue  = color_RedWhiteBlue.back().b;
      return;
    }
  
    void getRedWhiteRedColorAtValue(const float value, float &red, float &green, float &blue)
    {
      if(color_RedWhiteRed.size()==0)
        return;
  
      for(int i=0; i<color_RedWhiteRed.size(); i++)
      {
        ColorPoint &currC = color_RedWhiteRed[i];
        if(value < currC.val)
        {
          ColorPoint &prevC  = color_RedWhiteRed[ max(0,i-1) ];
          float valueDiff    = (prevC.val - currC.val);
          float fractBetween = (valueDiff==0) ? 0 : (value - currC.val) / valueDiff;
          red   = (prevC.r - currC.r)*fractBetween + currC.r;
          green = (prevC.g - currC.g)*fractBetween + currC.g;
          blue  = (prevC.b - currC.b)*fractBetween + currC.b;
          return;
        }
      }
      red   = color_RedWhiteRed.back().r;
      green = color_RedWhiteRed.back().g;
      blue  = color_RedWhiteRed.back().b;
      return;
    }

    void getBLACK_WhiteYellowColorAtValue(const float value, float &red, float &green, float &blue)
    {
      if(color_BLACK_WhiteYellow.size()==0)
        return;

      for(int i=0; i<color_BLACK_WhiteYellow.size(); i++)
      {
        ColorPoint &currC = color_BLACK_WhiteYellow[i];
        if(value < currC.val)
        {
          ColorPoint &prevC  = color_BLACK_WhiteYellow[ max(0,i-1) ];
          float valueDiff    = (prevC.val - currC.val);
          float fractBetween = (valueDiff==0) ? 0 : (value - currC.val) / valueDiff;
          red   = (prevC.r - currC.r)*fractBetween + currC.r;
          green = (prevC.g - currC.g)*fractBetween + currC.g;
          blue  = (prevC.b - currC.b)*fractBetween + currC.b;
          return;
        }
      }
      red   = color_BLACK_WhiteYellow.back().r;
      green = color_BLACK_WhiteYellow.back().g;
      blue  = color_BLACK_WhiteYellow.back().b;
      return;
    }

    void getWhiteBlueColorAtValue(const float value, float &red, float &green, float &blue)
    {
      if(color_WhiteBlue.size()==0)
        return;

      for(int i=0; i<color_WhiteBlue.size(); i++)
      {
        ColorPoint &currC = color_WhiteBlue[i];
        if(value < currC.val)
        {
          ColorPoint &prevC  = color_WhiteBlue[ max(0,i-1) ];
          float valueDiff    = (prevC.val - currC.val);
          float fractBetween = (valueDiff==0) ? 0 : (value - currC.val) / valueDiff;
          red   = (prevC.r - currC.r)*fractBetween + currC.r;
          green = (prevC.g - currC.g)*fractBetween + currC.g;
          blue  = (prevC.b - currC.b)*fractBetween + currC.b;
          return;
        }
      }
      red   = color_WhiteBlue.back().r;
      green = color_WhiteBlue.back().g;
      blue  = color_WhiteBlue.back().b;
      return;
    }
  
  };
  
  /* Create default Heatmap(s) with startup */
  ColorGradient heatMapGradient;
  
  void init_colorgradient(){
  /* Initialise the colour gradients */
  	heatMapGradient.createDefaultHeatMapGradient();
    heatMapGradient.createMonochromeGradient();
    heatMapGradient.createRedWhiteGreenGradient();
    heatMapGradient.createRedWhiteBlueGradient();
    heatMapGradient.createRedWhiteRedGradient();
    heatMapGradient.createBLACK_WhiteYellowGradient();
    heatMapGradient.createWhiteBlueGradient();
  }
  
  double normalise_value(double v_min, double v_max, double v){
  /* Normalise a value v in [v_min,v_max] to v in [0,1] */
  	if (v_max > v_min){
  		return (v - v_min)/(v_max-v_min);
  	}
  	else
  		return -1; //Error flag
  
  }
  
  int createRGB(int rE, int gE, int bE)
  {
  	  //sprintf(msg,"\t and as int: r%i,g%i,b%i",rE,gE,bE);
  		//plog(msg);
      //unsigned long convert =  ((rE & 0xff) << 16) + ((gE & 0xff) << 8) + (bE & 0xff);
      unsigned long convert = rE * 0x10000 + gE * 0x100 + bE;
  		//sprintf(msg," the li is %i as double %g\n",convert,(double)(convert));
  		//plog(msg);
      return ( (int)(convert)  );
  }
  
  
  void update_lattice_RGB(double line, double col, int v_r, int v_g, int v_b){
  	update_lattice(line, col, -createRGB(v_r,v_g,v_b));
  }
  
  void LSD_lattice_update(double line, double col, double yourGradientValue){
    /* red green blue */
  	float rC,gC,bC;
  	heatMapGradient.getColorAtValue(yourGradientValue, rC,gC,bC);
    update_lattice_RGB(line,col,(int)(255*rC),(int)(255*gC),(int)(255*bC));
  }
  
  void LSD_lattice_update_Monochrome(double line, double col, double yourGradientValue){
  	float rC,gC,bC;
  	heatMapGradient.getMonochromeColorAtValue(yourGradientValue, rC,gC,bC);
    update_lattice_RGB(line,col,(int)(255*rC),(int)(255*gC),(int)(255*bC));
  }
  
  void LSD_lattice_update_RedWhiteGreen(double line, double col, double yourGradientValue){
  	float rC,gC,bC;
  	heatMapGradient.getRedWhiteGreenColorAtValue(yourGradientValue, rC,gC,bC);
    update_lattice_RGB(line,col,(int)(255*rC),(int)(255*gC),(int)(255*bC));
  }
  
  void LSD_lattice_update_RedWhiteRed(double line, double col, double yourGradientValue){
  	float rC,gC,bC;
  	heatMapGradient.getRedWhiteRedColorAtValue(yourGradientValue, rC,gC,bC);
    update_lattice_RGB(line,col,(int)(255*rC),(int)(255*gC),(int)(255*bC));
  }

  void LSD_lattice_update_BLACK_WhiteYellow(double line, double col, double yourGradientValue){
  	float rC,gC,bC;
  	heatMapGradient.getBLACK_WhiteYellowColorAtValue(yourGradientValue, rC,gC,bC);
    update_lattice_RGB(line,col,(int)(255*rC),(int)(255*gC),(int)(255*bC));
  }

  void LSD_lattice_update_WhiteBlue(double line, double col, double yourGradientValue){
  	float rC,gC,bC;
  	heatMapGradient.getWhiteBlueColorAtValue(yourGradientValue, rC,gC,bC);
    update_lattice_RGB(line,col,(int)(255*rC),(int)(255*gC),(int)(255*bC));
  }

  int LSD_rgb(float value, int grad_choice){
    float r,g,b;
    if (grad_choice == -1) {
      heatMapGradient.getColorAtValue(value,r,g,b);  //default blue-green-red
    } else if (grad_choice == 0) {
      heatMapGradient.getMonochromeColorAtValue(value,r,g,b);
    } else if (grad_choice == 1) {
      heatMapGradient.getRedWhiteGreenColorAtValue(value,r,g,b);
    } else if (grad_choice == 2) {
      heatMapGradient.getRedWhiteBlueColorAtValue(value,r,g,b);
    } else if (grad_choice == 3) {
      heatMapGradient.getRedWhiteRedColorAtValue(value,r,g,b);
    } else if (grad_choice == 4) {
      heatMapGradient.getBLACK_WhiteYellowColorAtValue(value,r,g,b);
    } else if (grad_choice == 5) {
      heatMapGradient.getWhiteBlueColorAtValue(value,r,g,b);
    }
    return -createRGB(int(255*r),int(255*g),int(255*b)); //return rgb for LSD
  }

}
