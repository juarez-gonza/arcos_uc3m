#pragma once

#include <vector>

struct obj {
	double x, y, z;
	double m;
	double fx, fy, fz;
	/* vy y vz entran en otra linea de cache */
	double vx, vy, vz;
}__attribute__((aligned(8)));

/*
 * @__o: struct obj (NO puntero a struct obj)
 */
#define obj_exists(__o) ((__o).m > 0)
#define obj_delete(__o) ((__o).m = 0)

void init_obj_list(std::vector<struct obj> &o_list,
	unsigned int random_seed, double upperbound);

void merge_obj(struct obj &o_i, struct obj &o_j);
