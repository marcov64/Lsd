#include "fun_head.h"
#include "stdlib.h"

MODELBEGIN

if(!strcmp(label,"AvRevenue"))
{
/*
Average revenue. It also compute other descriptive statistics on revenues.
*/
p->stat("Revenue",v);
p->write("MaxRevenue",v[3], 0);
p->write("MinRevenue",v[4], 0);
p->write("VarRevenue",v[2], 0);
res=v[1];
goto end;
}

if(!strcmp(label, "Revenue"))
{
/*

Revenues computed just as sales times price.
*/

v[0]=p->cal("Sales",0);
v[1]=p->cal("Price",0);

res=v[1]*v[0];
goto end;
}

if(!strcmp(label,"AvProfit"))
{
/*
Average profits. It also compute other descriptive statistics.

*/
p->stat("Profit",v);
p->write("MaxProfit",v[3], 0);
p->write("MinProfit",v[4], 0);
p->write("VarProfit",v[2], 0);
res=v[1];
goto end;
}

if(!strcmp(label,"Profit"))
{
/*
Profits are computed as revenues minus costs
*/

v[0]=p->cal("Revenue",0);
v[1]=p->cal("Cost",0);

res=v[0]-v[1];
goto end;
}


if(!strcmp(label,"Asset"))
{
/*
Cumulated profits
*/
v[0]=p->cal("Asset",1);
v[1]=p->cal("Profit",0);
res=v[0]+v[1];
goto end;
}

if(!strcmp(label,"Cost"))
{
/*
Total cost is the production times total unit costs
*/
v[0]=p->cal("Production",0);
v[1]=p->cal("CostInput",0);

v[2]=p->cal("Wage",0);
res=v[0]*(v[1]+v[2]);
goto end;
}

if(!strcmp(label,"Production"))
{
/*
The production is computed from the previous period value plus a portion of its difference with the current actual demand
*/
v[0]=p->cal("Production",1);
v[1]=p->cal("ActualDemand",0);
v[2]=p->cal("ProdDemand",0);

res=v[0] + (v[1]-v[0])*v[2];
goto end;
}

if(!strcmp(label,"ActualDemand"))
{
/*
Actual demand is an average smoothing the Demand
*/
v[0]=p->cal("Demand",0);
v[1]=p->cal("CoeffDemand",0);
v[2]=p->cal("ActualDemand",1);
res=v[2]*v[1]+v[0]*(1-v[1]);
goto end;
}

if(!strcmp(label,"CostInput"))
{
/*
The cost of input is computed as a smoothing over a random draw from a normal function
*/
v[0]=p->cal("CentralCostInput",0);
v[1]=p->cal("DeltaCostInput",0);
v[2]=p->cal("CostInput",1);
v[3]=p->cal("CoeffSmoothing",0);

res=v[2]*v[3]+norm(v[0],v[1])*(1-v[2]);
goto end;
}

if(!strcmp(label,"Demand"))
{
/*
The demand is a random walk bounded within a max and min limit
*/
v[0]=p->cal("Demand",1);
v[1]=p->cal("DeltaDemand",0);
v[2]=p->cal("MaxDemand",0);
v[3]=p->cal("MinDemand",0);
v[6]=(RND-0.5)*2;
v[8]=(v[0]-v[3])/(v[2]-v[3])*v[6];
v[9]=v[8]*v[1];

res=v[0]+v[9];
goto end;
}

if(!strcmp(label,"Wage"))
{
/*
The wage is a smoothing average over a random draw from a normal function
*/
v[0]=p->cal("CentralWage",0);
v[1]=p->cal("DeltaWage",0);
v[2]=p->cal("Wage",1);
v[3]=p->cal("CoeffSmoothing",0);
res=v[2]*v[3]+norm(v[0],v[1])*(1-v[3]);
goto end;
}


if(!strcmp(label,"Sales"))
{
/*
Sales are the minimum between the actual demand and the current production
*/
v[0]=p->cal("Production",0);
v[1]=p->cal("ActualDemand",0);

res=min(v[0],v[1]);
goto end;
}

if(!strcmp(label,"Price"))
{
/*
Price is computed with a (uniform random) markup over the unit cost of production
*/
v[0]=p->cal("Wage",1);
v[2]=p->cal("CostInput",1);
v[1]=p->cal("MarkUp",0);
res=(v[0]+v[2])*(1+RND*v[1]);
goto end;
}

MODELEND

/*
This function is executed once at the end of a simulation run. It may be used
to perform some cleanup, in case the model allocated memory during the simulation.
*/
void close_sim(void)
{

}

