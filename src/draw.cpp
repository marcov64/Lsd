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
DRAW.CPP
Draw the graphical representation of the model. It is activated by
INTERF.CPP only in case a model is loaded.

The functions contained in this file are:

- void show_graph( object *t)
initialize the canvas and calls show_obj for the root

- void draw_obj(Tcl_Interp *inter, object *blk, object *t, int level, int center, int from)
recursive function that according to the level of the object type sets the
distances among the objects. Rather rigid, but it should work nicely
for most of the model structures. It assigns all the labels (above and below the
symbols) and the writing bound to the mouse.

- void put_node(Tcl_Interp *inter, int x1, int y1, int x2, int y2, char *str)
Draw the circle.

- void put_line(Tcl_Interp *inter, int x1, int y1, int x2, int y2)
draw the line

- void put_text(Tcl_Interp *inter, char *str, char *num, int x, int y, char *str2);
Draw the different texts and sets the bindings

Functions used here from other files are:

- object *skip_next_obj(object *t, int *i);
UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- void cmd(Tcl_Interp *inter, char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

- object *go_brother(object *cur);
UTIL.CPP returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.

****************************************************/


#include <tk.h>
#include "decl.h"

void cmd(Tcl_Interp *inter, char const *cc);
object *go_brother(object *cur);
object *skip_next_obj(object *t, int *i);
extern Tcl_Interp *inter;
void draw_obj(Tcl_Interp *ti, object *blk, object *t, int level, int center, int from);
void put_node(Tcl_Interp *interp, int x1, int y1, int x2, int y2, char *str);
void put_line(Tcl_Interp *interp, int x1, int y1, int x2, int y2);
void put_text(Tcl_Interp *inter, char *str, char *num, int x, int y, char *str2);

int range_type=90;
int step_level=15;
int step=10;


/****************************************************
SHOW_GRAPH

****************************************************/

void show_graph( object *t)
{
object *top;

//return;
cmd(inter, "set c .model_str");
//cmd(inter, "if {[winfo exists $c.c]==1} {wm deiconify $c; wm iconify .log; destroy $c.c} {if {[winfo exists $c]==1} {wm deiconify $c; wm iconify .log} {toplevel $c; wm transient $c .}}");
//cmd(inter, "if {[winfo exists $c.c]==1} {wm deiconify $c; wm iconify .log; destroy $c.c} {if {[winfo exists $c]==1} {wm deiconify $c; wm iconify .log} {toplevel $c; wm transient $c .; bind $c <FocusIn> {focus -force .l.v.c.var_name}}}");cmd(inter, "if {[winfo exists $c.c]==1} {wm deiconify $c; wm iconify .log; destroy $c.c} {if {[winfo exists $c]==1} {wm deiconify $c; wm iconify .log} {toplevel $c; wm transient $c .; bind $c <FocusIn> {focus -force .l.v.c.var_name}}}");
cmd(inter, "if {[winfo exists $c.c]==1} {wm deiconify $c; destroy $c.c} {if {[winfo exists $c]==1} {wm deiconify $c} {toplevel $c; wm transient $c .}}");

cmd(inter, "wm title $c \"Lsd Model Structure\"");
cmd(inter, "bind $c <Destroy> {set choice 35}");

cmd(inter, "canvas $c.c -width 15.c -height 10.c");
cmd(inter, "pack $c.c");
cmd(inter, "bind $c.c <1> {.log.text.text insert end 1}");
for(top=t; top->up!=NULL; top=top->up);

cmd(inter, "set color white");
draw_obj(inter, t, top, 10, 70, 0);
//cmd(inter, ".log.text.text insert end \"[bind $c.c]\"");
cmd(inter, "bind $c.c <1> {set choice_g 24}");
cmd(inter, "bind $c.c <2> {set choice_g 25}");
cmd(inter, "bind $c.c <3> {set choice_g 25}");
cmd(inter, "wm geometry $c +$posXLog+$posY"); 
//cmd(inter, "lower $c .");

}


/****************************************************
DRAW_OBJ

****************************************************/

void draw_obj(Tcl_Interp *inter, object *blk, object *t, int level, int center, int from)
{
int i, num, step_type, begin, x, count, num_groups;
object *cur, *cur1;
char str[80], ch[600], ch1[50];
variable *cv;
bridge *cb;

for(i=0, cur=t; cur!=NULL; cur=go_brother(cur), i++);
num=i;

begin=center-step*(num-1)/2;

x=center;
//writing to appear on the left of the window
sprintf(ch, "set list_%s \"\"", t->label);
cmd(inter, ch);
sprintf(ch, "append list_%s \"Object %s :\n\n\"",t->label, t->label);
cmd(inter,ch);

if(t->v==NULL)
 {sprintf(ch, "append list_%s \"(No Variables)\"", t->label);
  cmd(inter, ch);
 }
for(cv=t->v; cv!=NULL; cv=cv->next)
{sprintf(ch,"append list_%s \"%s", t->label,cv->label);
 if(cv->param==1)
  sprintf(str," (P)\n\"");
 else
  sprintf(str, " (%d)\n\"", cv->num_lag);
 strcat(ch, str);
  cmd(inter, ch);
}

// drawn only if it is not the root
if(t->up!=NULL)
  { // computes numerosity of groups of this typy
   sprintf(ch, "%s",t->label);
	sprintf(ch1,"");
	for(cur=t, num_groups=0; cur!=NULL ; num_groups++, cur=cur->hyper_next(cur->label) )
	 {
     if(num_groups >5)
      {strcat(ch1, ".");
       break;
      }
     cur1=skip_next_obj(cur, &count);
     sprintf(str, "%d ", count);
	  strcat(ch1, str);
	  for( ;cur->next!=NULL; cur=cur->next); // reaches the last object of this group
	 }
	put_text(inter, ch, ch1, x, level, t->label);
	put_node(inter, x-2, level+2, x+2, level+6, t->label);
	if(t->up->up!=NULL)
    put_line(inter, from, level-step_level +6, x, level+2);

  }

//for(i=0, cur=t->son; cur!=NULL; cur=skip_next_obj(cur, &num),num=0, i++);
for(i=0, cb=t->b; cb!=NULL; cb=cb->next, i++);
if(range_type>=15) //Here is changed. Placed a limit
  range_type-=15;
if(i<=1)
 begin=center+range_type/2;
else
  {step_type=range_type/(i-1);
	begin=center;
  }

if(t->up==NULL)
 level-=step_level;
//for(i=begin-range_type/2, cur=t->son; cur!=NULL; cur=skip_next_obj(cur, &num),num=0,  i+=step_type)
for(i=begin-range_type/2, cb=t->b; cb!=NULL; cb=cb->next,  i+=step_type)
 if(cb->head!=NULL)
   draw_obj(inter, blk, cb->head, level+step_level, i, center);

range_type+=15;
}

/****************************************************
PUT_NODE

****************************************************/

void put_node(Tcl_Interp *inter, int x1, int y1, int x2, int y2, char *str)
{
char ch[1000];

//Tcl_LinkVar(inter, "x1", (char *) &x1, TCL_LINK_INT);
//Tcl_LinkVar(inter, "x2", (char *) &x2, TCL_LINK_INT);
//Tcl_LinkVar(inter, "y1", (char *) &y1, TCL_LINK_INT);
//Tcl_LinkVar(inter, "y2", (char *) &y2, TCL_LINK_INT);

//sprintf(ch, "$c.c create oval $x1.m $y1.m $x2.m $y2.m -tags node -tags %s -fill $color", str);
sprintf(ch, "$c.c create oval %d.m %d.m %d.m %d.m -tags node -tags %s -fill $color",x1, y1, x2, y2, str);
cmd(inter, ch);

//Tcl_UnlinkVar(inter, "x1");
//Tcl_UnlinkVar(inter, "x2");
//Tcl_UnlinkVar(inter, "y1");
//Tcl_UnlinkVar(inter, "y2");

}

/****************************************************
PUT_LINE

****************************************************/

void put_line(Tcl_Interp *inter, int x1, int y1, int x2, int y2)
{

    char ch[1000];
//Tcl_LinkVar(inter, "x1", (char *) &x1, TCL_LINK_INT);
//Tcl_LinkVar(inter, "x2", (char *) &x2, TCL_LINK_INT);
//Tcl_LinkVar(inter, "y1", (char *) &y1, TCL_LINK_INT);
//Tcl_LinkVar(inter, "y2", (char *) &y2, TCL_LINK_INT);

// cmd(inter, "$c.c create line $x1.m $y1.m $x2.m $y2.m -tags node");
    sprintf(ch, "$c.c create line %d.m %d.m %d.m %d.m -tags node", x1, y1, x2, y2);
cmd(inter, ch);

//Tcl_UnlinkVar(inter, "x1");
//Tcl_UnlinkVar(inter, "x2");
//Tcl_UnlinkVar(inter, "y1");
//Tcl_UnlinkVar(inter, "y2");

}


/****************************************************
PUT_TEXT

****************************************************/

void put_text(Tcl_Interp *inter, char *str, char *n, int x, int y, char *str2)
{
char ch[1000];
const char *bah;

//Tcl_LinkVar(inter, "x", (char *) &x, TCL_LINK_INT);
//Tcl_LinkVar(inter, "y", (char *) &y, TCL_LINK_INT);

//text for node name
//strcpy(ch, "$c.c create text ");
//strcat(ch, "$x.m $y.m -font {{MS Times New Roman} 10} -text \"");
//strcat(ch, str);
//strcat(ch, "\" -tags node -fill red");
//strcat(ch, " -tags ");
//strcat(ch, str2);
sprintf(ch, "$c.c create text %d.m %d.m -font {{MS Times New Roman} 10} -text \"%s\" -tags node -fill red -tags %s",x,y,str,str2 );
cmd(inter, ch);

//text for node numerosity
y+=8;
//strcpy(ch, "$c.c create text ");
//strcat(ch, "$x.m $y.m -font {{MS Times New Roman} 10} -text \"");
//strcat(ch, n);
//strcat(ch, "\" -tags node -fill black");
//strcat(ch, " -tags ");
//strcat(ch, str2);

sprintf(ch, "$c.c create text %d.m %d.m -font {{MS Times New Roman} 10} -text \"%s\" -tags node -fill black -tags %s",x,y,n,str2 );

cmd(inter, ch);

/*
sprintf(ch, "$c.c bind %s <Enter> {$c.c create text 2 1 -font {{MS Times New Roman} 10} -text $%s -anchor nw -tags list}", str2, str2);
cmd(inter, ch);
sprintf(ch, "$c.c bind %s <Leave> {$c.c delete list}", str2);
cmd(inter, ch);
*/
sprintf(ch, "$c.c bind %s <Enter> {set res_g %s; if {[winfo exists .list]==1} {destroy .list} {};toplevel .list ;label .list.l -text \"$list_%s\" -justify left; pack .list.l;  align .list . }", str2, str2, str2);
cmd(inter, ch);
sprintf(ch, "$c.c bind %s <Leave> {destroy .list}", str2);
cmd(inter, ch);

sprintf(ch, "$c.c bind %s <Double-1> {set res_g %s; set choice_g 24}", str2, str2);
//cmd(inter, ch);
bah=Tcl_GetStringResult(inter);
sprintf(ch, "$c.c bind %s <Button-3> {.log.text.text insert end cazzo; wm withdraw $c; set res_g %s; set choice_g 25; }", str2, str2);
//cmd(inter, ch);
sprintf(ch, "$c.c bind %s <3> {.log.text.text insert end cazzo; wm withdraw $c; set res_g %s; set choice_g 25; }", str2, str2);
//cmd(inter, ch);
sprintf(ch, "$c.c bind %s <2> {.log.text.text insert end cazzo; wm withdraw $c; set res_g %s; set choice_g 25; }", str2, str2);
//cmd(inter, ch);

sprintf(ch, "$c.c bind %s <Button-2> {.log.text.text insert end cazzo; wm withdraw $c; set res_g %s; set choice_g 25; }", str2, str2);
//cmd(inter, ch);

sprintf(ch, "$c.c bind %s <Shift-Button-1> {set res_g %s; set choice_g 25;}", str2, str2);
//cmd(inter, ch);

//sprintf(ch, ".log.text.text insert end [$c.c bind %s <2>]; .log.text.text inser end \\n", str2);
//cmd(inter, ch);

bah=Tcl_GetStringResult(inter);

//Tcl_UnlinkVar(inter, "x");
//Tcl_UnlinkVar(inter, "y");
}


