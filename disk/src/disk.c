#include "disk.h"
#include "fcntl.h"
#include "listener.h"
#include "head.h"
#include "dispatcher.h"

int main(void) {
	t_blist *waiting = collection_blist_create(50);
	t_blist *processed = collection_blist_create(50);

	init_head(waiting, processed);

	init_dispatcher(processed);

	listener(waiting);

	return EXIT_SUCCESS;
}
