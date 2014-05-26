
/***************************************************
****************************************************
LSD 4.0 - July 2001

Copyright Marco Valente - Aalborg University - University of L'Aquila
Send comments and bug reports to mv@business.auc.dk
****************************************************
****************************************************/


/*
Model of NK fitness landscape and strategic random exploration.
*/

struct dynlink
{
dynlink *l0;
dynlink *l1;
double *f;
};



struct bit
{
int id;
struct dynlink l;
int *link;
int nlink;
double fitcontr;
};

bit *fc;



#include "../src/fun_head.h"
unsigned int *str=NULL, *str2=NULL, *str3=NULL, *str4=NULL;

object *fl=NULL;
void int2bin(double in, double n, unsigned int *str);
double bin2int(unsigned int *s2, double n);
void loaddatauniv(bit *b, int n, int k, FILE *f);
void shift(struct dynlink *c);

/***************************************
VARIABLE::FUN
***************************************/
double variable::fun(object *caller)
{
double v[40], res;
int i,j,k;
object *p, *c, *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cur10;
variable *cur_v;
p=up;
c=caller;
struct dynlink *cl;

FILE *f, *f1;
/**
if(t>=1099)
{
f=fopen("log.log","a");
 fprintf(f,"%s at %d\n",label, t);
 fclose(f);
}
**/

if(!strcmp(label, "FitFun") )
{
/* 
Compute the fitness of the binary point stored in "str".

This implementation makes use of dynamically allocated memory. It means that FitFun stores in memory the fitness of points already computed, including the fitness contribution of each bit of the string. If the point has nevery been computed before, then a new fitness value is randomly generated respecting the constraints defined in the landscape (the epistatic relations), and stored in memory for future uses.

The core idea of NK systems is that the fitness of a binary point is computed as average of the fitness contributions (fc) of each element. The epistatic relations of a NK system define which bit influences other bit. Therefore, the system must store in memory 2^(K+1) fc's for each bit linked to other K bits. The current implementation, summarised below, allows to create as complex landscapes as desired, even if the whole NK system would require and impossibly huge amount of memory. The system continues to allocate memory for new points until the OS memory limitations are reached. At that point the Lsd program crashes. Note that under Windows the system crashes more easily if more than one simulation is run in succession (WIN seems to not make available immediately the memory released). 

Technically, the data on fitness values are stored in a memory structure defined as:

struct bit
{
int id; //id of the bit (from 1 to N)
struct dynlink l; //see below
int *link; //vector of integers reporting the bit's linked to the bit
int nlink; //number of bit's linked to the bit (i.e. length of *link)
double fitcontr; //fitness contribution of the bit, computed by FitFun whenever requested
};

struct dynlink
{
dynlink *l0;
dynlink *l1;
double *f;
};

The dynlink is an element of the binary linked chain generated whenever the fitness of a point is computed. Starting from bit.l a unary linked chain is generated passing for l0 or l1 depending on the states of the related bits. Only the last dynlink contains an existing f field, for the fitness contributions.

When FitFun is requested then for each bit stores the states of related bits in *link. Then, in l checks whether l0 exists or is NULL (if the related bit is 0) or check l1, if the related bit is 1. Continue until either:
- a NULL l0 or l1 is encountered. Continue to generate a new linked chain until the end of related bits and generate a new fitness contribution stored in f.
- the related bits are finished. Return the f value as fitness contribution.

At the end of each simulation run the "close_sim()" function cleans up all the memory


*/

//overrule the standard Lsd automatic scheduling system. The equation is
//computed any time is requested and not only once in each time step.
last_update--;
if(c==NULL)
 {
 res=-1;
 goto end;
 }

v[0]=p->cal("N",0); //bit length

for(v[1]=0,i=0; i<(int)v[0]; i++)
 {//for each bit
 v[2]=0; //assume that the fc exists
  for(cl=&(fc[i].l), j=0; j<fc[i].nlink ; j++)
   {//for each link of the bit
    k=fc[i].link[j]; //this is the link
    if(v[2]==1)
      {//creating fc because at some point the link did not exist
       if(str[k]==0)
        {cl->l0=new dynlink; //the link created is 0
         cl->l0->l0=NULL;
         cl->l0->l1=NULL;
         if(j==fc[i].nlink-1)
          {cl->l0->f=new double;
           *cl->l0->f=RND; //last link, create the fc
          } 
         cl=cl->l0;  //next element of the linked chain
        }
       else
        {cl->l1=new dynlink;//the link created is 1
         cl->l1->l0=NULL;
         cl->l1->l1=NULL;
         if(j==fc[i].nlink-1)
          {cl->l1->f=new double;
           *cl->l1->f=RND; //last link, create the fc
          } 
         cl=cl->l1;  //next element of the linked chain
        } 

      }
    else
      {//existing link 
       if(str[k]==0)
        {//the next link is 0
         if(cl->l0!=NULL)
          cl=cl->l0; //the next link exists
         else
          {//create the next link 
           cl->l0=new dynlink; 
           cl->l0->l0=NULL;
           cl->l0->l1=NULL;
           if(j==fc[i].nlink-1)
            {cl->l0->f=new double;
            *cl->l0->f=RND; //last link, create the fc
            } 

           cl=cl->l0;  
           v[2]=1; //flag creating set on, from now on all links are created
          } 
        }
       else
        {//the next link is 1
         if(cl->l1!=NULL)
          cl=cl->l1; //the next link exists
         else
          {//create the next link
           cl->l1=new dynlink; 
           cl->l1->l0=NULL;
           cl->l1->l1=NULL;
           if(j==fc[i].nlink-1)
            {cl->l1->f=new double;
            *cl->l1->f=RND; //last link, create the fc
            } 
          cl=cl->l1;  
           v[2]==1; //flag creating set on, from now on all links are created
          } 

        } 

      
      }//end of existing fc
    
   }//end of for through links, therefore cl points to the last element, owning f
   v[1]+=*cl->f; //for the average
   fc[i].fitcontr=*cl->f; //individual fc
 }//end of for through bits

 
res=v[1]/v[0];
goto end;
}



if(!strcmp(label,"Mutation"))
{
/*
Mutation modifies the point of the agent. It returns 0 is not mutation took place, and the number of mutate bits otherwise.

There are several options concerning the timing and the type of mutation:
- MutationGlobal
- MutationTeam
- MutationIndividual
- MutationTeamParallel

All the above mutation style are based on the agent's strategy, that is, the grouping of the loci in blocks that are assumed to be reciprocally linked. They differ because of the fitness measures use to accept or reject the new mutated point.

I MutateAlways is set to 0, the mutation attempts take place only every few time steps, with a number of idle time steps equal to the dimensions of the loci groups in the agent strategy. This ensure that the expected number of mutated bits is identical for agents with different research strategies.

*/

v[6]=p->cal("CounterMutation",0); //time steps from the next mutation
v[21]=p->up->cal("MutateAlways",0); //option set on to force agents to mutate always
if(v[6]>0 && v[21]==0) //control whether mutate or not
 { p->increment("CounterMutation",-1); //don't mutate. Decrease timing next mutation
   res=0;
   goto end;
 }

//go on with mutation 
v[7]=p->cal("MaxBitLength",0);
/*reset the next time of mutation depending on the length of mutation pool */
p->write("CounterMutation",v[7], 0); 
   
//execute a test, if requested
v[2]=p->cal("TestDinner",0);
v[4]=p->cal("Fitness",0);
/*type of fitness measure to use deciding whether to acccept the mutated point */
v[0]=p->cal("RewardType",0); 

if(v[0]==1)
 v[1]=p->cal("MutationGlobal",0); //use the overall fitness
if(v[0]==2)
 v[1]=p->cal("MutationTeam",0); //use the fitness of the mutation pool
if(v[0]==3)
 v[1]=p->cal("MutationIndividual",0); //use the single fitness contributions
if(v[0]==4)
 v[1]=p->cal("MutationTeamParallel",0); //mutate all mutation pools in paralles

v[3]=p->cal("Fitness",0); //(possibly) new fitness
if(v[2]==0 && v[4]<v[3]) //Control on the TestDinner, if executed
 {plog("\nTest failed");
  v[1]*=-1;
 }
res=v[1];
goto end;
}

if(!strcmp(label,"MutationTeamParallel"))
{
/*
This mutation method tries to change all blocks, accepting the mutation if the block fitness is higher than before.
If successful set:
- new fitness in Fitness
- new Values for each Bit
- new fitness contributions for each bit

The implementation is tricky because each block must be mutated as compared to the original ones, even while blocks may have been mutated. Therefore, there are several strings storing points.

- str is always the string to test (used in FitFun)
- str3 stores always the original point, even when some blocks may be mutated
- str4 stores bits from the (possibly mutated) blocks
*/

last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}
v[11]=p->up->cal("ProbMut",0);

for(v[0]=0,cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1) )
    str3[(int)v[0]++]=(int)cur1->cal("Value",0); //store the original string
 }

