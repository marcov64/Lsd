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

/****************************************************
RUN_TIME.CPP contains initialization and management of run time plotting


The functions contained here are:

- void prepare_plot(object *r, int id_sim)
Checks is there are lsd variables to plot. If not, returns immediately. Otherwise
initiliaze the run time globale variables. Namely, the vector of the labels for
the variables of plot. The plot window is initialized according to the id_sim name

- void count(object *r, int *i);
Recursive function that increments i of one for any variable to plot.

- void assign(object *r, int *i, char *lab);
Create a list of Variables to plot and create the list of labels (adding
the indexes if necessary) to be used in the plot.


- void init_plot(int i, int id_sim);
create the canvas for the plot, the lines, button, labels, etc.

- void plot_rt(variable *v)
the function used run time to plot the value of variable v

Other functions used here:
- object *skip_next_obj(object *t, int *count);
Contained in UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- object *go_brother(object *c);
Contained in UTIL.CPP. returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.


- void cmd(Tcl_Interp *inter, char *cc);
Contained in UTIL.CPP. Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.


- void plog(char *m);
print  message string m in the Log screen. It is in LSDMAIN.CPP

- void init_canvas(void);
Contained in ANALYSIS.CPP. Simply sets the colors for any integer number

****************************************************/


#include <tk.h>
#include "decl.h"

void plog(char const *msg);
void assign(object *r, int *i, char *lab);
void count(object *r, int *i);
object *skip_next_obj(object *t, int *count);
object *go_brother(object *c);
void init_canvas(void);
void init_plot(int i, int id_sim);
void cmd(Tcl_Interp *inter, char const *cc);


#ifdef DUAL_MONITOR
// Main window constraints
int sidemarg=440;	// horizontal right margin from the screen borders
int topmarg=450;	// vertical margins from the screen borders
int shift=20;		// new window shift
char intval[16];	// string buffer
#endif


extern char msg[];
extern Tcl_Interp *inter;
extern char **tp;
extern variable **list;
extern int max_step;
extern int cur_plt;
extern int t;
extern int plot_flag;
double ymin;
double ymax;
double *old_val;
double plot_step;

double min(double a, double b);

/**************************************
PREPARE_PLOT
**************************************/
void prepare_plot(object *r, int id_sim)
{
int i=0;
char lab[20];

ymax=ymin=0;

strcpy(lab, "");

count(r, &i);
sprintf(msg, "\n\nSimulation %d running ...\n", id_sim);
plog(msg);
cmd(inter, "update");

if(i==0)
  return;
tp=new char*[i];
list=new variable*[i];
old_val=new double[i];
i=0;
assign(r, &i, lab);
init_plot(i, id_sim);

}


/**************************************
COUNT
**************************************/
void count(object *r, int *i)
{
variable *a;
object *c;
bridge *cb;

for(a=r->v; a!=NULL; a=a->next)
 if(a->plot==1)
  *i=*i+1;

//
for(cb=r->b; cb!=NULL; cb=cb->next)
{
 for(c=cb->head; c!=NULL; c=c->next)
  count(c, i);
}
}

/**************************************
ASSIGN
**************************************/
void assign(object *r, int *i, char *lab)
{
variable *a;
object *c, *c1;
char cur_lab[30];
int j;
bridge *cb;

for(a=r->v; a!=NULL; a=a->next)
 if(a->plot==1)
	{
	 list[*i]=a; //assigns the address of a to the list to plot
	 sprintf(msg, "%s%s",a->label, lab);
	 tp[*i]=new char[strlen(msg)+1];
	 strcpy(tp[*i], msg);
	 *i=*i+1;
	}
//for(c=r->son; c!=NULL; c=skip_next_obj(c,&j))
for(cb=r->b; cb!=NULL; cb=cb->next)
 {//skip_next_obj(c,&j);
  c=cb->head;
  if(c->next!=NULL) //Multiple instances
	{for(j=1,c1=c ;c1!=NULL; c1=go_brother(c1), j++ )
	  {sprintf(cur_lab, "%s_%d", lab, j);
		 assign(c1, i, cur_lab);
	  }
	}
  else //Unique instance
	assign(c, i, lab);
 }
}

