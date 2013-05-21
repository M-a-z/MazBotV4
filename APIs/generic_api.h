/* ********************************************************************
 *
 * @file generic_api.h
 * @brief generic helper functions which I always end up redoing.
 * Why on earth one never stores these? I need to rewrite these 
 * for each project I start...
 *
 *
 * -Revision History:
 * 
 *  22.07.2009/Maz  First draft
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


#ifndef MAZBOT_V_4_GENERIC_API_H
#ifndef MAZB_GENERIC_H
#define MAZBOT_V_4_GENERIC_API_H

#include "cexplode.h"
//#include <../global_defs/definitions.h>
#ifdef __cpp
extern "C"
{
#endif

#define ASSERT_ENABLE
#define MAZ_TRUE 1
#define MAZ_FALSE 0
typedef int MAZ_BOOLEAN;


#ifdef ASSERT_ENABLE
/**
	@brief Assertion macro. Prints explanation and filr&line where call occurred when assertions are enabled.
	Assertion macro. Prints explanation and filr&line where call occurred when assertions are enabled.
	Assertions can be turned on using ASSERT_ENABLE definition. Assertion occurs when first param is non true.
	
	@param ok Condition wheteher or not assertion should be raised. FALSE == raise, TRUE == ignore
	@param explanation explanation displayed in log prints.
	@returns When assertions are enabled, and conditional param is FALSE, call will not return. No returnvalue is specified.
*/
    #define MAZZERT(ok,explanation) \
    {\
        if(!(ok))\
            mazzert((explanation),__FILE__,__LINE__);\
        else\
        {\
        }\
    }
    void mazzert(char *explanation,char *file,int line);
#else
    #define MAZZERT(ok,explanation)
#endif
/*
#define LVL_TXT_LEN 9
#define LVL1_TXT "DEBUG:  " 
#define LVL2_TXT "INFO:   "
#define LVL3_TXT "WARNING:"
#define LVL4_TXT "ERROR:  "
#define LVL5_TXT "PANIC:  "
#define LVL6_TXT "UNKNOWN:"
*/
/**
	@brief Debug level print
	Prints specified string (as printf) appended with timestamp and severity level. Currently supports only printing to console.
*/
#define DPRINT(fmt,...) MazPrint(EPrintLevel_Debug,fmt,##__VA_ARGS__)
/**
	@brief Info level print
	Prints specified string (as printf) appended with timestamp and severity level. Currently supports only printing to console.
*/
#define IPRINT(fmt,...) MazPrint(EPrintLevel_Info,fmt,##__VA_ARGS__)
/**
	@brief Warning level print
	Prints specified string (as printf) appended with timestamp and severity level. Currently supports only printing to console.
*/
#define WPRINT(fmt,...) MazPrint(EPrintLevel_Warning,fmt,##__VA_ARGS__)
/**
	@brief Error level print
	Prints specified string (as printf) appended with timestamp and severity level. Currently supports only printing to console.
*/
#define EPRINT(fmt,...) MazPrint(EPrintLevel_Error,fmt,##__VA_ARGS__)
/**
	@brief Panic level print
	Prints specified string (as printf) appended with timestamp and severity level. Currently supports only printing to console.
*/
#define PPRINT(fmt,...) MazPrint(EPrintLevel_Panic,fmt,##__VA_ARGS__)


/**
 * @brief enum specifying print levels from debug to panic.
 */
typedef enum EPrintLevel
{
    EPrintLevel_Debug = 0,
    EPrintLevel_Info,
    EPrintLevel_Warning,
    EPrintLevel_Error,
    EPrintLevel_Panic,
    EPrintLevel_Unknown,
    EPrintLevel_NmbrOf
}EPrintLevel;

/**
 * @brief Sets filter level for prints. 
 * User can specify which severity level prints will be published, and which will be silently ignored. 
 * Default filtering depends on library's compile time settings. 
 *
 * @param EPrintLevel level Lowest priority level for prints being processed
 */
void MazPrintSetLevel(EPrintLevel level);
void MazPrint(EPrintLevel level, const char *fmt,...);

#ifdef __cpp
}
#endif //__cpp
#endif
#endif

