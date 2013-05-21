/* ********************************************************************
 *
 * @file config_file_parser.h
 * @brief functions to parse config files.
 *
 *
 * -Revision History:
 *
 *  - 0.0.3 16.08.2009/Maz  Simplified ScallbackConf, locationoftxt event needs only
 *  						to hold one location/event
 *  - 0.0.2 10.08.2009/Maz  Changed event cfg file separators
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

#ifndef MAZ_BOT_V4_FILE_PROTO_PARSER_H
#define MAZ_BOT_V4_FILE_PROTO_PARSER_H

//TODO: move these defines to some more appropriate place
#define CFG_ID_COLUMN 1
#define CFG_EVTYPE_COLUMN 2
#define CFG_USERLEVEL_COLUMN 3
#define CFG_ORIGINATOR_COLUMN 4
#define CFG_EVENT_FILE_SEPARATOR1 "\n"
#define CFG_EVENT_FILE_SEPARATOR2 "\t"


#include "parser.h"
#include "irc_protocol_definitions.h"
#include <helpers.h>
//#include <APIs/cexplode.h>
#include "splitter.h"
#include <callbackhandler.h>

//#include "irc_protocol_definitions.h"
#define PARSER_RESULT_FILE 2
//This lenght must be such that it  has all bits set to 1. (it is used to mask indexes)
#define FILE_DATA_BUF_LEN 0xFFFF

struct SFILEparser;
struct SFILEparserResult;
typedef enum EFileType
{
    EFileType_CbConf    = 0,
    EFileType_GenConf,
    EFileType_NmbrOf
}EFileType;


typedef int (*FILEparseParseF)(struct SFILEparser *_this);
typedef char *(*FILEparserResultGetF)(struct SFILEparserResult *);
typedef char *(*FILEparserResultGetParamF)(struct SFILEparserResult *,int paramno);
//Generic struct for file parser results. This will specify the cfg file type that was parsed.

//callback specific result struct, shall contain results for parsed callback config file.
typedef struct ScallbackConf
{
    int id;
    EMbotcallbackEventType type;            //Type of event which triggers CB, defined in callbackhandler.h
    int amntOfOriginators;
	EIRCuserLevelDirection userlevel_direction;	//"direction" Eg, exact userlevel, smaller (or equal), larger (or equal)
	EIRCuserLevel userlevel;				//Userlevel
    char *originator_chan;
    char *triggerstring;
//    int amntOfLocations;
//    EMbotEventLocation locationoftxtevent[EVENT_LOCATIONS_MAX];
	EMbotEventLocation locationoftxtevent;
}ScallbackConf;


typedef ScallbackConf *(*cfgCallbackResGetEventF)(SparserResult *);

//Result structure for all file parsers, will be returned by fileparser's parse function.
//Shall contain file specific struct.
typedef struct SFILEparserResult
{
	SparserResult gen; ///<common for all parser results
    SSplitterResult *splitterresult;
}SFILEparserResult;

//typedef void (*cfgCallbackResFreeF)(SparserResult **);
typedef struct SConfigResult_Callback
{
	SFILEparserResult fileparRes;
    int iterator;
    int amntOfCallbacks;
    ScallbackConf *callbackConfigs;
	cfgCallbackResGetEventF eventGet;
//	cfgCallbackResFreeF free;
}SConfigResult_Callback;

typedef struct SFILEparser
{
    Sparser parser;
	SSplitter *splitter;
    FILE *cfgfile;
//	EmsgType type;
//	char *channel;
//	SFILEmessage_parsed parsedMsg;
//	FILEparserParseF preparse;
//	CexplodeStrings exploded;
//	char *msg;
	char datatoparse[FILE_DATA_BUF_LEN+1]; //+1 char for NULL mark possibly written before handing data to Cexplode
	unsigned long long int datastart;
	unsigned long long int dataend;
	SFILEparserResult *result;
}SFILEparser;

//SFILEparser *FILEparserinit(void);
SFILEparser *CfgCbFileParserInit(void);
//Function not related to these classes!
//done specifically for parsing default callback string file!

#endif //MAZ_BOT_V4_FILE_PROTO_PARSER_H

