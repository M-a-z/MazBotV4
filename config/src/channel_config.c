/* ********************************************************************
 *
 * @file channel_config.c
 * @brief MazBot channel config storage implementation.
 *
 *
 * -Revision History:
 *
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



#include "irc_config.h"
#include <defaultcallbacks.h>
#include <splitter.h>


static int channelEventConfAdd(SChannelConfigs *_this,SmbotPseudoxmlTag *eventRootTag);
static int ChannelConfigsCfgAdd(SChannelConfigs *_this,SmbotPseudoxmlTag* channelRootTag);
static int sanity_check_channel_events(SChannelConfigs *_this,SServerEvents *newevent);

/* These propably need to allow more strange configs when own C callbacks are registered to handle event */
static int sanity_check_channel_events(SChannelConfigs *_this,SServerEvents *newevent)
{
	/* TODO: Find a way to ensure event Ids are unique.. */
	/* Check that eventtype is set */
	if(newevent->eventType==0 )
	{
		EPRINTC(PrintComp_IrcCfg,"Eventtype missing!");
		goto ErrgetOut;
	}
	/* On channel level we can only have events originating from a channel (and it is naturally the default then)*/
	if(0==newevent->inputsrc.inputsource)
	{
		newevent->inputsrc.inputsource = EMbotEventLocation_Chan;
	}
	/* Default output dev for events on channel is also the channel :) */
	if( 0 == newevent->outputdst.outputdevice)
	{
		newevent->outputdst.outputdevice = EMbotEventLocation_Chan;
	}
	/* On channel level we can only have events originating from a channel */
	if(EMbotEventLocation_Chan != newevent->inputsrc.inputsource )
	{
		EPRINT
		(
			"ERR, event%u, chan %s, srv %s: Events originating from somewhere else but chan must not be specified as channel subtags!",
			newevent->eventId,
			_this->chan,
			_this->myserver->domain
		);
		goto ErrgetOut;
	}
	/* We can have channel events which we wish to notify in privmsg. But then the receiver's nick (TODO: also level is option) must be specified */
	if
	(
		EMbotEventLocation_Privmsg==newevent->outputdst.outputdevice 
	)
	{
		if(NULL==newevent->outputdst.outputname)
		{
			EPRINTC(PrintComp_IrcCfg,"Event output expected to be sent in privmsg, but privmsg receiver not specified!");
			goto ErrgetOut;
		}
	}
	MAZZERT(newevent->levelamnt==0 || newevent->levellist!=NULL,"Nonzero level amnt,but NULL level list!" ); 
	MAZZERT(newevent->levelamnt!=0 || newevent->levellist==NULL,"Zero level amnt,but non NULL level list!" ); 
	/*
	 *
	 * typedef enum EMbotEventLocation
	 * {   
	 *     EMbotEventLocation_Privmsg      = 1,
	 *         EMbotEventLocation_Chan         = 2,
	 *             EMbotEventLocation_CPM          = 3, //Chan or privmsg
	 *                 EMbotEventLocation_Dcc          = 10, //Dcc (may not realize)
	 *                     EMbotEventLocation_CD           = 11, //Chan or Dcc (may not realize)
	 *                         EMbotEventLocation_PMD          = 12, //Privmsg or Dcc (may not realize)
	 *                             EMbotEventLocation_All          = 99
	 *                             }EMbotEventLocation;
	 */
	/* Fill non specified defaults */
	if
	(
		EMbotEventLocation_CPM == newevent->outputdst.outputdevice ||
		EMbotEventLocation_All == newevent->outputdst.outputdevice
	)
	{
		EPRINTC(PrintComp_IrcCfg,"Only one outputdevice / event allowed! (cannot specify EMbotEventLocation_CPM or EMbotEventLocation_All)");
		goto ErrgetOut;
	}
	if
	(
		newevent->eventType == EMbotcallbackEventType_LocalTxtEvent ||
		newevent->eventType == EMbotcallbackEventType_WebTxtEvent 
	)
	{
		if(NULL==newevent->event_string)
		{
			WPRINT
			(
				"Text event with no event string specified! (eventId %d, chan %s, server %s)",
				newevent->eventId,
				_this->chan,
				_this->myserver->domain
			);
		}
	}
	return 0;
ErrgetOut:
	/* TODO: Find a way if callback for this ID is registered. If so, ignore all checks. */
	EPRINT
	(
		"Invalid event %d, chan %s on server %s",
		newevent->eventId,
		_this->chan,
		_this->myserver->domain
	);
	return -1;
}



