/***********ANALYSIS VARIABLES************/

/*****COUNTRY STATS*****/

EQUATION("GDP")//Quarterly Nominal GDP
RESULT(VS(country, "Country_GDP"))

EQUATION("P")//Price Index
RESULT(VS(country, "Country_Price_Index"))

EQUATION("CPI")//Price Index
RESULT(VS(country, "Country_Consumer_Price_Index"))

EQUATION("P_G")//Annual Inflation
RESULT(VS(country, "Country_Annual_Inflation"))

EQUATION("CPI_G")//Annual CPI Inflation
RESULT(VS(country, "Country_Annual_CPI_Inflation"))

EQUATION("U")//Unemployment
RESULT(VS(country, "Country_Idle_Capacity"))

EQUATION("EMP")//Employment
RESULT(SUMS(country, "Sector_Employment"))

EQUATION("GDP_G")//GDP real annual growth rate
RESULT(VS(country, "Country_Annual_Real_Growth"))

EQUATION("G_n")//GDP nominal annual growth rate
RESULT(VS(country, "Country_Annual_Growth"))

EQUATION("Cri")//Crisis counters
RESULT(VS(country, "Country_Likelihood_Crisis"))

EQUATION("C")//Quarterly Nominal Consumption
RESULT(VS(country, "Country_Total_Classes_Expenses"))

EQUATION("I")//Quarterly Nominal Investment
RESULT(VS(country, "Country_Total_Investment_Expenses"))

EQUATION("PROD")//Average Productivity
RESULT(VS(country, "Country_Avg_Productivity"))

EQUATION("MK")//Average Markup
RESULT(VS(country, "Country_Avg_Markup"))

EQUATION("KL")//Capital labour ratio
RESULT(VS(country,"Country_Capital_Labor_Ratio"))

EQUATION("PR")//Profit Rate
RESULT(VS(country,"Country_Avg_Profit_Rate"))

EQUATION("PCU")//Productive Capacity Utilization Rate
RESULT(VS(country,"Country_Capacity_Utilization"))

EQUATION("PSH")//Profit Share
RESULT(VS(country,"Country_Profit_Share"))

EQUATION("WSH")//Wage Share
RESULT(VS(country,"Country_Wage_Share"))

EQUATION("GINI")//Wage Share
RESULT(VS(country,"Country_Gini_Index"))


/*****REAL STATS*****/

EQUATION("GDP_r")//Real GDP
RESULT(VS(country, "Country_Real_GDP_Demand"))

EQUATION("C_r")//Quarterly Real Consumption
RESULT(VS(country, "Country_Total_Classes_Expenses")/V("P"))

EQUATION("I_r")//Quarterly Real Investment
RESULT(VS(country, "Country_Total_Investment_Expenses")/V("P"))

EQUATION("INVE_r")//Real Aggregate Inventories
RESULT(VS(country, "Country_Inventories")/V("P"))

EQUATION("K_r")//Real Stock of Capital
RESULT(VS(country, "Country_Capital_Stock")/V("P"))

EQUATION("G_r")//Quarterly Real Government Expenses
RESULT(VS(government, "Government_Effective_Expenses")/V("P"))

EQUATION("PROFITS")//Real Profits
RESULT(VS(country, "Country_Total_Profits")/V("P"))

EQUATION("WAGE")//Real Wages
RESULT(VS(country, "Country_Total_Wages")/V("P"))

EQUATION("M_r")//Quarterly Real Imports
RESULT(VS(country, "Country_Nominal_Imports")/V("P"))

EQUATION("X_r")//Quarterly Real Exports
RESULT(VS(country, "Country_Nominal_Exports")/V("P"))

EQUATION("NX_r")//Quarterly Real Net Exports
RESULT(V("X_r")-V("M_r"))


/*****FINANCIAL STATS*****/

EQUATION("DEBT_RT_C")//Average Debt Rate of Consumption good sector
RESULT(VS(consumption, "Sector_Avg_Debt_Rate"))

EQUATION("DEBT_RT_K")//Average Debt Rate of Capital good sector
RESULT(VS(capital, "Sector_Avg_Debt_Rate"))

EQUATION("DEBT_RT_I")//Average Debt Rate of Intermediate good sector
RESULT(VS(input, "Sector_Avg_Debt_Rate"))

EQUATION("DEBT_RT_FI")//Average Debt Rate of all firms
RESULT(VS(country, "Country_Debt_Rate_Firms"))

