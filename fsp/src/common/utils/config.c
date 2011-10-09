#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "utils.h"

int xmlFileGetFileSize(t_xmlFile *xmlFile) {
	int fsize;
	fseek(xmlFile->f, 0, SEEK_END);
	fsize = ftell(xmlFile->f);
	rewind(xmlFile->f);
	return fsize;
}

char* xmlFileReadFile(t_xmlFile *xmlFile, int from, int to) {
	int len = to - from;
	char* buffer = (char*) malloc(len + 1);
	fseek(xmlFile->f, from, SEEK_SET);
	fread(buffer, sizeof(char), len, xmlFile->f);
	buffer[len] = '\0';
	return buffer;
}

char *xmlFileGetEndSection(const char *section) {
	int i;
	char* endSection = charmalloc(strlen(section)+1);
	endSection[0] = '/';
	for (i = 1; section[i - 1]; i++)
		endSection[i] = section[i - 1];
	endSection[i] = '\0';
	return endSection;
}

char *xmlFileGetParam(t_xmlFile *xmlFile, const char* param) {
	char* paramValue;
	int beginSection = strstr(xmlFile->Content, param) - xmlFile->Content + strlen(param) + 1;
	char* endParam = xmlFileGetEndSection(param);
	int endSection = strstr(xmlFile->Content, endParam) - xmlFile->Content - 2;
	free(endParam);
	if (beginSection < 0 || endSection < 0)
		return NULL;

	paramValue = substring(xmlFile->Content, beginSection, endSection);
	return paramValue;
}

t_xmlFile *newXmlFile(char *path) {
	t_xmlFile *xmlFile = (t_xmlFile*)malloc(sizeof(t_xmlFile));
	xmlFile->f = fopen(path, "r+");
	xmlFile->Size = xmlFileGetFileSize(xmlFile);
	xmlFile->Content = xmlFileReadFile(xmlFile, 0, xmlFile->Size);
	return xmlFile;
}

void freeXmlFile(t_xmlFile *xmlFile) {
	free(xmlFile->Content);
	free(xmlFile);
}

t_xmlFile *loadConfig(char* path) {
	return newXmlFile(path);
}

char *xmlStreamGetParam(char *xmlStream, const char *param) {
	char* paramValue;
	int beginSection = strstr(xmlStream, param) - xmlStream + strlen(param) + 1;
	char* endParam = xmlFileGetEndSection(param);
	int endSection = strstr(xmlStream, endParam) - xmlStream - 2;
	free(endParam);
	if (beginSection < 0 || endSection < 0)
		return NULL;

	paramValue = substring(xmlStream, beginSection, endSection);
	return paramValue;
}

config_fsp *xmlGetConfigStruct(t_xmlFile *xmlFile) {
	config_fsp *xmlParam = (config_fsp*)malloc(sizeof(config_fsp));
	strcpy(xmlParam->ip,xmlFileGetParam(xmlFile, "ip"));
	xmlParam->port = xmlFileGetParam(xmlFile, "port");
	xmlParam->maxConnections = xmlFileGetParam(xmlFile, "maxConnections");
	xmlParam->sizeCache = xmlFileGetParam(xmlFile, "sizeCache");
	return xmlParam;
}

