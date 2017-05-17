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

/***************************************************
****************************************************
FUN_HEAD.CPP

This file contains all the declarations and macros available in a model's equation file.


****************************************************
****************************************************/


#include "choose.h"

#include "decl.h"
#include <time.h>

extern double i_values[100];
#ifndef NO_WINDOW
#include <tk.h>

#endif

#ifndef NAMESPACE			// handle bug in some standard c libraries
#define NAMESPACE			// macro has to be defined BEFORE including this
#endif

extern object *root;
extern int seed;
extern long idum;
extern int sim_num;
extern char *simul_name;	// configuration name being run (for saving networks)
extern char *path;			// folder where the configuration is


void error(char *);
double log(double v);
double exp(double c);
double fact( double x );								// Factorial function
double ran1(long *idum);
#define RND (double)ran1(&idum)
double poidev(double xm, long *idum);
double poisson(double m) {return poidev(m, &idum); };
double poissoncdf( double lambda, double k );			// poisson cumulative distribution function
double gamdev(int ia, long *idum);
double gamma(double m) {return gamdev((int)m, &idum);};
double beta( double alpha, double beta );				// draw from a beta distribution
double betacf( double a, double b, double x );			// beta distribution function
double betacdf( double alpha, double beta, double x );	// beta cumulative distribution function
double alapl( double mu, double alpha1, double alpha2 );// draw from an asymmetric laplace distribution
double alaplcdf( double mu, double alpha1, double alpha2, double x );	// asymmetric laplace cumulative distribution function
double lnorm( double mu, double sigma );				// draw from a lognormal distribution
double lnormcdf( double mu, double sigma, double x );	// lognormal cumulative distribution function
double unifcdf( double a, double b, double x );			// uniform cumulative distribution function
void error_cycle(char const *l);

int deb(object *r, object *c, char const *lab, double *res);

double norm(double mean, double dev);
double normcdf( double mu, double sigma, double x );	// normal cumulative distribution function
double max(double a, double b);
double min(double a, double b);
double round(double r);
double rnd_integer(double m, double x);
double _abs(double a)
{if(a>0)
	return a;
  else
	return(-1*a);
};
#define abs( a ) _abs( a )	// redefine as macro to avoid conflicts with C++ version in <cmath>

extern int t;
extern int max_step;
extern int quit;
extern char msg[];
extern int debug_flag;
extern bool use_nan;	// flag to allow using Not a Number value
extern int fast;		// expose the global logging control variable


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
object register *cur=NULL, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cur10, *cyccur, *cyccur2, *cyccur3; \
cur=cur1=cur2=cur3=cur4=cur5=cyccur=NULL; \
netLink *curl, *curl1, *curl2, *curl3, *curl4, *curl5, *curl6, *curl7, *curl8, *curl9; \
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
if( ((!use_nan && NAMESPACE isnan(res)) || NAMESPACE isinf(res)==1) && quit!=1) \
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
if( ((!use_nan && NAMESPACE isnan(res)) || NAMESPACE isinf(res)==1) && quit!=1) \
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
#define VS(X,Y) X->cal(X,(char*)Y,0)
#define VLS(X,Y,Z) X->cal(X,(char*)Y,Z)

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

#define CYCLE2_SAFES(C,O,L) for(O=C->search((char*)L), cyccur2=go_brother(O); \
O!=NULL;O=cyccur2, cyccur2!=NULL?cyccur2=go_brother(cyccur2):cyccur2=cyccur2)
#define CYCLE2_SAFE(O,L) for(O=p->search((char*)L), cyccur2=go_brother(O); \
O!=NULL;O=cyccur2, cyccur2!=NULL?cyccur2=go_brother(cyccur2):cyccur2=cyccur2)

#define CYCLE3_SAFES(C,O,L) for(O=C->search((char*)L), cyccur3=go_brother(O); \
O!=NULL;O=cyccur3, cyccur3!=NULL?cyccur3=go_brother(cyccur3):cyccur3=cyccur3)
#define CYCLE3_SAFE(O,L) for(O=p->search((char*)L), cyccur3=go_brother(O); \
O!=NULL;O=cyccur3, cyccur3!=NULL?cyccur3=go_brother(cyccur3):cyccur3=cyccur3)


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

#define WRITE(X,Y) p->write((char*)X,Y,t)
#define WRITEL(X,Y,Z) p->write((char*)X,Y,Z)
#define WRITELL(X,Y,Z,L) p->write((char*)X,Y,Z,L)
#define WRITES(O,X,Y) O->write((char*)X,Y,t)
#define WRITELS(O,X,Y,Z) O->write((char*)X,Y,Z)
#define WRITELLS(O,X,Y,Z,L) O->write((char*)X,Y,Z,L)

