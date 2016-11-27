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
EDIT.CPP
This functions manage the computation, display and modification of
objects' number. Any call to these module starts by scanning the whole
model tree, counts the number of each type of objects and displays orderly
the information.
On request, it is possible to change these values, either for single "branches"
of the model or for the whole set of one type of Objects.
It can exit to return to the calling function (either the browser in INTERF.CPP
or the set initial values in EDIT.CPP) or going in setting initial values.

The functions contained in this file are:

- void set_obj_number(object *r, int *choice)
The main function, called from the browser. Initialize the text widget and wait
the actions of the users to take place.

- void insert_obj_num(object *root, char *tag, char *indent, int counter, int *i, int *value);
Does the real job. Scan the model from root recursively and for each Object found
counts the number, prepare its index if the parent has multiple instances,
and set the indentation. Each label is bound to return a unique integer number
in case it is clicked. Such number is used as guide for the following function

- void edit_str(object *root, char *tag, int counter, int *i, int res, int *num, int *choice, int *done);
Explore recursively the model tree giving a unique number for every group of
objects encountered. When it finds the one clicked by user prepare the
window to accept a new value for the number of instances. Passes thie value
to the next function

- void chg_obj_num(object *c, int value, int all, int *choice);
Depending on all (the flag to modify all the values of that type in the model)
changes only the number of instances following c, or otherwise, every group of
intences of the type of c. If it has to increase the number of instances,
it does it directly. If it has to decrease, checks again all. If all is false,
it activate the routine below, otherwise, it eliminates directly the surplus

- void eliminate_obj(object *r, int actual, int desired , int *choice);
Ask the user whether he wants to eliminate the last object or to choose
individually the ones to eliminate. In this second case, it asks for a list
numbers. The list is as long as are the instances to eliminate. Each element
is the ordinal number of one instance to eliminate


void search_title(object *root, char *tag, int *i, char *lab, int *incr);
void clean_cell(object *root, char *tag, char *lab);
void edit_data(object *root, int *choice, char *obj_name);
void set_title(object *c, char *lab, char *tag, int *incr);
void link_data(object *root, char *lab);

************************************/



#include <tk.h>
#include "decl.h"

void go_next(object **t);
object *go_brother(object *cur);
object *skip_next_obj(object *t, int *count);
void cmd(Tcl_Interp *inter, char const *cc);
void insert_obj_num(object *root, char const *tag, char const *indent, int counter, int *i, int *value);
void edit_str(object *root, char *tag, int counter, int *i, int res, int *num, int *choice, int *done);
void eliminate_obj(object **r, int actual, int desired , int *choice);
void search_title(object *root, char *tag, int *i, char *lab, int *incr);
void clean_cell(object *root, char *tag, char *lab);
void edit_data(object *root, int *choice, char *obj_name);
void set_title(object *c, char *lab, char *tag, int *incr);
void link_data(object *root, char *lab);
void chg_obj_num(object **c, int value, int all, int pippo[], int *choice, int cfrom);
void plog( char const *msg, char const *tag = "" );
bool unsaved_change(  );		// control for unsaved changes in configuration
bool unsaved_change( bool );

extern Tcl_Interp *inter;
extern char msg[];
extern char *simul_name;	// simulation name to use in title bar
extern bool in_edit_data;
extern char widthDE[];			// horizontal size in pixels
extern char heightDE[];			// vertical size in pixels

char lab_view[40];
char tag_view[40];
int level;
int max_depth;
// flags to avoid recursive usage (confusing and tk windows are not ready)
bool in_set_obj = false;
// Main window constraints (defined in edit_data.cpp)

/***************************************************
SET_OBJ_NUMBER
****************************************************/

void set_obj_number(object *r, int *choice)
{

char ch[80], *l;
int i, num, res, count, done;
object *cur, *first;
Tcl_LinkVar(inter, "val", (char *) &count, TCL_LINK_INT);
Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);
Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
Tcl_LinkVar(inter, "result", (char *) &res, TCL_LINK_INT);
Tcl_LinkVar(inter, "max_depth", (char *) &max_depth, TCL_LINK_INT);
Tcl_SetVar(inter, "widthDE", widthDE, 0);		// horizontal size in pixels
Tcl_SetVar(inter, "heightDE", heightDE, 0);		// vertical minimum size in pixels

cmd( inter, "set ini .ini" );
cmd( inter, "if { ! [ winfo exists .ini ] } { newtop .ini; showtop .ini centerS 1 1 1 $widthDE $heightDE }" );