static int channelEventConfAdd(SChannelConfigs *_this,SmbotPseudoxmlTag *eventRootTag)
{
	SmbotPseudoxmlTag *tmptag;
	void *usercbdata;
	SServerEvents *newevent;
	ServerCallbackF tmpcb;
	int (*handle_event_tag[POSSIBLE_EVENT_TAG_NAMES_AMNT])(SServerEvents *,SmbotPseudoxmlTag *);
	int i;
	handle_event_tag[0]=&serverEventHandleEventtype;
	handle_event_tag[1]=&serverEventHandleUserlevel;
	handle_event_tag[2]=&serverEventHandleTriggerstring;
	handle_event_tag[3]=&serverEventHandleOutputto;
	handle_event_tag[4]=&serverEventHandleOutputchan;
	handle_event_tag[5]=&serverEventHandleInputfrom;
	handle_event_tag[6]=&serverEventHandleOutputstring;
	/* WARNING! This macro may call return! */
	SANITY_CHECK_EVENT_ADD(_this,eventRootTag);
	newevent=malloc(sizeof(SServerEvents));
	if(NULL==newevent)
	{
		PPRINTC(PrintComp_IrcCfg,"mallocking event cfg struct FAILED, out of mem?");
		return -1;
	}
	memset(newevent,0,sizeof(SServerEvents));
	newevent->eventId=*(int*)(eventRootTag->value);
	tmptag=eventRootTag->subtags;
	while(NULL!=tmptag)
	{
		int found=0;
		MAZZERT(NULL!=tmptag->name,"Impossible happened! - name can not be null");
		for(i=0;i<POSSIBLE_EVENT_TAG_NAMES_AMNT;i++)
		{
			if(!strcmp(tmptag->name,allowed_event_tags[i]))
			{
				found=1;
				if(handle_event_tag[i](newevent,tmptag))
				{
					EPRINTC(PrintComp_IrcCfg,"Event Tag handling for channel %s event FAILED!",_this->chan);
					return -1;
				}
				break;
			}
		}
		if(!found)
		{
			EPRINTC(PrintComp_IrcCfg,"Unknow tag %s for channel %s found from cfgfile!",tmptag->name,_this->chan);
			return -1;
		}
		tmptag=tmptag->next;
	}
	/* TODO: Check that all required event tags for event were set, that no insane values were set, and complete defaults */
	if(sanity_check_channel_events(_this,newevent))
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid event for channel %s on server %s!",_this->chan,_this->myserver->domain);
	}

	/* Associate callback(s) */
	/* TODO: create mechanism to add more than 1 cb / event */
	if(!(MbotBitsetGet(_this->myserver->generic_configs->registered_cbids,newevent->eventId)))
	{
		tmpcb=get_default_cb(newevent->eventType);
	}
	else
	{
		/* TODO: Add possibility to add more than 1 cb / event Id */
		tmpcb=_this->myserver->generic_configs->get_cb_for_event(_this->myserver->generic_configs,newevent->eventId,&usercbdata);
	}
		
	if(NULL==tmpcb)
	{
		PPRINTC(PrintComp_IrcCfg,"Can't find callback for event id %u",newevent->eventId);
		return -1;
	}
	if(callbacklist_init(&(newevent->cblist),tmpcb,usercbdata))
	{
		PPRINTC(PrintComp_IrcCfg,"Failed to initialize callback list!");
		return -1;
	}



	_this->events[_this->amntofevents]=newevent;
	_this->amntofevents++;
	return 0;

}

