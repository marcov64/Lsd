#include "fun_head.h"

MODELBEGIN



EQUATION("ProdUsed")
/*
Determine the product used
*/
v[0]=V("IsBroken"); //breaks ?
if(v[0]==1)
  v[1]=V("Purchase"); //yes, buy a new produc
else 
  v[1]=val[0]; //no, keep on using the previous one

RESULT(v[1])

FUNCTION("IsBroken")
/*
Look whether the product breaks down or not. Return 1 if the product breaks or 0 if not.

The object c-> is the consumer checking its product
*/
v[0]=VLS(c,"ProdUsed",1); //product used by the consumer

cur=SEARCH_CND("IdFirm",v[0]);
v[2]=VS(cur,"BD");
if(RND<v[2])
 {v[1]=1; //product broken
 INCRS(cur,"NumLost",1);
 } 
else
 v[1]=0; //product not broken
RESULT(v[1] )



FUNCTION("Purchase")
/*
Make a purchase for the calling object (supposedly a consumer).
RNDDRAW("Obj", "VarOrPar") is a Lsd function choosing randomly an 
object called "Obj" probability equal to "VarOrPar". 
*/
V("InitTrade"); //ensure that firms are ready to sell
v[1]=V("IncludeBroken");
if(v[1]==0)
 {//if the broken product should not be included
 v[2]=VLS(c,"ProdUsed",1); //old product used
 cur=SEARCH_CND("IdFirm",v[2]); //Firm of the old product
 v[3]=VS(cur,"Visibility"); //its visibility
 WRITELS(cur,"Visibility",0, t); //impose (temporarily) zero visibility
//   INTERACT("Pippo", 1);
 }

cur1=RNDDRAW("Firm","Visibility");  //choose randomly one of the products
v[0]=VS(cur1,"IdFirm"); //return the ID of the chosen firm

if(v[1]==0 ) //if broken product was not included
 WRITELS(cur,"Visibility",v[3], t); //restore its original visibility
 
INCRS(cur1,"Sales",1); //increase the Sales of the chosen firm
RESULT(v[0] )

EQUATION("Visibility")
/*
The apparent quality of a firm is computed as the past market 
share raised to the power of alpha.
Shares are lagged because the new ones are not yet ready 
to be computed before consumers finish to buy.
*/
v[0]=V("alpha");
v[1]=VL("ms_user",1);
if(v[1]==0)
 v[2]=0;
else
 v[2]=pow(v[1],v[0]); 

RESULT( v[2])



EQUATION("ms_user")
/*
Market shares of users, computed as the ratio of this users over the sum of total users
*/
v[0]=V("TotalUsers");
v[1]=V("NumUsers");


RESULT(v[1]/v[0] )


EQUATION("ms_sales")
/*
Market shares of current sales
*/
v[0]=V("TotalSales");
v[1]=V("Sales");
if(v[0]>0)
 v[2]=v[1]/v[0];
else
 v[2]=0; 
RESULT(v[2])

EQUATION("TotalSales")
/*
Total sales
*/
V("EndTrade"); //ensure that consumers finished the shopping
v[0]=SUM("Sales");

RESULT(v[0] )

EQUATION("TotalUsers")
/*
Total number of users
*/
v[0]=0;
CYCLE_SAFE(cur,"Firm")
  v[0]+=VS(cur,"NumUsers");


RESULT(v[0] )



EQUATION("EndTrade")
/*
For each consumer ensures that the variable ProdUsed
is updated.

When this variable is completed firms can be sure that the
values of Sales and NumLost are filled with the correct values
*/
CYCLE(cur, "Consumer")
 VS(cur,"ProdUsed");


RESULT( 1)

EQUATION("InitTrade")
/*
Initialize the trading period. For each firm set to 0 the 
parameter Sales and NumLost, that will be filled by 
the shopping of consumers.
*/
V("Init");
CYCLE(cur, "Firm")
 { //for all firms set to 0 Sales and NumLost
  WRITES(cur,"Sales",0);
  WRITES(cur,"NumLost",0);
 }

RESULT(1 )

EQUATION("NumUsers")
/*
Number of users, computed, after the end of the trading period,
by summing to the previous users the new sales and 
removing the lost users
*/
V("EndTrade"); //ensure that buyers finished the shopping
v[0]=VL("NumUsers",1); //former number of users
v[1]=V("Sales"); //sales at this time
v[2]=V("NumLost"); //lost users at this time


v[4]=v[0]+v[1]-v[2];
RESULT(v[4] )

EQUATION("Init")
/*
Initialization variable, used to assign initial products to consumers before starting a simulation run.
This equation is executed only once at the beginning of the first time step and then is 
transformed in a parameter.

For each consumer a firm is chosen with equal probability,
and its IfFirm value is assigned to the ProdUsed of the consumer.

The second cycle scans all Firm and computes their ms_users.

At the end equation the command PARAMETER transform this variable in a parameter,
so that it is not computed again.

*/
v[0]=0;
CYCLE(cur, "Consumer")
 {
 cur1=RNDDRAWFAIR("Firm");  //choose randomly one of the products
 v[1]=VS(cur1,"IdFirm"); //return the ID of the chosen firm
 INCRS(cur1,"NumUsers",1);//increase the number of users for the chosen firm
 WRITES(cur,"ProdUsed",v[1]);
 v[0]=v[0]+1; //count the number of consumers
 }
 
CYCLE(cur, "Firm")
 {
  v[2]=VLS(cur,"NumUsers",1);
  WRITES(cur,"ms_user",v[2]/v[0]);
 }

PARAMETER;
RESULT(1 )



MODELEND




void close_sim(void)
{

}


