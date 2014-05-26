
#include "fun_head.h"
object *turboskip(int n);
int *prodlist;
object **l;
#define MAX_AGENT 300000
object *last, *first;




MODELBEGIN

EQUATION("TTB")
/*

TTB is defined as a function, being executed at each time step any time its value is requested. The function expects that the "calling" object that requested its value to be a DM object, from which it takes the required values.

Return the IdFirm of the chosen firm depending on the ranking of characteristics of the consumer, its error in evaluating the actual values of ch. and its tolerance.

Choose one option on the base of their characteristics "x", using "tau" as tolerance and "Delta" as random bias of the observed characteristics' values.

Consumers' preferences are the order by which the characteristics are used to filter out (perceived) inferior products.

For each characteristics, the perceived value for each option is a random value drawn from a normal centered around the true value and variance equal to Delta.

All options scoring more than tau times the max value for the current characteristics are considered equivalent to the maximum, and therefore not filtered away.

Before starting the selection procedure, options are scanned to remove those scoring less than a minimum on some characteristic.
*/



v[0]=0;
//select out the options scoring less than the minimum on some characteristic
CYCLE(cur, "Prod")
{
 v[24]=1; //assume the option to be viable, and change this to 0 if it is not
 CYCLES(cur, cur1, "Ch")
  {//for any characteristic
   v[20]=VS(cur1,"IdCh");
   cur2=SEARCH_CNDS(c,"AIdCh",v[20]); //find the ch. of the option
   v[4]=VS(c,"Deviation"); //bias of the consumer concerning this ch.
   v[21]=VS(cur2,"Minimum"); // this is the minimum for the consumer on this characteristic
   v[22]=VS(cur1,"Value"); //true value of the product on this ch.
   v[23]=norm(v[22],v[4]); //this is the observed value for this time step for the consumer
   WRITES(cur1,"obs_x",v[23]); //write the observed value on a temporary parameter
   v[25]=VS(cur2,"QualityType"); //this the sign (1 or -1) of the quality. E.g. price (negative) is -1.
   if(v[21]*v[25]>v[23]*v[25]) //
    { //option too low, below the minimum requirement
     v[24]=0;
     break;//stop here to scan the ch. of the firm, since the product fails to meet all the minimum requiremens
    }

  }
  if(v[24]==1)
   { //product is viable
    WRITES(cur,"app",1); //mark the product as viable
    v[0]++; //count the number of viable products
   }
  else
    WRITES(cur,"app",0); //mark the product as not viable

}	//end of scanning all products for viability



if(v[0]==0) //no option viable
 {
  //END_EQUATION(-1); //no point in continuing. Return -1, signaling no choice has been made
 //in this model I want a firm to be chosen always
 cur=RNDDRAWFAIR("Prod");
 v[16]=VS(cur,"IdProd");
 END_EQUATION(v[16]);
 }
 
 
if(v[0]==1)
 {//only one option viable, choose it outright
  cur=SEARCH_CND("app",1);
  v[0]=VS(cur,"IdProd");
  END_EQUATION(v[0]);
 } 
  
//Do a proper choice among the (more than one) viable options

//INTERACT("First", v[0]); //uncomment this line to force Lsd to stop before moving to choose.

v[30]=0;
CYCLES(c, cur, "AgCh")
 {//for each characteristic, in the order of the decision maker
  

  
  if(v[0]==1)
    {
   //plog("cazzo1\n");
    break; //no point in continuing if there is only one firm left
    
    }
  v[30]++; //counter of ch. used  
  v[27]=1;
  v[1]=VS(cur,"AIdCh");
  v[25]=VS(cur,"QualityType");
  v[3]=VS(cur,"tau");

  CYCLE(cur1, "Prod")
   { //find the (observed) maximum in respect of IdDCh, excluding the non viable, or already removed, options
    v[7]=VS(cur1,"app");
    if(v[7]==1)
     {//if it is still active (a potential choice)
     cur2=SEARCH_CNDS(cur1,"IdCh",v[1]); //find the ch. of the option
     v[5]=VS(cur2,"obs_x");//read its observed value
     if(v[5]<0)
      v[5]=0; //negative values mess up the threshold system. Only positive values can be considered.
     if(v[27]==1) //flag to distinguish the very first firm from the other ones. The first is considered the best
      {
       v[27]=0;//set the flag of the first to signal the following ones 
       v[6]=v[5]; //this is the first maximum
      } 
     if(v[5]*v[25]>v[6]*v[25])//check if it is the best, lowest for neg. characteristics and highest for positive ones
      v[6]=v[5];//record in v[6] the best (observed) value
     WRITES(cur1,"curr_x",v[5]); //write the observed value
     } 
    else 
     WRITES(cur1,"curr_x",-1); //write a default value for non-active options
   }
  //now we know the maximum, stored in v[6]*v[25] 
  CYCLE(cur1, "Prod")
   { //second cycle: remove options below maximum * tau
    v[7]=VS(cur1,"app");//is the firm still a potential choice?
    if(v[7]==1)
    {//yes
     v[8]=VS(cur1,"curr_x");//observed
     if(v[25]==-1)//if the ch. is a negative one
      v[33]=2-v[3];//yes, the tolerance is 2-tau since it is turned into neg. value (e.g. 90% -> 110%
     else
      v[33]=v[3]; 
     if(v[8]*v[25]<v[6]*v[33]*v[25])//test the threshold of tolerance: it the product is worse than tau% of the maximum it is discarded
      {//too low value: remove
       WRITES(cur1,"app",0);//mark the product as discarded
       v[0]--; //decrease the products still available.
      }
    }//end of the second cycle
   
   
   }//end of the cycle across characteristics.
   
//INTERACT("Subsequ", v[0]); //uncomment this line to interrupt the simulation after the choice
 }
