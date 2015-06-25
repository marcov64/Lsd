/***************************************************
****************************************************
LSD 6.4 - January 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/


/****************************************************
UTIL.CPP contains a set of utilities for different parts of the
program.
The functions contained in this file are:

- object *skip_next_obj(object *t, int *count);
Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- object *go_brother(object *c);
returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.


- void cmd(Tcl_Interp *inter, char *cc);
Standard routine to send the message string cc to the TCL interpreter in order
to execute a command for the graphical interfaces.
It should be enough to make a call to Tcl_Eval. But there are problems due to the
fact that this standard tcl function wants the string cc to be writable. Hence,
it shouldn't work when a constant string is passed. Actually, it worked under windows
but not under unix. Instead, I use Tcl_VarEval, that allows to use pieces
of strings (including the last terminating character NULL) and  it does not
require a writable string.

- int my_strcmp(char *a, char *b)
It is a normal strcmp, but it catches the possibility of both strings being
NULL

- void go_next(object **t)
returns next if (*t)->next is not null. Don't know it is used any longer

- double norm(double mean, double dev)
returns a random number drawn from a normal with mean mean and standard deviation\
dev.

- FILE *search_str(char *name, char *str)
given a string name, returns the file corresponding to name, and the current
position of the file is just after str. Think I don't use any longer.

- FILE *search_data_str(char *name, char *init, char *str)
given a string name, returns the file with that name and the current position
placed immediately after the string str found after the string init. Needed to
not get confused managing the data files, where the same string appears twice,
in the structure definition and in the data section.

- FILE *search_data_ent(char *name, variable *v)
given the file name name, the routine searches for the data line for the variable
(or parameter) v. It is not messed up by same labels for variables and objects.

- other various mathematical routines

 ****************************************************/
#include "choose.h"

#ifndef NO_WINDOW
 #include <tk.h>
 extern Tcl_Interp *inter;
#endif

#include "decl.h"
#include <ctype.h>

extern char *simul_file;
extern char *struct_file;
extern char *param_file;
extern char *result_file;
extern char *total_file;
extern char msg[];
extern variable *cemetery;
extern char *equation_name;
extern lsdstack *stacklog;
extern long idum;
extern description *descr;
extern char *eq_file;
extern char lsd_eq_file[];
extern int lattice_type;
extern int t;
extern int running;
extern char nonavail[];	// string for unavailable values

double ran1(long *idum);
#define RND (double)ran1(&idum)

double log(double v);
double exp(double c);
double sqrt(double v);

void plog(char const *msg);
void print_stack(void);
void clean_spaces(char *s);
void scan_used_lab(char *lab, int *choice);
char const *return_where_used(char *lab, char s[]);
void init_canvas(void);
void error_hard(void);
void add_description(char const *lab, char const *type, char const *text);
void save_single(variable *vcv);

#ifndef NO_WINDOW
/****************************************************
CMD
****************************************************/
void cmd(Tcl_Interp *inter, char const *cm)
{

int code;
FILE *f;

/**
The following code works (and much faster), only under win2k with tcl8.0. Under Linux and Tcl8.3 it crashes badly.
*
Tcl_Obj *o;
o=Tcl_NewStringObj(cm, strlen(cm));
code=Tcl_EvalObjEx(inter, o, TCL_EVAL_DIRECT);
if(code!=TCL_OK)
 {f=fopen("tk_err.err","a");
  sprintf(msg, "\n%s\n\n%s\n",cm, Tcl_GetStringResult(inter));
  fprintf(f,"%s", msg);
  fclose(f);
  plog("\nTcl-Tk Error. See file tk_err.err\n");
 }


return;
/**/
//code=Tcl_VarEval(inter, cm, NULL);
code=Tcl_Eval(inter, cm);

if(code!=TCL_OK)
 {f=fopen("tk_err.err","a");
  sprintf(msg, "\n%s\n\n%s\n",cm, Tcl_GetStringResult(inter));
  fprintf(f,"%s", msg);
  fclose(f);
  plog("\nTcl-Tk Error. See file tk_err.err\n");
 }

}

/****************************************************
CMD
****************************************************/
void cmd(char *cm)
{

int code;
FILE *f;

//code=Tcl_VarEval(inter, cm, NULL);
code=Tcl_Eval(inter, cm);

if(code!=TCL_OK)
 {f=fopen("tk_err.err","a");
  sprintf(msg, "\n%s\n\n%s\n",cm, Tcl_GetStringResult(inter));
  fprintf(f,"%s", msg);
  fclose(f);
  plog("\nTcl-Tk Error. See file tk_err.err\n");
 }

}

#else
void cmd(char *cm)
{

}
#endif
/****************************************************
GO_BROTHER
****************************************************/
object *go_brother(object *c)
{
if(c->next==NULL)
  return(NULL);
return(c->next);
}


object *skip_next_obj(object *tr)
{
bridge *cb;
if(tr==NULL)
 return(NULL);

if(tr->up==NULL)
 return(NULL);
for(cb=tr->up->b; cb!=NULL; cb=cb->next)
 {
  if(!strcmp(cb->blabel,tr->label))
   {
    if(cb->next==NULL)
     return(NULL);
    else
     return(cb->next->head); 
   }  
 }
}

/****************************************************
SKIP_NEXT_OBJ
This is the old version, before moving to the bridge-based representation
****************************************************/
object *skip_next_objOLD(object *t, int *count)
{
char *lab;
register object *c;
register int i;

*count=0;
if(t==NULL)
 return(NULL);
if(t->next==NULL)
 {*count+=1;
  return(NULL);
 }

for(lab=t->label,c=t, i=0; !strcmp(lab,c->label); i++)
  if(c->next==NULL)
    {c=NULL;
    i++;
    break;
    }
  else
    c=c->next;
*count=i;
return (c);
};

/****************************************************
SKIP_NEXT_OBJ
This is the new version, after moving to the bridge-based representation
****************************************************/
object *skip_next_obj(object *t, int *count)
{
char *lab;
register object *c;
register int i;

*count=0;
if(t==NULL)
 return(NULL);
for(c=t, i=0;c!=NULL; c=c->next, i++);
*count=i;

return skip_next_obj(t);
};

/****************************************************
MY_STRCMP
****************************************************/
int my_strcmp(char *a, char *b)
{
int res;
if(a==NULL && b==NULL)
 return 0;

res=strcmp(a,b);
return res;
};

/****************************************************
GO_NEXT
****************************************************/
void go_next(object **t)
{
if((*t)->next!=NULL)
  *t=(*t)->next;
else
  *t=NULL;
};



int dum;

