/***************************************************
****************************************************
LSD 6.1 - June 2011
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/



/************************
REPORT.CPP
This file contains the code for the report generating routines. The main function,
(report) needs the pointer to the root of the model. It also needs the equation
file name to be correctly set (if the file does not exist, the routine ask for it).
The output of the routine is a html file containing the main information on the model.
This file is meant to be used as basic structure to be filled with modellers' comments
in order to obtain the complete documentation of the model.

The report lists all the objects, variables and parameters, all linked with pointers,
so that readers can easily jump hypertextually through the whole report.

- Each object is listed with indication of its ancestors.
- For each object, the set of variables and parameters is listed.
- For each parameter is listed the set of variables whose equations make use of its value.
- For each variables is listed the set of variables whose equations make use of its values,
and also the whole set of variables and parameters used in the its own equation.


FILE SHOW_EQ.CPP
- int contains (FILE *f, char *lab, int len);
Checks if the equation beginning in the file position indicated by f contains
any function using lab as parameter.

************************/
#include <tk.h>
#include "decl.h"

#define MAX_INIT 1000

void cmd(Tcl_Interp *inter, char const *cc);
void plog(char const *msg);
int contains (FILE *f, char *lab, int len);
void write_var(variable *v, FILE *frep);
void write_obj(object *r, FILE *frep);
void write_str(object *r, FILE *frep, int dep, char const *prefix );

object *skip_next_obj(object *t, int *count);
object *skip_next_obj(object *t);
void find_using(object *r, variable *v, FILE *frep);
void insert_summary(object *r, FILE *frep);
void clean_spaces(char *s);
void write_list(FILE *frep, object *root, int flag_all, char const *prefix);
void fill_list_var(object *r, int flag_all, int flag_init);
void fill_list_par(object *r, int flag_all);
void createmodelhelp(int *choice, object *r);

void create_table_init(object *r);
int is_equation_header(char *line, char *var);
description *search_description(char *lab);
void create_initial_values(object *r);
void ancestors(object *r, FILE *f);
FILE *create_frames(char *t);
void create_form(int num, char const *title, char const *prefix);
void insert_docuoptions(FILE *frep, object *r);
void add_description(char const *lab, char const *type, char const *text);

extern char msg[];
extern Tcl_Interp *inter;
extern char *equation_name;
extern char *simul_name;
extern char *name_report;
extern int macro;
extern char name_rep[400];
int fatto;
int code=1;
int init=0;
int pos;
int ltext;
int lmenu;

