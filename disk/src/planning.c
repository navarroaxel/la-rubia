#include "planning.h"

t_planning *planning_create() {
	t_planning *p = malloc(sizeof(t_planning));
	p->time = 0;
	p->path = collection_list_create();
	p->next = NULL;
	return p;
}

void planning_log(t_planning *p, t_log *log) {
	char path[4096];
	char current[12];
	char requested[12];
	char next[12];

	location_string(&p->current, current);
	location_string(&p->requested, requested);
	locations_string(p->path, path);
	location_string(p->next, next);

	log_info(log, "FSP", "INFO PLANIFICACION\n"
	//"Cola de Pedidos: [%s]\n"
	//"Tamano: %i\n"
			"Posicion Actual: %s\n"
			"Sector solicitado: %s\n"
			"Sectores recorridos: %s\n"
			"Tiempo consumido: %ims\n"
			"Proximo sector: %s", current, requested, path, p->time, next);
}

void planning_destroy(t_planning *p) {
	void destroy(void *data) {
		location_destroy((t_location *) data);
	}

	collection_list_destroy(p->path, destroy);
	free(p);
}
