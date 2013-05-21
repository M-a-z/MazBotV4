/* ********************************************************************
 *
 * @file server_config.h
 * @brief MazBot server config storage implementation.
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



#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include "config_types.h"
#include "mbot_pseudo_xml.h"
#include "event_config.h"
#include "channel_config.h"
//static struct SServerConfigs;

typedef int (*ServerConfigsCfgAddF)(struct SServerConfigs *_this,SmbotPseudoxmlTag* serverRootTag);

typedef int (*serverEventConfAddF)(struct SServerConfigs *_this,SmbotPseudoxmlTag *eventRootTag);

typedef struct SServerConfigs
{
	EcfgStructType type;
	Tircnick mynick;
    int amntofchannels;
	int amntofevents;					///< amount of events which configs we have
	struct SircConfig *generic_configs;
    SChannelConfigs **channels;
	Tircserver domain;
    unsigned short int port;
	int useramnt;
	SircUser **users;
	/* TODO: Add different list for different type of events => eases parsing of incoming data */
	SServerEvents **events;
    ServerConfigsCfgAddF ServerConfigsCfgAdd;
	serverEventConfAddF serverEventConfAdd;
}SServerConfigs;

SServerConfigs *serverConfigInit(struct SircConfig *generic_configs);

#endif

