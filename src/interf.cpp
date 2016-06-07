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



/*
USED CASE 81
*/

/****************************************************
INTERF.CPP Manage the main interfaces, that the browser and all the menus.
It is re-build any time the window changes. There are some 20 actions that are
commanded from the browser window, implemented as a switch in operate.

The functions contained in this file are:

-object *create( object *root)
The main cycle for the Browser, from which it exits only to run a simulation
or to quit the program. The cycle is just once call to browsw followed by
a call to operate.

- int browse( object *r, int *choice);
build the browser window and waits for an action (on the form of
values for choice or choice_g different from 0)

- object *operate( int *choice, object *r);
takes the value of choice and operate the relative command on the
object r. See the switch for the complete list of the available commands

- void clean_debug(object *n);
remove all the flags to debug from any variable in the model

- void clean_save(object *n);
remove all the flags to save from any variable in the model

- void show_save(object *n)
shows all variables to be saved in the result files

- void clean_plot(object *n);
remove all the flags to plot from any variable in the model

- void wipe_out(object *d);
Eliminate all the Object like d from the model. Cancel also the their descendants


Functions used here from other files are:

- void plog(char *m);
LSDMAIN.CPP print  message string m in the Log screen.

- void analysis(int *choice);
ANALYSIS.CPP analysis of result files

- void show_eq(char *lab, int *choice);
SHOW_EQ.CPP shows one equation for variable lab

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

- int deb(object *r, object *c, char *lab, double *res);
Use the debugger interface to browse through the model

- int cd(char *patch)
The change directory function uses different headers for Windows and Unix
Therefore, it is defined in the mainwin/main_un.

- void myexit(int v);
Exit function, which is customized on the operative system.

****************************************************/



#include <sys/stat.h>
#include <time.h>
#include <tk.h>
#include "decl.h"

#ifndef ACCESSPERMS
#define ACCESSPERMS 0777 
#endif

#define MAX_SENS_POINTS 999		// default warning threshold for sensitivity analysis

object *operate( int *choice, object *r);
int browse( object *r, int *choice);
object *skip_next_obj(object *t, int *i);
object *skip_next_obj(object *t);
int my_strcmp(char *a, char *b);
void cmd(Tcl_Interp *inter, char const *cc);
//void cmd(Tcl_Interp *inter, const char cc[]) {cmd(inter, (char *)cc);};
object *go_brother(object *cur);
void show_graph( object *t);
void set_obj_number(object *r, int *choice);
void edit_data(object *root, int *choice, char *obj_name);
void clean_debug(object *n);
void clean_save(object *n);
void show_save(object *n);
void show_initial(object *n);
void show_observe(object *n);

void clean_plot(object *n);
FILE *search_str(char const *name, char const *str);
void plog(char const *msg);
void analysis(int *choice);
void show_eq(char *lab, int *choice);
void chg_obj_num(object **c, int value, int all, int pippo[], int *choice, int cfrom);
int deb(object *r, object *c, char const *lab, double *res);
void wipe_out(object *d);
int check_label(char *l, object *r);
int cd(char *path);
void set_blueprint(object *container, object *r);

void myexit(int i);
void scan_used_lab(char *lab, int *choice);
void scan_using_lab(char *lab, int *choice);
void report(int *choice, object *r);
void empty_cemetery(void);
void createmodelhelp(int *choice, object *r);
description *search_description(char *lab);
void change_descr_lab(char const *lab_old, char const *lab, char const *type, char const *text, char const *init);
void change_descr_text(char *lab);
void change_init_text(char *lab);
void empty_descr(void);
void show_description(char *lab);
void add_description(char const *lab, char const *type, char const *text);
void auto_document( int *choice, char const *lab, char const *which, bool append = false);
void create_logwindow(void);
void delete_bridge(object *d);
void control_tocompute(object *r, char *ch);
int compute_copyfrom(object *c, int *choice);
int reset_bridges(object *r);
char *choose_object( char *msg );
void insert_lb_object(object *r);
void autofill_descr(object *o);
void read_eq_filename(char *s);
void tex_report(object *r, FILE *f);
void tex_report_init(object *r, FILE *f);
void tex_report_observe(object *r, FILE *f);
void shift_var(int direction, char *vlab, object *r);
void shift_desc(int direction, char *dlab, object *r);
object *sensitivity_parallel(object *o, sense *s );
void sensitivity_sequential(long *findexSens, sense *s, double probSampl = 1.0);
void sensitivity_doe( long *findex, design *doe );
long num_sensitivity_points( sense *rsens );	// calculates the sensitivity space size
int num_sensitivity_variables( sense *rsens );	// calculates the number of variables to test
void empty_sensitivity(sense *cs);
void set_all(int *choice, object *original, char *lab, int lag);
void dataentry_sensitivity(int *choice, sense *s, int nval);
bool discard_change( bool checkSense = true, bool senseOnly = false );	// ask before discarding unsaved changes
void save_pos( object * );
object *restore_pos( object * );
int load_configuration( object * );
bool save_configuration( object *, long findex = 0 );

// comparison function for bsearch and qsort
int comp_ints ( const void *a, const void *b ) { return ( *( int * ) a - *( int * ) b ); }

extern object *root;
extern char *simul_name;
extern char *struct_file;
extern char *exec_file;		// name of executable file
extern char *exec_path;		// path of executable file
extern int add_to_tot;
extern int dozip;			// compressed results file flag
extern int struct_loaded;
extern char *path;
extern char *equation_name;
extern char name_rep[];
extern int debug_flag;
extern int stackinfo_flag;
extern int t;
extern int optimized;
extern int check_optimized;
extern int when_debug;
extern int running;

extern Tcl_Interp *inter;
extern int seed;
extern int sim_num;
extern int max_step;
extern char msg[];
char *res_g;
extern int choice_g;
extern int choice;
extern variable *cemetery;
int result_loaded;
extern int message_logged;
extern int actual_steps;
extern description *descr;
extern int no_error;
extern char *eq_file;
extern char lsd_eq_file[];
extern int ignore_eq_file;
extern int lattice_type;
extern int no_res;
extern int overwConf;
extern object *blueprint;
extern sense *rsense;
extern long nodesSerial;	// network node serial number global counter
extern bool unsavedData;	// control for unsaved simulation results

char lastObj[256]="";		// to save last shown object for quick reload (choice=38)

extern char *sens_file;		// current sensitivity analysis file
extern long findexSens;		// index to sequential sensitivity configuration filenames
extern int strWindowOn;		// control the presentation of the model structure window
extern bool justAddedVar;	// control the selection of last added variable
extern bool unsavedChange;	// control for unsaved changes in configuration
extern bool unsavedSense;	// control for unsaved changes in sensitivity data
extern bool redrawRoot;		// control for redrawing root window (.)

// list of choices that are bad with existing run data
int badChoices[] = { 1, 2, 3, 6, 7, 19, 21, 22, 25, 27, 28, 30, 31, 32, 33, 36, 43, 57, 62, 63, 64, 65, 68, 69, 71, 72, 74, 75, 76, 77, 78, 79 };
#define NUM_CHOICES ( sizeof( badChoices ) / sizeof( badChoices[ 0 ] ) )

 
/****************************************************
CREATE
****************************************************/
object *create( object *cr)
{
object *cur;
char *s;

// sort the list of choices that are bad with existing run data to use later
qsort( badChoices, NUM_CHOICES, sizeof ( int ), comp_ints );

redrawRoot = true;			// browser redraw when drawing the first time

cmd(inter, "set listfocus 1");
cmd(inter, "set itemfocus 0");

// restore previous object and cursor position in browser, if any
if ( strlen( lastObj ) > 0 )
{
	for ( cur = cr; cur->up != NULL; cur = cur->up );
	cur = cur->search( lastObj );
	if ( cur != NULL )
	{
		cr = cur;
		cmd(inter, "if [ info exists lastList ] { set listfocus $lastList }");
		cmd(inter, "if [ info exists lastItem ] { set itemfocus $lastItem }");
	}
}

cmd(inter, "set choice -1");
cmd(inter, "set c \"\"");
Tcl_LinkVar(inter, "strWindowOn", (char*)&strWindowOn, TCL_LINK_BOOLEAN);
Tcl_LinkVar(inter, "choice_g", (char *)&choice_g, TCL_LINK_INT);
choice_g=choice=0;
cmd(inter, "if { [winfo exist .log]==1} {wm resizable .log 1 1; raise .log; focus -force .log} {set choice -1}");
cmd(inter, "wm resizable . 1 1");
cmd(inter, "set cur 0"); //Set yview for vars listbox

sprintf(msg, "set ignore_eq_file %d", ignore_eq_file);
cmd(inter, msg);

sprintf(msg, "set lattype %d", lattice_type);
cmd(inter, msg);

while(choice!=1) //Main Cycle ********************************
{
if(choice==-1)
 {
 cmd(inter, "set choice [winfo exists .log]"); 
 if(choice==0)
   create_logwindow();
 choice=0;
 }

sprintf( msg, "wm title . \"%s%s - Lsd Browser\"", unsavedChange ? "*" : "", simul_name ) ;
cmd(inter, msg);
sprintf( msg, "wm title .log \"%s%s - Lsd Log\"", unsavedChange ? "*" : "", simul_name ) ;
cmd(inter, msg);

for(cur=cr; cur->up!=NULL; cur=cur->up);

if(cur->v==NULL && cur->b==NULL)
  struct_loaded=0;
else
 { struct_loaded=1;
     show_graph(cr);
 if(message_logged==1)
  {
  cmd(inter, "wm deiconify .log; raise .log; focus -force .log");
  message_logged=0;
  }    
 }    

cmd(inter, "raise .; focus -force .");
cmd(inter, "bind . <KeyPress-Escape> {}");
cmd(inter, "bind . <KeyPress-Return> {}");
cmd(inter, "bind . <Destroy> {set choice 35}");
cmd(inter, "bind .log <Destroy> {set choice 35}");

// browse only if not running two-cycle operations
if ( choice != 55 && choice != 75 && choice != 76 && choice != 77 && choice != 78 && choice != 79 )
  choice=browse(cr, &choice);

cr=operate( &choice, cr);

}

Tcl_UnlinkVar(inter,"save_option");
Tcl_UnlinkVar(inter, "choice_g");

cmd(inter, "if { [winfo exists .model_str] == 1} {wm withdraw .model_str} {}");
cmd(inter, "wm deiconify .log; wm deiconify .; raise .; focus -force .; update");
return(cr);
}



/****************************************************
BROWSE
****************************************************/
int browse(object *r, int *choice)
{
char ch[10000], ch1[10000];
variable *ap_v;
int count, heightB, widthB;
object *ap_o;
bridge *cb;

if ( redrawRoot )		// avoids redrawing if not required
{
cmd(inter, "destroy .l");

cmd(inter, "frame .l -relief groove -bd 2");
cmd(inter, "frame .l.v -relief groove -bd 2");
cmd(inter, "frame .l.s -relief groove -bd 2");

cmd(inter, "frame .l.v.c");
cmd(inter, "scrollbar .l.v.c.v_scroll -command \".l.v.c.var_name yview\"");
cmd(inter, "listbox .l.v.c.var_name -yscroll \".l.v.c.v_scroll set\"");

cmd(inter, "bind .l.v.c.var_name <Right> {focus -force .l.s.son_name; .l.s.son_name selection set 0}");

if(r->v==NULL)
  cmd( inter, ".l.v.c.var_name insert end \"(none)\"; set nVar 0" );
else
  {
  cmd(inter, "set app 0");
  for(ap_v=r->v; ap_v!=NULL; ap_v=ap_v->next)
	 {
	  if(ap_v->param==1)
		 sprintf(ch, ".l.v.c.var_name insert end \"%s (par.)\"",ap_v->label);
  	  if(ap_v->param==0)
		   sprintf(ch, ".l.v.c.var_name insert end \"%s (var. lag=%d)\"",ap_v->label, ap_v->num_lag);
  	  if(ap_v->param==2)
       sprintf(ch, " .l.v.c.var_name insert end \"%s (fun. lag=%d)\"",ap_v->label, ap_v->num_lag);
	  cmd(inter, ch);

	  if( ap_v->param == 0 && ap_v->num_lag == 0 )
		 sprintf(ch, ".l.v.c.var_name itemconf $app -fg blue");
	  if( ap_v->param == 0 && ap_v->num_lag > 0 )
		 sprintf(ch, ".l.v.c.var_name itemconf $app -fg purple");
	  if(ap_v->param==1)
		 sprintf(ch, ".l.v.c.var_name itemconf $app -fg black");
	  if( ap_v->param == 2 && ap_v->num_lag == 0 )
		 sprintf(ch, ".l.v.c.var_name itemconf $app -fg red");
	  if( ap_v->param == 2 && ap_v->num_lag > 0 )
		 sprintf(ch, ".l.v.c.var_name itemconf $app -fg tomato");
	  cmd(inter, ch);
    cmd(inter, "incr app");

	  if(ap_v->next == NULL && justAddedVar)	// last variable & just added a new variable?
	  {
		  justAddedVar=false;
		  cmd( inter, ".l.v.c.var_name selection clear 0 end; .l.v.c.var_name selection set end; set lst [ .l.v.c.var_name curselection ]; if { ! [ string equal $lst \"\" ] } { set res [ .l.v.c.var_name get $lst ]; set listfocus 1; set itemfocus $lst}");
	  }
	 }
	
	cmd( inter, "set nVar [ .l.v.c.var_name size ]" );
  }

cmd( inter, "label .l.v.lab -text \"Variables & Parameters ($nVar)\"" );

// variables context menu (right mouse button)
cmd( inter, "menu .l.v.c.var_name.v -tearoff 0" );
cmd( inter, ".l.v.c.var_name.v add command -label Change -command { set choice 7 }" );
cmd( inter, ".l.v.c.var_name.v add command -label Properties -command { set choice 75 }" );
cmd( inter, ".l.v.c.var_name.v add separator" );
cmd( inter, ".l.v.c.var_name.v add command -label \"Move Up\" -state disabled -command { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { $itemfocus > 0 } { incr itemfocus -1 }; set choice 58 }" );
cmd( inter, ".l.v.c.var_name.v add command -label \"Move Down\" -state disabled -command { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { $itemfocus < [ expr [ .l.v.c.var_name size ] - 1 ] } { incr itemfocus }; set choice 59 }" );
cmd( inter, ".l.v.c.var_name.v add separator" );
cmd( inter, ".l.v.c.var_name.v add command -label Move -command { set choice 79 }" );
cmd( inter, ".l.v.c.var_name.v add command -label Delete -command { set choice 76 }" );
cmd( inter, ".l.v.c.var_name.v add separator" );
cmd( inter, ".l.v.c.var_name.v add command -label Equation -state disabled -command { set choice 29 }" );
cmd( inter, ".l.v.c.var_name.v add command -label Using -state disabled -command { set choice 46 }" );
cmd( inter, ".l.v.c.var_name.v add command -label \"Used In\" -state disabled -command { set choice 47 }" );
cmd( inter, ".l.v.c.var_name.v add separator" );
cmd( inter, ".l.v.c.var_name.v add command -label \"Initial Values\" -state disabled -command { set choice 77 }" );
cmd( inter, ".l.v.c.var_name.v add command -label Sensitivity -state disabled -command { set choice 78 }" );

if(r->v!=NULL)
  {
	cmd(inter, "bind .l.v.c.var_name <Double-Button-1> { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 7 } }");
	cmd( inter, "bind .l.v.c.var_name <Return> { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 7 } }" );
	cmd( inter, "bind .l.v.c.var_name <Button-2> { .l.v.c.var_name selection clear 0 end;.l.v.c.var_name selection set @%x,%y; set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { .l.v.c.var_name.v entryconfig 3 -state normal; .l.v.c.var_name.v entryconfig 4 -state normal; .l.v.c.var_name.v entryconfig 9 -state normal; .l.v.c.var_name.v entryconfig 10 -state normal; .l.v.c.var_name.v entryconfig 11 -state normal; .l.v.c.var_name.v entryconfig 13 -state normal; .l.v.c.var_name.v entryconfig 14 -state normal; switch $color { purple { } blue { .l.v.c.var_name.v entryconfig 13 -state disabled; .l.v.c.var_name.v entryconfig 14 -state disabled } black { .l.v.c.var_name.v entryconfig 9 -state disabled; .l.v.c.var_name.v entryconfig 10 -state disabled } tomato { } red { .l.v.c.var_name.v entryconfig 13 -state disabled; .l.v.c.var_name.v entryconfig 14 -state disabled } }; if { $itemfocus == 0 } { .l.v.c.var_name.v entryconfig 3 -state disabled }; if { $itemfocus == [ expr [ .l.v.c.var_name size ] - 1 ] } { .l.v.c.var_name.v entryconfig 4 -state disabled }; tk_popup .l.v.c.var_name.v %X %Y } }");
	cmd( inter, "bind .l.v.c.var_name <Button-3> { .l.v.c.var_name selection clear 0 end;.l.v.c.var_name selection set @%x,%y; set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; set color [ lindex [ .l.v.c.var_name itemconf $itemfocus -fg ] end ]; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { .l.v.c.var_name.v entryconfig 3 -state normal; .l.v.c.var_name.v entryconfig 4 -state normal; .l.v.c.var_name.v entryconfig 9 -state normal; .l.v.c.var_name.v entryconfig 10 -state normal; .l.v.c.var_name.v entryconfig 11 -state normal; .l.v.c.var_name.v entryconfig 13 -state normal; .l.v.c.var_name.v entryconfig 14 -state normal; switch $color { purple { } blue { .l.v.c.var_name.v entryconfig 13 -state disabled; .l.v.c.var_name.v entryconfig 14 -state disabled } black { .l.v.c.var_name.v entryconfig 9 -state disabled; .l.v.c.var_name.v entryconfig 10 -state disabled } tomato { } red { .l.v.c.var_name.v entryconfig 13 -state disabled; .l.v.c.var_name.v entryconfig 14 -state disabled } }; if { $itemfocus == 0 } { .l.v.c.var_name.v entryconfig 3 -state disabled }; if { $itemfocus == [ expr [ .l.v.c.var_name size ] - 1 ] } { .l.v.c.var_name.v entryconfig 4 -state disabled }; tk_popup .l.v.c.var_name.v %X %Y } }");
	cmd( inter, "bind .l.v.c.var_name <Control-Up> { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { $itemfocus > 0 } { incr itemfocus -1 }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 58 } }" );
	cmd( inter, "bind .l.v.c.var_name <Control-Down> { set listfocus 1; set itemfocus [ .l.v.c.var_name curselection ]; if { $itemfocus < [ expr [ .l.v.c.var_name size ] - 1 ] } { incr itemfocus }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 59 } }" );
  }
cmd(inter, ".l.v.c.var_name yview $cur");

cmd(inter, "pack .l.v.c.v_scroll -side right -fill y");
cmd(inter, "listbox .l.s.son_name");
if(r->b==NULL)
  cmd( inter, ".l.s.son_name insert end \"(none)\"; set nDesc 0" );
else
{
 for(cb=r->b; cb!=NULL; cb=cb->next )
  {
	 strcpy(ch, ".l.s.son_name insert end ");
	 strcat(ch, cb->blabel);
   cmd(inter, ch);
	}
  cmd( inter, "set nDesc [ .l.s.son_name size ]" );
}	

cmd( inter, "label .l.s.lab -text \"Descendants ($nDesc)\"" );

// objects context menu (right mouse button)
cmd( inter, "menu .l.s.son_name.v -tearoff 0" );
cmd( inter, ".l.s.son_name.v add command -label \"Select\" -command { set choice 4 }" );
cmd( inter, ".l.s.son_name.v add command -label \"Parent\" -command { set choice 5 }" );
cmd( inter, ".l.s.son_name.v add command -label \"Insert Parent\" -command { set choice 32 }" );
cmd( inter, ".l.s.son_name.v add separator" );
cmd( inter, ".l.s.son_name.v add command -label \"Move Up\" -state disabled -command { set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; if { $itemfocus > 0 } { incr itemfocus -1 }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 60 } }" );
cmd( inter, ".l.s.son_name.v add command -label \"Move Down\" -state disabled -command { set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; if { $itemfocus < [ expr [ .l.s.son_name size ] - 1 ] } { incr itemfocus }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 61 } }" );
cmd( inter, ".l.s.son_name.v add separator" );
cmd( inter, ".l.s.son_name.v add command -label Change -command { set choice 6 }" );
cmd( inter, ".l.s.son_name.v add command -label Number -command { set choice 33 }" );
cmd( inter, ".l.s.son_name.v add command -label Delete -command { set choice 74 }" );
cmd( inter, ".l.s.son_name.v add separator" );
cmd( inter, ".l.s.son_name.v add cascade -label Add -menu .l.s.son_name.v.a");
cmd( inter, ".l.s.son_name.v add separator" );
cmd( inter, ".l.s.son_name.v add command -label \"Initial Values\" -command { set choice 21 }" );
cmd( inter, ".l.s.son_name.v add command -label \"Browse Data\" -command { set choice 34 }" );
cmd( inter, "menu .l.s.son_name.v.a -tearoff 0" );
cmd( inter, ".l.s.son_name.v.a add command -label Variable -command { set choice 2; set param 0 }" );
cmd( inter, ".l.s.son_name.v.a add command -label Parameter -command { set choice 2; set param 1 }" );
cmd( inter, ".l.s.son_name.v.a add command -label Function -command { set choice 2; set param 2 }" );
cmd( inter, ".l.s.son_name.v.a add command -label Object -command { set choice 3 }" );

// flag to select among the current or the clicked object
cmd( inter, "set useCurrObj yes" );

if(r->b!=NULL)
{
  cmd( inter, "bind .l.s.son_name <Double-Button-1> { set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 4 } }" );
  cmd( inter, "bind .l.s.son_name <Return> { set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 4 } }" );
  cmd( inter, "bind .l.s.son_name <Button-2> { .l.s.son_name selection clear 0 end; .l.s.son_name selection set @%x,%y; set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set useCurrObj no; if { $itemfocus == 0 } { .l.s.son_name.v entryconfig 4 -state disabled } { .l.s.son_name.v entryconfig 4 -state normal }; if { $itemfocus == [ expr [ .l.s.son_name size ] - 1 ] } { .l.s.son_name.v entryconfig 5 -state disabled } { .l.s.son_name.v entryconfig 5 -state normal }; tk_popup .l.s.son_name.v %X %Y } }" );
  cmd( inter, "bind .l.s.son_name <Button-3> { .l.s.son_name selection clear 0 end; .l.s.son_name selection set @%x,%y; set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set useCurrObj no; if { $itemfocus == 0 } { .l.s.son_name.v entryconfig 4 -state disabled } { .l.s.son_name.v entryconfig 4 -state normal }; if { $itemfocus == [ expr [ .l.s.son_name size ] - 1 ] } { .l.s.son_name.v entryconfig 5 -state disabled } { .l.s.son_name.v entryconfig 5 -state normal }; tk_popup .l.s.son_name.v %X %Y } }" );
  cmd( inter, "bind .l.s.son_name <Control-Up> { set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; if { $itemfocus > 0 } { incr itemfocus -1 }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 60 } }" );
  cmd( inter, "bind .l.s.son_name <Control-Down> { set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; if { $itemfocus < [ expr [ .l.s.son_name size ] - 1 ] } { incr itemfocus }; if { ! [ catch { set vname [ lindex [ split [ selection get ] ] 0 ] } ] } { set choice 61 } }" );
}
cmd( inter, "bind .l.s.son_name <BackSpace> { set choice 5 }" );
cmd(inter, "bind .l.s.son_name <Left> {focus -force .l.v.c.var_name; set listfocus 1; set itemfocus 0; .l.v.c.var_name selection set 0; .l.v.c.var_name activate 0}");

//cmd(inter, "bind .l.s.son_name <Down> {.l.s.son_name selection clear 0 end; .l.s.son_name selection set active}");

cmd(inter, "frame .l.up_name");
strcpy( ch1, "button .l.up_name.d -text \"Parent object: \" -relief flat" );
strcpy( ch, "button .l.up_name.n -relief flat -text \"" );
if( r->up==NULL )
{
  strcat( ch1, " -command { }" );
  strcat( ch, "(none)\" -command { }" );
}
else
 { 
  strcat( ch1, " -command { set itemfocus 0; set choice 5 }" );
  strcat( ch, ( r->up )->label );
  strcat( ch, "\" -command { set itemfocus 0; set choice 5 } -foreground red" );
 }
cmd( inter, ch1 );
cmd( inter, ch );

cmd(inter, "bind . <KeyPress-u> {catch {.l.up_name.n invoke}   }");

cmd(inter, "pack .l.up_name.d .l.up_name.n -side left");
cmd( inter, "pack .l.up_name" );

cmd(inter, "frame .l.tit -relief raised -bd 2");
strcpy( ch1, "button .l.tit.lab -text \"Object: \" -relief flat" );
strcpy( ch, "button .l.tit.but -foreground red -relief flat -text " );
strcat(ch, r->label);
if(r->up!=NULL) 
{
 strcat( ch1, " -command { set choice 6 }" );
 strcat( ch, " -command { set choice 6 }" );
}
else
{
 strcat( ch1, " -command { }" );
 strcat( ch, " -command { }" );
}
cmd( inter, ch1 );
cmd(inter, ch);
 
cmd(inter, "pack .l.tit.lab .l.tit.but -side left");
cmd( inter, "pack .l.tit -pady 2" );

// avoid redrawing the menu if it already exists and is configured
cmd(inter, "set existMenu [ winfo exists .m ]");
cmd(inter, "set confMenu [ . cget -menu ]");
if ( ! strcmp( Tcl_GetVar( inter, "existMenu", 0 ), "0" ) ||
	 strcmp( Tcl_GetVar( inter, "confMenu", 0 ), ".m" ) )
{
cmd(inter, "destroy .m");
cmd(inter, "menu .m -tearoff 0");

cmd(inter, "set w .m.file");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label File -menu $w -underline 0");
cmd(inter, "$w add command -label \"Open...\" -command {set choice 17} -underline 0 -accelerator Ctrl+L");
cmd(inter, "$w add command -label \"Reload\" -command {set choice 38} -underline 0 -accelerator Ctrl+W");
cmd(inter, "$w add command -label Save -command {set choice 18} -underline 0 -accelerator Ctrl+S");
cmd(inter, "$w add command -label \"Save As...\" -command {set choice 73} -underline 5");
cmd(inter, "$w add command -label Empty -command {set choice 20} -underline 0 -accelerator Ctrl+E");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Save Results...\" -command {set choice 37}  -underline 2 -accelerator Ctrl+Z");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Load Sensitivity...\" -command {set choice 64} -underline 3");
cmd(inter, "$w add command -label \"Save Sensitivity...\" -command {set choice 65} -underline 6");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Set Equation File...\" -command {set choice 28} -underline 2 -accelerator Ctrl+U");
cmd(inter, "$w add command -label \"Upload Equation File\" -command {set choice 51} -underline 0");
cmd(inter, "$w add command -label \"Offload Equation File...\" -command {set choice 52} -underline 1");
cmd(inter, "$w add command -label \"Compare Equation Files...\" -command {set choice 53} -underline 0");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label Quit -command {set choice 11} -underline 0 -accelerator Ctrl+Q");