//initialize the pseudo random functions
void init_random(int seed)
{
idum=-seed;
ran1(&idum);
dum=0;
}
/****************************************************
NORM
***************************************************/
double norm(double mean, double dev)
{

double gasdev, R, v1, v2, fac;
static double gset;
int boh=1;
if(dum==0)
{ do
  { v1=2.0*RND-1;
    boh=1;
    v2=2.0*RND-1;
    R=v1*v1+v2*v2;
  }
  while(R>=1);
 fac=log(R);
 fac=fac/R;
 fac=fac*(-2.0);
 fac=sqrt(fac);
 gset=v1*fac;
 gasdev=v2*fac;
 dum=1;
}
else
{gasdev=gset;
 dum=0;
}
gasdev=gasdev*dev+mean;
return(gasdev);
}


int draw(double p)
{double dice;

dice=RND;

if(dice<p)
  return(1);
return(0);
}

double max(double a, double b)
{
if(a>b)
 return a;
return(b);
};

double min(double a, double b)
{
if(a<b)
 return a;
return(b);
};


/****************************************************
SEARCH_STR
****************************************************/
FILE *search_str(char const *name, char const *str)
{
FILE *f;
char got[80];

f=fopen(name, "r");
if(f==NULL)
 {return(NULL);
 }

fscanf(f, "%s", got);
while(strcmp(got, str))
{
if(fscanf(f, "%s", got)==EOF)
 return(NULL);
}
return(f);
}

/****************************************************
SEARCH_STR_nospaces
****************************************************/
FILE *search_str_nospaces(char *name, char *str)
{
FILE *f;
char got[400];

f=fopen(name, "r");
if(f==NULL)
 {return(NULL);
 }

fgets(got, 400, f);
clean_spaces(got);
while(strncmp(got, str, strlen(str)))
{
if(fgets(got, 400, f)==NULL)
 return(NULL);
clean_spaces(got); 
}
return(f);
}


/****************************************************
SEARCH_DATA_STR
****************************************************/
FILE *search_data_str(char const *name, char const *init, char const *str)
{
FILE *f;
char got[80];

f=fopen(name, "r");
if(f==NULL)
 {return(NULL);
 }

fscanf(f, "%s", got);
while(strcmp(got, init))
{
if(fscanf(f, "%s", got)==EOF)
 return(NULL);
}


while(strcmp(got, str))
{
if(fscanf(f, "%s", got)==EOF)
 return(NULL);
}
return(f);
}

/****************************************************
SEARCH_DATA_ENT
****************************************************/
FILE *search_data_ent(char *name, variable *v)
{
FILE *f;
char got[80];
char temp[80];
char temp1[80];
char typ[20];

f=fopen(name, "r");
if(f==NULL)
 {return(NULL);
 }

fscanf(f, "%s", got);
while(strcmp(got, "DATA"))
{
if(fscanf(f, "%s", got)==EOF)
 return(NULL);
}

strcpy(temp, (v->up)->label); //Search for the section of the Object
fscanf(f, "%s", temp1);
fscanf(f, "%s", got);

while(strcmp(got, temp) || strcmp(temp1,"Object:") )
{
strcpy(temp1, got);
if(fscanf(f, "%s", got)==EOF)
 return(NULL);
}

//hopefully, we are at the beginning of the vars in the correct object
if(v->param==1)
 strcpy(typ,"Param:");
else
 if(v->param==2)
  strcpy(typ,"Func:");
 else
  strcpy(typ,"Var:");

fscanf(f, "%s", temp1); //Search for the line of the var
fscanf(f, "%s", got);

while(strcmp(got, v->label) || strcmp(temp1,typ) )
{
strcpy(temp1, got);
if(fscanf(f, "%s", got)==EOF)
 return(NULL);

}


return(f);
}

double round(double x)
{
if( (x-floor(x)) > (ceil(x) - x ) )
 return ceil(x);
return floor(x);
}

void set_counter(object *o)
{
object *cur;
bridge *cb;
int i;

if(o->up==NULL)
  return;

set_counter(o->up);  
for(cb=o->up->b; strcmp(cb->blabel,o->label); cb=cb->next);

if(cb->counter_updated==true)
  return;

for(cur=cb->head,i=1; cur!=NULL; cur=cur->next,i++)
 cur->acounter=i;

cb->counter_updated=true;
   
}
void set_lab_tit(variable *var)
{
/*
Ensure that all objects on top of the variables have the counter updated,
and then writes the lab_tit field.

lab_tit indicates the position of the object containing the variables in the model.
*/

object *cur, *ceil, *cur1;
bridge *cb;
char app[2000], app1[2000];
bool first=true;
if(var->up->up==NULL)
  {
   //this is the Root of the model
   if(var->lab_tit!=NULL)
     return; //already done in the past
   var->lab_tit=new char[strlen("R")+1];
   strcpy(var->lab_tit,"R");
   return;
  }

for(cur=var->up; cur->up!=NULL; cur=cur->up)
 {
//  for(cb=cur->b; strcmp(cb->blabel,var->up->label); cb=cb->next);
  //found the bridge containing the variable
  set_counter(cur);
  if(first==false)
   sprintf(app1, "%d_%s",cur->acounter,app);
  else
   {
   first=false;
   sprintf(app1,"%d",cur->acounter);
   }
   strcpy(app,app1);
  } 

if(var->lab_tit!=NULL)
  delete[] var->lab_tit;
var->lab_tit=new char[strlen(app)+1];
strcpy(var->lab_tit,app);  
}



void add_cemetery(variable *v)
{
/*
Store the variable in a list of variables in objects deleted
but to be used for analysis.
*/
variable *cv;
if(v->savei==true)
  save_single(v);
if(cemetery==NULL)
 {cemetery=v;
  v->next=NULL;
 }
else
 {for(cv=cemetery; cv->next!=NULL; cv=cv->next);
  cv->next=v;
  v->next=NULL;
 }
}

// Processes variables from an object required to go to cemetery 
void collect_cemetery( object *o )
{
	variable *cv, *nv;
	
	for ( cv = o->v; cv != NULL; cv = nv )	// scan all variables
	{
		nv = cv->next;						// pointer to next variable
		
		if ( running==1 && ( cv->save == true || cv->savei == true ) )	// need to save?
		{
			cv->end = t;					// define last period,
			cv->data[ t ] = cv->val[ 0 ];	// last value
			set_lab_tit( cv );				// and last lab_tit
			add_cemetery( cv );				// put in cemetery (destroy cv->next)
		}
	}
}

void empty_cemetery(void)
{
variable *cv, *cv1;
for(cv=cemetery; cv!=NULL; )
 {cv1=cv->next;
  cv->empty();
  delete cv;
  cv=cv1;
 }
cemetery=NULL;
}

