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



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int init(object *_up, char *_label, int _num_lag, double *val, int _save);
double cal(object *caller, int lag);
double fun(object *caller);
void empty(void);
};

class mnode
{
public:

mnode *son;
object *pntr;

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
int to_compute;

double cal(object *caller,  char const *l, int lag);
double cal( char const *l, int lag);

variable *search_var(object *caller,char const *label);
object *search_var_cond(char const *lab, double value, int lag);

double overall_max(char *lab, int lag);
double sum(char *lab, int lag);
double whg_av(char *lab, char *lab2, int lag);

int init(object *_up, char const *_label);
void update(void);
object *hyper_next(char const *lab);
void add_var(char *label, int lag, double *val, int save);
void add_obj(char *label, int num, int propagate);
void insert_parent_obj_one(char *lab);

object *search(char const *lab);
void chg_lab(char *lab);
void chg_var_lab(char *old, char *n);
variable *add_empty_var(char *str);
void add_var_from_example(variable *example);

void replicate(int num, int propagate);
int load_param(char *file_name, int repl, FILE *f);
void load_struct(FILE *f);
void save_param(FILE *f);
void save_struct(FILE *f, char *tab);
int read_param(char *file_name);

void sort_asc( object *from, char *l_var);
void sort_desc( object *from, char *l_var);
void sort(char *obj, char *var, char *direction);
void lsdqsort(char const *obj, char const *var, char const *direction);
void lsdqsort(char const *obj, char const *var1, char const *var2, char const *direction);
void empty(void);
void delete_obj(void);
void stat(char *lab, double *v);

object *add_n_objects2(char const *lab, int n, object *ex);
object *add_n_objects2(char const *lab, int n);

void write(char const *lab, double value, int time);
object *draw_rnd(char const *lo, char const *lv, int lag);
object *draw_rnd(char const *lo);
object *draw_rnd(char const *lo, char const *lv, int lag, double tot);
object *find_the_one(char *lv, double value);
double increment(char const *lv, double value);
double multiply(char *lv, double value);
object *lat_up(void);
object *lat_down(void);
object *lat_left(void);
object *lat_right(void);
double interact(char const *text, double v, double *tv);
void initturbo(char *label, double num);//set the structure to use the turbosearch
object *turbosearch(char *label, double tot, double num);

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
 int nvalues;
 int i;
 double *v;
 sense *next;
};

