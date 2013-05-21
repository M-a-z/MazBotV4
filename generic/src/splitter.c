/* ********************************************************************
 *
 * @file splitter.c
 * @brief Helper for parsing config files.
 * @todo Fix rewrite and so on. This is heavy as hell, 
 * and memory handling is utterly confusing
 *
 *
 * -Revision History:
 *
 *  - 0.0.4 04.08.2009/Maz  Fixed bad indexing and a few memory leaks.
 *  - 0.0.3 03.08.2009/Maz  Added splitter results handling
 *  - 0.0.2 02.08.2009/Maz  Seems to work (at least when not called recursilvely)
 *                          - leaks memory though. TODO: Specify way to get results.
 *  - 0.0.1 30.07.2009/Maz  First draft
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



#include "helpers.h"
#include "generic.h"
#include "splitter.h"
#include <unistd.h>
static void splitteResFree(SSplitterResult **_this_);
static SSplitterResult * SplitterSplit(SSplitter *_this);
static CexplodeStrings *splitteResGetNth(SSplitterResult *_this,int numberofres);
static CexplodeStrings *splitteResGet(SSplitterResult *_this);
static void SplitterUninit(SSplitter **_this_);
static int splitter_feed(SSplitter *_this,char *data);
static void helpprint(CexplodeStrings exp)
{
    int i;
    for(i=0;i<exp.amnt;i++)
    {
        DPRINTC(PrintComp_splitter,"Exp %d: \"%s\"",i,exp.strings[i]);
        //sleep(3);
    }
	fflush(stdout);
}
/*
 * TODO: Decide whether to use result_valid to something, or to get rid of it?
 */
static void SplitterUninit(SSplitter **_this_)
{
    int i;
    SSplitter *_this;
    if(NULL==_this_||NULL==*_this_)
    {
        WPRINTC(PrintComp_splitter,"Warning, freeing NULL splitter");
        return;
    }
    _this=*_this_;
    for(i=0;i<_this->sepamnt;i++)
    {
        free(_this->separators[i]);
    }
    free(_this->separators);

	if(_this->state>EsplitterState_Inited)
	{
		/* Free resources taken at Feed level */
		free(_this->splitme);
		_this->splitme=NULL;
	}
	if(_this->state>EsplitterState_Feed)
	{
		/* Free resources taken at Split level */
//		if(_this->splitted!=NULL)
//		{
			Cexplode_free(_this->splitted);
//		}
		free(_this->result_valid);
		_this->result_valid=NULL;
	    for(i=0;i</*_this->splitsinthisrow*/_this->amntoflines;i++)
    	{
/*        	if(_this->splits!=NULL)
        	{
            	Cexplode_free(_this->splits[i]);
        	}
*/    	}
    	free(_this->splits);
		_this->splits=NULL;
/*    if(_this->matchrange_high!=NULL)
        free(_this->matchrange_high);
    if(_this->matchrange_low!=NULL)
        free(_this->matchrange_low);
*/
		if(_this->explosionindex!=NULL)
			free(_this->explosionindex);
		_this->explosionindex=NULL;
	}
    free(_this);
    _this_=NULL;
}


