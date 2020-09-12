/*****INCOME CLASSES*****/ 


EQUATION("Class_Avg_Real_Income")
/*
Class average real income based on the past "class period" periods, considering financial obligations.
Will affect induced consumption, imports decisions and debt assessment
*/
	v[0]=V("class_period");										//define the class adjustment period
	v[1]=0;														//initializes the sum
		for (i=1; i<=v[0]; i++)									//for the number os lags equal the adjustment parameter
		{
		v[6]=VL("Class_Real_Income", i);
		v[1]=v[1]+v[6];
		}
	v[7]=v[1]/v[0];                  							//class average income in the last v[0] periods
RESULT(v[7])


EQUATION("Class_Avg_Nominal_Income")
/*
Class average nominal income based on thepast "class period" periods.
Will be the base for debt rate calculus
*/
	v[0]=V("class_period");										//define the class adjustment period
	v[1]=0;														//initializes the sum
		for (i=1; i<=v[0]; i++)									//for the number os lags equal the adjustment parameter
		{
		v[3]=VL("Class_Nominal_Income", i);           			//lagged class' income
		v[1]=v[1]+v[3];											//sum lagged income until the adjustment period
		}
	v[6]=v[1]/v[0];                  							//class average income in the last v[0] periods
RESULT(v[6])


EQUATION("Class_Real_Autonomous_Consumption")
/*
Class autonomous consumption depends on the average quality growth of the consumption goods sector, real
*/
v[0]=VL("Class_Real_Autonomous_Consumption",1);                 //class autonomous consumption in the last period
v[1]=V("class_period");										    //defines the class adjustment period 
v[2]= fmod((double) t,v[1]);									//divides time period by the class period and takes the rest
if (v[2]==0)													//if it is class adjustment period 	
	{
	cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);	//search consumption good sector
	v[3]=VLS(cur, "Sector_Avg_Quality",1);     					//sector average quality in the last period                        
	v[4]=VLS(cur, "Sector_Avg_Quality",(v[1]+1)); 				//sector average quality in the last adjustment period                          
  	if(v[4]!=0)													//if initial average quality is not zero                                                                              
		v[5]=(v[3]-v[4])/v[4];          						//computate quality growth                                                  			
  	else     													//if average quality is zero                                                                               
		v[5]=0;													//quality growth is zero
    
   v[6]=V("class_autonomous_consumption_adjustment");				//autonomous consumption adjustment parameter
		if(v[5]>0) 													//if quality growth was positive
			v[7]=v[0]*(1+v[6]*v[5]); 								//increase autonomous consumption by the adjustment parameter
		if(v[5]==0)													//if quality grwoth was zero
			v[7]=v[0];												//use last period autonomous consumption
		if(v[5]<0)													//if quality growth was negative
			v[7]=v[0]*(1+v[6]*v[5]);							 	//decrease autonomous consumption by the adjustment parameter
		
	v[8]=VL("Class_Stock_Deposits",1);     						//class stock of deposits in the last period                
	v[9]=VL("Class_Stock_Deposits",(v[1]+1)); 					//class stock of deposits in the las class period                          
  	if(v[9]!=0)													//if initial stock of deposits is not zero                                                                              
		v[10]=(v[8]-v[9])/v[9];          						//computate deposits growth growth                                                  			
  	else     													//if stock of deposits is zero                                                                               
		v[10]=0;		
   v[11]=V("class_autonomous_consumption_adjustment_deposits");
   
   	if(v[10]>0)
   		v[12]=v[7]*(1+v[11]*v[10]);
   	if(v[10]==0)
   		v[12]=v[7];
   	if(v[10]<0)
   		v[12]=v[7]; 	
  }
else																//if it is not class adjustment period
	v[12]=v[0]; 													//use lase period autonomous consumption                                                                       		
RESULT(max(0,v[12]))


EQUATION("Class_Real_Desired_Consumption")
/*
Class real domestic conumption is based on average past real income and on the class' propensity to consume, plus autonomous consumption
*/
	v[0]=V("Class_Avg_Real_Income");				//class average income in the last periods
  	v[1]=V("class_propensity_to_consume");          //class propensity to consume on income
  	v[2]=V("Class_Real_Autonomous_Consumption");    //class autonomous consumption
  	v[3]=v[0]*v[1]+v[2];                            //class real desired consumption
RESULT(v[3])


