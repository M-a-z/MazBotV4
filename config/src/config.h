/* ********************************************************************
 *
 * @file config.h
 * @brief simple configuration storage.
 *
 *
 * -Revision History:
 *
 *  17.08.2009/Maz  First draft
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

#ifndef MAZBOT_V4_CONFIG
#define MAZBOT_V4_CONFIG

#include "configNumbers.h"
#include <stddef.h>
#include <helpers.h>


/**
 * @brief Storage type for basic configurations - not used alone 
 * @see SextendedBasicConfig
 */
typedef struct SbasicConfig
{
    unsigned int cfgid;
    size_t datasize;
    void *data;
}SbasicConfig;

/**
 * @brief storage type in which real basic configs are stored. Also contains list of possible callbacks
 */
typedef struct SextendedBasicConfig
{
	SbasicConfig cfg;
	mbot_linkedList *listeners;	///< Smbot_cfgcb type memebers
}SextendedBasicConfig;

/**
 * @brief type define for callback which can be registered to config tag setting
 *
 * This type of function can be registered to be executed when specified configuration is obtained.
 * @see ConfigListenerReg, Smbot_cfgcb
 */
typedef void (*configListenerF)(int key,void *value, size_t valuesize, void *opaque);

/**
 * Storage type in which the config listener callback is stored.
 * @see SextendedBasicConfig
 */
typedef struct Smbot_cfgcb
{
	void *opaque;
	configListenerF func;
}Smbot_cfgcb;

typedef enum EconfigRet
{
	EconfigRet_Ok	=0,
	EconfigRet_Error=1
}EconfigRet;

/* Initializes config storage - Must be called before config is used */
int cfgInit(void);
/* Register a listener function for config - a callback which will be executed when specific value has been set */
EconfigRet ConfigListenerReg(unsigned int key,configListenerF func,void *opaque);
/* Get data matching specific config number */
void *GetConfig(unsigned int key,size_t *valuesize);
/* Set data matching specific config number */
EconfigRet SetConfig(unsigned int key,void *value,size_t valuesize);
/* Uninitializes config and frees allocated resources */
void cfgUninit(void);

#endif
