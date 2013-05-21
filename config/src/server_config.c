/* ********************************************************************
 *
 * @file server_config.c
 * @brief MazBot server config storage implementation.
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
#include <splitter.h>
#include <defaultcallbacks.h>



static int serverEventConfAdd(SServerConfigs *_this,SmbotPseudoxmlTag *eventRootTag);
static int ServerConfigsCfgAdd(SServerConfigs *_this,SmbotPseudoxmlTag* serverRootTag);

static int sanity_check_server_events(SServerConfigs *_this,SServerEvents *newevent)
{
	/* Check that compulsory things exist */
	if(newevent->eventType==0 )
	{
		EPRINTC(PrintComp_IrcCfg,"Eventtype missing!");
		goto ErrgetOutServer;
	}
	WPRINTC(PrintComp_IrcCfg,"No proper sanity checks for server event configs written!");
	/* Fill non specified defaults */
	if(NULL==newevent->event_string)
	{
		WPRINT
		(
			"Text event with no event string specified! (eventId %d, server %s)",
			newevent->eventId,
			_this->domain
		);
	}

	return 0;
ErrgetOutServer:
	/* TODO: Find a way if callback for this ID is registered. If so, ignore all checks. */
	EPRINTC(PrintComp_IrcCfg,"Invalid config for event %d on server %s",newevent->eventId,_this->domain);
	return -1;

}

static int serverEventConfAdd(SServerConfigs *_this,SmbotPseudoxmlTag *eventRootTag)
{
	SmbotPseudoxmlTag *tmptag;
	ServerCallbackF tmpcb;
	void *usercbdata;
	SServerEvents *newevent;
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
					EPRINTC(PrintComp_IrcCfg,"Event Tag handling FAILED for server %s event!",_this->domain);
					return -1;
				}
				break;
			}
		}
		if(!found)
		{
			EPRINTC(PrintComp_IrcCfg,"Unknow tag %s for server %s found from cfgfile!",tmptag->name,_this->domain);
			return -1;
		}
		tmptag=tmptag->next;
	}
	/* TODO: Check that all required event tags for event were set, that no insane values were set, and complete defaults */
	if(sanity_check_server_events(_this,newevent))
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid event for server %s!",_this->domain);
	}
	/* Associate callback(s) */
	/* TODO: create mechanism to add more than 1 cb / event */
	if(!(MbotBitsetGet(_this->generic_configs->registered_cbids,newevent->eventId)))
	{
		tmpcb=get_default_cb(newevent->eventType);
	}
	else
	{
		/* TODO: Add possibility to add more than 1 cb / event Id */
		tmpcb=_this->generic_configs->get_cb_for_event(_this->generic_configs,newevent->eventId,&usercbdata);
	}
		
	if(NULL==tmpcb)
	{
		PPRINTC(PrintComp_IrcCfg,"Can't find callback for event id %u",newevent->eventId);
		return -1;
	}
	if(callbacklist_init(&((*newevent).cblist),tmpcb,usercbdata))
	{
		PPRINTC(PrintComp_IrcCfg,"Failed to initialize callback list!");
		return -1;
	}


	_this->events[_this->amntofevents]=newevent;
	_this->amntofevents++;
	return 0;

}

SServerConfigs * serverConfigInit(SircConfig *generic_configs)
{
    SServerConfigs *_this;
	DPRINTC(PrintComp_IrcCfg,"Initializing server configs. Upperlayer struct at %p",generic_configs);
	MAZZERT(NULL!=generic_configs,"NULL arg given to serverConfigInit()");
    _this=malloc(sizeof(SServerConfigs));
    if(NULL==_this)
    {
        PPRINTC(PrintComp_IrcCfg,"Alloc failed at serverConfigInit()");
        return NULL;
    }
    memset(_this,0,sizeof(SServerConfigs));
	_this->generic_configs=generic_configs;
	_this->type=EcfgStructType_server;
    _this->ServerConfigsCfgAdd=&ServerConfigsCfgAdd;
	_this->serverEventConfAdd=&serverEventConfAdd;
	DPRINTC(PrintComp_IrcCfg,"serverConfigInit - server config struct initialized at %p",_this);
    return _this;
}


