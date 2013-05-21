/* ********************************************************************
 *
 * @file event_storage.c
 * @brief event storing and handling (callback engine) related functions
 *
 *
 * -Revision History:
 *
 *  - 0.0.5  19.08.2009/Maz  Added userlevels
 *  - 0.0.4  17.08.2009/Maz  Fixed too small allocations.
 *  - 0.0.3  16.08.2009/Maz  Added adding of local part and local join + triggerstring and location of txt event
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

#include <generic.h>
#include <helpers.h>
#include "event_storage.h"
#include "callbackhandler.h"
#include <config_file_parser.h>
#include "defaultcallbacks.h"
//TODO think if it is needed to have Sconn * for IRC connection in callback specific struct
//Usually callback is executed when something happens on some server ->
//Reply (callback action) should be done on same server! (Eg, callback should have the Sconn obj which sent the matching reply
//as param, not some predefined reply address registered by callback writer!!


#define ORIGINATOR_SEPARATOR_CHAR ';'
#define ORIGINATOR_SEPARATOR_STR ";"






//XXX: Although on most modern comp architectures the bus sizes are at least 32bit, and thus
//	   writing 32 bit value should be atomic, it is possible that one day I need to use
//	   semaphore ptorection or write arch specific atomic assignment functions.
/*
MbotAtomic32 * storageIndex_LocalTxtEvent;
MbotAtomic32 * storageIndex_WebTxtEvent;
MbotAtomic32 * storageIndex_LocalJoin;
MbotAtomic32 * storageIndex_WebJoin;
MbotAtomic32 * storageIndex_LocalPart;
MbotAtomic32 * storageIndex_WebPart;
//TODO: Make these struct internals
SMbotTextEvent *mbot_local_textevent_storage[EVENT_TEXT_STORAGE_SIZE];
SMbotHopEvent *mbot_local_joinevent_storage[EVENT_JOIN_STORAGE_SIZE];
SMbotHopEvent *mbot_local_partevent_storage[EVENT_PART_STORAGE_SIZE];
SMbotTextEvent *mbot_web_textevent_storage[EVENT_TEXT_STORAGE_SIZE];
SMbotHopEvent *mbot_web_joinevent_storage[EVENT_JOIN_STORAGE_SIZE];
SMbotHopEvent *mbot_web_partevent_storage[EVENT_PART_STORAGE_SIZE];
*/
/*
static int parse_def_texts(SMbotEventStorage *_this, FILE *readfile);
static int add_event(SMbotEventStorage * _this,ScallbackConf *eventspec);
static SMbotEvent *find_event_by_idandcbtype(SMbotEventStorage *_this,int id, EMbotcallbackEventType type);

static mbot_linkedList *callback_parse_originators(const char *originator);
static mbot_linkedList *callback_get_default_cb_list(EMbotcallbackEventType type);
static void mbot_event_storages_uninit(SMbotEventStorage **_this_)
{
	SMbotEventStorage *_this;
	MAZZERT(NULL!=_this_,"NULL ptr given to mbot_event_storages_uninit()")
	_this=*_this_;
		

	MbotAtomic32Uninit(&(_this->storageIndex_LocalTxtEvent));
	MbotAtomic32Uninit(&(_this->storageIndex_WebTxtEvent));
	MbotAtomic32Uninit(&(_this->storageIndex_LocalJoin));
	MbotAtomic32Uninit(&(_this->storageIndex_WebJoin));
	MbotAtomic32Uninit(&(_this->storageIndex_LocalPart));
	MbotAtomic32Uninit(&(_this->storageIndex_WebPart));
	free(_this);
	_this=NULL;
}
*/
/*XXX: We could allocate all event structs already, but it would cause performance loss 
 * (increase mem usage - it is not likely max amnt of events is always registered ;)) 
 * Also doing so would leave us without the benefit of using NULL as 'not used' tag
 */