cmd(inter, "set w .m.model");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Model -menu $w -underline 0");
cmd(inter, "$w add command -label \"Add Variable...\" -command {set param 0; set choice 2} -underline 4 -accelerator Ctrl+V");
cmd(inter, "$w add command -label \"Add Parameter...\" -command {set param 1; set choice 2} -underline 4 -accelerator Ctrl+P");
cmd(inter, "$w add command -label \"Add Function...\" -command {set param 2; set choice 2} -underline 5 -accelerator Ctrl+N");
cmd(inter, "$w add command -label \"Add Descending Object...\" -command {set choice 3} -underline 4 -accelerator Ctrl+D");
cmd(inter, "$w add command -label \"Insert New Parent...\" -command {set choice 32} -underline 9");
cmd(inter, "$w add separator");
cmd( inter, "$w add command -label \"Change Element...\" -command { if { ! [ catch { set vname [ .l.v.c.var_name get [ .l.v.c.var_name curselection ] ] } ] && ! [ string equal $vname \"\" ] } { set choice 7 } { tk_messageBox -type ok -icon error -title Error -message \"No element selected.\n\nPlease select an element (variable, parameter) before using this option.\" } } -underline 0" );
cmd(inter, "$w add command -label \"Change Object...\" -command {set choice 6} -underline 7");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Create Auto Descriptions\" -command {set choice 43} -underline 7");
cmd(inter, "$w add command -label \"Create Model Report...\" -command {set choice 36} -underline 7 -accelerator Ctrl+C");
cmd(inter, "$w add command -label \"Create LaTex report\" -command {set choice 57} -underline 9");

cmd(inter, "$w add separator");
cmd(inter, "$w add checkbutton -label \"Ignore Equation File Controls\" -variable ignore_eq_file -command {set choice 54} -underline 0");
cmd(inter, "$w add checkbutton -label \"Enable Structure Window\" -variable strWindowOn -command {set choice 70} -underline 7 -accelerator Ctrl+Tab");
cmd(inter, "$w add command -label \"Find Element...\" -command {set choice 50} -underline 0 -accelerator Ctrl+F");

cmd(inter, "set w .m.data");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Data -menu $w -underline 0");
cmd(inter, "$w add command -label \"Initial Values...\" -command {set choice 21} -underline 0 -accelerator Ctrl+I");
cmd(inter, "$w add cascade -label \"Set Number of Objects\" -underline 4 -menu $w.setobj");
cmd(inter, "$w add separator");
cmd(inter, "$w add cascade -label \"Sensitivity Analysis\" -underline 0 -menu $w.setsens");

cmd(inter, "$w add separator");

cmd(inter, "$w add command -label \"Analysis of Results...\" -command {set choice 26} -underline 0 -accelerator Ctrl+A");
cmd(inter, "$w add command -label \"Data Browse...\" -command {set choice 34} -underline 5 -accelerator Ctrl+B");

cmd(inter, "set w .m.data.setobj");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, "$w add command -label \"All objects...\" -command {set choice 19} -accelerator Ctrl+O -underline 0");
cmd(inter, "$w add command -label \"Only current object...\" -command {set choice 33} -underline 0");

cmd(inter, "set w .m.data.setsens");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, "$w add command -label \"Full (online)\" -command {set choice 62} -underline 0");
cmd(inter, "$w add command -label \"Full (batch)\" -command {set choice 63} -underline 6");
cmd(inter, "$w add command -label \"MC Point Sampling (batch)...\" -command {set choice 71} -underline 0");
cmd(inter, "$w add command -label \"MC Range Sampling (batch)...\" -command {set choice 80} -underline 3");
cmd(inter, "$w add command -label \"EE Sampling (batch)...\" -command {set choice 81} -underline 0");
cmd(inter, "$w add command -label \"NOLH Sampling (batch)...\" -command {set choice 72} -underline 0");

cmd(inter, "set w .m.run");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Run -menu $w -underline 0");
cmd(inter, "$w add command -label Run -command {set choice 1} -underline 0 -accelerator Ctrl+R");
cmd(inter, "$w add command -label \"Start 'No Window' Batch...\" -command {set choice 69} -underline 0");
cmd(inter, "$w add command -label \"Create/Run Parallel Batch...\" -command {set choice 68} -underline 11");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Simulation Settings...\" -command {set choice 22} -underline 2 -accelerator Ctrl+M");
cmd(inter, "$w add checkbutton -label \"Frequent Lattice Updating\" -variable lattype -command {set choice 56} -underline 2");

cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Remove Debug Flags\" -command {set choice 27} -underline 13 -accelerator Ctrl+F");
cmd(inter, "$w add command -label \"Remove Plot Flags\" -command {set choice 31} -underline 7");
cmd(inter, "$w add command -label \"Remove Save Flags\" -command {set choice 30} -underline 1 -accelerator Ctrl+G");
cmd(inter, "$w add command -label \"Remove Sensitivity Data\" -command {set choice 67} -underline 20");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Show Elements to Initialize\" -command {set choice 49} -underline 17");
cmd(inter, "$w add command -label \"Show Elements to Observe\" -command {set choice 42} -underline 17");
cmd(inter, "$w add command -label \"Show Elements to Save\" -command {set choice 39} -underline 1");
cmd(inter, "$w add command -label \"Show Elements of Sens. Anal.\" -command {set choice 66} -underline 17");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Close Runtime Plots\" -command {set choice 40} -underline 0");

cmd(inter, "set w .m.help");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Help -menu $w -underline 0");
cmd(inter, "$w add command -label \"Help on Browser\" -command {LsdHelp Browser.html} -underline 0");
cmd(inter, "$w add command -label \"Lsd Quick Help\" -command {LsdHelp QuickHelp.html} -underline 0");
cmd(inter, "$w add separator");
cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {$w add command -label \"Set Browser\" -command { set choice 48} -underline 0} {}");
cmd(inter, "$w add command -label \"Model Report\" -command {set choice 44} -underline 0");
cmd( inter, "$w add separator" );
sprintf( msg, "$w add command -label \"About Lsd...\" -command { tk_messageBox -type ok -icon info -title \"About Lsd\" -message \"Version %s (%s)\n\nPlatform: [ string totitle $tcl_platform(platform) ] ($tcl_platform(machine))\nOS: $tcl_platform(os) ($tcl_platform(osVersion))\nTcl/Tk: [ info patch ]\" } -underline 0", _LSD_VERSION_, _LSD_DATE_ ); 
cmd( inter, msg );

cmd(inter, "bind . <Control-l> {set choice 17}");
cmd(inter, "bind . <Control-s> {set choice 18}");
cmd(inter, "bind . <Control-e> {set choice 20}");
cmd(inter, "bind . <Control-q> {set choice 11}");
cmd(inter, "bind . <Control-v> {set param 0; set choice 2}");
cmd(inter, "bind . <Control-p> {set param 1; set choice 2}");
cmd(inter, "bind . <Control-n> {set param 2; set choice 2}");

cmd(inter, "bind . <Control-d> {set choice 3}");
cmd(inter, "bind . <Control-o> {set choice 19}");
cmd(inter, "bind . <Control-i> {set choice 21}");
cmd(inter, "bind . <Control-a> {set choice 26}");
cmd(inter, "bind . <Control-r> {set choice 1}");
cmd(inter, "bind . <Control-m> {set choice 22}");
cmd(inter, "bind . <Control-f> {set choice 50}");
cmd(inter, "bind . <Control-u> {set choice 28}");
cmd(inter, "bind . <Control-g> {set choice 30}");
cmd(inter, "bind . <Control-b> {set choice 34}");
cmd(inter, "bind . <Control-c> {set choice 36}");
cmd(inter, "bind . <Control-z> {set choice 37}");
cmd(inter, "bind . <Control-w> {set choice 38}");
cmd(inter, "bind . <Control-Tab> {set strWindowOn [expr ! $strWindowOn]; set choice 70}");


cmd(inter, "bind .log <Control-l> {set choice 17}");
cmd(inter, "bind .log <Control-s> {set choice 18}");
cmd(inter, "bind .log <Control-e> {set choice 20}");
cmd(inter, "bind .log <Control-q> {set choice 11}");
cmd(inter, "bind .log <Control-v> {set param 0; set choice 2}");
cmd(inter, "bind .log <Control-p> {set param 1; set choice 2}");
cmd(inter, "bind .log <Control-n> {set param 2; set choice 2}");
cmd(inter, "bind .log <Control-d> {set choice 3}");
cmd(inter, "bind .log <Control-o> {set choice 19}");
cmd(inter, "bind .log <Control-i> {set choice 21}");
cmd(inter, "bind .log <Control-a> {set choice 26}");
cmd(inter, "bind .log <Control-r> {set choice 1}");
cmd(inter, "bind .log <Control-m> {set choice 22}");
cmd(inter, "bind .log <Control-f> {set choice 50}");
cmd(inter, "bind .log <Control-u> {set choice 28}");
cmd(inter, "bind .log <Control-g> {set choice 30}");
cmd(inter, "bind .log <Control-b> {set choice 34}");
cmd(inter, "bind .log <Control-z> {set choice 37}");
cmd(inter, "bind .log <Control-w> {set choice 38}");
cmd(inter, "bind .log <Control-Tab> {set strWindowOn [expr ! $strWindowOn]; set choice 70}");

if(strWindowOn)
{
cmd(inter, "bind $c <Control-l> {set choice 17}");
cmd(inter, "bind $c <Control-s> {set choice 18}");
cmd(inter, "bind $c <Control-e> {set choice 20}");
cmd(inter, "bind $c <Control-q> {set choice 11}");
cmd(inter, "bind $c <Control-v> {set param 0; set choice 2}");
cmd(inter, "bind $c <Control-p> {set param 1; set choice 2}");
cmd(inter, "bind $c <Control-n> {set param 2; set choice 2}");
cmd(inter, "bind $c <Control-d> {set choice 3}");
cmd(inter, "bind $c <Control-o> {set choice 19}");
cmd(inter, "bind $c <Control-i> {set choice 21}");
cmd(inter, "bind $c <Control-a> {set choice 26}");
cmd(inter, "bind $c <Control-r> {set choice 1}");
cmd(inter, "bind $c <Control-m> {set choice 22}");
cmd(inter, "bind $c <Control-f> {set choice 50}");
cmd(inter, "bind $c <Control-u> {set choice 28}");
cmd(inter, "bind $c <Control-g> {set choice 30}");
cmd(inter, "bind $c <Control-b> {set choice 34}");
cmd(inter, "bind $c <Control-c> {set choice 36}");
cmd(inter, "bind $c <Control-z> {set choice 37}");
cmd(inter, "bind $c <Control-w> {set choice 38}");
cmd(inter, "bind $c <Control-Tab> {set strWindowOn [expr ! $strWindowOn]; set choice 70}");
}

cmd(inter, ". configure -menu .m");
}

cmd(inter, "pack .l.v.c.var_name -fill both -expand yes");
cmd(inter, "pack .l.v.lab -fill x");
cmd(inter, "pack .l.v.c -fill both -expand yes");
cmd(inter, "pack .l.s.lab -fill x");
cmd(inter, "pack .l.s.son_name -fill both -expand yes");

cmd(inter, "pack .l.up_name .l.tit");
cmd(inter, "pack .l.v .l.s -side left -fill both -expand yes");

cmd(inter, "pack .l -fill both -expand yes");

cmd(inter, "update");
cmd(inter, "if { [info exists ModElem]==1 } {set ModElem [lsort -dictionary $ModElem]} {}");
main_cycle:

cmd(inter, "if { $listfocus == 1} {focus -force .l.v.c.var_name; .l.v.c.var_name selection set $itemfocus; .l.v.c.var_name activate $itemfocus; .l.v.c.var_name see $itemfocus} {}");
cmd(inter, "if { $listfocus == 2} {focus -force .l.s.son_name; .l.s.son_name selection set $itemfocus; .l.s.son_name activate $itemfocus} {}");
}

*choice=0;

while(*choice==0 && choice_g==0)
 {
 try{
 Tcl_DoOneEvent(0);
   }
 catch(...) {
 goto main_cycle;
  }
 } 

if(choice_g!=0)
 {*choice=choice_g;
  res_g=(char *)Tcl_GetVar(inter, "res_g",0);
  cmd(inter, "focus -force .l.v.c.var_name");
  choice_g=0;
 }

if(actual_steps>0)
 { // search the sorted list of choices that are bad with existing run data
   if ( bsearch( choice, badChoices, NUM_CHOICES, sizeof ( int ), comp_ints ) != NULL )
   { // prevent changing data if analysis is open
	 cmd( inter, "if [ winfo exists .da ] { tk_messageBox -ok -icon warning -title Warning -message \"Analysis of results window is open.\n\nPlease close it before proceeding with any option that requires existing data to be removed.\"; set daOpen 1 } { set daOpen 0 }" );
	 if ( ! strcmp( Tcl_GetVar( inter, "daOpen", 0 ), "1" ) )
	 {
		 *choice = 0;				// discard option
		 goto main_cycle;
	 }
	 
     cmd( inter, "set T .warn" );
     cmd(inter, "newtop $T Warning");
     cmd(inter, "label $T.l -text \"Simulation just run.\nThe configuration currently loaded is the last step of the previous run.\nThe requested operation makes no sense on the final data of a simulation.\nChoose one of the followig options.\"");
     cmd(inter, "pack $T.l");
     cmd(inter, "set temp 38");
     cmd(inter, "frame $T.f -relief groove -bd 2");
     cmd(inter, "radiobutton $T.f.reload -variable temp -value 38 -text \"Reload the current initial configuration\" -justify left -anchor w");
     cmd(inter, "radiobutton $T.f.load -variable temp -value 17 -text \"Load a new initial configuration\" -justify left -anchor w");     
     cmd(inter, "radiobutton $T.f.ar -variable temp -value 26 -text \"Analyse the results\" -justify left -anchor w");     

   cmd(inter, "pack $T.f.reload $T.f.load $T.f.ar -anchor w -fill x ");
   cmd(inter, "pack $T.f -fill x");
   cmd( inter, "okhelpcancel $T b { set choice 1 } { LsdHelp QuickHelp.html#problem } { set choice 2 }");
  
   *choice=0;
   
   cmd(inter, "focus -force $T.b.ok");
   cmd(inter, "bind $T <Return> {set choice 1}");
   
   cmd(inter, "showtop $T");
   while(*choice==0 && choice_g==0)
     Tcl_DoOneEvent(0);
   cmd(inter, "destroytop $T");

   if(*choice==1)
     cmd(inter, "set choice $temp");
   else 
     { *choice=0;  
       goto main_cycle;
     }  

   }
  } 
 
if(*choice!=35)
{cmd(inter, "if {[winfo exists .]==1} {bind . <Destroy> {}} {}");
 cmd(inter, "if {[winfo exists $c]==1} {bind $c <Destroy> {}} {}");
 cmd(inter,"if {[winfo exists .list]==1} {destroy .list} {}");
}

return *choice;
}

/****************************************************
OPERATE
****************************************************/
object *operate( int *choice, object *r)
{
char *lab1,*lab2,*lab3,lab[300],lab_old[300], ch[300];
int sl, done=0, num, i, j, param, save, plot, nature, numlag, k, lag, fSeq, natBat, temp[5];
bool saveAs, delVar, reload;
char observe, initial, cc;
bridge *cb;

object *n, *cur, *cur1, *cur2;
variable *cur_v, *cv, *app;
FILE *f;
result *rf;					// pointer for results files (may be zipped or not)
double fake=0;
sense *cs;
description *cur_descr;

redrawRoot = false;			// assume no browser redraw

switch(*choice)
{
//Add a Variable to the current or the pointed object (defined in tcl $vname)
case 2:

Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);

// check if current or pointed object and save current if needed
lab1 = ( char * ) Tcl_GetVar( inter, "useCurrObj", 0 );
if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
{
	lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
	if ( lab1 == NULL )
		break;
	sscanf( lab1, "%s", lab_old );
	for ( n = r; n->up != NULL; n = n->up );
	n = n->search( lab_old );		// set pointer to $vname
	if ( n == NULL )
		break;
	cur2 = r;
	r = n;
}
else
	cur2 = NULL;

cmd( inter, "set T .addelem" );
cmd( inter, "newtop $T \"Add Element\" { set done 2 }" );

Tcl_LinkVar(inter, "copy_param", (char *) &param, TCL_LINK_INT);
cmd(inter, "set copy_param $param");

Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);
Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
if(param==0)
{

cmd(inter, "set num 0");
cmd(inter, "set lab \"\"");
sprintf(msg, "label $T.l -text \"Insert a new variable in object: %s\" -fg red", r->label);
cmd(inter, msg);
cmd(inter, "frame $T.f");
cmd(inter, "label $T.f.lab_ent -text \"New variable name: \"");
cmd(inter, "label $T.f.lab_num -text \"Maximum lags used: \"");
cmd(inter, "label $T.f.sp -text \"     \"");
cmd(inter, "entry $T.f.ent_var -width 20 -relief sunken -textvariable lab");
cmd(inter, "entry $T.f.ent_num -width 2 -relief sunken -textvariable num");

cmd(inter, "bind $T.f.ent_num <KeyPress-Return> {focus -force $T.b.ok}");
cmd(inter, "pack $T.f.lab_ent $T.f.ent_var $T.f.sp $T.f.lab_num $T.f.ent_num -side left");
}

if(param==2)
{

cmd(inter, "set num 0");
cmd(inter, "set lab \"\"");
sprintf(msg, "label $T.l -text \"Insert a new function in object: %s\" -fg red", r->label);
cmd(inter, msg);
cmd(inter, "frame $T.f");
cmd(inter, "label $T.f.lab_ent -text \"New Function Name: \"");
cmd(inter, "label $T.f.lab_num -text \"Maximum lags used: \"");
cmd(inter, "label $T.f.sp -text \"     \"");
cmd(inter, "entry $T.f.ent_var -width 20 -relief sunken -textvariable lab");
cmd(inter, "entry $T.f.ent_num -width 2 -relief sunken -textvariable num");

cmd(inter, "bind $T.f.ent_num <KeyPress-Return> {focus -force $T.b.ok}");
cmd(inter, "pack $T.f.lab_ent $T.f.ent_var $T.f.sp $T.f.lab_num $T.f.ent_num -side left");
}

if(param==1)
{ //insert a parameter
cmd(inter, "set lab \"\"");
sprintf(msg, "label $T.l -text \"Insert a new parameter in object: %s\" -fg red", r->label);
cmd(inter, msg);
cmd(inter, "frame $T.f");
cmd(inter, "label $T.f.lab_ent -text \"New Parameter Name: \"");
cmd(inter, "entry $T.f.ent_var -width 20 -relief sunken -textvariable lab");

cmd(inter, "pack $T.f.lab_ent $T.f.ent_var -side left");
}
cmd(inter, "bind $T.f.ent_var <KeyPress-Return> {focus -force $T.b.ok}");

cmd(inter, "set w $T.d");
cmd(inter, "frame $w");
cmd(inter, "frame $w.f -bd 2 -relief groove");
cmd(inter, "label $w.f.lab -text \"Description\"");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "text $w.f.text -wrap word -width 60 -height 4 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.lab $w.f.text -expand yes -fill both");
cmd(inter, "pack $w.f");

cmd(inter, "pack $T.l $T.f $T.d -pady 5");
if(param==0)
	cmd( inter, "okhelpcancel $T b { set done 1 } { LsdHelp menumodel.html#AddAVar } { set done 2 }" );
else
	if ( param == 1 )
		cmd( inter, "okhelpcancel $T b { set done 1 } { LsdHelp menumodel.html#AddAPar } { set done 2 }" );
	else
		cmd( inter, "okhelpcancel $T b { set done 1 } { LsdHelp menumodel.html } { set done 2 }" );
cmd(inter, "focus -force $T.f.ent_var");

cmd( inter, "showtop $T centerS" );
here_newelem:
while(done==0)
 Tcl_DoOneEvent(0);

if(done==1)
 {
lab1=(char *)Tcl_GetVar(inter, "lab",0);
strcpy(lab, lab1);
sl=strlen(lab);
if(sl!=0)
 {
 for(cur=r; cur->up!=NULL; cur=cur->up);
 done=check_label(lab, cur);
 if(done==1)
   {
	cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"The name already exists in the model.\\n\\nChoose a different name and try again.\"" );
   cmd(inter, "focus $T.f.ent_var; $T.f.ent_var selection range 0 end");
   done = 0;
   goto here_newelem;
   }
if ( done == 2 )
{
	cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"Invalid characters in name.\\n\\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
   cmd(inter, "focus $T.f.ent_var; $T.f.ent_var selection range 0 end");
   done = 0;
   goto here_newelem;
}

 if(done==0)
 {
 cmd(inter, "set text_description [$T.d.f.text get 1.0 end]");
 cmd(inter, "if { $text_description==\"\\n\"} {set text_description \"(no description available)\"} {}");
 lab1=(char *)Tcl_GetVar(inter, "text_description",0);
 if(param==1)
  add_description(lab, "Parameter", lab1);
 if(param==0)
  add_description(lab, "Variable", lab1);
 if(param==2)
  add_description(lab, "Function", lab1);

 for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
  { cur->add_empty_var(lab);
	cv=cur->search_var(NULL, lab);
	if(param==1)
	 num=0;
	cv->val=new double[num+1];
	cv->save=0;
	cv->param=param;
	cv->num_lag=num;
	cv->debug='n';
	if((param==0 || param==2) && num==0)
	  cv->data_loaded='+';
	else
	  cv->data_loaded='-';

	for(i=0; i<num+1; i++)
	 cv->val[i]=0;
 
	justAddedVar=true;		// flag variable just added (for acquiring focus)
  }
  unsavedChange = true;		// signal unsaved change
 }
 }

 }

cmd( inter, "destroytop $T" );
redrawRoot = ( done == 2 ) ? false : true;

cmd(inter, "unset lab done");

