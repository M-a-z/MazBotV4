/* ********************************************************************
 *
 * @file TCP.c
 * @brief a functions to use TCP connection via Sconn pointer.
 *
 *
 * -Revision History:
 *  -1.0.0 29.07.2009/Maz  ready for MazBot V 0.1 (ITERATION1)
 *	-0.0.6 29.07.2009/Maz  Fixed issue with "dice"
 *						   polished for v0.1 "release"
 *  -0.0.5 29.07.2009/Maz  Fixed SENDDATA2 define and ping-pong issue - 
 *  					   only some polishing needed and v0.1 (ITERATION 1)
 *  					   can be accepted.
 *  -0.0.4 2x.07.2009/Maz  Changed network test to connect to IRC.
 *  -0.0.3 20.07.2008/Maz  Changed the connection target && made them as defines
 *  -0.0.2 12.07.2009/Maz  Added sending HTTP request.
 *  -0.0.1 12.07.2009/Maz  First Draft.
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

#include <networking.h>
#include <TCP.h>
#include <generic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <irc_protocol_parser.h>
#define RECVBUFF 512
#define HOST "teotilcan.net"
#define PORT 6667
#define SENDDATA "NICK DiceSenior\r\n";
#define SENDDATA2 "USER Dice FOOBARHOST teotilcan.net :Maz Bot\r\n";

char *matchcommands[] = { "PRIVMSG", "PING" };
char *matchparams[] = { ":!roll" };

int main(void)
{
	int ret;
	Sconn *connection;
	char buff[RECVBUFF];
	EnetwRetval retval;
	EsockStat   pollval;
	Sparser *parser;	
	EparserState parstate;
	struct timeval tim;
	tim.tv_sec=1;
	tim.tv_usec=0;
	connection = connInit(EstructType_TCPconn);
	cfgInit();
	if(NULL==connection)
	{
		printf("Miserably failed to init connection struct\n");
		return -1;
	}
	DPRINT("connecting...");
	if(EnetwRetval_Ok!=(retval=connection->connect(connection,HOST,PORT)))
	{
		printf("Miserably failed to connect to %s:%d, retval %d\n",HOST,PORT,retval);
		return -1;
	}
	{
		//http://teotilcan.net/svn/MazBot/"
		char request[] = SENDDATA; 
		char request2[]=SENDDATA2;
		printf("sock ready for writing (no data coming in)\n");
		DPRINT("Sending...\t%s",request);
		if(strlen(request)!=(ret=connection->send(connection,request,strlen(request))))
		{
			if(ret<=0)
			{
				EPRINT("Error occurred when sending!, retval %d",ret);
			}
			else
		{
					WPRINT("odd amount of data sent! asked %d, send returned %d",sizeof(request),ret);
			}
		}
		DPRINT("Trying to receive:");
		memset(buff,0,sizeof(buff));
		while(0<(ret=connection->non_block_recv(connection,buff,RECVBUFF,tim)))
		{
			DPRINT("recvd more: %s",buff);
		}
		DPRINT("Sending req2 >> %s",request2);
	 	if(strlen(request2)!=(ret=connection->send(connection,request2,strlen(request2))))
        {
        	if(ret<=0)
            {
            	EPRINT("Error occurred when sending!, retval %d",ret);
            }
            else
            {
                WPRINT("odd amount of data sent! asked %d, send returned %d",sizeof(request2),ret);
            }
        }
        while(0<(ret=connection->non_block_recv(connection,buff,RECVBUFF,tim)))
        {
            DPRINT("recvd morei2: %s",buff);
        }
	}
	memset(buff,0,sizeof(buff));
	srand(time(NULL));
	parser=ParserInit(EparserType_Irc);
	if(NULL==parser)
	{
		EPRINT("YAY! parser alloc failed!");
		return -1;
	}
	while(0<=(ret=connection->recv(connection,buff,RECVBUFF)))
	{
		EparserRetVal pret;
		DPRINT("Feeding parser: \"%s\"",buff);
		if(EparserRetVal_Ok==(pret=parser->feed(parser,buff,ret, &parstate)) || EparserRetVal_OverFeed == pret)
		{
            if(parstate==EparserState_FatalError)
            {
                PPRINT("irc parser reported fatal error!");
                exit(-1);
            }
			while(EparserState_ResultReady == parstate )
			{
				SIRCparserResult *res;
				res=(SIRCparserResult *)parser->get_result(parser,&parstate);
				DPRINT("Getting result");
				if(NULL==res)
				{
					WPRINT("NULL result from parser!");
				}
				else
				{
					char *cmd;
					cmd=res->getcmd(res);
					DPRINT("command \"%s\"",cmd);
					if(!strcmp(cmd,matchcommands[0]))
					{
						DPRINT("Param 2 = \"%s\"",res->getparam(res,2));
						if(!strcmp(res->getparam(res,2),matchparams[0]))
						{
							char reply[330];
							char *tmp;
							char fmt[34]="PRIVMSG #teotilcan :Result %d\r\n";
							int throws;
							int dicesides;
							int diceval;
							int neg=1;
							char *endp;
							tmp=res->getparam(res,3);
							DPRINT("Param 3 = \"%s\"",res->getparam(res,3));
							DPRINT("Generating dice reply");
							if(NULL!=tmp)
							{
								throws=strtol(tmp,&endp,0);
								DPRINT("Throws %d",throws);
								memset(reply,0,sizeof(reply));
								if(throws<0 || throws>20)
								{
									DPRINT("Illegal amount of sides!");

//									connection->send(connection,"PRIVMSG #teotilcan :Oh, how clever, you thought I'd be fooled by sides?\r\n",strlen("PRIVMSG #teotilcan :Oh, how clever, you thought I'd be fooled by sides?\r\n"));
									neg=-1;
								}
								else
								{
									if(*endp=='d' || *endp=='D')
									{
										//valid throw
										//it is valid to omit number of throws if it is 1
										if(throws==0)
											throws=1;
										dicesides=strtol(endp+1,&endp,0);
										if(1>=dicesides || 1000<dicesides || endp!=&(tmp[strlen(tmp)]))
										{
											EPRINT("Malformed dice request %s%s","!dice ",tmp);
										}
										else
										{
											int throwloop;
											int dicesum=0;
											for(throwloop=0;throwloop<throws;throwloop++)
											{
		//										if(neg==-1)
												diceval=(rand()%dicesides)+1;
												diceval*=neg;
												dicesum+=diceval;
											}
											snprintf(reply,sizeof(reply),fmt,dicesum);
			//								reply[32]='\0';
											DPRINT(">> %s",reply);
											connection->send(connection,reply,strlen(reply)	);
										}
									}
								}
							}
							else
							{
								//D9
								int dice1=0;
								int dice2=0;
								int ctr;
								int result=0;
								//for(ctr=0;ctr<100;ctr++)
							//	{
									dice1=(rand()%10);
									dice2=(rand()%10);
							//	}
								DPRINT("!roll results: %d-%d",dice1,dice2);
								result=dice1-dice2;
								if(0==dice1 || 0==dice2)
								{
									result=0;
								}
								snprintf(reply,sizeof(reply),fmt,result);
								DPRINT(">> %s",reply);
								connection->send(connection,reply,strlen(reply)	);
							}
						}
					}
					if(!strcmp(cmd,matchcommands[1]))
					{
						//PONG:
						char *pingreply;
						size_t pongarglen=strlen(res->getparam(res,1));
						DPRINT("PING DETECTED - CREATING REPLY");
						pingreply=malloc( pongarglen+8); //+8 == PONG+space+\r+\n+\0
						snprintf(pingreply,pongarglen+8,"%s %s\r\n","PONG",res->getparam(res,1));
						DPRINT(">> \"%s\"",pingreply);
						connection->send(connection,pingreply,pongarglen+7);
					}
					DPRINT("Freeing result");
					res->gen.free(&res);
				}
			}
		}
		else
			EPRINT("Parser returned error! %d",pret);
		DPRINT("<< %s",buff);
		memset(buff,0,sizeof(buff));
	}
	

	connection->destroy(&connection);
	return 0;
}
