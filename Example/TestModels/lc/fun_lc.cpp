#include "fun_head.h"

MODELBEGIN




EQUATION("X")
/*
Logistic equation:
X(t)=m*X(t-1)*(1-X(t-1))

The series provides values in the range [0,1] for m in the range [0,4].
However, for m > 2 the series cycles or goes chaotic.
*/
v[0]=VL("X",1);
v[1]=V("m");

RESULT(v[1]*v[0]*(1-v[0]))





MODELEND




void close_sim(void)
{

}


