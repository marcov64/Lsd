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
DEBUG.CPP
Builds and manages the debug window. This window appears under two conditions:
- Simulation running in debug mode AND an equation for one of the variables
to be debugged has just been computed, or
- One conditional stop is met, whatever type of running mode is enabled
Moreover, it can be used to explore thoughrouly a model by choosing
the option Data Browse from the main Browser.
When the simulation is stopped by the debugger,  shows all the contents of the
objects, that is, it lists the Variables and Parameters of the object, their
value and their time of last updating, for Variables. User are then allowed
to set or remove conditional break and debug flags on any variable in the model.
Note that the browsing mode in the debugger is different from the main Browser,
since in the debugger you move along the physical model, hence you have
to browse through all the instances, instead of moving along object types.



The functions contained in this file are:

- int deb(object *r, object *c, char *lab, double *res)
initialize the debugging window and calls deb_show below. Then it waits for a
command from user. The available actions are

1) make a step. That is, continue till next stop, if any
2) disable debug mode and continue to run the simulation
3) observe the parent object of the current one
4) observe the object next to the current one
5) observe next type of object
6) observe the first descendant
7) stop the simulation and return to the browser
8) observe the variable detailed content (binding to clicking on the variable
   label).
9) observe the object from which this equation was triggered, if any.
10) Search for an Object containing a specific Variable with a specific value

- void deb_show(object *r)
fill in all the content of the object.

Functions used here from other files are:

- void show_eq(char *lab, int *choice);
SHOW_EQ.CPP shows one equation for variable lab

- object *skip_next_obj(object *t, int *i);
UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- object *search_var_cond(char *lab, double value, int lag);
Uses search_var, but returns the instance of the object that has the searched
variable with the desired value equal to value.