v[2]=fl->cal("N",0);
//for each block
for(cur3=p->search("Block"); cur3!=NULL; cur3=go_brother(cur3) )
{
for(i=0; i<(int)v[2]; i++)
 str[i]=str3[i]; //initialize the str

v[4]=cur3->cal("NumBits",0); //check out many bits contains the block
//draw randomly one bit in the chosen block and force the flipping of its state
v[9]=rnd_integer(1,v[4]);

cur1=cur3->search_var_cond("IdBitBlock",v[9],0); //here is the bit

v[8]=cur1->cal("IdALocus",0);
str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; //invert the bit

v[20]=1;
for(cur2=cur3->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
 {//for each bit in the block, starting from the first
  if(cur2!=cur1)
  {//if it is not the already flipped bit
  if(RND<v[11])
   {//flip the bit with probmut probability
    v[8]=cur2->cal("IdALocus",0); //# of the bit
    str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; //invert the bit    
    v[20]++;
   }
  }   
 }
for(v[15]=0, cur2=cur3->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) ) 
 v[15]+=cur2->cal("FitnessContribution",0); //Block fitness before mutation

v[18]=fl->cal("FitFun",0);  //new overall fitness after mutation

//compute the block fitness
for(v[16]=0, cur2=cur3->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) ) 
 {v[17]=cur2->cal("IdALocus",0);
  v[16]+=fc[(int)v[17]-1].fitcontr;
 } 

if(v[16]>v[15])
{//mutation succeed, replace old string values
  for(cur2=cur3->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
   {//for each bitblock 
    v[21]=cur2->cal("IdALocus",0);
    str4[(int)v[21]-1]=str[(int)v[21]-1]; //store in the result string the mutated bits
   }
} 
else
{//mutation failed, use original string values
  for(cur2=cur3->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
   {//for each bitblock 
    v[21]=cur2->cal("IdALocus",0);
    str4[(int)v[21]-1]=str3[(int)v[21]-1]; //store in the result string the mutated bits
   }
} 
} //end of for each block

//adjust overall data

//store the final string in str
for(v[20]=0,i=0; i<(int)v[2]; i++)
 {if(str[i]!=str4[i])
    v[20]++; //count mutations
  str[i]=str4[i]; //set the final string the str
 }

v[22]=fl->cal("FitFun",0);
p->write("Fitness",v[22], 0); //overall fitness

for(i=0,cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {//for each block
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1) )
    {//for each bit in the block
    cur1->write("Value",str[i],0); //store the bit value
    cur1->write("FitnessContribution",fc[i++].fitcontr, 0); //store the fc values
    }
 }
 
res=v[20]; //returns the number of mutated bits
goto end;
}


if(!strcmp(label,"TestDinner"))
{
/*
The test explore all the points that may be reached with the mutation from the current point
Of course, it executes the correct test for each type of reward.
The function returns the percentage of points with higher fitness, practically the probability of a successful mutation. 
The test does nothing to the functioning of the agent and takes a hell of a lot of time, so there is the possibility to switch it off setting NoTest=1.

Just for fun, it may compute also the test values assuming the other two types of reward. To avoid such useless and expensive extra-tests comment the cals to the tests (not assigned to v[1]) AND de-comment the subsequent p->write (to avoid the tests be computed automatically).
*/

if(p->up->cal("NoTest",0)==1)
 {//no test
  res=-1;
  goto end;
 }

v[0]=p->cal("RewardType",0);
if(v[0]==1)
 {v[1]=p->cal("TestDinnerGlobal",0);
  
  //p->cal("TestDinnerIndividual",0);
  //p->cal("TestDinnerTeam",0);
  p->write("TestDinnerTeam",0, t);
  p->write("TestDinnerIndividual",0, t);
 }
if(v[0]==2)
 {v[1]=p->cal("TestDinnerTeam",0);
 
  //p->cal("TestDinnerGlobal",0);
  //p->cal("TestDinnerIndividual",0);
  p->write("TestDinnerIndividual",0, t);
  p->write("TestDinnerGlobal",0, t);
 }
if(v[0]==3)
 {v[1]=p->cal("TestDinnerIndividual",0);
  
  //p->cal("TestDinnerTeam",0);  
  //p->cal("TestDinnerGlobal",0);
  p->write("TestDinnerGlobal",0, t);
  p->write("TestDinnerTeam",0, t); 
 }

res=v[1];
goto end;

}

if(!strcmp(label,"TestDinnerGlobal"))
{
/*
Test if there is still a potential improvement. Test all the possible mutations and 
return the percentage of success using the global reward

Return -1 if skipped.
*/

if(p->up->cal("NoTest",0)==1)
 {
  res=-1;
  goto end;
 }

v[5]=fl->cal("N",0);

for(v[0]=0,cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1),v[0]++ )
    str[(int)v[0]]=str3[(int)v[0]]=(int)cur1->cal("Value",0);
 }

v[4]=fl->cal("FitFun",0); //current fitness

v[7]=v[8]=0;

for(cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {// for all blocks
 v[3]=cur->cal("IdALocus",0)-1; //this is the first bit of the block. The rest is assumed contiguous
 v[2]=cur->cal("NumBits",0);
 v[1]=pow(2, v[2]); //number of possible combinations for the block
 for(v[10]=0; v[10]<v[1]; v[10]++) //for all the combinations
  {int2bin(v[10], v[2], str4); //store in str4 the binary version of the combination
   for(i=0; i<(int)v[2]; i++)
     str[i+(int)v[3]]=str4[i]; //load the current combination of the block in the original string
   v[6]=fl->cal("FitFun",0); 
   if(v[6]>v[4])
     v[7]++; //increment the counter of succesful combinations
   v[8]++; //increment the counter of combinations
  } 
 for(i=0; i<(int)v[2]; i++)
  str[i+(int)v[3]]=str3[i+(int)v[3]]; //re-install the original string
 }
/*
//control that the strings don't mess up
for(v[11]=0; v[11]<v[5]; v[11]++)
  if(str[(int)v[11]]!=str3[(int)v[11]])
   plog("\nDouble shit");
*/   
res=v[7]/v[8]; //ration of successes vs combinations
goto end;
}


if(!strcmp(label,"TestDinnerTeam"))
{
/*
Test if there is still a potential improvement. Test all the possible mutations and 
return the percentage of success using the team reward

Return -1 if skipped.
*/

if(p->up->cal("NoTest",0)==1)
 {
  res=-1;
  goto end;
 }

v[5]=fl->cal("N",0);

for(v[0]=0,cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {//load the original string
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1),v[0]++ )
    str[(int)v[0]]=str3[(int)v[0]]=(int)cur1->cal("Value",0);
 }

v[7]=v[8]=0;