in_set_obj = true;
strcpy(lab_view,"");
strcpy(tag_view,"");
level=1;
max_depth=1;
while(*choice==0)
{
  // reset title and destroy command because may be coming from edit_data
  sprintf( ch, "settop .ini \"%s%s - Lsd Object Number Editor\" { set choice 1; set result -1 }", unsaved_change() ? "*" : " ", simul_name );
  cmd( inter, ch );
  
  cmd(inter, "frame .ini.obj");
  cmd(inter, "pack .ini.obj -fill both -expand yes");
  cmd(inter, "set b .ini.obj");
  cmd(inter, "scrollbar $b.scroll -command \"$b.list yview\"");
  cmd(inter, "scrollbar $b.scrollh -command \"$b.list xview\" -orient horizontal");
  cmd(inter, "set t $b.list");
  cmd(inter, "text $t -yscrollcommand \"$b.scroll set\" -xscrollcommand \"$b.scrollh set\" -wrap none -cursor arrow");

   strcpy(ch, "");
  i=0;
  insert_obj_num( r,  ch, "",1, &i, &count);

  cmd(inter, "pack $b.scroll -side right -fill y");
  cmd(inter, "pack $b.scrollh -side bottom -fill x");  
  cmd(inter, "pack $b.list -fill both -expand yes");
  cmd(inter, "pack $b");
  cmd(inter, "set msg \"\"");
  cmd(inter, "label .ini.msglab -textvariable msg");
  cmd(inter, "frame .ini.f");

  cmd(inter, "label .ini.f.tmd -text \"Show hierarchical level: \"");
  cmd(inter, "entry .ini.f.emd -textvariable max_depth -width 5");
  cmd(inter, "button .ini.f.ud -width -9 -text Update -command {set choice 4}");
  cmd(inter, "pack .ini.f.tmd .ini.f.emd .ini.f.ud -side left");

  cmd(inter, "pack .ini.msglab .ini.f");
  cmd( inter, "donehelp .ini fb { set choice 1; set result -1 } { LsdHelp mdataobjn.html }" );
  
  cmd(inter, "$t configure -state disabled");
  cmd(inter, "bind .ini.f.emd <Return> {set choice 4}");

  if(strlen(lab_view)>0)
    cmd(inter, "$t see $toview");

noredraw:
  while(*choice==0)
   {
    try{Tcl_DoOneEvent(0);}
    catch(...) {
    goto noredraw;
    }
   } 

if ( *choice == 3 && in_edit_data )		// avoid recursion
{
	*choice = 0;
	goto noredraw;
}

if(*choice==2)
 {
 i=0;
 done=0;
 edit_str(r, ch, 1, &i, res, &num, choice, &done);
 *choice=2;
 }

  cmd(inter, "destroy $b .ini.msglab .ini.f .ini.fb");

  strcpy(ch, "");
  i=0;
  done=0;
  switch(*choice)
	{
	case 1: break;
	case 2: 
			  *choice=0;
			  break;
	case 3: l=(char *)Tcl_GetVar(inter, "obj_name",0);
			  strcpy(ch, l);
			  edit_data(r, choice,ch);
			  *choice=0;
			  break;
   case 4: *choice=0;
           break;
	default: plog("\nChoice not recognized");
           *choice=0;
           break;
	}

}

in_set_obj = false;

Tcl_UnlinkVar(inter, "i");
Tcl_UnlinkVar(inter, "val");
Tcl_UnlinkVar(inter, "result");
Tcl_UnlinkVar(inter, "num");
Tcl_UnlinkVar(inter, "max_depth");

cmd(inter, "unset result num choice val i");
}