FILE *frep;
/******************************
REPORT
*******************************/
void report(int *choice, object *r)
{
FILE *ffun, *f;
int count, es1=1, es2;
char *app, ch;
object *cur;
variable *cv;


sprintf(name_rep, "report_%s.html", simul_name);
sprintf(msg, "set mrep %s", name_rep);
cmd(inter, msg);
cmd(inter, "set choice [file exists $mrep]");
cmd(inter, "wm iconify .");
if(*choice == 1)
 {
  cmd(inter, "set answer [tk_messageBox -message \"Model Report already exists.\\nIf you do not change the file name you are going to overwrite it.\\n\" -type ok -title Warning -icon warning]");
  cmd(inter, "if {[string compare $answer \"yes\"] == 0} {set choice 0} {set choice 1}");
 }


*choice=0;

cmd(inter, "toplevel .w");
cmd(inter, "wm title .w \"Report Generation\"");
cmd(inter, "label .w.l1 -text \"Report Generation\" -font {System 14 bold}");
cmd(inter, "frame .w.f");
cmd(inter, "label .w.f.l -text \"Model title: \"");
sprintf(msg, "set reptit %s", simul_name);
cmd(inter, msg);
cmd(inter, "entry .w.f.e -width 40 -textvariable reptit");
cmd(inter, "pack .w.f.l .w.f.e -side left");

cmd(inter, "frame .w.l");


cmd(inter, "frame .w.l.opt");
cmd(inter, "label .w.l.opt.l -text \"Variables listing format: \"");
cmd(inter, "set lmenu 1; set ltext 0");
cmd(inter, "checkbutton .w.l.opt.popup -text \"Scrolling boxes\" -variable lmenu");
cmd(inter, "checkbutton .w.l.opt.text -text \"Simple text\" -variable ltext");
cmd(inter, "pack .w.l.opt.l .w.l.opt.popup .w.l.opt.text -anchor w -side left");
cmd(inter, "pack .w.l.opt");


cmd(inter, "frame .w.b");
cmd(inter, "button .w.b.b1 -text Ok -underline 0 -command {set choice 1}");
cmd(inter, "button .w.b.b2 -text \"New Name\" -underline 0 -command {set res [tk_getSaveFile -filetypes {{{HTML Files} {.html}} {{All Files} {*}} }]; set choice 2}");
cmd(inter, "button .w.b.b3 -text Cancel -underline 0 -command {set choice 3}");
cmd(inter, "button .w.b.b4 -text Help -command {LsdHelp menumodel.html#createreport}");

cmd(inter, "pack .w.b.b1 .w.b.b2 .w.b.b3 .w.b.b4 -side left");
cmd(inter, "bind .w <Control-o> {.w.b.b1 invoke}");


cmd(inter, "bind .w <Control-n> {.w.b.b2 invoke}");
cmd(inter, "bind .w <Control-c> {.w.b.b3 invoke}");
cmd(inter, "bind .w <KeyPress-Escape> {.w.b.b3 invoke}");

cmd(inter, "checkbutton .w.code -text \"Include Equations' Code\" -underline 8 -variable code");
cmd(inter, "bind .w <Control-e> {.w.code invoke}");
cmd(inter, "checkbutton .w.init -text \"Include Initial Values\" -underline 8 -variable init");
cmd(inter, "bind .w <Control-i> {.w.init invoke}");




cmd(inter, "frame .w.s -bd 2 -relief groove");
cmd(inter, "label .w.s.lab -text \"Extra Sections\"");
cmd(inter, "frame .w.s.e1 -bd 2 -relief groove");
cmd(inter, "label .w.s.e1.lab -text \"First Extra Section\"");
cmd(inter, "set es1 1");
cmd(inter, "frame .w.s.e1.c");
cmd(inter, "checkbutton .w.s.e1.c.c -text \"Include Section 1\" -variable es1 -command {if {$es1==1} {.w.s.e1.header.tit conf -state normal; .w.s.e1.file.tit conf -state normal} {.w.s.e1.header.tit conf -state disabled; .w.s.e1.file.tit conf -state disabled}}");
cmd(inter, "set html1 0");
cmd(inter, "checkbutton .w.s.e1.c.h -text \"Accept HTML formatting tags\" -variable html1");
cmd(inter, "pack .w.s.e1.c.c .w.s.e1.c.h -side left -fill x");
cmd(inter, "frame .w.s.e1.header");
cmd(inter, "label .w.s.e1.header.tlab -text \"Title for Section 1 \"");
cmd(inter, "set tit1 Description");
cmd(inter, "entry .w.s.e1.header.tit -width 50  -textvariable tit1");
cmd(inter, "pack .w.s.e1.header.tlab .w.s.e1.header.tit -side left");

cmd(inter, "frame .w.s.e1.file");
cmd(inter, "label .w.s.e1.file.tlab -text \"File Name\"");
cmd(inter, "set file1 description.txt");
cmd(inter, "entry .w.s.e1.file.tit -width 50 -textvariable file1");
cmd(inter, "button .w.s.e1.file.new -text \"Search File\" -command {set file1 [tk_getOpenFile -filetypes {{{All Files} {*}} }]}");
cmd(inter, "pack .w.s.e1.file.tlab .w.s.e1.file.tit .w.s.e1.file.new -anchor w -side left");

cmd(inter, "pack .w.s.e1.lab .w.s.e1.c .w.s.e1.header .w.s.e1.file -anchor w ");




cmd(inter, "frame .w.s.e2 -bd 2 -relief groove");
cmd(inter, "label .w.s.e2.lab -text \"Second Extra Section\"");
cmd(inter, "frame .w.s.e2.c");
cmd(inter, "checkbutton .w.s.e2.c.c -text \"Include Section 2\" -variable es2 -command {if {$es2==1} {.w.s.e2.header.tit conf -state normal; .w.s.e2.file.tit conf -state normal} {.w.s.e2.header.tit conf -state disabled; .w.s.e2.file.tit conf -state disabled}}");
cmd(inter, "set html2 0");
cmd(inter, "checkbutton .w.s.e2.c.h -text \"Accept HTML formatting tags\" -variable html2");
cmd(inter, "pack .w.s.e2.c.c .w.s.e2.c.h -side left -fill x");

cmd(inter, "frame .w.s.e2.header");
cmd(inter, "label .w.s.e2.header.tlab -text \"Title for Section 2 \"");
cmd(inter, "set tit2 Comments");
cmd(inter, "entry .w.s.e2.header.tit -width 50 -state disabled -textvariable tit2");
cmd(inter, "pack .w.s.e2.header.tlab .w.s.e2.header.tit -side left");

cmd(inter, "frame .w.s.e2.file");
cmd(inter, "label .w.s.e2.file.tlab -text \"File Name\"");
cmd(inter, "set file2 comments.txt");
cmd(inter, "entry .w.s.e2.file.tit -width 50 -state disabled -textvariable file2");
cmd(inter, "button .w.s.e2.file.new -text \"Search File\" -command {set file2 [tk_getOpenFile -filetypes {{{All Files} {*}} }]}");
cmd(inter, "pack .w.s.e2.file.tlab .w.s.e2.file.tit .w.s.e2.file.new -anchor w -side left");

cmd(inter, "pack .w.s.e2.lab .w.s.e2.c .w.s.e2.header .w.s.e2.file -anchor w ");

Tcl_LinkVar(inter, "es1", (char *) &es1, TCL_LINK_INT);
Tcl_LinkVar(inter, "es2", (char *) &es2, TCL_LINK_INT);






cmd(inter, "pack .w.s.lab .w.s.e1 .w.s.e2");

cmd(inter, "pack .w.l1 .w.f .w.l .w.s .w.b");
cmd(inter, "focus -force .w.b.b1");
sprintf(msg, "set code %d", code);
cmd(inter, msg);
sprintf(msg, "set init %d", init);
cmd(inter, msg);

Tcl_LinkVar(inter, "code", (char *) &code, TCL_LINK_INT);
Tcl_LinkVar(inter, "init", (char *) &init, TCL_LINK_INT);

cmd(inter, "focus -force .w.b.b1");
cmd(inter, "bind .w <Return> {.w.b.b1 invoke}");
cmd(inter, "bind .w <Escape> {.w.b.b3 invoke}");

here_create_report:
  while(*choice==0)
   Tcl_DoOneEvent(0);

Tcl_UnlinkVar(inter, "code");
Tcl_UnlinkVar(inter, "init");
Tcl_UnlinkVar(inter, "es1");
Tcl_UnlinkVar(inter, "es2");

cmd(inter, "destroy .w");
  if(*choice==2)
   { app=(char *)Tcl_GetVar(inter, "res",0);
     strcpy(name_rep, app);
     if(strlen(name_rep)==0)
      {
       cmd(inter, "wm deiconify .");
       return;
      }
     frep=create_frames(name_rep);
   }
  else
   if(*choice==1)
     frep=create_frames(name_rep);
   else
    {cmd(inter, "wm deiconify .");
     return;
    }



start:
if( (ffun=fopen(equation_name,"r"))==NULL)
 {*choice=0;
  cmd(inter, "toplevel .warn_eq");
  cmd(inter, "label .warn_eq.lab1 -text \"Equation file\"");
  sprintf(msg, "label .warn_eq.lab2 -text \"%s\" -foreground red", equation_name);
  cmd(inter, msg);
  cmd(inter, "label .warn_eq.lab3 -text \"not found\"");
  cmd(inter, "pack .warn_eq.lab1 .warn_eq.lab2 .warn_eq.lab3");
  cmd(inter, "frame .warn_eq.b");
  cmd(inter, "button .warn_eq.b.s -text Search -command {set res [file tail [tk_getOpenFile -filetypes {{{Lsd Equation Files} {.cpp}} {{All Files} {*}} }]]; set choice 1}");
  cmd(inter, "button .warn_eq.b.c -text Cancel -command {set choice 2}");
  cmd(inter, "pack .warn_eq.b.s .warn_eq.b.c -side left");
  cmd(inter, "pack .warn_eq.b");
  while(*choice==0)
   Tcl_DoOneEvent(0);
  cmd(inter, "destroy .warn_eq");

if(*choice==1)
 {
 app=(char *)Tcl_GetVar(inter, "res",0);
 strcpy(msg, app);
 if(strlen(msg)>0)
   {
 delete[] equation_name;
 equation_name=new char[strlen(msg)+1];
 strcpy(equation_name, msg);
   }

  }
if(*choice==2)
 return;



 goto start;
}

cmd(inter, "set choice $ltext");
ltext=*choice;
cmd(inter, "set choice $lmenu");
lmenu=*choice;


fprintf(frep, "<HTML>\n<HEAD> <META NAME=\"Author\" CONTENT=\"Automatically Generated by Lsd - Laboratory for Simulation Development, copyright by Marco Valente\">\n");
fprintf(frep, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\"></HEAD> <BODY>");

app=(char *)Tcl_GetVar(inter, "reptit",0);

sprintf(msg, "<TITLE>Lsd Report - Model: \"%s\"</TITLE>",app);
fprintf(frep, "%s",msg);


plog("\nWriting report. Wait...\n");
cmd(inter, "wm iconify .");


fprintf(frep, "<BR><I><A NAME=\"_DESCRIPTION_\">Automatically generated LSD report.</A></I><BR>");

sprintf(msg, "<BR><CENTER><B><FONT SIZE=+2>Model: <U>%s</U></FONT></B></CENTER><BR>\n<BR>\n", app);
fprintf(frep, "s", msg);

count=0;
if(es1==1)
 {app=(char *)Tcl_GetVar(inter, "file1",0);
  if(app==NULL)
   plog("Missing file name for first extra section. Skipped\n");
  else
  {
  f=fopen(app, "r");

  if(f!=NULL)
  {
  app=(char *)Tcl_GetVar(inter, "tit1",0);
  sprintf(msg, "<BR><I><U>%s</U></I><BR>", app);
  fprintf(frep, "%s",msg);
  cmd(inter, "set choice $html1");
  for(ch=fgetc(f); ch!=EOF; ch=fgetc(f) )
   {
   if(*choice==0)
   {
    switch(ch)
     {
      case '\n':
          fprintf(frep, "<br>\n");
          break;
      case '<':
          fprintf(frep, "&lt;");
          break;
      case '>':
          fprintf(frep, "&gt;");
          break;
      default:
          fprintf(frep, "%c", ch);
          break;
      }
     } 
     else
      fprintf(frep, "%c", ch); 
     } 
     
  fclose(f);
  fprintf(frep, "<BR>");
  }
  else
   {
   fprintf(frep, "<BR><I><U><B>Description</B></U></I><BR>");
   fprintf(frep, "No description file available.<BR>\n");
   sprintf(msg, "File %s not found. First extra section skipped\n", app);
   plog(msg);
   }
  }
 }
if(es2==1)
 {app=(char *)Tcl_GetVar(inter, "file2",0);
  if(app==NULL)
   plog("Missing file name for second extra section. Skipped\n");
  else
  {
  f=fopen(app, "r");
  if(f!=NULL)
  {
  app=(char *)Tcl_GetVar(inter, "tit2",0);
  sprintf(msg, "<BR><I><U>%s</U></I><BR>", app);
  fprintf(frep, "%s",msg);

  cmd(inter, "set choice $html2");
  for(ch=fgetc(f); ch!=EOF; ch=fgetc(f) )
   {
   if(*choice==0)
   {
    switch(ch)
     {
      case '\n':
          fprintf(frep, "<br>\n");
          break;
      case '<':
          fprintf(frep, "&lt;");
          break;
      case '>':
          fprintf(frep, "&gt;");
          break;
      default:
          fprintf(frep, "%c", ch);
          break;
      }
     } 
     else
      fprintf(frep, "%c", ch); 
    } 
   
  fclose(f);
  fprintf(frep, "<BR>");
  }
  else
   {sprintf(msg, "File %s not found. Second extra section skipped\n", app);
   plog(msg);
   }

  }
 }

insert_docuoptions(frep, r);
 fprintf(frep, "<P>&nbsp;\n<HR WIDTH=\"100%%\">");
fprintf(frep, "<BR><A NAME=\"_MODEL_STRUCTURE_\"><H1>Summary</I></H1></A><BR>\n");
fprintf(frep, "<I><U>Object Structure</I></U><BR>\n");

write_str(r, frep, count, "");
write_list(frep, r, 1, "");
fprintf(frep, "<BR><I><U>Equation File</I></U>: &nbsp;");
sprintf(msg, "set app [file tail \"%s\"]", equation_name);
cmd(inter, msg);
app=(char *)Tcl_GetVar(inter, "app",0);
strcpy(msg, app);
fprintf(frep, "<B>%s</B><BR><BR>\n", msg);



createmodelhelp(choice, r);

fprintf(frep, "<BR><A NAME=\"_DETAILS_\"><H1>Detailed Description</I></H1></A><BR>\n");
count=0;
fprintf(frep, "<I><U>Object Structure</I></U><BR>\n");
write_str(r, frep, count, "_d_");
write_list(frep, r, 1, "_d_");
write_obj(r, frep);

fprintf(frep,"</BODY> </HTML>");
fclose(frep);


sprintf(msg, "Finished. Report in file: \n%s\n", name_rep);
plog(msg);
cmd(inter, "wm deiconify .");
sprintf(msg, "set namerep %s", name_rep);
cmd(inter, msg);
cmd(inter, "LsdHtml $namerep");

}

