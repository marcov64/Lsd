/*****ENTRY AND EXIT VARIABLES*****/
//all these variable are sector variables


EQUATION("Exit")
/*
Macro Variable
This variable counts productive capacity that exited the sector in each time step. Firm objects are deleted inside that variable due to two possibilities: small market share or high debt rate. In the second case, if entry conitions are met, the firm can be bought by a new one.
*/

v[0]=V("survival_period");
v[17]=v[18]=v[19]=v[21]=v[23]=0;
CYCLE(cur1, "SECTORS")
{
	i = COUNTS(cur1, "FIRMS" );									// count the existing number of firms 
	v[11]=v[12]=v[14]=v[20]=v[24]=0;							// initialize the cycles
	CYCLE_SAFES( cur1, cur, "FIRMS" )							// use a robust cycle to delete objects
	{
     v[1]=VS(cur, "Firm_Market_Share");							//firm's curent market share
     v[2]=VS(cur, "Firm_Productive_Capacity");					//firm's current productive capacity																				
     v[3]=VS(cur, "firm_date_birth");							//firm's date birth
	 v[4]=VS(cur, "Firm_Debt_Rate");						//firm's avg debt rate
     v[5]=VLS(cur, "Firm_Avg_Debt_Rate", 1);					//firm's avg debt rate in the last period
     v[6]=VS(cur, "Firm_Stock_Loans");
     v[7]=VS(cur, "Firm_Stock_Deposits");
     v[8]=VS(cur, "Firm_Capital");
     v[9]=VS(cur, "firm_bank");
	 cur2=SEARCH_CNDS(root, "bank_id", v[9]);
	 
     if ( v[1] <= 0.001 && i > 1 && t>(v[0]+v[3]))				//if firm's market share is near zero
     {
      //plog("\nFirm Deleted - Small Market Share");			//write on log window
      	if (v[7]>=v[6])											//firm pays current debt with current deposits 
      		{
      		v[11]=v[11]+(v[7]-v[6]);							//deposits distributed to income classes (no need to substract from bank's stock)
      		v[12]=v[12]+v[2];									//available productive capacity
      		v[14]=v[14];										//defaulted loans
      		v[20]=v[20];										//bankruptcy events
      		}
      	else													//if firm can not pay current debt with current deposits
      		{
       		v[11]=v[11];										//deposits distributed to income classes (no need to substract from bank's stock)
      		v[12]=v[12]+v[2];									//available productive capacity
      		v[14]=v[14]+(v[6]-v[7]);							//defaulted loans
      		v[20]=v[20];										//bankruptcy events
			v[15]=VS(cur2, "bank_defaulted_loans_temporary");
			WRITES(cur2, "bank_defaulted_loans_temporary", (v[15]+v[6]-v[7]));
      		}
      DELETE(cur); 												//delete current firm
      v[24]=v[24]+1;											//count number of exited firms
	  --i;														//subtract 1 from the number of firms counter
     }
     else														//if firm's current market share is not near zero																																						//if firm's avg debt rate is not higher than 1 (do not delete current firm						
     {
		if(v[4]>1 &&i>1 && t>(v[0]+v[3]))
		{
		//plog("\nFirm Deleted - High Debt");					//write on log window
      	if (v[7]>=v[6])											//firm pays current debt with current deposits 
      		{
      		v[11]=v[11]+(v[7]-v[6]);							//deposits distributed to income classes (no need to substract from bank's stock)
      		v[12]=v[12]+v[2];									//available productive capacity
      		v[14]=v[14];										//defaulted loans
      		v[20]=v[20]+1;										//bankruptcy events
      		}
      	else													//if firm can not pay current debt with current deposits
      		{
       		v[11]=v[11];										//deposits distributed to income classes (no need to substract from bank's stock)
      		v[12]=v[12]+v[2];									//available productive capacity
      		v[14]=v[14]+(v[6]-v[7]);							//defaulted loans
      		v[20]=v[20]+1;										//bankruptcy events
			v[15]=VS(cur2, "bank_defaulted_loans_temporary");
			WRITES(cur2, "bank_defaulted_loans_temporary", (v[15]+v[6]-v[7]));
      		}
		DELETE(cur); 											//delete current firm
		v[24]=v[24]+1;											//count number of exited firms
		--i;					
		}
		else
		{
        v[11]=v[11];											//deposits distributed to income classes (no need to substract from bank's stock)
      	v[12]=v[12];											//available productive capacity
      	v[14]=v[14];					  						//defaulted loans
      	v[20]=v[20];											//bankruptcy events
      	v[24]=v[24];
      	}
	 }
	WRITES(cur1, "Sector_Productive_Capacity_Exit", v[12]);
	}
v[17]=v[17]+v[11];
v[18]=v[18]+v[12];
v[19]=v[19]+v[14];
v[21]=v[21]+v[20];
v[23]=v[23]+v[24];
}
WRITE("Exit_Deposits_Distributed", v[17]);
WRITE("Exit_Productive_Capacity", v[18]);
WRITE("Exit_Defaulted_Loans", v[19]);
WRITE("Exit_Bankruptcy_Events", v[21]);																														
RESULT(v[23])