- void cmd(char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.


****************************************************/

#include "decl.h"

bool invalidHooks = false;		// flag to invalid hooks pointers (set by simulation)
double i_values[1000];
int interact_flag=0;
lsdstack *asl=NULL;


/*******************************************
DEB
********************************************/
int deb(object *r, object *c,  char const *lab, double *res)
{
const char *bah;
variable *cv, *cv1;
char ch[4*MAX_ELEM_LENGTH], *ch1;
object *cur, *cur1, *cur2;
int count, old, i, cond;
bool pre_running;
double value_search;
double *app_values;
double app_res;
bridge *cb, *cb1;

// define the presentation mode ( 1 = normal debug, 2 = data browse, 3 = pause debug )
int mode = ( lab == NULL ) ? 2 : ( ! strcmp( lab, "Paused by User" ) ) ? 3 : 1; 
Tcl_SetVar( inter, "lab", lab, 0 );

cmd( "set deb .deb" );
cmd( "if { ! [ winfo exists .deb ] } { if [ string equal $lab \"\" ] { set debTitle \"Lsd Data Browser\" } { set debTitle \"Lsd Debugger\" }; newtop .deb \"%s%s - $debTitle\" { set choice 7 } \"\"; set justCreated true }", unsaved_change() ? "*" : " ", simul_name  );

// avoid redrawing the menu if it already exists and is configured
cmd( "set existMenu [ winfo exists .deb.m ]" );
cmd( "set confMenu [ .deb cget -menu ]" );
if ( ! strcmp( Tcl_GetVar( inter, "existMenu", 0 ), "0" ) ||
	 strcmp( Tcl_GetVar( inter, "confMenu", 0 ), ".deb.m" ) )
{
	cmd( "destroy .deb.m" );
	cmd( "menu .deb.m -tearoff 0 -relief groove -bd 2" );
	cmd( "set w .deb.m.exit" );
	cmd( ".deb.m add cascade -label Exit -menu $w -underline 0" );
	cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
	if ( mode == 3 )
		cmd( "$w add command -label \"Quit and resume simulation\" -command { set choice 7 } -underline 0 -accelerator Esc" );
	else
		cmd( "$w add command -label \"Quit and return to Browser\" -command { set choice 7 } -underline 0 -accelerator Esc" );
	cmd( "set w .deb.m.help" );
	cmd( "menu $w -tearoff 0 -relief groove -bd 2" );
	cmd( ".deb.m add cascade -label Help -menu $w -underline 0" );
	cmd( "$w add command -label \"Help on Lsd Debugger\" -command {LsdHelp debug.html} -underline 0" );
	cmd( "$w add separator" );
	cmd( "$w add command -label \"Model Report\" -command {set choice 44} -underline 0" );
	cmd( "$w add separator" );
	cmd( "$w add command -label \"About Lsd...\" -command { tk_messageBox -parent .deb -type ok -icon info -title \"About Lsd\" -message \"Version %s (%s)\" -detail \"Platform: [ string totitle $tcl_platform(platform) ] ($tcl_platform(machine))\nOS: $tcl_platform(os) ($tcl_platform(osVersion))\nTcl/Tk: [ info patch ]\" } -underline 0", _LSD_VERSION_, _LSD_DATE_ ); 
	cmd( ".deb configure -menu .deb.m" );
}

// avoid redrawing the buttons if they already exist
cmd( "set existButtons [ expr [ winfo exists .deb.b ] ]" );
if ( ! strcmp( Tcl_GetVar( inter, "existButtons", 0 ), "0" ) )
{ 
	cmd( "destroy .deb.b" );

	cmd( "frame .deb.b -border 6" );
	cmd( "frame .deb.b.move" );
	cmd( "frame .deb.b.act" );

	cmd( "button .deb.b.move.up -width -9 -text \"Up\" -command {set choice 3} -underline 0" );
	cmd( "button .deb.b.move.down -width -9 -text \"Down\" -command {set choice 6} -underline 0" );
	cmd( "button .deb.b.move.prev -width -9 -text \"Prev.\" -command {set choice 12} -underline 0" );
	cmd( "button .deb.b.move.broth -width -9 -text \"Next\" -command {set choice 4} -underline 0" );
	cmd( "button .deb.b.move.hypern -width -9 -text \"Next Type\" -command {set choice 5} -underline 5" );
	cmd( "button .deb.b.move.last -width -9 -text \"Last\" -command {set choice 14} -underline 0" );
	cmd( "button .deb.b.move.search -width -9 -text \"Find\" -command {set choice 10} -underline 0" );
	
	if ( mode == 3 )
	{
		cmd( "button .deb.b.move.run -width -9 -text Resume -command {set choice 2} -underline 0" );
		cmd( "button .deb.b.move.an -width -9 -text Analysis -command {set choice 11} -underline 0" );
		cmd( "pack .deb.b.move.up .deb.b.move.down .deb.b.move.prev .deb.b.move.broth .deb.b.move.hypern .deb.b.move.last .deb.b.move.search .deb.b.move.an .deb.b.move.run -padx 8 -pady 10 -side left -expand no -fill none" );

		cmd( "bind .deb <KeyPress-r> {.deb.b.move.run invoke}; bind .deb <KeyPress-R> {.deb.b.move.run invoke}" );
		cmd( "bind .deb <KeyPress-a> {.deb.b.move.an invoke}; bind .deb <KeyPress-A> {.deb.b.move.an invoke}" );
	}
	else
		cmd( "pack .deb.b.move.up .deb.b.move.down .deb.b.move.prev .deb.b.move.broth .deb.b.move.hypern .deb.b.move.last .deb.b.move.search -padx 10 -pady 10 -side left -expand no -fill none" );
	
	if ( mode == 1 )
	{
		cmd( "set stack_flag %d",stackinfo_flag );
		cmd( "button .deb.b.act.run -width -9 -text Run -command {set choice 2} -underline 0" );
		cmd( "button .deb.b.act.until -width -9 -text Until -command {set choice 16} -underline 3" );
		cmd( "button .deb.b.act.ok -width -9 -text Step -command {set choice 1; set_c_var done_in 3} -underline 0" );
		cmd( "button .deb.b.act.an -width -9 -text Analysis -command {set choice 11} -underline 0" );
		cmd( "button .deb.b.act.net -width -9 -text Network -command {set choice 22} -underline 3" );
		cmd( "button .deb.b.act.call -width -9 -text Caller -command {set choice 9} -underline 0" );
		cmd( "button .deb.b.act.hook -width -9 -text Hook -command {set choice 21} -underline 0" );
		cmd( "button .deb.b.act.prn_v -width -9 -text \"v\\\[...\\]\" -command {set choice 15}" );
		cmd( "button .deb.b.act.prn_stck -width -9 -text \"Print Stack\" -command {set choice 13}" );
		cmd( "frame .deb.b.act.stack" );
		cmd( "label .deb.b.act.stack.l -text \"Print stack level\"" );
		cmd( "entry .deb.b.act.stack.e -width 3 -validate focusout -vcmd { if [ string is integer %%P ] { set stack_flag %%P; return 1 } { %%W delete 0 end; %%W insert 0 $stack_flag; return 0 } } -invcmd { bell } -justify center" );
		cmd( ".deb.b.act.stack.e insert 0 $stack_flag" ); 
		cmd( "pack .deb.b.act.stack.l .deb.b.act.stack.e -side left -pady 1 -expand no -fill none" );
		cmd( "pack .deb.b.act.run .deb.b.act.until .deb.b.act.ok .deb.b.act.an .deb.b.act.net .deb.b.act.call .deb.b.act.hook .deb.b.act.prn_v .deb.b.act.prn_stck .deb.b.act.stack -padx 1 -pady 10 -side left -expand no -fill none" );

		cmd( "bind .deb <KeyPress-s> {.deb.b.act.ok invoke}; bind .deb <KeyPress-S> {.deb.b.act.ok invoke}" );
		cmd( "bind .deb <KeyPress-r> {.deb.b.act.run invoke}; bind .deb <KeyPress-R> {.deb.b.act.run invoke}" );
		cmd( "bind .deb <KeyPress-a> {.deb.b.act.an invoke}; bind .deb <KeyPress-A> {.deb.b.act.an invoke}" );
		cmd( "bind .deb <KeyPress-i> {.deb.b.act.until invoke}; bind .deb <KeyPress-I> {.deb.b.act.until invoke}" );
		cmd( "bind .deb <KeyPress-c> {.deb.b.act.call invoke}; bind .deb <KeyPress-C> {.deb.b.act.call invoke}" );
		cmd( "bind .deb <KeyPress-h> {set choice 21}; bind .deb <KeyPress-H> {set choice 21}" );
		cmd( "bind .deb <KeyPress-w> {set choice 22}; bind .deb <KeyPress-W> {set choice 22}" );
	}
	
	cmd( "pack .deb.b.move .deb.b.act -expand no -fill none" );

	cmd( "bind .deb <KeyPress-u> {.deb.b.move.up invoke}; bind .deb <KeyPress-U> {.deb.b.move.up invoke}" );
	cmd( "bind .deb <Up> {.deb.b.move.up invoke}" );
	cmd( "bind .deb <KeyPress-n> {.deb.b.move.broth invoke}; bind .deb <KeyPress-N> {.deb.b.move.broth invoke}" );
	cmd( "bind .deb <Right> {.deb.b.move.broth invoke}" );
	cmd( "bind .deb <KeyPress-t> {.deb.b.move.hypern invoke}; bind .deb <KeyPress-T> {.deb.b.move.hypern invoke}" );
	cmd( "bind .deb <KeyPress-l> {.deb.b.move.last invoke}; bind .deb <KeyPress-L> {.deb.b.move.last invoke}" );
	cmd( "bind .deb <KeyPress-d> {.deb.b.move.down invoke}; bind .deb <KeyPress-D> {.deb.b.move.down invoke}" );
	cmd( "bind .deb <Down> {.deb.b.move.down invoke}" );
	cmd( "bind .deb <KeyPress-f> {.deb.b.move.search invoke}; bind .deb <KeyPress-F> {.deb.b.move.search invoke}" );
	cmd( "bind .deb <KeyPress-p> {.deb.b.move.prev invoke}; bind .deb <KeyPress-P> {.deb.b.move.prev invoke}" );
	cmd( "bind .deb <Left> {.deb.b.move.prev invoke}" );
	cmd( "bind .deb <KeyPress-Escape> {set choice 7}" );
}

app_res=*res;
Tcl_LinkVar(inter, "value", (char *) &app_res, TCL_LINK_DOUBLE);

choice=0;

while (choice==0)
{
// if necessary, create the variable name and the time info bar
if ( mode == 1 )
{
	cmd( "if { ! [ winfo exists .deb.v ] } { \
			frame .deb.v -relief groove -bd 2; \
			frame .deb.v.v1; \
			label .deb.v.v1.name1 -text \"Variable:\"; \
			label .deb.v.v1.name2 -fg red -text \"\"; \
			label .deb.v.v1.time1 -text \"      Time step:\"; \
			label .deb.v.v1.time2 -fg red -text \"      \"; \
			label .deb.v.v1.val1 -text \"Value \"; \
			entry .deb.v.v1.val2 -width 10 -validate focusout -vcmd { \
				if [ string is integer %%P ] { \
					set value %%P; \
					return 1 \
				} { \
					%%W delete 0 end; \
					if [ string is double $value ] { \
						%%W insert 0 [ format \"%%.4g\" $value ]; \
					} { \
						%%W insert 0 $value; \
					} \
					return 0 \
				} \
			} -invcmd { bell } -justify center; \
			pack .deb.v.v1.name1 .deb.v.v1.name2 .deb.v.v1.time1 .deb.v.v1.time2 .deb.v.v1.val1 .deb.v.v1.val2 -side left; \
			bind .deb <KeyPress-g> {set choice 77}; \
			bind .deb <KeyPress-G> {set choice 77} \
		}" );
	cmd( ".deb.v.v1.name2 conf -text \"%s\"", lab );
	Tcl_LinkVar( inter, "time", (char *) &t, TCL_LINK_INT );
	cmd( ".deb.v.v1.time2 conf -text \"$time      \"" );
	Tcl_UnlinkVar( inter, "time" );
}

deb_show(r);

cmd( "pack .deb.b -side right -expand no -expand no -fill none" );

cmd( "if $justCreated { showtop .deb topleftW 0 1; set justCreated false }" );

cmd( "raise .deb; focus .deb" );

ch[0]=(char)NULL;
attach_instance_number(ch, r);

if(asl!=NULL && asl->vs->up!=r)
  asl=NULL;

debug_maincycle:

if ( mode == 1 )
{
	cmd( "if [ string is double $value ] { write_any .deb.v.v1.val2 [ format \"%%.4g\" $value ] } { write_any .deb.v.v1.val2 $value }" ); 
	cmd( "write_any .deb.b.act.stack.e $stack_flag" ); 
	if ( interact_flag == 1 )
	{
		cmd( ".deb.v.val2 selection range 0 end" );
		cmd( "focus .deb.v.val2" );
		cmd( "bind .deb.v.val2 <Return> {.deb.b.act.run invoke}" );
	}
}

// debug command loop
while( ! choice )
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
		goto debug_maincycle;
	}
}   
 
if ( mode == 1 )
{
 cmd( "set value [ .deb.v.v1.val2 get ]" ); 
 cmd( "set stack_flag [ .deb.b.act.stack.e get ]" ); 
 cmd( "bind .deb <KeyPress-g> {}; bind .deb <KeyPress-G> {}" );
 i=choice;
 cmd( "set choice $stack_flag" );
 stackinfo_flag=choice;
 choice=i;
} 

switch(choice)
{
case 1:
cmd( "destroytop .deb.val" );
cmd( "destroytop .deb.net" );

if ( t >= max_step )
{
	cmd( "destroytop .deb" );
	debug_flag = false;
	when_debug = 0;
}
break;

case 2:
cmd( "destroytop .deb.val" );
cmd( "destroytop .deb.net" );

debug_flag = false;
if ( t <= when_debug )
	when_debug = 0;
break;

case 3: if( r->up!=NULL)
			 choice=deb(r->up,c, lab, res);
		  else
			 choice=0;
		  break;

case 77: 
        if(asl==NULL && stacklog->vs!=NULL)
            {
            asl=stacklog;
            plog( "\nVariable: %s", "", asl->label );
   			choice=deb(asl->vs->up,c, lab, res);
            }
          else
           {
            if(asl->next==NULL)
             {
             while(asl->prev->prev!=NULL)
              asl=asl->prev;
             plog( "\nVariable: %s", "", asl->label );
         	 choice=deb(asl->vs->up,c, lab, res);
             }
             else
              {
               asl=asl->next;
               plog( "\nVariable: %s", "", asl->label );
               choice=deb(asl->vs->up,c, lab, res);
              }
           }  
         break;  

case 4: if( r->next!=NULL)
			 choice=deb(r->next,c, lab, res);
		  else
			 {
        cur=skip_next_obj(r);
        if(cur!=NULL)
         choice=deb(cur,c, lab, res);
        else 
         choice=0;
        } 

			break;
case 21: if(r->hook!=NULL)
		{
			if ( ! invalidHooks )
			{
				int lstUpd;
				// check if the hook contains a valid Lsd object pointer (not very effective, most likely will crash...)
				try { lstUpd = r->hook->lstCntUpd; }
				catch ( ... ) {	lstUpd = 0; }
				
				if ( lstUpd <= 0 || lstUpd > t )
				{
					cmd( "tk_messageBox -parent .deb -type ok -icon error -title Error -message \"Invalid hook pointer\" -detail \"Check if your code is using valid pointers to Lsd objects or avoid using this option. If non-standard hooks are used, consider adding the command 'invalidHooks = true' to your model code.\"" );
					choice = 0;
					break;
				}
				
				choice=deb(r->hook,c, lab, res);
			}
			else
			{
				cmd( "tk_messageBox -parent .deb -type ok -icon error -title Error -message \"Unavailable option\" -detail \"Your code is using non-standard pointers ('invalidHooks = true').\"" );
				choice = 0;
				break;
			}
		}
		  else
			 choice=0;

			break;
			
//Show network node attributes
case 22:
	if(r->node == NULL)
		break;

	cmd( "set n .deb.net" );
	cmd( "destroytop $n" );

	cmd( "newtop $n \"Network\" { destroytop .deb.net } .deb" );
	
	cmd( "label $n.l1 -text \"Node id: \"" );
	cmd( "label $n.l2 -foreground red -text \"%ld\"", r->node->id );
	cmd( "label $n.l3 -text \"Num. links out: \"" );
	cmd( "label $n.l4 -foreground red -text \"%ld\"", r->node->nLinks );
	cmd( "label $n.l5 -text \"Outgoing links: \"" );
	cmd( "label $n.l6 -text \"dest. id          (weight)\" -foreground red" );
	if(r->node->name != NULL)		// is node named?
	{
		cmd( "label $n.l7 -text \"Node name: \"" );
		cmd( "label $n.l8 -foreground red -text \"%s\"", r->node->name );
		cmd( "pack $n.l7 $n.l8 $n.l1 $n.l2 $n.l3 $n.l4 $n.l5" );
	}
	else
		cmd( "pack $n.l1 $n.l2 $n.l3 $n.l4 $n.l5" );
	cmd( "pack $n.l6 -anchor w" );
	cmd( "text $n.t -width 15 -yscrollcommand \"$n.yscroll set\" -wrap word" );
	cmd( "scrollbar $n.yscroll -command \"$n.t yview\"" );
	cmd( "pack $n.yscroll -side right -fill y" );
	cmd( "pack $n.t -expand yes -fill both" );
	cmd( "done $n c { destroytop .deb.net }" ); 
	cmd( "$n.t tag configure v -foreground red" );
	
	for(netLink *curLnk=r->node->first; curLnk != NULL; curLnk=curLnk->next)
	{
		cmd( "$n.t insert end \"%ld\"", curLnk->ptrTo->node->id );
		if(curLnk->weight != 0)
			cmd( "$n.t insert end \"\t(%g)\n\"", curLnk->weight );
		else
			cmd( "$n.t insert end \"\n\"" );
	}
	
	cmd( "showtop $n topleftW 0 1 0" );
	
	cmd( "if { ! [ winfo exists .deb.val ] } { align $n .deb } { align $n .deb.val }" );
	
	choice=0;

break;

case 12:if(r->up==NULL)
          {choice=0;
           break;
          }
        for(cb1=NULL, cb=r->up->b; strcmp(r->label, cb->blabel);cb1=cb, cb=cb->next);
        cur=cb->head;
        if(cur==r)
          {if(cb1!=NULL)
            {
             for(cur=cb1->head; cur->next!=NULL; cur=cur->next);
             choice=deb(cur,c, lab, res);
             break;
            }
           else 
            choice=0;
           break;
          }
          
        for(; cur->next!=r; cur=cur->next);
		    choice=deb(cur,c, lab, res);
        break;
case 5:
		  cur=skip_next_obj(r, &count);
		  if( cur!=NULL)
			 choice=deb(cur,c, lab, res);
		  else
			 choice=0;

		  break;
case 14: //go last

for(cur=r; cur!=NULL; cur=cur->next )
  cur1=cur;
choice=deb(cur1,c, lab, res);
break;


case 6:
		  if( r->b!=NULL && r->b->head!=NULL)
			 choice=deb(r->b->head,c, lab, res);
		  else
			 choice=0;
		  break;
case 9:
		  if( c!=NULL)
			 choice=deb(c,r, lab, res);
		  else
			 choice=0;
		  break;
case 10:
Tcl_LinkVar(inter, "value_search", (char *) &value_search, TCL_LINK_DOUBLE);
Tcl_LinkVar(inter, "condition", (char *) &cond, TCL_LINK_INT);
cond=0;
choice=0;
value_search = 0;
i=1;

cmd( "set s .deb.so" );
cmd( "newtop $s \"Find Object\" { set choice 2 } .deb" );

cmd( "frame $s.l" );
cmd( "label $s.l.l -text \"Find object containing variable\"" );
cmd( "entry $s.l.e -width 20 -justify center -textvariable en" );
cmd( "bind $s.l.e <KeyRelease> {if { %%N < 256 && [info exists ModElem] } { set bb1 [.deb.so.l.e index insert]; set bc1 [.deb.so.l.e get]; set bf1 [lsearch -glob $ModElem $bc1*]; if { $bf1 !=-1 } { set bd1 [lindex $ModElem $bf1]; .deb.so.l.e delete 0 end; .deb.so.l.e insert 0 $bd1; .deb.so.l.e index $bb1; .deb.so.l.e selection range $bb1 end } } }" );
cmd( "pack $s.l.l $s.l.e" );

cmd( "frame $s.c" );
cmd( "label $s.c.l -text \"Conditional to value\"" );

cmd( "frame $s.c.cond -relief groove -bd 2" );
cmd( "radiobutton $s.c.cond.any -text \"Any\" -variable condition -value 0 -command { .deb.so.v.e configure -state disabled }" );
cmd( "radiobutton $s.c.cond.eq -text \"=\" -variable condition -value 1 -command { .deb.so.v.e configure -state normal }" );
cmd( "radiobutton $s.c.cond.geq -text \u2265 -variable condition -value 2 -command { .deb.so.v.e configure -state normal }" );
cmd( "radiobutton $s.c.cond.leq -text \u2264 -variable condition -value 3 -command { .deb.so.v.e configure -state normal }" );
cmd( "radiobutton $s.c.cond.gr -text \">\" -variable condition -value 4 -command { .deb.so.v.e configure -state normal }" );
cmd( "radiobutton $s.c.cond.le -text \"<\" -variable condition -value 5 -command { .deb.so.v.e configure -state normal }" );
cmd( "radiobutton $s.c.cond.not -text \u2260 -variable condition -value 6 -command { .deb.so.v.e configure -state normal }" );
cmd( "pack $s.c.cond.any $s.c.cond.eq $s.c.cond.geq $s.c.cond.leq $s.c.cond.gr $s.c.cond.le $s.c.cond.not -anchor w" );

cmd( "pack $s.c.l $s.c.cond" );

cmd( "frame $s.v" );
cmd( "label $s.v.l -text Value" );
cmd( "entry $s.v.e -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set value_search %%P; return 1 } { %%W delete 0 end; %%W insert 0 $value_search; return 0 } } -invcmd { bell } -justify center -state disabled" );
cmd( "write_any $s.v.e $value_search" ); 
cmd( "pack $s.v.l $s.v.e" );

cmd( "pack $s.l $s.c $s.v -padx 5 -pady 5" );

cmd( "okcancel $s b { set choice 1 } { set choice 2 }" );

cmd( "bind $s.l.e <KeyPress-Return> { focus .deb.so.b.ok }" );
cmd( "bind $s.v.e <KeyPress-Return> { focus .deb.so.b.ok }" );

cmd( "showtop $s" );
cmd( "focus $s.l.e" );
cmd( "$s.l.e selection range 0 end" );

while(choice==0)
 Tcl_DoOneEvent(0);

if(choice==2)
 {
quit=0;
cmd( "destroytop $s" );
Tcl_UnlinkVar(inter, "value_search");
Tcl_UnlinkVar(inter, "condition");
choice=0;
break;
 }
 
pre_running = running;
running = false;

cmd( "set value_search [ $s.v.e get ]" ); 
ch1=(char *)Tcl_GetVar(inter, "en",0);
strcpy(ch, ch1);

cur = NULL;
switch(cond)
{
case 0:
cv=r->search_var(r, ch);
if ( cv != NULL)
	cur=cv->up;
break;
 
case 1:
i=0;
cur2=NULL;
for(cur1=r; cur1!=NULL && i==0; cur1=cur1->up)
{

cv=cur1->search_var(r, ch);
if(cv==NULL)
 break;
 
cur=cv->up;

for( ;cur!=NULL && i==0; cur=cur->hyper_next(cur->label))
 {app_res=cur->search_var(cur,ch)->val[0];
  if(app_res==value_search)
    {cur2=cur;
     i=1;
    } 
     
 }
}
cur=cur2;
break;

case 2:
cv=r->search_var(r, ch);
if ( cv != NULL)
	cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)<value_search; )
 cur=cur->hyper_next(cur->label);
break;

case 3:
cv=r->search_var(r, ch);
if ( cv != NULL)
	cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)>value_search; )
 cur=cur->hyper_next(cur->label);
