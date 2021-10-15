#include "obj.hpp"
#include <stdlib.h>
#include <random>

#include <iostream>

void merge_obj(struct obj &o_i, struct obj &o_j)
{
	o_j.exists = 0;
	o_i.m += o_j.m;
	o_i.vx += o_j.vx;
	o_i.vy += o_j.vy;
	o_i.vz += o_j.vz;
	/*
	std::cout << "merge " << o_jp << " into " << o_ip << '\n';
	*/
}

int init_obj_list(std::vector<struct obj> &o_list,
	unsigned int random_seed, double upperbound)
{
	std::mt19937_64 gen(random_seed);
	std::uniform_real_distribution<> uniform(0, upperbound);
	std::normal_distribution<> normal(1e+21, 1e+15);

	for (int i = 0; i < o_list.size(); ++i) {
		o_list[i].exists = 1;

		o_list[i].x = uniform(gen);
		o_list[i].y = uniform(gen);
		o_list[i].z = uniform(gen);
		o_list[i].m = normal(gen);

		o_list[i].fx = 0;
		o_list[i].fy = 0;
		o_list[i].fz = 0;

		o_list[i].vx = 0;
		o_list[i].vy = 0;
		o_list[i].vz = 0;
	}

	/*
	for (int i = 0; i < o_list.size(); i++)
		std::cout << "o:\t" << &o_list[i]
		<< "\n\tpos_x: " << std::fixed << o_list[i].x
		<< "\n\tpos_y: " << std::fixed << o_list[i].y
		<< "\n\tpos_z: " << std::fixed << o_list[i].z
		<< "\n\tvel_x: " << std::fixed << o_list[i].vx
		<< "\n\tvel_y: " << std::fixed << o_list[i].vy
		<< "\n\tvel_z: " << std::fixed << o_list[i].vz
		<< "\n\tm: " << o_list[i].m << "\n";
	*/

	return 0;
}