EQUATION("Class_Real_Desired_Imports")
/*
Class real imports are based on average past real income and on the class' propensity to import and restricted to the amount of existing international resevres
*/
	v[0]=V("Class_Avg_Real_Income");				//class average income in the last periods
  	v[1]=V("class_propensity_to_import");           //class propensity to import
  	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Class_Desired_Expenses")
/*
Class' nominal desired expenses depends on effective domestic consumption times avg price plus effective external consumption times foreign price
*/
	v[0]=V("Class_Real_Desired_Consumption");                    // class desired domestic consumption
	v[1]=V("Class_Real_Desired_Imports");                        // class desired external consumption
	v[2]=V("Exchange_Rate");                                     //exchange rate
	
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);     //identifies the consumption goods sector
		v[3]=VS(cur, "Sector_Avg_Price");                        //sector average price
		v[4]=VS(cur, "Sector_External_Price");                   //sector external price
		v[5]=v[0]*v[3] + v[1]*v[2]*v[4];     					 //total nominal expenses                  	
RESULT(v[5])


EQUATION("Class_Avg_Debt_Rate")
/*
Class avg debt rate of the last class period (equal to annual period)
*/
	v[0]=V("class_period");
	v[3]=0;														//initializes the sum
	for (i=1; i<=v[0]; i++)										//from 0 to investment period-1 lags
		{
		v[2]=VL("Class_Debt_Rate", i);							//computates class debt rate of the current lag
		v[3]=v[3]+v[2];											//sum up class lagged debt rate
		}
	v[4]=v[3]/v[0];												//average class debt rate of the last class period
RESULT(v[4])


EQUATION("Class_Interest_Rate")
/*
Interest rate paid by the class depends on a specific spread over basic interest rate, based on the average debt rate of the class.
*/
	v[0]=V("risk_premium_adjustment");
	v[1]=V("Class_Avg_Debt_Rate");
	v[2]=1+v[1]*v[0];
	v[3]=V("Interest_Rate_Loans_Short_Term");
	v[4]=v[2]*v[3];
RESULT(v[4])


EQUATION("Class_Interest_Payment")
/* 
Sum up total interest payment on all class' loans. Interest rates are fixed for each loan. 
*/
	v[0]=0;												//initializes the CYCLE
	CYCLE(cur, "CLASS_LOANS")							//CYCLE trough all class' loans
	{
		v[1]=VS(cur, "class_loan_total_amount");		//debt current amount 
		v[2]=VS(cur, "class_loan_interest_rate");		//debt interest rate
		v[3]=v[1]*v[2];									//current debt interest payment
		v[0]=v[0]+v[3];									//sum up interest payment of all loans
	}
	v[4]=VL("Class_Stock_Deposits", 1);     			//class current stock of deposits 
	v[5]=VL("Class_Nominal_Income",1);
	v[6]=v[4]+v[5];
	v[7]=min(v[0],v[6]);           						//total interest payment is limited to the amount of liquid resources                                              				               
RESULT(max(0,v[0]))										//can not be negative


EQUATION("Class_Debt_Payment")
/* 
Sum up total debt payment on all class' loans. Amortizations are fixed for each loan. 
This variable also adjusts the total amount of each loan and delete loan objects if all debt is paid.
*/
	v[0]=SUM("class_loan_fixed_amortization");		 	//sum up all amortizations for current period
	v[12]=VL("Class_Nominal_Income",1);
	v[1]=VL("Class_Stock_Deposits", 1);     			//class current stock of deposits  
	v[2]=V("Class_Interest_Payment");					//class current interest payment (priority)
	v[3]=v[1]+v[12]-v[2];										//class available resources for debt payment
	
	//if (v[3]>=v[0])
	if(v[0]>=0)	
	{
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
		v[10]=v[0];
	}
	else														//if class has not enough liquid resources to pay
	{	
		SORT ("CLASS_LOANS", "class_loan_interest_rate", "DOWN");//sort loans from higher interest rate to lower
		v[4]=v[3];												//initializes the cycle for total amount of resources available
		CYCLE_SAFE(cur, "CLASS_LOANS")							//CYCLE trough all class' loans
		{
		v[5]=VS(cur, "class_loan_total_amount");				//debt current amount 
		v[6]=VS(cur, "class_loan_fixed_amortization");			//debt fixed amortization
		if(v[4]>=v[6])											//if available resources is higher than amortization
			v[7]=v[6];											//effective amortization
		else 													//if available resources are not enough
			v[7]=v[4];											//effective amortization
		
		v[8]=v[5]-v[7];											//new total amount
		v[9]=VS(cur, "class_loan_fixed_object");				//identifies if it is fixed object, necessary for model structure
		if(v[9]!=1)												//if it is not the fixed object
			{	
			if (v[8]>0)											//if there is still amount to be amortized
				WRITES(cur, "class_loan_total_amount", v[8]);	//write the new amount
			else												//if all amount was already amortized
				DELETE(cur);									//delete current loan
			}
		
		v[4]=v[4]-v[7];											//updates the available resources
		}
	v[10]=v[3];													//total debt payment
	}	  
