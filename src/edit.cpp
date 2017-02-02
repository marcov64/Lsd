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

#include "decl.h"

bool in_set_obj = false;		// avoid recursive usage (confusing and tk windows are not ready)
char lab_view[MAX_ELEM_LENGTH];
char tag_view[MAX_ELEM_LENGTH];
int level;
int max_depth;


/***************************************************
SET_OBJ_NUMBER
****************************************************/

void set_obj_number(object *r, int *choice)
{

char ch[2*MAX_ELEM_LENGTH], *l;
int i, num, res, count, done;
object *cur, *first;
Tcl_LinkVar(inter, "val", (char *) &count, TCL_LINK_INT);
Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);
Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
Tcl_LinkVar(inter, "result", (char *) &res, TCL_LINK_INT);
Tcl_LinkVar(inter, "max_depth", (char *) &max_depth, TCL_LINK_INT);

cmd( "set ini .ini" );
cmd( "if { ! [ winfo exists .ini ] } { newtop .ini; showtop .ini topleftW 1 1 1 $hsizeN $vsizeN } { resizetop .ini $hsizeN $vsizeN }" );

in_set_obj = true;
strcpy(lab_view,"");
strcpy(tag_view,"");
level=1;
max_depth=1;
while(*choice==0)
{
  // reset title and destroy command because may be coming from edit_data
  cmd( "settop .ini \"%s%s - Lsd Object Number Editor\" { set choice 1; set result -1 }", unsaved_change() ? "*" : " ", simul_name );
  
  cmd( "frame .ini.obj" );
  cmd( "pack .ini.obj -fill both -expand yes" );
  cmd( "set b .ini.obj" );
  cmd( "scrollbar $b.scroll -command \"$b.list yview\"" );
  cmd( "scrollbar $b.scrollh -command \"$b.list xview\" -orient horizontal" );
  cmd( "set t $b.list" );
  cmd( "text $t -yscrollcommand \"$b.scroll set\" -xscrollcommand \"$b.scrollh set\" -wrap none -cursor arrow" );

  strcpy(ch, "");
  i=0;
  insert_obj_num( r,  ch, "", 1, &i, &count);

  cmd( "pack $b.scroll -side right -fill y" );
  cmd( "pack $b.scrollh -side bottom -fill x" );  
  cmd( "pack $b.list -fill both -expand yes" );
  cmd( "pack $b" );
  cmd( "set msg \"\"" );
  cmd( "label .ini.msglab -textvariable msg" );
  cmd( "frame .ini.f" );

  cmd( "label .ini.f.tmd -text \"Show hierarchical level: \"" );
  cmd( "entry .ini.f.emd -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set max_depth %%P; return 1 } { %%W delete 0 end; %%W insert 0 $max_depth; return 0 } } -invcmd { bell } -justify center" );
  cmd( ".ini.f.emd insert 0 $max_depth" ); 
  cmd( "button .ini.f.ud -width -9 -text Update -command {set choice 4}" );
  cmd( "pack .ini.f.tmd .ini.f.emd .ini.f.ud -side left" );

  cmd( "pack .ini.msglab .ini.f" );
  cmd( "donehelp .ini fb { set choice 1; set result -1 } { LsdHelp mdataobjn.html }" );
  
  cmd( "$t configure -state disabled" );
  cmd( "bind .ini.f.emd <Return> {set choice 4}" );

  if(strlen(lab_view)>0)
    cmd( "$t see $toview" );


noredraw:

  cmd( "write_any .ini.f.emd $max_depth" ); 

// editor command loop
while( ! *choice )
{
	try
	{
		Tcl_DoOneEvent( 0 );
	}
	catch ( std::bad_alloc& ) 	// raise memory problems
	{
		throw;
	}
	catch ( ... )				// ignore the rest
	{
		goto noredraw;
	}
}   

cmd( "set max_depth [ .ini.f.emd get ]" ); 

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

  cmd( "destroy $b .ini.msglab .ini.f .ini.fb" );

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

cmd( "unset result num choice val i" );
}


