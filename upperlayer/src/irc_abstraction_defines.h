/* ********************************************************************
 *
 * @file irc_abstraction_defines.h
 * @brief MazBot logical layer definitions.
 *
 *
 * -Revision History:
 *
 *  -0.0.2  20.06.2010/Maz  Renamed
 *  -0.0.1  ??.05.2010/Maz  First draft
 *
 *
 *  Lisence info: You're allowed to use / modify this - you're only required to
 *  write me a short note and your opinion about this to Mazziesaccount@gmail.com.
 *  if you redistribute this you're required to mention the original author:
 *  "Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
 *  channel.
 *
 *
 *  PasteLeft 2010 Maz.
 * ********************************************************************/



#ifndef IRC_ABSTRACTION_DEFINES_H
#define IRC_ABSTRACTION_DEFINES_H

#define SERVERTASK_MAX 255
//Note, this bitmask must be continuous
#define TX_QUEUE_SIZE 1023
#include <generic.h>
#include <MbotRingBuff.h>
#include <time.h>
#include <stdio.h>
#include <networking.h>
#include <online_storage.h>
#include <parser.h>
#include <irc_protocol_parser.h>
#include <event_storage.h>
#include <irc_config.h>
struct Sirc_servers;
struct Sirc_channels;
#include<internal_callbacks.h>
//This sets a limit to amount of simultaneous callback functions which require RX.
#define MAX_RX_QUEUES 50

#define USERLIST_USER "352"
#define UNABLE_TO_JOIN "405"
#define END_OF_MOTD "376"
//#include "irc_abstraction_defines.h"
//#include "irc_channel_abstraction.h"

/* Used when handling commands from server */
#define  known_commands_amnt 3
extern const char *known_commands[known_commands_amnt];
typedef enum EupperlayerStructType
{
    EupperlayerStructType_Server = 0,
    EupperlayerStructType_Channel = 1
}EupperlayerStructType;
typedef enum EServerSuccess
{
	EServerSuccess_Ok = 0,
	EServerSuccess_Disconnected = -1,
	EServerSuccess_FuckdUp		= -2
}EServerSuccess;

typedef enum ETimedEvent
{
	ETimedEvent_ServerReconnTimer	= 0,
	ETimedEvent_ChannelReconnTimer,
	ETimedEvent_NmbrOf
}ETimedEvent;

typedef enum EServerTasks
{
	EServerTasks_JoinServer	= 0,
	EServerTasks_InitParser,
	EServerTasks_InitChannels, //unnecessary?
	EServerTasks_InitUserCallbacks,
	EServerTasks_PollTimedEvents,	//Reconnects etc.
	EServerTasks_CheckInternalCallbacks, //like whois, names lists...
	EServerTasks_InspectCallbacks, //usercallbacks
	EServerTasks_CheckUserCallbackRequests,
}EServerTasks;

typedef struct STimedEvents
{
	ETimedEvent eventtype;
	struct timespec exptime;
//	Tircchan	channel;
}STimedEvents;
/*
 * Each server shall contain it's own
 * Channels (which contain userlists)
 * IrcProtocolParser
 * IrcConnection
 * PhpConnection
 * User callback list (later... now we just use slow global list due to planning mistake :])
 * Logger (TODO:)
 * Server shall be working as kind of state machine.
 * When ever there is a task to do, like initializing some lists, some info is waited, callbacks need to be checked etc
 * a flag is set for this task. A server shall have a do_mainloop() function, which shall be executed once/iteration.
 * It will investigate this state array, and if some task needs to be done, server shall call appropriate function.
 * When each flag is inspected, do_mainloop shall return with server's error status.
 * Possible statuses are, Ok, disconnected, or fatal error.
 * Ok and disconnect means no problem, or a problem which can be internally handled. But if
 * disconnected state will pop in continuously, it may be a good idea to skip this server's mainloop for a few iterations
 * (some minutes??)
 *
 * If last server shall report disconnected state for a while, then we should do what?? Keep trying or just give up?
 */

