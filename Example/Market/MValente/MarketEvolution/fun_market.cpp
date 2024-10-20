/***************************************************
****************************************************
LSD Model - October 1999
written by Marco Valente
Aalborg University

Comments and bug reports to mv@business.auc.dk
****************************************************
****************************************************/



/*****************************************************
This function file uses the CHOOSE Variable, allowing buyers to scan all the
products and all the characteristics, using a complete TAKE_THE_BEST strategy

*/
//#include "../src/fun_head.h"
#include "fun_head.h"
object *prod;
object *techno;
object *turboskip(int n);
int *prodlist;
object **l;
#define MAX_AGENT 30000
object *last, *first;

double variable::fun(object *caller)
{
//These are the local variables used by default
double v[100], res;
object *p, *c, *cur1, *cur2, *cur3, *cur4, *cur5;
variable *cv;

//Declare here any other local variable to be used in your equations
//You may need an integer to be used as a counter
int i, j;
//for(i=0; i<100; i++)
 //v[i]=0;
//and an object (a pointer to)
object *cur;


if(quit==2)
 return -1;

p=up;
c=caller;
FILE *f;


//Uncommenting the following lines the file "log.log" will
//contain the name of the variable just computed.
//To be used in case of unexpected crashes. It slows down sensibly the simulation
/**
f=fopen("log.log","a");
 fprintf(f,"t=%d %s\n",t,label);
 fclose(f);
**/
//Place here your equations. They must be blocks of the following type



/*****************************
NumProd
******************************/
if(!strcmp(label, "NumProd") )
{
/*
Number of producers currently on the market
*/
for(v[0]=0,cur=p->search("Prod"); cur!=NULL; cur=cur->next,v[0]++);
res=v[0];
goto end;
}
 

/***********************
Adopt
************************/
if(!strcmp(label, "Adopt") )
{
/*
Install the ch's values in the agent's place. Technical stuff used immediately
after an Agent makes a purchase.
*/
last_update--;

if(c==NULL)
 {
  res=-1;
  goto end;
 }

v[0]=c->cal("IdChoice",0);
cur=prod->up->search_var_cond("IdProd",v[0],0);
for(cur1=c->search("AgCh"); cur1!=NULL; cur1=go_brother(cur1) )
 {
  cur2=cur->search_var_cond("IdCh",cur1->cal("AIdCh",0),0);
  cur1->write("AValue",cur2->cal("Value",0),0);
 }
res=1;
goto end;
}

/***********************
SetARank
************************/
if(!strcmp(label, "SetARank") )
{
/*
Assign the rankings to the newly entered agents.
The function builds indexes for each characteristic as the sum of the Rank's
from each producer for each characteristic. These values are weighted with the
market shares of the install bases of the producers. The resulting indexes
are used as probabilities for each characteristic to be placed as ranking 1, 2
and so on.
Parameters:
- RangeRank; width of the (uniform) random function used around the actual
indexes. The highest the value of RangeRank the less close are the resulting
rankings to the actual probabilities.

- ExpChoose; exponent of the power function used to adjust the indexes. The
highest the value the more likely the drawn rankings follow the higher shares
producers' rankings;

*/

last_update--;

if(c==NULL)
 {
  res=-1;
  goto end;
 }
v[2]=prod->up->cal("RangeRank",0);
//Set App in each Ch and each Prod equal to the actual ranking times MSInstall
//with a RND equal to RangeRank
v[3]=c->up->cal("ExpChoose",0);

//Cycle for each producer
for(cur=prod; cur!=NULL; cur=go_brother(cur))
 {v[0]=cur->cal("MSInstall",1);
  v[1]=prod->cal("MinProb",0);
  if(v[0]<v[1])
   v[0]=v[1];//minimal weight assigned to 0 shares producers
//Cycle for each characteristic of each producer, assigning its prob. contribution
//as a function of the Rank and of the market share.
  for(cur1=cur->search("Ch"); cur1!=NULL; cur1=go_brother(cur1) )
   cur1->write("App",pow( cur1->cal("Rank",0)*v[0]*(1 + v[2]*(RND-0.5)),v[3]), 0);
 }

//Init to zero the ARank of the agents
for(cur=c->search("AgCh"); cur!=NULL; cur=go_brother(cur) )
 cur->write("ARank",0,0);

//Set the AProb summing up the contribution of each producer for each
//characteristic
for(cur1=c->search("AgCh"); cur1!=NULL; cur1=go_brother(cur1) )
 {cur1->write("AProb",0,0);
  for(cur2=prod; cur2!=NULL; cur2=go_brother(cur2) )
   {cur=cur2->search_var_cond("IdCh",cur1->cal("AIdCh",0),0);
    cur1->increment("AProb",cur->cal("App",0));
   }
 }

res=1;
//Set final ranking: 1 the most important and downward
v[10]=v[0]=prod->up->cal("NumCharacteristic",0);
for( cur=prod->search("Ch"); cur!=NULL; cur=go_brother(cur), v[0]-- ) //simply a counter
 {
  cur1=c->draw_rnd("AgCh", "AProb", 0);
  if(cur1->cal("AProb",0)==0)
  { plog("Warning: draw item with prob==0\nInit data error\n");
    res=-1;
    //deb(cur1, c, "Error", &res);
  }

  cur1->write("AProb",0,0);
  cur1->write("ARank",v[0]/v[10], 0);
 }

//Sort the AgCh Object incrementally according to the ARank values
c->lsdqsort("AgCh", "ARank","DOWN");
res=1;
goto end;
}

/***********************
ChooseTTB
************************/
if(!strcmp(label, "ChooseTTB") )
{

/*
The basic choice routine used by Agents to choose which product to buy.
It is based on the Take-The-Best algorithm, which is an implementation of the
lexicographic preferences. Basically, preferences are composed by a ranking
of the product's characteristic. The agent scans all the products comparing their
values on the highest ranking characteristic. If only one product scores highest,
this is chosen. If more than one product score similarly on that characteristic,
the others are deleted and the second characteristic is used to choose among the
remaining. The process continues using all the characteristics until one single
product remains. If no more characteristics are available and there are still
more than one choice, a uniform random draw is performed.

Relevant parameters:
- Learning, DevLearn and MaxLearn. Each agent has a Learning value (from 0 to 1)
When reading the value of products' characteristic the agent actually draws a value
from a normal function whose mean equals the actual characteristic value and whose
StdDev equals:
(1-Learning*MaxLearn)*DevLearn
Learning varies through time independently for each agent.
MaxLearn is the percentage of capacity of learning. If it is 0, agents never learn
and continue to make mistakes with range DevLearn. If it is 1, eventually agents
don't make mistakes anylonger (when Learning reaches 1).
DevLearn depends on the unit of measure of the characteristics.

- Tolerance. It is the percentage of the highest value of the characteristic that
is allowed for other products' not to be cancelled. If Tolerance is 1 (or higher)
the product scoring even slightly lower than the best one is discarded. If Tolerance
is 0.8, products scoring more than 80% of the value of the best product are not
discarded. In practice, Tolerance determines what percentage of best values is
considered identical.


*/
last_update--;

if(c==NULL)
 {
  res=-1;
  goto end;
 }



//Init all products as potentially candidate for the choice
for(cur=prod; cur!=NULL; cur=go_brother(cur))
 cur->write("Mark",1,0);

v[3]=prod->up->cal("Tolerance",0);//it is the minimum percentage of the max value
//that a ch's value must have to avoid be discarded; ex. Tolerance = 0.8 means
// that a ch's value less than 0.8 of the maximum will be discarded

v[8]=c->up->cal("MaxLearn",0);
v[5]=(1-c->cal("Learning",1)*v[8]);
//Learning is the complement to one of the deviation of learning. It means that
//the ch's value are drawn by a random normal distribution whose mean is the
//actual value of ch. and the deviation is DevLearn, when the buyer did never
//used the product before and 0 when the buyer learn "perfectly" about the product.

//For every characteristics the whole set of products is scannned. The ones
//having Mark=0 are discarded. The temporary variable v[0] contained the number
//of Prod still eligible. When it becomes 1, the function returns the unique
//remaining product (v[0] is initialized to 2 just to allow the cycle to start)
//v[10] is the counter of the number of characteristics necessary for reaching
//a solution.
for(v[0]=2,v[11]=0,cur1=c->search("AgCh"); cur1!=NULL && v[0]>1; cur1=go_brother(cur1),v[11]++ )
 {v[1]=cur1->cal("AIdCh",0); //Id of the Ch
  cur2=techno->search_var_cond("TGIdCh",v[1],0);
  v[10]=cur2->cal("DevLearn",1)*v[5];
   //Set every product
  for(cur=prod; cur!=NULL; cur=go_brother(cur))
    {if(cur->cal("Mark",0)==1) //if the product is still eligible
     {
      cur2=cur->search_var_cond("IdCh",v[1],0); //search the currently used characteristic
      v[6]=norm(v[7]=cur2->cal("Value",0),v[10]);//draw the random value
      if(v[6]<0)
      {PLOG("Warning: Value observed below zero\nMean %lf, Dev %lf, Norm %lf\n", v[7], v[10], v[6]);
        v[6]=0; //negative values may cause problems
      }
      cur->write("PApp",v[6], 0);//The producer's value
     }
     else
      cur->write("PApp",0,0);//the case in which the product has already been discarded (Mark=0)
    }
  prod->up->lsdqsort("Prod","PApp","DOWN"); //Sort producers descending from the highest
													 // PApp
  cur3=cur2=SEARCHS(prod->up, "Prod");//both cur3 and cur2 refer to the highest scoring product
  cur4=cur2->search_var_cond("IdCh",v[1],0); //Find the characteristic in the best product
  cur4->increment("Positive",1); //reward the characteristic for allowing the product to be the best
  v[2]=cur2->cal("PApp",0); //take the highest value
  if(v[2]==0)
  {f=fopen("log.log","a");
   fprintf(f,"Merda. Division by zero in TTB\n");
   plog("Merda. Division by zero in TTB\n");
   fclose(f);
   //deb(p, p, "Division by 0", &res);
  }

  //Kill worst products. v[0] is set to 1 because the best product is already
  //computed.
//Reward also the characteristic that allow a product to pass the turn

  for(v[0]=1,cur2=cur2->next; cur2!=NULL;cur2=go_brother(cur2) )
   {v[4]=cur2->cal("PApp",0);
    if(v[4]/v[2]>=v[3]) //if the ratio between the product's value and the best
     {v[0]++; // is at least as Tolerance, the product is not deleted.
      cur5=cur2->search_var_cond("IdCh",v[1],0);
      cur5->increment("Positive",1); //reward the characteristic
     }
    else
     {cur2->write("Mark",0,0); //Otherwise it is deleted
      cur5=cur2->search_var_cond("IdCh",v[1],0);
      cur5->increment("Negative",1); //punish the characteristic
     }
   }
  //re-sort the products according to their natural order
  prod->up->lsdqsort("Prod","IdProd","UP");
 }


if(v[0]==1) //if only one product remained, this is the one
  res=cur3->cal("IdProd",0);
else
 res=prod->up->cal(c,"TieBreak",0); //otherwise, tiebreak

//update the index tracking the number of characteristics used
//to reach a solution
p->write("IndexChUsed", p->cal("IndexChUsed",0)*0.99 + 0.01*v[11],0);

goto end;

}

/***********************
TieBreak
************************/
if(!strcmp(label, "TieBreak") )
{
/*
This function makes the choice for buyers who used all the characteristics
and still couldn't make their choice.
The function chooses randomly a characteristics with probability equal
the the ARank. Using the values of this characteristic it draws randomly
among the product not yet discarded.
*/
last_update--;

if(c==NULL)
 {
  res=-1;
  goto end;
 }

cur1=c->draw_rnd("ACh","ARank",0);
v[1]=cur1->cal("AIdCh",0);
v[2]=p->cal("ExpTieBreak",0);
for(cur=prod; cur!=NULL; cur=cur->next)
 {
  if(cur->cal("Mark",0)==1) //if the product is still eligible
  {
  cur1=cur->search_var_cond("IdCh",v[1],0);
  v[3]=pow(cur1->cal("Value",0),v[2]);
  cur->write("Mark",v[3],0);
  cur1->increment("Negative",1);
  }
 }
cur2=p->draw_rnd("Prod","Mark",0);
cur3=cur2->search_var_cond("IdCh",v[1],0);
cur3->increment("Negative",-1);
res=cur2->cal("IdProd",0);
goto end;

 }

/***********************
ChUsed
************************/
if(!strcmp(label, "ChUsed") )
{
/*
Index reporting the number of users that have this ch. as their major concern.

The computation is made using the temporary parameter Counter (which is
filled in during the equation for replacing), so that
*/
res=val[0]*0.8 + 0.2*p->cal("Counter",0);
p->write("Counter", 0,0);
goto end;
}
/***********************
Entry
************************/
if(!strcmp(label, "Entry") )
{

/*
Function governing the entry of new producers. A new product enters at each time
step with probability ProbEntry.
The new product is initialized as necessary. See InitializeCh for the relevant
initializations
*/
v[0]=p->cal("ProbEntry",0);
if(RND>v[0])
 {res=0;
  goto end;
 }
//cur=p->add_an_object("Prod",p->son);
cur=ADDOBJ_EX("Prod", SEARCH("Prod"));

cur->write("MS",0,t);
v[0]=p->cal("IssuerIdProd",0);
cur->write("IdProd",v[0],0);
cur->write("Num",0,0);
cur->write("Cancelled",0,0);
cur->write("Mark",0,0);
cur->write("Losses",0,0);
cur->write("InstallBase",0,t);
cur->write("MSInstall",0,t);
cur->write("RD",0,t);
cur->write("Birth",(double)t,0);
p->cal(cur,"InitializeCh",0);
res=1;
goto end;
}

/***********************
InitializeCh
************************/
if(!strcmp(label, "InitializeCh") )
{

/*
Function used when a new producer enters in the market. It initialized the
characteristics' values drawing randomly around the maximum values reached up to this
time.
The approach is to allow new entrants to reach no more than the maximum obtained
by incumbents. This disadvantage is compensated by the capacity to "mix" different
threads of innovation.
The function is composed by two main cycles. In the first, the new entrant draws
randomly the values of each characteristic, fishing in a range of the maximum values.
The second cycle pushes up the values of the lowest characteristics in case they
provoke a violation of the epistasis' binding.
The values are exctracted uniformely from the interval [MaxValue-X, MaxValue].
The coefficient X determines the width of the interval. It is computed as follows:
X=RangeInn*Range
Range is a slowly updated index tracking MaxValue. That is, if it is long time
that MaxValue remained at the same values, then Range is very close to it, and new
entrants will likely fish pretty high values (basically, it assumes that imitation
is easier after some time has passed).


*/
last_update--;
if(c==NULL)
 {res=0;
  goto end;
 }


v[0]=p->cal("NumCharacteristic",0);

/*
cur is the example producer
c is the new entrant
cur1 are the characteristics of example
cur2 are the characteristics of new entrant
*/
v[4]=p->cal("MinCapability",0);
v[13]=p->cal("RangeInn",0);
//for(v[2]=1, cur2=c->son; cur2!=NULL; cur2=cur2->next, v[2]++)
for(v[2]=1, cur2=SEARCHS(c, "Ch"); cur2!=NULL; cur2=cur2->next, v[2]++)
  {
    cur3=techno->search_var_cond("TGIdCh",v[2],0);
    v[5]=cur3->cal("MaxValue",0);
    v[14]=cur3->cal("Range",0);
    v[7]=v[5]-v[13]*(v[5]-v[14])*RND;
    cur2->write("Value",v[7],0);

  cur2->write("Rank",1/v[0],0);
  cur2->write("RDIndex",1/v[0],0);
  cur2->write("Positive",0,0);
  cur2->write("Negative",0,0);
  cur2->write("Capability",v[4],t);
//  cur2->write("UpperBound",0,t-1);
  cur2->write("UpperBound",0,t);
  cur2->recal("UpperBound");
  }

for(v[8]=1,v[9]=0; v[8]==1 && quit!=2; v[9]++)
{v[8]=0;
// for(v[10]=1,cur2=c->son; cur2!=NULL; cur2=cur2->next,v[10]++)
 for(v[10]=1,cur2=SEARCHS(c,"Ch"); cur2!=NULL; cur2=cur2->next,v[10]++)
 {
  v[7]=cur2->cal("UpperBound",0);
  v[6]=cur2->cal("Value",0);
  if(v[6]>v[7])
   {v[8]=1;
    cur1=techno->search_var_cond("TGIdCh",v[10],0);
    for(v[11]=1; v[11]<=v[0];v[11]++)
     {if(v[10]!=v[11])
      { cur3=cur1->search_var_cond("LIdCh",v[11],0);
        v[12]=cur3->cal("EpiValue",0);
        cur4=c->search_var_cond("IdCh",v[11],0);
        v[13]=cur4->cal("Value",0);
        if(v[13]*v[12]<v[6])
         cur4->write("Value",v[6]/v[12]+0.000001,0); //rounding mess
       }
     }

   }
//  cur2->write("UpperBound",v[7],t-1);
  cur2->write("UpperBound",v[7],t);
  cur2->recal("UpperBound");
 }
}
//for(cur2=c->son; cur2!=NULL; cur2=cur2->next)
for(cur2=SEARCHS(c, "Ch"); cur2!=NULL; cur2=cur2->next)
 cur2->cal("UpperBound",0);
res=1;
goto end;
}


if(!strcmp(label, "Init") )
{
/*
Technical initialization function. It is computed only once and then it is
transformed in a parameter and never computed again.

Sets the global point 'prod' pointing to the first products, so
to speed up the access to these objects

On the same token, declares a pointer 'techno' the the TechnoGod Object to
access it quickly.

Defines a vector of pointers so that objects 'Agent' can be accessed quickly
using the vector index instead of scanning the whole bunch. Note that the memory
so allocated must be removed at the end of the simulation (see C++ function
void close_sim(void)
at the end of the equation source file.

*/
prod=p->search("Prod");
techno=p->search("TechnoGod");
l=(object **)malloc(sizeof(object *)*MAX_AGENT);
res=1;
param=1;
goto end;
}


/*****************************
MSInstall
******************************/
if(!strcmp(label, "MSInstall") )
{
/*
Market shares of install bases, computed as the ratio of InstallBase over the
number of agents
*/
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
Actual core of the model when the market is expanding. The function searches for
any existing agent that is "ready" to trigger the birth of another agent. When
found, a new agent is added to the model. The new agent is assiged:
- an identification number
- a product chosen (obtained with the equation Choose)
- the number of future contacts, that is, the number of new agents it is able to
  contact in the future
- a counter, set to a random number (from TimeActive (<0) to -1) that is the time
  steps to wait before it can "generate" a new agent
- other initializations

The counter of new agent's the parent can create is reduced of
one unity and its time counter is reset to TimeActive, so that it will wait
some time before generating another offspring, if it is allowed so.

As technical "trick", buyers's addresses are stored in a vector so that
buyers who have exhausted the number of offspring are not scanned again and again.
*/

//When no more agents can be created these lines prevent the equation to be
//(uselessly) executed again and again if no more agents enters in the market

if(p->cal("NoNewAgent",0)==1)
 {
  res=0;
  goto end;
 }

//Maximum time that an agent generating an offspring has to wait
//before generating another one
v[0]=p->cal("TimeActive",0);

//The initialization of the very first agent at time of the first time step
if(p->cal("IdAgent",0)==-1)
 {cur=p->search("Agent");
  l[0]=cur;
  last=cur;
  first=cur;
  cur->write("IdAgent", p->cal("IssuerId",0),0);
  cur->write("IdChoice",v[10]=prod->cal(cur,"Choose",0),0);
  prod->up->cal(cur, "Adopt",0);
  cur->write("MaxCont",p->cal("NumContagion",0),0);
  v[13]=rnd_integer(v[0],0);
  cur->write("Active",v[13],0);

  //Add to the supply statistics
  cur1=prod->search_var_cond("IdProd",v[10],0);
  cur1->increment("Num",1);
 }


/*
Decrement of the number of offsprings. If an agent is generated from a parent
who can generate X offspring, its number of offsprings will be:
X-RadioReduction
*/
v[16]=p->cal("RadioReduction",0);

//Cycle through all the agents. The cycle starts from 'first', which is the first
//agent that has still to produce an offspring. The use of 'first' does not
//have any effect, but to speed up the execution
for(v[30]=0,v[12]=0,cur=first; cur!=NULL; cur=cur->next )
 {
  if( (cur->cal("CurrCont",0))<(v[6]=cur->cal("MaxCont",0)) )
   {//keeps updated 'first'
    if(v[30]==0)
      { v[13]=cur->cal("IdAgent",0);
        first=l[(int)(v[13]-1)];
      }
     v[30]=1;
   if( (cur->cal("Active",0))==1)
    {//case in which the current agent produces an offspring
     v[12]++;
     v[13]=rnd_integer(v[0],0);
     cur->write("Active",v[13],0); //set the time for the next generation
//     cur1=p->add_an_object("Agent",cur, last); //create the offspring
     cur1=ADDOBJS_EX(cur->up,"Agent",cur);
//     cur1->write("Learning",0.01,t-1);//set the initial learning of the offspring
     cur1->write("Learning",0.01,t,1);//set the initial learning of the offspring
     cur1->recal("Learning");
     cur1->cal("Learning",0);
     cur1->write("Date",(double)t, 0); //set birthdate of the offspring
     cur1->write("IdAgent",v[10]=p->cal("IssuerId",0),0); //assign a unique ID to the offspring
     if(v[10]>=MAX_AGENT)
      {plog("Error: to many Agent's. Change MAX_AGENT value in the function file\n");
       quit=2;
       res=0; //error message due to the inadequacy of the vector
       goto end;
      }
     l[(int)(v[10]-1)]=cur1; //include the offspring in the vector
     last=cur1;

     cur1->write("CurrCont",0,0); //set to 0 the number of generated offspring of the new agent
     v[13]=rnd_integer(v[0],0);
     cur1->write("Active",v[13],0); //set the time of first generation for the new agent

     v[5]=max(0,(cur->cal("MaxCont",0)-v[16])); //set the total number of offspring for the new agent
     cur1->write("MaxCont",v[5],0);
     cur->increment("CurrCont",1); //increment the number of offspring for the parent agent

//choose the product bought by the new agent
     cur1->write("IdChoice",v[31]=prod->cal(cur1, "Choose",0),0);
//"adopt" the new product
     prod->up->cal(cur1, "Adopt",0);
     cur2=prod->search_var_cond("IdProd",v[31],0);
//notify the sale to the chosen product
     cur2->increment("Num",1);

    }
   else
    cur->increment("Active",1); //case in which the agent does not generate an offspring
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
ExpChoose
******************************/
if(!strcmp(label, "ExpChoose") )
{
/*
Exponent of the probabilities for chosing the initial preferences (see SetARank).
It can change through time setting IncrExpChoose
*/
v[0]=p->cal("IncrExpChoose",0);
res=val[0]+v[0];
goto end;
}

/*****************************
TotAgent
******************************/
if(!strcmp(label, "TotAgent") )
{
/*
Total number of agents (buyers) in the model
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
Inverse Index of concentration, computed as the inverse of the sum of the squares
of the market shares (for install bases).
The value can be read as the number competitors that, in a hypothetical market
with identically distributed shares, would provide the same concentration.
*/
for(v[0]=0,cur=prod; cur!=NULL; cur=go_brother(cur) )
  {
  v[1]=cur->cal("MSInstall",0);
  v[0]+=v[1]*v[1];
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
This function is used when a new agent makes the first purchase, since they
need to set up their preferences. Subsequently, agents use directly ChooseTTB.
*/

last_update--;
res=-2;
if(c==NULL)
 goto end;

p->cal(c, "SetARank",0);
res=p->cal(c,"ChooseTTB",0);
goto end;
}

/*****************************
RatioAbortion
******************************/
if(!strcmp(label, "RatioAbortion") )
{
/*
Ration of failed producers who never made a sale (killed before they could develop
a decent technology).
*/
v[0]=p->cal("Abort",0);
v[1]=p->cal("Pension",0);
if(v[0]+v[1]>0)
 res=v[0]/(v[0]+v[1]);
else
 res=0;
goto end;

}
/*****************************
Exit
******************************/
if(!strcmp(label, "Exit") )
{
/*
Function for deleting failing producers. A producer is removed from the market if:
- no agent is currently using its products, and
- it is more than MaxLosses time steps in which makes losses (no sales)
When a product is removed, it is increased the index Abort or Pension.
Abort keeps track of the producers who lived shorter than MaxLosses, and presumibly
did never made a single sale. Pension for the other. To facilitate results reading,
aborting producers' variable are not saved and will not appear in the result file.

*/
v[7]=p->cal("MaxLosses",0);
for(v[0]=0,cur=p->search("Prod"); cur!=NULL; )
 {
  cur1=go_brother(cur);
  v[10]=cur->cal("InstallBase",0);
  v[2]=cur->cal("MS",0);
  v[1]=cur->cal("Losses",0);

  if(v[10]==0 && v[1]>v[7])
    {
     if(prod==cur)
      prod=cur1;
     v[8]=cur->cal("Birth",0);
     if(v[7]+3>(double)t-v[8]) //abort
       {prod->up->increment("Abort",1);
        for(cv=cur->v; cv!=NULL; cv=cv->next)
          { cv->save=0; //do not keep in the data for the analysis
            delete cv->data; //because they died without producing anything
            cv->data=NULL;
          }
       }
     else
       prod->up->increment("Pension",1);

     cur->delete_obj();
     v[0]++;
    }
  cur=cur1;
 }
res=v[0];
goto end;
}

/*****************************
MS
******************************/
if(!strcmp(label, "MS") )
{
/*
Market shares of sales. It also updates the following indexes:
- percentage of new sales over install base
- percentage of cancellations over install base
- losses, indicator determining whether the period must be considered as a failure
( see exit)
*/
v[0]=p->up->cal("TotNum",0);
v[1]=p->cal("Num",0);
v[2]=p->cal("Cancelled",0);
v[3]=p->cal("InstallBase",0);
v[4]=p->up->cal("MinimumSales",0);
if(v[3]>0)
  {p->write("PercNew",v[1]/v[3], 0);
   p->write("PercCan",v[2]/v[3],0);
  }
else
  {p->write("PercNew",0, 0);
   p->write("PercCan",0,0);
  }


if(v[1]<v[4])
 p->increment("Losses",1);
else
 p->write("Losses", 0,0);
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
Number of buyers currently using the product of the producer.
It is computed adding to the previous value the new sales and subtracting
the cancellations (previous users who switched to another product.
*/
v[1]=p->cal("Num",0);
v[0]=p->cal("Cancelled",0);
res=-1;
if(val[0]+v[1]-v[0]<0)
 {
 //deb(p, p, "Observe", &res);
 }


res=val[0]+v[1]-v[0];
goto end;
}

/*****************************
SumInstallBase
******************************/
if(!strcmp(label, "SumInstallBase") )
{
/*
Control variable, checking whether the sum of installed bases as recorded
in the producers is identical to the number of current buyers.
In the process, it computes alse the average age of producers.
*/
v[0]=v[2]=v[3]=0;
v[1]=(double)t;
for(cur=prod; cur!=NULL; cur=cur->next)
 {v[0]+=v[1]-cur->cal("Birth",0);
  v[2]+=cur->cal("InstallBase",0);
  v[3]++;
 }
p->write("AvAge",v[0]/v[3],0);
res=p->sum("InstallBase",0);
//if(res!=p->up->cal("TotAgent",0))
  //deb(p, p, "Cazzo", &res);
goto end;
}

/*****************************
ResetNum
******************************/
if(!strcmp(label, "ResetNum") )
{
/*
Technical variable. Resets to 0 the values of daily sales and of non-replacing
values for each producer.
*/
for(cur=prod; cur!=NULL; cur=cur->next)
 {cur->write("Num",0,0);
  cur->write("Cancelled",0,0);

 }

res=0;
goto end;
}


/*****************************
TotNum
******************************/
if(!strcmp(label, "TotNum") )
{
p->cal("ActionReplace",0);
res=p->sum("Num",0);
goto end;
}

/*****************************
Learning
******************************/
if(!strcmp(label, "Learning") )
{
/*
Learning to be able to "read" the correct values of products' characteristics.
The higher CoeffLearn (in [0,1]) the slower the learning. =1 no learning;
=0 immediate max learning.
*/
v[0]=p->up->cal("CoeffLearn",0);
res=val[0]*v[0]+(1-v[0])*(2-val[0]);
if(res>0.9999)
 {
  res=1;
  param=1;
 }
goto end;
}
/*****************************
ProdTechStats
******************************/
if(!strcmp(label, "ProdTechStats") )
{
/*
Equation in Prod that ollects statistics on technologies.
The equation assume the producer as part of the niche indicated from the
characteristic that had higher sales (using Positive).
In this characteristic, increases the value of ProdCounter and reports
the values for all the characteristics.
*/
v[1]=p->cal("MSInstall",0);
if(v[1]==0)
 {res=0;
  goto end;
 }
p->lsdqsort("Ch","Positive","DOWN");
//if(debug_flag==1 &&debug=='d')
  //deb(p, p, "Cazzo",&res);
cur=p->search("Ch");
v[3]=cur->cal("IdCh",0);
cur5=techno->search_var_cond("TGIdCh",v[3],0);
cur5->increment("ProdCounter",v[1]);
for( ; cur!=NULL; cur=cur->next)
 { cur2=cur5->search_var_cond("LIdCh",cur->cal("IdCh",0), 0);
   v[2]=cur->cal("Value",0)*v[1];
   cur2->increment("ValCounter",v[2]);

 }
p->lsdqsort("Ch","IdCh","UP");
res=1;
goto end;
}
/*****************************
TechStats
******************************/
if(!strcmp(label, "TechStats") )
{
/*
Record statistics on the technologies for the simgle niches.
The equation writes the values of NicheVal with the values of ValCounter
(containig the sums of the values times market shares)
*/
v[0]=p->cal("MSNiche",0);
if(v[0]==0)
 {res=0;
  goto end;
 }

//for(cur=p->son; cur!=NULL; cur=cur->next)
for(cur=SEARCH("Link"); cur!=NULL; cur=cur->next)
 {v[1]=cur->cal("ValCounter",0);
  cur->write("NicheVal",v[1]/v[0],0);
  cur->write("ValCounter",0,0);
 }
res=1;
goto end;
}
/*****************************
MSNiche
******************************/
if(!strcmp(label, "MSNiche") )
{
res=p->cal("ProdCounter",0);
p->write("ProdCounter",0,0);
goto end;
}
/*****************************
ActionReplace
******************************/
if(!strcmp(label, "ActionReplace") )
{
/*
Function scanning all existing buyers and checking is they want to discard
the currently used product and make another purchase.
The decision whether to replace or not depends on the time distance from
Date, recording the last time of purchase.

In the process, several indicators are computed:
- buyers' learning is updated;
- average learning is computed;
- Characteristic's counter is increased (used to compute ChUsed)
ONLY for purchasing buyers:
- Num and Cancelled are updated, respectively, for the newly chosen product
  and for the old one.
- Preferences are updated by the chosen producers.
*/
v[1]=p->cal("TimeRepl",0);

v[4]=p->cal("TotAgent",0);
p->write("AvLearning",0,0);
for(v[12]=0,cur=l[0]; cur!=NULL; cur=cur->next )
 {v[2]=cur->cal("Learning",0);
  p->increment("AvLearning",v[2]/v[4]);
  v[0]=cur->cal("Date",0);
  v[3]=cur->cal("AIdCh",0); //finds the first characteristic's ID
  cur5=techno->search_var_cond("TGIdCh",v[3],0);
  cur5->increment("Counter",1);

  if(v[0]+v[1]+RND*v[1]<(double)t)
   {
    v[7]=prod->up->cal(cur, "ChooseTTB",0);
    cur->write("Date",(double)t,0);
    v[5]=cur->cal(cur, "IdChoice",0);
    if(v[7]!=v[5])
     v[12]++;
    cur1=prod->search_var_cond("IdProd",v[5],0);
    cur1->increment("Cancelled", 1);
    cur1=prod->search_var_cond("IdProd",v[7],0);
    cur1->increment("Num",1);
    cur->write("Date",(double)t,0);
    cur->write("IdChoice",v[7],0);
    prod->up->cal(cur, "Adopt",0);
    cur1->cal(cur, "UpdatePref",0);

   } //end IF time to replace
 } //end FOR cycle through agents


res=v[12];
goto end;
}


/*****************************
UpdatePref
******************************/
if(!strcmp(label, "UpdatePref") )
{

/*
Update preferences. This function is activated at any replacing purchase, and the
Object c-> is the current buyer who is updating the preferences, while
Object p-> is the chosen product.
The chosen product modifies (at speed determined by SpeedUpdatePref)
the importance of characteristics, that is, their ranking in the TTB
decisional process.

*/
last_update--;
if(c==NULL)
 {res=-1;
  goto end;
 }
v[0]=p->cal("SpeedUpdatePref",0);
p->lsdqsort("Ch","IdCh","UP");
c->lsdqsort("AgCh","AIdCh","UP");

//for( cur1=c->son, cur=p->son; cur1!=NULL; cur=cur->next, cur1=cur1->next)
for( cur1=SEARCHS(c,"AgCh"), cur=SEARCH("Ch"); cur1!=NULL; cur=cur->next, cur1=cur1->next)
 {

  cur1->write("ARank",cur1->cal("ARank",0)*(1-v[0])+cur->cal("Rank",0)*v[0],0);
 }
c->lsdqsort("AgCh","ARank","DOWN");

res=1;
goto end;
}

/*****************************
UpdateRank
******************************/
if(!strcmp(label, "UpdateRank") )
{
/*
This function collects the scores cumulated, for each producer, by each characteristic
when been purchased. The stronger characteristics are the ones that avoided
the product being refused during the TTB. The relative scores are used
to adjust the current values of Rank, which are used to influence the preferences
of buyers.
*/
v[0]=p->cal("SpeedRank",0);

for(cur=prod; cur!=NULL; cur=cur->next)
 {
  for(v[1]=0,cur1=cur->search("Ch"); cur1!=NULL; cur1=cur1->next)
   v[1]+=cur1->cal("Positive",0);
  if(v[1]>0)
   {
   for(cur1=cur->search("Ch"); cur1!=NULL; cur1=cur1->next)
    {
     v[2]=cur1->cal("Rank",0);
     v[3]=cur1->cal("Positive",0)/v[1];
     cur1->write("Rank",v[2]*(1-v[0])+v[0]*v[3],0);

    }
   }
 }
 res=1;
 goto end;
}
/*****************************
UpdateRDIndex
******************************/
if(!strcmp(label, "UpdateRDIndex") )
{
/*
This function computes the indexes used to determine the allocation of RD resources
to the different characteristics of the product. Each characteristic
is assigned RDIndex as a function of its own past value (updated at speed SpeedRDIndex).
The limit value is the composition of two effects: the number of Positive and Negative
for the characteristic. These are respectively the number of times that
each consumer accepted the characteristic value or rejected it during the
choice algorithm (ChooseTTB).
*/

p->cal("UpdateRank",0);

v[0]=p->cal("SpeedRDIndex",0);
v[6]=p->cal("SharePosNeg",0);
for(cur=prod; cur!=NULL; cur=cur->next)
 {cur->cal("ProdTechStats",0);
  for(v[1]=v[5]=0,cur1=cur->search("Ch"); cur1!=NULL; cur1=cur1->next)
   {v[1]+=cur1->cal("Positive",0);
    v[5]+=cur1->cal("Negative",0);
   }
  v[7]=v[1]*v[6]+v[5]*(1-v[6]);
  if(v[7]>0)
   {
   for(cur1=cur->search("Ch"); cur1!=NULL; cur1=cur1->next)
    {
     v[2]=cur1->cal("RDIndex",0);
     v[3]=(cur1->cal("Positive",0)*v[6]+cur1->cal("Negative",0)*(1-v[6]))/v[7];
     cur1->write("RDIndex",v[2]*(1-v[0])+v[0]*v[3],0);
     cur1->write("Positive",0,0);
     cur1->write("Negative",0,0);

    }
   }
 }
 res=1;
 goto end;
}

/*****************************
RD
******************************/
if(!strcmp(label, "RD") )
{
/*
Research and Development equation. The total resources available for RD are
the amount of install base. These resources are distributed along the different
characteristics depending on the importance they had for sales (see RDIndex)
The efficacy of the resources spent on developing each characteristic depends
on the capability cumulated by the producer on that aspect of the product.
The maximum value reachable on each characteristic is bounded by the
value of UpperBound for each single characteristic.

The cycle is also used to compute the index of concentration of RD, expressing
the equivalent number of characteristics on which RD is focused on, and the
index of the most exploited characteristic.
*/
v[0]=p->cal("InstallBase",0);
for(v[1]=0,cur=p->search("Ch"); cur!=NULL; cur=cur->next)
  {v[1]+=cur->cal("RDIndex",0);
   cur->cal("UpperBound",0);
  }

v[11]=p->cal("SpeedInn",0);

for(v[14]=1,v[13]=v[12]=0,cur=p->search("Ch"); cur!=NULL; cur=cur->next)
  {v[2]=cur->cal("RDIndex",0);
   v[3]=v[2]/v[1];
   if(v[3]>v[13])
    {v[14]=cur->cal("IdCh",0);
     v[13]=v[3];
    }
   v[12]+=v[3]*v[3];
   cur->write("ShareRD",v[3],t);
   v[4]=cur->cal("Capability",0);
   v[5]=v[4]*v[0]*v[3];
   v[6]=cur->cal("Value",0);
   v[7]=cur->cal("UpperBound",0);
   v[8]=v[6]*(1+v[5]);
   v[9]=min(v[8],v[7]);
   v[10]=v[6]*(1-v[11])+v[9]*v[11];
   cur->write("Value",v[10],0);
  }
p->write("RDConc",1/v[12],0);
p->write("TopChRD",v[14],0);
res=1;
goto end;
}

/*****************************
Capability
******************************/
if(!strcmp(label, "Capability") )
{
/*
Capability to exploit a given amount of RD efforts to obtain an increment of
characteristic value.
The capability starts from the value of MinCapability when a producer starts
for the first time to make RD on a given characteristic. It increases as a function
of the share of RD devoted to that characteristic up to a maximum given by
MaxCapability. The increments are slow, governed by the parameter SpeedCapability.
*/
v[0]=prod->up->cal("MaxCapability",0);
v[1]=prod->up->cal("MinCapability",0);
v[2]=prod->up->cal("SpeedCapability",0);
v[3]=p->cal("InstallBase",0);
v[6]=p->cal("ShareRD",0);
v[4]=v[3]*v[6]/1957; //gets 1 when a huge monopolist spend all RD on one single ch.

v[5]=(v[0]-v[1])*v[4];
res=(val[0]-v[1])*(1-v[2])+v[5]*v[2] + v[1];
goto end;
}

/*****************************
AvRDConc
******************************/
if(!strcmp(label, "AvRDConc") )
{
/*
Average index of RD concentration, obtained as the values of the existing firms
weighted with the market shares of intalled bases.

The value is bounded within 1 and the number of characteristic. It expresses the
average number of characteristics on which producers are concentrating their RD efforts.

*/

for(v[0]=0,cur=prod; cur!=NULL; cur=cur->next)
 {cur->cal("RD",0);
  v[1]=cur->cal("RDConc",0);
  v[2]=cur->cal("MSInstall",0);
  v[0]+=v[2]*v[1];
 }
res=v[0];
goto end;
}
/*****************************
UpperBound
******************************/
if(!strcmp(label, "UpperBound") )
{
/*
It is the limit above which a producer cannot improve the value of a given
characteristic. The upperbound is given by an unbalance in the technology: each
characteristic value cannot improve above the technological multipliers (EpiValue)
for all the other characteristics
*/

v[0]=p->cal("IdCh",0);
cur=techno->search_var_cond("TGIdCh",v[0],0);
cur2=p->up->search("Ch");
/*
- p is the characteristic of the producer
- cur is the pointer to the characteristic in TechnoGod
- cur2 is the first characteristic of producer
- cur1 is the first characteristic descending from cur
*/
for(v[1]=0,v[4]=0,cur1=cur->search("Link");cur1!=NULL; cur1=cur1->next, cur2=cur2->next)
 {

  if(cur1->cal("LIdCh",0)!=v[0]) //do not compute the constrain for the same ch.
    {
     v[2]=cur1->cal("EpiValue",0);

     v[3]=cur2->cal("Value",0);
     v[5]=v[3]*v[2];

     if(v[1]>v[5]||v[4]==0) //execute the very first time and any time the constraint apply
      {
      v[1]=v[5];
       v[4]=1;
      }

    }
 }
res=v[1];
goto end;

}

/*****************************
MaxValue
******************************/
if(!strcmp(label, "MaxValue") )
{
/*
Maximum value for the characteristic reached by one of the producers.
The equation computes also the maximum upperbound (MaxUpperbound and the range
(Range). The Range is an index (updates slowly according to SpeedRange) of
MaxValue. It is slowly updated expressing
the time necessary for followers to catch-up with leaders.
*/

v[0]=p->cal("TGIdCh",0);
for(v[9]=v[1]=v[3]=0,cur=prod; cur!=NULL; cur=cur->next)
 {cur1=cur->search_var_cond("IdCh",v[0],0);
  v[2]=cur1->cal("Value",0);
  v[9]+=v[2]*cur->cal("MSInstall",0);
  if(v[2]>v[1])
   v[1]=v[2];
  v[4]=cur1->cal("UpperBound",0);
  if(v[4]>v[3])
   v[3]=v[4];

 }
p->write("AvValue",v[9],0);
p->write("MaxUpperBound",v[3],0);
v[5]=p->cal("SpeedRange",0);
v[6]=p->cal("Range",0);

v[7]=v[1];
v[8]=v[6]*(1-v[5]) +v[7]*v[5];
p->write("Range",v[8],0);
res=v[1];
goto end;
}

/*****************************
DevLearn
******************************/
if(!strcmp(label, "DevLearn") )
{
/*
Standard Deviation used by buyers with imperfect capacity to judge characteristics'
values. When observing the products, they draw a random value from a normal function
whose mean is the actual value and the deviation is a function of their own Learning
of of DevLearn.
DevLearn is computed as an index updated slowly (using SpeedDevLearn) and tending
toward the limit value given by a percentage (PercDevLearn) of the MaxValue
*/
v[0]=p->cal("PercDevLearn",0);
v[1]=p->cal("MaxValue",0);
v[2]=p->cal("SpeedDevLearn",0);
v[3]=v[0]*v[1];
res=val[0]*(1-v[2])+v[3]*v[2];
goto end;
}
PLOG("\nFunction for %s not found", label);
quit=2;
return -1;


end :
if( (isnan(res)==1 || isinf(res)==1) && quit!=1)
 { 
  PLOG( "At time %d the equation for '%s' produces the non-valid value '%lf'. Check the equation code and the temporary values v\\[...\\] to find the faulty line.",t, label, res );

  debug_flag=1;
 } 

if(debug_flag==1)
 {
// for(i=0; i<40; i++)
  //i_values[i]=v[i];
 }

return(res);
}


object *turboskip(int n)
{
object *c;

return l[n-1];
int i;
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



