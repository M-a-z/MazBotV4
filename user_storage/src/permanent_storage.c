/* ********************************************************************
 *
 * @file permanent_storage.c
 * @brief Functions used to handle user storage
 * TODO: Think ponder and so on, how to verify that one who initiates
 * change for levels/users is authorized to do so?
 *
 * -Revision History:
 *	- 0.0.3  26.08.2009/Maz  Added find_user and find_channel helperfuncs.
 *							 Added del functions
 *							 Modified add functions to use helpers.
 *							 Added Changefunctions.
 * 	- 0.0.2  24.08.2009/Maz  Added First draft of PermUserStorageSyncToFile()
 *   						And some other stuff :)
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

#include <user.h>
#include <errno.h>
#include <generic.h>
#include <helpers.h>
#include <config.h>

//This global is bad for protection, it can only be used to avoid double inits.
//TODO: Change to mbot_atomic - DONE
MbotAtomic32 * perm_storage_inited=NULL;
SIRCuserHandlerPermanent *G_permanent_user_storage;
/* 
 * Allocate storage struct, initialize function ptrs.
 * Read permanent config file && update data accordingly
 */

static int PermUserStorageSyncFromFile(SIRCuserHandlerPermanent *_this);
static int PermUserStorageSyncToFile(SIRCuserHandlerPermanent *_this);
static mbot_linkedList * AddNewChans(mbot_linkedList *oldlist,mbot_linkedList *newlist);
static mbot_linkedList * find_user(SIRCuserHandler *_this,char *nickhost, char *server, char *passwd,int verifypasswd);
static mbot_linkedList * find_chanlvl(SIRCuserHandler *_this,mbot_linkedList *userlistitem,char *chan);
static int TempUserStorageDelByHost(SIRCuserHandler *_this,char *nickhost, char *server,char *chan);
static int TempUserStorageDel(SIRCuserHandler *_this,char *nickhost, char *server,char *chan,char *password);
static int TempUserStorageDelByPasswd(SIRCuserHandler *_this,char *nickhost,char *server,char *chan, char *passwd);
static int TempUserStorageAddByHost(SIRCuserHandler *_this, char *nickhost_toadd,char *server,mbot_linkedList *lvlchan);
static int TempUserStorageAddByPasswd(SIRCuserHandler *_this,char *nickhost, char *server,mbot_linkedList *lvlchan, char *passwd);
static int TempUserStorageDelByHost(SIRCuserHandler *_this,char *nickhost, char *server,char *chan);
static int TempUserStorageChangeByPasswd(SIRCuserHandler *_this, char *nickhost,char *server, SuserChanLevelBondage chanlevel,char *passwd);
static int TempUserStorageChange(SIRCuserHandler *_this, char *nickhost,char *server, SuserChanLevelBondage chanlevel,char *password);
static int TempUserStorageChangeByHost(SIRCuserHandler *_this, char *nickhost,char *server, SuserChanLevelBondage chanlevel);
/*
static int UserStorageIsOwner(SIRCuserHandler *_this, char *nick,char *ownerpasswd);
static int UserStorageSetOwner(SIRCuserHandler *_this, char *nick,char *ownerpasswd);
*/
//The Init function is moved to own file => these must be public funcs.
//static SIRCuserHandler *PermUserStorageInit(EuserHandlerType type, EuserIdentMode mode);
//static SIRCuserHandler *TempUserStorageInit(EuserHandlerType type, EuserIdentMode mode);