if(v[0]==0)//this is not possible
 INTERACT("No firms left",v[0]);//error control: v[0] must be >=1

INCR("CounterChUsed",v[30]); 
cur=RNDDRAW("Prod","app"); //choose randomly among the firms still viable.

v[0]=VS(cur,"IdProd");

RESULT(v[0] )


/***********************
Adopt
************************/
FUNCTION("Adopt") 
/*
Install the ch's values in the agent's place
*/
    
v[0]=VS(c,"IdChoice");
cur=SEARCH_CND("IdProd",v[0]);

CYCLES(c, cur1, "AgCh")
 {
  v[1]=VS(cur1,"AIdCh");
  cur2=SEARCH_CND("IdCh",v[1]);
  v[2]=VS(cur2,"Value");
  WRITES(cur1,"AValue",v[2]);
 }

RESULT(1)

/***********************
SetARank
************************/
FUNCTION("SetARank") 
/*
Assign the consumer's preferences.
*/
v[2]=V("RangeRank");

//Set App in each Ch and each Prod equal to the actual ranking times MSInstall
//with a RND equal to RangeRank
v[3]=V("Delta");

//v[3]=c->up->cal("Delta",0);
CYCLE(cur, "Prod")
 {v[0]=cur->cal("MSInstall",1);
  v[1]=p->cal("MinProb",0);
  if(v[0]<v[1])
   v[0]=v[1];
  for(cur1=cur->search("Ch"); cur1!=NULL; cur1=go_brother(cur1) )
   {v[34]=pow( cur1->cal("Rank",0)*v[0]*(1 + v[2]*(RND-0.5)),v[3]);
    cur1->write("App",v[34], 0);

   } 
 }
//Init to zero the ARank
for(cur=c->search("AgCh"); cur!=NULL; cur=go_brother(cur) )
 cur->write("ARank",0,0);

//Set the AProb
for(cur1=c->search("AgCh"); cur1!=NULL; cur1=go_brother(cur1) )
 {
  v[17]=VS(cur1,"AIdCh");
  CYCLE(cur2, "Prod")
   {cur=cur2->search_var_cond("IdCh",v[17],0);
    cur1->increment("AProb",cur->cal("App",0));
   }
 }

if(debug_flag==1)
  deb(c, c, "Observe", &res);

//Set final ranking: 1 the most important, 2 the second and so on.
for(v[0]=1, cur=p->search("Ch"); cur!=NULL; cur=go_brother(cur), v[0]++ ) //simply a counter
 {
//  if(t==94)
  // plog("bah");
  cur1=c->draw_rnd("AgCh", "AProb", 0);
  if(cur1->cal("AProb",0)==0)
  { plog("Warning: draw item with prob==0\nInit data error\n");
    res=-1;
    deb(cur1, c, "Error", &res);
  }

  cur1->write("AProb",0,0);
  cur1->write("ARank",v[0], 0);
 }



c->lsdqsort("AgCh", "ARank","UP");
res=1;
goto end;
}

/***********************
ChooseTTB
************************/
FUNCTION("ChooseTTB") 

/*
Apply the Take-The-Best strategy, or lexicographic preferences to the decision of which product to buy.
All existing products are initially scanned for a potential choice. Preferences are taken from the consumer who requested the computation for this equation (c->).
*/


