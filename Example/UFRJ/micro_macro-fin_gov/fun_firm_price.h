

EQUATION("Firm_Price_Period")
/*
This variable writes for each firm, if it is price adjustment period or not, assuming 1 or 0.
*/
	v[0]=V("sector_price_frequency");						//sector price period parameter
	v[1]=fmod((t+v[0]),v[0]);								//devides the current time step by the price period and takes the rest
	v[2]=V("firm_id");										//firm number
	v[3]=fmod((v[2]+v[0]),v[0]);							//divides the firm number by the price period and takes the rest
	if (v[3]==v[1])											//if the firm number rest matches the time step rest
		v[4]=1;												//mark as price period for the firm	
	else													//if the rests do not match
		v[4]=0;												//do not mark as price period for the firm
RESULT(v[4])


EQUATION("Firm_Wage")
/*
Nominal Wage of the firm. It increases year by year depending on inflation and firm's avg productivity. Passtrough parameters are sectorial.
*/
	v[0]=CURRENT;                                                          	 			 //firm wage in the last period
	v[1]=V("annual_frequency");
	v[2]= fmod((double) t-1,v[1]);                                                      //divide the time period by the annual period parameter
	if(v[2]==0)                                                                      	 //if the rest of the above division is zero (beggining of the year, adjust wages)
		{
		v[4]=LAG_GROWTH(p, "Firm_Avg_Productivity", v[1], 1);
		v[5]=V("sector_passthrough_productivity");
		v[6]=VL("Country_Annual_CPI_Inflation", 1);
		v[7]=V("sector_passthrough_inflation");
		v[8]=VS(financial, "cb_target_annual_inflation");
		//v[9]=v[0]*(1+v[8]+v[5]*v[4]+v[7]*(max(0,v[6]-v[8])));                           //current wage will be the last period's multiplied by a rate of growth which is an expected rate on productivity plus an inflation adjustment in the wage price index
		v[9]=v[0]*(1+v[7]*max(0,v[6])+v[5]*v[4]);
		}
	else                                                                             	 //if the rest of the division is not zero, do not adjust wages
		v[9]=v[0];                                                                      //current wages will be the last period's
RESULT(v[9])


EQUATION("Firm_Input_Cost")
/*
Unitary costs of the inputs. It's given by the domestic input price plus the external input price, weighted by the proportion of the demand met by domestic and external sectors
*/
	v[0]=VL("Sector_Propensity_Import_Inputs",1);
	v[1]=VLS(input,"Sector_Avg_Price",1);                 //intermediate sector average price
	v[2]=VLS(input,"Sector_External_Price",1);            //sector external price
	v[3]=V("sector_input_tech_coefficient");              //input technical relationship 
	v[5]=V("Country_Exchange_Rate");                      //exchange rate
	v[8]=v[1]*v[3]*(1-v[0])+v[3]*v[0]*v[2]*v[5];     	  //input cost will be the amount demanded domesticaly multiplied by domestic price plus the amount demanded externally miltiplied by the external price
RESULT(v[8])


EQUATION("Firm_Variable_Cost")
/*
Variable unit cost is the wage cost (nominal wages over productivity) plus intermediate costs
*/
	v[0]=V("Firm_Input_Cost");
	v[1]=V("Firm_Wage");
	v[2]=VL("Firm_Avg_Productivity",1);
	v[3]= v[2]!=0? (v[1]/v[2])+v[0] : v[0];
RESULT(v[3])


EQUATION("Firm_Unit_Financial_Cost")
/*
Financial costs include interest payment and debt payment. Unit financial cost is total financial costs divided by effective production.
*/
	v[0]=V("sector_investment_frequency");
	v[6]=V("sector_desired_degree_capacity_utilization");
	v[4]=0;
	for(i=1; i<=v[0]; i++)
		{
		v[1]=VL("Firm_Interest_Payment",i);
		v[2]=VL("Firm_Productive_Capacity",i);
			if(v[2]!=0)
				v[3]=v[1]/(v[2]*v[6]);
			else
				v[3]=0;
		v[4]=v[4]+v[3];
		}
	v[5]=v[4]/v[0];
	
	v[7]=VL("Firm_Debt_Rate",1);
	v[8]=VL("Firm_Max_Debt_Rate",1);
	if(v[7]>v[8])
		v[9]=v[5];
	else
		v[9]=0;