EQUATION_DUMMY("Exit_Deposits_Distributed", "Exit")
EQUATION_DUMMY("Exit_Productive_Capacity", "Exit")
EQUATION_DUMMY("Exit_Defaulted_Loans", "Exit")
EQUATION_DUMMY("Exit_Bankruptcy_Events", "Exit")
EQUATION_DUMMY("Sector_Productive_Capacity_Exit", "Exit")


EQUATION("Exit_Bankruptcy_Share")
	v[0]=V("Exit_Bankruptcy_Events");
	v[1]=SUML("Sector_Number_Firms",1);
	v[2]=v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Sector_Entry_Condition")
/*
Can only be 0 or 1, if all enter conditions are met.
*/
	v[0]=V("switch_entry");
	v[1]=V("sector_investment_frequency");
	v[2]=VL("Sector_Effective_Orders",1);
	v[3]=VL("Sector_Effective_Orders",2);
	v[4]=VL("Sector_Effective_Orders",3);
	v[5]=VL("Sector_Effective_Orders",v[1]);
	v[6]=v[2]-v[5];
	v[7]=(v[2]-v[3])/v[3];
	v[8]=(v[3]-v[4])/v[4];
	v[10]=V("Sector_Profit_Rate");
	v[11]=VS(financial, "Financial_Sector_Interest_Rate_Deposits");
  
    if(v[0]==1 & v[6]>0 && v[7]>0 && v[8]>0 &&v[10]>v[11])
		v[9]=1;
    else
		v[9]=0;
RESULT(v[9])

