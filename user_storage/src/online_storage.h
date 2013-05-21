#ifndef MBOT_ONLINE_STORAGE_H
#define MBOT_ONLINE_STORAGE_H

#include <definitions.h>
#include <generic.h>
#include <helpers.h>
#include <user.h>

#define ONLINEUSER_NICKTOINDEX(nick) (((int)(nick)[0])%USER_LIST_SIZE)

struct SMbotOnlineUserStorage;
typedef struct SMbotOnlineUsers
{
    char nick[IRC_NICK_MAX];
    char host[IRC_HOST_MAX];
	char passwd[IRC_PASSWORD_MAX];
	int level;
}SMbotOnlineUsers;

typedef int (*OnlineUserStorage_addF)(struct SMbotOnlineUserStorage *_this,char *nick, char *host, char *passwd,int lvl);
typedef int (*OnlineUserStorage_delF)(struct SMbotOnlineUserStorage *_this, char *nick);
typedef mbot_linkedList * (*OnlineUserStorage_elem_seekF)(struct SMbotOnlineUserStorage *_this, char *nick);
typedef SMbotOnlineUsers * (*OnlineUserStorage_seekF)(struct SMbotOnlineUserStorage *_this,char *nick);
typedef int (*nick_changeF)(struct SMbotOnlineUserStorage *_this, char *oldnick,char *newnick);

typedef struct SMbotOnlineUserStorage
{
	char							channelname[IRC_CHANNEL_MAX];
	char							server[IRC_SERVER_MAX];
    mbot_linkedList         		*onlineusers[USER_LIST_SIZE]; //SMbotOnlineUsers
    EuserIdentMode          		mode;
	OnlineUserStorage_addF			raw_add;
	OnlineUserStorage_delF			raw_del;
	OnlineUserStorage_seekF 		seek;
	OnlineUserStorage_elem_seekF 	elemseek;
	nick_changeF					nick_change;
}SMbotOnlineUserStorage;


SMbotOnlineUserStorage * OnlinestorageInit(char *server,char *channel);

#endif
