/* ********************************************************************
 *
 * @file irc_protocol_parser.c
 * @brief functions to parse IRC message streams into [prefix] command [params] set, and extract sender info
 *
 *
 * -Revision History:
 *
 *  - 0.0.7 18.08.2009/Maz  Simplified(?) memory usage model, and hopefully eliminated some leaks.
 *  - 0.0.6 17.08.2009/Maz  Added uninit -> bad feeling that memory managing
 *                          is quite ...
 *  - 0.0.5 29.07.2009/Maz  Fixed a severe bug which caused get_result to 
 *  						occasionally (incorrectly) return NULL!
 *  - 0.0.4 24.07.2009/Maz  Changed IF for results handling.
 *  - 0.0.3 24.07.2009/Maz  At least looks like this is working now.
 *  - 0.0.2 22.07.2009/Maz  Still just a draft :)
 *  - 0.0.1 20.07.2009/Maz  First draft
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
#include <config.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "parser.h"
#include "irc_protocol_parser.h"
#include "irc_protocol_definitions.h"
#include <configNumbers.h>


static SIRCparserResult * InitResult();
static char *SIRCparserResultGetParam(SIRCparserResult *result, int paramno);
static char *SIRCparserResultGetPrefix(SIRCparserResult *result);
static char *SIRCparserResultGetCmd(SIRCparserResult *result);

// Used to feed data to parser. State of parser should be changed to ResultReady when fed enough. 
// After this feeding is forbidden untill data is parsed and the results have been obtained.
static EparserRetVal IRCparserfeed(struct Sparser *_this_,char *data,size_t datasize, EparserState *state);
static void IRCfreeresult(SparserResult **result);
//static void IRCfreeresult(Sparser *_this, SparserResult **result);
static void IRCparserReset(SIRCparser *_this);
//After the parser is fed, we will here parse the fed lines - internal only, called in feed func.
static EparserRetVal IRCparse(SIRCparser *_this);
/* This can be called after the results have been parsed. Should be called untill the state changes to "Need More Data" */
/* Reurned results should be freed with IRCfreeresult() */
static SparserResult *IRCgetresults(struct Sparser *_this, EparserState *state);
int IRCpreparse(SIRCparser *_this);

//static EparserRetVal IRCparserfeed(struct Sparser *_this_,char *data,size_t datasize, EparserState *state)
static void IRCparserUninit(Sparser **_this_)
{
    //This was added afterwards, it is unplanned, bad sad and mad && errorprone. Results may use contents of this struct??
    //Freeing this and still using results may lead to death and destruction!
    //CexplodeStrings are not freed, because I *think* that those might have been directly used in results?
    //TODO: Investigate && improve!
    SIRCparser *_this;
    if(NULL==_this_ || NULL == *_this_)
    {
        EPRINTC(PrintComp_Parser,"IRCparserUninit(): NULL pointer given to uninit!");
    }
    else
    {
        _this=(SIRCparser *)*_this_;
        if(NULL!=_this->channel)
            free(_this->channel);
        if(NULL!=_this->msg)
            free(_this->msg);
        if(NULL!=_this->result)
            mbot_ll_destroy(&(_this->result));
        free(_this);
        _this=NULL;
    }
}
SIRCparser *IRCparserinit(void)
{
	void *ownId=NULL;
	size_t ownIdSize=0;
	SIRCparser *IRCparser=malloc(sizeof(SIRCparser));
	if(NULL==IRCparser)
	{
		PPRINTC(PrintComp_Parser,"Malloc failed at %s:%d, requested %u bytes",__FILE__,__LINE__,sizeof(SIRCparser));
		return NULL;
	}
	memset(IRCparser,0,sizeof(SIRCparser));
	IRCparser->parser.type=EparserType_Irc;
	//TODO: Write config service which reads configs from file, and then executes a specified callback and/or can be queried for configs.
	//ownId=GetConfig(E_own_id,&ownIdSize);
	if(NULL==ownId)
	{
		//hardcoded defaults:
		ownIdSize=19;
		ownId=malloc(ownIdSize);
		memcpy(ownId,"MazBotV4@127.0.0.1",ownIdSize);
	}
	memcpy(IRCparser->parser.own_id,ownId,ownIdSize);
    IRCparser->result=mbot_ll_init();
    if(NULL==IRCparser->result)
    {
        PPRINTC(PrintComp_Parser,"Failed to init irc parser!");
        return NULL;
    }
	IRCparser->parser.feed=&IRCparserfeed;
	IRCparser->parser.get_result=&IRCgetresults;
    IRCparser->parser.uninit=&IRCparserUninit;  
	IRCparser->parser.state=EparserState_Inited;
	return IRCparser;
}