/***************************************************
INSERT_OBJ_NUM
****************************************************/
void insert_obj_num(object *root, char const *tag, char const *ind, int counter, int *i, int *value)
{
char ch[620], ch1[120], indent[30];
object *c, *cur;
variable *var;
int num=0, multi=0;
bridge *cb; 

strcpy(ch, tag);
strcpy(indent, ind);

if(root->up!=NULL)
  strcat(indent, "  +  ");

//for(c=root->son, counter=1; c!=NULL;c=skip_next_obj(c, &num), counter=1)
for(cb=root->b, counter=1; cb!=NULL;cb=cb->next, counter=1)
 {  *i=*i+1;

	 //skip_next_obj(c, &num);
   c=cb->head;
   for(cur=c, num=0; cur!=NULL; cur=cur->next, num++);
	 sprintf(ch, "label $t.lab$i -text \"%s %s \" -foreground red -bg white -justify left", indent, c->label);
	 cmd(inter, ch);
	 cmd(inter, "pack $t.lab$i -anchor w");

	 if(strlen(tag)!=0)
	  sprintf(ch, "label $t.tag$i -text \" in %s %s\" -bg white", (c->up)->label , tag);
	 else
	  sprintf(ch, "label $t.tag$i -text \" \" -bg white");
	 cmd(inter, ch);
	 cmd(inter, "pack $t.tag$i -anchor w");

	 *value=num;
	 cmd(inter, "set count$i $val");
	 cmd(inter, "label $t.val$i -text $val -width 7 -relief raised");
	 cmd(inter, "pack $t.val$i");
	 cmd(inter, "$t window create end -window $t.lab$i");
	 cmd(inter, "$t window create end -window $t.tag$i");
	 cmd(inter, "$t window create end -window $t.val$i");
    if(strlen(lab_view)>0)
     {if(!strcmp(lab_view, c->label) && !strcmp(tag_view, tag) )
       cmd(inter, "set toview \"$t.val$i\"");

     }
    if(level >= max_depth && c->b != NULL)
     {
     sprintf(ch, "label $t.more$i -text \"   (click here to see the descendants)\" -bg white");
     cmd(inter, ch);
     cmd(inter, "pack $t.more$i");
     cmd(inter, "$t window create end -window $t.more$i");
     sprintf(ch, "bind $t.more$i <Button-1> {incr max_depth; set toview \"$t.val%d\"; set choice 4}", *i);
     cmd(inter, ch);
     }


  	 cmd(inter, "$t insert end \\n");

	 sprintf(ch,"bind $t.val$i <Button-1> {set result %d;set num $count%d; set toview \"$t.val%d\" ; set choice 2}", *i, *i, *i);
	 cmd(inter, ch);

	 sprintf(ch,"bind $t.lab$i <Button-1> {set obj_name %s; set choice 3}", c->label);
	 cmd(inter, ch);
	 if ( ! in_edit_data )				// show only if not already recursing
	 {
		sprintf(ch,"bind $t.lab$i <Enter> {set msg \"Click here to edit initial values for %s\"}", c->label);
		cmd(inter, ch);
	 }
	 cmd(inter, "bind $t.lab$i <Leave> {set msg \"\"}");
   Tcl_DoOneEvent(0);

 if( go_brother(c)!=NULL)
	 multi=1;
  else
    multi=0;
  if(level<max_depth)
  {
  for(cur=c; cur!=NULL; counter++, cur=go_brother(cur))
   {
      if(strlen(tag)!=0)
        sprintf(ch, "%d - %s %s",counter, (c->up)->label,  tag );
      else
        sprintf(ch, "%d",counter);
   level++;
   insert_obj_num(cur, ch,indent, counter, i, value);
   level--;
   }
  }

  }

}

