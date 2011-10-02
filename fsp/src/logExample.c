/*
 * Example.c
 *
 *  Created on: 10/09/2011
 *      Author: shinichi
 */

#include <stdlib.h>
#include <stdio.h>
#include "utils/log.h"

int main2(int argc, char **argv) {

	//Log debug, info or warning messages in file and console
	t_log* log = log_create("Runner", "log.txt", DEBUG | INFO | WARNING, M_CONSOLE_ENABLE);
	log_debug(log, "thread-1", "Message debug: %s", "Paso por aca");
	log_info(log, "thread-1", "Message info: %s", "se conecto el cliente xxx");
	log_warning(log, "thread-1", "Message warning: %s", "not load configuration file, load configuration default");
	log_error(log, "thread-1", "Message error: %s", "Crash!!!!");
	log_destroy(log);

	//Log debug or info messages only in file
	t_log* log2 = log_create("Runner", "log.txt", DEBUG | INFO, M_CONSOLE_DISABLE);
	log_debug(log2, "thread-2", "Message debug: %s", "Paso por aca");
	log_info(log2, "thread-2", "Message info: %s", "se conecto el cliente xxx");
	log_warning(log2, "thread-2", "Message warning: %s", "not load configuration file, load configuration default");
	log_error(log2, "thread-2", "Message error: %s", "Crash!!!!");
	log_destroy(log2);

	//Only log error messages in file and console
	t_log* log3 = log_create("Runner", "log.txt", ERROR, M_CONSOLE_ENABLE);
	log_debug(log3, "thread-3", "Message debug: %s", "Paso por aca");
	log_info(log3, "thread-3", "Message info: %s", "se conecto el cliente xxx");
	log_warning(log3, "thread-3", "Message warning: %s", "not load configuration file, load configuration default");
	log_error(log3, "thread-3", "Message error: %s", "Crash!!!!");
	log_destroy(log3);

	return EXIT_SUCCESS;
}
