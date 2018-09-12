//#define EIGENLIB			// uncomment to use Eigen linear algebra library

#include "fun_head_fast.h"
#include "gis.cpp"

// do not add Equations in this area



MODELBEGIN

// insert your equations here, between the MODELBEGIN and MODELEND words


EQUATION("Init_GIS")
cur = SEARCH("Patch"); //Get first object of "Patch" type. This will be our fixed landscape.

if (p->init_gis_regularGrid("Patch",10,20) ){
	PLOG("\nInitialised the Grid.");
}	else {
	PLOG("\nError");
}
PARAMETER
RESULT(0.0)










MODELEND


// do not add Equations in this area


void close_sim( void )
{
	// close simulation special commands go here
}
