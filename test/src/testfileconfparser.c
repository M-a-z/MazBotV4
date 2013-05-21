//#include <APIs/generic_api.h>
//#include <APIs/networking.h>
#include <generic.h>
#include <parser.h>
#include <config_file_parser.h>


int main(void)
{
    Sparser *fileparser;
    EparserState state;
    SConfigResult_Callback *cfgres;
    ScallbackConf *callbackdetails;

    fileparser=ParserInit(EparserType_CfgCallback);
    MAZZERT(NULL!=fileparser,"ParserInit FAILED");
    DPRINT("Parser Inited");
    fileparser->feed(fileparser,"testfile",strlen("testfile"),&state);
    MAZZERT(state==EparserState_ResultReady,"Bad state returned by parser - expected EparserState_ResultReady");
    DPRINT("Parser Fed");
    cfgres=fileparser->get_result(fileparser,&state);
    MAZZERT(state==EparserState_ParserEmpty,"Bad state returned by parser - expected EparserState_ParserEmpty");
    DPRINT("File Parsed");
    while(NULL!=(callbackdetails=cfgres->eventGet(cfgres)))
    {
        int i;
        DPRINT("Details for id%d:",callbackdetails->id);
        DPRINT("type=%d\noriginator=\"%s\"\ntriggerstring=\"%s\"\n",
            callbackdetails->type,
            callbackdetails->originator,
                (NULL==callbackdetails->triggerstring)?
                    "NULL":callbackdetails->triggerstring
        );
        for(i=0;i<callbackdetails->amntOfLocations;i++)
        {
            printf("location %d, %d",i,callbackdetails->locationoftxtevent[i]);
        }
    }
    return 0;
}








