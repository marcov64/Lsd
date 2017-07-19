#include "fun_head.h"

// define an interoperable sleep function
#ifdef _WIN32
#include <windows.h>
void msleep( unsigned milliseconds )
{
   Sleep( milliseconds );
}
#else
#include <unistd.h>
void msleep( unsigned milliseconds )
{
   usleep( milliseconds * 1000 );			// takes microseconds
}
#endif


MODELBEGIN

const char *chstr;


EQUATION("State")
/*

Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
Any live cell with two or three live neighbours lives on to the next generation.
Any live cell with more than three live neighbours dies, as if by overpopulation.
Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
*/

v[0]=0;
CYCLE_LINK(curl)
 {
  cur=LINKTO(curl);
  v[0]+=VLS(cur,"State",1);
 }
v[1]=0; 
if(v[0]==3)
 v[1]=1;
 
if(CURRENT==1 && v[0]==2)
 v[1]=1;
if(CURRENT!=v[1])
 {
  v[10]=V("row");
  v[20]=V("col");
  update_lattice(v[10],v[20],v[1]);
 }  
 
RESULT(v[1] )

EQUATION("SlowDown")
/*
Equation wasting time to slow down the graph
*/
v[0]=V("TimeSleep");
msleep((int)v[0]);
RESULT(1
 )



EQUATION("InitLattice")
/*
Initialize the model. Generate the lattice and place agents randomly.
*/
v[0]= V("nrow");    // n. of rows in the lattice
v[1]= V("ncol"); //n.columns in the lattice
p->init_lattice_net(v[0],v[1],"node", 1);

p->initturbo("node", v[0]*v[1] );
v[4]=V("PixWidth");
v[5]=V("PixHeight"); 
init_lattice(v[4],v[5], v[0], v[1], "", "", "", NULL, 0);

v[3]=V("PercActive");
CYCLE(cur, "node")
 { k=VS_NODEID(cur)-1;
   i=k/(int)v[1];
   j=k-i*(int)v[1];
   WRITES(cur,"row",(double)i+1);
   WRITES(cur,"col",(double)j+1);
   if(v[3]>0 && RND<v[3])
    WRITELS(cur,"State",1, t-1);
   else
    WRITELS(cur,"State",0, t-1); 
   update_lattice( VS(cur,"row"), VS(cur,"col"), VLS(cur,"State",1));  
 }
if(v[3]<0)
 {
  cmd(inter, "set fname [tk_getOpenFile -title \"Select file with initial active cells\"]");
  chstr=(char *)Tcl_GetVar(inter, "fname",0);
  
  f=fopen(chstr, "r");
  if(f==NULL)
   {quit=2;
    plog("\nWrong file name\n\n");
    END_EQUATION(0);
   } 
  while(  fscanf(f, "%d %d", &i, &j)>0)
   {
    
    v[2]=(double)i*v[1]+(double)j;
    //cur=p->turbosearch("node",v[0]*v[1],v[2]);
    cur=TSEARCHT("node", v[0]*v[1],v[2]);
    WRITELS(cur,"State",1, t-1);
   
   }
  fclose(f);
 }
cmd(inter,"update");
PARAMETER 
END_EQUATION(0);
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
  cur1=TSEARCHT("node", v[12]
,v[13]);
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
 }
p->initturbo("Agent", v[10] ); 
PARAMETER
RESULT(1 )




MODELEND




void close_sim(void)
{

}


