/* ********************************************************************
 *
 * @file event_config.h
 * @brief MazBot event config storage implementation.
 *
 *
 * -Revision History:
 *
 *  -0.0.1  15.03.2010/Maz  Splitted from irc_config.h
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





#ifndef EVENT_CONFIG_H
#define EVENT_CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbot_pseudo_xml.h"
#include <callbackhandler.h>

#define POSSIBLE_EVENT_TAG_NAMES_AMNT 7

#define SANITY_CHECK_EVENT_ADD(_this,eventRootTag) { \
	MAZZERT(NULL!=_this && NULL != eventRootTag,"NULL param in *ConfAdd"); \
	MAZZERT(!strcmp(eventRootTag->name, "event"),"Bad eventRootTag at *ConfAdd"); \
	if(eventRootTag->valuetype != EmbotPseudoxmlType_32bit) \
	{ \
		EPRINT("Invalid type for event tag! Event tag's value must be 32 bit wide event Id!"); \
		return -1; \
	}\
	if(EVENTID_MAX<*(unsigned int *)eventRootTag->value) \
	{ \
		EPRINT("Event id must be from range 0-%u, found event %u",EVENTID_MAX,*(unsigned int *)eventRootTag->value); \
		return -1;\
	}\
	if(NULL==eventRootTag->subtags)\
	{\
		EPRINT("Event tag must not be closed without giving compulsory subtags! (eventId %u)",*(int *)eventRootTag->value);\
		return -1;\
	}\
}


struct SServerEvents;

extern const char *allowed_event_tags[POSSIBLE_EVENT_TAG_NAMES_AMNT];

/* TODO:*/
typedef struct SServerEventOutputUnit
{
	EMbotEventLocation outputdevice;
	char *outputname; ///< channel/nick where output is directed (defaults to chan where event occurred)
}SServerEventOutputUnit;
typedef struct SServerEventInputUnit
{
	EMbotEventLocation inputsource;
	char *inputname; ///< channel/nick where input is obtained (in chan events defaults to current chan, in server events, defaults to any chan on server. With global events defaults to any chan)
}SServerEventInputUnit;
/*
typedef struct SServerCallbackList
{
	SServerCallback *cb;
	struct SServerCallbackList *next;
}SServerCallbackList;
*/
typedef struct SServerEvents
{
	unsigned int eventId;
	EMbotcallbackEventType eventType;
	SServerEventInputUnit inputsrc;
	SServerEventOutputUnit outputdst;
	int levelamnt;
	EIRCuserLevel *levellist;
	char *event_string;
	char *trigger_string;
	SServerCallbackList *cblist;
    //callback func ptr here? The type of cbfunc to be redefined...
}SServerEvents;

int serverEventHandleEventtype(SServerEvents *newevent,SmbotPseudoxmlTag *tag);
int serverEventHandleUserlevel(SServerEvents *newevent,SmbotPseudoxmlTag *tag);
int serverEventHandleOutputstring(SServerEvents *newevent,SmbotPseudoxmlTag *tag);
int serverEventHandleTriggerstring(SServerEvents *newevent,SmbotPseudoxmlTag *tag);
int serverEventAddEventlocAt(EMbotEventLocation *location,SmbotPseudoxmlTag *tag);
int serverEventHandleOutputto(SServerEvents *newevent,SmbotPseudoxmlTag *tag);
int serverEventHandleOutputchan(SServerEvents *newevent,SmbotPseudoxmlTag *tag);
int serverEventHandleInputfrom(SServerEvents *newevent,SmbotPseudoxmlTag *tag);


#endif