SIRCuserHandler *StorageInit(EuserHandlerType type, EuserIdentMode mode)
{
	switch(type)
	{
		case EuserHandlerType_Permanent:
			return (SIRCuserHandler *)PermUserStorageInit(type,mode);
			break;
		case EuserHandlerType_Online:
			MAZZERT(0,"TempUserSorage separated from perm storage! This init is no longer supported!");
//			return TempUserStorageInit(type,mode);
		default:
			EPRINT("Invalid storage type given to UserStorageInit!");
			break;
	}
	return NULL;
}
/*
static int UserStorageIsOwner(SIRCuserHandler *_this, char *nick,char *ownerpasswd)
{
	int nicklen,passlen;
	nicklen=strlen(nick);
	passlen=strlen(ownerpasswd);
	if(strlen(_this->ownernick)!= nicklen || strlen(_this->ownerpass) != passlen)
		return 0;
	if(memcmp(_this->ownernick,nick,nicklen) || memcmp(_this->ownerpass,ownerpasswd,passlen))
		return 0;
	return 1;
}
static int UserStorageSetOwner(SIRCuserHandler *_this, char *nick,char *ownerpasswd)
{
	if(NULL==ownerpasswd || NULL == nick || NULL == _this)
	{
		PPRINT("Could not set owner - NULL items in call UserStorageSetOwner()");
		return -1;
	}
	if(strlen(nick)>=IRC_NICK_MAX || strlen(ownerpasswd)>=IRC_PASSWORD_MAX)
	{
		PPRINT("Cannot store owner info, too long nick or pass. Accpted pass %u, nick %u chars",IRC_PASSWORD_MAX,IRC_NICK_MAX);
		return -1;
	}
	memcpy(_this->ownernick,nick,strlen(nick)+1);
	memcpy(_this->ownerpass,ownerpasswd,strlen(ownerpasswd)+1);
	return 0;
}
*/
static int PermUserStorageSyncFromFile(SIRCuserHandlerPermanent *_this)
{
	FILE *readfile;
	char *tmp;
	CexplodeStrings userploder; 
	if(NULL==_this)
	{
		PPRINT("Null param to PermUserStorageSyncFromFile()");
		return -1;
	}
	if(NULL==(readfile=fopen(_this->cfgfilename,"r")))
	{
		PPRINT("PermUserStorageSyncFromFile(): FAILED to open cfg file %s",_this->cfgfilename);
		return -1;
	}
	while(0!=fscanf(readfile,"#%a[^\n]s",&tmp))
	{
		mbot_lrtrim(tmp,' ');
		if('#'==tmp[0])
		{
			free(tmp);
			continue;
		}
		//3> because we need at least nick@mask/pass:server:level:chan (or level:*)
		if(4>Cexplode(tmp,":",&userploder))
		{
			char *nick;
			char *passhost;
			char *servername;
			char *chan;
			char *lvltmp;
			int i,j;
//			int	 lvl;
			SuserChanLevelBondage *chanlvlstruct;
			mbot_linkedList *chanlvl;
			chanlvl=mbot_ll_init();
            if(NULL==chanlvl)
            {
                PPRINT("get_user_configs(): mbot_ll init FAILED!");
                return -1;
            }
			nick=Cexplode_getfirst(&userploder);
            MAZZERT(NULL!=nick,"ImpossibleHappened again");
            i=strlen(nick);
            for(j=0;j<i;j++)
            {
                if('@'==nick[j])
                {
                    nick[j]='\0';
                    if(nick[j+1]!='\0') //this can be the case when identmode regnick is used.
                    {
                        passhost=NULL;
                    }
                    else
                        passhost=&(nick[j+1]);
                    break;
                }
            }
            servername=Cexplode_getnext(&userploder);
            MAZZERT(NULL!=servername,"ImpossibleHappened again");
			lvltmp=Cexplode_getnext(&userploder);
			if(NULL==lvltmp)
			{
				WPRINT("No lvl:chan given for user \"%s\" in configs!",nick);
				//TODO: Clean this one and get next round!
				MAZZERT(0,"Error handling not yet done here!");
			}
			while(NULL!=lvltmp)
			{
				chanlvlstruct=malloc(sizeof(SuserChanLevelBondage));
				if(NULL==chanlvlstruct)
				{
					PPRINT("Malloc FAILED at %s:%d",__FILE__,__LINE__);
					//TODO: Free memory
					return -1;
				}
				memset(chanlvlstruct->channel,0,IRC_CHANNEL_MAX);
				if( EIRCuserLevel_NmbrOf<=(unsigned int)(lvltmp[0]-'0'))
				{
					EPRINT("Invalid level configured for user \"%s\"",nick);
					//Dummyread chan
					chan=Cexplode_getnext(&userploder);
					free(chanlvlstruct);
					lvltmp=Cexplode_getnext(&userploder);
					continue;
				}
				chanlvlstruct->userlevel=(EIRCuserLevel)(lvltmp[0]-'0');
				chan=Cexplode_getnext(&userploder);
				if(chan[0]!='#' && chan[0]!='*')
				{
					WPRINT("Invalid channel name given in userlevelconfig for user %s",nick);
					free(chanlvlstruct);
					lvltmp=Cexplode_getnext(&userploder);
					continue;
				}
				memcpy(chanlvlstruct->channel,chan,strlen(chan)+1);
				if(NULL==mbot_ll_add(chanlvl,chanlvlstruct))
				{
					PPRINT("mbot_ll_add() FAILED in UserStorageSyncFromFile!");
					return -1;
				}
				lvltmp=Cexplode_getnext(&userploder);
			}
            switch(_this->handler.mode)
            {
                case EuserIdentMode_Hostmask:
                case EuserIdentMode_RegNick:
                    nick[strlen(nick)]='@'; //switch it back to nick@host
					//These functions need to do checks for dublicates!
                    _this->handler.useraddby_host((SIRCuserHandler *)_this,nick,servername,chanlvl);
                    break;
                case EuserIdentMode_Passwd:
					//These functions need to do checks for dublicates!
                    _this->handler.useraddby_passwd((SIRCuserHandler *)_this,nick,servername,chanlvl,passhost);
                    break;
                default:
                    MAZZERT(0,"Nooooo Not AGaINnNnnNnnnnn!!!");
                    break;
			}
			Cexplode_free(userploder);
		}
		free(tmp);
	}
	return 0;
}

