#include "util.hpp"

int write_config(std::string filename, double size_enclosure,
		double time_step, std::vector<struct obj> &o_list)
{
	std::ofstream ofs(filename);
	if (!ofs)
		return 1;

	ofs << std::fixed << std::setprecision(3) << size_enclosure << ' '
		<< time_step << ' ' << o_list.size() << '\n';
	for (unsigned long i = 0; i < o_list.size(); ++i)
		ofs << std::fixed << std::setprecision(3) << o_list[i].x << ' '
			<< o_list[i].y << ' '  << o_list[i].z << ' '
			<< o_list[i].vx << ' ' << o_list[i].vy << ' '
			<< o_list[i].vz << ' ' << o_list[i].m << '\n';

	return 0;
}

void log_error(std::string error_msg, int exitcode)
{
	std::cerr << error_msg;
	exit(exitcode);
}
