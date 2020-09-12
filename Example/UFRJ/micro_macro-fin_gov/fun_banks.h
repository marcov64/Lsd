
/**********************************************************************
Bank Credit Demand and Supply
*********************************************************************/


EQUATION("Bank_Max_Total_Loans");
/*
Maximum new loans. Follows basileia or similar rule
Might impact effective loans
*/
	
	v[0]=VL("Bank_Accumulated_Profits",1);
	v[1]=V("minimum_capital_ratio");
	v[2]=V("fragility_sensitivity");
	v[3]=VL("Avg_Debt_Rate_Firms",1);
	v[4]=v[2]*v[3];
	v[6]=VL("Bank_Accumulated_Defaulted_Loans",1);
	v[7]=VL("Bank_Stock_Loans_Long_Term",1);
	if(v[7]!=0)
		v[8]=v[6]/v[7];
	else
		v[8]=0;
	v[9]=V("default_sensitivity");
	v[10]=v[9]*v[8];
	
	if(v[1]!=0)//no regulatory rule and no sensitivity
	{
		v[5]=v[0]/(v[1]+v[4]+v[10]);
		v[12]=max(0,v[5]);
	}
	else
		v[12]=-1;
RESULT(v[12])


EQUATION("Bank_Demand_Loans")
/*
Total demand for loans, firms and classes
*/
	v[10]=V("id_bank");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Demand_Loans");
			v[3]=VS(cur1, "id_firm_bank");
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
Effectrive Loans is the minimum between demanded loans and max loans.
*/
	v[0]=V("Bank_Demand_Loans");
	v[1]=V("Bank_Max_Total_Loans");
	v[2]=V("depreciation_period");
	v[4]=VL("Bank_Total_Stock_Loans",1);
	v[5]=max(0,(v[1]-v[4]));
	
	if(v[1]!=-1&&t>100)
		v[3]=min(v[0],v[5]);
	else//no regulatory rule, all demand met
		v[3]=v[0];
RESULT(max(0,v[3]))


EQUATION("Bank_Demand_Met")
/*
Share of demand for new loans met
*/
	v[0]=V("Bank_Effective_Loans");
	v[1]=V("Bank_Demand_Loans");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=1;
RESULT(v[2])


/**********************************************************************
Bank Competition
*********************************************************************/


EQUATION("Bank_Competitiveness")
/*
Bank competitiveness depends negativelly on the bank interest rate on loans and the loans not met
*/
	v[0]=VL("Bank_Interest_Rate_Long_Term",1);                  	   //bank interest rate on loans in the last period
	v[1]=V("e_interest");                                              //interest rate elasticity
	v[2]=VL("Bank_Total_Stock_Loans", 1);
	v[3]=VL("Bank_Defaulted_Loans",1);
	v[4]=VL("Bank_Competitiveness",1);                                 //bank's competitiveness in the last period
	if(v[2]!=0)
		v[5]=(v[2]-v[3])/v[2];										   //bank's loan not met in the last period
	else
		v[5]=0;
	v[8]=VL("Bank_Demand_Met",1);
	v[6]=V("e_loans");                                                 //loans not met elasticity	
   	if(v[0]!=0&&v[5]!=0)                                               //if the interest ratec was not zero neither the loans not met
     	v[7]=(1/pow(v[0],v[1]))*(pow(v[5],v[6]));   				   //bank's competitiveness will be given by the inverse of the interest rate, powered by its elasticicty, and the loans not met, powered by its elasticicty
   	else                                                               //if either the interest rate or the loans not met was zero 
     	v[7]=v[4];                                                     //bank's competitiveness will be the competitiveness in the last period
RESULT(v[7])


EQUATION("Bank_Market_Share")
/*
Bank Market Share evolves based on the bank competitiveness and sector average competitiveness
*/
	v[0]=VL("Bank_Market_Share", 1);                //bank's market share in the last period
	v[1]=V("Avg_Competitiveness_Financial_Sector"); //sector average competitiveness
	v[3]=V("Bank_Competitiveness");                 //bank's competitiveness
	if(v[1]!=0)                                     //if the sector average competitiveness is not zero
		v[4]=v[0]+v[0]*((v[3]/v[1])-1);             //bank's market share will be the last period's inscresed by the adjustment paramter times the ratio between firm's competitiveness and sector average competitiveness
	else                                            //if the sector average competitiveness is zero
		v[4]=0;                                     //bank's market share will be zero 
RESULT(v[4])


EQUATION("Bank_Desired_Long_Term_Spread")
/*
Bank Variable
*/
	v[0]=VL("Bank_Desired_Long_Term_Spread",1);                            //bank desired spread in the last period 
  	v[1]=VL("Bank_Competitiveness",1);                                     //bank's competitiveness in the last period
  	v[2]=VL("Avg_Competitiveness_Financial_Sector",1);                     //sector's average competitiveness in the last period
  	if (v[2]!=0)
  		v[3]=(v[1]-v[2])/v[2];                                             //diference between bank's competitiveness and sector's average competitiveness 
  	else
  		v[3]=0;
  	v[4]=v[0]*(1+v[3]);	                                                                  					
RESULT(max(0,v[4])) 


