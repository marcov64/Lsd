/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
LSD is distributed according to the GNU Public License

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

- void cmd(char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

****************************************************/

#include "decl.h"


/****************************************************
SHOW_EQ
****************************************************/
void show_eq(char *lab, int *choice)
{
char c1_lab[MAX_LINE_SIZE], c2_lab[MAX_LINE_SIZE], c3_lab[MAX_LINE_SIZE], *app, *fname;
FILE *f;
int i, j, k, done, bra, start, lun, printing_var=0, comment_line=0, temp_var=0;

cmd( "if [string compare [info command .eq_%s] .eq_%s ] {set ex yes} {set ex no}", lab, lab );
app= (char *)Tcl_GetVar(inter, "ex",0);
strcpy(msg,app);
if(strcmp(msg, "yes"))
 return;

// define the correct parent window
cmd( "switch %d { 0 { set parWnd . } 1 { set parWnd .chgelem } 2 { set parWnd .da } 3 { set parWnd .deb } }", *choice );

start:
fname = equation_name;
if( (f=fopen(equation_name,"r"))==NULL)
 {
  cmd( "set answer [ tk_messageBox -parent . -type okcancel -default ok -icon error -title Error -message \"Equation file not found\" -detail \"Check equation file name '%s' and press 'OK' to retry.\" ]; switch $answer { ok { set choice 1 } cancel { set choice 2 } } ", equation_name  );
  cmd( "if { $choice == 1 } { set res [ tk_getOpenFile -parent . -title \"Load Equation File\"  -initialdir [pwd] -filetypes { { { LSD Equation Files } { .cpp } } { { All Files } { * } } } ]; if [ fn_spaces $res . ] { set res \"\" } }" );

 if(*choice==1)
 {
 app=(char *)Tcl_GetVar(inter, "res",0);
 if ( app == NULL || ! strcmp( app, "" ) )
	 return;
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

// search in all extra source files
cmd( "if [ file exists \"%s/model_options.txt\" ] { \
		set f [ open model_options.txt r ]; \
		set a [ read -nonewline $f ]; \
		close $f; \
		set pos1 [ expr [ string first \"FUN_EXTRA=\" $a ] + 10 ]; \
		if { $pos1 != -1 } { \
			set pos2 [ expr [ string first \"\n\" $a $pos1 ] - 1 ]; \
			if { $pos2 == -1 } { \
				set pos2 end \
			}; \
			set fun_extra [ string range $a $pos1 $pos2 ]; \
			set fun_extra [ split \"$fun_extra\" \" \t\" ]; \
			set extra_files [ list ]; \
			foreach x $fun_extra { \
				if { [ file exists $x ] || [ file exists \"%s/$x\" ] } { \
					lappend extra_files $x \
				} \
			}; \
			set choice [ llength $extra_files ] \
		} else { \
			set choice 0 \
		} \
	}", exec_path, exec_path );

for ( done = 0, k = 0; done == 0 && k <= *choice; ++k )
{
	if ( k > 0 )
	{
		if ( f != NULL )
			fclose( f );
		cmd( "set brr [ lindex $extra_files %d ]", k - 1 );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );
		if ( ( f = fopen( fname, "r" ) ) == NULL )
			continue;
	}
	
	strcpy(c1_lab, "");
	strcpy(c2_lab, "");

	while ( done==0 && fgets( c1_lab, MAX_LINE_SIZE, f ) != NULL )
	{
		clean_spaces(c1_lab); //eliminate the spaces
		for(i=0; c1_lab[i]!='"' && c1_lab[i]!=(char)NULL; i++)
			c2_lab[i]=c1_lab[i];
		c2_lab[i]=(char)NULL; //close the string

		if(!strcmp(c2_lab, "if(!strcmp(label,")||!strcmp(c2_lab, "EQUATION(")||!strcmp(c2_lab, "FUNCTION("))
		{
			if(!strcmp(c2_lab, "if(!strcmp(label,"))
				macro=false;
			else
				macro=true;
			for(j=i+1; c1_lab[j]!='"'; j++)
				c3_lab[j-i-1]=c1_lab[j];
			c3_lab[j-i-1]=(char)NULL;
			if(!strcmp(c3_lab, lab) )
				done=1;
		}
	}
}

if(done==0)
{
	if ( f != NULL )
		fclose( f );
	cmd( "tk_messageBox -parent . -type ok -icon error -title Error -message \"Equation not found\" -detail \"Equation for '%s' not found (check the spelling or equation file name).\"", lab  );
	return;
}

cmd( "set w .eq_%s", lab );
cmd( "set s \"\"" );
cmd( "newtop $w \"%s Equation (%s)\" { destroytop .eq_%s } $parWnd", lab, fname, lab  );

cmd( "frame $w.f" );
cmd( "scrollbar $w.f.yscroll -command \"$w.f.text yview\"" );
cmd( "scrollbar $w.f.xscroll -orient horiz -command \"$w.f.text xview\"" );
cmd( "text $w.f.text -font \"$font_small\" -wrap none -tabstyle wordprocessor -yscrollcommand \"$w.f.yscroll set\" -xscrollcommand \"$w.f.xscroll set\"" );
cmd( "settab $w.f.text $tabsize \"$font_small\"" );
cmd( "pack $w.f.yscroll -side right -fill y" );
cmd( "pack $w.f.xscroll -side bottom -fill x" );
cmd( "pack $w.f.text -expand yes -fill both" );
cmd( "pack $w.f -expand yes -fill both" );
cmd( "finddone $w b { \
		set W .eq_%s; \
		set cur1 [ $W.f.text index insert ]; \
		newtop $W.s \"\" { destroytop $W.s } $W; \
		label $W.s.l -text \"Find\"; \
		entry $W.s.e -textvariable s -justify center; \
		focus $W.s.e; \
		button $W.s.b -width $butWid -text OK -command { \
			destroytop $W.s; \
			set cur1 [ $W.f.text search -count length $s $cur1 ]; \
			if { [ string length $cur1 ] > 0 } { \
				$W.f.text tag remove sel 1.0 end; \
				$W.f.text tag add sel $cur1 \"$cur1 + $length char\"; \
				$W.f.text mark set insert \"$cur1 + $length char\"; \
				update; \
				focus $W.f.text; \
				$W.f.text see $cur1 \
			} \
		}; \
		pack $W.s.l $W.s.e; \
		pack $W.s.b -padx 10 -pady 10; \
		bind $W.s <KeyPress-Return> { $W.s.b invoke }; \
		showtop $W.s centerS \
	} { destroytop .eq_%s }", lab, lab  );
cmd( "bind .eq_%s <Control-f> {.eq_%s.b.search invoke}; bind .eq_%s <Control-F> {.eq_%s.b.search invoke}", lab, lab, lab, lab );
cmd( "bind .eq_%s <F3> { \
		set W .eq_%s; \
		set cur1 [ $W.f.text index insert ]; \
		set cur1 [ $W.f.text search -count length $s $cur1 ]; \
		if { [ string length $cur1 ] > 0 } { \
			$W.f.text tag remove sel 1.0 end; \
			$W.f.text tag add sel $cur1 \"$cur1 + $length char\"; \
			$W.f.text mark set insert \"$cur1 + $length char\"; \
			update; \
			focus $W.f.text; \
			$W.f.text see $cur1 \
		} \
	}", lab, lab );

cmd( ".eq_%s.f.text tag conf vars -foreground blue4", lab );
cmd( ".eq_%s.f.text tag conf comment_line -foreground green4", lab );
cmd( ".eq_%s.f.text tag conf temp_var -foreground red4", lab );

cmd( "set mytag \"\"" );

if(!macro)
 {//standard type of equations
  start=1;
  bra=1;
 }
else
 {
 start=0;
 bra=2;
 }
 for(; (bra>1||start==1) && fgets(c1_lab, MAX_LINE_SIZE, f)!=NULL;  )
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
     {
		 cmd( ".eq_%s.f.text insert end \"{\" $mytag",lab );
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
		cmd( ".eq_%s.f.text insert end \"}\" $mytag ",lab );
      }
	  }
	 else
	 if(c1_lab[i]=='\\')
	  {
		  cmd( ".eq_%s.f.text insert end \\\\ $mytag",lab );
	  }
	 else
	 if(c1_lab[i]=='[')
	  {
		  cmd( ".eq_%s.f.text insert end \\[ $mytag ",lab );
	  }
	 else
	 if(c1_lab[i]==']')
	  {
	  cmd( ".eq_%s.f.text insert end \\]  $mytag",lab );
      if(temp_var==1)
       {
        temp_var=0;
        cmd( "set mytag \"\"" );
       }
	  }
	 else
	 if(c1_lab[i]=='"')
	  {
      if(printing_var==1 && comment_line==0)
       cmd( "set mytag \"\"" );

      cmd( ".eq_%s.f.text insert end {\"} $mytag",lab );
      if(printing_var==0 && comment_line==0)
       {cmd( "set mytag \"vars\"" );
       printing_var=1;
       }
      else
       printing_var=0;
	  }
	 else
     if(c1_lab[i]=='/' && c1_lab[i+1]=='/' && comment_line==0)
      {
       cmd( "set mytag comment_line" );
       comment_line=1;
       cmd( ".eq_%s.f.text insert end \"//\" $mytag",lab );
       i++;
      }
     else
     if(c1_lab[i]=='/' && c1_lab[i+1]=='*' && comment_line==0)
      {
       cmd( "set mytag comment_line" );
       comment_line=2;
       cmd( ".eq_%s.f.text insert end \"/*\" $mytag", lab );
       i++;
      }
     else
     if(c1_lab[i]=='*' && c1_lab[i+1]=='/' && comment_line==2)
      {
       comment_line=0;
       cmd( ".eq_%s.f.text insert end \"*/\" $mytag", lab );
       i++;
       cmd( "set mytag \"\"" );
      }
     else
     if(c1_lab[i]=='v' && c1_lab[i+1]=='[' && comment_line==0)
      {
       temp_var=1;
       cmd( "set mytag temp_var" );
       cmd( ".eq_%s.f.text insert end \"v\" $mytag", lab );
      }
     else
     if(c1_lab[i]!='\n')
	  {
		cmd( ".eq_%s.f.text insert end \"%c\"  $mytag",lab, c1_lab[i] );
	  }
	  else
	  {
	  cmd( ".eq_%s.f.text insert end \\n  $mytag", lab );
     if(comment_line==1)
      {cmd( "set mytag \"\"" );
       comment_line=0;
      }
	}
  }
 }
