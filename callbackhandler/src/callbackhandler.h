/* ********************************************************************
 *
 * @file callbackhandler.h
 * @brief callback handling engine related declarations
 *
 *
 * -Revision History:
 * 
 *  - 0.0.2  12.08.2009/Maz  Ready for first compile attempts ;)
 *  - 0.0.1  28.07.2009/Maz  First draft
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


#ifndef MAZBOT_v4_CALLBACKHANDLER_H
#define MAZBOT_v4_CALLBACKHANDLER_H
#define DOMAIN_MAX 499
#define URL_MAX 499
#include <irc_definitions.h>
#include <networking.h>
#include <irc_protocol_parser.h>
/* 
 *Event handler specification
 *
 *
 * So basically triggering actions can be specified in bot config files. In addition, the specified actions may have
 * callback functions registered to bot.
 *
 * Config file could be format:
 *
 * UniqueIdNumber		callbackEventType	UserLevel	Originator(chan)		[triggerstring] 	[Chan/Privmsg/Dcc]
 *
 * UniqueIdNumber:
 * This will be used when callbackfunction is registered. FunctionId must match the number specified in config file.
 * (callback matching specified even is detected based on this ID)
 *
 * callbackEventType:
 * A numeric value matching numeral in EMbotcallbackEventType enum.
 *
 * Userlevel:
 * Userlevel which is required for executing this event. Level is given as <prefix><numeral> where prefix can be one of following:
 * =		The level of event triggerer must exactly match the level specified
 * <		The level of event triggerer must be either smaller or equal to specified
 * >		The level of event triggerer must be either greater or equal to specified
 * Numeric value can be one of the following:
 * 0
 * 1
 * 2
 * 3
 *
 * Levels can be used as follows:
 * In main config file, the bot admin is specified. One can choose a password verification, nick@hostmask based verification
 * or nick based verification when nick is registered to IRC services (and when whois query for registered users give out certain tect line)
 * (Note that same method is used for identifying other users). Admin gets userlevel 3 (EIRCuserLevel_admin)
 *
 * By default all non registered IRC users are set to level 0 (EIRCuserLevel_guest) but exceptions can be added.
 * TODO: nick@mask - userlevel or password - userlevel pairs can be added in irc_users.conf file, depending on identification mode used.
 * If identification mode is set to "nicks only and with registered nicks", then the hostmask part is ignored (but may not be empty)
 * TODO: When user has level > 1, user can add new users up to his own level on IRC.
 *
 * Originating channel:
 * #chan can be specified. Also #chan1;#chan2;#chan3;#chan4 
 * If multiple chans is listed, then any specified chan is Ok.
 *
 * One can also define -#chan, this means that any chan EXCEPT specified is Ok. 
 *
 * Triggerstring (Only for text events):
 * Text string that triggers the action. * can be used as wildcard.
 *
 * Chan/Privmsg/Dcc (Only for text events) (optional):
 * Numeric value telling where (text) event should be detected (as in EMbotEventLocation). Default is all Dcc may never realize due to different protocol - 
 * but I wanted to leave space for it.
 *
 *
 *
 *
 * Web callbacks will call cb function (if such is specified), and in addition to event specific 
 * data also connected (and handshaked) connection object to web server. (bad english I know).
 * If no matching local cb is registered, default callback (which just sends event specific info to web server)
 * will be called.
 *
 * Local callbacks will also have a default action to say specified text string as reply to chan/PRIVMSG
 *
 * Join/part event will deliver user details + (server?) and channel information (and event type) to callback.
 *
 * Txt event will deliver user details, IRC message type (chan/priv/dcc msg - dcc support may be added one day?), 
 * chan/server info, event type and text string.
 */
//int mazbot_register_callback(

typedef enum ECallbackRetval
{
	ECallbackRetval_Ok = 0,
	ECallbackRetval_InvalidParam,
	ECallbackRetval_InternalError
}ECallbackRetval;

typedef enum EMbotEventLocation
{
	EMbotEventLocation_Privmsg 		= 1,
	EMbotEventLocation_Chan			= 2,
	EMbotEventLocation_CPM			= 3, //Chan or privmsg
	EMbotEventLocation_Dcc			= 10, //Dcc (may not realize)
	EMbotEventLocation_CD			= 11, //Chan or Dcc (may not realize)
	EMbotEventLocation_PMD			= 12, //Privmsg or Dcc (may not realize)
	EMbotEventLocation_All			= 99
}EMbotEventLocation;