/**************************************
INIT_PLOT
**************************************/
void init_plot(int num, int id_sim)
{
init_canvas();
int i, j, k;

if(max_step>500)
 plot_step=1;
else
 plot_step=(500/max_step);
sprintf(msg,"set activeplot .plt%d", id_sim);
cmd(inter, msg);
cmd(inter, "if {[winfo exists $activeplot]==1} {destroy $activeplot} {}");
cmd(inter, "toplevel $activeplot");
cmd(inter, "wm resizable $activeplot 1 0");
cmd(inter, "wm protocol $activeplot WM_DELETE_WINDOW { }");
#ifdef DUAL_MONITOR
i=sidemarg+(id_sim-1)*shift;				// calculate window position
j=topmarg+(id_sim-1)*shift;
sprintf(intval,"%i",i);
Tcl_SetVar(inter, "posXrt", intval, 0);	// horizontal right margin from the screen borders
sprintf(intval,"%i",j);
Tcl_SetVar(inter, "posYrt", intval, 0);	// vertical margins from the screen borders
cmd(inter, "wm geometry $activeplot +$posXrt+$posYrt");
#else
cmd(inter, "wm geometry $activeplot +0+340");
#endif
sprintf(msg,"wm title $activeplot \"%d Run Time Plot\"", id_sim);
cmd(inter,msg);
cmd(inter, "frame $activeplot.c");
cmd(inter, "frame $activeplot.c.c  ");
cmd(inter, "set p $activeplot.c.c.cn");
cmd(inter, "scrollbar $activeplot.c.c.hscroll -orient horiz -command \"$p xview\"");

sprintf(msg, "canvas $p -width 500 -height 330 -relief sunken -background white -bd 2 -scrollregion {0 0 %d 400} -xscrollcommand \"$activeplot.c.c.hscroll set\"", max_step);
cmd(inter, msg);
cmd(inter, "pack $activeplot.c.c.hscroll -side bottom -expand yes -fill x");
sprintf(msg, "$p create line 0 302 %d 302", (int)((double)max_step*plot_step));
cmd(inter, msg);
sprintf(msg, "$p create line 0 151 %d 151 -fill grey60",(int)((double) max_step*plot_step));
cmd(inter, msg);
cmd(inter, "pack $p -expand yes -fill both -anchor w");
cmd(inter, "$p xview moveto 0");
//cmd(inter, "canvas $activeplot.c.yscale -width 70 -height 330");
cmd(inter, "canvas $activeplot.c.yscale -width 70 -height 370");
//cmd(inter, "pack $activeplot.c.yscale $activeplot.c.c -anchor w -side left -fill both -expand yes");
cmd(inter, "pack $activeplot.c.yscale -anchor w -side left -anchor n -expand no");
cmd(inter, "pack $activeplot.c.c -anchor w -side left -fill both -expand yes");
cmd(inter, "pack $activeplot.c -anchor w -expand yes -fill both");

for(i=0; i<(int)((double)max_step*plot_step); i+=100)
 {sprintf(msg, "$p create line %d 0 %d 310 -fill grey60", i, i);
  cmd(inter, msg);
  sprintf(msg, "$p create text %d 310 -font {{MS Times New Roman} 10} -text %d -anchor nw", i, i/(int)plot_step);
  cmd(inter, msg);

 }
cmd(inter, "$activeplot.c.yscale create line 70 4 70 303");
cmd(inter, "$activeplot.c.yscale create line 65 4 70 4");
cmd(inter, "$activeplot.c.yscale create line 65 150 70 150");
cmd(inter, "$activeplot.c.yscale create line 65 302 70 302");
cmd(inter, "$activeplot.c.yscale create text 2 2 -font {{MS Times New Roman} 10} -anchor nw -text \"\" -tag ymax");
cmd(inter, "$activeplot.c.yscale create text 2 150 -font {{MS Times New Roman} 10} -anchor w -text \"\" -tag medy");
cmd(inter, "$activeplot.c.yscale create text 2 300 -font {{MS Times New Roman} 10} -anchor sw -text \"\" -tag ymin");
cmd(inter, "set posiziona 0");
cmd(inter, "button $activeplot.c.yscale.go -text \"Center\" -command {set oldposiziona $posiziona; set posiziona 1}");
cmd(inter, "$activeplot.c.c.cn conf -xscrollincrement 1");
cmd(inter, "bind $activeplot.c.yscale.go <3> {set posiziona 2}");
cmd(inter, "pack $activeplot.c.yscale.go -anchor nw -expand yes -fill both");
cmd(inter, "checkbutton $activeplot.c.yscale.shift -text Scroll -variable posiziona -onvalue 2");
cmd(inter, "pack $activeplot.c.yscale.shift -anchor nw -expand yes -fill both");

cmd(inter, "$activeplot.c.yscale create window 3 310 -window $activeplot.c.yscale.go -anchor nw");
cmd(inter, "$activeplot.c.yscale create window 3 340 -window $activeplot.c.yscale.shift -anchor nw");
cmd(inter, "canvas $activeplot.fond -height 50");
cmd(inter, "pack $activeplot.fond -expand yes -fill both");
for(i=0, j=0, k=0; i<num; i++)
 {sprintf(msg, "$activeplot.fond create text %d %d -font {{MS Times New Roman} 10} -anchor nw -text %s -fill $c%d",4+j*100, k*12, tp[i], i);
  cmd(inter, msg);
  if(j<5)
	j++;
  else
	{k++;
	 j=0;
	}
 }

}