/***************************************************
INSERT_OBJ_NUM
****************************************************/
void insert_obj_num(object *root, char const *tag, char const *ind, int counter, int *i, int *value)
{
char ch[2*MAX_ELEM_LENGTH], indent[30];
object *c, *cur;
variable *var;
int num=0, multi=0;
bridge *cb; 

strcpy(ch, tag);
strcpy(indent, ind);

if(root->up!=NULL)
  strcat(indent, "  +  ");

for(cb=root->b, counter=1; cb!=NULL;cb=cb->next, counter=1)
 {  *i=*i+1;

   c=cb->head;
   for(cur=c, num=0; cur!=NULL; cur=cur->next, num++);
	 cmd( "label $t.lab$i -text \"%s %s \" -foreground red -bg white -justify left", indent, c->label );
	 cmd( "pack $t.lab$i -anchor w" );

	 if(strlen(tag)!=0)
	  cmd( "label $t.tag$i -text \" in %s %s\" -bg white", (c->up)->label , tag );
	 else
	  cmd( "label $t.tag$i -text \" \" -bg white" );

     cmd( "pack $t.tag$i -anchor w" );

	 *value=num;
	 cmd( "set count$i $val" );
	 cmd( "label $t.val$i -text $val -width 7 -relief raised" );
	 cmd( "pack $t.val$i" );
	 cmd( "$t window create end -window $t.lab$i" );
	 cmd( "$t window create end -window $t.tag$i" );
	 cmd( "$t window create end -window $t.val$i" );
    if(strlen(lab_view)>0)
     {if(!strcmp(lab_view, c->label) && !strcmp(tag_view, tag) )
       cmd( "set toview \"$t.val$i\"" );

     }
    if(level >= max_depth && c->b != NULL)
     {
     cmd( "label $t.more$i -text \"   (click here to see the descendants)\" -bg white" );
     cmd( "pack $t.more$i" );
     cmd( "$t window create end -window $t.more$i" );
     cmd( "bind $t.more$i <Button-1> {incr max_depth; set toview \"$t.val%d\"; set choice 4}", *i );
     }

    cmd( "$t insert end \\n" );
    cmd( "bind $t.val$i <Button-1> {set result %d;set num $count%d; set toview \"$t.val%d\" ; set choice 2}", *i, *i, *i );
    cmd( "bind $t.lab$i <Button-1> {set obj_name %s; set choice 3}", c->label );
   
    if ( ! in_edit_data )				// show only if not already recursing
   	cmd( "bind $t.lab$i <Enter> {set msg \"Click here to edit initial values for %s\"}", c->label );
    cmd( "bind $t.lab$i <Leave> {set msg \"\"}" );
    
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
	cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Element in Root object\" -detail \"The Root object is always single-instanced, so any element contained in it has only one instance.\"" );
	return 1;
}

cmd( "set c .compcopy" );
cmd( "newtop $c \"Instance Number\" { set cconf 1; set choice 1 }" );

cmd( "set conf 0" );
cmd( "label $c.l -text \"Determine the sequential number of the instance of '%s' \\nby setting the sequential number of the containing objects.\\nPressing 'Compute' will give the sequential number.\\npressing 'Done' will copy the number and exit.\"", c->label );
cmd( "frame $c.f -relief groove -bd 2" );
for(j=1, cur=c; cur->up!=NULL; cur=cur->up, j++) 
  {
  cmd( "frame $c.f.f%d", j );
  cmd( "label $c.f.f%d.l -text \"Instance of '%s' number: \"", j, cur->label );
  cmd( "entry $c.f.f%d.e -width 8 -textvariable num%d -justify center", j,j );
  cmd( "pack $c.f.f%d.l $c.f.f%d.e -side left", j, j );
  
  for(i=1, cur1=cur->up->search(cur->label); cur1!=cur; cur1=cur1->next, i++);

  cmd( "set num%d %d", j,i );
  }
  
cmd( "focus $c.f.f%d.e; $c.f.f%d.e selection range 0 end", j-1, j-1 );

for(j--, cur=c; cur->up!=NULL; cur=cur->up, j--)
  {//pack in inverse order
  cmd( "pack $c.f.f%d -fill both -expand yes", j );
  cmd( "bind $c.f.f%d.e <Return> {focus $c.f.f%d.e; $c.f.f%d.e selection range 0 end}", j,j-1, j-1 );
  }

cmd( "bind $c.f.f1.e <Return> \"focus $c.com\"" );

cmd( "label $c.res -fg red -text \"Instance chosen is number: %d\"", i );
res=i;
cmd( "pack $c.l $c.f $c.res -pady 5 -fill both -expand yes" );
cmd( "comphelpdone $c b { set cconf 1; set choice 2 } { LsdHelp mdataobjn.html#SelectionInstance } { set cconf 1; set choice 1 }" );

cmd( "showtop $c centerS" );

here_ccompute:
for(cur=c->up; cur->up!=NULL; cur=cur->up); //cur is root
cur=cur->search(c->label); //find the first

