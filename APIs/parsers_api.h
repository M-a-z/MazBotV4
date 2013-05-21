/* ********************************************************************
 *
 * @file parsers_api.h
 * @brief Generic interface for parsers (IRC, bot - php - bot protocol...)
 *
 *
 * -Revision History:
 *
 *  - 0.0.1 22.07.2009/Maz  First draft
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


#ifndef MAZBOT_V_4_PARSERS_API_H
#define MAZBOT_V_4_PARSERS_API_H

#include <stddef.h>
#include "cexplode.h"

#ifdef __cpp
extern "C"
{
#endif

#define OWN_ID_LEN 200

#define IRC_NICK_MAX 15
#define IRC_HOST_MAX 63
#define IRC_SERVER_MAX 63
#define IRC_CHANNEL_MAX 200
#define IRC_PREFIX_MAX 508
#define IRC_CMD_MAX 510
#define IRC_PARAMS_MAX 508

struct Sparser;

typedef enum EparserType
{
    EparserType_Irc         = 0,
    EparserType_MazBotProto,
    EparserType_NmbrOf
}EparserType;

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

struct SparserResult;
typedef void (*ParserResultFreeF)(struct SparserResult **result);

typedef struct SparserResult
{
    int parserResultType;
	ParserResultFreeF free;
/*    char *sentBy;
    char *sentTo;
    EparserState state;
	*/
}SparserResult;


typedef enum EparserRetVal
{
    EparserRetVal_Ok                = 0,
    EparserRetVal_NotInitialized,
    EparserRetVal_InternalError,
    EparserRetVal_InsufficientFeed,
    EparserRetVal_InvalidParam,
    EparserRetVal_OverFeed
}EparserRetVal;

/**
 	@brief Gives more raw network data to parser for parsing.
    @param struct Sparser *_this pointer to parser struct returned by call to ParserInit
    @param char *data raw data to be parsed
    @param size_t datasize Amount of data given
    @param EparserState *state parser fills this according to state. Struct must be allocated by user.
    @returns Amount of 'commands' parsed given data, negative number upon error.
*/
typedef EparserRetVal (*ParserDataFeedF)(struct Sparser *_this,char *data,size_t datasize, EparserState *state);


/**
 @brief Receives parsed 'command' from parser.
 @param struct Sparser *_this pointer to parser struct returned by call to ParserInit
 @param EparserState *state parser returns it's status AFTER extracting this result.
 @returns struct SparserResult * pointer to parser type specific structure, filled and allocated by the parser. NULL on error.
 @see SIRCparserResult
*/
typedef SparserResult * (*ParserResultGetF)(struct Sparser *_this, EparserState *state);

//typedef void (*ParserResultFreeF)(struct Sparser *_this, SparserResult **result);
/**
 * @brief generalization of parsers
 * @warning Struct's data members are not meant to be touched! They're named only for easing debugging!
 */
typedef struct Sparser
{
    EparserType         type;
    EparserState        state;
    char                own_id[OWN_ID_LEN];
    ParserDataFeedF     feed;
    ParserResultGetF    get_result;
//    ParserResultFreeF   free_result;
}Sparser;
/**
 * @brief Initializes and allocates parser of desired type for use.
 * @note Must be called before parsed is used!
 * @param EparserType type Type of desired parser
 * @returns parser instance ready to be used.
 * @see ParserDataFeedF ParserResultGetF ParserResultFreeF
 */
Sparser *ParserInit(EparserType type);

// IRC specifics:
/**
 * @brief user information parsed from message
 * @see EIRCuserType
 */
typedef struct SIRC_user
{
    EIRCuserType type;
    char nick[IRC_NICK_MAX];
    char host[IRC_HOST_MAX];
    char server[IRC_SERVER_MAX];
    char channel[IRC_CHANNEL_MAX];
}SIRC_user;

struct SIRCparserResult;

typedef char *(*IRCparserResultGetF)(struct SIRCparserResult *);
typedef char *(*IRCparserResultGetParamF)(struct SIRCparserResult *,int paramno);

/**
 * @brief struct for returning results of IRC parser.
 * @see SIRC_user
 */
typedef struct SIRCparserResult
{   
    SparserResult 	gen;	///< generic parser results struct
//  SIRC_user 		user;		///< User information
    int 			hasprefix;		///< Internal use 
    int 			numofparams;	///< Internal	
	IRCparserResultGetF getcmd;			///<way to get command Returns char * takes (SIRCparserResult *result)
	IRCparserResultGetF getprefix;		///<way to get prefix Returns char * takes (SIRCparserResult *result)
	IRCparserResultGetParamF getparam;	///<way to get Nth param Returns char * takes (SIRCparserResult *,int paramno)
	CexplodeStrings parsed;				///<internal

}SIRCparserResult;

#ifdef __cpp
}
#endif


#endif

