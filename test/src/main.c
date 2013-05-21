/* ********************************************************************
 *
 * @file TCP.c
 * @brief a functions to use TCP connection via Sconn pointer.
 *
 *
 * -Revision History:
 *
 *  20.07.2008/Maz  Changed the connection target && made them as defines
 *  12.07.2009/Maz  Added sending HTTP request.
 *  12.07.2009/Maz  First Draft.
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

#include <networking.h>
#include <TCP.h>
#include <generic.h>
#include <stdio.h>
#include <string.h>

#define RECVBUFF 500
//#define HOST "teotilcan.net"
//#define PORT 80
//#define URL "/svn/MazBot/"
#define HOST "irc.freenode.net"
#define PORT 6667
#define SENDDATA "NICK --Maz--\r\n";

int main(void)
{
	Sconn *connection;
	EnetwRetval retval;
	EsockStat   pollval;
	struct timeval tim;
	tim.tv_sec=10;
	tim.tv_usec=0;
	connection = connInit(EstructType_TCPconn);
	if(NULL==connection)
	{
		printf("Miserably failed to init connection struct\n");
		return -1;
	}
	DPRINT("connecting...");
	if(EnetwRetval_Ok!=(retval=connection->connect(connection,HOST,PORT)))
	{
		printf("Miserably failed to connect to %s:%d, retval %d\n",HOST,PORT,retval);
		return -1;
	}
	printf("polling\n");
	switch((pollval=connection->writepoll(connection,tim)))
	{
		char buff[RECVBUFF];
		int ret;
		memset(buff,0,sizeof(buff));
		case EsockStat_quiet:
			printf("no data coming\n");
			break;
		case EsockStat_readyToRead:
			printf("data coming in\n");
			if(0>=(ret=connection->recv(connection,buff,RECVBUFF)))
			{
				EPRINT("Poll func was lying!, no data received. Returned %d",ret);
			}
			DPRINT("Recvd :%s",buff);
			memset(buff,0,sizeof(buff));
			tim.tv_sec=10;
			tim.tv_usec=0;

			if(0<(ret=connection->non_block_recv(connection,buff,RECVBUFF,tim)))
			{
				DPRINT("recvd more: %s",buff);
			}
			else
			{
				WPRINT("Next recv returned %d",ret);
			}
			break;
		case EsockStat_readyToWrite:
		{
			//http://teotilcan.net/svn/MazBot/"
#ifndef SENDDATA
			char fmt[]="GET %s http/1.1\r\nHost: %s\r\n\r\n";
			char request[500];
			memset(request,0,sizeof(request));
			if(sizeof(request)<snprintf(request,sizeof(request)-1,fmt,URL,HOST))
			{
				EPRINT("hostname too long!");
				return -1;
			}
#else
			char request[] = SENDDATA; 
#endif
			printf("sock ready for writing (no data coming in)\n");
			DPRINT("Sending...\t%s",request);
			if(strlen(request)!=(ret=connection->send(connection,request,strlen(request))))
			{
				if(ret<=0)
				{
					EPRINT("Error occurred when sending!, retval %d",ret);
				}
				else
				{
					WPRINT("odd amount of data sent! asked %d, send returned %d",sizeof(request),ret);
				}
			}
			DPRINT("Trying to receive:");
			if(0>=(ret=connection->recv(connection,buff,RECVBUFF)))
			{
				EPRINT("Blocking: no data received. Returned %d",ret);
			}
			DPRINT("Recvd :%s",buff);
			memset(buff,0,sizeof(buff));
			if(0<(ret=connection->non_block_recv(connection,buff,RECVBUFF,tim)))
			{
				DPRINT("recvd more: %s",buff);
			}
			else
			{
				WPRINT("Next recv returned %d",ret);
			}
			break;
		}
		case EsockStat_Error:
			EPRINT("Error while polling!");
			break;
		default:
			EPRINT("poll returned %d!",pollval);
			MAZZERT(0,"Unknown returnvalue from poll: %d");
			break;
	}
	connection->destroy(&connection);
	return 0;
}
