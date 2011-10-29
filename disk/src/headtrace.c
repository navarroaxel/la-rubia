#include "headtrace.h"

t_headtrace *headtrace_create() {
	t_headtrace *trace = malloc(sizeof(t_headtrace));
	trace->time = 0;
	trace->cylinder_path1 = trace->cylinder_path2 = NULL;
	trace->next = NULL;
	return trace;
}

void headtrace_log(t_headtrace *trace, t_log *log) {
	char path[4096];
	char current[12];
	char requested[12];
	char next[12];

	location_string(&trace->current, current);
	location_string(&trace->requested, requested);
	//locations_string(p->path, path);
	location_string(trace->next, next);

	log_info(log, "FSP", "INFO PLANIFICACION\n"
	//"Cola de Pedidos: [%s]\n"
	//"Tamano: %i\n"
					"Posicion Actual: %s\n"
					"Sector solicitado: %s\n"
	//"Sectores recorridos: %s\n"
			"Tiempo consumido: %ims\n"
			"Proximo sector: %s", current, requested, trace->time, next);
}

void headtrace_destroy(t_headtrace *trace) {
	if (trace->next != NULL
		)
		location_destroy(trace->next);

	if (trace->cylinder_path1 != NULL
		)
		free(trace->cylinder_path1);

	if (trace->cylinder_path2 != NULL
		)
		free(trace->cylinder_path2);

	free(trace);
}

void headtrace_setcylinderpath(t_headtrace *trace, uint16_t start, uint16_t end, int8_t step){
	t_headtrace_cylinder *path;
	if (trace->cylinder_path1 == NULL)
		path = trace->cylinder_path1 = headtrace_cylinder_create();
	else
		path = trace->cylinder_path2 = headtrace_cylinder_create();

	path->start = start;
	path->end = end;
	path->step = step;
}

t_headtrace_cylinder *headtrace_cylinder_create() {
	t_headtrace_cylinder *trace = malloc(sizeof(t_headtrace_cylinder));
	trace->step = trace->end = trace->start = 0;
	return trace;
}
