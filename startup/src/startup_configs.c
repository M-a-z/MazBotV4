/* ********************************************************************
 *
 * @file startup_configs.c
 * @brief read and set configurations at startup.
 *
 *
 * -Revision History:
 *
 *  - 0.0.1 17.08.2009/Maz  First Draft
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

#include <stdio.h>
#include <config.h>
#include <configNumbers.h>
#include <startup_configs.h>
#include <parser.h>
#include <config_file_parser.h>
#include <event_storage.h>
#include <basic_file_parser.h>
#include <user.h>

static int CFG_USER_IDENT_MODE_DEFAULT = EuserIdentMode_RegNick;
static char *CFG_FILE_DEFAULT="testfile";
//static char *EVENT_DEF_REPLY_FILE_DEFAULT="testfile2";

//static const char *identmodeFiles[3]={"userlevel_modeRegNick.conf","userlevel_modePasswd.conf","userlevel_modeHostmask.conf"};


int get_basic_configs(void)
{
    Sparser *cfgparser;
    SbasicParserResult *res;
    EparserState state;
    SbasicConfig *cfg;

    cfgparser=ParserInit(EparserType_CfgBasic);
    if(NULL==cfgparser)
    {
        PPRINT("Failed to create cfgparser!");
        return -1;
    }
    if(EparserRetVal_Ok!=cfgparser->feed(cfgparser,BOT_CONFIG_FILE,strlen(BOT_CONFIG_FILE)+1,&state))
    {
        PPRINT("Parsing config file %s FAILED!",BOT_CONFIG_FILE);
        //Free parser?
        return -1;
    }
    if(state!=EparserState_ResultReady)
    {
        WPRINT("No configs found!");
        //uninit!!!
        return 0;
    }
    res=(SbasicParserResult *)cfgparser->get_result(cfgparser,&state);
    if(NULL==res)
    {
        if(state!=EparserState_ParserEmpty)
        {
            PPRINT("Error while handling cfgfile");
            return -1;
        }
        else
        {
            WPRINT("No cfgs found from cfgfile!");
            return 0;
        }
    }
    cfg=res->getNextCfgs(res);
    while(NULL!=cfg)
    {
        if(EconfigRet_Ok!=SetConfig(cfg->cfgid,cfg->data,cfg->datasize))
        {
            PPRINT("Setting cfg %d FAILED",cfg->cfgid);
        }
    	cfg=res->getNextCfgs(res);
    }
    cfgparser->uninit(&cfgparser);
    ((SparserResult *)res)->free((SparserResult **)&res);
	//SetConfig(CALLBACK_EVENT_FILE,"valueReadFromGenCfgFile",strlen(valueReadFromGenCfgFile));
	//WPRINT("Just a dummy get_basic_configs() implementation!");
	return 0;
}

void clear_basic_configs(void)
{
    WPRINT("clear_pasic_configs() has just dummy implementation!");

}


int get_user_configs(void)
{
	char *IdentMode_char;
	EuserIdentMode IdentMode;
	size_t identModeSize;
	//Global defined in permanent_storage.c - Move init to runlevel 1!	
	perm_storage_inited=MbotAtomic32Init();
	//Global defined in permanent_storage.c
	if(mbot_atomicAdd(perm_storage_inited,1)!=0)
	{
		EPRINT("Permanent storage init problem, already inited?");
		return -1;
	}
    if((NULL==(IdentMode_char=GetConfig(E_user_ident_mode,&identModeSize))) || sizeof(char)!=identModeSize)
	{
		IdentMode=CFG_USER_IDENT_MODE_DEFAULT;
	}
	else
	{
		IdentMode=(int)(*IdentMode_char-'0');
		if((unsigned int)IdentMode>(unsigned int)EuserIdentMode_NmbrOf)
		{
			WPRINT("Garbage value for user ident mode in cfg file! - using defaults");
			IdentMode=CFG_USER_IDENT_MODE_DEFAULT;
		}
	}

	G_permanent_user_storage=(SIRCuserHandlerPermanent *)StorageInit(EuserHandlerType_Permanent, IdentMode);
	if(NULL==G_permanent_user_storage)
	{
		PPRINT("Perm storage init failed!");
		return -1;
	}
	if(NULL==perm_storage_inited)
	{
		PPRINT("Could not init atmic var to protect perm storage => out of mem?");
		return -1;
	}
	if(0>G_permanent_user_storage->syncFromFile(G_permanent_user_storage))
	{
		PPRINT("Getting user configs fromfile FAILED!");
		return -1;
	}
//setting perm_storage_inited to 2 advertices perm storage switching online. 
//TODO: Change this to use config? Then depending things could be started via listener callbacks.
	MAZZERT(1==mbot_atomicAdd(perm_storage_inited,1),"Impossible!"); 
	return 0;
	//TODO: Add userLevelFileNames in configs. - added, read from configs needed.
	//TODO: Cannot use basic parser, new file format is not numeral:text.
	//New format is:
	//nick@mask/passwd:server:chan1;level1:chan2;level2:chan3;level3...
	//TODO: Add reading the permanent storage in storage class - ongoing... But troublesome
	//
}
/*	Sparser *userparser;
	SbasicParserResult *res;
	EparserState state;
    SbasicConfig *cfg;
	CexplodeStrings userploder;

	userparser=ParserInit(EparserType_CfgBasic);
    if(NULL==userparser)
    {
        PPRINT("Failed to create userparser!");
        return -1;
    }
	}


    if(EparserRetVal_Ok!=userparser->feed(userparser,identmodeFiles[IdentMode],strlen(identmodeFiles[IdentMode])+1,&state))
    {
        PPRINT("Parsing config file %s FAILED!",identmodeFiles[IdentMode]);
        //Free parser?
        return -1;
    }
    if(state!=EparserState_ResultReady)
    {
        WPRINT("No configs found!");
        //uninit!!!
        return 0;
    }
    res=(SbasicParserResult *)userparser->get_result(userparser,&state);
    if(NULL==res)
    {
        if(state!=EparserState_ParserEmpty)
        {
            PPRINT("Error while handling cfgfile");
            return -1;
        }
        else
        {
            WPRINT("No cfgs found from cfgfile!");
            return 0;
        }
    }
    cfg=res->getNextCfgs(res);
	//cfg struct misused, it's now:
	//cfg->cfgid == userlevel.
	//cfg->data == nick@host:servername:chan1:chan2:cahan3:...
	//or
	//cfg->data == nick@passwd:servername:chan1:chan2:cahan3:...
	//or
	//data == nick@host/passwd:servername:*   
	//or
	//data == nick@host/passwd:*:*
	//So there should be at least 3 fields filled!
    while(NULL!=cfg)
    {
		if(3>Cexplode(cfg->data,":",&userploder))
		{
			WPRINT("Malicious line in userlevel cfg file!");
		}
		else
		{
			char *nick;
			char *passhost;
			char *servername;
			mbot_linkedList *channels;
			char *chantmp;
			size_t i,j;
			channels=mbot_ll_init();
			if(NULL==channels)
			{
				PPRINT("get_user_configs(): mbot_ll init FAILED!");
				return -1;
			}
			nick=Cexplode_getfirst(userploder);
			MAZZERT(NULL!=nick,"ImpossibleHappened again");
			i=strlen(nick);
			for(j=0;j<i;j++)
			{
				if('@'==nick[j])
				{
					nick[j]='\0';
					if(nick[j+1]!='\0') //this can be the case when identmode regnick is used.
					{
						passhost=NULL;
					}
					else
						passhost=nick[j+1];
					break;
				}
			}
			servername=Cexplode_getnext(userploder);
			MAZZERT(NULL!=servername,"ImpossibleHappened again");
			chantmp=Cexplode_getnext(userploder);
			while(NULL!=chantmp)
			{
				if(NULL==mbot_ll_add(channels,chantmp))
				{
					PPRINT("mbot_ll_add FAILED!");
					return -1;
				}
			}
			switch(IdentMode)
			{
				case EuserIdentMode_Hostmask:
				case EuserIdentMode_RegNick:
					nick[strlen(nick)]='@'; //switch it back to nick@host
					THEPERMANENTUSERSTORAGE.useraddby_host(nick,cfg->id,server,channel,NICKHOST_ADDER_SYSTEM);
					break;
				case EuserIdentMode_Passwd:
					THEPERMANENTUSERSTORAGE.useraddby_passwd(nick,cfg->id,server,channel,passhost,NICKHOST_ADDER_SYSTEM);
					break;
				default:
					MAZZERT(0,"Nooooo Not AGaINnNnnNnnnnn!!!");
					break;
			}
			//TODO: ADD MEMORY RELEASING.
		}
		cfg=res->getNextCfgs(res);
    }
    cfgparser->uninit(&cfgparser);
    ((SparserResult *)res)->free((SparserResult **)&res);
	//SetConfig(CALLBACK_EVENT_FILE,"valueReadFromGenCfgFile",strlen(valueReadFromGenCfgFile));
	//WPRINT("Just a dummy get_basic_configs() implementation!");
	return 0;
}

}
*/
void clear_event_configs(void)
{
	WPRINT("clear_event_configs() has just dummy implementation!");
}
int get_event_configs(void)
{
	char *testfilename;
	size_t testfilename_size;
    Sparser *fileparser;
    EparserState state;
    SConfigResult_Callback *cfgres;
    ScallbackConf *callbackdetails;

	if(NULL==(testfilename=GetConfig(E_callback_event_file,&testfilename_size)))
	{
		testfilename=CFG_FILE_DEFAULT;
		testfilename_size=strlen(testfilename)+1;
	}
    fileparser=ParserInit(EparserType_CfgCallback);
    MAZZERT(NULL!=fileparser,"ParserInit FAILED");
    DPRINT("Parser Inited");
    fileparser->feed(fileparser,testfilename,testfilename_size,&state);
    MAZZERT(state==EparserState_ResultReady,"Bad state returned by parser - expected EparserState_ResultReady");
    DPRINT("Parser Fed");
    cfgres=(SConfigResult_Callback *)fileparser->get_result(fileparser,&state);
    MAZZERT(state==EparserState_ParserEmpty,"Bad state returned by parser - expected EparserState_ParserEmpty");
    DPRINT("File Parsed");
    while(NULL!=(callbackdetails=(ScallbackConf *)cfgres->eventGet((SparserResult *)cfgres)))
    {
        //int i;
        DPRINT("Details for id%d:",callbackdetails->id);
        DPRINT("type=%d\nuserlevel=%c%d\noriginatorchan=\"%s\"\ntriggerstring=\"%s\"\n",
            callbackdetails->type,
			(callbackdetails->userlevel_direction!=EIRCuserLevelDirection_Larger)?
				(
				 	(callbackdetails->userlevel_direction==EIRCuserLevelDirection_Equal)?'=':
					'<'
				)
				:
				'>',
			callbackdetails->userlevel,
            callbackdetails->originator_chan,
                (NULL==callbackdetails->triggerstring || callbackdetails->type!=EMbotcallbackEventType_LocalTxtEvent || callbackdetails->type!=EMbotcallbackEventType_WebTxtEvent )?
                    "NULL":callbackdetails->triggerstring
        );
		
//        for(i=0;i<callbackdetails->amntOfLocations;i++)
//        {
		if
		(
			(
				callbackdetails->type!=EMbotcallbackEventType_LocalTxtEvent || 
				callbackdetails->type!=EMbotcallbackEventType_WebTxtEvent 
			)
			&&
			callbackdetails->locationoftxtevent!=0
		)
            printf("location %d",callbackdetails->locationoftxtevent);
		DPRINT("Blindly adding event - errorchecking, wildcardparsing etc should be done!?!");
		if(0>add_event(callbackdetails))
		{
			EPRINT("Adding event id %d FAILED!",callbackdetails->id);
		}
    }
    ((SparserResult *)cfgres)->free((SparserResult **)&cfgres);
    fileparser->uninit(&fileparser);
	DPRINT("Events Added!");
    return 0;
}


//TODO: Do basic configuration file reading. Use BasicParser

/*
 * int main(void)
{
	char *filename="testifilu2";
	return parse_def_texts(filename);
}
*/

//XXX: This functionality is moved to event storage struct.
//It is called via server struct, which calls channels containing events.
//
//TODO: Rewrite. There's no more one global event storage, so this should be done / channel basis.
//And after chans have been initialized!
//TODO: Change this to use BasicParser. (To keep things similar - leave this commented, so if there will be performance issues, we cann fall back.)
//Parses the default events for a channel. If error occurs, returns -1
//If end of file is reached returns 1
//If channel is successfully parsed, and no eof found, returns 0
/*
 * int parse_def_texts(FILE *readfile)
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
/*	if(NULL==readfile)
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
				printf("Error Error Error! Malformed default cb reply in config file!\n");
			}
			else
			{
				int retval;
				printf("Adding scanned %d, %d:%s\n",scanned,eventId,reply);
				retval=localevent_add_default_event_string(eventId,reply);
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
