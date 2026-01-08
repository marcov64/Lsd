#include "lsd_head.h"

MODELBEGIN


EQUATION( "E" )
// Number of people entering region in period

RESULT( poisson( V( "alpha" ) ) )


EQUATION( "X" )
// Number of people exiting region in period

RESULT( poisson( V( "beta" ) * VL( "x", 1 ) ) )


EQUATION( "x" )
// Number of people in region in period

RESULT( CURRENT + V( "E" ) - V( "X" ) )


EQUATION( "t" )
// Time for data export

RESULT( T )


EQUATION( "init_region" )
// Set the initial number of people in region

WRITEL( "x", norm( V( "x0" ), sqrt( V( "x0_var" ) ) ), 0 );
PARAMETER;

RESULT( 1 )


EQUATION( "init" )
// Set the initial parameters

WRITE( "alpha", norm( V( "alpha"), sqrt( V( "alpha_var" ) ) ) );
WRITE( "beta", norm( V( "beta"), sqrt( V( "beta_var" ) ) ) );
PARAMETER;

RESULT( 1 )


MODELEND


CLOSEBEGIN
CLOSEEND
