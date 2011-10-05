#include "disk.h"
#include "fcntl.h"
#include "listener.h"
#include "head.h"
#include "dispatcher.h"

int main(void) {
	t_blist *operations = collection_blist_create(50);
	t_blist *ready = collection_blist_create(50);

	init_head(operations, ready);

	init_dispatcher(ready);

	listener(operations);

	return EXIT_SUCCESS;
}