static void IRCfreeresult(SparserResult **result)
{
	MAZZERT( !(NULL==result || NULL == *result) ,"Double free attempted on parser result");
	SIRCparserResult *_this=(SIRCparserResult *)*result;
	Cexplode_free(_this->parsed);
	if(NULL!=_this->raw_data)
		free(_this->raw_data);
	free(_this);
	*result=NULL;
}
static void IRCparserReset(SIRCparser *_this)
{
	_this->datastart=0;
	_this->dataend=0;
	_this->parser.state=EparserState_Inited;
}
static EparserRetVal IRCparserfeed(struct Sparser *_this_,char *data,size_t datasize, EparserState *state)
{
	EparserRetVal ret;
	int retval;
	SIRCparser *_this=(SIRCparser *)_this_;
	if(EparserState_ResultReady == _this->parser.state)
	{
		*state=EparserState_Full;
		return EparserRetVal_OverFeed;
	}
	if(datasize>IRC_MSG_MAX_LEN)
	{
		EPRINTC(PrintComp_Parser,"IRCParser chocked, datalen=%lu",(unsigned long int)datasize);
		IRCparserReset(_this);
		*state=EparserState_Choked;
		return EparserRetVal_InvalidParam;
	}
	if(EparserState_NeedMoreData == _this->parser.state || EparserState_Inited == _this->parser.state)
	{
		//NULL the result!
		//concatenate new data at the end of old (to _this->parser.datatoparse)
		if( ((_this->dataend+datasize)&IRC_DATA_BUF_LEN)< ((_this->dataend)&IRC_DATA_BUF_LEN))
		{
			//the message would go over the ring buffer boundary => copy stuff to the beginning
			memmove(&(_this->datatoparse[0]),&(_this->datatoparse[((_this->datastart)&IRC_DATA_BUF_LEN)]),_this->dataend-_this->datastart);
			_this->dataend=_this->dataend-_this->datastart;
			_this->datastart=0;
		}
		memcpy(&(_this->datatoparse[((_this->dataend)&IRC_DATA_BUF_LEN)]),data,datasize);
		_this->dataend+=datasize;
		//Possible off by one error is prevented by allocating buffer to size IRC_DATA_BUF_LEN+1
		_this->datatoparse[((_this->dataend)&IRC_DATA_BUF_LEN)+1]='\0';
	}
	if(0>(retval=IRCpreparse(_this)))
	{
		EPRINTC(PrintComp_Parser,"Preparse FAILED!!!");
		MAZZERT(0,"WTF???");
		//XXX:errorhandling...?
	}
	if(0==retval)
	{
		//need more...
		*state=EparserState_NeedMoreData;
        Cexplode_free(_this->exploded);
		return EparserRetVal_InsufficientFeed;
	}
	if(0<retval)
	{
		if(EparserRetVal_Ok!=(ret=IRCparse(_this)))
		{
			WPRINTC(PrintComp_Parser,"Parsing msg FAILED");
		}
	}
	*state = _this->parser.state;
	return ret;
}