if(done!=2)
 {
  sprintf(msg, "lappend ModElem %s", lab);
  cmd(inter, msg);
 }  

if ( cur2 != NULL )			// restore original current object
	r = cur2;

Tcl_UnlinkVar(inter, "done");
Tcl_UnlinkVar(inter, "num");
Tcl_UnlinkVar(inter, "copy_param");

break;


//Add a Descendent type to the current or the pointed object (defined in tcl $vname)
//and assigns the number of its instances.
case 3:

// check if current or pointed object and save current if needed
lab1 = ( char * ) Tcl_GetVar( inter, "useCurrObj", 0 );
if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
{
	lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
	if ( lab1 == NULL )
		break;
	sscanf( lab1, "%s", lab_old );
	for ( n = r; n->up != NULL; n = n->up );
	n = n->search( lab_old );		// set pointer to $vname
	if ( n == NULL )
		break;
	cur2 = r;
	r = n;
}
else
	cur2 = NULL;

cmd( inter, "set T .addobj" );
cmd( inter, "newtop $T \"Add Object\" { set done 2 }" );

Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);

cmd(inter, "set lab \"\"");
sprintf(msg, "label $T.tit -text \"Add a new object type descending from: %s\" -fg red",r->label);
cmd(inter, msg);
cmd(inter, "frame $T.f");
cmd(inter, "label $T.f.lab_ent -text \"New object name: \"");
cmd(inter, "entry $T.f.ent_var -width 20 -relief sunken -textvariable lab");
cmd(inter, "pack $T.f.lab_ent $T.f.ent_var -side left");
cmd(inter, "bind $T.f.ent_var <KeyPress-Return> {focus -force $T.b.ok}");

cmd(inter, "set w $T.d");
cmd(inter, "frame $w");
cmd(inter, "frame $w.f -bd 2 -relief groove");
cmd(inter, "label $w.f.lab -text \"Description\"");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "text $w.f.text -wrap word -width 60 -height 4 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
cmd(inter, "bind $w.f.text <KeyPress-Return> {}");
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.lab $w.f.text -expand yes -fill both");
cmd(inter, "pack $w.f");

cmd(inter, "pack $T.tit $T.f $w -pady 5");
cmd( inter, "okhelpcancel $T b { set done 1 } { LsdHelp menumodel.html#AddADesc } { set done 2 }" );
cmd(inter, "focus $T.f.ent_var");

cmd( inter, "showtop $T centerS" );
here_newobject:
while(done==0)
 Tcl_DoOneEvent(0);

if(done==1)
{
 lab1=(char *)Tcl_GetVar(inter, "lab",0);
 strcpy(lab, lab1);
 if(strlen(lab)==0)
	goto here_endobject;
 for(cur=r; cur->up!=NULL; cur=cur->up);
 done=check_label(lab, cur); //check that the label does not exist already
 if(done==1)
   {
	cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"The name already exists in the model.\\n\\nChoose a different name and try again.\"" );
   cmd(inter, "focus $T.f.ent_var; $T.f.ent_var selection range 0 end");
   done = 0;
   goto here_newobject;
   }
 if(done==2)
  {
   cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"Invalid characters in name.\\n\\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
   cmd(inter, "focus $T.f.ent_var; $T.f.ent_var selection range 0 end");
   done = 0;
   goto here_newobject;
  }

 r->add_obj(lab, 1, 1);
 cmd(inter, "set text_description [$T.d.f.text get 1.0 end]");  
 cmd(inter, "if { $text_description==\"\\n\" || $text_description==\"\"} {set text_description \"(no description available)\"} {}");
 lab1=(char *)Tcl_GetVar(inter, "text_description",0);
 add_description(lab, "Object", lab1);

 unsavedChange = true;		// signal unsaved change
 redrawRoot = true;			// force browser redraw
 }

here_endobject:

if ( cur2 != NULL )			// restore original current object
	r = cur2;

cmd( inter, "destroytop $T" );
Tcl_UnlinkVar(inter, "done");
cmd(inter, "unset lab done");

break;


//Insert a parent Object just above the current or pointed object (defined in tcl $vname)
case 32:

Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);

// check if current or pointed object and save current if needed
lab1 = ( char * ) Tcl_GetVar( inter, "useCurrObj", 0 );
if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
{
	lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
	if ( lab1 == NULL )
		break;
	sscanf( lab1, "%s", lab_old );
	for ( n = r; n->up != NULL; n = n->up );
	n = n->search( lab_old );		// set pointer to $vname
	if ( n == NULL )
		break;
	cur2 = r;
	r = n;
}
else
	cur2 = NULL;

cmd( inter, "set T .inspar" );
cmd( inter, "newtop $T \"Insert Parent\" { set done 2 }" );

if(r->up==NULL)
 {
  cmd( inter, "set answer [ tk_messageBox -type okcancel -default cancel -title Warning -icon warning -message \"Cannot insert a parent of Root.\\n\\nPress 'Ok' if you want the new object to be a descendant of Root and contain all current descendants from Root.\" ]; if [ string equal -nocase $answer ok ] { set done 1 } { set done 2 }" );
  if ( done == 2 )
	goto here_endparent;
  done=0;
 }

cmd(inter, "frame $T.f");
if(r->up!=NULL)
  sprintf(msg, "label $T.f.lab_ent -text \"Insert the name of the new object\\n(parent of %s and descending from %s)\"", r->label, r->up->label);
else
  sprintf(msg, "label $T.f.lab_ent -text \"Insert the name of the new object\"");

cmd(inter, msg);
cmd(inter, "set lab \"\"");
cmd(inter, "entry $T.f.ent_var -width 20 -relief sunken -textvariable lab");
cmd(inter, "pack $T.f.lab_ent $T.f.ent_var");
cmd(inter, "bind $T.f.ent_var <KeyPress-Return> {focus -force $T.b.ok}");

cmd(inter, "set w $T.d");
cmd(inter, "frame $w");
cmd(inter, "frame $w.f -bd 2 -relief groove");
cmd(inter, "label $w.f.lab -text \"Description\"");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "text $w.f.text -wrap word -width 60 -height 4 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
cmd(inter, "bind $w.f.text <KeyPress-Return> {}");
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.lab $w.f.text -expand yes -fill both");
cmd(inter, "pack $w.f");

cmd(inter, "pack $T.f $w -pady 5");
cmd( inter, "okhelpcancel $T b { set done 1 } { LsdHelp menumodel.html#InsertAParent } { set done 2 }" );
cmd(inter, "focus $T.f.ent_var");

cmd( inter, "showtop $T centerS" );
here_newparent:
while(done==0)
 Tcl_DoOneEvent(0);

if(done==1)
{
 lab1=(char *)Tcl_GetVar(inter, "lab",0);
 sscanf( lab1, "%s", lab );
 if(strlen(lab)==0)
	goto here_endparent;
 for(cur=r; cur->up!=NULL; cur=cur->up);
 done=check_label(lab1, cur); //check that the label does not exist already
 if(done==1)
   {
	cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"The name already exists in the model.\\n\\nChoose a different name and try again.\"" );
   cmd(inter, "focus $T.f.ent_var; $T.f.ent_var selection range 0 end");
   done = 0;
   goto here_newparent;
   }
 if(done==2)
  {
   cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"Invalid characters in name.\\n\\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
   cmd(inter, "focus $T.f.ent_var; $T.f.ent_var selection range 0 end");
   done = 0;
   goto here_newparent;
  }

 if(r->up==NULL)
  {
  cur=new object;
  cur->init(NULL, lab);
  cur->next=NULL;

  cur->up=r;
  cur->to_compute=1;
  cur->b=r->b;
  r->b=new bridge;
  r->b->next=NULL;
  r->b->blabel=new char[strlen(lab)+1];
  strcpy(r->b->blabel, lab);
  
  r->b->head=cur;
  cur->v=r->v;
  r->v=NULL;
  for(cur1=cur->b->head; cur1!=NULL; cur1=cur1->next)
    cur1->up=cur;
  }
 else
  {r->insert_parent_obj_one(lab);
   r=r->up;
  }

 }

 cmd(inter, "set text_description [$T.d.f.text get 1.0 end]");  
 cmd(inter, "if { $text_description==\"\\n\" || $text_description==\"\"} {set text_description \"(no description available)\"} {}");
 lab1=(char *)Tcl_GetVar(inter, "text_description",0);
 add_description(lab, "Object", lab1);

 unsavedChange = true;		// signal unsaved change
 redrawRoot = true;			// force browser redraw

here_endparent:

if ( cur2 != NULL )			// restore original current object
	r = cur2;

cmd( inter, "destroytop $T" );
Tcl_UnlinkVar(inter, "done");
cmd(inter, "unset lab done");

break;


//Move browser to show one of the descendant object (defined in tcl $vname)
case 4:

*choice=0;
lab1=(char *)Tcl_GetVar(inter, "vname",0);
if ( lab1 == NULL || ! strcmp( lab1, "(none)" ) )
	break;
sscanf( lab1, "%s", lab_old );

n=r->search(lab_old);
if(n==NULL)
 {sprintf(ch, "\nDescendant %s not found",lab_old);
  plog(ch);
  break;
 }

cmd( inter, "set cur 0; set listfocus 2; set itemfocus 0" );

redrawRoot = true;			// force browser redraw
return (n);


//Move browser to show the parent object
case 5:

*choice=0;
if(r->up==NULL)
 return r;
for(i=0, cb=r->up->b; cb->head!=r; cb=cb->next, i++);
sprintf(msg, "set cur 0; set listfocus 2; set itemfocus %d", i);
cmd(inter, msg); 

redrawRoot = true;					// force browser redraw
return r->up;


//Edit current Object's name and give the option to disable the computation (defined in tcl $vname)
case 6:

sprintf( msg, "if $useCurrObj { set lab %s } { if [ info exists vname ] { set lab $vname } { set lab \"\" } }; set useCurrObj yes ", r->label );
cmd(inter, msg);
lab1=(char *)Tcl_GetVar(inter, "lab",0);

if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;

sscanf(lab1, "%s", lab_old);

// check if current or pointed object and save current if needed
if ( strcmp( r->label, lab_old ) )	// check if not current variable
{
	for ( n = r; n->up != NULL; n = n->up );
	n = n->search( lab_old );		// set pointer to $vname
	if ( n == NULL )
		break;
	cur2 = r;
	r = n;
}
else
	cur2 = NULL;

if ( ! strcmp( r->label, "Root" ) )	// cannot change Root
{
	cmd( inter, "tk_messageBox -type ok -title Warning -icon warning -message \"Cannot change Root.\\n\\nPlease select an existing object or insert a new one before using this option.\"" );
	break;
}

cmd( inter, "set T .objprop" );
cmd( inter, "newtop $T \"Object Properties\" { set choice 2 }" );

cmd(inter, "frame $T.b1");
sprintf(msg, "set to_compute %d",r->to_compute);
cmd(inter, msg);
cmd(inter, "checkbutton $T.b1.com -text \"Compute: require the simulation to compute the variables in this object.\" -variable to_compute -anchor w");
cmd(inter, "pack $T.b1.com -anchor w");

cur_descr=search_description(lab_old);
if(cur_descr==NULL)
  {
   add_description(lab_old, "Object", "(no description available)");
   sprintf(msg, "\nWarning! description for '%s' not found. New one created.", lab_old);
   plog(msg);
   cur_descr=search_description(lab_old);
  } 
  
