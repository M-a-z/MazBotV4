/* ********************************************************************
 *
 * @file definitions.h
 * @brief MazBot's global definitions.
 *
 *
 * -Revision History:
 * 
 *  10.07.2009/Maz  First draft
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

#ifndef MAZB_DEFINITIONS_H
#define MAZB_DEFINITIONS_H


#define ASSERT_ENABLE
#define DEBUGS_IN_USE 1
//#define 

#ifdef DEBUGS_IN_USE
	#define PRINT_FILTER_DEFAULT 0
#else
	#define PRINT_FILTER_DEFAULT EPrintLevel_Error;
#endif
#define MAZ_TRUE 1
#define MAZ_FALSE 0
#define IRC_NICK_MAX 15
#define IRC_HOST_MAX 63
#define IRC_SERVER_MAX 63
#define IRC_CHANNEL_MAX 200
#define IRC_PREFIX_MAX 508
#define IRC_CMD_MAX 510
#define IRC_PARAMS_MAX 508
#define IRC_PASSWORD_MAX 12
#define IRC_MSG_MAX 512



typedef int MAZ_BOOLEAN;
typedef char Tircchan[IRC_CHANNEL_MAX];
typedef char Tircserver[IRC_SERVER_MAX];
typedef char Tircnick[IRC_NICK_MAX];
typedef char Tirchost[IRC_HOST_MAX];
typedef char Tircpass[IRC_PASSWORD_MAX];

#endif //MAZB_DEFINITIONS_H

