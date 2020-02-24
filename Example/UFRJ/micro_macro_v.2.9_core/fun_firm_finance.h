
EQUATION("Firm_Interest_Rate")
/*
Interest rate paid by the firm depends on a individual spread over basic interest rate. The firm's spread is based on the average debt rate of the firm.
*/
	v[0]=V("basic_percent_spread");
	v[1]=VL("Firm_Avg_Debt_Rate",1);
	v[2]=1+v[1]*v[0];
	v[3]=V("Basic_Interest_Rate");
	v[4]=v[2]*v[3];
RESULT(v[4])


EQUATION("Firm_Total_Funds")
/*
Total available funds for investment in the current period
*/
	v[0]=V("Firm_Retained_Profits");
	v[1]=V("Firm_Available_Financial_Assets");
	v[2]=V("Firm_Available_Debt");
	v[3]=VL("Firm_Stock_Debt", 1);
	v[4]=VL("Firm_Stock_Financial_Assets", 1);
	v[5]=max(v[0],-v[4]);
	v[6]=V("Basic_Interest_Rate");
	v[7]=V("Firm_Interest_Rate");
	v[8]=v[0]+v[1]+v[2];
RESULT(v[8])


EQUATION("Firm_Available_Financial_Assets")
/*
Firm's available financial assets to spend in investment expenses. It's the diference between desired stock of financial assets and the currest stock. If the difference is positive, there are available financial assets that can be sold to pay for investment expenses. If the value is negative, firm must buy more financial assets to reach the desired amount, reducing the available total funds for investment.
*/
	v[0]=VL("Firm_Capital",1);
	v[1]=V("desired_financial_rate");
	v[2]=VL("Firm_Stock_Financial_Assets",1);
	v[3]=v[2]-(v[1]*v[0]);//can be positive or negative
RESULT(v[3])


EQUATION("Firm_Available_Debt")
/*
Firm's available debt or available external funds depends on the difference between desired stock of debt and current stock of debt. If currest stock of debt is greater than desired, the firm must repay some debt reducing the amount of external funds for investment. If the currest amount is smaller than desired, that difference is available to the firm as external finance, but that does not mean that the firm will increase effective debt by this amount.
*/
	v[0]=VL("Firm_Capital",1);
	v[1]=V("desired_debt_rate");
	v[2]=VL("Firm_Stock_Debt",1);
	v[3]=VL("Firm_Stock_Financial_Assets",1);
	v[4]=v[1]*(v[0]+v[3])-v[2];//can be negative or positive value 
RESULT(v[4])


EQUATION("Firm_Stock_Debt")
/*
Stock of Debt
*/
	v[0]=V("Firm_Debt_Flow");
	v[1]=VL("Firm_Stock_Debt", 1);
	v[2]=v[0]+v[1];
	v[3]=max(0, v[2]);
RESULT(v[3])


EQUATION("Firm_Debt_Rate")
/*
Degree of indebtedness, calculated by the ratio of the debt to the capital of the sector.
*/
	v[0]=V("Firm_Stock_Debt");                                                         //firm's stock of debt 
	v[1]=V("Firm_Capital");                                                            //firm fisical capital
	v[2]=VL("Firm_Stock_Financial_Assets",1);                                          //firm's stock of financial assets in the last period
	if(v[1]+v[2]>0)                                                                    //if the sum of the fisical capital plus the financial assets is positive
		v[3]=v[0]/(v[1]+v[2]);                                                         //debt rate is the stock of debt over the total capital
	else                                                                               //if the sum of the fisical capital plus the financial assets is not positive
		v[3]=1.1;                                                                      //debt rate is 1.1 (dúvida, pq 1.1?)
RESULT(v[3])


EQUATION("Firm_Avg_Debt_Rate")
/*
Firm's avg debt rate of the last investment period
*/
	v[0]=V("investment_period");
	v[3]=0;																				//initializes the sum
	for (v[1]=1; v[1]<=v[0]; v[1]=v[1]+1)												//from 0 to investment period-1 lags
		{
		v[2]=VL("Firm_Debt_Rate", v[1]);												//computates firm's debt rate of the current lag
		v[3]=v[3]+v[2];																	//sum up firm's lagged effective oders
		}
	v[4]=v[3]/v[0];																		//average firm's debt rate of the last investment period
RESULT(v[4])


EQUATION("Firm_Stock_Financial_Assets")
/*
Current Value of the stock of financial assets
*/
	v[0]=V("Firm_Financial_Assets_Flow");
	v[1]=VL("Firm_Stock_Financial_Assets", 1);
	v[2]=v[0]+v[1];
	v[3]=max(0, v[2]);
RESULT(v[3])