/**********************************
WRITE_OBJ
**********************************/
void write_obj(object *r, FILE *frep)
{
int count, i;

object *cur, *cur2;
variable *cv;
bridge *cb;

for(cur=r; cur!=NULL; cur=skip_next_obj(cur, &count ) )
 {
 fprintf(frep, "<P>&nbsp;\n<HR WIDTH=\"100%%\">");
 fprintf(frep, "<H2><b>Object <A NAME=\"_d_%s\"></b></A><B><U><FONT FACE=\"Arial\"><FONT SIZE=+1>%s</FONT></FONT></U></B></H2>\n", cur->label, cur->label);
 if(init==1)
  {for(i=0, cur2=cur; cur2!=NULL; )
   { count=1;
     while(cur2->next!=NULL && !strcmp(cur2->label, (cur2->next)->label) )
       {count++;
        cur2=cur2->next;
       }
     fprintf( frep, "%d ", count);
     cur2=cur2->hyper_next(cur2->label);
     i++;
     if(i>MAX_INIT)
      {fprintf(frep, "(more)");
       cur2=NULL;
      }
   }

  }
 if(cur->up!=NULL)
 {fprintf(frep,"<BR>\n<I>Contained in Object:</I>&nbsp");
  ancestors(cur, frep);
 }
fprintf(frep,"<BR><I>Containing:</I>&nbsp");
//for(cur2=cur->son; cur2!=NULL ; cur2=skip_next_obj(cur2, &count) )
for(cb=cur->b; cb!=NULL; cb=cb->next )
 {
 cur2=cb->head;
 fprintf(frep, "<A HREF=\"#%s\">%s</A> ", cur2->label, cur2->label);
 }
if(cur->b==NULL)
 fprintf(frep, "(none)");
fprintf(frep, "<BR>");
write_list(frep, cur, 0, "_d_");
  for(cv=cur->v; cv!=NULL; cv=cv->next)
    write_var(cv, frep);
  if(cur->b!=NULL)
   {cur2=cur->b->head;
    write_obj(cur2, frep);
   }

 }
}