break;

case 4:
cv=r->search_var(r, ch);
if ( cv != NULL)
	cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)<=value_search; )
 cur=cur->hyper_next(cur->label);
break;

case 5:
cv=r->search_var(r, ch);
if ( cv != NULL)
	cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)>=value_search; )
 cur=cur->hyper_next(cur->label);
break;

case 6:
cv=r->search_var(r, ch);
if ( cv != NULL)
	cur=cv->up;
for(; cur!=NULL && cur->cal(ch, 0)==value_search; )
 cur=cur->hyper_next(cur->label);
break;

default:
cur=NULL;
}

quit=0; //If var is mispelled don't stop the simulation!
cmd( "destroytop $s" );
Tcl_UnlinkVar(inter, "value_search");
Tcl_UnlinkVar(inter, "condition");

if(cur!=NULL)
 choice=deb(cur, r, lab, res);
else
 choice=0;

running = pre_running;
break;


// quit
case 7:

if ( mode == 1 )
 {
 cmd( "set answer [ tk_messageBox -parent .deb -type okcancel -default cancel -icon warning -title Warning -message \"Stop simulation\" -detail \"Quitting the simulation run.\nPress 'Ok' to confirm.\" ]; if [ string equal -nocase $answer ok ] { set choice 0 } { set choice 1 }" );
 } 
