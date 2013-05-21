/* ********************************************************************
 *
 * @file splitter.h
 * @brief Simple test for testing the splitter..
 *
 *
 * -Revision History:
 *
 *  - 0.0.1 31.07.2009/Maz  First draft
 *
 *
 *  Lisence info: You're allowed to use / modify this - you're only required to
 *  write me a short note and your opinion about this to Mazziesaccount@gmail.com.
 *  if you redistribute this you're required to mention the original author:
 *  "Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
 *  channel.
 *
 *
 *  PasteLeft 2009 Maz.
 *  *************************************************************/


#include <splitter.h>
#include <stdio.h>
#include <APIs/cexplode.h>
#include <APIs/generic_api.h>

int main(void)
{
    char tosplit[]="fooooooooxbarxbar\r\nbarxbar foo\r\nfoo the barxbarxian\r\n";
    char *splitters[]={"\r\n"," ","x"};
	CexplodeStrings *res_line;
	SSplitterResult *res_struct;
	char *token;
	int i=1;
	int j=1;
    printf("Initing Splitter\n");
    fflush(stdout);
    SSplitter* splitter = SplitterInit(splitters,3);
    printf("Feeding splitter\n");
    fflush(stdout);
    splitter->feed(splitter,tosplit);
    printf("Splitting\n");
    fflush(stdout);
    res_struct=splitter->split(splitter);
	MAZZERT(NULL!=res_struct,"RES_STRUCT init FAILED!");
	while(NULL!=(res_line=res_struct->get(res_struct)))
	{
		DPRINT("Obtained Cexplode results for first piece:");
		while(NULL!=(token=Cexplode_getnext(res_line)))
		{
			printf("piece %d for res%d \"%s\"\n",i,j,token);
			i++;
		}
		j++;
	}
    printf("Freeing\n");
    fflush(stdout);
	res_struct->free(&res_struct);
    splitter->uninit(&splitter);
    printf("Quitting.\n");
    fflush(stdout);
    return 0;
}

