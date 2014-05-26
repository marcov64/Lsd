/****************************************************
****************************************************
LSD-2.0 25 Gennaio 2000
Copyright Marco Valente - Aalborg University

Send comments and bug reports to mv@business.auc.dk
****************************************************
****************************************************/


#include "../src/fun_head.h"

//Model specific Objects, initialized by function "Init"
object *group, *market;



/***************************************
VARIABLE::FUN
***************************************/
double variable::fun(object *caller)
{
double v[40], res;
int i;
object *p, *c, *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9;

variable *cur_v;

p=up;
c=caller;


/**
FILE *f;

{f=fopen("log.log","w");
 fprintf(f,"%s\n",label);
 fclose(f);
}

**/

if(!strcmp(label, "Action") )
{
/*
Action of agents. It can perform three different type activities, depending
on the state of the agent and on random values:

1) Maintain or remove a previously posted proposal
If a proposal has already been posted, the agent checks the time. In case
the proposal lasted for more than TimeProposal time steps, the agent removes
it from the list. If this is not the case, the agent waits and the function returns.

2) Wait
Even in case there is not a standing proposal, with probability 1-ProbAction
the agent does nothing, waiting for the next turn.

3)
With probability ProbAction, the agent submits a proposal. The current implementation
allows agents to choose randomly (50%) between a selling or buying proposal.
The price of the proposal is determined with a uniform random function centered
on the previous period's Price (see its function computed in MarketAction):
Price(t-1) * (1+(Uniform[0,1] - 0.5) * Range)
The proposal is eventually stored creating a new Object descending from Market
(Sell or Buy for selling or buying proposal respectively). The new Object
stores the ID of the agent and the proposed price.

*/

if(val[0]!=0) //if the current value if not 0, then it means that
 {            // there exists a standing proposal
  v[1]=p->cal("TimeProposal",0);
  v[2]=group->cal("MaxTime",0);
  if((double)t - v[1] <v[2]) //if the time is not expired
    {
     res=val[0];           //Maintain the proposal
     goto end;             //End of the function
     
    }

  //otherwise remove the proposal
  if(val[0]==-1)
    cur=market->search_var_cond("SId",p->cal("Id",0),0); //it was a sell
  else
    cur=market->search_var_cond("BId",p->cal("Id",0),0); //it was a buy
  cur->delete_obj();
  res=0; //mark the agent as not having a standing proposal
  goto end; // end the function
 }

//The function arrives here only if the agent does not have a standing proposal


v[0]=group->cal("ProbAction",0); //probability of submitting a proposal

if(RND>v[0])
{ res=val[0]; //do not submit any proposal
  goto end;   //end the function
}

//else, submit a proposal (prob = ProbAction)

if(RND>0.5) //choose whether to submit a sale or a purchase
 v[1]=-1;   //probably, you may want to write a more sophisticated
else        //choice function
 v[1]=1;


//Draw the price for the proposal randomly, using a uniform of range Range centered
//on the previous period's price. Here again, we may think some better way to
//obtain the agents' proposed price. See, for example, QuickPrice and SlowPrice.

//Previous period's price.
v[2]=market->cal("Price",1);

//Maximum distance from the previous period's price and the proposed one
v[3]=group->cal("Range",0);


v[4]=v[2]*(1+(RND-0.5)*v[3]);

if(v[1]==-1) //if it is a sale
 {
  //create and set a new Object Sell with the proponent's ID
//  cur=market->add_an_object("Sell",market->search("Sell"));
  cur=market->add_n_objects2("Sell",1);
  
  //Technicality on Lsd internal mechanism:
  //the second argument of the function add_an_object can be
  //an example, so that the newly created object is totally identical to the
  //example. In this case, the example is whatever is the first object Sell
  //descending from Market. It does not matter, since its data are going to be
  //overwritten. add_an_object without the second parameter uses the model's file
  //to read the structure of the object to be created, but it takes far more time...
  cur->write("SId",p->cal("Id",0),0);
  cur->write("SPrice",v[4],0);
 }
else
 {
  //create and set a new Object Buy with the proponent's data
  //cur=market->add_an_object("Buy",market->search("Buy"));
  cur=ADDOBJS(market,"Buy");
  cur->write("BId",p->cal("Id",0),0);
  cur->write("BPrice",v[4],0);
 }

p->write("PropPrice", v[4],0); //store the price in the Agent too. Currently useless

p->write("Order",RND,0); //ranking for next round. Currently useless, but you
//may think of an implementation where the order of submission matters, and therefore
//agents draw randomly their turn of activation at each time step (somewhere you should
//call for a sorting of Agent's along Order

//Store this time step, for future decisions on whether to withdraw the proposal
p->write("TimeProposal",(double)t,0);


res=v[1]; //return the value of the action chosen (-1 for sale, 1 for purchase)
goto end;
}

/********************************
MarketAction
********************************/
if(!strcmp(label, "MarketAction") )
{
/*
MarketAction
Computed after each and every agent has stored its proposal, this function
completes as many transactions as possible, coupling the best offers (lowest prices)
with the best requests (highest prices). This is the mechanism used in Italian
exchange.
In the model's structure, Market contains two sets of entities for the selling
and buying proposals. The function sorts them placing in the initial places the
most attractive ones, and proceeds matching the best elements until the best
Buy (highest proposed price) is higher than the best Sell (lowest proposed price).
This function returns the number of transactions that actually took place. It also
computes the average price of the transactions, stored in Price. 
*/

//Sort the existing entities Buy so that the first contains the highest BPrice
p->lsdqsort("Buy","BPrice","DOWN");

//Sort the existing entities SPrice so that the first contains the lowest SPrice
p->lsdqsort("Sell","SPrice","UP");

//Store in the pointers cur and cur1 respectively the best Buy and Sell
cur=p->search("Buy");
cur1=p->search("Sell");

//Initialize the local variables used to store BPrice and SPrice
v[2]=v[3]=1;

//Cycle through every entities Sell and Buy as long as prices allow a viable transaction
//Note that there are two "artificial" entities Sell and BPrice with absurd SPrice (100000000)
//and BPrice (-1) so that the cycle will always stop, even though there may be different
//number of entities Sell and Buy
for(v[5]=v[4]=0, v[0]=0; v[2]>=v[3] ; )
 {
  //Prices of the current best proposals
  v[2]=cur->cal("BPrice",0);
  v[3]=cur1->cal("SPrice",0);

  //Pointers to the subsequent entities, stored to continue the cycle
  cur2=go_brother(cur);
  cur3=go_brother(cur1);
  if(v[2]>v[3]) //If the Buy price is higher than the Sell price...
   {             //the transaction takes place, notifying the fact to the two involved agents
    cur4=group->search_var_cond("Id",cur->cal("BId",0),0);
    cur5=group->search_var_cond("Id",cur1->cal("SId",0),0);
    cur4->write("Action",0,t);
    cur5->write("Action",0,t);

//here may be placed some other activity for the agents involved in the transaction,
//like profits, accounting etc.

    v[5]+=(v[2]+v[3])/2; //Storing variable for the computation of price
    v[4]++;              //storing variable for teh computation of number of transaction

   cur->delete_obj();    //remove the proposals involved in the transaction
   cur1->delete_obj();
   }
  cur=cur2; //move to the new "best" proposals
  cur1=cur3;
 }


 if(v[4]!=0) //if at least one transaction took place
  p->write("Price",v[5]/v[4],t); //compute the average price of the time step
  //Note that the Price is tagged as if it were computed at time t, the current
  //time step, overruling the standard Lsd default. If the equation for Price
  //were not already computed at this period of the simulation cycle, it will
  //never be

res=v[4]; //return the number of transactions, useless
goto end;

}


if(!strcmp(label, "Init") )
{

/*
Initialization function.
Set two Objects of frequent use as global variable, so that they can be used
to speed up the operations in which they are used.
*/
market=p->search("Market");
group=p->search("Group");
res=0;
param=1;
goto end;
}


/*******************************
QuickPrice
*******************************/
if(!strcmp(label, "QuickPrice") )
{
/*
Compute a "quick" smoothed indicator of Price with the function
QuickPrice(t) = QuickPrice(t-1) * (1-CoeffQPrice) + Price(t) * CoeffQPrice

The higher QuickPrice, the more rapid is the response of the indicator to
the changes in Price. Note that an identical indicator is computed in SlowPrice,
the only difference being the value of the coefficients:
CoeffQPrice > CoeffSPrice
*/

v[0]=p->cal("Price",0);
v[1]=p->cal("QuickPrice",1);
v[2]=p->cal("CoeffQPrice",0);

res=v[1]*(1-v[2])+v[0]*v[2];
goto end;
}

/*******************************
SlowPrice
*******************************/
if(!strcmp(label, "SlowPrice") )
{

/*
Compute a "slow" smoothed indicator of Price with the function
SlowPrice(t) = SlowPrice(t-1) * (1-CoeffSPrice) + Price(t) * CoeffSPrice

The lower CoeffSPrice, the slower is the response of the indicator to
the changes in Price. Note that an identical indicator is computed in QuickPrice,
the only difference being the value of the coefficients:
CoeffQPrice > CoeffSPrice
*/


v[0]=p->cal("Price",0);
v[1]=p->cal("SlowPrice",1);
v[2]=p->cal("CoeffSPrice",0);
res=v[1]*(1-v[2])+v[0]*v[2];
goto end;
}

/*******************************
Price
*******************************/
if(!strcmp(label, "Price") )
{
/*
Price at each time step, computed as the average of prices in all the transactions
in the time step.
Technically, the value is computed in MarketAction, so that the equation simply
calls that Variable, and returns its own value. It may well be possible that this
equation is never executed at all, because the system finds Price always updated
when MarketAction
*/
p->cal("MarketAction",0);
res=val[0];
goto end;
}


/*******************************
SumTransactions
*******************************/
if(!strcmp(label, "SumTransactions") )
{
/*
Sum of the transactions that took place in the latest PeriodSumTrans-1 periods.
It is computed by removing from the previous value of SumTransaction the oldest
component (MarketActions[t - PeriodSumTrans]) and adding the current one
(MarketAction[t]).

*/

v[0]=p->cal("PeriodSumTrans",0);
v[1]=p->cal("MarketAction",(int)v[0]);
v[2]=p->cal("MarketAction",0);
v[3]=p->cal("SumTransactions",1);
res=v[3]-v[1]+v[2];
goto end;
}

//The program flow reach here iff the variable's label does not match any
//block of code
sprintf(msg, "\nError 04: Function for %s not found", label);
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


void close_sim(void)
{

}