else
 choice=0; 

if(choice==1) 
 {
 choice=deb(r,c, lab, res);
 break;
 }

cmd( "destroytop .deb" );
choice=1;

// prevent changing run parameters when only data browse was called
if ( mode == 1 )
{
	quit=1;
	debug_flag=false;
}

if ( mode == 3 )
	cmd( "wm deiconify .log; raise .log; focus .log" );

break;

		 
case 29:
ch1=(char *)Tcl_GetVar(inter, "res",0);
strcpy(ch, ch1);
set_all(&choice, r, ch, 0);
choice=0;
break;
		
case 8:
ch1=(char *)Tcl_GetVar(inter, "res",0);
Tcl_LinkVar(inter, "debug", (char *) &count, TCL_LINK_INT);
Tcl_LinkVar(inter, "time", (char *) &t, TCL_LINK_INT);
Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);

cv=r->search_var(NULL, ch1);
i=cv->last_update;
count=(cv->debug=='d'?1:0);
app_values=new double[cv->num_lag+1];
cmd( "set debugall 0" );

cmd( "set e .deb.stat" );
cmd( "newtop $e \"Element Status\" { set choice 1 } .deb" );

cmd( "frame $e.n" );
switch ( cv->param )
{
	case 1:
		cmd( "label $e.n.l -text \"Parameter:\"" );
		break;

	case 2:
		cmd( "label $e.n.l -text \"Function:\"" );
		break;
		
	case 0:
		cmd( "label $e.n.l -text \"Variable:\"" );
		break;
}
cmd( "label $e.n.v -fg red -text $res" );
cmd( "pack $e.n.l $e.n.v -side left -padx 2" );