int compute_copyfrom(object *c, int *choice)
{

object *cur, *cur1, *cur2, *cur3;
int i,j, k,h, res;

if ( c->up == NULL )
{
	cmd( inter, "tk_messageBox -type ok -icon warning -title Warning -message \"Element contained in Root object.\\n\\n The Root object is always single-instanced, so any element contained in it has only one instance.\"" );
	return 1;
}

cmd( inter, "set c .compcopy" );
cmd( inter, "newtop $c \"Instance Number\" { set cconf 1; set choice 1 }" );

cmd(inter, "set conf 0");
sprintf(msg, "label $c.l -text \"Determine the sequential number of the instance of '%s' \\nby setting the sequential number of the containing objects.\\nPressing 'Compute' will give the sequential number.\\npressing 'Done' will copy the number and exit.\"", c->label);
cmd(inter, msg);
cmd(inter, "frame $c.f -relief groove -bd 2");
for(j=1, cur=c; cur->up!=NULL; cur=cur->up, j++) 
  {
  sprintf(msg, "frame $c.f.f%d", j);
  cmd(inter, msg);
  sprintf(msg, "label $c.f.f%d.l -text \"Instance of '%s' number: \"", j, cur->label);
  cmd(inter, msg);
  sprintf(msg, "entry $c.f.f%d.e -width 8 -textvariable num%d", j,j);
  cmd(inter, msg);
  sprintf(msg, "pack $c.f.f%d.l $c.f.f%d.e -side left", j, j);
  cmd(inter, msg);
  for(i=1, cur1=cur->up->search(cur->label); cur1!=cur; cur1=cur1->next, i++);

  sprintf(msg, "set num%d %d", j,i);
  cmd(inter, msg); 
  }
  
sprintf(msg, "focus -force $c.f.f%d.e; $c.f.f%d.e selection range 0 end", j-1, j-1);
cmd(inter, msg);

for(j--, cur=c; cur->up!=NULL; cur=cur->up, j--)
  {//pack in inverse order
  sprintf(msg, "pack $c.f.f%d -fill both -expand yes", j);
  cmd(inter, msg);
  sprintf(msg, "bind $c.f.f%d.e <Return> {focus -force $c.f.f%d.e; $c.f.f%d.e selection range 0 end}", j,j-1, j-1);
  cmd(inter, msg);
  }

  sprintf(msg, "bind $c.f.f1.e <Return> \"focus -force $c.com\"");
  cmd(inter, msg);

sprintf(msg, "label $c.res -fg red -text \"Instance chosen is number: %d\"",i);
cmd(inter, msg);
res=i;
cmd(inter, "pack $c.l $c.f $c.res -pady 5 -fill both -expand yes");
cmd( inter, "comphelpdone $c b { set cconf 1; set choice 2 } { LsdHelp mdataobjn.html#SelectionInstance } { set cconf 1; set choice 1 }" );

cmd( inter, "showtop $c centerS" );

here_ccompute:
for(cur=c->up; cur->up!=NULL; cur=cur->up); //cur is root
cur=cur->search(c->label); //find the first

for(i=0, k=0; k==0 && cur!=NULL ; cur3=cur, cur=cur->hyper_next(c->label), i++)
 {
  k=1;
  for(j=1, cur1=cur; cur1->up!=NULL; cur1=cur1->up, j++)
   {
   sprintf(msg, "set choice $num%d", j);
   cmd(inter, msg);
   for(h=1, cur2=cur1->up->search(cur1->label); cur2!=cur1; cur2=cur2->next, h++);   
   if(cur2->next==NULL && *choice>h)
     *choice=h;
   if(h<*choice)
    {k=0;
     break;
    }
   }
 }

res=i;
//reset possibly erroneous values
for(j=1, cur2=cur3; cur2->up!=NULL; cur2=cur2->up, j++) 
  {
  for(i=1, cur1=cur2->up->search(cur2->label); cur1!=cur2; cur1=cur1->next, i++);

  sprintf(msg, "set num%d %d", j,i);
  cmd(inter, msg); 
  }

sprintf(msg, "$c.res conf -text \"Instance chosen is num: %d\"",res);
cmd(inter, msg); 
   

here_cfrom:
*choice=0;
cmd(inter, "set ccfrom 0");
while(*choice==0)
 Tcl_DoOneEvent(0);
i=*choice;

cmd(inter, "set choice $cconf");
if(*choice==0)
  goto here_cfrom;
else
 *choice=i;  

if(*choice==2)
 goto here_ccompute;



cmd(inter, "destroytop $c");
return res;
}