for(cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {//for all the blocks
 
 //compute the team fitness
 for(v[4]=0, cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1),v[0]++ )
    v[4]+=cur1->cal("FitnessContribution",0);

 v[3]=cur->cal("IdALocus",0)-1; //this is the first bit of the block. The rest is assumed contiguous
 v[2]=cur->cal("NumBits",0);
 v[1]=pow(2, v[2]);//number of combinations of the block
 for(v[10]=0; v[10]<v[1]; v[10]++)
  {
   //for all the combinations
   int2bin(v[10], v[2], str4);
   for(i=0; i<(int)v[2]; i++)
     str[i+(int)v[3]]=str4[i]; //load in str the current combination
   fl->cal("FitFun",0);
   for(v[6]=0,i=0; i<(int)v[2]; i++)
     v[6]+=fc[i+(int)v[3]].fitcontr; //compute the team fitness of the current string

   if(v[6]>v[4])
     v[7]++; //increase the sucesses
   v[8]++; //increase the attempts
  } 
 for(i=0; i<(int)v[2]; i++)
  str[i+(int)v[3]]=str3[i+(int)v[3]]; //re-install the original string
 }
/*
//control that the strings don't mess up

for(v[11]=0; v[11]<v[5]; v[11]++)
  if(str[(int)v[11]]!=str3[(int)v[11]])
   plog("\nDouble shit");
*/
   
res=v[7]/v[8]; //ratio of successes vs combinations
goto end;
}

if(!strcmp(label,"TestDinnerIndividual"))
{
/*
Test if there is still a potential improvement. Test all the possible mutations and 
return the percentage of success using the individual reward.

Return -1 if skipped.
*/
if(p->up->cal("NoTest",0)==1)
 {
  res=-1;
  goto end;
 }

v[5]=fl->cal("N",0);

for(v[0]=0,cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {//load the original string
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1),v[0]++ )
    str[(int)v[0]]=str3[(int)v[0]]=(int)cur1->cal("Value",0);
 }

v[7]=v[8]=0;

for(cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {//for all the blocks
 v[3]=cur->cal("IdALocus",0)-1; //this is the first bit of the block. The rest is assumed contiguous
 v[2]=cur->cal("NumBits",0);
 v[1]=pow(2, v[2]); //compute the combinations of the block
 for(v[10]=0; v[10]<v[1]; v[10]++)
  {int2bin(v[10], v[2], str4);
   for(i=0; i<(int)v[2]; i++)
     str[i+(int)v[3]]=str4[i]; //load the combination in str
   fl->cal("FitFun",0);

   for(i=0, cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1),i++ )
     { v[4]=cur1->cal("FitnessContribution",0);
       v[6]=fc[i+(int)v[3]].fitcontr;
     if(v[6]>v[4])
       v[7]++; //increase the successes
     v[8]++; //increase the combinations
     }
   }  
 for(i=0; i<(int)v[2]; i++) //re-install the original string
  str[i+(int)v[3]]=str3[i+(int)v[3]];
 }

/*
//control that the strings don't mess up
 
for(v[11]=0; v[11]<v[5]; v[11]++)
  if(str[(int)v[11]]!=str3[(int)v[11]])
   plog("\nDouble shit");
*/   
res=v[7]/v[8];//ratio of successes vs combinations
goto end;
}


/************************************
INDIVIDUAL
*************************************/
if(!strcmp(label,"MutationIndividual"))
{
/*
This mutation method chooses one block and flip at least one (and at most all) bits of the original string. Each individual bit decides whether to accept or reject the mutation on whether the fitness contribution of the newly mutated string is higher than the previous fitness contribution.

Note that the actual fitness contributions of each bit after the whole process may well be below the previous time. In fact, if the bits are linked, then the decision of accepting a mutation may depend on a bit that, in the event, rejected the mutation. As a consequence, agents using this method may (and usually do) see their overall fitness moving up and down.


If successful set:
- new fitness in Fitness
- new Values for each Bit
- new fitness contributions for each bit

*/
last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}
v[11]=p->up->cal("ProbMut",0);

for(v[0]=0,cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1) )
    str[(int)v[0]++]=(int)cur1->cal("Value",0);
 }


//Choose the block to mutate
v[2]=p->cal("NumBlocks",0);
v[3]=rnd_integer(1, v[2]);
p->write("BlckChs",v[3], 0);
cur=p->search_var_cond("IdBlock",v[3], 0);
v[4]=cur->cal("NumBits",0); //check out many bits contains the block
//draw randomly one bit in the chosen block and force the flipping of its state
v[9]=rnd_integer(1,v[4]);
p->write("BitChs",v[9], 0);
cur1=cur->search_var_cond("IdBitBlock",v[9],0); //here is the bit

v[8]=cur1->cal("IdALocus",0);
str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; //invert the bit

v[20]=1;
for(cur2=cur->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
 {//for each bit in the block
  if(cur2!=cur1)
  {//if it is not the already flipped bit
  if(RND<v[11])
   {//with probmut probability
    v[8]=cur2->cal("IdALocus",0);
    str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; //invert the bit    
    v[20]++;
   }
  }   
 }
fl->cal("FitFun",0); 
for(cur2=cur->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
 { //for each bit of the block
  v[16]=cur2->cal("Value",0);
  v[8]=cur2->cal("IdALocus",0);

  if(str[(int)v[8]-1]!=v[16]) //only if it mutated
   {
    v[5]=cur2->cal("FitnessContribution",0);
    if(fc[(int)v[8]-1].fitcontr>v[5]) //the new fitness is better
      cur2->write("Value",  str[(int)v[8]-1], 0); //turn the old value as the new one
    else
      {str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; 
       v[20]--;//decrease the mutation counter
      } 
   }   
 }

v[7]=fl->cal("FitFun",0);
p->write("Fitness",v[7], 0);
for(v[7]=0, cur3=p->search("Block"); cur3!=NULL; cur3=go_brother(cur3) )
 {//for each block
   for(cur2=cur3->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
   {//for each bitblock compute and assign the true fitness contribution
    cur2->write("FitnessContribution",fc[(int)v[7]++].fitcontr,0);
   }
 }
res=v[20];
goto end;
}


/************************************
TEAM
*************************************/
if(!strcmp(label,"MutationTeam"))
{
/*
This mutation method selects one block and mutate at least one, and at most all, the bits in it. After the mutation the sum of the fitness contributions of the block's bit is compared with the sum before the mutation. If the new sum is higher the mutation is accepted.

Note that if the agents' block do not match the true decomposition of the landscape, then it is possible that the overall fitness falls. This is because after a succesful mutation the fitness contribution of bits outside the block (but actually linked) change unpredictably their contribution.

If successful set:
- new fitness in Fitness
- new Values for each Bit
- new fitness contributions for each bit
*/

last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}
v[11]=p->up->cal("ProbMut",0);

for(v[0]=0,cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 { //load the original string
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1) )
    str[(int)v[0]++]=(int)cur1->cal("Value",0);
 }


//Choose the block to mutate
v[2]=p->cal("NumBlocks",0);
v[3]=rnd_integer(1, v[2]);
p->write("BlckChs",v[3], 0);
cur=p->search_var_cond("IdBlock",v[3], 0);
v[4]=cur->cal("NumBits",0); //check out many bits contains the block
//draw randomly one bit in the chosen block and force the flipping of its state
v[9]=rnd_integer(1,v[4]);
p->write("BitChs",v[9], 0);
cur1=cur->search_var_cond("IdBitBlock",v[9],0); //here is the bit

v[8]=cur1->cal("IdALocus",0);
str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; //invert the bit

v[20]=1;
for(cur2=cur->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
 {//for each bit in the block
  if(cur2!=cur1)
  {//if it is not the already flipped bit
  if(RND<v[11])
   {//with probmut probability
    v[8]=cur2->cal("IdALocus",0);
    str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; //invert the bit    
    v[20]++;
   }
  }   
 }
//compute the block's fitness contribution _before_ the mutation
for(v[15]=0, cur2=cur->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) ) 
 v[15]+=cur2->cal("FitnessContribution",0);

v[18]=fl->cal("FitFun",0); 

//compute the block's fitness contribution _after_ the mutation
for(v[16]=0, cur2=cur->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) ) 
 {v[17]=cur2->cal("IdALocus",0);
  v[16]+=fc[(int)v[17]-1].fitcontr;
 } 

