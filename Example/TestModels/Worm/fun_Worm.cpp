#include "fun_head.h"

MODELBEGIN


EQUATION("init")
/*
Initialize the lattice
*/
v[0]=V("Nrow");
v[1]=V("Ncol");

init_lattice(600, 600,v[0],v[1],"","", "",p, 1);
CYCLE(cur, "Segment")
 {
  v[2]=VLS(cur,"row",1);
  v[3]=VLS(cur,"col",1);
  update_lattice(v[2],v[3],2);
 }

PARAMETER;
RESULT( 1)

EQUATION("Wait")
/*
Just slow down the simulation
doing nothing for LimitWait
*/

v[0]=VL("Wait",1);
for(i=0; i<(int)v[0]; i++)
 for(j=0; j<10000; j++);

RESULT(v[0])



EQUATION("Move")
/*
Comment
*/

v[4]=V("IdSegment");
if(v[4]==1)
 {
  v[0]=V("row");
  v[1]=V("col");
  update_lattice(v[0],v[1],2);
 } 
else
 {
  v[5]=V("NSegment");
 if(v[5]==v[4])
  {
  v[2]=VL("row",1);
  v[3]=VL("col",1);
  update_lattice(v[2],v[3],1);
  } 
 } 
RESULT(1 )

EQUATION("Direction")
/*

         0
        
    7          1
             
6                  2
              
    5          3
              
         4
             

*/

v[0]=V("IdSegment");
if(v[0]==1)
 {
  v[1]=rnd_integer(-1, 1);  
  v[2]=VL("Direction",1);
  v[3]=v[2]+v[1];
  if(v[3]<0)
   v[3]+=7;
  else
   if(v[3]>7)
    v[3]-=7;
    
 }
else
 {
 cur=SEARCH_CNDS(p->up,"IdSegment",v[0]-1);

 v[3]=VLS(cur,"Direction",1);

 
 } 

RESULT(v[3] )

EQUATION("row")
/*
Column position
*/

v[0]=V("Direction");
v[1]=VL("row",1);

if(v[0]==0 || v[0]==1 || v[0]==7 )
 v[2]=v[1]+1;
else
 if(v[0]==3 || v[0]==4 || v[0]==5)
  v[2]=v[1]-1;
 else
  v[2]=v[1]; 

v[3]=V("Nrow");
if(v[2]>v[3])
 v[2]=1;
else
 if(v[2]<1)
  v[2]=v[3];  
 
RESULT(v[2] )


EQUATION("col")
/*
Column position
*/

v[0]=V("Direction");
v[1]=VL("col",1);

if(v[0]==1 || v[0]==2 || v[0]==3 )
 v[2]=v[1]+1;
else
 if(v[0]==5 || v[0]==6 || v[0]==7)
  v[2]=v[1]-1;
 else
  v[2]=v[1]; 

v[3]=V("Ncol");
if(v[2]>v[3])
 v[2]=1;
else
 if(v[2]<1)
  v[2]=v[3];  
 
RESULT(v[2] )

MODELEND




void close_sim(void)
{

}


