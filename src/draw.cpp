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


/****************************************************
DRAW.CPP
Draw the graphical representation of the model. It is activated by
INTERF.CPP only in case a model is loaded.

The functions contained in this file are:

- void show_graph(object *t)
initialize the canvas and calls show_obj for the root

- void draw_obj(object *blk, object *t, int level, int center, int from)
recursive function that according to the level of the object type sets the
distances among the objects. Rather rigid, but it should work nicely
for most of the model structures. It assigns all the labels (above and below the
symbols) and the writing bound to the mouse.

- void put_node(int x1, int y1, int x2, int y2, char *str)
Draw the circle.

- void put_line(int x1, int y1, int x2, int y2)
draw the line

- void put_text(char *str, char *num, int x, int y, char *str2);
Draw the different texts and sets the bindings

Functions used here from other files are:

- object *skip_next_obj(object *t, int *i);
UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- void cmd(char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

- object *go_brother(object *cur);
UTIL.CPP returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.

****************************************************/

#include "decl.h"

float hpan0lin = 0.35;			// Linux
float hpan0mac = 0.23;			// Mac
float hpan0win = 0.35;			// initial horizontal scroll % - Windows
int h0 = 255;					// initial horizontal position
int hcvsz = 1920;				// horizontal canvas size
int range_type=90;
int step=10;
int step_level=15;
int v0 = 10;					// initial vertical position
int vcvsz = 1080;				// vertical canvas size


/****************************************************
SHOW_GRAPH

****************************************************/
void show_graph( object *t)
{
object *top;

if(!strWindowOn)	// model structure window is deactivated?
{
	cmd( "if [ winfo exists .str ] { destroytop .str }" );
	return;
}

cmd( "set c .str" );
cmd( "set color white" );
for(top=t; top->up!=NULL; top=top->up);

cmd( "set strExist [ winfo exists $c ]" );
if ( ! strcmp( Tcl_GetVar( inter, "strExist", 0 ), "0" ) )		// build window only if needed
{
	cmd( "newtop $c \"\" { set strWindowOn 0; set choice 70 } \"\"" );
	cmd( "wm transient $c ." );
	cmd( "wm title $c \"%s%s - Lsd Model Structure\"", unsaved_change() ? "*" : " ", simul_name );
	cmd( "sizetop $c" );

	cmd( "frame $c.f" );
	cmd( "scrollbar $c.f.vs -command \"$c.f.c yview\"" );
	cmd( "scrollbar $c.f.hs -orient horiz -command \"$c.f.c xview\"" );
	cmd( "canvas $c.f.c -width %d -height %d -yscrollcommand \"$c.f.vs set\" -xscrollcommand \"$c.f.hs set\" -scrollregion \"0 0 %d %d\"", hcvsz, vcvsz, hcvsz, vcvsz );

	cmd( "pack $c.f.vs -side right -fill y" );
	cmd( "pack $c.f.hs -side bottom -fill x" );
	cmd( "pack $c.f.c -expand yes -fill both" );
	cmd( "pack $c.f -expand yes -fill both" );
	cmd( "set hpan0win %f; set hpan0lin %f; set hpan0mac %f", hpan0win, hpan0lin, hpan0mac );
	cmd( "mouse_wheel $c.f.c" );
    cmd( "if [ string equal $tcl_platform(platform) windows ] { \
				$c.f.c xview moveto $hpan0win \
			} { \
				if [ string equal $tcl_platform(os) Darwin ] { \
					$c.f.c xview moveto $hpan0mac \
				} { \
					$c.f.c xview moveto $hpan0lin \
				} \
			}" );

	draw_obj(t, top, v0, h0, 0);

	cmd( "bind $c.f.c <1> { if [ info exists res_g ] { set choice_g 24 } }" );
	cmd( "bind $c.f.c <2> { if [ info exists res_g ] { set res $res_g; set vname $res; set useCurrObj no; tk_popup $c.f.c.v %%X %%Y } }" );
	cmd( "bind $c.f.c <3> { if [ info exists res_g ] { set res $res_g; set vname $res; set useCurrObj no; tk_popup $c.f.c.v %%X %%Y } }" );

	cmd( "menu $c.f.c.v -tearoff 0" );
	cmd( "$c.f.c.v add command -label \"Make Current\" -command { set choice 4 }" );
	cmd( "$c.f.c.v add command -label \"Insert Parent\" -command { set choice 32 }" );
	cmd( "$c.f.c.v add separator" );
	cmd( "$c.f.c.v add command -label Change -command { set choice 6 }" );
	cmd( "$c.f.c.v add command -label \"Number of Objects\" -command { set choice 33 }" );
	cmd( "$c.f.c.v add command -label Delete -command { set choice 74 }" );
	cmd( "$c.f.c.v add separator" );
	cmd( "$c.f.c.v add cascade -label Add -menu $c.f.c.v.a" );
	cmd( "$c.f.c.v add separator" );
	cmd( "$c.f.c.v add command -label \"Initial Values\" -command { set choice 21 }" );
	cmd( "$c.f.c.v add command -label \"Browse Data\" -command { set choice 34 }" );
	cmd( "menu $c.f.c.v.a -tearoff 0" );
	cmd( "$c.f.c.v.a add command -label Variable -command { set choice 2; set param 0 }" );
	cmd( "$c.f.c.v.a add command -label Parameter -command { set choice 2; set param 1 }" );
	cmd( "$c.f.c.v.a add command -label Function -command { set choice 2; set param 2 }" );
	cmd( "$c.f.c.v.a add command -label Object -command { set choice 3 }" );

	cmd( "showtop $c current yes yes no 0 0 b" );

	set_shortcuts( "$c" );
}
else	// or just update canvas
{
	cmd( "$c.f.c delete all" );
	draw_obj(t, top, v0, h0, 0);
}
}


