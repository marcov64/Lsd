
EQUATION("Revenue")
/*
Revenue depends on sales and price
*/
	v[0]=V("Sales");                                                  //firm's sales 
	v[1]=V("Exports_Sector");                                         //sector exports
	v[2]=V("Price");                                                  //firm's price
	v[4]=V("Exchange_Rate");                                          //exchange rate
	v[5]=V("Effective_Market_Share");                                 //firm's effective market share
	v[6]=v[2]*(v[0]-v[5]*v[1])+v[5]*v[1]*v[2]/v[4];                   //revenue is given by firm's price multiplied by domestic sales plus exports (weighted by firm's market share) multiplied by firm's price over the exchange rate
RESULT(v[6])


EQUATION("Indirect_Tax")
/*
Indirect Tax of the firm is the revenue multiplied by the tax parameter
*/
	v[0]=V("Revenue");
	v[1]=V("indirect_tax");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Net_Revenue")
/*
Firm's net revenue, discounting taxes and R&D expenses.
*/
	v[0]=V("Revenue");
	v[1]=V("indirect_tax");
	v[2]=V("innovation_revenue_proportion");
	v[3]=V("imitation_revenue_proportion");
	v[4]=v[0]*(1-v[1])*(1-v[2]+v[3]);
RESULT(v[4])


EQUATION("RND_Expenses")
/*
Firm's R&D expenses, subtracted from the revenue after taxes.
It will be distributed to income class as wages.
*/
	v[0]=V("Revenue");
	v[1]=V("indirect_tax");
	v[2]=V("innovation_revenue_proportion");
	v[3]=V("imitation_revenue_proportion");
	v[4]=v[0]*(1-v[1])*(v[2]+v[3]);
RESULT(v[4])


EQUATION("Net_Profits")
/*
Profit of each firm, difference between revenue and costs, being revenue as a function of price and sales and cost of total inputs and labor used in production
*/
	v[0]=V("Net_Revenue");                                            //firm's net revenue
	v[1]=V("Effective_Production");                                   //firm's effective production
	v[2]=V("Variable_Cost");                                          //firm's variable cost
	v[3]=VL("Stock_Debt",1);                                          //firm's stock of debt in the last period
	v[4]=V("Interest_Rate_Firm");                                     //interest rate paid by the firm
	v[6]=VL("Stock_Financial_Assets",1);                              //stock of financial assets in the last period
	v[7]=V("Interest_Rate");                                          //return rate of financial assets
	//v[8]=v[0]+v[6]*v[7]-(v[3]*v[4])-(v[2]*v[1]);                    //net profits is given by net revenue plus financial assets return minus variable unit cost times effective production, depreciation expenses and interest payment.
	v[8]=v[0]-(v[3]*v[4])-(v[2]*v[1]);
RESULT(v[8])


EQUATION("Retained_Profits")
/*
Profit retained by the sector after being distributed to class and paid interest on the debt and separate the expense for depreciation.
*/
	v[0]=V("Net_Profits");                                             //firm's profits            
	v[1]=V("profits_distribution_rate");                               //firm's profit distribution parameter                            
	if(v[0]>0)                                                         //if net profits is positive
		v[2]=(1-v[1])*v[0];                                            //retained profits
	else                                                               //if net profits is zero or negative                                                                     
		v[2]=v[0];                                                     //retained profits equals net profits                                                             
RESULT(v[2])


EQUATION("Profits_Distribution")
/*
Amount of profits distributed to the income classes
*/
	v[0]=V("Net_Profits");                                             //firm's profits            
	v[1]=V("profits_distribution_rate");                               //firm's profit distribution parameter  
	if(v[0]>0)                                                         //if net profits is positive
		v[2]=v[1]*v[0];                                                //distributed profits
	else                                                               //if net profits is zero or negative                                                                     
		v[2]=0;																											
RESULT(v[2])
