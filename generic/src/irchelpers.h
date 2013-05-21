#ifndef MBOT_IRCHELPERS_H
#define MBOT_IRCHELPERS_H
#include <sys/types.h>
#include <stdarg.h>

char *prepare_for_sending(size_t *sendsize, const char *fmt,...);

int copyprefixtonickmask(char *prefix, char **nick, char **mask);

#endif

