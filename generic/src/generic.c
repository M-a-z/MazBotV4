/* ********************************************************************
 *
 * @file generic.h
 * @brief generic helper functions which I always end up redoing.
 * Why on earth one never stores these? I need to rewrite these 
 * for each project I start...
 *
 *
 * -Revision History:
 *
 *  -0.0.3 18.02.2010/Maz  Added another print function for accepting file & line
 *  -0.0.2 26.07.2009/Maz  Added missing free() to print function
 *  -0.0.1 10.07.2009/Maz  First draft
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




#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define FMT_INITIAL_LEN 200

EPrintLevel G_printFilterLevel = PRINT_FILTER_DEFAULT;
static int printsdisabled[PrintComp_Last]={0,0,0,0,0};

void disableprint(PrintComp cmp)
{
	if((unsigned int)cmp<PrintComp_Last)
		printsdisabled[cmp]=1;
}
int isdisabled(PrintComp cmp)
{
	return printsdisabled[cmp];
}
void mazzert(char *explanation,char *file,int line)
{
	static char babbling[512];
	printf("Assertion Error:\n");
	snprintf(&babbling[0],511,"%s at %s:%d\n",explanation,file,line);
	printf(babbling);
	printf("Assert TODO: What are the needed cleanup routines (if any?) Clear shm? Kill processes?\n");
	//TODO: What are the needed cleanup routines (if any?) Clear shm?
	exit(-1);
}
//writing 32 bits should be atomic in most of the systems nowadays (due to 32 bit wide buses)
//=> No protection needed.
void MazPrintSetLevel(EPrintLevel level)
{
	G_printFilterLevel = level;
}
//XXX todo - add devices for printing to file - over UDP ...


void MazPrint2(EPrintLevel level, char *file, unsigned int fileline, const char *fmt,...)
{
	va_list argum;
	char *line;
	if((unsigned int)level>=EPrintLevel_NmbrOf)
		level=EPrintLevel_Unknown;
	if(G_printFilterLevel <= (unsigned int)level)
	{
		time_t prntime;
		size_t len=0;
		const char *(leveltxt[EPrintLevel_NmbrOf])=
		{
			LVL1_TXT,
			LVL2_TXT,
			LVL3_TXT,
			LVL4_TXT,
			LVL5_TXT,
			LVL6_TXT,
		};
		prntime=time(NULL);
		line=malloc(FMT_INITIAL_LEN);
		if(NULL==line)
		{
			printf("Malloc Failed\n");
			fflush(stdout);
			exit(-1);
		}
		if(FMT_INITIAL_LEN<(len=snprintf(line,FMT_INITIAL_LEN,"%s\t%s %s at %s:%u\n",ctime(&prntime),leveltxt[level],fmt,file,fileline)))
		{
			line=realloc(line,len);
			if(NULL==line)
			{
				printf("realloc Failed!\n");
				fflush(stdout);
				exit(-1);
			}
			snprintf(line,len,"%s\t%s %s\n",ctime(&prntime),leveltxt[level],fmt);
			line[len-1]='\0';
		}
		va_start(argum,fmt);
		vprintf(line,argum);
//		if(level>=EPrintLevel_Error)
			fflush(stdout);
		va_end(argum);
		free(line);
	}

}
void MazPrint(EPrintLevel level, const char *fmt,...)
{
	va_list argum;
	char *line;
	if((unsigned int)level>=EPrintLevel_NmbrOf)
		level=EPrintLevel_Unknown;
	if(G_printFilterLevel <= (unsigned int)level)
	{
		time_t prntime;
		size_t len=0;
		const char *(leveltxt[EPrintLevel_NmbrOf])=
		{
			LVL1_TXT,
			LVL2_TXT,
			LVL3_TXT,
			LVL4_TXT,
			LVL5_TXT,
			LVL6_TXT,
		};
		prntime=time(NULL);
		line=malloc(FMT_INITIAL_LEN);
		if(NULL==line)
		{
			printf("Malloc Failed\n");
			fflush(stdout);
			exit(-1);
		}
		if(FMT_INITIAL_LEN<(len=snprintf(line,FMT_INITIAL_LEN,"%s\t%s %s\n",ctime(&prntime),leveltxt[level],fmt)))
		{
			line=realloc(line,len);
			if(NULL==line)
			{
				printf("realloc Failed!\n");
				fflush(stdout);
				exit(-1);
			}
			snprintf(line,len,"%s\t%s %s\n",ctime(&prntime),leveltxt[level],fmt);
			line[len-1]='\0';
		}
		va_start(argum,fmt);
		vprintf(line,argum);
//		if(level>=EPrintLevel_Error)
			fflush(stdout);
		va_end(argum);
		free(line);
	}
}