/*
SMbotEventStorage * mbot_event_storages_init(void)
{
	SMbotEventStorage *_this;

	_this=calloc(1,sizeof(SMbotEventStorage));
	if(NULL==_this)
	{
		PPRINT("Event storage init failed for channel %s! (calloc failed)");
		return NULL;
	}
	_this->storageIndex_LocalTxtEvent=MbotAtomic32Init();
	_this->storageIndex_WebTxtEvent=MbotAtomic32Init();
	_this->storageIndex_LocalJoin=MbotAtomic32Init();
	_this->storageIndex_WebJoin=MbotAtomic32Init();
	_this->storageIndex_LocalPart=MbotAtomic32Init();
	_this->storageIndex_WebPart=MbotAtomic32Init();
	if
	(
		NULL==_this->storageIndex_LocalTxtEvent	||
		NULL==_this->storageIndex_WebTxtEvent		||
		NULL==_this->storageIndex_LocalJoin		||
		NULL==_this->storageIndex_WebJoin			||
		NULL==_this->storageIndex_LocalPart		||
		NULL==_this->storageIndex_WebPart
	)
	{
		perror("storages_init() FAILED!!\n");
		free(_this);
		return NULL;
	}
*/
/*	memset(&(_this->mbot_local_textevent_storage[0]),0,EVENT_TEXT_STORAGE_SIZE*sizeof(SMbotTextEvent*));
	memset(&(_this->mbot_local_joinevent_storage[0]),0,EVENT_JOIN_STORAGE_SIZE*sizeof(SMbotHopEvent*));
	memset(&(_this->mbot_local_partevent_storage[0]),0,EVENT_PART_STORAGE_SIZE*sizeof(SMbotHopEvent*));
	memset(&(_this->mbot_web_textevent_storage[0]),0,EVENT_TEXT_STORAGE_SIZE*sizeof(SMbotTextEvent*));
	memset(&(_this->mbot_web_joinevent_storage[0]),0,EVENT_JOIN_STORAGE_SIZE*sizeof(SMbotHopEvent*));
	memset(&(_this->mbot_web_partevent_storage[0]),0,EVENT_PART_STORAGE_SIZE*sizeof(SMbotHopEvent*));
*/
/*
	_this->uninit								=&mbot_event_storages_uninit;
	_this->find_by_idandcbtype					=&find_event_by_idandcbtype;
	_this->add_event							=&add_event;
	//This is not needed outside this file!
//	_this->localevent_add_default_event_string	=&localevent_add_default_event_string;
	_this->parse_def_events						=&parse_def_texts;

	return _this;
}
*/
/*
    int id;
        EMbotcallbackEventType type;            //Type of event which triggers CB, defined in callbackhandler.h
            int amntOfOriginators;
                char *originator;
                    char *triggerstring;
                        int amntOfLocations;
                        //    EMbotEventLocation locationoftxtevent[EVENT_LOCATIONS_MAX];
                        //        EMbotEventLocation *locationoftxtevent;
                        //
*/
/*
static int add_event(SMbotEventStorage * _this,ScallbackConf *eventspec)
{
	mbot_linkedList *default_cb_list;
    //Add event to storage based on type. (Next free slot??)
    //Initialize callback ID with default callback's ID.
	MAZZERT(NULL!=eventspec,"NULL event given to add_event()");


	if(NULL==(default_cb_list=callback_get_default_cb_list(eventspec->type)))
	{
		PPRINT("Getting default callback for event type %d failed!",eventspec->type);
		return -1;
	}
	switch(eventspec->type)
	{
		case EMbotcallbackEventType_LocalTxtEvent:
		{
            //TODO: Add triggerstrings and locations -Added, to be tested!
			unsigned int index;
//			char *originator;
			if(NULL==eventspec->triggerstring)
			{
				EPRINT("Tried to add text event with no triggerstring!");
				return -1;
			}
			mbot_atomicAdd(_this->storageIndex_LocalTxtEvent,1);
			if((index=mbot_atomicGet(_this->storageIndex_LocalTxtEvent))>EVENT_TEXT_STORAGE_SIZE)
			{
				mbot_atomicDec(_this->storageIndex_LocalTxtEvent,1);
				EPRINT("Maximum amount of local text callback reached, cannot register more!!");
				return -1;
			}
			index--; //indexing should start from 0, not from 1
			//Set mbot_local_textevent_storage[index]
			_this->mbot_local_textevent_storage[index]=malloc(sizeof(SMbotTextEvent));
			if(NULL==_this->mbot_local_textevent_storage[index])
			{
				mbot_atomicDec(_this->storageIndex_LocalTxtEvent,1);
				PPRINT("malloc() failed at add_event()");
				return -1;
			}
			_this->mbot_local_textevent_storage[index]->gen.type=eventspec->type;
			_this->mbot_local_textevent_storage[index]->gen.eventId=eventspec->id;
			_this->mbot_local_textevent_storage[index]->gen.userlevel=eventspec->userlevel;
			_this->mbot_local_textevent_storage[index]->gen.userlevel_direction=eventspec->userlevel_direction;
			_this->mbot_local_textevent_storage[index]->locationFilter=eventspec->locationoftxtevent;
			_this->mbot_local_textevent_storage[index]->triggerstring=malloc(strlen(eventspec->triggerstring)+1);
			if(NULL==_this->mbot_local_textevent_storage[index]->triggerstring)
			{
				mbot_atomicDec(_this->storageIndex_LocalTxtEvent,1);
				PPRINT("Malloc Failed at add_event()!");
				free(_this->mbot_local_textevent_storage[index]);
				return -1;
			}
			memcpy(_this->mbot_local_textevent_storage[index]->triggerstring,eventspec->triggerstring,strlen(eventspec->triggerstring)+1);
			if(NULL==(_this->mbot_local_textevent_storage[index]->gen.originators=callback_parse_originators(eventspec->originator_chan)))
			{
				mbot_atomicDec(_this->storageIndex_LocalTxtEvent,1);
				PPRINT("parsing originators FAILED!");
				free(_this->mbot_local_textevent_storage[index]->triggerstring);
				free(_this->mbot_local_textevent_storage[index]);
				_this->mbot_local_textevent_storage[index]=NULL;
//				memset(&(mbot_local_textevent_storage[index]),0,sizeof(SMbotTextEvent));
				return -1;
			}
			_this->mbot_local_textevent_storage[index]->gen.callbacks_for_this_event=default_cb_list;
			break;
		}
		case EMbotcallbackEventType_WebTxtEvent:
		{
			break;
		}
		case EMbotcallbackEventType_LocalJoin:
		{
			unsigned int index;
//			char *originator;
			mbot_atomicAdd(_this->storageIndex_LocalJoin,1);
			if((index=mbot_atomicGet(_this->storageIndex_LocalJoin))>EVENT_TEXT_STORAGE_SIZE)
			{
				mbot_atomicDec(_this->storageIndex_LocalJoin,1);
				EPRINT("Maximum amount of local join callback reached, cannot register more!!");
				return -1;
			}
			index--; //indexing should start from 0, not from 1
			//Set mbot_local_joinevent_storage[index]
			_this->mbot_local_joinevent_storage[index]=malloc(sizeof(SMbotHopEvent));
			if(NULL==_this->mbot_local_joinevent_storage[index])
			{
				mbot_atomicDec(_this->storageIndex_LocalJoin,1);
				PPRINT("malloc() failed at add_event()");
				return -1;
			}
			_this->mbot_local_joinevent_storage[index]->gen.type=eventspec->type;
			_this->mbot_local_joinevent_storage[index]->gen.eventId=eventspec->id;
			_this->mbot_local_joinevent_storage[index]->gen.userlevel=eventspec->userlevel;
			_this->mbot_local_joinevent_storage[index]->gen.userlevel_direction=eventspec->userlevel_direction;
			if(NULL==(_this->mbot_local_joinevent_storage[index]->gen.originators=callback_parse_originators(eventspec->originator_chan)))
			{
				mbot_atomicDec(_this->storageIndex_LocalJoin,1);
				PPRINT("parsing originators FAILED!");
				free(_this->mbot_local_joinevent_storage[index]);
				_this->mbot_local_joinevent_storage[index]=NULL;
//				memset(&(mbot_local_joinevent_storage[index]),0,sizeof(SMbotTextEvent));
				return -1;
			}
			_this->mbot_local_joinevent_storage[index]->gen.callbacks_for_this_event=default_cb_list;
			break;
		}
		case EMbotcallbackEventType_WebJoin:
		{
			break;
		}
		case EMbotcallbackEventType_LocalPart:
		{
			unsigned int index;
//			char *originator;
			mbot_atomicAdd(_this->storageIndex_LocalPart,1);
			if((index=mbot_atomicGet(_this->storageIndex_LocalPart))>EVENT_PART_STORAGE_SIZE)
			{
				mbot_atomicDec(_this->storageIndex_LocalPart,1);
				EPRINT("Maximum amount of local part callback reached, cannot register more!!");
				return -1;
			}
			index--; //indexing should start from 0, not from 1
			//Set values to mbot_local_partevent_storage[index]
			_this->mbot_local_partevent_storage[index]=malloc(sizeof(SMbotHopEvent));
			if(NULL==_this->mbot_local_partevent_storage[index])
			{
				mbot_atomicDec(_this->storageIndex_LocalPart,1);
				PPRINT("malloc() failed at add_event()");
				return -1;
			}
			_this->mbot_local_partevent_storage[index]->gen.type=eventspec->type;
			_this->mbot_local_partevent_storage[index]->gen.eventId=eventspec->id;
			_this->mbot_local_partevent_storage[index]->gen.userlevel=eventspec->userlevel;
			_this->mbot_local_partevent_storage[index]->gen.userlevel_direction=eventspec->userlevel_direction;
			if(NULL==(_this->mbot_local_partevent_storage[index]->gen.originators=callback_parse_originators(eventspec->originator_chan)))
			{
				mbot_atomicDec(_this->storageIndex_LocalPart,1);
				PPRINT("parsing originators FAILED!");
				free(_this->mbot_local_partevent_storage[index]);
				_this->mbot_local_partevent_storage[index]=NULL;
				return -1;
			}
			_this->mbot_local_partevent_storage[index]->gen.callbacks_for_this_event=default_cb_list;
			break;
		}
		case EMbotcallbackEventType_WebPart:
		{
			break;
		}
		default:
			MAZZERT(0,"Unknown callback event type given!");
			break;
	}
	return 0;
}

static mbot_linkedList *callback_parse_originators(const char *originator)
{
	mbot_linkedList *origlist;
	CexplodeStrings originator_exp;
	char *tmp;
	if(NULL==originator)
	{
		DPRINT("NULL originator given to callback_parse_originators");
		return NULL;
	}
	if(1>Cexplode(originator,(char *)ORIGINATOR_SEPARATOR_STR,&originator_exp))
	{
		PPRINT("Cexplode failed in callback_parse_originators()");
		return NULL;
	}
	origlist=mbot_ll_init();
	if(NULL==origlist)
	{
		PPRINT("mbot_ll_init() FAILED!");
		Cexplode_free(originator_exp);
		return NULL;
	}
	while(NULL!=(tmp=Cexplode_getnext(&originator_exp)))
	{
		if( !(tmp[0]=='#' || (tmp[0]=='-'&&tmp[1]=='#')) )
		{
			EPRINT("Malformed originator channel (lacking of # ?");
		Cexplode_free(originator_exp);
		return NULL;
		}
		mbot_ll_add(origlist,tmp);
	}
	Cexplode_free_allButPieces(originator_exp);
	return origlist;
}

static mbot_linkedList *callback_get_default_cb_list(EMbotcallbackEventType eventtype)
{
	mbot_linkedList *cblist;
	cblist=mbot_ll_init();
	SMazBotcallback_Local *cbstruct;	
	SMazBotcallback_Web *cbstruct_web;
	cbstruct=malloc(sizeof(SMazBotcallback_Local));
	cbstruct_web=malloc(sizeof(SMazBotcallback_Web));
	if(NULL==cbstruct || NULL==cbstruct_web)
	{
		PPRINT("Malloc failed at callback_reg()");
		return NULL;
	}
	memset(cbstruct_web,0,sizeof(SMazBotcallback_Local));
	memset(cbstruct_web,0,sizeof(SMazBotcallback_Web));
	if(NULL==cblist)
	{
		PPRINT("mbot_ll_init() FAILED!");
		free(cbstruct_web);
		free(cbstruct);
		return NULL;
	}
	switch(eventtype)
	{
		case EMbotcallbackEventType_LocalTxtEvent:
		{
			free(cbstruct_web);
			cbstruct->gen.type=EMbotcallbackEventType_LocalTxtEvent;
			cbstruct->cb=&defaultcb_LocalTxtEvent;
			mbot_ll_add(cblist,cbstruct);
			break;
		}
		case EMbotcallbackEventType_WebTxtEvent:
		{
			free(cbstruct);
			cbstruct_web->gen.type=EMbotcallbackEventType_WebTxtEvent;
			cbstruct_web->cb=&defaultcb_WebTxtEvent;
			MAZZERT(0,"Web events not yet implemented - serverdomain, serverport etc...");
			mbot_ll_add(cblist,cbstruct_web);
			break;
		}
		case EMbotcallbackEventType_LocalJoin:
		{
			free(cbstruct_web);
			cbstruct->gen.type=EMbotcallbackEventType_LocalJoin;
			cbstruct->cb=&defaultcb_LocalJoinEvent;
			mbot_ll_add(cblist,cbstruct);
			break;
		}
		case EMbotcallbackEventType_WebJoin:
		{
			free(cbstruct);
			cbstruct_web->gen.type=EMbotcallbackEventType_WebJoin;
			cbstruct_web->cb=&defaultcb_WebJoinEvent;
			MAZZERT(0,"Web events not yet implemented - serverdomain, serverport etc...");
			mbot_ll_add(cblist,cbstruct_web);
			break;
		}
		case EMbotcallbackEventType_LocalPart:
		{
			free(cbstruct_web);
			cbstruct->gen.type=EMbotcallbackEventType_LocalPart;
			cbstruct->cb=&defaultcb_LocalPartEvent;
			mbot_ll_add(cblist,cbstruct);
			break;
		}
		case EMbotcallbackEventType_WebPart:
		{
			free(cbstruct);
			cbstruct_web->gen.type=EMbotcallbackEventType_WebPart;
			cbstruct_web->cb=&defaultcb_WebPartEvent;
			MAZZERT(0,"Web events not yet implemented - serverdomain, serverport etc...");
			mbot_ll_add(cblist,cbstruct_web);
			break;
		}
		default:
			PPRINT("Unknown eventtype in callback_get_default_cb_list()!");
			mbot_ll_destroy(&cblist);
			break;
	}
	return cblist;
}
//XXX: If I ever fell in the trap of having multiple threads (which I am going to avaoid!) then it must be assured that all
//events have been registered before this is called. Othervice we're likely to get just increased index with mbot_atomicGet - before
//data is actually written in this index!
static SMbotEvent *find_event_by_idandcbtype(SMbotEventStorage *_this,int id, EMbotcallbackEventType type)
{
	SMbotEvent **evlist_to_use;	
	SMbotEvent *found=NULL;
	SMbotEvent **evlists[EMbotcallbackEventType_nmbrOf];
	int i;
	if((unsigned int)type>=EMbotcallbackEventType_nmbrOf || type==EMbotcallbackEventType_TEXTEVENTEND)
	{
		EPRINT("Invalid type given to find_event_by_idandcbtype()");
		return NULL;
	}
	evlists[EMbotcallbackEventType_LocalTxtEvent]=(SMbotEvent **)_this->mbot_local_textevent_storage;
	evlists[EMbotcallbackEventType_LocalJoin]=(SMbotEvent **)_this->mbot_local_joinevent_storage;
	evlists[EMbotcallbackEventType_LocalPart]=(SMbotEvent **)_this->mbot_local_partevent_storage;
	evlists[EMbotcallbackEventType_WebTxtEvent]=(SMbotEvent **)_this->mbot_web_textevent_storage;
	evlists[EMbotcallbackEventType_WebPart]=(SMbotEvent **)_this->mbot_web_partevent_storage;
	evlists[EMbotcallbackEventType_WebJoin]=(SMbotEvent **)_this->mbot_web_joinevent_storage;
	evlist_to_use=evlists[type];
	for(i=0;evlist_to_use[i]!=NULL;i++)
	{
		if(*(int *)evlist_to_use[i]==id)
		{
			found=evlist_to_use[i];
			DPRINT("find_event_by_idandcbtype() Found id %d",id);
			break;
		}
	}
	MAZZERT(EMbotcallbackEventType_LocalTxtEvent==type||EMbotcallbackEventType_LocalJoin==type||EMbotcallbackEventType_LocalPart==type,"Web events not yet implemented!");
	return found;
}

static SMbotEvent *find_event_by_id(SMbotEventStorage *_this,int id)
{
	int i;
	SMbotEvent *found=NULL;
	for(i=0;_this->mbot_local_textevent_storage[i]!=NULL&&found==NULL;i++)
	{
		if(id==*(int *)_this->mbot_local_textevent_storage[i])
		{
			found=(SMbotEvent *)_this->mbot_local_textevent_storage[i];
		}
	}
	for(i=0;_this->mbot_local_joinevent_storage[i]!=NULL&&found==NULL;i++)
	{
		if(id==*(int *)_this->mbot_local_joinevent_storage[i])
		{
			found=(SMbotEvent *)_this->mbot_local_joinevent_storage[i];
		}
	}
	for(i=0;_this->mbot_local_partevent_storage[i]!=NULL&&found==NULL;i++)
	{
		if(id==*(int *)_this->mbot_local_partevent_storage[i])
		{
			found=(SMbotEvent *)_this->mbot_local_partevent_storage[i];
		}
	}
	for(i=0;_this->mbot_web_textevent_storage[i]!=NULL&&found==NULL;i++)
	{
		if(id==*(int *)_this->mbot_web_textevent_storage[i])
		{
			found=(SMbotEvent *)_this->mbot_web_textevent_storage[i];
		}
	}
	for(i=0;_this->mbot_web_joinevent_storage[i]!=NULL&&found==NULL;i++)
	{
		if(id==*(int *)_this->mbot_web_joinevent_storage[i])
		{
			found=(SMbotEvent *)_this->mbot_web_joinevent_storage[i];
		}
	}
	for(i=0;_this->mbot_web_partevent_storage[i]!=NULL&&found==NULL;i++)
	{
		if(id==*(int *)_this->mbot_web_partevent_storage[i])
		{
			found=(SMbotEvent *)_this->mbot_web_partevent_storage[i];
		}
	}
	if(found!=NULL)
		DPRINT("Found event id %d",id);
	else
		WPRINT("Id %d not found from event storages!",id);
	return found;
}
static int localevent_add_default_event_string(SMbotEventStorage*_this,int id, char *string)
{
	SMbotEvent *ev=NULL;
	MAZZERT(NULL!=string,"Null string in localevent_add_default_event_string()");
	if(strlen(string)>=DEF_CB_TXT_MAX)
	{
		EPRINT("Too long defaultcallback string given in configs!");
		free(string);
		return -1;
	}
	ev=find_event_by_id(_this,id);
	if(NULL==ev)
	{
		EPRINT("Event %d matching defaultcb string \"%s\" not found! Bad Id?",id,string);
		return -1;
	}
	ev->default_cb_text=malloc(strlen(string)+1);
	if(NULL==ev->default_cb_text)
	{
		EPRINT("malloc FAILED at localevent_add_default_event_string()");
		return -2;
	}
	memcpy(ev->default_cb_text,string,strlen(string)+1);
	return 0;
}

//Parses the default events for a channel. If error occurs, returns -1
//If end of file is reached returns 1
//If channel is successfully parsed, and no eof found, returns 0
static int parse_def_texts(SMbotEventStorage *_this, FILE *readfile)
{
//	FILE *readfile;
	int ok=1,successcode=0;
	int end_reached=0;
	*/