//This is supposed to remove levels equal/smaller to regged from list, and downgrade admin level rights to one step smaller.
//It will return number of chans in new list, and negative on error. stripped list will be filled in newlist.
//Newlist must be initialized before passing to this func!
static int removeNonFileCapableUserLevels(mbot_linkedList *oldchans, mbot_linkedList *newchans)
{
	int retval=0;
	SuserChanLevelBondage *newchanlvl;
	SuserChanLevelBondage *oldchanlvl;
	oldchans=mbot_ll_get_first(oldchans);
	while( NULL!= oldchans )
	{
		oldchanlvl=mbot_ll_dataGet(oldchans);
		MAZZERT(NULL!=oldchanlvl,"NULL item in chanlist!");
		if(oldchanlvl->userlevel<EIRCuserLevel_owner && oldchanlvl->userlevel>EIRCuserLevel_regged)
		{
			newchanlvl=malloc(sizeof(SuserChanLevelBondage));
			if(NULL==newchanlvl)
			{
				PPRINT("removeNonFileCapableUserLevels(): malloc FAILED!");
				return -1;
			}
			retval++;
			memset(newchanlvl,0,sizeof(SuserChanLevelBondage));
			memcpy(newchanlvl->channel,oldchanlvl->channel,strlen(oldchanlvl->channel));//no need to copy '\0' due to abowe memset
			newchanlvl->userlevel=oldchanlvl->userlevel;
			newchans=mbot_ll_add(newchans,newchanlvl);
			if(NULL==newchans)
			{
				PPRINT("Failed to add level for writing to permanent storage!");
				return -1;
			}
		}
		oldchans=mbot_ll_get_next(oldchans);
	}
	return retval;
}

