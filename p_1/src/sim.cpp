/*
 * ARCOS, Universidad Carlos III de Madrid
 * Practica de programacion orientada al rendimiento.
 * Equipo 2:
 *	- Gonzalo Juarez Tello.
 *	- Adrian Mancera Gonzalez.
 *	- Hodei Urigoitia Merodio.
 *	- Gonzalo Martinez Martin.
 */

#include <cmath>

#include "util.hpp"
#include "obj.hpp"

static inline double calc_norm(struct obj_soa &o_soa, unsigned int i, unsigned int j)
{
	double d_x;
	double d_y;
	double d_z;

	d_x = o_soa.x[i] - o_soa.x[j];
	d_y = o_soa.y[i] - o_soa.y[j];
	d_z = o_soa.z[i] - o_soa.z[j];

	return std::sqrt(d_x * d_x + d_y * d_y + d_z * d_z);
}

static inline void calc_fgv(struct obj_soa &o_soa, unsigned int i, unsigned int j)
{
	/* fgv_no_recalc = G * mi * mj / denom */
	double norm;
	double denom;
	double fgv_no_recalc;

	double fx;
	double fy;
	double fz;

	norm = calc_norm(o_soa, i, j);
	denom = norm * norm * norm;
	fgv_no_recalc = 6.674e-11 * o_soa.m[i] * o_soa.m[j] / denom;

	fx = fgv_no_recalc * (o_soa.x[j] - o_soa.x[i]);
	o_soa.fx[i] = o_soa.fx[i] + fx;
	o_soa.fx[j] = o_soa.fx[j] - fx;

	fy = fgv_no_recalc * (o_soa.y[j] - o_soa.y[i]);
	o_soa.fy[i] = o_soa.fy[i] + fy;
	o_soa.fy[j] = o_soa.fy[j] - fy;

	fz = fgv_no_recalc * (o_soa.z[j] - o_soa.z[i]);
	o_soa.fz[i] = o_soa.fz[i] + fz;
	o_soa.fz[j] = o_soa.fz[j] - fz;

}

static inline void calc_vel(struct obj_soa &o_soa, unsigned int i, double time_step)
{
	double accel_no_recalc;
	accel_no_recalc = time_step/o_soa.m[i];
	/* v = vi + a * time_step = vi + F/m * time_step */
	o_soa.vx[i] = o_soa.vx[i] + accel_no_recalc * o_soa.fx[i];
	o_soa.vy[i] = o_soa.vy[i] + accel_no_recalc * o_soa.fy[i];
	o_soa.vz[i] = o_soa.vz[i] + accel_no_recalc * o_soa.fz[i];
	//printf("o_soa.vx[]: %.3f\to_soa.vy[]: %.3f\to_soa.vz[]: %.3f\n",
	//		o_soa.vx[i], o_soa.vy[i], o_soa.vz[i]);
}

static inline void calc_pos(struct obj_soa &o_soa, unsigned int i,
		double size_enclosure, double time_step)
{
	/* p = pi + v * time_step */
	o_soa.x[i] = o_soa.x[i] + o_soa.vx[i] * time_step;
	if (o_soa.x[i] >= size_enclosure) {
		o_soa.x[i] = size_enclosure;
		o_soa.vx[i] = -o_soa.vx[i];
	} else if (o_soa.x[i] <= 0) {
		o_soa.x[i] = 0;
		o_soa.vx[i] = -o_soa.vx[i];
	}

	o_soa.y[i] = o_soa.y[i] + o_soa.vy[i] * time_step;
	if (o_soa.y[i] >= size_enclosure) {
		o_soa.y[i] = size_enclosure;
		o_soa.vy[i] = -o_soa.vy[i];
	} else if (o_soa.y[i] <= 0) {
		o_soa.y[i] = 0;
		o_soa.vy[i] = -o_soa.vy[i];
	}

	o_soa.z[i] = o_soa.z[i] + o_soa.vz[i] * time_step;
	if (o_soa.z[i] >= size_enclosure) {
		o_soa.z[i] = size_enclosure;
		o_soa.vz[i] = -o_soa.vz[i];
	} else if (o_soa.z[i] <= 0) {
		o_soa.z[i] = 0;
		o_soa.vz[i] = -o_soa.vz[i];
	}

	//printf("o_soa.x[]: %f\to_soa.y[]: %f\to_soa.z[]: %f\n",
	//				o_soa.x[i], o_soa.y[i], o_soa.z[i]);
}

static unsigned int collision_check(struct obj_soa &o_soa)
{
	unsigned int merge_count{0};

	for (size_t i = 0; i < o_soa.x.size(); ++i) {
		if (!obj_exists(o_soa, i))
			continue;
		for (size_t j = i + 1; j < o_soa.x.size(); ++j) {
			if (!obj_exists(o_soa, j))
				continue;
			if (calc_norm(o_soa, i, j) < 1.0) {
				merge_obj(o_soa, i, j);
				merge_count++;
			}
		}
	}
	return merge_count;
}

size_t simulate(struct obj_soa &o_soa, unsigned int num_iterations,
		double size_enclosure, double time_step)
{
	size_t exist_ctr{o_soa.x.size()};
	/* chequeo pre-primera iteracion */
	exist_ctr -= collision_check(o_soa);
	for (unsigned int k = 0; k < num_iterations; ++k) {
		for (size_t i = 0; i < o_soa.x.size(); ++i) {

			if (!obj_exists(o_soa, i))
				continue;

			for (size_t j = i + 1; j < o_soa.x.size(); ++j) {
				if (!obj_exists(o_soa, j))
					continue;
				calc_fgv(o_soa, i, j);
			}
			//printf("o_soa.fx[%d]: %f\to_soa.fy[%d]: %f\to_soa.fz[%d]: %f\n",
			//		i, o_soa.fx[i], i, o_soa.fy[i], i, o_soa.fz[i]);

			/* necesita fuerza para calcular aceleracion */
			calc_vel(o_soa, i, time_step);

			/* ya no se necesita fuerza, limpiar para prox iteracion */
			o_soa.fx[i] = 0;
			o_soa.fy[i] = 0;
			o_soa.fz[i] = 0;

			calc_pos(o_soa, i, size_enclosure, time_step);

		}

		/* chequeo de final de c/iteracion */
		exist_ctr -= collision_check(o_soa);
	}


	return exist_ctr;
}

#include <omp.h>

int main(int argc, char *argv[])
{
	struct args arg_list{};
	size_t exist_num{};

	parse_args(arg_list, argc, argv);

	struct obj_soa o_soa(arg_list.num_objects,
		arg_list.random_seed, arg_list.size_enclosure);

	if (write_config("init_config.txt", arg_list.size_enclosure, arg_list.time_step,
				arg_list.num_objects, o_soa))
		log_n_exit("Error while trying to write to init_config.txt\n", 1);

	double tic = omp_get_wtime();

	exist_num = simulate(o_soa, arg_list.num_iterations, arg_list.size_enclosure, arg_list.time_step);


	double toc = omp_get_wtime();

	printf("tiempo de ejecucion: %f\n", toc-tic);

	if (write_config("final_config.txt", arg_list.size_enclosure, arg_list.time_step,
				exist_num, o_soa))
		log_n_exit("Error while trying to write to final_config.txt\n", 1);

	return 0;
}