EQUATION("Firm_Financial_Assets_Return")
/* 
Net return on firm's financial assets
*/
	v[0]=VL("Firm_Stock_Debt", 1);                                                    //firm's stock of debt in the last period
	v[1]=V("Firm_Interest_Rate");                                                     //interest rate paid by the firm
	v[2]=VL("Firm_Stock_Financial_Assets", 1);                                        //firm's stock of financial assets in the last period
	v[3]=V("Basic_Interest_Rate");                                                    //return rate on financial assets
	v[4]=v[2]*v[3]-v[0]*v[1];                                                         //net return on firm's financial assets                           
RESULT(v[4])


EQUATION("Firm_Debt_Flow")
/*
Firm Variable
*/
	v[0]=V("Firm_Retained_Profits");
	v[1]=V("Firm_Available_Debt");
	v[2]=V("Firm_Available_Financial_Assets");
	v[3]=V("Firm_Total_Funds"); 													//v[0]+v[1]+v[2]
	v[4]=V("Firm_Desired_Expansion_Investment_Expenses");
	v[5]=V("Firm_Effective_Expansion_Investment_Expenses");
	v[6]=V("Firm_Available_Funds_Replacement");
	v[7]=V("Firm_Replacement_Expenses");
	v[8]=V("Firm_Available_Funds_After_Replacement");
	v[9]=v[5]+v[7]; 																//total effective investment expenses
	v[10]=VL("Firm_Stock_Debt", 1);
	v[11]=VL("Firm_Stock_Financial_Assets", 1);
	v[12]=V("Basic_Interest_Rate");
	v[13]=V("Firm_Interest_Rate");

	if (v[3]<=0) 																	// total funds negative (no investment expenses)
		{
		if (v[11]+v[0]<0||(v[13]>v[12]&&v[10]-v[11]-v[0]>=0))						//external funds will be used to reduce debt
			v[15]=-v[11]-v[0];
		else
			v[15]=0;
		}
	else
		{
		if(v[3]<=v[4]) 																//available funds and enough for the investment
			v[15]=v[1];
		else 																		//available funds but not enought for the desired investment
			{
			if(v[1]>=0)																//if available debt is positive (may increase debt)
    			{
      			 	if(v[8]>=v[1])													//if available funds after replacement is higher than the available debt
         		 	v[15]=0;														//firm does not incur in new debt
      				else
        			v[15]=v[1]-v[8];
     			}
    			else 																//v[1]<0
    			{
     				if(v[10]+v[1]-v[8]>=0)
      				v[15]=v[1]-v[8];
     				else
     				v[15]=-v[10];
    			} 
			}
		}
RESULT(v[15])


EQUATION("Firm_Financial_Assets_Flow")
/*
Firm Variable
*/
 v[0]=V("Firm_Retained_Profits");
 v[1]=V("Firm_Available_Debt");
 v[2]=V("Firm_Available_Financial_Assets");
 v[3]=V("Firm_Total_Funds"); 															//v[0]+v[1]+v[2]
 v[4]=V("Firm_Desired_Expansion_Investment_Expenses");
 v[5]=V("Firm_Effective_Expansion_Investment_Expenses");
 v[6]=V("Firm_Available_Funds_Replacement");
 v[7]=V("Firm_Replacement_Expenses");
 v[8]=V("Firm_Available_Funds_After_Replacement");
 v[9]=v[5]+v[7]; 																		//total effective investment expenses
 v[10]=VL("Firm_Stock_Debt", 1);
 v[11]=VL("Firm_Stock_Financial_Assets", 1);
 v[12]=V("Basic_Interest_Rate");
 v[13]=V("Firm_Interest_Rate");
 
 if (v[3]<=0) 																			// total funds negative (no investment expenses)
 	{
 	if (v[11]+v[0]<0||(v[13]>v[12]&&v[10]-v[11]-v[0]>=0))								//external funds will be used to reduce debt
 		v[14]=-v[11];
 	else
 		v[14]=v[0];
 	}
 else
 	{
 	if(v[3]<=v[4]) 																		//available funds and enough for the investment
 		v[14]=-v[2]; 
 	else 																				//available funds but not enought for the desired investment
 		{
 		if(v[1]>=0)																		//if available debt is positive (may increase debt)
     		{
       		 	if(v[8]>=v[1])															//if available funds after replacement is higher than the available debt																						
          	 	v[14]=-v[2]+(v[8]-v[1]);												//the amount will be added to firm's financial assets 
       			else																	//firm does not incur in new debt
          		v[14]=-v[2];
      		}
     		else 																		//v[1]<0
     		{
      			if(v[46]+v[1]-v[8]>=0)
       			v[14]=-v[2];
      			else
      			v[14]=-v[2]+v[8]-v[1]-v[10];
     		} 
 		}
 	}
RESULT(v[14])



