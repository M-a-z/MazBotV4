#include <parser.h>
#include <irc_protocol_parser.h>
#include <config_file_parser.h>
#include <basic_file_parser.h>
Sparser *ParserInit(EparserType type)
{
	switch(type)
	{
		case EparserType_Irc:
			return (Sparser *)IRCparserinit();
			break;
		case EparserType_MazBotProto:
			MAZZERT(0,"EparserType_MazBotProto not yet implemented!");
			break;
        case EparserType_CfgCallback:
            return (Sparser *)CfgCbFileParserInit();
            break;
        case EparserType_CfgBasic:
            return (Sparser *)BasicParserInit();
            break;
		default:
			PPRINT("Unknown parsertype requested!");
			break;
	}
	return NULL;
}