cmd( "frame $e.t" );
cmd( "label $e.t.l -text \"Current time:\"" );
cmd( "label $e.t.v -fg red -text $time" );
cmd( "pack $e.t.l $e.t.v -side left -padx 2" );

cmd( "frame $e.u" );
cmd( "label $e.u.l -text \"Last update time:\"" );
cmd( "label $e.u.v -fg red -text %d", cv->last_update );
cmd( "pack $e.u.l $e.u.v -side left -padx 2" );

cmd( "frame $e.v" );
for ( i = 0; i <= cv->num_lag; ++i )
{
  cmd( "set val%d %g",i, cv->val[i] );
  app_values[i]=cv->val[i];
  sprintf(ch, "val%d",i);
  Tcl_LinkVar(inter, ch, (char *) &(app_values[i]), TCL_LINK_DOUBLE);
  
  cmd( "frame $e.v.l$i" );

  if ( cv->param == 1 || cv->num_lag == 0 )
    cmd( "label $e.v.l$i.l -text \"Value:\"" );
  else
    cmd( "label $e.v.l$i.l -text \"Lag $i:\"" );

  cmd( "entry $e.v.l%d.e -width 15 -validate focusout -vcmd { if [ string is double %%P ] { set val%d %%P; return 1 } { %%W delete 0 end; %%W insert 0 $val%d; return 0 } } -invcmd { bell } -justify center", i, i, i );
  cmd( "$e.v.l%d.e insert 0 $val%d", i, i ); 

  cmd( "button $e.v.l$i.sa -width 5 -text \"Set All\" -command { set sa %i; set choice 10 }", i );
  cmd( "pack $e.v.l$i.l $e.v.l$i.e $e.v.l$i.sa -side left -padx 2" );
  cmd( "pack $e.v.l$i" );
}

