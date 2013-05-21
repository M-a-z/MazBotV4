#include <APIs/parsers_api.h>
#include <APIs/generic_api.h>
#include <string.h>

int main(void)
{
	char test1[]=":jfsdojfdaio@nfdand PRIVMSG hsdao hsdaoo ashdohsaohsado ashod\r\nFOOBAR akljsakldajdlssjadl\r\njasjdlkjasdlk";
	char test2[]=" jdalsjsadl\r\n:jasdjasdl adjslkjsalk jaskljdsakld\r\n";
	int i;
	EparserState state;
	Sparser *par = ParserInit(EparserType_Irc);
	if(NULL==par)
	{
		EPRINT("OMG Init Failed!");
		return -1;
	}
	else
		DPRINT("YAY! parser inited!");
	state = EparserState_Inited;
	for(i=0;i<2;i++)
	{
		while(state != EparserState_ResultReady)
		{
			EparserRetVal rv;
			if(EparserRetVal_Ok!=(rv=par->feed(par,(i)?test2:test1,(i)?strlen(test2):strlen(test1),&state)))
			{
				EPRINT("YaY! Parser returned errorcode %d",rv);
			}
		}
		while(EparserState_ResultReady == state)
		{
			SIRCparserResult *res;	
			char *tmp;
			res=(SIRCparserResult *)par->get_result((Sparser *)par,&state);
			if(NULL==res)
			{
				EPRINT("ERROR TERROR, NULL result!");
				return -1;
			}
			IPRINT("Parsed!");
			tmp=res->getprefix(res);
			IPRINT("Prefix: %s",(NULL==tmp)?"No prefix":tmp);
			IPRINT("Command: %s",res->getcmd(res));
			tmp=res->getparam(res,1);
			for(i=2;NULL!=tmp;i++)
			{
				IPRINT("param %d %s",i,tmp);
				tmp=res->getparam(res,i);
			}
			res->gen.free(&res);
		}
	}
	return 0;
}
