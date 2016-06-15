/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/


/*******************************************
****************************************************
DOES NOT WORK
This is an attempt to speed up the loading process for large configuration files.

Standard system re-opens and re-scans the file for every element, which is not that practical
for large files. In this system the file is scanned only once reading sequentially the data for the elements

The problem lies in the latest updates after the description, which can be absent.

To make it work you need to replace load_param to accept also a file. Changes are required to:
- decl.h
- lsdmain.cpp
- interf.cpp

FIXED
Loading files strictly sequentially, stopping and automatically completing the necessary info for compatibility.
****************************************************/



/****************************************************
FILE.CPP
Contains the methods and functions used to save and load a model. The
data for a model are composed by a structure, initial values and simulation
setting.

The method of object contained in this file are:

- void object::save_struct(FILE *f, char *tab)
Saves the structure of the object (that is, the label,
variables and parameters and descendants, not number of objects).
This method is called first time by the browser in INTERF.CPP by the root of the
model.
Calls the save_struct for all the descendant type.

- void object::save_param(FILE *f)
Save the numerical values for the object (one digit
for each group of object of this type) and the initial values for variables.
It save also option information, that is whether to save, plot or debug the
variables.
It calls the save_param for all the descendant type.
It is called in the browser, INTERF.CPP, immediately after save_struct, by the
root of the model.

- void object::load_struct(FILE *f)
Initialize a model by creating  one as defined
in the data file. The model, after this stage, has only one instance for each
object type and variables and parameters are simply labels.

- int object::load_param(char *file_name, int repl)
It loads from the file named as specified the data
for the object. It is made in specular way in respect of save_param.
Called from browser in INTERF.CPP immediately after load_struct.

- void object::replicate(int num, int propagate)
Counts the number of instances (this included) of this type of object and
make as many copy as necessary to have num of them. In case propagate is 1,
it sends the replicate message to the hypernext, first. It should be applied
to a structure model, that is, where the object is there are no more than
one instance in any group.


- void copy_descendant(object *from, object *to)
This is a function called by replicate above, so that the replicated object
are initialized also the descendants.

-object *create(Tcl_Interp *inter, object *root)
The main cycle for the Browser, from which it exits only to run a simulation
or to quit the program. The cycle is just once call to browsw followed by
a call to operate.

- int browse(Tcl_Interp *inter, object *r, int *choice);
build the browser window and waits for an action (on the form of
values for choice or choice_g different from 0)

- object *operate(Tcl_Interp *in, int *choice, object *r);
takes the value of choice and operate the relative command on the
object r. See the switch for the complete list of the available commands

- void clean_debug(object *n);
remove all the flags to debug from any variable in the model

- void clean_save(object *n);
remove all the flags to save from any variable in the model

- void clean_plot(object *n);
remove all the flags to plot from any variable in the model


Functions used here from other files are:

- void plog(char *m);
LSDMAIN.CPP print  message string m in the Log screen.

- void analysis(int *choice);
ANALYSIS.CPP analysis of result files

- object *skip_next_obj(object *t, int *i);
UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- int my_strcmp(char *a, char *b);
UTIL.CPP It is a normal strcmp, but it catches the possibility of both strings being
NULL

- void cmd(Tcl_Interp *inter, char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

- object *go_brother(object *cur);
UTIL.CPP returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.

- void show_graph( object *t);
DRAW.CPP shows the grsphical representation of the model

- void set_obj_number(object *r, int *choice);
EDIT.CPP allows to edit the number of instances in the model

- void edit_data(object *root, int *choice, char *obj_name);
EDIT_DAT.CPP allows to edit the initial values

- FILE *search_str(char *name, char *str);
UTIL.CPP given a string name, returns the file corresponding to name, and the current
position of the file is just after str.

- FILE *search_data_str(char *name, char *init, char *str)
UTIL.CPP given a string name, returns the file with that name and the current position
placed immediately after the string str found after the string init. Needed to
not get confused managing the data files, where the same string appears twice,
in the structure definition and in the data section.

- FILE *search_data_ent(char *name, variable *v)
UTIL.CPP given the file name name, the routine searches for the data line for the variable
(or parameter) v. It is not messed up by same labels for variables and objects.



****************************************************/

#include "choose.h"

#include "decl.h"


object *skip_next_obj(object *t, int *count);
object *skip_next_obj(object *t);
object *go_brother(object *t);
void copy_descendant(object *from, object *to);
void load_description(char *label, char *type);

void kill_trailing_newline(char *s);
FILE *search_str(char *name, char *str);

