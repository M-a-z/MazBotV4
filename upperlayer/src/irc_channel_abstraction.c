/* ********************************************************************
 *
 * @file irc_channel_abstraction.c
 * @brief MazBot "logical layer" for channel level task handling.
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
#include <internal_callbacks.h>
#include <irchelpers.h>

static const char *chan_known_inter_cmm[]={"PRIVMSG",USERLIST_USER,"JOIN","PART"};//,"NICK"};//,UNABLE_TO_JOIN};
static const char *chan_known_inter_subcomm_pm[]={"!IDENTIFY","\1VERSION\1"};



static int checkjoinstate(Sirc_channels *_this);
static int chan_handle_pm_subcmms(Sirc_channels *_this,SIRCparserResult *res);
static int chan_handle_join_cmm(Sirc_channels *_this,SIRCparserResult *res);
static int chan_handle_part_cmm(Sirc_channels *_this,SIRCparserResult *res);
static int chan_handle_pm_cmm(Sirc_channels *_this,SIRCparserResult *res);
static EMbotCallbackRet parsecommand(Sirc_channels *_this,SIRCparserResult *res);

static int checkjoinstate(Sirc_channels *_this)
{
	DPRINT("Checking chan %s joinstate...",_this->channel);
	if(1>_this->chan_joined)
	{
		char *joinreq;
		char *joinfmt="JOIN %s";
		size_t reqlen;
		DPRINT("Channel %s not joined => sending joinreq",_this->channel);
		joinreq=prepare_for_sending(&reqlen,joinfmt,_this->channel);
		if(NULL==joinreq)
		{
			PPRINT("Failed to alloc request for joining to channel!");
			return -1;
		}
		DPRINT("%s: Sending joinreq \"%s\"",__FUNCTION__,joinreq);
		if(_this->myserv->IRCsend(_this->myserv,reqlen,joinreq))
		{
			EPRINT("%s(): Could not send joinreq (%s)!",__FUNCTION__,strerror(errno));
			return -1;
		}
	}
	return 0;
}



		
static EMbotCallbackRet chan_handle_pm_subcmms(Sirc_channels *_this,SIRCparserResult *res)
{
	int i;
	char *chan;
	char *cmparg;
	size_t cmplen;
	char *mynick=_this->myserv->mycfg->mynick;
	EMbotCallbackRet retval=EMbotCallbackRet_NotHandled;

	EMbotCallbackRet (*inter_chanpmcmhnd[chan_known_inter_subcomm_pm_amnt])(Sirc_channels *_this,SIRCparserResult *res) =
	{
		&chan_handle_identify_cmm,
		&chan_handle_version_cmm
	};

	DPRINT("chan_handle_pm_subcmms(): Called for chan %s or in privmsg",_this->channel);
	chan=res->getparam(res,1);
	/* This is utterly stupid way. I should parse privmsgs at server level && call only callbacks for correct channel */
	/* Also channels should be accessible via some better way, eg finding channel #foobar should not require doing strcmp() to all channels untill the name matches. TODO: Index channels for example based on first letter */
    PPRINT("TODO: Optimize  %s:%d",__FILE__,__LINE__);
	if(strcmp(chan,_this->channel) && strcmp(mynick,chan))
	{
		DPRINT("Channel %s: Refusing from handling PM on channel %s",_this->channel,chan);
		return EMbotCallbackRet_NotHandled;
	}
	cmparg=res->getparam(res,2);
	/* Eliminate the : which is part of IRC protocol, preceding the message */
	if(NULL==cmparg || '\0'==*cmparg)
	{
		WPRINT("NULL arg 2 in PRIVMSG!!!!!!");
		return EMbotCallbackRet_InternalCbFail;
	}
	cmparg+=1;
	DPRINT("Searching PM subcomm for arg %s",cmparg);
	cmplen=strlen(cmparg);
	
	for(i=0;i<chan_known_inter_subcomm_pm_amnt;i++)
	{
		if(cmplen!=strlen(chan_known_inter_subcomm_pm[i]) || memcmp(cmparg,chan_known_inter_subcomm_pm[i],cmplen))
			continue;
		retval=(*inter_chanpmcmhnd[i])(_this,res);
	}
	return retval;
}
static EMbotCallbackRet chan_handle_join_cmm(Sirc_channels *_this,SIRCparserResult *res)
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
			if(is_required_level_found(evnt,_this->chan_online_users,nick))
			{
					_this->myserv->queue_callbacks(_this->myserv,evnt,res);
			}
			else if(is_required_level_found(evnt,_this->myserv->online_users,nick))
			{
				_this->myserv->queue_callbacks(_this->myserv,evnt,res);
			}
		}
	}
	return retval;

	PPRINT("callback filtering/exec not properly implemented for chan JOIN event => TODO %s:%d",__FILE__,__LINE__);
	return 0;
}
static int chan_handle_part_cmm(Sirc_channels *_this,SIRCparserResult *res)
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
			DPRINT("PART event found for channel %s, checking user restrictions...",_this->channel);
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
			if(is_required_level_found(evnt,_this->chan_online_users,nick))
			{
				DPRINT("User Ok => Adding callback for execution");
				_this->myserv->queue_callbacks(_this->myserv,evnt,res);
			}
			else if(is_required_level_found(evnt,_this->myserv->online_users,nick))
			{
				DPRINT("User Ok => Adding callback for execution");
				_this->myserv->queue_callbacks(_this->myserv,evnt,res);
			}
		}
	}
	return retval;

	PPRINT("callback filtering/exec not properly implemented for chan PART event => TODO %s:%d",__FILE__,__LINE__);
	return 0;

}
static int chan_handle_pm_cmm(Sirc_channels *_this,SIRCparserResult *res)
{
	PPRINT("callback filtering/exec not properly implemented for chan PRIVMSG event => TODO %s:%d",__FILE__,__LINE__);
	return 0;

}
//Sirc_channels *initSirc_Sirc_channels(Tircchannel channel, Tircserver server)
//Should return 0 if there was nothing to do with command
//Should return 1 if command was handled, and no server/other channel can be interested in it.
//Should return negative upon error
//Should return 2 if command was handled, and some other channel/server may be interested too.
static EMbotCallbackRet parsecommand(Sirc_channels *_this,SIRCparserResult *res)
{

	int i;
	int deny_internalcbs=0;
	int deny_usercbs=0;
    EMbotCallbackRet rval;
	//unsigned int cmd_found=0;
//	int retval=0;
	char *cmd=NULL;
	int (*chancmhnd[known_commands_amnt])(Sirc_channels *_this,SIRCparserResult *res) =
	{
		&chan_handle_join_cmm,
		&chan_handle_part_cmm,
		&chan_handle_pm_cmm
	};

	int (*inter_chancmhnd[chan_known_inter_cmm_amnt])(Sirc_channels *_this,SIRCparserResult *res) =
	{
		&chan_handle_pm_subcmms,
		&chan_handle_userlist_user,
		&chan_handle_internal_join_cmm,
        &chan_handle_part,
	//	&chan_handle_nick
//		&chan_handle_internal_joinfail_cmm
	};
    PPRINT("TODO: Fix return values to use EMbotCallbackRet enum correctly!! %s:%d",__FILE__,__LINE__);
	cmd=res->getcmd(res);
	if(NULL==cmd)
	{
		EPRINT("parsecommand(): NULL command from parser!");
		return -1;
	}
	DPRINT("Channel %s checking callbacks for command %s",_this->channel,cmd); 
/* 
 * Internal callback for channels
 */
	DPRINT("Comm '%s' parser_rawdata '%s'",cmd,res->raw_data);

	for(i=0;i<chan_known_inter_cmm_amnt&&!deny_internalcbs;i++)
	{
		if(!strcmp(chan_known_inter_cmm[i],res->getcmd(res)))
		{
			DPRINT("****EXECUTING internal cb for %s",cmd);
			if((rval=inter_chancmhnd[i](_this,res)))
			{
				switch(rval)
				{
					case EMbotCallbackRet_InternalCbFailFatal:
						PPRINT("Internal callback handler returned FATAL error!");
						return rval;
						break;
					case EMbotCallbackRet_InternalCbFail:
						EPRINT("Internal callback handler returned ERROR!");
						break;
					case EMbotCallbackRet_InternalHandled:
						DPRINT("Internal callback handler returned HANDLED => won't execute other callbacks");
						deny_internalcbs=1;
						deny_usercbs=1;
						break;
					case EMbotCallbackRet_AllowInternalOnly:
						DPRINT("Internal callback handler returned AllowInternalOnly => forbidding user callbacks!");
						deny_usercbs=1;
						break;
					case EMbotCallbackRet_AllowUserOnly:
						deny_internalcbs=1;
						DPRINT("Internal allback handler returned AllowUserOnly => skipping rest of internal callbacks!");
						break;
					default:
						EPRINT("Internal callback handler returned %d which should not be possible! This propably is a bug...",rval);
						break;
				}
//				EPRINT("Failed to handle internal callback for %s command on channel %s!",chan_known_inter_cmm[i],_this->channel);
//				return rval;
			}
			/* 
			 * Internal commands can be such that no user callbacks should interefere? 
			 * If this provides to be bad solution, the remove the return from here! 
			 */

//			return rval;
//			If callback reports this action to be not handled by other callbacks => break.
/*            if(1==rval)
    			break;
*/
		}
	}
/* 
 * User registered callbacks for channels
 */
	for(i=0;i<known_commands_amnt && !deny_usercbs >=0 ;i++)
	{
		if(!strcmp(known_commands[i],cmd))
		{
			rval=chancmhnd[i](_this,res);
			break;
		}
	}
	/* Check raw events here (always) - TODO: Add raw events! */
    PPRINT("Check raw events here (always) - TODO: Add raw events! %s:%d",__FILE__,__LINE__);
	return rval;
}
static SircUser * static_channel_user_find(Sirc_channels *_this,char *nick)
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

