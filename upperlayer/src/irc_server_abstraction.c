/* ********************************************************************
 *
 * @file irc_server_abstraction.c
 * @brief MazBot "logical layer" for server level task handling.
 *
 *
 * -Revision History:
 *
 *  -0.0.2  20.06.2010/Maz  Splitted server and channel level to own files
 *  -0.0.1  ??.05.2010/Maz  First draft
 *
 *
 *  Lisence info: You're allowed to use / modify this - you're only required to
 *  write me a short note and your opinion about this to Mazziesaccount@gmail.com.
 *  if you redistribute this you're required to mention the original author:
 *  "Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
 *  channel.
 *
 *
 *  PasteLeft 2010 Maz.
 * ********************************************************************/




#include "irc_abstraction_defines.h"
//#include "irc_server_abstraction.h"
#include <config.h>
#include <online_storage.h>
#include <unistd.h>
#include <internal_callbacks.h>
#include <irchelpers.h>

#define RECVBUFF_SIZE 512
static const char LINEFEED[3]={(unsigned char)13,(unsigned int)10,'\0'};

static EMbotCallbackRet srvparsecommand(Sirc_servers *_this,SIRCparserResult *res);
static int queue_callbacks(Sirc_servers *_this,SServerEvents *evnt,SIRCparserResult *res);
static int negotiate_server_conn(Sirc_servers *_this)
{
	size_t nicklen,hostlen,wtflen,firstnamelen,lastnamelen;
    int ret;
    struct timeval tim;
    char buff[RECVBUFF_SIZE];
	char *fmt="NICK %s\r\n";
    char *fmt2="USER Dice FOOBARHOST teotilcan.net :Maz Bot\r\n";
    /* USER name? host? wtf? :%firstname? lastname? */
//    char *fmt2="USER %s %s %s :%s %s\r\n";
	char *request;
    char *nick=NULL;
    char *host="foobarhost";
    char *wtf="wtf.net";
    char *firstname="Maz";
    char *lastname="Bot";

	DPRINT("WORKING \\r=%u \\n=%u, ",(unsigned int)'\r',(unsigned int)'\n');

    tim.tv_sec=3;
    tim.tv_usec=0;
	nicklen=strlen(_this->mycfg->mynick);
	nick=_this->mycfg->mynick;
	if(0==nicklen || nicklen>sizeof(_this->mycfg->mynick))
	{
		PPRINT("Invalid nickname (%s)!",_this->mycfg->mynick);
		return -1;
	}
    hostlen=strlen(host);
    wtflen=strlen(wtf);
    firstnamelen=strlen(firstname);
    lastnamelen=strlen(lastname);
	/* nick+ fmt -%s + '\0' */
	request=malloc(nicklen+strlen(fmt)-1);
	if(NULL==request)
	{
		PPRINT("Malloc FAILED!");
		return -1;
	}
	sprintf(request,fmt,nick);
	DPRINT(" Sending >> %s",request);
	if(strlen(request)!=(ret=_this->connection->send(_this->connection,request,strlen(request))))
	{
		if(ret<=0)
		{
			EPRINT("Error occurred when sending!, retval %d",ret);
            return -1;
		}
		else
		{
		    WPRINT("odd amount of data sent! asked %d, send returned %d",sizeof(request),ret);
		}
	}
    free(request);
	DPRINT("Trying to receive:");
    memset(&buff[0],0,RECVBUFF_SIZE);
    /* TODO: See if these prints should be stored? */
    while(0<(ret=_this->connection->non_block_recv(_this->connection,buff,RECVBUFF_SIZE,tim)))
    {
        DPRINT("recvd: %s",buff);
        memset(&buff[0],0,RECVBUFF_SIZE);
    }
    request=malloc(strlen(fmt2)+nicklen+hostlen+wtflen+firstnamelen+lastnamelen-10+1);
    if(NULL==request)
    {
        PPRINT("negotiate_server_conn(): Failed to allocate USER request!");
        return -1;
    }
    sprintf(request,fmt2,nick,host,wtf,firstname,lastname);
	DPRINT(" Sending >> %s",request);
    if(strlen(request)!=(ret=_this->connection->send(_this->connection,request,strlen(request))))
    {
        if(ret<=0)
        {
            EPRINT("Error occurred when sending!, retval %d",ret);
            return -1;
        }
        else
        {
            WPRINT("odd amount of data sent! asked %d, send returned %d",sizeof(request),ret);
        }
    }
    return 0;
}
static  int ServerConnect(Sirc_servers *_this)
{
    int i;
	for(i=0;EnetwRetval_Ok!=_this->connection->connect(_this->connection,_this->servername,_this->mycfg->port);i++)
	{
		EPRINT("Failed to connect to %s:%u",_this->servername,(unsigned int)_this->mycfg->port);
		if(i<9)
		{
			DPRINT("...Retrying");
			sleep(1);
		}
		else
		{
			PPRINT("Could not connect to %s:%u and errorhandling (and delayed retry) not yet implemented!!",_this->servername,_this->mycfg->port);
			return -1;
		}
	}
	if('\0'==_this->mycfg->mynick)
	{
		memcpy(_this->mycfg->mynick,_this->mycfg->generic_configs->mynick,sizeof(_this->mycfg->mynick)-1);
	}
	MAZZERT('\0'!=_this->mycfg->mynick,"OOOOOONOOOO! BotNick not known in connecting phase! should have been catched when server configs were parsed!");
	if(0!=_this->negotiate_server_conn(_this))
	{
		EPRINT("Could not negotiate connection to server %s",_this->servername);
		return -1;
	}
	PPRINT("TODO: <lowprio> Add connect callback execution to here! (and connect callbacks) %s:%d",__FILE__,__LINE__);
    return 0;
}
/* Main loop for reading data from socket && executing callbacks */
static EServerSuccess process_data(Sirc_servers *_this)
{
	char recvbuff[RECVBUFF_SIZE];
	int i,recvsize;
	struct timeval tim;
	EparserRetVal pret;
	EparserState parstate;
	tim.tv_sec=0;
	tim.tv_usec=1000; /* Millisecond tmo to reduce cpu load ;) */
	/* Go through list of actions to-be-taken determined at last round */
	for(i=0;i<SERVERTASK_MAX;i++)
	{
		if(NULL==_this->servertasklistptr[i])
			break;
		_this->servertasklistptr[i]->args.handle=_this;
		if(NULL!=(*_this->servertasklistptr[i]->callback)(_this->servertasklistptr[i]->args))
		{
            
			EPRINT("Callback at location %p did not yield NULL retval!",_this->servertasklistptr[i]->callback);
			/* Should we return error here? */
			/* Lets assert for debugging period at least :) */
			MAZZERT(0,"Callback FAILED!");
		}
		_this->servertasklistptr[i]=NULL;
	}
	/* Same for channel events? Or should we also add channel event in same list to speed processing? */
	/* Receive next data chunk, if nothing interesting is there, just return */
	if(0>(recvsize=_this->connection->non_block_recv(_this->connection,recvbuff,RECVBUFF_SIZE-1,tim)))
	{
		EPRINT("Error in socket for server %s",_this->servername);
		return EServerSuccess_Disconnected;
	}
	if(0==recvsize)
		return 0;
	recvbuff[recvsize]='\0';
	DPRINT("%s: recvd %s",__FUNCTION__,recvbuff);
	/* Else fill in funcptrs to actions to-be-taken at next round */
	if(EparserRetVal_Ok==(pret=_this->ircparser->feed(_this->ircparser,recvbuff,recvsize, &parstate)) || EparserRetVal_OverFeed == pret)
	{
		/* TODO: Ponder how to handle this correctly? */
		/* I should get the channel where event occurred && pass the data for channel struct to analyze */
		/* Or if it was not any channel, then I should handle it locally at server struct */
		/* TODO: do it :) */
		SIRCparserResult *res;
		while(EparserState_ResultReady == parstate )
		{
			res=(SIRCparserResult *)_this->ircparser->get_result(_this->ircparser,&parstate);
			if(NULL==res)
			{
				WPRINT("NULL result from parser!");
			}
			else
			{
//				char *prefix;
				//char *cmd;
				EMbotCallbackRet chanhandled=EMbotCallbackRet_NotHandled;
				int i;
//				prefix=res->getprefix(res);
				DPRINT("server %s parsing command, amnt of chans %u",_this->servername,_this->amntofchannels);
				for(i=0;i<_this->amntofchannels;i++)
				{
					if((chanhandled=_this->channels[i]->parsecommand(_this->channels[i],res)))
					{
						if(EMbotCallbackRet_InternalHandled==chanhandled)
						{
							/* Channel handled, and no other chan should be interested */
							break;
						}
						else if(EMbotCallbackRet_InternalCbFail==chanhandled) 
						{
							EPRINT
							(
									"Channel %s (Internal callback )FAILED to process IRC command!",
									_this->channels[i]->channel
							);
							break;
						}
						else if(EMbotCallbackRet_InternalCbFailFatal==chanhandled)
						{
							PPRINT("FATAL error while handling Channel %s command!",_this->channels[i]->channel);
							return EServerSuccess_FuckdUp;
						}
						else if(EMbotCallbackRet_UserCbFail==chanhandled)
						{
							EPRINT("User callback for channel %s FAILED!",_this->channels[i]->channel);
						}
						else if(EMbotCallbackRet_UserHandled==chanhandled)
						{
							DPRINT("User callback for channel %s handled",_this->channels[i]->channel);
						}
					}
				}
				if(EMbotCallbackRet_InternalHandled==chanhandled)
				{
					/* Channels handled this, I have nothing to do */
					res->gen.free((SparserResult **)&res);
					continue;
				}
				/* Check server and generic configs */
				if((chanhandled=_this->srvparsecommand(_this,res)))
				{
					if(chanhandled==1)
					{
						DPRINT("Server %s parser command successfully",_this->servername);
					}
					else if(chanhandled<0)
					{
						EPRINT("Server %s FAILED to process IRC command!",_this->servername);
					}
				}
				res->gen.free((SparserResult **)&res);
			}	
			DPRINT("Result handled, trying to get next result!");
		}
	}
	else if(EparserRetVal_InsufficientFeed == pret)
	{
		return 0;
	}
	else
	{
		EPRINT("Irc parser returned error for server %s",_this->servername);
//		return 0;
	}
	if(_this->TxQueueFlush(_this))
	{
		EPRINT("Failed to flush tx_queue, assuming disconnect!");
		return EServerSuccess_Disconnected;
	}
	return 0;
}

