#pragma once

#include <vector>
#include <random>

struct obj_aos {
	std::vector<double> x, y, z;
	std::vector<double> m;
	std::vector<double> fx, fy, fz;
	std::vector<double> vx, vy, vz;

	obj_aos(unsigned int size, int random_seed, double upperbound) :
		x(size), y(size), z(size),
		m(size),
		fx(size, 0), fy(size, 0), fz(size, 0),
		vx(size, 0), vy(size, 0), vz(size, 0)
	{

		std::mt19937_64 gen(random_seed);
		std::uniform_real_distribution<> uniform(0, upperbound);
		std::normal_distribution<> normal(1e+21, 1e+15);

		for (unsigned int i = 0; i < size; ++i) {
			x[i] = uniform(gen);
			y[i] = uniform(gen);
			z[i] = uniform(gen);
			m[i] = normal(gen);
		}
	}
};

void init_obj_list(struct obj_aos &o_aos,
	unsigned int random_seed, double upperbound);

void merge_obj(struct obj_aos &o_aos,
		unsigned int i, unsigned int j);