/*
	Methods for results file saving (class result)
*/

// saves data to file in the specified period
void result::data( object *root, int initstep, int endtstep )
{
	endtstep = endtstep == 0 ? initstep : endtstep;	// adjust for 1 time step if needed
	
	for ( int i = initstep; i <= endtstep; i++ )
	{
		data_recursive( root, i );		// output one data line
		
		if ( dozip )				// and change line
		{
			#ifdef LIBZ
				gzprintf( fz, "\n" );
			#endif
		}
		else
			fprintf( f, "\n" );
	}
}

void result::data_recursive( object *r, int i )
{
object *cur;
variable *cv;
bridge *cb;

int flag_save;

for(cv=r->v; cv!=NULL; cv=cv->next)
 {
 
 if(cv->save==1)
  {
   if(cv->start <= i && cv->end >= i && !isnan(cv->data[i]))		// save NaN as n/a
		if ( dozip )
		{
			#ifdef LIBZ
				gzprintf( fz, "%g\t", cv->data[i] );
			#endif
		}
		else
			fprintf( f, "%g\t", cv->data[i] );
   else
		if ( dozip )
		{
			#ifdef LIBZ
				gzprintf( fz, "%s\t", nonavail );
			#endif
		}
		else
			fprintf( f, "%s\t", nonavail );
  }
 
 }
for(cb=r->b; cb!=NULL; cb=cb->next)
 {
  cur=cb->head;
  if(cur->to_compute==1)
  {
  for(; cur!=NULL; cur=cur->next)
	data_recursive( cur, i );
  } 
 }

if(r->up==NULL)
 {for(cv=cemetery; cv!=NULL; cv=cv->next)
    if(cv->start<=i && cv->end>=i && !isnan(cv->data[i]))		// save NaN as n/a
		if ( dozip )
		{
			#ifdef LIBZ
				gzprintf( fz, "%g\t", cv->data[i] );
			#endif
		}
		else
			fprintf( f, "%g\t", cv->data[i] );
     else
		if ( dozip )
		{
			#ifdef LIBZ
				gzprintf( fz, "%s\t", nonavail );
			#endif
		}
		else
			fprintf(f, "%s\t", nonavail);
 }
}

// saves header to file
void result::title( object *root, int flag )
{
	title_recursive( root, flag );		// output header
		
	if ( dozip )						// and change line
	{
		#ifdef LIBZ
			gzprintf( fz, "\n" );
		#endif
	}
	else
		fprintf( f, "\n" );
}

