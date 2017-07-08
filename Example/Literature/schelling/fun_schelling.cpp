#include "fun_head.h"

MODELBEGIN



EQUATION("InitLattice")
/*
Initialize the model. Generate the lattice and place agents randomly.
*/
v[0]= V("nrow");    // n. of rows in the lattice
v[1]= V("ncol"); //n.columns in the lattice
p->init_lattice_net(v[0],v[1],"node", 1);
   
CYCLE(cur, "node")
 { k=VS_NODEID(cur)-1;
   i=k/(int)v[1];
   j=k-i*(int)v[1];
   WRITES(cur,"row",(double)i+1);
   WRITES(cur,"col",(double)j+1);
 
 }
/***
CYCLE(cur, "node")
 {
 ADDNOBJS(cur,"testLink",7);
 cur1=SEARCHS(cur,"testLink");

 CYCLES_LINK(cur,curl)
 {
  cur2=LINKTO(curl);
  v[50]=VS(cur2,"row");
  WRITES(cur1,"tlrow",v[50]);
  v[50]=VS(cur2,"col");
  WRITES(cur1,"tlcol",v[50]);
  cur1->hook=cur2;
  cur1=go_brother(cur1);
 }
}
INTERACT("PROVA", v[0]);    
***/

v[2]=v[1];
v[3]=v[0];

v[80]=V("PlotLattice");
if(v[80]==1)
{
v[4]=V("PixWidth");
v[5]=V("PixHeight"); 
init_lattice(v[4],v[5], v[3], v[2], "", "", "", NULL, 2);
}
v[10]=V("numAgents");
ADDNOBJ("Agent",v[10]-1);

v[12]=v[0]*v[1];
v[16]=V("shareType");
v[99]=0;
CYCLE(cur, "Agent")
 {v[99]++;
  v[13]=rnd_integer(1, v[12]);
  cur1=TSEARCHT("node", v[12],v[13]);
  while(cur1!=NULL && cur1->hook!=NULL)    cur1=go_brother(cur1);
  if(cur1==NULL)
     {
      cur1=SEARCH("node");
      while( cur1!=NULL && cur1->hook!=NULL)    cur1=go_brother(cur1);
     } 
  if(cur1==NULL)
   { plog("\nError, lattice saturated, too many agents.\n");
     INTERACT("Saturation", v[13]);
     PARAMETER;
     END_EQUATION(-1);
   }
  cur->hook=cur1;
  cur1->hook=cur;   
  v[14]=VS(cur1,"row");
  v[15]=VS(cur1,"col");
  if(RND<v[16])
   v[17]=1;
  else
   v[17]=0; 
  if(v[80]==1) 
    update_lattice(v[14], v[15], v[17]);
  WRITES(cur,"Type",v[17]); 
  v[71]=V("meanThreshold");
  v[72]=V("varThreshold");
  v[73]=min(1,max(0,norm(v[71],v[72])));
  WRITES(cur,"Threshold",v[73]);    
 }
TSEARCHT_INI("Agent", v[10] ); 
PARAMETER
RESULT(1 )

EQUATION("Action")
/*
Action of one period.
The system picks randomly one agent who is dissatisfied with its condition because the share of neighbours members of the different is above the threshold. When one is encountered the agent is relocated randomly in one empty cell. 
*/
if(V("Complete")!=0) //If no agent is dissatified skip the computation
 END_EQUATION(0);
 
v[0]=V("nrow");
v[40]=V("ncol");
v[12]=v[0]*v[40];
v[31]=V("numAgents");
v[32]=rnd_integer(1,v[31]);
cur=TSEARCHT("Agent", v[31],v[32]);
v[1]=V("numAgents");
v[2]=0;
while(v[2]<v[1] && V_CHEAT("Assess",cur)==1) //while the agent is happy
 {
  cur=go_brother(cur);
  if(cur==NULL)
   cur=SEARCH("Agent");
  v[2]++;
 }
if(v[2]==v[1])
 {
  //quit=1;
  WRITE("Complete",(double)t);
  END_EQUATION(0);
 } 