for(i=0, k=0; k==0 && cur!=NULL ; cur3=cur, cur=cur->hyper_next(c->label), i++)
 {
  k=1;
  for(j=1, cur1=cur; cur1->up!=NULL; cur1=cur1->up, j++)
   {
   cmd( "if [ string is integer $num%d ] { set choice $num%d } { set choice -1 }", j, j );
   if ( *choice < 0 )
	   break;
   
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

  cmd( "set num%d %d", j,i );
  }

cmd( "$c.res conf -text \"Instance chosen is num: %d\"", res ); 
   

here_cfrom:
*choice=0;
cmd( "set ccfrom 0" );
while(*choice==0)
 Tcl_DoOneEvent(0);
i=*choice;

cmd( "set choice $cconf" );
if(*choice==0)
  goto here_cfrom;
else
 *choice=i;  

if(*choice==2)
 goto here_ccompute;



cmd( "destroytop $c" );
return res;
}


void entry_new_objnum(object *c, int *choice, char const *tag)
{  
object *cur,  *first;
int cfrom, j, affect, k, pippo[1000], num;

for(num=0, cur=c->up->search(c->label);cur!=NULL; cur=go_brother(cur),num++ );

cmd( "set num %d", num );

cmd( "set n .numobj" );
cmd( "newtop $n \"Number of Objects\" { set conf 1; set choice 2 }" );
cmd( "set conf 0" );

*choice=0;
strcpy(lab_view, c->label);
strcpy(tag_view, tag);
cmd( "label $n.l -text \"Number of objects '%s' in %s %s \"", c->label, c->up->label, tag );
cmd( "entry $n.e -width 10 -validate focusout -vcmd { if [ string is integer %%P ] { set num %%P; return 1 } { %%W delete 0 end; %%W insert 0 $num; return 0 } } -invcmd { bell } -justify center" );
cmd( "$n.e insert 0 $num" ); 
cmd( "pack $n.l $n.e" );

cmd( "frame $n.ef -relief groove -bd 2" );
cmd( "label $n.ef.l -text \"Groups to be modified\"" );

for(j=1, cur=c->up; cur->up!=NULL; cur=cur->up, j++)
 {
  if(j==1)
  {
  first=cur->up->search(cur->label);
  for(k=1; first!=cur; first=go_brother(first), k++);
  cmd( "set affect%d %d",j,k );
  cmd( "checkbutton $n.ef.r%d -text \"This group of '%s' contained in '%s' # %d\" -variable affect%d -onvalue %d -offvalue -1", j,c->label,cur->label, k,j, k );
  }
  else
  {first=cur->up->search(cur->label);
  for(k=1; first!=cur; first=go_brother(first), k++);
  cmd( "set affect%d -1", j );
  cmd( "checkbutton $n.ef.r%d -text \"All groups of '%s' contained in '%s' # %d\" -variable affect%d -onvalue %d -offvalue -1 ", j, c->label ,cur->label, k, j,k );
  
  }
  
 }
  cmd( "set affect%d -1", j );       
  cmd( "checkbutton $n.ef.r%d -text \"All groups of '%s' in the model\" -variable affect%d -onvalue 1 -offvalue -1", j,c->label,j );
  if(j==1) //we are dealing with root's descendants
   cmd( "set affect1 1" );
cmd( "pack $n.ef.l" );        
for( ; j>0;  j--)
 {
 cmd( "if {[winfo exist $n.ef.r%d] == 1} {pack $n.ef.r%d  -anchor w} {}", j,j );
 }
cmd( "set affect 1" );

cmd( "frame $n.cp -relief groove -bd 2" );
cmd( "label $n.cp.l -text \"Copy from instance: \"" );
cmd( "set cfrom 1" );
cmd( "entry $n.cp.e -width 10 -validate focusout -vcmd { if [ string is integer %%P ] { set cfrom %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cfrom; return 0 } } -invcmd { bell } -justify center" );
cmd( "$n.cp.e insert 0 $cfrom" ); 
cmd( "button $n.cp.compute -width -9 -text Compute -command {set conf 1; set choice 3}" );
cmd( "pack $n.cp.l $n.cp.e $n.cp.compute -side left" );
 cmd( "$n.e selection range 0 end" );
 cmd( "bind $n.e <KeyPress-Return> {set conf 1; set choice 1}" );
 cmd( "bind $n <KeyPress-Return> {set conf 1; set choice 1}" );
 cmd( "pack $n.cp $n.ef -pady 5 -fill both -expand yes" );
 cmd( "okhelpcancel $n b { set conf 1; set choice 1 } { LsdHelp mdataobjn.html#modifyNumberObj } { set conf 1; set choice 2 }" );

 cmd( "showtop $n centerS" );
 cmd( "focus $n.e" );

here_objec_num:
 while(*choice==0)
  Tcl_DoOneEvent(0);
	  
cmd( "set num [ $n.e get ]" ); 
cmd( "set cfrom [ $n.cp.e get ]" ); 

k=*choice;

cmd( "set choice $conf" );
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
   cmd( "set cfrom %d", k );
   }
   cmd( "set conf 0" );
   *choice=0;
   goto here_objec_num;
  } 