static SircUser * static_server_user_find(Sirc_servers *_this,char *nick)
{
    SircUser *found=NULL;
    int i;
    for(i=0;i<_this->mycfg->useramnt;i++)
    {
        MAZZERT(NULL!=_this->mycfg->users[i],"user amount and user array imbalance!");
        if(!strcmp(_this->mycfg->users[i]->nick,nick))
        {
            found=_this->mycfg->users[i];
            break;
        }
    }
    return found;
}

static int TxQueueFlush(struct Sirc_servers *_this)
{
	SUpperlayerIrcMsg **msgstructpp;
	while(NULL!=(msgstructpp=buff_xxx_peep(_this->tx_queue,TX_QUEUE_SIZE)))
	{
		int retval;
		SUpperlayerIrcMsg *msgstruct=*msgstructpp;
		if(0>(retval=_this->connection->send(_this->connection,msgstruct->msg,msgstruct->sendsize)))
		{
			/* MsgSend FAILED!!! */
			return -1;
		}
		if(retval!=msgstruct->sendsize)
		{
			WPRINT("Requested send size was %d, send returned %d!",msgstruct->sendsize,retval);
		}
		free(msgstruct->msg);
		free(msgstruct);
	}
	return 0;
}
char *Mbot_user_getmynick(void *handle)
{
	Sirc_servers *_this=(Sirc_servers *)handle;
	char *mynick;
	char *retnick;
	size_t nicklen;
	mynick=_this->mycfg->mynick;
	if(NULL==handle)
	{
		EPRINT("invalid %s() request from user callback, NULL handle!",__FUNCTION__);
		return NULL;
	}
	nicklen=strlen(mynick);
	retnick=malloc(nicklen+1);
	if(NULL==mynick)
	{
		PPRINT("%s(): MALLOC FAILED!",__FUNCTION__);
		return NULL;
	}
	memcpy(retnick,mynick,nicklen);
	retnick[nicklen]='\0';
	return retnick;
}
int Mbot_user_irc_send(void *handle,char *msg, size_t len)
{
	Sirc_servers *_this=(Sirc_servers *)handle;
	if(NULL==handle || NULL==msg || 0==len)
	{
		EPRINT("invalid user_irc_send() request from user callback. (Either NULL params, or zero messagelenght");
		return -1;
	}
	
	return _this->IRCsend(_this,len,msg);
}

