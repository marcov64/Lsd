//#define EIGENLIB			// uncomment to use Eigen linear algebra library
//#define NO_POINTER_INIT	// uncomment to disable pointer checking

#include "fun_head.h"

MODELBEGIN

EQUATION("LINKS") // this equation creates linkages across objects (via pointers), it is executed only at t=1
if(t==1)
{
CYCLE(cur,"sector")
{
cur2=cur;
for(i=1; i<=V("n_country"); i++)
{
cur1=cur2->up->next;
if(cur1==NULL)
cur2->hook=cur;
else
cur2->hook=SEARCH_CNDS(cur1,"Id_sector", VS(cur2, "Id_sector"));
cur2=cur2->hook;
}
}
CYCLE(cur, "country")
{
CYCLES(cur, cur1, "sector")
{
CYCLES(cur1, cur2, "firm")
{
CYCLES(cur2, cur3, "comp")
{
cur4=SEARCH_CND("Id_country", VS(cur3,"Id_comp"));
cur3->hook=SEARCH_CNDS(cur4,"Id_sector", VS(cur1,"Id_sector"));
/*if(VS(cur3,"Id_comp")!=VS(cur,"Id_country")) //activate only in case of pure autarky (to set shares in foreign markets to zero)
WRITELLS(cur3, "ff", 0,t,0);*/
}
}
}
}
}
RESULT(1)

//WORLD level variables

EQUATION("Cw")    //world total expenditure for consumption expressed in common currency
RESULT(SUM("expenditure_comm"))

//aggregate variables country level

EQUATION("TB_Y")  //trade balances over world total consumption
RESULT(V("TB")/V("Cw"))

EQUATION("e") // Exchange rates
RESULT(max(0.0000001,VL("e",1)*(1+V("mu")*VL("TB_Y",1)+norm(0,V("sigma_u")))))    //flexible exchange rates
//RESULT(5)   //fixed exchange rates (exchange rates can also become exogenous and different across countries)

EQUATION ("Y") //Nominal GDP
v[0]=0;
CYCLE(cur,"sector")
{
v[0]=v[0]+WHTAVES(cur,"y","p");
}
RESULT (v[0])

EQUATION("Y_r") //Real GDP
RESULT(SUM("Ys_r"))

EQUATION("Y_L") //Real GDP per worker
RESULT (V("Y_r")/V("L"))

/*EQUATION("C_r")
RESULT(SUM("Cs_r"))
*/

EQUATION ("L") //total employment in the consumption sector
v[0]=0;
CYCLE(cur,"sector")
{
v[0]=v[0]+SUMS(cur,"l");
}
RESULT (v[0])

EQUATION("AvgProd")
v[0]=0;
CYCLE(cur,"sector")
{
v[1]=VS(cur,"AvgProd_s")*(VS(cur,"Fs")/SUM("Fs"));
v[0]=v[0]+v[1];
}
RESULT(v[0])

EQUATION("expenditure") // Demand is given by an exogenous expenditure + wages and profits paid by firms minus savings (in end of period equilibrium savings=G)
RESULT(VL("G",1)+VL("TOT_WAGE",1))

EQUATION("TOT_WAGE")
RESULT(V("W")*V("L"))

EQUATION("G") //exogenous expenditure component
RESULT(VL("G",1)*(1+V("g_d")))

EQUATION("expenditure_comm") //consumption expenditure expresses in international currency
RESULT(V("expenditure")*V("e"))

EQUATION("TB") //trade balance
v[0]=0;
v[1]=0;
v[2]=0;
CYCLE(cur, "sector")
{
CYCLES(cur, cur1,"firm")
{
v[0]=v[0]+VS(cur1,"exp");
v[1]=v[1]+VS(cur1,"int");
v[2]=v[2]+VS(cur1,"exc_dem");
}
}
WRITE("EXP", v[0]*V("e"));    //export in nominal terms
WRITE("IMP",V("expenditure")*V("e")-v[1]*V("e")); //import in nominal terms computed as a residual (total expenditure - internal expenditure)
RESULT(V("EXP")-(V("IMP")-v[2]*V("e")))

EQUATION("W") //national wage (the wage rate can also become an exogenous parameter heterogeneous across countries)
RESULT(VL("W",1)*(1+(VL("AvgProd",1)/VL("AvgProd",2)-1)))

EQUATION("Pindex") //the price index
v[0]=0;
CYCLE(cur,"sector")
{
v[1]=WHTAVES(cur,"p","f")/SUM("Fs");
v[0]=v[0]+v[1];
}
RESULT (v[0])

