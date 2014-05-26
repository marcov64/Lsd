
/***************************************************
****************************************************
LSD 2.0 - April 2000
written by Marco Valente
Aalborg University

Example for an equation file. Users should include in this header a
brief description of the model.

Include the equations in the space indicated below, after the line:

Place here your equations


****************************************************
****************************************************/

#include "../src/fun_head.h"
int deb(object *r, object *c, char *lab, double *res);
double  S(double x, double maxX, double maxY);
int roundInt(double a);
extern int max_step;
object *demand;
#define BFIRM 1
#define INDIV 2

#define TR 1
#define MP 2

#include <math.h>
#include <string.h>
#include <stdio.h>



double variable::fun(object *caller)
{
//These are the local variables used by default

double v[100];
double res;
object *p, *c, *cur1, *cur2, *cur3, *cur4, *cur5;
variable *cur_v;



//Declare here any other local variable to be used in your equations
//You may need an integer to be used as a counter
int i, j;
//and an object (a pointer to)
register object *cur;


if(quit==2)
 return -1;

p=up;
c=caller;
FILE *f;


//Uncommenting the following lines the file "log.log" will
//contain the name of the variable just computed.
//To be used in case of unexpected crashes. It slows down sensibly the simulation
/**
f=fopen("log.log","a");
 fprintf(f,"t=%d %s\n",t,label);
 fclose(f);
**/

//Place here your equations. They must be blocks of the following type



if(!strcmp(label,"maxEbwExpenditure"))
{
/*
max useful level of ebw stock expenditure. Auto value is maxInitBud/10
*/
v[0]=p->cal("maxInitBud",0);
res=v[0]/10;
goto end;
}


if(!strcmp(label,"main"))
{
/*
Main cycle calling the different function in the desired order. Since this equation is placed in an Object on the top of the hierarchy it is computed before everything else at each time step.
*/

p->cal("secondGenerationCreation",0);
p->cal("diversification",0);
p->cal("RDInvest",0);
p->cal("ebwInvest",0);
p->cal("adoption",0);
p->cal("innovation",0);
p->cal("checkEntry",0);
p->cal("marketBfirm",0);
p->cal("marketIndiv",0);
p->cal("bookKeeping",0);

res=1;
goto end;
}


if(!strcmp(label,"bookKeeping"))
{
/*
Update the financial values and other statistics
*/
cur1=demand->search_var_cond("IdUserClass",BFIRM, 0);
cur2=demand->search_var_cond("IdUserClass",INDIV, 0);
cur1->write("UCProfitRate",0, 0);
cur2->write("UCProfitRate",0, 0);


v[1]=p->cal("projectDuration",0);
v[4]=p->cal("fractionOfProfitForDebtPayBack",0);
v[7]=p->cal("r",0);
v[13]=p->cal("wheightOfPast",0);
v[21]=p->cal("slopeOfFailure",0);

for(v[17]=v[18]=0,cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[0]=cur->cal("debt",0);
  v[3]=cur->cal("birth",0);
  if(v[0]>0)
   {
    v[2]=cur->cal("profit",0);
    if(v[2]>0 && ((double)t-v[3])>v[1])
     {
      v[6]=cur->increment("debt",-1*v[2]*v[4]);
      v[5]=cur->increment("bud",-1*v[2]*v[4]);
      if(v[6]<0)
        {
         cur->increment("bud",-1*v[6]);
         cur->write("debt",0, 0);
         
        }
     }
    v[0]=cur->multiply("debt",(1+v[7])); 
   }
  v[9]=cur->multiply("bud",(1+v[7]));   
  v[10]=cur->cal("profitRate",0);
  cur->write("oldProfitRate",v[10], 0);
  v[11]=cur->cal("initBud",0);
  v[12]=(v[9]-v[0])/(v[11]*pow(1+v[7],(double)t-v[3]));
  cur->write("profitRate",v[12], 0);
  v[14]=cur->cal("slopeOfProfitRate",0);
  v[15]=v[14]*((v[13]-v[1])/v[1])+(v[12]-v[14])/v[13];
  cur->write("slopeOfProfitRate",v[14], 0);
  v[16]=cur->cal("servedUserClass",0);
  if(v[16]==BFIRM)
   {
    v[17]++; //NumA
    cur1->increment("UCProfitRate",v[12]);
    if(v[17]==1)
      v[19]=v[12];
    else
      if(v[19]<v[12])
        v[19]=v[12]; //maxProfitRate  
    
   }
  if(v[16]==INDIV)
   {
    v[18]++; //NumB
    cur2->increment("UCProfitRate",v[12]);
    if(v[18]==1)
      v[20]=v[12];
    else
      if(v[20]<v[12])
        v[20]=v[12]; //maxProfitRate  
    
   }
   
   v[22]=cur->cal("alive",0);
   v[23]=cur->cal("entered",0);
   if(v[22]==1 && v[23]==1 && v[12]<0 && v[14]<v[21])
    cur->cal("failFirm",0);
 }

cur1->write("maxProfitRate",v[19], 0);
cur2->write("maxProfitRate",v[20], 0);

if(v[17]>0)
 cur1->multiply("UCProfitRate",1/v[17]);
if(v[18]>0)
 cur1->multiply("UCProfitRate",1/v[18]);
 
res=1;
goto end;
}


if(!strcmp(label,"marketIndiv"))
{
/*
manages the market functioning for indivual users (PC)
*/

cur1=demand->search_var_cond("IdUserClass",INDIV, 0);

cur1->write("techlevelCheap",0, 0);
cur1->write("techlevelPerf",0, 0);
cur1->write("DIVshare",0, 0);
cur1->write("MPshare",0, 0);
cur1->write("quantityPurchased",0, 0);
cur1->write("herfindal",0, 0);
cur1->write("numOfFirstGenerationSellingFirms",0, 0);
cur1->write("numOfSecondGenerationSellingFirms",0, 0);
cur1->write("numOfDaughterSellingFirms",0, 0); 
cur1->write("generationOfLeader",0, 0);
cur1->write("shareOfSecondGenerationLeader",0, 0);
cur1->write("shareOfDaughterLeader",0, 0);
cur1->write("profitRateOfSecondGenerationLeader",0, 0);
cur1->write("profitRateOfDaughterLeader",0, 0);
cur1->write("distanceOfSecondGenerationLeader",0, 0);
cur1->write("distanceOfDaughterLeader",0, 0);


v[45]=cur1->cal("whenMktOpen",0);
if(v[45]>0)
 {
  v[46]=cur1->cal("maxAlfaDesign",0);
  v[47]=cur1->cal("startAlfaDesign",0);
  v[48]=cur1->cal("startAlfaDesign",0);
  v[49]=(v[46]- v[48])/((double)max_step-v[45]); 
  v[5]=cur1->increment("alfaDesign",v[49]);
 }
else
 v[5]=cur1->cal("alfaDesign",0); 
v[19]=cur1->cal("brandLoyalty",0);
v[24]=cur1->cal("numOfSubMarkets",0);

v[56]=p->cal("maxEbwExpenditure",0);
v[7]=p->cal("maxEbwEffect",0);
v[10]=p->cal("bw",0);
v[11]=p->cal("errorDemand",0);
v[17]=p->cal("breakdown",0);
v[41]=p->cal("fullVisible",0);

for(v[21]=0,v[14]=0,v[2]=0, cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[0]=cur->cal("alive",0);
  v[1]=cur->cal("servedMkt",0);
  if(v[0]==1 && v[1]==2)
   {
   v[2]++; //numOfSellingFirms
   v[3]=cur->cal("generation",0);
   if(v[3]==1)
     cur1->increment("numOfFirstGenerationSellingFirms",1);
    if(v[3]==2)
      cur1->increment("numOfSecondGenerationSellingFirms",1);
    if(v[3]==3)
      cur1->increment("numOfDaughterSellingFirms",1);  
      
    cur->write("numberOfNewSubMarkets",0, 0);  
    v[4]=cur1->cal(cur,"utility",0);
    cur->write("u",v[4], 0);
    v[8]=cur->cal("share",0);
    v[9]=cur->cal("ebw",0);
    v[12]= pow(v[4],v[5]+1)*pow(1+v[8],v[10]-0.9)*pow(1+S(v[9],v[56],v[7]),v[7]+1); //!?! different from BFIRM case
    v[13]=v[12]*(1-v[11]+2*RND*v[11]);
    cur->write("share",v[13], 0);
    v[14]+=v[13]; //cumulatedProb
    v[15]=cur->cal("numberOfServedSubMarkets",0);
    v[20]=cur->cal("numberOfBLReturns",0);
    if(v[15]>0)
      {
       v[16]=cur->cal("averageAgeOfSubMarkets",0);
       v[18]=round(v[15]*v[16]/v[17]);
       cur->write("numberOfBreakdowns",v[18], 0);
       v[20]=round(v[18]*v[19]);
       cur->write("numberOfBLReturns",v[20], 0);
       v[15]=cur->increment("numberOfServedSubMarkets",-1*v[18]);
       cur->write("numberOfNewSubMarkets",v[20], 0); //!?! see below
      }
    v[21]+=v[15]+v[20]; //busySubMarkets)
    } //if firm is alive and serves the market
   } //end of cycle for through Firm's

   if(v[2]<=v[41]) 
     v[23]= round( (v[24]-v[21])*v[2]/v[41]); //numberOfPurchasingSubMarkets
   else
     v[23]= round( v[24]-v[21]);
     
   //second cycle  
   for(v[40]=0,v[30]=v[29]=v[27]=v[25]=0,cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
    {
     v[0]=cur->cal("alive",0);
     v[1]=cur->cal("servedMkt",0);
     if(v[0]==1 && v[1]==2)
      {
       v[26]=cur->cal("generation",0);
       v[24]=cur->multiply("share",1/v[14]);  
       if(v[25]<v[24])
        {//share of leader
         v[25]=v[24];
         cur1->write("generationOfLeader",v[26], 0);
        } 
       if(v[26]==2 && v[27]<v[24]) //leader share of second generation firms
        {
         v[27]=v[24];
         v[28]=cur->cal("profitRate",0);
         cur1->write("shareOfSecondGenerationLeader",v[24], 0);
         cur1->write("profitRateOfSecondGenerationLeader",v[28], 0);
         cur1->write("distanceOfSecondGenerationLeader",cur->cal("distanceFromCorner",0), 0);
         } 
       if(v[26]==3 && v[40]<v[24])
        {
         v[40]=v[24];
         v[28]=cur->cal("profitRate",0);         
         cur1->write("shareOfDaughterLeader",v[40], 0);
         cur1->write("profitRateOfDaughterLeader",v[28], 0);
         cur1->write("distanceOfDaughterLeader",cur->cal("distanceFromCorner",0), 0);
        }  
       
       v[29]+=v[24]*v[24];  //herfindal
       v[42]=cur->increment("numberOfNewSubMarkets",roundInt(v[24]*v[23]));
       cur1->increment("quantityPurchased",v[42]*cur->cal("u",0));
       v[31]=cur->cal("averageAgeOfSubMarkets",0);
       v[22]=cur->cal("numberOfServedSubMarkets",0);
       if(v[22]+v[42]>0)
         v[32]=1+v[31]*v[22]/(v[22]+v[42]);
       else
         v[32]=1;  
       cur->write("averageAgeOfSubMarkets",v[32], 0);
       v[33]=cur->increment("numberOfServedSubMarkets",v[42]);
       v[34]=cur->cal("cheap",0);
       v[35]=cur->cal("mup",0);
       v[36]=cur->cal("u",0);
       v[37]=(1/v[34])*v[35]*v[36]*v[42];
       cur->write("profit",v[37], 0);
       cur->increment("bud",v[37]);
       v[38]=cur->cal("perf",0);
       cur1->increment("techlevelCheap",v[34]*v[24]);
       cur1->increment("techlevelPerf",v[38]*v[24]);
       
       v[39]=cur->cal("generation",0);
       if(v[39]==2)
         cur1->increment("MPshare",v[24]);  
       v[43]=cur->cal("diversified",0);
       if(v[43]==1)
         cur1->increment("DIVshare",v[24]);  

        
      }
      
    }    
cur1->write("herfindal",v[29], 0);

res=1;
goto end;

res=1;
goto end;
}


if(!strcmp(label,"distanceFromCorner"))
{
/*
returns (1 - the relative distance from the corner) with respect to the diagonal
*/

v[6]=p->cal("IdTecUsed",0);
cur=p->search_var_cond("tecLabel",v[6], 0);
v[0]=cur->cal("cheapLim",0);
v[1]=p->cal("cheap",0);
v[2]=cur->cal("perfLim",0);
v[3]=p->cal("perf",0);
v[4]=cur->cal("diagonal",0);

res=sqrt((v[0]-v[1])*(v[0]-v[1])+(v[2]-v[3])*(v[2]-v[3]))/v[4];
goto end;
}




if(!strcmp(label,"marketBfirm"))
{
/*
Manages the market functioning for big firms (mainframes)
*/
cur1=demand->search_var_cond("IdUserClass",BFIRM, 0);

cur1->write("techlevelCheap",0, 0);
cur1->write("techlevelPerf",0, 0);
cur1->write("TRshare",0, 0);
cur1->write("MPshare",0, 0);
cur1->write("quantityPurchased",0, 0);
cur1->write("herfindal",0, 0);
cur1->write("numOfFirstGenerationSellingFirms",0, 0);
cur1->write("numOfFirstGenerationDiversifiedSellingFirms",0, 0);
cur1->write("numOfSecondGenerationSellingFirms",0, 0);
cur1->write("numOfDaughterSellingFirms",0, 0); 
cur1->write("generationOfLeader",0, 0);
cur1->write("shareOfFirstGenerationLeader",0, 0);
cur1->write("profitRateOfFirstGenerationLeader",0, 0);
cur1->write("distanceOfFirstGenerationLeader",0, 0);


v[45]=cur1->cal("whenMktOpen",0);
if(v[45]>0)
 {
  v[46]=cur1->cal("maxAlfaDesign",0);
  v[47]=cur1->cal("startAlfaDesign",0);
  v[48]=cur1->cal("startAlfaDesign",0);
  v[49]=(v[46]-v[48])/((double)max_step-v[45]); 
  v[5]=cur1->increment("alfaDesign",v[49]);
 }
else
 v[5]=cur1->cal("alfaDesign",0); 

v[19]=cur1->cal("brandLoyalty",0);
v[44]=cur1->cal("numOfSubMarkets",0);

v[56]=p->cal("maxEbwExpenditure",0);
v[7]=p->cal("maxEbwEffect",0);
v[10]=p->cal("bw",0);
v[11]=p->cal("errorDemand",0);
v[17]=p->cal("breakdown",0);
v[41]=p->cal("fullVisible",0);

v[21]=0;
for(v[14]=0,v[2]=0, cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[0]=cur->cal("alive",0);
  v[1]=cur->cal("servedMkt",0);
  if(v[0]==1 && v[1]==1)
   {
   v[2]++; //numOfSellingFirms
   v[3]=cur->cal("generation",0);
   if(v[3]==1)
    {
     cur1->increment("numOfFirstGenerationSellingFirms",1);
     v[6]=cur->cal("adopted",0);
     if(v[6]==1)
      cur1->increment("numOfFirstGenerationDiversifiedSellingFirms",1);
    }
    if(v[3]==2)
      cur1->increment("numOfSecondGenerationSellingFirms",1);
    if(v[3]==3)
      cur1->increment("numOfDaughterSellingFirms",1);  
      
    cur->write("numberOfNewSubMarkets",0, 0);  
    v[4]=cur1->cal(cur,"utility",0);
    cur->write("u",v[4], 0);
    v[8]=cur->cal("share",0);
    v[9]=cur->cal("ebw",0);
    v[12]= pow(v[4],v[5]);
    v[12]*=pow(1+v[8],v[10]);
    v[12]*=pow(1+S(v[9],v[56],v[7]),v[7]);
    v[13]=v[12]*(1-v[11]+2*RND*v[11]);
    cur->write("share",v[13], 0);
    v[14]+=v[13]; //cumulatedProb
    v[15]=cur->cal("numberOfServedSubMarkets",0);
    v[20]=cur->cal("numberOfBLReturns",0);
    if(v[15]>0)
      {
       v[16]=cur->cal("averageAgeOfSubMarkets",0);
       v[18]=round(v[15]*v[16]/v[17]);
       cur->write("numberOfBreakdowns",v[18], 0);
       v[20]=round(v[18]*v[19]);
       cur->write("numberOfBLReturns",v[20], 0);
       v[15]=cur->increment("numberOfServedSubMarkets",-1*v[18]);
       cur->write("numberOfNewSubMarkets",v[20], 0); //!?! see below
      }
    v[21]+=v[20]+v[15]; //busySubMarkets)
    } //if firm is alive and serves the market
   } //end of cycle for through Firm's

   if(v[2]>v[41]) 
     v[23]= round( (v[44]-v[21])*v[2]/v[41]); //numberOfPurchasingSubMarkets
   else
     v[23]= round( v[44]-v[21]);
     
   //second cycle  
   for(v[30]=v[29]=v[27]=v[25]=0,cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
    {
     v[0]=cur->cal("alive",0);
     v[1]=cur->cal("servedMkt",0);
     if(v[0]==1 && v[1]==1)
      {
       v[26]=cur->cal("generation",0);
       v[24]=cur->multiply("share",1/v[14]);  
       if(v[25]<v[24])
        {//share of leader
         v[25]=v[24];
         cur1->write("generationOfLeader",v[26], 0);
        } 
       if(v[26]==1 && v[27]<v[24]) //leader share of first generation firms
        {
         v[27]=v[24];
         v[28]=cur->cal("profitRate",0);
         cur1->write("shareOfFirstGenerationLeader",v[24], 0);
         cur1->write("profitRateOfFirstGenerationLeader",v[28], 0);
         cur1->write("distanceOfFirstGenerationLeader",cur->cal("distanceFromCorner",0), 0);
         } 
       v[29]+=v[24]*v[24];  //herfindal

       v[42]=cur->increment("numberOfNewSubMarkets",roundInt(v[24]*v[23]));
       cur1->increment("quantityPurchased",v[42]*cur->cal("u",0));

       v[31]=cur->cal("averageAgeOfSubMarkets",0);
       v[22]=cur->cal("numberOfServedSubMarkets",0);
       if(v[22]+v[42]>0)
         v[32]=1+v[31]*v[22]/(v[22]+v[42]);
       else
         v[32]=1;  
       cur->write("averageAgeOfSubMarkets",v[32], 0);
       v[33]=cur->increment("numberOfServedSubMarkets",v[42]);
       v[34]=cur->cal("cheap",0);
       v[35]=cur->cal("mup",0);
       v[36]=cur->cal("u",0);
       v[37]=(1/v[34])*v[35]*v[36]*v[42];
       cur->write("profit",v[37], 0);
       cur->increment("bud",v[37]);
       v[38]=cur->cal("perf",0);
       cur1->increment("techlevelCheap",v[34]*v[24]);
       cur1->increment("techlevelPerf",v[38]*v[24]);
       v[39]=cur->cal("generation",0);
       if(v[39]==1)
         cur1->increment("TRshare",v[24]);  
       if(v[39]==2)
         cur1->increment("MPshare",v[24]);  
      }
      
    }    

cur1->write("herfindal",v[29], 0);

res=1;
goto end;
}





if(!strcmp(label,"utility"))
{
/*
Function contained in UserClass called by Firm. Therefore:
p-> refers to the userclass 
c-> refers to the firm
*/
last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}

v[0]=p->cal("gammaUt",0); //0.08
v[1]=c->cal("cheap",0);
v[2]=c->cal("mup",0);
v[3]=p->cal("cheapThreshold",0);
v[4]=c->cal("perf",0);
v[5]=p->cal("perfThreshold",0);
v[6]=p->cal("lambda",0);
v[7]=p->cal("cheapExp",0);
v[8]=p->cal("perfExp",0);
v[9]=v[1]/(1+v[2]);
if(v[9]<=v[3] || v[4]<= v[5]) 
 res=0;
else
 {
  v[10]=v[0]*pow(v[9]-v[3],v[6]+v[7])*pow(v[4]-v[5],v[6]+v[8]);
  res=v[10];
 } 


//v[9]=501/(1.01)=455.45*pow(239,0.85)*(1)=3825.9527254107266758641098038425



goto end;
}


if(!strcmp(label,"checkEntry"))
{
/*
Uhu? Strangely enough, a firm entering BFIRM does not have any chance to enter subsequently
in INDIV, at least using this function. In fact, when "entered" is set to TRUE the firm skips this
code.
Probably there is another function (diversification?) allowing for firms to operate in two markets.

*/
cur1=demand->search_var_cond("IdUserClass",BFIRM, 0);
v[0]=cur1->cal("whenMktOpen",0);

cur2=demand->search_var_cond("IdUserClass",INDIV, 0);
v[1]=cur2->cal("whenMktOpen",0);

for(v[10]=0,cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[2]=cur->cal("alive",0);
  v[3]=cur->cal("entered",0);
  if(v[2]==1 && v[3]==0)
   {
    v[4]=cur1->cal(cur,"utility",0);
    if(v[4]>0)
    {v[10]++;
     cur->write("entered",1, 0);
     v[5]=cur1->cal("whenMktOpen",0);
     if(v[5]==0)
      cur1->write("whenMktOpen",(double)t, 0);
     cur->write("whenEntered",(double)t, 0); 
     cur->write("servedUserClass",BFIRM, 0);
     cur->write("servedMkt",1, 0);
    }
    v[6]=cur2->cal(cur,"utility",0);
    if(v[6]>0)
    {v[10]++;
     cur->write("entered",1, 0);
     v[7]=cur2->cal("whenMktOpen",0);
     if(v[7]==0)
      cur2->write("whenMktOpen",(double)t, 0);
     cur->write("whenEntered",(double)t, 0); 
     cur->write("servedUserClass",INDIV, 0);
     cur->write("servedMkt",2, 0);
    }
   //here should go the standard settings  
    
   }
 }



res=v[10];
goto end;
}



if(!strcmp(label,"innovation"))
{
/*
Implement the innovation, modifying the characteristics of product (cheap and performance) 
*/

res=1;

v[0]=p->cal("gammaDxCheap",0);
v[17]=p->cal("gammaDxPerf",0);
v[1]=p->cal("alfa1Dx",0);
v[2]=p->cal("alfa2Dx",0);
v[3]=p->cal("alfa3Dx",0);
v[4]=p->cal("alfa4Dx",0);

for(cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  if(cur->cal("alive",0)==1)
  {
    cur2=cur->search("Product");
    v[5]=cur->cal("IdTecUsed",0);
    cur1=p->search_var_cond("tecLabel",v[5], 0);
    v[5]=cur1->cal("cheapLim",0);
    v[14]=cur2->cal("cheap",0);
    v[6]=cur->cal("cheapEng",0);
    v[7]=cur->cal("experience",0);
    v[8]=cur->cal("averageExperience",0);
    v[9]=v[0]*pow(v[5]-v[14],v[1])*pow(v[6],v[2])*pow(v[7],v[3])*pow(v[8],v[4]);
    v[10]=cur2->increment("cheap",v[9]);
    if(v[10]>v[5])
     cur2->write("cheap",v[5], 0);
  
    v[11]=cur1->cal("perfLim",0);
    v[15]=cur2->cal("perf",0);
    v[12]=cur->cal("perfEng",0);
    v[13]=v[17]*pow(v[11]-v[15],v[1])*pow(v[12],v[2])*pow(v[7],v[3])*pow(v[8],v[4]);
    v[16]=cur2->increment("perf",v[13]);
    if(v[16]>v[11])
     cur2->write("perf",v[11], 0);
    
    cur->increment("experience",1);
    }  
    
 }

goto end;
}



if(!strcmp(label,"findBestMPDistance"))
{
/*

Best distance in micro-processor technology
*/
v[0]=v[3]=0;
for(cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[1]=cur->cal("alive",0);
  v[2]=cur->cal("IdTecUsed",0);
  if(v[1]==1 && v[2]==MP)
   {
    v[3]=cur->cal("distanceCovered",0);
    if(v[3]>v[0])
      v[0]=v[3];
   
   }
 }

res=v[3];
goto end;
}


if(!strcmp(label,"adoption"))
{
/*
Manages the switch of technology from transistors to micro-processors
*/
v[0]=p->cal("secondGenerationTime",0);
if(v[0]>(double)t)
 {res=-1;
  goto end;
 }
v[1]=p->cal("findBestMPDistance",0);
v[5]=p->cal("adoption1",0);
v[6]=p->cal("adoption2",0);
v[7]=p->cal("difficultyOfAdoption",0);
v[10]=p->cal("percCostOfAdoption",0);
v[11]=p->cal("fixedCostOfAdoption",0);
v[15]=p->cal("Efactor",0);
for(v[18]=0,cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[2]=cur->cal("alive",0);
  v[3]=cur->cal("entered",0);
  v[4]=cur->cal("IdTecUsed",0);
  if(v[2]==1 && v[3]==1 && v[4]==TR)
   {v[9]=cur->cal("distanceCovered",0);
    v[8]=pow(0.5*pow(v[9],v[5]) + 0.5*pow(v[1],v[6]),v[7]);
    v[12]=cur->cal("bud",0);
    v[13]=(v[12]*(1-v[10])-v[11]);
    if(RND<v[8] && v[13]>0)
      {v[18]++;
       cur->write("bud",v[13],0);

       cur->write("IdTecUsed",MP, 0);
       cur->write("adopted",1, 0);
       v[14]=cur->cal("experience",0);
       v[16]=cur->cal("averageExperience",0);
       v[17]=1+roundInt(v[15]*(v[14]-v[16]));
       cur->write("experience",v[17], 0);
       // bonus if average experience is low !!!!
       if(v[17] > v[14])
         cur->write("experience",v[14], 0); //the lower the better ?!?
       cur->write("averageExperience",1, 0);
      }
   }
 }
res=v[18];
goto end;
}


if(!strcmp(label,"ebwInvest"))
{
/*
Equation managing the endogenous band wagon effect (marketing)
*/
v[0]=p->cal("ebwErosion",0);
v[1]=p->cal("ebwExpenditure",0);
for(cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
 v[5]=cur->cal("alive",0);
 if(v[5]==1)
  {
  cur->multiply("ebw",1-v[0]);
  v[2]=cur->cal("profit",0);
  cur->increment("ebw",v[1]*v[2]);
  v[3]=cur->cal("bud",0);
  v[4]=cur->increment("bud",-v[2]*v[1]);
  if(v[4]<=0)
    cur->write("alive",0, 0);
  }  
 }

res=1;
goto end;
}


if(!strcmp(label,"failFirm"))
{
/*
function killing firms 
*/
last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}

v[0]=c->cal("bud",0);
c->increment("debt",-v[0]);
c->write("alive",0, 0);


res=1;
goto end;
}


if(!strcmp(label,"RDInvest"))
{
/*
Manages research team of engineers (setting values in Obejct ResTeam). Calculates avg experience of team if the team grows. Generally innovation expenditures are on profits but if project time has not finished thenexpenditures are on a constant fraction of initial budget (+ profits'contribution if there are).
*/
res=1;
v[0]=p->cal("projectDuration",0);
v[8]=p->cal("engineerCost",0);

for(cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[1]=cur->cal("alive",0);
  if(v[1]==1)
   {
    cur2=cur->search("ResTeam");
    v[19]=cur2->cal("cheapEng",0)+cur2->cal("perfEng",0);
    v[2]=cur->cal("birth",0);
    if((double)t-v[2]<v[0])
     {//  firm hasn't finished its project yet
      v[3]=cur->cal("initBud",0);
      v[4]=cur->cal("profit",0);
      v[5]=cur->cal("RDExpenditures",0);
      v[6]=cur->cal("cheapMix",0);
      v[7]=cur->cal("perfMix",0);
      
      v[9]=round(((v[3]/v[0]+v[4]*v[5])*v[6])/v[8]);

      cur2->write("cheapEng",v[9], 0);
      v[10]=round(((v[3]/v[0]+v[4]*v[5])*v[7])/v[8]);
      cur2->write("perfEng",v[10], 0);
      v[17]=v[3]/v[0]+v[4]*v[5];
      cur->increment("bud",-1*v[17]);

     }
    else
     {// initial project has finished and debt payback is on
      v[3]=cur->cal("cheapMix",0);
      v[12]=cur->cal("perfMix",0);
      v[4]=cur->cal("profit",0);
      v[5]=cur->cal("RDExpenditures",0);
      v[6]=cur2->cal("cheapEng",0);
      v[7]=cur2->cal("perfEng",0);
      
      if(v[4]*v[5]<v[8]*v[19])
       {// profits are too low to mantain current team
        v[10]=round(v[6]*0.9);
        v[11]=round(v[7]*0.9);
        cur2->write("cheapEng",v[10], 0);
        cur2->write("perfEng",v[11], 0);
        cur->increment("bud",round(-0.9*v[19])*v[8]);
       }
      else
       {// profits are enough to mantain current	team at least
        v[10]=round(((v[4]*v[5])*v[3])/v[8]);
        cur2->write("cheapEng",v[10], 0);
        v[11]=round(((v[4]*v[5])*v[12])/v[8]);
        cur2->write("perfEng",v[11], 0);
        
        v[18]=v[4]*v[5];
        cur->increment("bud",-1*v[18]);
       } 
     } 
    v[15]=cur2->cal("cheapEng",0);
    v[16]=cur2->cal("perfEng",0);

    v[17]=cur->cal("bud",0);
    if(v[15]+v[16]<1 || v[17]<=0)
      cur->cal("failFirm",0); 
    cur->increment("averageExperience",1);  
    if(v[15]+v[16]>v[19])
      {
       cur->multiply("averageExperience",v[19]/(v[15]+v[16]));
       cur->increment("averageExperience",(v[15]+v[16]-v[19])/(v[15]+v[16]));
      }  
   }  
 }


goto end;
}



if(!strcmp(label,"diversification"))
{
/*
equation setting allowing for firms to diversify in the new market for PC 
*/
cur=demand->search_var_cond("IdUserClass",INDIV,0);
v[0]=cur->cal("quantityPurchased",0);
if(v[0]<=0)
 {res=-2;
  goto end;
 }
cur=demand->search_var_cond("IdUserClass",BFIRM,0);
v[1]=cur->cal("quantityPurchased",0);
v[2]=p->cal("goDiversification",0);
if(v[1]/v[0]<v[2])
 {res=-1;
  goto end;
 } 

v[10]=p->cal("percCostOfDiversification",0);
v[11]=p->cal("fixedCostOfDiversification",0);


cur3=p->search_var_cond("tecLabel",MP, 0);

v[14]=cur3->cal("cheapLim",0);
v[15]=cur3->cal("perfLim",0);
v[16]=p->cal("initMarkUp",0);
v[18]=p->cal("RDexpendituresOnProfits",0);  
v[19]=p->cal("percEbwDiversification",0);
v[20]=p->cal("Efactor",0);
v[23]=p->cal("MedCheapPCFirm",0);
v[24]=p->cal("MedPerfPCFirm",0);
v[26]=p->cal("NumFirm",0)+1;
v[27]=0;
for(cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[3]=cur->cal("alive",0);
  v[4]=cur->cal("mother",0);  
  v[5]=cur->cal("daughter",0);
  v[6]=cur->cal("servedMkt",0);
  v[7]=cur->cal("IdTecUsed",0);
  v[8]=cur->cal("profitRate",0);
  v[9]=cur->cal("bud",0);
  if(v[3]==1 && v[4]==0 && v[5]==0 && v[6]==1 && v[7]==MP && v[8]>0 && v[9]*(1-v[10])> v[11])
   {v[27]++;
    //cur1=p->add_an_object("Firm",cur);
    cur1=ADDOBJ_EX("Firm",cur);
    cur1->write("id",v[26]+v[27], 0);
    cur1->write("birth",(double)t, 0);
    cur1->write("generation",3, 0);
 
    cur1->write("IdTecUsed",MP, 0);
    
    cur2=cur1->search("Mix");
    cur2->write("randomMix",-1, t-1);
    cur2->cal("randomMix",0);
    
    cur1->write("alive",1, 0);
    cur1->write("entered",0, 0);
    cur1->write("diversified",0, 0);
    cur1->write("adopted",0, 0);
    
    cur1->write("daughter",1, 0);
    cur1->write("mother",0, 0);
    v[17]=cur->cal("bud",0);
    cur1->write("initBud",v[17]*v[10], 0);
    cur1->write("bud",v[17]*v[10], 0);
    cur1->write("debt",v[17]*v[10], 0);
    cur1->write("mup",v[16], 0);
    cur1->write("RDExpenditures",v[18], 0);
    cur1->write("ebw",cur->cal("ebw",0)*v[19], 0);
    
    v[22]=cur->cal("averageExperience",0);
    
    v[21]=cur->cal("experience",0);
    cur1->write("experience",(v[21]-v[22])*v[20], 0);
    
    cur2=cur1->search("Product");
    cur2->write("cheap",v[23], 0);
    cur2->write("perf",v[24], 0);
    cur->multiply("bud",(1-v[10]));
    cur->increment("bud",-1*v[11]);
    
    cur1->write("entered",1, 0);
    cur1->write("diversified",1, 0);
    cur1->write("whenEntered",(double)t, 0);
    cur1->write("servedUserClass",INDIV, 0);
    cur1->write("servedMkt",2, 0);

    cur->write("mother",1, 0);
    
    
   }
 
 }
 
res=v[27];
goto end;
}


if(!strcmp(label,"secondGenerationCreation"))
{
/*
Introduce the second generation
*/
v[0]=p->cal("secondGenerationTime",0);
if((double)t!=v[0])
 {res=0;
  goto end;
 }
v[0]=p->cal("firstGeneration",0); 
v[1]=p->cal("secondGeneration",0);

v[6]=p->cal("initBudReductionFactor",0);
v[7]=p->cal("initMarkUp",0);
v[8]=p->cal("RDexpendituresOnProfits",0);  
v[20]=p->cal("minInitBud",0);
v[21]=p->cal("maxInitBud",0);

cur1=p->search("Firm");
for(v[13]=1; v[13]<=v[1]; v[13]++ )
 {
  //cur=p->add_an_object("Firm",cur1);
  cur=ADDOBJ_EX("Firm",cur1);
  for(cur_v=cur->v; cur_v!=NULL; cur_v=cur_v->next)
    cur_v->val[0]=0; //I suspect that newly created Objects are set to 0 in Java
  cur->write("id",v[0]+v[13], 0);
  cur->write("birth",(double)t, 0);
  cur->write("generation",2, 0);
  cur->write("IdTecUsed",MP, 0);
  cur2=cur->search("Mix");
  cur2->write("randomMix",-1, t-1);  
  cur2->cal("randomMix",0);
  cur->write("alive",1, 0); 
  cur->write("entered",0, 0);
  cur->write("diversified",0, 0);
  cur->write("adopted",0, 0);
  cur->write("daughter",0, 0);
  cur->write("mother",0, 0);
  v[3]=v[20]+RND*(v[21]-v[20]);
  cur->write("initBud",v[3]*v[6], 0);
  cur->write("debt",v[3]*v[6], 0);  
  cur->write("bud",v[3]*v[6], 0);  
  cur->write("mup",v[7], 0);
  cur->write("RDExpenditures",v[8], 0);
  cur2=cur->search("ResTeam");
  cur2->write("cheapEng",0, 0);
  cur2->write("perfEng",0, 0);
  cur2->write("RTAverageExperience",0, 0);
  cur2=cur->search("Product");
  cur2->write("cheap",0, 0);
  cur2->write("perf",0, 0);
 }
 
res=1;
goto end;
}

if(!strcmp(label,"init"))
{
/*
Initialisation function of firms. It is computed just the first time step and never again
*/
demand=p->search("Demand");
v[0]=p->cal("minInitBud",0);
v[1]=p->cal("maxInitBud",0);
v[4]=p->cal("initMarkUp",0);
v[5]=p->cal("RDexpendituresOnProfits",0);  


for(v[2]=1,cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur),v[2]++ )
 {
  cur->write("id",v[2], 0);
  cur->write("birth",1, 0);
  cur->write("generation",1, 0);
  cur->write("IdTecUsed",TR, 0);

  cur1=cur->search("Mix");
  cur1->cal("randomMix",0);

  cur->write("alive",1, 0);
  cur->write("entered",0, 0);
  cur->write("diversified",0, 0);
  cur->write("adopted",0, 0);
  cur->write("daughter",0, 0);
  cur->write("mother",0, 0);
  v[3]=v[0]+RND*(v[1]-v[0]);
  cur->write("initBud",v[3], 0);
  cur->write("debt",v[3], 0);  
  cur->write("bud",v[3], 0);  
  cur->write("mup",v[4], 0);
  cur->write("RDExpenditures",v[5], 0);

 }

res=v[2];
param=1;
goto end;
}