fclose(f);

cmd( ".eq_%s.f.text mark set insert 1.0", lab );

cmd( "bind .eq_%s.f.text <KeyPress-Prior> {.eq_%s.f.text yview scroll -1 pages}", lab, lab );
cmd( "bind .eq_%s.f.text <KeyPress-Next> {.eq_%s.f.text yview scroll 1 pages}", lab, lab );
cmd( "bind .eq_%s.f.text <KeyPress-Up> {.eq_%s.f.text yview scroll -1 units}", lab, lab );
cmd( "bind .eq_%s.f.text <KeyPress-Down> {.eq_%s.f.text yview scroll 1 units}", lab, lab );
cmd( "bind .eq_%s.f.text <KeyPress-Left> {.eq_%s.f.text xview scroll -1 units}", lab, lab );
cmd( "bind .eq_%s.f.text <KeyPress-Right> {.eq_%s.f.text xview scroll 1 units}", lab, lab );

cmd( "bind .eq_%s.f.text <Double-1> {.eq_%s.f.text tag remove sel 1.0 end; set a @%%x,%%y; .eq_%s.f.text tag add sel \"$a wordstart\" \"$a wordend\"; set res [.eq_%s.f.text get sel.first sel.last]; set choice 29 }", lab, lab, lab, lab );

