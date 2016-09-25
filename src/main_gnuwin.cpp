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

#ifndef NO_WINDOW
 #include <tk.h>
#endif 
#include "decl.h"

int lsdmain(int argn, char **argv);
void myexit(int v);

extern object *root; 

int main(int argn, char **argv)
{
object *cur, *cur1;

return lsdmain(argn, argv);
}

void myexit(int v)
{
#ifndef NO_WINDOW
 Tcl_Exit(0);
#endif
exit(v);
}