static int IRCsend(Sirc_servers *_this,size_t sendsize, char *senddata)
{
	SUpperlayerIrcMsg *msgstruct;
//	char *msg;
	if(NULL==_this || 0==sendsize || NULL==senddata)
	{
		EPRINT("Cannot send, NULL ptr in %s (%s:%d)",__FUNCTION__,__FILE__,__LINE__);
		return -1;
	}
	msgstruct=malloc(sizeof(SUpperlayerIrcMsg));
	if(NULL==msgstruct)
	{
		PPRINT("%s() Malloc FAILED!",__FUNCTION__);
		return -1;
	}
	msgstruct->sendsize=sendsize;
	msgstruct->msg=malloc(sendsize+1);
	if(NULL==msgstruct->msg)
	{
		PPRINT("%s(): Malloc FAILED!",__FUNCTION__);
		return -1;
	}
	memcpy(msgstruct->msg,senddata,sendsize);
	msgstruct->msg[sendsize]='\0';
	buff_xxx_grope(_this->tx_queue,&msgstruct,TX_QUEUE_SIZE);
	return 0;
}

Sirc_servers *InitSirc_server(SServerConfigs *servercfg)
{
	Sirc_servers *_this;
	int i=0;

	MAZZERT(NULL!=servercfg,"NULL ptr in InitSirc_server()");
	_this=calloc(1,sizeof(Sirc_servers));
	if(NULL==_this)
	{
		PPRINT("calloc FAILED");
		return NULL;
	}

    _this->mytype=EupperlayerStructType_Server;
	_this->ServerConnect=&ServerConnect;
    _this->negotiate_server_conn=&negotiate_server_conn;
	_this->process_data=process_data;
	_this->srvparsecommand=&srvparsecommand;
    _this->static_server_user_find=&static_server_user_find;
	_this->queue_callbacks=&queue_callbacks;
	_this->IRCsend=&IRCsend;
	_this->TxQueueFlush=&TxQueueFlush;

	/* Prepare TX queue for C-callbacks, and internal callbacks as well - I have a TODO for centralized msg sending... */
	_this->tx_queue=Undress_xxx(sizeof(SUpperlayerIrcMsg *),TX_QUEUE_SIZE);
	if(NULL==_this->tx_queue)
	{
		PPRINT("FAILED to create TX queue for server %s",_this->servername);
		return NULL;
	}

	/* Prepare irc stream parser for use */
	_this->ircparser=ParserInit(EparserType_Irc);
	if(NULL==_this->ircparser)
	{
		PPRINT("OhNo! IrcServerStructInit():irc parser init FAILED!");
		free(_this);
//		goto error_out;
		return NULL;
	}
	_this->online_users=OnlinestorageInit(servercfg->domain,"srvdummy");
	if(NULL==_this->online_users)
	{
		PPRINT("FAILED to init online user storage!");
		return NULL;
	}
	/* TODO: Store events somehow! */
	/* Maybe not, maybe it is enough to have the serveconfigs stored here? Let's see how good the cfg struct is for fast accesses... */
/*	_this->amntofevents=servercfg->amntofevents;
*/
	/* Allocate and prepare channels */
	/* Maybe copying the data would be better? Then the alloc-free procedure would be cleaner... */
	/* however this is faster , so lets keep it like this for now... */
	_this->mycfg=servercfg;
	if('\0'==servercfg->mynick[0])
	{
		if('\0'==servercfg->generic_configs->mynick[0])
		{
			EPRINT("No nick found for me!");
			return NULL;
		}
		memcpy(&(_this->mycfg->mynick[0]),&(servercfg->generic_configs->mynick[0]),strlen(servercfg->generic_configs->mynick));
	}

	_this->channels=calloc(_this->mycfg->amntofchannels,sizeof(Sirc_channels *));
	if(NULL==_this->channels)
	{
		PPRINT("OhNo! IrcServerStructInit(): calloc FAILED!");
		_this->ircparser->uninit(&_this->ircparser);
		free(_this);
		return NULL;
	}
	/* Store own domainname */
	memcpy(_this->servername,servercfg->domain,sizeof(Tircserver)-1);
	_this->servername[sizeof(Tircserver)-1]='\0';
	/* Prepare IRC connection */
	_this->connection=connInit(EstructType_TCPconn);
	if(NULL==_this->connection)
	{
		PPRINT("Failed to initialize connection, out of mem?? (aborting)");
		return NULL;
	}
	if(0!=_this->ServerConnect(_this))
	{
		EPRINT("Failed to connect to %s:%u, later retry not yet implemented=> giving up.",_this->servername,(unsigned int)_this->mycfg->port);
		return NULL;
	}
    /* After the ServerConnect we have lots of stuff in input buffer, but we cannot extract it before channel structs are ready since
     * there may have been autojoin channel(s)
     */
	for(i=0;i<_this->mycfg->amntofchannels;i++)
	{
		_this->channels[i]=initSirc_Sirc_channels(servercfg->channels[i],_this);
		if(NULL==_this->channels[i])
		{
			WPRINT("IrcServerStructInit(): initSirc_Sirc_channels( FAILED for channel %s!",servercfg->channels[i]->chan);
			PPRINT("Recovery action for failed chan init not yet done!");
			MAZZERT(0,"Cannot continue");
		}
		_this->amntofchannels++;
//		_this->channels[i]->load_known_users(_this->channels[i]);
	}
	/* Store user information actually, user info is already available in cfg struct, let's leave it as is for now && later improve it if needed.*/
	EPRINT("Not FULLY implemented yet - add callbackregs!");
	return _this;
}
/*
static int IrcServer_GetChannelsToJoin(Sirc_servers *_this);

static int irc_server_find_chan_no(Sirc_servers *_this,char *chann)
{
	int chanid=-1;
	int i;
	unsigned int chanlen;
	MAZZERT(NULL!=_this && NULL != chann,"NULL param given to irc_server_find_chan_no()!");
	chanlen=strlen(chann);
	for(i=0;i<_this->chanamt;i++)
	{
		if(strlen(_this->channels[i]->channel)!=chanlen)
			continue;
		if(!memcmp(_this->channels[i]->channel,chann,chanlen))
		{
			chanid=i;
			break;
		}
	}
	return chanid;
}
*/
/*
static int irc_server_add_channel_def_events(Sirc_servers *_this,char *chann,FILE *readfile)
{
*/
	/* find channel matching chann */