/*	char *filename;
	size_t filenamesize;
	if(NULL==(filename=GetConfig(E_event_def_reply_file,&filenamesize)))
	{
		filename=EVENT_DEF_REPLY_FILE_DEFAULT;
//		testfilename_size=strlen(testfilename)+1;
	}
	readfile=fopen(filename,"r");
*/
	/*
	if(NULL==readfile)
	{
		perror("NULL file given to parse_def_texts()\n");
//		printf("Bad cofig file %s\n",filename);
		return -1;
	}
	else
	{
		while(ok)
		{
			int scanned;
			char *reply;
			int eventId;
			scanned=fscanf(readfile,"#%a[^\n]\n",&reply);
			if(scanned>0)
			{
				free(reply);
				continue;
			}
			scanned=fscanf(readfile,"[/CHANNEL]%a[^\n]\n",&reply);
			if(scanned>0)
			{
				free(reply);
				break;
			}
			scanned=fscanf(readfile,"%d:%a[^\n]\n",&eventId,&reply);
			if(scanned<=0)
			{
				printf("Probs we found the end.\n");
				ok=0;
				successcode=1;
			}
			else if(scanned!=2)
			{
				printf("Error Error Error! Malformed default cb reply file!\n");
			}
			else
			{
				int retval;
				printf("Adding scanned %d, %d:%s\n",scanned,eventId,reply);
				retval=localevent_add_default_event_string(_this,eventId,reply);
				if(-2==retval)
				{
					PPRINT("OMG! Something terribly bad happened while adding def cb string to event %d!",eventId);
					ok=0;
					successcode=-1;
				}
				else if(-1==retval)
				{
					WPRINT("Something a little bad happened - could not add defstring %s for event %d",reply,eventId);
				}
				else if(!retval)
				{
					DPRINT("Default Cb String added for event %d",eventId);
				}
			}
		}
	}
//	fclose(readfile);
	return successcode;
}
*/