//Init all products
for(cur=p->search("Prod"); cur!=NULL; cur=go_brother(cur))
 cur->write("Mark",1,0);

v[3]=p->cal("Tolerance",0);//it is the minimum percentage of the max value
//that a ch's value must have to avoid be discarded; ex. Tolerance = 0.8 means
// that a ch's value less than 0.8 of the maximum will be discarded

v[5]=c->cal("Deviation",0);
//Deviation is the width of probability when reading the characteristics' values.
//the ch's value are drawn by a random normal distribution whose mean is the
//actual value of ch. and the deviation is Deviation

v[10]=0; //this is the counter of the number of characteristic necesary to make a decision
//For every characteristics
for(v[0]=2,cur1=c->search("AgCh"); cur1!=NULL && v[0]>1; cur1=go_brother(cur1), v[10]++ )
 {v[1]=cur1->cal("AIdCh",0);
   //Set every product
  for(cur=p->search("Prod"); cur!=NULL; cur=go_brother(cur))
    {if(cur->cal("Mark",0)==1)
     {
      cur2=cur->search_var_cond("IdCh",v[1],0);
      v[6]=norm(v[7]=cur2->cal("Value",0),v[5]);
      if(v[6]<0)
      {sprintf(msg, "Warning: Value observed below zero\nMean %lf, Dev %lf, Norm %lf\n", v[7], v[5], v[6]);
       plog(msg);
        v[6]=0;
      }

      cur->write("PApp",v[6], 0);
     }
     else
      cur->write("PApp",0,0);
    }
  p->lsdqsort("Prod","PApp","DOWN");
  cur2=p->search("Prod");
  v[2]=cur2->cal("PApp",0);
  if(v[2]==0)
  {f=fopen("log.log","a");
   fprintf(f,"Merda. Division by zero in TTB\n");
   plog("Merda. Division by zero in TTB\n");
   INTERACT("Division by 0 ttb", 1);
   fclose(f);
  }

  //Kill worst products
  for(v[0]=1,cur2=cur2->next; cur2!=NULL;cur2=go_brother(cur2) )
   {v[4]=cur2->cal("PApp",0);
    if(v[4]/v[2]>=v[3])
     v[0]++;
    else
     cur2->write("Mark",0,0);
   }

  p->lsdqsort("Prod","IdProd","UP");
 }


p->increment("CounterChUsed",v[10]);


cur=p->draw_rnd("Prod","Mark",0);
cur->increment("ChUsed",v[10]);
res=cur->cal("IdProd",0);

goto end;

}

  

if(!strcmp(label, "Init") )
{
/*
Technical initialization function. It is computed only once and then it is
transformed in a parameter and never computed again.

Sets the global point 'prod' pointing to the first products, so
to speed up the access to these objects

Defines a vector of pointers so that objects 'Agent' can be accessed quickly
*/




param=1;
//prod=p->search("Prod");
l=(object **)malloc(sizeof(object *)*MAX_AGENT);
cur=p->search("Agent");
l[0]=cur;
res=1;
goto end;
}


/*****************************
MSInstall
******************************/
if(!strcmp(label, "MSInstall") )
{
v[0]=p->cal("InstallBase",0);
v[1]=p->up->up->cal("TotAgent",0);
res=v[0]/v[1];
goto end;
}