void result::title_recursive( object *r, int header )
{
object *cur;
variable *cv;

bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
  {
  if(cv->save==1)
   {set_lab_tit(cv);
   if(header==1)
		if ( dozip )
		{
			#ifdef LIBZ
				gzprintf( fz, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
			#endif
		}
		else
			fprintf( f, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
   else
		if ( dozip )
		{
			#ifdef LIBZ
				gzprintf( fz, "%s %s (-1 -1)\t", cv->label, cv->lab_tit );
			#endif
		}
		else
			fprintf( f, "%s %s (-1 -1)\t", cv->label, cv->lab_tit );
   }
 }
 
for(cb=r->b; cb!=NULL; cb=cb->next)
 {
  cur=cb->head;
  if(cur->to_compute==1)
  {
  for(; cur!=NULL; cur=cur->next)
    title_recursive( cur, header );
  } 
 } 

if(r->up==NULL)
 {for(cv=cemetery; cv!=NULL; cv=cv->next)
	if ( dozip )
	{
		#ifdef LIBZ
			gzprintf( fz, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
		#endif
	}
	else
		fprintf( f, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
 }

}

// open the appropriate file for saving the results (constructor)
result::result( char const *fname, char const *fmode, bool dozip )
{
	#ifndef LIBZ
		dozip = false;			// disable zip if libraries not available
	#endif
	this->dozip = dozip;		// save local class flag
	if ( dozip )
	{
		#ifdef LIBZ
			char *fnamez = new char[ strlen( fname ) + 4 ];	// append .gz to the file name
			strcpy( fnamez, fname );
			strcat( fnamez, ".gz");
			fz = gzopen( fnamez, fmode );
			delete fnamez;
		#endif
	}
	else
		f = fopen( fname, fmode );
}

// close the appropriate results file (destructor)
result::~result( void )
{
	if ( dozip )
	{
		#ifdef LIBZ
			gzclose( fz );
		#endif
	}
	else
		fclose( f );
}


double rnd_integer(double m, double x)
{
double r;

r=m+RND*(x+1-m);

return(floor(r));
}








double gammaln(double xx)
{
double x, y, tmp, ser;
static double cof[6]={76.18009172947146,-86.50532032941677, 24.01409824083091,-1.231739572450155, 0.1208650973866179e-2,-0.5395239384953e-5};
int j;
y=x=xx;
tmp=x+5.5;
tmp -= (x+0.5)*log(tmp);
ser=1.000000000190015;
for(j=0;j<=5;j++) ser += cof[j]/++y;
return -tmp+log(2.5066282746310005*ser/x);

}


#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)
#define PI 3.141592654

double ran1(long *idum)
{

int j;
long k;
static long iy=0;
static long iv[NTAB];

double temp;

if (*idum <= 0 || !iy) {
if (-(*idum) < 1) *idum=1;
else *idum = -(*idum);
for (j=NTAB+7;j>=0;j--) {
   k=(*idum)/IQ;
   *idum=IA*(*idum-k*IQ)-IR*k;
   if (*idum < 0) *idum += IM;
   if (j < NTAB) iv[j] = *idum;
  }
  iy=iv[0];
  
}
k=(*idum)/IQ;
*idum=IA*(*idum-k*IQ)-IR*k;
if (*idum < 0) *idum += IM;
j=iy/NDIV;
iy=iv[j];
iv[j] = *idum;
if ((temp=AM*iy) > RNMX) return RNMX;
else return temp;
}
   

extern int quit;

double gamdev(int ia, long *idum)
{

int j;
double am, e, s, v1, v2, x, y;
if(ia<1) 
 {
  plog("Error in routine gamdev");
  quit=1;
  return 0;
 } 
if(ia<6)
 {
 x=1.0;
 for(j=1; j<=ia; j++) x*=ran1(idum);
 x=-log(x);
 }
else
 {
 do
  {
   do
   {
    do
    { 
    v1=ran1(idum);
    v2=2*ran1(idum)-1.0;
    }
    while(v1*v1+v2*v2>1.0);
   y=v2/v1;
   am=ia-1;
   s=sqrt(2.0*am+1.0);
   x=s*y+am;
   }
   while(x<=0);
  e=(1+y*y)*exp(am*log(x/am)-s*y);
 }
 while(ran1(idum)>e);
} 
return x;
}


double poidev(double xm, long *idum)
{

double gammaln(double xx);
double ran1(long *idum);
static double sq,alxm,g,oldm=(-1.0);
double em,t,y;
if (xm < 12.0) {
  if (xm != oldm) {
    oldm=xm;
    g=exp(-xm);
  }
em = -1;
t=1.0;
do {
  ++em;
  t *= ran1(idum);
 } while (t > g);
} else {
  if (xm != oldm) {
    oldm=xm;
    sq=sqrt(2.0*xm);
    alxm=log(xm);
    g=xm*alxm-gammaln(xm+1.0);
  }
 do {
   do {
      y=tan(PI*ran1(idum));
      em=sq*y+xm;
      
   } while (em < 0.0);   
   em=floor(em);
   t=0.9*(1.0+y*y)*exp(em*alxm-gammaln(em+1.0)-g);
   } while (ran1(idum) > t);
 }
 return em;
}

// ################### ADDITIONAL STATISTICAL C FUNCTIONS ################### //

/*
 * Return a draw from a lognormal distribution
 */

double lnorm( double mu, double sigma )
{
	if ( sigma <= 0.0 )
	{
		plog( "\nWarning: bad sigma in function: lnorm" );
		return 0.0;
	}

	return exp( norm( mu, sigma ) );
}


/*
 * Return a draw from an asymmetric laplace distribution
 */

double alapl( double mu, double alpha1, double alpha2 )
{
	if ( alpha1 <= 0.0 || alpha2 <= 0.0 )
	{
		plog( "\nWarning: bad alpha in function: alapl" );
		return 0.0;
	}

	double draw = RND;
	if( draw < ( alpha1 / ( alpha1 + alpha2 ) ) )
		return mu + alpha1 * log( draw * ( 1 + alpha1 / alpha2 ) );
	else 
		return mu - alpha2 * log( ( 1 - draw ) * ( 1 + alpha1 / alpha2 ) );
}


/*
 * Return a draw from a Beta(alfa,beta) distribution
 * Dosi et al. (2010) K+S
 */

double beta( double alpha, double beta )

{
	if ( alpha <= 0.0 || beta <= 0.0 )
	{
		plog( "\nWarning: bad alpha or beta in function: beta" );
		return 0.0;
	}
	
	double U, V, den;
	U = RND;
	V = RND; 
	den = pow( U, ( 1 / alpha ) ) + pow( V, ( 1 / beta ) );
	
	while ( den <= 0 || den > 1)
	{
		U = RND;
		V = RND;
		den = pow( U,( 1 / alpha ) ) + pow( V, ( 1 / beta ) );
	}

	return pow( U , ( 1 / alpha ) ) / den;
}


/*
 * Factorial function
 */

double fact( double x )
{
	x = floor( x );
	if ( x < 0.0 )
	{
		plog( "\nWarning: bad x in function: fact" );
		return 0.0;
	}

	double fact = 1.0;
	long i = 1;
	while (i <= x)
		fact *= i++;
	
	return fact;
}


/*
 * Uniform cumulative distribution function
 */

double unifcdf( double a, double b, double x )
{
	if ( a >= b )
	{
		plog( "\nWarning: bad a or b in function: uniformcdf" );
		return 0.0;
	}

	if ( x <= a )
		return 0.0;
	if ( x >= b )
		return 1.0;

	return ( x - a ) / ( b - a );
}


/*
 * Poisson cumulative distribution function
 */

double poissoncdf( double lambda, double k )
{
	k = floor( k );
	if ( lambda <= 0.0 || k < 0.0 )
	{
		plog( "\nWarning: bad lambda or k in function: poissoncdf" );
		return 0.0;
	}
	
	double sum = 0.0;
	long i;
	for ( i = 0; i <= k; i++ )
		sum += pow( lambda, i ) / fact( i );
	
	return exp( -lambda ) * sum;
}


/*
 * Normal cumulative distribution function
 */

double normcdf( double mu, double sigma, double x )
{
	if ( sigma <= 0.0 )
	{
		plog( "\nWarning: bad sigma in function: normalcdf" );
		return 0.0;
	}
	
	return 0.5 * ( 1 + erf( ( x - mu ) / ( sigma * sqrt( 2.0 ) ) ) );
}


/*
 * Lognormal cumulative distribution function
 */

double lnormcdf( double mu, double sigma, double x )
{
	if ( sigma <= 0.0 || x <= 0.0 )
	{
		plog( "\nWarning: bad sigma or x in function: lnormalcdf" );
		return 0.0;
	}
	
	return 0.5 + 0.5 * erf( ( log( x ) - mu ) / ( sigma * sqrt( 2.0 ) ) );
}


/*
 * Asymmetric laplace cumulative distribution function
 */

double alaplcdf( double mu, double alpha1, double alpha2, double x )
{
	if ( alpha1 <= 0.0 || alpha2 <= 0.0 )
	{
		plog( "\nWarning: bad alpha in function: alaplcdf" );
		return 0.0;
	}
	
	if ( x < mu )									// cdf up to upper bound
		return 0.5 * exp( ( x - mu ) / alpha1 );
	else
		return 1 - 0.5 * exp( -( x - mu ) / alpha2 );
}


/*
 * Beta distribution: continued fraction evaluation function
 * Press et al. (1992) Numerical Recipes in C, 2nd Ed.
 */

#define MAXIT 100
#define BEPS 3.0e-7
#define FPMIN 1.0e-30

double betacf( double a, double b, double x )
{
	void nrerror(char error_text[]);
	int m, m2;
	double aa, c, d, del, h, qab, qam, qap;

	qab = a + b;
	qap = a + 1.0;
	qam = a - 1.0;
	c = 1.0;
	d = 1.0 - qab * x / qap;

	if ( fabs( d ) < FPMIN ) 
		d = FPMIN;
	d = 1.0 / d;
	h = d;

	for ( m = 1; m <= MAXIT; m++ ) 
	{
		m2 = 2 * m;
		aa = m * ( b - m ) * x / ( ( qam + m2 ) * ( a + m2 ) );
		d = 1.0 + aa * d;
		if ( fabs( d ) < FPMIN)
			d = FPMIN;
		
		c = 1.0 + aa / c;
		if ( fabs( c ) < FPMIN )
			c=FPMIN;

		d = 1.0 / d;
		h *= d * c;
		aa = -( a + m ) * ( qab + m ) * x / ( ( a + m2 ) * ( qap + m2 ) );
		d = 1.0 + aa * d;
		if ( fabs( d ) < FPMIN )
			d = FPMIN;

		c = 1.0 + aa / c;
		if ( fabs( c ) < FPMIN )
			c = FPMIN;

		d = 1.0 / d;
		del = d * c;
		h *= del;
		if ( fabs( del - 1.0) < BEPS )
			break;
	}

	if ( m > MAXIT ) 
		plog( "\nWarning: a or b too big (or MAXIT too small) in function: betacf");

	return h;
}


/*
 * Beta cumulative distribution function: incomplete beta function
 * Press et al. (1992) Numerical Recipes in C, 2nd Ed.
 */

double betacdf( double alpha, double beta, double x )
{
	double bt;

	if ( alpha <= 0.0 || beta <= 0.0 || x < 0.0 || x > 1.0 )
	{
		plog( "\nWarning: bad alpha, beta or x in function: betacdf" );
		return 0.0;
	}

	if ( x == 0.0 || x == 1.0 )
		bt = 0.0;
	else
		bt = exp( lgamma( alpha + beta ) - lgamma( alpha ) - lgamma( beta ) 
				 + alpha * log( x ) + beta * log( 1.0 - x ) );

	if ( x < ( alpha + 1.0 ) / ( alpha + beta + 2.0 ) )
		return bt * betacf( alpha, beta, x ) / alpha;
	else
		return 1.0 - bt * betacf( beta, alpha, 1.0 - x ) / beta;
}


void error(char *m)
{
char app[1100];
sprintf(app, "tk_messageBox -type ok -title \"Lsd error\" -message \"%s\"",m );
#ifndef NO_WINDOW
cmd(inter, app);
#else
 printf("%s\n",m);
#endif 
}

void error_cycle(char const *l)
{
sprintf(msg, "\nError in CYCLE (var. %s): object %s not found",stacklog->vs->label, l);
plog(msg);
error_hard();
}

struct s
{
int x;
struct s *son;
struct s *next;

};

struct s d;

int store(int x1, int x2, int x3, int x4);
int store(struct s *c, int x2, int x3, int x4);
int store(struct s *c, int x3, int x4);
int store(struct s *c, int x4);
void free_storage(struct s *c);


int shrink_gnufile(void)
{


d.son=NULL;
d.next=NULL;
d.x=-1;

char str[200], str1[200], str2[200], str3[200], str4[200], fin[300];

int x1, x2, x3, x4, count=1;

FILE *f, *f1;
int i, j, h=0;
f=fopen("plot.file", "r");
if(f==NULL)
 exit(1);

f1=fopen("plot_clean.file", "w");
if(f==NULL)
 exit(1);


while(fgets(str, 200, f)!=NULL)
 {if(h++==1)
   fprintf(f1, "set font \"{Times 10}\"\n");
 sscanf(str, "%s %s", str1, str2);
 if(!strcmp(str1, "$can") && !strcmp(str2, "create") )
   {
   i=strcspn(str, "[");
   j=strcspn(str, "]");
   strncpy(str1, str+i, j-i+1);
   str1[j-i+1]='\0';
   sscanf(str1,"[expr $cmx * %d /1000]", &x1);

   i=strcspn(str+j+1, "[");
   i+=j+1;
   j=strcspn(str+i+1, "]");
   j+=i+1;
   strncpy(str2, str+i, j-i+1);
   str2[j-i+1]='\0';
   sscanf(str2,"[expr $cmy * %d /1000]", &x2);

   i=strcspn(str+j+1, "[");
   i+=j+1;
   j=strcspn(str+i+1, "]");
   j+=i+1;
   strncpy(str3, str+i, j-i+1);
   str3[j-i+1]='\0';
   sscanf(str3,"[expr $cmx * %d /1000]", &x3);

   i=strcspn(str+j+1, "[");
   i+=j+1;
   j=strcspn(str+i+1, "]");
   j+=i+1;
   strncpy(str4, str+i, j-i+1);
   str4[j-i+1]='\0';
   sscanf(str4,"[expr $cmy * %d /1000]", &x4);
   if(store(x1, x2, x3, x4)==1)   //if new data are stored, then add it to the cleaned file
     fprintf(f1, "%s", str);
   }
  else
   fprintf(f1, "%s", str);

 }
fclose(f);
fclose(f1);


if(d.next!=NULL)
 free_storage(d.next);
if(d.son!=NULL)
 free_storage(d.son);

return 0;
}

int store(int x1, int x2, int x3, int x4)
{
int flag=0, res;
struct s *app, *prev;

for(app=&d; app!=NULL; app=app->next)
 {if(app->x==x1)
   { res=store(app->son,x2, x3, x4);
     return res;
     break;
   }
  else
   prev=app;
 }
if(app==NULL)
 {prev->next=new struct s;
  app=prev->next;
  app->x=x1;
  app->next=NULL;
  app->son=new struct s;
  app=app->son;
  app->x=x2;
  app->next=NULL;
  app->son=new struct s;
  app=app->son;
  app->x=x3;
  app->next=NULL;
  app->son=new struct s;
  app=app->son;
  app->x=x4;
  app->next=NULL;
  app->son=NULL;
  return 1;
 }
printf("\nWhat the hell am I doing here?\n");
exit(2);

}

int store(struct s *c, int x2, int x3, int x4)
{
int flag=0, res;
struct s *app, *prev;

for(app=c; app!=NULL; app=app->next)
 {if(app->x==x2)
   { res=store(app->son, x3, x4);
     return res;
     break;
   }
  else
   prev=app;
 }
if(app==NULL)
 {prev->next=new struct s;
  app=prev->next;
  app->x=x2;
  app->next=NULL;
  app->son=new struct s;
  app=app->son;
  app->x=x3;
  app->next=NULL;
  app->son=new struct s;
  app=app->son;
  app->x=x4;
  app->next=NULL;
  app->son=NULL;

  return 1;
 }
printf("\nWhat the hell am I doing here?\n");
exit(2);

}

int store(struct s *c, int x3, int x4)
{
int flag=0, res;
struct s *app, *prev;

for(app=c; app!=NULL; app=app->next)
 {if(app->x==x3)
   { res=store(app->son, x4);
     return res;
     break;
   }
  else
   prev=app;
 }
if(app==NULL)
 {prev->next=new struct s;
  app=prev->next;
  app->x=x3;
  app->next=NULL;
  app->son=new struct s;
  app=app->son;
  app->x=x4;
  app->next=NULL;
  app->son=NULL;

  return 1;
 }
printf("\nWhat the hell am I doing here?\n");
exit(2);

}

int store(struct s *c, int x4)
{
int flag=0, res;
struct s *app, *prev;

for(app=c; app!=NULL; app=app->next)
 {if(app->x==x4)
   { return 0;
     break;
   }
  else
   prev=app; 
 }
if(app==NULL)
 {prev->next=new struct s;
  app=prev->next;
  app->x=x4;
  app->next=NULL;
  app->son=NULL;

  return 1;
 }
printf("\nWhat the hell am I doing here?\n");
exit(2);

}

void free_storage(struct s *c)
{
struct s *app, *n, *down;

if(c->next!=NULL)
 free_storage(c->next);
if(c->son!=NULL)
 free_storage(c->son);

delete c;

}


description *search_description(char *lab)
{
description *cur;

for(cur=descr; cur!=NULL; cur=cur->next)
 {
  if(!strcmp(cur->label,lab) )
   return cur;
 }
return NULL;
} 

#ifndef NO_WINDOW
/********************
autofill_descr

generate recur. the descriptions of the model as it is
*********************/
void autofill_descr(object *o)
{

description *cur;
variable *cv;
object *co;
int i;
bridge *cb;

cur=search_description(o->label);
if(cur==NULL)
 add_description(o->label, "Object", "(no description available)");

for(cv=o->v; cv!=NULL; cv=cv->next)
 {
  cur=search_description(cv->label);
  if(cur==NULL)
   {i=cv->param;
   if(i==1)
    add_description(cv->label, "Parameter", "(no description available)");
   if(i==0)
    add_description(cv->label, "Variable", "(no description available)");
   if(i==2)
    add_description(cv->label, "Function", "(no description available)");
   } 
 } 
//for(co=o->son; co!=NULL; co=skip_next_obj(co)) 
for(cb=o->b; cb!=NULL; cb=cb->next)
  autofill_descr(cb->head);
}

#endif
void change_descr_lab(char const *lab_old, char const *lab, char const *type, char const *text, char const *init)
{
description *cur, *cur1;

for(cur=descr; cur!=NULL; cur=cur->next)
 {
  if(!strcmp(cur->label, lab_old) )
   {

   if(!strcmp(lab,"") && !strcmp(type,"") && !strcmp(text,"") && !strcmp(init,"") )
    {
     delete[] cur->label;
     delete[] cur->type;
     delete[] cur->text;
     if(cur->init!=NULL)
      delete[] cur->init;
      
    if(cur==descr)
     {descr=cur->next;	
      delete cur;
     }
    else
     {for(cur1=descr; cur1->next!=cur; cur1=cur1->next);
      cur1->next=cur->next;
      delete cur;
     } 
     
    }
   if(strcmp(lab,"") )
    {
     delete[] cur->label;
     cur->label=new char[strlen(lab)+1];
     strcpy(cur->label, lab);
    } 
   if(strcmp(type,"") )
    {
     delete[] cur->type;
     cur->type=new char[strlen(type)+1];
     strcpy(cur->type, type);
    }
   if(strcmp(text,"") )
    {
     delete[] cur->text;
     cur->text=new char[strlen(text)+1];
     strcpy(cur->text, text);
    } 
   if(strcmp(init,"") )
    {
     if(cur->init!=NULL)
       delete[] cur->init;
     cur->init=new char[strlen(init)+1];
     strcpy(cur->init, init);
    } 

   return;
  
   }
 }
}

void add_description(char const *lab, char const *type, char const *text)
{
description *cur;

if(descr==NULL)
 cur=descr=new description;
else
{ for(cur=descr; cur->next!=NULL; cur=cur->next);
  
  cur->next=new description;
  cur=cur->next;
 
}  

cur->next=NULL;
cur->label=new char[strlen(lab)+1];
strcpy(cur->label, lab);
cur->type=new char[strlen(type)+1];
strcpy(cur->type, type);
if(text!=NULL && strlen(text)!=0)
 {cur->text=new char[strlen(text)+1]; 
  strcpy(cur->text, text);
 }
else
 {cur->text=new char[29]; 
  strcpy(cur->text, "(no description available)");
 }
   
cur->init=(char)NULL;

}

#ifndef NO_WINDOW

void change_descr_text(char *lab)
{
description *cur, *cur1;
char *lab1;

for(cur=descr; cur!=NULL; cur=cur->next)
 {
  if(!strcmp(cur->label, lab) )
   {
     delete[] cur->text;
     lab1=(char *)Tcl_GetVar(inter, "text_description",0);
     cur->text = new char[strlen(lab1)+1];
     strcpy(cur->text, lab1);
     return;

   }
 }
}

void change_init_text(char *lab)
{
description *cur, *cur1;
char *lab1;

for(cur=descr; cur!=NULL; cur=cur->next)
 {
  if(!strcmp(cur->label, lab) )
   {
     lab1=(char *)Tcl_GetVar(inter, "text_description",0);
     if(strlen(lab1)>0)
      {
      if(cur->init!=NULL)
        delete[] cur->init;
      cur->init = new char[strlen(lab1)+1];
      strcpy(cur->init, lab1);
      }
     return;

   }
 }
}

void show_description(char *lab)
{

description *cur;
int i;
Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);
sprintf(msg, "if { [winfo exists .desc_%s]==1} {set i 1} {set i 0}", lab);
cmd(inter, msg);
Tcl_UnlinkVar(inter, "i");
if(i==1)
 {
  
  sprintf(msg, "set vname %s",lab);
  cmd(inter, msg);
  cmd(inter, "set raise_description 1");
  cmd(inter, "raise .desc_$vname .");
  return;
 }
 
 cmd(inter, "set w .desc_$vname");
 cmd(inter, "toplevel $w");
 cmd(inter, "wm transient $w .");
 cur=search_description(lab);
 if(!strcmp(cur->type,"Parameter") )
   cmd(inter, "wm title $w \"Description: Parameter $vname\"");
 else
   cmd(inter, "wm title $w \"Description: Variable $vname\"");  
 cmd(inter, "frame $w.f");
 cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
 cmd(inter, "text $w.f.text -wrap word -width 60 -height 5 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
 cmd(inter, "pack $w.f.yscroll -side right -fill y");
 cmd(inter, "pack $w.f.text -expand yes -fill both");
 cmd(inter, "pack $w.f");
 sprintf(msg, "$w.f.text insert end \"%s\"",cur->text);
 cmd(inter, msg);
 cmd(inter, "frame $w.b");
 sprintf(msg, "button $w.b.close -text Close -command {destroy .desc_%s}",lab);
 cmd(inter, msg); 
 sprintf(msg, "button $w.b.save -text Save -command {set vname %s; raise .desc_%s; set raise_description 1; set text_description [.desc_%s.f.text get 1.0 end]; set choice 45}",lab, lab, lab);
 cmd(inter, msg); 
 sprintf(msg, "button $w.b.eq -text Equation -command {set vname %s; raise .desc_%s; set raise_description 1; set choice 46}",lab, lab);
 cmd(inter, msg); 
 sprintf(msg, "button $w.b.us -text Used -command {set vname %s; raise .desc_%s; set raise_description 1; set choice 47}",lab, lab);
 cmd(inter, msg); 
 sprintf(msg, "button $w.b.help -text Help -command {LsdHelp equation.html; set raise_description 1}");
 cmd(inter, msg); 


 if(!strcmp(cur->type, "Parameter"))
   cmd(inter, "pack $w.b.close $w.b.save $w.b.us $w.b.help -side left -expand yes");
 else
   cmd(inter, "pack $w.b.close $w.b.save $w.b.eq $w.b.us $w.b.help -side left -expand yes");
  
 cmd(inter, "pack $w.b -fill x -expand yes");



sprintf(msg, "set vname %s",lab);
cmd(inter, msg);
cmd(inter, "set raise_description 1");
//cmd(inter, "wm resizable .desc_$vname 0 0");
cmd(inter, "raise .desc_$vname .");


}
void empty_descr(void)
{
description *cur, *cur1;
for(cur1=descr; cur1!=NULL; cur1=cur)
  {cur=cur1->next;
  delete[] cur1->label;
  delete[] cur1->type;
  delete[] cur1->text;
  delete cur1;
  }
descr=NULL;
}


void auto_document(int *choice, char const *lab, char const *which)
{
FILE *f;
description *cd;
char str1[400], app[3000];
int i, j=0, done;

for(cd=descr; cd!=NULL; cd=cd->next)
 {
  if( (lab==NULL && (!strcmp(which, "ALL")||!strcmp(cd->type,"Variable") ||!strcmp(cd->type,"Function"))) || (lab!=NULL && !strcmp(lab, cd->label)) )
  { 
  app[0]=(char)NULL;
  //for each description
  if(!strcmp(cd->type, "Variable")==1 || !strcmp(cd->type, "Function")==1 )
   { //if it is a Variable
   
    sprintf(msg, "EQUATION(\"%s\")", cd->label); //search its equation
    f=search_str_nospaces(equation_name, msg);
    if(f==NULL)
     {sprintf(msg, "FUNCTION(\"%s\")", cd->label);
      f=search_str_nospaces(equation_name, msg);
     }
    if(f==NULL)
     {sprintf(msg, "if(!strcmp(label,\"%s\"))", cd->label);
      f=search_str_nospaces(equation_name, msg);
     }
    if(f!=NULL)
     {
     done=-1;
     j=0;
     while(done!=1)
      {
      fgets(str1, 400, f);
      for(i=0; str1[i]!=(char)NULL && done!=1; i++)
       {
       if(done==-1) //no comment found yet
        {
         if(isalpha(str1[i])!=0) //no comment exists
          done=1;
         if(str1[i]=='/' && str1[i+1]=='*')
          { done=0; //beginning of a multiline comment
           i+=2;
          } 
         if(str1[i]=='/' && str1[i+1]=='/')
          { done=2; //beginning of a single line comment
           i+=2;
          } 

        }
        if(done==0 ) //we are in a comment
         {
         if(str1[i]=='*' && str1[i+1]=='/')
          done=1;
          
         }
        if(done==0 || done ==2)
         {
         if(str1[i]!='\r')
           app[j++]=str1[i];
         }
        if(done==2 && str1[i]=='\n')
         done=-1; 
       }
      }
       strcat(app, "\n");
       
       
     } //end of the if(found equation)
   
  } //end of the if(cd==Variable)
  
  return_where_used(cd->label, str1); 
  sprintf(msg, "'%s' appears in the equation for: ",cd->label);
  app[j++]='\n';
  app[j]=(char)NULL; //close the string
  strcat(app, msg);
  strcat(app, str1);
  strcat(app, ".");
  delete[] cd->text;
  cd->text= new char[strlen(app)+1];
  strcpy(cd->text, app);
  } //end of the label to document
 }//end of the for(desc)

}

char const *return_where_used(char *lab, char s[]) 
{
int choice;
char *r; 

scan_used_lab(lab, &choice);
cmd(inter, "set l [join [$list.l get 0 end] \", \"]");
cmd(inter, "destroy $list"); 
r=(char *)Tcl_GetVar(inter, "l",0);
strcpy(s, r);

return(""); //just to avoind a warning of no return
}





/*
init_lattice
Create a new run time lattice having:
- pix=maximum pixel (600 should fit in typical screens)
- nrow= number of rows
- ncol= number of columns
- lrow= label of variable or parameter indicating the row value
- lcol= label of variable or parameter indicating the column value
- lvar= label of variable or parameter from which to read the color of the cell
- p= pointer of the object containing the initial color of the cell (if flag==-1)
- init_color= indicate the type of initialization. If init_color==-1, the function uses the lvar values reading the coordinates in each lrow and lcol to initialize the lattice. Otherwise, the lattice is homogeneously initialized to the color specified by init_color
*/
double dimW, dimH;
double init_lattice(double pixW, double pixH, double nrow, double ncol, char const lrow[], char const lcol[], char const lvar[], object *p, int init_color)
{
object *cur;
double i, j,color;

init_canvas();
dimH=pixH/nrow;
dimW=pixW/ncol;
cmd(inter, "if {[winfo exists .lat]==1} {destroy .lat} {}");
//create the window with the lattice, roughly 600 pixels as maximum dimension
cmd(inter, "toplevel .lat");
cmd(inter, "if {$tcl_platform(platform) != \"windows\"} {wm iconbitmap .lat @$RootLsd/$LsdSrc/lsd.xbm} {}");
sprintf(msg, "wm title .lat \"Lsd lattice (%.0lf x %.0lf)\"",nrow, ncol);
cmd(inter, msg);

cmd(inter, "set lat_update 1");
cmd(inter, "bind .lat <1> {if {$lat_update == 1 } {set lat_update 0; } {set lat_update 1} }");
cmd(inter, "bind .lat <3> {set a [tk_getSaveFile ]; if {$a != \"\" } {.lat.c postscript -colormode color -file \"$a\"} {} }");
cmd(inter, "bind .lat <2> {set a [tk_getSaveFile ]; if {$a != \"\" } {.lat.c postscript -colormode color -file \"$a\"} {} }");

//sprintf(msg, "canvas .lat.c -height %d -width %d -bg $c%d", (int)(dim*nrow), (int)(dim*ncol),init_color);
if(init_color==1001)
{
sprintf(msg, "canvas .lat.c -height %d -width %d -bg white", (int)pixH, (int)pixW);
cmd(inter, msg);

sprintf(msg, ".lat.c create rect 0 0 %d %d -fill white", (int)pixW, (int)pixH);
cmd(inter, msg);
}
else
{
sprintf(msg, "canvas .lat.c -height %d -width %d -bg $c%d", (int)pixH, (int)pixW,init_color);
cmd(inter, msg);

sprintf(msg, ".lat.c create rect 0 0 %d %d -fill $c%d", (int)pixW, (int)pixH,init_color);
cmd(inter, msg);
}
cmd(inter, "pack .lat.c");

sprintf(msg, "set dimH %lf", dimH);
cmd(inter, msg);
sprintf(msg, "set dimW %lf", dimW);
cmd(inter, msg);

if(lattice_type==1)
{
for(i=1; i<=nrow; i++)
 {
  for(j=1; j<=nrow; j++)
   {
    sprintf(msg, ".lat.c addtag c%d_%d withtag [.lat.c create poly %d %d %d %d %d %d %d %d -fill $c%d",(int)i,(int)j, (int)((j-1)*dimW), (int)((i - 1)*dimH), (int)((j-1)*dimW), (int)((i)*dimH), (int)((j)*dimW), (int)((i )*dimH), (int)((j)*dimW), (int)((i - 1)*dimH), init_color);
   cmd(inter, msg);

   }
 }
} 
return(0);

}

/*
update_lattice.
update the cell line.col to the color val
*/
double update_lattice(double line, double col, double val)
{
/* *
 sprintf(msg, ".lat.c itemconfigure c%d_%d -fill $c%d",(int)line, (int)col, (int)val);
/**/ 
 if(lattice_type==1)
 {
 sprintf(msg, ".lat.c itemconfigure c%d_%d -fill $c%d",(int)line, (int)col, (int)val);
 cmd(inter, msg);
 return 0;
}

//sprintf(msg, ".lat.c dtag [.lat.c create rect %d %d %d %d -outline $c%.0lf -fill $c%.0lf]", (int)((col-1)*dimW), (int)((line - 1)*dimH), (int)(col*dimW), (int)(line*dimH), val,val);
//sprintf(msg, ".lat.c dtag [.lat.c create poly %d %d %d %d %d %d %d %d -fill $c%.0lf]", (int)((col-1)*dimW), (int)((line - 1)*dimH), (int)((col-1)*dimW), (int)((line)*dimH), (int)((col)*dimW), (int)((line )*dimH), (int)((col)*dimW), (int)((line - 1)*dimH), val);
sprintf(msg, ".lat.c create poly %d %d %d %d %d %d %d %d -fill $c%.0lf", (int)((col-1)*dimW), (int)((line - 1)*dimH), (int)((col-1)*dimW), (int)((line)*dimH), (int)((col)*dimW), (int)((line )*dimH), (int)((col)*dimW), (int)((line - 1)*dimH), val);
cmd(inter, msg);
cmd(inter, "if {$lat_update == 1} {update} {}");
return 0;  


}
#endif

void execmd(char *str)
{
#ifndef NO_WINDOW
cmd(inter, str);
#endif
}

void kill_initial_newline(char *s)
{
char *d;
int i, j;
j=strlen(s);

d=new char[j+1];

for(i=0; i<j; i++)
 {
  if(s[i]!='\n')
   {
    break;
   }
 }

strcpy(d, s+i);
strcpy(s,d);
}
void kill_trailing_newline(char *s)
{
int i, done=0;
kill_initial_newline(s);

while(done==0)
 { done=1;
  for(i=0; s[i]!=(char)NULL; i++)
   {
    if(s[i]=='\n' && s[i+1]==(char)NULL)
     {s[i]=(char)NULL;
      done=0;
     } 
   }
 }
}

void clean_spaces(char *s)
{
int i, j;
char app[2000];

for(j=0, i=0; s[i]!=(char)NULL; i++)
 {
 switch(s[i])
  {
  case ' ':
  case '\t':
   break;
  default: 
     app[j++]=s[i];
     break;
  }
 }   
app[j]=(char)NULL;
strcpy(s, app);
}

#ifndef NO_WINDOW
void set_window_size(void)
{
cmd(inter, "update");
cmd(inter, "set reqw [winfo reqw .]");
cmd(inter, "set reqh [winfo reqh .]");
cmd(inter, "set reqsize [format %dx%d $reqw $reqh]");
cmd(inter, "wm geom . $reqsize");

}

void read_eq_filename(char *s)
{
FILE *f;
char lab[200];

f=fopen("makefile", "r");
if(f==NULL)
 {
  cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"File 'makefile' not found.\\nCannot upload the equation file.\"");
  return;
 }
fscanf(f, "%s", lab);
while(strncmp(lab, "FUN=", 4) && fscanf(f, "%s", lab)!=EOF);    
fclose(f);
if(strncmp(lab, "FUN=", 4)!=0)
 {
  cmd(inter, "tk_messageBox -type ok -title \"Error\" -message \"Makefile corrupted. Check Model Compilation options.\"");
  return;
 }

strcpy(s, lab+4);
strcat(s, ".cpp");

return;
    
}


int compare_eqfile(void)
{
FILE *f;
char *s, lab[200];
int i;

read_eq_filename(lab);
f=fopen(lab, "r");
s=new char[1000000];
while( fgets(msg, 1000, f)!=NULL)
   strcat(s, msg);
fclose(f);  
if(strcmp(s,eq_file)==0)
 i=0;
else
 i=1;
delete[] s;

return i;

}
#endif

void save_eqfile(FILE *f)
{
FILE *o;
char *lab, s[200];
int i;
i=strlen(lsd_eq_file);
if(i==0)
 strcpy(lsd_eq_file, eq_file);
 
fprintf(f, "\nEQ_FILE\n");
fprintf(f, "%s", lsd_eq_file);
fprintf(f, "\nEND_EQ_FILE\n");
return;

}

#ifndef NO_WINDOW
char *upload_eqfile(void)
{
//load into the string eq_file the equation file
char s[200], *eq;
int i;
FILE *f;

Tcl_LinkVar(inter, "eqfiledim", (char *) &i, TCL_LINK_INT);

read_eq_filename(s);
sprintf(msg, "set eqfiledim [file size %s]",s);
cmd(inter, msg);

eq=new char[i+1];
Tcl_UnlinkVar(inter, "i");
eq[0]=(char)NULL;
f=fopen(s, "r");
while( fgets(msg, 1000, f)!=NULL)
  strcat(eq, msg);
fclose(f);  
return eq;
}

#endif