if(!strcmp(label,"diagonal"))
{
/*
value of the diagonal in technologies
*/
v[0]=p->cal("cheapLim",0);
v[1]=p->cal("perfLim",0);

res=sqrt(v[0]*v[0]+v[1]*v[1]);
goto end;
}


if(!strcmp(label,"randomMix"))
{
/*
Set cheapMix and perfMix to new random values in between 0 and 1 such that their sum is 1.

It is computed only when requested
*/
last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}
v[0]=101*RND;
if(v[0]>100)
 v[0]=100;
p->write("cheapMix",v[0]/100, 0);
p->write("perfMix",(100-v[0])/100, 0);

 
res=1;
goto end;
}


if(!strcmp(label,"MedCheapPCFirm"))
{
/*
Average cheap value of firms in PC user class 
*/

for(v[0]=v[1]=0,cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[2]=cur->cal("alive",0);
  v[3]=cur->cal("servedMkt",0);
  if(v[2]==1 && v[3]==2)
    {v[1]+=cur->cal("cheap",0);
     v[0]++;
    } 
  
 }
if(v[0]>0)
  res=v[1]/v[0];
else
  res=0;  
goto end;
}

if(!strcmp(label,"MedPerfPCFirm"))
{
/*
Average performance value of firms in PC user class 
*/

for(v[0]=v[1]=0,cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
 {
  v[2]=cur->cal("alive",0);
  v[3]=cur->cal("servedMkt",0);
  if(v[2]==1 && v[3]==2)
    {v[1]+=cur->cal("perf",0);
     v[0]++;
    } 
  
 }
if(v[0]>0)
  res=v[1]/v[0];
else
  res=0;  
goto end;
}


if(!strcmp(label,"gammaDxCheap"))
{
/*
scale of design change 
*/

v[0]=p->cal("gammaDxCheapSt",0);
v[1]=p->cal("TRCheapLimSt",0);
v[2]=p->cal("alfa1DxSt",0);
cur=p->search_var_cond("tecLabel",TR, 0);
v[3]=cur->cal("cheapLim",0);
v[4]=p->cal("alfa1Dx",0);
v[19]=p->cal("alfa2Dx",0);
v[20]=p->cal("alfa3Dx",0);
v[21]=p->cal("alfa1DxSt",0);
v[22]=p->cal("alfa2DxSt",0);
v[23]=p->cal("alfa3DxSt",0);

v[5]=p->cal("minInitBudSt",0);
v[6]=p->cal("maxInitBudSt",0);
v[7]=p->cal("engineerCostSt",0);
v[8]=p->cal("projectDurationSt",0);
v[9]=p->cal("minInitBud",0);
v[10]=p->cal("maxInitBud",0);
v[11]=p->cal("engineerCost",0);
v[12]=p->cal("projectDuration",0);

v[16]=(0.5*(v[5]+v[6]))/(2*v[7]*v[8]); //RR0
v[17]=(0.5*(v[9]+v[10]))/(2*v[11]*v[12]);//RR1




v[18]=v[0]*pow(0.5*v[1],v[21])/pow(0.5*v[3],v[4]) *
							pow(v[16],v[22])/pow(v[17],v[19]) *
							pow(v[8],v[23])/pow(v[12],v[20]) *
							v[3]/v[1];

res=v[18];
goto end;
}

if(!strcmp(label,"gammaDxPerf"))
{
/*

scale of design change
*/

v[0]=p->cal("gammaDxPerfSt",0);
v[1]=p->cal("TRPerfLimSt",0);
v[2]=p->cal("alfa1DxSt",0);
cur=p->search_var_cond("tecLabel",TR, 0);
v[3]=cur->cal("perfLim",0);
v[4]=p->cal("alfa1Dx",0);
v[19]=p->cal("alfa2Dx",0);
v[20]=p->cal("alfa3Dx",0);
v[21]=p->cal("alfa1DxSt",0);
v[22]=p->cal("alfa2DxSt",0);
v[23]=p->cal("alfa3DxSt",0);

v[5]=p->cal("minInitBudSt",0);
v[6]=p->cal("maxInitBudSt",0);
v[7]=p->cal("engineerCostSt",0);
v[8]=p->cal("projectDurationSt",0);
v[9]=p->cal("minInitBud",0);
v[10]=p->cal("maxInitBud",0);
v[11]=p->cal("engineerCost",0);
v[12]=p->cal("projectDuration",0);

v[16]=(0.5*(v[5]+v[6]))/(2*v[7]*v[8]); //RR0
v[17]=(0.5*(v[9]+v[10]))/(2*v[11]*v[12]);//RR1




v[18]=v[0]*pow(0.5*v[1],v[21])/pow(0.5*v[3],v[4]) *
							pow(v[16],v[22])/pow(v[17],v[19]) *
							pow(v[8],v[23])/pow(v[12],v[20]) *
							v[3]/v[1];

res=v[18];
goto end;
}


if(!strcmp(label,"distanceCovered"))
{
/*
returns the fraction of the diagonal (from the origin to the tech corner) covered by the firm
*/
v[0]=p->cal("cheap",0);
v[1]=p->cal("perf",0);

v[6]=p->cal("IdTecUsed",0);
cur=p->search_var_cond("tecLabel",v[6], 0);

v[2]=cur->cal("diagonal",0);


res=sqrt(v[0]*v[0]+v[1]*v[1])/v[2];
goto end;
}

if(!strcmp(label,"NumFirm"))
{
/*
number of existing Object firm 
*/
v[0]=0;
for(cur=p->search("Firm"); cur!=NULL; cur=go_brother(cur) )
    v[0]++;
res=v[0];
goto end;
}


/*********************

Do not place equations beyond this point.

*********************/

sprintf(msg, "\nEquation for %s not found", label);
plog(msg);
quit=2;
return -1;


end :

if( (isnan(res)==1 || isinf(res)==1) && quit!=1)
 { plog("\\nMath error. Simulation switch to debug mode.\\nThe code for the current equation (see Debugger) produces illegal data. Check the equation code and the temporary values v[...] to find the faulty line(s).\\n\\n");
  debug_flag=1;
  debug='d';
 } 
 

if(debug_flag==1)
 {
 for(i=0; i<40; i++)
  i_values[i]=v[i];
 }


return(res);
}

/*
This function is executed once at the end of a simulation run. It may be used
to perform some cleanup, in case the model allocated memory during the simulation.
*/
void close_sim(void)
{

}



double  S(double x, double maxX, double maxY) 
{

   /* logistic function: this function is defined in the box [0,0][1,1]
      on x values 0, 0.25, 0.5, 0.75, 1. The input value is parametrized
      on maxX and the output value on maxY: if X>maxX then y=maxY whilest
      we have a 's' shaped relationship for other values of x.            */

		x=x/maxX;
			if (x<=0) return 0;
				else if (x<=0.25) return (x*0.5)*maxY;
					else if ((x>0.25)&&(x<=0.75)) return ((x-0.25)*1.5+0.125)*maxY;
						else if ((x>0.75)&&(x<=1)) return ((x-0.75)*0.5+0.875)*maxY;
							else if (x>1) return maxY;
		return 1;
}

int roundInt(double a) {
		if (a-(int)a>=0.5) return (int)(a+1);
		else return (int)a;
	}

