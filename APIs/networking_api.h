/* ********************************************************************
 * 
 * @file networking_api.h
 * @brief generic functions for different type of connections.
 * This file contains wrappers to call connection specific functions using the 
 * correctly initialized Sconn* pointer. (Like virtual classes in C)
 *
 *
 * -Revision History:
 *
 *  - 0.0.1 22.07.2009/Maz  First draft
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

#ifndef MAZBOT_V_4_NETWORKING_API_H
#define MAZBOT_V_4_NETWORKING_API_H

#include <generic_api.h>
#include <sys/time.h>
#ifdef __cpp
extern "C"
{
#endif
struct Sconn;

typedef enum EnetwRetval
{
    EnetwRetval_Ok              = 0,
    EnetwRetval_InternalError   = -1,
    EnetwRetval_UnableToResolve = -2,
    EnetwRetval_InvParam        = -3
}EnetwRetval;

typedef enum EstructType
{
    /* Connected protocols */
    /* MUST be listed before non connected ones */
    EstructType_TCPconn = 0,
    /*Non connected protocols */
    EstructType_UDPconn,
    /* Keep this always as last member */
    EstructType_NmbrOf
}EstructType;

typedef enum EconnState
{
    EconnState_inited   = 0,
    EconnState_sock_created,
    EconnState_connected,
    EconnState_disconnected,
    EconnState_Nmbrof
}EconnState;

typedef enum EsockStat
{
    EsockStat_quiet = 0,
    EsockStat_readyToRead,
    EsockStat_readyToWrite,
    EsockStat_Error,
    EsockStat_NmbrOf
}EsockStat;


typedef struct Sconn * (*initF)(void);
typedef EnetwRetval (*connectF)(struct Sconn *,char *,unsigned short int);
typedef EnetwRetval (*sockCreateF)(struct Sconn *);
typedef EsockStat (*pollF) (struct Sconn *,struct timeval );
typedef void (*uninitF)(struct Sconn **);
typedef size_t (*sendF)(struct Sconn *,void *,size_t);
typedef size_t (*recvF)(struct Sconn *_this_,void *buff,size_t buflen);
typedef size_t (*nbrecvF)(struct Sconn *_this_,void *buff,size_t buflen,struct timeval );


typedef struct Sconn
{
    EstructType         type;
    int                 sock;
    MAZ_BOOLEAN         connectible;
    EconnState          state;
    initF               initialize;
    uninitF             destroy;
    sockCreateF         create_socket;
    struct sockaddr_in  server;
    sendF               send;
    recvF               recv;
    nbrecvF             non_block_recv;
    connectF            connect;
    pollF               readpoll;
    pollF               writepoll;
}Sconn;

Sconn *connInit(EstructType type);

#ifdef __cpp            
}   
#endif //__cpp 

#endif