/******************************
WRITE_VAR
*******************************/
void write_var(variable *v, FILE *frep)
{
FILE *ffun ;
char c1_lab[2000], c2_lab[2000], c3_lab[2000], *app;
int done, i, one, j,flag_begin, flag_string, flag_comm, flag_var;
object *cur, *cur2;

cmd(inter, "update");
if(v->param==1)
  fprintf(frep, "<P>\n<A NAME=\"_d_%s\"><u><I>Parameter</I></u><B> %s</B></A>", v->label, v->label);
if(v->param==0)
  fprintf(frep, "<P>\n<A NAME=\"_d_%s\"><u><I>Variable</U></I></u><B> %s</B></A>", v->label, v->label);
if(v->param==2)
  fprintf(frep, "<P>\n<A NAME=\"_d_%s\"><u><I>Function</U></I></u><B> %s</B></A>", v->label, v->label);

fprintf(frep, "<br>\n<i>In Object</i>&nbsp; <A HREF=\"#%s\">%s</A> ",v->up->label,v->up->label);
 
fprintf(frep, "<BR>\n<I>Used in:</I>&nbsp;");

if( (ffun=fopen(equation_name,"r"))==NULL)
 return;

strcpy(c1_lab, "");
strcpy(c2_lab, "");

for(one=0 ; fgets(c1_lab, 1999, ffun)!=NULL;  )
 {

  if(is_equation_header(c1_lab,c2_lab)==1)
	{
    done=0;
    done=contains(ffun, v->label, strlen(v->label));

    if(done==1)
     {fprintf(frep, "<A HREF=\"#%s\">%s</A> ", c2_lab, c2_lab);
      one=1;
     }

	}
 }
if(one==0)
 fprintf(frep, "(never used)");

fprintf(frep, "<BR>\n");

fclose(ffun);

if(v->param==1)
 {
  fprintf(frep,  "Go to: <A HREF=\"#%s\">Description</A>,&nbsp; <A HREF=\"#_i_%s\">Initial values</A>,&nbsp; <A HREF=\"#_MODEL_STRUCTURE_\">Model Structure</A><BR> ", v->label, v->label);
 return;
 }
if(v->param==0 || v->param==2)
 {
 fprintf(frep,"<I>Using:</I>&nbsp;");
 for(cur=v->up; cur->up!=NULL; cur=cur->up); //Reach the root
 fatto=0;
 find_using(cur, v, frep);
 if(fatto==0)
  fprintf(frep, "(nothing)");
 } 
if(v->num_lag==0)
  fprintf(frep,  "<br>\nGo to: <A HREF=\"#%s\">Description</A>,&nbsp; <A HREF=\"#_MODEL_STRUCTURE_\">Model Structure</A>", v->label);
else
  fprintf(frep,  "<br>\nGo to: <A HREF=\"#%s\">Description</A>,&nbsp; <A HREF=\"#_i_%s\">Initial values</A>,&nbsp; <A HREF=\"#_MODEL_STRUCTURE_\">Model Structure</A>", v->label, v->label);


fprintf(frep,"<BR>\n<I>Equation Code:</I>\n");

if( (ffun=fopen(equation_name,"r"))==NULL)
 return;
strcpy(c1_lab, "");
strcpy(c2_lab, "");
flag_comm=0;
for(one=0 ; fgets(c1_lab, 1999, ffun)!=NULL;  )
 {

  if(is_equation_header(c1_lab,c2_lab)==1)
	{
   if(macro==0)
     done=0;
   else
     done=1; //will never stop with {} only
    if(!strcmp(c2_lab, v->label))
    {fatto=0;
     fprintf(frep, "<BR>\n<TT>%s</TT>",c1_lab);
     
     while(done>0 || fatto==0)
      {fatto=1;
      fgets(c1_lab, 1999, ffun);
      app=strstr(c1_lab, "{");
      if(app!=NULL)
       done++;
      app=strstr(c1_lab, "}");
      if(app!=NULL)
       done--;
       flag_string=flag_var=0;
       fprintf(frep, "<BR><TT>");
       if(flag_comm==1)
        fprintf(frep, "<FONT COLOR=\"#009900\">");
       flag_begin=0;
      for(j=0; c1_lab[j]!=(char)NULL; j++)
       {
        if(flag_comm==0 && flag_string==0 && c1_lab[j]=='\"')
          {for(i=j+1; c1_lab[i]!='\"'; i++)
            c2_lab[i-j-1]=c1_lab[i];
           c2_lab[i-j-1]=(char)NULL;
           fprintf(frep, "\"<A HREF=\"#_d_%s\">", c2_lab);
           flag_string=1;
           j++;
          }
        if(flag_comm==0 && flag_string==1 && c1_lab[j]=='\"')
          {fprintf(frep, "</A>\"");
           flag_string=0;
           j++ ;
          }

        if(flag_comm==0 && c1_lab[j]=='/' && c1_lab[j+1]=='*')
          {fprintf(frep, "<FONT COLOR=\"#009900\">");
           flag_comm=1;
          }
        if(flag_comm==1 && c1_lab[j]=='/' && c1_lab[j-1]=='*')
          {fprintf(frep, "/</FONT>");
           flag_comm=0;
           j++;
          }
        if(flag_comm==0 && c1_lab[j]=='/' && c1_lab[j+1]=='/')
          {fprintf(frep, "<FONT COLOR=\"#009900\">");
           flag_comm=2;
          }
        if(flag_comm==0 && c1_lab[j]=='v' && c1_lab[j+1]=='[')
          {fprintf(frep, "<FONT COLOR=\"#FF0000\">");
           flag_var=1;
          }
        if(flag_comm==0 && flag_var==1 && c1_lab[j]==']')
          {fprintf(frep, "]</FONT>");
           flag_var=0;
           j++;
          }

        if(c1_lab[j]=='<' || c1_lab[j]=='>')
         {
          if(c1_lab[j]=='<')
            fprintf(frep, "&lt;");
          else
            fprintf(frep, "&gt;");
         }
       else
        if(c1_lab[j]==' ' && flag_begin==0)
         {fprintf(frep, "&nbsp;");

         }
        else
         if(c1_lab[j]!='\n')
          {fprintf(frep, "%c", c1_lab[j]);
           flag_begin=1;
          }
         else
         if(flag_comm==1)
          fprintf(frep, "</FONT>");
       }
      if(flag_comm==2)
       {fprintf(frep, "</FONT>");
        flag_comm=0;
       }

      fprintf(frep, "</TT>\n");
      
      clean_spaces(c1_lab);
      if(!strncmp(c1_lab, "RESULT(",7) && macro==1)
        done=0; //force it to stop
      }//end of the while()
      
      fprintf(frep, "</FONT><BR>\n");
      

     fclose(ffun);
     fprintf(frep, "<FONT SIZE=-2><A HREF=\"#_d_%s\">Return</A></FONT><BR>\n", v->label);
     return;
    }
	}
 }

}



/******************************
FIND_USING
*******************************/
void find_using(object *r, variable *v, FILE *frep)
{
object *cur;
variable *cv;
int count;
char c1_lab[2000], c2_lab[2000], c3_lab[2000], *app;
FILE *ffun;
int done, i, one;



for(cur=r; cur!=NULL; cur=skip_next_obj(cur, &count) )
{
 for(cv=cur->v; cv!=NULL; cv=cv->next)
  {if( (ffun=fopen(equation_name,"r"))==NULL)
    return;

    strcpy(c1_lab, "");
    strcpy(c2_lab, "");

    for(one=0 ; fgets(c1_lab, 1999, ffun)!=NULL;  )
     {


      if(is_equation_header(c1_lab,c2_lab)==1)
	    {
        done=0;
        if(!strcmp(c2_lab, v->label) )
         done=contains(ffun, cv->label, strlen(cv->label));
        if(done==1)
         {
          if(frep!=NULL)
            fprintf(frep, "<A HREF=\"#%s\">%s</A> ", cv->label, cv->label);
          else
            {sprintf(msg, "$list.l insert end %s",cv->label);
             cmd(inter, msg);
            } 
          fatto=1;
         }
       }
     }
   fclose(ffun);

  }

 if(cur->b!=NULL)
  find_using(cur->b->head, v, frep);
}

}

/******************************
INSERT_SUMMARY
*******************************/
void insert_summary(object *r, FILE *frep)
{
object *cur;
int count;
bridge *cb;

//for(cur=r; cur!=NULL; cur=skip_next_obj(cur, &count) )
for(cb=r->up->b; cb!=NULL; cb=cb->next )
 {
 cur=cb->head;
 fprintf(frep, "<OPTION VALUE=\"%s\">%s\n", cur->label, cur->label);
 if(cur->b!=NULL)
  insert_summary(cur->b->head, frep);
 }

}

/******************************
WRITE_STR
*******************************/
void write_str(object *r, FILE *frep, int dep, char const *prefix )
{
int len, i, j, trash;
object *cur, *cur2;
bridge *cb;
trash=0;
if(r->up!=NULL)
  {
  //if(r->up->son!=r)
  if(r->up->b->head!=r)
   for(i=0; i<dep; i++)
    {if(msg[i]==' ')
      fprintf(frep, "<TT>&nbsp;</TT>");
     else
      fprintf(frep, "<TT>|</TT>");
    }
  fprintf(frep, "<TT>-&gt</TT>");
  cur=skip_next_obj(r, &trash);
  if(cur!=NULL)
   msg[dep]='|';
  else
   msg[dep]=' ';
  msg[++dep]=' ';
  trash=1;
  }

fprintf(frep, "<TT><A HREF=\"#%s%s\">%s</A></TT>",prefix, r->label, r->label);
len=strlen(r->label);
for(i=0; i<len; i++)
  msg[dep+i+trash]=' ';
dep=dep+len+trash;
j=dep;
//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur, &trash) )
for(cb=r->b; cb!=NULL; cb=cb->next)
 {
 cur=cb->head;
 write_str(cur, frep, j, prefix);
 }
if(r->b==NULL)
 {
  fprintf(frep, "<BR>\n");
  for(i=0; i<dep; i++)
   {if(msg[i]==' ')
     fprintf(frep, "<TT>&nbsp;</TT>");
    else
     fprintf(frep, "<TT>|</TT>");
   }
  fprintf(frep, "<BR>\n");
 }


}

