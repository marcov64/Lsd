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
v[3]=V("MarketingIndex");
v[2]=pow(v[1]+v[3],v[0]); 

RESULT( v[2])

EQUATION("Marketing")
/*
Compute the marketing expenses as a smoothed 
variable tracking sales.
*/
V("EndTrade");
v[0]=V("Sales");
v[1]=VL("Marketing",1);
v[2]=V("SpeedTrack");

v[3]=v[1] + v[2]*(v[0]-v[1]);
RESULT(v[3] )

EQUATION("MarketingIndex")
/*
Index of the relative marketing
*/
v[0]=VL("TotalMarketing",1);
v[1]=VL("Marketing",1);

RESULT(v[1]/v[0] )



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
v[3]=V("NumFirms");
cur=SEARCH("Supply");

WRITES(cur,"GenerateId",v[3]);
PARAMETER;
RESULT(1 )

EQUATION("Exit")
/*
Firms with NumUsers equal to 0 exit the market.

Return the total number of firms killed.
*/

V("EndTrade");
v[1]=0;

CYCLE_SAFE(cur, "Firm") //use CYCLE_SAFE because the cycling object could be removed.
 {//remove firms with no users
 v[0]=VS(cur,"NumUsers");
 if(v[0]==0)
  {DELETE(cur);
   v[1]++;
  } 
 }

RESULT(v[1])

EQUATION("Entry")
/*
Firms with NumUsers equal to 0 exit the market.

Return 1 is a new firm has been created or 0 otherwise

*/

V("Exit"); //ensure that exit has been computed
v[1]=V("ProbEntry");
if(RND<v[1]) //with this probability create a new firm
 {// a new firm enter
 v[10]=VL("TotalMarketing",1);
 v[11]=V("NumFirms");

 cur=SEARCH("Firm"); //take a firm, as example
 cur1=ADDOBJ_EX("Firm",cur); //add a new firm
 WRITELS(cur1,"ms_user",0, t); //assign null market share
 WRITELS(cur1,"ms_sales",0, t);//null sales share
 WRITELS(cur1,"NumUsers",0, t); //null initial users
 WRITES(cur1,"Sales",0); //null initial sales
 WRITELS(cur1,"Marketing",v[10]/v[11], t); //initial marketing endowment
 WRITELS(cur1,"MarketingIndex",1/v[11], t); //initial marketing index
 v[2]=VL("AvBD",1);
 v[3]=V("MinBD");
 v[4]=UNIFORM(v[3],v[2]);
 WRITELS(cur1,"BD",v[4],t);
 v[7]=V("GenerateId");
 WRITES(cur1,"IdFirm",v[7]); //assign an id
 v[5]=1;
 }
else
 v[5]=0; 
RESULT(v[5] )


EQUATION("AvBD")
/*
Comment
*/
v[1]=WHTAVE("BD","ms_user"); //compute descriptive statistics on BD

RESULT(v[1] )

FUNCTION("GenerateId")
/*
Generator of ID.

Being a function is computed only if requested (entry of a new firm) returning increasing integers.
*/

RESULT(CURRENT+1 )

EQUATION("NumFirms")
/*
Compute the number of firms
*/
v[0]=0;
CYCLE(cur, "Firm")
 v[0]++;
RESULT(v[0] )



EQUATION("BD")
/*
Probability that the product breaks down when a consumer is using it, triggering a replacement.

It is implemented as a random walk with barriers
*/
v[0]=VL("BD",1);
v[1]=V("StepBD");
v[2]=V("HighestBD");
v[3]=V("LowestBD");

if(v[0]-v[3]<v[1])
 {// if too close to the lower bound
  v[7]=v[3]-v[0];
  v[8]=v[7]+v[1];
 }
else
 {
 if(v[2]-v[0]<v[1])
  {// if too close to the upper bound
   v[8]=v[2]-v[0];
   v[7]=v[8]-v[1];
  }  
 else
  {// otherwise
   v[7]=-v[1]/2;
   v[8]=v[1]/2;
  } 
 } 
v[4]=v[0]+UNIFORM(v[7],v[8]);

RESULT(v[4] )


EQUATION("MinBD")
/*
Find the minimum BD among the firm
*/
SORT("Firm","BD", "UP"); //sort the firm in ascending order of BD
v[0]=V("BD"); //request the BD from the first Firm, i.e. the minimum
SORT("Firm","IdFirm", "UP"); //re-sort the firms in increasing order of ID
RESULT(v[0] )


EQUATION("TotalMarketing")
/*
Sum of all marketing values
*/

RESULT(SUM("Marketing") )








MODELEND
void close_sim(void)
{

}


