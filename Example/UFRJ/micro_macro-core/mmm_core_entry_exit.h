/*****ENTRY AND EXIT VARIABLES*****/


EQUATION("Exit")
/*
Macro Variable
This variable counts productive capacity that exited the sector in each time step. Firm objects are deleted inside that variable due to two possibilities: small market share or high debt rate. In the second case, if entry conitions are met, the firm can be bought by a new one.
*/

v[0]=V("sector_survival_period");
v[17]=v[18]=v[19]=0;
CYCLE(cur1, "SECTORS")
{
	i = COUNTS(cur1, "FIRMS" );									// count the existing number of firms 
	v[11]=v[12]=v[14]=0;										// initialize the cycles
	CYCLE_SAFES( cur1, cur, "FIRMS" )							// use a robust cycle to delete objects
	{
     v[1]=VS(cur, "Firm_Market_Share");							//firm's curent market share
     v[2]=VS(cur, "Firm_Productive_Capacity");					//firm's current productive capacity																				
     v[3]=VS(cur, "firm_date_birth");							//firm's date birth
	 v[4]=VS(cur, "Firm_Avg_Debt_Rate");						//firm's avg debt rate
     v[5]=VLS(cur, "Firm_Avg_Debt_Rate", 1);					//firm's avg debt rate in the last period
     v[6]=VS(cur, "Firm_Stock_Debt");
     v[7]=VS(cur, "Firm_Stock_Financial_Assets");
  
     if ( v[1] <= 0.001 && i > 1 && t>(v[0]+v[3]))				//if firm's market share is near zero
     {
     //plog("\nFirm Deleted - Small Market Share");				//write on log window
      	if (v[7]>=v[6])											//firm pays current debt with current deposits 
      		{
      		v[11]=v[11]+(v[7]-v[6]);							//deposits distributed to income classes (no need to substract from bank's stock)
      		v[12]=v[12]+v[2];									//available productive capacity
      		v[14]=v[14];										//defaulted loans
      		}
      	else													//if firm can not pay current debt with current deposits
      		{
       		v[11]=v[11];										//deposits distributed to income classes (no need to substract from bank's stock)
      		v[12]=v[12]+v[2];									//available productive capacity
      		v[14]=v[14]+(v[6]-v[7]);							//defaulted loans
      		}
      DELETE(cur); 												//delete current firm
	  --i;														//subtract 1 from the number of firms counter
     }
     else														//if firm's current market share is not near zero																																						//if firm's avg debt rate is not higher than 1 (do not delete current firm						
     {
		if(v[4]>1 &&i>1 && t>(v[0]+v[3]))
		{
	//plog("\nFirm Deleted - High Debt");						//write on log window
      	if (v[7]>=v[6])											//firm pays current debt with current deposits 
      		{
      		v[11]=v[11]+(v[7]-v[6]);							//deposits distributed to income classes (no need to substract from bank's stock)
      		v[12]=v[12]+v[2];									//available productive capacity
      		v[14]=v[14];										//defaulted loans
      		}
      	else													//if firm can not pay current debt with current deposits
      		{
       		v[11]=v[11];										//deposits distributed to income classes (no need to substract from bank's stock)
      		v[12]=v[12]+v[2];									//available productive capacity
      		v[14]=v[14]+(v[6]-v[7]);							//defaulted loans
      		}
		DELETE(cur); 											//delete current firm
		--i;					
		}
		else
		{
        v[11]=v[11];											//deposits distributed to income classes (no need to substract from bank's stock)
      	v[12]=v[12];											//available productive capacity
      	v[14]=v[14];					  						//defaulted loans
      	}
	 }
	WRITES(cur1, "Sector_Productive_Capacity_Exit", v[12]);
	}
v[17]=v[17]+v[11];
v[18]=v[18]+v[12];
v[19]=v[19]+v[14];
}
WRITE("Exit_Deposits_Distributed", v[17]);
WRITE("Exit_Productive_Capacity", v[18]);
WRITE("Exit_Defaulted_Loans", v[19]);																														
RESULT(0)

EQUATION("Exit_Deposits_Distributed")
RESULT(CURRENT)
EQUATION("Exit_Productive_Capacity")
RESULT(CURRENT)
EQUATION("Exit_Defaulted_Loans")
RESULT(CURRENT)
EQUATION("Sector_Productive_Capacity_Exit")
RESULT(CURRENT)


EQUATION("Sector_Entry_Condition")
/*
Can only be 0 or 1, if all enter conditions are met.
*/
	v[1]=V("sector_investment_period");
	v[2]=VL("Sector_Effective_Orders",1);
	v[3]=VL("Sector_Effective_Orders",2);
	v[4]=VL("Sector_Effective_Orders",3);
	v[5]=VL("Sector_Effective_Orders",v[1]);
	v[6]=v[2]-v[5];
	v[7]=(v[2]-v[3])/v[3];
	v[8]=(v[3]-v[4])/v[4];
  
    if(v[6]>0 && v[7]>0 && v[8]>0)
     v[9]=1;
    else
     v[9]=0;
            
RESULT(v[9])