/* Move these typedefs to some header */
//#ifndef IRC_SERVER_ABSTRACTION_H
//#define IRC_SERVER_ABSTRACTION_H

#define srv_known_inter_cmm_amnt 4

typedef int (*GetChansF)(struct Sirc_servers *);
typedef void (*ServerDisconnectedCleanupF)(struct Sirc_servers *);
struct Sirc_channels;
typedef void (*ChannelDisconnectedCleanupF)(struct Sirc_channels *);
typedef void (*ServerArmReconnTimerF)(struct Sirc_servers *);
typedef void (*ChannelArmReconnTimerF)(struct Sirc_channels *);
typedef int (*TxQueueFlushF)(struct Sirc_servers *_this);
typedef int (*IRCsendF)(struct Sirc_servers *_this,size_t sendsize, char *senddata);

typedef struct SMbot_rx_queues
{
	unsigned int queue_id;
	Mbot_buffer buff;
}SMbot_rx_queues;
SMbot_rx_queues *queueInit(void);

typedef int (*txQueueCreate)(struct Sirc_servers *_this);

typedef struct SUpperlayerIrcMsg
{
	size_t sendsize;
	char *msg;
}SUpperlayerIrcMsg;

/* This will be the main func in server. It shall
 * investigate server/chan state (things to do, pending replies, callbacks etc.)
 * and perform calls to appropriate funcs when such a thing is needed. 
 * After checking each conditions once, it shall return with status.
 * Status shall be 0, if everything is fine, Edisconnected if server is disconnected, 
 * and Efuckdup if something is really bad wrong as Kari says. 
 * */

typedef EServerSuccess (*server_doloop)(struct Sirc_servers*);
/* I want to avoid multithreading as long as possible. Hence this poll 
 * This should be called with decent intervals, 
 * and it should loop through timed events list to see 
 * if some event should be executed. 
 *
 * (Like reconnect timer)
 * We can give the timed event we're interested in, or
 * ETimedEvent_NmbrOf if we want to go through all the possible events.
 *
 * Actually, server struct needs to be a "state machine", Eg. it should have a "todo" list, which will cause calls to correct funcs.
 * */
typedef int (*queue_callbacksF)(struct Sirc_servers *_this,SServerEvents *evnt,SIRCparserResult *res);
typedef SircUser * (*static_server_user_findF)(struct Sirc_servers *_this,char *nick);
typedef EMbotCallbackRet (*srvparsecommandF)(struct Sirc_servers *_this,SIRCparserResult *res);
typedef EServerSuccess (*process_dataF)(struct Sirc_servers *_this);
typedef int (*ServerConnectF)(struct Sirc_servers *_this);
typedef int (*negotiate_server_connF)(struct Sirc_servers *_this);
/* Old funcs below, see what is usable && get rid of the rest */
//do we need separate Chanadd_channel_def_eventsF - yes
typedef void (*PollTimedEventsF)( void *,ETimedEvent); //Either Sirc_servers or Sirc_channels - depending on if we poll server or chann queue
typedef int (*add_channel_def_eventsF)(struct Sirc_servers *_this,char *chann,FILE *readfile);
typedef int (*Chanadd_channel_def_eventsF)(struct Sirc_channels *_this,FILE *readfile);
typedef int (*find_chan_noF)(struct Sirc_servers *_this,char *chann);
//typedef int (*sendtoircF)(struct Sirc_servers *_this,char *tgt,char *txt);
/* List structure would be more flexible, but static array will be faster.
typedef struct SServerCallbackList
{
	SServerCallback *callback;
	SServerCallbackArgs args;
	SServerCallbackList *next;
}
typedef struct SServerCallbackList
{
	SServerCallback *callback;
	SServerCallbackArgs args;
}SServerCallbackList;
*/