if(v[16]>v[15])
{//mutation succeed, replace old string, fitcontr and fitness
 p->write("Fitness",v[18], 0);
 for(v[7]=0, cur3=p->search("Block"); cur3!=NULL; cur3=go_brother(cur3) )
 {//for each block
   for(cur2=cur3->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
   {//for each bitblock compute and assign the true fitness contribution
    cur2->write("FitnessContribution",fc[(int)v[7]].fitcontr,0);
    cur2->write("Value",str[(int)v[7]++], 0);
   }
 }
} 
else
 v[20]=0;
res=v[20]; //returns the number of mutated bits
goto end;
}


/************************************
GLOBAL
*************************************/
if(!strcmp(label,"MutationGlobal"))
{
/*
This mutation method selects one block and mutate at least one, and at most all, the bits in it. After the mutation the overall fitness of the mutated string is compared with the fitness before the mutation. If the new value is higher the mutation is accepted.

Unless parameter "AcceptAlways" is set on, the mutation method ensures that the values of fitness can only go upwards.


If successful set:
- new fitness in Fitness
- new Values for each Bit
- new fitness contributions for each bit

*/
last_update--;//repeat the computation any time is requested
if(c==NULL)//Avoids to be computed when the system activates the equation
{
res=-1;
goto end;
}
v[11]=p->up->cal("ProbMut",0);

for(v[0]=0,cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {//load the current point in str.
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1) )
    str[(int)v[0]++]=(int)cur1->cal("Value",0);
 }


//Choose the block to mutate
v[2]=p->cal("NumBlocks",0);
v[3]=rnd_integer(1, v[2]);
p->write("BlckChs",v[3], 0);
cur=p->search_var_cond("IdBlock",v[3], 0);
v[4]=cur->cal("NumBits",0); //check out many bits contains the block
//draw randomly one bit in the chosen block and force the flipping of its state
v[9]=rnd_integer(1,v[4]);
p->write("BitChs",v[9], 0);
cur1=cur->search_var_cond("IdBitBlock",v[9],0); //here is the bit

v[8]=cur1->cal("IdALocus",0);
str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; //invert the bit

v[20]=1;
for(cur2=cur->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
 {//for each bit in the block
  if(cur2!=cur1)
  {//if it is not the already flipped bit
  if(RND<v[11])
   {//with probmut probability
    v[8]=cur2->cal("IdALocus",0);
    str[(int)v[8]-1]=str[(int)v[8]-1]==1?0:1; //invert the bit    
    v[20]++;
   }
  }   
 }
v[15]=p->cal("Fitness",0);//Fitness before
v[18]=fl->cal("FitFun",0);//Fitness after

v[21]=p->up->cal("AcceptAlways",0);
if(v[18]>v[15]||v[21]==1)
{//mutation succeed, replace old string, fitcontr and fitness
 p->write("Fitness",v[18], 0);
 for(v[7]=0, cur3=p->search("Block"); cur3!=NULL; cur3=go_brother(cur3) )
 {//for each block
   for(cur2=cur3->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2) )
   {//for each bitblock compute and assign the true fitness contribution
    cur2->write("FitnessContribution",fc[(int)v[7]].fitcontr,0);
    cur2->write("Value",str[(int)v[7]++], 0);
   }
 }
} 
else
 v[20]=0;
res=v[20]; //number of mutated bits
goto end;
}


if(!strcmp(label, "MaxFitness") )
{
/*****
This equation computes the maximum fitness of the population. It also performs
a number of other computations. Namely, it resets to zero the values of a set 
of statistics in each object Class. The function continues triggering the computation of
Mutation for each agent. Then it reads the value of Fitness of the just mutated
agent and adds this value to its class (determined by value of MaxBitLength).
After having finished the mutations for each agents, it computes the average fitness of
of each class by dividing the total fitness by the number of Agents in the each class.
The funtion writes also the id number of the highest fitness agent in the parameter
IdWinner.
*****/
v[3]=p->cal("NumAgent",0);
//resets to 0 all the statistics collected by Classes
for(cur=p->search("Class"); cur!=NULL; cur=go_brother(cur) )
 {cur->write("AvClFitness",0,0);
  cur->write("SucMut",0,0);
  cur->write("VarClFitness",0,0);
  cur->write("AvGain",0,0);
  cur->write("AvRank",0,0);
  cur->write("AvAge",0,0);
  cur->write("BestCFit",0,0);
  cur->write("AvTestDinner",0,0);
  cur->write("AvOldFitness",0,0);
 }


//triggers the mutation for each agent and collect statics.
for(v[20]=0,v[6]=1, v[2]=0, cur=p->search("Agent"); cur!=NULL; cur=go_brother(cur))
 {
  v[22]=cur->cal("Fitness",0); //fitness before

  cur->cal("Mutation",0);
  cur->increment("Age",1); //agents age
  v[1]=cur->cal("Fitness",0); //fitness after mutation
  v[2]+=v[1]; //sum of new fitness
  v[4]=cur->cal("AgentType",0);
  cur2=p->search_var_cond("IdClass",v[4],0); //the class of the agent
  cur2->increment("AvGain",v[1]-v[22]);
  cur2->increment("AvOldFitness",v[22]);  
  if(v[1]>v[22])
     cur2->increment("SucMut",1);
  cur2->increment("AvClFitness",v[1]);
  cur2->increment("VarClFitness",v[1]*v[1]);
  v[5]=cur2->cal("BestCFit",0);
  if(v[1]>v[5])
   cur2->write("BestCFit",v[1],0);
  cur2->increment("AvAge",cur->cal("Age",0));
  cur2->increment("AvTestDinner",cur->cal("TestDinner",0));
  if(v[1]>v[20])
   {v[20]=v[1]; //overall max fitness
    v[9]=cur->cal("IdAgent",0);
   }
 }

//Write the average rank
p->lsdqsort("Agent", "Fitness", "DOWN");
for(cur=p->search("Agent"),v[0]=1; cur!=NULL; v[0]++,cur=go_brother(cur) )
 {cur->write("Rank",v[0],0);
  v[1]=cur->cal("AgentType",0);
  cur1=p->search_var_cond("IdClass",v[1],0);
  cur1->increment("AvRank",v[0]);
 }
p->write("IdWinner",v[9],0);
//determine the final statics for the period
for(cur=p->search("Class"); cur!=NULL; cur=go_brother(cur) )
 {v[7]=cur->cal("NumClass",0);
  v[8]=cur->cal("AvClFitness",0);
  v[10]=cur->cal("VarClFitness",0);
  v[11]=cur->cal("AvOldFitness",0);
  v[9]=cur->cal("SucMut",0);
  if(v[9]>0)
    cur->write("AvGain",cur->cal("AvGain",0)/v[9],0);
  if(v[7]!=0)
   {cur->write("AvClFitness",v[8]/v[7],0);
    cur->write("AvOldFitness",v[11]/v[7],0);
    cur->write("SucMut",v[9]/v[7],0);
    cur->write("VarClFitness",(v[10]/v[7])-(v[8]/v[7])*(v[8]/v[7]), 0);
    cur->write("AvRank",cur->cal("AvRank",0)/v[7],0);
    cur->write("AvAge",cur->cal("AvAge",0)/v[7],0);
    cur->write("AvTestDinner",cur->cal("AvTestDinner",0)/v[7],0);
   }
 }
p->write("AvFitness",v[2]/v[3],0);
if(debug_flag==1 && debug=='d')
 deb(p, NULL, "Agents ranked by Fitness", &res);
p->lsdqsort("Agent", "IdAgent", "UP");
res=v[20];
goto end;
}

