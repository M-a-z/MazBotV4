/* ********************************************************************
 *
 * @file startuplocks.c
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

#include "startuplocks.h"
#include <generic.h>
#include <helpers.h>


static int islockedstuplock(Sstartuplock *_this)
{
	return (int)mbot_atomicGet(_this->lockval);
}

static void uninitstuplock(Sstartuplock **_this_)
{
	Sstartuplock *_this=*_this_;
	if(NULL==_this_ || NULL==*_this_)
	{
		EPRINT("NULL detected at uninit stuplock() - doublefree?");
		return;
	}
	if(_this->lockval!=NULL)
		MbotAtomic32Uninit(&((*_this_)->lockval));
	free(*_this_);
	_this_=NULL;
}
static void lockstuplock(Sstartuplock *_this)
{
	mbot_atomicAdd(_this->lockval,1);
}

static void releasestuplock(Sstartuplock *_this)
{
	mbot_atomicDec(_this->lockval,1);
}

Sstartuplock *initstuplock()
{
	Sstartuplock *_this=malloc(sizeof(Sstartuplock));
	if(NULL==_this)
	{
		PPRINT("Malloc Failed at *initstuplock()");
		return NULL;
	}
	_this->lockval=MbotAtomic32Init();
	if(NULL==_this->lockval)
	{
		PPRINT("Malloc Failed at *initstuplock()");
		free(_this);
		return NULL;
	}
	_this->init=&initstuplock;
	_this->uninit=&uninitstuplock;
	_this->lock=&lockstuplock;
	_this->release=&releasestuplock;
	_this->islocked=&islockedstuplock;
	return _this;
}
