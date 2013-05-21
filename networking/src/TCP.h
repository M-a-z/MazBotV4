/* ********************************************************************
 *
 * @file TCP.h
 * @brief a functions to use TCP connection via Sconn pointer.
 *
 *
 * -Revision History:
 * 
 *  12.07.2009/Maz  Decided to create a C++ style "virtual class" thingy
 *                  which will allow using different connections underneath
 *  10.07.2009/Maz  First draft
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




#ifndef MAZB_TCP_H
#define MAZB_TCP_H

#include <definitions.h>
#include <generic.h>
#include "networking.h"


typedef struct STCPconn
{
	Sconn		connection;
//	connectF 	connect;
}STCPconn;
//int tcp_connect_to_server(Sconn *_this_,char *domain,unsigned short int port);

size_t TCPSend(Sconn *_this_,void *data,size_t datasize);
EnetwRetval TCPcreate_socket(Sconn *_this_);
void TCPuninit(Sconn **_this_);
STCPconn *TCPinit(void);
EnetwRetval TCPconnect(Sconn *_this_,char *domain,unsigned short int port);

#endif //MAZB_TCP_H