EQUATION("DEBT_RT_CL")//Average Debt Rate of all classes
RESULT(VS(country, "Country_Debt_Rate_Class"))

EQUATION("DEBT_FS_ST")//Stock of short term debt in the financial sector
RESULT(VS(financial, "Financial_Sector_Stock_Loans_Short_Term"))

EQUATION("DEBT_FS_LT")//Stock of long term debt in the financial sector
RESULT(VS(financial, "Financial_Sector_Stock_Loans_Long_Term"))

EQUATION("DEBT_FS")//Stock of total debt in the financial sector
RESULT(VS(financial, "Financial_Sector_Total_Stock_Loans"))

EQUATION("DEP_FS")//Stock of total deposits in the financial sector
RESULT(VS(financial, "Financial_Sector_Stock_Deposits"))

EQUATION("FS_STR")//Financial sector short term rate
RESULT(VS(financial, "Financial_Sector_Short_Term_Rate"))

EQUATION("FS_LEV")//Financial sector leverage
RESULT(VS(financial, "Financial_Sector_Leverage"))

EQUATION("FS_HHI")//Financial sector HHI
RESULT(VS(financial, "Financial_Sector_Normalized_HHI"))

EQUATION("FS_DR")//Financial sector Default rate
RESULT(VS(financial, "Financial_Sector_Default_Rate"))

EQUATION("FS_DEF")//Financial sector Default
RESULT(VS(financial, "Financial_Sector_Defaulted_Loans"))

EQUATION("FS_DMET")//Financial sector Demand Met
RESULT(VS(financial, "Financial_Sector_Demand_Met"))

EQUATION("FS_RES")//Financial sector Rescue
RESULT(VS(financial, "Financial_Sector_Rescue"))
   
EQUATION("FS_PR")//Financial sector profits
RESULT(VS(financial, "Financial_Sector_Profits"))   

EQUATION("PONZI")//Share of Firms in Ponzi position
RESULT(VS(country, "Country_Ponzi_Share"))    

EQUATION("SPEC")//Share of Firms in Speculative position
RESULT(VS(country, "Country_Speculative_Share"))  

EQUATION("HEDGE")//Share of Firms in Hedge position
RESULT(VS(country, "Country_Hedge_Share")) 

EQUATION("IR")//Basic Interest Rate
RESULT(VS(financial, "Central_Bank_Basic_Interest_Rate"))     

EQUATION("IR_DEP")//Interest Rate on Deposits
RESULT(VS(financial, "Financial_Sector_Interest_Rate_Deposits"))  

EQUATION("IR_ST")//Interest Rate on Short Term Loans
RESULT(VS(financial, "Financial_Sector_Avg_Interest_Rate_Short_Term"))  

EQUATION("IR_LT")//Interest Rate on Long Term Loans
RESULT(VS(financial, "Financial_Sector_Avg_Interest_Rate_Long_Term"))   

EQUATION("BKR")//Number of Bankrupt Events
RESULT(VS(country, "Exit_Bankruptcy_Events")) 

EQUATION("BKR_RT")//Bankrupt Rate
RESULT(VS(country, "Exit_Bankruptcy_Share")) 

/*****CLASS STATS*****/
EQUATION("YSH_A")
RESULT(VS(aclass, "Class_Income_Share"))

EQUATION("YSH_B")
RESULT(VS(bclass, "Class_Income_Share"))

EQUATION("YSH_C")
RESULT(VS(cclass, "Class_Income_Share"))

EQUATION("WSH_A")
RESULT(VS(aclass, "Class_Wealth_Share"))

EQUATION("WSH_B")
RESULT(VS(bclass, "Class_Wealth_Share"))

EQUATION("WSH_C")
RESULT(VS(cclass, "Class_Wealth_Share"))


/*****SECTORAL STATS*****/

EQUATION("P_C")//Average Price of Consumption good secto
RESULT(VS(consumption, "Sector_Avg_Price"))

EQUATION("P_K")//Average Price of Capital good sector
RESULT(VS(capital, "Sector_Avg_Price"))

EQUATION("P_I")//Average Price of Intermediate good sector
RESULT(VS(input, "Sector_Avg_Price"))

EQUATION("PX_C")//Average External Price of Consumption good secto
RESULT(VS(consumption, "Sector_External_Price"))

EQUATION("PX_K")//Average External Price of Capital good sector
RESULT(VS(capital, "Sector_External_Price"))

EQUATION("PX_I")//Average External Price of Intermediate good sector
RESULT(VS(input, "Sector_External_Price"))

