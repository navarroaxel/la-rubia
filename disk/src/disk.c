#include "disk.h"
#include "fcntl.h"
#include "listener.h"
#include "head.h"
#include "dispatcher.h"
#include "console.h"
#include "pconsole.h"
#include "common/utils/log.h"
#include "common/utils/config.h"

config_disk *config;
t_log *logFile;

int main(void) {
	//TODO: Revisar porque con el fork tira SIGSEGV.
	/*if (fork() == 0) {
	 pconsole(NULL);
	 return EXIT_SUCCESS;
	 }*/
	init_pconsole();

	t_xmlFile *configFile = loadConfig("config2.xml");
	config = xmlGetConfigStructDisk(configFile);

	t_blist *waiting = collection_blist_create(50);
	t_blist *processed = collection_blist_create(50);

	logFile = log_create("PPD", config->logFilePath,
			WARNING | DEBUG | ERROR | INFO, M_CONSOLE_DISABLE);

	init_console(waiting);

	init_head(waiting, processed, logFile);

	init_dispatcher(processed, logFile);
	if (strcmp(config->mode, "CONNECT") == 0) {
		connectraid(waiting, logFile);
	} else if (strcmp(config->mode, "LISTEN") == 0) {
		listener(waiting, logFile);
	} else {
		perror("Modo incorrecto");
	}

	free(config);
	freeXmlFile(configFile);
	return EXIT_SUCCESS;
}

