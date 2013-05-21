/* ********************************************************************
 *
 * @file server_events.c
 * @brief MazBot event config storage implementation.
 *
 *
 * -Revision History:
 *
 *  -0.0.2  16.03.2010/Maz  Small bug fixes
 *  -0.0.1  15.03.2010/Maz  Splitted from irc_config.c
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





#include "event_config.h"
#include "mbot_pseudo_xml.h"
const char *allowed_event_tags[POSSIBLE_EVENT_TAG_NAMES_AMNT] = {"eventtype","userlevel","triggerstring","outputto","outputchan","inputfrom","outputstring"};


int serverEventHandleEventtype(SServerEvents *newevent,SmbotPseudoxmlTag *tag)
{
	if(tag->valuetype!=EmbotPseudoxmlType_char)
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid value type for tag %s!",tag->name);
		return -1;
	}
	if(!strcmp("EMbotcallbackEventType_LocalTxtEvent",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_LocalTxtEvent;
	}
	else if(!strcmp("EMbotcallbackEventType_WebTxtEvent",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_WebTxtEvent;
	}
	else if(!strcmp("EMbotcallbackEventType_LocalJoin",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_LocalJoin;
	}
	else if(!strcmp("EMbotcallbackEventType_WebJoin",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_WebJoin;
	}
	else if(!strcmp("EMbotcallbackEventType_LocalPart",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_LocalPart;
	}
	else if(!strcmp("EMbotcallbackEventType_WebPart",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_WebPart;
	}
	else if(!strcmp("EMbotcallbackEventType_LocalCtCp",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_LocalCtCp;
	}
	else if(!strcmp("EMbotcallbackEventType_WebCtCp",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_WebCtCp;
	}
	else if(!strcmp("EMbotcallbackEventType_LocalRaw",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_LocalRaw;
	}
	else if(!strcmp("EMbotcallbackEventType_WebRaw",(char *)(tag->value)))
	{
		newevent->eventType=EMbotcallbackEventType_WebRaw;
	}
	else
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid type for event (%s)",(char *)tag->value);
		return -1;
	}
	return 0;
}
int serverEventHandleUserlevel(SServerEvents *newevent,SmbotPseudoxmlTag *tag)
{
	if(tag->valuetype!=EmbotPseudoxmlType_32bit)
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid value type for tag %s!",tag->name);
		return -1;
	}
	if(0<tag->size)
	{
		int i;
		newevent->levellist=calloc(tag->size,sizeof(newevent->levellist[0]));
		if(NULL==newevent->levellist)
		{
			PPRINTC(PrintComp_IrcCfg,"calloc failed!");
			return -1;
		}
		for(i=0;i<tag->size;i++)
		{
			DPRINTC(PrintComp_IrcCfg,"userlevel %d found for event",*(int *)(tag->value));
			newevent->levellist[i]=((int *)(tag->value))[i];
			newevent->levelamnt++;
		}
	}
	else
	{
		EPRINTC(PrintComp_IrcCfg,"invalid length for tag %s!",tag->name);
		return -1;
	}
	return 0;
}
int serverEventHandleTriggerstring(SServerEvents *newevent,SmbotPseudoxmlTag *tag)
{
	if(tag->valuetype!=EmbotPseudoxmlType_char)
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid value type for tag %s!",tag->name);
        return -1;
    }
    newevent->trigger_string=calloc(tag->size+1,sizeof(char));
    if(NULL==newevent->trigger_string)
    {
        PPRINTC(PrintComp_IrcCfg,"calloc FAILED for event tag %s",tag->name);
        return -1;
    }
    memcpy(newevent->trigger_string,tag->value,tag->size);
    DPRINTC(PrintComp_IrcCfg,"Trigger string %s parsed",newevent->event_string);
    return 0;
}

int serverEventHandleOutputstring(SServerEvents *newevent,SmbotPseudoxmlTag *tag)
{	
	if(tag->valuetype!=EmbotPseudoxmlType_char)
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid value type for tag %s!",tag->name);
		return -1;
	}
	newevent->event_string=calloc(tag->size+1,sizeof(char));
	if(NULL==newevent->event_string)
	{
		PPRINTC(PrintComp_IrcCfg,"calloc FAILED for event tag %s",tag->name);
		return -1;
	}
	memcpy(newevent->event_string,tag->value,tag->size);
	DPRINTC(PrintComp_IrcCfg,"Event string %s parsed",newevent->event_string);
	return 0;
}
int serverEventAddEventlocAt(EMbotEventLocation *location,SmbotPseudoxmlTag *tag)
{
	if(tag->valuetype!=EmbotPseudoxmlType_char)
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid value type for tag %s!",tag->name);
		return -1;
	}
	if(!strcmp("EMbotEventLocation_Chan",(char *)(tag->value)))
	{
		*location=EMbotEventLocation_Chan;
	}
	else if(!strcmp("EMbotEventLocation_Privmsg",(char *)(tag->value)))
	{
		*location=EMbotEventLocation_Privmsg;
	}
	else if(!strcmp("EMbotEventLocation_CPM",(char *)(tag->value)))
	{
		*location=EMbotEventLocation_CPM;
	}
	else if(!strcmp("EMbotEventLocation_Dcc",(char *)(tag->value)))
	{
		*location=EMbotEventLocation_Dcc;
	}
	else if(!strcmp("EMbotEventLocation_CD",(char *)(tag->value)))
	{
		*location=EMbotEventLocation_CD;
	}
	else if(!strcmp("EMbotEventLocation_PMD",(char *)(tag->value)))
	{
		*location=EMbotEventLocation_PMD;
	}
	else if(!strcmp("EMbotEventLocation_All",(char *)(tag->value)))
	{
		*location=EMbotEventLocation_All;
	}
	else
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid location for event (%s)",(char *)tag->value);
		return -1;
	}
	return 0;

}
int serverEventHandleOutputto(SServerEvents *newevent,SmbotPseudoxmlTag *tag)
{
	return serverEventAddEventlocAt(&(newevent->outputdst.outputdevice),tag);
}
int serverEventHandleOutputchan(SServerEvents *newevent,SmbotPseudoxmlTag *tag)
{
	if(tag->valuetype!=EmbotPseudoxmlType_char)
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid value type for tag %s!",tag->name);
		return -1;
	}
	newevent->outputdst.outputname=calloc(tag->size+1,sizeof(char));
	if(NULL==newevent->outputdst.outputname)
	{
		PPRINTC(PrintComp_IrcCfg,"calloc FAILED for event tag %s",tag->name);
		return -1;
	}
	memcpy(newevent->outputdst.outputname,tag->value,tag->size);
	DPRINTC(PrintComp_IrcCfg,"Event outputname %s parsed",newevent->outputdst.outputname);
	return 0;

}
int serverEventHandleInputfrom(SServerEvents *newevent,SmbotPseudoxmlTag *tag)
{
	return serverEventAddEventlocAt(&(newevent->inputsrc.inputsource),tag);
}


