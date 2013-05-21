//
//Abowe server will be global things.
//ATM: events, permanent user storage, bot owner, callbacks, configs, php connection(s)
//callbacks and events would be good to be dropped to channel / server level??
//
//
//
//Server will consist of:
//
//Server name, ip, id?  (in connection?)
//Connection
//channels
//
//Channels will consist of
//chan name
//online users
//callbacks ?? Would be clever, but requires some rewriting - actually, events will contain callback data.
//events ?? Would be clever but requires some rewriting
//
typedef struct SServer
{
	Sconn *irc_conn;  //TCP connection to IRC server.
	mbot_linkedList * channels; //SChannel structs in data
}SServer;

typedef struct SChannel
{
//I'll need event container here
Sevents chanevents;
//user container
SuserHandler *onlineusers;

}SChannel;