SSplitter * SplitterInit(char **separators,int amntOfSeps)
{
	SSplitter *_this;
	size_t seplens;
	int i,j;
	_this = malloc(sizeof(SSplitter));
	if(NULL==_this)
	{
		EPRINTC(PrintComp_splitter,"Malloc Failed at SplitterInit()!");
		return NULL;
	}
/*	memset(_this,0,sizeof(SSplitter));
	_this->splitted = malloc(amntOfSeps*sizeof(CexplodeStrings));
	if(NULL==_this->splitted)
    {
		EPRINTC(PrintComp_splitter,"Malloc Failed at SplitterInit()!");
		free( _this);
		return NULL;
	}   
	memset(_this->splitted,0,amntOfSeps*sizeof(CexplodeStrings));
*/
	_this->separators=malloc(sizeof(char *)*amntOfSeps);
    if(NULL==_this->separators)
    {  
        EPRINTC(PrintComp_splitter,"Malloc Failed at SplitterInit()!");
//		free( _this->splitted);
		free( _this);
        return NULL;
    }
	_this->splitdepth=0;
	_this->splitno=0;
	for(i=0;i<amntOfSeps;i++)
	{
		seplens=strlen(separators[i])+1; //+1 for \0
		_this->separators[i]=malloc(seplens);
        if(NULL==_this->separators[i])
        {
            PPRINTC(PrintComp_splitter,"YaY! NULL returned by malloc in SplitterInit!");
            for(j=0;j<i;j++)
                free(_this->separators[i]);
            free( _this);
            return NULL;
        }
		memcpy(_this->separators[i],separators[i],seplens);
		_this->separators[i][seplens-1]='\0';
	}
    _this->uninit=&SplitterUninit;
    _this->init=&SplitterInit;
    _this->feed=&splitter_feed;
    _this->split=&SplitterSplit;
	_this->sepamnt=amntOfSeps;
	_this->nmbrofsplits=0;
	_this->state=EsplitterState_Inited;
    return _this;
}
static int splitter_feed(SSplitter *_this,char *data)
{
	size_t splitlen;
	splitlen=strlen(data)+1;
	_this->splitme=malloc(splitlen);
	if(NULL==_this->splitme)
	{
		EPRINTC(PrintComp_splitter,"malloc failed at splitter_feed()!");
		return -1;
	}
	memcpy(_this->splitme,data,splitlen);
	_this->state=EsplitterState_Feed;
	return 0;
}
SSplitterResult * SplitterSplit(SSplitter *_this)
{
	int i,j;
	int amntOfPieces;
	int rangeindex;
    int range;
	int splitsinnextrow=0;
//	int rangewaschanged=0;
	int *explosionindextmp;
	CexplodeStrings *splitsTmp;
	if(NULL==_this->splitme || EsplitterState_Feed > _this->state)
	{
		EPRINTC(PrintComp_splitter,"ERROR! splitter not feed and split attempted!");
		return NULL;
	}
	_this->state=EsplitterState_Splitted; 

	if(0==_this->splitdepth && 0==_this->splitno)
	{
		//Initial split)
        DPRINTC(PrintComp_splitter,"First Explode:");
        DPRINTC(PrintComp_splitter,"Exploder \"%s\"",_this->separators[0]);
        DPRINTC(PrintComp_splitter,"ToBeExploded: \"%s\"",_this->splitme);
		if(0>=(_this->amntoflines=(_this->splitsinthisrow=Cexplode(_this->splitme,_this->separators[0],&_this->splitted))))
		{
			//Initial split failed => need to feed more?
			//Error occurred in cexplode.
            EPRINTC(PrintComp_splitter,"Cexplode Failed!");
            return NULL;
		}
        helpprint(_this->splitted);

        //XXX: Add NULL checks to allocs
        _this->result_valid=malloc(_this->amntoflines*sizeof(int));
		_this->splits=malloc(_this->splitsinthisrow*sizeof(CexplodeStrings));
		//Set 'matchrange_' to know which exploded pieces belong to which original "line"
		_this->explosionindex=malloc((1+_this->amntoflines)*sizeof(int));
		explosionindextmp=malloc((1+_this->amntoflines)*sizeof(int)+1);
        splitsTmp=malloc(_this->splitsinthisrow*sizeof(CexplodeStrings));
		rangeindex=0;
		range=0;
        DPRINTC(PrintComp_splitter,"Storing to splitArr: Explode Results:");
        DPRINTC(PrintComp_splitter,"Exploder \"%s\"",_this->separators[0]);
		for(i=0;i<_this->splitsinthisrow;i++)
		{
            //init splits
            DPRINTC(PrintComp_splitter,"ToBeExploded: \"%s\"",Cexplode_getNth(i+1,&_this->splitted));
            if(1!=Cexplode(Cexplode_getNth(i+1,&_this->splitted),_this->separators[0],&(_this->splits[i])))
            {
                PPRINTC(PrintComp_splitter,"OMG! I do not know what I am doing!!");
            }

            helpprint(_this->splits[i]);
            
            //init ranges
            _this->explosionindex[i]/*=_this->matchrange_low[i]=_this->matchrange_high[i]*/=i;
        }
		_this->explosionindex[_this->amntoflines-1]=_this->splitsinthisrow-1;
        _this->splitdepth++;
		//Cexplode should return number >0 unless error occurs. And
		//if error occurs, we should not be in here!
		//NULL checks
	}
	else
	{
        //In case of recursive call, do allocations
		if(_this->splitdepth<_this->sepamnt)
			_this->splits=realloc(_this->splits,_this->splitsinthisrow*sizeof(CexplodeStrings));
        if(_this->splitdepth<_this->sepamnt)
            splitsTmp=malloc(_this->splitsinthisrow*sizeof(CexplodeStrings));
        if(_this->splitdepth<_this->sepamnt)
			explosionindextmp=malloc((1+_this->amntoflines)*sizeof(int));
	} 
	for(i=0;i<_this->amntoflines;i++)
	{
		DPRINTC(PrintComp_splitter,"CHECKING before cpy: expindex[%d]=%d",i,_this->explosionindex[i]);
	}
	memcpy(explosionindextmp,_this->explosionindex,(1+_this->amntoflines)*sizeof(int));
	for(i=0;i<_this->amntoflines;i++)
	{
		DPRINTC(PrintComp_splitter,"CHECKING after cpy: expindex[%d]=%d",i,explosionindextmp[i]);
	}
	memset(_this->explosionindex,0,(1+_this->amntoflines)*sizeof(int));
	if(_this->splitdepth<_this->sepamnt)
	{
		j=0;
		for(i=0;i<_this->splitsinthisrow;i++)
		{

			int tmp;
			int rangechanged=0;
			if(explosionindextmp[j]<i)
			{
				j++;
				rangechanged=1;
			}
            DPRINTC(PrintComp_splitter,"Split[%d], string[%d] Explode:",j,i-((!j)?0:(explosionindextmp[j-1]+1)));
            DPRINTC(PrintComp_splitter,"Exploder \"%s\"",_this->separators[_this->splitdepth]);
            if(_this->amntoflines<j)
				PPRINTC(PrintComp_splitter,"J = %d > %d amntoflines",j,_this->amntoflines);
            MAZZERT(_this->amntoflines>=j,"YaY! J larger than  amntoflines(Should be Impossible)");
			DPRINTC(PrintComp_splitter,"REQUESTING expindex[%d]=%d",j,i-((!j)?0:explosionindextmp[j-1]));	
			splitsinnextrow+=(tmp=Cexplode(_this->splits[j].strings[i-((!j)?0:(explosionindextmp[j-1]+1))],_this->separators[_this->splitdepth],&splitsTmp[i]));
            helpprint(splitsTmp[i]);
			//_this->splitsinthisrow should never be larger than matchrange_high[_this->amntoflines], 
			// nor should any matchrange_[j-1]>=matchrange_[j]
//			MAZZERT(i <= _this->matchrange_high[j],"Something has messed my matchrange logic!");
			if(0>=tmp)
			{
				EPRINTC(PrintComp_splitter,"Cexplode failed! returned %d",tmp);
				_this->result_valid[j]=0;
				MAZZERT(0,"SplitterSplit: ATM. The result not valid case is not handled... TODO: Handle?");
				splitsinnextrow-=tmp;
                rangechanged=0;
			}
        }
        j=0;
		amntOfPieces=0;
		_this->explosionindex[_this->amntoflines-1]=splitsinnextrow-1;
		DPRINTC(PrintComp_splitter,"FILLING expindex[%d]=%d",_this->amntoflines-1,splitsinnextrow-1);
	    for(i=0;i<_this->splitsinthisrow;i++)
        {
            int rangechanged=0;
//Calculate new range
			if(i > explosionindextmp[j])
			{
				rangechanged=1;
				DPRINTC(PrintComp_splitter,"FILLING expindex[%d]=%d",j,amntOfPieces-1);
				_this->explosionindex[j]=amntOfPieces-1;
				j++;
			}
			amntOfPieces+=splitsTmp[i].amnt;
			
			{
				//concatenate splitted pieces in first _this->amntoflines spots of _this->splits[] array
				if(0!=i)
				{
					if(rangechanged)
					{
						Cexplode_free(_this->splits[j]);
						_this->splits[j]=splitsTmp[i];
                        DPRINTC(PrintComp_splitter,"Starting Concatenating: splits[%d]",j);
						fflush(stdout);
                        helpprint(_this->splits[j]);
                        //rangechanged=0;
					}
					else
                    {
						//concat maintains sepwasatend member setup from latter CexplodeStrings (it will be the last in created "obj"
                        CexplodeStrings temppi=splitsTmp[i];
						Cexplode_concat(&_this->splits[j],&splitsTmp[i]);
                        DPRINTC(PrintComp_splitter,"Continuing Concatenating: splits[%d]",j);
						fflush(stdout);
                        helpprint(_this->splits[j]);
					    Cexplode_free(temppi); //free unnecessary strings.
						//Cexplode_free(_this->splits[j]);
//                        Cexplode_free(splitsTmp[i]);
                    }
				}
                else
                {
                    DPRINTC(PrintComp_splitter,"Starting Concatenating: splits[%d]",j);
					fflush(stdout);
					Cexplode_free(_this->splits[j]);
                    _this->splits[j]=splitsTmp[i];
                    helpprint(_this->splits[j]);
                }
			}
/*			rangewaschanged=0;
			if(rangechanged)
			{
				_this->matchrange_high[j]=amntOfPieces-1;//make it index
				j++;
				rangechanged=0;
				rangewaschanged=1;
			}
*/
		}
		free(splitsTmp);
		free(explosionindextmp);
		_this->splitdepth++;
		_this->splitsinthisrow=splitsinnextrow;
		//Calculate new range:
/*		rangeindex=0;
		range=0;
		for(i=0;i<_this->splitsinthisrow;i++)
		{
			if(i==_this->matchrange_low[rangeindex])
			{
				_this->matchrange_low[rangeindex]=range;
				if(rangeindex!=0)
					_this->matchrange_high[rangeindex-1]=range;
				rangeindex++;
                if(rangeindex>=_this->amntoflines)
                    break;
			}
    		range+=_this->splits[rangeindex-1].amnt-1;
		}
        if(0!=rangeindex)
            _this->matchrange_high[rangeindex-1]=range;
*/
	}
	else
	{
END:
		;
        int x;
		_this->res=splitteResInit(_this->splits,_this->amntoflines);
		if(NULL==_this->res)
		{
			EPRINTC(PrintComp_splitter,"Result Init FAILED!!");
			return NULL;
		}
		//end reached
		// results are now in
        //_this->splits[0] - _this->splits[_this->amntoflines-1]; 
        DPRINTC(PrintComp_splitter,"Original Line: \"%s\"",_this->splitme);
        //sleep(1);
        DPRINTC(PrintComp_splitter,"Splitters:");
        for(x=0;x<_this->sepamnt;x++)
        {
            DPRINTC(PrintComp_splitter,"%d: \"%s\"",x,_this->separators[x]);
          //  sleep(1);
        }
        for(x=0;x<_this->amntoflines;x++)
        {
            char *temp;
            int no=0;
            while(NULL!=(temp=Cexplode_getnext(&(_this->splits[x]))))
            {
                no++;
                DPRINTC(PrintComp_splitter,"Splitrow: %d, split: %d, piece = \"%s\"",x,no,temp);
            //    sleep(1);
            }
        }
        return _this->res;
	}
	if(_this->splitdepth>=_this->sepamnt)
		goto END;
    _this->res=_this->split(_this);
    return _this->res;
	
}
// Initializes result struct.
// User getting result needs to call splitteResFree, User who calls Init should not!!! 
// Initer needs just to free the array containing Cexplode strings.
// (Data is not copied for performance reasons).
SSplitterResult * splitteResInit(CexplodeStrings *strings,int amnt)
{
	SSplitterResult *_this=malloc(sizeof(SSplitterResult));
	int size=amnt*sizeof(CexplodeStrings);
	if(NULL==_this)
	{
		EPRINTC(PrintComp_splitter,"Malloc failed at splitteRresInit()");
		return NULL;
	}
	_this->iterator=0;
	_this->resamnt=amnt;
	_this->CexpResult=malloc(size);
	//if(NULL==_this->CexpResult)
    if(NULL==_this->CexpResult)
    {
        EPRINTC(PrintComp_splitter,"Malloc failed at splitteRresInit()");
		free(_this);
        return NULL;
    }   
	memcpy(_this->CexpResult,strings,size);
	_this->get=&splitteResGet;
	_this->getNth=&splitteResGetNth;
	_this->init=&splitteResInit;
	_this->free=&splitteResFree;
	return _this;
}
//XXX: Think if this should be made safer by operating on copies of strings, 
// instead of operating with stringes pointed by many pointers.
// I am really not happy with this.



