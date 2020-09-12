

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


EQUATION("Firm_Interest_Rate_Short_Term")
/*
Interest rate paid by the firm depends on a individual spread over basic interest rate. The firm's spread is based on the average debt rate of the firm.
*/
	v[0]=V("risk_premium_adjustment");
	v[1]=V("Firm_Avg_Debt_Rate");
	v[2]=V("Interest_Rate_Loans_Short_Term");
	v[3]=(1+v[1]*v[0])*v[2];	
RESULT(v[3])


EQUATION("Firm_Interest_Rate_Long_Term")
/*
Interest rate paid by the firm depends on a individual spread over basic interest rate. The firm's spread is based on the average debt rate of the firm.
*/
	v[0]=V("risk_premium_adjustment");
	v[1]=V("Firm_Avg_Debt_Rate");
	v[2]=V("id_firm_bank");
	cur=SEARCH_CNDS(root, "id_bank", v[2]);
	v[3]=VS(cur, "Bank_Interest_Rate_Long_Term");
	v[4]=(1+v[1]*v[0])*v[3];	
RESULT(v[4])


EQUATION("Firm_Liquidity_Preference")
/*
Firm's desired deposits to be kept as liquid assets. 
Formulation proposed by Moreira (2010)
Proportion of physical capital. 
Evolves based on average debt rate and profit growth.
*/
	v[0]=V("investment_period");
	v[1]=V("Firm_Investment_Period");
	v[2]=V("Firm_Net_Profits");
	v[3]=VL("Firm_Net_Profits",v[0]);
	if(v[3]!=0)
		v[4]=(v[2]-v[3])/v[3];
	else
		v[4]=0;
	v[5]=V("Firm_Avg_Debt_Rate");
	v[6]=V("Firm_Desired_Debt_Rate");
	v[7]=VL("Firm_Liquidity_Preference",1);
	v[8]=V("firm_liquidity_preference_adjustment");
	
	if(v[1]==1)
	{
		if(v[4]<=0)
			v[9]=v[7]+v[8];
		else
			v[9]=v[7]-v[8];
	}
	else
			v[9]=v[7];
	
	v[10]=max(0,(min(1,v[9])));
RESULT(v[10])


EQUATION("Firm_Retained_Deposits")
/*
Firm retained profits at current period. To be summed in the stock of deposits at the end of period
Based on the liquidity preference. If liquidity preference is zero, retained deposits is zero.
Can not be negative, meaning that firm will not incur in new debt to finance retained liquidity.
*/
	v[1]=V("Firm_Retained_Profits");		//current net retained proffits
	v[2]=VL("Firm_Stock_Deposits",1);		//stock of deposits
	v[3]=v[1]+v[2];							//total internal funds (can be negative)
	v[4]=VL("Firm_Capital",1);				//current stock of capital
	v[5]=V("Firm_Liquidity_Preference");	//current liquidity preference, as a ratio of capital
	v[6]=v[4]*v[5];							//desired amount of retained deposits (always positive)
	v[7]=max(0,(min(v[6],v[3])));			//effective retained deposits can not be negative and cannot be higher than current deposits (no new debt to ratain liquidity)
RESULT(v[7])


EQUATION("Firm_Internal_Funds")
/*
Firm internal funds.
Current stock of deposits minus retained liquidity. 
Can be negative, if retained profits is negative (in that case, no retained liquidity). In that case, firm will incur in new debt to finance losses and pay for advanced production costs.
*/
	v[1]=V("Firm_Retained_Profits");		//current net retained proffits
	v[2]=VL("Firm_Stock_Deposits",1);		//stock of deposits
	v[3]=V("Firm_Retained_Deposits");		//retained deposits
	v[4]=v[1]+v[2]-v[3];					//current internal funds.
RESULT(v[4])


EQUATION("Firm_Desired_Debt_Rate")
/*
Firm's desired debt rate as a proportion to total capital. 
Formulation proposed by Moreira (2010) 
Evolves based on capacity utilization and profit growth.
*/
	v[0]=V("investment_period");
	v[1]=V("Firm_Investment_Period");
	v[2]=V("Firm_Net_Profits");
	v[3]=VL("Firm_Net_Profits",v[0]);
	if(v[3]!=0)
		v[4]=(v[2]-v[3])/v[3];
	else
		v[4]=0;
		
	v[9]=V("desired_degree_capacity_utilization");
	v[10]=VL("Firm_Desired_Debt_Rate",1);
	v[11]=V("firm_debt_rate_adjustment");
	
	if(v[1]==1)
	{
		if(v[4]>0)
			v[12]=v[10]+v[11];
		else
			v[12]=v[10]-v[11];
	}
	else
			v[12]=v[10];
	
	v[13]=max(0,(min(1,v[12])));
