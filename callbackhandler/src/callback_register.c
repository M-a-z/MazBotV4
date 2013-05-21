/* ********************************************************************
 *
 * @file callback_register.c
 * @brief handles callback registration. (Glues registered Cbs to matching events)
 *
 *
 * -Revision History:
 * 
 *  - 0.0.1  28.07.2009/Maz  First draft (Local cb's registration ready for first 
 *  						 compile attempts...)
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



#include "callbackhandler.h"
#include <generic.h>
#include <helpers.h>
#include "event_storage.h"
#include <string.h>
#include "defaultcallbacks.h"
#include <user_callbacks.h>
#include <irc_config.h>


/* Just adds all callbacks in generic cfg struct's list (unsorted) */
/* User calls this from cbreg_hook when registering a callback. */
int callback_reg(unsigned int eventid,ServerCallbackF cbf, void *useropaque, size_t userdatasize, void *systemopaque)
{
	SircConfig *cfg;
	void *user=NULL;
	if(0<userdatasize)
	{
		if(NULL==useropaque)
		{
			EPRINT("NULL userdata pointer in callback_reg(), but userdatasize not zero(%u)!",userdatasize);
			return -1;
		}
		user=malloc(userdatasize);
		if(NULL==user)
		{
			PPRINTC(PrintComp_callback,"Malloc failed to allocate %u bytes userdata for user callback for event %u",userdatasize,eventid);
			return -1;
		}
		memcpy(user,useropaque,userdatasize);
	}
	if(NULL==systemopaque)
	{
		EPRINTC(PrintComp_callback,"NULL systemopaque ptr in callback_reg()!");
		return -1;
	}
	if(NULL==cbf)
	{
		EPRINTC(PrintComp_callback,"Null callback function for event %u in callback_reg()!",eventid);
		return -1;
	}
	if(eventid>EVENTID_MAX)
	{
		EPRINTC(PrintComp_callback,"Invalid eventid %u specified in callback_reg(), eventid range 0-%u",eventid,EVENTID_MAX);
		return -1;
	}
	cfg=(SircConfig *)systemopaque;
	return cfg->callbacklist_add(cfg,eventid,user,cbf);
}