typedef struct Sirc_servers
{
    EupperlayerStructType mytype;
	unsigned int srv_connected;
	STimedEvents *timed_events_pending[ETimedEvent_NmbrOf];
	SServerConfigs *mycfg;
	Sconn *connection;
	Tircserver servername;
	Sparser *ircparser;

	int amntofchannels;
	int amntofevents;

	SMbot_rx_queues *rx_queues[MAX_RX_QUEUES];
	Mbot_buffer *tx_queue;

	// Do we want to have ptrs, or ptrptrs?
	struct Sirc_channels **channels;
	SServerCallbackList *servertasklistptr[SERVERTASK_MAX];
	SServerCallbackList servertasklist[SERVERTASK_MAX];
	SMbotOnlineUserStorage *online_users;
	srvparsecommandF srvparsecommand;
	IRCsendF         IRCsend;
	TxQueueFlushF TxQueueFlush;
	
	ServerConnectF ServerConnect;
	negotiate_server_connF negotiate_server_conn;
	process_dataF process_data;
    static_server_user_findF static_server_user_find;
	queue_callbacksF queue_callbacks;

	GetChansF GetChannelsToJoin;
	ServerDisconnectedCleanupF disconnected_flush;
	ServerArmReconnTimerF arm_reconn_timer;
	add_channel_def_eventsF add_channel_def_events;
	PollTimedEventsF pollEvents;
	find_chan_noF find_chan_no;
}Sirc_servers;
/* TODO: Do this */
Sirc_servers *InitSirc_server(SServerConfigs *servercfg);

#define chan_known_inter_subcomm_pm_amnt 2
#define chan_known_inter_cmm_amnt 4
typedef int (*checkjoinstateF)(struct Sirc_channels *_this);
typedef SircUser * (*static_channel_user_findF)(struct Sirc_channels *_this,char *nick);
//TODO: DO this too :)
typedef EMbotCallbackRet (*parsecommandF)(struct Sirc_channels *_this,SIRCparserResult *res);
//TODO: Do this. - zero the struct, set the channelname, chanId?
struct Sirc_channels *initSirc_Sirc_channels(SChannelConfigs *cfg,Sirc_servers *myserv);
//TODO: Do this. - Initialize the online storage inside channel struct
int initSirc_channels_storage(/* params to decide */);
typedef int (*Channel_joinF)(struct Sirc_channels *,Sconn *,Sparser *);
//typedef int 
typedef struct Sirc_channels
{
    EupperlayerStructType mytype;
	unsigned int chan_joined;
	STimedEvents timed_events_pending[ETimedEvent_NmbrOf];
	SChannelConfigs *mycfg;
	Sirc_servers *myserv;
	Tircserver server;
	Tircchan channel;
	SMbotOnlineUserStorage *chan_online_users;
	parsecommandF parsecommand;
    static_channel_user_findF static_channel_user_find;
	checkjoinstateF checkjoinstate;

	ChannelDisconnectedCleanupF disconnected_flush;
	ChannelArmReconnTimerF arm_reconn_timer;
	SMbotEventStorage *events;
	Chanadd_channel_def_eventsF add_channel_def_events;
	Channel_joinF irc_channel_join;
}Sirc_channels;

static __inline__ int is_required_level_found(SServerEvents *evnt, SMbotOnlineUserStorage *storage, char *nick)
{
	SMbotOnlineUsers *usr;
	int lvlindex;
	if(evnt->levelamnt>0)
	{
		for(lvlindex=0;lvlindex<evnt->levelamnt;lvlindex++)
		{
			if(evnt->levellist[lvlindex]==EIRCuserLevel_guest)
				return 1;
		}
		if(NULL!=(usr=storage->seek(storage,nick)))
		{
			for(lvlindex=0;lvlindex<evnt->levelamnt;lvlindex++)
			{
				if(evnt->levellist[lvlindex]==usr->level)
				{
					return 1;
				}
			}
		}
		return 0;
	}
	else
		return 1;
}



//#endif
#endif //IRC_ABSTRACTION_DEFINES_H

