/* ********************************************************************
 *
 * @file irc_config.c
 * @brief MazBot pseudo xml config storage implementation.
 *
 *
 * -Revision History:
 *
 *  -0.0.7  16.03.2010/Maz  Small bug fixes
 *  -0.0.6  15.03.2010/Maz  Splitted
 *  -0.0.5  12.03.2010/Maz  Added some handling for events
 *  -0.0.4  10.03.2010/Maz  Added event structs and (empty) event adding funcs.
 *  -0.0.3  02.03.2010/Maz  Ironed out a few bugs - loads left still :D
 *  -0.0.2  28.02.2010/Maz  Still a draft :)
 *  -0.0.1  24.02.2010/Maz  First draft
 *
 * TODO: Split irc, server and channel stuff in separate files - this one looks like a puke now...
 *
 *  Lisence info: You're allowed to use / modify this - you're only required to
 *  write me a short note and your opinion about this to Mazziesaccount@gmail.com.
 *  if you redistribute this you're required to mention the original author:
 *  "Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
 *  channel.
 *
 *
 *  PasteLeft 2009 Maz.
 * ********************************************************************/

#include <irc_config.h>
#include <splitter.h>
#include <defaultcallbacks.h>
#define DEBUG_PRINTS

/**
 * @brief reads next tag, or if predefined tag name is given, finds matching tag.
 * @returns 0 on success, -1 if no (predefined / next) tag found.
 */
static int ircEventConfAdd(SircConfig *_this,SmbotPseudoxmlTag *eventRootTag);

static int IrcConfigRead(SircConfig *_this);

static int ircUserConfAdd(void *_this_,SmbotPseudoxmlTag *userRootTag);
//extern int callback_reg(unsigned int eventid,ServerCallbackF cbf, void *useropaque, size_t userdatasize, void *systemopaque);
extern int callback_register_hook(void *systemopaque);

#ifdef DEBUG_PRINTS

void printServerInfo(SircConfig *cfgs)
{
	int i,j;
	DPRINTC(PrintComp_IrcCfg,"Global Configs:");
	DPRINTC(PrintComp_IrcCfg,"Amnt of servers %d",cfgs->amntofservers);
	DPRINTC(PrintComp_IrcCfg,"Used cfg file '%s'",cfgs->configfile);
	DPRINTC(PrintComp_IrcCfg,"Tmp tag Storage ptr %p",cfgs->tag);

	DPRINTC(PrintComp_IrcCfg,"Server info:");
	for(i=0;i<cfgs->amntofservers;i++)
	{
		SServerConfigs *srv=cfgs->servers[i];
		DPRINTC(PrintComp_IrcCfg,"Server name %s",srv->domain);
		DPRINTC(PrintComp_IrcCfg,"\tServer port %d",(int)srv->port);
		DPRINTC(PrintComp_IrcCfg,"\tamntofchannels %d",srv->amntofchannels);
		DPRINTC(PrintComp_IrcCfg,"\tusers %d",srv->useramnt);
		DPRINTC(PrintComp_IrcCfg,"\tevents %d",srv->amntofevents);
		DPRINTC(PrintComp_IrcCfg,"\tchannel names:");
		for(j=0;j<srv->amntofchannels;j++)
		{
			DPRINTC(PrintComp_IrcCfg,"\t\t%s",srv->channels[j]->chan);
		}
		DPRINTC(PrintComp_IrcCfg,"\tserver users");
		for(j=0;j<srv->useramnt;j++)
		{
			DPRINTC(PrintComp_IrcCfg,"%s",srv->users[j]->nick);
		}
		DPRINTC(PrintComp_IrcCfg,"\tevents %u",srv->amntofevents);
		for(j=0;j<srv->amntofevents;j++)
		{
			DPRINTC(PrintComp_IrcCfg,"events TBD");
		}
	}
}

#endif



