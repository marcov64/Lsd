/*****INCOME CLASSES*****/ 


EQUATION("Class_Avg_Real_Income")
RESULT(LAG_AVE(p, "Class_Real_Disposable_Income", V("annual_frequency"),1))

EQUATION("Class_Avg_Nominal_Income")
RESULT(LAG_AVE(p, "Class_Nominal_Disposable_Income", V("annual_frequency"),1))


EQUATION("Class_Real_Autonomous_Consumption")
/*
Class autonomous consumption depends on the average quality growth of the consumption goods sector
*/
v[0]=CURRENT;                 										//class autonomous consumption in the last period
v[1]=V("annual_frequency");										    //defines the class adjustment period 
v[2]= fmod((double) t,v[1]);										//divides time period by the class period and takes the rest
if (v[2]==0)														//if it is class adjustment period 	
	{
	v[3]=LAG_GROWTH(consumption, "Sector_Avg_Quality", v[1], 1);
	v[4]=V("class_autonomous_consumption_adjustment");				//autonomous consumption adjustment parameter
		if(v[3]>0) 													//if quality growth was positive
			v[5]=v[0]*(1+v[4]*v[3]); 								//increase autonomous consumption by the adjustment parameter
		if(v[3]==0)													//if quality grwoth was zero
			v[5]=v[0];												//use last period autonomous consumption
		if(v[3]<0)													//if quality growth was negative
			v[5]=v[0]*(1+v[4]*v[3]);							 	//decrease autonomous consumption by the adjustment parameter
  }
else																//if it is not class adjustment period
	v[5]=v[0]; 														//use last period autonomous consumption                                                                       		
RESULT(max(0,v[5]))


EQUATION("Class_Imports_Share")
	v[1]=V("class_initial_propensity_import");							//class propensity to import
	v[3]=VS(consumption, "Sector_Avg_Price");                       //consumption sector average price
	v[4]=VS(consumption, "Sector_External_Price");                  //consumption sector external price
	v[5]=VS(external,"Country_Exchange_Rate");						//exchange rate
	v[6]=V("class_import_elasticity_price");
	v[7]=v[1]*pow((v[3]/(v[4]*v[5])),v[6]);
	v[8]=max(0,min(v[7],1));
RESULT(v[8])


EQUATION("Class_Real_Desired_Domestic_Consumption")
/*
Class real domestic conumption is based on average past real disposable income from profits and wages and on the class' propensity to consume, plus autonomous consumption
*/
	v[0]=V("Class_Avg_Real_Income");
	v[1]=V("class_propensity_to_spend");          				//class propensity to consume on income
  	v[2]=V("Class_Real_Autonomous_Consumption");    			//class autonomous consumption
	v[4]=V("Class_Imports_Share");
  	v[3]=v[0]*v[1]*(1-v[4])+v[2];                            	//class real desired consumption
RESULT(v[3])


EQUATION("Class_Real_Desired_Imported_Consumption")
	
	v[0]=V("Class_Avg_Real_Income");
	v[1]=V("class_propensity_to_spend");						//class propensity to import
	v[2]=V("Class_Imports_Share");
	v[3]=v[0]*v[1]*v[2];
RESULT(v[3])
	

EQUATION("Class_Desired_Expenses")
/*
Class' nominal desired expenses depends on effective domestic consumption times avg price plus effective external consumption times foreign price
*/
	v[0]=V("Class_Real_Desired_Domestic_Consumption");              // class desired domestic consumption
	v[1]=V("Class_Real_Desired_Imported_Consumption");              // class desired external consumption
	v[2]=VS(external,"Country_Exchange_Rate");                      //exchange rate
	v[3]=VS(consumption, "Sector_Avg_Price");                       //sector average price
	v[4]=VS(consumption, "Sector_External_Price");                  //sector external price
	v[5]=v[0]*v[3] + v[1]*v[2]*v[4];     					 		//total nominal expenses                  	
RESULT(v[5])


EQUATION("Class_Avg_Debt_Rate")
/*
Class avg debt rate of the last class period (equal to annual period)
*/
RESULT(LAG_AVE(p, "Class_Debt_Rate", V("annual_frequency"),1))


EQUATION("Class_Interest_Rate")
/*
Interest rate paid by the class depends on a specific spread over basic interest rate, based on the average debt rate of the class.
If risk premium is zero, the interest rate will be the same for all classes
*/
	v[0]=VS(financial,"fs_risk_premium_class");										//class risk premium defined by the banks							
	v[1]=V("Class_Avg_Debt_Rate");													//class avg debt rate											
	v[2]=VS(financial,"Financial_Sector_Avg_Interest_Rate_Short_Term");				//avg base interest rate on short term loans
	v[3]=(1+v[1]*v[0])*v[2];
