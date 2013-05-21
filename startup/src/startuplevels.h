/* ********************************************************************
 *
 * @file startuplevels.h
 * @brief functions to organize startup in synchronized levels (needed if I later fell on multithreading - which I avoid)
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

#ifndef MAZBOT_v4_STARTUPLEVELS_H
#define MAZBOT_v4_STARTUPLEVELS_H
#include <helpers.h>

int init_startlevel1();
int init_startlevel2();
int init_startlevel3();
int init_startlevel4();
int init_startlevel5();
int deinit_startlevel1();
int deinit_startlevel2();
int deinit_startlevel3();
int deinit_startlevel4();
int deinit_startlevel5();

void unregister_callbacks_hook(void);
int register_callbacks_hook(void);

#endif //MAZBOT_v4_STARTUPLEVELS_H

