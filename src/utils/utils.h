#ifndef UTILS_H
#define UTILS_H

#include "log.h"

char *charmalloc(int len);

char *substring(char *string, int begin, int end);

//void stringconcat(char *str1, char *str2);

char *charrealloc(char *mem, int len);

#endif