/*
int chanId;
	if(0>(chanId=_this->find_chan_no(_this,chann)))
	{
		EPRINT("Server %s - configs for channel %s given, but no such channel asked to be joined!",_this->connection->servername,chann);
		return -1;
	}
	return (_this->channels[chanId])->add_channel_def_events(_this->channels[chanId],readfile);
}

static int irc_channel_add_channel_def_events(Sirc_channels *_this,FILE *readfile)
{
	return _this->events->parse_def_events(_this->events,readfile);
}

int irc_server_handshake(Sirc_servers *_this,Sparser *irc_parser)
{

}

int irc_server_join_channels(Sirc_servers *_this,Sparser *irc_parser)
{
	int channo;
	for(channo=0;channo<_this->chanamt;channo++)
	{
		if(0>_this->channels[channo]->irc_channel_join(_this->channels[channo],_this->connection,irc_parser))
		{
			WPRINT("Joining to channel FAILED, arming reconn timer");
			_this->channels[channo]->arm_reconn_timer(_this->channels[channo]);
		}
		else
		{
*/
			/* Join msg sent */
			//Add join success resp in "things to be listened" list
			//Add names list in "things to be listened" list
/*
		}
	}
}

int irc_channel_send_whois(Sirc_channels *_this,Sconn *connection)
{

}
*/
/* this should clean the userstorage and arm the reconnect timer(?) */
/*
void irc_channel_disconnected_flush()
{
	
}
*/
/* This should clean the channels and arm reconn timer(s) */
/*
void irc_server_disconnected_flush()
{

}
*/
/* this will be called if join fails? */
/*
void irc_channels_arm_reconn_timer()
{

}
*/
/* This will be called if connect has failed, or if server disconnects */
/*
void irc_server_arm_reconn_timer()
{

}

int irc_channel_join(Sirc_channels *_this,Sconn *connection)
{
	//send join req,
	char joinreq[IRC_MSG_MAX];
	MAZZERT(IRC_MSG_MAX>=snprintf(joinreq, IRC_MSG_MAX,"JOIN %s\r\n",_this->channel),"NOOOOOO!! Impossible, channame is checked to be valid!");
	if(-1==connection->send(connection,joinreq,strlen(joinreq)))
	{
		EPRINT("Sending join req FAILED!!");
		return -1;
	}
	//Add names list to "interested in output" list
}

int irc_channel_analyze_online_list(Sirc_channels *_this,char *onlinelist,Sconn *connection)
{
	//whois online users
	
	//init online storage

}
*/
/* We do not need to store servername here, we have it in server struct, which holds the info.
 * We can give it as argument in call to initialize user storage
 */
