
/* ******************************************************** */
/*                                                          *
 *      Implementation of php's explode written in C        *
 *      Written by  Maz (2008)                              *
 *      http://maz-programmersdiary.blogspot.com/           *
 *                                                          *
 *      You're free to use this piece of code.              *
 *      You can also modify it freely, but if you           *
 *      improve this, you must write the improved code      *
 *      in comments at:                                     *
 *      http://maz-programmersdiary.blogspot.com/           *
 *      or at:                                              *
 *      http://c-ohjelmoijanajatuksia.blogspot.com/         *
 *      or mail the corrected version to me at              *
 *      Mazziesaccount@gmail.com                            *
 *                                                          *
 *      Revision History:                                   *
 *
 *      -v0.0.3 31.07.2009/Maz  Added Cexplode_concat
 *      						(untested)
 *  	-v0.0.2 21.07.2009/Maz  Some additions for better   *
 *  							usability in MazBotV4		*
 *      -v0.0.1 16.09.2008/Maz                              *
 *                                                          */
/* ******************************************************** */

#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct CexplodeStrings
{
    int amnt;
    char **strings;
	char *separator;
	int sepwasatend;
	int startedWdelim;
	int index;
}CexplodeStrings;

typedef enum ECexplodeRet
{
    ECexplodeRet_InternalFailure    = -666,
    ECexplodeRet_InvalidParams         = -667
}ECexplodeRet;

int Cexplode(const char *string,const char *delim, CexplodeStrings *exp_obj );
int Cexplode_nextexists(CexplodeStrings exp_obj);
char *Cexplode_getNth(int index,CexplodeStrings *exp_obj);
char *Cexplode_getfirst(CexplodeStrings *exp_obj);
char *Cexplode_getnext(CexplodeStrings *exp_obj);
char *Cexplode_getlast(CexplodeStrings *exp_obj);
void Cexplode_free(CexplodeStrings exp_obj);
size_t Cexplode_getlentilllast(CexplodeStrings exp_obj);
int Cexplode_sepwasatend(CexplodeStrings exp_obj);
int Cexplode_concat(CexplodeStrings *first,CexplodeStrings *second);

#endif
