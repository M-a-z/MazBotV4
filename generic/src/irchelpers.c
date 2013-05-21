#include "irchelpers.h"
#include "generic.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int copyprefixtonickmask(char *prefix, char **nick, char **mask)
{
	size_t nmlen,i,mlen,j;
	*nick=NULL;
	*mask=NULL;
//	*nick=prefix;
	if(NULL == prefix || NULL==nick || NULL==mask)
	{
		EPRINT("NULL ptr in %s at %s:%d!",__FUNCTION__,__FILE__,__LINE__);
		return -1;
	}
	nmlen=strlen(prefix);
	for(i=1;i<nmlen;i++)
	{
		if(prefix[i]=='!')
		{
			*nick=malloc(i);
			if(NULL==*nick)
			{
				PPRINT("MALLOC FAILED!!");
				return -1;
			}
			memcpy(*nick,&(prefix[1]),i-1);
			(*nick)[i-1]='\0';
			break;
		}
	}
	if(NULL==*nick)
	{
		EPRINT("FAILED to parse nick from prefix , ! - char is missing!");
		return -1;
	}
	mlen=nmlen-i;
	for(j=0;j<mlen;j++)
	{
		if(prefix[i+j]=='@')
		{
			*mask=malloc(mlen-j+1);
			if(NULL==*mask)
			{
				PPRINT("MALLOC FAILED!!");
				return -1;
			} 
			memcpy(*mask,&(prefix[i+j+1]),mlen-j+1);
		}
	}
	if(i==nmlen || NULL==*mask)
	{
		PPRINT("FAILED to parse nick/mask!");
		return -1;
	}
	return 0;
}


char *prepare_for_sending(size_t *sendsize, const char *fmt,...)
{
    int alloclen=20;
    int bufsize=0;
    int required=-1;
    size_t tmpsize;
    char *buff=NULL;
    va_list ap;

    while(1)
    {
        printf("allocing %u bytes\n",alloclen);
        buff=malloc(alloclen);
        if(NULL==buff)
        {
            PPRINT("alloc FAILED at %s:%d!",__FILE__,__LINE__);
            *sendsize=0;
            return NULL;
        }
        bufsize=alloclen;

        va_start(ap,fmt);
        required=vsnprintf(buff,bufsize,fmt,ap);
        va_end(ap);
        if(bufsize-2 > required && -1<required)
        {
            printf("bufsize=%u, snprint returned %u\n",bufsize,required);
            printf("success! - buffercontents: %s\n",buff);
            /* Success! */
            break;
        }
        else if(required<0)
        {
            /* Some odd error occurred, perhaps we're not on linux system? */
            PPRINT("Odd error occurred at %s:%d, maybe you're not running linux?",__FILE__,__LINE__);
            *sendsize=0;
            free(buff);
            return NULL; 
        }
        else
        {
            /* there was not enough space, at least not enough to hold \r\n\0! */
            printf("bufsize=%u, snprint returned %u\n",bufsize,required);
            required+=3;
            alloclen=required;
            printf("new req = %u - retrying\n",required);
            free(buff);
        }

    }
    tmpsize=strlen(buff);
    buff[tmpsize]=(unsigned char)10;  //\r
    buff[tmpsize+1]=(unsigned char)13; //\n
    buff[tmpsize+2]='\0';
    *sendsize=tmpsize+2;
    return buff;
}
