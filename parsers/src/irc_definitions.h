/* ********************************************************************
 *
 * @file irc_definitions.h
 * @brief This file is going to contain some more IRC related definitions
 *
 *
 * -Revision History:
 *
 *  - 0.0.1 11.08.2009/Maz  First draft
 *
 *  
 *  Lisence info: You're allowed to use / modify this - you're only required to
 *  write me a short note and your opinion about this to Mazziesaccount@gmail.com.
 *  if you redistribute this you're required to mention the original author:
 *  "Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
 *  channel. NOTE: Contents are mostly obtained from RFC 2812, I do not claim
 *  copyright of any sort to the information included, only the C code format
 *  (which is going to be in this file some day :D)
 *  
 *  
 *  PasteLeft 2009 Maz.
 *  *************************************************************/



#ifndef MBOT_V4_IRC_DEFINITIONS_H
#define MBOT_V4_IRC_DEFINITIONS_H

#include <irc_protocol_definitions.h>

/**
 * User type - tells which fields are relevant in SIRC_user struct
 * @see SIRC_user
 */
typedef enum EIRCuserLevelDirection
{
	EIRCuserLevelDirection_Larger	= 0,
	EIRCuserLevelDirection_Equal	= 1,
	EIRCuserLevelDirection_Smaller
}EIRCuserLevelDirection;

typedef enum EIRCuserType
{
    EIRCuserType_User = 0,
    EIRCuserType_Server,
    EIRCuserType_Channel
}EIRCuserType;

typedef enum EIRCuserLevel
{
    EIRCuserLevel_guest = 0,
    EIRCuserLevel_regged,
	EIRCuserLevel_mod,
    EIRCuserLevel_admin,
	EIRCuserLevel_owner,
	EIRCuserLevel_NmbrOf
}EIRCuserLevel;

typedef struct SIRC_user
{
	EIRCuserType type;
	char nick[IRC_NICK_MAX];
	char host[IRC_HOST_MAX];
	char server[IRC_SERVER_MAX];
	char channel[IRC_CHANNEL_MAX];
    EIRCuserLevel userlevel;
}SIRC_user;





#endif //MBOT_V4_IRC_DEFINITIONS_H