/* Checks generic and server events for callbacks to be executed */
/* Returns 0 if there was nothing to do, 1 if event was found && handled successfully, negative no if something went wrong */
const char *known_commands[known_commands_amnt]={"JOIN","PART","PRIVMSG"};
/* IDENTIFY - used to give password matching the nick => change status to identified in online storage */

static const char *srv_known_inter_cmm[]={"PING","QUIT",END_OF_MOTD,"NICK"};

/* Add here handling of internal join cb too? Eg. check if joiner is known in glob, but unknown in online storage
, and if yes => request identification 
  See chan_handle_userlist_user().
*/

static int queue_callbacks(Sirc_servers *_this,SServerEvents *evnt,SIRCparserResult *res)
{
	SServerCallbackList *cbtoadd;
/*
 typedef struct SServerCallbackList
 {   
         ServerCallbackF callback;
		         SServerCallbackArgs args;
				         struct SServerCallbackList *next;
						 }SServerCallbackList;
typedef struct SServerCallbackArgs
{   
    void *handle;  
         Currently this will probs be pointer to connection object, but things may change... 
         Some day it may well be pointer to server data, or even something completely else.
         Anyways, one should never use it to anything else but being a handle to use with
         send/requestdata/XXX macros/functions offered by MazBot's support libraries.
																							                    
    char *raw_data;  Raw data as server passed it 
    SIRCparserResult *parsed_data;  Parsed irc data in format explained at APIs/parsers_api.h 
    void *userdataptr;               Pointer to data user provided when registering the callback 
}SServerCallbackArgs;

 */
	cbtoadd=evnt->cblist;
	while(NULL!=cbtoadd)
	{
		cbtoadd->args.handle=_this;
		if(get_default_cb(evnt->eventType)==cbtoadd->callback)
		{
			/* Format args to suit default events */
			/* I know, this is ugly hack, but since I have no idea what I was thinking about, (If I did think at all), I'll do this way...*/
			/* And yes, I do not think it is reasonable to create own thread for sending a message, 
			 * creation overhead would neglect benefits*/
			/* I am planning to create bunch of threads in "worker pool" ready to be assigned to exec the callback, so when the pool is
			 * done, then I may reconsider this. On multiproc machines it might benefit if network conn is bad enough... */
			cbtoadd->args.userdataptr=evnt;
			cbtoadd->args.parsed_data=res;
			(*cbtoadd->callback)(cbtoadd->args);
		}
		else
		{
			/* TODO: Execute user callbacks in own threads! */
			cbtoadd->args.raw_data=NULL;
			cbtoadd->args.parsed_data=res;
			(*cbtoadd->callback)(cbtoadd->args);
		}
		cbtoadd=cbtoadd->next;
	}
	return 0;
}