cmd( "showtop $w centerS 1 1" );

cmd( ".eq_%s.f.text conf -state disabled", lab );
}


/****************************************************
SCAN_USED_LAB
****************************************************/
void scan_used_lab(char *lab, int *choice)
{
char c1_lab[MAX_LINE_SIZE], c2_lab[MAX_LINE_SIZE], *fname;
FILE *f;
int i, j, k, done, bra, start, exist;
int caller = *choice;

cmd( "set list .list_%s", lab );

cmd( "if {[winfo exists $list]==1} { set choice 1 } { set choice 0 }" );
if(*choice==1)
 return;

cmd( "newtop $list \"Used In\" { destroytop .list_%s }", lab  );

cmd( "frame $list.lf " );
cmd( "label $list.lf.l1 -text \"Equations using\"" );
cmd( "label $list.lf.l2 -fg red -text \"%s\"", lab );
cmd( "pack $list.lf.l1 $list.lf.l2" );

cmd( "frame $list.l" );
cmd( "scrollbar $list.l.v_scroll -command \".list_%s.l.l yview\"", lab );
cmd( "listbox $list.l.l -width 25 -selectmode single -yscroll \".list_%s.l.v_scroll set\"", lab );
cmd( "pack $list.l.l  $list.l.v_scroll -side left -fill y" );
cmd( "mouse_wheel $list.l.l" );

if ( caller != 1 )
	cmd( "label $list.l3 -text \"(double-click to\\nobserve the element)\"" );
else
	cmd( "label $list.l3" );

cmd( "pack $list.lf $list.l $list.l3 -pady 5 -expand yes -fill both" );

cmd( "done $list b { destroytop .list_%s }", lab );		// done button

// search in all extra source files
cmd( "if [ file exists \"%s/model_options.txt\" ] { \
		set f [ open model_options.txt r ]; \
		set a [ read -nonewline $f ]; \
		close $f; \
		set pos1 [ expr [ string first \"FUN_EXTRA=\" $a ] + 10 ]; \
		if { $pos1 != -1 } { \
			set pos2 [ expr [ string first \"\n\" $a $pos1 ] - 1 ]; \
			if { $pos2 == -1 } { \
				set pos2 end \
			}; \
			set fun_extra [ string range $a $pos1 $pos2 ]; \
			set fun_extra [ split \"$fun_extra\" \" \t\" ]; \
			set extra_files [ list ]; \
			foreach x $fun_extra { \
				if { [ file exists $x ] || [ file exists \"%s/$x\" ] } { \
					lappend extra_files $x \
				} \
			}; \
			set choice [ llength $extra_files ] \
		} else { \
			set choice 0 \
		} \
	}", exec_path, exec_path );

for ( exist = 0, k = 0; k <= *choice; ++k )
{
	if ( k == 0 )
		fname = equation_name;
	else
	{
		cmd( "set brr [ lindex $extra_files %d ]", k - 1 );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );
	}
	
	if ( ( f = fopen( fname, "r" ) ) != NULL )
	{
		strcpy(c1_lab, "");
		strcpy(c2_lab, "");

		for(done=0; fgets(c1_lab, MAX_LINE_SIZE, f)!=NULL;  )
		{
		  clean_spaces(c1_lab); //eliminate the spaces
		  for(i=0; c1_lab[i]!='"' && c1_lab[i]!=(char)NULL ; i++)
		   c2_lab[i]=c1_lab[i];
		  c2_lab[i]=(char)NULL; //close the string
		  if(!strcmp(c2_lab, "if(!strcmp(label,")||!strcmp(c2_lab, "EQUATION(")||!strcmp(c2_lab, "FUNCTION("))
			{
		   if(!strcmp(c2_lab, "if(!strcmp(label,") )
			macro=false;
		   else
			macro=true;
		   for(j=0;c1_lab[i+1+j]!='"'; j++)
			  c2_lab[j]=c1_lab[i+1+j]; //prepare the c2_lab to store the var's label
			 c2_lab[j]=(char)NULL;
			done=contains(f, lab, strlen(lab));
			if(done==1)
			 {cmd( "$list.l.l insert end %s", c2_lab );
			  exist=1;
			 }
			}
		}
		fclose(f);
	}
}