RESULT(max(0,v[10]))		


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
	v[0]=V("class_period");
	v[1]=fmod((double)t,v[0]);
	v[2]=VL("Class_Nominal_Income",1);
	v[3]=VL("Class_Nominal_Income",v[0]);
	if(v[3]!=0)
		v[4]=(v[2]-v[3])/v[3];
	else
		v[4]=0;
	v[5]=V("Class_Avg_Debt_Rate");
	v[6]=V("class_desired_debt_rate");
	v[7]=VL("Class_Liquidity_Preference",1);
	v[8]=V("class_liquidity_preference_adjustment");
	
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


EQUATION("Class_Retained_Deposits")
/*
Class retained deposits at current period. To be summed in the stock of deposits at the end of period
Based on the liquidity preference. If liquidity preference is zero, retained deposits is zero.
Can not be negative, meaning that class will not incur in new debt to finance retained liquidity.
*/
	v[1]=V("Class_Financial_Obligations");	//current financial obligations
	v[2]=VL("Class_Stock_Deposits", 1);		//stock of deposits
	v[8]=VL("Class_Nominal_Income",1);
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
	v[4]=VL("Class_Nominal_Income",1);
	v[2]=V("Class_Retained_Deposits");
	v[3]=v[4]+v[1]-v[0]-v[2];							//available deposits to use as internal funds
RESULT(max(0,v[3]))	


EQUATION("Class_Desired_Debt_Rate")
/*
Class desired debt rate. 
Formulation proposed by Moreira (2010) 
Evolves based on nominal income growth.
*/
	v[0]=V("class_period");
	v[1]=fmod((double)t,v[0]);
	v[2]=VL("Class_Nominal_Income",1);
	v[3]=VL("Class_Nominal_Income",v[0]);
	if(v[3]!=0)
		v[4]=(v[2]-v[3])/v[3];
	else
		v[4]=0;
		
	v[10]=VL("Class_Desired_Debt_Rate",1);
	v[11]=V("class_debt_rate_adjustment");
		
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


EQUATION("Class_Max_Loans")
/*
Class available debt depends on the difference between desired stock of debt and current stock of debt. 
If current stock of debt is greater than desired, the class must repay some debt reducing the amount of external funds for investment. 
If the currest amount is smaller than desired, that difference is available to the class as external finance, but that does not mean that the class will increase effective debt by this amount.
*/
	v[0]=V("Class_Desired_Debt_Rate");
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
	if(v[2]>0)															//if there is available debt 
		{
		v[3]=v[0]-v[1];													//will demand loans for the amount of desired expenses that internal funds can not pay for
		v[4]=min(v[3],v[2]);											//demand will be the minimum between amount needed and amount available
		}
	else																//if there is no available debt
		v[4]=0;															//no demand for debt
	v[5]=max(0,v[4]);													//demand for new loans can not be negative
RESULT(v[5])


EQUATION("Class_Effective_Loans")
/*
Class effective loans is the amount demanded that the financial sector was able to met
*/
	v[0]=V("Class_Demand_Loans");
	v[3]=V("Class_Interest_Rate");
	v[4]=V("class_period");

	cur = ADDOBJ("CLASS_LOANS");
	WRITES(cur, "class_loan_total_amount", v[0]);
	WRITES(cur, "class_loan_interest_rate", v[3]);
	WRITES(cur, "class_loan_fixed_amortization", (v[0]/v[4]));
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
	if(v[1]<=0)													//if no available funds
		v[2]=0;													//class effective expenses are zero
	else														//if there are funds available
		v[2]=min(v[0],v[1]);
RESULT(v[2])


