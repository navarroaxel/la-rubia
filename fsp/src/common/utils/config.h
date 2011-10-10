#ifndef CONFIG_C
#define CONFIG_C

typedef struct t_xmlFile {
	FILE* f;
	char* Content;
	int Size;
} t_xmlFile;


typedef struct config_raid {
   uint16_t portFs;
   uint16_t portPpd;
   uint8_t activateConsola;
} config_raid;

typedef struct config_disk {
	char ip[15];
	uint16_t portProc;
	char mode[10];
	char algorithm[4];
	uint16_t portConsole;
	uint8_t logEnabled;
	uint8_t diskId;
	uint8_t cylinder;
	uint8_t head;
	uint16_t sector;
	uint16_t jumpTime;
	uint16_t writeTime;
	uint16_t readTime;
	uint16_t rpm;
} config_disk;

typedef struct config_fsp {
	char ip[15];
	uint16_t port;
	uint16_t maxConnections;
	uint16_t sizeCache;
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
