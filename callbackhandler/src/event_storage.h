/* ********************************************************************
 *
 * @file event_storage.h
 * @brief event storing and handling (callback engine) related definitions
 *
 *
 * -Revision History:
 *
 *  - 0.0.2  12.08.2009/Maz  Ready for first compile attempts ;)
 *  - 0.0.1  11.08.2009/Maz  First draft
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



#ifndef MAZBOT_v4_EVENT_STORAGE_H
#define MAZBOT_v4_EVENT_STORAGE_H
#include <callbackhandler.h>
#include <defaultcallbacks.h>
#include <helpers.h>
#include <config_file_parser.h>
/* Event storage is filled from config files at start up.
 * The triggering actions and filters are fully specified here.
 * Also an event ID is assigned, and a callback matching to this ID is then searched.
 * If no callback is found, default callback is assigned for this ID
 */
#define EVENT_TEXT_STORAGE_SIZE (100*sizeof(SMbotTextEvent))
#define EVENT_JOIN_STORAGE_SIZE (100*sizeof(SMbotHopEvent))
#define EVENT_PART_STORAGE_SIZE (100*sizeof(SMbotHopEvent))

struct SMbotEventStorage;

/* Generic event struct */
typedef struct SMbotEvent
{
    int eventId; //Must be first in here?
    EMbotcallbackEventType type;
	EIRCuserLevelDirection userlevel_direction;
	EIRCuserLevel userlevel;
	mbot_linkedList *originators;
	char *default_cb_text;
    mbot_linkedList *callbacks_for_this_event;
}SMbotEvent;

/* Text Event Struct */
typedef struct SMbotTextEvent
{
	SMbotEvent gen;
    EMbotEventLocation locationFilter;
	char *triggerstring;
}SMbotTextEvent;

typedef struct SMbotHopEvent
{
	SMbotEvent gen;
}SMbotHopEvent;

typedef int (*parse_def_eventsF)(struct SMbotEventStorage *_this,FILE *readfile);
typedef void (*mbot_event_storages_uninitF)(struct SMbotEventStorage**);
typedef SMbotEvent *(*find_event_by_idandcbtypeF)(struct SMbotEventStorage*, int id, EMbotcallbackEventType type);
typedef int (*add_eventF)(struct SMbotEventStorage*,ScallbackConf *eventspec);
typedef int (*localevent_add_default_event_stringF)(struct SMbotEventStorage*,int id, char *string);
typedef struct SMbotEventStorage
{
/* Text Event storages */
	SMbotTextEvent *mbot_local_textevent_storage[EVENT_TEXT_STORAGE_SIZE];
	SMbotTextEvent *mbot_web_textevent_storage[EVENT_TEXT_STORAGE_SIZE];
/* Join Event Storage */
	SMbotHopEvent *mbot_local_joinevent_storage[EVENT_JOIN_STORAGE_SIZE];
	SMbotHopEvent *mbot_web_joinevent_storage[EVENT_JOIN_STORAGE_SIZE];
/* Part Event Storage */
	SMbotHopEvent *mbot_local_partevent_storage[EVENT_PART_STORAGE_SIZE];
	SMbotHopEvent *mbot_web_partevent_storage[EVENT_PART_STORAGE_SIZE];
	MbotAtomic32 * storageIndex_LocalTxtEvent;
	MbotAtomic32 * storageIndex_WebTxtEvent;
	MbotAtomic32 * storageIndex_LocalJoin;
	MbotAtomic32 * storageIndex_WebJoin;
	MbotAtomic32 * storageIndex_LocalPart;
	MbotAtomic32 * storageIndex_WebPart;
	mbot_event_storages_uninitF uninit;
	find_event_by_idandcbtypeF find_by_idandcbtype;
	add_eventF add_event;
//	localevent_add_default_event_stringF localevent_add_default_event_string;
	parse_def_eventsF parse_def_events;
}SMbotEventStorage;

//Find event by id and callback type.
//SMbotEvent *find_event_by_idandcbtype(int id, EMbotcallbackEventType type);
//Add new event to list
int add_event(ScallbackConf *eventspec);
/*
extern SMbotTextEvent *mbot_local_textevent_storage[EVENT_TEXT_STORAGE_SIZE];
extern SMbotTextEvent *mbot_web_textevent_storage[EVENT_TEXT_STORAGE_SIZE];
extern SMbotHopEvent *mbot_local_joinevent_storage[EVENT_JOIN_STORAGE_SIZE];
extern SMbotHopEvent *mbot_web_joinevent_storage[EVENT_JOIN_STORAGE_SIZE];
extern SMbotHopEvent *mbot_local_partevent_storage[EVENT_PART_STORAGE_SIZE];
extern SMbotHopEvent *mbot_web_partevent_storage[EVENT_PART_STORAGE_SIZE];
*/
/* Initializes storages for use */
SMbotEventStorage * mbot_event_storages_init();
/* Uninitializes storages */
//void mbot_event_storages_uninit(MbotEventStorage **_this);
//int localevent_add_default_event_string(int id, char *string);

#endif //MAZBOT_v4_EVENT_STORAGE_H

