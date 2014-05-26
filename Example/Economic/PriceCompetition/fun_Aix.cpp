 
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

object *supply;

double variable::fun(object *caller)
{
//These are the local variables used by default
double v[40], res;
object *p, *c, *cur1, *cur2, *cur3, *cur4, *cur5;

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

sds
//Uncommenting the following lines the file "log.log" will
//contain the name of the variable just computed.
//To be used in case of unexpected crashes. It slows down sensibly the simulation
/**
f=fopen("log.log","a");
 fprintf(f,"t=%d %s\n",t,label);
 fclose(f);
**/

//Place here your equations. They must be blocks of the following type

if(!strcmp(label, "ActionDemand"))
{
/*
ActionDemand
This equation makes each consumer choice between its current seller and another one chosen randomly. The consumer compares the price's of the two sellers and decides for the smaller.
The price read from the new seller is subject to noise, drawn from a normal function centered on the true price and variance NoiseReading.
Note that the price of the current seller is re-loades, since in the meanwhile the old seller may have changed the price.
*/

supply->cal("ActionSupply",0);
v[0]=supply->cal("NumSeller",0);
v[8]=p->cal("NumConsumer",0);
v[5]=v[6]=v[7]=0;
v[11]=p->cal("NoiseReading",0);
for(cur2=p->search("Consumer"); cur2!=NULL; cur2=go_brother(cur2) )
{


  v[1]=rnd_integer(1, v[0]);
  cur=supply->search_var_cond("IdSeller", v[1],0);
  if(cur==NULL)
   printf("Shit");
  v[12]=cur->cal("Price",0);
  v[3]=supply->search_var_cond("IdSeller",cur2->cal("CurrSeller",0),0)->cal("Price", 0);
//Pas mal, ne c'est pas?  The supply->search_var_cond returns a pointer, to which I apply the cal
//function on Price. 
  v[2]=norm(v[12],v[11]);

  if(v[2]<v[3])
   {v[5]+=v[12];
    v[6]+=v[12]*v[12];
    cur2->write("CurrPrice", v[12],0);
    cur2->write("CurrSeller", v[1],0);
  
    cur->increment("ms",1/v[8]);
    cur->increment("Sales",1);
    supply->increment("NumSwitch",1);
   }
  else 
   {v[5]+=v[3];
    v[6]+=v[3]*v[3];
    v[4]=cur2->cal("CurrSeller",0);
    cur1=supply->search_var_cond("IdSeller",v[4],0);
    cur1->increment("ms",1/v[8]);
    cur1->increment("Sales",1);
    supply->increment("NumStay",1);
   }
  } 
v[10]=v[5]/v[8];
v[9]=v[6]/v[8]-(v[10]*v[10]);
p->write("AvPrice",v[10],0);
p->write("VarPrice", v[9],0);
res=v[9]; //return the variance for a check. It is of no use, of course
goto end;
}

if(!strcmp(label, "ActionSupply"))
{
/*
ActionSupply
Does nothing, but reinitialize the parameters where statistics are collected:
ms
Sales
NumStay
NumSwitch

*/
for(cur=supply->search("Seller"); cur!=NULL; cur=go_brother(cur) )
 {
 cur->cal("Price",0); //so that each price is updated before removing the data
 									//used for its computation
  cur->write("ms",0,0); 
  cur->write("Sales",0,0); 

 } 
p->write("NumSwitch",0,0);
p->write("NumStay",0,0);

res=1;
goto end;

}

if(!strcmp(label, "Price"))
{
/*
Price
Increased or decreased of Epsilon, depending on whether the ms are below or above TargetMs

*/
v[0]=p->cal("TargetMs",0);
v[1]=p->cal("Epsilon",0);
v[2]=p->cal("ms",0);
v[3]=p->cal("Price",1);
if(v[2]<v[0])
 {
  res=v[3]-v[1];
 }
else
 {
  res=v[3]+v[1];
 }  

goto end;
}

if(!strcmp(label, "Init"))
{
/*
Init
Initialization equation. 
- Assigns the global pointer supply, so that each equation
con address directly the group of sellers without using the inefficient Lsd default
search system (skips all the groups of consumers).
- Assigns randomly the consumers to one of the available sellers.

At the end, this Variable transforms itself in a Parameter, so that it is never computed again
during the simulation.

*/
supply=p->search("Supply");
v[1]=p->cal("NumSeller",0);
v[3]=p->cal("NumConsumer",0);
for(cur=p->search("Consumer"); cur!=NULL; cur=go_brother(cur) )
 {v[0]=rnd_integer(1,v[1]);
  cur->write("CurrSeller",v[0],0); 
  cur1=supply->search_var_cond("IdSeller", v[0],0);
  cur1->increment("ms",1/v[3]);
  cur1->increment("Sales",1);
  v[12]=cur1->cal("Price",0);
  cur->write("CurrPrice",v[12],0); 

 } 

res=1;
param=1;
goto end;

}

if(!strcmp(label, "NumSeller"))
{
/*
Computes the number of sellers and become a parameter
*/
v[0]=0;

for(cur=supply->search("Seller"); cur!=NULL; cur=go_brother(cur) )
 v[0]++;

res=v[0];
param=1;
goto end; 
}

if(!strcmp(label, "NumConsumer"))
{

/*
Computes the number of Consumers and become a parameter
*/

v[0]=0;

for(cur=p->search("Consumer"); cur!=NULL; cur=go_brother(cur) )
 v[0]++;

res=v[0];
param=1;
goto end; 
}

/*********************

Do not place equations beyond this point.

*********************/

sprintf(msg, "\nFunction for %s not found", label);
plog(msg);
quit=2;
return -1;


end :
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