EQUATION("Bank_Desired_Interest_Rate_Long_Term")
/*
Bank desired interest rate on loans is the current base interest rate with a bank desired spread
*/
	v[0]=V("Bank_Desired_Long_Term_Spread");                         //bank's desired spread
	v[1]=V("Basic_Interest_Rate");                          		 //central bank interest rate
	v[2]=(1+v[0])*v[1];                                  	         //bank's desired interest rate
RESULT(v[2])


EQUATION("Bank_Interest_Rate_Long_Term")
/*
Bank's effective interest rate on loans is a average between the desired interest rate and the sector average interest rate
*/
	v[0]=VL("Bank_Interest_Rate_Long_Term",1);                                 //bank's interest rate on loans in the last period
	v[1]=V("Bank_Desired_Interest_Rate_Long_Term");                            //bank's desired interest rate on loans
	v[2]=V("parameter2");                                                      //weight parameter
	v[3]=VL("Avg_Interest_Rate_Long_Term", 1);                                 //sector average interest rate on loans in the last period
	v[4]=v[2]*(v[1])+(1-v[2])*(v[3]);                                          //firm's price is a average between the desired price and the sector average price
	if(v[1]>0)                                                                 //if desired interest rate is positive
		v[5]=max(0,v[4]);                                                      //bank's interest rate can never be negative
	else                                                                       //if desired interest rate is not positive
		v[5]=v[0];                                                             //bank's interest rate will be the last period's
RESULT(v[5])


/**********************************************************************
Bank Stocks and Flows - Collection from bank's clients
*********************************************************************/


EQUATION("Bank_Stock_Loans_Short_Term")
/*
Total Stock of short term loans, firms and classes
*/

	v[10]=V("id_bank");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Stock_Loans_Short_Term");
			v[3]=VS(cur1, "id_firm_bank");
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

	v[10]=V("id_bank");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Stock_Loans_Long_Term");
			v[3]=VS(cur1, "id_firm_bank");
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
	
	v[10]=V("id_bank");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Stock_Deposits");
			v[3]=VS(cur1, "id_firm_bank");
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
v[10]=V("id_bank");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Deposits_Return");
			v[3]=VS(cur1, "id_firm_bank");
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
	v[10]=V("id_bank");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Interest_Payment");
			v[3]=VS(cur1, "id_firm_bank");
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
	v[6]=V("Government_Interest_Payment");
	v[7]=V("Bank_Market_Share");
	v[8]=v[0]+v[7]*(v[4]+v[6]);
	
RESULT(v[8])


EQUATION("Bank_Debt_Payment")
/*
Total interest payment from firms and classes
*/
	v[10]=V("id_bank");

	v[0]=0;
	CYCLES(root, cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Debt_Payment");
			v[3]=VS(cur1, "id_firm_bank");
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
Current bank profits is the difference between Interest Return and Interest Payment
*/
v[0]=V("Bank_Interest_Receivment");
v[1]=V("Bank_Interest_Payment");
v[2]=V("Bank_Defaulted_Loans");
v[3]=v[0]-v[1]-v[2];
RESULT(v[3])


EQUATION("Bank_Distributed_Profits")
/*
Current bank profits distributed 
*/
v[0]=V("Bank_Profits");
v[1]=V("financial_sector_profits_distribution_rate");
if(v[0]>0)
	v[2]=v[0]*v[1];
else
	v[2]=0;
RESULT(v[2])


EQUATION("Bank_Accumulated_Profits")
/*
Total Stock of deposits of the financial sector
*/
v[0]=VL("Bank_Accumulated_Profits",1);
v[1]=V("Bank_Profits");
v[2]=V("Bank_Distributed_Profits");
v[4]=v[1]-v[2]+v[0];

if(v[4]<0)
{
	v[5]=V("minimum_capital_ratio");
	v[6]=V("fragility_sensitivity");
	v[7]=VL("Avg_Debt_Rate_Firms",1);
	v[8]=v[2]*v[3];
	v[9]=V("default_sensitivity");
	v[10]=VL("Bank_Accumulated_Defaulted_Loans",1);
	v[11]=VL("Bank_Stock_Loans_Long_Term",1);
	if(v[11]!=0)
		v[12]=v[10]/v[11];
	else
		v[12]=0;
	v[13]=VL("Bank_Total_Stock_Loans",1);
	v[14]=v[13]*v[5];							//minimal capital required to current stock of loans
	v[15]=-v[4]+v[14];							//central bank loans (rescue)
	WRITE("Bank_Rescue", v[15]);	
}
else
{
	v[14]=v[4];
	v[15]=0;
	WRITE("Bank_Rescue", v[15]);
}
RESULT(v[14])

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
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])


EQUATION("Bank_Leverage")
/*
Total Stock of loans over total stock of deposits
*/
v[0]=V("Bank_Total_Stock_Loans");
v[1]=V("Bank_Stock_Deposits");
if(v[1]!=0)
	v[2]=v[0]/v[1];
else
	v[2]=0;
RESULT(v[2])


EQUATION("Bank_Default_Share");
/*
Current bank defaulted loans over stock of long term loans
*/
v[0]=V("Bank_Defaulted_Loans");
v[1]=V("Bank_Stock_Loans_Long_Term");
if(v[1]!=0)
	v[2]=v[0]/v[1];
else
	v[2]=0;
RESULT(v[2])









