/* ********************************************************************
 *
 * @file irc_protocol_parserh.
 * @brief functions to parse IRC message streams into [prefix] command [params] set, and extract sender info
 *
 *
 * -Revision History:
 *
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

#ifndef MAZ_BOT_V4_IRC_PROTO_PARSER_H
#define MAZ_BOT_V4_IRC_PROTO_PARSER_H


#include "parser.h"
#include "irc_definitions.h"
#include <helpers.h>

//#include "irc_protocol_definitions.h"
//This lenght must be such that it  has all bits set to 1. (it is used to mask indexes)
#define IRC_DATA_BUF_LEN 0xFFF

/*typedef struct SIRCmessage_parsed
{
}IRCmessage_preparsed;
*/
struct SIRCparser;

typedef int (*IRCparseParseF)(struct SIRCparser *_this);

struct SIRCparserResult;

typedef char *(*IRCparserResultGetF)(struct SIRCparserResult *);
typedef char *(*IRCparserResultGetParamF)(struct SIRCparserResult *,int paramno);

typedef struct SIRCparserResult
{
	SparserResult gen; ///<common for all parser results
//	SIRC_user user; 
	int hasprefix;	   ///<internal use
	int numofparams;   ///<internal use

	IRCparserResultGetF getcmd;			///<way to get command Returns char * takes (SIRCparserResult *result)
	IRCparserResultGetF getprefix;		///<way to get prefix Returns char * takes (SIRCparserResult *result)
	IRCparserResultGetParamF getparam;	///<way to get Nth param Returns char * takes (SIRCparserResult *,int paramno)
	char *raw_data;
	CexplodeStrings parsed;				///<internal

}SIRCparserResult;

typedef struct SIRCparser
{
	Sparser parser;
//	EmsgType type;
	char *channel;
//	SIRCmessage_parsed parsedMsg;
//	IRCparserParseF preparse;
	CexplodeStrings exploded;
	char *msg;
	char datatoparse[IRC_DATA_BUF_LEN+1]; //+1 char for NULL mark possibly written before handing data to Cexplode
	unsigned long long int datastart;
	unsigned long long int dataend;
    mbot_linkedList *result;
//	SIRCparserResult *result;
}SIRCparser;

SIRCparser *IRCparserinit(void);


#endif //MAZ_BOT_V4_IRC_PROTO_PARSER_H

