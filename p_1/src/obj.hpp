#pragma once

#include <vector>

struct obj {
	double x, y, z;
	double m;
	double fx, fy, fz;
	/* vy y vz entran en otra linea de cache */
	double vx, vy, vz;
}__attribute__((aligned(8)));

int init_obj_list(std::vector<struct obj> &o_list,
	unsigned int random_seed, double upperbound);

void merge_obj(std::vector<struct obj> &o_list,
		unsigned int i, unsigned int j);
