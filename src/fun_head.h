/***************************************************
****************************************************
LSD 6.3 - May 2014
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/

/***************************************************
****************************************************
FUN_HEAD.CPP

This file contains all the declarations and macros available in a model's equation file.


****************************************************
****************************************************/


#include "choose.h"

#include "decl.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

extern double i_values[100];
#ifndef NO_WINDOW
#include <tk.h>

#endif

extern object *root;
extern int seed;
extern long idum;
extern int sim_num;

void error(char *);
double log(double v);
double exp(double c);
double ran1(long *idum);
#define RND (double)ran1(&idum)
double poidev(double xm, long *idum);
double poisson(double m) {return poidev(m, &idum); };
double gamdev(int ia, long *idum);
double gamma(double m) {return gamdev((int)m, &idum);};
void error_cycle(char const *l);

int deb(object *r, object *c, char const *lab, double *res);

double norm(double mean, double dev);
double max(double a, double b);
double min(double a, double b);
double round(double r);
double rnd_integer(double m, double x);
double abs(double a)
{if(a>0)
	return a;
  else
	return(-1*a);
};

extern int t;
extern int max_step;
extern int quit;
extern char msg[];
extern int debug_flag;


void plog(char const *msg);
object *go_brother(object *c);

#ifndef NO_WINDOW
void cmd(Tcl_Interp *inter, char const *cc);
//void cmd(Tcl_Interp *inter, const char cc[]);
extern Tcl_Interp *inter;
double init_lattice(double pixW, double pixH, double nrow, double ncol, char const lrow[], char const lcol[], char const lvar[], object *p, int init_color);

double update_lattice(double line, double col, double val);

#endif
#define DEBUG f=fopen("log.log","a"); \
 fprintf(f,"t=%d %s\n",t,label); \
 fclose(f);

#define DEBUG_AT(X) if(t>=X) \
 { f=fopen("log.log","a"); \
   fprintf(f,"t=%d %s\n",t,label); \
   fclose(f); \
 }  

#define MODELBEGIN double variable::fun(object *caller) \
{ \
double res; \
object *p, *c, app; \
int i,j,h,k; \
double v[1000]; \
object register *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cur10, *cyccur; \
cur=cur1=cur2=cur3=cur4=cur5=cyccur=NULL; \
if(quit==2) \
 return -1; \
p=up; \
c=caller; \
FILE *f;

#define EQUATION(X) if(!strcmp(label,X)) {
#define FUNCTION(X) if(!strcmp(label,X)) { last_update--; if(c==NULL) {  res=val[0];  goto end; } }; \
 if(!strcmp(label,X)) {
#define END_EQUATION(X) {res=X; goto end; }


#ifndef NO_WINDOW

#define MODELEND sprintf(msg, "\nFunction for %s not found", label); \
plog(msg); \
sprintf(msg, "tk_messageBox -type ok -icon error -message \"Error trying to compute variable '%s': Equation not found.\n\nPossible problems:\n- There is no equation for variable '%s';\n- The spelling in equation's code is different from the name in the configuration;\n- The equation's code was terminated incorrectly.\n\nCheck the Lsd help.\"", label, label); \
cmd(inter, msg); \
quit=2; \
return -1; \
end : \
if( (isnan(res)==1 || isinf(res)==1) && quit!=1) \
 { \
  sprintf(msg, "At time %d the equation for '%s' produces the non-valid value '%lf'. Check the equation code and the temporary values v\\[...\\] to find the faulty line.",t, label, res ); \
  error(msg); \
  debug_flag=1; \
  debug='d'; \
 } \
if(debug_flag==1) \
 { \
 for(i=0; i<100; i++) \
  i_values[i]=v[i]; \
 } \
return(res); \
}

#else

#define MODELEND sprintf(msg, "\nFunction for %s not found", label); \
plog(msg); \
sprintf(msg, "Error trying to compute variable '%s': Equation not found.\n\nPossible problems:\n- There is no equation for variable '%s';\n- The spelling in equation's code is different from the name in the configuration;\n- The equation's code was terminated incorrectly.\n\nCheck the Lsd help.\"", label, label); \
printf("s",msg); \
exit(0); \
end : \
if( (isnan(res)==1 || isinf(res)==1) && quit!=1) \
 { \
  sprintf(msg, "At time %d the equation for '%s' produces the non-valid value '%lf'. Check the equation code and the temporary values v\\[...\\] to find the faulty line.",t, label, res ); \
  printf(msg); \
  exit(0); \
 } \
return(res); \
}
#endif

#define RESULT(X) res=X; goto end; }
#define CURRENT val[0]

#define V(X) p->cal(p,(char*)X,0)
#define VL(X,Y) p->cal(p,(char*)X,Y)
#define VS(X,Y) X->cal(p,(char*)Y,0)
#define VLS(X,Y,Z) X->cal(p,(char*)Y,Z)

#define V_CHEAT(X,C) p->cal(C,(char*)X,0)
#define VL_CHEAT(X,Y,C) p->cal(C,(char*)X,Y)
#define VS_CHEAT(X,Y,C) X->cal(C,(char*)Y,0)
#define VLS_CHEAT(X,Y,Z,C) X->cal(C,(char*)Y,Z)

#define SUM(X) p->sum((char*)X,0)
#define SUML(X,Y) p->sum((char*)X,Y)
#define SUMS(X,Y) X->sum((char*)Y,0)
#define SUMLS(X,Y,Z) X->sum((char*)Y,Z)

#define STAT(X) p->stat((char*)X, v)
#define STATS(O,X) O->stat((char*)X, v)

