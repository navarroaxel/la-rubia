#include "disk.h"
#include "fcntl.h"
#include "listener.h"
#include "head.h"
#include "dispatcher.h"
#include "console.h"
#include "common/utils/log.h"


t_log *logFile;

int main(void) {
	console();

	/*
	 if (fork() == 0) {
	 console();
	 return EXIT_SUCCESS;
	 }*/

	t_blist *waiting = collection_blist_create(50);
	t_blist *processed = collection_blist_create(50);

	logFile = log_create("PPD", "/home/utn_so/ppd.log", WARNING | DEBUG | ERROR | INFO, M_CONSOLE_DISABLE);

	init_head(waiting, processed, logFile);

	init_dispatcher(processed);

	listener(waiting, logFile);

	return EXIT_SUCCESS;
}

