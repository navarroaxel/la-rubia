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

config_disk *xmlGetConfigStruct(t_xmlFile *xmlFile) {
	config_disk *xmlParam = (config_disk*)malloc(sizeof(config_disk));
	strcpy(xmlParam->ip,xmlFileGetParam(xmlFile, "ip"));
	xmlParam->portProc = xmlFileGetParam(xmlFile, "portproc");
	strcpy(xmlParam->mode,xmlFileGetParam(xmlFile, "mode"));
	strcpy(xmlParam->algorithm,xmlFileGetParam(xmlFile, "algorithm"));
	xmlParam->portConsole = xmlFileGetParam(xmlFile, "portconsole");
	xmlParam->logEnabled = xmlFileGetParam(xmlFile, "logenabled");
	xmlParam->diskId = xmlFileGetParam(xmlFile, "diskid");
	xmlParam->chs = xmlFileGetParam(xmlFile, "chs");
	xmlParam->diskId = xmlFileGetParam(xmlFile, "diskid");
	xmlParam->jumpTime = xmlFileGetParam(xmlFile, "jumptime");
	xmlParam->readTime = xmlFileGetParam(xmlFile, "readtime");
	xmlParam->writeTime = xmlFileGetParam(xmlFile, "writetime");
	xmlParam->rpm = xmlFileGetParam(xmlFile, "rpm");
	return xmlParam;
}
