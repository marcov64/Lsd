#include "fun_head.h"

/***********************
nw2004 - Revisiting the Nelson-Winter model of industrial dynamics

Esben Sloth Andersen, esa@business.auc.dk
Version: 23 Jan 2004

This file contains the Lsd macro code for exploring the Nelson-Winter
model of Schumpeterian competition (Nelson and Winter, 1982, ch. 12).
It builds on programs of Murat Yildizoglu (version: 21 Oct 2001), Marco
Valente, and Toke Reichstein (version: 27 Jan 2003).
************************/

MODELBEGIN

/***********************
Statistics-level parameters
   Stats      - Parameter determining whether advanced
                population is calculated. If 1, the statistics
                is calculated
Industry-level parameters
   Bank       - External financing rate
   Dem_Coeff  - Coefficient of the demand function
   Dem_elast  - Price elasticity of demand
   Dep_rate   - Depreciation rate of physical capital
   Inn_stddev - Standard deviation of innovative draws
   Regime_imi - Imitative regime of the industry
              0: no imitation
              1: imitation is based on the industry's best productivity
              2: imitation is based on the industry's mean productivity
   Regime_inno - Innovative regime of the industry
              0: no innovation
              1: innovation is based on the firm's present productivity
              2: innovation is based on the industry's mean productivity
              3: innovation is based on the growing sciencefic knowledge
   Regime_restraint - Monopolistic behaviour in the industry
              0: no monopolistic behaviour
              1: monopolistic restraint due to mark-up pricing
   Regime_fission - Splitting of large firms
              0: no fission of large firms
              1: fission of large firms
   Fiss       - Determines the probability of fission, since
                fiss * ms is lambda in a Poisson distribution
                that determines whether fission takes place
Firm-level parameters
   RIM - Imitative R&D per unit of capital
   RIN - Innovative R&D per unit of capital
   Inn - Boolean (0 or 1) parameter; 1 if the firm is an innovator.
             0 if not an innovator
   C   - Production cost per unit of capital
   AN  - Productivity of Innovative R&D Investment
   AM  - Productivity of Imitative R&D Investment
************************/

/*************************
Root-level equations
**************************/

EQUATION("Startup")
/*
The equation is only run once. It secures that parameters are set
consistently.
*/
if (t==1 && V("Regime_imi")==0)
{
   CYCLE(cur, "Firm")
   {
    WRITES(cur,"RIM",0);
   }
}
if (t==1 && V("Regime_inno")==0)
{
   CYCLE(cur, "Firm")
   {
    WRITES(cur,"RIM",0);
   }
}
if (t==1 && V("Regime_inno")!=0)
{
   CYCLE(cur, "Firm")
   {
     if (VS(cur,"Inn")==0)
      {
       WRITES(cur,"RIN",0);
      }
   }
}
RESULT(1)

EQUATION("T")
/*
Saves the time step for plotting purposes. To make Gnuplot time series,
place T in the first line of Selected Series in the Data Analysis
window. Add the series to plot, check XYplot, and click Plot.
*/
v[0] = t;
RESULT(v[0])

/*************************
Statistics-level equations
**************************/

EQUATION("InvHerf")
/*
Inverse Herfindal index, 1/SUM(ms^2) 
   The Herfindal is an indicator of the concentration of the market, 
computed as the square of market shares. It varies from 1 (one ms=1 
and all the other equal 0) to 1/n (all firms with identical shares).
   Its inverse corresponds to the number of firms with the same 
dimension that would provide the same concentration index as the one 
measured. It varies between 1 (total concentration) and n (minimal 
concentration).
*/
v[0]=0;
CYCLE(cur,"Firm")
 {
  v[1]=VS(cur,"ms"); //compute the market share
  v[0]=v[0]+v[1]*v[1]; //sum in v[0] the square of ms
 }
RESULT(1/v[0])

EQUATION("HP_index")
/*
The Hymer & Pashigian Instability Index
   Sum of the absolute changes in firm market shares. 
See Hymer and Pashigian, The Review of Economics and 
Statistics, Vol. 44 (1962).
*/
v[0]=SUM("ch_ms");
RESULT(v[0])

EQUATION("A_Mean")
/*
E(A) = SUM[s[t-1] * A[t-1]]
   Weighted mean of the firms' productivities. In contrast to A_mean,
where we weight by market shares (ms), we weight by capital shares (s)
in A_Mean.
*/
v[0]=0;
if (V("Stats")==1)
{
 CYCLE(cur, "Firm")
 {
  v[1] = VLS(cur,"s",1);
  v[2] = VLS(cur,"A",1);
  v[0] = v[0] + v[1]*v[2];
 }
}
else
  v[0] = 0;
