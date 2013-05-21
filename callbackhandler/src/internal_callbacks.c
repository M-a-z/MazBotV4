#include <irc_abstraction_defines.h>
#include <irchelpers.h>
#include "internal_callbacks.h"
static const char LINEFEED[3]={(unsigned char)13,(unsigned int)10,'\0'};

EMbotCallbackRet srv_handle_nick(Sirc_servers *_this,SIRCparserResult *res)
{
	char *newnick;
	char *mask;
	char *oldnick;
    char *mynick;
	int i;
	int whoreq_sent=0;
	EMbotCallbackRet rval=0;
	//SMbotOnlineUsers *user;
    mynick=_this->mycfg->mynick;
	
	if(copyprefixtonickmask(res->getprefix(res),&oldnick,&mask))
	{
		EPRINT("%s: Failed to parse nick&mask from command prefix!",__FUNCTION__);
		return EMbotCallbackRet_AllowUserOnly;
	}
	newnick=res->getparam(res,1);
	if(NULL==newnick)
	{
		EPRINT("Malformed NICK command from server!!!");
		rval |= MBOT_CBRET_INTERNAL_ERROR;
		goto nickchout;
	} 
	if(strlen(newnick)>=IRC_NICK_MAX)
	{
		EPRINT("Cannot handle nick change %s => %s, newnick too long, max %u",oldnick,newnick,IRC_NICK_MAX-1);
		rval |= MBOT_CBRET_INTERNAL_ERROR;
		goto nickchout;
	}
	if(!strcmp(mynick,oldnick))
	{
		memcpy(_this->mycfg->mynick,newnick,strlen(newnick)+1);
		rval |= MBOT_CBRET_NO_INTERNAL_CBS;
	}
	else
	{
		/* Handle channel nicks */
		for(i=0;i<_this->amntofchannels;i++)
		{
			if(_this->channels[i]->static_channel_user_find(_this->channels[i],newnick))
			{
				//newnick was registered.
				const char *whoreq_fmt="WHO %s";
				char *whoreq;
				size_t wholen;
				whoreq_sent=1;
				whoreq=prepare_for_sending(&wholen,whoreq_fmt,_this->channels[i]->channel);
				if(NULL!=whoreq)
				{
					DPRINT("%s(): Sending whoreq \"%s\"",__FUNCTION__,whoreq);
					if(_this->IRCsend(_this,wholen,whoreq))
					{
						EPRINT("%s(): Could not send whoreq!",__FUNCTION__);
					}
					free(whoreq);
				}
				else
				{
					PPRINT("Failed to allocate WHO request!");
					return EMbotCallbackRet_InternalCbFailFatal;
				}
				if(_this->channels[i]->chan_online_users->seek(_this->channels[i]->chan_online_users,oldnick))
				{
					_this->channels[i]->chan_online_users->raw_del(_this->channels[i]->chan_online_users,oldnick);
					DPRINT("Removing nick %s from chan %s online storage!",oldnick,_this->channels[i]->channel);
				}
			}
			else
			{
				if(_this->channels[i]->chan_online_users->seek(_this->channels[i]->chan_online_users,oldnick))
				{
					DPRINT("Changing nick %s to %s for chan %s",oldnick,newnick,_this->channels[i]->channel);
					_this->channels[i]->chan_online_users->nick_change(_this->channels[i]->chan_online_users,oldnick,newnick);
				}
			}
		}


		/* If whoreq was sent by channel stuff, then it's no longer needed here! */
		/* Go through server user storage, and see if user is there */
		if(!whoreq_sent)
		{
			if(_this->static_server_user_find(_this,newnick))
			{
				/* New user was on server storage! How should I handle this? */
				/* Basically, I should send identreq as in join case... */
			}
		}
		if(_this->online_users->nick_change(_this->online_users,oldnick,newnick))
		{
			DPRINT("Could not handle NICK command for srv %s, probably nick was not registered on server",_this->servername);
		}
		else
		{
			DPRINT("Nick changed for user! %s => %s, on srv %s",oldnick,newnick,_this->servername);
		}
	}
nickchout:
	free(oldnick);
	free(mask);
	return rval;
}
/*
EMbotCallbackRet chan_handle_nick(Sirc_channels *_this,SIRCparserResult *res)
{
	char *newnick;
	char *mask;
	char *oldnick;
    char *mynick;
	EMbotCallbackRet rval=0;
    mynick=_this->myserv->mycfg->mynick;

	if(copyprefixtonickmask(res->getprefix(res),&oldnick,&mask))
	{
		EPRINT("%s: Failed to parse nick&mask from command prefix!",__FUNCTION__);
		return EMbotCallbackRet_AllowUserOnly;
	}

//Channels need not handle my own nick change.
	if(!strcmp(mynick,oldnick))
	{
		DPRINT("%s(): Oldnick (%s) matched my nick (%s), Won't handle NICK msg on chan level",__FUNCTION__,oldnick,mynick);
		rval |= EMbotCallbackRet_AllowServerAndUserOnly;
	}
	else
	{
		newnick=res->getparam(res,1);
		if(NULL==newnick)
		{
			EPRINT("Malformed NICK command from server!!!");
			rval|= MBOT_CBRET_INTERNAL_ERROR;
		}
		else if(strlen(newnick)>=IRC_NICK_MAX)
		{
			EPRINT("Cannot handle nick change %s => %s, newnick too long, max %u",oldnick,newnick,IRC_NICK_MAX-1);
			rval |= MBOT_CBRET_INTERNAL_ERROR;
		}

		// Go through channel's user storage, and see if user is there 
		else if(_this->chan_online_users->seek(_this->chan_online_users,oldnick))
		{
			// We have user on this chan! 
			// Check if newnick is registered 
			// Bullshit!!!! I need to check this from static chan configs, not from online storage! 

			if(_this->static_channel_user_find(_this,newnick))
			{
				// It was => send whorequest 
				char *whoreq_format="WHO %s";
				char *whoreq;
				size_t wholen;
				whoreq=prepare_for_sending(&wholen,whoreq_format,_this->channel);
				if(NULL!=whoreq)
				{
					DPRINT("%s(): Sending whoreq \"%s\"",__FUNCTION__,whoreq);
					if(_this->myserv->IRCsend(_this->myserv,wholen,whoreq))
					{
						EPRINT("%s(): Could not send whoreq!",__FUNCTION__);
					}
					free(whoreq);
				}
				else
				{
					PPRINT("Failed to allocate WHO request!");
					return EMbotCallbackRet_InternalCbFailFatal;
				}
				// Delete old user:
				_this->chan_online_users->raw_del(_this->chan_online_users,oldnick);


			}
			else
			{
				// Newnick is not protected => change user 
				if(_this->chan_online_users->nick_change(_this->chan_online_users,oldnick,newnick))
				{
					EPRINT("FAILED to change nick %s => %s for user!",oldnick,newnick);
					rval |= MBOT_CBRET_INTERNAL_ERROR;
				}
				else
					DPRINT("Nick changed for user! %s => %s, on channel %s",oldnick,newnick,_this->channel);
			}
		}
		else
		{
			DPRINT("Could not handle NICK command for chan %s, probably nick was not registered on chan",_this->channel);
		}
	}
	free(oldnick);
	free(mask);
	return rval;
}
*/

