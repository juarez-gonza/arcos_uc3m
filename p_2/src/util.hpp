#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "obj.hpp"

struct args {
	int num_objects;
	int num_iterations;
	int random_seed;
	double size_enclosure;
	double time_step;

	args() :
		num_objects{0}, num_iterations{0}, random_seed{0},
		size_enclosure{0}, time_step{0}
	{
	}
};

void parse_args(struct args &arg_list, int argc, char *argv[]);

int write_config(std::string filename, double size_enclosure,
		double time_step, struct soa &o_soa);

void log_n_exit(std::string error_msg, int exitcode);

size_t inline nxt_pow_2(size_t n)
{
	n--;
	n = n | (n >> 1);
	n = n | (n >> 2);
	n = n | (n >> 4);
	n = n | (n >> 8);
	n = n | (n >> 16);
	return ++n;
}
