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
#include <omp.h>

#include "util.hpp"
#include "obj.hpp"

#ifndef OMP_NUM_THREADS
#define OMP_NUM_THREADS 8
#endif

static inline double calc_norm(struct obj &o_i, struct obj &o_j)
{
	double d_x = o_i.x - o_j.x;
	double d_y = o_i.y - o_j.y;
	double d_z = o_i.z - o_j.z;

	return std::sqrt(d_x * d_x + d_y * d_y + d_z * d_z);
}

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

static void calc_fgv(my_vector<struct obj> &o_list)
{
	const size_t b = nxt_pow_2(o_list.get_len()/OMP_NUM_THREADS);
	for (size_t i = 0; i < o_list.get_len(); ++i) {
		#pragma omp parallel
		{
		/* preferible std::vector a VLA. aunque vector usa heap que tecnicamente no es memoria
		 * privada al thread.
		 */

		std::vector<double> fgv_no_recalc(b);
		#pragma omp for ordered schedule(auto)
		for (size_t jj = i + 1; jj < o_list.get_len(); jj += b) {
			for (size_t j = jj; j < std::min(jj+b, o_list.get_len()); ++j) {
				double norm = calc_norm(o_list[i], o_list[j]);
				/* a diferencia de SoA, fgv_no_recalc no es vectorizable,
				 * o_list[j].m no estan proximas en la memoria.
				 */
				fgv_no_recalc[j-jj] = 6.674e-11 * o_list[i].m * o_list[j].m
					/ (norm * norm * norm);
			}

			#pragma omp ordered
			for (size_t j = jj; j < std::min(jj+b, o_list.get_len()); ++j) {
				double fx = fgv_no_recalc[j-jj] * (o_list[j].x - o_list[i].x);
				double fz = fgv_no_recalc[j-jj] * (o_list[j].z - o_list[i].z);
				double fy = fgv_no_recalc[j-jj] * (o_list[j].y - o_list[i].y);

				o_list[j].fx = o_list[j].fx - fx;
				o_list[j].fy = o_list[j].fy - fy;
				o_list[j].fz = o_list[j].fz - fz;

				o_list[i].fx = o_list[i].fx + fx;
				o_list[i].fy = o_list[i].fy + fy;
				o_list[i].fz = o_list[i].fz + fz;
			}
		}} /* fin #pragma omp for y pragma omp parallel */
	}
}

static inline void calc_vel(struct obj &o, double time_step)
{
	/* v = vi + a * time_step = vi + F/m * time_step */
	double accel_no_recalc = time_step/o.m;
	o.vx = o.vx + accel_no_recalc * o.fx;
	o.vy = o.vy + accel_no_recalc * o.fy;
	o.vz = o.vz + accel_no_recalc * o.fz;
	//printf("o.vx: %f, o.vy: %f, o.vz: %f\n", o.vx, o.vy, o.vz);
}

static inline void adjust_limits(double size_enclosure,
				double &pos, double &vel)
{
	if (pos >= size_enclosure) {
		pos = size_enclosure;
		vel = -vel;
	} else if (pos <= 0) {
		pos = 0;
		vel = -vel;
	}
}

static inline void calc_pos(struct obj &o, double size_enclosure, double time_step)
{
	/* p = pi + v * time_step */
	o.x = o.x + o.vx * time_step;
	adjust_limits(size_enclosure, o.x, o.vx);

	o.y = o.y + o.vy * time_step;
	adjust_limits(size_enclosure, o.y, o.vy);

	o.z = o.z + o.vz * time_step;
	adjust_limits(size_enclosure, o.z, o.vz);
	//printf("o.x: %f, o.y: %f, o.z: %f\n", o.x, o.y, o.z);
}

static inline void mark(struct obj &o)
{
	o.m = 0;
}

static inline bool obj_marked(struct obj &o)
{
	return o.m <= 0;
}

void mark_collisions(my_vector<struct obj> &o_list)
{
/* paralelizable sin modificar orden de operaciones en merge_obj()?? */
	for (size_t i = 0ul; i < o_list.get_len(); ++i) {
		if (obj_marked(o_list[i]))
			continue;
		for (size_t j = i + 1ul; j < o_list.get_len(); ++j) {
			if (obj_marked(o_list[j]))
				continue;
			if (calc_norm(o_list[i], o_list[j]) < 1.0) {
				merge_obj(o_list[i], o_list[j]);
				mark(o_list[j]);
			}
		}
	}
}

void delete_marked(my_vector<struct obj> &o_list)
{
	size_t last = 0ul;
	for (size_t i = 0ul; i < o_list.get_len(); ++i, ++last) {
		while (obj_marked(o_list[i]) && i < o_list.get_len())
			i++;
		if (i >= o_list.get_len())
			break;
		obj_copy_into(o_list[last], o_list[i]);
	}
	o_list.set_len(last);
}

static void inline collision_check(my_vector<struct obj> &o_list)
{
	mark_collisions(o_list);
	delete_marked(o_list);
}

static void simulate(my_vector<struct obj> &o_list, unsigned int num_iterations,
		double size_enclosure, double time_step)
{
	/* chequeo pre-primera iteracion */
	collision_check(o_list);
	for (unsigned int k = 0; k < num_iterations; ++k) {
		calc_fgv(o_list);

		/* paralelizar esto parece no ser un speedup en AoS */
		//#pragma omp parallel for schedule(auto)
		for (size_t i = 0; i < o_list.get_len(); ++i) {
			/* necesita fuerza para calcular aceleracion */
			calc_vel(o_list[i], time_step);

			/* ya no se necesita fuerza, limpiar para prox iteracion */
			o_list[i].fx = 0.0;
			o_list[i].fy = 0.0;
			o_list[i].fz = 0.0;

			calc_pos(o_list[i], size_enclosure, time_step);
		}

		/* chequeo de final de c/iteracion */
		collision_check(o_list);
	}
}

int main(int argc, char *argv[])
{
	struct args arg_list{};

	parse_args(arg_list, argc, argv);

	my_vector<struct obj> o_list(arg_list.num_objects, arg_list.random_seed, arg_list.size_enclosure);

	if (write_config("init_config.txt", arg_list.size_enclosure, arg_list.time_step, o_list))
		log_n_exit("Error while trying to write to init_config.txt\n", 1);

	double tic = omp_get_wtime();

	simulate(o_list, arg_list.num_iterations, arg_list.size_enclosure, arg_list.time_step);

	double toc = omp_get_wtime();

	printf("tiempo de ejecucion: %f\n", toc-tic);

	if (write_config("final_config.txt", arg_list.size_enclosure, arg_list.time_step, o_list))
		log_n_exit("Error while trying to write to final_config.txt\n", 1);

	return 0;
}
