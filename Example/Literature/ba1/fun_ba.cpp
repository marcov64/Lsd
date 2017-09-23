#include "fun_head.h"

MODELBEGIN



EQUATION("NewAgent")
/*
Create a new agent
*/

if(RND<0.5)
 v[1]=0;
else
 v[1]=1; 
RESULT(v[1] )

EQUATION("NumAgent0")
/*
Count the number of agents 0
*/
v[0]=V("NewAgent");
v[1]=VL("NumAgent0",1);

if(v[0]==0)
  v[1]++; 
RESULT(v[1] )

EQUATION("NumAgent1")
/*
Count the number of agents 1
*/
v[0]=V("NewAgent");
v[1]=VL("NumAgent1",1);

if(v[0]==1)
  v[1]++; 
RESULT(v[1] )


EQUATION("UtilityMS")
/* Utility in using Microsoft */
v[0]=V("NewAgent"); //type of new agent
if(v[0]==0)
 {v[1]=V("User0MS"); //value for 0 in using MS
  v[2]=V("User0Net"); //net. ext. coefficient for 0
 }
else
 {v[1]=V("User1MS"); //value for 1 in using MS
  v[2]=V("User1Net"); //net. ext. coefficient for 1
 }
v[3]=VL("NumMS",1); //number of existing consumers using MS
v[4]=v[1]+v[2]*v[3]; 
RESULT(v[4] )


EQUATION("UtilityApple")
/* Utility in using Apple */
v[0]=V("NewAgent"); //type of new agent
if(v[0]==0)
 {v[1]=V("User0Apple"); //value for 0 in using Apple
  v[2]=V("User0Net"); //net. ext. coefficient for 0
 }
else
 {v[1]=V("User1Apple"); //value for 1 in using Apple
  v[2]=V("User1Net"); //net. ext. coefficient for 1
 }
v[3]=VL("NumApple",1); //number of existing consumers using Apple
v[4]=v[1]+v[2]*v[3]; 
RESULT(v[4] )


EQUATION("NumMS")
/*
Number of MS users
*/
v[0]=VL("NumMS",1);
v[1]=V("UtilityApple");
v[2]=V("UtilityMS");

if(v[2]>v[1])
 v[3]=v[0]+1;
else
 v[3]=v[0]; 
RESULT(v[3] )


EQUATION("NumApple")
/*
Number of Apple users
*/
v[0]=VL("NumApple",1);
v[1]=V("UtilityApple");
v[2]=V("UtilityMS");

if(v[1]>v[2])
 v[3]=v[0]+1;
else
 v[3]=v[0]; 
RESULT(v[3] )


EQUATION("ShareMS")
/* Share of MS users */
v[0]=V("NumMS");
v[1]=V("NumApple");
RESULT(v[0]/(v[0]+v[1]) )

EQUATION("ShareApple")
/* Share of Apple users */
v[0]=V("NumMS");
v[1]=V("NumApple");
RESULT(v[1]/(v[0]+v[1]) )


EQUATION("Statistics")
/*
Compute the shares of markets for the two brands
*/
v[0]=v[1]=0;
CYCLE(cur, "Market")
 {
  v[2]=VS(cur,"NumApple");
  v[3]=VS(cur,"NumMicrosoft");
  if(v[2]>v[3])
   v[0]++;
  else
   v[1]++; 
 }

WRITE("NumMarketApple",v[0]/(v[0]+v[1]));
WRITE("NumMarketMS",v[1]/(v[0]+v[1]));
RESULT( 1)

MODELEND




void close_sim(void)
{

}


