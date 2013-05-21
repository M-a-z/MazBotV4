/* ********************************************************************
 *
 * @file channel_config.h
 * @brief MazBot channel config storage implementation.
 *
 *
 * -Revision History:
 *
 *  -0.0.1  15.03.2010/Maz  Splitted from irc_config.h
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



#ifndef CHANNEL_CONFIG_H
#define CHANNEL_CONFIG_H

#include "config_types.h"
#include "mbot_pseudo_xml.h"
#include "event_config.h"
#include <user.h>
struct SChannelConfigs;
//static struct SServerConfigs;

typedef int (*ChannelConfigsCfgAddF)(struct SChannelConfigs *_this,SmbotPseudoxmlTag* channelRootTag);
typedef int (*channelEventConfAddF)(struct SChannelConfigs *_this,SmbotPseudoxmlTag *eventRootTag);


typedef struct SChannelConfigs
{
	EcfgStructType type;
    Tircchan chan;
	int useramnt;
	int amntofevents;					///< amount of events which configs we have
	SircUser **users;
	SServerEvents **events;
	struct SServerConfigs *myserver;
    ChannelConfigsCfgAddF ChannelConfigsCfgAdd;
	channelEventConfAddF channelEventConfAdd;
}SChannelConfigs;

SChannelConfigs *channelConfigInit(struct SServerConfigs *server);

#endif