typedef enum EMbotcallbackEventType
{
	EMbotcallbackEventType_LocalTxtEvent = 1,//Event when specified txt string is encountered on chan/PRIVMSG. Cb is executed locally.
	EMbotcallbackEventType_WebTxtEvent,		//Event when specified txt string is encountered on chan/PRIVMSG. Cb sends data to server.
	EMbotcallbackEventType_TEXTEVENTEND,
	EMbotcallbackEventType_LocalJoin,		//Event when user joins, cb is executed locally
	EMbotcallbackEventType_WebJoin,			//Event when user joins, cb sends data to server
	EMbotcallbackEventType_LocalPart,		//Event when user parts, cb is executed locally
	EMbotcallbackEventType_WebPart,			//Event when user parts, cb sends data to server
	EMbotcallbackEventType_LocalCtCp,
	EMbotcallbackEventType_WebCtCp,
	EMbotcallbackEventType_LocalRaw,
	EMbotcallbackEventType_WebRaw,
	EMbotcallbackEventType_nmbrOf			//Keep this as last item!
}EMbotcallbackEventType;

/* The callback storage shall be filled by user in callback registration hook
 * User provides all callback arguments, specific to callback type, and an eventID
 * specifying the event callback should be bound to.
 */ 
typedef struct SServerCallbackArgs
{
	void *handle;  /* 
					  Currently this will probs be pointer to connection object, but things may change... 
					  Some day it may well be pointer to server data, or even something completely else.
					  Anyways, one should never use it to anything else but being a handle to use with
					  send/requestdata/XXX macros/functions offered by MazBot's support libraries.
					*/
	char *raw_data; /* Raw data as server passed it */
	SIRCparserResult *parsed_data; /* Parsed irc data in format explained at APIs/parsers_api.h */
	void *userdataptr;				/* Pointer to data user provided when registering the callback */
}SServerCallbackArgs;

typedef void *(*ServerCallbackF)(SServerCallbackArgs args);
typedef struct SServerCallbackList
{
	    ServerCallbackF callback;
		SServerCallbackArgs args;
		struct SServerCallbackList *next;
}SServerCallbackList;

int callbacklist_init(SServerCallbackList **lpp,ServerCallbackF cb,void *userptr);
int callbacklist_add(SServerCallbackList *list,ServerCallbackF cb,void *userptr);

/*
typedef struct Scallbackworker
{
	unsigned int callbacktypemask;
}Scallbackworker;
*/

/*
struct SWebServerDetails;
//typedef void (*mazbot_local_event_callbackF)(int eventId,char *ircCommand, SIRC_user issuer, char **params, Sconn *conn, void *opaque);
typedef void (*mazbot_callbackF)(int eventId,char *ircCommand, SIRC_user issuer, char **params, Sconn *conn, struct SWebServerDetails *webservdetails, void *opaque);
typedef struct SMazBotcallback
{
    int eventId;
	EMbotcallbackEventType type;
	void *opaque;
	size_t opaquesize;
}SMazBotcallback;

typedef struct SMazBotcallback_Local
{
    SMazBotcallback gen;
	mazbot_callbackF cb;
}SMazBotcallback_Local;
typedef struct SWebServerDetails
{
    char domain[DOMAIN_MAX+1];
	char url[URL_MAX+1];
	unsigned short int port;
	Sconn webConn;
}SWebServerDetails;
typedef struct SMazBotcallback_Web
{
    SMazBotcallback gen;
	mazbot_callbackF cb;
//	mazbot_web_event_callbackF cb;
    SWebServerDetails serverinfo;
}SMazBotcallback_Web;
//Extension creator calls these at startup phase.
//Add callback to callback storage. (If there's event registered, if not, print warning and ignore callback)
//Update callback ID (overwrite default ID, or add new callback ID. in event storage for fast access.)
//NOTE: When callback is registered, parameters given to func will be copied => data given via this call
//must be freed by caller!
int callback_reg(SMazBotcallback *cbdetails,int overwrite_defaultcb, void *opaque, size_t opaquesize);
*/
#endif //MAZBOT_v4_CALLBACKHANDLER_H


