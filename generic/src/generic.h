/* ********************************************************************
 *
 * @file generic.h
 * @brief generic helper functions which I always end up redoing.
 * Why on earth one never stores these? I need to rewrite these 
 * for each project I start...
 *
 *
 * -Revision History:
 *
 *  18.02.2010/Maz  Added filename and line print in error & panic prints
 *                  Added documentation
 *  12.08.2009/Maz  Added MBOT_NULLSAFE_FREE()
 *  12.07.2009/Maz  Fixed XPRINT() macros to accept only one arg.
 *  10.07.2009/Maz  First draft
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


#ifndef MAZB_GENERIC_H
#define MAZB_GENERIC_H

#include "definitions.h"
#include <errno.h>
#ifdef ASSERT_ENABLE
/**
 * @brief Assertion macro
 *
 * Id ASSERT_ENABLE is defined and assertion is called, the condition (ok) is evaluated. If condition is false, 
 * program will be killed with given explanation string, appended with filename and line where assertion was called. 
 * If confition is true, or ASSERT_ENABLE is not defined, execution returns straight back to the point where MAZZERT was called.
 *
 * @param ok Condition to be evaluated.
 * @param explanation, pointer to NULL terminated character array which contents will be printed if condition (ok) is false.
 *
 * @returns void if condition is true, or if ASSERT_ENABLE is not defined. If ASSERT_ENABLE is defined, and condition is false, the call shall not return.
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
#define LVL_TXT_LEN 9
#define LVL1_TXT "DEBUG:  " 
#define LVL2_TXT "INFO:   "
#define LVL3_TXT "WARNING:"
#define LVL4_TXT "ERROR:  "
#define LVL5_TXT "PANIC:  "
#define LVL6_TXT "UNKNOWN:"

typedef enum PrintComp
{
	PrintComp_gen = 0,
	PrintComp_netw = 1,
	PrintComp_splitter = 2,
	PrintComp_Cexpl = 3,
	PrintComp_Parser = 4,
	PrintComp_IrcCfg = 5,
	PrintComp_PackAr = 6,
	PrintComp_bitset = 7,
	PrintComp_ringbuff = 7,
	PrintComp_callback = 8,
	PrintComp_Last
}PrintComp;


void disableprint(PrintComp cmp);
int isdisabled(PrintComp cmp);
/**
 * @brief Printing macro for debug prints
 *
 * Macro shall print timestamp and given debug string to enabled print devices - if printlevel is set to show debugprints
 * @see MazPrintSetLevel,EPrintLevel,IPRINT,WPRINT,EPRINT,PPRINT
 */