if(!strcmp(label,"NumAgent"))
{
/* 
Compute how many agents are present.
BEWARE. It is tranformed in a parameter because, at this time, the total number of agents never changes. Removes the line if used in a model where the number of agents do change.

*/ 
cur=p->search("Agent"); 
for(v[0]=0; cur!=NULL; cur=go_brother(cur) ) 
 v[0]++; 
res=v[0]; 
param=1; //remove here to execute this equation always.
goto end; 
}

/*************************************
SHAKE
*************************************/
if(!strcmp(label, "Shake") )
{
/*
Every PeriodShk moves each agent in a new randomly chosen point.
If parameter AllEqual is 1, then all agents move to the same point. Otherwise, each agent moves to a different point.
*/

v[0]=p->cal("PeriodShk",0);
v[1]=p->cal("CounterShk",0);

if(v[1]<v[0])
 {p->write("CounterShk",v[1]+1,0);
  res=0;
  goto end;
 }
p->cal("Genetic",0); //ensure that Genetic is done _before_ Shake
p->write("CounterShk",1,0);
cur=p->search("Agent");
v[0]=fl->cal("N",0);
v[6]=p->cal("AllEqual",0);

if(v[6]==1) //Any agents set to the same randomly chosen point
 {
  for(v[1]=0; v[1]<v[0]; v[1]++ )
   RND>0.5?str[(int)v[1]]=1:str[(int)v[1]]=0;
  v[4]=fl->cal("FitFun",0);
 }
for( ; cur!=NULL; cur=go_brother(cur) )
 {if(v[6]==0)
   {//AllEqual==0
    for(v[1]=0, cur1=cur->search("Block"); cur1!=NULL; cur1=go_brother(cur1) )
     {//set the new point
     for(cur2=cur1->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2), v[1]++)
      {
      RND>0.5?str[(int)v[1]]=1:str[(int)v[1]]=0;
      cur2->write("Value", (double)str[(int)v[1]],0);
      }
     }
    v[4]=fl->cal("FitFun",0);
    cur->write("Fitness",v[4],0);
    for(v[1]=0, cur1=cur->search("Block"); cur1!=NULL; cur1=go_brother(cur1) )
     {
     for(cur2=cur1->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2), v[1]++)
      cur2->write("FitnessContribution" ,fc[(int)v[1]].fitcontr,0);
     }

   }
  else
   {//AllEqual == 1
    for(v[1]=0, cur1=cur->search("Block"); cur1!=NULL; cur1=go_brother(cur1) )
     {
     for(cur2=cur1->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2), v[1]++)
      {
      cur2->write("Value", (double)str[(int)v[1]],0);
      cur2->write("FitnessContribution" ,fc[(int)v[1]].fitcontr,0);
      }
     }
    }
 cur->write("Fitness",v[4],0);

}

res=1; 
goto end;
}


/*************************************
GENETIC New
*************************************/
if(!strcmp(label, "Genetic") )
{
/*
This equation modifies the composition of the population of the Agents removing the worst performing with "copies" of the best ones.
The probability for an agent to be removed equals (1-Fitness)^ElasRemove, while the probability to replicate (i.e. produce a copy) is Fitness^ElasReplicate. 
Only the worst NumReplace Agents may be removed (the others are assigned null probabilities). Similarly, only NumParent Agent can replicate, being assigned positive probabilities. Therefore, the middle NumAgent-NumParent-NumReplicate cannot replicate nor die.

The equation is executed only once every PeriodGen time steps, and in these cases after the updating of the Fitness of each Agent. It is composed by the following steps:
1) Reset some Class statistics on replication
2) Sort agents by Fitness and assign the probs to die or replicate
3) For NumReplicate steps:
	a) draw one agent to kill and one to replicate
	b) destroy the agent to kill and create a new agent identical to the one to replicate
	c) reset the data of the new agent as necessary (e.g. Age, IdAgent, NumCopies, etc.)
	d) for the newly created agent set to 0 the probability to replicate or die
	e) with prob=ProbRandRelocationGenetic draw randomly a new point for the new agent, otherwise leave it in the same point as the replicated agent
	f) if SingleReplication==1 set to 0 the prob of the copied agent to replicate again

*/

v[0]=p->cal("PeriodGen",0);
v[1]=p->cal("CounterGen",0);
if(v[1]<v[0])
 {p->write("CounterGen",v[1]+1,0);
  res=0;
  goto end;
 }
v[30]=fl->cal("N",0);
p->write("CounterGen",1,0);
v[21]=p->cal("MaxFitness",0);// Ensures that the new fitness is computed

cur7=p->search("Class"); //reset these statistics in Class
for(cur=cur7; cur!=NULL; cur=go_brother(cur) )
   {cur->write("AvAgeDeath",0,0);
    cur->write("NClCopies",0,0);
    cur->write("NumDeath",0,0);
   }


//SORT AGENTS
p->lsdqsort("Agent", "Fitness", "UP");

v[0]=p->cal("NumReplace",0);
v[1]=p->cal("NumAgent",0);
v[22]=p->cal("NumParent",0);

v[4]=p->cal("ElasRemove",0);
v[5]=p->cal("ElasReplicate",0);


//assign the probabilities
for(cur=p->search("Agent"), v[2]=0; v[2]<v[1] ; v[2]++, cur=go_brother(cur))
 {
  if(v[2]<v[0])
   {//agents to die
    v[3]=cur->cal("Fitness",0);
    cur->write("ProbDie",pow(1-v[3],v[4]), 0); 
    cur->write("ProbReplicate",0, 0);
   }
  if(v[2]>=v[0] && v[2]<(v[1]-v[22]))
   {//agents nor dieing nor replicating
    cur->write("ProbDie",0, 0); 
    cur->write("ProbReplicate",0, 0);
    
   }
  if(v[2]>=(v[1]-v[22]) )
   {//Agents replicating
    v[3]=cur->cal("Fitness",0);
    cur->write("ProbDie",0, 0); 
    cur->write("ProbReplicate",pow(v[3],v[5]), 0);
   }
   
 }

v[32]=p->cal("ProbRandRelocationGenetic",0);
v[33]=p->cal("SingleReplication",0);

//if the agent replicating can do it only once, they cannot be less than the agents to replicate
if(v[33]==1 && v[0]>v[22])
 v[0]=v[22]; 
 
res=-2;
if(debug_flag==1 && debug=='d')
 deb(p, NULL, "BeforeReplication", &res);


for(v[2]=0; v[2]<v[0]; v[2]++)
 {
  cur=p->draw_rnd("Agent", "ProbDie", 0); //dying
  v[11]=cur->cal("AgentType",0);
  cur2=cur7->search_var_cond("IdClass",v[11],0);
  cur2->increment("NumClass",-1);
  cur2->increment("NumDeath",1);
  v[12]=cur->cal("DateBirth",0);
  cur2->increment("AvAgeDeath", +(double)t-v[12]);
  v[12]=cur->cal("IdAgent",0); //Identification number of the deleting agent

  cur2=p->draw_rnd("Agent", "ProbReplicate", 0); //replicating agent
  cur2->increment("NumCopies",1);
  cur->delete_obj(); //erase the agent. Now cur is free
  cur=p->search("Agent"); //find the set of agents down in the tree
  //cur=cur->up->add_an_object("Agent",cur2); //Add a new object copying the one to imitate
  cur=ADDOBJS_EX(cur->up,"Agent",cur2);
  v[11]=cur2->cal("IdAgent",0);
  cur->write("IdOrigin",v[11],0);
  cur->write("IdAgent",v[12],0); //Set the identification number of the replaced agent
  cur->write("NumCopies",0,0);
  cur->write("Age",0,0);
  cur->write("DateBirth",(double)t, 0);
  cur->write("CounterMutation",RND*cur2->cal("MaxBitLength",0), 0);
  for(cur5=cur->search("Block"); cur5!=NULL; cur5=go_brother(cur5) )
   cur5->write("NumMut",1, 0);
  v[11]=cur->cal("AgentType",0);
  cur3=cur7->search_var_cond("IdClass",v[11],0); //pointer to the newborn class 
  cur3->increment("NumClass",1);
  cur3->increment("NClCopies",1);
//Sets initial point
  if(RND<=v[32])
  {//set it randomly
  for(v[1]=0; v[1]<v[30]; v[1]++ )
    RND>0.5?str[(int)v[1]]=1:str[(int)v[1]]=0;
  v[6]=fl->cal("FitFun",0);   
  for(v[1]=0, cur9=cur->search("Block"); cur9!=NULL; cur9=go_brother(cur9) )
    {
     for(cur10=cur9->search("BitBlock"); cur10!=NULL; cur10=go_brother(cur10),v[1]++ )
       {
       RND>0.5?str[(int)v[1]]=1:str[(int)v[1]]=0;
       cur10->write("Value", (double)str[(int)v[1]],0);
       cur10->write("FitnessContribution", fc[(int)v[1]].fitcontr,0);
       } 
    }
  cur->write("Fitness",v[6],0);
  }
  else
  {// set the new point equal to the parent
   // DO NOTHING, since the copy already transfer all the necessary information
  }

 cur->write("ProbDie",0, 0);
 cur->write("ProbReplicate",0, 0);

 if(v[33]==1)
  cur2->write("ProbReplicate",0, 0);
 } //End for the agents to be replaced
  
  

//normalize the average age at death
for(cur=cur7; cur!=NULL; cur=go_brother(cur) )
 {v[20]=cur->cal("NumDeath",0);
  if(v[20]>0)
   cur->write("AvAgeDeath",cur->cal("AvAgeDeath",0)/v[20],0);
 }
res=-1;
if(debug_flag==1 && debug=='d')
 deb(p, NULL, "AfterReplication", &res);

p->lsdqsort("Agent", "IdAgent", "UP");
res=v[2];
goto end;
}