EQUATION("W_C")//Average Wage of Consumption good sector
RESULT(VS(consumption, "Sector_Avg_Wage"))

EQUATION("W_K")//Average Wage of Capital good sector
RESULT(VS(capital, "Sector_Avg_Wage"))

EQUATION("W_I")//Average Wage of Intermediate good sector
RESULT(VS(input, "Sector_Avg_Wage"))

EQUATION("MK_C")//Average Markup of Consumption good sector
RESULT(VS(consumption, "Sector_Avg_Markup"))

EQUATION("MK_K")//Average Markup of Capital good sector
RESULT(VS(capital, "Sector_Avg_Markup"))

EQUATION("MK_I")//Average Markup of Intermediate good sector
RESULT(VS(input, "Sector_Avg_Markup"))

EQUATION("PROD_C")//Average Productivity of Consumption good sector
RESULT(VS(consumption, "Sector_Avg_Productivity"))

EQUATION("PROD_K")//Average Productivity of Capital good sector
RESULT(VS(capital, "Sector_Avg_Productivity"))

EQUATION("PROD_I")//Average Productivity of Intermediate good sector
RESULT(VS(input, "Sector_Avg_Productivity"))

EQUATION("U_C")//Unemployment Rate of Consumption good sector
RESULT(VS(consumption, "Sector_Idle_Capacity"))

EQUATION("U_K")//Unemployment Rate of Capital good sector
RESULT(VS(capital, "Sector_Idle_Capacity"))

EQUATION("U_I")//Unemployment Rate of Intermediate good sector
RESULT(VS(input, "Sector_Idle_Capacity"))

EQUATION("HHI_C")//Inverse HHI of Consumption good sector
RESULT(VS(consumption, "Sector_Normalized_HHI"))

EQUATION("HHI_K")//Inverse HHI of Capital good sector
RESULT(VS(capital, "Sector_Normalized_HHI"))

EQUATION("HHI_I")//Inverse HHI of Intermediate good sector
RESULT(VS(input, "Sector_Normalized_HHI"))

EQUATION("IRST_C")//Average Short Term Interest Rate of Consumption good sector
RESULT(VS(consumption, "Sector_Avg_Interest_Rate_Short_Term"))

EQUATION("IRST_K")//Average Short Term Interest Rate of Capital good sector
RESULT(VS(capital, "Sector_Avg_Interest_Rate_Short_Term"))

EQUATION("IRST_I")//Average Short Term Interest Rate of Intermediate good sector
RESULT(VS(input, "Sector_Avg_Interest_Rate_Short_Term"))

EQUATION("IRLT_C")//Average Long Term Interest Rate of Consumption good sector
RESULT(VS(consumption, "Sector_Avg_Interest_Rate_Long_Term"))

EQUATION("IRLT_K")//Average Long Term Interest Rate of Capital good sector
RESULT(VS(capital, "Sector_Avg_Interest_Rate_Long_Term"))

EQUATION("IRLT_I")//Average Long Term Interest Rate of Intermediate good sector
RESULT(VS(input, "Sector_Avg_Interest_Rate_Long_Term"))

/*****COUNTRY GROWTH STATS*****/

EQUATION("EMP_G")//Quarterly Employment Growth rate
RESULT(LAG_GROWTH(p,"EMP",1))

EQUATION("CON_G")//Quarterly Real Consumption Growth rate
RESULT(LAG_GROWTH(p,"C_r",1))

EQUATION("INV_G")//Quarterly Real Investment Growth rate
RESULT(LAG_GROWTH(p,"I_r",1))

EQUATION("PROD_G")//Average Productivity Growth
RESULT(LAG_GROWTH(p,"PROD",1))

EQUATION("MK_G")//Average Markup Growth
RESULT(LAG_GROWTH(p,"MK",1))

EQUATION("INVE_G")//Real Aggregate Inventories Growth
RESULT(LAG_GROWTH(p,"INVE_r",1))

EQUATION("K_G")//Real Stock of Capital Growth
RESULT(LAG_GROWTH(p,"K_r",1))

EQUATION("PROFITS_G")//Real Profits Growth rate
RESULT(LAG_GROWTH(p,"PROFITS",1))

EQUATION("WAGE_G")//Real Wages growth rate
RESULT(LAG_GROWTH(p,"WAGE",1))

EQUATION("GOV_G")//Quarterly Real Government Expenses Growth rate
RESULT(LAG_GROWTH(p,"G_r",1))

