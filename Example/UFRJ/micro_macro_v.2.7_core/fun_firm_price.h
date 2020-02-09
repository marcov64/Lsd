
EQUATION("Desired_Markup")
/*
Firm Variable
*/
	v[11]=V("markup_period");
	v[0]=VL("Desired_Markup",1);                                             		//strategic markup in the last period
	v[1]= fmod((double) t-1, v[11]);                                         		//divides the time period by 8

	if(v[1]==0)                                                              		//if the rest of the above division is zero, adjust strategic markup
		{
		v[2]=VL("Avg_Potential_Markup",1);                                     		//average potential markup of the firm in the last period        
		v[3]=V("expand");                                                      		//market-share expantion parameter
		v[4]=V("aprop");                                                       		//profits apropriation parameter   
		v[5]=V("Desired_Market_Share");                                        		//desired market-share 
		v[6]=VL("Firm_Avg_Market_Share",1);                                    		//firm average market share in the last period   
		v[7]=VL("Competitiveness",1);                                          		//firm's competitiveness in the last period
		v[8]=VL("Avg_Competitiveness_Sector",1);                               		//sector's average competitiveness in the last period
		v[9]=v[3]*((v[7]-v[8])/v[8]);                                          		//how much to adjust based on the percentual diference between firm's competitiveness and sector's average competitiveness and the expantion parameter
		if(v[2]*(1+v[9])>v[0]&&v[6]>v[5])                                      		//if the already adjusted average potential markup is higher than desired strategic markup of the last period and firm's average market share is higher than desired market share
			v[10]=v[0]+v[4]*(v[2]*(1+v[9])-v[0]);                              		//the firm adjusts the strategic markup. 
		else                                                                   		//if the adjusted average potential markup is not higher than desired nor the firm's average market-share is not higher than desired
			v[10]=v[0];                                                        		//strategic markup will be the last period's                                            
		}
	else                                                                     		//if the rest of the above division is not zero, do not adjust strategic markup
		v[10]=v[0];                                                            		//strategic markup will be the last period's			
RESULT(v[10]) 


EQUATION("Wage")
/*
Nominal Wage of the firm. It increases year by year depending on inflation and firm's avg productivity. Passtrough parameters are sectorial.
*/
	v[0]=VL("Wage",1);                                                          	 	 //firm wage in the last period
	v[1]= fmod((double) t,4);                                                            //divide the time period by four
	if(v[1]==0)                                                                      	 //if the rest of the above division is zero (beggining of the year, adjust wages)
		{
		v[2]=VL("Firm_Avg_Productivity", 1);                                           	 //firm average productivity in the last period
		v[3]=VL("Firm_Avg_Productivity", 5);                                           	 //firm average productivity five periods befor
		v[4]=(v[2]-v[3])/v[3];                                                           //annual growth of sector average productivity
		v[5]=V("passthrough_productivity");                                              //pass through of productivity to wages
		v[6]=VLS(GRANDPARENT, "Consumer_Price_Index", 1);                                //price index in the last period
		v[7]=VLS(GRANDPARENT, "Consumer_Price_Index", 5);                                //price index five periods before
		v[8]=(v[6]-v[7])/v[7];                                                           //annual growth of price index (annual inflation)
		v[9]=V("passthrough_inflation");                                                 //pass through of inflation to wages   	
		v[10]=v[0]*(1+v[5]*v[4]+v[9]*v[8]);                                              //current wage will be the last period's multiplied by a rate of growth which is an expected rate on productivity plus an inflation adjustment in the wage price index
		}
	else                                                                             	 //if the rest of the division is not zero, do not adjust wages
		v[10]=v[0];                                                                      //current wages will be the last period's
RESULT(v[10])


EQUATION("Variable_Cost")
/*
Variable unit cost is the wage cost (nominal wages over productivity) plus intermediate costs
*/
	v[0]=V("Input_Cost");
	v[1]=V("Wage");
	v[2]=VL("Firm_Avg_Productivity",1);
	v[3]=(v[1]/v[2])+v[0];
