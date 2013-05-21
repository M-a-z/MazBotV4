/* ********************************************************************
 *
 * @file networking.h
 * @brief a wrapper/generic functions for different type of connections.
 * This file contains wrappers to call connection specific functions using the 
 * correctly initialized Sconn* pointer. (Like virtual classes in C)
 *
 *
 * -Revision History:
 *
 *  - 0.0.4 18.02.2010/Maz  Added documentation
 *  - 0.0.3 20.07.2009/Maz	Separated read and write poll to own functions.
 *  						(Fixed bad design :) )
 *  - 0.0.2 12.07.2009/Maz  Decided to create a C++ style "virtual class" thingy
 *                  		which will allow using different connections underneath
 *  - 0.0.1 10.07.2009/Maz  First draft
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


#ifndef MAZB_NETWORKING_H
#define MAZB_NETWORKING_H

#include <generic.h>
#include <definitions.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef __cpp
extern "C"
{

#endif
#define ISTYPESANE(struc) MAZZERT((unsigned int)(struc)->type<EstructType_NmbrOf,"BadConnType")
#define ISTYPECONN(struc) MAZZERT((unsigned int)(struc)->type<CONN_PROTOCOLS,"ConnTypeNotConnected")

#define SUPP_PROTOCOLS 1
#define CONN_PROTOCOLS 1

struct Sconn;
/**
 * @brief Return values for networking service calls
 */
typedef enum EnetwRetval
{
	EnetwRetval_Ok 				= 0,  ///< Operation successfull
	EnetwRetval_InternalError 	= -1, ///< Unspecified internal error
	EnetwRetval_UnableToResolve = -2, ///< Domain name resolving failed
	EnetwRetval_InvParam		= -3  ///< Invalid param given to call
}EnetwRetval;


/**
 * @brief types of object which can be created using this generic IF
 */
typedef enum EstructType
{
	/* Connected protocols */
	/* MUST be listed before non connected ones */
	EstructType_TCPconn = 0,		///< TCP/IP connection
	/*Non connected protocols */
	EstructType_UDPconn,			///< UDP connection
	/* Keep this always as last member */
	EstructType_NmbrOf				///< Not to be used, internal value.
}EstructType;

/**
 * @brief Connection object state
 */
typedef enum EconnState
{
	EconnState_inited	= 0,   ///< Initialized but not used yet
	EconnState_sock_created,   ///< Socket created
	EconnState_connected,	   ///< Connected (only with connected protocols (TCP))
	EconnState_disconnected,   ///< Connection lost/disconnected (only with connected protocols (TCP))
	EconnState_Nmbrof		   ///< Not to be used, internal value.
}EconnState;
/**
 * @brief poll function's return values, specify socket state.
 */
typedef enum EsockStat
{
	EsockStat_quiet = 0,		///< No data coming
	EsockStat_readyToRead,		///< Data received, ready for reading
	EsockStat_readyToWrite,		///< Ready for writing
	EsockStat_Error,			///< Error occurred
	EsockStat_NmbrOf			///< Not to be used, internal value.
}EsockStat;

typedef struct Sconn * (*initF)(void);
typedef EnetwRetval (*connectF)(struct Sconn *,char *,unsigned short int);
typedef EnetwRetval (*sockCreateF)(struct Sconn *);
typedef EsockStat (*pollF) (struct Sconn *,struct timeval ); 
typedef void (*uninitF)(struct Sconn **);
typedef size_t (*sendF)(struct Sconn *,void *,size_t); 
typedef size_t (*recvF)(struct Sconn *_this_,void *buff,size_t buflen);
typedef size_t (*nbrecvF)(struct Sconn *_this_,void *buff,size_t buflen,struct timeval );

/**
 * @brief The actual connection object
 *
 * This will be created by call to Sconn *connInit(EstructType type). Returned struct
 * will be initialized, Eg. initial values to data have been assigned, and function pointers initialized
 */