static CexplodeStrings *splitteResGetNth(SSplitterResult *_this,int numberofres)
{
	if(NULL==_this)
	{
		EPRINTC(PrintComp_splitter,"NULL given to splitterResGet");
		return NULL;
	}
	if((unsigned int)numberofres > (unsigned int)_this->resamnt)
	{
		DPRINTC(PrintComp_splitter,"Attempted to retrieve next splitter result when last result already retrieved");
		return NULL;
	}
//	MAZZERT(_this->CexpResult[numberofres-1]!=NULL,"YaY! Something has messed my iterator says splitterResult");
	return &_this->CexpResult[numberofres-1];
}
static CexplodeStrings *splitteResGet(SSplitterResult *_this)
{
    if(NULL==_this)
    {
        EPRINTC(PrintComp_splitter,"NULL given to splitterResGet");
        return NULL;
    }   
	return splitteResGetNth(_this,++_this->iterator);
}

static void splitteResFree(SSplitterResult **_this_)
{
	SSplitterResult *_this;
	int i;
	if(NULL==_this_ || NULL==*_this_)
	{
		EPRINTC(PrintComp_splitter,"splitteResFree - NULL pointer given! (doublefree?)");
        return;
	}
	_this=*_this_;
	for(i=0;i<_this->resamnt;i++)
	{
		Cexplode_free(_this->CexpResult[i]);
	}
	//free(_this->CexpResult);
	free(_this->CexpResult);
	free(_this);
	_this=NULL;
}