#define DPRINTC(comp,fmt,...) \
{ \
	if(!isdisabled( comp ))\
	{\
		MazPrint(EPrintLevel_Debug,fmt,##__VA_ARGS__); \
	}\
	else\
	{;}\
}
/**
 * @brief Printing macro for info prints
 *
 * Macro shall print timestamp and given info string to enabled print devices - if printlevel is set to show infoprints
 * @see MazPrintSetLevel,EPrintLevel,DPRINT,WPRINT,EPRINT,PPRINT
 */
#define IPRINTC(comp,fmt,...) \
{ \
	if(!isdisabled(comp))\
	{\
		MazPrint(EPrintLevel_Info,fmt,##__VA_ARGS__); \
	}\
	else\
	{;}\
}
/**
 * @brief Printing macro for warning prints
 *
 * Macro shall print timestamp and given warning string to enabled print devices - if printlevel is set to show warningprints
 * @see MazPrintSetLevel,EPrintLevel,DPRINT,IPRINT,EPRINT,PPRINT
 */
#define WPRINTC(comp,fmt,...) \
{ \
	if(!isdisabled(comp))\
	{\
		MazPrint(EPrintLevel_Warning,fmt,##__VA_ARGS__); \
	}\
	else\
	{;}\
}
/**
 * @brief Printing macro for error prints
 *
 * Macro shall print timestamp and given error string to enabled print devices - if printlevel is set to show errorprints
 * @see MazPrintSetLevel,EPrintLevel,DPRINT,IPRINT,WPRINT,PPRINT
 */
#define EPRINTC(comp,fmt,...) MazPrint2(EPrintLevel_Error,__FILE__,__LINE__,fmt,##__VA_ARGS__)
/*
 * \
{ \
	if(!isdisabled(comp))\
	{\
		MazPrint2(EPrintLevel_Error,__FILE__,__LINE__,fmt,##__VA_ARGS__); \
	}\
	else\
	{;}\
}
*/
/**
 * @brief Printing macro for debug prints
 *
 * Macro shall print timestamp and given panic print string to enabled print devices.
 * @see MazPrintSetLevel,EPrintLevel,DPRINT,IPRINT,WPRINT,EPRINT
 */
#define PPRINTC(comp,fmt,...) MazPrint2(EPrintLevel_Panic,__FILE__,__LINE__,fmt,##__VA_ARGS__)
/*
\
{ \
	if(!isdisabled(comp))\
	{\
		MazPrint2(EPrintLevel_Panic,__FILE__,__LINE__,fmt,##__VA_ARGS__); \
	}\
	else\
	{;}\
}
*/
















/**
 * @brief Printing macro for debug prints
 *
 * Macro shall print timestamp and given debug string to enabled print devices - if printlevel is set to show debugprints
 * @see MazPrintSetLevel,EPrintLevel,IPRINT,WPRINT,EPRINT,PPRINT
 */
#define DPRINT(fmt,...) MazPrint(EPrintLevel_Debug,fmt,##__VA_ARGS__)
/**
 * @brief Printing macro for info prints
 *
 * Macro shall print timestamp and given info string to enabled print devices - if printlevel is set to show infoprints
 * @see MazPrintSetLevel,EPrintLevel,DPRINT,WPRINT,EPRINT,PPRINT
 */
#define IPRINT(fmt,...) MazPrint(EPrintLevel_Info,fmt,##__VA_ARGS__)
/**
 * @brief Printing macro for warning prints
 *
 * Macro shall print timestamp and given warning string to enabled print devices - if printlevel is set to show warningprints
 * @see MazPrintSetLevel,EPrintLevel,DPRINT,IPRINT,EPRINT,PPRINT
 */
#define WPRINT(fmt,...) MazPrint(EPrintLevel_Warning,fmt,##__VA_ARGS__)
/**
 * @brief Printing macro for error prints
 *
 * Macro shall print timestamp and given error string to enabled print devices - if printlevel is set to show errorprints
 * @see MazPrintSetLevel,EPrintLevel,DPRINT,IPRINT,WPRINT,PPRINT
 */
#define EPRINT(fmt,...) MazPrint2(EPrintLevel_Error,__FILE__,__LINE__,fmt,##__VA_ARGS__)
/**
 * @brief Printing macro for debug prints
 *
 * Macro shall print timestamp and given panic print string to enabled print devices.
 * @see MazPrintSetLevel,EPrintLevel,DPRINT,IPRINT,WPRINT,EPRINT
 */
#define PPRINT(fmt,...) MazPrint2(EPrintLevel_Panic,__FILE__,__LINE__,fmt,##__VA_ARGS__)

/**
 * @brief macro to call free to given pointer, if it differs from NULL
 */
#define MBOT_NULLSAFE_FREE(data) \
{\
	void *MBOT_NULLSAFE_FREE_TMP=(data);\
	if(MBOT_NULLSAFE_FREE_TMP!=NULL)\
		free(MBOT_NULLSAFE_FREE_TMP);\
	else\
	;\
}

/**
 * @brief enumeration defining print levels
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
 * @brief controll which level of prints shall be outputted
 *
 * @param EPrintLevel This level prints and higher level prints shall be outputed. Lower level prints shall be ignored.
 */
void MazPrintSetLevel(EPrintLevel level);
void MazPrint(EPrintLevel level, const char *fmt,...)__attribute__((format(printf,2,3)));
void MazPrint2(EPrintLevel level,char *file,unsigned int line,const char *fmt,...)__attribute__((format(printf,4,5)));

#endif //MAZB_GENERIC_H
