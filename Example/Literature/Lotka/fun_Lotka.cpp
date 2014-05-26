
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


//Uncommenting the following lines the file "log.log" will
//contain the name of the variable just computed.
//To be used in case of unexpected crashes. It slows down sensibly the simulation
/**
f=fopen("log.log","a");
 fprintf(f,"t=%d %s\n",t,label);
 fclose(f);
**/

//Place here your equations. They must be blocks of the following type

if(!strcmp(label, "ActionWolf"))
{
/*
Activities of the wolf.
If starving, it dies. Otherwise hunts and reproduce.
*/
v[0]=p->cal("Hunt",0); //hunting
if(v[0]==1)
  {p->write("Hunger",0, 0); //a sheep is eaten
   v[1]=0;
  } 
else
 v[1]=p->increment("Hunger",1);  //not sheep is eaten and the wold is more hungry

v[3]=p->cal("ProbWReproduce",0);
if(RND<v[3] )
  p->cal("NewWolf",0); //add a new wolf

v[2]=p->cal("MaxHunger",0);
if(v[1]>v[2])
 { //too hungry
  v[0]=p->cal("NumWolf",0); 
  if(v[0]>2)
    res=0;  //this wolf is going to starve
  else
    res=1;  //it is not killed if there are not enough wolfs

 }
else
 res=1; 
goto end;
}

if(!strcmp(label,"Hunt"))
{
/*
Hunting. It is a function activated only if a wolf requests it.
With a probability proportional to the density of sheep the wolf gets a dinner.

*/
last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}

v[3]=p->cal("MaxHunger",0);
v[4]=c->cal("Hunger",0);
if(RND>v[4]/v[3]) //hunt only if hungry
 {res=0;
  goto end;
 } 

v[0]=p->cal("NumSheep",0);
if(v[0]<2)
 {res=0; //if there are too few sheep, then fail
  goto end;
 } 
v[1]=p->cal("MaxSheepHunting",0); //max number of sheep
if(RND<v[0]/v[1])
 {res=1;
  cur=p->draw_rnd("Sheep","SHunger",0); //choose randomly a sheep proportional to the hunger
  cur->delete_obj();
  p->increment("NumSheep",-1);	
  p->increment("SheepEaten",1);
 }
else
 res=0; //bad hunting
goto end;
}

if(!strcmp(label,"NewWolf"))
{
/*
Entry of a new wolf. It is a function activated only if it is requested by a wolf.
*/
last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}


cur=p->search("Wolf"); //just for copy the structure of the Object Wolf
//cur1=p->add_an_object("Wolf", cur); //add a new wolf.
cur1=ADDOBJ_EX("Wolf",cur);
cur1->write("Hunger",0, 0); //initialize its hunger
cur1->write("AgeWolf",0, 0);
p->increment("NumWolf",1); //increase the number of wolfs
res=1;
goto end;
}



if(!strcmp(label,"NewSheep"))
{
/*
Add a new sheep. It is a function activated only if it is requested by a sheep
*/
last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}
cur=p->search("Sheep"); //just for copy
//cur1=p->add_an_object("Sheep", cur);
cur1=ADDOBJ_EX("Sheep",cur);
cur1->write("SHunger",1, 0); //initialize the new sheep
cur1->write("AgeSheep",0, 0);
p->increment("NumSheep",1);
res=1;
goto end;
}

if(!strcmp(label,"ActionSheep"))
{
/*
Activities of a sheep. Eat, if there is some grass, and reproduce.
*/
v[1]=p->cal("SHunger",0);
v[2]=p->cal("MaxSHunger",0);
if(v[1]>v[2])
 { //starving sheep
  res=0;
  goto end;
 }
v[3]=p->cal("NumSheep",0);
v[4]=p->cal("MaxSheepFood",0);
if(RND>v[3]/v[4]) //probability to eat proportional to the density of sheep
 {v[5]=p->cal("SHunger",0);
  if(v[5]>2)
    p->increment("SHunger",-1); //if the sheep found some grass, it is less hungry
 }   
else
 p->increment("SHunger",1); //otherwise it is more hungry (beware of the wolfs)

v[6]=p->cal("ProbSReproduce",0);
if(RND<v[6])
 { //reproduce
  p->cal("NewSheep",0);
 }  

res=1;
goto end;
}

if(!strcmp(label,"Action"))
{
/*
General routine for a step.
First, all the sheep make die for age, otherwise do their activities and, if starving, they die. The all wolfs do the same
*/
//initialize some indicators
p->write("SheepEaten",0,0);
p->write("SheepStarved",0,0);
p->write("SheepAged",0,0);
p->write("WolfStarved",0,0);
p->write("WolfAged",0,0);

//update the sheep
v[3]=p->cal("MaxAgeSheep",0); //time to rest
for(cur=p->search("Sheep"); cur!=NULL; cur=cur1 )
 {
  cur1=go_brother(cur);
  v[1]=cur->increment("AgeSheep",1);
  if(v[1]>v[3])
   { //too old
   v[5]=p->increment("NumSheep",-1);
   if(v[5]>2)
    {cur->delete_obj(); //too few sheep, so it has to survive
     p->increment("SheepAged",1);
    }
   else
    p->increment("NumSheep",1);  

   } 
  else
   { //still young
    v[0]=cur->cal("ActionSheep",0);
    if(v[0]==0)
    {cur->delete_obj(); //starve to death
     p->increment("NumSheep",-1);
     p->increment("SheepStarved",1);
    } 
   }
 }
 
//update the wolfs
v[3]=p->cal("MaxAgeWolf",0); 
for(cur=p->search("Wolf"); cur!=NULL; cur=cur1 )
 {
  cur1=go_brother(cur);
  v[1]=cur->increment("AgeWolf",1);
  if(v[1]>v[3])
   { //to old
    cur->delete_obj();
    p->increment("NumWolf",-1);
    p->increment("WolfAged",1);
   } 
  else
   { //still young
    v[0]=cur->cal("ActionWolf",0);
    if(v[0]==0)
     {cur->delete_obj(); //starved
      p->increment("WolfStarved",1);
      p->increment("NumWolf",-1);
     } 
   }
 }
res=1;
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












