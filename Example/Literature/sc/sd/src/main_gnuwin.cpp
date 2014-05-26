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



#include "choose.h"

#include <stdio.h>

#include <stdlib.h>
//#include <unistd.h>
#ifndef NO_WINDOW
 #include <tk.h>
#endif 
#include <stdio.h>
#include <stdlib.h>

#include "decl.h"

void copy(char *f1, char *f2);

//int cd(char *np) {return (chdir(np)); };
int errormsg( char *lpszText,  char *lpszTitle);
int lsdmain(int argn, char **argv);
void error_hard(void);
int myexit(int v);

extern object *root; 

int main(int argn, char **argv)
{
object *cur, *cur1;

return lsdmain(argn, argv);
/*

try {
return lsdmain(argn, argv);
}

catch(...)
 {
 for(cur=root->son; cur!=NULL; )
  {
   cur1=cur->next;
   cur->delete_obj();
   cur=cur1;
  } 
//error_hard();
 }

return myexit(0);
*/
}


int errormsg( char const *lpszText,  char const *lpszTitle)
{

//printf("\n%s",*lpszText);
myexit(1);
return 0; //just to avoid warning messages
}

int myexit(int v)
{

#ifndef NO_WINDOW
 Tcl_Exit(0);
#endif
exit(v);
}




