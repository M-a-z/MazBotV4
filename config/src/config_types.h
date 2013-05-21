/* ********************************************************************
 *
 * @file config_types.h
 * @brief Definitions for MazBot's config "object's" types.
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





#ifndef CONFIG_TYPES_H
#define CONFIG_TYPES_H

typedef enum EcfgStructType
{
	EcfgStructType_irc,
	EcfgStructType_server,
	EcfgStructType_chan,
	EcfgStructType_nmbrof
}EcfgStructType;

#endif