RESULT(v[13])


EQUATION("Firm_Available_Loans")
/*
Firm available debt depends on the difference between desired stock of debt and current stock of debt. 
If current stock of debt is greater than desired, the firm must repay some debt reducing the amount of external funds for investment. 
If the current amount is smaller than desired, that difference is available to the firm as external finance, but that does not mean that the class will increase effective debt by this amount.
*/
	v[0]=V("Firm_Desired_Debt_Rate");
	v[1]=VL("Firm_Stock_Loans",1);
	v[2]=VL("Firm_Stock_Deposits",1);
	v[3]=VL("Firm_Capital",1);
	v[4]=v[0]*(v[2]+v[3])-v[1];
	v[5]=max(0,v[4]);
RESULT(v[5])


EQUATION("Firm_Demand_Loans")
/*
Firm demand for loans is the amount that internal funds can not pay
*/
	v[0]=V("Firm_Desired_Investment_Expenses");
	v[1]=V("Firm_Internal_Funds");
	v[2]=V("Firm_Available_Loans");
	if(v[2]>0)															//if there is available debt 
		{
		v[3]=v[0]-v[1];													//will demand loans for the amount of desired expenses that internal funds can not pay for
		v[4]=min(v[3],v[2]);											//demand will be the minimum between amount needed and amount available
		}
	else																//if there is no available debt
		v[4]=0;															//no demand for debt
	v[5]=max(0,v[4]);													//demand for new loans can not be negative
RESULT(v[5])


EQUATION("Firm_Effective_Loans")
/*
Firm effective loans is the amount demanded.
This variable also creates the object LOAN.
It is assumed that the firm has 2 investment period to repay. This can be changed
*/
	v[0]=V("Firm_Demand_Loans");
	v[1]=V("Firm_Investment_Period");
	v[2]=V("annual_period");
	v[3]=V("Firm_Interest_Rate_Short_Term");
	v[4]=V("investment_period");
	v[5]=V("Firm_Interest_Rate_Long_Term");
	v[6]=V("Financial_Sector_Demand_Met");
	v[7]=v[0]*v[6];
	v[8]=V("firm_effective_loans");
	v[9]=V("amortization_period");
	
	cur = ADDOBJ("FIRM_LOANS");
	WRITES(cur, "firm_loan_total_amount", v[8]);
	WRITES(cur, "firm_loan_fixed_object", 0);
	if(v[1]==1)
	{
		WRITES(cur, "firm_loan_fixed_amortization", (v[8]/v[9]));
		WRITES(cur, "id_firm_loan_long_term", 1);
		WRITES(cur, "firm_loan_interest_rate", v[5]);
	}
	else
	{
		WRITES(cur, "firm_loan_fixed_amortization", (v[8]/v[2]));
		WRITES(cur, "id_firm_loan_short_term", 1);
		WRITES(cur, "firm_loan_interest_rate", v[3]);
	}
RESULT(v[8])


EQUATION("Firm_Credit_Rationing")
/*
Ex post variable for analysis.
Ratio between effective and demanded loans.
*/
v[0]=V("Firm_Effective_Loans");
v[1]=V("Firm_Demand_Loans");
if(v[1]!=0)
	v[2]=v[0]/v[1];
else	
	v[2]=0;
RESULT(v[2])



EQUATION("Firm_Total_Funds")
/*
Total available funds for the firm in the current period is the internal funds plus effective new loans
*/
	v[0]=V("Firm_Internal_Funds");
	v[1]=V("Firm_Effective_Loans");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Firm_Stock_Loans_Short_Term")
/*
Firm Stock of Short Term Loans
*/
	v[0]=0;
	CYCLE(cur, "FIRM_LOANS")
	{
		v[1]=VS(cur, "id_firm_loan_short_term");
		if (v[1]==1)
			v[2]=VS(cur, "firm_loan_total_amount");
		else
			v[2]=0;
	v[0]=v[0]+v[2];
	}
RESULT(v[0])


