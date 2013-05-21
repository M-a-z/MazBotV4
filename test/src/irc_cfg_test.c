#define debug
#include <generic.h>
#include <irc_config.h>
#include <irc_abstraction_defines.h>

const char *DEFCFGFILE="irc_test_cfg.txt";



int main(int argc, char *argv[])
{
	int i;
	char *filename;
	int amntofservers;
	Sirc_servers **servers;
	SircConfig *cfg;
#ifndef debug
	if(argc>2)
	{
		printf("Usage: %s [cfgfilename]",argv[0]);
		return -1;
	}
#else
if(argc>2)
{
	int i;
	for(i=2;i<argc;i++)
	{
		if(!strcmp(argv[i],"-Qsplit"))
			disableprint(PrintComp_splitter);
		else if(!strcmp(argv[i],"-Qgen"))
			disableprint(PrintComp_gen);
		else if(!strcmp(argv[i],"-Qnetw"))
			disableprint(PrintComp_netw);
		else if(!strcmp(argv[i],"-Qcexp"))
			disableprint(PrintComp_Cexpl);
		else if(!strcmp(argv[i],"-Qparse"))
			disableprint(PrintComp_Parser);
		else if(!strcmp(argv[i],"-Qcfg"))
			disableprint(PrintComp_IrcCfg);
		else if(!strcmp(argv[i],"-Qpackarr"))
			disableprint(PrintComp_PackAr);
		else if(!strcmp(argv[i],"-Qbitset"))
			disableprint(PrintComp_bitset);
		else
		{
			EPRINT("Invalid startup param. First param is config file name, rest are used to limit prints from components. Supported params:");
			printf("-Qsplit\n-Qgen\n-Qnetw\n-Qcexp\n-Qparse\n-Qcfg\n-Qpackarr\n-Qbitset\n");
			return -1;
		}
	}
}
#endif
	if(argc>1)
	{
		filename=argv[1];
	}
	else
		filename=(char *)DEFCFGFILE;
	cfg=ircConfigInit(filename);
	if(NULL==cfg)
	{
		EPRINT("FAILED to init SircConfig *");
		return -1;
	}
	DPRINT("SircConfig * inited");
	DPRINT("Trying to read cfg file %s && parse tags:",filename);
	if(0!=cfg->IrcConfigRead(cfg))
	{
		EPRINT("Cfg parsing FAILED!");
		return -1;
	}
	DPRINT("CFG parsing seemingly successfull!!");
	amntofservers=cfg->ircConfigGetServeramnt(cfg);
	DPRINT("Found %d servers",amntofservers);
	DPRINT("Trying server initializations");
	servers=calloc(amntofservers,sizeof(servers));
	if(NULL==servers)
	{
		PPRINT("calloc FAILED - aborting");
		return -1;
	}
	for(i=0;i<amntofservers;i++)
	{
		servers[i]=InitSirc_server(cfg->ircConfigGetServer(cfg,i));
		if(NULL==servers[i])
		{
			EPRINT("Server %d init FAILED!",i);
			return -1;
		}
	}
    /* Servers, channels and events  should be initialized now */
    /* Callback registration ??? Maybe it would be better to do it before the connection + add it in config struct(s) */
    /* Actually, that is propably the best idea now... */
    /* Then we shoul be ready to start the main loop with data listening && callback executions */
    for(;;)
    {
        int serverstate;
        for(i=0;i<amntofservers;i++)
        {
            if((serverstate=servers[i]->process_data(servers[i])))
            {
                switch(serverstate)
                {
                    case EServerSuccess_Disconnected:
						PPRINT("Server %s disconnected! aborting!",servers[i]->servername);
						return EXIT_FAILURE;
//                        servers[i]->reconnect...
                        break;
//                    case EServerState_Fatal:
//                        PPRINT("Fatal error occurred during handling server %s, aborting",servers[i]->domain);
//                        return -1;
//                        break;
                    default:
//                    /* TODO: Build proper recovery actions
//                        EPRINT("Unknown error in server %s",servers[i]->domain);
//                        EPRINT("aborting");
//                        return -1;
                        break;
                }
            }
            else
            {
                /* Normal operation, go on with next server */
            }
        }
    }
	return 0;
}

