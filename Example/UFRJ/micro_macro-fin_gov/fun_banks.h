
/**********************************************************************
Bank Credit Demand and Supply
*********************************************************************/


EQUATION("Bank_Max_Total_Loans");
/*
Maximum stock of loans. Follows basileia or similar rule
Might impact effective loans
*/
	v[0]=VL("Bank_Accumulated_Profits",1);								//bank's accumulated profits (net worth) in the last period
	v[1]=V("cb_minimum_capital_ratio");									//minimum capital ratio defined by the regulatory rule
	v[2]=V("fs_sensitivity_debt_rate");									//bank's sensitivity to overall indebtedness of the economy
	v[3]=VL("Country_Debt_Rate_Firms",1);								//average debt rate of firms of the economy
	v[4]=VL("Bank_Default_Share",1);									//bank's share of accumulated defaulted loans over total loans
	v[5]=V("fs_sensitivity_default");									//bank's sensitivity to its own default ratio
	v[6]=v[1]+v[2]*v[3]+v[5]*v[4];										//desired share of net worth in relation to total loans
	if(v[6]!=0)															//if desired share is not zero												
		v[7]=v[0]/v[6];													//maximum stock of loans to meet the desired share
	else																//if desired share is zero
		v[7]=-1;														//no limit to stock of loans
RESULT(v[7])


EQUATION("Bank_Demand_Loans")
/*
Total demand for loans, firms and classes
*/
	v[10]=V("bank_id");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Demand_Loans");
			v[3]=VS(cur1, "firm_bank");
			if(v[3]==v[10])
				v[1]=v[1]+v[2];
			else
				v[1]=v[1];
		}
		v[0]=v[0]+v[1];
	}
	
	v[4]=0;
	CYCLES(root, cur, "CLASSES")
	{
		v[5]=VS(cur, "Class_Demand_Loans");
		v[4]=v[4]+v[5];
	}
	v[6]=V("Bank_Market_Share");
	v[7]=v[0]+v[6]*v[4];
	
RESULT(v[7])


EQUATION("Bank_Effective_Loans")
/*
Effective Loans is the minimum between demanded loans and max loans.
*/
	v[0]=V("Bank_Demand_Loans");
	v[1]=V("Bank_Max_Total_Loans");
	v[2]=V("sector_capital_duration");
	v[3]=VL("Bank_Total_Stock_Loans",1);
	v[4]=max(0,(v[1]-v[3]));
	v[6]=V("begin_credit_rationing");
	if(t>v[6]&&v[6]!=-1&&v[1]!=-1)
		v[7]=min(v[0],v[4]);
	else										
		v[7]=v[0];
RESULT(max(0,v[7]))


EQUATION("Bank_Demand_Met")
/*
Share of demand for new loans met
*/
	v[0]=V("Bank_Effective_Loans");
	v[1]=V("Bank_Demand_Loans");
	v[2]= v[1]!=0? v[0]/v[1] : 1;
RESULT(v[2])


/**********************************************************************
Bank Competition
*********************************************************************/

EQUATION_DUMMY("Bank_Number_Clients", "Loans_Distribution_Firms")


EQUATION("Bank_Competitiveness")
/*
Bank competitiveness depends negativelly on the bank interest rate on loans and the loans not met
*/
	v[0]=VL("Bank_Competitiveness",1);                                 							//bank's competitiveness in the last period
	v[1]=VL("Bank_Default_Share",1);                                  							//bank share of defaulted loans
	v[2]=VL("Bank_Demand_Met",1);                                      							//bank demand not met
	v[3]=VL("Bank_Interest_Rate_Long_Term",1);                  	   							//bank interest rate on loans in the last period
	v[4]=VL("Bank_Interest_Rate_Short_Term",1);                  	   							//bank interest rate on short term loans in the last period
	
	v[5]=V("fs_elasticity_default");
	v[6]=V("fs_elasticity_rationing");  
	v[7]=V("fs_elasticity_interest_long_term");	
	v[8]=V("fs_elasticity_interest_short_term");	
   	if(v[2]!=0&&v[3]!=0&&v[4]!=0)                                      							//if competitiveness' determinants are not zero
     	v[9]=(pow((1-v[1]),v[5]))*(pow(v[2],v[6]))*(1/pow(v[3],v[7]))*(1/pow(v[4],v[8])); 
   	else                                                               							//if either the interest rate or the loans not met was zero 
     	v[9]=v[0];                                                     							//bank's competitiveness will be the competitiveness in the last period
RESULT(v[9])


