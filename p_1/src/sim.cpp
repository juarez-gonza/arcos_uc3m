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
#include <stdlib.h>

#include "util.hpp"
#include "obj.hpp"

static inline double calc_norm(struct obj_aos &o_aos, unsigned int i, unsigned int j)
{
	double d_x;
	double d_y;
	double d_z;

	d_x = o_aos.x[i] - o_aos.x[j];
	d_y = o_aos.y[i] - o_aos.y[j];
	d_z = o_aos.z[i] - o_aos.z[j];

	return std::sqrt(d_x * d_x + d_y * d_y + d_z * d_z);
}

static inline void calc_fgv(struct obj_aos &o_aos, unsigned int i, unsigned int j)
{
	/* fgv_no_recalc = G * mi * mj / denom */
	double denom;
	double fgv_no_recalc;

	double fx;
	double fy;
	double fz;

	denom = calc_norm(o_aos, i, j);
	denom = denom * denom * denom;
	fgv_no_recalc = 6.674e-11 * o_aos.m[i] * o_aos.m[j] / denom;

	fx = fgv_no_recalc * (o_aos.x[j] - o_aos.x[i]);
	o_aos.fx[i] = o_aos.fx[i] + fx;
	o_aos.fx[j] = o_aos.fx[j] - fx;

	fy = fgv_no_recalc * (o_aos.y[j] - o_aos.y[i]);
	o_aos.fy[i] = o_aos.fy[i] + fy;
	o_aos.fy[j] = o_aos.fy[j] - fy;

	fz = fgv_no_recalc * (o_aos.z[j] - o_aos.z[i]);
	o_aos.fz[i] = o_aos.fz[i] + fz;
	o_aos.fz[j] = o_aos.fz[j] - fz;
}

static inline void calc_vel(struct obj_aos &o_aos, unsigned int i, double time_step)
{
	double accel_no_recalc;
	accel_no_recalc = time_step/o_aos.m[i];
	/* v = vi + a * time_step = vi + F/m * time_step */
	o_aos.vx[i] = o_aos.vx[i] + accel_no_recalc * o_aos.fx[i];
	o_aos.vy[i] = o_aos.vy[i] + accel_no_recalc * o_aos.fy[i];
	o_aos.vz[i] = o_aos.vz[i] + accel_no_recalc * o_aos.fz[i];
}

static inline void calc_pos(struct obj_aos &o_aos, unsigned int i,
		double size_enclosure, double time_step)
{
	/* p = pi + v * time_step */
	o_aos.x[i] = o_aos.x[i] + o_aos.vx[i] * time_step;
	if (o_aos.x[i] >= size_enclosure) {
		o_aos.x[i] = size_enclosure;
		o_aos.vx[i] = -o_aos.vx[i];
	} else if (o_aos.x[i] <= 0) {
		o_aos.x[i] = 0;
		o_aos.vx[i] = -o_aos.vx[i];
	}

	o_aos.y[i] = o_aos.y[i] + o_aos.vy[i] * time_step;
	if (o_aos.y[i] >= size_enclosure) {
		o_aos.y[i] = size_enclosure;
		o_aos.vy[i] = -o_aos.vy[i];
	} else if (o_aos.y[i] <= 0) {
		o_aos.y[i] = 0;
		o_aos.vy[i] = -o_aos.vy[i];
	}

	o_aos.z[i] = o_aos.z[i] + o_aos.vz[i] * time_step;
	if (o_aos.z[i] >= size_enclosure) {
		o_aos.z[i] = size_enclosure;
		o_aos.vz[i] = -o_aos.vz[i];
	} else if (o_aos.z[i] <= 0) {
		o_aos.z[i] = 0;
		o_aos.vz[i] = -o_aos.vz[i];
	}
}

/* efectos colaterales: cambia tamaño de los vectores en o_aos */
static void collision_check(struct obj_aos &o_aos)
{
	/* itera en reverso porque facilita el loop sobre un vector
	 * que cambia la cantidad de elementos.
	 */
	for (int i = o_aos.x.size() - 1; i >= 0; --i)
		for (int j = i - 1; j >= 0; --j)
			if (calc_norm(o_aos, i, j) < 1.0) {
				/* primer argumento de merge_obj() es j
				 * porque por consigna el primer objeto en la
				 * lista se combina con el segundo. al ser
				 * un loop en reverso, el primer objeto es j
				 */
				merge_obj(o_aos, j, i);
				/* objeto en i ya no existe */
				i--;
			}
}

void simulate(struct obj_aos &o_aos, unsigned int num_iterations,
		double size_enclosure, double time_step)
{
	/* chequeo pre-primera iteracion */
	collision_check(o_aos);
	for (unsigned long k = 0; k < num_iterations; ++k) {
		for (unsigned long i = 0; i < o_aos.x.size(); ++i) {
			for (unsigned long j = i + 1; j < o_aos.x.size(); ++j)
				calc_fgv(o_aos, i, j);

			/* necesita fuerza para calcular aceleracion */
			calc_vel(o_aos, i, time_step);

			/* ya no se necesita fuerza, limpiar para prox iteracion */
			o_aos.fx[i] = 0;
			o_aos.fy[i] = 0;
			o_aos.fz[i] = 0;

			calc_pos(o_aos, i, size_enclosure, time_step);
		}

		collision_check(o_aos); /* chequeo de final de c/iteracion */
	}
}

int main(int argc, char *argv[])
{
	struct args arg_list{};

	parse_args(arg_list, argc, argv);

	struct obj_aos o_aos(arg_list.num_objects,
		arg_list.random_seed, arg_list.size_enclosure);

	if (write_config("init_config.txt", arg_list.size_enclosure, arg_list.time_step, o_aos))
		log_n_exit("Error while trying to write to init_config.txt\n", 1);

	simulate(o_aos, arg_list.num_iterations, arg_list.size_enclosure, arg_list.time_step);

	if (write_config("final_config.txt", arg_list.size_enclosure, arg_list.time_step, o_aos))
		log_n_exit("Error while trying to write to final_config.txt\n", 1);

	return 0;
}
