#include <irchelpers.h>
#include <pthread.h>
#include <unistd.h>
#include <generic.h>
#include "user_callbacks.h"
#include <networking.h>
#include "Mbot_revchange_cb.h"

/* THIS IS BAD EXAMPLE! DO NOT CREATE THREAD ACCESSING TO ARGS FROM CALLBACK. ELSE YOU MAY CRASH THE BOT IF SERVER STATE CHANGES! */

void *revchangeproc(void *arg)
{
	int i;
	char revision[4];
	char oldrev[4];
	char recvd[1024];
	size_t sendsize;
	const char *httpreq="GET /svn/MazBot/ HTTP/1.0\r\nHost: blackdiam.net\r\nAccept: text/html\r\nKeep-Alive: 300\r\nConnection: keep-alive\r\n\r\n";
	Sconn *conn;
	CexplodeStrings httploder;
	SServerCallbackArgs *args=(SServerCallbackArgs *)arg;
	sendsize=strlen(httpreq);
	DPRINT("Revcheck proc started");

rerecv:
	conn=connInit(EstructType_TCPconn);
	if(NULL==conn)
	{
		EPRINT("Conn init FAILED!");
	}
	conn->connect(conn,"www.blackdiam.net",80);
	conn->send(conn,httpreq,sendsize);
	if(0==conn->recv(conn,recvd,1024))
	{
		conn->destroy(&conn);
		goto rerecv;	
	}
	if(2!=Cexplode(recvd,"<title>",&httploder))
	{
		conn->destroy(&conn);
		goto rerecv;
	}
	else
	{
		char *dummy;
		dummy=Cexplode_getNth(2,&httploder);
		if(NULL==dummy)
		{
			MAZZERT(0,"Unexpected NULL result from Cexplode!!!");
		}
		memcpy(oldrev,&(dummy[9]),3);
		oldrev[3]='\0';
	}
	Cexplode_free(httploder);
	conn->destroy(&conn);
	for(;;)
	{
		sleep(60);
		memset(recvd,0,sizeof(recvd));
	    conn=connInit(EstructType_TCPconn);
    	if(NULL==conn)
	    {
    	    EPRINT("Conn init FAILED!");
	    }
    	conn->connect(conn,"www.blackdiam.net",80);
	    conn->send(conn,httpreq,sendsize);
    	if(0==conn->recv(conn,recvd,1024))
	    {
    	    conn->destroy(&conn);
	        continue;
    	}
	    if(2!=Cexplode(recvd,"<title>",&httploder))
    	{
	        conn->destroy(&conn);
    	    continue;
	    }
    	else
	    {
    	    char *dummy;
        	dummy=Cexplode_getNth(2,&httploder);
	        if(NULL==dummy)
    	    {
        	    MAZZERT(0,"Unexpected NULL result from Cexplode!!!");
	        }
        	memcpy(revision,&(dummy[9]),3);
	        revision[3]='\0';
			if(strcmp(revision,oldrev))
			{
				char *ircmsg;
				const char *ircmsg_fmt="PRIVMSG #Teotilcan :YaY! Svn was updated! My new revision is %s!";
                size_t ircmsglen;
				ircmsg=prepare_for_sending(&ircmsglen,ircmsg_fmt,revision);
				if(NULL!=ircmsg)
				{
					Mbot_user_irc_send(args->handle,ircmsg,ircmsglen);
                    free(ircmsg);
				}
				DPRINT("Revision changed, was %s, is now %s",oldrev,revision);
				memcpy(oldrev,revision,3);
			}
    	}
		Cexplode_free(httploder);
	    conn->destroy(&conn);
	}
	return NULL;
}


void *Mbot_revchange_cb(SServerCallbackArgs args)
{
	static int launched=0;
	pthread_t tid;
	SServerCallbackArgs *argum;
	//Not thread safe!
	if(launched!=0)
	{
		return NULL;
	}
	launched++;
	argum = malloc(sizeof(SServerCallbackArgs));
	if(NULL==argum)
	{
		PPRINT("Malloc FAILED!");
		launched=0;
		return NULL;
	}
	memcpy(argum,&args,sizeof(SServerCallbackArgs));
	if(pthread_create(&tid,NULL,revchangeproc,argum))
	{
		DPRINT("Failed to create thread!");
		launched=0;
	}
	return NULL;
}
