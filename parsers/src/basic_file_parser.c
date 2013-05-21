/* ********************************************************************
 *
 * @file basic_file_parser.c
 * @brief functions to parse config files.
 *
 *
 * -Revision History:
 *
 *  - 0.0.2 20.08.2009/Maz  Ignored comment lines
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

//#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <parser.h>
#include <basic_file_parser.h>
static EparserRetVal BasicParserFeed(Sparser *_this_,char *data,size_t datasize, EparserState *state);
static SbasicConfig *BasicParserResGetNextCfg(SbasicParserResult *_this);
static SparserResult * BasicParserGetResult(Sparser *_this_, EparserState *state);
static void BasicParserResultFree(SparserResult **_this_);
static void BasicParserFree(Sparser **_this_);


SbasicFileParser * BasicParserInit()
{
    SbasicFileParser *_this=malloc(sizeof(SbasicFileParser));
    if(NULL==_this)
    {
        PPRINTC(PrintComp_Parser,"Malloc Failed at BasicParserInit()!");
        return NULL;
    }
    memset(_this,0,sizeof(SbasicFileParser));
    _this->gen.type=EparserType_CfgBasic;
    _this->gen.state=EparserState_Inited;
    _this->gen.uninit=&BasicParserFree;
    _this->gen.feed=&BasicParserFeed;
    _this->gen.get_result=&BasicParserGetResult;
   return _this; 
}
static void BasicParserFree(Sparser **_this_)
{
    SbasicFileParser *_this;
    if(NULL==_this_||NULL==*_this_)
    {
        PPRINTC(PrintComp_Parser,"NULL ptr given to BasicParserFree()");
        return;
    }
    _this=*(SbasicFileParser **)_this_;
    if(NULL!=_this->result)
    {
        //Result container should be copied, so it should be Ok to free this here.
        free(_this->result);
    }
    free(_this);
    _this=NULL;
}
static void BasicParserResultFree(SparserResult **_this_)
{
    SbasicParserResult *_this;
    if(NULL==_this_ || NULL==*_this_)
    {
        EPRINTC(PrintComp_Parser,"NULL detected at BasicParserResultFree()");
        return;
    }
    _this=*(SbasicParserResult **)_this_;
    if(NULL!=_this->cfgs)
    {
/*  XXX: ponder. Okay. this may cause problems, but I do not want to copy the actual results each time results are obtained.
 *  So I'll leave it for user to free data. ALthough it also gives user access to data used in here... Someday it may be that user can cause
 *  this to crash by freeing data...
 *
 * SbasicConfig *tmp;
        tmp=mbot_ll_...
        while(NULL!=tmp)
        {
            free(tmp->data);
            tmp->datasize=0;....
*/
        mbot_ll_destroy(&(_this->cfgs));
    }
    free(_this);
    _this=NULL;
}
static SbasicConfig *BasicParserResGetNextCfg(SbasicParserResult *_this)
{
    SbasicConfig *ret;
    mbot_linkedList *tmp;
    tmp=mbot_ll_get_next(_this->cfgs);
    if(NULL==tmp)
    {
        DPRINTC(PrintComp_Parser,"BasicParserResGetNextCfg(): Last cfg already returned??");
        return NULL;
    }
    _this->cfgs=tmp;
    ret=_this->cfgs->data;  //We do not copy data, which is better for performance, but bad for handling the memory... 
    //Who owns which memory block, and who should free what??? This is going to be a huge issue with this bot - sooner or later.
    return ret;
}
static SparserResult * BasicParserGetResult(Sparser *_this_, EparserState *state)
{
    SbasicFileParser *_this=(SbasicFileParser*)_this_;
    SbasicParserResult *ret;

    if(NULL==_this || NULL==state)
    {
        EPRINTC(PrintComp_Parser,"NULL item given to BasicParserGetResult()");
        *state=EparserState_FatalError;
        return NULL;
    }
    if(NULL==_this->result)
    {
        DPRINTC(PrintComp_Parser,"BasicParserResultGet() called, but no results stored!");
        *state=EparserState_ParserEmpty;
        return NULL;
    }
    ret=malloc(sizeof(SbasicParserResult));
    if(NULL==ret)
    {
        PPRINTC(PrintComp_Parser,"Malloc failed at BasicParserResultGet()");
        *state=EparserState_FatalError;
        return NULL;
    }
    //XXX
    //We do not allocate actual results here, just the container.
    //So when parser is de-inited, container can be freed, but not the actual data.
    //This is likely to cause memory leaks, but it also sppeds up things.
    //All in all, this is symptom of bad planning, and more or less all parsers suffer from this.
    memcpy(ret,_this->result,sizeof(SbasicParserResult));
    return (SparserResult *)ret;
}
//XXX: Is this reusable?
//typedef EparserRetVal (*ParserDataFeedF)(struct Sparser *_this,char *data,size_t datasize, EparserState *state);
static EparserRetVal BasicParserFeed(Sparser *_this_,char *data,size_t datasize, EparserState *state)
{
    FILE *readfile;
    int ok=1;
    SbasicParserResult *result;
    SbasicFileParser *_this=(SbasicFileParser *)_this_;

    result=malloc(sizeof(SbasicParserResult));
    if(NULL==result)
    {
        PPRINTC(PrintComp_Parser,"Malloc Failed at BasicParserFeed()");
        *state=EparserState_FatalError;
        return EparserRetVal_InternalError;
    }
    memset(result,0,sizeof(SbasicParserResult));
    result->cfgs=mbot_ll_init();
    if(NULL==result->cfgs)
    {
        PPRINTC(PrintComp_Parser,"Linked List creation FAILED at BasicParserFeed() (probs malloc failure!)");
        free(result);
        *state=EparserState_FatalError;
        return EparserRetVal_InternalError;
    }
    result->gen.parserResultType=EparserResultType_CfgBasic;
    result->gen.free=&BasicParserResultFree;
    result->getNextCfgs=BasicParserResGetNextCfg;
    readfile=fopen(data,"r");
    if(NULL==readfile)
    {
        perror("file opening failed!\n");
        EPRINTC(PrintComp_Parser,"Bad config file %s",data);
        mbot_ll_destroy(&(result->cfgs));
        free(result);
        *state=EparserState_FatalError;
        return EparserRetVal_InvalidParam;
    }
    else
    {
        while(ok&&!feof(readfile))
        {
            int scanned;
            char *reply;
            int eventId;
            scanned=fscanf(readfile,"#%a[^\n]\n",&reply);
            //Ignore comment lines
            if(scanned>0)
            {
                free(reply);
				continue;
            }
            if(scanned<0)
            {
                DPRINTC(PrintComp_Parser,"fscanf() returned error while reading file %s (%s)",data,strerror(errno));
                ok=0;
                continue;
            }
			scanned=fscanf(readfile,"%d:%a[^\n]\n",&eventId,&reply);
            if(scanned<0)
            {
                ok=0;
            }
            else if(scanned!=2 && scanned != 0)
            {
                printf("Error Error Error! Malformed default cb reply file %s\n",data);
            }
            else if(scanned==0)
            {
                //Oh yay! This line was neither a comment, nor a dataline => scanf does not remove it from buffer => read it and warn.
                char *tmp=NULL;
                size_t tmp2=0;
                int tmpret;
                if(-1==(tmpret=getline(&tmp,&tmp2,readfile)))
                {
                    EPRINTC(PrintComp_Parser,"getline spotted an error while reading %s! (%s)",data,strerror(errno));
                    return EparserRetVal_InternalError;
                }
                else
                {
                    WPRINTC(PrintComp_Parser,"line was not recognized as comment or config:\n%s",tmp);
                    free(tmp);
//                    free(tmp2);
                }
            }
            else
            {
            //    int retval;
                SbasicConfig *cfg;
                cfg=malloc(sizeof(SbasicConfig));
                if(NULL==cfg)
                {
                    PPRINTC(PrintComp_Parser,"Malloc Failed at BasicParserFeed()!");
                    //XXX: See if mbot_ll_destroy calls free to stored datas (I believe it does not).
                    //Actually, since this is propably handled as fatal error and bot is about to exit, no proper freeing is demanded.
                    mbot_ll_destroy(&(result->cfgs));
                    free(result);
                    *state=EparserState_FatalError;
                    return EparserRetVal_InvalidParam;
                }
                cfg->datasize=strlen(reply)+1;
                cfg->cfgid=eventId;
                cfg->data=reply;
                mbot_ll_add(result->cfgs,cfg);
            }
        }
    }
    _this->result=result;
    //XXX: Should we investigate if results were obtained, and if no results were found from file, use
    //EparserState_ParserEmpty
    *state=EparserState_ResultReady;
    return EparserRetVal_Ok;
}


