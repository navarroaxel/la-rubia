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


int xmlFileGetFileSize(t_xmlFile *xmlFile);
char *xmlFileReadFile(t_xmlFile *xmlFile, int from, int to);
char *xmlFileGetEndSection(const char *section);
char *xmlFileGetParam(t_xmlFile *xmlFile, const char *param);
t_xmlFile *newXmlFile(char *path);
void freeXmlFile(t_xmlFile *xmlFile);
t_xmlFile *loadConfig(char* path);
char *xmlStreamGetParam(char *xmlStream, const char *param);
config_raid *xmlGetConfigStruct(t_xmlFile *xmlFile);
#endif
