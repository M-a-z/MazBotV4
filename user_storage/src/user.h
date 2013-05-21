/* ********************************************************************
 *
 * @file user.h
 * @brief UserStorage definitions
 *
 *
 * -Revision History:
 *
 *  - 0.0.1 24.08.2009/Maz  First (non compiling) draft
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


#ifndef MAZ_BOT_V4_USER_H
#define MAZ_BOT_V4_USER_H

#include <irc_definitions.h>
#include <helpers.h>

//We're going to have 3 different type of user storages:
//THEPERMANENTUSERSTORAGE which is loaded to ram from file. It contains all channels etc where user has levels.
//THEEVENTUSERSTORAGE which is inside event struct, and which contains user levels required
//THEONLINEUSERSTORAGE which contains levels and channels for users who are online.

//On join event, THEPERMANENTUSERSTORAGE is scanned for matching user, and joined user is added to THEONLINEUSERSTORAGE if it is not there already. If join is done by bot itself, this scan is done for all online users. one THEONLINEUSERSTORAGE shall be created for each channel the bot joins.

//When userlevel is for user is changed by admin, THEPERMANENTUSERSTORAGE is updated on file, and THEONLINEUSERSTORAGE is scanned for matching user, and if such is found, the level is changed.

//When an event occurs, THEONLINEUSERSTORAGE is scanned by bot, and if THEEVENTUSERSTORAGE entry in event struct matches, then callback is called.

//With current config reader, only one userlevelcan be added / user /server. Eg. One can set 
//TODO: Bind chan and level together (Eg. struct {Elevel level,char *chan}), and change the userlevel parsing. Format of file can be:
//TODO: nick@mask/passwd:server:chan1;level1:chan2;level2:chan3;level3...

#define USER_LIST_SIZE 15

struct SIRCuserHandler;
struct SIRCuserHandlerPermanent;

typedef enum EuserHandlerType
{
	EuserHandlerType_Permanent 	= 0,
	EuserHandlerType_Online
}EuserHandlerType;

typedef enum EuserIdentMode
{
	EuserIdentMode_RegNick	= 0,
	EuserIdentMode_Passwd	= 1,
	EuserIdentMode_Hostmask	= 2,
	EuserIdentMode_NmbrOf   = 3
}EuserIdentMode;
typedef struct SuserChanLevelBondage
{
	EIRCuserLevel userlevel;
	char channel[IRC_CHANNEL_MAX];
}SuserChanLevelBondage;

typedef struct SuserStorageUser
{
	char nick[IRC_NICK_MAX];
	char host[IRC_HOST_MAX];
	char server[IRC_SERVER_MAX];
	mbot_linkedList *chanlevels; //onlinechans
	char password[IRC_PASSWORD_MAX];
//	EIRCuserLevel userlevel;
}SuserStorageUser;
 
typedef struct SircUser
{
	Tircnick nick;
	Tirchost host;
	Tircpass pass;
	EIRCuserLevel ulevel;
	EuserIdentMode identmode;
}SircUser;
/* This sets user to owner level - if password matches one set in config */
/* Owner is never added in permanent storage */
typedef int (*setOwnerF)(struct SIRCuserHandler *_this, char *nick,char *ownerpasswd);
typedef int (*isOwnerF)(struct SIRCuserHandler *_this, char *nick,char *ownerpasswd);
/* Adder/Changer/Remover level validation should be done before calling these! */
typedef int (*addUserByNickHostF)(struct SIRCuserHandler *_this, char *nickhost_toadd,char *server,mbot_linkedList *lvlchan);
typedef int (*addUserByNickPasswdF)(struct SIRCuserHandler *_this,char *nickhost, char *server,mbot_linkedList *lvlchan, char *passwd);
//TODO: Check if these remove user totally, or does they just remove user from one channel?
//If they remove user completely, then we need a remove a channel from user function.
typedef int(*removeUserByNickHostF)(struct SIRCuserHandler *_this,char *nickhost, char *server,char *chan);
typedef int (*removeUserByNickPasswdF)(struct SIRCuserHandler *_this,char *nickhost,char *server,char *chan, char *passwd);
typedef int(*levelChangeByNickHostF)(struct SIRCuserHandler *_this, char *nickhost_toachange, char *server, SuserChanLevelBondage chanlevel);
typedef int(*levelChangeByPasswd)(struct SIRCuserHandler *_this, char *nickhost_toachange, char *server, SuserChanLevelBondage chanlevel,char *passwd);
//Will find user based on server, nick, nick&host or nick & passwd depending on identification mode. Returns user info in SuserStorageUser *, NULL on error
typedef SuserStorageUser * (*findUserFromPermStorageF)(struct SIRCuserHandler *_this,char *nickhost,char *server, char *passwd);
typedef struct SIRCuserHandler *(*userhandlerInitF)(EuserHandlerType type, EuserIdentMode mode);
typedef struct SIRCuserHandler
{
	EuserHandlerType 		type;
//	char					channelname[IRC_CHANNEL_MAX];
//	char					server[IRC_SERVER_MAX];
	char					ownernick[IRC_NICK_MAX];
	char					ownerpass[IRC_PASSWORD_MAX];
    mbot_linkedList 		*user[USER_LIST_SIZE]; //Contains  structs SuserStorageUser
	EuserIdentMode	 		mode;

					//These functions need to do checks for dublicates!
	addUserByNickHostF 		useraddby_host;
					//These functions need to do checks for dublicates!
	addUserByNickPasswdF    useraddby_passwd;
	removeUserByNickHostF   userdelby_host;
	removeUserByNickPasswdF userdelby_passwd;
	levelChangeByNickHostF  userchangeby_host;
	levelChangeByPasswd     userchangeby_passwd;
	setOwnerF				set_owner;
	isOwnerF				is_owner;
}SIRCuserHandler;

typedef int (*SIRCuserHandlerPermSyncToFileF)(struct SIRCuserHandlerPermanent *);
typedef int (*SIRCuserHandlerPermSyncFromFileF)(struct SIRCuserHandlerPermanent *);
typedef struct SIRCuserHandlerPermanent
{
	SIRCuserHandler                     handler;
	char                                *cfgfilename;
	//This needs to perform checks for dublicates?
	SIRCuserHandlerPermSyncToFileF      syncToFile;
	SIRCuserHandlerPermSyncFromFileF    syncFromFile;
    findUserFromPermStorageF            findUser;
}SIRCuserHandlerPermanent;
SIRCuserHandler *PermUserStorageInit(EuserHandlerType type, EuserIdentMode mode);
SIRCuserHandler *TempUserStorageInit(EuserHandlerType type, EuserIdentMode mode);

SIRCuserHandler *StorageInit(EuserHandlerType type, EuserIdentMode mode);

extern SIRCuserHandlerPermanent *G_permanent_user_storage;
extern MbotAtomic32 * perm_storage_inited;
//Funcs needed:

//Depends on server where connected
//Or is it better to have one user storage/server? Propably yes.
//find_if_regged();

//hostmask_to_user();
//requires command from server, user / chan filter from events and userdata
//Users will be in mbot_linkedList *user[11]; datastructure as follows:
//10 first slots are reserved for normal users, 11'th slot is for servers/chans (if I ever need to keep track of them?)
//user is assigned to slot 0-9 based on the second letter of nick@mask%10
//this is done in order to speed the lookups based on nick@host, and second letter because first is way too often the tilde.
//event_matchto_user();

#endif //MAZ_BOT_V4_USER_H
