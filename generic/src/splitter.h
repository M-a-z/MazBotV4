/* ********************************************************************
 *
 * @file splitter.h
 * @brief Helper for parsing config files.
 *
 *
 * -Revision History:
 *
 *  - 0.0.2 02.08.2009/Maz  Seems to work (at least when not called recursilvely)
 *							- leaks memory though. TODO: Specify way to get results.
 *  - 0.0.1 30.07.2009/Maz  First draft
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


#ifndef MAZBOT_V4_SPLITTER_H
#define MAZBOT_V4_SPLITTER_H

#include "helpers.h"
#include "generic.h"
#include "splitter.h"

struct SSplitter;
struct SSplitterResult;
typedef enum EsplitterState
{
	EsplitterState_Inited 	= 0,
	EsplitterState_Feed		= 1,
	EsplitterState_Splitted	= 2
}EsplitterState;

/* Get's splitted items from splitter result struct (returned by splitter->split function ) in CexplodeStrings struct, 
 * compatible with Cexplode_* functions 
 * Each call will return splits relative to first given delimiter (See example below since explanation is bad)
 * When last "row's" splits have been returned, next call shall return NULL
 * Exmple: Original data "1:2:3:4 5:6:7 8:9
 * Split requested with delimiters ' ' and ':'
 * Result struct obtained using split
 * first get result shall return CexplodStrings containing splits 1,2,3 and 4
 * Next call shall return 5,6 and 7
 * next call shall return 8 and 9
 * next (and further calls) shall return NULL
 */
typedef CexplodeStrings *(*splitterResGetF)(struct SSplitterResult *);
/*
 * Frees resources allocated for result struct and NULLs the pointer. May also free returned Cexplode pieces!!
 * TODO: Check if pieces are freed, and if so, create optional free which does not free the pieces!"
 */
typedef void (*splitterResFreeF)(struct SSplitterResult **);
/*
 * Initializes splitter results - for splitter internal use only!!
 */
typedef struct SSplitterResult * (*splitterResInitF)(CexplodeStrings *,int);
/*
 * Gets splits matching Nth "row". Updates internal iterator, so if splitterGetResF is called after this, it shall return result
 * N+1
 */
typedef CexplodeStrings *(*splitterResGetNthF)(struct SSplitterResult *,int);
/*
 * Struct holding results for splitted data
 */
typedef struct SSplitterResult
{
	int resamnt;
	int iterator;
	splitterResGetF get;
	splitterResGetNthF getNth;
	splitterResInitF init;
	splitterResFreeF free;
	CexplodeStrings *CexpResult;
}SSplitterResult;

//TODO: Check if splitter works when it is fed multiple times!
//Eg. sequence 
//init()
//feed()
//split()
//feed()
//split()
//feed()...
//uninit()
//
//At the moment only sequence
//init()
//feed()
//split()
//uninit()
//is tested.

/*
 * Initializes splitter for use
 */
typedef struct SSplitter* (*SplitterInitF) (char **separators,int amntOfSeps);
/*
 * Uninitializes splitter (frees resources allocated for splitter.
 */
typedef void (*SplitterUninitF)(struct SSplitter **);
/*
 * Performs the split for feed data according to split delimiters given in init.
 * Returns result struct containing all the splitted pieces.
 */
typedef SSplitterResult * (*SplitterSplitF)(struct SSplitter *);
/*
 * Feed data to be splitted
 */
typedef int (*SplitterFeedF)(struct SSplitter *_this,char *data);
/* Splitter struct, returned by SSplitter * SplitterInit(char **separators,int amntOfSeps); */ 
typedef struct SSplitter
{
	EsplitterState state;
    SplitterUninitF uninit;
	SplitterInitF init;
    SplitterSplitF split;
    SplitterFeedF feed;
	CexplodeStrings splitted;
	int sepamnt;
    int *result_valid;
	char **separators;
	char *splitme;
	int splitdepth;
	int *splitno;
	int nmbrofsplits;
/*	int *matchrange_low;
	int *matchrange_high;
*/	int *explosionindex;
	int amntoflines;
	int splitsinthisrow;
	CexplodeStrings *splits;
	SSplitterResult *res;
}SSplitter;

SSplitter * SplitterInit(char **separators,int amntOfSeps);
SSplitterResult * splitteResInit(CexplodeStrings *strings,int amnt);
#endif //MAZBOT_V4_SPLITTER_H

