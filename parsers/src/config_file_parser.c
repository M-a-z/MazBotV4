/* ********************************************************************
 *
 * @file config_file_parser.c
 * @brief functions to parse config files.
 *
 *
 * -Revision History:
 *
 *  - 0.0.4 17.08.2009/Maz  Added uninit - bad feeling that memory management is 
 *                          a total mess..
 *  - 0.0.3 10.08.2009/Maz  Changed event cfg file separators. (Bug 0000003)
 *  - 0.0.2 09.08.2009/Maz  Fixed NULL termination of some strigs,
 *  						Fixed returning of uninitialized value.
 *  						Fixed indexing bug in result amount calculation.
 *  - 0.0.1 05.08.2009/Maz  First (non compiling) draft
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

#include <callbackhandler.h>
#include <stdio.h>
#include "parser.h"
#include "config_file_parser.h"
#include <generic.h>
#include <config.h>
#include <configNumbers.h>

static EparserRetVal FileParserFeed(Sparser *_this_, char *data, size_t datasize, EparserState *state);
static SparserResult * CfgCbGetResult(Sparser *_this_, EparserState *state);
static ScallbackConf * CfgCbRes_eventGet(SparserResult *_this_);
static void CfgCbRes_free(SparserResult **);

void CfgCbFileParserUninit(Sparser **_this_)
{
    SFILEparser *_this;
    if(NULL==_this_ || NULL==*_this_)
    {
        EPRINTC(PrintComp_Parser,"CfgCbFileParserUninit(): NULL param given!");
    }
    else
    {
        _this=(SFILEparser *)*_this_;
        free(_this->result);
        free(_this);
        _this=NULL;
    }
}

SFILEparser *CfgCbFileParserInit(void)
{
    //Allocate needes space:
    SFILEparser *_this;
    char *separators[]={CFG_EVENT_FILE_SEPARATOR1,CFG_EVENT_FILE_SEPARATOR2};
    int amntOfSeps=2;
	char *ownId;
	size_t ownIdSize;
    _this=malloc(sizeof(SFILEparser));
    if(NULL==_this)
    {
        EPRINTC(PrintComp_Parser,"File parser alloc FAILED!");
        return NULL;
    }
    memset(_this,0,sizeof(SFILEparser));
	_this->result=malloc(sizeof(SConfigResult_Callback));
	if(NULL==_this->result)
	{
		EPRINTC(PrintComp_Parser,"File parser alloc FAILED!");
		free(_this);
        return NULL;
    }

    //initialize struct:
	memset(_this->result,0,sizeof(SConfigResult_Callback));
    memset(_this->datatoparse,0,FILE_DATA_BUF_LEN+1);
    _this->parser.type=EparserType_CfgCallback;

//    config=getconfig(OWN_ID);
    //TODO: Write config service which reads configs from file, and then executes a specified callback and/or can be queried for configs.
    ownId=GetConfig(E_own_id,&ownIdSize);
    if(NULL==ownId)
    {
        //hardcoded defaults:
        ownIdSize=19;
        ownId=malloc(ownIdSize);
        memcpy(ownId,"MazBotV4@127.0.0.1",ownIdSize);
    }

    memcpy(_this->parser.own_id,ownId,ownIdSize);
    _this->parser.feed=&FileParserFeed;
    _this->parser.get_result=&CfgCbGetResult;
    _this->parser.uninit=&CfgCbFileParserUninit;
    _this->splitter=SplitterInit(separators,amntOfSeps);
    if(NULL==_this->splitter)
    {
        PPRINTC(PrintComp_Parser,"Could not init cfg file splitter!");
		free(_this->result);
		free(_this);
        return NULL;
    }
    _this->datastart=0; //May be this is not needed
    _this->dataend=0;   //May be this is not needed either :)
    _this->parser.state=EparserState_Inited;
    return _this;
}










static ScallbackConf * CfgCbRes_eventGet(SparserResult *_this_)
{
	//XXX: Do result typecheck!
	SConfigResult_Callback *_this=(SConfigResult_Callback *)_this_;
	ScallbackConf *ret;
	ret=malloc(sizeof(ScallbackConf));
	if(NULL==_this)
	{
		WPRINTC(PrintComp_Parser,"NULL conf given to CfgCbRes_eventGet()");
		return NULL;
	}
	printf("Gerring callback results, amnt=%d, iterator=%d",_this->amntOfCallbacks,_this->iterator);
	if(_this->iterator<_this->amntOfCallbacks)
	{
		_this->iterator++;
		memcpy(ret,&_this->callbackConfigs[_this->iterator-1],sizeof(ScallbackConf));
		return ret;
	}
	else
	{
		WPRINTC(PrintComp_Parser,"New callbacCfg result requested, but no more results available (ant=%d,iterator=%d)",_this->amntOfCallbacks,_this->iterator);
	}
	return NULL;
}

static void CfgCbRes_free(SparserResult **_this_)
{
	//XXX: Do result typecheck!
	int i;
	SConfigResult_Callback *_this;
	if(_this_!=NULL)
		_this=*(SConfigResult_Callback **)_this_;	
	else
	{
		WPRINTC(PrintComp_Parser,"Null (callback config)result given to free???");
	}
	for(i=0;i<_this->amntOfCallbacks;i++)
	{
		if(NULL!=_this->callbackConfigs[i].originator_chan)
			free(_this->callbackConfigs[i].originator_chan);
		if
        ( 
            ( 
                _this->callbackConfigs[i].type==EMbotcallbackEventType_LocalTxtEvent 
                || 
                _this->callbackConfigs[i].type==EMbotcallbackEventType_WebTxtEvent
            ) 
            && 
            NULL!=_this->callbackConfigs[i].triggerstring
        ) 
			free(_this->callbackConfigs[i].triggerstring);
/*		if(NULL!=_this->callbackConfigs[i].locationoftxtevent)
			free(_this->callbackConfigs[i].locationoftxtevent);
*/
	}
	free(_this->callbackConfigs);
	free(_this);
	_this_=NULL;
}






