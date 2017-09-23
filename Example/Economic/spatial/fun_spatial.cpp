#include "fun_head.h"

MODELBEGIN


FUNCTION("Shopping")
/*
Function: Shopping

This function is executed again and again every time it is requested (note that, on the contrary, normal variables are executed only once at every time step). It is assuming that it is a Consumer
who asked for this value.

The code of this equation reads the spatial location of the consumer (location is one dimension between 0 and 1). Then scans all the firms and, to each of them, computes the distance between the firm and the consumer. This distance value is raised to the power of "alpha" to allow for higher or lower sensitivity to distance. This 'distorted' distance is used as probability (remember that locations must be defined between 0 and 1), so that with prob 1-(D^a) the firm is selected, otherwise it is discarded.

Among all the selected firms the function chooses randomly one firm with probability equal to the value of parameter Quality. The function return the IdFirm of the chosen firm. If no firm is selected (may happen), then it returns the IdFirm of the previously chosen product, indicated in the lagged consumer's variable Purchase.
*/
v[0]=VS(c,"LocationC"); //spatial location of the consumer
v[3]=V("alpha"); //exponent used to bias probabilities

v[8]=0; //flag to control whether at least one firm is selected
CYCLE(cur, "Firm")
 {//cycle scanning all the firms
  v[1]=VS(cur,"LocationF");
  v[2]=1-pow(abs(v[0]-v[1]),v[3]); //probability indicator
  if(RND<v[2])
   { // happen with v[2] prob
    v[4]=VS(cur,"Quality");
    WRITES(cur,"Prob",v[4]); //assign to the parameter Prob of the current firm the value of its Quality
    v[8]++;
   }
  else
    WRITES(cur,"Prob",0); //assign to the parameter Prob of the current firm the value 0 (not selected
  
 }


v[6]=VLS(c,"Purchase",1); //read the IdFirm of the previously purchased product by the consumer
if(v[8]>0)
{ //at least one firm have been selected
  if(v[6]>0)
   { //The consumer did have a previous product (at time 0 no consumer is using any product)
   cur=SEARCH_CND("IdFirm",v[6]); //search the Firm
   v[7]=INCRS(cur,"Sales",-1); //decrease its sales
   }
  cur=RNDDRAW("Firm","Prob"); //choose randomly one firm according to their probabilities Prob
  v[5]=VS(cur,"IdFirm"); //read the IdFirm of the chosen firm
  INCRS(cur,"Sales",1); //increase the Sales of the chosen firm
 }
else
 v[5]=v[6];  //no firm have been selected, return the consumer's previous purchase
RESULT(v[5] )

EQUATION("Purchase")
/*
Variable Purchase.

Choose one product. All the job is done by function Shopping.

For optimization purposes, the Lsd function used is not V("Shopping"). In fact, this would require Lsd to 
retrive the object containing Shopping automatically. Since variable Purchase is stored in object Consumer, 
the automatic Lsd retrival system would need to scan : 
- all Consumer;
- the Demand object;
- the Root object
- the Supply object, and here it is found

Instead, it uses the V_CHEAT Lsd function. This function permits to specify the exact object where the required variable is stored (Supply) relative to the calling object. In our case, the calling object is a Consumer, therefore the path from Consumer to Supply is: Consumer->Demand->Supply, therefore p->up->next. 
The last field of VS_CHEAT is the object that will appear to the called variable as the calling object, therefore this Consumer, p.
*/

RESULT(V("Shopping") )




EQUATION("SmoothSales")
/*

SmoothSales[t]=SmoothSales[t-1]*parSmooth + Sales*(1-parSmooth)

Descriptive statistics used to smooth away short period volatility of Sales.

SmoothSales can be considered a sort of moving average over Sales with weights that decrease exponentially for older values of Sales.
*/
v[0]=VL("SmoothSales",1);
v[3]=V("parSmooth");
v[2]=V("Sales");
if(v[0]==0)
 v[1]=v[2];
else
 v[1]=v[0]*v[3]+(1-v[3])*v[2];

 
RESULT(v[1] )


MODELEND




void close_sim(void)
{

}


