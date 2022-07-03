EQUATION("Firm_Effective_Orders")
/*
Sectoral effective orders is distributed to firms by market shares.
*/
	v[0]=V("Firm_Market_Share"); 
	v[1]=V("Sector_Effective_Orders");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Firm_Sales")
/*
Effective Sales depends on effective orders but is restricted by firm's effective production plus stock of inventories.
*/
	v[0]=VL("Firm_Stock_Inventories",1);                   //stock of inventories of the firm
	v[1]=V("Firm_Effective_Production");                   //firm's effective production in that period
	v[2]=V("Firm_Effective_Orders");                       //firm's effective orders
	v[3]=v[0]+v[1];                                        //total amount of available products
	v[4]=max(0, min(v[3],v[2]));                           //firm's sales will be the effective orders, constrained by the total amount of available products, and can never be negative
RESULT(v[4])


EQUATION("Firm_Stock_Inventories")
/*
Current firm's stock of inventories is the current stock plus the net difference between production and sales. Cannot be negative.
*/
	v[0]=V("Firm_Effective_Production");                    //firm's effective production                 
	v[1]=V("Firm_Sales");                                   //firm's sales
	v[3]=v[0]-v[1]+CURRENT;                                 //the stock of inventories in the end of the period will be the difference between effective production and sales, added to the current stock of inventories
RESULT(max(0,v[3]))


EQUATION("Firm_Inventories_Variation")
/*
Firm's inventories variation in current nominal values
*/
	v[0]=V("Firm_Price");
	v[1]=VL("Firm_Stock_Inventories", 1);
	v[2]=V("Firm_Stock_Inventories");
	v[3]=v[0]*(v[2]-v[1]);
RESULT(v[3])


EQUATION("Firm_Market_Share")
/*
Firm's market share evolves based on the difference between firm's competitiveness index and sector's average competitiveness.
*/
	v[0]=CURRENT;                					//firm's market share in the last period
	v[1]=V("Sector_Avg_Competitiveness");           //sector average competitiveness
	v[2]=V("sector_competitiveness_adjustment");	//sector parameter that adjustts market share
	v[3]=V("Firm_Competitiveness");                 //firm's competitiveness
	if(v[1]!=0)                                     //if the sector average competitiveness is not zero
		v[4]=v[0]+v[2]*v[0]*((v[3]/v[1])-1);        //firm's market share will be the last period's inscresed by the adjustment paramter times the ratio between firm's competitiveness and sector average competitiveness
	else                                            //if the sector average competitiveness is zero
		v[4]=v[0];                                  //firm's market share will be zero (testar, remover)
RESULT(v[4])


EQUATION("Firm_Delivery_Delay")
/*
Firm Variable. Calculates the share of effevtive orders that are not met by firm's capacity. 
If all demand is met, this variable's result is 1. The higher the demand not met, higher is this variable.
*/
	v[0]=V("Firm_Effective_Orders");;               //firm's effective orders
	v[1]=V("Firm_Sales");                           //firm's sales
	v[2]= v[1]!=0? v[0]/v[1] : 1;                   //delivery delay will be determined by the ratio between effective orders and sales
RESULT(v[2])


EQUATION("Firm_Competitiveness")
/*
Firm's Competitiveness index depends on the quality of the product, the price and the delivery delay of the firm
*/
	v[0]=VL("Firm_Price",1);                                           //firm's price in the last period
	v[1]=V("sector_elasticity_price");                                 //price elasticity
	v[2]=VL("Firm_Quality",1);                                         //product quaility
	v[3]=V("sector_elasticity_quality");                               //quality elasticity
	v[4]=VL("Firm_Competitiveness",1);                                 //firm's competitiveness in the last period
	v[5]=VL("Firm_Delivery_Delay",1);                                  //firm's delivery delay in the last period
	v[6]=V("sector_elascitity_delay");                                 //delay elasticity	
   	if(v[0]!=0&&v[2]!=0&&v[5]!=0)                                      //if the price was not zero neither the quality
     	v[7]=(pow(v[2],v[3]))*(1/pow(v[0],v[1]))*(1/pow(v[5],v[6]));   //firm's competitiveness will be given by the quality powered by its elasticity over the price, powered by its elasticicty, and the delivery delay, powered by its elasticicty
   	else                                                               //if either the price or the quality was zero 
     	v[7]=v[4];                                                     //firm's competitiveness will be the competitiveness in the last period
RESULT(v[7])


EQUATION("Firm_Effective_Market_Share")
/*
Effective market share is given by firm's sales over total sales of the sector
*/
	v[0]=V("Firm_Sales");
	v[1]=V("Sector_Sales");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