RESULT(v[3])


EQUATION("Class_Interest_Payment")
/* 
Sum up total interest payment on all class' loans. Interest rates can be fixed or flexible

switch_interest_payment
0-->fixed interest, defined when the loan was taken
1-->flexible interest, the rate is calculated evert period

*/
	v[0]=0;												//initializes the CYCLE
	v[4]=V("switch_interest_payment");
	v[5]=V("Class_Interest_Rate");
	CYCLE(cur, "CLASS_LOANS")							//CYCLE trough all class' loans
	{
		v[1]=VS(cur, "class_loan_total_amount");		//debt current amount 
		v[2]=VS(cur, "class_loan_interest_rate");		//debt interest rate
		if(v[4]==0)										//if fixed interest rate
			v[3]=v[1]*v[2];								//current debt interest payment
		if(v[4]==1)										//i flexible interest rate
			v[3]=v[1]*v[5];								//current debt interest payment
		v[0]=v[0]+v[3];									//sum up interest payment of all loans
	}
RESULT(max(0,v[0]))										//can not be negative


EQUATION("Class_Debt_Payment")
/* 
Sum up total debt payment on all class' loans. Amortizations are fixed for each loan. 
This variable also adjusts the total amount of each loan and delete loan objects if all debt is paid.
*/
	v[0]=SUM("class_loan_fixed_amortization");		 			//sum up all amortizations for current period
	
		CYCLE_SAFE(cur, "CLASS_LOANS")							//CYCLE trough all class' loans
		{
		v[4]=VS(cur, "class_loan_total_amount");				//debt current amount 
		v[5]=VS(cur, "class_loan_fixed_amortization");			//debt fixed amortization
		v[6]=v[4]-v[5];											//new total amount
		v[7]=VS(cur, "class_loan_fixed_object");				//identifies if it is fixed object, necessary for model structure
		if (v[7]!=1)
			{	
			if (v[6]>0)											//if there is still amount to be amortized
				WRITES(cur, "class_loan_total_amount", v[6]);	//write the new amount
			else												//if all amount was already amortized
				DELETE(cur);									//delete current loan
			}
		}
RESULT(max(0,v[0]))		


EQUATION("Class_Financial_Obligations")
/*
Class Financial Obligations in the current period is the sum of interest payment and debt payment
*/
	v[1]=V("Class_Interest_Payment");
	v[2]=V("Class_Debt_Payment");
	v[3]=v[1]+v[2]; 
RESULT(v[3])


EQUATION("Class_Liquidity_Preference")
/*
Class' desired deposits to be kept as liquid assets. 
Inspired by Moreira (2010)
Proportion of average nominal income capital. 
Evolves based on average debt rate and income growth.
*/
	v[0]=V("annual_frequency");
	v[1]=fmod((double)t,v[0]);
	v[4]=LAG_GROWTH(p, "Class_Nominal_Disposable_Income", v[0], 1);
	v[5]=VL("Class_Debt_Rate",1);
	v[6]=V("Class_Max_Debt_Rate");
	v[7]=CURRENT;
	v[8]=V("class_liquidity_preference_adjustment");
	
	if(v[1]==1)
	{
		if(v[4]<0&&v[5]>v[6])
			v[9]=v[7]+v[8];
		if(v[4]>0&&v[5]<v[6])
			v[9]=v[7]-v[8];
		else
			v[9]=v[7];
	}
	else
			v[9]=v[7];
	
	v[10]=max(0,(min(1,v[9])));
RESULT(v[10])


EQUATION("Class_Retained_Deposits")
/*
Class retained deposits at current period. To be summed in the stock of deposits at the end of period
Based on the liquidity preference. If liquidity preference is zero, retained deposits is zero.
Can not be negative, meaning that class will not incur in new debt to finance retained liquidity.
*/
	v[1]=V("Class_Financial_Obligations");	//current financial obligations
	v[2]=VL("Class_Stock_Deposits", 1);		//stock of deposits
	v[8]=VL("Class_Nominal_Disposable_Income",1);
	v[3]=max(0, (v[2]+v[8]-v[1]));			//current internal funds
	v[4]=V("Class_Avg_Nominal_Income");		//current average income
	v[5]=V("Class_Liquidity_Preference");	//current liquidity preference, as a ratio of capital
	v[6]=v[4]*v[5];							//desired amount of retained deposits (always positive)
	v[7]=max(0,(min(v[6],v[3])));			//effective retained deposits can not be negative and cannot be higher than current deposits (no new debt to ratain liquidity)
