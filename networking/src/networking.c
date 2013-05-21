/* ********************************************************************
 *
 * @file networking.c
 * @brief a wrapper/generic functions for different type of connections.
 * This file contains wrappers to call connection specific functions using the 
 * correctly initialized Sconn* pointer. (Like virtual classes in C)
 *
 *
 * -Revision History:
 * 
 *  - 0.0.3 20.07.2009/Maz	Separated read and write poll to own functions.
 *  						(Fixed bad design :) )
 *  - 0.0.2 12.07.2009/Maz 	Decided to create a C++ style "virtual class" thingy
 *  				which will allow using different connections underneath
 * 	- 0.0.1 10.07.2009/Maz 	First draft
 *
 *
 * 	Lisence info: You're allowed to use / modify this - you're only required to
 * 	write me a short note and your opinion about this to Mazziesaccount@gmail.com.
 * 	if you redistribute this you're required to mention the original author:
 * 	"Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
 * 	channel.
 *
 *
 * 	PasteLeft 2009 Maz.
 * 	*************************************************************/






#include "definitions.h"
#include "networking.h"
#include <generic.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <arpa/inet.h>
#include <string.h> 
#include <sys/types.h>
#include <netdb.h>
#include <TCP.h>


#define WRITEPOLL 1
#define READPOLL 2
#define POLLEMALL 3
/*
connectF connectJMP[SUPP_PROTOCOLS] = 
{
	&tcp_connect_to_server
};
*/

static EsockStat netwpoll(Sconn *_this,struct timeval tmo,int polltype);

Sconn *connInit(EstructType type)
{
	Sconn *conn=NULL;
	if(EstructType_NmbrOf<=(unsigned int)type)
	{
		EPRINTC(PrintComp_netw,"Invalid type given to connInit!\n");
		return NULL;
	}
	switch(type)
	{
		case EstructType_TCPconn:
			conn=(Sconn *)TCPinit();
			break;
		case EstructType_UDPconn:
			MAZZERT(0,"No UDP support implemented yet!");
			break;
		default:
			EPRINTC(PrintComp_netw,"init => type %d",type);
			MAZZERT(0,"The impossible happened - invalid type in init");
			break;
	}
	if(NULL==conn)
	{
		EPRINTC(PrintComp_netw,"Failed to init conn obj!");
	}
	return conn;
}
//#define TESTS_ON
//This could have been done also by checking type and then casting _this to correct member
//&& calling the specific connect function.
//XXX which way is better?
/*
int connect_to_server(Sconn *_this,char *domain,unsigned short int port)
{
	ISTYPESANE(_this);
	ISTYPECONN(_this);
	return connectJMP[_this->type](_this,domain,port);
}
*/
//for non connected protocols
EnetwRetval dummyconnect(Sconn *_this,char *domain,unsigned short int port)
{
	(void)_this;
	(void)domain;
	(void)port;
	return EnetwRetval_Ok;
}
//XXX: fixme!
void uninit(Sconn **_this)
{
	ISTYPESANE(*_this);
//	(*_this)->destroy(*_this);
	free(*_this);
	*_this=NULL;
}

//Generic helper function
//XXX: move to more suitable file
struct addrinfo * resolve_host(const char *host,EstructType type)
{
	int ecode;
    struct addrinfo serverGuess;
    struct addrinfo *serverReal;
	memset(&serverGuess,0,sizeof(struct addrinfo));
	switch(type)
	{
		case EstructType_TCPconn:
			serverGuess.ai_family       = AF_INET;
			serverGuess.ai_socktype		= SOCK_STREAM;
			serverGuess.ai_flags		|= AI_CANONNAME;
			break;
		case EstructType_UDPconn:
			MAZZERT(0,"UDP support not added yet!");
			break;
		default:
			EPRINTC(PrintComp_netw,"Bad type given when resolving host %s",host);
			return NULL;
			break;
	}

	if(0!=(ecode=getaddrinfo(host,NULL,&serverGuess,&serverReal)))
	{
		EPRINTC(PrintComp_netw,"getaddrinfo failed: %s (%s:%d)\n",gai_strerror(ecode),__FILE__,__LINE__);
		return NULL;
	}
	return serverReal;
}

int create_socket(EstructType proto)
{
	int sock;
	switch(proto)
	{
		case EstructType_TCPconn:
			if( (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ) < 0 )
			{
				perror("Socket creation failed!");
				return -1;
			}
			break;
		default:
			MAZZERT(0,"No other protocols but TCP done yet!");
			break;
	}
	return sock;
}
EsockStat writepoll(Sconn *_this,struct timeval tmo)
{
	return  netwpoll(_this,tmo,WRITEPOLL);
}
EsockStat readpoll(Sconn *_this,struct timeval tmo)
{
	 return  netwpoll(_this,tmo,READPOLL);
}
static EsockStat netwpoll(Sconn *_this,struct timeval tmo,int polltype)
{
	int retval;
	EsockStat ret=EsockStat_quiet;
	fd_set rwset,rdset;
	FD_ZERO(&rwset);
	FD_ZERO(&rdset);
	FD_SET(_this->sock,&rwset);
	FD_SET(_this->sock,&rdset);
	switch(polltype)
	{
		case READPOLL:
			retval=select(_this->sock+1,&rdset,0,0,&tmo);
			if(FD_ISSET(_this->sock,&rdset))
				ret=EsockStat_readyToRead;
			break;
		case WRITEPOLL:
			retval=select(_this->sock+1,0,&rwset,0,&tmo);
			if(FD_ISSET(_this->sock,&rwset))
				ret=EsockStat_readyToWrite;
			break;
		case POLLEMALL:
			retval=select(_this->sock+1,&rdset,&rwset,0,&tmo);
			if(FD_ISSET(_this->sock,&rdset))
				ret=EsockStat_readyToRead;
			else if(FD_ISSET(_this->sock,&rwset))
				ret=EsockStat_readyToWrite;
			break;
		default:
			MAZZERT(0,"unrecognized polltype!");
			break;
	}
	if(-1==retval)
	{
		if(errno!=EINTR)
			return EsockStat_Error;
		else
			return EsockStat_quiet;
	}
	return ret;
}


#ifdef TESTS_ON
int main(void)
{
	struct addrinfo *sonera;
	struct sockaddr_in *server;
	sonera=resolve_host("www.sonera.fi");
	if(NULL==sonera||NULL==sonera->ai_addr)
	{
		printf("FAILED");
		return -1;
	}
	server=(struct sockaddr_in *)sonera->ai_addr;
	printf("Sonera: %s\n",inet_ntoa(server->sin_addr));
	if(0>connect_to_server("www.sonera.fi",80))
	{
		printf("FAIL\n");
		return -1;
	}
	printf("PASS\n");
	return 0;
}
#endif