static int sanity_check_irc_events(SircConfig *_this,SServerEvents *newevent)
{
/* Check that compulsory things exist */
	if(newevent->eventType==0 )
	{
		EPRINTC(PrintComp_IrcCfg,"Eventtype missing!");
		goto ErrgetOutGeneric;
	}
/* Check that compulsory things exist */
	WPRINTC(PrintComp_IrcCfg,"No sanity checks for generic event configs written!");

/* Fill non specified defaults */
	if(NULL==newevent->event_string)
	{
		WPRINT
		(
			"Text event with no event string specified! (global event, eventId %d)",
			newevent->eventId
		);
	}

	return 0;
ErrgetOutGeneric:
	/* TODO: Find a way if callback for this ID is registered. If so, ignore all checks. */
	EPRINTC(PrintComp_IrcCfg,"Invalid config for global event %d",newevent->eventId);
	return -1;
}

static ServerCallbackF get_cb_for_event(SircConfig *_this,unsigned int eventId,void **usercbdata)
{
	ServerCallbackF foundcb=NULL;
	Sall_callbacks_list *cb=_this->cblist;
	*usercbdata=NULL;
	while(NULL!=cb)
	{
		if(cb->eventid == eventId)
		{
			foundcb=cb->cbf;
			*usercbdata=cb->useropaque;
			break;
		}
		cb=cb->next;
	}
	return foundcb;
}

