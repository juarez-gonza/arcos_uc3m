#pragma once

#include <vector>

struct obj {
	double x, y, z;
	double m;
	double fx, fy, fz;
	/* 1 cuando el objeto existe, 0 cuando no existe */
	/* facilita el merge de objetos sin necesitar copiar el array
	 * para quitar un objeto ni recurrir a una linked list */
	unsigned long exists;
	/* fin de linea de caché */
	double vx, vy, vz;
}__attribute__((aligned(8)));

#define obj_exists(_o) (_o).exists

void merge_obj(struct obj &o_i, struct obj &o_j);

int init_obj_list(std::vector<struct obj> &o_list,
	unsigned int random_seed, double upperbound);
