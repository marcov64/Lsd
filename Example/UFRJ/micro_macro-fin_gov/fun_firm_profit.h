
EQUATION("Firm_Revenue")
RESULT(V("Firm_Sales")*V("Firm_Price"))


EQUATION("Firm_Net_Revenue")
/*
Firm's net revenue, discounting taxes and R&D expenses.
*/
	v[0]=V("Firm_Revenue");
	v[1]=V("sector_indirect_tax_rate");
	v[2]=v[0]*v[1];
	v[3]=v[0]-v[2];
	WRITE("Firm_Indirect_Tax", v[2]);
RESULT(v[3])

EQUATION_DUMMY("Firm_Indirect_Tax", "Firm_Net_Revenue")


EQUATION("Firm_Interest_Payment")
/* 
Sum up total interest payment on all firm's loans. Interest rates are fixed for each loan. 
switch_interest_payment
0-->pre-fixed interest
1-->pos-fixed interest
*/
	v[0]=V("switch_interest_payment");
	v[1]=V("Firm_Interest_Rate_Long_Term");
	v[2]=V("Firm_Interest_Rate_Short_Term");
	v[3]=0;												//initializes the CYCLE
	CYCLE(cur, "FIRM_LOANS")							//CYCLE trough all firm's loans
	{
		v[4]=VS(cur, "firm_loan_total_amount");			//debt current amount 
		v[5]=VS(cur, "firm_loan_interest_rate");		//debt interest rate
		v[6]=VS(cur, "id_firm_loan_short_term");
		v[7]=VS(cur, "id_firm_loan_long_term");
		if(v[0]==0)
			v[8]=v[4]*v[5];
		if(v[0]==1)
		{
			if(v[6]==1)
				v[8]=v[4]*v[2];
			else 
				v[8]=v[4]*v[1];
		}
		v[3]=v[3]+v[8];
	}                                      			               
RESULT(v[3])								


EQUATION("Firm_Debt_Payment")
/* 
Sum up total debt payment on all firm's loans. Amortizations are fixed for each loan. 
This variable also adjusts the total amount of each loan and delete loan objects if all debt is paid.
*/
	v[0]=SUM("firm_loan_fixed_amortization");		 			//sum up all amortizations for current period
		CYCLE_SAFE(cur, "FIRM_LOANS")							//CYCLE trough all firm's loans
		{
		v[4]=VS(cur, "firm_loan_total_amount");					//debt current amount 
		v[5]=VS(cur, "firm_loan_fixed_amortization");			//debt fixed amortization
		v[6]=v[4]-v[5];											//new total amount
		v[7]=VS(cur, "firm_loan_fixed_object");					//identifies if it is fixed object, necessary for model structure
		if (v[7]!=1)
			{	
			if (v[6]>0)											//if there is still amount to be amortized
				WRITES(cur, "firm_loan_total_amount", v[6]);	//write the new amount
			else												//if all amount was already amortized
				DELETE(cur);									//delete current loan
			}
		}
RESULT(v[0])


EQUATION("Firm_Financial_Obligations")
/*
Firm Financial Obligations in the current period is the sum of interest payment and debt payment
*/
	v[1]=V("Firm_Interest_Payment");
	v[2]=V("Firm_Debt_Payment");
	v[3]=v[1]+v[2]; 
RESULT(v[3])


EQUATION("Firm_Deposits_Return")
/*
Firm interest receivment on deposits
*/
	v[0]=VL("Firm_Stock_Deposits",1);
	v[1]=V("Financial_Sector_Interest_Rate_Deposits");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Firm_Net_Profits")
/*
Firm profit, including
*/
	v[0]=V("Firm_Net_Revenue");                                       //firm's net revenue
	
	v[1]=V("Firm_Effective_Production");                              //firm's effective production
	v[2]=V("Firm_Variable_Cost");                                     //firm's variable cost	
	v[3]=v[1]*v[2];													  //production cost

	v[4]=V("sector_rnd_revenue_proportion");						  //share of net profits to allocate in R&D
	v[5]=v[0]*v[4];													  //R&d expenses
	
	v[9]=v[0]-v[3]-v[5];										  	  //firm profits
	
	v[10]=V("Firm_Interest_Payment");							 	  //firm's financial obligations
	v[11]=V("Firm_Deposits_Return");								  //firm's financial revenue
	
	v[12]=v[9]-v[10]+v[11];						  					  //firm's net profits, after interest
	
	v[13]=V("sector_profits_distribution_rate");					  //distribution rate
	if(v[12]>0)
		{
		v[14]=v[12]*v[13];											  //distributed profits
		v[15]=v[12]*(1-v[13]);										  //retained profits
		}
	else
		{
		v[14]=0;													  //do not distribute negative net profits
		v[15]=v[12];												  //retain negative net profits
		}
	
	if(v[9]!=0)											
		v[16]=v[10]/v[9];											  //financial obligations over net profits
	else	
		v[16]=0;
	
	v[17]=VL("Firm_Capital",1);
	if(v[17]!=0)
		v[18]=(v[12]-v[11])/v[17];
	else
		v[18]=0;
	
	WRITE("Firm_RND_Expenses", v[5]);
	WRITE("Firm_Profits", v[9]);
	WRITE("Firm_Liquidity_Rate", v[16]);
	WRITE("Firm_Distributed_Profits", v[14]);
	WRITE("Firm_Retained_Profits", v[15]);
	WRITE("Firm_Profit_Rate", v[18]);
RESULT(v[12])

EQUATION_DUMMY("Firm_RND_Expenses", "Firm_Net_Profits" )
EQUATION_DUMMY("Firm_Profits", "Firm_Net_Profits" )
EQUATION_DUMMY("Firm_Liquidity_Rate", "Firm_Net_Profits" )
EQUATION_DUMMY("Firm_Distributed_Profits", "Firm_Net_Profits" )
EQUATION_DUMMY("Firm_Retained_Profits", "Firm_Net_Profits")
EQUATION_DUMMY("Firm_Profit_Rate", "Firm_Net_Profits")