cmd(inter, "set w $T.desc");
cmd(inter, "frame $w");
cmd(inter, "frame $w.f -bd 2 -relief groove");
cmd(inter, "label $w.f.int -text \"Description\"");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "text $w.f.text -wrap word -width 60 -height 10 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.int $w.f.text -anchor w -expand yes -fill both");
cmd(inter, "pack $w.f");
  for(i=0; cur_descr->text[i]!=(char)NULL; i++)
   {
   if(cur_descr->text[i]!='[' && cur_descr->text[i]!=']' && cur_descr->text[i]!='{' && cur_descr->text[i]!='}' && cur_descr->text[i]!='\"' )
     {
      sprintf(msg, "$w.f.text insert end \"%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }
    else
     {
      sprintf(msg, "$w.f.text insert end \"\\%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }

   }
 cmd(inter, "$w.f.text delete \"end - 1 char\"");
cmd(inter, "pack $w.f -fill x -expand yes");


cmd(inter, "frame $T.h");
cmd(inter, "label $T.h.ent_var -width 30 -relief sunken -fg red -text $lab");
cmd(inter, "bind $T.h.ent_var <KeyPress-Return> {focus -force $T.b.ok}");

cmd(inter, "label $T.h.lab_ent -text \"Object\"");

cmd(inter, "frame $T.h.b");
cmd(inter, "button $T.h.b.prop -width -9 -text \"Change Name\" -command {set choice 5}" );
cmd(inter, "button $T.h.b.del -width -9 -text Delete -command {set choice 3}");
cmd(inter, "pack $T.h.b.prop $T.h.b.del -padx 10 -pady 5 -side left");

cmd(inter, "bind $T.h <Double-1> {set choice 5}");
cmd(inter, "bind $T.h <KeyPress-c> \"$T.b1.com invoke\"");
cmd(inter, "bind $T.h <KeyPress-Delete> {set choice 3}");
cmd(inter, "bind $T <Control-d> {}");
cmd(inter, "bind $T <Control-z> {}");
cmd(inter, "bind $T.h.ent_var <Control-d> \"focus -force $T.desc.f.text\"");
cmd(inter, "bind $T.desc.f.text <Control-z> {set choice 1}");
cmd(inter, "pack $T.h.lab_ent $T.h.ent_var $T.h.b");

cmd(inter, "pack $T.h $T.b1 $w -pady 5 -fill x -expand yes");
cmd( inter, "okhelpcancel $T b { set choice 1 } { LsdHelp menumodel.html#ChangeObjName } { set choice 2 }" );
cmd(inter, "focus -force $T.h.ent_var");

*choice=0;
cmd( inter, "showtop $T centerS" );
while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==1|| *choice==5 || *choice==3)
{

unsavedChange = true;		// signal unsaved change
change_descr_text(lab_old);

if(*choice==5 || *choice==3)
{
if(*choice==3)
{
	 cmd(inter, "set answer [tk_messageBox -title \"Delete Object\" -icon warning -type okcancel -default cancel -message \"Press 'Ok' to confirm deleting:\n$lab\n\nNote that all descendents will be also deleted!\"]");
	 cmd(inter, "switch -- $answer {ok {set choice 1} cancel {set choice 2}}");
	 if(*choice == 1)				// simulate a name change
		cmd(inter, "set lab \"\"");	// to empty string (delete)
}
else
{
cmd( inter, "set TT .chgnam" );
cmd( inter, "newtop $TT \"Change Name\" { set choice 2 }" );

cmd(inter, "label $TT.l -text \"New name for object: $lab\"");
cmd(inter, "entry $TT.e -width 30 -textvariable lab");
cmd(inter, "pack $TT.l $TT.e -anchor w");

cmd( inter, "okcancel $TT b { set choice 1 } { set choice 2 }" );
cmd(inter, "focus -force $TT.e");

cmd( inter, "showtop $TT centerS" );
here_newname:
*choice=0;
while(*choice==0)
 Tcl_DoOneEvent(0);

if ( *choice == 1 )
{
	lab1= ( char * ) Tcl_GetVar( inter, "lab", 0 );
	sscanf( lab1, "%s", lab );
	if ( strlen( lab ) == 0 )
		goto here_newname;
	if( strcmp( lab, r->label ) )
	{
		for( cur = r; cur->up != NULL; cur = cur->up );
		done = check_label( lab, cur );
		if(done==1)
		{
			cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"The name already exists in the model.\\n\\nChoose a different name and try again.\"" );
			cmd(inter, "focus $TT.e; $TT.e selection range 0 end");
			goto here_newname;
		}
		if(done==2)
		{
			cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"Invalid characters in name.\\n\\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
			cmd(inter, "focus $TT.e; $TT.e selection range 0 end");
			goto here_newname;
		}

	}
	else
		*choice = 2;
}
cmd( inter, "destroytop $TT" );
}

if(*choice==2)
	goto here_endobjprop;

if(strlen(lab1)!=0)
{

 if(strcmp(lab, r->label))
  {
   change_descr_lab(r->label, lab, "", "", "");
   r->chg_lab(lab);
   goto here_endobjprop;
  }
}
else //Delete the Object !
 {for(cur=r->up; cur->up!=NULL; cur=cur->up);
  cur=cur->search(r->label);
  r=r->up;
  for(cv=cur->v; cv!=NULL; cv=cv->next)
    change_descr_lab(cv->label,"" , "", "", "");
  wipe_out(cur);
  goto here_endobjprop;

 }
}//end of *choice==5
cmd(inter, "set choice $to_compute");

if(*choice!=r->to_compute)
{cur=blueprint->search(r->label);
 cur->to_compute=*choice;
 for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
   cur->to_compute=*choice;
}   
//control for elements to save in objects to be not computed
if(*choice==0)
{
control_tocompute(r, r->label);
}
cmd(inter, "set text_description \"[$T.desc.f.text get 1.0 end]\"");
cmd(inter, "if { $text_description==\"\\n\" || $text_description==\"\"} {set text_description \"(no description available)\"} {}");
lab1=(char *)Tcl_GetVar(inter, "text_description",0);
add_description(lab, "Object", lab1);

here_endobjprop:

redrawRoot = true;			// force browser redraw
}//end of *choice==1 || *choice==5

if ( cur2 != NULL )			// restore original current object
	r = cur2;

cmd( inter, "destroytop $T" );

break;


//Delete object (defined in tcl $vname)
case 74:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab_old );

for ( cur = r; cur->up != NULL; cur = cur->up );
cur = cur->search( lab_old );		// get pointer to vname
if ( cur == NULL )
{
	sprintf( msg, "\nObject %s not found", lab_old );
	plog( msg );
	break;
}

cmd(inter, "set answer [tk_messageBox -title \"Delete Object\" -icon warning -type okcancel -default cancel -message \"Press 'Ok' to confirm deleting:\n$vname\n\nNote that all descendents will be also deleted!\"]");
cmd(inter, "switch $answer {ok {set choice 1} cancel {set choice 2}}");
if( *choice == 2 )
	break;

r = cur->up;
for ( cv = cur->v; cv != NULL; cv = cv->next )
	change_descr_lab( cv->label, "" , "", "", "" );
wipe_out( cur );

unsavedChange = true;				// signal unsaved change
redrawRoot = true;					// force browser redraw

break;


//Edit variable name (defined in tcl $vname) and set debug/saving/plot flags
case 7:

redrawRoot = true;					// assume browser redraw required

int savei;

lab1=(char *)Tcl_GetVar(inter, "vname",0);
if ( lab1 == NULL || ! strcmp( lab1, "(none)" ) )
	break;
sscanf(lab1, "%s", lab_old);

cv=r->search_var(NULL, lab_old);
Tcl_LinkVar(inter, "debug", (char *) &num, TCL_LINK_INT);
Tcl_LinkVar(inter, "save", (char *) &save, TCL_LINK_INT);
Tcl_LinkVar(inter, "savei", (char *) &savei, TCL_LINK_INT);

Tcl_LinkVar(inter, "plot", (char *) &plot, TCL_LINK_INT);

save=cv->save;
num=cv->debug=='d'?1:0;
plot=cv->plot;
savei=cv->savei;

cmd( inter, "set T .chgelem" );
cmd( inter, "newtop $T \"Change Element\" { set done 2 }" );

cmd(inter, "frame $T.b1");
cmd(inter, "checkbutton $T.b1.sav -text \"Save: save the series for later analysis\" -variable save -anchor w");
cmd(inter, "checkbutton $T.b1.savi -text \"Save file: save the series in a separate file\" -variable savei -anchor w");
cmd(inter, "checkbutton $T.b1.plt -text \"Run time plot: observe the series during the simulation execution\" -variable plot -anchor w");
cmd(inter, "checkbutton $T.b1.deb -text \"Debug: allow interruption after this equation/function\" -variable debug -anchor w");

if(cv->param==1)
 cmd(inter, "pack $T.b1.sav $T.b1.savi $T.b1.plt -anchor w");
if(cv->param==0||cv->param==2)
 cmd(inter, "pack $T.b1.sav $T.b1.savi $T.b1.plt $T.b1.deb -anchor w");

cmd(inter, "bind $T.b1 <KeyPress-d> \"$T.b1.deb invoke\"");
cmd(inter, "bind $T.b1 <KeyPress-s> \"$T.b1.sav invoke\"");
cmd(inter, "bind $T.b1 <KeyPress-f> \"$T.b1.savi invoke\"");
cmd(inter, "bind $T.b1 <KeyPress-p> \"$T.b1.plt invoke\"");
cmd(inter, "bind $T.b1 <KeyPress-i> \"$T.desc.opt.ini invoke\"");
cmd(inter, "bind $T.b1 <KeyPress-o> \"$T.desc.opt.obs invoke\"");

sprintf(ch, "set vname %s", lab_old);
cmd(inter, ch);

 cur_descr=search_description(lab_old);
 if(cur_descr==NULL)
  {if(cv->param==0)
     add_description(lab_old, "Variable", "(no description available)");
   if(cv->param==1)
     add_description(lab_old, "Parameter", "(no description available)");  
   if(cv->param==2)
     add_description(lab_old, "Function", "(no description available)");  
   sprintf(msg, "\nWarning! description for '%s' not found. New one created.", lab_old);
   plog(msg);
   cur_descr=search_description(lab_old);
  } 

 cmd(inter, "set w $T.desc");
 cmd(inter, "frame $w");
 cmd(inter, "frame $w.f -bd 2 -relief groove");
 cmd(inter, "label $w.f.int -text \"Description\"");

  cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
 cmd(inter, "text $w.f.text -wrap word -width 60 -height 10 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
 cmd(inter, "pack $w.f.yscroll -side right -fill y");
 cmd(inter, "pack $w.f.int $w.f.text -anchor w -expand yes -fill both");
 cmd(inter, "pack $w.f");
  for(i=0; cur_descr->text[i]!=(char)NULL; i++)
   {
   if(cur_descr->text[i]!='[' && cur_descr->text[i]!=']' && cur_descr->text[i]!='{' && cur_descr->text[i]!='}' && cur_descr->text[i]!='\"' )
     {
      sprintf(msg, "$w.f.text insert end \"%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }
    else
     {
      sprintf(msg, "$w.f.text insert end \"\\%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }

   }
 cmd(inter, "frame $w.opt");
 sprintf(msg, "set observe %d", cur_descr->observe=='y'?1:0);
 cmd(inter, msg);
 sprintf(msg, "set initial %d", cur_descr->initial=='y'?1:0);
 cmd(inter, msg);
 
 cmd(inter, "label $w.opt.l -text \"In model documentation set the element to be: \"");
 cmd(inter, "pack $w.opt.l -side left");
 cmd(inter, "checkbutton $w.opt.obs -text \"Observed\" -variable observe -anchor w");
 cmd(inter, "checkbutton $w.opt.ini -text \"Initialized\" -variable initial -anchor w"); 
 if(cv->param==1 || cv->num_lag>0)
  cmd(inter, "pack $w.opt.obs $w.opt.ini -side left -anchor w");
 else
  cmd(inter, "pack $w.opt.obs -anchor w");
 cmd(inter, "$w.f.text delete \"end - 1 char\"");
 cmd(inter, "pack $w.opt -expand yes -fill x");
 cmd(inter, "frame $w.b");
 cmd(inter, "button $w.b.eq -width -9 -text \"View Code\" -command {set done 3}");
 cmd(inter, "button $w.b.auto_doc -width -9 -text \"Auto Doc.\" -command {set done 9}");
 cmd(inter, "button $w.b.us -width -9 -text \"Using Element\" -command {set done 4}");
 cmd(inter, "button $w.b.using -width -9 -text \"Elements Used\" -command {set done 7}");
 if(!strcmp(cur_descr->type, "Parameter"))
   cmd(inter, "pack $w.b.auto_doc $w.b.us -padx 10 -pady 5 -side left");
 else
   cmd(inter, "pack $w.b.eq $w.b.auto_doc $w.b.us $w.b.using -padx 10 -pady 5  -side left -fill both");
 cmd(inter, "pack $w.f $w.b");
 if(cv->param==1 || cv->num_lag>0)
  {
   cmd(inter, "frame $w.i -bd 2 -relief groove");
   cmd(inter, "label $w.i.int -text \"Comments on the initial values\"");
   cmd(inter, "scrollbar $w.i.yscroll -command \"$w.i.text yview\"");
   cmd(inter, "text $w.i.text -wrap word -width 60 -height 4 -relief sunken -yscrollcommand \"$w.i.yscroll set\"");
   cmd(inter, "pack $w.i.yscroll -side right -fill y");
   if(cur_descr->init!=NULL)
    {
     for(i=0; cur_descr->init[i]!=(char)NULL; i++)
      {
      if(cur_descr->init[i]!='[' && cur_descr->init[i]!=']' && cur_descr->init[i]!='{' && cur_descr->init[i]!='}' && cur_descr->init[i]!='\"' )
        {
         sprintf(msg, "$w.i.text insert end \"%c\"", cur_descr->init[i]);
         cmd(inter, msg);
        }
       else
        {
         sprintf(msg, "$w.i.text insert end \"\\%c\"", cur_descr->init[i]);
         cmd(inter, msg);
        }
   
      }
     cmd(inter, "$w.i.text delete \"end - 1 char\"");
    }
   cmd(inter, "pack $w.i.int $w.i.text -anchor w -expand yes -fill both");
   cmd(inter, "pack $w.i -pady 5 -anchor w -expand yes -fill both");
   cmd(inter, "frame $w.b2");
   cmd(inter, "button $w.b2.setall -width -9 -text \"Initial Values\" -command {set done 11}" );
   cmd(inter, "button $w.b2.sens -width -9 -text \"Sensitivity Analysis\" -command {set done 12}" );
   cmd(inter, "pack $w.b2.setall $w.b2.sens -padx 10 -pady 5 -side left");
   cmd(inter, "pack $w.b2");
   cmd(inter, "bind $w.f.text <Control-i> {focus -force $w.i.text}");
   cmd(inter, "bind $w.i.text <Control-z> {set done 1}");   
   }

Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);

cmd(inter, "frame $T.h");
cmd(inter, "label $T.h.ent_var -width 30 -relief sunken -fg red -text $vname");

cmd(inter, "bind $T.b1 <KeyPress-Return> {set done 1}");

if(cv->param==0)
  cmd(inter, "label $T.h.lab_ent -text \"Variable\"");
if(cv->param==1)
  cmd(inter, "label $T.h.lab_ent -text \"Parameter\"");
if(cv->param==2)
  cmd(inter, "label $T.h.lab_ent -text \"Function\"");

sprintf(msg, "set obj_name %s", cv->up->label);
cmd(inter, msg);
cmd(inter, "label $T.h.obj -text \"in object $obj_name\"");

cmd(inter, "frame $T.h.b");
cmd(inter, "button $T.h.b.prop -width -9 -text \"Properties\" -command {set done 5}" );
cmd(inter, "button $T.h.b.mov -width -9 -text \"Move\" -command {set done 13}");
cmd(inter, "button $T.h.b.del -width -9 -text \"Delete\" -command {set done 10}");
cmd(inter, "bind $T.h <Double-1> {set done 5}");
cmd(inter, "pack $T.h.b.prop $T.h.b.mov $T.h.b.del -padx 10 -pady 5 -side left");
cmd(inter, "pack $T.h.lab_ent $T.h.ent_var $T.h.obj $T.h.b");
cmd(inter, "pack $T.h $T.b1 $w -pady 5 -fill x -expand yes");
cmd(inter, "bind $T.h <KeyPress-Delete> {set done 10}");
cmd(inter, "bind $T.b1 <Control-d> {focus -force $w.f.text}");
cmd(inter, "bind $T.desc.f.text <Control-z> {set done 1}");   

cmd( inter, "donehelp $T b { set done 1 } { LsdHelp menumodel.html#variables }" );

cmd(inter, "showtop $T topleftW");
cmd(inter, "focus $T.b1");
cycle_var:
while(done==0)
 Tcl_DoOneEvent(0);

if(done==1)
 {
  cmd(inter, "set choice $observe");
  *choice==1?observe='y':observe='n';
  cmd(inter, "set choice $initial");
  *choice==1?initial='y':initial='n';
  cur_descr->initial=initial;
  cur_descr->observe=observe;
 }

*choice = 1;	// point .top window as parent for the following windows
if(done == 3)
 show_eq(lab_old, choice);
if(done == 4)
 scan_used_lab(lab_old, choice);
if(done == 7)
 scan_using_lab(lab_old, choice);
*choice = 0;

if(done == 9) 
 {
  auto_document( choice, lab_old, "ALL", true );
  cmd(inter, "$w.f.text delete 1.0 end");

  for(i=0; cur_descr->text[i]!=(char)NULL; i++)
   {
   if(cur_descr->text[i]!='[' && cur_descr->text[i]!=']' && cur_descr->text[i]!='{' && cur_descr->text[i]!='}' && cur_descr->text[i]!='\"' )
     {
      sprintf(msg, "$w.f.text insert end \"%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }
    else
     {
      sprintf(msg, "$w.f.text insert end \"\\%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }
      
   } 
  unsavedChange = true;		// signal unsaved change
 }
if(done == 7 || done == 4 || done == 3 || done == 9)
 {
  done=0;
  goto cycle_var;
 }

if(done==1) 
  {
   cmd(inter, "set text_description \"[$w.f.text get 1.0 end]\"");
   change_descr_text(lab_old);
   if(cv->param==1 || cv->num_lag>0)
    {cmd(inter, "set text_description \"[$w.i.text get 1.0 end]\"");
     change_init_text(lab_old);
    }
  
  unsavedChange = true;			// signal unsaved change
  }

cmd( inter, "destroytop $T" );

if ( done == 2 || done == 8 )	// esc/cancel
{
	redrawRoot = false;			// no browser redraw
	goto here_endelem;
}

if(done==1)
 {if(save==1 || savei==1)
   {
   for(cur=r; cur!=NULL; cur=cur->up)
    if(cur->to_compute==0)
     {
       sprintf(msg, "tk_messageBox -type ok -title Warning -icon warning -message \"Item\n'%s'\nset to be saved, but will not be available for the Analysis of Results, since object\n'%s'\nis set to be not computed.\"", lab_old, cur->label);
   cmd(inter, msg);
     }
   }
  for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
   {
  	cv=cur->search_var(NULL, lab_old);
  	cv->save=save;
    cv->savei=savei;
  	cv->debug=num==1?'d':'n';
  	cv->plot=plot;
   }
    
 }
if(done!=8)
  *choice=0;
else
  *choice=7;  

here_endelem:

Tcl_UnlinkVar(inter, "done");
Tcl_UnlinkVar(inter, "save");
Tcl_UnlinkVar(inter, "savei");
Tcl_UnlinkVar(inter, "debug");
Tcl_UnlinkVar(inter, "plot");

// options to be handled in a second run of the operate function
switch ( done )
{
	case 5:
		*choice = 75;			// open properties box for $vname
		break;
	case 10:
		*choice = 76;			// delete element in $vname
		break;
	case 11:
		*choice = 77;			// change initial values for $vname
		break;
	case 12:
		*choice = 78;			// change sensitivity values for $vname
		break;
	case 13:
		*choice = 79;			// move element in $vname
		break;
	default:
		*choice = 0;
		break;
}
if ( *choice != 0 )
{
	redrawRoot = false;			// no browser redraw yet
	return r;					// execute command
} 

break;


// Edit variable/parameter (defined by tcl $vname) properties
case 75:
// Delete variable/parameter (defined by tcl $vname)
case 76:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab_old );		// get var/par name in lab_old

if ( *choice == 76 )
{
	delVar = true;
	cmd( inter, "set answer [ tk_messageBox -title \"Delete Element\" -icon warning -type okcancel -default ok -message \"Press 'Ok' to confirm deleting:\n$vname\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } }" );
	if( *choice == 1 )
		cmd( inter, "set vname \"\"; set nature 3; set numlag 0" );	// configure to delete
	else
		goto here_endprop;
}
else
{
	delVar = false;

	cv=r->search_var(NULL, lab_old);
	cmd( inter, "set nature 3" );
	sprintf(msg, "set numlag %d", cv->num_lag);
	cmd(inter, msg);

	cmd( inter, "set T .prop" );
	cmd( inter, "newtop $T \"Properties\" { set choice 2 }" );

	cmd(inter, "frame $T.l");
	cmd(inter, "radiobutton $T.l.var -text \"Change Name\" -variable nature -value 3");
	cmd(inter, "entry $T.l.e -width 30 -textvariable vname");
	cmd(inter, "bind $T.l.e <1> \"$T.l.var invoke\"");
	cmd(inter, "pack $T.l.var $T.l.e -side left");

	cmd(inter, "frame $T.v");
	cmd(inter, "radiobutton $T.v.var -text Variable -variable nature -value 0");
	cmd(inter, "label $T.v.l -text Lags");
	cmd(inter, "entry $T.v.e -width 3 -textvariable numlag");
	cmd(inter, "pack $T.v.var $T.v.l $T.v.e -side left");
	cmd(inter, "frame $T.p");
	cmd(inter, "radiobutton $T.p.par -text Parameter -variable nature -value 1");
	cmd(inter, "pack $T.p.par");
	cmd(inter, "frame $T.f");
	cmd(inter, "radiobutton $T.f.fun -text Function -variable nature -value 2");
	cmd(inter, "label $T.f.l -text Lags");
	cmd(inter, "entry $T.f.e -width 3 -textvariable numlag");
	cmd(inter, "pack $T.f.fun $T.f.l $T.f.e -side left");
	cmd(inter, "pack $T.f.fun");

	cmd(inter, "pack $T.l $T.v $T.p $T.f -anchor w");
	cmd( inter, "okhelpcancel $T b { set choice 1 } { LsdHelp menumodel.html#change_nature } { set choice 2 }" );
	cmd(inter, "bind $T <KeyPress-Return> {set choice 1}");
	cmd(inter, "bind $T.l.e <KeyPress-Return> {set choice 1}");
	*choice=0;
	cmd(inter, "showtop $T");
}

while(*choice==0)
	Tcl_DoOneEvent(0);

cmd( inter, "if [ winfo exists .prop ] { destroytop $T }" );

if(*choice==2)
	goto here_endprop;

cmd(inter, "set choice $nature");
nature=*choice;

if(nature==4)
{
	sprintf(msg, "set choice [string equal $movelabel %s]", r->label);
	cmd(inter, msg);
	if(*choice==1)
		goto here_endprop;

	lab1=(char *)Tcl_GetVar(inter, "movelabel",0); 
	cv=r->search_var(NULL, lab_old);
	if(cv->param==1 || cv->num_lag>0) 
		cv->data_loaded='-';
	for(cur=root->search(lab1); cur!=NULL; cur=cur->hyper_next(cur->label) )
		cur->add_var_from_example(cv);
	cmd(inter, "set vname \"\"");
	delVar = true;
}

cmd(inter, "set choice $numlag");
numlag=*choice;

if(nature==3 || nature==4)
{
	lab1=(char *)Tcl_GetVar(inter, "vname",0);
	if ( strlen( lab1 ) > 0 )
		sscanf( lab1, "%s", lab );
	else
		if ( delVar )
			strcpy( lab, "" );
		else
			goto here_endprop;
	
	if ( strcmp( lab, lab_old ) && ! delVar )
	{
		for(cur=r; cur->up!=NULL; cur=cur->up);
		*choice=check_label(lab, cur);

		if(*choice==1)
		{
			cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"The name already exists in the model.\\n\\nChoose a different name and try again.\"" );
			goto here_endprop;
		}
		if(*choice==2)
		{
			cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"Invalid characters in name.\\n\\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces. Choose a different label and try again.\"" );
			goto here_endprop;
		}
	}
	
	if(nature==3)
		change_descr_lab(lab_old, lab, "", "", "");
	
	for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
	{
		if(strlen(lab)!=0)
		{
		cur->chg_var_lab(lab_old, lab);
		cv=cur->search_var(NULL, lab);
		}
		else
		{
			if(!strcmp(lab_old,cur->v->label))
			{
				app=cur->v->next;
				delete[] cur->v->label;
				delete[] cur->v->val;
				delete cur->v;
				cur->v=app;
			}
			else
			{
				for(cur_v=cur->v; cur_v->next!=NULL; cur_v=cur_v->next)
				{
					if(!strcmp(lab_old,cur_v->next->label))
					{
						app=cur_v->next->next;
						delete[] cur_v->next->label;
						delete[] cur_v->next->val;
						delete cur_v->next;
						cur_v->next=app;
						break;
					}
				}
			}
		}
	}
}

if(nature==1 || nature==0 || nature==2)
{
	if(nature==0)
		change_descr_lab(lab_old, "", "Variable", "", "");
	if(nature==1)
		change_descr_lab(lab_old, "", "Parameter", "", "");
	if(nature==2)
		change_descr_lab(lab_old, "", "Function", "", "");

	for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
	{ 
		cv=cur->search_var(NULL, lab_old);
		cv->num_lag=numlag;
		delete[] cv->val;
		cv->val=new double[numlag+1];
		for(i=0; i<numlag+1; i++)
			cv->val[i]=0;
		cv->param=nature;
		if(cv->param==1 || cv->num_lag>0)
			cv->data_loaded='-';
	}
}

unsavedChange = true;		// signal unsaved change
redrawRoot = true;			// request browser redraw

here_endprop:

break;


// Move variable/parameter (defined by tcl $vname)
case 79:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab_old );		// get var/par name in lab_old

sprintf( msg, "Select an object\nto move '%s' to", lab_old );

lab1 =  choose_object( msg );
if ( lab1 == NULL || ! strcmp( lab1, r->label ) )		// same object?
	break;
	
cv = r->search_var( NULL, lab_old );
if ( cv->param == 1 || cv->num_lag > 0 ) 
	cv->data_loaded = '-';
for ( cur = root->search( lab1 ); cur != NULL; cur = cur->hyper_next( cur->label ) )
	cur->add_var_from_example( cv );

for( cur = r; cur != NULL; cur = cur->hyper_next( cur->label ) )
{
	if( ! strcmp( lab_old, cur->v->label ) )
	{
		app = cur->v->next;
		delete[] cur->v->label;
		delete[] cur->v->val;
		delete cur->v;
		cur->v = app;
	}
	else
	{
		for( cur_v = cur->v; cur_v->next != NULL; cur_v = cur_v->next )
		{
			if( ! strcmp( lab_old, cur_v->next->label) )
			{
				app = cur_v->next->next;
				delete[] cur_v->next->label;
				delete[] cur_v->next->val;
				delete cur_v->next;
				cur_v->next = app;
				break;
			}
		}
	}
}

unsavedChange = true;		// signal unsaved change
redrawRoot = true;			// request browser redraw

break;


// Change variable/parameter (defined by tcl $vname) initial values
case 77:
// Change variable/parameter (defined by tcl $vname) sensitivity values
case 78:

done = ( *choice == 77 ) ? 1 : 2;

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab_old );		// get var/par name in lab_old
cv = r->search_var( NULL, lab_old );	// get var/par pointer
if ( cv == NULL )
	break;

// do lag selection, if necessary, for initialization/sensitivity data entry
lag = 0;							// lag option for the next cases (first lag)
if ( ( cv->param == 0 || cv->param == 2 ) && cv->num_lag > 1 )
{									// more than one lag to choose?
	// confirm which lag to use
	cmd( inter, "set T .lag" );
	cmd( inter, "newtop $T \"Lag\" {set choice 0}" );

	cmd( inter, "frame $T.i" );
	cmd( inter, "label $T.i.l -text \"Select the lag to edit\"" );
	cmd( inter, "set lag \"1\"" );
	cmd( inter, "entry $T.i.e -justify center -textvariable lag" );
	cmd( inter, "$T.i.e selection range 0 end" );
	sprintf( msg, "label $T.i.w -text \"Valid values are: 1 to %d\" -fg red", cv->num_lag );
	cmd( inter, msg );
	cmd( inter, "pack $T.i.l $T.i.e $T.i.w -pady 5" );
	cmd( inter, "pack $T.i" );
	cmd( inter, "okcancel $T b { set choice $lag } { set choice 0 }");
	cmd( inter, "bind $T <KeyPress-Return> { set choice $lag }");
	*choice=-1;
	cmd( inter, "focus $T.i.e" );
	cmd( inter, "showtop $T" );
	while ( *choice == -1 )		// wait for user action
		Tcl_DoOneEvent( 0 );
	cmd( inter, "destroytop $T" );
	
	lag = abs( *choice ) - 1;	// try to extract chosed lag
	
	if ( lag == -1 )
		break;
	// abort if necessary
	if ( lag < 0 || lag > ( cv->num_lag - 1 ) )
	{
		cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"Invalid lag selected.\n\nSelect a valid lag value for the variable or change the number of lagged values for this variable.\"" );
		break;
	}
}

// initialize
if( done == 1 )
{
	*choice = 0;		// set top window as parent
	set_all( choice, r, cv->label, lag );
}
// edit sensitivity analysis data
else
{
	*choice = 0;
	bool exist = false;
	sense *cs, *ps = NULL;

    if ( rsense == NULL )		// no sensitivity analysis structure yet?
        rsense = cs = new sense;
    else
    {
		// check if sensitivity data for the variable already exists 
		for ( cs = rsense, ps = NULL; cs != NULL; ps = cs, cs = cs->next )
			if ( ! strcmp( cs->label, cv->label ) && 
				 ( cs->param == 1 || cs->lag == lag ) )
			{
				exist = true;
				break;	// get out of the inner for loop
			}
			
		if ( ! exist )	// if new variable, append at the end of the list
		{
			for ( cs = rsense; cs->next != NULL; cs = cs->next );	// pick last
			cs->next = new sense;	// create new variable
			ps = cs;	// keep previous sensitivity variable
			cs = cs->next;
		}
	}
		
	if ( ! exist )		// do only for new variables in the list
	{
		cs->label = new char[ strlen( cv->label ) + 1 ];
		strcpy( cs->label, cv->label );
		cs->next = NULL;
		cs->nvalues = 0;
		cs->v = NULL;
		cs->entryOk = false;	// no valid data yet
	}
	else
		cs->entryOk = true;		// valid data already there

	// save type and specific lag in this case
	cs->param = cv->param;
	cs->lag = lag;
	
	dataentry_sensitivity( choice, cs, 0 );
	
	if ( ! cs->entryOk )		// data entry failed?
	{
		if( rsense == cs )		// is it the first variable?
			rsense = cs->next;	// update list root
		else
			ps->next = cs->next;// remove from sensitivity list		
		delete [ ] cs->label;	// garbage collection
		delete cs;
	}
	else
		unsavedSense = true;	// signal unsaved change
}

break;


//Exit the browser and run the simulation
case 1:

if(struct_loaded==0)
 break;

// save the current object & cursor position for quick reload
strcpy( lastObj, r->label );
cmd( inter, "if { ! [ string equal [ .l.s.son_name curselection ] \"\" ] } { set lastList 2 } { set lastList 1 }" );
cmd( inter, "if { $lastList == 1 } { set lastItem [ .l.v.c.var_name curselection ] } { set lastItem [ .l.s.son_name curselection ] }" );
cmd( inter, "if { $lastItem == \"\" } { set lastItem 0 }" );

cmd( inter, "set T .run" );
cmd( inter, "newtop $T \"Run Simulation\" { set choice 2 }" );

cmd(inter, "frame $T.f1 -bd 2 -relief groove");
cmd(inter, "label $T.f1.war1 -text \"Running the model configuration:\"");
sprintf(ch, "label $T.f1.war2 -text \"%s\" -fg red", simul_name);
cmd(inter, ch);
cmd(inter, "pack $T.f1.war1 $T.f1.war2");
cmd(inter, "pack $T.f1 -expand yes -fill x");

if(sim_num>1)
{
sprintf(ch, "label $T.war3 -text \"\\nNum. of simulations: %d\"", sim_num);
cmd(inter, ch);
sprintf(ch, "label $T.war4 -text \"Steps for each simulation (max.): %d\"", max_step);
cmd(inter, ch);
cmd(inter, "pack $T.war3 $T.war4");

cmd(inter, "frame $T.f2 -bd 2 -relief groove");
cmd(inter, "label $T.f2.war5 -text \"Results files (single simulations): \"");
sprintf(ch, "label $T.f2.war6 -text \"from: %s_%d.res\\nto: %s_%d.res\"", simul_name, seed, simul_name, seed+sim_num-1);
cmd(inter, ch);
cmd(inter, "checkbutton $T.f2.nores -text \"Skip generating results files\" -variable no_res");
cmd(inter, "pack $T.f2.war5 $T.f2.war6 $T.f2.nores");

Tcl_LinkVar(inter, "no_res", (char *)&no_res, TCL_LINK_INT);
Tcl_LinkVar(inter, "add_to_tot", (char *)&add_to_tot, TCL_LINK_INT);
Tcl_LinkVar(inter, "dozip", (char *)&dozip, TCL_LINK_INT);
Tcl_LinkVar(inter, "overwConf", (char *)&overwConf, TCL_LINK_INT);

cmd(inter, "checkbutton $T.dozip -text \"Generate zipped files\" -variable dozip");

cmd(inter, "frame $T.f3 -bd 2 -relief groove");
cmd(inter, "label $T.f3.war7 -text \"Totals file (simulations last step only): \"");
sprintf(ch, "label $T.f3.war8 -text \"%s_%d_%d.tot\"", simul_name, seed, seed+sim_num-1);
cmd(inter, ch);
sprintf(msg, "set choice [file exist %s_%d_%d.tot] ", simul_name, seed, seed+sim_num-1);
cmd(inter, msg);

if(*choice==1)
 {
 cmd(inter, "frame $T.f3.c");
 cmd(inter, "label $T.f3.c.l -text \"Warning: totals file already exists\" -fg red");
 cmd(inter, "radiobutton $T.f3.c.b1 -text \"Overwrite existing totals file\" -variable add_to_tot -value 0 -anchor w");
 cmd(inter, "radiobutton $T.f3.c.b2 -text \"Append to existing totals file\" -variable add_to_tot -value 1 -anchor w");
 cmd(inter, "pack $T.f3.c.l $T.f3.c.b1 $T.f3.c.b2 -expand yes -fill x");
 cmd(inter, "pack $T.f3.war7 $T.f3.war8 $T.f3.c");
 cmd(inter, "pack $T.f2 $T.f3 $T.dozip -pady 10 -expand yes -fill x");
 }
else
 cmd(inter, "pack $T.f2 $T.dozip -pady 10 -expand yes -fill x");
}
else
{
sprintf(ch, "label $T.war3 -text \"\\nSteps for simulation (max.): %d\"", max_step);
cmd(inter, ch);
cmd(inter, "label $T.war4 -text \"Results will be saved in memory only\\n\"");
cmd(inter, "pack $T.war3 $T.war4");
}

// Only ask to overwrite configuration if there are changes
if ( unsavedChange )
{
	cmd( inter, "set overwConf 1" );
	cmd( inter, "checkbutton $T.tosave -text \"Overwrite the existing configuration\nfile with the current values\" -variable overwConf" );
	cmd( inter, "pack $T.tosave" );
}
else
	cmd( inter, "set overwConf 0" );

cmd( inter, "okhelpcancel $T b { set choice 1 } { LsdHelp menumodel.html#run } { set choice 2 }" );
cmd(inter, "bind $T <KeyPress-Return> {set choice 1}");
cmd(inter, "focus -force $T.b.ok");

cmd( inter, "showtop $T topleftW" );
*choice=0;
while(*choice==0)
 Tcl_DoOneEvent(0);

cmd( inter, "destroytop $T" );
Tcl_UnlinkVar(inter, "no_res");
Tcl_UnlinkVar(inter, "add_to_tot");
Tcl_UnlinkVar(inter, "dozip");
Tcl_UnlinkVar(inter, "overwConf");

if(*choice==2)
  break;

*choice=1;

for(n=r; n->up!=NULL; n=n->up);

blueprint->empty();			    // update blueprint to consider last changes
set_blueprint(blueprint, n);

if ( overwConf == 1 )			// save if needed
	if ( ! save_configuration( r ) )
	{
		sprintf( msg , "set answer [ tk_messageBox -type okcancel -default cancel -icon warning -title Warning -message \"File '%s.lsd' cannot be saved.\n\nCheck if the drive or the file is set READ-ONLY. Press 'Ok' to run the simulation without saving the initialization file.\" ]; switch -- $answer { ok { set choice 1 } cancel { set choice 2 } } ", simul_name );
		cmd( inter, msg );
		if( *choice == 2 )
		{
			*choice=0;
			break;
		}
	}

*choice=1; 

return(n);


//Exit Lsd
case 11:

if ( discard_change( ) )	// unsaved configuration changes ?
	myexit(0);
break;


//Load a model
case 17:
case 38: //quick reload

reload = ( *choice == 38 ) ? true : false;

if ( reload )
	save_pos( r );

if(struct_loaded==1)
{ 
	if ( ! discard_change( ) )		// unsaved configuration?
	 break;

   cmd(inter, "set a [split [winfo children .] ]");  // remove old runtime plots
   cmd(inter, " foreach i $a {if [string match .plt* $i] {destroy $i}}");
   cmd( inter, "if [ winfo exists .lat ] { destroy .lat }" );	// remove lattice
   for(n=r; n->up!=NULL; n=n->up);
   r=n;
   cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
  
  empty_sensitivity(rsense); 	// discard sensitivity analysis data
  rsense=NULL;
  unsavedSense = false;			// nothing to save
  findexSens=0;
  nodesSerial=0;				// network node serial number global counter
}

actual_steps=0;					//Flag that no simulation has been run
unsavedData = false;			// no unsaved simulation results

if(*choice==17)
{
  *choice=0;
  strcpy(lastObj,"");	// disable last object for quick reload
  sprintf(lab, "set res %s", simul_name);
  cmd(inter, lab);
  if(strlen(path)>0)
   sprintf(msg, "set path \"%s\"", path);
  else
   sprintf(msg, "set path [pwd]");
  cmd(inter, msg);
  cmd(inter, "cd $path");
  cmd(inter, "set a \"\"");
  sprintf(msg, " set bah [tk_getOpenFile -title \"Load Lsd File\"  -defaultextension \".lsd\" -initialdir $path  -filetypes {{{Lsd Model Files} {.lsd}}  }]");

  cmd(inter, msg);
  
  cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .lsd $res] -1];set res [string range $res 0 $last]} {set choice 2}");
  if(*choice==2)
   {
    break;
   }
  lab1=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(lab, lab1);
  
  if(strlen(lab)==0)
   break;
  delete[] simul_name;
  simul_name=new char[strlen(lab)+1];
  strcpy(simul_name, lab);
  lab1=(char *)Tcl_GetVar(inter, "path",0);
  strcpy(msg, lab1);
  if(strlen(msg)>0)
   {delete[] path;
    path =new char[strlen(msg)+1];
    strcpy(path, msg);
    sprintf(msg, "cd %s", path);
    cmd(inter, "cd $path");
   }
  else
   strcpy(path,"");
 } 

