#include <startuplevels.h>
#include <generic.h>


void run_the_bot();

int main(void)
{
	DPRINT("Initing startlevel 1");
	if(0!=init_startlevel1())
	{
		PPRINT("Failed to init startlevel 1");
	}
	DPRINT("Initing startlevel 2");
	if(0!=init_startlevel2())
	{
		PPRINT("Failed to init startlevel 2");
	}
	DPRINT("Initing startlevel 3");
	if(0!=init_startlevel3())
	{
		PPRINT("Failed to init startlevel 3");
	}
	DPRINT("Initing startlevel 4");
	if(0!=init_startlevel4())
	{
		PPRINT("Failed to init startlevel 4");
	}
	DPRINT("Initing startlevel 5");
	if(0!=init_startlevel5())
	{
		PPRINT("Failed to init startlevel 5");
	}
	run_the_bot(); 
	DPRINT("Going down!");
	deinit_startlevel5();
	return 0;
}



/* TODO: Encapsulate this functionality into Sirc_servchan */


int getConnections(Sconn **servers,int *srvamnt)
{
	int serveramnt;
	int rejected=0;
	char *servers_txt;
	size_t cfgsize;
	CexplodeStrings servploder;
	char *srvport;
	char *srv
	short port;
	int serverindex=0;
	servers_txt=GetConfig(E_serverlist,&cfgsize);
	if(cfgsize<=1)
	{
		PPRINT("Invalid servers to connect!");
		return;
	}
	if(1>(serveramnt=Cexplode(servers_txt,",",&servploder)))
	{
		PPRINT("parsing servers FAILED! (given servers %s)",servers);
		return;
	}
	*servers=malloc(serveramnt*sizeof(Sconn *));
	for(i=0;i<serveramnt;i++)
	{
		servers[i]=connInit(EstructType_TCPconn);
		if(NULL==servers[i])
		{
			/* This is propably alloc failure => All hope is probs lost */
			PPRINT("Failed to init server struct %d!",i);
			*srvamnt=i;
			return -1;
		}
	}
	srvport=Cexplode_getfirst(&servploder);
	while(NULL!=srvport)
	{
		int i=0;
		while(1)
		{
			if('\0'==srvport[i] || '\0'==srvport[i+1])
			{
				EPRINT("Invalid server spec (no port given?), no port found for %s! => Rejecting server",srvport);
				rejected++;
				break;
			}
			if(':'==srvport[i])
			{
				char *port_txt;
				char *port_txt_end;
				short int port;
				srvport[i]='\0';
				port_txt=&(srvport[i+1]);
				port=(unsigned short)strtol(port_txt,&port_txt_end,10);
				if('\0'!=*port_txt_end )
				{
					EPRINT("Invalid server spec (bad port given?), no port found for %s! => Rejecting server",srvport);
					rejected++;
					break;
				}
				if( EnetwRetval_Ok != servers[serverindex]->create_socket(servers[serverindex]) )
				{
					EPRINT("Socket creation FAILED for server %s",srvport);
					rejected++;
					break;
				}
				for
				(
					attempts=0; 
					attempts<5 && 
					EnetwRetval_Ok != 
					(
						connret=servers[serverindex]->connect
						(
							servers[serverindex],
							srvport,
							port
						)
					);
					attempts++
				)
				{
					WPRINT("Connection attempt %d to %s:%d FAILED",attempts+1,srvport,port);
					sleep(1);
				}
				if(connret != EnetwRetval_Ok)
				{
					EPRINT("Could not connect to server %s port %d => Rejecting",srvport,port);
					rejected++;
					break;
				}
				DPRINT("server %s port %d connected!",srvport,port);
				serverindex++;
				break;
			}
		}
        srvport=Cexplode_getnext(&servploder);
	}
	*srvamnt=serveramnt-rejected;
	return !*srvamnt;
}