static EMbotCallbackRet srv_handle_join_cmm(Sirc_servers *_this,SIRCparserResult *res)
{
	int i;
	char *nick;
	char *mask;
	/* 0 defaults to nothing done */
	int retval=0;
//	SMbotOnlineUsers *usr;
	SServerEvents *evnt=NULL;
//	SServerCallbackList *cbtoadd;

	if(_this->mycfg->amntofevents>0)
	{
		for(i=0;i<_this->mycfg->amntofevents && NULL!=(evnt=_this->mycfg->events[i]);i++)
		{
			if(evnt->eventType != EMbotcallbackEventType_LocalJoin && evnt->eventType != EMbotcallbackEventType_WebJoin)
			{
				/* non join event => check next event from list */
				continue;
			}
			/* Correct type => proceed with investigations */
			/* inputchan => that's stupid. Drop support for creating server events with input chan specified - unless
			 * the input chan can say "channel which is NOT X"
			 */
			/* User filter next */
			/* TODO: */
			if(copyprefixtonickmask(res->getprefix(res),&nick,&mask))
			{
				EPRINT("%s: Failed to parse nick&mask from command prefix!",__FUNCTION__);
				return EMbotCallbackRet_UserCbFail;
			}
			if(is_required_level_found(evnt,_this->online_users,nick))
			{
					queue_callbacks(_this,evnt,res);
			}
		}
	}
	return retval;
}
static EMbotCallbackRet srv_handle_part_cmm(Sirc_servers *_this,SIRCparserResult *res)
{
	int i;
	char *nick;
	char *mask;
	/* 0 defaults to nothing done */
	int retval=0;
//	SMbotOnlineUsers *usr;
	SServerEvents *evnt=NULL;
//	SServerCallbackList *cbtoadd;

	if(_this->mycfg->amntofevents>0)
	{
		for(i=0;i<_this->mycfg->amntofevents && NULL!=(evnt=_this->mycfg->events[i]);i++)
		{
			if(evnt->eventType != EMbotcallbackEventType_LocalPart && evnt->eventType != EMbotcallbackEventType_WebPart)
			{
				/* non join event => check next event from list */
				continue;
			}
			DPRINT("PART event found for server %s, checking user restrictions...",_this->servername);
			/* Correct type => proceed with investigations */
			/* inputchan => that's stupid. Drop support for creating server events with input chan specified - unless
			 * the input chan can say "channel which is NOT X"
			 */
			/* User filter next */
			/* TODO: */
			if(copyprefixtonickmask(res->getprefix(res),&nick,&mask))
			{
				EPRINT("%s: Failed to parse nick&mask from command prefix!",__FUNCTION__);
				return EMbotCallbackRet_UserCbFail;
			}
			if(is_required_level_found(evnt,_this->online_users,nick))
			{
				DPRINT("User Ok => Adding callback for execution");
				queue_callbacks(_this,evnt,res);
			}
		}
	}
	return retval;
}

static EMbotCallbackRet srv_handle_pm_cmm(Sirc_servers *_this,SIRCparserResult *res)
{
	int i,j;
	int startindex;
	size_t rawlen;
	char *nick;
	char *mask;
	char *pm_tgt;
	char *mynick=_this->mycfg->mynick;
	/* 0 defaults to nothing done */
	int retval=0;
//	SMbotOnlineUsers *usr;
	SServerEvents *evnt=NULL;
//	SServerCallbackList *cbtoadd;

	pm_tgt=res->getparam(res,1);
	if(NULL==pm_tgt)
	{
		EPRINT("Failed to determine PM tgt at %s:%d!",__FILE__,__LINE__);
		return EMbotCallbackRet_InternalCbFail;
	}
	if(_this->mycfg->amntofevents>0)
	{
		for(i=0;i<_this->mycfg->amntofevents && NULL!=(evnt=_this->mycfg->events[i]);i++)
		{
			if(evnt->eventType != EMbotcallbackEventType_LocalTxtEvent && evnt->eventType != EMbotcallbackEventType_WebTxtEvent)
			{
				/* non text event => check next event from list */
				continue;
			}
			DPRINT("XXXXXXXXXXXXXXXX pm tgt is %s, right?",pm_tgt);
			DPRINT("Checking event's restrictions for PM origin");
			/* Check if event has requirements for channel it was sent through */
			if(NULL!=evnt->inputsrc.inputname)
			{
				if(strcmp(evnt->inputsrc.inputname,pm_tgt))
				{
					DPRINT
					(
						"Discarding event %d since PM was sent to %s, but event required inputname %s",
						evnt->eventId,
						pm_tgt,
						evnt->inputsrc.inputname
					);
					continue;
				}
			}
			else if( 0!=evnt->inputsrc.inputsource && evnt->inputsrc.inputsource!=EMbotEventLocation_CPM)
			{
				if(!strcmp(mynick,pm_tgt))
				{
					/* This was a PM straight to me */
					if(evnt->inputsrc.inputsource!=EMbotEventLocation_Privmsg)
						continue;
				}
				else
				{
					/* This was PM to channel pm_tgt */
					if(evnt->inputsrc.inputsource!=EMbotEventLocation_Chan)
						continue;
				}
			}

			/* Okay, let's analyze inputed chars... This takes time, but most of the pkgs shall be fropped here */
			DPRINT("Analyzing input string against event's triggerstring...");
			PPRINT("TODO: Add wildcard support!");
			rawlen=strlen(res->raw_data);
			startindex=0;
			for(j=2;j<rawlen;j++)
			{
				if(':'==res->raw_data[j])
				{
					startindex=j+1;
					break;
				}
			}
			if(0!=startindex)
			{
				if(strcmp( &(res->raw_data[startindex]),evnt->trigger_string))
				{
					DPRINT("IRC data='%s', evnt data='%s' => wont exec callback",&(res->raw_data[startindex]),evnt->trigger_string);
					continue;
				}
				else
				{
					DPRINT("IRC data='%s', evnt data='%s' => MATCH, checking user privs",&(res->raw_data[startindex]),evnt->trigger_string);
				}
			}
			else
			{
				EPRINT("Error while handling raw_data from parserresult at %s:%d",__FILE__,__LINE__);
				return EMbotCallbackRet_InternalCbFail;
			}


			/* What is the fastest way to get rid of most of unnecessary checks? */
			/* I'm expecting the bot shall hang in here for quite a bit... The IRC is made for text messages for fuck's sake */
			DPRINT("PM event found for server %s, checking user restrictions...",_this->servername);
			/* Correct type => proceed with investigations */
			/* inputchan => that's stupid. Drop support for creating server events with input chan specified - unless
			 * the input chan can say "channel which is NOT X"
			 */
			/* User filter next */
			/* TODO: */
			if(copyprefixtonickmask(res->getprefix(res),&nick,&mask))
			{
				EPRINT("%s: Failed to parse nick&mask from command prefix!",__FUNCTION__);
				return EMbotCallbackRet_UserCbFail;
			}
			if(is_required_level_found(evnt,_this->online_users,nick))
			{
				DPRINT("User Ok => Adding callback for execution");
				queue_callbacks(_this,evnt,res);
			}
		}
			
	}
	return retval;














	PPRINT("callback filtering/exec not properly implemented for srv PRIVMSG event => TODO%s:%d",__FILE__,__LINE__);
	return 0;

}