EQUATION("Sector_Productive_Capacity_Entry")
/*
Sector Variable
In this variable a new firm enters if there is market space available and the entry condiion is met
*/

   v[1]=V("Sector_Entry_Condition");									//value of entry conditions
   v[34]=VL("Sector_Productive_Capacity_Available",1);					//productive capacity available in the last period
   v[6]=SUM("Firm_Market_Share");										//sum of firm's market share 
      if(v[1]>0&&v[6]<1)												//if entry conditions are met and there are market space 
      {
      v[20]=V("Sector_Effective_Orders");								//sector effective orders
      v[0]=V("sector_investment_period");								//sector investment period
      v[22]=V("sector_desired_degree_capacity_utilization");			//sector degree of capacity utilization
      v[23]=V("sector_desired_inventories_proportion");					//sector inventories proportion
	  v[25]=V("Price_Capital_Goods");									//price of capital goods
      v[34]=VL("Sector_Productive_Capacity_Available",1);				//productive capacity available in the last period
	  v[44]=V("Sector_Productive_Capacity_Exit");						//productive capacity exited in the current period
      v[36]=V("Sector_Avg_Price");										//sector avg price
      v[35]=V("sector_capital_output_ratio");							//sector capital output ratio
      v[38]=V("sector_depreciation_period");							//sector depreciation period
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
		v[51]=MIN("Firm_Market_Share");
		
		cur5=SEARCH_CND("Firm_Market_Share", v[12]);					//search the firm with the closest market share to the average one
		v[2]=VS(cur5, "Firm_Market_Share");								//market share of the average firm
		v[3]=((v[2]*v[20]))/v[22];										//new firm's productive capacity to reach the average market share   
		v[4]=min(v[3],(v[34]+v[44]));									//new firm's productive capacity can not be higher than productive capacity available
		v[7]=v[4]*v[35];												//new firm's number of capitals
   		
              cur=ADDOBJ_EX("FIRMS",cur5);								//add new firm using as exemple the firm with closest market share to the average
			  
              //begin writting some lagged variables and parameters           
              WRITES(cur, "firm_date_birth", t);										//firm's date of birth
              WRITES(cur, "id_firm_number",t);											//firm's number
              WRITELS(cur, "Firm_Market_Share",v[2], t);								//firm's market share
              WRITELS(cur, "Firm_Effective_Market_Share",v[2], t);						//firm's effective market share
              WRITELS(cur, "Firm_Desired_Market_Share",v[2], t);						//firm's desired market share
              WRITELS(cur, "Firm_Avg_Market_Share",v[2], t);							//firm's avg market share
              WRITELS(cur, "Firm_Effective_Orders",(v[2]*v[20]), t);					//firm's effective orders
              WRITELS(cur, "Firm_Stock_Inventories",(v[2]*v[20]*v[23]), t);				//firm's inventories
              WRITELS(cur, "Firm_Price",v[36], t);										//firm's price
              WRITELS(cur, "Firm_Max_Productivity",v[39], t);							//firm's max capital productivity
              WRITELS(cur, "Firm_Frontier_Productivity",v[39], t);						//firm's frontier productivity
			  WRITELS(cur, "Firm_Stock_Debt",v[7]*v[25],t);								//firm's stock of debt is the price of capital goods bought
              WRITELS(cur, "Firm_Stock_Financial_Assets",0,t);							//firm's stock of financial assets is zero
              WRITELS(cur, "Firm_Productive_Capacity",v[7],t);
			  for(i=0;i<=v[0];i++)
				WRITELLS(cur,"Firm_Demand_Capital_Goods", 0, t, i);
			 
			  
			  v[50]=COUNTS(cur,"CAPITALS");
			  CYCLE_SAFES(cur, cur1, "CAPITALS")										//CYCLE trough firm's capitals
				{
					if(v[50]>1)
						{
						DELETE(cur1);													//delete the current capital
						v[50]=v[50]-1;
						}
					else	
						v[50]=v[50];
				}
			cur2=ADDNOBJS(cur, "CAPITALS", v[7]);
			CYCLES(cur, cur1, "CAPITALS")
			{
			WRITES(cur1, "capital_good_productive_capacity",(1/v[35]));
			WRITES(cur1, "capital_good_date_birth",t);									//write the new date of birth
			WRITES(cur1, "capital_good_depreciation_period",(t+v[38])); 				//write the new date of birth
			WRITES(cur1, "capital_good_to_replace",0);									//write current capital goods as not to replace
			WRITES(cur1, "capital_good_productivity_initial",v[39]);					//write current capital initial productivity
			WRITELS(cur1, "Capital_Good_Acumulated_Production",0,t);					//write current capital acumulated production as zero
			}         
       }
       else
       v[4]=0;
   
	v[60]=SUM("Firm_Market_Share");														//sum of firms market share 
	CYCLE(cur, "FIRMS")																	//cycle trough firms to normalize market shares
	{
	v[61]=VS(cur,"Firm_Market_Share");													//current firm's market share
	WRITES(cur, "Firm_Market_Share", (v[61]/v[60]));									//divide firm's market share by the sum of market shares
	}
   
RESULT(v[4])


EQUATION("Sector_Productive_Capacity_Available")
/*
This variable acumulates the productive capacity that exited the sector minus the productive capacity used by new possible firms
*/
	v[0]=V("Sector_Productive_Capacity_Exit");
	v[1]=V("Sector_Productive_Capacity_Entry");
	v[2]=VL("Sector_Productive_Capacity_Available", 1);
	v[3]=v[2]-v[1]+v[0];
RESULT(v[3])