EQUATION("Firm_Stock_Loans_Long_Term")
/*
Firm Stock of Long Term Loans
*/
	v[0]=0;
	CYCLE(cur, "FIRM_LOANS")
	{
		v[1]=VS(cur, "id_firm_loan_long_term");
		if (v[1]==1)
			v[2]=VS(cur, "firm_loan_total_amount");
		else
			v[2]=0;
	v[0]=v[0]+v[2];
	}
RESULT(v[0])


EQUATION("Firm_Stock_Loans")
/*
Firm Stock of Debt, short term +  long term
*/
	v[0]=V("Firm_Stock_Loans_Long_Term");
	v[1]=V("Firm_Stock_Loans_Short_Term");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Firm_Stock_Deposits")
/*
Firm Stock of Deposits
*/
	v[0]=V("Firm_Available_Funds_After_Replacement");
	v[1]=V("Firm_Retained_Deposits");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Firm_Debt_Rate")
/*
Degree of indebtedness, calculated by the ratio of the debt to the capital of the sector.
*/
	v[0]=V("Firm_Stock_Loans");                                                         //firm's stock of debt 
	v[1]=V("Firm_Capital");                                                            //firm fisical capital
	v[2]=VL("Firm_Stock_Deposits",1);                                          			//firm's stock of financial assets in the last period
	if(v[1]+v[2]>0)                                                                    //if the sum of the fisical capital plus the financial assets is positive
		v[3]=v[0]/(v[1]+v[2]);                                                         //debt rate is the stock of debt over the total capital
	else                                                                               //if the sum of the fisical capital plus the financial assets is not positive
		v[3]=1.1;                                                                      //debt rate is 1.1 (dúvida, pq 1.1?)
RESULT(v[3])


EQUATION("Firm_Extra_Debt_Payment")
/*
Firm's extra amortization. 
Formulation proposed by Moreira (2010) with some differences, since our loan's amortization is fixed.
Starting from zero (firm only repay fixed amortizations), can increase or decrease this rate.
Affects "Firm_Debt_Payment".
Evolves based on average debt rate and profit growth.
*/
	v[0]=V("investment_period");
	v[1]=V("Firm_Investment_Period");
	v[2]=VL("Firm_Net_Profits",1);
	v[3]=VL("Firm_Net_Profits",v[0]);
	if(v[3]!=0)
		v[4]=(v[2]-v[3])/v[3];
	else
		v[4]=0;
	v[5]=V("Firm_Avg_Debt_Rate");
	v[6]=VL("Firm_Desired_Debt_Rate",1);
	v[7]=VL("Firm_Extra_Debt_Payment",1);
	v[8]=V("extra_debt_payment_adjustment");
	
	if(v[1]==1)
	{
		if(v[5]>=v[6])
			v[9]=v[7]+v[8];
		else
			v[9]=v[7]-v[8];
	}
	else
			v[9]=v[7];
	
	v[10]=max(0,(min(1,v[9])));
RESULT(v[10])


EQUATION("Firm_Financial_Position")
/*
This variable writes parameters for the current financial position of the firm.
*/
	v[0]=V("Firm_Net_Revenue");                                       //firm's net revenue
	v[1]=V("Firm_Effective_Production");                              //firm's effective production
	v[2]=V("Firm_Variable_Cost");                                     //firm's variable cost	
	v[3]=V("Firm_Deposits_Return");
	v[4]=v[0]-v[1]*v[2];
	
	v[5]=V("Firm_Interest_Payment");
	v[6]=V("Firm_Debt_Payment");
	v[7]=V("Firm_Financial_Obligations");
	
	if (v[4]>=v[7])
	{
		WRITE("firm_hedge",1);
		WRITE("firm_speculative", 0);
		WRITE("firm_ponzi",0);
		v[9]=0;
	}
	
	if (v[4]<v[7]&&v[4]>=v[5])
	{
		WRITE("firm_hedge",0);
		WRITE("firm_speculative", 1);
		WRITE("firm_ponzi",0);
		v[9]=1;
	}
	
	if (v[4]<v[7]&&v[4]<v[5])
	{
		WRITE("firm_hedge",0);
		WRITE("firm_speculative", 0);
		WRITE("firm_ponzi",1);
		v[9]=2;
	}
	
	if(v[7]!=0)
		v[8]=v[4]/v[7];
	else
		v[8]=1;
RESULT(v[9])