EQUATION("Class_Real_Consumption_Demand")
/*
Class effective domestic consumption goods demand. There is a priority between domestic and imported, in which the first is preferible. The effective real demand will be the minimum between the desired and the possible amount.
*/
	v[0]=V("Class_Maximum_Expenses");
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);     //identifies the consumption goods sector
	v[1]=VS(cur, "Sector_Avg_Price"); 							 //consumption goods price
	v[2]=v[0]/v[1];												 //real effective consumption demand possible																																		 
	v[3]=V("Class_Real_Desired_Consumption");                    //real desired consumption demand desired
	v[4]=min(v[2],v[3]);
RESULT(v[4])


EQUATION("Class_Real_Imports_Demand")
/*
Class effective external domestic consumption, depending on desired level of imports plus the demand not met by the domestic production
*/
	v[0]=V("Class_Maximum_Expenses");
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);    //identifies the consumption goods sector
	v[1]=VS(cur, "Sector_Avg_Price"); 							//consumption goods price
	v[2]=VS(cur, "Sector_External_Price");						//consumption goods external price
	v[3]=V("Class_Real_Consumption_Demand");                    //real effetive demand for domestic consumption gooods
	v[4]=v[3]*v[1];												//nominal effective expenses with domestic caital goods
	v[5]=max(0, (v[0]-v[4]));									//effective amount that can be spended with external consumption goods
	v[6]=V("Exchange_Rate");
	v[7]=v[5]/(v[2]*v[6]);										//effective real demand for imported consumption goods
	v[8]=V("Class_Real_Desired_Imports");
	v[9]=min(v[7],v[8]);
RESULT(v[9])


EQUATION("Class_Effective_Consumption")
/*
Class effective real domestic consumption, depending on how much the domestic consumption goods sector was able to meet demand.
*/
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);     //identifies the consumption goods sector
	v[0]=VS(cur,"Sector_Demand_Met");                     		 //percentage of the total demand met by the sector
	v[1]=V("Class_Real_Consumption_Demand");			         //real demand    
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Class_Effective_Imports")
/*
Class effective external consumption, depending on desired level of imports plus the demand not met by the domestic production
*/
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);     //identifies the consumption goods sector
	v[0]=VS(cur,"Sector_Demand_Met");                     		 //percentage of the total demand met by the sector
	v[1]=VS(cur,"Sector_Demand_Met_By_Imports");                 //identifies if classes were capable of importing the amount not mey by the domestic production
	v[2]=(1-v[0])*v[1];											 //percentage of domestic demand met by extra imports
	v[3]=V("Class_Real_Consumption_Demand");					 //desired level of domestic consumption 
	v[4]=V("Class_Real_Imports_Demand"); 						 //desired level of external consumption
	v[5]=v[2]*v[3] + v[4];						
RESULT(v[5])


EQUATION("Class_Effective_Expenses")
/*
Class effective expenses is the sum of effective domestic consumption and effective imports, in nominal values.
*/
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);     //identifies the consumption goods sector
	v[0]=VS(cur,"Sector_Avg_Price");                     	     //domestic price of consumption goods
	v[1]=VS(cur,"Sector_External_Price");                 		 //external price of consumption goods
	v[2]=V("Exchange_Rate");																					 
	v[3]=V("Class_Effective_Consumption");						 //effective real domestic consumption
	v[4]=V("Class_Effective_Imports"); 						     //effective real imports
	v[5]=v[0]*v[3] + v[1]*v[2]*v[4];							 //effective nominal expenses		
RESULT(v[5])


EQUATION("Class_Available_Deposits")
/* 
Class available deposits after expenses and financial obligations
*/
	v[0]=V("Class_Funds");					//class total funds
	v[1]=V("Class_Effective_Expenses");		//class effective expenses, already limited by total funds
	v[2]=v[0]-v[1];							//if effective expenses were lower than total funds, there are available deposits
	v[3]=max(0, v[2]);                                                                                                 				                          
RESULT(v[3])


EQUATION("Class_Deposits_Return")
/* 
Net return on class deposits
*/
	v[0]=V("Class_Available_Deposits");
	v[3]=V("Class_Retained_Deposits");
	v[1]=V("Interest_Rate_Deposits");  
	v[2]=(v[0]+v[3])*v[1];	
RESULT(v[2])