EQUATION("Bank_Market_Share")
/*
Bank Market Share evolves based on the bank competitiveness and sector average competitiveness
*/
	v[0]=VL("Bank_Market_Share", 1);                										//bank's market share in the last period
	v[1]=V("Financial_Sector_Avg_Competitiveness"); 										//sector average competitiveness
	v[3]=V("Bank_Competitiveness");                 										//bank's competitiveness
	v[4]= v[1]!=0? v[0]+v[0]*((v[3]/v[1])-1) : 0;             								//bank's market share will be the last period's inscresed by the adjustment paramter times the ratio between firm's competitiveness and sector average competitiveness
RESULT(v[4])


EQUATION("Bank_Desired_Long_Term_Spread")
/*
Bank Variable
*/
	v[0]=VL("Bank_Desired_Long_Term_Spread",1);                            //bank desired spread in the last period 
  	v[1]=VL("Bank_Competitiveness",1);                                     //bank's competitiveness in the last period
  	v[2]=VL("Financial_Sector_Avg_Competitiveness",1);                     //sector's average competitiveness in the last period
  	v[3]=V("fs_spread_long_term_adjustment");							   //determines how much desired spread is adjusted
	v[4]= v[2]!=0? (v[1]-v[2])/v[2] : 0;                                   //diference between bank's competitiveness and sector's average competitiveness 
  	v[5]=v[0]*(1+v[3]*v[4]);	
RESULT(max(0,v[5])) 


EQUATION("Bank_Desired_Interest_Rate_Long_Term")
/*
Bank desired interest rate on loans is the current base interest rate with a bank desired spread
*/
	v[0]=V("Bank_Desired_Long_Term_Spread");                        			 //bank's desired spread
	v[1]=V("Central_Bank_Basic_Interest_Rate");                          		 //central bank interest rate
	v[2]=v[0]+v[1];                                  	         				 //bank's desired interest rate
RESULT(v[2])


EQUATION("Bank_Interest_Rate_Long_Term")
/*
Bank's effective interest rate on loans is a average between the desired interest rate and the sector average interest rate
*/
	v[0]=VL("Bank_Interest_Rate_Long_Term",1);                                 //bank's interest rate on loans in the last period
	v[1]=V("Bank_Desired_Interest_Rate_Long_Term");                            //bank's desired interest rate on loans
	v[2]=V("fs_price_strategy_long_term");                                     //weight parameter for long term interest rates
	v[3]=VL("Financial_Sector_Avg_Interest_Rate_Long_Term", 1);                //sector average interest rate on loans in the last period
	v[4]=V("Central_Bank_Basic_Interest_Rate");                          	   //central bank interest rate
	v[5]=v[2]*(v[1])+(1-v[2])*(v[3]);                                      	   //bank's interest rate is a average between the desired and the sector average 
RESULT(max(0,v[5]))


EQUATION("Bank_Desired_Short_Term_Spread")
/*
Bank Variable
*/
	v[0]=VL("Bank_Desired_Short_Term_Spread",1);                           //bank desired spread in the last period 
  	v[1]=VL("Bank_Competitiveness",1);                                     //bank's competitiveness in the last period
  	v[2]=VL("Financial_Sector_Avg_Competitiveness",1);                     //sector's average competitiveness in the last period
  	v[3]=V("fs_spread_short_term_adjustment");							   //determines how much desired spread is adjusted
	v[4]=v[2]!=0? (v[1]-v[2])/v[2] : 0;                                    //diference between bank's competitiveness and sector's average competitiveness 
  	v[5]=v[0]*(1+v[3]*v[4]);	
RESULT(max(0,v[5])) 


EQUATION("Bank_Desired_Interest_Rate_Short_Term")
/*
Bank desired interest rate on loans is the current base interest rate with a bank desired spread
*/
	v[0]=V("Bank_Desired_Short_Term_Spread");                       			 //bank's desired spread
	v[1]=V("Central_Bank_Basic_Interest_Rate");                          		 //central bank interest rate
	v[2]=v[0]+v[1];
RESULT(max(0,v[2]))


EQUATION("Bank_Interest_Rate_Short_Term")
/*
Bank's effective interest rate on loans is a average between the desired interest rate and the sector average interest rate
*/
	v[0]=VL("Bank_Interest_Rate_Short_Term",1);                                 //bank's interest rate on loans in the last period
	v[1]=V("Bank_Desired_Interest_Rate_Short_Term");                            //bank's desired interest rate on loans
	v[2]=V("fs_price_strategy_short_term");                                     //weight parameter for short term interest rates
	v[3]=VL("Financial_Sector_Avg_Interest_Rate_Short_Term", 1);                //sector average interest rate on loans in the last period
	v[4]=V("Central_Bank_Basic_Interest_Rate");                          		//central bank interest rate
	v[5]=v[2]*(v[1])+(1-v[2])*(v[3]);                                      		//bank's interest rate is a average between the desired and the sector average 
