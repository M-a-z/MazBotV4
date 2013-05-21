/* ********************************************************************
 *
 * @file testgen.c
 * Basic tests for generic functions
 *
 * -Revision History:
 * 
 *  10.07.2009/Maz  First draft
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



#include <generic.h>
#include <stdio.h>
int main(void)
{
	int i,j;
	for(i=0;i<50;i++)
	{
		for(j=0;j<=EPrintLevel_Panic;j++)
		{
			MazPrint(j,"print %d using level %d and straight fcall",i,j);
			switch(j)
			{
				case 0:
					DPRINT("print %d using level %d and macro",i,j);
					break;
				case 1:
					IPRINT("print %d using level %d and macro",i,j);
					break;
				case 2:
					WPRINT("print %d using level %d and macro",i,j);
					break;
				case 3:
					EPRINT("print %d using level %d and macro",i,j);
					break;
				case 4:
					PPRINT("print %d using level %d and macro",i,j);
					break;
				default:
					break;
			}
		}
	}
	fflush(stdout);
	MAZZERT(1,"Should Not Fail!");
	MAZZERT(0,"Should Fail");
	return 0;
}