RESULT(v[7])


EQUATION("Class_Internal_Funds")
/*
Total available funds for class expenses in the current period
*/
	v[0]=V("Class_Financial_Obligations");				//class financial obligations
	v[1]=VL("Class_Stock_Deposits", 1);					//current stock of deposits
	v[4]=VL("Class_Nominal_Disposable_Income",1);
	v[2]=V("Class_Retained_Deposits");
	v[3]=v[4]+v[1]-v[0]-v[2];							//available deposits to use as internal funds	
RESULT(v[3])


EQUATION("Class_Max_Debt_Rate")
/*
Class desired debt rate. 
Formulation proposed by Moreira (2010) 
Evolves based on nominal income growth.
*/
	v[0]=V("annual_frequency");
	v[1]=fmod((double)t,v[0]);
	v[4]=LAG_GROWTH(p, "Class_Nominal_Disposable_Income", v[0], 1);
	v[5]=CURRENT;
	v[6]=V("class_debt_rate_adjustment");	
	if(v[1]==1)
	{
		if(v[4]>0)
			v[7]=v[5]+v[6];
		if(v[4]<0)
			v[7]=v[5]-v[6];
		else
			v[7]=v[5];
	}
	else
		v[7]=v[5];
	v[8]=max(0,(min(1,v[7])));
RESULT(v[7])


EQUATION("Class_Max_Loans")
/*
Class available debt depends on the difference between desired stock of debt and current stock of debt. 
If current stock of debt is greater than desired, the class must repay some debt reducing the amount of external funds for investment. 
If the currest amount is smaller than desired, that difference is available to the class as external finance, but that does not mean that the class will increase effective debt by this amount.
*/
	v[0]=V("Class_Max_Debt_Rate");
	v[1]=VL("Class_Stock_Loans",1);
	v[2]=VL("Class_Stock_Deposits",1);
	v[3]=VL("Class_Avg_Nominal_Income",1);
	v[4]=v[0]*(v[2]+v[3])-v[1];
	v[5]=max(0,v[4]);
RESULT(v[5])


EQUATION("Class_Demand_Loans")
/*
Class demand for loans is the amount that internal funds (already discounted required debt payment) can not pay
*/
	v[0]=V("Class_Desired_Expenses");
	v[1]=V("Class_Internal_Funds");
	v[2]=V("Class_Max_Loans");
	v[3]=v[0]-v[1];															//will demand loans for the amount of desired expenses that internal funds can not pay for
	v[4]=max(0,min(v[3],v[2]));												//demand for new loans can not be negative
RESULT(v[4])


EQUATION("Class_Effective_Loans")
/*
Class effective loans is the amount demanded that the financial sector was able to met
*/
	v[0]=V("Class_Demand_Loans");
	v[1]=V("Class_Interest_Rate");
	v[2]=V("annual_frequency");

	cur = ADDOBJ("CLASS_LOANS");
	WRITES(cur, "class_loan_total_amount", v[0]);
	WRITES(cur, "class_loan_interest_rate", v[1]);
	WRITES(cur, "class_loan_fixed_amortization", (v[0]/v[2]));
	WRITES(cur, "class_loan_fixed_object", 0);
RESULT(v[0])


EQUATION("Class_Funds")
/*
Total available funds for class expenses in the current period is the internal funds (already discounted required debt payment) plus effective new loans
*/
	v[0]=V("Class_Internal_Funds");
	v[1]=V("Class_Effective_Loans");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Class_Maximum_Expenses")
/*
Nominal value of possible expenses, restricted to the amount of funds available.
*/
	v[0]=V("Class_Desired_Expenses");
	v[1]=V("Class_Funds");
	v[2]=max(0,min(v[0],v[1]));
RESULT(v[2])