static int ircEventConfAdd(SircConfig *_this,SmbotPseudoxmlTag *eventRootTag)
{
	SmbotPseudoxmlTag *tmptag;
	SServerEvents *newevent;
	ServerCallbackF tmpcb;
	int (*handle_event_tag[POSSIBLE_EVENT_TAG_NAMES_AMNT])(SServerEvents *,SmbotPseudoxmlTag *);
	int i;
	void *usercbdata=NULL;
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
					EPRINTC(PrintComp_IrcCfg,"Event Tag handling FAILED!");
					return -1;
				}
				break;
			}
		}
		if(!found)
		{
			EPRINTC(PrintComp_IrcCfg,"Unknow tag %s found from cfgfile!",tmptag->name);
			return -1;
		}
		tmptag=tmptag->next;
	}
	/* TODO: Check that all required event tags for event were set, that no insane values were set, and complete defaults */
	if(sanity_check_irc_events(_this,newevent))
	{
		EPRINTC(PrintComp_IrcCfg,"Invalid global event!");
	}
	if(!(MbotBitsetGet(_this->registered_cbids,newevent->eventId)))
	{
		tmpcb=get_default_cb(newevent->eventType);
	}
	else
	{
		/* TODO: Add possibility to add more than 1 cb / event Id */
		tmpcb=_this->get_cb_for_event(_this,newevent->eventId,&usercbdata);
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

int ircUserConfAdd(void *_this_,SmbotPseudoxmlTag *userRootTag)
{
	SircConfig *tmp;
	SircUser *newuser;
	SmbotPseudoxmlTag *attribtag;
	int retval=0;
	tmp=(SircConfig *)_this_;
	MAZZERT(NULL!=_this_ && NULL!=userRootTag, "NULL arg given to ircUserConfAdd()");
    MAZZERT( !strcmp(userRootTag->name, "user"), "tag which type is not 'user' given to ircUserConfAdd() as root tag!");
    MAZZERT( userRootTag->valuetype==EmbotPseudoxmlType_char, "Unsupported type for user tag!");
    MAZZERT( (int)userRootTag->size > 0 && userRootTag->size<sizeof(Tircnick), "Zero or negative user name lenght!");
	newuser=malloc(sizeof(SircUser));
	if(NULL==newuser)
	{
		PPRINTC(PrintComp_IrcCfg,"Alloc FAILED!");
		return -1;
	}
	memcpy(newuser->nick,userRootTag->value,userRootTag->size);
	if(NULL==userRootTag->subtags)
	{
		EPRINTC(PrintComp_IrcCfg,"user tag with no compulsory attribs found from cfgfile!");
		goto useradd_failure;
	}
	newuser->identmode=0xFFFF;
	//TODO: Ident mode decision => could be given in cfg file for each user!
	attribtag=userRootTag->subtags;
	while(NULL!=attribtag)
	{
		/* parse all user attribs */
		MAZZERT(NULL!=attribtag->name,"NULL name in tag - impossible!");
		if(!strcmp(attribtag->name,"host"))
		{
			if( attribtag->valuetype!=EmbotPseudoxmlType_char)
			{
				EPRINTC(PrintComp_IrcCfg,"Unsupported type for user (%s) host tag!",newuser->nick);
				goto useradd_failure;
			}
			if(attribtag->size>=sizeof(Tirchost) || attribtag->size<0)
			{
				EPRINTC(PrintComp_IrcCfg,"Invalid host name for user (%s)'s host tag!",newuser->nick);
				goto useradd_failure;
			}
			memcpy(newuser->host,attribtag->value,attribtag->size);
		}
		else if(!strcmp(attribtag->name,"pass"))
		{
			if( attribtag->valuetype!=EmbotPseudoxmlType_char)
			{
				EPRINTC(PrintComp_IrcCfg,"Unsupported type for user (%s) pass tag!",newuser->nick);
				goto useradd_failure;
			}
			if(attribtag->size>=sizeof(Tirchost) || attribtag->size<0)
			{
				EPRINTC(PrintComp_IrcCfg,"Invalid host name for user (%s)'s pass tag!",newuser->nick);
				goto useradd_failure;
			}
			memcpy(newuser->pass,attribtag->value,attribtag->size);
	
		}
		else if(!strcmp(attribtag->name,"ulevel"))
		{
			if( attribtag->valuetype!=EmbotPseudoxmlType_32bit)
			{
				EPRINTC(PrintComp_IrcCfg,"Unsupported type for user (%s) ulevel tag (must be 32bit)!",newuser->nick);
				goto useradd_failure;
			}
			if(attribtag->size!=1)
			{
				EPRINTC(PrintComp_IrcCfg,"User %s's ulevel tag cannot be an array!!",newuser->nick);
				goto useradd_failure;
			}
			if(EIRCuserLevel_owner<=*(unsigned int *)attribtag->value)
			{
				EPRINTC(PrintComp_IrcCfg,"Invalid userlevel %u given for user %s",*(unsigned int *)attribtag->value,newuser->nick);
				goto useradd_failure;
			}
			newuser->ulevel=*(EIRCuserLevel *)attribtag->value;
		}
		else if(!strcmp(attribtag->name,"userIdentMode"))
		{
			if( attribtag->valuetype!=EmbotPseudoxmlType_32bit)
			{
				EPRINTC(PrintComp_IrcCfg,"Unsupported type for user (%s) userIdentMode tag (must be 32bit)!",newuser->nick);
				goto useradd_failure;
			}
			if(attribtag->size!=1)
			{
				EPRINTC(PrintComp_IrcCfg,"User %s's userIdentMode tag cannot be an array!!",newuser->nick);
				goto useradd_failure;
			}
			if(EuserIdentMode_NmbrOf<=*(unsigned int *)attribtag->value)
			{
				EPRINTC(PrintComp_IrcCfg,"Invalid userIdentMode %u given for user %s",*(unsigned int *)attribtag->value,newuser->nick);
				goto useradd_failure;
			}
			newuser->identmode=*(EuserIdentMode *)attribtag->value;	
		}
		else
		{
			EPRINTC(PrintComp_IrcCfg,"Invalid user config for user %s - unknown subtag %s",newuser->nick,attribtag->name);
			goto useradd_failure;
		}
		attribtag=attribtag->next;
	}
	switch(newuser->identmode)
	{
		case EuserIdentMode_RegNick:
		case 0xFFFF:
		break;
		case EuserIdentMode_Passwd:
			if('\0'==newuser->pass[0])
			{
				EPRINTC(PrintComp_IrcCfg,"User %s ident mode set to Passwd, but no passwd specified!",newuser->nick);
				return -1;
			}
		break;
		case EuserIdentMode_Hostmask:
			if('\0'==newuser->host[0])
			{
				EPRINTC(PrintComp_IrcCfg,"User %s ident mode set to Host, but no host specified!",newuser->nick);
				goto useradd_failure;
			}
		break;
		default:
			MAZZERT(0,"Should not be here!");
			break;
	}
	switch(tmp->type)
	{
		case EcfgStructType_irc:
		{
		//	SircConfig *_this=(SircConfig *)_this_;
			/* Currently I believe we do not need global users, but if we do... Well, then this handling can be added */
			MAZZERT(0,"User tags outside server/channel not allowed!");
			break;
		}
		case EcfgStructType_server:
		{
			SServerConfigs *_this=(SServerConfigs *)_this_;
			DPRINTC(PrintComp_IrcCfg,"Adding user[%d] (nick=%s) to server %s",_this->useramnt,newuser->nick,_this->domain);
			if(0==_this->useramnt)
				_this->users=malloc(10*sizeof(SircUser *));
			else if(_this->useramnt%10)
			{
				DPRINTC(PrintComp_IrcCfg,
						"%d's user to be added, but space only for %d => reallocing to %d users",
						_this->useramnt+1,
						_this->useramnt,
						_this->useramnt+10
				);
				_this->users=realloc(_this->users,(_this->useramnt+10)*sizeof(SircUser *));
			}
			if(NULL==_this->users)
			{
				PPRINTC(PrintComp_IrcCfg,"Alloc/Realloc failed at ircUserConfAdd()! useramnt = %u",_this->useramnt);
				goto useradd_failure;
			}
			/* 
			 * TODO: If I ever make this multithreaded, then the assignment of newuser and increment of _this->useramnt should be atomic
			 * Eg. we should get here a local copy of _this->useramnt && increment it atomically. Then update according to local copy
			 */
			_this->users[_this->useramnt]=newuser;
			_this->useramnt++;
			break;
		}
		case EcfgStructType_chan:
		{
			SChannelConfigs *_this=(SChannelConfigs *)_this_;
			DPRINTC(PrintComp_IrcCfg,
					"Adding user[%d] (nick=%s) to channel %s on server %s",
					_this->useramnt,
					newuser->nick,
					_this->chan,
					_this->myserver->domain
			);
			if(0==_this->useramnt)
				_this->users=malloc(10*sizeof(SircUser *));
			else if(_this->useramnt%10)
			{
				DPRINTC(PrintComp_IrcCfg,
						"%d's user to be added, but space only for %d => reallocing to %d users",
						_this->useramnt+1,
						_this->useramnt,
						_this->useramnt+10
				);
				_this->users=realloc(_this->users,(_this->useramnt+10)*sizeof(SircUser *));
			}
			if(NULL==_this->users)
			{
				PPRINTC(PrintComp_IrcCfg,"Alloc/Realloc failed at ircUserConfAdd()! useramnt = %u",_this->useramnt);
				goto useradd_failure;
			}
			/* 
			 * TODO: If I ever make this multithreaded, then the assignment of newuser and increment of _this->useramnt should be atomic
			 * Eg. we should get here a local copy of _this->useramnt && increment it atomically. Then update according to local copy
			 */
			_this->users[_this->useramnt]=newuser;
			_this->useramnt++;
			break;
		}
		default:
			EPRINTC(PrintComp_IrcCfg,"Unknown struct type (%u) at ircUserConfAddF()",tmp->type);
			retval=-1;
			break;
	}
	if(0)
	{
useradd_failure:
		retval=-1;
		free(newuser);
	}
	return retval;	
}

static int ircConfigGetServeramnt(SircConfig *_this)
{
	return _this->amntofservers;
}
/* To return a copy of configs, or ptr to configs? What would be the best approach?? */
/* Currently returns ptr to cfgs, may be a problem if cfg struct is destroyed... */
static SServerConfigs * ircConfigGetServer(struct SircConfig *_this, int serverno)
{
	if(serverno<0 || serverno>=_this->amntofservers)
	{
		EPRINTC(PrintComp_IrcCfg,"Requested configs for server %d, no such server!",serverno);
		return NULL;
	}
	return _this->servers[serverno];
}
static int cfgcallbacklist_add(SircConfig *_this, unsigned int eventid,void *useropaque,ServerCallbackF cbf)
{
	Sall_callbacks_list *tmp,*new;
	if(NULL==_this->cblist)
	{
		_this->cblist=malloc(sizeof(Sall_callbacks_list));
		if(NULL==_this->cblist)
		{
			PPRINTC(PrintComp_IrcCfg,"FAILED to allocate callback! Out of mem?");
			return -1;
		}
		tmp=new=_this->cblist;
	}
	else
	{
		tmp=_this->cblist;
	
		while(NULL!=(new=tmp->next))
		{
			tmp=new;
		}
		new=malloc(sizeof(Sall_callbacks_list));
		if(NULL==new)
		{
			PPRINTC(PrintComp_IrcCfg,"FAILED to allocate callback! Out of mem?");
			return -1;
		}
	}
	new->eventid=eventid;
	new->useropaque=useropaque;
	new->cbf=cbf;
	tmp->next=new;
	new->next=NULL;
	/* write bit to 1 for this event ID - so we know we have a callback for the ID here! */
	MbotBitsetSet(_this->registered_cbids,eventid);
	return 0;
}
SircConfig *ircConfigInit(char *configfile)
{
    SircConfig *_this;
	int rval;
    size_t filenamelen;
	DPRINTC(PrintComp_IrcCfg,"Starting cfg initialization, cfg file=%s",configfile);
    MAZZERT(NULL!=configfile,"NULL configfile given!");
    filenamelen=strlen(configfile);
    _this=malloc(sizeof(SircConfig));
    if(NULL==_this)
    {
        PPRINTC(PrintComp_IrcCfg,"IrcConfigObj alloc FAILED!");
        return NULL;
    }
    memset(_this,0,sizeof(SircConfig));
    _this->configfile=malloc(filenamelen+1);
    if(NULL==_this->configfile)
    {
        PPRINTC(PrintComp_IrcCfg,"Alloc failed at ircConfigInit()");
        return NULL;
    }
    memcpy(_this->configfile,configfile,filenamelen+1);
	_this->callbacklist_add=&cfgcallbacklist_add;
	_this->type=EcfgStructType_irc;
	_this->ircUserConfAdd=&ircUserConfAdd;
    _this->IrcConfigRead=&IrcConfigRead;
	_this->ircEventConfAdd=&ircEventConfAdd;
	_this->ircConfigGetServeramnt=&ircConfigGetServeramnt;
	_this->ircConfigGetServer=&ircConfigGetServer;
	_this->get_cb_for_event=&get_cb_for_event;
	/* events from 0->EVENTID_MAX */
	_this->registered_cbids=MbotBitsetInit(EVENTID_MAX+1);
	if(NULL==_this->registered_cbids)
	{
		EPRINTC(PrintComp_IrcCfg,"ircConfigInit - Failed to init bitset for callback ids");
		return NULL;
	}
	if((rval=callback_register_hook(_this)))
	{
		EPRINTC(PrintComp_IrcCfg,"ircConfigInit - callback_register_hook() reported error %d in registration!",rval);
		return NULL;
	}
	DPRINTC(PrintComp_IrcCfg,"ircConfigInit - User callbacks successfully registered!");

	DPRINTC(PrintComp_IrcCfg,"ircConfigInit - toplevel config struct initialized at %p",_this);
    return _this;
}

static int IrcConfigRead(SircConfig *_this)
{
	FILE *readfile;
	int allocd_servers;
	int allocd_events;
	//int ok;
//    SServerConfigs *servers=NULL;;
    SmbotPseudoxmlTag *tmptag;
    _this->tag=NULL;
	MAZZERT(NULL!=_this,"Null ptr given to serverConfigRead");
	readfile=fopen(_this->configfile,"r");
    if(NULL==readfile)
    {
        EPRINTC(PrintComp_IrcCfg,"Failed to open config file (%s)",_this->configfile);
        return -1;
    }
	/**
	 * Find server tag 
	 * */
    if(0!=get_tags(&_this->tag, readfile))
    {
        //Error, tag handling failed!
        EPRINTC(PrintComp_IrcCfg,"Failed to parse tags from file %s",_this->configfile);
        fclose(readfile);
        return -1;
    }
    fclose(readfile);
    if(_this->tag==NULL)
    {
        EPRINTC(PrintComp_IrcCfg,"No configs parsed, problem in file %s ?? !",_this->configfile);
        return -1;
    }
    /* This tag struct should now be used to fill up servers */
#ifdef DEBUGPRINTS
    print_tags(tag);
#endif
    DPRINTC(PrintComp_IrcCfg,"Tags preparsed => starting to fill server structs");
    allocd_servers=10;
	allocd_events=10;
    _this->servers=malloc(allocd_servers*sizeof(SServerConfigs *));
	if(NULL==_this->servers)
	{
		PPRINTC(PrintComp_IrcCfg,"malloc FAILED, out of mem?");
		return -1;
	}
    _this->events=malloc(allocd_events*sizeof(SServerEvents *));
	if(NULL==_this->events)
	{
		PPRINTC(PrintComp_IrcCfg,"malloc FAILED, out of mem?");
		return -1;
	}

    tmptag=_this->tag;
    for(;tmptag!=NULL;)
    {
        /* Depth 0 tag parsing: */
        if(NULL==tmptag->name)
        {
            EPRINTC(PrintComp_IrcCfg,"Tag without name found, assuming logic error && aborting tag reading!");
            return -1;
        }
 		else if(!strcmp(tmptag->name,"event"))
		{
			/* This is event tag */
	        if(allocd_events<=_this->amntofevents)
            {
                allocd_events+=10;
                _this->events=realloc(_this->events,sizeof(SServerEvents *)*allocd_events);
                if(NULL==_this->events)
                {
                    PPRINTC(PrintComp_IrcCfg,"Realloc failed while allocating space for event specific configs!");
                    return -1;
                }
            }
    		if(_this->ircEventConfAdd(_this,tmptag))
			{
				EPRINTC(PrintComp_IrcCfg,"Global Event adding FAILED");
				return -1;
			}
		}
		else if(!strcmp(tmptag->name,"botnick"))
		{
			DPRINTC(PrintComp_IrcCfg,"Global botnick tag found!");
            if(tmptag->subtags!=NULL)
		    {
				EPRINTC(PrintComp_IrcCfg,"tag botnick should be closed!");
				return -1;
			}
		    if(tmptag->valuetype!=EmbotPseudoxmlType_char)
			{
				EPRINTC(PrintComp_IrcCfg,"tag botnick has bad valuetype, should be EmbotPseudoxmlType_char!");
				return -1;
			}
			if( (int)tmptag->size <= 0 || tmptag->size>sizeof(Tircnick))
			{
				EPRINTC(PrintComp_IrcCfg,"Zero or negative bot name lenght!");
				return -1;
			}
			DPRINTC(PrintComp_IrcCfg,"Global botnick %s found!",(char *)tmptag->value);
			strncpy(_this->mynick,tmptag->value,sizeof(Tircnick));
			_this->mynick[sizeof(Tircnick)-1]='\0';
			DPRINTC(PrintComp_IrcCfg,"botnick %s set to global configs!",_this->mynick);

		}
       /*else if( TODO: Add all allowed global tags here )
         * {
         * Handle adding global tag
         * tmptag=tmptag->next;
         * continue;
         * }
         */
        else if(strcmp(tmptag->name,"server")) 
        {
            EPRINTC(PrintComp_IrcCfg,"Unsupported tag found from file %s (or tag not at appropriate place): tagname=%s: Ignoring",_this->configfile,tmptag->name);
            tmptag=tmptag->next;
            continue;
        }
        else
        {
			/* This is server tag */
            /* Depth 0 server tag parsing */
            if(allocd_servers<=_this->amntofservers)
            {
                allocd_servers+=10;
                _this->servers=realloc(_this->servers,sizeof(SServerConfigs *)*allocd_servers);
                if(NULL==_this->servers)
                {
                    PPRINTC(PrintComp_IrcCfg,"Realloc failed while allocating space for server specific configs!");
                    return -1;
                }
            }
            _this->servers[_this->amntofservers]=serverConfigInit(_this);
            if(NULL==_this->servers[_this->amntofservers])
            {
                PPRINTC(PrintComp_IrcCfg,"ServerCfgInit FAILED");
                return -1;
            }
            if(0!=_this->servers[_this->amntofservers]->ServerConfigsCfgAdd(_this->servers[_this->amntofservers],tmptag))
            {
                EPRINTC(PrintComp_IrcCfg,"Final parsing for server %s configs FAILED!",(char *)tmptag->value);
                return -1;
            }
            _this->amntofservers++;
        }
        tmptag=tmptag->next;
    }
#ifdef DEBUG_PRINTS
	printServerInfo(_this);
#endif
    return 0;
}