static int ServerConfigsCfgAdd(SServerConfigs *_this,SmbotPseudoxmlTag* serverRootTag)
{
    size_t servernamelen;
	int allocd_channels;
	int allocd_events;
    SmbotPseudoxmlTag *temptag;
    //SmbotPseudoxmlTag *chantag;

    /* Sanity checks */
    MAZZERT(NULL!=_this && NULL!=serverRootTag,"NULL ptr given to ServerConfigsCfgAdd()");
    MAZZERT( !strcmp(serverRootTag->name, "server"),"tag which type is not 'server' given to ServerConfigsCfgAdd() as root tag!");
    MAZZERT( serverRootTag->valuetype==EmbotPseudoxmlType_char,"Unsupported type for server tag!");
    MAZZERT( (int)serverRootTag->size > 0,"Zero or negative server name lenght!");
    servernamelen=strlen(serverRootTag->value);
    if(servernamelen>=sizeof(_this->domain))
    {
        EPRINTC(PrintComp_IrcCfg,"Too long server domain given! Max lenght %u chars",sizeof(_this->domain));
        return -1;
    }
    /* Fill server name */
    memcpy(_this->domain,serverRootTag->value,servernamelen+1);
    if(NULL==serverRootTag->subtags)
    {
        WPRINTC(PrintComp_IrcCfg,"No channels. users or other configs given for server %s",_this->domain);
    }
    /* Loop through the subtags && set values */
    DPRINTC(PrintComp_IrcCfg,"Tags preparsed => starting to fill channel structs");
    allocd_channels=10;
	allocd_events=10;
    _this->channels=malloc(allocd_channels*sizeof(SChannelConfigs *));
 	_this->events=malloc(allocd_events*sizeof(SServerEvents *));
	if(NULL==_this->channels || NULL == _this->events)
	{
		PPRINTC(PrintComp_IrcCfg,"malloc failed, out of mem??");
		return -1;
	}
    temptag=serverRootTag->subtags;
    for(;NULL!=temptag;)
    {
        if(!strcmp(temptag->name,"channel"))
        {
			DPRINTC(PrintComp_IrcCfg,"Found channel for server %s",_this->domain);
            /* Depth 1 channel tag parsing */
            if(allocd_channels<=_this->amntofchannels)
            {
				DPRINTC(PrintComp_IrcCfg,
						"Channel %d, but allocated only %d slots => reallocking to %d slots",
						_this->amntofchannels+1,
						allocd_channels,
						allocd_channels+10
				);
                allocd_channels+=10;
                _this->channels=realloc(_this->channels,sizeof(SChannelConfigs *)*allocd_channels);
                if(NULL==_this->channels)
                {
                    PPRINTC(PrintComp_IrcCfg,"Realloc failed while allocating space for channel specific configs!");
                    return -1;
                }
            }
			DPRINTC(PrintComp_IrcCfg,"Initializing channel[%d] for server %s",_this->amntofchannels,_this->domain);
            _this->channels[_this->amntofchannels]=channelConfigInit(_this);
            if(NULL==_this->channels[_this->amntofchannels])
            {
                PPRINTC(PrintComp_IrcCfg,"ChannelCfgInit FAILED");
                return -1;
            }
            if(0!=_this->channels[_this->amntofchannels]->ChannelConfigsCfgAdd(_this->channels[_this->amntofchannels],temptag))
            {
                EPRINTC(PrintComp_IrcCfg,"Final parsing for channel %s configs FAILED!",(char *)temptag->value);
                return -1;
            }
			DPRINTC(PrintComp_IrcCfg,
					"Channel %s at index[%d] adding successfull for server %s",
					_this->channels[_this->amntofchannels]->chan,
					_this->amntofchannels,
					_this->domain
			);
            DPRINTC(PrintComp_IrcCfg,"Channel configs successfully parsed for channel %s",(char *)temptag->value);
			_this->amntofchannels++;
        }
        else if(!strcmp(temptag->name,"port"))
        {
            DPRINTC(PrintComp_IrcCfg,"Port tag for server %s found, processing...",_this->domain);
            if(temptag->valuetype!=EmbotPseudoxmlType_16bit)
            {
                WPRINTC(PrintComp_IrcCfg,"Port tag for server %s is not expected type '16bit'",_this->domain);
                
            }
            /* TODO: Add function / macro to do this conversion safely */
            if(EmbotPseudoxmlType_8bit==temptag->valuetype || EmbotPseudoxmlType_char==temptag->valuetype)
                _this->port=*(unsigned char *)temptag->value;
            else if(EmbotPseudoxmlType_16bit==temptag->valuetype)
                _this->port=*(unsigned short *)temptag->value;
            else if(EmbotPseudoxmlType_32bit == temptag->valuetype)
                _this->port=(unsigned short int)*(unsigned int *)temptag->value;
            else if(EmbotPseudoxmlType_64bit == temptag->valuetype)
                _this->port=(unsigned short int)*(unsigned long long int *)temptag->value;
        }
		else if(!strcmp(temptag->name,"user"))
		{
			DPRINTC(PrintComp_IrcCfg,"Found user for server %s, adding..",_this->domain);
			if(_this->generic_configs->ircUserConfAdd(_this,temptag))	
			{
				EPRINTC(PrintComp_IrcCfg,"User adding for server %s FAILED",_this->domain);
				return -1;
			}
			DPRINTC(PrintComp_IrcCfg,
					"User at index %d (nick=%s) added to server %s",
					_this->useramnt-1,
					_this->users[_this->useramnt-1]->nick,
					_this->domain
			);
//			_this->useramnt++;
		}
		else if(!strcmp(temptag->name,"event"))
		{
			if(allocd_events<=_this->amntofevents)
            {
				DPRINTC(PrintComp_IrcCfg,
						"Event %d, but allocated only %d slots => reallocking to %d slots",
						_this->amntofevents+1,
						allocd_events,
						allocd_events+10
				);
                allocd_events+=10;
                _this->events=realloc(_this->events,sizeof(SServerEvents *)*allocd_events);
                if(NULL==_this->events)
                {
                    PPRINTC(PrintComp_IrcCfg,"Realloc failed while allocating space for event specific configs!");
                    return -1;
                }
            }

			if(_this->serverEventConfAdd(_this,temptag))
			{
				EPRINTC(PrintComp_IrcCfg,"Event adding for server %s FAILED",_this->domain);
				return -1;
			}
		}
		else if(!strcmp(temptag->name,"botnick"))
		{
			if(temptag->subtags!=NULL)
			{
				EPRINTC(PrintComp_IrcCfg,"tag botnick should be closed!");
				return -1;
			}
			if(temptag->valuetype!=EmbotPseudoxmlType_char)
			{
				EPRINTC(PrintComp_IrcCfg,"tag botnick has bad valuetype, should be EmbotPseudoxmlType_char!");
				return -1;
			}
    		if( (int)temptag->size <= 0 || temptag->size>sizeof(Tircnick))
			{
				EPRINTC(PrintComp_IrcCfg,"Zero or negative bot name lenght!");
				return -1;
			}
			memcpy(_this->mynick,temptag->value,sizeof(Tircnick));
		}
        /* else if(!strcmp(temptag->name,"ADD HERE OTHER ALLOWED TAGS AT SERVER LEVEL") */
        else
        {
            EPRINTC(PrintComp_IrcCfg,"Unknown server config %s given! - ignoring",temptag->name);
        }
        temptag=temptag->next;

    }
    /* Check that compulsory tags are given */
    if(0==_this->port)
    {
        WPRINTC(PrintComp_IrcCfg,"No port given for server %s => assuming 6667",_this->domain);
       _this->port=6667;
    }
	
	if('\0'==_this->generic_configs->mynick[0] && '\0'==_this->mynick[0])
	{
		WPRINTC(PrintComp_IrcCfg,"No generic or server specific nickname given when server %s seems to be configured!",_this->domain);
		DPRINTC(PrintComp_IrcCfg,"This is not fatal, since nothing says the 'botnick' cannot be given after server specific stuff");
	}
   return 0; 
}



