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
SHOW_EQ.CPP show one window containig the equation for the label clicked on.
Less simple as it seems, given that it has to deal with all weird characters
like parenthesis, quotes, brakets, that tk commands consider as special
characters. The basic trick is that it loads one line per time. If it finds
the name of the variable following the command "strcmp" than starts sending the
line to be printed. Lines are printed character per character, so that it can
deal with special characters. While printing it computes the number of parenthesis
open and closed, and when it meets the last parenthesis exits.

Everything is within just one single function,
- void show_eq(char *lab)

- void scan_used_lab(char *lab, int *choice)
Looks in the equation file whether the variable or parameter lab is contained
in some equations. It creates a window containing the list of the equations
using in any way the variable indicated. By clicking on the names in the
list the code for that variable is shown.
It is based on the recognition of the string lab between quotes, thus any function
is recognized.

- int contains (FILE *f, char *lab, int len);
Checks if the equation beginning in the file position indicated by f contains
any function using lab as parameter.


other functions are the usual:
- void plog(char *m);
LSDMAIN.CPP print  message string m in the Log screen.

- void cmd(Tcl_Interp *inter, char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.



****************************************************/


#include <tk.h>
#include "decl.h"

void cmd(Tcl_Interp *inter, char const *cc);
void plog(char const *msg);
int contains (FILE *f, char *lab, int len);
void clean_spaces(char *s);
void find_using(object *r, variable *v, FILE *frep);

extern char msg[];
extern Tcl_Interp *inter;
extern char *equation_name;
extern object *root;

int macro;
/****************************************************
SHOW_EQ
****************************************************/
void show_eq(char *lab, int *choice)
{
char c1_lab[400], c2_lab[400], c3_lab[400], *app;
FILE *f;
int i,j, done, bra, start, lun, printing_var=0, comment_line=0, temp_var=0;

sprintf(msg, "if [string compare [info command .eq_%s] .eq_%s ] {set ex yes} {set ex no}",lab, lab);
cmd(inter, msg);
app= (char *)Tcl_GetVar(inter, "ex",0);
strcpy(msg,app);
if(strcmp(msg, "yes"))
 return;

start:
if( (f=fopen(equation_name,"r"))==NULL)
 {*choice=0;
  cmd( inter, "newtop .top .warn_eq \"Equation file\" {set choice 2}" );
  sprintf(msg, "label .warn_eq.lab2 -text \"%s\" -foreground red", equation_name);
  cmd(inter, msg);
  cmd(inter, "label .warn_eq.lab3 -text \"not found (check equation file name)\"");
  cmd(inter, "pack .warn_eq.lab1 .warn_eq.lab2 .warn_eq.lab3");
  cmd(inter, "frame .warn_eq.b");
  cmd(inter, "button .warn_eq.b.s -width -9 -text Search -command {set res [tk_getOpenFile -title \"Load Equation File\" -filetypes {{{Lsd Equation Files} {.cpp}} {{All Files} {*}} }]; set choice 1}");
  cmd(inter, "button .warn_eq.b.esc -width -9 -text Cancel -command {set choice 2}");
  cmd(inter, "pack .warn_eq.b.s .warn_eq.b.esc -side left");
  cmd(inter, "pack .warn_eq.b");
  cmd( inter, "showtop .warn_eq" );
  while(*choice==0)
   Tcl_DoOneEvent(0);
  cmd(inter, "destroytop .warn_eq");
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
 strcpy(c1_lab, "");
strcpy(c2_lab, "");

for(done=0; done==0 && fgets(c1_lab, 399, f)!=NULL;  )
 {
  clean_spaces(c1_lab); //eliminate the spaces
  for(i=0; c1_lab[i]!='"' && c1_lab[i]!=(char)NULL; i++)
   c2_lab[i]=c1_lab[i];
  c2_lab[i]=(char)NULL; //close the string

  if(!strcmp(c2_lab, "if(!strcmp(label,")||!strcmp(c2_lab, "EQUATION(")||!strcmp(c2_lab, "FUNCTION("))
	{
   if(!strcmp(c2_lab, "if(!strcmp(label,"))
    macro=0;
   else
    macro=1;
   for(j=i+1; c1_lab[j]!='"'; j++)
     c3_lab[j-i-1]=c1_lab[j];
   c3_lab[j-i-1]=(char)NULL;
	if(!strcmp(c3_lab, lab) )
	  {
     done=1;
    } 
	}
 }
if(done==0)
 {fclose(f);
  sprintf(msg,"\nEquation for %s not found (check the spelling or equation file name)", lab);
  plog(msg);
  fclose(f);
  return;
  }
//fgets(c1_lab, 399, f);

sprintf( msg, "newtop .top .eq_%s \"%s Equation\" \".eq_%s.close\"", lab , lab, lab );
cmd(inter, msg);

sprintf(msg, "set w .eq_%s", lab);
cmd(inter, msg);
cmd(inter, "frame $w.f");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "scrollbar $w.f.xscroll -orient horiz -command \"$w.f.text xview\"");
cmd( inter, "set tabwidth \"[ expr { 4 * [ font measure Courier 0 ] } ] left\"" );
cmd( inter, "text $w.f.text -wrap none -tabs $tabwidth -tabstyle wordprocessor -relief sunken -yscrollcommand \"$w.f.yscroll set\" -xscrollcommand \"$w.f.xscroll set\"" );
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.xscroll -side bottom -fill x");
cmd(inter, "pack $w.f.text -expand yes -fill both");
cmd(inter, "pack $w.f -expand yes -fill both");
sprintf(msg, "button $w.close -width -9 -text Close -command {destroytop .eq_%s}", lab);
cmd(inter, msg);
sprintf(msg, "button $w.search -width -9 -text Search -command {set cur [$w.f.text index insert]; newtop $w $w.s \"destroytop $w.s\"; label $w.s.l -text Search; entry $w.s.e -textvariable s; focus $w.s.e; button $w.s.b -width -9 -text Ok -command {destroytop $w.s; set cur [$w.f.text search -count length $s $cur]; if {[string length $cur] > 0} {set ends \"$cur + $length char\"; $w.f.text tag remove sel 1.0 end; $w.f.text tag add sel $cur \"$cur + $length char\"; update; $w.f.text see $cur} }; pack $w.s.l $w.s.e $w.s.b; bind $w.s <KeyPress-Return> {$w.s.b invoke}; showtop $w.s }");
cmd(inter, msg);

cmd(inter, "pack $w.search $w.close");
sprintf(msg, "bind .eq_%s <Control-f> {.eq_%s.search invoke}", lab, lab);
cmd(inter, msg);
sprintf(msg, ".eq_%s.f.text conf -font {Courier 10}", lab);
cmd(inter, msg);
sprintf(msg, ".eq_%s.f.text tag conf vars -foreground red", lab);
cmd(inter, msg);

sprintf(msg, ".eq_%s.f.text tag conf comment_line -foreground green", lab);
cmd(inter, msg);
sprintf(msg, ".eq_%s.f.text tag conf temp_var -foreground blue", lab);
cmd(inter, msg);

cmd(inter, "set mytag \"\"");

if(macro==0)
 {//standard type of equations
  start=1;
  bra=1;
 }
else
 {
 start=0;
 bra=2;
 }
 for(; (bra>1||start==1) && fgets(c1_lab, 399, f)!=NULL;  )
 {sscanf(c1_lab, "%s", c2_lab);
  strcpy(c2_lab, c1_lab);
  clean_spaces(c2_lab);
  if(!strncmp(c2_lab,"RESULT(",7) )
   bra--;
  for(i=0; c1_lab[i]!=0; i++)
   {
  if(c1_lab[i]=='\r') 
   i++;
  if(c1_lab[i]=='{')
	 {if(bra!=1)
     {sprintf(c2_lab, ".eq_%s.f.text insert end \"{\" $mytag",lab);
	  cmd(inter, c2_lab);
     }
     else
      start=0;
	  bra++;
	 }
	else
	 if(c1_lab[i]=='}')
	  {bra--;
      if(bra>1)
      {
      sprintf(c2_lab, ".eq_%s.f.text insert end \"}\" $mytag ",lab);
		cmd(inter, c2_lab);
      }
	  }
	 else
	 if(c1_lab[i]=='\\')
	  {sprintf(c2_lab, ".eq_%s.f.text insert end \\\\ $mytag",lab);
		cmd(inter, c2_lab);
	  }
	 else
	 if(c1_lab[i]=='[')
	  {sprintf(c2_lab, ".eq_%s.f.text insert end \\[ $mytag ",lab);
		cmd(inter, c2_lab);
	  }
	 else
	 if(c1_lab[i]==']')
	  {sprintf(c2_lab, ".eq_%s.f.text insert end \\]  $mytag",lab);
		cmd(inter, c2_lab);
      if(temp_var==1)
       {
        temp_var=0;
        cmd(inter, "set mytag \"\"");
       }
	  }
	 else
	 if(c1_lab[i]=='"')
	  {
      if(printing_var==1 && comment_line==0)
       cmd(inter, "set mytag \"\"");

      sprintf(c2_lab, ".eq_%s.f.text insert end {\"} $mytag",lab);
		cmd(inter, c2_lab);
      if(printing_var==0 && comment_line==0)
       {cmd(inter, "set mytag \"vars\"");
       printing_var=1;
       }
      else
       printing_var=0;
	  }
	 else
     if(c1_lab[i]=='/' && c1_lab[i+1]=='/' && comment_line==0)
      {
       cmd(inter, "set mytag comment_line");
       comment_line=1;
       sprintf(c2_lab, ".eq_%s.f.text insert end \"//\" $mytag",lab);
	    cmd(inter, c2_lab);
       i++;
      }
     else
     if(c1_lab[i]=='/' && c1_lab[i+1]=='*' && comment_line==0)
      {
       cmd(inter, "set mytag comment_line");
       comment_line=2;
       sprintf(c2_lab, ".eq_%s.f.text insert end \"/*\" $mytag",lab);
	    cmd(inter, c2_lab);
       i++;
      }
     else
     if(c1_lab[i]=='*' && c1_lab[i+1]=='/' && comment_line==2)
      {
       comment_line=0;
       sprintf(c2_lab, ".eq_%s.f.text insert end \"*/\" $mytag",lab);
	    cmd(inter, c2_lab);
       i++;
       cmd(inter, "set mytag \"\"");
      }
     else
     if(c1_lab[i]=='v' && c1_lab[i+1]=='[' && comment_line==0)
      {
       temp_var=1;
       cmd(inter, "set mytag temp_var");
       sprintf(c2_lab, ".eq_%s.f.text insert end \"v\" $mytag",lab);
	    cmd(inter, c2_lab);
      }
     else
     if(c1_lab[i]!='\n')
	  {sprintf(c2_lab, ".eq_%s.f.text insert end \"%c\"  $mytag",lab, c1_lab[i]);
		cmd(inter, c2_lab);
	  }
	  else
	  {
	  sprintf(c2_lab, ".eq_%s.f.text insert end \\n  $mytag", lab);
	  cmd(inter, c2_lab);
     if(comment_line==1)
      {cmd(inter, "set mytag \"\"");
       comment_line=0;
      }
	  }

  }

 }
fclose(f);
//sprintf(msg, ".eq_%s.f.text conf -state disabled", lab);
//cmd(inter, msg);
sprintf(msg, "bind .eq_%s.f.text <KeyPress-Prior> {.eq_%s.f.text yview scroll -1 pages}", lab, lab);
cmd(inter, msg);
sprintf(msg, "bind .eq_%s.f.text <KeyPress-Next> {.eq_%s.f.text yview scroll 1 pages}", lab, lab);
cmd(inter, msg);
sprintf(msg, "bind .eq_%s.f.text <KeyPress-Up> {.eq_%s.f.text yview scroll -1 units}", lab, lab);
cmd(inter, msg);
sprintf(msg, "bind .eq_%s.f.text <KeyPress-Down> {.eq_%s.f.text yview scroll 1 units}", lab, lab);
cmd(inter, msg);
sprintf(msg, "bind .eq_%s.f.text <KeyPress-Left> {.eq_%s.f.text xview scroll -1 units}", lab, lab);
cmd(inter, msg);
sprintf(msg, "bind .eq_%s.f.text <KeyPress-Right> {.eq_%s.f.text xview scroll 1 units}", lab, lab);
cmd(inter, msg);

sprintf(c2_lab, "bind .eq_%s.f.text <Double-1> {.eq_%s.f.text tag remove sel 0.0 end; set a @%%x,%%y; .eq_%s.f.text tag add sel \"$a wordstart\" \"$a wordend\"; set res [.eq_%s.f.text get sel.first sel.last]; set choice 29 }",lab, lab, lab, lab);
cmd(inter, c2_lab);

cmd( inter, "showtop $w centerS 1" );
}



/****************************************************
SCAN_USED_LAB
****************************************************/
void scan_used_lab(char *lab, int *choice)
{
char c1_lab[400], c2_lab[400], c3_lab[400], *app;
FILE *f;
int i,j, done, bra, start, exist;

sprintf(msg, "set list .list_%s", lab);
cmd(inter, msg);

*choice=0;
cmd(inter, "if {[winfo exists $list]==1} {set choice 1} {}");
if(*choice==1)
 return;

cmd(inter, "toplevel $list");
cmd(inter, "wm transient $list .");
sprintf(msg, "wm title $list \"%s Users\"", lab);
cmd(inter, msg);
cmd(inter, "listbox $list.l ");
sprintf(msg, "button $list.close -width -9 -text Close -command {destroy .list_%s}", lab);
cmd(inter, msg);
sprintf(msg, "label $list.l1 -text \"List of variables whose equations contain:\\n%s\\nDouble-click on a label to observe the variable\"", lab);
cmd(inter, msg);

sprintf(msg, "pack $list.l1 $list.l $list.close",lab);
cmd(inter, msg);

if( (f=fopen(equation_name,"r"))==NULL)
 return;
strcpy(c1_lab, "");
strcpy(c2_lab, "");

for(exist=0,done=0; fgets(c1_lab, 399, f)!=NULL;  )
 {
  clean_spaces(c1_lab); //eliminate the spaces
  for(i=0; c1_lab[i]!='"' && c1_lab[i]!=(char)NULL ; i++)
   c2_lab[i]=c1_lab[i];
  c2_lab[i]=(char)NULL; //close the string
  if(!strcmp(c2_lab, "if(!strcmp(label,")||!strcmp(c2_lab, "EQUATION(")||!strcmp(c2_lab, "FUNCTION("))
	{
   if(!strcmp(c2_lab, "if(!strcmp(label,") )
    macro=0;
   else
    macro=1;
   for(j=0;c1_lab[i+1+j]!='"'; j++)
	  c2_lab[j]=c1_lab[i+1+j]; //prepare the c2_lab to store the var's label
	 c2_lab[j]=(char)NULL;
    done=contains(f, lab, strlen(lab));
    if(done==1)
     {sprintf(msg,"$list.l insert end %s", c2_lab);
      cmd(inter, msg);
      exist=1;
     }

	}
 }
if(exist==1)
 {sprintf(msg, "bind $list <Double-Button-1> {set bidi [selection get]; ; set done 8; set choice 55}");
  cmd(inter, msg);
 } 
else
 cmd(inter, "$list.l insert end \"(Never Used)\"");
fclose(f);
}

/****************************************************
SCAN_USING_LAB
****************************************************/
void scan_using_lab(char *lab, int *choice)
{
char c1_lab[400], c2_lab[400], c3_lab[400], *app;
FILE *f;
int i,j, done, bra, start, exist;
variable *cv;

sprintf(msg, "set list .listusing_%s", lab);
cmd(inter, msg);

*choice=0;
cmd(inter, "if {[winfo exists $list]==1} {set choice 1} {}");
if(*choice==1)
  return;

cmd(inter, "toplevel $list");
cmd(inter, "wm transient $list .");
sprintf(msg, "wm title $list \"Used in %s\"", lab);
cmd(inter, msg);
cmd(inter, "listbox $list.l");
sprintf(msg, "button $list.close -width -9 -text Close -command {destroy .listusing_%s}", lab);
cmd(inter, msg);
sprintf(msg, "label $list.l1 -text \"List of vars./pars. used in the equation for:\\n%s\\nDouble-click on a label to observe the var./par.\"", lab);
cmd(inter, msg);

sprintf(msg, "pack $list.l1 $list.l $list.close",lab);
cmd(inter, msg);
cv=root->search_var(root, lab);
find_using(root,cv,NULL);
cmd(inter, "set choice [$list.l size]");
if(*choice!=0)
 {sprintf(msg, "bind $list <Double-Button-1> {set bidi [selection get]; set choice 55; set done 8}");
  cmd(inter, msg);
 } 
else
 cmd(inter, "$list.l insert end \"(no Vars/Pars)\"");

}


//scans an equation checking if it contains anywhere the string
//lab between quotes. Returns 1 if found, and 0 otherwise.
//The file passed is moved to point to the next equation
//It correctly skip the commented text, either by // or by /* ... */
/****************************************************
SHOW_EQ
****************************************************/
int contains (FILE *f, char *lab, int len)
{
int bra, found, start, i, got, j, comm=0;
char c1_lab[400], pot[600];
if(macro==0)
 {
 start=1;
 bra=1;
 }
else
 {
 start=0;
 bra=2;
 }

//for each line of the equation ...
 for(found=0; (bra>1||start==1) && fgets(c1_lab, 399, f)!=NULL;  )
 {

 if(comm==1)
   comm=0;
  strcpy(pot, c1_lab);
  clean_spaces(pot);
  if(!strncmp(pot, "RESULT(",7) )
   bra--;
  for(i=0; c1_lab[i]!=0; i++) //scans each character
  {


  if(c1_lab[i]=='{') //if it is an open braket
	   {bra++;
       start=0;
      }
	else
	 if(c1_lab[i]=='}') //if it is a closed braket
	  bra--;
   else
    {
     if(c1_lab[i]=='/' && c1_lab[i+1]=='/')
      comm=1;
     else
       if(c1_lab[i]=='/' && c1_lab[i+1]=='*')
        comm=2;
       else
         if(comm==2 && c1_lab[i]=='*' && c1_lab[i+1]=='/')
          comm=0;
         if(comm==0 && c1_lab[i]=='\"' && found==0) //if it is a quote (supposedly open quotes)
           {
            for(j=i+1; c1_lab[j]!='\"'; j++) //copy the characters till the last quotes
              pot[j-i-1]=c1_lab[j];
            pot[j-i-1]=c1_lab[j];

           //scan the whole word, until a different char is not found
            if(pot[0]!='\"') //in case the eq. contains "" it gets fucked up..
            for(j=0, got=1 ; got==1 && pot[j]!='\"' && lab+j!=NULL; j++)
             if(pot[j]!=lab[j])
              got=0;
            for( ;pot[j]!='\"'; j++); //finishes the word, until the closed quotes
            i=i+j+1;
            if(got==1 && j==len)
             found=1;
            }
     }//New for comm
  }
 }
return found;
}




