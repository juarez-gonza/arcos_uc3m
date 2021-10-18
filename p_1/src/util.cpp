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
	case ARGC_ERR: // sigue de largo adrede en este caso
		break;
	}

	argc--;
	ss << argv[0] << " invoked with " << argc << " parameters.\nArguments:\n  num_objects: "
		<< (argc > 0 ? argv[1] : "?") << "\n  num_iterations: "
		<< (argc > 1 ? argv[2] : "?") << "\n  random_seed: "
		<< (argc > 2 ? argv[3] : "?") << "\n  size_enclosure: "
		<< (argc > 3 ? argv[4] : "?") << "\n  time_step: "
		<< (argc > 4 ? argv[5] : "?") << "\n";
	std::cout << ss.str();

	if (pa_ret == ARGC_ERR)
		exit(-1);
	if (pa_ret != PARSE_OK)
		exit(-2);
}

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

void log_n_exit(std::string error_msg, int exitcode)
{
	std::cerr << error_msg;
	exit(exitcode);
}