RESULT(v[0])

EQUATION("A_Mean_Delta")
/*
Change in the weighted mean of the firms' productivities.
*/
if (V("Stats")==1)
   v[0] = V("A_Mean")-VL("A_Mean",1);
else
   v[0] = 0;
RESULT(v[0])

EQUATION("A_Var")
/*
Var(A) = SUM[ s[t-1]*(A[t-1]-A_mean[t-1])^2) ]
   Weighted variance of the firms' productivities.
*/
v[0]=0;
if (V("Stats")==1)
{
 CYCLE(cur, "Firm")
 {
  v[1] = VLS(cur,"s",1);
  v[2] = VLS(cur,"A",1);
  v[3] = VS(cur,"A_Mean");
  v[0] = v[0] + v[1]*(v[2]-v[3])*(v[2]-v[3]);
 }
}
else
	v[0] = 0;
RESULT(v[0])

EQUATION("Krepro_Mean")
/*
E(Krepro) = SUM[ s[t-1]*Krepro[t] ]
   Weighted mean of the firms' reproduction coefficients.
*/
v[0]=0;
if (V("Stats")==1)
{
 CYCLE(cur, "Firm")
 {
  v[1] = VLS(cur,"s",1);
  v[2] = VS(cur,"Krepro");
  v[0] = v[0] + v[1]*v[2];
 }
}
else
	v[0] = 0;
RESULT(v[0])

EQUATION("Covar_Krepro_A")
/*
Cov(Krepro,A) = 
  SUM[ s[t-1]*(Krepro[t-1]-Krepro_Mean[t-1])*(A[t-1]-A_mean[t-1])) ]
Covariance between firms' reproduction coefficients and their 
productivities.
*/
v[0]=0;
v[3] = V("Krepro_Mean");
v[5] = V("A_Mean");
if (V("Stats")==1)
{
 CYCLE(cur, "Firm")
 {
  v[1] = VLS(cur,"s",1);
  v[2] = VS(cur,"Krepro");
  v[4] = VLS(cur,"A",1);
  v[0] = v[0] + v[1]*(v[2]-v[3])*(v[4]-v[5]);
 }
}
else
	v[0] = 0;
RESULT(v[0])

EQUATION("Regres_Krepro_A")
/*
Regress(Krepro,A) = Regress(Krepro,A)/Var(A)
   Regression coefficient of firms' reproduction coefficients on their 
productivities.
*/
if (V("Stats")==1)
{
   if (V("A_Var")==0)
      v[0] = 0;
   else 
      v[0] = V("Covar_Krepro_A")/V("A_Var");
}
else
	v[0] = 0;
RESULT(v[0])

EQUATION("Innovation_effect")
/*
E(s[t]*(A[t]-A[t-1])) / Krepro_mean
   The innovation effect as defined by George Price's equation.
*/
v[0]=0;
if (V("Stats")==1)
{
 CYCLE(cur, "Firm")
 {
  v[1] = VS(cur,"s");
  v[2] = VS(cur,"A");
  v[3] = VLS(cur,"A",1);
  v[0] = v[0] + v[1]*(v[2]-v[3]);
 }
}
else
	v[0] = 0;
RESULT(v[0])

EQUATION("Selection_effect")
/*
Selection effect = A_Mean_Delta - Innovation effect
   Calculation according to Price's equation.
*/
v[1] = V("Covar_Krepro_A");
v[2] = V("Krepro_Mean");
if (V("Stats")==1)
   v[0] = v[1]/v[2];
else
	v[0] = 0;
RESULT(v[0])

/***********************
Industry-level equations
************************/

EQUATION("Supply")
/*
Supply(t) = Sum[Q(t)]
   The total quantity is the sum of all the firms' quantities.
*/
RESULT(SUM("Q"))

EQUATION("Price")
/*
Price(t) = Dem_Coeff / (Supply(0)^Dem_elast)
   For the original model of Nelson and Winter, take Dem_Coeff=67 
and Dem_elast=1
*/
RESULT(V("Dem_Coeff")/(pow(V("Supply"),V("Dem_elast"))))

EQUATION("A_Max")
/*
Max_Prod(t) = max[A(t-1)] for all firms
   Max_Prod is the maximum productivity among all the existing firms
in the beginning of the period
*/
RESULT(MAXL("A",1))