/********************************
WRITE_LIST
*********************************/
void write_list(FILE *frep, object *root, int flag_all, char const *prefix)
{
int num, i;
char *app, s1[200], s2[50];

Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
if(flag_all==1) //initial listing
  fprintf(frep, "<I><U>List of Variables</I></U>: &nbsp; <BR>\n");
else
  fprintf(frep, "<I>List of Variables:</I> &nbsp;");

cmd(inter, "lappend rawlist"); //create the list if not existed
cmd(inter, "unset rawlist"); //empty the list
cmd(inter, "lappend rawlist"); //create a surely empty list

if(!strcmp(prefix, "_i_") )
 fill_list_var(root, flag_all, 1); //insert only lagged variables
else
 fill_list_var(root, flag_all, 0); //insert all the variables

cmd(inter, "set alphalist [lsort -dictionary $rawlist]");
cmd(inter, "set num [llength $alphalist]");
if(flag_all==0) //distinguish the case you are compiling the initial list of element (all) or for a single Object)
  sprintf(s1, "form_v_%s_%s",root->label, prefix);
else
  sprintf(s1, "form_v_all_%s_%s",root->label, prefix);  
if(lmenu==1 || (lmenu==0 && ltext==0) )
 create_form(num, s1, prefix); 
if(ltext==1)
{
for(i=0; i<num; i++)
 {sprintf(msg, "set app [lindex $alphalist %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "app",0);
  strcpy(msg, app);
  sscanf(msg, "%s %s", s1, s2);
  fprintf(frep, "<A HREF=\"#%s%s\">%s</A>%s",prefix, s1, s1, s2);
  if(i<num-1)
   fprintf(frep, ", ");
 }
if(num==0)
 fprintf(frep, "(no Variables)");
}
if(flag_all==1) //initial listing
 fprintf(frep, "<BR><BR><I><U>List of Parameters</I></U>: &nbsp; <BR>\n");
else
 fprintf(frep, "<BR><I>List of Parameters:</I> &nbsp;");

cmd(inter, "lappend rawlist"); //create the list if not existed
cmd(inter, "unset rawlist"); //empty the list
cmd(inter, "lappend rawlist"); //create a surely empty list

fill_list_par(root, flag_all);

cmd(inter, "set alphalist [lsort -dictionary $rawlist]");
cmd(inter, "set num [llength $alphalist]");
if(flag_all==0) //distinguish the case you are compiling the initial list of element (all) or for a single Object)
 sprintf(s1, "form_p_%s_%s",root->label, prefix);
else
 sprintf(s1, "form_p_all_%s_%s",root->label, prefix); 
if(lmenu==1 || (lmenu==0 && ltext==0) )
  create_form(num, s1, prefix); 

if(ltext==1)
{
for(i=0; i<num; i++)
 {sprintf(msg, "set app [lindex $alphalist %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "app",0);
  strcpy(msg, app);
  fprintf(frep, "<A HREF=\"#%s%s\">%s</A>",prefix, msg, msg);
  if(i<num-1)
   fprintf(frep, ", ");
 }

if(num==0)
 fprintf(frep, "(no Parameters)");
else
 cmd(inter, "unset app");
}
Tcl_UnlinkVar(inter, "num");

fprintf(frep, "<BR>\n");


}

/********************************
FILL_LIST_VAR
*********************************/
void fill_list_var(object *r, int flag_all, int flag_init)
{
object *cur;
variable *cv;
int count;
bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
 {if( (cv->param==0 || cv->param==2) && (flag_init==0 || cv->num_lag>0))
   {sprintf(msg, "lappend rawlist \"%s (%d)\"", cv->label, cv->num_lag);
    cmd(inter, msg);
   }

 }
if(flag_all==0)
 return;
//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur, &count ))
for(cb=r->b; cb!=NULL; cb=cb->next)
 fill_list_var(cb->head, flag_all, flag_init);
}

/********************************
FILL_LIST_PAR
*********************************/
void fill_list_par(object *r, int flag_all)
{
object *cur;
variable *cv;
int count;
bridge *cb;

for(cv=r->v; cv!=NULL; cv=cv->next)
 {if(cv->param==1)
   {sprintf(msg, "lappend rawlist \"%s\"", cv->label);
    cmd(inter, msg);
   }

 }
if(flag_all==0)
 return;
//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur, &count ))
for(cb=r->b; cb!=NULL; cb=cb->next)
 fill_list_par(cb->head, flag_all);
}


/******************************
CREATEMODELHELP
Create a help file specific for the model
*******************************/

void createmodelhelp(int *choice, object *r)
{
int count;

create_table_init(r);

fprintf(frep, "<HR WIDTH=\"100%%\">");
count=0;
fprintf(frep, "<BR><A NAME=\"_INITIALVALUES_\"><H1>Initial Values</I></H1></A><BR>\n");
fprintf(frep, "<I><U>Object Structure</I></U><BR>\n");

write_str(r, frep, count, "_i_");
write_list(frep, r, 1, "_i_");

create_initial_values(r);

}


/*
Create recursively the help table for an Object
*/
void create_table_init(object *r)
{
int count=0, i;
object *cur;
variable *curv;
description *cur_descr;
bridge *cb;

 fprintf(frep, "<P>&nbsp;\n<HR WIDTH=\"100%%\">");
 fprintf(frep, "<BR><FONT SIZE=+2><b><a NAME=\"%s\">Object %s</a></b></FONT>", r->label, r->label);
 if(r->up!=NULL)
  fprintf(frep, "<BR><i>Contained in Object: &nbsp;</i>"); 
 ancestors(r, frep);
 if(r->b!=NULL)
  {fprintf(frep, "<BR><i>Containing Objects: &nbsp;</i> ");
   fprintf(frep, " <a HREF=\"#%s\">%s</a>",r->b->blabel,r->b->blabel);
   //cur=skip_next_obj(r->son, &i );
   
   //for(; cur!=NULL; cur=skip_next_obj(cur, &i ))
   for(cb=r->b->next ; cb!=NULL; cb=cb->next)
    fprintf(frep, ", <a HREF=\"#%s\">%s</a>",cb->blabel, cb->blabel);
  }
fprintf(frep, "<BR>\n");
write_list(frep, r, 0, "");

cur_descr=search_description(r->label);
if(cur_descr==NULL)
  {
   add_description(r->label, "Object", "(no description available)");
   sprintf(msg, "Warning! description for '%s' not found. New one created.\\n", r->label);
   plog(msg);
   cur_descr=search_description(r->label);
  } 

if(cur_descr !=NULL && cur_descr->text !=NULL && strcmp(cur_descr->text,"(no description available)") )
 {
 fprintf(frep, "<u><i>Description</i></u><br>\n");
   for(i=0; cur_descr->text[i]!=(char)NULL; i++)
    {
    switch(cur_descr->text[i])
     {
      case '\n':
         fprintf(frep, "<br>\n");
         break;
      case '<':
         fprintf(frep, "&lt;");
         break;
      case '>':
         fprintf(frep, "&gt;");
         break;

      default:
         fprintf(frep, "%c", cur_descr->text[i]);
         break;
     }
    }
 }
else
  fprintf(frep, "&nbsp; ");
fprintf(frep, "<BR><BR>");
if(r->v!=NULL)
{
 fprintf(frep, "<table BORDER>");
 fprintf(frep, "<tr>");
 fprintf(frep, "<td><center><b>Object</b></center></td>");
 fprintf(frep, "<td><center><b>Label</b></center></td>\n");
 fprintf(frep, "<td><center><b>Comment</b></center></td>\n");
 fprintf(frep, "</tr>");

 for(curv=r->v; curv!=NULL; curv=curv->next)
 {

   fprintf(frep, "<tr VALIGN=TOP>");

   fprintf(frep, "<td><A HREF=\"#%s\">%s</A>&nbsp;</td>\n",r->label,r->label);
   if(curv->param==1)
     fprintf(frep, "<td><a NAME=\"%s\"><A HREF=\"#_d_%s\">%s</A>&nbsp(P)&nbsp;</a></td>\n",curv->label,curv->label,curv->label);
   else
     fprintf(frep, "<td><a NAME=\"%s\"><A HREF=\"#_d_%s\">%s</A>&nbsp(%d)&nbsp;</a></td>\n",curv->label,curv->label,curv->label, curv->num_lag);
   cur_descr=search_description(curv->label);
   if(cur_descr==NULL)
  {if(curv->param==0)
     add_description(curv->label, "Variable", "(no description available)");
   if(curv->param==1)
     add_description(curv->label, "Parameter", "(no description available)");  
   if(curv->param==2)
     add_description(curv->label, "Function", "(no description available)");  
   sprintf(msg, "Warning! description for '%s' not found. New one created.\\n", cur->label);
   plog(msg);
   cur_descr=search_description(curv->label);
  } 

   fprintf(frep, "<td> ");
if(cur_descr->text !=NULL && strcmp(cur_descr->text,"(no description available)") )
 {   
   for(i=0; cur_descr->text[i]!=(char)NULL; i++)
    {
    switch(cur_descr->text[i])
     {
      case '\n':
         fprintf(frep, "<br>\n");
         break;
      case '<':
         fprintf(frep, "&lt;");
         break;
      case '>':
         fprintf(frep, "&gt;");
         break;

      default:
         fprintf(frep, "%c", cur_descr->text[i]);
         break;
     }
    }
    fprintf(frep, "<br>\n");
  }
   if(curv->param==1 || curv->num_lag>0)
    {
    fprintf(frep, "\n<A HREF=\"#_i_%s\">Init. values</A>\n", curv->label);
    if(cur_descr->init!=NULL)
     fprintf(frep, "&nbsp; %s</td>\n", cur_descr->init);
    else
      fprintf(frep, "&nbsp;</td>\n");
    }
   fprintf(frep, "</tr>");
 
 }
 fprintf(frep, "</table>");
}//end if exist a var

//cur= skip_next_obj(r->son, &count);
//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur, &count))
for(cb=r->b; cb!=NULL; cb=cb->next)
   create_table_init(cb->head);

}

/*
Create recursively the help table for the initial values of an Object
*/
void create_initial_values(object *r)
{
int count=0, i;
object *cur, *cur1;
variable *curv, *cv;
description *cur_descr;
bridge *cb;

 for(curv=r->v; curv!=NULL; curv=curv->next)
  if(curv->param==1 || curv->num_lag>0)
    count=1;

if(count==0)
 {//no need for initialization, typically root
  //for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur, &count))
  for(cb=r->b; cb!=NULL; cb=cb->next)
   create_initial_values(cb->head);
  return;
 }  
 
for(count=0,cur=r; cur!=NULL;cur=cur->hyper_next(cur->label) )
  count++;

 fprintf(frep, "<HR WIDTH=\"100%%\">");
 fprintf(frep, "<BR><FONT SIZE=+2><b><a NAME=\"_i_%s\">Object %s</a></b></FONT> Total instances = %d", r->label, r->label, count);
 fprintf(frep, "<table BORDER>");
 fprintf(frep, "<tr>");
 fprintf(frep, "<td><center><b>Label</b></center></td>\n");
 fprintf(frep, "<td><center><b>Initial values</b></center></td>\n");
 fprintf(frep, "</tr>");

 for(curv=r->v; curv!=NULL; curv=curv->next)
 {

   if(curv->param==1)
    {
      fprintf(frep, "<tr VALIGN=TOP>");
      fprintf(frep, "<td><a NAME=\"_i_%s\"><A HREF=\"#%s\">%s</A>&nbsp(P)&nbsp;</a></td>\n",curv->label,curv->label,curv->label);
      fprintf(frep, "<td>%g", curv->val[0]);
      for(cur=r->hyper_next(r->label); cur!=NULL; cur=cur->hyper_next(cur->label))
       {
        cv=cur->search_var(cur, curv->label);
        fprintf(frep, " %g", cv->val[0]);
       }
      fprintf(frep, "</td>\n");
      fprintf(frep, "</tr>");
 
    }   
   else
    {
    for(i=0; i<curv->num_lag; i++)
       {
       fprintf(frep, "<tr VALIGN=TOP>");
       fprintf(frep, "<td><a NAME=\"_i_%s\"><A HREF=\"#%s\">%s</A> &nbsp Lag(%d) &nbsp; </a></td>\n",curv->label,curv->label,curv->label, i+1);
       fprintf(frep, "<td>%g", curv->val[i]);
       for(cur=r->hyper_next(r->label); cur!=NULL; cur=cur->hyper_next(cur->label))
        {
         cv=cur->search_var(cur, curv->label);
         fprintf(frep, " %g", cv->val[i]);
        }
      fprintf(frep, "</td>\n");
      fprintf(frep, "</tr>");
        
       }
     }

 }
 fprintf(frep, "</table>");

//cur= skip_next_obj(r->son, &count);
//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur, &count))
for(cb=r->b; cb!=NULL; cb=cb->next)
   create_initial_values(cb->head);

}


/*
Squeeze the spaces out of line and returns 1 if the line is an equation header,
placing the Variable label in Var
*/
int is_equation_header(char *line, char *var)
{
int i, j;
clean_spaces(line);
if(!strncmp(line, "if(!strcmp(label,",17)|| !strncmp(line, "EQUATION(",9) || !strncmp(line, "FUNCTION(",9))
 {
  if(!strncmp(line, "if(!strcmp(label,",17))
   macro=0;
  else
   macro=1;
   for(i=0; line[i]!='"'; i++);
   for(j=++i; line[i]!='"'; i++)
    var[i-j]=line[i];
   var[i-j]=(char)NULL;
   return 1;
 }

return 0;
}


void ancestors(object *r, FILE *f)
{
if(r->up!=NULL)
 {ancestors(r->up, f);
  if(r->up->up==NULL)
    fprintf(f, "<A HREF=\"#%s\">%s</A>", r->up->label, r->up->label);
  else
  fprintf(f, "-&gt;<A HREF=\"#%s\">%s</A>", r->up->label, r->up->label);
 } 
  
}

FILE *create_frames(char *t)
{
FILE *f;

f=fopen(t, "w");
fprintf(f, "<html> <head> <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\"> <meta name=\"AUTHOR\" content=\"Marco Valente\"> </head>\n");
fprintf(f, "<FRAMESET framespacing=0 border=1 rows=\"30,*\"> <FRAME NAME=\"testa\" SRC=\"head_%s\"",t);

fprintf(f, "frameborder=0 MARGINHEIGHT=\"0\" MARGINWIDTH=\"0\" SCROLLING=\"NO\" NORESIZE> <FRAME NAME=\"body\" SRC=\"body_%s\" frameborder=0 MARGINHEIGHT=\"0\" MARGINWIDTH=\"0\" SCROLLING=\"AUTO\"> </FRAMESET> <NOFRAMES>  <p>Use a frame-enabled browser</p> </NOFRAMES> </html>",t);
fclose(f);
sprintf(msg, "head_%s",t);
f=fopen(msg,"w");
fprintf(f, "<html> <head> </head > <body bgcolor=\"#D6A9FE\"> <center>");
fprintf(f, "<a href=\"body_%s#_DESCRIPTION_\" target=body>Description</a> &nbsp", t);
fprintf(f, "<a href=\"body_%s#_MODEL_STRUCTURE_\" target=body>Summary</a> &nbsp", t);
fprintf(f, "<a href=\"body_%s#_INITIALVALUES_\" target=body>Initial Values</a> &nbsp", t);
fprintf(f, "<a href=\"body_%s#_DETAILS_\" target=body>Details</a> &nbsp", t);

fprintf(f, "</body> </html>");
fclose(f);

sprintf(msg, "body_%s",t);
f=fopen(msg,"w");
return(f);

}


/************
CREATE LIST FORMS FOR LABELS. 
**********/
void create_form(int num, char const *title, char const *prefix)
{
int i;
char s1[200],s2[200], *app;

if(num==0)
 return;


sprintf(msg, "<form name=\"%s\" method=\"POST\">\n",title);
fprintf(frep,"%s",msg);


  
sprintf(msg, "\n		<select name=\"entry\" class=\"form\">\n");
fprintf(frep,"%s", msg);
			


for(i=0; i<num; i++)
 {sprintf(msg, "set app [lindex $alphalist %d]",i);
  cmd(inter, msg);
  app=(char *)Tcl_GetVar(inter, "app",0);
  strcpy(msg, app);
  sscanf(msg, "%s %s", s1, s2);
  fprintf(frep, "\n<option value=\"#%s%s\">%s",prefix,s1,s1);
 }
fprintf(frep, "\n</select>\n");
fprintf(frep, "<INPUT TYPE=button VALUE=\"Go\" onClick=\"location.href = document.%s.entry.options[document.%s.entry.selectedIndex].value\">\n",title,title);
fprintf(frep, "</form>");
}

/*********/

/****************************************************
SHOWREP_OBSERVE
****************************************************/
void showrep_observe(FILE *f, object *n, int *begin)
{
variable *cv, *cv1;
object *co;
description *cd;
int app, i;
bridge *cb;

for(cv=n->v; cv!=NULL; cv=cv->next)
 {
 cd=search_description(cv->label);
  if(cd==NULL)
  {if(cv->param==0)
     add_description(cv->label, "Variable", "(no description available)");
   if(cv->param==1)
     add_description(cv->label, "Parameter", "(no description available)");  
   if(cv->param==2)
     add_description(cv->label, "Function", "(no description available)");  
   sprintf(msg, "Warning! description for '%s' not found. New one created.\\n", cv->label);
   plog(msg);
   cd=search_description(cv->label);
  } 

 if(cd->observe=='y')
  {
  if(*begin==1)
   {
    *begin=0;
    fprintf(f,"<B><U>Results</U></B><BR>\nElements relevant to observe.<BR>\n");
    fprintf(f, "<table BORDER>");
    fprintf(f, "<tr>");
    fprintf(f, "<td><center><b>Object</b></center></td>");
    fprintf(f, "<td><center><b>Label</b></center></td>\n");
    fprintf(f, "<td><center><b>Description</b></center></td>\n");    
    fprintf(f, "</tr>");
   }
    fprintf(f, "<tr>");
    fprintf(f, "<td><A HREF=\"#%s\">%s</A></td>",n->label, n->label);
  if(cv->param==1)
   fprintf(f, "<td><B>Param:</B> <A HREF=\"#%s\">%s</A></td>", cv->label, cv->label);
  if(cv->param==0)
   fprintf(f, "<td><B>Var:</B> <A HREF=\"#%s\">%s</A></td>", cv->label, cv->label);
  if(cv->param==2)
   fprintf(f, "<td><B>Function:</B> <A HREF=\"#%s\">%s</A></td>", cv->label, cv->label);
    
    fprintf(f, "<td>"); 

    for(i=0; cd->text[i]!=(char)NULL; i++)
    {
    switch(cd->text[i])
     {
      case '\n':
         fprintf(f, "<br>\n");
         break;
      case '<':
         fprintf(f, "&lt;");
         break;
      case '>':
         fprintf(f, "&gt;");
         break;

      default:
         fprintf(f, "%c", cd->text[i]);
         break;
     }
    }
    fprintf(f, "</td></tr>\n");    

   
   
/*
   if(cv->param==1)
    fprintf(f, "<li><B>Param:</B> <A HREF=\"#%s\">%s</A> </li>\n", cv->label, cv->label);
   else
    fprintf(f, "<li><B>Var.</B>: <A HREF=\"#%s\">%s</A> </li>\n", cv->label, cv->label);
*/

  }
 }

//for(co=n->son; co!=NULL; co=skip_next_obj(co, &app))
for(cb=n->b; cb!=NULL; cb=cb->next)
 showrep_observe(f, cb->head, begin);
if(n->up==NULL)
  fprintf(f, "</table>");
}

/****************************************************
SHOWREP_INITIAL
****************************************************/
void showrep_initial(FILE *f, object *n, int *begin)
{
variable *cv, *cv1;
object *co;
description *cd;
int app, i;
bridge *cb;

for(cv=n->v; cv!=NULL; cv=cv->next)
 {
 cd=search_description(cv->label);
  if(cd==NULL)
  {if(cv->param==0)
     add_description(cv->label, "Variable", "(no description available)");
   if(cv->param==1)
     add_description(cv->label, "Parameter", "(no description available)");  
   if(cv->param==2)
     add_description(cv->label, "Function", "(no description available)");  
   sprintf(msg, "Warning! description for '%s' not found. New one created.\\n", cv->label);
   plog(msg);
   cd=search_description(cv->label);
  } 

 if(cd->initial=='y')
  {
  if(*begin==1)
   {
    *begin=0;
    fprintf(f,"<B><U>Initialization</U></B><BR>\nElements relevant to initialize.<BR>\n");
    fprintf(f, "<table BORDER>");
    fprintf(f, "<tr>");
    fprintf(f, "<td><center><b>Object</b></center></td>");
    fprintf(f, "<td><center><b>Label</b></center></td>\n");
    fprintf(f, "<td><center><b>Description</b></center></td>\n");    
//    fprintf(f, "<td><center><b>Data</b></center></td>\n");    
    fprintf(f, "</tr>");
   }
    fprintf(f, "<tr>");
    fprintf(f, "<td><A HREF=\"#%s\">%s</A></td>",n->label, n->label);
  if(cv->param==1)
   fprintf(f, "<td><B>Param:</B> <A HREF=\"#%s\">%s</A></td>", cv->label, cv->label);
  if(cv->param==0)
   fprintf(f, "<td><B>Var:</B> <A HREF=\"#%s\">%s</A></td>", cv->label, cv->label);
  if(cv->param==2)
   fprintf(f, "<td><B>Function:</B> <A HREF=\"#%s\">%s</A></td>", cv->label, cv->label);
    
    fprintf(f, "<td>"); 

    for(i=0; cd->text[i]!=(char)NULL; i++)
    {
    switch(cd->text[i])
     {
      case '\n':
         fprintf(f, "<br>\n");
         break;
      case '<':
         fprintf(f, "&lt;");
         break;
      case '>':
         fprintf(f, "&gt;");
         break;

      default:
         fprintf(f, "%c", cd->text[i]);
         break;
     }
    }
    if(strlen(cd->init)>0)
     fprintf(f,"<br><b>Initialization function:</b>");
    for(i=0; cd->init[i]!=(char)NULL; i++)
    {
    switch(cd->init[i])
     {
      case '\n':
         fprintf(f, "<br>\n");
         break;
      case '<':
         fprintf(f, "&lt;");
         break;
      case '>':
         fprintf(f, "&gt;");
         break;

      default:
         fprintf(f, "%c", cd->init[i]);
         break;
     }
    }
    /*
    fprintf(f, "</td><td>"); 
    for(co=n; co!=NULL; co=co->hyper_next(co->label) )
     {
       cv1=co->search_var(co, cv->label);
       fprintf(f, " %g", cv1->val[0]);
     }
    */
    fprintf(f, " <A HREF=\"#_i_%s\">See initial values</A>,</td>",  cv->label);
    fprintf(f, "</td></tr>\n");    

   
   
/*
   if(cv->param==1)
    fprintf(f, "<li><B>Param:</B> <A HREF=\"#%s\">%s</A> </li>\n", cv->label, cv->label);
   else
    fprintf(f, "<li><B>Var.</B>: <A HREF=\"#%s\">%s</A> </li>\n", cv->label, cv->label);
*/

  }
 }

//for(co=n->son; co!=NULL; co=skip_next_obj(co, &app))
for(cb=n->b; cb!=NULL; cb=cb->next)
 showrep_initial(f, cb->head, begin);
if(n->up==NULL)
  fprintf(f, "</table>");
 
}

void insert_docuoptions(FILE *frep, object *r)
{

int begin=1;


showrep_initial(frep, r, &begin);
fprintf(frep, "</ul><BR>\n");

begin=1;
showrep_observe(frep, r, &begin);
fprintf(frep, "</ul><BR>\n");

}


void tex_report(object *r, FILE *f)
{
variable *cv;
description *cd;
object *cur;
int count;
bridge *cb;

if(r->up==NULL)
  fprintf(f, "\\section{List of all elements}\n");

fprintf(f, "\\subsection{Object \\textbf{%s}}\n\n", r->label);
cd=search_description(r->label);
if(strcmp(cd->text, "(no description available )"))
 {
 fprintf(f, "Description: %s\n\n\n", cd->text);
 }
fprintf(f, "\\begin{longtable}{||p{3cm}|p{11cm}||}\n  \\hline\n  \\textbf{Label} & \\textbf{Description} \\\\  \\hline \\endhead \n");

for(cv=r->v; cv!=NULL; cv=cv->next)
 {
  cd=search_description(cv->label);
  fprintf(f, "\\lsd{%s} &", cv->label);
  if(cv->param==0)
   {
    fprintf(f, "\\textbf{Type: } Variable, %d lags \n \n ", cv->num_lag);
   } 
  if(cv->param==2)
   {
    fprintf(f, "\\textbf{Type: } Function, %d lags \n \n", cv->num_lag);
   } 
  if(cv->param==1)
   {
    fprintf(f, "\\textbf{Type: } Parameter\n \n");
   } 
  fprintf(f, "%s ", cd->text);
  if(cd->init!=NULL && strlen(cd->init)>0)
   fprintf(f, "\n \n \\textbf{Initial values:} %s \\\\ \\hline \n",cd->init);
  else
   fprintf(f, "\\\\ \\hline \n"); 
 }
fprintf(f, "\\end{longtable}\n\n");

//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur))
for(cb=r->b; cb!=NULL; cb=cb->next)
 tex_report(cb->head, f);

}

void tex_report_observe(object *r, FILE *f)
{
variable *cv;
description *cd;
object *cur;
int count;
bridge *cb;

if(r->up==NULL)
 {
  fprintf(f, "\\section{Elements relevants to observe}\n");
 fprintf(f, "\\begin{longtable}{||p{3cm}|p{10cm}||}\n  \\hline\n  \\textbf{Label} & \\textbf{Description} \\\\  \\hline \\endhead \n");
 }
for(cv=r->v; cv!=NULL; cv=cv->next)
 {
  
  cd=search_description(cv->label);
  if(cd->observe=='y')
  {
  fprintf(f, "\\lsd{%s} &", cv->label);
  if(cv->param==0)
   {
    fprintf(f, "\\textbf{Object: } \\lsd{%s} \\textbf{Type: } Variable, %d lags \n \n ", r->label, cv->num_lag);
   } 
  if(cv->param==2)
   {
    fprintf(f, "\\textbf{Object: } \\lsd{%s} \\textbf{Type: } Function, %d lags \n \n",r->label, cv->num_lag);
   } 
  if(cv->param==1)
   {
    fprintf(f, "\\textbf{Object: } \\lsd{%s} \\textbf{Type: } Parameter\n \n", r->label);
   } 
  fprintf(f, "%s ", cd->text); 
  fprintf(f, "\\\\ \\hline \n"); 
  } 
 }


//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur))
for(cb=r->b; cb!=NULL; cb=cb->next)
 tex_report_observe(cb->head, f);
if(r->up==NULL)
 fprintf(f, "\\end{longtable}\n\n");
}

void tex_report_init(object *r, FILE *f)
{
variable *cv;
description *cd;
object *cur;
int count;
bridge *cb;

if(r->up==NULL)
 {
 fprintf(f, "\\section{Values relevants to initialize}\n");
 fprintf(f, "\\begin{longtable}{||p{3cm}|p{10cm}||}\n  \\hline\n  \\textbf{Label} & \\textbf{Description} \\\\  \\hline \\endhead \n");
 }

for(cv=r->v; cv!=NULL; cv=cv->next)
 {
  
  cd=search_description(cv->label);
  if(cd->initial=='y')
  {
  fprintf(f, "\\lsd{%s} &", cv->label);
  if(cv->param==0)
   {
    fprintf(f, "\\textbf{Object: } \\lsd{%s} \\textbf{Type: } Variable, %d lags \n \n", r->label, cv->num_lag);
   } 
  if(cv->param==2)
   {
    fprintf(f, "\\textbf{Object: } \\lsd{%s} \\textbf{Type: } Function, %d lags \n \n",r->label, cv->num_lag);
   } 
  if(cv->param==1)
   {
    fprintf(f, "\\textbf{Object: } \\lsd{%s} \\textbf{Type: } Parameter\n \n", r->label);
   } 
  fprintf(f, "%s ", cd->text); 
  if(cd->init!=NULL && strlen(cd->init)>0)
   fprintf(f, "\n \n \\textbf{Initial values:} %s \\\\ \\hline \n",cd->init);
  else
   fprintf(f, "\\\\ \\hline \n"); 
  } 
 }



//for(cur=r->son; cur!=NULL; cur=skip_next_obj(cur))
for(cb=r->b; cb!=NULL; cb=cb->next)
 tex_report_init(cb->head, f);
if(r->up==NULL)
 fprintf(f, "\\end{longtable}\n\n");
}