RESULT(max(0,v[5]))


/**********************************************************************
Bank Stocks and Flows - Collection from bank's clients
*********************************************************************/


EQUATION("Bank_Stock_Loans_Short_Term")
/*
Total Stock of short term loans, firms and classes
*/

	v[10]=V("bank_id");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Stock_Loans_Short_Term");
			v[3]=VS(cur1, "firm_bank");
			if(v[3]==v[10])
				v[1]=v[1]+v[2];
			else
				v[1]=v[1];
		}
		v[0]=v[0]+v[1];
	}
	
	v[4]=0;
	CYCLES(root, cur, "CLASSES")
	{
		v[5]=VS(cur, "Class_Stock_Loans");
		v[4]=v[4]+v[5];
	}
	v[6]=V("Bank_Market_Share");
	v[7]=v[0]+v[6]*v[4];
	
RESULT(v[7])


EQUATION("Bank_Stock_Loans_Long_Term")
/*
Total Stock of short term loans, firms and classes
*/

	v[10]=V("bank_id");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Stock_Loans_Long_Term");
			v[3]=VS(cur1, "firm_bank");
			if(v[3]==v[10])
				v[1]=v[1]+v[2];
			else
				v[1]=v[1];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Bank_Total_Stock_Loans")
/*
Total Stock of loans
*/
	v[0]=V("Bank_Stock_Loans_Long_Term");
	v[1]=V("Bank_Stock_Loans_Short_Term");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Bank_Stock_Deposits")
/*
Total Stock of deposits, firms and classes
*/
	
	v[10]=V("bank_id");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Stock_Deposits");
			v[3]=VS(cur1, "firm_bank");
			if(v[3]==v[10])
				v[1]=v[1]+v[2];
			else
				v[1]=v[1];
		}
		v[0]=v[0]+v[1];
	}
	
	v[4]=0;
	CYCLES(root, cur, "CLASSES")
	{
		v[5]=VS(cur, "Class_Stock_Deposits");
		v[4]=v[4]+v[5];
	}
	v[6]=V("Bank_Market_Share");
	v[7]=v[0]+v[6]*v[4];
	
RESULT(v[7])


EQUATION("Bank_Interest_Payment")
/*
Bank Interest Return
*/
v[10]=V("bank_id");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Deposits_Return");
			v[3]=VS(cur1, "firm_bank");
			if(v[3]==v[10])
				v[1]=v[1]+v[2];
			else
				v[1]=v[1];
		}
		v[0]=v[0]+v[1];
	}
	
	v[4]=0;
	CYCLES(root, cur, "CLASSES")
	{
		v[5]=VS(cur, "Class_Deposits_Return");
		v[4]=v[4]+v[5];
	}
	v[6]=V("Bank_Market_Share");
	v[7]=v[0]+v[6]*v[4];
	
RESULT(v[7])


EQUATION("Bank_Interest_Receivment")
/*
Total interest payment from firms and classes
*/
	v[10]=V("bank_id");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Interest_Payment");
			v[3]=VS(cur1, "firm_bank");
			if(v[3]==v[10])
				v[1]=v[1]+v[2];
			else
				v[1]=v[1];
		}
		v[0]=v[0]+v[1];
	}
	
	v[4]=0;
	CYCLES(root, cur, "CLASSES")
	{
		v[5]=VS(cur, "Class_Interest_Payment");
		v[4]=v[4]+v[5];
	}
	v[6]=V("Bank_Market_Share");
	v[7]=v[0]+v[6]*v[4];
	
RESULT(v[7])


EQUATION("Bank_Debt_Payment")
/*
Total interest payment from firms and classes
*/
	v[10]=V("bank_id");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Debt_Payment");
			v[3]=VS(cur1, "firm_bank");
			if(v[3]==v[10])
				v[1]=v[1]+v[2];
			else
				v[1]=v[1];
		}
		v[0]=v[0]+v[1];
	}
	
	v[4]=0;
	CYCLES(root, cur, "CLASSES")
	{
		v[5]=VS(cur, "Class_Debt_Payment");
		v[4]=v[4]+v[5];
	}
	v[6]=V("Bank_Market_Share");
	v[7]=v[0]+v[6]*v[4];
	
RESULT(v[7])


/**********************************************************************
Bank Profits
*********************************************************************/


EQUATION("Bank_Defaulted_Loans");
/*
Current bank defaulted loans 
*/
	v[0]=V("bank_defaulted_loans_temporary");
	WRITE("bank_defaulted_loans_temporary", 0);