/***************************
INIT AGENT
***************************/
if(!strcmp(label, "InitAgent") )
{
/*

This equation is computed once at time 1 and never again. It creates a model structure automatically avoiding users to define each agent, type of reward, initial point and so on.

After this equation the model contains a group of agents equal to the sum of NumClass for each Class. Each agent is endowed with:
- Blocks of maximum dimension TypeNumBits from its own class (the last may possibly have less bits)
- the type of mutation RewardType equal to the one of the class TypeRewardClass
- a randomly chosen point. The point is identical for agents if AllEqual ==1, or different if ==0.
- an incremental integer in IdAgent.
- a random value for CounterMutation, so to trigger the mutations of agents smoothly

IMPORTANT:
before applying computing this equation the very first agent must contain one single block with one single bit block.
*/

v[3]=fl->cal("N",0);

v[22]=p->cal("AllEqual",0);
cur2=p->search("Class");
v[30]=cur2->cal("NumClass",0);
v[31]=cur2->cal("TypeRewardClass",0);
v[32]=1;
v[33]=cur2->cal("IdClass",0);

if(v[22]==1)
 {for(v[25]=0;v[25]<v[3]; v[25]++)
   str[(int)v[25]]=RND>0.5?1:0;
  v[29]=fl->cal("FitFun",0); 
 }  

plog("\nCreating agents ...");
for(v[21]=1; v[21]>0; v[21]++ )
{ //continue creating agents until v[21]>0
  if(v[21]==1)
    cur=p->search("Agent"); //agent to initialize (the existing one)
  else
    cur=ADDOBJ("Agent");
    //cur=p->add_an_object("Agent");//agent to initialize (a new one)
  cur->write("RewardType",v[31], 0);
  cur->write("IdAgent",v[21], 0);
  cur->write("AgentType",v[33], 0);
  v[1]=cur2->cal("TypeNumBits",0);
  if(v[1]>v[3]) //on hour lost ...
   {
    sprintf(msg, "\\nTypeNumBits must be <= than N.\\nClass %d is forced to replace %d with %d\\n", (int)v[33], (int)v[1], (int)v[3]);
    plog(msg);
    v[1]=v[3];
   }
  cur->write("MaxBitLength",v[1], 0);
  cur->write("CounterMutation",RND*v[1], 0);
  v[2]=ceil(v[3]/v[1]);
  cur->write("NumBlocks",v[2], 0);
  
  cur3=cur->search("Block");
  cur3->write("IdBlock",1, 0);
  cur3->write("NumBits",v[1], 0);
  if(v[22]==0)
   {for(v[25]=0;v[25]<v[3]; v[25]++)
     str[(int)v[25]]=RND>0.5?1:0;
    v[29]=fl->cal("FitFun",0); 
   }  
  v[25]=0;
  for(v[5]=2;v[5]<=v[2]; v[5]++)
   { //create all the blocks, each with only one BitBlock
    //cur5=cur->add_an_object("Block",cur3);
    cur5=ADDOBJS_EX(cur,"Block",cur3);
    cur5->write("IdBlock",v[5], 0);
    v[6]=min(v[1], v[3]-((v[5]-1)*v[1]));
    cur5->write("NumBits",v[6], 0);
   } 
  
  for(cur5=cur->search("Block"), v[4]=1; cur5!=NULL; cur5=go_brother(cur5) )
   {
   cur4=cur5->search("BitBlock");
   cur4->write("IdBitBlock",1,0);
   cur4->write("IdALocus",v[4]++,0);
   v[7]=cur5->cal("NumBits",0);
   cur4->write("Value",str[(int)v[25]++],0);
   
   for(v[6]=1; v[6]<v[7]; v[6]++)
    {//create the next BitBlock in the very first block
     //cur6=cur5->add_an_object("BitBlock",cur4);
     cur6=ADDOBJS_EX(cur5,"BitBlock",cur4);
     cur6->write("IdBitBlock",v[6]+1,0);
     cur6->write("IdALocus",v[4]++,0);
     cur6->write("Value",str[(int)v[25]++],0);
    }
   }
   
  for(v[8]=0,cur1=cur->search("Block"); cur1!=NULL; cur1=go_brother(cur1) )
   {
   for(cur6=cur1->search("BitBlock"); cur6!=NULL; cur6=go_brother(cur6) )
    cur6->write("FitnessContribution",fc[(int)v[8]++].fitcontr, 0);
   }
   cur->write("Fitness",v[29], 0); 
   
if(v[32]<v[30])   
 v[32]++;
else
 {
  cur2=go_brother(cur2); 
  if(cur2==NULL)
   v[21]=-1; //stop creating agents
  else
   {v[32]=1;
    v[30]=cur2->cal("NumClass",0);
    v[31]=cur2->cal("TypeRewardClass",0);
    v[33]=cur2->cal("IdClass",0);
   } 
     
 } 
 
 
}
plog(" done\n");
param=1;
res=1;
goto end;
}


if(!strcmp(label, "CreateFitContrib") )
{
/*
Variable in Landscape that creates the landscape values.
Initialize the vectors used for computating and storing the landscape values.
*/

v[0]=p->cal("N",0);
str=new unsigned int[(int)v[0]]; //temporary strings, used to store the binary point
str2=new unsigned int[(int)v[0]]; //numbers
str3=new unsigned int[(int)v[0]]; //numbers
str4=new unsigned int[(int)v[0]]; //numbers

fc=new bit[(int)v[0]]; //memory structure used to store landscape. See FitFun comments


for(v[1]=0,cur=p->search("LLocus"); cur!=NULL; cur=go_brother(cur), v[1]++ )
 {//store the epistatic links in the fc
  v[0]=cur->cal("NLink",0);
  fc[(int)v[1]].nlink=(int)v[0];
  fc[(int)v[1]].link=new int[(int)v[0]];
  for(i=0,cur1=cur->search("Link"); cur1!=NULL; cur1=go_brother(cur1),i++ )
    fc[(int)v[1]].link[i]=(int)cur1->cal("IdLink",0)-1;
  fc[(int)v[1]].l.l0=NULL;
  fc[(int)v[1]].l.l1=NULL;
 } 
 

res=1;
param=1;
goto end;
}

