#include "fun_head.h"

MODELBEGIN



EQUATION("X")
/*
A variable moving as a random walk
*/

v[0]=V("RandomEvent");
v[1]=VL("X",1);

v[2]=v[0]+v[1];
RESULT(v[2] )


EQUATION("RandomEvent")
/*
A random event
*/
v[0]=V("minX");
v[1]=V("maxX");
v[3]=UNIFORM(v[0],v[1]);
RESULT(v[3] )








MODELEND




void close_sim(void)
{

}