if ( cv->param == 1 )
{
	cmd( "pack $e.n $e.t" );
	cmd( "pack $e.v -pady 5 -padx 5" );
}
else
{
	cmd( "pack $e.n $e.t $e.u" );	
	
	cmd( "frame $e.d" );
	cmd( "checkbutton $e.d.deb -text \"Debug (this instance only)\" -variable debug" );
	cmd( "checkbutton $e.d.deball -text \"Debug all instances\" -variable debugall -command { if { $debugall == 1 } { set debug 1; .deb.stat.d.deb configure -state disabled } { set debug 0; .deb.stat.d.deb configure -state normal } }" );
	cmd( "pack $e.d.deb $e.d.deball" );

	cmd( "pack $e.v $e.d -pady 5 -padx 5" );	

	cmd( "frame $e.b1" );
	cmd( "button $e.b1.eq -width -9 -text Equation -command {set choice 8}" );
	cmd( "button $e.b1.cond -width -9 -text \"Set Break\" -command {set choice 7}" );
	cmd( "button $e.b1.exec -width -9 -text Update -command {set choice 9}" );
	cmd( "pack $e.b1.eq $e.b1.cond $e.b1.exec -padx 10 -side left" );
	cmd( "pack $e.b1" );	
}

cmd( "donehelp $e b { set choice 1 } { LsdHelp debug.html#content }" );
cmd( "bind $e.v.l0.e <Return> { set choice 1 }" );

cmd( "showtop $e" );
cmd( "focus $e.v.l0.e" );
cmd( "$e.v.l0.e selection range 0 end" );

choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);

cv->data_loaded='+';

for ( i = 0; i <= cv->num_lag; ++i )
{
  cmd( "set val%d [ $e.v.l%d.e get ]", i, i ); 

  cv->val[i]=app_values[i];
  sprintf(ch, "val%d",i);

  Tcl_UnlinkVar(inter, ch);
  cmd( "unset val$i" );
}
delete[] app_values;
Tcl_UnlinkVar(inter, "i");

cmd( "destroytop $e" );

cv->debug=(count==1?'d':'n');
Tcl_UnlinkVar(inter, "debug");
count=choice;

cmd( "set choice $debugall" );
if(choice==1)
{
  for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label) )
  {
    cv1=cur->search_var(cur,cv->label);
    cv1->debug=cv->debug;
  }
}
 
