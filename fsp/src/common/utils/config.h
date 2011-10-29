#ifndef CONFIG_C
#define CONFIG_C
#include <stdio.h>


typedef struct t_xmlFile {
	FILE* f;
	char* Content;
	int Size;
} t_xmlFile;

typedef struct config_raid {
   char bindIpFs[15];
   uint16_t fsPort;
   char bindIpDisk[15];
   uint16_t diskPort;
   uint8_t consoleEnabled;
   char logFilePath[255];
} config_raid;

typedef struct config_disk {
	char raidIp[16];
	uint16_t raidPort;
	char bindIp[15];
	uint16_t bindPort;
	char mode[10];
	char algorithm[6];
	uint16_t portConsole;
	uint8_t logEnabled;
	char diskname[13];
	uint16_t cylinders;
	uint8_t heads;
	uint16_t sectors;
	uint16_t jumpTime;
	uint16_t writeTime;
	uint16_t readTime;
	uint16_t rpm;
	char volumeFilePath[255];
	char logFilePath[255];
	char socketunixpath[255];
} config_disk;

typedef struct config_fsp {
	char diskIp[16];
	uint16_t diskPort;
	char bindIp[15];
	uint16_t bindPort;
	uint16_t maxConnections;
	uint16_t sizeCache;
	char logFilePath[255];
} config_fsp;

int xmlFileGetFileSize(t_xmlFile *xmlFile);
char *xmlFileReadFile(t_xmlFile *xmlFile, int from, int to);
char *xmlFileGetEndSection(const char *section);
char *xmlFileGetParam(t_xmlFile *xmlFile, const char *param);
t_xmlFile *newXmlFile(char *path);
void freeXmlFile(t_xmlFile *xmlFile);
t_xmlFile *loadConfig(char* path);
char *xmlStreamGetParam(char *xmlStream, const char *param);
config_raid *xmlGetConfigStructRaid(t_xmlFile *xmlFile);
config_fsp *xmlGetConfigStructFsp(t_xmlFile *xmlFile);
config_disk *xmlGetConfigStructDisk(t_xmlFile *xmlFile);
#endif