switch ( load_configuration( r ) )
{
	case 1:							// file/path not found
		if( strlen( path ) > 0 )
			sprintf( msg, "tk_messageBox -type ok -title Error -icon error -message \"File not found.\\n\\nFile for model '%s' not found in directory '%s'.\"", simul_name, path );
		else
			sprintf( msg, "tk_messageBox -type ok -title Error -icon error -message \"File not found.\\n\\nFile for model '%s' not found in current directory\"", simul_name );
		cmd( inter, msg );
		*choice = 0;
		break;
		
	case 2:							// problem from MODELREPORT section
	case 3:							// problem from DESCRIPTION section
		autofill_descr( r );
	case 4:							// problem from DOCUOBSERVE section
		cmd( inter, "tk_messageBox -type ok -title Error -icon error -message \"Invalid or damaged file.\\n\\nPlease check if a proper file was selected and if the loaded configuration is correct.\"" );
		*choice = 0;
	default:						// load ok
		cmd( inter, "catch {unset ModElem}" );
		show_graph( r );
		unsavedChange = false;		// no changes to save
		redrawRoot = true;			// force browser redraw
		if ( ! reload )
			cmd( inter, "set cur 0" ); // point for first var in listbox
}

// restore pointed object and variable
n = restore_pos( r );
if ( n != r )
{
	*choice = 0;
	return n;
}

break;

	
//Save a model
case 73:
case 18:

saveAs = ( *choice == 73 ) ? true : false;

Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);

if(struct_loaded==0)
{
	cmd( inter, "set answer [ tk_messageBox -type okcancel -default cancel -icon warning -title Warning -message \"No configuration to save.\\n\\nPress 'Ok' to save an empty configuration file.\" ]; switch -- $answer { ok { set done 1 } cancel { set done 2 } } " );

   if(done==2)
	{Tcl_UnlinkVar(inter, "done");
	 break;
	}
	saveAs = true;	// require file name to save
 }
if(actual_steps>0)
{ 
	cmd( inter, "set answer [ tk_messageBox -type okcancel -default cancel -icon warning -title Warning -message \"The loaded configuration is the final state of a simulation run.\\n\\nPress 'Ok' to save it anyway or 'Cancel' to abort saving.\" ]; switch -- $answer { ok { set done 1 } cancel { set done 2 } } " );

   if(done==2)
	{Tcl_UnlinkVar(inter, "done");
	 break;
	}
	saveAs = true;	// require file name to save
 }

done=0;
sprintf(lab, "set res %s", simul_name);
cmd(inter, lab);
if(strlen(path)>0)
 sprintf(msg, "set path \"%s\"", path);
else
 sprintf(msg, "set path [pwd]");
cmd(inter, msg);

if ( saveAs )			// only asks file name if instructed to or necessary
{
sprintf(msg, "set bah [tk_getSaveFile -title \"Save Lsd Model\" -defaultextension \".lsd\" -initialfile $res -initialdir [pwd] -filetypes {{{Lsd Model Files} {.lsd}} {{All Files} {*}} }]");
cmd(inter, msg);

cmd(inter, "set res $bah");

cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .lsd $res] -1];if {$last > 0} {set res [string range $res 0 $last]} {}} {set done 2}");
if(done==2)
 {Tcl_UnlinkVar(inter, "done");
  break;
 }
lab1=(char *)Tcl_GetVar(inter, "res",0);
strcpy(lab, lab1);

if(strlen(lab)==0)
 break;
delete[] simul_name;
simul_name=new char[strlen(lab)+1];
strcpy(simul_name, lab);
lab1=(char *)Tcl_GetVar(inter, "path",0);
strcpy(msg, lab1);
if(strlen(msg)>0)
 {delete[] path;
  path =new char[strlen(msg)+1];
  strcpy(path, msg);
  sprintf(msg, "%s", path);
  cmd(inter, "pwd");
  cmd(inter, "cd $path");
  cmd(inter, "pwd");
 }
else
 strcpy(path,"");
}

if ( ! save_configuration( r ) )
{
	sprintf( msg , "tk_messageBox -type ok -icon error -title Error -message \"File '%s.lsd' cannot be saved.\n\nThe model is NOT saved! Check if the drive or the file is set READ-ONLY, change file name or select a drive with write permission and try again.\"", simul_name );
	cmd( inter, msg );
}
	
Tcl_UnlinkVar(inter, "done");
break;


//Edit Objects' numbers
case 19:

for(n=r; n->up!=NULL; n=n->up);

*choice=0;
strcpy(lab, r->label);
set_obj_number(n, choice);
cmd( inter, "destroytop .ini" );
r=n->search(lab);

unsavedChange = true;			// signal unsaved change

break;


//Edit initial values for Objects currently selected or pointed by the browser (defined by tcl $vname)
case 21:

*choice=0;

// check if current or pointed object and save current if needed
lab1 = ( char * ) Tcl_GetVar( inter, "useCurrObj", 0 );
if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
{
	lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
	if ( lab1 == NULL )
		break;
	sscanf( lab1, "%s", lab_old );
	for ( n = r; n->up != NULL; n = n->up );
	n = n->search( lab_old );		// set pointer to $vname
	if ( n == NULL )
		break;
	cur2 = r;
	r = n;
}
else
	cur2 = NULL;

for(n=r; n->up!=NULL; n=n->up);

edit_data(n, choice, r->label);

unsavedChange = true;			// signal unsaved change
cmd( inter, "destroytop .ini" );

if ( cur2 != NULL )				// restore original current object
	r = cur2;


break;


//Empty the model
case 20:

if ( ! discard_change( ) )	// check for unsaved configuration changes
	break;

cmd( inter, "if { [ winfo exists .model_str ] == 1 } { wm withdraw .model_str }");
cmd( inter, "if [ winfo exists .lat ] { destroy .lat }" );	// remove lattice

for(n=r; n->up!=NULL; n=n->up);
n->empty();
empty_cemetery();
lsd_eq_file[0]=(char)NULL;
n->label = new char[strlen("Root")+1];
strcpy(n->label, "Root");
r=n;
strcpy(lastObj,"");	// disable last object for quick reload
actual_steps=0;
unsavedData = false;			// no unsaved simulation results
empty_descr();
empty_sensitivity(rsense); 	// discard sensitivity analysis data
rsense=NULL;
unsavedSense = false;			// nothing to save
findexSens=0;
nodesSerial=0;				// network node serial number global counter
add_description("Root", "Object", "(no description available)");      
cmd(inter, "catch {unset ModElem}");

unsavedChange = false;		// signal no unsaved change
redrawRoot = true;			// force browser redraw
cmd( inter, "set cur 0" ); 	// point for first var in listbox

break;


//Simulation manager: sets seeds, number of steps, number of simulations
case 22:

*choice=0;
// save previous values to allow canceling operation
temp[1] = sim_num; 
temp[2] = seed; 
temp[3] = max_step; 
temp[4] = when_debug;

Tcl_LinkVar(inter, "sim_num", (char *) &sim_num, TCL_LINK_INT);
Tcl_LinkVar(inter, "seed", (char *) &seed, TCL_LINK_INT);
Tcl_LinkVar(inter, "max_step", (char *) &max_step, TCL_LINK_INT);

cmd( inter, "set T .simset" );
cmd( inter, "newtop $T \"Simulation Settings\" { set choice 2 }" );

cmd(inter, "label $T.tit -text \"Settings for running the simulation\"");
cmd(inter, "frame $T.f -relief groove -bd 2");
cmd(inter, "frame $T.f.a -bd 2");
cmd(inter, "label $T.f.a.l -width 25 -text \"Number of simulation runs\"");
cmd(inter, "entry $T.f.a.e -textvariable sim_num -width 5");
cmd(inter, "$T.f.a.e selection range 0 end");
cmd(inter, "pack $T.f.a.l $T.f.a.e -side left -anchor w");
cmd(inter, "frame $T.f.b -bd 2");
cmd(inter, "label $T.f.b.l1 -width 25 -text \"Random numbers initial seed\"");
cmd(inter, "entry $T.f.b.e1 -textvariable seed -width 5");
cmd(inter, "$T.f.b.e1 selection range 0 end");
cmd(inter, "pack $T.f.b.l1 $T.f.b.e1 -side left -anchor w");
cmd(inter, "frame $T.f.c -bd 2");
cmd(inter, "label $T.f.c.l2 -width 25 -text \"Simulation steps\"");
cmd(inter, "entry $T.f.c.e2 -textvariable max_step -width 8");
cmd(inter, "$T.f.c.e2 selection range 0 end");
cmd(inter, "pack $T.f.c.l2 $T.f.c.e2 -side left -anchor w");

cmd(inter, "frame $T.f.d -bd 2");
cmd(inter, "label $T.f.d.l2 -width 25 -text \"Start debugger at step\"");
cmd(inter, "entry $T.f.d.e2 -textvariable when_debug -width 8");
cmd(inter, "$T.f.d.e2 selection range 0 end");
cmd(inter, "pack $T.f.d.l2 $T.f.d.e2 -side left -anchor w");

cmd(inter, "frame $T.f.e -bd 2");
cmd(inter, "label $T.f.e.l2 -width 25 -text \"Print until stack\"");
sprintf(msg, "set stack_info %d", stackinfo_flag);
cmd(inter, msg);
cmd(inter, "entry $T.f.e.e2 -textvariable stack_info -width 8");
cmd(inter, "$T.f.e.e2 selection range 0 end");
cmd(inter, "pack $T.f.e.l2 $T.f.e.e2 -side left -anchor w");

cmd(inter, "pack $T.f.a $T.f.b $T.f.c $T.f.d $T.f.e -anchor w");
cmd(inter, "pack $T.tit $T.f -expand yes -fill both");
cmd( inter, "okhelpcancel $T b { set choice 1 } { LsdHelp menurun.html#simsetting } { set choice 2 }" );
cmd(inter, "focus $T.f.a.e");
cmd(inter, "bind $T.f.a.e <KeyPress-Return> {focus $T.f.b.e1; $T.f.b.e1 selection range 0 end}");
cmd(inter, "bind $T.f.b.e1 <KeyPress-Return> {focus $T.f.c.e2; $T.f.c.e2 selection range 0 end}");
cmd(inter, "bind $T.f.c.e2 <KeyPress-Return> {focus $T.f.d.e2; $T.f.d.e2 selection range 0 end}");
cmd(inter, "bind $T.f.d.e2 <KeyPress-Return> {focus $T.f.e.e2; $T.f.e.e2 selection range 0 end}");
cmd(inter, "bind $T.f.e.e2 <KeyPress-Return>  {focus $T.b.ok}");

cmd( inter, "showtop $T centerS" );
while(*choice==0)
 Tcl_DoOneEvent(0);

cmd(inter, "destroytop $T");

if ( *choice == 2 )	// Escape - revert previous values
{
	sim_num = temp[1];
	seed = temp[2];
	max_step = temp[3];
	when_debug = temp[4];
}
else
{
	cmd(inter, "set choice $stack_info");
	stackinfo_flag=*choice;

	unsavedChange = true;		// signal unsaved change
}

Tcl_UnlinkVar(inter, "seed");
Tcl_UnlinkVar(inter, "sim_num");

break;


//Move browser to Object pointed on the graphical model map
case 24:

*choice=0;

if(res_g==NULL)
  break;

for(n=r; n->up!=NULL; n=n->up);
n=n->search(res_g);

if ( n != r )
{
	redrawRoot = true;	// force browser redraw
	cmd( inter, "set cur 0; set listfocus 1; set itemfocus 0" ); // point for first var in listbox
}

return n;


//Edit initial values of Objects pointed on the graphical map (NOT USED)
case 25:
*choice=0;

if(res_g==NULL)
  break;

for(n=r; n->up!=NULL; n=n->up);
r=n->search(res_g);
edit_data(n, choice, r->label);
cmd( inter, "destroytop .ini" );

unsavedChange = true;		// signal unsaved change
choice_g=0;

break;


//Enter the analysis of results module
case 26:

cmd( inter, "if [ winfo exists .model_str ] { wm withdraw .model_str }" );
cmd( inter, "wm withdraw ." );
analysis(choice);
cmd( inter, "wm deiconify ." );
cmd( inter, "if [ winfo exists .model_str ] { wm deiconify .model_str }" );

break;


//Remove all the debugging flags
case 27:

cmd( inter, "set answer [ tk_messageBox -type okcancel -default cancel -icon question -title \"Remove Debug Flags\" -message \"Confirm the removal of all debugging information?\\n\\nDebugger will not stop in any variable update.\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } }" );

if(*choice==1)
{
	for(n=r; n->up!=NULL; n=n->up);
	clean_debug(n);
	unsavedChange = true;		// signal unsaved change
}

break;


//Change Equation File from which to take the code to show
case 28:

cmd(inter, "set res1 [file tail [tk_getOpenFile -title \"Select New Equation File\" -initialdir [pwd] -filetypes {{{Lsd Equation Files} {.cpp}} {{All Files} {*}} }]]");

lab1=(char *)Tcl_GetVar(inter, "res1",0);
if ( lab1 == NULL || strlen ( lab1 ) == 0 )
	break;
sscanf( lab1, "%s", lab_old );
delete[] equation_name;
equation_name=new char[strlen(lab)+1];
strcpy(equation_name, lab);
unsavedChange = true;		// signal unsaved change

break;


//Shortcut to show equation window
case 29:

lab1=(char *)Tcl_GetVar(inter, "vname",0);
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf(lab1, "%s", lab_old);

show_eq(lab_old, choice);

break;


//Remove all the save flags
case 30:

cmd( inter, "set answer [ tk_messageBox -type okcancel -default cancel -icon question -title \"Remove Debug Flags\" -message \"Confirm the removal of all saving information?\\n\\nNo data will be saved.\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } }" );

if(*choice==1)
{
	for(n=r; n->up!=NULL; n=n->up);
	clean_save(n);
	unsavedChange = true;		// signal unsaved change
}

break;


//Show variables to be saved
case 39:

for(n=r; n->up!=NULL; n=n->up);
plog("\n\nVariables and parameters saved:\n");
show_save(n);

break;


//Show variables to be observed
case 42:

for(n=r; n->up!=NULL; n=n->up);
plog("\n\nVariables and parameters containing results:\n");
show_observe(n);

break;


//Show variables to be initialized
case 49:

for(n=r; n->up!=NULL; n=n->up);
plog("\n\nVariables and parameters relevant to initialize:\n");
show_initial(n);

break;


//Close all Runtime Plots
case 40:

cmd(inter, "set a [split [winfo children .] ]");
cmd(inter, " foreach i $a {if [string match .plt* $i] {destroy $i}}");

break;


//Remove all the plot flags
case 31:

cmd( inter, "set answer [ tk_messageBox -type okcancel -default cancel -icon question -title \"Remove Plot Flags\" -message \"Confirm the removal of all run time plot information?\\n\\nNo data will be plotted during run time.\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } }" );

if(*choice==1)
{
	for(n=r; n->up!=NULL; n=n->up);
	clean_plot(n);
	unsavedChange = true;		// signal unsaved change
}

break;


//Changes the number of instances of only the Object type shown
//in the browser or the pointed object (defined in tcl $vname)
case 33:

// check if current or pointed object and save current if needed
lab1 = ( char * ) Tcl_GetVar( inter, "useCurrObj", 0 );
if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
{
	lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
	if ( lab1 == NULL )
		break;
	sscanf( lab1, "%s", lab_old );
	for ( n = r; n->up != NULL; n = n->up );
	n = n->search( lab_old );		// set pointer to $vname
	if ( n == NULL )
		break;
	cur2 = r;
	r = n;
}
else
	cur2 = NULL;

*choice=0;
if(r->up==NULL)
 {
  cmd(inter, "tk_messageBox -title Error -icon error -type ok -message \"You cannot create copies of the 'Root' object.\n\nConsider, if necessary, to add a new parent object here: all the elements will be moved in the newly created object, which can be multiplied in many copies.\"");
  goto here_endinst;
 }

skip_next_obj(r, &num);
Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
sprintf(msg, "set num %d",num);
cmd(inter, msg);

cmd( inter, "set T .numinst" );
cmd( inter, "newtop $T \"Number of Instances\" { set choice 2 }" );

cmd(inter, "label $T.l1 -text \"Insert the new number of instances of object\"");
sprintf(msg, "label $T.l2 -text \"%s\" -fg red",r->label);
cmd(inter, msg);
cmd(inter, "pack $T.l1 $T.l2");
cmd(inter, "entry $T.ent -width 20 -relief sunken -textvariable num");
cmd(inter, "label $T.l3 -text \"(all groups of this object will be affected)\"");
cmd(inter, "pack $T.ent $T.l3 -pady 5");

cmd(inter, "frame $T.cp -relief groove -bd 2");
cmd(inter, "label $T.cp.l -text \"Copy from instance:\"");
cmd(inter, "set cfrom 1");
cmd(inter, "entry $T.cp.e -textvariable cfrom -width 10");
cmd(inter, "button $T.cp.compute -width -9 -text Compute -command {set conf 1; set choice 3}");
cmd(inter, "pack $T.cp.l $T.cp.e $T.cp.compute -side left");
cmd(inter, "pack $T.cp -pady 5 -expand yes -fill x");

cmd( inter, "okhelpcancel $T b { set choice 1 } { LsdHelp mdataobjn.html#this } { set choice 2 }" );
cmd( inter, "showtop $T" );
cmd(inter, "bind $T.ent <KeyPress-Return> {set choice 1}");
cmd(inter, "$T.ent selection range 0 end");
cmd(inter, "focus -force $T.ent");

here_objec_num1:
while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==3)
{	k=compute_copyfrom(r, choice);
	if(k>0)
	{
	sprintf(msg, "set cfrom %d",k);
	cmd(inter, msg);
	}
	cmd(inter, "set conf 0");
	*choice=0;
	goto here_objec_num1;
} 

cmd( inter, "destroytop $T" );
Tcl_UnlinkVar(inter, "num");

if(*choice==2)
  goto here_endinst;

cmd(inter, "set choice $cfrom");
k=*choice;
*choice=0;

for(i=0, cur=r->up;cur!=NULL; i++, cur=cur->up); 
chg_obj_num(&r, num, i, NULL, choice,k);

unsavedChange = true;		// signal unsaved change

here_endinst:
if ( cur2 != NULL )			// restore original current object
	r = cur2;

break;


//Browse through the model instance by instance
case 34:

// check if current or pointed object and save current if needed
lab1 = ( char * ) Tcl_GetVar( inter, "useCurrObj", 0 );
if ( lab1 != NULL && ! strcmp( lab1, "no" ) )
{
	lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
	if ( lab1 == NULL )
		break;
	sscanf( lab1, "%s", lab_old );
	for ( n = r; n->up != NULL; n = n->up );
	n = n->search( lab_old );		// set pointer to $vname
	if ( n == NULL )
		break;
	cur2 = r;
	r = n;
}
else
	cur2 = NULL;

deb(r, NULL,NULL,&fake);

if ( cur2 != NULL )			// restore original current object
	r = cur2;

break;


//Windows destroyed
case 35:

if ( discard_change( ) )	// check for unsaved configuration changes
	myexit(0);

break;


//create model report
case 36:

for(n=r; n->up!=NULL; n=n->up);

report(choice, n);

break;


//save result
case 37:

*choice=0;
if(actual_steps==0)
 {
	cmd( inter, "tk_messageBox -title Error -icon error -type ok -message \"Simulation not run, there is nothing to save.\\n\\nPlease select in the menu Run the option Run before using this option.\"" );
	break;
 }

//Choose a name
  cmd( inter, "newtop .n \"Save Results\" { set choice 2 }" );

  cmd(inter, "label .n.l1 -text \"Choose a base name\nfor the results file\"");

  time_t rawtime;
  time( &rawtime );
  struct tm *timeinfo;
  char ftime[80];
  timeinfo = localtime( &rawtime );
  strftime ( ftime, 80, "%Y%m%d-%H%M%S", timeinfo );

  sprintf( msg, "set lab \"result_%s_%s\"", simul_name, ftime );
  cmd( inter, msg );
  cmd(inter, "entry .n.e -width 35 -relief sunken -textvariable lab");
  cmd(inter, "label .n.l2 -text \"(all data saved will be stored in a '.res' file\\nand the configuration that produced it \\nwill be copied in a new '.lsd' file)\"");
  Tcl_LinkVar(inter, "dozip", (char *)&dozip, TCL_LINK_INT);
  cmd(inter, "checkbutton .n.dozip -text \"Generate zipped results file\" -variable dozip");
  cmd(inter, "pack .n.l1 .n.e .n.l2 .n.dozip -pady 10");
  cmd( inter, "okcancel .n b { set choice 1 } { set choice 2 }" );
  cmd( inter, "showtop .n centerW" );
  cmd(inter, "bind .n <KeyPress-Return> {set choice 1}");
  cmd(inter, ".n.e selection range 0 end");  
  cmd(inter, "focus -force .n.e");
  while(*choice==0)
   Tcl_DoOneEvent(0);

  cmd(inter, "if {[string length lab] == 0} {set choice 2}");
  cmd(inter, "destroytop .n");
  Tcl_UnlinkVar(inter, "dozip");
  
  if(*choice==2)
	break;

lab1=(char *)Tcl_GetVar(inter, "lab",0);
strcpy(lab, lab1);
sprintf(msg, "%s.lsd", simul_name);
sprintf(ch, "%s.lsd", lab);
sprintf(msg, "file copy -force %s.lsd %s.lsd", simul_name, lab);
cmd(inter, msg);
sprintf(msg, "\nLsd result file: %s.res\nLsd data file: %s.lsd\nSaving data...",lab,lab);
plog(msg);
cmd(inter, "focus .log");
//cmd(inter, "raise .log");

if( strlen( path ) == 0 )
	sprintf( msg, "%s.res", lab );
else
	sprintf( msg, "%s/%s.res", path, lab );
	
rf = new result( msg, "wt", dozip );	// create results file object
for( n = r; n->up != NULL; n = n->up );	// get root object
rf->title( n, 1 );						// write header
rf->data( n, 0, actual_steps );			// write all data
delete rf;								// close file and delete object
plog(" Done\n");

unsavedData = false;					// no unsaved simulation results

break;


//help on lsd
case 41:

cmd(inter, "LsdHelp QuickHelp.html");

break;


//Create automatically the documentation
case 43:

*choice=0;

cmd(inter, "set answer [tk_messageBox -message \"The automatic documentation will replace any previous documentation.\\n\\nDo you want to proceed?\" -type okcancel -title Warning -icon warning -default cancel]");
cmd(inter, "if {[string compare -nocase $answer ok] == 0} {set choice 0} {set choice 1}");

