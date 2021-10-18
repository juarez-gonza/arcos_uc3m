#include "util.hpp"

enum pargs_retcode {
	PARSE_OK,
	ARGC_ERR,
	NUM_OBJ_ERR,
	NUM_ITER_ERR,
	RND_SEED_ERR,
	SIZE_ENCLOSURE_ERR,
	TIME_STEP_ERR
};

enum pargs_retcode _parse_args(struct args &arg_list,
		int argc, char *argv[])
{
	std::stringstream ss;

	if (argc != 6)
		return ARGC_ERR;

	for (int i = 1; i < argc; i++)
		ss << argv[i] << " ";

	ss >> arg_list.num_objects;
	if ((ss.fail() || arg_list.num_objects < 0)) {
		arg_list.num_objects = 0;
		return NUM_OBJ_ERR;
	}

	ss >> arg_list.num_iterations;
	if ((ss.fail() || arg_list.num_iterations < 0)) {
		arg_list.num_iterations = 0;
		return NUM_ITER_ERR;
	}

	ss >> arg_list.random_seed;
	if ((ss.fail() || arg_list.random_seed < 0)) {
		arg_list.random_seed = 0;
		return RND_SEED_ERR;
	}

	ss >> arg_list.size_enclosure;
	if ((ss.fail() || arg_list.size_enclosure < 0)) {
		arg_list.size_enclosure = 0;
		return SIZE_ENCLOSURE_ERR;
	}

	ss >> arg_list.time_step;
	if ((ss.fail() || arg_list.time_step < 0)) {
		arg_list.time_step = 0;
		return TIME_STEP_ERR;
	}

	return PARSE_OK;
}

void parse_args(struct args &arg_list,
		int argc, char *argv[])
{
	pargs_retcode pa_ret;
	std::stringstream ss;

	pa_ret = _parse_args(arg_list, argc, argv);

	switch (pa_ret) {
	case PARSE_OK:
		ss << "Creating simulation:\n";
		break;
	case NUM_OBJ_ERR:
		ss << "Error: Invalid number of objects\n";
		break;
	case NUM_ITER_ERR:
		ss << "Error: Invalid number of iterations\n";
		break;
	case RND_SEED_ERR:
		ss << "Error: Invalid random seed\n";
		break;
	case SIZE_ENCLOSURE_ERR:
		ss << "Error: Invalid size enclosure\n";
		break;
	case TIME_STEP_ERR:
		ss << "Error: Invalid delta time\n";
		break;
	}

	argc--;
	ss << argv[0] << " invoked with " << argc << " parameters.\nArguments:\n  num_objects: "
		<< (argc-- ? argv[1] : "?\n") << "\n  num_iterations: "
		<< (argc-- ? argv[2] : "?\n") << "\n  random_seed: "
		<< (argc-- ? argv[3] : "?\n") << "\n  size_enclosure: "
		<< (argc-- ? argv[4] : "?\n") << "\n  time_step: "
		<< (argc-- ? argv[5] : "?\n") << "\n";
	std::cout << ss.str();

	if (pa_ret == ARGC_ERR)
		exit(-1);
	if (pa_ret != PARSE_OK)
		exit(-2);
}

int write_config(std::string filename, double size_enclosure,
		double time_step, struct obj_aos &o_aos)
{
	std::ofstream ofs(filename);
	if (!ofs)
		return 1;

	ofs << std::fixed << std::setprecision(3) << size_enclosure << ' '
		<< time_step << ' ' << o_aos.x.size() << '\n';
	for (unsigned long i = 0; i < o_aos.x.size(); ++i)
		ofs << std::fixed << std::setprecision(3) << o_aos.x[i] << ' '
			<< o_aos.y[i] << ' '  << o_aos.z[i] << ' '
			<< o_aos.vx[i] << ' ' << o_aos.vy[i] << ' '
			<< o_aos.vz[i] << ' ' << o_aos.m[i] << '\n';

	return 0;
}

void log_n_exit(std::string error_msg, int exitcode)
{
	std::cerr << error_msg;
	exit(exitcode);
}