EQUATION("Class_Nominal_Income")
/*
Class net nominal income shall be calculated by summing the ratio of the total surplus to the proportion of the net salary that is allocated to the class plus the payment of government interest on the domestic public debt.

taxation_structure
0--> Only Wages
1--> Wages and Profits
2--> Wages, Profits and Interest
3--> Wages, Profits, Interest and Wealth

switch_unemployment_benefits
0--> Distributed by wage share
1--> Distributed to lowest income class only 

*/
	v[0]=V("Total_Distributed_Profits");                   			  //total distributed profits
	v[1]=V("Total_Wages");                                			  //total wages
	v[2]=V("class_profit_share");                          		      //profit share of each class
	v[3]=V("class_wage_share");                            			  //wage share of each class
	v[4]=V("Class_Deposits_Return");                                  //interest receivment
	v[5]=V("Government_Effective_Unemployment_Benefits");             //unemployment benefits (never taxed)
	v[13]=V("switch_unemployment_benefits"); 
	if(v[13]==0)                                                      //if unemployment benefits are distributed by wage share
		v[6]=v[0]*v[2]+v[1]*v[3]+v[4]+v[5]*v[3];     		          //class' gross total income
	if(v[13]==1)                                                      //if unemployment benefits are distributed only for the lowest income class
	{
		v[16]=V("id_class");                                          //current object id
		if(v[16]==3)                                              //if current object is the one with minimum income
			v[6]=v[0]*v[2]+v[1]*v[3]+v[4]+v[5];     		          //class' gross total income, including unemployment benefits
		else                                                          //if it is not
			v[6]=v[0]*v[2]+v[1]*v[3]+v[4];                            //class' gross total income excluding unemployment benefits
	} 
	v[7]=V("taxation_structure");                          //defines taxation structure
	v[8]=V("class_direct_tax");                            //class tax rate
	if(v[7]==0)
		v[9]=v[1]*v[3]*v[8];							   //class total tax
	if(v[7]==1)
		v[9]=(v[0]*v[2]+v[1]*v[3])*v[8];                   //class total tax
	if(v[7]==2)
		v[9]=(v[0]*v[2]+v[1]*v[3]+v[4])*v[8];              //class total tax
	if(v[7]==3)
	{
		v[10]=VL("Class_Stock_Deposits",1);                //class stock of deposits in the last period
		v[11]=V("class_wealth_tax");                       //tax rate on stock of wealth
		v[12]=v[10]*v[11];                                 //amount of tax on wealth
		v[9]=(v[0]*v[2]+v[1]*v[3]+v[4])*v[8]+v[12];        //class total tax
	}
	WRITE("Class_Taxation",v[9]);                          //write class taxation equation_dummy
	WRITE("Class_Gross_Nominal_Income",v[6]);              //write class gross income equation_dummy
RESULT(v[6]-v[9])


EQUATION_DUMMY("Class_Taxation","Class_Nominal_Income")
EQUATION_DUMMY("Class_Gross_Nominal_Income","Class_Nominal_Income")



EQUATION("Class_Real_Income")
/*
Class real income is the nominal income divided by the class price index
*/
	v[0]=V("Class_Nominal_Income");
	v[1]=V("Consumer_Price_Index");
	v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("Class_Stock_Loans")
/*
Class Stock of Debt
*/
	v[0]=SUM("class_loan_total_amount");
RESULT(v[0])


EQUATION("Class_Stock_Deposits")
/*
Class stock of deposits
*/
	v[0]=V("Class_Available_Deposits");
	v[1]=V("Class_Retained_Deposits");
	v[2]=V("class_profit_share");
	v[3]=V("Exit_Deposits_Distributed");
	v[5]=V("Class_Deposits_Return");
	v[4]=v[0]+v[1]+v[2]*v[3];
RESULT(v[4])


EQUATION("Class_Debt_Rate")
/*
Degree of indebtedness, calculated by the ratio of the debt to deposits.
*/
	v[0]=V("Class_Stock_Loans");                      //class stock of debt 
	v[1]=V("Class_Stock_Deposits");
	v[2]=V("Class_Avg_Nominal_Income");				  //class average real income in the last periods
	if((v[2]+v[1])>0)                                 //if the stock of deposits is positive
		v[4]=v[0]/(v[2]+v[1]);                        //debt rate is the stock of debt over stock of deposits and average income
	else                                              //if the sum of the fisical capital plus the financial assets is not positive
		v[4]=1.1;                                     //debt rate is 1.1 
RESULT(v[4])


EQUATION("Class_Income_Share")
/*
Class share of nominal income
*/
	v[0]=V("Class_Nominal_Income");
	v[1]=SUMS(PARENT,"Class_Nominal_Income");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])


EQUATION("Class_Wealth_Share")
/*
Class share of nominal income
*/
	v[0]=V("Class_Stock_Deposits");
	v[1]=SUMS(PARENT,"Class_Stock_Deposits");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])