void execmd(char *);
FILE *search_data_str(char const *name, char const *init, char const *str);
FILE *search_data_ent(char *name, variable *v);
void find_lags(object *r);
void plog(char const *msg);
void save_eqfile(FILE *f);
void set_blueprint(object *container, object *r);
void add_description(char const *lab, char const *type, char const *text);
void empty_cemetery(void);
void empty_descr(void);

extern char msg[];
extern char name_rep[];
extern char lsd_eq_file[];
extern char *path;
extern char *simul_name;
extern char *equation_name;
extern char *eq_file;
extern char *struct_file;
extern int struct_loaded;
extern int t;
extern int seed;
extern int sim_num;
extern int max_step;
extern int ignore_eq_file;
extern bool unsavedChange;
extern object *blueprint;
       
extern description *descr;
void recur_description(object *r, FILE *f);
description *search_description(char *lab);

int inbp;// global variable signaling whether it is exploring the blueprint (=1) or the model (=0)

fpos_t *fpos=NULL;

/****************************************************
OBJECT::SAVE_STRUCT

****************************************************/

void object::save_struct(FILE *f, char const *tab)
{
object *o, *o1;
bridge *cb, *cb1;
variable *var, *c_var;
int count=0, i, j;
char tab1[30];

if(up==NULL)
  {
  
  fprintf(f, "\t\n");
  }

strcpy(tab1, tab);

fprintf(f, "%sLabel %s\n%s{\n",tab1, label, tab1);
strcat(tab1, "\t");
if(b!=NULL)
  cb1=b->next;

for(cb=b; cb!=NULL;cb=cb->next )
 {
 fprintf(f,"%sSon: %s\n",tab1,cb->blabel);
 if(cb->head==NULL)
  {
   o=blueprint->search(cb->blabel);
  }
 else
  {
   o=cb->head;
  }  
 o->save_struct(f,tab1);
 
 }


for(var=v; var!=NULL; var=var->next)
 {if(var->param==0)
   fprintf(f,"%sVar: %s\n",tab1, var->label);
  if(var->param==1)
    fprintf(f,"%sParam: %s\n",tab1, var->label);
  if(var->param==2)
    fprintf(f,"%sFunc: %s\n",tab1, var->label);
    
 }   
 
 fprintf(f,"\n");

fprintf(f,"%s}\n\n", tab);


} 


/****************************************************
OBJECT::SAVE_PARAM

****************************************************/

