#include "fun_head.h"


MODELBEGIN


FUNCTION("Shopping")
/*
Function in Market returning a random product
*/

cur=RNDDRAW("Firm","ProbShopping"); //choose randomly one firm
v[1]=VS(cur,"IdFirm"); //record the id. number of the firm
cur->increment("Sales",1);
RESULT(v[1] )



EQUATION("ProductUsed")
/*
Identification number of product used, either buying it 
or using the previously used one.
*/

//Look at the product used.
v[0]=VL("ProductUsed",1); 
cur=SEARCH_CND("IdFirm",v[0]);

//Probability of product failure
v[1]=VS(cur,"ProbBreak");


if(RND<v[1])
 {//The product broke down, a new one must be bought
  v[2]=V("Shopping");
 }
else
 {//The product is still working
 v[2]=v[0]; 
 }
 
RESULT(v[2] )




EQUATION("DayAction")
/*
Compute the level of sales
*/


CYCLE(cur, "Firm")
 {
  WRITES(cur,"InUse",0);
  WRITES(cur,"Sales",0);
 }
CYCLE(cur, "Consumer")
 {
  v[0]=VS(cur,"ProductUsed");
  cur1=SEARCH_CND("IdFirm",v[0]);
  INCRS(cur1,"InUse",1);

 }



RESULT(1)


EQUATION("ms")
/*
Market Shares
*/
V("DayAction");
v[0]=V("N"); //number of consumers
v[1]=V("InUse");

RESULT(v[1]/v[0] )

EQUATION("N")
/*
Number of consumers
*/

v[0]=0;
CYCLE(cur, "Consumer")
 {
  v[0]++; 
 }
PARAMETER
RESULT(v[0] )




EQUATION("ProbShopping")
/*
Probabiliy of purchasing the product
*/
V("Innovation");
v[0]=V("Quality");
v[1]=V("alphaQ");

v[2]=V("Productivity");
v[3]=V("alphaP");


RESULT(v[0]*v[1]+v[2]*v[3] )

EQUATION("sigma")
/*
variance of making a discovery
*/
v[0]=V("incrsigma");
v[1]=CURRENT+v[0];
RESULT(v[1] )



EQUATION("Productivity")
/*
Costs, decreasing by incremental learning
*/

v[2]=VL("Productivity",1);
v[5]=V("incrProd");
v[20]=V("MaxProd");

v[6]=VL("ms",1);
v[10]=v[2]*(1-v[6]*v[5])+v[6]*v[5]*v[20]; //new cost
//v[10]=v[2]+(1+v[6]*v[5]); //new cost


RESULT(v[10] )



EQUATION("Innovation")
/*
Attempt an innovation
*/

v[0]=V("Quality");
v[1]=V("alphaQ");
v[10]=V("Productivity");
v[3]=V("alphaP");
v[4]=V("sigma");

v[8]=norm(v[0],v[4]);

v[9]=V("InitProd");

if(v[0]*v[1]+v[3]*v[10]<v[8]*v[1]+v[3]*v[9])
 {//radical innovation
  
  WRITEL("Productivity",v[9],t);
  WRITE("Quality",v[8]);
  WRITEL("sigma",0, t);
  v[11]=1;
 }
else
 {//no innovation
  v[11]=2;
 } 
 

RESULT(v[11] )


EQUATION("TotalSales")
/*
Comment
*/
v[0]=SUM("Sales");

RESULT(v[0] )




EQUATION("Herf")
/*
Herfindal Index:
the higher the index the lower the concentration
*/
v[0]=0;
CYCLE(cur, "Firm")
 {
  v[1]=VS(cur,"ms");
  v[0]+=v[1]*v[1];
 }

RESULT( 1/v[0])




EQUATION("TurnOver")
/*
Comment
*/
v[0]=V("InUse");
v[1]=V("Removed");
if(v[0]>0)
 v[2]=v[1]/v[0];
else
 v[2]=-1; 
RESULT(v[2])


EQUATION("God")
/*
Comment
*/
v[10]=VL("God",1);
v[11]=V("Step");
if(v[10]<v[11])
 {
 res=v[10]+1;
 goto end;
 }
 
v[1]=V("TopQuality");
v[2]=V("LowQuality");

cur=SEARCH_CND("Quality",v[1]);
WRITES(cur,"Quality",v[2]);

v[3]=V("IncrQuality");

v[4]=0;
CYCLE(cur, "Firm")
 {
  v[0]=VS(cur,"Quality");
  v[4]+=MULTS(cur,"Quality",(1+v[3]));



 }

v[5]=0;
CYCLE(cur, "Firm")
 {
  v[0]=VS(cur,"Quality");
  WRITES(cur,"Quality",v[0]/v[4]);
  if(v[0]/v[4]>v[5])
   v[5]=v[0]/v[4];



 }
WRITE("TopQuality",v[5]);
RESULT( 1)


EQUATION("MS")
/*
Comment
*/
v[0]=VL("MS",1);
v[1]=V("Quality");
v[2]=V("smooth");

RESULT(v[0]+(v[1]-v[0])*v[2] )

MODELEND




void close_sim(void)
{

}



