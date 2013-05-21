#include <startuplevels.h>
#include <generic.h>

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
	DPRINT("Oh Joy, I would be Up now!");

    getoonbaby(configGetServeramnt());

	deinit_startlevel5();
	return 0;
}
//Just a pseudocode used for planning
//It is better to make one large struct containing all one server specific datas, like userdetails,
//
void getoonbaby(int amntofservers)
{
    //Use something else but array so there'll be no need to move things around when some server disconnects
    DaHugeStruct *serverspecifics[amntofservers];
    int i;
    //Get server and channel configs from config storage
    //Create connection objects and connect them
    //join channels

    connect_em_all(serverspecifics,&amntofservers); //If some server is not connectible, update amntofservers
    if(Regged+nic==userIdentificationMode)
    {
        for(i=0;i<amntofservers;i++)
            serverspecifics[i].do_initial_whoises() //Set user details

    }
    while(1)
    {
        for(i=0;i<amntofservers;i++)
        {
            if(endcondition)
            {
                break;
            }
            if(!serverspecifics[i].non_blocking_recv(&data_received))
            {
                //connection dropped?
                serverspecifics[i].try_reconnect();
                if(reconfails)
                serverspecifics[i].remove();
                amntofservers--;
            }
            if(data_received)
            {
                serverspecifics[i].parse_data();
                if(notenoughdataforparsing)
                    continue;
                serverspecifics[i].filter_event();
                if(NULL!=event)
                    serverspecifics[i].execute_callback();
            }
        }
        for(i=0;i<amntofservers;i++)
        {
            serverspecifics[i].wait_callbacks(TIMEOUT);
            if(callbacknotfinished)
                serverspecifics[i].terminate_callbackthreads();
            serverspecifics[i].remove();
        }

    }
    //Return for deiniting runlevels
}
