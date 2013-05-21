/* ********************************************************************
 *
 * @file startup_configs.h
 * @brief read and set configurations at startup.
 *
 *
 * -Revision History:
 *
 *  - 0.0.1 17.08.2009/Maz  First Draft
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

#ifndef MAZBOT_v4_STARTUP_CONFIGS_H
#define MAZBOT_v4_STARTUP_CONFIGS_H

#include <generic.h>
#include <helpers.h>

#define BOT_CONFIG_FILE "MazBotV4_conf.txt"

int get_basic_configs(void);
int get_event_configs(void);
int parse_def_texts(void);
void clear_basic_configs();
void clear_event_configs();

#endif //MAZBOT_v4_STARTUP_CONFIGS_H