static char *SIRCparserResultGetCmd(SIRCparserResult *result)
{
	int cmdloc=2;
	if(NULL==result || NULL==result->parsed.strings)
	{
		WPRINTC(PrintComp_Parser,"NULL ptr given to SIRCparserResultGetCmd");
		return NULL;
	}
	if(!result->hasprefix)
		cmdloc=1;

	if(result->parsed.amnt<cmdloc)
	{
		EPRINTC(PrintComp_Parser,"No command found from result struct! (at SIRCparserResultGetCmd)");
		return NULL;
	}
	return Cexplode_getNth(cmdloc,&(result->parsed));
}
static char *SIRCparserResultGetPrefix(SIRCparserResult *result)
{
	if(NULL==result || NULL==result->parsed.strings)
    {
        WPRINTC(PrintComp_Parser,"NULL ptr given to SIRCparserResultGetCmd");
        return NULL;
    }   
    if(!result->hasprefix)
	{
		WPRINTC(PrintComp_Parser,"prefix queried from msg with no prefix!");
		return NULL;
	}
	return Cexplode_getfirst(&(result->parsed));
}
static char *SIRCparserResultGetParam(SIRCparserResult *result, int paramno)
{
	int OtherButParams = 2; //prefix and command as default
    if(NULL==result || NULL==result->parsed.strings)
    {
        WPRINTC(PrintComp_Parser,"NULL ptr given to SIRCparserResultGetCmd");
        return NULL;
    }
	if(paramno>IRC_PROTO_PARAM_MAX_AMNT || paramno < 1)
	{
		WPRINTC(PrintComp_Parser,"param beyond max/min allowed params (min 1), (max 15 #RFC 1459, section-2.3.1#), requested %d",paramno);
	}   
    if(!result->hasprefix)
		OtherButParams = 1;
	return Cexplode_getNth(paramno+OtherButParams ,&(result->parsed));
}
static SIRCparserResult * InitResult()
{
	SIRCparserResult *result=malloc(sizeof(SIRCparserResult));
	if(NULL==result)
	{
		PPRINTC(PrintComp_Parser,"malloc returned NULL at %s:%d",__FILE__,__LINE__);
		return NULL;
	}
	memset(result,0,sizeof(SIRCparserResult));
	result->gen.parserResultType=PARSER_RESULT_IRC;
	result->gen.free=IRCfreeresult;
	result->getcmd=&SIRCparserResultGetCmd;
	result->getprefix=&SIRCparserResultGetPrefix;
	result->getparam=&SIRCparserResultGetParam;
	return result;
}
static EparserRetVal IRCparse(SIRCparser *_this)
{
	char *thisroundmsg=NULL;
	char *tmp;
	size_t paramlen;
    SIRCparserResult *res;    
//    int i=0;
	MAZZERT(NULL!=_this,"NULL ptr in IRC parse!!");
	MAZZERT(EparserState_ResultReady==_this->parser.state,"Invalid parser state!");
    while(NULL!=(tmp=Cexplode_getnext(&(_this->exploded))))
    {
		int failed=0;

       if(!Cexplode_nextexists(_this->exploded)   &&!Cexplode_sepwasatend(_this->exploded))
       {
           //Situation where only incomplete row is in feed data should be handled by preparse,
           //Hence we can here assume that there is at least on complete record to be parsed.
           //Actually, if some error has occurred in parsing first piece(s), then it may be there's no
           //valid results. Thus we should remove faulty pieces from _this->exploded, and 
           //check here if this is the first piece!
          //Last piece is incomplete => discard.
          //Move the dataptrs accordingly
		  free(thisroundmsg);
		  thisroundmsg=NULL;
          if(0==_this->exploded.index)
          {
                _this->parser.state=EparserState_NeedMoreData;
                Cexplode_free(_this->exploded);
                return EparserRetVal_InsufficientFeed;
          }
          _this->datastart+=Cexplode_getlentilllast(_this->exploded);
//          Cexplode_free(_this->exploded);
          break;
       }
        thisroundmsg=malloc(strlen(tmp)+1);
        if(NULL==thisroundmsg)
        {
            PPRINTC(PrintComp_Parser,"malloc FAILED at %s:%d",__FILE__,__LINE__);
            _this->parser.state=EparserState_FatalError;
            return EparserRetVal_InternalError;
        }
        memcpy(thisroundmsg,tmp,strlen(tmp)+1);
       res=InitResult();
       if(NULL==res)
       {
           PPRINTC(PrintComp_Parser,"Failed to allocate irc_parser result!");
            _this->parser.state=EparserState_FatalError;
			free(thisroundmsg);
			thisroundmsg=NULL;
           return EparserRetVal_InternalError;
       }

	   res->raw_data=malloc(strlen(tmp)+1);
	   if(NULL==res->raw_data)
	   {
		   PPRINTC(PrintComp_Parser,"malloc FAILED at %s:%d",__FILE__,__LINE__);
		   _this->parser.state=EparserState_FatalError;
		   return EparserRetVal_InternalError;
	   }
	   memcpy(res->raw_data,thisroundmsg,strlen(tmp)+1);

       if(IRC_PREFIX_IND_CHAR==thisroundmsg[0])
       {
           res->hasprefix=1;
       }
       Cexplode(thisroundmsg,IRC_SPACE_STR, &(res->parsed));
       tmp=Cexplode_getnext(&(res->parsed));
       if(NULL==tmp)
       {
            EPRINTC(PrintComp_Parser,"ParseError, msg=%s",thisroundmsg);       
            res->gen.free((SparserResult **)&res);
            free(Cexplode_removeCurrent(&(_this->exploded)));
			free(thisroundmsg);
			thisroundmsg=NULL;
            //Just discard this one, and try parsing next result.
            continue;
        }
        if(res->hasprefix)
        {
            if(IRC_PREFIX_MAX<strlen(tmp))
            {
                EPRINTC(PrintComp_Parser,"ParseError (prefix too long), msg=%s",thisroundmsg);
                res->gen.free((SparserResult **)&res);
                free(Cexplode_removeCurrent(&(_this->exploded)));
				free(thisroundmsg);
				thisroundmsg=NULL;
                continue;
            }
            tmp=Cexplode_getnext(&(res->parsed));
        }
        if(NULL==tmp||IRC_CMD_MAX<strlen(tmp))
        {
            EPRINTC(PrintComp_Parser,"ParseError (command NULL or too long), msg=%s",thisroundmsg);
            res->gen.free((SparserResult **)&res);
            free(Cexplode_removeCurrent(&(_this->exploded)));
			free(thisroundmsg);
			thisroundmsg=NULL;
            continue;
        }
        paramlen=0;
        while(NULL!=(tmp=Cexplode_getnext(&(res->parsed))))
        {
    		paramlen+=strlen(tmp);
            if(IRC_PARAMS_MAX<paramlen)
            {
                EPRINTC(PrintComp_Parser,"ParseError (params too long), msg=%s",thisroundmsg);
                res->gen.free((SparserResult **)&res);
                free(Cexplode_removeCurrent(&(_this->exploded)));
                failed=1;
				break;
            }
            if(res->hasprefix)
            {
                //parse user info from prefix
            }
        }
		if(failed)
		{
			free(thisroundmsg);
			thisroundmsg=NULL;
			continue;
		}
        _this->result=mbot_ll_add(_this->result,res);
//		DPRINTC(PrintComp_Parser,"Adding new result");
	    free(thisroundmsg);
		thisroundmsg=NULL;
    }
    Cexplode_free(_this->exploded);
    if(NULL==mbot_ll_get_first(_this->result))
    {
        _this->parser.state=EparserState_NeedMoreData;
        return EparserRetVal_InsufficientFeed;
    }
    _this->parser.state=EparserState_ResultReady;

	return EparserRetVal_Ok;
}
static SparserResult *IRCgetresults(Sparser *_this_, EparserState *state)
{
	//EparserRetVal returnv;
    SparserResult *ret;
	SIRCparser *_this=(SIRCparser *)_this_;
    mbot_linkedList *tmp;
    if(_this->parser.state!=EparserState_ResultReady)
    {
        WPRINTC(PrintComp_Parser,"IRCgetresults: All results already retrieved!");
        return NULL;
    }
	if(NULL==_this || NULL==state)
	{
		EPRINTC(PrintComp_Parser,"NULL parameters delivered to IRCgetresults");
		return NULL;
	}
    tmp=mbot_ll_get_first(_this->result);
    MAZZERT(NULL!=tmp,"NoooOooOOoOOOOooooOOoOOoOOOOOOOO!!!");
    ret=mbot_ll_dataGet(tmp);
    if(NULL==mbot_ll_get_next(tmp))
    {
        *state=EparserState_NeedMoreData;
        _this->parser.state=EparserState_NeedMoreData;
        _this->result=mbot_ll_head_get(tmp);
    }
    free(mbot_ll_release(tmp));
	return (SparserResult *)ret;
}