EMbotCallbackRet srv_handle_ping(Sirc_servers *_this,SIRCparserResult *res)
{   
	int i;
    char *pingreply;
    size_t pongarglen=strlen(res->getparam(res,1));
    DPRINT("PING DETECTED - CREATING REPLY");

	for(i=0;i<_this->amntofchannels;i++)
	{
		if(_this->channels[i]->checkjoinstate(_this->channels[i]))
		{
			EPRINT("Failed to check join state for chan %s!",_this->channels[i]->channel);
        	return EMbotCallbackRet_InternalCbFail;
		}
	}


    pingreply=malloc( pongarglen+8); //+8 == PONG+space+\r+\n+\0
    if(NULL==pingreply)
    {   
        PPRINT("malloc FAILED! (pongarglen=%u)",pongarglen);
        return EMbotCallbackRet_InternalCbFail;
    }
    snprintf(pingreply,pongarglen+8,"%s %s%s","PONG",res->getparam(res,1),LINEFEED);
    DPRINT(">> \"%s\"",pingreply);
    if(_this->IRCsend(_this,pongarglen+7,pingreply))
	{
		EPRINT("Sending PONG FAILED!");
		return EMbotCallbackRet_InternalCbFail;
	}
    free(pingreply);
    return EMbotCallbackRet_InternalHandled;
}