EQUATION("Expsh") //Computing shares of exporters (i.e. firms achieving a minimum level of market shares in at least one foreign market). Firms are considered exporters when they have a threshold level of market share equal to 5%*the minimum entry share. This equation should be changed when introducing export decisions and export costs.
v[0]=0;
CYCLE(cur, "firm")
{
cur1=SEARCHS(cur, "comp");
for(i=1; i<=V("n_country"); i++)
{
if(VS(cur1, "Id_comp")!= V("Id_country") && VS(cur1, "ff")>V("f_min")*1.05)
{
v[0]=v[0]+1;
break;
}
cur1=cur1->next;
}
}
RESULT(v[0]/V("n_firm"))

/*EQUATION("TEST")     //equation for testing aggregate identities and their consistency with micro data
v[0]=0;
CYCLE(cur,"sector")
{
CYCLES(cur,cur1,"firm")
{

//v[1]=VS(cur1,"p")*(VS(cur1,"D")-VS(cur1,"y"));
//v[1]=VS(cur1,"p")*max(0,VS(cur1,"y")-VS(cur1,"D"));
v[1]=VS(cur1,"p")*max(0,VS(cur1,"D")-VS(cur1,"y"));
v[0]=v[0]+v[1];
}
}
//v[1]=V("Y")-V("C")-V("I")+v[0];
//v[1]=V("Y")-V("L")*V("W")-V("Profits")-v[0];
v[1]=V("I")-V("Profits")-v[0];
RESULT(v[1])*/

//sectoral level variables


EQUATION ("Ys_r") //real sectoral production
v[0]=0;
CYCLE(cur,"firm")
{
v[1]=V("p_0")*VS(cur,"y")*V("e_0");
v[0]=v[0]+v[1];
}
RESULT (v[0])

EQUATION("Fs") //countries sectoral shares in the world market
RESULT (SUM ("f"))

EQUATION("AvgProd_s") //average (weighted) productivity in the sector
RESULT(WHTAVE("a","f")/V("Fs"))

EQUATION("competitiveness") //this function computes the (weighted) average of fitness levels by country and sector
CYCLE(cur, "country")
{
v[0]=VS(cur, "e");
CYCLES(cur, cur1, "sector")
{
v[1]=0;
CYCLE(cur2, "country")
{
v[2]=VS(cur2, "e");
cur3=SEARCH_CNDS(cur2, "Id_sector", VS(cur1,"Id_sector"));
CYCLES(cur3, cur4, "firm")
{
cur5=SEARCH_CNDS(cur4, "Id_comp", VS(cur,"Id_country"));
if(VS(cur,"Id_country")==VS(cur2,"Id_country"))
v[3]=1/(VS(cur4,"p")*v[2]/v[0]);  //fitness is equal to the inverse of price times exchange rates
else
v[3]=1/(VS(cur4,"p")*(1+V("tau"))*v[2]/v[0]); //the price is augmented by iceberg costs (tau) when fitness is computed abroad
//v[3]=0; //activate in case of autarky (replace the previous line)
v[1]=v[1]+v[3]*VLS(cur5,"ff",1);
}
}
WRITES(cur1, "Ehat", v[1]); //weighted average of fitness
}
}
RESULT(1)

EQUATION("m_min_int") //minum mark up level in the sector (to be used for initializing new firms)
cur=SEARCH("firm");
v[0]=VS(cur,"m");
CYCLE(cur, "firm")
{
if(VS(cur,"m")<v[0])
v[0]=VS(cur,"m");
}
RESULT(v[0])

EQUATION("a_max_int") //maximum productivity level in the sector within country
RESULT(MAXL("a",1))

EQUATION("sales_min_int") //minimum value of sales in the sector within country
cur=SEARCH("firm");
v[0]=VLS(cur,"sales",1);
CYCLE(cur, "firm")
{
if(VLS(cur,"sales",1)<v[0])
v[0]=VLS(cur,"sales",1);
}
RESULT(v[0])


EQUATION("min_max_tot") //the function compute the sectoral minimum of different variables by sector (across countries)
V("LINKS");
CYCLE(cur,"sector")
{
v[0]=VS(cur,"sales_min_int")*VLS(cur->up, "e",1);
v[1]=VS(cur,"m_min_int");
v[2]=VS(cur,"a_max_int");
cur1=cur;
for(i=1; i<=2*(V("n_country")); i++)
{
if(i > V("n_country"))
{
WRITES(cur1, "sales_min", v[0]); //minimum sales 
WRITES(cur1, "m_min", v[1]); //minimum mark up
WRITES(cur1, "a_max", v[2]); //maximum productivity
//WRITES(cur1, "Cs_r",0);
//WRITES(cur1, "imps_r",0);
}
else
{
if(v[0]>VS(cur1->hook, "sales_min_int")*VLS(cur1->hook->up,"e",1)) 
v[0]=VS(cur1->hook, "sales_min_int")*VLS(cur1->hook->up,"e",1);
if(v[1]>VS(cur1->hook, "m_min_int")) 
v[1]=VS(cur1->hook, "m_min_int");
if(v[2]<VS(cur1->hook, "a_max_int")) 
v[2]=VS(cur1->hook, "a_max_int");
}
cur1=cur1->hook;
}
}
RESULT(1)

