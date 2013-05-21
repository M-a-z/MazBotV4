#include <irchelpers.h>
#include <user_callbacks.h>
#include <generic.h>
#include <startuplevels.h>
#include "Mbot_revchange_cb.h"

void unregister_callbacks_hook(void)
{
	//Add here things that should be done when bot is exiting.
	//(If there's some such thing in callbacks)
	;
}
/*
 * typedef struct SServerCallbackArgs
 * {
 *     void *handle;     one should never use this to anything else but being a handle to use with
 *                       send/requestdata/XXX macros/functions offered by MazBot's support funcs.
 *                    
 *     char *raw_data;   Raw data as server passed it 
 *     SIRCparserResult *parsed_data;  Parsed irc data in format explained at APIs/parsers_api.h
 *     void *userdataptr;              Pointer to data user provided when registering the callback 
 *     }SServerCallbackArgs;
 */
static void * examplecallback(SServerCallbackArgs args)
{
	char *msgtome;
	size_t sendsize;
	char *mynick;
	const char *msgtome_fmt="PRIVMSG %s :event %d occurred";
	DPRINT("TEST CALLBACK: YaY! event %u occurred!",*(int *)(args.userdataptr));
	DPRINT("TEST CALLBACK: Raw data from server was %s",args.raw_data);
	DPRINT("TEST CALLBACK: Testing sending...");
	mynick=Mbot_user_getmynick(args.handle);
	if(NULL==mynick)
	{
		EPRINT("TEST CALLBACK: Mbot_user_getmynick() returned NULL!");
		return USERCB_RET_FAIL;
	}
	msgtome=prepare_for_sending(&sendsize, msgtome_fmt,mynick,*(int *)(args.userdataptr));
	if(NULL==msgtome || 0==sendsize)
	{
		EPRINT("TEST CALLBACK: prepare_for_sending() FAILED (sendsize=%u msg=%p)!",sendsize,msgtome);
		free(mynick);
		return USERCB_RET_FAIL;
	}

	if(Mbot_user_irc_send(args.handle,msgtome,sendsize))
	{
		EPRINT("TEST CALLBACK: Mbot_user_irc_send() FAILED!");
		free(msgtome);
		free(mynick);
		return USERCB_RET_FAIL;
	}  
	free(msgtome);
	free(mynick);
	return USERCB_RET_SUCCESS;
}

#define CALLBACK_AMOUNT 2

int callback_register_hook(void *systemopaque)
{
	int retval=REG_SUCCESS;
	int i;
//	int cbamount=2;
	
/*
  	Example callback registrations

	//Register first callback:
*/
	unsigned int eventid[CALLBACK_AMOUNT] = {1,2};
	ServerCallbackF cbf[CALLBACK_AMOUNT]={&examplecallback,&Mbot_revchange_cb};
	void *ExtraDataToPassToCallback[CALLBACK_AMOUNT];
	size_t ExtraDataToPassToCallback_size[CALLBACK_AMOUNT];

	DPRINT("Registering user callbacks");
	for(i=0;i<CALLBACK_AMOUNT;i++)
	{
		if(0==i)
		{
			ExtraDataToPassToCallback[i]=malloc(sizeof(int));
			if(NULL==ExtraDataToPassToCallback[i])
			{
				PPRINT("Malloc Failed!");
				return REG_FAILURE;
			}
			memcpy(ExtraDataToPassToCallback[i],&(eventid[i]),sizeof(int));
			ExtraDataToPassToCallback_size[i]=sizeof(int);
		}
		else
		{
			ExtraDataToPassToCallback[i]=NULL;
			ExtraDataToPassToCallback_size[i]=0;
		}
		/* Arguments to registration func are: 
		 * 1. event Id of matching event in cfg file
		 * 2. pointer to own extra data (which will be copied && passed to callback when event occurs). Can be left NULL if no data is needed
		 * 3. extra data size (so that copying does copy correct amount. If extra data is NULL, then size must be set to 0.
		 * 4. the argument pointer passed in this hook by the system - do not modify it!
		 */
		retval-=callback_reg(eventid[i],cbf[i],ExtraDataToPassToCallback[i],ExtraDataToPassToCallback_size[i],systemopaque);
		if(NULL!=ExtraDataToPassToCallback[i])
			free(ExtraDataToPassToCallback[i]);
	}
	//Register second callback:
	return retval;
}