/*
EQUATION("Entry")
v[26]=MAXL("Class_Stock_Deposits", 1);							//check current stock of deposits of all classes
cur=SEARCH_CNDL("Class_Stock_Deposits", v[26], 1);
v[24]=VLS(cur, "Class_Stock_Deposits", 1);							//check current stock of deposits of all classes
v[25]=V("Exit_Deposits_Distributed");
v[28]=v[24]+v[25];
SORT("SECTORS", "Sector_Profit_Rate", "DOWN");
CYCLE(cur6, "SECTORS")
{
   v[0]=VS(cur6,"Sector_Entry_Condition");									//value of entry conditions										//sum of firm's market share 
      if(v[0]==1)														//if entry conditions are met and there are market space 
      {
	  v[1]=AVES(cur6, "Firm_Market_Share");
	  v[2]=1;
	  v[3]=0;														//initializes the cycle
     	CYCLES(cur6, cur, "FIRMS")											//CYCLE trough all firms to firm the average firm 
     	{
     	v[4]=VS(cur, "Firm_Market_Share");							//current firm's market share
        v[5]=abs(v[4]-v[1]);										//absolute difference between current firm's market share and sector's avg
        v[2]=min(v[5],v[2]);										//min between 1 and the current difference
            if(v[5]==v[2])											//if the current difference is the lowest one
               v[3]=v[4];											//use current firm's market share
            else													//if current difference is not the lowest one
               v[3]=v[3];											//use the current value, until find the lowest difference
     	}
		cur=SEARCH_CNDS(cur6, "Firm_Market_Share", v[3]);
		v[6]=VS(cur, "Firm_Market_Share");							//market share of the firm to copy
		v[7]=VS(cur, "Firm_Productive_Capacity");					//productive capacity of the firm to copy
		v[8]=VS(cur, "Firm_Capital");								//total entry cost
		
		v[9]=VLS(cur6, "Sector_Productive_Capacity_Available",1);			//productive capacity available in the last period
		v[10]=VS(cur6, "Sector_Productive_Capacity_Exit");					//new productive capacity available in this period
		v[11]=v[10]+v[9];											//total productive capacity available in the sector
		
		v[12]=V("entry_debt_share");								//parameter that defines the share of debt
		v[13]=V("Country_Capital_Goods_Price");								//current price of capital goods
		
		if(v[7]<=v[11])        										//new productive capacity will bre free, from available capacity
			{
			v[14]=v[7];
			v[17]=0;
			v[18]=0;
			}
		else														//more capacity is needed
			{
			v[14]=v[11];											//initial capacity will be all that is available
			v[15]=v[7]-v[11];										//still needed capacity
			if((1-v[12])*v[15]*v[13]<=v[28])						//if the cost of still needed capacity is lower that available deposits
				v[17]=(1-v[12])*v[15]*v[13];						//reduce the cost of still needed capacity from the stock of deposits
			else													//if deposits is not enough
				v[17]=v[28];										//reduce what is available
			v[18]=v[15]*v[13]-v[17];								//value still to be financed
			}
			
		cur1=RNDDRAWS(root, "BANKS", "Bank_Market_Share");		//choose a random bank
		v[19]=VS(cur1, "bank_id");								//identify the chosen bank
		v[20]=VS(cur1, "Bank_Interest_Rate_Long_Term");
		
		v[21]=VS(cur6, "sector_capital_output_ratio");
		v[22]=V("sector_capital_duration");
		v[23]=VS(cur6, "Sector_Avg_Productivity");
		
		cur2=ADDOBJ_EXS(cur6,"FIRMS",cur);							//create new firm
		WRITES(cur2, "firm_date_birth", t);
		WRITES(cur2, "firm_id",t);	
		WRITES(cur2, "firm_bank", v[19]);
		WRITELS(cur2, "Firm_Stock_Deposits",0,t);
		WRITELS(cur2, "Firm_Stock_Loans",v[18],t);
		WRITELS(cur2, "Firm_Demand_Capital_Goods_Expansion", v[15]*v[21], t);
		
		USE_ZERO_INSTANCE
		CYCLE_SAFES(cur2, cur4, "CAPITALS")
			DELETE(cur4);
			
		for(i=1; i<=v[14]*v[21]; i++)
		{
		cur3=ADDOBJS(cur2, "CAPITALS");
		WRITES(cur3, "capital_good_productive_capacity",(1/v[21]));
		WRITES(cur3, "capital_good_date_birth",t);					//write the new date of birth
		WRITES(cur3, "capital_good_depreciation_period",(t+v[22])); //write the new date of birth
		WRITES(cur3, "capital_good_to_replace",0);					//write current capital goods as not to replace
		WRITES(cur3, "capital_good_to_depreciate",0);
		WRITES(cur3, "capital_good_productivity_initial",v[23]);	//write current capital initial productivity
		WRITELS(cur3, "Capital_Good_Acumulated_Production",0,t);	//write current capital acumulated production as zero
		}
	
		CYCLE_SAFES(cur2, cur4, "FIRM_LOANS")
		{
			if (VS(cur4, "firm_loan_fixed_object")!=1)
			DELETE(cur4);
		}
		
		cur5 = ADDOBJS(cur2, "FIRM_LOANS");
		WRITES(cur5, "firm_loan_total_amount", v[18]);
		WRITES(cur5, "firm_loan_fixed_object", 0);
		WRITES(cur5, "firm_loan_fixed_amortization", v[18]/v[22]);
		WRITES(cur5, "id_firm_loan_long_term", 1);
		WRITES(cur5, "firm_loan_interest_rate", v[20]);
		
		v[28]=v[28]-v[17];   
	  }
       else
       {
		 v[14]=0;
		 v[28]=v[28];
	   }
	   WRITES(cur6, "Sector_Productive_Capacity_Entry", v[14]);
}
WRITE("Entry_Deposits_Remaining", v[28]);
RESULT(0)
EQUATION_DUMMY("Entry_Deposits_Remaining", "Entry")
*/