/* Channel should handle part, but server should handle quit */
EMbotCallbackRet chan_handle_part(Sirc_channels *_this,SIRCparserResult *res)
{
    // clean the online user storage if needed.
    char *chan;
    char *nick;
	char *mask;
    SMbotOnlineUsers *user;
    char *mynick;
    mynick=_this->myserv->mycfg->mynick;
    chan=res->getparam(res,1);

    if(strcmp(chan,_this->channel))
    {
        DPRINT("PART command targeted for channel %s not handled on channel %s",chan,_this->channel);
        return EMbotCallbackRet_NotHandled;
    }
	if(copyprefixtonickmask(res->getprefix(res),&nick,&mask))
	{
		EPRINT("chan_handle_internal_part_cmm(): Failed to parse nick&mask from command prefix!");
		return EMbotCallbackRet_AllowUserOnly;
	}

//    nick=res->getparam(res,6);
    DPRINT("%s: User %s parting channel %s",__FUNCTION__,nick,_this->channel);
    if(!strcmp(mynick,nick))
    {
        DPRINT("Server reported me leaving channel %s",_this->channel);
		_this->chan_joined=0;
        PPRINT("TODO: specify corrective actions...");
		free(nick);
		free(mask);
        return EMbotCallbackRet_InternalCbFailFatal;
    }

    if(NULL!=(user=_this->chan_online_users->seek(_this->chan_online_users,nick)) )
    {
        DPRINT("User %s leaving the channel %s => removing from online storage!",nick,_this->channel);
        if(_this->chan_online_users->raw_del(_this->chan_online_users,nick))
        {
            PPRINT("User %s deletion from online storage at chan %s FAILED!",nick,_this->channel);
			free(nick);
			free(mask);
            return EMbotCallbackRet_InternalCbFailFatal;
        }
    }
    else
    {
        PPRINT("TODO: User join/part && add/remove from user storage is not atomic => add state which informs that IDENTIFY request has been sent && delay removal if user is not found fromstorage! Otherwise we'll end up messing the storage!!");
    }
	free(nick);
	free(mask);
	return EMbotCallbackRet_AllowUserOnly;
}