RESULT(v[9])


EQUATION("Firm_Desired_Markup")
/*
Firm Variable.
New formulation.
Adjusted when is price adjustment period for the firm. 
If firm's market share increased and it is over the desired market share, the firm can adjust the desired markup 
If firm's market share increased but effective avg market share is lower than desired, the firm do not adjust markup to do not increase prices and try to gain cfurther competitiveness.
*/
	v[0]=CURRENT;          										//current desired markup                                   						
	v[1]=V("Firm_Price_Period");  								//defines if it is price adjustment period fot that firm                                       			
	v[2]=V("sector_price_frequency");							//defines the frequency in which firms adjust prices
	if(v[1]==1)                       							//if it is price adjustment period                                       		
		{
		v[3]=LAG_AVE(p, "Firm_Market_Share", v[2]);				//average market share for the last price frequency periods
		v[4]=LAG_GROWTH(p, "Firm_Market_Share", v[2]);  		//market share growth for the last price frequency periods  
		v[5]=V("sector_desired_market_share");					//desired market share
		v[6]=V("sector_markup_adjustment");						//defines the markup adjustment intensity
		if(v[4]>0&&v[3]>v[5])									//if market share growth is positive and above desired
			v[7]=v[0]*(1+v[6]*v[4]);
		else													//otherwise
			v[7]=v[0];											//use current desired markup
		}
	else          												//if it is not price adjustment period                                                           		
		v[7]=v[0];      										//use current desired markup                                                      					
RESULT(v[7]) 


EQUATION("Firm_Desired_Price")
/*
Firm's desired price is a desired markup over variable costs plus (possible) financial costs.
*/
	v[0]=V("Firm_Desired_Markup");                         						//firm's desired markup
	v[1]=V("Firm_Variable_Cost");                          						//firm's variable cost 
	v[2]=V("Firm_Unit_Financial_Cost");											//firm's unit financial cost
	v[3]=V("sector_financial_cost_passtrough");									//defines how financial costs are passed to prices
	v[4]=v[0]*(v[1]+v[2]*v[3]);                                  				//firm's desired price will be the desired markup applied to unit cost
RESULT(v[4])


EQUATION("Firm_Price")
/*
Firm's effective price is a average between the desired price and the sector average price
*/
	v[0]=VL("Sector_External_Price",1);
	v[1]=VL("Sector_Avg_Price", 1);                                            //sector average price in the last period
	v[2]=V("Country_Exchange_Rate");                      					   //exchange rate
	v[3]=VL("Sector_Exports_Share",1);
	v[7]=V("sector_external_price_weight");
	v[4]=(1-v[7]*v[3])*v[1] + v[7]*v[3]*v[2]*v[0];									   //reference price, weighted by the expoerts share
	v[5]=V("Firm_Desired_Price");                                              //firm's desired price
	v[6]=V("sector_strategic_price_weight");                                   //strategic weight parameter
	v[8]=v[6]*v[5]+(1-v[6])*v[4];                            				   //firm's price is a average between the desired price and the reference price
	v[9]=V("Firm_Price_Period");											  
	if(v[9]==1)																    //if it is price adjustment perod for that firm
		v[10]=v[8];																//set new price
	else																		//if it is not price adjustment period
		v[10]=CURRENT;															//use current price
RESULT(v[10])


EQUATION("Firm_Effective_Markup")
/*
Effective Markup is the Effective Price over the Variable Cost
*/
	v[0]=V("Firm_Price");
	v[1]=V("Firm_Variable_Cost");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])