EQUATION("Class_Real_Domestic_Consumption_Demand")
/*
Class effective external domestic consumption, depending on desired level of imports plus the demand not met by the domestic production
*/
	v[0]=V("Class_Maximum_Expenses");
	v[1]=VS(consumption, "Sector_Avg_Price"); 					//consumption goods price
	v[2]=VS(consumption, "Sector_External_Price");				//consumption goods external price
	v[3]=V("Class_Real_Desired_Domestic_Consumption");          //real desired demand for domestic consumption gooods
	v[4]=V("Class_Real_Desired_Imported_Consumption");          //real desired demand for imported consumption gooods
	v[5]=VS(external,"Country_Exchange_Rate");                  //exchange rate
	v[6]=v[3]*v[1];												//nominal desired expenses in domestic consumption
	v[7]=v[4]*v[2]*v[5];										//nominal desired expenses in imported consumption
	v[8]=min(v[0],v[6]);										//effective domestic expenses 
	v[9]=v[8]/v[1];												//real effective domestic consumption
	v[10]=min(v[7],v[0]-v[8]);									//effective imported expenses
	v[11]=v[10]/(v[2]*v[5]);									//real effective imported consumption

	WRITE("Class_Real_Imported_Consumption_Demand", v[11]);
RESULT(v[9])

EQUATION_DUMMY("Class_Real_Imported_Consumption_Demand", "Class_Real_Domestic_Consumption_Demand")
	

EQUATION("Class_Effective_Real_Domestic_Consumption")
/*
Class effective real domestic consumption, depending on how much the domestic consumption goods sector was able to meet demand.
*/
	v[0]=VS(consumption,"Sector_Demand_Met");                    //percentage of the total demand met by the sector
	v[1]=V("Class_Real_Domestic_Consumption_Demand");			 //real demand    
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Class_Effective_Real_Imported_Consumption")
/*
Class effective external consumption, depending on desired level of imports plus the demand not met by the domestic production
*/
	v[0]=VS(consumption,"Sector_Demand_Met");                    //percentage of the total demand met by the sector
	v[1]=VS(consumption,"Sector_Demand_Met_By_Imports");         //identifies if classes were capable of importing the amount not mey by the domestic production
	v[2]=(1-v[0])*v[1];											 //percentage of domestic demand met by extra imports
	v[3]=V("Class_Real_Domestic_Consumption_Demand");			 //desired level of domestic consumption 
	v[4]=V("Class_Real_Imported_Consumption_Demand"); 			 //desired level of external consumption
	v[5]=v[2]*v[3] + v[4];						
RESULT(v[5])


EQUATION("Class_Effective_Expenses")
/*
Class effective expenses is the sum of effective domestic consumption and effective imports, in nominal values.
*/
	v[0]=VS(consumption,"Sector_Avg_Price");                     //domestic price of consumption goods
	v[1]=VS(consumption,"Sector_External_Price");                //external price of consumption goods
	v[2]=VS(external,"Country_Exchange_Rate");																					 
	v[3]=V("Class_Effective_Real_Domestic_Consumption");		 //effective real domestic consumption
	v[4]=V("Class_Effective_Real_Imported_Consumption"); 	     //effective real imports
	v[5]=v[0]*v[3] + v[1]*v[2]*v[4];							 //effective nominal expenses		
RESULT(v[5])


EQUATION("Class_Available_Deposits")
/* 
Class available deposits after expenses and financial obligations
*/
	v[0]=V("Class_Funds");					//class total funds
	v[1]=V("Class_Effective_Expenses");		//class effective expenses, already limited by total funds
	v[2]=max(0, v[0]-v[1]);                                                                                                 				                          
RESULT(v[2])


EQUATION("Class_Deposits_Return")
/* 
Net return on class deposits
*/
	v[0]=V("Class_Available_Deposits");
	v[1]=V("Class_Retained_Deposits");
	v[2]=VS(financial,"Financial_Sector_Interest_Rate_Deposits");  
	v[3]=(v[0]+v[1])*v[2];	
RESULT(v[3])