EMbotCallbackRet srv_handle_endOfMotd(Sirc_servers *_this,SIRCparserResult *res)
{
	int i;
	_this->srv_connected=1;
	DPRINT("Motd recvd, checking channel states!");
	for(i=0;i<_this->amntofchannels;i++)
	{
		if(_this->channels[i]->checkjoinstate(_this->channels[i]))
		{
			EPRINT("Failed to check join state for chan %s!",_this->channels[i]->channel);
        	return EMbotCallbackRet_InternalCbFail;
		}
	}
	return EMbotCallbackRet_AllowUserOnly;
}
EMbotCallbackRet srv_handle_quit(Sirc_servers *_this,SIRCparserResult *res)
{
    // Clean the server online storage + check if chan storage cleaning is needed?
    //char *chan;
    char *nick;
	char *mask;
	char *tmp;
    SMbotOnlineUsers *user;
    char *mynick;
    int i;
    mynick=_this->mycfg->mynick;
/*    chan=res->getparam(res,2);

    if(strcmp(chan,_this->channel))
    {
        DPRINT("PART command targeted for channel %s not handled on channel %s",chan,_this->channel);
        return EMbotCallbackRet_NotHandled;
    }
*/
	tmp=res->getprefix(res);
	if(NULL==tmp)
	{
		EPRINT("QUIT message from server with no prefix => can't handle!");
		return EMbotCallbackRet_AllowUserOnly;
	}

	if(copyprefixtonickmask(tmp,&nick,&mask))
	{
		EPRINT("%s(): Failed to parse nick&mask from command prefix!",__FUNCTION__);
		return EMbotCallbackRet_AllowUserOnly;
	}
	
    DPRINT("%s: User %s quitting on server %s",__FUNCTION__,nick,_this->servername);
    if(!strcmp(mynick,nick))
    {
		int k;
        DPRINT("Server reported me quitting on server %s?!?!?!?!",_this->servername);
        PPRINT("TODO: specify corrective actions...");
		/* These should be done in a centralized way, whether the network disconnects, or we quit 
		for(k=0;k<_this->amntofchannels;k++)
		{
		*/
			/* clear online storages */
			/* set channel to not joined */
		/*
			_this->channel[k]->chan_joined=0;
		}
		
		_this->server_connected=0;
		*/
		free(nick);
		free(mask);
        return EMbotCallbackRet_InternalCbFailFatal;
    }
    /* Clean also channel storages */
    for(i=0;i<_this->amntofchannels;i++)
    {
        if(NULL!=_this->channels[i]->chan_online_users->seek(_this->channels[i]->chan_online_users,nick))
        {
            DPRINT("Removing user %s from channel %s onlinestorage due to quit",nick,_this->channels[i]->channel);
            if(_this->channels[i]->chan_online_users->raw_del(_this->channels[i]->chan_online_users,nick))
            {
                PPRINT("User %s deletion from online storage at chan %s FAILED!",nick,_this->channels[i]->channel);
				free(nick);
				free(mask);
                return EMbotCallbackRet_InternalCbFailFatal;
            }

        }
    }

    if(NULL!=(user=_this->online_users->seek(_this->online_users,nick)) )
    {
        DPRINT("User %s leaving the server %s => removing from online storage!",nick,_this->servername);
        if(_this->online_users->raw_del(_this->online_users,nick))
        {
            PPRINT("User %s deletion from online storage at srv %s FAILED!",nick,_this->servername);
			free(nick);
			free(mask);
            return EMbotCallbackRet_InternalCbFailFatal;
        }
    }
    else
    {
        PPRINT("TODO: User join/part && add/remove from user storage is not atomic => add state which informs that IDENTIFY request has been sent && delay removal if user is not found fromstorage! Otherwise we'll end up messing the storage!!");
    }
	free(nick);
	free(mask);
	return EMbotCallbackRet_AllowUserOnly;
}

