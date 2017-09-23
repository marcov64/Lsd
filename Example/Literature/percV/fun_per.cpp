#include "fun_head.h"

int **dat;
int **sta;
MODELBEGIN








EQUATION("Action")
/*
Comment
*/
v[0]=(double)t;
v[2]=V("NCol");
v[3]=V("NRow");

v[5]=V("PlotLattice");

v[4]=0;
v[10]=0;
CYCLE_SAFE(cur, "Job")
 {v[10]++;
  v[1]=VS(cur,"T");
  if(v[1]!=v[0])
   break;
  v[20]=VS(cur,"x");
  v[21]=VS(cur,"y");
  
  if(v[20]==v[2]-1) // go E
    h=0;
  else
    h=(int)v[20]+1;
  k=(int)v[21];
  if(dat[h][k]==1 && sta[h][k]==0)
   {
   v[4]++;
   sta[h][k]=1;
   cur2=ADDOBJ("Job");
   WRITES(cur2,"T",v[0]+1);
   WRITES(cur2,"x",(double)h);
   WRITES(cur2,"y",(double)k);
   if(v[5]==1)
    update_lattice((double)h+1, (double)k+1, 2);
   }  

  if(v[20]==0) // go W
    h=(int)v[2]-1;
  else
    h=(int)v[20]-1;
  k=(int)v[21];
  if(dat[h][k]==1 && sta[h][k]==0)
   {
   v[4]++;
   sta[h][k]=1;
   cur2=ADDOBJ("Job");
   WRITES(cur2,"T",v[0]+1);
   WRITES(cur2,"x",(double)h);
   WRITES(cur2,"y",(double)k);
   if(v[5]==1)
    update_lattice((double)h+1, (double)k+1, 2);
   }  

  if(v[21]==0) // go N
    k=(int)v[3]-1;
  else
    k=(int)v[21]-1;
  h=(int)v[20];
  if(dat[h][k]==1 && sta[h][k]==0)
   {
   v[4]++;
   sta[h][k]=1;
   cur2=ADDOBJ("Job");
   WRITES(cur2,"T",v[0]+1);
   WRITES(cur2,"x",(double)h);
   WRITES(cur2,"y",(double)k);
   if(v[5]==1)
    update_lattice((double)h+1, (double)k+1, 2);
   }  

  if(v[21]==v[3]-1) // go S
    k=0;
  else
    k=(int)v[21]+1;
  h=(int)v[20];
  if(dat[h][k]==1 && sta[h][k]==0)
   {
   v[4]++;
   sta[h][k]=1;
   cur2=ADDOBJ("Job");
   WRITES(cur2,"T",v[0]+1);
   WRITES(cur2,"x",(double)h);
   WRITES(cur2,"y",(double)k);
   if(v[5]==1)
    update_lattice((double)h+1, (double)k+1, 2);
   }  

  if(go_brother(cur)!=NULL) 
   {
    DELETE(cur);
    v[10]--;
   }
 }
if(v[4]==0)
 {
  
  quit=2;
 } 
RESULT(v[4] )

EQUATION("SumPoint")
/*
Comment
*/

RESULT( CURRENT+V("Action"))


EQUATION("Init")
/*
Initialize the lattice:

- If PlotLattice !=0 then generate the lattice window

- Set the pointers to the neighbours of each cell

*/

v[10]=V("PlotLattice");
v[8]=V("p");
v[2]=V("NCol");
v[3]=V("NRow");
v[4]=V("PixWidth");
v[5]=V("PixHeight");
if(v[10]==1)
 init_lattice(v[4],v[5], v[3], v[2], "IdRow", "IdCol", "State", NULL, 0);

dat=new int *[(int)v[3]];
sta=new int *[(int)v[3]];

for(i=0; i<(int)v[3]; i++)
 {
  dat[i]=new int [(int)v[2]];
  sta[i]=new int [(int)v[2]];
  j=(int)v[2];
  for(h=0; h<j; h++)
   {sta[i][h]=0;
    if(RND<v[8])
     dat[i][h]=1;
    else
     dat[i][h]=0; 
   }
 }  
v[11]=rnd_integer(0,v[2]-1);
v[12]=rnd_integer(0,v[3]-1);

cur=SEARCH("Job");
WRITES(cur,"T",1);
sta[(int)v[11]][(int)v[12]]=1;

WRITES(cur,"x",v[11]);
WRITES(cur,"y",v[12]);
if(v[10]==1)
 update_lattice(v[11], v[12], 2);

PARAMETER;

RESULT( 1)



MODELEND




void close_sim(void)
{
double v[10];
v[0]=root->cal("NRow",0);
v[1]=root->cal("NCol",0);
for(v[2]=0; v[2]<v[1]; v[2]++)
 {
  delete sta[(int)v[2]];
  delete dat[(int)v[2]];
 }
 delete sta;
 delete dat;
}