EQUATION("Class_Nominal_Disposable_Income")
/*
Class net nominal income shall be calculated by summing the ratio of the total surplus to the proportion of the net salary that is allocated to the class plus the payment of government interest on the domestic public debt.

switch_class_tax_structure
0--> No Tax
1--> Only Wages
2--> Only Profits
3--> Wages and Profits, no Interest, no Wealth
4--> Wages, Profits and Interest
5--> Wages, Profits, Interest and Wealth

switch_unemployment_benefits
0--> Distributed by wage share
1--> Distributed to lowest income class only 

*/
	v[0]=VS(country,"Country_Distributed_Profits");                     //total distributed profits
	v[1]=VS(country,"Country_Total_Wages");                             //total wages
	v[2]=V("class_profit_share");                          		      	//profit share of each class
	v[3]=V("class_wage_share");                            			  	//wage share of each class
	v[4]=V("Class_Deposits_Return");                                  	//interest receivment
	v[5]=VS(government,"Government_Effective_Unemployment_Benefits"); 	//unemployment benefits (never taxed)

		v[17]=MAXS(PARENT, "class_wage_share");				 		 	//search the lowest value of nominal income in the last period
		cur=SEARCH_CNDS(PARENT, "class_wage_share", v[17] );  			//search the class with nominal income equal to the lowest value
		v[18]=VS(cur,"class_id");									 	//identify lowest income class
		v[16]=V("class_id");                                          	//current object id
		if(v[16]==v[18])                                              	//if current object is the one with minimum income
			v[6]=v[0]*v[2]+v[1]*v[3]+v[5];     		          	//class' gross total income, including unemployment benefits
		else                                                          	//if it is not
			v[6]=v[0]*v[2]+v[1]*v[3];                            	//class' gross total income excluding unemployment benefits
	
	v[7]=V("switch_class_tax_structure");                 			 	//defines taxation structure
	v[8]=V("class_direct_tax");                            				//class tax rate
	if(v[7]==0)											   				//taxation structure = no tax
		v[9]=0;							   				   				//class total tax
	if(v[7]==1)											   				//taxation structure = only wages
		v[9]=(v[1]*v[3])*v[8];                  		   				//class total tax
	if(v[7]==2)											   				//taxation structure = only profits
		v[9]=(v[0]*v[2])*v[8];              			   				//class total tax
	if(v[7]==3)											   				//taxation structure = profits and wages 
		v[9]=(v[0]*v[2]+v[1]*v[3])*v[8];              	   				//class total tax
	if(v[7]==4)											   				//taxation structure = profits, wages and interest
		v[9]=(v[0]*v[2]+v[1]*v[3]+v[4])*v[8];              				//class total tax
	
	v[19]=VS(country,"Country_Consumer_Price_Index");
	
	WRITE("Class_Taxation",v[9]);                          				//write class taxation equation_dummy
	WRITE("Class_Nominal_Gross_Income",v[6]);              				//write class gross income equation_dummy
	WRITE("Class_Real_Disposable_Income",(v[6]-v[9])/v[19]); 		   	//write class real income equation_dummy
RESULT(v[6]-v[9])

EQUATION_DUMMY("Class_Taxation","Class_Nominal_Disposable_Income")
EQUATION_DUMMY("Class_Nominal_Gross_Income","Class_Nominal_Disposable_Income")
EQUATION_DUMMY("Class_Real_Disposable_Income","Class_Nominal_Disposable_Income")

EQUATION("Class_Stock_Loans")
RESULT(SUM("class_loan_total_amount"))

EQUATION("Class_Stock_Deposits")
/*
Class stock of deposits
*/
	v[0]=V("Class_Available_Deposits");
	v[1]=V("Class_Retained_Deposits");
	v[2]=V("class_profit_share");
	v[3]=V("Exit_Deposits_Distributed");
	v[4]=SUMS(country, "Sector_Entry_Deposits_Needed");
	v[6]=V("Class_Deposits_Return");
	v[5]=v[0]+v[1]+v[2]*(v[3]-v[4])+v[6];
RESULT(v[5])


EQUATION("Class_Debt_Rate")
/*
Degree of indebtedness, calculated by the ratio of the debt to deposits.
*/
	v[0]=V("Class_Stock_Loans");                      //class stock of debt 
	v[1]=V("Class_Stock_Deposits");
	v[2]=V("Class_Avg_Nominal_Income");																									 //class average real income in the last periods
	if((v[2]+v[1])>0)                                 //if the stock of deposits is positive
		v[4]=v[0]/(v[2]+v[1]);                        //debt rate is the stock of debt over stock of deposits and average income
	else                                              //if the sum of the fisical capital plus the financial assets is not positive
		v[4]=1.1;                                     //debt rate is 1.1 
RESULT(v[4])


EQUATION("Class_Income_Share")
/*
Class share of nominal income
*/
	v[0]=V("Class_Nominal_Disposable_Income");
	v[1]=SUMS(PARENT,"Class_Nominal_Disposable_Income");
	v[2]= v[1]!=0? v[0]/v[1]: 0;
RESULT(v[2])


EQUATION("Class_Wealth_Share")
/*
Class share of nominal income
*/
	v[0]=V("Class_Stock_Deposits");
	v[1]=SUMS(PARENT,"Class_Stock_Deposits");
	v[2]= v[1]!=0? v[0]/v[1]: 0;
RESULT(v[2])


