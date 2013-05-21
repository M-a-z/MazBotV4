/* ********************************************************************
 *
 * @file mbot_pseudo_xml.c
 * @brief MazBot pseudo xml implementation.
 *
 *
 * -Revision History:
 *
 *  -0.0.2  16.03.2010/Maz  Small bug fixes
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



#include "mbot_pseudo_xml.h"
#include <splitter.h>

#define MBOT_ATTRS_MAX 4
static int handle_tag(SmbotPseudoxmlTag **taglist, char *string);
static int addValueAttrib(SmbotPseudoxmlTag *newtag,char *value);
static int addLenAttrib(SmbotPseudoxmlTag *newtag,char *value);
static int addTypeAttrib(SmbotPseudoxmlTag *newtag,char *value);
static int addNameAttrib(SmbotPseudoxmlTag *newtag,char *value);
static SmbotPseudoxmlTag *IrcCfgAllocNewTag(SmbotPseudoxmlTag **taglist);
static void IrcCfgFinalizeNewTag(SmbotPseudoxmlTag *newtag);

static const char *G_mbot_allowed_attribs[]={"name","type","length","value"};

static void IrcCfgFinalizeNewTag(SmbotPseudoxmlTag *newtag)
{
	if(NULL!=newtag->prev)
		newtag->prev->next=newtag;
	if(NULL!=newtag->parenttag && NULL==newtag->parenttag->subtags)
		newtag->parenttag->subtags=newtag;
}

static SmbotPseudoxmlTag *IrcCfgAllocNewTag(SmbotPseudoxmlTag **taglist)
{
	SmbotPseudoxmlTag *prevtag;
	SmbotPseudoxmlTag *newtag;

	newtag=malloc(sizeof(SmbotPseudoxmlTag));
	if(NULL==newtag)
	{
		//Error, alloc failed!
		EPRINTC(PrintComp_IrcCfg,"Error, alloc failed");
		return NULL;
	}	
	memset(newtag,0,sizeof(SmbotPseudoxmlTag));
	if(NULL==*taglist)
	{
		/* This is the first tag */
		*taglist=newtag;
		newtag->first=newtag;
	}
	else
	{
		prevtag=*taglist;
		if(!prevtag->closed)
		{
			/* YaY! I'm the first child */
//			prevtag->subtags=newtag;
			newtag->parenttag=prevtag;
			newtag->depth=prevtag->depth+1;
			newtag->next=NULL;
			newtag->prev=NULL;
			newtag->first=newtag;
			newtag->subtags=NULL;
		}
		else if(NULL!=(*taglist)->parenttag && !(*taglist)->parenttag->closed)
		{
			/* I'm yet another subtag */
//			prevtag->next=newtag;
			newtag->parenttag=prevtag->parenttag;
			newtag->depth=prevtag->depth;
			newtag->next=NULL;
			newtag->prev=prevtag;
			newtag->first=prevtag->first;
			newtag->subtags=NULL;
		}
		else
		{
			/* I'm depth 0 tag, but not first one */
//			prevtag->next=newtag;			
			newtag->parenttag=NULL;
			newtag->depth=0;
			newtag->next=NULL;
			newtag->prev=prevtag;
			newtag->first=prevtag->first;
			newtag->subtags=NULL;
		}
	}
	return newtag;
}
/*typedef struct SmbotPseudoxmlTag
 * {   
 *     char *name;                      ///< tag name;
 *         EmbotPseudoxmlType valuetype;   ///< tag value type
 *             unsigned int depth;             
 *                 unsigned int size;              ///< value size
 *                     void *value;
 *                         int closed;
 *                             SmbotPseudoxmlTag *next;        ///< next tag in same level
 *                                 SmbotPseudoxmlTag *prev;        ///< previous tag in same level
 *                                     SmbotPseudoxmlTag *first;       ///< first tag in this level
 *                                         SmbotPseudoxmlTag *parenttag;       ///< upper level tags - see example
 *                                             SmbotPseudoxmlTag *subtags;     ///< list of tags belonging under this tag - see example
 * }SmbotPseudoxmlTag;
 */