if(*choice==1)
  break;

cmd(inter, "newtop .warn \"Auto Documentation\" { set choice 2 }");

cmd(inter, "label .warn.l -text \"Choose which elements of the model should\nhave the documentation replaced\"");
cmd(inter, "frame .warn.o -relief groove -bd 2");
cmd(inter, "radiobutton .warn.o.var -text Variables -variable x -value 1");
cmd(inter, "radiobutton .warn.o.all -text \"All elements\" -variable x -value 2");
cmd(inter, "pack .warn.o.var .warn.o.all -anchor w");
cmd(inter, "pack .warn.l .warn.o -fill both -expand yes");

cmd( inter, "okhelpcancel .warn f { set choice 1 } { LsdHelp menumodel.html#auto_docu } { set choice 2 }" );

cmd( inter, "showtop .warn centerS");
cmd(inter, "focus -force .warn.f.ok");
cmd(inter, "bind .warn <KeyPress-Return> {.warn.f.ok invoke}");

cmd(inter, "set x 1");

while(*choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "destroytop .warn");

if(*choice==2)
 {*choice=0;
  break;
 } 
cmd(inter, "set choice $x");
if(*choice==1)
   auto_document( choice, NULL, "VARIABLES");
else
   auto_document( choice, NULL, "ALL"); 

unsavedChange = true;		// signal unsaved change

break;


//see model report
case 44:

sprintf(name_rep, "report_%s.html", simul_name);
sprintf(msg, "set choice [file exists %s]", name_rep);
cmd(inter, msg);
if(*choice == 0)
 {
  cmd(inter, "set answer [tk_messageBox -message \"Model report not found.\\n\\nYou may create a model report file from menu Model or press 'Cancel' to look for another HTML file.\" -type okcancel -title Warning -icon warning -default ok]");
  cmd(inter, "if {[string compare -nocase $answer ok] == 0} {set choice 0} {set choice 1}");
 if(*choice == 0)
  break;
 cmd(inter, "set fname [tk_getOpenFile -title \"Load Report File\" -defaultextension \".html\" -initialdir [pwd] -filetypes {{{HTML Files} {.html}} {{All Files} {*}} }]");
 cmd(inter, "if {$fname == \"\"} {set choice 0} {set fname [file tail $fname]; set choice 1}");
 if(*choice == 0)
  break;

 }
else
 {
 sprintf(msg, "set fname %s", name_rep);
 cmd(inter, msg);
 }

lab1=(char *)Tcl_GetVar(inter, "app",0);
cmd(inter, "set app $tcl_platform(osVersion)");

lab1=(char *)Tcl_GetVar(inter, "app",0);

if(*choice==1) //model report exists
  cmd(inter, "LsdHtml $fname");
  
break;


//save descriptions
case 45:

lab1=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(lab, lab1);

change_descr_text(lab);

unsavedChange = true;		// signal unsaved change

break;


//show the vars./pars. vname is using
case 46:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab );

*choice = 0;				// make . the parent window
scan_using_lab(lab, choice);

break;


//show the equations where vname is used 
case 47:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab );

*choice = 0;				// make . the parent window
scan_used_lab(lab, choice);

break;


//set the Html browser for Unix systems
case 48:

cmd( inter, "newtop .a \"Set Browser\" { set choice 2 }" );

cmd(inter, "set temp_var $HtmlBrowser");
cmd(inter, "label .a.l2 -text \"HTML Browser to use for help pages\"");
cmd(inter, "entry .a.v_num2 -width 30 -textvariable temp_var");
cmd(inter, "bind .a.v_num2 <Return> {set choice 1}");
cmd(inter, "pack .a.l2 .a.v_num2 -pady 10");

cmd( inter, "xokhelpcancel .a f Default { set temp_var mozilla } { set choice 1 } { LsdHelp lsdfuncMacro.html#V } { set choice 2 }" );
cmd( inter, "showtop .a centerW");

cmd(inter, ".a.v_num2 selection range 0 end");
cmd(inter, "focus -force .a.v_num2");

*choice=0;
while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==1)
 cmd(inter, "set HtmlBrowser $temp_var");

cmd(inter, "destroytop .a");

break;


//find an element of the model
case 50: 

cmd( inter, "newtop .srch \"Find Element\" { set choice 2 }" );

cmd(inter, "frame .srch.i -relief groove -bd 2");
cmd(inter, "label .srch.i.l1 -text \"Type the initial letters of the searched\nelement, the system will propose a name\"");
cmd(inter, "set bidi \"\"");
cmd(inter, "entry .srch.i.e -textvariable bidi");
cmd(inter, "label .srch.i.l2 -text \"Press Enter when the desired label appears\"");
cmd(inter, "pack .srch.i.l1 .srch.i.e .srch.i.l2 -pady 10");
cmd(inter, "pack .srch.i");
cmd( inter, "okcancel .srch b { set choice 1 } { set choice 2 }" );
cmd( inter, "showtop .srch centerW" );
cmd(inter, "bind .srch.i.e <KeyPress-Return> {set choice 1}");
cmd(inter, "focus .srch.i.e");
cmd(inter, "bind .srch.i.e <KeyRelease> {if { %N < 256 && [ info exists ModElem] } { set b [.srch.i.e index insert]; set c [.srch.i.e get]; set f [lsearch -glob $ModElem $c*]; if { $f !=-1 } {set d [lindex $ModElem $f]; .srch.i.e delete 0 end; .srch.i.e insert 0 $d; .srch.i.e index $b; .srch.i.e selection range $b end } { } } { } }");

*choice=0;
while(*choice==0)
	Tcl_DoOneEvent(0);
cmd(inter, "destroytop .srch");
if(*choice==2)
  break;

 
case 55: //arrive here from the list of vars used.

*choice = 0;
lab1=(char *)Tcl_GetVar(inter, "bidi",0);
strcpy(msg,lab1);
no_error=1;
cv=r->search_var(r, msg);
no_error=0;
if(cv!=NULL)
 {
 for(i=0, cur_v=cv->up->v; cur_v!=cv; cur_v=cur_v->next, i++);
 sprintf(msg, "set cur %d; set listfocus 1; set itemfocus $cur", i);
 cmd(inter, msg);
 redrawRoot = true;			// request browser redraw
 return cv->up;
 }
else
	 cmd( inter, "tk_messageBox -type ok -icon error -title Error -message \"Element not found.\n\nCheck the spelling of the element name.\"" );

break;


//upload in memory current equation file
case 51:
/*
replace lsd_eq_file with the eq_file. That is, make appear actually used equation file as the one used for the current configuration
*/
if(!strcmp(eq_file, lsd_eq_file))
  break;//no need to do anything

cmd(inter, "set answer [tk_messageBox -title Warning -icon warning -message \"The equations associated to the configuration file are going to be replaced with the equations used for the Lsd model program.\\n\\nPress 'Ok' to confirm.\" -type okcancel -default cancel]");
cmd(inter, "if {[string compare -nocase $answer ok] == 0} {set choice 1} {set choice 0}");
 if(*choice == 0)
  break;
strcpy(lsd_eq_file, eq_file);
unsavedChange = true;		// signal unsaved change

break;


//offload configuration's equations in a new equation file
case 52: 
/*
Used to re-generate the equations used for the current configuration file
*/
if(strlen(lsd_eq_file)==0 || !strcmp(eq_file, lsd_eq_file) )
 {cmd(inter, "tk_messageBox -title Warning -icon warning -message \"There are no equations to be offloaded differing from the current equation file.\" -type ok");
 *choice=0;
 break;
 }

sprintf(msg, "set res1 fun_%s.cpp", simul_name);
cmd(inter, msg);
sprintf(msg, "set tk_strictMotif 0; set bah [tk_getSaveFile -title \"Save Equation File\" -defaultextension \".cpp\" -initialfile $res1 -initialdir [pwd] -filetypes {{{Lsd Equation Files} {.cpp}} {{All Files} {*}} }]; set tk_strictMotif 1");
cmd(inter, msg);

cmd(inter,"if {[string length $bah] > 0} { set choice 1; set res1 [file tail $bah]} {set choice 0}");
if ( *choice == 0 )
  break;

lab1=(char *)Tcl_GetVar(inter, "res1",0);
strcpy(lab, lab1);

if(strlen(lab)==0)
 break;
f=fopen(lab, "wb");
fprintf(f, "%s", lsd_eq_file);
fclose(f);
cmd(inter, "tk_messageBox -title \"File Created\" -icon info -message \"The new equation file '$res1' has been created.\\n\\nYou need to generate a new Lsd model program to use these equations, replacing the name of the equation file in LMM with the command 'Model Compilation Options' (menu Model).\" -type ok");

break;


//Compare equation files
case 53: 

if ( strlen( lsd_eq_file ) == 0 )
  break;

sprintf( ch, "orig-eq_%s.tmp", simul_name);
f=fopen( ch, "wb" );
fprintf(f, "%s", lsd_eq_file);
fclose(f);

read_eq_filename(lab);
sprintf( msg, "LsdTkDiff %s %s", lab, ch );
cmd(inter, msg);

break;


 //toggle ignore eq. file controls
case 54:

cmd(inter, "set choice $ignore_eq_file");
ignore_eq_file=*choice;

break; 


//toggle fast lattice updating
case 56:

cmd(inter, "set choice $lattype");
lattice_type=*choice;
sprintf( msg, "\nLattice updating mode: %s\n", lattice_type == 0 ? "infrequent cells changes" : "frequent cells changes" );
plog( msg );

break; 


//Generate Latex report
case 57:

sprintf( ch, "report_%s.tex", simul_name );
f = fopen( ch, "wt" );
fprintf(f,"\\newcommand{\\lsd}[1] {\\textit{#1}}\n");
tex_report_init(root,f);
tex_report_observe(root, f);
tex_report(root,f);
fclose(f);

Tcl_SetVar( inter, "res1", ch, 0 );
cmd(inter, "tk_messageBox -title \"File Created\" -icon info -message \"The Latex version of the model report has been created.\\n\\nThe file '$res1' is available in the model directory.\" -type ok");

break;


// Move variable up in the list box
case 58:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab_old );

shift_var(-1, lab_old, r);

unsavedChange = true;		// signal unsaved change
redrawRoot = true;			// request browser redraw

break;


// Move variable down in the list box
case 59:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab_old );

shift_var(1, lab_old, r);

unsavedChange = true;		// signal unsaved change
redrawRoot = true;			// request browser redraw

break;


// Move object up in the list box
case 60:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab_old );

shift_desc(-1, lab_old, r);

unsavedChange = true;		// signal unsaved change
redrawRoot = true;			// request browser redraw

break;


// Move object down in the list box
case 61:

lab1 = ( char * ) Tcl_GetVar( inter, "vname", 0 );
if ( lab1 == NULL || ! strcmp( lab1, "" ) )
	break;
sscanf( lab1, "%s", lab_old );

shift_desc(1, lab_old, r);

unsavedChange = true;		// signal unsaved change
redrawRoot = true;			// request browser redraw

break;


//Create parallel sensitivity analysis configuration
case 62:

if (rsense!=NULL) 
  {
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);
	long ptsSa = num_sensitivity_points(rsense);	// total number of points in sensitivity space
	sprintf(msg, "\nSensitivity analysis space size: %ld", ptsSa);
	plog(msg);
	// Prevent running into too big sensitivity spaces (high computation times)
	if(ptsSa > MAX_SENS_POINTS)
	{
		plog("\nWarning: sensitivity analysis space size is too big!");
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}");
		if(*choice == 0)
			break;
	}
	
    for (i=1,cs=rsense; cs!=NULL; cs=cs->next)
        i*=cs->nvalues;
    cur=root->b->head;
    root->add_n_objects2(cur->label, i-1, cur);
    sensitivity_parallel(cur,rsense);
	unsavedChange = true;		// signal unsaved change
 	cmd(inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has changed your model structure, replicating the entire model for each sensitivity configuration.\\n\\nIf you want to preserve your original configuration file, save your new configuration using a different name BEFORE running the model.\"");
  }
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Initial Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

break;


//Create sequential sensitivity analysis configuration
case 63:

if (rsense!=NULL) 
{
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);
	long ptsSa = num_sensitivity_points(rsense);	// total number of points in sensitivity space
	sprintf(msg, "\nSensitivity analysis space size: %ld", ptsSa);
	plog(msg);
	// Prevent running into too big sensitivity spaces (high computation times)
	if(ptsSa > MAX_SENS_POINTS)
	{
		plog("\nWarning: sensitivity analysis space size is too big!");
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}");
		if(*choice == 0)
			break;
	}
	
	// save the current object & cursor position for quick reload
	save_pos( r );

    findexSens=1;
    sensitivity_sequential(&findexSens,rsense);
	sprintf( msg, "\nSensitivity analysis configurations produced: %ld", findexSens - 1 );
	plog( msg );
 	cmd(inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has created configuration files for the sequential sensitivity analysis.\\n\\nTo run the analysis first you have to create a 'No Window' version of the model program, using the 'Model'/'Generate 'No Window' Version' option in LMM and following the instructions provided. This step has to be done every time you modify your equations file.\\n\\nThen execute this command in the directory of the model:\\n\\n> lsd_gnuNW  -f  <configuration_file>  -s  <n>\\n\\nReplace <configuration_file> with the name of your original configuration file WITHOUT the '.lsd' extension and <n> with the number of the first configuration file to run (usually 1).\"");
	
	// now reload the previously existing configuration
	for ( n = r; n->up != NULL; n = n->up );
	r = n;
	cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
	load_configuration( r );
	// restore pointed object and variable
	n = restore_pos( r );
	if ( n != r )
	{
		*choice = 0;
		return n;
	}
}
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Initial Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

break;


//Create Monte Carlo (MC) random sensitivity analysis sampling configuration (over user selected point values)
case 71:

if (rsense!=NULL) 
{
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);
	long maxMC = num_sensitivity_points(rsense);	// total number of points in sensitivity space
	sprintf(msg, "\nSensitivity analysis space size: %ld", maxMC);
	plog(msg);

	// get the number of Monte Carlo samples to produce
	double sizMC;
	Tcl_LinkVar(inter, "sizMC", (char *)&sizMC, TCL_LINK_DOUBLE);
	cmd(inter, "newtop .s \"Num. of MC Samples\" { set choice 2 }");
	cmd(inter, "frame .s.i -relief groove -bd 2");
	cmd(inter, "label .s.i.l -text \"Type the Monte Carlo sample size\nas % of sensitivity space size.\n(from 0 to 100)\"");
	cmd(inter, "set sizMC \"10.0\"");
	cmd(inter, "entry .s.i.e -justify center -textvariable sizMC");
	cmd(inter, ".s.i.e selection range 0 end");
	cmd(inter, "label .s.i.w -text \"Requesting a too big\nsample is not recommended.\nThe sample size represents the\napproximated target average.\"");
	cmd(inter, "pack .s.i.l .s.i.e .s.i.w");
	cmd(inter, "pack .s.i");
	cmd( inter, "okcancel .s b { set choice 1 } { set choice 2 }" );
	cmd(inter, "focus .s.i.e");
	cmd(inter, "showtop .s centerW");
	*choice = 0;
	while(*choice == 0)
		Tcl_DoOneEvent(0);
	cmd(inter, "destroytop .s");
	Tcl_UnlinkVar(inter, "sizMC");
	if(*choice == 2)
		break;
	
	// Check if number is valid
	sizMC /= 100.0;
	if( (sizMC * maxMC) < 1 || sizMC > 1.0)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Invalid Monte Carlo sample size to perform the sensitivity analysis.\\n\\nSelect a number between 0% and 100% that produces at least one sample (in average).\"");
		*choice=0;
		break;
	}

	// Prevent running into too big sensitivity space samples (high computation times)
	if((sizMC * maxMC) > MAX_SENS_POINTS)
	{
		sprintf(msg, "\nWarning: sampled sensitivity analysis space size (%ld) is still too big!", (long)(sizMC * maxMC));
		plog(msg);
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}");
		if(*choice == 0)
			break;
	}
	
	// save the current object & cursor position for quick reload
	save_pos( r );

	sprintf(msg, "\nTarget sensitivity analysis sample size: %ld (%.1f%%)", (long)(sizMC * maxMC), 100 * sizMC);
	plog(msg);
    findexSens=1;
    sensitivity_sequential(&findexSens, rsense, sizMC);
	sprintf(msg, "\nSensitivity analysis samples produced: %ld", findexSens - 1);
	plog(msg);
 	cmd(inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has created configuration files for the Monte Carlo sensitivity analysis.\\n\\nTo run the analysis first you have to create a 'No Window' version of the model program, using the 'Model'/'Generate 'No Window' Version' option in LMM and following the instructions provided. This step has to be done every time you modify your equations file.\\n\\nThen execute this command in the directory of the model:\\n\\n> lsd_gnuNW  -f  <configuration_file>  -s  <n>\\n\\nReplace <configuration_file> with the name of your original configuration file WITHOUT the '.lsd' extension and <n> with the number of the first configuration file to run (usually 1).\"");
	
	// now reload the previously existing configuration
	for ( n = r; n->up != NULL; n = n->up );
	r = n;
	cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
	load_configuration( r );
	// restore pointed object and variable
	n = restore_pos( r );
	if ( n != r )
	{
		*choice = 0;
		return n;
	}
}
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Initial Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

break;


//Create Near Orthogonal Latin Hypercube (NOLH) sensitivity analysis sampling configuration
case 72:

if (rsense!=NULL) 
{
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);

	cmd(inter, "newtop .s \"NOLH DoE\" { set choice 2 }");
	cmd(inter, "frame .s.i -relief groove -bd 2");
	cmd(inter, "set extdoe 0");	// flag for using external DoE file
	cmd(inter, "checkbutton .s.i.c -text \"Use external design file\" -variable extdoe -command { if { $extdoe == 1 } { .s.o.c configure -state disabled; .s.o.e configure -state disabled; .s.i.e configure -state normal; .s.i.e selection range 0 end; focus .s.i.e } { .s.o.c configure -state normal; .s.o.e configure -state normal; .s.i.e configure -state disabled } }");
	cmd(inter, "label .s.i.l -text \"If required, type the name\nof the design file to be used\"");
	cmd(inter, "set NOLHfile \"NOLH.csv\"");
	cmd(inter, "entry .s.i.e -justify center -textvariable NOLHfile -state disabled");
	cmd(inter, "label .s.i.w -text \"The file must be in the same\nfolder of the selected configuration file.\nThe file must be in CSV format\nwith NO empty lines.\"");
	cmd(inter, "pack .s.i.c .s.i.l .s.i.e .s.i.w");
	cmd(inter, "frame .s.o -relief groove -bd 2");
	cmd(inter, "label .s.o.l -text \"Select NOLH table (number of variables)\"");
	cmd(inter, "set doesize Auto");	// flag for selecting the DoE size (0=default)
	cmd(inter, "ttk::combobox .s.o.c -text \"DoE number of variables\" -textvariable doesize -values [list Auto 7 11 16 22 29]");
	cmd(inter, "set doeext 0");	// flag for using extended number of samples
	cmd(inter, "checkbutton .s.o.e -text \"Use extended number of samples\" -variable doeext");
	cmd(inter, "pack .s.o.l .s.o.c .s.o.e");
	cmd(inter, "pack .s.i .s.o");
	cmd( inter, "okcancel .s b { set choice 1 } { set choice 2 }" );
	cmd(inter, "showtop .s centerW");
	*choice = 0;
	while(*choice == 0)
		Tcl_DoOneEvent(0);
	cmd(inter, "destroytop .s");
	if(*choice == 2)
		break;
	
	char NOLHfile[300];
	char const *extdoe = Tcl_GetVar(inter, "extdoe", 0);
	char const *doesize = Tcl_GetVar(inter, "doesize", 0);
	char const *doeext = Tcl_GetVar(inter, "doeext", 0);
	
	if ( *extdoe == '0' )
		strcpy( NOLHfile, "" );
	else
	{
		char const *fname = Tcl_GetVar(inter, "NOLHfile", 0);
		strcpy(NOLHfile, fname);
	}
	
	int doesz = strcmp( doesize, "Auto" ) ? atoi( doesize ) : 0;
	int samples = ( *doeext == '0') ? 0 : -1;

	// adjust an NOLH design of experiment (DoE) for the sensitivity data
	plog( "\nCreating design of experiments, it may take a while, please wait... " );
	design *NOLHdoe = new design( rsense, 1, NOLHfile, 1, samples, doesz );
	
	if ( NOLHdoe -> n == 0 )					// DoE configuration is not ok?
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"It was not possible to create a Non Orthogonal Latin Hypercube (NOLH) Design of Experiment (DoE) for the current sensitivity configuration.\\n\\nIf the number of variables (factors) is large than 29, an external NOLH has to be provided in the file NOLH.csv (empty lines not allowed).\"" );
		delete NOLHdoe;
		break;
	}

	// Prevent running into too big sensitivity space samples (high computation times)
	if ( NOLHdoe -> n > MAX_SENS_POINTS )
	{
		sprintf( msg, "\nWarning: sampled sensitivity analysis space size (%d) is still too big!", NOLHdoe -> n );
		plog( msg );
		cmd( inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}" );
		if( *choice == 0 )
		{
			delete NOLHdoe;
			break;
		}
	}
	
	// save the current object & cursor position for quick reload
	save_pos( r );

    findexSens = 1;
    sensitivity_doe( &findexSens, NOLHdoe );
	sprintf( msg, "\nSensitivity analysis samples produced: %ld", findexSens - 1 );
	plog( msg );
 	cmd( inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has created configuration files for the Monte Carlo sensitivity analysis.\\n\\nTo run the analysis first you have to create a 'No Window' version of the model program, using the 'Model'/'Generate 'No Window' Version' option in LMM and following the instructions provided. This step has to be done every time you modify your equations file.\\n\\nThen execute this command in the directory of the model:\\n\\n> lsd_gnuNW  -f  <configuration_file>  -s  <n>\\n\\nReplace <configuration_file> with the name of your original configuration file WITHOUT the '.lsd' extension and <n> with the number of the first configuration file to run (usually 1).\"" );
	
	delete NOLHdoe;

	// now reload the previously existing configuration
	for ( n = r; n->up != NULL; n = n->up );
	r = n;
	cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
	load_configuration( r );
	// restore pointed object and variable
	n = restore_pos( r );
	if ( n != r )
	{
		*choice = 0;
		return n;
	}
}
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Initial Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

break;


//Create Monte Carlo (MC) random sensitivity analysis sampling configuration (over selected range values)
case 80:

if (rsense!=NULL) 
{
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);

	// get the number of Monte Carlo samples to produce
	double sizMC;
	Tcl_LinkVar(inter, "sizMC", (char *)&sizMC, TCL_LINK_DOUBLE);
	cmd(inter, "newtop .s \"Num. of MC Samples\" { set choice 2 }");
	cmd(inter, "frame .s.i -relief groove -bd 2");
	cmd(inter, "label .s.i.l -text \"Type the Monte Carlo sample size\nas number of samples.\"");
	cmd(inter, "set sizMC \"10\"");
	cmd(inter, "entry .s.i.e -justify center -textvariable sizMC");
	cmd(inter, ".s.i.e selection range 0 end");
	if ( findexSens > 1 )			// there are previously saved sensitivity files?
	{
		cmd(inter, "set applst 1");	// flag for appending to existing configuration files
		cmd(inter, "checkbutton .s.i.c -text \"Append to existing\nconfiguration files\" -variable applst");
		cmd(inter, "pack .s.i.l .s.i.e .s.i.c");
	}
	else
		cmd(inter, "pack .s.i.l .s.i.e");
	cmd(inter, "pack .s.i");
	cmd( inter, "okcancel .s b { set choice 1 } { set choice 2 }" );
	cmd(inter, "focus .s.i.e");
	cmd(inter, "showtop .s centerW");
	*choice = 0;
	while(*choice == 0)
		Tcl_DoOneEvent(0);
	cmd(inter, "destroy .s");
	Tcl_UnlinkVar(inter, "sizMC");
	if(*choice == 2)
		break;
	
	// Check if number is valid
	if( sizMC < 1 )
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Invalid Monte Carlo sample size to perform the sensitivity analysis.\\n\\nSelect at least one sample.\"");
		*choice=0;
		break;
	}

	// Prevent running into too big sensitivity space samples (high computation times)
	if( sizMC  > MAX_SENS_POINTS )
	{
		sprintf(msg, "\nWarning: sampled sensitivity analysis space size (%ld) is too big!", (long)sizMC);
		plog(msg);
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}");
		if(*choice == 0)
			break;
	}
	
	// save the current object & cursor position for quick reload
	save_pos( r );

	// check if design file numberig should pick-up from previously generated files
	if ( findexSens > 1 )
	{
		const char *applst = Tcl_GetVar( inter, "applst", 0 );
		if ( *applst == '0' )
			findexSens = 1;
	}
	else
		findexSens = 1;
	
	// adjust a design of experiment (DoE) for the sensitivity data
	plog( "\nCreating design of experiments, it may take a while, please wait... " );
	design *rand_doe = new design( rsense, 2, "", findexSens, sizMC );

    sensitivity_doe( &findexSens, rand_doe );
	sprintf( msg, "\nSensitivity analysis samples produced: %ld", findexSens - 1 );
	plog( msg );
 	cmd( inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has created configuration files for the Monte Carlo sensitivity analysis.\\n\\nTo run the analysis first you have to create a 'No Window' version of the model program, using the 'Model'/'Generate 'No Window' Version' option in LMM and following the instructions provided. This step has to be done every time you modify your equations file.\\n\\nThen execute this command in the directory of the model:\\n\\n> lsd_gnuNW  -f  <configuration_file>  -s  <n>\\n\\nReplace <configuration_file> with the name of your original configuration file WITHOUT the '.lsd' extension and <n> with the number of the first configuration file to run (usually 1).\"" );

	delete rand_doe;
	
	// now reload the previously existing configuration
	for ( n = r; n->up != NULL; n = n->up );
	r = n;
	cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
	load_configuration( r );
	// restore pointed object and variable
	n = restore_pos( r );
	if ( n != r )
	{
		*choice = 0;
		return n;
	}
}
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Initial Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