EMbotCallbackRet chan_handle_userlist_user(Sirc_channels *_this,struct SIRCparserResult *res)
{
	SMbotOnlineUsers *user;
	char *nick;
	char *chan;
	char *mask;
	char *mynick;
	int i;
	mynick=_this->myserv->mycfg->mynick;
	chan=res->getparam(res,2);

	if(strcmp(chan,_this->channel))
	{
		/* This is not the correct channel => let other chans to handle this */
		/* Does the getparam()  copy the param, or return ptr to res's internal data? */
		/* TODO: Find out and free if needed */
		DPRINT("userlist report for chan %s not handled on chan %s",chan,_this->channel);
		return EMbotCallbackRet_NotHandled;
	}
	nick=res->getparam(res,6);
	if(!strcmp(mynick,nick))
	{
		DPRINT("Server reported me as online guy => won't handle");
		return EMbotCallbackRet_InternalHandled;
	}
	mask=res->getparam(res,4);
	DPRINT("userlist reports user (nick=%s,mask=%s) on channel %s, seeing if user is on online storage",nick,mask,chan);
    /* At first look for channel users, which is preferred over server users since chan user is specific for this chan */
    /* Then check for serveruser if there's no channel user */
	if
    (
        NULL==(user=_this->chan_online_users->seek(_this->chan_online_users,nick)) &&
        NULL==(user=_this->myserv->online_users->seek(_this->myserv->online_users,nick)) 
    )
	{
		int request_identification=0;
		char *identreq;
		const char *identfmt="PRIVMSG %s :%s is registered nick, please send me !IDENTIFY %s <password> message or change your nick! (nick registered at %s level)";
		/* Unknown user in list => check if global cfg knows him. */
		DPRINT("user %s not found from online list, searching from static configs",nick);
        DPRINT("static cfg has %u users for chan %s",_this->mycfg->useramnt,chan);
        PPRINT("Add also searching of server users!");
		for(i=0;i<_this->mycfg->useramnt;i++)
		{
			MAZZERT(NULL!=_this->mycfg->users[i],"user amount and user array imbalance!");
			if(!strcmp(_this->mycfg->users[i]->nick,nick))
			{
				/* User found! */ 
				/* send identify request. (and fire a timer for "user not identified in time")
				 * action
				 */
				DPRINT("User %s found from static channel configs, request identification",nick);
				request_identification=1;
			}
		}
		for(i=0;i<_this->mycfg->myserver->useramnt;i++)
		{
			MAZZERT(NULL!=_this->mycfg->myserver->users[i],"user amount and user array imbalance!");
			if(!strcmp(_this->mycfg->myserver->users[i]->nick,nick))
			{
				/* User found! */ 
				/* send identify request. (and fire a timer for "user not identified in time")
				 * action
				 */
				DPRINT("User %s found from static server configs, request identification",nick);
				request_identification+=2;
			}
		}
		if(request_identification)
		{
			size_t identlen;
			const char *idcap[3]={"channel","server","channel/server"};
			const char *srvreq="SERVER";
			char *idreqtgt;
			if(1==request_identification)
			{
				idreqtgt=_this->channel;
			}
			else if(2==request_identification)
			{
				idreqtgt=srvreq;
			}
			else
			{
				idreqtgt=malloc(strlen(_this->channel)+1+strlen("SERVER")+1);
				if(NULL==idreqtgt)
				{
					PPRINT("Malloc FAILED!!");
					return EMbotCallbackRet_InternalCbFailFatal;
				}
				memcpy(idreqtgt,srvreq,strlen("SERVER"));
				idreqtgt[strlen("SERVER")]='/';
				memcpy(&(idreqtgt[strlen("SERVER/")]),_this->channel,strlen(_this->channel)+1);
			}
			identreq=prepare_for_sending(&identlen,identfmt,nick,nick,idreqtgt,idcap[request_identification-1]);
			if(NULL==identreq)
			{
				PPRINT("Identreq alloc FAILED!");
				return EMbotCallbackRet_InternalCbFailFatal;
			}
			else
			{
				DPRINT("%s: Sending identreq \"%s\"",__FUNCTION__,identreq);
				if(_this->myserv->IRCsend(_this->myserv,identlen,identreq))
				{
					EPRINT("%s(): Could not send identreq!",__FUNCTION__);
				}
                PPRINT("TODO: Add timer which does specified action if someone tries stealing the nick!\n%s:%d",__FILE__,__LINE__);
				free(identreq);
			}
		}
	}
    /* No other internal Cb should be interested in this */
	return EMbotCallbackRet_AllowUserOnly;
}

EMbotCallbackRet chan_handle_version_cmm(Sirc_channels *_this,SIRCparserResult *res)
{
    char *nick;
    char *mask=NULL; 
	char *pass;
	const char *vers_reply_fmt="NOTICE %s :%cVERSION MazBotV4 0.2.0_dev SOMEOS SOMEARCH%c";
	char *vers_reply;
	size_t replylen;

	DPRINT("%s called",__FUNCTION__);


    if(copyprefixtonickmask(res->getprefix(res),&nick,&mask))
    {
        EPRINT("%s(): Failed to parse nick&mask from command prefix!",__FUNCTION__);
        return EMbotCallbackRet_AllowUserOnly;
    }
	vers_reply=prepare_for_sending(&replylen,vers_reply_fmt,nick,'\1','\1');
	if(0>_this->myserv->IRCsend(_this->myserv,replylen,vers_reply))
	{
		EPRINT("%s(): Sending data FAILED!",__FUNCTION__);
		free(nick);
		free(mask);
		return EMbotCallbackRet_InternalCbFail;
	}
	DPRINT("Sending version reply '%s'",vers_reply);
	free(nick);
	free(mask);
	free(vers_reply);
	return EMbotCallbackRet_AllowUserOnly;
}

