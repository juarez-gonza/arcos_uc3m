#include "util.hpp"
int write_config(std::string filename, double size_enclosure,
		double time_step, struct obj_aos &o_aos)
{
	std::ofstream ofs(filename);
	if (!ofs)
		return 1;

	ofs << std::fixed << std::setprecision(3) << size_enclosure << ' '
		<< time_step << ' ' << o_aos.x.size() << '\n';
	for (int i = 0; i < o_aos.x.size(); ++i)
		ofs << std::fixed << std::setprecision(3) << o_aos.x[i] << ' '
			<< o_aos.y[i] << ' '  << o_aos.z[i] << ' '
			<< o_aos.vx[i] << ' ' << o_aos.vy[i] << ' '
			<< o_aos.vz[i] << ' ' << o_aos.m[i] << '\n';

	return 0;
}

void log_error(std::string error_msg, int exitcode)
{
	std::cerr << error_msg;
	exit(exitcode);
}
