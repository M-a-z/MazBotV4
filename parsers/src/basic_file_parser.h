/* ********************************************************************
 *
 * @file basic_file_parser.h
 * @brief functions to parse config files.
 *
 *
 * -Revision History:
 *
 *  - 0.0.1 17.08.2009/Maz  First Draft
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
#include <parser.h>
#include <config.h>
#include <generic.h>
#include <helpers.h>

struct SbasicParserResult;
struct SbasicFileParser;

typedef SbasicConfig *(*SbasicParserResultGetNextF)(struct SbasicParserResult *_this);
typedef struct SbasicParserResult
{
    SparserResult gen;
    mbot_linkedList * cfgs; //linked list of SbasicConfig structs
    SbasicParserResultGetNextF getNextCfgs;
}SbasicParserResult;

typedef void (*SbasicParserFreeF )(struct SbasicFileParser **_this);
typedef struct SbasicFileParser
{
    Sparser gen;
    SbasicParserFreeF free;
    SbasicParserResult *result;
}SbasicFileParser;

SbasicFileParser * BasicParserInit();