//Extension creator calls these at startup phase.
//Add callback to callback storage. (If there's event registered, if not, print warning and ignore callback)
//Update callback ID (overwrite default ID, or add new callback ID. in event storage for fast access.)
/*
int callback_reg(SMazBotcallback *cbdetails,int overwrite_defaultcb, void *opaque, size_t opaquesize)
{
	SMazBotcallback_Local *cbstruct;
	SMazBotcallback_Web *cbstruct_web;
	cbstruct=malloc(sizeof(SMazBotcallback_Local));
	cbstruct_web=malloc(sizeof(SMazBotcallback_Web));
	if(NULL==cbstruct || NULL==cbstruct_web)
	{
		PPRINT("Malloc failed at callback_reg()");
		return -1;
	}
	switch(cbdetails->type)
	{

		case EMbotcallbackEventType_LocalTxtEvent:
		{
			int index=0;
			free(cbstruct_web);
			*cbstruct=*(SMazBotcallback_Local *)cbdetails;
			SMbotTextEvent *event=(SMbotTextEvent *)find_event_by_idandcbtype(cbstruct->gen.eventId, EMbotcallbackEventType_LocalTxtEvent);

			if(NULL==event)
			{
				EPRINT("Callback registered for event %d, but no event spec found from cfg file!",cbstruct->gen.eventId);
				free(cbstruct);
				return -2;
			}
			else
				DPRINT("Cfg for callback (eventId %d) FOUND!",cbstruct->gen.eventId);
			//Sanity check that regged callback and event read from cfg files have same event type!
			if(event->gen.type!=cbstruct->gen.type)
			{
				EPRINT("Different types assigned for event and callback id %d!",cbstruct->gen.eventId);
				free(cbstruct);
				return -2;
			}
			cbstruct->gen.opaque=malloc(opaquesize);
			if(NULL==cbstruct->gen.opaque)
			{
				PPRINT("Malloc Failed while allocing opaque at callback_reg()!");
				free(cbstruct);
				return -2;
			}
			memcpy(cbstruct->gen.opaque,opaque,opaquesize);
			cbstruct->gen.opaquesize=opaquesize;
			//Assign cbinfo to event storage.
			//Dirty way to remove default cb.. 
			if(overwrite_defaultcb)
			{
				SMazBotcallback_Local tmp;
				memset(&tmp,0,sizeof(SMazBotcallback_Local));
				tmp.gen.type=EMbotcallbackEventType_LocalTxtEvent;
				tmp.cb=&defaultcb_LocalTxtEvent;
				MBOT_NULLSAFE_FREE(mbot_ll_safe_release(event->gen.callbacks_for_this_event,&tmp));
			}
			mbot_ll_add(event->gen.callbacks_for_this_event,cbstruct);

			break;
		}
		case EMbotcallbackEventType_LocalJoin:
		{
			int index=0;
			free(cbstruct_web);
			*cbstruct=*(SMazBotcallback_Local *)cbdetails;
			SMbotHopEvent *event=(SMbotHopEvent *)find_event_by_idandcbtype(cbstruct->gen.eventId, EMbotcallbackEventType_LocalJoin);
			if(NULL==event)
			{
				EPRINT("Callback registered for event %d, but no event spec found from cfg file!",cbstruct->gen.eventId);
				free(cbstruct);
				return -2;
			}
			else
				DPRINT("Cfg for callback (eventId %d) FOUND!",cbstruct->gen.eventId);
			//Sanity check that regged callback and event read from cfg files have same event type!
			if(event->gen.type!=cbstruct->gen.type)
			{
				EPRINT("Different types assigned for event and callback id %d!",cbstruct->gen.eventId);
				free(cbstruct);
				return -2;
			}
			cbstruct->gen.opaque=malloc(opaquesize);
			if(NULL==cbstruct->gen.opaque)
			{
				PPRINT("Malloc Failed while allocing opaque at callback_reg()!");
				free(cbstruct);
				return -2;
			}
			memcpy(cbstruct->gen.opaque,opaque,opaquesize);
			cbstruct->gen.opaquesize=opaquesize;
			//Assign cbinfo to event storage.
			//Dirty way to remove default cb.. 
			if(overwrite_defaultcb)
			{
				SMazBotcallback_Local tmp;
				memset(&tmp,0,sizeof(SMazBotcallback_Local));
				tmp.gen.type=EMbotcallbackEventType_LocalJoin;
				tmp.cb=&defaultcb_LocalJoinEvent;
				MBOT_NULLSAFE_FREE(mbot_ll_safe_release(event->gen.callbacks_for_this_event,&tmp));
			}
			mbot_ll_add(event->gen.callbacks_for_this_event,cbstruct);

			break;
		}
		case EMbotcallbackEventType_LocalPart:
		{
			int index=0;
			free(cbstruct_web);
			*cbstruct=*(SMazBotcallback_Local *)cbdetails;
			SMbotHopEvent *event=(SMbotHopEvent *)find_event_by_idandcbtype(cbstruct->gen.eventId, EMbotcallbackEventType_LocalPart);
			if(NULL==event)
			{
				EPRINT("Callback registered for event %d, but no event spec found from cfg file!",cbstruct->gen.eventId);
				free(cbstruct);
				return -2;
			}
			else
				DPRINT("Cfg for callback (eventId %d) FOUND!",cbstruct->gen.eventId);
			//Sanity check that regged callback and event read from cfg files have same event type!
			if(event->gen.type!=cbstruct->gen.type)
			{
				EPRINT("Different types assigned for event and callback id %d!",cbstruct->gen.eventId);
				free(cbstruct);
				return -2;
			}
			cbstruct->gen.opaque=malloc(opaquesize);
			if(NULL==cbstruct->gen.opaque)
			{
				PPRINT("Malloc Failed while allocing opaque at callback_reg()!");
				free(cbstruct);
				return -2;
			}
			memcpy(cbstruct->gen.opaque,opaque,opaquesize);
			cbstruct->gen.opaquesize=opaquesize;
			//Assign cbinfo to event storage.
			//Dirty way to remove default cb.. 
			if(overwrite_defaultcb)
			{
				SMazBotcallback_Local tmp;
				memset(&tmp,0,sizeof(SMazBotcallback_Local));
				tmp.gen.type=EMbotcallbackEventType_LocalPart;
				tmp.cb=&defaultcb_LocalPartEvent;
				MBOT_NULLSAFE_FREE(mbot_ll_safe_release(event->gen.callbacks_for_this_event,&tmp));
			}
			mbot_ll_add(event->gen.callbacks_for_this_event,cbstruct);

			break;
		}
		case EMbotcallbackEventType_WebTxtEvent:
		case EMbotcallbackEventType_WebJoin:
		case EMbotcallbackEventType_WebPart:
		{
			free(cbstruct);
			MAZZERT(0,"WebEvents not yet implemented!");
			break;
		}
		default:
			free(cbstruct_web);
			free(cbstruct);
			EPRINT("Unknown callback event type given!");
			return -1;
			break;
	}
	DPRINT("Callback for event %d added!",cbdetails->eventId);
	return 0;
}
*/
