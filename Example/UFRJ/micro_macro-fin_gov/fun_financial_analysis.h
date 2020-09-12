
EQUATION("DEBT_RT_C")
/*
Average Debt Rate of Consumption good sector
*/
cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Debt_Rate");
RESULT(v[0])

EQUATION("DEBT_RT_K")
/*
Average Debt Rate of Capital good sector
*/
cur = SEARCH_CNDS(root, "id_capital_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Debt_Rate");
RESULT(v[0])

EQUATION("DEBT_RT_I")
/*
Average Debt Rate of Intermediate good sector
*/
cur = SEARCH_CNDS(root, "id_intermediate_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Debt_Rate");
RESULT(v[0])

EQUATION("DEBT_RT_1")
/*
Average Debt Rate of Income Class 1
*/
cur = SEARCH_CNDS(root, "id_class", 1);
v[0]=VS(cur, "Class_Avg_Debt_Rate");
RESULT(v[0])

EQUATION("DEBT_RT_2")
/*
Average Debt Rate of Income Class 2
*/
cur = SEARCH_CNDS(root, "id_class", 2);
v[0]=VS(cur, "Class_Avg_Debt_Rate");
RESULT(v[0])

EQUATION("DEBT_RT_3")
/*
Average Debt Rate of Income Class 3
*/
cur = SEARCH_CNDS(root, "id_class", 3);
v[0]=VS(cur, "Class_Avg_Debt_Rate");
RESULT(v[0])

EQUATION("DEBT_RT_FI")
/*
Average Debt Rate of all firms
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Avg_Debt_Rate_Firms");
RESULT(v[0])

EQUATION("DEBT_RT_CL")
/*
Average Debt Rate of all classes
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Avg_Debt_Rate_Class");
RESULT(v[0])

EQUATION("DEBT_FS_ST")
/*
Stock of short term debt in the financial sector
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Stock_Loans_Short_Term");
RESULT(v[0])

EQUATION("DEBT_FS_LT")
/*
Stock of long term debt in the financial sector
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Stock_Loans_Long_Term");
RESULT(v[0])

EQUATION("DEBT_FS")
/*
Stock of total debt in the financial sector
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Total_Stock_Loans");
RESULT(v[0])

EQUATION("DEP_FS")
/*
Stock of total deposits in the financial sector
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Stock_Deposits");
RESULT(v[0])

EQUATION("DEBT_FS_ST_G")
/*
Stock of short term debt growth in the financial sector
*/
v[0]=V("DEBT_FS_ST");
v[1]=VL("DEBT_FS_ST", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])

EQUATION("DEBT_FS_LT_G")
/*
Stock of long term debt growth in the financial sector
*/
v[0]=V("DEBT_FS_LT");
v[1]=VL("DEBT_FS_LT", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])

EQUATION("DEBT_FS_G")
/*
Stock of total debt growth in the financial sector
*/
v[0]=V("DEBT_FS");
v[1]=VL("DEBT_FS", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])

EQUATION("DEP_FS_G")
/*
Stock of total deposits growth in the financial sector
*/
v[0]=V("DEP_FS");
v[1]=VL("DEP_FS", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])

EQUATION("FS_STR")
/*
Financial sector short term rate
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Short_Term_Rate");
RESULT(v[0])

EQUATION("FS_LEV")
/*
Financial sector leverage
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Leverage");
RESULT(v[0])

EQUATION("FS_HHI")
/*
Financial sector HHI
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Normalized_HHI");
RESULT(v[0])

EQUATION("FS_DR")
/*
Financial sector Default rate
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Default_Rate");
RESULT(v[0])

EQUATION("FS_DEF")
/*
Financial sector Default
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Defaulted_Loans");
RESULT(v[0])
                  
EQUATION("FS_PR")
/*
Financial sector profits
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Financial_Sector_Profits");
RESULT(v[0])   

EQUATION("PONZI")
/*
Share of Firms in Ponzi position
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Avg_Ponzi_Share");
RESULT(v[0])    

EQUATION("SPEC")
/*
Share of Firms in Speculative position
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Avg_Speculative_Share");
RESULT(v[0])  

EQUATION("HEDGE")
/*
Share of Firms in Hedge position
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Avg_Hedge_Share");
RESULT(v[0]) 

EQUATION("IR")
/*
Basic Interest Rate
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Basic_Interest_Rate");
RESULT(v[0])     

EQUATION("IR_DEP")
/*
Interest Rate on Deposits
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Interest_Rate_Deposits");
RESULT(v[0])  

EQUATION("IR_ST")
/*
Interest Rate on Short Term Loans
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Interest_Rate_Loans_Short_Term");
RESULT(v[0])  

EQUATION("IR_LT")
/*
Interest Rate on Long Term Loans
*/
cur = SEARCHS(root, "FINANCIAL");
v[0]=VS(cur, "Avg_Interest_Rate_Long_Term");
RESULT(v[0])   
 
                   
