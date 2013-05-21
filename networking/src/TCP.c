/* ********************************************************************
 *
 * @file TCP.c
 * @brief a functions to use TCP connection via Sconn pointer.
 *
 *
 * -Revision History:
 *
 *  - 0.0.4 20.07.2009/Maz	Separated read and write poll to own functions.
 *  						(Fixed bad design :) )
 *  - 0.0.3 12.07.2009/Maz  Added send function, fixed a few things.
 *  - 0.0.2 12.07.2009/Maz  Decided to create a C++ style "virtual class" thingy
 *                  		which will allow using different connections underneath
 *  - 0.0.1 10.07.2009/Maz  First draft
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


#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <definitions.h>
#include <generic.h>
#include "networking.h"
#include "TCP.h" 

#define TCPSANITYCHECK(str) {\
	MAZZERT(NULL!=(str),"NULL tcpobj detected!");\
	MAZZERT((str)->connection.type==EstructType_TCPconn,"Non TCPconn type obj used!");\
}

size_t TCPSend(Sconn *_this_,void *data,size_t datasize)
{
	STCPconn *_this=(STCPconn *)_this_;
	TCPSANITYCHECK(_this);
	if(_this->connection.state != EconnState_connected)
	{
		EPRINTC(PrintComp_netw,"Send attempted on non connected TCP socket!");
		return -1;
	}
	return send(_this->connection.sock, data, datasize, 0);
}

size_t TCPrecv(Sconn *_this_,void *buff,size_t buflen)
{
	int rbytes;
	STCPconn *_this=(STCPconn *)_this_;
	TCPSANITYCHECK(_this);
	if(_this->connection.state!=EconnState_connected)
	{
		WPRINTC(PrintComp_netw,"read attempted on non connected TCP socket!");
		return -1;
	}
	MAZZERT(buff!=NULL,"Null pointer given to recv as buffer!");
rerecv:
	rbytes=recv(_this->connection.sock, buff, buflen,0);
	if(-1==rbytes)
	{
		if(EINTR==errno)
			goto rerecv;
		else
		{
			perror("recv failed!");
			EPRINTC(PrintComp_netw,"Receive failed on TCP sock");
		}
	}
	if(0>=rbytes)
		_this->connection.state=EconnState_disconnected;
	return rbytes;
}

EnetwRetval TCPconnect(Sconn *_this_,char *domain,unsigned short int port)
{
	struct addrinfo *serverinfo;
	int err;
	size_t domainlen;
	STCPconn *_this=(STCPconn *)_this_;
	TCPSANITYCHECK(_this);
	if(NULL==_this||NULL==domain)
	{
		printf("NULL params given to TCPconnect\n");
		return EnetwRetval_InvParam; 
	}
	domainlen=strlen(domain);
	if(domainlen>=sizeof(_this->connection.servername))
	{
		printf("servername %s too long (%u bytes, max %u bytes)",domain,domainlen+1,sizeof(_this->connection.servername));
		return EnetwRetval_InternalError;
	}
	memset(_this->connection.servername,0,sizeof(_this->connection.servername));

	if(_this->connection.state!=EconnState_inited)
	{
		printf("TCPconn object not ready!\n");
		return EnetwRetval_InternalError;
	}
	serverinfo = resolve_host(domain,_this->connection.type);
	if(NULL==serverinfo||NULL==serverinfo->ai_addr)
	{
		printf("Failed to connect to server\n");
		return EnetwRetval_UnableToResolve;
	}
	_this->connection.server=*((struct sockaddr_in *)(serverinfo->ai_addr));
	_this->connection.server.sin_port = htons(port);
	freeaddrinfo(serverinfo);
	if( 
        (err=
			connect
        	(
           		_this->connection.sock,
           		(struct sockaddr *)&(_this->connection.server),
           		sizeof(_this->connection.server)
        	)
		) 
        != 0 
    )
    {
		perror("Connect FAILED!\n");
		EPRINTC(PrintComp_netw,"Address was %s\n",inet_ntoa(_this->connection.server.sin_addr));
		return EnetwRetval_InternalError;
    }
	_this->connection.state = EconnState_connected;
	return EnetwRetval_Ok;
}
/*
int connect_to_server(char *domain,unsigned short int port)
{
    int sock;
	int err;
	struct sockaddr_in server;
	return sock;
}
*/
//XXX redo - bad interface. This does not distinct errors from disconnected sock.
size_t TCPnbrecv(Sconn *_this_,void *buff,size_t buflen,struct timeval tmo)
{
//	int len;
	EsockStat pollret;
	STCPconn *_this=(STCPconn *)_this_;
	TCPSANITYCHECK(_this);
	if(_this->connection.state!=EconnState_connected)
	{
		WPRINTC(PrintComp_netw,"read attempted on non connected TCP socket!");
		return -1;
	}
	MAZZERT(buff!=NULL,"Null pointer given to recv as buffer!");
	if(EsockStat_readyToRead==(pollret=_this->connection.readpoll(_this_,tmo)))
	{
		return _this->connection.recv(_this_,buff,buflen);
	}
	if(EsockStat_quiet==pollret)
	{
		return 0;
	}
	else
		return -1;
}

EnetwRetval TCPcreate_socket(Sconn *_this_)
{
	STCPconn *_this=(STCPconn *)_this_;
	TCPSANITYCHECK(_this);
	if( (_this->connection.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ) < 0 )
	{
		perror("Socket creation failed!");
		return EnetwRetval_InternalError;
	}
	return _this->connection.sock;
}
void TCPuninit(Sconn **_this_)
{
	STCPconn *_this=(STCPconn *)*_this_;
	TCPSANITYCHECK(_this);
	free(_this);
	_this=NULL;
}
STCPconn *TCPinit(void)
{
	STCPconn *conn;
	conn=malloc(sizeof(STCPconn));
	if(NULL==conn)
	{
		perror("Malloc FAILED");
		printf("connect init FAILED at %s:%d",__FILE__,__LINE__);
		return NULL;
	}
	memset(conn,0,sizeof(STCPconn));
	conn->connection.type=EstructType_TCPconn;
	if( (conn->connection.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ) < 0 )
    {
        perror("Socket creation failed!");
		free(conn);
        return NULL;
    }
	conn->connection.send			= &TCPSend;
	conn->connection.recv			= &TCPrecv;
	conn->connection.create_socket	= &TCPcreate_socket;
	conn->connection.non_block_recv	= &TCPnbrecv;
	conn->connection.destroy	    = &TCPuninit;
	conn->connection.connect		= &TCPconnect;
	conn->connection.connectible	= MAZ_TRUE;
	conn->connection.state			= EconnState_inited;
	conn->connection.readpoll		= readpoll;
	conn->connection.writepoll		= writepoll;
	return conn;
}
