#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *charmalloc(int len) {
	return (char*) calloc(len + 1, sizeof(char));
}

char *charrealloc(char *mem, int len) {
	return (char*)realloc(mem, (strlen(mem) + len + 1) * sizeof(char));
}

char *substring(char *str, int begin, int end) {
	int i, j;
	char *substr = charmalloc(end - begin + 1);
	for (i = begin, j = 0; i <= end; i++, j++) {
		substr[j] = str[i];
	}
	substr[j] = '\0';
	return substr;
}

/*void stringconcat2(char *str1, char *str2) {
	int i,j;
	for(i=0;str1[i];i++)
		i++;
	for(j=0;str2[j];j++,i++)
		str1[i] = str2[j];
	str1[i] = '\0';
}*/