// TODO: Split this so that one can use this from user join too! 
EMbotCallbackRet chan_handle_identify_cmm(Sirc_channels *_this,SIRCparserResult *res)
{
    SMbotOnlineUsers *user;
    SircUser *staticUser;
    char *nick;
    char *tgt;
    char *mask=NULL; 
	char *pass;
    int id_success=0;
	//size_t nmlen,mlen;
	DPRINT("%s called",__FUNCTION__);
	tgt=res->getparam(res,3);
	if(NULL==tgt)
	{
		DPRINT("NULL tgt provided in identify command!");
	}
	if(strcmp(_this->channel,tgt) && strcmp("SERVER",tgt))
	{
		DPRINT("Channel %s ignoring IDENTIFY request with 1. arg as %s",_this->channel,tgt);
		return 0;
	}
    if(copyprefixtonickmask(res->getprefix(res),&nick,&mask))
    {
        EPRINT("%s(): Failed to parse nick&mask from command prefix!",__FUNCTION__);
        return EMbotCallbackRet_AllowUserOnly;
    }

    DPRINT("%u users for chan %s and",_this->mycfg->useramnt,_this->channel);
    DPRINT("%u users for srv %s",_this->mycfg->myserver->useramnt,_this->myserv->servername);
	if(!strcmp("SERVER",tgt))
	{
        if(NULL==(user=_this->myserv->online_users->seek(_this->myserv->online_users,nick)))
        {
            /* Unknown user in list => check if global cfg knows him. */
			if(NULL!=(staticUser=_this->myserv->static_server_user_find(_this->myserv,nick)))
			{
				/* User found! */ 
				DPRINT("User %s found from static server configs, check passwd",nick);
				pass=res->getparam(res,4);
				if(NULL==pass)
				{
					DPRINT("NULL passwd provided in identify command!");
					PPRINT("This may be because user tries using nick@mask id mode which is not yet implemented!");
				    free(nick);
				    free(mask);
					return EMbotCallbackRet_InternalCbFail;
				}
				PPRINT("TODO: Add other identification modes! If ID-mode == nick&mask => compare masks, don't check pass!");
				if(!strcmp(staticUser->pass,pass))
				{
					DPRINT("Passwds matched => adding user to online storage!");
					if(_this->myserv->online_users->raw_add(_this->myserv->online_users,nick,mask, pass,staticUser->ulevel))
					{
						EPRINT("USER ADDING FAILED! %s:%d",__FILE__,__LINE__);
				        free(nick);
				        free(mask);
						return EMbotCallbackRet_InternalCbFail;
					}
    				id_success=1;
				}
				else
				{
					DPRINT("User %s(@%s) provided bad password (%s), expected (%s)",nick,mask,pass,staticUser->pass);
				}
			}//if user is in static server configs ends
            else
            {
                DPRINT("Non registered (server)user %s tried identifying",nick);   
            }

		}//server identification ends
    	else
	    {
		    /* Allready identified => ignore */
    		DPRINT("User %s who has already identified sent me !IDENTIFY cmnd! => won't re-evaluate",nick);
	    	DPRINT("Is this correct behaviour?");
	    }

    }// if !strcmp(SERVER) ends
	else
	{

        if(NULL==(user=_this->chan_online_users->seek(_this->chan_online_users,nick)))
        {
            /*
           	for(i=0;i<_this->mycfg->useramnt;i++)
	        {
                MAZZERT(NULL!=_this->mycfg->users[i],"user amount and user array imbalance!");
       	        if(!strcmp(_this->mycfg->users[i]->nick,nick))
               	{
                */
            if(NULL!=(staticUser=_this->static_channel_user_find(_this,nick)))
            {
	                /* User found! */
		    		/* TODO: See print */
			    if(!strcmp(staticUser->pass,res->getparam(res,4)))
    			{
	    			DPRINT("Passwds matched => adding user to chan %s online storage!",_this->channel);
		    		if(_this->chan_online_users->raw_add(_this->chan_online_users,nick,mask, staticUser->pass,staticUser->ulevel))
			    	{
				    	EPRINT("USER ADDING FAILED! %s:%d",__FILE__,__LINE__);
				        free(nick);
				        free(mask);
    			    	return EMbotCallbackRet_InternalCbFail;
	    			}
		    		id_success=1;
			    }

				PPRINT("TODO: Add identification modes %s:%d",__FILE__,__LINE__);
            }// If static chan configs know the nick
            else
            {
                DPRINT("Non registered user %s tried identifying",nick);   
            }
    	} //If online user is already added 
    	else
	    {
		    /* Allready identified => ignore */
    		DPRINT("User %s who has already identified sent me !IDENTIFY cmnd! => won't re-evaluate",nick);
	    	DPRINT("Is this correct behaviour?");
	    }

    }//channel identification ends
	if(id_success)
	{
		const char *fmt="PRIVMSG %s :%s";
		char *msgtosend;
		size_t sendlen;
		if(NULL==(msgtosend=prepare_for_sending(&sendlen,fmt,nick,"You've now successfully identified!")))
		{
			PPRINT("Msg alloc FAILED!");
			return EMbotCallbackRet_InternalCbFail;
		}
		if(0>_this->myserv->IRCsend(_this->myserv,sendlen,msgtosend))
		{
			EPRINT("Sending data to socket FAILED!");
			return EMbotCallbackRet_InternalCbFail;
		}
		DPRINT("Sending \"%s\"",msgtosend);
		free(msgtosend);
	}
    free(nick);
    free(mask);
    return EMbotCallbackRet_AllowUserOnly;
}
/*
EMbotCallbackRet chan_handle_internal_joinfail_cmm(Sirc_channels *_this,SIRCparserResult *res)
{
	char *chan;
	char *nick;
	char *mynick=_this->myserv->mycfg->mynick;

	nick=res->getparam(res,1);
	if(NULL==nick)
	{
		PPRINT("Got 405 (cannot join), but unable to parse nick!");
		return EMbotCallbackRet_InternalCbFail;
	}
	chan=res->getparam(res,2);
	if(NULL==chan)
	{
		PPRINT("Got 405 (cannot join), but unable to parse chan!");
		return EMbotCallbackRet_InternalCbFail;
	}

	WPRINT("Got 405 (unable to join) from server %s, nick=%s, channel=%s",_this->myserv->servername,nick,chan);
	if(strcmp(chan,_this->channel))
	{
		DPRINT("405 was targeted to %s, wont handle on %s!",chan,_this->channel);
    	return EMbotCallbackRet_NotHandled;
	}
	if(strcmp(nick,mynick))
	{
		DPRINT("Strange, obtained unable to join for nick %s!",nick);
		PPRINT("TODO: Handle as PART would be handled! %s:%d",__FILE__,__LINE__);
    	return EMbotCallbackRet_NotHandled;
	}
	_this->chan_joined--;
	DPRINT("Decreasing joinstatus, was %d, is now %d",_this->chan_joined+1,_this->chan_joined);
	return EMbotCallbackRet_InternalHandled; 
}
*/
EMbotCallbackRet chan_handle_internal_join_cmm(Sirc_channels *_this,SIRCparserResult *res)
{
	char *chan,*nick,*mask;
	char *whoreq_format="WHO %s";
	char *whoreq;

	DPRINT("Internally handling JOIN command:");
	chan=res->getparam(res,1);
	chan+=1;
	if(strcmp(chan,_this->channel))
	{
		DPRINT("Wont handle JOIN cmm for chan %s at chan %s!",chan,_this->channel);
		return EMbotCallbackRet_NotHandled;
	}
	DPRINT("chan_handle_internal_join_cmm(): Parsing nick&mask");
	if(copyprefixtonickmask(res->getprefix(res),&nick,&mask))
	{
		EPRINT("chan_handle_internal_join_cmm(): Failed to parse nick&mask from command prefix!");
		free(nick);
		free(mask);
		return EMbotCallbackRet_AllowUserOnly;
	}
	else
	{
		size_t wholen;
		char *mynick=_this->myserv->mycfg->mynick;
		if(!strcmp(nick,mynick))
		{
			_this->chan_joined++;
			DPRINT("Increasing joinstatus, was %d, is now %d",_this->chan_joined-1,_this->chan_joined);

		}
		whoreq=prepare_for_sending(&wholen,whoreq_format,chan);
		if(NULL!=whoreq)
		{
			DPRINT("chan_handle_internal_join_cmm(): Sending whoreq \"%s\"",whoreq);
			if(_this->myserv->IRCsend(_this->myserv,wholen,whoreq))
			{
				EPRINT("chan_handle_internal_join_cmm(): Could not send whoreq!");
			}
			free(whoreq);
		}
		else
		{
			PPRINT("Failed to allocate WHO request!");
			return EMbotCallbackRet_InternalCbFailFatal;
		}
	}
	return EMbotCallbackRet_AllowUserOnly;
}