static int PermUserStorageSyncToFile(SIRCuserHandlerPermanent *_this)
{
	char *backupfilename;
	FILE *writefile;
	int userlist;
	mbot_linkedList *list_item;
	SuserStorageUser *userdata;
	//mbot_linkedList *lvlchanlist_item;
	SuserChanLevelBondage *chanlvl;
	if(NULL==_this)
	{
		PPRINT("NULL ptr given to PermUserStorageSyncToFile()");
		return -1;
	}
	backupfilename=malloc(strlen(_this->cfgfilename)+5);
	if(NULL==backupfilename)
	{
		PPRINT("Malloc FAILED at PermUserStorageSyncToFile()");
		return -1;
	}   
	memcpy(backupfilename,_this->cfgfilename,strlen(_this->cfgfilename));
	memcpy(&(backupfilename[strlen(_this->cfgfilename)]),".bak",5);
	writefile=fopen(backupfilename,"w");
	if(NULL==writefile)
	{
		PPRINT("Failed to open temp user storage (%s) file for writing! - %s",backupfilename,strerror(errno));
		return -1;
	}
	for(userlist=0;userlist<USER_LIST_SIZE;userlist++)
	{
		list_item=_this->handler.user[userlist];
		list_item=mbot_ll_get_first(list_item);
		while(NULL!=list_item)
		{
			int retval;
			char *hostpart;
			mbot_linkedList *newlist=mbot_ll_init();
			if(NULL==newlist)
			{
				PPRINT("ll_init() FAILED - malloc pb??");
				return -1;
			}
			userdata=mbot_ll_dataGet(list_item);
			MAZZERT(NULL!=userdata,"OMG! Null userdata in list!");
			//This is supposed to remove levels equal/smaller to regged from list, and downgrade admin level rights to one step smaller.
			//It will return number of chans in new list, and negative on error. stripped list will be filled in newlist.
			//Newlist must be initialized before passing to this func!
			if(0==(retval=removeNonFileCapableUserLevels(userdata->chanlevels,newlist)))
			{
				//No valid channels left to be stored => get next user
				list_item=mbot_ll_get_next(list_item);
				continue;
			}
			if(retval<0)
			{
				PPRINT("YaY! removeNonFileCapableUserLevels() FAILED, cannot store userlist!");
				return -1;
			}
			hostpart=(EuserIdentMode_Hostmask==_this->handler.mode)?userdata->password
			:
			(
				(EuserIdentMode_RegNick==_this->handler.mode)?
					"dummyhost":
					userdata->host
			);
			if(3!=fprintf(writefile,"%s@%s:%s",userdata->nick,hostpart,userdata->server))
			{
				PPRINT("Write to permanent user storage file FAILED! - %s",strerror(errno));
				return -1;
			}
			newlist=mbot_ll_get_first(newlist);
			while(NULL!=newlist)
			{
				chanlvl=mbot_ll_dataGet(newlist);
				MAZZERT(NULL!=chanlvl,"OMG! mbot_ll_contained NULL data!");
				if(2!=fprintf(writefile,"%s;%d",chanlvl->channel,chanlvl->userlevel))
				{
					PPRINT("Write to permanent user storage file FAILED! - %s",strerror(errno));
					return -1;
				}
				free(chanlvl);
				newlist=mbot_ll_get_next(newlist);
			}
			mbot_ll_destroy(&newlist);
			list_item=mbot_ll_get_next(list_item);
		}
	}
	fclose(writefile);
	//copy temp file over real conf file!
	if(rename(backupfilename,_this->cfgfilename))
	{
		EPRINT("Failed to rename temporary user storage file \"%s\" to \"%s\" - (%s)",backupfilename,_this->cfgfilename,strerror(errno));
		EPRINT("Please do the renaming manually!");
	}
	return 0;
}
static mbot_linkedList * AddNewChans(mbot_linkedList *oldlist,mbot_linkedList *newlist)
{
    SuserChanLevelBondage *old_chanlvl;
    SuserChanLevelBondage *new_chanlvl;

    mbot_linkedList *oldtmp;
    mbot_linkedList *newtmp;
    if(NULL==(oldtmp=mbot_ll_get_first(oldlist)))
        return newlist;
    if(NULL==(newtmp=mbot_ll_get_first(newlist)))
        return oldlist;
    while(NULL!=oldtmp)
    {
        old_chanlvl=mbot_ll_dataGet(oldtmp);
        MAZZERT(NULL!=old_chanlvl,"NULL data in \"oldchan\" list!");
        newtmp=mbot_ll_get_first(newlist);
        while(NULL!=newtmp)
        {
            new_chanlvl=mbot_ll_dataGet(newtmp);
            MAZZERT(NULL!=new_chanlvl,"NULL data in \"newchan\" list!");
            if(!memcmp(old_chanlvl->channel,new_chanlvl->channel,strlen(new_chanlvl->channel)))
            {
                mbot_linkedList *tmp;
                //Match found -> do not add this chan
                free(old_chanlvl);
                //The released entry should still be usable for getting prev item. Should.
                tmp=mbot_ll_release(oldtmp);
                oldtmp=mbot_ll_get_prev(oldtmp);
                free(tmp);
                break;
            }
            newtmp=mbot_ll_get_next(newtmp);
        }
        if(NULL==newtmp)
        {
            new_chanlvl=malloc(sizeof(SuserChanLevelBondage));
            if(NULL==new_chanlvl)
            {
                PPRINT("malloc FAILED at %s:%d",__FILE__,__LINE__);
                return NULL;
            }
//Since channel is not defined as char *chan, but char chan[] in bondage struct, we can just happily copy.
			if(EIRCuserLevel_admin==old_chanlvl->userlevel)
			{
				WPRINT("Admin Level attempted to grant at runtime via same channel as other levels! (%s:%d)",__FILE__,__LINE__);			
			}
			else if((unsigned int)old_chanlvl->userlevel>=(unsigned int)EIRCuserLevel_NmbrOf)
			{
				EPRINT("Attempted to add invalid userlevel!");
			}
			else
			{
            	memcpy(new_chanlvl,old_chanlvl,sizeof(SuserChanLevelBondage)); 
            	//Add chan (old to new)
            	mbot_ll_add(newlist,new_chanlvl);
			}
        }
        else
        {
            ;//Do not add, match was found.
        }
        oldtmp=mbot_ll_get_next(oldtmp);
    }
    return newlist;
}

