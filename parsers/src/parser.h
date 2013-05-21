/* ********************************************************************
 *
 * @file parser.h
 * @brief Generic interface for parsers (IRC, bot - php - bot protocol...)
 *
 *
 * -Revision History:
 *
 *  - 0.0.3 17.08.2009/Maz  Added uninit.
 *                          Added basicCfgParser
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




#ifndef MAZ_BOT_V4_PARSER_H
#define MAZ_BOT_V4_PARSER_H

#define OWN_ID_LEN 200

#include <stddef.h>
#include <generic.h>

struct Sparser;
struct SparserResult;

typedef enum EparserType
{
	EparserType_Irc 		= 0,
	EparserType_MazBotProto,
    EparserType_CfgCallback,
    EparserType_CfgBasic,
	EparserType_NmbrOf
}EparserType;

typedef enum EparserResultType
{
    EparserResultType_Unknown       = 0,
    PARSER_RESULT_IRC               = 1,
    EparserResultType_CfgCallback,
    EparserResultType_CfgBasic,
    EparserResultType_NmbrOf
}EparserResultType;


typedef enum EparserState
{
    EparserState_FatalError     =  -3,           ///< Some error occurred in parser, and it is no longer usable.
	EparserState_Choked		    = -2,
	EparserState_Full			= -1,
	EparserState_Inited			=  0,				///< Initialized but no data given yet.
	EparserState_ParserEmpty	=  1,			///< All results extracted, ready for feeding. 
	EparserState_ResultReady	=  2,			///< Results pending.
	EparserState_NeedMoreData	=  3,			///< Some data given, but unable to parse since rest is missing => feed more.
	EparserState_NmbrOf			=  4
}EparserState;

///Frees the allocated parser specific result structure. Pointer is NULLed upon freeing.
/* When parser user has obtained result struct from parser and read all results,
 * this should be called to free the space allocated in result struct. After the call, the result struct is no
 * longer available. WARNING, this may free data pointed by ptrs obtained when using parser result type specific
 * functions. (which return the actual parsed strings ) 
 * TODO: See what is actually freed, and try to unify freeing policy between different parsers. If necessary, create 2 different
 * freeing functions, one freeing result struct internal data only, and the other freeing also results. */
typedef void (*ParserResultFreeF)(struct SparserResult **result);
/* Generic result function. When you obtain result from parser, cast this to parser specific result struct. */
//TODO: See if there's more convenient way.
typedef struct SparserResult
{
	EparserResultType parserResultType;
	ParserResultFreeF free;
}SparserResult;

typedef enum EparserRetVal
{
	EparserRetVal_Ok				= 0,
	EparserRetVal_NotInitialized,
	EparserRetVal_InternalError,
	EparserRetVal_InsufficientFeed,
	EparserRetVal_InvalidParam,
	EparserRetVal_OverFeed
}EparserRetVal;

/*
 ///\function Gives more raw network data to parser for parsing.
	@param struct Sparser *_this pointer to parser struct
	@param char *data raw data received from network.
	@param size_t datasize Amount of data given
	@param EparserState *state parser fills this according to state. Struct must be allocated by user.
	@returns Amount of 'commands' parsed given data, negative number upon error.
*/
typedef EparserRetVal (*ParserDataFeedF)(struct Sparser *_this,char *data,size_t datasize, EparserState *state);

/*
 ///\function Receives parsed 'command' from parser.
 @param struct Sparser *_this pointer to parser struct
 @param EparserState *state parser returns it's status AFTER extracting this result.
 @returns struct SparserResult * pointer to parser type specific structure, filled and allocated by the parser. NULL on error.
*/
typedef SparserResult * (*ParserResultGetF)(struct Sparser *_this, EparserState *state);
/* Initializes  parser - Must be called before parser is used (feed, or results obtained */
typedef struct Sparser *(*ParserInitF)(EparserType type);
typedef void (*ParserUninitF)(struct Sparser **);
/* Generic parser structure */
typedef struct Sparser
{
	EparserType 		type;
	EparserState		state;
	char 				own_id[OWN_ID_LEN];
	ParserDataFeedF 	feed;
	ParserResultGetF 	get_result;
    ParserUninitF       uninit;
//	ParserResultFreeF	free_result;
}Sparser;
Sparser *ParserInit(EparserType type);

#endif //MAZ_BOT_V4_PARSER_H