//TODO: Change this to SparserResult *, but still return the correct fileparser result. User needs to cast SparserResult * to correct type (accrding to init type of parser)
//SConfigResult_Callback *
static SparserResult * CfgCbGetResult(Sparser *_this_, EparserState *state)
{
    SConfigResult_Callback *ret;
    SFILEparser *_this=(SFILEparser *)_this_;
    int id;
	int userlevel;
    int cfgindex=0;
    char *tmp;
    char *strtolRet;
//	int locindex;
	CexplodeStrings *configline;
//TODO: add typecheck
    if(NULL==_this->result || _this->splitter == NULL || _this->splitter->amntoflines<=0)
    {
        //ERROR, no results!
    }
    //Fill result struct, plan which params will be in which lines etc.
    ret=malloc(sizeof(SConfigResult_Callback));
    if(NULL==ret)
    {
        PPRINTC(PrintComp_Parser,"Malloc FAILED at CfgCbGetResult()!");
        *state=EparserState_FatalError;
        return NULL;
    }
	ret->fileparRes.gen.free=&CfgCbRes_free;
	ret->fileparRes.gen.parserResultType=EparserResultType_CfgCallback;
	ret->eventGet=&CfgCbRes_eventGet;
    ret->amntOfCallbacks=0;
    ret->callbackConfigs=malloc(_this->splitter->amntoflines*sizeof(ScallbackConf));
    if(NULL==ret->callbackConfigs)
    {
        PPRINTC(PrintComp_Parser,"Malloc FAILED at CfgCbGetResult()!");
        free(ret);
        *state=EparserState_FatalError;
        return NULL;
    }
//    ret->cfggen.cfgtype = EFileType_CbConf;
    ret->iterator=0;
	ret->amntOfCallbacks=0;
    while
	( 
		(
		 	configline=
		 	_this->result->splitterresult->get
			(
			 	_this->result->splitterresult
			)
		)
		!=NULL
	)
    {
//		SSplitterResult *res;
		if(NULL==configline)
		{
			WPRINTC(PrintComp_Parser,"ConfigParser did something odd... O_o");
			continue;
		}
        tmp=Cexplode_getNth(CFG_ID_COLUMN,configline); //id is first in row => CFG_ID_COLUMN == 1
        MAZZERT(NULL!=tmp,"YaY! Splitter should have ignored this!!");
        mbot_lrtrim(tmp,' ');
		if('#'==tmp[0])
		{
			DPRINTC(PrintComp_Parser,"Comment line detected (%s...)",configline->strings[0]);
			continue;
		}
        id=strtol(tmp,&strtolRet,10);
        if(tmp+strlen(tmp)!=strtolRet || ('0'>tmp[0] || '9'<tmp[0]))
        {
            perror("strtol FAILED in CfgCbGetResult()");
            WPRINTC(PrintComp_Parser,"Config file seems to have bad id numbers! (%s)",tmp);
            continue;
        }
        ret->callbackConfigs[cfgindex].id=id;
        tmp=Cexplode_getNth(CFG_EVTYPE_COLUMN,configline);
        if(NULL==tmp)
        {
            EPRINTC(PrintComp_Parser,"Invalid event type found from cfg file (%s)",tmp);
            continue;
        }
        mbot_lrtrim(tmp,' ');
            
        if(!strcmp(tmp,"EMbotcallbackEventType_LocalTxtEvent"))
        {
                //YaY! Valid type foo!
            ret->callbackConfigs[cfgindex].type=EMbotcallbackEventType_LocalTxtEvent;
        }
        else if(!strcmp(tmp,"EMbotcallbackEventType_WebTxtEvent"))
        {
                //YaY! Valid type bar!
            ret->callbackConfigs[cfgindex].type=EMbotcallbackEventType_WebTxtEvent;
        }
        else if(!strcmp(tmp,"EMbotcallbackEventType_LocalJoin"))
        {
                //YaY! Valid type bar!
            ret->callbackConfigs[cfgindex].type=EMbotcallbackEventType_LocalJoin;
        }
        else if(!strcmp(tmp,"EMbotcallbackEventType_WebJoin"))
        {
                //YaY! Valid type bar!
            ret->callbackConfigs[cfgindex].type=EMbotcallbackEventType_WebJoin;
        }
        else if(!strcmp(tmp,"EMbotcallbackEventType_LocalPart"))
        {
                //YaY! Valid type bar!
            ret->callbackConfigs[cfgindex].type=EMbotcallbackEventType_LocalPart;
        }
        else if(!strcmp(tmp,"EMbotcallbackEventType_WebPart"))
        {
                //YaY! Valid type bar!
            ret->callbackConfigs[cfgindex].type=EMbotcallbackEventType_WebPart;
        }
        else
        {
            EPRINTC(PrintComp_Parser,"Invalid callback Type deteceted (%s) in cfg file! (id = %d)",tmp,ret->callbackConfigs[cfgindex].id);
            continue;
        }
		tmp=Cexplode_getNth(CFG_USERLEVEL_COLUMN,configline);
        if(NULL==tmp)
        {
            EPRINTC(PrintComp_Parser,"No userlevel set for event %d in cfg file!",ret->callbackConfigs[cfgindex].id);
            continue;
        }
        mbot_lrtrim(tmp,' ');
		if(strlen(tmp)!=2 || (tmp[0]!='='&&tmp[0]!='>'&&tmp[0]!='<'))
		{
			EPRINTC(PrintComp_Parser,"Malicious userlevel in cfg file for event %d!",ret->callbackConfigs[cfgindex].id);
			continue;
		}
		userlevel=strtol(&tmp[1],&strtolRet,10);
		if(&tmp[2]!=strtolRet||userlevel<0 || userlevel>=EIRCuserLevel_NmbrOf)
		{
			EPRINTC(PrintComp_Parser,"Incorrect userlevel numeral given for event %d in cfg file!",ret->callbackConfigs[cfgindex].id);
			continue;
		}
		ret->callbackConfigs[cfgindex].userlevel=userlevel;
		switch((int)tmp[0])
		{
			case (int)'=':
				ret->callbackConfigs[cfgindex].userlevel_direction=EIRCuserLevelDirection_Equal;
				break;
			case (int)'<':
				ret->callbackConfigs[cfgindex].userlevel_direction=EIRCuserLevelDirection_Smaller;
				break;
			case (int)'>':
				ret->callbackConfigs[cfgindex].userlevel_direction=EIRCuserLevelDirection_Larger;
				break;
			default:
				MAZZERT(0,"Something is really bad wrong, should not be here as Kari says!");
		}
        tmp=Cexplode_getNth(CFG_ORIGINATOR_COLUMN,configline); 
        if(NULL==tmp)
        {
            EPRINTC(PrintComp_Parser,"No Originator set for event %d in cfg file!",ret->callbackConfigs[cfgindex].id);
            continue;
        }
        mbot_lrtrim(tmp,' ');
        ret->callbackConfigs[cfgindex].originator_chan=malloc(strlen(tmp)+1);
        if(NULL==ret->callbackConfigs[cfgindex].originator_chan)
        {
            PPRINTC(PrintComp_Parser,"Error while handling configs! - malloc() failed!");
            //free? No need - bot is about to exit
//            return badretval;
        	*state=EparserState_FatalError;
		    return NULL;
        }
        memcpy(ret->callbackConfigs[cfgindex].originator_chan,tmp,strlen(tmp)+1);
//		ret->callbackConfigs[cfgindex].originator[strlen(tmp)]='\0';
        if(NULL!=(tmp=Cexplode_getnext(configline)))
        {
            mbot_lrtrim(tmp,' ');
            ret->callbackConfigs[cfgindex].triggerstring=malloc(strlen(tmp)+1);
            if(NULL==ret->callbackConfigs[cfgindex].triggerstring)
            {
                PPRINTC(PrintComp_Parser,"Error while handling configs! - malloc() failed!");
                //free? No need - bot is about to exit
//                return badretval;
        		*state=EparserState_FatalError;
		        return NULL;

            }
            memcpy(ret->callbackConfigs[cfgindex].triggerstring,tmp,strlen(tmp)+1);
        }
        else if(ret->callbackConfigs[cfgindex].type < EMbotcallbackEventType_TEXTEVENTEND)
        {
            EPRINTC(PrintComp_Parser,"Bad configs, no triggerstring for textevent given (id = %d)",ret->callbackConfigs[cfgindex].id);
            continue;
        }
        if(NULL!=(tmp=Cexplode_getnext(configline)))
        {
//XXX: There should be no more than one location specification needed => simplifying this
//			int numoflocs=0;
			char *loc=tmp;
//			CexplodeStrings locxploder;
            mbot_lrtrim(tmp,' ');
//			numoflocs=Cexplode(tmp,";",&locxploder);
//			ret->callbackConfigs[cfgindex].locationoftxtevent=malloc(numoflocs*sizeof(EMbotEventLocation));
/*			if(NULL==ret->callbackConfigs[cfgindex].locationoftxtevent)
			{
				PPRINTC(PrintComp_Parser,"Malloc Failed while handling configs");
//				return badretval;
        		*state=EparserState_FatalError;
		        return NULL;

			}
			locindex=0;
			ret->callbackConfigs[cfgindex].amntOfLocations=0;
			while(NULL!=(loc=Cexplode_getnext(&locxploder)))
			{
*/
				if(!strcmp(loc,"EMbotEventLocation_All"))
				{
//					ret->callbackConfigs[cfgindex].locationoftxtevent[locindex]=EMbotEventLocation_All;
					ret->callbackConfigs[cfgindex].locationoftxtevent=EMbotEventLocation_All;
				}
				else if(!strcmp(loc,"EMbotEventLocation_PMD"))
				{
					ret->callbackConfigs[cfgindex].locationoftxtevent=EMbotEventLocation_PMD;
//					ret->callbackConfigs[cfgindex].locationoftxtevent[locindex]=EMbotEventLocation_PMD;
				}
				else if(!strcmp(loc,"EMbotEventLocation_CD"))
				{
					ret->callbackConfigs[cfgindex].locationoftxtevent=EMbotEventLocation_CD;
//					ret->callbackConfigs[cfgindex].locationoftxtevent[locindex]=EMbotEventLocation_CD;
				}
				else if(!strcmp(loc,"EMbotEventLocation_Dcc"))
				{
//					ret->callbackConfigs[cfgindex].locationoftxtevent[locindex]=EMbotEventLocation_Dcc;
					ret->callbackConfigs[cfgindex].locationoftxtevent=EMbotEventLocation_Dcc;
				}
				else if(!strcmp(loc,"EMbotEventLocation_CPM"))
				{
					ret->callbackConfigs[cfgindex].locationoftxtevent=EMbotEventLocation_CPM;
//					ret->callbackConfigs[cfgindex].locationoftxtevent[locindex]=EMbotEventLocation_CPM;
				}
				else if(!strcmp(loc,"EMbotEventLocation_Chan"))
				{
					ret->callbackConfigs[cfgindex].locationoftxtevent=EMbotEventLocation_Chan;
//					ret->callbackConfigs[cfgindex].locationoftxtevent[locindex]=EMbotEventLocation_Chan;
				}
				else if(!strcmp(loc,"EMbotEventLocation_Privmsg"))
				{
//					ret->callbackConfigs[cfgindex].locationoftxtevent[locindex]=EMbotEventLocation_Privmsg;
					ret->callbackConfigs[cfgindex].locationoftxtevent=EMbotEventLocation_Privmsg;
				}
				else
				{
            		EPRINTC(PrintComp_Parser,"Invalid callback Type deteceted (%s) in cfg file! (id = %d)",tmp,ret->callbackConfigs[cfgindex].id);
					continue;
				}
/*				locindex++;
				ret->callbackConfigs[cfgindex].amntOfLocations=locindex;
			}
			Cexplode_free(locxploder);
*/
		}
		cfgindex++;
        ret->amntOfCallbacks=cfgindex;
		//res=&(_this->result->splitterresult);
    }
	_this->result->splitterresult->free(&(_this->result->splitterresult));
	*state=EparserState_ParserEmpty;
    return (SparserResult *)ret;
}
static EparserRetVal FileParserFeed(Sparser *_this_, char *data, size_t datasize, EparserState *state)
{
    SFILEparser *_this=(SFILEparser *)_this_;
    int charctr;
    _this->cfgfile =fopen(data,"r");
    if(NULL==_this->cfgfile)
    {
        perror("Config file open FAILED!");
		PPRINTC(PrintComp_Parser,"Failed to open cfg file %s",data);
//        fclose(_this->cfgfile);
        _this->cfgfile=NULL;
        return EparserRetVal_InvalidParam;
    }
    //read file to _this->datatoparse
    for(charctr=0;charctr<FILE_DATA_BUF_LEN && EOF!=(_this->datatoparse[charctr]=fgetc(_this->cfgfile));charctr++);
    _this->datatoparse[charctr]='\0';
    if(0>_this->splitter->feed(_this->splitter,_this->datatoparse))
    {
        EPRINTC(PrintComp_Parser,"File Feeding to splitter FAILED!");
        _this->splitter->uninit(&_this->splitter);
        _this->parser.state=*state=EparserState_FatalError;
        fclose(_this->cfgfile);
        _this->cfgfile=NULL;

        //Uninit?
        return EparserRetVal_InternalError;
    }
    _this->result->splitterresult=_this->splitter->split(_this->splitter);
    if(NULL==_this->result)
    {
        *state=EparserState_Inited;
        PPRINTC(PrintComp_Parser,"No suitable configs found");
        _this->parser.state=EparserState_Inited;
        //return values to initial ones..
        //TODO: Re init splitter if it cannot be reused
        return EparserRetVal_InvalidParam;
    }
    _this->parser.state=*state=(_this->result->splitterresult->resamnt>0)?EparserState_ResultReady:EparserState_ParserEmpty;
    fclose(_this->cfgfile);
    _this->cfgfile=NULL;
    return EparserRetVal_Ok;
}


