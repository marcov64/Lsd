/***************************************************
****************************************************
LSD 5.2 - December 2003
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to mv@business.auc.dk
****************************************************
****************************************************/

/**********************
Add the directory CD\gnu\bin to the path if it does not already exists. 

This program expects to find the files:

cd.txt 
and
cp.txt

containing respectivey the current directory and the current path

CD to the Lsd root directory and run the command:
> gnu\bin\gcc -g -Ignu\include -Lgnu\lib src\setpath.cpp -o src\setpath.exe
after having copied cygwin1.dll in the Lsd root directory


**********************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
int main(void)
{
   char *path, *ptr, cmd[400], cp[1000], cd[1000], str[1000];
   int i = 0;
   FILE *f;



   f=fopen("src\\cd.txt","r");
	   if(f==NULL)
    return 1;
   fgets(cd, 999, f);
   fclose(f);
   i=strlen(cd)-1;
   cd[i]=(char)NULL;
   if(cd[i-1]=='\r')
     cd[i-1]=(char)NULL;
   f=fopen("src\\cp.txt","r");
   if(f==NULL)
    return 1;
   fgets(cp, 999, f);
   fclose(f);
   i=strlen(cp)-1;
   cp[i]=(char)NULL;


   sprintf(str, "%s\\gnu\\bin", cd);
   for(i=0; i<strlen(str); i++)
    str[i]=tolower(str[i]);
   for(i=0; i<strlen(cp); i++)
    cp[i]=tolower(cp[i]);

   if(strstr(cp+5, str)!=NULL)
    {
//    printf("\nYes\n");
    return 0;
    }
   else
    {
    sprintf(str, "PATH %s\\gnu\\bin;%s\n", cd, cp+5);
//    printf("\nNo\nNew path = %s\n", str);
    f=fopen("src\\kicklmm.bat","w");
    fprintf(f,str);
    fprintf(f, "set LSDROOT=%s\n",cd);
    fprintf(f,"start lmm %%1");
    fclose(f);


    };
/*
   for( i=0; cd[i]!=NULL; i++)

    
   printf("\n%s\n", cd);
  
   path = getenv("UGO");
   printf("\n%s\n", path);
   ptr=strstr(path, "/gnu/bin");
   
   if(ptr==NULL)
      {printf("No");
      
       return 0;
      } 
   else
     {printf("Yes");
      return 1;
     } 
*/
}
 


