#include "../src/fun_head.h"

MODELBEGIN

EQUATION("Mean")
/*
Compute the mean strategy of the population. In the process compute also
the Variance of the strategy and the average fitness.

Note that the average fitness is computed with a lag because this equation is computed before
the fitness at present time. I chose this way to save one cycle through all the agents
*/
v[6]=v[0]=v[2]=v[3]=0;
CYCLE(cur, "Agent")
 {
  v[1]=VS(cur,"Strategy");
  v[0]+=v[1];
  v[3]+=v[1]*v[1];
  v[2]++;
  v[6]+=VLS(cur,"Fitness",1);
 }
 
v[5]=v[0]/v[2];
v[4]=v[3]/v[2]-v[5]*v[5];
WRITE("Variance",v[4]);
WRITE("AvFitness",v[6]/v[2]);
RESULT(v[5])


EQUATION("Fitness")
/*
Fitness = c1 - c2*|Strategy - Omega*Mean*(1-Mean)|

The fitness is computed as a function of the distance between Strategy and Mean
*/

v[0]=V("Strategy");
v[4]=V("Mean");
v[1]=V("c1");
v[2]=V("c2");
v[3]=V("Omega");
v[5]=v[1]-v[2]*abs(v[0]-v[3]*v[4]*(1-v[4]));

RESULT(v[5])

EQUATION("Genetic")
/*
Replace the strategy of worst agents with new agents (i.e. new strategies). The number of replaced agent is set by parameter Replace.

Note that the agents to remove are not physically removed, but their Strategy is overwritten 
with a new random number
*/

V("Mean"); //ensure that this equation is computed only after Mean has been computed
SORT("Agent", "Fitness", "UP"); //sort agents for increasing values of Fitness

v[1]=V("Replace"); //this is the number of agents to replace

//Starting from the first agent (worst Fitness) overwrites Strategy
v[0]=0;
CYCLE(cur, "Agent")
 {
 if(v[0]<v[1])
  WRITELS(cur,"Strategy",RND, t);
 else
  break;//interrupt the cycle 
 v[0]++; //increase the counter  
 }
   
   
RESULT(1)

MODELEND
/*
This function is executed once at the end of a simulation run. It may be used
to perform some cleanup, in case the model allocated memory during the simulation.
*/
void close_sim(void)
{

}