/********************************
InitEvenK
/********************************/
if(!strcmp(label, "InitEvenK") )
{
/*
Creation of the landscape structure, created once and never again. It is used to form the memory structure meant to keep the landscape data (see eq. for FitFun)
The  equation reads the desired N, EvenK, AftOverlap and ForeOverlap then creates a set of 
LLocus (N-1 because one exists from the beginning). 
For each of the LLocus, the equation creates groups of EvenK + AftOverlap + ForeOverlap links, initialized so that each group refers to the same set loci.

EvenK is the number of loci that each bit is reciprocally connected to. For example, if EvenK is 2, then the loci will be grouped as:
{1,2,3}; {4,5,6}; {7,8,9}; ...
That is, e.g., the 4th, 5th and 6th elements are reciprocally connected. The dimension of the blocks is therefore EvenK+1 (the last block may be smaller if N/(EvenK+1) is not an integer).

AftOverlap indicates how many loci contiguous and consecutive are to be added unilaterally to the blocks of EvenK+1 elements. If AftOverlap is 1, then all the loci in the block {1,2,3} from the previous example are also connected to 4 (but 4th locus is not connected to 1st, 2nd or 3rd). Elements in the last block set links with the very first bits.

ForeOverlap is like AftOverlap but links unilaterally the blocks with the loci before the blocks.

Probably the best way to understand how the equation works is to observe the model structure after InitEvenK has created it. To do this simply make the debugger start at the very first time step and move down. LLocus's are the Objects representing loci and link's are the Object containing the id of the loci connected to it.

*/
v[7]=p->cal("N",0);
v[0]=p->cal("EvenK",0)+1;
v[11]=p->cal("AftOverlap",0);
v[12]=p->cal("ForeOverlap",0);
if(v[7]<v[0]+v[11]+v[12])
 {sprintf(msg,"\nWarning: you set an unsensible parameterisation.\nN = %d, EvenK = %d, AftOverlap = %d, ForeOverlap = %d!\nThe setting must be such that EvenK+AftOveral+ForeOverlap<N\n", (int)v[7],(int)v[0],(int)v[11],(int)v[12]);
  plog(msg);
  plog("\nParameters forced to: EvenK=0, AftOverlap=0, ForeOverlap=0\n");
  v[0]=1;
  v[11]=0;
  v[12]=0;
 }
cur=p->search("LLocus");
//Create the missing LLocus'
t=0; //WARNING: hack to prevent makestr2 to be initialized at the wrong time
for(v[1]=1; v[1]<v[7] ; v[1]++)
  ADDOBJ_EX("LLocus",cur);
  //p->add_an_object("LLocus",cur);
t=1; //restore the actual time
//For each Locus
for(v[3]=v[0], v[1]=1,cur=p->search("LLocus"); cur!=NULL; cur=go_brother(cur), v[3]++, v[1]++ )
 {
  //Assign the Id
  cur->write("IdLocus",v[1],0);
  //determine the initial Locus from which it depends upon
  if(v[3]==v[0]) //the counter reached the number of links
   {v[4]=v[2]=v[1]; //set the initial locus of the dependents to the Locus itself
    v[3]=0; //reset the counter
   }
  else
   v[2]=v[4]; //set the initial locus of dependents to the one used for the previous Locus

//Create the necessary Link's (one already exists), and initialize it
cur1=cur->search("Link");
cur1->write("IdLink",v[2]++,0);
  for(v[10]=1; v[10]<v[0] && v[2]<=v[7]; v[10]++, v[2]++)
    {//cur1=cur->add_an_object("Link",cur1);
     cur1=ADDOBJS_EX(cur,"Link",cur1);
     cur1->write("IdLink",v[2],0);
    }
//Create the the aft-posted overlapping links, using the initial one when
//N is reached
  for(v[13]=0; v[13]<v[11] ; v[13]++, v[10]++, v[2]++)
    {if(v[2]>v[7])
      v[2]=1;
     //cur1=cur->add_an_object("Link",cur1);
     cur1=ADDOBJS_EX(cur,"Link",cur1);
     cur1->write("IdLink",v[2],0);
    }
//Create the the fore-posted overlapping links, using the last ones when
//the first is involved
v[2]=v[4]-1;
  for(v[13]=0; v[13]<v[12] ; v[13]++, v[10]++, v[2]--)
    {if(v[2]<1)
      v[2]=v[7];
     //cur1=cur->add_an_object("Link",cur1);
     cur1=ADDOBJS_EX(cur,"Link",cur1);
     cur1->write("IdLink",v[2],0);
    }

  cur->write("NLink",v[10],0);
 }
res=1;
param=1;
goto end;
}

/*****************************
Init
******************************/
if(!strcmp(label, "Init") )
{
/*
Equation setting the basic initializations. Computed only once, it transformed
itself in parameter and is not computed again.
*/
/*
assign the Landscape object to a specific pointer to speed up calls to the landscape
*/
fl=p->search("Landscape"); 
								
fl->cal("InitEvenK",0); //initialize the landscape structure
fl->cal("CreateFitContrib",0); //create the memory location for the landscape data
fl->cal("SaveLandscape",0); //if requested save the landscape data
for(cur=p->search("Population"); cur!=NULL; cur=go_brother(cur))
  cur->cal("InitAgent",0);
param=1;
res=0;
goto end;
}

/**********************************
SaveLandscape
***********************************/
if(!strcmp(label, "SaveLandscape") )
{
/*
Save in a file the landscape's fitness values. 
Beware that the lines are equal to 2^N. Better use for small N only.

*/
v[0]=p->cal("FlagSaveLandscape",0);
if(v[0]!=1)
 {param=1;
 res=0;
 goto end;
 }

v[0]=fl->cal("N",0);
v[1]=pow(2,v[0]);
v[6]=fl->cal("EvenK",0)+1;

sprintf(msg,"n%d_k%d.txt",(int)v[0], (int)v[6]);
f=fopen(msg,"w");

for(v[3]=0; v[3]<v[1] ;v[3]++)
 {int2bin(v[3],v[0],str);
  v[5]=fl->cal("FitFun",0);
  fprintf(f,"%d\t%.10lf\t",(int)v[3],v[5]);
  for(v[7]=0; v[7]<v[0]; v[7]++)
   fprintf(f,"%u",str[(int)v[7]]);
  fprintf(f,"\n"); 
 }
 
fclose(f);

param=1;
res=1;
goto end;
}




if(!strcmp(label,"BackDoor"))
{
/*
This equation permit to force an agent to occupy a given point. It is used only for control, or for forcing agents to move to a different place.

To operate BackDoor it is necessary to set BackDoor to a value different from 0. At the next time step the equation open the file "string.txt" that must contain the binary version of the point and nothing else. The equation replace the current point with the one in the file and computes its fitness.
*/
if(val[0]==0)
 {res=0;
  goto end;
 }
f=fopen("string.txt","r");
fscanf(f, "%s",msg);
fclose(f);
i=0;
for(cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1) )
    {cur1->write("Value",msg[i]=='1'?1:0, 0);
     str[i]=msg[i++]=='1'?1:0;
    } 
 }
i=0;
v[0]=fl->cal("FitFun",0);
for(cur=p->search("Block"); cur!=NULL; cur=go_brother(cur) )
 {
  for(cur1=cur->search("BitBlock"); cur1!=NULL; cur1=go_brother(cur1) )
    cur1->write("FitnessContribution",fc[i++].fitcontr,0); 
 }   