void entry_new_objnum(object *c, int *choice, char const *tag)
{  
object *cur,  *first;
int cfrom, j, affect, k, pippo[100], num;

for(num=0, cur=c->up->search(c->label);cur!=NULL; cur=go_brother(cur),num++ );

sprintf(msg, "set num %d",num);
cmd(inter, msg);

cmd( inter, "set n .numobj" );
cmd( inter, "newtop $n \"Number of Objects\" { set conf 1; set choice 2 }" );
cmd(inter, "set conf 0");

*choice=0;
strcpy(lab_view, c->label);
strcpy(tag_view, tag);
sprintf(msg, "label $n.l -text \"Number of objects '%s' in %s %s \"", c->label, c->up->label, tag);
cmd(inter, msg);
cmd(inter, "entry $n.e -textvariable num -width 10");
cmd( inter, "pack $n.l $n.e" );

cmd(inter, "frame $n.ef -relief groove -bd 2");
cmd(inter, "label $n.ef.l -text \"Groups to be modified\"");

//      sprintf(msg, "radiobutton $n.ef.r0 -text \"Only this group of %s\" -variable affect -value 0 ", c->label);
//      cmd(inter, msg);
for(j=1, cur=c->up; cur->up!=NULL; cur=cur->up, j++)
 {
  if(j==1)
  {
  first=cur->up->search(cur->label);
  for(k=1; first!=cur; first=go_brother(first), k++);
  sprintf(msg, "set affect%d %d",j,k);
  cmd(inter, msg);        
  sprintf(msg, "checkbutton $n.ef.r%d -text \"This group of '%s' contained in '%s' # %d\" -variable affect%d -onvalue %d -offvalue -1", j,c->label,cur->label, k,j, k);
  cmd(inter, msg);
  }
  else
  {first=cur->up->search(cur->label);
  for(k=1; first!=cur; first=go_brother(first), k++);
  sprintf(msg, "set affect%d -1",j);
  cmd(inter, msg);
  sprintf(msg, "checkbutton $n.ef.r%d -text \"All groups of '%s' contained in '%s' # %d\" -variable affect%d -onvalue %d -offvalue -1 ", j, c->label ,cur->label, k, j,k);
  cmd(inter, msg);
  
  }
  
 }
  sprintf(msg, "set affect%d -1",j);
  cmd(inter, msg);       
  sprintf(msg, "checkbutton $n.ef.r%d -text \"All groups of '%s' in the model\" -variable affect%d -onvalue 1 -offvalue -1", j,c->label,j);
  cmd(inter, msg);
  if(j==1) //we are dealing with root's descendants
   cmd(inter, "set affect1 1");
cmd(inter, "pack $n.ef.l");        
//      cmd(inter, "pack $n.ef.r0 -anchor w");
//for(j=1, cur=c->up; cur!=NULL; cur=cur->up, j++)
for( ; j>0;  j--)
 {
 sprintf(msg, "if {[winfo exist $n.ef.r%d] == 1} {pack $n.ef.r%d  -anchor w} {}",j,j);
 cmd(inter, msg);
 }
cmd(inter, "set affect 1");

cmd(inter, "frame $n.cp -relief groove -bd 2");
cmd(inter, "label $n.cp.l -text \"Copy from instance: \"");
cmd(inter, "set cfrom 1");
cmd(inter, "entry $n.cp.e -textvariable cfrom -width 10");
cmd(inter, "button $n.cp.compute -width -9 -text Compute -command {set conf 1; set choice 3}");
cmd(inter, "pack $n.cp.l $n.cp.e $n.cp.compute -side left");
 cmd(inter, "$n.e selection range 0 end");
 cmd(inter, "bind $n.e <KeyPress-Return> {set conf 1; set choice 1}");
 cmd(inter, "bind $n <KeyPress-Return> {set conf 1; set choice 1}");
 cmd(inter, "pack $n.cp $n.ef -pady 5 -fill both -expand yes");
 cmd( inter, "okhelpcancel $n b { set conf 1; set choice 1 } { LsdHelp mdataobjn.html#modifyNumberObj } { set conf 1; set choice 2 }" );
 cmd(inter, "focus $n.e");

 cmd( inter, "showtop $n centerS" );

here_objec_num:
		 while(*choice==0)
		  Tcl_DoOneEvent(0);
k=*choice;

cmd(inter, "set choice $conf");
if(*choice==0)
  goto here_objec_num;
else
 *choice=k;  
 if(*choice==3)
  {
   *choice=0;
   k=compute_copyfrom(c, choice);
   if(k>0)
   {
   sprintf(msg, "set cfrom %d",k);
   cmd(inter, msg);
   }
   cmd(inter, "set conf 0");
   *choice=0;
   goto here_objec_num;
  } 

skip_next_obj(c, &j);

		 
 if(*choice==2)
 {cmd(inter, "destroytop $n");
  return;
  }
cmd(inter, "destroytop $n");  
cmd(inter, "set choice $cfrom");  
cfrom=*choice;


for(cur=c->up, j=1; cur!=NULL; cur=cur->up, j++)
 {sprintf(msg, "set choice $affect%d", j);
  cmd(inter, msg);
  pippo[j]=*choice;
  if(*choice!=-1)
   affect=j;
 } 
if(affect>0)
  {
   cmd(inter, "set choice $num");
   chg_obj_num(&c, *choice, affect,pippo, choice, cfrom);
  } 
}
/***************************************************
EDIT_STR
****************************************************/

