/* ********************************************************************
 *
 * @file defaultcallbacks.c
 * @brief contains default callbacks
 *  
 *  
 * -Revision History:
 *  
 *  - 0.0.1  13.08.2009/Maz  First draft
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

#include "defaultcallbacks.h"
#include <generic.h>
#include <irchelpers.h>
#include <irc_abstraction_defines.h>

ECallbackRetval callback_send_to_irc(void *callbackhandle)
{
    return -1;
}
ECallbackRetval callback_wait_data_irc(void *callbackhandle,char *datatowait,char **recvddata,struct timespec timetowait)
{
    return -1;

}
/*
 * Use ring buffer, register ringbuffer writing hook to Conn object && extract data when recv is requested 
ECallbackRetval callback_recv_dublicate_from_irc(void *callbackhandle,void *recvhandle)
{

}
Extract data from ringbuffer registered abowe, return NULL if no data is available
char *callback_peek_dublicated_irc_data(void *recvhandle)
{

}
Remove ringbuffer hook from Conn obj && Destroy the ringbuffer
ECallbackRetval callback_cancel_recv_dublicate_from_irc(void *callbackhandle,void *recvhandle)
{

}
*/
ServerCallbackF get_default_cb(EMbotcallbackEventType cbtype)
{
	ServerCallbackF ret;
	switch(cbtype)
	{
		case EMbotcallbackEventType_LocalTxtEvent:
			ret=defaultcb_LocalTxtEvent;
			break;
		case EMbotcallbackEventType_WebTxtEvent:
			ret=defaultcb_WebTxtEvent;
			break;
		case EMbotcallbackEventType_LocalJoin:
			ret=defaultcb_LocalJoinEvent;
			break;
		case EMbotcallbackEventType_WebJoin:
			ret=defaultcb_WebJoinEvent;
			break;
		case EMbotcallbackEventType_LocalPart:
			ret=defaultcb_LocalPartEvent;
			break;
		case EMbotcallbackEventType_WebPart:
			ret=defaultcb_WebPartEvent;
			break;
		default:
			EPRINT("get_default_cb(): Unknown eventtype specified (%d)",cbtype);
			ret=NULL;
			break;
	}
	return ret;
}
void *defaultcb_LocalTxtEvent(SServerCallbackArgs args)
{
	int rval;
	char *nick;
	char *mask;
	char *chan;
	char *msgtgt;
	char *msgtosend;
	const char *msgfmt="PRIVMSG %s :%s";
	size_t sendsize;
	Sirc_servers *srv=(Sirc_servers *)args.handle;
	SServerEvents *evnt=(SServerEvents *)args.userdataptr;
	PPRINT("%s(): TODO, allow wildcards like mirc's $nick in triggerstring! %s:%d",__FUNCTION__,__FILE__,__LINE__);

	if(copyprefixtonickmask(args.parsed_data->getprefix(args.parsed_data),&nick,&mask))
	{
		EPRINT("DEFAULT JOIN CALLBACK: Failed to parse joiner's nick!!");
		return (void *)-1;
	}
	chan = args.parsed_data->getparam(args.parsed_data,1);
//	chan; //remove : in front of the channel name
	msgtgt=chan;
	if(evnt->outputdst.outputdevice!=0 || evnt->outputdst.outputname!=NULL)
	{
	
		if(evnt->outputdst.outputname!=NULL)
		{
			msgtgt=evnt->outputdst.outputname;
		}
		else
		{
			switch(evnt->outputdst.outputdevice)
			{
				case EMbotEventLocation_Privmsg:
					msgtgt=nick;
					break;
				case EMbotEventLocation_Chan:
					break;
				default:
					EPRINT("Invalid outputdevice for event %d, defaulting to channel msg!",evnt->eventId);
					EPRINT("Output device specified for event %d can only be EMbotEventLocation_Privmsg or EMbotEventLocation_Chan!",evnt->eventId);
					break;
			}
		}
	}
	if(NULL==(msgtosend=prepare_for_sending(&sendsize, msgfmt, msgtgt,evnt->event_string)))
	{
		//FAIL
		PPRINT("Failed to prepare default join message for sending (event %d)",evnt->eventId);
		free(nick);
		free(mask);
		return -1;
	}
	if(0>srv->IRCsend(srv,sendsize,msgtosend))
	{
		PPRINT("%s(): Msg send FAILED! %s:%d",__FUNCTION__,__FILE__,__LINE__);
		rval= -1;
	}
	else
	{
		DPRINT("Default cb for user %s@%s join on chan %s executed! Sent %s (event %d)",nick,mask,chan,msgtosend,evnt->eventId);
		rval=0;
	}
	free(nick);
	free(mask);
	free(msgtosend);
   // DPRINT("defaultcb_LocalJoinEvent called! event %d, command %s",eventId,ircCommand);
    return (void *)rval;

    //DPRINT("defaultcb_LocalTxtEvent called! event %d, command %s",eventId,ircCommand);
    EPRINT("defaultcb_LocalTxtEvent not yet implemented!");
    return NULL;
}
void *defaultcb_LocalJoinEvent(SServerCallbackArgs args)
{
	int rval;
	char *nick;
	char *mask;
	char *chan;
	char *msgtgt;
	char *msgtosend;
	const char *msgfmt="PRIVMSG %s :%s";
	size_t sendsize;
	Sirc_servers *srv=(Sirc_servers *)args.handle;
	SServerEvents *evnt=(SServerEvents *)args.userdataptr;
	PPRINT("%s(): TODO, allow wildcards like mirc's $nick in triggerstring! %s:%d",__FUNCTION__,__FILE__,__LINE__);

	if(copyprefixtonickmask(args.parsed_data->getprefix(args.parsed_data),&nick,&mask))
	{
		EPRINT("DEFAULT JOIN CALLBACK: Failed to parse joiner's nick!!");
		return (void *)-1;
	}
	chan = args.parsed_data->getparam(args.parsed_data,1);
	chan+=1; //remove : in front of the channel name
	msgtgt=chan;
	if(evnt->outputdst.outputdevice!=0 || evnt->outputdst.outputname!=NULL)
	{
	
		if(evnt->outputdst.outputname!=NULL)
		{
			msgtgt=evnt->outputdst.outputname;
		}
		else
		{
			switch(evnt->outputdst.outputdevice)
			{
				case EMbotEventLocation_Privmsg:
					msgtgt=nick;
					break;
				case EMbotEventLocation_Chan:
					break;
				default:
					EPRINT("Invalid outputdevice for event %d, defaulting to channel msg!",evnt->eventId);
					EPRINT("Output device specified for event %d can only be EMbotEventLocation_Privmsg or EMbotEventLocation_Chan!",evnt->eventId);
					break;
			}
		}
	}
	if(NULL==(msgtosend=prepare_for_sending(&sendsize, msgfmt, msgtgt,evnt->event_string)))
	{
		//FAIL
		PPRINT("Failed to prepare default join message for sending (event %d)",evnt->eventId);
		free(nick);
		free(mask);
		return -1;
	}
	if(0>srv->IRCsend(srv,sendsize,msgtosend))
	{
		PPRINT("%s(): Msg send FAILED! %s:%d",__FUNCTION__,__FILE__,__LINE__);
		rval= -1;
	}
	else
	{
		DPRINT("Default cb for user %s@%s join on chan %s executed! Sent %s (event %d)",nick,mask,chan,msgtosend,evnt->eventId);
		rval=0;
	}
	free(nick);
	free(mask);
	free(msgtosend);
   // DPRINT("defaultcb_LocalJoinEvent called! event %d, command %s",eventId,ircCommand);
    return (void *)rval;
}
void *defaultcb_LocalPartEvent(SServerCallbackArgs args)
{
	int rval;
	char *nick;
	char *mask;
	char *chan;
	char *msgtgt;
	char *msgtosend;
	const char *msgfmt="PRIVMSG %s :%s";
	size_t sendsize;
	Sirc_servers *srv=(Sirc_servers *)args.handle;
	SServerEvents *evnt=(SServerEvents *)args.userdataptr;
	PPRINT("%s(): TODO, allow wildcards like mirc's $nick in triggerstring! %s:%d",__FUNCTION__,__FILE__,__LINE__);

	if(copyprefixtonickmask(args.parsed_data->getprefix(args.parsed_data),&nick,&mask))
	{
		EPRINT("DEFAULT PART CALLBACK: Failed to parse parter's nick!!");
		return (void *)-1;
	}
	chan = args.parsed_data->getparam(args.parsed_data,1);
	chan+=1; //remove : in front of the channel name
	msgtgt=chan;
	if(evnt->outputdst.outputdevice!=0 || evnt->outputdst.outputname!=NULL)
	{
	
		if(evnt->outputdst.outputname!=NULL)
		{
			msgtgt=evnt->outputdst.outputname;
		}
		else
		{
			switch(evnt->outputdst.outputdevice)
			{
				case EMbotEventLocation_Privmsg:
					msgtgt=nick;
					break;
				case EMbotEventLocation_Chan:
					break;
				default:
					EPRINT("Invalid outputdevice for event %d, defaulting to channel msg!",evnt->eventId);
					EPRINT("Output device specified for event %d can only be EMbotEventLocation_Privmsg or EMbotEventLocation_Chan!",evnt->eventId);
					break;
			}
		}
	}
	if(NULL==(msgtosend=prepare_for_sending(&sendsize, msgfmt, msgtgt,evnt->event_string)))
	{
		//FAIL
		PPRINT("Failed to prepare default join message for sending (event %d)",evnt->eventId);
		free(nick);
		free(mask);
		return -1;
	}
	if(srv->IRCsend(srv,sendsize,msgtosend))
	{
		PPRINT("%s(): Msg send FAILED! %s:%d",__FUNCTION__,__FILE__,__LINE__);
		rval= -1;
	}
	else
	{
		DPRINT("Default cb for user %s@%s join on chan %s executed! Sent %s (event %d)",nick,mask,chan,msgtosend,evnt->eventId);
		rval=0;
	}
	free(nick);
	free(mask);
	free(msgtosend);
   // DPRINT("defaultcb_LocalJoinEvent called! event %d, command %s",eventId,ircCommand);
    return (void *)rval;

   // DPRINT("defaultcb_LocalPartEvent called! event %d, command %s",eventId,ircCommand);
    EPRINT("defaultcb_LocalPartEvent not yet implemented!");
    return NULL;
}
void *defaultcb_WebTxtEvent(SServerCallbackArgs args)
{
   // DPRINT("defaultcb_WebTxtEvent called! event %d, command %s",eventId,ircCommand);
    EPRINT("defaultcb_WebTxtEvent not yet implemented!");
    return NULL;
}
void *defaultcb_WebJoinEvent(SServerCallbackArgs args)
{
    //DPRINT("defaultcb_WebJoinEvent called! event %d, command %s",eventId,ircCommand);
    EPRINT("defaultcb_WebJoinEvent not yet implemented!");
    return NULL;
}
void *defaultcb_WebPartEvent(SServerCallbackArgs args)
{
   // DPRINT("defaultcb_WebPartEvent called! event %d, command %s",eventId,ircCommand);
    EPRINT("defaultcb_WebPartEvent not yet implemented!");
    return NULL;
}