SChannelConfigs * channelConfigInit(SServerConfigs *myserver)
{
    SChannelConfigs *_this;
	DPRINTC(PrintComp_IrcCfg,"Initializing channel configs. Server struct at %p",myserver);
	MAZZERT(NULL!=myserver,"NULL arg to channelConfigInit()!");
    _this=malloc(sizeof(SChannelConfigs));
    if(NULL==_this)
    {
        PPRINTC(PrintComp_IrcCfg,"Alloc failed at channelConfigInit()");
        return NULL;
    }
    memset(_this,0,sizeof(SChannelConfigs));
	_this->myserver=myserver;
	_this->type=EcfgStructType_chan;
    _this->ChannelConfigsCfgAdd=&ChannelConfigsCfgAdd;
	_this->channelEventConfAdd=&channelEventConfAdd;
	DPRINTC(PrintComp_IrcCfg,"serverConfigInit - channel config struct initialized (server %s)",_this->myserver->domain);
    return _this;

}

static int ChannelConfigsCfgAdd(SChannelConfigs *_this,SmbotPseudoxmlTag* channelRootTag)
{
	size_t channamelen;
	SmbotPseudoxmlTag* temptag;
	int allocd_events;
    /* Sanity checks */
    MAZZERT(NULL!=_this && NULL!=channelRootTag,"NULL ptr given to ChannelConfigsCfgAdd()");
    MAZZERT( !strcmp(channelRootTag->name, "channel"),"tag which type is not 'channel' given to ChannelConfigsCfgAdd() as root tag!");
    MAZZERT( channelRootTag->valuetype==EmbotPseudoxmlType_char,"Unsupported type for channel tag!");
    MAZZERT( (int)channelRootTag->size > 0,"Zero or negative channel name lenght!");
	if( (channamelen=strlen(channelRootTag->value))>sizeof(_this->chan))
	{
		EPRINTC(PrintComp_IrcCfg,"Given channel name too long(%s:%u chars), max allowed channamelen = %u",(char *)channelRootTag->value,channelRootTag->size,sizeof(_this->chan));
		return -1;
	}
	DPRINTC(PrintComp_IrcCfg,"Adding configs for channel (%s)",(char *)channelRootTag->value);
	memcpy(_this->chan,channelRootTag->value,channamelen+1);
	allocd_events=10;
	_this->events=malloc(allocd_events*sizeof(SServerEvents *));
	if(NULL==_this->events)
	{
		PPRINTC(PrintComp_IrcCfg,"malloc failed, out of mem?");
		return -1;
	}
	/* Lets see the subtags then... */
	temptag=channelRootTag->subtags;
    for(;NULL!=temptag;)
    {
        if(!strcmp(temptag->name,"user"))
        {
			/* user cfg:
			 * <name=user type=char value=username>
			 * 		<name=mask type=char value=hostmask/>
			 * 		<name=passwd type=char value=passwd/>
			 * 		<name=ulevel type=32bit value=N/>
			 * 		<name=userIdentMode type=32bit value=N/>
			 * </user>
			 */
			DPRINTC(PrintComp_IrcCfg,"Found user for chan %s, (server %s) adding user",_this->chan,_this->myserver->domain);
			if(_this->myserver->generic_configs->ircUserConfAdd(_this,temptag))	
			{
				EPRINTC(PrintComp_IrcCfg,"User adding for channel %s FAILED",_this->chan);
				return -1;
			}
		}
		else if(!strcmp(temptag->name,"event"))
		{
			if(_this->amntofevents>=allocd_events)
			{
				allocd_events+=10;
				_this->events=realloc(_this->events,allocd_events*sizeof(SServerEvents *));
				if(NULL==_this->events)
				{
					PPRINTC(PrintComp_IrcCfg,"realloc FAILED, out of mem?");
					return -1;
				}
			}
			if(_this->channelEventConfAdd(_this,temptag))
			{
				EPRINTC(PrintComp_IrcCfg,"Event adding FAILED for channel %s, server %s",_this->chan,_this->myserver->domain);
				return -1;
			}
		}
		/* TODO: Add all the rest possible configs here
		else if(!strcmp(temptag->name,""))
		{

		}
		*/
		else
		{
			EPRINTC(PrintComp_IrcCfg,"Unknow config tag (%s) for channel %s",temptag->name,_this->chan);
			free(temptag);
			return -1;
		}
		temptag=temptag->next;
	}
	return 0;
}





