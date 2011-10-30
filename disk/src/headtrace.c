#include "headtrace.h"

t_headtrace *headtrace_create() {
	t_headtrace *trace = malloc(sizeof(t_headtrace));
	trace->time = 0;
	trace->cylinder_path1 = trace->cylinder_path2 = NULL;
	trace->next = NULL;
	return trace;
}

void waiting_log(t_blist *waiting, t_log *log){
	int length;

	if (log == NULL)
		return;

	char *s = NULL;
	int i = 0;
	t_location *location = location_create(0);
	void locstr(void *data){
		if (s == NULL)
			s = malloc((length = collection_blist_size(waiting)) * 12);
		else
			s[i++] = ',';

		t_disk_operation *op = data;
		location_set(location, op->offset);
		i += location_string(location, s + i);
	}
	collection_blist_iterator(waiting, locstr);
	log_info(log, "HEAD", "COLA DE PEDIDOS: [%s] (%i)", s, length);
}

void inprogress_log(t_list *inprogress, t_log *log){
	int length;
	if (log == NULL)
		return;

	char *s = NULL;
	int i = 0;
	t_location *location = location_create(0);
	void locstr(void *data){
		if (s == NULL)
			s = malloc((length = collection_list_size(inprogress)) * 12);
		else
			s[i++] = ',';

		t_disk_operation *op = data;
		location_set(location, op->offset);
		i += location_string(location, s + i);
	}
	collection_list_iterator(inprogress, locstr);
	log_info(log, "HEAD", "COLA DE PEDIDOS: [%s] (%i)", s, length);
}

void headtrace_log(t_headtrace *trace, t_log *log) {
	char *path;
	char current[12];
	char requested[12];
	char next[12];
	int i, parts;

	if (log == NULL)
			return;

	location_string(&trace->current, current);
	location_string(&trace->requested, requested);
	location_string(trace->next, next);

	log_info(log, "HEAD", "PLANIFICACION\n"
			"Posicion Actual: %s\n"
			"Sector solicitado: %s\n"
			"Tiempo consumido: %ims\n"
			"Proximo sector: %s", current, requested, trace->time, next);

	path = cylinders_string(trace->cylinder_path1, trace->current.sector);
	if (path != NULL) {
		parts = strlen(path) / (165 * 12);
		for (i = 0; i <= parts; i++){
			if (i < parts)
				path[(i + 1) * 165 * 12 - 1] = '\0';

			log_info(log, "HEAD", "Sectores recorridos: %s", path + i * 165 * 12);
		}
		free(path);
	}
	if (trace->cylinder_path2 != NULL){
		path = cylinders_string(trace->cylinder_path2, trace->current.sector);
		parts = strlen(path) / (165 * 12);
		for (i = 0; i <= parts; i++){
			if (i < parts)
				path[(i + 1) * 165 * 12 - 1] = '\0';

			log_info(log, "HEAD", "Sectores recorridos: %s", path + i * 165 * 12);
		}
		free(path);
	}
	path = sectors_string(trace->current.sector, trace->requested.sector, trace->limitsector, trace->requested.cylinder);
	if (path != NULL) {
		parts = strlen(path) / (165 * 12);
		for (i = 0; i <= parts; i++){
			if (i < parts)
				path[(i + 1) * 165 * 12 - 1] = '\0';

			log_info(log, "HEAD", "Sectores recorridos: %s", path + i * 165 * 12);
		}
		free(path);
	}
}

char *cylinders_string(t_headtrace_cylinder *path, uint16_t sector) {
	int i, j;
	if (path == NULL)
		return NULL;

	j = 0;
	char *s = malloc((abs(path->end - path->start) + 1) * 12);
	for (i = path->start; ; i += path->step) {
		if (j != 0)
			s[j++] = ',';

		j += sprintf(s + j, "%5i:%5i", i, sector);
		if (i == path->end)
			break;
	}

	s[j] = '\0';
	return s;
}

char *sectors_string(uint16_t start, uint16_t end, uint16_t limit,uint16_t cylinder) {
	int i, j;
	if (start == end)
		return NULL;

	j = 0;
	char *s = malloc((end > start ? end - start : limit - start + end) * 12);
	for (i = start + 1; i != end; i++) {
		if (j != 0)
			s[j++] = ',';

		j += sprintf(s + j, "%5i:%5i", cylinder, i);
		if (i == limit)
			i = -1;
	}
	s[j] = '\0';
	return s;
}

void headtrace_destroy(t_headtrace *trace) {
	if (trace->next != NULL)
		location_destroy(trace->next);

	if (trace->cylinder_path1 != NULL)
		free(trace->cylinder_path1);

	if (trace->cylinder_path2 != NULL)
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