EQUATION("A_average")
/*
Average productivity 
   The function STAT("Var") returns several statistics for variable Var
and place them in different the elements of vector v starting from v[0]
to v[4]. The values used in stat are referred to the present time, 
with no lag.
*/
SUM("PROF"); //ensure new capital are updated for all firms
STAT("A"); 
RESULT(v[1])

EQUATION("A_mean")
/*
E(A) = SUM[ms[t-1] * A[t-1]]
   Weighted mean of the firms' productivities. In Andersen's note on 
NW experiments in Lsd, it is emphasised how to use A_mean instead of 
A_average to calculate the innovative result (A_IN).
*/
v[0]=0;
CYCLE(cur, "Firm")
 {
  v[1] = VLS(cur,"ms",1);
  v[2] = VLS(cur,"A",1);
  v[0] = v[0] + v[1]*v[2];
 }
RESULT(v[0])

EQUATION("A_science")
/*
Ascience[t] = (1 + g)*Ascience[t-1]
   Mean of science-based innovative results, i.e. scientific knowledge 
grows exponentially.
*/
RESULT((1+V("A_science_grow"))*VL("A_science",1))


EQUATION("K_total")
/*
K_total(t) = Sum[K(t)]
   The total capital is the sum of all the firms' capitals.
*/
RESULT(SUM("K"))

/*******************
Firm-level equations
********************/

EQUATION("Q")
/*
Q(t) = K(t-1) * A(t-1)
   Quantity is is computed as Capital times the productivity,
both with lagged values
*/
RESULT(VL("K",1)*VL("A",1))

EQUATION("PROF")
/*
PROF(t) = P(t) * A(t-1) -C -RIM - RIN*Inn
   Profits per unit of capital are equal current price times lagged
productivity minus the cost for research (innovative firms spend for
both type of research) and fixed costs.
*/
RESULT(V("Price")*VL("A",1) - V("C") - V("RIM") - V("RIN")*V("Inn"))

EQUATION("A")
/*
A(t) = max(A(t-1),A_IM(t),A_IN(t)
   The new productivity is the maximum among the previous one and the
ones possibly obtained imitation and innovation. The latters are 
assigned the value 0 if the research fails to provide a new 
productivity (see their equation).
*/
RESULT( max(VL("A",1), max(V("A_IN"), V("A_IM"))) )

EQUATION("A_IM")
/*
if {RANDOM[K(0)*RIM*1.25]} then A_IM(t) = A_Max(t)
else A_IM(t) = 0
   The probability of imitating is directly proportional to the
amount of expensese in imitation, times a constant factor. These
expenses are equal to a constant amount of expenses per unit of capital
(RIM) times the current capital (K).
   If the research succedes, then the productivity assumes the value
of the best productivity among all the firms, in the previous period.
If the research does not succed, the returned value is 0. It is also 0
if Regime_imi = 0.
*/
v[0] = VL("K",1)*V("RIM")*V("AM");
v[1] = V("Regime_imi");
v[2] = 0;
if (RND<v[0])
  {
   if (v[1]==0) v[1]=0; //no imitation
   if (v[1]==1) v[1]=VL("A_Max",1); //imitation of best practice
   if (v[1]==2) v[1]=VL("A_mean",1); //imitation of mean practice
  }
else
  v[0]=0;
RESULT(v[0])

EQUATION("Inn_mean")
/*
The mean of the normal distribution that determines the outcome of an
innovation. It depends on the "technological regime" of the industry.
If Regime_inno = 0, then the mean is zero. If the regime is 1, 
the mean is A (the firm's present productivity). If the regime is 2, the
mean is A_mean (the industry's mean productivity). If the regime is 3, 
the mean is drawn from an exponentially growing scientific frontier.
*/
v[0] = V("Regime_inno");
v[1] = 0;
if (v[0]==0) v[1]=0; //no innovation
if (v[0]==1) v[1]=VL("A",1); //cumulative at firm level
if (v[0]==2) v[1]=VL("A_Mean",1); //industry cumulation
if (v[0]==3) v[1]=VL("A_science",0); //science-based innovation
RESULT(v[1])

EQUATION("A_IN")
/*
if {RANDOM[K(0)*RIN*AN] AND Inn==1}
   then A_IN(t) = Normal[Inn_mean,Ind_stddev)]
else A_IN(t) = 0
   The probability of innovating is directly proportional to the
amount of expenses in innovation, times a constant factor.
   These expenses are equal a constant amount of expenses per unit of 
capital (RIN) times the current capital (K). Only firms with the 
parameter Inn equals 1 can actually innovate.
   If the research succedes, then the productivity assumes the a random
value draw from a normal function, around the mean productivity of 
the industry in the beginning of the period and with a standard 
deviation given by Std_Prod. If the research does not succeed the 
returned value is 0.
*/
v[0] = VL("K",1)*V("RIN")*V("AN");
v[1] = RND;
v[3] = V("Inn_mean");
v[4] = V("Inn_stddev");
if (V("Inn") == 0)
   v[2]=0;
