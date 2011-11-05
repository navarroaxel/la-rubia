#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
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
	char* endSection = charmalloc(strlen(section) + 1);
	endSection[0] = '/';
	for (i = 1; section[i - 1]; i++)
		endSection[i] = section[i - 1];
	endSection[i] = '\0';
	return endSection;
}

char *xmlFileGetParam(t_xmlFile *xmlFile, const char* param) {
	char* paramValue;
	int beginSection = strstr(xmlFile->Content, param) - xmlFile->Content
			+ strlen(param) + 1;
	char* endParam = xmlFileGetEndSection(param);
	int endSection = strstr(xmlFile->Content, endParam) - xmlFile->Content - 2;
	free(endParam);
	if (beginSection < 0 || endSection < 0)
		return NULL;

	paramValue = substring(xmlFile->Content, beginSection, endSection);
	return paramValue;
}

t_xmlFile *newXmlFile(char *path) {
	t_xmlFile *xmlFile = (t_xmlFile*) malloc(sizeof(t_xmlFile));
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

config_raid *xmlGetConfigStructRaid(t_xmlFile *xmlFile) {
	config_raid *xmlParam = (config_raid*) malloc(sizeof(config_raid));
	strcpy(xmlParam->bindIpFs, xmlFileGetParam(xmlFile, "bindipfs"));
	xmlParam->fsPort = (uint16_t) atoi(xmlFileGetParam(xmlFile, "fsport"));
	strcpy(xmlParam->bindIpDisk, xmlFileGetParam(xmlFile, "bindipdisk"));
	xmlParam->diskPort = (uint16_t) atoi(xmlFileGetParam(xmlFile, "diskport"));
	xmlParam->consoleEnabled = (uint16_t) atoi(
			xmlFileGetParam(xmlFile, "consoleenabled"));
	strcpy(xmlParam->logFilePath, xmlFileGetParam(xmlFile, "logfile"));
	return xmlParam;
}

config_fsp *xmlGetConfigStructFsp(t_xmlFile *xmlFile) {
	config_fsp *xmlParam = (config_fsp*) malloc(sizeof(config_fsp));
	strcpy(xmlParam->diskIp, xmlFileGetParam(xmlFile, "diskip"));
	xmlParam->diskPort = (uint16_t) atoi(xmlFileGetParam(xmlFile, "diskport"));
	strcpy(xmlParam->bindIp, xmlFileGetParam(xmlFile, "bindip"));
	xmlParam->bindPort = (uint16_t) atoi(xmlFileGetParam(xmlFile, "bindport"));
	xmlParam->maxConnections = (uint16_t) atoi(
			xmlFileGetParam(xmlFile, "maxconnections"));
	xmlParam->sizeCache = (uint16_t) atoi(
			xmlFileGetParam(xmlFile, "sizecache"));
	strcpy(xmlParam->logFilePath, xmlFileGetParam(xmlFile, "logfile"));
	return xmlParam;
}

config_disk *xmlGetConfigStructDisk(t_xmlFile *xmlFile) {
	config_disk *xmlParam = (config_disk*) malloc(sizeof(config_disk));
	strcpy(xmlParam->raidIp, xmlFileGetParam(xmlFile, "raidip"));
	xmlParam->raidPort = (uint16_t) atoi(xmlFileGetParam(xmlFile, "raidport"));
	strcpy(xmlParam->bindIp, xmlFileGetParam(xmlFile, "bindip"));
	xmlParam->bindPort = (uint16_t) atoi(xmlFileGetParam(xmlFile, "bindport"));
	strcpy(xmlParam->mode, xmlFileGetParam(xmlFile, "mode"));
	strcpy(xmlParam->algorithm, xmlFileGetParam(xmlFile, "algorithm"));
	xmlParam->portConsole = (uint16_t) atoi(xmlFileGetParam(xmlFile, "portconsole"));
	xmlParam->logEnabled = (uint8_t) atoi(xmlFileGetParam(xmlFile, "logenabled"));
	xmlParam->cylinders = (uint16_t) atoi(xmlFileGetParam(xmlFile, "cylinders"));
	xmlParam->heads = (uint8_t) atoi(xmlFileGetParam(xmlFile, "heads"));
	xmlParam->sectors = (uint16_t) atoi(xmlFileGetParam(xmlFile, "sectors"));
	strcpy(xmlParam->diskname, xmlFileGetParam(xmlFile, "diskname"));
	xmlParam->jumpTime = (uint16_t) atoi(xmlFileGetParam(xmlFile, "jumptime"));
	xmlParam->readTime = (uint16_t) atoi(xmlFileGetParam(xmlFile, "readtime"));
	xmlParam->writeTime = (uint16_t) atoi(xmlFileGetParam(xmlFile, "writetime"));
	xmlParam->rpm = (uint16_t) atoi(xmlFileGetParam(xmlFile, "rpm"));
	strcpy(xmlParam->volumeFilePath, xmlFileGetParam(xmlFile, "volumefile"));
	strcpy(xmlParam->logFilePath, xmlFileGetParam(xmlFile, "logfile"));
	strcpy(xmlParam->consolePath, xmlFileGetParam(xmlFile, "consolepath"));
	strcpy(xmlParam->socketunixpath, xmlFileGetParam(xmlFile, "socketunixpath"));

	return xmlParam;
}