EQUATION("PDEBT_G")//Public Debt Growth rate
RESULT(LAG_GROWTH(p,"PDEBT",1))

EQUATION("M_G")//Quarterly Real Imports Growth rate
RESULT(LAG_GROWTH(p,"M_r",1))

EQUATION("X_G")//Quarterly Real Exports Growth rate
RESULT(LAG_GROWTH(p,"X_r",1))

EQUATION("NX_G")//Quarterly Real Net Exports Growth rate
RESULT(LAG_GROWTH(p,"NX_r",1))


/*****MACRO SHARE STATS*****/

EQUATION("CGDP")
RESULT(V("Country_Total_Classes_Expenses")/V("Country_GDP_Demand"))

EQUATION("IGDP")
RESULT(V("Country_Total_Investment_Expenses")/V("Country_GDP_Demand"))

EQUATION("GGDP")
RESULT(V("Government_Effective_Expenses")/V("Country_GDP_Demand"))

EQUATION("NXGDP")
RESULT((V("Country_Nominal_Exports")-V("Country_Nominal_Imports"))/V("Country_GDP_Demand"))

EQUATION("XGDP")
RESULT(V("Country_Nominal_Exports")/V("Country_GDP_Demand"))

EQUATION("MGDP")
RESULT(V("Country_Nominal_Imports")/V("Country_GDP_Demand"))

EQUATION("INVGDP")
RESULT(V("Country_Inventories")/V("Country_GDP_Demand"))

EQUATION("KGDP")
RESULT(V("Country_Capital_Stock")/V("Country_GDP_Demand")) 


/*****FINANCIAL GROWTH STATS*****/

EQUATION("DEBT_FS_ST_G")//Stock of short term debt growth in the financial sector
RESULT(LAG_GROWTH(p,"DEBT_FS_ST",1))

EQUATION("DEBT_FS_LT_G")//Stock of long term debt growth in the financial sector
RESULT(LAG_GROWTH(p,"DEBT_FS_LT",1))

EQUATION("DEBT_FS_G")//Stock of total debt growth in the financial sector
RESULT(LAG_GROWTH(p,"DEBT_FS",1))

EQUATION("DEP_FS_G")//Stock of total deposits growth in the financial sector
RESULT(LAG_GROWTH(p,"DEP_FS",1))


/*****GOVERNMENT STATS*****/

EQUATION("G")//Quarterly Government Expenses
RESULT(VS(government, "Government_Effective_Expenses"))

EQUATION("TT")//Quarterly Total Taxes
RESULT(VS(government, "Government_Total_Taxes"))

EQUATION("DT")//Quarterly Direct Taxes
RESULT(VS(government, "Government_Income_Taxes"))

EQUATION("IT")//Quarterly Indiret Taxes
RESULT(VS(government, "Government_Indirect_Taxes"))

EQUATION("PST")//Primary Surplus Target
RESULT(VS(government, "Government_Surplus_Rate_Target"))

EQUATION("PS_GDP")//Primary Surplus to GDP
RESULT(VS(government, "Government_Surplus_GDP_Ratio"))

EQUATION("PDEBT")//Public Debt
RESULT(VS(government, "Government_Debt"))

EQUATION("PDEBT_GDP")//Public Debt to GDP
RESULT(VS(government, "Government_Debt_GDP_Ratio"))


/*****EXTERNAL STATS*****/

EQUATION("GDPX_r")//Real External Income
RESULT(VS(external, "External_Real_Income"))

EQUATION("X")//Quarterly Nominal Exports
RESULT(VS(external, "Country_Nominal_Exports"))

EQUATION("M")//Quarterly Nominal Imports
RESULT(VS(external, "Country_Nominal_Imports"))

EQUATION("NX")//Quarterly Trade Balance
RESULT(VS(external, "Country_Trade_Balance"))

EQUATION("CF")//Quarterly Capital Flows
RESULT(VS(external, "Country_Capital_Flows"))

EQUATION("RES")//Stock of International Reserves
RESULT(VS(external, "Country_International_Reserves"))

EQUATION("RES_GDP")//Stock of International Reserves to GDP
RESULT(VS(external, "Country_International_Reserves_GDP_Ratio"))

EQUATION("DX_GDP")//Stock of International Reserves to GDP
RESULT(VS(external, "Country_External_Debt_GDP_Ratio"))

EQUATION("ER")//Exchange Rate
RESULT(VS(external, "Country_Exchange_Rate"))

