
#ifndef MAZBOT_v4_USER_CALLBACKS_H
#define MAZBOT_v4_USER_CALLBACKS_H

#include <callbackhandler.h>

#define REG_SUCCESS 0
#define DONT_REPLACE_DEFAULT_CALLBACK 0
#define REPLACE_DEFAULT_CALLBACK 1

#define REG_FAILURE -1

#define USERCB_RET_SUCCESS NULL
#define USERCB_RET_FAIL ((void *)-1)
#define USERCB_RET_FAIL_FATAL ((void *)-2)


int callback_register_hook(void *systemopaque);
extern int callback_reg(unsigned int eventid,ServerCallbackF cbf, void *useropaque, size_t userdatasize, void *systemopaque);
/* Sends message to IRC server */
/* NOTE, user's responcebility is to provide valid IRC message, including \r\n at the end. */
/* Size given size MUST NOT include string terminator ('\0'), since that should not be sent to server */
/* handle must be the handle given in callback function's arguments when bot engine called the callback */
/* returns 0 if send is seemingly successfull, -1 othervice */
/* NOTE: even if this function returns 0, that does not mean the message was successfully sent to server. It just means the 
 * message was successfully queued for sending!
 */
extern int Mbot_user_irc_send(void *handle,char *msg, size_t len);
extern char *Mbot_user_getmynick(void *handle);
#endif //MAZBOT_v4_USER_CALLBACKS_H