int IRCpreparse(SIRCparser *_this)
{
	int expAmnt;
	DPRINTC(PrintComp_Parser,"Preparsing: \"%s\"",&(_this->datatoparse[(_this->datastart&IRC_DATA_BUF_LEN)]));
	expAmnt=Cexplode(&(_this->datatoparse[(_this->datastart&IRC_DATA_BUF_LEN)]),IRC_MSG_SEPARATOR_STR,&(_this->exploded));
	if(expAmnt<0)
	{
		//TODO: Create Cexplode_error(ECexplodeRet ecode); function
		switch(expAmnt)
		{
			case ECexplodeRet_InternalFailure:
				EPRINTC(PrintComp_Parser,"Cexplode failed at %s:%d (InternalError)",__FILE__,__LINE__);
				return -1;
				break;
			case ECexplodeRet_InvalidParams:
				EPRINTC(PrintComp_Parser,"Cexplode failed at %s:%d (InvalidParams)",__FILE__,__LINE__);
				return -1;
				break;
		}
	}
//TODO: Check if Cexplode returned number of delimiters found, or number of strings exploded.
    else if( (expAmnt==1 && !Cexplode_sepwasatend(_this->exploded)) || expAmnt < 1)
    {
			//No complete commands feeded
			_this->parser.state = EparserState_NeedMoreData;
            expAmnt=0;

    }
    else
	{
		_this->parser.state = EparserState_ResultReady;
		if(Cexplode_sepwasatend(_this->exploded))
		{
			//One complete msg ready for retriewing
            _this->datastart=_this->dataend;
			// 
			// XXX:Do something else before return?
		}
		else
		{
            _this->datastart+=Cexplode_getlentilllast(_this->exploded);
            free(Cexplode_removeNth(CEXPLODE_LAST_ITEM,&(_this->exploded)));

		}
	}
	return expAmnt;
}