choice=count;
if ( choice == 7 )
{
	cmd( "set cond %d", cv->deb_cond );
	cmd( "set cond_val %4g", cv->deb_cnd_val );

	cmd( "set cb .deb.cbrk" );

	cmd( "newtop $cb \"Conditional Break\" { set choice 2 } .deb" );

	cmd( "frame $cb.l" );
	cmd( "label $cb.l.l -text \"Variable:\"" );
	cmd( "label $cb.l.n -fg red -text %s", cv->label );
	cmd( "pack $cb.l.l $cb.l.n -side left -padx 2" );

	cmd( "frame $cb.t" );
	cmd( "label $cb.t.l -text \"Type of conditional break\"" );

	cmd( "frame $cb.t.t -relief groove -bd 2" );
	cmd( "radiobutton $cb.t.t.none -text \"None\" -variable cond -value 0 -command { .deb.cbrk.v.e configure -state disabled }" );
	cmd( "radiobutton $cb.t.t.eq -text \"=\" -variable cond -value 1 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
	cmd( "radiobutton $cb.t.t.gr -text \">\" -variable cond -value 2 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
	cmd( "radiobutton $cb.t.t.le -text \"<\" -variable cond -value 3 -command { .deb.cbrk.v.e configure -state normal; .deb.cbrk.v.e selection range 0 end; focus .deb.cbrk.v.e }" );
	cmd( "pack $cb.t.t.none $cb.t.t.eq $cb.t.t.gr $cb.t.t.le -anchor w" );

	cmd( "pack $cb.t.l $cb.t.t" );

	cmd( "frame $cb.v" );
	cmd( "label $cb.v.l -text Value" );
	cmd( "entry $cb.v.e -width 10 -validate focusout -vcmd { if [ string is double %%P ] { set cond_val %%P; return 1 } { %%W delete 0 end; %%W insert 0 $cond_val; return 0 } } -invcmd { bell } -justify center -state disabled" );
	cmd( "write_any $cb.v.e $cond_val" ); 
	cmd( "if { $cond != 0 } { $cb.v.e configure -state normal; $cb.v.e selection range 0 end; focus $cb.v.e }" );
	cmd( "pack $cb.v.l $cb.v.e" );

	cmd( "pack $cb.l $cb.t $cb.v -padx 5 -pady 5" );

	cmd( "okcancel $cb b { set choice 1 } { set choice 2 }" );
	cmd( "bind $cb.v.e <Return> { set choice 1 }" );

	cmd( "showtop $cb" );

	choice = 0;
	while ( choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set cond_val [ $cb.v.e get ]" ); 

	cmd( "destroytop $cb" );

	if ( choice == 1 )
	{
		get_int( "cond", &cv->deb_cond );
		get_double( "cond_val", &cv->deb_cnd_val );
	}
}

if(choice==8)
{
 choice = 3;	// point .deb window as parent for the following window
 show_eq(cv->label, &choice);
 choice = 8;
}

if(choice==9)
 cv->cal(cv->up, 0);

if(choice==10)
{
 ch1=(char *)Tcl_GetVar(inter, "res",0);
 strcpy(ch, ch1);
 
 cmd( "set choice $sa" );
 i=choice;
 choice=0;
 
 set_all(&choice, r, ch, i);
} 

choice=0;
break;

//Analysis
case 11:
cmd( "destroytop .deb.val" );
cmd( "destroytop .deb.net" );

actual_steps=t;
for(cur=r; cur->up!=NULL; cur=cur->up);
reset_end(cur);
analysis(&choice);
choice=0;
break;

//Print stack
case 13:
print_stack();
cmd( "wm deiconify .log; raise .log; focus .log" );
choice=0;
break;

case 15:
cmd( "set in .deb.val" );
cmd( "destroytop $in" );

cmd( "newtop $in \"v\\[...\\]\" { destroytop .deb.val } .deb" ); 

cmd( "text $in.t -width 20 -yscrollcommand \"$in.yscroll set\" -wrap word" ); 
cmd( "scrollbar $in.yscroll -command \"$in.t yview\"" ); 
cmd( "pack $in.yscroll -side right -fill y" ); 
cmd( "pack $in.t -expand yes -fill both" );

cmd( "done $in c { destroytop .deb.val }" );

cmd( "showtop $in topleftW 0 1 0" );

cmd( "$in.t tag configure v -foreground red" );

cmd( "if { ! [ winfo exists .deb.net ] } { align $in .deb } { align $in .deb.net }" );

for(i=0; i<100; i++)
 {
  cmd( "$in.t insert end \"v\\\[%d\\]\" v",i );
  cmd( "$in.t insert end \"=%g\n\"",i_values[i] );
 }
 
choice=0;
break;

case 16:
//run until

cmd( "set tdebug %d", t + 1 );

cmd( "set t .deb.tdeb" );
cmd( "newtop $t \"Run Until\" { set choice 2 } .deb" );

cmd( "frame $t.t" );
cmd( "label $t.t.l -text \"Run until time step\"" );
cmd( "entry $t.t.val -width 5 -validate focusout -vcmd { if [ string is integer %%P ] { set tdebug %%P; return 1 } { %%W delete 0 end; %%W insert 0 $tdebug; return 0 } } -invcmd { bell } -justify center" );
cmd( "$t.t.val insert 0 $tdebug" ); 
cmd( "pack $t.t.l $t.t.val" );

cmd( "pack $t.t -padx 5 -pady 5" );

cmd( "okhelpcancel $t b { set choice 1 } { LsdHelp debug.html#until } { set choice 2 }" );

cmd( "bind $t.t.val <Return> { set choice 1 }" );

cmd( "showtop $t" );
cmd( "$t.t.val selection range 0 end" );
cmd( "focus $t.t.val" );

choice=0;
while(choice==0)
 Tcl_DoOneEvent(0);

cmd( "set tdebug [ $t.t.val get ]" ); 
cmd( "destroytop $t" );

if ( choice == 1 )
{
	//restart execution
	choice=2;
	debug_flag=false;
	cmd( "if { $tdebug > %d } { set when_debug $tdebug } { set when_debug %d }", t, t + 1 );
	cmd( "destroytop .deb" );
}
else
	choice=0;

break;

case 17:
//change the object number
if(r->up!=NULL)
 entry_new_objnum(r, &choice, "");


choice=0;
break;


case 44:
//see model report

cmd( "set name_rep %s", name_rep );

cmd( "set choice [file exists $name_rep]" );

cmd( "if {$choice == 1} {LsdHtml $name_rep} {}" );
cmd( "if {$choice == 0} {tk_messageBox -parent .deb -type ok -title Error -icon error -message \"Report file not available\" -detail \"You can create the report in menu Model.\"} {}" );
choice=0;
break;



default:
plog("\nChoice not recognized");
choice=0;
break;
}
}

Tcl_UnlinkVar(inter, "value");
*res=app_res;

return(choice);
}


