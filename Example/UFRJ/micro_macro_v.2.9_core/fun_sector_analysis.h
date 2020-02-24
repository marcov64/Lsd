
EQUATION("P_C")
/*
Average Price of Consumption good sector
*/
cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Price");
RESULT(v[0])

EQUATION("P_K")
/*
Average Price of Capital good sector
*/
cur = SEARCH_CNDS(root, "id_capital_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Price");
RESULT(v[0])

EQUATION("P_I")
/*
Average Price of Intermediate good sector
*/
cur = SEARCH_CNDS(root, "id_intermediate_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Price");
RESULT(v[0])


EQUATION("W_C")
/*
Average Wage of Consumption good sector
*/
cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Wage");
RESULT(v[0])

EQUATION("W_K")
/*
Average Price of Capital good sector
*/
cur = SEARCH_CNDS(root, "id_capital_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Wage");
RESULT(v[0])

EQUATION("W_I")
/*
Average Price of Intermediate good sector
*/
cur = SEARCH_CNDS(root, "id_intermediate_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Wage");
RESULT(v[0])


EQUATION("MK_C")
/*
Average Markup of Consumption good sector
*/
cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Markup");
RESULT(v[0])

EQUATION("MK_K")
/*
Average Markup of Capital good sector
*/
cur = SEARCH_CNDS(root, "id_capital_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Markup");
RESULT(v[0])

EQUATION("MK_I")
/*
Average Markup of Intermediate good sector
*/
cur = SEARCH_CNDS(root, "id_intermediate_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Markup");
RESULT(v[0])


EQUATION("PROD_C")
/*
Average Productivity of Consumption good sector
*/
cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Productivity");
RESULT(v[0])

EQUATION("PROD_K")
/*
Average Productivity of Capital good sector
*/
cur = SEARCH_CNDS(root, "id_capital_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Productivity");
RESULT(v[0])

EQUATION("PROD_I")
/*
Average Productivity of Intermediate good sector
*/
cur = SEARCH_CNDS(root, "id_intermediate_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Productivity");
RESULT(v[0])


EQUATION("DEBT_C")
/*
Average Debt Rate of Consumption good sector
*/
cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Debt_Rate");
RESULT(v[0])

EQUATION("DEBT_K")
/*
Average Debt Rate of Capital good sector
*/
cur = SEARCH_CNDS(root, "id_capital_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Debt_Rate");
RESULT(v[0])

EQUATION("DEBT_I")
/*
Average Debt Rate of Intermediate good sector
*/
cur = SEARCH_CNDS(root, "id_intermediate_goods_sector", 1);
v[0]=VS(cur, "Sector_Avg_Debt_Rate");
RESULT(v[0])


EQUATION("U_C")
/*
Unemployment Rate of Consumption good sector
*/
cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);
v[0]=VS(cur, "Sector_Unemployment");
RESULT(v[0])

EQUATION("U_K")
/*
Unemployment Rate of Capital good sector
*/
cur = SEARCH_CNDS(root, "id_capital_goods_sector", 1);
v[0]=VS(cur, "Sector_Unemployment");
RESULT(v[0])

EQUATION("U_I")
/*
Unemployment Rate of Intermediate good sector
*/
cur = SEARCH_CNDS(root, "id_intermediate_goods_sector", 1);
v[0]=VS(cur, "Sector_Unemployment");
RESULT(v[0])


EQUATION("HHI_C")
/*
Inverse HHI of Consumption good sector
*/
cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);
v[0]=VS(cur, "Sector_Inverse_HHI");
RESULT(v[0])

EQUATION("HHI_K")
/*
Inverse HHI of Capital good sector
*/
cur = SEARCH_CNDS(root, "id_capital_goods_sector", 1);
v[0]=VS(cur, "Sector_Inverse_HHI");
RESULT(v[0])

EQUATION("HHI_I")
/*
Inverse HHI of Intermediate good sector
*/
cur = SEARCH_CNDS(root, "id_intermediate_goods_sector", 1);
v[0]=VS(cur, "Sector_Inverse_HHI");
RESULT(v[0])