static EMbotCallbackRet srvparsecommand(Sirc_servers *_this,SIRCparserResult *res)
{
	int i;
	//unsigned int cmd_found=0;
	EMbotCallbackRet retval=0;
	static int (*srvcmhnd[known_commands_amnt])(Sirc_servers *_this,SIRCparserResult *res) =
	{
		&srv_handle_join_cmm,
		&srv_handle_part_cmm,
		&srv_handle_pm_cmm
	};
	static EMbotCallbackRet (*inter_srvcmhnd[srv_known_inter_cmm_amnt])(Sirc_servers *_this,SIRCparserResult *res) =
	{
		&srv_handle_ping,
        &srv_handle_quit,
		&srv_handle_endOfMotd,
		&srv_handle_nick

	};

	for(i=0;i<srv_known_inter_cmm_amnt;i++)
	{
		if(!strcmp(srv_known_inter_cmm[i],res->getcmd(res)))
		{
			DPRINT("Server %s, handling cmm %s",_this->servername,res->getcmd(res));
			if((retval=inter_srvcmhnd[i](_this,res)))
			{
				if(retval==EMbotCallbackRet_InternalCbFailFatal)
				{	
					PPRINT("Fatal error when handling internal callback for %s command on server %s!",srv_known_inter_cmm[i],_this->servername);
					return retval;
				}
				else if(retval==EMbotCallbackRet_InternalCbFail)
				{
					EPRINT("Failed to execute internal server cb!");
					return retval;
				}
				else if(retval==EMbotCallbackRet_InternalHandled)
				{
					DPRINT("nternal callback for %s command on server %s HANDLED",srv_known_inter_cmm[i],_this->servername);
					return retval;
				}
				else if(retval==EMbotCallbackRet_AllowInternalOnly)
				{
					return retval;
				}
				else
				{
					;
				}
					
			}
			break;
		}
	}

	for(i=0;i<known_commands_amnt && retval >=0;i++)
	{
		if(!strcmp(known_commands[i],res->getcmd(res)))
		{
			retval=srvcmhnd[i](_this,res);
			break;
		}
	}
    PPRINT("Add raw events handling here (always) - TODO: Add raw events in bot! %s:%d",__FILE__,__LINE__);
	/* Check raw events here (always) - TODO: Add raw events! */
	return retval;
}
/*
Sirc_servers * IrcServerStructInit(Sconn *connection)
{
	Sirc_servers *_this=NULL;
	if(NULL==connection)
	{
		PPRINT("IrcServerStructInit(): NULL connection given!");
		goto error_out;
	}
	_this=calloc(1,sizeof(Sirc_servers));
	if(NULL==_this)
	{
		PPRINT("OhNo! IrcServerStructInit(): calloc FAILED at %s:%d!",__FILE__,__LINE__);
		goto error_out;
	}
*/
	/* init parser */
