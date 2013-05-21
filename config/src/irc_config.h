/* ********************************************************************
 *
 * @file irc_config.h
 * @brief MazBot generic config storage implementation.
 *
 *
 * -Revision History:
 *
 *  -0.0.3  15.03.2010/Maz  Splitted
 *  -0.0.2  10.03.2010/Maz  Added event structs
 *  -0.0.1  24.02.2010/Maz  First draft
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

#ifndef MBOT_IRC_CONFIG_H
#define MBOT_IRC_CONFIG_H
#include <generic.h>
#include <user.h>
#include <MbotBitset.h>
#include <callbackhandler.h>
#include "config_types.h"
#include "event_config.h"
#include "channel_config.h"
#include "server_config.h"
#include "mbot_pseudo_xml.h"

#define EVENTID_MAX 65535

struct SircConfig;
struct Sall_callbacks_list;
//static struct SmbotPseudoxmlTag;

typedef int (*IrcConfigReadF)(struct SircConfig *_this);


typedef int (*ircUserConfAddF)(void *_this_,SmbotPseudoxmlTag *userRootTag);
typedef int (*ircEventConfAddF)(struct SircConfig *_this,SmbotPseudoxmlTag *eventRootTag);
typedef int (*ircConfigGetServeramntF)(struct SircConfig *_this);
typedef SServerConfigs * (*ircConfigGetServerF)(struct SircConfig *_this, int serverno);
typedef int (*callbacklist_addF)(struct SircConfig *_this, unsigned int eventid,void *useropaque,ServerCallbackF cbf);
typedef struct Sall_callbacks_list
{
	unsigned int eventid;
	void *useropaque;
	ServerCallbackF cbf;
	struct Sall_callbacks_list *next;
}Sall_callbacks_list;

typedef ServerCallbackF (*get_cb_for_eventF)(struct SircConfig *_this,unsigned int eventId,void **usercbdata);

typedef struct SircConfig
{
	EcfgStructType type;
	Sall_callbacks_list *cblist;
	SMbotBitsSet *registered_cbids;
	int amntofservers;                  ///< amount of servers which configurations we have
	int amntofevents;					///< amount of events which configs we have
	Tircnick mynick;
    char *configfile;                   ///< name of the file where configs were read from
    SmbotPseudoxmlTag *tag;             ///< temporary storage for read (and preparsed) configurations @see SmbotPseudoxmlTag 
	SServerConfigs **servers;           ///< pointer to array of pointers to server structs holding all server specific configs.
	SServerEvents **events;             ///< struct containing global events 
    /**
     * @brief Pointer to function int IrcConfigRead(SServerConfigs *_this)
     *
     * Reads config file and updates temporary SmbotPseudoxml *tag storage struct which contains info for all configurations.
     * @param _this pointer to SServerConfigs struct returned by ircConfigInit.
     * @returns 0 on success, negative value upon error.
     */
	callbacklist_addF callbacklist_add;
	IrcConfigReadF IrcConfigRead;
	ircUserConfAddF ircUserConfAdd;
	ircEventConfAddF ircEventConfAdd;
	ircConfigGetServeramntF ircConfigGetServeramnt;
	ircConfigGetServerF ircConfigGetServer;
	get_cb_for_eventF get_cb_for_event;
}SircConfig;
/**
 * @brief Initializes irc config storage.
 *
 * @param configfile pointer to character array holding name of the file
 * @returns pointer to ready-for-use SircConfig struct, or NULL upon error.
 */
SircConfig *ircConfigInit(char *configfile);



#endif //MBOT_IRC_CONFIG_H