void edit_str(object *root, char *tag, int counter, int *i, int res, int *num, int *choice, int *done)
{
char ch[120];
object *c, *cur, *first;
variable *var;
int multi=0, cazzo, param, cfrom, j, affect, k, pippo[100];
bridge *cb;

param=0;
strcpy(ch, tag);
//for(c=root->son, counter=1; c!=NULL && *done==0;c=skip_next_obj(c, &cazzo), counter=1)
for(cb=root->b, counter=1; cb!=NULL && *done==0;cb=cb->next, counter=1)
 { c=cb->head; 
   *i=*i+1;
	 if(*i==res)
	 {
    entry_new_objnum(c, choice, tag);
    *done=1;
    }

	 if( go_brother(c)!=NULL)
	 multi=1;
  else
	 multi=0;
  for(cur=c; cur!=NULL && *done==0; counter++, cur=go_brother(cur))
	{
	if(multi==1 || multi==0)
		if(strlen(tag)!=0)
		  sprintf(ch, "%s-%d",tag, counter);
		else
		  sprintf(ch, "%d",counter);
	else
		sprintf(ch, "%s",tag);
   if(level < max_depth)
   {level++;
	edit_str(cur, ch, counter, i, res, num, choice, done);
    level--;
   } 

	}
  }

}

/***************************************************
ELIMINATE_OBJ
****************************************************/

void eliminate_obj(object **r, int actual, int desired , int *choice)
{
char ch[120];
int i, *del, val, last;
object *cur, *app, *prev;
bridge *cb, *first;

*choice=0;

cmd( inter, "set d .delobj" );
cmd( inter, "newtop $d \"Delete Objects\" { set choice 3 }" );
cmd(inter, "set conf 0");


sprintf(ch, "label $d.txt1 -text \"Do you want to delete the last\"");
cmd(inter, ch);

sprintf(ch, "label $d.txt2 -text \"%d object(s)\" -fg red", actual-desired);
cmd(inter, ch);
sprintf(ch, "label $d.txt3 -text \"or you want to choose them?\"");
cmd(inter, ch);
cmd(inter, "pack $d.txt1 $d.txt2 $d.txt3");
cmd(inter, "frame $d.b");
cmd(inter, "button $d.b.last -width -9 -text Last -command {set choice 1}");
cmd(inter, "button $d.b.choose -width -9 -text Choose -command {set choice 2}");
cmd(inter, "pack $d.b.last $d.b.choose -padx 10 -side left");
cmd(inter, "pack $d.b -pady 5");
cmd( inter, "helpcancel $d b2 { LsdHelp mdataobjn.html#pick_remove } { set choice 3 }" );
cmd( inter, "showtop $d centerS" );

while(*choice==0)
  Tcl_DoOneEvent(0);

cmd(inter, "destroytop $d");
if( *choice==3)
 return;

if(*choice==1)
 {
 for(i=1, cur=*r; i<desired; i++,cur=cur->next);
 for( ; go_brother(cur)!=NULL; )
	{ app=cur->next->next;
	  cur->next->empty();
	  delete cur->next;
	  cur->next=app;
	}
 }
else
{ del=new int[actual-desired];
  Tcl_LinkVar(inter, "val", (char *) &val, TCL_LINK_INT);
  Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);
  cmd( inter, "newtop $d \"Delete Objects\" { set choice 2 }" );
  cmd(inter, "set conf 0");

  sprintf(ch, "label $d.tit -text \"Select instances of %s to delete\"", (*r)->label);
  cmd(inter, ch);
    cmd(inter, "entry $d.e -width 6 -textvariable val");
	 cmd(inter, "label $d.tit1 -text \"(0 instance(s) done)\"");
    cmd(inter, "bind $d.e <KeyPress-Return> {set choice 1}");
	 cmd(inter, "pack $d.tit $d.tit1 $d.e");
	cmd( inter, "okhelpcancel $d b { set choice 1 } { LsdHelp mdataobjn.html#pick_remove } { set choice 2 }" );
	cmd( inter, "showtop $d centerS" );
    cmd(inter, "focus $d.e");
    cmd(inter, "$d.e selection range 0 end");
  
  last=0;
  val=1;
  for(i=1; i<=actual-desired && last<=actual; i++)
   {
   do
    {
    *choice=0;
    cmd(inter, "$d.tit1 conf -text \"([ expr $i - 1 ] instance(s) done)\"");
    cmd(inter, "$d.e conf -textvariable val");
    cmd(inter, "$d.e selection range 0 end");
    while(*choice==0)
     Tcl_DoOneEvent(0);

    if(*choice==2)
      {
       cmd(inter, "destroytop $d");
       Tcl_UnlinkVar(inter, "val");
       Tcl_UnlinkVar(inter, "i");
       *choice=0;
       return; 
      }  
    *choice=0;  

    del[i-1]=val;
    }
	 while(last>=val);
    last=val;
    val++;
	}
 
   cmd(inter, "destroytop $d");
   Tcl_UnlinkVar(inter, "val");
   Tcl_UnlinkVar(inter, "i");
   
   
   /***
   if( ((*r)->up)->son==(*r))
		prev=(*r)->up;
   else 
    for(prev=((*r)->up)->son; strcmp( (prev->next)->label, (*r)->label); prev=prev->next);
   *****/  
   for(cb=(*r)->up->b; cb!=NULL; cb=cb->next)
    if(!strcmp(cb->blabel,(*r)->label) )
     break;
   //here cb is the bridge containing the obj's to remove  
   
   for(i=1, val=0,prev=NULL, cur=cb->head;cur!=NULL && i<=actual && val<actual-desired; i++)
    { 
   
     if(i==del[val])
      {
       if(cb->head==cur)
        *r=cb->head=cur->next;
       app=cur->next;
       cur->empty();
 		   delete cur;
       if(prev!=NULL)
        prev->next=app;
       prev=cur=app;
       val++;
        
       } 
     else
      {
       prev=cur;
       cur=cur->next;
      } 
    }    
    
  }//choice==2
}