#define SEARCH_CND(X,Y) p->search_var_cond((char*)X,Y,0)
#define SEARCH_CNDL(X,Y,Z) p->search_var_cond((char*)X,Y,Z)
#define SEARCH_CNDS(O,X,Y) O->search_var_cond((char*)X,Y,0)
#define SEARCH_CNDLS(O,X,Y,Z) O->search_var_cond((char*)X,Y,Z)

#define SEARCH(X) p->search((char*)X)
#define SEARCHS(Y,X) Y->search((char*)X)

// Seeds turbo search: O=pointer to container object where searched objects are
//                     X=name of object contained inside the searched objects
#define TSEARCHS_INI(O,X) O->initturbo((char*)X,0)
// Performs turbo search: O, X as in TSEARCHS_INI
//                        Y=position of object X to be searched for
#define TSEARCHS(O,X,Y) O->turbosearch((char*)X,0,Y)

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

// NETWORK MACROS

// create a network using as nodes object label X, located inside object O,
// applying generator Y, number of nodes Z, out-degree W and 
// parameter V
#define NETWORK_INI(X,Y,Z,W,V) (p->init_stub_net((char*)X,(char*)Y,(long)Z,(long)W,V))
#define NETWORKS_INI(O,X,Y,Z,W,V) (O==NULL?0.:O->init_stub_net((char*)X,(char*)Y,(long)Z,(long)W,V))

// read a network in Pajek format from file named Y/Z_xx.net (xx is the current seed) 
// using as nodes object with label X located inside object O
#define NETWORK_LOAD(X,Y,Z) (p->read_file_net((char*)X,(char*)Y,(char*)Z,"net",seed-1))
#define NETWORKS_LOAD(O,X,Y,Z) (O==NULL?0.:O->read_file_net((char*)X,(char*)Y,(char*)Z,"net",seed-1))

// save a network in Pajek format to file from the network formed by nodes
// with label X located inside object O with filename Y/Z (file name is Y/Z_xx.net)
#define NETWORK_SAVE(X,Y,Z) (p->write_file_net((char*)X,(char*)Y,(char*)Z,"net",seed-1))
#define NETWORKS_SAVE(O,X,Y,Z) (O==NULL?0.:O->write_file_net((char*)X,(char*)Y,(char*)Z,"net",seed-1))

// shuffle the nodes of a network composed by objects with label X, contained in O
#define SHUFFLE(X) p->shuffle_nodes_net((char*)X);
#define SHUFFLES(O,X) if(O!=NULL)O->shuffle_nodes_net((char*)X);

// random draw one node from a network composed by objects with label X, contained in O
#define RNDDRAW_NET(X) (p->draw_node_net((char*)X))
#define RNDDRAWS_NET(O,X) (O==NULL?NULL:O->draw_node_net((char*)X))

// get the number of nodes of network based on object X, contained in O
#define STAT_NET(X) p->stats_net((char*)X,v);
#define STATS_NET(O,X) if(O!=NULL)O->stats_net((char*)X,v);

// search node objects X, contained in O, for first occurrence of id=Y
#define SEARCH_NET(X,Y) (p->search_node_net((char*)X,(long)Y))
#define SEARCHS_NET(O,X,Y) (O==NULL?NULL:O->search_node_net((char*)X,(long)Y))

// get the id of the node object O
#define V_NODEID (p->node==NULL?0.:(double)p->node->id)
#define VS_NODEID(O) (O==NULL?0.:O->node==NULL?0.:(double)O->node->id)

// get the name of the node object O
#define V_NODENAME (p->node==NULL?"":p->node->name==NULL?"":p->node->name)
#define VS_NODENAME(O) (O==NULL?"":O->node==NULL?"":O->node->name==NULL?"":O->node->name)

// set the id of the node object O
#define WRITE_NODEID(X) if(p->node!=NULL)p->node->id=(double)X;
#define WRITES_NODEID(O,X) if(O!=NULL)if(O->node!=NULL)O->node->id=(double)X;

// set the name of the node object O to X
#define WRITE_NODENAME(X) p->name_node_net((char*)X);
#define WRITES_NODENAME(O,X) if(O!=NULL)O->name_node_net((char*)X);