static int addNameAttrib(SmbotPseudoxmlTag *newtag,char *value)
{
	size_t len;
	MAZZERT(NULL!=newtag && NULL!=value,"NULL ptr given to addNameAttrib()");
	len=strlen(value);
	newtag->name=malloc(len+1);
	if(NULL==newtag->name)
	{
		EPRINTC(PrintComp_IrcCfg,"NULL ptr given to addNameAttrib()");
		return -1;
	}
	memcpy(newtag->name,value,len+1);
	return 0;
}
static int addTypeAttrib(SmbotPseudoxmlTag *newtag,char *value)
{
	MAZZERT(NULL!=newtag && NULL!=value,"NULL ptr given to addTypeAttrib()");
	if(!strcmp(value,"char"))
		newtag->valuetype=EmbotPseudoxmlType_char;
	else if(!strcmp(value,"8bit"))
		newtag->valuetype=EmbotPseudoxmlType_8bit;
	else if(!strcmp(value,"16bit"))
		newtag->valuetype=EmbotPseudoxmlType_16bit;
	else if(!strcmp(value,"32bit"))
        newtag->valuetype=EmbotPseudoxmlType_32bit;
	else if(!strcmp(value,"64bit"))
        newtag->valuetype=EmbotPseudoxmlType_64bit;
	else
	{
		//Unknown value type given!
		EPRINTC(PrintComp_IrcCfg,"Unknown value type given %s",newtag->name);
		return -1;
	}
	return 0;
}
static int addLenAttrib(SmbotPseudoxmlTag *newtag,char *value)
{
	unsigned int len;
	char *endptr;
	MAZZERT(NULL!=newtag && NULL!=value,"NULL ptr given to addLenAttrib()");
	len=strtol(value,&endptr,10);
	if('\0'!=*value && '\0'==*endptr)
	{
		newtag->size=len;
	}
	else
	{
		/* Non numeric value for length attribute! => error*/
		EPRINTC(PrintComp_IrcCfg,"Non numeric value for length attribute!");
		return -1;
	}
	return 0;
}
static int addValueAttrib(SmbotPseudoxmlTag *newtag,char *value)
{
	size_t len;
	//int i;
	int numeric=0;
	MAZZERT(NULL!=newtag && NULL!=value,"NULL ptr given to addValueAttrib()");
	if(0==newtag->size)
	{
		if(newtag->valuetype!=EmbotPseudoxmlType_char && newtag->valuetype!=0)
			newtag->size=1;
		else
			newtag->size=strlen(value)+1;
	}
		
	switch(newtag->valuetype)
	{

		case EmbotPseudoxmlType_64bit:
			newtag->value=malloc(sizeof(unsigned long long int)*newtag->size);
			numeric=1;
			break;
		case EmbotPseudoxmlType_32bit:
			newtag->value=malloc(sizeof(int)*newtag->size);
			numeric=1;
			break;
		case EmbotPseudoxmlType_16bit:
			newtag->value=malloc(sizeof(short int)*newtag->size);
			numeric=1;
			break;
		case EmbotPseudoxmlType_8bit:
			newtag->value=malloc(sizeof(char)*newtag->size);
			numeric=1;
			break;
		case EmbotPseudoxmlType_char:
			if(0==newtag->size)
				newtag->size=strlen(value)+1;
			newtag->value=malloc(sizeof(char)*newtag->size);
		default:
			/* Warning, unknown type! - assuming char */
			WPRINTC(PrintComp_IrcCfg,"unknown type config (%s)! - assuming char",newtag->name);
			newtag->valuetype=EmbotPseudoxmlType_char;
			if(0==newtag->size)
				newtag->size=strlen(value)+1;
			newtag->value=malloc(sizeof(char)*newtag->size);
	}
	len=newtag->size;
	if(NULL==newtag->value)
	{
		/* Error, malloc failed! */
		PPRINTC(PrintComp_IrcCfg,"Malloc FAILED!");
		return -1;
	}
	if(numeric)
	{
		unsigned long long int val;
		char *endptr;
		CexplodeStrings ploder;
		int ploded;
		int index;
		char *value_cut;
		if(len<(ploded=Cexplode(value,",",&ploder)))
		{
			EPRINTC(PrintComp_IrcCfg,"Lenght %d given to tag %s, but %d values assigned!",len,newtag->name,ploded);
			return -1;
		}
		value_cut=Cexplode_getfirst(&ploder);
		for(index=0;value_cut!=NULL;index++)
		{
			val=strtoll(value_cut,&endptr,0);
			if('\0'==*value_cut || '\0'!=*endptr)
			{
				EPRINTC(PrintComp_IrcCfg,"Expected numeric config for tag %s, but value %s given!",newtag->name,value_cut);
					Cexplode_free(ploder);
				return -1;
			}
			switch(newtag->valuetype)
			{
				case EmbotPseudoxmlType_64bit:
					((unsigned long long *)newtag->value)[index]=val;
					break;
				case EmbotPseudoxmlType_32bit:
					((unsigned int *)newtag->value)[index]=(unsigned int)val;
					break;
				case EmbotPseudoxmlType_16bit:
					((unsigned short int *)newtag->value)[index]=(unsigned short int)val;
					break;
				case EmbotPseudoxmlType_8bit:
					((unsigned char*)newtag->value)[index]=(unsigned char)val;
					break;
				default:
					MAZZERT(0,"Unknown valuetype!");
					break;
			}
			value_cut=Cexplode_getnext(&ploder);
		}
		for(;index<len-1;index++)
		{
			switch(newtag->valuetype)
			{
					case EmbotPseudoxmlType_64bit:
					((unsigned long long *)newtag->value)[index]=0;
					break;
				case EmbotPseudoxmlType_32bit:
					((unsigned int *)newtag->value)[index]=0;
					break;
				case EmbotPseudoxmlType_16bit:
					((unsigned short int *)newtag->value)[index]=0;
					break;
				case EmbotPseudoxmlType_8bit:
					((unsigned char*)newtag->value)[index]=0;
					break;	
                default:
                    MAZZERT(0,"WTF???");
                    break;
			}
		}
		
	}
	else
	{
		/* Non Numeric value */
		memcpy(newtag->value,value,newtag->size-1);
		((char *)newtag->value)[newtag->size-1]='\0';
	}
	return 0;
}