v[4]=VS(cur->hook,"row");
v[5]=VS(cur->hook,"col"); 
if(V("PlotLattice")==1)
 update_lattice(v[4], v[5], 2);
cur->hook->hook=NULL;

v[13]=rnd_integer(1, v[12]);
cur1=TSEARCHT("node", v[12],v[13]);
while(cur1!=NULL && cur1->hook!=NULL)    cur1=go_brother(cur1);
if(cur1==NULL)
{
 cur1=SEARCH("node");
 while(cur1->hook!=NULL)    cur1=go_brother(cur1);
} 
  cur->hook=cur1;
  cur1->hook=cur;   
  v[14]=VS(cur1,"row");
  v[15]=VS(cur1,"col");
  v[17]=VS(cur,"Type"); 
if(V("PlotLattice")==1)  
  update_lattice(v[14], v[15], v[17]);
  
RESULT( 1)


EQUATION("Assess")
/*
Controls whether the number of different type neighbours are above the threshold.
Returns 1 if the agent is happy (few strangers) and 0 if it is not (too many strangers).
*/
v[0]=VS(c,"Type");

v[1]=v[2]=0;

v[5]=V("Threshold");
CYCLES_LINK(c->hook,curl)
 {
  cur2=LINKTO(curl);
  if(cur2->hook!=NULL)
   {
    v[20]=VS(cur2->hook,"Type");
    if(v[20]==v[0])
     v[1]++;
    else
     v[2]++; 
   }
 }
if(v[1]+v[2]>0) 
 v[6]=v[1]/(v[1]+v[2]);
else
 v[6]=1;
   
if(v[6]>v[5])
 v[3]=1;
else
 v[3]=0;  
RESULT(v[3] )




EQUATION("ShareSameType")
/*
Share of neighbour with the same type of the agent.
In the process the equation computes also the average value of Threshold and its variance.
*/
v[0]=v[1]=v[2]=v[3]=v[5]=v[6]=0;
CYCLE(cur, "Agent")
 {
  v[2]=VS(cur,"Type");
  CYCLES_LINK(cur->hook,curl)
   {
    cur2=LINKTO(curl);
    if(cur2->hook!=NULL)
     {
      v[0]++;
      v[4]=VS(cur2->hook,"Type");
      if(v[4]==v[2])
       v[1]++;
     }
   }
  v[7]=VS(cur,"Threshold"); 
  v[3]+=v[7]; 
  v[6]+=v[7]*v[7];
  v[5]++;
 }  
v[8]=v[6]/v[5]-(v[3]/v[5])*(v[3]/v[5]);
WRITE("ActVarThreshold",v[8]);
WRITE("ActAvThreshold",v[3]/v[5]); 
RESULT(v[1]/v[0] )


EQUATION("numAgents")
/*
Number of agents in the lattice, computed as a percentage of all the cells.

*/

v[0]=V("ShareAgents");
v[1]=V("nrow");
v[2]=V("ncol");
v[3]=floor(v[0]*v[2]*v[1]);
PARAMETER
RESULT(v[3] )

EQUATION("AssignState")
/*
Function called manually to assign the value of its agent's type, if any.

Used only to plot a lattice from AoR.
*/
CYCLE(cur, "node")
 {
  if(cur->hook==NULL)
   WRITES(cur,"State",3);
  else
   {
    v[0]=VS(cur->hook,"Type");
    WRITES(cur,"State",v[0]);
   } 
 }

RESULT(1 )

EQUATION("Threshold")
/*
Tolerance level of an agent in respect of having neighbours form different type.
The higher the level the lower the tolerance to different type 

The equation approximates the average tolerance from neighbours.
*/
v[0]=v[1]=0;

CYCLES_LINK(p->hook,curl)
 {
  cur=LINKTO(curl);
  if(cur->hook!=NULL)
   {
    v[1]++;
    v[0]+=VLS(cur->hook,"Threshold",1);
   }
 }

if(v[1]==0)
 END_EQUATION(VL("Threshold",1));
v[2]=VL("Threshold",1);
v[3]=V("aThreshold");
v[4]=v[2]*v[3]+(1-v[3])*v[0]/v[1];

RESULT(v[4] )

MODELEND


void close_sim(void)
{

}


