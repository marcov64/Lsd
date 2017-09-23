#include "fun_head.h"



MODELBEGIN


EQUATION("Fitness")
/*
Fitness, computed as a random walk of DeltaFit. Two alternative models
are considered. In the non-cumulative model (Parameter CumulativeModel=0)
the fitness is computed as:
Fitness[t]=Fitness[t-1]+RndUniform(Fitness[t-1]-DeltaFit, Fitness[t-1]+DeltaFit)

In the alternative model (Parameter CumulativeModel=1) the maximum range is
proportional to the shares:
Fitness[t]=
=Fitness[t-1]+RndUniform(Fitness[t-1]-ms[t-1]*DeltaFit, Fitness[t-1]+ms[t-1]*DeltaFit)
*/


v[0]=V("DeltaFit");
v[1]=VL("Fitness",1);
v[2]=RND;
v[5]=V("Drift");
v[3]=V("CumulativeModel");
if(v[3]==0)
  v[6]=v[1]+v[5]+(v[2]-0.5)*2*v[0]; //Non-cumulative model
else
 {v[4]=VL("ms",1);
  v[6]=v[1]+v[5]*v[4]+(v[2]-0.5)*2*v[0]; //Cumulative model
 }
RESULT(v[6])




EQUATION("AvFit")
/*
Average fitness. Note that the average uses the
Num[t-1] as weights, since this Variable is part of the
computation of Num[t].
*/
v[3]=0,v[2]=0;
CYCLE(cur,"Species")
 {
  v[0]=VLS(cur,"Num",1);
  v[1]=VS(cur,"Fitness");
  v[2]+=v[0]*v[1];
  v[3]+=v[0];
 }

RESULT(v[2]/v[3]);

EQUATION( "ms")

/*
Shares of population for the Species
*/
v[0]=V("Num");
v[1]=V("TotNum");
RESULT(v[0]/v[1]);

EQUATION( "Num")
/*
Number of individual present for each species. It is computed as:
Num[t]=Num[t-1](1+Alpha(Fitness[t]-AvFitness[t])/AvFitness[t])

That is, the number of individuals increases if the species has an above average fitness and decreases otherwise.

The speed of change is set by Alpha.
*/
v[0]=V("Fitness"); //Fitness
v[1]=VL("Num",1);
v[2]=V("Alpha");
v[3]=V("AvFit");

RESULT(round(v[1]+v[1]*v[2]*(v[0]-v[3])/v[3]))

EQUATION( "TotNum")
/*
Sum of Num for each Species (the rounding causes the total to change)
*/
v[0]=0;

CYCLE(cur,"Species")
  v[0]+=VS(cur,"Num");
RESULT(v[0])


MODELEND




void close_sim(void)
{

}