/**************************************
PLOT_RT
**************************************/
void plot_rt(variable *v)
{
int y1, y2;
double dy, step, value;
if(plot_flag==0)
 return;
if(ymax==ymin) //Very initial setting
 { if(v->val[0]>0)
	 {
	  ymax=v->val[0]*(1.001);
     ymin=v->val[0];
	 }
	else
	 {
	  ymax=v->val[0]*(0.009);
	  ymin=v->val[0];
	 }
   if(ymax==ymin) //It must be zero...
    {ymax=0.0001;
//     ymin=-0.001; //Most of the times, if initial vars are zero means that
// they are supposed to grow
    }

	sprintf(msg, "$activeplot.c.yscale itemconf ymax -text %lf", ymax);
	cmd(inter, msg);
	sprintf(msg, "$activeplot.c.yscale itemconf ymin -text %lf", ymin);
	cmd(inter, msg);
	sprintf(msg, "$activeplot.c.yscale itemconf medy -text %lf", (ymax-ymin)/2+ymin);
	cmd(inter, msg);

 }
if(v->val[0]>=ymax)
 {if(v->val[0]>=0)
	step=1.1;
  else
	step=0.9;
  sprintf(msg, "$activeplot.c.c.cn scale punto 0 300 1 %lf",(ymax-ymin)/(v->val[0]*step-ymin) );
  cmd(inter, msg);
  ymax=v->val[0]*step;
  sprintf(msg, "$activeplot.c.yscale itemconf ymax -text %g", ymax);
  cmd(inter, msg);
  sprintf(msg, "$activeplot.c.yscale itemconf medy -text %g", (ymax-ymin)/2+ymin);
  cmd(inter, msg);

 }

if(v->val[0]<=ymin)
 {if(v->val[0]>0)
	step=0.9;
  else
	step=1.1;
  value=min(v->val[0]*step, ymin-(ymax-ymin)/300);

  sprintf(msg, "$activeplot.c.c.cn scale punto 0 0 1 %lf",(ymax-ymin)/(ymax - value) );
  cmd(inter, msg);
  ymin=value;
	sprintf(msg, "$activeplot.c.yscale itemconf ymin -text %g", ymin);
  cmd(inter, msg);
  sprintf(msg, "$activeplot.c.yscale itemconf medy -text %g", (ymax-ymin)/2+ymin);
  cmd(inter, msg);

 }

if(t==1)
 {old_val[cur_plt]=v->val[0];
  cur_plt++;
  return;
 }


dy=(300-((v->val[0]-ymin)/(ymax-ymin))*300);
y1=(int)dy;
dy=(300-((old_val[cur_plt]-ymin)/(ymax-ymin))*300);
y2=(int)dy;
old_val[cur_plt]=v->val[0];

sprintf(msg, "$activeplot.c.c.cn create line %d %d %d %d -tag punto -fill $c%d", (t-1)*(int)plot_step,y2, t*(int)(plot_step), y1, cur_plt);
cmd(inter, msg);
cur_plt++;

}
