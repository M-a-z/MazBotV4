/* ********************************************************************
 *
 * @file startuplevels.c
 * @brief functions to organize startup at synchronous levels. (Needed if I ever fell to multithreading - which I avoid)
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
#include "startuplevels.h"
#include "startup_configs.h"
#include <event_storage.h>
#include <unistd.h>
#include <config.h>

#define STARTLEVEL_AMNT 5

Sstartuplock *startlevel_locks[STARTLEVEL_AMNT];


static void wait_startuplock(Sstartuplock *lock);
static int init_startuplocks();



int init_startlevel1()
{
	if(0>init_startuplocks())
	{
		PPRINT("Failed to init startuplocks -> aborting");
		return -1;
	}
	if(0>cfgInit())
	{
		PPRINT("Config storage init FAILED!");
		return -1;
	}
	wait_startuplock(startlevel_locks[0]);
	return 0;
}

int init_startlevel2()
{
    if(0!=get_basic_configs())
	{
		PPRINT("Failed to read MazBorCfg.txt");
		deinit_startlevel1();
    	return -1;
	}
/*
 * This shall now be done in the channel / server struct init 
	if(0>mbot_event_storages_init())
	{
		PPRINT("Failed to init event storage!");
		deinit_startlevel1();
		return -1;
	}
	else
	{
		DPRINT("Event storage successfully inited, reading events...");
	}
*/
    if(0!=get_event_configs())
	{
		PPRINT("Reading event cfgs FAILED!");
		deinit_startlevel1();
		return -1;
	}
	if(0!=parse_def_texts())
	{
		PPRINT("Parsing default callback strings failed!");
		clear_event_configs();
		deinit_startlevel1();
		return -1;
	}
    if(0!=register_callbacks_hook())
	{
		PPRINT("Registering user callbacks FAILED!");
		clear_event_configs();
		deinit_startlevel1();
		return -1;
	}
	wait_startuplock(startlevel_locks[1]);
	return 0;
}

int init_startlevel3()
{
;    
	wait_startuplock(startlevel_locks[2]);
	return 0;

}

int init_startlevel4()
{
	wait_startuplock(startlevel_locks[3]);
	return 0;
}

void connect_and_run()
{
	DPRINT("Init done, now I would be connecting...");
}

int init_startlevel5()
{
    //Connections are taken at the beginning of mainloop?
//    connect_and_store_connections_etc();
	wait_startuplock(startlevel_locks[4]);
	return 0;
	// Now, return to wait shutdown requests like SIG_INT - if such arrives => terminate threads.
}



int deinit_startlevel1()
{
	cfgUninit();
	DPRINT("Level 1 deinited");
	return 0;
}
void clear_callback_configs()
{
	WPRINT("clear_callback_configs() just dummy implementation");
}

int deinit_startlevel2()
{
    clear_basic_configs();
    unregister_callbacks_hook();
    clear_callback_configs();
/* TODO: uninit for each server struct, event storages are now inside them */
//	mbot_event_storages_uninit();
	DPRINT("Level 2 deinited");
	deinit_startlevel1();
	return 0;
}

int deinit_startlevel3()
{
    

	DPRINT("Level 3 deinited");
	deinit_startlevel2();
	return 0;
}

int deinit_startlevel4()
{


	DPRINT("Level 4 deinited");
	deinit_startlevel3();
	return 0;
}
void stop()
{
	DPRINT("Stopping");
}
void disconnect()
{
	DPRINT("Woould be disconnecting");
}
int deinit_startlevel5()
{
    stop();
    disconnect();
	DPRINT("Level 5 deinited");
	deinit_startlevel4();
	return 0;
}



static int init_startuplocks()
{
	int i;
	for(i=0;i<STARTLEVEL_AMNT;i++)
	{
		startlevel_locks[i]=initstuplock();
		if(NULL==startlevel_locks[i])
		{
			PPRINT("Failed to init Startup Lock for level %d",i);
			i--;
			for(;i>=0;i--)
			{
				startlevel_locks[i]->uninit(&(startlevel_locks[i]));
			}
			return -1;
		}
	}
	return 0;
}


static void wait_startuplock(Sstartuplock *lock)
{
	while(lock->islocked(lock))
	{
		DPRINT("Waiting startuplock...");
		sleep(1);
	}
}
