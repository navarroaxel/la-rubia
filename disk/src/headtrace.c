#include "headtrace.h"

t_headtrace *headtrace_create() {
	t_headtrace *trace = malloc(sizeof(t_headtrace));
	trace->time = trace->cylinder_path1.start = trace->cylinder_path2.start =
			trace->cylinder_path1.end = trace->cylinder_path2.end =
					trace->cylinder_path1.step = trace->cylinder_path2.step = 0;

	trace->hasnext = false;
	return trace;
}

void headtrace_log(t_headtrace *trace, t_log *log) {
	char *path;
	char current[12];
	char requested[12];
	char next[12];
	int i, parts;

	if (log == NULL
		)
		return;

	location_string(&trace->current, current);
	location_string(&trace->requested, requested);
	location_string(trace->hasnext ? &trace->next : NULL, next);

	log_info(log, "HEAD", "PLANIFICACION\n"
			"Posicion Actual: %s\n"
			"Sector solicitado: %s\n"
			"Tiempo consumido: %ims\n"
			"Proximo sector: %s", current, requested, trace->time, next);

	path = cylinders_string(&trace->cylinder_path1, trace->current.sector);
	if (path != NULL) {
		parts = strlen(path) / (165 * 12);
		for (i = 0; i <= parts; i++) {
			if (i < parts)
				path[(i + 1) * 165 * 12 - 1] = '\0';

			log_info(log, "HEAD", "Sectores recorridos: %s",
					path + i * 165 * 12);
		}
		free(path);
	}
	if (trace->cylinder_path2.step != 0) {
		path = cylinders_string(&trace->cylinder_path2, trace->current.sector);
		parts = strlen(path) / (165 * 12);
		for (i = 0; i <= parts; i++) {
			if (i < parts)
				path[(i + 1) * 165 * 12 - 1] = '\0';

			log_info(log, "HEAD", "Sectores recorridos: %s",
					path + i * 165 * 12);
		}
		free(path);
	}
	path = sectors_string(trace->current.sector, trace->requested.sector,
			trace->sectors, trace->requested.cylinder);
	if (path != NULL) {
		parts = strlen(path) / (165 * 12);
		for (i = 0; i <= parts; i++) {
			if (i < parts)
				path[(i + 1) * 165 * 12 - 1] = '\0';

			log_info(log, "HEAD", "Sectores recorridos: %s",
					path + i * 165 * 12);
		}
		free(path);
	}
}

void headtrace_printf(t_headtrace *trace) {
	char *path;
	char current[12];
	char requested[12];
	char next[12];

	location_string(&trace->current, current);
	location_string(&trace->requested, requested);
	location_string(trace->hasnext ? &trace->next : NULL, next);

	printf("Posicion Actual: %s\n"
		"Sector solicitado: %s\n"
		"Tiempo consumido: %ims\n"
		"Proximo sector: %s\n", current, requested, trace->time, next);

	if (trace->cylinder_path1.step != 0) {
		path = cylinders_string(&trace->cylinder_path1, trace->current.sector);
		printf("Sectores recorridos: %s\n", path);
		free(path);
		if (trace->cylinder_path2.step != 0) {
			path = cylinders_string(&trace->cylinder_path2, trace->current.sector);
			printf("Sectores recorridos: %s\n", path);
			free(path);
		}
	}
	path = sectors_string(trace->current.sector, trace->requested.sector, trace->sectors, trace->requested.cylinder);
	if (path != NULL) {
		printf("Sectores recorridos: %s\n", path);
		free(path);
	}
}

char *cylinders_string(t_headtrace_cylinder *path, uint16_t sector) {
	int i, j;

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

char *sectors_string(uint16_t start, uint16_t end, uint16_t sectors,
		uint16_t cylinder) {
	int i, j;
	if (start == end)
		return NULL;

	j = 0;
	char *s = malloc((end > start ? end - start : sectors - end + start) * 12);
	for (i = start + 1; ; i++) {
		if (i == sectors)
			i = 0;

		if (j != 0)
			s[j++] = ',';

		j += sprintf(s + j, "%5i:%5i", cylinder, i);

		if (i == end)
			break;
	}
	s[j] = '\0';
	return s;
}

void headtrace_destroy(t_headtrace *trace) {
	free(trace);
}

void headtrace_setcylinderpath(t_headtrace *trace, uint16_t start, uint16_t end,
		int8_t step) {
	t_headtrace_cylinder *path;
	if (trace->cylinder_path1.step == 0)
		path = &trace->cylinder_path1;
	else
		path = &trace->cylinder_path2;

	path->start = start;
	path->end = end;
	path->step = step;
}

int locations_string(t_list *locations, char *s) {
	int i = 0;
	void closure(void *data) {
		if (i != 0)
			s[i++] = ',';

		t_location *l = data;
		i += location_string(l, s + i);
	}

	collection_list_iterator(locations, closure);
	return i;
}

int location_string(t_location *l, char *s) {
	if (l == NULL) {
		*s = '\0';
		return 0;
	}

	return sprintf(s, "%i:%i", l->cylinder, l->sector);
}
