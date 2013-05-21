/* ******************************************************** */
/*                                                          *
 *      Implementation of php's explode written in C        *
 *      Written by  Maz (2008)                              *
 *      http://maz-programmersdiary.blogspot.com/           *
 *                                                          *
 *      You're free to use this piece of code.              *
 *      You can also modify it freely, but if you           *
 *      improve this, you must write the improved code      *
 *      in comments at:                                     *
 *      http://maz-programmersdiary.blogspot.com/           *
 *      or at:                                              *
 *      http://c-ohjelmoijanajatuksia.blogspot.com/         *
 *      or mail the corrected version to me at              *
 *      Mazziesaccount@gmail.com                            *
 *                                                          *
 *      Revision History:                                   *
 *                                                          *
 *      -v0.0.1 16.09.2008/Maz                              *
 *                                                          */
/* ******************************************************** */

#include "stdio.h"
#include <helpers.h>

int main(int argc,char *argv[])
{
    char *string;
    char *delim;
    int retval;
    int index=0;
    char *token;
    CexplodeStrings expString;
    if(argc!=3)
    {
        printf("Test Command should be:\n");
        printf("<testExe> \"original string\" \"delimiter string\"\n");
        return -1;
    }
    string=argv[1];
    delim=argv[2];

    printf("TestString is \"%s\"\n",string);
    printf("Test Delimiter is \"%s\"\n",delim);
    if(0>(retval=Cexplode(string,delim,&expString)))
    {
        printf("CexplodeFailed!\n");
        return -1;
    }
    else
    {

		//Test lastwastoken and getlentilllast and getlast and getnext
		printf("lentilllast=%u\n",Cexplode_getlentilllast(expString));
		 printf("separator %s in the end!\n",(Cexplode_sepwasatend(expString))?"WAS":"WAS NOT");
		 //New Way:
		while(NULL!=(token=Cexplode_getnext(&expString)))
		{
			printf("getnext: token=\"%s\"\n",token);
			fflush(stdout);
		}
		printf("First was \"%s\"\n",Cexplode_getfirst(&expString));
		printf("Last was \"%s\"\n",Cexplode_getlast(&expString));
		printf("##########NEW TEST ENDED############\n");
		//Way 1, use expString straight away:
    	printf("Way 1, use expString straight away:\n");
    	for(index=0;index<expString.amnt;index++)
    	{
    		printf("token %d = %s\n",index+1,expString.strings[index]);
    	}
    	//Way 2, you can use Cexplode_getNth, or Cexplode_getfirst()
    	printf("Way 2, use Cexplode_getNth, or Cexplode_getfirst():\n");
        token=Cexplode_getfirst(&expString);
        printf("first token %s\n",token);
        index=1; 
        while(NULL!=(token=Cexplode_getNth(++index,&expString)))
        {
            printf("token %d = %s\n",index,token);
        }
    }
    Cexplode_free(expString);
    return 0;
}
