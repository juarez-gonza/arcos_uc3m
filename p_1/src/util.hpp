#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>

#include <regex>

#include <vector>

#include "obj.hpp"

#define likely(exp) __builtin_expect((exp), 1)
#define unlikely(exp) __builtin_expect((exp), 0)
struct args {
	int num_objects;
	int num_iterations;
	int random_seed;
	double size_enclosure;
	double time_step;
};

int write_config(std::string filename, double size_enclosure,
		double time_step, std::vector<struct obj> &o_list);

void log_error(std::string error_msg, int exitcode);

inline bool is_double(char *str)
{
	static std::regex double_expr("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");
	return std::regex_match(str, double_expr);
}

inline bool is_int(char *str)
{
	static std::regex int_expr("[-]?[0-9]+");
	return std::regex_match(str, int_expr);
}