// get the number of outgoing links from object O
#define STAT_NODE p->node==NULL?v[0]=0.:v[0]=(double)p->node->nLinks;
#define STATS_NODE(O) O==NULL?v[0]=0.:O->node==NULL?v[0]=0.:v[0]=(double)O->node->nLinks;

// add a link from object O to object X, both located inside same parent, same label
// and optional weight Y
#define ADDLINK(X) (p->add_link_net(X,0,1))
#define ADDLINKS(O,X) (O==NULL?NULL:O->add_link_net(X,0,1))
#define ADDLINKW(X,Y) (p->add_link_net(X,Y,1))
#define ADDLINKWS(O,X,Y) (O==NULL?NULL:O->add_link_net(X,Y,1))

// delete the link pointed by O
#define DELETELINK(O) if(O!=NULL)O->ptrFrom->delete_link_net(O);

// search outgoing links from object O for first occurrence of id=X
#define SEARCH_LINK(X) (p->search_link_net((long)X))
#define SEARCHS_LINK(O,X) (O==NULL?NULL:O->search_link_net((long)X))

// get the destination object of link pointed by O
#define LINKTO(O) (O==NULL?NULL:O->ptrTo)

// get the destination object of link pointed by O
#define LINKFROM(O) (O==NULL?NULL:O->ptrFrom)

// get the weight of link pointed by O
#define VS_WEIGHT(O) (O==NULL?0.:O->weight)

// set the weight of link pointed by O to X
#define WRITES_WEIGHT(O,X) if(O!=NULL)O->weight=X;

// cycle through set of links of object C, using link pointer O
#define CYCLE_LINK(O) if(p->node==NULL)error_cycle("invalid node");else O=p->node->first;for(;O!=NULL;O=O->next)
#define CYCLES_LINK(C,O) if(C==NULL)error_cycle("invalid node");else if(C->node==NULL)error_cycle("invalid node");else O=C->node->first;for(;O!=NULL;O=O->next)

// EXTENDED DATA/ATTRIBUTES MANAGEMENT MACROS
// macros for handling extended attributes (usually lists) attached to objects' hook pointer

// add/delete extension c++ data structures of type CLASS to the LSD object pointer by current/PTR
#define ADD_EXT( CLASS ) p->hook = reinterpret_cast< object * >( new CLASS );
#define ADDS_EXT( PTR, CLASS ) PTR->hook = reinterpret_cast< object * >( new CLASS );
#define DELETE_EXT( CLASS ) delete reinterpret_cast< CLASS * >( p->hook );
#define DELETES_EXT( PTR, CLASS ) delete reinterpret_cast< CLASS * >( PTR->hook );
// convert current (or a pointer PTR from) LSD object type in the user defined CLASS type
#define P_EXT( CLASS ) ( reinterpret_cast< CLASS * >( p->hook ) )
#define PS_EXT( PTR, CLASS ) ( reinterpret_cast< CLASS * >( PTR->hook ) )
// read/write from object OBJ pointed by pointer current/PTR of type CLASS
#define V_EXT( CLASS, OBJ ) ( P_EXT( CLASS ) -> OBJ )
#define VS_EXT( PTR, CLASS, OBJ ) ( PS_EXT( PTR, CLASS ) -> OBJ )
#define WRITE_EXT( CLASS, OBJ, VAL ) ( P_EXT( CLASS ) -> OBJ = VAL )
#define WRITES_EXT( PTR, CLASS, OBJ, VAL ) ( PS_EXT( PTR, CLASS ) -> OBJ = VAL )
// execute METHOD contained in OBJ pointed by pointer current/PTR of type CLASS with the parameters ...
#define EXEC_EXT( CLASS, OBJ, METHOD, ... ) ( P_EXT( CLASS ) -> OBJ.METHOD( __VA_ARGS__ ) )
#define EXECS_EXT( PTR, CLASS, OBJ, METHOD, ... ) ( PS_EXT( PTR, CLASS ) -> OBJ.METHOD( __VA_ARGS__ ) )
// cycle over elements of OBJ pointed by pointer current/PTR of type CLASS using iterator ITER
#define CYCLE_EXT( ITER, CLASS, OBJ ) for ( ITER = EXEC_EXT( CLASS, OBJ, begin ); ITER != EXEC_EXT( CLASS, OBJ, end ); ++ITER )
#define CYCLES_EXT( PTR, ITER, CLASS, OBJ ) for ( ITER = EXECS_EXT( PTR, CLASS, OBJ, begin ); ITER != EXECS_EXT( PTR, CLASS, OBJ, end ); ++ITER )