break;


//Create Elementary Effects (EE) sensitivity analysis sampling configuration (over selected range values)
case 81:

if (rsense!=NULL) 
{
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);

	// get the number of Monte Carlo samples to produce
	int nLevels = 4, jumpSz = 2, nTraj = 10, nSampl = 100;
	Tcl_LinkVar(inter, "varSA", (char *)&varSA, TCL_LINK_INT);
	Tcl_LinkVar(inter, "nLevels", (char *)&nLevels, TCL_LINK_INT);
	Tcl_LinkVar(inter, "jumpSz", (char *)&jumpSz, TCL_LINK_INT);
	Tcl_LinkVar(inter, "nTraj", (char *)&nTraj, TCL_LINK_INT);
	Tcl_LinkVar(inter, "nSampl", (char *)&nSampl, TCL_LINK_INT);
	cmd(inter, "newtop .s \"Elementary Effects Samples\" { set choice 2 }");
	cmd(inter, "frame .s.i -relief groove -bd 2");
	cmd(inter, "label .s.i.l1 -text \"Number of trajectories (r)\n([expr $varSA + 1]\u00D7r samples to create)\"");
	cmd(inter, "entry .s.i.e1 -justify center -textvariable nTraj");
	cmd(inter, "label .s.i.l2 -text \"Trajectories pool size (M)\n(M > r enables optimization)\"");
	cmd(inter, "entry .s.i.e2 -justify center -textvariable nSampl");
	cmd(inter, "label .s.i.l3 -text \"Number of levels (p)\n(must be even)\"");
	cmd(inter, "entry .s.i.e3 -justify center -textvariable nLevels");
	cmd(inter, "label .s.i.l4 -text \"Jump size\n( \u0394\u00D7(p - 1) )\"");
	cmd(inter, "entry .s.i.e4 -justify center -textvariable jumpSz");
	cmd(inter, "label .s.i.t -text \"\nFor details on setting Elementary\nEffects sampling parameters\nsee Morris (1991),  Campolongo\net al. (2007) and Ruano et al. (2012).\"");
	cmd(inter, ".s.i.e1 selection range 0 end");
	cmd(inter, "pack .s.i.l1 .s.i.e1 .s.i.l2 .s.i.e2 .s.i.l3 .s.i.e3 .s.i.l4 .s.i.e4 .s.i.t");
	cmd(inter, "pack .s.i");
	cmd( inter, "okcancel .s b { set choice 1 } { set choice 2 }" );
	cmd(inter, "focus .s.i.e1");
	cmd(inter, "showtop .s centerW");
	*choice = 0;
	while(*choice == 0)
		Tcl_DoOneEvent(0);
	cmd(inter, "destroy .s");
	Tcl_UnlinkVar(inter, "varSA");
	Tcl_UnlinkVar(inter, "nLevels");
	Tcl_UnlinkVar(inter, "jumpSz");
	Tcl_UnlinkVar(inter, "nTraj");
	Tcl_UnlinkVar(inter, "nSampl");
	if(*choice == 2)
		break;
	
	// Check if numbers are valid
	if( nLevels < 2 || nLevels % 2 != 0 || nTraj < 2 || nSampl < nTraj || jumpSz < 1 )
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Invalid Elementary Effects configuration to perform the sensitivity analysis.\\n\\nCheck Morris (1991) and Campolongo et al. (2007) for details.\"");
		*choice=0;
		break;
	}
	
	// Prevent running into too big sensitivity space samples (high computation times)
	if( nTraj * ( varSA + 1 )  > MAX_SENS_POINTS )
	{
		sprintf(msg, "\nWarning: sampled sensitivity analysis space size (%ld) is too big!", (long)( nTraj * ( varSA + 1 ) ) );
		plog(msg);
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}");
		if(*choice == 0)
			break;
	}
	
	// save the current object & cursor position for quick reload
	save_pos( r );

	findexSens = 1;
	
	// adjust a design of experiment (DoE) for the sensitivity data
	plog( "\nCreating design of experiments, it may take a while, please wait... " );
	design *rand_doe = new design( rsense, 3, "", findexSens, nSampl, nLevels, jumpSz, nTraj );

    sensitivity_doe( &findexSens, rand_doe );
	sprintf( msg, "Done\nSensitivity analysis samples produced: %ld", findexSens - 1 );
	plog( msg );
 	cmd( inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has created configuration files for the Elementary Effects sensitivity analysis.\\n\\nTo run the analysis first you have to create a 'No Window' version of the model program, using the 'Model'/'Generate 'No Window' Version' option in LMM and following the instructions provided. This step has to be done every time you modify your equations file.\\n\\nThen execute this command in the directory of the model:\\n\\n> lsd_gnuNW  -f  <configuration_file>  -s  <n>\\n\\nReplace <configuration_file> with the name of your original configuration file WITHOUT the '.lsd' extension and <n> with the number of the first configuration file to run (usually 1).\"" );

	delete rand_doe;
	
	// now reload the previously existing configuration
	for ( n = r; n->up != NULL; n = n->up );
	r = n;
	cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
	load_configuration( r );
	// restore pointed object and variable
	n = restore_pos( r );
	if ( n != r )
	{
		*choice = 0;
		return n;
	}
}
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Initial Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

break;


//Load a sensitivity analysis configuration
case 64:
	
	// check a model is already loaded
	if(struct_loaded==0)
	{ 
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"There is no model loaded.\\n\\nPlease load one before trying to load a sensitivity analysis configuration.\"");
		break;
    } 
	// check for existing sensitivity data loaded
	if (rsense!=NULL) 
	{
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"There is sensitivity data already loaded.\\n\\nPress 'Ok' if you want to discard the existing data before loading a new sensitivity configuration.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 2}}");
		if(*choice == 2)
			break;
		
		// empty sensitivity data
		empty_sensitivity(rsense); 			// discard read data
		rsense=NULL;
		unsavedSense = false;				// nothing to save
		findexSens=0;
	}
	// set default name and path to conf. file folder
	sprintf(lab, "set res %s_sens", simul_name);
	cmd(inter, lab);
	if(strlen(path)>0)
		sprintf(msg, "set path \"%s\"", path);
	else
		sprintf(msg, "set path [pwd]");
	cmd(inter, msg);
	cmd(inter, "cd $path");
	// open dialog box to get file name & folder
	sprintf(msg, " set bah [tk_getOpenFile -title \"Load Sensitivity Analysis File\" -defaultextension \".txt\" -initialfile $res -initialdir $path  -filetypes {{{Sensitivity analysis text files} {.txt}}  }]");
	cmd(inter, msg);
	cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .txt $res] -1];set res [string range $res 0 $last]} {set choice 2}");
	if(*choice==2)
		break;
	lab1=(char *)Tcl_GetVar(inter, "res",0);
	lab2=(char *)Tcl_GetVar(inter, "path",0);
	// form full name
	if(sens_file!=NULL)
		delete sens_file;
	sens_file=new char[strlen(lab1)+strlen(lab2)+7];
	if(strlen(lab1)>0)
	{
		cmd(inter, "cd $path");
		sprintf(sens_file,"%s/%s.txt",lab2,lab1);
	}
	else
		sprintf(sens_file,"%s.txt",lab1);
	// read sensitivity file (text mode)
	f=fopen(sens_file, "rt");
	if(f==NULL)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Sensitivity analysis file not found.\"");
		break;
	}

	// read data from file (1 line per element, '#' indicate comment)
	while(!feof(f))
	{	// read element by element, skipping comments
		fscanf(f, "%s", lab);				// read string
		while(lab[0]=='#')					// start of a comment
		{
			do								// jump to next line
				cc=fgetc(f);
			while(!feof(f) && cc!='\n');
			fscanf(f, "%s", lab);			// try again
		}

		if(feof(f))							// ended too early?
			break;

		for(n=r; n->up!=NULL; n=n->up);		// check if element exists
		cv=n->search_var(n,lab);
		if(cv==NULL || (cv->param!=1 && cv->num_lag==0))
			goto error64;					// and not parameter or lagged variable
		// create memory allocation for new variable		
		if (rsense==NULL)					// allocate first element
			rsense=cs=new sense;
		else								// allocate next ones
		{
			cs->next=new sense;
			cs=cs->next;
		}
		cs->v=NULL;							// initialize struct pointers
		cs->next=NULL;

		cs->label=new char[strlen(lab)+1];  // save element name
		strcpy(cs->label,lab);
		// get lags and # of values to test
		if(!fscanf(f, "%d %d:", &cs->lag, &cs->nvalues))
			goto error64;
		
		if(cs->lag==0)						// adjust type and lag #
			cs->param=1;
		else
		{
			cs->param=0;
			cs->lag=abs(cs->lag)-1;
		}

		cs->v=new double[cs->nvalues];		// get values
		for (i=0; i < cs->nvalues; i++)
			if(!fscanf(f, "%lf", &cs->v[i]))
				goto error64;
	}	
	fclose(f);
	break;
	
	// error handling
	error64:
		empty_sensitivity(rsense); 			// discard read data
		rsense=NULL;
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Invalid sensitivity analysis file.\"");
		fclose(f);
		break;


//Save a sensitivity analysis configuration
case 65:

	// check for existing sensitivity data loaded
	if (rsense==NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon warning -title \"Sensitivity Analysis\" -message \"There is no sensitivity data to save.\\n\\nBefore using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Initial Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");
		break;
	}
	// default file name and path
	sprintf(lab, "set res %s_sens", simul_name);
	cmd(inter, lab);
	if(strlen(path)>0)
		sprintf(msg, "set path \"%s\"", path);
	else
		sprintf(msg, "set path [pwd]");
	cmd(inter, msg);
	cmd(inter, "cd $path");
	// open dialog box to get file name & folder
	sprintf(msg, "set bah [tk_getSaveFile -title \"Save Sensitivity Analysis File\" -defaultextension \".txt\" -initialfile $res -initialdir $path  -filetypes {{{Sensitivity analysis text files} {.txt}} {{All Files} {*}} }]");
	cmd(inter, msg);
	cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .txt $res] -1];set res [string range $res 0 $last]} {set choice 2}");
	if(*choice==2)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Invalid sensitivity analysis file name or path.\"");
		break;
	}
	lab1=(char *)Tcl_GetVar(inter, "res",0);
	lab2=(char *)Tcl_GetVar(inter, "path",0);
	// form full name
	if(sens_file!=NULL)
		delete sens_file;
	sens_file=new char[strlen(lab2)+strlen(lab1)+7];
	if(strlen(lab2)>0)
	{
		cmd(inter, "cd $path");
		sprintf(sens_file,"%s/%s.txt",lab2,lab1);
	}
	else
		sprintf(sens_file,"%s.txt",lab1);
	// write sensitivity file (text mode)
	f=fopen(sens_file, "wt");  // use text mode for Windows better compatibility
	if(f==NULL)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Sensitivity analysis file not saved.\\n\\nPlease check the file name and path are valid.\"");
		break;
	}
	for(cs=rsense; cs!=NULL; cs=cs->next)
	{
		if(cs->param==1)
			fprintf(f, "%s 0 %d:", cs->label, cs->nvalues);	
		else
			fprintf(f, "%s -%d %d:", cs->label, cs->lag+1, cs->nvalues);
		for(i=0; i<cs->nvalues; i++)
			fprintf(f," %g", cs->v[i]);
		fprintf(f,"\n");
	}
	fclose(f);
	unsavedSense = false;			// nothing to save
	break;


//Show sensitivity analysis configuration
case 66:
	*choice=50;

	// check for existing sensitivity data loaded
	if (rsense==NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon warning -title \"Sensitivity Analysis\" -message \"There is no sensitivity data to show.\"");
		break;
	}
	// print data to log window
	plog("\n\nVariables and parameters set for sensitivity analysis :\n");
	for(cs=rsense; cs!=NULL; cs=cs->next)
	{
		if(cs->param==1)
			sprintf(msg, "Param: %s\t#%d:\t", cs->label, cs->nvalues);
		else
			sprintf(msg, "Var: %s(-%d)\t#%d:\t", cs->label, cs->lag+1, cs->nvalues);
		plog(msg);
		cmd(inter, "");

		for(i=0; i<cs->nvalues; i++)
		{
			sprintf(msg, ".log.text.text.internal insert end \"%g\\t\" highlight", cs->v[i]);
			cmd(inter, msg);
		}
		plog("\n");
	}
    cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
	break;


//Remove sensitivity analysis configuration
case 67:
	*choice=0;

	// check for existing sensitivity data loaded
	if (rsense==NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon warning -title \"Sensitivity Analysis\" -message \"There is no sensitivity data to remove.\"");
		break;
	}
	
	if ( ! discard_change( true, true ) )	// unsaved configuration?
		break;

	// empty sensitivity data
	empty_sensitivity(rsense); 			// discard read data
	rsense=NULL;
	unsavedSense = false;				// nothing to save
	findexSens=0;
	break;


//Create batch for multi-runs jobs and optionally run it
case 68:

	// check a model is already loaded
	if( struct_loaded == 0 )
		findexSens = 0;									// no sensitivity created
	else
		if ( ! discard_change( false ) )				// unsaved configuration?
			break;

	// check for existing NW executable
	sprintf(ch, "%s/%s", exec_path, exec_file);			// form full executable name
	cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
	if(*choice == 1)
	{
		// remove Windows extension, if present
		if((lab1 = strstr(ch, ".exe")) != NULL)
			lab1[0]='\0';
		else
			if((lab1 = strstr(ch, ".EXE")) != NULL) 
				lab1[0]='\0';
			
		strcat(ch, "NW.exe");							// add Windows ending
	}
	else
		strcat(ch, "NW");								// add Unix ending

	if ((f=fopen(ch, "rb")) == NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Create Batch\" -message \"The executable file 'lsd_gnuNW' was not found.\n\nPlease create the required executable file using the option 'Model'/'Generate 'No Window' Version' in LMM menu first.\"");
		break;
	}
	fclose(f);
	
	// check if serial sensitivity configuration was just created
	*choice=0;
	if(findexSens > 0)
		cmd(inter, "set answer [tk_messageBox -type yesnocancel -icon question -default yes -title \"Create Batch\" -message \"A sequential sensitivity set of configuration files was just created and can be used to create the script/batch.\n\nPress 'Yes' to confirm or 'No' to select a different set of files.\"]; switch -- $answer {yes {set choice 1} no {set choice 0} cancel {set choice 2}}"); 
	if(*choice == 2)
		break;
	
	// get configuration files to use
	int ffirst, fnext;
	if(*choice == 1)					// use current configuration files
	{
		ffirst=1;
		fnext=findexSens;
		findexSens = 0;
		lab1=simul_name;
		lab2=path;
		Tcl_SetVar(inter, "res", simul_name, 0);
		Tcl_SetVar(inter, "path", path, 0);
	}
	else								// ask for first configuration file
	{
		cmd(inter, "set answer [tk_messageBox -type yesnocancel -icon question -default no -title \"Create Batch\" -message \"Do you want to select a sequence of numbered configuration files?\n\nPress 'Yes' to choose the first file of the continuous sequence (format: 'name_NNN.lsd') or 'No' to select a different set of files (use 'Ctrl' to pick multiple files).\"]; switch -- $answer {yes {set choice 1} no {set choice 0} cancel {set choice 2}}"); 
		if(*choice == 2)
			break;
		else
			fSeq = *choice;
		
		if(strlen(simul_name) > 0)				// default name
			sprintf(msg, "set res \"%s_1.lsd\"", simul_name);
		else
			strcpy(msg, "set res \"\"");
		cmd(inter, msg);
		if(strlen(path) > 0)					// default path
			sprintf(msg, "set path \"%s\"", path);
		else
			sprintf(msg, "set path [pwd]");
		cmd(inter, msg);
		// open dialog box to get file name & folder
		if( fSeq )								// file sequence?
		{
			cmd(inter, "set bah [tk_getOpenFile -title \"Load First Configuration File\" -defaultextension \".lsd\" -initialfile $res -initialdir $path  -filetypes {{{Lsd Model Files} {.lsd}}} -multiple no]");
			cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res]; set last [expr [string last .lsd $res] - 1]; set res [string range $res 0 $last]; set numpos [expr [string last _ $res] + 1]; if {$numpos > 0} {set choice [expr [string range $res $numpos end]]; set res [string range $res 0 [expr $numpos - 2]]} {plog \"\nInvalid file name for sequential set: $res\n\"; set choice 0} } {set choice 0}");
			if(*choice == 0)
				break;
			ffirst=*choice;
			lab1=(char *)Tcl_GetVar(inter, "res",0);
			lab2=(char *)Tcl_GetVar(inter, "path",0);
			f=NULL;
			do									// search for all sequential files
			{
				if(strlen(lab2) == 0)			// default path
					sprintf(lab, "%s_%d.lsd", lab1, (*choice)++);
				else
					sprintf(lab, "%s/%s_%d.lsd", lab2, lab1, (*choice)++);
				if(f != NULL) fclose(f);
				f=fopen(lab, "r");
			}
			while(f != NULL);
			fnext=*choice - 1;
		}
		else									// bunch of files?
		{
			cmd( inter, "set bah [tk_getOpenFile -title \"Load Configuration Files\" -defaultextension \".lsd\" -initialfile $res -initialdir $path  -filetypes {{{Lsd Model Files} {.lsd}}} -multiple yes]" );
			cmd( inter,"set choice [llength $bah]; if {$choice > 0} {set res [lindex $bah 0]; set path [file dirname $res]; set res [file tail $res]; set last [expr [string last .lsd $res] - 1]; set res [string range $res 0 $last]; set numpos [expr [string last _ $res] + 1]; if {$numpos > 0} {set res [string range $res 0 [expr $numpos - 2]]}}" );
			if( *choice == 0 )
				break;
			ffirst = 1;
			fnext = *choice + 1;
			lab2=(char *)Tcl_GetVar(inter, "path",0);
		}
	}

	// confirm number of cores to use
	cmd( inter, "newtop .s \"Number of Processes\" { set choice 0 }" );
	cmd(inter, "frame .s.i -relief groove -bd 2");
	cmd(inter, "frame .s.i.c");
	cmd(inter, "label .s.i.c.l -text \"Number of parallel processes\"");
	cmd(inter, "set cores 4");
	cmd(inter, "entry .s.i.c.e -justify center -textvariable cores");
	cmd(inter, ".s.i.c.e selection range 0 end");
	cmd(inter, "label .s.i.c.w -text \"(using a number higher than the number\nof processors/cores is not recommended)\"");
	cmd(inter, "frame .s.i.b");
	cmd(inter, "label .s.i.b.l -text \"Batch file base name\"");
	cmd(inter, "entry .s.i.b.e -justify center -textvariable res");
	cmd(inter, "frame .s.i.o");
	natBat = 1;
	Tcl_LinkVar(inter, "natBat", (char *)&natBat, TCL_LINK_INT);
	cmd(inter, "checkbutton .s.i.o.n -text \"Use native batch format\" -variable natBat");
	Tcl_LinkVar(inter, "dozip", (char *)&dozip, TCL_LINK_INT);
	cmd(inter, "checkbutton .s.i.o.dozip -text \"Generate zipped files\" -variable dozip");
	cmd(inter, "pack .s.i.c.l .s.i.c.e .s.i.c.w");
	cmd(inter, "pack .s.i.b.l .s.i.b.e");
	cmd(inter, "pack .s.i.o.n .s.i.o.dozip");
	cmd(inter, "pack .s.i.c .s.i.b .s.i.o -pady 10");
	cmd(inter, "pack .s.i");
	cmd( inter, "okcancel .s b { set choice $cores } { set choice 0 }" );
	cmd(inter, "bind .s <KeyPress-Return> {set choice $cores}");
	cmd( inter, "showtop .s centerW" );
	cmd(inter, "focus .s.i.c.e");
	*choice=-1;
	while(*choice==-1)
		Tcl_DoOneEvent(0);
	cmd(inter, "destroytop .s");
	Tcl_UnlinkVar(inter, "natBat");
	Tcl_UnlinkVar(inter, "dozip");

	if(*choice==0)
		break;
	param=*choice;
	
	if(param < 1 || param > 64) 
		param=4;
	
	lab3 = ( char * ) Tcl_GetVar( inter, "res", 0 );
	
	// select batch format & create batch file
	char wpath[300];
	
	cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {if {$natBat == 1} {set choice 1} {set choice 0}} {if {$natBat == 1} {set choice 0} {set choice 1}}");
	if ( fSeq )
		if(*choice == 1)
			sprintf(lab, "%s/%s_%d_%d.bat", lab2, lab3, ffirst, fnext - 1);
		else
			sprintf(lab, "%s/%s_%d_%d.sh", lab2, lab3, ffirst, fnext - 1);
	else
		if( *choice == 1 )
			sprintf( lab, "%s/%s.bat", lab2, lab3 );
		else
			sprintf( lab, "%s/%s.sh", lab2, lab3 );
		
	f=fopen(lab, "wt");
	if(*choice == 1)						// Windows header
	{
		fprintf(f, "@echo off\nrem Batch generated by Lsd\n");
		fprintf(f, "echo Processing %d configuration files in up to %d parallel processes...\n", fnext - ffirst, param);

		// convert to Windows folder separators (\)
		for(i=0; i < strlen(ch); i++) 
			if(ch[i] == '/') ch[i]='\\';
		strcpy(wpath, lab2);
		for(i=0; i < strlen(wpath); i++) 
			if(wpath[i] == '/') wpath[i]='\\';
		
	}
	else									// Unix header
	{
		fprintf(f, "#!/bin/bash\n# Script generated by Lsd\n");
		fprintf(f, "echo \"Processing %d configuration files in up to %d parallel processes...\"\n", fnext - ffirst, param);

		if ( ! natBat )						// Unix in Windows?
		{
			if( strchr( ch, ':' ) != NULL )					// remove Windows drive letter
			{
				strcpy( msg, strchr( ch, ':' ) + 1 );
				strcpy( ch, msg );
			}
			if( strchr( lab2, ':' ) != NULL )				// remove Windows drive letter
			{
				strcpy( msg, strchr( lab2, ':' ) + 1 );
				strcpy( lab2, msg );
			}

			if ( ( lab3 = strstr( ch, ".exe" ) ) != NULL )	// remove Windows extension, if present
				lab3[0]='\0';
			else
				if ( ( lab3 = strstr( ch, ".EXE" ) ) != NULL ) 
					lab3[0]='\0';
		}
	}
	
	if( fSeq && ( fnext - ffirst ) > param )// if possible, work in blocks
	{
		num=(fnext - ffirst)/param;			// base number of cases per core
		sl=(fnext - ffirst)%param;			// remaining cases per core
		for(i=ffirst, j=1; j <= param; j++)	// allocates files by the number of cores
		{
			if(*choice == 1)				// Windows
				fprintf(f, "start \"Lsd Process %d\" /B \"%s\" -f %s\\%s -s %d -e %d %s 1>%s\\%s_%d.log 2>&1\n", j, ch, wpath, lab1, i, j <= sl ? i + num : i + num - 1, dozip ? "-z" : "", wpath, lab1, j);
			else							// Unix
				fprintf(f, "%s -f %s/%s -s %d -e %d %s >%s/%s_%d.log 2>&1 &\n", ch, lab2, lab1, i, j <= sl ? i + num : i + num - 1, dozip ? "-z" : "", lab2, lab1, j);
			j <= sl ? i+=num+1 : i+=num;
		}
	}
	else									// if not, do one by one
		for(i=ffirst, j=1; i < fnext; i++, j++)
			if( fSeq )
				if(*choice == 1)			// Windows
					fprintf(f, "start \"Lsd Process %d\" /B \"%s\" -f %s\\%s_%d.lsd %s 1>%s\\%s_%d.log 2>&1\n", j, ch, wpath, lab1, i, dozip ? "-z" : "", wpath, lab1, i);
				else						// Unix
					fprintf(f, "%s -f %s/%s_%d.lsd %s >%s/%s_%d.log 2>&1 &\n", ch, lab2, lab1, i, dozip ? "-z" : "", lab2, lab1, i);
			else
			{	// get the selected file names, one by one
				sprintf( msg, "set res [lindex $bah %d]; set res [file tail $res]; set last [expr [string last .lsd $res] - 1]; set res [string range $res 0 $last]", j - 1 );
				cmd( inter, msg );
				lab1 = ( char * ) Tcl_GetVar( inter, "res", 0 );
				
				if(*choice == 1)			// Windows
					fprintf(f, "start \"Lsd Process %d\" /B \"%s\" -f %s\\%s.lsd %s 1>%s\\%s.log 2>&1\n", j, ch, wpath, lab1, dozip ? "-z" : "", wpath, lab1);
				else						// Unix
					fprintf(f, "%s -f %s/%s.lsd %s >%s/%s.log 2>&1 &\n", ch, lab2, lab1, dozip ? "-z" : "", lab2, lab1);
			}
	
	if ( fSeq )
		if(*choice == 1)					// Windows closing
		{
			fprintf(f, "echo %d log files being generated: %s_1.log to %s_%d.log .\n", j - 1, lab1, lab1, j - 1);
			fclose(f);
		}
		else								// Unix closing
		{
			fprintf(f, "echo \"%d log files being generated: %s_1.log to %s_%d.log .\"\n", j - 1, lab1, lab1, j - 1);
			fprintf(f, "echo \"This terminal shell must not be closed during processing.\"\n");
			fclose(f);
			chmod(lab, ACCESSPERMS);		// set executable perms
		}
	else
		if(*choice == 1)					// Windows closing
		{
			fprintf(f, "echo %d log files being generated.\n", j - 1);
			fclose(f);
		}
		else								// Unix closing
		{
			fprintf(f, "echo \"%d log files being generated.\"\n", j - 1);
			fprintf(f, "echo \"This terminal shell must not be closed during processing.\"\n");
			fclose(f);
			chmod(lab, ACCESSPERMS);		// set executable perms
		}
		
	sprintf( msg, "\nParallel batch file created: %s", lab );
	plog( msg );
	
	if( ! natBat )
		break;

	// ask if script/batch should be executed right away
	cmd(inter, "set answer [tk_messageBox -type yesno -icon question -default no -title \"Run Batch\" -message \"The script/batch for running the configuration files was saved.\n\nPress 'Yes' if you want to start the script/batch as separated processes right now or 'No' to return.\"]; switch -- $answer {yes {set choice 1} no {set choice 2}}"); 
	if(*choice == 2)
		break;

	// start the job
	sprintf(msg, "set path \"%s\"", lab2);
	cmd(inter, msg);
	cmd(inter, "cd $path");

	cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
	if(*choice == 1)						// Windows?
		sprintf(msg, "exec %s &", lab);
	else									// Unix
		sprintf(msg, "exec %s &", lab);
    cmd(inter, msg);

	sprintf( msg, "\nParallel batch file started: %s", lab );
	plog( msg );
	
	cmd(inter, "tk_messageBox -type ok -icon info -title \"Run Batch\" -message \"The script/batch was started in separated process(es).\\n\\nThe results and log files are being created in the folder:\\n\\n$path\\n\\nCheck the '.log' files to see the results or use the command 'tail  -F  <name>.log' in a shell/command prompt to follow simulation execution (there is one log file per assigned process/core).\"");
