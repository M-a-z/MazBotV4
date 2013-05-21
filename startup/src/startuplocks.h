/* ********************************************************************
 *
 * @file startuplocks.h
 * @brief functions to handle synchronization of startup levels
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


#ifndef MAZBOT_v4_STARTUPLOCKS_H
#define MAZBOT_v4_STARTUPLOCKS_H
#include <helpers.h>

struct startuplock;
typedef struct Sstartuplock * (*startuplock_initF)();
typedef void (*startuplock_uninitF)(struct Sstartuplock **_this_);
typedef void (*startuplock_lockF)(struct Sstartuplock *lock);
typedef void (*startuplock_releaseF)(struct Sstartuplock *lock);
typedef int (*startuplock_islockedF)(struct Sstartuplock *lock);


typedef struct Sstartuplock
{
	MbotAtomic32 *lockval;
	startuplock_initF init;
	startuplock_uninitF uninit;
	startuplock_lockF lock;
	startuplock_releaseF release;
	startuplock_islockedF islocked;
}Sstartuplock;

Sstartuplock *initstuplock();
#endif //MAZBOT_v4_STARTUPLOCKS_H

