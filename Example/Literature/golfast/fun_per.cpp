#include "fun_head.h"

object *find_nei(object *o,int r, int c, double nrow, double ncol, object *lattice);

MODELBEGIN

EQUATION("EvalCell")
/*
State can assume three different values: 0, 1 and 2.

If it is 0 it does nothing. State=0 means that the cell never used a product, nor it has been triggered to act (i.e. no neighbour uses a product.

State=1 means that the agent did not have a product, but the previous period a neighbour bought a product. Therefore, chooses a product (which one is decided by "Choose") and triggers the state of neighbours to be 1, subject to the neighbour has the "Flag" value of 0. Flag is 0 if you did never bought a product, or more than TimeReActivate steps have passed since last purchase.

State=2 means that the agent is currently using a product, and no neighbours had triggered it to make a new purchase.
*/
v[0]=0;
CYCLES(c,cur, "Nei")
 {
  v[0]+=VS(cur->hook,"State");
 }
 
if ( v[0]==3 || (VS(c,"State")==1 && v[0]==2) )
 v[1]=1;
else
 v[1]=0;  
  
RESULT( v[1])


EQUATION("Action")
/*
Doesn't work.
*/
v[0]=(double)t;
v[5]=V("PlotLattice");
v[4]=0;
v[10]=0;
CYCLE_SAFE(cur, "Job")
 {
  v[1]=VS(cur,"T");
  if(v[1]==v[0])
  { 
  
  v[2]=VS(cur,"JState");
  WRITES(cur->hook,"State",v[2]);
  if(v[5]==1)
  {
   v[6]=VS(cur->hook,"CopyRow");
   v[7]=VS(cur->hook,"IdCol");       
   update_lattice(v[6], v[7], v[2]);
  } 

  CYCLES(cur->hook, cur1, "Nei")
   {
    cur2=ADDOBJ("Job");
    WRITES(cur2,"T",v[0]+1);
    v[6]=VS(cur1->hook,"CopyRow");
    WRITES(cur2,"JRow",v[6]);
    v[6]=VS(cur1->hook,"IdCol");
    WRITES(cur2,"JCol",v[6]);
    cur2->hook=cur1->hook;
    v[4]++;

   }
  DELETE(cur);
  //WRITES(cur,"T",v[0]+1);
  } 
  else
   {//INTERACT("asa",1);
    if(VS(cur->hook, "Evaluated")==v[0])
     DELETE(cur);
    else
     {
      v[3]=V_CHEAT("EvalCell", cur->hook);
      v[6]=VS(cur->hook,"State");
      if(v[6]!=v[3])
       {
        WRITES(cur->hook,"Evaluated",v[0]);
        WRITES(cur,"JState",v[3]);
       }
       else
        DELETE(cur);
     } 
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

v[2]=V("NCol");
v[3]=V("NRow");
v[4]=V("PixWidth");
v[5]=V("PixHeight");
if(v[10]==1)
 init_lattice(v[4],v[5], v[3], v[2], "IdRow", "IdCol", "State", NULL, 0);



cur=SEARCH("Lattice");
ADDNOBJS(cur,"Row",v[3]-1);
v[9]=1;
CYCLE(cur, "Row")
 {
  WRITES(cur,"IdRow",v[9]++); 
  ADDNOBJS(cur,"Col",v[2]-1);
  
  v[13]=1;

  CYCLES(cur, cur1, "Col")
   {
    WRITES(cur1,"IdCol",v[13]++);
    ADDNOBJS(cur1,"Nei",7);
    if(RND<0)
    {
     update_lattice(v[9]-1, v[13]-1, 1);
     WRITELS(cur1,"State",1, t-1);
    } 

   } 

 }

/*
v[20]=1;
v[21]=2;
update_lattice(v[20],v[21] ,1);
cur=SEARCH_CND("IdRow",v[20]);
cur1=SEARCH_CNDS(cur,"IdCol",v[21]);
WRITELS(cur1,"State",1, t-1);

v[20]=2;
v[21]=3;
update_lattice(v[20],v[21] ,1);
cur=SEARCH_CND("IdRow",v[20]);
cur1=SEARCH_CNDS(cur,"IdCol",v[21]);
WRITELS(cur1,"State",1, t-1);

v[20]=3;
v[21]=1;
update_lattice(v[20],v[21] ,1);
cur=SEARCH_CND("IdRow",v[20]);
cur1=SEARCH_CNDS(cur,"IdCol",v[21]);
WRITELS(cur1,"State",1, t-1);


v[20]=3;
v[21]=2;
update_lattice(v[20],v[21] ,1);
cur=SEARCH_CND("IdRow",v[20]);
cur1=SEARCH_CNDS(cur,"IdCol",v[21]);
WRITELS(cur1,"State",1, t-1);

v[20]=3;
v[21]=3;
update_lattice(v[20],v[21] ,1);
cur=SEARCH_CND("IdRow",v[20]);
cur1=SEARCH_CNDS(cur,"IdCol",v[21]);
WRITELS(cur1,"State",1, t-1);
*/
for(cur6=cur5=SEARCH("Row"); go_brother(cur6)!=NULL; cur6=go_brother(cur6) );
//cur5 first row
//cur6 last row


CYCLE(cur, "Row")
 {
  v[0]=v[9]=VS(cur, "IdRow");
  
  sprintf(msg, "\n2) Row %d", (int)v[9]);
  plog(msg);
  cmd(inter, "update");
  
  cur7=SEARCHS(cur,"Col");
  cur8=cur->hook;
  
  CYCLES(cur, cur1, "Col")
  { 
   WRITES(cur1,"CopyRow",v[9]);   
   v[1]=v[13];
   v[14]=1;
   cur2=SEARCHS(cur1,"Nei");
   cur4=find_nei(cur2,-1,-1,v[3],v[2], p);
   cur2->hook=cur4;
   cur2=go_brother(cur2);

   cur4=find_nei(cur2,-1,0,v[3],v[2], p);
   cur2->hook=cur4;   
   cur2=go_brother(cur2);

   cur4=find_nei(cur2,-1,1,v[3],v[2], p);
   cur2->hook=cur4;   
   cur2=go_brother(cur2);
   
   cur4=find_nei(cur2,0,-1,v[3],v[2], p);
   cur2->hook=cur4;   
   cur2=go_brother(cur2);
 
   cur4=find_nei(cur2,0,1,v[3],v[2], p);
   cur2->hook=cur4;   
   cur2=go_brother(cur2);
   
   cur4=find_nei(cur2,1,-1,v[3],v[2], p);
   cur2->hook=cur4;   
   cur2=go_brother(cur2);
   
   cur4=find_nei(cur2,1,0,v[3],v[2], p);
   cur2->hook=cur4;   
   cur2=go_brother(cur2);
   
   cur4=find_nei(cur2,1,1,v[3],v[2], p);
   cur2->hook=cur4;   
   cur2=go_brother(cur2);
   

  }
 } 


CYCLE(cur, "Job")
 {
  v[0]=VS(cur,"JRow");
  cur2=SEARCH_CND("IdRow",v[0]);
  v[1]=VS(cur,"JCol");
  cur1=SEARCH_CNDS(cur2,"IdCol",v[1]);
  cur->hook=cur1;
 }

PARAMETER
RESULT(1)


EQUATION("State")
/*
State can assume three different values: 0, 1 and 2.

If it is 0 it does nothing. State=0 means that the cell never used a product, nor it has been triggered to act (i.e. no neighbour uses a product.

State=1 means that the agent did not have a product, but the previous period a neighbour bought a product. Therefore, chooses a product (which one is decided by "Choose") and triggers the state of neighbours to be 1, subject to the neighbour has the "Flag" value of 0. Flag is 0 if you did never bought a product, or more than TimeReActivate steps have passed since last purchase.

State=2 means that the agent is currently using a product, and no neighbours had triggered it to make a new purchase.
*/
v[0]=0;
CYCLE(cur, "Nei")
 {
  v[0]+=VLS(cur->hook,"State",1);
 }
 
if ( v[0]==3 || (CURRENT==1 && v[0]==2) )
 v[1]=1;
else
 v[1]=0;  
if(v[1]!=CURRENT)
 {
  v[6]=V("CopyRow");
  v[7]=V("IdCol");
  update_lattice(v[6], v[7], v[1]); 
 } 
  
RESULT( v[1])


FUNCTION("Choose")
/*
Choose the product to buy for the calling cell.

The function scans all neighbours of the calling cell. When it finds the nei. that has chosen its product the previous period, then stores within the "Prod" objects (parameter "app") some probabilities.

The objects assigned positive probabilities are: the one chosen by the triggering nei.; and those around it for a lenght of "NumTail" (the total is always an odd number). Boundaries conditions are accounted for, so to keep the number of total objects with positive probabilities positive.

The concerned "Prod" are discarded if their PP is lesser than the q of the choosing agent. That is, a product to be considered needs to have a quality above the consumer's threshold.

Notice that if the consumer has two triggering neighbours, then it can choose among a double-size option set.

The probabioities assigned to each producer to be chosen are proportional to the PP of the producer raised to the power of alpha. Setting alpha to 0 assigns identical probabilities.

For statistical purposes the parameter "tempSales" of the newly chosen firm is increased, and that of the discarded product, if exist, is decreased.

*/

v[0]=VS(c,"q");


CYCLE(cur, "Prod")
 {
  WRITES(cur,"app",0);
  v[1]++;
 }


v[13]=V("alpha");
v[7]=0;
v[1]=V("NumProducers");
v[8]=V("NumTail");
i=(int)v[8]*2+1;
CYCLES(c,cur, "Nei")
 {
  v[2]=VLS(cur->hook,"State",2);
  v[3]=VLS(cur->hook,"State",1);
  if( v[2]==1 && v[3]==2)
   {
    v[4]=VS(cur->hook,"IdUsed");
    v[10]=v[4]-v[8];
    v[11]=v[4]+v[8];
    if(v[10]<1)
     {
      v[11]-=v[10]+1;
      v[10]=1;
     }    
    if(v[11]>v[1])
     {
      v[10]-=(v[11]-v[1]);
      v[11]-=(v[11]-v[1]);
     }
    cur1=SEARCH_CND("IdProd",v[10]);
    
    for( j=0; j<i; j++)
     {
      v[5]=VS(cur1,"PP");
      if(v[5]>v[0])
      {
       v[6]=pow(v[5],v[13]);
       INCRS(cur1,"app",v[6]);
       v[7]++;
      }
      cur1=go_brother(cur1);
     } 

   }//end of triggering neighours
  }//end of the scanning of neighborus

if(v[7]>0)
 {

 cur=RNDDRAW("Prod","app");
 v[4]=VS(cur,"IdProd");
 INCRS(cur,"tempSales",1);
 v[8]=VS(c,"IdUsed");
 if(v[8]>0)
  {
  cur1=SEARCH_CND("IdProd",v[8]);
  INCRS(cur1,"tempSales",-1);
  
  }

 }
else
 v[4]=0; 
RESULT(v[4] )

//5809250 I-A

EQUATION("NumProducers")
/*
Compute the number of producers currently in the model.
*/
v[0]=0;
CYCLE(cur, "Prod")
 v[0]++;
RESULT(v[0] )



EQUATION("PP")
/*
Quality of the product, increasing with the number of consumers.

The function is:
              [1-PP(0)]
PP(t)= 1 - ----------------
            [1+k*N(t-1)]^a
            
The function increases (with decreasing steps) from PP(0) (initial PP at time 0) up to 1 for N going to infinite. K and a influence the speed.
*/

v[0]=VL("Users",1);
v[1]=V("pp0");
v[2]=V("a");
v[3]=V("k");

v[4]=1+v[3]*v[0];

v[5]=1-(1-v[1])*1/pow(v[4],v[2]);

RESULT(v[5])

EQUATION("Users")
/*
UPdate the statistics of the number of users.

To speed things up, the computation is indirect. Previous period users are increased of the parameter "tempSales", which is the reset for the next period computation. Current net sales (new consumers minus abandoning ones) are stored in Sales.
*/
VS(p->up->next,"ActionDemand");
v[0]=VL("Users",1);
v[1]=V("tempSales");

WRITE("Sales",v[1]);
WRITE("tempSales",0);
RESULT(v[0]+v[1] )


EQUATION("CopyRow")
/*
Copy of the IdRow placed in Cell, to improve speed.
*/
v[0]=V("IdRow");
PARAMETER
RESULT(v[0] )


EQUATION("Entry")
/*
Entry of new producers, allowed in every PeriodEntry time steps
*/

v[0]=VL("Entry",1);

if(v[0]<0)
 END_EQUATION(v[0]+1)
/*
CYCLE_SAFE(cur, "Prod")
 {
 v[5]=VS(cur,"Users");
 if(v[5]==0)
  DELETE(cur);
 }
*/  
 
cur=SEARCH("Prod");
v[3]=VS(cur,"pp0");//pp0 of the very first product
cur=ADDOBJ_EX("Prod",cur);
v[2]=V("IssueIdProd");
WRITES(cur,"IdProd",v[2]);  


WRITELS(cur,"PP",v[3],t);
WRITES(cur,"pp0",v[3]);
WRITELS(cur,"Users",0,t);
WRITES(cur,"Sales",0);

v[4]=V("PeriodEntry");

RESULT(v[4] )

FUNCTION("IssueIdProd")
/*
Deliver the IdProd for entrants
*/

RESULT(val[0]+1 )



MODELEND




void close_sim(void)
{

}


object *find_nei(object *o,int r, int c, double nrow, double ncol, object *lattice)
{

double IndR, IndC;
object *ores;

IndR=VS_CHEAT(o,"IdRow",o)+r;
if(IndR<1)
 IndR=nrow;
if(IndR>nrow)
 IndR=1;

IndC=VS_CHEAT(o,"IdCol",o)+c;
if(IndC<1)
 IndC=ncol;
if(IndC>ncol)
 IndC=1;
  

   ores=SEARCH_CNDS(lattice,"IdRow",IndR);
   ores=SEARCH_CNDS(ores,"IdCol",IndC);

return ores;
}