/****************************************************
DRAW_OBJ

****************************************************/
void draw_obj(object *blk, object *t, int level, int center, int from)
{
int i, num, step_type, begin, x, count, num_groups;
object *cur, *cur1;
char str[MAX_LINE_SIZE], ch[TCL_BUFF_STR], ch1[MAX_ELEM_LENGTH];
variable *cv;
bridge *cb;

for(i=0, cur=t; cur!=NULL; cur=go_brother(cur), i++);
num=i;

begin=center-step*(num-1)/2;

x=center;
//writing to appear on the left of the window
cmd( "set list_%s \"\"", t->label );
cmd( "append list_%s \"Object: %s\n\n\"", t->label, t->label );

if(t->v==NULL)
 cmd( "append list_%s \"(no variables)\"", t->label);

for(cv=t->v; cv!=NULL; cv=cv->next)
{sprintf(ch,"append list_%s \"%s", t->label,cv->label);
 if(cv->param==1)
  sprintf(str," (P)\n\"");
 else
  sprintf(str, " (%d)\n\"", cv->num_lag);
 strcat(ch, str);
  cmd( ch );
}

// drawn only if it is not the root
if(t->up!=NULL)
  { // computes numerosity of groups of this typy
   sprintf(ch, "%s",t->label);
	strcpy(ch1,"");
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
	put_text(ch, ch1, x, level, t->label);
	put_node(x-2, level+2, x+2, level+6, t->label);
	if(t->up->up!=NULL)
    put_line(from, level-step_level +6, x, level+2);

  }

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
for(i=begin-range_type/2, cb=t->b; cb!=NULL; cb=cb->next,  i+=step_type)
 if(cb->head!=NULL)
   draw_obj(blk, cb->head, level+step_level, i, center);

range_type+=15;
}


/****************************************************
PUT_NODE

****************************************************/
void put_node(int x1, int y1, int x2, int y2, char *str)
{
	cmd( "$c.f.c create oval %d.m %d.m %d.m %d.m -tags node -tags %s -fill $color",x1, y1, x2, y2, str );
}


/****************************************************
PUT_LINE

****************************************************/
void put_line(int x1, int y1, int x2, int y2)
{
    cmd( "$c.f.c create line %d.m %d.m %d.m %d.m -tags node", x1, y1, x2, y2 );
}


/****************************************************
PUT_TEXT

****************************************************/
void put_text(char *str, char *n, int x, int y, char *str2)
{
cmd( "$c.f.c create text %d.m %d.m -font {{MS Times New Roman} 10} -text \"%s\" -tags node -fill red -tags %s", x,y,str,str2  );

//text for node numerosity
y+=8;
cmd( "$c.f.c create text %d.m %d.m -font {{MS Times New Roman} 10} -text \"%s\" -tags node -fill black -tags %s", x,y,n,str2  );

cmd( "$c.f.c bind %s <Enter> { set res_g %s; if [winfo exists .list] { destroy .list }; toplevel .list; wm transient .list $c; wm title .list \"\"; wm protocol .list WM_DELETE_WINDOW { }; label .list.l -text \"$list_%s\" -justify left; pack .list.l; align .list $c }", str2, str2, str2 );
cmd( "$c.f.c bind %s <Leave> { if [ info exists res_g ] { unset res_g }; destroy .list}", str2 );
}