static SuserStorageUser *findUserFromPermStorage(SIRCuserHandler *_this, char *nickhost, char *server,char *passwd)
{
    mbot_linkedList *tmp;
    int verifypass=0;
    tmp=find_user(_this,nickhost,server,passwd,verifypass);
    if(NULL==tmp)
        return (SuserStorageUser *)tmp;
    return (SuserStorageUser *)mbot_ll_dataGet(tmp);
}

//TODO: Optimize!
//Store lenghts and avoid multiple strlen() calls.
static mbot_linkedList * find_user(SIRCuserHandler *_this,char *nickhost, char *server, char *passwd,int verifypass)
{
	mbot_linkedList *tmp;
	SuserStorageUser *Suser;
	//int found=0;
    size_t newnicksize=0;
	int i;


	if(NULL==_this ||NULL == nickhost || NULL == server )
	{
		EPRINT("Invalid params (NULL) given to find_user()");
		return NULL;
	}
	if(strlen(nickhost)<2)
	{
        EPRINT("Requested adding user \"%s\", cannot add because name too short!",nickhost);
		return NULL;
	}
	for(i=0;i<strlen(nickhost)&&nickhost[i]!='@';++i);
	MAZZERT('@'==nickhost[i],"Nickendchar not found!");
	newnicksize=i; //Does not contain \0!
	tmp=_this->user[((unsigned int)nickhost[1])%USER_LIST_SIZE];
    MAZZERT(NULL!=tmp,"UserStorage not initialized!");
	tmp=mbot_ll_get_first(tmp);
	while(NULL!=tmp)
	{
		Suser=mbot_ll_dataGet(tmp);
		MAZZERT(NULL!=Suser,"Linked list had entry with no data!");
		//Check if nick and mask len match.
		//TODO: Add wildcard checks.
		if(strlen(Suser->nick)==newnicksize)
		{
			if(nickhost[strlen(Suser->nick)]=='@')
			{
				if(!memcmp(Suser->nick,nickhost,strlen(Suser->nick)))
				{
					//nick matched, if mode is "isregged" => confirm server and chan only. Else check the mask too
					if(EuserIdentMode_Hostmask==_this->mode)
					{
						if(strlen(Suser->host)!=strlen(nickhost)-strlen(Suser->nick)-1)
						{
							tmp=mbot_ll_get_next(tmp);
							continue;
						}
						//Hostname len matches
						if(memcmp(Suser->host,&(nickhost[strlen(Suser->nick)+1]),strlen(Suser->host)))
						{
							tmp=mbot_ll_get_next(tmp);
							continue;
						}
						//host matches!
					}
					else if(EuserIdentMode_Passwd == _this->mode)
					{
						if(strlen(Suser->password)!=strlen(passwd))
						{
							tmp=mbot_ll_get_next(tmp);
							continue;
						}
						if(memcmp(Suser->password,passwd,strlen(passwd)))
						{
							tmp=mbot_ll_get_next(tmp);
							continue;
						}
					}
					if(strlen(Suser->server)!=strlen(server) || memcmp(Suser->server,server,strlen(server)))
					{
						//Wrong server
						tmp=mbot_ll_get_next(tmp);
						continue;
					}
					DPRINT("find_user((): Matching user found!");
					break;
				}
			}
		}
	}
	return tmp;
}
//TODO: Optimize!
//Store lenghts and avoid multiple strlen() calls.
static mbot_linkedList * find_chanlvl(SIRCuserHandler *_this,mbot_linkedList *userlistitem,char *chan)
{
	SuserStorageUser *user;
	mbot_linkedList *chanlvl_list_item;
	user=mbot_ll_dataGet(userlistitem);
	SuserChanLevelBondage *uslvl;
	MAZZERT(NULL!=user,"NULL item in list!");
	chanlvl_list_item=user->chanlevels;
	chanlvl_list_item=mbot_ll_get_first(chanlvl_list_item);
	while(NULL!=chanlvl_list_item)
	{
		uslvl=mbot_ll_dataGet(chanlvl_list_item);
		MAZZERT(NULL!=uslvl,"NULL item in list!");
		if(strlen(chan)==strlen(uslvl->channel))
		{
			if(!memcmp(chan,uslvl->channel,strlen(chan)))
			{
				DPRINT("find_chanlvl(): Match channel found!");
				//Matching channel found!
				break;
			}
		}
		chanlvl_list_item=mbot_ll_get_next(chanlvl_list_item);
	}
	return chanlvl_list_item;
}

