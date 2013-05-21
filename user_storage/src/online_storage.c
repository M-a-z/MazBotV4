/* ******************************************************************************************* */
/*
 * This is a though thing. I'll try to write down some points to clear my mind.
 *
 * 1. Users must be identificated to limit certain rights to some users only.
 * 		*There will be 3 possible modes for doing the identification.
 * 		*1 Identification based on registering nick to server
 * 		*2 Identification based on nick@hostmask
 * 		*3 Identification based on registering nick to bot with password
 *
 * 		1. When user joins to a channel / bot joins, bot performs whois query
 * 		And sees if user is registered based on the whois reply (*See difficulties this causes)
 * 		owner/admin level users can give higher levels to other registered nicks.
 *
 * 		2. owner/admin level users can give levels to other users. Bot will examine user's nick@hostmask
 * 		and maintains a text file about these levels. Owner will be configured in main config file.
 *
 * 		3. Users can register their nicks to bot (with password). When user joins to a channel, bot
 * 		asks for password to see if user has registered. (see difficulties). Owner/admin level
 * 		users can give levels to registered users when they're online. Nick/password pairs are 
 * 		maintained in text file.
 *
 * 		Userlevels shall be server and channel specific. User identification info is server specific.
 * 		Nicks must be unique inside a server.
 *
 * 		The 1 and 3 will cause some problems (especially 3), because when user joins on channel, he may
 * 		not have identified. So bot cannot know right away if user has registered. So callbacks requiring some
 * 		level information cannot be reliably executed before bot has tried to identificate user, and before
 * 		certain amount of time has been given for a user to prove his identity. This sucks.
 *
 * 		Possible solutions: 
 * 		1. Define timelimit (for example 30 seconds) for bot to retry identification.
 * 		2. Allow user to send specific message to bot, to ask for re-evaluation of identification state.
 *
 * 		NOTE: If bot is protecting for some channel (only users having certain level can be online),
 * 		disconnect - reconnect will cause idle users to be kicked after the timelimit has expired (or instantly
 * 		if kicking is bound to on join event.) => There really should be some way to define delay in callbacks - 
 * 		as well as request confirmation for user's userlevel. So user storage should be accessible from callbacks.
 *
 * 		Also callbacks should be able to query some data from IRC server straight away - which is hard to do.
 * 		Perhaps there should be a hook in networking obj, in which callback could register a package grapper, Eg.
 * 		Networking obj could be asked to deliver copies of each IRC server mesage to callback. A ring buffer which 
 * 		was constantly updated, and which could be read by somefunc, starting at someaddr (someaddr would be given
 * 		to callback when callback requested IRC server messages) might be sufficient. But callbacks should also be able to
 * 		write data to IRC server... This really sucks. Fortunately this is not a topic of user data storage :)
 *
 * 		Basically user data storage must be divided in 2 parts. 1 for handling the file which maintains user info for
 * 		all servers/channels. File should be read at startup, and info stored on RAM to speed up the access. File would
 * 		be then updated when a level is added/removed for/from user. This is implemented in permanent_storage.c Note, 
 * 		permanent storage has no intelligence whatsoever. It won't check if adder/remover have correct rights etc. Only
 * 		check it performs is that there won't be dublicate nicks. All privilege checks should be done on online_storage level.
 *
 * 		Permanent storage cannot be called straight - all calls will be made via online storage.
 *
 * 		Second part should be channel specific. Eg. There should be a storage for user info regarding each known user online -
 * 		different storage for each channel. When bot joins at channel (or user joins on channel), a whois is performed, and
 * 		online user info is compared to info in permanent_storage. If user info is found, a level will be assigned to user. 
 * 		When user parts, online info is updated accordingly. When level is added, online and permanent storage are upated.
 * 		Same when user level is removed/changed.
 *		
 *		So online user storage needs at least following apis: (pseudocode)
 *		InitStorage(identification mode) 
 *		//Initialize storage for use
 *		SetChannelName(channame)
 *		//Binds the online storage to certain channel.
 *		//This could be done at init - unless the init interface was meant to be identical to permanent storage IF
 *		//May be that the online storage needs to be totally separated from perm storage IF.
 *		InspectAndAddNewUser(userdetails) 
 *		//New user joined, check perm storage for match and update online storage if necessary
 *		AddNewUser(userdetails,adderdetails) 
 *		//Check adder level, if admin/owner => query perm storage to see if new user already exists.
 *		//If user exists see that level is smaller than owner - if not, reject request., see if level change was requested. 
 *		//If so, update permanent storage.
 *		//Check if user is online, and if so, update online storage.
 *		//If user does not exist, add new user to perm and online storage.
 *		//In other cases, reject add request.
 *		RemoveUser(userdetails,removerdetails)
 *		//Check if remover has owner/admin level. If yes, check if user to be removed exists. If so, check the user to be 
 *		//removed has no owner level. If it has => reject request remove user from both storages.
 *		ReEvaluateUser(s) (userdetails/allusers)
 *		//Force bot to do the re-evaluation
 *		getUserLevel(userdetails)
 *		//Get user's level. channel and nick should be enough??
 *
 *		OKAY: FINAL DECISION => Create wholly new storage class for online storage. Rationale:
 *		1. Function IF with perm and online storages need to be different
 *		2. server and channel name info are not needed for each user in online storage (they're same for all since own storage
 *		is created for each channel.)
 *		3. (Most heavy reason), mbot_linkedList listing levels for each channel for each user is not clever on online storage.
 *
 *		=> When online storage updates info on permanent storage, it will only update info regarding the channel it handles.
 *		XXX: How to allow owner to change/remove user info for all channels while staying online? (Online storage cannot know
 *		what are the already set levels on other channels (unless it queries the old info first and then updates only it's own
 *		channel's info => this may be the best approach)
 */
 /* ************************************************************************************************************************** */



