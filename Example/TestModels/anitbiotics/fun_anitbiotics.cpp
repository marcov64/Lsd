#include "fun_head.h"

MODELBEGIN


EQUATION("Age")
/*
time passing by
*/

RESULT(CURRENT+1 )

EQUATION("Life")
/*
Check if the bug is too old, starved, or it can mate
*/

v[0]=V("Age");
v[1]=V("MaxAge");
if(v[1]<=v[0])
 v[2]=1;
else
 {
 
 v[3]=V("Eat");
 if(v[3]==0)
  v[2]=2;
 else
  {
   v[2]=3;
   v[3]=V("ProbMate");
   if(RND<v[3])
    WRITE("ReadyMate",1);
   else
    WRITE("ReadyMate",0); 
  } 
 } 
RESULT( v[2])

FUNCTION("Eat")
/*
Test if there is food
*/
v[0]=V("Fed");
v[1]=VL("Stock",1);
v[2]=(v[1]-v[0])/v[1];
if(RND<v[2])
 {
 INCR("Fed",1);
 v[2]=1;
 }
else
 v[2]=0; 

RESULT(v[2] )


EQUATION("Mate")
/*
Scan all bugs and reproduce the ones ready to mate
*/
v[10]=V("Action");
v[11]=0;
CYCLE(cur, "Bug")
 {
  v[0]=VS(cur,"ReadyMate");
  if(v[0]>0)
   {
    cur2=ADDOBJ_EX("Bug",cur);
    WRITES(cur2,"ReadyMate",0);
    WRITELS(cur2,"Age",0, t);
    v[11]++;
   }
 }

RESULT(v[11] )

EQUATION("Action")
/*
Main cycle on existing bugs: remove dead ones
and keep statistics
*/
WRITE("DeadAge",0);
WRITE("DeadStarved",0);
WRITE("Fed",0);

v[4]=0;
CYCLE_SAFE(cur, "Bug")
 {
  v[2]=VS(cur,"Life");
  if(v[2]==2 || v[2]==1)
   {
   DELETE(cur);
   if(v[2]==1)
    INCR("DeadAge",1);
   if(v[2]==2)
    INCR("DeadStarved",1);
   }
   else
    v[4]++;
 }

RESULT(v[4] )

EQUATION("Stock")
/*
compute the amount of food
*/
V("Action");
v[0]=V("Fed");
v[1]=V("IncrStock");
v[2]=V("NewStock");

v[3]=CURRENT-v[0];
v[4]=v[3]*v[1]+(1-v[1])*v[2];

RESULT(v[4] )

EQUATION("NumBugs")
/*
Number of bugs as the survivors from previous generation plus
new offspinrgs
*/
v[0]=V("Action");
v[1]=V("Mate");

RESULT(v[0]+v[1] )


FUNCTION("ProbMate")
/*
Comment
*/
v[0]=VS(c,"Age");
v[2]=V("StepAge");

if(v[0]>0)
 {
  v[1]=1-v[2]*(v[0]-1);
 }
else
 v[1]=0; 
RESULT(v[1] )







MODELEND




void close_sim(void)
{

}