static int TempUserStorageDelByHost(SIRCuserHandler *_this,char *nickhost, char *server,char *chan)
{
	return TempUserStorageDel(_this,nickhost,server,chan,NULL);
}


static int TempUserStorageChangeByPasswd(SIRCuserHandler *_this, char *nickhost,char *server, SuserChanLevelBondage chanlevel,char *passwd)
{
	return TempUserStorageChange(_this,nickhost,server,chanlevel,passwd);
}
static int TempUserStorageChangeByHost(SIRCuserHandler *_this, char *nickhost,char *server, SuserChanLevelBondage chanlevel)
{
	return TempUserStorageChange(_this,nickhost,server,chanlevel,NULL);
}
static int TempUserStorageChange(SIRCuserHandler *_this, char *nickhost,char *server, SuserChanLevelBondage chanlevel,char *password)
{
	mbot_linkedList *user;
	mbot_linkedList *chanlvl;
	SuserChanLevelBondage* oldchlv;
    int verifypasswd=1;
	if(NULL==password)
		user=find_user(_this,nickhost,server,NULL,verifypasswd);
	else
		user=find_user(_this,nickhost,server,password,verifypasswd);
	if(NULL==user)
	{
		WPRINT("Attempt to change non existing user from storage!");
		return -1;
	}
	chanlvl=find_chanlvl(_this,user,chanlevel.channel);
	if(NULL==chanlvl)
	{
		WPRINT("Userchange asked for user %s, on channel %s. User found but has no levels on that chan!",nickhost,chanlevel.channel);
		return -2;
	}
	oldchlv=mbot_ll_dataGet(chanlvl);
	MAZZERT(NULL!=oldchlv,"NULL data stored in container!");
	if(chanlevel.userlevel==oldchlv->userlevel)
	{
		WPRINT("Userlevel change requested for user %s, but old level %d matches new %d",nickhost,chanlevel.userlevel,oldchlv->userlevel);
	}
	else
		oldchlv->userlevel=chanlevel.userlevel;
	return 0;
}

static int TempUserStorageDel(SIRCuserHandler *_this,char *nickhost, char *server,char *chan,char *password)
{
	mbot_linkedList *user;
	mbot_linkedList *chanlvl;
    int verifypasswd=1;
	if(NULL==password)
		user=find_user(_this,nickhost,server,NULL,verifypasswd);
	else
		user=find_user(_this,nickhost,server,password,verifypasswd);
	if(NULL==user)
	{
		WPRINT("Attempt to delete non existing user from storage!");
		return -1;
	}
	chanlvl=find_chanlvl(_this,user,chan);
	if(NULL==chanlvl)
	{
		WPRINT("Userdel asked for user %s, on channel %s. User found but has no levels on that chan!",nickhost,chan);
		return -2;
	}
	if(mbot_ll_get_first(chanlvl)==mbot_ll_get_last(chanlvl))
	{
		//This is the last entry
		chanlvl=mbot_ll_release(chanlvl);
		free(chanlvl->data);
		free(chanlvl);
		mbot_ll_destroy(&chanlvl);
		user=mbot_ll_release(user);
		free(user->data);
		free(user);
	}
	else
	{
		chanlvl=mbot_ll_release(chanlvl);
		free(chanlvl->data);
		free(chanlvl);
	}
	return 0;
}

static int TempUserStorageDelByPasswd(SIRCuserHandler *_this,char *nickhost,char *server,char *chan, char *passwd)
{
	return TempUserStorageDel(_this,nickhost,server,chan,passwd);
}


