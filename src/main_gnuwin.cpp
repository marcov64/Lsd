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

int lsdmain(int argn, char **argv);
int myexit(int v);

extern object *root; 

int main(int argn, char **argv)
{
object *cur, *cur1;

return lsdmain(argn, argv);
}


int errormsg( char const *lpszText,  char const *lpszTitle)
{
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