Sirc_channels *initSirc_Sirc_channels(SChannelConfigs *cfg,Sirc_servers *myserv)
{
	Sirc_channels *_this;
	size_t chanlen;

	MAZZERT(NULL != cfg /* && NULL != server*/ ,"initSirc_Sirc_channels(): NULL channel config given");
	MAZZERT(NULL != myserv,"initSirc_Sirc_channels(): NULL serverptr!");
	chanlen=strlen(cfg->chan);
//	srvlen=strlen(srvlen);
	if
	(
	 	chanlen>=IRC_CHANNEL_MAX
//		|| srvlen>=IRC_SERVER_MAX
	)
	{
		EPRINT(
				"initSirc_Sirc_channels(): Too long channelname (%s)",
				cfg->chan
//				,server
		);
		return NULL;
	}
	_this=calloc(1,sizeof(Sirc_channels));
	if(NULL==_this)
	{
		PPRINT("calloc FAILED at initSirc_Sirc_channels()!");
		return NULL;
	}
    _this->mytype=EupperlayerStructType_Channel;
	//calloc has zeroed the space => no need to copy the terminator.
	memcpy(_this->channel,cfg->chan,chanlen);
	_this->myserv=myserv;
	/* Here it is same story as in servercfgs, copying would be intuitively cleaner mem management, but this is faster */
	_this->mycfg=cfg;
    _this->chan_online_users=OnlinestorageInit(myserv->servername,_this->channel);
	if(NULL==_this->chan_online_users)
	{
		PPRINT("FAILED to init online user storage for chan %s on server %s!",_this->channel,myserv->servername);
		return NULL;
	}

	_this->parsecommand=&parsecommand;
    _this->static_channel_user_find=&static_channel_user_find;
	_this->checkjoinstate=&checkjoinstate;
//	memcpy(_this->server,server,srvlen);

	/* Set funcptrs */
	return _this;
}









