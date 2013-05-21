/* ********************************************************************
 *
 * @file defaultcallbacks.h
 * @brief contains default callbacks
 *  
 *  
 * -Revision History:
 *  
 *  - 0.0.1  13.08.2009/Maz  First draft
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


#ifndef MAZBOT_v4_DEFAULTCALLBACKS_H
#define MAZBOT_v4_DEFAULTCALLBACKS_H

#include "callbackhandler.h"
#include "event_storage.h"

#define DEF_CB_TXT_MAX 512

ServerCallbackF get_default_cb(EMbotcallbackEventType cbtype);
void *defaultcb_LocalTxtEvent(SServerCallbackArgs args);
void *defaultcb_LocalJoinEvent(SServerCallbackArgs args);
void *defaultcb_LocalPartEvent(SServerCallbackArgs args);
void *defaultcb_WebTxtEvent(SServerCallbackArgs args);
void *defaultcb_WebJoinEvent(SServerCallbackArgs args);
void *defaultcb_WebPartEvent(SServerCallbackArgs args);
#endif //MAZBOT_v4_DEFAULTCALLBACKS_H