void object::save_param(FILE *f)
{

int i, count;
object *cur;
variable *cv, *cv1;
char ch,ch1;
description *cur_descr;
bridge *cb;

count=0;
fprintf(f, "\nObject: %s", label);
if(to_compute==1)
  fprintf(f, " C");
else
  fprintf(f, " N");

for(cur=this;cur!=NULL; )
 {skip_next_obj(cur, &count);
  fprintf(f, "\t%d", count);  
  for( ;go_brother(cur)!=NULL; cur=cur->next);

  cur=cur->hyper_next(cur->label);

 }
fprintf(f, "\n");

for(cv=v; cv!=NULL; cv=cv->next)
{
  //search for unloaded data
  ch='+';
  if(cv->param==1 || cv->num_lag>0)
  {
  for(cur=this; cur!=NULL; cur=cur->hyper_next(label))
    {cv1=cur->search_var(NULL, cv->label);
     if(cv1->data_loaded=='-')
      {ch='-';
       break;
      }
    }
  }
  else
   {//avoid marking as to initialize for elements not worth it
    cur_descr = search_description(cv->label);
    cur_descr->initial='n';
   }

 if(cv->save==true && cv->savei==false)
  ch1='s';
 if(cv->save==false && cv->savei==false)
  ch1='n';
 if(cv->save==true && cv->savei==true)
  ch1='S';
 if(cv->save==false && cv->savei==true)
  ch1='N';
  

 if(cv->param==0)
	fprintf(f, "Var: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch, cv->debug,cv->plot==1?'p':'n');
 if(cv->param==1)
	fprintf(f, "Param: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch, cv->debug,cv->plot==1?'p':'n');
 if(cv->param==2)
	fprintf(f, "Func: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch, cv->debug,cv->plot==1?'p':'n');

 for(cur=this; cur!=NULL; cur=cur->hyper_next(label))
   {cv1=cur->search_var(NULL, cv->label);
    if(cv1->param==1)
      {if(cv1->data_loaded=='+')
        fprintf(f, "\t%g", cv1->val[0]);
       else
        fprintf(f, "\t%c", '0');
       }
     else
      {
      for(i=0; i<cv->num_lag; i++)
		  {

         if(cv1->data_loaded=='+')
          fprintf(f, "\t%g", cv1->val[i]);
         else
			 fprintf(f, "\t%c", '0');
        }  
      }  
   }
 fprintf(f, "\n");
}
for(cb=b; cb!=NULL; cb=cb->next)
{
  cur=cb->head;
  cur->save_param(f);
 
}
}

/****************************************************
OBJECT::LOAD_PARAM

****************************************************/

int object::load_param(char *file_name, int repl, FILE *f)
{

char str[100], ch, ch1, ch2;
int num, i;
object *cur;
variable *cv, *cv1;
double app;
bridge *cb;

//printf("\nLoading Data for Object %s",label);

if(f==NULL)
 f=search_data_str(file_name, "DATA", label); 
else
 {
  fscanf(f, "%s", str); //skip the 'Object: '
  fscanf(f, " %s", str); //skip the 'label'  
 } 

if(f==NULL)
 { sprintf(msg,"\nError: search of data for %s in file %s failed\n", label, file_name);
	plog(msg);
	return 1;
 }
else
{
if(fscanf(f, " %c", &ch)!=1)
	{sprintf(msg, "\nError: data for object %s not found", label);
	 plog(msg);
	 return 1;
   }
if(ch=='C')
 to_compute=1;
else
 to_compute=0;
for(cur=this;cur!=NULL; )
 {if(fscanf(f, "\t%d", &num)!=1)
	{sprintf(msg, "\nError: data for object %s not found", label);
	 plog(msg);
	 return 1;
   }
  cur->to_compute=to_compute;
  cur->replicate(num, 0);

  for( ;go_brother(cur)!=NULL; cur=cur->next);
  cur=cur->hyper_next(cur->label);
 }
 //fclose(f);

for(cv=v; cv!=NULL; cv=cv->next)
 {
  //f=search_data_ent(file_name,cv);
  fscanf(f, "%s ", str); //skip the 'Element: '
  fscanf(f, "%s ", str); //skip the 'label'
  if(f==NULL)
  { sprintf(msg, "\nError: search of data for %s in file %s failed\n", cv->label, file_name);
	 plog(msg);
	 return 1;
  }

  if(fscanf(f, "%d %c %c %c %c", &(cv->num_lag), &ch1, &ch, &(cv->debug), &ch2   )!=5)
	 {sprintf(msg, "\nError: values not found for %s\n", cv->label);
	  plog(msg);
	 }

  if(ch1=='s')
	{
     cv->save=true;
     cv->savei=false;
    }
  if(ch1=='n')
	{
     cv->save=false;
     cv->savei=false;
    }
  if(ch1=='S')
	{
     cv->save=true;
     cv->savei=true;
    }
  if(ch1=='N')
	{
     cv->save=false;
     cv->savei=true;
    }
     

  if(ch2=='p')
	cv->plot=1;
  else
	cv->plot=0;

  for(cur=this; cur!=NULL; repl==1?cur=cur->hyper_next(label):cur=NULL)
	{cv1=cur->search_var(NULL, cv->label);
	 cv1->val=new double[cv->num_lag+1];

	 cv1->num_lag=cv->num_lag;
	 cv1->save=cv->save;
     cv1->savei=cv->savei;
	 cv1->plot=cv->plot;
	 cv1->param=cv->param;
	 cv1->debug=cv->debug;
	 cv1->data_loaded=ch;
	 if(cv1->param==1)
	  {if(fscanf(f, "%lf", &app )!=1)
			 {sprintf(msg,"\nError: value not found for parameter: %s\n", cv1->label);
			  plog(msg);
			  return 1;
			 }
			else
			 cv1->val[0]=app;
	  }
	 else
	 {for(i=0; i<cv->num_lag; i++)
		{
		  if ( ( num = fscanf(f, "\t%lf", &app ) ) != 1 )
			{sprintf(msg, "\nError: value not found for variable: %s\n", cv1->label);
			  plog(msg);
			  return 1;
			}
			else // Places values shifted one position, since they are "time 0" values
			 cv1->val[i]=app;
		}
	  cv1->val[cv->num_lag]=0;
	 }
	}
  //fclose(f);
 }
}
//for(cur=son, num=0; cur!=NULL; cur=skip_next_obj(cur))
for(cb=b; cb!=NULL; cb=cb->next)
 {
  cur=cb->head;
  i=cur->load_param(file_name, repl, f);
  if(i==1)
	return 1;
  num=0;
 }

 
if(up==NULL)
 {//this is the root, and therefore the end of the loading
 
  set_blueprint(blueprint, this);
 
 }
 
return 0;
 
}


/****************************************************
OBJECT::REPLICATE

****************************************************/

void object::replicate(int num, int propagate)
{
object *cur, *app, *cur1, *app1, *cur2;
variable *cv;
int i, usl;
bridge *cb;

if(propagate==1)
  cur=hyper_next(label);
else
  cur=NULL;
if(cur!=NULL)
 cur->replicate(num, 1);
skip_next_obj(this, &usl);
for(cur=this, i=1; i<usl; cur=cur->next, i++);

for(i=usl; i<num; i++)
 {app=cur->next;
  cur->next=new object;
  cur->next->init(up, label);
  cur->next->to_compute=to_compute;
  cur->next->next=app;
  cur->to_compute=to_compute;
  app=cur->next;
  for(cv=v; cv!=NULL; cv=cv->next)
	app->add_var_from_example(cv);

  copy_descendant(this, app);


 }

}


/****************************************************
COPY_DESCENDANT

****************************************************/
void copy_descendant(object *from, object *to)
{
object *cur, *app;
bridge *cb, *cb1;
variable *cv;
int count;

if(from->b==NULL)
 {to->b=NULL;
  return;
 }
to->b=new bridge;
to->b->blabel=new char[strlen(from->b->blabel)+1];
strcpy(to->b->blabel, from->b->blabel);
to->b->counter_updated=false;
to->b->mn=NULL;

to->b->head=new object;
(to->b->head)->init(to, (from->b->head)->label);
for(cv=(from->b->head)->v; cv!=NULL; cv=cv->next)
  (to->b->head)->add_var_from_example(cv);
to->b->head->to_compute=from->b->head->to_compute;

copy_descendant(from->b->head, to->b->head);
to->b->next=NULL;
cb=to->b;

//for(cur=skip_next_obj(from->son, &count), app=to->son; cur!=NULL; cur=skip_next_obj(cur, &count), app=app->next)
for(cb1=from->b->next; cb1!=NULL; cb1=cb1->next)
  { 
    cb->next=new bridge;
    cb=cb->next;
    cb->next=NULL;
    cb->mn=NULL;
    cb->blabel=new char[strlen(cb1->blabel)+1];
    strcpy(cb->blabel, cb1->blabel);
    cb->counter_updated=false;
    
	  cb->head=new object;    
	  cb->head->init(to, cb1->head->label);
	  cb->head->next=NULL;
    cb->head->to_compute=cb1->head->to_compute;
	 for(cv=cb1->head->v; cv!=NULL; cv=cv->next)
	  cb->head->add_var_from_example(cv);
	 copy_descendant(cb1->head, cb->head);
  }

}


/****************************************************
OBJECT::LOAD_STRUCT

****************************************************/

void object::load_struct(FILE *f)
{
char ch[100];
int len;
object *cur;
bridge *cb;
variable *cv;

if(up==NULL)
 fpos=NULL;

//fscanf(f, "%*[{\t\n]Label %s", ch);
fscanf(f,"%s",ch);
while(strcmp(ch,"Label"))
 fscanf(f,"%s",ch);

fscanf(f,"%s",ch);
len=strlen(ch);
if(label==NULL)
  {label=new char[len+1];
	strcpy(label, ch);
  }
fscanf(f, "%*[{\r\t\n]%s", ch);
while(strcmp(ch, "}"))
{
 if(!strcmp(ch, "Son:"))
   { fscanf(f, "%*[ ]%s", ch);
     add_obj(ch, 1, 0);
   //load_description(ch, "Object");
   //for(cur=son; strcmp(ch, cur->label); cur=cur->next);
   for(cb=b; strcmp(cb->blabel, ch); cb=cb->next);
   cur=cb->head;
   cur->load_struct(f);
   }
 if(!strcmp(ch, "Var:"))
   { fscanf(f, "%*[ ]%s", ch);
     add_empty_var(ch);
   //  load_description(ch, "Variable");
     sprintf(msg, "lappend ModElem %s",ch);
     execmd(msg);
   }
  if(!strcmp(ch, "Param:"))
	{ fscanf(f, "%*[ ]%s", ch);
     cv=add_empty_var(ch);
     cv->param=1;
     //load_description(ch, "Parameter");
     sprintf(msg, "lappend ModElem %s",ch);
     execmd( msg);
     
   }
  if(!strcmp(ch, "Func:"))
	{ fscanf(f, "%*[ ]%s", ch);
     cv=add_empty_var(ch);
     cv->param=2;
     //load_description(ch, "Function");
     sprintf(msg, "lappend ModElem %s",ch);
     execmd(msg);

   }

 fscanf(f, "%*[{\r\t\n]%s", ch);
}


}


/********************************************
Experimental code. Not used
void find_lags(object *r)
{
FILE *f;
variable *cv;
char ch[50];
int res, l=-1, found;

for(cv=r->v; cv!=NULL; cv=cv->next)
{
 if(cv->param==0)
 {
 strcpy(ch, "cal(\"");
 strcat(ch, cv->label);
 strcat(ch, "\"");
 f=search_str("fun.cpp", ch);
 while(f!=NULL)
  {res=fscanf(f, "%*[, ]%d", &found);
   if(res==1)
	 {if(l<found)
		l=found;
	 }
	fclose(f);
	f=search_str("fun.cpp", ch);
  }
 cv->num_lag=l;
 }
}
}

****************************************/

int object::read_param(char *file_name)
{
FILE *f=NULL;
char str[40], ch, ch1, ch2;
int num, i;
object *cur;
variable *cv, *cv1;
double app;
bridge *cb;

for(cv=v; cv!=NULL; cv=cv->next)
 {
  f=search_data_ent(file_name,  cv);
  if(f==NULL)
  { sprintf(msg,"\nError: search of data for %s in file %s failed\n", cv->label, file_name);
	 plog(msg);
	 return 1;
  }

  if(fscanf(f, "%d %c %c %c %c", &(cv->num_lag), &ch1, &ch, &(cv->debug), &ch2 )!=5)
	 {sprintf(msg,"\nError: values not found for %s\n", cv->label);
	 plog(msg);
	 return 1;
	 }
  if(ch1=='s')
	cv->save=1;
  else
	cv->save=0;

  cv->debug='n';
  cv->data_loaded='+';
  cv->val=new double[cv->num_lag+1];
  if(cv->param==1)
   fscanf(f,"%lf",cv->val);
  else
   {for(i=1; i<cv->num_lag+1; i++)
	  fscanf(f,"%lf",&(cv->val[i]));
    cv->val[0]=0;
   }
  fclose(f);
 }

//for(cur=son, num=0; cur!=NULL; cur=skip_next_obj(cur,&num))
for(cb=b; cb!=NULL; cb=cb->next)
 {cur=cb->head;
  i=cur->read_param(file_name);
  if(i==1)
	 return 1;
  num=0;
 }
return 0;
}










FILE *quick_file_search(FILE *f, char str1[])
{
char str[10000];


fgets(str,10000,f);
while(strcmp(str, str1) )
 {
 if(fgets(str, 10000, f)==NULL)
    break;
 kill_trailing_newline(str);   
 //plog(str);
 }


if(strcmp(str, str1))
 return NULL;
return f;

}

void load_description( FILE *f)
{
char type[20],label[100];
description *app;
char str[10000], str1[10000];
int done, i;

if(strncmp(msg, "Object", 6)==0)
 {
  strcpy(type, "Object");
  strcpy(label, msg+7);
 } 
if(strncmp(msg, "Variable", 8)==0)
 {
 strcpy(type, "Variable");
 strcpy(label, msg+9);
 } 

if(strncmp(msg, "Parameter", 9)==0)
 {
 strcpy(type, "Parameter");
 strcpy(label, msg+10);
 } 

if(strncmp(msg, "Function", 6)==0)
 {
  strcpy(type, "Function");
  strcpy(label, msg+9);
 } 
 
if(descr==NULL)
  app=descr=new description;
else  
  {
   for(app=descr; app->next!=NULL; app=app->next);
   app->next=new description;
   app=app->next;
  } 
app->next=NULL;
app->label=new char[strlen(label)+1];
strcpy(app->label, label);
app->type=new char[strlen(type)+1];
strcpy(app->type, type);

strcpy(str1, "");
fgets(str, 1000, f);//skip the first newline character
for( ; fgets(str, 1000, f)!=NULL && strncmp(str, "END_DESCRIPTION", 15)&& strncmp(str, "_INIT_", 6) ; )
  strcat(str1, str);

kill_trailing_newline(str1);

app->text=new char[strlen(str1)+1];
strcpy(app->text, str1);

if(!strcmp(str, "_INIT_\n") ||!strcmp(str, "_INIT_\r\n") )
 {
  strcpy(str1, "");
  for( ; fgets(str, 1000, f)!=NULL && strncmp(str, "END_DESCRIPTION", 15) ; )
    strcat(str1, str);
  kill_trailing_newline(str1);
  app->init=new char[strlen(str1)+1];
  strcpy(app->init, str1);
 }
else
 {
  app->init=new char[1];
  strcpy(app->init,"");
 }
app->initial='n';
app->observe='n';  
} 

void load_description(char const *label, char const *type, FILE *f)
{

description *app;
char str[10000], str1[10000];
int done, i;

//return;// BEWARE
if(descr==NULL)
  app=descr=new description;
else  
  {
   for(app=descr; app->next!=NULL; app=app->next);
   app->next=new description;
   app=app->next;
  } 
app->next=NULL;
app->label=new char[strlen(label)+1];
strcpy(app->label, label);
app->type=new char[strlen(type)+1];
strcpy(app->type, type);

sprintf(str, "%s_%s",type, label);

//fsetpos(f, fpos);
f=quick_file_search(f,str);
if(f==NULL)
 {
  app->text=new char[30];
  strcpy(app->text, "(no description available)");
  app->init=NULL;
  return ;

 }
fgets(str,1000,f);

strcpy(str1, "");

for( ; fgets(str, 1000, f)!=NULL && strncmp(str, "END_DESCRIPTION", 15)&& strncmp(str, "_INIT_", 6) ; )
  strcat(str1, str);

kill_trailing_newline(str1);

app->text=new char[strlen(str1)+1];
strcpy(app->text, str1);

if(!strcmp(str, "_INIT_\n") )
 {
  strcpy(str1, "");
  for( ; fgets(str, 1000, f)!=NULL && strncmp(str, "END_DESCRIPTION", 15) ; )
    strcat(str1, str);
  kill_trailing_newline(str1);
  app->init=new char[strlen(str1)+1];
  strcpy(app->init, str1);
}
else
 app->init=NULL;

//fclose(f);


app->initial='n';
app->observe='n';
//f=search_str(struct_file, "DOCUOBSERVE");
for( ; fgets(str, 1000, f)!=NULL && strncmp(str, "DOCUOBSERVE", 11) ; );
if(f!=NULL)
 {
 for( ; fgets(str, 1000, f)!=NULL && strncmp(str, "END_DOCUOBSERVE", 15) ; )
   {if(!strncmp(label, str, strlen(label)) )
     app->observe='y';
   }  
 
 //fclose(f);
 }

//f=search_str(struct_file, "DOCUINITIAL");
for( ; fgets(str, 1000, f)!=NULL && strncmp(str, "DOCUINITIAL", 11) ; );
if(f!=NULL)
 {
 for( ; fgets(str, 1000, f)!=NULL && strncmp(str, "END_DOCUINITIAL", 15) ; )
   {if(!strncmp(label, str, strlen(label)) )
     app->initial='y';
   } 
 

 }

}


void recur_description(object *r, FILE *f)
{
object *cur;
variable *cv;
bridge *cb;
load_description(r->label, "Object", f);

//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur) )
for(cb=r->b; cb!=NULL; cb=cb->next)
{
 cur=cb->head;
 recur_description(cur, f);
}
for(cv=r->v; cv!=NULL; cv=cv->next)
 {
  switch(cv->param)
   {
   case 0: load_description(cv->label, "Parameter", f);
           break;
   case 1: load_description(cv->label, "Variable", f);
           break;
   case 2: load_description(cv->label, "Function", f);
           break;
   default: plog("Error: wrong element loading description\n");
           break;
   }
 }

}




/*
copy the naked structure of the model into another object, called blueprint, to be used for adding objects
without example
*/
void set_blueprint(object *container, object *r)
{
object *cur, *cur1;
variable *cv, *cv1;
bridge *cb, *cb1;
int temp;
if(r==NULL)
 return;

for(cv=r->v; cv!=NULL; cv=cv->next)
   container->add_var_from_example(cv);
delete[] container->label;
container->label=new char[strlen(r->label)+1];
strcpy(container->label, r->label);

for(cb=r->b; cb!=NULL; cb=cb->next)
 {cur1=cb->head;
  container->add_obj(cur1->label, 1, 0);
  for(cb1=container->b; strcmp(cb1->blabel, cb->blabel); cb1=cb1->next);
  cur=cb1->head;
  set_blueprint(cur, cur1);
 }
}

void save_description(object *r, FILE *f)
{
object *cur;
variable *cv;
description *cd;
bridge *cb;

cd=search_description(r->label);
if(cd==NULL)
  {
   add_description(r->label, "Object", "(no description available)");
   sprintf(msg, "Warning! description for '%s' not found. New one created.\\n", r->label);
   plog(msg);
   cd=search_description(r->label);
  } 

if(cd->init==NULL)     
  fprintf(f, "%s_%s\n%s\nEND_DESCRIPTION\n\n",cd->type,cd->label,cd->text);
else
  fprintf(f, "%s_%s\n%s\n_INIT_\n%s\nEND_DESCRIPTION\n\n",cd->type,cd->label,cd->text, cd->init);

for(cv=r->v; cv!=NULL; cv=cv->next)
 {
  cd=search_description(cv->label);
  if(cd==NULL)
  {if(cv->param==0)
     add_description(cv->label, "Variable", "(no description available)");
   if(cv->param==1)
     add_description(cv->label, "Parameter", "(no description available)");  
   if(cv->param==2)
     add_description(cv->label, "Function", "(no description available)");  
   add_description(cv->label, "Object", "(no description available)");
   sprintf(msg, "Warning! description for '%s' not found. New one created.\\n", cv->label);
   plog(msg);
   cd=search_description(cv->label);
  } 

  if(cd->init==NULL)     
    fprintf(f, "%s_%s\n%s\nEND_DESCRIPTION\n\n",cd->type,cd->label,cd->text);
  else
    fprintf(f, "%s_%s\n%s\n_INIT_\n%s\nEND_DESCRIPTION\n\n",cd->type,cd->label,cd->text, cd->init);
   
 }

//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur) )
for(cb=r->b; cb!=NULL; cb=cb->next)
 {
  cur=cb->head;
  save_description(cur, f);
 }
}

/*
	Load current defined configuration
	Returns: 0: load ok, 1,2,3,4: load failure
*/
int load_configuration( object *r )
{
	int i, load = 0;
	object *cur;
	description *cur_descr;
	
	for ( cur = r; cur->up != NULL; cur = cur->up );
	r = cur;
	
	r->empty( );
	empty_cemetery( );
	lsd_eq_file[ 0 ] = ( char ) NULL;
	
	r->label = new char[ strlen( "Root" ) + 1 ];
	strcpy( r->label, "Root" );
	
	empty_descr( );
	add_description( "Root", "Object", "(no description available)" );

	delete [] struct_file;
	
	if ( strlen( path ) > 0 )
	{
		struct_file = new char[ strlen( path ) + strlen( simul_name ) + 6 ];
		sprintf( struct_file, "%s/%s.lsd", path, simul_name );
	}
	else
	{
		struct_file = new char[ strlen( simul_name ) + 6 ];
		sprintf( struct_file, "%s.lsd", simul_name );
	}

	FILE * f = fopen( struct_file, "r" );
	if( f == NULL )
		return 1;

    r->load_struct( f );
	struct_loaded = 1;
    fscanf( f, "%s", msg );					//should be DATA
	r->load_param( struct_file, 1, f );
	fscanf( f, "%s", msg );					//should be SIM_NUM 
	fscanf( f, "%d", &sim_num );
	fscanf( f, "%s", msg );					//should be SEED
	fscanf( f, "%d", &seed );
	fscanf( f, "%s", msg );					//should be MAX_STEP
	fscanf( f, "%d", &max_step );
	fscanf( f, "%s", msg );					//should be EQUATION
	fgets( msg, 200, f );

	delete [] equation_name;
	equation_name = new char[ strlen( msg ) + 1 ];
	strcpy( equation_name, msg + 1 );
    if ( equation_name[ strlen( equation_name ) - 1 ] == '\n' )
		equation_name[ strlen( equation_name ) - 1 ] = ( char ) NULL;
    if ( equation_name[ strlen( equation_name ) - 1 ] == '\r' )
		equation_name[ strlen( equation_name ) - 1 ] = ( char ) NULL;
  
	if ( fscanf( f, "%s", msg ) != 1 )		//should be MODELREPORT
	{
		load = 2;
		goto endLoad;
	}
	fscanf( f, "%s", name_rep );

	if ( fscanf( f, "%s", msg ) != 1 )		//should be DESCRIPTION
	{
		load = 3;
		goto endLoad;
	}  
	fscanf( f, "%s", msg );					//should be the first description   

	i = 1;
	while ( strcmp( msg, "DOCUOBSERVE" ) != 0 && i == 1 )
	{ 
		load_description( f );
		if( fscanf( f, "%s", msg ) != 1 ) 
			i = 0;
	}
	if( i == 0 )
	{
		load = 4;
		goto endLoad;
	} 
	
	fscanf( f, "%s", msg );  
	while ( strcmp( msg, "END_DOCUOBSERVE" ) != 0 )
	{
		cur_descr = search_description( msg );
		if( cur_descr == NULL )
		{
			sprintf( msg, "\nWarning: description for '%s' not found. Check elements to observe.", msg );
			plog( msg );
			fscanf( f, "%s", msg );
		}
		else 
		{
			cur_descr->observe = 'y';
			fscanf( f, "%s", msg );
		} 
	}
	
	fscanf( f, "%s", msg );  
	fscanf( f, "%s", msg );  
	while ( strcmp( msg, "END_DOCUINITIAL" ) != 0 )
	{
		cur_descr = search_description( msg );
		if ( cur_descr == NULL )
		{
			sprintf( msg, "\nWarning: description for '%s' not found. Check elements to initialize.", msg );
			plog( msg );
			fscanf( f, "%s", msg );
		}
		else
		{
			cur_descr->initial = 'y';
			fscanf( f, "%s", msg );
		}
	}
	
	if ( fscanf( f, "%s", msg ) == 1 )		//here is the equation file
	{
		fscanf( f, "%s", msg );
		strcpy( lsd_eq_file, msg );
		
		while ( fgets( msg, 1000, f ) != NULL && strncmp( msg, "END_EQ_FILE", 11 ) )
			strcat( lsd_eq_file, msg );
		
		i = strlen( lsd_eq_file );
		lsd_eq_file[ i - 1 ] = lsd_eq_file[ i ];
		
		if( ignore_eq_file == 0 && strcmp( lsd_eq_file, eq_file ) != 0 )
		{
			sprintf( msg, "\nWarning: the configuration file has been previously run with different equations\nfrom those used to create the Lsd model program.\nChanges may affect the simulation results. You can offload the original\nequations in a new equation file and compare differences using TkDiff in LMM\n(menu File)." );
			plog( msg );
		}
	}  
	
endLoad:
	fclose( f );
	
	t = 0;
	unsavedChange = false;

	return load;
}

/*
	Save current defined configuration (renaming if appropriate)
	Returns: true: save ok, false: save failure
*/
bool save_configuration( object *r, long findex )
{
	int indexDig = ( findex > 0 ) ? floor( log( findex ) / log( 10 ) + 2 ) : 0;
	object *cur;
	description *cur_descr;
	
	delete [] struct_file;
	
	if ( strlen( path ) > 0 )
	{
		struct_file = new char[ strlen( path ) + strlen( simul_name ) + 6 + indexDig ];
		sprintf( struct_file, "%s/%s", path, simul_name );
	}
	else
	{
		struct_file = new char[ strlen( simul_name ) + 6 + indexDig ];
		sprintf( struct_file, "%s", simul_name );
	}
	if ( findex > 0 )
		sprintf( struct_file, "%s_%ld.lsd", struct_file, findex );
	else
		sprintf( struct_file, "%s.lsd", struct_file );
	
	FILE * f = fopen( struct_file, "w" );
	if ( f == NULL )
		return false;

	for ( cur = r; cur->up != NULL; cur = cur->up );
	
	cur->save_struct( f, "" );
	fprintf( f, "\nDATA\n" );
	cur->save_param( f );
	
	int delta = ( findex > 0 ) ? sim_num * ( findex - 1 ) : 0;
	fprintf( f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d\nEQUATION %s\n MODELREPORT %s\n", sim_num, seed + delta, max_step, equation_name, name_rep );
	
	fprintf( f, "\nDESCRIPTION\n\n" );
	save_description( cur, f );
	
	fprintf( f, "\nDOCUOBSERVE\n" );
	for ( cur_descr = descr; cur_descr != NULL; cur_descr = cur_descr->next )
		if ( cur_descr->observe == 'y' )   
			fprintf( f, "%s\n", cur_descr->label );
	fprintf( f, "\nEND_DOCUOBSERVE\n\n" );
	
	fprintf( f, "\nDOCUINITIAL\n" );
	for ( cur_descr = descr; cur_descr != NULL; cur_descr = cur_descr->next )
		if( cur_descr->initial == 'y' )     
			fprintf( f, "%s\n", cur_descr->label );
	fprintf( f, "\nEND_DOCUINITIAL\n\n" );
	
	save_eqfile( f );
	
	fclose( f );
	
	if ( findex <= 0 )
		unsavedChange = false;		// no changes to save
	
	return true;
}