EQUATION ("Imitation") //function to perform imiation across firms (in case of successfull imitation the parameter a_im takes the new coefficent)
V("min_max_tot");
CYCLE(cur, "country")
{
CYCLES(cur, cur1, "sector")
{
CYCLES(cur1,cur2, "firm")
{
if(RND<VS(cur2,"prob_im") && VLS(cur2,"a",1)!=VS(cur1,"a_max") && VLS(cur2,"a",1)!=VS(cur1,"a_max_int"))
{
cur3=cur1;
for(i=1; i<=V("n_country"); i++)
{
v[0]=0;
CYCLES(cur3,cur4, "firm")
{
if(VLS(cur4, "a",1)>VLS(cur2, "a",1))
{
if(VS(cur,"Id_country")==VS(cur3->up,"Id_country"))
WRITES(cur4,"prob", 1/(VLS(cur4,"a", 1)-VLS(cur2,"a",1)));                   
else
WRITES(cur4,"prob", 1/(V("Im_penalty")*(VLS(cur4,"a", 1)-VLS(cur2,"a",1))));
}
else
WRITES(cur4, "prob", 0);
v[0]=v[0]+VS(cur4, "prob");
}
WRITES(cur3->up, "PROB",v[0]);
cur3=cur3->hook;
}
cur3=RNDDRAW("country","PROB");
cur3=SEARCH_CNDS(cur3, "Id_sector", VS(cur1, "Id_sector"));
cur3=RNDDRAWS(cur3, "firm", "prob");
WRITES(cur2, "a_im", VLS(cur3,"a",1));
}
else
WRITES(cur2,"a_im",0);
}
}
}
RESULT(1)

//firm level variables

EQUATION("IN") //R&D expenditure in innovation expressed in common currency
RESULT(VL("sales",1)*VL("e",1)*V("v")*V("xi"))

EQUATION("IM") //R&D expenditure in imitation expressed in common currency
RESULT(VL("sales",1)*VL("e",1)*V("v")*(1-V("xi")))

EQUATION("prob_in") //probability to innovate
V("min_max_tot");
//RESULT(1)  //by setting the probability to 1 you remove the first stage of innovation/imitation
RESULT(min(V("theta_max"),1-exp(-V("xi_1")*(V("IN")/(V("sales_min")*V("xi")*V("v"))))))


EQUATION("prob_im") //probability to imitate
V("min_max_tot");
//RESULT(1) //by setting the probability to 1 you remove the first stage of innovation/imitation
RESULT(min(V("theta_max"),1-exp(-V("xi_2")*(V("IM")/(V("sales_min")*(1-V("xi"))*V("v"))))))


EQUATION("a_in") //new coefficient in case of successful innovation (drawn from a beta = composition of gammas)
if(RND<V("prob_in"))
{
v[0]=beta(V("alpha1"),V("beta1"));
v[1]=beta(V("alpha2"),V("beta2"));
v[2]=v[0]/(v[0]+v[1]);
v[3]=V("x")+(V("xhat")-V("x"))*v[2];
v[4]=VL("a",1)*(1+v[3]);
}
else
v[4]=0;
RESULT(v[4])

EQUATION("a") //firm productivty coefficient
V("Imitation");
RESULT(max(max(V("a_im"),V("a_in")),VL("a",1)))

EQUATION("m") //mark up 
RESULT(max(0,VL("m",1)*(1+V("ni")*(VL("f",1)/VL("f",2)-1))))

EQUATION("p") //price
RESULT((1+V("m"))*V("W")/V("a"))

EQUATION("f") //share in the world market (sum across countries holding the sector constant)
v[0]=0;
CYCLE(cur, "comp")
{
v[0]=v[0] + VS(cur, "ff")*(VS(cur->hook->up, "expenditure")*V("dshare")*VS(cur->hook->up, "e"))/(V("Cw")*V("dshare"));
}
RESULT(v[0]) //weighted average
//RESULT(SUM("ff")/V("n_country")) //unweighted average
//RESULT(SUM("ff")) //by removing the denumerator the firm will exit when f is lower than f_min in the domestic market instead of the international one (this should be used only in the case of pure autarky when all the ff in foreign markets are set to 0)