int check_pippo(object *c, object *pivot, int level, int pippo[])
{
int res=1, i, j;
object *cur, *cur1;

for(cur=c->up, i=1; i<=level && res==1; i++, cur=cur->up)
 {
 if(pippo[i]!=-1 && cur->up!=NULL)//don't check if it is in Root or if there is no constraint
  {
  cur1=cur->up->search(cur->label);
  for(j=1; cur1!=cur; cur1=cur1->next,j++);//find the id of cur
  if(j!=pippo[i])
   res=0;
  }
 }
return res;
}

void chg_obj_num(object **c, int value, int level, int pippo[], int *choice, int cfrom)
{
object *cur, *cur1, *last, *app, *first, *pivot;
int cazzo, i;

for(cur=*c; cur->up!=NULL; cur=cur->up); //goto root
//select the object example
for(first=cur->search((*c)->label), i=1; i<cfrom && first!=NULL; first=first->hyper_next(first->label), i++);
if(first==NULL)
 {
 sprintf(msg, "set mess \"Error: instance %d of object %s not found.\"", cfrom, (*c)->label);
 cmd(inter, "tk_message -type ok -icon error -message \"$mess\"");
 return;
 }
 
//select the pivot 
for(i=0,pivot=*c; i<level; i++)
  pivot=pivot->up;
//select the first object of the type to change under pivot
cur=pivot->search((*c)->label);
  
for( ;cur!=NULL; )
  {//as long as necessary
	if(pippo==NULL || check_pippo(cur, pivot, level, pippo)==1)
   {
   skip_next_obj(cur,&cazzo); //count the existing objects
	if(cazzo<=value)             
	  {//add objects
      cur->up->add_n_objects2(first->label,value-cazzo,first); //add the necessary num of objects
     }
	else
	  { //remove objects
      if(level==1) //you have the option to choose the items to be removed, only if you operate on one group
	     {eliminate_obj(&cur, cazzo, value, choice);
        *c=cur;
       } 
      else
      {// remove automatically the excess of objects
       
		 for(i=1, cur1=cur; i<value; i++,cur1=cur1->next);
		 for( ; go_brother(cur1)!=NULL; )
		 { app=cur1->next->next;
		  cur1->next->empty();
		  delete cur1->next;
		  cur1->next=app;
		 }
      }
	  }
    }//end check_pippo
    for(cur1=cur;cur1!=NULL; cur1=go_brother(cur1) )
	   last=cur1 ; //skip the just updated group of objects
	  cur=last->hyper_next(cur->label); //first copy of the object to change after the just adjusted bunch

    if(level>0 && cur!=NULL)
    {//search the next pivot
    for(cur1=cur->up, i=1; i<level; i++, cur1=cur1->up); //hopefully, cur1 is of type pivot
    if(cur1!=pivot)//a new set of objects to change has been found, but descends from another pivot
     cur=NULL;
	  }
    else
     cur=NULL;

 }
}



