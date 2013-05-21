/* ********************************************************************
 *
 * @file mbot_pseudo_xml.h
 * @brief MazBot pseudo xml implementation.
 *
 *
 * -Revision History:
 *
 *  -0.0.2  16.03.2010/Maz  Changed PSEUDOXML_ATTRIB_SEPARATOR to :\:, since \t is a bit clumsy
 *  -0.0.1  15.03.2010/Maz  Splitted from irc_config.h
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

#ifndef MBOT_PSEUDO_XML_H
#define MBOT_PSEUDO_XML_H

#define PSEUDOXML_ATTRIB_SEPARATOR ":\\:"

#include <stdio.h>

//static struct SmbotPseudoxmlTag;
typedef enum EmbotPseudoxmlType
{
	EmbotPseudoxmlType_char  = 1,
	EmbotPseudoxmlType_8bit  = 2,
	EmbotPseudoxmlType_16bit = 3,
	EmbotPseudoxmlType_32bit = 4,
	EmbotPseudoxmlType_64bit = 5,
	EmbotPseudoxmlType_nmbrof
}EmbotPseudoxmlType;


typedef struct SmbotPseudoxmlTag
{
	char *name;						///< tag name;
	EmbotPseudoxmlType valuetype;	///< tag value type
	unsigned int depth;
	unsigned int size;				///< value size
	void *value;
	int closed;
	struct SmbotPseudoxmlTag *next;		///< next tag in same level
	struct SmbotPseudoxmlTag *prev;		///< previous tag in same level
	struct SmbotPseudoxmlTag *first;		///< first tag in this level
	struct SmbotPseudoxmlTag *parenttag;		///< upper level tags - see example
	struct SmbotPseudoxmlTag *subtags;		///< list of tags belonging under this tag - see example @TODO: example
}SmbotPseudoxmlTag;

int get_tags(SmbotPseudoxmlTag **tag,FILE *cfgfile);
#endif