skip_next_obj(c, &j);

		 
 if(*choice==2)
 {cmd( "destroytop $n" );
  return;
  }
cmd( "destroytop $n" );  
cmd( "set choice $cfrom" );  
cfrom=*choice;


for(cur=c->up, j=1; cur!=NULL; cur=cur->up, j++)
 {cmd( "set choice $affect%d", j );
  pippo[j]=*choice;
  if(*choice!=-1)
   affect=j;
 } 
if(affect>0)
  {
   cmd( "set choice $num" );
   chg_obj_num(&c, *choice, affect,pippo, choice, cfrom);
  } 
}
/***************************************************
EDIT_STR
****************************************************/

void edit_str(object *root, char *tag, int counter, int *i, int res, int *num, int *choice, int *done)
{
char ch[2*MAX_ELEM_LENGTH];
object *c, *cur, *first;
variable *var;
int multi=0, cazzo, param, cfrom, j, affect, k;
bridge *cb;

param=0;
strcpy(ch, tag);
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
char ch[2*MAX_ELEM_LENGTH];
int i, *del, val, last;
object *cur, *app, *prev;
bridge *cb, *first;

*choice=0;

cmd( "set d .delobj" );
cmd( "newtop $d \"Delete Objects\" { set choice 3 }" );
cmd( "set conf 0" );

cmd( "label $d.txt1 -text \"Do you want to delete the last\"" );

cmd( "label $d.txt2 -text \"%d object(s)\" -fg red", actual-desired );
cmd( "label $d.txt3 -text \"or you want to choose them?\"" );
cmd( "pack $d.txt1 $d.txt2 $d.txt3" );
cmd( "frame $d.b" );
cmd( "button $d.b.last -width -9 -text Last -command {set choice 1}" );
cmd( "button $d.b.choose -width -9 -text Choose -command {set choice 2}" );
cmd( "pack $d.b.last $d.b.choose -padx 10 -side left" );
cmd( "pack $d.b -pady 5" );
cmd( "helpcancel $d b2 { LsdHelp mdataobjn.html#pick_remove } { set choice 3 }" );

cmd( "showtop $d centerS" );

while(*choice==0)
  Tcl_DoOneEvent(0);

cmd( "destroytop $d" );
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
  cmd( "newtop $d \"Delete Objects\" { set choice 2 }" );
  cmd( "set conf 0" );

  cmd( "label $d.tit -text \"Select instances of %s to delete\"", (*r)->label );
    cmd( "entry $d.e -width 6 -validate focusout -vcmd { if [ string is integer %%P ] { set val %%P; return 1 } { %%W delete 0 end; %%W insert 0 $val; return 0 } } -invcmd { bell } -justify center" );
	 cmd( "label $d.tit1 -text \"(0 instance(s) done)\"" );
    cmd( "bind $d.e <KeyPress-Return> {set choice 1}" );
	 cmd( "pack $d.tit $d.tit1 $d.e" );
	cmd( "okhelpcancel $d b { set choice 1 } { LsdHelp mdataobjn.html#pick_remove } { set choice 2 }" );
	
	cmd( "showtop $d centerS" );
    cmd( "focus $d.e" );
    cmd( "$d.e selection range 0 end" );
  
  last=0;
  val=1;
  for(i=1; i<=actual-desired && last<=actual; i++)
   {
   do
    {
    cmd( "$d.tit1 conf -text \"([ expr $i - 1 ] instance(s) done)\"" );
	cmd( "write_any $d.e $val" ); 
    cmd( "$d.e selection range 0 end" );
	
    *choice=0;
    while(*choice==0)
     Tcl_DoOneEvent(0);

	cmd( "set val [ $d.e get ]" ); 

    if(*choice==2)
      {
       cmd( "destroytop $d" );
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
 
   cmd( "destroytop $d" );
   Tcl_UnlinkVar(inter, "val");
   Tcl_UnlinkVar(inter, "i");
   
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
 cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Object instance not found\" -detail \"Instance %d of object '%s' not found.\"", cfrom, (*c)->label );
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