if(exist==1)
{
	if ( caller != 1 )
		cmd( "bind $list <Double-Button-1> {set bidi [selection get]; set done 8; set choice 55}" );
}
else
	cmd( "$list.l.l insert end \"(never used)\"" );

cmd( "showtop $list" );
}

/****************************************************
SCAN_USING_LAB
****************************************************/
void scan_using_lab(char *lab, int *choice)
{
FILE *f;
int i,j, done, bra, start, exist;
variable *cv;
int caller = *choice;

cmd( "set list .listusing_%s", lab );

*choice=0;
cmd( "if {[winfo exists $list]==1} {set choice 1} {}" );
if(*choice==1)
  return;

cmd( "newtop $list \"Using\" { destroytop .listusing_%s }", lab  );

cmd( "frame $list.lf " );
cmd( "label $list.lf.l1 -justify center -text \"Elements used in\"" );
cmd( "label $list.lf.l2 -fg red -text \"%s\"", lab );
cmd( "pack $list.lf.l1 $list.lf.l2" );

cmd( "frame $list.l" );
cmd( "scrollbar $list.l.v_scroll -command \".listusing_%s.l.l yview\"", lab );
cmd( "listbox $list.l.l -width 25 -selectmode single -yscroll \".listusing_%s.l.v_scroll set\"", lab );
cmd( "pack $list.l.l $list.l.v_scroll -side left -fill y" );
cmd( "mouse_wheel $list.l.l" );

if ( caller != 1 )
	cmd( "label $list.l3 -text \"(double-click to\\nobserve the element)\"" );
else
	cmd( "label $list.l3" );

cmd( "pack $list.lf $list.l $list.l3 -pady 5 -expand yes -fill both" );

cmd( "done $list b { destroytop .listusing_%s }", lab );		// done button

cv=root->search_var(root, lab);
find_using(root,cv,NULL);
cmd( "set choice [$list.l.l size]" );
if(*choice!=0)
{
	if ( caller != 1 )
		cmd( "bind $list <Double-Button-1> {set bidi [selection get]; set choice 55; set done 8}" );
}
else
 cmd( "$list.l.l insert end \"(none)\"" );

cmd( "showtop $list" );
}


/****************************************************
CONTAINS
 scans an equation checking if it contains anywhere the string
 lab between quotes. Returns 1 if found, and 0 otherwise.
 The file passed is moved to point to the next equation
 It correctly skip the commented text, either by // or by / * ... * /
****************************************************/
int contains (FILE *f, char *lab, int len)
{
int bra, found, start, i, got, j, comm=0;
char c1_lab[MAX_LINE_SIZE], pot[MAX_LINE_SIZE];
if(!macro)
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
 for(found=0; (bra>1||start==1) && fgets(c1_lab, MAX_LINE_SIZE, f)!=NULL;  )
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