RESULT(v[3])


EQUATION("Desired_Price")
/*
Firm's desired price is a desired markup over variable costs.
*/
	v[0]=V("Desired_Markup");                         							//firm's desired markup
	v[1]=V("Variable_Cost");                          							//firm's variable cost 
	v[2]=v[0]*v[1];                                  							//firm's desired price will be the desired markup applied to labor cost plus inputs cost, labor cost defined as wages over productivity
RESULT(v[2])


EQUATION("Price")
/*
Firm's effective price is a average between the desired price and the sector average price
*/
	v[0]=VL("Price",1);                                                        //firm's price in the last period
	v[1]=V("Desired_Price");                                                   //firm's desired price
	v[2]=V("parameter1");                                                      //weight parameter
	v[3]=VL("Avg_Price", 1);                                                   //sector average price in the last period
	v[4]=v[2]*(v[1])+(1-v[2])*(v[3]);                                          //firm's price is a average between the desired price and the sector average price
	if(v[1]>0)                                                                 //if desired price is positive
		v[5]=max(0.01,v[4]);                                                   //firm's price can never be zero or lower
	else                                                                       //if desired price is not positive
		v[5]=v[0];                                                             //firm's price will be the last period's
RESULT(v[5])


EQUATION("Effective_Markup")
/*
Effective Markup is the Effective Price over the Variable Cost
*/
	v[0]=V("Price");
	v[1]=V("Variable_Cost");
	v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("Desired_Market_Share")
/*
Desired Market Share is a simple average between last period's desired market share and firm's average market share
*/
	v[0]=V("markup_period");
	v[1]=VL("Desired_Market_Share", 1);                           	//desired market share in the last period
	v[2]=VL("Firm_Avg_Market_Share", 1);                        	//firm's average market share (desired)
	v[3]= fmod((double) t-1, v[0]);                               	//devides the last period by eight
	if(v[3]==0)                                                   	//if the rest of the above division is zero, adjust desired market share
		v[4]=(v[1]+v[2])/2;                                         //current desired market share is a simple average between last period's desired market share and firm's average market share
	else                                                          	//if the rest of the above division is not zero, do not adjust desired market share
 		v[4]=v[1];                                                  //firm's desired market share will be equal to the last period's
RESULT(v[4])


EQUATION("Firm_Avg_Market_Share")
/*
Average Market Share between the market share of the firm in the last markup period
*/
	v[0]=V("markup_period");
	v[3]=0;										   						//initializes the sum
	for (v[1]=0; v[1]<=(v[0]-1); v[1]=v[1]+1)							//from 0 to markup period-1 lags
		{
		v[2]=VL("Market_Share", v[1]);									//computates firm's market share of the current lag
		v[3]=v[3]+v[2];													//sum up firm's lagged market share
		}
	v[4]=v[3]/v[0];														//average firm's market share of the last investment period
RESULT(v[4])


EQUATION("Potential_Markup")
/*
Potential markup is the sector average price over firm's variable costs
*/
	v[0]=V("Avg_Price");                                              //sector average price
	v[1]=V("Variable_Cost");                                          //firm's variable cost
	if(v[1]!=0)                                                       //if firm's variable cost is not zero
		v[2]=v[0]/v[1];                                               //potential markup is the sector average price over firm's variable costs
	else                                                              //if firm's variable cost is zero
		v[2]=0;                                                       //potential markup is zero
RESULT(v[2])


EQUATION("Avg_Potential_Markup")
/*
Average Potential Markup between the potential markup of the firm in the last 8 periods
*/
	v[0]=V("markup_period");
	v[3]=0;																//initializes the sum
	for (v[1]=0; v[1]<=(v[0]-1); v[1]=v[1]+1)							//from 0 to markup period-1 lags
		{
		v[2]=VL("Potential_Markup", v[1]);								//computates firm's potential markup of the current lag
		v[3]=v[3]+v[2];													//sum up firm's lagged potential markup
		}
	v[4]=v[3]/v[0];														//average firm's market share of the last potential markup
RESULT(v[4])