EQUATION("Sector_Productive_Capacity_Entry")
/*
Sector Variable
In this variable a new firm enters if there is market space available and the entry condiion is met
*/

   v[1]=V("Sector_Entry_Condition");									//value of entry conditions
   v[6]=SUM("Firm_Market_Share");										//sum of firm's market share 
      if(v[1]>0&&v[6]<1)												//if entry conditions are met and there are market space 
      {
	  v[20]=V("Sector_Effective_Orders");								//sector effective orders
      v[0]=V("sector_investment_frequency");								//sector investment period
      v[22]=V("sector_desired_degree_capacity_utilization");			//sector degree of capacity utilization
      v[23]=V("sector_desired_inventories_proportion");					//sector inventories proportion
	  v[25]=V("Country_Capital_Goods_Price");									//price of capital goods
      v[34]=VL("Sector_Productive_Capacity_Available",1);				//productive capacity available in the last period
	  v[44]=V("Sector_Productive_Capacity_Exit");						//productive capacity exited in the current period
      v[36]=V("Sector_Avg_Price");										//sector avg price
      v[35]=V("sector_capital_output_ratio");									//sector capital output ratio
      v[38]=V("sector_capital_duration");									//sector depreciation period
      v[39]=V("Sector_Avg_Productivity");   							//sector avg productivity	
      v[42]=uniform_int(1, v[0]);										//randon integer number between 1 and investment period
      v[5]=COUNT("FIRMS");												//current number of firms
     	v[8]=v[6]/v[5];													//sector simple avg market share (deveria pondear?)
      
     	v[10]=1;														//initializes the cycle
     	v[12]=0;														//initializes the cycle
     	CYCLE(cur4, "FIRMS")											//CYCLE trough all firms to firm the average firm 
     	{
     	v[9]=VS(cur4, "Firm_Market_Share");								//current firm's market share
        v[11]=abs(v[9]-v[8]);											//absolute difference between current firm's market share and sector's avg
        v[10]=min(v[11],v[10]);											//min between 1 and the current difference
            if(v[11]==v[10])											//if the current difference is the lowest one
               v[12]=v[9];												//use current firm's market share
            else														//if current difference is not the lowest one
               v[12]=v[12];												//use the current value, until find the lowest difference
     	}
		
		cur5=SEARCH_CND("Firm_Market_Share", v[12]);					//search the firm with the closest market share to the average one
		v[2]=VS(cur5, "Firm_Market_Share");								//market share of the average firm
		v[3]=((v[2]*v[20])*(1+v[23]))/v[22];							//new firm's productive capacity to reach the average market share   
		v[4]=min(v[3],(v[34]+v[44]));									//new firm's productive capacity can not be higher than productive capacity available
		v[7]=v[4]*v[35];												//new firm's number of capitals
   		v[64]=v[3]-v[4];												//new firm's capacity needed
		
		v[62]=V("entry_debt_share");									//if =1, all debt.
		v[63]=v[7]*v[25];												//cost of initial capital
		
			cur6=RNDDRAWS(root, "BANKS", "Bank_Market_Share");
			v[19]=VS(cur6, "bank_id");
		
              cur=ADDOBJ_EX("FIRMS",cur5);								//add new firm using as exemple the firm with closest market share to the average
              //begin writting some lagged variables and parameters           
              WRITES(cur, "firm_date_birth", t);										//firm's date of birth
              WRITES(cur, "firm_id",t);											//firm's number
			  WRITES(cur, "firm_bank", v[19]);										//firm's bank identifier
              WRITELS(cur, "Firm_Market_Share",v[2], t);								//firm's market share
              WRITELS(cur, "Firm_Effective_Market_Share",v[2], t);						//firm's effective market share
              WRITELS(cur, "Firm_Effective_Orders",(v[2]*v[20]), t);					//firm's effective orders
              WRITELS(cur, "Firm_Stock_Inventories",(v[2]*v[20]*v[23]), t);				//firm's inventories
              WRITELS(cur, "Firm_Price",v[36], t);										//firm's price
              WRITELS(cur, "Firm_Max_Productivity",v[39], t);							//firm's max capital productivity
              WRITELS(cur, "Firm_Frontier_Productivity",v[39], t);						//firm's frontier productivity
			  WRITELS(cur, "Firm_Stock_Loans",v[62]*v[63],t);							//firm's stock of debt is the price of capital goods bought
              WRITELS(cur, "Firm_Stock_Deposits",0,t);									//firm's stock of financial assets is zero
              for(i=0;i<=v[0];i++)
				WRITELLS(cur,"Firm_Demand_Capital_Goods", 0, t, i);
			
			  v[50]=COUNTS(cur,"CAPITALS");
			  CYCLE_SAFES(cur, cur1, "CAPITALS")								//CYCLE trough firm's capitals
				{
					if(v[50]>1)
						{
						DELETE(cur1);												//delete the current capital
						v[50]=v[50]-1;
						}
					else	
						v[50]=v[50];
				}
			cur2=ADDNOBJS(cur, "CAPITALS", v[7]);
			CYCLES(cur, cur1, "CAPITALS")
			{
			WRITES(cur1, "capital_good_productive_capacity",(1/v[35]));
			WRITES(cur1, "capital_good_date_birth",t);					//write the new date of birth
			WRITES(cur1, "capital_good_depreciation_period",(t+v[38])); //write the new date of birth
			WRITES(cur1, "capital_good_to_replace",0);					//write current capital goods as not to replace
			WRITES(cur1, "capital_good_productivity_initial",v[39]);	//write current capital initial productivity
			WRITELS(cur1, "Capital_Good_Acumulated_Production",0,t);	//write current capital acumulated production as zero
			}
		
			v[13]=VS(cur6,"Bank_Interest_Rate_Long_Term");
			
			CYCLE_SAFES(cur, cur1, "FIRM_LOANS")
			{
				v[51]=VS(cur1, "firm_loan_fixed_object");
				if (v[51]!=1)
				DELETE(cur1);
			}
			
			cur1 = ADDOBJS(cur, "FIRM_LOANS");
			WRITES(cur1, "firm_loan_total_amount", v[62]*v[63]);
			WRITES(cur1, "firm_loan_fixed_object", 0);
			WRITES(cur1, "firm_loan_fixed_amortization", (v[62]*v[63]/(2*v[0])));
			WRITES(cur1, "id_firm_loan_long_term", 1);
			WRITES(cur1, "firm_loan_interest_rate", v[13]);
       
		WRITE("Sector_Entry_Deposits_Needed", (1-v[62])*v[63] );	   
       }
       else
       v[4]=0;
   
	v[60]=SUM("Firm_Market_Share");												//sum of firms market share 
	CYCLE(cur, "FIRMS")															//cycle trough firms to normalize market shares
	{
	v[61]=VS(cur,"Firm_Market_Share");											//current firm's market share
	WRITES(cur, "Firm_Market_Share", (v[61]/v[60]));							//divide firm's market share by the sum of market shares
	}
RESULT(v[4])

EQUATION_DUMMY("Sector_Entry_Deposits_Needed", "Sector_Productive_Capacity_Entry")


EQUATION("Sector_Productive_Capacity_Available")
/*
This variable acumulates the productive capacity that exited the sector minus the productive capacity used by new possible firms
*/
	v[0]=V("Sector_Productive_Capacity_Exit");
	v[1]=V("Sector_Productive_Capacity_Entry");
	v[2]=VL("Sector_Productive_Capacity_Available", 1);
	v[3]=v[2]-v[1]+v[0];
RESULT(v[3])