/*******************************************
DEB_SHOW
********************************************/
void deb_show(object *r)
{
char ch[2*MAX_ELEM_LENGTH];
variable *ap_v;
int count, i;
object *ap_o;

// fix the top frame before proceeding
cmd( "if { ! [ winfo exists .deb.v ] } { frame .deb.v -relief groove -bd 2 }" );
cmd( "if { ! [ winfo exists .deb.v.v2 ] } { \
		frame .deb.v.v2; \
		label .deb.v.v2.obj -text \"Level & object instance: \"; \
		label .deb.v.v2.instance -fg red -text \"\"; \
		pack .deb.v.v2.obj .deb.v.v2.instance -side left; \
		if [ winfo exists .deb.v.v1 ] { \
			pack .deb.v.v1 .deb.v.v2 -padx 5 -pady 5 -anchor w \
		} { \
			pack .deb.v.v2 -padx 5 -pady 5 -anchor w \
		}; \
		pack .deb.v -anchor w -expand no -fill x \
	}" );

if(r->up!=NULL)
{
 cmd( "bind .deb.v.v2.obj <Button-1> {if { [winfo exist .deb.w] } { destroy .deb.w }; set choice 17}" );
 cmd( "bind .deb.v.v2.instance <Button-1> {if { [winfo exist .deb.w] } { destroy .deb.w }; set choice 17}" );
}

strcpy( ch, "" );
attach_instance_number( ch, r );
cmd( ".deb.v.v2.instance config -text \"%s\"", ch  );

// adjust spacing to align labels with data and increase columns width to better fill window
cmd( "if {$tcl_platform(platform) == \"windows\"} {set w1 26; set w2 10; set w3 20} {set w1 19; set w2 9; set w3 15}" );
cmd( "if {$tcl_platform(os)==\"Darwin\"} {set wwidth 115} {set wwidth 100}" );

cmd( "if { ! [ winfo exists .deb.tit ] } { \
		frame .deb.tit; \
		frame .deb.tit.h1; \
		label .deb.tit.h1.name -text Variable -width $w1 -pady 0 -bd 0 -anchor w; \
		label .deb.tit.h1.last -text LastUpdate -width $w2 -pady 0 -bd 0 -anchor w; \
		label .deb.tit.h1.val -text Value -width $w3 -pady 0 -bd 0 -fg red -anchor w; \
		pack .deb.tit.h1.name .deb.tit.h1.val .deb.tit.h1.last -side left; \
		frame .deb.tit.h2; \
		label .deb.tit.h2.name -text Variable -width $w1 -pady 0 -bd 0 -anchor w; \
		label .deb.tit.h2.last -text LastUpdate -width $w2 -pady 0 -bd 0 -anchor w; \
		label .deb.tit.h2.val -text Value -width $w3 -pady 0 -bd 0 -fg red -anchor w; \
		pack .deb.tit.h2.name .deb.tit.h2.val .deb.tit.h2.last -side left -anchor w; \
		pack .deb.tit.h1 .deb.tit.h2 -expand no -side left; \
		pack .deb.tit -side top -anchor w -expand no -after .deb.v \
	}" );

cmd( "if { ! [ winfo exists .deb.cc ] } { \
		frame .deb.cc; \
		scrollbar .deb.cc.scroll -command \".deb.cc.l yview\"; \
		pack .deb.cc.scroll -side right -fill y; \
		text .deb.cc.l -yscrollcommand \".deb.cc.scroll set\" -wrap word -width $wwidth  -cursor arrow; \
		mouse_wheel .deb.cc.l; \
		pack .deb.cc.l -expand yes -fill y; \
		pack .deb.cc -expand yes -fill y\
	}" );

cmd( ".deb.cc.l delete 1.0 end" );

if(r->v==NULL)
  {cmd( "label .deb.cc.l.no_var -text \"(no variables defined)\"" );
	cmd( ".deb.cc.l window create end -window .deb.cc.l.no_var" );
  }
else
  {Tcl_LinkVar(inter, "i", (char *) &i, TCL_LINK_INT);

  for(i=1,ap_v=r->v; ap_v!=NULL; ap_v=ap_v->next, i++)
	 {
      cmd( "set last %d", ap_v->last_update );
      cmd( "set val %.4g", ap_v->val[0] );
	  cmd( "frame .deb.cc.l.e$i" );
	  cmd( "label .deb.cc.l.e$i.name -width $w1 -pady 0 -anchor w -bd 0 -text %s", ap_v->label );
	  if(ap_v->param==0)
		 cmd( "label .deb.cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text $last" );
	  if(ap_v->param==1)
		 cmd( "label .deb.cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text Par" );
	  if(ap_v->param==2)
		 cmd( "label .deb.cc.l.e$i.last -width $w2 -pady 0 -bd 0 -text Fun" );
	  cmd( "label .deb.cc.l.e$i.val -width $w3 -pady 0 -bd 0 -fg red -anchor w -text $val" );
	  cmd( "pack .deb.cc.l.e$i.name .deb.cc.l.e$i.val .deb.cc.l.e$i.last -side left" );

	  cmd( "bind .deb.cc.l.e$i.name <Button-1> {set res %s; set lstDebPos [ .deb.cc.l index @%%x,%%y ]; set choice 8}", ap_v->label );
	  cmd( "bind .deb.cc.l.e$i.name <Button-2> {set res %s; set lstDebPos [ .deb.cc.l index @%%x,%%y ]; set choice 29}", ap_v->label );
	  cmd( "bind .deb.cc.l.e$i.name <Button-3> { event generate .deb.cc.l.e$i.name <Button-2> -x %%x -y %%y }" );
      
	  cmd( ".deb.cc.l window create end -window .deb.cc.l.e$i" );
     if( (i%2)==0)
       cmd( ".deb.cc.l insert end \\n" );
	 }
   
	cmd( "if [ info exists lstDebPos ] { .deb.cc.l see $lstDebPos; unset lstDebPos }" );
	
	Tcl_UnlinkVar(inter, "i");
  }
}


/*******************************************
ATTACH_INSTANCE_NUMBER
********************************************/
int depth;

void attach_instance_number(char *ch, object *r)
{
object *cur;
int i, j;

if(r==NULL)
 return;
attach_instance_number(ch, r->up);

if(r->up!=NULL)
 for(i=1,cur=r->up->search(r->label); cur!=NULL; cur=go_brother(cur) )
   {
   if(cur==r)
    j=i;
   i++;
   }

if(r->up==NULL)
   sprintf( msg, "%d:%s (1/1) ", depth = 1, r->label );
else
   sprintf( msg, " |  %d:%s (%d/%d) ", ++depth, r->label, j, i - 1 );
strcat(ch, msg);
}


/*******************************************
INTERACT
********************************************/
#ifndef NO_WINDOW
double object::interact(char const *text, double v, double *tv)
{
/*
Interrupt the simulation, as for the debugger, allowing the insertion of a value.
Note that the debugging window, in this model, accept the entry key stroke as a run.
*/
int i;

double app;
if(quit!=0)
 return v;
app=v;
interact_flag=1;
for(i=0; i<100; i++)
 i_values[i]=tv[i];
deb(this, NULL, text, &app);
interact_flag=0;
return app;
}

#else
double object::interact(char const *text, double v, double *tv)
{
 return v;
}
#endif