break;


//Start NO WINDOW job as a separate background process
case 69:

	// check a model is already loaded
	if(struct_loaded==0)
	{ 
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Start 'No Window' Batch\" -message \"There is no model loaded.\\n\\nPlease select one before trying to start a 'No Window' batch.\"");
		break;
	}

	// confirm overwriting current configuration
	cmd( inter, "set b .batch" );
	cmd( inter, "newtop $b \"Start Batch\" { set choice 2 }" );

	cmd(inter, "label $b.war1 -text \"Starting 'No Window' batch for the model configuration: \"");
	sprintf(ch, "label $b.war2 -text \"%s\" -fg red", simul_name);
	cmd(inter, ch);
	sprintf(ch, "label $b.war3 -text \"Number of simulations: %d\"", sim_num);
	cmd(inter, ch);
	sprintf(ch, "label $b.war4 -text \"Time steps (max): %d\"", max_step);
	cmd(inter, ch);
	cmd(inter, "label $b.war5 -text \"Results file(s) (single simulation): \"");
	cmd(inter, "label $b.war7 -text \"Total file (last steps): \"");
	sprintf(ch, "label $b.war8 -text \"%s_%d_%d.tot\"", simul_name, seed, seed+sim_num-1);
	cmd(inter, ch);
	cmd(inter, "label $b.tosave -text \"\\nYou are going to overwrite the existing configuration file\\nand any results files in the destination folder\\n\"");
	Tcl_LinkVar(inter, "dozip", (char *)&dozip, TCL_LINK_INT);
	cmd(inter, "checkbutton $b.dozip -text \"Generate zipped files\" -variable dozip");

	
	if(sim_num>1)	// multiple runs case
	{
		sprintf(ch, "label $b.war6 -text \"from %s_%d.res to %s_%d.res\"", simul_name, seed, simul_name, seed+sim_num-1);
		cmd(inter, ch);
		cmd(inter, "set wind \"$b.war1 $b.war2 $b.war3 $b.war4 $b.war5 $b.war6 $b.war7 $b.war8 $b.tosave $b.dozip\"");
	}
	else			// single run case
	{
		sprintf(ch, "label $b.war6 -text \"%s_%d.res\"", simul_name, seed);
		cmd(inter, ch);
		cmd(inter, "set wind \"$b.war1 $b.war2 $b.war4 $b.war5 $b.war6 $b.war7 $b.war8 $b.tosave $b.dozip\"");
	}
	cmd(inter, "foreach i $wind {pack $i}");

	cmd( inter, "okcancel $b b { set choice 1 } { set choice 2 }" );
	cmd(inter, "bind $b <KeyPress-Return> {set choice 1}");
	cmd( inter, "showtop $b topleftW" );
	
	*choice=0;
	while(*choice==0)
		Tcl_DoOneEvent(0);
	
	cmd(inter, "destroytop $b");
	Tcl_UnlinkVar(inter, "dozip");

	if(*choice==2)
		break;

	// save the current configuration
	for(n=r; n->up!=NULL; n=n->up);
	blueprint->empty();			    // update blueprint to consider last changes
	set_blueprint(blueprint, n);
	
	if ( ! save_configuration( r ) )
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Start 'No Window' Batch\" -message \"Configuration file cannot be opened.\n\nCheck if the file is set READ-ONLY.");
		break;
	}

	// check for existing NW executable
	sprintf(lab, "%s/%s", exec_path, exec_file);		// form full executable name
	cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
	if(*choice == 1)
	{
		// remove Windows extension, if present
		if((lab1 = strstr(lab, ".exe")) != NULL)
			lab1[0]='\0';
		else
			if((lab1 = strstr(lab, ".EXE")) != NULL) 
				lab1[0]='\0';
			
		strcat(lab, "NW.exe");							// add Windows ending
	}
	else
		strcat(lab, "NW");								// add Unix ending

	if ((f=fopen(lab, "rb")) == NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Start 'No Window' Batch\" -message \"The executable file 'lsd_gnuNW' was not found.\n\nPlease create the required executable file using the option 'Model'/'Generate 'No Window' Version' in LMM first.\"");
		break;
	}
	fclose(f);
	
	// start the job
	if(strlen(path)>0)
		sprintf(msg, "set path \"%s\"", path);
	else
		sprintf(msg, "set path [pwd]");
	cmd(inter, msg);
	cmd(inter, "cd $path");

	if(*choice == 1)							// Windows?
		sprintf(msg, "exec %s -f %s %s >& %s.log  &", lab, struct_file, dozip ? "-z" : "", simul_name);
	else										// Unix
		sprintf(msg, "exec %s -f %s %s >& %s.log  &", lab, struct_file, dozip ? "-z" : "", simul_name);
    cmd(inter, msg);

	sprintf(msg, "tk_messageBox -type ok -icon info -title \"Start 'No Window' Batch\" -message \"The current configuration was started as a 'No Window' background job.\\n\\nThe results files are being created in the folder:\\n\\n$path\\n\\nCheck the '%s.log' file to see the results or use the command 'tail  -F  %s.log' in a shell/command prompt to follow simulation execution.\"", simul_name, simul_name);
	cmd(inter, msg);
break;


// toggle the state of the model structure windows
case 70:
break;

default:
sprintf(ch,"\nChoice %d not recognized\n",*choice);
plog(ch);
break;
}

*choice=0;
return r;
}


/****************************************************
CLEAN_DEBUG
****************************************************/
void clean_debug(object *n)
{
variable *cv;
object *co;
bridge *cb;

for(cv=n->v; cv!=NULL; cv=cv->next)
 cv->debug='n';

for(cb=n->b; cb!=NULL; cb=cb->next)
for(co=cb->head; co!=NULL; co=co->next)
 clean_debug(co);
}

/****************************************************
CLEAN_SAVE
****************************************************/
void clean_save(object *n)
{
variable *cv;
object *co;
bridge *cb; 

for(cv=n->v; cv!=NULL; cv=cv->next)
{
 cv->save=0;
 cv->savei=0;
}
for(cb=n->b; cb!=NULL; cb=cb->next)
for(co=cb->head; co!=NULL; co=co->next)
 clean_save(co);
}


/****************************************************
SHOW_SAVE
****************************************************/
void show_save(object *n)
{
variable *cv;
object *co;
bridge *cb;

int app;
for(cv=n->v; cv!=NULL; cv=cv->next)
 {if(cv->save==1)
  {
   if(cv->param==1)
    sprintf(msg, "Object: %s \tParam:\t", n->label);
   else
    sprintf(msg, "Object: %s \tVar:\t", n->label);
   plog(msg);

   cmd(inter, "");
   sprintf(msg, ".log.text.text.internal insert end \"%s\" highlight", cv->label);
   cmd(inter, msg);
   plog("\n");

  }
 }

for(cb=n->b; cb!=NULL; cb=cb->next)
 {
 if(cb->head==NULL)
  co=blueprint->search(cb->blabel);
 else
  co=cb->head; 
 show_save(co);
 }
}



/****************************************************
SHOW_OBSERVE
****************************************************/
void show_observe(object *n)
{
variable *cv;
object *co;
description *cd;
int app;
bridge *cb;


for(cv=n->v; cv!=NULL; cv=cv->next)
 {
 cd=search_description(cv->label);
 if(cd!=NULL && cd->observe=='y')
  {
   if(cv->param==1)
    sprintf(msg, "Object: %s \tParam:\t", n->label);
   else
    sprintf(msg, "Object: %s \tVar:\t", n->label);
   plog(msg);

   cmd(inter, "");
   sprintf(msg, ".log.text.text.internal insert end \"%s (%lf)\" highlight", cv->label, cv->val[0]);
   cmd(inter, msg);
   plog("\n");

  }
 }

for(cb=n->b; cb!=NULL; cb=cb->next)
 {
 if(cb->head==NULL)
  co=blueprint->search(cb->blabel);
 else
  co=cb->head; 
 show_observe(co);
 }
}

/****************************************************
SHOW_INITIAL
****************************************************/
void show_initial(object *n)
{
variable *cv, *cv1;
object *co;
description *cd;
int app, i;
char s[1];
bridge *cb;

for(cv=n->v; cv!=NULL; cv=cv->next)
 {
 cd=search_description(cv->label);
 if(cd!=NULL && cd->initial=='y')
  {
   if(cv->param==1)
    sprintf(msg, "Object: %s \tParam:\t", n->label);
   if(cv->param==0)
    sprintf(msg, "Object: %s \tVar:\t", n->label);
   if(cv->param==2)
    sprintf(msg, "Object: %s \tFunc:\t", n->label);
   plog(msg);

   cmd(inter, "");
   sprintf(msg, ".log.text.text.internal insert end \"%s \t\" highlight", cv->label);
   cmd(inter, msg);
   if(cd->init==NULL || strlen(cd->init)==0)
    {
    for(co=n; co!=NULL; co=co->hyper_next(co->label))
     {
      cv1=co->search_var(NULL,cv->label);
      sprintf(msg, " %g",cv1->val[0]);
      plog(msg);
     }
    }
   else
    {
     for(i=0; cd->init[i]!=0; i++)
      {
        switch(cd->init[i])
        {
        case '[': plog("\\\[");
                  break;
        case ']': plog("]");
                  break;
        case '"': plog("\\\"");
                  break;
        case '{': plog("\{");
                  break;
        default: sprintf(msg, "%c", cd->init[i]);
                 plog(msg);
                 break;          
                  
        }
  
      }
    
    } 
   plog("\n");

  }
 }

for(cb=n->b; cb!=NULL; cb=cb->next)
 {
 if(cb->head!=NULL)
  {co=cb->head; 
   show_initial(co);
  }
 } 
}


/****************************************************
CLEAN_PLOT
****************************************************/
void clean_plot(object *n)
{
variable *cv;
object *co;
bridge *cb;

for(cv=n->v; cv!=NULL; cv=cv->next)
 cv->plot=0;
 
for(cb=n->b; cb!=NULL; cb=cb->next)
for(co=cb->head; co!=NULL; co=co->next)
 clean_plot(co);
}

/****************************************************
WIPE_OUT
****************************************************/
void wipe_out(object *d)
{
object *cur;


cur=d->hyper_next(d->label);
if(cur!=NULL)
 wipe_out(cur);


delete_bridge(d);
}

/****************************************************
CHECK_LABEL
Control that the label l does not already exist in the model
Also prevents invalid characters in the names
****************************************************/
int check_label(char *l, object *r)
{
object *cur;
variable *cv;
bridge *cb;

Tcl_SetVar( inter, "nameVar", l, 0 );
 cmd( inter, "if [ regexp {^[a-zA-Z_][a-zA-Z0-9_]*$} $nameVar ] { set answer 1 } { set answer 0 }" );
const char *answer = Tcl_GetVar( inter, "answer", 0 );
if ( *answer == '0' )
	return 2;				// error if invalid characters (incl. spaces)

if(!strcmp(l, r->label) )
 return 1;

for(cv=r->v; cv!=NULL; cv=cv->next)
 if(!strcmp(l, cv->label) )
  return 1;

for(cb=r->b; cb!=NULL; cb=cb->next)
{
 if(cb->head==NULL)
  cur=blueprint->search(cb->blabel);
 else
  cur=cb->head; 
 if(check_label(l, cur)==1)
   return 1;
} 

return 0;
}



void control_tocompute(object *r, char *l)
{
object *cur;
variable *cv;
bridge *cb;

 for(cv=r->v; cv!=NULL; cv=cv->next)
  {
  if(cv->save==1)
   {
   sprintf(msg, "tk_messageBox -type ok -title Warning -icon warning -message \"Item '%s' set to be saved, but will not be available, since object '%s' is set to be not computed.\"", cv->label, l);
   cmd(inter, msg);
   } 
   }


for(cb=r->b; cb!=NULL; cb=cb->next)
{
 if(cb->head==NULL)
  cur=blueprint->search(cb->blabel);
 else
  cur=cb->head; 
  control_tocompute(cur,l);
  } 
  

}

char *choose_object( char *msg )
{
int done1;
char *ol = NULL;

Tcl_SetVar( inter, "msg", msg, 0 );
Tcl_LinkVar(inter, "done1", (char *) &done1, TCL_LINK_INT);

cmd( inter, "set TT .objs" );
cmd( inter, "newtop $TT \"Move\" { set done1 2 }" );
cmd(inter, "label $TT.l -text $msg");
cmd(inter, "pack $TT.l");
cmd(inter, "frame $TT.v");
cmd(inter, "scrollbar $TT.v.v_scroll -command \"$TT.v.lb yview\"");
cmd(inter, "listbox $TT.v.lb -selectmode single -yscroll \"$TT.v.v_scroll set\"");
cmd(inter, "pack $TT.v.lb $TT.v.v_scroll -side left -fill y");

insert_lb_object(root);

cmd(inter, "$TT.v.lb selection set 0");
cmd(inter, "pack $TT.v -pady 5");

cmd( inter, "okcancel $TT b { set done1 1 } { set done1 2 }" );	// insert ok button
cmd(inter, "bind $TT.v.lb <Double-1> { set done1 1 }");

cmd( inter, "showtop $TT centerW" );
cmd(inter, "focus -force $TT.v.lb");
done1=0;
while(done1==0)
 Tcl_DoOneEvent(0);

if ( done1 == 1 )
{
	cmd(inter, "set movelabel [$TT.v.lb get [$TT.v.lb curselection]]");
	ol=(char *)Tcl_GetVar(inter, "movelabel",0);
}

cmd(inter, "destroytop $TT");
Tcl_UnlinkVar(inter,"done1");
return ol;
}


void insert_lb_object(object *r)
{
object *cur;
bridge *cb;

sprintf(msg, "$TT.v.lb insert end %s", r->label);
cmd(inter, msg);

for(cb=r->b; cb!=NULL; cb=cb->next)
 {
  if(cb->head==NULL)
   cur=blueprint->search(cb->blabel);
  else
   cur=cb->head; 
  insert_lb_object(cur);
 }

}

void shift_var(int direction, char *vlab, object *r)
{
variable *cv, *cv1=NULL, *cv2=NULL;
if(direction==-1)
 {//shift up
  if(!strcmp(vlab, r->v->label))
   return; //variable already at the top
  if(!strcmp(vlab, r->v->next->label))
   {//second var, must become the head of the chain
    cv=r->v->next->next;//third
    cv1=r->v; //first
    r->v=r->v->next; //shifted up
    r->v->next=cv1;
    cv1->next=cv;
    return;
   }  
  for(cv=r->v; cv!=NULL; cv=cv->next)
   {
    if(!strcmp(vlab,cv->label) )
     {
      cv2->next=cv;
      cv1->next=cv->next;
      cv->next=cv1;
      return;
     }
    cv2=cv1;
    cv1=cv;
   }
 
 
 }
if(direction==1)
 {
  //move down
  if(!strcmp(vlab, r->v->label) )
   {//it the first
    if(r->v->next==NULL)
     return; //it is unique
    cv=r->v;//first
    cv1=cv->next->next;//third
    
    r->v=cv->next;//first is former second
    r->v->next=cv;//second is former first
    cv->next=cv1;//second points to third
    return;
   }
  for(cv=r->v; cv!=NULL; cv=cv->next)
   {
    
    if(!strcmp(vlab,cv->label) )
     {
      if(cv->next==NULL)
       return;//already at the end
      cv1->next=cv->next;
      cv->next=cv->next->next;
      cv1->next->next=cv;
      return;
     }
    cv1=cv;
   }
   
 } 
plog("\nError in move_var: should never reach this line\n"); 
}

void shift_desc(int direction, char *dlab, object *r)
{
bridge *cb, *cb1=NULL, *cb2=NULL;
if(direction==-1)
 {//shift up
  if(!strcmp(dlab, r->b->blabel))
   return; //object already at the top
  if(!strcmp(dlab, r->b->next->blabel))
   {//second var, must become the head of the chain
    cb=r->b->next->next;//third
    cb1=r->b; //first
    r->b=r->b->next; //shifted up
    r->b->next=cb1;
    cb1->next=cb;
    return;
   }  
  for(cb=r->b; cb!=NULL; cb=cb->next)
   {
    if(!strcmp(dlab,cb->blabel) )
     {
      cb2->next=cb;
      cb1->next=cb->next;
      cb->next=cb1;
      return;
     }
    cb2=cb1;
    cb1=cb;
   }
 
 
 }
if(direction==1)
 {
  //move down
  if(!strcmp(dlab, r->b->blabel) )
   {//it the first
    if(r->b->next==NULL)
     return; //it is unique
    cb=r->b;//first
    cb1=cb->next->next;//third
    
    r->b=cb->next;//first is former second
    r->b->next=cb;//second is former first
    cb->next=cb1;//second points to third
    return;
   }
  for(cb=r->b; cb!=NULL; cb=cb->next)
   {
    
    if(!strcmp(dlab,cb->blabel) )
     {
      if(cb->next==NULL)
       return;//already at the end
      cb1->next=cb->next;
      cb->next=cb->next->next;
      cb1->next->next=cb;
      return;
     }
    cb1=cb;
   }
   
 } 
plog("\nError in shift_desc: should never reach this line\n"); 
}

/*
	Save user position in browser
*/
void save_pos( object *r )
{
	// save the current object & cursor position for quick reload
	strcpy( lastObj, r->label );
	cmd( inter, "if { ! [ string equal [ .l.s.son_name curselection ] \"\" ] } { set lastList 2 } { set lastList 1 }" );
	cmd( inter, "if { $lastList == 1 } { set lastItem [ .l.v.c.var_name curselection ] } { set lastItem [ .l.s.son_name curselection ] }" );
	cmd( inter, "if { $lastItem == \"\" } { set lastItem 0 }" );
}

/*
	Restore user position in browser
*/
object *restore_pos( object *r )
{
	object *n;
	
	if ( strlen( lastObj ) > 0 )
	{
		for ( n = r; n->up != NULL; n = n->up );
		n = n->search( lastObj );
		if ( n != NULL )
		{
			cmd(inter, "if [ info exists lastList ] { set listfocus $lastList }");
			cmd(inter, "if [ info exists lastItem ] { set itemfocus $lastItem }");
			return n;
		}
	}
	return r;
}

/*
	Ask user to discard changes in configuration, if applicable
	Returns: 0: abort, 1: continue without saving
*/
bool discard_change( bool checkSense, bool senseOnly )
{
	// don't stop if simulation is runnig
	if ( running )
	{
		cmd( inter, "set answer [tk_messageBox -type ok -icon error -title Error -message \"Cannot quit while simulation is running.\n\nPress 'Ok' to continue simulation processing. If you really want to abort the simulation, press 'Stop' in the 'Log' window first.\"]" );
		return false;
	}
	// nothing to save?
	if ( ! unsavedData && ! unsavedChange && ! unsavedSense )
		return true;					// yes: simply discard configuration
	else								// no: ask for confirmation
		if ( ! senseOnly && unsavedData )
			cmd( inter, "set answer [tk_messageBox -type okcancel -default ok -icon warning -title \"Discard data?\" -message \"All data generated and not saved will be lost!\n\nDo you want to continue?\"]" );
		else
			if ( ! senseOnly && unsavedChange )
			{
				Tcl_SetVar( inter, "filename", simul_name , 0 );
				cmd( inter, "set answer [tk_messageBox -type okcancel -default ok -icon warning -title \"Discard changes?\" -message \"Recent changes to configuration '$filename' are not saved!\n\nDo you want to discard and continue?\"]" );
			}
			else						// there is unsaved sense data
				if ( checkSense )
					cmd( inter, "set answer [tk_messageBox -type okcancel -default ok -icon warning -title \"Discard changes?\" -message \"Recent changes to sensitivity data are not saved!\n\nDo you want to discard and continue?\"]" );
				else
					return true;		// checking sensitivity data is disabled

	cmd( inter, "if [ string equal -nocase $answer ok ] { set ans 1 } { set ans 0 }" );  
	const char *ans = Tcl_GetVar( inter, "ans", 0 );
	if ( atoi( ans ) == 1 )
		return true;
	else
		return false;
}

// Entry point function for access from the Tcl interpreter
int Tcl_discard_change( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[] )
{
	if ( discard_change( ) == 1 )
		Tcl_SetResult( inter, ( char * ) "ok", TCL_VOLATILE );
	else
		Tcl_SetResult( inter, ( char * ) "cancel", TCL_VOLATILE );
	return TCL_OK;
}
