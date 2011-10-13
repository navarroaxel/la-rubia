#include "disk.h"
#include "fcntl.h"
#include "listener.h"
#include "head.h"
#include "dispatcher.h"
#include "console.h"
#include "pconsole.h"
#include "common/utils/log.h"

t_log *logFile;

int main(void) {
	if (fork() == 0) {
		pconsole();
		return EXIT_SUCCESS;
	}

	t_blist *waiting = collection_blist_create(50);
	t_blist *processed = collection_blist_create(50);

	logFile = log_create("PPD", "/home/utn_so/ppd.log",
			WARNING | DEBUG | ERROR | INFO, M_CONSOLE_DISABLE);

	init_console();

	init_head(waiting, processed, logFile);

	init_dispatcher(processed);

	connectraid(waiting, logFile);

	return EXIT_SUCCESS;
}