/*****************************
Action
******************************/
if(!strcmp(label, "Action") )
{
/*
Actual core of the model, as demand is concerned. The function searches for
any existing agent that is "ready" to trigger the birth of another agent. When
found, a new agent is added to the model. The new agent is initialized as necessary and starts purchasing a product every given (random) periods.

New agents can introduce new agents, but each generation decreases the number of agents that can introduce.


*/

//When no more agents can be created these lines prevent the equation to be
//(uselessly) executed
if(p->cal("NoNewAgent",0)==1)
 {
  res=0;
  goto end;
 }

 v[0]=p->cal("TimeActive",0);

//The initialization of the very first agent at time of the first time step
if(p->cal("IdAgent",0)==-1)
 {cur=p->search("Agent");
  l[0]=cur;
  last=cur;
  first=cur;
  cur->write("IdAgent", p->cal("IssuerId",0),0);
  cur->write("Deviation",p->cal("MaxDeviation",0),1);
  cur->write("IdChoice",v[10]=p->cal(cur,"Choose",0),0);
  p->cal(cur, "Adopt",0);
  cur->write("MaxCont",p->cal("NumContagion",0),0);
  v[13]=rnd_integer(v[0],0);
  cur->write("Active",v[13],0);

  //Add to the supply statistics
  cur1=p->search_var_cond("IdProd",v[10],0);
  cur1->increment("Num",1);
  
 }
v[20]=p->cal("MaxDeviation",0);


v[16]=p->cal("RadioReduction",0);
for(v[30]=0,v[12]=0,cur=first; cur!=NULL; cur=cur->next )
 {
  if( (cur->cal("CurrCont",0))<(v[6]=cur->cal("MaxCont",0)) )
   { if(v[30]==0)
      { v[13]=cur->cal("IdAgent",0);
        first=l[(int)(v[13]-1)];
      }
     v[30]=1;
   if( (cur->cal("Active",0))==1)
    {
     v[12]++;
     v[13]=rnd_integer(v[0],0);
     cur->write("Active",v[13],0);
     cur1=SEARCH("Group");
     cur1=ADDOBJS_EX(cur1,"Agent",cur);
     //cur1=p->son->add_an_object("Agent",cur); //added from Group
     cur1->write("Deviation",v[20],t);
     cur1->write("Date",(double)t, 0);
     cur1->write("IdAgent",v[10]=p->cal("IssuerId",0),0);
     if(v[10]>=MAX_AGENT)
      {plog("Error: to many Agent's. Change MAX_AGENT value in the function file\n");
       quit=2;
       res=0;
       goto end;
      }
     l[(int)(v[10]-1)]=cur1;
     last=cur1;

     cur1->write("CurrCont",0,0);
     v[13]=rnd_integer(v[0],0);
     cur1->write("Active",v[13],0);

     v[5]=max(0,(cur->cal("MaxCont",0)-v[16]));
     cur1->write("MaxCont",v[5],0);
     cur->increment("CurrCont",1);


     cur1->write("IdChoice",v[31]=p->cal(cur1, "Choose",0),0);
     p->cal(cur1, "Adopt",0);
     cur2=p->search_var_cond("IdProd",v[31],0);
     cur2->increment("Num",1);

    }
   else
    cur->increment("Active",1);
  }

 }

if(v[30]==0)
  {cur=SEARCH("Group");
   cur->write("NoNewAgent",1,0);
  } 
 //p->son->write("NoNewAgent",1,0);
 
res=v[12];
goto end;
}




/*****************************
TotAgent
******************************/
if(!strcmp(label, "TotAgent") )
{
/*
Total number of agents in the model
*/
res=val[0]+p->up->cal("Action",0);
goto end;
}

/*****************************
IssuerId
******************************/
if(!strcmp(label, "IssuerId") )
{
/*
Provides identification numbers for new entering buyers
*/

last_update--;
if(c==NULL)
 {res=val[0];
 goto end;
 }
res=val[0]+1;
goto end;
}


/*****************************
IssuerIdProd
******************************/
if(!strcmp(label, "IssuerIdProd") )
{
/*
Provides identification numbers for new entering producers
*/

last_update--;
if(c==NULL)
 {res=val[0];
 goto end;
 }
res=val[0]+1;
goto end;
}


/*****************************
InvHerfindal
******************************/
if(!strcmp(label, "InvHerfindal") )
{
/*
(inverse) concentration index.
*/
for(v[0]=0,cur=p->search("Prod"); cur!=NULL; cur=go_brother(cur) )
  {
  v[1]=cur->cal("MSInstall",0);
  v[0]+=v[1]*v[1];
  v[2]=cur->cal("Num",0);
  if(v[2]!=0)
    cur->multiply("ChUsed",1/v[2]);
  }
if(v[0]==0)
 res=0;
else
 res=1/v[0];
goto end;

}

/*****************************
Choose
******************************/
if(!strcmp(label, "Choose") )
{

/*
Decisional function used by new entering buyers only. Creates preferences and then calls the usual ChooseTTB

*/
last_update--;
res=-2;
if(c==NULL)
 goto end;
p->cal(c, "SetARank",0);
res=p->cal(c,"TTB",0);
goto end;
}




/*****************************
MS
******************************/
if(!strcmp(label, "MS") )
{
/*
Market shares of daily sales (there are also the shares of install bases).
The equation computes also the percentage (over install bases) of new buyers and of defectors.
*/
v[0]=p->cal("TotNum",0);
v[1]=p->cal("Num",0);
v[2]=p->cal("Cancelled",0);
v[3]=p->cal("InstallBase",0);
if(v[3]>0)
  {p->write("PercNew",v[1]/v[3], 0);
   p->write("PercCan",v[2]/v[3],0);
  }
else
  {p->write("PercNew",0, 0);
   p->write("PercCan",0,0);
  }


if(v[0]==0)
 res=0;
else
 res=v[1]/v[0];
goto end;
}

