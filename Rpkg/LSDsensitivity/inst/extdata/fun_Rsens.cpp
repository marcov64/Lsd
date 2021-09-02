//#define EIGENLIB			// uncomment to use Eigen linear algebra library

#include "fun_head_fast.h"

// do not add Equations in this area

MODELBEGIN

// insert your equations here, ONLY between the MODELBEGIN and MODELEND words

EQUATION("var1")
RESULT( V( "par1" ) * RND + T * RND )

EQUATION("var2")
RESULT( V( "par2" ) * log( 1 + V( "var1" ) + RND + T * RND ) )

EQUATION("var3")
RESULT( 0.9 * CURRENT + sqrt( ( V( "var1" ) * V( "var2" ) / ( T * T / 4 ) * V( "par3" ) * RND ) ) )

MODELEND

// do not add Equations in this area

void close_sim( void )
{
	// close simulation special commands go here
}
