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



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

// comment the next line to compile without libz. It will not be possible to generate zipped result files.
#define LIBZ 
#ifdef LIBZ
#include <zlib.h>
#endif

// redefine NAN to use faster non-signaling NaNs
#if has_quiet_NaN 
#undef NAN
#define NAN quiet_NaN()
#endif
#define NaN NAN

#define DUAL_MONITOR true		// define this variable to better handle dual-monitor setups

//class speedup;
class object;
class variable
{
public:
char *label;
object *up;
double *val;
int last_update;
int num_lag;
variable *next;
bool save;
bool savei;
bool under_computation;
int param;
char debug;
int deb_cond;
double deb_cnd_val;
char data_loaded;
char computable;
bool plot;
double *data;
char *lab_tit;
int start;
int end;
//speedup *su;

int init(object *_up, char const *_label, int _num_lag, double *val, int _save);
double cal(object *caller, int lag);
double fun(object *caller);
void empty(void);
};

class mnode
{
public:

mnode *son;
object *pntr;
long deflev;		// saves the log of number of objects to allow defaulting

void create(double level);
object *fetch(double *n, double level);
};

class bridge
{

public:

object *head;
char *blabel;
bridge *next;
bool counter_updated;
mnode *mn;
};

// network data structures
struct netLink		// individual outgoing link
{
	long serTo;		// destination node serial number (fixed)
	object *ptrTo;	// pointer to destination number
	object *ptrFrom;// network node containing the link
	netLink *prev;	// pointer to next link (NULL if first)
	netLink *next;	// pointer to next link (NULL if last)
	double weight;	// link weight
	double probTo;	// destination node draw probability
	
	netLink( object *origNode, object *destNode, double linkWeight = 0, double destProb = 1 ); 
					// constructor
	~netLink( void ); // destructor
};

struct netNode		// network node data
{
	long id;		// node unique ID number (reorderable)
	char *name;		// node textual name (not required)
	long serNum;	// node serial number (initial order, fixed)
	long nLinks;	// number of arcs FROM node
	netLink *first;	// first link in the linked list of links
	netLink *last;	// last link in the linked list of links
	double prob;	// assigned node draw probability
	
	netNode( long nodeId = -1, char const nodeName[] = "", double nodeProb = 1 );
					// constructor
	~netNode( void );// destructor
};

class object
{
public:

char *label;
object *up;
object *next;
object *hook;
variable *v;
bridge *b;

int acounter;
int lstCntUpd;		// period of last counter update (to avoid multiple updates)
int to_compute;

netNode *node;		// pointer to network node data structure

double cal(object *caller,  char const *l, int lag);
double cal( char const *l, int lag);

variable *search_var(object *caller,char const *label);
object *search_var_cond(char const *lab, double value, int lag);

double overall_max(char const *lab, int lag);
double sum(char const *lab, int lag);
double whg_av(char const *lab, char const *lab2, int lag);

int init(object *_up, char const *_label);
void update(void);
object *hyper_next(char const *lab);
void add_var(char const *label, int lag, double *val, int save);
void add_obj(char const *label, int num, int propagate);
void insert_parent_obj_one(char const *lab);

object *search(char const *lab);
void chg_lab(char const *lab);
void chg_var_lab(char const *old, char const *n);
variable *add_empty_var(char const *str);
void add_var_from_example(variable *example);

void replicate(int num, int propagate);
int load_param(char *file_name, int repl, FILE *f);
void load_struct(FILE *f);
void save_param(FILE *f);
void save_struct(FILE *f, char const *tab);
int read_param(char *file_name);

void sort_asc( object *from, char *l_var);
void sort_desc( object *from, char *l_var);
void sort(char const *obj, char const *var, char *direction);
void lsdqsort(char const *obj, char const *var, char const *direction);
void lsdqsort(char const *obj, char const *var1, char const *var2, char const *direction);
void empty(void);
void delete_obj(void);
void stat(char const *lab, double *v);

object *add_n_objects2(char const *lab, int n, object *ex);
object *add_n_objects2(char const *lab, int n);

void write(char const *lab, double value, int time);//write value as if computed at time
void write(char const *lab, double value, int time, int lag);//write value in the lag field
object *draw_rnd(char const *lo, char const *lv, int lag);
object *draw_rnd(char const *lo);
object *draw_rnd(char const *lo, char const *lv, int lag, double tot);
object *find_the_one(char const *lv, double value);
double increment(char const *lv, double value);
double multiply(char const *lv, double value);
object *lat_up(void);
object *lat_down(void);
object *lat_left(void);
object *lat_right(void);
double interact(char const *text, double v, double *tv);
void initturbo(char const *label, double num);//set the structure to use the turbosearch
object *turbosearch(char const *label, double tot, double num);

// set the network handling methods
netNode *add_node_net( long id, char const *nodeName, double prob );
void delete_node_net( void );
void name_node_net( char const *nodeName );
void stats_net( char const *lab, double *r );
object *search_node_net( char const *lab, long id ); 
object *draw_node_net( char const *lab ); 
object *shuffle_nodes_net( char const *lab );
netLink *add_link_net( object *destPtr, double weight, double probTo );
void delete_link_net( netLink *ptr );
netLink *search_link_net( long id ); 
long read_file_net( char const *lab, char const *dir, char const *base_name, char const *ext, int serial );
long write_file_net( char const *lab, char const *dir, char const *base_name, char const *ext, int serial );
long init_stub_net( char const *lab, const char* gen, long numNodes, long par1, double par2 );
long init_discon_net( char const *lab, long numNodes );
long init_random_dir_net( char const *lab, long numNodes, long numLinks );
long init_random_undir_net( char const *lab, long numNodes, long numLinks );
long init_uniform_net( char const *lab, long numNodes, long outDeg );
long init_circle_net( char const *lab, long numNodes, long outDeg );
long init_renyi_erdos_net( char const *lab, long numNodes, double linkProb );
long init_small_world_net( char const *lab, long numNodes, long outDeg, double rho );
long init_scale_free_net( char const *lab, long numNodes, long outDeg, double expLink );
};

struct lsdstack
{
lsdstack *prev;
lsdstack *next;
char label[100];
int ns;
variable *vs;
};

struct description
{
char *label;
char *type;
char *text;
char *init;
char initial;
char observe;
description *next;
};

struct sense
{
 char *label;
 int param;			// save element type/lag to allow
 int lag;			// handling lags > 1
 int nvalues;
 int i;
 double *v;
 sense *next;
 bool entryOk;		// flag valid data entered
};

// design of experiment object
struct design 
{ 
	int typ, tab, n, k, *par, *lag;		// experiment parameters
	double *hi, *lo, **ptr; 
	char **lab;

	design( sense *rsens, int typ = 1, char const *fname = "" );// constructor
	~design( void );					// destructor
};

// results file object
class result
{
	FILE *f;					// uncompressed file pointer
	#ifdef LIBZ
		gzFile fz;				// compressed file pointer
	#endif
	bool dozip;					// compressed file flag

	void title_recursive( object *r, int i );	// write file header (recursively)
	void data_recursive( object *r, int i );	// save a single time step (recursively)

public:
	result( char const *fname, char const *fmode, bool dozip = false );	// constructor
	~result( void );							// destructor
	void title( object *root, int flag );		// write file header
	void data( object *root, int initstep, int endtstep = 0 );	// write data
};