typedef struct Sconn
{
	EstructType 		type;			///< internal (underlying implementation type, used for casting)
	int 				sock;			///< internal (the actual socket)
	MAZ_BOOLEAN			connectible;	///< internal (whether used protocol is connectible)
	EconnState  		state;			///< internal (socket state)
	/**
	 * \function Sconn *initialize(void) 
	 * @brief Initializes the undrlying implementation (maybe?)
	 */
	initF				initialize;	
	/**
	 * \function void destroy(struct Sconn **_this)
	 *  @brief Destroying function, releases allocated resources
	 *  
	 *  releases allocated resources, closes connection and shall set the _this ptr to NULL upon return
	 *
	 *  @param Sconn **_this Pointer to pointer to connection struct created with Sconn *connInit(EstructType type) 
	 *  Shall be set to NULL upon return
	 *
	 *  @returns Nothing
	 *  @see connInit
	 *
	 */
	uninitF 			destroy;
	/**
	 * \function EnetwRetval create_socket(struct Sconn *_this)
	 * @brief Socket creation function
	 *
	 * Creates the socket for connection (internal?)
	 * @param Sconn *_this pointer to connection struct created with Sconn *connInit(EstructType type)
	 * @returns EnetwRetval_Ok upon successfull completion, othervice error value from EnetwRetval enum
	 */
	sockCreateF			create_socket;
	struct sockaddr_in 	server;			///< internal shall contain target info
	Tircserver			servername;		///< internal target system domainname
	/**
	 * \function size_t send(struct Sconn *_this,void *data,size_t datasize)
	 * @brief Function for sending data to socket
	 *
	 * Sends given data to socket.
	 * @param Sconn *_this pointer to connection struct created with Sconn *connInit(EstructType type)
	 * @param void *data pointer to the beginning of data to be sent
	 * @param size_t datasize amount of bytes to be sent from beginning of data
	 *
	 * @returns size_t amount of sent bytes, ??? upon error
	 */
	sendF				send;
	/** 
	 * \function size_t recv(struct Sconn *_this_,void *buff,size_t buflen)
	 * @brief Function to receive data from socket
	 *
	 * Receives (at max) given amount of bytes from socket. Blocks untill at least some data is available, 
	 * or socket state goes invalid.
	 *
	 * @param Sconn *_this pointer to connection struct created with Sconn *connInit(EstructType type)
	 * @param void *buff Buffer in which the data is copied - does not perform any initialization for buffer
	 * @param size_t buflen Lenght of provided buffer
	 *
	 * @returns amount of received data
	 */
	recvF				recv;
	/**
	 * \function size_t non_block_recv(struct Sconn *_this,void *buff,size_t buflen,struct timeval tmo)
	 * @brief Non blocking receiving function
	 *
	 * Receives (at max) given amount of bytes from socket. Blocks untill at least some data is available, 
	 * timeout occurs or socket state goes invalid.
	 *
	 * @param Sconn *_this pointer to connection struct created with Sconn *connInit(EstructType type)
	 * @param void *buff Buffer in which the data is copied - does not perform any initialization for buffer
	 * @param size_t buflen Lenght of provided buffer
	 * @param struct timeval tmo Maximum time to wait data, returns 0 if no data is received before tmo.
	 *
	 * @returns amount of received data
	 *
	 */
	nbrecvF				non_block_recv;
	/**
	 * \function EnetwRetval connect(struct Sconn *_this,char *server,unsigned short int port)
	 * @brief Connection creation function
	 *
	 * Creates connection (only for connected protocols) to specified target server and port
	 *
	 * @param Sconn *_this pointer to connection struct created with Sconn *connInit(EstructType type)
	 * @param char *server domain name or numbers and dots notation IP (as char array) whre to connect
	 * @param unsigned short int port port in which the connection should be created.
	 *
	 * @returns EnetwRetval_Ok upon successfull completion, othervice error value from EnetwRetval enum
	 *
	 */
	connectF			connect;
	/**
	 * \function EsockStat readpoll(struct Sconn *,struct timeval tmo)
	 * @brief polling function for reading
	 *
	 * Probes socket to see if there's data ready for reading. Blocks untill data is available, 
	 * error occurs, or specified maximum wait time is up.
	 *
	 * @param Sconn *_this pointer to connection struct created with Sconn *connInit(EstructType type)
	 * @param struct timeval tmo Maximum blocking time while waiting for data
	 * @returns EsockStat struct member telling the state of socket
	 * @see EsockStat
	 */
	pollF       		readpoll;
	/**
	 * \function EsockStat writepoll(struct Sconn *,struct timeval tmo)
	 * @brief polling function for writing
	 *
	 * Probes socket to see if it is ready for writing. Blocks untill socket becomes ready, 
	 * error occurs, or specified maximum wait time is up.
	 *
	 * @param Sconn *_this pointer to connection struct created with Sconn *connInit(EstructType type)
	 * @param struct timeval tmo Maximum blocking time while waiting for socket to be rady
	 * @returns EsockStat struct member telling the state of socket
	 * @see EsockStat
	 */
	pollF				writepoll;
}Sconn;

/**
 * @brief function to initialize connection object
 *
 * Prepares connection object of specified type ready for being used.
 *
 * @param EstructType type Type of connection object to be created.
 * @returns pointer to created connection object, NULL on error.
 */
Sconn *connInit(EstructType type);

EnetwRetval dummyconnect(Sconn *_this,char *domain,unsigned short int port);
struct addrinfo * resolve_host(const char *host,EstructType type);
int connect_to_server(Sconn *_this,char *domain,unsigned short int port);
EsockStat writepoll(Sconn *_this,struct timeval tmo);
EsockStat readpoll(Sconn *_this,struct timeval tmo);

	//EsockStat netwpoll(Sconn *_this,struct timeval tmo);
#ifdef __cpp
}
#endif //__cpp
#endif //MAZB_NETWORKING_H