//TODO: Dublicate Chan Check to be done properly - or replan. - DONE ( AddNewChans() )
static int TempUserStorageAddByHost(SIRCuserHandler *_this, char *nickhost_toadd,char *server,mbot_linkedList *lvlchan)
{
	mbot_linkedList *tmp;
//	SuserStorageUser *user;
	//int found=0;
    size_t newnicksize=0;
    int verifypasswd=0;

//In order to speed up user search, users are stored in list corresponding to second char in nickname.
	if(NULL==_this || NULL == nickhost_toadd || NULL == server || NULL == lvlchan )
	{
		EPRINT("Invalid params (NULL) given to TempUserStorageAddByHost()");
		return -1;
	}
	if(NULL!=(tmp=find_user(_this,nickhost_toadd,server, NULL,verifypasswd)))
	{
		//Check channels, if chan is there => dublicate => just return 0
		//If no chan exists, add chan+level in list for the user.
        
        //This will use the latter argument's list as a base for new list => contents of tmp->data->chanlevels will be kept.
        //lvlchan The dublicate entries will be removed, and no data is stored to returned list.
        ((SuserStorageUser*)tmp->data)->chanlevels=AddNewChans(lvlchan,((SuserStorageUser*)tmp->data)->chanlevels);
        if(NULL==((SuserStorageUser*)tmp->data)->chanlevels)
        {
            PPRINT("OMG, AddNewChans() FAILED, Probs malloc problem??");
            return -1;
        }
        return 0;
	}
	else
	{
        SuserStorageUser *newuser;
        newuser = malloc(sizeof(SuserStorageUser));
        if(NULL==newuser)
        {
            PPRINT("OMG, Malloc FAILED at %s:%d",__FILE__,__LINE__);
            return -1;
        }
		//Add user in list
        memset(newuser,0,sizeof(SuserStorageUser));
        //No need to copy '\0' - memset'0' handled the NULL termination.
        memcpy(newuser->nick,nickhost_toadd,newnicksize);
        memcpy(newuser->host,&(nickhost_toadd[newnicksize+1]),strlen(nickhost_toadd-newnicksize-1));
        memcpy(newuser->server,server,strlen(server));
        newuser->chanlevels=mbot_ll_copylist_wdata(lvlchan,sizeof(SuserChanLevelBondage)); //This needs to copy it all. Both data and items.
	}
	return 0;
}