if( V("Inn")==1 && v[1]<v[0])
   v[2]= norm(v[3],v[4]); 
         // exp(norm(log(v[3]),v[4])); 
else
   v[2]=0;
RESULT(v[2])

EQUATION("MU_target")
/*
MU_target = Dem_elast/(Dem_elast - ms)
   If Regime_restraint = 1, then the firm's target markup is determined
by the firm's market share and the market's demand elasticity. 
If Regime_restraint = 0, then there is no monopolistic behaviour.
*/
if (V("Regime_restraint") == 1)
  v[0] = V("Dem_elast")/(V("Dem_elast")-V("ms"));
else
  v[0] = 1; //no monopolistic behaviour
RESULT(v[0])

EQUATION("MU_actual")
/*
MU_actual = Price * productivity / costs per capital unit
   The firm's actual markup is determined by the firm's productivity,
the market price, and the constant costs per unit of capital.
*/
RESULT(V("Price")*(V("A")/V("C")))

EQUATION("I_rate")
/*
Investment Rate
   The rate is formed by taking the minimum between a desired 
level and a performance constrained level. If both are 
negative the investment rate is calculated as being zero.
The Investment rate is used in the capital function. 
*/
if(V("PROF")<=0)
   v[1]=V("PROF"); //no external finance
else
   v[1]=V("PROF")*(1+V("Bank")); //external finance included
v[0] = V("Regime_restraint");
if (v[0]==0 || v[0]==1)
{
  v[2]=1+V("Dep_rate")-V("MU_target")/V("MU_actual"); //desired inv. rate
  v[3]=max(0,min(v[1],v[2])); //final investment rate
}
else
  v[3]=V("PROF");
RESULT(v[3])

EQUATION("K")
/*
K(t) = K(t-1) * (1 - Dep_rate + Final Investment rate)
   The capital is reduced by depreciation and increased by investments.
Investmens are aimed at reaching a desired expansion or contraction
of productive capacity K, as a function of unit price cost and market 
shares. But there are also financial contraints that can limit the 
possibility to obtain the desired expansion.
*/
if (V("Regime_restraint")!=2)
  v[0]=VL("K",1)*(1-V("Dep_rate")+V("I_rate"));
else
  v[0]=VL("K",1)*(1+V("I_rate"));
RESULT(v[0])

EQUATION("s")
/*
Capital shares, computed as the ratio of K[i] and K_total
*/
RESULT(V("K")/V("K_total"))

EQUATION("ms")
/*
Market shares, computed as the ratio of Q and Supply.
*/
RESULT(V("Q")/V("Supply"))

EQUATION("ch_ms")
/*
Changes in market shares in absolute values. 
Used in the Hymer-Pashigian Instability Index, see HP_index equation.
*/
v[0]=V("ms")-VL("ms",1);
RESULT(abs(v[0]))

EQUATION("Krepro")
/*
Krepro = K[t]/K[t-1]
The reproduction coefficient of capital.
Used in the calculation of population statistics.
*/
RESULT(V("K")/VL("K",1))

EQUATION("Fission")
/*
Fissions of firms are determined after the new productivity is found. The probability of a fission depends on the parameter psi times the market share of the firm (as a Poisson process). If a fission takes place, the capital of the firm is split according to a uniform distribution, and the smallest is set up as a new firm with the same productivity as the mother firm.
*/
v[0] = V("K"); //ensures update
v[1] = V("A"); //ensures update
v[2] = V("Fiss");
v[3] = V("ms");
v[4]=RND;
v[5]=0.49*v[4]*v[0];
v[6]=(1 - 0.49*v[4])*v[0];
v[7] = V("Regime_fission");
if (poisson(v[2]*v[3])>0 && v[7]==1)
  {
  cur=p->up; 
  //cur=cur->add_an_object("Firm",p);
  cur=ADDOBJS_EX(cur,"Firm",p);
  cur->write("K",v[5],t);
  cur->write("ms",v[3]*0.49*v[4],t);
  p->write("K",v[6],t);
  p->write("ms",v[3]*(1 - 0.49*v[4]),t);
  }
RESULT(v[6])


MODELEND

//function used after the end of a simulation run, sometime used to
//remove C++ elements created outside the standard Lsd functions
void close_sim(void)
{
}