/*****************************
InstallBase
******************************/
if(!strcmp(label, "InstallBase") )
{
/*
Install base for a product is modified by new buyers (+) and by defectors (-)
*/
v[1]=p->cal("Num",0);
v[0]=p->cal("Cancelled",0);

res=val[0]+v[1]-v[0];
goto end;
}


/*****************************
ResetNum
******************************/
if(!strcmp(label, "ResetNum") )
{
/*
Clean up parameters used to store statistics during a single time step
*/
for(cur=p->search("Prod"); cur!=NULL; cur=cur->next)
 {cur->write("Num",0,0);
  cur->write("Cancelled",0,0);
  cur->write("ChUsed",0,0);

 }
cur=p->search("Supply");
cur->write("CounterChUsed",0, 0);

res=0;
goto end;
}

if(!strcmp(label, "AvChUsed"))
{
/*
Average number of characteristics used during the decision algorithm
*/
v[0]=p->cal("CounterChUsed",0);
v[1]=p->cal("TotNum",0);
if(v[1]>0)
 res=v[0]/v[1];
else
 res=0; 
goto end;
}


/*****************************
TotNum
******************************/
if(!strcmp(label, "TotNum") )
{
/*
Total sales
*/
p->cal("ActionReplace",0);
res=p->sum("Num",0);
goto end;
}

/*****************************
Deviation
******************************/
if(!strcmp(label, "Deviation") )
{
/*
Deviation used when "reading" the correct values of products' characteristics.
The higher CoeffDeviation (in [0,1]) the faster the convergence to MinDeviation. =1 =>Deviation=MinDeviation immediately; 
=0 Deviation never change.
*/
v[0]=p->up->cal("CoeffDeviation",0);
v[1]=p->up->cal("MinDeviation",0);
res=val[0]+v[0]*(v[1]-val[0]);
//if(t==25)
 //INTERACT("Che cazzo", res);

goto end;
}

/*****************************
TicTolerance
******************************/
if(!strcmp(label, "TicTolerance") )
{
/*
Change tolerance values. If this Variable is not inserted in the model Tolerance never change.
Actually, this Variable is used only rarely.
*/
if(val[0]<0)
 {res=100;
  p->increment("Tolerance",0.01);
 } 
else
 res=val[0]-1;

goto end;
}

/*****************************
ActionReplace
******************************/
if(!strcmp(label, "ActionReplace") )
{
/*
Scans all buyers and manage the statistic collection for buyers (if the actually buy something).
*/
v[1]=p->cal("TimeRepl",0);

v[4]=p->cal("TotAgent",0);
for(v[12]=v[29]=v[28]=0,cur=l[0]; cur!=NULL; cur=cur->next )
 {v[29]+=cur->cal("Deviation",0);
  v[28]++;
  v[0]=cur->cal("Date",0);
  if(v[0]+v[1]+RND*v[1]<(double)t)
   {
    v[7]=p->cal(cur, "TTB",0);
    cur->write("Date",(double)t,0);
    v[5]=cur->cal(cur, "IdChoice",0);
    if(v[7]!=v[5])
     v[12]++;
    cur1=p->search_var_cond("IdProd",v[5],0);
    cur1->increment("Cancelled", 1);
    cur1=p->search_var_cond("IdProd",v[7],0);
    cur1->increment("Num",1);
    cur->write("Date",(double)t,0);
    cur->write("IdChoice",v[7],0);
    p->cal(cur, "Adopt",0);

   } //end IF time to replace
 } //end FOR cycle through agents
p->write("AvDeviation",v[29]/v[28], 0);
res=v[12];
goto end;
}

sprintf(msg, "\nFunction for %s not found", label);
plog(msg);
quit=2;
return -1;


end :
if( (isnan(res)==1 || isinf(res)==1) && quit!=1)
 { 
  sprintf(msg, "At time %d the equation for '%s' produces the non-valid value '%lf'. Check the equation code and the temporary values v\\[...\\] to find the faulty line.",t, label, res );
  error(msg);

  debug_flag=1;
  debug='d';
 } 

if(debug_flag==1)
 {
 for(i=0; i<40; i++)
  i_values[i]=v[i];
 }

return(res);
}




object *turboskip(int n)
{
register object *c;

return l[n-1];
register int i;
i=n/500;
c=l[i];
i=500*i;
for(; i<n; c=c->next, i++);

return c;
}

void close_sim(void)
{
free(l);
}