int get_default_callbacks(serverstructs,srvamnt)
{
	int ok=1;
	Sirc_servchan *srvtohandle=NULL;
	successcode=0;
	FILE *readfile;
	char *filename;
	size_t filenamesize;
	if(NULL==(filename=GetConfig(E_event_def_reply_file,&filenamesize)))
	{
		filename=EVENT_DEF_REPLY_FILE_DEFAULT;
//		testfilename_size=strlen(testfilename)+1;
	}
	readfile=fopen(filename,"r");

	if(NULL==readfile)
	{
		perror("file opening failed!\n");
		printf("Bad cofig file %s\n",filename);
		successcode = -1;
	}
	else
	{
		while(ok)
		{
			int scanned;
			char *srv;
			char *reply;
			int eventId;
			int srv_index;
			if(scanned>0)
			{
				free(reply);
				continue;
			}
			scanned=fscanf(readfile,"[SERVER=%a]\n",&srv);
			if(scanned>0)
			{
				unsigned int srvlen=strlen(srv);
				for(srv_index=0;srv_index<srvamnt;srv_index++)
				{
					if(strlen(serverstructs[srv_index]->...)==srvlen)
					{
						if(!memcmp(serverstructs[srv_index]->...,srv,srvlen))
						{
							/* Server found! */
							srvtohandle=serverstructs[srv_index];
							break;
						}
						else
							srvtohandle=NULL;
					}
					else
						srvtohandle=NULL;
				}
				if(NULL==srvtohandle)
				{
					WPRINT("No server matching %s FOUND!",srv);
					srv=NULL;
					continue;
				}
				free(srv);
			}
			while(ok && 0==(scanned=fscanf(readfile,"[/SERVER]\n",&reply)))
			{
				int retval;
				scanned=fscanf(readfile,"[CHANNEL=%a]\n",&reply);

				if(scanned>0)
				{
					retval=srvtohandle->add_channel_def_events(srvtohandle,reply,readfile);
					if(0==retval)
					{
						//Ok, chan added, chan end tag found.
						continue;
					}
					if(1==retval)
					{
						/* EOF?? */
						ok=0;
						break;
					}
					if(-1==retval)
					{
						/* Error while parsing file */
						ok=0;
						successcode = -1;
						break;
					}
					free(reply);
					break;
				}
			}
			if(EOF==scanned)
			{
				/* End reached */
				ok=0;
			}
		}
	}
	fclose(readfile);
	return successcode;
}




void run_the_bot()
{
	int srvamnt;
	char *original_amnt;
	int orig_amnt;
	int dummy;
	int i;
	Sirc_servers **serverstructs;
	SConn **connections;
	/* Get channel names */

	/* Get server names and connect */
	if(0!=getConnections(&srvamnt,connections))
	{
		DPRINT("No Connections alive! giving up!");
		return;
	}
	serverstructs=calloc(srvamnt,sizeof(Sirc_servers *));
	if(NULL==serverstructs)
	{
		PPRINT("malloc() FAILED at %s:%d",__FILE__,__LINE__);
		return;
	}
	for(i=0;i<srvamnt;i++)
	{
		serverstructs[i]=IrcServerStructInit(connections[i]);
		if(NULL==serverstructs[i])
		{
			PPRINT("Server struct init FAILED!");
			return;
		}
		serverstructs[i]->GetChannelsToJoin(serverstructs[i]);
	}
	free(connections);
	connections=NULL;
	original_amnt=GetConfig(E_serveramnt,original_amnt,&dummy);
/*	if((orig_amnt=atoi(original_amnt))!=srvamnt)
	{
	*/
		MAZZERT(orig_amnt>=srvamnt,"Something has messed up the server configs!, invalid serveramnt?");
		/* Not all connections succeeded => there propably is chans to filter. */
		/* TODO: Do filtering! */
//	}
	if(0>get_default_callbacks(serverstructs,srvamnt))
	{
		PPRINT("Error occurred while getting default callbacks!");
		return;
	}
	/* Create OnlineUserStorages for each channel */

	/* Initiate IRC handshakes */

	/* Execute startup callbacks ? */

	/* Join to channels and do whoises */

	/* Fill online storage */

	/* Start the callback engine, and foreverloop */
	DPRINT("Oh Joy, I would be Up now!");
}