static int TempUserStorageAddByPasswd(SIRCuserHandler *_this,char *nickhost, char *server,mbot_linkedList *lvlchan, char *passwd)
{
	mbot_linkedList *tmp;
	//SuserStorageUser *user;
	//int found=0;
    size_t newnicksize=0;
    int verifypasswd=1;

//In order to speed up user search, users are stored in list corresponding to second char in nickname.
	if(NULL==_this || NULL == nickhost || NULL == server || NULL == lvlchan )
	{
		EPRINT("Invalid params (NULL) given to TempUserStorageAddByHost()");
		return -1;
	}
	if(NULL!=(tmp=find_user(_this,nickhost,server, passwd,verifypasswd)))
	{
		//Check channels, if chan is there => dublicate => just return 0
		//If no chan exists, add chan+level in list for the user.
        
        //This will use the latter argument's list as a base for new list => contents of tmp->data->chanlevels will be kept.
        //lvlchan The dublicate entries will be removed, and no data is stored to returned list.
        ((SuserStorageUser*)tmp->data)->chanlevels=AddNewChans(lvlchan,((SuserStorageUser*)tmp->data)->chanlevels);
        if(NULL==((SuserStorageUser*)tmp->data)->chanlevels)
        {
            PPRINT("OMG, AddNewChans() FAILED, Probs malloc problem??");
            return -1;
        }
        return 0;
	}
	else
	{
        SuserStorageUser *newuser;
        newuser = malloc(sizeof(SuserStorageUser));
        if(NULL==newuser)
        {
            PPRINT("OMG, Malloc FAILED at %s:%d",__FILE__,__LINE__);
            return -1;
        }
		//Add user in list
        memset(newuser,0,sizeof(SuserStorageUser));
/* Newuser contains:
 *     char nick[IRC_NICK_MAX];
 *     char host[IRC_HOST_MAX];
 *     char server[IRC_SERVER_MAX];
 *     mbot_linkedList *chanlevels; //onlinechans
 *     char password[IRC_PASSWORD_MAX];
 */  
        //No need to copy '\0' - memset'0' handled the NULL termination.
        memcpy(newuser->nick,nickhost,newnicksize);
        memcpy(newuser->password,passwd,strlen(passwd));
        memcpy(newuser->server,server,strlen(server));
        newuser->chanlevels=mbot_ll_copylist_wdata(lvlchan,sizeof(SuserChanLevelBondage)); //This needs to copy it all. Both data and items.
	}
	return 0;
}
/* Moved to user.c
SIRCuserHandler *StorageInit(EuserHandlerType type, EuserIdentMode mode)
{
	switch(type)
	{
		case EuserHandlerType_Permanent:
			return (SIRCuserHandler *)PermUserStorageInit(type,mode);
			break;
		case EuserHandlerType_Online:
			return TempUserStorageInit(type,mode);
		default:
			EPRINT("Invalid storage type given to UserStorageInit!");
			break;
	}
	return NULL;
}
*/
/* moved to online_storage.c
static SIRCuserHandler *TempUserStorageInit(EuserHandlerType type, EuserIdentMode mode)
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
    _this->useraddby_host		= &TempUserStorageAddByHost;
	_this->useraddby_passwd		= &TempUserStorageAddByPasswd;
	_this->userdelby_host		= &TempUserStorageDelByHost;
	_this->userdelby_passwd		= &TempUserStorageDelByPasswd;
	_this->userchangeby_host	= &TempUserStorageChangeByHost;
	_this->userchangeby_passwd	= &TempUserStorageChangeByPasswd;
	_this->set_owner			= &UserStorageSetOwner;
	_this->is_owner				= &UserStorageIsOwner;
	DPRINT("Temp user storage initialized and loaded!");
	return _this;
}
*/
SIRCuserHandler *PermUserStorageInit(EuserHandlerType type, EuserIdentMode mode)
{
	SIRCuserHandlerPermanent *_this;
	int i,j;
	size_t cfgnamelen;
	_this=malloc(sizeof(SIRCuserHandlerPermanent));
	if(NULL==_this)
	{
		PPRINT("Malloc FAILED at InitPermUserStorage()");
		return NULL;
	}
	for(i=0;i<USER_LIST_SIZE;i++)
	{
		_this->handler.user[i]=mbot_ll_init();
		if(NULL==_this->handler.user[i])
		{
			PPRINT("InitPermUserStorage(): mbot_ll_init() FAILED!");
			for(j=0;j<i;j++)
			{
				mbot_ll_destroy(&(_this->handler.user[j]));
			}
			free(_this);
			return NULL;
		}
	}
	_this->handler.mode=mode;
	_this->cfgfilename=GetConfig(E_user_level_file_name_reg+(int)mode,&cfgnamelen);
	if(NULL==_this->cfgfilename)
	{
		PPRINT("Could not get userlevel cfg file name for mode %d!",mode);
        for(j=0;j<USER_LIST_SIZE;j++)
        {
            mbot_ll_destroy(&(_this->handler.user[j]));
        }
        free(_this);
        return NULL;
	}
	DPRINT("InitPermUserStorage(): Allocations done");
	_this->handler.type					= EuserHandlerType_Permanent;
	
	/* Fill handler funcs */
	_this->handler.useraddby_host		= &TempUserStorageAddByHost;
	_this->handler.useraddby_passwd		= &TempUserStorageAddByPasswd;
	_this->handler.userdelby_host		= &TempUserStorageDelByHost;
	_this->handler.userdelby_passwd		= &TempUserStorageDelByPasswd;
	_this->handler.userchangeby_host	= &TempUserStorageChangeByHost;
	_this->handler.userchangeby_passwd	= &TempUserStorageChangeByPasswd;
//XXX: Ponder if we want to have owner info here?
	/* Fillpermanent specific funcs. */
    _this->findUser                     = &findUserFromPermStorage;
	_this->syncToFile					= &PermUserStorageSyncToFile;	//XXX: !!! DO NOT WRITE REGGED/GUEST statuses in file!
	_this->syncFromFile					= &PermUserStorageSyncFromFile;

	if(0>_this->syncFromFile(_this))
	{
		PPRINT("Could not sync user storage using cfg file %s!",_this->cfgfilename);
        for(j=0;j<USER_LIST_SIZE;j++)
        {
            mbot_ll_destroy(&(_this->handler.user[j]));
        }
        free(_this);
        return NULL;
	}
	DPRINT("Permanent user storage initialized and loaded!");
	return (SIRCuserHandler *)_this;
}