#include <user.h>
#include <online_storage.h>
//This func is done to scan user info from permanent storage, and assign a level to user if such exists. (Add user to online stor)
//Will be called when user / bot joins to a chan - in regged mode, after whois is done.
//No checks at this point (this is initial add, not level changing add/add by poweruser)
static int OnlineUserStorage_add(SMbotOnlineUserStorage *_this,char *nick, char *host, char *passwd,int lvl);
static int OnlineUserStorage_del(SMbotOnlineUserStorage *_this, char *nick);
static int nick_change(SMbotOnlineUserStorage *_this, char *oldnick,char *newnick);
static SMbotOnlineUsers * OnlineUserStorage_seek(SMbotOnlineUserStorage *_this,char *nick);
static mbot_linkedList *OnlineUserStorage_elem_seek(SMbotOnlineUserStorage *_this,char *nick);

SMbotOnlineUserStorage * OnlinestorageInit(char *server,char *channel)
{
	int cpylen;
	int i,j;
	SMbotOnlineUserStorage *_this;
	if(NULL==server || NULL==channel)
	{
		EPRINT("NULL server/channel name given in OnlinestorageInit()!");
		return NULL;
	}
	_this = malloc(sizeof(SMbotOnlineUserStorage));
	if(NULL==_this)
	{
		PPRINT("malloc() FAILED at %s:%d",__FILE__,__LINE__);
		return NULL;
	}
	memset(_this,0,sizeof(SMbotOnlineUserStorage));
	for(i=0;i<USER_LIST_SIZE;i++)
	{
		if(NULL==(_this->onlineusers[i] = mbot_ll_init()))
		{
			PPRINT("mbot_ll_init() FAILED at %s:%d",__FILE__,__LINE__);
			for(j=0;j<i;j++)
			{
				mbot_ll_destroy(&(_this->onlineusers[j]));
			}
			free(_this);
			return NULL;
		}
	}
	if(IRC_CHANNEL_MAX<(cpylen=snprintf(_this->channelname,IRC_CHANNEL_MAX,"%s",channel)) || 0>=cpylen)
	{
		PPRINT("mbot_ll_init() FAILED at %s:%d",__FILE__,__LINE__);
		free(_this);
		return NULL;	
	}
	if(IRC_SERVER_MAX<(cpylen=snprintf(_this->server,IRC_SERVER_MAX,"%s",server)) || 0>=cpylen)
	{
		PPRINT("mbot_ll_init() FAILED at %s:%d",__FILE__,__LINE__);
		free(_this);
		return NULL;
	}
	/* Set funcptrs */
	_this->raw_add=&OnlineUserStorage_add;
	_this->raw_del=&OnlineUserStorage_del;
	_this->seek=&OnlineUserStorage_seek;
	_this->elemseek=&OnlineUserStorage_elem_seek;
	_this->nick_change=&nick_change;
	return _this;
}
static mbot_linkedList *OnlineUserStorage_elem_seek(SMbotOnlineUserStorage *_this,char *nick)
{
	mbot_linkedList *elem;
	SMbotOnlineUsers *user=NULL;
	int userindex;
	if(NULL==_this||NULL==nick)
	{
		EPRINT("NULL param given to OnlineUserStorage_elem_seek()!");
		return NULL;
	}
	userindex=ONLINEUSER_NICKTOINDEX(nick);
	if(NULL==_this->onlineusers[userindex])
	{
		DPRINT("OnlineUserStorage_elem_seek(): No match found for nick %s (no nicks starting with %c)",nick,nick[0]);
		return NULL;
	}
	elem=mbot_ll_get_first(_this->onlineusers[userindex]);
	while(NULL!=elem && NULL!=(user=mbot_ll_dataGet(elem)))
	{
		if(!strcmp(user->nick,nick))
		{
			DPRINT("OnlineUserStorage_elem_seek(): elem found for user %s",nick);
			return elem;
		}
		elem=mbot_ll_get_next(elem);
	}
	DPRINT("OnlineUserStorage_elem_seek(): No match found for nick %s",nick);
	return NULL;
}
static int OnlineUserStorage_add(SMbotOnlineUserStorage *_this,char *nick, char *host, char *passwd,int lvl)
{
	SMbotOnlineUsers *user;
	if(NULL==_this||NULL==nick||NULL==host)
	{
		EPRINT("NULL param given to OnlineUserStorage_add()!");
		return -1;
	}
	if(strlen(nick)>=IRC_NICK_MAX || strlen(host)>=IRC_HOST_MAX || strlen(passwd) >= IRC_PASSWORD_MAX)
	{
		EPRINT("Ill sized nick, host or passwd given to OnlineUserStorage_add() => rejecting user %s",nick);
		return -1;
	}
	if(NULL==(user=_this->seek(_this,nick)))
	{
		/* Add new */
		DPRINT("User not found from storage => adding");
		user=malloc(sizeof(SMbotOnlineUsers));
		if(NULL==user)
		{
			PPRINT("malloc() FAILED at %s:%d",__FILE__,__LINE__);
			return -1;
		}
		memset(user,0,sizeof(SMbotOnlineUsers));
		strcpy(user->nick,nick);
		strcpy(user->host,host);
		strcpy(user->passwd,passwd);
		user->level=lvl;
		if(NULL==mbot_ll_add(_this->onlineusers[((int)*nick)%USER_LIST_SIZE],user))
		{
			EPRINT("user adding FAILED at %s:%d",__FILE__,__LINE__);
			return -1;
		}
	}// Add new
	else
	{
		/* update existing */
		DPRINT("Updating existing user %s",nick);
		if(_this->mode == EuserIdentMode_Passwd)
		{
			if(0!=strcmp(passwd,user->passwd))
			{
				memset(user->passwd,0,IRC_PASSWORD_MAX);
				strcpy(user->passwd,passwd);
			}
		} else if(_this->mode == EuserIdentMode_Hostmask)
		{
			if(0!=strcmp(user->host,host))
			{
				memset(user->host,0,IRC_HOST_MAX);
				strcpy(user->host,host);
			}
		}
		user->level=lvl;
	}//update existing
	DPRINT("Online user data updated for %s, lvl %d",nick,lvl);
	return 0;
}
static int nick_change(SMbotOnlineUserStorage *_this, char *oldnick,char *newnick)
{
	SMbotOnlineUsers *user;
	SMbotOnlineUsers *user2;
	if(NULL==_this||NULL==oldnick||NULL==newnick)
	{
		EPRINT("NULL param given to %s()!",__FUNCTION__);
		return -1;
    }
    if(strlen(newnick)>=IRC_NICK_MAX)
    {
        EPRINT("Ill sized newnick %s() => rejecting change (olduser %s, newuser %s)",__FUNCTION__,oldnick,newnick);
        return -1;
    }
	if(NULL==(user=_this->seek(_this,oldnick)))
    {
		WPRINT("%s(): Cannot change nick for user %s, no such user in storage!",__FUNCTION__,oldnick);
        return -1;
	}
	if(newnick[0]==oldnick[0])
	{
		memset(user->nick,0,IRC_NICK_MAX);
		memcpy(user->nick,newnick,strlen(newnick)+1);
	}
	else
	{
		if(_this->raw_add(_this,newnick, user->host, user->passwd,user->level))
		{
			EPRINT("%s(): Failed to update nick %s => %s, adding new user did not work!",__FUNCTION__,oldnick,newnick);
			_this->raw_del(_this,oldnick);
			return -1;
		}
		if(_this->raw_del(_this,oldnick))
		{
			EPRINT("%s(): Failed to update nick %s => %s, adding new user deletion did not work!",__FUNCTION__,oldnick,newnick);
			return -1;
		}
	}
	return 0;
}
static int OnlineUserStorage_del(SMbotOnlineUserStorage *_this, char *nick)
{
	mbot_linkedList* elem;
	if(NULL==(elem=_this->elemseek(_this,nick)))
	{
		WPRINT("OnlineUserStorage_del(): Cannot delete %s, no such user!",nick);
	}
	else
	{
		DPRINT("OnlineUserStorage_del():Deleting user %s",nick);
		elem=mbot_ll_release(elem);
		MAZZERT(NULL!=elem,"WTF just happened?");
		free(mbot_ll_dataGet(elem));
		free(elem);
	}
	return 0;
}
static SMbotOnlineUsers * OnlineUserStorage_seek(SMbotOnlineUserStorage *_this,char *nick)
{
	mbot_linkedList* elem;
	SMbotOnlineUsers *user=NULL;
	if(NULL!=(elem=_this->elemseek(_this,nick)))
	{
		user=mbot_ll_dataGet(elem);
	}
	return user;
}
/*
 int TempUserStorageInspectAndAddNewUser(SIRCuserHandler *_this,char *nickhost_toadd,char *passwd,int isregged)
{
	SuserStorageUser *usertoadd;
	SIRCuserHandlerPermanent *permstorage = IRC_USERSTORAGE_GET_PERMANENT();
	//Just check perm storage for matching user.
	
	if(NULL!=(usertoadd=permstorage->findUser(nickhost_toadd,_this->server,passwd)))
	{
		//Add info found from permstorage
		if(NULL==mbot_ll_add(_this->user[GETUSERINDEX(nickhost_toaddnickhost_toadd)],usertoadd))
		{
			EPRINT("Could not add user %s to storage! %s:%d",__FILE__:__LINE__);
			return -1;
		}
		DPRINT("userinfo for %s found from permanent storage, and added to online storage with level %d",nickhost_toadd,
	}
	else
	{
		//Add with default settings
	}
		
}
//This function is done to add a new user by request of an admin/owner. (How about registering a nick to bot with default level in passwd mode?)
int TempUserStorageAddNewUser(SIRCuserHandler *_this,char *nickhost_toadd,newlevel,char *passwd,int isregged,char *nickhost_adder)
{
	adderlevel
	oldlevel;
	SIRCuserHandlerPermanent *permstorage = IRC_USERSTORAGE_GET_PERMANENT();
	//Query Temp storage to see if adder has correct level. (On this channel!)
	if(ADMIN<=(adderlevel=_this->getUserLevel(nickhost_adder)))
	{
		oldlevel=_this->getUserLevel(nickhost_toadd);
		//Allright, adder hasda power!
		if(NULL==oldlevel)
		{
			//Really new user => query perm storage just in case. If nick does not exist already => add.
			
		}
		if
		(
			adderlevel<= oldlevel &&
			newlevel <= oldlevel
		)
		{
			//No change to levels, or lower (or same) level user tries to downgrade higher level user
			return -1;
		}
		else
		{
	}
}

int TempUserStorageSetChannelAndServerName(SIRCuserHandler *_this,char *channame,char *server)
{
	size_t chanlen,servlen;
	MAZZERT(NULL!=channame && NULL!=server,"NULL chan/server name!");
	if((chanlen=strlen(channame))>=IRC_CHANNEL_MAX || IRC_SERVER_MAX <= (servlen=strlen(server)))
	{
		EPRINT("Channel name \"%s\" or server name \"%s\" too long (chan%u serv%u bytes, max chan%u serv%u bytes)",channame,server,chanlen,servlen,IRC_CHANNEL_MAX,IRC_SERVER_MAX);
		return -1;
	}
	strcpy(_this->channelname,channame);
	strcpy(_this->server,server);
	return 0;
}
*/
/*
SIRCuserHandler *TempUserStorageInit(EuserHandlerType type, EuserIdentMode mode)
{
	SIRCuserHandler *_this;
	int i,j;
//	size_t cfgnamelen;
	_this=malloc(sizeof(SIRCuserHandler));
	if(NULL==_this)
	{
		PPRINT("Malloc FAILED at InitTempUserStorage()");
		return NULL;
	}
	for(i=0;i<USER_LIST_SIZE;i++)
	{
		_this->user[i]=mbot_ll_init();
		if(NULL==_this->user[i])
		{
			PPRINT("InitTempUserStorage(): mbot_ll_init() FAILED!");
			for(j=0;j<i;j++)
			{
				mbot_ll_destroy(&(_this->user[j]));
			}
			free(_this);
			return NULL;
		}
	}
	_this->mode=mode;
	DPRINT("InitTempUserStorage(): Allocations done");
	_this->type=EuserHandlerType_Online;
*/
	/* Fill handler funcs */
/*
 * _this->useraddby_host		= &TempUserStorageAddByHost;
	_this->useraddby_passwd		= &TempUserStorageAddByPasswd;
	_this->userdelby_host		= &TempUserStorageDelByHost;
	_this->userdelby_passwd		= &TempUserStorageDelByPasswd;
	_this->userchangeby_host	= &TempUserStorageChangeByHost;
	_this->userchangeby_passwd	= &TempUserStorageChangeByPasswd;
	_this->set_owner			= &UserStorageSetOwner;
	_this->is_owner				= &UserStorageIsOwner;
*/
/* Fillpermanent specific funcs. */
/*
	DPRINT("Temp user storage initialized and loaded!");
	return _this;
}
*/
