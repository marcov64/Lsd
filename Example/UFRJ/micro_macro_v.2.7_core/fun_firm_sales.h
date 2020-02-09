
EQUATION("Sales")
/*
The observed sales are defined by the minimum between the effective orders and the effective production plus inventories of each sector.In the case of intermediate sectors, the sale of inputs from their inventories to meet the extraordinary demand at the beginning of the period should be added to the total sold.In the case of the agricultural sector, the observed sales are equal to the current production.
*/
	v[0]=VL("Inventories",1);                              //stock of inventories of the firm
	v[1]=V("Effective_Production");                        //firm's effective production in that period
	v[2]=V("Effective_Orders");                            //firm's effective orders
	v[3]=v[0]+v[1];                                        //total amount of available products
	v[4]=max(0, min(v[3],v[2]));                           //firm's sales will be the effective orders, constrained by the total amount of available products, and can never be negative
RESULT(v[4])


EQUATION("Inventories")
/*
The finished product stock at the end of the period will be calculated by adding the remaining stock of products at the beginning of the period to which it was produced and discounting the quantity sold. In the case of the agricultural sector, there is no stock of finished products at the end of the period.
*/
	v[0]=V("Effective_Production");                         //firm's effective production                 
	v[1]=V("Sales");                                        //firm's sales
	v[2]=VL("Inventories", 1);                              //stock of inventories in the last period
	v[3]=v[0]-v[1]+v[2];                                    //the stock of inventories in the end of the period will be the difference between effective production and sales, added to the current stock of inventories
RESULT(v[3])


EQUATION("Inventories_Variation")
/*
Firm's inventories variation in current nominal values
*/
	v[0]=V("Price");
	v[1]=VL("Inventories", 1);
	v[2]=V("Inventories");
	v[3]=v[0]*(v[2]-v[1]);
RESULT(v[3])


EQUATION("Effective_Orders")
/*
Firm Variable
*/
	v[0]=V("id_intermediate_goods_sector");
	v[1]=V("id_consumption_goods_sector");
	v[2]=V("id_capital_goods_sector");

	if (v[0]==1);
		v[3]=V("intermediate_effective_orders_firm_temporary");
	if (v[1]==1)
		v[3]=V("consumption_effective_orders_firm_temporary");
	if (v[2]==1)
		v[3]=V("Effective_Orders_Capital_Goods_Firm");

	WRITE("intermediate_effective_orders_firm_temporary", 0);
	WRITE("consumption_effective_orders_firm_temporary", 0);
RESULT(v[3])


EQUATION("Market_Share")
/*
Firm Variable
*/
	v[0]=VL("Market_Share", 1);                     //firm's market share in the last period
	v[1]=V("Avg_Competitiveness_Sector");           //sector average competitiveness
	v[3]=V("Competitiveness");                      //firm's competitiveness
	if(v[1]!=0)                                     //if the sector average competitiveness is not zero
		v[4]=v[0]+v[0]*((v[3]/v[1])-1);             //firm's market share will be the last period's inscresed by the adjustment paramter times the ratio between firm's competitiveness and sector average competitiveness
	else                                            //if the sector average competitiveness is zero
		v[4]=0;                                     //firm's market share will be zero (testar, remover)

RESULT(v[4])


EQUATION("Delivery_Delay")
/*
Firm Variable
*/
	v[0]=V("Effective_Orders_Sector");              //effective orders of the sector
	v[1]=V("Market_Share");                         //firm's market share
	v[2]=v[0]*v[1];                                 //firm's effective orders
	v[3]=V("Sales");                                //firm's sales
	if (v[3]!=0)                                    //if firm's sales is not zero
		v[4]=v[2]/v[3];                             //delivery delay will be determined by the ratio between effective orders and sales
	else                                            //if firm's sales is zero
		v[4]=1;                                     //delivery delay will be one
RESULT(v[4])


EQUATION("Competitiveness")
/*
Competitiveness depends on the quality of the product, the price and the delivery delay of the firm
*/
	v[0]=VL("Price",1);                                                //firm's price in the last period
	v[8]=VL("Avg_Price",1);
	v[9]=VL("Avg_Quality_Sector",1);
	v[10]=v[0]/v[8];
	v[11]=v[2]/v[9];
	v[1]=V("e_price");                                                 //price elasticity
	v[2]=VL("Quality",1);                                              //product quaility
	v[3]=V("e_quality");                                               //quality elasticity
	v[4]=VL("Competitiveness",1);                                      //firm's competitiveness in the last period
	v[5]=VL("Delivery_Delay",1);                                       //firm's delivery delay in the last period
	v[6]=V("e_delay");                                                 //delay elasticity	
   	if(v[0]!=0&&v[2]!=0)                                               //if the price was not zero neither the quality
     	v[7]=(pow(v[2],v[3]))*(1/pow(v[0],v[1]))*(1/pow(v[5],v[6]));   //firm's competitiveness will be given by the quality powered by its elasticity over the price, powered by its elasticicty, and the delivery delay, powered by its elasticicty
   	else                                                               //if either the price or the quality was zero 
     	v[7]=v[4];                                                     //firm's competitiveness will be the competitiveness in the last period
RESULT(v[7])


EQUATION("Effective_Market_Share")
/*
Effective market share is given by firm's sales over total sales of the sector
*/
	v[0]=V("Sales");
	v[1]=V("Sales_Sector");
	if (v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])