static int handle_tag(SmbotPseudoxmlTag **taglist, char *string)
{
	const char *separators[2]={PSEUDOXML_ATTRIB_SEPARATOR,"="};
	int i;
	int attrindex;
	SmbotPseudoxmlTag **tag=taglist;
    SmbotPseudoxmlTag *newtag;
//	int retval=0;
	SSplitter *splitter;
	SSplitterResult *splitres;
	//int cexploderet;
	size_t stringlen;
	char *tmp;
	CexplodeStrings *explode;
	int closed=0;
	int (*add_attrib[4])(SmbotPseudoxmlTag *,char *);

	add_attrib[0]=&addNameAttrib;
	add_attrib[1]=&addTypeAttrib;
	add_attrib[2]=&addLenAttrib;
	add_attrib[3]=&addValueAttrib;

	MAZZERT(NULL != string,"NULL string given to handle_tag!");
	mbot_ltrim(string,' ');
	mbot_ltrim(string,'\t');
	//Allowed formats:
	//<name=name type=type value=value/>
	//<name=name type=type value=value>
	// <othertags>
	//</endtag>
	
	stringlen=strlen(string);
	if(string[stringlen-1]!='>')
	{
		//Error
		EPRINTC(PrintComp_IrcCfg,"Error");
		goto errOut;
	}
	//get rid of the ending '>' char
	string[stringlen-1]='\0';
	if('/'==string[stringlen-2])
	{
		closed=1;
		string[stringlen-2]='\0';
	}
	//get tags:
	for(i=0;i<stringlen-1;i++)
	{
		if('<'==string[i] || '>'==string[i])
		{
			//Illegal character in string
			EPRINTC(PrintComp_IrcCfg," < or > character included inside tag - split failed!");
			goto errOut;
		}
	}
	if(NULL==(splitter=SplitterInit((char **)&separators,2)))
	{
		//Error splitter init failed!
		EPRINTC(PrintComp_IrcCfg,"split failed!");
		goto errOut;
	}
	if(0!=splitter->feed(splitter,string))
	{
		//splitter feed probs failed?
		EPRINTC(PrintComp_IrcCfg,"split failed!");
		goto errOut;
	}
	if(NULL==(splitres=splitter->split(splitter)))
	{
		//split failed!
		EPRINTC(PrintComp_IrcCfg,"split failed!");
		goto errOut;
	}
	//with empty cfg lines we should never get here!
	MAZZERT(NULL!=(explode=splitres->get(splitres)),"splitter result returned NULL! Impossible!");
	//First piece should always be there!
	MAZZERT(NULL!=(tmp=Cexplode_getfirst(explode)),"Impossible, Cexplode_getfirst returned NULL!");;
	if(tmp[0]=='/')
	{
		/* Were closing tag which name is (char *)&tmp[1] */
		if(NULL==*tag || NULL==(*tag)->parenttag)
		{
			/* Error, invalid closing tag in cfg file! */
			EPRINTC(PrintComp_IrcCfg,"Invalid closig tag in configs!");
			goto errOut;
		}
		if(strcmp(&tmp[1],(*tag)->parenttag->name))
		{
			//Noo, parent tag was tag->parenttag->name, but closing tag for &tmp[1] was found! 
			EPRINTC(PrintComp_IrcCfg,"Invalid closig tag in configs!");
			goto errOut;
		}
		if((*tag)->parenttag->closed==1)
		{
			//Double closure for tag &tmp[1]
			EPRINTC(PrintComp_IrcCfg,"Double closure for tag %s",&tmp[1]);
			goto errOut;
		}
		/* CHECK THERE'S NO MORE RESULTS IN SPLITTER! */
		/* No tag specified in this line (closing tag must be alone => go up one level, and do not add new tag */
		(*tag)=(*tag)->parenttag;
		(*tag)->closed=1;
		return 0;
	}
	else
	{
		newtag=IrcCfgAllocNewTag(tag);
		MAZZERT(NULL!=newtag,"Allocating new config tag FAILED!");
		if(closed)
		{
			newtag->closed=1;
		}
		while(explode!=NULL)
		{
		/* This should be opening tag, not closing tag lets check through the attribs */
			while(NULL!=tmp)
			{
				int found=0;
				char *value;
				value=Cexplode_getnext(explode);
				if(NULL==value)
				{
					//Error, tag attribute in mbot pseudo xml must always have a value - unless it is a closing tag (which was already handled abowe)
					EPRINTC(PrintComp_IrcCfg,"tag with no value in configs!");
					goto err_freenewtag;
				}
			
		/* TODO: Change this so, that attribs will be scanned in order name, type, len, value */	
				for(attrindex=0;attrindex<MBOT_ATTRS_MAX;attrindex++)
				{
					if(!strcmp(G_mbot_allowed_attribs[attrindex],tmp))
					{
						found=1;
						break;
					}
				}
				if(1==found)
				{
					/* add value for attrib */
					/* Jump table to add correct thing in tag struct */
					if(0!=add_attrib[attrindex](newtag,value))
					{
						EPRINTC(PrintComp_IrcCfg,"attribute adding failed!");
						goto err_freenewtag;
						/* attribute adding failed! */
					}
				}
				else
				{
					/* Error, inexisting attribute given in tag! */
					EPRINTC(PrintComp_IrcCfg,"inexisting attribute (%s) given in tag!",tmp);
					goto err_freenewtag;
				}
				tmp=Cexplode_getnext(explode);
			}
			explode=splitres->get(splitres);
			if(NULL!=explode)
				tmp=Cexplode_getnext(explode);
		}
//#endif

	}
	if(NULL==newtag->name || NULL==newtag->value)
	{
		EPRINTC(PrintComp_IrcCfg,"name (%s) && value (%s) attribute are compulsory for config tags!",newtag->name,(char *)newtag->value);
		goto err_freenewtag;
	}
	if(0==newtag->valuetype)
	{
		DPRINTC(PrintComp_IrcCfg,"No type set for tag %s => defaulting to char",newtag->name);
		newtag->valuetype=EmbotPseudoxmlType_char;
	}
	if(EmbotPseudoxmlType_char==newtag->valuetype)
	{
		if(0==newtag->size)
		{
			newtag->size=strlen(newtag->value)+1;
		}
	}

	//Add memory releasing (splitter och so vidare!)
	if(0)
	{
	err_freenewtag:
		//TODO: Add splitter etc freeing
		if(NULL!=newtag->value)
		{
			free(newtag->value);
		}
		if(NULL!=newtag->name)
			free(newtag->name);
/*
	if(NULL!=newtag->value)
			free(newtag->value);
*/
		if(newtag->prev!=NULL)
			newtag->prev->next=NULL;
		else if(newtag==*taglist)
			taglist=NULL;
		else
			newtag->parenttag->subtags=NULL;
		free(newtag);
	errOut:
		return -1;
	}

	IrcCfgFinalizeNewTag(newtag);
	*taglist=newtag;
	return 0;
}
int get_tags(SmbotPseudoxmlTag **tag, FILE *cfgfile)
{
	char *string;
	//int ok=1;
	int retval=0;
	int lineno;
	MAZZERT(NULL!=cfgfile && NULL != tag,"Null ptr given in get_tags()");
	*tag=NULL;
	for(lineno=0;;lineno++)
	{
		int scanned;
		/* eliminate comment rows */
		scanned=fscanf(cfgfile,"#%a[^\n]\n",&string);
		if(scanned>0)
		{
			free(string);
			continue;
		}
		else if(EOF==scanned)
		{
			/* EOF ? */
			break;
		}
		scanned=fscanf(cfgfile,"<%a[^\n]\n",&string);
		if(scanned>0)
		{
			if(0!=(retval=handle_tag(tag,string)))
			{
				EPRINTC(PrintComp_IrcCfg,"Error while handling configfile(around line %d)!",lineno+1);
				break;
			}
		}
		if(EOF==scanned)
		{
			/* EOF ? */
			break;
		}
	}
	/* revert *tag to point back at the first of first tag */
	/* We should already be at level 0 (because all opened tags must be closed) */
	MAZZERT(*tag==NULL || (0==(*tag)->depth && NULL==(*tag)->parenttag),"Nooo, some tag left open in cfg file??");
	/* Now, revind back to first tag at level 0 */
	if(NULL!=*tag)
		*tag=(*tag)->first; 
/*	*tag=*tag->first; -- crashes gcc :)*/ 
	MAZZERT(NULL!=*tag && *tag == (*tag)->first && 0==(*tag)->depth,"Logic messed up :(");
	return retval;
}


