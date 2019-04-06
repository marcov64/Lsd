/* ========================================================================== */
/*                                                                            */
/*   PAJEK utilities - produce pajek timeseries files                         */
/*                                                                            */
/* ========================================================================== */


#include "external/Pajek/PajekFromCpp_head.h"
//for pajek svgs
double const svg_margin = 0.15;
double const base_svg_size = 0.2;
double const base_svg_lw = 2.0;

//for the layout inside the patch. pos: 1=low left, 2=lr, 3=ur, 4=ul, 5=center
inline double pos_x(int pos, double x_low)
{
    switch (pos) {
        case 1:
            return x_low + svg_margin;
        case 2:
            return x_low + 1.0 - svg_margin;
        case 3:
            return x_low + svg_margin;
        case 4:
            return x_low + 1.0 - svg_margin;
        case 5:
            return x_low + 0.5;
        default:
            return x_low;
    }
}
inline double pos_y(int pos, double y_low)
{
    switch (pos) {
        case 1:
        case 2:
            return y_low + svg_margin;
        case 3:
        case 4:
            return y_low + 1.0 - svg_margin;
        case 5:
            return y_low + 0.5;
        default:
            return y_low;
    }
}
inline const std::string MapColor (int map_color)
{
    switch (map_color) {
        case 0:
            return "Black";
        case 1:
            return "Red";
        case 3:
            return "Yellow";
        case 4:
            return "Pink";
        case 5:
            return "RoyalBlue";
        case 7:
            return "Gray";
        case 1000:
        default:
            return "White";
    }
}

inline double MaizeSize (double min_y, double yield)
{
    return  yield > min_y ? ((yield - min_y) / (1201.0 - min_y)) / 2 + .5 : 0.5;
}