EQUATION ("D") //firm demand devided bewtween internal demand and extenral demand
WRITE("d_int",0);
WRITE("d_est",0);
CYCLE(cur, "comp")
{
if(V("Id_country")==VS(cur->hook->up, "Id_country"))
{
WRITE("d_int", VS(cur->hook->up, "expenditure")*V("dshare")*VS(cur,"ff"));
}
else
{
INCR("d_est", VS(cur->hook->up, "expenditure")*V("dshare")*VS(cur->hook->up, "e")/V("e")*VS(cur,"ff"));
}
}
RESULT((V("d_int")+V("d_est"))/V("p"))

EQUATION("y") //actual production (notice that VL(D,1) is past demand = desired production)
RESULT(max(0,VL("D",1)))

EQUATION("exp") //exported prodution (computed using the share of external dedemand)
V("D");
RESULT(V("p")*V("y")*V("d_est")/(V("D")*V("p")))

EQUATION("int") //production for the domestic market (computed using the share of internal demand)
V("D");
RESULT(V("p")*V("y")*V("d_int")/(V("D")*V("p")))

EQUATION("exc_dem") //excess demand (or production)
RESULT(V("p")*(V("D")-V("y")))

EQUATION("l")  //employment
RESULT(VL("D",1)/V("a"))

EQUATION("sales") //total sales
RESULT(V("p")*min(V("D"),V("y")))

//comp level variables

EQUATION("ff") //computing firms market shares in each market in which they are competing 
V("competitiveness");
if(V("Id_comp")==V("Id_country"))
v[0]=1/V("p");
else
v[0]=1/(V("p")*(1+V("tau"))*V("e")/VS(p->hook->up,"e"));
//v[0]=0; //activate if you want to remove trade across countries (replace the previous line)
v[1]=max(0.00000000000000001,VL("ff",1)*(1+V("chi")*(v[0]/VS(p->hook,"Ehat")-1)));
RESULT(v[1])

//Firms exit the market

EQUATION("exit_entry") //funtion executed at the end of each time step to perform entry and exit
V("Cw");
V("e");
V("Pindex");
V("Y");
V("TB");
V("Cw");
V("Y_r");
V("L");
V("min_max_tot");
V("Expsh");
CYCLE(cur,"sector")
{
CYCLES(cur, cur1, "firm")
{
if(VS(cur1,"f")<V("f_min"))
{
v[0]=beta(V("alpha1"),V("beta1"));         
v[1]=beta(V("alpha2"),V("beta2"));
v[2]=v[0]/(v[0]+v[1]);
v[3]=V("x_ent")+(V("xhat_ent")-V("x_ent"))*v[2];
WRITELS(cur1,"a",VS(cur,"AvgProd_s")*(1+v[3]),t);//entrants take the countryXsector average productivty plus a shock
WRITELS(cur1,"m",VS(cur,"m_min_int"),t);   //entrants take the countryXsector minimum mark up
WRITELLS(cur1,"f",V("f_min"),t,0);
WRITELLS(cur1,"f",V("f_min"),t,1);
WRITELS(cur1,"y",0,t); //production of entrants set to zero. Notice that total production is computed before entry and exit (in order to reflect the actual value of production)
WRITELS(cur1,"p",(1+VS(cur,"m_min_int"))*V("W")/VS(cur1,"a"),t);
WRITELS(cur1,"D",V("dshare")*V("Cw")*V("f_min")/(VS(cur1,"p")*V("e")),t); //initial level of demand is computed using the minimum market share
//WRITELS(cur1,"D",V("dshare")*V("expenditure")*V("f_min")/(VS(cur1,"p")),t); //use in the case of pure autarky (replace previous line)
WRITELS(cur1,"l",0,t); //employment of entrants is set to zero and is computed in the next step. Notice that total employment in each time step is computed before such adjustment (in order to reflect the actual level of employment)
WRITELS(cur1,"sales",VS(cur1,"p")*VS(cur1,"D"),t); 
INCRS(cur1,"Id",1);
CYCLES(cur1, cur2, "comp")
{
WRITELS(cur2, "ff", V("f_min"),t);
/*if(VS(cur2,"Id_comp")!=V("Id_country")) //activate in case of pure autarky
WRITELS(cur2, "ff", 0,t);
else
WRITELS(cur2,"ff",V("f_min"),t);*/
}
}
}
}
RESULT(1)


MODELEND




void close_sim(void)
{

}