/*	_this->ircparser=ParserInit(EparserType_Irc);
	if(NULL==_this->ircparser)
	{
		PPRINT("OhNo! IrcServerStructInit():irc parser init FAILED!");
		goto error_out;
	}
	_this->connection=connection;
	_this->GetChannelsToJoin=&IrcServer_GetChannelsToJoin;
	if(_this->GetChannelsToJoin(_this)<=0)
	{
		PPRINT("IrcServerStructInit(): Getting channels for server %s FAILED",_this->connection->servername);
		goto error_out;
	}
*/
/*
 * _this->onlineUsers=...;
	if(NULL==_this->onlineUsers)
	{
		PPRINT("IrcServerStructInit(): OnlineUserStorage init FAILED!");
		goto error_out;
	}
*/
/* It may be that we only had one global callbackhandler and eventhandler. In that case these must be obtained by macro, or given as args */
/* TODO: Re-Implement callback and eventhandlers ! */
/*
 * _this->callbackhandler=...;
	if(NULL==_this->callbackhandler)
	{
		PPRINT("IrcServerStructInit(): Callback handler init FAILED!");
		goto error_out;
	}
	_this->eventhandler=...;
	if(NULL==_this->eventhandler)
	{
		PPRINT("IrcServerStructInit(): Event handler init FAILED!");
		goto error_out;
	}
*/
	/* Set rest of the funcptrs */
/* TODO: Implement!
	_this->disconnected_flush=&;
	_this->arm_reconn_timer=&;
	_this->pollEvents=&;
	_this->=&;
*/
/*
	_this->find_chan_no=&irc_server_find_chan_no;
	_this->add_channel_def_events=&irc_server_add_channel_def_events;
	return _this;
error_out:
	if(NULL!=_this)
	{
		if(NULL!=_this->ircparser)
		{
			if(0<_this->chanamt)
			{
				int i;
				for(i=0;i<_this->chanamt;i++)
				{
					if(NULL!=_this->channels[i])
						free(_this->channels[i]);
*/
					/* TODO: _this->channels[i]->uninit(&_this->channels[i]); */
/*				}
				free(_this->channels);
			}
			_this->ircparser->uninit(&(_this->ircparser));
		}
		free(_this);
	}
	return NULL;
}
*/
/*
static int IrcServer_GetChannelsToJoin(Sirc_servers *_this)
{
	char *srvchans;
	size_t srvchanlen;
	int srvamnt;
	int chanamnt=-1;
	CexplodeStrings srvchanploder;
	char *tmp;
	srvchans=GetConfig(E_serverchans,&srvchanlen);
	if(NULL==srvchans||srvchanlen<1)
	{
		PPRINT("Fatal error while getting channel configuration for server %s!",_this->connection->servername);
		return -2;
	}
	srvamnt=Cexplode(srvchans,":",&srvchanploder);
	if(srvamnt<1)
	{
		PPRINT("Mysterious error occurred at %s:%d, Cexplode retruned %d",__FILE__,__LINE__,srvamnt);
		return -2;
	}
	tmp=Cexplode_getfirst(&srvchanploder);
	while(tmp!=NULL)
	{
		if(strlen(tmp)<strlen(_this->connection->servername))
		{
			tmp=Cexplode_getnext(&srvchanploder);
			continue;
		}
		if(!memcmp(tmp,_this->connection->servername,strlen(_this->connection->servername)))
		{
*/
			/* Matching server found! */
/*			CexplodeStrings chanploder;
			chanamnt=Cexplode(tmp,",",&chanploder);
			if(chanamnt<1)
			{
				PPRINT("Mysterious error occurred at %s:%d, Cexplode retruned %d",__FILE__,__LINE__,srvamnt);
				return -2;
			}
			chanamnt--;
			if(chanamnt>0)
			{
				char *chan;
				int j;
				_this->channels=calloc(chanamnt,sizeof(Sirc_channels *));

				for(j=0;j<chanamnt;j++)
				{
					chan=Cexplode_getNth(j+2,&chanploder);
					MAZZERT(chan!=NULL,"OhNo! cexplode ate one channel??? !");
					if(strlen(chan)>=IRC_CHANNEL_MAX)
					{
						EPRINT("Too long channelname (%s) given in configs! (%s:%d)",chan,__FILE__,__LINE__);
						WPRINT("Ignoring channel %s",chan);
						chanamnt--;
						j--;
						free(Cexplode_removeNth(j+2,&chanploder));
						continue;
					}
					if(NULL==(_this->channels[j]=initSirc_Sirc_channels(chan)))
					{
						PPRINT("Channel struct init FAILED!");
						return -2;
					}
//					strcpy(_this->channels[j],chan);
				}
			}
			break;
		}
		else
		{
*/
			/* No matching server found => probs connection failure? */
			//TODO: Change connection making to store server info even if connect fails,
			//Just do some 'inactive' state && launch reconnect timer.
/*
tmp=Cexplode_getnext(&srvchanploder);
		}
	}
	_this->chanamt=chanamnt;
	return chanamnt;
}
*/