#define WHTAVE(X,W) p->whg_av((char*)W,(char*)X,0)
#define WHTAVEL(X,W,Y) p->whg_av((char*)W,(char*)X,Y)
#define WHTAVES(X,W,Y) X->whg_av((char*)W,(char*)Y,0)
#define WHTAVELS(X,W,Y,Z) X->whg_av((char*)W,(char*)Y,Z)


#define INCRS(Q,X,Y) Q->increment((char*)X,Y)
#define INCR(X,Y) p->increment((char*)X,Y)

#define MULT(X,Y) p->multiply((char*)X,Y)
#define MULTS(Q,X,Y) Q->multiply((char*)X,Y)




#define CYCLE_SAFES(C,O,L) for(O=C->search((char*)L), cyccur=go_brother(O); \
O!=NULL;O=cyccur, cyccur!=NULL?cyccur=go_brother(cyccur):cyccur=cyccur)
#define CYCLE_SAFE(O,L) for(O=p->search((char*)L), cyccur=go_brother(O); \
O!=NULL;O=cyccur, cyccur!=NULL?cyccur=go_brother(cyccur):cyccur=cyccur)


#define CYCLE(O,L) O=p->search((char*)L); \
if(O==NULL) error_cycle((char*)L); \
for(;O!=NULL;O=go_brother(O))

#define CYCLES(C,O,L) O=C->search((char*)L); \
if(O==NULL) error_cycle((char*)L); \
for(;O!=NULL;O=go_brother(O))


#define MAX(X) p->overall_max((char*)X,0)
#define MAXL(X,Y) p->overall_max((char*)X,Y)
#define MAXS(X,Y) X->overall_max((char*)X,0)
#define MAXLS(O,X,Y) O->overall_max((char*)X,Y)

#define WRITE(X,Y) p->write((char*)X,Y,0)
#define WRITEL(X,Y,Z) p->write((char*)X,Y,Z)
#define WRITES(O,X,Y) O->write((char*)X,Y,0)
#define WRITELS(O,X,Y,Z) O->write((char*)X,Y,Z)

#define SEARCH_CND(X,Y) p->search_var_cond((char*)X,Y,0)
#define SEARCH_CNDL(X,Y,Z) p->search_var_cond((char*)X,Y,Z)
#define SEARCH_CNDS(O,X,Y) O->search_var_cond((char*)X,Y,0)
#define SEARCH_CNDLS(O,X,Y,Z) O->search_var_cond((char*)X,Y,Z)

#define SEARCH(X) p->search((char*)X)
#define SEARCHS(Y,X) Y->search((char*)X)

// Seeds turbo search: O=pointer to container object where searched objects are
//                     X=name of variable contained inside the searched objects
#define INI_TSEARCHS(O,X) O->initturbo((char*)X,0)
// Performs turbo search: O, X as in INI_TSEARCHS
//                        Y=value of the variable X to be searched for
#define TSEARCH_CNDS(O,X,Y) O->turbosearch((char*)X,0,Y)

#define SORT(X,Y,Z) p->lsdqsort((char*)X,(char*)Y,(char*)Z)
#define SORTS(O,X,Y,Z) O->lsdqsort((char*)X,(char*)Y,(char*)Z)
#define SORT2(X,Y,L,Z) p->lsdqsort((char*)X,(char*)Y,(char*)L,(char*)Z)
#define SORTS2(O,X,Y,L,Z) O->lsdqsort((char*)X,(char*)Y,(char*)L,(char*)Z)

#define UNIFORM( X, Y ) ((X) + RND*((Y)-(X)))

#define ADDOBJ(X) p->add_n_objects2((char*)X,1)
#define ADDOBJS(X,Y) X->add_n_objects2((char*)Y,1)

#define ADDOBJ_EX(X,Y) p->add_n_objects2((char*)X,1,Y)
#define ADDOBJS_EX(O,X,Y) O->add_n_objects2((char*)X,1,Y)

#define ADDNOBJ(X,Y) p->add_n_objects2((char*)X, (int)Y)
#define ADDNOBJS(O,X,Y) O->add_n_objects2((char*)X, (int)Y)

#define ADDNOBJ_EX(X,Y,Z) p->add_n_objects2((char*)X, (int)Y, Z)
#define ADDNOBJS_EX(O,X,Y,Z) O->add_n_objects2((char*)X, (int)Y, Z)

#define DELETE(X) X->delete_obj()

#define RNDDRAW(X,Y) p->draw_rnd((char*)X, (char*)Y, 0)
#define RNDDRAWL(X,Y,Z) p->draw_rnd((char*)X, (char*)Y, Z)
#define RNDDRAWS(Z,X,Y) Z->draw_rnd((char*)X, (char*)Y,0)
#define RNDDRAWLS(O,X,Y,Z) O->draw_rnd((char*)X, (char*)Y, Z)

#define RNDDRAWFAIR(X) p->draw_rnd((char*)X)
#define RNDDRAWFAIRS(Z,X) Z->draw_rnd((char*)X)



#define RNDDRAWTOT(X,Y, T) p->draw_rnd((char*)X, (char*)Y, 0, T)
#define RNDDRAWTOTL(X,Y,Z, T) p->draw_rnd((char*)X, (char*)Y, Z, T)
#define RNDDRAWTOTS(Z,X,Y, T) Z->draw_rnd((char*)X, (char*)Y,0, T)
#define RNDDRAWTOTLS(O,X,Y,Z, T) O->draw_rnd((char*)X, (char*)Y, Z, T)

#define PARAMETER param=1;

#define INTERACT(X,Y)  p->interact((char*)X,Y, v)
#define INTERACTS(Z,X,Y) Z->interact((char*)X,Y, v)