RESULT(v[0])


EQUATION("Bank_Accumulated_Defaulted_Loans");
/*
Current bank defaulted loans 
*/
	v[0]=VL("Bank_Accumulated_Defaulted_Loans",1);
	v[1]=V("Bank_Defaulted_Loans");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Bank_Profits")
/*
Current bank profits is the difference between Interest Return and Interest Payment, minus defaulted loans. can be negative
*/
	v[0]=V("Bank_Interest_Receivment");
	v[1]=V("Bank_Interest_Payment");
	v[2]=V("Bank_Defaulted_Loans");
	v[3]=v[0]-v[1]-v[2];
	v[4]=V("Government_Interest_Payment");
	v[5]=V("Bank_Market_Share");
	v[6]=v[4]*v[5];
RESULT(v[3]+v[6])


EQUATION("Bank_Distributed_Profits")

v[0]=V("Bank_Profits");
if(v[0]<=0)																//losses
{
	v[1]=0;																//distribution is zero
	v[2]=v[0];															//losses are fully internalized
}
else
{

	v[3]=VL("Bank_Accumulated_Profits",1);
	v[4]=V("Bank_Total_Stock_Loans");
	v[5]=V("cb_minimum_capital_ratio");									//minimum capital ratio defined by the regulatory rule
	v[6]=V("fs_sensitivity_debt_rate");									//bank's sensitivity to overall indebtedness of the economy
	v[7]=V("Country_Debt_Rate_Firms");									//average debt rate of firms of the economy
	v[8]=V("Bank_Default_Share");										//bank's share of accumulated defaulted loans over total loans
	v[9]=V("fs_sensitivity_default");									//bank's sensitivity to its own default ratio
	v[10]=(v[5]+v[9]*v[8]+v[6]*v[7]);
	v[11]=v[4]*v[10];													//needed accumulated profits
	v[12]=LAG_GROWTH(p,"Bank_Demand_Loans",1);
	v[13]=V("fs_expectations");
	v[15]=V("Bank_Demand_Loans");
	v[14]=(v[4]+v[15]*(1+v[13]*v[12]))*v[10];
	if(v[14]<=v[3])														//if what is needed is lower than what the bank already has
		{
		v[1]=v[0];														//distribute everything
		v[2]=0;															//retain nothing
		}
	else
		{
		v[2]=min(v[0],(v[14]-v[3]));									//retain the needed difference, limited to current profits
		v[1]=v[0]-v[2];													//distribute the rest
		}

}
WRITE("Bank_Retained_Profits", v[2]);
RESULT(v[1])

EQUATION_DUMMY("Bank_Retained_Profits", "Bank_Distributed_Profits")


EQUATION("Bank_Accumulated_Profits")
/*
Total Stock of deposits of the financial sector
*/
v[0]=VL("Bank_Accumulated_Profits",1);
v[1]=V("Bank_Retained_Profits");
v[2]=v[1]+v[0];

if(v[2]<0)
{
	v[3]=V("cb_minimum_capital_ratio");
	v[4]=VL("Bank_Total_Stock_Loans",1);
	v[5]=v[4]*v[3];							//minimal capital required to current stock of loans
	v[6]=-v[2]+v[5];						//central bank loans (rescue)
}
else
{
	v[5]=v[2];
	v[6]=0;
}
WRITE("Bank_Rescue", v[6]);
RESULT(v[5])

EQUATION_DUMMY("Bank_Rescue", "Bank_Accumulated_Profits")


/**********************************************************************
Bank Analysis
*********************************************************************/


EQUATION("Bank_Short_Term_Rate");
/*
Share of short term loans over total loans
*/
	v[0]=V("Bank_Stock_Loans_Short_Term");
	v[1]=V("Bank_Total_Stock_Loans");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Bank_Leverage")
/*
Total Stock of loans over total stock of deposits
*/
	v[0]=V("Bank_Total_Stock_Loans");
	v[1]=V("Bank_Stock_Deposits");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Bank_Default_Share");
/*
Current bank defaulted loans over stock of long term loans
*/
	v[0]=V("Bank_Accumulated_Defaulted_Loans");
	v[1]=V("Bank_Stock_Loans_Long_Term");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(min(1,v[2]))


EQUATION("Bank_Effective_Capital_Ratio");
/*
Current bank accumulated profits over stock of loans
*/
	v[0]=V("Bank_Accumulated_Profits");
	v[1]=V("Bank_Total_Stock_Loans");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Bank_Effective_Profit_Rate");
/*
Effective profit rate on total loans
*/
	v[0]=V("Bank_Profits");
	v[1]=V("Bank_Total_Stock_Loans");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])





