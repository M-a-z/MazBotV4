
#ifndef MBOT_INTERNAL_CALLBACKS_H
#define MBOT_INTERNAL_CALLBACKS_H

/* Define bitbasks for allowed further callbacks */

#define MBOT_CBRET_INTERNAL_ERROR_FATAL ((int)0x80000000)
#define MBOT_CBRET_INTERNAL_ERROR ((int)0x40000000)
#define MBOT_CBRET_USER_ERROR ((int)0x20000000)
#define MBOT_CBRET_NO_USER_CHAN_CBS ((int)1)
#define MBOT_CBRET_NO_INTERNAL_CHAN_CBS ((int)2)
#define MBOT_CBRET_NO_USER_SRV_CBS ((int)4)
#define MBOT_CBRET_NO_INTERNAL_SRV_CBS ((int)8)

#define MBOT_CBRET_NO_INTERNAL_CBS (MBOT_CBRET_NO_INTERNAL_SRV_CBS | MBOT_CBRET_NO_INTERNAL_CHAN_CBS)
#define MBOT_CBRET_NO_USER_CBS (MBOT_CBRET_NO_USER_CHAN_CBS | MBOT_CBRET_NO_USER_SRV_CBS)
#define MBOT_CBRET_NO_CHAN_CBS (MBOT_CBRET_NO_USER_CHAN_CBS | MBOT_CBRET_NO_INTERNAL_CHAN_CBS)
#define MBOT_CBRET_NO_SRV_CBS (MBOT_CBRET_NO_USER_SRV_CBS | MBOT_CBRET_NO_INTERNAL_SRV_CBS)

#define MBOT_CBRET_NO_CBS (MBOT_CBRET_NO_SRV_CBS | MBOT_CBRET_NO_CHAN_CBS)

/* Quick ugly hack to support old enum... TODO: Start using bitmasks instead. */

typedef int EMbotCallbackRet;

#define EMbotCallbackRet_InternalCbFailFatal MBOT_CBRET_INTERNAL_ERROR_FATAL
#define EMbotCallbackRet_InternalCbFail MBOT_CBRET_INTERNAL_ERROR		///< Do not call other channel or user callbacks, print
#define    EMbotCallbackRet_UserCbFail MBOT_CBRET_USER_ERROR			///< EPRINT
#define    EMbotCallbackRet_NotHandled 0			///< Proceed with other callbacks
#define    EMbotCallbackRet_InternalHandled MBOT_CBRET_NO_CBS		///< Do not call user, channel or server callbacks, this is handled.
#define    EMbotCallbackRet_UserHandled MBOT_CBRET_NO_USER_CBS			///< DPRINT, no other user callbacks should be called
#define    EMbotCallbackRet_AllowInternalOnly MBOT_CBRET_NO_USER_CBS		///< Do not call user callbacks for (rest of the) channels
#define    EMbotCallbackRet_AllowUserOnly MBOT_CBRET_NO_INTERNAL_CBS			///< Do not call internal callbacks for rest of the channels
#define	EMbotCallbackRet_AllowServerOnly MBOT_CBRET_NO_CHAN_CBS
#define	EMbotCallbackRet_AllowChannelsOnly MBOT_CBRET_NO_SRV_CBS
#define	EMbotCallbackRet_AllowServerAndUserOnly MBOT_CBRET_NO_INTERNAL_CHAN_CBS
#define	EMbotCallbackRet_AllowChannelAndUserOnly MBOT_CBRET_NO_INTERNAL_SRV_CBS


EMbotCallbackRet chan_handle_version_cmm(struct Sirc_channels *_this,SIRCparserResult *res);
EMbotCallbackRet chan_handle_identify_cmm(struct Sirc_channels *_this,struct SIRCparserResult *res);
EMbotCallbackRet chan_handle_userlist_user(struct Sirc_channels *_this,struct SIRCparserResult *res);
EMbotCallbackRet chan_handle_internal_join_cmm(struct Sirc_channels *_this,struct SIRCparserResult *res);
EMbotCallbackRet chan_handle_part(struct Sirc_channels *_this,struct SIRCparserResult *res);
EMbotCallbackRet chan_handle_nick(struct Sirc_channels *_this,SIRCparserResult *res);


EMbotCallbackRet chan_handle_internal_joinfail_cmm(struct Sirc_channels *_this,SIRCparserResult *res);
EMbotCallbackRet srv_handle_endOfMotd(struct Sirc_servers *_this,SIRCparserResult *res);
EMbotCallbackRet srv_handle_quit(struct Sirc_servers *_this,struct SIRCparserResult *res);
EMbotCallbackRet srv_handle_ping(struct Sirc_servers *_this,SIRCparserResult *res);
EMbotCallbackRet srv_handle_nick(struct Sirc_servers *_this,SIRCparserResult *res);


#endif