p->write("Fitness",v[0], 0);
res=0;
goto end;
}

if(!strcmp(label,"NumForms"))
{
/*
Compute how many different points are occupied by the agents. If NumForms equal 1, it means that all agents in the population stay in the same point.

It computes first the number of different points for agents in the classes separatedly and then the number of forms in the whole population.
*/
p->cal("MaxFitness",0);
p->lsdqsort("Agent","AgentType", "Fitness", "UP"); //rank agents first for Class and then for Fitness
v[0]=p->cal("AgentType",0);
v[2]=1; //counter of fitnesss
v[4]=p->cal("Fitness",0);
for(cur=p->search("Agent"); cur!=NULL; cur=go_brother(cur) )
 {//for each agent
  v[1]=cur->cal("AgentType",0);
  if(v[1]==v[0])
   {//same class
    v[3]=cur->cal("Fitness",0);
    if(v[3]!=v[4])
     { v[2]++; //new form
       v[4]=v[3];
     }  
   }
  else
   {//change class
   cur1=p->search_var_cond("IdClass",v[0],0);
   cur1->write("NumFormsClass",v[2], 0); //store the values of numforms
   v[2]=1;
   v[4]=cur->cal("Fitness",0);
   v[0]=v[1];
   } 
 }

cur1=p->search_var_cond("IdClass",v[1],0); //for the last class
cur1->write("NumFormsClass",v[2], 0); //store the values of numforms

p->lsdqsort("Agent", "Fitness", "UP"); 
v[2]=1;
v[4]=p->cal("Fitness",0);

for(cur=p->search("Agent"); cur!=NULL; cur=go_brother(cur) )
 {//for each agent
    v[3]=cur->cal("Fitness",0);
    if(v[3]!=v[4])
     { v[2]++; //new form
       v[4]=v[3];
     }  
   }
p->lsdqsort("Agent", "IdAgent", "UP"); 

res=v[2];
goto end;
}

/*************************************
SHIFT
*************************************/
if(!strcmp(label, "Shift") )
{
/*
Every PeriodShift steps the fitness contributions are shifted. That is, one of the fitness contributions is changed
*/

v[0]=p->cal("PeriodShift",0);
v[1]=p->cal("CounterShift",0);

if(v[1]<v[0])
 {p->write("CounterShift",v[1]+1,0);
  res=0;
  goto end;
 }
p->write("CounterShift",1,0);
cur=p->search("Agent");
v[0]=fl->cal("N",0);
v[10]=rnd_integer(0,v[0]-1);
shift(&fc[(int)v[10]].l);

//for each agent
for( ; cur!=NULL; cur=go_brother(cur) )
 {  for(v[1]=0, cur1=cur->search("Block"); cur1!=NULL; cur1=go_brother(cur1) )
     {//load the point of the agent
     for(cur2=cur1->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2), v[1]++)
      {
      str[(int)v[1]]=(int)cur2->cal("Value" ,0);
      }
     }
 v[4]=fl->cal("FitFun", 0); //compute the (shifted) fitness
 cur->write("Fitness",v[4],0);
   for(v[1]=0, cur1=cur->search("Block"); cur1!=NULL; cur1=go_brother(cur1) )
     {//assign the new fitness contributions
     for(cur2=cur1->search("BitBlock"); cur2!=NULL; cur2=go_brother(cur2), v[1]++)
      {
      cur2->write("FitnessContribution" ,fc[(int)v[1]].fitcontr,0);
      }
     }
 }

res=1; 
goto end;
}



sprintf(msg, "\nError 04: Function for %s not found", label);
plog(msg);
quit=2;
return -1;

end :
if( (isnan(res)==1 || isinf(res)==1) && quit!=1)
 { 
  sprintf(msg, "At time %d the equation for '%s' produces the non-valid value '%lf'. Check the equation code and the temporary values v\\[...\\] to find the faulty line.",t, label, res );
  error(msg);

  debug_flag=1;
  debug='d';
 } 

if(debug_flag==1 || quit==2)
 {
 for(i=0; i<40; i++)
  i_values[i]=v[i];
 }
return(res);
}

double power(double a, double b)
{
if( a<=0)
 {sprintf(msg,"\na=%lf/tb=%lf", a, b);
  plog(msg);
  quit=2;
 return 0;
 }
return(pow(a,b));
}


void freememuniversal(dynlink l)
{
if(l.l0!=NULL)
 {freememuniversal(*l.l0);
  delete l.l0;
 } 
 
if(l.l1!=NULL)
 {freememuniversal(*l.l1);
  delete l.l1;
 } 

}
//This is a C++ function executed once, and only once, after the end
//of a simulation. The purpose is, like in this case, you need to make
//some cleaning before exiting

void cmd(char *s);
void close_sim(void)
{
//removes the memory requested in the initializatin of the simulation in
//CreateFitContrib
double n;
int i;

delete str2;
delete str3;
delete str4;
delete str;


n=fl->cal("N",0);
for(i=0; i<(int)n; i++)
 {delete fc[i].link;
  sprintf(msg, "Memory bit %d (of %d) freed (press Stop to interrupt)\n",i+1, (int)n);
  plog(msg);
  cmd("if {$done_in == 1} {set debug_flag -1} {}");
  if(debug_flag==-1)
   {
    debug_flag=0;
    break;
   }
  freememuniversal(fc[i].l);
  fc[i].l.l0=NULL;
  fc[i].l.l1=NULL;
  
 }  

delete fc;

}
/*
void int2bin(double in, double n, unsigned int *s)
{
//REVOLTING
double k;
for(k=n; k>0; k--)
 {
  if(pow(2,k-1)<=in)
   {s[(int)k-1]=1;
    in-=pow(2,k-1);
   }
  else
   s[(int)k-1]=0;
 }
}

double bin2int(unsigned int *s2, double n)
{
//REVOLTING
double i, res;
for(res=0,i=n; i>0; i--)
 {
 if(s2[(int)(i-1)]==1)
   res+=pow(2,i-1);
 }
return res;
}

*/

void int2bin(double in, double n, unsigned int *s)
{
double k;
for(k=n; k>0; k--)
 {
  if(pow(2,k-1)<=in)
   {s[(int)(n-k)]=1;
    in-=pow(2,k-1);
   }
  else
   s[(int)(n-k)]=0;
 }
}

double bin2int(unsigned int *s2, double n)
{
double i, res;
for(res=0,i=n; i>0; i--)
 {
 if(s2[(int)(n-i)]==1)
   res+=pow(2,i-1);
 }
return res;
}

void loaddatauniv(bit *b, int n, int k, FILE *f)
{
int i, j, h, m, app;
dynlink *c;
double d;
int *s;
unsigned int *s1;

s=new int[n];
s1=new unsigned int[k];
m=(int)pow(2,(double)k);
for(j=0; j<m; j++)
 {
  int2bin((double)j,(double)k, s1);
  for(i=0; i<n; i++)
    {c=&b[i].l;
     for(h=0;h<k; h++)
      {
      if(s1[h]==0)
       {
         if(c->l0==NULL)
         {
         c->l0=new dynlink;
         c=c->l0;
         c->l0=NULL;
         c->l1=NULL;
         }
        else
         c=c->l0;
       }
      else
       {
        if(c->l1==NULL)
         {
         c->l1=new dynlink;
         c=c->l1;
         c->l0=NULL;
         c->l1=NULL;
         }
        else
         c=c->l1; 
       }

      }
     fscanf(f,"%lf",&d);
     c->f=new double;
     *c->f=d;
    }
 }
}


void shift(struct dynlink *c)
{
if(c->l0==NULL && c->l1==NULL)
 *c->f=RND;
else
 {
 if(c->l0!=NULL)
  shift(c->l0);
 if(c->l1!=NULL)
  shift(c->l1);
  
 } 

}



